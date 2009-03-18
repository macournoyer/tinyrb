def yielder
  yield
end

x = "yeaaah!"
y = yielder do
  x
end
puts y
# => yeaaah!

y = yielder do
  yielder do
    x
  end
end
puts y
# => yeaaah!

yielder do
  x = "waaaa?"
end
puts x
# => waaaa?