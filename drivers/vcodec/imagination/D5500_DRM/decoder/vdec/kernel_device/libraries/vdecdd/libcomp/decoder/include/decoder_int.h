/*!
 *****************************************************************************
 *
 * @File       decoder_int.h
 * @Title      VDECDD Internal Structures.
 * @Description    This file contains the VDECDD internal structures.
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

#ifndef __DECODER_INT_H__
#define __DECODER_INT_H__

#include "img_defs.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
    DECODER_RESTYPE_TRANSACTION = 0,
    DECODER_RESTYPE_HDR,
    DECODER_RESTYPE_BATCH_MSG,
#ifdef HAS_HEVC
    DECODER_RESTYPE_HEVC_BUF,
#ifdef VDEC_USE_PVDEC_SEC
    DECODER_RESTYPE_HEVC_SEC_BUF,
#endif /* VDEC_USE_PVDEC_SEC */
#endif /* HAS_HEVC */

    DECODER_RESTYPE_MAX,

} DECODER_eResType;

/// Resource Structure for DECODER_sDdResourceInfo to be used with pools
typedef struct
{
    LST_LINK;

    IMG_HANDLE              hResource;
    VXDIO_sDdBufInfo      * psDdBufInfo;

} RES_sResInfo;

/*!
******************************************************************************
 This structure contains the Decoded attributes
 @brief Decoded attributes
******************************************************************************/
typedef struct
{
    IMG_BOOL                    bFirstFieldReceived;
    IMG_UINT32                  ui32FEError;            /*!< Flags to indicate presence of detected errors during the decoding of this picture
                                                        (#VDECFW_eMsgFlagDecodedFeError).                                                               */
    IMG_UINT32                  ui32NoBEDWT;            /*!< No of BEWDT events                                                                         */
    IMG_UINT32                  ui32MbsDropped;         /*!< Number of BE macroblocks dropped for this picture                                          */
    IMG_UINT32                  ui32MbsRecovered;       /*!< Number of BE macroblocks revovered for this picture                                        */
    IMG_BOOL                    bDWRFired;              /*!< Signals that DWR fired during the decoding of this picture                                 */
    IMG_BOOL                    bMMUFault;              /*!< Signals that MMU fault occured during the decoding of this picture                         */

} DECODER_sPictAttrs;

/// Debug.
typedef struct
{
    LST_LINK;                                     /*!< List link (allows the structure to be part of a MeOS list). */

    DECODER_sPictAttrs          sPictAttrs;
    VDECFW_sPerformanceData     sPerformanceData;
    VDEC_sPictHwCrc             sPictHwCrc;

} DECODER_sFwMessage;


/*!
******************************************************************************
 This structure contains the device decode resource (used for decoding and held
 for subsequent decoding).
 @brief  Decoder Device Resource
******************************************************************************/
typedef struct
{
    LST_LINK;                                           /*!< List link (allows the structure to be part of a MeOS list).    */

    IMG_UINT32              ui32TransactionId;

    VXDIO_sDdBufInfo        sFwCtxBuf;                  /*!< FW context buffer (share state with next picture in stream).   */
#ifdef HAS_H264
    VXDIO_sDdBufInfo        sH264SgmBuf;                /*!< H.264 SGM buffer.                                              */
#endif /* HAS_H264 */
#ifdef HAS_MPEG4
    VXDIO_sDdBufInfo        sMPEG4FEVLRBackup;          /*!< MPEG4 FE Buffer to backup not coded flags from VLR.            */
#endif /* HAS_MPEG4 */
    IMG_UINT32              ui32RefCount;

} DECODER_sPictDecRes;


/*!
******************************************************************************
 This structure contains the stream decode resource (persistent for longer than decoding).
 @brief  Decoder Stream Resource
******************************************************************************/
typedef struct
{
    LST_LINK;                              /*!< List link (allows the structure to be part of a MeOS list).*/

    VXDIO_sDdBufInfo        sFwCtrlBuf;

    IMG_UINT32              ui32RefCount;

} DECODER_sPictRefRes;


/*!
******************************************************************************
 This structure contains the pointer to the picture segment.

 All the segments could be added to the list in DECODER_sDecPict, but because
 list items cannot belong to more than one list this wrapper is used which is
 added in the list sDecPictSegList inside DECODER_sDecPict

 @brief  Decoder Picture Segment

******************************************************************************/
typedef struct
{
    LST_LINK;                       /*!< List link (allows the structure to be part of a MeOS list). */

    BSPP_sBitStrSeg * psBitStrSeg;

    IMG_BOOL          bInternalSeg;

} DECODER_sDecPictSeg;

/*!
******************************************************************************
 This structure contains offsets of register spaces
 @brief  Register spaces offsets
******************************************************************************/
typedef struct
{
    IMG_UINT32 ui32VdmcCmdOffset;
    IMG_UINT32 ui32VecOffset;
    IMG_UINT32 ui32EntropyOffset;
    IMG_UINT32 ui32VecBeRegsOffset;
    IMG_UINT32 ui32VdecBeCodecRegsOffset;

} DECODER_sRegsOffsets;

/*!
******************************************************************************
 This structure defines the decode picture.
 @brief  Decoder Picture
******************************************************************************/
typedef struct
{
    LST_LINK;                                        /*!< List link (allows the structure to be part of a MeOS list).                            */

    IMG_UINT32             ui32TransactionId;        /*!< Transaction ID as defined by #GET_STREAM_PICTURE_ID and other associated macros.       */
    IMG_UINT8              ui8Pipe;                  /*!< Pipe to be decode on.                                                                  */
    IMG_VOID             * psDecStrCtx;              /*!< Decoder stream context structure.                                                      */

    IMG_BOOL               bTwoPass;                 /*!< Flag to indicate whether two passes are required.                                      */
    IMG_BOOL               bFirstFieldReceived;      /*!< Flag to indicate whether the first field has been received                             */

    RES_sResInfo         * psTransactionInfo;        /*!< Pointer to FW "Transaction Data Structure" (#VDECFW_sTransaction) memory structure.    */
    RES_sResInfo         * psHdrInfo;                /*!< Pointer to FW "Header Data Structure" memory structure.                                */
#ifdef HAS_HEVC
    RES_sResInfo         * psHevcInfo;               /*!< Pointer to additional HEVC buffer  memory structure.                                   */
    IMG_UINT32             ui32TemporalOutAddr;      /*!< Holds address of temporal mvp outout                                                   */
#endif /* HAS_HEVC */

    VDECDD_sDdPictBuf    * psReconPict;              /*!< Pointer to "Reconstructed Picture Buffer" memory structure.                            */
    VDECDD_sDdPictBuf    * psAltPict;                /*!< Pointer to "Alternative Picture Buffer" memory structure.                              */

    RES_sResInfo         * psBatchMsgInfo;           /*!< Pointer to buffer that is used to hold the batch message                               */

    // Pointers to core/static data.
    VXDIO_sDdBufInfo     * psIntraBufInfo;           /*!< Pointer to "Intra Buffer" memory structure.                                            */
    VXDIO_sDdBufInfo     * psAuxLineBufInfo;         /*!< Pointer to "Aux Line Buffer" memory structure.                                         */
    VXDIO_sDdBufInfo     * psVlcTablesBufInfo;       /*!< Pointer to "VLC Tables Buffer" memory structure.                                       */
    VXDIO_sDdBufInfo     * psVlcIdxTableBufInfo;     /*!< Pointer to "VLC Index Table Buffer" memory structure.                                  */
    VXDIO_sDdBufInfo     * psStartCodeBufInfo;       /*!< Pointer to "Start Code Buffer" memory structure.                                       */

    VXDIO_sDdBufInfo     * psStrPtdBufInfo;          /*!< Pointer to memory structure which holds Stream Page Table Directory address.           */

    DECODER_sFwMessage   * psFirstFieldFwMsg;        /*!< Debug                                                                                  */
    DECODER_sFwMessage   * psSecondFieldFwMsg;       /*!< Debug                                                                                  */

    BSPP_sPictHdrInfo    * psPictHdrInfo;            /*!< Pointer to picture header information.                                                 */

    DECODER_sPictDecRes  * psCurPictDecRes;          /*!< Pointer to device resource for current picture (inc. fw context).                      */
    DECODER_sPictDecRes  * psPrevPictDecRes;         /*!< Pointer to device resource for previous picture (inc. fw context).                     */

    DECODER_sPictRefRes  * psPictRefRes;             /*!< Pointer to stream resource for current picture.                                        */

    LST_T                  sDecPictSegList;          /*!< List of segments (DECODER_sDecPictSeg) for the current picture                         */

    IMG_UINT32             ui32FeWdtPeriod;          /*!< Front-end hardware watchdog timeout period (cycles?)                                   */
    IMG_UINT32             ui32BeWdtPeriod;          /*!< Back-end hardware watchdog timeout period (cycles?)                                    */
    IMG_UINT32             ui32PSRWdtPeriod;         /*!< Parser hardware watchdog timeout period (mtx clocks)                                   */

    IMG_UINT32             ui32OperatingMode;
    IMG_UINT16             ui16GencId;               /*!< Genc buffer ID.                                                                        */
    VDECDD_sDdBufMapInfo **pasGencBuffers;           /*!< GENC buffers TO DO that's not a good place to hold it*/
    VDECDD_sDdBufMapInfo * psGencFragmentBuffer;           /*!< GENC buffers TO DO that's not a good place to hold it*/
    IMG_UINT32             ui32CtrlAllocBytes;       /*!< Control allocation size in bytes.                                                      */
    IMG_UINT32             ui32TileCfg;              /*!< MMU tiling configuration                                                               */
    IMG_BOOL               bDecoded;                 /*!< Flag set when pic is decoded.                                                          */
    VXDIO_sDdBufInfo     * psStrPvdecFwCtxBuf;       /*!< Additional PVDEC FW context buffer.                                                    */
#ifndef IMG_KERNEL_MODULE
    IMG_UINT32             ui32NumAvailPipes;       /*!< Number of available pipes.                                                              */
#endif

#ifdef VDEC_USE_PVDEC_SEC
    IMG_UINT8              ui8RefPicQty;
    IMG_UINT16             ui16RefPicMask;
    IMG_UINT8              ui8ColPicQty;
    IMG_UINT32             aui32RefPicLumaAddrList[VDECFW_MAX_NUM_PICTURES];
    IMG_UINT32             aui32RefPicChromaAddrList[VDECFW_MAX_NUM_PICTURES];
#ifdef ENABLE_SEPARATE_COLOUR_PLANES
    IMG_UINT32             aui32RefPicChroma2AddrList[VDECFW_MAX_NUM_PICTURES];
#endif /* ENABLE_SEPARATE_COLOUR_PLANES */
    IMG_UINT32             aui32ColPicAddrList[VDECFWBSP_MAX_NUM_COL_PICS];
    VDECDD_sDPBState       sDPBState;               /*!< State of firmware DPB, used to display pictures during stream flush                     */
    IMG_UINT32             ui32RingBufMark;         /*!< Ring buffer start marker for current picture.                                           */
#endif /* VDEC_USE_PVDEC_SEC */

} DECODER_sDecPict;


#ifdef __cplusplus
}
#endif

#endif /* __DECODER_INT_H__ */
