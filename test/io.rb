STDOUT.syswrite("ohaie")
puts "" # to add new line
# => ohaie

puts STDIN.fileno
# => 0
puts STDOUT.fileno
# => 1
puts STDERR.fileno
# => 2
