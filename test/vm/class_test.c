#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

static OBJ cat_lol(VM, OBJ self)
{
  return TR_TRUE;
}

static OBJ cat_args(VM, OBJ self, OBJ other)
{
  return other;
}

static OBJ cat_var_args(VM, OBJ self, int argc, OBJ argv[])
{
  return tr_fixnum_new(vm, argc);
}

static OBJ cat_new(VM)
{
  OBJ c = tr_class_new(vm, "Cat", tr_const_get(vm, "Object"));
  
  tr_def(vm, c, "def", cat_lol, 0);
  tr_metadef(vm, c, "metadef", cat_lol, 0);
  tr_metadef(vm, c, "args", cat_args, 1);
  tr_metadef(vm, c, "varargs", cat_var_args, -1);
  
  return c;
}

void test_send_to_class()
{
  SETUP_VM;
  OBJ    obj, ret;
  OBJ    argv[0];
  
  obj = tr_new2(vm, cat_new(vm));
  ret = tr_send(vm, obj, tr_intern(vm, "def"), 0, argv, TR_NIL);
  
  assert_equal(TR_TRUE, ret);
}

void test_send_to_metaclass()
{
  SETUP_VM;
  OBJ    obj, ret;
  OBJ    argv[0];
  
  obj = cat_new(vm);
  ret = tr_send(vm, obj, tr_intern(vm, "metadef"), 0, argv, TR_NIL);
  
  assert_equal(TR_TRUE, ret);
}

void test_send_multi_args()
{
  SETUP_VM;
  OBJ argv[] = { TR_FALSE };
  OBJ ret = tr_send(vm, cat_new(vm), tr_intern(vm, "args"), 1, argv, TR_NIL);
  
  assert_equal(TR_FALSE, ret);
}

void test_send_var_args()
{
  SETUP_VM;
  OBJ argv[] = { TR_FALSE };
  OBJ ret = tr_send(vm, cat_new(vm), tr_intern(vm, "varargs"), 1, argv, TR_NIL);
  
  assert_equal(1, TR_FIX(ret));
}

TEST_START;
  test_send_to_class();
  test_send_to_metaclass();
  test_send_multi_args();
  test_send_var_args();
TEST_END;