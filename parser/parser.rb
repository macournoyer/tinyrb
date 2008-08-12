# grammar interpreter written in Ruby
# by Markus Liedl
#
# todo:
#   optimize


require 'pp'
require 'set'

$: << ".";
require 'sexp-reader'

class Array
  def each_pairwise()
    i = 0;
    while i < size()
      yield(self[i], self[i+1])
      i += 2;
    end
    self;
  end
end

# need a nil that's not `false'
class <<(Pnil=Object.new)
  def ==(other)
    self.object_id == other.object_id \
      or other == :nil
  end
  def pretty_print(pp)
    pp.text("Pnil");
  end
  def to_s() "Pnil"; end
end

class Cons # SinglyLinkedListCell
  def self.from_array(ary, pos=0)
    if ary.length > pos
      Cons.new(ary[pos], from_array(ary, pos+1));
    else
      Pnil
    end
  end

  attr_accessor :first, :rest
  def initialize(first, rest)
    @first, @rest = first, rest
  end

  def ==(other)
    if Cons === other
      @first == other.first() \
      and @rest == other.rest();
    elsif Array === other
      s = self;
      pos = 0;
      loop do
        if s == Pnil
          return pos == other.length()
        elsif s.first() != other[pos]
          return false;
        end
        s = s.rest();
        pos += 1;
      end
      return true;
    else
      false;
    end
  end

  def pretty_print(pp)
    pp.group 2, "#<List", ">" do
      s = self;
      while s != Pnil
        pp.breakable();
        pp.pp(s.first());
        s = s.rest();
      end
    end
  end
end


class Grammar
  def self.from_string(gram, param)
    new(SExpReader.read_string(gram), param);
  end

  def self.from_file(gramfile, param)
    new(SExpReader.read_file_all(gramfile), param);
  end

  def initialize(rules, param)

    @keep_locations = param[:keep_locations];
    @trace = param[:trace];

    @modes = Hash.new();
    @rules = Hash.new();

    # first create and insert the Rule instances
    rules.each_pairwise do |head, body|
      add_rule(head, body)
    end

    # then, resolve the rules (afterwards, there can be arbitrary
    # references between the rules)
    for rule in @rules.values() do
      rule.resolve(self);
    end

    @entry_rule = lookup_rule(rules[0]);
  end

  def parse(input0)
    input = input0 + " ";  # important!
    context = Context.new(input);
    res = @entry_rule.call_without_arguments(context);
    context.at_end() and res;
  end

  def add_rule(name_args, body)
    name, args = nil, nil;
    if Symbol === name_args
      name = name_args;
    else
      name, *args = name_args;
    end
    @rules[name] = Rule.make(name, args, body);
  end

  def lookup_rule(name)
    @rules[name] or fail "unknown rule: #{name.inspect()}"
  end

  def resolve_form(form)
    if String === form
      StringParser.new(form)
    elsif Integer === form
      getter = {1=>:first, 2=>:second, 3=>:third}[form];
      GetCaptureVariable.new(getter);
    elsif Symbol === form
      if form == :"@"
        GetCaptureVariable.new(:retvar);
      else
        CallWithoutArguments.new(lookup_rule(form), @trace);
      end
    elsif Array === form
      rest = form[1..-1];
      case form[0]
      when :range
        RangeParser.new(*rest);
      when :one_of
        OneOf.new(rest[0]);

      when :seq
        Sequence.implicite(resolve_all(rest))
      when :or
        Alternatives.new(split_resolve_rest(0, rest))
      when :rep
        Repetition.new(0, split_resolve_rest(0, rest))
      when :rep1
        Repetition.new(1, split_resolve_rest(0, rest))
      when :opt
        Optional.new(split_resolve_rest(0, rest))
      when :until
        Until.new(*split_resolve_rest(0, rest))

      when :"@"
        AssignCaptureVariable.new(:retvar=, resolve_form(form[1]));
      when Integer
        setter = {1=>:first=, 2=>:second=, 3=>:third=}[form[0]];
        AssignCaptureVariable.new(setter, resolve_form(form[1]));
      when :make
        Make.new(*split_resolve_rest(1, rest));

      when :string
        ConstructString.new(resolve_all(rest))
      when :yield
        YieldStringPart.new(resolve_form(rest[0]));

      when :new_mark_scope
        NewMarkScope.new(split_resolve_rest(0, rest));
      when :add_mark_scope
        AddMarkScope.new(split_resolve_rest(0, rest));
      when :mark
        MarkWord.new(resolve_form(rest[0]));
      when :is_marked
        IsMarked.new(resolve_form(rest[0]));

      when :error
        PError.new(resolve_form(rest[0]), rest[1]);

      when :trace    # todo: ignored
        resolve_form(rest[0]);

      when :except
        Except.new(resolve_all(rest));

      when :enter_mode
        mode = rest[0];
        EnterMode.new(mode, encode_mode(mode), resolve_form(rest[1]));
      when :leave_mode
        mode = rest[0];
        LeaveMode.new(mode, encode_mode(mode), resolve_form(rest[1]));
      when :not_in_mode
        mode = rest[0];
        NotInMode.new(mode, encode_mode(mode), resolve_form(rest[1]));
      when :only_in_mode
        mode = rest[0];
        OnlyInMode.new(mode, encode_mode(mode), resolve_form(rest[1]));

      when :call
        rule = lookup_rule(rest[0]);
        args = rest[1..-1];
        if args
          CallWithArguments.new(rule, resolve_all(args), @trace);
        else
          CallWithoutArguments.new(rule, @trace);
        end
      when :arg
        GetArg.new(rest[0]);
        
      when :location
        sub = resolve_form(rest[0]);
        @keep_locations ? Location.new(sub) : sub;

      when :postpone_rest_of_line
        PostponeRestOfLine.new(resolve_form(rest[0]));

      when :again
        Again.new(resolve_form(rest[0]));

      when :return
        Return.new(rest[0]);

      else
        fail "form unknown: #{form.inspect}";
      end
    else
      fail "form unknown: #{form.inspect}";
    end
  end

  def split_resolve_rest(count, forms)
    forms[0, count] + resolve_all(forms[count..-1])
  end

  def resolve_all(forms)
    forms.map { |form| resolve_form(form) }
  end

  def encode_mode(modename)
    bit = @modes[modename] ||= @modes.size();
    1 << bit;
  end
end

class Context
  attr_accessor :position, :marks, :modes, :consumed_region

  def initialize(input)
    @input = input;
    @position = 0;
    @rule_activations = Array.new();
    @constructed_strings = Array.new();
    @marks = Set.new();
    @modes = 0;
    @consumed_region = -1 .. -1; # nothing is consumed yet
    @trace_indent = 0;
    @cache = Hash.new();
  end

  def advance(count)
    @position += count;
    correct_position();
  end

  def at_end()
    @position+1 >= @input.length();
  end

  def current_char()
    @input[@position]
  end

  def char_at_offset(offset)
    @input[@position+offset]
  end

  def consume_string(string)
    # todo: no better way?
    0.upto(string.length-1) do |offset|
      if char_at_offset(offset) != string[offset]
        return false;
      end
    end
    # puts "#{@position} accepted #{string}";
    advance(string.length());
    true;
  end

  def save()
    [@position,
     marks_cloned(),  # todo: maybe too expensive
     @consumed_region ] 
  end

  def revert_to(state)
    @position, @marks, @consumed_region = *state;
  end

  def current_rule_activation()
    @rule_activations.last();
  end

  def push_rule_activation(act)
    @rule_activations.push(act);
  end

  def pop_rule_activation()
    @rule_activations.pop();
  end

  def push_string_construction()
    @constructed_strings.push(Array.new());
  end

  def pop_constructed_string()
    @constructed_strings.pop();
  end

  def yield_to_constructed_string(part)
    @constructed_strings.last().push(part)
  end

  def marks_cloned()
    @marks.clone();
  end
  
  def reset_marks()
    @marks = Set.new();
  end
  
  def mark(word)
    @marks.add(word);
  end

  def marked?(word)
    @marks.include?(word);
  end

  def mark_region_as_consumed(from, to)
    @consumed_region =
      if @consumed_region === from
        @consumed_region.begin() .. to
      else
        from .. to;
      end
  end

  def correct_position()
    if @consumed_region === @position
      @position = @consumed_region.end;
    end
  end

  def skip_to_beginning_of_next_line()
    nl = @input.index("\n", @position) \
         || @input.index("\r", @position);
    if nl
      @position = nl+1;
      correct_position();
    else
      fail 'there is no next line.';
    end
  end

  def trace_indent()
    0.upto(@trace_indent) do
      print("  ");
    end
  end

  def trace_enter(rule)
    trace_indent();
    @trace_indent += 1;
    puts "#{@position}  enter #{rule.name}";
  end

  def trace_leave(rule, res)
    @trace_indent -= 1;
    trace_indent();
    puts "#{@position}  leave #{rule.name}  -- #{res.inspect()}";
  end


  # memoization

  def cache_put(start, startmarks, name, res)
    key = [start, name, @modes]
    @cache[key] = [@position, res, @marks - startmarks, @consumed_region];
  end

  def cache_lookup(name)
    key = [@position, name, @modes];
    if @cache.include?(key)
      @position, res, added_marks, @consumed_region = *@cache[key];
      @marks += added_marks;
      return true, res;
    else
      return false, nil
    end
  end


end

class Rule
  def self.make(name, args, body)
    if not args
      RuleWithoutArguments.new(name, body);
    else
      RuleWithArguments.new(name, args, body);
    end
  end

  attr_reader :name

  def resolve(grammar)
    @body = grammar.resolve_form(@body);
    @use_retvar = @body.uses_retvar();
    @uses_any_capture_variable = @body.uses_any_capture_variable();
  end

  def rule_return_value(res, act)
    if @use_retvar
      res and act.retvar();
    else
      res
    end
  end
end

class RuleActivation
  # the capture variables
  attr_accessor :retvar, :first, :second, :third
end

class RuleWithoutArguments < Rule
  def initialize(name, body)
    @name, @body = name, body
  end
  def call_without_arguments(context)
    success, res = context.cache_lookup(@name);
    if success
      res
    else
      start = context.position();
      startmarks = context.marks_cloned();
      if @uses_any_capture_variable
        context.push_rule_activation(Act.new());
        xres = @body.parse(context)
        act = context.pop_rule_activation();
        res = rule_return_value(xres, act);
      else
        res = @body.parse(context)
      end
      context.cache_put(start, startmarks, @name, res);
      res;
    end
  end

  def call_with_arguments(context, args)
    fail "the rule #{@name} accepts no arguments!"
  end

  class Act < RuleActivation
  end
end

class RuleWithArguments < Rule
  def initialize(name, args, body)
    @name, @args, @body = name, args, body;
    @arg_pos = Hash.new();
    @args.each_index {|i| @arg_pos[@args[i]] = i }
  end

  def call_without_arguments(context)
    fail "the rule #{@name} wants #{@args.length} arguments!"
  end

  def call_with_arguments(context, args)
    if args.length != @args.length
      fail "the rule #{@name} expected #{@args.length} arguments "\
           "but caller gave #{args.length}!"
    end
    context.push_rule_activation(Act.new(@arg_pos, args));
    xres = @body.parse(context)
    act = context.pop_rule_activation();
    rule_return_value(xres, act);
  end

  class Act < RuleActivation
    attr_reader :args
    def initialize(arg_pos, args)
      @arg_pos, @args = arg_pos, args;
    end
    def arg(argname)
      pos = @arg_pos[argname] or fail "unknown argument #{argname}.";
      @args[pos];
    end
  end
end



module HasSub
  def uses_retvar() @sub.uses_retvar() end
  def uses_any_capture_variable() @sub.uses_any_capture_variable() end
end

module NoVariables
  def uses_retvar() false end
  def uses_any_capture_variable() false end
end

class StringParser
  include NoVariables

  def initialize(string)
    @string = string;
  end

  def parse(context)
    context.consume_string(@string) \
      and @string;
  end
end

class RangeParser
  include NoVariables

  def initialize(from, to)
    @range = from[0] .. to[0]
  end

  def parse(context)
    if @range.include?(res = context.current_char())
      context.advance(1)
      res.chr();
    else
      false
    end
  end
end

class OneOf
  include NoVariables

  def initialize(string)
    @chars = Hash.new();
    0.upto(string.length-1) do |i|
      @chars[string[i]] = true;
    end
  end

  def parse(context)
    if @chars[res = context.current_char()]
      context.advance(1)
      res.chr();
    else
      false;
    end
  end
end


class Sequence
  def self.implicite(args)
    if args.length == 1
      args[0]
    else
      new(args)
    end
  end

  def initialize(sub)
    @sub = sub;
  end

  def uses_retvar() @sub.any? { |s| s.uses_retvar } end
  def uses_any_capture_variable()
    @sub.any? { |s| s.uses_any_capture_variable }
  end

  def parse(context)
    start = context.save();
    last = Pnil;
    for sub in @sub do
      if not (res = sub.parse(context))
        context.revert_to(start);
        return false;
      end
      last = res;
    end
    last;
  end
end

class Alternatives
  def initialize(alternatives)
    @alternatives = alternatives;
  end

  def uses_retvar() @alternatives.any? { |alt| alt.uses_retvar() } end
  def uses_any_capture_variable()
    @alternatives.any? { |alt| alt.uses_any_capture_variable() }
  end

  def parse(context)
    for alt in @alternatives
      if res = alt.parse(context)
        return res;
      end
    end
    false;
  end
end



class Repetition
  include HasSub

  def initialize(mincount, sub)
    @mincount, @sub = mincount, Sequence.implicite(sub)
  end

  def parse(context)
    count = 0;
    while @sub.parse(context)
      count += 1;
    end
    count >= @mincount ? Pnil : false;
  end
end


class Optional
  include HasSub
  def initialize(sub)
    @sub = Sequence.implicite(sub);
  end
  
  def parse(context)
    @sub.parse(context) or Pnil
  end
end

class Until
  include HasSub
  def initialize(cond, body)
    @cond, @body = cond, body
  end

  def uses_retvar() @cond.uses_retvar() or @body.uses_retvar(); end
  def uses_any_capture_variable()
    @cond.uses_any_capture_variable() \
      or @body.uses_any_capture_variable();
  end

  def parse(context)
    until @cond.parse(context)
      return false unless @body.parse(context);
    end
    Pnil;
  end
end




class AssignCaptureVariable
  def initialize(setter, sub)
    @setter, @sub = setter, sub
  end

  def uses_retvar() @setter == :retvar=; end
  def uses_any_capture_variable() true; end

  def parse(context)
    if res = @sub.parse(context)
      context.current_rule_activation().send(@setter, res);
      res
    else
      false
    end
  end
end

class GetCaptureVariable
  def initialize(getter)
    @getter = getter;
  end

  def uses_retvar() @getter == :retvar=; end
  def uses_any_capture_variable() true; end

  def parse(context)
    context.current_rule_activation().send(@getter);
  end
end

def parse_all_and_keep(sub, context)
  res = Array.new(sub.length());
  i = 0;
  start = context.save();
  for s in sub do
    unless res[i] = s.parse(context)
      context.revert_to(start);
      return false;
    end
    i += 1;
  end
  res;
end

class Make
  def initialize(name, *sub)
    @name, @sub = name, sub;
  end
  
  def uses_retvar() @sub.any? { |s| s.uses_retvar() } end
  def uses_any_capture_variable()
    @sub.any? { |s| s.uses_any_capture_variable() }
  end

  def parse(context)
    res = parse_all_and_keep(@sub, context);
    if res
      if @name == :cons
        Cons.new(*res);  # must be two arguments
      elsif @name == :nil
        Pnil
      else
        # todo: use factory
        [@name, *res];
      end
    end
  end
end

class CallWithoutArguments
  def initialize(rule, trace)
    @rule, @trace = rule, trace;
  end

  def uses_retvar() false end
  def uses_any_capture_variable() false end


  def parse(context)
    context.trace_enter(@rule) if @trace;
    res = @rule.call_without_arguments(context);
    context.trace_leave(@rule, res) if @trace;
    res;
  end
end

class GetArg
  include NoVariables
  def initialize(argname)
    @argname = argname;
  end
  def parse(context)
    context.current_rule_activation.arg(@argname);
  end
end


class CallWithArguments
  def initialize(rule, args, trace)
    @rule, @args, @trace = rule, args, trace;
  end

  def uses_retvar() @args.any? {|a| a.uses_retvar() } end
  def uses_any_capture_variable()
    @args.any? {|a| a.uses_any_capture_variable() }
  end

  def parse(context)
    args = parse_all_and_keep(@args, context);
    if args
      context.trace_enter(@rule) if @trace;
      res = @rule.call_with_arguments(context, args);
      context.trace_leave(@rule, res) if @trace;
    else
      res = false;
    end
    res;
  end
end


class ConstructString
  include HasSub
  def initialize(sub)
    @sub = Sequence.implicite(sub);
  end
  def parse(context)
    context.push_string_construction();
    res = @sub.parse(context);
    string_parts = context.pop_constructed_string();
    res and string_parts.join("");
  end
end

class YieldStringPart
  include HasSub

  def initialize(sub)
    @sub = sub;
  end
  def parse(context)
    if res = @sub.parse(context)
      context.yield_to_constructed_string(res);
      res;
    else
      false;
    end
  end
end

class NewMarkScope
  include HasSub

  def initialize(sub)
    @sub = Sequence.implicite(sub)
  end

  def parse(context)
    outer_marks = context.marks_cloned();
    context.reset_marks();
    res = @sub.parse(context);
    context.marks = outer_marks;
    res;
  end
end

class AddMarkScope
  include HasSub

  def initialize(sub)
    @sub = Sequence.implicite(sub)
  end

  def parse(context)
    outer_marks = context.marks_cloned();
    res = @sub.parse(context);
    context.marks = outer_marks;
    res;
  end
end


class MarkWord
  include HasSub
  def initialize(sub)
    @sub = sub;
  end
  def parse(context)
    res = @sub.parse(context);
    if res
      context.mark(res)
    end
    res
  end
end

class IsMarked
  include HasSub
  def initialize(sub)
    @sub = sub;
  end
  def parse(context)
    res = @sub.parse(context);
    if res
      if context.marked?(res)
        res
      else
        false
      end
    else
      false
    end
  end
end

class ParserError < Exception
  def initialize(msg)
    @msg = msg;
  end
end

class PError
  include HasSub

  def initialize(sub, msg)
    @sub, @msg = sub, msg;
  end

  def parse(context)
    if @sub.parse(context)
      raise ParserError.new(@msg);
    end
    Pnil
  end
end


class Except
  include HasSub

  def initialize(sub)
    @sub = Sequence.implicite(sub);
  end

  def parse(context)
    start = context.save();
    res = if @sub.parse(context)
            false;
          else
            Pnil;
          end
    context.revert_to(start);
    res;
  end
end


class EnterMode
  include HasSub
  def initialize(modename, bitmask, sub)
    @modename, @bitmask, @sub = modename, bitmask, sub;
  end
  def parse(context)
    outer_modes = context.modes();
    context.modes |= @bitmask;
    res = @sub.parse(context);
    context.modes = outer_modes;
    res;
  end
end

class LeaveMode
  include HasSub
  def initialize(modename, bitmask, sub)
    @modename, @bitmask, @sub = modename, bitmask, sub;
  end
  def parse(context)
    outer_modes = context.modes();
    context.modes &= ~@bitmask;
    res = @sub.parse(context);
    context.modes = outer_modes;
    res;
  end
end


class NotInMode
  include HasSub
  def initialize(modename, bitmask, sub)
    @modename, @bitmask, @sub = modename, bitmask, sub;
  end
  def parse(context)
    if context.modes & @bitmask == 0
      @sub.parse(context);
    else
      false
    end
  end
end


class OnlyInMode
  include HasSub
  def initialize(modename, bitmask, sub)
    @modename, @bitmask, @sub = modename, bitmask, sub;
  end
  def parse(context)
    if context.modes & @bitmask != 0
      @sub.parse(context);
    else
      false
    end
  end
end


class Location
  include HasSub
  def initialize(sub)
    @sub = sub;
  end
  def parse(context)
    start = context.position;
    res = @sub.parse(context);
    res and [:loc, start, context.position, res]
  end
end

class PostponeRestOfLine
  include HasSub
  def initialize(sub)
    @sub = sub;
  end
  def parse(context)
    start = context.position;
    context.skip_to_beginning_of_next_line();
    beginning_of_next_line = context.position;
    if res = @sub.parse(context);
      xend = context.position;
      context.mark_region_as_consumed(beginning_of_next_line, xend);
      context.position = start;
      res
    else
      false
    end
  end
end

class Again
  include HasSub
  def initialize(sub)
    @sub = sub;
  end
  def parse(context)
    string = @sub.parse(context)
    if String === string
      context.consume_string(string) \
        and string
    else
      fail 'the "again" form expected to get a String.';
    end
  end
end

class Return
  include NoVariables
  def initialize(value)
    @value = value;
  end
  def parse(context)
    @value
  end
end

if __FILE__ == $PROGRAM_NAME
  pp Grammar.from_file('gram.lisp', {}).parse(<<-EOS)
    Kernel.puts "ohaie"
  EOS
end