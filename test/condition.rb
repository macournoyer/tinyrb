if true
  :true.display
end
# => true

if false
  :nope.display
end
# =>

if nil
  :nope.display
end
# =>

unless nil
  :yay.display
end
# => yay
