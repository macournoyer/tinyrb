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
  end
  it "should slice over end" do
    "ohaie"[2,10].should == "aie"
  end
  it "should slice with Range" do
    "ohaie"[0..1].should == "oh"
    "ohaie"[2..10].should == "aie"
  end
  it "should slice with negative" do
    # TODO hangs!11!!??: "ohaie"[2..-1].should == "aie"
    "ohaie"[0..-4].should == "oh"
  end
  it "should have size" do
    "ohaie".size.should == 5
    "ohaie".length.should == 5
  end
  it "should replace" do
    "ohaie".replace("lol").should == "lol"
  end
end

class Poop
  def initialize(ivar, cvar=nil)
    @ivar  = ivar
    @@cvar = cvar
  end
  def ivar; @ivar end
  def cvar; @@cvar end
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
  xit "should include?" do
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
  xit "should inspect" do
    [1, "hi"].inspect.should == '[1, "hi"]'
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
  
  it "should call with block" do
    method_with_block { "block" }.should == "yield:block"
  end

  it "should call with block arg" do
    method_with_block_args { |arg, exl| arg + exl }.should == "yield:arg!"
  end
  
  it "should be aliasable" do
    some_alias("var").should == "varcool"
  end
end

describe VM do
  it "should run opcode" do
    VM.run("boot.rb", [
      [1, 19, "yeah!"]
    ]).should == "yeah!"
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
end


print_spec_summary!
