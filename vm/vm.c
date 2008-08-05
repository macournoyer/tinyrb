#include "tinyrb.h"

static void tr_op_send(VM, const char *method)
{
  tr_frame *f    = CUR_FRAME;
  int       argc;
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  
  OBJ  arg = tr_array_pop(f->stack);
  OBJ  c   = tr_array_pop(f->stack);
  
  /* TODO support multiple args */
  argc    = 1;
  argv[0] = arg;
  
  tr_send(vm, c, tr_intern(method), argc, argv);
}

void tr_step(VM, tr_op *ops, size_t n)
{
  tr_frame *f = CUR_FRAME;
  tr_op    *op;
  size_t    i;
  
  for (i = 0; i < n; ++i) {
    op = &ops[i];
    switch (op->inst) {
      case GETCONSTANT:
        tr_array_push(f->stack, tr_hash_get(f->consts, tr_intern((char *) op->cmd[0])));
        break;
      case PUTNIL:
        tr_array_push(f->stack, TR_NIL);
        break;
      case PUTSTRING:
        tr_array_push(f->stack, tr_string_new((char *) op->cmd[0]));
        break;
      case POP:
        tr_array_pop(f->stack);
        break;
      case SEND:
        tr_op_send(vm, (char *) op->cmd[0]);
        break;
      case LEAVE:
        return;
      default:
        tr_log("unsupported instruction: %d", op->inst);
    }
  }
}

static void tr_init_frame(tr_frame *f)
{
  f->stack  = tr_array_new();
  f->consts = tr_hash_new();
}

int tr_run(VM, tr_op *ops, size_t n)
{
  tr_frame  *f = CUR_FRAME;
  
  if (f->stack == TR_NIL)
    tr_init_frame(f);
  
  tr_step(vm, ops, n);
  
  return TR_OK;
}

void tr_init(VM)
{
  size_t i;
  
  vm->cf = 0;
  
  for (i = 0; i < TR_MAX_FRAMES; ++i) {
    vm->frames[i].stack  = TR_NIL;
    vm->frames[i].consts = TR_NIL;
  }
  tr_init_frame(CUR_FRAME);
  tr_builtins_add(vm);
}
