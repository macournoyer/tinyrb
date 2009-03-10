# new is method of Object's metaclass

puts Object.method(:new).object_id != Class.method(:new).object_id
# => true

obj = Object.new

puts obj.class.name
# => Object