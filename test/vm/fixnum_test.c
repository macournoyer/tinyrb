#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_isfixnum()
{
  OBJ n = tr_fixnum_new(10);
  
  assert_equal(TR_FIXNUM, TR_TYPE(n));
}

void test_fixval()
{
  OBJ n = tr_fixnum_new(52);
  
  assert_equal(52, TR_FIX(n));
}

TEST_START;
  test_isfixnum();
  test_fixval();
TEST_END;