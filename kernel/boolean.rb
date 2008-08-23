class TrueClass
  def !
    false
  end
  def inspect
    "true"
  end
end

class FalseClass
  def !
    true
  end
  def inspect
    "false"
  end
end

class NilClass
  def !
    true
  end
  def inspect
    "nil"
  end
end