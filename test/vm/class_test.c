#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

static OBJ lol(VM)
{
  return TR_TRUE;
}

static OBJ cat_new(VM)
{
  OBJ c = tr_class_new(vm, "Cat", TR_NIL);
  
  tr_def(vm, c, "def", lol, 0);
  tr_metadef(vm, c, "metadef", lol, 0);
  
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

void test_send_not_found()
{
  SETUP_VM;
  OBJ    argv[0];
  
  assert_equal(TR_UNDEF, tr_send(vm, cat_new(vm), tr_intern(vm, "not_found"), 0, argv));  
}

TEST_START;
  test_send_to_class();
  test_send_to_metaclass();
  test_send_not_found();
TEST_END;