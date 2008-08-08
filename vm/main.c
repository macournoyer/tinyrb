#include "tinyrb.h"
#include "boot.h"

int main(int argc, char const *argv[])
{
  tr_vm  vm;
  
  tr_init(&vm, argc, argv);
  tr_run(&vm, tr_boot_insts, sizeof(tr_boot_insts) / sizeof(tr_op));
  
  return 0;
}