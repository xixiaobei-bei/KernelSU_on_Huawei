/*!
 *****************************************************************************
 *
 * @File       tal.h
 * @Title      TAL
 * @Description    This file contains the structure and function prototypes
 *  for the VXD Bitstream Buffer Pre-Parser.
 * ---------------------------------------------------------------------------
 *
 * Copyright (c) Imagination Technologies Ltd.
 * 
 * The contents of this file are subject to the MIT license as set out below.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 * 
 * Alternatively, the contents of this file may be used under the terms of the 
 * GNU General Public License Version 2 ("GPL")in which case the provisions of
 * GPL are applicable instead of those above. 
 * 
 * If you wish to allow use of your version of this file only under the terms 
 * of GPL, and not to allow others to use your version of this file under the 
 * terms of the MIT license, indicate your decision by deleting the provisions 
 * above and replace them with the notice and other provisions required by GPL 
 * as set out in the file called "GPLHEADER" included in this distribution. If 
 * you do not delete the provisions above, a recipient may use your version of 
 * this file under the terms of either the MIT license or GPL.
 * 
 * This License is also included in this distribution in the file called 
 * "MIT_COPYING".
 *
 *****************************************************************************/

#ifndef __TAL_H__
#define __TAL_H__

#include "img_defs.h"
#include "lst.h"
#include "target.h"

/*
 * These symbols are exported with _SECURE_ infix but can be used in the simple
 * form textually as well. This allows linking with other TAL implementations
 * when for instance, building with ISP
 */
#define TAL_DeviceRegister TAL_SECURE_DeviceRegister
#define TAL_DeviceUnRegister TAL_SECURE_DeviceUnRegister
#define TAL_GetMemSpaceHandle TAL_SECURE_GetMemSpaceHandle
#define TALREG_WriteWord32 TALREG_SECURE_WriteWord32
#define TALREG_ReadWord32 TALREG_SECURE_ReadWord32
#define TALREG_Poll32 TALREG_SECURE_Poll32
#define TAL_Wait TAL_SECURE_Wait
#define TAL_GetMemSpaceOffset TAL_SECURE_GetMemSpaceOffset

#ifdef __cplusplus
extern "C" {
#endif

#define TAL_CHECKFUNC_ISEQUAL		(0x0)
#define TAL_CHECKFUNC_GREATEREQ		(0x4)

/*!
******************************************************************************

 @Function                TAL_DeviceRegister

******************************************************************************/
extern IMG_RESULT 
TAL_SECURE_DeviceRegister(
    const TARGET_sDevice  * psTargetDeviceInfo
);


/*!
******************************************************************************

 @Function                TAL_MemSpaceRegister

******************************************************************************/
extern IMG_RESULT 
TAL_SECURE_MemSpaceRegister(
    const TARGET_sMemorySpace      * pTargetMemSpaceInfo
);


/*!
******************************************************************************

 @Function                TAL_GetMemSpaceHandle

******************************************************************************/
extern IMG_HANDLE 
TAL_SECURE_GetMemSpaceHandle (
    const IMG_CHAR *  psMemSpaceName
);


/*!
******************************************************************************

 @Function				TAL_GetMemSpaceOffset

******************************************************************************/
IMG_RESULT TAL_SECURE_GetMemSpaceOffset(
	IMG_HANDLE		hMemSpace,
	IMG_UINT64	*	pui64MemSpaceOffset
);


/*!
******************************************************************************

 @Function              TALREG_WriteWord32

******************************************************************************/
extern IMG_RESULT 
TALREG_SECURE_WriteWord32(
    IMG_HANDLE  hMemSpace,
    IMG_UINT32  ui32Offset,
    IMG_UINT32  ui32Value
);


/*!
******************************************************************************

 @Function              TALREG_ReadWord32

******************************************************************************/
extern IMG_RESULT 
TALREG_SECURE_ReadWord32(
    IMG_HANDLE    hMemSpace,
    IMG_UINT32    ui32Offset,
    IMG_UINT32 *  pui32Value
);


/*!
******************************************************************************

 @Function                TALREG_Poll32

******************************************************************************/
extern IMG_RESULT 
TALREG_SECURE_Poll32(
    IMG_HANDLE  hMemSpace,
    IMG_UINT32  ui32Offset,
    IMG_UINT32  ui32CheckFuncIdExt,
    IMG_UINT32  ui32RequValue,
    IMG_UINT32  ui32Enable,
    IMG_UINT32  ui32PollCount,
    IMG_UINT32  ui32TimeOut
);


/*!
******************************************************************************

 @Function                TAL_Wait

******************************************************************************/
extern IMG_RESULT
TAL_SECURE_Wait(
   IMG_HANDLE						hMemSpace,
	IMG_UINT32						ui32Time
);


/*!
******************************************************************************

 @Function                TAL_DeviceUnRegister

******************************************************************************/
IMG_RESULT 
TAL_SECURE_DeviceUnRegister(
    const char * pszDeviceName
);

/*
 * Copied here when tal_old.h was obsoleted.
 * Secure TAL does not implement PDUMP but the encoder is using these macros
 * unconditionally thus breaking the build if secure TAL is selected.
 * Until a better solution is found, we add these here.
 */
#define TAL_NULL_MANGLER_ID (0xFFFFFFFF)
#define TALPDUMP_Comment(hMemSpace, psCommentText)
#define TALPDUMP_ConsoleMessage(hMemSpace, psCommentText)
#define TALPDUMP_MiscOutput(hMemSpace, pOutputText)

#ifdef __cplusplus
}
#endif

#endif /* __TAL_H__   */
