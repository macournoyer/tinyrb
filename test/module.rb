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

y = You.new
y.yeah
# => you're hot

y.nice
# => you're hot
