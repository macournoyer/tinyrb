#include "tinyrb.h"

OBJ tr_fixnum_new(VM, int val)
{
  tr_fixnum *num = (tr_fixnum *) tr_malloc(sizeof(tr_fixnum));
  
  tr_obj_init(vm, TR_FIXNUM, (OBJ) num, tr_const_get(vm, "Fixnum"));
  num->val  = val;
  
  return (OBJ) num;
}

static OBJ tr_fixnum_to_s(VM, OBJ self)
{
  OBJ str = tr_string_new(vm, "2147483647"); /* HACK should compute len of int instead */
  sprintf(TR_STR(str), "%d", TR_FIX(self));
  return str;
}

void tr_fixnum_init(VM)
{
  OBJ class = tr_class_new(vm, "Fixnum", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "to_s", tr_fixnum_to_s, 0);
}

