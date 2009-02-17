def make_awesome
  yield
  yield
end

make_awesome do
  puts "awesome!"
end
# => awesome!
# => awesome!

def yielder(arg)
  yield arg
end

yielder("I feel like having a cupcake") do |txt|
  puts txt
end
# => I feel like having a cupcake

class Array
  def each
    i = 0
    while i < size
      yield self[i]
      i = i + 1
    end
  end
end

[1, 2].each do |i|
  puts i
end
# => 1
# => 2