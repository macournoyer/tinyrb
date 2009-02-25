#include "tr.h"

static int usage() {
  printf("usage: tinyrb [options] [file]\n"
         "options:\n"
         "  -e   eval code\n"
         "  -d   show debug info (multiple times for more)\n"
         "  -v   print version\n"
         "  -h   print this\n");
  return 1;
}

#define OPTION(n) if (strcmp(argv[i], n) == 0)

int main (int argc, char *argv[]) {
  int i;
  TrVM *vm = TrVM_new();
  
  if (argc > 1) {
    for (i = 0; i < argc; i++) {
      OPTION("-e") {
        TrVM_eval(vm, argv[++i], "<eval>");
        return 0;
      }
      OPTION("-v") {
        printf("tinyrb %s\n", TR_VERSION);
        return 1;
      }
      OPTION("-d") {
        vm->debug++;
        continue;
      }
      OPTION("-h") {
        return usage();
      }
    }
    TrVM_load(vm, argv[argc-1]);
    return 0;
  }
  
  TrVM_destroy(vm);
  
  return usage();
}
