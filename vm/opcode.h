#ifndef _OPCODE_H_
#define _OPCODE_H_

typedef struct {
  u8 code:8;
  int a:12;
  int b:12;
} TrOp;

enum TrOpCode {
  TR_OP_NONE,
  TR_OP_MOVE,
  TR_OP_RETURN
}

#endif /* _OPCODE_H_ */
