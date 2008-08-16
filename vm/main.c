#include "tinyrb.h"
#include "boot.cx"

int main(int argc, char *argv[])
{
  tr_vm  vm;
  
  tr_init(&vm, argc, argv);
  tr_run(&vm, tr_boot(&vm));
  
  return 0;
}