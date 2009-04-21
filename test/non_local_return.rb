def yielder
  yield
end

def non_local_return
  yielder do
    puts "ok"
    return 1
    puts "nop"
  end
  puts "should not get here"
end

puts non_local_return
# => ok
# => 1
