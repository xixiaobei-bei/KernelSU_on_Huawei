/*!
 *****************************************************************************
 *
 * @File       fwbsp_bridge.h
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

#ifndef __FWBSP_BRIDGE_H__
#define __FWBSP_BRIDGE_H__

#include "img_defs.h"
#include "vdecdd_int.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FWBSP_BRIDGE_MAX_BITSTREAM_SEGMENTS     (16)

/* PARSE_ID(32): unused(4) | stream_id(8) | unused(4) | pic_id(16) */
#define FWBSP_BRIDGE_NUMBITS_PICTURE_ID         (16)
#define FWBSP_BRIDGE_OFFSET_PICTURE_ID          (0)

#define FWBSP_BRIDGE_NUMBITS_STREAM_ID          (8)
#define FWBSP_BRIDGE_OFFSET_STREAM_ID           (4 + 16)

#define FWBSP_BRIDGE_BIT_MASK(num) ((1 << num) - 1)

#define FWBSP_BRIDGE_SET_PICTURE_ID(pic_id)                                   \
    ((pic_id & FWBSP_BRIDGE_BIT_MASK(FWBSP_BRIDGE_NUMBITS_PICTURE_ID))        \
    << FWBSP_BRIDGE_OFFSET_PICTURE_ID)

#define FWBSP_BRIDGE_SET_STREAM_ID(stream_id)                                 \
    ((stream_id & FWBSP_BRIDGE_BIT_MASK(FWBSP_BRIDGE_NUMBITS_STREAM_ID))      \
    << FWBSP_BRIDGE_OFFSET_STREAM_ID)

#define FWBSP_BRIDGE_CREATE_PARSE_ID(strid, pic)                              \
    FWBSP_BRIDGE_SET_STREAM_ID(strid) | FWBSP_BRIDGE_SET_PICTURE_ID(pic)

#define FWBSP_BRIDGE_GET_PICTURE_ID(parse_id)                                 \
    (parse_id & FWBSP_BRIDGE_BIT_MASK(FWBSP_BRIDGE_NUMBITS_PICTURE_ID))

#define FWBSP_BRIDGE_GET_STREAM_ID(parse_id)                                  \
    ((parse_id >> FWBSP_BRIDGE_OFFSET_STREAM_ID)                              \
    & FWBSP_BRIDGE_BIT_MASK(FWBSP_BRIDGE_NUMBITS_STREAM_ID))

/*!
******************************************************************************
 This enumeration describes the FWBSP_BRIDGE callback types.
 @brief  FWBSP_BRIDGE Callback Types
******************************************************************************/
typedef enum
{
    /*!< New encrypted bitstream resource bundle (pvData: VDECDD_sPictResInt */
    FWBSP_BRIDGE_CB_BITSTREAM_RESOURCE_ADD = VDECDD_CBGRP_FWBSP_BRIDGE,
    /*!< Encrypted bitstream resource should no longer be referenced.        */
    FWBSP_BRIDGE_CB_BITSTREAM_RESOURCE_REMOVE,
    /*!< Unit processed by FWBSP_BRIDGE. (pvData: #VDECDD_sStrUnit)          */
    FWBSP_BRIDGE_CB_UNIT_PROCESSED,
    FWBSP_BRIDGE_CB_MAX,

} FWBSP_BRIDGE_eCBType;


/*!
******************************************************************************
 This structure contains FW_BRIDGE messages data.
 @brief  FWBSP_BRIDGE messages data.
******************************************************************************/
typedef struct
{
    /*!< Init message                                                        */
    IMG_UINT8  ui8VidStd;
    IMG_UINT32 ui32PtdAddr;
    IMG_UINT32 ui32SPSAddr;
    IMG_UINT32 ui32SPSSize;  /* Size of SPS buffer in bytes */
    IMG_UINT32 ui32PPSAddr;
    IMG_UINT32 ui32PPSSize;  /* Size of PPS buffer in bytes */
    IMG_UINT32 ui32VPSAddr;
    IMG_UINT32 ui32VPSSize;  /* Size of VPS buffer in bytes */
    IMG_UINT32 ui32RingBufAddr;
    IMG_UINT32 ui32RingBufSize;
    IMG_UINT32 ui32RingBufMark;

    /*!< Submit encrypted bitstream message                                  */
    IMG_UINT32 ui32FwCtxLoadAddr;
    IMG_UINT32 ui32FwCtxSaveAddr;
    IMG_UINT32 ui32FwCtxSize;
    IMG_UINT8  ui8BitstreamQty;
    IMG_BOOL   bCodecCfg;
    IMG_BOOL   bSizeDelimMode;
    IMG_BOOL   bFirstGOP;
    IMG_UINT32 aui32BitstreamAddr[FWBSP_BRIDGE_MAX_BITSTREAM_SEGMENTS];
    IMG_UINT32 aui32BitstreamSize[FWBSP_BRIDGE_MAX_BITSTREAM_SEGMENTS];
    IMG_UINT32 ui32ParseId;
    IMG_UINT32 ui8CodecMode;
    IMG_UINT32 ui32Mem2RegAddr;
    IMG_UINT32 ui32Mem2RegSize; /* Size of MEM2REG buffer in bytes */
    IMG_UINT32 ui32SliceParamsAddr;
    IMG_UINT32 ui32SliceParamsSize;

} FWBSP_BRIDGE_sFwMsgData;


/*!
******************************************************************************
 This structure contains FW_BRIDGE messages data.
 @brief  FWBSP_BRIDGE messages data.
******************************************************************************/
typedef struct
{
    /*!< Common fields                                                       */
    IMG_UINT16 ui16MsgId;

    /*!< New sequence message                                                */
    IMG_UINT16 ui16PicWidth;
    IMG_UINT16 ui16PicHeight;
    IMG_UINT16 ui16MaxPicWidth;
    IMG_UINT16 ui16MaxPicHeight;
    IMG_UINT16 ui16FrameCropLeft;
    IMG_UINT16 ui16FrameCropRight;
    IMG_UINT16 ui16FrameCropTop;
    IMG_UINT16 ui16FrameCropBottom;
    IMG_UINT8  ui8LevelIdc;
    IMG_UINT8  ui8ProfileIdc;
    IMG_UINT8  ui8ChromaFormatIdc;
    IMG_UINT8  ui8BitDepthChroma;
    IMG_UINT8  ui8BitDepthLuma;
    IMG_UINT8  ui8SeqHdrId;
    IMG_BOOL   bFrameMbsOnly;
    IMG_BOOL   bSepColPlane;
    IMG_BOOL   bFrameCropping;
    IMG_BOOL   bAdaptiveFrameField;
    IMG_BOOL   bVUIParamsPresent;

    /*!< VUI params                                                          */
    IMG_BOOL   bColorSpacePresent;
    IMG_BOOL   bBstrRestrFlag;
    IMG_BOOL   bTimingInfo;
    IMG_UINT8  ui8ColorPrimaries;
    IMG_UINT8  ui8TransferCharacteristic;
    IMG_UINT8  ui8MartixCoeffs;
    IMG_UINT32 ui32TimeScale;
    IMG_UINT32 ui32NumUnitsInTick;
    IMG_UINT32 ui32MaxDecFrameBuff;
    IMG_UINT16 ui16SarWidth;
    IMG_UINT16 ui16SarHeight;

    /*!< New picture message                                                 */
    IMG_UINT8  ui8PicToDispQty;
    IMG_UINT8  ui8PicToRelQty;
    IMG_UINT8  ui8RefPicQty;
    IMG_UINT8  ui8ColPicQty;
    IMG_UINT8  ui8PicType;
    IMG_UINT16 ui16RefPicMask;
    IMG_BOOL   bFieldCoded;
    IMG_BOOL   b2ndFieldOfPair;
    IMG_BOOL   bTopFieldFirst;
    IMG_BOOL   bClosedGOP;
    IMG_BOOL   bBackwardRefs;
    IMG_BOOL   bRepeatFirstField;
    IMG_UINT8  ui8MaxFrmRepeat;
    IMG_BOOL   bNotDpbFlush;
    IMG_UINT32 ui32ParseId;
    IMG_UINT32 ui32ErrorFlags;
    IMG_UINT32 ui32RingBufMark;
    /*!< List of ParseIds indicating buffers ready to display,
     * in display order                                                      */
    IMG_UINT32 aui32DisplayList[VDECFW_MAX_NUM_PICTURES];
    /*!< List of flags for each TID in the DisplayList                       */
    IMG_UINT8  aui8DisplayFlags[VDECFW_MAX_NUM_PICTURES];
    /*!< List of ParseIds indicating buffers that are no longer
     * required for reference                                                */
    IMG_UINT32 aui32ReleaseList[VDECFW_MAX_NUM_PICTURES + VDECFW_MAX_NUM_VIEWS];
    /*!< List of ParseIds indicating referece pictures                       */
    IMG_UINT32 aui32RefPicsList[VDECFW_MAX_NUM_PICTURES];
    /*!< List of ParseIds indicating colocated pictures                      */
    IMG_UINT32 aui32ColPicPids[VDECFWBSP_MAX_NUM_COL_PICS];

    /*!< Data derived from bitstream required for picture management. Parse IDs
     * are used to identify picture in this structure. */
    VDECDD_sFWBSPData sFWBSPDataPids;

} FWBSP_BRIDGE_sHostMsgData;



/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_Initialise

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_Initialise(
    IMG_VOID *                 pvInitUserData,
    VDECDD_pfnCompCallback     pfnCallback,
    IMG_HANDLE                *phCtx,
    IMG_HANDLE                 hDecCtx,
    const VDECDD_sDdDevConfig *psDdDevConfig
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_DeInitialise

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_DeInitialise(
    IMG_HANDLE hCtx
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamCreate

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_StreamCreate(
    IMG_HANDLE          hCtx,
    IMG_HANDLE         *phStrCtx,
    IMG_HANDLE          hMmuStrHandle,
    IMG_HANDLE          hDecStrHandle,
    IMG_VOID           *pvStrUserData,
    IMG_UINT32          ui32StrId,
    SYS_eMemPool        eSecureMemPool,
    VDEC_sStrConfigData sStrConfig
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamDestroy

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_StreamDestroy(
    const IMG_HANDLE hStrContext
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamProcessUnit

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_StreamProcessUnit(
    IMG_HANDLE hDecStrCtx,
    VDECDD_sStrUnit *psStrUnit
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PictureAdd

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_PictureAdd(
    IMG_HANDLE hStrCtx,
    VDECDD_sStrUnit * psStrUnit,
    IMG_UINT32 ui32TransId,
    VDECDD_sPicture *psPicture
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PicturePrepare
******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_PicturePrepare(
    IMG_HANDLE hStrCtx,
    VDECDD_sStrUnit *psStrUnit,
    IMG_HANDLE hDecPict
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PictureRemove

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_PictureRemove(
    IMG_HANDLE hStrCtx,
    IMG_UINT32 ui32Id,
    IMG_BOOL   bTransId
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_PictureReturn

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_PictureReturn(
    IMG_HANDLE hStrCtx,
    IMG_UINT32 ui32Id,
    IMG_BOOL   bTransId
);

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_FwMsgBufFill

*****************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_FwMsgBufFill(
    IMG_HANDLE  hStrCtx,
    IMG_UINT32  ui32ParseId,
    IMG_UINT32 *pui32Msg,
    IMG_UINT16  ui16MsgSize
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_ProcessFwMsg

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_ProcessFwMsg(
    IMG_HANDLE hStrCtx
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_IsStreamIdle

 @Description

 This function report whether the stream has no outstanding

 stream units to process.

 @Input     hStrCtx  : FWBSP_BRIDGE stream context handle.

 @Return    IMG_BOOL : Indicates whether stream is idle.

******************************************************************************/
extern IMG_BOOL
FWBSP_BRIDGE_IsStreamIdle(
    IMG_HANDLE hStrCtx
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_IsStreamStopped

 @Description

 This function report whether the stream is stopped.

 @Input     hStrCtx  : FWBSP_BRIDGE stream context handle.

 @Return    IMG_BOOL : Indicates whether stream is stopped.

******************************************************************************/
extern IMG_BOOL
FWBSP_BRIDGE_IsStreamStopped(
    IMG_HANDLE hStrCtx
);


/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_StreamPlay

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_StreamPlay(
    IMG_HANDLE hStrCtx,
    VDEC_eStopPoint eStopPoint
);

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_GetAddresses

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_GetAddresses(
    IMG_HANDLE        hStrCtx,
    IMG_UINT32        ui32ParseId,
    IMG_UINT32      * pui32LumaAddr,
    IMG_UINT32      * pui32ChromaAddr,
    IMG_UINT32      * pui32MbParamAddr
);

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_DiscardBitstreamUnit

 @Description

******************************************************************************/
extern IMG_RESULT
FWBSP_BRIDGE_DiscardBitstreamUnit(
    IMG_HANDLE        hStrCtx,
    IMG_UINT32        ui32PID
);

/*!
******************************************************************************

 @Function              FWBSP_BRIDGE_GetStreamCount

******************************************************************************/
extern IMG_UINT8
FWBSP_BRIDGE_GetStreamCount(
    IMG_HANDLE hCtx
);


#if defined(__cplusplus)
}
#endif

#endif /* not __FWBSP_BRIDGE_H__ */
