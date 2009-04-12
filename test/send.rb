puts 1.send(:to_s)
# => 1

send :puts, 2, "cheezburgers"
# => 2
# => cheezburgers
