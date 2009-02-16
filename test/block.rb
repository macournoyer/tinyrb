def make_awesome
  yield
  yield
end

make_awesome do
  puts "awesome!"
end
# => awesome!
# => awesome!

# def yielder(arg)
#   yield arg
# end
# 
# puts yielder("I feel like havin 2 muffins")
# # => I feel like havin 2 muffins
