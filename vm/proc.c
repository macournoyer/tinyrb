#include "tinyrb.h"

OBJ tr_proc_new(VM, OBJ ops)
{
  tr_proc *proc = (tr_proc *) tr_malloc(sizeof(tr_proc));
  
  tr_obj_init(vm, TR_PROC, (OBJ) proc, tr_const_get(vm, "Proc"));
  proc->cf       = vm->cf;
  proc->ops      = TR_OPS(ops, CODE);
  proc->filename = TR_OPS(ops, FILENAME);
  proc->argc     = TR_FIX(TR_OPS(ops, ARGC));
  proc->localc   = TR_FIX(TR_OPS(ops, LOCALC));
  proc->labels   = TR_OPS(ops, LABELS);
  
  return (OBJ) proc;
}

OBJ tr_proc_call(VM, OBJ self, int argc, OBJ argv[])
{
  tr_proc *proc = TR_CPROC(self);
  tr_frame *f = &vm->frames[proc->cf];
  OBJ      ret;
  size_t   i;
  
  tr_next_frame(vm, f->self, f->class);

  for (i = 0; i < argc; ++i)
    tr_hash_set(vm, CUR_FRAME->locals, tr_fixnum_new(vm, argc-i), argv[i]);
  
  ret = tr_run(vm, proc->filename, proc->ops);
  
  tr_prev_frame(vm);
  
  return ret;
}

static OBJ tr_proc_current(VM, OBJ class)
{
  return CUR_FRAME->block;
}

void tr_proc_init(VM)
{
  OBJ class = tr_class_new(vm, "Proc", tr_const_get(vm, "Object"));
  
  tr_metadef(vm, class, "new", tr_proc_current, 0);
  tr_def(vm, class, "call", tr_proc_call, -1);
}

