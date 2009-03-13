class Fixnum
  def times
    i = 0
    while i < self
      yield i
      i = i + 1
    end
  end
  
  def -@
    0 - self
  end
end
