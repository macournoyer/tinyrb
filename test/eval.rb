class Eval
end

puts Eval.allocate.instance_eval("self.class.name")
# => Eval
