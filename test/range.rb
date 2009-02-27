puts (0..2).first
# => 0
puts (0..2).last
# => 2

puts (0..2).exclude_end?
# => false

puts (0...2).exclude_end?
# => true

# TODO fix failure in fixnum.rb first
# puts (0..2).to_s
# # => 0..2

# puts (0..2).to_a
# # => 0
# # => 1
# # => 2
# 
# puts (0...2).to_a
# # => 0
# # => 1
