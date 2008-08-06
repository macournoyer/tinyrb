#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_type()
{
  SETUP_VM;
  
  assert_equal(TR_HASH, ((tr_obj *) tr_hash_new(vm))->type);
  assert_equal(TR_ARRAY, ((tr_obj *) tr_array_new(vm))->type);
}

TEST_START;
  test_type();
TEST_END;