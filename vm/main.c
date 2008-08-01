#include "tinyrb.h"
#include "boot.h"

int main(int argc, char const *argv[])
{
  size_t n = sizeof(tr_boot_insts) / sizeof(tr_inst);
  tr_exec_insts(tr_boot_insts, n);
  return 0;
}