#include "tinyrb.h"

OBJ tr_range_new(VM, OBJ first, OBJ last)
{
  tr_range *range = (tr_range *) tr_malloc(sizeof(tr_range));
  
  tr_obj_init(vm, TR_RANGE, (OBJ) range, tr_const_get(vm, "Range"));
  range->first = first;
  range->last  = last;
  
  return (OBJ) range;
}

static OBJ tr_range_first(VM, OBJ self)
{
  return TR_CRANGE(self)->first;
}

static OBJ tr_range_last(VM, OBJ self)
{
  return TR_CRANGE(self)->last;
}

void tr_range_init(VM)
{
  OBJ class = tr_class_new(vm, "Range", tr_const_get(vm, "Object"));
  
  tr_def(vm, class, "first", tr_range_first, 0);
  tr_def(vm, class, "last", tr_range_last, 0);
}
