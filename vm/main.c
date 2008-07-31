#include "vm.h"
#include "boot.h"

int main(int argc, char const *argv[])
{
  size_t n = sizeof(trb_boot_insts) / sizeof(trb_inst_t);
  trb_exec_insts(trb_boot_insts, n);
  return 0;
}