class Range
  include Enumerable
  
  def inspect
    "(" + first.inspect + ".." + last.inspect + ")"
  end
end