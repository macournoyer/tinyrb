require "lexer"
require "emitter"

class Parser
  def initialize(string)
    @tokens = Lexer.new(string).tokenize
    @result = []
    @cur_token = 0
    @token = nil
  end
  
  def advance
    prev = @token
    @token = @tokens[@cur_token]
    
    if @token.type == :name
      @result << :asgn
      @result << @token.value
    end
  end
  
  def parse
    advance
  end
end

if __FILE__ == $PROGRAM_NAME
  code = <<-EOS
    a = "0"
  EOS

  puts Parser.new(code).parse.inspect
end