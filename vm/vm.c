#include "tinyrb.h"

static tr_send(const char *method, tr_sf *sf)
{
  /* TODO support multiple args */
  OBJECT arg = (OBJECT) sf->stack->items;
  
  /* HACK yeah ok... this is just proof of concept */
  if (strcmp(method, "puts") == 0)
    printf("%s\n", arg);
  else if (strcmp(method, "print") == 0)
    printf("%s", arg);
}

int tr_exec_inst(tr_sf *sf, tr_inst *inst)
{
  void *ptr;
  
  switch (inst->code) {
    case PUTNIL:
      /* TODO reset stack */
      break;
    case PUTSTRING:
      ptr = array_push(sf->stack);
      memcpy(ptr, (void *) inst->ops[0], sizeof(OBJECT));
      break;
    case POP:
      sf->stack->nitems--;
      break;
    case SEND:
      tr_send((char *) inst->ops[0], sf);
      break;
    case LEAVE:
      /* TODO clear stack */
      break;
    default:
      fprintf(stderr, "unsupported instruction: %d\n", inst->code);
      return TRB_ERROR;
  }
  
  return TRB_OK;
}

int tr_exec_insts(tr_inst *insts, size_t n)
{
  size_t   i;
  tr_sf sf;

  sf.sp = 0;
  sf.stack = array_create(5, sizeof(OBJECT));
  
  for (i = 0; i < n; ++i) {
    tr_exec_inst(&sf, &(insts[i]));
  }
  
  return TRB_OK;
}
