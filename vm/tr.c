#include <stdio.h>
#include "tr.h"

int main (int argc, char const *argv[]) {
  TrVM vm;
  TrOp code[] = {
    {0},
    {3},
    {4}
  };
  
  tr_run(&vm, code);
  
  return 0;
}