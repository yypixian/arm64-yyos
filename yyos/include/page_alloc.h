void mem_init(unsigned long start_mem, unsigned long end_mem);

unsigned long get_free_page(void);


void dump_pgtable(void);
void free_page(unsigned long p);
