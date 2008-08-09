SYMBOLS = %w(= > < !)

def tokenize(string)
  i      = 0
  len    = string.size
  tokens = []
  
  while i < len
    c = string[i,1]
    
    if c == "\n" || c == ";"
      tokens << [:nl, nil]
      i += 1
    elsif SYMBOLS.include?(c)
      tokens << [:msg, c]
      i += 1
    elsif c[0] >= ?0 && c[0] <= ?9
      tokens << [:number, c.to_i]
      i += 1
    elsif c[0] >= ?a && c[0] <= ?z
      str = tok_name(string, i, len)
      tokens << [:name, str]
      i += str.size
    elsif c == '"'
      str = tok_string(string, i, len)
      tokens << [:dstring, str]
      i += str.size + 2
    elsif c == "'"
      str = tok_string(string, i, len)
      tokens << [:string, str]
      i += str.size + 2
    elsif c == "."
      tokens << [:dot, nil]
      i += 1
    elsif c == " " || c == "\t"
      i += 1
    else
      raise "Can't tokenize: " + c
    end
  end
  
  tokens
end

def tok_string(string, i, l)
  quote = string[i,1]
  s = i = i + 1
  i += 1 while string[i,1] != quote && i < l
  string[s,i-s]
end

def tok_name(str, i, l)
  s = i
  i += 1 while str[i,1] >= "a" && str[i,1] <= "z"
  str[s,i-s]
end

puts tokenize(<<-EOS).inspect
  a = "0"
  x = 1
  puts "allo"
  obj.hi
EOS