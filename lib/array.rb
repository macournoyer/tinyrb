class Array
  include Enumerable
  
  def each
    i = 0
    while i < size
      yield self[i]
      i = i + 1
    end
  end
  
  def first
    self[0]
  end
end
