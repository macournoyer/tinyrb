#include "tinyrb.h"

static OBJ tr_vm_send(VM, const char *method, int argc)
{
  tr_frame *f    = CUR_FRAME;
  size_t    i;
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  OBJ       obj;
  
  for(i = 0; i < argc; ++i)
    argv[i] = tr_array_pop(vm, f->stack);
  obj = tr_array_pop(vm, f->stack);
  
  return tr_send(vm, obj, tr_intern(vm, method), argc, argv);
}

void tr_step(VM, tr_op *ops, size_t n)
{
  tr_frame *f = CUR_FRAME;
  tr_op    *op;
  size_t    i;
  OBJ       label2i = tr_hash_new(vm);
  
  /* store labels=>i mapping for later jumping */
  for (i = 0; i < n; ++i) {
    op = &ops[i];
    if (op->inst == LABEL)
      tr_hash_set(vm, label2i, tr_intern(vm, op->cmd[0]), (OBJ) i);
  }
  
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
        tr_array_push(vm, f->stack, tr_const_get(vm, (char *) op->cmd[0]));
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
        tr_array_push(vm, f->stack, tr_vm_send(vm, (char *) op->cmd[0], (int) op->cmd[1]));
        break;
      case JUMP:
        i = (int) tr_hash_get(vm, label2i, tr_intern(vm, op->cmd[0]));
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

void tr_raise(VM, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "Exception: ");
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(-1);
}

void tr_const_set(VM, const char *name, OBJ obj)
{
  tr_hash_set(vm, CUR_FRAME->consts, tr_intern(vm, name), obj);
}

OBJ tr_const_get(VM, const char *name)
{
  OBJ c = tr_hash_get(vm, CUR_FRAME->consts, tr_intern(vm, name));
  
  if (c == TR_NIL)
    tr_raise(vm, "Constant not found: %s", name);
  
  return c;
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
  tr_class_init(vm);
  tr_object_init(vm);
  
  OBJ class  = tr_const_get(vm, "Class");
  OBJ object = tr_const_get(vm, "Object");
  OBJ module = tr_class_new(vm, "Module", tr_const_get(vm, "Object"));
  
  /* Since Class was not defined yet, have to do it manually */
  TR_CCLASS(class)->super  = TR_CCLASS(module);
  TR_COBJ(object)->class   =
  TR_COBJ(module)->class   =
  TR_COBJ(class)->class    = TR_CCLASS(class);
  
  tr_kernel_init(vm);
  tr_string_init(vm);
  tr_fixnum_init(vm);
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
