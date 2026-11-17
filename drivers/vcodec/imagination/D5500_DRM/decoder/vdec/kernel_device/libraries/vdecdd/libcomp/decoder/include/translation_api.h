/*!
 *****************************************************************************
 *
 * @File       translation_api.h
 * @Title      VDECDD Translation API.
 * @Description    This file contains the VDECDD translation api functions prototype.
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

#ifndef __TRANSLATION_API_H__
#define __TRANSLATION_API_H__

#include "img_defs.h"
#include "img_types.h"
#include "vdec_api.h"
#include "vdecdd_int.h"
#include "decoder_int.h"
#include "vxd_int.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
******************************************************************************

 @Function              TRANSLATION_CtrlAllocPrepare

 @Description

 This function submits a stream unit for translation into a control allocation
 buffer used in PVDEC operation.

 @Input     psStrConfigData : A pointer to a #VDEC_sStrConfigData structure.

 @Input     psStrUnit       : Pointer to stream unit.

 @Input     psDecPict       : Pointer to decoder picture.

 @Input     psCoreProps     : Pointer to core properties.

 @Input     psRegsOffsets   : Offsets of register spaces.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT
TRANSLATION_CtrlAllocPrepare(
    VDEC_sStrConfigData       * psStrConfigData,
    VDECDD_sStrUnit           * psStrUnit,
    DECODER_sDecPict          * psDecPict,
    const VXD_sCoreProps      * psCoreProps,
    DECODER_sRegsOffsets      * psRegsOffsets
);

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              TRANSLATION_PicturePrepareSecure

 @Description

 TODO add desc.

 @Input     psStrConfigData : A pointer to a #VDEC_sStrConfigData structure.

 @Input     psStrUnit       : Pointer to stream unit.

 @Input     psDecPict       : Pointer to decoder picture.

 @Input     psCoreProps     : Pointer to core properties.

 @Input     psRegsOffsets   : Offsets of register spaces.

 @Return    IMG_RESULT      : This function returns either IMG_SUCCESS or an
                              error code.

******************************************************************************/
extern IMG_RESULT
TRANSLATION_PicturePrepareSecure(
    IMG_HANDLE                  hFWBSPStrCtx,
    VDEC_sStrConfigData       * psStrConfigData,
    VDECDD_sStrUnit           * psStrUnit,
    DECODER_sDecPict          * psDecPict,
    const VXD_sCoreProps      * psCoreProps
);
#endif /* def VDEC_USE_PVDEC_SEC*/

#ifdef __cplusplus
}
#endif

#endif /* __TRANSLATION_API_H__   */
