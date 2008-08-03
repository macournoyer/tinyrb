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