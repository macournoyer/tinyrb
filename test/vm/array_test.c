#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void array_dump(tr_array *a)
{
  size_t  i;
  
  for (i = 0; i < a->nitems; ++i)
    printf("[%d] %s\n", i, (char *) a->items + i * a->size);
}

void test_push()
{
  tr_array *a;
  char     *c;
  
  a = tr_array_new(10, sizeof(char));
  
  c = tr_array_push(a);
  
  assert_equal(1, a->nitems);
  assert_equal(a->items, c);
  
  c = tr_array_push(a);
  
  assert_equal(2, a->nitems);
  assert_equal(a->items + sizeof(char), c);
  
  tr_array_destroy(a);
}

void test_push_grow_array()
{
  tr_array *a;
  
  a = tr_array_new(2, sizeof(char));
  
  tr_array_push(a);
  tr_array_push(a);
  
  assert_equal(2, a->nalloc);
  
  tr_array_push(a);
  
  assert_equal(4, a->nalloc);
  
  tr_array_destroy(a);
}

void test_pop()
{
  tr_array *a;
  char     *c;
  
  a = tr_array_new(2, sizeof(c));
  
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