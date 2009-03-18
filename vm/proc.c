#include "tr.h"
#include "internal.h"

TrClosure *TrClosure_new(VM, TrBlock *b) {
  TrClosure *cl = TR_ALLOC(TrClosure);
  cl->block = b;
  cl->upvals = TR_ALLOC_N(TrUpval, kv_size(b->upvals));
  return cl;
}
