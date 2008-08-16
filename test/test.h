#include <stdio.h>

#define TEST_INIT \
  static int assertions; \
  static int failures

#define TEST_START \
  int main(int argc, char *argv[]) { \
  printf("== %s\n", argv[0])

#define TEST_END \
  printf("== %d assertions, %d success, %d failures\n", assertions, assertions - failures, failures); \
  if (failures > 0) { return -1; } else { return 0; } }
  
#define _assert(test, msg, ...) \
  assertions++; \
  if (!(test)) { \
    printf("  %s:%d: " msg "\n", __FUNCTION__, __LINE__, __VA_ARGS__); \
    failures++; \
  }

#define assert_equal(expected, actual) \
  _assert(expected == actual, "%d expected but was %d", expected, actual)

#define assert_same(expected, actual) \
  _assert(expected == actual, "%p expected but was %p", expected, actual)

#define assert_str_equal(expected, actual) \
  _assert(strcmp(expected, actual) == 0, "%s expected but was %s", expected, actual)

#define SETUP_VM tr_vm _vm; tr_vm *vm = &_vm; char *__vm_argv[0]; tr_init(vm, 0, __vm_argv);