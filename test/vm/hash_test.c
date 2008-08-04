#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_set_get()
{
  tr_hash *h = tr_hash_new();
  char    *v = "val";
  
  tr_hash_set(h, (void*) "key", (void*)v);
  
  assert_equal(v, tr_hash_get(h, (void*) "key"));
}

void test_not_found()
{
  tr_hash *h = tr_hash_new();
  
  assert_equal(NULL, tr_hash_get(h, (void*) "key"));
}

TEST_START;
  test_set_get();
  test_not_found();
TEST_END;