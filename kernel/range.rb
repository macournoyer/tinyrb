class Range
  include Enumerable
  
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