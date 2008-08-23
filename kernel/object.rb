class Object
  def ==(other)
    object_id == other.object_id
  end
  alias :=== :==
  alias :equal? :==
  alias :eql? :==
  
  def <(other)
    object_id < other.object_id
  end

  def >(other)
    object_id > other.object_id
  end
end