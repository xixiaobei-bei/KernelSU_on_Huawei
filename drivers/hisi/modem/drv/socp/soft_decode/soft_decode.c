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
  1 头文件包含
**************************************************************************** */
#include <linux/module.h>
#include <mdrv.h>
#include <eagleye.h>
#include <osl_sem.h>
#include <osl_thread.h>
#include <osl_malloc.h>
#include <securec.h>
#include <bsp_rfile.h>
#include <bsp_dump.h>
#include <bsp_slice.h>
#include "hdlc.h"
#include "soft_cfg.h"
#include "soft_decode.h"


#define  THIS_MODU mod_soft_dec
/* ****************************************************************************
  2 全局变量定义
**************************************************************************** */
/* 自旋锁，用来作OM数据接收的临界资源保护 */
spinlock_t             g_stScmSoftDecodeDataRcvSpinLock;

/* HDLC控制结构 */
OM_HDLC_STRU             g_stScmHdlcSoftDecodeEntity;

/* SCM数据接收数据缓冲区 */
s8                 g_aucSCMDataRcvBuffer[SCM_DATA_RCV_PKT_SIZE];

/* SCM数据接收任务控制结构 */
SCM_DATA_RCV_CTRL_STRU   g_stSCMDataRcvTaskCtrlInfo;

SCM_SOFTDECODE_INFO_STRU   g_stScmSoftDecodeInfo;

/*****************************************************************************
  3 外部引用声明
*****************************************************************************/
extern void diag_save_soft_decode_info(void);
extern void CPM_LogicRcvReg(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, CPM_RCV_FUNC pRcvFunc);
/*****************************************************************************
  4 函数实现
*****************************************************************************/


u32 SCM_SoftDecodeCfgDataRcv(u8 *pucBuffer, u32 ulLen)
{
    u32                          ulRstl;
    unsigned long                           ulLockLevel;

    spin_lock_irqsave(&g_stScmSoftDecodeDataRcvSpinLock, ulLockLevel);

    ulRstl = SCM_SoftDecodeDataRcv(pucBuffer, ulLen);

    spin_unlock_irqrestore(&g_stScmSoftDecodeDataRcvSpinLock, ulLockLevel);

    return ulRstl;
}


u32 SCM_SoftDecodeDataRcv(u8 *pucBuffer, u32 ulLen)
{
    s32                           sRet;

    diag_PTR(EN_DIAG_PTR_SCM_SOFTDECODE, 0, 0, 0);

    if (ulLen > (u32)diag_RingBufferFreeBytes(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId))
    {
        g_stScmSoftDecodeInfo.stRbInfo.ulBufferNotEnough++;
        diag_PTR(EN_DIAG_PTR_SCM_ERR1, 0, 0, 0);

        return ERR_MSP_FAILURE;
    }

    sRet = diag_RingBufferPut(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId,
                            (s8 *)pucBuffer,
                            (s32)ulLen);
    COVERITY_TAINTED_SET((VOID *)&(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId.buf));

    if (ulLen == (u32)sRet)
    {
        osl_sem_up(&(g_stSCMDataRcvTaskCtrlInfo.SmID));
        return BSP_OK;
    }

    g_stScmSoftDecodeInfo.stRbInfo.ulRingBufferPutErr++;
    diag_PTR(EN_DIAG_PTR_SCM_ERR2, 0, 0, 0);

    return ERR_MSP_FAILURE;
}


u32 SCM_SoftDecodeAcpuRcvData(
    OM_HDLC_STRU                       *pstHdlcCtrl,
    u8                          *pucData,
    u32                          ulLen)
{
    u32                          i;
    u32                          ulResult;
    u8                           ucGutlType;
    u8                           ucChar;

    ulResult = ERR_MSP_FAILURE;

    for( i = 0; i < ulLen; i++ )
    {
        ucChar = (u8)pucData[i];

        ulResult = diag_HdlcDecap(pstHdlcCtrl, ucChar);

        if ( HDLC_SUCC == ulResult )
        {
            g_stScmSoftDecodeInfo.stHdlcDecapData.ulDataLen += pstHdlcCtrl->ulInfoLen;
            g_stScmSoftDecodeInfo.stHdlcDecapData.ulNum++;

            ucGutlType = pstHdlcCtrl->pucDecapBuff[0];

            diag_PTR(EN_DIAG_PTR_SCM_RCVDATA_SUCCESS, 0, 0, 0);

            SCM_RcvDataDispatch(pstHdlcCtrl, ucGutlType);
        }
        else if (HDLC_NOT_HDLC_FRAME == ulResult)
        {
            /*不是完整分帧,继续HDLC解封装*/
        }
        else
        {
            g_stScmSoftDecodeInfo.ulFrameDecapErr++;
        }
    }

    return BSP_OK;
}


u32 SCM_SoftDecodeCfgHdlcInit(OM_HDLC_STRU *pstHdlc)
{
    /* 申请用于HDLC解封装的缓存 */
    pstHdlc->pucDecapBuff    = (u8 *)osl_malloc(SCM_DATA_RCV_PKT_SIZE);

    if (NULL == pstHdlc->pucDecapBuff)
    {
        soft_decode_error("Alloc Decapsulate buffer fail!\n");

        return ERR_MSP_FAILURE;
    }

    /* HDLC解封装缓存长度赋值 */
    pstHdlc->ulDecapBuffSize = SCM_DATA_RCV_PKT_SIZE;

    /* 初始化HDLC解封装控制上下文 */
    diag_HdlcInit(pstHdlc);

    return BSP_OK;
}


void SCM_SoftDecodeCfgRcvSelfTask(void)
{
    s32                           sRet;
    s32                           lLen;
    s32                           lRemainlen;
    s32                           lReadLen;
    u32                          ulPktNum;
    u32                          i;
    unsigned long                           ulLockLevel;

    for (;;)
    {
        osl_sem_down(&(g_stSCMDataRcvTaskCtrlInfo.SmID));

        diag_PTR(EN_DIAG_PTR_SCM_SELFTASK, 0, 0, 0);

        COVERITY_TAINTED_SET((VOID *)&(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId.buf));

        lLen = diag_RingBufferNBytes(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId);

        if (lLen <= 0)
        {
            continue;
        }

        ulPktNum = (u32)((lLen + SCM_DATA_RCV_PKT_SIZE - 1) / SCM_DATA_RCV_PKT_SIZE);

        lRemainlen = lLen;

        for (i = 0; i < ulPktNum; i++)
        {
            if (SCM_DATA_RCV_PKT_SIZE < lRemainlen)
            {
                lReadLen = SCM_DATA_RCV_PKT_SIZE;

                sRet = diag_RingBufferGet(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId,
                                        g_stSCMDataRcvTaskCtrlInfo.pucBuffer,
                                        SCM_DATA_RCV_PKT_SIZE);
            }
            else
            {
                lReadLen = lRemainlen;

                sRet = diag_RingBufferGet(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId,
                                        g_stSCMDataRcvTaskCtrlInfo.pucBuffer,
                                        lRemainlen);
            }

            if (sRet != lReadLen)
            {
                spin_lock_irqsave(&g_stScmSoftDecodeDataRcvSpinLock, ulLockLevel);

                diag_RingBufferFlush(g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId);

                spin_unlock_irqrestore(&g_stScmSoftDecodeDataRcvSpinLock, ulLockLevel);

                g_stScmSoftDecodeInfo.stRbInfo.ulRingBufferFlush++;
                diag_PTR(EN_DIAG_PTR_SCM_ERR3, 0, 0, 0);

                continue;
            }

            lRemainlen -= lReadLen;

            g_stScmSoftDecodeInfo.stGetInfo.ulDataLen += lReadLen;

            diag_PTR(EN_DIAG_PTR_SCM_RCVDATA, 0, 0, 0);

            /* 调用HDLC解封装函数 */
            if (BSP_OK != SCM_SoftDecodeAcpuRcvData(&g_stScmHdlcSoftDecodeEntity,
                                                    (u8 *)g_stSCMDataRcvTaskCtrlInfo.pucBuffer,
                                                    (u32)lReadLen))
            {
                soft_decode_error("SCM_SoftDecodeAcpuRcvData Fail\n");
            }
        }


    /* coverity[loop_bottom] */
    }
}



int SCM_SoftDecodeCfgRcvTaskInit(void)
{
    u32                              ulRslt;
    OSL_TASK_ID                      task_id;


    g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId = diag_RingBufferCreate(SCM_DATA_RCV_BUFFER_SIZE);

    if (NULL == g_stSCMDataRcvTaskCtrlInfo.rngOmRbufId)
    {
        soft_decode_error("Creat OMCFG ringBuffer Fail.\n");

        g_stScmSoftDecodeInfo.stRbInfo.ulRingBufferCreatErr++;

        (void)diag_save_soft_decode_info();

        return BSP_ERROR;
    }

    osl_sem_init(0, &(g_stSCMDataRcvTaskCtrlInfo.SmID));

    /* 注册OM配置数据接收自处理任务 */
    ulRslt = (u32)osl_task_init("soft_dec", 76, 8096,(OSL_TASK_FUNC)SCM_SoftDecodeCfgRcvSelfTask, NULL, &task_id);
    if (ulRslt )
    {
        soft_decode_error("RegisterSelfTaskPrio Fail.\n");
        return BSP_ERROR;
    }

    (void)memset_s(&g_stScmSoftDecodeInfo, sizeof(g_stScmSoftDecodeInfo), 0, sizeof(g_stScmSoftDecodeInfo));

    if (BSP_OK != SCM_SoftDecodeCfgHdlcInit(&g_stScmHdlcSoftDecodeEntity))
    {
        soft_decode_error("HDLC Init Fail.\n");

        g_stScmSoftDecodeInfo.ulHdlcInitErr++;

        return BSP_ERROR;
    }

    g_stSCMDataRcvTaskCtrlInfo.pucBuffer = &g_aucSCMDataRcvBuffer[0];

    spin_lock_init(&g_stScmSoftDecodeDataRcvSpinLock);

    scm_soft_decode_init();

    soft_decode_crit("[init]soft decode init ok\n");

    return BSP_OK;
}
module_init(SCM_SoftDecodeCfgRcvTaskInit);
void  scm_soft_decode_init(void)
{
    CPM_LogicRcvReg(CPM_OM_CFG_COMM, SCM_SoftDecodeCfgDataRcv);

    g_stScmSoftDecodeInfo.ulCpmRegLogicRcvSuc++;
}

void SCM_SoftDecodeInfoShow(void)
{
    soft_decode_crit("\r\nSCM_SoftDecodeInfoShow:\r\n");

    soft_decode_crit("\r\nSem Creat Error %d:\r\n",                   g_stScmSoftDecodeInfo.stRbInfo.ulSemCreatErr);
    soft_decode_crit("\r\nSem Give Error %d:\r\n",                    g_stScmSoftDecodeInfo.stRbInfo.ulSemGiveErr);
    soft_decode_crit("\r\nRing Buffer Creat Error %d:\r\n",           g_stScmSoftDecodeInfo.stRbInfo.ulRingBufferCreatErr);
    soft_decode_crit("\r\nTask Id Error %d:\r\n",                     g_stScmSoftDecodeInfo.stRbInfo.ulTaskIdErr);
    soft_decode_crit("\r\nRing Buffer not Enough %d:\r\n",            g_stScmSoftDecodeInfo.stRbInfo.ulBufferNotEnough);
    soft_decode_crit("\r\nRing Buffer Flush %d:\r\n",                 g_stScmSoftDecodeInfo.stRbInfo.ulRingBufferFlush);
    soft_decode_crit("\r\nRing Buffer Put Error %d:\r\n",             g_stScmSoftDecodeInfo.stRbInfo.ulRingBufferPutErr);

    soft_decode_crit("\r\nRing Buffer Put Success Times %d:\r\n",     g_stScmSoftDecodeInfo.stPutInfo.ulNum);
    soft_decode_crit("\r\nRing Buffer Put Success Bytes %d:\r\n",     g_stScmSoftDecodeInfo.stPutInfo.ulDataLen);

    soft_decode_crit("\r\nRing Buffer Get Success Times %d:\r\n",     g_stScmSoftDecodeInfo.stGetInfo.ulNum);
    soft_decode_crit("\r\nRing Buffer Get Success Bytes %d:\r\n",     g_stScmSoftDecodeInfo.stGetInfo.ulDataLen);

    soft_decode_crit("\r\nHDLC Decode Success Times %d:\r\n",         g_stScmSoftDecodeInfo.stHdlcDecapData.ulNum);
    soft_decode_crit("\r\nHDLC Decode Success Bytes %d:\r\n",         g_stScmSoftDecodeInfo.stHdlcDecapData.ulDataLen);

    soft_decode_crit("\r\nHDLC Decode Error Times %d:\r\n",           g_stScmSoftDecodeInfo.ulFrameDecapErr);

    soft_decode_crit("\r\nHDLC Init Error Times %d:\r\n",             g_stScmSoftDecodeInfo.ulHdlcInitErr);

    soft_decode_crit("\r\nHDLC Decode Data Type Error Times %d:\r\n", g_stScmSoftDecodeInfo.ulDataTypeErr);

    soft_decode_crit("\r\nCPM Reg Logic Rcv Func Success Times %d:\r\n", g_stScmSoftDecodeInfo.ulCpmRegLogicRcvSuc);
}
EXPORT_SYMBOL(SCM_SoftDecodeInfoShow);

#define    DIAG_LOG_PATH       MODEM_LOG_ROOT"/drv/DIAG/"

/*软解码*************************************************************************/
void diag_save_soft_decode_info(void)
{
    u32  pFile;
    u32  ret;
    char *DirPath = DIAG_LOG_PATH;
    char *FilePath = DIAG_LOG_PATH"DIAG_SoftDecode.bin";
    char aucInfo[32];

    /* 如果DIAG目录不存在则先创建目录 */
    if (BSP_OK != bsp_access(DirPath, 0))
    {
        if (BSP_OK != bsp_mkdir(DirPath, 0755))
        {
            soft_decode_error("mkdir %s failed.\n", DIAG_LOG_PATH);
            return ;
        }
    }

    pFile = bsp_open(FilePath, RFILE_RDWR|RFILE_CREAT, 0664);
    if(pFile == 0)
    {
        soft_decode_error(" file open failed.\n");
        return;
    }

    ret = diag_fill_soft_header(pFile);
    if(BSP_OK != ret)
    {
        soft_decode_error(" DIAG_DebugFileHeader failed\n");
        (void)bsp_close(pFile);
        return ;
    }

    (void)memset_s(aucInfo, sizeof(aucInfo), 0, sizeof(aucInfo));
    (void)memcpy_s(aucInfo, sizeof(aucInfo), "DIAG SoftDecode info", strnlen("DIAG SoftDecode info", sizeof(aucInfo)-1));

    ret = bsp_write(pFile, (s8 *)aucInfo, sizeof(aucInfo));
    if(ret != sizeof(aucInfo))
    {
        soft_decode_error(" write DIAG number info failed.\n");
    }

    ret = bsp_write(pFile, (s8 *)&g_stScmSoftDecodeInfo,
        sizeof(g_stScmSoftDecodeInfo));
    if(ret != sizeof(g_stScmSoftDecodeInfo))
    {
        soft_decode_error("write pData failed.\n");
    }

    diag_fill_soft_tail(pFile, FilePath);

    (void)bsp_close(pFile);

    return ;
}


u32 diag_fill_soft_header(u32 pFile)
{
    u32 ret;
    u32 ulValue;

    ret = (u32)bsp_lseek(pFile, 0, DRV_SEEK_SET);
    if(BSP_OK != ret)
    {
        soft_decode_error("file seek failed .\n");
        return (u32)BSP_ERROR;
    }

    ulValue = 0xaaaa5555;

    /* file start flag */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        soft_decode_error("write start flag failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = 0x00010000;

    /* debug version */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        soft_decode_error("write debug version failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = 0;

    /* file size */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        soft_decode_error("write file size failed.\n");
        return (u32)BSP_ERROR;
    }

    ulValue = bsp_get_slice_value();

    /* 当前的slice */
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        soft_decode_error("write ulTime failed.\n");
        return (u32)BSP_ERROR;
    }

    return BSP_OK;
}




void diag_fill_soft_tail(u32 pFile, s8 *FilePath)
{
    u32 ret;
    u32 ulValue;

    /* file end flag */
    ulValue = 0x5555aaaa;
    ret = (u32)bsp_write(pFile, (s8 *)&ulValue, sizeof(ulValue));
    if(ret != sizeof(ulValue))
    {
        soft_decode_error("write start flag failed.\n");
    }
}





