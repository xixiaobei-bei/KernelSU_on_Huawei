/*!
 *****************************************************************************
 *
 * @File       hwctrl_api.c
 * @Title      VDECDD Hardware control API.
 * @Description    This file contains the VDECDD Hardware control API.
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

#include "hwctrl_api.h"
#include "hwctrl_int.h"
#include "core_api.h"
#include "report_api.h"
#include "rman_api.h"
#include "sysos_api_km.h"
#include "vxd_ext.h"
#include "hwctrl_int.h"

#if defined VDEC_USE_PVDEC_SEC && defined SYSBRG_BRIDGING
#include "pvdecsec_int.h"
#endif
#include "pvdec_int.h"

#include "hwdefs/pvdec_core_regs.h"

#ifndef SYSBRG_BRIDGING
    #include "talmmu_api.h"
#else /* def SYSBRG_BRIDGING */
    #include "sysmem_utils.h"
#endif /* def SYSBRG_BRIDGING */

#ifdef USE_REAL_FW
    #include "vdecfw_bin.h"
#endif /* USE_REAL_FW */

/*!
******************************************************************************
 This structure contains the hwctrl context.
 @brief  Hardware Control Context
******************************************************************************/
typedef struct
{
    IMG_BOOL                bInitialised;           /*!< Indicates whether the core has been enumerated.                            */
    volatile IMG_BOOL       bPower;                 /*!< Indicating whether the core is powered up.                                 */

    IMG_UINT32              ui32NumPipes;
    VDECDD_sDdDevConfig     sDevConfig;             /*!< Device configuration.                                                      */

    IMG_HANDLE              hVxd;                   /*!< Handle to video decoder context.                                           */
    HWCTRL_sFunctions       sVxdFuncs;              /*!< Low level video decoder routines.                                          */

    IMG_VOID *              pvDecCore;
    IMG_VOID *              pvCompInitUserData;

    VXDIO_sDdBufInfo        aui32RendecBufInfo[VDEC_NUM_RENDEC_BUFFERS];  /*!< Device virtual address of rendec buffers.            */
    VXDIO_sDdBufInfo        sDevPtdBufInfo;                               /*!< Buffer to hold device Page Table directory address.  */

    // State
    LST_T                   sPendPictList;          /*!< List of pictures that are being decoded on this core.                      */
    IMG_UINT32              ui32NumFreeSlots[VDEC_MAX_PIXEL_PIPES];       /*!< Number of front-end decoding slots (in pictures).                          */
    IMG_UINT32              ui32Load[VDEC_MAX_PIXEL_PIPES];               /*!< Metric to indicate the fullness of the core.                               */
    HWCTRL_sMsgStatus       sHostMsgStatus;         /*!< Video decoder  message status.                                             */

    IMG_HANDLE              hTimerHandle;
    IMG_BOOL                bMonitorBusy;

    HWCTRL_sState           sState;                 /*!< Current HWCTRL state. In core context because it is too big for stack.     */
    HWCTRL_sState           sPrevState;
    IMG_BOOL                bPrevHwStateSet;

    // Firmware message.
    VXD_sMsgQueue           sMsgQueue;              /*!< The queue of firmware messages.                                            */
    VXD_sIntStatus          sIntStatus;
    VXDIO_sHISRMsg          asHISRMsg[MSVDX_MAX_LISR_HISR_MSG]; /*!< Messages passed between LISR and HISR.                         */

#if !defined(IMG_KERNEL_MODULE) && defined(VDEC_USE_PVDEC)
    IMG_UINT32              ui32NumAvailPipes;      /*!< Number of available pipes.                                                 */
#endif
    IMG_BOOL                bSecureStream;
#ifdef VDEC_USE_PVDEC_SEC
    FWBSP_BRIDGE_sFwMsgData * psFwMsgData;
    IMG_UINT32                ui32PendingBitStreams;  /*!< Keeps track of bit streams number submitted to firmware                    */
    IMG_BOOL                  bFwBspLockUp;           /*!< Indicator for FWBSP lockup in firmware side                                */
    IMG_BOOL                  ui32FwBspLockUpPID;     /*!< Parse ID that caused the lockup                                            */
#endif

} HWCTRL_sHwCtx;


/*****************************************************************************
 Local function prototypes.
******************************************************************************/
static IMG_VOID hwctrl_SetupDWR(
    HWCTRL_sHwCtx     * psHwCtx,
    SYSOSKM_pfnTimer    pfnTimer);

static IMG_RESULT hwctrl_PowerOn(
    IMG_HANDLE  hHwCtx);

static IMG_VOID hwctrl_DumpState(
    VXDIO_sState    * psPrevState,
    VXDIO_sState    * psCurState,
    VXD_sIntStatus  * psIntStatus,
    IMG_UINT8         ui8PipeMinus1);

static IMG_BOOL hwctrl_HwStateIsLockUp(
    HWCTRL_sHwCtx  * psHwCtx,
    IMG_BOOL         bLogState);

static IS_NOT_USED IMG_BOOL hwctrl_MonitorHwState(
    IMG_VOID  * pvParam);

static IMG_UINT32 hwctrl_CalcMbLoad(
    IMG_UINT32  ui32NumMbs,
    IMG_BOOL    bIntraCoded);

static IMG_UINT32 hwctrl_CalculateLoad(
    BSPP_sPictHdrInfo  * psPictHdrInfo);

static IMG_RESULT hwctrl_SendInitMessage(
    HWCTRL_sHwCtx               * psHwCtx,
    const VDECFW_sCoreInitData  * psCoreInitData,
    const VXDIO_sDdBufInfo      * psDevPtdBufInfo);

static IMG_RESULT hwctrl_SendBatchMessage(
    HWCTRL_sHwCtx     * psHwCtx,
    DECODER_sDecPict  * psDecPict);

static IMG_RESULT hwctrl_PictureReplay(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict);

static IMG_RESULT hwctrl_ProcessMTXMsgs(
    HWCTRL_sHwCtx  * psHwCtx,
    VXD_sMsgQueue  * psMsgQueue,
    IMG_VOID       * pvUserData);

static IMG_RESULT hwctrl_PowerOn(
    IMG_HANDLE  hHwCtx);

#ifdef VDEC_USE_PVDEC_SEC
static IMG_RESULT hwctrl_SendFWSECInitMessage(
    HWCTRL_sHwCtx               * psHwCtx,
    const VDECFW_sCoreInitData  * psCoreInitData,
    const VXDIO_sDdBufInfo      * psDevPtdBufInfo);

static IMG_RESULT hwctrl_SendFWDECDecodeMessage(
    HWCTRL_sHwCtx           * psHwCtx,
    DECODER_sDecPict        * psDecPict);

static IMG_RESULT hwctrl_SendParseBitstreamMessage(
    HWCTRL_sHwCtx           * psHwCtx,
    IMG_HANDLE                hFWBSP_BRIDGECtx,
    FWBSP_BRIDGE_sFwMsgData * psFwMsgData);

#endif /* VDEC_USE_PVDEC_SEC */

static IMG_VOID
hwctrl_SetupDWR(
    HWCTRL_sHwCtx     * psHwCtx,
    SYSOSKM_pfnTimer    pfnTimer
)
{
    if (psHwCtx->hTimerHandle == IMG_NULL)
    {
        if (psHwCtx->sDevConfig.ui32DwrPeriod)
        {
            SYSOSKM_CreateTimer(pfnTimer, psHwCtx, psHwCtx->sDevConfig.ui32DwrPeriod, &psHwCtx->hTimerHandle);
        }
    }

    psHwCtx->bMonitorBusy = IMG_FALSE;
    psHwCtx->bPrevHwStateSet = IMG_FALSE;
    psHwCtx->sPrevState.ui32DWRRetry = 0;
}

static IMG_VOID
hwctrl_DumpState(
    VXDIO_sState    * psPrevState,
    VXDIO_sState    * psCurState,
    VXD_sIntStatus  * psIntStatus,
    IMG_UINT8         ui8PipeMinus1
)
{
    // XXX: TODO[ML]: This has to be updated for multi-pipe cases.
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Back-End MbX                          [% 10d]", psPrevState->sFwState.asPipeState[ui8PipeMinus1].sBeMb.ui32X);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Back-End MbY                          [% 10d]", psPrevState->sFwState.asPipeState[ui8PipeMinus1].sBeMb.ui32Y);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Front-End MbX                         [% 10d]", psPrevState->sFwState.asPipeState[ui8PipeMinus1].sFeMb.ui32X);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Front-End MbY                         [% 10d]", psPrevState->sFwState.asPipeState[ui8PipeMinus1].sFeMb.ui32Y);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_BE_PICTURE_COMPLETE [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_BE_PICTURE_COMPLETE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_BE_1SLICE_DONE      [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_BE_1SLICE_DONE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_BE_PICTURE_STARTED  [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_BE_PICTURE_STARTED]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_FE_PICTURE_COMPLETE [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_FE_PICTURE_COMPLETE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_FE_PARSE_DONE       [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_FE_PARSE_DONE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_FE_1SLICE_DONE      [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_FE_1SLICE_DONE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_ENTDEC_STARTED      [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_ENTDEC_STARTED]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_FIRMWARE_SAVED      [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_FIRMWARE_SAVED]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_PICMAN_COMPLETE     [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_PICMAN_COMPLETE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_SYNC_COMPLETE       [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_SYNC_COMPLETE]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_FIRMWARE_READY      [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_FIRMWARE_READY]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "VDECFW_CHECKPOINT_PICTURE_STARTED     [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_PICTURE_STARTED]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "eFirmwareAction                       [0x%08X]", psCurState->sFwState.asPipeState[ui8PipeMinus1].eFirmwareAction);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "DMACStatus                            [0x%08X]", psPrevState->sFwState.asPipeState[ui8PipeMinus1].aui32DMACStatus[0]);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Interrupt Status                      [0x%08X]", psIntStatus->ui32Pending);
}

static IMG_BOOL
hwctrl_HwStateIsLockUp(
    HWCTRL_sHwCtx  * psHwCtx,
    IMG_BOOL         bLogState
)
{
    IMG_BOOL bReturnVal = IMG_FALSE;

    // Check if the HW State indicates a LockUp
    if (!LST_empty(&psHwCtx->sPendPictList)
#ifdef VDEC_USE_PVDEC_SEC
        || psHwCtx->ui32PendingBitStreams
#endif
       )
    {
        IMG_UINT32 aui32NumFreeSlots[VDEC_MAX_PIXEL_PIPES];
        IMG_UINT32 aui32PipeLoad[VDEC_MAX_PIXEL_PIPES];

        HWCTRL_GetCoreStatus(psHwCtx, aui32NumFreeSlots, aui32PipeLoad, &psHwCtx->sState);

        /* Check for firmware PANIC. */
        if (psHwCtx->sState.sCoreState.sFwState.bPanic)
        {
#ifdef VDEC_USE_PVDEC_SEC
            psHwCtx->bFwBspLockUp = IMG_TRUE;
            psHwCtx->ui32FwBspLockUpPID = 0;
#endif
            bReturnVal = IMG_TRUE;
        }
        // Make sure we actually have set the Previous State once at least before checking if it has been remained unmodified
        // Without this we may compare the first state to uninitialised state
        else if (psHwCtx->bPrevHwStateSet)
        {
            VXDIO_sState * psCurState = &psHwCtx->sState.sCoreState;
            VXDIO_sState * psPrevState = &psHwCtx->sPrevState.sCoreState;
            IMG_UINT8      uiPipeMinus1 = 0;

            for(uiPipeMinus1 = 0; uiPipeMinus1 < psHwCtx->ui32NumPipes; uiPipeMinus1++)
            {
                bReturnVal = IMG_FALSE;
                // If HW state has not been modified for this core since the last time and there
                // is no progress of data transfer to SR
                if ( (psPrevState->sFwState.asPipeState[uiPipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_PICTURE_STARTED] ==
                      psCurState->sFwState.asPipeState[uiPipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_PICTURE_STARTED]) &&
                     (psPrevState->sFwState.asPipeState[uiPipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_BE_PICTURE_COMPLETE] ==
                      psCurState->sFwState.asPipeState[uiPipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_BE_PICTURE_COMPLETE]) &&
                     (psCurState->sFwState.asPipeState[uiPipeMinus1].aui32DMACStatus[0] == psPrevState->sFwState.asPipeState[uiPipeMinus1].aui32DMACStatus[0])
                   )
               {
                    VDECFW_eCodecType eCodec = (VDECFW_eCodecType) psCurState->sFwState.asPipeState[uiPipeMinus1].ui8CurCodec;
                    /* We support large JPEGs, which can be encoded in one slice. There is no way to determine if
                       HW is locked up in this case (latest MBs are not updated in ENTDEC_INFORMATION register
                       for JPEG), so retry timer <psHwCtx->sDevConfig.ui32DwrJPEGRetry> times. */
                    if (eCodec == VDECFW_CODEC_JPEG)
                    {
                        if (psHwCtx->sPrevState.ui32DWRRetry < psHwCtx->sDevConfig.ui32DwrJPEGRetry)
                        {
                            psHwCtx->sState.ui32DWRRetry++;
                        }
                        else
                        {
                            bReturnVal = IMG_TRUE;
                        }
                    }
#ifdef VDEC_USE_PVDEC_SEC
                    // Test for fwbsp progress
                    else if (psHwCtx->bSecureStream &&
                             psCurState->sFwState.asPipeState[uiPipeMinus1].eFirmwareAction > VDECFW_FWACT_IDLE &&
                             psPrevState->sFwState.asPipeState[uiPipeMinus1].eFwBspStep == psCurState->sFwState.asPipeState[uiPipeMinus1].eFwBspStep)
                    {
                            psHwCtx->bFwBspLockUp = IMG_TRUE;
                            psHwCtx->ui32FwBspLockUpPID = psCurState->sFwState.asPipeState[uiPipeMinus1].aui32CheckPoint[VDECFW_CHECKPOINT_PICTURE_STARTED];
                            bReturnVal = IMG_TRUE;
                    }
                    else if (LST_empty(&psHwCtx->sPendPictList) &&
                             psHwCtx->bSecureStream)
                    {
                        /* No need to check FE/BE when pic list empty for secure case */
                        break;
                    }
#endif
                    // Now test for backend macroblock number
                    else if ( (psCurState->sFwState.asPipeState[uiPipeMinus1].sBeMb.ui32X == psPrevState->sFwState.asPipeState[uiPipeMinus1].sBeMb.ui32X) &&
                              (psCurState->sFwState.asPipeState[uiPipeMinus1].sBeMb.ui32Y == psPrevState->sFwState.asPipeState[uiPipeMinus1].sBeMb.ui32Y) &&
                              (psCurState->sFwState.asPipeState[uiPipeMinus1].sFeMb.ui32X == psPrevState->sFwState.asPipeState[uiPipeMinus1].sFeMb.ui32X) &&
                              (psCurState->sFwState.asPipeState[uiPipeMinus1].sFeMb.ui32Y == psPrevState->sFwState.asPipeState[uiPipeMinus1].sFeMb.ui32Y)
                            )
                    {
                        IMG_UINT32 i;
                        bReturnVal = IMG_TRUE;

                        // Check that we haven't received any messages since last time
                        for (i = 0; i < VDECFW_MSGID_COMPLETION_TYPES; i++)
                        {
                            if ((psHwCtx->sState.sFwMsgStatus.aui8CompletionFenceID[i] !=
                                psHwCtx->sPrevState.sFwMsgStatus.aui8CompletionFenceID[i]) ||
                                (psHwCtx->sState.sHostMsgStatus.aui8CompletionFenceID[i] !=
                                psHwCtx->sPrevState.sHostMsgStatus.aui8CompletionFenceID[i]))
                            {
                                bReturnVal = IMG_FALSE;
                                break;
                            }
                        }
                        if ((psPrevState->sFwState.asPipeState[uiPipeMinus1].ui32BeSlices != psCurState->sFwState.asPipeState[uiPipeMinus1].ui32BeSlices) ||
                            (psPrevState->sFwState.asPipeState[uiPipeMinus1].ui32FeSlices != psCurState->sFwState.asPipeState[uiPipeMinus1].ui32FeSlices))
                        {
                            bReturnVal = IMG_FALSE;
                        }

#ifdef VDEC_USE_PVDEC_SEC
                        // We may still need to check fwbsp progress to call away false alarm.
                        if (psHwCtx->bSecureStream &&
                            psPrevState->sFwState.asPipeState[uiPipeMinus1].eFwBspStep != psCurState->sFwState.asPipeState[uiPipeMinus1].eFwBspStep)
                        {
                            bReturnVal = IMG_FALSE;
                        }
#endif
                    }

                }

                //there was some action on current pipe, not need to check others
                if(bReturnVal == IMG_FALSE)
                {
                    break;
                }
            }

            if (bReturnVal)
            {
                VXD_sIntStatus  sIntStatus;
                IMG_UINT32      ui32Result;
                IMG_UINT8       uiPipeMinus1 = 0;

                IMG_MEMSET(&sIntStatus, 0, sizeof(sIntStatus));

                /* Lets dump some useful debug information from the Hardware */
                IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetIntStatus != IMG_NULL);
                if (psHwCtx->sVxdFuncs.pfnGetIntStatus)
                {
                    ui32Result = psHwCtx->sVxdFuncs.pfnGetIntStatus(psHwCtx->hVxd, &sIntStatus);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return bReturnVal;
                    }
                }

                if(bLogState)
                {
                    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "====================================================");
                    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "There might be a core lock-up.");

                    for(uiPipeMinus1 = 0; uiPipeMinus1 < psHwCtx->ui32NumPipes; uiPipeMinus1++)
                    {
                        if (psCurState->sFwState.asPipeState[0].ui32IntStatus & MSVDX_CORE_CR_MSVDX_INTERRUPT_STATUS_CR_MTX_COMMAND_TIMEOUT_IRQ_MASK)
                        {
                            REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Possibly due to MTX command timeout on pipe %d", uiPipeMinus1 + 1);
                        }
                    }

                    if (psHwCtx->sVxdFuncs.pfnGetIntStatus)
                    {
                        for(uiPipeMinus1 = 0; uiPipeMinus1 < psHwCtx->ui32NumPipes; uiPipeMinus1++)
                        {
                            REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Dumping debug info for pipe %d:", uiPipeMinus1 + 1);
                            hwctrl_DumpState(psPrevState, psCurState, &sIntStatus, uiPipeMinus1);
                        }
                    }
                    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "====================================================");

                    /* Now lets dump The complete Register spaces for CORE, MTX, VEC, VDMC, VDEB, DMAC and VLR. Also Dump RTM register for HW Debug. */
                    if (psHwCtx->sDevConfig.bCoreDump)
                    {
                        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnDumpRegisters != IMG_NULL);
                        if (psHwCtx->sVxdFuncs.pfnDumpRegisters)
                        {
                            ui32Result = psHwCtx->sVxdFuncs.pfnDumpRegisters(psHwCtx->hVxd);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return bReturnVal;
                            }
                        }
                    }
                }
            }
        }

        psHwCtx->bPrevHwStateSet = IMG_TRUE;
        IMG_MEMCPY( &psHwCtx->sPrevState, &psHwCtx->sState, sizeof(HWCTRL_sState) );
    }

    return bReturnVal;
}


IMG_BOOL HWCTRL_HwStateIsLockUp(
    IMG_VOID  * pvParam
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *)pvParam;

    if(hwctrl_HwStateIsLockUp(psHwCtx, IMG_TRUE))
    {
        return IMG_TRUE;
    }
    else
    {
        psHwCtx->bMonitorBusy = IMG_FALSE;
        return IMG_FALSE;
    }
}

static IS_NOT_USED IMG_BOOL
hwctrl_MonitorHwState(
    IMG_VOID  * pvParam
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *)pvParam;
    IMG_UINT32 ui32Result;

    // bMonitorBusy is used to detect if we are already in a process of resetting the core
    if (psHwCtx->bMonitorBusy)
    {
        // ALWAYS return true if you want the timer to keep running
        return IMG_TRUE;
    }

    // Check for state and reset if needed
    if (hwctrl_HwStateIsLockUp(psHwCtx, IMG_FALSE))
    {
        psHwCtx->bMonitorBusy = IMG_TRUE;

        // Do the Device Reset
        // ...
        ui32Result = CORE_DevServiceTimeExpiry(psHwCtx->pvCompInitUserData);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            //return ui32Result;
        }
    }

    // ALWAYS return true if you want the timer to keep running
    return IMG_TRUE;
}


#ifdef VDEC_MSVDX_HARDWARE

/*!
******************************************************************************

 @Function              hwctrl_CalcMbLoad

******************************************************************************/
static IMG_UINT32
hwctrl_CalcMbLoad(
    IMG_UINT32  ui32NumMbs,
    IMG_BOOL    bIntraCoded
)
{
    IMG_UINT32 ui32Load = 0;

    // XXX: improve load to take picture type into account.
    ui32Load = ui32NumMbs;
    if (bIntraCoded)
    {
        //ui32Load = (ui32Load*3)/2;
    }

    return ui32Load;
}


/*!
******************************************************************************

 @Function              hwctrl_CalculateLoad

******************************************************************************/
static IMG_UINT32
hwctrl_CalculateLoad(
    BSPP_sPictHdrInfo  * psPictHdrInfo
)
{
    IMG_UINT32 ui32NumMbs;

    ui32NumMbs = ((psPictHdrInfo->sCodedFrameSize.ui32Width+15)/16) * ((psPictHdrInfo->sCodedFrameSize.ui32Height+15)/16);

    return hwctrl_CalcMbLoad(ui32NumMbs, psPictHdrInfo->bIntraCoded);
}


/*!
******************************************************************************

 @Function              hwctrl_SendInitMessage

 @Description

 This function sends the init message to the firmware.

******************************************************************************/
static IMG_RESULT
hwctrl_SendInitMessage(
    HWCTRL_sHwCtx               * psHwCtx,
    const VDECFW_sCoreInitData  * psCoreInitData,
    const VXDIO_sDdBufInfo      * psDevPtdBufInfo
)
{
    IMG_RESULT ui32Result;

    /* Create a control picture message here from the config. */
    IMG_UINT8 * pui8Msg = IMG_NULL;

    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage != IMG_NULL);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        HWCTRL_sMsgData sMsgData = {{{0}}};
        sMsgData.sInitNonSecDecMsgData.psRendecBufInfo = psHwCtx->aui32RendecBufInfo;

        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_INIT_NON_SEC_DEC,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to get init message");
            return ui32Result;
        }
        IMG_ASSERT(pui8Msg != IMG_NULL);

        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[MID=0x%02X] [CONTROL] [0x%02X]",
            psHwCtx->sHostMsgStatus.aui8ControlFenceID[VDECFW_MSGID_FIRMWARE_INIT-VDECFW_MSGID_BASE_PADDING], VDECFW_MSGID_FIRMWARE_INIT);

#ifndef IMG_KERNEL_MODULE
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, "<+SendInitMsg>");
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
            return ui32Result;
        }
#endif

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnSendFirmwareMessage != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd, pui8Msg);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

#ifndef IMG_KERNEL_MODULE
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, "<-SendInitMsg>");
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
            return ui32Result;
        }
#endif

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnReturnMsg)
        {
            psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
        }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              hwctrl_SendBatchMessage

 @Description

 This function sends a batch message that contains a decode message,
 a number of bitstream buffer messages and the end bytes.

******************************************************************************/
static IMG_RESULT
hwctrl_SendBatchMessage(
    HWCTRL_sHwCtx     * psHwCtx,
    DECODER_sDecPict  * psDecPict
)
{
    IMG_RESULT              ui32Result;
    IMG_UINT8             * pui8Msg = IMG_NULL;
    VXDIO_sDdBufInfo      * psStrPtdBufInfo = psDecPict->psStrPtdBufInfo;
    VXDIO_sDdBufInfo      * psBatchMsgBufInfo = psDecPict->psBatchMsgInfo->psDdBufInfo;
    HWCTRL_sMsgData         sMsgData = {{{0}}};
    HWCTRL_sBatchMsgData  * psBatchMsgData = &sMsgData.sBatchMsgData;

    psBatchMsgData->psBatchMsgBufInfo    = psBatchMsgBufInfo;
    psBatchMsgData->psStrPtdBufInfo      = psStrPtdBufInfo;

    psBatchMsgData->ui32TransactionId    = psDecPict->ui32TransactionId;
    psBatchMsgData->psPvdecFwCtx         = psDecPict->psStrPvdecFwCtxBuf;
    psBatchMsgData->ui32CtrlAllocBytes   = psDecPict->ui32CtrlAllocBytes;
    psBatchMsgData->ui32OperatingMode    = psDecPict->ui32OperatingMode;
    psBatchMsgData->ui8Pipe              = psDecPict->ui8Pipe;
    psBatchMsgData->ui16GencId           = psDecPict->ui16GencId;
    psBatchMsgData->bSizeDelimitedMode   = (psDecPict->psPictHdrInfo->eParserMode != VDECFW_SCP_ONLY) ? (IMG_TRUE) : (IMG_FALSE) ;
    psBatchMsgData->ui32TileCfg          = psDecPict->ui32TileCfg;

    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage != IMG_NULL);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_BATCH,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to get decode message");
            return ui32Result;
        }
        IMG_ASSERT(pui8Msg != IMG_NULL);

        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[MID=0x%02X] [DECODE] [0x%02X]", 0, VDECFW_MSGID_BATCH);

#ifndef IMG_KERNEL_MODULE
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, "<+SendBatchMsg>");
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
            return ui32Result;
        }
#endif

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnSendFirmwareMessage != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd, pui8Msg);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

#ifndef IMG_KERNEL_MODULE
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, "<-SendBatchMsg>");
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
            return ui32Result;
        }
#endif

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnReturnMsg)
        {
            psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
        }
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              hwctrl_PictureReplay

******************************************************************************/
static IMG_RESULT
hwctrl_PictureReplay(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            IMG_RESULT ui32Result;

            ui32Result = hwctrl_PowerOn(psHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
#ifdef VDEC_USE_PVDEC_SEC
            if (psHwCtx->bSecureStream)
            {
                ui32Result = hwctrl_SendFWDECDecodeMessage(psHwCtx, psDecPict);
            }
            else
#endif /* VDEC_USE_PVDEC_SEC */
            {
                ui32Result = hwctrl_SendBatchMessage(psHwCtx, psDecPict);
            }
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }
#endif
    }

    return IMG_SUCCESS;
}


/*
******************************************************************************

 @Function              decoder_ProcessMTXMsgs

 @Description

 This function is used for now to read through firmware messages and send Dev HW Interrupt.

 @Input     psDdDevContext  : Pointer to hwctrl context.

 @Input     pvUserData      : Pointer to component wide user data.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
static IMG_RESULT
hwctrl_ProcessMTXMsgs(
    HWCTRL_sHwCtx     * psHwCtx,
    VXD_sMsgQueue     * psMsgQueue,
    IMG_VOID          * pvUserData
)
{
    IMG_RESULT eResult;

    VXDIO_sHISRMsg *  psHISRMsg;
    VXDIO_sHISRMsg    sHISRMsg;

    IMG_BOOL  bDecodedMsg;

    /* Process all pending HISR messages. */
    psHISRMsg = LST_first(&psMsgQueue->sNewMsgList);
    while (psHISRMsg != IMG_NULL)
    {
        // Make a local copy of the message.
        sHISRMsg = *psHISRMsg;

        /* Remove the message from the queue...*/
        psHISRMsg = LST_removeHead(&psMsgQueue->sNewMsgList);
        if (psHISRMsg != IMG_NULL)
        {
            /* Process the fw message and populate the Pic */
            bDecodedMsg = IMG_FALSE;
            IMG_ASSERT(psHwCtx->sVxdFuncs.pfnProcessMessage != IMG_NULL);
            if (psHwCtx->sVxdFuncs.pfnProcessMessage)
            {
                eResult = psHwCtx->sVxdFuncs.pfnProcessMessage(psHwCtx->hVxd,
                                                               &psMsgQueue->aui32MsgBuf[sHISRMsg.ui32MsgIndex],
                                                               &psHwCtx->sPendPictList,
                                                               psHwCtx->hTimerHandle,
                                                               &bDecodedMsg);
                IMG_ASSERT(eResult == IMG_SUCCESS);
                if (eResult != IMG_SUCCESS)
                {
                    /* Free the message */
                    LST_add(&psMsgQueue->sFreeMsgList, psHISRMsg);
                    return eResult;
                }

#ifdef VDEC_USE_PVDEC_SEC
                if (psHwCtx->bSecureStream)
                {
                    eResult = CORE_DevProcessFwMsg((VDECDD_sDdDevContext *)pvUserData);
                    IMG_ASSERT(eResult == IMG_SUCCESS);
                    if (eResult != IMG_SUCCESS)
                    {
                        return eResult;
                    }
                }
#endif /* VDEC_USE_PVDEC_SEC */

                if (sHISRMsg.ui32MsgIndex < psMsgQueue->ui32ReadIdx)
                {
                    // We assume here that the buffer wrapped around and this is why the ui32MsgIndex < ui32ReadIdx
                    // so the message should start at the beginning of the buffer. Otherwise the ReadIdx should be set
                    // at the value of ui32MsgIndex;
                    IMG_ASSERT(sHISRMsg.ui32MsgIndex == 0);
                    // When the message didn't fit in the last portion of message buffer
                    // move the read index to the start of the message.
                    psMsgQueue->ui32ReadIdx = 0;
                }

                psMsgQueue->ui32ReadIdx += sHISRMsg.ui32MsgSize;

                /* Free the message */
                LST_add(&psMsgQueue->sFreeMsgList, psHISRMsg);

                if (bDecodedMsg)
                {
#ifndef IMG_KERNEL_MODULE
                    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnPDUMPUnlock != IMG_NULL);
                    if (psHwCtx->sVxdFuncs.pfnPDUMPUnlock)
                    {
                        eResult = psHwCtx->sVxdFuncs.pfnPDUMPUnlock(psHwCtx->hVxd);
                        IMG_ASSERT(eResult == IMG_SUCCESS);
                        if (eResult != IMG_SUCCESS)
                        {
                            return eResult;
                        }
                    }
#endif
                    CORE_DevHwInterrupt(pvUserData, psHwCtx->pvDecCore);
                }
            }
        }

        /* Get the next message...*/
        psHISRMsg = LST_first(&psMsgQueue->sNewMsgList);
    }

    return IMG_SUCCESS;
}

#endif // VDEC_MSVDX_HARDWARE

#ifndef IMG_KERNEL_MODULE
/*!
******************************************************************************

 @Function              HWCTRL_SyncPDumpContexts

 @Description

 Sync pdump contexts

******************************************************************************/
IMG_RESULT
HWCTRL_SyncPDumpContexts(
    IMG_HANDLE          hHwCtx
)
{
    HWCTRL_sHwCtx         * psHwCtx;
    IMG_UINT32              ui32Result = IMG_SUCCESS;

    IMG_ASSERT(hHwCtx != IMG_NULL);
    if(hHwCtx == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psHwCtx = (HWCTRL_sHwCtx *)hHwCtx;

#ifdef VDEC_MSVDX_HARDWARE
    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnPDUMPSync);
    if (psHwCtx->sVxdFuncs.pfnPDUMPSync)
    {
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPSync(psHwCtx->hVxd);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
#endif

    return ui32Result;
}
#endif

/*!
******************************************************************************

 @Function              HWCTRL_GetCoreCachedStatus

******************************************************************************/
IMG_UINT32
HWCTRL_GetCoreCachedStatus(
    IMG_HANDLE          hHwCtx,
    HWCTRL_sState     * psState
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    IMG_ASSERT(psState != IMG_NULL);

    if (psHwCtx->bPrevHwStateSet)
    {
        IMG_MEMCPY(psState, &psHwCtx->sPrevState, sizeof(HWCTRL_sState));
    }
    else
    {
        return IMG_ERROR_UNEXPECTED_STATE;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_GetCoreStatus

******************************************************************************/
IMG_UINT32
HWCTRL_GetCoreStatus(
    IMG_HANDLE          hHwCtx,
    IMG_UINT32          aui32FreeSlots[VDEC_MAX_PIXEL_PIPES],
    IMG_UINT32          aui32Load[VDEC_MAX_PIXEL_PIPES],
    HWCTRL_sState     * psState
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32      ui32Result = IMG_SUCCESS;

    IMG_MEMCPY(aui32FreeSlots,
               psHwCtx->ui32NumFreeSlots,
               VDEC_MAX_PIXEL_PIPES*sizeof(aui32FreeSlots[0]));

    IMG_MEMSET(aui32Load, 0, VDEC_MAX_PIXEL_PIPES*sizeof(aui32Load[0]));

#ifdef VDEC_MSVDX_HARDWARE
    if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER &&
        psHwCtx->bPower &&
        psHwCtx->sVxdFuncs.pfnGetCoreState != IMG_NULL)
    {
        DECODER_sDecPict  *  psDecPict;
        IMG_UINT32           ui32PictLoadDone = 0;
        VXDIO_sState         sCoreState;
        VXDIO_sState *       psCoreState = IMG_NULL;

        if (IMG_NULL == psState)
        {
            psCoreState = &sCoreState;
        }
        else
        {
            psCoreState  = &psState->sCoreState;
        }

        VDEC_BZERO(psCoreState);
        ui32Result = psHwCtx->sVxdFuncs.pfnGetCoreState(psHwCtx->hVxd, psCoreState);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        if (IMG_NULL != psState)
        {
            IMG_MEMCPY(psState->sFwMsgStatus.aui8ControlFenceID,
                       psState->sCoreState.sFwState.aui8ControlFenceID,
                       sizeof(psState->sFwMsgStatus.aui8ControlFenceID));
            IMG_MEMCPY(psState->sFwMsgStatus.aui8DecodeFenceID,
                       psState->sCoreState.sFwState.aui8DecodeFenceID,
                       sizeof(psState->sFwMsgStatus.aui8DecodeFenceID));
            IMG_MEMCPY(psState->sFwMsgStatus.aui8CompletionFenceID,
                       psState->sCoreState.sFwState.aui8CompletionFenceID,
                       sizeof(psState->sFwMsgStatus.aui8CompletionFenceID));

            IMG_MEMCPY(psState->sHostMsgStatus.aui8ControlFenceID,
                       psHwCtx->sHostMsgStatus.aui8ControlFenceID,
                       sizeof(psState->sHostMsgStatus.aui8ControlFenceID));
            IMG_MEMCPY(psState->sHostMsgStatus.aui8DecodeFenceID,
                       psHwCtx->sHostMsgStatus.aui8DecodeFenceID,
                       sizeof(psState->sHostMsgStatus.aui8DecodeFenceID));
            IMG_MEMCPY(psState->sHostMsgStatus.aui8CompletionFenceID,
                       psHwCtx->sHostMsgStatus.aui8CompletionFenceID,
                       sizeof(psState->sHostMsgStatus.aui8CompletionFenceID));

            psState->ui32DWRRetry = psHwCtx->sPrevState.ui32DWRRetry;
        }

        // Obtain up-to-date load based upon the progress of the current picture(s) on the back-end.
        {
            IMG_BOOL abPipeFound[VDEC_MAX_PIXEL_PIPES] = {0};
            psDecPict = LST_first(&psHwCtx->sPendPictList);
            while (psDecPict)
            {
                if(abPipeFound[psDecPict->ui8Pipe-1] == IMG_FALSE)
                {
                    IMG_UINT32 ui32NumMbProcessed;
                    IMG_UINT32 ui32WidthInMb;
                    if (psDecPict->psPictHdrInfo == NULL)
                    {
                        psDecPict = LST_next(psDecPict);
                        continue;
                    }
                    ui32WidthInMb = (psDecPict->psPictHdrInfo->sCodedFrameSize.ui32Width + (VDEC_MB_DIMENSION - 1)) / VDEC_MB_DIMENSION;

                    ui32NumMbProcessed = (sCoreState.sFwState.asPipeState[psDecPict->ui8Pipe-1].sBeMb.ui32Y * ui32WidthInMb) +
                                                                            sCoreState.sFwState.asPipeState[psDecPict->ui8Pipe-1].sBeMb.ui32X;
                    ui32PictLoadDone = hwctrl_CalcMbLoad(ui32NumMbProcessed, psDecPict->psPictHdrInfo->bIntraCoded);
                    aui32Load[psDecPict->ui8Pipe-1] = psHwCtx->ui32Load[psDecPict->ui8Pipe-1] - ui32PictLoadDone;
                    abPipeFound[psDecPict->ui8Pipe-1] = IMG_TRUE;
                }

                psDecPict = LST_next(psDecPict);
            }

            IMG_MEMCPY(aui32Load, psHwCtx->ui32Load, VDEC_MAX_PIXEL_PIPES*sizeof(aui32Load[0]));
        }
    }

error:
#endif
    return ui32Result;
}


/*!
******************************************************************************

 @Function              HWCTRL_CoreFlushMmuCache

******************************************************************************/
IMG_RESULT
HWCTRL_CoreFlushMmuCache(
    IMG_HANDLE  hHwCtx
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32      ui32Result;

    if (psHwCtx->bInitialised)
    {
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER && psHwCtx->bPower)
        {
            IMG_ASSERT(psHwCtx->sVxdFuncs.pfnFlushMmuCache != IMG_NULL);
            if (psHwCtx->sVxdFuncs.pfnFlushMmuCache)
            {
                ui32Result = psHwCtx->sVxdFuncs.pfnFlushMmuCache(psHwCtx->hVxd);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    goto error;
                }
            }
        }
    }

    return IMG_SUCCESS;

error:
    return ui32Result;
}


/*!
******************************************************************************

 @Function              HWCTRL_HandleInterrupt

 @Description

 This function handles the device interrupts and makes a request for the decoder
 to later service the core interrupt.

******************************************************************************/
IMG_RESULT
HWCTRL_HandleInterrupt(
    IMG_HANDLE  hHwCtx,
    IMG_VOID *  pvUserData
)
{
    IMG_UINT32          ui32Result = IMG_SUCCESS;

#ifdef VDEC_MSVDX_HARDWARE
    HWCTRL_sHwCtx     * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_BOOL            bMtxIrq;

    IMG_ASSERT(hHwCtx);

    // We want to make sure Power has not been turned off and this interrupt needs to be ignored.
    // Also need to make sure we are configured to handle this.
    if (psHwCtx->bPower)
    {
        VXD_sIntStatus* psIntStatus = &psHwCtx->sIntStatus;

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnHandleInt != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnHandleInt)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnHandleInt(psHwCtx->hVxd, psIntStatus);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);

            if (psIntStatus->ui32Pending & PVDEC_CORE_CR_PVDEC_HOST_INTERRUPT_STATUS_CR_HOST_MMU_FAULT_IRQ_MASK)
            {
                IMG_UINT32     aui32NumFreeSlots[VDEC_MAX_PIXEL_PIPES];
                IMG_UINT32     aui32PipeLoad[VDEC_MAX_PIXEL_PIPES];
                HWCTRL_sState  sState;
                IMG_UINT8      uiPipeMinus1 = 0;

                // Report the fault.
                REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE,
                       "MMU %s fault from %s %s @ 0x%08X",
                       (psIntStatus->MMU_PF_N_RW) ? "PAGE" : "PROTECTION",
                           (psIntStatus->ui32Requestor & (1<<0)) ? "DMAC" :
                           (psIntStatus->ui32Requestor & (1<<1)) ? "VEC"  :
                           (psIntStatus->ui32Requestor & (1<<2)) ? "VDMC" :
                           (psIntStatus->ui32Requestor & (1<<3)) ? "VDEB" :
                                                                   "INVALID",
                       (psIntStatus->MMU_FAULT_RNW) ? "READING" : "WRITING",
                       psIntStatus->MMU_FAULT_ADDR);

                if (psIntStatus->MMU_SECURE_FAULT)
                {
                    REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                           "MMU secure transaction policy violation detected !");
                    IMG_ASSERT(!"MMU security policy violation detected !");
                }

                HWCTRL_GetCoreStatus(psHwCtx, aui32NumFreeSlots, aui32PipeLoad, &sState);

                psHwCtx->bPrevHwStateSet = IMG_TRUE;
                IMG_MEMCPY( &psHwCtx->sPrevState, &sState, sizeof(HWCTRL_sState) );

                for(uiPipeMinus1 = 0; uiPipeMinus1 < psHwCtx->ui32NumPipes; uiPipeMinus1++)
                {
                    REPORT(REPORT_MODULE_HWCTRL, REPORT_NOTICE, "Dumping debug info for pipe %d:", uiPipeMinus1 + 1);
                    hwctrl_DumpState(&sState.sCoreState, &sState.sCoreState, psIntStatus, uiPipeMinus1);
                }

                /* Now lets dump The complete Register spaces for CORE, MTX, VEC, VDMC, VDEB, DMAC and VLR. Also Dump RTM register for HW Debug. */
                if (psHwCtx->sDevConfig.bCoreDump)
                {
                    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnDumpRegisters != IMG_NULL);
                    if (psHwCtx->sVxdFuncs.pfnDumpRegisters)
                    {
                        ui32Result = psHwCtx->sVxdFuncs.pfnDumpRegisters(psHwCtx->hVxd);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            goto error;
                        }
                    }
                }

                // Do the Device Reset
                // ...
                ui32Result = CORE_DevReset(psHwCtx->pvCompInitUserData, RESET_MMUPF);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    goto error;
                }
            }

            // xxx-lukasz: int status in psIntStatus->ui32Pending should be saved with some VDEC internally defined
            // xxx-lukasz: bitfield instead of HW-defined values.
            bMtxIrq = psIntStatus->ui32Pending & PVDEC_CORE_CR_PVDEC_HOST_INTERRUPT_STATUS_CR_HOST_PROC_IRQ_MASK;

            if (bMtxIrq && (IMG_NULL != psIntStatus->psMsgQueue))
            {
#ifdef IMG_KERNEL_MODULE
                // Checking the consistency of the message queue: either the queue is not empty and it's signalled as not empty
                // or the queue is empty and it's not signalled as not empty. The second case is acceptable as messages for this
                // interrupt may have been handled with messages signalled by the previous interrupt due to latencies.
                IMG_ASSERT((psIntStatus->psMsgQueue->bQueued && !LST_empty(&psIntStatus->psMsgQueue->sNewMsgList)) ||
                           (!psIntStatus->psMsgQueue->bQueued && LST_empty(&psIntStatus->psMsgQueue->sNewMsgList)));
                // Process messages only if there are any.
                if(psIntStatus->psMsgQueue->bQueued && !LST_empty(&psIntStatus->psMsgQueue->sNewMsgList))
                {
#else /* ndef IMG_KERNEL_MODULE */
                // XXX-ftc: We may need to improve this for no-bridging
                // In no-bridging if there are no messages just continue. We must have
                // processed them
                if(psIntStatus->psMsgQueue->bQueued)
                {
#endif /* ndef IMG_KERNEL_MODULE */
                    do
                    {
                        /* Deal with MTX messages. */
                        hwctrl_ProcessMTXMsgs(psHwCtx, psIntStatus->psMsgQueue, pvUserData);

                        if (!psIntStatus->psMsgQueue->bEmpty)
                        {
                            REPORT(REPORT_MODULE_HWCTRL, REPORT_WARNING, "Must service message buffer again...");

                            IMG_ASSERT(psHwCtx->sVxdFuncs.pfnHandleInt != IMG_NULL);
                            if (psHwCtx->sVxdFuncs.pfnHandleInt)
                            {
                                ui32Result = psHwCtx->sVxdFuncs.pfnHandleInt(psHwCtx->hVxd, psIntStatus);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    goto error;
                                }
                            }
                        }

                    }
                    while (!psIntStatus->psMsgQueue->bEmpty && psIntStatus->psMsgQueue->bQueued);
                }
            }
        }
    }

    return IMG_SUCCESS;

error:
#endif /* VDEC_MSVDX_HARDWARE */

    return ui32Result;

}


/*!
******************************************************************************

 @Function              HWCTRL_PowerOff

 @Description

 This function handles when the device is going off.

******************************************************************************/
IMG_RESULT
HWCTRL_PowerOff(
    IMG_HANDLE  hHwCtx
)
{
    IMG_UINT32  ui32Result = IMG_SUCCESS;

#ifdef VDEC_MSVDX_HARDWARE
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx->bPower && psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
    {
        /* Disable and clear all core interrupts before going further with reset.
         * This guards against getting core interrupts in the middle of the reset procedure,
         * which can end up in interrupt storm. */
        if (psHwCtx->sVxdFuncs.pfnDisableCoreInterrupts)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnDisableCoreInterrupts(psHwCtx->hVxd);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }
        }

        // XXX: Danish Reset and Resume doesn't work with FAKE mtx. Stop and Start MTX needs to be fixed
        // XXX: to get it to work.
        // Turn the power flag off. Any subsequent interrupts from the hardware will be ignored.
        // We need to disable interrupts. There is a Service Time Expiry task running, which when
        // check the status of the hardware in interrupt context, that might read from hardware while
        // we have disabled the clocks. This will lockup the hardware.
        SYSOSKM_DisableInt();
        psHwCtx->bPower = IMG_FALSE;
        SYSOSKM_EnableInt();

        // Destroy the service timer expiry off.
        if(psHwCtx->hTimerHandle)
        {
            SYSOSKM_DestroyTimer(psHwCtx->hTimerHandle);
            psHwCtx->hTimerHandle = IMG_NULL;
        }

#ifndef IMG_KERNEL_MODULE
        // Sync all pdump scripts before deinitializing. This makes sure
        // that teardown will not happen while the backend context is waiting
        // for pictures to come up
        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnPDUMPSync != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnPDUMPSync)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPSync(psHwCtx->hVxd);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }
        }
#endif
        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnDeInitialise != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnDeInitialise)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnDeInitialise(psHwCtx->hVxd);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }
        }

        DEBUG_REPORT(REPORT_MODULE_HWCTRL,
            "HWCTRL has performed Power off operations");
    }

error:

#endif // VDEC_MSVDX_HARDWARE

    return ui32Result;
}

/*!
******************************************************************************

 @Function              hwctrl_PowerOn

 @Description

 This function handles when the device is turning on.

******************************************************************************/
static IMG_RESULT
hwctrl_PowerOn(
    IMG_HANDLE                  hHwCtx
)
{
    IMG_UINT32  ui32Result = IMG_SUCCESS;

#ifdef VDEC_MSVDX_HARDWARE
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (!psHwCtx->bPower  && psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
    {
        VDECFW_sCoreInitData    sInitConfig;

        VDEC_BZERO(&sInitConfig);
#ifndef IMG_KERNEL_MODULE
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, "<+PowerOn>");
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
            goto error;
        }
#endif

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnInitialise != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnInitialise)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnInitialise(psHwCtx->hVxd,
                                                          &sInitConfig,
                                                          psHwCtx->bSecureStream);
            if (ui32Result != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                        "ERROR: Failed initialising core!");
                goto error;
            }

            /* Send init message to the firmware. */
#ifdef VDEC_USE_PVDEC_SEC
            if (psHwCtx->bSecureStream)
            {
                ui32Result = hwctrl_SendFWSECInitMessage(psHwCtx,
                                                         &sInitConfig,
                                                         &psHwCtx->sDevPtdBufInfo);
            }
            else
#endif /* VDEC_USE_PVDEC_SEC */
            {
                ui32Result = hwctrl_SendInitMessage(psHwCtx,
                                                    &sInitConfig,
                                                    &psHwCtx->sDevPtdBufInfo);
            }
            if (ui32Result != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                       "ERROR: Failed sending init message to firmware !");
            }

            hwctrl_SetupDWR(psHwCtx, (SYSOSKM_pfnTimer)hwctrl_MonitorHwState);

            SYSOSKM_DisableInt();
            psHwCtx->bPower = IMG_TRUE;
            SYSOSKM_EnableInt();

#ifndef IMG_KERNEL_MODULE
            {
                // Unlock pdump context that many times as the decode slots per core we have minus 1
                // (we lock after we submit a picture).
                // This will allow that many pictures in the pipeline
                IMG_UINT32  ui32I = 0;
                // PVDEC pipes may have different features (e.g. some standards may be supported by one pipe only)
                // so we need to use limited number of pipes
                IMG_UINT32  ui32NumPipes = psHwCtx->ui32NumAvailPipes;
                IMG_ASSERT(psHwCtx->sVxdFuncs.pfnPDUMPUnlock != IMG_NULL);
                if (psHwCtx->sVxdFuncs.pfnPDUMPUnlock &&
                    ui32NumPipes != 0 &&
                    psHwCtx->sDevConfig.ui32NumSlotsPerPipe != 0)
                {
                    for(ui32I = 0; ui32I < ui32NumPipes * psHwCtx->sDevConfig.ui32NumSlotsPerPipe - 1; ui32I++)
                    {
                        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPUnlock(psHwCtx->hVxd);
                        if(ui32Result != IMG_SUCCESS)
                        {
                            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                               "ERROR: Failed unlock pdump context");
                            goto error;
                        }
                    }
                }

                // Add a sync across all pdump scripts. This will prevent backend pdump
                // from trying to poll for pictures before driver submitted any.
                IMG_ASSERT(psHwCtx->sVxdFuncs.pfnPDUMPSync != IMG_NULL);
                if (psHwCtx->sVxdFuncs.pfnPDUMPSync)
                {
                    ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPSync(psHwCtx->hVxd);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                           "ERROR: Failed sync pdump contexts");
                        goto error;
                    }
                }
            }
#endif

            DEBUG_REPORT(REPORT_MODULE_HWCTRL,
                "HWCTRL has performed Power on operations");
        }

#ifndef IMG_KERNEL_MODULE
        ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, "<-PowerOn>");
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
            goto error;
        }
#endif
    }

error:

#endif // VDEC_MSVDX_HARDWARE

    return ui32Result;
}

/*!
******************************************************************************

 @Function              HWCTRL_DiscardHeadPicList

******************************************************************************/
IMG_RESULT
HWCTRL_DiscardHeadPicList(
    IMG_HANDLE          hHwCtx,
    VDECDD_eResetReason eReason
)
{
    DECODER_sDecPict    *   psDecPict;
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32  ui32Result;

    if(psHwCtx->bInitialised)
    {
        // Peek the head of the list
        psDecPict = LST_first(&psHwCtx->sPendPictList);
        if(psDecPict)
        {
            // Mark the picture
            if(eReason == RESET_MMUPF)
            {
                psDecPict->psFirstFieldFwMsg->sPictAttrs.bMMUFault = IMG_TRUE;
            }
            else
            {
                psDecPict->psFirstFieldFwMsg->sPictAttrs.bDWRFired = IMG_TRUE;
            }
            // Send async message for this discarded picture
            ui32Result = CORE_DevSwInterruptPicDiscarded(psHwCtx->pvCompInitUserData, psHwCtx->pvDecCore);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;

            }
            DEBUG_REPORT(REPORT_MODULE_HWCTRL,
                "HWCTRL has discarded the picture at the head");
        }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_PostCoreReplay

******************************************************************************/
IMG_RESULT
HWCTRL_PostCoreReplay(
    IMG_HANDLE          hHwCtx
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32  ui32Result = IMG_SUCCESS;

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        // XXX: add code to post a replay request to the core
        ui32Result = CORE_DevReplay(psHwCtx->pvCompInitUserData);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
        DEBUG_REPORT(REPORT_MODULE_HWCTRL,
                     "HWCTRL has posted a AsynMessage for Replay");
#endif
    }

#ifdef VDEC_MSVDX_HARDWARE
error:
#endif
    return ui32Result;
}

/*!
******************************************************************************

@Function              HWCTRL_CoreReplay

******************************************************************************/
IMG_RESULT
HWCTRL_CoreReplay(
    IMG_HANDLE          hHwCtx
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32  ui32Result = IMG_SUCCESS;

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        DECODER_sDecPict  * psDecPict = LST_first(&psHwCtx->sPendPictList);

        /* Replay the contents of the pending list. */
        while (psDecPict)
        {
            /* Reschedule the picture. */
            ui32Result = hwctrl_PictureReplay(hHwCtx, psDecPict);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }

            psDecPict = LST_next(psDecPict);
        }

        DEBUG_REPORT(REPORT_MODULE_HWCTRL,
                     "HWCTRL has performed Replay operation");
#endif
    }

#ifdef VDEC_MSVDX_HARDWARE
error:
#endif
    return ui32Result;
}

/*!
******************************************************************************

 @Function              HWCTRL_PictureSubmitBatch

******************************************************************************/
IMG_RESULT
HWCTRL_PictureSubmitBatch(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict,
    IMG_HANDLE          hResources,
    VXDIO_sPtdInfo    * psPtdInfo
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            IMG_RESULT ui32Result;
#if !defined(IMG_KERNEL_MODULE) && defined(VDEC_USE_PVDEC)
            psHwCtx->ui32NumAvailPipes = psDecPict->ui32NumAvailPipes;
#endif
            ui32Result = hwctrl_PowerOn(psHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            // Add picture to core decode list
            LST_add(&psHwCtx->sPendPictList, psDecPict);

            ui32Result = hwctrl_SendBatchMessage(psHwCtx,
                                                 psDecPict);

            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            psHwCtx->ui32Load[psDecPict->ui8Pipe - 1] += hwctrl_CalculateLoad(psDecPict->psPictHdrInfo);
        }
        else
#endif
        {
            // Add picture to core decode list
            LST_add(&psHwCtx->sPendPictList, psDecPict);
        }

        psHwCtx->ui32NumFreeSlots[psDecPict->ui8Pipe - 1]--;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_KickSwInterrupt

******************************************************************************/
IMG_RESULT
HWCTRL_KickSwInterrupt(
    IMG_HANDLE  hHwCtx,
    IMG_HANDLE  hDecServiceInt
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *)     hHwCtx;

    {
        IMG_RESULT ui32Result;

        /* Generate a Dev SW Interrupt */
        ui32Result = CORE_DevSwInterrupt(psHwCtx->pvCompInitUserData, hDecServiceInt);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            //return ui32Result;
        }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_GetPicPendPictList

******************************************************************************/
IMG_RESULT
HWCTRL_GetPicPendPictList(
    IMG_HANDLE           hHwCtx,
    IMG_UINT32           ui32TransactionId,
    DECODER_sDecPict  ** ppsDecPict
)
{
    HWCTRL_sHwCtx     * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    DECODER_sDecPict  * psDecPic;

    psDecPic = LST_first(&psHwCtx->sPendPictList);
    while(psDecPic)
    {
        if(psDecPic->ui32TransactionId == ui32TransactionId)
        {
            *ppsDecPict = psDecPic;
            break;
        }
        psDecPic = LST_next(psDecPic);
    }

    if(psDecPic == IMG_NULL)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
            "Failed to find pending picture from transaction ID");
        return IMG_ERROR_INVALID_ID;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              HWCTRL_PeekHeadPicList

******************************************************************************/
IMG_RESULT
HWCTRL_PeekHeadPicList(
    IMG_HANDLE           hHwCtx,
    DECODER_sDecPict  ** ppsDecPict
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx)
    {
        *ppsDecPict = LST_first(&psHwCtx->sPendPictList);
    }

    if (*ppsDecPict)
    {
        return IMG_SUCCESS;
    }
    else
    {
        return IMG_ERROR_GENERIC_FAILURE;
    }
}
/*!
******************************************************************************

 @Function              HWCTRL_RemoveHeadPicList

******************************************************************************/
IMG_RESULT
HWCTRL_RemoveHeadPicList(
    IMG_HANDLE           hHwCtx,
    DECODER_sDecPict  ** ppsDecPict
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if(psHwCtx)
    {
        *ppsDecPict = LST_removeHead(&psHwCtx->sPendPictList);

        if (*ppsDecPict)
        {
            // Indicate that a decode slot is now free.
            psHwCtx->ui32NumFreeSlots[(*ppsDecPict)->ui8Pipe - 1]++;

#ifdef VDEC_MSVDX_HARDWARE
            if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER && (*ppsDecPict)->psPictHdrInfo)
            {
                psHwCtx->ui32Load[(*ppsDecPict)->ui8Pipe - 1] -= hwctrl_CalculateLoad((*ppsDecPict)->psPictHdrInfo);
            }
#endif
        }
    }
    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_GetDecodedPicture

******************************************************************************/
IMG_RESULT
HWCTRL_GetDecodedPicture(
    IMG_HANDLE           hHwCtx,
    DECODER_sDecPict  ** ppsDecPict
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx)
    {
        DECODER_sDecPict *  psCurDecPict;

        // Ensure that this picture is in the list.
        psCurDecPict = LST_first(&psHwCtx->sPendPictList);
        while (psCurDecPict)
        {
            if (psCurDecPict->bDecoded)
            {
                *ppsDecPict = psCurDecPict;
                return IMG_SUCCESS;
            }

            psCurDecPict = LST_next(psCurDecPict);
        }
    }

    return IMG_ERROR_VALUE_OUT_OF_RANGE;
}

/*!
******************************************************************************

 @Function              HWCTRL_RemoveFromPicList

******************************************************************************/
IMG_RESULT
HWCTRL_RemoveFromPicList(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx)
    {
        DECODER_sDecPict *  psCurDecPict;

        // Ensure that this picture is in the list.
        psCurDecPict = LST_first(&psHwCtx->sPendPictList);
        while (psCurDecPict)
        {
            if (psCurDecPict == psDecPict)
            {
                LST_remove(&psHwCtx->sPendPictList, psDecPict);

                // Indicate that a decode slot is now free.
                psHwCtx->ui32NumFreeSlots[psDecPict->ui8Pipe - 1]++;

                break;
            }

            psCurDecPict = LST_next(psCurDecPict);
        }
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              HWCTRL_Configure

******************************************************************************/
IMG_RESULT
HWCTRL_Configure(
    IMG_HANDLE                  hHwCtx,
    const VXD_sFirmware       * psFirmware,
    const VXDIO_sDdBufInfo      aui32RendecBufinfo[],
    const VXDIO_sDdBufInfo    * psDevPtdBufInfo,
    const VXDIO_sPtdInfo      * psPtdInfo
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32  ui32Result = IMG_SUCCESS;

#ifdef VDEC_MSVDX_HARDWARE
    REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,"+~+hwctrl init FW check");
    if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
    {
        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnPrepareFirmware != IMG_NULL);
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,"+~+pfnPrepareFirmware is not NULL");
        if (psHwCtx->sVxdFuncs.pfnPrepareFirmware)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnPrepareFirmware(psHwCtx->hVxd, psFirmware, psPtdInfo,
                                                               psHwCtx->bSecureStream);

            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }
        }

        psHwCtx->aui32RendecBufInfo[0] = aui32RendecBufinfo[0];
        psHwCtx->aui32RendecBufInfo[1] = aui32RendecBufinfo[1];
        psHwCtx->sDevPtdBufInfo = *psDevPtdBufInfo;
    }

error:
#endif
    return ui32Result;
}


/*!
******************************************************************************

 @Function              HWCTRL_Initialise

******************************************************************************/
IMG_RESULT
HWCTRL_Initialise(
    IMG_VOID                   * pvDecCore,
    IMG_VOID                   * pvCompInitUserData,
    const VDECDD_sDdDevConfig  * psDdDevConfig,
    VXD_sCoreProps             * psCoreProps,
    HWCTRL_sRendec             * psRendec,
    IMG_HANDLE                 * phHwCtx
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) *phHwCtx;
    IMG_RESULT ui32Result;

    if (psHwCtx == IMG_NULL)
    {
        VDEC_MALLOC(psHwCtx);
        IMG_ASSERT(psHwCtx);
        if(psHwCtx == IMG_NULL)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to allocate memory for HWCTRL context");
            return IMG_ERROR_OUT_OF_MEMORY;
        }
        VDEC_BZERO(psHwCtx);
        *phHwCtx = psHwCtx;
    }

    if (!psHwCtx->bInitialised)
    {
        IMG_UINT32  ui32Msg;
        IMG_UINT32  ui32I;

        psHwCtx->sIntStatus.psMsgQueue = &psHwCtx->sMsgQueue;

#if defined VDEC_USE_PVDEC_SEC && defined SYSBRG_BRIDGING
        ui32Result = PVDECSEC_Create(psDdDevConfig,
                                  psCoreProps,
                                  psRendec,
                                  &psHwCtx->hVxd,
                                  &psHwCtx->sVxdFuncs);
        // if TEE fails try to use the old way
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_WARNING,
                   "TEE not available, using standard IO interface !");
            ui32Result = PVDEC_Create(psDdDevConfig,
                                      psCoreProps,
                                      psRendec,
                                      &psHwCtx->hVxd,
                                      &psHwCtx->sVxdFuncs);
        }
#elif VDEC_USE_PVDEC
        ui32Result = PVDEC_Create(psDdDevConfig,
                                  psCoreProps,
                                  psRendec,
                                  &psHwCtx->hVxd,
                                  &psHwCtx->sVxdFuncs);
#else
#error "Currently only PVDEC is supported!!!"
#endif
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to create decoder (%d)", ui32Result);
            goto error;
        }

        IMG_ASSERT(psHwCtx->hVxd);

        /* Initialise message info...*/
        LST_init(&psHwCtx->sMsgQueue.sFreeMsgList);
        LST_init(&psHwCtx->sMsgQueue.sNewMsgList);
        for (ui32Msg = 0; ui32Msg < MSVDX_MAX_LISR_HISR_MSG; ui32Msg++)
        {
            LST_add(&psHwCtx->sMsgQueue.sFreeMsgList, &psHwCtx->asHISRMsg[ui32Msg]);
        }

        LST_init(&psHwCtx->sPendPictList);

        psHwCtx->sDevConfig         = *psDdDevConfig;
        for(ui32I = 0; ui32I < psCoreProps->ui32NumPixelPipes; ui32I++)
        {
            psHwCtx->ui32NumFreeSlots[ui32I]   = psDdDevConfig->ui32NumSlotsPerPipe;
        }

        psHwCtx->ui32NumPipes       = psCoreProps->ui32NumPixelPipes;
        psHwCtx->pvCompInitUserData = pvCompInitUserData;
        psHwCtx->pvDecCore          = pvDecCore;
        psHwCtx->bInitialised       = IMG_TRUE;
        psHwCtx->hTimerHandle       = IMG_NULL;
#if !defined(IMG_KERNEL_MODULE) && defined(VDEC_USE_PVDEC)
        psHwCtx->ui32NumAvailPipes = psCoreProps->ui32NumPixelPipes;
#endif
    }

    return IMG_SUCCESS;

error:
    if (psHwCtx->bInitialised)
    {
        if (IMG_SUCCESS != HWCTRL_Deinitialise(*phHwCtx))
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "HWCTRL_Deinitialise() failed to tidy-up after error");
        }
    }
    else
    {
        IMG_FREE(*phHwCtx);
        *phHwCtx = IMG_NULL;
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              HWCTRL_Deinitialise

******************************************************************************/
IMG_RESULT
HWCTRL_Deinitialise(
    IMG_HANDLE  hHwCtx
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32      ui32Result;

    if (psHwCtx->bInitialised)
    {
        if(psHwCtx->hTimerHandle)
        {
            SYSOSKM_DestroyTimer( psHwCtx->hTimerHandle );
            psHwCtx->hTimerHandle = IMG_NULL;
        }

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnDestroy != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnDestroy)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnDestroy(psHwCtx->hVxd);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

        IMG_FREE(psHwCtx);
        psHwCtx = IMG_NULL;
    }

    return IMG_SUCCESS;
}


#ifndef IMG_KERNEL_MODULE
/*!
******************************************************************************

 @Function              HWCTRL_PdumpComment

******************************************************************************/
IMG_RESULT
HWCTRL_PdumpComment(
    IMG_HANDLE      hHwCtx,
    IMG_CHAR      * pszComment
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32      ui32Result = IMG_SUCCESS;

    if (hHwCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
               "Invalid parameters provided (%d)", __LINE__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    ui32Result = psHwCtx->sVxdFuncs.pfnPDUMPComment(psHwCtx->hVxd, pszComment);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "ERROR: Failed to add comment to PDUMP");
    }

    return ui32Result;
}
#endif


/*!
******************************************************************************

 @Function              HWCTRL_PrintMSVDXProps

******************************************************************************/
IMG_VOID
HWCTRL_PrintMSVDXProps(
    VXD_sCoreProps    * psCoreProps
)
{
    IMG_UINT8    ui8PipeMinus1;
#ifdef __RELEASE_RELEASE__
    REPORT(REPORT_MODULE_HWCTRL,
           REPORT_INFO,
           "[%s, %s] "
           "MMU:%s, "
           "Samples{ChromaUp:%s}, "
           "MemLat:%u, CmdBuf:%ubytes",
           "Core Configuration",
           psCoreProps->acVersion,
           acMmuTypeNames[psCoreProps->eMmuType],
           (psCoreProps->bChromaUpsample) ? "y" : "n",
           psCoreProps->ui32Latency,
           psCoreProps->ui32CmdBufferSize);

    for(ui8PipeMinus1 = 0; ui8PipeMinus1 < psCoreProps->ui32NumPixelPipes; ui8PipeMinus1++)
    {
        REPORT(REPORT_MODULE_HWCTRL,
               REPORT_INFO,
               "Std_pipe_%d{avs:%s, h264:%s, jpeg:%s, mpeg2:%s, mpeg4:%s, real:%s, vc1:%s, vp6:%s, vp8:%s, hevc:%s}, "
               "SR_pipe_%d{streams:%u, DMABurst:%u}, "
               "Rot_pipe_%d:%s, "
               "Scale_pipe_%d{%s, %u-bit}",
               ui8PipeMinus1,
               (psCoreProps->abAvs[ui8PipeMinus1])   ? "y" : "n",
               (psCoreProps->abH264[ui8PipeMinus1])  ? "y" : "n",
               (psCoreProps->abJpeg[ui8PipeMinus1])  ? "y" : "n",
               (psCoreProps->abMpeg2[ui8PipeMinus1]) ? "y" : "n",
               (psCoreProps->abMpeg4[ui8PipeMinus1]) ? "y" : "n",
               (psCoreProps->abReal[ui8PipeMinus1])  ? "y" : "n",
               (psCoreProps->abVc1[ui8PipeMinus1])   ? "y" : "n",
               (psCoreProps->abVp6[ui8PipeMinus1])   ? "y" : "n",
               (psCoreProps->abVp8[ui8PipeMinus1])   ? "y" : "n",
               (psCoreProps->abHEVC[ui8PipeMinus1])  ? "y" : "n",
               ui8PipeMinus1,
               psCoreProps->aui32NumStreams[ui8PipeMinus1],
               psCoreProps->ui32SrDmaBurstSize,
               ui8PipeMinus1,
               (psCoreProps->abRotationSupport[ui8PipeMinus1]) ? "y" : "n",
               ui8PipeMinus1,
               (psCoreProps->abScalingSupport[ui8PipeMinus1]) ? "y" : "n",
                psCoreProps->ui32ScalingPrecision);
    }
#else
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "======================================================");
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Core Configuration [%s]", psCoreProps->acVersion);


    for(ui8PipeMinus1 = 0; ui8PipeMinus1 < psCoreProps->ui32NumPixelPipes; ui8PipeMinus1++)
    {
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "----------------------");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Pipe (%d) ", ui8PipeMinus1);
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "----------------------");
        // Video standards.
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Video Standards >>>>>>>>>>>>>");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    AVS:                               %s", (psCoreProps->abAvs[ui8PipeMinus1])   ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    H.264:                             %s", (psCoreProps->abH264[ui8PipeMinus1])  ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    HEVC:                              %s", (psCoreProps->abHEVC[ui8PipeMinus1])  ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    JPEG:                              %s", (psCoreProps->abJpeg[ui8PipeMinus1])  ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    MPEG2:                             %s", (psCoreProps->abMpeg2[ui8PipeMinus1]) ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    MPEG4:                             %s", (psCoreProps->abMpeg4[ui8PipeMinus1]) ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    REAL:                              %s", (psCoreProps->abReal[ui8PipeMinus1])  ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    VC-1:                              %s", (psCoreProps->abVc1[ui8PipeMinus1])   ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    VP6:                               %s", (psCoreProps->abVp6[ui8PipeMinus1])   ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    VP8:                               %s", (psCoreProps->abVp8[ui8PipeMinus1])   ? "yes" : "no");

        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Transformations >>>>>>>>>>>>>");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Rotation:                          %s", (psCoreProps->abRotationSupport[ui8PipeMinus1]) ? "yes" : "no");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Scaling:                           %s", (psCoreProps->abScalingSupport[ui8PipeMinus1]) ? "yes" : "no");

        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Shift register >>>>>>>>>>>>>>");
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Quantity:                          %d", psCoreProps->aui32NumStreams[ui8PipeMinus1]);
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    DMA burst size:                    %d", psCoreProps->ui32SrDmaBurstSize);
    }
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "----------------------");

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Stride Support >>>>>>>>>>");

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "MMU >>>>>>>>>>>>>>>>>>>>>>>>>");
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Type:                              %s", acMmuTypeNames[psCoreProps->eMmuType]);
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Support stride per context:        %s", (psCoreProps->bMmuSupportStridePerContext) ? "yes" : "no");
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Support secure transactions:       %s", (psCoreProps->bMmuSupportSecure) ? "yes" : "no");

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Samples >>>>>>>>>>>>>>>>>>>>>");
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Chroma upsampling:                 %s",    (psCoreProps->bChromaUpsample) ? "yes" : "no");

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Other >>>>>>>>>>>>>>>>>>>>>>>");
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Memory Latency:                    %d", psCoreProps->ui32Latency);
    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "    Command buffer size:               %d", psCoreProps->ui32CmdBufferSize);

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "======================================================");
#endif
}

/*!
******************************************************************************

 @Function              HWCTRL_GetRegsOffsets

******************************************************************************/
IMG_RESULT
HWCTRL_GetRegsOffsets(
    IMG_HANDLE              hHwCtx,
    DECODER_sRegsOffsets  * psRegsOffsets
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    IMG_UINT32      ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetRegsOffsets != IMG_NULL);
    if (psHwCtx->sVxdFuncs.pfnGetRegsOffsets)
    {
        ui32Result = psHwCtx->sVxdFuncs.pfnGetRegsOffsets(psHwCtx->hVxd, psRegsOffsets);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }

    return ui32Result;
}

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              HWCTRL_SetSecured

******************************************************************************/
IMG_VOID
HWCTRL_SetSecured(
    IMG_HANDLE              hHwCtx,
    IMG_BOOL                bSecureStream
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;
    psHwCtx->bSecureStream = bSecureStream;
}

/*!
******************************************************************************

 @Function              hwctrl_SendFWSECInitMessage

 @Description

 This function sends the FWBSP and FWDEC init message to the firmware.

******************************************************************************/
static IMG_RESULT
hwctrl_SendFWSECInitMessage(
    HWCTRL_sHwCtx               * psHwCtx,
    const VDECFW_sCoreInitData  * psCoreInitData,
    const VXDIO_sDdBufInfo      * psDevPtdBufInfo
)
{
    IMG_RESULT ui32Result;

    /* Create a control picture message here from the config. */
    IMG_UINT8 * pui8Msg = IMG_NULL;

    IMG_ASSERT(psHwCtx);

    /* Send FWBSP init message */
    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        HWCTRL_sMsgData sMsgData = {{{0}}};

        sMsgData.sInitBSPMsgData.psFwMsgData = psHwCtx->psFwMsgData;

        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_INIT_BSP,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to get FWBSP init message");
            return ui32Result;
        }
        IMG_ASSERT(pui8Msg != IMG_NULL);

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnSendFirmwareMessage != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd,
                                                                   pui8Msg);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnReturnMsg)
        {
            psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
        }
    }

    /* Send FWDEC init message */
    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        HWCTRL_sMsgData sMsgData = {{{0}}};
        sMsgData.sInitSecDecMsgData.psRendecBufInfo = psHwCtx->aui32RendecBufInfo;

        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_INIT_SEC_DEC,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to get FWDEC init message");
            return ui32Result;
        }
        IMG_ASSERT(pui8Msg != IMG_NULL);

        if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd,
                                                                   pui8Msg);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnReturnMsg)
        {
            psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
        }
    }

    /* Send FWDEC configure message */
    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        HWCTRL_sMsgData sMsgData = {{{0}}};

        sMsgData.sConfigureMsgData.psDevConfig = &psHwCtx->sDevConfig;
        sMsgData.sConfigureMsgData.psCoreInitData = psCoreInitData;

        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_CONFIG,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_ERROR_INTERRUPTED)
        {
            if (ui32Result != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                       "Failed to get FWDEC configure message");
                return ui32Result;
            }
            IMG_ASSERT(pui8Msg != IMG_NULL);

            if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
            {
                ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd,
                                                                       pui8Msg);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }
            }

            IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
            if (psHwCtx->sVxdFuncs.pfnReturnMsg)
            {
                psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
            }
        }
        else
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_INFO,
                   "Skipping configuration message");
        }
    }

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[MID=0x%02X] [CONTROL] [0x%02X]",
        psHwCtx->sHostMsgStatus.aui8ControlFenceID[VDECFW_MSGID_FIRMWARE_INIT
        -VDECFW_MSGID_BASE_PADDING], VDECFW_MSGID_FIRMWARE_INIT);

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_BistreamSubmit

******************************************************************************/
IMG_RESULT
HWCTRL_BistreamSubmit(
    IMG_HANDLE                  hHwCtx,
    IMG_HANDLE                  hFWBSP_BRIDGECtx,
    FWBSP_BRIDGE_sFwMsgData   * psFwMsgData,
    FWBSP_BRIDGE_sHostMsgData * psHostMsgData
)
{
    HWCTRL_sHwCtx *psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    IMG_ASSERT(hHwCtx);

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            IMG_RESULT ui32Result;

            /* TO DO: save data needed for fwbsp init msg, this needs to be
               reworked for multistream sceanrio */
            psHwCtx->psFwMsgData = psFwMsgData;

            ui32Result = hwctrl_PowerOn(psHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            ui32Result = hwctrl_SendParseBitstreamMessage(psHwCtx, hFWBSP_BRIDGECtx, psFwMsgData);

            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            // xxx-todo: max pending encrypted bit streams ?
            psHwCtx->ui32PendingBitStreams++;
        }
        else
#endif
        {
            IMG_ASSERT(0);
        }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_BistreamProcessed

******************************************************************************/
IMG_RESULT
HWCTRL_BistreamProcessed(
    IMG_HANDLE                  hHwCtx
)
{
    HWCTRL_sHwCtx *psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    IMG_ASSERT(hHwCtx);

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            IMG_ASSERT(psHwCtx->ui32PendingBitStreams);
            if (psHwCtx->ui32PendingBitStreams > 0)
            {
                psHwCtx->ui32PendingBitStreams--;
            }
            else
            {
                return IMG_ERROR_FATAL;
            }
        }
        else
#endif
        {
            IMG_ASSERT(0);
        }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              HWCTRL_HasFwBspLockUp

******************************************************************************/
IMG_BOOL
HWCTRL_HasFwBspLockUp(
    IMG_HANDLE                  hHwCtx,
    IMG_UINT32                * pui32LockUpPID
)
{
    HWCTRL_sHwCtx *psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    IMG_ASSERT(hHwCtx);

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            IMG_BOOL bLockup = psHwCtx->bFwBspLockUp;

            psHwCtx->bFwBspLockUp = IMG_FALSE;   // clear it
            /* Read lockup PID */
            *pui32LockUpPID = psHwCtx->ui32FwBspLockUpPID;

            return psHwCtx->ui32PendingBitStreams && bLockup;
        }
        else
#endif
        {
            IMG_ASSERT(0);
        }
    }

    return IMG_FALSE;
}


/*!
******************************************************************************

 @Function              HWCTRL_PictureSubmitSecure

******************************************************************************/
IMG_RESULT
HWCTRL_PictureSubmitSecure(
    IMG_HANDLE          hHwCtx,
    DECODER_sDecPict  * psDecPict,
    IMG_HANDLE          hResources,
    VXDIO_sPtdInfo    * psPtdInfo
)
{
    HWCTRL_sHwCtx * psHwCtx = (HWCTRL_sHwCtx *) hHwCtx;

    if (psHwCtx->bInitialised)
    {
#ifdef VDEC_MSVDX_HARDWARE
        if (psHwCtx->sDevConfig.eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            IMG_RESULT ui32Result;

            ui32Result = hwctrl_PowerOn(psHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            // Add picture to core decode list
            LST_add(&psHwCtx->sPendPictList, psDecPict);

            ui32Result = hwctrl_SendFWDECDecodeMessage(psHwCtx, psDecPict);

            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            psHwCtx->ui32Load[psDecPict->ui8Pipe - 1] += hwctrl_CalculateLoad(psDecPict->psPictHdrInfo);
        }
        else
#endif // VDEC_MSVDX_HARDWARE
        {
            // Add picture to core decode list
            LST_add(&psHwCtx->sPendPictList, psDecPict);
        }

        psHwCtx->ui32NumFreeSlots[psDecPict->ui8Pipe - 1]--;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              hwctrl_SendParseBitstreamMessage

 @Description

 This function sends a parse bitstream messsage to FWBSP_BRIDGE module.

******************************************************************************/
static IMG_RESULT
hwctrl_SendParseBitstreamMessage(
    HWCTRL_sHwCtx     * psHwCtx,
    IMG_HANDLE         hFWBSP_BRIDGECtx,
    FWBSP_BRIDGE_sFwMsgData *psFwMsgData
)
{
    IMG_RESULT ui32Result;

    IMG_UINT8 *pui8Msg = IMG_NULL;

    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage != IMG_NULL);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        HWCTRL_sMsgData sMsgData = {{{0}}};

        sMsgData.sParseMsgData.hFWBSP_BRIDGECtx = hFWBSP_BRIDGECtx;
        sMsgData.sParseMsgData.psFwMsgData = psFwMsgData;

        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_PARSE,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to get decode message");
            return ui32Result;
        }
        IMG_ASSERT(pui8Msg != IMG_NULL);

        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[MID=0x%02X] [PARSE BITSTREAM] [0x%02X]",
            psHwCtx->sHostMsgStatus.aui8DecodeFenceID[VDECFW_MSGID_BITSTREAM_BUFFER-VDECFW_MSGID_PSR_PADDING], VDECFW_MSGID_BITSTREAM_BUFFER);

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnSendFirmwareMessage != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd, pui8Msg);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnReturnMsg)
        {
            psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
        }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              hwctrl_SendFWDECDecodeMessage

 @Description

 irekb: TODO add desc.

******************************************************************************/
static IMG_RESULT
hwctrl_SendFWDECDecodeMessage(
    HWCTRL_sHwCtx           * psHwCtx,
    DECODER_sDecPict        * psDecPict
)
{
    IMG_RESULT ui32Result;
    IMG_UINT8 *pui8Msg = IMG_NULL;

    IMG_ASSERT(psHwCtx->sVxdFuncs.pfnGetMessage != IMG_NULL);
    if (psHwCtx->sVxdFuncs.pfnGetMessage)
    {
        HWCTRL_sMsgData sMsgData = {{{0}}};
        sMsgData.sDecMsgData.psDecPict = psDecPict;

        ui32Result = psHwCtx->sVxdFuncs.pfnGetMessage(psHwCtx->hVxd,
                                                      HWCTRL_MSGID_DEC,
                                                      &sMsgData,
                                                      &pui8Msg);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                   "Failed to get decode message");
            return ui32Result;
        }
        IMG_ASSERT(pui8Msg != IMG_NULL);
        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[MID=0x%02X] [DECODE] [0x%02X]", 0, VDECFW_MSGID_BATCH);

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnSendFirmwareMessage != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnSendFirmwareMessage)
        {
            ui32Result = psHwCtx->sVxdFuncs.pfnSendFirmwareMessage(psHwCtx->hVxd,
                                                                   pui8Msg);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }

        IMG_ASSERT(psHwCtx->sVxdFuncs.pfnReturnMsg != IMG_NULL);
        if (psHwCtx->sVxdFuncs.pfnReturnMsg)
        {
            psHwCtx->sVxdFuncs.pfnReturnMsg(psHwCtx->hVxd, pui8Msg);
        }
    }

    return IMG_SUCCESS;
}
#endif /* VDEC_USE_PVDEC_SEC */
