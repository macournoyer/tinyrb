#ifndef _VM_H_
#define _VM_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "inst.h"

#define TRB_ERROR -1
#define TRB_OK     0

typedef struct trb_inst_s trb_inst_t;
typedef struct trb_sf_s trb_sf_t;
typedef unsigned long OBJECT;

/* instruction */
struct trb_inst_s {
  trb_inst_e code;
  OBJECT     ops[5];
};

/* stack frame */
struct trb_sf_s {
  off_t    sp;
  array_t *stack;
};

int trb_exec_inst(trb_sf_t *sf, trb_inst_t *inst);
int trb_exec_insts(trb_inst_t *insts, size_t n);

#endif /* _VM_H_ */
