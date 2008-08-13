#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_isfixnum()
{
  SETUP_VM;
  OBJ n = tr_fixnum_new(vm, 10);
  
  assert_equal(TR_FIXNUM, TR_TYPE(n));
}

void test_fixval()
{
  SETUP_VM;
  OBJ n = tr_fixnum_new(vm, 52);
  
  assert_equal(52, TR_FIX(n));
}

void test_fixnum_to_s()
{
  SETUP_VM;
  OBJ n = tr_fixnum_new(vm, 52);
  OBJ argv[0];
  OBJ v = tr_send(vm, n, tr_intern(vm, "to_s"), 0, argv, TR_NIL);
  
  assert_str_equal("52", TR_STR(v));
}

TEST_START;
  test_isfixnum();
  test_fixval();
  test_fixnum_to_s();
TEST_END;