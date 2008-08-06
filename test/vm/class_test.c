#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

static OBJ lol()
{
  return TR_TRUE;
}

static OBJ cat_new(VM)
{
  OBJ c = tr_class_new(vm, "Cat", TR_NIL);
  
  tr_def(c, "def", lol, 0);
  tr_metadef(c, "metadef", lol, 0);
  
  return c;
}

void test_send_to_class()
{
  tr_vm  vm;
  OBJ    obj;
  OBJ    argv[0];
  
  tr_init(&vm);
  
  obj = tr_new(cat_new(&vm));
  
  assert_equal(TR_TRUE, tr_send(obj, tr_intern("def"), 0, argv));
}

void test_send_to_metaclass()
{
  tr_vm  vm;
  OBJ    obj;
  OBJ    argv[0];
  
  tr_init(&vm);
  
  obj = cat_new(&vm);
  
  assert_equal(TR_TRUE, tr_send(obj, tr_intern("metadef"), 0, argv));
}

void test_send_not_found()
{
  tr_vm  vm;
  OBJ    argv[0];
  
  tr_init(&vm);
  
  assert_equal(TR_UNDEF, tr_send(cat_new(&vm), tr_intern("not_found"), 0, argv));  
}

TEST_START;
  test_send_to_class();
  test_send_to_metaclass();
  test_send_not_found();
TEST_END;