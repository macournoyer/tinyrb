require "parser"

parser = RDParser.new do
   token(/\s+/)
   token(/\d+/) {|m| m.to_i }
   token(/./) {|m| m }

   start :expr do
     match(:expr, '+', :term) {|a, _, b| a + b }
     match(:expr, '-', :term) {|a, _, b| a - b }
     match(:term)
   end

   rule :term do
     match(:term, '*', :dice) {|a, _, b| a * b }
     match(:term, '/', :dice) {|a, _, b| a / b }
     match(:dice)
   end

   def roll(times, sides)
     (1..times).inject(0) {|a, b| a + rand(sides) + 1 }
   end

   rule :dice do
     match(:atom, 'd', :sides) {|a, _, b| roll(a, b) }
     match('d', :sides) {|_, b| roll(1, b) }
     match(:atom)
   end

   rule :sides do
     match('%') { 100 }
     match(:atom)
   end

   rule :atom do
     match(Integer)
     match('(', :expr, ')') {|_, a, _| a }
   end
end

puts parser.parse("1 d 2")