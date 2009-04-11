def guacamole
  raise "ouch!"
end

def deep_in
  guacamole
end

begin
  deep_in
  puts "oh no boy, ur not coming here!"
rescue RuntimeError
  puts $!
  puts $@
rescue
  puts "nooo!"
ensure
  puts "ensured"
end

# => ouch!
# => test/raise.rb:2:in `guacamole'
# => test/raise.rb:6:in `deep_in'
# => test/raise.rb:10
# => ensured
