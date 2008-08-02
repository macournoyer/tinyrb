def rake(dir, task)
  cd(dir) { system "rake #{task}" }
end

task :compile do
  rake "vm", :compile
end

task :clean do
  rake "vm", :clean
  rake "test/vm", :clean
end

task :test do
  rake "test/vm", :test
end