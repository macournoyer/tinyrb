CC = gcc
CFLAGS = -Wall -DDEBUG -g -O2
INCS = -Ivm -Ivm/vendor/basekit/_build/headers
BASEKIT = vm/vendor/basekit/_build/lib/libbasekit.a
GC = vm/vendor/garbagecollector/_build/lib/libgarbagecollector.a
LIBS = ${BASEKIT} ${GC}
LEMON = tools/lemon/lemon
RAGEL = ragel

SRC = vm/string.c vm/number.c vm/array.c vm/class.c vm/object.c vm/compiler.c vm/grammar.c vm/scanner.c vm/vm.c vm/tr.c
OBJ = ${SRC:.c=.o}
OBJ_MIN = vm/tr.o

all: tinyrb

.c.o:
	@echo "   cc $<"
	@${CC} -c ${CFLAGS} ${INCS} -o $@ $<

tinyrb: ${LIBS} ${OBJ}
	@${CC} ${CFLAGS} ${OBJ_POTION} ${OBJ} ${LIBS} -o tinyrb

vm/scanner.c: vm/scanner.rl
	@echo ragel $<
	@${RAGEL} $< -C -o $@

vm/grammar.c: ${LEMON} vm/grammar.y
	@echo lemon vm/grammar.y
	@${LEMON} vm/grammar.y

${LEMON}: ${LEMON}.c
	@echo "   cc lemon"
	@${CC} -o ${LEMON} ${LEMON}.c

${BASEKIT}:
	@echo " make basekit"
	@cd vm/vendor/basekit && make -s

${GC}:
	@echo " make garbagecollector"
	@cp -f vm/vendor/basekit/Makefile.lib vm/Makefile.lib
	@cd vm/vendor/garbagecollector && make -s
	@rm vm/Makefile.lib

test: tinyrb
	@ruby test/runner

sloc: clean
	@cp vm/scanner.rl vm/scanner.rl.c
	@sloccount vm
	@rm vm/scanner.rl.c

size: clean
	@ruby -e 'puts "%0.2fK" % (Dir["vm/**.{c,y,rl,h}"].inject(0) {|s,f| s += File.size(f)} / 1024.0)'

clean:
	@rm -f vm/*.o vm/scanner.c vm/grammar.{c,h,out}

rebuild: clean tinyrb

.PHONY: all sloc size clean rebuild test
