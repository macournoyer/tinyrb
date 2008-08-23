class Object
  def ==(other)
    object_id == other.object_id
  end
  
  # Do not use alias so this is inherited
  def ===(other)
    self == other
  end
  def equal?(other)
    self == other
  end
  def eql?(other)
    self == other
  end
end