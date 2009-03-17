def yielder
  yield
end

x = "yeaaah!"
puts yielder { x }
# => yeaaah!

# TODO
# HACK nested {} cause infinite recursion in parser
# puts yielder do; yielder do; x end end
# # => yeaaah!