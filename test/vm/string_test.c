#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_str()
{
  OBJ        o = tr_string_new("ohaie");
  tr_string *s = TR_CSTRING(o);
  
  assert_equal(TR_STRING, s->type);
  assert_equal(5, s->len);
  assert_str_equal("ohaie", TR_STR(o));
}

TEST_START;
  test_str();
TEST_END;