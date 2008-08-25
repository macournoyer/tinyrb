class Object
  def ==(other)
    object_id == other.object_id
  end
  
  def is_a?(klass)
    self.class == klass
  end
  
  def nil?
    false
  end
  
  def !
    false
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
  
  def inspect
    '#<' + self.class.name + ':0x' + object_id.to_s + '>'
  end
  
  alias :to_s :inspect
end