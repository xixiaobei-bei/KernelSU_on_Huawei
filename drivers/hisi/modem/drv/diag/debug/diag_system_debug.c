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
#include  <mdrv.h>
#include  <mdrv_diag_system.h>
#include  <osl_thread.h>
#include  <osl_malloc.h>
#include  <bsp_slice.h>
#include  <bsp_rfile.h>
#include  <securec.h>
#include  "OmCommonPpm.h"
#include  "diag_system_debug.h"
#include  "scm_ind_dst.h"
#include  "scm_common.h"


#define    DIAG_LOG_PATH       MODEM_LOG_ROOT"/drv/DIAG/"

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

spinlock_t  g_stDiagThroughputSpinLock;

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



void DIAG_DebugPTR(void)
{
    s32 pFile;
    u32 ret;
    u32 ulValue;
    s8 *DirPath = DIAG_LOG_PATH;
    s8 *FilePath = DIAG_LOG_PATH"DIAG_PTR.bin";

    /* 如果DIAG目录不存在则先创建目录 */
    if (BSP_OK != bsp_access(DirPath, RFILE_RDONLY))
    {
        if (BSP_OK != bsp_mkdir(DirPath, 0775))
        {
            diag_error("mkdir %s failed.\n", DIAG_LOG_PATH);
            return ;
        }
    }

    pFile = bsp_open(FilePath, RFILE_RDWR|RFILE_CREAT, 0664);
    if(pFile < 0)
    {
        diag_error(" bsp_open failed.\n");
        return ;
    }

    ret = DIAG_SystemDebugFileHeader(pFile);
    if(BSP_OK != ret)
    {
        diag_error("DIAG_DebugFileHeader failed .\n");
        (void)bsp_close(pFile);
        return ;
    }

    /* 打点信息长度 */
    ulValue = DIAG_DEBUG_SIZE_FLAG | sizeof(g_stPtrInfo);
    ret = bsp_write((u32)pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        diag_error("write sizeof g_stPtrInfo failed.\n");
    }

    /* 再写入打点信息 */
    ret = bsp_write((u32)pFile, (s8 *)&g_stPtrInfo, sizeof(g_stPtrInfo));
    if(ret != sizeof(g_stPtrInfo))
    {
        diag_error("write g_stPtrInfo failed.\n");
    }

    DIAG_SystemDebugFileTail(pFile, FilePath);

    (void)bsp_close((u32)pFile);

    return ;
}
EXPORT_SYMBOL(DIAG_DebugPTR);

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

    (void)memcpy_s(pPtrTmp, sizeof(*pPtrTmp), &g_stPtrInfo, sizeof(g_stPtrInfo));

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


u32 DIAG_SystemDebugFileHeader(u32 pFile)
{
    u32 ret;
    u32 ulValue;

    ret = (u32)bsp_lseek(pFile, 0, DRV_SEEK_SET);
    if(BSP_OK != ret)
    {
        diag_error("file_seek failed .\n");
        return (u32)BSP_ERROR;
    }

    ulValue = DIAG_DEBUG_START;

    /* file start flag */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        diag_error("write start flag failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = DIAG_DEBUG_VERSION;

    /* debug version */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        diag_error("write debug version failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = 0;

    /* file size */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        diag_error("write file size failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = bsp_get_slice_value();

    /* 当前的slice */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        diag_error("write ulTime failed.\n");
        return (u32)BSP_ERROR;
    }

    return BSP_OK;
}




void DIAG_SystemDebugFileTail(u32 pFile, s8 *FilePath)
{
    u32 ret;
    u32 ulValue;

    /* file end flag */
    ulValue = DIAG_DEBUG_END;
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        diag_error("write start flag failed.\n");
    }
}

DIAG_THRPUT_INFO_STRU g_astThroughput[EN_DIAG_THRPUT_MAX] = {{0,0,0,0,{{0,0},}}};


void diag_ThroughputSave(DIAG_THRPUT_ID_ENUM enChn, u32 bytes)
{
    u32 slice = mdrv_timer_get_normal_timestamp();
    u32 ptr;
    unsigned long ulLockLevel;

    g_astThroughput[enChn].ulBytes += bytes;

    scm_SpinLockIntLock(&g_stDiagThroughputSpinLock, ulLockLevel);
    if(slice >= g_astThroughput[enChn].ulSlice)
    {
        /* 每次统计吞吐率最少间隔5秒以上 */
        if((slice - g_astThroughput[enChn].ulSlice) > DIAG_THRPUT_DEBUG_LEN)
        {
            ptr = g_astThroughput[enChn].ulPtr;

            g_astThroughput[enChn].stNode[ptr].ulSlice = slice;
            g_astThroughput[enChn].stNode[ptr].ulThroughput = \
                g_astThroughput[enChn].ulBytes/((slice - g_astThroughput[enChn].ulSlice)/32768);

            g_astThroughput[enChn].ulSlice = slice;

            if(g_astThroughput[enChn].ulMax < g_astThroughput[enChn].stNode[ptr].ulThroughput)
            {
                g_astThroughput[enChn].ulMax = g_astThroughput[enChn].stNode[ptr].ulThroughput;
            }

            if((++g_astThroughput[enChn].ulPtr) >= DIAG_THRPUT_DEBUG_NUM)
            {
                g_astThroughput[enChn].ulPtr = 0;
            }

            g_astThroughput[enChn].ulBytes = 0;
        }
    }
    else    /* 如果反转则清零，重新统计 */
    {
        g_astThroughput[enChn].ulSlice = slice;
        g_astThroughput[enChn].ulBytes = 0;
    }
    scm_SpinUnlockIntUnlock(&g_stDiagThroughputSpinLock, ulLockLevel);
}

void DIAG_ShowThroughput(u32 ulIndex)
{
    u32 i;

    if(ulIndex >= EN_DIAG_THRPUT_MAX)
    {
        diag_error("ulIndex %d.\n", ulIndex);
        return ;
    }

    diag_crit("max throughput 0x%x Bytes/s.\n", g_astThroughput[ulIndex].ulMax);

    for(i = 0; i < DIAG_THRPUT_DEBUG_NUM; i++)
    {
        diag_crit("slice 0x%08x, throughput 0x%08x Bytes/s.\n",
            g_astThroughput[ulIndex].stNode[i].ulSlice,
            g_astThroughput[ulIndex].stNode[i].ulThroughput);
    }
}
u32 DIAG_GetThrputInfo(DIAG_THRPUT_ID_ENUM type)
{
    u32 ulptr = 0;

    ulptr = g_astThroughput[type].ulPtr;
    ulptr = (0 == ulptr) ? (DIAG_THRPUT_DEBUG_NUM-1) : (ulptr - 1);
    return g_astThroughput[type].stNode[ulptr].ulThroughput;
}


void DIAG_Throughput(void)
{
    u32  pFile;
    u32     ret;
    char *DirPath = DIAG_LOG_PATH;
    char *FilePath = DIAG_LOG_PATH"DIAG_Throughput.bin";
    char   aucInfo[DIAG_DEBUG_INFO_LEN];

    /* 如果DIAG目录不存在则先创建目录 */
    if (BSP_OK != bsp_access(DirPath, 0))
    {
        if (BSP_OK != bsp_mkdir(DirPath, 0755))
        {
            diag_error("mkdir %s failed.\n", DIAG_LOG_PATH);
            return ;
        }
    }

    pFile = bsp_open(FilePath, RFILE_RDWR|RFILE_CREAT, 0664);
    if(pFile == 0)
    {
        diag_error(" mdrv_file_open failed.\n");
        return;
    }

    ret = DIAG_SystemDebugFileHeader(pFile);
    if(BSP_OK != ret)
    {
        diag_error(" DIAG_DebugFileHeader failed .\n");
        (void)bsp_close(pFile);
        return ;
    }

    (void)memset_s(aucInfo, sizeof(aucInfo), 0, sizeof(aucInfo));
    (void)memcpy_s(aucInfo, sizeof(aucInfo), "DIAG Throughput info", strnlen("DIAG Throughput info", sizeof(aucInfo)-1));

    ret = bsp_write(pFile, (s8 *)aucInfo, DIAG_DEBUG_INFO_LEN);
    if(ret != DIAG_DEBUG_INFO_LEN)
    {
        diag_error(" mdrv_file_write DIAG number info failed.\n");
    }

    ret = bsp_write(pFile, (s8 *)&g_astThroughput[EN_DIAG_THRPUT_DATA_CHN_PHY],
        sizeof(g_astThroughput[EN_DIAG_THRPUT_DATA_CHN_PHY]));
    if(ret != sizeof(g_astThroughput[EN_DIAG_THRPUT_DATA_CHN_PHY]))
    {
        diag_error(" mdrv_file_write pData failed.\n");
    }

    ret = bsp_write(pFile, (s8 *)&g_astThroughput[EN_DIAG_THRPUT_DATA_CHN_CB],
        sizeof(g_astThroughput[EN_DIAG_THRPUT_DATA_CHN_CB]));
    if(ret != sizeof(g_astThroughput[EN_DIAG_THRPUT_DATA_CHN_CB]))
    {
        diag_error(" mdrv_file_write pData failed.\n");
    }

    DIAG_SystemDebugFileTail(pFile, FilePath);

    (void)bsp_close(pFile);

    return ;
}
EXPORT_SYMBOL(DIAG_Throughput);

/*目的端丢包定时上报*************************************************************************/
DIAG_MNTN_DST_INFO_STRU g_ind_dst_mntn_info = {};
u32              g_ulSendUSBStartSlice = 0;
/*复位维测信息记录*/
void diag_reset_dst_mntn_info(void)
{
    memset_s(&g_ind_dst_mntn_info, sizeof(g_ind_dst_mntn_info), 0, sizeof(g_ind_dst_mntn_info));
}

u32 diag_debug_log_level(u32 level)
{
    g_DiagLogLevel = level;/*diag log level: 0=error, !0=debug*/
    return g_DiagLogLevel;
}

s32 diag_system_debug_event_cb(unsigned int u32ChanID, SOCP_EVENT_ENUM_UIN32 u32Event, unsigned int u32Param)
{
    u32                     ret = 0;
    u32                     data_percentage;
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
        data_percentage = (stSocpBuff.u32RbSize + stSocpBuff.u32Size)*100/dst_size;
        if(data_percentage >= 80)
        {
            g_ind_dst_mntn_info.ulDeltaPartOverFlowCnt++;
        }
    }
    return BSP_OK;
}

void diag_debug_get_dst_mntn_info(DIAG_MNTN_DST_INFO_STRU * dst_mntn)
{
    memcpy_s(dst_mntn, sizeof(*dst_mntn), (void*)&g_ind_dst_mntn_info, sizeof(g_ind_dst_mntn_info));
    return;
}

EXPORT_SYMBOL(diag_save_soft_decode_info);


