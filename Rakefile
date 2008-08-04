def rake(dir, task)
  cd(dir) { system "rake #{task}" }
end

desc "Compile tinyrb"
task :compile do
  rake "vm", :compile
end

desc "Remove all generated files"
task :clean do
  rake "vm", :clean
  rake "test/vm", :clean
end

desc "Run all tests (default)"
task :test do
  rake "test/vm", :test
end
task :default => :test

desc "Compute size of codebase"
task :size do
  puts "%0.2fK" % (Dir["vm/*.{c,rb,h}"].inject(0) { |sum, f| sum += File.size(f) } / 1024.0)
end