CFLAGS = -g -O3 -Wno-unused-function

all: test_l2_eviction
.PHONY: clean

ARCH = undef


test_l2_eviction: test_l2_eviction.c
	gcc  -o $@ test_l2_eviction.c evict_sys_table_l2.c $(CFLAGS) -D$(ARCH) -DVERBOSE

clean:
	rm -f test_l2_eviction
