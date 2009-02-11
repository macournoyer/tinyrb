class Pony
  def talk
    puts "lol"
    puts self.class.name
  end
end

puts Pony.name
# => Pony

Pony.allocate.talk
# => lol
# => Pony
