

#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__

typedef unsigned char           hi_u8;
typedef unsigned int            hi_u32;

typedef signed char             hi_s8;
typedef signed int              hi_s32;

#ifndef _M_IX86
typedef unsigned long long      hi_u64;
typedef long long               hi_s64;
#else
typedef __int64                 hi_u64;
typedef __int64                 hi_s64;
#endif

typedef char                    hi_char;
typedef char                   *hi_pchar;

#define hi_void                 void

typedef unsigned long           hi_size_t;

typedef unsigned long           hi_virt_addr_t;

typedef enum {
	HI_FALSE = 0,
	HI_TRUE = 1,
} hi_bool;

#ifndef NULL
#define NULL              0L
#endif

#define HI_NULL           0L

#define HI_SUCCESS        0
#define HI_FAILURE        (-1)

#if defined(__KERNEL__)
#include <linux/version.h>
#endif
#include "securec.h"
#endif
