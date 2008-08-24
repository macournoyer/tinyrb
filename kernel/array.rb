class Array
  include Enumerable
  
  alias :length :size
  alias :push :<<
  
  def first(arg)
    self[0]
  end
  
  def each
    i = 0
    while i < size
      yield self[i]
      i += 1
    end
  end
  
  def inspect
    out = "["
    each do |i|
      out << i.inspect
      out << ", "
    end
    out << "]"
    out
  end
end