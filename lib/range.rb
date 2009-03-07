class Range
  # include Enumerable
  
  def to_s
    first.to_s + ".." + last.to_s
  end
end
