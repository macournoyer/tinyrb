a = "oh"
a += "aie".to_s
a += 1.to_s
if a
  Kernel.puts a.class.name
  Kernel.puts a
end
Kernel.raise a
