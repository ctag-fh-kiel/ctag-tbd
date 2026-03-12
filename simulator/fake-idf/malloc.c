#include "malloc.h"

static struct mallinfo_t mockmallinfo = {
    .arena = 1024 * 512,
    .ordblks = 0,
    .smblks = 0,
    .hblks = 0,
    .hblkhd = 0,
    .usmblks = 0,
    .fsmblks = 0,
    .uordblks = 1024 * 256,
    .fordblks = 1024 * 256,
    .keepcost = 0
};

struct mallinfo_t mallinfo() {
    return mockmallinfo;
}

void *memalign(size_t blocksize, size_t bytes) {
    void *result=0;
    posix_memalign(&result, blocksize, bytes);
    return result;
}
