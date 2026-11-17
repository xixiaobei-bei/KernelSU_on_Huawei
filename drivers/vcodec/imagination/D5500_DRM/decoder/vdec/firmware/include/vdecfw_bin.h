/*!
 *****************************************************************************
 *
 * @File       vdecfw_bin.h
 * @Title      Declarations of data structures for the compiled firmware.
 * @Description    Declarations of data structures for the compiled firmware.
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

#ifndef _VDECFW_BIN_H_
#define _VDECFW_BIN_H_

#include "img_include.h"
#include "vdec_api.h"
#include "firmware.h"

#ifdef VDEC_USE_PVDEC_SEC
#define PVDEC_FULL_FW_NAME "pvdec_full_bin.fw"
#define PVDEC_SEC_FW_NAME "pvdec_sec_bin.fw"
#endif

/*!
******************************************************************************
 This structure contains configuration of buffers with firmware.
 @brief  Firmware buffers info.
******************************************************************************/
typedef struct
{
    IMG_UINT32 * aui32Text;                      /*!< Buffer with firmware text.                          */
    IMG_UINT32   ui32TextBufSize;                /*!< Size of firmware text buffer.                       */
    IMG_UINT32   ui32TextDmaSize;                /*!< Size of firmware text buffer to be DMA'ed
                                                      (it's possible do DMA only part of firmware text).  */
    IMG_UINT32   ui32TextOrigin;                 /*!< Address for which firmware text was linked.         */

} VDECFW_sFirmwareBinInfo;


IMG_RESULT VDECFW_GetPvdecFirmware(
    VDECFW_sFirmwareBinInfo *  psFirmwareBinInfo,
    IMG_BOOL                   bSecure
);
#ifdef VDEC_USE_PVDEC_SEC
IMG_RESULT VDECFW_InspectPvdecFirmware(
    IMG_PUINT8 pui8Buffer,
    IMG_UINT32 ui32Size,
    IMG_BOOL   bSecure
);
#endif /* VDEC_USE_PVDEC_SEC */

#ifdef VDEC_USE_PVDEC_SEC
const PVDECFW_sPvdecFwFileInfo * VDECFW_GetFwFileInfoSec(
    IMG_UINT32 ui32FwFileNameHash
);
#endif /* VDEC_USE_PVDEC_SEC */

const PVDECFW_sPvdecFwFileInfo * VDECFW_GetFwFileInfoFull(
    IMG_UINT32 ui32FwFileNameHash
);

#endif /* _VDECFW_BIN_H_ */
