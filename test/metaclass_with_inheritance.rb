class A
  def self.new
    puts "A.new"
  end
end

class B < A; end
B.new
# => A.new
