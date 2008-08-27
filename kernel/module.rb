module Module
  # TODO not working yet...
  def attr_reader(name)
    define_method(name) { instance_variable_get("@" + name) }
  end
  def attr_writer(name)
    define_method(name + "=") { |value| instance_variable_set("@" + ivar, value) }
  end
  def attr_accessor(name)
    attr_reader(name)
    attr_writer(name)
  end
end