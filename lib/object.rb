class Object
  def respond_to?(message)
    !!method(message)
  end
end