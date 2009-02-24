class Eval
  def smash(b)
    @who = 1
    eval("@who", b)
  end
end

puts Eval.allocate.instance_eval("self.class.name")
# => Eval

# Test binding
@who = "lucas"
puts Eval.allocate.smash(binding)
# => lucas
