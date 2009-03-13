def add(x)
  Proc.new do |y|
    x + y
  end
end

puts add(2).call(3)
# => 5

puts add(3).call(5)
# => 8
