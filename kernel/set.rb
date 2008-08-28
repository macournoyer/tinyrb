class Set
  include Enumerable
  
  def initialize(enum=nil)
    @hash = {}
    enum.each { |item| add(item) } if enum
  end
  
  def add(item)
    @hash[item] = true
  end
  alias :<< :add

  def delete(item)
    @hash[item] = nil
  end
  
  def include?(item)
    @hash[item]
  end
  
  def |(enum)
    dup.merge(enum)
  end
  alias :union :|
  alias :+ :|
  
  def -(enum)
    dup.subtract(enum)
  end
  
  def each
    # TODO @hash.keys.each(&block)
  end

  def merge(enum)
    if enum.is_a?(Set)
      @hash.update(enum.instance_variable_get(:@hash))
    else
      enum.each { |item| add(item) }
    end
    self
  end
  
  def subtract(enum)
    enum.each { |item| delete(item) }
    self
  end
  
  def size
    @hash.size
  end
  alias :length :size
  
  def clear
    @hash.clear
  end
end