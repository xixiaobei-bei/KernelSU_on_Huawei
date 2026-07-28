
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
#include <vos.h>
#include <mdrv.h>
#include <msp.h>
#include <nv_stru_lps.h>
#include <nv_id_tlas.h>
#include <soc_socp_adapter.h>
#include "diag_common.h"
#include "diag_api.h"
#include "diag_cfg.h"
#include "diag_debug.h"
#include "diag_message.h"
#include "diag_service.h"


#define    THIS_FILE_ID        MSP_FILE_ID_DIAG_API_C
/*****************************************************************************
  2 Declare the Global Variable
*****************************************************************************/

extern VOS_UINT8 g_EventModuleCfg[DIAG_CFG_PID_NUM];

VOS_UINT32 diag_GetLayerMsgCfg(VOS_UINT32 ulCatId, VOS_UINT32 ulMsgId);
VOS_UINT32 diag_GetLayerSrcCfg(VOS_UINT32 ulModuleId);
VOS_UINT32 diag_GetLayerDstCfg(VOS_UINT32 ulModuleId);
VOS_UINT32 diag_GetLayerCfg(VOS_UINT32 ulSrcModuleId, VOS_UINT32 ulDstModuleId, VOS_UINT32 ulMsgId);
VOS_UINT32 diag_GetPrintCfg(VOS_UINT32 ulModuleId, VOS_UINT32 ulLevel);

VOS_TRANSID_LEN g_ulTransId = 0;

DIAG_LOG_PKT_NUM_ACC_STRU g_DiagLogPktNum ={0};
DIAG_LayerMsgMatchFunc  g_pLayerMatchFunc  = VOS_NULL;
DIAG_LayerMsgMatchNotifyFunc  g_pLayerMatchNotifyFunc  = VOS_NULL;

/*****************************************************************************
  3 Function
*****************************************************************************/


VOS_UINT32 DIAG_GetConnState(VOS_VOID)
{
    return (VOS_UINT32)((DIAG_IS_CONN_ON)?1:0);
}


VOS_UINT32 DIAG_LogPortSwich(VOS_UINT32 ulPhyPort, VOS_BOOL ulEffect)
{
    VOS_UINT32 enLogPort = 0;  // 切换之前的logport
    VOS_UINT32 DiagStatus;     // 工具连接状态
    VOS_UINT32 ulRet;

    (VOS_VOID)mdrv_PPM_QueryLogPort(&enLogPort); /* 获取切换之前的端口类型*/

    DiagStatus = DIAG_GetConnState();   /* 获取diag的连接状态 */

    /* diag连接状态下，不允许从USB切换到VCOM */
    if((1 == DiagStatus) && (DIAG_CPM_OM_PORT_TYPE_USB == enLogPort) && (DIAG_CPM_OM_PORT_TYPE_VCOM == ulPhyPort))
    {
        diag_crit("diag connected, USB does not allowed to change to vcom\n");
        return ERR_MSP_AT_CHANNEL_BUSY;
    }
    else
    {
        ulRet = mdrv_PPM_LogPortSwitch(ulPhyPort, ulEffect);
    }
    return ulRet;
}


VOS_UINT32 diag_GetLayerMsgCfg(VOS_UINT32 ulCatId, VOS_UINT32 ulMsgId)
{
    DIAG_CFG_LOG_CAT_MSG_CFG_STRU *pstItemCfg =NULL;
    VOS_UINT32 i;

    for(i = 0 ; i < g_stMsgCfg.ulCfgCnt; i++)
    {
       pstItemCfg = (g_stMsgCfg.astMsgCfgList + i);

       if(ulMsgId == pstItemCfg->ulId )
       {
           if(DIAG_CFG_SWT_CLOSE == pstItemCfg->ulSwt)
           {
              return ERR_MSP_CFG_LOG_NOT_ALLOW;
           }
           else if(DIAG_CFG_SWT_OPEN == pstItemCfg->ulSwt)
           {
              return ERR_MSP_SUCCESS;
           }
           else
           {
              return ERR_MSP_DIAG_MSG_CFG_NOT_SET;
           }
       }
    }

    return ERR_MSP_DIAG_MSG_CFG_NOT_SET;
}


VOS_UINT32 diag_GetLayerSrcCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 ulOffset = 0;

    if(DOPRA_PID_TIMER == ulModuleId)
    {
        return ERR_MSP_SUCCESS;
    }

    if(DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId))
    {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);

        if(DIAG_CFG_SWT_OPEN == g_ALayerSrcModuleCfg[ulOffset])
        {
            return ERR_MSP_SUCCESS;
        }
    }
    else if(DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId))
    {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);

        if(DIAG_CFG_SWT_OPEN == g_CLayerSrcModuleCfg[ulOffset])
        {
            return ERR_MSP_SUCCESS;
        }
    }
    else if(DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId))
    {
        ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId);

        if(DIAG_CFG_SWT_OPEN == g_NrmLayerSrcModuleCfg[ulOffset])
        {
            return ERR_MSP_SUCCESS;
        }
    }

    return ERR_MSP_CFG_LOG_NOT_ALLOW;
}

VOS_UINT32 diag_GetLayerDstCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 ulOffset = 0;

    if(DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId))
    {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);

        if(DIAG_CFG_SWT_OPEN == g_ALayerDstModuleCfg[ulOffset])
        {
            return  ERR_MSP_SUCCESS;
        }
    }
    else if(DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId))
    {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);

        if(DIAG_CFG_SWT_OPEN == g_CLayerDstModuleCfg[ulOffset])
        {
            return  ERR_MSP_SUCCESS;
        }
    }
    else if(DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId))
    {
        ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId);

        if(DIAG_CFG_SWT_OPEN == g_NrmLayerDstModuleCfg[ulOffset])
        {
            return  ERR_MSP_SUCCESS;
        }
    }

    return ERR_MSP_CFG_LOG_NOT_ALLOW;
}


VOS_UINT32 diag_GetLayerCfg(VOS_UINT32 ulSrcModuleId, VOS_UINT32 ulDstModuleId, VOS_UINT32 ulMsgId)
{
    VOS_UINT32 ret = ERR_MSP_CFG_LOG_NOT_ALLOW;
    VOS_UINT32 ret2 = ERR_MSP_CFG_LOG_NOT_ALLOW;
    VOS_UINT32 ulMsgCfg;

    /*先判断消息ID过滤功能是否打开，如果有，则根据消息ID过滤*/
    ulMsgCfg = diag_GetLayerMsgCfg(DIAG_CMD_LOG_CATETORY_LAYER_ID, ulMsgId);
    if(ERR_MSP_DIAG_MSG_CFG_NOT_SET == ulMsgCfg )
    {
        ret = diag_GetLayerSrcCfg(ulSrcModuleId);
        ret2 = diag_GetLayerDstCfg(ulDstModuleId);
        if((ERR_MSP_SUCCESS != ret) && (ERR_MSP_SUCCESS != ret2))
        {
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
        else
        {
            return ERR_MSP_SUCCESS;
        }
    }
    else if(ERR_MSP_SUCCESS == ulMsgCfg )
    {
        return ERR_MSP_SUCCESS;
    }
    else
    {
        return ERR_MSP_CFG_LOG_NOT_ALLOW;
    }
}


VOS_UINT32 diag_GetPrintCfg(VOS_UINT32 ulModuleId, VOS_UINT32 ulLevel)
{
    VOS_UINT32 ulLevelFilter = 0;
    VOS_UINT32 ulTmp = 0;

    if((PS_LOG_LEVEL_OFF == ulLevel)||(ulLevel >= PS_LOG_LEVEL_BUTT))
    {
        diag_LNR(EN_DIAG_LNR_LOG_LOST, ulLevel, 0xAAAAAAAA);/* [false alarm]:alarm */
        return ERR_MSP_CFG_LOG_NOT_ALLOW;
    }

    /* 将协议栈的LEVEL值转换成MSP与HSO之间的LEVEL值 */
    /*
        TOOL        <->     MSP     <->     PS
        0x40000000  <->     0x40    <->     1 (ERROR);
        0x20000000  <->     0x20    <->     2 (WARNING);
        0x10000000  <->     0x10    <->     3 (NORMAL);
        0x08000000  <->     0x08    <->     4 (INFO)
    */

    ulLevelFilter = ((VOS_UINT32)1<<(7-ulLevel));

    ulTmp = (ulLevelFilter << 16) | g_PrintTotalCfg;

    /* 打印总开关模块打开情况*/
    if(DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE != g_PrintTotalCfg)
    {
        if(ulLevelFilter & g_PrintTotalCfg)
        {
            return ERR_MSP_SUCCESS;
        }
        else
        {
            diag_LNR(EN_DIAG_LNR_LOG_LOST, ulTmp, ulModuleId);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
    }
    else
    {
        /*获取模块打印开关状态*/
        if(DIAG_CFG_MODULE_IS_INVALID((VOS_INT32)ulModuleId))
        {
            diag_LNR(EN_DIAG_LNR_LOG_LOST, ulModuleId, 0xBBBBBBBB);/* [false alarm]:alarm */
            return  ERR_MSP_CFG_LOG_NOT_ALLOW;
        }

        ulTmp = (ulLevelFilter << 16) | g_PrintModuleCfg[ulModuleId - VOS_PID_DOPRAEND];

        if(ulLevelFilter & g_PrintModuleCfg[ulModuleId - VOS_PID_DOPRAEND])
        {
            return ERR_MSP_SUCCESS;
        }
        else
        {
            diag_LNR(EN_DIAG_LNR_LOG_LOST, ulTmp, ulModuleId);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
    }
}


#define LTE_DIAG_PRINTF_LEN             (256+sizeof(VOS_UINT32)+sizeof(VOS_UINT32))

/*****************************************************************************
 函 数 名  : diag_GetFileNameFromPath
 功能描述  : 得到文件路径名的偏移值
 输入参数  : char* pcFileName
  1. cuijunqiang
*****************************************************************************/
VOS_CHAR * diag_GetFileNameFromPath(VOS_CHAR* pcFileName)
{
    VOS_CHAR    *pcPathPos1;
    VOS_CHAR    *pcPathPos2;
    VOS_CHAR    *pcPathPos;

    /* 操作系统可能使用'\'来查找路径 */
    pcPathPos1 = (VOS_CHAR*)strrchr(pcFileName, '\\');
    if(VOS_NULL_PTR == pcPathPos1)
    {
        pcPathPos1 = pcFileName;
    }

    /* 操作系统可能使用'/'来查找路径 */
    pcPathPos2 = (VOS_CHAR*)strrchr(pcFileName, '/');
    if(VOS_NULL_PTR == pcPathPos2)
    {
        pcPathPos2 = pcFileName;
    }

    pcPathPos = (pcPathPos1 > pcPathPos2) ? pcPathPos1 : pcPathPos2;

    /* 如果没找到'\'或'/'则使用原来的字符串，否则使用截断后的字符串 */
    if (pcFileName != pcPathPos)
    {
        pcPathPos++;
    }

    return pcPathPos;
}


/*****************************************************************************
 函 数 名  : DIAG_LogReport
 功能描述  : 打印上报接口
 输入参数  : ulModuleId( 31-24:modemid,23-16:modeid,15-12:level )
             pcFileName(上报时会把文件路径删除，只保留文件名)
             ulLineNum(行号)
             pszFmt(可变参数)
注意事项   : 由于此接口会被协议栈频繁调用，为提高处理效率，本接口内部会使用1K的局部变量保存上报的字符串信息，
             从而此接口对协议栈有两点限制，一是调用此接口的任务栈中的内存要至少为此接口预留1K空间；
                                           二是调用此接口输出的log不要超过1K（超出部分会自动丢弃）
*****************************************************************************/
VOS_UINT32 DIAG_LogReport(VOS_UINT32 ulModuleId, VOS_UINT32 ulPid, VOS_CHAR *cFileName, VOS_UINT32 ulLineNum, VOS_CHAR* pszFmt, ...)
{
    DIAG_CMD_LOG_PRINT_RAW_TXT_IND_STRU stRptInfo;
    DIAG_SRV_LOG_HEADER_STRU    stLogHeader;
    DIAG_MSG_REPORT_HEAD_STRU   stDiagHead;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_CHAR   *cOffsetName;
    VOS_UINT32 ulDataLength =0;
    VOS_UINT32 ulModule = 0;
    VOS_UINT32 ulLevel = 0;
    va_list    arg;
    VOS_INT32  ulParamLen =0;
    VOS_INT32  usRet =0;
    VOS_ULONG  ulLockLevel;

    /*获取数组下标*/
    ulModule = ulPid;

    /*获取模块开关状态*/
    ulLevel = DIAG_GET_PRINTF_LEVEL(ulModuleId);

    if(!DIAG_IS_POLOG_ON)
    {
        /*检查DIAG是否初始化且HSO是否连接上*/
        if(!DIAG_IS_CONN_ON)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ERR_MSP_NO_INITILIZATION, ulModuleId, 1);
            return ERR_MSP_NO_INITILIZATION;
        }

        ret = diag_GetPrintCfg(ulModule, ulLevel);
        if(ERR_MSP_SUCCESS != ret)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ERR_MSP_UNAVAILABLE, ulModuleId, 2);
            return ERR_MSP_UNAVAILABLE;
        }
    }

    if(VOS_NULL == cFileName)
    {
        cFileName= " ";
    }

    /* 文件名截短 */
    cOffsetName = diag_GetFileNameFromPath(cFileName);

    /*给HSO的打印字符串形式如下:pszFileName[ulLineNum]data。HSO根据中括号[]去截取相应的信息*/
    /* coverity[negative_return_fn] */
    ulParamLen = VOS_nsprintf_s(stRptInfo.szText, DIAG_PRINTF_MAX_LEN, DIAG_PRINTF_MAX_LEN-1,"%s[%d]", cOffsetName, ulLineNum);
    if(ulParamLen < 0)
    {
        diag_error("LTE_LOG_ASSERT\n");
        return ERR_MSP_FAILURE;
    }
    else if(ulParamLen > DIAG_PRINTF_MAX_LEN)
    {
        /* 内存越界，主动复位 */
        mdrv_om_system_error((VOS_INT)DIAG_ERROR_MODID_OVERFLOW, __LINE__, ulParamLen, 0, 0);
        return ERR_MSP_FAILURE;
    }

    (VOS_VOID)VOS_MemSet_s(&arg, (VOS_UINT32)sizeof(arg), 0, (VOS_UINT32)sizeof(va_list));

    va_start(arg, pszFmt);

    /* coverity[negative_returns] */
    usRet = VOS_nvsprintf_s((stRptInfo.szText + ulParamLen), (VOS_UINT32)(DIAG_PRINTF_MAX_LEN - ulParamLen),\
                 (VOS_UINT32)((DIAG_PRINTF_MAX_LEN - ulParamLen) -1), (const VOS_CHAR *)pszFmt, arg);
    if(usRet < 0)
    {
        diag_error("LTE_LOG_ASSERT\n");
        va_end(arg);
        return ERR_MSP_FAILURE;
    }

    ulParamLen += usRet;
    if(ulParamLen > DIAG_PRINTF_MAX_LEN)
    {
        /* 内存越界，主动复位 */
        mdrv_om_system_error((VOS_INT)DIAG_ERROR_MODID_OVERFLOW, __LINE__, ulParamLen, 0, 0);
    }
    va_end(arg);

    stRptInfo.szText[DIAG_PRINTF_MAX_LEN-1] = '\0';
    ulDataLength = VOS_StrNLen(stRptInfo.szText, DIAG_PRINTF_MAX_LEN)+1;

    /*组装DIAG命令参数*/
    stRptInfo.ulModule = ulModule;
    VOS_SpinLockIntLock(&g_DiagLogPktNum.ulPrintLock, ulLockLevel);
    stRptInfo.ulNo = (g_DiagLogPktNum.ulPrintNum)++;
    VOS_SpinUnlockIntUnlock(&g_DiagLogPktNum.ulPrintLock, ulLockLevel);

    /* 1:error, 2:warning, 3:normal, 4:info */
    /* (0|ERROR|WARNING|NORMAL|INFO|0|0|0) */
    stRptInfo.ulLevel  = (0x80000000) >> ulLevel;

    /* 字符串的长度加上信息的长度 */
    ulDataLength += (sizeof(DIAG_CMD_LOG_PRINT_RAW_TXT_IND_STRU) - (DIAG_PRINTF_MAX_LEN+1));

    /* 填充数据头 */
    diag_SvcFillHeader((DIAG_SRV_HEADER_STRU *)&stLogHeader);
    DIAG_SRV_SET_MODEM_ID(&stLogHeader.frame_header, DIAG_GET_MODEM_ID(ulModuleId));
    DIAG_SRV_SET_TRANS_ID(&stLogHeader.frame_header, g_ulTransId++);
    DIAG_SRV_SET_COMMAND_ID(&stLogHeader.frame_header, DIAG_MSG_TYPE_PS, DIAG_GET_MODE_ID(ulModuleId), DIAG_MSG_PS_PRINT, 0);
    DIAG_SRV_SET_MSG_LEN(&stLogHeader.frame_header, ulDataLength);

    stDiagHead.ulHeaderSize     = sizeof(stLogHeader);
    stDiagHead.pHeaderData      = &stLogHeader;

    stDiagHead.ulDataSize       = ulDataLength;
    stDiagHead.pData            = &stRptInfo;

    ret = diag_ServicePackData(&stDiagHead);

    if(ret)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ret, 0, 3);
    }
    else
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_OK, 0, 0, 4);
    }

    return ret;
}


/*****************************************************************************
 函 数 名  : DIAG_TransReport_Ex
 功能描述  : 结构化数据上报扩展接口(比DIAG_TransReport多传入了DIAG_MESSAGE_TYPE)
 输入参数  : DIAG_TRANS_IND_STRU->ulModule( 31-24:modemid,23-16:modeid,15-12:level,11-8:groupid )
             DIAG_TRANS_IND_STRU->ulMsgId(透传命令ID)
             DIAG_TRANS_IND_STRU->ulLength(透传信息的长度)
             DIAG_TRANS_IND_STRU->pData(透传信息)

*****************************************************************************/
VOS_UINT32 DIAG_TransReport_Ex(DIAG_TRANS_IND_STRU *pstData)
{
    DIAG_SRV_TRANS_HEADER_STRU  stTransHeader;
    DIAG_CMD_TRANS_IND_STRU    *pstTransInfo;
    DIAG_MSG_REPORT_HEAD_STRU   stDiagHead;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_ULONG  ulLockLevel;

    /*检查DIAG是否初始化且HSO是否连接上*/
    if((!DIAG_IS_CONN_ON) && (!DIAG_IS_POLOG_ON))
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
        return ERR_MSP_NO_INITILIZATION;
    }


    /*检查参数合法性*/
    if((VOS_NULL_PTR == pstData) || (NULL == pstData->pData) || (0 == pstData->ulLength))
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    diag_LNR(EN_DIAG_LNR_TRANS_IND, pstData->ulMsgId, VOS_GetSlice());

    pstTransInfo = &stTransHeader.trans_header;
    pstTransInfo->ulModule = pstData->ulPid;
    pstTransInfo->ulMsgId  = pstData->ulMsgId;

    VOS_SpinLockIntLock(&g_DiagLogPktNum.ulTransLock, ulLockLevel);
    pstTransInfo->ulNo     = (g_DiagLogPktNum.ulTransNum)++;
    VOS_SpinUnlockIntUnlock(&g_DiagLogPktNum.ulTransLock, ulLockLevel);

    (VOS_VOID)VOS_MemSet_s(&stDiagHead, (VOS_UINT32)sizeof(stDiagHead), 0, sizeof(stDiagHead));

    /* 填充数据头 */
    diag_SvcFillHeader((DIAG_SRV_HEADER_STRU *)&stTransHeader);
    DIAG_SRV_SET_MODEM_ID(&stTransHeader.frame_header, DIAG_GET_MODEM_ID(pstData->ulModule));
    DIAG_SRV_SET_TRANS_ID(&stTransHeader.frame_header, g_ulTransId++);
    DIAG_SRV_SET_COMMAND_ID(&stTransHeader.frame_header, DIAG_GET_GROUP_ID(pstData->ulModule),
                            DIAG_GET_MODE_ID(pstData->ulModule), DIAG_MSG_PS_STRUCT, ((pstData->ulMsgId)&0x7ffff));
    DIAG_SRV_SET_MSG_LEN(&stTransHeader.frame_header, sizeof(stTransHeader.trans_header) + pstData->ulLength);

    stDiagHead.ulHeaderSize     = sizeof(stTransHeader);
    stDiagHead.pHeaderData      = &stTransHeader;
    stDiagHead.ulDataSize       = pstData->ulLength;
    stDiagHead.pData            = pstData->pData;

    ret = diag_ServicePackData(&stDiagHead);

    if(ret)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ret, 0, 4);
    }
    else
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_OK, pstData->ulMsgId, VOS_GetSlice(), 5);
    }

    return ret;
}



VOS_UINT32 DIAG_TransReport(DIAG_TRANS_IND_STRU *pstData)
{
    /*检查DIAG是否初始化且HSO是否连接上*/
    if((!DIAG_IS_CONN_ON) && (!DIAG_IS_POLOG_ON))
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
        return ERR_MSP_NO_INITILIZATION;
    }

    /*检查参数合法性*/
    if((VOS_NULL_PTR == pstData) || (NULL == pstData->pData) || (0 == pstData->ulLength))
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    pstData->ulModule = DIAG_GEN_MODULE_EX( DIAG_GET_MODEM_ID(pstData->ulModule), \
                                            DIAG_GET_MODE_ID(pstData->ulModule),  \
                                            DIAG_MSG_TYPE_PS);

    return DIAG_TransReport_Ex(pstData);
}



VOS_UINT32 DIAG_EventReport(DIAG_EVENT_IND_STRU *pstEvent)
{
    DIAG_MSG_REPORT_HEAD_STRU   stDiagHead;
    DIAG_CMD_LOG_EVENT_IND_STRU *pstEventIndInfo;
    DIAG_SRV_EVENT_HEADER_STRU  stEventHeader;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_ULONG  ulLockLevel;

    if(VOS_NULL_PTR == pstEvent)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ERR_MSP_INVALID_PARAMETER, 0, 2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    if(!DIAG_IS_POLOG_ON)
    {
        /*检查是否允许事件上报*/
        if(!DIAG_IS_EVENT_ON)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
            return ERR_MSP_NO_INITILIZATION;
        }

        /* 融合GU的event机制后，需要判断子开关 */
        if((DIAG_CFG_MODULE_IS_VALID(pstEvent->ulPid))&&(0 == g_EventModuleCfg[pstEvent->ulPid - VOS_PID_DOPRAEND]))
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ERR_MSP_INVALID_PARAMETER, 0, 3);
            return ERR_MSP_INVALID_PARAMETER;
        }
    }

    pstEventIndInfo = &stEventHeader.event_header;

    /*组装DIAG命令参数*/
    VOS_SpinLockIntLock(&g_DiagLogPktNum.ulEventLock, ulLockLevel);
    pstEventIndInfo->ulNo     = (g_DiagLogPktNum.ulEventNum)++;
    VOS_SpinUnlockIntUnlock(&g_DiagLogPktNum.ulEventLock, ulLockLevel);

    pstEventIndInfo->ulId     = pstEvent->ulEventId;
    pstEventIndInfo->ulModule = pstEvent->ulPid;

    /* 填充数据头 */
    diag_SvcFillHeader((DIAG_SRV_HEADER_STRU *)&stEventHeader);
    DIAG_SRV_SET_MODEM_ID(&stEventHeader.frame_header, DIAG_GET_MODEM_ID(pstEvent->ulModule));
    DIAG_SRV_SET_TRANS_ID(&stEventHeader.frame_header, g_ulTransId++);
    DIAG_SRV_SET_COMMAND_ID(&stEventHeader.frame_header, DIAG_MSG_TYPE_PS,
                            DIAG_GET_MODE_ID(pstEvent->ulModule), DIAG_MSG_PS_EVENT, 0);
    DIAG_SRV_SET_MSG_LEN(&stEventHeader.frame_header, sizeof(stEventHeader.event_header) + pstEvent->ulLength);

    stDiagHead.ulHeaderSize     = sizeof(stEventHeader);
    stDiagHead.pHeaderData      = &stEventHeader;
    stDiagHead.ulDataSize       = pstEvent->ulLength;
    stDiagHead.pData            = pstEvent->pData;

    ret = diag_ServicePackData(&stDiagHead);
    if(ret)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ret, 0, 4);
    }
    else
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_OK, 0, 0, 5);
    }
    return ret;
}



VOS_UINT32 DIAG_AirMsgReport(DIAG_AIR_IND_STRU *pstAir)
{
    DIAG_MSG_REPORT_HEAD_STRU   stDiagHead;
    DIAG_SRV_AIR_HEADER_STRU    stAirHeader;
    DIAG_CMD_LOG_AIR_IND_STRU   *pstRptInfo = VOS_NULL;
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_ULONG  ulLockLevel;

    /*检查是否允许LT 空口上报*/
    if ((!DIAG_IS_LT_AIR_ON) && (!DIAG_IS_POLOG_ON))
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
        return ERR_MSP_NO_INITILIZATION;
    }

    /*检查参数合法性*/
    if((VOS_NULL == pstAir) || (VOS_NULL == pstAir->pData) || (0 == pstAir->ulLength))
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ERR_MSP_INVALID_PARAMETER, 0, 2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    pstRptInfo = &stAirHeader.air_header;

    pstRptInfo->ulModule  = pstAir->ulPid;
    pstRptInfo->ulId      = pstAir->ulMsgId;
    pstRptInfo->ulSide    = pstAir->ulDirection;

    VOS_SpinLockIntLock(&g_DiagLogPktNum.ulAirLock, ulLockLevel);
    pstRptInfo->ulNo      = (g_DiagLogPktNum.ulAirNum)++;
    VOS_SpinUnlockIntUnlock(&g_DiagLogPktNum.ulAirLock, ulLockLevel);

    /* 填充数据头 */
    diag_SvcFillHeader((DIAG_SRV_HEADER_STRU *)&stAirHeader);
    DIAG_SRV_SET_MODEM_ID(&stAirHeader.frame_header, DIAG_GET_MODEM_ID(pstAir->ulModule));
    DIAG_SRV_SET_TRANS_ID(&stAirHeader.frame_header, g_ulTransId++);
    DIAG_SRV_SET_COMMAND_ID(&stAirHeader.frame_header, DIAG_MSG_TYPE_PS,
                            DIAG_GET_MODE_ID(pstAir->ulModule), DIAG_MSG_PS_AIR, 0);
    DIAG_SRV_SET_MSG_LEN(&stAirHeader.frame_header, sizeof(stAirHeader.air_header) + pstAir->ulLength);

    stDiagHead.ulHeaderSize     = sizeof(stAirHeader);
    stDiagHead.pHeaderData      = &stAirHeader;

    stDiagHead.ulDataSize       = pstAir->ulLength;
    stDiagHead.pData            = pstAir->pData;

    ret = diag_ServicePackData(&stDiagHead);

    if(ret)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ret, 0, 3);
    }
    else
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_OK, 0, 0, 4);
    }

    return ret;
}


VOS_VOID DIAG_TraceReport(VOS_VOID *pMsg)
{
    DIAG_MSG_REPORT_HEAD_STRU   stDiagHead;
    DIAG_SRV_TRACE_HEADER_STRU  stTraceHeader;
    DIAG_CMD_LOG_LAYER_IND_STRU *pstTrace;
    DIAG_OSA_MSG_STRU *pDiagMsg = (DIAG_OSA_MSG_STRU *)pMsg;
    VOS_UINT32  ret, ulSrcModule, ulDstModule, ulMsgId;
    VOS_ULONG   ulLockLevel;
    VOS_UINT32  ulMsgLen;

    if(VOS_NULL == pDiagMsg)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_INVALID_PARAMETER, 0, 1);
        return ;
    }

    ulSrcModule = pDiagMsg->ulSenderPid;
    ulDstModule = pDiagMsg->ulReceiverPid;
    ulMsgId     = pDiagMsg->ulMsgId;

    if(!DIAG_IS_POLOG_ON)
    {
        /*检查DIAG是否初始化且HSO是否连接上*/
        if(!DIAG_IS_CONN_ON)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
            return ;
        }

        /*检查是否允许层间消息上报*/
        ret = diag_GetLayerCfg(ulSrcModule, ulDstModule, ulMsgId);
        if(ret)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 1);
            return ;
        }
    }

    pstTrace = &stTraceHeader.trace_header;
    pstTrace->ulModule    = ulSrcModule;
    pstTrace->ulDestMod   = ulDstModule;
    pstTrace->ulId        = ulMsgId;

    VOS_SpinLockIntLock(&g_DiagLogPktNum.ulTraceLock, ulLockLevel);
    pstTrace->ulNo        = (g_DiagLogPktNum.ulTraceNum++);
    VOS_SpinUnlockIntUnlock(&g_DiagLogPktNum.ulTraceLock, ulLockLevel);

    (VOS_VOID)VOS_MemSet_s(&stDiagHead, (VOS_UINT32)sizeof(stDiagHead), 0, sizeof(stDiagHead));

    /* 填充数据头 */
    diag_SvcFillHeader((DIAG_SRV_HEADER_STRU *)&stTraceHeader);
    DIAG_SRV_SET_MODEM_ID(&stTraceHeader.frame_header, DIAG_MODEM_0);
    DIAG_SRV_SET_TRANS_ID(&stTraceHeader.frame_header, g_ulTransId++);
    DIAG_SRV_SET_COMMAND_ID(&stTraceHeader.frame_header, DIAG_MSG_TYPE_PS,
                            DIAG_MODE_COMM, DIAG_MSG_PS_LAYER, 0);
    ulMsgLen = sizeof(DIAG_OSA_MSG_STRU) - sizeof(pDiagMsg->ulMsgId) + pDiagMsg->ulLength;
    DIAG_SRV_SET_MSG_LEN(&stTraceHeader.frame_header, sizeof(stTraceHeader.trace_header) + ulMsgLen);

    stDiagHead.ulHeaderSize     = sizeof(stTraceHeader);
    stDiagHead.pHeaderData      = &stTraceHeader;
    stDiagHead.ulDataSize       = sizeof(DIAG_OSA_MSG_STRU) - sizeof(pDiagMsg->ulMsgId) + pDiagMsg->ulLength;
    stDiagHead.pData            = pDiagMsg;

    ret = diag_ServicePackData(&stDiagHead);

    if(ret)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ret, 0, 3);
    }
    else
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_OK, 0, 0, 5);
    }
    return ;
}




VOS_VOID DIAG_LayerMsgReport(VOS_VOID *pMsg)
{
    DIAG_SRV_LAYER_HEADER_STRU  stLayerHeader;
    DIAG_CMD_LOG_LAYER_IND_STRU *pstTrace;
    DIAG_MSG_REPORT_HEAD_STRU   stDiagHead;    
    DIAG_OSA_MSG_STRU *pNewMsg  = NULL;
    VOS_UINT32  ret, ulSrcModule, ulDstModule, ulMsgId;
    VOS_ULONG   ulLockLevel;
    VOS_UINT32  ulMsgLen;
    DIAG_OSA_MSG_STRU *pDiagMsg = (DIAG_OSA_MSG_STRU *)pMsg;
    
    if(VOS_NULL == pDiagMsg)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ERR_MSP_INVALID_PARAMETER, 0, 1);
        return ;
    }

    ulSrcModule = pDiagMsg->ulSenderPid;
    ulDstModule = pDiagMsg->ulReceiverPid;
    ulMsgId     = pDiagMsg->ulMsgId;

    if(!DIAG_IS_POLOG_ON)
    {
        /*检查DIAG是否初始化且HSO是否连接上*/
        if(!DIAG_IS_CONN_ON)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
            return ;
        }

        /*检查是否允许层间消息上报*/
        ret = diag_GetLayerCfg(ulSrcModule, ulDstModule, ulMsgId);
        if(ret)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 1);
            return ;
        }
    }

    pNewMsg = pDiagMsg;

    /* 对外提供的消息匹配接口，可能对消息进行替换 */
    /* 返回值为过滤完成后的新消息指针，如果该指针和原指针不一致需要调用notify接口对返回指针进行释放 */
    if(VOS_NULL != g_pLayerMatchFunc)
    {
        pNewMsg = g_pLayerMatchFunc(pDiagMsg);
        if(VOS_NULL == pNewMsg)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_MATCH, ulSrcModule, ulDstModule, ulMsgId);
            return ;
        }
    }

    pstTrace = &stLayerHeader.layer_header;
    pstTrace->ulModule    = ulSrcModule;
    pstTrace->ulDestMod   = ulDstModule;
    /*有些组件match函数中会将msg结构体替换掉，msgid是层间消息结构体在工具显示的依据，因此msgid需要填写替换完成后的新的msgid,否则有可能结构体显示不正确*/
    pstTrace->ulId        = pNewMsg->ulMsgId;

    VOS_SpinLockIntLock(&g_DiagLogPktNum.ulTraceLock, ulLockLevel);
    pstTrace->ulNo        = (g_DiagLogPktNum.ulTraceNum++);
    VOS_SpinUnlockIntUnlock(&g_DiagLogPktNum.ulTraceLock, ulLockLevel);

    /* 填充数据头 */
    diag_SvcFillHeader((DIAG_SRV_HEADER_STRU *)&stLayerHeader);
    DIAG_SRV_SET_MODEM_ID(&stLayerHeader.frame_header, DIAG_MODEM_0);
    DIAG_SRV_SET_TRANS_ID(&stLayerHeader.frame_header, g_ulTransId++);
    DIAG_SRV_SET_COMMAND_ID(&stLayerHeader.frame_header, DIAG_MSG_TYPE_PS,
                            DIAG_MODE_COMM, DIAG_MSG_PS_LAYER, 0);

    ulMsgLen = sizeof(DIAG_OSA_MSG_STRU) - sizeof(pNewMsg->ulMsgId) + pNewMsg->ulLength;
    DIAG_SRV_SET_MSG_LEN(&stLayerHeader.frame_header, sizeof(stLayerHeader.layer_header) + ulMsgLen);

    stDiagHead.ulHeaderSize     = sizeof(stLayerHeader);
    stDiagHead.pHeaderData      = &stLayerHeader;

    stDiagHead.ulDataSize       = ulMsgLen;
    stDiagHead.pData            = pNewMsg;

    ret = diag_ServicePackData(&stDiagHead);

    if(ret)
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ret, 0, 3);
    }
    else
    {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_OK, 0, 0, 5);
    }

    if((pNewMsg != pDiagMsg)&&(g_pLayerMatchNotifyFunc != NULL))
    {
        ret = g_pLayerMatchNotifyFunc(ulSrcModule, pNewMsg);
        if(ret)
        {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_NOTIFY, 0, 0, 6);
        }
    }
    return ;
}


VOS_UINT32 DIAG_LayerMsgMatchFuncReg(DIAG_LayerMsgMatchFunc pFun)
{
    if(VOS_NULL == g_pLayerMatchFunc)
    {
        g_pLayerMatchFunc = pFun;
        return ERR_MSP_SUCCESS;
    }
    else
    {
        return ERR_MSP_FAILURE;
    }
}
/*****************************************************************************
 Function Name   : DIAG_TraceMatchFuncReg
 Description     : 用于注册层间消息匹配后的notify接口(此接口不支持重复注册，多次注册返还失败)
                   用于通知logfilter进行资源释放等操作
 Input           :
 Output          : None
 Return          : 返回值为注册结果: 0-注册成功；其他-注册失败
*****************************************************************************/
VOS_UINT32 DIAG_LayerMsgMatchNotifyFuncReg(DIAG_LayerMsgMatchNotifyFunc pFun)
{
    if(VOS_NULL == g_pLayerMatchNotifyFunc)
    {
        g_pLayerMatchNotifyFunc = pFun;
        return ERR_MSP_SUCCESS;
    }
    else
    {
        return ERR_MSP_FAILURE;
    }
}

/* 为MBB提供的errorlog功能，应推动MBB与phone拉通 */
VOS_UINT32 DIAG_ErrorLog(VOS_CHAR * cFileName,VOS_UINT32 ulFileId, VOS_UINT32 ulLine,
                VOS_UINT32 ulErrNo, VOS_VOID * pBuf, VOS_UINT32 ulLen)
{
    return 0;
}



VOS_UINT32 diag_SendMsg(VOS_UINT32 ulSenderId, VOS_UINT32 ulRecverId, VOS_UINT32 ulMsgId, VOS_UINT8* pDta, VOS_UINT32 dtaSize)
{
    VOS_UINT32 ret = ERR_MSP_UNKNOWN;
    DIAG_DATA_MSG_STRU* pDataMsg = NULL;

    pDataMsg = (DIAG_DATA_MSG_STRU*)VOS_AllocMsg(ulSenderId,(sizeof(DIAG_DATA_MSG_STRU)+ dtaSize - VOS_MSG_HEAD_LENGTH));

    if (pDataMsg != NULL)
    {
        pDataMsg->ulReceiverPid = ulRecverId;
        pDataMsg->ulSenderPid   = ulSenderId;
        pDataMsg->ulLength      = sizeof(DIAG_DATA_MSG_STRU)+ dtaSize - 20;

        pDataMsg->ulMsgId    = ulMsgId;
        pDataMsg->ulLen      = dtaSize;
        if(0 != dtaSize)
        {
            VOS_MemCpy_s(pDataMsg->pContext,pDataMsg->ulLen,pDta,dtaSize);
        }

        ret = VOS_SendMsg(ulSenderId, pDataMsg);
        if (ret != VOS_OK)
        {
            diag_error("ulRecverId=0x%x,ulMsgId=0x%x,dtaSize=0x%x,ret=0x%x\n",ulRecverId,ulMsgId,dtaSize,ret);
        }
        else
        {
            ret = ERR_MSP_SUCCESS;
        }
    }

    return ret;
}


VOS_UINT32 Diag_GetLogLevel(VOS_UINT32 ulPid)
{
    VOS_UINT8 level;

    /*判断模块ID是否在CCPU支持的PS范围内*/
    if ((VOS_PID_DOPRAEND <= ulPid) && (VOS_PID_BUTT > ulPid))
    {
	if( g_PrintTotalCfg == DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE)
	{
		level = g_PrintModuleCfg[ulPid - VOS_PID_DOPRAEND];
	}
	else
	{
		level = (VOS_UINT8)g_PrintTotalCfg;
	}

        /* level中存储的值(0|ERROR|WARNING|NORMAL|INFO|0|0|0) bit 6-3 分别表示ERROR-INFO */

        if(level & 0x08)
        {
            return PS_LOG_LEVEL_INFO;
        }
        else if(level & 0x10)
        {
            return PS_LOG_LEVEL_NORMAL;
        }
        else if(level & 0x20)
        {
            return PS_LOG_LEVEL_WARNING;
        }
        else if(level & 0x40)
        {
            return PS_LOG_LEVEL_ERROR;
        }
        else
        {
            return PS_LOG_LEVEL_OFF;
        }
    }

    return PS_LOG_LEVEL_OFF;
}



/*****************************************************************************
 Function Name   : diag_FailedCmdCnf
 Description     : MSP处理失败的诊断命令应答处理
 Input           : pData     诊断命令请求的内容
                   ulErrcode 给工具返回的错误码
 Output          : None
 Return          : VOS_UINT32

 History         :
    1.c64416      2016-02-15  Draft Enact

*****************************************************************************/
VOS_UINT32 diag_FailedCmdCnf(DIAG_FRAME_INFO_STRU *pData, VOS_UINT32 ulErrcode)
{
    DIAG_COMM_CNF_STRU stCnfCmd   = {0};
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = {0};

    mdrv_diag_PTR(EN_DIAG_PTR_FAIL_CMD_CNF, 1, pData->ulCmdId, ulErrcode);

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnfCmd, pData);

    stDiagInfo.ulMsgType = pData->stID.pri4b;

    stCnfCmd.ulRet = ulErrcode;

    return DIAG_MsgReport(&stDiagInfo, &stCnfCmd, sizeof(stCnfCmd));
}


/*****************************************************************************
 Function Name   : diag_IsPowerOnLogOpen
 Description     : 获取协议栈的开机log上报功能是否打开
 Input           : None
 Output          : None
 Return          : VOS_TRUE:yes ; VOS_FALSE:no

 History         :
    1.c64416      2016-07-18  Draft Enact

*****************************************************************************/
VOS_BOOL diag_IsPowerOnLogOpen(VOS_VOID)
{
    VOS_UINT32          ulRet;
    VOS_INT32           stPowerOnLogA;
    NV_POWER_ON_LOG_SWITCH_STRU stNVPowerOnLog = {};

    stPowerOnLogA = mdrv_diag_shared_mem_read(POWER_ON_LOG_A);

    if(stPowerOnLogA)
    {
        ulRet =  mdrv_nv_read(EN_NV_ID_POWER_ON_LOG_SWITCH, &stNVPowerOnLog, sizeof(stNVPowerOnLog));
        if((ERR_MSP_SUCCESS == ulRet) && (stNVPowerOnLog.cPowerOnlogC))
        {
            return VOS_TRUE;
        }
        return VOS_FALSE;
    }

    return VOS_FALSE;

}


EXPORT_SYMBOL(DIAG_LogPortSwich);




