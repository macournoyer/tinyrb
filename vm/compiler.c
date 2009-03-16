#include <stdio.h>
#include "tr.h"
#include "opcode.h"
#include "internal.h"

/* generation */
#define PUSH_OP_ABC(BLK,OP,A,B,C) ({ \
  TrInst *op = (kv_pushp(TrInst, (BLK)->code)); \
  op->i = TR_OP_##OP; op->a = (A); op->b = (B); op->c = (C); \
  kv_size(BLK->code)-1;\
})
#define PUSH_OP_A(BLK,OP,A)     PUSH_OP_ABC((BLK), OP,(A),0,0)
#define PUSH_OP_AB(BLK,OP,A,B)  PUSH_OP_ABC((BLK), OP,(A),(B),0)
#define PUSH_OP_ABx(BLK,OP,A,B) PUSH_OP_ABC((BLK), OP,(A),(B)>>8,(B)-((B)>>8<<8))
#define INST(BLK,I)             (&kv_A((BLK)->code,I))
#define SET_Bx(I,B)             (I)->b=(B)>>8; (I)->c=(B)-((B)>>8<<8)
#define INSPECT(K)              (TR_IS_A(K, Symbol) ? TR_STR_PTR(K) : (sprintf(buf, "%d", TR_FIX2INT(K)), buf))
#define VBx(OP)                 (unsigned short)(((OP.b<<8)+OP.c))
#define sVBx(OP)                (short)(((OP.b<<8)+OP.c))

/* ast node */
OBJ TrNode_new(VM, TrNodeType type, OBJ a, OBJ b, OBJ c, size_t line) {
  TrNode *n = TR_ALLOC(TrNode);
  n->ntype = type;
  n->type = TR_T_Node;
  n->args[0] = a;
  n->args[1] = b;
  n->args[2] = c;
  n->line = line;
  return (OBJ)n;
}

/* code block */
static TrBlock *TrBlock_new(TrCompiler *compiler, TrBlock *parent) {
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
      case TR_OP_SETLOCAL: printf(" ; %s = R[%d]", INSPECT(kv_A(b->locals, op.b)), op.a); break;
      case TR_OP_GETLOCAL: printf(" ; R[%d] = %s", op.a, INSPECT(kv_A(b->locals, op.b))); break;
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

static int TrBlock_hasupval(TrBlock *blk, OBJ name) {
  size_t i;
  for (i = 0; i < kv_size(blk->upvals); ++i)
    if (kv_A(blk->upvals, i) == name) return i;
  return -1;
}

static int TrBlock_upval(TrBlock *blk, OBJ name) {
  size_t i = TrBlock_hasupval(blk, name);
  if (i != -1) return i;
  kv_push(OBJ, blk->upvals, name);
  return kv_size(blk->upvals)-1;
}

/* compiler */

TrCompiler *TrCompiler_new(VM, const char *fn) {
  TrCompiler *c = TR_ALLOC(TrCompiler);
  c->line = 1;
  c->vm = vm;
  c->block = TrBlock_new(c, 0);
  c->reg = 0;
  c->node = TR_NIL;
  c->filename = TrString_new2(vm, fn);
  return c;
}

void TrCompiler_compile_node(VM, TrCompiler *c, TrBlock *b, TrNode *n, int reg) {
  if (!n) return;
  if (reg >= b->regc) b->regc++;
  b->line = n->line;
  /* TODO this shit is very repetitive, need to refactor */
  switch (n->ntype) {
    case AST_ROOT:
    case AST_BLOCK:
      TR_ARRAY_EACH(n->args[0], i, v, {
        TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg);
      });
      break;
    case AST_VALUE: {
      int i = TrBlock_pushk(b, n->args[0]);
      PUSH_OP_ABx(b, LOADK, reg, i);
    } break;
    case AST_STRING: {
      int i = TrBlock_push_string(b, TR_STR_PTR(n->args[0]));
      PUSH_OP_ABx(b, STRING, reg, i);
    } break;
    case AST_ARRAY: {
      size_t size = 0;
      if (n->args[0]) {
        size = TR_ARRAY_SIZE(n->args[0]);
        /* compile args */
        TR_ARRAY_EACH(n->args[0], i, v, {
          TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg+i+1);
        });
      }
      PUSH_OP_AB(b, NEWARRAY, reg, size);
    } break;
    case AST_HASH: {
      size_t size = 0;
      if (n->args[0]) {
        size = TR_ARRAY_SIZE(n->args[0]);
        /* compile args */
        TR_ARRAY_EACH(n->args[0], i, v, {
          TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg+i+1);
        });
      }
      PUSH_OP_AB(b, NEWHASH, reg, size/2);
    } break;
    case AST_RANGE: {
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg+1);
      PUSH_OP_ABC(b, NEWRANGE, reg, reg+1, n->args[2]);
    } break;
    case AST_ASSIGN: {
      int i = TrBlock_local(b, n->args[0]);
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg);
      PUSH_OP_AB(b, SETLOCAL, reg, i);
    } break;
    case AST_SETIVAR:
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg);
      PUSH_OP_ABx(b, SETIVAR, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_GETIVAR:
      PUSH_OP_ABx(b, GETIVAR, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_SETCVAR:
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg);
      PUSH_OP_ABx(b, SETCVAR, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_GETCVAR:
      PUSH_OP_ABx(b, GETCVAR, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_SETGLOBAL:
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg);
      PUSH_OP_ABx(b, SETGLOBAL, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_GETGLOBAL:
      PUSH_OP_ABx(b, GETGLOBAL, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_SEND: { /* can be a method send or a local var access */
      TrNode *msg = (TrNode *)n->args[1];
      assert(msg->ntype == AST_MSG);
      int i;
      /* local */
      if ((i = TrBlock_haslocal(b, msg->args[0])) != -1) {
        PUSH_OP_AB(b, GETLOCAL, reg, i);
        
      /* upval */
      } else if (b->parent && (i = TrBlock_haslocal(b->parent, msg->args[0])) != -1) {
        i = TrBlock_upval(b, msg->args[0]);
        PUSH_OP_AB(b, GETUPVAL, reg, i);
        
      /* method call */
      } else {
        /* receiver */
        if (n->args[0])
          TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
        else
          PUSH_OP_A(b, SELF, reg);
        i = TrBlock_pushk(b, msg->args[0]);
        /* args */
        size_t argc = 0;
        if (msg->args[1]) {
          argc = TR_ARRAY_SIZE(msg->args[1]) << 1;
          TR_ARRAY_EACH(msg->args[1], i, v, {
            TrNode *arg = (TrNode *)v;
            assert(arg->ntype == AST_ARG);
            TrCompiler_compile_node(vm, c, b, (TrNode *)arg->args[0], reg+i+2);
            if (arg->args[1]) argc |= 1; /* splat */
          });
        }
        /* block */
        size_t blki = 0;
        if (n->args[2]) {
          TrBlock *blk = TrBlock_new(c, b);
          TrNode *blkn = (TrNode *)n->args[2];
          blki = kv_size(b->blocks) + 1;
          blk->argc = 0;
          if (blkn->args[1]) {
            blk->argc = TR_ARRAY_SIZE(blkn->args[1]);
            /* add parameters as locals in block context */
            TR_ARRAY_EACH(blkn->args[1], i, v, {
              TrNode *param = (TrNode *)v;
              TrBlock_local(blk, param->args[0]);
            });
          }
          kv_push(TrBlock *, b->blocks, blk);
          TrCompiler_compile_node(vm, c, blk, blkn, 0);
          PUSH_OP_A(blk, RETURN, 0);
        }
        PUSH_OP_A(b, BOING, 0);
        PUSH_OP_ABx(b, LOOKUP, reg, i);
        PUSH_OP_ABC(b, CALL, reg, argc, blki);
      }
    } break;
    case AST_IF:
    case AST_UNLESS: {
      /* condition */
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
      int jmp;
      if (n->ntype == AST_IF)
        jmp = PUSH_OP_A(b, JMPUNLESS, reg);
      else
        jmp = PUSH_OP_A(b, JMPIF, reg);
      /* body */
      TR_ARRAY_EACH(n->args[1], i, v, {
        TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg);
      });
      SET_Bx(INST(b, jmp), kv_size(b->code) - jmp);
      /* else body */
      jmp = PUSH_OP_A(b, JMP, reg);
      if (n->args[2]) {
        TR_ARRAY_EACH(n->args[2], i, v, {
          TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg);
        });
      } else {
        /* if condition fail and not else block
           nil is returned */
        PUSH_OP_A(b, NIL, reg);
      }
      SET_Bx(INST(b, jmp), kv_size(b->code) - jmp - 1);
    } break;
    case AST_WHILE:
    case AST_UNTIL: {
      size_t jmp_beg = kv_size(b->code);
      /* condition */
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
      if (n->ntype == AST_WHILE)
        PUSH_OP_ABx(b, JMPUNLESS, reg, 0);
      else
        PUSH_OP_ABx(b, JMPIF, reg, 0);
      size_t jmp_end = kv_size(b->code);
      /* body */
      TR_ARRAY_EACH(n->args[1], i, v, {
        TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg);
      });
      SET_Bx(b->code.a + jmp_end - 1, kv_size(b->code) - jmp_end + 1);
      PUSH_OP_ABx(b, JMP, 0, 0-(kv_size(b->code) - jmp_beg));
    } break;
    case AST_AND:
    case AST_OR: {
      /* receiver */
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
      int jmp;
      if (n->ntype == AST_AND)
        jmp = PUSH_OP_A(b, JMPUNLESS, reg);
      else
        jmp = PUSH_OP_A(b, JMPIF, reg);
      /* arg */
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg);
      SET_Bx(INST(b, jmp), kv_size(b->code) - jmp - 1);
    } break;
    case AST_BOOL:
      PUSH_OP_AB(b, BOOL, reg, n->args[0]);
      break;
    case AST_NIL:
      PUSH_OP_A(b, NIL, reg);
      break;
    case AST_SELF:
      PUSH_OP_A(b, SELF, reg);
      break;
    case AST_RETURN:
      if (n->args[0]) TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
      PUSH_OP_A(b, RETURN, reg);
      break;
    case AST_YIELD: {
      size_t argc = 0;
      if (n->args[0]) {
        argc = TR_ARRAY_SIZE(n->args[0]);
        TR_ARRAY_EACH(n->args[0], i, v, {
          TrCompiler_compile_node(vm, c, b, (TrNode *)v, reg+i+1);
        });
      }
      PUSH_OP_AB(b, YIELD, reg, argc);
    } break;
    case AST_DEF: {
      TrNode *method = (TrNode *)n->args[0];
      assert(method->ntype == AST_METHOD);
      TrBlock *blk = TrBlock_new(c, 0);
      size_t blki = kv_size(b->blocks);
      kv_push(TrBlock *, b->blocks, blk);
      if (n->args[1]) {
        blk->argc = TR_ARRAY_SIZE(n->args[1]);
        /* add parameters as locals in method context */
        TR_ARRAY_EACH(n->args[1], i, v, {
          TrNode *param = (TrNode *)v;
          TrBlock_local(blk, param->args[0]);
          if (param->args[1]) blk->arg_splat = 1;
        });
      }
      /* compile body of method */
      TR_ARRAY_EACH(n->args[2], i, v, {
        TrCompiler_compile_node(vm, c, blk, (TrNode *)v, 0);
      });
      PUSH_OP_A(blk, RETURN, 0);
      if (method->args[0]) {
        /* metaclass def */
        TrCompiler_compile_node(vm, c, b, (TrNode*)method->args[0], 0);
        PUSH_OP_ABx(b, METADEF, blki, TrBlock_pushk(b, method->args[1]));
      } else {
        PUSH_OP_ABx(b, DEF, blki, TrBlock_pushk(b, method->args[1]));
      }
    } break;
    case AST_CLASS:
    case AST_MODULE: {
      TrBlock *blk = TrBlock_new(c, 0);
      size_t blki = kv_size(b->blocks);
      kv_push(TrBlock *, b->blocks, blk);
      /* compile body of class */
      TR_ARRAY_EACH(n->args[2], i, v, {
        TrCompiler_compile_node(vm, c, blk, (TrNode *)v, 0);
      });
      PUSH_OP_A(blk, RETURN, 0);
      if (n->ntype == AST_CLASS) {
        /* superclass */
        if (n->args[1])
          PUSH_OP_ABx(b, GETCONST, 0, TrBlock_pushk(b, n->args[1]));
        else
          PUSH_OP_A(b, NIL, 0);
        PUSH_OP_ABx(b, CLASS, blki, TrBlock_pushk(b, n->args[0]));
      } else {
        PUSH_OP_ABx(b, MODULE, blki, TrBlock_pushk(b, n->args[0]));
      }
    } break;
    case AST_CONST:
      PUSH_OP_ABx(b, GETCONST, reg, TrBlock_pushk(b, n->args[0]));
      break;
    case AST_SETCONST:
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[1], reg);
      PUSH_OP_ABx(b, SETCONST, reg, TrBlock_pushk(b, n->args[0]));
      break;
    default:
      printf("Compiler: unknown node type: %d\n", n->ntype);
      if (vm->debug) assert(0);
  }
}

void TrCompiler_compile(TrCompiler *c) {
  TrBlock *b = c->block;
  b->filename = c->filename;
  TrCompiler_compile_node(c->vm, c, b, (TrNode *)c->node, 0);
  PUSH_OP_A(b, RETURN, 0);
}
