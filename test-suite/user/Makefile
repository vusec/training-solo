CFLAGS = -g -O3 -Wno-unused-function -no-pie

ARCH = undef
TEST = undef

COMMON = ../../common

all: main
.PHONY: clean

main: main.c snippets/snippet.o snippets/history.o flush_and_reload.c jitting.c helper.c mitigations.c branch_evict.c flip_tests.c ${COMMON}/kmmap/mm.c
	gcc  -o $@ main.c snippets/snippet.o snippets/history.o flush_and_reload.c jitting.c helper.c mitigations.c branch_evict.c flip_tests.c ${COMMON}/kmmap/mm.c $(CFLAGS) -D$(ARCH) -D$(TEST) $(EXTRA)

clean:
	rm -f main snippets/snippet.o snippets/history.o

