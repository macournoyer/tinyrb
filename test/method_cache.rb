def twice(x)
  puts x.to_s
end

# This caches the lookup for `Fixnum + Fixnum`
twice 1
# => 1

# This will skip the previous cache
twice "yo"
# => yo
