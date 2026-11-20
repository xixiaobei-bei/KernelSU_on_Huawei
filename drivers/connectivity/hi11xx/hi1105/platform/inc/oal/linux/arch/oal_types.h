

#ifndef __OAL_LINUX_TYPES_H__
#define __OAL_LINUX_TYPES_H__
#include <linux/types.h>

/* ∫Í∂®“Â */
#ifdef INLINE_TO_FORCEINLINE
#define OAL_INLINE __forceinline
#else
#define OAL_INLINE inline
#endif

#define OAL_STATIC static

#if defined(_PRE_PC_LINT)
#define OAL_VOLATILE
#else
#define OAL_VOLATILE volatile
#endif

#endif /* end of oal_types.h */
