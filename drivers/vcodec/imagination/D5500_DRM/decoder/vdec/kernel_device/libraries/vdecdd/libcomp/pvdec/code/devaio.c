/*!
 *****************************************************************************
 *
 * @File       devaio.c
 * @Title      Low-level MTX interface with DEVA PVDEC firmware
 * @Description    This file contains functions to communicate with MTX DEVA PVDEC firmware.
 *  This includes sending and receiving messages and state indicators.
 *  It is a subcomponent of the Decoder block and operates on the Decoder
 *  block state.
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

#include "img_errors.h"
#include "img_types.h"
#ifndef SYSBRG_BRIDGING
#include "gzip_fileio.h"
#include "dbgopt_api.h"
#endif

#include "pvdec_io.h"
#include "devaio.h"
#include "report_api.h"
#include "sysos_api_km.h"
#include "dbgopt_api_km.h"

#include "interface/deva_fw_msg.h"

#include "hwdefs/pvdec_core_regs.h"


#ifdef PVDEC_DEBUG_FACILITY
#include "tal.h"
#include "hwdefs/msvdx_mtx_regs.h"
#endif /* PVDEC_DEBUG_FACILITY */

#include "firmware.h"

#define DBG_FIFO_FIRST_PARAM_OFFSET    (2)
#define DBG_TASK_INDENT_DIFF           (2)
#define DEVAIO_DBG_LOG_FMT             "%6s:"
#define DEVAIO_DBG_LOG_UNKOWN_TASK     "???"

/* Logs format when logs database is not present or no
 * relevant entry was found: */
/* Log prefix for normal messages. */
#define DEVAIO_DBG_LOG_PFX_MISSING_LOG      "|||"
/* Log prefix for asserts. */
#define DEVAIO_DBG_LOG_PFX_MISSING_ASSERT   "!!!"
/* "<pfx> <file id>:<line number> <num args> <task SP>[ : args]" */
#define DEVAIO_DBG_LOG_FMT_MISSING_BARE     "%s 0x%08x:%u %u 0x%08x%s"
/* "<pfx> <file name>:<line number> <num args> <task SP> : [args]" */
#define DEVAIO_DBG_LOG_FMT_MISSING_FILE     "%s %s:%u %u 0x%08x%s"

#if (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING))

#ifndef SYSBRG_BRIDGING
/* Set to IMG_TRUE in the Debug FIFO log file is already created.
 * This allows for logging all the firmware debug data through all the resets that can occur. */
static IMG_BOOL bDebugFifoFileOpened = IMG_FALSE;
#else
/*!
******************************************************************************

 @Function              devaio_fnDebugFifo

******************************************************************************/
static IMG_BOOL devaio_fnDebugFifo(
    IMG_VOID *  pvParam
)
{
    DEVAIO_sContext  * psContex = (DEVAIO_sContext *)pvParam;

    IMG_ASSERT(psContex);

    /* Wake up the suspended process...*/
    DEVAIO_HandleDebugFifo(psContex);

    /* Indicate that we want to be rescheduled...*/
    return IMG_TRUE;
}
#endif /* not SYSBRG_BRIDGING */


/*!
******************************************************************************

 @Function              devaio_InitDebugFifo

******************************************************************************/
static IMG_RESULT devaio_InitDebugFifo(
    DEVAIO_sContext  * psContext
)
{
#ifdef SYSBRG_BRIDGING
    IMG_HANDLE        hAuxTimer = IMG_NULL;
#else /* ndef SYSBRG_BRIDGING */
    const IMG_CHAR  * pui8FileName = "mtx_debug_0.txt";
    IMG_HANDLE        hAuxFile = IMG_NULL;
#endif /* ndef SYSBRG_BRIDGING */
    IMG_UINT32        ui32Result;

    IMG_ASSERT(psContext);
    /* Store some already initialised values. */
#ifdef SYSBRG_BRIDGING
    if (IMG_NULL != psContext->sDebugFifo.hTimer)
    {
        hAuxTimer = psContext->sDebugFifo.hTimer;
    }
#else /* ndef SYSBRG_BRIDGING */
    if (IMG_TRUE == bDebugFifoFileOpened)
    {
        hAuxFile = psContext->sDebugFifo.hFile;
    }
#endif /* ndef SYSBRG_BRIDGING */
    /* Reset debug FIFO structure. */
    IMG_MEMSET(&psContext->sDebugFifo, 0, sizeof(psContext->sDebugFifo));
    LST_init(&psContext->sDebugFifo.asDbgTasks);

    /* Restore some already initialised values. */
#ifdef SYSBRG_BRIDGING
    if (IMG_NULL != hAuxTimer)
    {
        psContext->sDebugFifo.hTimer = hAuxTimer;
    }
#else /* ndef SYSBRG_BRIDGING */
    if (IMG_TRUE == bDebugFifoFileOpened)
    {
        psContext->sDebugFifo.hFile = hAuxFile;
    }
#endif /* ndef SYSBRG_BRIDGING */

#ifdef SYSBRG_BRIDGING
    /* Create the debug FIFO handling timer. */
    if (IMG_NULL == psContext->sDebugFifo.hTimer)
    {
        ui32Result = SYSOSKM_CreateTimer(devaio_fnDebugFifo, (IMG_VOID *)psContext, 100, &psContext->sDebugFifo.hTimer);
        if (IMG_SUCCESS != ui32Result)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
                   "Failed to create debug FIFO timer");
            return ui32Result;
        }
    }
#else /* ndef SYSBRG_BRIDGING */
    if (IMG_FALSE == bDebugFifoFileOpened)
    {
        ui32Result = IMG_FILEIO_OpenFile(pui8FileName, "w", &psContext->sDebugFifo.hFile, IMG_FALSE);
        if (ui32Result != IMG_SUCCESS)
        {
            psContext->sDebugFifo.hFile = IMG_NULL;
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
                   "Failed to open file for saving FIFO debug output (%s)", pui8FileName);
            return ui32Result;
        }
        bDebugFifoFileOpened = IMG_TRUE;
    }
#endif /* ndef SYSBRG_BRIDGING */
    /* Get fifo size */
    {
        IMG_UINT32 ui32FifoCtrl;
        IMG_UINT8  ui8Pipe = 0;
        ui32FifoCtrl = PVDECIO_ReadRegister(psContext->hPvdecIoCtx,
                                            REGION_PVDEC_CORE_REGSPACE,
                                            PVDEC_CORE_CR_PROC_DEBUG_FIFO_CTRL_0_OFFSET,
                                            ui8Pipe);
        psContext->sDebugFifo.ui32FifoSize = REGIO_READ_FIELD(ui32FifoCtrl,
            PVDEC_CORE, CR_PROC_DEBUG_FIFO_CTRL_0, CR_PROC_DEBUG_FIFO_SIZE);
    }

    DEVAIO_EnableDebugFifo(psContext);

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              devaio_FindDbgTask

 @Description

 This function searches for task on psFifo->asDbgTasks list.
 Returned task corresponds to passed task's stack pointer ui32TaskSp.

 When no task is found, NULL is returned.

******************************************************************************/
static DEVAIO_sDbgTaskDesc * devaio_FindDbgTask(
    DEVAIO_DebugFifo  * psFifo,
    IMG_UINT32          ui32TaskSp
)
{
    DEVAIO_sDbgTaskDesc *psDbgTask;
    DEVAIO_sDbgTaskDesc *psNextDbgTask;

    IMG_ASSERT(psFifo);

    psDbgTask = LST_first(&psFifo->asDbgTasks);
    if (psDbgTask == IMG_NULL)
    {
        return IMG_NULL;
    }

    if (ui32TaskSp < psDbgTask->ui32TaskSp)
    {
        return IMG_NULL;
    }

    psNextDbgTask = LST_next(psDbgTask);

    while (psNextDbgTask)
    {
        if ((psDbgTask->ui32TaskSp <= ui32TaskSp) && (psNextDbgTask->ui32TaskSp > ui32TaskSp))
        {
            return psDbgTask;
        }

        psDbgTask = psNextDbgTask;
        psNextDbgTask = LST_next(psNextDbgTask);
    }

    return psDbgTask;

}

/*!
******************************************************************************

 @Function              devaio_AddDbgTask

 @Description

 This function adds new debug task description to psFifo->asDbgTasks.
 Tasks on this list are sorted by value of task's stack pointer.

 This function evaluates task name basing on pcFmt and parameters received
 from firmware and saves it to psNewDbgTask->acTaskName.

******************************************************************************/
static DEVAIO_sDbgTaskDesc * devaio_AddDbgTask(
    DEVAIO_DebugFifo  * psFifo,
    const IMG_CHAR    * pszFmt,
    IMG_UINT32          ui32TaskSp
)
{

    DEVAIO_sDbgTaskDesc *psDbgTask;
    DEVAIO_sDbgTaskDesc *psNewDbgTask;
    IMG_UINT32 ui32ArgsCount = 0, ui32I, ui32TaskNameLen;

    IMG_ASSERT(psFifo);
    IMG_ASSERT(pszFmt);

    psDbgTask = devaio_FindDbgTask(psFifo, ui32TaskSp);

    psNewDbgTask = IMG_MALLOC(sizeof(DEVAIO_sDbgTaskDesc));

    if (psNewDbgTask == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Failed to allocate debug task description structure.");
        return IMG_NULL;
    }

    REPORT(REPORT_MODULE_VXDIO, REPORT_INFO, "Adding new task for sp: %08x", ui32TaskSp);

    IMG_MEMSET(psNewDbgTask, 0, sizeof(DEVAIO_sDbgTaskDesc));

    psNewDbgTask->ui32TaskSp = ui32TaskSp;

    ui32TaskNameLen = strlen(pszFmt);
    ui32TaskNameLen = (ui32TaskNameLen > DEVAIO_MAX_DBG_TASK_NAME_SIZE) ?
                      DEVAIO_MAX_DBG_TASK_NAME_SIZE : ui32TaskNameLen;
    /* Character '%' in task name indicates that name should be completed
     * with debug message parameters. Only one '%' in task name is supported. */
    for (ui32I = 1; ui32I < ui32TaskNameLen; ui32I++)
    {
        if (pszFmt[ui32I] == '%')
        {
            ui32ArgsCount++;
        }
    }

    if (ui32ArgsCount > 1)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Invalid task name %s", pszFmt);
        snprintf(psNewDbgTask->acTaskName, DEVAIO_MAX_DBG_TASK_NAME_SIZE, "invalid:");
    }
    else
    {
        snprintf(psNewDbgTask->acTaskName, DEVAIO_MAX_DBG_TASK_NAME_SIZE, &pszFmt[1],
                 psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET]);
    }

    if (psDbgTask == IMG_NULL)
    {
        /* NULL psDbgTask means that either the list is empty, or new task context should
         * be inserted before any tasks currently present on list. LST_addHead() covers
         * both cases. */
        LST_addHead(&psFifo->asDbgTasks, psNewDbgTask);
    }
    else
    {
        /* Insert new debug task context on position indicated by devaio_FindDbgTask(),
         * just after psDbgTask */
        psNewDbgTask->LST_link = LST_next(psDbgTask);
        psDbgTask->LST_link = (IMG_VOID **)psNewDbgTask;
    }

    return psNewDbgTask;;
}


/*!
******************************************************************************

 @Function              devaio_ProcessDbgTaskCtx

 @Description

 This function finds relevant debug task context basing on task's stack
 pointer inside firmware.

 This function modifies found context of debug task basing on control
 characters present at offset 0 of log message string:

 '+' : increase indent
 '-' : decrease indent
 '*' : add new task

 At the end, function fills psFifo->acText with relevant task name and
 indent.

******************************************************************************/
static IMG_UINT32 devaio_ProcessDbgTaskCtx(
        DEVAIO_DebugFifo    * psFifo,
        IMG_UINT32            ui32TaskSp,
        const IMG_CHAR      * pszFmt,
        IMG_UINT32            ui32TextSize
)
{
    IMG_CHAR cMark = pszFmt[0];
    IMG_UINT ui32Indent, ui32Pos = 0;
    IMG_CHAR * pcText;
    DEVAIO_sDbgTaskDesc *psDbgTask;

    IMG_ASSERT(psFifo);
    IMG_ASSERT(pszFmt);

    pcText = (IMG_CHAR *)psFifo->acText;

    if (cMark == '*')
    {
        psDbgTask = devaio_AddDbgTask(psFifo, pszFmt, ui32TaskSp);
    }
    else
    {
        psDbgTask = devaio_FindDbgTask(psFifo, ui32TaskSp);
    }

    if (psDbgTask == IMG_NULL)
    {
        return snprintf(pcText, ui32TextSize, DEVAIO_DBG_LOG_FMT,
                        DEVAIO_DBG_LOG_UNKOWN_TASK);
    }

    ui32Indent = psDbgTask->i32Indent;

    switch(cMark)
    {
        case '+':
        {
            psDbgTask->i32Indent += DBG_TASK_INDENT_DIFF;
            break;
        }
        case '-':
        {
            IMG_INT32 i32Res = psDbgTask->i32Indent - DBG_TASK_INDENT_DIFF;
            psDbgTask->i32Indent = (i32Res > 0) ? (i32Res) : 0;
            ui32Indent = psDbgTask->i32Indent ;
            break;
        }
    }

    ui32Pos = snprintf(&pcText[ui32Pos], ui32TextSize,
                       DEVAIO_DBG_LOG_FMT, psDbgTask->acTaskName);

    if (ui32Pos >= ui32TextSize)
    {
        return ui32Pos;
    }

    ui32TextSize -= ui32Pos;

    while (ui32Indent-- && ui32TextSize--)
    {
        pcText[ui32Pos++] = ' ';
    }
    pcText[ui32Pos] = 0;

    return ui32Pos;
}

/*!
******************************************************************************

 @Function              devaio_CreateLogForEmptyEntry

 @Description

 This functions creates log string for firmware debug messages with missing
 entry in firmware logs database. Log string is saved to psFifo->acText.

******************************************************************************/
static IMG_VOID devaio_CreateLogForEmptyEntry(
    DEVAIO_DebugFifo                * psFifo,
    IMG_UINT32                        ui32ArgsCount,
    IMG_UINT32                        ui32FileNameHash,
    IMG_UINT32                        ui32LineNo,
    IMG_UINT32                        ui32TaskSp,
    IMG_BOOL                          bIsAssert,
    const PVDECFW_sPvdecFwFileInfo  * psPvdecFwFileInfo
)
{
    IMG_INT32 i32LogLen = 0;
    IMG_UINT32 ui32ArgNum = DBG_FIFO_FIRST_PARAM_OFFSET;
    IMG_CHAR * pszLogPfx = DEVAIO_DBG_LOG_PFX_MISSING_LOG;
    IMG_CHAR * pszArgPfx = ui32ArgsCount > 0 ? " : " : "";

    IMG_ASSERT(psFifo);

    if (bIsAssert)
    {
        pszLogPfx = DEVAIO_DBG_LOG_PFX_MISSING_ASSERT;
    }

    if (IMG_NULL != psPvdecFwFileInfo)
    {
        /* If there's a firmware file info available log file name. */
        i32LogLen += snprintf(psFifo->acText,
                              DEVAIO_DEBUG_TEXT_SIZE,
                              DEVAIO_DBG_LOG_FMT_MISSING_FILE,
                              pszLogPfx,
                              psPvdecFwFileInfo->pszFwFileName,
                              ui32LineNo,
                              ui32ArgsCount,
                              ui32TaskSp,
                              pszArgPfx);
    }
    else
    {
        /* Otherwise just log bare data. */
        i32LogLen += snprintf(psFifo->acText,
                              DEVAIO_DEBUG_TEXT_SIZE,
                              DEVAIO_DBG_LOG_FMT_MISSING_BARE,
                              pszLogPfx,
                              ui32FileNameHash,
                              ui32LineNo,
                              ui32ArgsCount,
                              ui32TaskSp,
                              pszArgPfx);
    }

    /* Log all the arguments. */
    while (ui32ArgsCount--)
    {
        IMG_INT32 i32CurPos = i32LogLen - 1;
        if (i32CurPos < 0)
        {
            i32CurPos = 0;
        }
        else if (i32CurPos >= DEVAIO_DEBUG_TEXT_SIZE)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
                    "Insufficient space when trying to create log message"
                    " for empty DB entry (file: 0x%08x, line: %u, sp: 0x%08x, args: %u)",
                    ui32FileNameHash, ui32LineNo, ui32TaskSp, ui32ArgsCount);
            return;
        }
        i32LogLen += snprintf(&psFifo->acText[i32CurPos],
                              DEVAIO_DEBUG_TEXT_SIZE - i32CurPos,
                              " 0x%08x",
                              psFifo->aui32DebugData[ui32ArgNum++]);
    }
}


/*!
******************************************************************************

 @Function              devaio_FindFwFileLogEntry

 @Description

 This functions finds a log string for a provided line number in the firmware
 file logs database.

******************************************************************************/
const IMG_CHAR * devaio_FindFwFileLogEntry(
    const PVDECFW_sPvdecFwFileLogDB  * psPvdecFwFileLogDB,
    IMG_UINT32                         ui32LineNo
)
{
    /* If there's a non-empty firmware file log database... */
    if ((IMG_NULL != psPvdecFwFileLogDB) && (psPvdecFwFileLogDB->ui32ItemCount > 0))
    {
        /* Calculate starting pointers for binary search. */
        IMG_UINT32  ui32First  = 0;
        IMG_UINT32  ui32Last   = psPvdecFwFileLogDB->ui32ItemCount - 1;
        IMG_UINT32  ui32Middle = (ui32First + ui32Last) / 2;

        if ((ui32LineNo >= psPvdecFwFileLogDB->asLogItems[ui32First].ui32LineNumber) &&
            (ui32LineNo <= psPvdecFwFileLogDB->asLogItems[ui32Last].ui32LineNumber))
        {
            /* Do the binary search on firmware file log database. */
            while ((ui32LineNo != psPvdecFwFileLogDB->asLogItems[ui32Middle].ui32LineNumber) &&
                   (ui32First < ui32Last))
            {
                if (ui32LineNo > psPvdecFwFileLogDB->asLogItems[ui32Middle].ui32LineNumber)
                {
                    ui32First = ui32Middle + 1;
                }
                else if (ui32LineNo < psPvdecFwFileLogDB->asLogItems[ui32Middle].ui32LineNumber)
                {
                    ui32Last = ui32Middle - 1;
                }
                ui32Middle = (ui32First + ui32Last) / 2;
            }
            /* Return log line text if any is found. */
            if (ui32LineNo == psPvdecFwFileLogDB->asLogItems[ui32Middle].ui32LineNumber)
            {
                return psPvdecFwFileLogDB->asLogItems[ui32Middle].pszLogFormat;
            }
        }
    }

    return IMG_NULL;
}


/*!
******************************************************************************

 @Function              devaio_ProcessDbgFifoMsgs

 @Description

 This function is used to process single debug message from firmware.
 Data received from debug FIFO for this single message is supposed to be
 placed in psFifo->aui32DebugData.

 Word at offset 0 contains control data: number of parameters, id of
 firmware source file containing log and line number at which log occurs.

 Number of debug message parameters follows.

 Address of task's stack pointer inside firmware is present at the end.

******************************************************************************/
static IMG_VOID devaio_ProcessDbgFifoMsgs(
    DEVAIO_DebugFifo  * psFifo
)
{
    /* Collect all the message data. */
    IMG_UINT32  ui32DataIndex    = 0;
    IMG_UINT32  ui32Ctrl         = psFifo->aui32DebugData[ui32DataIndex++];
    IMG_UINT32  ui32FileNameHash = psFifo->aui32DebugData[ui32DataIndex];
    IMG_UINT32  ui32ParamsCount  = DEBUG_DATA_MSG_GET_ARG_COUNT(ui32Ctrl);
    IMG_UINT32  ui32LineNo       = DEBUG_DATA_MSG_GET_LINE_NO(ui32Ctrl);
    IMG_BOOL    bIsAssert        = DEBUG_DATA_MSG_TYPE_IS_ASSERT(ui32Ctrl);
    /* Task stack pointer is added after last parameter */
    IMG_UINT32  ui32TaskSp       = psFifo->aui32DebugData[ui32ParamsCount + DBG_FIFO_FIRST_PARAM_OFFSET];

    /* Get the firmware file info. */
    const PVDECFW_sPvdecFwFileInfo  * psPvdecFwFileInfo = IMG_NULL;
    const IMG_CHAR                  * pszFwFileLogEntry = IMG_NULL;
    /* Logs database is provided with firmware binary */
#ifdef VDEC_USE_PVDEC_SEC
    if (psFifo->bSecureFwLogs)
    {
        psPvdecFwFileInfo = VDECFW_GetFwFileInfoSec(ui32FileNameHash);
    }
    else
#endif /* VDEC_USE_PVDEC_SEC */
    {
        psPvdecFwFileInfo = VDECFW_GetFwFileInfoFull(ui32FileNameHash);
    }


    /* Reset log text. */
    IMG_MEMSET(psFifo->acText, 0, DEVAIO_DEBUG_TEXT_SIZE);

    /* If firmware file info is available and there's a log entry present in its logs database... */
    if ((IMG_NULL != psPvdecFwFileInfo) &&
        (IMG_NULL != (pszFwFileLogEntry =
                        devaio_FindFwFileLogEntry(&psPvdecFwFileInfo->sPvdecFwFileLogDB, ui32LineNo))))
    {
        IMG_UINT32 ui32CharsAdded;

        /* Identify and log the task. */
        ui32CharsAdded = devaio_ProcessDbgTaskCtx(psFifo,
                                                  ui32TaskSp,
                                                  pszFwFileLogEntry,
                                                  DEVAIO_DEBUG_TEXT_SIZE);

        if (ui32CharsAdded >= DEVAIO_DEBUG_TEXT_SIZE)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
                   "Insufficient space when trying to add task name"
                   " (file: %s, line: %u, sp: %u, args: %u)",
                   psPvdecFwFileInfo->pszFwFileName, ui32LineNo,
                   ui32TaskSp, ui32ParamsCount);
            return;
        }

        /* Add the actual log text with params. */
        ui32CharsAdded += snprintf(&psFifo->acText[ui32CharsAdded],
                                   DEVAIO_DEBUG_TEXT_SIZE - ui32CharsAdded,
                                   pszFwFileLogEntry,
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 1],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 2],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 3],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 4],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 5],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 6],
                                   psFifo->aui32DebugData[DBG_FIFO_FIRST_PARAM_OFFSET + 7]);

        /* Add location info for asserts. */
        if (bIsAssert)
        {
            snprintf(&psFifo->acText[ui32CharsAdded],
                     DEVAIO_DEBUG_TEXT_SIZE - ui32CharsAdded,
                     "    @%s:%d",
                     psPvdecFwFileInfo->pszFwFileName,
                     ui32LineNo);
        }
    }
    else
    {
        /* Create bare log line if no relevant log info is available. */
        devaio_CreateLogForEmptyEntry(
                psFifo,
                ui32ParamsCount,
                ui32FileNameHash,
                ui32LineNo,
                ui32TaskSp,
                bIsAssert,
                psPvdecFwFileInfo);
    }

    REPORT(REPORT_MODULE_FW, REPORT_INFO, "%s", psFifo->acText);

#ifndef SYSBRG_BRIDGING
    if (psFifo->hFile != IMG_NULL)
    {
        IMG_FILEIO_Printf(psFifo->hFile, "%s\n", psFifo->acText);
    }
#endif /* not SYSBRG_BRIDGING */
}


#if defined(FW_PERFORMANCE_LOGGING)
/*!
******************************************************************************

 @Function              devaio_perfInfo

******************************************************************************/
static IMG_VOID devaio_perfInfo(
    IMG_UINT32 * pui32PerfData
)
{
    IMG_UINT16 ui16MsgId = (pui32PerfData[0])&0xffff;
    IMG_UINT8 ui8Pipe = (pui32PerfData[0]>>16)&0xf;
    IMG_UINT8 ui8Type = (pui32PerfData[0]>>24)&0xf;
    IMG_UINT8 ui8Idx = (pui32PerfData[0]>>20)&0xf;
    IMG_UINT32 ui32Data = pui32PerfData[1];
    IMG_BOOL bDataAttached = (pui32PerfData[0]&0x10000000);
    IMG_UINT32 ui32Tick =  bDataAttached ? pui32PerfData[2] : pui32PerfData[1];

    if (bDataAttached)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_INFO,
                "PERF_INFO: m=%u p=%u type=%u i=%u d1=%u d2=%u",
                ui16MsgId, ui8Pipe, ui8Type, ui8Idx, ui32Tick, ui32Data);
    }
    else
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_INFO,
                "PERF_INFO: m=%u p=%u type=%u i=%u d1=%u",
                ui16MsgId, ui8Pipe, ui8Type, ui8Idx, ui32Tick);
    }
}
#endif

/*!
******************************************************************************

 @Function              devaio_HandleDebugFifoData

 @Description

 This function reads debug data submitted by firmware from debug fifo and
 saves it to psFifo->aui32DebugData.

 Once whole chunk of data is collected, relevant processing function is
 called.

 In general, chunks with debug messages and performance data are currently
 supported.

******************************************************************************/
static IMG_VOID devaio_HandleDebugFifoData(
    DEVAIO_sContext  * psContext,
    IMG_UINT32         ui32Val
)
{
    DEVAIO_DebugFifo * psFifo;

    IMG_ASSERT(psContext);

    psFifo = &psContext->sDebugFifo;

    switch(psFifo->ui32Collecting) /* What kind of chunk are we processing */
    {
        /* header*/
        case DEBUG_DATA_TYPE_HEADER:
        {
            psFifo->ui32DataIndex = 0;
            IMG_MEMSET(psFifo->aui32DebugData, 0, DEVAIO_DEBUG_MAX_PARAMS * sizeof(psFifo->aui32DebugData[0]));

            /* Check for performance data first. */
            if (DEBUG_DATA_GET_TYPE(ui32Val) >= DEBUG_DATA_TYPE_PERF)
            {
                psFifo->ui32ParamsCount = 1 + DEBUG_DATA_TYPE_PERF_IS_INC_TIME(ui32Val);
                psFifo->ui32Collecting = DEBUG_DATA_TYPE_PERF;
                psFifo->aui32DebugData[psFifo->ui32DataIndex++] = ui32Val;
            }
            /* Other debug data received. */
            else
            {
                psFifo->ui32ParamsCount = 0;
                psFifo->ui32Collecting  = DEBUG_DATA_GET_TYPE(ui32Val);
                psFifo->aui32DebugData[psFifo->ui32DataIndex] = ui32Val;
            }
            break;
        }

        /* Collect performance data chunk parameters */
        case DEBUG_DATA_TYPE_PERF:
        {
            psFifo->ui32ParamsCount--;
            if (psFifo->ui32DataIndex < DEVAIO_DEBUG_MAX_PARAMS)
            {
                psFifo->aui32DebugData[psFifo->ui32DataIndex++] = ui32Val;
            }
            else
            {
                REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
                       "Max number of firmware performance parameters exceeded!");
            }
#if defined(FW_PERFORMANCE_LOGGING)
            if(psFifo->ui32ParamsCount == 0)
            {
                /* Process perfomance data. */
                devaio_perfInfo(psFifo->aui32DebugData);
                psFifo->ui32Collecting = DEBUG_DATA_TYPE_HEADER;
            }
#endif
            break;
        }

        /* String */
        case DEBUG_DATA_TYPE_STRING:
        {
            IMG_ASSERT(0 == "DEBUG_DATA_TYPE_STRING is no longer supported!");
            break;
        }

        /* Debug message chunk */
        case DEBUG_DATA_TYPE_MSG:
        {
            psFifo->ui32ParamsCount = DEBUG_DATA_MSG_GET_ARG_COUNT(psFifo->aui32DebugData[psFifo->ui32DataIndex++]);
            psFifo->aui32DebugData[psFifo->ui32DataIndex++] = ui32Val;
            psFifo->ui32Collecting = DEBUG_DATA_TYPE_PARAMS;
            break;
        }

        /* Debug message chunk parameters */
        case DEBUG_DATA_TYPE_PARAMS:
        {
            if (psFifo->ui32ParamsCount > 0)
            {
                psFifo->ui32ParamsCount--;
                if (psFifo->ui32DataIndex < DEVAIO_DEBUG_MAX_PARAMS)
                {
                    psFifo->aui32DebugData[psFifo->ui32DataIndex++] = ui32Val;
                }
                else
                {
                    REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING,
                           "Max number of firmware debug message parameters exceeded!");
                }
            }
            else
            {
                /*Collect the stack pointer/ task pointer*/
                psFifo->aui32DebugData[psFifo->ui32DataIndex] = ui32Val;
                devaio_ProcessDbgFifoMsgs(psFifo);
                psFifo->ui32Collecting = DEBUG_DATA_TYPE_HEADER;
            }
            break;
        }
    }
}

/*!
******************************************************************************

 @Function              DEVAIO_HandleDebugFifo

******************************************************************************/
IMG_VOID DEVAIO_HandleDebugFifo(
    DEVAIO_sContext  * psContext
)
{
    DEVAIO_DebugFifo * psFifo;
    IMG_UINT32  ui32FifoCtrl;
    IMG_UINT8   ui8Pipe = 0;

    IMG_ASSERT(psContext);
    if(psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return;
    }

    psFifo = &psContext->sDebugFifo;

    if(IMG_TRUE != psFifo->bReady)
    {
        return;
    }

    DEVAIO_DisableDebugFifo(psContext);

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       " + Processing debug fifo");
#endif

    do
    {
        IMG_UINT32 ui32Words, ui32FifoCount;
        ui32FifoCtrl = PVDECIO_ReadRegister(psContext->hPvdecIoCtx,
                                            REGION_PVDEC_CORE_REGSPACE,
                                            PVDEC_CORE_CR_PROC_DEBUG_FIFO_CTRL_0_OFFSET,
                                            ui8Pipe);
        ui32FifoCount = REGIO_READ_FIELD(ui32FifoCtrl,
            PVDEC_CORE, CR_PROC_DEBUG_FIFO_CTRL_0, CR_PROC_DEBUG_FIFO_COUNT);

        if(ui32FifoCount == 0)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_WARNING, "Debug fifo overflow.");
        }

        ui32Words = psFifo->ui32FifoSize - ui32FifoCount;
        if(ui32Words == 0)
        {
            break;
        }

        while(ui32Words--)
        {
            IMG_UINT32 ui32Val = PVDECIO_ReadRegister(psContext->hPvdecIoCtx,
                                                      REGION_PVDEC_CORE_REGSPACE,
                                                      PVDEC_CORE_CR_PROC_DEBUG_FIFO_OFFSET,
                                                      ui8Pipe);

            devaio_HandleDebugFifoData(psContext, ui32Val);
        }
    }
    while(1);

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       " + Processing debug fifo done");
#endif
    DEVAIO_EnableDebugFifo(psContext);
}

/*!
******************************************************************************

 @Function              DEVAIO_CloseDebugFifo

******************************************************************************/
IMG_RESULT DEVAIO_CloseDebugFifo(
    DEVAIO_sContext  * psContext
)
{
    DEVAIO_sDbgTaskDesc *psDbgNextTask, *psDbgTask;

    IMG_ASSERT(psContext);
    if(psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psDbgTask = LST_first(&psContext->sDebugFifo.asDbgTasks);

#if !defined(SYSBRG_BRIDGING)
    if (psContext->sDebugFifo.hFile != IMG_NULL)
    {
        IMG_FILEIO_CloseFile(psContext->sDebugFifo.hFile);
    }
#else /* def SYSBRG_BRIDGING */
    if(IMG_NULL != psContext->sDebugFifo.hTimer)
    {
        /* Destroy the debug FIFO handling timer. */
        SYSOSKM_DestroyTimer(psContext->sDebugFifo.hTimer);
        psContext->sDebugFifo.hTimer = IMG_NULL;
    }
#endif /* def SYSBRG_BRIDGING */

    DEVAIO_DisableDebugFifo(psContext);

    while (psDbgTask)
    {
        psDbgNextTask = LST_next(psDbgTask);
        IMG_FREE(psDbgTask);
        psDbgTask = psDbgNextTask;
    }

    LST_init(&psContext->sDebugFifo.asDbgTasks);

    return IMG_SUCCESS;
}

/*!
******************************************************************************

 @Function              DEVAIO_DisableDebugFifo

******************************************************************************/
IMG_VOID DEVAIO_DisableDebugFifo(
    DEVAIO_sContext * psContext
)
{
    DEVAIO_DebugFifo * psFifo;

    IMG_ASSERT(psContext);
    if(psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return;
    }
    psFifo = &psContext->sDebugFifo;

    psFifo->bReady = IMG_FALSE;
}

#ifdef VDEC_USE_PVDEC_SEC
/*!
******************************************************************************

 @Function              DEVAIO_SelectSecureFwLogs

******************************************************************************/
IMG_VOID DEVAIO_SelectSecureFwLogs(
    DEVAIO_sContext * psContext
)
{
    DEVAIO_DebugFifo * psFifo;

    IMG_ASSERT(psContext);
    if(psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return;
    }

    psFifo = &psContext->sDebugFifo;

    psFifo->bSecureFwLogs = IMG_TRUE;
}
#endif

/*!
******************************************************************************

 @Function              DEVAIO_EnableDebugFifo

******************************************************************************/
IMG_VOID DEVAIO_EnableDebugFifo(
    DEVAIO_sContext * psContext
)
{
    DEVAIO_DebugFifo * psFifo;

    IMG_ASSERT(psContext);
    if(psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return;
    }
    psFifo = &psContext->sDebugFifo;

    psFifo->bReady = IMG_TRUE;
}

/*!
******************************************************************************

 @Function              DEVAIO_GetDebugFifoSize

******************************************************************************/
IMG_UINT32 DEVAIO_GetDebugFifoSize(
    DEVAIO_sContext  * psContext
)
{
    DEVAIO_DebugFifo * psFifo;

    IMG_ASSERT(psContext);
    if(psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    psFifo = &psContext->sDebugFifo;

    return psFifo->ui32FifoSize;
}

#endif /* (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)) */

/*!
******************************************************************************

 @Function              devaio_fnTimer

******************************************************************************/
static IMG_BOOL devaio_fnTimer(
    IMG_VOID *  pvParam
)
{
    IMG_HANDLE  hEventHandle = (IMG_HANDLE)pvParam;

    /* Wake up the suspended process...*/
    SYSOSKM_SignalEventObject(hEventHandle);

    /* Indicate that we want to be rescheduled...*/
    return IMG_TRUE;
}


#ifdef PVDEC_DEBUG_FACILITY
/*
******************************************************************************

 @Function              devaio_dumpRegisters

 @Description

 This function dumps some of register spaces, MTX registers and MTX RAM.
 Can be used for debuging purposes, to diagnose FW/HW bring-up issues.

******************************************************************************/
static IMG_RESULT devaio_dumpRegisters(
        DEVAIO_sContext  * psContext
)
{
    #define DEVAIO_MAX_REGSPACE_WRDS 0x1000
    #define DEVAIO_MTX_CORE_MEMORY  0x18

    #define DEVAIO_PRINT_MTX_REG(_NAME_) \
        do { \
            IMG_UINT32 ui32Value; \
            PVDECIO_WriteRegister(psContext->hPvdecIoCtx, REGION_PVDEC_MTX_CORE_REGSPACE, \
                    MTX_CORE_CR_MTX_REGISTER_READ_WRITE_REQUEST_OFFSET, ui32RegVal, \
                    0xffffffff, 0); \
            ui32Value = PVDECIO_ReadRegister(psContext->hPvdecIoCtx, REGION_PVDEC_MTX_CORE_REGSPACE, \
                    MTX_CORE_CR_MTX_REGISTER_READ_WRITE_DATA_OFFSET, 0); \
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, _NAME_ ": 0x%08x", ui32Value); \
        } while (0);

    typedef struct _DEVAIO_regspace {
        const IMG_UINT8 * pui8Name;
        MEM_REGIONS_PVDEC eMemReg;
        IMG_UINT32 ui32Size;
    } DEVAIO_regspace;

    DEVAIO_regspace asRegSpaces[] = {
        { (IMG_UINT8 *)"CORE", REGION_PVDEC_CORE_REGSPACE, 0x3ff },
        { (IMG_UINT8 *)"BUS4_MMU", REGION_IMG_VIDEO_BUS4_MMU_REGSPACE, 0x1ff },
        { (IMG_UINT8 *)"VLR", REGION_PVDEC_VLR_REGSPACE, 0xfff },
        { (IMG_UINT8 *)"PIXEL PIPE", REGION_PVDEC_PIXEL_PIPE_REGSPACE, 0x1ff },
        { (IMG_UINT8 *)"PIXEL DMA", REGION_PVDEC_PIXEL_DMA_REGSPACE, 0x1ff },
        /* CSIM aborts when register 0x34c is read without active DMA. */
        { (IMG_UINT8 *)"MTX CORE", REGION_PVDEC_MTX_CORE_REGSPACE, 0x3ff /* 0x34c */ }
    };

    IMG_UINT32 ui32i, ui32RegVal, ui32RamId;
    IMG_RESULT ui32Result = IMG_SUCCESS;
    IMG_HANDLE * pahMemSpaces = IMG_NULL;
    IMG_UINT32 ui32MTXBankSize, ui32RamInfo;
    IMG_UINT32 ui32NumWrdsToDump = 0x8000, ui32DumpOffsetWrds = 0x0;

    IMG_UINT32 * pui32Buf = IMG_MALLOC(DEVAIO_MAX_REGSPACE_WRDS*sizeof(IMG_UINT32));

    if (pui32Buf == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to allocate buffer for registers dump!");
        return IMG_ERROR_MALLOC_FAILED;
    }

    pahMemSpaces = IMG_MALLOC(REGION_PVDEC_MAX * sizeof(IMG_HANDLE));
    if (pahMemSpaces == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
               "Memory allocation for mem spaces failed.");
        IMG_FREE(pui32Buf);
        pui32Buf = IMG_NULL;
        return IMG_ERROR_MALLOC_FAILED;
    }
    IMG_MEMSET(pahMemSpaces, 0, REGION_PVDEC_MAX * sizeof(IMG_HANDLE));

    ui32Result = PVDECIO_GetMemSpaces(pahMemSpaces);
    if (IMG_SUCCESS != ui32Result)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to get register spaces handles!");
        goto finish;
    }

    for (ui32i = 0; ui32i < sizeof(asRegSpaces)/sizeof(DEVAIO_regspace); ui32i++)
    {
        IMG_UINT32 ui32Word, ui32Offset, ui32SizeWrds = asRegSpaces[ui32i].ui32Size/sizeof(IMG_UINT32);
        IMG_UINT64 ui64RegspaceOffset;

        ui32Result = TAL_GetMemSpaceOffset(pahMemSpaces[asRegSpaces[ui32i].eMemReg],
                                           &ui64RegspaceOffset);
        if (IMG_SUCCESS != ui32Result)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to get offset of %d memspace.",
                   asRegSpaces[ui32i].eMemReg);
            goto finish;
        }
        ui32Offset = IMG_UINT64_TO_UINT32(ui64RegspaceOffset);

        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Dumping register space %s (id %d), offset: %08x, size 0x%08x",
               asRegSpaces[ui32i].pui8Name, asRegSpaces[ui32i].eMemReg, ui32Offset, asRegSpaces[ui32i].ui32Size);

        if (ui32SizeWrds > DEVAIO_MAX_REGSPACE_WRDS)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Size of register space is larger than"
                   " supplied buffer (%u > %u), clipping!", ui32SizeWrds, DEVAIO_MAX_REGSPACE_WRDS);
            ui32SizeWrds = DEVAIO_MAX_REGSPACE_WRDS;
        }

        IMG_MEMSET(pui32Buf, 0, DEVAIO_MAX_REGSPACE_WRDS*sizeof(IMG_UINT32));
        ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                        asRegSpaces[ui32i].eMemReg,
                                        0,
                                        ui32SizeWrds,
                                        pui32Buf,
                                        IMG_FALSE);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to read register area %d",
                   asRegSpaces[ui32i].eMemReg);
            goto finish;
        }

        for (ui32Word = 0; ui32Word < ui32SizeWrds; ui32Word+=4)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x",
                   ui32Offset + ui32Word*sizeof(IMG_UINT32),
                   pui32Buf[ui32Word],
                   (ui32Word + 1) < ui32SizeWrds ? pui32Buf[ui32Word + 1] : 0,
                   (ui32Word + 2) < ui32SizeWrds ? pui32Buf[ui32Word + 2] : 0,
                   (ui32Word + 3) < ui32SizeWrds ? pui32Buf[ui32Word + 3] : 0);
        }
    }

    ui32RegVal = 0;
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RNW, 1);         // Read
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_USPECIFIER, 5);  // PC or PCX
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RSPECIFIER, 0);  // PC
    DEVAIO_PRINT_MTX_REG("MTX PC");

    ui32RegVal = 0;
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RNW, 1);         // Read
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_USPECIFIER, 5);  // PC or PCX
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RSPECIFIER, 1);  // PCX
    DEVAIO_PRINT_MTX_REG("MTX PCX");

    ui32RegVal = 0;
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RNW, 1);         // Read
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_USPECIFIER, 3);  // A0StP
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RSPECIFIER, 0);
    DEVAIO_PRINT_MTX_REG("MTX A0STP");

    ui32RegVal = 0;
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RNW, 1);         // Read
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_USPECIFIER, 3);  // A0FrP
    REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_REGISTER_READ_WRITE_REQUEST, MTX_RSPECIFIER, 1);
    DEVAIO_PRINT_MTX_REG("MTX A0FRP");

    ui32RamInfo = PVDECIO_ReadRegister(psContext->hPvdecIoCtx, REGION_PVDEC_CORE_REGSPACE,
                                       PVDEC_CORE_CR_PROC_DEBUG_OFFSET, 0);
    ui32MTXBankSize = REGIO_READ_FIELD(ui32RamInfo, PVDEC_CORE, CR_PROC_DEBUG, CR_MTX_RAM_BANK_SIZE);
    ui32MTXBankSize = 1 << (ui32MTXBankSize + 2);

    REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Trying to dump 0x%x words from MTX RAM at 0x%x, bank: 0x%x",
           ui32NumWrdsToDump, ui32DumpOffsetWrds, ui32MTXBankSize);
    ui32RamId = 0;
    for (ui32i = ui32DumpOffsetWrds; ui32i < ui32NumWrdsToDump; ui32i+=4)
    {
        IMG_UINT32 aui32Value[4], ui32j, ui32Addr;

        for (ui32j = 0; ui32j < 4; ui32j++)
        {
            ui32RegVal = 0, ui32Addr = (ui32i + ui32j)*sizeof(IMG_UINT32);

            if ((DEVAIO_MTX_CORE_MEMORY + (ui32Addr / ui32MTXBankSize)) != ui32RamId)
            {
                ui32RamId = (DEVAIO_MTX_CORE_MEMORY + (ui32Addr / ui32MTXBankSize));
            }

            REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_RAM_ACCESS_CONTROL, MTX_MCMID, ui32RamId);
            REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_RAM_ACCESS_CONTROL, MTX_MCM_ADDR, ui32i + ui32j);
            REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_RAM_ACCESS_CONTROL, MTX_MCMAI, 0);
            REGIO_WRITE_FIELD(ui32RegVal, MTX_CORE, CR_MTX_RAM_ACCESS_CONTROL, MTX_MCMR, 1);

            PVDECIO_WriteRegister(psContext->hPvdecIoCtx, REGION_PVDEC_MTX_CORE_REGSPACE,
                                  MTX_CORE_CR_MTX_RAM_ACCESS_CONTROL_OFFSET, ui32RegVal,
                                  0xffffffff, 0);

            aui32Value[ui32j] = PVDECIO_ReadRegister(psContext->hPvdecIoCtx, REGION_PVDEC_MTX_CORE_REGSPACE,
                                                     MTX_CORE_CR_MTX_RAM_ACCESS_DATA_EXCHANGE_OFFSET, 0);
        }
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x",
               ui32i*sizeof(IMG_UINT32), aui32Value[0], aui32Value[1], aui32Value[2], aui32Value[3]);
    }

finish:
    IMG_FREE(pahMemSpaces);
    pahMemSpaces = IMG_NULL;
    IMG_FREE(pui32Buf);
    pui32Buf = IMG_NULL;
    return ui32Result;
}
#endif /* PVDEC_DEBUG_FACILITY */


/*
******************************************************************************

 @Function              devaio_GetCommsConfigFromMTX

******************************************************************************/
static IMG_RESULT devaio_GetCommsConfigFromMTX(
    DEVAIO_sContext  * psContext
)
{
    IMG_RESULT  ui32Result;
    IMG_UINT32  ui32Reg;

    IMG_ASSERT(psContext);

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       "Poll for FW signature");
#endif

    ui32Result = PVDECIO_Poll(psContext->hPvdecIoCtx,
                              REGION_PVDEC_VLRFE_REGSPACE,
                              PVDEC_COM_RAM_SIGNATURE_OFFSET,
                              FW_READY_SIGNATURE,
                              0xffffffff,
                              VXDIO_POLL_EQUAL);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to poll for firmware signature!");
#ifdef PVDEC_DEBUG_FACILITY
        if (IMG_SUCCESS != devaio_dumpRegisters(psContext))
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to dump registers!");
        }
#endif /* PVDEC_DEBUG_FACILITY */
        return ui32Result;
    }

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       "Getting comms config from MTX");
#endif

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_TO_MTX_BUF_SIZE_AND_OFFSET_OFFSET,
                                    1,
                                    &ui32Reg,
                                    IMG_FALSE);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to read buffer size and offset!");
        return ui32Result;
    }

    psContext->sToMtxBuf.ui32Size   = PVDEC_COM_RAM_BUF_GET_SIZE(ui32Reg, TO_MTX);
    psContext->sToMtxBuf.ui32Offset = PVDEC_COM_RAM_BUF_GET_OFFSET(ui32Reg, TO_MTX);

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_TO_HOST_BUF_SIZE_AND_OFFSET_OFFSET,
                                    1,
                                    &ui32Reg,
                                    IMG_FALSE);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to read buffer size and offset!");
        return ui32Result;
    }

    psContext->sToHostBuf.ui32Size   = PVDEC_COM_RAM_BUF_GET_SIZE(ui32Reg, TO_HOST);
    psContext->sToHostBuf.ui32Offset = PVDEC_COM_RAM_BUF_GET_OFFSET(ui32Reg, TO_HOST);

    psContext->bReady = IMG_TRUE;

    return IMG_SUCCESS;
}


/*!
******************************************************************************

 @Function              devaio_CommsWriteWords

******************************************************************************/
static IMG_VOID devaio_CommsWriteWords(
    DEVAIO_sContext *  psContext,
    IMG_UINT32         ui32Offset,
    IMG_UINT32         ui32NumWords,
    IMG_UINT32 *       pui32Values,
    MEM_REGIONS_PVDEC  eMemSpace
)
{
    IMG_ASSERT(psContext);
#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, eMemSpace,
                       "Writing data to VLR");
#endif
    PVDECIO_VLRWriteWords(psContext->hPvdecIoCtx, eMemSpace,
                        ui32Offset, ui32NumWords, pui32Values);
}


/*!
******************************************************************************

 @Function              devaio_CommsReadWords

******************************************************************************/
static IMG_VOID devaio_CommsReadWords(
    DEVAIO_sContext  * psContext,
    IMG_UINT32         ui32Offset,
    IMG_UINT32         ui32NumWords,
    IMG_UINT32       * pui32Values,
    IMG_BOOL           bValidate
)
{
    IMG_ASSERT(psContext);
#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                       "Reading data from VLR");
#endif
    PVDECIO_VLRReadWords(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                       ui32Offset, ui32NumWords, pui32Values, bValidate);
}

static IMG_BOOL devaio_IsFwStatusOk(
    DEVAIO_sContext *  psContext
)
{
    IMG_UINT32  ui32FwStatus = 0;
    IMG_UINT32  ui32FwSignature = 0;
    IMG_RESULT  ui32Result;

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                REGION_PVDEC_VLRFE_REGSPACE,
                                PVDEC_COM_RAM_FW_STATUS_OFFSET,
                                1,
                                &ui32FwStatus,
                                IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                REGION_PVDEC_VLRFE_REGSPACE,
                                PVDEC_COM_RAM_SIGNATURE_OFFSET,
                                1,
                                &ui32FwSignature,
                                IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    return (ui32FwStatus != FW_STATUS_PANIC) &&
           (ui32FwStatus != FW_STATUS_ASSERT) &&
           (ui32FwStatus != FW_STATUS_GAMEOVER) &&
           (ui32FwSignature == FW_READY_SIGNATURE);
}


/*!
******************************************************************************

 @Function              devaio_WaitForCommsSpace

******************************************************************************/
static IMG_VOID devaio_WaitForCommsSpace(
    DEVAIO_sContext *  psContext,
    IMG_UINT32         ui32WriteIdx,
    IMG_UINT32         ui32MessageSize,
    IMG_UINT32         ui32BufferSize,
    IMG_HANDLE         hPvdecIoCtx
)
{
    IMG_UINT32  ui32ReadIndex;
    IMG_UINT32  ui32WaitRetry = MSVDX_MAX_NUM_COMMS_SPACE_RETRIES;

    /* setup wait object */
    IMG_HANDLE  hEventHandle = IMG_NULL;
    IMG_HANDLE  hTimerHandle = IMG_NULL;
    IMG_RESULT  ui32Result;

    IMG_ASSERT(psContext);

    /* create an event object */
    ui32Result = SYSOSKM_CreateEventObject( &hEventHandle );
    IMG_ASSERT( ui32Result == IMG_SUCCESS );
    if ( ui32Result != IMG_SUCCESS )
    {
        return;
    }
    /* create a timer and attach to the event object */
    ui32Result = SYSOSKM_CreateTimer(devaio_fnTimer, hEventHandle, 1, &hTimerHandle);
    IMG_ASSERT( ui32Result == IMG_SUCCESS );
    if ( ui32Result != IMG_SUCCESS )
    {
        SYSOSKM_DestroyEventObject(hEventHandle);
        return;
    }

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       " + Waiting for free space in VLR comms area");
#endif
    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_TO_MTX_RD_INDEX_OFFSET,
                                    1,
                                    &ui32ReadIndex,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    while (
           devaio_IsFwStatusOk(psContext)
           &&
           (ui32WaitRetry--)
           &&
           (
            /* write pointer will overtake read pointer */
            (
             (ui32WriteIdx < ui32ReadIndex) &&
             ((ui32WriteIdx + ui32MessageSize) >= ui32ReadIndex)
            )
            ||
            /* or message will fill buffer and hit read pointer at start of buffer */
            (
             (ui32ReadIndex == 0) &&
             ((ui32WriteIdx + ui32MessageSize) == ui32BufferSize)
            )
           )
          )
    {
        /* wait for a millisecond */
        SYSOSKM_WaitEventObject( hEventHandle, IMG_TRUE );

        ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                        REGION_PVDEC_VLRFE_REGSPACE,
                                        PVDEC_COM_RAM_TO_MTX_RD_INDEX_OFFSET,
                                        1,
                                        &ui32ReadIndex,
                                        IMG_FALSE);
    }

    /* remove the timer before the event object */
    /* remove wait object */
    if ( hTimerHandle )
    {
        SYSOSKM_DestroyTimer( hTimerHandle );
    }

    /* remove wait object */
    if ( hEventHandle )
    {
        SYSOSKM_DestroyEventObject( hEventHandle );
    }

#if !defined(SYSBRG_BRIDGING)
    {
        PVDECIO_PDUMPPollCircBuff(psContext->hPvdecIoCtx,
                                REGION_PVDEC_VLRFE_REGSPACE,
                                PVDEC_COM_RAM_TO_MTX_RD_INDEX_OFFSET,
                                ui32WriteIdx,
                                ui32MessageSize,
                                ui32BufferSize);
    }
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       " - Waiting for free space in VLR comms area done");
#endif
}


/*!
******************************************************************************

 @Function              devaio_CopyMessageData

******************************************************************************/
static IMG_UINT32 devaio_CopyMessageData(
    DEVAIO_sContext  * psContext,
    IMG_UINT32         ui32ReadIdx,
    IMG_UINT32         ui32HostWriteIdx,
    IMG_UINT32         ui32HostReadIdx,
    IMG_UINT32         ui32MessageSize,
    IMG_UINT32         ui32BufferSize,
    VXDIO_sHISRMsg   * psMessage,
    IMG_UINT32       * pui32CopyMsgBuf
)
{
    IMG_ASSERT(psContext);
    IMG_ASSERT(psMessage);
    IMG_ASSERT(pui32CopyMsgBuf);

    /* set the write index into the internal DEVAIO buffer */
    if ( (ui32HostWriteIdx + ui32MessageSize) > MSVDX_SIZE_MSG_BUFFER )
    {
        ui32HostWriteIdx = 0;
    }

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                       " + Obtaining message Data");
#endif

    /* sanity check - message will fit in internal MTXIO buffer */
    IMG_ASSERT( (ui32HostWriteIdx + ui32MessageSize) <= MSVDX_SIZE_MSG_BUFFER );
    IMG_ASSERT( (ui32HostReadIdx <= ui32HostWriteIdx) || (ui32HostWriteIdx + ui32MessageSize) <= ui32HostReadIdx);

    /* record the location of the message in the internal DEVAIO buffer */
    psMessage->ui32MsgIndex = ui32HostWriteIdx;
    psMessage->ui32MsgSize = ui32MessageSize;

    /* if the message wraps in the MTX buffer, read the portion the the end of the buffer */
    if ( (ui32ReadIdx + ui32MessageSize) >= ui32BufferSize )
    {
        IMG_UINT32 ui32FirstPartSize = ui32BufferSize - ui32ReadIdx;

        devaio_CommsReadWords(psContext,
                              psContext->sToHostBuf.ui32Offset + (ui32ReadIdx << 2),
                              ui32FirstPartSize,
                              &pui32CopyMsgBuf[ui32HostWriteIdx],
                              IMG_TRUE);

        ui32ReadIdx = 0;
        ui32MessageSize -= ui32FirstPartSize;
        ui32HostWriteIdx += ui32FirstPartSize;
    }

    if (ui32MessageSize > 0)
    {
        devaio_CommsReadWords(psContext,
                              psContext->sToHostBuf.ui32Offset + (ui32ReadIdx << 2),
                              ui32MessageSize,
                              &pui32CopyMsgBuf[ui32HostWriteIdx],
                              IMG_TRUE);
    }

    ui32ReadIdx += ui32MessageSize;
    ui32HostWriteIdx += ui32MessageSize;

    /* update DEVAIO internal buffer write index */
    psMessage->ui32NewWriteIndex = ui32HostWriteIdx;

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                         " - Obtaining message Data done");
#endif

    return ui32ReadIdx;
}


/*!
******************************************************************************

 @Function              DEVAIO_InitMTXComms

******************************************************************************/
IMG_RESULT DEVAIO_InitMTXComms(
    IMG_HANDLE       * hPvdecIoCtx,
    DEVAIO_sContext  * psContext,
    IMG_BOOL           bSecureFw
)
{
    IMG_RESULT  ui32Result = IMG_SUCCESS;
    IMG_UINT32  ui32FwDbgLevel = 0xffffffff;
    IMG_UINT32  ui32FwSignature = 0;
    IMG_UINT32  ui32FwFlags = FWFLAG_FORCE_FS_FLOW |
                              FWFLAG_DISABLE_GENC_FLUSHING |
                              FWFLAG_DISABLE_AUTONOMOUS_RESET
#ifdef FW_PERFORMANCE_LOGGING
                             | FWFLAG_DISABLE_IDLE_GPIO
#endif
#ifdef ERROR_CONCEALMENT
                             | FWFLAG_ENABLE_CONCEALMENT
#endif
    ;

    IMG_ASSERT(hPvdecIoCtx);
    IMG_ASSERT(psContext);
    if(hPvdecIoCtx == IMG_NULL ||
       psContext == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameters for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    /* In secure flow, messages which we return from MTX are much bigger
     * than in in case of regular flow. Configure firmware to increase
     * buffer in comms RAM used to send messages from MTX to host. */
    if (bSecureFw)
    {
        ui32FwFlags |= FWFLAG_BIG_TO_HOST_BUFFER;
#ifdef VDEC_USE_PVDEC_SEC
        /* Assuming that secure memory is available */
        ui32FwFlags |= FWFLAG_ENABLE_SECURITY;
#endif
    }

#if !defined(SYSBRG_BRIDGING)
    /* check DBGOPTS for ConsistentMultiPipeFlow property */
    {
        DBGOPT_sValue sVal;
        if (DBGOPTKM_GetSize("ConsistentMultiPipeFlow", DBGOPT_TYPE_BOOL) &&
            DBGOPTKM_Get("ConsistentMultiPipeFlow", IMG_NULL, &sVal))
        {
            ui32FwFlags |= FWFLAG_CONSISTENT_MULTIPIPE_FLOW;
        }
    }
#endif

    psContext->hPvdecIoCtx = hPvdecIoCtx;

    /* Set firmware debug level */
    devaio_CommsWriteWords(psContext, PVDEC_COM_RAM_FW_ID_OFFSET,
                           1, &ui32FwDbgLevel, REGION_PVDEC_VLRFE_REGSPACE);
    /* Set firmware flags */
    devaio_CommsWriteWords(psContext, PVDEC_COM_RAM_FLAGS_OFFSET,
                           1, &ui32FwFlags, REGION_PVDEC_VLRFE_REGSPACE);
    /* Reset signature. */
    devaio_CommsWriteWords(psContext, PVDEC_COM_RAM_SIGNATURE_OFFSET,
                           1, &ui32FwSignature, REGION_PVDEC_VLRFE_REGSPACE);

#if (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING))
    /* Initialise debug FIFO. */
    ui32Result = devaio_InitDebugFifo(psContext);
    if (ui32Result != IMG_SUCCESS)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to initialize debug FIFO context!");
    }
#endif /* (defined(IMG_DEBUG_FLAG) || defined(FW_PERFORMANCE_LOGGING)) */

    return ui32Result;
}


/*
******************************************************************************

 @Function              DEVAIO_SendMTXMsg

******************************************************************************/
IMG_RESULT DEVAIO_SendMTXMsg(
    DEVAIO_sContext   * psContext,
    const IMG_VOID    * psMsgHdr,
    const IMG_HANDLE    hPvdecIoCtx
)
{
    IMG_RESULT    ui32Result;
    IMG_UINT32    ui32NumTxWords, ui32ReadIndex, ui32WriteIndex;
    IMG_UINT32  * pui32Msg = (IMG_UINT32*)psMsgHdr;

    IMG_ASSERT(psContext);
    IMG_ASSERT(psMsgHdr);
    if(psContext == IMG_NULL||
       psMsgHdr == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameter for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }


    if (!psContext->bReady)
    {
        ui32Result = devaio_GetCommsConfigFromMTX(psContext);
        if (ui32Result != IMG_SUCCESS)
        {
            REPORT(REPORT_MODULE_VXDIO, REPORT_ERR, "Failed to initialize communication area!");
            return ui32Result;
        }
    }

    ui32NumTxWords = MEMIO_READ_FIELD(pui32Msg, FW_DEVA_GENMSG_MSG_SIZE);

    IMG_ASSERT(ui32NumTxWords <= psContext->sToMtxBuf.ui32Size);

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       " + Sending message to MTX");
#endif

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_TO_MTX_RD_INDEX_OFFSET,
                                    1,
                                    &ui32ReadIndex,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_TO_MTX_WRT_INDEX_OFFSET,
                                    1,
                                    &ui32WriteIndex,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    /* Will the message wrap, if so need to send a pad message */
    if (ui32WriteIndex + ui32NumTxWords > psContext->sToMtxBuf.ui32Size)
    {
        IMG_UINT8 aui8Msg[FW_DEVA_GENMSG_SIZE]; // buffer for FW message
        IMG_INT32 i32PaddingWords = psContext->sToMtxBuf.ui32Size - ui32WriteIndex;
        IMG_UINT32 ui32MsgSizeWords = FW_DEVA_GENMSG_SIZE/4;
        IMG_ASSERT(i32PaddingWords > 0);
        IMG_MEMSET(&aui8Msg[0], 0, sizeof(aui8Msg));

        MEMIO_WRITE_FIELD(&aui8Msg[0], FW_DEVA_GENMSG_MSG_SIZE, i32PaddingWords);
        MEMIO_WRITE_FIELD(&aui8Msg[0], FW_DEVA_GENMSG_MSG_TYPE, MSG_TYPE_PADDING);

        devaio_WaitForCommsSpace(psContext, ui32WriteIndex, i32PaddingWords, psContext->sToMtxBuf.ui32Size, hPvdecIoCtx);

        devaio_CommsWriteWords(psContext, psContext->sToMtxBuf.ui32Offset + (ui32WriteIndex << 2),
                               ui32MsgSizeWords, (IMG_UINT32 *)&aui8Msg[0], REGION_PVDEC_VLRFE_REGSPACE);

        ui32WriteIndex = 0;
        devaio_CommsWriteWords(psContext, PVDEC_COM_RAM_TO_MTX_WRT_INDEX_OFFSET,
                               1, &ui32WriteIndex, REGION_PVDEC_VLRFE_REGSPACE);

        PVDECIO_KickMTX(hPvdecIoCtx);
    }

    devaio_WaitForCommsSpace(psContext, ui32WriteIndex, ui32NumTxWords, psContext->sToMtxBuf.ui32Size, hPvdecIoCtx);

    devaio_CommsWriteWords(psContext, psContext->sToMtxBuf.ui32Offset + (ui32WriteIndex << 2),
                           ui32NumTxWords, pui32Msg, REGION_PVDEC_VLRFE_REGSPACE);

    ui32WriteIndex += ui32NumTxWords;

    if (ui32WriteIndex == psContext->sToMtxBuf.ui32Size)
    {
        ui32WriteIndex = 0;
    }

    devaio_CommsWriteWords(psContext, PVDEC_COM_RAM_TO_MTX_WRT_INDEX_OFFSET,
                           1, &ui32WriteIndex, REGION_PVDEC_VLRFE_REGSPACE);

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRFE_REGSPACE,
                       " - Sending message to MTX done");
#endif

    return IMG_SUCCESS;
}


/*
******************************************************************************

 @Function              DEVAIO_ProcessMTXMsgs

******************************************************************************/
IMG_RESULT DEVAIO_ProcessMTXMsgs(
    DEVAIO_sContext  * psContext,
    VXD_sMsgQueue    * psMsgQStatus
)
{
    IMG_UINT32        ui32ReadIndex, ui32WriteIndex, ui32BufferSize, ui32FirstWord;
    IMG_UINT32        ui32MessageSize; /* in bytes */
    IMG_UINT16        ui16MsgType;
    IMG_UINT32      * pui32FirstWord;
    VXDIO_sHISRMsg  * psMessage;
    IMG_RESULT        ui32Result;

    IMG_ASSERT(psContext);
    IMG_ASSERT(psMsgQStatus);
    if(psContext == IMG_NULL ||
       psMsgQStatus == IMG_NULL)
    {
        REPORT(REPORT_MODULE_VXDIO, REPORT_ERR,
              "Invalid parameters for %s", __FUNCTION__);
        return IMG_ERROR_INVALID_PARAMETERS;
    }

    IMG_ASSERT(!LST_empty(&psMsgQStatus->sFreeMsgList));
    IMG_ASSERT(LST_empty(&psMsgQStatus->sNewMsgList));

    // Initialise the output flags.
    psMsgQStatus->bEmpty = IMG_FALSE;
    psMsgQStatus->bQueued = IMG_FALSE;

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                       " + Processing MTX message");
#endif

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRBE_REGSPACE,
                                    PVDEC_COM_RAM_TO_HOST_RD_INDEX_OFFSET,
                                    1,
                                    &ui32ReadIndex,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRBE_REGSPACE,
                                    PVDEC_COM_RAM_TO_HOST_WRT_INDEX_OFFSET,
                                    1,
                                    &ui32WriteIndex,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    ui32BufferSize = psContext->sToHostBuf.ui32Size;

    IMG_MEMSET(&ui32FirstWord, 0, sizeof(ui32FirstWord));

    /* While there are messages to read and somewhere to put them. */
    while (ui32ReadIndex != ui32WriteIndex &&
           LST_first(&psMsgQStatus->sFreeMsgList))
    {
#if !defined(SYSBRG_BRIDGING)
        PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                           "Wait for FW message");
        /* reproduce message reading in a pdump script */
        PVDECIO_Poll(psContext->hPvdecIoCtx,
                     REGION_PVDEC_VLRBE_REGSPACE,
                     PVDEC_COM_RAM_TO_HOST_WRT_INDEX_OFFSET,
                     ui32ReadIndex,
                     0xffffffff,
                     VXDIO_POLL_NOT_EQUAL);
#endif

        /* read the first word of the message */
        devaio_CommsReadWords(psContext,
                              psContext->sToHostBuf.ui32Offset + (ui32ReadIndex << 2),
                              1,
                              &ui32FirstWord,
                              IMG_TRUE);


        pui32FirstWord = &ui32FirstWord;
        ui32MessageSize = MEMIO_READ_FIELD(pui32FirstWord, FW_DEVA_GENMSG_MSG_SIZE);
        ui16MsgType = MEMIO_READ_FIELD(pui32FirstWord, FW_DEVA_GENMSG_MSG_TYPE);

        /* sanity check - message fits in buffer */
        IMG_ASSERT(ui32MessageSize < ui32BufferSize);
        IMG_ASSERT((ui32ReadIndex > ui32WriteIndex) || ((ui32ReadIndex + ui32MessageSize) <= ui32WriteIndex));
        /* sanity check - message is of the MTX -> Host type */
        IMG_ASSERT(ui16MsgType >= FW_DEVA_COMPLETED);

        psMessage = LST_removeHead(&psMsgQStatus->sFreeMsgList);
        IMG_ASSERT(psMessage);

        /* copy message into internal buffer and get new read index */
        ui32ReadIndex = devaio_CopyMessageData(psContext,
                                               ui32ReadIndex,
                                               psMsgQStatus->ui32WriteIdx,
                                               psMsgQStatus->ui32ReadIdx,
                                               ui32MessageSize,
                                               ui32BufferSize,
                                               psMessage,
                                               psMsgQStatus->aui32MsgBuf);

        psMsgQStatus->ui32WriteIdx = psMessage->ui32NewWriteIndex;

        /* Add the message to the HISR active list */
        LST_add(&psMsgQStatus->sNewMsgList, psMessage);

        /* update the read index */
        devaio_CommsWriteWords(psContext, PVDEC_COM_RAM_TO_HOST_RD_INDEX_OFFSET,
                               1, &ui32ReadIndex, REGION_PVDEC_VLRBE_REGSPACE);

        psMsgQStatus->bQueued = IMG_TRUE;
    }

#if !defined(SYSBRG_BRIDGING)
    PVDECIO_PDUMPComment(psContext->hPvdecIoCtx, REGION_PVDEC_VLRBE_REGSPACE,
                       " - Processing MTX message done");
#endif

    /* report success or otherwise */
    psMsgQStatus->bEmpty = ((ui32ReadIndex == ui32WriteIndex) ? IMG_TRUE: IMG_FALSE);

    return IMG_SUCCESS;
}

/*
******************************************************************************

 @Function              DEVAIO_ProcessMMUReport

******************************************************************************/
IMG_RESULT DEVAIO_ProcessMMUReport(
    DEVAIO_sContext  * psContext,
    IMG_UINT32       * pui32Status0,
    IMG_UINT32       * pui32Status1
)
{
    IMG_RESULT  ui32Result = IMG_SUCCESS;

    *pui32Status0 = 0;
    *pui32Status1 = 0;

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_FW_MMU_REPORT_OFFSET,
                                    1,
                                    pui32Status0,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);

    ui32Result = PVDECIO_VLRReadWords(psContext->hPvdecIoCtx,
                                    REGION_PVDEC_VLRFE_REGSPACE,
                                    PVDEC_COM_RAM_FW_MMU_REPORT_OFFSET + 4,
                                    1,
                                    pui32Status1,
                                    IMG_FALSE);
    IMG_ASSERT(ui32Result == IMG_SUCCESS);
    return ui32Result;
}

/* EOF */
