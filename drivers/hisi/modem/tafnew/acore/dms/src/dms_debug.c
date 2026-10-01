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
*****************************************************************************/

#include <dms.h>
#include "dms_core.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID                    PS_FILE_ID_DMS_DEBUG_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

DMS_DEBUG_INFO_TBL_STRU                 g_astDmsSdmInfoTable[DMS_SDM_DBG_INFO_MAX];
DMS_DEBUG_AT_SERV_NV_STRU               g_dms_debug_atserv_nv_info = {0};
VOS_UINT32                              g_ulDmsDebugLevel = DMS_LOG_LEVEL_NONE;
DMS_MNTN_NLK_STATS_STRU                 g_stDmsMntnNlkStats = {0};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

DMS_DEBUG_INFO_TBL_STRU* DMS_GetDebugInfo(VOS_VOID)
{
   return &(g_astDmsSdmInfoTable[0]);
}

VOS_VOID DMS_Debug(DMS_SDM_MSG_ID_ENUM ulType,
    VOS_UINT32 ulRserved1, VOS_UINT32 ulRserved2, VOS_UINT32 ulRserved3)
{
    g_astDmsSdmInfoTable[ulType].ulCalledNum += 1;
    g_astDmsSdmInfoTable[ulType].ulRserved1 = ulRserved1;
    g_astDmsSdmInfoTable[ulType].ulRserved2 = ulRserved2;
    g_astDmsSdmInfoTable[ulType].ulRserved3 = ulRserved3;
    g_astDmsSdmInfoTable[ulType].ulRtcTime  = VOS_GetTick();
    return;
}

VOS_VOID DMS_Debug_SetDataSize(VOS_UINT32 ullength)
{
    DMS_NLK_ENTITY_STRU                *pstNlkEntity = VOS_NULL_PTR;

    pstNlkEntity = DMS_GET_NLK_ENTITY();

    pstNlkEntity->ulThreshSize = ullength;

    return;
}


VOS_VOID DMS_SetLogLevel(VOS_UINT32 ulLvl)
{
    if (ulLvl >= DMS_LOG_LEVEL_TOP)
    {
        g_ulDmsDebugLevel = DMS_LOG_LEVEL_TOP;
    }
    else
    {
        g_ulDmsDebugLevel = ulLvl;
    }

    return;
}


VOS_VOID DMS_ShowAtServerNvInfo(VOS_VOID)
{
    PS_PRINTF("Operation return value is %d.\n", g_dms_debug_atserv_nv_info.lOperatRet);
    PS_PRINTF("The nv value is %d.\n", g_dms_debug_atserv_nv_info.ulNvValue);
}


VOS_VOID  DMS_ShowPortCfgInfo(VOS_VOID)
{
    PS_PRINTF("ulPortCfgValue: %d\n", g_stDmsMainInfo.ulPortCfgValue);
    PS_PRINTF("bNveReadFlg: %d\n", g_stDmsMainInfo.bPortCfgFlg);
    PS_PRINTF("bPortOpenFlg: %d\n", g_stDmsMainInfo.bPortOpenFlg);

    return;
}

VOS_VOID DMS_ShowVcomStats(VOS_VOID)
{
    DMS_DEBUG_INFO_TBL_STRU* pstTable;

    /* 如果某个成员在某个类别没有使用可以不打印 */

    pstTable = DMS_GetDebugInfo();


    PS_PRINTF("*------------ENABLE-------------------\n");
    PS_PRINTF("USB ENABLE counts  : %d \n USB ENABLE fail counts  : %d \n USB ENABLE fail branch  : %d \n\n",
        pstTable[DMS_SDM_USB_ENABLE].ulCalledNum,
        pstTable[DMS_SDM_USB_ENABLE_ERR].ulCalledNum,
        pstTable[DMS_SDM_USB_ENABLE_ERR].ulRserved3);
    PS_PRINTF("*------------DISABLE-------------------\n");
    PS_PRINTF("USB DISABLE counts  : %d \n USB DISABLE fail counts  : %d \n USB DISABLE fail branch  : %d \n\n",
        pstTable[DMS_SDM_USB_DISABLE].ulCalledNum,
        pstTable[DMS_SDM_USB_DISABLE_ERR].ulCalledNum,
        pstTable[DMS_SDM_USB_DISABLE_ERR].ulRserved3);

    PS_PRINTF("*------------OPEN-------------------\n");
    PS_PRINTF("VCOM_PCUI OPEN counts  : %d \n VCOM_PCUI OPEN fail counts  : %d \n  VCOM_PCUI OPEN fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_OPEN_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI].ulRserved1);
    PS_PRINTF("VCOM_CTRL OPEN counts  : %d \n VCOM_CTRL OPEN fail counts  : %d \n  VCOM_CTRL OPEN fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_OPEN_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_CTRL].ulRserved1);
    PS_PRINTF("VCOM_PCUI2 OPEN counts  : %d \n VCOM_PCUI2 OPEN fail counts  : %d \n  VCOM_PCUI2 OPEN fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_OPEN_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_OPEN_ERR_PCUI2].ulRserved1);

    PS_PRINTF("*------------CLOSE-------------------\n");
    PS_PRINTF("VCOM_AT CLOSE counts  : %d \n VCOM_AT CLOSE fail counts  : %d \n  VCOM_AT CLOSE fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_CLOSE_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI].ulRserved1);
    PS_PRINTF("VCOM_CTRL CLOSE counts  : %d \n VCOM_CTRL CLOSE fail counts  : %d \n  VCOM_CTRL CLOSE fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_CLOSE_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_CTRL].ulRserved1);
    PS_PRINTF("VCOM_PCUI2 CLOSE counts  : %d \n VCOM_PCUI2 CLOSE fail counts  : %d \n  VCOM_PCUI2 CLOSE fail value : %d\n\n",
        pstTable[DMS_SDM_VCOM_CLOSE_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_CLOSE_ERR_PCUI2].ulRserved1);

    PS_PRINTF("*------------WRITE_异步写-------------------\n");
    PS_PRINTF("VCOM_AT write counts  : %d \n VCOM_AT write OK counts  : %d \n  VCOM_AT Last write OK len : %d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI].ulRserved1);
     PS_PRINTF("VCOM_CTRL write counts  : %d \n VCOM_CTRL write OK counts  : %d \n  VCOM_CTRL Last write OK len : %d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_CTRL].ulRserved1);
     PS_PRINTF("VCOM_PCUI2 write counts  : %d \n VCOM_PCUI2 write OK counts  : %d \n  VCOM_PCUI2 Last write OK len : %d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_PCUI2].ulRserved1);

    PS_PRINTF("*------------WRITE CALL BACK-------------------\n");
    PS_PRINTF("VCOM_AT WRITE CALL BACK counts  : %d \n VCOM_AT WRITE CALL BACK fail counts  : %d \n  VCOM_AT VCOM_AT fail addr : %d\nVCOM2 WRITE fail len :%d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CB_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI].ulRserved2);
    PS_PRINTF("VCOM_CTRL WRITE CALL BACK counts  : %d \n VCOM_CTRL WRITE CALL BACK fail counts  : %d \n  VCOM_CTRL VCOM_AT fail addr : %d\nVCOM2 WRITE fail len :%d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CB_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_CTRL].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_CTRL].ulRserved2);
    PS_PRINTF("VCOM_PCUI2 WRITE CALL BACK counts  : %d \n VCOM_PCUI2 WRITE CALL BACK fail counts  : %d \n  VCOM_PCUI2 VCOM_AT fail addr : %d\nVCOM2 WRITE fail len :%d\n\n",
        pstTable[DMS_SDM_VCOM_WRT_CB_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI2].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_CB_ERR_PCUI2].ulRserved2);

    PS_PRINTF("*------------EVT CALL BACK-------------------\n");
    PS_PRINTF("VCOM_PCUI EVT CALL BACK counts  : %d \n VCOM_PCUI EVT  type : %d\n\n",
        pstTable[DMS_SDM_VCOM_EVT_PCUI].ulCalledNum,
        pstTable[DMS_SDM_VCOM_EVT_PCUI].ulRserved1);
    PS_PRINTF("VCOM_CTRL EVT CALL BACK counts  : %d \n VCOM_CTRL EVT  type : %d\n\n",
        pstTable[DMS_SDM_VCOM_EVT_CTRL].ulCalledNum,
        pstTable[DMS_SDM_VCOM_EVT_CTRL].ulRserved1);
    PS_PRINTF("VCOM_PCUI2 EVT CALL BACK counts  : %d \n VCOM_PCUI2 EVT  type : %d\n\n",
        pstTable[DMS_SDM_VCOM_EVT_PCUI2].ulCalledNum,
        pstTable[DMS_SDM_VCOM_EVT_PCUI2].ulRserved1);

    PS_PRINTF("*------------NCM SEND TO DRV INFO------------------\n");
    PS_PRINTF("发送到驱动次数   : %d\n 发送buf: %x\n 发送长度:%d\n 驱动返回成功次数  : %d \n\n",
        pstTable[DMS_SDM_VCOM_WRT_NCM].ulCalledNum,
        pstTable[DMS_SDM_VCOM_WRT_NCM].ulRserved1,
        pstTable[DMS_SDM_VCOM_WRT_NCM].ulRserved2,
        pstTable[DMS_SDM_VCOM_WRT_SUSS_NCM].ulCalledNum);

    return;
}



VOS_VOID DMS_ShowNlkUlStats(VOS_VOID)
{
    PS_PRINTF("====================================\n");
    PS_PRINTF("DMS NLK UL STATS                    \n");
    PS_PRINTF("====================================\n");
    PS_PRINTF("Create SOCK Fail NUM:           %u\n", g_stDmsMntnNlkStats.ulCreatSockFailNum);
    PS_PRINTF("Total Total MSG NUM:            %u\n", g_stDmsMntnNlkStats.ulUlTotalMsgNum);
    PS_PRINTF("ERR MSG NUM:                    %u\n", g_stDmsMntnNlkStats.ulUlErrMsgNum);
    PS_PRINTF("UnSupport Input Log Num:        %u\n", g_stDmsMntnNlkStats.ulUlUnSupportInputLogNum);
    PS_PRINTF("UnSupport Write Log Num:        %u\n", g_stDmsMntnNlkStats.ulUlUnSupportWriteLogNum);
    PS_PRINTF("UNKNOWN MSG NUM:                %u\n", g_stDmsMntnNlkStats.ulUlUnknownMsgNum);
    PS_PRINTF("Send LTE CFG MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_LTE_CFG]);
    PS_PRINTF("Send LTE CTRL MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_LTE_CTRL]);
    PS_PRINTF("Send LTE DATA MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_LTE_DATA]);
    PS_PRINTF("Send GU CFG MSG NUM:            %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_GU_CFG]);
    PS_PRINTF("Send GU DATA MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlSendMsgNum[DMS_NLK_MSG_TYPE_GU_DATA]);
    PS_PRINTF("Free LTE CFG MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_LTE_CFG]);
    PS_PRINTF("Free LTE CTRL MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_LTE_CTRL]);
    PS_PRINTF("Free LTE DATA MSG NUM:          %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_LTE_DATA]);
    PS_PRINTF("Free GU CFG MSG NUM:            %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_GU_CFG]);
    PS_PRINTF("Free GU DATA MSG NUM:           %u\n", g_stDmsMntnNlkStats.aulUlFreeMsgNum[DMS_NLK_MSG_TYPE_GU_DATA]);

    return;
}


VOS_VOID DMS_ShowNlkDlStats(VOS_VOID)
{
    PS_PRINTF("====================================\n");
    PS_PRINTF("DMS NLK DL STATS                    \n");
    PS_PRINTF("====================================\n");
    PS_PRINTF("Total PKT NUM:                  %u\n", g_stDmsMntnNlkStats.ulDlTotalPktNum);
    PS_PRINTF("ERR PARA PKT NUM:               %u\n", g_stDmsMntnNlkStats.ulDlErrParaPktNum);
    PS_PRINTF("ERR CHAN PKT NUM:               %u\n", g_stDmsMntnNlkStats.ulDlErrChanPktNum);
    PS_PRINTF("LTE CTRL PKT NUM:               %u\n", g_stDmsMntnNlkStats.aulDlNormChanPktNum[DMS_OM_CHAN_LTE_CTRL]);
    PS_PRINTF("LTE DATA PKT NUM:               %u\n", g_stDmsMntnNlkStats.aulDlNormChanPktNum[DMS_OM_CHAN_LTE_DATA]);
    PS_PRINTF("GU DATA PKT NUM:                %u\n", g_stDmsMntnNlkStats.aulDlNormChanPktNum[DMS_OM_CHAN_GU_DATA]);
    PS_PRINTF("Total MSG NUM:                  %u\n", g_stDmsMntnNlkStats.ulDlTotalMsgNum);
    PS_PRINTF("ERR SOCK MSG NUM:               %u\n", g_stDmsMntnNlkStats.ulDlErrSockMsgNum);
    PS_PRINTF("ERR PID MSG NUM:                %u\n", g_stDmsMntnNlkStats.ulDlErrPidMsgNum);
    PS_PRINTF("Alloc MSG Fail NUM:             %u\n", g_stDmsMntnNlkStats.ulDlAllocMsgFailNum);
    PS_PRINTF("Put MSG Fail NUM:               %u\n", g_stDmsMntnNlkStats.ulDlPutMsgFailNum);
    PS_PRINTF("Unicast Fail NUM:               %u\n", g_stDmsMntnNlkStats.ulDlUnicastMsgFailNum);
    PS_PRINTF("Unicast Succ NUM:               %u\n", g_stDmsMntnNlkStats.ulDlUnicastMsgSuccNum);

    return;
}


VOS_VOID DMS_Help(VOS_VOID)
{
    PS_PRINTF("****************************************************\n");
    PS_PRINTF("DMS软调信息                                         \n");
    PS_PRINTF("****************************************************\n");
    PS_PRINTF("DMS_SetLogLevel                                     \n");
    PS_PRINTF("DMS_ShowDebugInfo                                   \n");
    PS_PRINTF("DMS_ShowAtServerNvInfo                              \n");
    PS_PRINTF("DMS_ShowPortCfgInfo                                 \n");
    PS_PRINTF("DMS_ShowVcomStats                                   \n");
    PS_PRINTF("DMS_ShowNlkEntityInfo                               \n");
    PS_PRINTF("DMS_ShowNlkUlStats                                  \n");
    PS_PRINTF("DMS_ShowNlkDlStats                                  \n");

    return;
}



