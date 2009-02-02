#include <stdio.h>
#include <assert.h>
#include "tr.h"

#define POP_OP         (*++ip)
#define LITERAL        literals[ind + POP_OP]
#define STACK_POP      kv_pop(frame->stack)
#define STACK_PUSH(v)  kv_push(OBJ, frame->stack, v)
#define RESET_IND      (ind = 0)

#ifdef MIN_THREADED_DISPATCH
#define OPCODES        goto *labels[*ip]
#define END_OPCODES    
#define OP(name)       op_##name
#define DISPATCH       RESET_IND; goto *labels[POP_OP]
#else
#define OPCODES        for(;;) { switch(*ip) {
#define END_OPCODES    default: printf("unknown opcode: %d\n", (int)*ip); }}
#define OP(name)       case MIN_OP_##name
#define DISPATCH       RESET_IND; POP_OP; break
#endif

/* OBJ tr_run(VM, TrCode *code) {
  MinOpCode *ip = &kv_A(code->opcodes, 0);
  MinOpCode ind = 0;
  OBJ *lit = &kv_A(code->lit, 0);
  struct MinFrame *frame = VM_FRAME;
  
#ifdef MIN_THREADED_DISPATCH
  static void *labels[] = { MIN_OP_LABELS };
#endif
  
  OPCODES;
    OP(SELF):        STACK_PUSH(frame->self); DISPATCH;
    OP(LITERAL):     STACK_PUSH(LITERAL); DISPATCH;
    OP(SELF_SEND):   STACK_PUSH(min_send(frame->self, LITERAL)); DISPATCH;
    OP(SUPER_SEND):  assert(0 && "unimplemented"); DISPATCH;
    OP(RETURN):      return STACK_POP;
    OP(INDEX_EXT):   ind += POP_OP; DISPATCH;
  END_OPCODES;
} */
