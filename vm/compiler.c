#include <stdio.h>
#include "tr.h"
#include "opcode.h"
#include "internal.h"

/* generation */
#define PUSH_OP_ABC(OP,A,B,C) ({ \
  TrInst *op = (kv_pushp(TrInst, b->code)); \
  op->i = TR_OP_##OP; op->a = (A); op->b = (B); op->c = (C); \
})
#define PUSH_OP_A(OP,A)     PUSH_OP_ABC(OP,(A),0,0)
#define PUSH_OP_AB(OP,A,B)  PUSH_OP_ABC(OP,(A),(B),0)
#define PUSH_OP_ABx(OP,A,B) PUSH_OP_ABC(OP,(A),(B)>>8,(B)-((B)>>8<<8))
#define SET_Bx(I,B)         (I)->b=(B)>>8; (I)->c=(B)-((B)>>8<<8)
#define INSPECT(K)          (TR_IS_A(K, Symbol) ? TR_STR_PTR(K) : (sprintf(buf, "%d", FIX2INT(K)), buf))
#define VBx(OP)             (unsigned short)(((OP.b<<8)+OP.c))

/* ast node */
OBJ TrNode_new(VM, TrNodeType type, OBJ a, OBJ b, OBJ c) {
  TrNode *n = TR_ALLOC(TrNode);
  n->ntype = type;
  n->type = TR_T_Node;
  n->args[0] = a;
  n->args[1] = b;
  n->args[2] = c;
  return (OBJ)n;
}

/* code block */
static TrBlock *TrBlock_new() {
  TrBlock *b = TR_ALLOC(TrBlock);
  kv_init(b->k);
  kv_init(b->code);
  kv_init(b->locals);
  kv_init(b->strings);
  b->regc = 0;
  return b;
}

static void TrBlock_dump2(TrBlock *b, int level) {
  static char *opcode_names[] = { TR_OP_NAMES };
  char buf[10];
  
  size_t i;
  printf("; block definition: %p (level %d)\n", b, level);
  printf("; %lu registers ; %lu nested blocks\n", b->regc, kv_size(b->blocks));
  for (i = 0; i < kv_size(b->locals); ++i)
    printf(".local  %-8s ; %lu\n", INSPECT(kv_A(b->locals, i)), i);
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
      case TR_OP_LOOKUP:   printf(" ; R[%d] = R[%d].method(:%s)", op.a+1, op.a, INSPECT(kv_A(b->k, op.b))); break;
      case TR_OP_CALL:     printf(" ; R[%d] = R[%d].R[%d](%d)", op.a, op.a, op.a+1, op.b); break;
      case TR_OP_SETLOCAL: printf(" ; %s = R[%d]", INSPECT(kv_A(b->locals, op.a)), op.b); break;
      case TR_OP_GETLOCAL: printf(" ; R[%d] = %s", op.a, INSPECT(kv_A(b->locals, op.b))); break;
    }
    printf("\n");
  }
  for (i = 0; i < kv_size(b->blocks); ++i)
    TrBlock_dump2(kv_A(b->blocks, i), level+1);
  printf("; block end\n\n");
}

void TrBlock_dump(VM, TrBlock *b) {
  TrBlock_dump2(b, 0);
}

void TrBlock_destroy(VM, TrBlock *b) {
  /* TODO */
}

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
  size_t len = strlen(str);
  char *ptr = TR_ALLOC_N(char, len+1);
  TR_MEMCPY_N(ptr, str, char, len+1);
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

/* compiler */

TrCompiler *TrCompiler_new(VM, const char *fn) {
  TrCompiler *c = TR_ALLOC(TrCompiler);
  c->curline = 1;
  c->vm = vm;
  c->block = TrBlock_new(vm);
  c->reg = 0;
  c->node = TR_NIL;
  return c;
}

void TrCompiler_compile_node(TrCompiler *c, TrBlock *b, TrNode *n, int reg) {
  if (reg >= b->regc) b->regc++;
  switch (n->ntype) {
    case AST_ROOT:
    case AST_BLOCK:
      TR_ARRAY_EACH(n->args[0], i, v, {
        TrCompiler_compile_node(c, b, (TrNode *)v, reg);
      });
      break;
    case AST_CONST: {
      int i = TrBlock_pushk(b, n->args[0]);
      PUSH_OP_ABx(LOADK, reg, i);
    } break;
    case AST_STRING: {
      int i = TrBlock_push_string(b, TR_STR_PTR(n->args[0]));
      PUSH_OP_ABx(STRING, reg, i);
    } break;
    case AST_ASSIGN: {
      int i = TrBlock_local(c->block, n->args[0]);
      TrCompiler_compile_node(c, b, (TrNode *)n->args[1], reg);
      PUSH_OP_AB(SETLOCAL, i, reg);
    } break;
    case AST_SEND: {
      TrNode *msg = (TrNode *)n->args[1];
      assert(msg->ntype == AST_MSG);
      int i = TrBlock_haslocal(b, msg->args[0]);
      if (i != -1) { /* var */
        PUSH_OP_AB(GETLOCAL, reg, i);
      } else { /* method */
        if (n->args[0])
          TrCompiler_compile_node(c, b, (TrNode *)n->args[0], reg);
        else
          PUSH_OP_A(SELF, reg);
        i = TrBlock_pushk(b, msg->args[0]);
        PUSH_OP_A(BOING, 0);
        PUSH_OP_AB(LOOKUP, reg, i);
        size_t argc = 0;
        if (msg->args[1]) {
          argc = TR_ARRAY_SIZE(msg->args[1]);
          TR_ARRAY_EACH(msg->args[1], i, v, {
            TrCompiler_compile_node(c, b, (TrNode *)v, reg+i+2);
          });
        }
        PUSH_OP_ABC(CALL, reg, argc, 0);
      }
    } break;
    case AST_IF:
    case AST_UNLESS: {
      TrCompiler_compile_node(c, b, (TrNode *)n->args[0], reg);
      if (n->ntype == AST_IF)
        PUSH_OP_ABx(JMPUNLESS, reg, 0);
      else
        PUSH_OP_ABx(JMPIF, reg, 0);
      size_t i = kv_size(b->code);
      TR_ARRAY_EACH(n->args[1], i, v, { /* TODO fugly... */
        TrCompiler_compile_node(c, b, (TrNode *)v, reg);
      });
      SET_Bx(b->code.a + i - 1, kv_size(b->code) - i);
    } break;
    case AST_BOOL:
      PUSH_OP_AB(BOOL, reg, n->args[0]);
      break;
    case AST_NIL:
      PUSH_OP_A(NIL, reg);
      break;
    case AST_SELF:
      PUSH_OP_A(SELF, reg);
      break;
    case AST_RETURN:
      PUSH_OP_A(RETURN, reg);
      break;
    default:
      printf("unknown node type: %d\n", n->ntype);
  }
}

void TrCompiler_compile(TrCompiler *c) {
  TrBlock *b = c->block;
  TrCompiler_compile_node(c, b, (TrNode *)c->node, 0);
  PUSH_OP_A(RETURN, 0);
}

void TrCompiler_destroy(TrCompiler *c) {
  
}