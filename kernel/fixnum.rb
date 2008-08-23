class Fixnum
  def <=(other)
    self == other || self < other
  end

  def >=(other)
    self == other || self > other
  end
  
  def inspect
    self.to_s
  end
end