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