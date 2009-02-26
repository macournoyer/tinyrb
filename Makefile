CC = gcc
CFLAGS = -Wall -DDEBUG -g -O2
INCS = -Ivm -Ivendor/gc/build/include
LIBS = ${GC}
GC = vendor/gc/build/lib/libgc.a
LEMON = vendor/lemon/lemon
RAGEL = ragel
RAGELV = `${RAGEL} -v | sed "/ version /!d; s/.* version //; s/ .*//"`

SRC = vm/string.c vm/number.c vm/primitive.c vm/array.c vm/hash.c vm/class.c vm/kernel.c vm/object.c vm/compiler.c vm/grammar.c vm/scanner.c vm/vm.c vm/tr.c
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
	@if [ "${RAGELV}" != "6.3" ]; then \
		echo "   !! Ragel v6.3 not found, things might be buggy"; \
	fi
	@${RAGEL} $< -C -o $@

vm/grammar.c: ${LEMON} vm/grammar.y
	@echo lemon vm/grammar.y
	@${LEMON} vm/grammar.y

${LEMON}: ${LEMON}.c
	@echo "   cc lemon"
	@${CC} -o ${LEMON} ${LEMON}.c

${GC}:
	@echo " make gc"
	@cd vendor/gc && ./configure --prefix=`pwd`/build --disable-threads -q && make -s && make install -s

test: tinyrb
	@ruby test/runner

sloc: clean
	@cp vm/scanner.rl vm/scanner.rl.c
	@sloccount vm lib
	@rm vm/scanner.rl.c

size: clean
	@ruby -e 'puts "%0.2fK" % (Dir["vm/*.{c,y,rl,h}"].inject(0) {|s,f| s += File.size(f)} / 1024.0)'

clean:
	@rm -f vm/*.o vm/scanner.c vm/grammar.{c,h,out}

rebuild: clean tinyrb

site:
	scp site/* macournoyer@code.macournoyer.com:code.macournoyer.com/tinyrb

.PHONY: all sloc size clean rebuild test site
