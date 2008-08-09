#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_push_pop()
{
  SETUP_VM;
  OBJ a;
  OBJ c1 = tr_string_new(vm, "1");
  OBJ c2 = tr_string_new(vm, "2");
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, c1);
  tr_array_push(vm, a, c2);
  
  assert_equal(2, TR_FIX(tr_array_count(vm, a)));
  
  assert_equal(c2, tr_array_pop(vm, a));
  assert_equal(c1, tr_array_last(vm, a));
  assert_equal(c1, tr_array_pop(vm, a));
  assert_equal(TR_NIL, tr_array_pop(vm, a));
  
  tr_array_destroy(vm, a);
}

void test_pop_nil()
{
  SETUP_VM;
  OBJ a;
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, TR_NIL);
  tr_array_pop(vm, a);
  
  assert_equal(0, TR_FIX(tr_array_count(vm, a)));
  
  tr_array_destroy(vm, a);
}

void test_push_special()
{
  SETUP_VM;
  OBJ a;
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, TR_TRUE);
  
  assert_equal(TR_TRUE, tr_array_pop(vm, a));
  
  tr_array_destroy(vm, a);
}

void test_push_grow_array()
{
  SETUP_VM;
  OBJ    a;
  OBJ    x = tr_string_new(vm, "bai");
  size_t i;
  
  a = tr_array_new(vm);
  
  for (i = 0; i < 100; ++i)
    tr_array_push(vm, a, x);
  
  assert_equal(100, TR_CARRAY(a)->nalloc);
  
  tr_array_push(vm, a, x);
  
  assert_equal(200, TR_CARRAY(a)->nalloc);
  
  tr_array_destroy(vm, a);
}

TEST_START;
  test_push_pop();
  test_pop_nil();
  test_push_grow_array();
  test_push_special();
TEST_END;