i=0
str = 'xxxhogexxx'
while i<6000000 # benchmark loop 2
  # /hoge/ =~ str
  /hoge/.match str
  i = i + 1
end
