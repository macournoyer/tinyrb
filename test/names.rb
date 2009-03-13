def nice?
  true
end

puts nice?
# => true

def bang!
  "ouch"
end

puts bang!
# => ouch

def name=(v)
  @name = v
end

self.name = 1
puts @name
# => 1

def <<(x)
  puts x
end

self << "yooo"
# => yooo
