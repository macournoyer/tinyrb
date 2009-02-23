#ifndef _OPCODE_H_
#define _OPCODE_H_

/*
== TinyRb opcodes.
Format of one instruction: OPCODE A B C
Bx   -- unsigned value of BC
sBx  -- signed value of BC
R[A] -- Value of register which index is stored in A of the current instruction.
K[A] -- Value which index is stored in A of the current instruction.
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
  TR_OP_CALL,       /* A B C    call method R[A+1] on R[A] with B args starting at R[A+2], w/ block[C-1] if C */
  TR_OP_JMP,        /*   sBx    jump sBx instructions */
  TR_OP_JMPIF,      /* A sBx    jump sBx instructions if R[A] */
  TR_OP_JMPUNLESS,  /* A sBx    jump sBx instructions unless R[A] */
  TR_OP_RETURN,     /* A        return R[A] */
  TR_OP_SETLOCAL,   /* A B      locals[A] = R[B] */
  TR_OP_GETLOCAL,   /* A B      R[A] = locals[B] */
  TR_OP_FIXNUM_ADD,
  TR_OP_FIXNUM_SUB,
  TR_OP_FIXNUM_LT,
  TR_OP_DEF,        /* A Bx     R[0] = define method k[Bx] on self w/ blocks[A] */
  TR_OP_GETCONST,   /* A Bx     R[A] = Consts[k[Bx]] */
  TR_OP_SETCONST,   /* A Bx     Consts[k[Bx]] = R[A] */
  TR_OP_CLASS,      /* A Bx     R[0] = define class k[Bx] on self w/ blocks[A] and superclass R[0] */
  TR_OP_MODULE,     /* A Bx     R[0] = define module k[Bx] on self w/ blocks[A] */
  TR_OP_NEWARRAY,   /* A B      R[A] = Array.new(R[A+1]..R[A+1+B]) */
  TR_OP_NEWHASH,    /* A B      R[A] = Hash.new(R[A+1] => R[A+2] .. R[A+1+B*2] => R[A+2+B*2]) */
  TR_OP_YIELD,      /* A B      R[A] = passed_block.call(R[A+1]..R[A+1+B]) */
  TR_OP_GETIVAR,    /* A Bx     R[A] = self.ivars[k[Bx]] */
  TR_OP_SETIVAR,    /* A Bx     self.ivars[k[Bx]] = R[A] */
  TR_OP_GETCVAR,    /* A Bx     R[A] = class.ivars[k[Bx]] */
  TR_OP_SETCVAR,    /* A Bx     class.ivars[k[Bx]] = R[A] */
  TR_OP_GETGLOBAL,  /* A Bx     R[A] = globals[k[Bx]] */
  TR_OP_SETGLOBAL,  /* A Bx     globals[k[Bx]] = R[A] */
  TR_OP_UNDEF,
  TR_OP_ALIAS,
  TR_OP_SUPER,
  TR_OP_GETDYN,
  TR_OP_SETDYN,
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
  "fixnum_sub", \
  "fixnum_lt", \
  "def", \
  "getconst", \
  "setconst", \
  "class", \
  "module", \
  "newarray", \
  "newhash", \
  "yield", \
  "getivar", \
  "setivar", \
  "getcvar", \
  "setcvar", \
  "getglobal", \
  "setglobal", \
  "undef", \
  "alias", \
  "super", \
  "getdyn", \
  "setdyn", \
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
  &&op_FIXNUM_SUB, \
  &&op_FIXNUM_LT, \
  &&op_DEF, \
  &&op_GETCONST, \
  &&op_SETCONST, \
  &&op_CLASS, \
  &&op_MODULE, \
  &&op_NEWARRAY, \
  &&op_NEWHASH, \
  &&op_YIELD, \
  &&op_GETIVAR, \
  &&op_SETIVAR, \
  &&op_GETCVAR, \
  &&op_SETCVAR, \
  &&op_GETGLOBAL, \
  &&op_SETGLOBAL
  
#endif

#endif /* _OPCODE_H_ */
