#include "tr.h"

/* Error management stuff */

/* Exception
 NoMemoryError
 ScriptError
   LoadError
   NotImplementedError
   SyntaxError
 SignalException
   Interrupt
 StandardError
   ArgumentError
   IOError
     EOFError
   IndexError
   LocalJumpError
   NameError
     NoMethodError
   RangeError
     FloatDomainError
   RegexpError
   RuntimeError
   SecurityError
   SystemCallError
   SystemStackError
   ThreadError
   TypeError
   ZeroDivisionError
 SystemExit
 fatal */

OBJ TrException_new(VM, OBJ class, OBJ message) {
  OBJ e = TrObject_alloc(vm, class);
  tr_setivar(e, "@message", message);
  tr_setivar(e, "@backtrace", TR_NIL);
  return (OBJ)e;
}

static OBJ TrException_cexception(VM, OBJ self, int argc, OBJ argv[]) {
  if (argc == 0) return TrException_new(vm, self, TR_CCLASS(self)->name);
  return TrException_new(vm, self, argv[0]);
}

static OBJ TrException_iexception(VM, OBJ self, int argc, OBJ argv[]) {
  if (argc == 0) return self;
  return TrException_new(vm, TR_CLASS(self), argv[0]);
}

static OBJ TrException_message(VM, OBJ self) {
  return tr_getivar(self, "@message");
}

OBJ TrException_backtrace(VM, OBJ self) {
  return tr_getivar(self, "@backtrace");
}

OBJ TrException_set_backtrace(VM, OBJ self, OBJ backtrace) {
  return tr_setivar(self, "@backtrace", backtrace);
}

OBJ TrException_default_handler(VM, OBJ exception) {
  TrClass *c = TR_CCLASS(TR_CLASS(exception));
  OBJ msg = tr_getivar(exception, "@message");
  OBJ backtrace = tr_getivar(exception, "@backtrace");
  
  printf("%s: %s\n", TR_STR_PTR(c->name), TR_STR_PTR(msg));
  if (backtrace) {
    TR_ARRAY_EACH(backtrace, i, v, {
      printf("%s\n", TR_STR_PTR(v));
    });
  }
  
  TrVM_destroy(vm);
  exit(1);
}

void TrError_init(VM) {
  OBJ c = vm->cException = tr_defclass("Exception", 0);
  tr_metadef(c, "exception", TrException_cexception, -1);
  tr_def(c, "exception", TrException_iexception, -1);
  tr_def(c, "backtrace", TrException_backtrace, 0);
  tr_def(c, "message", TrException_message, 0);
  tr_def(c, "to_s", TrException_message, 0);
  
  vm->cScriptError = tr_defclass("ScriptError", vm->cException);
  vm->cSyntaxError = tr_defclass("SyntaxError", vm->cScriptError);
  vm->cStandardError = tr_defclass("StandardError", vm->cException);
  vm->cArgumentError = tr_defclass("ArgumentError", vm->cStandardError);
  vm->cRegexpError = tr_defclass("RegexpError", vm->cStandardError);
  vm->cRuntimeError = tr_defclass("RuntimeError", vm->cStandardError);
  vm->cTypeError = tr_defclass("TypeError", vm->cStandardError);
  vm->cSystemCallError = tr_defclass("SystemCallError", vm->cStandardError);
  vm->cIndexError = tr_defclass("IndexError", vm->cStandardError);
  vm->cLocalJumpError = tr_defclass("LocalJumpError", vm->cStandardError);
  vm->cSystemStackError = tr_defclass("SystemStackError", vm->cStandardError);
  vm->cNameError = tr_defclass("NameError", vm->cStandardError);
  vm->cNoMethodError = tr_defclass("NoMethodError", vm->cNameError);
}
