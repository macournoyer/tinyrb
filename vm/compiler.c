#include "tr.h"
#include "opcode.h"
#include "internal.h"

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

/* code generation macros */
#define PUSH_OP(BLK,I) ({ \
  kv_push(TrInst, (BLK)->code, (I)); \
  kv_size(BLK->code)-1; \
})
#define PUSH_OP_A(BLK, OP, A)         PUSH_OP(BLK, CREATE_ABC(TR_OP_##OP, A, 0, 0))
#define PUSH_OP_AB(BLK, OP, A, B)     PUSH_OP(BLK, CREATE_ABC(TR_OP_##OP, A, B, 0))
#define PUSH_OP_ABC(BLK, OP, A, B, C) PUSH_OP(BLK, CREATE_ABC(TR_OP_##OP, A, B, C))
#define PUSH_OP_ABx(BLK, OP, A, Bx)   PUSH_OP(BLK, CREATE_ABx(TR_OP_##OP, A, Bx))
#define PUSH_OP_AsBx(BLK, OP, A, sBx) ({ \
  TrInst __i = CREATE_ABx(TR_OP_##OP, A, 0); SETARG_sBx(__i, sBx); \
  PUSH_OP(BLK, __i); \
})

#define COMPILE_NODE(BLK,NODE,REG) ({\
  int nlocal = kv_size(BLK->locals); \
  TrCompiler_compile_node(vm, c, BLK, (TrNode *)NODE, REG); \
  kv_size(BLK->locals) - nlocal; \
})

#define ASSERT_NO_LOCAL_IN(MSG) \
  if (start_reg != reg) tr_raise("Can't create local variable inside " #MSG)

#define COMPILE_NODES(BLK,NODES,I,REG,REGOFF) \
  TR_ARRAY_EACH(NODES, I, v, { \
    REG += COMPILE_NODE(BLK, v, REG+REGOFF); \
  })
  
#define CNODE(N)              ((TrNode *)N)
#define NODE_TYPE(N)          (CNODE(N)->ntype)
#define NODE_ARG(N,I)         (CNODE(N)->args[I])

void TrCompiler_compile_node(VM, TrCompiler *c, TrBlock *b, TrNode *n, int reg);

static int TrCompiler_compile_node_to_RK(VM, TrCompiler *c, TrBlock *b, TrNode *n, int reg) {
  int i;
  
  /* k value */
  if (NODE_TYPE(n) == AST_VALUE) {
    return TrBlock_push_value(b, NODE_ARG(n, 0)) | 0x100;
    
  /* local */
  } else if (NODE_TYPE(n) == AST_SEND && (i = TrBlock_find_local(b, NODE_ARG(NODE_ARG(n, 1), 0))) != -1) {
    return i;
  
  /* not a local, need to compile */
  } else {
    COMPILE_NODE(b, n, reg);
    return reg;
  }
  
}

void TrCompiler_compile_node(VM, TrCompiler *c, TrBlock *b, TrNode *n, int reg) {
  if (!n) return;
  int start_reg = reg;
  if (reg >= b->regc) b->regc++;
  b->line = n->line;
  /* TODO this shit is very repetitive, need to refactor */
  switch (n->ntype) {
    case AST_ROOT:
    case AST_BLOCK:
      COMPILE_NODES(b, n->args[0], i, reg, 0);
      break;
    case AST_VALUE: {
      int i = TrBlock_push_value(b, n->args[0]);
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
        COMPILE_NODES(b, n->args[0], i, reg, i+1);
        ASSERT_NO_LOCAL_IN(Array);
      }
      PUSH_OP_AB(b, NEWARRAY, reg, size);
    } break;
    case AST_HASH: {
      size_t size = 0;
      if (n->args[0]) {
        size = TR_ARRAY_SIZE(n->args[0]);
        /* compile args */
        COMPILE_NODES(b, n->args[0], i, reg, i+1);
        ASSERT_NO_LOCAL_IN(Hash);
      }
      PUSH_OP_AB(b, NEWHASH, reg, size/2);
    } break;
    case AST_RANGE: {
      COMPILE_NODE(b, n->args[0], reg);
      COMPILE_NODE(b, n->args[1], reg+1);
      ASSERT_NO_LOCAL_IN(Range);
      PUSH_OP_ABC(b, NEWRANGE, reg, reg+1, n->args[2]);
    } break;
    case AST_ASSIGN: {
      OBJ name = n->args[0];
      COMPILE_NODE(b, n->args[1], reg);
      if (TrBlock_find_upval_in_scope(b, name) != -1) {
        /* upval */
        PUSH_OP_AB(b, SETUPVAL, reg, TrBlock_push_upval(b, name));
      } else {
        /* local */
        int i = TrBlock_push_local(b, name);
        TrInst *last_inst = &kv_A(b->code, kv_size(b->code) - 1);
        switch (GET_OPCODE(*last_inst)) {
          case TR_OP_ADD: /* Those instructions can load direcly into a local */
          case TR_OP_SUB:
          case TR_OP_LT:
            SETARG_A(*last_inst, i); break;
          default:
            if (i != reg) PUSH_OP_AB(b, MOVE, i, reg);
        }
      }
    } break;
    case AST_SETIVAR:
      COMPILE_NODE(b, n->args[1], reg);
      PUSH_OP_ABx(b, SETIVAR, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_GETIVAR:
      PUSH_OP_ABx(b, GETIVAR, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_SETCVAR:
      COMPILE_NODE(b, n->args[1], reg);
      PUSH_OP_ABx(b, SETCVAR, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_GETCVAR:
      PUSH_OP_ABx(b, GETCVAR, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_SETGLOBAL:
      COMPILE_NODE(b, n->args[1], reg);
      PUSH_OP_ABx(b, SETGLOBAL, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_GETGLOBAL:
      PUSH_OP_ABx(b, GETGLOBAL, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_SEND:
    /* can also be a variable access */
    {
      TrNode *msg = (TrNode *)n->args[1];
      OBJ name = msg->args[0];
      assert(msg->ntype == AST_MSG);
      int i;
      /* local */
      if ((i = TrBlock_find_local(b, name)) != -1) {
        if (reg != i) PUSH_OP_AB(b, MOVE, reg, i);
        
      /* upval */
      } else if (TrBlock_find_upval_in_scope(b, name) != -1) {
        i = TrBlock_push_upval(b, name);
        PUSH_OP_AB(b, GETUPVAL, reg, i);
        
      /* method call */
      } else {
        /* receiver */
        if (n->args[0])
          COMPILE_NODE(b, n->args[0], reg);
        else
          PUSH_OP_A(b, SELF, reg);
        i = TrBlock_push_value(b, name);
        /* args */
        size_t argc = 0;
        if (msg->args[1]) {
          argc = TR_ARRAY_SIZE(msg->args[1]) << 1;
          TR_ARRAY_EACH(msg->args[1], i, v, {
            TrNode *arg = (TrNode *)v;
            assert(arg->ntype == AST_ARG);
            reg += COMPILE_NODE(b, arg->args[0], reg+i+2);
            if (arg->args[1]) argc |= 1; /* splat */
          });
          ASSERT_NO_LOCAL_IN(arguments);
        }
        /* block */
        size_t blki = 0;
        TrBlock *blk = 0;
        if (n->args[2]) {
          blk = TrBlock_new(c, b);
          TrNode *blkn = (TrNode *)n->args[2];
          blki = kv_size(b->blocks) + 1;
          blk->argc = 0;
          if (blkn->args[1]) {
            blk->argc = TR_ARRAY_SIZE(blkn->args[1]);
            /* add parameters as locals in block context */
            TR_ARRAY_EACH(blkn->args[1], i, v, {
              TrNode *param = (TrNode *)v;
              TrBlock_push_local(blk, param->args[0]);
            });
          }
          kv_push(TrBlock *, b->blocks, blk);
          int blk_reg = kv_size(blk->locals);
          COMPILE_NODE(blk, blkn, blk_reg);
          PUSH_OP_A(blk, RETURN, blk_reg);
        }
        PUSH_OP_A(b, BOING, 0);
        PUSH_OP_ABx(b, LOOKUP, reg, i);
        PUSH_OP_ABC(b, CALL, reg, argc, blki);
        
        /* if passed block has upvalues generate one pseudo-instructions for each (A reg is ignored). */
        if (blk && kv_size(blk->upvals)) {
          for(i = 0; i < kv_size(blk->upvals); ++i) {
            OBJ upval_name = kv_A(blk->upvals, i);
            size_t vali = TrBlock_find_local(b, upval_name);
            if (vali != -1)
              PUSH_OP_AB(b, MOVE, 0, vali);
            else
              PUSH_OP_AB(b, GETUPVAL, 0, TrBlock_find_upval(b, upval_name));
          }
        }
      }
    } break;
    case AST_IF:
    case AST_UNLESS: {
      /* condition */
      COMPILE_NODE(b, n->args[0], reg);
      int jmp;
      if (n->ntype == AST_IF)
        jmp = PUSH_OP_A(b, JMPUNLESS, reg);
      else
        jmp = PUSH_OP_A(b, JMPIF, reg);
      /* body */
      COMPILE_NODES(b, n->args[1], i, reg, 0);
      SETARG_sBx(kv_A(b->code, jmp), kv_size(b->code) - jmp);
      /* else body */
      jmp = PUSH_OP_A(b, JMP, reg);
      if (n->args[2]) {
        COMPILE_NODES(b, n->args[2], i, reg, 0);
      } else {
        /* if condition fail and not else block
           nil is returned */
        PUSH_OP_A(b, NIL, reg);
      }
      SETARG_sBx(kv_A(b->code, jmp), kv_size(b->code) - jmp - 1);
    } break;
    case AST_WHILE:
    case AST_UNTIL: {
      size_t jmp_beg = kv_size(b->code);
      /* condition */
      COMPILE_NODE(b, n->args[0], reg);
      if (n->ntype == AST_WHILE)
        PUSH_OP_ABx(b, JMPUNLESS, reg, 0);
      else
        PUSH_OP_ABx(b, JMPIF, reg, 0);
      size_t jmp_end = kv_size(b->code);
      /* body */
      COMPILE_NODES(b, n->args[1], i, reg, 0);
      SETARG_sBx(kv_A(b->code, jmp_end - 1), kv_size(b->code) - jmp_end + 1);
      PUSH_OP_AsBx(b, JMP, 0, 0-(kv_size(b->code) - jmp_beg) - 1);
    } break;
    case AST_AND:
    case AST_OR: {
      /* receiver */
      COMPILE_NODE(b, n->args[0], reg);
      TrCompiler_compile_node(vm, c, b, (TrNode *)n->args[0], reg);
      int jmp;
      if (n->ntype == AST_AND)
        jmp = PUSH_OP_A(b, JMPUNLESS, reg);
      else
        jmp = PUSH_OP_A(b, JMPIF, reg);
      /* arg */
      COMPILE_NODE(b, n->args[1], reg);
      SETARG_sBx(kv_A(b->code, jmp), kv_size(b->code) - jmp - 1);
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
      if (n->args[0]) COMPILE_NODE(b, n->args[0], reg);
      PUSH_OP_A(b, RETURN, reg);
      break;
    case AST_YIELD: {
      size_t argc = 0;
      if (n->args[0]) {
        argc = TR_ARRAY_SIZE(n->args[0]);
        COMPILE_NODES(b, n->args[0], i, reg, i+1);
        ASSERT_NO_LOCAL_IN(yield);
      }
      PUSH_OP_AB(b, YIELD, reg, argc);
    } break;
    case AST_DEF: {
      TrNode *method = (TrNode *)n->args[0];
      assert(method->ntype == AST_METHOD);
      TrBlock *blk = TrBlock_new(c, 0);
      size_t blki = kv_size(b->blocks);
      int blk_reg = 0;
      kv_push(TrBlock *, b->blocks, blk);
      if (n->args[1]) {
        /* add parameters as locals in method context */
        blk->argc = TR_ARRAY_SIZE(n->args[1]);
        TR_ARRAY_EACH(n->args[1], i, v, {
          TrNode *param = (TrNode *)v;
          TrBlock_push_local(blk, param->args[0]);
          if (param->args[1]) blk->arg_splat = 1;
          /* compile default expression and store location
             in defaults table for later jump when executing. */
          if (param->args[2]) {
            COMPILE_NODE(blk, param->args[2], blk_reg);
            kv_push(int, blk->defaults, kv_size(blk->code));
          }
          blk_reg++;
        });
      }
      /* compile body of method */
      COMPILE_NODES(blk, n->args[2], i, blk_reg, 0);
      PUSH_OP_A(blk, RETURN, blk_reg);
      if (method->args[0]) {
        /* metaclass def */
        COMPILE_NODE(b, method->args[0], reg);
        PUSH_OP_ABx(b, METADEF, blki, TrBlock_push_value(b, method->args[1]));
        PUSH_OP_A(b, BOING, reg);
      } else {
        PUSH_OP_ABx(b, DEF, blki, TrBlock_push_value(b, method->args[1]));
      }
    } break;
    case AST_CLASS:
    case AST_MODULE: {
      TrBlock *blk = TrBlock_new(c, 0);
      size_t blki = kv_size(b->blocks);
      kv_push(TrBlock *, b->blocks, blk);
      reg = 0;
      /* compile body of class */
      COMPILE_NODES(blk, n->args[2], i, reg, 0);
      PUSH_OP_A(blk, RETURN, reg);
      if (n->ntype == AST_CLASS) {
        /* superclass */
        if (n->args[1])
          PUSH_OP_ABx(b, GETCONST, reg, TrBlock_push_value(b, n->args[1]));
        else
          PUSH_OP_A(b, NIL, reg);
        PUSH_OP_ABx(b, CLASS, blki, TrBlock_push_value(b, n->args[0]));
        PUSH_OP_A(b, BOING, reg);
      } else {
        PUSH_OP_ABx(b, MODULE, blki, TrBlock_push_value(b, n->args[0]));
      }
    } break;
    case AST_CONST:
      PUSH_OP_ABx(b, GETCONST, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_SETCONST:
      COMPILE_NODE(b, n->args[1], reg);
      PUSH_OP_ABx(b, SETCONST, reg, TrBlock_push_value(b, n->args[0]));
      break;
    case AST_ADD:
    case AST_SUB:
    case AST_LT: {
      int rcv = TrCompiler_compile_node_to_RK(vm, c, b, CNODE(NODE_ARG(n, 0)), reg);
      int arg = TrCompiler_compile_node_to_RK(vm, c, b, CNODE(NODE_ARG(n, 1)), reg);
      switch (n->ntype) {
        case AST_ADD: PUSH_OP_ABC(b, ADD, reg, rcv, arg); break;
        case AST_SUB: PUSH_OP_ABC(b, SUB, reg, rcv, arg); break;
        case AST_LT:  PUSH_OP_ABC(b, LT, reg, rcv, arg); break;
        default: assert(0);
      }
    } break;
    default:
      printf("Compiler: unknown node type: %d in %s:%lu\n", n->ntype, TR_STR_PTR(b->filename), b->line);
      if (vm->debug) assert(0);
  }
}

void TrCompiler_compile(TrCompiler *c) {
  TrBlock *b = c->block;
  b->filename = c->filename;
  TrCompiler_compile_node(c->vm, c, b, (TrNode *)c->node, 0);
  PUSH_OP_A(b, RETURN, 0);
}
