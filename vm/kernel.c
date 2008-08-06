#include "tinyrb.h"

static OBJ tr_kernel_puts(OBJ txt)
{
  puts(TR_STR(txt));
  
  return TR_NIL;
}

OBJ tr_kernel(VM)
{
  OBJ mod = tr_new(tr_const_get(vm, "Module"));
  tr_const_set(vm, "Kernel", mod);
  
  tr_metadef(mod, "puts", tr_kernel_puts, 1);
  
  return mod;
}