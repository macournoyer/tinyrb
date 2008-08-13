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
  tr_io  *io  = TR_CIO(self);
  char   *buf = (char *) tr_malloc(TR_FIX(len) * sizeof(char));
  ssize_t red;
  
  red = read(io->fd, buf, TR_FIX(len));
  
  if (red == 0)
    return TR_NIL;
  
  return tr_string_new(vm, buf);
}

static OBJ tr_io_read_file(VM, OBJ self, OBJ fname)
{
  int     fd  = open(TR_STR(fname), O_RDONLY);
  tr_io  *io  = (tr_io *) tr_io_new(vm, fd);
  off_t   len = lseek(io->fd, 0, SEEK_END);
  char   *buf = (char *) tr_malloc(len * sizeof(char));
  ssize_t red;
  
  if (len == -1)
    return TR_NIL;
  
  lseek(io->fd, 0, SEEK_SET);
  red = read(io->fd, buf, len);
  
  if (red == 0)
    return TR_NIL;
  
  return tr_string_new(vm, buf);
}

void tr_io_init(VM)
{
  OBJ io = tr_class_new(vm, "IO", tr_const_get(vm, "Object"));
  
  tr_def(vm, io, "read", tr_io_read, 1);
  tr_metadef(vm, io, "read", tr_io_read_file, 1);
  
  /* register global constants */
  OBJ in = tr_io_new(vm, 0);
  tr_const_set(vm, "STDIN", in);
}
