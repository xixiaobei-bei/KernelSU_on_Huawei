

#ifndef __OAL_SOFTWDT_H__
#define __OAL_SOFTWDT_H__
#include "oal_types.h"

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
int32_t oal_softwdt_init(void);
void oal_softwdt_exit(void);
#else
OAL_STATIC OAL_INLINE int32_t oal_softwdt_init(void)
{
    return OAL_SUCC;
}
OAL_STATIC OAL_INLINE void oal_softwdt_exit(void)
{
    return;
}
#endif

#endif /* end of oal_softwdt.h */
