#ifndef _TINYRB_H_
#define _TINYRB_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "inst.h"

#define TRB_ERROR -1
#define TRB_OK     0

typedef unsigned long OBJECT;

/* instruction */
typedef struct tr_inst {
  tr_inst_e code;
  OBJECT    ops[5];
} tr_inst ;

/* stack frame */
typedef struct tr_sf {
  off_t    sp;
  array_t *stack;
} tr_sf;

int tr_exec_inst(tr_sf *sf, tr_inst *inst);
int tr_exec_insts(tr_inst *insts, size_t n);

#endif /* _TINYRB_H_ */
