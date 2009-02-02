#ifndef _OPCODE_H_
#define _OPCODE_H_

enum TrOpCode {
  TR_OP_NONE,
  TR_OP_MOVE,
  TR_OP_ADD,
  TR_OP_PRINT,
  TR_OP_RETURN
};

#define TR_OP_LABELS \
  &&op_NONE, \
  &&op_MOVE, \
  &&op_ADD, \
  &&op_PRINT, \
  &&op_RETURN

#endif /* _OPCODE_H_ */
