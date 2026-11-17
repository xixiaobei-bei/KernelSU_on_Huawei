/*!
 *****************************************************************************
 *
 * @File       hwctrl_int.h
 * @Title      Low-level video decoder interface
 * @Description    This file contains the low-level video decoder interface.
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

#ifndef __HWCTRL_INT_H__
#define __HWCTRL_INT_H__

#include "img_defs.h"
#include "vxd_ext.h"
#include "vxd.h"
#include "vdecdd_int.h"
#include "decoder_int.h"

#ifdef VDEC_USE_PVDEC_SEC
#include "fwbsp_bridge.h"
#endif /* VDEC_USE_PVDEC_SEC */

#ifdef __cplusplus
extern "C" {
#endif

#define VDEC_NUM_RENDEC_BUFFERS 2

/*!
******************************************************************************
 This type defines the core message Ids.
 @brief  HWCTRL Message Ids
******************************************************************************/
typedef enum
{
    HWCTRL_MSGID_INIT_NON_SEC_DEC = 0,          /*!< Initialise non secure decode message.   */
    HWCTRL_MSGID_BATCH            = 1,          /*!< Batch message.                          */
#ifdef VDEC_USE_PVDEC_SEC
    HWCTRL_MSGID_INIT_BSP         = 2,          /*!< Batch message.                          */
    HWCTRL_MSGID_INIT_SEC_DEC     = 3,          /*!< Batch message.                          */
    HWCTRL_MSGID_DEC              = 4,          /*!< Batch message.                          */
    HWCTRL_MSGID_CONFIG           = 5,          /*!< Batch message.                          */
    HWCTRL_MSGID_PARSE            = 6,          /*!< Batch message.                          */
#endif /* VDEC_USE_PVDEC_SEC */

    CORE_MSGID_MAX,                             /*!< Max. message id.                        */

} HWCTRL_eMsgId;

/*!
******************************************************************************
 This structure contains buffers needed to create init message.
 @brief  Buffers needed for init message.
******************************************************************************/
typedef struct
{
    VXDIO_sDdBufInfo             * psRendecBufInfo;
} HWCTRL_sInitNonSecDecMsgData;

/*!
******************************************************************************
 This structure contains buffers needed to create batch message.
 @brief  Buffers needed for batch message.
******************************************************************************/
typedef struct
{
    VXDIO_sDdBufInfo    * psBatchMsgBufInfo;
    VXDIO_sDdBufInfo    * psStrPtdBufInfo;
    VXDIO_sDdBufInfo    * psPvdecFwCtx;
    IMG_UINT32            ui32CtrlAllocBytes;
    IMG_UINT32            ui32OperatingMode;
    IMG_UINT32            ui32TransactionId;
    IMG_UINT32            ui32TileCfg;
    IMG_UINT8             ui8Pipe;
    IMG_UINT32            ui16GencId;
    IMG_BOOL              bSizeDelimitedMode;

} HWCTRL_sBatchMsgData;

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************
 This structure contains buffers needed to create init message.
 @brief  Buffers needed for init message.
******************************************************************************/
typedef struct
{
    FWBSP_BRIDGE_sFwMsgData   * psFwMsgData;
} HWCTRL_sInitBSPMsgData;

/*!
******************************************************************************
 This structure contains buffers needed to create init message.
 @brief  Buffers needed for init message.
******************************************************************************/
typedef struct
{
    VXDIO_sDdBufInfo             * psRendecBufInfo;
} HWCTRL_sInitSecDecMsgData;

/*!
******************************************************************************
 This structure contains buffers needed to create decode message.
 @brief  Buffers needed for decode message.
******************************************************************************/
typedef struct
{
    DECODER_sDecPict             * psDecPict;
} HWCTRL_sDecMsgData;

/*!
******************************************************************************
 This structure contains buffers needed to create configure message.
 @brief  Buffers needed for configure message.
******************************************************************************/
typedef struct
{
    const VDECDD_sDdDevConfig  * psDevConfig;
    const VDECFW_sCoreInitData * psCoreInitData;
} HWCTRL_sConfigureMsgData;

/*!
******************************************************************************
 This structure contains buffers needed to create parse message.
 @brief  Buffers needed for parse message.
******************************************************************************/
typedef struct
{
    IMG_HANDLE                  hFWBSP_BRIDGECtx;
    FWBSP_BRIDGE_sFwMsgData   * psFwMsgData;
} HWCTRL_sParseMsgData;
#endif /* VDEC_USE_PVDEC_SEC */


typedef struct
{
    union
    {
        HWCTRL_sInitNonSecDecMsgData   sInitNonSecDecMsgData;
        HWCTRL_sBatchMsgData           sBatchMsgData;
#ifdef VDEC_USE_PVDEC_SEC
        HWCTRL_sInitBSPMsgData         sInitBSPMsgData;
        HWCTRL_sInitSecDecMsgData      sInitSecDecMsgData;
        HWCTRL_sDecMsgData             sDecMsgData;
        HWCTRL_sConfigureMsgData       sConfigureMsgData;
        HWCTRL_sParseMsgData           sParseMsgData;
#endif /* VDEC_USE_PVDEC_SEC */
    };

} HWCTRL_sMsgData;

/*!
******************************************************************************
 This structure contains MSVDX Message information.
 @brief  MSVDX Message Information
******************************************************************************/
typedef struct
{
    IMG_UINT8  aui8ControlFenceID[VDECFW_MSGID_CONTROL_TYPES];        /*!< FenceID of last message sent to the control message buffer.         */
    IMG_UINT8  aui8DecodeFenceID[VDECFW_MSGID_DECODE_TYPES];          /*!< FenceID of last message sent to the decode message buffer.          */
    IMG_UINT8  aui8CompletionFenceID[VDECFW_MSGID_COMPLETION_TYPES];  /*!< FenceID of last message received in the completion message buffer.  */

} HWCTRL_sMsgStatus;


/*!
******************************************************************************
 This structure contains rendec properties.
 @brief  Rendec Properties
******************************************************************************/
typedef struct
{
    IMG_BOOL    bInUse;                                        /*!< Enable/Disable use of external memory.              */
    IMG_UINT32  aui32BufferSize[VDEC_NUM_RENDEC_BUFFERS];      /*!< Size of Rendec buffer 0 in multiples of 4KB.        */
    IMG_UINT32  ui32DecodeStartSize;                           /*!< Threshold in bytes before Rendec starts processing. */

    IMG_UINT8   ui8BurstSizeWrite;                             /*!< Burst size of Rendec write: 0--2.                   */
    IMG_UINT8   ui8BurstSizeRead;                              /*!< Burst size of Rendec read: 0--3.                    */

    IMG_UINT32  aui32InitialContext[6];                        /*!< Initial context for Rendec.                         */

} HWCTRL_sRendec;

/*!
******************************************************************************

 @Function              HWCTRL_pfnHandleInt

 @Description

 This is the prototype of function for writing to VLR.

 @Input    hVxd         : A handle to the video decoder context.

 @Output   psIntStatus  : Interrupt status to update.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnHandleInt)(
    const IMG_HANDLE    hVxd,
    VXD_sIntStatus    * psIntStatus
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnGetStatus

 @Description

 This is the prototype of function for getting the interrupts status.
 Unlike HWCTRL_pfnHandleInt, it ignores psMsgQueue and does not clear MMU
 fault status.
 @note: it's NOT atomic (manually create critical section if needed)

 @Input    hVxd         : A handle to the video decoder context.

 @Output   psIntStatus  : Interrupt status to update.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnGetIntStatus)(
    const IMG_HANDLE    hVxd,
    VXD_sIntStatus    * psIntStatus
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnDumpRegisters

 @Description

 This is prototype of function for reporting state of registers

 @Input    hVxd         : A handle to the video decoder context.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnDumpRegisters)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnSendFirmwareMessage

 @Description

 This is the prototype of function for submitting message to video decoder
 firmware.

 @Input    hVxd         : A handle to the video decoder context.

 @Input    eArea        : Type of communication area to use.

 @Input    psMsgHdr     : Buffer with message to send.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnSendFirmwareMessage)(
    const IMG_HANDLE    hVxd,
    const IMG_VOID    * psMsgHdr
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnGetCoreState

 @Description

 This is the prototype of function for obtaining video decoder core state.

 @Input    hVxd         : A handle to the video decoder context.

 @Output   psState      : Video decoder core state to fill.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnGetCoreState)(
    const IMG_HANDLE     hVxd,
    VXDIO_sState       * psState
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnFlushMmuCache

 @Description

 This is the prototype of function for flushing MMU cache of video decoder.

 @Input    hVxd         : A handle to the video decoder context.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnFlushMmuCache)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnInitialise

 @Description

 This is the prototype of function for initialisation of video decoder context.

 @Input    hVxd         : A handle to the video decoder context.

 @Input    psInitConfig : Pointer to the structure with MMU and Rendec
                          configuration.

 @Input    bSecureStream: Indicates whether secure stream is being decoded.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnInitialise)(
    const IMG_HANDLE        hVxd,
    VDECFW_sCoreInitData  * psInitConfig,
    const IMG_BOOL          bSecureStream
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnDeInitialise

 @Description

 This is the prototype of function for de-initialisation of video decoder
 context.

 @Input    hVxd         : A handle to the video decoder context.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnDeInitialise)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnDestroy

 @Description

 This is the prototype of function which destroys video decoder context.

 @Input    hVxd         : A handle to the video decoder context.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnDestroy)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnPrepareFirmware

 @Description

 This is the prototype of function which prepares buffer with firmware for
 uploading to video decoder device.

 @Input    hVxd         : A handle to the video decoder context.

 @Input    psFw         : Structure with pointers to buffers with firmware
                          binaries.

 @Input    psPtdInfo    : Structure which contains location of page table
                          directory within video decoder memory.

 @Input    bSecureStream: Indicates whether secure stream is being decoded.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnPrepareFirmware)(
    const IMG_HANDLE        hVxd,
    const VXD_sFirmware   * psFirmware,
    const VXDIO_sPtdInfo  * psPtdInfo,
    const IMG_BOOL          bSecureStream
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnGetRegsOffsets

 @Description

 This is the prototype of function for getting register spaces offsets.

 @Input    hVxd           : A handle to the video decoder context.

 @Output   psRegsOffsets  : Structure with register spaces offsets.

 @Return   IMG_RESULT     : This function returns either IMG_SUCCESS or an error
                            code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnGetRegsOffsets)(
    const IMG_HANDLE        hVxd,
    DECODER_sRegsOffsets  * psRegsOffsets
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnGetMessage

 @Description

 This is the prototype of function which prepares initial message to video
 decoder firmware.

 @Input   hVxd               : A handle to the video decoder context.

 @Input   eMsgId             : Id of a message to be prepared.

 @Input   psMsgData          : Data needed to prepeare message of given Id.

 @Output  ppui8Msg           : Pointer to buffer with message. Buffer is
                               allocated in this function.

 @Return  IMG_RESULT         : This function returns either IMG_SUCCESS or an
                               error code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnGetMessage)(
    const IMG_HANDLE               hVxd,
    const HWCTRL_eMsgId            eMsgId,
    const HWCTRL_sMsgData       *  psMsgData,
    IMG_UINT8                   ** ppui8Msg
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnReturnMsg

 @Description

 This is the prototype of function which has to called once message buffer
 obtained with one of functions for getting messages.

 @Input   hVxd               : A handle to the video decoder context.

 @Input   pui8Msg            : Buffer to return.

******************************************************************************/
typedef IMG_VOID ( * HWCTRL_pfnReturnMsg)(
    const IMG_HANDLE    hVxd,
    IMG_UINT8         * pui8Msg
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnProcessMessage

 @Description

 This is the prototype of function for processing messages received from
 video decoder firmware.

 @Input   hVxd               : A handle to the video decoder context.

 @Output  pui8Msg            : Pointer to buffer with message. Buffer is
                               allocated in this function.

 @Input   psHostMsgStatus    : IDs of messages submitted to particular
                               message areas.

 @Input   psPendPictList     : List of pictures pending decode.

 @Input   hDWRTimerHandle    : DWR timer handle.

 @Output  pbDecodedMsg       : Whether message was a decode indication.

 @Return  IMG_RESULT         : This function returns either IMG_SUCCESS or an
                               error code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnProcessMessage)(
    const IMG_HANDLE       hVxd,
    IMG_UINT32           * pui32Msg,
    LST_T                * psPendPictList,
    IMG_HANDLE             hDWRTimerHandle,
    IMG_BOOL             * pbDecodedMsg
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnDisableCoreInterrupts

 @Description

 This is the prototype of function for disabling video decoder core interrupts.

 @Input    hVxd         : A handle to the video decoder context.

 @Return   IMG_RESULT   : This function returns either IMG_SUCCESS or an error
                          code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnDisableCoreInterrupts)(
    const IMG_HANDLE  hVxd
);


#ifndef IMG_KERNEL_MODULE
/*!
******************************************************************************

 @Function              HWCTRL_pfnPDUMPSync

 @Description

 Sync pdump contexts

 @Input   hVxd : A handle to the video decoder context.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnPDUMPSync)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnPDUMPUnlock

 @Description

 Unlock pdump context

 @Input   hVxd : A handle to the video decoder context.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnPDUMPUnlock)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnPDUMPLock

 @Description

 Lock pdump context

 @Input   hVxd : A handle to the video decoder context.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnPDUMPLock)(
    const IMG_HANDLE  hVxd
);

/*!
******************************************************************************

 @Function              HWCTRL_pfnPDUMPComment

 @Description

 Write comment to primary PDUMP context (C0_MSVDX_out2.txt).

 @Input   hVxd : A handle to the video decoder context.

 @Input   pszComment : Pointer to comment string

 @Return  IMG_RESULT : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
typedef IMG_RESULT ( * HWCTRL_pfnPDUMPComment)(
    const IMG_HANDLE  hVxd,
    const IMG_CHAR *  pszComment
);
#endif /* IMG_KERNEL_MODULE */

/*!
******************************************************************************
 This structure contains low level video decoder callbacks.

******************************************************************************/
typedef struct
{
    HWCTRL_pfnHandleInt                pfnHandleInt;
    HWCTRL_pfnGetIntStatus             pfnGetIntStatus;
    HWCTRL_pfnDumpRegisters            pfnDumpRegisters;
    HWCTRL_pfnSendFirmwareMessage      pfnSendFirmwareMessage;
    HWCTRL_pfnGetCoreState             pfnGetCoreState;
    HWCTRL_pfnFlushMmuCache            pfnFlushMmuCache;
    HWCTRL_pfnDisableCoreInterrupts    pfnDisableCoreInterrupts;
    HWCTRL_pfnInitialise               pfnInitialise;
    HWCTRL_pfnDeInitialise             pfnDeInitialise;
    HWCTRL_pfnDestroy                  pfnDestroy;
    HWCTRL_pfnPrepareFirmware          pfnPrepareFirmware;
    HWCTRL_pfnGetRegsOffsets           pfnGetRegsOffsets;
    HWCTRL_pfnGetMessage               pfnGetMessage;
    HWCTRL_pfnReturnMsg                pfnReturnMsg;
    HWCTRL_pfnProcessMessage           pfnProcessMessage;
#ifndef IMG_KERNEL_MODULE
    HWCTRL_pfnPDUMPUnlock              pfnPDUMPUnlock;
    HWCTRL_pfnPDUMPLock                pfnPDUMPLock;
    HWCTRL_pfnPDUMPSync                pfnPDUMPSync;
    HWCTRL_pfnPDUMPComment             pfnPDUMPComment;
#endif

} HWCTRL_sFunctions;

#ifdef __cplusplus
}
#endif

#endif /* __HWCTRL_INT_H__   */
