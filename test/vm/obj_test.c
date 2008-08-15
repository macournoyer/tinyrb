#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_type()
{
  SETUP_VM;
  OBJ v;
  
  v = tr_hash_new(vm);
  assert_equal(TR_HASH, TR_TYPE(v));
  v = tr_array_new(vm);
  assert_equal(TR_ARRAY, TR_TYPE(v));
}

void test_send_to_superclass()
{
  SETUP_VM;
  OBJ str = tr_string_new(vm, "lol");
  OBJ argv[0];
  OBJ v = tr_send(vm, str, tr_intern(vm, "inspect"), 0, argv, TR_NIL);
  
  assert_equal(1, TR_NIL != v);
}

void test_obj_type()
{
  SETUP_VM;
  
  assert_equal(TR_SPECIAL, tr_type_get(TR_NIL));
  assert_equal(TR_SPECIAL, tr_type_get(TR_TRUE));
  assert_equal(TR_SPECIAL, tr_type_get(TR_UNDEF));
  assert_equal(TR_SYMBOL, tr_type_get(tr_intern(vm, "hi")));  
  assert_equal(TR_ARRAY, tr_type_get((OBJ)tr_array_struct(vm)));
}

TEST_START;
  test_type();
  test_send_to_superclass();
  test_obj_type();
TEST_END;