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
end

describe String do
  it "should equal same string" do
    "ohaie".should == "ohaie"
  end
  it "should not equal different string" do
    "ohaie".should_not == "ohaie2"
    "ohaie".should != "ohaie2"
  end
  it "should concatenate" do
    ("oh" + "aie").should == "ohaie"
  end
  it "should slice" do
    "ohaie"[0,2].should == "oh"
    "ohaie"[2,10].should == "aie"
  end
  xit "should slice with Range" do
    "ohaie"[0..1].should == "oh"
    "ohaie"[2..10].should == "aie"
  end
end

describe Class do
  it "should have a name" do
    String.name.should == "String"
    Fixnum.name.should == "Fixnum"
  end
  it "should equal same class" do
    String.should == String
    String.should_not == "hi"
  end
  it "should === object of same class" do
    String.should === "hi"
    Fixnum.should_not === "hi"
  end
end

describe Fixnum do
  it "should convert to string" do
    1.to_s.should == "1"
  end
end

describe true do
  it "should convert to string" do
    true.to_s.should == "true"
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

describe "stdio" do
  it "should return argv" do
    ARGV.class.should == Array
  end
end

describe "method" do
  def some_method!(x, y="cool")
    x + y
  end
  def method_with_block
    "yield:" + yield
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
  xit "should call with arg" do
    Proc.new { |hi| hi + " from proc" }.call("hi").should == "hi from proc"
  end
end

print_spec_summary!

# # class
# class Poop
#   def initialize(answer)
#     @smell = "smell"
#     @@answer = answer
#   end
#   def do_you_smell?
#     "do you " + @smell + "? "
#   end
#   def answer(excl)
#     @@answer + excl
#   end
# end
# p = Poop.new("yes")
# puts p.do_you_smell? + p.answer("!!!")
