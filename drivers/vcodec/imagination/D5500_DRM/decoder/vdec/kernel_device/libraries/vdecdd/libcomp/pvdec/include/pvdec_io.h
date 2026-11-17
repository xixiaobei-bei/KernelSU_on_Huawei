/*!
 *****************************************************************************
 *
 * @File       pvdec_io.h
 * @Title      Low-level PVDEC interface component
 * @Description    This file contains the interface to communicate with an PVDEC core.
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

#if !defined (__PVDECIO_H__)
#define __PVDECIO_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "vxd_ext.h"

#ifdef SYSBRG_BRIDGING

#include "sysmem_utils.h"
// Ensure insecure memory is synchronised between host cpu and device
#define UPDATE_DEVICE(buf) SYSMEMU_UpdateMemory(buf->hMemoryAlloc, CPU_TO_DEV)
#define UPDATE_HOST(buf)   SYSMEMU_UpdateMemory(buf->hMemoryAlloc, DEV_TO_CPU)

#else

#include "talmmu_api.h"
// Update the device/host memory.
#define UPDATE_DEVICE(buf) { TALMMU_CopyHostToDevMem(buf->hMemory); }
#define UPDATE_HOST(buf)   { TALMMU_CopyDevMemToHost(buf->hMemory); }

#endif

typedef enum _MEM_REGIONS_PVDEC_
{
    // Register spaces.
    REGION_PVDEC_REGSPACE,
    REGION_PVDEC_CORE_REGSPACE,
    REGION_IMG_VIDEO_BUS4_MMU_REGSPACE,
    REGION_PVDEC_PIXEL_PIPE_REGSPACE,
    REGION_PVDEC_ENTROPY_PIPE_REGSPACE,
    REGION_PVDEC_VEC_BE,
    REGION_PVDEC_VEC_BE_CODEC,
    REGION_PVDEC_MTX_CORE_REGSPACE,
    REGION_PVDEC_PIXEL_DMA_REGSPACE,
    REGION_PVDEC_MSVDX_VEC,
    REGION_PVDEC_MSVDX_VDMC,
    REGION_PVDEC_MSVDX_VDEB,
    REGION_PVDEC_MSVDX_CMD,
    REGION_PVDEC_VLR_REGSPACE,
    REGION_PVDEC_VLRFE_REGSPACE,          //!< Vec Local Ram register space (FE Context).
    REGION_PVDEC_VLRBE_REGSPACE,          //!< Vec Local Ram register space (BE Context).
    REGION_PVDEC_TEST_REGSPACE,

    REGION_PVDEC_MAX,

} MEM_REGIONS_PVDEC;

/*!
******************************************************************************

 @Function              PVDECIO_ReadRegister

 @Description

 Read value from register.

 @Input     hCoreContext    : Handle to video device IO context.

 @Input     ui32MemRegion   : Memory region of register.

 @Input     ui32Offset      : Register offset within register/memory space.

 @Input     ui8Pipe         : Id of pipe, only meaningful for some cores.

 @Return    IMG_RESULT      : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_UINT32
PVDECIO_ReadRegister(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset,
    IMG_UINT8           ui8Pipe
);


/*!
******************************************************************************

 @Function              PVDECIO_WriteRegister

 @Description

 Write value to register.

 @Input     hCoreContext    : Handle to video device IO context.

 @Input     ui32MemRegion   : Memory region of register.

 @Input     ui32Offset      : Register offset within register/memory space.

 @Input     ui32Value       : Value to write to register.

 @Input     ui32Mask        : Mask of bits to modify to register.

 @Input     ui8Pipe         : Id of pipe, only meaningful for some cores.

 @Return    IMG_RESULT      : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_WriteRegister(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset,
    IMG_UINT32          ui32Value,
    IMG_UINT32          ui32Mask,
    IMG_UINT8           ui8Pipe
);


/*!
******************************************************************************

 @Function              PVDECIO_VLRWriteWords

 @Description

 This function writes an array of words to VEC local RAM (VLR).

 @Input     hCoreContext     : Handle to video device IO context.

 @Input     ui32MemRegion    : VLR memory space.

 @Input     ui32Addr         : Address (byte) in VLR.

 @Input     ui32NumWords     : Number of 32-bits words to write.

 @Input     pui32Values      : Array of words to write.

 @Return    IMG_RESULT       : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_VLRWriteWords(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Addr,
    IMG_UINT32          ui32NumWords,
    const IMG_UINT32  * pui32Values
);


/*!
******************************************************************************

 @Function              PVDECIO_VLRReadWords

 @Description

 This function reads an array of words from VEC local RAM (VLR).

 @Input     hCoreContext          : Handle to video device IO context.

 @Input     ui32MemRegion    : VLR memory space.

 @Input     ui32Addr         : Address (byte) in VLR.

 @Input     ui32NumWords     : Number of 32-bits words to read.

 @Input     pui32Values      : Array of words to read.

 @Input     bValidate        : POL values read from VLR

 @Return    IMG_RESULT       : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_VLRReadWords(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Addr,
    IMG_UINT32          ui32NumWords,
    IMG_UINT32        * pui32Values,
    IMG_BOOL            bValidate
);

/*!
******************************************************************************

 @Function              PVDECIO_Poll

 @Description

 Poll for a specific register value.

 @Input     hCoreContext         : Handle to video device IO context.

 @Input     ui32MemRegion   : Memory region of register.

 @Input     ui32Offset      : Register offset within register/memory space.

 @Input     ui32RequValue   : Required register value.

 @Input     ui32Enable      : Mask of live bits to apply to register.

 @Input     PVDECIO_ePollMode : Check function to be used (equals, !equals ect)

 @Return    IMG_RESULT      : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_Poll(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset,
    IMG_UINT32          ui32RequValue,
    IMG_UINT32          ui32Enable,
    VXDIO_ePollMode     ePollMode
);

/*!
******************************************************************************

 @Function              PVDECIO_GetCoreState

 @Description

 Obtain the PVDEC core state.

 @Input     hCoreContext    : Handle to PVDEC IO context.

 @Output    psState         : Pointer to state information.

 @Return    IMG_RESULT      : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_GetCoreState(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32NumPixelPipes,
    IMG_UINT32          ui32NumEntPipes,
    VXDIO_sState      * psState
);

/*!
******************************************************************************

 @Function              PVDECIO_DisableClocks

 @Description

 Disables video decoder device clocks.

 @Input     hVxdCtx         : Handle to video device IO context.

 @Return    IMG_RESULT      : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_DisableClocks(
    const IMG_HANDLE    hVxdCtx
);

/*
******************************************************************************

 @Function              PVDECIO_EnableCoreClocks

 @Description

 This function enables PVDEC core clocks.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
extern IMG_VOID
PVDECIO_EnableCoreClocks(
    IMG_HANDLE hCoreContext
);

/*
******************************************************************************

 @Function              PVDECIO_EnableCoreAndProcClocks

 @Description

 This function enables PVDEC core and processor clocks.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

 @Return    IMG_RESULT       : This function returns either IMG_SUCCESS or an
                               error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_EnableCoreAndProcClocks(
    const IMG_HANDLE    hCoreContext
);

/*
******************************************************************************

 @Function              PVDECIO_GetMemSpaces

 @Description

 This function obtains PVDEC memory spaces handles.

 @Output    pahMemSpace      : Buffer where memory space handles will be
                               written.

 @Return    IMG_RESULT       : This function returns either IMG_SUCCESS or an
                               error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_GetMemSpaces(
    IMG_HANDLE        * pahMemSpace
);

/*
******************************************************************************

 @Function              PVDECIO_Initialise

 @Description

 This function initialises an PVDECIO context.

 @Input     hCommonCtx       : A handle to the VXDIO context.

 @Output    phCoreContext    : Pointer used to return handle to new
                               PVDEC IO context.

 @Return    IMG_RESULT       : This function returns either IMG_SUCCESS or an
                               error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_Initialise(
    IMG_BOOL      bIOBypass,
    IMG_HANDLE  * phCoreContext
);

/*
******************************************************************************

 @Function              PVDECIO_DeInitialise

 @Description

 This function deinitialises an PVDECIO context.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

 @Return    IMG_RESULT       : This function returns either IMG_SUCCESS or an
                               error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_DeInitialise(
    IMG_HANDLE   hCoreContext
);


/*!
******************************************************************************

 @Function              PVDECIO_PvdecReset

 @Description

 Resets PVDEC core.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
extern IMG_RESULT
PVDECIO_PvdecReset(
    IMG_HANDLE          hCoreContext
);


/*!
******************************************************************************

 @Function              PVDECIO_SendFirmwareMessage

 @Description

 Send message to firmware.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

 @Input     psMsgHdr         : Pointer to buffer with message in DEVA format.


******************************************************************************/
extern IMG_RESULT
PVDECIO_SendFirmwareMessage(
    const IMG_HANDLE           hCoreContext,
    const IMG_VOID           * psMsgHdr
);

/*!
******************************************************************************

 @Function              PVDECIO_LoadBaseFirmware

 @Description

 Loads base firmware component to MTX.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
extern IMG_RESULT
PVDECIO_LoadBaseFirmware(
    const IMG_HANDLE        hCoreContext
);


/*!
******************************************************************************

 @Function              PVDECIO_HandleInterrupts

******************************************************************************/
IMG_RESULT
PVDECIO_HandleInterrupts(
    const IMG_HANDLE    hCoreContext,
    VXD_sIntStatus    * psIntStatus
);

/*!
******************************************************************************

 @Function              PVDECIO_PrepareFirmware

******************************************************************************/
IMG_RESULT
PVDECIO_PrepareFirmware(
    const IMG_HANDLE            hCoreContext,
    const IMG_HANDLE            hFirmware
);

/*
******************************************************************************

 @Function              PVDECIO_KickMTX

 @Description

 Triggers MTX interrupt.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
IMG_VOID
PVDECIO_KickMTX(
    const IMG_HANDLE hCoreContext
);


/*!
******************************************************************************

 @Function              PVDECIO_TriggerSim

 @Description

 This functions triggers internal processing loop of simulator.

 @Input     hVxdCtx                 : Handle to video device IO context.

 @Return    none

******************************************************************************/
extern IMG_VOID
PVDECIO_TriggerSim(
    const IMG_HANDLE    hCoreContext
);

#if (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING))
/*
******************************************************************************

 @Function              PVDECIO_HandleDebugFifo

 @Description

 Handles PVDEC debug fifo with logs from firmware.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
IMG_VOID
PVDECIO_HandleDebugFifo(
    const IMG_HANDLE   * hCoreContext
);

/*
******************************************************************************

 @Function              PVDECIO_EnableDebugFifo

 @Description

 Enables PVDEC debug fifo.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
IMG_VOID
PVDECIO_EnableDebugFifo(
    const IMG_HANDLE   * hCoreContext
);

/*
******************************************************************************

 @Function              PVDECIO_DisableDebugFifo

 @Description

 Disables PVDEC debug fifo.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
IMG_VOID
PVDECIO_DisableDebugFifo(
    const IMG_HANDLE   * hCoreContext
);

/*
******************************************************************************

 @Function              PVDECIO_GetDebugFifoSize

 @Description

 Returns PVDEC debug fifo size.

 @Input     hCoreContext     : A handle to the PVDEC IO context.

******************************************************************************/
IMG_UINT32
PVDECIO_GetDebugFifoSize(
    const IMG_HANDLE   * hCoreContext
);

#endif /* (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)) */

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              PVDECIO_SetIOBypass

 @Description

 This functions indicates that restricted IO calls shall not be handled.

 @Input     hVxdCtx                 : Handle to video device IO context.

 @Input     bEnabled                : enable/disable IO bypass.

 @Return    none

******************************************************************************/
extern IMG_VOID
PVDECIO_SetIOBypass(
    const IMG_HANDLE    hCoreContext,
    IMG_BOOL bEnabled
);
/*!
******************************************************************************

 @Function              PVDECIO_SetSecureFw

 @Description

 This functions indicates that secure fw is in use.

 @Input     hVxdCtx                 : Handle to video device IO context.

 @Input     bEnabled                : enable/disable secure firmware.

 @Return    none

******************************************************************************/
extern IMG_VOID
PVDECIO_SetSecureFw(
    const IMG_HANDLE    hCoreContext,
    IMG_BOOL bEnabled
);
#endif

#if !defined(SECURE_MEDIA_SUPPORT) && !defined(SYSBRG_BRIDGING)
/*!
******************************************************************************

 @Function              PVDECIO_PDUMPVerifPoll

 @Description

 Poll only for pdump purposes (to force the pdump script to verify this value.

 @Input     hCoreContext    : Handle to video device IO context.

 @Input     ui32MemRegion   : Memory region of register.

 @Input     ui32Offset      : Register offset within register/memory space.

 @Input     ui32RequValue   : Required register value.

 @Input     ui32Enable      : Mask of live bits to apply to register.

******************************************************************************/
extern IMG_RESULT
PVDECIO_PDUMPVerifPoll(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset,
    IMG_UINT32          ui32RequValue,
    IMG_UINT32          ui32Enable
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPComment

 @Description

 Insert a comment in the pdump script

 @Input     hCoreContext    : Handle to video device IO context.

 @Input     ui32MemRegion   : Memory region of register.

 @Input     pszComment      : Comment

 @Return    IMG_RESULT      : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_RESULT
PVDECIO_PDUMPComment(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    const IMG_CHAR *    pszComment
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPSync

 @Description

 Sync pdump contexts

 @Input     hCoreContext    : Handle to video device IO context.

******************************************************************************/
IMG_RESULT
PVDECIO_PDUMPSync(
    const IMG_HANDLE    hCoreContext
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPLock

 @Description

 Lock pdump contexts

 @Input     hCoreContext    : Handle to video device IO context.

******************************************************************************/
extern IMG_RESULT
PVDECIO_PDUMPLock(
    const IMG_HANDLE    hCoreContext
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPUnLock

 @Description

 Unlock pdump contexts

 @Input     hCoreContext     : Handle to video device IO context.

******************************************************************************/
extern IMG_RESULT
PVDECIO_PDUMPUnLock(
    const IMG_HANDLE    hCoreContext
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPPollCircBuff

 @Description

 Poll only for pdump purposes (to force the pdump script to verify this value.

 @Input     hCoreContext         : Handle to video device IO context.

 @Input     ui32MemRegion        : Memory region of register.

 @Input     ui32Offset           : Register offset within register/memory space.

 @Input     ui32WriteOffsetVal   : Required register value.

 @Input     ui32PacketSize       : Required register value.

 @Input     ui32BufferSize       : Required register value.

******************************************************************************/
IMG_RESULT
PVDECIO_PDUMPPollCircBuff(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset,
    IMG_UINT32          ui32WriteOffsetVal,
    IMG_UINT32          ui32PacketSize,
    IMG_UINT32          ui32BufferSize
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPDisableCmds

 @Description

 Disable the capture of specified PDUMP commands for a specific register.

 @Input     hCoreContext     : Handle to video device IO context.

 @Input     ui32MemRegion    : Memory region of register.

 @Input     ui32Offset       : Register offset within register/memory space.

 @Input     ui32DisableFlags : PDUMP commands disable flags.

 @Return    IMG_RESULT       : Returns either IMG_SUCCESS or an error code.

******************************************************************************/
extern IMG_UINT32
PVDECIO_PDUMPDisableCmds(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset,
    IMG_UINT32          ui32DisableFlags
);

/*!
******************************************************************************

 @Function              PVDECIO_PDUMPGetDisableCmds

 @Description

 Get PDUMP commands disable flags for a specific register.

 @Input     hCoreContext     : Handle to video device IO context.

 @Input     ui32MemRegion    : Memory region of register.

 @Input     ui32Offset       : Register offset within register/memory space.

 @Return    IMG_RESULT       : Returns PDUMP disable flags for ui32Offset.

******************************************************************************/
extern IMG_UINT32
PVDECIO_PDUMPGetDisableCmds(
    const IMG_HANDLE    hCoreContext,
    IMG_UINT32          ui32MemRegion,
    IMG_UINT32          ui32Offset
);

#endif /* !defined(SECURE_MEDIA_SUPPORT) && !defined(SYSBRG_BRIDGING) */

#if defined(__cplusplus)
}
#endif

#endif /* __PVDECIO_H__ */



