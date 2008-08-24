module Enumerable
  def each_with_index
    i = 0
    each do |item|
      yield item, i
      i += 1
    end
  end
  
  def include?(item)
    each do |i|
      return true if i == item
    end
    false
  end
end