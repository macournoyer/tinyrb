class Eval
  def smash(b)
    @who = 1
    eval("@who", b)
  end
end

puts Eval.new.instance_eval("self.class.name")
# => Eval

# Test binding
@who = "lucas"
puts Eval.new.smash(binding)
# => lucas
