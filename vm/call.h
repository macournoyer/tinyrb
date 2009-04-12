/* Inlined functions frequently used in the method calling process. */

inline void TrFrame_push(VM, OBJ self, OBJ class, TrClosure *closure) {
  register int cf = ++vm->cf;
  if (cf >= TR_MAX_FRAMES) tr_raise(SystemStackError, "Stack overflow");
  register TrFrame *f = FRAME;
  f->self = self;
  f->class = class;
  f->closure = closure;
}

inline void TrFrame_pop(VM) {
  /* TODO for GC: release everything on the stack */
  vm->cf--;
}

inline OBJ TrMethod_call(VM, OBJ self, OBJ receiver, int argc, OBJ *args, int splat, TrClosure *cl) {
  /* prepare call frame */
  TrFrame_push(vm, receiver, TR_CLASS(receiver), cl);
  TrFrame *f = FRAME;
  TrMethod *m = f->method = TR_CMETHOD(self);
  TrFunc *func = f->method->func;
  OBJ ret = TR_NIL;

  /* splat last arg is needed */
  if (unlikely(splat)) {
    OBJ splated = args[argc-1];
    int splatedn = TR_ARRAY_SIZE(splated);
    OBJ *new_args = TR_ALLOC_N(OBJ, argc);
    TR_MEMCPY_N(new_args, args, OBJ, argc-1);
    TR_MEMCPY_N(new_args + argc-1, &TR_ARRAY_AT(splated, 0), OBJ, splatedn);
    argc += splatedn-1;
    args = new_args;
  }
  
  if (m->arity == -1) {
    ret = func(vm, receiver, argc, args);
  } else {
    if (m->arity != argc) tr_raise(ArgumentError, "Expected %d arguments, got %d.", f->method->arity, argc);
    switch (argc) {
      case 0:  ret = func(vm, receiver); break;
      case 1:  ret = func(vm, receiver, args[0]); break;
      case 2:  ret = func(vm, receiver, args[0], args[1]); break;
      case 3:  ret = func(vm, receiver, args[0], args[1], args[2]); break;
      case 4:  ret = func(vm, receiver, args[0], args[1], args[2], args[3]); break;
      case 5:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4]); break;
      case 6:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5]); break;
      case 7:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6]); break;
      case 8:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]); break;
      case 9:  ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]); break;
      case 10: ret = func(vm, receiver, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]); break;
      default: tr_raise(ArgumentError, "Too much arguments: %d, max is %d for now.", argc, 10);
    }
  }
  TrFrame_pop(vm);
  return ret;
}
