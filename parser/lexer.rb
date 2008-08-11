class Lexer
  OPERATORS = %w(= + - / * > < !)
  KEYWORDS = %w(if unless while until do end def class)
  
  def initialize(string)
    @string = string
    @i = 0
    @tokens = []
  end
  
  def size
    @string.size
  end
  
  def tokenize
    while @i < size
      c = @string[@i,1]

      if c == "\n" || c == ";"
        @tokens << [:nl, nil]
        @i += 1
      elsif c == '#'
        str = tok_comment
        @i += str.size
      elsif OPERATORS.include?(c)
        @tokens << [:msg, c]
        @i += 1
      elsif c[0] >= ?0 && c[0] <= ?9
        @tokens << [:num, c.to_i]
        @i += 1
      elsif c[0] >= ?a && c[0] <= ?z
        str = tok_name
        if KEYWORDS.include?(str)
          @tokens << [:keyw, str]
        else
          @tokens << [:name, str]
        end
        @i += str.size
      elsif c == '"'
        str = tok_string
        @tokens << [:str, str]
        @i += str.size + 2
      elsif c == "."
        @tokens << [:dot, nil]
        @i += 1
      elsif c == " " || c == "\t"
        @i += 1
      else
        raise "Can't tokenize: " + c
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
    i += 1 while @string[i,1] >= "a" && @string[i,1] <= "z"
    @string[s,i-s]
  end

  def tok_comment
    s = i = @i
    i += 1 while @string[i,1] != "\n"
    @string[s,i-s]
  end
end

if __FILE__ == $PROGRAM_NAME
  code = <<-EOS
    a = "0"
    x = 1
    puts "allo"
    obj.hi
  EOS

  puts Lexer.new(code).tokenize.inspect
end