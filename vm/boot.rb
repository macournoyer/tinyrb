a = "ohaie"[0,2]
a += "aie".to_s
a += 1.to_s
if a
  Kernel.puts "a is a " + a.class.name
  Kernel.puts "a = " + a
end
Kernel.puts "prog name: " + ARGV.last
Kernel.puts "type a key:"
Kernel.puts STDIN.read(1)
def method!(x); x end
Kernel.puts "method! returned: " + method!("var")
Kernel.raise true.to_s
