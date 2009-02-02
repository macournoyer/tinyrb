#include <stdio.h>
#include <assert.h>
#include "tr.h"
#include "opcode.h"

/* dispatch macros */
#define NEXT_OP        (++ip, e=*ip)
#ifdef TR_THREADED_DISPATCH
#define OPCODES        goto *labels[e.i]
#define END_OPCODES    
#define OP(name)       op_##name
#define DISPATCH       NEXT_OP; goto *labels[e.i]
#else
#define OPCODES        for(;;) { switch(e.i) {
#define END_OPCODES    default: printf("unknown opcode: %d\n", (int)e.i); }}
#define OP(name)       case TR_OP_##name
#define DISPATCH       NEXT_OP; break
#endif

/* register access macros */
#define VA   ((int)e.regs.a)
#define VB   ((int)e.regs.b)
#define VC   ((int)e.regs.c)
#define RA   regs[e.regs.a]
#define RB   regs[e.regs.b]
#define RC   regs[e.regs.c]
#define UVBC (unsigned short)(((VB<<8)+VC))
#define SVBC (short)(((VB<<8)+VC))

OBJ tr_run(VM, TrOp *code) {
  TrOp *ip = code;
  TrOp e = *ip;
  OBJ regs[10];
  /* TrFrame *frame = FRAME; */
  
#ifdef TR_THREADED_DISPATCH
  static void *labels[] = { TR_OP_LABELS };
#endif
  
  OPCODES;
    OP(NONE):     DISPATCH;
    OP(MOVE):     RA = RB; DISPATCH;
    OP(ADD):      RA = RB + RC; DISPATCH;
    OP(PRINT):    printf("ok\n"); DISPATCH;
    OP(RETURN):   return 0;
  END_OPCODES;
}
