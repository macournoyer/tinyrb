def fact(n)
  if n > 1
    n * fact(n-1)
  else
    1
  end
end

puts fact(4)