CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra -DDEBUG -g -O2 -funroll-loops -fomit-frame-pointer -fstrict-aliasing
INCS = -Ivm -Ivendor/gc/build/include
LIBS = ${GC}
GC = vendor/gc/build/lib/libgc.a
LEG = vendor/peg/leg

SRC = vm/string.c vm/number.c vm/range.c vm/primitive.c vm/proc.c vm/array.c vm/hash.c vm/class.c vm/kernel.c vm/object.c vm/block.c vm/compiler.c vm/grammar.c vm/vm.c vm/tr.c
OBJ = ${SRC:.c=.o}
OBJ_MIN = vm/tr.o

all: tinyrb

.c.o:
	@echo "   cc $<"
	@${CC} -c ${CFLAGS} ${INCS} -o $@ $<

tinyrb: ${LIBS} ${OBJ}
	@${CC} ${CFLAGS} ${OBJ_POTION} ${OBJ} ${LIBS} -o tinyrb

vm/grammar.c: ${LEG} vm/grammar.leg
	@echo "  leg vm/grammar.leg"
	@${LEG} -ovm/grammar.c vm/grammar.leg

${LEG}:
	@echo " make peg/leg"
	@cd vendor/peg && make -s

${GC}:
	@echo " make gc"
	@cd vendor/gc && ./configure --prefix=`pwd`/build --disable-threads -q && make -s && make install -s

test: tinyrb
	@ruby test/runner

sloc: clean
	@cp vm/grammar.leg vm/grammar.leg.c
	@sloccount vm lib
	@rm vm/grammar.leg.c

size: clean
	@ruby -e 'puts "%0.2fK" % (Dir["vm/*.{c,leg,h}"].inject(0) {|s,f| s += File.size(f)} / 1024.0)'

clean:
	@rm -f vm/*.o vm/grammar.c

rebuild: clean tinyrb

site:
	scp site/* macournoyer@code.macournoyer.com:code.macournoyer.com/tinyrb

.PHONY: all sloc size clean rebuild test site
