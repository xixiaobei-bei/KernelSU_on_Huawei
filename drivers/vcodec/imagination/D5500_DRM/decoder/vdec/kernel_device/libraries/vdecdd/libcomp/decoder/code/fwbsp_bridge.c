/*!
 *****************************************************************************
 *
 * @File       fwbsp_bridge.c
 * @Title      TODO
 * @Description    TODO
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

#include "fwbsp_bridge.h"

#include "core_api.h"
#include "decoder.h"
#include "hwctrl_api.h"
#include "vdec_api.h"
#include "vdec_params.h"
#include "rman_api.h"
#include "vdecdd_mmu.h"
#include "idgen_api.h"
#include "resource.h"
#include "vxd_int.h"

#include "img_profiles_levels.h"
#include "report_api.h"

#include "interface/bsp_fw_msg.h"
#include "source/core/vdec_fw_interface.h"


/* FWBSP_BRIDGE defines and macros                                           */
#define FWBSP_BRIDGE_RING_BUF_SIZE              (1024 * 1024)
#define FWBSP_BRIDGE_MSG_BUF_SIZE               MSVDX_SIZE_MSG_BUFFER
#define FWBSP_BRIDGE_MAX_NUM_PICTURES           VDECFW_MAX_NUM_PICTURES
#define FWBSP_BRIDGE_MAX_NUM_RES                (FWBSP_ENC_BSTR_BUF_QUEUE_LEN)
#define FWBSP_BRIDGE_MAX_FW_MSG_PER_BITSTREAM   (4)
#define FWBSP_BRIDGE_MAX_CONCURRENT_STREAMS     (16)

#define FWBSP_BRIDGE_FOR_EACH(item, list)                                     \
    for (item = (typeof(item))LST_first(list);                                \
         item != IMG_NULL;                                                    \
         item = (typeof(item))LST_next(item))

/* FWBSP_BRIDGE error codes and error macros                                 */
#define FWBSP_BRIDGE_INT_ERROR_BASE        (0x40000000)
#define FWBSP_BRIDGE_TOO_MANY_BSTR         (FWBSP_BRIDGE_INT_ERROR_BASE | 0x01)

#define FWBSP_IS_UNSUPPORTED_CODE(code) ((code) & VDECFW_UNSUPPORTED_CODE_BASE)
#define FWBSP_IS_INT_ERROR(code) ((code) & FWBSP_BRIDGE_INT_ERROR_BASE)
#define FWBSP_GET_ERROR_CODE(code) ((code) & VDECFW_ERROR_CODE_MASK)


/* FWBSP_BRIDGE internal data types                                          */
/*!
******************************************************************************
 This structure defines the decode picture.
 @brief  FWBSP_BRIDGE Picture
******************************************************************************/
typedef struct
{
    IMG_UINT32 ui32ParseId;
    IMG_UINT32 ui32TransId;
    IMG_UINT32 ui32LumaBaseAddr;
    IMG_UINT32 ui32ChromaBaseAddr;
#ifdef ENABLE_SEPARATE_COLOUR_PLANES
    IMG_UINT32 ui32Chroma2BaseAddr;
#endif /* ENABLE_SEPARATE_COLOUR_PLANES */
    IMG_UINT32 ui32MBParamsAddr;
    IMG_BOOL   bValid;

    /*!< Pointer to stream resource for current picture.                     */
    VDECDD_sPictResInt *psPictResInt;

} fwbsp_bridge_sPicture;


/*!
******************************************************************************
 This type defines the FWBSP_BRIDGE stream state.
 @brief  FWBSP_BRIDGE Stream State
******************************************************************************/
typedef enum
{
    /*!< Stream stopped.                                                     */
    FWBSP_BRIDGE_STRSTATE_STOPPED = 0x00,
    /*!< Stream playing.                                                     */
    FWBSP_BRIDGE_STRSTATE_PLAYING,
    /*!< Stream stopping.                                                    */
    FWBSP_BRIDGE_STRSTATE_STOPPING,

    /*!< Max. stream state.                                                  */
    FWBSP_BRIDGE_STRSTATE_MAX,

} fwbsp_bridge_eStrState;


/*!
******************************************************************************
 This structure contains FW_BRIDGE messages data.
 @brief  FWBSP_BRIDGE messages data.
******************************************************************************/
typedef struct
{
    /*!< List link (allows the structure to be part of a MeOS list).         */
    LST_LINK;
    IMG_BOOL   bProcess;
    IMG_UINT32 ui32ParseId;
    IMG_UINT32 aui32MsgBuf[FWBSP_BRIDGE_MSG_BUF_SIZE];

} fwbsp_bridge_sFwMsgItem;


/*!
******************************************************************************
 This structure contains the FWBSP_BRIDGE Context.
 @brief  FWBSP_BRIDGE Context
******************************************************************************/
typedef struct
{
    /*!< Indicates whether component is initialised.                         */
    IMG_BOOL                   bInitialised;
    /*!< Callback from component to return output.                           */
    VDECDD_pfnCompCallback     pfnCompCallback;
    /*!< Callback user data from component.                                  */
    IMG_VOID *                 pvCompInitUserData;
    /*!< List of incoming F/W messages allocated per stream.                 */
    LST_T                     sFwMsgBufList;
    /*!< List of stream context structures                                   */
    LST_T                     sStreamList;
    /*!< VDECDDMMU Device context.                                           */
    IMG_HANDLE                 hMmuDevHandle;
    /*!< Decoder handle.                                                     */
    IMG_HANDLE                 hDecCtx;

    /*!< Device configuration.                                               */
    const VDECDD_sDdDevConfig *psDevConfig;
    /*!< Concurrent streams counter                                          */
    IMG_UINT8                  ui8StrCnt;

} fwbsp_bridge_sContext;


/*!
******************************************************************************
 This structure contains the stream context.
 @brief  FWBSP_BRIDGE Stream Context
******************************************************************************/
typedef struct
{
    /*!< List link (allows the structure to be part of a MeOS list).         */
    LST_LINK;
    /*!< Pointer to FWBSP_BRIDGE context.                                    */
    fwbsp_bridge_sContext    *psCtx;
    /*!< FWBSP_BRIDGE stream state                                           */
    fwbsp_bridge_eStrState    eStrState;
     /*!< Global configuration.                                              */
    VDEC_sStrConfigData       sConfig;
    /*!< Global Stream ID                                                    */
    IMG_UINT32                ui32StrId;
    /*!< Decoder Stream ID                                                   */
    IMG_UINT32                ui32DecStrId;
    /*!< PTD device virtual address                                          */
    IMG_UINT32                ui32PtdDevVirt;
    /*!< VDECDDMMU stream context.                                           */
    IMG_HANDLE                hMmuStrHandle;
    /*!< Picture-based ID generator handle for use with firmware.            */
    IMG_HANDLE                hParseIdGen;
    /*!< DECODER stream context.                                             */
    IMG_HANDLE                hDecStrHandle;
    /*!< Callback user data from component.                                  */
    IMG_VOID                 *pvUserData;
    /*!< List of encrypted bitstream stream units                            */
    LST_T                     sEncBitstreamStrUnitList;
    /*!< List of fence stream units                                          */
    LST_T                     sFenceStrUnitList;
    /*!< List of picture resources allocated per stream.                     */
    LST_T                     sPictResList;
    /*!< Buffer to hold SPS                                                  */
    VXDIO_sDdBufInfo          sSPSBufInfo;
    /*!< Buffer to hold PSP                                                  */
    VXDIO_sDdBufInfo          sPSPBufInfo;
    /*!< Buffer to hold VPS                                                  */
    VXDIO_sDdBufInfo          sVPSBufInfo;
    /*!< irekb: TODO                                                         */
    VXDIO_sDdBufInfo          sRingBufInfo;
    /*!< FWBSP_BRIDGE host <-> firmware messages data                        */
    FWBSP_BRIDGE_sFwMsgData   sFwMsgData;
    /*!< FWBSP_BRIDGE firmware <-> host messages data                        */
    FWBSP_BRIDGE_sHostMsgData sHostMsgData;
    /*!< irekb: TODO                                                         */
    VDEC_sComSequHdrInfo     *psComSequHdrInfo;
    /*!< irekb: TODO                                                         */
    IMG_UINT32                ui32LastParseId;
    /*!< Pointer to device resource of previous picture.                     */
    VDECDD_sPictResInt       *psCurPictResInt;
    /*!< Pointer to device resource of previous picture.                     */
    VDECDD_sPictResInt       *psPrevPictResInt;
    /*!< irekb: TODO                                                         */
    fwbsp_bridge_sPicture     asPicture[FWBSP_BRIDGE_MAX_NUM_PICTURES];
    /*!< Ring buffer marker of the last parsed unit.                         */
    IMG_UINT32                ui32LastRBMark;
    /*!< Secure memory pool id.                                              */
    SYS_eMemPool              eSecurePool;
    /*!< Missing references in GOP indicator                                 */
    IMG_BOOL                  bMissingRefsInGOP;
    /*!< GOP counter                                                         */
    IMG_UINT32                ui32GOPCnt;
} fwbsp_bridge_sStrContext;


/* FWBSP_BRIDGE internal functions - prototypes                              */
static IMG_RESULT
fwbsp_bridge_StreamStop(
    fwbsp_bridge_sStrContext *psStrCtx);

static IMG_RESULT
fwbsp_bridge_StreamResourceCreate(
    fwbsp_bridge_sStrContext *psStrCtx);

static IMG_RESULT
fwbsp_bridge_StreamResourceDestroy(
    fwbsp_bridge_sStrContext *psStrCtx);

static IMG_RESULT
fwbsp_bridge_StreamCtxAdd(
    fwbsp_bridge_sStrContext *psStrCtx);

static fwbsp_bridge_sStrContext *
fwbsp_bridge_StreamCtxGet(
    fwbsp_bridge_sContext *psCtx,
    IMG_UINT32             ui32StrId);

static IMG_RESULT
fwbsp_bridge_StreamCtxRemove(
    fwbsp_bridge_sStrContext *psStrCtx);

static VDECDD_sStrUnit *
fwbsp_bridge_StreamUnitGet(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    VDECDD_eStrUnitType       eStrUnitType);

static VDECDD_sStrUnit *
fwbsp_bridge_StreamUnitRemove(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    VDECDD_eStrUnitType       eStrUnitType);

static IMG_RESULT
fwbsp_bridge_PictureRegister(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sPictResInt       *psPictResInt,
    IMG_UINT32                ui32ParseId);

static fwbsp_bridge_sPicture *
fwbsp_bridge_PictureGet(
     fwbsp_bridge_sStrContext *psStrCtx,
     IMG_UINT32                ui32Id,
     IMG_BOOL                  bTID);

static IMG_RESULT
fwbsp_bridge_PictureReturn(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32Id,
    IMG_BOOL                  bTID);

static IMG_RESULT
fwbsp_bridge_PictureRemove(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32Id,
    IMG_BOOL                  bTID);

static IMG_RESULT
fwbsp_bridge_PictureSetValid(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId);

static IMG_BOOL
fwbsp_bridge_PictureIsValid(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId);

static IMG_RESULT
fwbsp_bridge_ProcessEncryptedBitstream(
    fwbsp_bridge_sContext    *psCtx,
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit);

static IMG_RESULT
fwbsp_bridge_ProcessStop(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit);

static IMG_RESULT
fwbsp_bridge_ProcessFence(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit);

static IMG_RESULT
fwbsp_bridge_ProcessUnsupported(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit);

static IMG_RESULT
fwbsp_bridge_ParseNewSeqMsg(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32               *pui32Msg);

static IMG_RESULT
fwbsp_bridge_ParseNewPicMsg(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32               *pui32Msg);

static IMG_RESULT
fwbsp_bridge_SignalBufEmpty(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    IMG_BOOL                 *pbRemovePic);

static IMG_RESULT
fwbsp_bridge_ParseErrMsg(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32               *pui32Msg);

static IMG_RESULT
fwbsp_bridge_SignalNewSequence(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT8                 ui8SeqId);

static IMG_RESULT
fwbsp_bridge_SignalNewPicture(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId);

static IMG_RESULT
fwbsp_bridge_SignalStreamStopped(
    fwbsp_bridge_sStrContext *psStrCtx);

static IMG_RESULT
fwbsp_bridge_SignalUnsupportedFeature(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    IMG_UINT32                ui32ErrorFlags);

static IMG_RESULT
fwbsp_bridge_SignalFwError(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    IMG_UINT32                ui32ErrorFlags);

static IMG_VOID
fwbsp_bridge_PopulateSeqHdr(
    VDEC_eVidStd        eVidStd,
    const FWBSP_BRIDGE_sHostMsgData *psHostMsgData,
    VDEC_sComSequHdrInfo            *psComSequHdrInfo);

static IMG_RESULT
fwbsp_bridge_TranslatePIDTotTID(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit);

static IMG_RESULT
fwbsp_bridge_TranslateTIDToBufAddr(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit,
    DECODER_sDecPict         *psDecPict);

static IMG_UINT32
fwbsp_bridge_GetFwCtxBufSize(
    VDEC_eVidStd eVidStd);

static IMG_UINT32
fwbsp_bridge_GetSPSBufSize(
    VDEC_eVidStd eVidStd);

static IMG_UINT32
fwbsp_bridge_GetPPSBufSize(
    VDEC_eVidStd eVidStd);

static IMG_UINT32
fwbsp_bridge_GetVPSBufSize(
    VDEC_eVidStd eVidStd);

static IMG_UINT32
fwbsp_bridge_GetTransId(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId);

static IMG_UINT32
fwbsp_bridge_GetParseId(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32TransId);

static const IMG_CHAR *
fwbsp_bridge_GetMsgName(
    IMG_UINT8 ui8MsgType);

static const IMG_CHAR *
fwbsp_bridge_GetErrDesc(
    VDEC_eVidStd eVidStd,
    IMG_UINT32   ui32Err);

static const IMG_CHAR *
fwbsp_bridge_GetErrDesc_H264(
    IMG_UINT32 ui32Err);

static const IMG_CHAR *
fwbsp_bridge_GetErrDesc_HEVC(
    IMG_UINT32 ui32Err);


/* H264 stuff - move it later to separate file */
/* Remap H.264 colour format into internal representation. */
static const PIXEL_FormatIdc aeH264PixelFormatIdc[] =
{
    PIXEL_FORMAT_MONO,
    PIXEL_FORMAT_420,
    PIXEL_FORMAT_422,
    PIXEL_FORMAT_444
};

/* source: ITU-T H.264 2010/03, page 20 Table 6-1 */
const IMG_INT32 ai32H264subHeightC[] =
{
    -1, 2, 1, 1
};

/* source: ITU-T H.264 2010/03, page 20 Table 6-1 */
const IMG_INT32 ai32H264subWidthC[] =
{
    -1, 2, 2, 1
};


const IMG_INT32 ai32HEVCsubHeightC[] =
{
    1, 2, 1, 1
};

const IMG_INT32 ai32HEVCsubWidthC[] =
{
    1, 2, 2, 1
};


/* FWBSP_BRIDGE internal functions - implementation                          */
/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamStop

******************************************************************************/
IMG_RESULT
fwbsp_bridge_StreamStop(
    fwbsp_bridge_sStrContext *psStrCtx
)
{
    /* Check input params. */
    IMG_ASSERT(psStrCtx);
    if(psStrCtx == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if (FWBSP_BRIDGE_IsStreamIdle((IMG_HANDLE)psStrCtx))
    {
        fwbsp_bridge_SignalStreamStopped(psStrCtx);
    }
    else
    {
        psStrCtx->eStrState = FWBSP_BRIDGE_STRSTATE_STOPPING;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamResourceCreate

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_StreamResourceCreate(
    fwbsp_bridge_sStrContext *psStrCtx
)
{
    fwbsp_bridge_sContext *psCtx;
    IMG_RESULT ui32Result, ui32Size;
    VXDIO_sMemPool sMemPool;
    VDEC_eVidStd eVidStd;

    IMG_ASSERT(psStrCtx->psCtx);

    psCtx = psStrCtx->psCtx;
    eVidStd = psStrCtx->sConfig.eVidStd;

    // always secure
    sMemPool.eMemPoolId = psStrCtx->eSecurePool;
    sMemPool.eMemPoolType = VXDIO_MEMPOOL_SECURE;

    ui32Result = IDGEN_CreateContext(0xFFFF,
                                     0x100,
                                     IMG_TRUE,   /* We need incrementing Ids */
                                     &psStrCtx->hParseIdGen);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto err_out_of_memory;
    }

    /* Allocate SPS, PSP, VPS buffers */
    /* Allocate buffer for SPS */
    ui32Size = fwbsp_bridge_GetSPSBufSize(eVidStd);
    if (ui32Size)
    {
        ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                      MMU_HEAP_STREAM_BUFFERS,
                                      sMemPool, //Secure if Secure allocation is possible
                                      ui32Size,
                                      DEV_MMU_PAGE_ALIGNMENT,
                                      &psStrCtx->sSPSBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto err_out_of_memory;
        }
    }

    /* Allocate buffer for PSP */
    ui32Size = fwbsp_bridge_GetPPSBufSize(eVidStd);
    if (ui32Size)
    {
        ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                      MMU_HEAP_STREAM_BUFFERS,
                                      sMemPool, //Secure if Secure allocation is possible
                                      ui32Size,
                                      DEV_MMU_PAGE_ALIGNMENT,
                                      &psStrCtx->sPSPBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto err_out_of_memory;
        }
    }

    if(eVidStd == VDEC_STD_HEVC)
    {
        /* Allocate buffer for VSP */
        ui32Size = fwbsp_bridge_GetVPSBufSize(eVidStd);
        if (ui32Size)
        {
            ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                            MMU_HEAP_STREAM_BUFFERS,
                                            sMemPool, //Secure if Secure allocation is possible
                                            ui32Size,
                                            DEV_MMU_PAGE_ALIGNMENT,
                                            &psStrCtx->sVPSBufInfo);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto err_out_of_memory;
            }
        }
    }

    /* Allocate the ring buffer */
    ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                  MMU_HEAP_STREAM_BUFFERS,
                                  sMemPool,
                                  FWBSP_BRIDGE_RING_BUF_SIZE,
                                  DEV_MMU_PAGE_ALIGNMENT,
                                  &psStrCtx->sRingBufInfo);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto err_out_of_memory;
    }

    /* Allocate the F/W context buffers */
    {
        VDECDD_sPictResInt *psPictResInt;
        IMG_UINT8 ui8i;

        for (ui8i = 0; ui8i < FWBSP_BRIDGE_MAX_NUM_RES; ui8i++)
        {
            /* Allocate the picture resources structure. */
            VDEC_MALLOC(psPictResInt);
            IMG_ASSERT(IMG_NULL != psPictResInt);
            if(IMG_NULL == psPictResInt)
            {
                goto err_out_of_memory;
            }
            VDEC_BZERO(psPictResInt);

            /* Allocate the firmware context buffer to contain
             data required for subsequent picture. */
            ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                        MMU_HEAP_STREAM_BUFFERS,
                                        sMemPool,
                                        fwbsp_bridge_GetFwCtxBufSize(eVidStd),
                                        DEV_MMU_PAGE_ALIGNMENT,
                                        &psPictResInt->sFwCtxBuf);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
              REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                     "[USERSID=0x%08X] Failed to create a resource (sFwCtxBuf)",
                     psStrCtx->sConfig.ui32UserStrId);
              goto err_out_of_memory;
            }

#if 0
              // xxx-todo: move memset to fw side as this is secure mem and can't be accessed from ROS
              /* Clear the context data in preparation for first time use by the F/W */
              /*IMG_MEMSET(psPictResInt->sFwCtxBuf.pvCpuVirt, 0,
                psPictResInt->sFwCtxBuf.ui32BufSize);
              UPDATE_DEVICE(&psPictResInt->sFwCtxBuf);*/
#endif

            /* HEVC specific */
            if (psStrCtx->sConfig.eVidStd == VDEC_STD_HEVC)
            {
                ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                                MMU_HEAP_STREAM_BUFFERS,
                                                sMemPool,
                                                MEM_TO_REG_BUF_SIZE,
                                                DEV_MMU_PAGE_ALIGNMENT,
                                                &psPictResInt->sMem2RegInfo);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                  REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                         "[USERSID=0x%08X] Failed to create a resource (sMem2RegInfo)",
                         psStrCtx->sConfig.ui32UserStrId);
                  goto err_out_of_memory;
                }

                ui32Result = MMU_StreamMalloc(psStrCtx->hMmuStrHandle,
                                                MMU_HEAP_STREAM_BUFFERS,
                                                sMemPool,
                                                SLICE_PARAMS_BUF_SIZE,
                                                DEV_MMU_PAGE_ALIGNMENT,
                                                &psPictResInt->sSliceParamsInfo);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                  REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                         "[USERSID=0x%08X] Failed to create a resource (sSliceParamsInfo)",
                         psStrCtx->sConfig.ui32UserStrId);
                  goto err_out_of_memory;
                }
            }

              /* Push the resource out of fwbsp_bridge */
              ui32Result = psCtx->pfnCompCallback(
                  FWBSP_BRIDGE_CB_BITSTREAM_RESOURCE_ADD,
                  psCtx->pvCompInitUserData,
                  IMG_NULL,
                  psStrCtx->pvUserData,
                  psPictResInt);
              IMG_ASSERT(IMG_SUCCESS == ui32Result);

              LST_add(&psStrCtx->sPictResList, psPictResInt);
        }
    }

    return IMG_SUCCESS;

err_out_of_memory:
    REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
           "[USERSID=0x%08X] Failed to allocate device memory for stream resources",
           psStrCtx->sConfig.ui32UserStrId);
    fwbsp_bridge_StreamResourceDestroy(psStrCtx);

    return IMG_ERROR_OUT_OF_MEMORY;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamResourceDestroy

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_StreamResourceDestroy(
    fwbsp_bridge_sStrContext *psStrCtx
)
{
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx);

    if (psStrCtx->hParseIdGen)
    {
        IDGEN_DestroyContext(psStrCtx->hParseIdGen);
    }

    /* Free SPS, PSP, VPS buffers */
    /* Free SPS buffer */
    if (psStrCtx->sSPSBufInfo.hMemory)
    {
        ui32Result = MMU_FreeMem(&psStrCtx->sSPSBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psStrCtx->sSPSBufInfo.hMemory = IMG_NULL;
    }

    /* Free PPS buffer */
    if (psStrCtx->sPSPBufInfo.hMemory)
    {
        ui32Result = MMU_FreeMem(&psStrCtx->sPSPBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psStrCtx->sPSPBufInfo.hMemory = IMG_NULL;
    }

    /* Free VPS buffer - TODO (HEVC only) */
    if (psStrCtx->sVPSBufInfo.hMemory)
    {
        ui32Result = MMU_FreeMem(&psStrCtx->sVPSBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psStrCtx->sVPSBufInfo.hMemory = IMG_NULL;
    }

    /* Free picture resources */
    {
        VDECDD_sPictResInt *psPicResInt;
        psPicResInt= (VDECDD_sPictResInt *)LST_removeHead(&psStrCtx->sPictResList);

        while (psPicResInt)
        {
            if (psPicResInt->sFwCtxBuf.hMemory)
            {
                ui32Result = MMU_FreeMem(&psPicResInt->sFwCtxBuf);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }

                psPicResInt->sFwCtxBuf.hMemory = IMG_NULL;
            }

            if (psPicResInt->sMem2RegInfo.hMemory)
            {
                ui32Result = MMU_FreeMem(&psPicResInt->sMem2RegInfo);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }

                psPicResInt->sMem2RegInfo.hMemory = IMG_NULL;
            }

            if (psPicResInt->sSliceParamsInfo.hMemory)
            {
                ui32Result = MMU_FreeMem(&psPicResInt->sSliceParamsInfo);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }

                psPicResInt->sSliceParamsInfo.hMemory = IMG_NULL;
            }

            IMG_FREE(psPicResInt);
            psPicResInt = IMG_NULL;

            psPicResInt = (VDECDD_sPictResInt *)LST_removeHead(&psStrCtx->sPictResList);
        }
    }

    /* Free ring buffer */
    if (psStrCtx->sRingBufInfo.hMemory)
    {
        ui32Result = MMU_FreeMem(&psStrCtx->sRingBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psStrCtx->sRingBufInfo.hMemory = IMG_NULL;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamCtxAdd

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_StreamCtxAdd(
    fwbsp_bridge_sStrContext *psStrCtx)
{
    fwbsp_bridge_sContext *psCtx;

    IMG_ASSERT(psStrCtx);
    if (psStrCtx == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psCtx = psStrCtx->psCtx;
    IMG_ASSERT(psCtx);
    IMG_ASSERT(psStrCtx->ui32DecStrId);

    LST_add(&psCtx->sStreamList, psStrCtx);
    psCtx->ui8StrCnt++;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamCtxGet

******************************************************************************/
static fwbsp_bridge_sStrContext *
fwbsp_bridge_StreamCtxGet(
    fwbsp_bridge_sContext *psCtx,
    IMG_UINT32             ui32StrId)
{
    IMG_ASSERT(psCtx);

    if (psCtx)
    {
        fwbsp_bridge_sStrContext *psStrContext;
        FWBSP_BRIDGE_FOR_EACH(psStrContext, &psCtx->sStreamList)
        {
            if (psStrContext->ui32DecStrId == ui32StrId)
            {
                return psStrContext;
            }
        }
    }

    return  IMG_NULL;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamCtxRemove

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_StreamCtxRemove(
    fwbsp_bridge_sStrContext *psStrCtx)
{
    fwbsp_bridge_sContext *psCtx;

    IMG_ASSERT(psStrCtx);
    if (psStrCtx == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psCtx = psStrCtx->psCtx;
    IMG_ASSERT(psCtx);
    IMG_ASSERT(psStrCtx->ui32DecStrId);

    LST_remove(&psCtx->sStreamList, psStrCtx);
    psCtx->ui8StrCnt--;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamUnitGet

******************************************************************************/
static VDECDD_sStrUnit *
fwbsp_bridge_StreamUnitGet(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    VDECDD_eStrUnitType       eStrUnitType
)
{
    VDECDD_sStrUnit *psStrUnit = IMG_NULL;
    LST_T *psStrUnitList;

    switch (eStrUnitType)
    {
        case VDECDD_STRUNIT_FWBSP_ENC_BSTR:
            psStrUnitList = &psStrCtx->sEncBitstreamStrUnitList;
            break;

        case VDECDD_STRUNIT_FENCE:
            psStrUnitList = &psStrCtx->sFenceStrUnitList;
            break;

        default:
            psStrUnitList = IMG_NULL;
            IMG_ASSERT(IMG_FALSE);
            break;
    }

    psStrUnit = (VDECDD_sStrUnit *)LST_first(psStrUnitList);
    while (psStrUnit)
    {
        if (psStrUnit->ui32ParseId == ui32ParseId &&
            psStrUnit->eStrUnitType == eStrUnitType)
        {
            return psStrUnit;
        }

        psStrUnit = (VDECDD_sStrUnit *)LST_next(psStrUnit);
    }

    return psStrUnit;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_StreamUnitRemove

******************************************************************************/
static VDECDD_sStrUnit *
fwbsp_bridge_StreamUnitRemove(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    VDECDD_eStrUnitType       eStrUnitType
)
{
    VDECDD_sStrUnit *psStrUnit = IMG_NULL;
    LST_T *psStrUnitList;

    switch (eStrUnitType)
    {
        case VDECDD_STRUNIT_FWBSP_ENC_BSTR:
            psStrUnitList = &psStrCtx->sEncBitstreamStrUnitList;
            break;

        case VDECDD_STRUNIT_FENCE:
            psStrUnitList = &psStrCtx->sFenceStrUnitList;
            break;

        default:
            psStrUnitList = IMG_NULL;
            IMG_ASSERT(IMG_FALSE);
            break;
    }

    psStrUnit = fwbsp_bridge_StreamUnitGet(psStrCtx, ui32ParseId, eStrUnitType);
    if (psStrUnit)
    {
        psStrUnit = (VDECDD_sStrUnit *)LST_remove(psStrUnitList,
                                                  psStrUnit);
        IMG_ASSERT(psStrUnit);
    }

    if (psStrCtx->eStrState == FWBSP_BRIDGE_STRSTATE_STOPPING &&
        FWBSP_BRIDGE_IsStreamIdle((IMG_HANDLE)psStrCtx))
    {
        fwbsp_bridge_SignalStreamStopped(psStrCtx);
    }

    return psStrUnit;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_PictureRegister

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_PictureRegister(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sPictResInt       *psPictResInt,
    IMG_UINT32                ui32ParseId
)
{
    fwbsp_bridge_sPicture *psPicture;

    IMG_ASSERT(psStrCtx && psPictResInt);

    /* Get free picture */
    psPicture = fwbsp_bridge_PictureGet(psStrCtx, 0, IMG_FALSE);
    IMG_ASSERT(psPicture);
    if (psPicture)
    {
        /* Register parseId with picture resources in empty slot */
        psPicture->ui32ParseId = ui32ParseId;
        psPicture->ui32TransId = 0;
        psPicture->psPictResInt = psPictResInt;
        return IMG_SUCCESS;
    }

    return IMG_ERROR_STORAGE_TYPE_FULL;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_PictureGet

******************************************************************************/
static fwbsp_bridge_sPicture *
fwbsp_bridge_PictureGet(
     fwbsp_bridge_sStrContext *psStrCtx,
     IMG_UINT32                ui32Id,
     IMG_BOOL                  bTID)
{
    const IMG_UINT32 *pui32IdBase;
    IMG_UINT32 *pui32Id;
    IMG_UINT8 ui8i;

    IMG_ASSERT(psStrCtx);

    if (bTID)
    {
        pui32IdBase = &psStrCtx->asPicture[0].ui32TransId;
    }
    else
    {
        pui32IdBase = &psStrCtx->asPicture[0].ui32ParseId;
    }

    for (ui8i = 0; ui8i < FWBSP_BRIDGE_MAX_NUM_PICTURES; ui8i++)
    {
        pui32Id = (IMG_UINT32 *)((IMG_UINT8 *)pui32IdBase +
            ui8i * sizeof(fwbsp_bridge_sPicture));

        if (*pui32Id == ui32Id)
        {
            return &psStrCtx->asPicture[ui8i];
        }
    }

    return IMG_NULL;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_PictureReturn

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_PictureReturn(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32Id,
    IMG_BOOL                  bTID
)
{
    fwbsp_bridge_sPicture *psPicture;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx && ui32Id);

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32Id, bTID);
    IMG_ASSERT(psPicture);
    if (psPicture)
    {
        ui32Result = RESOURCE_ItemReturn(&psPicture->psPictResInt->ui32RefCount);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if(RESOURCE_ItemIsAvailable(&psPicture->psPictResInt->ui32RefCount))
        {
            psPicture->psPictResInt = IMG_NULL;
        }
        return ui32Result;
    }

    return IMG_ERROR_INVALID_PARAMETERS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_PictureRemove

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_PictureRemove(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32Id,
    IMG_BOOL                  bTID
)
{
    fwbsp_bridge_sFwMsgItem *psFwMsgItem;
    fwbsp_bridge_sPicture *psPicture;
    IMG_RESULT ui32Result;
    IMG_UINT8 ui8Counter;
    LST_T *psFwMsgBufList;

    IMG_ASSERT(psStrCtx);

    psFwMsgBufList = &psStrCtx->psCtx->sFwMsgBufList;

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32Id, bTID);
    IMG_ASSERT(psPicture);
    if (psPicture)
    {
        ui8Counter = 0;
        psFwMsgItem = (fwbsp_bridge_sFwMsgItem *)LST_first(psFwMsgBufList);
        while(psFwMsgItem)
        {
            fwbsp_bridge_sFwMsgItem *psFwMsgItemTmp = IMG_NULL;
            if (psFwMsgItem->ui32ParseId == psPicture->ui32ParseId)
            {
                IMG_ASSERT(psFwMsgItem->bProcess == IMG_FALSE);
                psFwMsgItemTmp = (fwbsp_bridge_sFwMsgItem *)
                    LST_remove(psFwMsgBufList, psFwMsgItem);
                ui8Counter++;
            }

            psFwMsgItem = (fwbsp_bridge_sFwMsgItem *)LST_next(psFwMsgItem);

            if (psFwMsgItemTmp)
            {
                IMG_FREE(psFwMsgItemTmp);
                psFwMsgItemTmp = IMG_NULL;
            }

            if (ui8Counter == FWBSP_BRIDGE_MAX_FW_MSG_PER_BITSTREAM)
            {
                break;
            }
        }

        /* Remove Id */
        ui32Result = IDGEN_FreeId(psStrCtx->hParseIdGen,
                                 FWBSP_BRIDGE_GET_PICTURE_ID(psPicture->ui32ParseId));
        IMG_ASSERT(ui32Result == IMG_SUCCESS);

        psPicture->ui32TransId = 0;
        psPicture->ui32ParseId = 0;
        psPicture->bValid = IMG_FALSE;

        return ui32Result;
    }

    return IMG_ERROR_INVALID_PARAMETERS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_PictureSetValid

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_PictureSetValid(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId
)
{
    fwbsp_bridge_sPicture *psPicture;

    IMG_ASSERT(psStrCtx);

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32ParseId, IMG_FALSE);
    IMG_ASSERT(psPicture);
    if (!psPicture)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if (psPicture->bValid)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
               "Received more than one FWBSP_NEW_PIC msg for PID=0x%08X",
               psPicture->ui32ParseId);
        return IMG_ERROR_FATAL;
    }

    psPicture->bValid = IMG_TRUE;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_PictureIsValid

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_PictureIsValid(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId
)
{
    fwbsp_bridge_sPicture *psPicture;

    IMG_ASSERT(psStrCtx);

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32ParseId, IMG_FALSE);
    IMG_ASSERT(psPicture);
    if (!psPicture)
    {
        return IMG_FALSE;
    }

    return psPicture->bValid;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ProcessEncryptedBitstream

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ProcessEncryptedBitstream(
    fwbsp_bridge_sContext    *psCtx,
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit)
{
    IMG_RESULT ui32Result;
    FWBSP_BRIDGE_sFwMsgData *psFwMsgData = &psStrCtx->sFwMsgData;
    VDECDD_sPictResInt *psPictResInt;

    IMG_ASSERT(psStrCtx->eStrState == FWBSP_BRIDGE_STRSTATE_PLAYING);

    LST_add(&psStrCtx->sEncBitstreamStrUnitList, psStrUnit);
    psStrUnit->bDecode = IMG_FALSE;

    /* Fill message data */
    psPictResInt = (VDECDD_sPictResInt *)psStrUnit->pvDdPictData;
    IMG_ASSERT(psPictResInt && psPictResInt->sFwCtxBuf.ui32DevVirt);

    psFwMsgData = &psStrCtx->sFwMsgData;

    psFwMsgData->bSizeDelimMode =
        psStrCtx->sConfig.eBstrFormat == VDEC_BSTRFORMAT_DEMUX_SIZEDELIMITED;
    psFwMsgData->bFirstGOP = psStrCtx->ui32GOPCnt == 0 ? 1 : 0;
    psFwMsgData->bCodecCfg =
        psStrUnit->eBstrElementType == VDEC_BSTRELEMENT_CODEC_CONFIG;
    psFwMsgData->ui8VidStd = (IMG_UINT8)psStrCtx->sConfig.eVidStd;
    psFwMsgData->ui32PtdAddr = psStrCtx->ui32PtdDevVirt;
    psFwMsgData->ui32SPSAddr = psStrCtx->sSPSBufInfo.ui32DevVirt;
    psFwMsgData->ui32SPSSize = psStrCtx->sSPSBufInfo.ui32BufSize;
    psFwMsgData->ui32PPSAddr = psStrCtx->sPSPBufInfo.ui32DevVirt;
    psFwMsgData->ui32PPSSize = psStrCtx->sPSPBufInfo.ui32BufSize;
    psFwMsgData->ui32RingBufAddr = psStrCtx->sRingBufInfo.ui32DevVirt;
    psFwMsgData->ui32RingBufSize = FWBSP_BRIDGE_RING_BUF_SIZE;
    psFwMsgData->ui32RingBufMark = psStrCtx->ui32LastRBMark;
    psFwMsgData->ui8CodecMode = VXD_GetCodedMode(psStrCtx->sConfig.eVidStd);

    if (psStrCtx->psCurPictResInt)
    {
        psStrCtx->psPrevPictResInt = psStrCtx->psCurPictResInt;
        psStrCtx->psCurPictResInt = psPictResInt;
    }
    else
    {
        psStrCtx->psCurPictResInt = psPictResInt;
        psStrCtx->psPrevPictResInt = IMG_NULL;
    }

    if (psStrCtx->psPrevPictResInt)
    {
        psFwMsgData->ui32FwCtxLoadAddr =
            psStrCtx->psPrevPictResInt->sFwCtxBuf.ui32DevVirt;
    }
    else
    {
        psFwMsgData->ui32FwCtxLoadAddr = 0;
    }

    psFwMsgData->ui32FwCtxSaveAddr =
        psStrCtx->psCurPictResInt->sFwCtxBuf.ui32DevVirt;
    psFwMsgData->ui32FwCtxSize =
        psStrCtx->psCurPictResInt->sFwCtxBuf.ui32BufSize;

    /* HEVC specific */
    if (psStrCtx->sConfig.eVidStd == VDEC_STD_HEVC)
    {
        psFwMsgData->ui32VPSAddr = psStrCtx->sVPSBufInfo.ui32DevVirt;
        psFwMsgData->ui32VPSSize = psStrCtx->sVPSBufInfo.ui32BufSize;
        psFwMsgData->ui32Mem2RegAddr =
            psStrCtx->psCurPictResInt->sMem2RegInfo.ui32DevVirt;
        psFwMsgData->ui32Mem2RegSize =
            psStrCtx->psCurPictResInt->sMem2RegInfo.ui32BufSize;
        psFwMsgData->ui32SliceParamsAddr =
            psStrCtx->psCurPictResInt->sSliceParamsInfo.ui32DevVirt;
        psFwMsgData->ui32SliceParamsSize =
            psStrCtx->psCurPictResInt->sSliceParamsInfo.ui32BufSize;
    }

    /* Get encrypted bitstreams addresses */
    {
        BSPP_sBitStrSeg *psBitStrSeg;
        VDECDD_sDdBufMapInfo *psDdBufMapInfo;
        IMG_UINT32 ui32PictureId, ui32ParseId;

        /* Allocate a unique id to the picture. */
        ui32Result = IDGEN_AllocId(psStrCtx->hParseIdGen,
                                   psStrUnit,
                                   &ui32PictureId);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        ui32ParseId = FWBSP_BRIDGE_CREATE_PARSE_ID(psStrCtx->ui32DecStrId,
                                                   ui32PictureId);
        psStrUnit->ui32ParseId = ui32ParseId;

        ui32Result = fwbsp_bridge_PictureRegister(psStrCtx,
                                                  psPictResInt,
                                                  ui32ParseId);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);

        psFwMsgData->ui8BitstreamQty = 0;
        psBitStrSeg = (BSPP_sBitStrSeg *)LST_first(&psStrUnit->sBitStrSegList);
        IMG_ASSERT(psBitStrSeg);
        while(psBitStrSeg)
        {
            if (psFwMsgData->ui8BitstreamQty >=
                FWBSP_BRIDGE_MAX_BITSTREAM_SEGMENTS)
            {
                /* Too many bstr sgmts signal unsupported feature */
                ui32Result = fwbsp_bridge_SignalUnsupportedFeature(psStrCtx,
                    ui32ParseId, FWBSP_BRIDGE_TOO_MANY_BSTR);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                return ui32Result;
            }

            /* Get access to map info context...*/
            ui32Result = RMAN_GetResource(psBitStrSeg->ui32BufMapId,
                                          VDECDD_BUFMAP_TYPE_ID,
                                          (IMG_VOID **)&psDdBufMapInfo,
                                          IMG_NULL);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            psFwMsgData->aui32BitstreamAddr[psFwMsgData->ui8BitstreamQty] =
                psDdBufMapInfo->sDdBufInfo.ui32DevVirt;
            psFwMsgData->aui32BitstreamSize[psFwMsgData->ui8BitstreamQty] =
                psBitStrSeg->ui32DataSize;

            psBitStrSeg = (BSPP_sBitStrSeg *)LST_next(psBitStrSeg);
            psFwMsgData->ui8BitstreamQty++;

            UPDATE_DEVICE(&psDdBufMapInfo->sDdBufInfo);
        }

        psFwMsgData->ui32ParseId = ui32ParseId;
        psStrCtx->ui32LastParseId = ui32ParseId;
    }

    /* Allocate buffers for incoming F/W messages */
    {
        IMG_UINT8 ui8i;
        for (ui8i = 0; ui8i < FWBSP_BRIDGE_MAX_FW_MSG_PER_BITSTREAM; ui8i++)
        {
            fwbsp_bridge_sFwMsgItem *psFwMsgItem;

            VDEC_MALLOC(psFwMsgItem);
            IMG_ASSERT(psFwMsgItem);
            if (psFwMsgItem == IMG_NULL)
            {
                REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                    "Failed to allocate memory for FWBSP_BRIDGE F/W message");
                return IMG_ERROR_OUT_OF_MEMORY;
            }
            VDEC_BZERO(psFwMsgItem);

            psFwMsgItem->ui32ParseId = psStrUnit->ui32ParseId;

            LST_add(&psStrCtx->psCtx->sFwMsgBufList, psFwMsgItem);
        }
    }

    /* Submit encrypted bitstream */
    {
        IMG_HANDLE hHwCtx;
        hHwCtx = DECODER_GethHwCtxHandle(psCtx->hDecCtx);
        IMG_ASSERT(hHwCtx);
        if(!hHwCtx)
        {
            return IMG_ERROR_INVALID_PARAMETERS;
        }

        ui32Result = HWCTRL_BistreamSubmit(hHwCtx,
                                           (IMG_HANDLE)psStrCtx,
                                           psFwMsgData,
                                           &psStrCtx->sHostMsgData);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result == IMG_SUCCESS)
        {
            psStrUnit->bDecode = IMG_TRUE;
        }
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ProcessStop

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ProcessStop(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit)
{
    IMG_RESULT ui32Result;

    fwbsp_bridge_StreamStop(psStrCtx);

    ui32Result = VDECDDUTILS_FreeStrUnit(psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ProcessFence

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ProcessFence(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit)
{
    VDECDD_sStrUnit * psEncBstrUnit;

    IMG_ASSERT(psStrCtx->ui32LastParseId);

    /* psStrUnit->ui32ParseId is used outside of fwbsp_bridge to check whether
     * fence stream unit was already processed by fwbsp_brdige. It is also
     * used internally to submit fence to core atfer appropriate picture. */
    psStrUnit->ui32ParseId = psStrCtx->ui32LastParseId;

    /* Handle case in which pictures derived from bitstream buffer
     * corresponding to psStrCtx->ui32LastParseId were already submitted to
     * core and all related stream units were removed from internal lists. */
    psEncBstrUnit = fwbsp_bridge_StreamUnitGet(psStrCtx,
                                               psStrCtx->ui32LastParseId,
                                               VDECDD_STRUNIT_FWBSP_ENC_BSTR);
    if (psEncBstrUnit == IMG_NULL)
    {
        /* Encrypted buffer corresponding to psStrCtx->ui32LastParseId was
         * already processed and removed from internal lists. The event which
         * would trigger sending this fence unit will never occur, so do it
         * now. */
        IMG_RESULT ui32Result;

        ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
    else
    {
        LST_add(&psStrCtx->sFenceStrUnitList, psStrUnit);
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ProcessUnsupported

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ProcessUnsupported(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit)
{
    IMG_RESULT ui32Result;

    ui32Result = VDECDDUTILS_FreeStrUnit(psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ParseNewSeqMsg

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ParseNewSeqMsg(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32               *pui32Msg
)
{
    FWBSP_BRIDGE_sHostMsgData *psHostMsgData = &psStrCtx->sHostMsgData;
    IMG_UINT16 ui16MsgId;

    ui16MsgId = MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_MSG_ID);

    psHostMsgData->ui16MsgId = ui16MsgId;
    psHostMsgData->ui8ProfileIdc =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_PROFILE_IDC);
    psHostMsgData->ui8LevelIdc =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_LEVEL_IDC);
    psHostMsgData->ui8BitDepthLuma =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_BIT_DEPTH_LUMA);
    psHostMsgData->ui8BitDepthChroma =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_BIT_DEPTH_CHROMA);
    psHostMsgData->ui8ChromaFormatIdc =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_CHROMA_FORMAT_IDC);
    psHostMsgData->bFrameMbsOnly =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_FRAME_MBS_ONLY);
    psHostMsgData->bSepColPlane =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_SEP_COL_PLANE);
    psHostMsgData->bFrameCropping =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_FRAME_CROPPING);
    psHostMsgData->bAdaptiveFrameField =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_ADAPTIVE_FRAME_FIELD);
    psHostMsgData->bClosedGOP =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_CLOSED_GOP);
    psHostMsgData->bNotDpbFlush=
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_NOT_DPB_FLUSH);
    psHostMsgData->bVUIParamsPresent =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_PARAMS_PRESENT);
    psHostMsgData->ui16PicWidth =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_PIC_WIDTH);
    psHostMsgData->ui16PicHeight =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_PIC_HEIGHT);
    psHostMsgData->ui16MaxPicWidth =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_MAX_PIC_WIDTH);
    psHostMsgData->ui16MaxPicHeight =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_MAX_PIC_HEIGHT);
    psHostMsgData->ui16FrameCropLeft =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_FRAME_CROP_LEFT);
    psHostMsgData->ui16FrameCropRight =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_FRAME_CROP_RIGHT);
    psHostMsgData->ui16FrameCropTop =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_FRAME_CROP_TOP);
    psHostMsgData->ui16FrameCropBottom =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_FRAME_CROP_BOTTOM);
    psHostMsgData->ui8SeqHdrId =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_SEQ_HDR_ID);

    /* VUI params */
    if (psHostMsgData->bVUIParamsPresent)
    {
        psHostMsgData->bColorSpacePresent =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_COLOR_SPACE_PRESENT);
        psHostMsgData->ui8ColorPrimaries =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_COLOR_PRIMARIES);
        psHostMsgData->ui8TransferCharacteristic =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_TRANS_CHARACTER);
        psHostMsgData->ui8MartixCoeffs =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_MATRIX_COEFFS);
        psHostMsgData->bTimingInfo =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_TIMING_INFO);
        psHostMsgData->ui32TimeScale =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_TIME_SCALE);
        psHostMsgData->ui32NumUnitsInTick =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_NUM_UNITS_IN_TICK);
        psHostMsgData->ui32MaxDecFrameBuff =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_MAX_DEC_FRAME_BUFF);
        psHostMsgData->ui16SarWidth =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_SAR_WIDTH);
        psHostMsgData->ui16SarHeight =
            MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_VUI_SAR_HEIGHT);
    }
    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ParseNewPicMsg

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ParseNewPicMsg(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32               *pui32Msg
)
{
    FWBSP_BRIDGE_sHostMsgData *psHostMsgData;
    IMG_UINT16 ui16MsgId, ui16MsgSize;

    IMG_ASSERT(psStrCtx);

    psHostMsgData = &psStrCtx->sHostMsgData;

    ui16MsgSize = MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_MSG_SIZE);
    ui16MsgId = MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_MSG_ID);
    psHostMsgData->ui16MsgId = ui16MsgId;
    psHostMsgData->ui8PicToDispQty =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_PIC_TO_DISP_QTY);
    psHostMsgData->ui8PicToRelQty =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_PIC_TO_REL_QTY);
    psHostMsgData->ui8PicType =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_PIC_TYPE);
    psHostMsgData->bFieldCoded =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_FIELD_CODED);
    psHostMsgData->b2ndFieldOfPair =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_FIELD_OF_PAIR);
    psHostMsgData->bTopFieldFirst =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_TOP_FIELD_FIRST);
    psHostMsgData->bClosedGOP =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_CLOSED_GOP);
    psHostMsgData->ui32ParseId =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_PARSE_ID);
    psHostMsgData->ui16RefPicMask =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_REF_PIC_MASK);
    psHostMsgData->bRepeatFirstField =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_REPEAT_FIRST_FIELD);
    psHostMsgData->ui8MaxFrmRepeat =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_MAX_FRM_REPEAT);
    psHostMsgData->bBackwardRefs =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_BACKWARD_REFS);
    psHostMsgData->ui8ColPicQty =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_COL_PICS_NUM);
    psHostMsgData->sFWBSPDataPids.sDPBState.ui8DPBDisPicLen =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_DPB_NUM_DIS_PICS);
    psHostMsgData->sFWBSPDataPids.sDPBState.ui32DPBFieldCodedMask =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_DPB_FIELD_CODED_BITMASK);
    psHostMsgData->sFWBSPDataPids.sDPBState.ui32DPBSingleFieldMask =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_DPB_SINGLE_FIELD_BITMASK);
    psHostMsgData->sFWBSPDataPids.sDPBState.ui32DPBBotFieldMask =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_DPB_BOTTOM_FIELD_BITMASK);

    psHostMsgData->sFWBSPDataPids.ui32MergePid =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_MERGE_PID);
    psHostMsgData->ui32RingBufMark =
        MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_PIC_RB_MARK);

    /* Do we have additional data? */
    if (ui16MsgSize > FWBSP_NEW_PIC_SIZE / sizeof(IMG_UINT32))
    {
#define MEMIO_READ_DWORD(vpMem, offset)                                        \
(((*((IMG_UINT32 *)(((IMG_UINTPTR)(vpMem)) + offset))) & 0xFFFFFFFF))
        IMG_UINT32 ui32Tmp;
        IMG_UINT16 ui16Offset, ui16ToRead =
            ui16MsgSize -(FWBSP_NEW_PIC_SIZE / sizeof(IMG_UINT32));
        IMG_UINT8 ui8i;

        /* Get ui8RefPicQty */
        psHostMsgData->ui8RefPicQty = 0;
        if (psHostMsgData->ui16RefPicMask)
        {
            IMG_UINT16 ui16Tmp = psHostMsgData->ui16RefPicMask;

            /* Count bits in ui16RefPicMask */
            for (; ui16Tmp; psHostMsgData->ui8RefPicQty++)
            {
                ui16Tmp &= ui16Tmp - 1;
            }
        }

        /* Validate additional message size */
        IMG_ASSERT(ui16ToRead == (2 * psHostMsgData->ui8PicToDispQty
            + psHostMsgData->ui8PicToRelQty + psHostMsgData->ui8RefPicQty
            + psHostMsgData->ui8ColPicQty
            + psHostMsgData->sFWBSPDataPids.sDPBState.ui8DPBDisPicLen));

        ui16Offset = FWBSP_NEW_PIC_SIZE;

        /* Read M x pictures to display (M x 32)
           where M = psHostMsgData->ui8PicToDispQty */
        IMG_ASSERT(psHostMsgData->ui8PicToDispQty <
            sizeof(psHostMsgData->aui32DisplayList) /
            sizeof(psHostMsgData->aui32DisplayList[0]));

        for (ui8i = 0; ui8i < psHostMsgData->ui8PicToDispQty; ui8i++)
        {
            IMG_ASSERT(ui16Offset <= ui16MsgSize * sizeof(IMG_UINT32));
            ui32Tmp = MEMIO_READ_DWORD(pui32Msg, ui16Offset);
            IMG_ASSERT(ui32Tmp);
            psHostMsgData->aui32DisplayList[ui8i] = ui32Tmp;
            ui16Offset += sizeof(IMG_UINT32);
        }

        /* Read M x pictures to display flags (M x 32)
           where M = psHostMsgData->ui8PicToDispQty */
        IMG_ASSERT(psHostMsgData->ui8PicToDispQty <
            sizeof(psHostMsgData->aui8DisplayFlags) /
            sizeof(psHostMsgData->aui8DisplayFlags[0]));

        for (ui8i = 0; ui8i < psHostMsgData->ui8PicToDispQty; ui8i++)
        {
            IMG_ASSERT(ui16Offset <= ui16MsgSize * sizeof(IMG_UINT32));
            ui32Tmp = MEMIO_READ_DWORD(pui32Msg, ui16Offset);
            psHostMsgData->aui8DisplayFlags[ui8i] = (IMG_UINT8)ui32Tmp;
            ui16Offset += sizeof(IMG_UINT32);
        }

        /* Read N x pictures to release (N x 32)
           where N = psHostMsgData->ui8PicToRelQty */
        IMG_ASSERT(psHostMsgData->ui8PicToRelQty <
            sizeof(psHostMsgData->aui32ReleaseList) /
            sizeof(psHostMsgData->aui32ReleaseList[0]));

        for (ui8i = 0; ui8i < psHostMsgData->ui8PicToRelQty; ui8i++)
        {
            IMG_ASSERT(ui16Offset <= ui16MsgSize * sizeof(IMG_UINT32));
            ui32Tmp = MEMIO_READ_DWORD(pui32Msg, ui16Offset);
            /* That may happen for some strange argon streams */
            IMG_ASSERT(ui32Tmp || psStrCtx->sConfig.eVidStd == VDEC_STD_HEVC);
            psHostMsgData->aui32ReleaseList[ui8i] = ui32Tmp;
            ui16Offset += sizeof(IMG_UINT32);
        }

        /* Read O x ref. pictures (O x 32)
           where O = psHostMsgData->ui16RefPicMask */
        if (psHostMsgData->ui8RefPicQty)
        {
            IMG_ASSERT(psHostMsgData->ui8RefPicQty <
                sizeof(psHostMsgData->aui32RefPicsList) /
                sizeof(psHostMsgData->aui32RefPicsList[0]));

            for (ui8i = 0; ui8i < psHostMsgData->ui8RefPicQty; ui8i++)
            {
                IMG_ASSERT(ui16Offset <= ui16MsgSize * sizeof(IMG_UINT32));
                ui32Tmp = MEMIO_READ_DWORD(pui32Msg, ui16Offset);
                IMG_ASSERT(ui32Tmp);
                psHostMsgData->aui32RefPicsList[ui8i] = ui32Tmp;
                ui16Offset += sizeof(IMG_UINT32);
            }
        }

        /* Read colocated picture buffers */
        if (psHostMsgData->ui8ColPicQty)
        {
            IMG_ASSERT(psHostMsgData->ui8ColPicQty <=
            sizeof(psHostMsgData->aui32ColPicPids) /
            sizeof(psHostMsgData->aui32ColPicPids[0]));

            for (ui8i = 0; ui8i < psHostMsgData->ui8ColPicQty; ui8i++)
            {
                IMG_ASSERT(ui16Offset <= ui16MsgSize * sizeof(IMG_UINT32));
                ui32Tmp = MEMIO_READ_DWORD(pui32Msg, ui16Offset);
                psHostMsgData->aui32ColPicPids[ui8i] = ui32Tmp;
                ui16Offset += sizeof(IMG_UINT32);
            }
        }

        /* Read DPB dump */
        if (psHostMsgData->sFWBSPDataPids.sDPBState.ui8DPBDisPicLen)
        {
            IMG_UINT8 ui8DPBDisPicLen =
                psHostMsgData->sFWBSPDataPids.sDPBState.ui8DPBDisPicLen;
            IMG_ASSERT(ui8DPBDisPicLen <=
            sizeof(psHostMsgData->sFWBSPDataPids.sDPBState.aui32DPBDisList) /
            sizeof(psHostMsgData->sFWBSPDataPids.sDPBState.aui32DPBDisList[0]));

            for (ui8i = 0; ui8i < ui8DPBDisPicLen; ui8i++)
            {
                IMG_ASSERT(ui16Offset <= ui16MsgSize * sizeof(IMG_UINT32));
                ui32Tmp = MEMIO_READ_DWORD(pui32Msg, ui16Offset);
                psHostMsgData->sFWBSPDataPids.sDPBState.aui32DPBDisList[ui8i] =
                    ui32Tmp;
                ui16Offset += sizeof(IMG_UINT32);
            }
        }

#undef MEMIO_READ_DWORD
    }
    else
    {
        /* No additional data */
        IMG_ASSERT(psHostMsgData->ui8PicToDispQty == 0);
        IMG_ASSERT(psHostMsgData->ui8PicToRelQty == 0);
        IMG_ASSERT(psHostMsgData->ui16RefPicMask == 0);
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_SignalBufEmpty

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_SignalBufEmpty(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    IMG_BOOL                 *pbRemovePic
)
{
    VDECDD_sStrUnit *psStrUnit;
    IMG_RESULT ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psStrCtx && pbRemovePic);

    /* Parsing done, decrease reference count */
    ui32Result = fwbsp_bridge_PictureReturn(psStrCtx, ui32ParseId, IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    /* Remove VDECDD_STRUNIT_FWBSP_ENC_BSTR */
    psStrUnit = fwbsp_bridge_StreamUnitRemove(psStrCtx,
                                              ui32ParseId,
                                              VDECDD_STRUNIT_FWBSP_ENC_BSTR);
    if (psStrUnit == IMG_NULL)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[USERSID=0x%08X] Unable to find current stream unit for parse ID: %d",
            psStrCtx->sConfig.ui32UserStrId, ui32ParseId);
        return IMG_ERROR_COULD_NOT_OBTAIN_RESOURCE;
    }

    /* Inform hardware api that bitstream has been handled by the fw side */
    {
        IMG_HANDLE hHwCtx;
        hHwCtx = DECODER_GethHwCtxHandle(psStrCtx->psCtx->hDecCtx);
        IMG_ASSERT(hHwCtx);
        if(!hHwCtx)
        {
            return IMG_ERROR_INVALID_PARAMETERS;
        }

        ui32Result = HWCTRL_BistreamProcessed(hHwCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }

    /* Did we receive FWBSP_NEW_PIC?, if not remove the picture */
    if (!fwbsp_bridge_PictureIsValid(psStrCtx, ui32ParseId))
    {
        /* Trigger sending BSTRBUF_EMPTY event */
        ui32Result = psStrCtx->psCtx->pfnCompCallback(
            FWBSP_BRIDGE_CB_UNIT_PROCESSED,
            psStrCtx->psCtx->pvCompInitUserData,
            IMG_NULL,
            psStrCtx->pvUserData,
            psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);

        /* We have to remove the picture outside the loop */
         *pbRemovePic = IMG_TRUE;
    }
    else
    {
        DEBUG_REPORT(REPORT_MODULE_FWBSP_BRIDGE,
            "[SID=0x%08X] [UTYPE=0x%08X] DESTROYED",
            psStrCtx->ui32StrId,
            psStrUnit->eStrUnitType);

        VDECDDUTILS_FreeStrUnit(psStrUnit);
    }

    /* Pass the fence stream unit to the core if it's present. */
    psStrUnit = fwbsp_bridge_StreamUnitRemove(psStrCtx,
                                              ui32ParseId,
                                              VDECDD_STRUNIT_FENCE);
    if (psStrUnit)
    {
        ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_ParseErrMsg

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_ParseErrMsg(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32               *pui32Msg
)
{
    FWBSP_BRIDGE_sHostMsgData *psHostMsgData;

    IMG_ASSERT(psStrCtx);

    psHostMsgData = &psStrCtx->sHostMsgData;
    psHostMsgData->ui16MsgId = MEMIO_READ_FIELD(pui32Msg, FWBSP_ERROR_MSG_ID);
    psHostMsgData->ui32ParseId = MEMIO_READ_FIELD(pui32Msg, FWBSP_ERROR_PARSE_ID);
    psHostMsgData->ui32ErrorFlags = MEMIO_READ_FIELD(pui32Msg, FWBSP_ERROR_ERROR_FLAGS);

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_SignalNewSequence

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_SignalNewSequence(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT8                 ui8SeqHdrId
)
{
    FWBSP_BRIDGE_sHostMsgData *psHostMsgData;
    VDEC_sComSequHdrInfo *psComSequHdrInfo;
    VDECDD_sStrUnit *psStrUnit;
    IMG_UINT32 ui32Result;

    IMG_ASSERT(psStrCtx);

    psHostMsgData = &psStrCtx->sHostMsgData;
    IMG_ASSERT(psHostMsgData->ui8SeqHdrId == ui8SeqHdrId);

    /* Allocate a stream unit */
    ui32Result = VDECDDUTILS_CreateStrUnit(&psStrUnit, IMG_NULL);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[SID=0x%08X] Failed to allocate memory for stream unit",
            psStrCtx->ui32StrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    psStrUnit->eStrUnitType = VDECDD_STRUNIT_SEQUENCE_START;

    /* Allocate a sequence header.*/
    VDEC_MALLOC(psStrUnit->psSequHdrInfo);
    IMG_ASSERT(psStrUnit->psSequHdrInfo);
    if (!psStrUnit->psSequHdrInfo)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[SID=0x%08X] Failed to allocate memory for sequence header",
            psStrCtx->ui32StrId);
        ui32Result = IMG_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    VDEC_BZERO(psStrUnit->psSequHdrInfo);

    psStrUnit->psSequHdrInfo->ui32RefCount = 1;
    psStrUnit->ui32SeqHdrId = ui8SeqHdrId;
    psStrUnit->bClosedGOP = psHostMsgData->bClosedGOP;

    /* Populate sequence header */
    psComSequHdrInfo = &psStrUnit->psSequHdrInfo->sComSequHdrInfo;
    fwbsp_bridge_PopulateSeqHdr(psStrCtx->sConfig.eVidStd,
                                psHostMsgData,
                                psComSequHdrInfo);
    psStrCtx->psComSequHdrInfo = psComSequHdrInfo;

    /* Pass stream unit to the core. */
    ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    DEBUG_REPORT(REPORT_MODULE_FWBSP_BRIDGE,
                 "[SID=0x%08X] Unit SEQUENCE_START SUBMITTED",
                 psStrCtx->ui32StrId);

    return IMG_SUCCESS;

error:
    if (psStrUnit)
    {
        VDECDDUTILS_FreeStrUnit(psStrUnit);
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_SignalNewPicture

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_SignalNewPicture(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId
)
{
    FWBSP_BRIDGE_sHostMsgData *psHostMsgData;
    VDECDD_sStrUnit *psStrUnit = IMG_NULL, *psStrUnitTmp = IMG_NULL;
    VDEC_sComSequHdrInfo *psComSequHdrInfo;
    fwbsp_bridge_sPicture *psPicture;	
    IMG_UINT32 ui32Result;

    IMG_ASSERT(psStrCtx);

    psHostMsgData = &psStrCtx->sHostMsgData;
    IMG_ASSERT(psHostMsgData->ui32ParseId == ui32ParseId);

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32ParseId, IMG_FALSE);
    IMG_ASSERT(psPicture);
    if (psPicture)
    {
        /* Increase reference count for decoding */
        RESOURCE_ItemUse(&psPicture->psPictResInt->ui32RefCount);
    }

    /* Mark picture as valid */
    ui32Result = fwbsp_bridge_PictureSetValid(psStrCtx, ui32ParseId);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    /* Submit VDECDD_STRUNIT_CLOSED_GOP */
    if (psHostMsgData->bClosedGOP)
    {
        ui32Result = VDECDDUTILS_CreateStrUnit(&psStrUnit, IMG_NULL);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
        psStrUnit->eStrUnitType = VDECDD_STRUNIT_CLOSED_GOP;
        psStrUnit->bClosedGOP = IMG_TRUE;
        psStrCtx->bMissingRefsInGOP = IMG_FALSE;
        psStrCtx->ui32GOPCnt++;

        /* Pass stream unit to the core. */
        ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
    }

    /* Submit VDECDD_STRUNIT_PICTURE_START */
    ui32Result = IDGEN_GetHandle(psStrCtx->hParseIdGen,
                                 FWBSP_BRIDGE_GET_PICTURE_ID(ui32ParseId),
                                 (IMG_VOID **)&psStrUnitTmp);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    IMG_ASSERT(psStrUnitTmp != IMG_NULL);

    if (psStrUnitTmp == IMG_NULL)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[USERSID=0x%08X] Unable to find current stream unit for parse ID: %d",
            psStrCtx->sConfig.ui32UserStrId, ui32ParseId);
        ui32Result = IMG_ERROR_COULD_NOT_OBTAIN_RESOURCE;
        goto error;
    }

    /* Allocate a stream unit */
    ui32Result = VDECDDUTILS_CreateStrUnit(&psStrUnit, &psStrUnitTmp->sBitStrSegList);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[SID=0x%08X] Failed to allocate memory for stream unit",
            psStrCtx->ui32StrId);
        goto error;
    }

    /* Populate the stream unit */
    psStrUnit->eStrUnitType = VDECDD_STRUNIT_PICTURE_START;
    psStrUnit->ui32ParseId = ui32ParseId;
    psStrUnit->bClosedGOP = psHostMsgData->bClosedGOP;
    psStrUnit->bBackwardRefs = psHostMsgData->bBackwardRefs;
    psStrUnit->pvStrUnitTag = psStrUnitTmp->pvStrUnitTag;

    /* Allocate memory for the picture header information to go downstream */
    VDEC_MALLOC(psStrUnit->psPictHdrInfo);
    IMG_ASSERT(psStrUnit->psPictHdrInfo != IMG_NULL);
    if (psStrUnit->psPictHdrInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
               "Failed to allocate memory for picture header information");

        ui32Result = IMG_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    VDEC_BZERO(psStrUnit->psPictHdrInfo);

    /* Populate BSPP_sPictHdrInfo data */
    psComSequHdrInfo = psStrCtx->psComSequHdrInfo;
    IMG_ASSERT(psComSequHdrInfo);
    psStrUnit->psPictHdrInfo->bField = psHostMsgData->bFieldCoded;
    psStrUnit->psPictHdrInfo->bPostProcessing = IMG_FALSE;
    psStrUnit->psPictHdrInfo->sCodedFrameSize.ui32Width =
        psComSequHdrInfo->sMaxFrameSize.ui32Width;
    psStrUnit->psPictHdrInfo->sCodedFrameSize.ui32Height =
        psComSequHdrInfo->sMaxFrameSize.ui32Height;
    psStrUnit->psPictHdrInfo->sDispInfo.sEncDispRegion =
        psComSequHdrInfo->sOrigDisplayRegion;
    psStrUnit->psPictHdrInfo->sDispInfo.sDispRegion =
        psComSequHdrInfo->sOrigDisplayRegion;
    psStrUnit->psPictHdrInfo->sDispInfo.ui32NumPanScanWindows = 0;
    VDEC_BZERO(&psStrUnit->psPictHdrInfo->sDispInfo.asPanScanWindows);
    psStrUnit->psPictHdrInfo->sPictSgmData.ui32Id = BSPP_INVALID;

    psStrUnit->psPictHdrInfo->sDispInfo.bRepeatFirstField =
        psHostMsgData->bRepeatFirstField;
    psStrUnit->psPictHdrInfo->sDispInfo.ui32MaxFrmRepeat =
        psHostMsgData->ui8MaxFrmRepeat;

    /* Populate sBufferControl */
    {
        IMG_UINT8 ui8i, ui8Size;

        if (psHostMsgData->ui8PicToDispQty)
        {
            /* aui32DisplayList */
            ui8Size = sizeof(psStrUnit->sBufferControl.aui32DisplayList) /
                sizeof(psStrUnit->sBufferControl.aui32DisplayList[0]);

            IMG_ASSERT(ui8Size == sizeof(psHostMsgData->aui32DisplayList) /
                sizeof(psHostMsgData->aui32DisplayList[0]));
            IMG_ASSERT(psHostMsgData->ui8PicToDispQty <= ui8Size);

            for(ui8i = 0; ui8i < psHostMsgData->ui8PicToDispQty; ui8i++)
            {
                psStrUnit->sBufferControl.aui32DisplayList[ui8i] =
                    psHostMsgData->aui32DisplayList[ui8i];
            }

            /* aui8DisplayFlags */
            ui8Size = sizeof(psStrUnit->sBufferControl.aui8DisplayFlags) /
                sizeof(psStrUnit->sBufferControl.aui8DisplayFlags[0]);

            IMG_ASSERT(ui8Size == sizeof(psHostMsgData->aui8DisplayFlags) /
                sizeof(psHostMsgData->aui8DisplayFlags[0]));
            IMG_ASSERT(psHostMsgData->ui8PicToDispQty <= ui8Size);

            for(ui8i = 0; ui8i < psHostMsgData->ui8PicToDispQty; ui8i++)
            {
                psStrUnit->sBufferControl.aui8DisplayFlags[ui8i] =
                    psHostMsgData->aui8DisplayFlags[ui8i];
            }
        }

        if (psHostMsgData->ui8PicToRelQty)
        {
            /* aui32ReleaseList */
            ui8Size = sizeof(psStrUnit->sBufferControl.aui32ReleaseList) /
                sizeof(psStrUnit->sBufferControl.aui32ReleaseList[0]);

            IMG_ASSERT(ui8Size == sizeof(psHostMsgData->aui32ReleaseList) /
                sizeof(psHostMsgData->aui32ReleaseList[0]));
            IMG_ASSERT(psHostMsgData->ui8PicToRelQty <= ui8Size);

            for(ui8i = 0; ui8i < psHostMsgData->ui8PicToRelQty; ui8i++)
            {
                psStrUnit->sBufferControl.aui32ReleaseList[ui8i] =
                        psHostMsgData->aui32ReleaseList[ui8i];
            }
        }

        if (psHostMsgData->ui8RefPicQty)
        {
            /* aui32RefPicAddrList */
            ui8Size = sizeof(psStrUnit->aui32RefPicList) /
                sizeof(psStrUnit->aui32RefPicList[0]);

            IMG_ASSERT(ui8Size == sizeof(psHostMsgData->aui32RefPicsList) /
                sizeof(psHostMsgData->aui32RefPicsList[0]));
            IMG_ASSERT(psHostMsgData->ui8RefPicQty <= ui8Size);

            for(ui8i = 0; ui8i < psHostMsgData->ui8RefPicQty; ui8i++)
            {
                psStrUnit->aui32RefPicList[ui8i] =
                    psHostMsgData->aui32RefPicsList[ui8i];
            }

            psStrUnit->ui8RefPicQty = psHostMsgData->ui8RefPicQty;
            psStrUnit->ui16RefPicMask = psHostMsgData->ui16RefPicMask;
        }

        /* Colocated pictures */
        if (psHostMsgData->ui8ColPicQty)
        {
            ui8Size = sizeof(psStrUnit->aui32ColPicList) /
                sizeof(psStrUnit->aui32ColPicList[0]);

            IMG_ASSERT(ui8Size == sizeof(psHostMsgData->aui32ColPicPids) /
                sizeof(psHostMsgData->aui32ColPicPids[0]));
            IMG_ASSERT(psHostMsgData->ui8ColPicQty <= ui8Size);

            for(ui8i = 0; ui8i < psHostMsgData->ui8ColPicQty; ui8i++)
            {
                psStrUnit->aui32ColPicList[ui8i] =
                    psHostMsgData->aui32ColPicPids[ui8i];
            }

            psStrUnit->ui8ColPicQty = psHostMsgData->ui8ColPicQty;
        }

        /* Copy data required for picture management. */
        VDECDDUTILS_CopyFWBSPData(&psStrUnit->sFWBSPData,
                                  &psHostMsgData->sFWBSPDataPids);

        psStrUnit->sBufferControl.ui32DisplayListLength =
            psHostMsgData->ui8PicToDispQty;
        psStrUnit->sBufferControl.ui32ReleaseListLength =
            psHostMsgData->ui8PicToRelQty;
        psStrUnit->sBufferControl.bTopFieldFirst =
            psHostMsgData->bTopFieldFirst;
        psStrUnit->sBufferControl.b2ndFieldOfPair =
            psHostMsgData->b2ndFieldOfPair;
        psStrUnit->sBufferControl.eDecPictType =
            (IMG_eBufferType)psHostMsgData->ui8PicType;

        psStrUnit->sBufferControl.ui32Width = 0;
        psStrUnit->sBufferControl.ui32Height = 0;
#if 0
        /* Handle RPR (reference picture resampling) for REAL */
        if (psStrCtx->sConfig.eVidStd == VDEC_STD_REAL)
        {
            // TODO: fetch resolution from firmware message
            psStrUnit->sBufferControl.ui32Width = -1;
            psStrUnit->sBufferControl.ui32Height = -1;
        }
#endif
        /* Store ring buffer marker in the unit context
         * Will be used to synchronize reader when decoding stage takes place */
        psStrUnit->ui32RingBufMark = psHostMsgData->ui32RingBufMark;
        /* Store ring buffer marker in the stream context
         * Will be used to restore writer postion after potential device reset */
        psStrCtx->ui32LastRBMark = psStrUnit->ui32RingBufMark;
    }

    /* Pass stream unit to the core. */
    ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    DEBUG_REPORT(REPORT_MODULE_FWBSP_BRIDGE,
                 "[SID=0x%08X] Unit PICTURE_START SUBMITTED",
                 psStrCtx->ui32StrId);

    /* Signal end of picture */
    ui32Result = VDECDDUTILS_CreateStrUnit(&psStrUnit, IMG_NULL);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
    psStrUnit->eStrUnitType = VDECDD_STRUNIT_PICTURE_END;

    /* Pass stream unit to the core. */
    ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    /* Submit VDECDD_STRUNIT_FENCE when needed */
    psStrUnit = fwbsp_bridge_StreamUnitRemove(psStrCtx,
                                              ui32ParseId,
                                              VDECDD_STRUNIT_FENCE);
    if (psStrUnit)
    {
        /* Pass stream unit to the core. */
        ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
    }

    return IMG_SUCCESS;

error:
    if (psStrUnit)
    {
        VDECDDUTILS_FreeStrUnit(psStrUnit);
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_SignalStreamStopped

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_SignalStreamStopped(
    fwbsp_bridge_sStrContext *psStrCtx)
{
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx);

    psStrCtx->eStrState = FWBSP_BRIDGE_STRSTATE_STOPPED;

    ui32Result = CORE_StreamStop(psStrCtx->ui32StrId, VDEC_STOPPOINT_NOW, 0);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_SignalUnsupportedFeature

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_SignalUnsupportedFeature(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    IMG_UINT32                ui32ErrorFlags)
{
    VDECDD_sStrUnit *psStrUnit;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx && ui32ParseId);

    REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
           "[USERSID=0x%08x, PID=0x%08x] UNSUPPORTED[SW]: Id %d, %s",
           psStrCtx->ui32StrId, ui32ParseId, FWBSP_GET_ERROR_CODE(ui32ErrorFlags),
           fwbsp_bridge_GetErrDesc(psStrCtx->sConfig.eVidStd, ui32ErrorFlags));

    /* Allocate a stream unit */
    ui32Result = VDECDDUTILS_CreateStrUnit(&psStrUnit, IMG_NULL);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[SID=0x%08X] Failed to allocate memory for stream unit",
            psStrCtx->ui32StrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    psStrUnit->eStrUnitType = VDECDD_STRUNIT_FWBSP_UNSUPPORTED;
    psStrUnit->ui32ParseId = ui32ParseId;

    ui32Result = CORE_StreamSubmitUnit(psStrCtx->ui32StrId, psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_SignalFwError

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_SignalFwError(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId,
    IMG_UINT32                ui32ErrorFlags)
{
    IMG_ASSERT(psStrCtx && ui32ParseId);

    REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_WARNING,
           "[USERSID=0x%08x, PID=0x%08x] ERROR[SW]: Id %d, %s",
           psStrCtx->ui32StrId, psStrCtx->sHostMsgData.ui32ParseId,
           FWBSP_GET_ERROR_CODE(ui32ErrorFlags),
           fwbsp_bridge_GetErrDesc(psStrCtx->sConfig.eVidStd, ui32ErrorFlags));

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              fwbsp_bridge_PopulateH264SeqHdr

******************************************************************************/
static IMG_VOID
fwbsp_bridge_PopulateH264SeqHdr(
    const FWBSP_BRIDGE_sHostMsgData *psHostMsgData,
    VDEC_sComSequHdrInfo            *psComSequHdrInfo
)
{
    psComSequHdrInfo->sMaxFrameSize.ui32Width = (psHostMsgData->ui16PicWidth + 1) * 16;
    /* H264 has always coded size MB aligned. For sequences which *may* have
     * Field-Coded pictures, as described by the frame_mbs_only_flag,
     * the pic_height_in_map_units_minus1 refers to field height in MBs,
     * so to find the actual Frame height we need to do Field_MBs_InHeight * 32 */
    psComSequHdrInfo->sMaxFrameSize.ui32Height = (psHostMsgData->ui16PicHeight + 1)
        * (psHostMsgData->bFrameMbsOnly ? 1 : 2) * 16;
    psComSequHdrInfo->bFieldCodedMBlocks = psHostMsgData->bAdaptiveFrameField;

    if (psHostMsgData->bFrameCropping)
    {
         IMG_INT32 i32SubWidthC, i32SubHeightC, i32CropUnitX, i32CropUnitY;
         IMG_INT32 i32FrameCropLeft, i32FrameCropRight;
         IMG_INT32 i32FrameCropTop, i32FrameCropBottom;

         i32SubWidthC = ai32H264subWidthC[psHostMsgData->ui8ChromaFormatIdc];
         i32SubHeightC = ai32H264subHeightC[psHostMsgData->ui8ChromaFormatIdc];
         /* equation source: ITU-T H.264 2010/03, page 77 */
         /* ChromaArrayType == 0 */
         if(psHostMsgData->bSepColPlane || psHostMsgData->ui8ChromaFormatIdc == 0)
         {
             /* (7-18) */
             i32CropUnitX = 1;
             /* (7-19) */
             i32CropUnitY = 2 - !!psHostMsgData->bFrameMbsOnly;
         }
         else /* ChromaArrayType == chroma_format_idc */
         {
             /* (7-20) */
             i32CropUnitX = i32SubWidthC;
             /* (7-21) */
             i32CropUnitY = i32SubHeightC *(2 - !!psHostMsgData->bFrameMbsOnly);
         }

         /* source: ITU-T H.264 2010/03, page 77 */
         IMG_ASSERT(psHostMsgData->ui16FrameCropLeft <=
            (psComSequHdrInfo->sMaxFrameSize.ui32Width / i32CropUnitX)
            - (psHostMsgData->ui16FrameCropRight + 1));

         IMG_ASSERT(psHostMsgData->ui16FrameCropTop  <=
            (psComSequHdrInfo->sMaxFrameSize.ui32Height / i32CropUnitY)
            - (psHostMsgData->ui16FrameCropBottom + 1));
         i32FrameCropLeft = i32CropUnitX * psHostMsgData->ui16FrameCropLeft;
         i32FrameCropRight = psComSequHdrInfo->sMaxFrameSize.ui32Width -
            (i32CropUnitX * psHostMsgData->ui16FrameCropRight);
         i32FrameCropTop = i32CropUnitY * psHostMsgData->ui16FrameCropTop;
         i32FrameCropBottom = psComSequHdrInfo->sMaxFrameSize.ui32Height
            - (i32CropUnitY * psHostMsgData->ui16FrameCropBottom);
         psComSequHdrInfo->sOrigDisplayRegion.ui32LeftOffset = i32FrameCropLeft;
         psComSequHdrInfo->sOrigDisplayRegion.ui32TopOffset = i32FrameCropTop;
         psComSequHdrInfo->sOrigDisplayRegion.ui32Width =
            i32FrameCropRight - i32FrameCropLeft ;
         psComSequHdrInfo->sOrigDisplayRegion.ui32Height =
            i32FrameCropBottom - i32FrameCropTop;
    }
    else
    {
        psComSequHdrInfo->sOrigDisplayRegion.ui32LeftOffset = 0;
        psComSequHdrInfo->sOrigDisplayRegion.ui32TopOffset = 0;
        psComSequHdrInfo->sOrigDisplayRegion.ui32Width =
            psComSequHdrInfo->sMaxFrameSize.ui32Width;
        psComSequHdrInfo->sOrigDisplayRegion.ui32Height =
            psComSequHdrInfo->sMaxFrameSize.ui32Height;
    }

}

/*!
******************************************************************************

 @Function              fwbsp_bridge_PopulateHEVCSeqHdr

******************************************************************************/
static IMG_VOID
fwbsp_bridge_PopulateHEVCSeqHdr(
    const FWBSP_BRIDGE_sHostMsgData *psHostMsgData,
    VDEC_sComSequHdrInfo            *psComSequHdrInfo
)
{
    psComSequHdrInfo->sMaxFrameSize.ui32Width = psHostMsgData->ui16MaxPicWidth;
    psComSequHdrInfo->sMaxFrameSize.ui32Height = psHostMsgData->ui16MaxPicHeight;

    psComSequHdrInfo->sFrameSize.ui32Width = psHostMsgData->ui16PicWidth;
    psComSequHdrInfo->sFrameSize.ui32Height = psHostMsgData->ui16PicHeight;

    psComSequHdrInfo->bFieldCodedMBlocks = IMG_FALSE;

    if(psHostMsgData->bClosedGOP)
    {
        psComSequHdrInfo->bNotDpbFlush = psHostMsgData->bNotDpbFlush;
    }

    if (psHostMsgData->bFrameCropping)
    {
        IMG_INT32 i32SubWidthC, i32SubHeightC;
        VDEC_sRect * psDispRegion = &psComSequHdrInfo->sOrigDisplayRegion;

        i32SubWidthC = ai32HEVCsubWidthC[psHostMsgData->ui8ChromaFormatIdc];
        i32SubHeightC = ai32HEVCsubHeightC[psHostMsgData->ui8ChromaFormatIdc];

        psDispRegion->ui32TopOffset =
            i32SubHeightC * psHostMsgData->ui16FrameCropTop;
        psDispRegion->ui32LeftOffset =
            i32SubWidthC * psHostMsgData->ui16FrameCropLeft;
        psDispRegion->ui32Width =
            psHostMsgData->ui16PicWidth -
            psDispRegion->ui32LeftOffset -
            i32SubWidthC * psHostMsgData->ui16FrameCropRight;
        psDispRegion->ui32Height =
            psHostMsgData->ui16PicHeight -
            psDispRegion->ui32TopOffset -
        i32SubHeightC * psHostMsgData->ui16FrameCropBottom;
    }
    else
    {
        psComSequHdrInfo->sOrigDisplayRegion.ui32LeftOffset = 0;
        psComSequHdrInfo->sOrigDisplayRegion.ui32TopOffset = 0;
        psComSequHdrInfo->sOrigDisplayRegion.ui32Width =
            psHostMsgData->ui16PicWidth;
        psComSequHdrInfo->sOrigDisplayRegion.ui32Height =
            psHostMsgData->ui16PicHeight;
    }
}
/*!
******************************************************************************

 @Function              fwbsp_bridge_PopulateSeqHdr

******************************************************************************/
static IMG_VOID
fwbsp_bridge_PopulateSeqHdr(
    VDEC_eVidStd        eVidStd,
    const FWBSP_BRIDGE_sHostMsgData *psHostMsgData,
    VDEC_sComSequHdrInfo            *psComSequHdrInfo
)
{
    IMG_ASSERT(psComSequHdrInfo && psHostMsgData);

    psComSequHdrInfo->ui32CodecProfile = psHostMsgData->ui8ProfileIdc;
    psComSequHdrInfo->ui32CodecLevel = psHostMsgData->ui8LevelIdc;

    psComSequHdrInfo->ui32NumViews = 1;

    /* VUI params */
    if (psHostMsgData->bVUIParamsPresent && psHostMsgData->bTimingInfo)
    {
        psComSequHdrInfo->ui32FrameRateNum = psHostMsgData->ui32TimeScale;
        psComSequHdrInfo->ui32FrameRateDen = 2 * psHostMsgData->ui32NumUnitsInTick;
        /* Zero frame rate using memset, assigning zero won't compile on some
         * toolchains (e.g. aarch64-linux-android-gcc (GCC) 4.9) */
        IMG_MEMSET(&psComSequHdrInfo->fFrameRate, 0, sizeof(psComSequHdrInfo->fFrameRate));
    }

    psComSequHdrInfo->sColorSpaceInfo.bIsPresent = psHostMsgData->bColorSpacePresent;
    if (psComSequHdrInfo->sColorSpaceInfo.bIsPresent)
    {
        psComSequHdrInfo->sColorSpaceInfo.ui8ColourPrimaries =
            psHostMsgData->ui8ColorPrimaries;
        psComSequHdrInfo->sColorSpaceInfo.ui8TransferCharacteristics =
            psHostMsgData->ui8TransferCharacteristic;
        psComSequHdrInfo->sColorSpaceInfo.ui8MatrixCoefficients =
            psHostMsgData->ui8MartixCoeffs;
    }

    psComSequHdrInfo->ui32AspectRatioNum = psHostMsgData->ui16SarWidth;
    psComSequHdrInfo->ui32AspectRatioDen = psHostMsgData->ui16SarHeight;

#ifdef REDUCED_DPB_NO_PIC_REDORDERING
    psComSequHdrInfo->ui32MaxReorderPicts = psHostMsgData->ui32MaxDecFrameBuff;
#else /* REDUCED_DPB_NO_PIC_REDORDERING */
    if (psHostMsgData->bVUIParamsPresent && psHostMsgData->bBstrRestrFlag)
    {
        psComSequHdrInfo->ui32MaxReorderPicts = psHostMsgData->ui32MaxDecFrameBuff;
    }
    else
    {
        psComSequHdrInfo->ui32MaxReorderPicts = 0;
    }
#endif /* ndef REDUCED_DPB_NO_PIC_REDORDERING */
    psComSequHdrInfo->ui32MinPicBufNum = psHostMsgData->ui32MaxDecFrameBuff;

    psComSequHdrInfo->bInterlacedFrames =
        psHostMsgData->bFrameMbsOnly ? IMG_FALSE : IMG_TRUE;

    IMG_ASSERT(psHostMsgData->ui8ChromaFormatIdc < 4);
    psComSequHdrInfo->sPixelInfo.bChromaFormat =
        (psHostMsgData->ui8ChromaFormatIdc == 0) ? IMG_FALSE : IMG_TRUE;
    psComSequHdrInfo->sPixelInfo.eChromaFormatIdc =
        aeH264PixelFormatIdc[psHostMsgData->ui8ChromaFormatIdc];

    psComSequHdrInfo->sPixelInfo.eChromaInterleaved =
        ((psHostMsgData->ui8ChromaFormatIdc == 0) ||
         (psHostMsgData->ui8ChromaFormatIdc == 3 && psHostMsgData->bSepColPlane)
        ) ? PIXEL_INVALID_CI : PIXEL_UV_ORDER;

    psComSequHdrInfo->sPixelInfo.ui32NoPlanes =
        (psHostMsgData->ui8ChromaFormatIdc == 0) ? 1 :
        (psHostMsgData->ui8ChromaFormatIdc == 3 && psHostMsgData->bSepColPlane) ? 3 : 2;

    psComSequHdrInfo->sPixelInfo.ui32BitDepthY = psHostMsgData->ui8BitDepthLuma + 8;
    psComSequHdrInfo->sPixelInfo.ui32BitDepthC = psHostMsgData->ui8BitDepthChroma + 8;
    psComSequHdrInfo->sPixelInfo.eMemoryPacking =
        (psComSequHdrInfo->sPixelInfo.ui32BitDepthY > 8 ||
         psComSequHdrInfo->sPixelInfo.ui32BitDepthC > 8)
         ? PIXEL_BIT10_MSB_MP : PIXEL_BIT8_MP;

    psComSequHdrInfo->sPixelInfo.ePixelFormat =
        PIXEL_GetPixelFormat(
            psComSequHdrInfo->sPixelInfo.eChromaFormatIdc,
            psComSequHdrInfo->sPixelInfo.eChromaInterleaved,
            psComSequHdrInfo->sPixelInfo.eMemoryPacking,
            psComSequHdrInfo->sPixelInfo.ui32BitDepthY,
            psComSequHdrInfo->sPixelInfo.ui32BitDepthC,
            psComSequHdrInfo->sPixelInfo.ui32NoPlanes);

    //TO DO: move populating ComSequHdr to FW?
    switch (eVidStd)
    {
    case VDEC_STD_H264:
        {
            fwbsp_bridge_PopulateH264SeqHdr(psHostMsgData,
                                            psComSequHdrInfo);
            break;
        }
    case VDEC_STD_HEVC:
        {
            fwbsp_bridge_PopulateHEVCSeqHdr(psHostMsgData,
                                            psComSequHdrInfo);
            break;
        }
    default:
        break;
    }

    psComSequHdrInfo->bSeparateChromaPlanes = psHostMsgData->bSepColPlane;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_TranslatePIDTotTID

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_TranslatePIDTotTID(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit
)
{
    IMG_UINT32 ui32ParseId;
    IMG_UINT8 ui8i, ui8DisplayPicQty, ui8ReleasePicQty;

    ui8DisplayPicQty = sizeof(psStrUnit->sBufferControl.aui32DisplayList) /
        sizeof(psStrUnit->sBufferControl.aui32DisplayList[0]);
    ui8ReleasePicQty = sizeof(psStrUnit->sBufferControl.aui32ReleaseList) /
        sizeof(psStrUnit->sBufferControl.aui32ReleaseList[0]);

    /* Translate PID's on display pic list */
    for (ui8i = 0; ui8i < ui8DisplayPicQty; ui8i++)
    {
        ui32ParseId = psStrUnit->sBufferControl.aui32DisplayList[ui8i];
        if (ui32ParseId)
        {
            psStrUnit->sBufferControl.aui32DisplayList[ui8i] =
                fwbsp_bridge_GetTransId(psStrCtx, ui32ParseId);
        }
    }

    /* Translate PID's on release pic list */
    for (ui8i = 0; ui8i < ui8ReleasePicQty; ui8i++)
    {
        ui32ParseId = psStrUnit->sBufferControl.aui32ReleaseList[ui8i];
        if (ui32ParseId)
        {
            psStrUnit->sBufferControl.aui32ReleaseList[ui8i] =
                fwbsp_bridge_GetTransId(psStrCtx, ui32ParseId);
        }
    }

    /* Translate PID's on reference pic list */
    for (ui8i = 0; ui8i < psStrUnit->ui8RefPicQty; ui8i++)
    {
        ui32ParseId = psStrUnit->aui32RefPicList[ui8i];
        if (ui32ParseId)
        {
            psStrUnit->aui32RefPicList[ui8i] =
                fwbsp_bridge_GetTransId(psStrCtx, ui32ParseId);
        }
    }

    /* Translate PIDs of colocated picture buffers list. */
    for (ui8i = 0; ui8i < psStrUnit->ui8ColPicQty; ui8i++)
    {
        ui32ParseId = psStrUnit->aui32ColPicList[ui8i];
        if (ui32ParseId)
        {
            psStrUnit->aui32ColPicList[ui8i] =
                fwbsp_bridge_GetTransId(psStrCtx, ui32ParseId);
        }
    }

    /* Translate PIDs of potential display pictures in DPB list. */
    for (ui8i = 0;
         ui8i < psStrUnit->sFWBSPData.sDPBState.ui8DPBDisPicLen;
         ui8i++)
    {
        ui32ParseId = psStrUnit->sFWBSPData.sDPBState.aui32DPBDisList[ui8i];
        if (ui32ParseId)
        {
            psStrUnit->sFWBSPData.sDPBState.aui32DPBDisList[ui8i] =
                fwbsp_bridge_GetTransId(psStrCtx, ui32ParseId);
        }
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_TranslateTIDToBufAddr

******************************************************************************/
static IMG_RESULT
fwbsp_bridge_TranslateTIDToBufAddr(
    fwbsp_bridge_sStrContext *psStrCtx,
    VDECDD_sStrUnit          *psStrUnit,
    DECODER_sDecPict         *psDecPict
)
{
    fwbsp_bridge_sPicture *psPicture;
    IMG_UINT32 ui32TransId;
    IMG_UINT8 ui8i;

    /* Translate transcation IDs of reference picture into addresses
     * of corresponding buffers. */
    for (ui8i = 0; ui8i < psStrUnit->ui8RefPicQty; ui8i++)
    {
        ui32TransId = psStrUnit->aui32RefPicList[ui8i];

        if (ui32TransId)
        {
            psPicture = fwbsp_bridge_PictureGet(psStrCtx,
                                                ui32TransId,
                                                IMG_TRUE);
            IMG_ASSERT(psPicture);
            if (!psPicture)
            {
                return IMG_ERROR_FATAL;
            }

            /* Luma base address */
            psDecPict->aui32RefPicLumaAddrList[ui8i] =
               psPicture->ui32LumaBaseAddr;

            /* Chroma base address */
            psDecPict->aui32RefPicChromaAddrList[ui8i] =
               psPicture->ui32ChromaBaseAddr;

#ifdef ENABLE_SEPARATE_COLOUR_PLANES
            /* Chroma2 base address */
             psDecPict->aui32RefPicChroma2AddrList[ui8i] =
                psPicture->ui32Chroma2BaseAddr;
#endif /* ENABLE_SEPARATE_COLOUR_PLANES */
        }
        else
        {
            psStrCtx->bMissingRefsInGOP = IMG_TRUE;
        }
    }

    /* Translate transcation ID of colocated pictures into addresses
     * of corresponding MB params buffers. */
    for (ui8i = 0; ui8i < psStrUnit->ui8ColPicQty; ui8i++)
    {
        ui32TransId = psStrUnit->aui32ColPicList[ui8i];

        if (ui32TransId)
        {
            psPicture = fwbsp_bridge_PictureGet(psStrCtx,
                                                ui32TransId,
                                                IMG_TRUE);
            IMG_ASSERT(psPicture);
            if (!psPicture)
            {
                return IMG_ERROR_FATAL;
            }

            psDecPict->aui32ColPicAddrList[ui8i] =
               psPicture->ui32MBParamsAddr;
        }
    }

    if (psStrCtx->bMissingRefsInGOP &&
        !psStrCtx->ui32GOPCnt &&
        psStrUnit->bBackwardRefs &&
        psDecPict->psFirstFieldFwMsg)
    {
        psDecPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError |=
                FLAG_MASK(VDECFW_MSGFLAG_DECODED_MISSING_REFERENCES);
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetFwCtxBufSize

******************************************************************************/
static IMG_UINT32
fwbsp_bridge_GetFwCtxBufSize(
    VDEC_eVidStd eVidStd
)
{
    switch(eVidStd)
    {
        case VDEC_STD_H264:
            return sizeof(H264FW_sContextData);
        case VDEC_STD_HEVC:
            return sizeof(HEVCFW_sContextData);
        default:
            IMG_ASSERT(0=="Unknown context buffer size!");
            return 0;
    }
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetSPSBufSize

******************************************************************************/
static IMG_UINT32
fwbsp_bridge_GetSPSBufSize(
    VDEC_eVidStd eVidStd
)
{
    IMG_UINT32 ui32Size = 0;

    switch(eVidStd)
    {
        case VDEC_STD_H264:
            ui32Size = sizeof(H264FW_sSequencePS);
            break;

        case VDEC_STD_HEVC:
            ui32Size = sizeof(HEVCFW_sSequencePS);
            break;

        default:
            IMG_ASSERT(0=="Unknown SPS buffer size!");
            break;
    }

    ui32Size = SEQUENCE_SLOTS * (((ui32Size >> 6) + 1) << 6);

    return ui32Size;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetPPSBufSize

******************************************************************************/
static IMG_UINT32
fwbsp_bridge_GetPPSBufSize(
    VDEC_eVidStd eVidStd
)
{
    IMG_UINT32 ui32Size = 0;

    switch(eVidStd)
    {
        case VDEC_STD_H264:
            ui32Size = sizeof(H264FW_sPicturePS);
            break;

        case VDEC_STD_HEVC:
            ui32Size = sizeof(HEVCFW_sPicturePS);
            break;

        default:
            IMG_ASSERT(0=="Unknown PPS buffer size!");
            break;
    }

    ui32Size = PPS_SLOTS * (((ui32Size >> 6) + 1) << 6);

    return ui32Size;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetPPSBufSize

******************************************************************************/
static IMG_UINT32
fwbsp_bridge_GetVPSBufSize(
    VDEC_eVidStd eVidStd
)
{
    IMG_UINT32 ui32Size = 0;

    switch(eVidStd)
    {
        case VDEC_STD_H264:
            // no VPS so take 0
            break;

        case VDEC_STD_HEVC:
            ui32Size = sizeof(HEVCFW_sVideoPS);
            break;

        default:
            IMG_ASSERT(0=="Unknown PPS buffer size!");
            break;
    }

    ui32Size = VPS_SLOTS * (((ui32Size >> 6) + 1) << 6);

    return ui32Size;
}

/*!
******************************************************************************

 @Function              fwbsp_bridge_GetTransId

******************************************************************************/
static IMG_UINT32
fwbsp_bridge_GetTransId(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32ParseId
)
{
    fwbsp_bridge_sPicture *psPicture;

    IMG_ASSERT(psStrCtx);

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32ParseId, IMG_FALSE);
    if (psPicture)
    {
        return psPicture->ui32TransId;
    }

    /* TransId not found */
    return 0;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetParseId

******************************************************************************/
static IMG_UINT32
fwbsp_bridge_GetParseId(
    fwbsp_bridge_sStrContext *psStrCtx,
    IMG_UINT32                ui32TransId
)
{
    fwbsp_bridge_sPicture *psPicture;

    IMG_ASSERT(psStrCtx);

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32TransId, IMG_TRUE);
    IMG_ASSERT(psPicture);
    if (psPicture)
    {
        return psPicture->ui32ParseId;
    }

    /* ParseId not found */
    return 0;
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetMsgName

******************************************************************************/
static const IMG_CHAR  *
fwbsp_bridge_GetMsgName(
    IMG_UINT8 ui8MsgType
)
{
    switch (ui8MsgType)
    {
        case FWBSP_NEW_SEQ:
            return "FWBSP_NEW_SEQ";
        case FWBSP_NEW_PIC:
            return "FWBSP_NEW_PIC";
        case FWBSP_BUF_EMPTY:
            return "FWBSP_BUF_EMPTY";
        case FWBSP_ERROR:
            return "FWBSP_ERROR";
        case FWDEC_COMPLETED:
            return "FWDEC_COMPLETED";
        default:
            break;
    }

    return "Unknown message";
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetErrDesc

******************************************************************************/
static const IMG_CHAR *
fwbsp_bridge_GetErrDesc(
    VDEC_eVidStd eVidStd,
    IMG_UINT32   ui32Err)
{
    if (FWBSP_IS_INT_ERROR(ui32Err)) /* fwbsp_bridge internal error */
    {
        switch(ui32Err)
        {
            case FWBSP_BRIDGE_TOO_MANY_BSTR:
                return "Too many bitstream segments!";
                break;

            default:
                return "Unknown error code!";
                break;
        }
    }
    else /* firmware error */
    {
        switch (eVidStd)
        {
            case VDEC_STD_H264:
                return fwbsp_bridge_GetErrDesc_H264(ui32Err);
                break;

            case VDEC_STD_HEVC:
                return fwbsp_bridge_GetErrDesc_HEVC(ui32Err);
                break;

            default:
                return "Unknown video standard!";
                break;
        }
    }
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetErrDesc_H264

******************************************************************************/
static const IMG_CHAR *
fwbsp_bridge_GetErrDesc_H264(
    IMG_UINT32 ui32Err
)
{
    switch (ui32Err)
    {
        case H264_ERR_SPS_INV_PROFILE_IDC:
            return "SPS: invalid profile IDC";
        case H264_ERR_SPS_INV_SPS_ID:
            return "SPS: invalid SPS ID";
        case H264_ERR_SPS_INV_FORMAT_IDC:
            return "SPS: invalid format IDC";
        case H264_ERR_SPS_INV_BIT_DEPTH_LUMA_MIN8:
            return "SPS: invalid luma bit depth";
        case H264_ERR_SPS_INV_BIT_DEPTH_CHROMA_MIN8:
            return "SPS: invalid chroma bit depth";
        case H264_ERR_SPS_INV_SCAL_LIST:
            return "SPS: invalid scaling lists";
        case H264_ERR_SPS_INV_LOG2_MAX_FRAME_NUM:
            return "SPS: invalid max frame number";
        case H264_ERR_SPS_INV_PIC_ORD_CNT:
            return "SPS: invalid picture order count";
        case H264_ERR_SPS_INV_MAX_PIC_ORD_CNT:
            return "SPS: invalid max picture order count";
        case H264_ERR_SPS_INV_NUM_REF_FRM_IN_PIC:
            return "SPS: invalid number of reference frames in picture";
        case H264_ERR_SPS_INV_NUM_REF_FRM:
            return "SPS: invalid number of reference frames";
        case H264_ERR_SPS_INV_PIC_WIDTH:
            return "SPS: invalid picture width";
        case H264_ERR_SPS_INV_PIC_HEIGHT:
            return "SPS: invalid picture height";
        case H264_ERR_VUI_INV_ASPECT_RATIO:
            return "VUI: invalid aspect ratio";
        case H264_ERR_VUI_INV_TIMING:
            return "VUI: invalid timing";
        case H264_ERR_VUI_INV_NUM_REORDERED_FRAMES:
            return "VUI: invalid number of reordered frames";
        case H264_ERR_VUI_INV_CPB_CNT:
            return "VUI: invalid CPB count";
        case H264_ERR_PPS_SLICE_GROUPS_NOT_SUP:
            return "PPS: slice groups not supported";
        case H264_ERR_PPS_PPS_INV_ID:
            return "PPS: invalid ID";
        case H264_ERR_PPS_SPS_INV_ID:
            return "PPS: invalid SPS ID";
        case H264_ERR_PPS_SPS_MISSING:
            return "PPS: SPS missing";
        case H264_ERR_PPS_INV_NUM_SLICE_GROUPS:
            return "PPS: invalid num slice groups";
        case H264_ERR_PPS_INV_SIZE_IN_MAP:
            return "PPS: invalid size in map";
        case H264_ERR_PPS_INV_NUM_REF_LX:
            return "PPS: invalid number of LX references";
        case H264_ERR_PPS_INV_SCAL_LIST_NUM:
            return "PPS: invalid scaling lists number";
        case H264_ERR_PPS_INV_SCAL_LIST:
            return "PPS: invalid scaling lists";
        case H264_ERR_SEI_DATA_OVERFLOW:
            return "SEI: data overflow";
        case H264_ERR_SEI_DATA_UNDERFLOW:
            return "SEI: data underflow";
        case H264_ERR_FAILED_TO_STORE_SPS:
            return "GENERIC: failed to store SPS";
        case H264_ERR_FAILED_TO_STORE_PPS:
            return "GENERIC: failed to store PPS";
        case H264_ERR_FAILED_TO_FETCH_SPS:
            return "GENERIC: failed to fetch SPS";
        case H264_ERR_FAILED_TO_FETCH_PPS:
            return "GENERIC: failed to fetch PPS";
        case H264_ERR_FIELD_NOT_SUP:
            return "GENERIC: field coded pictures not supported!";
        case H264_ERR_ASO_NOT_SUP:
            return "GENERIC: discontinuous MB (ASO) not supported!";
        default:
            break;
    }

    return "Unknown error code!";
}


/*!
******************************************************************************

 @Function              fwbsp_bridge_GetErrDesc_HEVC

******************************************************************************/
static const IMG_CHAR *
fwbsp_bridge_GetErrDesc_HEVC(
    IMG_UINT32 ui32Err
)
{
    switch (ui32Err)
    {
        case HEVC_ERR_INV_VIDEO_DIMENSION:
            return "SPS: invalid video dimensions";
        case HEVC_ERR_NO_SEQUENCE_HDR:
            return "PPS: invalid SPS ID";
        case HEVC_ERR_SPS_EXT_UNSUPP:
            return "SPS: range extensions unsupported";
        case HEVC_ERR_PPS_EXT_UNSUPP:
            return "PPS: range extensions unsupported";
        case HEVC_ERR_FAILED_TO_STORE_VPS:
            return "GENERIC: failed to store VPS";
        case HEVC_ERR_FAILED_TO_STORE_SPS:
            return "GENERIC: failed to store SPS";
        case HEVC_ERR_FAILED_TO_STORE_PPS:
            return "GENERIC: failed to store PPS";
        case HEVC_ERR_FAILED_TO_FETCH_VPS:
            return "GENERIC: failed to fetch VPS";
        case HEVC_ERR_FAILED_TO_FETCH_SPS:
            return "GENERIC: failed to fetch SPS";
        case HEVC_ERR_FAILED_TO_FETCH_PPS:
            return "GENERIC: failed to fetch PPS";
        default:
            break;
    }

    return "Unknown error code!";
}



/* FWBSP_BRIDGE external functions - implementation                          */
/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_Initialise

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_Initialise(
    IMG_VOID *                 pvInitUserData,
    VDECDD_pfnCompCallback     pfnCallback,
    IMG_HANDLE                *phCtx,
    IMG_HANDLE                 hDecCtx,
    const VDECDD_sDdDevConfig *psDdDevConfig
)
{
    fwbsp_bridge_sContext *psCtx = (fwbsp_bridge_sContext*)*phCtx;

    if (psCtx == IMG_NULL)
    {
        VDEC_MALLOC(psCtx);
        IMG_ASSERT(psCtx);
        if (psCtx == IMG_NULL)
        {
            REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                "Failed to allocate memory for FWBSP_BRIDGE context");
            return IMG_ERROR_OUT_OF_MEMORY;
        }
        VDEC_BZERO(psCtx);
        *phCtx = psCtx;
    }

    if (!psCtx->bInitialised)
    {
        psCtx->pfnCompCallback = pfnCallback;
        psCtx->pvCompInitUserData = pvInitUserData;
        psCtx->hDecCtx = hDecCtx;
        psCtx->psDevConfig = psDdDevConfig;
        LST_init(&psCtx->sFwMsgBufList);
        LST_init(&psCtx->sStreamList);
        psCtx->bInitialised = IMG_TRUE;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_DeInitialise

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_DeInitialise(
    IMG_HANDLE hCtx
)
{
    fwbsp_bridge_sContext * psCtx = (fwbsp_bridge_sContext*)hCtx;

    if (psCtx && psCtx->bInitialised)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_INFO, "FWBSP_BRIDGE_DeInitialise");

        /* Remove F/W messages buffers */
        while (!LST_empty(&psCtx->sFwMsgBufList))
        {
            fwbsp_bridge_sFwMsgItem *psFwMsgItem = (fwbsp_bridge_sFwMsgItem *)
                LST_removeHead(&psCtx->sFwMsgBufList);
            if (psFwMsgItem)
            {
                IMG_ASSERT(psFwMsgItem->bProcess == IMG_FALSE);
                IMG_FREE(psFwMsgItem);
                psFwMsgItem = IMG_NULL;
            }
        }

        psCtx->bInitialised = IMG_FALSE;
        IMG_FREE(psCtx);
        psCtx = IMG_NULL;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamCreate

******************************************************************************/
IMG_RESULT FWBSP_BRIDGE_StreamCreate(
    IMG_HANDLE          hCtx,
    IMG_HANDLE         *phStrCtx,
    IMG_HANDLE          hMmuStrHandle,
    IMG_HANDLE          hDecStrHandle,
    IMG_VOID           *pvStrUserData,
    IMG_UINT32          ui32StrId,
    SYS_eMemPool        eSecureMemPool,
    VDEC_sStrConfigData sStrConfig
)
{
    fwbsp_bridge_sContext *psCtx = (fwbsp_bridge_sContext*)hCtx;
    fwbsp_bridge_sStrContext *psStrCtx;
    IMG_RESULT ui32Result;

    /* Check input params */
    IMG_ASSERT(sStrConfig.eVidStd == VDEC_STD_H264 ||
               sStrConfig.eVidStd == VDEC_STD_HEVC);
    IMG_ASSERT(hMmuStrHandle);
    IMG_ASSERT(psCtx);

    if(psCtx == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if (psCtx->ui8StrCnt >= FWBSP_BRIDGE_MAX_CONCURRENT_STREAMS)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "Device has too many concurrent streams. "
            "Number of Concurrent streams allowed: %d.",
            MAX_CONCURRENT_STREAMS);
        return IMG_ERROR_DEVICE_UNAVAILABLE;
    }

    VDEC_MALLOC(psStrCtx);
    IMG_ASSERT(psStrCtx);
    if (psStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for Decoder stream context",
            sStrConfig.ui32UserStrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psStrCtx);

    *phStrCtx = (IMG_HANDLE)psStrCtx;
    psStrCtx->hDecStrHandle = hDecStrHandle;
    psStrCtx->pvUserData = pvStrUserData;
    psStrCtx->ui32StrId = ui32StrId;
    psStrCtx->ui32DecStrId = DECODER_GetStreamId(psStrCtx->hDecStrHandle);
    DECODER_SetFWBSP_BRIDGEStrCtxHandle(psStrCtx->hDecStrHandle, *phStrCtx);
    psStrCtx->hMmuStrHandle = hMmuStrHandle;
    psStrCtx->psCtx = psCtx;
    psStrCtx->sConfig = sStrConfig;
    psStrCtx->ui32PtdDevVirt = DECODER_GetPtdDevAddr(psCtx->hDecCtx);
    psStrCtx->eSecurePool = eSecureMemPool;

    LST_init(&psStrCtx->sEncBitstreamStrUnitList);
    LST_init(&psStrCtx->sFenceStrUnitList);
    LST_init(&psStrCtx->sPictResList);

    ui32Result = fwbsp_bridge_StreamResourceCreate(psStrCtx);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    fwbsp_bridge_StreamCtxAdd(psStrCtx);

    return IMG_SUCCESS;

    /* Roll back in case of errors. */
error:
    FWBSP_BRIDGE_StreamDestroy((IMG_HANDLE)psStrCtx);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamDestroy - TODO

******************************************************************************/
IMG_RESULT FWBSP_BRIDGE_StreamDestroy(
    const IMG_HANDLE hStrContext
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext*)hStrContext;
    VDECDD_sStrUnit *psStrUnit;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx);

    /* Free encrypted bitstream stream units */
    psStrUnit = (VDECDD_sStrUnit *)LST_removeHead(&psStrCtx->sEncBitstreamStrUnitList);
    while (psStrUnit)
    {
        ui32Result = VDECDDUTILS_FreeStrUnit(psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        psStrUnit = (VDECDD_sStrUnit *)LST_removeHead(&psStrCtx->sEncBitstreamStrUnitList);
    }

    /* Free fence stream units */
    psStrUnit = (VDECDD_sStrUnit *)LST_removeHead(&psStrCtx->sFenceStrUnitList);
    while (psStrUnit)
    {
        ui32Result = VDECDDUTILS_FreeStrUnit(psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        psStrUnit = (VDECDD_sStrUnit *)LST_removeHead(&psStrCtx->sFenceStrUnitList);
    }

    ui32Result = fwbsp_bridge_StreamResourceDestroy(psStrCtx);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    fwbsp_bridge_StreamCtxRemove(psStrCtx);

    IMG_FREE(psStrCtx);
    psStrCtx = IMG_NULL;

    return ui32Result;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamProcessUnit

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_StreamProcessUnit(
    IMG_HANDLE hStrCtx,
    VDECDD_sStrUnit * psStrUnit
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext*)hStrCtx;
    fwbsp_bridge_sContext *psCtx;
    IMG_RESULT ui32Result;

    /* Check input params */
    IMG_ASSERT(hStrCtx);
    IMG_ASSERT(psStrUnit);

    if(hStrCtx == IMG_NULL || psStrUnit == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psCtx = psStrCtx->psCtx;
    IMG_ASSERT(psCtx);
    if(!psCtx)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    switch (psStrUnit->eStrUnitType)
    {
        case VDECDD_STRUNIT_FWBSP_ENC_BSTR:
            ui32Result = fwbsp_bridge_ProcessEncryptedBitstream(psCtx,
                                                                psStrCtx,
                                                                psStrUnit);
            break;

        case VDECDD_STRUNIT_FENCE:
            ui32Result = fwbsp_bridge_ProcessFence(psStrCtx,
                                                   psStrUnit);
            break;

        case VDECDD_STRUNIT_FWBSP_STOP:
            ui32Result = fwbsp_bridge_ProcessStop(psStrCtx,
                                                  psStrUnit);
            break;

        case VDECDD_STRUNIT_FWBSP_UNSUPPORTED:
            ui32Result = fwbsp_bridge_ProcessUnsupported(psStrCtx,
                                                         psStrUnit);
            break;

        default:
            ui32Result = IMG_ERROR_FATAL;
            break;
    }

    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PictureAdd

 @Description

 This function stores addresses of picture and MB buffers into
 a database indexed by parse id. This database is used later on
 to fetch correct reference and colocated pictures addresses.

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_PictureAdd(
    IMG_HANDLE        hStrCtx,
    VDECDD_sStrUnit * psStrUnit,
    IMG_UINT32        ui32TransId,
    VDECDD_sPicture * psPicture
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    fwbsp_bridge_sPicture *psIntPicture;
    IMG_UINT32 ui32ParseId = psStrUnit->ui32ParseId;
    IMG_UINT32 ui32MergePid = psStrUnit->sFWBSPData.ui32MergePid;
    IMG_UINT32 ui32LumaAddr = 0, ui32ChromaAddr = 0, ui32MbParamAddr = 0;

    IMG_ASSERT(psStrCtx);
    IMG_ASSERT(psPicture);

    /* If there is a paired field, try to find addresses of buffers
     * associated with this paired field. Both fields are supposed
     * to be decoded into the same buffer. */
    if (ui32MergePid != 0)
    {
        FWBSP_BRIDGE_GetAddresses(hStrCtx,
                                  ui32MergePid,
                                  &ui32LumaAddr,
                                  &ui32ChromaAddr,
                                  &ui32MbParamAddr);
    }

    /* If we failed to obtain addresses of buffers used to store paired field,
     * use the buffers allocated for this picture. This also covers the
     * non-interlaced case. */
    if ((ui32LumaAddr == 0) || (ui32ChromaAddr == 0) || (ui32MbParamAddr == 0))
    {
        ui32LumaAddr =
            psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.ui32DevVirt +
            psPicture->sDisplayPictBuf.sRendInfo.asPlaneInfo[0].ui32Offset;

        ui32ChromaAddr =
            psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.ui32DevVirt +
            psPicture->sDisplayPictBuf.sRendInfo.asPlaneInfo[1].ui32Offset;

        ui32MbParamAddr =
            (psPicture->psPictResInt->psMBParamBuf == IMG_NULL) ? 0 :
            ((IMG_UINT32)GET_HOST_ADDR(
                &psPicture->psPictResInt->psMBParamBuf->sDdBufInfo));
    }

    /* Save addresses into the database. */
    psIntPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32ParseId, IMG_FALSE);
    IMG_ASSERT(psIntPicture);
    if (psIntPicture)
    {
        IMG_ASSERT(psIntPicture->psPictResInt);
        psIntPicture->ui32TransId = ui32TransId;

        /* Luma base address */
        psIntPicture->ui32LumaBaseAddr = ui32LumaAddr;

        /* Chroma base address */
        psIntPicture->ui32ChromaBaseAddr = ui32ChromaAddr;

#ifdef ENABLE_SEPARATE_COLOUR_PLANES
        /* Chroma2 base address */
        psIntPicture->ui32Chroma2BaseAddr =
            psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.ui32DevVirt +
            psPicture->sDisplayPictBuf.sRendInfo.asPlaneInfo[2].ui32Offset;
#endif /* ENABLE_SEPARATE_COLOUR_PLANES */

        /* MB params buffer address */
        psIntPicture->ui32MBParamsAddr = ui32MbParamAddr;

        return IMG_SUCCESS;
    }

    return IMG_ERROR_STORAGE_TYPE_FULL;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PicturePrepare

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_PicturePrepare(
    IMG_HANDLE        hStrCtx,
    VDECDD_sStrUnit  *psStrUnit,
    IMG_HANDLE        hDecPict
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    DECODER_sDecPict * psDecPict = (IMG_HANDLE)hDecPict;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx);
    IMG_ASSERT(psStrUnit);
    IMG_ASSERT(psDecPict);

    ui32Result = fwbsp_bridge_TranslatePIDTotTID(psStrCtx,
                                                 psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    ui32Result = fwbsp_bridge_TranslateTIDToBufAddr(psStrCtx,
                                                    psStrUnit,
                                                    psDecPict);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    /* Copy data required for picture management. */
    VDECDDUTILS_CopyDPBState(&psDecPict->sDPBState,
                             &psStrUnit->sFWBSPData.sDPBState);

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PictureRemove

*****************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_PictureRemove(
    IMG_HANDLE hStrCtx,
    IMG_UINT32 ui32Id,
    IMG_BOOL   bTransId
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    fwbsp_bridge_sPicture *psPicture;
    IMG_RESULT ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psStrCtx);
    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32Id, bTransId);
    IMG_ASSERT(psPicture);
    if(psPicture->psPictResInt == IMG_NULL)
    {
        /* No references, the picture can be safetly removed */
        ui32Result = fwbsp_bridge_PictureRemove(psStrCtx, ui32Id, bTransId);
    }
    else
    {
        /* Someone is keeping a reference, invalidate the picture,
           it will be removed on FWBSP_BUF_EMPTY message */
        psPicture->bValid = IMG_FALSE;
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PictureReturn

*****************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_PictureReturn(
    IMG_HANDLE hStrCtx,
    IMG_UINT32 ui32Id,
    IMG_BOOL   bTransId
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psStrCtx);

    ui32Result = fwbsp_bridge_PictureReturn(psStrCtx, ui32Id, bTransId);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_FwMsgBufFill

*****************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_FwMsgBufFill(
    IMG_HANDLE  hStrCtx,
    IMG_UINT32  ui32ParseId,
    IMG_UINT32 *pui32Msg,
    IMG_UINT16  ui16MsgSize
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    fwbsp_bridge_sFwMsgItem *psFwMsgItem;

    IMG_ASSERT(psStrCtx);
    IMG_ASSERT(pui32Msg);
    IMG_ASSERT(ui16MsgSize);

    FWBSP_BRIDGE_FOR_EACH(psFwMsgItem, &psStrCtx->psCtx->sFwMsgBufList)
    {
        if (!psFwMsgItem->bProcess && psFwMsgItem->ui32ParseId == ui32ParseId)
        {
            /* Copy incoming F/W message */
            IMG_MEMCPY(psFwMsgItem->aui32MsgBuf,
                       pui32Msg,
                       ui16MsgSize * sizeof(IMG_UINT32));
            psFwMsgItem->bProcess = IMG_TRUE;
            return IMG_SUCCESS;
        }
    }

    return IMG_ERROR_COULD_NOT_OBTAIN_RESOURCE;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_ProcessFwMsg

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_ProcessFwMsg(
    IMG_HANDLE hCtx
)
{
    fwbsp_bridge_sContext *psCtx = (fwbsp_bridge_sContext *)hCtx;
    fwbsp_bridge_sStrContext *psStrCtx = IMG_NULL;
    fwbsp_bridge_sFwMsgItem *psFwMsgItem;
    IMG_BOOL bRemovePic = IMG_FALSE;
    IMG_UINT32 ui32ParseId = 0;
    IMG_RESULT ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psCtx);

    FWBSP_BRIDGE_FOR_EACH(psFwMsgItem, &psCtx->sFwMsgBufList)
    {
        if (psFwMsgItem->bProcess)
        {
            IMG_UINT32 ui32StrId, *pui32Msg = psFwMsgItem->aui32MsgBuf;
            IMG_UINT16 ui16MsgSize;
            IMG_UINT8 ui8MsgType;

            ui32ParseId = psFwMsgItem->ui32ParseId;
            ui32StrId = FWBSP_BRIDGE_GET_STREAM_ID(ui32ParseId);
            psStrCtx = fwbsp_bridge_StreamCtxGet(psCtx, ui32StrId);
            IMG_ASSERT(psStrCtx);
            if (psStrCtx == IMG_NULL)
            {
                REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                       "Failed to extract stream context");
                return IMG_ERROR_FATAL;
            }

            ui8MsgType = MEMIO_READ_FIELD(pui32Msg, FWBSP_GENMSG_MSG_TYPE);
            ui16MsgSize = MEMIO_READ_FIELD(pui32Msg, FWBSP_NEW_SEQ_MSG_SIZE);

            IMG_ASSERT(ui16MsgSize < (sizeof(psFwMsgItem->aui32MsgBuf)/
                                      sizeof(psFwMsgItem->aui32MsgBuf[0])));

            DEBUG_REPORT(REPORT_MODULE_FWBSP_BRIDGE,
                   "[SID=0x%08x][PID=0x%08x] received FW message %d (%s)",
                   psStrCtx->ui32StrId, ui32ParseId, ui8MsgType,
                   fwbsp_bridge_GetMsgName(ui8MsgType));

            switch (ui8MsgType)
            {
                case FWBSP_NEW_SEQ:
                    ui32Result = fwbsp_bridge_ParseNewSeqMsg(psStrCtx,
                                                             pui32Msg);
                    if (ui32Result == IMG_SUCCESS)
                    {
                        ui32Result = fwbsp_bridge_SignalNewSequence(psStrCtx,
                            psStrCtx->sHostMsgData.ui8SeqHdrId);
                    }
                    break;

                case FWBSP_NEW_PIC:
                    ui32Result = fwbsp_bridge_ParseNewPicMsg(psStrCtx,
                                                             pui32Msg);
                    if (ui32Result == IMG_SUCCESS)
                    {
                        ui32Result = fwbsp_bridge_SignalNewPicture(psStrCtx,
                                                                   ui32ParseId);
                    }
                    break;

                case FWBSP_ERROR:
                    ui32Result = fwbsp_bridge_ParseErrMsg(psStrCtx,
                                                          pui32Msg);
                    if (ui32Result == IMG_SUCCESS)
                    {
                        IMG_UINT32 ui32ErrorFlags;
                        ui32ErrorFlags = psStrCtx->sHostMsgData.ui32ErrorFlags;
                        if (FWBSP_IS_UNSUPPORTED_CODE(ui32ErrorFlags))
                        {
                            ui32Result = fwbsp_bridge_SignalUnsupportedFeature(
                                psStrCtx, ui32ParseId, ui32ErrorFlags);
                        }
                        else
                        {
                            ui32Result = fwbsp_bridge_SignalFwError(
                                psStrCtx, ui32ParseId, ui32ErrorFlags);
                        }
                    }
                    break;

                case FWBSP_BUF_EMPTY:
                    {
                        ui32Result = fwbsp_bridge_SignalBufEmpty(psStrCtx,
                                                                 ui32ParseId,
                                                                 &bRemovePic);
                    }
                    break;

                default:
                    ui32Result = IMG_ERROR_FATAL;
                    break;
            }
            psFwMsgItem->bProcess = IMG_FALSE;

            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS || bRemovePic)
            {
                break;
            }
        }
    }

    /* Remove the picture when requested */
    if (bRemovePic)
    {
        VDECDD_sDdDevContext *psDdDevContext;

        IMG_ASSERT(psStrCtx);

        ui32Result = fwbsp_bridge_PictureRemove(psStrCtx, ui32ParseId, IMG_FALSE);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);

        psDdDevContext = (VDECDD_sDdDevContext *)psStrCtx->psCtx->pvCompInitUserData;
        ui32Result = CORE_DevSchedule(psDdDevContext);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_IsStreamIdle

******************************************************************************/
IMG_BOOL
FWBSP_BRIDGE_IsStreamIdle(
    IMG_HANDLE hStrCtx
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;

    /* Check input params. */
    IMG_ASSERT(psStrCtx);

    if (LST_empty(&psStrCtx->sEncBitstreamStrUnitList))
    {
        return IMG_TRUE;
    }
    else
    {
        VDECDD_sStrUnit *psStrUnit;
        FWBSP_BRIDGE_FOR_EACH(psStrUnit, &psStrCtx->sEncBitstreamStrUnitList)
        {
            if (psStrUnit->bDecode)
            {
                return IMG_FALSE;
            }
        }
    }

    return IMG_TRUE;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_IsStreamStopped

******************************************************************************/
IMG_BOOL
FWBSP_BRIDGE_IsStreamStopped(
    IMG_HANDLE hStrCtx
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;

    /* Check input params. */
    IMG_ASSERT(psStrCtx);

    return psStrCtx->eStrState == FWBSP_BRIDGE_STRSTATE_STOPPED;
}


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamPlay

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_StreamPlay(
    IMG_HANDLE hStrCtx,
    VDEC_eStopPoint eStopPoint
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;

    /* Check input params. */
    IMG_ASSERT(psStrCtx);
    if(psStrCtx == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psStrCtx->eStrState = FWBSP_BRIDGE_STRSTATE_PLAYING;

    if (eStopPoint == VDEC_STOPPOINT_SEQUENCE_START ||
        eStopPoint == VDEC_STOPPOINT_CLOSED_GOP)
    {
        psStrCtx->ui32GOPCnt = 0;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_GetAddresses

 @Description

 This function returns addresses of picture and MB param buffers stored in
 database for given parse id.

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_GetAddresses(
    IMG_HANDLE        hStrCtx,
    IMG_UINT32        ui32ParseId,
    IMG_UINT32      * pui32LumaAddr,
    IMG_UINT32      * pui32ChromaAddr,
    IMG_UINT32      * pui32MbParamAddr
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    fwbsp_bridge_sPicture *psPicture;

    if ((pui32LumaAddr == IMG_NULL) || (pui32ChromaAddr == IMG_NULL) ||
        (pui32MbParamAddr == IMG_NULL))
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psPicture = fwbsp_bridge_PictureGet(psStrCtx, ui32ParseId, IMG_FALSE);
    IMG_ASSERT(psPicture);
    if (psPicture)
    {
        *pui32LumaAddr = psPicture->ui32LumaBaseAddr;
        *pui32ChromaAddr = psPicture->ui32ChromaBaseAddr;
        *pui32MbParamAddr = psPicture->ui32MBParamsAddr;

        return IMG_SUCCESS;
    }

    return IMG_ERROR_INVALID_ID;
}

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_DiscardBitstreamUnits

 @Description

******************************************************************************/
IMG_RESULT
FWBSP_BRIDGE_DiscardBitstreamUnit(
    IMG_HANDLE        hStrCtx,
    IMG_UINT32        ui32PID
)
{
    fwbsp_bridge_sStrContext *psStrCtx = (fwbsp_bridge_sStrContext *)hStrCtx;
    VDECDD_sDdDevContext *psDdDevContext;
    VDECDD_sStrUnit *psStrUnit = IMG_NULL;
    IMG_RESULT ui32Result = IMG_SUCCESS;

    /* Find suspicious bit stream unit */
    psStrUnit = (VDECDD_sStrUnit *)LST_first(&psStrCtx->sEncBitstreamStrUnitList);
    while (psStrUnit)
    {
        if (psStrUnit->ui32ParseId == ui32PID || !ui32PID)
        {
            break;
        }

        psStrUnit = LST_next(psStrUnit);
    }

    if (psStrUnit == IMG_NULL)
    {
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    LST_remove(&psStrCtx->sEncBitstreamStrUnitList, psStrUnit);
    /* Trigger sending BSTRBUF_EMPTY event */
    ui32Result = psStrCtx->psCtx->pfnCompCallback(
        FWBSP_BRIDGE_CB_UNIT_PROCESSED,
        psStrCtx->psCtx->pvCompInitUserData,
        IMG_NULL,
        psStrCtx->pvUserData,
        psStrUnit);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    /* Check if unit have already a picture associated with it */
    if (!fwbsp_bridge_PictureIsValid(psStrCtx, psStrUnit->ui32ParseId))
    {
        /* Return the picture */
        ui32Result = fwbsp_bridge_PictureReturn(psStrCtx, psStrUnit->ui32ParseId, IMG_FALSE);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);

        ui32Result = fwbsp_bridge_PictureRemove(psStrCtx, psStrUnit->ui32ParseId, IMG_FALSE);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }
    else
    {
        ui32Result = VDECDDUTILS_FreeStrUnit(psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }
    psDdDevContext = (VDECDD_sDdDevContext *)psStrCtx->psCtx->pvCompInitUserData;
    ui32Result = CORE_DevSchedule(psDdDevContext);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    /* Inform hardware api that bitstream has been handled by the fw side.
     * Actually it caused the lockup, so we need to skip it. */
    {
        IMG_HANDLE hHwCtx;
        hHwCtx = DECODER_GethHwCtxHandle(psStrCtx->psCtx->hDecCtx);
        IMG_ASSERT(hHwCtx);
        if(!hHwCtx)
        {
            return IMG_ERROR_INVALID_PARAMETERS;
        }

        ui32Result = HWCTRL_BistreamProcessed(hHwCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_ProcessFwMsg

******************************************************************************/
extern IMG_UINT8
FWBSP_BRIDGE_GetStreamCount(
    IMG_HANDLE hCtx
)
{
    fwbsp_bridge_sContext *psCtx = (fwbsp_bridge_sContext *)hCtx;

    IMG_ASSERT(psCtx);
    if(psCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_FWBSP_BRIDGE, REPORT_ERR,
                "Invalid parameter");
        return 0;
    }

    return psCtx->ui8StrCnt;
}
