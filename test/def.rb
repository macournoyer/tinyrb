def pony
  puts "glitter"
end

pony
# => glitter
pony
# => glitter

def volume(msg, level)
  puts msg
  puts level
end

volume("it goes to", 11)
# => it goes to
# => 11

def lolcat(say)
  say
end

puts lolcat("meow")
# => meow

def local(x)
  y = x
  y
end

puts local(1)
# => 1