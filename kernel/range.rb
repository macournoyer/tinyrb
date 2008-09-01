class Range
  include Enumerable
  
  alias :begin :first
  alias :end :last
  
  def include?(other)
    other.is_a?(first.class) && first <= other && other <= last
  end
  alias :=== :include?
  
  def each
    current = first
    yield current
    while (current <=> last) != 0
      current = current.succ
      yield current
    end
  end
  
  def inspect
    "(" + first.inspect + ".." + last.inspect + ")"
  end
end