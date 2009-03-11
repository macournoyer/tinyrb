def oye(x, *a)
  puts x
  puts a.size
  puts a[0]
  puts a[1]
end

oye 1, 2, 3
# => 1
# => 2
# => 2
# => 3

oye 1
# => 1
# => 0
# => 
# => 
