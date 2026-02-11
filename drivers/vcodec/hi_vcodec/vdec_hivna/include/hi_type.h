#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__

typedef unsigned char           HI_U8;
typedef unsigned int            HI_U32;

typedef signed char             HI_S8;
typedef signed int              HI_S32;

#ifndef _M_IX86
typedef unsigned long long      HI_U64;
typedef long long               HI_S64;
#else
typedef __int64                 HI_U64;
typedef __int64                 HI_S64;
#endif

typedef char                    HI_CHAR;
typedef char*                   HI_PCHAR;

#define HI_VOID                 void

typedef unsigned long           HI_SIZE_T;

typedef unsigned long  	        HI_VIRT_ADDR_T;

typedef enum {
	HI_FALSE = 0,
	HI_TRUE = 1,
} HI_BOOL;

#ifndef NULL
#define NULL              (0L)
#endif

#define HI_NULL           (0L)

#define HI_SUCCESS        (0)
#define HI_FAILURE        (-1)

#if defined(__KERNEL__)
#include <linux/version.h>
#endif

#endif
