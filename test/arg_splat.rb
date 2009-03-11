puts *[1, 2]
# => 1
# => 2

def anything(*args)
  puts *args
end

anything 1, 2
# => 1
# => 2

class A
  def initialize(x)
    puts x
  end
end

A.new("yarly")
# => yarly