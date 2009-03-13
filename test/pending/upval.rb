def print_block
  puts yield
end

x = "yeaaah!"
print_block { x }
# => yeaaah!
