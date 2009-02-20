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

def reyielder(arg)
  yielder(arg) do |arg|
    yield "again"
    yield arg
  end
end

reyielder("I really feel like having a cupcake") do |txt|
  puts txt
end
# => again
# => I really feel like having a cupcake

class Array
  def each_with_index
    i = 0
    while i < size
      yield self[i], i
      i = i + 1
    end
  end
  
  def each # toplevel block
    each_with_index do |x, i|
      yield x
    end
  end
end

[1, 2].each do |i|
  puts i
end
# => 1
# => 2

[1, 2].each_with_index do |x, i|
  puts x, i
end
# => 1
# => 0
# => 2
# => 1
