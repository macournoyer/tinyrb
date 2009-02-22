class Array
  def each
    i = 0
    while i < size
      yield self[i]
      i = i + 1
    end
  end
end
