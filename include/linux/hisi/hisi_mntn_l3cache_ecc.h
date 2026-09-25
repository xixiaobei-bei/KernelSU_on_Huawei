

#ifndef __HISI_MNTN_L3CACHE_ECC_H__
#define __HISI_MNTN_L3CACHE_ECC_H__

#include "../../../drivers/hisi/mntn/mntn_l3cache_ecc.h"

#ifdef CONFIG_HISI_L3CACHE_ECC
extern enum serr_type l3cache_ecc_get_status(u64 *err1_status, u64 *err1_misc0);
#else
static inline enum serr_type l3cache_ecc_get_status(u64 *err1_status, u64 *err1_misc0)
{
    return NO_EEROR;
}
#endif
#endif /* __HISI_MNTN_L3CACHE_ECC_H__ */
