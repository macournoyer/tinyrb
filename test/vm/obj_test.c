#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_type()
{
  assert_equal(TR_HASH, ((tr_obj *) tr_hash_new())->type);
  assert_equal(TR_ARRAY, ((tr_obj *) tr_array_new())->type);
}

TEST_START;
  test_type();
TEST_END;