if true
  puts :true
end
# => true

if false; puts :nope end
# =>

if nil
  puts :nope
end
# =>

unless nil
  puts :yay
end
# => yay

puts :true if true
# => true

puts :nope unless true
# =>

if false
  puts :nope
else
  puts :yay
end
# => yay

unless true
  puts :nope
else
  puts :yay
end
# => yay

x = if false
      1
    end
puts x
# => 

x = if true
      1
    end
puts x
# => 1
