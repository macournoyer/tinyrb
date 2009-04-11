puts Regexp.new("[0-9]").match("aaa 1!!").to_a.inspect
# => ["1"]

puts /./.match("aaa 1!!").to_a.inspect
# => ["a"]

puts /z/.match("aaa 1!!")
# => 

# TODO options
# puts /A/i.match("aaa 1!!").to_a.inspect
# # => ["a"]
