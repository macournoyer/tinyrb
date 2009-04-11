def guacamole
  raise ArgumentError, "ouch!"
end

def deep_in
  guacamole
end

deep_in
puts "oh no boy, ur not coming here!"

# => ArgumentError: ouch!
# => 	from test/raise.rb:2:in `guacamole'
# => 	from test/raise.rb:6:in `deep_in'
# => 	from test/raise.rb:10
