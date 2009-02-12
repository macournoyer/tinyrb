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

puts String.name
# => String

puts Object.name
# => Object

puts Class.name
# => Class

puts Symbol.name
# => Symbol
