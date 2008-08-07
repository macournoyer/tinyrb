#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

static OBJ lol(VM, OBJ self)
{
  return TR_TRUE;
}

static OBJ return_other(VM, OBJ self, OBJ other)
{
  return other;
}

static OBJ cat_new(VM)
{
  OBJ c = tr_class_new(vm, "Cat", TR_NIL);
  
  tr_def(vm, c, "def", lol, 0);
  tr_metadef(vm, c, "metadef", lol, 0);
  tr_metadef(vm, c, "args", return_other, 1);
  
  return c;
}

void test_send_to_class()
{
  SETUP_VM;
  OBJ    obj;
  OBJ    argv[0];
  
  obj = tr_new(vm, cat_new(vm));
  
  assert_equal(TR_TRUE, tr_send(vm, obj, tr_intern(vm, "def"), 0, argv));
}

void test_send_to_metaclass()
{
  SETUP_VM;
  OBJ    obj;
  OBJ    argv[0];
  
  obj = cat_new(vm);
  
  assert_equal(TR_TRUE, tr_send(vm, obj, tr_intern(vm, "metadef"), 0, argv));
}

void test_send_multi_args()
{
  SETUP_VM;
  OBJ    argv[] = { TR_FALSE };
  
  assert_equal(TR_FALSE, tr_send(vm, cat_new(vm), tr_intern(vm, "args"), 1, argv));
}

TEST_START;
  test_send_to_class();
  test_send_to_metaclass();
  test_send_multi_args();
TEST_END;