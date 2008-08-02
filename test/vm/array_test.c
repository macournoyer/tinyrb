#include "../test.h"
#include "tinyrb.h"

TEST_INIT();

void test_push(void)
{
  tr_array *a;
  char    *c;
  
  a = tr_array_create(10, sizeof(char));
  
  c = tr_array_push(a);
  
  assert_equal(1, a->nitems);
  assert_equal(a->items, c);
  
  c = tr_array_push(a);
  
  assert_equal(2, a->nitems);
  assert_equal(a->items + sizeof(char), c);
  
  tr_array_destroy(a);
}

void test_push_grow_array(void)
{
  tr_array *a;
  
  a = tr_array_create(2, sizeof(char));
  
  tr_array_push(a);
  tr_array_push(a);
  
  assert_equal(2, a->nalloc);
  
  tr_array_push(a);
  
  assert_equal(4, a->nalloc);
  
  tr_array_destroy(a);
}

int main(int argc, char const *argv[])
{
  TEST_START();
  
  test_push();
  test_push_grow_array();
  
  TEST_END();
}