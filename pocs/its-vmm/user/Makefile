CFLAGS = -g -O3 -Wno-unused-function -no-pie

ARCH = undef
TEST = undef

COMMON = ../../../common

all: main
.PHONY: clean

main: main.c snippets/snippet.o snippets/history.o flush_and_reload.c jitting.c helper.c mitigations.c leakage_rate.c config.c ${COMMON}/kmmap/mm.c
	cc -c snippets/snippet.S -o snippets/snippet.o
	cc -c snippets/history.S -o snippets/history.o
	gcc  -o $@ main.c snippets/snippet.o snippets/history.o flush_and_reload.c jitting.c helper.c mitigations.c leakage_rate.c config.c ${COMMON}/kmmap/mm.c $(CFLAGS) -D$(ARCH) -D$(TEST)

clean:
	rm -f main snippets/snippet.o snippets/history.o

