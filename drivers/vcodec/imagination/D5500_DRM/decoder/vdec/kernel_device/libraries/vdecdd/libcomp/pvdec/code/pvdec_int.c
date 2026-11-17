/*************************************************************************/ /*!
@File           pvdec_int.c
@Title          Low-level PVDEC interface component
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    This file contains functions to interface with an PVDEC core.
@License        Strictly Confidential.
*/ /**************************************************************************/
#include "img_include.h"
#include "img_defs.h"
#include "lst.h"
#include "talmmu_api.h"
#ifdef VDEC_USE_PVDEC_SEC
#include "pvdecsec_int.h"
#ifdef SYSBRG_BRIDGING
#include "secureapi_ree.h"
#endif
#endif
#include "hwctrl_api.h"
#include "pvdec_int.h"
#include "pvdec_msg.h"
#include "pvdec_ctx.h"
#include "pvdec_ext_reg.h"
#include "vdecdd_mmu.h"

#include "tal.h"

#include "sysos_api_km.h"
#include "report_api.h"
#include "dbgopt_api_km.h"

#include "tal.h"
#include "pvdec_io.h"
#include "devaio.h"

#include "reg_io2.h"

#include "hwdefs/msvdx_mtx_regs.h"
#include "hwdefs/pvdec_core_regs.h"
#include "hwdefs/pvdec_pixel_regs.h"
#include "hwdefs/img_video_bus4_mmu_regs.h"
#include "hwdefs/pvdec_entropy_regs.h"
#include "hwdefs/pvdec_test_regs.h"

#define GET_BITS(v, lb, n)       ((v >> lb) & ((1 << n) - 1))
#define IS_PVDEC_PIPELINE(std)   ( std == VDEC_STD_HEVC ? IMG_TRUE : IMG_FALSE )

/*!
******************************************************************************
 PVDEC core version information (for use of revision macro).
 NOTE: It is assumed that all cores on a device are the same revision.
******************************************************************************/
static IMG_UINT32  ui32MajRev;    /*!< VXD Core major revision.        */
static IMG_UINT32  ui32MinRev;    /*!< VXD Core minor revision.        */
static IMG_UINT32  ui32MaintRev;  /*!< VXD Core maintenance revision.  */


/*!
******************************************************************************
 This array defines the properties of each video standard for various core
 versions (and on). TRM 1.1: 4k for all but HEVC and JPEG

 D5500      7.0.0

*****************************************************************************/
static VXD_sVidStdProps asStdProps[] =
{
    { VDEC_STD_MPEG2,       CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_MPEG4,       CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_H263,        CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_H264,        CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0x10000,  8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_VC1,         CORE_REVISION(7,0,0), 80, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_AVS,         CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_REAL,        CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_JPEG,        CORE_REVISION(7,0,0), 64, 16, 32768, 32768, 0,      8, 8, PIXEL_FORMAT_444 },

    { VDEC_STD_VP6,         CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_VP8,         CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_SORENSON,    CORE_REVISION(7,0,0), 64, 16, 4096, 4096, 0,        8, 8, PIXEL_FORMAT_420 },

    { VDEC_STD_HEVC,        CORE_REVISION(7,0,0), 64, 16, 8192, 8192, 0,        8, 8, PIXEL_FORMAT_420 },
};

#if defined VDEC_USE_PVDEC_SEC && defined SYSBRG_BRIDGING
/*
******************************************************************************

 @Function              pvdec_switch2Secure

 @Description

 Switches callbacks to secure wrapper

******************************************************************************/
static IMG_VOID pvdec_switch2Secure(
    PVDEC_sContext   * psPvdecContext
)
{
    HWCTRL_sFunctions * psVxdFuncs;

    IMG_ASSERT(psPvdecContext);

    psVxdFuncs = psPvdecContext->psVxdFuncs;

    IMG_ASSERT(psVxdFuncs);

    psVxdFuncs->pfnInitialise          = PVDECSEC_Initialise;
    psVxdFuncs->pfnDeInitialise        = PVDECSEC_DeInitialise;
    psVxdFuncs->pfnFlushMmuCache       = PVDECSEC_FlushMmuCache;

    PVDECIO_SetIOBypass(psPvdecContext->hDev, IMG_TRUE);
}
#endif

#ifndef SYSBRG_BRIDGING
/*
******************************************************************************

 @Function              pvdec_CheckInterruptFunc

 @Description

 Check interrupt function.

******************************************************************************/
static IMG_BOOL pvdec_CheckInterruptFunc(
    IMG_HANDLE  ui32MemSpaceId,
    IMG_VOID *  pCallbackParameter
)
{
    IMG_UINT32        ui32IntStatus = 0;
    PVDEC_sContext  * psContext = (PVDEC_sContext *)pCallbackParameter;

    IMG_ASSERT(psContext);

    /* Disable interrupts.  */
    SYSOSKM_DisableInt();

    /* Read the status register. */
    ui32IntStatus = REGIO_READ_REGISTER(psContext, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_STATUS);

#if defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)
    PVDECIO_HandleDebugFifo(psContext->hDev);
#endif

    if (psContext->bCSim)
    {
        PVDECIO_TriggerSim(psContext->hDev);
    }

    /* Re enable interrupts */
    SYSOSKM_EnableInt();

    return (ui32IntStatus != 0);
}
#endif /* not SYSBRG_BRIDGING */


/*!
******************************************************************************

 @Function              PVDEC_SendFirmwareMessage

******************************************************************************/
IMG_RESULT PVDEC_SendFirmwareMessage(
    const IMG_HANDLE    hVxd,
    const IMG_VOID    * psMsgHdr
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32        ui32Result;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psMsgHdr);
    if (psPvdecContext == IMG_NULL ||
        psMsgHdr == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and message header must be provided");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    ui32Result = PVDECIO_SendFirmwareMessage(psPvdecContext->hDev,
                                             psMsgHdr);

    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    return IMG_SUCCESS;

error:
    return ui32Result;
}


/*!
******************************************************************************

 @Function              PVDEC_GetCoreState

******************************************************************************/
IMG_RESULT PVDEC_GetCoreState(
    const IMG_HANDLE    hVxd,
    VXDIO_sState      * psState
)
{
    PVDEC_sContext       * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_RESULT             ui32Result;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psState);
    if (psPvdecContext == IMG_NULL ||
        psState == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and state struct must be provided");
        ui32Result = IMG_ERROR_INVALID_PARAMETERS;
        goto error;
    }

    ui32Result = PVDECIO_GetCoreState(psPvdecContext->hDev,
                                      psPvdecContext->sProps.ui32NumPixelPipes,
                                      psPvdecContext->sProps.ui32NumEntropyPipes,
                                      psState);

    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        goto error;
    }

    /* Add firmware PANIC indicator. */
    psState->sFwState.bPanic = psPvdecContext->bFWPanic;

    return IMG_SUCCESS;

error:
    return ui32Result;
}


/*!
******************************************************************************

 @Function              pvdec_PrepareMTX

******************************************************************************/
static IMG_VOID pvdec_PrepareMTX(
    PVDEC_sContext  * psPvdecContext,
    VXDIO_sPtdInfo  * psPtdInfo,
    MMU_eMmuType      eMmuType
)
{
    IMG_UINT32  ui32Reg;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psPtdInfo);
    /* Reset MTX */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, MTX_CORE, CR_MTX_SOFT_RESET, MTX_RESET, 1);
    REGIO_WRITE_REGISTER(psPvdecContext, MTX_CORE, CR_MTX_SOFT_RESET, ui32Reg);

    /* always use bank 0 */
    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_BANK_INDEX, 0);

    /* Set base address of PTD */
#ifdef SYSBRG_BRIDGING
    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_DIR_BASE_ADDR, psPtdInfo->ui32PtdPhysAddr);
#else /* not SYSBRG_BRIDGING */
    TALINTVAR_WriteToReg32(psPvdecContext->hVideoBus4RegSpace, IMG_VIDEO_BUS4_MMU_MMU_DIR_BASE_ADDR_OFFSET,
            psPtdInfo->hPtdMemSpace, psPtdInfo->ui32IntRegNum);
#endif /* not SYSBRG_BRIDGING */

    /* clear the bypass bits */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_ADDRESS_CONTROL, UPPER_ADDRESS_FIXED, 0 );
    REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_ADDRESS_CONTROL, MMU_ENABLE_EXT_ADDRESSING,
            (eMmuType == MMU_TYPE_40BIT) ? 1 : 0 );
    REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_ADDRESS_CONTROL, MMU_BYPASS, 0 );
    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_ADDRESS_CONTROL, ui32Reg);

    //        ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1);
    //        REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_PAUSE_CLEAR, 1);
    //        REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, ui32Reg);
}


/*!
******************************************************************************

 @Function              pvdec_Poll

******************************************************************************/
static IMG_RESULT pvdec_Poll(
    const PVDEC_sContext  * psPvdecContext,
    IMG_UINT32              ui32MemRegion,
    IMG_UINT32              ui32Offset,
    IMG_UINT32              ui32RequValue,
    IMG_UINT32              ui32Enable
)
{
    IMG_UINT32  ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psPvdecContext);

    ui32Result = PVDECIO_Poll(psPvdecContext->hDev,
                              ui32MemRegion,
                              ui32Offset,
                              ui32RequValue,
                              ui32Enable,
                              VXDIO_POLL_EQUAL);

    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    return ui32Result;
}

/*!
******************************************************************************

 @Function              pvdec_ConfigureMemStaller

******************************************************************************/
static IMG_VOID
pvdec_ConfigureMemStaller(
    PVDEC_sContext  * psPvdecContext
)
{
    IMG_ASSERT(psPvdecContext);

    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, MEM_READ_LATENCY, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, MEM_WRITE_RESPONSE_LATENCY, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, MEM_CTRL, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, RAND_STL_MEM_CMD_CONFIG, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, RAND_STL_MEM_WDATA_CONFIG, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, RAND_STL_MEM_WRESP_CONFIG, 0);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_TEST, RAND_STL_MEM_RDATA_CONFIG, 0);
}

/*!
******************************************************************************

 @Function              pvdec_confMMUTiling

******************************************************************************/
static IMG_RESULT
pvdec_confMMUTiling(
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

    ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL0);
    REGIO_WRITE_FIELD_LITE(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL0,
        MMU_TILING_SCHEME, eTileScheme == VDEC_TS0_256x16 ? 0 : 1);
    if (psPvdecContext->sProps.bMmuSupportStridePerContext)
    {
        REGIO_WRITE_FIELD_LITE(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL0,
            USE_TILE_STRIDE_PER_CONTEXT, 1);
    }

    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL0, ui32Reg);
#ifdef VDEC_USE_PVDEC_SEC
    psInitConfig->sMmuConfig.ui32MmuControl0 = ui32Reg;
#endif

    for (ui8i = 0; ui8i < ui32NumTiledHeaps; ui8i++)
    {
        REGIO_WRITE_OFFSET_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU,
            MMU_TILE_MIN_ADDR, ui8i, asTilingInfo[ui8i].ui32StartOffset);
        REGIO_WRITE_OFFSET_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU,
            MMU_TILE_MAX_ADDR, ui8i, asTilingInfo[ui8i].ui32StartOffset +
            asTilingInfo[ui8i].ui32Size - 1);
#ifdef VDEC_USE_PVDEC_SEC
        psInitConfig->sMmuConfig.ui32MmuTileMinAddr[ui8i] =
            asTilingInfo[ui8i].ui32StartOffset;
        psInitConfig->sMmuConfig.ui32MmuTileMaxAddr[ui8i] =
            asTilingInfo[ui8i].ui32StartOffset + asTilingInfo[ui8i].ui32Size - 1;
#endif
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              pvdec_CheckMMUOutstandingRequests

******************************************************************************/
static IMG_RESULT pvdec_CheckMMUOutstandingRequests (
    PVDEC_sContext * psPvdecContext,
    IMG_UINT32 ui32Count,
    IMG_UINT32 ui32MaxChecks
)
{
    IMG_UINT32 ui32Reg, ui32I, ui32CheckCounter = 0;

    for (ui32I = 0; ui32I < ui32MaxChecks; ui32I++)
    {
        /* Read TAG_OUTSTANDING */
        ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_MEM_REQ);
        ui32Reg = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_MEM_REQ,
                                   TAG_OUTSTANDING);
        if (ui32Reg != 0)
        {
            continue;
        }

        /* Read READ_WORDS_OUTSTANDING */
        ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU,
                                      MMU_MEM_EXT_OUTSTANDING);
        ui32Reg = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU,
                                   MMU_MEM_EXT_OUTSTANDING, READ_WORDS_OUTSTANDING);
        if (ui32Reg == 0)
        {
            ui32CheckCounter++;
            if (ui32CheckCounter == ui32Count)
            {
                break;
            }
        }
        else /* reset the counter and continue */
        {
            ui32CheckCounter = 0;
        }
    }

    if (ui32CheckCounter != ui32Count)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
               "Check MMU outstanding requests error!");
        return IMG_ERROR_FATAL;
    }

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              pvdec_PvdecReset

******************************************************************************/
static IMG_RESULT pvdec_PvdecReset(
    PVDEC_sContext  * psPvdecContext
)
{
    IMG_UINT32  ui32Reg;
    IMG_RESULT  ui32Result = IMG_SUCCESS;
    IMG_UINT8   ui8Pipe = 1;

    DEBUG_REPORT(REPORT_MODULE_VXDIO, "Doing PVDEC Reset");

    /* Turn all clocks on */
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE,
                         CR_PVDEC_MAN_CLK_ENABLE, 0xffffffff);

    for(ui8Pipe = 1; ui8Pipe <= psPvdecContext->ui32NumPixelPipes; ui8Pipe++)
    {
        REGIO_WRITE_PIPE_REGISTER(psPvdecContext, ui8Pipe, PVDEC_PIXEL,
                                  CR_PIXEL_MAN_CLK_ENABLE, 0xffffffff);
    }

    for(ui8Pipe = 1; ui8Pipe <= psPvdecContext->ui32NumEntropyPipes; ui8Pipe++)
    {
        REGIO_WRITE_PIPE_REGISTER(psPvdecContext, ui8Pipe, PVDEC_ENTROPY,
                                  CR_ENTROPY_MAN_CLK_ENABLE , 0xffffffff);
    }

    /* 1st MMU outstanding requests check */
    (IMG_VOID)pvdec_CheckMMUOutstandingRequests(psPvdecContext, 1000, 2000);

    /* Wait until MMU_SOFT_RESET -> 0 */
    ui32Result = pvdec_Poll(psPvdecContext,
                            REGION_IMG_VIDEO_BUS4_MMU_REGSPACE,
                            IMG_VIDEO_BUS4_MMU_MMU_CONTROL1_OFFSET,
                            0,
                            IMG_VIDEO_BUS4_MMU_MMU_CONTROL1_MMU_SOFT_RESET_MASK);
    IMG_ASSERT((ui32Result == IMG_SUCCESS) || (ui32Result == IMG_ERROR_TIMEOUT));
    if ((ui32Result != IMG_SUCCESS) && (ui32Result != IMG_ERROR_TIMEOUT))
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed poll for MMU soft reset.");
        return ui32Result;
    }

    /* Write 1 to MMU_PAUSE_SET */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_PAUSE_SET, 1);
    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, ui32Reg);

    /* 2nd MMU outstanding requests check */
    (IMG_VOID)pvdec_CheckMMUOutstandingRequests(psPvdecContext, 100, 1000);

    /* Issue software reset for all but core */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, PVDEC_CORE, CR_PVDEC_SOFT_RESET,
                      CR_PVDEC_PIXEL_PROCESSING_SOFT_RESET, 0xFF);
    REGIO_WRITE_FIELD(ui32Reg, PVDEC_CORE, CR_PVDEC_SOFT_RESET,
                      CR_PVDEC_ENTROPY_SOFT_RESET, 0xFF);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_SOFT_RESET, ui32Reg);

    REGIO_READ_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_SOFT_RESET);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_SOFT_RESET, 0);

    /* Write 1 to MMU_PAUSE_CLEAR in MMU_CONTROL1 reg */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_PAUSE_CLEAR, 1);
    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, ui32Reg);
    /* Confirm MMU_PAUSE_SET is cleared */
    ui32Result = pvdec_Poll(psPvdecContext,
                            REGION_IMG_VIDEO_BUS4_MMU_REGSPACE,
                            IMG_VIDEO_BUS4_MMU_MMU_CONTROL1_OFFSET,
                            0,
                            IMG_VIDEO_BUS4_MMU_MMU_CONTROL1_MMU_PAUSE_SET_MASK);
    IMG_ASSERT((ui32Result == IMG_SUCCESS) || (ui32Result == IMG_ERROR_TIMEOUT));
    if ((ui32Result != IMG_SUCCESS) && (ui32Result != IMG_ERROR_TIMEOUT))
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
            "Failed poll waiting for MMU_PAUSE_SET bit to be cleared.");
        return ui32Result;
    }

    /* Issue software reset for MMU */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_SOFT_RESET, 1);
    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1 , ui32Reg);
    /* Wait until MMU_SOFT_RESET -> 0 */
    ui32Result = pvdec_Poll(psPvdecContext,
                            REGION_IMG_VIDEO_BUS4_MMU_REGSPACE,
                            IMG_VIDEO_BUS4_MMU_MMU_CONTROL1_OFFSET,
                            0,
                            IMG_VIDEO_BUS4_MMU_MMU_CONTROL1_MMU_SOFT_RESET_MASK);
    IMG_ASSERT((ui32Result == IMG_SUCCESS) || (ui32Result == IMG_ERROR_TIMEOUT));
    if ((ui32Result != IMG_SUCCESS) && (ui32Result != IMG_ERROR_TIMEOUT))
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed poll for MMU soft reset.");
        return ui32Result;
    }

    /* Issue software reset for entire PVDEC */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, PVDEC_CORE, CR_PVDEC_SOFT_RESET, CR_PVDEC_SOFT_RESET, 0x1);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_SOFT_RESET, ui32Reg);
    /* Wait until PVDEC_SOFT_RESET -> 0 */
    ui32Result = pvdec_Poll(psPvdecContext,
                            REGION_PVDEC_CORE_REGSPACE,
                            PVDEC_CORE_CR_PVDEC_SOFT_RESET_OFFSET,
                            0,
                            PVDEC_CORE_CR_PVDEC_SOFT_RESET_CR_PVDEC_SOFT_RESET_MASK);
    IMG_ASSERT((ui32Result == IMG_SUCCESS) || (ui32Result == IMG_ERROR_TIMEOUT));
    if ((ui32Result != IMG_SUCCESS) && (ui32Result != IMG_ERROR_TIMEOUT))
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
            "Failed poll waiting for reset bits to be set.");
        return ui32Result;
    }

    /* Clear interrupt enabled flag */
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_ENABLE, 0);

    /* Clear any pending interrupt flags */
    ui32Reg = 0;
    REGIO_WRITE_FIELD(ui32Reg, PVDEC_CORE, CR_PVDEC_INTERRUPT_CLEAR, CR_IRQ_CLEAR, 0xFFFF);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_INTERRUPT_CLEAR, ui32Reg);

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_Initialise

******************************************************************************/
IMG_RESULT PVDEC_Initialise(
    const IMG_HANDLE        hVxd,
    VDECFW_sCoreInitData  * psInitConfig,
    const IMG_BOOL          bSecureStream
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32        ui32Result;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psInitConfig);
    if (psPvdecContext == IMG_NULL ||
        psInitConfig == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and init config must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

#if defined VDEC_USE_PVDEC_SEC && defined SYSBRG_BRIDGING
    if (bSecureStream)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
               "Cannot use PVDEC with secure stream !");

        // Try to switch back to secure mode and use TEE
        pvdec_switch2Secure(psPvdecContext);
        return PVDECSEC_Initialise(hVxd, psInitConfig, bSecureStream);
    }
#endif

    // Enable the clocks to allow for reset.
    ui32Result = PVDECIO_EnableCoreAndProcClocks(psPvdecContext->hDev);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    // Reset PVDEC: the core may have been powered down.
    ui32Result = pvdec_PvdecReset(psPvdecContext);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    // Configure PVDEC MMU tiling
    ui32Result = pvdec_confMMUTiling(psPvdecContext, psInitConfig);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to configure MMU tiling.");
        return ui32Result;
    }

    // Enable the clocks since the core could have been powered down.
    ui32Result = PVDECIO_EnableCoreAndProcClocks(psPvdecContext->hDev);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

#if !defined(SECURE_MEDIA_SUPPORT) && (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING))
    PVDECIO_EnableDebugFifo(psPvdecContext->hDev);
#endif

    pvdec_PrepareMTX(psPvdecContext,
                     &psPvdecContext->sPtdInfo,
                     psPvdecContext->sProps.eMmuType);

    /* Load the firmware base component. */
    ui32Result =  PVDECIO_LoadBaseFirmware(psPvdecContext->hDev);

    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
                "Failed loading firmware base component");
        return ui32Result;
    }

#if defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)
    PVDECIO_EnableDebugFifo(psPvdecContext->hDev);
#endif

#ifndef SYSBRG_BRIDGING
    ui32Result = TALINTERRUPT_RegisterCheckFunc(psPvdecContext->hPvdecCoreRegSpace,
                                                pvdec_CheckInterruptFunc,
                                                psPvdecContext);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }
#endif /* not SYSBRG_BRIDGING */

    /* check DBGOPTS for CSim property */
    {
        DBGOPT_sValue sVal;
        if (DBGOPTKM_GetSize("CSim", DBGOPT_TYPE_BOOL) &&
            DBGOPTKM_Get("CSim", IMG_NULL, &sVal))
        {
            psPvdecContext->bCSim = sVal.bVal;
        }
        else
        {
            psPvdecContext->bCSim = IMG_FALSE;
        }
    }

    psPvdecContext->bInitialised = IMG_TRUE;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_DeInitialise

******************************************************************************/
IMG_RESULT
PVDEC_DeInitialise(
    const IMG_HANDLE  hVxd
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32        ui32Result;

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

#if (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING))
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
        pvdec_Poll(psPvdecContext, REGION_PVDEC_CORE_REGSPACE,
                                   PVDEC_CORE_CR_PROC_DEBUG_FIFO_CTRL_0_OFFSET,
                                   ui32FifoCtrl,
                                   0xFFFFFFFF);
        PVDECIO_DisableDebugFifo(psPvdecContext->hDev);
    }
#endif /* (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)) */

    /* Reset the MTX-PVDEC first. */
    ui32Result = pvdec_PvdecReset(psPvdecContext);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    /* Reset PANIC indicator. */
    psPvdecContext->bFWPanic = IMG_FALSE;

#ifndef SYSBRG_BRIDGING
    ui32Result = TALINTERRUPT_UnRegisterCheckFunc(psPvdecContext->hPvdecCoreRegSpace);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }
#endif /* not SYSBRG_BRIDGING */

    ui32Result = PVDECIO_DisableClocks(psPvdecContext->hDev);

    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    psPvdecContext->bInitialised = IMG_FALSE;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDEC_Destroy

******************************************************************************/
IMG_RESULT PVDEC_Destroy(
    const IMG_HANDLE  hVxd
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32        ui32Result = IMG_SUCCESS;;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    PVDEC_DeInitialise(hVxd);

    if (psPvdecContext->eDecodeLevel > VDECDD_DECODELEVEL_DECODER)
    {
        //Using Secure Interface
        ui32Result = PVDECIO_DeInitialise(psPvdecContext->hDev);

#if defined(VDEC_USE_PVDEC_SEC) && defined(SYSBRG_BRIDGING)
        if(psPvdecContext->ui32SecureId)
        {
            SECURE_REE_ReleaseId(psPvdecContext->ui32SecureId);
        }
#endif
        IMG_ASSERT(ui32Result == IMG_SUCCESS);
    }

    IMG_FREE(psPvdecContext);
    psPvdecContext = IMG_NULL;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_InitCoreProperties

******************************************************************************/
IMG_VOID PVDEC_InitCoreProperties(
    VXD_sCoreProps    * psCoreProps,
    PVDEC_sCoreRev    * psCoreRev,
    IMG_UINT32          ui32CoreConfig,
    IMG_UINT32          ui32MaxFrameConfig,
    IMG_UINT32        * aui32PixelPipeConfig,
    IMG_UINT32        * aui32PixelMiscConfig
)
{
    IMG_UINT8    ui8PipeMinus1 = 0;
    IMG_UINT32   i;
    IMG_UINT32   ui32MaxH264HwChromaFormat = 0;
    IMG_UINT32   ui32MaxHEVCHwChromaFormat = 0;
    IMG_UINT32   ui32MaxBitDepthLuma = 0;

    IMG_ASSERT(psCoreProps);
    IMG_ASSERT(psCoreRev);
    IMG_ASSERT(aui32PixelPipeConfig);
    IMG_ASSERT(aui32PixelMiscConfig);
    if (psCoreProps == IMG_NULL ||
        psCoreRev == IMG_NULL ||
        aui32PixelPipeConfig == IMG_NULL ||
        aui32PixelMiscConfig == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters!");
        return;
    }

    // Set global rev info variables used by macros
    ui32MajRev = psCoreRev->ui32MajRev;
    ui32MinRev = psCoreRev->ui32MinRev;
    ui32MaintRev = psCoreRev->ui32MaintRev;

    //////////////////////////////////////
    //  Default settings                //
    //////////////////////////////////////
    for (i = 0; i < sizeof(asStdProps)/sizeof(asStdProps[0]); i++)
    {
        VXD_sVidStdProps *psVidStdProps = &psCoreProps->asVidStdProps[asStdProps[i].eVidStd];
        // Update video standard properties if the core is beyond specified version and
        // the properties are for newer cores than the previous.
        if (FROM_REV(MAJOR_REVISION(asStdProps[i].ui32CoreRev),
                     MINOR_REVISION(asStdProps[i].ui32CoreRev),
                     MAINT_REVISION(asStdProps[i].ui32CoreRev)) &&
            asStdProps[i].ui32CoreRev >= psVidStdProps->ui32CoreRev)
        {
            *psVidStdProps = asStdProps[i];

            if(FROM_REV(8,0,0))
            {
                // override default values with values specified in HW (register does not exist in previous cores)
                if(IS_PVDEC_PIPELINE(asStdProps[i].eVidStd))
                {
                    psVidStdProps->ui32MaxWidth = 2 << REGIO_READ_FIELD(ui32MaxFrameConfig, PVDEC_PIXEL, CR_MAX_FRAME_CONFIG, CR_PVDEC_HOR_MSB);
                    psVidStdProps->ui32MaxHeight = 2 << REGIO_READ_FIELD(ui32MaxFrameConfig, PVDEC_PIXEL, CR_MAX_FRAME_CONFIG, CR_PVDEC_VER_MSB);
                }
                else
                {
                    psVidStdProps->ui32MaxWidth = 2 << REGIO_READ_FIELD(ui32MaxFrameConfig, PVDEC_PIXEL, CR_MAX_FRAME_CONFIG, CR_MSVDX_HOR_MSB);
                    psVidStdProps->ui32MaxHeight = 2 << REGIO_READ_FIELD(ui32MaxFrameConfig, PVDEC_PIXEL, CR_MAX_FRAME_CONFIG, CR_MSVDX_VER_MSB);
                }
            }
        }
    }

    // Populate the core properties.
    if (GET_BITS(ui32CoreConfig, 11, 1))
    {
        psCoreProps->bHdSupport = IMG_TRUE;
    }

    for(ui8PipeMinus1 = 0; ui8PipeMinus1 < psCoreProps->ui32NumPixelPipes; ui8PipeMinus1++)
    {
        IMG_UINT32 ui32CurrentBitDepth = GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 4, 3) + 8;
        IMG_UINT32 ui32CurrentH264HwChromaFormat = GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 0, 2);
        IMG_UINT32 ui32CurrentHEVCHwChromaFormat = GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 2, 2);

        if (GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 7, 1))
        {
            psCoreProps->bHevcRangeExt[ui8PipeMinus1] = IMG_TRUE;
        }

        if (GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 8, 1))
        {
            psCoreProps->abRotationSupport[ui8PipeMinus1] = IMG_TRUE;
        }
        if (GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 9, 1))
        {
            psCoreProps->abScalingSupport[ui8PipeMinus1] = IMG_TRUE;
        }

        psCoreProps->aui32NumStreams[ui8PipeMinus1] = GET_BITS(aui32PixelMiscConfig[ui8PipeMinus1], 12, 2) + 1;

    // Video standards.
        psCoreProps->abMpeg2[ui8PipeMinus1]    = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 0, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abMpeg4[ui8PipeMinus1]    = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 1, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abH264[ui8PipeMinus1]     = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 2, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abVc1[ui8PipeMinus1]      = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 3, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abJpeg[ui8PipeMinus1]     = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 5, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abAvs[ui8PipeMinus1]      = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 7, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abReal[ui8PipeMinus1]     = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 8, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abVp6[ui8PipeMinus1]      = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 9, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abVp8[ui8PipeMinus1]      = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 10, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abSorenson[ui8PipeMinus1] = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 11, 1) ? IMG_TRUE : IMG_FALSE;
        psCoreProps->abHEVC[ui8PipeMinus1]     = GET_BITS(aui32PixelPipeConfig[ui8PipeMinus1], 22, 1) ? IMG_TRUE : IMG_FALSE;

        ui32MaxBitDepthLuma = (ui32MaxBitDepthLuma > ui32CurrentBitDepth ? ui32MaxBitDepthLuma : ui32CurrentBitDepth);
        ui32MaxH264HwChromaFormat = (ui32MaxH264HwChromaFormat > ui32CurrentH264HwChromaFormat ? ui32MaxH264HwChromaFormat : ui32CurrentH264HwChromaFormat);
        ui32MaxHEVCHwChromaFormat = (ui32MaxHEVCHwChromaFormat > ui32CurrentHEVCHwChromaFormat ? ui32MaxHEVCHwChromaFormat : ui32CurrentHEVCHwChromaFormat);
    }

    // Override default bit-depth with value signalled explicitly by core.
    psCoreProps->asVidStdProps[0].ui32MaxLumaBitDepth = ui32MaxBitDepthLuma;
    psCoreProps->asVidStdProps[0].ui32MaxChromaBitDepth = psCoreProps->asVidStdProps[0].ui32MaxLumaBitDepth;
    for (i = 1; i < VDEC_STD_MAX; i++)
    {
        psCoreProps->asVidStdProps[i].ui32MaxLumaBitDepth = psCoreProps->asVidStdProps[0].ui32MaxLumaBitDepth;
        psCoreProps->asVidStdProps[i].ui32MaxChromaBitDepth = psCoreProps->asVidStdProps[0].ui32MaxChromaBitDepth;
    }

    // Colour Format (H.264)
    switch (ui32MaxH264HwChromaFormat)
    {
    case 1:
        psCoreProps->asVidStdProps[VDEC_STD_H264].eMaxChromaFormat = PIXEL_FORMAT_420;
        break;

    case 2:
        psCoreProps->asVidStdProps[VDEC_STD_H264].eMaxChromaFormat = PIXEL_FORMAT_422;
        break;

    case 3:
        psCoreProps->asVidStdProps[VDEC_STD_H264].eMaxChromaFormat = PIXEL_FORMAT_444;
        break;

    default:
        break;
    }

    // Colour Format (H.265)
    switch (ui32MaxHEVCHwChromaFormat)
    {
    case 1:
        psCoreProps->asVidStdProps[VDEC_STD_HEVC].eMaxChromaFormat = PIXEL_FORMAT_420;
        break;

    case 2:
        psCoreProps->asVidStdProps[VDEC_STD_HEVC].eMaxChromaFormat = PIXEL_FORMAT_422;
        break;

    case 3:
        psCoreProps->asVidStdProps[VDEC_STD_HEVC].eMaxChromaFormat = PIXEL_FORMAT_444;
        break;

    default:
        break;
    }

    //////////////////////////////////////////////////////////////////////////
    //  Version specific settings (we are in PVDEC so version 7.0 onwards)  //
    //////////////////////////////////////////////////////////////////////////

    psCoreProps->ui32CmdBufferSize       = 64;       // from rev 3.0.0
    psCoreProps->ui32Latency       = 384; // from rev 5.0.0
    psCoreProps->ui32VdmcCacheSize = 4;        // from rev 5.5.0
    psCoreProps->ui32SrDmaBurstSize = 8; // from rev 5.5.2
    psCoreProps->ui32ScalingPrecision = 12;       // from rev 5.6.0

    if (FROM_REV(8,5,0))
    {
        //BRN54321
        psCoreProps->b444H264Rotation = IMG_TRUE;
    }
}

/*!
******************************************************************************

 @Function              pvdec_GetCoreProperties

******************************************************************************/
static IMG_RESULT pvdec_GetCoreProperties(
    PVDEC_sContext    * psPvdecContext,
    VXD_sCoreProps    * psProps
)
{
    IMG_UINT32  ui32Reg;
    IMG_UINT32  ui32Revision;
    IMG_UINT32  ui32CoreConfig;
    IMG_UINT32  ui32GroupId;
    IMG_UINT32  ui32PvdecCoreId;
    IMG_UINT32  ui32CoreId;
    IMG_UINT32  ui32ExtendedAddressRange;
    IMG_UINT32  ui32MaxFrameConfig = 0;

    IMG_UINT8   ui8PipeNum;
    IMG_UINT32  aui32PixelPipeConfig[VDEC_MAX_PIXEL_PIPES] = {0};
    IMG_UINT32  aui32PixelMiscConfig[VDEC_MAX_PIXEL_PIPES] = {0};

    PVDEC_sCoreRev sCoreRev;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psProps);

    /* PVDEC Core Revision Information */
    ui32Revision =
        REGIO_READ_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_CORE_REV);
    sCoreRev.ui32MajRev = ui32MajRev =
        REGIO_READ_FIELD(ui32Revision, PVDEC_CORE, CR_PVDEC_CORE_REV, CR_PVDEC_MAJOR_REV);
    sCoreRev.ui32MinRev = ui32MinRev =
        REGIO_READ_FIELD(ui32Revision, PVDEC_CORE, CR_PVDEC_CORE_REV, CR_PVDEC_MINOR_REV);
    sCoreRev.ui32MaintRev = ui32MaintRev =
        REGIO_READ_FIELD(ui32Revision, PVDEC_CORE, CR_PVDEC_CORE_REV, CR_PVDEC_MAINT_REV);

    // ui32LatencyConfig =
    //      REGIO_READ_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_LATENCY_CONFIG);

    /* Core ID */
    ui32PvdecCoreId =
        REGIO_READ_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_CORE_ID);
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

    // Insert Group and Core Id into PDUMP
#if 0
    MSVDXIO_Poll(psPvdecContext->hDev,
                 REGION_PVDEC_CORE_REGSPACE,
                 PVDEC_CORE_CR_PVDEC_CORE_ID_OFFSET,
                 ui32GroupId << PVDEC_CORE_CR_PVDEC_CORE_ID_CR_GROUP_ID_SHIFT,
                 PVDEC_CORE_CR_PVDEC_CORE_ID_CR_GROUP_ID_MASK,
                 VXDIO_POLL_EQUAL);
    MSVDXIO_Poll(psPvdecContext->hDev,
                 REGION_PVDEC_CORE_REGSPACE,
                 PVDEC_CORE_CR_PVDEC_CORE_ID_OFFSET,
                 ui32CoreId << PVDEC_CORE_CR_PVDEC_CORE_ID_CR_CORE_ID_SHIFT,
                 PVDEC_CORE_CR_PVDEC_CORE_ID_CR_CORE_ID_MASK,
                 VXDIO_POLL_EQUAL);
#endif

    ui32CoreConfig = REGIO_READ_FIELD(ui32PvdecCoreId,
        PVDEC_CORE, CR_PVDEC_CORE_ID, CR_PVDEC_CORE_CONFIG);

    // check DBGOPTS for core properties
    {
        DBGOPT_sValue     sVal;
        DBGOPT_eType      eType;
        VXD_sCoreProps    sCoreProps;

        if (DBGOPTKM_GetSize("CoreConfig", DBGOPT_TYPE_BUF) == sizeof(sCoreProps) &&
            DBGOPTKM_Get("CoreConfig", &eType, &sVal))
        {
            *psProps = *((VXD_sCoreProps *) sVal.sBufVal.pvVal);

            DEBUG_REPORT(REPORT_MODULE_VXDIO,
                         "Setting core configuration from debug options");
            goto setting_props_ending;
        }
    }

    VDEC_BZERO(psProps);

    // Construct core version name.
    sprintf(psProps->acVersion, "%d.%d.%d",
            sCoreRev.ui32MajRev, sCoreRev.ui32MinRev, sCoreRev.ui32MaintRev);

    ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONFIG1);
    psProps->bMmuSupportStridePerContext = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU,
        MMU_CONFIG1, SUPPORT_STRIDE_PER_CONTEXT) == 1 ? IMG_TRUE : IMG_FALSE;

    psProps->bMmuSupportSecure = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU,
        MMU_CONFIG1, SUPPORT_SECURE) == 1 ? IMG_TRUE : IMG_FALSE;

    ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONFIG0);
    ui32ExtendedAddressRange =
        REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONFIG0, EXTENDED_ADDR_RANGE);
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

    ui32Reg = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONFIG0, GROUP_OVERRIDE_SIZE);

    psProps->ui32NumEntropyPipes = ui32CoreConfig & 0xF;
    psProps->ui32NumPixelPipes = ui32CoreConfig >> 4 & 0xF;

    REPORT(REPORT_MODULE_VXDIO, REPORT_INFO,
           "PVDEC revision %08x detected, id %08x."
           " Found %d entropy pipe(s), %d pixel pipe(s), %d group size",
           ui32Revision,
           ui32CoreId,
           psProps->ui32NumEntropyPipes,
           psProps->ui32NumPixelPipes, ui32Reg);

    for (ui8PipeNum = 1; ui8PipeNum <= psProps->ui32NumPixelPipes; ++ui8PipeNum)
    {
        aui32PixelPipeConfig[ui8PipeNum - 1] |= REGIO_READ_PIPE_REGISTER(psPvdecContext,
                                                        ui8PipeNum,
                                                        PVDEC_PIXEL,
                                                        CR_PIXEL_PIPE_CONFIG);

        aui32PixelMiscConfig[ui8PipeNum - 1] |= REGIO_READ_PIPE_REGISTER(psPvdecContext,
                                                        ui8PipeNum,
                                                        PVDEC_PIXEL,
                                                        CR_PIXEL_MISC_CONFIG);
    }

    if(FROM_REV(8,0,0))
    {
        ui32MaxFrameConfig = REGIO_READ_PIPE_REGISTER(psPvdecContext, 0, PVDEC_PIXEL, CR_MAX_FRAME_CONFIG);
    }

    PVDEC_InitCoreProperties(psProps,
                             &sCoreRev,
                             ui32CoreConfig,
                             ui32MaxFrameConfig,
                             aui32PixelPipeConfig,
                             aui32PixelMiscConfig);

setting_props_ending:
    // Write trusted host pin
    //TALREG_WriteWord32(psPvdecContext->hPinTestMemSpace, 0x148, 1);

    return IMG_SUCCESS;
}


#ifndef IMG_KERNEL_MODULE

/*!
******************************************************************************

 @Function              PVDEC_PdumpComment

******************************************************************************/
IMG_RESULT
PVDEC_PdumpComment(
    IMG_HANDLE              hVxd,
    const IMG_CHAR        * pszComment
)
{
    PVDEC_sContext * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32        ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "A pointer to PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    ui32Result = PVDECIO_PDUMPComment(psPvdecContext->hDev,
                                      REGION_PVDEC_REGSPACE,
                                      pszComment);

    return ui32Result;
}

/*!
******************************************************************************

 @Function              PVDEC_PDUMPSync

******************************************************************************/
IMG_RESULT PVDEC_PDUMPSync(
    const IMG_HANDLE  hVxd
)
{
    PVDEC_sContext * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "A pointer to PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    return PVDECIO_PDUMPSync(psPvdecContext->hDev);
}


/*!
******************************************************************************

 @Function              PVDEC_PDUMPLock

******************************************************************************/
IMG_RESULT PVDEC_PDUMPLock(
    const IMG_HANDLE hVxd
)
{
    PVDEC_sContext * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "A pointer to PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    return PVDECIO_PDUMPLock(psPvdecContext->hDev);
}


/*!
******************************************************************************

 @Function              PVDEC_PDUMPUnLock

******************************************************************************/
IMG_RESULT PVDEC_PDUMPUnLock(
    const IMG_HANDLE hVxd
)
{
    PVDEC_sContext * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "A pointer to PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    return PVDECIO_PDUMPUnLock(psPvdecContext->hDev);
}
#endif

/*!
******************************************************************************

 @Function              PVDEC_Setup

******************************************************************************/
IMG_VOID PVDEC_Setup(
    const VDECDD_sDdDevConfig  * psDdDevConfig,
    IMG_HANDLE                   hPvdecContext,
    HWCTRL_sFunctions          * psVxdFuncs
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hPvdecContext;

    IMG_ASSERT(psDdDevConfig);
    IMG_ASSERT(psVxdFuncs);
    IMG_ASSERT(psPvdecContext);
    if (psDdDevConfig == IMG_NULL ||
        psVxdFuncs == IMG_NULL ||
        psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid paremeters!");
        return;
    }

    if (psDdDevConfig->ui32RendecSize)
    {
        /* Create two buffers with a 4:1 split both multiples of 4k */
        psPvdecContext->sRendec.aui32BufferSize[1] = psDdDevConfig->ui32RendecSize / 5;
        psPvdecContext->sRendec.aui32BufferSize[1] &= ~0xfff;
        psPvdecContext->sRendec.aui32BufferSize[0] = psDdDevConfig->ui32RendecSize - psPvdecContext->sRendec.aui32BufferSize[1];
        psPvdecContext->sRendec.aui32BufferSize[0] &= ~0xfff;

        psPvdecContext->sRendec.aui32InitialContext[0] = 0x00101010;
        psPvdecContext->sRendec.aui32InitialContext[1] = 0x00101010;
        psPvdecContext->sRendec.aui32InitialContext[2] = 0x00101010;
        psPvdecContext->sRendec.aui32InitialContext[3] = 0x00101010;
        psPvdecContext->sRendec.aui32InitialContext[4] = 0x00101010;
        psPvdecContext->sRendec.aui32InitialContext[5] = 0x00101010;

        psPvdecContext->sRendec.bInUse = IMG_TRUE;
    }

    psVxdFuncs->pfnHandleInt                = PVDEC_HandleInterrupts;
    psVxdFuncs->pfnGetIntStatus             = PVDEC_GetIntStatus;
    psVxdFuncs->pfnDumpRegisters            = IMG_NULL;
    psVxdFuncs->pfnSendFirmwareMessage      = PVDEC_SendFirmwareMessage;
    psVxdFuncs->pfnGetCoreState             = PVDEC_GetCoreState;
    psVxdFuncs->pfnFlushMmuCache            = PVDEC_FlushMmuCache;
    psVxdFuncs->pfnDisableCoreInterrupts    = PVDEC_DisableCoreInterrupts;
    psVxdFuncs->pfnInitialise               = PVDEC_Initialise;
    psVxdFuncs->pfnDeInitialise             = PVDEC_DeInitialise;
    psVxdFuncs->pfnDestroy                  = PVDEC_Destroy;
    psVxdFuncs->pfnPrepareFirmware          = PVDEC_PrepareFirmware;
    psVxdFuncs->pfnGetRegsOffsets           = PVDEC_GetRegsOffsets;
    psVxdFuncs->pfnGetMessage               = PVDEC_GetMessage;
    psVxdFuncs->pfnReturnMsg                = PVDEC_ReturnMsg;
    psVxdFuncs->pfnProcessMessage           = PVDEC_ProcessMessage;
#ifndef IMG_KERNEL_MODULE
    psVxdFuncs->pfnPDUMPLock                = PVDEC_PDUMPLock;
    psVxdFuncs->pfnPDUMPUnlock              = PVDEC_PDUMPUnLock;
    psVxdFuncs->pfnPDUMPSync                = PVDEC_PDUMPSync;
    psVxdFuncs->pfnPDUMPComment             = PVDEC_PdumpComment;
#endif
#ifdef VDEC_USE_PVDEC_SEC
    psPvdecContext->psVxdFuncs              = psVxdFuncs;
#endif /* VDEC_USE_PVDEC_SEC */
}

/*!
******************************************************************************

 @Function              PVDEC_Create

******************************************************************************/
IMG_RESULT PVDEC_Create(
    const VDECDD_sDdDevConfig  * psDdDevConfig,
    VXD_sCoreProps             * psCoreProps,
    HWCTRL_sRendec             * psRendec,
    IMG_HANDLE                 * phPvdecContext,
    HWCTRL_sFunctions          * psVxdFuncs
)
{
    PVDEC_sContext  * psPvdecContext = IMG_NULL;
    IMG_UINT32        ui32Result;

    IMG_ASSERT(psDdDevConfig);
    IMG_ASSERT(psCoreProps);
    IMG_ASSERT(psRendec);
    IMG_ASSERT(phPvdecContext);
    IMG_ASSERT(psVxdFuncs);

    if (psDdDevConfig == IMG_NULL || 
        psCoreProps == IMG_NULL ||
        psRendec == IMG_NULL ||
        phPvdecContext == IMG_NULL ||
        psVxdFuncs == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid parameters passed to PVDEC Create");
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
        ui32Result = PVDECIO_Initialise(IMG_FALSE,/* IO bypass */
                                        &psPvdecContext->hDev);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        pvdec_ConfigureMemStaller(psPvdecContext);

        pvdec_PvdecReset(psPvdecContext);

        ui32Result = pvdec_GetCoreProperties(psPvdecContext, &psPvdecContext->sProps);
        if (ui32Result != IMG_SUCCESS)
        {
            goto error;
        }

        HWCTRL_PrintMSVDXProps(&psPvdecContext->sProps);
    }

#ifndef SYSBRG_BRIDGING
    {
        psPvdecContext->hPvdecCoreRegSpace = TAL_GetMemSpaceHandle("P0_REG_PVDEC_CORE");
        psPvdecContext->hVideoBus4RegSpace = TAL_GetMemSpaceHandle("P0_REG_VIDEO_BUS4_MMU");
    }
#endif
    /* Setup */
    PVDEC_Setup(psDdDevConfig, psPvdecContext, psVxdFuncs);

    *psCoreProps = psPvdecContext->sProps;
    *psRendec    = psPvdecContext->sRendec;

    *phPvdecContext = psPvdecContext;

    return IMG_SUCCESS;

error:
    if(psPvdecContext)
    {
        IMG_FREE(psPvdecContext);
        psPvdecContext = IMG_NULL;
    }
    return ui32Result;
}

/*!
******************************************************************************

 @Function              PVDEC_HandleInterrupts

******************************************************************************/
IMG_RESULT PVDEC_HandleInterrupts(
    const IMG_HANDLE    hVxd,
    VXD_sIntStatus    * psIntStatus
)
{
    PVDEC_sContext           * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32                 ui32Result = IMG_SUCCESS;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psIntStatus);
    if (psPvdecContext == IMG_NULL ||
        psIntStatus == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and interrupt status struct must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    /* Disable interrupts */
    SYSOSKM_DisableInt();

    ui32Result = PVDECIO_HandleInterrupts(psPvdecContext->hDev,
                                          psIntStatus);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    /* Re enable interrupts */
    SYSOSKM_EnableInt();

    return ui32Result;
}

/*!
******************************************************************************

 @Function              PVDEC_GetIntStatus

******************************************************************************/
IMG_RESULT
PVDEC_GetIntStatus(
    const IMG_HANDLE    hVxd,
    VXD_sIntStatus    * psIntStatus
)
{
    IMG_UINT32        ui32Reg;
    IMG_UINT32        ui32Enable;
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psIntStatus);
    if (psPvdecContext == IMG_NULL ||
        psIntStatus == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and interrupt status struct must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psIntStatus->ui32Pending = REGIO_READ_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_STATUS);

    ui32Enable = REGIO_READ_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_ENABLE);

    psIntStatus->ui32Pending &= ui32Enable;

    if (psIntStatus->ui32Pending & PVDEC_CORE_CR_PVDEC_HOST_INTERRUPT_STATUS_CR_HOST_MMU_FAULT_IRQ_MASK)
    {
        ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_STATUS0);
        psIntStatus->MMU_FAULT_ADDR = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_STATUS0, MMU_FAULT_ADDR) << 12 ;
        psIntStatus->MMU_PF_N_RW = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_STATUS0, MMU_PF_N_RW);

        ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_STATUS1);
        psIntStatus->ui32Requestor = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_STATUS1, MMU_FAULT_REQ_ID);
        psIntStatus->MMU_FAULT_RNW = REGIO_READ_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_STATUS1, MMU_FAULT_RNW);
    }

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_FlushMmuCache

******************************************************************************/
IMG_RESULT PVDEC_FlushMmuCache(
    const IMG_HANDLE  hVxd
)
{
    IMG_UINT32        ui32Reg = 0;
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }
#ifdef VDEC_USE_PVDEC_SEC
    psPvdecContext->bMMUFlushNeeded = IMG_TRUE;
#endif
    ui32Reg = REGIO_READ_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1);

    REGIO_WRITE_REPEATED_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_INVALDC, 0, 1 );
    REGIO_WRITE_REPEATED_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_INVALDC, 1, 1 );
    REGIO_WRITE_REPEATED_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_INVALDC, 2, 1 );
    REGIO_WRITE_REPEATED_FIELD(ui32Reg, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1, MMU_INVALDC, 3, 1 );

    REGIO_WRITE_REGISTER(psPvdecContext, IMG_VIDEO_BUS4_MMU, MMU_CONTROL1 , ui32Reg);

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              PVDEC_DisableCoreInterrupts

******************************************************************************/
IMG_RESULT PVDEC_DisableCoreInterrupts(
    const IMG_HANDLE  hVxd
)
{
    IMG_UINT32        ui32Reg = 0;
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;

    IMG_ASSERT(psPvdecContext);
    if (psPvdecContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    /* Clear interrupt enabled flag */
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_HOST_INTERRUPT_ENABLE, 0);

    /* Clear any pending interrupt flags */
    REGIO_WRITE_FIELD(ui32Reg, PVDEC_CORE, CR_PVDEC_INTERRUPT_CLEAR, CR_IRQ_CLEAR, 0xFFFF);
    REGIO_WRITE_REGISTER(psPvdecContext, PVDEC_CORE, CR_PVDEC_INTERRUPT_CLEAR, ui32Reg);

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDEC_PrepareFirmware

******************************************************************************/
IMG_RESULT PVDEC_PrepareFirmware(
    const IMG_HANDLE        hVxd,
    const VXD_sFirmware   * psFirmware,
    const VXDIO_sPtdInfo  * psPtdInfo,
    const IMG_BOOL          bSecureStream
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT32        ui32Result;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psFirmware);
    IMG_ASSERT(psPtdInfo);
    if (psPvdecContext == IMG_NULL ||
        psFirmware == IMG_NULL ||
        psPtdInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

#ifdef VDEC_USE_PVDEC_SEC
    PVDECIO_SetSecureFw(psPvdecContext->hDev, bSecureStream);

    /* Store the firmware in local context to be loaded later on */
    if (bSecureStream)
    {
        psPvdecContext->psFwBufInfo = (VXDIO_sDdBufInfo *)&psFirmware->sFwSecBufInfo;
        psPvdecContext->psFirmwareInfo = (VDECFW_sFirmwareBinInfo *)&psFirmware->sFwSecBinInfo;
    }
    else
    {
        psPvdecContext->psFwBufInfo = (VXDIO_sDdBufInfo *)&psFirmware->sFwBaseBufInfo;
        psPvdecContext->psFirmwareInfo = (VDECFW_sFirmwareBinInfo *)&psFirmware->sFwBaseBinInfo;
    }

#ifdef SYSBRG_BRIDGING
    if (bSecureStream)
    {
        /* The aquired firmware is prepended with a firmware header info,
         * but this will be handled by below tee call, as firmware can be encrypted at this point */
        VXD_SECURE_sVerifyFirmwareArgs sVerifFirmwareArgs;

#ifdef PVDEC_SEC_TEE_PHYADDR
        sVerifFirmwareArgs.ui64SecFwBuf = (IMG_UINT64)psPvdecContext->psFwBufInfo->ppaPhysAddr[0];
#else
        sVerifFirmwareArgs.ui64SecFwBuf = (IMG_UINT64)psPvdecContext->psFwBufInfo->pvCpuVirt;
#endif

        ui32Result = SECURE_REE_SendMessageWithBuf(psPvdecContext->ui32SecureId,
                                                   (IMG_BYTE *)&sVerifFirmwareArgs,
                                                   sizeof(sVerifFirmwareArgs),
                                                   ENDPOINT_VERIFY_FIRMWARE,
                                                   (IMG_BYTE *)psPvdecContext->psFirmwareInfo->aui32Text,
                                                   psPvdecContext->psFirmwareInfo->ui32TextBufSize * sizeof(IMG_UINT32));
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
                   "Firmware verification failed (%s) !", bSecureStream ? "secure" : "non secure");
            return IMG_ERROR_GENERIC_FAILURE;
        }

        psPvdecContext->psFirmwareInfo->ui32TextDmaSize = sVerifFirmwareArgs.ui32FwCoreSize;

        // Update core size
        psPvdecContext->psFirmwareInfo->ui32TextDmaSize /= sizeof(IMG_UINT32); // core size in words

        UPDATE_DEVICE(psPvdecContext->psFwBufInfo);
    }
    else
    {
        /* Assuming non secure fw is not encrypted */
        IMG_UINT32 * aui32FwInfo = psPvdecContext->psFirmwareInfo->aui32Text;
        if (aui32FwInfo)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_INFO,
                   "Got firmware info: core size %d blob size %d firmware id 0x%x "
                   "time stamp %d", aui32FwInfo[0], aui32FwInfo[1],
                                    aui32FwInfo[2], aui32FwInfo[3]);
        }
        else
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_INFO, "aui32FwInfo is null\n");
        }
    }

    PVDECIO_SetIOBypass(psPvdecContext->hDev, bSecureStream);

#endif /* SYSBRG_BRIDGING */
#endif /* VDEC_USE_PVDEC_SEC */

    /* Prepare the firmware for loading to MTX. */
    ui32Result = PVDECIO_PrepareFirmware(psPvdecContext->hDev,
                                         (IMG_HANDLE)psFirmware);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    // Copy the device PTD for power-up initialisation.
    psPvdecContext->sPtdInfo = *psPtdInfo;

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              PVDEC_GetRegsOffsets

******************************************************************************/
IMG_RESULT PVDEC_GetRegsOffsets(
    const IMG_HANDLE        hVxd,
    DECODER_sRegsOffsets  * psRegsOffsets
)
{
    PVDEC_sContext  * psPvdecContext = (PVDEC_sContext *)hVxd;
    IMG_UINT64        ui64RegspaceOffset;
    IMG_RESULT        ui32Result;
    IMG_HANDLE      * pahMemSpaces = IMG_NULL;

    IMG_ASSERT(psPvdecContext);
    IMG_ASSERT(psRegsOffsets);
    if (psPvdecContext == IMG_NULL ||
        psRegsOffsets == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "An PVDEC context handle and reg offsets struct must be provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    pahMemSpaces = IMG_MALLOC(REGION_PVDEC_MAX * sizeof(IMG_HANDLE));
    if (pahMemSpaces == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Memory allocation for mem spaces failed.");
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pahMemSpaces, 0, REGION_PVDEC_MAX * sizeof(IMG_HANDLE));

    ui32Result = PVDECIO_GetMemSpaces(pahMemSpaces);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Failed to get mem spaces.");
        goto error;
    }

    ui32Result = TAL_GetMemSpaceOffset(pahMemSpaces[REGION_PVDEC_MSVDX_CMD], &ui64RegspaceOffset);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Failed to get VDMC CMD offset.");
        goto error;
    }
    psRegsOffsets->ui32VdmcCmdOffset = IMG_UINT64_TO_UINT32(ui64RegspaceOffset);

    ui32Result = TAL_GetMemSpaceOffset(pahMemSpaces[REGION_PVDEC_MSVDX_VEC], &ui64RegspaceOffset);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Failed to get VEC RAM offset.");
        goto error;
    }
    psRegsOffsets->ui32VecOffset = IMG_UINT64_TO_UINT32(ui64RegspaceOffset);

    ui32Result = TAL_GetMemSpaceOffset(pahMemSpaces[REGION_PVDEC_ENTROPY_PIPE_REGSPACE], &ui64RegspaceOffset);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Failed to get PVDEC ENTROPY offset.");
        goto error;
    }
    psRegsOffsets->ui32EntropyOffset = IMG_UINT64_TO_UINT32(ui64RegspaceOffset);

    ui32Result = TAL_GetMemSpaceOffset(pahMemSpaces[REGION_PVDEC_VEC_BE], &ui64RegspaceOffset);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Failed to get PVDEC VEC BE offset.");
        goto error;
    }
    psRegsOffsets->ui32VecBeRegsOffset = IMG_UINT64_TO_UINT32(ui64RegspaceOffset);

    ui32Result = TAL_GetMemSpaceOffset(pahMemSpaces[REGION_PVDEC_VEC_BE_CODEC], &ui64RegspaceOffset);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_HWCTRL, REPORT_ERR, "Failed to get PVDEC VEC BE CODEC offset.");
        goto error;
    }
    psRegsOffsets->ui32VdecBeCodecRegsOffset = IMG_UINT64_TO_UINT32(ui64RegspaceOffset);

error:
    IMG_FREE(pahMemSpaces);
    pahMemSpaces = IMG_NULL;

    return ui32Result;
}

IMG_RESULT PVDEC_GetTiledHeaps(
    VDEC_eTileScheme        eTileScheme,
    IMG_UINT32            * pui32NumHeaps,
    MMU_sHeapTilingInfo   * pasTilingInfo
)
{

    IMG_RESULT ui32Result;

    IMG_ASSERT(pui32NumHeaps);
    IMG_ASSERT(pasTilingInfo);
    if (pui32NumHeaps == IMG_NULL ||
        pasTilingInfo == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Invalid input params provided");
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    ui32Result = MMU_GetTiledHeaps(eTileScheme, pui32NumHeaps, pasTilingInfo);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    if (ui32Result != IMG_SUCCESS)
    {
        return ui32Result;
    }

    return IMG_SUCCESS;
}

/* EOF */
