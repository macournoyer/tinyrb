#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_type()
{
  SETUP_VM;
  
  assert_equal(TR_HASH, ((tr_obj *) tr_hash_new(vm))->type);
  assert_equal(TR_ARRAY, ((tr_obj *) tr_array_new(vm))->type);
}

void test_send_to_superclass()
{
  SETUP_VM;
  OBJ str = tr_string_new(vm, "lol");
  OBJ argv[0];
  
  assert_equal(1, TR_NIL != tr_send(vm, str, tr_intern(vm, "inspect"), 0, argv));
}

TEST_START;
  test_type();
  test_send_to_superclass();
TEST_END;