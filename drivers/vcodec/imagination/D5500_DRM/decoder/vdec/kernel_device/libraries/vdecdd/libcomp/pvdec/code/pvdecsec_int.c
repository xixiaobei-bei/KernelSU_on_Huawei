/*!
 *****************************************************************************
 *
 * @File       pvdecsec_int.c
 * @Title      Low-level PVDEC interface component
 * @Description    This file contains functions to interface with an PVDEC core.
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

#include "img_include.h"
#include "img_defs.h"
#include "lst.h"
#include "talmmu_api.h"
#include "pvdecsec_int.h"
#include "pvdec_int.h"
#include "pvdec_msg.h"
#include "pvdec_ctx.h"
#include "pvdec_ext_reg.h"
#include "pvdec_io.h"
#include "devaio.h"
#include "vdecdd_mmu.h"

#include "tal.h"

#include "hwctrl_api.h"
#include "sysos_api_km.h"
#include "report_api.h"
#include "dbgopt_api_km.h"

#include "hwdefs/pvdec_core_regs.h"
#include "hwdefs/img_video_bus4_mmu_regs.h"
#include "hwdefs/dmac_regs.h"

#ifdef PVDECSEC_FAKE_SECURITY
#include "hwdefs/pvdec_test_regs.h"
#endif

#include "secureapi_ree.h"
/*
******************************************************************************

 @Function              pvdecsec_switch2NonSecure

 @Description

 Switches callbacks to non secure wrapper

******************************************************************************/
static IMG_VOID pvdecsec_switch2NonSecure(
    PVDEC_sContext   * psPvdecContext
)
{
    HWCTRL_sFunctions * psVxdFuncs;

    IMG_ASSERT(psPvdecContext);

    psVxdFuncs = psPvdecContext->psVxdFuncs;
    IMG_ASSERT(psVxdFuncs);

    psVxdFuncs->pfnInitialise          = PVDEC_Initialise;
    psVxdFuncs->pfnDeInitialise        = PVDEC_DeInitialise;
    psVxdFuncs->pfnFlushMmuCache       = PVDEC_FlushMmuCache;

    PVDECIO_SetIOBypass(psPvdecContext->hDev, IMG_FALSE);
}

#ifdef PVDECSEC_FAKE_SECURITY
/*!
******************************************************************************

 @Function              pvdecsec_enableFakeSecurity

******************************************************************************/
static IMG_RESULT pvdecsec_enableFakeSecurity(
    PVDEC_sContext  * psPvdecContext
)
{
    IMG_UINT32 ui32Reg = 0;

    IMG_ASSERT(psPvdecContext);
    /* Indicate no secure mode.
     * From now on, all forbidden transactions are ignored */
    REGIO_WRITE_FIELD_LITE(ui32Reg, PVDEC_TEST, PROT_CONTROL_0,
                           AXI_ARPROT, 2);
    REGIO_WRITE_FIELD_LITE(ui32Reg, PVDEC_TEST, PROT_CONTROL_0,
                           AXI_AWPROT, 2);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, PROT_CONTROL_0, ui32Reg);
    REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING, "Entering secure mode");

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              pvdecsec_disableFakeSecurity

******************************************************************************/
static IMG_RESULT pvdecsec_disableFakeSecurity(
    PVDEC_sContext  * psPvdecContext
)
{
    IMG_UINT32 ui32Reg = 0;

    IMG_ASSERT(psPvdecContext);
    /* Indicate secure mode.
     * From now on, all forbidden transactions are accepted */
    REGIO_WRITE_FIELD_LITE(ui32Reg, PVDEC_TEST, PROT_CONTROL_0,
                           AXI_ARPROT, 0);
    REGIO_WRITE_FIELD_LITE(ui32Reg, PVDEC_TEST, PROT_CONTROL_0,
                           AXI_AWPROT, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, PROT_CONTROL_0, ui32Reg);
    REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING, "Exiting secure mode");

    return IMG_SUCCESS;
}
#endif

/*!
******************************************************************************

 @Function              pvdecsec_confMMUTiling

******************************************************************************/
static IMG_RESULT
pvdecsec_confMMUTiling(
    PVDEC_sContext  * psPvdecContext,
    VDECFW_sCoreInitData  * psInitConfig
)
{
    IMG_UINT32 ui32Result, ui32Reg, ui32NumTiledHeaps;
    MMU_sHeapTilingInfo asTilingInfo[MMU_HEAP_MAX];
    IMG_UINT8 ui8i;
    VDEC_eTileScheme eTileScheme;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psInitConfig);

    eTileScheme = psPvdecContext->eTileScheme == VDEC_TS_NONE
        ? VDEC_TS0_256x16 : psPvdecContext->eTileScheme;

    ui32Result = PVDEC_GetTiledHeaps(eTileScheme, &ui32NumTiledHeaps, asTilingInfo);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    if(ui32NumTiledHeaps > MSVDX_CORE_CR_MMU_TILE_NO_ENTRIES)
    {
        return IMG_ERROR_GENERIC_FAILURE;
    }

    /* MMU_CONTROL0 is set only here */
    ui32Reg = 0;
    REGIO_WRITE_FIELD_LITE(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL0,
        MMU_TILING_SCHEME, eTileScheme == VDEC_TS0_256x16 ? 0 : 1);
    if (psPvdecContext->sProps.bMmuSupportStridePerContext)
    {
        REGIO_WRITE_FIELD_LITE(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL0,
            USE_TILE_STRIDE_PER_CONTEXT, 1);
    }

    psInitConfig->sMmuConfig.ui32MmuControl0 = ui32Reg;

    for (ui8i = 0; ui8i < ui32NumTiledHeaps; ui8i++)
    {
        psInitConfig->sMmuConfig.ui32MmuTileMinAddr[ui8i] =
            asTilingInfo[ui8i].ui32StartOffset;
        psInitConfig->sMmuConfig.ui32MmuTileMaxAddr[ui8i] =
            asTilingInfo[ui8i].ui32StartOffset + asTilingInfo[ui8i].ui32Size - 1;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDECSEC_Initialise

******************************************************************************/
IMG_RESULT PVDECSEC_Initialise(
    const IMG_HANDLE        hVxd,
    VDECFW_sCoreInitData  * psInitConfig,
    const IMG_BOOL          bSecureStream
)
{
    PVDEC_sContext                * psPvdecContext = (PVDEC_sContext *)hVxd;
    VXDIO_sPtdInfo                * psPtdInfo;
    VXDIO_sDdBufInfo              * psFwBufInfo;
    VDECFW_sFirmwareBinInfo       * psFirmwareInfo;
    IMG_UINT32                      ui32Result;
    VXD_SECURE_sPowerOnArgs         sPowerOnArgs;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psInitConfig);
    if (psPvdecContext == IMG_NULL ||
        psInitConfig == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and init config must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psPtdInfo = &psPvdecContext->sPtdInfo;
    psFwBufInfo = psPvdecContext->psFwBufInfo;
    psFirmwareInfo = psPvdecContext->psFirmwareInfo;

    if (bSecureStream == IMG_FALSE)
    {
        // Try to switch to non secure mode
        pvdecsec_switch2NonSecure(psPvdecContext);
        REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
        "Trying non secure mode !");

#ifdef PVDECSEC_FAKE_SECURITY
        pvdecsec_disableFakeSecurity(psPvdecContext);
#endif
       return PVDEC_Initialise(hVxd, psInitConfig, bSecureStream);
    }

#ifdef PVDECSEC_FAKE_SECURITY
    pvdecsec_disableFakeSecurity(psPvdecContext);
#endif
    ui32Result = PVDECIO_PrepareFirmware(psPvdecContext->hDev,
                                         IMG_NULL);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    sPowerOnArgs.ui64Ptd = psPtdInfo->ui32PtdPhysAddr;
#ifdef PVDEC_REG_FW_UPLOAD
    sPowerOnArgs.ui64SecFwBufVirt = psFwBufInfo->pvCpuVirt;
#else
    sPowerOnArgs.ui64SecFwBufVirt = psFwBufInfo->ui32DevVirt;
#endif
    sPowerOnArgs.ui32FwCoreSize = psFirmwareInfo->ui32TextDmaSize;

    ui32Result = SECURE_REE_SendMessage(psPvdecContext->ui32SecureId,
                                        (IMG_BYTE *)&sPowerOnArgs,
                                        sizeof(sPowerOnArgs),
                                        ENDPOINT_POWER_ON);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Power on failed");
    }

#ifdef PVDEC_REG_FW_UPLOAD
    /* Write virtual address of Blob into DMAC_SETUP,
     * as firmware reads it and uses to load modules. */
    PVDECIO_WriteRegister(psPvdecContext->hDev, REGION_PVDEC_PIXEL_DMA_REGSPACE,
                DMAC_DMAC_SETUP_OFFSET, psFwBufInfo->ui32DevVirt, -1, 0);
#endif

#ifdef PVDECSEC_FAKE_SECURITY
    pvdecsec_enableFakeSecurity(psPvdecContext);
#endif
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
               "PVDECSEC TEE runtime error !");
        return IMG_ERROR_NOT_SUPPORTED;
    }

    // Configure PVDEC MMU tiling
    ui32Result = pvdecsec_confMMUTiling(psPvdecContext, psInitConfig);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to configure MMU tiling.");
        return ui32Result;
    }

#if defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)
    PVDECIO_EnableDebugFifo(psPvdecContext->hDev);
#endif

    psPvdecContext->bInitialised = IMG_TRUE;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDECSEC_DeInitialise

******************************************************************************/
IMG_RESULT
PVDECSEC_DeInitialise(
    const IMG_HANDLE  hVxd
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32  ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    if (!psPvdecContext->bInitialised)
    {
        return IMG_SUCCESS;
    }

#if defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)
    /* Check debug FIFO before reset */
    {
        IMG_UINT32 ui32FifoSize, ui32FifoCtrl = 0;

        PVDECIO_HandleDebugFifo(psPvdecContext->hDev);
        ui32FifoSize = PVDECIO_GetDebugFifoSize(psPvdecContext->hDev);
        REGIO_WRITE_FIELD_LITE(ui32FifoCtrl, PVDEC_CORE,
            CR_PROC_DEBUG_FIFO_CTRL_0, CR_PROC_DEBUG_FIFO_COUNT, ui32FifoSize);
        REGIO_WRITE_FIELD_LITE(ui32FifoCtrl, PVDEC_CORE,
            CR_PROC_DEBUG_FIFO_CTRL_0, CR_PROC_DEBUG_FIFO_SIZE, ui32FifoSize);

        /* Wait for empty fifo */
        ui32Result = PVDECIO_Poll(psPvdecContext->hDev, REGION_PVDEC_CORE_REGSPACE,
                                  PVDEC_CORE_CR_PROC_DEBUG_FIFO_CTRL_0_OFFSET,
                                  ui32FifoCtrl,
                                  0xFFFFFFFF,
                                  VXDIO_POLL_EQUAL);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        PVDECIO_DisableDebugFifo(psPvdecContext->hDev);
    }
#endif /* defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING) */

    /* Reset PANIC indicator. */
    psPvdecContext->bFWPanic = IMG_FALSE;

#ifdef PVDECSEC_FAKE_SECURITY
    pvdecsec_disableFakeSecurity(psPvdecContext);
#endif
    // Disable the clocks
    ui32Result = SECURE_REE_SendMessage(psPvdecContext->ui32SecureId,
                                        IMG_NULL,
                                        0,
                                        ENDPOINT_POWER_OFF);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Power of failed");
    }
#ifdef PVDECSEC_FAKE_SECURITY
    pvdecsec_enableFakeSecurity(psPvdecContext);
#endif

    {
    IMG_UINT32 ui32i;
    /* Clear comms header in VLR */
    for (ui32i=0; ui32i<COMMS_HEADER_SIZE; ui32i++)
    {
        PVDECIO_WriteRegister(psPvdecContext->hDev, REGION_PVDEC_VLR_REGSPACE,
                              ui32i*sizeof(IMG_UINT32), 0, -1, 0xFF);
    }
    }

    psPvdecContext->bInitialised = IMG_FALSE;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              pvdecsec_GetCoreProperties

******************************************************************************/
static IMG_RESULT pvdecsec_GetCoreProperties(
    PVDEC_sContext    * psPvdecContext,
    VXD_sCoreProps    * psProps,
    IMG_UINT32 ui32CoreRev,
    IMG_UINT32 ui32PvdecCoreId,
    IMG_UINT32 ui32MmuConfig0,
    IMG_UINT32 ui32MmuConfig1,
    IMG_UINT32 *paui32PixelPipeCfg,
    IMG_UINT32 *paui32PixelMiscCfg,
    IMG_UINT32 ui32PixelMaxFrameCfg
)
{
    IMG_UINT32  ui32GroupId;
    IMG_UINT32  ui32CoreId;
    IMG_UINT32  ui32CoreConfig;
    IMG_UINT32  ui32ExtendedAddressRange;
    IMG_UINT8   ui8GroupSize;

    PVDEC_sCoreRev sCoreRev;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psProps);
    IMG_ASSERT(paui32PixelPipeCfg);
    IMG_ASSERT(paui32PixelMiscCfg);

    /* PVDEC Core Revision Information */
    sCoreRev.ui32MajRev =
        REGIO_READ_FIELD(ui32CoreRev, PVDEC_CORE, CR_PVDEC_CORE_REV, CR_PVDEC_MAJOR_REV);
    sCoreRev.ui32MinRev =
        REGIO_READ_FIELD(ui32CoreRev, PVDEC_CORE, CR_PVDEC_CORE_REV, CR_PVDEC_MINOR_REV);
    sCoreRev.ui32MaintRev =
        REGIO_READ_FIELD(ui32CoreRev, PVDEC_CORE, CR_PVDEC_CORE_REV, CR_PVDEC_MAINT_REV);

    /* Core ID */
    ui32GroupId =
        REGIO_READ_FIELD(ui32PvdecCoreId, PVDEC_CORE, CR_PVDEC_CORE_ID, CR_GROUP_ID);
    ui32CoreId =
        REGIO_READ_FIELD(ui32PvdecCoreId, PVDEC_CORE, CR_PVDEC_CORE_ID, CR_CORE_ID);

    // Ensure that the core is IMG Video Decoder (PVDEC).
    if (ui32GroupId != 3 || ui32CoreId != 3)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
            "IMG Video Decoder (VXD) not detected");
        return IMG_ERROR_DEVICE_NOT_FOUND;
    }

    ui32CoreConfig = REGIO_READ_FIELD(ui32PvdecCoreId,
        PVDEC_CORE, CR_PVDEC_CORE_ID, CR_PVDEC_CORE_CONFIG);

    VDEC_BZERO(psProps);

    // Construct core version name.
    sprintf(psProps->acVersion, "%d.%d.%d",
            sCoreRev.ui32MajRev, sCoreRev.ui32MinRev, sCoreRev.ui32MaintRev);

    psProps->bMmuSupportStridePerContext = REGIO_READ_FIELD(ui32MmuConfig1, IMG_VIDEO_BUS4_MMU,
        MMU_CONFIG1, SUPPORT_STRIDE_PER_CONTEXT) == 1 ? IMG_TRUE : IMG_FALSE;

    psProps->bMmuSupportSecure = REGIO_READ_FIELD(ui32MmuConfig1, IMG_VIDEO_BUS4_MMU,
        MMU_CONFIG1, SUPPORT_SECURE) == 1 ? IMG_TRUE : IMG_FALSE;

    ui32ExtendedAddressRange =
        REGIO_READ_FIELD(ui32MmuConfig0, IMG_VIDEO_BUS4_MMU, MMU_CONFIG0, EXTENDED_ADDR_RANGE);
    switch (ui32ExtendedAddressRange)
    {
        case 0:
            psProps->eMmuType = MMU_TYPE_32BIT;
            break;
        case 4:
            psProps->eMmuType = MMU_TYPE_36BIT;
            break;
        case 8:
            psProps->eMmuType = MMU_TYPE_40BIT;
            break;
        default:
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
                   "Unsupported MMU address extension: %d",
                   ui32ExtendedAddressRange);
            IMG_FREE(psPvdecContext);
            psPvdecContext = IMG_NULL;
            return IMG_ERROR_NOT_SUPPORTED;
    }

    ui8GroupSize = REGIO_READ_FIELD(ui32MmuConfig0, IMG_VIDEO_BUS4_MMU, MMU_CONFIG0, GROUP_OVERRIDE_SIZE);

    psProps->ui32NumEntropyPipes = ui32CoreConfig & 0xF;
    psProps->ui32NumPixelPipes = ui32CoreConfig >> 4 & 0xF;

    REPORT(REPORT_MODULE_VXDIO, REPORT_INFO,
           "PVDEC SEC revision %08x detected, id %08x."
           " Found %d entropy pipe(s), %d pixel pipe(s), %d group size",
           ui32CoreRev,
           ui32CoreId,
           psProps->ui32NumEntropyPipes,
           psProps->ui32NumPixelPipes, ui8GroupSize);

    PVDEC_InitCoreProperties(psProps,
                             &sCoreRev,
                             ui32CoreConfig,
                             ui32PixelMaxFrameCfg,
                             paui32PixelPipeCfg,
                             paui32PixelMiscCfg);

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDECSEC_Create

******************************************************************************/
IMG_RESULT PVDECSEC_Create(
    const VDECDD_sDdDevConfig  * psDdDevConfig,
    VXD_sCoreProps             * psCoreProps,
    HWCTRL_sRendec             * psRendec,
    IMG_HANDLE                 * phPvdecContext,
    HWCTRL_sFunctions          * psVxdFuncs
)
{
    PVDEC_sContext        * psPvdecContext = IMG_NULL;
    IMG_UINT32              ui32Result;

    IMG_ASSERT(psDdDevConfig);
    IMG_ASSERT(psCoreProps);
    IMG_ASSERT(psRendec);
    IMG_ASSERT(phPvdecContext);
    IMG_ASSERT(psVxdFuncs);

    if (phPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "A pointer to PVDEC context handle must be provided");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    if (psDdDevConfig == IMG_NULL ||
        psCoreProps == IMG_NULL ||
        psRendec == IMG_NULL ||
        psVxdFuncs == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameter!");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    VDEC_MALLOC(psPvdecContext);
    IMG_ASSERT(psPvdecContext);
    if(psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
            "Failed to allocate memory for PVDEC context");
        return IMG_ERROR_OUT_OF_MEMORY;
    }
    VDEC_BZERO(psPvdecContext);

    psPvdecContext->eDecodeLevel = psDdDevConfig->eDecodeLevel;
    psPvdecContext->eTileScheme = psDdDevConfig->eTileScheme;

    if (psDdDevConfig->eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
    {
        VXD_SECURE_sVxdCorePropsArgs sProps = { 0 };
        ui32Result = PVDECIO_Initialise(IMG_TRUE, /* own restricted IO calls - will be handled by TEE */
                                        &psPvdecContext->hDev);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

#ifdef PVDECSEC_FAKE_SECURITY
        pvdecsec_disableFakeSecurity(psPvdecContext);
#endif

        ui32Result = SECURE_REE_GetId(VXD0, &psPvdecContext->ui32SecureId);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to establish communications with a secure driver.");
            return ui32Result;
        }

        // ptd & firmware buff address set to zero for the very first time, only read props
        ui32Result = SECURE_REE_SendMessage(psPvdecContext->ui32SecureId,
                                            (IMG_BYTE *)&sProps,
                                            sizeof(sProps),
                                            ENDPOINT_GET_PROPS);
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
#ifdef PVDECSEC_FAKE_SECURITY
        pvdecsec_enableFakeSecurity(psPvdecContext);
#endif
        if (ui32Result != IMG_SUCCESS)
        {
            // clean up this instance
            PVDEC_Destroy((IMG_HANDLE)psPvdecContext);

            ui32Result = IMG_ERROR_FATAL;
            goto error;
        }
        ui32Result = pvdecsec_GetCoreProperties(psPvdecContext, &psPvdecContext->sProps,
                                                sProps.ui32CoreRev,
                                                sProps.ui32PvdecCoreId,
                                                sProps.ui32MmuConfig0,
                                                sProps.ui32MmuConfig1,
                                                sProps.aui32PixelPipeCfg,
                                                sProps.aui32PixelMiscCfg,
                                                sProps.ui32PixelMaxFrameCfg);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }
        HWCTRL_PrintMSVDXProps(&psPvdecContext->sProps);
    }

    /* Setup common */
    PVDEC_Setup(psDdDevConfig, psPvdecContext, psVxdFuncs);

    *psCoreProps = psPvdecContext->sProps;
    *psRendec    = psPvdecContext->sRendec;

    *phPvdecContext = psPvdecContext;

    /* Override callbacks */
    psVxdFuncs->pfnInitialise          = PVDECSEC_Initialise;
    psVxdFuncs->pfnDeInitialise        = PVDECSEC_DeInitialise;
    psVxdFuncs->pfnFlushMmuCache       = PVDECSEC_FlushMmuCache;

    return IMG_SUCCESS;

error:
    if(psPvdecContext)
    {
        if(psPvdecContext->ui32SecureId)
        {
            SECURE_REE_ReleaseId(psPvdecContext->ui32SecureId);
            psPvdecContext->ui32SecureId = 0;
        }

        IMG_FREE(psPvdecContext);
        psPvdecContext = IMG_NULL;
        *phPvdecContext = IMG_NULL;;
    }

    return ui32Result;
}

/*!
******************************************************************************

 @Function              PVDECSEC_FlushMmuCache

******************************************************************************/
IMG_RESULT PVDECSEC_FlushMmuCache(
    const IMG_HANDLE  hVxd
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    /* MMU is now flushed in the firmware during message reception
     * This applies when secure firmware is being used */
    psPvdecContext->bMMUFlushNeeded = IMG_TRUE;

    return IMG_SUCCESS;
}
/* EOF */
