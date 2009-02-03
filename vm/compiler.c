#include <stdio.h>
#include "tr.h"
#include "opcode.h"

char *TrOpCode_names[] = {
  "none", "move", "loadk", "send", "jmp", "jmp_if", "jmp_unless", "return" };

static TrBlock *TrBlock_new() {
  TrBlock *b = TR_ALLOC(TrBlock);
  kv_init(b->k);
  kv_init(b->code);
  return b;
}

static void TrBlock_dump(TrBlock *b) {
  size_t i;
  printf("; block definition: %p\n", b);
  printf("; %lu registers\n", kv_size(b->k));
  for (i = 0; i < kv_size(b->k); ++i)
    printf(".value %-8s ; %lu\n", TR_STR_PTR(kv_A(b->k, i)), i);
  for (i = 0; i < kv_size(b->code); ++i) {
    TrOp op = kv_A(b->code, i);
    printf("[%03lu] %-10s  %d  %d\n", i, TrOpCode_names[op.i], op.a, op.b);
  }
  printf("; block end\n");
}

static int TrBlock_pushk(TrBlock *blk, OBJ k) {
  /* TODO reuse */
  int id = kv_size(blk->k);
  kv_push(OBJ, blk->k, k);
  return id;
}

static void TrBlock_pushop(TrBlock *blk, int i, int a, int b) {
  int ind = kv_size(blk->code);
  kv_pushp(TrOp, blk->code);
  TrOp *op = blk->code.a + ind;
  op->i = i;
  op->a = a;
  op->b = b;
}

TrCompiler *TrCompiler_new(VM, const char *fn) {
  TrCompiler *c = TR_ALLOC(TrCompiler);
  c->curline = 1;
  c->vm = vm;
  c->block = TrBlock_new(vm);
  return c;
}

void TrCompiler_dump(TrCompiler *c) {
  TrBlock_dump(c->block);
}

void TrCompiler_call(TrCompiler *c, OBJ msg) {
  int i = TrBlock_pushk(c->block, msg);
  TrBlock_pushop(c->block, TR_OP_SEND, 0, i);
}

void TrCompiler_pushk(TrCompiler *c, OBJ k) {
  int i = TrBlock_pushk(c->block, k);
  TrBlock_pushop(c->block, TR_OP_LOADK, 0, i);
}

void TrCompiler_finish(TrCompiler *c) {
  TrBlock_pushop(c->block, TR_OP_RETURN, 0, 0);
}

void TrCompiler_destroy(TrCompiler *c) {
  
}