#include "../test.h"
#include "tinyrb.h"

TEST_INIT;

void test_new(void)
{
  tr_vm      vm;
  tr_module *mod;
  
  tr_init(&vm);
  mod = (tr_module *) tr_module_new(&vm, "Lolcat");
  
  assert_str_equal("Lolcat", mod->name);
  assert_equal(TR_MODULE, mod->type);
  assert_equal(TR_MODULE, ((tr_obj *) mod)->type);
}

static OBJ lol()
{
  return TR_TRUE;
}

void test_def(void)
{
  tr_vm      vm;
  tr_module *mod;
  OBJ        argv[0];
  
  tr_init(&vm);
  mod = (tr_module *) tr_module_new(&vm, "Cat");
  
  tr_def(&vm, (OBJ) mod, "lol", lol, 0);
  
  assert_equal(TR_TRUE, tr_send(&vm, (OBJ) mod, tr_intern("lol"), 0, argv));
}

TEST_START;
  test_new();
  test_def();
TEST_END;