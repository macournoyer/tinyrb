SRC = vm/vm.c vm/tr.c
OBJ = ${SRC:.c=.o}
OBJ_MIN = vm/tr.o

CC = gcc
CFLAGS = -Wall -fno-strict-aliasing -DDEBUG -g -O2
INCS = -Ivm
LEMON = tools/lemon
LIBS = -lm
RAGEL = ragel

all: tinyrb

.c.o:
	${CC} -c ${CFLAGS} ${INCS} -o $@ $<

tinyrb: ${OBJ_MIN} ${OBJ}
	${CC} ${CFLAGS} ${OBJ_POTION} ${OBJ} ${LIBS} -o tinyrb

# vm/scanner.c: vm/scanner.rl
#   ${RAGEL} vm/scanner.rl -C -o $@

# vm/grammar.c: tools/lemon vm/grammar.y
#   ${LEMON} vm/grammar.y

# tools/lemon: tools/lemon.c
#   ${CC} -o tools/lemon tools/lemon.c

clean:
	rm -f vm/*.o vm/scanner.c vm/grammar.{c,h,out}

rebuild: clean min
