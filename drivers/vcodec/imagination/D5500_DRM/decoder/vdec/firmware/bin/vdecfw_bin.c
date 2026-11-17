/*!
 *****************************************************************************
 *
 * @File       vdecfw_bin.c
 * @Description    Set of tables (containing firmware in binary form) and offsets
 *  allowing for loading firmware into MTX RAM for execution.
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

#include "dbgopt_api_km.h"
#include "vdecfw_bin.h"
#include "report_api.h"

#include "firmware.h"

#if !defined(VDEC_USE_PVDEC_SEC) || (defined(VDEC_USE_PVDEC_SEC) && !defined(SYSBRG_BRIDGING))
#include "pvdec_full_bin.c"
#endif /* not VDEC_USE_PVDEC_SEC || (VDEC_USE_PVDEC_SEC && SYSBRG_BRIDGING) */
#if defined(VDEC_USE_PVDEC_SEC) && !defined(SYSBRG_BRIDGING)
#include "pvdec_sec_bin.c"
#endif /* VDEC_USE_PVDEC_SEC && not SYSBRG_BRIDGING */

#include "pvdec_full_bin_ldb.c"
#ifdef VDEC_USE_PVDEC_SEC
#include "pvdec_sec_bin_ldb.c"

#include "pvdec_full_bin.h"
#include "pvdec_sec_bin.h"
#endif /* VDEC_USE_PVDEC_SEC */

#define __SET_BASE_FIRMWARE_INFO(info, mode, std) \
    do { \
        info->aui32Text              = aui32MTXBase##mode##std##Text; \
        info->aui32Data              = aui32MTXBase##mode##std##Data; \
        info->ui32TextBufSize        = ui32MTXBase##mode##std##TextSize; \
        info->ui32TextDmaSize        = ui32MTXBase##mode##std##TextSize; \
        info->ui32DataSize           = ui32MTXBase##mode##std##DataSize; \
        info->ui32TextOrigin         = ui32MTXBase##mode##std##TextOrigin; \
        info->ui32DataOrigin         = ui32MTXBase##mode##std##DataOrigin; \
    } while(0)

#define SET_BASE_FIRMWARE_INFO_SINGLE(info, mode, std) \
    __SET_BASE_FIRMWARE_INFO(info, _##mode##_single_, std##_)

#define SET_BASE_FIRMWARE_INFO_MULTI(info, mode) \
    __SET_BASE_FIRMWARE_INFO(info, _##mode##_multi, _)


#ifdef USE_FW_CTX_TRIMMING
#define __SET_MOD_FIRMWARE_CTX_TRIMMING_INFO(info, mode, std) \
    do { \
        info->ui32PsrCtxSize = ui32MTXParser##mode##std##CtxSize; \
    } while(0)
#endif /* USE_FW_CTX_TRIMMING */
#ifdef USE_FW_RELOC_INFO_PACKING
#define __SET_MOD_FIRMWARE_RELOC_PACKING_INFO(info, mode, std) \
    do { \
        info->ui32PackedRelocInfoSize = ui32MTXParser##mode##std##PackedRelocInfoSize; \
    } while(0)
#else /* not USE_FW_RELOC_INFO_PACKING */
#define __SET_MOD_FIRMWARE_RELOC_INFO(info, mode, std) \
    do { \
        info->aui32TextReloc          = aui32MTXParser##mode##std##TextReloc; \
        info->ui32TextRelocSize       = ui32MTXParser##mode##std##TextRelocSize; \
        info->aui8TextRelocType       = aui8MTXParser##mode##std##TextRelocType; \
        info->aui32TextRelocFullAddr  = aui32MTXParser##mode##std##TextRelocFullAddr; \
        info->aui32DataReloc          = aui32MTXParser##mode##std##DataReloc; \
        info->ui32DataRelocSize       = ui32MTXParser##mode##std##DataRelocSize; \
    } while(0)
#endif /* not USE_FW_RELOC_INFO_PACKING */
#define __SET_MOD_FIRMWARE_INFO(info, mode, std) \
    do { \
        info->aui32Text               = aui32MTXParser##mode##std##Text; \
        info->aui32Data               = aui32MTXParser##mode##std##Data; \
        info->ui32TextBufSize         = ui32MTXParser##mode##std##TextSize; \
        info->ui32TextDmaSize         = ui32MTXParser##mode##std##TextSize; \
        info->ui32DataSize            = ui32MTXParser##mode##std##DataSize; \
        info->ui32TextOrigin          = ui32MTXParser##mode##std##TextOrigin; \
        info->ui32DataOrigin          = ui32MTXParser##mode##std##DataOrigin; \
    } while(0)

#if defined(USE_FW_CTX_TRIMMING) && defined(USE_FW_RELOC_INFO_PACKING)
#define SET_MOD_FIRMWARE_INFO(info, mode, std) \
    do { \
        __SET_MOD_FIRMWARE_INFO(info, _##mode##_multi_, std##_); \
        __SET_MOD_FIRMWARE_CTX_TRIMMING_INFO(info, _##mode##_multi_, std##_); \
        __SET_MOD_FIRMWARE_RELOC_PACKING_INFO(info, _##mode##_multi_, std##_); \
    } while(0)
#elif defined(USE_FW_CTX_TRIMMING)
#define SET_MOD_FIRMWARE_INFO(info, mode, std) \
    do { \
        __SET_MOD_FIRMWARE_INFO(info, _##mode##_multi_, std##_); \
        __SET_MOD_FIRMWARE_CTX_TRIMMING_INFO(info, _##mode##_multi_, std##_); \
        __SET_MOD_FIRMWARE_RELOC_INFO(info, _##mode##_multi_, std##_); \
    } while(0)
#elif defined(USE_FW_RELOC_INFO_PACKING)
#define SET_MOD_FIRMWARE_INFO(info, mode, std) \
    do { \
        __SET_MOD_FIRMWARE_INFO(info, _##mode##_multi_, std##_); \
        __SET_MOD_FIRMWARE_RELOC_PACKING_INFO(info, _##mode##_multi_, std##_); \
    } while(0)
#else
#define SET_MOD_FIRMWARE_INFO(info, mode, std) \
    __SET_MOD_FIRMWARE_INFO(info, _##mode##_multi_, std##_); \
    __SET_MOD_FIRMWARE_RELOC_INFO(info, _##mode##_multi_, std##_);
#endif

/*!
******************************************************************************

 @Function              VDECFW_GetPvdecFirmware

 ******************************************************************************/
IMG_RESULT VDECFW_GetPvdecFirmware(
    VDECFW_sFirmwareBinInfo *  psFirmwareBinInfo,
    IMG_BOOL                   bSecure
)
{
    const FIRMWARE * psFirmware = IMG_NULL;

    if (bSecure)
    {
#if defined(VDEC_USE_PVDEC_SEC)
#if !defined(SYSBRG_BRIDGING)
        psFirmware = &sFwPvdec_sec;
#endif /* SYSBRG_BRIDGING */
#else /* not VDEC_USE_PVDEC_SEC*/
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Secure firmware not available!");
        return IMG_ERROR_NOT_SUPPORTED;
#endif /* VDEC_USE_PVDEC_SEC */
    }
    else
    {
#if !defined(VDEC_USE_PVDEC_SEC) || (defined(VDEC_USE_PVDEC_SEC) && !defined(SYSBRG_BRIDGING))
        psFirmware = &sFwPvdec_full;
#endif /* not VDEC_USE_PVDEC_SEC || (VDEC_USE_PVDEC_SEC && SYSBRG_BRIDGING) */
    }

    IMG_MEMSET(psFirmwareBinInfo, 0, sizeof(*psFirmwareBinInfo));

    // do not link built-in fw blob for bridging mode - it will be acquired from file system.
#if !defined(VDEC_USE_PVDEC_SEC) || (defined(VDEC_USE_PVDEC_SEC) && !defined(SYSBRG_BRIDGING))
    psFirmwareBinInfo->aui32Text = (IMG_UINT32 *)psFirmware->pui8Blob;
    psFirmwareBinInfo->ui32TextBufSize = psFirmware->uiBlobSize/sizeof(IMG_UINT32);
    psFirmwareBinInfo->ui32TextDmaSize = psFirmware->uiCoreSize/sizeof(IMG_UINT32);
    REPORT(REPORT_MODULE_HWCTRL, REPORT_INFO, "Using PVDEC firmware from %s built at %s, core: %d, blob: %d",
           psFirmware->psVersion, psFirmware->psBuildDate, psFirmware->uiCoreSize, psFirmware->uiBlobSize);
#endif /* not VDEC_USE_PVDEC_SEC || (VDEC_USE_PVDEC_SEC && SYSBRG_BRIDGING) */

    return IMG_SUCCESS;
}

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              VDECFW_InspectPvdecFirmware

 ******************************************************************************/
IMG_RESULT VDECFW_InspectPvdecFirmware(
    IMG_PUINT8 pui8Buffer,
    IMG_UINT32 ui32Size,
    IMG_BOOL   bSecure
)
{
#ifdef IMG_DEBUG_FLAG
    IMG_PUINT32 pui32Buf = (IMG_PUINT32)pui8Buffer;
    // check against blob size and creation time
    const IMG_UINT32 ui32BlobSize = pui32Buf[1];
    const IMG_UINT32 ui32TimeStamp = pui32Buf[3];

    // Blob size does not include header section (16 bytes)
    IMG_ASSERT(ui32Size  == (ui32BlobSize + 4 * sizeof(IMG_UINT32)));

    if (!bSecure)
    {
        // unlikely
        IMG_ASSERT(ui32BlobSize == PVDEC_FULL_BIN_BLOB_SIZE);

        // check when the firmware was created
        IMG_ASSERT(ui32TimeStamp == PVDEC_FULL_BIN_TIME_STAMP);
        if (ui32TimeStamp != PVDEC_FULL_BIN_TIME_STAMP)
        {
            return IMG_ERROR_FATAL;
        }
    }
    else
    {
        // unlikely
        IMG_ASSERT(ui32BlobSize == PVDEC_SEC_BIN_BLOB_SIZE);

        // check when the firmware was created
        IMG_ASSERT(ui32TimeStamp == PVDEC_SEC_BIN_TIME_STAMP);
        if (ui32TimeStamp != PVDEC_SEC_BIN_TIME_STAMP)
        {
            return IMG_ERROR_FATAL;
        }

    }
#endif

    return IMG_SUCCESS;
}
#endif /* VDEC_USE_PVDEC_SEC */

/* EOF */
