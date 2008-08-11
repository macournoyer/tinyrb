require "lexer"
require "emitter"

class Parser
  def initialize(string)
    @emitter = Emitter.new
    @lexer   = Lexer.new(string)
    @line    = 1
  end
  
  def parse
    @tokens = @lexer.tokenize
    
    
    
    @emitter.line = @line
    @emitter.op :putstring, "hi"
    @emitter.op :getlocal, "a"
    
    @emitter.out
  end
end

if __FILE__ == $PROGRAM_NAME
  code = <<-EOS
    a = "0"
    x = 1
    puts "allo"
    obj.hi
  EOS

  puts Parser.new(code).parse.inspect
end