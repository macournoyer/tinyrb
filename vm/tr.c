#include <stdio.h>
#include "tr.h"
#include "opcode.h"

int main (int argc, char const *argv[]) {
  TrVM vm;
  OBJ k[] = { 1, 2 };
  TrOp code[] = {
    {TR_OP_LOADK, 0, 0},
    {TR_OP_LOADK, 1, 1},
    {TR_OP_ADD, 0, 1},
    {TR_OP_PRINT, 0},
    {TR_OP_RETURN, 0}
  };
  TrBlock block;
  block.k = k;
  block.code = code;
  
  tr_run(&vm, &block);
  
  return 0;
}