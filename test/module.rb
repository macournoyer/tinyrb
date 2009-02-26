puts Module
# => Module

module Awesome
  def yeah
    puts "you're hot"
  end
  alias_method :nice, :yeah
end

class You
  include Awesome
end

y = You.allocate
y.yeah
# => you're hot

y.nice
# => you're hot
