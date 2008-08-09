#include "tinyrb.h"

/* #define TRACE_STACK */

static OBJ tr_vm_send(VM, const char *method, int argc)
{
  tr_frame *f    = CUR_FRAME;
  size_t    i;
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  OBJ       obj;
  
  for (i = argc; i > 0; --i)
    argv[i-1] = tr_array_pop(vm, f->stack);
  obj = tr_array_pop(vm, f->stack);
  
  return tr_send(vm, obj, tr_intern(vm, method), argc, argv);
}

static int tr_vm_branch(VM, int b, OBJ val)
{
  if (b)
    return val != TR_NIL && val != TR_FALSE; /* if */
  return val == TR_NIL || val == TR_FALSE;   /* unless */
}

static void tr_dump_stack(VM)
{
  size_t    i;
  tr_array *a = TR_CARRAY(CUR_FRAME->stack);
  OBJ      *o;
  
  for (i = 0; i < a->count; ++i) {
    o = (OBJ *) a->items + i * sizeof(OBJ *);
    printf("    [%d] %p\n", i, *o);
  }
}

#define STACK_PUSH(o)  tr_array_push(vm, f->stack, (o))
#define STACK_POP()    tr_array_pop(vm, f->stack)
#define JUMP_TO(label) ip = (int) tr_hash_get(vm, label2ip, tr_intern(vm, label));

OBJ tr_run(VM, tr_op *ops, size_t n)
{
  tr_frame *f = CUR_FRAME;
  tr_op    *op;
  size_t    ip;
  OBJ       label2ip = tr_hash_new(vm);
  
  /* store labels=>ip mapping for later jumping */
  for (ip = 0; ip < n; ++ip) {
    op = &ops[ip];
    if (op->inst == LABEL)
      tr_hash_set(vm, label2ip, tr_intern(vm, op->cmd[0]), (OBJ) ip);
  }
  
  for (ip = 0; ip < n; ++ip) {
    op = &ops[ip];
    f->line = op->line;
    
    #ifdef TRACE_STACK
    if ((int) op->cmd[0] > 100)
      printf("[%d] %d, %s, %d, %d\n", ip, op->inst, op->cmd[0], op->cmd[1], op->cmd[2]);
    else
      printf("[%d] %d, %d, %d, %d\n", ip, op->inst, op->cmd[0], op->cmd[1], op->cmd[2]);
    #endif
    
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
      case SETINSTANCEVARIABLE:
        tr_hash_set(vm, TR_COBJ(f->self)->ivars, tr_intern(vm, (char *) op->cmd[0]), STACK_POP());
        break;
      case GETINSTANCEVARIABLE:
        STACK_PUSH(tr_hash_get(vm, TR_COBJ(f->self)->ivars, tr_intern(vm, (char *) op->cmd[0])));
        break;
      case SETCLASSVARIABLE:
        tr_hash_set(vm, TR_COBJ(f->class)->ivars, tr_intern(vm, (char *) op->cmd[0]), STACK_POP());
        break;
      case GETCLASSVARIABLE:
        STACK_PUSH(tr_hash_get(vm, TR_COBJ(f->class)->ivars, tr_intern(vm, (char *) op->cmd[0])));
        break;
      case GETCONSTANT:
        STACK_POP(); /* TODO class */
        STACK_PUSH(tr_const_get(vm, (char *) op->cmd[0]));
        break;
      case NEWARRAY:
        /* TODO init items argc = op->cmd[0] */
        STACK_PUSH(tr_array_new(vm));
        break;
      
      /* put */
      case PUTNIL:
        STACK_PUSH(TR_NIL);
        break;
      case PUTSTRING:
        STACK_PUSH(tr_string_new(vm, (char *) op->cmd[0]));
        break;
      case PUTOBJECT:
        switch ((OBJ) op->cmd[1]) {
          case TR_SPECIAL:
            STACK_PUSH(tr_special_get(vm, (OBJ) op->cmd[0]));
            break;
          case TR_FIXNUM:
            STACK_PUSH(tr_fixnum_new(vm, (int) op->cmd[0]));
            break;
        }
        break;
      case PUTSELF:
        STACK_PUSH(f->self);
        break;
      
      /* stack */
      case POP:
        STACK_POP();
        break;
      case DUP:
        STACK_PUSH(tr_array_last(vm, f->stack));
        break;
      
      /* method */
      case SEND:
        STACK_PUSH(tr_vm_send(vm, (char *) op->cmd[0], (int) op->cmd[1]));
        break;
      case LEAVE:
        return STACK_POP();
      case DEFINEMETHOD:
        tr_ops_def(vm, f->class,
                       (char *) op->cmd[0],  /* name */
                       (tr_op *) op->cmd[1], /* ops */
                       (int) op->cmd[3]);    /* nops */
        break;
      
      /* class */
      case DEFINECLASS:
        tr_class_define(vm, (char *) op->cmd[0],  /* name */
                            STACK_POP(),          /* cbase */
                            STACK_POP(),          /* super */
                            (tr_op *) op->cmd[1], /* ops */
                            (int) op->cmd[2],     /* define_type */
                            (int) op->cmd[3]);    /* nops */
        break;
      
      /* jump */
      case JUMP:
        JUMP_TO(op->cmd[0]);
        break;
      case BRANCHUNLESS:
        if (tr_vm_branch(vm, 0, STACK_POP()))
          JUMP_TO(op->cmd[0]);
        break;
      case BRANCHIF:
        if (tr_vm_branch(vm, 1, STACK_POP()))
          JUMP_TO(op->cmd[0]);
        break;
      case LABEL:
        break;
      
      default:
        tr_log("unsupported instruction: %d (ip=%d)", op->inst, ip);
    }
    
    #ifdef TRACE_STACK
    tr_dump_stack(vm);
    #endif
  }
}

void tr_raise(VM, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  fprintf(stderr, "Exception: ");
  vfprintf(stderr, msg, args);
  fprintf(stderr, "\n");
  fprintf(stderr, "     from (?):%d\n", CUR_FRAME->line);
  va_end(args);
  exit(-1);
}

void tr_next_frame(VM, OBJ obj, OBJ class)
{
  vm->cf ++;
  #ifdef TRACE_STACK
  printf("moving to frame: %d\n", vm->cf);
  #endif
  tr_frame  *f = CUR_FRAME;
  
  /* TODO ??? f->consts = tr_hash_new(vm); */
  f->consts = vm->frames[vm->cf-1].consts;
  f->stack  = tr_array_new(vm);
  f->locals = tr_hash_new(vm);
  f->self   = obj;
  f->class  = class;
  f->line   = 0;
}

void tr_prev_frame(VM)
{
  vm->cf --;
  #ifdef TRACE_STACK
  printf("back to frame: %d\n", vm->cf);
  #endif
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
  tr_array_init(vm);
  tr_io_init(vm);
  tr_special_init(vm);
}

void tr_init(VM, int argc, char const *argv[])
{
  size_t    i;
  tr_frame *f;
  
  vm->cf = 0;
  
  f = CUR_FRAME;
  f->consts = tr_hash_new(vm);
  f->locals = tr_hash_new(vm);
  
  tr_define_builtins(vm);
  
  f->stack   = tr_array_new(vm);
  f->class   = tr_const_get(vm, "Object");
  f->self    = tr_new(vm, f->class);
  f->line    = 0;
  
  /* init argv */
  OBJ argv_ary = tr_array_new(vm);
  for(i = 0; i < argc; ++i)
    tr_array_push(vm, argv_ary, tr_string_new(vm, argv[i]));
  tr_const_set(vm, "ARGV", argv_ary);
}
