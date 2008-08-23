class Class
  def ==(other)
    other.class.to_s == "Class" && name == other.name
  end

  def ===(obj)
    self == obj.class
  end
  
  def to_s
    name
  end
  
  def inspect
    name
  end
end
