#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_set_get()
{
  OBJ h = tr_hash_new();
  OBJ v = tr_string_new("val");
  
  tr_hash_set(h, tr_intern("key"), v);
  
  assert_equal(v, tr_hash_get(h, tr_intern("key")));
}

void test_not_found()
{
  OBJ h = tr_hash_new();
  
  assert_equal(TR_NIL, tr_hash_get(h, tr_intern("ceiling cat")));
}

TEST_START;
  test_set_get();
  test_not_found();
TEST_END;