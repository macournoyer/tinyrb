SRC = vm/string.c vm/class.c vm/object.c vm/compiler.c vm/grammar.c vm/scanner.c vm/vm.c vm/tr.c
OBJ = ${SRC:.c=.o}
OBJ_MIN = vm/tr.o

CC = gcc
CFLAGS = -Wall -fno-strict-aliasing -DDEBUG -g -O2
INCS = -Ivm
LEMON = vendor/lemon/lemon
LIBS = -lm
RAGEL = ragel

all: tinyrb

.c.o:
	${CC} -c ${CFLAGS} ${INCS} -o $@ $<

tinyrb: ${OBJ_MIN} ${OBJ}
	${CC} ${CFLAGS} ${OBJ_POTION} ${OBJ} ${LIBS} -o tinyrb

vm/scanner.c: vm/scanner.rl
	${RAGEL} vm/scanner.rl -C -o $@

vm/grammar.c: ${LEMON} vm/grammar.y
	${LEMON} vm/grammar.y

${LEMON}: ${LEMON}.c
	${CC} -o ${LEMON} ${LEMON}.c

test: tinyrb
	@ruby test/runner

sloc: clean
	@cp vm/scanner.rl vm/scanner.rl.c
	sloccount vm
	@rm vm/scanner.rl.c

size: clean
	@ruby -e 'puts "%0.2fK" % (Dir["vm/**.{c,y,rl,h}"].inject(0) {|s,f| s += File.size(f)} / 1024.0)'

clean:
	rm -f vm/*.o vm/scanner.c vm/grammar.{c,h,out}

rebuild: clean tinyrb

.PHONY: all sloc size clean rebuild test
