
#define OPTION_TRIGGER_GADGET 10


struct config {

    uint8_t * caller0;
    uint8_t * enter_jit_chain;

    uint8_t * fr_buf;
    uint8_t * fr_buf_p;

    uint8_t * secret_page;

    uint8_t * host_fr_buf;
    uint8_t * host_secret_page;

    uint8_t * host_arg_fr_buf;
    uint8_t * host_arg_secret;

    void ** evict_list;
	uint8_t * leaked_bytes;



} typedef config;


int leak_byte_forwards(struct config * cfg, uint64_t prefix);
int leak_page(struct config * cfg);
