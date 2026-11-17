/*!
 *****************************************************************************
 *
 * @File       pvdec_msg.c
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

#include "report_api.h"
#include "rman_api.h"
#include "sysos_api_km.h"
#include "dbgopt_api_km.h"
#include "vxd_ext.h"
#include "vxd_int.h"
#include "hwctrl_api.h"
#include "pvdec_msg.h"
#include "pvdec_int.h"
#include "pvdec_ctx.h"
#include "interface/deva_fw_msg.h"
#ifdef VDEC_USE_PVDEC_SEC
#include "interface/bsp_fw_msg.h"
#endif /* VDEC_USE_PVDEC_SEC */
#include "hwdefs/pvdec_core_regs.h"

#ifdef USE_REAL_FW
    #include "vdecfw_bin.h"
#endif

#ifndef SYSBRG_BRIDGING
    #include "talmmu_api.h"
#else
    #include "sysmem_utils.h"
#endif

/* Included from PVDEC firmware. */
#include "source/core/vdec_fw_interface.h"

/* Panic reason identifier. */
typedef enum
{
    PANIC_REASON_OTHER = 0,
    PANIC_REASON_WDT,
    PANIC_REASON_READ_TIMEOUT,
    PANIC_REASON_CMD_TIMEOUT,

    PANIC_REASON_MAX

} pvdec_ePanicReason;

/* Panic reason strings.
 * NOTE: Should match the pvdec_ePanicReason ids. */
static IMG_CHAR *  apszPanicReason[PANIC_REASON_MAX] =
{
    "Other, ",
    "Watch Dog Timeout, ",
    "Command Timeout, ",
    "Read Timeout, ",
};

/* Maximum length of the panic reason string. */
#define PANIC_REASON_LEN  (PANIC_REASON_MAX * 24)

#ifdef VDEC_USE_PVDEC_SEC
#if 0 /* Enable when needed */
/*!
******************************************************************************
 @Function              pvdec_DumpMsg
******************************************************************************/
static IMG_VOID pvdec_DumpMsg(
    IMG_UINT8  * pui8Msg,
    IMG_UINT32 ui32Size,
    IMG_CHAR * sMsgName
)
{
    IMG_UINT32 ui32i;

    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "Dumping msg: %s, size %d B",
                 sMsgName, ui32Size);

    for (ui32i = 0; ui32i < ui32Size/4; ui32i++)
    {
        IMG_UINT32 ui32j = ui32i*4;
        IMG_UINT8 b0 = ((ui32j + 0) < ui32Size) ? pui8Msg[ui32j + 0] : 0;
        IMG_UINT8 b1 = ((ui32j + 1) < ui32Size) ? pui8Msg[ui32j + 1] : 0;
        IMG_UINT8 b2 = ((ui32j + 2) < ui32Size) ? pui8Msg[ui32j + 2] : 0;
        IMG_UINT8 b3 = ((ui32j + 3) < ui32Size) ? pui8Msg[ui32j + 3] : 0;

        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "0x%02x%02x%02x%02x",
                     b3, b2, b1, b0);
    }
}
#else /* pvdec_DumpMsg func */
#define pvdec_DumpMsg(...)
#endif /* pvdec_DumpMsg empty macro */
#endif /* VDEC_USE_PVDEC_SEC */
/*!
******************************************************************************

 @Function              PVDEC_GetInitMessage

******************************************************************************/
static IMG_RESULT pvdec_GetInitNonSecMessage(
    const HWCTRL_sInitNonSecDecMsgData    * psInitMsgData,
    IMG_UINT8                            ** ppui8Msg
)
{
    /* Create a control picture message here from the config. */
    IMG_BOOL bHWCRCGen = IMG_FALSE;
    VXDIO_sDdBufInfo * psRendecBufInfo = IMG_NULL;
    IMG_UINT8 * pui8Msg = IMG_NULL;

    IMG_ASSERT(ppui8Msg);
    IMG_ASSERT(psInitMsgData);

    psRendecBufInfo = psInitMsgData->psRendecBufInfo;
    IMG_ASSERT(psRendecBufInfo);
    if(psRendecBufInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Rendec buffer info missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    pui8Msg = IMG_MALLOC(FW_DEVA_INIT_SIZE);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, FW_DEVA_INIT_SIZE);

    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE, FW_DEVA_INIT_SIZE/sizeof(IMG_UINT32));
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE, FW_DEVA_INIT);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_TRANS_ID, 0);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_BATCH_START, 0);

    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_INIT_HEVC_MIN_H_FOR_DUAL_PIPE, 0); /* no minimum height restriction */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_INIT_HEVC_MAX_H_FOR_PIPE_WAIT, ~0); /* never wait */

    /* DEVA PVDEC FW uses only first (in message) rendec buffer currently and splits it itself. */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_INIT_RENDEC_ADDR0, psRendecBufInfo[0].ui32DevVirt);
    /* DEVA PVDEC FW expects rendec buffer size to be given in pages */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_INIT_RENDEC_SIZE0, psRendecBufInfo[0].ui32BufSize/(DEV_MMU_PAGE_SIZE));

    /* Check if CRC generation is required. */
    {
        DBGOPT_sValue  sVal;
        if (DBGOPTKM_GetSize("HardwareCRCGen", DBGOPT_TYPE_BOOL) &&
            DBGOPTKM_Get("HardwareCRCGen", IMG_NULL, &sVal))
        {
            bHWCRCGen = sVal.bVal;
        }
    }

    /* Request signatures (CRCs) for picture verification if required. */
    if(bHWCRCGen)
    {
        MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_INIT_HW_SIG_SELECT, (PVDEC_SIGNATURE_GROUP_20 | PVDEC_SIGNATURE_GROUP_24));
    }

    DEBUG_REPORT(REPORT_MODULE_VXDIO, "[MID=0x%02X] [CONTROL] [0x%02X]", 0, VDECFW_MSGID_FIRMWARE_INIT);

    *ppui8Msg = pui8Msg;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDEC_GetBatchMessage

******************************************************************************/
static IMG_RESULT pvdec_GetBatchMessage(
    PVDEC_sContext                    * psPvdecContext,
    const HWCTRL_sBatchMsgData        * psBatchMsgData,
    IMG_UINT8                        ** ppui8Msg
)
{
    IMG_UINT8         ui8VdecFlags = 0;
    IMG_UINT16        ui16Flags = 0;

    VXDIO_sDdBufInfo  * psStrPtdBufInfo = IMG_NULL;
    VXDIO_sDdBufInfo  * psBatchMsgBufInfo = IMG_NULL;
    IMG_UINT8         * pui8Msg = IMG_NULL;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(ppui8Msg);
    IMG_ASSERT(psBatchMsgData);

    psStrPtdBufInfo = psBatchMsgData->psStrPtdBufInfo;
    IMG_ASSERT(psStrPtdBufInfo);
    if(psBatchMsgData->psStrPtdBufInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Ptd buffor info missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psBatchMsgBufInfo = psBatchMsgData->psBatchMsgBufInfo;
    IMG_ASSERT(psBatchMsgBufInfo);
    if(psBatchMsgData->psBatchMsgBufInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Batch message info missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    pui8Msg = IMG_MALLOC(FW_DEVA_DECODE_SIZE);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, FW_DEVA_DECODE_SIZE);
    if (psBatchMsgData->bSizeDelimitedMode)
    {
        ui8VdecFlags |= FW_VDEC_NAL_SIZE_DELIM;
    }
    ui16Flags |= FW_DEVA_RENDER_HOST_INT;

    /* Message type and stream ID */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE, FW_DEVA_PARSE);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_TRANS_ID, 0);

    /* Control allocation size and address */
    IMG_ASSERT(CTRL_ALLOC_MAX_SEGMENT_SIZE > psBatchMsgData->ui32CtrlAllocBytes);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_CTRL_ALLOC_ADDR, (IMG_UINT32)psBatchMsgBufInfo->ui32DevVirt);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_BUFFER_SIZE, psBatchMsgData->ui32CtrlAllocBytes/sizeof(IMG_UINT32));

    /* Operating mode and decode flags */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_OPERATING_MODE, psBatchMsgData->ui32OperatingMode);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_FLAGS, ui16Flags);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_VDEC_FLAGS, ui8VdecFlags);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_GENC_ID, psBatchMsgData->ui16GencId);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_PIPE, psBatchMsgData->ui8Pipe);

    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_STREAMID, GET_STREAM_ID(psBatchMsgData->ui32TransactionId));

    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_EXT_STATE_BUFFER, (IMG_UINT32)psBatchMsgData->psPvdecFwCtx->ui32DevVirt);
    DEBUG_REPORT(REPORT_MODULE_VXDIO, "[TID=0x%08X] [PARSE] [0x%02X]", psBatchMsgData->ui32TransactionId, FW_DEVA_PARSE);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_MSG_ID, ++psPvdecContext->ui16CurrentMsgId) ;
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_TRANS_ID, psBatchMsgData->ui32TransactionId) ;

    /* Tile CFG */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_TILE_CFG, psBatchMsgData->ui32TileCfg);

    /* PTD. Host pointer has to be updated because the value was temporary stored in the device memory*/
    UPDATE_HOST(psStrPtdBufInfo);
    /* Fill stream ptd buffer address - ptd send as reference */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_DECODE_MMU_STRPTD_BUF_ADDR, psStrPtdBufInfo->ui32DevVirt +
                     (GET_STREAM_ID(psBatchMsgData->ui32TransactionId) * sizeof(IMG_UINT32)));

    /* Size of message */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE, FW_DEVA_DECODE_SIZE/sizeof(IMG_UINT32));

    *ppui8Msg = pui8Msg;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_ReturnMsg

******************************************************************************/
IMG_VOID PVDEC_ReturnMsg(
    const IMG_HANDLE    hVxd,
    IMG_UINT8         * pui8Msg
)
{
    IMG_FREE(pui8Msg);
    pui8Msg = IMG_NULL;
}


/*!
******************************************************************************

 @Function              PVDEC_ProcessMessage

******************************************************************************/
IMG_RESULT PVDEC_ProcessMessage(
    const IMG_HANDLE     hVxd,
    IMG_UINT32         * pui32Msg,
    LST_T              * psPendPictList,
    IMG_HANDLE           hDWRTimerHandle,
    IMG_BOOL           * pbDecodedMsg
)
{
    PVDEC_sContext    * psPvdecCtx = (PVDEC_sContext *)hVxd;
    IMG_UINT8           ui8MsgType;
    IMG_UINT8           ui8MsgGroup;
    DECODER_sDecPict  * psDecPict;
    IMG_UINT32          ui32TransId = 0;
    VDEC_sPictHwCrc *   psPictHwCrc = IMG_NULL;
    IMG_BOOL            bIsHevc = IMG_FALSE;

    #define MSG_GROUP_MASK  0xf0
    #define MSG_PIPE_MASK   0x0f

    IMG_ASSERT(psPvdecCtx);
    IMG_ASSERT(pui32Msg);
    IMG_ASSERT(pbDecodedMsg);
    IMG_ASSERT(psPendPictList);

    if(psPvdecCtx == IMG_NULL ||
       pui32Msg == IMG_NULL ||
       pbDecodedMsg == IMG_NULL ||
       psPendPictList == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameters for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    *pbDecodedMsg = IMG_FALSE;

    ui32TransId = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_GENMSG_TRANS_ID);
    ui8MsgType  = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_GENMSG_MSG_TYPE);
    ui8MsgGroup = ui8MsgType & MSG_GROUP_MASK;

    switch (ui8MsgGroup)
    {
        case MSG_TYPE_START_PSR_MTXHOST_MSG:
        {
            switch (ui8MsgType)
            {
                /* TO DO: there is a separate error message in secure mode,
                          needs to be checked if errors are propagated to user space in such a case*/
                case FW_DEVA_COMPLETED:
#ifdef VDEC_USE_PVDEC_SEC
                case FWDEC_COMPLETED:
#endif /* VDEC_USE_PVDEC_SEC */
                {
                    DECODER_sPictAttrs * psPictAttrs = IMG_NULL;
                    IMG_UINT16           ui16ErrorFlags = 0;
                    IMG_UINT32           ui32NoBeWDTs = 0;
                    IMG_UINT32           ui32MbsDropped = 0;
                    IMG_UINT32           ui32MbsRecovered = 0;


#ifdef VDEC_USE_PVDEC_SEC
                    if(ui8MsgType == FWDEC_COMPLETED)
                    {
                        ui16ErrorFlags = MEMIO_READ_FIELD(pui32Msg, FWDEC_COMPLETED_ERROR_FLAGS);
                    }
                    else
#endif /* VDEC_USE_PVDEC_SEC */
                    {
                        ui16ErrorFlags = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_COMPLETED_ERROR_FLAGS);
                        ui32NoBeWDTs = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_COMPLETED_NUM_BEWDTS);
                        ui32MbsDropped = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_COMPLETED_NUM_MBSDROPPED);
                        ui32MbsRecovered = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_COMPLETED_NUM_MBSRECOVERED);
                    }

                    psDecPict = LST_first(psPendPictList);
                    while (psDecPict)
                    {
                        if (psDecPict->ui32TransactionId == ui32TransId)
                        {
                            break;
                        }
                        psDecPict = LST_next(psDecPict);
                    }

                    // We must have a picture in the list that matches the transaction id
                    IMG_ASSERT(psDecPict);
                    if(psDecPict == IMG_NULL)
                    {
                        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                            "Firmware decoded message received with no pending picture");
                        return IMG_ERROR_FATAL;
                    }

                    IMG_ASSERT(psDecPict->psFirstFieldFwMsg);
                    IMG_ASSERT(psDecPict->psSecondFieldFwMsg);
                    if(psDecPict->psFirstFieldFwMsg == IMG_NULL ||
                       psDecPict->psSecondFieldFwMsg == IMG_NULL)
                    {
                        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                            "Invalid pending picture struct");
                        return IMG_ERROR_FATAL;
                    }

                    if(psDecPict->psFirstFieldFwMsg->sPictAttrs.bFirstFieldReceived)
                    {
                        psPictAttrs = &psDecPict->psSecondFieldFwMsg->sPictAttrs;
                    }
                    else
                    {
                        psPictAttrs = &psDecPict->psFirstFieldFwMsg->sPictAttrs;
                        psDecPict->psFirstFieldFwMsg->sPictAttrs.bFirstFieldReceived = IMG_TRUE;
                    }

                    psPictAttrs->ui32FEError |= (IMG_UINT32)ui16ErrorFlags;
                    psPictAttrs->ui32NoBEDWT = ui32NoBeWDTs;
                    psPictAttrs->ui32MbsDropped = ui32MbsDropped;
                    psPictAttrs->ui32MbsRecovered = ui32MbsRecovered;

                    psDecPict->bDecoded = IMG_TRUE;

                    *pbDecodedMsg = IMG_TRUE;

                    DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[TID=0x%08X] [COMPLETION_DECODED]", ui32TransId);

                    break;
                }

                case FW_DEVA_PANIC:
                {
                    IMG_UINT32  ui32PanicInfo =  MEMIO_READ_FIELD(pui32Msg, FW_DEVA_PANIC_ERROR_INT);
                    IMG_CHAR    szPanicReason[PANIC_REASON_LEN] = "Reason(s): ";
                    IMG_BOOL    bPanicResonIdentified = IMG_FALSE;

                    psPvdecCtx->bFWPanic = IMG_TRUE;

                    /* Create panic reason string. */
                    if (REGIO_READ_FIELD(ui32PanicInfo, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_STATUS, CR_HOST_SYS_WDT))
                    {
                        strncat(szPanicReason, apszPanicReason[PANIC_REASON_WDT], PANIC_REASON_LEN - 1);
                        bPanicResonIdentified = IMG_TRUE;
                    }
                    if (REGIO_READ_FIELD(ui32PanicInfo, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_STATUS, CR_HOST_READ_TIMEOUT_PROC_IRQ))
                    {
                        strncat(szPanicReason, apszPanicReason[PANIC_REASON_READ_TIMEOUT], PANIC_REASON_LEN - 1);
                        bPanicResonIdentified = IMG_TRUE;
                    }
                    if (REGIO_READ_FIELD(ui32PanicInfo, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_STATUS, CR_HOST_COMMAND_TIMEOUT_PROC_IRQ))
                    {
                        strncat(szPanicReason, apszPanicReason[PANIC_REASON_CMD_TIMEOUT], PANIC_REASON_LEN - 1);
                        bPanicResonIdentified = IMG_TRUE;
                    }
                    if (!bPanicResonIdentified)
                    {
                        strncat(szPanicReason, apszPanicReason[PANIC_REASON_OTHER], PANIC_REASON_LEN - 1);
                    }
                    szPanicReason[IMG_STRLEN(szPanicReason) - 2] = 0;

                    if (0 != ui32TransId)
                    {
                        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "[TID=0x%08X] [!!!FIRMWARE PANIC!!! %s]", ui32TransId, szPanicReason);
                    }
                    else
                    {
                        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "[TID=NULL] [!!!GENERAL FIRMWARE PANIC!!! %s]", szPanicReason);
                    }

                    /* Fire the DWR timer immediately if handle provided. */
                    if(hDWRTimerHandle)
                    {
                        SYSOSKM_FireTimer(hDWRTimerHandle);
                    }

                    return IMG_ERROR_FATAL;
                }

                case FW_ASSERT:
                {
                    const PVDECFW_sPvdecFwFileInfo  * psPvdecFwFileInfo = IMG_NULL;

                    IMG_UINT32 ui32FwFileNameHash = MEMIO_READ_FIELD(pui32Msg, FW_ASSERT_FILE_NAME_HASH);
                    IMG_UINT32 ui32FwFileLine     = MEMIO_READ_FIELD(pui32Msg, FW_ASSERT_FILE_LINE);
#ifdef VDEC_USE_PVDEC_SEC
                    IMG_BOOL   bSecure            = MEMIO_READ_FIELD(pui32Msg, FW_ASSERT_SECURE);

                    /* Logs database is provided with firmware binary */
                    if (bSecure)
                    {
                        psPvdecFwFileInfo = VDECFW_GetFwFileInfoSec(ui32FwFileNameHash);
                    }
                    else
#endif /* VDEC_USE_PVDEC_SEC */
                    {
                        psPvdecFwFileInfo = VDECFW_GetFwFileInfoFull(ui32FwFileNameHash);
                    }

                    psPvdecCtx->bFWPanic = IMG_TRUE;

                    if (IMG_NULL != psPvdecFwFileInfo)
                    {
                        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[TID=NULL] [!!!FIRMWARE ASSERT: %s:%d!!!]",
                                     psPvdecFwFileInfo->pszFwFileName, ui32FwFileLine);
                    }
                    else
                    {
                        DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[TID=NULL] [!!!FIRMWARE ASSERT: 0x%08x:%d!!!]",
                                     ui32FwFileNameHash, ui32FwFileLine);
                    }

                    /* Fire the DWR timer immediately if handle provided. */
                    if(hDWRTimerHandle)
                    {
                        SYSOSKM_FireTimer(hDWRTimerHandle);
                    }

                    return IMG_ERROR_FATAL;
                }

#ifdef FW_PERFORMANCE_LOGGING
                case FW_PERF:
                {
                    IMG_UINT32 ui32Timestamp;
                    ui32Timestamp = MEMIO_READ_FIELD(pui32Msg, FW_PERF_TIMESTAMP);
                    REPORT(REPORT_MODULE_VXDIO, REPORT_NOTICE,
                           "PERF_SYNC: 0x%08X",
                           ui32Timestamp);
                    break;
                }
#endif /* FW_PERFORMANCE_LOGGING */

#ifdef VDEC_USE_PVDEC_SEC
                case FWBSP_NEW_SEQ:
                case FWBSP_NEW_PIC:
                case FWBSP_ERROR:
                case FWBSP_BUF_EMPTY:
                {
                    IMG_UINT16 ui16MsgSize;
                    IMG_UINT32 ui32ParseId;
                    IMG_RESULT ui32Result;

                    ui16MsgSize = MEMIO_READ_FIELD(pui32Msg, FWBSP_GENMSG_MSG_SIZE);
                    ui32ParseId = MEMIO_READ_FIELD(pui32Msg, FWBSP_GENMSG_PARSE_ID);

                    pvdec_DumpMsg((IMG_UINT8 *)pui32Msg, ui16MsgSize * sizeof(IMG_UINT32),
                        pszMsgString);

                    ui32Result = FWBSP_BRIDGE_FwMsgBufFill(psPvdecCtx->hFWBSP_BRIDGEStrCtx,
                                                           ui32ParseId,
                                                           pui32Msg,
                                                           ui16MsgSize);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return IMG_ERROR_FATAL;
                    }

                    break;
                }
#endif /* VDEC_USE_PVDEC_SEC */

                default:
                {
                    IMG_ASSERT(IMG_FALSE);
                    break;
                }
            }
            break;
        }

        case FW_DEVA_SIGNATURES_HEVC:
        {
            bIsHevc = IMG_TRUE;
        }
        case FW_DEVA_SIGNATURES_LEGACY:
        {
            IMG_UINT32  * pui32Signatures = pui32Msg + (FW_DEVA_SIGNATURES_SIGNATURES_OFFSET / sizeof(IMG_UINT32));
            IMG_UINT8     ui8SigCount  = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_SIGNATURES_MSG_SIZE) -
                                         ((FW_DEVA_SIGNATURES_SIZE/sizeof(IMG_UINT32)) - 1);
            IMG_UINT32    ui32Selected = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_SIGNATURES_SIGNATURE_SELECT);
            IMG_UINT8     i, j = 0;

            psDecPict = LST_first(psPendPictList);
            while (psDecPict)
            {
                if (psDecPict->ui32TransactionId == ui32TransId)
                {
                    break;
                }
                psDecPict = LST_next(psDecPict);
            }

            // We must have a picture in the list that matches the tid
            IMG_ASSERT(psDecPict);
            if(psDecPict == IMG_NULL)
            {
                REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                       "Firmware signatures message received with no pending picture");
                return IMG_ERROR_FATAL;
            }

            IMG_ASSERT(psDecPict->psFirstFieldFwMsg);
            IMG_ASSERT(psDecPict->psSecondFieldFwMsg);
            if(psDecPict->psFirstFieldFwMsg == IMG_NULL ||
                psDecPict->psSecondFieldFwMsg == IMG_NULL)
            {
                REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR,
                    "Invalid pending picture struct");
                return IMG_ERROR_FATAL;
            }
            if(psDecPict->psFirstFieldFwMsg->sPictHwCrc.bFirstFieldReceived)
            {
                psPictHwCrc = &psDecPict->psSecondFieldFwMsg->sPictHwCrc;
            }
            else
            {
                psPictHwCrc = &psDecPict->psFirstFieldFwMsg->sPictHwCrc;
                if (ui32Selected & (PVDEC_SIGNATURE_GROUP_20 | PVDEC_SIGNATURE_GROUP_24))
                {
                    psDecPict->psFirstFieldFwMsg->sPictHwCrc.bFirstFieldReceived = IMG_TRUE;
                }
            }

            for (i = 0; i < 32; i++)
            {
                IMG_UINT32 ui32Group = ui32Selected & (1 << i);

                switch(ui32Group)
                {
                    case PVDEC_SIGNATURE_GROUP_1:
                    {
                         /* This group is requested only when EXTRA_HWDEBUG i/f is turned on
                          * in firmware side, so just make dummy read to get this recorded by pdump.
                          * In case of hevc we have 2 additional preceding signatures in the buffer */
                         if (bIsHevc)
                         {
                             psPictHwCrc->ui32CrcHevcParser = pui32Signatures[j++];
                             psPictHwCrc->ui32CrcEncap = pui32Signatures[j++];
                         }
                         /* Output i/f signature */
                         psPictHwCrc->ui32CrcHwParams = pui32Signatures[j++];
                         // XXX TODO: would be good to add pdump comments describing signatures
                         break;
                    }
                    case PVDEC_SIGNATURE_GROUP_20:
                    {
                         psPictHwCrc->ui32CrcVdmcPixRecon = pui32Signatures[j++];
                         break;
                    }
                    case PVDEC_SIGNATURE_GROUP_24:
                    {
                         psPictHwCrc->ui32VdebSysMemWriteData = pui32Signatures[j++];
                         break;
                    }
                    default:
                    {
                         // do nothing
                         break;
                    }
                }
            }

            // sanity check
            ui8SigCount -= j;
            IMG_ASSERT(ui8SigCount == 0);

            // suppress PVDEC_SIGNATURE_GROUP_1 and notify only about groups used for verification
            if (ui32Selected & (PVDEC_SIGNATURE_GROUP_20 | PVDEC_SIGNATURE_GROUP_24))
            {
                DEBUG_REPORT(REPORT_MODULE_HWCTRL, "[TID=0x%08X] [SIGNATURES]", ui32TransId);
            }

            break;
        }

        default:
        {
            IMG_ASSERT(IMG_FALSE);
            break;
        }
    }

    return IMG_SUCCESS;
}


#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              PVDEC_GetFWBSPInitMessage

******************************************************************************/
static IMG_RESULT pvdec_GetFWBSPInitMessage(
    PVDEC_sContext               * psPvdecContext,
    const HWCTRL_sInitBSPMsgData * psInitBSPMsgData,
    IMG_UINT8                   ** ppui8Msg
)
{
    FWBSP_BRIDGE_sFwMsgData *psFwMsgData;
    IMG_UINT8 ui8StreamId, *pui8Msg;
 
    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psInitBSPMsgData);
    IMG_ASSERT(ppui8Msg);

    psFwMsgData = psInitBSPMsgData->psFwMsgData;
    IMG_ASSERT(psFwMsgData);
    if(psFwMsgData == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "FW message data missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    pui8Msg = IMG_MALLOC(FWBSP_INIT_SIZE);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, FWBSP_INIT_SIZE);

    ui8StreamId = FWBSP_BRIDGE_GET_STREAM_ID(psFwMsgData->ui32ParseId);

    /* TO DO: init message contains stream-releated data, this needs to be reworked of 
       multistream scenario */

    /* Header */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE, FWBSP_INIT_SIZE/sizeof(IMG_UINT32));
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE, FWBSP_INIT);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_ID, ++psPvdecContext->ui16CurrentMsgId);

    /* Data */
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_VID_STD, psFwMsgData->ui8VidStd);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_PTD_ADDR,
        psFwMsgData->ui32PtdAddr + (ui8StreamId * sizeof(IMG_UINT32)));
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_RING_BUF_ADDR, psFwMsgData->ui32RingBufAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_RING_BUF_SIZE, psFwMsgData->ui32RingBufSize);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_RING_BUF_MARK, psFwMsgData->ui32RingBufMark);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_SPS_BUF_ADDR, psFwMsgData->ui32SPSAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_SPS_BUF_SIZE, psFwMsgData->ui32SPSSize);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_PPS_BUF_ADDR, psFwMsgData->ui32PPSAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_PPS_BUF_SIZE, psFwMsgData->ui32PPSSize);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_VPS_BUF_ADDR, psFwMsgData->ui32VPSAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_INIT_VPS_BUF_SIZE, psFwMsgData->ui32VPSSize);

    pvdec_DumpMsg(pui8Msg, FWBSP_INIT_SIZE, "FWBSP_INIT");

    *ppui8Msg = pui8Msg;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_GetFWDECInitMessage

******************************************************************************/
static IMG_RESULT pvdec_GetFWDECInitMessage(
    PVDEC_sContext                    * psPvdecContext,
    const HWCTRL_sInitSecDecMsgData   * psInitSecDecMsgData,
    IMG_UINT8                        ** ppui8Msg
)
{
    IMG_BOOL                       bHWCRCGen = IMG_FALSE;
    IMG_UINT8                    * pui8Msg;
    VXDIO_sDdBufInfo             * psRendecBufInfo;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psInitSecDecMsgData);
    IMG_ASSERT(ppui8Msg);

    psRendecBufInfo = psInitSecDecMsgData->psRendecBufInfo;
    IMG_ASSERT(psRendecBufInfo);
    if(psRendecBufInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Rendec buffer info missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    pui8Msg = IMG_MALLOC(FWDEC_INIT_SIZE);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, FWDEC_INIT_SIZE);

    /* Header */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE, FWDEC_INIT_SIZE/sizeof(IMG_UINT32));
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE, FWDEC_INIT);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_ID, ++psPvdecContext->ui16CurrentMsgId);

    /* Data */
    /* DEVA PVDEC FW uses only first (in message) rendec buffer currently and splits it itself. */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_INIT_RENDEC_ADDR0, psRendecBufInfo[0].ui32DevVirt);
    /* DEVA PVDEC FW expects rendec buffer size to be given in pages */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_INIT_RENDEC_SIZE0, psRendecBufInfo[0].ui32BufSize/(DEV_MMU_PAGE_SIZE));

    /* Check if CRC generation is required. */
    {
        DBGOPT_sValue  sVal;
        if (DBGOPTKM_GetSize("HardwareCRCGen", DBGOPT_TYPE_BOOL) &&
            DBGOPTKM_Get("HardwareCRCGen", IMG_NULL, &sVal))
        {
            bHWCRCGen = sVal.bVal;
        }
    }

    /* Request signatures (CRCs) for picture verification if required. */
    if(bHWCRCGen)
    {
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_INIT_HW_SIG_SELECT, (PVDEC_SIGNATURE_GROUP_20 | PVDEC_SIGNATURE_GROUP_24));
    }

    pvdec_DumpMsg(pui8Msg, FWDEC_INIT_SIZE, "FWDEC_INIT");

    *ppui8Msg = pui8Msg;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDEC_GetFWDECConfigure

******************************************************************************/
static IMG_RESULT pvdec_GetFWDECConfigureMessage(
    PVDEC_sContext                   * psPvdecContext,
    const HWCTRL_sConfigureMsgData   * psConfigureMsgData,
    IMG_UINT8                       ** ppui8Msg
)
{
    IMG_UINT8 * pui8Msg;
    const VDECDD_sDdDevConfig  * psDevConfig;
    const VDECFW_sCoreInitData * psCoreInitData;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(ppui8Msg);
    IMG_ASSERT(psConfigureMsgData);

    psDevConfig = psConfigureMsgData->psDevConfig;
    IMG_ASSERT(psDevConfig);
    if(psDevConfig == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Device config info missing");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psCoreInitData = psConfigureMsgData->psCoreInitData;
    IMG_ASSERT(psCoreInitData);
    if(psCoreInitData == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Core init data missing");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if (psDevConfig->eTileScheme == VDEC_TS_NONE)
    {
        return IMG_ERROR_INTERRUPTED;
    }

    pui8Msg = IMG_MALLOC(FWDEC_CONFIGURE_SIZE);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, FWDEC_CONFIGURE_SIZE);

    /* Header */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE, FWDEC_CONFIGURE_SIZE/sizeof(IMG_UINT32));
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE, FWDEC_CONFIGURE);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_ID, ++psPvdecContext->ui16CurrentMsgId);

    /* Fill mmu tiling configuration info */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_CONTROL0,
        psCoreInitData->sMmuConfig.ui32MmuControl0);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MIN_ADDR0,
        psCoreInitData->sMmuConfig.ui32MmuTileMinAddr[0]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MAX_ADDR0,
        psCoreInitData->sMmuConfig.ui32MmuTileMaxAddr[0]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MIN_ADDR1,
        psCoreInitData->sMmuConfig.ui32MmuTileMinAddr[1]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MAX_ADDR1,
        psCoreInitData->sMmuConfig.ui32MmuTileMaxAddr[1]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MIN_ADDR2,
        psCoreInitData->sMmuConfig.ui32MmuTileMinAddr[2]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MAX_ADDR2,
        psCoreInitData->sMmuConfig.ui32MmuTileMaxAddr[2]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MIN_ADDR3,
        psCoreInitData->sMmuConfig.ui32MmuTileMinAddr[3]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_CONFIGURE_MMU_TILE_MAX_ADDR3,
        psCoreInitData->sMmuConfig.ui32MmuTileMaxAddr[3]);

    pvdec_DumpMsg(pui8Msg, FWDEC_CONFIGURE_SIZE, "FWDEC_CONFIGURE");

    *ppui8Msg = pui8Msg;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDEC_GetFWDECDecodeMessage

******************************************************************************/
static IMG_RESULT pvdec_GetFWDECDecodeMessage(
    PVDEC_sContext               * psPvdecContext,
    const HWCTRL_sDecMsgData     * psDecMsgData,
    IMG_UINT8                   ** ppui8Msg
)
{
    VDECFW_sTransaction          * psTransaction;
    VXDIO_sDdBufInfo             * psStrPtdBufInfo;
    IMG_UINT16                    ui16Flags = 0;
    IMG_UINT16                    ui16MsgSize;
    IMG_UINT8                     ui8VdecFlags = 0;
    IMG_UINT8                   * pui8Msg;
    DECODER_sDecPict            * psDecPict;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(ppui8Msg);
    IMG_ASSERT(psDecMsgData);

    psDecPict = psDecMsgData->psDecPict;
    IMG_ASSERT(psDecPict);
    if(psDecPict == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Decoded picture missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psStrPtdBufInfo = psDecPict->psStrPtdBufInfo;
    IMG_ASSERT(psStrPtdBufInfo);
    if(psStrPtdBufInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Ptd buffer info missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    IMG_ASSERT(psDecPict->psTransactionInfo);
    IMG_ASSERT(psDecPict->psTransactionInfo && 
               psDecPict->psTransactionInfo->psDdBufInfo);
    IMG_ASSERT(psDecPict->psTransactionInfo && 
               psDecPict->psTransactionInfo->psDdBufInfo && 
               psDecPict->psTransactionInfo->psDdBufInfo->pvCpuVirt);
    if(psDecPict->psTransactionInfo == IMG_NULL ||
       psDecPict->psTransactionInfo->psDdBufInfo == IMG_NULL ||
       psDecPict->psTransactionInfo->psDdBufInfo->pvCpuVirt == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Transaction info missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    //TO DO: find better way to get transaction info, can't we just reuse vdec_ext structure ?
    psTransaction = (VDECFW_sTransaction *)psDecPict->psTransactionInfo->psDdBufInfo->pvCpuVirt;
    /* Calc msg size, FWDEC_DECODE_SIZE + noOfPlanes * noOfReferences +
     * noOfColocatedPictures*/
#ifdef ENABLE_SEPARATE_COLOUR_PLANES
    /* We could check whether bSepColPlane is set */
    ui16MsgSize = FWDEC_DECODE_SIZE + (3 * psDecPict->ui8RefPicQty +
        psDecPict->ui8ColPicQty) * sizeof(IMG_UINT32);
#else /* ENABLE_SEPARATE_COLOUR_PLANES */
    ui16MsgSize = FWDEC_DECODE_SIZE + (2 * psDecPict->ui8RefPicQty +
        psDecPict->ui8ColPicQty) * sizeof(IMG_UINT32);
#endif /* ndef ENABLE_SEPARATE_COLOUR_PLANES */

    pui8Msg = IMG_MALLOC(ui16MsgSize);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, ui16MsgSize);

    ui16Flags |= FW_DEVA_RENDER_HOST_INT;

    if (psPvdecContext->bMMUFlushNeeded == IMG_TRUE)
    {
        ui8VdecFlags |= FW_VDEC_MMU_FLUSH_CACHE;
        psPvdecContext->bMMUFlushNeeded = IMG_FALSE;
    }

    ui16Flags |= FW_SECURE_STREAM;

    /* PTD. Host pointer has to be updated because the value was temporary stored in the device memory*/
    UPDATE_HOST(psStrPtdBufInfo);

    /* Header */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE, ui16MsgSize / sizeof(IMG_UINT32));
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE, FWDEC_DECODE);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_ID, ++psPvdecContext->ui16CurrentMsgId);

    /* Data */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_TRANS_ID,
        psTransaction->ui32TransactionId);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_FLAGS, ui16Flags);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_VDEC_FLAGS, ui8VdecFlags);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_VID_STD, psTransaction->eCodec);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_STREAMID,
        GET_STREAM_ID(psTransaction->ui32TransactionId));

    {
        PIXEL_eChromaInterleaved eChromaInterleaved;
        IMG_BOOL bChromaInterleaved;

        IMG_ASSERT(psDecPict->psReconPict);

        if (psDecPict->psReconPict->psPictBuf->psDdStrContext)
        {
            eChromaInterleaved =
                psDecPict->psReconPict->psPictBuf->psDdStrContext->sComSequHdrInfo.sPixelInfo.eChromaInterleaved;
        }
        else
        {
            IMG_ASSERT(psDecPict->psAltPict);
            eChromaInterleaved =
                psDecPict->psAltPict->psPictBuf->psDdStrContext->sComSequHdrInfo.sPixelInfo.eChromaInterleaved;
        }
        bChromaInterleaved = PIXEL_GET_HW_CHROMA_INTERLEAVED(eChromaInterleaved);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_CHROMA_INTERLEAVED,
            bChromaInterleaved ? 1 : 0);
    }

    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_REF_PIC_MASK,
        psDecPict->ui16RefPicMask);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_PTD_ADDR, psStrPtdBufInfo->ui32DevVirt +
        (GET_STREAM_ID(psTransaction->ui32TransactionId) * sizeof(IMG_UINT32)));
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_TILE_CFG, psDecPict->ui32TileCfg);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_MB_PARAMS_ADDR,
        psTransaction->ui32MbParamsBaseAddress);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_MB_PARAMS_PLANE_SIZE,
        psTransaction->ui32MbParamsSizePerPlane);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_PLANE_ADDR0,
        psTransaction->aui32PictCmds[VDECFW_CMD_LUMA_RECONSTRUCTED_PICTURE_BASE_ADDRESS]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_PLANE_ADDR1,
        psTransaction->aui32PictCmds[VDECFW_CMD_CHROMA_RECONSTRUCTED_PICTURE_BASE_ADDRESS]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_PLANE_ADDR2,
        psTransaction->aui32PictCmds[VDECFW_CMD_CHROMA2_RECONSTRUCTED_PICTURE_BASE_ADDRESS]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_DISPLAY_PIC_SIZE,
        psTransaction->aui32PictCmds[VDECFW_CMD_DISPLAY_PICTURE]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_CODED_PIC_SIZE,
        psTransaction->aui32PictCmds[VDECFW_CMD_CODED_PICTURE]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_INTRA_BUF_ADDR,
        psTransaction->aui32PictCmds[VDECFW_CMD_INTRA_BUFFER_BASE_ADDRESS]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_INTRA_BUF_PLANE_SIZE,
         psTransaction->aui32PictCmds[VDECFW_CMD_INTRA_BUFFER_PLANE_SIZE]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_AUX_LINE_BUF_ADDR,
        psTransaction->aui32PictCmds[VDECFW_CMD_AUX_LINE_BUFFER_BASE_ADDRESS]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_OPERATING_MODE,
        psTransaction->aui32PictCmds[VDECFW_CMD_OPERATING_MODE]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_EXT_OP_MODE,
        psTransaction->aui32PictCmds[VDECFW_CMD_EXT_OP_MODE]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_EXT_ROW_STRIDE,
        psTransaction->aui32PictCmds[VDECFW_CMD_EXTENDED_ROW_STRIDE]);

    /* Error concealment info */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_LUMA_ERROR_BASE_ADDR,
        psTransaction->aui32PictCmds[VDECFW_CMD_LUMA_ERROR_PICTURE_BASE_ADDRESS]);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_CHROMA_ERROR_BASE_ADDR,
        psTransaction->aui32PictCmds[VDECFW_CMD_CHROMA_ERROR_PICTURE_BASE_ADDRESS]);

    /* H264 only */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_CHROMA_ROW_STRIDE,
        psTransaction->aui32PictCmds[VDECFW_CMD_CHROMA_ROW_STRIDE]);

    /* VLC tables */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_VLC_BUF_ADDR,
        psDecPict->psVlcTablesBufInfo->ui32DevVirt);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_VLC_BUF_SIZE,
        psDecPict->psVlcTablesBufInfo->ui32BufSize);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_VLC_IDX_BUF_ADDR,
        psDecPict->psVlcIdxTableBufInfo->ui32DevVirt);
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_VLC_IDX_BUF_SIZE,
        psDecPict->psVlcIdxTableBufInfo->ui32BufSize);

    /* Number of colocated pictures */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_COL_PICS_NUM,
        psDecPict->ui8ColPicQty);

    /* Ring buffer mark position */
    MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_RB_MARK,
        psDecPict->ui32RingBufMark);

    if(psDecPict->psHevcInfo)
    {
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_ID,
            psDecPict->ui16GencId);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_ADDR0,
            psDecPict->pasGencBuffers[0]->sDdBufInfo.ui32DevVirt);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_ADDR1,
            psDecPict->pasGencBuffers[1]->sDdBufInfo.ui32DevVirt);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_ADDR2,
            psDecPict->pasGencBuffers[2]->sDdBufInfo.ui32DevVirt);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_ADDR3,
            psDecPict->pasGencBuffers[3]->sDdBufInfo.ui32DevVirt);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_SIZE0,
            psDecPict->pasGencBuffers[0]->sDdBufInfo.ui32BufSize);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_SIZE1,
            psDecPict->pasGencBuffers[1]->sDdBufInfo.ui32BufSize);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_SIZE2,
            psDecPict->pasGencBuffers[2]->sDdBufInfo.ui32BufSize);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_BUF_SIZE3,
            psDecPict->pasGencBuffers[3]->sDdBufInfo.ui32BufSize);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_GENC_FRAG_BUF_ADDR,
            psDecPict->psGencFragmentBuffer->sDdBufInfo.ui32DevVirt);
        MEMIO_WRITE_FIELD(pui8Msg, FWDEC_DECODE_ABOVE_PARAMS_ADDR,
            psDecPict->psHevcInfo->psDdBufInfo->ui32DevVirt);
    }
    /* References */
    if (psDecPict->ui8RefPicQty || psDecPict->ui8ColPicQty)
    {
#define MEMIO_WRITE_DWORD(vpMem, offset, uiValue)                                                   \
    MEMIO_CHECK_ALIGNMENT(vpMem);                                                                   \
    (*((IMG_UINT32 *)(((IMG_UINTPTR)(vpMem)) + offset))) =                                          \
    (IMG_UINT32)(((*((IMG_UINT32 *)(((IMG_UINTPTR)(vpMem)) + offset))) & ~(IMG_UINT32)0xFFFFFFFF) | \
    (IMG_UINT32)((uiValue) & 0xFFFFFFFF));

        IMG_UINT8 ui8i;
        IMG_UINT16 ui16Offset;

        ui16Offset = FWDEC_DECODE_SIZE;
        for (ui8i = 0; ui8i < psDecPict->ui8RefPicQty; ui8i++)
        {
            MEMIO_WRITE_DWORD(pui8Msg, ui16Offset,
                psDecPict->aui32RefPicLumaAddrList[ui8i]);
            ui16Offset += sizeof(IMG_UINT32);

            MEMIO_WRITE_DWORD(pui8Msg, ui16Offset,
                psDecPict->aui32RefPicChromaAddrList[ui8i]);
            ui16Offset += sizeof(IMG_UINT32);

#ifdef ENABLE_SEPARATE_COLOUR_PLANES
            MEMIO_WRITE_DWORD(pui8Msg, ui16Offset,
                psDecPict->aui32RefPicChroma2AddrList[ui8i]);
            ui16Offset += sizeof(IMG_UINT32);
#endif /* ENABLE_SEPARATE_COLOUR_PLANES */
        }

        for (ui8i = 0; ui8i < psDecPict->ui8ColPicQty; ui8i++)
        {
            MEMIO_WRITE_DWORD(pui8Msg, ui16Offset,
                psDecPict->aui32ColPicAddrList[ui8i]);
            ui16Offset += sizeof(IMG_UINT32);
        }
        IMG_ASSERT(ui16Offset == ui16MsgSize);
#undef MEMIO_WRITE_DWORD
    }

    pvdec_DumpMsg(pui8Msg, ui16MsgSize, "FWDEC_DECODE");

    *ppui8Msg = pui8Msg;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_GetParseBitstreamMessage

******************************************************************************/
static IMG_RESULT pvdec_GetParseBitstreamMessage(
    PVDEC_sContext                * psPvdecContext,
    const HWCTRL_sParseMsgData    * psParseMsgData,
    IMG_UINT8                    ** ppui8Msg
)
{
    FWBSP_BRIDGE_sFwMsgData *psFwMsgData;
    IMG_UINT16 ui16Offset, ui16MsgSize;
    IMG_UINT8 ui8i, *pui8Msg;
    IMG_UINT8 ui8StreamId, ui8VdecFlags = 0;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psParseMsgData);
    IMG_ASSERT(ppui8Msg);

    psFwMsgData = psParseMsgData->psFwMsgData;
    IMG_ASSERT(psFwMsgData);
    if(psFwMsgData == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "FW message data missing!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }
    /* this assumes only one secure stream in the system, it needs to be reworked for
       multistream scenario */
    psPvdecContext->hFWBSP_BRIDGEStrCtx = psParseMsgData->hFWBSP_BRIDGECtx;

    ui16MsgSize = FWBSP_PARSE_BITSTREAM_SIZE + 8 * psFwMsgData->ui8BitstreamQty;
    pui8Msg = IMG_MALLOC(ui16MsgSize);
    if (pui8Msg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate message buffer");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pui8Msg, 0, ui16MsgSize);

    ui8StreamId = FWBSP_BRIDGE_GET_STREAM_ID(psFwMsgData->ui32ParseId);
    if (psPvdecContext->bMMUFlushNeeded == IMG_TRUE)
    {
        ui8VdecFlags |= FW_VDEC_MMU_FLUSH_CACHE;
        psPvdecContext->bMMUFlushNeeded = IMG_FALSE;
    }

    if (psFwMsgData->bSizeDelimMode)
    {
        ui8VdecFlags |= FW_VDEC_NAL_SIZE_DELIM;
    }

    // put security flag into vdec command flags for now
    // TODO: add generic flags field for FWBSP_PARSE_BITSTREAM
    ui8VdecFlags |= FW_SECURE_STREAM;

    /* Header */
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_SIZE,
        ui16MsgSize / sizeof(IMG_UINT32));
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_TYPE,
        FWBSP_PARSE_BITSTREAM);
    MEMIO_WRITE_FIELD(pui8Msg, FW_DEVA_GENMSG_MSG_ID,
        ++psPvdecContext->ui16CurrentMsgId);

    /* Data */
    /* F/W load/save contextes */
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_FW_CTX_LOAD_ADDR,
        psFwMsgData->ui32FwCtxLoadAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_FW_CTX_SAVE_ADDR,
        psFwMsgData->ui32FwCtxSaveAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_FW_CTX_SIZE,
        psFwMsgData->ui32FwCtxSize);

    /* Encrypted bitstream */
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_CODEC_CFG,
        psFwMsgData->bCodecCfg);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_FIRST_GOP,
        psFwMsgData->bFirstGOP);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_VDEC_FLAGS,
        ui8VdecFlags);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_CODEC_MODE,
        psFwMsgData->ui8CodecMode);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_BITSTREAM_QTY,
        psFwMsgData->ui8BitstreamQty);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_PARSE_ID,
        psFwMsgData->ui32ParseId);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_PTD_ADDR,
        psFwMsgData->ui32PtdAddr + (ui8StreamId * sizeof(IMG_UINT32)));

    /* Picture resource buffers */
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_MEM2REG_BUF_ADDR,
        psFwMsgData->ui32Mem2RegAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_MEM2REG_BUF_SIZE,
        psFwMsgData->ui32Mem2RegSize);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_SLICE_PARAMS_BUF_ADDR,
        psFwMsgData->ui32SliceParamsAddr);
    MEMIO_WRITE_FIELD(pui8Msg, FWBSP_PARSE_BITSTREAM_SLICE_PARAMS_BUF_SIZE,
        psFwMsgData->ui32SliceParamsSize);

    /* Encrypted bitstream - addresses and sizes */
#define MEMIO_WRITE_DWORD(vpMem, offset, uiValue)                                                   \
    MEMIO_CHECK_ALIGNMENT(vpMem);                                                                   \
    (*((IMG_UINT32 *)(((IMG_UINTPTR)(vpMem)) + offset))) =                                          \
    (IMG_UINT32)(((*((IMG_UINT32 *)(((IMG_UINTPTR)(vpMem)) + offset))) & ~(IMG_UINT32)0xFFFFFFFF) | \
    (IMG_UINT32)((uiValue) & 0xFFFFFFFF));

    ui16Offset = FWBSP_PARSE_BITSTREAM_SIZE;

    for (ui8i = 0; ui8i < psFwMsgData->ui8BitstreamQty; ui8i++)
    {
        MEMIO_WRITE_DWORD(pui8Msg, ui16Offset,
            psFwMsgData->aui32BitstreamAddr[ui8i]);
        ui16Offset += sizeof(IMG_UINT32);

        MEMIO_WRITE_DWORD(pui8Msg, ui16Offset,
            psFwMsgData->aui32BitstreamSize[ui8i]);
        ui16Offset += sizeof(IMG_UINT32);
    }
    IMG_ASSERT(ui16Offset == ui16MsgSize);

#undef MEMIO_WRITE_DWORD

    *ppui8Msg = pui8Msg;

    pvdec_DumpMsg(pui8Msg, ui16MsgSize, "FWBSP_PARSE_BITSTREAM");

    return IMG_SUCCESS;
}
#endif /* VDEC_USE_PVDEC_SEC */

IMG_RESULT PVDEC_GetMessage(
    const IMG_HANDLE               hVxd,
    const HWCTRL_eMsgId            eMsgId,
    const HWCTRL_sMsgData       *  psMsgData,
    IMG_UINT8                   ** ppui8Msg
)
{
    PVDEC_sContext *psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(ppui8Msg);
    IMG_ASSERT(psMsgData);

    if(psPvdecContext == IMG_NULL ||
       ppui8Msg == IMG_NULL ||
       psMsgData == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameters for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    switch (eMsgId)
    {
    case HWCTRL_MSGID_INIT_NON_SEC_DEC:
        {
            return pvdec_GetInitNonSecMessage(&psMsgData->sInitNonSecDecMsgData, ppui8Msg);
        }
    case HWCTRL_MSGID_BATCH:
        {
            return pvdec_GetBatchMessage(psPvdecContext, &psMsgData->sBatchMsgData, ppui8Msg);
        }
#ifdef VDEC_USE_PVDEC_SEC
    case HWCTRL_MSGID_INIT_BSP:
        {
            return pvdec_GetFWBSPInitMessage(psPvdecContext, &psMsgData->sInitBSPMsgData, ppui8Msg);
        }
    case HWCTRL_MSGID_INIT_SEC_DEC:
        {
            return pvdec_GetFWDECInitMessage(psPvdecContext, &psMsgData->sInitSecDecMsgData, ppui8Msg);
        }
    case HWCTRL_MSGID_DEC:
        {
            return pvdec_GetFWDECDecodeMessage(psPvdecContext, &psMsgData->sDecMsgData, ppui8Msg);
        }
    case HWCTRL_MSGID_CONFIG:
        {
            return pvdec_GetFWDECConfigureMessage(psPvdecContext, &psMsgData->sConfigureMsgData, ppui8Msg);
        }
    case HWCTRL_MSGID_PARSE:
        {
            return pvdec_GetParseBitstreamMessage(psPvdecContext, &psMsgData->sParseMsgData, ppui8Msg);
        }
#endif /* VDEC_USE_PVDEC_SEC */
    default:
        REPORT(REPORT_MODULE_VXDIO, REPORT_NOTICE,
              "Trying to get a message that is not supported by PVDEC");
        break;
    }

    return IMG_SUCCESS;
}
