#include <stdio.h>
#include "tr.h"
#include "opcode.h"

/* generation */
#define REG(R) if (R >= c->block->regc) c->block->regc++
#define PUSH_OP_ABC(OP,A,B,C) ({ \
  TrInst *op = (kv_pushp(TrInst, c->block->code)); \
  op->i = TR_OP_##OP; op->a = (A); op->b = (B); op->c = (C); \
})
#define PUSH_OP_AB(OP,A,B) PUSH_OP_ABC(OP,(A),(B),0)
#define PUSH_OP_ABx(OP,A,B) PUSH_OP_ABC(OP,(A),(B)>>8,(B)-((B)>>8<<8))

/* dumping */
#define VBx(OP) (unsigned short)(((OP.b<<8)+OP.c))

static TrBlock *TrBlock_new() {
  TrBlock *b = TR_ALLOC(TrBlock);
  kv_init(b->k);
  kv_init(b->code);
  kv_init(b->locals);
  kv_init(b->strings);
  b->regc = 0;
  return b;
}

static void TrBlock_dump(TrBlock *b, int level) {
  static char *opcode_names[] = { TR_OP_NAMES };
  
  size_t i;
  printf("; block definition: %p (level %d)\n", b, level);
  printf("; %lu registers ; %lu nested blocks\n", b->regc, kv_size(b->blocks));
  for (i = 0; i < kv_size(b->locals); ++i)
    printf(".local  %-8s ; %lu\n", TR_STR_PTR(kv_A(b->locals, i)), i);
  for (i = 0; i < kv_size(b->k); ++i)
    printf(".value  %-8s ; %lu\n", TR_STR_PTR(kv_A(b->k, i)), i);
  for (i = 0; i < kv_size(b->strings); ++i)
    printf(".string %-8s ; %lu\n", kv_A(b->strings, i), i);
  for (i = 0; i < kv_size(b->code); ++i) {
    TrInst op = kv_A(b->code, i);
    printf("[%03lu] %-10s %3d %3d %3d", i, opcode_names[op.i], op.a, op.b, op.c);
    switch (op.i) {
      case TR_OP_LOADK:    printf(" ; R[%d] = %s", op.a, TR_STR_PTR(kv_A(b->k, VBx(op)))); break;
      case TR_OP_STRING:   printf(" ; R[%d] = \"%s\"", op.a, kv_A(b->strings, VBx(op))); break;
      case TR_OP_SEND:     printf(" ; R[%d].%s", op.a, TR_STR_PTR(kv_A(b->k, op.b))); break;
      case TR_OP_SETLOCAL: printf(" ; %s = R[%d]", TR_STR_PTR(kv_A(b->locals, op.a)), op.b); break;
      case TR_OP_GETLOCAL: printf(" ; R[%d] = %s", op.a, TR_STR_PTR(kv_A(b->locals, op.b))); break;
    }
    printf("\n");
  }
  for (i = 0; i < kv_size(b->blocks); ++i)
    TrBlock_dump(kv_A(b->blocks, i), level+1);
  printf("; block end\n\n");
}

/* TODO refactor this, errk */

static int TrBlock_pushk(TrBlock *blk, OBJ k) {
  size_t i;
  for (i = 0; i < kv_size(blk->k); ++i)
    if (kv_A(blk->k, i) == k) return i;
  kv_push(OBJ, blk->k, k);
  return kv_size(blk->k)-1;
}

static int TrBlock_push_string(TrBlock *blk, char *str) {
  size_t i;
  for (i = 0; i < kv_size(blk->strings); ++i)
    if (strcmp(kv_A(blk->strings, i), str) == 0) return i;
  char *ptr = TR_ALLOC_N(char, strlen(str));
  TR_MEMCPY_N(ptr, str, char, strlen(str));
  kv_push(char *, blk->strings, ptr);
  return kv_size(blk->strings)-1;
}

static int TrBlock_haslocal(TrBlock *blk, OBJ name) {
  size_t i;
  for (i = 0; i < kv_size(blk->locals); ++i)
    if (kv_A(blk->locals, i) == name) return i;
  return -1;
}

static int TrBlock_local(TrBlock *blk, OBJ name) {
  size_t i = TrBlock_haslocal(blk, name);
  if (i != -1) return i;
  kv_push(OBJ, blk->locals, name);
  return kv_size(blk->locals)-1;
}

TrCompiler *TrCompiler_new(VM, const char *fn) {
  TrCompiler *c = TR_ALLOC(TrCompiler);
  c->curline = 1;
  c->vm = vm;
  c->block = TrBlock_new(vm);
  c->reg = 0;
  return c;
}

void TrCompiler_dump(TrCompiler *c) {
  TrBlock_dump(c->block, 1);
}

/* TODO how to find which reg # to use ??? */

int TrCompiler_call(TrCompiler *c, OBJ msg) {
  REG(c->reg);
  size_t i = TrBlock_haslocal(c->block, msg);
  if (i != -1) {
    /* TODO optimization: skip if value already in register */
    PUSH_OP_AB(GETLOCAL, c->reg, i);
  } else {
    PUSH_OP_AB(SEND, c->reg, TrBlock_pushk(c->block, msg));
  }
  return c->reg;
}

int TrCompiler_setlocal(TrCompiler *c, OBJ name, int reg) {
  REG(reg);
  PUSH_OP_AB(SETLOCAL, TrBlock_local(c->block, name), reg);
  return reg;
}

int TrCompiler_pushk(TrCompiler *c, OBJ k) {
  REG(c->reg);
  PUSH_OP_ABx(LOADK, c->reg, TrBlock_pushk(c->block, k));
  return c->reg;
}

int TrCompiler_string(TrCompiler *c, OBJ str) {
  REG(c->reg);
  PUSH_OP_ABx(STRING, c->reg, TrBlock_push_string(c->block, (char*)str));
  return c->reg;
}

void TrCompiler_finish(TrCompiler *c) {
  PUSH_OP_AB(RETURN, c->reg, 0);
}

void TrCompiler_destroy(TrCompiler *c) {
  
}