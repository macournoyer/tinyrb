require 'stringio'

# Original by Markus Liedl

class SExpReader
  attr_reader :factory

  def self.read_string(string)
    new(StringIO.new(string)).read();
  end

  def self.read_file_all(filename)
    rd = new(File.new(filename, "r"));
    if block_given?
      while (x = rd.read())
        yield x;
      end
    else
      res = Array.new();
      while (x = rd.read())
        res.push(x);
      end
      res;
    end
  end

  def initialize(input)
    @input = input
    consume()
    skip_space()
  end

  def consume()
    @ch = @input.getc()
  end

  def many_of(chars)
    consume() while @ch and chars.include?(@ch)
  end

  def skip_space()
    many_of(" \t\n")
    if pass(";")
      consume while @ch and @ch != ?\n;
      skip_space();
    end
  end

  def looking_at(chars)
    @ch and chars.include?(@ch)
  end

  def pass(chars)
    looking_at(chars) and (consume(); true)
  end

  def read()
    skip_space();
    if pass("(")
      read_form()
    elsif pass('"')
      read_string()
    elsif looking_at_digit() or looking_at("-")
      read_number()
    elsif looking_at_wordchar1
      read_symbol()
    elsif pass("'")
      [:quote, read()]
    elsif pass("?")
      if @ch == ?\\
        consume;
        if @ch == ?x
          res = @input.read(2).to_i(16).chr();
        else fail 'bad chararter syntax';
        end
      else
        res = @ch.chr
      end
      consume;
      res;
    elsif @ch == nil
      nil;
    else
      puts "bad char: #{@ch.chr}" if Integer === @ch;
      fail "bad syntax (current char: #{@ch.inspect})"
    end
  end

  def looking_at_digit()
    @ch and @ch >= ?0 and @ch <= ?9
  end

  Wordchars1 = (?a..?z).to_a + (?A..?Z).to_a + [?_, ?@] 
  def looking_at_wordchar1()
    @ch and Wordchars1.include?(@ch)
  end

  Wordchars = (?a..?z).to_a + (?A..?Z).to_a + (?0..?9).to_a + [?-, ?_, ?@]
  def looking_at_wordchar()
    @ch and Wordchars.include?(@ch)
  end

  def read_number()
    res = 0
    negative = pass("-")
    while looking_at_digit()
      res = res*10 + @ch-?0
      consume
    end
    res;
  end

  def read_symbol()
    res = []
    while looking_at_wordchar()
      res << @ch.chr
      consume()
    end
    res.join().gsub("-", "_").intern();
  end

  def read_string()
    res = [];
    while @ch != ?"
      string_backslash if @ch == ?\\;
      res << @ch.chr
      consume()
    end
    consume();
    res.join();
  end

  StringBackslash = { ?n => ?\n, ?r => ?\r, ?t => ?\t, ?\\ => ?\\, ?" => ?" }
  def string_backslash()
    consume();
    @ch = (StringBackslash[@ch] or fail "backslash preceeding `#{@ch.chr}'");
  end

  def read_form()
    elements = Array.new
    while true
      skip_space()
      fail 'expected closing paren' if not @ch;
      break if pass(')');
      elements << read();
    end
    elements;
  end
end

if __FILE__ == $PROGRAM_NAME
  puts "GRAMMAR = " + SExpReader.read_file_all("gram.lisp").inspect
end