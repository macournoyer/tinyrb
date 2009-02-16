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
