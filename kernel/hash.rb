class Hash
  def values
    v = []
    keys.each { |k| v << self[k] }
    v
  end
  
  def empty?
    size == 0
  end
  
  def clear
    keys.each { |k| delete(k) }
  end
  
  def update(other_hash)
    other_hash.keys.each do |key|
      self[key] = other_hash[key]
    end
    self
  end
  alias :merge! :update
  
  def inspect
    s = "{"
    keys.each do |key|
      s << key.inspect + "=" + self[key].inspect
      s << ", "
    end
    s << "}"
    s
  end
end