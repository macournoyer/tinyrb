#include "tr.h"
#include "opcode.h"
#include "internal.h"

TrBlock *TrBlock_new(TrCompiler *compiler, TrBlock *parent) {
  TrBlock *b = TR_ALLOC(TrBlock);
  kv_init(b->k);
  kv_init(b->code);
  kv_init(b->locals);
  kv_init(b->strings);
  kv_init(b->sites);
  b->regc = 0;
  b->argc = 0;
  b->filename = compiler->filename;
  b->line = 1;
  b->parent = parent;
  return b;
}

#define INSPECT(K)  (TR_IS_A(K, Symbol) ? TR_STR_PTR(K) : (sprintf(buf, "%d", TR_FIX2INT(K)), buf))
#define VBx(OP)     (unsigned short)(((OP.b<<8)+OP.c))
#define sVBx(OP)    (short)(((OP.b<<8)+OP.c))

static void TrBlock_dump2(VM, TrBlock *b, int level) {
  static char *opcode_names[] = { TR_OP_NAMES };
  char buf[10];
  
  size_t i;
  printf("; block definition: %p (level %d)\n", b, level);
  printf("; %lu registers ; %lu nested blocks\n", b->regc, kv_size(b->blocks));
  printf("; %lu args ", b->argc);
  if (b->arg_splat) printf(", splat");
  printf("\n");
  for (i = 0; i < kv_size(b->locals); ++i)
    printf(".local  %-8s ; %lu\n", INSPECT(kv_A(b->locals, i)), i);
  for (i = 0; i < kv_size(b->upvals); ++i)
    printf(".upval  %-8s ; %lu\n", INSPECT(kv_A(b->upvals, i)), i);
  for (i = 0; i < kv_size(b->k); ++i) {
    printf(".value  %-8s ; %lu\n", INSPECT(kv_A(b->k, i)), i);
  }
  for (i = 0; i < kv_size(b->strings); ++i)
    printf(".string %-8s ; %lu\n", kv_A(b->strings, i), i);
  for (i = 0; i < kv_size(b->code); ++i) {
    TrInst op = kv_A(b->code, i);
    printf("[%03lu] %-10s %3d %3d %3d", i, opcode_names[op.i], op.a, op.b, op.c);
    switch (op.i) {
      case TR_OP_LOADK:    printf(" ; R[%d] = %s", op.a, INSPECT(kv_A(b->k, VBx(op)))); break;
      case TR_OP_STRING:   printf(" ; R[%d] = \"%s\"", op.a, kv_A(b->strings, VBx(op))); break;
      case TR_OP_LOOKUP:   printf(" ; R[%d] = R[%d].method(:%s)", op.a+1, op.a, INSPECT(kv_A(b->k, VBx(op)))); break;
      case TR_OP_CALL:     printf(" ; R[%d] = R[%d].R[%d](%d)", op.a, op.a, op.a+1, op.b>>1); break;
      case TR_OP_SETUPVAL: printf(" ; %s = R[%d]", INSPECT(kv_A(b->upvals, op.b)), op.a); break;
      case TR_OP_GETUPVAL: printf(" ; R[%d] = %s", op.a, INSPECT(kv_A(b->upvals, op.b))); break;
      case TR_OP_JMP:      printf(" ; %d", sVBx(op)); break;
      case TR_OP_DEF:      printf(" ; %s => %p", INSPECT(kv_A(b->k, VBx(op))), kv_A(b->blocks, op.a)); break;
    }
    printf("\n");
  }
  printf("; block end\n\n");

  for (i = 0; i < kv_size(b->blocks); ++i)
    TrBlock_dump2(vm, kv_A(b->blocks, i), level+1);
}

void TrBlock_dump(VM, TrBlock *b) {
  TrBlock_dump2(vm, b, 0);
}

int TrBlock_push_value(TrBlock *blk, OBJ k) {
  size_t i;
  for (i = 0; i < kv_size(blk->k); ++i)
    if (kv_A(blk->k, i) == k) return i;
  kv_push(OBJ, blk->k, k);
  return kv_size(blk->k)-1;
}

int TrBlock_push_string(TrBlock *blk, char *str) {
  size_t i;
  for (i = 0; i < kv_size(blk->strings); ++i)
    if (strcmp(kv_A(blk->strings, i), str) == 0) return i;
  size_t len = strlen(str);
  char *ptr = TR_ALLOC_N(char, len+1);
  TR_MEMCPY_N(ptr, str, char, len+1);
  kv_push(char *, blk->strings, ptr);
  return kv_size(blk->strings)-1;
}

int TrBlock_find_local(TrBlock *blk, OBJ name) {
  size_t i;
  for (i = 0; i < kv_size(blk->locals); ++i)
    if (kv_A(blk->locals, i) == name) return i;
  return -1;
}

int TrBlock_push_local(TrBlock *blk, OBJ name) {
  size_t i = TrBlock_find_local(blk, name);
  if (i != -1) return i;
  kv_push(OBJ, blk->locals, name);
  return kv_size(blk->locals)-1;
}

int TrBlock_find_upval(TrBlock *blk, OBJ name) {
  size_t i;
  for (i = 0; i < kv_size(blk->upvals); ++i)
    if (kv_A(blk->upvals, i) == name) return i;
  return -1;
}

int TrBlock_find_upval_in_scope(TrBlock *blk, OBJ name) {
  if (!blk->parent) return -1;
  int i = -1;
  while (blk && (i = TrBlock_find_local(blk, name)) == -1)
    blk = blk->parent;
  return i;
}

int TrBlock_push_upval(TrBlock *blk, OBJ name) {
  size_t i = TrBlock_find_upval(blk, name);
  if (i != -1) return i;
  
  TrBlock *b = blk;
  while (b->parent) {
    if (TrBlock_find_upval(b, name) == -1) kv_push(OBJ, b->upvals, name);
    b = b->parent;
  }
  
  return kv_size(blk->upvals)-1;
}
