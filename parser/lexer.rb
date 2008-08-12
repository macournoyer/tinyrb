class ParseToken
  attr_accessor :type, :value
  def initialize(type, value)
    @type  = type
    @value = value
  end
  def inspect
    "<#{@type}:#{@value.inspect}>"
  end
end

class Lexer
  OPERATORS = "=+-/*><!?[](){},.|&"
  KEYWORDS = ["if", "unless", "while", "until", "do", "end", "def", "class", "else", "elsif"]
  NAME = "$@_:"
  
  def initialize(string)
    @string = string
    @i = 0
    @line = 1
    @tokens = []
  end
  
  def size
    @string.size
  end
  
  def tokenize
    while @i < size
      c = @string[@i,1]

      if c == "\n" || c == ";"
        @tokens << ParseToken.new(:nl, nil)
        @i += 1
        @line += 1 if c == "\n"
      elsif c == '#'
        str = tok_comment
        @i += str.size
      elsif OPERATORS.include?(c)
        @tokens << ParseToken.new(:operator, c)
        @i += 1
      elsif c[0] >= ?0 && c[0] <= ?9
        @tokens << ParseToken.new(:number, c.to_i)
        @i += 1
      elsif c[0] >= ?a && c[0] <= ?z
        str = tok_name
        if KEYWORDS.include?(str)
          @tokens << ParseToken.new(:keyword, str)
        else
          @tokens << ParseToken.new(:name, str)
        end
        @i += str.size
      elsif c[0] >= ?A && c[0] <= ?Z || c == "_"
        str = tok_name
        @tokens << ParseToken.new(:const, str)
        @i += str.size
      elsif c == "@"
        str = tok_name
        @tokens << ParseToken.new(:ivar, str)
        @i += str.size
      elsif c == "$"
        str = tok_name
        @tokens << ParseToken.new(:gvar, str)
        @i += str.size
      elsif c == ":"
        str = tok_name
        @tokens << ParseToken.new(:symbol, str)
        @i += str.size
      elsif c == '"' || c == "'"
        str = tok_string
        @tokens << ParseToken.new(:string, str)
        @i += str.size + 2
      elsif c == " " || c == "\t"
        @i += 1
      else
        raise "Can't tokenize: " + c + " at line " + @line.to_s
      end
    end

    @tokens
  end

  def tok_string
    quote = @string[@i,1]
    s = i = @i + 1
    i += 1 while @string[i,1] != quote && i < size
    @string[s,i-s]
  end

  def tok_name
    s = i = @i
    i += 1 while @string[i] && ((@string[i] >= ?a && @string[i] <= ?z) || (@string[i] >= ?A && @string[i] <= ?Z) || NAME.include?(@string[i,1]))
    @string[s,i-s]
  end

  def tok_comment
    s = i = @i
    i += 1 while @string[i,1] != "\n"
    @string[s,i-s]
  end
end

if __FILE__ == $PROGRAM_NAME
  code = File.read(__FILE__)
  puts Lexer.new(code).tokenize.inspect
end