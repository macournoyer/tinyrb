#include "tinyrb.h"

/* #define TRACE_STACK */

#ifdef TRACE_STACK
const char *tr_inst_names[] = {"NOP","GETLOCAL","SETLOCAL","GETSPECIAL","SETSPECIAL","GETDYNAMIC","SETDYNAMIC",
  "GETINSTANCEVARIABLE","SETINSTANCEVARIABLE","GETCLASSVARIABLE","SETCLASSVARIABLE","GETCONSTANT","SETCONSTANT",
  "GETGLOBAL","SETGLOBAL","PUTNIL","PUTSELF","PUTUNDEF","PUTOBJECT","PUTSTRING","CONCATSTRINGS","TOSTRING",
  "TOREGEXP","NEWARRAY","DUPARRAY","EXPANDARRAY","CONCATARRAY","SPLATARRAY","CHECKINCLUDEARRAY","NEWHASH",
  "NEWRANGE","PUTNOT","POP","DUP","DUPN","SWAP","REPUT","TOPN","EMPTSTACK","DEFINEMETHOD","ALIAS","UNDEF","DEFINED",
  "POSTEXE","TRACE","DEFINECLASS","SEND","INVOKESUPER","INVOKEBLOCK","LEAVE","FINISH","THROW","JUMP","BRANCHIF",
  "BRANCHUNLESS", "SETN", /* mine */ "LABEL","PUTFIXNUM","PUTSYMBOL","PUTSPECIAL"};
#endif

#define STACK_PUSH(o)  tr_array_push(vm, CUR_FRAME->stack, (o))
#define STACK_POP()    tr_array_pop(vm, CUR_FRAME->stack)

static OBJ tr_vm_send(VM, OBJ method, int argc, OBJ block_ops)
{
  size_t    i;
  OBJ      *argv = tr_malloc(sizeof(OBJ) * argc);
  OBJ       obj;
  
  for (i = argc; i > 0; --i)
    argv[i-1] = STACK_POP();
  obj = STACK_POP();
  
  return tr_send(vm, obj, method, argc, argv, block_ops);
}

static int tr_vm_branch(VM, int b, OBJ val)
{
  if (b)
    return val != TR_NIL && val != TR_FALSE; /* if */
  return val == TR_NIL || val == TR_FALSE;   /* unless */
}

static OBJ tr_vm_newarray(VM, int argc)
{
  OBJ    a = tr_array_new(vm);
  size_t i;
  
  for (i = argc; i > 0; --i)
    tr_array_insert(vm, a, 0, STACK_POP());
  
  return a;
}

static void tr_dump_stack(VM)
{
  tr_array       *a = TR_CARRAY(CUR_FRAME->stack);
  tr_array_entry *e = a->first;
  size_t          i = 0;
  
  while (e != NULL) {
    printf("    [%d] %p\n", i++, e->value);
    e = e->next;
  }
}

#define JUMP_TO(label) ip = (int) tr_hash_get(vm, label2ip, label)
#define LINENUM        TR_FIX(tr_array_at(vm, op, 0))
#define OPCODE         TR_FIX(tr_array_at(vm, op, 1))
#define CMD(i)         tr_array_at(vm, op, i+2)

OBJ tr_run(VM, OBJ ops)
{
  tr_frame *f = CUR_FRAME;
  OBJ       op;
  size_t    ip;
  size_t    n = TR_FIX(tr_array_count(vm, ops));
  OBJ       label2ip = tr_hash_new(vm);
  
  /* store labels=>ip mapping for later jumping */
  for (ip = 0; ip < n; ++ip) {
    op = tr_array_at(vm, ops, ip);
    if (OPCODE == LABEL)
      tr_hash_set(vm, label2ip, CMD(0), (OBJ) ip);
  }
  
  for (ip = 0; ip < n; ++ip) {
    op = tr_array_at(vm, ops, ip);
    f->line = LINENUM;
    
    #ifdef TRACE_STACK
    printf("[%d] %s   (line %d)\n", ip, tr_inst_names[OPCODE], LINENUM);
    #endif
    
    switch (OPCODE) {
      /* nop */
      case NOP:
        break;
      
      /* variable */
      case GETLOCAL:
        STACK_PUSH(tr_hash_get(vm, f->locals, CMD(0)));
        break;
      case SETLOCAL:
        tr_hash_set(vm, f->locals, CMD(0), STACK_POP());
        break;
      case GETDYNAMIC:
        /* huuu so not sure about this... see proc.c for HACK */
        STACK_PUSH(VM_FRAME(TR_FIX(CMD(1)))->block_argv[VM_FRAME(TR_FIX(CMD(1)))->block_argc-TR_FIX(CMD(0))]);
        break;
      case SETINSTANCEVARIABLE:
        tr_hash_set(vm, TR_COBJ(f->self)->ivars, CMD(0), STACK_POP());
        break;
      case GETINSTANCEVARIABLE:
        STACK_PUSH(tr_hash_get(vm, TR_COBJ(f->self)->ivars, CMD(0)));
        break;
      case SETCLASSVARIABLE:
        tr_hash_set(vm, TR_COBJ(f->class)->ivars, CMD(0), STACK_POP());
        break;
      case GETCLASSVARIABLE:
        STACK_PUSH(tr_hash_get(vm, TR_COBJ(f->class)->ivars, CMD(0)));
        break;
      case GETCONSTANT:
        STACK_POP(); /* TODO class */
        STACK_PUSH(tr_const_get(vm, TR_STR(CMD(0))));
        break;
      case NEWARRAY:
        STACK_PUSH(tr_vm_newarray(vm, TR_FIX(CMD(0))));
        break;
      case DUPARRAY:
        STACK_PUSH(CMD(0));
        break;
      
      /* put */
      case PUTNIL:
        STACK_PUSH(TR_NIL);
        break;
      case PUTSTRING:
        STACK_PUSH(CMD(0));
        break;
      case PUTFIXNUM:
        STACK_PUSH(CMD(0));
        break;
      case PUTSYMBOL:
        STACK_PUSH(CMD(0));
        break;
      case PUTSPECIAL:
        STACK_PUSH(tr_special_get(vm, CMD(0)));
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
        STACK_PUSH(tr_vm_send(vm, CMD(0),         /* method */
                                  TR_FIX(CMD(1)), /* argc */
                                  CMD(2)));       /* block opcode */
        break;
      case LEAVE:
        return STACK_POP();
      case DEFINEMETHOD:
        tr_ops_def(vm, f->class, CMD(0),  /* name */
                                 CMD(1)); /* opcode */
        break;
      case INVOKEBLOCK:
        STACK_PUSH(tr_yield(vm, 0, 0));
        break;
      
      /* class */
      case DEFINECLASS:
        tr_class_define(vm, CMD(0),           /* name */
                            STACK_POP(),      /* cbase */
                            STACK_POP(),      /* super */
                            CMD(1),           /* opcode */
                            TR_FIX(CMD(2)));  /* define_type */
        break;
      
      /* jump */
      case JUMP:
        JUMP_TO(CMD(0));
        break;
      case BRANCHUNLESS:
        if (tr_vm_branch(vm, 0, STACK_POP()))
          JUMP_TO(CMD(0));
        break;
      case BRANCHIF:
        if (tr_vm_branch(vm, 1, STACK_POP()))
          JUMP_TO(CMD(0));
        break;
      case LABEL:
        break;
      
      default:
        tr_log("unsupported instruction: %d (ip=%d)", OPCODE, ip);
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

OBJ tr_yield(VM, int argc, OBJ argv[])
{
  tr_frame *f = CUR_FRAME;
  OBJ       ret;
  
  if (f->block == TR_NIL)
    tr_raise(vm, "No block given");
  
  return tr_proc_call(vm, f->block, argc, argv);
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
  f->block  = TR_NIL;
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
  
  tr_vm_init(vm);
  tr_proc_init(vm);
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
  f->self    = tr_new2(vm, f->class);
  f->line    = 0;
  f->block   = TR_NIL;
  
  /* init argv */
  OBJ argv_ary = tr_array_new(vm);
  for(i = 0; i < argc; ++i)
    tr_array_push(vm, argv_ary, tr_string_new(vm, argv[i]));
  tr_const_set(vm, "ARGV", argv_ary);
}

static OBJ tr_vm_run(VM, OBJ self, OBJ ops)
{
  return tr_run(vm, ops);
}

static OBJ tr_vm_to_s(VM, OBJ self)
{
  return tr_string_new(vm, "VM");
}

void tr_vm_init(VM)
{
  OBJ obj = tr_new2(vm, tr_const_get(vm, "Object"));
  
  tr_metadef(vm, obj, "run", tr_vm_run, 1);
  tr_metadef(vm, obj, "to_s", tr_vm_to_s, 0);
  
  tr_const_set(vm, "VM", obj);
}
