class String
  include Comparable
  
  alias :length :size
  
  def !=(other)
    !(self == other)
  end
  
  def [](f, l=nil)
    if Range === f
      start = f.first
      if f.last < 0
        len = size + f.last + 1
      else
        len = size - f.last - 2
      end
    else
      start = f
      len   = l
    end
    
    len = size - start if len < 0 || start + len > size
    
    substring(start, len)
  end
  
  def inspect
    '"' + self + '"'
  end
end
