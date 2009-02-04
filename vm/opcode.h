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
  TR_OP_NONE,       /* do nothing with elegance and frivolity */
  TR_OP_MOVE,       /* R[A] = R[B]  */
  TR_OP_LOADK,      /* R[A] = K[Bx] */
  TR_OP_SEND,       /* send message K[B] to R[A] w/ C args, store answer in R[A] */
  TR_OP_JMP,        /* jump sBx instructions */
  TR_OP_JMPIF,      /* jump sBx instructions if R[A] */
  TR_OP_JMPUNLESS,  /* jump sBx instructions unless R[A] */
  TR_OP_RETURN,     /* return R[A] */
  TR_OP_SETLOCAL,   /* locals[A] = R[B] */
  TR_OP_GETLOCAL,   /* R[A] = locals[B] */
  TR_OP_GETDYN,
  TR_OP_SETDYN,
  TR_OP_GETCONST,
  TR_OP_SETCONST,
  TR_OP_GETIVAR,
  TR_OP_SETIVAR,
  TR_OP_GETCVAR,
  TR_OP_SETCVAR,
  TR_OP_GETGLOBAL,
  TR_OP_SETGLOBAL,
  TR_OP_NIL,
  TR_OP_SELF,
  TR_OP_NEWARRAY,
  TR_OP_NEWHASH,
  TR_OP_NEWRANGE,
  TR_OP_DEF,
  TR_OP_UNDEF,
  TR_OP_ALIAS,
  TR_OP_CLASS,
  TR_OP_SUPER,
  TR_OP_YIELD
};

#define TR_OP_NAMES \
  "none", \
  "move", \
  "loadk", \
  "send", \
  "jmp", \
  "jmpif", \
  "jmpunless", \
  "return", \
  "setlocal", \
  "getlocal", \
  "getdyn", \
  "setdyn", \
  "getconst", \
  "setconst", \
  "getivar", \
  "setivar", \
  "getcvar", \
  "setcvar", \
  "getglobal", \
  "setglobal", \
  "nil", \
  "self", \
  "newarray", \
  "newhash", \
  "newrange", \
  "def", \
  "undef", \
  "alias", \
  "class", \
  "super", \
  "yield"

#ifdef TR_THREADED_DISPATCH
#define TR_OP_LABELS \
  &&op_NONE, \
  &&op_MOVE, \
  &&op_LOADK, \
  &&op_SEND, \
  &&op_JMP, \
  &&op_JMPIF, \
  &&op_JMPUNLESS, \
  &&op_RETURN, \
  &&op_SETLOCAL, \
  &&op_GETLOCAL
#endif

#endif /* _OPCODE_H_ */
