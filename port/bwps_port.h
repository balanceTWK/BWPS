#ifndef H_BWPS_PORT_
#define H_BWPS_PORT_

#ifdef __cplusplus
extern "C" {
#endif

#define DBG_TAG "bwps"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

// #if 1
// #define LOG_D(...) rt_kprintf(__VA_ARGS__)
// #else
// #define LOG_D(...)
// #endif
int bwps_user_init(void);
void* bwps_malloc(int nbytes);
void bwps_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* H_BWPS_PORT_ */
