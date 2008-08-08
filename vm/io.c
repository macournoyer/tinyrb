#include "tinyrb.h"

OBJ tr_io_new(VM, int fd)
{
  tr_io *io = (tr_io *) tr_malloc(sizeof(tr_io));
  
  tr_obj_init(vm, TR_IO, (OBJ) io, tr_const_get(vm, "IO"));
  io->fd = fd;
  
  return (OBJ) io;
}

static OBJ tr_io_read(VM, OBJ self, OBJ len)
{
  tr_io  *io  = (tr_io *) self;
  char   *buf = (char *) tr_malloc(TR_FIX(len) * sizeof(char));
  ssize_t red;
  
  red = read(io->fd, buf, TR_FIX(len));
  
  if (red == 0)
    return TR_NIL;
  
  return tr_string_new(vm, buf);
}

void tr_io_init(VM)
{
  OBJ io = tr_class_new(vm, "IO", tr_const_get(vm, "Object"));
  
  tr_def(vm, io, "read", tr_io_read, 1);
  
  /* register global constants */
  OBJ in = tr_io_new(vm, 0);
  tr_const_set(vm, "STDIN", in);
}
