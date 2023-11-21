#include <stdlib.h>

void *memalign(size_t blocksize, size_t bytes) {
    void *result=0;
    posix_memalign(&result, blocksize, bytes);
    return result;
}