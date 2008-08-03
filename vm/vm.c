#include "tinyrb.h"

static void tr_send(VM, const char *method)
{
  tr_frame *f    = CUR_FRAME;
  int       argc;
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  
  OBJ  arg = (OBJ) tr_array_pop(f->stack);
  OBJ  *c  = (OBJ *) tr_array_pop(f->stack);
  
  /* TODO support multiple args */
  argc    = 1;
  argv[0] = arg;
  
  tr_call(vm, *c, method, argc, argv);
}

int tr_step(VM, tr_op *op)
{
  tr_frame *f = CUR_FRAME;
  void     *c;
  
  switch (op->inst) {
    case GETCONSTANT:
      c = tr_hash_get(f->consts, (void *) op->cmd[0]);
      memcpy(tr_array_push(f->stack), &c, sizeof(OBJ));
      break;
    case PUTNIL:
      c = (void *) TR_NIL; /* huh? should be a pointer? */
      memcpy(tr_array_push(f->stack), &c, sizeof(OBJ));
      break;
    case PUTSTRING:
      memcpy(tr_array_push(f->stack), (void *) op->cmd[0], sizeof(OBJ));
      break;
    case POP:
      tr_array_pop(f->stack);
      break;
    case SEND:
      tr_send(vm, (char *) op->cmd[0]);
      break;
    case LEAVE:
      break;
    default:
      tr_log("unsupported instruction: %d\n", op->inst);
      return TR_ERROR;
  }
  
  return TR_OK;
}

static void tr_init_frame(tr_frame *f)
{
  f->stack  = tr_array_new(5, sizeof(OBJ));
  f->consts = tr_hash_new();
}

int tr_run(VM, tr_op *ops, size_t n)
{
  size_t     i;
  tr_frame  *f = CUR_FRAME;
  
  if (f->stack == NULL)
    tr_init_frame(f);
  
  for (i = 0; i < n; ++i)
    tr_step(vm, &(ops[i]));
  
  return TR_OK;
}

void tr_init(VM)
{
  size_t i;
  
  vm->cf = 0;
  
  for (i = 0; i < TR_MAX_FRAMES; ++i) {
    vm->frames[i].stack  = NULL;
    vm->frames[i].consts = NULL;
  }
  tr_init_frame(CUR_FRAME);
  tr_builtins_add(vm);
}
