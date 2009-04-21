def yielder
  yield
end

def non_local_break
  yielder do
    puts "ok"
    break
    puts "nop"
  end
  puts "still"
end

non_local_break
puts "end"
# => ok
# => still
# => end
