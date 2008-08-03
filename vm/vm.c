#include "tinyrb.h"

static void tr_send(VM, const char *method)
{
  tr_frame *f    = CUR_FRAME;
  int       argc = 1; /* TODO guess from stack size */
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  
  /* TODO support multiple args */
  /* HACK Kernel hardcoded omgomg!!!! */
  OBJ c   = (OBJ) tr_hash_get(f->consts, "Kernel");
  OBJ arg = (OBJ) f->stack->items + sizeof(OBJ);
  
  argv[0] = arg;

  tr_call(vm, c, method, argc, argv);
}

int tr_step(VM, tr_op *op)
{
  tr_frame *f = CUR_FRAME;
  void     *ptr, *c;
  
  switch (op->inst) {
    case GETCONSTANT:
      c = tr_hash_get(f->consts, (void *) op->cmd[0]);
      ptr = tr_array_push(f->stack);
      memcpy(ptr, c, sizeof(OBJ));
      break;
    case PUTNIL:
      /* TODO reset stack */
      break;
    case PUTSTRING:
      ptr = tr_array_push(f->stack);
      memcpy(ptr, (void *) op->cmd[0], sizeof(OBJ));
      break;
    case POP:
      f->stack->nitems--;
      break;
    case SEND:
      tr_send(vm, (char *) op->cmd[0]);
      break;
    case LEAVE:
      /* TODO clear stack */
      break;
    default:
      tr_log("unsupported instruction: %d\n", op->inst);
      return TR_ERROR;
  }
  
  return TR_OK;
}

static void tr_init_frame(tr_frame *f)
{
  f->stack  = tr_array_create(5, sizeof(OBJ));
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
