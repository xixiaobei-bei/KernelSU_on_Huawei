/*!
 *****************************************************************************
 *
 * @File       secureapi_ree.h
 * @Title      Secure API REE interface.
 * @Description    This file contains the interface to a secure API for VXE/TOPAZ
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

#if !defined (__SECUREAPI_REE_H__)
#define __SECUREAPI_REE_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "img_include.h"
#include "secure_defs.h"
#include "sysdev_utils.h"

/*!
******************************************************************************

 @Function              SECURE_REE_GetId

 @Description

 This function establishes communications with a secure driver and returns an
 ID to allow commands to be sent to the driver

 @Input     nCore  			 : The core to establish secure communications with
 
 @Output    pui32SecureId	 : This function returns an ID if successful, or 
                               zero otherwise.
 
 @Return    This function returns either IMG_SUCCESS or an error code.

******************************************************************************/
IMG_RESULT SECURE_REE_GetId(IMG_VIDEO_CORE nCore, IMG_UINT32 *pui32SecureId);


/*!
******************************************************************************

 @Function              SECURE_REE_ReleaseId

 @Description

 Releases a reference to a secure driver communications channel

 @Input     ui32SecureId    : The secure ID for communicating with the requested
                              core.
 
 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
IMG_RESULT SECURE_REE_ReleaseId(IMG_UINT32 ui32SecureId);


/*!
******************************************************************************

 @Function              SECURE_REE_SendMessage

 @Description

 Sends a message to the firmware / secure API driver

 @Input     ui32SecureId    : The secure ID for communicating with the requested
                              core
 
 @Input     pbyMsg           : The message to be sent
 
 @Input     ui16Size         : The size in bytes of the message
 
 @Input 	nEndpoint		: The endpoint the message is targeted at

 
 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
IMG_RESULT SECURE_REE_SendMessage(
    IMG_UINT32 ui32SecureId,
    IMG_BYTE* pbyMsg,
    IMG_UINT16 ui16Size,
    MSG_ENDPOINT nEndpoint
);


/*!
******************************************************************************

 @Function              SECURE_REE_SendMessageWithBuf

 @Description

 Sends a message to the firmware / secure API driver with an additional
 buffer

 @Input     ui32SecureId    : The secure ID for communicating with the requested
                              core
 
 @Input     pbyMsg          : The message to be sent
 
 @Input     ui16Size        : The size in bytes of the message
 
 @Input 	nEndpoint		: The endpoint the message is targeted at

 @Input     pbyBuf          : Optional buffer to be sent to the TEE
 
 @Input     ui32BufSize     : Size in bytes of the optional buffer

 
 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.


******************************************************************************/
IMG_RESULT SECURE_REE_SendMessageWithBuf(
    IMG_UINT32          ui32SecureId,
    IMG_BYTE          * pbyMsg,
    IMG_UINT16          ui16Size,
    MSG_ENDPOINT        nEndpoint,
    IMG_BYTE          * pbyBuf,
    IMG_UINT32          ui32BufSize
);

/*!
******************************************************************************

 @Function              SECURE_REE_ReleaseMsgBuffer

 @Description

 Provides shared memoryu

 @Input     ui32SecureId     : The secure ID for communicating with the
                               requested core

 @Input     pMsgBuff         : Buffer to release.

 @Return    This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
IMG_RESULT SECURE_REE_ReleaseMsgBuffer(
    IMG_UINT32  ui32SecureId,
	IMG_VOID  * pMsgBuff
);

IMG_VOID SECURE_REE_Init(SYSDEVU_sInfo *psInfo);
IMG_VOID SECURE_REE_DeInit(IMG_VOID);

IMG_VOID SECURE_REE_StreamCreate(IMG_VOID);
IMG_VOID SECURE_REE_StreamDestroy(IMG_VOID);
#if defined(__cplusplus)
}
#endif

#endif /* __SECUREAPI_REE_H__ */



