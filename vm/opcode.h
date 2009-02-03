#ifndef _OPCODE_H_
#define _OPCODE_H_

/*
== TinyRb opcodes.
Format of one instruction: OPCODE A B
R[A] -- Value of register which index is stored in A of the current instruction.
k[A] -- Value of the constant which index is stored in A of the current instruction.
*/
enum TrOpCode {
  TR_OP_NONE,       /* do nothing with elegance */
  TR_OP_MOVE,       /* copy R[A] into R[B] */
  TR_OP_LOADK,      /* load k[B] into R[A] */
  TR_OP_SEND,       /* send message k[B] to object in R[A], store answer in R[A] */
  TR_OP_JMP,        /* jump A instructions forward */
  TR_OP_JMP_IF,     /* jump B instructions forward if R[A] */
  TR_OP_JMP_UNLESS, /* jump B instructions forward unless R[A] */
  TR_OP_RETURN      /* return R[A] */
};

#ifdef TR_THREADED_DISPATCH
#define TR_OP_LABELS \
  &&op_NONE, \
  &&op_MOVE, \
  &&op_LOADK, \
  &&op_SEND, \
  &&op_JMP, \
  &&op_JMP_IF, \
  &&op_JMP_UNLESS, \
  &&op_RETURN
#endif

#endif /* _OPCODE_H_ */
