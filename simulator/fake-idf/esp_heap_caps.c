#include "esp_heap_caps.h"
#include <stdlib.h>

void *heap_caps_malloc(unsigned int size, unsigned int  caps){
    return malloc(size);
}
void heap_caps_free(void *ptr){
    free(ptr);
}
void *heap_caps_calloc(unsigned int n, unsigned int size, unsigned int caps){
    return calloc(n, size);
}

void *heap_caps_realloc(void * ptr, unsigned int size, unsigned int caps){
    return realloc(ptr, size);
}