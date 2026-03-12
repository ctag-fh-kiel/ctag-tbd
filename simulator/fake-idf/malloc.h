#include <stdlib.h>
#include <stdint.h>

struct mallinfo_t {
    uint32_t arena;
    uint32_t ordblks;
    uint32_t smblks;
    uint32_t hblks;
    uint32_t hblkhd;
    uint32_t usmblks;
    uint32_t fsmblks;
    uint32_t uordblks;
    uint32_t fordblks;
    uint32_t keepcost;
};


void *memalign(size_t blocksize, size_t bytes);