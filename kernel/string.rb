class String
  include Comparable
  include Enumerable
  
  alias :length :size
  
  def !=(other)
    !(self == other)
  end
  
  def <<(s)
    replace self + s
  end
  
  def [](f, l=nil)
    if Range === f
      start = f.first
      last  = f.last < 0 ? size + f.last : f.last
      len   = last - start + 1
    else
      start = f
      len   = l
    end
    
    len = size - start if len < 0 || start + len > size
    
    substring(start, len)
  end
  
  def succ
    
  end
  
  def to_s
    self
  end
  
  def inspect
    '"' + self + '"'
  end
end
