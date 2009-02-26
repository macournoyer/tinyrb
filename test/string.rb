puts "ohaie"
# => ohaie

puts "ohaie".size
# => 5

puts "ohaie".class.name
# => String

puts "oh" + "aie"
# => ohaie

puts "oh\naie"
# => oh
# => aie

puts "\"oh\" \\ \\n"
# => "oh" \ \n

puts '\''
# => '

puts "
hey
"
# => 
# => hey
# => 

a = "xxx"
a.replace "aaa"
puts a
# => aaa

puts "a" <=> "b"
# => -1
puts "a" <=> "a"
# => 0

puts "ohaie".substring(0,2)
# => oh
puts "ohaie".substring(2,3)
# => aie
puts "ohaie".substring(2,4)
# => 
