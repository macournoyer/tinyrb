def yielder
  yield
end

def non_local_return
  yielder do
    puts "ok"
    return
    puts "nop"
  end
  puts "should not get here"
end

non_local_return
# => ok
