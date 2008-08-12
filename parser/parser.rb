require "lexer"

# Inspired by http://javascript.crockford.com/tdop/tdop.html
class Parser
  SYMBOL_TABLE = {}
  ITSELF = proc { |t| t }
  
  class Symbol
    attr_accessor :nud, :led, :id, :lbp, :value, :arity, :reserved, :std, :assignment
    def initialize
      @nud = proc { raise "Undefined" }
      @led = proc { |left| raise "Missing operator" }
    end
  end
  
  class Token
    attr_accessor :nud, :led, :std, :lbp, :scope
  end
  
  class Scope
    attr_accessor :parent, :def
    
    def initialize(parent)
      @parent = parent
      @def = {}
    end
    
    def define(n)
      @def[n.value] = n
      n.nud   = ITSELF
      n.led   = nil
      n.std   = nil
      n.lbp   = 0
      n.scope = self
      n
    end
    
    def find(n)
      e = self
      while true
        o = e.def[n]
        return o if o
        e = e.parent
        return SYMBOL_TABLE[n] || SYMBOL_TABLE["(name)"] unless e
      end
    end
    
    def reserve(n)
      return if n.arity != "name" || n.reserved
      t = @def[n.value]
      if t
        return if t.reserved
        if t.arity === "name"
          raise "Already defined."
        end
      end
      @def[n.value] = n
      n.reserved = true
    end
  end
  
  def initialize(string)
    @tokens   = Lexer.new(string).tokenize
    @token    = nil
    @token_nr = 0
    @scope    = Scope.new(nil)
    
    symbol("(end)")
    symbol("(name)")
    symbol(":")
    symbol(";")
    symbol("\n")
    symbol(",")
    symbol(")")
    symbol("]")
    symbol("}")
    symbol("else")
    symbol("end")
    
    constant("true", true)
    constant("false", false)
    constant("nil", nil);
    
    symbol("(literal)").nud = ITSELF
    
    symbol("self").nud = proc do |t|
      @scope.reserve(t)
      t.arity = "self"
      t
    end
    
    assignment("=")
    assignment("+=")
    assignment("-=")
    
    infixr("&&", 30)
    infixr("||", 30)
    
    infixr("==", 40)
    infixr("!=", 40)
    infixr("<", 40)
    infixr("<=", 40)
    infixr(">", 40)
    infixr(">=", 40)
    
    infixr("+", 50)
    infixr("-", 50)
    
    infixr("*", 60)
    infixr("/", 60)
    
    infix(".", 80) do |t, left|
      t.first = left;
      if @token.arity != "name"
        raise "Expected a property name"
      end
      @token.arity = "literal"
      t.second = @token
      t.arity = "binary"
      advance
      t
    end
    
    infix("[", 80) do |t, left|
      t.first = left
      t.second = expression(0)
      t.arity = "binary"
      advance("]")
      t
    end
    
    # infix("(", 80) TODO
    
    prefix("!")
    prefix("-")
    
    prefix("(") do |t|
      e = expression(0)
      advance(")")
      return e
    end
  end
  
  def scope_pop
    @scope = @scope.parent
  end
  
  def advance(id=nil)
    if id && @token.id != id
      raise "Expected: " + id.inspect
    end
    if @token_nr >= @tokens.size
      @token = SYMBOL_TABLE["(end)"]
      return
    end
    t = @tokens[@token_nr]
    @token_nr += 1
    v = t.value
    a = t.type
    if a == :name
      o = @scope.find(v)
    elsif a == :operator
      o = SYMBOL_TABLE[v]
      raise "Unknown operator: " + v.inspect unless o
    elsif a == :string || a == :number
      a = "literal"
      o = SYMBOL_TABLE["(literal)"]
    else
      raise "Unexpected token: " + t.inspect
    end
    @token = o.dup
    @token.value = v
    @token.arity = a
    @token
  end
  
  def expression(rbp)
    t = @token
    advance
    left = t.nud.call(t)
    while rbp < @token.lbp
      t = @token
      advance
      left = t.left.call(left)
    end
    left
  end
  
  def statement
    n = @token
    if n.std
      advance
      @scope.reserve(n)
      n.std
    end
    v = expression(0)
    if !v.assignment && v.id != "("
      raise "Bad expression statement."
    end
    advance(";")
    v
  end
  
  def statements
    a = []
    while true
      break if @token.id == "}" || @token.id == "(end)"
      s = statement
      a << s if s
    end
    return a.size == 0 ? nil : a.size == 1 ? a[0] : a
  end
  
  def block
    t = @token
    advance("{")
    t.std
  end
  
  def symbol(id, bp=0)
    s = SYMBOL_TABLE[id]
    if s
      if bp > s.lbp
        s.lbp = bp
      end
    else
      s = Symbol.new
      s.id = id
      s.lbp = bp
      SYMBOL_TABLE[id] = s
    end
    s
  end
  
  def constant(s, v)
    x = symbol(s)
    x.nud = proc do |t|
      @scope.reserve(t)
      t.value = SYMBOL_TABLE[t.id].value
      t.arity = "literal"
      t
    end
    x.value = v
    x
  end
  
  def infix(id, bp, &led)
    s = symbol(id, bp)
    s.led = led || proc do |t, left|
      t.first = left
      t.second = expression(bp)
      t.arity = "binary"
      t
    end
    s
  end
  
  def infixr(id, bp, &led)
    s = symbol(id, bp)
    s.led = led || proc do |t, left|
      t.first = left
      t.second = expression(bp - 1)
      t.arity = "binary"
      t
    end
    s
  end
  
  def assignment(id)
    infixr(id, 10) do |t, left|
      if left.id != "." && left.id != "[" && left.arity != "name"
        raise "Bad lvalue."
      end
      t.first = left
      t.second = expression(9)
      t.assignment = true
      t.arity = "binary"
      t
    end
  end
  
  def prefix(id, &nud)
    s = symbol(id)
    s.nud = nud || proc do |t|
      @scope.reserve(t)
      t.first = expression(70)
      t.arity = "unary"
      t
    end
    s
  end
  
  def stmt(s, f)
    x = symbol(s)
    x.std = f
    x
  end
  
  def parse
    advance
    s = statements
    advance("(end)")
    scope_pop
    s
  end
end

if __FILE__ == $PROGRAM_NAME
  code = %{a}
  
  puts Parser.new(code).parse.inspect
end