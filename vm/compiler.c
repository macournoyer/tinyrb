#include <stdio.h>
#include "tr.h"
#include "opcode.h"

#define REG(b,reg) if (reg >= b->regc) b->regc++

char *TrOpCode_names[] = { TR_OP_NAMES };

static TrBlock *TrBlock_new() {
  TrBlock *b = TR_ALLOC(TrBlock);
  kv_init(b->k);
  kv_init(b->code);
  kv_init(b->locals);
  b->regc = 0;
  return b;
}

static void TrBlock_dump(TrBlock *b, int level) {
  size_t i;
  printf("; block definition: %p (level %d)\n", b, level);
  printf("; %lu registers ; %lu nested blocks\n", b->regc, kv_size(b->blocks));
  for (i = 0; i < kv_size(b->locals); ++i)
    printf(".local %-8s ; %lu\n", TR_STR_PTR(kv_A(b->locals, i)), i);
  for (i = 0; i < kv_size(b->k); ++i)
    printf(".value %-8s ; %lu\n", TR_STR_PTR(kv_A(b->k, i)), i);
  for (i = 0; i < kv_size(b->code); ++i) {
    TrOp op = kv_A(b->code, i);
    printf("[%03lu] %-10s  %d  %d", i, TrOpCode_names[op.i], op.a, op.b);
    switch (op.i) {
      case TR_OP_LOADK:    printf(" ; %s => %d", TR_STR_PTR(kv_A(b->k, op.b)), op.a); break;
      case TR_OP_SEND:     printf(" ; %s", TR_STR_PTR(kv_A(b->k, op.b))); break;
      case TR_OP_SETLOCAL: printf(" ; %s", TR_STR_PTR(kv_A(b->locals, op.a))); break;
      case TR_OP_GETLOCAL: printf(" ; %s", TR_STR_PTR(kv_A(b->locals, op.b))); break;
    }
    printf("\n");
  }
  for (i = 0; i < kv_size(b->blocks); ++i)
    TrBlock_dump(kv_A(b->blocks, i), level+1);
  printf("; block end\n\n");
}

static int TrBlock_pushk(TrBlock *blk, OBJ k) {
  size_t i;
  for (i = 0; i < kv_size(blk->k); ++i)
    if (kv_A(blk->k, i) == k) return i;
  kv_push(OBJ, blk->k, k);
  return kv_size(blk->k)-1;
}

static int TrBlock_local(TrBlock *blk, OBJ name) {
  size_t i;
  for (i = 0; i < kv_size(blk->locals); ++i)
    if (kv_A(blk->locals, i) == name) return i;
  kv_push(OBJ, blk->locals, name);
  return kv_size(blk->locals)-1;
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
  TrBlock_dump(c->block, 1);
}

int TrCompiler_call(TrCompiler *c, OBJ msg) {
  int i = TrBlock_pushk(c->block, msg);
  REG(c->block, 0);
  TrBlock_pushop(c->block, TR_OP_SEND, 0, i);
  return 0;
}

int TrCompiler_setlocal(TrCompiler *c, OBJ name, int reg) {
  REG(c->block, reg);
  TrBlock_pushop(c->block, TR_OP_SETLOCAL, reg, TrBlock_local(c->block, name));
  return 0;
}

int TrCompiler_getlocal(TrCompiler *c, OBJ name) {
  /* TODO push OP_SEND if not a local var */
  REG(c->block, 0);
  TrBlock_pushop(c->block, TR_OP_GETLOCAL, 0, TrBlock_local(c->block, name));
  return 0;
}

int TrCompiler_pushk(TrCompiler *c, OBJ k) {
  int i = TrBlock_pushk(c->block, k);
  /* TODO how to find w/ reg to use? */
  REG(c->block, 0);
  TrBlock_pushop(c->block, TR_OP_LOADK, 0, i);
  return 0;
}

void TrCompiler_finish(TrCompiler *c) {
  TrBlock_pushop(c->block, TR_OP_RETURN, 0, 0);
}

void TrCompiler_destroy(TrCompiler *c) {
  
}