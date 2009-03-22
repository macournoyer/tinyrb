module Enumerable
  def each_with_index
    i = 0
    each do |item|
      yield item, i
      i = i + 1
    end
  end
  
  # TODO need non-local return
  # def include?(item)
  #   each do |i|
  #     return true if i == item
  #   end
  #   false
  # end
  
  def to_a
    a = []
    each do |item|
      a << item
    end
    a
  end
end