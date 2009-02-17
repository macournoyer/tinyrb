class Pony
  def set_cname(name)
    @@name = name
  end
  def set_name(name)
    @name = name
  end
  def talk
    puts @name
    puts @@name
    puts self.class.name
  end
end

puts Pony
# => Pony

Pony.allocate.set_cname("Bob")
p = Pony.allocate
p.set_name "Rick"
p.talk
# => Rick
# => Bob
# => Pony

puts String
# => String

puts Object
# => Object

puts Class
# => Class

puts Symbol
# => Symbol

# Reopening
class String
  def yodelay
    puts "yhoo!"
  end
end

"sing".yodelay
# => yhoo!
