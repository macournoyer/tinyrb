def make_awesome
  yield
  yield
end

make_awesome do
  puts "awesome!"
end
# => awesome!
# => awesome!