#define MALLOC_CAP_INTERNAL 0
#define MALLOC_CAP_8BIT 0
#define MALLOC_CAP_SPIRAM 0
#define MALLOC_CAP_DEFAULT 0

#ifdef __cplusplus
extern "C"
{
#endif
void *heap_caps_malloc(unsigned int , unsigned int  );
void heap_caps_free(void *);
void *heap_caps_calloc(unsigned int , unsigned int , unsigned int );
void *heap_caps_realloc(void *, unsigned int , unsigned int );
#ifdef __cplusplus
}
#endif
