def twice(x)
  puts x + x
end

# This caches the lookup for `Fixnum + Fixnum`
twice 1
# => 2

# This will skip the previous cache
twice "yo"
# => yoyo
