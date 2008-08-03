#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_set(void)
{
  tr_hash *h;
  char    *k = "key", *v = "val";
  
  h = tr_hash_new(5);
  
  tr_hash_set(h, (void*)k, (void*)v);
  
  assert_equal(v, tr_hash_get(h, (void*)k));
}

TEST_START;
  test_set();
TEST_END;