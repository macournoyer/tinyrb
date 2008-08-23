#include "tinyrb.h"

OBJ tr_proc_new(VM, OBJ ops)
{
  tr_proc *proc = (tr_proc *) tr_malloc(sizeof(tr_proc));
  
  tr_obj_init(vm, TR_PROC, (OBJ) proc, tr_const_get(vm, "Proc"));
  proc->ops   = ops;
  proc->cf    = vm->cf;
  
  return (OBJ) proc;
}

OBJ tr_proc_call(VM, OBJ self, int argc, OBJ argv[])
{
  tr_proc *proc = TR_CPROC(self);
  OBJ      ret;
  size_t   i;
  off_t    cf = vm->cf;
  
  vm->cf = proc->cf;
  
  for (i = 0; i < argc; ++i)
    tr_hash_set(vm, vm->frames[proc->cf].locals, tr_fixnum_new(vm, argc-i), argv[i]);
  
  ret = tr_run(vm, tr_string_new(vm, "?"), proc->ops);
  
  vm->cf = cf;
  
  return ret;
}

static OBJ tr_proc_new2(VM, OBJ class)
{
  return CUR_FRAME->block;
}

void tr_proc_init(VM)
{
  OBJ class = tr_class_new(vm, "Proc", tr_const_get(vm, "Object"));
  
  tr_metadef(vm, class, "new", tr_proc_new2, 0);
  tr_def(vm, class, "call", tr_proc_call, -1);
}

