class Hash
  def values
    v = []
    keys.each { |k| v << self[k] }
    v
  end
  
  def empty?
    size == 0
  end
end