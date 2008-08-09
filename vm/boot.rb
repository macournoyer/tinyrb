# String stuff
a = "ohaie"[0,2]
a += "aie".to_s
a += 1.to_s
a += true.to_s

# Conditional
if a
  Kernel.puts "a is a " + a.class.name
  Kernel.puts "a = " + a
end

# stdio
Kernel.puts "prog name: " + ARGV.last
Kernel.puts "type a key:"
Kernel.puts STDIN.read(1)

# method
def method!(x); x end
Kernel.puts "method! returned: " + method!("var")

# class
class Poop
  def to_s
    "you smell"
  end
end
Kernel.puts Poop.new.to_s

# exception
Kernel.raise "This is not so exceptional"
