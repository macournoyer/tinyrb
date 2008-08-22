class Class
  def ===(obj)
    name == obj.class.name
  end
  
  def to_s
    name
  end
  
  def inspect
    name
  end
end
