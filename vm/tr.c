#include <stdio.h>
#include "tr.h"
#include "opcode.h"

int main (int argc, char const *argv[]) {
  TrVM *vm = TrVM_new();

  /* OBJ k[] = {
    tr_intern("ohaie"),
    tr_intern("inspect"),
  };
  TrOp code[] = {
    {TR_OP_LOADK,  0, 0},
    {TR_OP_SEND,   0, 1},
    {TR_OP_RETURN, 0, 0}
  };
  TrBlock block;
  block.k = k;
  block.code = code; */
  
  TrCompiler *c = TrCompiler_new(vm, "tr.c");
  tr_compile(vm, c, "inspect", 0);
  TrCompiler_dump(c);
  
  /* tr_run(vm, c->block); */
  
  TrVM_destroy(vm);
  
  return 0;
}