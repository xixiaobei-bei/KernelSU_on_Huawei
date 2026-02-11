/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2008-2019. All rights reserved.
 * Description: The common data type defination
 * Author: z44949
 * Create: 2008-10-31
 */
#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/kfifo.h>
#include <linux/timer.h>

/*
 * The common data type, will be used in the whole project.
*/
typedef unsigned char           HI_U8;
typedef unsigned char           HI_UCHAR;
typedef unsigned short          HI_U16;
typedef unsigned int            HI_U32;

typedef signed char             HI_S8;
typedef short                   HI_S16;
typedef int                     HI_S32;

#ifndef _M_IX86
typedef unsigned long long      HI_U64;
typedef long long               HI_S64;
#else
typedef __int64                 HI_U64;
typedef __int64                 HI_S64;
#endif

typedef char                    HI_CHAR;
typedef char                   *HI_PCHAR;

typedef float                   HI_FLOAT;
typedef double                  HI_DOUBLE;
typedef void                    HI_VOID;

typedef unsigned long           HI_SIZE_T;
typedef unsigned long           HI_LENGTH_T;

typedef HI_U64 HI_HANDLE;

#ifdef HIVCODECV500
typedef unsigned long long  UADDR;
#else
typedef unsigned int        UADDR;
#endif

/*
 * const defination
*/
typedef enum {
	HI_FALSE = 0,
	HI_TRUE  = 1,
} HI_BOOL;

#ifndef NULL
#define NULL             0L
#endif
#define HI_NULL          0L

#define HI_SUCCESS       (0)
#define HI_FAILURE       (-1)

/* magic number 0-255 */
#define IOC_TYPE_VENC	  'V'
#define VERSION_STRING    "1234"
#define HI_INVALID_HANDLE (0xffffffff)


#endif /* __HI_TYPE_H__ */

