def method_missing(name, *args)
  puts "called method_missing"
  puts name
end

ohaie!
# => called method_missing
# => ohaie!

def ohaie!
  puts "ok"
end

ohaie!
# => ok
