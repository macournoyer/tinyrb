def rake(dir, task)
  cd(dir) { system "rake #{task}" }
end

desc "Compile tinyrb"
task :compile do
  rake "kernel", :compile
  rake "vm", :compile
end

desc "Remove all generated files"
task :clean do
  rake "kernel", :clean
  rake "vm", :clean
  rake "test/vm", :clean
end

desc "Run all tests (default)"
task :test do
  rake "test/vm", :test
end

task :default => [:compile, :test]

def compute_size(dir)
  "%0.2fK" % (Dir["{#{dir}}/**.{c,rb,h}"].inject(0) { |sum, f| sum += File.size(f) } / 1024.0)
end
desc "Compute size of codebase"
task :size do
  puts "kernel: " + compute_size("kernel")
  puts "vm:     " + compute_size("vm")
  puts "parser: " + compute_size("parser")
  puts "total:  " + compute_size("kernel,vm,parser")
end