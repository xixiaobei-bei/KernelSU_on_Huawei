/*!
 *****************************************************************************
 *
 * @File       vxd.h
 * @Description    This file contains definition of VXD_sCoreProps structure.
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

#if !defined (__VXD_H__)
#define __VXD_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <img_types.h>
#include <vdecdd_mmu_defs.h>
#include <vdec_api.h>

/* Maximum number of supported pixel pipes */
#ifndef VDEC_USE_PVDEC
#define VDEC_MAX_PIXEL_PIPES 1
#else
#define VDEC_MAX_PIXEL_PIPES 4
#endif


#define FROM_REV(maj, min, maint)    ((ui32MajRev > maj) || \
                                     ((ui32MajRev == maj) && (ui32MinRev > min)) || \
                                     ((ui32MajRev == maj) && (ui32MinRev == min) && ((IMG_INT32)(ui32MaintRev) >= maint)))

#define BEFORE_REV(maj, min, maint)  (((IMG_INT32)(ui32MajRev) < maj) || \
                                     ((ui32MajRev == maj) && ((IMG_INT32)(ui32MinRev) < min)) || \
                                     ((ui32MajRev == maj) && (ui32MinRev == min) && ((IMG_INT32)(ui32MaintRev) < maint)))

#define AT_REV(maj, min, maint)      ((ui32MajRev == (IMG_UINT32)maj) && (ui32MinRev == (IMG_UINT32)min) && (ui32MaintRev == (IMG_UINT32)maint))



#define CORE_REVISION(maj, min, maint)      ((((maj) & 0xff) << 16) | (((min) & 0xff) << 8) | (((maint) & 0xff)))
#define MAJOR_REVISION(rev)                 (((rev) >> 16) & 0xff)
#define MINOR_REVISION(rev)                 (((rev) >> 8) & 0xff)
#define MAINT_REVISION(rev)                 ((rev) & 0xff)

/*!
******************************************************************************
 This structure contains the standard-specific core properties.
 @brief  Standard Specific Core Properties
******************************************************************************/
typedef struct
{
    VDEC_eVidStd    eVidStd;                //!< Video standard to associate properties
    IMG_UINT32      ui32CoreRev;            //!< Core revision from which to start applying properties

    IMG_UINT32      ui32MinWidth;           //!< Minimum coded frame width (in pixels)
    IMG_UINT32      ui32MinHeight;          //!< Minimum coded frame height (in pixels). Round-up to multiple of 32 for field coded frame
    IMG_UINT32      ui32MaxWidth;           //!< Maximum coded frame width (in pixels)
    IMG_UINT32      ui32MaxHeight;          //!< Maximum coded frame height (in pixels). Round-up to multiple of 32 for field coded frame
    IMG_UINT32      ui32MaxMacroblocks;     //!< Maximum number of MBs in a coded frame of video

    IMG_UINT32      ui32MaxLumaBitDepth;    //!< Max bit-depth for luma channel
    IMG_UINT32      ui32MaxChromaBitDepth;  //!< Max bit-depth for chroma channel

    PIXEL_FormatIdc eMaxChromaFormat;       //!< Max chroma format

} VXD_sVidStdProps;


/*!
******************************************************************************
 This structure contains the core properties (inherent and internal).
 @brief  Core Properties
******************************************************************************/
typedef struct VXD_sCoreProps_tag
{
    IMG_CHAR            acVersion[64];                           /*!< Deliberately a string since this should not be used outside
                                                                  MSVDXIO to determine features.                                     */
    /* Video Standards. */
    IMG_BOOL            abMpeg2[VDEC_MAX_PIXEL_PIPES];           /*!< MPEG2 supported                                                */
    IMG_BOOL            abMpeg4[VDEC_MAX_PIXEL_PIPES];           /*!< MPEG4 supported                                                */
    IMG_BOOL            abH264[VDEC_MAX_PIXEL_PIPES];            /*!< H.264 supported                                                */
    IMG_BOOL            abVc1[VDEC_MAX_PIXEL_PIPES];             /*!< VC-1 supported                                                 */
    IMG_BOOL            abAvs[VDEC_MAX_PIXEL_PIPES];             /*!< AVS supported                                                  */
    IMG_BOOL            abReal[VDEC_MAX_PIXEL_PIPES];            /*!< Real Video supported                                           */
    IMG_BOOL            abJpeg[VDEC_MAX_PIXEL_PIPES];            /*!< JPEG supported                                                 */
    IMG_BOOL            abVp6[VDEC_MAX_PIXEL_PIPES];             /*!< On2 VP6 supported                                              */
    IMG_BOOL            abVp8[VDEC_MAX_PIXEL_PIPES];             /*!< On2 VP8 supported                                              */
    IMG_BOOL            abSorenson[VDEC_MAX_PIXEL_PIPES];        /*!< Sorenson supported                                             */
    IMG_BOOL            abHEVC[VDEC_MAX_PIXEL_PIPES];            /*!< HEVC supported                                                 */

    /* Features (explicit) */
    IMG_BOOL            abRotationSupport[VDEC_MAX_PIXEL_PIPES]; /*!< Rotation (alternate out-of-loop output) supported              */
    IMG_BOOL            abScalingSupport[VDEC_MAX_PIXEL_PIPES];  /*!< Scaling (alternate out-of-loop output) supported               */
    IMG_BOOL            bHevcRangeExt[VDEC_MAX_PIXEL_PIPES];     /*!< Range extensions supported by hw -> used only by hevc          */
    IMG_BOOL            bHdSupport;                              /*!< High-definition video supported                                */
    IMG_UINT32          aui32NumStreams[VDEC_MAX_PIXEL_PIPES];   /*!< Number of shift registers for concurrent parsing of streams    */

    IMG_UINT32          ui32NumEntropyPipes;                     /*!< Number of entropy pipes, read out from H/W                     */
    IMG_UINT32          ui32NumPixelPipes;                       /*!< Number of pixel pipes, read out from H/W                       */

    /* Features (version-based) */
    IMG_BOOL            bChromaUpsample;                         /*!<                                                                */
    IMG_UINT32          ui32SrDmaBurstSize;                      /*!< Burst size (in bytes)                                          */
    IMG_UINT32          ui32ScalingPrecision;                    /*!<                                                                */
    IMG_UINT32          ui32CmdBufferSize;                       /*!< Size (in words) of back-end command FIFO                       */
    IMG_UINT32          ui32VdmcCacheSize;                       /*!< Multiplier from original/default (e.g. x1, x2, x4).            */
    IMG_BOOL            ui32Latency;                             /*!<                                                                */
    IMG_BOOL            b444H264Rotation;                        /*!< 4:4:4 rotation without downsampling for H264                   */
    VXD_sVidStdProps    asVidStdProps[VDEC_STD_MAX];             /*!< Core properties that vary depending upon video standard.       */

    /* Features (derived) */
    MMU_eMmuType        eMmuType;                                /*!< MMU Type                                                       */
    IMG_BOOL            bMmuSupportStridePerContext;             /*!< Indicates if mmu stride per context is supported               */
    IMG_BOOL            bMmuSupportSecure;                       /*!< Indicates whether mmu secure faults are supported or not       */

} VXD_sCoreProps;


#if defined(__cplusplus)
}
#endif

#endif /* __VXD_H__ */
