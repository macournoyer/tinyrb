class Pony
  def cname=(name)
    @@name = name
  end
  def name=(name)
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

Pony.new.cname = "Bob"
p = Pony.new
p.name = "Rick"
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

# Subclassing
class Unicorn < Pony; end
Unicorn.new.talk
# => 
# => 
# => Unicorn
