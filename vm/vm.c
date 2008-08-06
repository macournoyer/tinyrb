#include "tinyrb.h"

static OBJ tr_vm_send(VM, const char *method)
{
  tr_frame *f    = CUR_FRAME;
  int       argc;
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  
  OBJ  arg = tr_array_pop(vm, f->stack);
  OBJ  obj = tr_array_pop(vm, f->stack);
  
  /* TODO support multiple args */
  argc    = 1;
  argv[0] = arg;
  
  return tr_send(vm, obj, tr_intern(vm, method), argc, argv);
}

void tr_step(VM, tr_op *ops, size_t n)
{
  tr_frame *f = CUR_FRAME;
  tr_op    *op;
  size_t    i;
  
  for (i = 0; i < n; ++i) {
    op = &ops[i];
    switch (op->inst) {
      case GETLOCAL:
        tr_array_push(vm, f->stack, tr_hash_get(vm, f->locals, tr_fixnum_new(vm, (int) op->cmd[0])));
        break;
      case SETLOCAL:
        tr_hash_set(vm, f->locals, tr_fixnum_new(vm, (int) op->cmd[0]), tr_array_pop(vm, f->stack));
        break;
      case GETCONSTANT:
        tr_array_push(vm, f->stack, tr_hash_get(vm, f->consts, tr_intern(vm, (char *) op->cmd[0])));
        break;
      case PUTNIL:
        tr_array_push(vm, f->stack, TR_NIL);
        break;
      case PUTSTRING:
        tr_array_push(vm, f->stack, tr_string_new(vm, (char *) op->cmd[0]));
        break;
      case PUTOBJECT:
        /* TODO can be other then fixnum probly */
        tr_array_push(vm, f->stack, tr_fixnum_new(vm, (int) op->cmd[0]));
        break;
      case POP:
        tr_array_pop(vm, f->stack);
        break;
      case SEND:
        tr_array_push(vm, f->stack, tr_vm_send(vm, (char *) op->cmd[0]));
        break;
      case LEAVE:
        return;
      default:
        tr_log("unsupported instruction: %d", op->inst);
    }
  }
}

static void tr_init_frame(VM, tr_frame *f)
{
  f->stack  = tr_array_new(vm);
  f->consts = tr_hash_new(vm);
  f->locals = tr_hash_new(vm);
}

int tr_run(VM, tr_op *ops, size_t n)
{
  tr_frame  *f = CUR_FRAME;
  
  if (f->stack == TR_NIL)
    tr_init_frame(vm, f);
  
  tr_step(vm, ops, n);
  
  return TR_OK;
}

static tr_define_builtins(VM)
{
  OBJ object = tr_const_get(vm, "Object"); /* TODO */
  OBJ module = tr_class_new(vm, "Module", object);
  OBJ class  = tr_class_new(vm, "Class", module);
  
  /* Since Class was not defined yet, have to do it manually */
  /* TR_COBJ(object)->class = */
  TR_COBJ(module)->class =
  TR_COBJ(class)->class  = TR_CCLASS(class);
  
  tr_kernel_init(vm);
}

void tr_init(VM)
{
  size_t i;
  
  vm->cf = 0;
  
  for (i = 0; i < TR_MAX_FRAMES; ++i) {
    vm->frames[i].stack  = TR_NIL;
    vm->frames[i].consts = TR_NIL;
  }
  tr_init_frame(vm, CUR_FRAME);
  tr_define_builtins(vm);
}
