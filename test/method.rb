puts 1.method(:to_s).name
# => to_s
puts 1.method(:to_s).arity
# => 0
puts method(:method).arity
# => 1
puts method(:puts).arity
# => -1
puts method(:poop)
# => 

puts :name.method(:to_s).name
# => to_s
