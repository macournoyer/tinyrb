#include "tinyrb.h"

#define FIX_APPLY_OP(op, n1, n2) tr_fixnum_new(vm, TR_FIX(n1) op TR_FIX(n2))
#define FIX_APPLY_UOP(op, n1)    tr_fixnum_new(vm, op TR_FIX(n1))
#define FIX_CMP_OP(op, n1, n2)   TR_CBOOL(TR_FIX(n1) op TR_FIX(n2))

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

static OBJ tr_fixnum_add(VM, OBJ self, OBJ other)   { return FIX_APPLY_OP(+, self, other); }
static OBJ tr_fixnum_sub(VM, OBJ self, OBJ other)   { return FIX_APPLY_OP(-, self, other); }
static OBJ tr_fixnum_eq(VM, OBJ self, OBJ other)    { return FIX_CMP_OP(==, self, other); }
static OBJ tr_fixnum_lt(VM, OBJ self, OBJ other)    { return FIX_CMP_OP(<, self, other); }
static OBJ tr_fixnum_gt(VM, OBJ self, OBJ other)    { return FIX_CMP_OP(>, self, other); }
static OBJ tr_fixnum_bwand(VM, OBJ self, OBJ other) { return FIX_APPLY_OP(&, self, other); }
static OBJ tr_fixnum_bwor(VM, OBJ self, OBJ other)  { return FIX_APPLY_OP(|, self, other); }
static OBJ tr_fixnum_bwnot(VM, OBJ self)            { return FIX_APPLY_UOP(~, self); }
static OBJ tr_fixnum_lbs(VM, OBJ self, OBJ other)   { return FIX_APPLY_OP(<<, self, other); }
static OBJ tr_fixnum_rbs(VM, OBJ self, OBJ other)   { return FIX_APPLY_OP(>>, self, other); }

void tr_fixnum_init(VM)
{
  OBJ class = tr_class_new(vm, "Fixnum", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "to_s", tr_fixnum_to_s, 0);
  tr_def(vm, class, "+", tr_fixnum_add, 1);
  tr_def(vm, class, "-", tr_fixnum_sub, 1);
  tr_def(vm, class, "==", tr_fixnum_eq, 1);
  tr_def(vm, class, "<", tr_fixnum_lt, 1);
  tr_def(vm, class, ">", tr_fixnum_gt, 1);
  tr_def(vm, class, "&", tr_fixnum_bwand, 1);
  tr_def(vm, class, "|", tr_fixnum_bwor, 1);
  tr_def(vm, class, "~", tr_fixnum_bwnot, 0);
  tr_def(vm, class, "<<", tr_fixnum_lbs, 1);
  tr_def(vm, class, ">>", tr_fixnum_rbs, 1);
}

