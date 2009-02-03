#include <stdio.h>
#include "tr.h"
#include "opcode.h"

int main (int argc, char const *argv[]) {
  TrVM *vm = TrVM_new();
  
  TrCompiler *c = TrCompiler_new(vm, "tr.c");
  tr_compile(vm, c, ":ohaie.inspect\n:ohaie.inspect", 0);
  TrCompiler_dump(c);
  
  tr_run(vm, c->block);
  
  TrVM_destroy(vm);
  
  return 0;
}