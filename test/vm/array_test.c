#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void dump_array(tr_array *a)
{
  tr_array_entry *e = a->first;
  off_t           i = 0;
  
  printf("count: %d\n", a->count);
  printf("first: %p\n", a->first);
  printf("last:  %p\n", a->last);
  
  while (e != NULL) {
    assert(i < a->count);
    printf("[%d] ", i++);
    printf("%p\n", e);
    printf("    value: %p\n", e->value);
    printf("    prev:  %p\n", e->prev);
    printf("    next:  %p\n", e->next);
    e = e->next;
  }
}

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
}

void test_pop_nil()
{
  SETUP_VM;
  OBJ a;
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, TR_NIL);
  tr_array_pop(vm, a);
  
  assert_equal(0, TR_FIX(tr_array_count(vm, a)));
}

void test_push_special()
{
  SETUP_VM;
  OBJ a;
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, TR_TRUE);
  
  assert_equal(TR_TRUE, tr_array_pop(vm, a));
}

void test_at()
{
  SETUP_VM;
  OBJ a;
  OBJ c1 = tr_string_new(vm, "1");
  OBJ c2 = tr_string_new(vm, "2");
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, c1);
  tr_array_push(vm, a, c2);
  
  assert_equal(c1, tr_array_at(vm, a, 0));
  assert_equal(c2, tr_array_at(vm, a, 1));
  assert_equal(TR_NIL, tr_array_at(vm, a, 2));
}

void test_insert()
{
  SETUP_VM;
  OBJ a;
  OBJ c1 = tr_string_new(vm, "1");
  OBJ c2 = tr_string_new(vm, "2");
  OBJ c3 = tr_string_new(vm, "3");
  
  a = tr_array_new(vm);
  
  tr_array_push(vm, a, c3);
  tr_array_insert(vm, a, 0, c1);
  tr_array_insert(vm, a, 1, c2);
  
  assert_equal(c1, tr_array_at(vm, a, 0));
  assert_equal(c2, tr_array_at(vm, a, 1));
  assert_equal(c3, tr_array_at(vm, a, 2));
}

void test_insert2()
{
  SETUP_VM;
  OBJ a;
  OBJ c1 = tr_fixnum_new(vm, 1);
  OBJ c2 = tr_fixnum_new(vm, 2);
  OBJ c3 = tr_string_new(vm, "3");
  OBJ c4 = tr_string_new(vm, "4");
  
  a = tr_array_new(vm);
  
  tr_array_insert(vm, a, 0, c3);
  tr_array_insert(vm, a, 0, c2);
  tr_array_insert(vm, a, 0, c1);
  tr_array_insert(vm, a, 10, c4);
  
  assert_same(c1, tr_array_at(vm, a, 0));
  assert_same(c2, tr_array_at(vm, a, 1));
  assert_same(c3, tr_array_at(vm, a, 2));
  assert_same(c4, tr_array_at(vm, a, 3));
}

TEST_START;
  test_push_pop();
  test_pop_nil();
  test_push_special();
  test_at();
  test_insert();
  test_insert2();
TEST_END;