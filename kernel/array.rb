class Array
  alias :length :size
  alias :push :<<
  
  def first
    self[0]
  end
  
  def each
    i = 0
    while i < size
      yield self[i]
      i += 1
    end
  end
  
  def each_with_index
    i = 0
    while i < size
      yield self[i], i
      i += 1
    end
  end
end