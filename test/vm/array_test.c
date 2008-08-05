#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_push_pop()
{
  OBJ a;
  OBJ c1 = tr_string_new("1");
  OBJ c2 = tr_string_new("2");
  
  a = tr_array_new();
  
  tr_array_push(a, c1);
  tr_array_push(a, c2);
  
  assert_equal(2, tr_array_count(a));
  
  assert_equal(c2, tr_array_pop(a));
  assert_equal(c1, tr_array_pop(a));
  assert_equal(TR_NIL, tr_array_pop(a));
  
  tr_array_destroy(a);
}

void test_push_special()
{
  OBJ a;
  
  a = tr_array_new();
  
  tr_array_push(a, TR_TRUE);
  
  assert_equal(TR_TRUE, tr_array_pop(a));
  
  tr_array_destroy(a);
}

void test_push_grow_array()
{
  OBJ    a;
  OBJ    x = tr_string_new("bai");
  size_t i;
  
  a = tr_array_new();
  
  for (i = 0; i < 5; ++i)
    tr_array_push(a, x);
  
  assert_equal(5, TR_CARRAY(a)->nalloc);
  
  tr_array_push(a, x);
  
  assert_equal(10, TR_CARRAY(a)->nalloc);
  
  tr_array_destroy(a);
}

TEST_START;
  test_push_pop();
  test_push_grow_array();
  test_push_special();
TEST_END;