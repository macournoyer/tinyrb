# Truish

class Object
  def !
    false
  end
end

class TrueClass
end

# Falsy

class NilClass
  def !
    true
  end
end

class FalseClass
  def !
    true
  end
end