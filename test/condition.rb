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
