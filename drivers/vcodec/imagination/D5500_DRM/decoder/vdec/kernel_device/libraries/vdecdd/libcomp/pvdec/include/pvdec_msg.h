/*!
 *****************************************************************************
 *
 * @File       pvdec_msg.h
 * @Title      Messages to PVDEC firmware.
 * @Description    This file contains functions for creating messages to PVDEC firmware.
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

#ifndef __PVDEC_MSG_H__
#define __PVDEC_MSG_H__

#include <img_defs.h>
#include "vdecdd_int.h"
//#include "hwctrl_api.h"
#include "vxd_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
******************************************************************************

 @Function              PVDEC_ReturnMsg

 @Description

 This functions should be called once buffer returned by one of functions
 for getting messages is no longer needed.

 @Input       pui8Msg      : Buffer to return.


******************************************************************************/
extern IMG_VOID
PVDEC_ReturnMsg(
    IMG_HANDLE    hVxd,
    IMG_UINT8   * pui8Msg
);

/*!
******************************************************************************

 @Function              PVDEC_ProcessMessage

 @Description

 Processes message from PVDEC firmware. Updates status of picture in
 pending pictures list and status of messages queues.

 @Input     psHostMsgStatus : Pointer to PVDEC message status.

 @Input     pui32Msg        : Pointer to the message.

 @Input     psPendPictList  : Pointer to the pictures pending list.

 @Input     hDWRTimerHandle : DWR timer handle.

 @Output    pbDecodedMsg    : Pointer to boolean to return whether messages
                              was decoded message.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT
PVDEC_ProcessMessage(
    IMG_HANDLE           hVxd,
    IMG_UINT32         * pui32Msg,
    LST_T              * psPendPictList,
    IMG_HANDLE           hDWRTimerHandle,
    IMG_BOOL           * pbDecodedMsg
);

/*!
******************************************************************************

 @Function              PVDEC_GetMessage

 @Description

 Prepares a message for PVDEC firmware of given type (eMsgId).

 @Input     hVxd            : Context handler.

 @Input     eMsgId          : Massage Id.

 @Input     psMsgData       : Massage data.

 @Output    ppui32Msg       : Pointer to the message.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
IMG_RESULT PVDEC_GetMessage(
    const IMG_HANDLE               hVxd,
    const HWCTRL_eMsgId            eMsgId,
    const HWCTRL_sMsgData       *  psMsgData,
    IMG_UINT8                   ** ppui8Msg
);

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              PVDEC_SetFWBSP_BRIDGEMessageData

 @Description

 Sets FWBSP_BRIDGE message data.

 @Return   IMG_RESULT      : This function returns IMG_SUCCESS.

******************************************************************************/
IMG_RESULT PVDEC_SetFWBSP_BRIDGEMessageData(
    IMG_HANDLE                  hVxd,
    IMG_HANDLE                  hFWBSP_BRIDGECtx,
    FWBSP_BRIDGE_sFwMsgData   * psFwMsgData,
    FWBSP_BRIDGE_sHostMsgData * psHostMsgData
);

#endif /* VDEC_USE_PVDEC_SEC */


#ifdef __cplusplus
}
#endif

#endif /* __PVDEC_MSG_H__   */
