#include "tinyrb.h"

static OBJ tr_kernel_puts(VM, OBJ self, OBJ txt)
{
  puts(TR_STR(txt));
  return TR_NIL;
}

static OBJ tr_kernel_raise(VM, OBJ self, OBJ msg)
{
  tr_raise(vm, TR_STR(msg));
  return TR_NIL;
}

void tr_kernel_init(VM)
{
  OBJ mod = tr_new(vm, tr_const_get(vm, "Module"));
  tr_const_set(vm, "Kernel", mod);
  
  tr_metadef(vm, mod, "puts", tr_kernel_puts, 1);
  tr_metadef(vm, mod, "raise", tr_kernel_raise, 1);
}