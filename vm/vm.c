#include "tinyrb.h"

static tr_send(const char *method, tr_frame *f)
{
  /* TODO support multiple args */
  OBJ arg = (OBJ) f->stack->items;
  
  /* HACK yeah ok... this is just proof of concept */
  if (strcmp(method, "puts") == 0)
    printf("%s\n", arg);
  else if (strcmp(method, "print") == 0)
    printf("%s", arg);
}

int tr_step(tr_frame *f, tr_op *op)
{
  void *ptr;
  
  switch (op->inst) {
    /* case GETCONSTANT:
      break; */
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
      tr_send((char *) op->cmd[0], f);
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
    tr_step(f, &(ops[i]));
  
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
}
