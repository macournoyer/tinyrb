#include <unistd.h>
#include "tr.h"
#include "internal.h"

OBJ TrIO_new(VM, int fd) {
  TrIO *io = TR_INIT_OBJ(IO);
  io->fd = fd;
  return (OBJ)io;
}

OBJ TrIO_fileno(VM, OBJ self) {
  TrIO *io = TR_CIO(self);
  return TrFixnum_new(vm, io->fd);
}

OBJ TrIO_close(VM, OBJ self) {
  TrIO *io = TR_CIO(self);
  close(io->fd);
  return TR_NIL;
}

OBJ TrIO_sysread(VM, OBJ self, OBJ integer) {
  TrIO *io = TR_CIO(self);
  int len = TR_FIX2INT(integer);
  OBJ buf = TrString_new3(vm, len);
  ssize_t red = read(io->fd, TR_STR_PTR(buf), len);
  return TrFixnum_new(vm, red);
}

OBJ TrIO_syswrite(VM, OBJ self, OBJ string) {
  TrIO *io = TR_CIO(self);
  ssize_t written = write(io->fd, TR_STR_PTR(string), TR_STR_LEN(string));
  return TrFixnum_new(vm, written);
}

OBJ TrIO_sysseek(VM, OBJ self, OBJ offset, OBJ whence) {
  TrIO *io = TR_CIO(self);
  off_t off = lseek(io->fd, TR_FIX2INT(offset), TR_FIX2INT(whence));
  return TrFixnum_new(vm, off);
}

void TrIO_init(VM) {
  OBJ c = TR_INIT_CLASS(IO, Object);
  tr_def(c, "fileno", TrIO_fileno, 0);
  tr_def(c, "close", TrIO_close, 0);
  tr_def(c, "sysread", TrIO_sysread, 2);
  tr_def(c, "syswrite", TrIO_syswrite, 1);
  tr_def(c, "sysseek", TrIO_sysseek, 2);
}
