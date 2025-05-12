CFLAGS = -g -O3 -Wno-unused-function -no-pie

all: main
.PHONY: clean

main: main.c
	gcc  -o $@ main.c ../mm.c $(CFLAGS)

clean:
	rm -f main

