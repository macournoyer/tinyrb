puts Module
# => Module

module Awesome
  def yeah
    puts "you're hot"
  end
end

class You
  include Awesome
end

You.allocate.yeah
# => you're hot
