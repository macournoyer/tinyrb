class Hash
  def values
    v = []
    keys.each { |k| v << self[k] }
    v
  end
  
  def include?(key)
    keys.include?(key)
  end
  alias :has_key? :include?
  alias :key? :include?
  alias :member? :include?
  
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
    a = []
    keys.each do |key|
      a << key.inspect + "=" + self[key].inspect
    end
    "{" + a.join(", ") + "}"
  end
end