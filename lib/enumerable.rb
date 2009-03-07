module Enumerable
  def each_with_index
    i = 0
    each do |item|
      yield item, i
      i = i + 1
    end
  end
  
  # TODO need return(args...)
  # def include?(item)
  #   each do |i|
  #     return true if i == item
  #   end
  #   false
  # end
  
  # TODO need upval
  # def to_a
  #   a = []
  #   each do |item|
  #     a << item
  #   end
  #   a
  # end
end