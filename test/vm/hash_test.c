#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_set_get_string()
{
  SETUP_VM;
  OBJ h = tr_hash_new(vm);
  OBJ v = tr_string_new(vm, "val");
  
  tr_hash_set(vm, h, tr_intern(vm, "key"), v);
  
  assert_equal(v, tr_hash_get(vm, h, tr_intern(vm, "key")));
}

void test_set_get_fixnum()
{
  SETUP_VM;
  OBJ h = tr_hash_new(vm);
  OBJ v = tr_string_new(vm, "val");
  
  tr_hash_set(vm, h, tr_fixnum_new(vm, 123), v);
  
  assert_equal(v, tr_hash_get(vm, h, tr_fixnum_new(vm, 123)));
}

void test_not_found()
{
  SETUP_VM;
  OBJ h = tr_hash_new(vm);
  
  assert_equal(TR_NIL, tr_hash_get(vm, h, tr_intern(vm, "ceiling cat")));
}

TEST_START;
  test_set_get_string();
  test_set_get_fixnum();
  test_not_found();
TEST_END;