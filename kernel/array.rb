class Array
  include Enumerable
  
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
  
  def join(sep=$,)
    str = ""
    i   = 0
    while i < size
      str << self[i].to_s
      str << sep.to_s unless i == size - 1
      i += 1
    end
    str
  end
  
  def inspect
    "[" + join(", ") + "]"
  end
end