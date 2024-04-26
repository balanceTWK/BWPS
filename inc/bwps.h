#ifndef H_BWPS__
#define H_BWPS__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


enum bwps_error
{
    BWPS_OK              = 0,
    BWPS_ENOMEM          = 1,
    BWPS_EINVAL          = 2,
    BWPS_INVALID_PARAM   = 3,
    BWPS_MEM_NOT_ALIGNED = 4,
    BWPS_BAD_MUTEX       = 5,
    BWPS_TIMEOUT         = 6,
    BWPS_ERR_IN_ISR      = 7,
    BWPS_ERR_PRIV        = 8,
    BWPS_OS_NOT_STARTED  = 9,
    BWPS_ENOENT          = 10,
    BWPS_EBUSY           = 11,
    BWPS_ERROR           = 12,
};
typedef enum bwps_error bwps_error_t;

#include "bwps_link_layer.h"
#include "bwps_data_cache.h"
#include "bwps_data_control_logic_layer.h"

#ifdef __cplusplus
}
#endif

#endif /* H_BWPS__ */
