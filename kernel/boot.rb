# == specials.rb
class TrueClass
  def !
    false
  end
  def to_s
    "true"
  end
  def inspect
    "true"
  end
end

class FalseClass
  def !
    true
  end
  def to_s
    "false"
  end
  def inspect
    "false"
  end
end

class NilClass
  def ==(other)
    NilClass === other || FalseClass === other
  end
  def !
    true
  end
  def nil?
    true
  end
  def to_s
    ""
  end
  def inspect
    "nil"
  end
end
# == object.rb
class Object
  def instance_variable_get(name)
    VM.run [
      [0, OPCODES[:getinstancevariable], name]
    ]
  end
  
  def instance_variable_set(name, value)
    VM.run [
      [0, OPCODES[:putobject], value],
      [0, OPCODES[:setinstancevariable], name]
    ]
  end
  
  def ==(other)
    object_id == other.object_id
  end
  
  def is_a?(klass)
    self.class == klass
  end
  
  def nil?
    false
  end
  
  def !
    false
  end
  
  # Do not use alias so this is inherited
  def ===(other)
    self == other
  end
  def equal?(other)
    self == other
  end
  def eql?(other)
    self == other
  end
  
  def hash
    object_id
  end
  
  def inspect
    '#<' + self.class.name + ':0x' + object_id.to_s + '>'
  end
  alias :to_s :inspect
  
  def dup
    obj = self.class.new
    instance_variables.each do |name|
      obj.instance_variable_set name, instance_variable_get(name)
    end
    obj
  end
  
  def clone
    dup
  end
end
# == module.rb
module Module
end
# == class.rb
class Class
  def ==(other)
    other.class.name == "Class" && name == other.name
  end
  
  # TODO move to Module
  def ===(obj)
    self == obj.class
  end
  
  def to_s
    name
  end
  
  def inspect
    name
  end
end

# == enumerable.rb
module Enumerable
  def each_with_index
    i = 0
    each do |item|
      yield item, i
      i += 1
    end
  end
  
  def include?(item)
    each do |i|
      return true if i == item
    end
    false
  end
  
  def to_a
    a = []
    each { |item| a << item }
    a
  end
end
# == comparable.rb
module Comparable
  def <(other)
    (self <=> other) < 0
  end
  
  def <=(other)
    (self <=> other) <= 0
  end
  
  def ==(other)
    (self <=> other) == 0
  end
  
  def >(other)
    (self <=> other) > 0
  end
  
  def >=(other)
    (self <=> other) >= 0
  end
  
  def between?(min, max)
    self >= min && self <= max
  end
end
# == fixnum.rb
class Fixnum
  def upto(last)
    i = self
    while i <= last
      yield i
      i += 1
    end
  end
  
  def times
    i = 0
    while i < self
      yield
      i += 1
    end
    nil
  end
  
  def <=(other)
    self == other || self < other
  end
  
  def >=(other)
    self == other || self > other
  end
  
  alias :<=> :-
  
  def !=(other)
    !(self == other)
  end
  
  def succ
    self + 1
  end
  
  def dup
    0 + self
  end
  
  def inspect
    self.to_s
  end
end
# == string.rb
class String
  include Comparable
  include Enumerable
  
  alias :length :size
  
  def !=(other)
    !(self == other)
  end
  
  def <<(s)
    replace self + s
  end
  
  def [](f, l=nil)
    if Range === f
      start = f.first
      last  = f.last < 0 ? size + f.last : f.last
      len   = last - start + 1
    else
      start = f
      len   = l
    end
    
    len = size - start if len < 0 || start + len > size
    
    substring(start, len)
  end
  
  def to_s
    self
  end
  
  def inspect
    '"' + self + '"'
  end
end

# == symbol.rb
class Symbol
  def to_s
    "" + self
  end
  
  def inspect
    ":" + self.to_s
  end
end
# == array.rb
class Array
  include Enumerable
  
  alias :length :size
  alias :push :<<
  
  def first
    self[0]
  end
  
  def each
    i = 0
    while i < size
      yield self[i]
      i += 1
    end
  end
  
  def join(sep=$,)
    str = ""
    i   = 0
    while i < size
      str << self[i].to_s
      str << sep.to_s unless i == size - 1
      i += 1
    end
    str
  end
  
  def inspect
    "[" + join(", ") + "]"
  end
end
# == hash.rb
class Hash
  def values
    v = []
    keys.each { |k| v << self[k] }
    v
  end
  
  def include?(key)
    keys.include?(key)
  end
  alias :has_key? :include?
  alias :key? :include?
  alias :member? :include?
  
  def empty?
    size == 0
  end
  
  def clear
    keys.each { |k| delete(k) }
  end
  
  def update(other_hash)
    other_hash.keys.each do |key|
      self[key] = other_hash[key]
    end
    self
  end
  alias :merge! :update
  
  def inspect
    a = []
    keys.each do |key|
      a << key.inspect + "=" + self[key].inspect
    end
    "{" + a.join(", ") + "}"
  end
end
# == range.rb
class Range
  include Enumerable
  
  alias :begin :first
  alias :end :last
  
  def include?(other)
    other.is_a?(first.class) && first <= other && other <= last
  end
  alias :=== :include?
  
  def each
    current = first
    yield current
    while (current <=> last) != 0
      current = current.succ
      yield current
    end
  end
  
  def inspect
    "(" + first.inspect + ".." + last.inspect + ")"
  end
end
# == vm.rb
OPCODES = {
  :nop                 =>  0,
  :getlocal            =>  1,
  :setlocal            =>  2,
  :getspecial          =>  3,
  :setspecial          =>  4,
  :getdynamic          =>  5,
  :setdynamic          =>  6,
  :getinstancevariable =>  7,
  :setinstancevariable =>  8,
  :getclassvariable    =>  9,
  :setclassvariable    => 10,
  :getconstant         => 11,
  :setconstant         => 12,
  :getglobal           => 13,
  :setglobal           => 14,
  :putnil              => 15,
  :putself             => 16,
  :putundef            => 17,
  :putobject           => 18,
  :putstring           => 19,
  :concatstrings       => 20,
  :tostring            => 21,
  :toregexp            => 22,
  :newarray            => 23,
  :duparray            => 24,
  :expandarray         => 25,
  :concatarray         => 26,
  :splatarray          => 27,
  :checkincludearray   => 28,
  :newhash             => 29,
  :newrange            => 30,
  :putnot              => 31,
  :pop                 => 32,
  :dup                 => 33,
  :dupn                => 34,
  :swap                => 35,
  :reput               => 36,
  :topn                => 37,
  :emptstack           => 38,
  :definemethod        => 39,
  :alias               => 40,
  :undef               => 41,
  :defined             => 42,
  :postexe             => 43,
  :trace               => 44,
  :defineclass         => 45,
  :send                => 46,
  :invokesuper         => 47,
  :invokeblock         => 48,
  :leave               => 49,
  :finish              => 50,
  :throw               => 51,
  :jump                => 52,
  :branchif            => 53,
  :branchunless        => 54,
  :setn                => 55,
  
  # my own lil instructions
  :label               => 55

}

class VM
  
end
# == set.rb
class Set
  include Enumerable
  
  def initialize(enum=nil)
    @hash = {}
    enum.each { |item| add(item) } if enum
  end
  
  def add(item)
    @hash[item] = true
  end
  alias :<< :add

  def delete(item)
    @hash[item] = nil
  end
  
  def include?(item)
    @hash[item]
  end
  
  def |(enum)
    dup.merge(enum)
  end
  alias :union :|
  alias :+ :|
  
  def -(enum)
    dup.subtract(enum)
  end
  
  def each
    # TODO @hash.keys.each(&block)
  end

  def merge(enum)
    if enum.is_a?(Set)
      @hash.update(enum.instance_variable_get(:@hash))
    else
      enum.each { |item| add(item) }
    end
    self
  end
  
  def subtract(enum)
    enum.each { |item| delete(item) }
    self
  end
  
  def size
    @hash.size
  end
  alias :length :size
  
  def clear
    @hash.clear
  end
end
# == globals.rb
$, = nil
# == spec.rb
# This is just a basic spec framework to test the kernel while I work running the parser.
# I'll try to make it compatible w/ mspec so I copy them when I'm finished.
# Some of it is ugly cause I don't have Exception rescue yet.

$spec_count = 0
$spec_fail = 0
$spec_ignore = 0

def describe(name)
  puts name.to_s
  yield
  puts ""
end

def it(name)
  puts "  it " + name
  yield
end

def xit(name)
  puts "  ignoring: it " + name
  $spec_ignore += 1
end

def print_spec_summary!
  puts $spec_count.to_s + " examples, " +
       $spec_fail.to_s + " failures, " +
       $spec_ignore.to_s + " ignored"
end

class SpecMatcher
  def initialize(object, negate)
    @object = object
    @negate = negate
  end
  
  def ==(other);  _match "==",  other, @object == other  end
  def >(other);   _match ">",   other, @object > other   end
  def >=(other);  _match ">=",  other, @object >= other  end
  def <(other);   _match "<",   other, @object < other   end
  def <=(other);  _match "<=",  other, @object <= other  end
  def !=(other);  _match "!=",  other, @object != other  end
  def ===(other); _match "===", other, @object === other end
  
  def _cond(cond)
    @negate ? !cond : cond
  end
  def _match(op, other, result)
    $spec_count += 1
    $spec_fail += 1 unless _cond(result)
    puts "    " + @object.inspect + " " + (@negate ? "not " : "") + op + " " + other.inspect + ": " +
                  (_cond(result) ? "\e[0;32mSUCCESS\e[m" : "\e[0;31mFAIL\e[m")
  end
end

class Object
  def should
    SpecMatcher.new(self, false)
  end
  def should_not
    SpecMatcher.new(self, true)
  end
end

# == specs.rb
describe Object do
  it "should have a class" do
    "hi".class.should == String
  end
  it "should inspect" do
    Object.new.inspect.should_not == ""
  end
  it "should have a unique id" do
    Object.new.object_id.should > 10
  end
  it "should be a class" do
    "ohaie".is_a?(String).should == true
    "ohaie".is_a?(Fixnum).should == false
  end
  it "should send message" do
    1.send(:to_s).should == "1"
    1.send("to_s").should == "1"
  end
  it "should dup" do
    obj = Object.new
    obj.instance_variable_set :@var, "val"
    obj2 = obj.dup
    obj2.class.should == obj.class
    obj2.instance_variable_get(:@var).should == "val"
  end
end

describe String do
  it "should equal same string" do
    "ohaie".should == "ohaie"
  end
  it "should not equal different string" do
    "ohaie".should_not == "ohaie2"
    "ohaie".should != "ohaie2"
  end
  it "should compare" do
    "a".should < "z"
    "a".should <= "a"
    "z".should > "a"
    "z".should >= "a"
  end
  it "should concatenate" do
    ("oh" + "aie").should == "ohaie"
  end
  it "should slice" do
    "ohaie"[0,2].should == "oh"
    "ohaie"[2,3].should == "aie"
  end
  it "should slice over end" do
    "ohaie"[2,10].should == "aie"
  end
  it "should slice with Range" do
    "ohaie"[0..1].should == "oh"
    "ohaie"[0..2].should == "oha"
  end
  it "should slice with Range over end" do
    "ohaie"[2..10].should == "aie"
  end
  it "should slice with negative" do
    "ohaie"[2..-1].should == "aie"
    "ohaie"[2..-2].should == "ai"
    "ohaie"[0..-4].should == "oh"
  end
  it "should have size" do
    "ohaie".size.should == 5
    "ohaie".length.should == 5
  end
  it "should replace" do
    "ohaie".replace("lol").should == "lol"
  end
  it "should convert to symbol" do
    "ohaie".to_sym.should == :ohaie
  end
end

class Poop
  def initialize(ivar, cvar=nil)
    @ivar  = ivar
    @@cvar = cvar
  end
  def ivar; @ivar end
  def cvar; @@cvar end
  def self.cmethod; end
end

describe Class do
  it "should have a name" do
    Poop.name.should == "Poop"
  end
  it "should equal same class" do
    Poop.should == Poop
    Poop.should_not == "hi"
  end
  it "should === object of same class" do
    String.should === "hi"
    Fixnum.should_not === "hi"
  end
  it "should call initializer" do
    Poop.new("ivar").ivar.should == "ivar"
  end
  it "should set class var initializer" do
    Poop.new("ivar", "cvar").cvar.should == "cvar"
  end
  it "should define class method" do
    Poop.new("ivar", "cvar")
    Poop.cmethod.should == nil
  end
  it "should get instance variable dynamicly" do
    Poop.new("ivar").instance_variable_get(:@ivar).should == "ivar"
  end
  it "should set instance variable dynamicly" do
    p = Poop.new("ivar")
    p.instance_variable_set(:@ivar, "awesome!")
    p.ivar.should == "awesome!"
  end
end

describe Fixnum do
  it "should add" do
    (3 + 4).should == 7
  end
  it "should sub" do
    (3 - 4).should == -1
  end
  it "should equal same" do
    7.should == 7
    -7.should == -7
  end
  it "should not equal other objects" do
    7.should_not == "7"
  end
  it "should convert to string" do
    1.to_s.should == "1"
  end
  it "should bit shift" do
    (1 << 3).should == 8
    (8 >> 3).should == 1
  end
  it "should bitwise and" do
    (3 & 2).should == 2
  end
  it "should bitwise or" do
    (3 | 2).should == 3
  end
  it "should bitwise not" do
    (~1).should == -2
  end
  it "should negate" do
    (!1).should == false
  end
  it "should yield upto" do
    a = []
    1.upto(3) { |i| a << i }
    a.size.should == 3
  end
  it "should return chr" do
    49.chr.should == "1"
    97.chr.should == "a"
  end
end

describe Range do
  it "should have first and last" do
    (1..2).first == 1
    (1..2).last == 2
  end
  it "should include" do
    (1..2).include?(1).should == true
    (1..2).include?(2).should == true
    (1..2).include?(3).should == false
    (1..2).include?("a").should == false
  end
  it "should to_a (Fixnum)" do
    a = (1..2).to_a
    a.size.should == 2
  end
  xit "should to_a (String)" do
    a = ('a'..'z').to_a
    a.size.should == 26
  end
end

describe true do
  it "should convert to string" do
    true.to_s.should == "true"
  end
end

describe "nil" do
  it "should convert to string" do
    nil.to_s.should == ""
  end
  it "should be false" do
    nil.should == false
    (!nil).should == true
  end
  it "should be nil?" do
    nil.nil?.should == true
    "nil".nil?.should == false
  end
end

describe Symbol do
  it "should convert to string" do
    :that.to_s.class.should == String
    :that.to_s.should == "that"
  end
end

describe "conditional" do
  it "should branch if" do
    cond = "true"
    result = false
    result = true if cond
    result.should == true
  end
  
  it "should branch unless" do
    cond = nil
    result = false
    result = true if cond
    result.should == false
  end
  
  it "should jump on while" do
    count = 0
    while count < 3
      count += 1
    end
    count.should == 3
  end
end

describe Array do
  it "should get" do
    [1][0].should == 1
  end
  it "should set" do
    a = []
    a[0] = 1
    a[0].should == 1
  end
  xit "+"
  it "should iterate each" do
    $i = 0
    [1, 1, 1].each do |i|
      $i += i
    end
    $i.should == 3
  end
  xit "should iterate each_with_index" do
    $i = 0
    [1, 1, 1].each_with_index do |i, ind|
      $i += ind
    end
    $i.should == 3
  end
  it "should include?" do
    [1, 2].include?(2).should == true
    [1, 2].include?(3).should == false
  end
  xit "hash"
  xit "join"
  xit "map"
  it "pop" do
    a = [1, 2]
    a.pop.should == 2
    a.pop.should == 1
  end
  it "push" do
    a = []
    a.push 1
    a[0].should == 1
  end
  it "should have a size" do
    [1, 2, 3].size.should == 3
  end
  it "should join" do
    [1, "a", :x].join.should == "1ax"
    [1, "a", :x].join("-").should == "1-a-x"
  end
end

describe Hash do
  it "should create hash" do
    Hash.should === {}
  end
  it "should create hash with init values" do
    h = { :k => "v" }
    h[:k].should == "v"
  end
  it "should hash Object" do
    o = Object.new
    h = { o => true }
    h[o].should == true
  end
  it "should set" do
    h = {}
    h["hi"] = 1
    h["hi"].should == 1
    h["hi"] = 1
    h.size.should == 1
  end
  it "should return nil if non existant" do
    h = {}
    h["none"].should == nil
  end
  it "should have a size" do
    h = {}
    h.size.should == 0
    h[:k] = 1
    h.size.should == 1
  end
  it "should clear" do
    h = { :k => 1 }
    h.clear
    h[:k].should == nil
    h.size.should == 0
  end
  it "should return keys" do
    h = { :a => 1, :b => 2 }
    h.keys.size.should == 2
    h.keys[0].should == :a
    h.keys[1].should == :b
  end
  it "should return values" do
    h = { :a => 1, :b => 2 }
    h.values.size.should == 2
    h.values[0].should == 1
    h.values[1].should == 2
  end
  it "should update" do
    h = { :a => 1 }
    h.update(:a => 2)
    h[:a].should == 2
  end
  it "should include" do
    h = { :a => 1 }
    h.include?(:a).should == true
    h.include?(:b).should == false
  end
end

describe "stdio" do
  it "should return argv" do
    ARGV.class.should == Array
  end
end

describe "method" do
  def some_method!(x, y="cool")
    x + y
  end
  def some_method2(x, y="cool", z="yes")
    a = 1.to_s
    b = :local.to_s
    c = "ok?"
    a + b + c + x + y + z
  end
  alias :some_alias :some_method!
  def method_with_block
    "yield:" + yield
  end
  def method_with_block_args
    "yield:" + yield("arg", "!")
  end

  it "should call" do
    some_method!("var", "!").should == "var!"
  end
  
  it "should chain calls" do
    some_method!("var", "!").to_s.to_s.to_s.should == "var!"
  end
  
  it "should use default value" do
    some_method!("var").should == "varcool"
  end
  
  it "should use default value with localvars" do
    some_method2("var", "wow").should == "1localok?varwowyes"
  end
  
  it "should call with block" do
    method_with_block { "block" }.should == "yield:block"
  end

  it "should call with block arg" do
    method_with_block_args { |arg, exl| arg + exl }.should == "yield:arg!"
  end
  
  it "should be aliasable" do
    some_alias("var").should == "varcool"
  end
  
  it "should define singleton method" do
    a = "hi"
    def a.to_s; true end
    a.to_s.should == true
    "a".to_s.should == "a"
  end
end

describe VM do
  it "should run opcode" do
    VM.run([[1, 18, "yeah!"]]).should == "yeah!"
  end
  it "should run opcode in different frame" do
    VM.run([[1, 16]], "boot.rb", String, String).should == String
  end
end

describe Proc do
  it "should call" do
    Proc.new { "hi from proc" }.call.should == "hi from proc"
  end
  it "should set var in proc" do
    Proc.new { hi = "hi from proc"; hi }.call.should == "hi from proc"
  end
  it "should call with arg" do
    Proc.new { |hi| hi + " from proc" }.call("hi").should == "hi from proc"
  end
  
  def doubler
    yield
    yield
  end
  it "should use localvar in block" do
    i = 0
    doubler do
      i += 1
    end
    i.should == 2
  end
  
  def doubler_yielder
    i = 0
    yield
    doubler do
      yield
      i += 1
    end
    i
  end
  xit "should use localvar in yielder block" do
    i = doubler_yielder {}
    i.should == 2
  end
end

describe Set do
  it "should create from Array" do
    Set.new([1, 2]).size.should == 2
  end
  
  it "should +" do
    set = Set.new([1, 3]) + Set.new([1, 2])
    set.size.should == 3
  end

  it "should -" do
    set = Set.new([1, 3]) - Set.new([1, 2])
    set.size.should == 2
  end
end
 
print_spec_summary!

