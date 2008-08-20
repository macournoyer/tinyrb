class String
  alias :original_slice :[]
  def [](f, l=nil)
    if Range === f
      original_slice(f.first, size - f.last)
    else
      original_slice(f,l)
    end
  end
end
