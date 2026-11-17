/*!
 *****************************************************************************
 *
 * @File       decoder.c
 * @Title      VDECDD Decoder Component
 * @Description    This file contains the VDECDD decoder component.
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

#include "decoder.h"

#include "vxd_ext.h"
#include "decoder_int.h"
#include "vxd.h"
#include "hwctrl_api.h"
#include "idgen_api.h"
#include "dbgopt_api_km.h"
#include "pool_api.h"
#include "vdecdd_mmu.h"
#include "resource.h"
#include "translation_api.h"
#include "dec_resources.h"
#include "dman_api_km.h"

#ifndef SYSBRG_BRIDGING
    #include "talmmu_api.h"
#else
    #include "sysmem_utils.h"
#endif

#include "vdecfw.h"
#ifdef HAS_AVS
#include "avsfw_data.h"
#endif /* HAS_AVS */
#ifdef HAS_H264
#include "h264fw_data.h"
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
#include "mpeg4fw_data.h"
#endif /* HAS_MPEG4 */
#ifdef HAS_VP6
#include "vp6fw_data.h"
#endif /* HAS_VP6 */
#ifdef HAS_VP8
#include "vp8fw_data.h"
#endif /* HAS_VP8 */
#ifdef HAS_VC1
#include "vc1fw_data.h"
#endif /* HAS_VC1 */
#ifdef HAS_JPEG
#include "jpegfw_data.h"
#endif /* HAS_JPEG */
#ifdef HAS_MPEG2
#include "mpeg2fw_data.h"
#endif /* HAS_MPEG2 */
#ifdef HAS_REAL
#include "realfw_data.h"
#endif /* HAS_REAL */
#ifdef HAS_HEVC
#include "hevcfw_data.h"
#endif /* HAS_HEVC */

#include "pixel_api.h"

#include "sysos_api_km.h"

#include "lst.h"
#include "dq.h"

#include "report_api.h"
#include "vdecfw_bin.h"

#if defined(CONFIG_X86)
#include <asm/cacheflush.h>
#endif

/*! Default number of slots (pictures) in each core per pipe. */
#define CORE_NUM_DECODE_SLOTS 2

/*! Default device watch timer period (in ms). */
#define CORE_DEFAULT_DWR_PERIOD 100         // Around 3 frames period @30FPS. This is good enough for production driver. Needs to be increase for c-sim using debug options.

/*! Default retry count of device watch timer for JPEG */
#define CORE_DEFAULT_DWR_JPEG_RETRY 5       // JPEGs can take longer to decode and there's no way to determine if HW is locked up.

/*! Default core FE HW timeout. */
#define CORE_DEFAULT_FE_TIMEOUT 120000      // 300x 400clock avg mb time.

/*! Default core BE HW timeout. */
#define CORE_DEFAULT_BE_TIMEOUT 0x3FFFC0 //0x7FFF80    // Maximum value.

/*! Default core PSR HW timeout. */
#define CORE_DEFAULT_PSR_TIMEOUT 0x3FFFF //0x7FFF80    // Maximum value.

/*! Maximum number of unique picture ids within stream. */
#define DECODER_MAX_PICT_ID             GET_STREAM_PICTURE_ID(((1ULL << 32) - 1ULL))

/*! Maximum number of concurrent pictures within stream. */
#define DECODER_MAX_CONCURRENT_PICT     0x100

#define GET_NEXT_PICTURE_ID(current_picture_id) \
    ((current_picture_id == FWIF_BIT_MASK(FWIF_NUMBITS_STREAM_PICTURE_ID)) ? 1 : current_picture_id + 1)

#define GET_PREV_PICTURE_ID(current_picture_id) \
    ((current_picture_id == 1) ? FWIF_BIT_MASK(FWIF_NUMBITS_STREAM_PICTURE_ID) : current_picture_id - 1)

#ifdef HAS_H264
#define H264_SGM_BUFFER_BYTES_PER_MB  1
#define H264_SGM_MAX_MBS              3600
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
#define MPEG4_FE_VRL_NOT_CODED  1088
#endif /* HAS_MPEG4 */
#ifdef HAS_VC1
#define VC1_MAX_HEIGHT_MBS  (256)   //4096/16 (MAX_WIDTH/MB_SIZE)
#define VC1_MAX_WIDTH_MBS   (256)   //4096/16 (MAX_WIDTH/MB_SIZE)
#endif /* HAS_VC1 */

#define CONTEXT_BUFF_SIZE   (72)

#if ((MAX_CONCURRENT_STREAMS) > FWIF_BIT_MASK(FWIF_NUMBITS_STREAM_ID))
#error Maximum number of concurrent streams (MAX_CONCURRENT_STREAMS) should be less than or equal to FWIF_BIT_MASK(FWIF_NUMBITS_STREAM_ID)
#endif


/*!
******************************************************************************
 This macro reports a Decoder warning once only.
******************************************************************************/
#define IMG_WARN_ONCE(...)                                              \
{                                                                       \
    static IMG_BOOL bDone = IMG_FALSE;                                  \
    if (!bDone)                                                         \
    {                                                                   \
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING, __VA_ARGS__);     \
        bDone = IMG_TRUE;                                               \
    }                                                                   \
}

#define DECODER_STR_CTX_TO_CORE_CTX(_strCtx_) (psDecStrCtx && psDecStrCtx->psDecCtx ? psDecStrCtx->psDecCtx->psDecCoreCtx : IMG_NULL)

/*!
******************************************************************************
 This union contains firmware contexts. Used to allocate buffers for firmware
 context.
 @brief  Decoder Context
******************************************************************************/
typedef union
{
#ifdef HAS_AVS
    AVSFW_sContextData   sAVSContext;
#endif /* HAS_AVS */
#ifdef HAS_H264
    H264FW_sContextData   sH264Context;
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
    MPEG4FW_sContextData  sMPEG4Context;
#endif /* HAS_MPEG4 */
#ifdef HAS_VP6
    VP6FW_sContextData  sVP6Context;
#endif /* HAS_VP6 */
#ifdef HAS_VP8
    VP8FW_sContextData  sVP8Context;
#endif /* HAS_VP8 */
#ifdef HAS_JPEG
    JPEGFW_sContextData  sJPEGContext;
#endif /* HAS_JPEG */
#ifdef HAS_REAL
    REALFW_sContextData  sREALContext;
#endif /* HAS_REAL */
#ifdef HAS_VC1
    VC1FW_sContextData   sVC1Context;
#endif
#ifdef HAS_MPEG2
    MPEG2FW_sContextData sMPEG2Context;
#endif /* HAS_MPEG2 */
#ifdef HAS_HEVC
    HEVCFW_sContextData  sHEVCContext;
#endif /* HAS_HEVC */
} DECODER_uFwContexts;

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************
 This enum defines Decoder firmware configuration.
 @brief  Decoder Firmware Configuration
******************************************************************************/
typedef enum {
    eDCONF_FwNone,
    eDCONF_FwNonSecure,
    eDCONF_FwSecure,
} DECODER_eFwConf;
#endif

struct DECODER_sCoreContext_tag;
/*!
******************************************************************************
 This structure contains the Decoder Context.
 @brief  Decoder Context
******************************************************************************/
typedef struct
{
    // Configuration
    IMG_BOOL                    bInitialised;                       /*!< Indicates whether component is initialised.                */
    VDECDD_pfnCompCallback      pfnCompCallback;                    /*!< Callback from component to return output.                  */
    IMG_VOID *                  pvCompInitUserData;                 /*!< Callback user data from component.                         */

    const VDECDD_sDdDevConfig * psDevConfig;                        /*!< Device configuration.                                      */
    IMG_UINT32                  ui32NumPipes;                       /*!< Number of pipes per core.                                 */

    struct DECODER_sCoreContext_tag * psDecCoreCtx;
    LST_T                       sStreamList;                        /*!< List of stream context structures.                         */
#ifdef VDEC_USE_PVDEC_SEC
    DECODER_eFwConf                   eDecFwConf;                         /*!< Indicates actual decoder firmware configuration            */
#endif

    // Handles
    IMG_HANDLE                  hMmuDevHandle;                      /*!< VDECDDMMU Device context.                                  */
    IMG_HANDLE                  hStrIdGen;                          /*!< Stream-based ID generator handle for use with firmware.    */
    IMG_HANDLE                  hDevHandle;                         /*!< Device handle.                                             */

    VXDIO_sDdBufInfo            sPtdBufInfo;                        /*!< Buffer to hold device (index 0) and stream (indexes 1-)
                                                                         PTDs.                                                      */
    VXDIO_sPtdInfo              sPtdInfo;                           /*!< Base address of PTD related informations.                  */

    IMG_BOOL                    abSupportedStandards[VDEC_STD_MAX]; /*!< Defines if particular standard is supported.               */
    VXD_sFirmware                     sFirmware;

    SYS_eMemPool                eSecurePool;
    SYS_eMemPool                eInsecurePool;

    // Concurrent streams
    IMG_UINT32                  ui32StrCnt;

} DECODER_sContext;



/*!
******************************************************************************
 This structure contains the stream context.
 @brief  Decoder Stream Context
******************************************************************************/
typedef struct
{
    LST_LINK;                                           /*!< List link (allows the structure to be part of a MeOS list).    */

    // Configuration.
    IMG_UINT32              ui32StrId;                  /*!< Stream Id used with firmware.                                  */
    IMG_UINT8               ui8Pipe;                    /*!< Pipe which stream will be decoded on, may change during playback */
    VDEC_sStrConfigData     sConfig;                    /*!< Global configuration.                                          */
    DECODER_sContext *      psDecCtx;                   /*!< Pointer to Decoder context.                                    */
    IMG_VOID *              pvUserData;                 /*!< Callback user data from component.                             */
    IMG_VOID *              pvUserIntData;              /*!< User data associated with interrupt handling.                  */

    // Handles
    IMG_HANDLE              hMmuStrHandle;              /*!< VDECDDMMU stream context.                                        */
    IMG_HANDLE              hPictIdGen;                 /*!< Picture-based ID generator handle for use with firmware.       */
#ifdef VDEC_USE_PVDEC_SEC
    IMG_HANDLE              hFWBSP_BRIDGEStrCtx;        /*!< FWBSP_BRIDGE stream context handle                             */
#endif

    // Lists: in-flight data/resources
    LST_T                   sPendStrUnitList;           /*!< List of pictures that are pending decode complete (in-flight).                         */
    DQ_T                    sStrDecdPictList;           /*!< Doubly linked-list of decoded pictures, held by the firmware.                          */

    // Resources
    IMG_UINT32              ui32NumRefRes;              /*!< Maximum number of active pictures in stream.                                           */
    LST_T                   sRefResList;                /*!< List of buffer control structures.                                                     */
    IMG_UINT32              ui32NumDecRes;              /*!< Maximum number of simultaneously decoding pictures.                                    */
    LST_T                   sDecResList;                /*!< List of decode resources allocated per core/stream.                                    */
    IMG_UINT32              ui32AvailPipes;             /*!< Pipes available for decoding current picture (not HW related, may change during playback) */

    // State (decoding)
    VDECDD_sDecStrStatus    sDecStrStatus;              /*!< Stream status.                                                                         */

    VXDIO_sDdBufInfo        sPvdecFwCtxBuf;             /*!< Additional PVDEC FW context buffer.                                                    */

    IMG_UINT32              ui32LastFeTransactionId;    /*!< Transaction ID of last picture on stream.                                              */
    IMG_UINT32              ui32NextDecPictId;          /*!< Serial number (incrementing) of next decoded picture (order) which needs processing .  */

    IMG_UINT32              ui32NextPictIdExpected;     /*!< Serial number of the next picture expected to come up (from fw) as decoded */

    DECODER_sPictDecRes *   psCurFePictDecRes;          /*!< Pointer to device resource of current front-end picture.                                */
    DECODER_sPictDecRes *   psPrevFePictDecRes;         /*!< Pointer to device resource of previous front-end picture.                               */
    DECODER_sPictDecRes *   psLastBePictDecRes;         /*!< Pointer to device resource of newest decoded back-end picture
                                                             (in decode order).                                                                      */
#ifdef VDEC_USE_PVDEC_SEC
    VDECDD_sDPBState        sLastDPBState;              /*!< State of firmware DPB, used to display pictures during stream flush.                    */
#endif /* VDEC_USEC_PVDEC_SEC */

    DECODER_sDecPict *      psCurrentPicture;           /*!< Pointer to current front-end picture being processed.                                   */

} DECODER_sStrContext;

#ifndef IMG_KERNEL_MODULE
/*!
******************************************************************************
 This structure contains the resources that was used by the last picture sent
 for decode. It is used for pdumping. It enables the driver to add an extra lock
 between two pictures when they share a resource.
******************************************************************************/
typedef struct
{
    IMG_BOOL        bInit;
    IMG_HANDLE      hTransactionInfo;
    IMG_HANDLE      hHdrInfo;
    IMG_HANDLE      hReconPict;
    IMG_HANDLE      hAltPict;
    IMG_HANDLE      hBatchMsgInfo;
    IMG_HANDLE      hPictHdrInfo;
    IMG_HANDLE      hPictRefRes;
    IMG_HANDLE      hCurPictDecRes;
    IMG_HANDLE      hPrevPictDecRes;

} DECODER_sLastResSubmitted;

#endif
/*!
******************************************************************************
 This structure contains the core context.
 @brief  Decoder Core Context
******************************************************************************/
typedef struct DECODER_sCoreContext_tag
{
    LST_LINK;                                           /*!< List link (allows the structure to be part of a MeOS list).    */

    DECODER_sContext          * psDecCtx;                   /*!< Pointer to Decoder context.                                    */

    IMG_BOOL                    bEnumerated;                /*!< Indicates whether the core has been enumerated.              */
    IMG_BOOL                    bMaster;                    /*!< Core is master (controls clocks).                            */
    IMG_BOOL                    bConfigured;                /*!< Core is configured.                                          */

    IMG_UINT32                  ui32CoreFeatures;           /*!< Core feature flags - see #VDECDD_eCoreFeatureFlags.          */
    IMG_UINT32                  aui32PipeFeatures[VDEC_MAX_PIXEL_PIPES];  /*!< Core feature flags - see #VDECDD_eCoreFeatureFlags.          */
    VXD_sCoreProps              sCoreProps;

    IMG_HANDLE                  hResources;                 /*!< Core resources.                                              */
    IMG_HANDLE                  hHwCtx;                     /*!< Hardware control context.                                    */
    HWCTRL_sRendec              sRendec;                    /*!< Rendec properties                                            */
    VXDIO_sDdBufInfo            asRendecBufInfo[2];         /*!< Buffer contaning rendec properties                           */

    IMG_UINT32                  ui32CumulativePics;         /*!<                                                              */

    IMG_BOOL                    bAPM;                       /*!< Indicates whether Active Power Management is enabled
                                                               for the device. This turns off power to the core when
                                                               there is nothing to decode.                                    */
    IMG_BOOL                    bBusy;                      /*!< Core busy flag, TRUE when end bytes are expected             */

#ifndef IMG_KERNEL_MODULE
    DECODER_sLastResSubmitted   sLasRes;
#endif

} DECODER_sCoreContext;


/*!
******************************************************************************
 This structure defines the decode picture reference.
 @brief  Decoder Picture Reference
******************************************************************************/
typedef struct
{
    LST_LINK;                                   /*!< List link (allows the structure to be part of a MeOS list).    */

    // These are only required to remove pictures from
    // core when stream is destroyed
    DECODER_sDecPict        * psDecPict;        /*!< Pointer to decoding picture                                    */
 //   DECODER_sCoreContext    * psDecCoreCtx;     /*!< Pointer to core where stream unit is decoded.                  */

    VDECDD_sStrUnit         * psStrUnit;        /*!< Pointer to stream unit.                                        */

} DECODER_sDecStrUnit;


/*!
******************************************************************************
 This structure defines the decoded picture.
 @brief  Decoded Picture
******************************************************************************/
typedef struct
{
    DQ_LINK;                /*!< Context is DQ listable. Pictures can be decoded on multiple cores thus may arrive out of order.    */

    IMG_UINT32              ui32TransactionId;

    IMG_BOOL                bProcessed;         /*!< Flag to indicate whether decoded picture has been processed.                   */
    IMG_BOOL                bProcessFailed;     /*!< Defaul: IMG_FALSE, set to IMG_TRUE if we failed to complete the processing     */
    IMG_BOOL                bForceDisplay;      /*!< Force displaying the picture due to processing errors.                         */
    IMG_UINT32              ui32DisplayIdx;     /*!< Index within buffer control of next display picture.                           */
    IMG_UINT32              ui32ReleaseIdx;     /*!< Index within buffer control of next release picture.                           */

    VDECDD_sPicture       * psPicture;          /*!< Pointer to decoded picture.                                                    */
    DECODER_sFwMessage    * psFirstFieldFwMsg;  /*!< Pointer to firmware decoded information.                                       */
    DECODER_sFwMessage    * psSecondFieldFwMsg; /*!< Pointer to firmware decoded information.                                       */
    DECODER_sPictRefRes   * psPictRefRes;       /*!< Pointer to stream resource used for picture.                                   */

} DECODER_sDecodedPict;


/*!
******************************************************************************
 This array defines the names for each decode level.
 NOTE: these should be kept in sync with values in #VDECDD_eDecodeLevel.
******************************************************************************/
IMG_CHAR * apszDecodeLevels[] =
{
    "scheduler [parse only]",
    "decoder [fake hw]",
    "firmware",
    "front-end hardware",
    "all",
};

static IMG_RESULT
decoder_GetFirmware(
    IMG_HANDLE                  hMmuDevHandle,
    IMG_HANDLE                  hDevHandle,
    VXDIO_sMemPool              sMemPool,
    IMG_BOOL                    bSecure,
    VXD_sFirmware             * psFirmware
);

static IMG_RESULT
decoder_DestroyFirmware(
    VXD_sFirmware   * psFirmware,
    IMG_HANDLE        hDevHandle
);

/*!
******************************************************************************

 @Function              decoder_UpdateAvailPipes

******************************************************************************/
static IMG_RESULT
decoder_UpdateAvailPipes(
    DECODER_sStrContext * psDecStrCtx)
{
    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psDecStrCtx && psDecStrCtx->psDecCtx);

    switch (psDecStrCtx->sConfig.eVidStd)
    {
#ifdef HAS_H264
        case VDEC_STD_H264:
            {
                IMG_UINT32 ui32NumPipes = psDecStrCtx->psDecCtx->ui32NumPipes;
                psDecStrCtx->ui32AvailPipes  = 0;

                //H264 following picture can be decoded only on current...
                psDecStrCtx->ui32AvailPipes = (1 << (psDecStrCtx->ui8Pipe-1));
                // ...or following pipe (may wrap)
                psDecStrCtx->ui32AvailPipes |= (psDecStrCtx->ui8Pipe == ui32NumPipes ? 1 : (1 << (psDecStrCtx->ui8Pipe)));
                break;
            }
#endif
        default:
            {
                break;
            }
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_InitAvailPipes

******************************************************************************/
static IMG_RESULT
decoder_InitAvailPipes(
    DECODER_sStrContext * psDecStrCtx)
{
    IMG_ASSERT(psDecStrCtx);

    switch (psDecStrCtx->sConfig.eVidStd)
    {
#ifdef HAS_HEVC
        case VDEC_STD_HEVC:
            {
                //only first pipe can be master when decoding HEVC in multipipe mode (FW restriction)
                psDecStrCtx->ui32AvailPipes = 1;
                break;
            }
#endif
        default:
            {
                //all pipes by default
                psDecStrCtx->ui32AvailPipes = ~0;
                break;
            }
    }


    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_SetFeatureFlags

 @Description

 This function sets the features flags from the core properties.

 @Input     p                     : Pointer to core properties.

 @Input     pui32CoreFeatureFlags : Pointer to core feature flags word.

******************************************************************************/
static IMG_VOID
decoder_SetFeatureFlags(
    VXD_sCoreProps      * psCoreProps,
    IMG_UINT32          * pui32CoreFeatureFlags,
    IMG_UINT32          * aui32PipeFeatureFlags
)
{
    IMG_UINT8 ui8PipeMinus1;

    IMG_ASSERT(psCoreProps);
    IMG_ASSERT(pui32CoreFeatureFlags);
    IMG_ASSERT(aui32PipeFeatureFlags);

    for(ui8PipeMinus1 = 0; ui8PipeMinus1 < psCoreProps->ui32NumPixelPipes; ui8PipeMinus1++)
    {
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abMpeg2[ui8PipeMinus1]) ? VDECDD_COREFEATURE_MPEG2 : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abMpeg4[ui8PipeMinus1]) ? VDECDD_COREFEATURE_MPEG4 : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abH264[ui8PipeMinus1])  ? VDECDD_COREFEATURE_H264  : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abVc1[ui8PipeMinus1])   ? VDECDD_COREFEATURE_VC1   : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abAvs[ui8PipeMinus1])   ? VDECDD_COREFEATURE_AVS   : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abReal[ui8PipeMinus1])  ? VDECDD_COREFEATURE_REAL  : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abJpeg[ui8PipeMinus1])  ? VDECDD_COREFEATURE_JPEG  : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abVp6[ui8PipeMinus1])   ? VDECDD_COREFEATURE_VP6   : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abVp8[ui8PipeMinus1])   ? VDECDD_COREFEATURE_VP8   : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abHEVC[ui8PipeMinus1])  ? VDECDD_COREFEATURE_HEVC  : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->bHdSupport)       ? VDECDD_COREFEATURE_HD_DECODE         : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abRotationSupport[ui8PipeMinus1]) ? VDECDD_COREFEATURE_ROTATION          : 0;
        *pui32CoreFeatureFlags |= aui32PipeFeatureFlags[ui8PipeMinus1] |= (psCoreProps->abScalingSupport[ui8PipeMinus1])  ? VDECDD_COREFEATURE_SCALING           : 0;
    }
}

#if defined(ERROR_CONCEALMENT) && defined(VDEC_USE_PVDEC)
/*!
******************************************************************************

 @Function              decoder_GetPictProcessingInfo

******************************************************************************/
static IMG_BOOL
decoder_GetPictProcessingInfo(
    DECODER_sCoreContext  * psDecCoreCtx,
    BSPP_sPictHdrInfo     * psPictHdrInfo,
    DECODER_sDecodedPict  * psDecodedPict,
    IMG_UINT32            * pui32PictLastMb
)
{
    IMG_RESULT ui32Result = IMG_SUCCESS;
    IMG_UINT8  ui8PipeMinus1;
    HWCTRL_sState sLastState;
    IMG_UINT32 ui32WidthInMb;
    IMG_UINT32 ui32HeightInMb;
    IMG_UINT32 ui32i;

    IMG_MEMSET(&sLastState, 0, sizeof(sLastState));

    IMG_ASSERT(psPictHdrInfo != IMG_NULL);
    ui32WidthInMb = (psPictHdrInfo->sCodedFrameSize.ui32Width +
                        (VDEC_MB_DIMENSION - 1)) / VDEC_MB_DIMENSION;
    ui32HeightInMb = (psPictHdrInfo->sCodedFrameSize.ui32Height +
                        (VDEC_MB_DIMENSION - 1)) / VDEC_MB_DIMENSION;

    IMG_ASSERT(pui32PictLastMb != IMG_NULL);
    *pui32PictLastMb = ui32WidthInMb * ui32HeightInMb;

    IMG_ASSERT(psDecodedPict != IMG_NULL);
    if (psDecodedPict->psFirstFieldFwMsg->sPictAttrs.bDWRFired ||
        psDecodedPict->psSecondFieldFwMsg->sPictAttrs.bDWRFired ||
        psDecodedPict->psFirstFieldFwMsg->sPictAttrs.bMMUFault ||
        psDecodedPict->psSecondFieldFwMsg->sPictAttrs.bMMUFault)
    {
        DECODER_sPictAttrs * psPictAttrs = &psDecodedPict->psFirstFieldFwMsg->sPictAttrs;

        IMG_ASSERT(psDecCoreCtx != IMG_NULL);
        /* Obtain the last available core status - cached before clocks where switched off */
        ui32Result = HWCTRL_GetCoreCachedStatus(psDecCoreCtx->hHwCtx,
                                                &sLastState);
        if (ui32Result != IMG_SUCCESS)
        {
            return IMG_FALSE;
        }

        /* Try to determine pipe where the last picture was decoded on (BE) */
        for(ui8PipeMinus1 = 0; ui8PipeMinus1 < VDEC_MAX_PIXEL_PIPES; ui8PipeMinus1++)
        {
            for (ui32i = VDECFW_CHECKPOINT_BE_END; ui32i >= VDECFW_CHECKPOINT_BE_START; ui32i--)
            {
                VXD_sPipeState * psPipeState = &sLastState.sCoreState.sFwState.asPipeState[ui8PipeMinus1];
                if (psPipeState->aui32CheckPoint[ui32i] == psDecodedPict->ui32TransactionId)
                {
                    /* Calculate last macroblock number processed on BE */
                    IMG_UINT32 ui32NumMbProcessed = (psPipeState->sBeMb.ui32Y * ui32WidthInMb) +
                                                        psPipeState->sBeMb.ui32X;
                    /* Sanity check, as HW may signal MbYX position beyond picture for corrupted streams */
                    if (ui32NumMbProcessed > (*pui32PictLastMb))
                    {
                        ui32NumMbProcessed = (*pui32PictLastMb); // trim
                    }

                    /* Copy info */
                    psPictAttrs->ui32MbsDropped = psPipeState->ui32BeMbsDropped;
                    psPictAttrs->ui32MbsDropped += ((*pui32PictLastMb) - ui32NumMbProcessed);
                    psPictAttrs->ui32MbsRecovered = psPipeState->ui32BeMbsRecovered;
                    psPictAttrs->ui32NoBEDWT = psPipeState->ui32BeErroredSlices;

                    return IMG_TRUE;
                }
            }

            /* If not found, we probbaly stuck on FE ? */
            for (ui32i = VDECFW_CHECKPOINT_FE_END; ui32i >= VDECFW_CHECKPOINT_FE_START; ui32i--)
            {
                VXD_sPipeState * psPipeState = &sLastState.sCoreState.sFwState.asPipeState[ui8PipeMinus1];
                if (psPipeState->aui32CheckPoint[ui32i] == psDecodedPict->ui32TransactionId)
                {
                    /* Mark all MBs as dropped */
                    psPictAttrs->ui32MbsDropped = *pui32PictLastMb;
                    psPictAttrs->ui32MbsRecovered = 0;

                    return IMG_TRUE;
                }
            }
        }

        return IMG_FALSE;
    }

    /* Picture was decoded without DWR, so we have already the required info */
    return IMG_TRUE;
}
#endif /* defined(ERROR_CONCEALMENT) && defined(VDEC_USE_PVDEC) */

/*!
******************************************************************************

 @Function              decoder_GetPipeAndLoad

******************************************************************************/
static IMG_RESULT
decoder_GetPipeAndLoad(
    DECODER_sStrContext   * psDecStrCtx,
    IMG_UINT32              ui32StreamFeature,
    IMG_UINT32            * pui32AvailLoad,
    IMG_UINT32            * pui32MinLoad,
    IMG_UINT8             * pui8MinLoadedPipe
)
{
    DECODER_sCoreContext * psDecCoreCtx;
    IMG_UINT32             ui32AvailLoad = -1;
    IMG_UINT32             ui32MinLoad = -1;
    IMG_RESULT             ui32Result;

    IMG_ASSERT(psDecStrCtx);
    psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);
    IMG_ASSERT(psDecCoreCtx);

    // Determine the main/available load on suitable cores.
    if ((ui32StreamFeature & psDecCoreCtx->ui32CoreFeatures) == ui32StreamFeature && !psDecCoreCtx->bBusy)
    {
        IMG_UINT32 aui32NumFreeSlots[VDEC_MAX_PIXEL_PIPES];
        IMG_UINT32 aui32PipeLoad[VDEC_MAX_PIXEL_PIPES];
        IMG_UINT8  ui8PipeMinus1 = 0;

        ui32Result = HWCTRL_GetCoreStatus(psDecCoreCtx->hHwCtx,
            aui32NumFreeSlots,
            aui32PipeLoad,
            IMG_NULL);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        for(ui8PipeMinus1 = 0; ui8PipeMinus1 < VDEC_MAX_PIXEL_PIPES;ui8PipeMinus1++)
        {
            if((((ui32StreamFeature & psDecCoreCtx->aui32PipeFeatures[ui8PipeMinus1])) == ui32StreamFeature) && (psDecStrCtx->ui32AvailPipes & ( 1 << ui8PipeMinus1)))
            {
                if (aui32PipeLoad[ui8PipeMinus1] < ui32MinLoad)
                {
                    ui32MinLoad = aui32PipeLoad[ui8PipeMinus1];
                }

                if (aui32NumFreeSlots[ui8PipeMinus1] &&
                    aui32PipeLoad[ui8PipeMinus1] < ui32AvailLoad)
                {
                    ui32AvailLoad = aui32PipeLoad[ui8PipeMinus1];

                    if(pui8MinLoadedPipe)
                    {
                        *pui8MinLoadedPipe = ui8PipeMinus1 + 1;
                    }
                }
            }
        }
    }

    if (pui32MinLoad)
    {
        *pui32MinLoad = ui32MinLoad;
    }
    if (pui32AvailLoad)
    {
        *pui32AvailLoad = ui32AvailLoad;
    }

    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              decoder_PictureDestroy

 @Description

 Free the picture container and optionally release image buffer back to client.
 Default is to decrement the reference count held by this picture.

******************************************************************************/
static IMG_RESULT
decoder_PictureDestroy(
    DECODER_sStrContext * psDecStrCtx,
    IMG_UINT32 ui32PictId,
    IMG_BOOL bReleaseImage
)
{
    VDECDD_sPicture * psPicture;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psDecStrCtx);

    ui32Result = IDGEN_GetHandle(psDecStrCtx->hPictIdGen, ui32PictId, (IMG_VOID **)&psPicture);
    if (ui32Result == IMG_SUCCESS)
    {
        IMG_ASSERT(psPicture);

        ui32Result = IDGEN_FreeId(psDecStrCtx->hPictIdGen, ui32PictId);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        if (psPicture->psDecPictInfo)
        {
            // Destroy the picture
            IMG_FREE(psPicture->psDecPictInfo);
            psPicture->psDecPictInfo = IMG_NULL;
        }

        // Return unused picture and internal resources
        if (psPicture->sDisplayPictBuf.psPictBuf)
        {
            if (bReleaseImage)
            {
                RESOURCE_ItemRelease(&psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.ui32RefCount);
            }
            else
            {
                RESOURCE_ItemReturn(&psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.ui32RefCount);
            }
            VDEC_BZERO(&psPicture->sDisplayPictBuf);
        }

        if (psPicture->psPictResInt)
        {
            RESOURCE_ItemReturn(&psPicture->psPictResInt->ui32RefCount);
            psPicture->psPictResInt = IMG_NULL;
        }

        IMG_FREE(psPicture);
        psPicture = IMG_NULL;
    }
    else
    {
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        return ui32Result;
    }

    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              decoder_DecodedPictureDestroy

******************************************************************************/
static IMG_RESULT
decoder_DecodedPictureDestroy(
    DECODER_sStrContext  * psDecStrCtx,
    DECODER_sDecodedPict * psDecodedPict,
    IMG_BOOL               bReleaseImageToClient
)
{
    IMG_RESULT ui32Result;

    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psDecodedPict);

    if (psDecodedPict->psPicture)
    {
        IMG_ASSERT(psDecodedPict->psPicture->ui32PictId == GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId));

        ui32Result = decoder_PictureDestroy(psDecStrCtx,
                                            psDecodedPict->psPicture->ui32PictId,
                                            bReleaseImageToClient);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psDecodedPict->psPicture = IMG_NULL;
    }

    DQ_remove(psDecodedPict); // DQ List: psDecStrCtx->sStrDecdPictList
    psDecStrCtx->sDecStrStatus.ui32NumPictDecoded--;

    RESOURCE_ItemReturn(&psDecodedPict->psPictRefRes->ui32RefCount);

    DEBUG_REPORT(REPORT_MODULE_DECODER,
        "[USERSID=0x%08X] [TID=0x%08X] COMPLETE",
        GET_STREAM_ID(psDecodedPict->ui32TransactionId),
        psDecodedPict->ui32TransactionId);

#ifdef VDEC_USE_PVDEC_SEC
    if (psDecStrCtx->sConfig.bSecureStream)
    {
        ui32Result = FWBSP_BRIDGE_PictureRemove(psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                                                psDecodedPict->ui32TransactionId,
                                                IMG_TRUE);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
#endif /* VDEC_USE_PVDEC_SEC */

    IMG_FREE(psDecodedPict->psFirstFieldFwMsg);
    psDecodedPict->psFirstFieldFwMsg = IMG_NULL;
    IMG_FREE(psDecodedPict->psSecondFieldFwMsg);
    psDecodedPict->psSecondFieldFwMsg = IMG_NULL;

    IMG_FREE(psDecodedPict);
    psDecodedPict = IMG_NULL;

    return IMG_SUCCESS;
}


#ifdef VDEC_MSVDX_HARDWARE

/*!
******************************************************************************

 @Function              decoder_GetDecodedPict

******************************************************************************/
static DECODER_sDecodedPict *
decoder_GetDecodedPict(
    IMG_UINT32      ui32TransactionId,
    DQ_T          * psDqList
)
{
    DECODER_sDecodedPict * psDecodedPict;
    IMG_VOID * pvItem = IMG_NULL;

    IMG_ASSERT(psDqList);

    psDecodedPict = DQ_first(psDqList);
    while (psDecodedPict)
    {
        if (psDecodedPict->ui32TransactionId == ui32TransactionId)
        {
            pvItem = psDecodedPict;
            break;
        }

        if (psDecodedPict != DQ_last(psDqList))
        {
            psDecodedPict = DQ_next(psDecodedPict);
        }
        else
        {
            psDecodedPict = IMG_NULL;
        }
    }

    return pvItem;
}


/*!
******************************************************************************

 @Function              decoder_GetDecodedPictOfStream

******************************************************************************/
static DECODER_sDecodedPict *
decoder_GetDecodedPictOfStream(
    IMG_UINT32      ui32PictId,
    DQ_T          * psDqList
)
{
    DECODER_sDecodedPict * psDecodedPict;
    IMG_VOID * pvItem = IMG_NULL;

    IMG_ASSERT(psDqList);

    psDecodedPict = DQ_first(psDqList);
    while (psDecodedPict)
    {
        if (GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId) == ui32PictId)
        {
            pvItem = psDecodedPict;
            break;
        }

        if (psDecodedPict != DQ_last(psDqList))
        {
            psDecodedPict = DQ_next(psDecodedPict);
        }
        else
        {
            psDecodedPict = IMG_NULL;
        }
    }

    return pvItem;
}

#endif


/*!
******************************************************************************

 @Function              decoder_PictureDisplay

******************************************************************************/
static IMG_RESULT
decoder_PictureDisplay(
    DECODER_sStrContext * psDecStrCtx,
    IMG_UINT32 ui32PictId,
    IMG_BOOL bLast
)
{
    VDECDD_sPicture * psPicture;
    IMG_RESULT        ui32Result;
    static IMG_UINT32 ui32DisplayNum = 0;

    IMG_ASSERT(psDecStrCtx);

    ui32Result = IDGEN_GetHandle(psDecStrCtx->hPictIdGen, ui32PictId, (IMG_VOID **)&psPicture);
    if (ui32Result == IMG_SUCCESS)
    {
        VDECDD_sDdBufMapInfo *  psPictBuf;

        IMG_ASSERT(psPicture);
        IMG_ASSERT(psPicture && psPicture->psDecPictInfo);

        psPictBuf = psPicture->sDisplayPictBuf.psPictBuf;
        IMG_ASSERT(psPictBuf);

        // Indicate whether there are more pictures coming for display.
        psPicture->psDecPictInfo->bLastInSequence = bLast;

        // Set decode order id
        psPicture->psDecPictInfo->ui32DecodeId = ui32PictId;

        // XXX-mbs: don't display pictures whose layout is different from the current output configuration.
        // this can happen when reference pictures span different configurations.

        // Return the picture to the client for display
        psDecStrCtx->sDecStrStatus.ui32TotalPictDisplayed++;
        RESOURCE_ItemUse(&psPictBuf->sDdBufInfo.ui32RefCount);
        if(psDecStrCtx->sConfig.bUpdateYUV)
        {
            // Update the image buffer content for dumping/diffing
            UPDATE_HOST((&psPictBuf->sDdBufInfo));
        }

        ui32DisplayNum++;
        DEBUG_REPORT(REPORT_MODULE_DECODER,
            "[USERSID=0x%08X] DISPLAY(%d): PIC_ID[%d]",
            psDecStrCtx->sConfig.ui32UserStrId,
            ui32DisplayNum,
            ui32PictId);

#if defined(CONFIG_X86)
        if(psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.pvCpuVirt)
        {
            clflush_cache_range(psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.pvCpuVirt, psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.ui32BufSize);
        }
#endif
#if 0
        {
            IMG_UINT32 i;
            IMG_UINT32 ui32CodedWidth;
            IMG_UINT32 ui32CodedHeight;
            static IMG_UINT32 ui32OutputNum = 0;
            static IMG_VOID * fpDump = IMG_NULL;
            static VDEC_sPictRendInfo sPictBufInfo;

            if (fpDump == IMG_NULL ||
                IMG_MEMCMP(&psPicture->sDisplayPictBuf.sPictBufInfo, &sPictBufInfo, sizeof(sPictBufInfo)))
            {
                IMG_CHAR acName[128];

                if (fpDump)
                {
                    fclose(fpDump);
                }
                else
                {
                    IMG_MEMSET(&sPictBufInfo, 0, sizeof(sPictBufInfo));
                }

                sprintf(acName,"dump_%d.res", ui32OutputNum++);

                fpDump = fopen(acName, "wb");
                IMG_ASSERT(fpDump);
            }

            if (psPicture->psDecPictInfo->sOutputConfig.eRotMode == VDEC_ROTATE_90 ||
                psPicture->psDecPictInfo->sOutputConfig.eRotMode == VDEC_ROTATE_270)
            {
                ui32CodedWidth = psPicture->sDisplayPictBuf.sPictBufInfo.sRendPictSize.ui32Height;
                ui32CodedHeight = psPicture->sDisplayPictBuf.sPictBufInfo.sRendPictSize.ui32Width;
            }
            else
            {
                ui32CodedWidth = psPicture->sDisplayPictBuf.sPictBufInfo.sRendPictSize.ui32Width;
                ui32CodedHeight = psPicture->sDisplayPictBuf.sPictBufInfo.sRendPictSize.ui32Height;
            }

            // Write luma.
            for (i = 0; i < ui32CodedHeight; i++)
            {
                fwrite((IMG_UINT8*)psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.pvCpuVirt +
                       (i * psPicture->sDisplayPictBuf.sPictBufInfo.asPlaneInfo[0].ui32Stride),
                       1,
                       psPicture->sDisplayPictBuf.sPictBufInfo.asPlaneInfo[0].ui32Stride,
                       fpDump);
            }

            // Write chroma.
            for (i = 0; i < (ui32CodedHeight+1)/2; i++)
            {
                fwrite((IMG_UINT8 * )psPicture->sDisplayPictBuf.psPictBuf->sDdBufInfo.pvCpuVirt +
                            psPicture->sDisplayPictBuf.sPictBufInfo.asPlaneInfo[1].ui32Offset +
                       (i * psPicture->sDisplayPictBuf.sPictBufInfo.asPlaneInfo[1].ui32Stride),
                       1,
                       psPicture->sDisplayPictBuf.sPictBufInfo.asPlaneInfo[1].ui32Stride,
                       fpDump);
            }

            sPictBufInfo = psPicture->sDisplayPictBuf.sPictBufInfo;
        }
#endif

        IMG_ASSERT(psDecStrCtx->psDecCtx);
        ui32Result = psDecStrCtx->psDecCtx->pfnCompCallback(
            DECODER_CB_PICTURE_DISPLAY,
            psDecStrCtx->psDecCtx->pvCompInitUserData,
            IMG_NULL,
            psDecStrCtx->pvUserData,
            psPicture);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
    else
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] ERROR: DISPLAY PICTURE HAS AN EXPIRED ID",
            psDecStrCtx->sConfig.ui32UserStrId);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_GetNextDecPictContiguous

******************************************************************************/
static DECODER_sDecodedPict *
decoder_GetNextDecPictContiguous(
    DECODER_sDecodedPict  * psDecodedPicture,
    IMG_UINT32              ui32NextDecPictId,
    DQ_T                  * psStrDecdPictList
)
{
    DECODER_sDecodedPict * psNextDecPict = IMG_NULL;
    DECODER_sDecodedPict * psResultDecPict = IMG_NULL;

    IMG_ASSERT(psStrDecdPictList);
    if(psStrDecdPictList == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter");
        return IMG_NULL;
    }

    if (psDecodedPicture)
    {
        if (psDecodedPicture != DQ_last(psStrDecdPictList))
        {
            psNextDecPict = DQ_next(psDecodedPicture);

            if (IMG_NULL != psNextDecPict->psPicture)
            {
                // If we have no holes in the decoded list (i.e. next
                // decoded picture is next in bitstream decode order).
                if (HAS_X_REACHED_Y(ui32NextDecPictId, psNextDecPict->psPicture->ui32PictId, 1 << FWIF_NUMBITS_STREAM_PICTURE_ID))
                {
                    psResultDecPict = psNextDecPict;
                }
            }
        }
    }

    return psResultDecPict;
}


/*!
******************************************************************************

 @Function              decoder_NextPicture

 @Description

 Returns the next unprocessed picture or IMG_NULL if the next picture is not
 next in bitstream decode order or there are no more decoded pictures in the
 list.

 @Input     psCurrentDecodedPicture : Pointer to current decoded picture.

 @Input     ui32NextDecPictId       : Picture ID of next picture in decode order.

 @Input     psStrDecdPictList       : Pointer to decoded picture list.

 @Return    DECODER_sDecodedPict *  : Pointer to next decoded picture to process.

******************************************************************************/
static DECODER_sDecodedPict *
decoder_NextPicture(
    DECODER_sDecodedPict  * psCurrentDecodedPicture,
    IMG_UINT32              ui32NextDecPictId,
    DQ_T                  * psStrDecdPictList
)
{
    DECODER_sDecodedPict* psReturn = IMG_NULL;

    IMG_ASSERT(psStrDecdPictList != IMG_NULL);
    if(psStrDecdPictList == IMG_NULL)
    {
        return IMG_NULL;
    }

    if (psCurrentDecodedPicture == IMG_NULL)
    {
        psCurrentDecodedPicture = DQ_first(psStrDecdPictList);
    }

    if (psCurrentDecodedPicture != IMG_NULL && !psCurrentDecodedPicture->bProcessFailed)
    {
        // Search for picture ID greater than picture in list
        do
        {
            if (!psCurrentDecodedPicture->bProcessed)
            {
                //Return the current one because it has not been processed
                psReturn = psCurrentDecodedPicture;
                break;
            }

            // Obtain a pointer to the next picture in bitstream decode order.
            psCurrentDecodedPicture = decoder_GetNextDecPictContiguous(psCurrentDecodedPicture, ui32NextDecPictId, psStrDecdPictList);
        }
        while (psCurrentDecodedPicture && !psCurrentDecodedPicture->bProcessFailed);
    }

    return psReturn;
}

/*!
******************************************************************************

 @Function              decoder_PictureDecoded

******************************************************************************/
static IMG_RESULT
decoder_PictureDecoded(
    DECODER_sStrContext   * psDecStrCtx,
    DECODER_sCoreContext * psDecCoreCtx,	
    VDECDD_sPicture       * psPicture,
    DECODER_sDecPict     * psDecPict,
    BSPP_sPictHdrInfo    * psPictHdrInfo,	
    VDECDD_sStrUnit      * psStrUnit
)
{
    DECODER_sFwMessage   * psFirstFieldFwMsg;
    DECODER_sFwMessage   * psSecondFieldFwMsg;
    DECODER_sPictRefRes  * psPictRefRes;
    IMG_UINT32             ui32TransactionId;
    DECODER_sDecodedPict * psDecodedPict;
    DECODER_sDecodedPict * psNextDecodedPict;
    VDECDD_sDdBufMapInfo * psPictBuf;
#ifdef VDEC_MSVDX_HARDWARE
    DECODER_sDecodedPict * psPrevDecodedPict;
    VDECFW_sBufferControl * psBufferControl;
#endif
    VDEC_sComSequHdrInfo  * psComSequHdrInfo;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psStrUnit);

    IMG_ASSERT(psDecPict);
    psFirstFieldFwMsg = psDecPict->psFirstFieldFwMsg;
    psSecondFieldFwMsg = psDecPict->psSecondFieldFwMsg;
    psPictRefRes = psDecPict->psPictRefRes;
    ui32TransactionId = psDecPict->ui32TransactionId;

    IMG_ASSERT(psPicture);
    psPictBuf = psPicture->sDisplayPictBuf.psPictBuf;
    IMG_ASSERT(psPictBuf);
    psComSequHdrInfo = &psPictBuf->psDdStrContext->sComSequHdrInfo;

    // Create a container for decoded picture.
    VDEC_MALLOC(psDecodedPict);
    IMG_ASSERT(psDecodedPict);
    if (psDecodedPict == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for decoded picture",
            psDecStrCtx->sConfig.ui32UserStrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psDecodedPict);

    psDecodedPict->psPicture = psPicture;
    psDecodedPict->psFirstFieldFwMsg = psFirstFieldFwMsg;
    psDecodedPict->psSecondFieldFwMsg = psSecondFieldFwMsg;
    psDecodedPict->psPictRefRes = psPictRefRes;
    psDecodedPict->ui32TransactionId = ui32TransactionId;

    //Updating the Buffer Control which contains information about pictures to display and release
    UPDATE_HOST(&psDecodedPict->psPictRefRes->sFwCtrlBuf);
    // Updating standard specific context.
    UPDATE_HOST(&psDecStrCtx->psCurFePictDecRes->sFwCtxBuf);

    // Populate the decoded picture information structure.
    psPicture->psDecPictInfo->ePictState = VDEC_PICT_STATE_DECODED;

    IMG_MEMCPY(&psPicture->psDecPictInfo->sFirstFieldTagContainer.sPictHwCrc, &psFirstFieldFwMsg->sPictHwCrc, sizeof(psPicture->psDecPictInfo->sFirstFieldTagContainer.sPictHwCrc));
    IMG_MEMCPY(&psPicture->psDecPictInfo->sSecondFieldTagContainer.sPictHwCrc, &psSecondFieldFwMsg->sPictHwCrc, sizeof(psPicture->psDecPictInfo->sSecondFieldTagContainer.sPictHwCrc));

#ifdef VDEC_MSVDX_HARDWARE
    psBufferControl = (VDECFW_sBufferControl*)psDecodedPict->psPictRefRes->sFwCtrlBuf.pvCpuVirt;
    if (psBufferControl->b2ndFieldOfPair)
    {
        // Search the first field anc fill the sSecondFieldTagContainer
        IMG_UINT32 ui32PrevDecPictId = (GET_PREV_PICTURE_ID(GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId)));
        psPrevDecodedPict = decoder_GetDecodedPictOfStream(ui32PrevDecPictId,&psDecStrCtx->sStrDecdPictList);
        if(psPrevDecodedPict != IMG_NULL)
        {
            IMG_MEMCPY(&psPicture->psDecPictInfo->sSecondFieldTagContainer.sPictHwCrc, &psPrevDecodedPict->psFirstFieldFwMsg->sPictHwCrc, sizeof(psPicture->psDecPictInfo->sSecondFieldTagContainer.sPictHwCrc));
        }
        else
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_ERR, "[USERSID=0x%08X] [TID 0x%08X] Failed to find decoded picture to attatch sSecondFieldTagContainer",
                psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        }
        psPrevDecodedPict = IMG_NULL;
    }
#endif

    // Report any issues in decoding
    if (psDecodedPict->psPicture->psDecPictInfo->ui32ErrorFlags )
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [PID=0x%08X] BSPP reported errors [flags: 0x%08X]",
            psDecStrCtx->sConfig.ui32UserStrId,
            psDecodedPict->psPicture->ui32PictId,
            psDecodedPict->psPicture->psDecPictInfo->ui32ErrorFlags);
    }

    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_PSRWDT)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_PSRWDT)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Firmware parsing timed-out.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_PSR_TIMEOUT;
    }
    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_ENTDECERROR)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_ENTDECERROR)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Front-end HW processing terminated prematurely due to an error.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_FEHW_DECODE;
    }
    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_SRERROR)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_SRERROR)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] HW Shift Register access returned an error during FEWH parsing.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_SR_ERROR;
    }
    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_SRERROR_FW_PARSING)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_SRERROR_FW_PARSING)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] HW Shift Register access returned an error during firmware header parsing.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_PSR_SR_ERROR;
    }
    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_FW_PARSING_SEQ_ERROR)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_FW_PARSING_SEQ_ERROR)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Firmware parser failed to  follow all the required processing steps.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_PSR_PROCESS_NOT_NORMAL;
    }
    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_HWWDT)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_FEERROR_HWWDT)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Front-end HW processing timed-out.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_FEHW_TIMEOUT;
    }

    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_MISSING_REFERENCES)) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32FEError & FLAG_MASK(VDECFW_MSGFLAG_DECODED_MISSING_REFERENCES)))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] There are missing references for the current frame.May have corruption",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);

        // This is not a serious error, indicate host app to drop the frame as may have corruption.
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_MISSING_REFERENCES;
    }

    if (psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32NoBEDWT > 0)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Back-end HW processing timed-out. Aborted slices %d",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId,
            psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32NoBEDWT);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_BEHW_TIMEOUT;
    }
    if (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32NoBEDWT > 0)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Back-end HW processing timed-out. Aborted slices %d",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId,
            psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32NoBEDWT);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_BEHW_TIMEOUT;
    }

#if defined(ERROR_CONCEALMENT) && defined(VDEC_USE_PVDEC)
    /* Estimate error level in percentage */
    {
        IMG_UINT32 ui32FirstFieldErrLevel = 0;
        IMG_UINT32 ui32SecondFieldErrLevel = 0;
        IMG_UINT32 ui32PictLastMb = 0;

        if ((decoder_GetPictProcessingInfo(psDecCoreCtx,
                                           psPictHdrInfo,
                                           psDecodedPict,
                                           &ui32PictLastMb) == IMG_TRUE))
        {
#define GET_ERR_LEVEL(_result_, _lastmb_, _dropped_, _recovered_) \
            { \
               IMG_UINT32 ui32Ratio = \
                   (100 * (_lastmb_ - _dropped_ + _recovered_)) / _lastmb_; \
               _result_ = 100 - ui32Ratio; \
            }

            if (ui32PictLastMb)
            {
                GET_ERR_LEVEL(ui32FirstFieldErrLevel,
                      ui32PictLastMb,
                      psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32MbsDropped,
                      psDecodedPict->psFirstFieldFwMsg->sPictAttrs.ui32MbsRecovered);

                GET_ERR_LEVEL(ui32SecondFieldErrLevel,
                      ui32PictLastMb,
                      psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32MbsDropped,
                      psDecodedPict->psSecondFieldFwMsg->sPictAttrs.ui32MbsRecovered);
            }
#undef GET_ERR_LEVEL

            /* does not work properly with discontinous mbs */
            if(psPictHdrInfo->bDiscontinuousMbs == IMG_FALSE)
            {
                psPicture->psDecPictInfo->ui32ErrorLevel =
                    ui32FirstFieldErrLevel > ui32SecondFieldErrLevel ?
                    ui32FirstFieldErrLevel : ui32SecondFieldErrLevel;
            }

            IMG_ASSERT(psPicture->psDecPictInfo->ui32ErrorLevel <= 100);
            if (psPicture->psDecPictInfo->ui32ErrorLevel)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
                    "[USERSID=0x%08X] [TID 0x%08X] Picture error level: %d(%%)",
                    psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId,
                    psPicture->psDecPictInfo->ui32ErrorLevel);
            }
        }
    }

#endif /* defined(ERROR_CONCEALMENT) && defined(VDEC_USE_PVDEC) */

    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.bDWRFired) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.bDWRFired))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] VXD Device Reset (Lockup).",
            psDecStrCtx->sConfig.ui32UserStrId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_SERVICE_TIMER_EXPIRY;
    }

    if ((psDecodedPict->psFirstFieldFwMsg->sPictAttrs.bMMUFault) ||
        (psDecodedPict->psSecondFieldFwMsg->sPictAttrs.bMMUFault))
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] VXD Device Reset (MMU fault).",
            psDecStrCtx->sConfig.ui32UserStrId);
        psPicture->psDecPictInfo->ui32ErrorFlags |= VDEC_ERROR_MMU_FAULT;
    }

#if defined(VDEC_MSVDX_HARDWARE) && defined(VDEC_USE_PVDEC)
    if (!(((IMG_BUFFERTYPE_PAIR == psBufferControl->eDecPictType) &&
           VDECFW_PICMGMT_FIELD_CODED_PICTURE_EXECUTED(psBufferControl->ui8PictMgmtFlags))
          ||
          FLAG_IS_SET(psBufferControl->ui8PictMgmtFlags, VDECFW_PICMGMTFLAG_PICTURE_EXECUTED))
          && !psDecStrCtx->sConfig.bSecureStream)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "[USERSID=0x%08X] [TID 0x%08X] Picture management was not executed for this picture; forcing display.",
            psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
        psDecodedPict->bForceDisplay = IMG_TRUE;
    }
#endif

    psDecStrCtx->sDecStrStatus.ui32TotalPictFinished++;

    //Use NextPictIdExpected to do this check. ui32NextPictId could be different
    //from what expected at this point because we failed to process a picture the last
    //time run this function (this is still an error (unless doing multi-core) but not
    //the error reported here.
    if (psPicture->ui32PictId != psDecStrCtx->ui32NextPictIdExpected)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] ERROR: MISSING DECODED PICTURE (%d)",
            psDecStrCtx->sConfig.ui32UserStrId,
            psDecStrCtx->ui32NextDecPictId);
    }

    psDecStrCtx->ui32NextDecPictId = GET_NEXT_PICTURE_ID(GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId));
    psDecStrCtx->ui32NextPictIdExpected = psDecStrCtx->ui32NextDecPictId;

    // Add the picture itself to the decoded list
    psNextDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    while (psNextDecodedPict &&
           !HAS_X_REACHED_Y(GET_STREAM_PICTURE_ID(psNextDecodedPict->ui32TransactionId), psPicture->ui32PictId, 1 << FWIF_NUMBITS_STREAM_PICTURE_ID))
    {
        if (psNextDecodedPict != DQ_last(&psDecStrCtx->sStrDecdPictList))
        {
            psNextDecodedPict = DQ_next(psNextDecodedPict);
        }
        else
        {
            psNextDecodedPict = IMG_NULL;
        }
    }

    if (psNextDecodedPict)
    {
        DQ_addBefore(psNextDecodedPict, psDecodedPict);
    }
    else
    {
        DQ_addTail(&psDecStrCtx->sStrDecdPictList, psDecodedPict);
    }

    psDecStrCtx->sDecStrStatus.ui32NumPictDecoded++;

    // Process the decoded pictures in the encoded order.
    psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    IMG_ASSERT(psDecodedPict);
    if (psDecodedPict == IMG_NULL)
    {
        ui32Result = IMG_ERROR_UNEXPECTED_STATE;
        goto error;
    }

    // Send picture decoded message for BSPP
    {
        IMG_UINT32  ui32Result;
        ui32Result = psDecStrCtx->psDecCtx->pfnCompCallback(
            DECODER_CB_PICTURE_DECODED,
            psDecStrCtx->psDecCtx->pvCompInitUserData,
            IMG_NULL,
            psDecStrCtx->pvUserData,
            psPicture);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }

    //Loop on the unprocessed pictures until we failed to process one or we have processed them all
    for(psNextDecodedPict = decoder_NextPicture(psDecodedPict, psDecStrCtx->ui32NextDecPictId, &psDecStrCtx->sStrDecdPictList);
        psNextDecodedPict != IMG_NULL;
        psNextDecodedPict = decoder_NextPicture(psDecodedPict, psDecStrCtx->ui32NextDecPictId, &psDecStrCtx->sStrDecdPictList))
    {
#ifdef VDEC_MSVDX_HARDWARE

        IMG_UINT32 i = 0;
        DECODER_sDecodedPict* psDisplayPict = IMG_NULL;
        DECODER_sDecodedPict* psReleasePict = IMG_NULL;
        IMG_BOOL bLastToDisplayForSeq = IMG_FALSE;
#endif
        //psNextDecodedPict is used to temporarily store psDecodedPict so that we can clear the bProcessFailed flag before returning
        psDecodedPict = psNextDecodedPict;

#ifdef VDEC_MSVDX_HARDWARE

        if ((psDecStrCtx->psDecCtx->psDevConfig->eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
                && !psDecodedPict->bForceDisplay
            )
        {
            VDECFW_sBufferControl* psBufCtrl = IMG_NULL;

#ifdef VDEC_USE_PVDEC_SEC
            if (psDecStrCtx->sConfig.bSecureStream)
            {
                psBufCtrl = &psStrUnit->sBufferControl;

                /* Return resources associated with this picture */
                FWBSP_BRIDGE_PictureReturn(psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                                           psDecodedPict->ui32TransactionId,
                                           IMG_TRUE);
            }
            else
#endif /* VDEC_USE_PVDEC_SEC */
            {
            psBufCtrl = (VDECFW_sBufferControl*)psDecodedPict->psPictRefRes->sFwCtrlBuf.pvCpuVirt;
            }

            if (psBufCtrl->ui32Width && psBufCtrl->ui32Height)
            {
                /* Firmware sets image size as it is in bitstream. */

                switch(psPicture->sOutputConfig.eRotMode)
                {
                case VDEC_ROTATE_90:
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Width = psBufCtrl->ui32Height;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Height = psBufCtrl->ui32Width;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32TopOffset = 0;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32LeftOffset =
                        VDEC_ALIGN_SIZE(psBufCtrl->ui32Height, VDEC_MB_DIMENSION) - psBufCtrl->ui32Height;
                    break;
                case VDEC_ROTATE_180:
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Width = psBufCtrl->ui32Width;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Height = psBufCtrl->ui32Height;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32TopOffset =
                        VDEC_ALIGN_SIZE(psBufCtrl->ui32Height, VDEC_MB_DIMENSION) - psBufCtrl->ui32Height;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32LeftOffset = VDEC_ALIGN_SIZE(psBufCtrl->ui32Width, VDEC_MB_DIMENSION) - psBufCtrl->ui32Width;
                    break;
                case VDEC_ROTATE_270:
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Width = psBufCtrl->ui32Height;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Height = psBufCtrl->ui32Width;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32TopOffset = VDEC_ALIGN_SIZE(psBufCtrl->ui32Width, VDEC_MB_DIMENSION) - psBufCtrl->ui32Width;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32LeftOffset = 0;
                    break;
                case VDEC_ROTATE_0:
                default:
                    if ((psBufCtrl->ui32ScaleWidth!=0) && (psBufCtrl->ui32ScaleHeight!=0))
                    {
                        psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Width = psBufCtrl->ui32ScaleWidth;
                        psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Height = psBufCtrl->ui32ScaleHeight;
                    }
                    else
                    {
                        psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Width = psBufCtrl->ui32Width;
                        psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Height = psBufCtrl->ui32Height;
                    }
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32TopOffset = 0;
                    psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32LeftOffset = 0;
                    break;
                }

                psPicture->psDecPictInfo->sRendInfo.sRendPictSize.ui32Width = psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Width;
                psPicture->psDecPictInfo->sRendInfo.sRendPictSize.ui32Height = psPicture->psDecPictInfo->sDispInfo.sDispRegion.ui32Height;

                /* Update encoded size with values coded in bitstream, so golden image can be loaded correctly */
                psPicture->psDecPictInfo->sDispInfo.sEncDispRegion.ui32Width = psBufCtrl->ui32Width;
                psPicture->psDecPictInfo->sDispInfo.sEncDispRegion.ui32Height = psBufCtrl->ui32Height;
            }

            //if(psBufCtrl->eDecPictType == IMG_BUFFERTYPE_PAIR)
            //{
            //    psDecStrCtx->sDecStrStatus.ui32FieldsAsFrameDecodes++;
            //}
            psDecodedPict->psPicture->psDecPictInfo->ui32TimeStamp              =   psBufCtrl->ui32TimeStamp; // should be used in RV only
            psDecodedPict->psPicture->psDecPictInfo->sDispInfo.bTopFieldFirst = psBufCtrl->bTopFieldFirst;

            psDecodedPict->psPicture->psDecPictInfo->ui32IdForHWCrcCheck = GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId) - 1;
            psDecodedPict->psPicture->psDecPictInfo->ui32IdForHWCrcCheck += psDecStrCtx->sDecStrStatus.ui32FieldsAsFrameDecodes;

            if (IMG_BUFFERTYPE_PAIR == psBufCtrl->eDecPictType && !psBufCtrl->b2ndFieldOfPair)
            {
                psDecStrCtx->sDecStrStatus.ui32FieldsAsFrameDecodes++;
            }

            if (psBufCtrl->b2ndFieldOfPair)
            {
                // Second field of pair is always complementary type to the eFirstPictTagType of the previous picture
                IMG_UINT32 ui32PrevDecPictId = (GET_PREV_PICTURE_ID(GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId)));
                psPrevDecodedPict = decoder_GetDecodedPictOfStream(ui32PrevDecPictId,&psDecStrCtx->sStrDecdPictList);
                if(psPrevDecodedPict != IMG_NULL)
                {
                    psPrevDecodedPict->psPicture->psDecPictInfo->sSecondFieldTagContainer.pvPictTagParam = psDecodedPict->psPicture->psDecPictInfo->sFirstFieldTagContainer.pvPictTagParam;

                    //Copy the first field info in the proper place
                    IMG_MEMCPY(&psPrevDecodedPict->psPicture->psDecPictInfo->sSecondFieldTagContainer.sPictHwCrc,
                        &psFirstFieldFwMsg->sPictHwCrc,
                        sizeof(psPrevDecodedPict->psPicture->psDecPictInfo->sSecondFieldTagContainer.sPictHwCrc));

#ifndef IMG_KERNEL_MODULE
                    if (psDecStrCtx->psDecCtx->psDevConfig->bPdumpAndRes)
                    {
                        /* Update the image buffer content for pdump */
                        UPDATE_HOST(&psPictBuf->sDdBufInfo);
                    }
#endif /* ndef IMG_KERNEL_MODULE */
                }
                else
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR, "[USERSID=0x%08X] [TID 0x%08X] Failed to find decoded picture to attatch tag",
                        psDecStrCtx->sConfig.ui32UserStrId, psDecodedPict->ui32TransactionId);
                }
            }
            else
            {
                // Not Second-field-of-pair picture tag correlates its Tag to the its type by setting the eFirstPictTagType in the following way
                psDecodedPict->psPicture->psDecPictInfo->sFirstFieldTagContainer.ePicType = psBufCtrl->eDecPictType;
                IMG_MEMCPY(&psPicture->psDecPictInfo->sFirstFieldTagContainer.sPictHwCrc,
                        &psFirstFieldFwMsg->sPictHwCrc,
                        sizeof(psPicture->psDecPictInfo->sFirstFieldTagContainer.sPictHwCrc));

#ifndef IMG_KERNEL_MODULE
                if (psDecStrCtx->psDecCtx->psDevConfig->bPdumpAndRes)
                {
                    if (psBufCtrl->eDecPictType != IMG_BUFFERTYPE_FIELD_TOP &&
                        psBufCtrl->eDecPictType != IMG_BUFFERTYPE_FIELD_BOTTOM)
                    {
                        /* Update the image buffer content for pdump */
                        UPDATE_HOST(&psPictBuf->sDdBufInfo);
                    }
                }
#endif /* ndef IMG_KERNEL_MODULE */
             }

            //Update the id of the next picture to process. It has to be update always (even if we fail to process)
            //This has to be a global flag because it will be passed in both decoder_NextPicture (and then to DECODER_NextDecPictContiguous inside it)
            //and to the corner case check below
            psDecStrCtx->ui32NextDecPictId = GET_NEXT_PICTURE_ID(GET_STREAM_PICTURE_ID(psDecodedPict->ui32TransactionId));

            //Display all the picture in the list that have been decoded and signalled by the fw to be displayed
            for (i = psDecodedPict->ui32DisplayIdx;
                 i < psBufCtrl->ui32DisplayListLength && !psDecodedPict->bProcessFailed;
                 i++, psDecodedPict->ui32DisplayIdx++)
            {
                // Display picture if it has been decoded (i.e. in decoded list).
                psDisplayPict = decoder_GetDecodedPict(psBufCtrl->aui32DisplayList[i], &psDecStrCtx->sStrDecdPictList);
                if (psDisplayPict)
                {
                    if (FLAG_IS_SET((psBufCtrl->aui8DisplayFlags[i]), VDECFW_BUFFLAG_DISPLAY_FIELD_CODED))
                    {
                        if ( !FLAG_IS_SET((psBufCtrl->aui8DisplayFlags[i]), VDECFW_BUFFLAG_DISPLAY_SINGLE_FIELD) )
                        {
                            psDisplayPict->psPicture->psDecPictInfo->eBufferType = IMG_BUFFERTYPE_PAIR;
                            if (FLAG_IS_SET((psBufCtrl->aui8DisplayFlags[i]), VDECFW_BUFFLAG_DISPLAY_INTERLACED_FIELDS))
                            {
                                psDisplayPict->psPicture->psDecPictInfo->bInterlacedFields = IMG_TRUE;
                            }
                        }
                        else
                        {
                            psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                                FLAG_IS_SET((psBufCtrl->aui8DisplayFlags[i]), VDECFW_BUFFLAG_DISPLAY_BOTTOM_FIELD)
                                    ? IMG_BUFFERTYPE_FIELD_BOTTOM : IMG_BUFFERTYPE_FIELD_TOP;
                        }
                    }
                    else
                    {
                        psDisplayPict->psPicture->psDecPictInfo->eBufferType = IMG_BUFFERTYPE_FRAME;
                    }

                    psDisplayPict->psPicture->psDecPictInfo->ui16ViewID = psBufCtrl->aui8DisplayViewIds[i];

                    // When no reference pictures are left to display and this is the last display picture in response to the last decoded picture, signal.
                    if (/*LST_empty(&psDecStrCtx->sStrRefList) &&*/ psDecodedPict->psPicture->bLastPictInSeq && (i == (psBufCtrl->ui32DisplayListLength - 1)))
                    {
                        bLastToDisplayForSeq = IMG_TRUE;
                    }

                    DEBUG_REPORT(REPORT_MODULE_DECODER,
                        "[USERSID=0x%08X] [TID=0x%08X] DISPLAY",
                        psDecStrCtx->sConfig.ui32UserStrId,
                        psBufCtrl->aui32DisplayList[i]);
                    ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                                        GET_STREAM_PICTURE_ID(psBufCtrl->aui32DisplayList[i]),
                                                        bLastToDisplayForSeq);

                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return ui32Result;
                    }
                }
                else
                {
                    //In single core senario should not come here.
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                           "[USERSID=0x%08X] Failed to find decoded picture [TID = 0x%08X] to send for display",
                           psDecStrCtx->sConfig.ui32UserStrId, psBufCtrl->aui32DisplayList[i]);
                    }
                }

            // Release all unused pictures (firmware request)
            for (i = psDecodedPict->ui32ReleaseIdx;
                 i < psBufCtrl->ui32ReleaseListLength && !psDecodedPict->bProcessFailed;
                 i++,  psDecodedPict->ui32ReleaseIdx++)
            {
                psReleasePict = decoder_GetDecodedPict(psBufCtrl->aui32ReleaseList[i], &psDecStrCtx->sStrDecdPictList);
                if (psReleasePict)
                {
                    DEBUG_REPORT(REPORT_MODULE_DECODER,
                            "[USERSID=0x%08X] RELEASE( ): PIC_ID[%d]",
                            psDecStrCtx->sConfig.ui32UserStrId,
                            psReleasePict->psPicture->ui32PictId);
                    if (psReleasePict->bProcessed)
                    {
                        // If the decoded picture has been processed, destroy now.
                        ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx, psReleasePict, IMG_FALSE);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }
                    }
                    else
                    {
                        // If the decoded picture is not processed just destroy the containing picture.
                        if (psReleasePict->psPicture != IMG_NULL)
                        {
                            ui32Result = decoder_PictureDestroy(psDecStrCtx,
                                                            GET_STREAM_PICTURE_ID(psBufCtrl->aui32ReleaseList[i]),
                                                            IMG_FALSE);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return ui32Result;
                            }
                            psReleasePict->psPicture = IMG_NULL;
                        }
                    }
                }
                else
                {
                    //In single core senario should not come here.
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                           "[USERSID=0x%08X] Failed to find decoded picture [TID = 0x%08X] to release",
                           psDecStrCtx->sConfig.ui32UserStrId, psBufCtrl->aui32ReleaseList[i]);
                    }
            }
        }
        else
#endif
        {
            // Always display the picture if we have no hardware
            DEBUG_REPORT(REPORT_MODULE_DECODER,
                "[USERSID=0x%08X] [TID=0x%08X] DISPLAY",
                psDecStrCtx->sConfig.ui32UserStrId,
                psDecodedPict->ui32TransactionId);
            ui32Result = decoder_PictureDisplay( psDecStrCtx, psDecodedPict->psPicture->ui32PictId, psDecodedPict->psPicture->bLastPictInSeq);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            // Always release the picture if we have no hardware
            ui32Result = decoder_PictureDestroy(psDecStrCtx, psDecodedPict->psPicture->ui32PictId, IMG_FALSE);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
            psDecodedPict->psPicture = IMG_NULL;
        }

        //If we have processed the current picture
        if (!psDecodedPict->bProcessFailed)
        {
            psDecodedPict->bProcessed = IMG_TRUE;

            //If the current picture has been released then remove the container from the decoded list
            if (psDecodedPict->psPicture == IMG_NULL)
            {
                // Only destroy the decoded picture once it is processed and the fw has instructed to release the picture.
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx, psDecodedPict, IMG_FALSE);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }
                psDecodedPict = IMG_NULL;
            } //end if(psDecodedPict->psPicture == IMG_NULL)
        } //end if(!psDecodedPict->bProcessFailed)
    }//end for

    // Always clear the bProcessFailed flag to ensure that this picture will be processed on the next function call
    if (psDecodedPict != IMG_NULL)
    {
        psDecodedPict->bProcessFailed = IMG_FALSE;
    }

    /* Go through the list of decoded pictures to check if there are any pictures left for displaying
     * and that are still not displayed due to picture management errors. */
    {
        IMG_UINT32 ui32PictBufNum;
        IMG_UINT32 ui32DecPictNum = 0;
        IMG_UINT32 ui32RequiredPictNum = 0;
        DECODER_sDecodedPict * psAuxDecodedPict;
        DECODER_sDecodedPict * psDisplayDecodedPict = IMG_NULL;

        /* Get the minimum required number of picture buffers. */
        VDECDDUTILS_RefPictGetMaxNum(&psDecStrCtx->sConfig, psComSequHdrInfo, &ui32RequiredPictNum);
        ui32RequiredPictNum += (psComSequHdrInfo->bInterlacedFrames ? 2 : 1); // This gives minimum required buffers

        /* Get the number of currently available picture buffers. */
        psDecStrCtx->psDecCtx->pfnCompCallback(
                    DECODER_CB_GET_PICT_BUF_NUMBER,
                    IMG_NULL,
                    IMG_NULL,
                    psDecStrCtx->pvUserData,
                    &ui32PictBufNum);

        /* Start the procedure only if there is enough picture buffers registered. */
        if (ui32PictBufNum >= ui32RequiredPictNum)
        {
            /* Allow for one picture buffer for display. */
            ui32PictBufNum--;

            /* Count the number of decoded pictures that were not displayed yet. */
            psAuxDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
            while (psAuxDecodedPict)
            {
                if(IMG_NULL != psAuxDecodedPict->psPicture)
                {
                    ui32DecPictNum++;
                    if(IMG_NULL == psDisplayDecodedPict)
                    {
                        psDisplayDecodedPict = psAuxDecodedPict;
                    }
                }
                if (psAuxDecodedPict != DQ_last(&psDecStrCtx->sStrDecdPictList))
                {
                    psAuxDecodedPict = DQ_next(psAuxDecodedPict);
                }
                else
                {
                    psAuxDecodedPict = IMG_NULL;
                }
            }

            /* If there is at least one not displayed picture... */
            if (psDisplayDecodedPict)
            {
                /* While the number of not displayed decoded pictures exceeds
                 * the number of maximum allowed number of pictures being held by VDEC... */
                while(ui32DecPictNum > ui32PictBufNum)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
                        "[USERSID=0x%08X] Number of outstanding decoded pictures exceeded number of available pictures buffers.",
                        psDecStrCtx->sConfig.ui32UserStrId);

                    /* Find the picture with the least picture id. */
                    psAuxDecodedPict = DQ_next(psDisplayDecodedPict);
                    while (psAuxDecodedPict)
                    {
                        if (psAuxDecodedPict != DQ_last(&psDecStrCtx->sStrDecdPictList))
                        {
                            if((IMG_NULL != psAuxDecodedPict->psPicture) &&
                               (psAuxDecodedPict->psPicture->ui32PictId < psDisplayDecodedPict->psPicture->ui32PictId))
                            {
                                psDisplayDecodedPict = psAuxDecodedPict;
                            }
                            psAuxDecodedPict = DQ_next(psAuxDecodedPict);
                        }
                        else
                        {
                            if((IMG_NULL != psAuxDecodedPict->psPicture) &&
                               (psAuxDecodedPict->psPicture->ui32PictId < psDisplayDecodedPict->psPicture->ui32PictId))
                            {
                                psDisplayDecodedPict = psAuxDecodedPict;
                            }
                            psAuxDecodedPict = IMG_NULL;
                        }
                    }

                    /* Display and release the picture with the least picture id. */
                    DEBUG_REPORT(REPORT_MODULE_DECODER,
                        "[USERSID=0x%08X] [TID=0x%08X] DISPLAY FORCED",
                        psDecStrCtx->sConfig.ui32UserStrId,
                        psDisplayDecodedPict->ui32TransactionId);
                    ui32Result = decoder_PictureDisplay( psDecStrCtx, psDisplayDecodedPict->psPicture->ui32PictId, psDisplayDecodedPict->psPicture->bLastPictInSeq);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return ui32Result;
                    }

                    ui32Result = decoder_PictureDestroy(psDecStrCtx, psDisplayDecodedPict->psPicture->ui32PictId, IMG_FALSE);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return ui32Result;
                    }
                    psDisplayDecodedPict->psPicture = IMG_NULL;
                    psDisplayDecodedPict->bProcessed = IMG_TRUE;

                    ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx, psDisplayDecodedPict, IMG_FALSE);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return ui32Result;
                    }
                    psDisplayDecodedPict = IMG_NULL;

                    /* Decrease the number of not displayed decoded pictures. */
                    ui32DecPictNum--;
                }
            }
        }
    }
    return IMG_SUCCESS;

error:
    return ui32Result;
}


static IMG_RESULT decoder_WrapBitStrSegments(
    LST_T *              psBitStrSegList,//VDECDD_sBitStrSeg
    LST_T *              psDecPictSegList,//DECODER_sDecPictSeg
    IMG_UINT32           ui32UserStrId)
{
    // Required for attaching segments to the decode picture
    BSPP_sBitStrSeg   * psBitStrSeg;
    DECODER_sDecPictSeg * psDecPictSeg;

    IMG_ASSERT(psBitStrSegList);
    IMG_ASSERT(psDecPictSegList);

    // Add the segments to the Decode Picture
    psBitStrSeg = LST_first(psBitStrSegList);
    while(psBitStrSeg)
    {
        VDEC_MALLOC(psDecPictSeg);
        IMG_ASSERT(psDecPictSeg);
        if (psDecPictSeg == IMG_NULL)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                "[USERSID=0x%08X] Failed to allocate memory for current picture's bitstream segments",
                ui32UserStrId);
            return IMG_ERROR_OUT_OF_MEMORY;
        }
        VDEC_BZERO(psDecPictSeg);

        psDecPictSeg->psBitStrSeg = psBitStrSeg;
        psDecPictSeg->bInternalSeg = IMG_FALSE;
        LST_add(psDecPictSegList,psDecPictSeg);

        psBitStrSeg = LST_next(psBitStrSeg);
    }

    return IMG_SUCCESS;
}

static IMG_VOID decoder_CleanBitStrSegments(
    LST_T *              psDecPictSegList)//DECODER_sDecPictSeg
{
    DECODER_sDecPictSeg * psDecPictSeg;

    while(IMG_NULL != (psDecPictSeg = LST_removeHead(psDecPictSegList)))
    {
        if (psDecPictSeg->bInternalSeg)
        {
            IMG_ASSERT(psDecPictSeg->psBitStrSeg);
            IMG_FREE(psDecPictSeg->psBitStrSeg);
            psDecPictSeg->psBitStrSeg = IMG_NULL;
        }

        IMG_FREE(psDecPictSeg);
        psDecPictSeg = IMG_NULL;
    }
}

/*!
******************************************************************************

 @Function              decoder_PictureDecode

******************************************************************************/
static IMG_RESULT
decoder_PictureDecode(
    DECODER_sStrContext *   psDecStrCtx,
    VDECDD_sStrUnit *       psStrUnit,
    DECODER_sDecPict **     ppsDecPict
)
{
    VDECDD_sPicture       * psPicture;
    DECODER_sCoreContext  * psDecCoreCtx;
    DECODER_sDecPict    * psDecPict;
    IMG_RESULT            ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psStrUnit);
    IMG_ASSERT(psStrUnit && psStrUnit->psPictHdrInfo);
    IMG_ASSERT(ppsDecPict);

    psPicture = (VDECDD_sPicture *)psStrUnit->pvDdPictData;
    psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);
    IMG_ASSERT(psPicture);
    IMG_ASSERT(psDecCoreCtx);

    // Ensure this is a new picture.
    IMG_ASSERT(psDecStrCtx->psCurrentPicture == IMG_NULL);
    IMG_ASSERT(psStrUnit->eStrUnitType == VDECDD_STRUNIT_PICTURE_START);

    psDecCoreCtx->ui32CumulativePics++;

    // Allocate a unique id to the picture.
    ui32Result = IDGEN_AllocId(psDecStrCtx->hPictIdGen, psPicture, &psPicture->ui32PictId);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    /* Allocate the decoded picture information structure. */
    VDEC_MALLOC(psPicture->psDecPictInfo);
    IMG_ASSERT(psPicture->psDecPictInfo);
    if (psPicture->psDecPictInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for decoded picture information",
            psDecStrCtx->sConfig.ui32UserStrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psPicture->psDecPictInfo);

    // Extract decoded information from the stream unit.
    psPicture->psDecPictInfo->ui32ErrorFlags = psStrUnit->ui32ErrorFlags;
    psPicture->psDecPictInfo->sFirstFieldTagContainer.pvPictTagParam = (IMG_UINTPTR)(psStrUnit->pvStrUnitTag);
    psPicture->psDecPictInfo->sOutputConfig  = psPicture->sOutputConfig;
    psPicture->psDecPictInfo->sRendInfo      = psPicture->sDisplayPictBuf.sRendInfo;
    psPicture->psDecPictInfo->sDispInfo      = psStrUnit->psPictHdrInfo->sDispInfo;

    // Extract aux picture information from the stream unit.
    psPicture->sDecPictAuxInfo.ui32SequenceHdrId = psStrUnit->psSequHdrInfo->ui32SequHdrId;
    psPicture->sDecPictAuxInfo.ui32PPSId         = psStrUnit->psPictHdrInfo->sPictAuxData.ui32Id;
    psPicture->sDecPictAuxInfo.ui32SecondPPSId   = psStrUnit->psPictHdrInfo->sSecondPictAuxData.ui32Id;

    // Create a new decoder picture container.
    VDEC_MALLOC(psDecPict);
    IMG_ASSERT(psDecPict);
    if (psDecPict == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for decode picture",
            psDecStrCtx->sConfig.ui32UserStrId);
        ui32Result = IMG_ERROR_OUT_OF_MEMORY;
        goto error_dec_pict;
    }
    VDEC_BZERO(psDecPict);

    // Attach decoder/picture context information.
    psDecPict->psDecStrCtx = psDecStrCtx;
    psDecPict->ui8Pipe = psDecStrCtx->ui8Pipe;

    // Construct the transaction Id.
    // This consists of stream and core number in addition to picture number in stream
    // and a 4-bit value representing the picture number in core.
    psDecPict->ui32TransactionId = CREATE_TRANSACTION_ID(psDecStrCtx->ui32StrId,
                                                         psDecCoreCtx->ui32CumulativePics,
                                                         psPicture->ui32PictId);

#ifdef VDEC_USE_PVDEC_SEC
    if (psDecStrCtx->sConfig.bSecureStream)
    {
        FWBSP_BRIDGE_PictureAdd(psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                                psStrUnit,
                                psDecPict->ui32TransactionId,
                                psPicture);
    }
#endif /* VDEC_USE_PVDEC_SEC */

    // Add picture to core decode list
    psDecStrCtx->sDecStrStatus.ui32NumPictDecoding++;

    // Fake a FW message to process when decoded.
    VDEC_MALLOC(psDecPict->psFirstFieldFwMsg);
    IMG_ASSERT(psDecPict->psFirstFieldFwMsg);
    if (psDecPict->psFirstFieldFwMsg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for firmware message for decoded picture",
            psDecStrCtx->sConfig.ui32UserStrId);
        ui32Result = IMG_ERROR_OUT_OF_MEMORY;
        goto error_fw_msg;
    }
    VDEC_BZERO(psDecPict->psFirstFieldFwMsg);

    VDEC_MALLOC(psDecPict->psSecondFieldFwMsg);
    IMG_ASSERT(psDecPict->psSecondFieldFwMsg);
    if (psDecPict->psSecondFieldFwMsg == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for firmware message for decoded picture",
            psDecStrCtx->sConfig.ui32UserStrId);
        ui32Result = IMG_ERROR_OUT_OF_MEMORY;
        goto error_fw_msg;
    }
    VDEC_BZERO(psDecPict->psSecondFieldFwMsg);

    // Add the segments to the Decode Picture
    ui32Result = decoder_WrapBitStrSegments(&psStrUnit->sBitStrSegList,
                               &psDecPict->sDecPictSegList,
                               psDecStrCtx->sConfig.ui32UserStrId);

    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error_segments;
    }

    // Shuffle the current and previous
    // Hold a reference to the last context on the FE
    if (psDecStrCtx->psPrevFePictDecRes)
    {
        // Return previous last FW context.
        RESOURCE_ItemReturn(&psDecStrCtx->psPrevFePictDecRes->ui32RefCount);

#ifndef IMG_KERNEL_MODULE
        if(RESOURCE_ItemIsAvailable(&psDecStrCtx->psPrevFePictDecRes->ui32RefCount))
        {
            RESOURCE_ListRemove(&psDecStrCtx->sDecResList, psDecStrCtx->psPrevFePictDecRes);
            RESOURCE_ListAdd(&psDecStrCtx->sDecResList, psDecStrCtx->psPrevFePictDecRes, 0, &psDecStrCtx->psPrevFePictDecRes->ui32RefCount);
        }
#endif
    }

    psDecStrCtx->psPrevFePictDecRes = psDecStrCtx->psCurFePictDecRes;
    psDecPict->psPrevPictDecRes = psDecStrCtx->psPrevFePictDecRes;

    // Get a new stream decode resource bundle for current picture.
    psDecPict->psCurPictDecRes = RESOURCE_ListGetAvail(&psDecStrCtx->sDecResList);
    IMG_ASSERT(psDecPict->psCurPictDecRes);
    if (psDecPict->psCurPictDecRes == IMG_NULL)
    {
        ui32Result = IMG_ERROR_UNEXPECTED_STATE;
        goto error_dec_res;
    }

#ifdef HAS_H264
    if (VDEC_STD_H264 == psDecStrCtx->sConfig.eVidStd)
    {
        // Copy any SGM for current picture.
        if (psStrUnit->psPictHdrInfo->sPictSgmData.ui32Id != BSPP_INVALID)
        {
            IMG_ASSERT(psStrUnit->psPictHdrInfo->sPictSgmData.ui32Size <= psDecPict->psCurPictDecRes->sH264SgmBuf.ui32BufSize);
            //Updated in translation_api
            IMG_MEMCPY(psDecPict->psCurPictDecRes->sH264SgmBuf.pvCpuVirt,
                psStrUnit->psPictHdrInfo->sPictSgmData.pvData,
                psStrUnit->psPictHdrInfo->sPictSgmData.ui32Size);
        }
    }
#endif /* HAS_H264 */

    psDecPict->psCurPictDecRes->ui32TransactionId = psDecPict->ui32TransactionId;
    psDecStrCtx->psCurFePictDecRes = psDecPict->psCurPictDecRes;
    RESOURCE_ItemUse(&psDecStrCtx->psCurFePictDecRes->ui32RefCount);

    // Get a new control buffer.
    psDecPict->psPictRefRes = RESOURCE_ListGetAvail(&psDecStrCtx->sRefResList);
    IMG_ASSERT(psDecPict->psPictRefRes);
    if (psDecPict->psPictRefRes == IMG_NULL)
    {
        ui32Result = IMG_ERROR_UNEXPECTED_STATE;
        goto error_ref_res;
    }

    IMG_ASSERT(psDecStrCtx->psDecCtx);
    IMG_ASSERT(psDecStrCtx->psDecCtx->psDevConfig);

    psDecPict->psStrPtdBufInfo = &psDecStrCtx->psDecCtx->sPtdBufInfo;

#ifdef VDEC_USE_PVDEC
    psDecPict->psStrPvdecFwCtxBuf = &psDecStrCtx->sPvdecFwCtxBuf;
#endif
    psDecPict->psPictHdrInfo = psStrUnit->psPictHdrInfo;

    // Take the WDT period configured for the device.
    psDecPict->ui32FeWdtPeriod = psDecStrCtx->psDecCtx->psDevConfig->ui32FeWdtPeriod;
    psDecPict->ui32BeWdtPeriod = psDecStrCtx->psDecCtx->psDevConfig->ui32BeWdtPeriod;
    psDecPict->ui32PSRWdtPeriod = psDecStrCtx->psDecCtx->psDevConfig->ui32PSRWdtPeriod;

    // Obtain (core) resources for the picture.
    ui32Result = RESOURCE_PictureAttach(
        &psDecCoreCtx->hResources,
        psDecStrCtx->sConfig.eVidStd,
        psDecPict,
        psDecStrCtx->sConfig.bSecureStream);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error_res_attach;
    }

#ifndef IMG_KERNEL_MODULE
    if(psDecCoreCtx->sLasRes.bInit)
    {
           // If two consecutive pictures use the same output buffer and we want to dump the output at the pdump.
        if( (((psDecPict->psAltPict != IMG_NULL && psDecCoreCtx->sLasRes.hAltPict == psDecPict->psAltPict) ||
              (psDecPict->psReconPict != IMG_NULL && psDecCoreCtx->sLasRes.hReconPict == psDecPict->psReconPict))
              && psDecCoreCtx->psDecCtx->psDevConfig->bPdumpAndRes) ||
           (psDecPict->psHdrInfo != IMG_NULL && psDecCoreCtx->sLasRes.hHdrInfo == psDecPict->psHdrInfo) ||
           (psDecPict->psBatchMsgInfo != IMG_NULL && psDecCoreCtx->sLasRes.hBatchMsgInfo == psDecPict->psBatchMsgInfo) ||
           (psDecPict->psTransactionInfo != IMG_NULL && psDecCoreCtx->sLasRes.hTransactionInfo == psDecPict->psTransactionInfo) ||
           (psDecPict->psPictHdrInfo != IMG_NULL && psDecCoreCtx->sLasRes.hPictHdrInfo == psDecPict->psPictHdrInfo) ||
           (psDecPict->psPictRefRes != IMG_NULL && psDecCoreCtx->sLasRes.hPictRefRes == psDecPict->psPictRefRes) ||
           // If the context buffer which the previous picture will load from is the same as this one will save into
           (psDecPict->psCurPictDecRes != IMG_NULL && psDecCoreCtx->sLasRes.hPrevPictDecRes == psDecPict->psCurPictDecRes))
        {

            // Sync pdump contexts if two consecutive pictures share a resource. This is because if we don't the script
            // may try to schedule them together for decoding.
            HWCTRL_SyncPDumpContexts(psDecCoreCtx->hHwCtx);
        }
    }

    psDecCoreCtx->sLasRes.bInit = IMG_TRUE;
    psDecCoreCtx->sLasRes.hAltPict = psDecPict->psAltPict;
    psDecCoreCtx->sLasRes.hReconPict = psDecPict->psReconPict;
    psDecCoreCtx->sLasRes.hHdrInfo = psDecPict->psHdrInfo;
    psDecCoreCtx->sLasRes.hBatchMsgInfo = psDecPict->psBatchMsgInfo;
    psDecCoreCtx->sLasRes.hTransactionInfo = psDecPict->psTransactionInfo;
    psDecCoreCtx->sLasRes.hPictHdrInfo = psDecPict->psPictHdrInfo;
    psDecCoreCtx->sLasRes.hPictRefRes = psDecPict->psPictRefRes;
    psDecCoreCtx->sLasRes.hPrevPictDecRes = psDecPict->psPrevPictDecRes;

#ifdef VDEC_USE_PVDEC
    {
        IMG_UINT32 ui8PipeMinus1;
        IMG_UINT32 ui32StreamFeature = psStrUnit->ui32Features;

        //count the number of pipes that the stream can be decoded on
        psDecPict->ui32NumAvailPipes = 0;
        for(ui8PipeMinus1 = 0; ui8PipeMinus1 < VDEC_MAX_PIXEL_PIPES;ui8PipeMinus1++)
        {
            psDecPict->ui32NumAvailPipes += (((ui32StreamFeature & psDecCoreCtx->aui32PipeFeatures[ui8PipeMinus1])) == ui32StreamFeature) ? 1 : 0;
        }
    }
#endif
#endif
    // Clear fw context data for re-use.
    IMG_MEMSET(psDecPict->psCurPictDecRes->sFwCtxBuf.pvCpuVirt, 0, psDecPict->psCurPictDecRes->sFwCtxBuf.ui32BufSize);
    UPDATE_DEVICE(&psDecPict->psCurPictDecRes->sFwCtxBuf);

    // Clear the control data in case the picture is discarded before being prepared by firmware.
    IMG_MEMSET(psDecPict->psPictRefRes->sFwCtrlBuf.pvCpuVirt, 0, psDecPict->psPictRefRes->sFwCtrlBuf.ui32BufSize);
    UPDATE_DEVICE(&psDecPict->psPictRefRes->sFwCtrlBuf);

    // Translate all the host data into a native and optimal firmware representation.
#ifdef VDEC_USE_PVDEC_SEC
    if (psDecStrCtx->sConfig.bSecureStream)
    {
        ui32Result = TRANSLATION_PicturePrepareSecure(
                        psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                        &psDecStrCtx->sConfig,
                        psStrUnit,
                        psDecPict,
                        &psDecCoreCtx->sCoreProps);
    }
    else
#endif /* VDEC_USE_PVDEC_SEC */
    {
        DECODER_sRegsOffsets sRegsOffsets = { 0 };

        ui32Result = HWCTRL_GetRegsOffsets(psDecCoreCtx->hHwCtx, &sRegsOffsets);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error_other;
        }

        ui32Result = TRANSLATION_CtrlAllocPrepare(
                        &psDecStrCtx->sConfig,
                        psStrUnit,
                        psDecPict,
                        &psDecCoreCtx->sCoreProps,
                        &sRegsOffsets);
    }

    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error_other;
    }

    // This particular core might have gone into APM, so make sure we resume before proceeding.
    if(psDecCoreCtx->bAPM)
    {
        DMANKM_ResumeDevice(psDecCoreCtx->psDecCtx->hDevHandle, IMG_TRUE);
        psDecCoreCtx->bAPM = IMG_FALSE;
    }

#ifdef VDEC_USE_PVDEC_SEC
    if (!psDecStrCtx->sConfig.bSecureStream)
#endif /* VDEC_USE_PVDEC_SEC */
    {
        ui32Result = HWCTRL_PictureSubmitBatch(
                        psDecCoreCtx->hHwCtx,
                        psDecPict,
                        psDecCoreCtx->hResources,
                        &psDecCoreCtx->psDecCtx->sPtdInfo);
        }
#ifdef VDEC_USE_PVDEC_SEC
    else
    {
        ui32Result = HWCTRL_PictureSubmitSecure(
                        psDecCoreCtx->hHwCtx,
                        psDecPict,
                        psDecCoreCtx->hResources,
                        &psDecCoreCtx->psDecCtx->sPtdInfo);
    }
#endif /* VDEC_USE_PVDEC_SEC */
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if(ui32Result != IMG_SUCCESS)
    {
        goto error_other;
    }

    IMG_ASSERT(psDecCoreCtx->bBusy == IMG_FALSE);
    psDecCoreCtx->bBusy = IMG_TRUE;
    // Store this transaction ID in stream context.
    //if ()
    {
        psDecStrCtx->ui32LastFeTransactionId = psDecPict->ui32TransactionId;
        psDecStrCtx->psCurrentPicture = psDecPict;
        decoder_UpdateAvailPipes(psDecStrCtx);
    }

    psDecStrCtx->sDecStrStatus.ui32Features = psStrUnit->ui32Features;

    *ppsDecPict = psDecPict;

    return IMG_SUCCESS;

    /* Roll back in case of errors. */
error_other:
    RESOURCE_PictureDetach(&psDecCoreCtx->hResources,
                           psDecPict,
                           psDecStrCtx->sConfig.bSecureStream);
error_res_attach:
error_ref_res:
error_dec_res:
error_segments:
    decoder_CleanBitStrSegments(&psDecPict->sDecPictSegList);
    IMG_FREE(psDecPict->psFirstFieldFwMsg);
    psDecPict->psFirstFieldFwMsg = IMG_NULL;
    IMG_FREE(psDecPict->psSecondFieldFwMsg);
    psDecPict->psSecondFieldFwMsg = IMG_NULL;
error_fw_msg:
    IMG_FREE(psDecPict);
    psDecPict = IMG_NULL;
error_dec_pict:
    IMG_FREE(psPicture->psDecPictInfo);
    psPicture->psDecPictInfo = IMG_NULL;

    return ui32Result;
}

/*!
******************************************************************************

 @Function              decoder_KickListService

******************************************************************************/
static IMG_RESULT
decoder_KickListService(
        DECODER_sStrContext *   psDecStrCtx,
        VDECDD_sStrUnit *       psStrUnit
)
{
    IMG_RESULT             ui32Result;
    DECODER_sCoreContext * psCoreCtx;

    IMG_ASSERT(psDecStrCtx);
    psCoreCtx =  DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);
    IMG_ASSERT(psCoreCtx);
    // Kick SW Interrupt
    ui32Result = HWCTRL_KickSwInterrupt(psCoreCtx->hHwCtx, (IMG_HANDLE)psDecStrCtx);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              decoder_PictureFinalize

******************************************************************************/
static IMG_RESULT
decoder_PictureFinalize(
        DECODER_sStrContext *   psDecStrCtx,
        VDECDD_sStrUnit *       psStrUnit
)
{
    VDECDD_sPicture      * psPicture = IMG_NULL;
    IMG_RESULT             ui32Result;
    DECODER_sDecPict     * psDecPict;
    DECODER_sCoreContext * psDecCoreCtx = IMG_NULL;

    IMG_ASSERT(psStrUnit->eStrUnitType == VDECDD_STRUNIT_PICTURE_END);
    IMG_ASSERT(psDecStrCtx);

    psDecPict = psDecStrCtx->psCurrentPicture;
    if (IMG_NULL == psDecPict)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
               "[USERSID=0x%08X] Unable to get the current picture from Decoder context",
               psDecStrCtx->sConfig.ui32UserStrId);
        return IMG_ERROR_GENERIC_FAILURE;
    }

    // Get pointer to appropriate VDECDD_sPicture via picture id
    ui32Result = IDGEN_GetHandle(psDecStrCtx->hPictIdGen,
                                 GET_STREAM_PICTURE_ID(psDecStrCtx->ui32LastFeTransactionId),
                                 (IMG_VOID **) &psPicture);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    IMG_ASSERT(psPicture != IMG_NULL);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }
    if (psPicture == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Unable to find current picture from transaction ID",
            psDecStrCtx->sConfig.ui32UserStrId);
        return IMG_ERROR_COULD_NOT_OBTAIN_RESOURCE;
    }

    // Update error flags
    psPicture->psDecPictInfo->ui32ErrorFlags |= psStrUnit->ui32ErrorFlags;

    psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);
    IMG_ASSERT(psDecCoreCtx);

    IMG_ASSERT(psDecCoreCtx->bBusy == IMG_TRUE);
    psDecCoreCtx->bBusy = IMG_FALSE;

    // Picture data are now complete, nullify pointer
    psDecStrCtx->psCurrentPicture = IMG_NULL;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_StreamGetContext

 @Description

 This function returns the stream context structure for the given stream handle.

 @Return    DECODER_sStrContext : This function returns a pointer to the stream
                                   context structure or IMG_NULL if not found.

******************************************************************************/
static DECODER_sStrContext *
decoder_StreamGetContext(
    IMG_HANDLE hDecStrCtx
)
{
    return (DECODER_sStrContext *)hDecStrCtx;
}


#ifdef VDEC_MSVDX_HARDWARE
#ifdef HAS_AVS
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_AVS

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_AVS(
    DECODER_sStrContext *   psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                bDiscardRefs
)
{
    AVSFW_sContextData   * psCtxData =
        (AVSFW_sContextData *)psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;
    DECODER_sDecodedPict * psDisplayPict;
    IMG_RESULT                ui32Result;

                // Update the fw context for analysing the dpb in order to display or release any outstanding picture
    // If core 0 is fake, all are fake
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

    if(psCtxData->sNewRefPic.ui32TransId &&
       (!psCtxData->sNewRefPic.bSentForDisplay))
                {
        IMG_UINT32 ui32StrPicId;
                    DEBUG_REPORT(REPORT_MODULE_DECODER,
                        "[TID=0x%08X] DISPLAY", psCtxData->sNewRefPic.ui32TransId);

        psDisplayPict =
            decoder_GetDecodedPict(psCtxData->sNewRefPic.ui32TransId,
                                   &psDecStrCtx->sStrDecdPictList);

        if (psDisplayPict &&
            psDisplayPict->psPicture &&
            psDisplayPict->psPicture->psDecPictInfo)
                    {
            if (FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                VDECFW_BUFFLAG_DISPLAY_FIELD_CODED))
                        {
                if (!FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                    VDECFW_BUFFLAG_DISPLAY_SINGLE_FIELD))
                            {
                    psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        IMG_BUFFERTYPE_PAIR;
                            }
                            else
                            {
                                psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                                    VDECFW_BUFFLAG_DISPLAY_BOTTOM_FIELD) ?
                        IMG_BUFFERTYPE_FIELD_BOTTOM :
                        IMG_BUFFERTYPE_FIELD_TOP;
                            }
                        }
                        else
                        {
                psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                    IMG_BUFFERTYPE_FRAME;
                        }
                    }
                    else
                    {
                        IMG_ASSERT(IMG_FALSE);
                    }

        ui32StrPicId = GET_STREAM_PICTURE_ID(psCtxData->sNewRefPic.ui32TransId);

                    ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                            ui32StrPicId,
                                                        IMG_TRUE);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return ui32Result;
                    }
                }
                if (bDiscardRefs)
                {
                    IMG_UINT32 ui32i;
        IMG_UINT32 aui32PicIdsToDestroy[] =
            { psCtxData->sNewRefPic.ui32TransId,
                                                          psCtxData->sOldRefPic.ui32TransId, };

        for (ui32i = 0;
             ui32i < sizeof(aui32PicIdsToDestroy)/sizeof(IMG_UINT32);
             ui32i++)
                    {
                        if(aui32PicIdsToDestroy[ui32i])
                        {
                psDecodedPict =
                    decoder_GetDecodedPict(aui32PicIdsToDestroy[ui32i],
                                    &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                    ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                               psDecodedPict,
                                                               IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                            }
                        }
                    }

                    psCtxData->sNewRefPic.ui32TransId  = 0;
                    psCtxData->sOldRefPic.ui32TransId  = 0;
                }

    return IMG_SUCCESS;
}
#endif /* HAS_AVS */

#ifdef HAS_H264
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_H264

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_H264(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    H264FW_sContextData * psCtxData = (H264FW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;
                    IMG_BOOL bFound = IMG_TRUE;
                    IMG_UINT32 i;
                    IMG_INT32  i32MinCount;
                    IMG_UINT32 ui32MinCountIdx;
                    IMG_UINT32 ui32NumDisplayPics = 0;
                    IMG_UINT32 ui32NumPicsDisplayed = 0;
                    DECODER_sDecodedPict* psDisplayPict = IMG_NULL;
    IMG_UINT32 ui32LastDisPicTID;

    // Update the fw context for analysing the dpb in order to display
    // or release any outstanding picture.
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

                    // Determine how many display pictures reside in the DPB.
    if (psCtxData->ui32DpbSize > H264FW_MAX_DPB_SIZE)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
               "[USERSID=0x%08X] Incorrect DPB size: %d",
               psDecStrCtx->sConfig.ui32UserStrId, psCtxData->ui32DpbSize);
               psCtxData->ui32DpbSize = H264FW_MAX_DPB_SIZE;
    }
                    for (i = 0; i < psCtxData->ui32DpbSize; i++)
                    {
                        if (psCtxData->asDpb[i].ui32TransactionId)
                        {
                            if (psCtxData->asDpb[i].bNeededForOutput)
                            {
                                ui32NumDisplayPics++;
                            }
                        }
                    }

    ui32LastDisPicTID = psCtxData->asLastDisplayedPicData[0].ui32TransactionId;
                    // Check for picture stuck outside the dpb
    if (ui32LastDisPicTID)
                    {
                        DEBUG_REPORT(REPORT_MODULE_DECODER,
                            "[USERSID=0x%08X] [TID=0x%08X] DISPLAY",
                            psDecStrCtx->sConfig.ui32UserStrId,
            ui32LastDisPicTID);
        IMG_ASSERT(ui32LastDisPicTID != 0xffffffff);

        psDisplayPict = decoder_GetDecodedPict(ui32LastDisPicTID,
                                                               &psDecStrCtx->sStrDecdPictList);

        if (psDisplayPict &&
            psDisplayPict->psPicture &&
            psDisplayPict->psPicture->psDecPictInfo)
                        {
            if (FLAG_IS_SET((psCtxData->ui8PrevDisplayFlags),
                VDECFW_BUFFLAG_DISPLAY_FIELD_CODED))
                            {
                if (!FLAG_IS_SET((psCtxData->ui8PrevDisplayFlags),
                    VDECFW_BUFFLAG_DISPLAY_SINGLE_FIELD))
                                {
                    psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        IMG_BUFFERTYPE_PAIR;
                                }
                                else
                                {
                                    psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        FLAG_IS_SET((psCtxData->ui8PrevDisplayFlags),
                                    VDECFW_BUFFLAG_DISPLAY_BOTTOM_FIELD) ?
                        IMG_BUFFERTYPE_FIELD_BOTTOM :
                        IMG_BUFFERTYPE_FIELD_TOP;
                                }
                            }
                            else
                            {
                psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                    IMG_BUFFERTYPE_FRAME;
                            }

                        }
                        else
                        {
                            IMG_ASSERT(psDisplayPict);
            IMG_ASSERT(psDisplayPict &&
                       psDisplayPict->psPicture);
            IMG_ASSERT(psDisplayPict &&
                       psDisplayPict->psPicture &&
                       psDisplayPict->psPicture->psDecPictInfo);
                        }

        ui32Result =
            decoder_PictureDisplay(psDecStrCtx,
                                   GET_STREAM_PICTURE_ID(ui32LastDisPicTID),
                                   IMG_TRUE);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }
                    }

                    while (bFound)
                    {
                        i32MinCount = ((IMG_UINT32)(1 << 31)) - 1;
                        ui32MinCountIdx = -1;
                        bFound = IMG_FALSE;

                        // Loop over the DPB to find the first in order
                        for (i = 0; i < psCtxData->ui32DpbSize; i++)
                        {
                            if (psCtxData->asDpb[i].ui32TransactionId)
                            {
                                if ( psCtxData->asDpb[i].bNeededForOutput || bDiscardRefs )
                                {
                                    if (psCtxData->asDpb[i].i32TopFieldOrderCount < i32MinCount)
                                    {
                                        i32MinCount = psCtxData->asDpb[i].i32TopFieldOrderCount;
                                        ui32MinCountIdx = i;
                                        bFound = IMG_TRUE;
                                    }
                                }
                            }
                        }

                        if (bFound)
                        {
            IMG_UINT32 ui32MinCountTID =
                psCtxData->asDpb[ui32MinCountIdx].ui32TransactionId;
                            if (psCtxData->asDpb[ui32MinCountIdx].bNeededForOutput)
                            {
                                DEBUG_REPORT(REPORT_MODULE_DECODER,
                                    "[USERSID=0x%08X] [TID=0x%08X] DISPLAY",
                                    psDecStrCtx->sConfig.ui32UserStrId,
                    ui32MinCountTID);
                IMG_ASSERT(ui32MinCountTID != 0xffffffff);

                psDisplayPict =
                    decoder_GetDecodedPict(ui32MinCountTID,
                                                                       &psDecStrCtx->sStrDecdPictList);

                if (psDisplayPict &&
                    psDisplayPict->psPicture &&
                    psDisplayPict->psPicture->psDecPictInfo)
                                {
                    if (FLAG_IS_SET((psCtxData->asDpb[ui32MinCountIdx].ui8DisplayFlags),
                        VDECFW_BUFFLAG_DISPLAY_FIELD_CODED))
                                    {
                        if (!FLAG_IS_SET((psCtxData->asDpb[ui32MinCountIdx].ui8DisplayFlags),
                            VDECFW_BUFFLAG_DISPLAY_SINGLE_FIELD))
                                        {
                            psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                                IMG_BUFFERTYPE_PAIR;
                                        }
                                        else
                                        {
                                            psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                                FLAG_IS_SET((psCtxData->asDpb[ui32MinCountIdx].ui8DisplayFlags),
                                            VDECFW_BUFFLAG_DISPLAY_BOTTOM_FIELD) ?
                                    IMG_BUFFERTYPE_FIELD_BOTTOM :
                                    IMG_BUFFERTYPE_FIELD_TOP;
                                        }
                                    }
                                    else
                                    {
                        psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                            IMG_BUFFERTYPE_FRAME;
                                    }

                    psDisplayPict->psPicture->psDecPictInfo->ui16ViewID =
                        psCtxData->asDpb[ui32MinCountIdx].ui16ViewId;
                                }
                                else
                                {
                                    IMG_ASSERT(psDisplayPict);
                    IMG_ASSERT(psDisplayPict &&
                               psDisplayPict->psPicture);
                    IMG_ASSERT(psDisplayPict &&
                               psDisplayPict->psPicture &&
                               psDisplayPict->psPicture->psDecPictInfo);
                                }

                                ui32NumPicsDisplayed++;
                ui32Result =
                    decoder_PictureDisplay(psDecStrCtx,
                                           GET_STREAM_PICTURE_ID(ui32MinCountTID),
                                            (ui32NumPicsDisplayed == ui32NumDisplayPics) ?
                                            IMG_TRUE : IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                                psCtxData->asDpb[ui32MinCountIdx].bNeededForOutput = IMG_FALSE;
                            }

                            if (bDiscardRefs)
                            {
                psDecodedPict =
                    decoder_GetDecodedPict(ui32MinCountTID,
                                                                       &psDecStrCtx->sStrDecdPictList);
                                if (psDecodedPict)
                                {
                    ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                               psDecodedPict,
                                                               IMG_FALSE);
                                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                    if (ui32Result != IMG_SUCCESS)
                                    {
                                        return ui32Result;
                                    }
                                }


                                psCtxData->asDpb[ui32MinCountIdx].ui32TransactionId = 0;
                            }
                        }
                    }

                    IMG_ASSERT(ui32NumPicsDisplayed == ui32NumDisplayPics);

    return IMG_SUCCESS;
}
#endif /* HAS_H264*/

#ifdef HAS_MPEG4
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_MPEG4

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_MPEG4(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    MPEG4FW_sContextData * psCtxData = (MPEG4FW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;

    // Update the fw context for analysing the dpb in order to display
    // or release any outstanding picture
    // If core 0 is fake, all are fake
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

    if(psCtxData->sNewRefPic.ui32TransactionId &&
       (!psCtxData->sNewRefPic.bSentForDisplay))
                    {
        IMG_UINT32 ui32StrPicId =
            GET_STREAM_PICTURE_ID(psCtxData->sNewRefPic.ui32TransactionId);

                        DEBUG_REPORT(REPORT_MODULE_DECODER,
                            "[TID=0x%08X] DISPLAY", psCtxData->sNewRefPic.ui32TransactionId);

                        ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                            ui32StrPicId,
                                                            IMG_TRUE);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }

                    }
                    if (bDiscardRefs)
                    {
                        if(psCtxData->sNewRefPic.ui32TransactionId)
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psCtxData->sNewRefPic.ui32TransactionId,
                                                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {

                                    return ui32Result;
                                }
                            }

                            psCtxData->sNewRefPic.ui32TransactionId = 0;
                        }
                        if(psCtxData->sOldRefPic.ui32TransactionId)
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psCtxData->sOldRefPic.ui32TransactionId,
                                                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                ui32Result =
                    decoder_DecodedPictureDestroy(psDecStrCtx,
                                                  psDecodedPict,
                                                  IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                            }

                            psCtxData->sOldRefPic.ui32TransactionId = 0;
                        }
                    }

    return IMG_SUCCESS;
}
#endif /* HAS_MPEG4 */

#ifdef HAS_VP6
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_VP6

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_VP6(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    VP6FW_sContextData * psCtxData = (VP6FW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;

    // Update the fw context for analysing the dpb in order to display
    // or release any outstanding picture
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

                    if (bDiscardRefs)
                    {
                        IMG_ASSERT(psCtxData->sLastPic.ui32TransactionId);
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psCtxData->sLastPic.ui32TransactionId,
                                                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                            }
                        }
                        if(psCtxData->sGoldenPic.ui32TransactionId &&
           (psCtxData->sGoldenPic.ui32TransactionId !=
            psCtxData->sLastPic.ui32TransactionId))
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psCtxData->sGoldenPic.ui32TransactionId,
                                                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                            }
                            psCtxData->sGoldenPic.ui32TransactionId = 0;
                        }
                        psCtxData->sGoldenPic.ui32TransactionId = 0;
                        psCtxData->sLastPic.ui32TransactionId = 0;
                    }

    return IMG_SUCCESS;
}

#endif /* HAS_VP6 */

#ifdef HAS_VP8
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_VP8

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_VP8(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    VP8FW_sContextData * psCtxData = (VP8FW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;

    // Update the fw context for analysing the dpb in order to display
    // or release any outstanding picture
    // If core 0 is fake, all are fake
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

                        if (bDiscardRefs)
                        {
        psDecodedPict =
            decoder_GetDecodedPict(psCtxData->sLastPic.ui32TransactionId,
                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
            ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                       psDecodedPict,
                                                       IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {

                                    return ui32Result;
                                }
                            }
                            psCtxData->sLastPic.ui32TransactionId = 0;

        psDecodedPict =
            decoder_GetDecodedPict(psCtxData->sGoldenPic.ui32TransactionId,
                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
            ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                       psDecodedPict,
                                                       IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {

                                    return ui32Result;
                                }
                            }
                            psCtxData->sGoldenPic.ui32TransactionId = 0;

        psDecodedPict =
            decoder_GetDecodedPict(psCtxData->sAltPic.ui32TransactionId,
            &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
            ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                       psDecodedPict,
                                                       IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {

                                    return ui32Result;
                                }
                            }
                            psCtxData->sAltPic.ui32TransactionId = 0;
                        }

    return IMG_SUCCESS;
}
#endif /* HAS_VP8 */

#ifdef HAS_MPEG2
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_MPEG2

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_MPEG2(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    MPEG2FW_sContextData * psCtxData = (MPEG2FW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;
                    DECODER_sDecodedPict * psDisplayPict;

    // Update the fw context for analysing the dpb in order to display
    // or release any outstanding picture
    // If core 0 is fake, all are fake
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

                    if(psCtxData->sLastBField.ui32TransactionId)
                    {
                        // We have an unpaired B field we need to display first.
        IMG_UINT32 ui32StrPicId =
            GET_STREAM_PICTURE_ID(psCtxData->sLastBField.ui32TransactionId);

                        DEBUG_REPORT(REPORT_MODULE_DECODER,
                     "[TID=0x%08X] DISPLAY",
                     psCtxData->sLastBField.ui32TransactionId);

                        ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                            ui32StrPicId,
                                                            IMG_TRUE);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }

        psDecodedPict =
            decoder_GetDecodedPict(psCtxData->sLastBField.ui32TransactionId,
                                &psDecStrCtx->sStrDecdPictList);
                        if (psDecodedPict)
                        {
            ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                       psDecodedPict,
                                                       IMG_FALSE);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return ui32Result;
                            }
                        }
                        psCtxData->sLastBField.ui32TransactionId = 0;
                    }


    if(psCtxData->sNewRefPic.ui32TransactionId &&
       (!psCtxData->sNewRefPic.bSentForDisplay))
                    {
        IMG_UINT32 ui32StrPicId;
                        DEBUG_REPORT(REPORT_MODULE_DECODER,
                     "[TID=0x%08X] DISPLAY",
                     psCtxData->sNewRefPic.ui32TransactionId);

        psDisplayPict =
            decoder_GetDecodedPict(psCtxData->sNewRefPic.ui32TransactionId,
                                   &psDecStrCtx->sStrDecdPictList);
        if (psDisplayPict &&
            psDisplayPict->psPicture &&
            psDisplayPict->psPicture->psDecPictInfo)
                        {
            if (FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                            VDECFW_BUFFLAG_DISPLAY_FIELD_CODED))
                            {
                if (!FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                    VDECFW_BUFFLAG_DISPLAY_SINGLE_FIELD))
                                {
                    psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        IMG_BUFFERTYPE_PAIR;
                    if (FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                                    VDECFW_BUFFLAG_DISPLAY_INTERLACED_FIELDS))
                                    {
                        psDisplayPict->psPicture->psDecPictInfo->bInterlacedFields =
                            IMG_TRUE;
                                    }
                                }
                                else
                                {
                                    psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        FLAG_IS_SET((psCtxData->sNewRefPic.ui8DisplayFlags),
                                    VDECFW_BUFFLAG_DISPLAY_BOTTOM_FIELD) ?
                        IMG_BUFFERTYPE_FIELD_BOTTOM :
                        IMG_BUFFERTYPE_FIELD_TOP;
                                }
                            }
                            else
                            {
                psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                    IMG_BUFFERTYPE_FRAME;
                            }
                        }
                        else
                        {
                            IMG_ASSERT(psDisplayPict);
            IMG_ASSERT(psDisplayPict && psDisplayPict->psPicture);
            IMG_ASSERT(psDisplayPict && psDisplayPict->psPicture && psDisplayPict->psPicture->psDecPictInfo);
                        }

        ui32StrPicId =
            GET_STREAM_PICTURE_ID(psCtxData->sNewRefPic.ui32TransactionId);

                        ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                            ui32StrPicId,
                                                            IMG_TRUE);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }
                    }

                    if (bDiscardRefs)
                    {
                        IMG_UINT32 ui32i;
        IMG_UINT32 aui32PicIdsToDestroy[] =
            { psCtxData->sNewRefPic.ui32TransactionId,
                                                              psCtxData->sOldRefPic.ui32TransactionId };

        for (ui32i = 0;
             ui32i < sizeof(aui32PicIdsToDestroy)/sizeof(IMG_UINT32);
             ui32i++)
                        {
                            if(aui32PicIdsToDestroy[ui32i])
                            {
                psDecodedPict =
                    decoder_GetDecodedPict(aui32PicIdsToDestroy[ui32i],
                                        &psDecStrCtx->sStrDecdPictList);
                                if (psDecodedPict)
                                {
                    ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                               psDecodedPict,
                                                               IMG_FALSE);
                                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                    if (ui32Result != IMG_SUCCESS)
                                    {
                                        return ui32Result;
                                    }
                                }
                            }
                        }

                        psCtxData->sNewRefPic.ui32TransactionId = 0;
                        psCtxData->sOldRefPic.ui32TransactionId = 0;
                    }

    return IMG_SUCCESS;
}

#endif /* HAS_MPEG2 */

#ifdef HAS_REAL
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_REAL

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_REAL(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    REALFW_sContextData * psCtxData = (REALFW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;

    // Update the fw context for analysing the dpb in order to display
    // or release any outstanding picture
    // If core 0 is fake, all are fake
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

    if(psCtxData->sNewRefPic.ui32TransactionId &&
       (!psCtxData->sNewRefPic.bSentForDisplay))
                    {
        IMG_UINT32 ui32StrPicId =
            GET_STREAM_PICTURE_ID(psCtxData->sNewRefPic.ui32TransactionId);
                        DEBUG_REPORT(REPORT_MODULE_DECODER,
                     "[TID=0x%08X] DISPLAY",
                     psCtxData->sNewRefPic.ui32TransactionId);

                        ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                            ui32StrPicId,
                                                            IMG_TRUE);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }

                    }
                    if (bDiscardRefs)
                    {
                        if(psCtxData->sNewRefPic.ui32TransactionId)
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psCtxData->sNewRefPic.ui32TransactionId,
                                                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {

                                    return ui32Result;
                                }
                            }

                            psCtxData->sNewRefPic.ui32TransactionId = 0;
                        }
                        if(psCtxData->sOldRefPic.ui32TransactionId)
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psCtxData->sOldRefPic.ui32TransactionId,
                                                                   &psDecStrCtx->sStrDecdPictList);
                            if (psDecodedPict)
                            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                            }

                            psCtxData->sOldRefPic.ui32TransactionId = 0;
                        }
                    }

    return IMG_SUCCESS;
}
#endif /* HAS_REAL */

#ifdef HAS_VC1
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_VC1

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_VC1(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

    VC1FW_sContextData * psCtxData = (VC1FW_sContextData *)
        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;
                    IMG_BOOL bFound = IMG_TRUE;
                    IMG_UINT32 i;
                    IMG_UINT32 ui32MinCountIdx;
                    IMG_UINT32 ui32NumDisplayPics = 0;
                    IMG_UINT32 ui32NumPicsDisplayed = 0;
                    DECODER_sDecodedPict* psDisplayPict = IMG_NULL;

    // Update the fw context for analysing the references in order to display
    // or release any outstanding picture
    // If core 0 is fake, all are fake
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

                    // Determine how many display pictures reside in the DPB.
                    for (i = 1; i < VC1FW_MAX_DPB_SIZE; i++)
                    {
                        if (psCtxData->asDpb[i].ui32TransactionId)
                        {
                            //Always needed for output in VC-1
                            ui32NumDisplayPics++;
                        }
                    }

    // Destroy the picture from VC1FW_DPB_OLDREF_SLOT, it has been
    // already displayed
                    if (bDiscardRefs && psCtxData->asDpb[0].ui32TransactionId)
                    {
        psDecodedPict =
            decoder_GetDecodedPict(psCtxData->asDpb[0].ui32TransactionId,
                                   &psDecStrCtx->sStrDecdPictList);
                        if (psDecodedPict)
                        {
            ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                       psDecodedPict,
                                                       IMG_FALSE);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return ui32Result;
                            }
                        }
                        psCtxData->asDpb[0].ui32TransactionId = 0;
                    }

                    while (bFound)
                    {
                        ui32MinCountIdx = -1;
                        bFound = IMG_FALSE;

        // Loop over the DPB to find the first in order, starting
        // from VC1FW_DPB_NEWREF_SLOT
                        for (i = 1; i < VC1FW_MAX_DPB_SIZE && !bFound; i++)
                        {
                            if (psCtxData->asDpb[i].ui32TransactionId)
                            {
                                if ( bDiscardRefs )
                                {
                                    ui32MinCountIdx = i;
                                    bFound = IMG_TRUE;
                                }
                            }
                        }

                        if (bFound)
                        {
            IMG_UINT32 ui32MinCountTID =
                psCtxData->asDpb[ui32MinCountIdx].ui32TransactionId;

                            DEBUG_REPORT(REPORT_MODULE_DECODER,
                                "[USERSID=0x%08X] [TID=0x%08X] DISPLAY",
                                psDecStrCtx->sConfig.ui32UserStrId,
                ui32MinCountTID);

            IMG_ASSERT(ui32MinCountTID != 0xffffffff);

            psDisplayPict =
                decoder_GetDecodedPict(ui32MinCountTID,
                                       &psDecStrCtx->sStrDecdPictList);

            if (psDisplayPict &&
                psDisplayPict->psPicture &&
                psDisplayPict->psPicture->psDecPictInfo)
                            {
                if (FLAG_IS_SET((psCtxData->asDpb[ui32MinCountIdx].ui8DisplayFlags),
                    VDECFW_BUFFLAG_DISPLAY_FIELD_CODED))
                                {
                    if (!FLAG_IS_SET((psCtxData->asDpb[ui32MinCountIdx].ui8DisplayFlags),
                        VDECFW_BUFFLAG_DISPLAY_SINGLE_FIELD))
                                    {
                        psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                            IMG_BUFFERTYPE_PAIR;
                                    }
                                    else
                                    {
                                        psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                            FLAG_IS_SET((psCtxData->asDpb[ui32MinCountIdx].ui8DisplayFlags),
                                        VDECFW_BUFFLAG_DISPLAY_BOTTOM_FIELD) ?
                            IMG_BUFFERTYPE_FIELD_BOTTOM :
                            IMG_BUFFERTYPE_FIELD_TOP;
                                    }
                                }
                                else
                                {
                    psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                        IMG_BUFFERTYPE_FRAME;
                                }
                            }
                            else
                            {
                                IMG_ASSERT(psDisplayPict);
                IMG_ASSERT(psDisplayPict && psDisplayPict->psPicture);
                IMG_ASSERT(psDisplayPict && psDisplayPict->psPicture && psDisplayPict->psPicture->psDecPictInfo);
                            }

                            ui32NumPicsDisplayed++;
            ui32Result =
                decoder_PictureDisplay(psDecStrCtx,
                                       GET_STREAM_PICTURE_ID(ui32MinCountTID),
                                       (ui32NumPicsDisplayed == ui32NumDisplayPics) ?
                                       IMG_TRUE : IMG_FALSE);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return ui32Result;
                            }

                            if (bDiscardRefs)
                            {
                psDecodedPict =
                    decoder_GetDecodedPict(ui32MinCountTID,
                                           &psDecStrCtx->sStrDecdPictList);
                                if (psDecodedPict)
                                {
                    ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                               psDecodedPict,
                                                               IMG_FALSE);
                                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                    if (ui32Result != IMG_SUCCESS)
                                    {
                                        return ui32Result;
                                    }
                                }
                                psCtxData->asDpb[ui32MinCountIdx].ui32TransactionId = 0;
                            }
                        }
                    }

                    IMG_ASSERT(ui32NumPicsDisplayed == ui32NumDisplayPics);

    return IMG_SUCCESS;
}
#endif /* HAS_VC1 */

#ifdef HAS_HEVC
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB_HEVC

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB_HEVC(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result;

                    HEVCFW_sContextData *psCtx = (HEVCFW_sContextData *)
                        psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt;
                    HEVCFW_sDecodedPictureBuffer * psDpb;

                    IMG_BOOL bFound = IMG_TRUE;
                    HEVCFW_DpbIdx iIdx;
                    HEVCFW_Poc iMinPOCVal;
                    HEVCFW_DpbIdx iDpbIdx;
                    IMG_UINT8 ui8NumDisplayPics = 0;
                    IMG_UINT8 ui8NumPicsDisplayed = 0;
                    DECODER_sDecodedPict* psDisplayPict = IMG_NULL;

                    // Update the fw context for analysing the dpb in order
                    // to display or release any outstanding picture
    UPDATE_HOST(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);

                    psDpb = &psCtx->sDpb;

                    // Determine how many display pictures reside in the DPB.
                    for (iIdx = 0; iIdx < HEVCFW_MAX_DPB_SIZE; ++iIdx)
                    {
                        HEVCFW_sPictureInDpb * psDpbPic = &psDpb->asPictures[iIdx];
                        if (psDpbPic->bValid && psDpbPic->bNeededForOutput)
                        {
                            ++ui8NumDisplayPics;
                        }
                    }

                    while (bFound)
                    {
                        HEVCFW_sPictureInDpb * psDpbPic;
                        iMinPOCVal = 0x7fffffff;
                        iDpbIdx = HEVCFW_DPB_IDX_INVALID;
                        bFound = IMG_FALSE;

                        // Loop over the DPB to find the first in order
                        for (iIdx = 0; iIdx < HEVCFW_MAX_DPB_SIZE; ++iIdx)
                        {
                            psDpbPic = &psDpb->asPictures[iIdx];
                            if (psDpbPic->bValid &&
                                (psDpbPic->bNeededForOutput || bDiscardRefs))
                            {
                                if (psDpbPic->sPicture.iPicOrderCntVal < iMinPOCVal)
                                {
                                    iMinPOCVal = psDpbPic->sPicture.iPicOrderCntVal;
                                    iDpbIdx = iIdx;
                                    bFound = IMG_TRUE;
                                }
                            }
                        }

                        if (!bFound)
                        {
                            break;
                        }

                        psDpbPic = &psDpb->asPictures[iDpbIdx];

                        if (psDpbPic->bNeededForOutput)
                        {
            IMG_UINT32 ui32StrPicId =
                GET_STREAM_PICTURE_ID(psDpbPic->sPicture.ui32TransactionId);

                            DEBUG_REPORT(REPORT_MODULE_DECODER,
                                "[USERSID=0x%08X] [TID=0x%08X] DISPLAY",
                                psDecStrCtx->sConfig.ui32UserStrId,
                                psDpbPic->sPicture.ui32TransactionId);

                            IMG_ASSERT(psDpbPic->sPicture.ui32TransactionId != 0xffffffff);

            psDisplayPict =
                decoder_GetDecodedPict(psDpbPic->sPicture.ui32TransactionId,
                                                &psDecStrCtx->sStrDecdPictList);

            if (psDisplayPict &&
                psDisplayPict->psPicture &&
                                psDisplayPict->psPicture->psDecPictInfo)
                            {
                                psDisplayPict->psPicture->psDecPictInfo->eBufferType =
                                                                IMG_BUFFERTYPE_FRAME;
                            }
                            else
                            {
                                IMG_ASSERT(psDisplayPict);
                IMG_ASSERT(psDisplayPict && psDisplayPict->psPicture);
                IMG_ASSERT(psDisplayPict && psDisplayPict->psPicture && psDisplayPict->psPicture->psDecPictInfo);
                            }

                            ++ui8NumPicsDisplayed;
            ui32Result =
                decoder_PictureDisplay(psDecStrCtx,
                                       ui32StrPicId,
                                    ui8NumPicsDisplayed == ui8NumDisplayPics);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return ui32Result;
                            }
                            psDpbPic->bNeededForOutput = IMG_FALSE;
                        }

                        if (bDiscardRefs)
                        {
            psDecodedPict =
                decoder_GetDecodedPict(psDpbPic->sPicture.ui32TransactionId,
                                    &psDecStrCtx->sStrDecdPictList);

                            if (psDecodedPict)
                            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                                if (ui32Result != IMG_SUCCESS)
                                {
                                    return ui32Result;
                                }
                            }

                            psDpbPic->bValid = IMG_FALSE;
                        }
                    }

                    IMG_ASSERT(ui8NumPicsDisplayed == ui8NumDisplayPics);

    return IMG_SUCCESS;
}
#endif /* HAS_HEVC */

/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPBRegular

 @Description

 Process DPB derived from stream CTX, display and release relevant pictures.
 This functions is used in regular, non-secure flow, when firmware context
 is available to host.

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPBRegular(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result = IMG_SUCCESS;

    if (psDecodedPict)
    {
        switch(psDecStrCtx->sConfig.eVidStd)
        {
#ifdef HAS_AVS
            case VDEC_STD_AVS:
            {
                ui32Result = decoder_StreamFlushProcessDPB_AVS(psDecStrCtx,
                                                               psDecodedPict,
                                                               bDiscardRefs);
                break;
            }
#endif /* HAS_AVS */
#ifdef HAS_H264
            case VDEC_STD_H264:
            {
                ui32Result = decoder_StreamFlushProcessDPB_H264(psDecStrCtx,
                                                                psDecodedPict,
                                                                bDiscardRefs);

                break;
            }
#endif /* HAS_H264 */

#ifdef HAS_MPEG4
            case VDEC_STD_MPEG4:
            case VDEC_STD_H263:
            case VDEC_STD_SORENSON:
            {
                ui32Result = decoder_StreamFlushProcessDPB_MPEG4(psDecStrCtx,
                                                                 psDecodedPict,
                                                                 bDiscardRefs);

                break;
            }
#endif /* HAS_MPEG4 */

#ifdef HAS_VP6
            case VDEC_STD_VP6:
            {
                ui32Result = decoder_StreamFlushProcessDPB_VP6(psDecStrCtx,
                                                               psDecodedPict,
                                                               bDiscardRefs);
                    break;
                }
#endif /* HAS_VP6 */
#ifdef HAS_VP8
            case VDEC_STD_VP8:
            {
                ui32Result = decoder_StreamFlushProcessDPB_VP8(psDecStrCtx,
                                                               psDecodedPict,
                                                               bDiscardRefs);
                break;
            }
#endif /* HAS_VP8 */
#ifdef HAS_MPEG2
            case VDEC_STD_MPEG2:
            {
                ui32Result = decoder_StreamFlushProcessDPB_MPEG2(psDecStrCtx,
                                                                 psDecodedPict,
                                                                 bDiscardRefs);
                break;
            }
#endif /* HAS_MPEG2 */
#ifdef HAS_REAL
            case VDEC_STD_REAL:
            {
                ui32Result = decoder_StreamFlushProcessDPB_REAL(psDecStrCtx,
                                                                psDecodedPict,
                                                                bDiscardRefs);
                break;
            }
#endif /* HAS_REAL */
#ifdef HAS_VC1
            case VDEC_STD_VC1:
            {
                ui32Result = decoder_StreamFlushProcessDPB_VC1(psDecStrCtx,
                                                               psDecodedPict,
                                                               bDiscardRefs);
                break;
            }
#endif /* HAS_VC1 */
#ifdef HAS_HEVC
            case VDEC_STD_HEVC:
            {
                ui32Result = decoder_StreamFlushProcessDPB_HEVC(psDecStrCtx,
                                                                psDecodedPict,
                                                                bDiscardRefs);
                break;
            }
#endif /* HAS_HEVC */

                default:
                    break;
            }
        }

    return ui32Result;
}

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPBFWBSP

 @Description

 Process DPB derived from firmware messages, display and release relevant
 pictures.
 This functions is used in FWBSP flow, secure flow, when firmware context
 is not available to host and DPB state is sent in each NEW_PIC message.

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPBFWBSP(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{
    IMG_RESULT ui32Result = IMG_SUCCESS;

    IMG_UINT8 ui8i;
    IMG_UINT8 ui8DPBDisPicLen = psDecStrCtx->sLastDPBState.ui8DPBDisPicLen;

    DECODER_sDecodedPict * psDisplayPict = IMG_NULL;

    for (ui8i = 0; ui8i < ui8DPBDisPicLen; ui8i++)
    {
        IMG_UINT32 ui32TID = psDecStrCtx->sLastDPBState.aui32DPBDisList[ui8i];
        IMG_UINT32 ui32StrPicId = GET_STREAM_PICTURE_ID(ui32TID);
        IMG_UINT32 ui32Mask = (1 << ui8i);
        IMG_BOOL bLast = ((ui8i == (ui8DPBDisPicLen - 1)));
        IMG_BOOL bFieldCoded =
            (psDecStrCtx->sLastDPBState.ui32DPBFieldCodedMask & ui32Mask);
        IMG_BOOL bSingleField =
            (psDecStrCtx->sLastDPBState.ui32DPBSingleFieldMask & ui32Mask);
        IMG_BOOL bBotField =
            (psDecStrCtx->sLastDPBState.ui32DPBBotFieldMask & ui32Mask);

        psDisplayPict = decoder_GetDecodedPict(ui32TID,
                                               &psDecStrCtx->sStrDecdPictList);

        if (psDisplayPict &&
            psDisplayPict->psPicture &&
            psDisplayPict->psPicture->psDecPictInfo)
        {
            IMG_eBufferType eBufferType;

            if (bFieldCoded)
            {
                if (!bSingleField)
                {
                    eBufferType = IMG_BUFFERTYPE_PAIR;
                }
                else
                {
                    eBufferType = bBotField ? IMG_BUFFERTYPE_FIELD_BOTTOM :
                                              IMG_BUFFERTYPE_FIELD_TOP;
                }
            }
            else
            {
                eBufferType = IMG_BUFFERTYPE_FRAME;
            }
            psDisplayPict->psPicture->psDecPictInfo->eBufferType = eBufferType;
        }
        else if(psDecStrCtx->sConfig.eVidStd == VDEC_STD_HEVC)
        {
            //valid case for some strange streams (e.g. #58462)
            //but do not process it any further
            continue;
        }
        else
        {
            IMG_ASSERT(psDisplayPict);
            IMG_ASSERT(psDisplayPict &&
                       psDisplayPict->psPicture);
            IMG_ASSERT(psDisplayPict &&
                       psDisplayPict->psPicture &&
                       psDisplayPict->psPicture->psDecPictInfo);
        }

        ui32Result = decoder_PictureDisplay(psDecStrCtx,
                                            ui32StrPicId,
                                            bLast);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        if (bDiscardRefs)
        {
            psDecodedPict =
                decoder_GetDecodedPict(ui32TID,
                                       &psDecStrCtx->sStrDecdPictList);

            if (psDecodedPict)
            {
                ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx,
                                                           psDecodedPict,
                                                           IMG_FALSE);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
    }
            }
        }
    }

    return IMG_SUCCESS;
}
#endif /* VDEC_USE_PVDEC_SEC */
#endif
/*!
******************************************************************************

 @Function              decoder_StreamFlushProcessDPB

 @Description

 Process DPB fetched from firmware, display and release relevant pictures.

******************************************************************************/
static IMG_RESULT
decoder_StreamFlushProcessDPB(
    DECODER_sStrContext     * psDecStrCtx,
    DECODER_sDecodedPict    * psDecodedPict,
    IMG_BOOL                  bDiscardRefs
)
{

#ifdef VDEC_MSVDX_HARDWARE
    if (psDecStrCtx->psDecCtx->psDevConfig->eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
    {
        // Get oldest reference to display.
        psDecodedPict = DQ_last(&psDecStrCtx->sStrDecdPictList);
#ifdef VDEC_USE_PVDEC_SEC
        if (psDecStrCtx->sConfig.bSecureStream)
        {

            return decoder_StreamFlushProcessDPBFWBSP(psDecStrCtx,
                                                      psDecodedPict,
                                                      bDiscardRefs);
        }
        else
#endif /* VDEC_USE_PVDEC_SEC */
        {
            return decoder_StreamFlushProcessDPBRegular(psDecStrCtx,
                                                        psDecodedPict,
                                                        bDiscardRefs);
        }
    }
    else
#endif
    {
        IMG_ASSERT(psDecodedPict == IMG_NULL);
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_StreamFlush

 @Description

 Flush all display images from reference picture list and release all
 references held (if requested). Firmware stream context destroyed?

******************************************************************************/
static IMG_RESULT
decoder_StreamFlush(
    DECODER_sStrContext *   psDecStrCtx,
    IMG_BOOL                bDiscardRefs
)
{
    DECODER_sDecStrUnit     * psDecStrUnit;
    DECODER_sDecodedPict    * psDecodedPict;
    IMG_RESULT                ui32Result;

    IMG_ASSERT(psDecStrCtx);

    // Since the stream should be stopped before flushing
    // there should be no pictures in the stream list.
    psDecStrUnit = LST_first(&psDecStrCtx->sPendStrUnitList);
    while (psDecStrUnit)
    {
        IMG_ASSERT(psDecStrUnit->psStrUnit->eStrUnitType != VDECDD_STRUNIT_PICTURE_START);
        psDecStrUnit = LST_next(psDecStrUnit);
    }

    psDecodedPict = DQ_last(&psDecStrCtx->sStrDecdPictList);


    ui32Result = decoder_StreamFlushProcessDPB(psDecStrCtx,
                                               psDecodedPict,
                                               bDiscardRefs);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    if (bDiscardRefs)
    {
        while (!DQ_empty(&psDecStrCtx->sStrDecdPictList))
        {
            DECODER_sDecodedPict * psNonDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
            REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
                "[USERSID=0x%08X] Decoded picture list contains item ID:0x%08x when DPB is empty",
                psDecStrCtx->sConfig.ui32UserStrId, psNonDecodedPict->ui32TransactionId);
            ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx, psNonDecodedPict, IMG_FALSE);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }
        IMG_ASSERT(DQ_empty(&psDecStrCtx->sStrDecdPictList));

        if (psDecStrCtx->psLastBePictDecRes)
        {
#ifndef IMG_KERNEL_MODULE
            DECODER_sCoreContext * psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);

            IMG_ASSERT(psDecCoreCtx);

            // Synchronise before memseting last context structure.
            HWCTRL_SyncPDumpContexts(psDecCoreCtx->hHwCtx);
#endif
            // Clear the firmware context so that reference pictures are no longer referred to.
            IMG_MEMSET(psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt, 0, psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.ui32BufSize);
            UPDATE_DEVICE(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);
        }
    }

    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              DECODER_StreamFlush

******************************************************************************/
IMG_RESULT
DECODER_StreamFlush(
    IMG_HANDLE hDecStrCtx,
    IMG_BOOL bDiscardRefs
)
{
    DECODER_sStrContext * psDecStrCtx;
    IMG_RESULT ui32Result;

    psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    ui32Result = decoder_StreamFlush(psDecStrCtx, bDiscardRefs);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              DECODER_StreamReleaseBuffers

******************************************************************************/
IMG_RESULT
DECODER_StreamReleaseBuffers(
    IMG_HANDLE hDecStrCtx
)
{
    DECODER_sStrContext * psDecStrCtx;
    DECODER_sDecodedPict * psDecodedPict;
    IMG_RESULT ui32Result;

    psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);

    // Decoding queue should be empty since we are stopped.
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }
    IMG_ASSERT(LST_empty(&psDecStrCtx->sPendStrUnitList));

    // Destroy all pictures in the decoded list
    psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    while (psDecodedPict)
    {
        ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx, psDecodedPict, IMG_TRUE);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    }

    // XXX-matt: the stream context must be cleared of everything right now
    // if and only if the output buffers were used for reference.
    //IMG_ASSERT("Stream context must be cleared on an output buffer release" == IMG_NULL);

    if (psDecStrCtx->psLastBePictDecRes)
    {
#ifndef IMG_KERNEL_MODULE
        DECODER_sCoreContext * psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);

        IMG_ASSERT(psDecStrCtx);

        // Synchronise before memseting last context structure.
        HWCTRL_SyncPDumpContexts(psDecCoreCtx->hHwCtx);
#endif
        // Clear the firmware context so that reference pictures are no longer referred to.
        IMG_MEMSET(psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.pvCpuVirt, 0, psDecStrCtx->psLastBePictDecRes->sFwCtxBuf.ui32BufSize);
        UPDATE_DEVICE(&psDecStrCtx->psLastBePictDecRes->sFwCtxBuf);
    }

    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              decoder_StreamReferenceResourceDestroy

******************************************************************************/
static IMG_RESULT
decoder_StreamReferenceResourceDestroy(
    IMG_VOID  * pvItem,
    IMG_VOID  * pvFreeCbParam
)
{
    DECODER_sPictRefRes * psPictRefRes = pvItem;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psPictRefRes);
    IMG_ASSERT(RESOURCE_ItemIsAvailable(&psPictRefRes->ui32RefCount));

    // Free memory (device-based) to store fw contexts for stream.
    ui32Result = MMU_FreeMem(&psPictRefRes->sFwCtrlBuf);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    IMG_FREE(psPictRefRes);
    psPictRefRes = IMG_NULL;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              decoder_StreamReferenceResourceCreate

******************************************************************************/
static IMG_RESULT
decoder_StreamReferenceResourceCreate(
    DECODER_sStrContext   * psDecStrCtx,
    IMG_VOID              * pvFreeCbParam
)
{
    DECODER_sPictRefRes * psPictRefRes;
    IMG_BOOL bFwCtrlBuf = IMG_FALSE;
    IMG_RESULT ui32Result;
    VXDIO_sMemPool sInsecureMemPool;

    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psDecStrCtx && psDecStrCtx->psDecCtx);

    sInsecureMemPool.eMemPoolId = psDecStrCtx->psDecCtx->eInsecurePool;
    sInsecureMemPool.eMemPoolType = VXDIO_MEMPOOL_INSECURE;

    /* Allocate the firmware context buffer info structure. */
    VDEC_MALLOC(psPictRefRes);
    IMG_ASSERT(psPictRefRes);
    if (psPictRefRes == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for stream reference resource",
                psDecStrCtx->sConfig.ui32UserStrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psPictRefRes);

    /* Allocate the firmware context buffer to contain data required for subsequent picture. */
    // XXX: improve memory use by sharing a 4k page between multiple resources
    ui32Result = MMU_StreamMalloc(psDecStrCtx->hMmuStrHandle,
                                  MMU_HEAP_STREAM_BUFFERS,
                                  sInsecureMemPool, //FW Control always in insecure memory
                                  sizeof(VDECFW_sBufferControl),
                                  DEV_MMU_PAGE_ALIGNMENT,
                                  &psPictRefRes->sFwCtrlBuf);
    IMG_ASSERT(IMG_SUCCESS == ui32Result);
    if (IMG_SUCCESS != ui32Result)
    {
        goto err_out_of_memory;
    }
    bFwCtrlBuf = IMG_TRUE;

    // Clear the context data in preparation for first time use by the firmware.
    IMG_MEMSET(psPictRefRes->sFwCtrlBuf.pvCpuVirt, 0, psPictRefRes->sFwCtrlBuf.ui32BufSize);
    UPDATE_DEVICE(&psPictRefRes->sFwCtrlBuf);

    psPictRefRes->ui32RefCount = 1;

    ui32Result = RESOURCE_ListAdd(&psDecStrCtx->sRefResList,
                      psPictRefRes,
                      0,
                      &psPictRefRes->ui32RefCount);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to add resource",
            psDecStrCtx->sConfig.ui32UserStrId);
    }

    return IMG_SUCCESS;

err_out_of_memory:
    if (psPictRefRes)
    {
        if (bFwCtrlBuf)
        {
            MMU_FreeMem(&psPictRefRes->sFwCtrlBuf);
        }

        IMG_FREE(psPictRefRes);
        psPictRefRes = IMG_NULL;
    }

    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
        "[USERSID=0x%08X] Failed to allocate device memory for stream reference resources",
                psDecStrCtx->sConfig.ui32UserStrId);

    return IMG_ERROR_OUT_OF_MEMORY;
}



/*!
******************************************************************************

 @Function              decoder_StreamDecodeResourceDestroy

******************************************************************************/
static IMG_RESULT
decoder_StreamDecodeResourceDestroy(
    IMG_VOID * pvItem,
    IMG_VOID * pvFreeCbParam
)
{
    DECODER_sPictDecRes * psPictDecRes = pvItem;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psPictDecRes);
    IMG_ASSERT(RESOURCE_ItemIsAvailable(&psPictDecRes->ui32RefCount));

    // Free memory (device-based) to store fw contexts for stream.
    ui32Result = MMU_FreeMem(&psPictDecRes->sFwCtxBuf);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

#ifdef HAS_H264
    if (psPictDecRes->sH264SgmBuf.hMemory)
    {
        // Free memory (device-based) to store SGM.
        ui32Result = MMU_FreeMem(&psPictDecRes->sH264SgmBuf);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
#endif /* HAS_H264 */

#ifdef HAS_MPEG4
    if (psPictDecRes->sMPEG4FEVLRBackup.hMemory)
    {
        // Free memory (device-based) to store FE VLR Backup.
        ui32Result = MMU_FreeMem(&psPictDecRes->sMPEG4FEVLRBackup);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
#endif /* HAS_MPEG4 */

    IMG_FREE(psPictDecRes);
    psPictDecRes = IMG_NULL;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_StreamDecodeResourceCreate

******************************************************************************/
static IMG_RESULT
decoder_StreamDecodeResourceCreate(
    DECODER_sStrContext * psDecStrCtx
)
{
    DECODER_sPictDecRes * psPictDecRes;
    IMG_BOOL bFwCtxBuf = IMG_FALSE;
    IMG_RESULT ui32Result;
    VXDIO_sMemPool sMemPool;
    VXDIO_sMemPool sInsecureMemPool;
    const VDECDD_sDdDevConfig * psDevConfig;
#ifdef HAS_H264
    IMG_BOOL bH264SgmBuf = IMG_FALSE;
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
    IMG_BOOL bMPEG4FEVLRBackup = IMG_FALSE;
#endif /* HAS_MPEG4 */

    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psDecStrCtx && psDecStrCtx->psDecCtx);
    psDevConfig = psDecStrCtx->psDecCtx->psDevConfig;
    IMG_ASSERT(psDevConfig);

    sMemPool.eMemPoolId = (psDevConfig->bSecureMemoryAvailable && psDecStrCtx->sConfig.bSecureStream) ?
        psDecStrCtx->psDecCtx->eSecurePool : psDecStrCtx->psDecCtx->eInsecurePool;
    sMemPool.eMemPoolType = (psDevConfig->bSecureMemoryAvailable && psDecStrCtx->sConfig.bSecureStream) ?
        VXDIO_MEMPOOL_SECURE : VXDIO_MEMPOOL_INSECURE;

    sInsecureMemPool.eMemPoolId = psDecStrCtx->psDecCtx->eInsecurePool;
    sInsecureMemPool.eMemPoolType = VXDIO_MEMPOOL_INSECURE;

    /* Allocate the firmware context buffer info structure. */
    VDEC_MALLOC(psPictDecRes);
    IMG_ASSERT(psPictDecRes);
    if (psPictDecRes == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for stream decode resource",
                psDecStrCtx->sConfig.ui32UserStrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psPictDecRes);

    /* Allocate the firmware context buffer to contain
       data required for subsequent picture. */
    ui32Result = MMU_StreamMalloc(psDecStrCtx->hMmuStrHandle,
                                  MMU_HEAP_STREAM_BUFFERS,
                                  sInsecureMemPool, //FW Context always in insecure memory
                                  sizeof(DECODER_uFwContexts),
                                  DEV_MMU_PAGE_ALIGNMENT,
                                  &psPictDecRes->sFwCtxBuf);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto err_out_of_memory;
    }
    bFwCtxBuf = IMG_TRUE;

    // Clear the context data in preparation for first time use by the firmware.
    IMG_MEMSET(psPictDecRes->sFwCtxBuf.pvCpuVirt, 0, psPictDecRes->sFwCtxBuf.ui32BufSize);
    UPDATE_DEVICE(&psPictDecRes->sFwCtxBuf);

    switch (psDecStrCtx->sConfig.eVidStd)
    {
#ifdef HAS_H264
    case VDEC_STD_H264:
        /* Allocate the SGM buffer */
        ui32Result = MMU_StreamMalloc(psDecStrCtx->hMmuStrHandle,
                                      MMU_HEAP_STREAM_BUFFERS,
                                      sMemPool, //Secure if Secure Content and secure allocation is possible
                                      H264_SGM_BUFFER_BYTES_PER_MB * H264_SGM_MAX_MBS,
                                      DEV_MMU_PAGE_ALIGNMENT,
                                      &psPictDecRes->sH264SgmBuf);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto err_out_of_memory;
        }
        bH264SgmBuf = IMG_TRUE;

        if (!psDevConfig->bSecureMemoryAvailable || !psDecStrCtx->sConfig.bSecureStream)
        {
            // Clear the SGM data.
            IMG_MEMSET(psPictDecRes->sH264SgmBuf.pvCpuVirt, 0, psPictDecRes->sH264SgmBuf.ui32BufSize);
            UPDATE_DEVICE(&psPictDecRes->sH264SgmBuf);
        }
        break;
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
    case VDEC_STD_MPEG4:
    case VDEC_STD_H263:
    case VDEC_STD_SORENSON:
        /* Allocate buffer to backup not coded mpeg4 flags */
        ui32Result = MMU_StreamMalloc(psDecStrCtx->hMmuStrHandle,
                                      MMU_HEAP_STREAM_BUFFERS,
                                      sMemPool, //Secure if Secure Content and secure allocation is possible
                                      MPEG4_FE_VRL_NOT_CODED,
                                      DEV_MMU_PAGE_ALIGNMENT,
                                      &psPictDecRes->sMPEG4FEVLRBackup);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto err_out_of_memory;
        }
        bMPEG4FEVLRBackup = IMG_TRUE;

        if (!psDevConfig->bSecureMemoryAvailable || !psDecStrCtx->sConfig.bSecureStream)
        {
            // Clear the MPEG4 FE VLR data.
            IMG_MEMSET(psPictDecRes->sMPEG4FEVLRBackup.pvCpuVirt, 0, psPictDecRes->sMPEG4FEVLRBackup.ui32BufSize);
            UPDATE_DEVICE(&psPictDecRes->sMPEG4FEVLRBackup);
        }
        break;
#endif /* HAS_MPEG4 */

    default:
        break;
    }

    psPictDecRes->ui32RefCount = 1;

    ui32Result = RESOURCE_ListAdd(&psDecStrCtx->sDecResList,
                                  psPictDecRes,
                                  0,
                                  &psPictDecRes->ui32RefCount);

    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
               "[USERSID=0x%08X] Failed to add resource",
               psDecStrCtx->sConfig.ui32UserStrId);
    }


    return IMG_SUCCESS;

err_out_of_memory:
    if (psPictDecRes)
    {
        if (bFwCtxBuf)
        {
            MMU_FreeMem(&psPictDecRes->sFwCtxBuf);
        }
#ifdef HAS_H264
        if (bH264SgmBuf)
        {
            MMU_FreeMem(&psPictDecRes->sH264SgmBuf);
        }
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
        if (bMPEG4FEVLRBackup)
        {
            MMU_FreeMem(&psPictDecRes->sMPEG4FEVLRBackup);
        }
#endif /* HAS_MPEG4 */
        IMG_FREE(psPictDecRes);
        psPictDecRes = IMG_NULL;
    }

    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
           "[USERSID=0x%08X] Failed to allocate device memory for stream decode resources",
           psDecStrCtx->sConfig.ui32UserStrId);

    return IMG_ERROR_OUT_OF_MEMORY;
}



/*!
******************************************************************************

 @Function              DECODER_StreamGetStatus

******************************************************************************/
IMG_RESULT
DECODER_StreamGetStatus(
    IMG_HANDLE hDecStrCtx,
    VDECDD_sDecStrStatus * psDecStrStatus
)
{
    DECODER_sStrContext   * psDecStrCtx;
    DECODER_sDecodedPict  * psDecodedPict;
    DECODER_sDecStrUnit   * psDecStrUnit;
    DECODER_sCoreContext  * psDecCoreCtx;
    IMG_RESULT              ui32Result;
    IMG_UINT32              ui32Item;
    IMG_UINT32              i;
    HWCTRL_sState           sCoreState;
    IMG_UINT32 aui32NumFreeSlots[VDEC_MAX_PIXEL_PIPES];
    IMG_UINT32 aui32PipeLoad[VDEC_MAX_PIXEL_PIPES];

    IMG_ASSERT(psDecStrStatus);
    if (psDecStrStatus == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder streams status pointer!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    // Obtain the loading and free slots on most available core.
    decoder_GetPipeAndLoad(psDecStrCtx,
                           psDecStrCtx->sDecStrStatus.ui32Features,
                           &psDecStrCtx->sDecStrStatus.ui32MinAvailLoad,
                           &psDecStrCtx->sDecStrStatus.ui32MinLoad,
                           IMG_NULL);

    // Obtain the state of each core.
    psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);
    IMG_ASSERT(psDecCoreCtx);
        ui32Result = HWCTRL_GetCoreStatus(psDecCoreCtx->hHwCtx,
                                          aui32NumFreeSlots,
                                          aui32PipeLoad,
                                        &sCoreState);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }


    // Obtain the display and release list of first unprocessed picture in decoded list
    psDecStrCtx->sDecStrStatus.ui32DisplayPics = 0;
    psDecStrCtx->sDecStrStatus.ui32ReleasePics = 0;
    psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    while (psDecodedPict)
    {
        // if this is the first unprocessed picture
        if (!psDecodedPict->bProcessed)
        {
            IMG_UINT32 ui32Idx;
            VDECFW_sBufferControl * psBufCtrl;

            IMG_ASSERT(psDecodedPict->psPictRefRes);
            psBufCtrl = (VDECFW_sBufferControl *)psDecodedPict->psPictRefRes->sFwCtrlBuf.pvCpuVirt;
            IMG_ASSERT(psBufCtrl);

            //Update the fw context for analysing the upcoming display picture (for dumping we need to know which is the next picture coming for display)
            UPDATE_HOST(&psDecodedPict->psPictRefRes->sFwCtrlBuf);

            // Get display pictures
            ui32Idx = psDecodedPict->ui32DisplayIdx;
            ui32Item = psDecStrCtx->sDecStrStatus.ui32DisplayPics;
            while (ui32Idx < psBufCtrl->ui32DisplayListLength &&
                   ui32Item < VDECFW_MAX_NUM_PICTURES)
            {
                psDecStrCtx->sDecStrStatus.aui32NextDisplayItems[ui32Item] = psBufCtrl->aui32DisplayList[ui32Idx];
                psDecStrCtx->sDecStrStatus.aui32NextDisplayItemParent[ui32Item] = psDecodedPict->ui32TransactionId;
                ui32Idx++;
                ui32Item++;
            }
            psDecStrCtx->sDecStrStatus.ui32DisplayPics = ui32Item;

            // Get release pictures
            ui32Idx = psDecodedPict->ui32ReleaseIdx;
            ui32Item = psDecStrCtx->sDecStrStatus.ui32ReleasePics;
            while (ui32Idx < psBufCtrl->ui32ReleaseListLength &&
                   ui32Item < VDECFW_MAX_NUM_PICTURES)
            {
                psDecStrCtx->sDecStrStatus.aui32NextReleaseItems[ui32Item] = psBufCtrl->aui32ReleaseList[ui32Idx];
                psDecStrCtx->sDecStrStatus.aui32NextReleaseItemParent[ui32Item] = psDecodedPict->ui32TransactionId;
                ui32Idx++;
                ui32Item++;
            }
            psDecStrCtx->sDecStrStatus.ui32ReleasePics = ui32Item;
            break;
        }

        if (psDecodedPict != DQ_last(&psDecStrCtx->sStrDecdPictList))
        {
            psDecodedPict = DQ_next(psDecodedPict);
        }
        else
        {
            psDecodedPict = IMG_NULL;
        }
    }

    // Get list of held decoded pictures.
    ui32Item = 0;
    psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    while (psDecodedPict)
    {
        psDecStrCtx->sDecStrStatus.aui32DecodedPicts[ui32Item] = psDecodedPict->ui32TransactionId;
        ui32Item++;

        if (psDecodedPict != DQ_last(&psDecStrCtx->sStrDecdPictList))
        {
            psDecodedPict = DQ_next(psDecodedPict);
        }
        else
        {
            psDecodedPict = IMG_NULL;
        }
    }
    IMG_ASSERT(ui32Item == psDecStrCtx->sDecStrStatus.ui32NumPictDecoded);
    {
        VXDIO_sState * psCoreState = &sCoreState.sCoreState;

        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_PC                 = psCoreState->sEPRuntimeStatus.ui32MTX_PC;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_PCX                = psCoreState->sEPRuntimeStatus.ui32MTX_PCX;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_ENABLE             = psCoreState->sEPRuntimeStatus.ui32MTX_ENABLE;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_STATUS_BITS        = psCoreState->sEPRuntimeStatus.ui32MTX_STATUS_BITS;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_FAULT0             = psCoreState->sEPRuntimeStatus.ui32MTX_FAULT0;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_A0StP              = psCoreState->sEPRuntimeStatus.ui32MTX_A0StP;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32MTX_A0FrP              = psCoreState->sEPRuntimeStatus.ui32MTX_A0FrP;
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_COUNT[0]           = psCoreState->sEPRuntimeStatus.ui32DMA_COUNT[0];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_COUNT[1]           = psCoreState->sEPRuntimeStatus.ui32DMA_COUNT[1];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_COUNT[2]           = psCoreState->sEPRuntimeStatus.ui32DMA_COUNT[2];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_SETUP[0]           = psCoreState->sEPRuntimeStatus.ui32DMA_SETUP[0];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_SETUP[1]           = psCoreState->sEPRuntimeStatus.ui32DMA_SETUP[1];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_SETUP[2]           = psCoreState->sEPRuntimeStatus.ui32DMA_SETUP[2];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_PERIPHERAL_ADDR[0] = psCoreState->sEPRuntimeStatus.ui32DMA_PERIPHERAL_ADDR[0];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_PERIPHERAL_ADDR[1] = psCoreState->sEPRuntimeStatus.ui32DMA_PERIPHERAL_ADDR[1];
        psDecStrCtx->sDecStrStatus.sCoreStatus.ui32DMA_PERIPHERAL_ADDR[2] = psCoreState->sEPRuntimeStatus.ui32DMA_PERIPHERAL_ADDR[2];
    }

    // Get list of decoding pictures
    ui32Item = 0;
    psDecStrUnit = LST_first(&psDecStrCtx->sPendStrUnitList);
    while (psDecStrUnit)
    {
        if (psDecStrUnit->psStrUnit->eStrUnitType == VDECDD_STRUNIT_PICTURE_START)
        {
            HWCTRL_sState * psCoreState = &sCoreState;
            VDECDD_sDecPictStatus * psDecPictStat = &psDecStrCtx->sDecStrStatus.asDecPictStatus[ui32Item];

            IMG_ASSERT(psDecPictStat);

            psDecPictStat->ui32TransactionId = psDecStrUnit->psDecPict->ui32TransactionId;

            psDecPictStat->ui32DMACStatus = -1;

            IMG_MEMCPY(psDecPictStat->aui8FWControlMsg,
                       psCoreState->sFwMsgStatus.aui8ControlFenceID,
                       sizeof(psDecPictStat->aui8FWControlMsg));
            IMG_MEMCPY(psDecPictStat->aui8FWDecodeMsg,
                       psCoreState->sFwMsgStatus.aui8DecodeFenceID,
                       sizeof(psDecPictStat->aui8FWDecodeMsg));
            IMG_MEMCPY(psDecPictStat->aui8FWCompletionMsg,
                       psCoreState->sFwMsgStatus.aui8CompletionFenceID,
                       sizeof(psDecPictStat->aui8FWCompletionMsg));

            IMG_MEMCPY(psDecPictStat->aui8HostControlMsg,
                       psCoreState->sHostMsgStatus.aui8ControlFenceID,
                       sizeof(psDecPictStat->aui8HostControlMsg));
            IMG_MEMCPY(psDecPictStat->aui8HostDecodeMsg,
                       psCoreState->sHostMsgStatus.aui8DecodeFenceID,
                       sizeof(psDecPictStat->aui8HostDecodeMsg));
            IMG_MEMCPY(psDecPictStat->aui8HostCompletionMsg,
                       psCoreState->sHostMsgStatus.aui8CompletionFenceID,
                       sizeof(psDecPictStat->aui8HostCompletionMsg));

            // Find the last checkpoint of each group for current decoding picture.
            psDecPictStat->eFwCheckPoint = -1;
            for (i = VDECFW_CHECKPOINT_FW_END; i >= VDECFW_CHECKPOINT_FW_START; i--)
            {
                // If the transaction has already been processed by this checkpoint mark this as the most
                // advanced point the transaction has reached and stop tracing back through the system.
                // XXX: TODO[ML]: This has to be updated for multi-pipe cases.
                if (psCoreState->sCoreState.sFwState.asPipeState[0].aui32CheckPoint[i] >= psDecStrUnit->psDecPict->ui32TransactionId)
                {
                    psDecPictStat->eFwCheckPoint = i;
                    psDecPictStat->ui32DMACStatus = psCoreState->sCoreState.sFwState.asPipeState[0].aui32DMACStatus[0];
                    break;
                }
            }
            psDecPictStat->eFeHwCheckPoint = -1;
            psDecPictStat->ui32FeMbX = -1;
            psDecPictStat->ui32FeMbY = -1;
            for (i = VDECFW_CHECKPOINT_FE_END; i >= VDECFW_CHECKPOINT_FE_START; i--)
            {
                // If the transaction has already been processed by this checkpoint mark this as the most
                // advanced point the transaction has reached and stop tracing back through the system.
                if (psCoreState->sCoreState.sFwState.asPipeState[0].aui32CheckPoint[i] >= psDecStrUnit->psDecPict->ui32TransactionId)
                {
                    psDecPictStat->eFeHwCheckPoint = i;
                    //if (psDecPictStat->eFeHwCheckPoint != VDECFW_CHECKPOINT_FE_END)
                    {
                        // Only store the last processed MB while the picture has not been completed.
                        psDecPictStat->ui32FeMbX = psCoreState->sCoreState.sFwState.asPipeState[0].sFeMb.ui32X;
                        psDecPictStat->ui32FeMbY = psCoreState->sCoreState.sFwState.asPipeState[0].sFeMb.ui32Y;
                    }
                    break;
                }
            }
            psDecPictStat->eBeHwCheckPoint = -1;
            psDecPictStat->ui32BeMbX = -1;
            psDecPictStat->ui32BeMbY = -1;
            for (i = VDECFW_CHECKPOINT_BE_END; i >= VDECFW_CHECKPOINT_BE_START; i--)
            {
                // If the transaction has already been processed by this checkpoint mark this as the most
                // advanced point the transaction has reached and stop tracing back through the system.
                if (psCoreState->sCoreState.sFwState.asPipeState[0].aui32CheckPoint[i] >= psDecStrUnit->psDecPict->ui32TransactionId)
                {
                    psDecPictStat->eBeHwCheckPoint = i;
                    //if (psDecPictStat->eBeHwCheckPoint != VDECFW_CHECKPOINT_BE_END)
                    {
                        // Only store the last processed MB while the picture has not been completed.
                        psDecPictStat->ui32BeMbX = psCoreState->sCoreState.sFwState.asPipeState[0].sBeMb.ui32X;
                        psDecPictStat->ui32BeMbY = psCoreState->sCoreState.sFwState.asPipeState[0].sBeMb.ui32Y;
                    }
                    break;
                }
            }

            ui32Item++;
        }
        psDecStrUnit = LST_next(psDecStrUnit);
    }
    IMG_ASSERT(ui32Item == psDecStrCtx->sDecStrStatus.ui32NumPictDecoding);

    *psDecStrStatus = psDecStrCtx->sDecStrStatus;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_StreamProcessUnit

******************************************************************************/
static IMG_RESULT
decoder_StreamProcessUnit(
    DECODER_sStrContext * psDecStrCtx,
    VDECDD_sStrUnit * psStrUnit,
    IMG_BOOL * pbDone
)
{
    DECODER_sDecStrUnit * psDecStrUnit;
    DECODER_sDecPict * psDecPict = IMG_NULL;
    IMG_BOOL bProcessed = IMG_FALSE;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psDecStrCtx);
    IMG_ASSERT(psStrUnit);

    switch (psStrUnit->eStrUnitType)
    {
    case VDECDD_STRUNIT_SEQUENCE_END:
    case VDECDD_STRUNIT_ANONYMOUS:
    case VDECDD_STRUNIT_CLOSED_GOP:
    case VDECDD_STRUNIT_PICTURE_PORTENT:
    case VDECDD_STRUNIT_STOP:
    case VDECDD_STRUNIT_FENCE:
        // Nothing more to do so mark the stream unit as processed.
        bProcessed = IMG_TRUE;
        break;

    case VDECDD_STRUNIT_SEQUENCE_START:
        {
            IMG_UINT32 ui32MaxNumActivePict;

            IMG_ASSERT(psStrUnit->psSequHdrInfo);
            // Determine how many decoded pictures can be held for reference in
            // the decoder for this stream.
            ui32Result = VDECDDUTILS_RefPictGetMaxNum(&psDecStrCtx->sConfig,
                                                      &psStrUnit->psSequHdrInfo->sComSequHdrInfo,
                                                      &ui32MaxNumActivePict);
            if (IMG_SUCCESS != ui32Result)
            {
                return ui32Result;
            }

            // Double for field coding.
            ui32MaxNumActivePict *= 2;

            // Ensure that there are enough resource to have pictures filling all slots on all cores.
            // XXX: this is not very efficient but it's simple!
            ui32MaxNumActivePict += psDecStrCtx->psDecCtx->psDevConfig->ui32NumSlotsPerPipe * psDecStrCtx->psDecCtx->ui32NumPipes;

            // Increase decoder stream resources if necessary.
            while (psDecStrCtx->ui32NumRefRes < ui32MaxNumActivePict)
            {
                ui32Result = decoder_StreamReferenceResourceCreate(psDecStrCtx, IMG_NULL);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }
                psDecStrCtx->ui32NumRefRes++;
            }
        }

        // Nothing more to do so mark the stream unit as processed.
        bProcessed = IMG_TRUE;
        break;

    case VDECDD_STRUNIT_PICTURE_START:
        if (psStrUnit->bDecode)
        {
            ui32Result = decoder_GetPipeAndLoad(psDecStrCtx,
                                                psStrUnit->ui32Features,
                                                IMG_NULL,
                                                IMG_NULL,
                                                &psDecStrCtx->ui8Pipe);

            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            // Prepare and submit picture to decode.
            ui32Result = decoder_PictureDecode(
                            psDecStrCtx,
                            psStrUnit,
                            &psDecPict);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
            DEBUG_REPORT(REPORT_MODULE_DECODER,
                "[USERSID=0x%08X] [TID=0x%08X] START pipe: %x",
                psDecStrCtx->sConfig.ui32UserStrId,
                psDecStrCtx->ui32LastFeTransactionId, psDecStrCtx->ui8Pipe);
        }
        else
        {
            bProcessed = IMG_TRUE;
#ifdef VDEC_USE_PVDEC_SEC
            if (psDecStrCtx->sConfig.bSecureStream)
            {
                DEBUG_REPORT(REPORT_MODULE_DECODER,
                   "[USERSID=0x%08X] [PID=0x%08X] DROP",
                    psDecStrCtx->sConfig.ui32UserStrId,
                    psStrUnit->ui32ParseId);

                ui32Result = FWBSP_BRIDGE_PictureReturn(psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                                                        psStrUnit->ui32ParseId,
                                                        IMG_FALSE);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }

                ui32Result = FWBSP_BRIDGE_PictureRemove(psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                                                        psStrUnit->ui32ParseId,
                                                        IMG_FALSE);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }
            }
#endif /* VDEC_USE_PVDEC_SEC */
        }
        break;

    case VDECDD_STRUNIT_PICTURE_END:
        if (psStrUnit->bDecode)
        {
            ui32Result = decoder_PictureFinalize(psDecStrCtx, psStrUnit);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
            DEBUG_REPORT(REPORT_MODULE_DECODER,
                "[USERSID=0x%08X] [TID=0x%08X] END",
                psDecStrCtx->sConfig.ui32UserStrId,
                psDecStrCtx->ui32LastFeTransactionId);
        }
        else
        {
            bProcessed = IMG_TRUE;
        }
        break;

    default:
        IMG_ASSERT(IMG_FALSE);
        break;
    }

    // If this or any preceding stream unit(s) could not be
    // completely processed, add this unit to the queue.
    if (!LST_empty(&psDecStrCtx->sPendStrUnitList) || !bProcessed)
    {
        // Add unit to stream decode list.
        VDEC_MALLOC(psDecStrUnit);
        IMG_ASSERT(psDecStrUnit);
        if (psDecStrUnit == IMG_NULL)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                "[USERSID=0x%08X] Failed to allocate memory for Decoder stream unit wrapper",
                psDecStrCtx->sConfig.ui32UserStrId);
            return IMG_ERROR_OUT_OF_MEMORY;
        }

        VDEC_BZERO(psDecStrUnit);
        psDecStrUnit->psStrUnit = psStrUnit;

        // make PICTURE_START owner of psDecPict
        if (psDecPict != IMG_NULL)
        {
            IMG_ASSERT(psStrUnit->eStrUnitType == VDECDD_STRUNIT_PICTURE_START);
            psDecStrUnit->psDecPict = psDecPict;
        }

        LST_add(&psDecStrCtx->sPendStrUnitList, psDecStrUnit);

        decoder_KickListService(psDecStrCtx, psStrUnit);
    }
    else
    {
        *pbDone = IMG_TRUE;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_StreamProcessUnit

******************************************************************************/
IMG_RESULT
DECODER_StreamProcessUnit(
    IMG_HANDLE hDecStrCtx,
    VDECDD_sStrUnit * psStrUnit
)
{
    DECODER_sStrContext * psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);
    IMG_BOOL bProcessed = IMG_FALSE;
    IMG_RESULT ui32Result;

    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }
    // Process the stream unit
    ui32Result = decoder_StreamProcessUnit(psDecStrCtx, psStrUnit, &bProcessed);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to process stream unit of type [UTYPE=0x%08X]",
            psDecStrCtx->sConfig.ui32UserStrId, psStrUnit->eStrUnitType);
        return ui32Result;
    }

    if (bProcessed)
    {
        // If there is nothing being decoded for this stream immediately
        // handle the unit (non-picture so doesn't need decoding).
        // Report that this unit has been processed.
        IMG_ASSERT(psDecStrCtx->psDecCtx);
        ui32Result = psDecStrCtx->psDecCtx->pfnCompCallback(
            DECODER_CB_UNIT_PROCESSED,
            psDecStrCtx->psDecCtx->pvCompInitUserData,
            IMG_NULL,
            psDecStrCtx->pvUserData,
            psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_StreamFlushInput

******************************************************************************/
IMG_RESULT
DECODER_StreamFlushInput(
    IMG_HANDLE      hDecServiceInt,
    IMG_VOID     ** ppvStrUserIntData
)
{
    IMG_RESULT              ui32Result;
    DECODER_sStrContext   * psDecStrCtx;
    DECODER_sDecStrUnit   * psDecStrUnit;
    IMG_BOOL                bPictStart = IMG_FALSE;

    psDecStrCtx = (DECODER_sStrContext*)hDecServiceInt;
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    *ppvStrUserIntData = psDecStrCtx->pvUserIntData;

    // Peek the first stream unit
    psDecStrUnit = LST_first(&psDecStrCtx->sPendStrUnitList);
    if ((psDecStrUnit==IMG_NULL) ||
        (psDecStrUnit->psStrUnit->eStrUnitType==VDECDD_STRUNIT_PICTURE_START))
    {
        bPictStart = IMG_TRUE;
    }

    // Process all units from the pending stream list until the next picture start.
    while (!bPictStart)
    {
        // Actually remove the unit now from the pending stream list.
        LST_remove(&psDecStrCtx->sPendStrUnitList, psDecStrUnit);

        DEBUG_REPORT(REPORT_MODULE_DECODER, "[Stream Unit: %d] LATE SERVICE", psDecStrUnit->psStrUnit->eStrUnitType);

        IMG_ASSERT(psDecStrCtx->psDecCtx);
        // Report that this unit has been processed.
        ui32Result = psDecStrCtx->psDecCtx->pfnCompCallback(
            DECODER_CB_UNIT_PROCESSED,
            psDecStrCtx->psDecCtx->pvCompInitUserData,
            IMG_NULL,
            psDecStrCtx->pvUserData,
            psDecStrUnit->psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        // Destroy the Decoder stream unit wrapper.
        IMG_FREE(psDecStrUnit);
        psDecStrUnit = IMG_NULL;

        // Peek at the next stream unit.
        psDecStrUnit = LST_first(&psDecStrCtx->sPendStrUnitList);
        if ((psDecStrUnit==IMG_NULL) ||
            (psDecStrUnit->psStrUnit->eStrUnitType==VDECDD_STRUNIT_PICTURE_START))
        {
            bPictStart = IMG_TRUE;
        }
    }

    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              DECODER_StreamIsIdle

******************************************************************************/
IMG_BOOL
DECODER_StreamIsIdle(
    IMG_HANDLE hDecStrCtx
)
{
    DECODER_sStrContext * psDecStrCtx;

    psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_FALSE;
    }


    return LST_empty(&psDecStrCtx->sPendStrUnitList);
}


/*!
******************************************************************************

 @Function              DECODER_StreamDestroy

******************************************************************************/
IMG_RESULT
DECODER_StreamDestroy(
    IMG_HANDLE  hDecStrCtx,
    IMG_BOOL    bAbort
)
{
    DECODER_sStrContext *   psDecStrCtx;
    DECODER_sDecStrUnit *   psDecStrUnit;
    DECODER_sDecodedPict *  psDecodedPict;
    IMG_UINT32              i;
    IMG_RESULT              ui32Result;

    // Required for getting segments from decode picture to free
    DECODER_sDecPictSeg * psDecPictSeg;

    // Get the Decoder stream context.
    psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);

    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid decoder stream context handle!");
        return IMG_FALSE;
    }
    IMG_ASSERT(psDecStrCtx->psDecCtx);

    // Decrement the stream count
    psDecStrCtx->psDecCtx->ui32StrCnt--;

    // Ensure that there are no pictures for this stream outstanding
    // on any decoder cores.
    // This should not be removed, it is important to see it if it ever happens.
    // In practice we see it many times with Application Timeout.
    if(!bAbort)
    {
        IMG_ASSERT(LST_empty(&psDecStrCtx->sPendStrUnitList));
    }

    // At this point all resources for the stream are guaranteed to not be used
    // and no further hardware interrupts will be received.

    // Destroy all stream units submitted for processing.
    psDecStrUnit = LST_removeHead(&psDecStrCtx->sPendStrUnitList);
    while (psDecStrUnit)
    {
        // If the unit was submitted for decoding (picture)...
        if (psDecStrUnit->psDecPict)
        {
            // Explicitly remove picture from core decode queue and destroy.
            DECODER_sCoreContext * psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);
            IMG_ASSERT(psDecCoreCtx);

            ui32Result = HWCTRL_RemoveFromPicList(psDecCoreCtx->hHwCtx,
                                               psDecStrUnit->psDecPict);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
            // Free decoder picture
            IMG_FREE(psDecStrUnit->psDecPict->psFirstFieldFwMsg);
            psDecStrUnit->psDecPict->psFirstFieldFwMsg = IMG_NULL;
            IMG_FREE(psDecStrUnit->psDecPict->psSecondFieldFwMsg);
            psDecStrUnit->psDecPict->psSecondFieldFwMsg = IMG_NULL;
            RESOURCE_PictureDetach(&psDecCoreCtx->hResources,
                                   psDecStrUnit->psDecPict,
                                   psDecStrCtx->sConfig.bSecureStream);

            // Free all the segments of the picture
            psDecPictSeg = LST_removeHead(&psDecStrUnit->psDecPict->sDecPictSegList);
            while(psDecPictSeg)
            {
                if (psDecPictSeg->bInternalSeg)
                {
                    IMG_ASSERT(psDecPictSeg->psBitStrSeg);
                    IMG_FREE(psDecPictSeg->psBitStrSeg);
                    psDecPictSeg->psBitStrSeg = IMG_NULL;
                }

                IMG_FREE(psDecPictSeg);
                psDecPictSeg = IMG_NULL;

                psDecPictSeg = LST_removeHead(&psDecStrUnit->psDecPict->sDecPictSegList);
            }

            IMG_ASSERT(psDecStrUnit->psDecPict->psDecStrCtx == psDecStrCtx);

            psDecStrCtx->sDecStrStatus.ui32NumPictDecoding--;

            ui32Result = decoder_PictureDestroy(psDecStrCtx,
                                      GET_STREAM_PICTURE_ID(psDecStrUnit->psDecPict->ui32TransactionId),
                                                IMG_TRUE);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            IMG_FREE(psDecStrUnit->psDecPict);
            psDecStrUnit->psDecPict = IMG_NULL;
        }

        // XXX: perhaps replace with UNIT_ABORTED event?
        // Report that the picture has been decoded and
        // have all non-reference elements released.
        IMG_ASSERT(psDecStrCtx->psDecCtx);
        ui32Result = psDecStrCtx->psDecCtx->pfnCompCallback(
            DECODER_CB_UNIT_PROCESSED,
            psDecStrCtx->psDecCtx->pvCompInitUserData,
            IMG_NULL,
            psDecStrCtx->pvUserData,
            psDecStrUnit->psStrUnit);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        // Free the picture container
        IMG_FREE(psDecStrUnit);
        psDecStrUnit = IMG_NULL;

        psDecStrUnit = LST_removeHead(&psDecStrCtx->sPendStrUnitList);
    }

    // Destroy all pictures in the decoded list
    psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    while (psDecodedPict)
    {
        ui32Result = decoder_DecodedPictureDestroy(psDecStrCtx, psDecodedPict, IMG_TRUE);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        psDecodedPict = DQ_first(&psDecStrCtx->sStrDecdPictList);
    }

    // Ensure all picture queues are empty.
    IMG_ASSERT(LST_empty(&psDecStrCtx->sPendStrUnitList));
    IMG_ASSERT(DQ_empty(&psDecStrCtx->sStrDecdPictList));

#ifdef VDEC_USE_PVDEC
    // Free memory to store stream context buffer.
    ui32Result = MMU_FreeMem(&psDecStrCtx->sPvdecFwCtxBuf);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }
#endif

    // Release any fw contexts held by stream.
    if (psDecStrCtx->psPrevFePictDecRes)
    {
        RESOURCE_ItemReturn(&psDecStrCtx->psPrevFePictDecRes->ui32RefCount);
    }
    if (psDecStrCtx->psCurFePictDecRes)
    {
        RESOURCE_ItemReturn(&psDecStrCtx->psCurFePictDecRes->ui32RefCount);
    }
    if (psDecStrCtx->psLastBePictDecRes)
    {
        RESOURCE_ItemReturn(&psDecStrCtx->psLastBePictDecRes->ui32RefCount);
    }

    // Remove the device resources used for decoding and the two
    // added to hold the last on front and back-end for stream.
    for (i = 0; i < psDecStrCtx->ui32NumDecRes + 2; i++)
    {
        ui32Result = RESOURCE_ListEmpty(&psDecStrCtx->sDecResList,
                                        IMG_FALSE,
                                        decoder_StreamDecodeResourceDestroy,
                                        IMG_NULL);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
    IMG_ASSERT(LST_empty(&psDecStrCtx->sDecResList));


    // Remove all stream decode resources.
    ui32Result = RESOURCE_ListEmpty(&psDecStrCtx->sRefResList,
                                     IMG_FALSE,
                                     decoder_StreamReferenceResourceDestroy,
                                     IMG_NULL);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }
    IMG_ASSERT(LST_empty(&psDecStrCtx->sRefResList));

    IDGEN_DestroyContext(psDecStrCtx->hPictIdGen);

    ui32Result = IDGEN_FreeId(psDecStrCtx->psDecCtx->hStrIdGen,psDecStrCtx->ui32StrId);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

#ifdef VDEC_USE_PVDEC_SEC
    {
        DECODER_sContext * psDecCtx = psDecStrCtx->psDecCtx;
        DECODER_sCoreContext * psDecCoreCtx = DECODER_STR_CTX_TO_CORE_CTX(psDecStrCtx);

        IMG_ASSERT(psDecCtx);
        IMG_ASSERT(psDecCoreCtx);

        /* Destroy firmware. */
        ui32Result = decoder_DestroyFirmware(&psDecCtx->sFirmware,
                                             psDecCtx->hDevHandle);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                "decoder_DestroyFirmware() failed to tidy-up after error");
        }

        /* Check if we have no any streams active */
        if (!psDecStrCtx->psDecCtx->ui32StrCnt &&
            psDecCoreCtx->hResources)
        {
            ui32Result = RESOURCES_Destroy(psDecCoreCtx->hResources);
            if (ui32Result != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "RESOURCES_Destroy() failed to tidy-up after error");
            }
            psDecCoreCtx->hResources = IMG_NULL;

            psDecCtx->eDecFwConf = eDCONF_FwNone;
        }
    }
#endif /* VDEC_USE_PVDEC_SEC */

    LST_remove(&psDecStrCtx->psDecCtx->sStreamList, psDecStrCtx);

    IMG_FREE(psDecStrCtx);
    psDecStrCtx = IMG_NULL;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              decoder_GetRequiredCoreFeatures

 @Description

 This function obtains the core features.

 @Input     psStreamConfig  : Pointer to stream configuration.

 @Input     psOutputConfig  : Pointer to output configuration.

 @Output    peFeatures  : Pointer to core features.

 @Return    IMG_RESULT  : This function returns either IMG_SUCCESS or an
                          error code.

******************************************************************************/
static IMG_RESULT
decoder_GetRequiredCoreFeatures(
    const VDEC_sStrConfigData   * psStreamConfig,
    const VDEC_sStrOutputConfig * psOutputConfig,
    IMG_UINT32                  * pui32Features
)
{
    IMG_UINT32 ui32Features = 0;

    IMG_ASSERT(psStreamConfig);
    IMG_ASSERT(pui32Features);

    /* Check Video Standard. */
    switch (psStreamConfig->eVidStd)
    {
    case VDEC_STD_MPEG2:
        ui32Features = VDECDD_COREFEATURE_MPEG2;
        break;

    case VDEC_STD_MPEG4:
    case VDEC_STD_H263:
    case VDEC_STD_SORENSON:
        ui32Features = VDECDD_COREFEATURE_MPEG4;
        break;

    case VDEC_STD_H264:
        ui32Features = VDECDD_COREFEATURE_H264;
        break;

    case VDEC_STD_VC1:
        ui32Features = VDECDD_COREFEATURE_VC1;
        break;

    case VDEC_STD_AVS:
        ui32Features = VDECDD_COREFEATURE_AVS;
        break;

    case VDEC_STD_REAL:
        ui32Features = VDECDD_COREFEATURE_REAL;
        break;

    case VDEC_STD_JPEG:
        ui32Features = VDECDD_COREFEATURE_JPEG;
        break;

    case VDEC_STD_VP6:
        ui32Features = VDECDD_COREFEATURE_VP6;
        break;

    case VDEC_STD_VP8:
        ui32Features = VDECDD_COREFEATURE_VP8;
        break;

    case VDEC_STD_HEVC:
        ui32Features = VDECDD_COREFEATURE_HEVC;
        break;

    default:
        IMG_ASSERT(IMG_FALSE);
        break;
    }

    if (psOutputConfig)
    {
        if (psOutputConfig->eRotMode != VDEC_ROTATE_0)
        {
            ui32Features |= VDECDD_COREFEATURE_ROTATION;
        }

        if (psOutputConfig->bScale)
        {
            ui32Features |= VDECDD_COREFEATURE_SCALING;
        }
    }

    *pui32Features = ui32Features;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              decoder_IsSupportedByAtLeastOnePipe

******************************************************************************/
static IMG_BOOL decoder_IsSupportedByAtLeastOnePipe(IMG_BOOL *aFeatures, IMG_UINT32 ui32NumPipes)
{
    IMG_UINT32 i;
    IMG_ASSERT(aFeatures);
    IMG_ASSERT(ui32NumPipes <= VDEC_MAX_PIXEL_PIPES);

    for(i = 0; i < ui32NumPipes;i++)
    {
        if(aFeatures[i])
        {
            return IMG_TRUE;
        }
    }

    return IMG_FALSE;
}

/*!
******************************************************************************

 @Function              DECODER_CheckSupport

******************************************************************************/
IMG_RESULT
DECODER_CheckSupport(
    IMG_HANDLE                      hDecCtx,
    const VDEC_sStrConfigData     * psStrConfig,
    const VDEC_sStrOutputConfig   * psOutputConfig,
    const VDECDD_sDdPictBuf       * psDispPictBuf,
    const VDEC_sPictRendInfo      * psReqPictRendInfo,
    const VDEC_sComSequHdrInfo    * psComSequHdrInfo,
    const BSPP_sPictHdrInfo       * psPictHdrInfo,
    const VDEC_sComSequHdrInfo    * psPrevComSequHdrInfo,
    const BSPP_sPictHdrInfo       * psPrevPictHdrInfo,
    VDEC_sUnSupportedFlags        * psUnsupported,
    IMG_UINT32                    * pui32Features
)
{
    DECODER_sContext *          psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext      * psDecCoreCtx;
    VXD_sCoreProps            * psCoreProps;
    const VDEC_sPictRendInfo  * psDispPictRendInfo = IMG_NULL;
    IMG_RESULT                  ui32Result = IMG_ERROR_NOT_SUPPORTED;

    // Ensure input parameters are valid.
    IMG_ASSERT(hDecCtx);
    IMG_ASSERT(psStrConfig);
    IMG_ASSERT(psUnsupported);

    if(hDecCtx == IMG_NULL || 
        psStrConfig == IMG_NULL ||
        psUnsupported == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    if (psDispPictBuf)
    {
        psDispPictRendInfo = &psDispPictBuf->sRendInfo;
    }

    // Validate compatibility between the supplied configuration/state
    // and the master core only at the moment (assumed to have superset of features).
    // Some features may not be present on any slave cores which might
    // cause poor utilisation of hardware.

        VDEC_BZERO(psUnsupported);

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);

        psCoreProps = &psDecCoreCtx->sCoreProps;
        IMG_ASSERT(psCoreProps);

        if (psStrConfig)
        {
#ifdef VDEC_USE_PVDEC_SEC
        if (psStrConfig->bSecureStream &&
            psStrConfig->eVidStd != VDEC_STD_H264 &&
            psStrConfig->eVidStd != VDEC_STD_HEVC)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                "[USERSID=0x%08X] UNSUPPORTED[SW]: Only H264 and HEVC"
                " secure decode are currently supported!",
                psStrConfig->ui32UserStrId);
            psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
        }
#endif /* VDEC_USE_PVDEC_SEC */

            // Check that the video standard is supported.
            switch (psStrConfig->eVidStd)
            {
            case VDEC_STD_AVS:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abAvs, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (AVS)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_H263:
                /* h263 is included in mpeg4 */
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abMpeg4, psCoreProps->ui32NumPixelPipes) )
                {
                    IMG_WARN_ONCE("UNSUPPORTED: VIDEO STANDARD [H263]\n");
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_H264:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abH264, psCoreProps->ui32NumPixelPipes))
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (H.264)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                if (psComSequHdrInfo &&
                    (H264_PROFILE_MVC_HIGH == psComSequHdrInfo->ui32CodecProfile ||
                     H264_PROFILE_MVC_STEREO == psComSequHdrInfo->ui32CodecProfile) &&
                     psComSequHdrInfo->ui32NumViews > VDEC_H264_MVC_MAX_VIEWS )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[SW]: NUMBER OF VIEWS",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_NUM_OF_VIEWS;
                }
#ifndef ENABLE_SEPARATE_COLOUR_PLANES
                if(psComSequHdrInfo && psComSequHdrInfo->bSeparateChromaPlanes)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[SW]: SEPARATE COLOUR PLANE SUPPORT NOT ENABLED",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_SEP_COLOUR_PLANE;
                }
#endif
                break;

            case VDEC_STD_JPEG:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abJpeg, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (JPEG)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_MPEG2:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abMpeg2, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (MPEG2)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_MPEG4:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abMpeg4, psCoreProps->ui32NumPixelPipes))
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (MPEG4)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_REAL:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abReal, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (REAL)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_VC1:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abVc1, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (VC-1)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_VP6:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abVp6, psCoreProps->ui32NumPixelPipes))
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (VP6)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_VP8:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abVp8, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (VP8)",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_SORENSON:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abSorenson, psCoreProps->ui32NumPixelPipes))
                {
                    IMG_WARN_ONCE("UNSUPPORTED: VIDEO STANDARD [SORENSON]\n");
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                break;

            case VDEC_STD_HEVC:
                if (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abHEVC, psCoreProps->ui32NumPixelPipes) )
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                           "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (HEVC)",
                           psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                }
                if (psPictHdrInfo && psPictHdrInfo->sHEVCPictHdrInfo.bRangeExtPresent)
                {
                    if ((psPictHdrInfo->sHEVCPictHdrInfo.bIsFullRangeExt && // full support
                        !decoder_IsSupportedByAtLeastOnePipe(psCoreProps->bHevcRangeExt, psCoreProps->ui32NumPixelPipes)) ||
                        (!psPictHdrInfo->sHEVCPictHdrInfo.bIsFullRangeExt && // partial support
                        psCoreProps->asVidStdProps[psStrConfig->eVidStd].eMaxChromaFormat == PIXEL_FORMAT_420))
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                               "[USERSID=0x%08X] UNSUPPORTED[HW]: HEVC RANGE EXTENSIONS",
                               psStrConfig->ui32UserStrId);
                        psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_HEVC_RANGE_EXT;
                    }
                }
                break;

            default:
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: VIDEO STANDARD (UNKNOWN)",
                        psStrConfig->ui32UserStrId);
                psUnsupported->ui32StrConfig |= VDECDD_UNSUPPORTED_STRCONFIG_STD;
                break;
            }
        }

        if (psOutputConfig)
        {
            // Ensure that each display feature is supported by the hardware.
            if (psOutputConfig->eRotMode != VDEC_ROTATE_0 &&
                (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abRotationSupport, psCoreProps->ui32NumPixelPipes) ||
                 psStrConfig->bSecureStream))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION",
                    psStrConfig->ui32UserStrId);
                psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_ROTATION;
            }

            if (psOutputConfig->bScale &&
                (!decoder_IsSupportedByAtLeastOnePipe(psCoreProps->abScalingSupport, psCoreProps->ui32NumPixelPipes) ||
                 psStrConfig->bSecureStream))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: SCALING",
                    psStrConfig->ui32UserStrId);
                psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_SCALING;
            }
            if (psComSequHdrInfo)
            {

                /*if ((psComSequHdrInfo->sPixelInfo.eChromaFormatIdc != psOutputConfig->sPixelInfo.eChromaFormatIdc) &&
                    (psComSequHdrInfo->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_420) &&
                    (psOutputConfig->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_422))

                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[SW]: UP/DOWN-SAMPLING",
                        psStrConfig->ui32UserStrId);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_UP_DOWNSAMPLING;
                }*/
                //Validate display pixel format
                switch(psOutputConfig->sPixelInfo.eChromaFormatIdc)
                {
                case PIXEL_FORMAT_MONO:

                    if(psComSequHdrInfo->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_MONO)
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: INVALID CODED-DISPLAY PIXEL FORMAT COMBINATION",
                            psStrConfig->ui32UserStrId);
                            psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
                    }
                    break;

                case PIXEL_FORMAT_420:

                    if(psComSequHdrInfo->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_MONO)
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: TRANSFORM PIXEL FORMAT FROM 400 TO 420",
                            psStrConfig->ui32UserStrId);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
                    }
                    break;

                case PIXEL_FORMAT_422:

                    if(psComSequHdrInfo->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_420)
                    {
                        if(!psCoreProps->bChromaUpsample)
                        {
                            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                                "[USERSID=0x%08X] UNSUPPORTED[HW]: CHROMA UPSAMPLING",
                                psStrConfig->ui32UserStrId);
                            psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
                        }
                        else if(psOutputConfig->sPixelInfo.ui32NoPlanes > 1)
                        {
                            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                                "[USERSID=0x%08X] UNSUPPORTED[HW]: REQUESTED NUMBER OF PLANES FOR 422 UPSAMPLING",
                                psStrConfig->ui32UserStrId);
                            psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
                        }
                    }
                    else if(psComSequHdrInfo->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_MONO)
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: TRANSFORM PIXEL FORMAT FROM 400 TO 420",
                            psStrConfig->ui32UserStrId);
                    }
                    break;

                case PIXEL_FORMAT_444:

                    if(psComSequHdrInfo->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_444)
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: CHROMA UPSAMPLING TO 444",
                            psStrConfig->ui32UserStrId);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
                    }

#if 0
                    else if((psOutputConfig->sPixelInfo.ui32NoPlanes != 0) ||
                            (psOutputConfig->sPixelInfo.ui32NoPlanes != 2) ||
                            (psOutputConfig->sPixelInfo.ui32NoPlanes != 3))
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: INVALID NUMBER OF PLANES[%d] FOR 444",
                            psStrConfig->ui32UserStrId,
                            psOutputConfig->sPixelInfo.ui32NoPlanes);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
                    }
#endif
                    break;
                default:
                    break;
                }

                if (psOutputConfig->bScale)
                {
                    if (psComSequHdrInfo->sOrigDisplayRegion.ui32TopOffset ||
                        psComSequHdrInfo->sOrigDisplayRegion.ui32LeftOffset)
                    {
                        // XXX-matt: top crop may be possible without corruption?
                        IMG_WARN_ONCE("Cannot scale correctly with top or left crop");
                    }
                }
            }

            // Validate combintation of display features.
            if (psOutputConfig->eRotMode != VDEC_ROTATE_0 && psOutputConfig->bScale)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION & SCALING",
                    psStrConfig->ui32UserStrId);
                psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_ROTATION_WITH_SCALING;
            }

            if(psComSequHdrInfo)
			{
				if((psOutputConfig->eRotMode != VDEC_ROTATE_0) &&
					( (psOutputConfig->sPixelInfo.eChromaFormatIdc != psComSequHdrInfo->sPixelInfo.eChromaFormatIdc) ||
					(psOutputConfig->sPixelInfo.ui32BitDepthY != psComSequHdrInfo->sPixelInfo.ui32BitDepthY) ||
					(psOutputConfig->sPixelInfo.ui32BitDepthC != psComSequHdrInfo->sPixelInfo.ui32BitDepthC)))
				{
					REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
						"[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION AND DOWNSAMPLING",
						psStrConfig->ui32UserStrId);
					psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_DOWNSAMPLING_WITH_ROTATION;
				}

				if((psOutputConfig->eRotMode != VDEC_ROTATE_0) &&
					(psOutputConfig->sPixelInfo.eMemoryPacking != psComSequHdrInfo->sPixelInfo.eMemoryPacking))
				{
					REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
						"[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION WITH 10 BIT PACKED FORMAT",
						psStrConfig->ui32UserStrId);
					psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_ROTATION_WITH_10BIT_PACKED;
				}

                if (psOutputConfig->eRotMode != VDEC_ROTATE_0)
                {
                IMG_BOOL b444RotationSupport = (psStrConfig->eVidStd == VDEC_STD_HEVC) ||
                                                (psStrConfig->eVidStd == VDEC_STD_H264 && psCoreProps->b444H264Rotation);

                    if( ( (psOutputConfig->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_422) &&
                          (!((psComSequHdrInfo->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_422) && (psOutputConfig->eRotMode == VDEC_ROTATE_180))) &&
                          (!((psComSequHdrInfo->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_444) && (psOutputConfig->sPixelInfo.ui32NoPlanes == 3))) ) ||

                        ( (psOutputConfig->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_444) && (psOutputConfig->sPixelInfo.ui32NoPlanes != 3) &&
                        !b444RotationSupport) )
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION WITH HIGH COLOUR DISPLAY PIXEL FORMAT",
                            psStrConfig->ui32UserStrId);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_ROTATION_WITH_HIGH_COLOUR;
                    }

                }
            }


            if (psOutputConfig->sScaledPictSize.ui32Width % 2 ||
                psOutputConfig->sScaledPictSize.ui32Height % 2)
            {
                // Must scale to even number of pixels in width and height.
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: SCALING TO AN ODD PIXEL RESOLUTION [%dx%d]",
                        psStrConfig->ui32UserStrId,
                        psOutputConfig->sScaledPictSize.ui32Width,
                        psOutputConfig->sScaledPictSize.ui32Height);
                psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_SCALING_SIZE;
            }

            if (psStrConfig &&
                (psOutputConfig->sPixelInfo.ui32BitDepthY > psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxLumaBitDepth ||
                 psOutputConfig->sPixelInfo.ui32BitDepthY < 8 || psOutputConfig->sPixelInfo.ui32BitDepthY == 9))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: DISPLAY PICTURE LUMA BIT DEPTH %d [RANGE: 8->%d for %s]",
                        psStrConfig->ui32UserStrId,
                        psOutputConfig->sPixelInfo.ui32BitDepthY,
                        psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxLumaBitDepth,
                        gapszVidStd[psStrConfig->eVidStd]);
                psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
            }

            if (psOutputConfig->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_MONO &&
               (psOutputConfig->sPixelInfo.ui32BitDepthC > psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxChromaBitDepth ||
                psOutputConfig->sPixelInfo.ui32BitDepthC < 8 || psOutputConfig->sPixelInfo.ui32BitDepthC == 9))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: DISPLAY PICTURE CHROMA BIT DEPTH %d [RANGE: 8->%d for %s]",
                        psStrConfig->ui32UserStrId,
                        psOutputConfig->sPixelInfo.ui32BitDepthC,
                        psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxChromaBitDepth,
                        gapszVidStd[psStrConfig->eVidStd]);
                psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_PIXFORMAT;
            }

            // Validate display configuration against existing stream configuration.
            if (psStrConfig->eVidStd == VDEC_STD_JPEG)
            {
                if (psOutputConfig->eRotMode != VDEC_ROTATE_0)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION WITH JPEG",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_X_WITH_JPEG;
                }
                if (psOutputConfig->bScale)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: SCALING WITH JPEG",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_X_WITH_JPEG;
                }
                if (psOutputConfig->bForceOold)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: OOLD WITH JPEG",
                        psStrConfig->ui32UserStrId);
                    psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_X_WITH_JPEG;
                }
            }
            }

        if (psDispPictRendInfo)
        {
            if (psReqPictRendInfo)
            {
                // Picture size declared in buffer must be at least as large as that required by bitstream/output config.
                if (!VDEC_SIZE_GE(psDispPictRendInfo->sRendPictSize, psReqPictRendInfo->sRendPictSize))
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] Picture size of output picture buffer [%dx%d] is not large enough for sequence [%dx%d]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->sRendPictSize.ui32Width, psDispPictRendInfo->sRendPictSize.ui32Height,
                            psReqPictRendInfo->sRendPictSize.ui32Width, psReqPictRendInfo->sRendPictSize.ui32Height);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_PICTURE_SIZE;
                }

                // Size of each plane must be at least as large as that required.
                if (psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Size < psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Size)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] Y plane of output picture buffer [%d bytes] is not large enough for bitstream/config [%d bytes]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Size,
                            psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Size);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_Y_SIZE;
                }

                // Stride of each plane must be at least as large as that required.
                if (psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Stride < psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Stride)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] Y stride of output picture buffer [%d bytes] is not large enough for bitstream/config [%d bytes]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Stride,
                            psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Stride);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_Y_STRIDE;
                }

                // Size of each plane must be at least as large as that required.
                if (psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Size < psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Size)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] UV plane of output picture buffer [%d bytes] is not large enough for bitstream/config [%d bytes]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Size,
                            psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Size);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_UV_SIZE;
                }

                // Stride of each plane must be at least as large as that required.
                if (psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Stride < psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Stride)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] UV stride of output picture buffer [%d bytes] is not large enough for bitstream/config [%d bytes]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Stride,
                            psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Stride);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_UV_STRIDE;
                }
            }

            {
                if ((psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Stride % VDEC_MSVDX_EXT_STRIDE_ALIGNMENT) != 0)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: Y STRIDE [%d] must be a multiple of %d bytes",
                        psStrConfig->ui32UserStrId,
                        psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_Y].ui32Stride,
                        VDEC_MSVDX_EXT_STRIDE_ALIGNMENT);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_64BYTE_STRIDE;
                }
                if ((psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Stride % VDEC_MSVDX_EXT_STRIDE_ALIGNMENT) != 0)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: UV STRIDE [%d] must be a multiple of %d bytes",
                        psStrConfig->ui32UserStrId,
                        psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_UV].ui32Stride,
                        VDEC_MSVDX_EXT_STRIDE_ALIGNMENT);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_64BYTE_STRIDE;
                }
                if ((psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_V].ui32Stride % VDEC_MSVDX_EXT_STRIDE_ALIGNMENT) != 0)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: V STRIDE [%d] must be a multiple of %d bytes",
                        psStrConfig->ui32UserStrId,
                        psDispPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_V].ui32Stride,
                        VDEC_MSVDX_EXT_STRIDE_ALIGNMENT);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_64BYTE_STRIDE;
                }
            }

            if (psReqPictRendInfo)
            {
                if (psOutputConfig)
                {
                    if (psOutputConfig->sPixelInfo.ui32NoPlanes <= 2)
                    {
                        // V plane only required when chroma is separated.
                        IMG_ASSERT(psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_V].ui32Size == 0);
                    }

                    if (psOutputConfig->sPixelInfo.ui32NoPlanes <= 3)
                    {
                        // Alpha planes should not be required.
                        IMG_ASSERT(psReqPictRendInfo->asPlaneInfo[VDEC_PLANE_VIDEO_A].ui32Size == 0);
                    }
                }

                // Size of buffer must be at least as large as that required.
                if (psDispPictRendInfo->ui32RenderedSize < psReqPictRendInfo->ui32RenderedSize)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] Output picture buffer [%d bytes] is not large enough for bitstream/config [%d bytes]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->ui32RenderedSize,
                            psReqPictRendInfo->ui32RenderedSize);
                    psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_BUFFER_SIZE;
                }
            }

            if (psOutputConfig)
            {
                if (psOutputConfig->bScale)
                {
                    /* ...the picture size has to be at least the scaled picture size. */
                    if (!VDEC_SIZE_GE(psDispPictRendInfo->sRendPictSize, psOutputConfig->sScaledPictSize))
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] Picture buffers [%dx%d] must be allocated at least as large as the scaled picture size [%dx%d]",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->sRendPictSize.ui32Width,
                            psDispPictRendInfo->sRendPictSize.ui32Height,
                            psOutputConfig->sScaledPictSize.ui32Width,
                            psOutputConfig->sScaledPictSize.ui32Height);
                        psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_PICTURE_SIZE;
                    }
                }
                else if (psComSequHdrInfo)
                {
                    if (VDEC_SIZE_LT(psDispPictRendInfo->sRendPictSize, psComSequHdrInfo->sMaxFrameSize))
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_INFO,
                            "[USERSID=0x%08X] Buffers [%dx%d] must be large enough to contain the maximum frame size [%dx%d] when not scaling",
                            psStrConfig->ui32UserStrId,
                            psDispPictRendInfo->sRendPictSize.ui32Width, psDispPictRendInfo->sRendPictSize.ui32Height,
                            psComSequHdrInfo->sMaxFrameSize.ui32Width, psComSequHdrInfo->sMaxFrameSize.ui32Height);
                        psUnsupported->ui32OutputBufConfig |= VDECDD_UNSUPPORTED_OUTPUTBUFCONFIG_PICTURE_SIZE;
                    }
                }
            }
        }

        if (psComSequHdrInfo)
        {
            IMG_UINT32 ui32MaxWidth = VDEC_SIZE_MIN(psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxWidth,
                                                    MAX_PLATFORM_SUPPORTED_WIDTH);
            IMG_UINT32 ui32MaxHeight = VDEC_SIZE_MIN(psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxHeight,
                                                     MAX_PLATFORM_SUPPORTED_HEIGHT);

            if ((psComSequHdrInfo->sMaxFrameSize.ui32Width > ui32MaxWidth) ||
                (psComSequHdrInfo->sMaxFrameSize.ui32Height > ui32MaxHeight))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: FRAME WIDTH %dpx or HEIGHT %dpx are over maximum allowed value [%d, %d]",
                        psStrConfig->ui32UserStrId,
                        psComSequHdrInfo->sMaxFrameSize.ui32Width,
                        psComSequHdrInfo->sMaxFrameSize.ui32Height,
                        ui32MaxWidth, ui32MaxHeight);
                psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_SIZE;
            }

            if (psComSequHdrInfo->bInterlacedFrames)
            {
                // Coded height should be rounded to pairs of MBs in order to store field pictures.
                if (psComSequHdrInfo->sMaxFrameSize.ui32Height != VDEC_ALIGN_SIZE(psComSequHdrInfo->sMaxFrameSize.ui32Height, 2 * VDEC_MB_DIMENSION))
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED HEIGHT %dpx not a multiple pair of MBs required for interlaced frame material [%dpx]",
                        psStrConfig->ui32UserStrId,
                        psComSequHdrInfo->sMaxFrameSize.ui32Height,
                        VDEC_ALIGN_SIZE(psComSequHdrInfo->sMaxFrameSize.ui32Height, 2 * VDEC_MB_DIMENSION));
                    psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_CODED_HEIGHT;
                }
            }

            if (psComSequHdrInfo->sPixelInfo.ui32BitDepthY > psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxLumaBitDepth ||
                psComSequHdrInfo->sPixelInfo.ui32BitDepthY < 8 || psComSequHdrInfo->sPixelInfo.ui32BitDepthY == 9)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE LUMA BIT DEPTH %d [RANGE: 8->%d for %s]",
                    psStrConfig->ui32UserStrId,
                    psComSequHdrInfo->sPixelInfo.ui32BitDepthY,
                    psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxLumaBitDepth,
                    gapszVidStd[psStrConfig->eVidStd]);
                psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_PIXFORMAT_BIT_DEPTH;
            }

            if (psComSequHdrInfo->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_MONO &&
                (psComSequHdrInfo->sPixelInfo.ui32BitDepthC > psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxChromaBitDepth ||
                psComSequHdrInfo->sPixelInfo.ui32BitDepthC < 8 || psComSequHdrInfo->sPixelInfo.ui32BitDepthC == 9))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE CHROMA BIT DEPTH %d [RANGE: 8->%d for %s]",
                    psStrConfig->ui32UserStrId,
                    psComSequHdrInfo->sPixelInfo.ui32BitDepthC,
                    psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxChromaBitDepth,
                    gapszVidStd[psStrConfig->eVidStd]);
                psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_PIXFORMAT_BIT_DEPTH;
            }

            if (psComSequHdrInfo->sPixelInfo.eChromaFormatIdc != PIXEL_FORMAT_MONO &&
                psComSequHdrInfo->sPixelInfo.ui32BitDepthY != psComSequHdrInfo->sPixelInfo.ui32BitDepthC)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE MIXED BIT DEPTH [%d vs %d]",
                    psStrConfig->ui32UserStrId,
                    psComSequHdrInfo->sPixelInfo.ui32BitDepthY,
                    psComSequHdrInfo->sPixelInfo.ui32BitDepthC);
                psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_PIXFORMAT_BIT_DEPTH;
            }

            if (psComSequHdrInfo->sPixelInfo.eChromaFormatIdc > psCoreProps->asVidStdProps[psStrConfig->eVidStd].eMaxChromaFormat)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PIXEL FORMAT IDC %s [MAX = %s for %s]",
                    psStrConfig->ui32UserStrId,
                    psComSequHdrInfo->sPixelInfo.eChromaFormatIdc < VDEC_ARRAY_SIZE(acPixelFormatIdcNames) ?
                        acPixelFormatIdcNames[psComSequHdrInfo->sPixelInfo.eChromaFormatIdc] : "Invalid",
                    acPixelFormatIdcNames[psCoreProps->asVidStdProps[psStrConfig->eVidStd].eMaxChromaFormat],
                    gapszVidStd[psStrConfig->eVidStd]);
                psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_PIXEL_FORMAT;
            }

            if (psComSequHdrInfo->sPixelInfo.eChromaFormatIdc == PIXEL_FORMAT_INVALID)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[SW]: UNKNOWN CODED PIXEL FORMAT",
                    psStrConfig->ui32UserStrId);
                psUnsupported->ui32SequHdr |= VDECDD_UNSUPPORTED_SEQUHDR_PIXEL_FORMAT;
            }
        }

        if (psPictHdrInfo && psComSequHdrInfo)
        {
            IMG_UINT32 ui32CodedCmdWidth;
            IMG_UINT32 ui32CodedCmdHeight;
            IMG_UINT32 ui32MinWidth = psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MinWidth;
            IMG_UINT32 ui32MinHeight = VDEC_ALIGN_SIZE(psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MinHeight, (psPictHdrInfo->bField) ? 2*VDEC_MB_DIMENSION : VDEC_MB_DIMENSION);
            IMG_UINT32 ui32PictSizeInMbs;
            IMG_UINT32 ui32MaxHeight = psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxHeight;
            IMG_UINT32 ui32MaxWidth = psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxWidth;
            IMG_UINT32 ui32MaxMBs = psCoreProps->asVidStdProps[psStrConfig->eVidStd].ui32MaxMacroblocks;

            /* For JPEG, max picture size of four plane images is 16k*16k. */
            if (psStrConfig->eVidStd == VDEC_STD_JPEG)
            {
                if (psComSequHdrInfo->sPixelInfo.ui32NoPlanes >= 4)
                {
                    ui32MaxWidth = (ui32MaxWidth > 16*1024) ? 16*1024 : ui32MaxWidth;
                    ui32MaxHeight = (ui32MaxHeight > 16*1024) ? 16*1024 : ui32MaxHeight;
                }
            }

            ui32CodedCmdWidth = VDEC_ALIGN_SIZE(psPictHdrInfo->sCodedFrameSize.ui32Width, VDEC_MB_DIMENSION);
            ui32CodedCmdHeight = VDEC_ALIGN_SIZE(psPictHdrInfo->sCodedFrameSize.ui32Height,
                                                (psPictHdrInfo->bField) ? 2*VDEC_MB_DIMENSION : VDEC_MB_DIMENSION);

            ui32PictSizeInMbs = (ui32CodedCmdWidth * ui32CodedCmdHeight) / (VDEC_MB_DIMENSION * VDEC_MB_DIMENSION);

            if ((psStrConfig->eVidStd == VDEC_STD_H264 &&
                ui32MaxMBs && ui32PictSizeInMbs > ui32MaxMBs) ||
                ui32CodedCmdWidth > ui32MaxWidth ||
                ui32CodedCmdHeight > ui32MaxHeight)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE SIZE %dx%d [MAX: %dx%d or %d MBs]",
                        psStrConfig->ui32UserStrId,
                        ui32CodedCmdWidth, ui32CodedCmdHeight,
                        ui32MaxWidth, ui32MaxHeight, ui32MaxMBs);
                psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_RESOLUTION;
            }

            if (psPictHdrInfo->sCodedFrameSize.ui32Width < ui32MinWidth ||
                psPictHdrInfo->sCodedFrameSize.ui32Height < ui32MinHeight)
            {
#ifdef USE_STRICT_MIN_PIC_SIZE_CHECK
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                       "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE SIZE %dx%d [MIN: %dx%d]",
                       psStrConfig->ui32UserStrId,
                       psPictHdrInfo->sCodedFrameSize.ui32Width,
                       psPictHdrInfo->sCodedFrameSize.ui32Height,
                       ui32MinWidth, ui32MinHeight);
                psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_RESOLUTION;
#else /* ndef USE_STRICT_MIN_PIC_SIZE_CHECK */
            IMG_WARN_ONCE("[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE SIZE %dx%d [MIN: %dx%d]",
                            psStrConfig->ui32UserStrId,
                            psPictHdrInfo->sCodedFrameSize.ui32Width,
                            psPictHdrInfo->sCodedFrameSize.ui32Height,
                            ui32MinWidth, ui32MinHeight);
#endif /* ndef USE_STRICT_MIN_PIC_SIZE_CHECK */
            }

            if (psStrConfig->eVidStd == VDEC_STD_MPEG4 ||
                psStrConfig->eVidStd == VDEC_STD_H263 ||
                psStrConfig->eVidStd == VDEC_STD_SORENSON)
            {
                if (ui32CodedCmdWidth > 2048 || ui32CodedCmdHeight > 2048)
                {
                    REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                        "[USERSID=0x%08X] UNSUPPORTED[HW]: CODED PICTURE SIZE %dx%d [MAX: %d] FOR EITHER DIMENSION FOR MPEG4",
                        psStrConfig->ui32UserStrId,
                        ui32CodedCmdWidth, ui32CodedCmdHeight,
                        2048);
                    psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_RESOLUTION;
                }
            }

            if (psPictHdrInfo->sPictSgmData.ui32Id != BSPP_INVALID && (psPictHdrInfo->sCodedFrameSize.ui32Width > 1280))
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "[USERSID=0x%08X] UNSUPPORTED[HW]: SGM & coded frame width > 1280",
                    psStrConfig->ui32UserStrId);
                psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_OVERSIZED_SGM;
            }

            if (psPictHdrInfo->bDiscontinuousMbs)
            {
                DEBUG_REPORT(REPORT_MODULE_DECODER, "Stream has Discontinuous Macroblocks");
            }

            if (psOutputConfig)
            {
                if (psOutputConfig->bScale)
                {
                    IMG_UINT32 ui32DisplayCmdWidth;
                    IMG_UINT32 ui32DisplayCmdHeight;

                    ui32DisplayCmdWidth = psPictHdrInfo->sDispInfo.sEncDispRegion.ui32Width +
                                              psPictHdrInfo->sDispInfo.sEncDispRegion.ui32LeftOffset;
                    ui32DisplayCmdHeight = psPictHdrInfo->sDispInfo.sEncDispRegion.ui32Height +
                                              psPictHdrInfo->sDispInfo.sEncDispRegion.ui32TopOffset;

                    // Scaling to a larger size than the display picture.
                    if (psOutputConfig->sScaledPictSize.ui32Width > ui32DisplayCmdWidth ||
                        psOutputConfig->sScaledPictSize.ui32Height > ui32DisplayCmdHeight)
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: UPSCALING [%dx%d] to [%dx%d]",
                            psStrConfig->ui32UserStrId,
                            ui32DisplayCmdWidth,
                            ui32DisplayCmdHeight,
                            psOutputConfig->sScaledPictSize.ui32Width,
                            psOutputConfig->sScaledPictSize.ui32Height);
                        psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_UPSCALING;
                    }

                    if (ui32DisplayCmdWidth < 64UL || ui32DisplayCmdHeight < 64UL)
                    {
                        // Cannot scale when original display resolution is less than 4x4 MBs.
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: Cannot scale down from [%dx%d]. MIN [64x64]",
                            psStrConfig->ui32UserStrId,
                            ui32DisplayCmdWidth, ui32DisplayCmdHeight);
                        psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_SCALING_ORIGINALSIZE;
                    }

                    if (psOutputConfig->sScaledPictSize.ui32Width % 2)
                    {
                        // Must scale to even number of pixels in width.
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                                "[USERSID=0x%08X] UNSUPPORTED[HW]: SCALING WIDTH MUST BE MULTIPLE OF 2 [%dx%d]",
                                psStrConfig->ui32UserStrId,
                                psOutputConfig->sScaledPictSize.ui32Width,
                                psOutputConfig->sScaledPictSize.ui32Height);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_PICTHDR_SCALING_SIZE;
                    }

                    if (psOutputConfig->sScaledPictSize.ui32Height % (psPictHdrInfo->bField?4:2))
                    {
                        // Must scale to even number of pixels in field height (even pair in frame height).
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                                "[USERSID=0x%08X] UNSUPPORTED[HW]: SCALING HEIGHT MUST BE MULTIPLE OF 2 OR FIELD-CODED 4 [%dx%d]",
                                psStrConfig->ui32UserStrId,
                                psOutputConfig->sScaledPictSize.ui32Width,
                                psOutputConfig->sScaledPictSize.ui32Height);
                        psUnsupported->ui32StrOutputConfig |= VDECDD_UNSUPPORTED_PICTHDR_SCALING_SIZE;
                    }

                    if( (((1UL << (psCoreProps->ui32ScalingPrecision + 3)) - 1) * psOutputConfig->sScaledPictSize.ui32Width < ((1UL << psCoreProps->ui32ScalingPrecision) * ui32DisplayCmdWidth) ) ||
                        (((1UL << (psCoreProps->ui32ScalingPrecision + 3)) - 1) * psOutputConfig->sScaledPictSize.ui32Height < ((1UL << psCoreProps->ui32ScalingPrecision) * ui32DisplayCmdHeight) ))
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: Cannot scale down that far [%dx%d] to [%dx%d]",
                            psStrConfig->ui32UserStrId,
                            ui32DisplayCmdWidth,
                            ui32DisplayCmdHeight,
                            psOutputConfig->sScaledPictSize.ui32Width,
                            psOutputConfig->sScaledPictSize.ui32Height);
                        psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_PICTHDR_SCALING_SIZE;
                    }
                    }

                if (psComSequHdrInfo)
                {
                    if (psOutputConfig->eRotMode != VDEC_ROTATE_0 &&
                        (psPictHdrInfo->bField || psComSequHdrInfo->bFieldCodedMBlocks ||
                        (psStrConfig->eVidStd == VDEC_STD_VC1 && psPictHdrInfo->sVC1PictHdrInfo.bInterlacedFrame)))
                    {
                        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                            "[USERSID=0x%08X] UNSUPPORTED[HW]: ROTATION OF FIELD CODED PICTURE",
                            psStrConfig->ui32UserStrId);
                        psUnsupported->ui32PictHdr |= VDECDD_UNSUPPORTED_OUTPUTCONFIG_ROTATION_WITH_FIELDS;
                    }
                }
            }

            decoder_GetRequiredCoreFeatures(psStrConfig, psOutputConfig, pui32Features);
        }

        if (psUnsupported->ui32StrConfig == 0 &&
            psUnsupported->ui32StrOutputConfig == 0 &&
            psUnsupported->ui32OutputBufConfig == 0 &&
            //psUnsupported->ui32SequHdr == 0 &&
            psUnsupported->ui32PictHdr == 0)
        {
            ui32Result = IMG_SUCCESS;
    }

error:
    return ui32Result;
}



/*!
******************************************************************************

 @Function              DECODER_StreamCreate

******************************************************************************/
IMG_RESULT
DECODER_StreamCreate(
    IMG_HANDLE              hDecCtx,
    SYS_eMemPool            eSecureMemPool,
    VDEC_sStrConfigData     sStrConfig,
    IMG_HANDLE            * phMmuStrHandle,
    IMG_VOID              * pvStrUserData,
    IMG_VOID              * pvStrUserIntData,
    IMG_HANDLE            * phDecStrCtx
)
{
    DECODER_sContext *     psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sStrContext *  psDecStrCtx = IMG_NULL;
    IMG_UINT32             ui32I;
    IMG_RESULT             ui32Result;
    VXDIO_sMemPool         sInsecureMemPool;

    /* Check input parameters. */
    IMG_ASSERT(IMG_NULL != hDecCtx);
    if(hDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if(psDecCtx->ui32StrCnt >= MAX_CONCURRENT_STREAMS)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "Device has too many concurrent streams. Number of Concurrent streams allowed: %d.",
            MAX_CONCURRENT_STREAMS);
        return IMG_ERROR_DEVICE_UNAVAILABLE;
    }

    sInsecureMemPool.eMemPoolId = psDecCtx->eInsecurePool;
    sInsecureMemPool.eMemPoolType = VXDIO_MEMPOOL_INSECURE;

#ifdef VDEC_USE_PVDEC_SEC
    {
        DECODER_sCoreContext * psDecCoreCtx = psDecCtx->psDecCoreCtx;
        VXDIO_sMemPool sMemPool = sInsecureMemPool;

        IMG_ASSERT(psDecCtx->psDecCoreCtx);

        if (sStrConfig.bSecureStream)
        {
            sMemPool.eMemPoolId = psDecCtx->eSecurePool = eSecureMemPool;
            sMemPool.eMemPoolType = VXDIO_MEMPOOL_SECURE;
        }

        HWCTRL_SetSecured(psDecCoreCtx->hHwCtx, sStrConfig.bSecureStream);

        /* Obtain firmware */
        ui32Result = decoder_GetFirmware(psDecCtx->hMmuDevHandle,
                                         psDecCtx->hDevHandle,
                                         sMemPool,
                                         sStrConfig.bSecureStream,
                                         &psDecCtx->sFirmware);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        if ((psDecCtx->eDecFwConf == eDCONF_FwNone) ||
            (sStrConfig.bSecureStream && psDecCtx->eDecFwConf == eDCONF_FwNonSecure) ||
            (!sStrConfig.bSecureStream && psDecCtx->eDecFwConf == eDCONF_FwSecure))
        {
            ui32Result = HWCTRL_PowerOff(psDecCoreCtx->hHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);

            // Create core resources.
            ui32Result = RESOURCE_Create(psDecCtx->hMmuDevHandle,
                                         &psDecCoreCtx->sCoreProps,
                                         &psDecCoreCtx->sRendec,
                                         psDecCtx->psDevConfig->ui32NumSlotsPerPipe * psDecCtx->ui32NumPipes,
                                         sMemPool,
                                         sInsecureMemPool,
                                         psDecCoreCtx->asRendecBufInfo,
                                         &psDecCoreCtx->hResources,
                                         sStrConfig.bSecureStream);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }

            ui32Result = HWCTRL_Configure(psDecCoreCtx->hHwCtx,
                                          &psDecCtx->sFirmware,
                                          psDecCoreCtx->asRendecBufInfo,
                                          &psDecCtx->sPtdBufInfo,
                                          &psDecCtx->sPtdInfo);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }

            psDecCtx->eDecFwConf = sStrConfig.bSecureStream ? eDCONF_FwSecure : eDCONF_FwNonSecure;
        }
    }
#endif /* VDEC_USE_PVDEC_SEC */

    VDEC_MALLOC(psDecStrCtx);
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "[USERSID=0x%08X] Failed to allocate memory for Decoder stream context",
            sStrConfig.ui32UserStrId);
        return IMG_ERROR_OUT_OF_MEMORY;
    }

    // Increment the stream counter
    psDecCtx->ui32StrCnt++;

    // Initialise the context structure to NULL. Any non-zero
    // default values should be set at this point.
    VDEC_BZERO(psDecStrCtx);
    psDecStrCtx->sConfig = sStrConfig;
    psDecStrCtx->pvUserData = pvStrUserData;
    psDecStrCtx->pvUserIntData = pvStrUserIntData;
    psDecStrCtx->psDecCtx = psDecCtx;

    decoder_InitAvailPipes(psDecStrCtx);

    psDecStrCtx->ui32NextDecPictId = 1;
    psDecStrCtx->ui32NextPictIdExpected = 1;

    ui32Result = IDGEN_AllocId(psDecCtx->hStrIdGen, psDecStrCtx, &psDecStrCtx->ui32StrId);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
    IMG_ASSERT(psDecStrCtx->ui32StrId > 0);

    LST_init(&psDecStrCtx->sPendStrUnitList);
    DQ_init(&psDecStrCtx->sStrDecdPictList);
    LST_init(&psDecStrCtx->sRefResList);
    LST_init(&psDecStrCtx->sDecResList);

    ui32Result = IDGEN_CreateContext(DECODER_MAX_PICT_ID + 1, /* IDGEN will allocate ids < DECODER_MAX_PICT_ID while we need <= DECODER_MAX_PICT_ID so add 1 */
                        DECODER_MAX_CONCURRENT_PICT,
                        IMG_TRUE,   /* We need incrementing Ids */
                        &psDecStrCtx->hPictIdGen);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    /* Create an MMU context for this stream. */
    ui32Result = MMU_StreamCreate(psDecCtx->hMmuDevHandle,
                                  &psDecStrCtx->hMmuStrHandle);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    // Obtain stream PTD physical address.
    MMU_StreamWritePtdToMem(psDecStrCtx->hMmuStrHandle,
                            &psDecStrCtx->psDecCtx->sPtdBufInfo,
                            psDecStrCtx->ui32StrId * sizeof(IMG_UINT32));

    /* Allocate the PVDEC firmware context buffer */
    ui32Result = MMU_StreamMalloc(psDecStrCtx->hMmuStrHandle,
            MMU_HEAP_STREAM_BUFFERS,
            sInsecureMemPool,
            CONTEXT_BUFF_SIZE,
            DEV_MMU_PAGE_ALIGNMENT,
            &psDecStrCtx->sPvdecFwCtxBuf);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    // Clear the context data in preparation for first time use by the firmware.
    IMG_MEMSET(psDecStrCtx->sPvdecFwCtxBuf.pvCpuVirt, 0, psDecStrCtx->sPvdecFwCtxBuf.ui32BufSize);
    UPDATE_DEVICE(&psDecStrCtx->sPvdecFwCtxBuf);

    // Create enough device resources to hold last context on
    // front and back-end for stream.
    psDecStrCtx->ui32NumDecRes = psDecStrCtx->psDecCtx->psDevConfig->ui32NumSlotsPerPipe * psDecStrCtx->psDecCtx->ui32NumPipes;
    for (ui32I = 0; ui32I < psDecStrCtx->ui32NumDecRes + 2; ui32I++)
    {
        ui32Result = decoder_StreamDecodeResourceCreate(psDecStrCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
    }

    LST_add(&psDecCtx->sStreamList, psDecStrCtx);

    *phDecStrCtx = (IMG_HANDLE)psDecStrCtx;
    *phMmuStrHandle = psDecStrCtx->hMmuStrHandle;

    return IMG_SUCCESS;

    /* Roll back in case of errors. */
error:
    if (psDecStrCtx)
    {
    DECODER_StreamDestroy((IMG_HANDLE)psDecStrCtx, IMG_FALSE);
    }

    return ui32Result;
}


#ifdef USE_REAL_FW

#ifdef WIN32
    /*! DMAC_LinkElement alignment not required under Windows. */
#   define DMAC_ALIGN_16_BYTE
#else
    /*! DMAC_LinkElement must be 16-byte alligned */
#   define DMAC_ALIGN_16_BYTE       __attribute__ ((aligned (16)))
#endif

// XXX-matt: this should be removed.
/*! Hardware defined of fields of linked list element */
typedef struct
{
    IMG_UINT32  ui32DMAWord_0;      /*!< Word 0 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_1;      /*!< Word 1 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_2;      /*!< Word 2 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_3;      /*!< Word 3 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_4;      /*!< Word 4 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_5;      /*!< Word 5 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_6;      /*!< Word 6 of DMA linked-list. */
    IMG_UINT32  ui32DMAWord_7;      /*!< Word 7 of DMA linked-list. */
} DMAC_LinkElement DMAC_ALIGN_16_BYTE;

/*!
******************************************************************************

 @Function              decoder_GetFirmware

 @Description

 This function provides relevant firmware text, data, and relocation buffers
 according to configuration.

 Usage: only once during initialization, before RESOURCE_Create() and
        decoder_CoreEnumerate() calls.

 @Input     hMmuDevHandle           : MMU device context handle.

 @Input     hDevHandle              : Device context handle.

 @Input     sMemPool                : Memory pool to use for allocations.

 @Input     abSupportedStandards    : Array of supported standards.

 @Input     bSecure                 : Secure firmware flag

 @Output    psFw                    : Pointer to structure with firmware
                                      buffers configuration.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

 ******************************************************************************/
static IMG_RESULT
decoder_GetFirmware(
    IMG_HANDLE                  hMmuDevHandle,
    IMG_HANDLE                  hDevHandle,
    VXDIO_sMemPool              sMemPool,
    IMG_BOOL                    bSecure,
    VXD_sFirmware             * psFirmware
)
{
    IMG_RESULT  ui32Result = IMG_SUCCESS;
    IMG_UINT32  ui32Size = 0;
    IMG_BOOL    bAcquire = IMG_FALSE;

    IMG_ASSERT(psFirmware);

    if (bSecure == IMG_FALSE)
    {
        /* Get the firmware info. */
        ui32Result = VDECFW_GetPvdecFirmware(&psFirmware->sFwBaseBinInfo,
                                             IMG_FALSE);
    IMG_ASSERT_TESTED(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                   "Unable to obtain PVDEC firmware");
        return ui32Result;
    }

        bAcquire = RESOURCE_ItemIsAvailable(&psFirmware->sFwBaseBufInfo.ui32RefCount);
#if defined VDEC_USE_PVDEC_SEC && defined SYSBRG_BRIDGING
        // sanity check
        IMG_ASSERT(psFirmware->sFwBaseBinInfo.aui32Text == IMG_NULL);

        if (bAcquire)
    {
            // take firmware from file system
            ui32Result = DMANKM_AcquireDevFirmware(hDevHandle,
                                                   PVDEC_FULL_FW_NAME,
                                                   IMG_FALSE, // always give virtual address
                                                   (IMG_PUINT8 *)&psFirmware->sFwBaseBinInfo.aui32Text,
                                                   &ui32Size);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

            // Sanity check of firmware file - assuming it's not encrypted
            ui32Result = VDECFW_InspectPvdecFirmware((IMG_PUINT8 )psFirmware->sFwBaseBinInfo.aui32Text,
                                                     ui32Size,
                                                     IMG_FALSE);
        if (ui32Result != IMG_SUCCESS)
        {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                       "PVDEC firmware inspection failed !");
            return ui32Result;
        }

            /* Store firmware size in words */
            psFirmware->sFwBaseBinInfo.ui32TextBufSize = ui32Size / sizeof(IMG_UINT32);
        }

#else
        /* Get the buffer size form built-in info */
        ui32Size = psFirmware->sFwBaseBinInfo.ui32TextBufSize * sizeof(IMG_UINT32);
#endif /* VDEC_USE_PVDEC_SEC && SYSBRG_BRIDGING */

        /* Allocate buffer for firmware blob */
        if (bAcquire && ui32Size)
        {
        ui32Result = MMU_DeviceMalloc(hMmuDevHandle,
                                      MMU_HEAP_DEVICE_BUFFERS,
                                          sMemPool, //Secure if Secure allocation
                                          ui32Size,
                                      DEV_MMU_PAGE_ALIGNMENT,
                                          &psFirmware->sFwBaseBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }

        RESOURCE_ItemUse(&psFirmware->sFwBaseBufInfo.ui32RefCount);
    }

#ifdef VDEC_USE_PVDEC_SEC
    if (bSecure == IMG_TRUE)
    {
        /* Get the secure firmware info. */
        ui32Result = VDECFW_GetPvdecFirmware(&psFirmware->sFwSecBinInfo,
                                             IMG_TRUE);
            IMG_ASSERT_TESTED(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                   "Unable to obtain PVDEC secure firmware");
                return ui32Result;
            }

        bAcquire = RESOURCE_ItemIsAvailable(&psFirmware->sFwSecBufInfo.ui32RefCount);
#ifdef SYSBRG_BRIDGING
        // sanity check
        IMG_ASSERT(psFirmware->sFwSecBinInfo.aui32Text == IMG_NULL);

        if (bAcquire)
        {
            // take firmware from file system
            ui32Result = DMANKM_AcquireDevFirmware(hDevHandle,
                                                   PVDEC_SEC_FW_NAME,
                                                   IMG_FALSE, // give virt address
                                                   (IMG_PUINT8 *)&psFirmware->sFwSecBinInfo.aui32Text,
                                                   &ui32Size);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }

            // Sanity check of firmware file - assuming it's not encrypted
            ui32Result = VDECFW_InspectPvdecFirmware((IMG_PUINT8 )psFirmware->sFwSecBinInfo.aui32Text,
                                                     ui32Size,
                                                     IMG_TRUE);
                if (ui32Result != IMG_SUCCESS)
                {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                       "PVDEC secure firmware inspection failed !");
                    return ui32Result;
                }
            /* Store firmware size in words */
            psFirmware->sFwSecBinInfo.ui32TextBufSize = ui32Size / sizeof(IMG_UINT32);
        }

#else
        /* Get the buffer size form built-in info */
        ui32Size = psFirmware->sFwSecBinInfo.ui32TextBufSize * sizeof(IMG_UINT32);
#endif /* SYSBRG_BRIDGING */

        /* Allocate buffer for the firmware blob */
        if (bAcquire && ui32Size)
        {
                ui32Result = MMU_DeviceMalloc(hMmuDevHandle,
                                              MMU_HEAP_DEVICE_BUFFERS,
                                          sMemPool, //Secure if Secure allocation
                                          ui32Size,
                                              DEV_MMU_PAGE_ALIGNMENT,
                                          &psFirmware->sFwSecBufInfo);
                IMG_ASSERT(ui32Result == IMG_SUCCESS);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }
            }

        RESOURCE_ItemUse(&psFirmware->sFwSecBufInfo.ui32RefCount);
        }
#endif /* not VDEC_USE_PVDEC_SEC */

    return ui32Result;
}

/*!
******************************************************************************

 @Function              decoder_DestroyFirmware

 @Description


 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

 ******************************************************************************/
static IMG_RESULT
decoder_DestroyFirmware(
    VXD_sFirmware   * psFirmware,
    IMG_HANDLE        hDevHandle
)
{
    IMG_UINT32      ui32Result;
    IMG_BOOL   bRelease = IMG_FALSE;

    IMG_ASSERT(psFirmware);

    RESOURCE_ItemReturn(&psFirmware->sFwBaseBufInfo.ui32RefCount);

    bRelease = RESOURCE_ItemIsAvailable(&psFirmware->sFwBaseBufInfo.ui32RefCount) &&
                   psFirmware->sFwBaseBufInfo.hMemory;

#if defined(VDEC_USE_PVDEC_SEC) && defined(SYSBRG_BRIDGING)
    if (bRelease)
    {
        ui32Result = DMANKM_ReleaseDevFirmware(hDevHandle, PVDEC_FULL_FW_NAME);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        }
#endif

    if (bRelease)
    {
        ui32Result = MMU_FreeMem(&psFirmware->sFwBaseBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        psFirmware->sFwBaseBufInfo.hMemory = IMG_NULL;
    }

#ifdef VDEC_USE_PVDEC_SEC
    RESOURCE_ItemReturn(&psFirmware->sFwSecBufInfo.ui32RefCount);

    bRelease = RESOURCE_ItemIsAvailable(&psFirmware->sFwSecBufInfo.ui32RefCount) &&
                   psFirmware->sFwSecBufInfo.hMemory;

#ifdef SYSBRG_BRIDGING
    if (bRelease)
    {
        ui32Result = DMANKM_ReleaseDevFirmware(hDevHandle, PVDEC_SEC_FW_NAME);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            }
#endif

    if (bRelease)
        {
        ui32Result = MMU_FreeMem(&psFirmware->sFwSecBufInfo);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }

        psFirmware->sFwSecBufInfo.hMemory = IMG_NULL;
        }
#endif /* VDEC_USE_PVDEC_SEC */

    return IMG_SUCCESS;

error:
    return ui32Result;
}

#endif /* USE_REAL_FW */


/*!
******************************************************************************

 @Function              DECODER_Deinitialise

******************************************************************************/
IMG_RESULT DECODER_Deinitialise(
    IMG_HANDLE      hDecCtx
)
{
    DECODER_sContext * psDecCtx = (DECODER_sContext*)hDecCtx;
    IMG_RESULT ui32Result;

    if (psDecCtx && psDecCtx->bInitialised)
    {
        DECODER_sCoreContext * psDecCoreCtx = psDecCtx->psDecCoreCtx;

        IMG_ASSERT(psDecCoreCtx);
        // Stream list should be empty.
        IMG_ASSERT(LST_empty(&psDecCtx->sStreamList));

        // Remove and free all core context structures
        {
            DECODER_sDecPict * psDecPict;

            // All cores should now be idle since there are no connections/streams.
            ui32Result = HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict);
            IMG_ASSERT(ui32Result != IMG_SUCCESS);

            /* Power Off the core */
            ui32Result = HWCTRL_PowerOff(psDecCoreCtx->hHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);

#ifdef USE_REAL_FW
                /* Destroy firmware. */
            ui32Result = decoder_DestroyFirmware(&psDecCtx->sFirmware,
                                                 psDecCoreCtx->psDecCtx->hDevHandle);
                if (ui32Result != IMG_SUCCESS)
                {
                    return ui32Result;
                }
#endif /* USE_REAL_FW */

            if (psDecCoreCtx->hResources)
            {
            RESOURCES_Destroy(psDecCoreCtx->hResources);
                psDecCoreCtx->hResources = IMG_NULL;
            }
        }

        // Free memory to store PTD base addresses.
        ui32Result = MMU_FreeMem(&psDecCtx->sPtdBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        /* Destroy a device MMU context. */
        ui32Result = MMU_DeviceDestroy(psDecCtx->hMmuDevHandle);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }

        // Remove and free core context structure
        psDecCoreCtx = psDecCtx->psDecCoreCtx;
        IMG_ASSERT(psDecCoreCtx);
            if(psDecCoreCtx->bAPM)
            {
                // Do a resume to shut down properly. Do not lock, it has been already locked previously
                DMANKM_ResumeDevice(psDecCoreCtx->psDecCtx->hDevHandle, IMG_FALSE);
                psDecCoreCtx->bAPM = IMG_FALSE;
            }
            HWCTRL_Deinitialise(psDecCoreCtx->hHwCtx);

            IMG_FREE(psDecCoreCtx);
            psDecCoreCtx = IMG_NULL;

        IMG_ASSERT(psDecCtx->psDevConfig);
        if (psDecCtx->psDevConfig)
        {
            IMG_FREE(psDecCtx->psDevConfig);
            psDecCtx->psDevConfig = IMG_NULL;
        }

        IDGEN_DestroyContext(psDecCtx->hStrIdGen);

        psDecCtx->pfnCompCallback = IMG_NULL;
        psDecCtx->pvCompInitUserData = IMG_NULL;

#if defined(FW_PERFORMANCE_LOGGING) && !defined(VDEC_USE_PVDEC) && !defined(SYSBRG_BRIDGING)
        PERFLOG_Deinitialise();
#endif /* defined(FW_PERFORMANCE_LOGGING) && !defined(VDEC_USE_PVDEC) && !defined(SYSBRG_BRIDGING) */
        psDecCtx->bInitialised = IMG_FALSE;

        IMG_FREE(psDecCtx);
        psDecCtx = IMG_NULL;
    }
    else
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
            "Decoder has not been initialised so cannot be de-initialised");
        return IMG_ERROR_NOT_INITIALISED;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_Replay

******************************************************************************/
IMG_RESULT DECODER_Replay(
    IMG_HANDLE      hDecCtx
)
{
    DECODER_sContext * psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;
    IMG_RESULT ui32Result;
    DECODER_sDecPict * psDecPict;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT (psDecCoreCtx);
    if (IMG_SUCCESS == HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict))
    {
        if(psDecCoreCtx->bAPM)
        {
            DMANKM_ResumeDevice(psDecCtx->hDevHandle, IMG_TRUE);
            psDecCoreCtx->bAPM = IMG_FALSE;
        }

        ui32Result = HWCTRL_CoreReplay(psDecCoreCtx->hHwCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
    else
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "DECODER_Replay-->Nothing to replay!");
    }
    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              DECODER_ResetCores

******************************************************************************/
IMG_RESULT DECODER_ResetCore(
    IMG_HANDLE      hDecCtx
)
{
    DECODER_sContext * psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;
    IMG_RESULT ui32Result;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
    ui32Result = HWCTRL_PowerOff(psDecCoreCtx->hHwCtx);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_PowerOff

******************************************************************************/
IMG_RESULT
DECODER_PowerOff(
    IMG_HANDLE          hDecCtx,
    IMG_BOOL            bForceAll
)
{
    DECODER_sContext *      psDecCtx = (DECODER_sContext *)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;
    DECODER_sDecPict * psDecPict;
    IMG_UINT32  ui32Result = IMG_SUCCESS;

    /* Check input parameters. */
    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
            "Invalid parameters!");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);

    if(!bForceAll)
    {
        // Peek the head of the core decode queue to determine whether this
        // picture needs to be processed. If nothing, we can shut it down
        ui32Result = HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict);
    }

    // If we are focing the cores to be turned off (PPM), or we have nothing on the core (APM)
    // Powering Off/On when we have one core. Need to change the interface to SYSDEVKM_ApmDeviceSuspend
    // to take Core ID as well to suspend all.
    if( bForceAll || (ui32Result != IMG_SUCCESS) )
    {
        // PrePower down steps
        ui32Result = HWCTRL_PowerOff(psDecCoreCtx->hHwCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
        if(!bForceAll && !psDecCoreCtx->bAPM)
        {
            // We are actively going down.
            if (DMANKM_SuspendDevice(psDecCtx->hDevHandle))
            {
                psDecCoreCtx->bAPM = IMG_TRUE;
            }
        }
    }

    return IMG_SUCCESS;

error:

    return ui32Result;
}


/*!
******************************************************************************

 @Function              DECODER_PrePowerOff

******************************************************************************/
IMG_RESULT
DECODER_PrePowerOff(
    IMG_HANDLE          hDecCtx
)
{
    IMG_UINT32  ui32Result;
    DECODER_sContext *      psDecCtx = (DECODER_sContext *)hDecCtx;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    ui32Result = DECODER_PowerOff(hDecCtx, IMG_TRUE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

#if defined(VDEC_PM_EVENTS_INJECTION) && defined(VDEC_MSVDX_HARDWARE)
    {
        IMG_UINT32      ui32Offset;
        IMG_UINT32      ui32VecRamVal = 0;
        DECODER_sCoreContext *  psDecCoreCtx;

        //clear VLR for each core
        psDecCoreCtx = psDecCtx->psDecCoreCtx;
        IMG_ASSERT(psDecCoreCtx);
            for (ui32Offset = 0; ui32Offset < VLR_SIZE; ui32Offset += 4)
            {
                MSVDXIO_VLRWriteWords(IMG_NULL, REGION_VLRFE_REGSPACE, ui32Offset, 1, &ui32VecRamVal );
            }
    }
#endif // VDEC_PM_EVENTS_INJECTION && VDEC_MSVDX_HARDWARE

    DEBUG_REPORT(REPORT_MODULE_DECODER,
                 "Decoder has performed pre power off operations");

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_PostPowerOn

******************************************************************************/
IMG_RESULT
DECODER_PostPowerOn(
    IMG_HANDLE          hDecCtx
)
{
    DECODER_sContext *      psDecCtx = (DECODER_sContext *)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;
    IMG_RESULT ui32Result;
    DECODER_sDecPict * psDecPict;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
    if (IMG_SUCCESS == HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict))
    {
        ui32Result = HWCTRL_CoreReplay(psDecCoreCtx->hHwCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }

    DEBUG_REPORT(REPORT_MODULE_DECODER,
                 "Decoder has performed post power on operations");

    return IMG_SUCCESS;
}



/*!
******************************************************************************

 @Function              DECODER_CoreServiceTimerExpiry

******************************************************************************/
IMG_RESULT
DECODER_CoreServiceTimerExpiry(
    IMG_HANDLE  hDecCtx
)
{
    DECODER_sContext *      psDecCtx = (DECODER_sContext *)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
    // find the effected core
    if (HWCTRL_HwStateIsLockUp(psDecCoreCtx->hHwCtx))
    {
        DECODER_CoreReset(hDecCtx, RESET_DWR);

        DEBUG_REPORT(REPORT_MODULE_DECODER,
                     "Decoder has Serviced a Service Time Expiry");
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              DECODER_CoreReset

******************************************************************************/
IMG_RESULT
DECODER_CoreReset(
    IMG_HANDLE  hDecCtx,
    VDECDD_eResetReason eReason
)
{

    IMG_UINT32  ui32Result = IMG_SUCCESS;
    DECODER_sContext *      psDecCtx = (DECODER_sContext *)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    DEBUG_REPORT(REPORT_MODULE_DECODER,
                 "Decoder has started servicing a reset (reason=%d)", eReason);

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);

    ui32Result = HWCTRL_PowerOff(psDecCoreCtx->hHwCtx);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
#ifdef VDEC_USE_PVDEC_SEC
    {
        IMG_UINT32 ui32LockUpPID = 0;
        if (HWCTRL_HasFwBspLockUp(psDecCoreCtx->hHwCtx, &ui32LockUpPID))
        {
            DECODER_sStrContext * psDecStrCtx;
            IMG_UINT32 ui32i = 0;

            psDecStrCtx = LST_first(&psDecCoreCtx->psDecCtx->sStreamList);
            while (psDecStrCtx)
            {
                do
                {
                    ui32Result = FWBSP_BRIDGE_DiscardBitstreamUnit(psDecStrCtx->hFWBSP_BRIDGEStrCtx,
                                                                   ui32LockUpPID);
                    ui32i++;
                }
                while (ui32Result == IMG_SUCCESS);

                psDecStrCtx = LST_next(psDecStrCtx);
            }
            REPORT(REPORT_MODULE_DECODER, REPORT_INFO, "Dropping %u bstrbufs!", ui32i);
        }
    }
    // xxx:todo: If picture decode hasn't been started yet, the below call will not make any harm ?
#endif
            // Throw away the picture
            // XXX-michals: I don't like that DECODER_CoreReset is broken down into multiple pieces
            // XXX-michals: This will be serviced on next spin of CORE_ProcessEvent
            // XXX-michals: What if some other message get posted to the list in-between?
            ui32Result = HWCTRL_DiscardHeadPicList(psDecCoreCtx->hHwCtx, eReason);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }

            // Post a replay here
            // XXX-michals: I don't like that DECODER_CoreReset is broken down into multiple pieces
            // XXX-michals: This will be serviced on next spin of CORE_ProcessEvent
            // XXX-michals: What if some other message get posted to the list in-between?
            // XXX-michals: I think we should (?) do it synchrounously like that:
            // XXX-michals: ui32Result = DECODER_CoreReplay(hDecCtx, psDecCoreCtx->ui32CoreNum);
            ui32Result = HWCTRL_PostCoreReplay(psDecCoreCtx->hHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                goto error;
            }

error:
    DEBUG_REPORT(REPORT_MODULE_DECODER,
                 "Decoder has serviced a reset (reason=%d)", eReason);

    return ui32Result;
}

/*!
******************************************************************************

 @Function              DECODER_CoreReplay

******************************************************************************/
IMG_RESULT
DECODER_CoreReplay(
    IMG_HANDLE  hDecCtx
)
{

    IMG_UINT32  ui32Result = IMG_SUCCESS;
    DECODER_sContext *      psDecCtx = (DECODER_sContext *)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;
    DECODER_sDecPict * psDecPict;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
    if (IMG_SUCCESS == HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict))
    {
        if(psDecCoreCtx->bAPM)
        {
            DMANKM_ResumeDevice(psDecCtx->hDevHandle, IMG_TRUE);
            psDecCoreCtx->bAPM = IMG_FALSE;
        }
        ui32Result = HWCTRL_CoreReplay(psDecCoreCtx->hHwCtx);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            return ui32Result;
        }
    }
    else
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "DECODER_CoreReplay-->Nothing to replay!");
    }
    return IMG_SUCCESS;
}


/*!
 ******************************************************************************
 
 @Function              DECODER_IsHWQueueEmpty

******************************************************************************/
IMG_BOOL
DECODER_IsHWQueueEmpty(
    IMG_HANDLE          hDecCtx
)
{
    DECODER_sContext * psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext *  psDecCoreCtx;
    DECODER_sDecPict * psDecPict = NULL;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if(psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);

    if (IMG_SUCCESS == HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict))
    {
        return IMG_FALSE;
    }

    return IMG_TRUE;
}


/*!
******************************************************************************

 @Function              DECODER_CoreServiceInterrupt

******************************************************************************/
IMG_RESULT
DECODER_CoreServiceInterrupt(
    IMG_HANDLE      hDecServiceInt,
    IMG_VOID     ** ppvStrUserIntData
)
{
    IMG_RESULT ui32Result = IMG_SUCCESS;
    IMG_BOOL    bPictureDone = IMG_TRUE;

    // Picture processing done interrupt.
    // XXX: Add check for picture done interrupt.
    if (bPictureDone)
    {
        IMG_BOOL bHeadOfQueue = IMG_TRUE;
        DECODER_sCoreContext * psDecCoreCtx;
        DECODER_sStrContext * psDecStrCtx;
        DECODER_sDecStrUnit * psDecStrUnit;
        DECODER_sDecPict * psDecPict = IMG_NULL;
        IMG_BOOL bPictStart = IMG_FALSE;
        VDECDD_eStrUnitType eStrUnitType;

        psDecCoreCtx = (DECODER_sCoreContext*)hDecServiceInt;

        /* Check input parameters. */
        IMG_ASSERT(psDecCoreCtx);
        if(psDecCoreCtx == IMG_NULL)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
                   "Invalid parameters!");
            return IMG_ERROR_INVALID_PARAMETERS;
        }

        // Peek the head of the core decode queue to determine whether this
        // picture needs to be processed.
        if(psDecCoreCtx->sCoreProps.ui32NumPixelPipes != 1)
        {
            //multipipe: picture may have been decoded out of order
            ui32Result = HWCTRL_GetDecodedPicture(psDecCoreCtx->hHwCtx, &psDecPict);
        }
        else
        {
            //one pipe: decoded picture always at head
            ui32Result = HWCTRL_PeekHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict);
        }
        IMG_ASSERT(ui32Result == IMG_SUCCESS && psDecPict);
        if (ui32Result != IMG_SUCCESS || psDecPict == IMG_NULL)
        {
            return IMG_SUCCESS;
        }

        // Try and locate the stream context in the list of active streams.
        IMG_ASSERT(psDecCoreCtx->psDecCtx);
        psDecStrCtx = LST_first(&psDecCoreCtx->psDecCtx->sStreamList);
        while (psDecStrCtx)
        {
            if (psDecStrCtx == psDecPict->psDecStrCtx)
            {
                break;
            }

            psDecStrCtx = LST_next(psDecStrCtx);
        }

        // If the stream is not in the list of active streams then it must have been destroyed.
        // This interrupt should be ignored.
        if (psDecStrCtx != psDecPict->psDecStrCtx)
        {
            return IMG_SUCCESS;
        }

        // Retrieve the picture from the head of the core decode queue
        // primarily to obtain the correct stream context.
        if(psDecCoreCtx->sCoreProps.ui32NumPixelPipes != 1)
        {
            //multipipe: picture may have been decoded out of order
            ui32Result = HWCTRL_RemoveFromPicList(psDecCoreCtx->hHwCtx, psDecPict);
        }
        else
        {
            //one pipe: decoded picture always at head
            ui32Result = HWCTRL_RemoveHeadPicList(psDecCoreCtx->hHwCtx, &psDecPict);
        }
        IMG_ASSERT(psDecPict);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS || psDecPict == IMG_NULL)
        {
            return IMG_ERROR_GENERIC_FAILURE;
        }

        // Store the stream context of the picture that has been decoded.
        psDecStrCtx = psDecPict->psDecStrCtx;
        IMG_ASSERT(IMG_NULL != psDecStrCtx);

        if(IMG_NULL != psDecStrCtx)
        {
            *ppvStrUserIntData = psDecStrCtx->pvUserIntData;

            // Peek the first stream unit and validate against core queue to ensure that
            // this really is the next picture for the stream.
            psDecStrUnit = LST_first(&psDecStrCtx->sPendStrUnitList);
            if (IMG_NULL != psDecStrUnit)
            {
                if(psDecStrUnit->psDecPict != psDecPict)
                {
                    //DECODER_sDecStrUnit * psPrevDecStrUnit = IMG_NULL;
                    bHeadOfQueue = IMG_FALSE;

                    // For pictures to be decoded out-of-order there must be
                    // more than one decoder core.
                    IMG_ASSERT(psDecStrCtx->psDecCtx->ui32NumPipes > 1);
                    while (psDecStrUnit)
                    {
                        psDecStrUnit = LST_next(psDecStrUnit);
                        if (psDecStrUnit->psDecPict == psDecPict)
                        {
                            break;
                        }
                    }
                }
                IMG_ASSERT(psDecStrUnit != IMG_NULL);
                if(psDecStrUnit == IMG_NULL)
                {
                    return IMG_ERROR_FATAL;
                }
                IMG_ASSERT(psDecStrUnit->psDecPict == psDecPict);
                IMG_ASSERT(psDecStrUnit->psStrUnit->eStrUnitType == VDECDD_STRUNIT_PICTURE_START);
            }

            // Process all units from the pending stream list until the next picture start.
            while (psDecStrUnit && !bPictStart)
            {
                // Actually remove the unit now from the pending stream list.
                LST_remove(&psDecStrCtx->sPendStrUnitList, psDecStrUnit);

                if(psDecStrUnit->psStrUnit)
                {
                    eStrUnitType = psDecStrUnit->psStrUnit->eStrUnitType;

                    if (eStrUnitType == VDECDD_STRUNIT_PICTURE_START)
                    {
                        VDECDD_sPicture * psPicture;
                        DECODER_sStrContext * psDecStrCtx = psDecPict->psDecStrCtx;

                        psDecStrCtx->sDecStrStatus.ui32NumPictDecoding--;
                        psDecStrCtx->sDecStrStatus.ui32TotalPictDecoded++;

                        ui32Result = IDGEN_GetHandle(psDecStrCtx->hPictIdGen,
                            GET_STREAM_PICTURE_ID(psDecStrUnit->psDecPict->ui32TransactionId),
                            (IMG_VOID **)&psPicture);
                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS ||
                            psPicture == IMG_NULL)
                        {
                            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                                   "[USERSID=0x%08X] Failed to find picture from ID",
                                   psDecStrCtx->sConfig.ui32UserStrId);
                            return IMG_ERROR_INVALID_ID;
                        }

                        IMG_ASSERT(psPicture == psDecStrUnit->psStrUnit->pvDdPictData);

                        // Hold a reference to the last context on the BE
                        if (psDecStrCtx->psLastBePictDecRes &&
                            HAS_X_PASSED_Y(psPicture->ui32PictId, GET_STREAM_PICTURE_ID(psDecStrCtx->psLastBePictDecRes->ui32TransactionId), 1 << FWIF_NUMBITS_STREAM_PICTURE_ID))
                        {
                            // Return previous last FW context.
                            RESOURCE_ItemReturn(&psDecStrCtx->psLastBePictDecRes->ui32RefCount);
#ifndef IMG_KERNEL_MODULE
                            if(RESOURCE_ItemIsAvailable(&psDecStrCtx->psLastBePictDecRes->ui32RefCount))
                            {
                                RESOURCE_ListRemove(&psDecStrCtx->sDecResList, psDecStrCtx->psLastBePictDecRes);
                                RESOURCE_ListAdd(&psDecStrCtx->sDecResList, psDecStrCtx->psLastBePictDecRes, 0, &psDecStrCtx->psLastBePictDecRes->ui32RefCount);
                            }
#endif
                        }

                        if (psDecStrCtx->psLastBePictDecRes == IMG_NULL ||
                            (psDecStrCtx->psLastBePictDecRes &&
                            HAS_X_PASSED_Y(psPicture->ui32PictId, GET_STREAM_PICTURE_ID(psDecStrCtx->psLastBePictDecRes->ui32TransactionId), 1 << FWIF_NUMBITS_STREAM_PICTURE_ID)))
                        {
                            // Hold onto last FW context.
                            psDecStrCtx->psLastBePictDecRes = psDecPict->psCurPictDecRes;
                            RESOURCE_ItemUse(&psDecStrCtx->psLastBePictDecRes->ui32RefCount);
                        }

#ifdef VDEC_USE_PVDEC_SEC
                        /* Update list of pictures in DPF which would be sent for display
                         * in case of stream flush. */
                        if (psDecStrCtx->sConfig.bSecureStream)
                        {
                            VDECDDUTILS_CopyDPBState(&psDecStrCtx->sLastDPBState,
                                                     &psDecPict->sDPBState);
                        }
#endif /* VDEC_USE_PVDEC_SEC */

                        RESOURCE_ItemReturn(&psDecPict->psCurPictDecRes->ui32RefCount);
#ifndef IMG_KERNEL_MODULE
                        if(RESOURCE_ItemIsAvailable(&psDecPict->psCurPictDecRes->ui32RefCount))
                        {
                            RESOURCE_ListRemove(&psDecStrCtx->sDecResList, psDecPict->psCurPictDecRes);
                            RESOURCE_ListAdd(&psDecStrCtx->sDecResList, psDecPict->psCurPictDecRes, 0, &psDecPict->psCurPictDecRes->ui32RefCount);
                        }
#endif
                        DEBUG_REPORT(REPORT_MODULE_DECODER,
                            "[USERSID=0x%08X] [TID=0x%08X] DECODED",
                            psDecStrCtx->sConfig.ui32UserStrId,
                            psDecPict->ui32TransactionId);

#if defined(FW_PERFORMANCE_LOGGING) && !defined(VDEC_USE_PVDEC)
                        if(psDecCoreCtx->psDecCtx &&
                           psDecCoreCtx->psDecCtx->psDevConfig &&
                           psDecCoreCtx->psDecCtx->psDevConfig->bPerformanceLog)
                        {
                            //Dumping out Deva style XPL file here
                            ui32Result = decoder_SavePerfData(psDecStrUnit);
                            IMG_ASSERT(ui32Result == IMG_SUCCESS);
                            if (ui32Result != IMG_SUCCESS)
                            {
                                return ui32Result;
                            }
                        }
#endif /* defined(FW_PERFORMANCE_LOGGING) && !defined(VDEC_USE_PVDEC) */

                        ui32Result = decoder_PictureDecoded(psDecStrCtx,
                                                            psDecCoreCtx,
                                                            psPicture,
                                                            psDecPict,
                                                            psDecPict->psPictHdrInfo,
                                                            psDecStrUnit->psStrUnit);

                        IMG_ASSERT(ui32Result == IMG_SUCCESS);
                        if (ui32Result != IMG_SUCCESS)
                        {
                            return ui32Result;
                        }

                        RESOURCE_PictureDetach(&psDecCoreCtx->hResources,
                                               psDecPict,
                                               psDecStrCtx->sConfig.bSecureStream);

                        // Free the segments from the decode picture
                        decoder_CleanBitStrSegments(&psDecPict->sDecPictSegList);
                        // Free decoder picture
                        IMG_FREE(psDecPict);
                        psDecPict = IMG_NULL;

                        bPictStart = (!bHeadOfQueue) ? IMG_TRUE : IMG_FALSE;
                    }
                    else
                    {
                        IMG_ASSERT(psDecStrUnit->psDecPict == IMG_NULL);
                    }

                    // Report that this unit has been processed.
                    ui32Result = psDecStrCtx->psDecCtx->pfnCompCallback(
                        DECODER_CB_UNIT_PROCESSED,
                        psDecStrCtx->psDecCtx->pvCompInitUserData,
                        IMG_NULL,
                        psDecStrCtx->pvUserData,
                        psDecStrUnit->psStrUnit);
                    IMG_ASSERT(ui32Result == IMG_SUCCESS);
                    if (ui32Result != IMG_SUCCESS)
                    {
                        return ui32Result;
                    }

                    // Destroy the Decoder stream unit wrapper.
                    IMG_FREE(psDecStrUnit);
                    psDecStrUnit = IMG_NULL;

                    // Peek at the next stream unit.
                    psDecStrUnit = LST_first(&psDecStrCtx->sPendStrUnitList);
                    if (psDecStrUnit)
                    {
                        bPictStart = (psDecStrUnit->psStrUnit->eStrUnitType == VDECDD_STRUNIT_PICTURE_START && psDecStrUnit->psDecPict != psDecPict);
                    }
                }
            }
        }
        else
        {
            ui32Result = IMG_ERROR_FATAL;
        }
    }

    return ui32Result;
}


/*!
******************************************************************************

 @Function              decoder_MmuDeviceCallback

 @Description

 This function handles the VDECDD MMU device callback event.

 @Input     eEvent   : VDECDD MMU device callback event.

 @Input     pvDecCtx : Pointer to Decoder context.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

 ******************************************************************************/
static IMG_RESULT
decoder_MmuDeviceCallback(
    VDECDDMMU_eEvent    eEvent,
    IMG_VOID          * pvDecCtx
)
{
    DECODER_sContext      * psDecCtx = pvDecCtx;
    DECODER_sCoreContext  * psDecCoreCtx;
    IMG_RESULT              ui32Result;

    switch (eEvent)
    {
    case VDECDDMMU_EVENT_FLUSH_CACHE:
        // Cache will be flushed whenever a PTD or PTE is validated/invalidated (normally at start-up/shutdown).
        // Since memory could have been "seen" by multiple cores (frames scheduled on different cores) the cache
        // of all MMUs must be invalidated. Loop over all cores and flush MMU cache.
        IMG_ASSERT(psDecCtx);
        psDecCoreCtx = psDecCtx->psDecCoreCtx;
        if(psDecCoreCtx)
        {
            ui32Result = HWCTRL_CoreFlushMmuCache(psDecCoreCtx->hHwCtx);
            IMG_ASSERT(ui32Result == IMG_SUCCESS);
            if (ui32Result != IMG_SUCCESS)
            {
                return ui32Result;
            }
        }
        break;

    default:
        break;
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              decoder_CoreEnumerate

 @Description

 This function enumerates a decoder core and returns its handle.

 Usage: before calls to other DECODE_Core or DECODE_Stream functions.

 @Input     psDecCtx : Pointer to Decoder context.

 @Input     psDevConfig : Device configuration.

 @Return    IMG_RESULT : This function returns either IMG_SUCCESS or an
                         error code.

 ******************************************************************************/
static IMG_RESULT
decoder_CoreEnumerate(
    DECODER_sContext          * psDecCtx,
    const VDECDD_sDdDevConfig * psDevConfig,
    IMG_UINT32                * pui32NumPipes
)
{
    DECODER_sCoreContext  * psDecCoreCtx;
    IMG_UINT32              ui32Result;
    VXDIO_sMemPool          sInsecureMemPool;
    IMG_UINT32              ui32PtdAlignment = DEV_MMU_PAGE_ALIGNMENT;

    /* Create the core. */
    VDEC_MALLOC(psDecCoreCtx);
    if (psDecCoreCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "Failed to allocate memory for Decoder core context");
        ui32Result = IMG_ERROR_OUT_OF_MEMORY;
        goto error;
    }
    VDEC_BZERO(psDecCoreCtx);

    psDecCoreCtx->psDecCtx = psDecCtx;

    // Initialise the hwctrl block here
    ui32Result = HWCTRL_Initialise(psDecCoreCtx,
                                   psDecCtx->pvCompInitUserData,
                                   psDevConfig,
                                   &psDecCoreCtx->sCoreProps,
                                   &psDecCoreCtx->sRendec,
                                   &psDecCoreCtx->hHwCtx);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    decoder_SetFeatureFlags(&psDecCoreCtx->sCoreProps, &psDecCoreCtx->ui32CoreFeatures, psDecCoreCtx->aui32PipeFeatures);

    sInsecureMemPool.eMemPoolId = psDecCtx->eInsecurePool;
    sInsecureMemPool.eMemPoolType = VXDIO_MEMPOOL_INSECURE;

    // Perform device setup for master core.
    {
#ifndef IMG_KERNEL_MODULE
#define MAX_BYTES 1024
        IMG_CHAR acComment[MAX_BYTES];
        IMG_INT32 i32NumBytes;

        i32NumBytes = snprintf(acComment,
           MAX_BYTES,
           "INFO:\t[%s]\n"
           "-- INFO:\tAPM:%s\n"
           "-- INFO:\tSlots:%u\n"
           "-- INFO:\tLevel:%s\n"
           "-- INFO:\tDWR:%ums\n"
           "-- INFO:\tFEWDT:%u\n"
           "-- INFO:\tBEWDT:%u\n"
           "-- INFO:\tPSRWDT:%u\n"
           "-- INFO:\tRendec:%u bytes\n"
           "-- INFO:\tPerfLog:%s\n"
           "-- INFO:\tTiling:%s",
           "Device Configuration",
           (psDecCtx->psDevConfig->bAPM) ? "y" : "n",
           psDecCtx->psDevConfig->ui32NumSlotsPerPipe,
           apszDecodeLevels[psDecCtx->psDevConfig->eDecodeLevel],
           psDecCtx->psDevConfig->ui32DwrPeriod,
           psDecCtx->psDevConfig->ui32FeWdtPeriod,
           psDecCtx->psDevConfig->ui32BeWdtPeriod,
           psDecCtx->psDevConfig->ui32PSRWdtPeriod,
           psDecCtx->psDevConfig->ui32RendecSize,
           (psDecCtx->psDevConfig->bPerformanceLog) ? "y" : "n",
           (psDecCtx->psDevConfig->eTileScheme == VDEC_TS0_256x16) ? "0" :
           (psDecCtx->psDevConfig->eTileScheme == VDEC_TS1_512x8) ? "1" : "None");

        IMG_ASSERT(i32NumBytes > 0 && i32NumBytes < MAX_BYTES);

        // Write device configuration to master core.
        ui32Result = HWCTRL_PdumpComment(psDecCoreCtx->hHwCtx, acComment);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
#endif
        if (pui32NumPipes)
        {
            *pui32NumPipes = psDecCoreCtx->sCoreProps.ui32NumPixelPipes;
        }

        /* DEVA PVDEC FW requires PTD to be 64k aligned. */
        ui32PtdAlignment = 0x10000;
        /* Create a device MMU context. */
        ui32Result = MMU_DeviceCreate(psDecCoreCtx->sCoreProps.eMmuType,
                                      psDevConfig->eTileScheme,
                                      decoder_MmuDeviceCallback,
                                      sInsecureMemPool,
                                      psDecCtx,
                                      ui32PtdAlignment,
                                      &psDecCtx->hMmuDevHandle);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        // Allocate memory to store PTD base addresses.
        ui32Result = MMU_DeviceMalloc(psDecCtx->hMmuDevHandle,
                                      MMU_HEAP_DEVICE_BUFFERS,
                                      sInsecureMemPool,    //MMU Page table directory base address container always in insecure memory
                                      (MAX_CONCURRENT_STREAMS + 1) * sizeof(IMG_UINT32),    // Stream Ids start from 1. 0 is reserved for device PTD.
                                      DEV_MMU_PAGE_ALIGNMENT,
                                      &psDecCtx->sPtdBufInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        // Obtain device PTD physical address.
        ui32Result = MMU_DeviceWritePtdToMem(psDecCtx->hMmuDevHandle,
                                             &psDecCtx->sPtdBufInfo,
                                             &psDecCtx->sPtdInfo);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
    }

#ifndef VDEC_USE_PVDEC_SEC
#ifdef USE_REAL_FW
    /* Obtain firmware */
    ui32Result = decoder_GetFirmware(psDecCtx->hMmuDevHandle,
                                     psDecCtx->hDevHandle,
                                     sInsecureMemPool,
                                     IMG_FALSE, // always false for the old flow
                                     &psDecCtx->sFirmware);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
#endif /* USE_REAL_FW */

#ifndef IMG_KERNEL_MODULE
    ui32Result = HWCTRL_PdumpComment(psDecCoreCtx->hHwCtx, "<+ResourceCreate>");
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
#endif

    // Create core resources.
    ui32Result = RESOURCE_Create(psDecCtx->hMmuDevHandle,
                                 &psDecCoreCtx->sCoreProps,
                                 &psDecCoreCtx->sRendec,
                                 psDecCtx->psDevConfig->ui32NumSlotsPerPipe * psDecCtx->ui32NumPipes,
                                 sInsecureMemPool,
                                 sInsecureMemPool,
                                 psDecCoreCtx->asRendecBufInfo,
                                 &psDecCoreCtx->hResources,
                                 IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

#ifndef IMG_KERNEL_MODULE
    ui32Result = HWCTRL_PdumpComment(psDecCoreCtx->hHwCtx, "<-ResourceCreate>");
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
#endif

    ui32Result = HWCTRL_Configure(psDecCoreCtx->hHwCtx,
                                  &psDecCtx->sFirmware,
                                  psDecCoreCtx->asRendecBufInfo,
                                  &psDecCtx->sPtdBufInfo,
                                  &psDecCtx->sPtdInfo);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }
#endif /* !VDEC_USE_PVDEC_SEC */
    psDecCoreCtx->bEnumerated = IMG_TRUE;

    psDecCoreCtx->bAPM = IMG_FALSE;
    // make sure apm-ppm flags are reset
    DMANKM_ResetPowerManagementFlag(psDecCoreCtx->psDecCtx->hDevHandle);

    psDecCtx->psDecCoreCtx = psDecCoreCtx;

    return IMG_SUCCESS;

error:
    if (psDecCoreCtx)
    {
        IMG_UINT32 ui32DeinitResult = IMG_SUCCESS;

#ifndef VDEC_USE_PVDEC_SEC
#ifdef USE_REAL_FW
            /* Destroy firmware. */
        ui32DeinitResult = decoder_DestroyFirmware(&psDecCtx->sFirmware,
                                                   psDecCoreCtx->psDecCtx->hDevHandle);
            if (ui32DeinitResult != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "decoder_DestroyFirmware() failed to tidy-up after error");
            }
#endif /* USE_REAL_FW */

        if (psDecCoreCtx->hResources)
        {
            ui32DeinitResult = RESOURCES_Destroy(psDecCoreCtx->hResources);
            if (ui32DeinitResult != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "RESOURCES_Destroy() failed to tidy-up after error");
            }
            psDecCoreCtx->hResources = IMG_NULL;
        }
#endif

        /* Destroy a device MMU context. */
        if (psDecCtx->hMmuDevHandle)
        {
            ui32DeinitResult = MMU_DeviceDestroy(psDecCtx->hMmuDevHandle);
            IMG_ASSERT(ui32DeinitResult == IMG_SUCCESS);
            if (ui32DeinitResult != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "MMU_DeviceDestroy() failed to tidy-up after error");
            }
        }

        IMG_FREE(psDecCoreCtx);
        psDecCoreCtx = IMG_NULL;
    }

    return ui32Result;
}



/*!
******************************************************************************

 @Function              DECODER_HandleInterrupt

******************************************************************************/
IMG_RESULT DECODER_HandleInterrupt(
    VDECDD_sDdDevContext *        psDdDevContext
)
{
    DECODER_sContext * psDecCtx;
    DECODER_sCoreContext * psDecCoreCtx;
    IMG_UINT32 ui32Result;

    /* Check input parameters. */
    IMG_ASSERT(psDdDevContext);
    if (psDdDevContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    psDecCtx = (DECODER_sContext *)psDdDevContext->hDecoderContext;
    IMG_ASSERT(psDecCtx);
    // Determine which core this interrupt is for.
    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
    HWCTRL_HandleInterrupt(psDecCoreCtx->hHwCtx,psDdDevContext);

    return IMG_SUCCESS;

error:
    return ui32Result;
}



/*!
******************************************************************************

 @Function              DECODER_GetLoad

******************************************************************************/
IMG_RESULT DECODER_GetLoad(
    IMG_HANDLE          hDecStrCtx,
    IMG_UINT32          ui32CoreFeatures,
    IMG_UINT32        * pui32AvailLoad,
    IMG_UINT32        * pui32MinLoad
)
{
    DECODER_sStrContext * psDecStrCtx = decoder_StreamGetContext(hDecStrCtx);
    IMG_RESULT          ui32Result;

    /* Check input parameters. */
    IMG_ASSERT(psDecStrCtx);
    if (psDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    ui32Result = decoder_GetPipeAndLoad(psDecStrCtx,
                                        ui32CoreFeatures,
                                        pui32AvailLoad,
                                        pui32MinLoad,
                                        IMG_NULL);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              DECODER_IsSupportedFeature

******************************************************************************/
IMG_RESULT DECODER_IsSupportedFeature(
    IMG_HANDLE          hDecCtx,
    VDEC_eFeature       eFeature,
    IMG_BOOL          * pbSupported
)
{
    DECODER_sContext * psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext * psDecCoreCtx;
    IMG_BOOL bSupported = IMG_FALSE;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    if (psDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    /* Ensure that Decoder component is initialised. */
    IMG_ASSERT(psDecCtx->bInitialised);

    /* Loop over all cores checking for support. */
    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
        // Determine whether the required core attribute
        // is present to support requested feature
        switch(eFeature)
        {
        case VDEC_FEATURE_H264:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_H264) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_MPEG2:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_MPEG2) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_MPEG4:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_MPEG4) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_VC1:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_VC1) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_AVS:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_AVS) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_REAL:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_REAL) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_JPEG:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_JPEG) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_VP6:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_VP6) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_VP8:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_VP8) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_HEVC:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_HEVC) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_HD_DECODE:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_HD_DECODE) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_ROTATION:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_ROTATION) ? IMG_TRUE : IMG_FALSE;
            break;

        case VDEC_FEATURE_SCALING:
                    bSupported |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_SCALING) ? IMG_TRUE : IMG_FALSE;
            break;

        default:
            IMG_ASSERT(IMG_FALSE);
            break;
        }

    *pbSupported = bSupported;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_SupportedFeatures

******************************************************************************/
IMG_RESULT DECODER_SupportedFeatures(
    IMG_HANDLE          hDecCtx,
    VDEC_sFeatures    * psFeatures
)
{
    DECODER_sContext * psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext * psDecCoreCtx;

    /* Check input parameters. */
    IMG_ASSERT(psDecCtx);
    IMG_ASSERT(psFeatures);
    if (psDecCtx == IMG_NULL ||
        psFeatures == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    /* Ensure that Decoder component is initialised. */
    IMG_ASSERT(psDecCtx->bInitialised);

    /* Loop over all cores checking for support. */
    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);

        // Determine whether the required core attribute
        // is present to support requested feature
        psFeatures->bFeatureH264 |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_H264) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureMPEG2 |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_MPEG2) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureMPEG4 |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_MPEG4) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureVC1 |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_VC1) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureAVS |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_AVS) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureREAL |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_REAL) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureJPEG |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_JPEG) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureVP6 |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_VP6) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureVP8 |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_VP8) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureHD |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_HD_DECODE) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureRotation |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_ROTATION) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureScaling |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_SCALING) ? IMG_TRUE : IMG_FALSE;
        psFeatures->bFeatureHEVC |= (psDecCoreCtx->ui32CoreFeatures & VDECDD_COREFEATURE_HEVC) ? IMG_TRUE : IMG_FALSE;

    return IMG_SUCCESS;
}


static const VDECDD_sDdDevConfig sDefaultDeviceConfig =
{
    VDECDD_DECODELEVEL_MAX,      /* eDecodeLevel;            */
    CORE_NUM_DECODE_SLOTS,       /* ui32NumSlotsPerPipe;     */
    IMG_FALSE,
    4*1024*1024,                 /* ui32RendecSize;          */
    IMG_FALSE,                   /* bSecureMemoryAvailable;  */
    IMG_FALSE,                   /* bPerformanceLog;         */
    CORE_DEFAULT_DWR_PERIOD,     /* ui32DwrPeriod;           */
    CORE_DEFAULT_DWR_JPEG_RETRY, /* ui32DwrJPEGRetry;        */
    CORE_DEFAULT_FE_TIMEOUT,     /* ui32FeWdtPeriod;         */
    CORE_DEFAULT_BE_TIMEOUT,     /* ui32BeWdtPeriod;         */
    CORE_DEFAULT_PSR_TIMEOUT,    /* ui32PSRWdtPeriod;        */
    IMG_FALSE,                   /* bCoreDump;               */
    VDEC_TS0_256x16,             /* eTileScheme;             */
    IMG_FALSE,                   /* bCSim;                   */
    IMG_FALSE,                   /* bPdumpAndRes;            */
};



/*!
******************************************************************************

 @Function              decoder_SetDeviceConfig

******************************************************************************/
static IMG_RESULT decoder_SetDeviceConfig(
    const VDECDD_sDdDevConfig   ** ppsDdDevConfig
)
{
    VDECDD_sDdDevConfig   * psDevConfig;
    DBGOPT_sValue           sVal;
    DBGOPT_eType            eType;

    IMG_ASSERT(ppsDdDevConfig);

    // Allocate device configuration structure.
    VDEC_MALLOC(psDevConfig);
    IMG_ASSERT(psDevConfig);
    if (psDevConfig == IMG_NULL)
    {
        REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
            "Failed to allocate memory for device configuration");
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psDevConfig);

    // Set the default device configuration.
    *psDevConfig = sDefaultDeviceConfig;

#ifdef VDEC_TARGET_CEDARVIEW
    psDevConfig->bAPM = IMG_FALSE;
#endif

#ifdef FW_PERFORMANCE_LOGGING
    // Override with values provided to DBGOPT
    if (DBGOPTKM_GetSize("PerformanceLog", DBGOPT_TYPE_BOOL) &&
        DBGOPTKM_Get("PerformanceLog", IMG_NULL, &sVal))
    {
        psDevConfig->bPerformanceLog = sVal.bVal;
    }
#endif

    // Override with values provided to DBGOPT
    if (DBGOPTKM_GetSize("APM", DBGOPT_TYPE_BOOL) &&
        DBGOPTKM_Get("APM", IMG_NULL, &sVal))
    {
        psDevConfig->bAPM = sVal.bVal;
    }
    if (DBGOPTKM_GetSize("CoreDecodeSlots", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("CoreDecodeSlots", IMG_NULL, &sVal))
    {
        IMG_ASSERT(sVal.ui32Val <= psDevConfig->ui32NumSlotsPerPipe);
        if (sVal.ui32Val < psDevConfig->ui32NumSlotsPerPipe)
        {
            psDevConfig->ui32NumSlotsPerPipe = sVal.ui32Val;
        }
    }
    if (DBGOPTKM_GetSize("DecodeLevel", DBGOPT_TYPE_STRING) &&
        DBGOPTKM_Get("DecodeLevel", &eType, &sVal))
    {
        if (strncmp(sVal.pszVal, "scheduler", 32) == 0)
        {
            psDevConfig->eDecodeLevel = VDECDD_DECODELEVEL_SCHEDULER;
        }
        else if (strncmp(sVal.pszVal, "decoder", 32) == 0)
        {
            psDevConfig->eDecodeLevel = VDECDD_DECODELEVEL_DECODER;
        }
        else if (strncmp(sVal.pszVal, "fw", 32) == 0)
        {
            psDevConfig->eDecodeLevel = VDECDD_DECODELEVEL_FWHDRPARSING;
        }
        else if (strncmp(sVal.pszVal, "fehw", 32) == 0)
        {
            psDevConfig->eDecodeLevel = VDECDD_DECODELEVEL_FEHWDECODING;
        }
        else if (strncmp(sVal.pszVal, "all", 32) == 0)
        {
            psDevConfig->eDecodeLevel = VDECDD_DECODELEVEL_MAX;
        }
        else
        {
            REPORT(REPORT_MODULE_CORE, REPORT_ERR,
                   "Invalid decode level [\"%s\"] using default \"all\"",
                   sVal.pszVal);
        }

#ifndef VDEC_MSVDX_HARDWARE
        // Without hardware clip the decode level to decoder.
        if (psDevConfig->eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
        {
            psDevConfig->eDecodeLevel = VDECDD_DECODELEVEL_DECODER;
        }
#endif
    }
    if (DBGOPTKM_GetSize("DWRPeriod", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("DWRPeriod", IMG_NULL, &sVal))
    {
        psDevConfig->ui32DwrPeriod = sVal.ui32Val;
    }
    if (DBGOPTKM_GetSize("CoreDump", DBGOPT_TYPE_BOOL) &&
        DBGOPTKM_Get("CoreDump", IMG_NULL, &sVal))
    {
        psDevConfig->bCoreDump = sVal.bVal;
    }
    if (DBGOPTKM_GetSize("DWRJPEGRetry", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("DWRJPEGRetry", IMG_NULL, &sVal))
    {
        psDevConfig->ui32DwrJPEGRetry = sVal.ui32Val;
    }
    if (DBGOPTKM_GetSize("RendecSize", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("RendecSize", IMG_NULL, &sVal))
    {
        psDevConfig->ui32RendecSize = sVal.ui32Val;
    }
    if (DBGOPTKM_GetSize("FeWdtPeriod", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("FeWdtPeriod", IMG_NULL, &sVal))
    {
        psDevConfig->ui32FeWdtPeriod = sVal.ui32Val;
    }
    if (DBGOPTKM_GetSize("PSRWdtPeriod", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("PSRWdtPeriod", IMG_NULL, &sVal))
    {
        psDevConfig->ui32PSRWdtPeriod = sVal.ui32Val;
    }
    if (DBGOPTKM_GetSize("BeWdtPeriod", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("BeWdtPeriod", IMG_NULL, &sVal))
    {
        psDevConfig->ui32BeWdtPeriod = sVal.ui32Val;
    }
    if (DBGOPTKM_GetSize("CSim", DBGOPT_TYPE_BOOL) &&
        DBGOPTKM_Get("CSim", IMG_NULL, &sVal))
    {
        psDevConfig->bCSim = sVal.bVal;
    }

    if (DBGOPTKM_GetSize("MMUTilingScheme", DBGOPT_TYPE_NUMBER) &&
        DBGOPTKM_Get("MMUTilingScheme", IMG_NULL, &sVal))
    {
        switch(sVal.ui32Val)
        {
        case 0:
            psDevConfig->eTileScheme = VDEC_TS0_256x16;
            break;
        case 1:
            psDevConfig->eTileScheme = VDEC_TS1_512x8;
            break;
        default:
            IMG_ASSERT(IMG_FALSE);
        }
    }

#ifndef IMG_KERNEL_MODULE
    if (DBGOPTKM_GetSize("PdumpRes", DBGOPT_TYPE_BOOL) &&
        DBGOPTKM_Get("PdumpRes", IMG_NULL, &sVal))
    {
        psDevConfig->bPdumpAndRes = sVal.bVal;
    }
#endif

    *ppsDdDevConfig =  psDevConfig;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              DECODER_Initialise

******************************************************************************/
IMG_RESULT DECODER_Initialise(
    IMG_VOID *              pvInitUserData,
    VDECDD_pfnCompCallback  pfnCallback,
    SYS_eMemPool            eInsecurePool,
    SYS_eMemPool            eSecureMemPool,
    VDECDD_sDdDevConfig   * psDdDevConfig,
    IMG_UINT32            * pui32NumPipes,
    IMG_HANDLE *            phDecCtx
)
{
    DECODER_sContext  * psDecCtx = (DECODER_sContext*)*phDecCtx;
    IMG_RESULT          ui32Result;

    if (psDecCtx == IMG_NULL)
    {
        VDEC_MALLOC(psDecCtx);
        IMG_ASSERT(psDecCtx);
        if (psDecCtx == IMG_NULL)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                "Failed to allocate memory for Decoder context");
            return IMG_ERROR_OUT_OF_MEMORY;
        }
        VDEC_BZERO(psDecCtx);

        *phDecCtx = psDecCtx;
    }

    /*! Determine which standards are supported. */
    VDEC_BZERO(&psDecCtx->abSupportedStandards);
#ifdef HAS_AVS
    psDecCtx->abSupportedStandards[VDEC_STD_AVS]      = IMG_TRUE;
#endif /* HAS_AVS */
#ifdef HAS_H264
    psDecCtx->abSupportedStandards[VDEC_STD_H264]     = IMG_TRUE;
#endif /* HAS_H264 */
#ifdef HAS_MPEG2
    psDecCtx->abSupportedStandards[VDEC_STD_MPEG2]    = IMG_TRUE;
#endif /* HAS_MPEG2 */
#ifdef HAS_MPEG4
    psDecCtx->abSupportedStandards[VDEC_STD_MPEG4]    = IMG_TRUE;
    psDecCtx->abSupportedStandards[VDEC_STD_H263]     = IMG_TRUE;
    psDecCtx->abSupportedStandards[VDEC_STD_SORENSON] = IMG_TRUE;
#endif /* HAS_MPEG4 */
#ifdef HAS_VC1
    psDecCtx->abSupportedStandards[VDEC_STD_VC1]      = IMG_TRUE;
#endif /* HAS_VC1 */
#ifdef HAS_AVS
    psDecCtx->abSupportedStandards[VDEC_STD_AVS]      = IMG_TRUE;
#endif /* HAS_VC1 */
#ifdef HAS_REAL
    psDecCtx->abSupportedStandards[VDEC_STD_REAL]     = IMG_TRUE;
#endif /* HAS_REAL */
#ifdef HAS_JPEG
    psDecCtx->abSupportedStandards[VDEC_STD_JPEG]     = IMG_TRUE;
#endif /* HAS_JPEG */
#ifdef HAS_VP6
    psDecCtx->abSupportedStandards[VDEC_STD_VP6]      = IMG_TRUE;
#endif /* HAS_VP6 */
#ifdef HAS_VP8
    psDecCtx->abSupportedStandards[VDEC_STD_VP8]      = IMG_TRUE;
#endif /* HAS_VP8 */
#ifdef HAS_VC1
    psDecCtx->abSupportedStandards[VDEC_STD_VC1]      = IMG_TRUE;
#endif /* HAS_VC1 */
#ifdef HAS_HEVC
    psDecCtx->abSupportedStandards[VDEC_STD_HEVC]     = IMG_TRUE;
#endif /* HAS_HEVC */

    if (!psDecCtx->bInitialised)
    {
        /* Check input parameters. */
        IMG_ASSERT(pfnCallback);

        /* Store the input data. */
        psDecCtx->pfnCompCallback = pfnCallback;
        psDecCtx->pvCompInitUserData = pvInitUserData;
        psDecCtx->hDevHandle = ((VDECDD_sDdDevContext *)pvInitUserData)->hDevHandle;

        /* Initialise the context lists. */
        LST_init(&psDecCtx->sStreamList);

        ui32Result = IDGEN_CreateContext(MAX_CONCURRENT_STREAMS, 64, IMG_FALSE, &psDecCtx->hStrIdGen);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        /* Make sure POOL API is initialised */
        ui32Result = POOL_Initialise();
        if(ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        ui32Result = decoder_SetDeviceConfig(&psDecCtx->psDevConfig);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        psDecCtx->eInsecurePool = eInsecurePool;
        psDecCtx->eSecurePool =
            psDecCtx->psDevConfig->bSecureMemoryAvailable ? eSecureMemPool : eInsecurePool;

        /* Enumerate the master core. */
        ui32Result = decoder_CoreEnumerate(psDecCtx,
                                           psDecCtx->psDevConfig,
                                           &psDecCtx->ui32NumPipes);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

#if defined(FW_PERFORMANCE_LOGGING) && !defined(VDEC_USE_PVDEC)
#ifndef SYSBRG_BRIDGING
        ui32Result = PERFLOG_Initialise();
        if(ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
                    "Failed to initialise performance log");
        }
#endif /* not SYSBRG_BRIDGING */

        ui32Result = PERFLOG_Open();
        if(ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_DECODER, REPORT_WARNING,
                    "Failed to open performance log file");
        }
#endif /* defined(FW_PERFORMANCE_LOGGING) && !defined(VDEC_USE_PVDEC) */

        if (psDdDevConfig)
        {
            *psDdDevConfig = *psDecCtx->psDevConfig;
        }
        if (pui32NumPipes)
        {
            *pui32NumPipes = psDecCtx->ui32NumPipes;
        }

        psDecCtx->bInitialised = IMG_TRUE;
    }

    return IMG_SUCCESS;

error:
    if (*phDecCtx)
    {
        if (psDecCtx->bInitialised)
        {
            IMG_UINT32 ui32DeinitResult;

            /* Undo the initialisation. */
            ui32DeinitResult = DECODER_Deinitialise(*phDecCtx);
            if (ui32DeinitResult != IMG_SUCCESS)
            {
                REPORT(REPORT_MODULE_DECODER, REPORT_ERR,
                    "DECODER_Deinitialise() failed to tidy-up after error");
            }
        }
        else
        {
            IMG_FREE(*phDecCtx);
            *phDecCtx = IMG_NULL;
        }
    }

    return ui32Result;
}


#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              DECODER_GethHwCtxHandle

******************************************************************************/
extern IMG_HANDLE
DECODER_GethHwCtxHandle(
    IMG_HANDLE hDecCtx
)
{
    DECODER_sContext *psDecCtx = (DECODER_sContext*)hDecCtx;
    DECODER_sCoreContext * psDecCoreCtx;

    /* Check input parameters. */
    IMG_ASSERT(hDecCtx);
    if(hDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_NULL;
    }

    psDecCoreCtx = psDecCtx->psDecCoreCtx;
    IMG_ASSERT(psDecCoreCtx);
    if (!psDecCoreCtx)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid pointer to decoder core context!");
        return IMG_NULL;
    }

    return psDecCoreCtx->hHwCtx;
}


/*!
******************************************************************************

 @Function              DECODER_GetPtdDevAddr

******************************************************************************/
extern IMG_UINT32
DECODER_GetPtdDevAddr(
    IMG_HANDLE hDecCtx
)
{
    DECODER_sContext *psDecCtx = (DECODER_sContext*)hDecCtx;

    /* Check input parameters. */
    IMG_ASSERT(hDecCtx);
    if(hDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return 0;
    }

    return psDecCtx->sPtdBufInfo.ui32DevVirt;
}


/*!
******************************************************************************

 @Function              DECODER_GetStreamId

******************************************************************************/
extern IMG_UINT32
DECODER_GetStreamId(
    IMG_HANDLE hDecStrCtx
)
{
    DECODER_sStrContext *psDecStrCtx = (DECODER_sStrContext*)hDecStrCtx;

    IMG_ASSERT(hDecStrCtx);
    if(hDecStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return 0;
    }

    return psDecStrCtx->ui32StrId;
}


/*!
******************************************************************************

 @Function              DECODER_SetFWBSP_BRIDGEStrCtxHandle

******************************************************************************/
extern IMG_RESULT
DECODER_SetFWBSP_BRIDGEStrCtxHandle(
    IMG_HANDLE hDecStrCtx,
    IMG_HANDLE hFWBSP_BRIDGEStrCtx
)
{
    DECODER_sStrContext *psDecStrCtx = (DECODER_sStrContext*)hDecStrCtx;

    IMG_ASSERT(hDecStrCtx && hFWBSP_BRIDGEStrCtx);
    if(hDecStrCtx == IMG_NULL || hFWBSP_BRIDGEStrCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDecStrCtx->hFWBSP_BRIDGEStrCtx = hFWBSP_BRIDGEStrCtx;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              DECODER_GetStreamId

******************************************************************************/
extern IMG_UINT32
DECODER_GetStreamCount(
    IMG_HANDLE hDecCtx
)
{
    DECODER_sContext *psDecCtx = (DECODER_sContext*)hDecCtx;

    /* Check input parameters. */
    IMG_ASSERT(hDecCtx);
    if(hDecCtx == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return 0;
    }

    return psDecCtx->ui32StrCnt;
}
#endif /* VDEC_USE_PVDEC_SEC */
