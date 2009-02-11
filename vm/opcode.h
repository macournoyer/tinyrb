#ifndef _OPCODE_H_
#define _OPCODE_H_

/*
== TinyRb opcodes.
Format of one instruction: OPCODE A B C
Bx  -- unsigned value of BC
sBx  -- signed value of BC
R[A] -- Value of register which index is stored in A of the current instruction.
K[A] -- Value of the constant which index is stored in A of the current instruction.
*/
enum TrInstCode {
  /* opname            operands description */
  TR_OP_BOING,      /*          do nothing with elegance and frivolity */
  TR_OP_MOVE,       /* A B      R[A] = R[B]  */
  TR_OP_LOADK,      /* A Bx     R[A] = K[Bx] */
  TR_OP_STRING,     /* A Bx     R[A] = strings[Bx] */
  TR_OP_BOOL,       /* A B      R[A] = B + 1 */
  TR_OP_NIL,        /* A        R[A] = nil */
  TR_OP_SELF,       /* A        put self in R[A] */
  TR_OP_LOOKUP,     /* A Bx     R[A+1] = lookup method K[Bx] on R[A] and store */
  TR_OP_CACHE,      /* A B C    if sites[C] matches R[A].type, jmp +B and R[A+1] = sites[C].method */
  TR_OP_CALL,       /* A B C    call method R[A+1] on R[A] with B args starting at R[A+2], C = some flags (block, splat, etc.) */
  TR_OP_JMP,        /*   sBx    jump sBx instructions */
  TR_OP_JMPIF,      /* A sBx    jump sBx instructions if R[A] */
  TR_OP_JMPUNLESS,  /* A sBx    jump sBx instructions unless R[A] */
  TR_OP_RETURN,     /* A        return R[A] */
  TR_OP_SETLOCAL,   /* A B      locals[A] = R[B] */
  TR_OP_GETLOCAL,   /* A B      R[A] = locals[B] */
  TR_OP_FIXNUM_ADD,
  TR_OP_FIXNUM_LT,
  TR_OP_DEF,        /* A Bx     define method k[Bx] on self w/ blocks[A] */
  TR_OP_GETCONST,   /* A Bx     R[A] = Consts[k[Bx]] */
  TR_OP_SETCONST,   /* A Bx     Consts[k[Bx]] = R[A] */
  TR_OP_CLASS,      /* A Bx     R[A] = define class k[Bx] on self w/ blocks[A] */
  TR_OP_UNDEF,
  TR_OP_ALIAS,
  TR_OP_SUPER,
  TR_OP_YIELD,
  TR_OP_GETDYN,
  TR_OP_SETDYN,
  TR_OP_GETIVAR,
  TR_OP_SETIVAR,
  TR_OP_GETCVAR,
  TR_OP_SETCVAR,
  TR_OP_GETGLOBAL,
  TR_OP_SETGLOBAL,
  TR_OP_NEWARRAY,
  TR_OP_NEWHASH,
  TR_OP_NEWRANGE
};

#define TR_OP_NAMES \
  "boing", \
  "move", \
  "loadk", \
  "string", \
  "bool", \
  "nil", \
  "self", \
  "lookup", \
  "cache", \
  "call", \
  "jmp", \
  "jmpif", \
  "jmpunless", \
  "return", \
  "setlocal", \
  "getlocal", \
  "fixnum_add", \
  "fixnum_lt", \
  "def", \
  "getconst", \
  "setconst", \
  "class", \
  "undef", \
  "alias", \
  "super", \
  "yield", \
  "getdyn", \
  "setdyn", \
  "getivar", \
  "setivar", \
  "getcvar", \
  "setcvar", \
  "getglobal", \
  "setglobal", \
  "newarray", \
  "newhash", \
  "newrange"
  

#ifdef TR_THREADED_DISPATCH
/* has to be in some order as in enum TrInstCode */
#define TR_OP_LABELS \
  &&op_BOING, \
  &&op_MOVE, \
  &&op_LOADK, \
  &&op_STRING, \
  &&op_BOOL, \
  &&op_NIL, \
  &&op_SELF, \
  &&op_LOOKUP, \
  &&op_CACHE, \
  &&op_CALL, \
  &&op_JMP, \
  &&op_JMPIF, \
  &&op_JMPUNLESS, \
  &&op_RETURN, \
  &&op_SETLOCAL, \
  &&op_GETLOCAL, \
  &&op_FIXNUM_ADD, \
  &&op_FIXNUM_LT, \
  &&op_DEF, \
  &&op_GETCONST, \
  &&op_SETCONST, \
  &&op_CLASS
  
#endif

#endif /* _OPCODE_H_ */
