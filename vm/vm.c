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
  
  if (obj == TR_NIL)
    obj = CUR_FRAME->cur_obj;
  
  return tr_send(vm, obj, tr_intern(vm, method), argc, argv);
}

#define STACK_PUSH(o)  tr_array_push(vm, f->stack, (o))
#define STACK_POP()    tr_array_pop(vm, f->stack)
#define JUMP_TO(label) ip = (int) tr_hash_get(vm, label2ip, tr_intern(vm, label));

void tr_step(VM, tr_op *ops, size_t n)
{
  tr_frame *f = CUR_FRAME;
  tr_op    *op;
  size_t    ip;
  OBJ       label2ip = tr_hash_new(vm);
  OBJ       val;
  
  /* store labels=>ip mapping for later jumping */
  for (ip = 0; ip < n; ++ip) {
    op = &ops[ip];
    if (op->inst == LABEL)
      tr_hash_set(vm, label2ip, tr_intern(vm, op->cmd[0]), (OBJ) ip);
  }
  
  for (ip = 0; ip < n; ++ip) {
    op = &ops[ip];
    switch (op->inst) {
      /* nop */
      case NOP:
        break;
      
      /* variable */
      case GETLOCAL:
        STACK_PUSH(tr_hash_get(vm, f->locals, tr_fixnum_new(vm, (int) op->cmd[0])));
        break;
      case SETLOCAL:
        tr_hash_set(vm, f->locals, tr_fixnum_new(vm, (int) op->cmd[0]), STACK_POP());
        break;
      case GETCONSTANT:
        STACK_PUSH(tr_const_get(vm, (char *) op->cmd[0]));
        break;
      
      /* put */
      case PUTNIL:
        STACK_PUSH(TR_NIL);
        break;
      case PUTSTRING:
        STACK_PUSH(tr_string_new(vm, (char *) op->cmd[0]));
        break;
      case PUTOBJECT:
        /* TODO can be other then fixnum probly */
        STACK_PUSH(tr_fixnum_new(vm, (int) op->cmd[0]));
        break;
      
      /* stack */
      case POP:
        STACK_POP();
        break;
      
      /* method */
      case SEND:
        STACK_PUSH(tr_vm_send(vm, (char *) op->cmd[0], (int) op->cmd[1]));
        break;
      case LEAVE:
        return;
      
      /* jump */
      case JUMP:
        JUMP_TO(op->cmd[0]);
        break;
      case BRANCHUNLESS:
        val = tr_array_pop(vm, f->stack);
        if (val == TR_NIL || val == TR_FALSE)
          JUMP_TO(op->cmd[0]);
        break;
      case BRANCHIF:
        val = tr_array_pop(vm, f->stack);
        if (val != TR_NIL && val != TR_FALSE)
          JUMP_TO(op->cmd[0]);
        break;
      
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
  
  if (f->stack == TR_NIL) {
    tr_init_frame(vm, f);
    /* TODO default cur_obj should be Object when Module.include is implemented */
    f->cur_obj = tr_new(vm, tr_const_get(vm, "Kernel"));    
  }
  
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
