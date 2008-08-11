class Emitter
  def initialize
    @instructions = []
    @line = 1
  end
  
  def next_line(l)
    @line += 1
  end
  
  def op(code, *cmds)
    @instructions << [@line, code.to_s] + cmds
  end
  
  def run
    VM.run @instructions
  end
end
