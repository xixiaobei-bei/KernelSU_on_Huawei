/*
* Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.
*
* File Name     : hi_type.h
* Version       : Initial Draft
* Author        : Hisilicon multimedia software group
* Created       : 2005/4/23
*
* Last Modified :
* Description   : The common data type defination
*
* Function List :
* History       :
* 1.Date        : 2008/06/28
* Author      : c42025
* Modification: modified definition for HI_S8
* 2.Date        :   2008/10/31
* Author      :   z44949
* Modification:   Translate the chinese comment
*/
#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__
#include <linux/fs.h>
#include <linux/platform_device.h>


/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/
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
typedef char*                   HI_PCHAR;

typedef float                   HI_FLOAT;
typedef double                  HI_DOUBLE;
typedef void                    HI_VOID;

typedef unsigned long           HI_SIZE_T;
typedef unsigned long           HI_LENGTH_T;

typedef HI_U64 HI_HANDLE;

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
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

