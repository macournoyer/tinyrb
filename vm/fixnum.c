#include "tinyrb.h"

OBJ tr_fixnum_new(VM, int val)
{
  tr_fixnum *num = (tr_fixnum *) tr_malloc(sizeof(tr_fixnum));
  
  num->type = TR_FIXNUM;
  num->val  = val;
  
  return (OBJ) num;
}
