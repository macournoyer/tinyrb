class Class
  def ==(other)
    other.class.to_s == "Class" && name == other.name
  end
  
  # TODO move to Module
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
