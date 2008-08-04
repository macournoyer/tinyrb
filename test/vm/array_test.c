#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void array_dump(tr_array *a)
{
  size_t  i;
  
  for (i = 0; i < tr_array_count(a); ++i)
    printf("[%d] %s\n", i, (char *) a->items + i * sizeof(OBJ));
}

void test_push()
{
  tr_array *a;
  OBJ      *o;
  
  a = tr_array_new();
  
  o = tr_array_push(a);
  
  assert_equal(1, tr_array_count(a));
  assert_equal(a->items, o);
  
  o = tr_array_push(a);
  
  assert_equal(2, tr_array_count(a));
  assert_equal(a->items + sizeof(OBJ), o);
  
  tr_array_destroy(a);
}

void test_push_grow_array()
{
  tr_array *a;
  size_t    i;
  
  a = tr_array_new();
  
  for (i = 0; i < 5; ++i)
    tr_array_push(a);
  
  assert_equal(5, a->nalloc);
  
  tr_array_push(a);
  
  assert_equal(10, a->nalloc);
  
  tr_array_destroy(a);
}

void test_pop()
{
  tr_array *a;
  char     *c;
  
  a = tr_array_new();
  
  c = tr_array_push(a); strcpy(c, "1");
  c = tr_array_push(a); strcpy(c, "2");
  
  assert_str_equal("2", (char *) tr_array_pop(a));
  assert_str_equal("1", (char *) tr_array_pop(a));
  assert_equal(NULL, tr_array_pop(a));
  
  tr_array_destroy(a);
}

TEST_START;
  test_push();
  test_push_grow_array();
  test_pop();
TEST_END;