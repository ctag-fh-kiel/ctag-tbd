#ifdef __cplusplus
#define TBD_IS_CPP 1
#define TBD_C_BEGIN extern "C" {
#define TBD_C_END }
#else
#define TBD_IS_CPP 0
#define TBD_C_BEGIN
#define TBD_C_END
#endif
