class Symbol
  def to_s
    "" + self
  end
  
  def inspect
    ":" + self.to_s
  end
end