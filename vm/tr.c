#include <stdio.h>
#include <sys/stat.h>
#include "tr.h"
#include "internal.h"

static int usage() {
  printf("usage: tinyrb [options] [file]\n"
         "options:\n"
         "  -e   eval code\n"
         "  -d   show debug info (multiple times for more)\n"
         "  -v   print version\n"
         "  -h   print this\n");
  return 1;
}

static int version() {
  printf("tinyrb %s\n", TR_VERSION);
  return 1;
}

static int eval(char *code, char *filename, int verbose) {
  TrVM *vm = TrVM_new();
  TrBlock *b = TrBlock_compile(vm, code, filename, 0, verbose > 1);
  if (verbose) TrBlock_dump(vm, b);
  TrVM_start(vm, b);
  TrVM_destroy(vm);
  return 0;
}

static int eval_file(char *filename, int verbose) {
  FILE *fp;
  struct stat stats;
  
  if (stat(filename, &stats) == -1) {
    perror(filename);
    return 1;
  }
  
  fp = fopen(filename, "rb");
  if (!fp) {
    perror(filename);
    return 1;
  }
  
  char *buf = TR_ALLOC_N(char, stats.st_size + 1);
  
  if (fread(buf, 1, stats.st_size, fp) == stats.st_size) {
    eval(buf, filename, verbose);
  } else {
    perror(filename);
  }
  
  TR_FREE(buf);
  
  return 0;
}

#define OPTION(n) if (strcmp(argv[i], n) == 0)

int main (int argc, char *argv[]) {
  int verbose = 0;
  int i;
  
  if (argc > 1) {
    for (i = 0; i < argc; i++) {
      OPTION("-e") {
        return eval(argv[++i], "<eval>", verbose);
      }
      OPTION("-v") {
        return version();
      }
      OPTION("-d") {
        verbose++;
        continue;
      }
      OPTION("-h") {
        return usage();
      }
    }
    return eval_file(argv[argc-1], verbose);
  }
  
  return usage();
}
