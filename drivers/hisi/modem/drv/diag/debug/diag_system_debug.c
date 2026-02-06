/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */





/*****************************************************************************
  1 Include HeadFile
*****************************************************************************/
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <securec.h>
#include <osl_thread.h>
#include <osl_types.h>
#include <osl_malloc.h>
#include <osl_sem.h>
#include <bsp_slice.h>
#include <bsp_rfile.h>
#include <securec.h>
#include "OmCommonPpm.h"
#include "scm_ind_src.h"
#include "scm_cnf_src.h"
#include "scm_common.h"
#include "scm_ind_dst.h"
#include "diag_system_debug.h"

/* debug提示信息的长度 */
#define DIAG_DEBUG_INFO_LEN     (32)

#define    DIAG_LOG_PATH       MODEM_LOG_ROOT"/drv/DIAG/"
#define    DIRPAH_LEN  64


DIAG_DRV_DEBUG_INFO_STRU   g_stDiagDrvDebugInfo = {0};

/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/
/*****************************************************************************
  3 Function
*****************************************************************************/

/*****************************************************************************
 Function Name   : PTR : Process Trace Record (流程跟踪记录)
 Description     : 跟踪整个处理流程
*****************************************************************************/
DIAG_PTR_INFO_STRU g_stPtrInfo = {0};
u32 g_DiagLogLevel = 0;

void diag_PTR(DIAG_PTR_ID_ENUM enType, u32 paraMark, u32 para0, u32 para1)
{
    g_stPtrInfo.stPtr[g_stPtrInfo.ulCur].enStep     = (u16)enType;
    g_stPtrInfo.stPtr[g_stPtrInfo.ulCur].paraMark   = (u16)paraMark;
    g_stPtrInfo.stPtr[g_stPtrInfo.ulCur].ulTime = bsp_get_slice_value();
    if(paraMark)
    {
        g_stPtrInfo.stPtr[g_stPtrInfo.ulCur].para[0]= para0;
        g_stPtrInfo.stPtr[g_stPtrInfo.ulCur].para[1]= para1;
    }
    g_stPtrInfo.ulCur = (g_stPtrInfo.ulCur + 1) % DIAG_PTR_NUMBER;
}

char *g_PtrName[EN_DIAG_PTR_CFG_END + 1] =
{
    "begin",            "ppm_rcv",      "cpm_rcv",          "scm_soft",     "scm_self",     "scm_rcv",      "scm_rcv1",     "scm_disp",
    "mspsvic1",         "mspsvic2",     "diagsvc1",         "diagsvc2",     "diagmsg1",     "diagmsg2",     "msgmsp1",      "msptrans",
    "msp_ps1",          "msp_ps2",      "connect",          "disconn",      "msg_rpt",      "svicpack",     "snd_code",     "scm_code",
    "code_dst",         "scm_send",     "cpm_send",         "ppm_send",     "bsp_msg",      "trans_bbp",    "bbp_msg",      "bbp_msg_cnf",
    "bbp_get_chan_size","chan_size_cnf","sample_gen",       "sample_gen_cnf","bbp_get_addr","get_addr_cnf", "bbp_get_version","get_version_cnf",
    "BBP_ABLE_CHAN",    "ABLE_CHAN_CNF","APP_TRANS_PHY",    "DSP_MSG",      "DSP_MSG_CNF",  "TRANS_HIFI",   "HIFI_MSG",     "HIFI_MSG_CNF",
    "SOCP_VOTE",        "APP_TRANS_MSG","FAIL_CMD_CNF",     "PRINT_CFG",    "PRINT_CFG_OK", "PRINT_CFG_FAIL","LAYER_CFG",   "LAYER_CFG_SUCESS",
    "LAYER_CFG_FAIL",   "AIR_CFG",      "AIR_CFG_SUCESS",   "AIR_CFG_FAIL", "EVENT_CFG",    "EVENT_CFG_OK", "EVENT_CFG_FAIL","MSG_CFG",
    "MSG_CFG_SUCESS",   "MSG_CFG_FAIL", "GTR_CFG",          "GTR_CFG_OK",   "GTR_CFG_FAIL", "GUGTR_CFG",    "GET_TIME",     "GET_TIME_STAMP_CNF",
    "GET_MODEM_NUM",    "MODEM_NUM_CNF","GET_PID_TABLE",    "PID_TABLE_CNF","TRANS_CNF",    "FS_QUERY",     "FS_SCAN",      "FS_MAKE_DIR",
    "FS_OPEN",          "FS_IMPORT",    "FS_EXOPORT",       "FS_DELETE",    "FS_SPACE",     "NV_RD",        "NV_RD_OK",     "NV_RD_FAIL",
    "GET_NV_LIST",      "NV_LIST_OK",   "GET_NV_LIST_FAIL", "GET_RESUME_LIST","RESUME_LIST_OK","RESUME_LIST_FAIL","NV_WR",  "NV_WR_SUCESS",
    "NV_WR_FAIL",       "AUTH_NV_CFG",  "NV_AUTH_PROC",     "NV_AUTH_FAIL", "TRANS",        "TRANS_CNF",    "HIGHTS",       "HIGHTS_CNF",
    "USERPLANE",        "USERPLANE_CNF",
};

void DIAG_DebugShowPTR(u32 ulnum)
{
    u32 i, cur;
    DIAG_PTR_INFO_STRU *pPtrTmp;
    u32 ptrInfoSize = 0;
    u32 event = 0;

    ptrInfoSize = sizeof(g_stPtrInfo);

    pPtrTmp = (DIAG_PTR_INFO_STRU *)osl_malloc(ptrInfoSize);
    if(NULL == pPtrTmp)
    {
        return;
    }

    if(memcpy_s(pPtrTmp, ptrInfoSize, &g_stPtrInfo, sizeof(g_stPtrInfo)))
    {
        osl_free(pPtrTmp);
        return;
    }

    cur = (pPtrTmp->ulCur - ulnum + DIAG_PTR_NUMBER)%DIAG_PTR_NUMBER;

    diag_crit("current ptr:0x%x\n", pPtrTmp->ulCur);
    for(i = 0; i < ulnum; i++)
    {
        if(0 != pPtrTmp->stPtr[cur].ulTime)
        {
            if(0 == (i % 20))
            {
                (void)osl_task_delay(10);
            }
            event = pPtrTmp->stPtr[cur].enStep;
            diag_crit("%02d %-20s %08d ms \n", \
                    event, g_PtrName[event], (pPtrTmp->stPtr[cur].ulTime/33));
        }

        cur = (cur + 1) % DIAG_PTR_NUMBER;
    }
    diag_crit("i = %d, over!\n", i);

    osl_free(pPtrTmp);

    return ;
}
EXPORT_SYMBOL(DIAG_DebugShowPTR);

u32 diag_get_usb_type(void)
{
    return (u32)NOT_SUPPORT_QUERY;
}

/*****************************************************************************
 Function Name   : DIAG_DebugPTR
 Description        : DIAG处理流程的打点信息
*****************************************************************************/
void DIAG_DebugPTR(void)
{
    void* pFile;
    u32 ret;
    u32 ulValue;
    s8 *DirPath = DIAG_LOG_PATH;
    s8 *FilePath = DIAG_LOG_PATH"DIAG_PTR.bin";

    /* 如果DIAG目录不存在则先创建目录 */
    if (BSP_OK != mdrv_file_access(DirPath, RFILE_RDONLY))
    {
        if (BSP_OK != mdrv_file_mkdir(DirPath))
        {
            diag_error("mkdir %s failed.\n", DIAG_LOG_PATH);
            return ;
        }
    }

    pFile = mdrv_file_open(FilePath, "wb+");
    if(!pFile)
    {
        diag_error(" bsp_open failed.\n");
        return ;
    }

    ret = DIAG_DebugFileHeader(pFile);
    if(BSP_OK != ret)
    {
        diag_error("DIAG_DebugFileHeader failed .\n");
        (void)mdrv_file_close(pFile);
        return ;
    }

    /* 打点信息长度 */
    ulValue = DIAG_DEBUG_SIZE_FLAG | sizeof(g_stPtrInfo);
    ret = mdrv_file_write(pFile, 1, sizeof(ulValue), (s8 *)&ulValue);
    if(ret != sizeof(ulValue))
    {
        diag_error("write sizeof g_stPtrInfo failed.\n");
    }

    /* 再写入打点信息 */
    ret = mdrv_file_write(pFile, 1, sizeof(g_stPtrInfo), (s8 *)&g_stPtrInfo);
    if(ret != sizeof(g_stPtrInfo))
    {
        diag_error("write g_stPtrInfo failed.\n");
    }

    DIAG_DebugFileTail(pFile, FilePath);

    (void)mdrv_file_close(pFile);

    return ;
}
EXPORT_SYMBOL(DIAG_DebugPTR);


u32 DIAG_DebugFileHeader(void *pFile)
{
    u32 ret;
    u32 ulValue;

    ret = (u32)mdrv_file_seek(pFile, 0, DRV_SEEK_SET);
    if(BSP_OK != ret)
    {
        diag_error("file_seek failed .\n");
        return (u32)BSP_ERROR;
    }

    ulValue = DIAG_DEBUG_START;

    /* file start flag */
    ret = (u32)mdrv_file_write(&ulValue, 1, sizeof(ulValue), pFile);
    if(ret != sizeof(ulValue))
    {
        diag_error("write start flag failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = DIAG_DEBUG_VERSION;

    /* debug version */
    ret = (u32)mdrv_file_write(&ulValue, 1, sizeof(ulValue), pFile);
    if(ret != sizeof(ulValue))
    {
        diag_error("write debug version failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = 0;

    /* file size */
    ret = (u32)mdrv_file_write(&ulValue, 1, sizeof(ulValue), pFile);
    if(ret != sizeof(ulValue))
    {
        diag_error("write file size failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = bsp_get_slice_value();

    /* 当前的slice */
    ret = (u32)mdrv_file_write(&ulValue, 1, sizeof(ulValue), pFile);
    if(ret != sizeof(ulValue))
    {
        diag_error("write ulTime failed.\n");
        return (u32)BSP_ERROR;
    }

    return ERR_MSP_SUCCESS;
}




void DIAG_DebugFileTail(void *pFile, char *FilePath)
{
    u32 ret;
    u32 ulValue;

    /* file end flag */
    ulValue = DIAG_DEBUG_END;
    ret = (u32)mdrv_file_write(&ulValue, 1, sizeof(ulValue), pFile);
    if(ret != sizeof(ulValue))
    {
        diag_error("write start flag failed.\n");
    }
}

u32 g_astThroughput[EN_DIAG_THRPUT_MAX] = {};

/*****************************************************************************
 Function Name   : diag_ThroughputIn
 Description     : 吞吐率记录
*****************************************************************************/
u32 DIAG_GetThrputInfo(DIAG_THRPUT_ID_ENUM type)
{
    return g_astThroughput[type];
}

/*目的端丢包定时上报*************************************************************************/
DIAG_MNTN_DST_INFO_STRU g_ind_dst_mntn_info = {};
u32              g_ulSendUSBStartSlice = 0;
u32              g_ulSendPcdevStartSlice = 0;
/*复位维测信息记录*/
void diag_reset_dst_mntn_info(void)
{
    memset_s(&g_ind_dst_mntn_info, sizeof(g_ind_dst_mntn_info), 0, sizeof(g_ind_dst_mntn_info));
    memset_s(&g_astThroughput, sizeof(g_astThroughput), 0, sizeof(g_astThroughput));
}

s32 diag_system_debug_event_cb(unsigned int u32ChanID, SOCP_EVENT_ENUM_UIN32 u32Event, unsigned int u32Param)
{
    u32                     ret = 0;
    u32                     busy_size;
    u32                     dst_size;
    SOCP_BUFFER_RW_STRU     stSocpBuff;

    if((SOCP_CODER_DST_OM_IND == u32ChanID)
    &&((SOCP_EVENT_OUTBUFFER_OVERFLOW == u32Event)
        ||(SOCP_EVENT_OUTBUFFER_THRESHOLD_OVERFLOW == u32Event)))
    {
        g_ind_dst_mntn_info.ulDeltaOverFlowCnt++;

        dst_size = scm_ind_get_dst_buff_size();
        ret = bsp_socp_get_read_buff(SOCP_CODER_DST_OM_IND, &stSocpBuff);
        if(ret)
        {
            return ret;
        }

        busy_size = stSocpBuff.u32RbSize + stSocpBuff.u32Size;
        /* 目的通道buff 80%上溢次数统计 */
        if(busy_size*100 >= dst_size*80)
        {
            g_ind_dst_mntn_info.ulDeltaPartOverFlowCnt++;
        }
    }
    return BSP_OK;
}

DIAG_MNTN_DST_INFO_STRU * diag_DebugGetDstMntnInfo(void)
{
    return &g_ind_dst_mntn_info;
}


u32 diag_CreateDFR(char *name, u32 ulLen, DIAG_DFR_INFO_STRU *pDfr)
{
    if((NULL == name)
        || (NULL == pDfr)
        || (0 == ulLen)
        || (ulLen%4)
        || (ulLen > DIAG_DFR_BUFFER_MAX))
    {
        diag_error("parm error\n");
        return ERR_MSP_FAILURE;
    }

    osl_sem_init(1, &pDfr->semid);

    pDfr->pData = osl_malloc(ulLen);
    if(NULL == pDfr->pData)
    {
        diag_error("malloc fail\n");
        return ERR_MSP_FAILURE;
    }

    if(memset_s(pDfr->pData, pDfr->ulLen, 0, ulLen))
    {
        diag_error("memset fail\n");
    }

    if(memcpy_s(pDfr->name, sizeof(pDfr->name), name, strnlen(name, DIAG_DFR_NAME_MAX-1)))
    {
        diag_error("memcpy fail\n");
    }

    pDfr->name[DIAG_DFR_NAME_MAX-1] = 0;

    pDfr->ulCur = 0;
    pDfr->ulLen = ulLen;
    pDfr->ulMagicNum = DIAG_DFR_MAGIC_NUM;

    return ERR_MSP_SUCCESS;
}

void diag_SaveDFR(DIAG_DFR_INFO_STRU *pDfr, u8 *pData, u32 ulLen)
{
    u32 ulSize;
    DIAG_DFR_HEADER_STRU stDfrHeader;

    if((NULL== pDfr)
        || (NULL == pData)
        || (DIAG_DFR_MAGIC_NUM != pDfr->ulMagicNum)
        || (ulLen > DIAG_DFR_BUFFER_MAX)
        || (pDfr->ulLen > DIAG_DFR_BUFFER_MAX))
    {
        return ;
    }

    /* 如果有任务正在进行中，需要等待其完成 */
    osl_sem_down(&pDfr->semid);

    stDfrHeader.ulStart = DIAG_DFR_START_NUM;
    stDfrHeader.ulTime  = bsp_get_slice_value();

    /* 拷贝开始标记和时间戳 */
    if((pDfr->ulCur + sizeof(DIAG_DFR_HEADER_STRU)) <= pDfr->ulLen)
    {
        if(memcpy_s(&(pDfr->pData[pDfr->ulCur]), pDfr->ulLen-pDfr->ulCur, &stDfrHeader, sizeof(stDfrHeader)))
       {
            diag_error("memcpy fail\n");
        }
    }
    else
    {
        ulSize = (pDfr->ulLen - pDfr->ulCur);
        if(memcpy_s(&(pDfr->pData[pDfr->ulCur]), ulSize, &stDfrHeader, ulSize))
        {
            diag_error("memcpy fail\n");
        }
        if(memcpy_s(&(pDfr->pData[0]), pDfr->ulCur,
            (((u8*)&stDfrHeader)+ulSize), (sizeof(stDfrHeader) - ulSize)))
       {
            diag_error("memcpy fail\n");
        }
    }
    pDfr->ulCur = (DFR_ALIGN_WITH_4BYTE(pDfr->ulCur + sizeof(DIAG_DFR_HEADER_STRU))) % pDfr->ulLen;

    /* 拷贝码流 */
    if((pDfr->ulCur + ulLen) <= pDfr->ulLen)
    {
        if(memcpy_s(&(pDfr->pData[pDfr->ulCur]), pDfr->ulLen-pDfr->ulCur, pData, ulLen))
       {
            diag_error("memcpy fail\n");
        }
    }
    else
    {
        ulSize = (pDfr->ulLen - pDfr->ulCur);
        if(memcpy_s(&(pDfr->pData[pDfr->ulCur]), ulSize, pData, ulSize))
       {
            diag_error("memcpy fail\n");
        }
        if(memcpy_s(&(pDfr->pData[0]), pDfr->ulCur, (pData + ulSize), (ulLen - ulSize)))
       {
            diag_error("memcpy fail\n");
        }
    }
    pDfr->ulCur = (DFR_ALIGN_WITH_4BYTE(pDfr->ulCur + ulLen)) % pDfr->ulLen;

    osl_sem_up(&pDfr->semid);

    return ;
}



void diag_GetDFR(DIAG_DFR_INFO_STRU *pDfr)
{
    void *pFile;
    u32 ret, len;
    char FilePath[64] = {0};
    char *DirPath = DIAG_LOG_PATH;
    char aucInfo[DIAG_DEBUG_INFO_LEN];

    if((NULL == pDfr) || (DIAG_DFR_MAGIC_NUM != pDfr->ulMagicNum))
    {
        diag_error("parm error\n");
        return ;
    }

    /* 如果DIAG目录不存在则先创建目录 */
    if (BSP_OK != mdrv_file_access(DirPath, 0))
    {
        if (BSP_OK != mdrv_file_mkdir(DirPath))
        {
            diag_error("mkdir %s failed.\n", DirPath);
            return ;
        }
    }

    len = strnlen(DIAG_LOG_PATH, DIRPAH_LEN);

    (void)memcpy_s(FilePath, sizeof(FilePath), DIAG_LOG_PATH, len);

    if(memcpy_s((FilePath + len), sizeof(FilePath)-len, pDfr->name, strnlen(pDfr->name,DIAG_DFR_NAME_MAX)))
    {
        diag_error("memcpy fail\n");
    }

    pFile = mdrv_file_open(FilePath, "wb+");
    if(pFile == 0)
    {
        diag_error(" mdrv_file_open failed.\n");
        return;
    }

    ret = DIAG_DebugFileHeader(pFile);
    if(BSP_OK != ret)
    {
        diag_error(" DIAG_DebugFileHeader failed .\n");
        (void)mdrv_file_close(pFile);
        return ;
    }

    (void)memset_s(aucInfo, sizeof(aucInfo), 0, sizeof(aucInfo));
    (void)memcpy_s(aucInfo, sizeof(aucInfo), "DIAG DFR info", strnlen(("DIAG DFR info"),(DIAG_DEBUG_INFO_LEN-1)));

    /* 通用信息 */
    ret = (u32)mdrv_file_write(aucInfo, 1, DIAG_DEBUG_INFO_LEN, pFile);
    if(ret != DIAG_DEBUG_INFO_LEN)
    {
        diag_error(" mdrv_file_write DIAG number info failed.\n");
    }

    /* 当前指针 */
    ret = (u32)mdrv_file_write(&pDfr->ulCur, 1, sizeof(pDfr->ulCur), pFile);
    if(ret != sizeof(pDfr->ulCur))
    {
        diag_error(" mdrv_file_write pData failed.\n");
    }

    /* 缓冲区长度 */
    ret = (u32)mdrv_file_write(&pDfr->ulLen, 1, sizeof(pDfr->ulCur), pFile);
    if(ret != sizeof(pDfr->ulCur))
    {
        diag_error(" mdrv_file_write pData failed.\n");
    }

    ret = (u32)mdrv_file_write(pDfr->pData, 1, pDfr->ulLen, pFile);
    if(ret != pDfr->ulLen)
    {
        diag_error(" mdrv_file_write pDfr->pData failed.\n");
    }

    DIAG_DebugFileTail(pFile, FilePath);

    (void)mdrv_file_close(pFile);

    return ;
}

void diag_DebugOverFlowRecord(u32 ulPackageLen)
{
    if(ulPackageLen < DIAG_SRC_50_OVERFLOW_THR)
    {
        g_stDiagDrvDebugInfo.ulOverFlow50Num++;
    }
    if(ulPackageLen < DIAG_SRC_80_OVERFLOW_THR)
    {
        g_stDiagDrvDebugInfo.ulOverFlow80Num++;
    }
}

u32 diag_debug_log_level(u32 level)
{
    g_DiagLogLevel = level;/*diag log level: 0=error, !0=debug*/
    return g_DiagLogLevel;
}

void diag_debug_ind_src_lost(u32 len)
{
    g_stDiagDrvDebugInfo.ulAbandonNum++;
    g_stDiagDrvDebugInfo.ulAbandonLen += len;
}

void diag_DebugPackageRecord(u32 ulPackageLen)
{
    g_stDiagDrvDebugInfo.ulPackageLen += ulPackageLen;
    g_stDiagDrvDebugInfo.ulPackageNum++;
}

void diag_DebugResetSrcMntnInfo(void)
{
    memset_s(&g_stDiagDrvDebugInfo, sizeof(g_stDiagDrvDebugInfo), 0, sizeof(g_stDiagDrvDebugInfo));
}
DIAG_DRV_DEBUG_INFO_STRU * diag_DebugGetSrcMntnInfo(void)
{
    return &g_stDiagDrvDebugInfo;
}

/*****************************************************************************
 Function Name   : LNR : Last N Ring buffer store (最后N条信息循环存储功能)
 Description     : 保存最近的N条信息
*****************************************************************************/

DRV_DIAG_LNR_INFO_TBL_STRU g_astDrvLNRInfoTbl[EN_DIAG_DRV_LNR_INFO_MAX] = {{0}};

/*****************************************************************************
 Function Name   : diag_LNR
 Description     : 最后NV条信息的记录接口
*****************************************************************************/
void diag_DrvLNR(DIAG_DRV_LNR_ID_ENUM ulType, u32 ulRserved1, u32 ulRserved2)
{
    g_astDrvLNRInfoTbl[ulType].ulRserved1[g_astDrvLNRInfoTbl[ulType].ulCur] = ulRserved1;
    g_astDrvLNRInfoTbl[ulType].ulRserved2[g_astDrvLNRInfoTbl[ulType].ulCur] = ulRserved2;
    g_astDrvLNRInfoTbl[ulType].ulCur = (g_astDrvLNRInfoTbl[ulType].ulCur+1)%DIAG_DRV_LNR_NUMBER;
}

void DIAG_DrvShowLNR(DIAG_DRV_LNR_ID_ENUM ulType, u32 n)
{
    u32 i;
    u32 cur;

    cur = (g_astDrvLNRInfoTbl[ulType].ulCur + DIAG_DRV_LNR_NUMBER - n)%DIAG_DRV_LNR_NUMBER;

    for(i = 0; i <n; i++)
    {
        diag_crit("0x%x, 0x%x.\n", g_astDrvLNRInfoTbl[ulType].ulRserved1[cur], g_astDrvLNRInfoTbl[ulType].ulRserved2[cur]);
        cur = (cur + 1)%DIAG_DRV_LNR_NUMBER;
    }
}



