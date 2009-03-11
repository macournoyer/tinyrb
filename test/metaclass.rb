obj = Object.new

def obj.to_s
  "metawesome"
end

puts obj.to_s
# => metawesome

module Cookie
  def self.chocolat
    "yummy"
  end
end

puts Cookie.chocolat
# => yummy
