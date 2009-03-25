$enum = [1,2,3]

def non_local_return
  $enum.each do |i|
    puts i
    return
  end
  raise "should not get here"
end

non_local_return
# => 1

def non_local_break
  $enum.each do |i|
    puts i
    break
  end
  puts "="
end

non_local_break
# => 1
# => =
