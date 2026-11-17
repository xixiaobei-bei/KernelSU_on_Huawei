/*!
 *****************************************************************************
 *
 * @File       hwctrl_api.h
 * @Title      VDECDD Hardware control API.
 * @Description    This file contains the VDECDD Hardware control API fucntions prototype.
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

#ifndef __HWCTRL_API_H__
#define __HWCTRL_API_H__

#include <img_defs.h>
#include "hwctrl_int.h"
#include "vdecdd_int.h"
#include "decoder_int.h"
#include "vxd.h"
#ifdef VDEC_USE_PVDEC_SEC
    #include "fwbsp_bridge.h"
#endif /* VDEC_USE_PVDEC_SEC */

#ifdef __cplusplus
extern "C" {
#endif

//Size of additional buffers needed for each HEVC picture
#ifdef HAS_HEVC

#define MEM_TO_REG_BUF_SIZE 0x2000

// Max. no. of slices found in stream db: approx. 2200,
// set MAX_SLICES to 2368 to get buffer size page aligned
#define MAX_SLICES 2368
#define SLICE_PARAMS_SIZE 64
#define SLICE_PARAMS_BUF_SIZE (MAX_SLICES * SLICE_PARAMS_SIZE)

// Size of buffer for "above params" structure, sufficient for stream of width 8192
// 192 * (8192/64) == 0x6000, see "above_param_size" in TRM
#define ABOVE_PARAMS_BUF_SIZE 0x6000
#endif /* HAS_HEVC */

/*!
******************************************************************************
 This structure contains the HWCTRL Core state.
 @brief  Hardware Control State
******************************************************************************/
typedef struct
{
    VXDIO_sState            sCoreState;

    HWCTRL_sMsgStatus       sFwMsgStatus;       /*!< MTX Message Status    - Read from FW           */
    HWCTRL_sMsgStatus       sHostMsgStatus;     /*!< MTX Message Status    - Inside HWctrl          */

    IMG_UINT32              ui32DWRRetry;       /*!< Retry count of device watch timer.             */

} HWCTRL_sState;

/*!
******************************************************************************

 @Function              HWCTRL_Initialise

 @Description

 This function prepares the state of hwctrl for a particular core.

 @Input     pvDecCore           : Pointer to opaque data relating to the core.

 @Input     pvCompInitUserData  : Callback user data from component. Used to callback back
                                  into the core api in case of no hardware.

 @Input     psDdDevConfig       : Pointer to device configuration.

 @Output    p                   : Pointer to VXD core properties.

 @Output    psRendec            : Pointer to rendec configuration.

 @Output    phHwCtx             : Pointer to Hwctrl context handle.

 @Return    IMG_RESULT  : This function returns either IMG_SUCCESS or an
                          error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_Initialise(
    IMG_VOID                  * pvDecCore,
    IMG_VOID                  * pvCompInitUserData,
    const VDECDD_sDdDevConfig * psDdDevConfig,
    VXD_sCoreProps            * psCoreProps,
    HWCTRL_sRendec            * psRendec,
    IMG_HANDLE                * phHwCtx
);


/*!
******************************************************************************

 @Function              HWCTRL_Deinitialise

 @Description

 This function shuts-down the hwctrl.

 @Input     hHwCtx      : Hwctrl context handle.

 @Return    IMG_RESULT  : This function returns either IMG_SUCCESS or an
                          error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_Deinitialise(
    IMG_HANDLE      hHwCtx
);


/*!
******************************************************************************

 @Function              HWCTRL_Configure

******************************************************************************/
extern IMG_RESULT
HWCTRL_Configure(
    IMG_HANDLE                  hHwCtx,
    const VXD_sFirmware       * psFirmware,
    const VXDIO_sDdBufInfo      aui32RendecBufinfo[],
    const VXDIO_sDdBufInfo    * psDevPtdBufInfo,
    const VXDIO_sPtdInfo      * psPtdInfo
);


/*!
******************************************************************************

 @Function              HWCTRL_CoreReplay

 @Description

 This function replays all outstanding pictures on a core.

 Usage: Used when doing device watch reset, resuming from a suspend, or when
 implementing stream destroy via core reset and replay.

 @Input     hHwCtx          : Hwctrl context handle

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT HWCTRL_CoreReplay(
    IMG_HANDLE  hHwCtx
);

/*!
******************************************************************************

 @Function              HWCTRL_PostCoreReplay

 @Description

 This function post a replay for a core to the core api.

 Usage: Used when doing device watch reset.

 @Input     hHwCtx          : Hwctrl context handle

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.
******************************************************************************/
extern IMG_RESULT HWCTRL_PostCoreReplay(
    IMG_HANDLE          hHwCtx
);

/*!
******************************************************************************

 @Function              HWCTRL_KickSwInterrupt

******************************************************************************/
extern IMG_RESULT
HWCTRL_KickSwInterrupt(
    IMG_HANDLE           hHwCtx,
    IMG_HANDLE           hDecServiceInt
);

/*!
******************************************************************************

@Function              HWCTRL_PictureSubmitBatch

 @Description

 This function is used to send a batch message to the fw. The batch message
 contains a decode message, a number of bitstream messages and an end bytes
 message.
 Implicitly this function loads the base component and all the prerequisites
 setup necessary for the core to be setup for decode.

 @Input     hHwCtx              : HwCtrl context handle

 @Input     psDecPict           : Pointer to decoder picture.

 @Input     hResources          : Core resources, contains buffers used to upload
                                  firmware framwork to MTX with DMA.

 @Input     psPtdInfo           : Base address of PTD related informations.


 @Return    IMG_RESULT          : This function returns either IMG_SUCCESS or an
                                  error code.

******************************************************************************/
IMG_RESULT HWCTRL_PictureSubmitBatch(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict,
    IMG_HANDLE          hResources,
    VXDIO_sPtdInfo    * psPtdInfo
);

/*!
******************************************************************************

 @Function              HWCTRL_PeekHeadPicList

 @Description

 This function returns the head of the sPendPictList in HwCtrl Ctx without
 removing it from the queue.

 @Input     hHwCtx          : handle to hwctrl context.

 @Output    ppsDecPict      : pointer to pointer to DECODER_sDecPict.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT HWCTRL_PeekHeadPicList(
    IMG_HANDLE           hHwCtx,
    DECODER_sDecPict **  ppsDecPict
);

/*!
******************************************************************************

 @Function              HWCTRL_GetDecodedPicture

 @Description

 This function returns first decoded picture from the sPendPictList in HwCtrl Ctx without
 removing it from the queue.

 @Input     hHwCtx          : handle to hwctrl context.

 @Output    ppsDecPict      : pointer to pointer to DECODER_sDecPict.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT HWCTRL_GetDecodedPicture(
    IMG_HANDLE           hHwCtx,
    DECODER_sDecPict **  ppsDecPict
);

/*!
******************************************************************************

 @Function              HWCTRL_DiscardHeadPicList

 @Description

 This function throws the head of the sPendPictList in HwCtrl Ctx.

 @Input     hHwCtx          : handle to hwctrl context.

 @Input     eReason         : reset reason

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT HWCTRL_DiscardHeadPicList(
    IMG_HANDLE          hHwCtx,
    VDECDD_eResetReason eReason
);

/*!
******************************************************************************

 @Function              HWCTRL_RemoveHeadPicList

 @Description

 This function returns the Head of the sPendPictList in HwCtrl Ctx and removes
 it from the queue.

 @Input     hHwCtx          : handle to hwctrl context.

 @Output    ppsDecPict      : pointer to pointer to DECODER_sDecPict.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_RemoveHeadPicList(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict ** ppsDecPict
);


/*!
******************************************************************************

 @Function              HWCTRL_RemoveFromPicList

 @Description

 This function returns the Head of the sPendPictList in HwCtrl Ctx and removes
 it from the queue.

 @Input     hHwCtx          : handle to hwctrl context.

 @Input     psDecPict       : pointer to DECODER_sDecPictm to be removed.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_RemoveFromPicList(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict *  psDecPict
);


#ifndef IMG_KERNEL_MODULE
/*!
******************************************************************************

 @Function              HWCTRL_SyncPDumpContexts

 @Description

 Sync pdump contexts

 @Input     phHwCtx             : Pointer to HwCtrl context handle

******************************************************************************/
extern IMG_RESULT
HWCTRL_SyncPDumpContexts(
    IMG_HANDLE          hHwCtx
);
#endif

/*!
******************************************************************************

 @Function              HWCTRL_GetCoreCachedStatus

 @Description

 This function returns the core last available status.

 @Input     hHwCtx          : Handle to hardware control context.

 @Output    psState         : Pointer to core state information.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

******************************************************************************/
extern IMG_UINT32
HWCTRL_GetCoreCachedStatus(
    IMG_HANDLE          hHwCtx,
    HWCTRL_sState     * psState
);

/*!
******************************************************************************

 @Function              HWCTRL_GetCoreStatus

 @Description

 This function returns the core status.

 @Input     hHwCtx          : Handle to hardware control context.

 @Output    pui32FreeSlots  : Pointer to number of free slots.

 @Output    pui32Load       : Pointer to core load.

 @Output    psState         : Pointer to core state information.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

******************************************************************************/
extern IMG_UINT32
HWCTRL_GetCoreStatus(
    IMG_HANDLE              hHwCtx,
    IMG_UINT32            * pui32FreeSlots,
    IMG_UINT32            * pui32Load,
    HWCTRL_sState         * psState
);


/*!
******************************************************************************

 @Function              HWCTRL_CoreFlushMmuCache

 @Description

 This function flushes the MMU cache.

 @Input     hHwCtx          : Handle to hardware control context.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_CoreFlushMmuCache(
    IMG_HANDLE  hHwCtx
);


/*!
******************************************************************************

 @Function              HWCTRL_HandleInterrupt

 @Description

 This function handles the device interrupts and makes a request for the decoder
 to later service the core interrupt.

 Usage: when an interrupt has been received from the core.

 @Input     hHwCtx      : Handle to hwctrl context.

 @Input     pvUserData  : Pointer to component wide user data.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_HandleInterrupt(
    IMG_HANDLE           hHwCtx,
    IMG_VOID *           pvUserData
);

/*!
******************************************************************************

 @Function              HWCTRL_PowerOff

 @Description

 This function resets msvdx, and disables the clocks. The core can be powered
 off after this.

 Usage: when we recieve an event from the OS that device is going off, or
 we want to swtich the core off for active power management.

 @Input     hHwCtx      : Handle to hwctrl context.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.
******************************************************************************/
extern IMG_RESULT
HWCTRL_PowerOff(
    IMG_HANDLE      hHwCtx
);

/*!
******************************************************************************

 @Function              HWCTRL_GetPicPendPictList
 @Description

 This function returns the decode pic on this core with the provided Tid.

 Usage: In low latency mode when we need to submit fragments, we'd get the
 decode picture from this fucntion, and attach the framgments.

 @Input     hHwCtx              : Handle to hwctrl context.

 @Input     ui32TransactionId   : Tid.

 @Output    ppsDecPict          : pointer to pointer to DECODER_sDecPict.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.
******************************************************************************/
extern IMG_RESULT
HWCTRL_GetPicPendPictList(
    IMG_HANDLE                  hHwCtx,
    IMG_UINT32                  ui32TransactionId,
    DECODER_sDecPict         ** ppsDecPict
);

extern IMG_BOOL
HWCTRL_HwStateIsLockUp(
    IMG_VOID * pvParam
);


/*!
******************************************************************************

 @Function              HWCTRL_PdumpComment

 @Description

 Inserts comment into primary PDUMP context (C0_MSVDX_out2.txt).

 @Input     hHwCtx              : HwCtrl context handle

 @Input     pszComment          : Pointer to comment string

 @Return    IMG_RESULT          : This function returns either IMG_SUCCESS or an
                                  error code.

******************************************************************************/
extern IMG_RESULT
HWCTRL_PdumpComment(
    IMG_HANDLE      hHwCtx,
    IMG_CHAR      * pszComment
);


/*!
******************************************************************************

 @Function              HWCTRL_PrintMSVDXProps

 @Description

 This function prints properties of MSVDX core to debug output.

 @Input     psCoreProps         : MSVDX core properties.

 @Return    None

******************************************************************************/
extern IMG_VOID
HWCTRL_PrintMSVDXProps(
    VXD_sCoreProps           * psCoreProps
);


/*!
******************************************************************************

 @Function              HWCTRL_GetRegsOffsets

 @Description

 This function is used to obtain offsets to register spaces, as specified
 in device configuration.

 @Input     hHwCtx              : HwCtrl context handle

 @Input     psRegsOffsets       : Register spaces offsets.


 @Return    IMG_RESULT          : This function returns either IMG_SUCCESS or an
                                  error code.

******************************************************************************/
IMG_RESULT
HWCTRL_GetRegsOffsets(
    IMG_HANDLE                 hHwCtx,
    DECODER_sRegsOffsets     * psRegsOffsets
);

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              HWCTRL_SetSecured

******************************************************************************/
extern IMG_VOID
HWCTRL_SetSecured(
    IMG_HANDLE           hHwCtx,
    IMG_BOOL             bSecureStream
);

/*!
******************************************************************************

@Function              HWCTRL_BistreamSubmit

 @Description

irekb: TUDU ADD desc

******************************************************************************/
IMG_RESULT HWCTRL_BistreamSubmit(
    IMG_HANDLE                  hHwCtx,
    IMG_HANDLE                  hFWBSP_BRIDGECtx,
    FWBSP_BRIDGE_sFwMsgData   * psFwMsgData,
    FWBSP_BRIDGE_sHostMsgData * psHostMsgData
);

/*!
******************************************************************************

@Function              HWCTRL_BistreamProcessed

 @Description

irekb: TUDU ADD desc

******************************************************************************/
IMG_RESULT
HWCTRL_BistreamProcessed(
    IMG_HANDLE                  hHwCtx
);

/*!
******************************************************************************

@Function              HWCTRL_HasFwBspLockUp

 @Description

irekb: TUDU ADD desc

******************************************************************************/
IMG_BOOL
HWCTRL_HasFwBspLockUp(
    IMG_HANDLE                  hHwCtx,
    IMG_UINT32                * pui32LockUpPID
);

/*!
******************************************************************************

@Function              HWCTRL_PictureSubmitSecure

 @Description

 irekb: TODO add desc.

 @Input     hHwCtx              : HwCtrl context handle

 @Input     psDecPict           : Pointer to decoder picture.

 @Input     hResources          : Core resources, contains buffers used to upload
                                  firmware framwork to MTX with DMA.

 @Input     psPtdInfo           : Base address of PTD related informations.


 @Return    IMG_RESULT          : This function returns either IMG_SUCCESS or an
                                  error code.

******************************************************************************/
IMG_RESULT HWCTRL_PictureSubmitSecure(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict,
    IMG_HANDLE          hResources,
    VXDIO_sPtdInfo    * psPtdInfo
);

#endif /* VDEC_USE_PVDEC_SEC */

#ifdef __cplusplus
}
#endif

#endif /* __HWCTRL_API_H__   */
