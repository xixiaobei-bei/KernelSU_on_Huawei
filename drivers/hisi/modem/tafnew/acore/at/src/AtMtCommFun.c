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
#include "AtMtInterface.h"
#include "TafDrvAgent.h"
#include "AtParse.h"
#include "AtMtCommFun.h"
#include "AtMtMsgProc.h"
#include "ATCmdProc.h"
#include "AtInputProc.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_MT_COMM_FUN_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
AT_DEVICE_CMD_CTRL_STRU                 g_stAtDevCmdCtrl        = {0};




/*****************************************************************************
  3 旧设置函数实现
*****************************************************************************/

VOS_UINT32  At_CheckSupportFChanRat(VOS_VOID)
{
    if ((AT_RAT_MODE_GSM != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_EDGE != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_WCDMA != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_AWS != gastAtParaList[0].ulParaValue)
     && (AT_RAT_MODE_CDMA != gastAtParaList[0].ulParaValue)
    )
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}


VOS_UINT32  At_CheckWifiFChanPara(VOS_VOID)
{
    if (((AT_RAT_MODE_WIFI == gastAtParaList[0].ulParaValue) && (AT_BAND_WIFI != gastAtParaList[1].ulParaValue))
      ||((AT_BAND_WIFI == gastAtParaList[1].ulParaValue)&&(AT_RAT_MODE_WIFI != gastAtParaList[0].ulParaValue)))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT32  At_SetFChanPara(VOS_UINT8 ucIndex )
{
    DRV_AGENT_FCHAN_SET_REQ_STRU         stFchanSetReq;

    /* 调用 LTE 模的接口分支 */

    if ( (AT_RAT_MODE_FDD_LTE == gastAtParaList[0].ulParaValue)
       ||(AT_RAT_MODE_TDD_LTE == gastAtParaList[0].ulParaValue))
    {

        g_stAtDevCmdCtrl.ucDeviceRatMode = (AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)(gastAtParaList[0].ulParaValue);

        return atSetFCHANPara(ucIndex);
    }


    if(AT_RAT_MODE_TDSCDMA == gastAtParaList[0].ulParaValue)
    {
        g_stAtDevCmdCtrl.ucDeviceRatMode = (AT_DEVICE_CMD_RAT_MODE_ENUM_UINT8)(gastAtParaList[0].ulParaValue);
        return atSetFCHANPara(ucIndex);
    }


    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FCHAN_OTHER_ERR;
    }
        /* 参数不符合要求 */
    if (gucAtParaIndex != 3)
    {
        return AT_FCHAN_OTHER_ERR;
    }

    /* 清除WIFI模式 */
    /* WIFI的第一个参数必须为8，第二个参数必须为15*/
    if (VOS_FALSE == At_CheckWifiFChanPara())
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* WIFI 分支 */
    if (BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI))
    {
        if (AT_RAT_MODE_WIFI == gastAtParaList[0].ulParaValue)
        {
            /*WIFI未Enable直接返回失败*/
            if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
            {
                return AT_FCHAN_OTHER_ERR;
            }

            g_stAtDevCmdCtrl.ucDeviceRatMode = AT_RAT_MODE_WIFI;

            return AT_OK;
        }
    }
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 检查FCHAN 的接入模式是否支持*/
    if (VOS_FALSE == At_CheckSupportFChanRat())
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (AT_BAND_BUTT <= gastAtParaList[1].ulParaValue)
    {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    TAF_MEM_SET_S(&stFchanSetReq, sizeof(stFchanSetReq), 0x00, sizeof(DRV_AGENT_FCHAN_SET_REQ_STRU));

    stFchanSetReq.ucLoadDspMode       = At_GetDspLoadMode (gastAtParaList[0].ulParaValue);
    stFchanSetReq.ucCurrentDspMode    = At_GetDspLoadMode (g_stAtDevCmdCtrl.ucDeviceRatMode);  /*当前接入模式 */
    stFchanSetReq.bDspLoadFlag        = g_stAtDevCmdCtrl.bDspLoadFlag;
    stFchanSetReq.ucDeviceRatMode     = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stFchanSetReq.ucDeviceAtBand      = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stFchanSetReq.usChannelNo         = (VOS_UINT16)gastAtParaList[2].ulParaValue;

    if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                              gastAtClientTab[ucIndex].opId,
                                              DRV_AGENT_FCHAN_SET_REQ,
                                              &stFchanSetReq,
                                              sizeof(stFchanSetReq),
                                              I0_WUEPS_PID_DRV_AGENT))
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FCHAN_SET;             /*设置当前操作模式 */
        return AT_WAIT_ASYNC_RETURN;                                           /* 等待异步事件返回 */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 AT_SetWifiFwavePara(VOS_VOID)
{
    VOS_CHAR                        acCmd[200] = {0};
    
    /*WIFI未Enable直接返回失败*/
    if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
    {
        return AT_ERROR;
    }

    /*向WIFI发送单音波形信号请求*/
    VOS_sprintf_s(acCmd, sizeof(acCmd), "athtestcmd -ieth0 --tx sine --txpwr %d", gastAtParaList[1].ulParaValue/100);

    WIFI_TEST_CMD(acCmd);

    return AT_OK;
}


VOS_UINT32 AT_SetFwavePara(VOS_UINT8 ucIndex)
{
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
        || (AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFWAVEPara(ucIndex);
    }

    if (AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFWAVEPara(ucIndex);
    }

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数不正确，必须包括波形类型和波形功率 */
    if (gucAtParaIndex != 2)
    {
        return AT_ERROR;
    }

    /* 目前波形类型只支持设置单音*/
    if (gastAtParaList[0].ulParaValue > 7)
    {
        return AT_ERROR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* WIFI*/
    if (AT_RAT_MODE_WIFI == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_SetWifiFwavePara();
    }


    /* 把设置保存在本地变量
       AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值
       功率值以0.01为单位，传给DSP的值会除10，所以AT还需要将该值再除10*/
    g_stAtDevCmdCtrl.usPower    = (VOS_UINT16)(gastAtParaList[1].ulParaValue/10);
    g_stAtDevCmdCtrl.bPowerFlag = VOS_TRUE;
    g_stAtDevCmdCtrl.bFdacFlag  = VOS_FALSE;
    /* 记录下type信息，并转换为G物理层使用的TxMode，在向物理层发送ID_AT_GHPA_RF_TX_CFG_REQ时携带 */
    if (0 == gastAtParaList[0].ulParaValue)
    {
        g_stAtDevCmdCtrl.usTxMode = 8;
    }
    else
    {
        g_stAtDevCmdCtrl.usTxMode = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    }


    /* WCDMA */
    if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* 向WDSP发送开关单音信号的原语请求 */
        if (AT_FAILURE == At_SendContinuesWaveOnToHPA(g_stAtDevCmdCtrl.usPower, ucIndex))
        {
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FWAVE;
        g_stAtDevCmdCtrl.ucIndex = ucIndex;

        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }

    if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        /* 向X DSP发送开关单音信号的原语请求 */
        if (AT_FAILURE == At_SendContinuesWaveOnToCHPA(WDSP_CTRL_TX_ONE_TONE, g_stAtDevCmdCtrl.usPower))
        {
            return AT_ERROR;
        }

        /* 设置当前操作类型 */
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FWAVE;
        g_stAtDevCmdCtrl.ucIndex = ucIndex;

        return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */
    }

    return AT_OK;

}


VOS_UINT32 At_CheckFTxonPara(VOS_UINT8 ucSwitch)
{
    if (ucSwitch > AT_DSP_RF_SWITCH_ON)
    {
        return AT_FTXON_OTHER_ERR;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    return AT_OK;
}



VOS_UINT32  At_SetFTxonPara(VOS_UINT8 ucIndex )
{
    TAF_UINT8                           ucSwitch;
    VOS_UINT32                          ulResult;

    /* 添加 LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFTXONPara(ucIndex);
    }

    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFTXONPara(ucIndex);
    }

    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FTXON_OTHER_ERR;
    }
    /* 参数不符合要求 */
    if (1 != gucAtParaIndex)
    {
        return AT_FTXON_OTHER_ERR;
    }

    ucSwitch = (VOS_UINT8) gastAtParaList[0].ulParaValue;
    g_stAtDevCmdCtrl.ucTempRxorTxOnOff = ucSwitch;
    ulResult = At_CheckFTxonPara(ucSwitch);
    if (AT_OK != ulResult)
    {
        return ulResult;
    }    

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (AT_FAILURE == At_SendTxOnOffToHPA(ucSwitch, ucIndex))
        {
            return AT_FTXON_SET_FAIL;
        }
    }
    else if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        if (AT_FAILURE == At_SendTxOnOffToCHPA(ucSwitch))
        {
            return AT_FTXON_SET_FAIL;
        }
    }
    else
    {
        if (AT_FAILURE == At_SendTxOnOffToGHPA(ucIndex, ucSwitch))
        {
            return AT_FTXON_SET_FAIL;
        }
    }
    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FTXON;
    g_stAtDevCmdCtrl.ucIndex = ucIndex;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */

}


VOS_UINT32  At_SetFRxonPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                           ulSwitch;

     /* 调用 LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFRXONPara(ucIndex);
    }

    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFRXONPara(ucIndex);
    }


    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FRXON_OTHER_ERR;
    }

    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_FRXON_OTHER_ERR;
    }

    /* 该AT命令在AT^TMODE=1非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该AT命令需要在AT^FCHAN设置非信令信道命令执行成功后使用 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    ulSwitch = gastAtParaList[0].ulParaValue;
    g_stAtDevCmdCtrl.ucTempRxorTxOnOff = (AT_DSP_RF_ON_OFF_ENUM_UINT8)ulSwitch;

    /* 把开关接收机请求发给W物理层 */
    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS   == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (AT_FAILURE == At_SendRxOnOffToHPA(ulSwitch, ucIndex))
        {
            return AT_FRXON_SET_FAIL;
        }
    }
    else if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        if (AT_FAILURE == At_SendRxOnOffToCHPA(ulSwitch))
        {
            return AT_FRXON_SET_FAIL;
        }
    }
    else
    {
        /* 把开关接收机请求发给G物理层 */
        if (AT_FAILURE == At_SendRxOnOffToGHPA(ucIndex, ulSwitch))
        {
            return AT_FRXON_SET_FAIL;
        }
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_SET_FRXON;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;

}


VOS_UINT32  AT_ProcTSelRfWifiPara(VOS_VOID)
{
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, begin */
    if ( BSP_MODULE_SUPPORT == mdrv_misc_support_check(BSP_MODULE_TYPE_WIFI) )
    {
        /*WIFI未Enable直接返回失败*/
        if(VOS_FALSE == (VOS_UINT32)WIFI_GET_STATUS())
        {
            return AT_ERROR;
        }

        g_stAtDevCmdCtrl.ucDeviceRatMode = AT_RAT_MODE_WIFI;

        return AT_OK;
    }
    else
    {
        return AT_ERROR;
    }
    /* Modified by s62952 for BalongV300R002 Build优化项目 2012-02-28, end */
}


VOS_UINT32  AT_ProcTSelRfWDivPara(VOS_UINT8 ucIndex)
{
    if (DRV_AGENT_DSP_RF_SWITCH_ON != g_stAtDevCmdCtrl.ucRxOnOff)
    {
        g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_DIV_ON;
        return AT_OK;
    }
    if (AT_FAILURE == At_SendRfCfgAntSelToHPA(AT_RX_DIV_ON, ucIndex))
    {
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32  AT_ProcTSelRfWPriPara(VOS_UINT8 ucIndex)
{
    if (DRV_AGENT_DSP_RF_SWITCH_ON != g_stAtDevCmdCtrl.ucRxOnOff)
    {
        g_stAtDevCmdCtrl.ucPriOrDiv = AT_RX_PRI_ON;
        return AT_OK;
    }

    if (AT_FAILURE == At_SendRfCfgAntSelToHPA(AT_RX_PRI_ON, ucIndex))
    {
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    return AT_WAIT_ASYNC_RETURN;
}



VOS_UINT32 AT_SetTSelRfPara(VOS_UINT8 ucIndex)
{
    DRV_AGENT_TSELRF_SET_REQ_STRU       stTseLrf;
    VOS_BOOL                            bLoadDsp;

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数不符合要求 */
    if ((2 < gucAtParaIndex)
     || (0 == gastAtParaList[0].usParaLen))
    {
        return AT_ERROR;
    }

    if (AT_TSELRF_PATH_WIFI == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWifiPara();
    }

    if(AT_TSELRF_PATH_TD == gastAtParaList[0].ulParaValue)
    {
        return atSetTselrfPara(ucIndex);
    }
    

    if ((AT_TSELRF_PATH_WCDMA_PRI!=gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_WCDMA_DIV!=gastAtParaList[0].ulParaValue)
     && (AT_TSELRF_PATH_GSM !=gastAtParaList[0].ulParaValue))
    {
        return atSetTselrfPara(ucIndex);
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_ERROR;
    }

    /* 打开分集必须在FRXON之后，参考RXDIV实现 */
    if (AT_TSELRF_PATH_WCDMA_DIV == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWDivPara(ucIndex);
    }

    if (AT_TSELRF_PATH_WCDMA_PRI == gastAtParaList[0].ulParaValue)
    {
        return AT_ProcTSelRfWPriPara(ucIndex);
    }

    /* 此处判断是否需要重新加载DSP: 需要则发请求到C核加载DSP，否则，直接返回OK */
    AT_GetTseLrfLoadDspInfo(gastAtParaList[0].ulParaValue, &bLoadDsp, &stTseLrf);
    if (VOS_TRUE == bLoadDsp)
    {
        if (TAF_SUCCESS == AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                                   gastAtClientTab[ucIndex].opId,
                                                   DRV_AGENT_TSELRF_SET_REQ,
                                                   &stTseLrf,
                                                   sizeof(stTseLrf),
                                                   I0_WUEPS_PID_DRV_AGENT))
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_TSELRF_SET;             /*设置当前操作模式 */
            return AT_WAIT_ASYNC_RETURN;                                           /* 等待异步事件返回 */
        }
        else
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}


VOS_UINT32 At_SetFpaPara(VOS_UINT8 ucIndex)
{
    /* 调用LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return AT_CMD_NOT_SUPPORT;
    }
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FPA_OTHER_ERR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_FPA_OTHER_ERR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用,即^FCHAN成功执行后 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* 把设置保存在本地变量 */
    g_stAtDevCmdCtrl.ucPaLevel = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;

}


VOS_UINT32 At_SetFlnaPara(VOS_UINT8 ucIndex)
{

    /* 调用 LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atSetFLNAPara(ucIndex);
    }

    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atSetFLNAPara(ucIndex);
    }

    /* 命令状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FLNA_OTHER_ERR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_FLNA_OTHER_ERR;
    }

    /* 该命令需在非信令模式下使用 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 该命令需在设置非信令信道后使用 */
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /* 参数LNA等级取值范围检查 */
    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        /* WDSP LNA等级取值为0-2 */
        if (gastAtParaList[0].ulParaValue > DSP_LNA_HIGH_GAIN_MODE)
        {
            return AT_FLNA_OTHER_ERR;
        }
    }

    /* 把设置保存在本地变量 */
    g_stAtDevCmdCtrl.ucLnaLevel = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;
}



VOS_UINT32 At_SetDpdtPara(VOS_UINT8 ucIndex)
{
    AT_MTA_SET_DPDT_VALUE_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_SET_DPDT_VALUE_REQ_STRU));
    stAtCmd.enRatMode   = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;
    stAtCmd.ulDpdtValue = gastAtParaList[1].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_SET_DPDT_VALUE_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_SET_DPDT_VALUE_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }

}


VOS_UINT32 At_SetQryDpdtPara(VOS_UINT8 ucIndex)
{
    AT_MTA_QRY_DPDT_VALUE_REQ_STRU      stAtCmd;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    TAF_MEM_SET_S(&stAtCmd, sizeof(stAtCmd), 0x00, sizeof(AT_MTA_QRY_DPDT_VALUE_REQ_STRU));
    stAtCmd.enRatMode = (AT_MTA_CMD_RATMODE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    /* 发送消息给C核处理 */
    ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                   0,
                                   ID_AT_MTA_QRY_DPDT_VALUE_REQ,
                                   &stAtCmd,
                                   sizeof(AT_MTA_QRY_DPDT_VALUE_REQ_STRU),
                                   I0_UEPS_PID_MTA);

    if (AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DPDTQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
    else
    {
        return AT_ERROR;
    }
}

/*****************************************************************************
 函 数 名  : AT_SetDcxotempcompPara
 功能描述  : ^DCXOTEMPCOMP设置命令处理函数
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_SetDcxotempcompPara(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    g_stAtDevCmdCtrl.enDcxoTempCompEnableFlg = (AT_DCXOTEMPCOMP_ENABLE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

    return AT_OK;
}


VOS_UINT32  AT_SetFDac(VOS_UINT8 ucIndex )
{

    TAF_UINT16                           usDAC;

    /*调用 LTE 模的接口分支*/
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return AT_CMD_NOT_SUPPORT;
    }
    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 参数检查 */
    if(AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_FDAC_SET_FAIL;
    }
    /* 参数不符合要求 */
    if (gucAtParaIndex != 1)
    {
        return AT_FDAC_SET_FAIL;
    }
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    usDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        if (usDAC > WDSP_MAX_TX_AGC)
        {
            return AT_FDAC_SET_FAIL;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        }
    }
    else
    {
        if (usDAC > GDSP_MAX_TX_VPA)
        {
            return AT_FDAC_SET_FAIL;
        }
        else
        {
            g_stAtDevCmdCtrl.usFDAC = (VOS_UINT16)gastAtParaList[0].ulParaValue;
        }
    }

    /*AT^FDAC设置的FDAC值和AT^FWAVE设置的power值表示的含义相同，取后设置的值*/
    g_stAtDevCmdCtrl.bFdacFlag  = VOS_TRUE;
    g_stAtDevCmdCtrl.bPowerFlag = VOS_FALSE;

    return AT_OK;    /* 返回命令处理挂起状态 */
}



VOS_UINT32 AT_SetMipiWrPara(VOS_UINT8 ucIndex)
{
    AT_HPA_MIPI_WR_REQ_STRU             *pstMsg;
    VOS_UINT32                          ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (5 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    ulLength = sizeof(AT_HPA_MIPI_WR_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_MIPI_WR_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetMipiWrPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue)
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                         = ID_AT_HPA_MIPI_WR_REQ;
    pstMsg->usSlaveAddr                     = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usRegAddr                       = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usRegData                       = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;
    pstMsg->usMipiChannel                   = ( VOS_UINT16 )gastAtParaList[4].ulParaValue;

    /*GSM and UMTS share the same PID*/
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetMipiWrPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_MIPI_WR;                   /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetSSIWrPara(VOS_UINT8 ucIndex)
{
    AT_HPA_SSI_WR_REQ_STRU                  *pstMsg;
    VOS_UINT32                              ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if ( 4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    ulLength = sizeof(AT_HPA_SSI_WR_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_SSI_WR_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if ( VOS_NULL_PTR == pstMsg )
    {
        AT_WARN_LOG("AT_SetSSIWrPara: alloc msg fail!");
        return AT_ERROR;
    }

    if ( AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue )
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                         = ID_AT_HPA_SSI_WR_REQ;
    pstMsg->usRficSsi                       = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usRegAddr                       = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usData                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if ( VOS_OK != PS_SEND_MSG( WUEPS_PID_AT, pstMsg ) )
    {
        AT_WARN_LOG("AT_SetSSIWrPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_SSI_WR;                    /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetSSIRdPara(VOS_UINT8 ucIndex)
{
    AT_HPA_SSI_RD_REQ_STRU                  *pstMsg;
    VOS_UINT32                              ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (3 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if ( AT_RAT_MODE_FDD_LTE == gastAtParaList[0].ulParaValue
      || AT_RAT_MODE_TDD_LTE == gastAtParaList[0].ulParaValue
      || AT_RAT_MODE_TDSCDMA == gastAtParaList[0].ulParaValue )
    {
        /* 调用TL函数处理 */
        return AT_SetTlRficSsiRdPara( ucIndex );
    }

    ulLength = sizeof(AT_HPA_SSI_RD_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_SSI_RD_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetMipiRdPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue)
    {
        pstMsg->ulReceiverPid                   = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid                   = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                             = ID_AT_HPA_SSI_RD_REQ;
    pstMsg->usChannelNo                         = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->uwRficReg                           = gastAtParaList[2].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetMipiRdPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt      = AT_CMD_SSI_RD;                /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetMipiRdPara(VOS_UINT8 ucIndex)
{
    AT_HPA_MIPI_RD_REQ_STRU             *pstMsg;
    VOS_UINT32                          ulLength;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    if (AT_RAT_MODE_FDD_LTE == gastAtParaList[0].ulParaValue || AT_RAT_MODE_TDD_LTE == gastAtParaList[0].ulParaValue)
    {
        return AT_ERROR;
    }

    ulLength = sizeof(AT_HPA_MIPI_RD_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    pstMsg   = (AT_HPA_MIPI_RD_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetMipiRdPara: alloc msg fail!");
        return AT_ERROR;
    }

    if (AT_RAT_MODE_GSM == gastAtParaList[0].ulParaValue)
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else
    {
        pstMsg->ulReceiverPid               = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }

    pstMsg->usMsgID                         = ID_AT_HPA_MIPI_RD_REQ;
    pstMsg->uhwChannel                      = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->uhwSlaveAddr                    = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->uhwReg                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetMipiRdPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt  = AT_CMD_MIPI_RD;                   /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}


VOS_UINT32 AT_SetMipiReadPara(VOS_UINT8 ucIndex)
{
    AT_MTA_MIPI_READ_REQ_STRU           stMipiReadReq;
    VOS_UINT32                          ulResult;

    /*局部变量初始化*/
    TAF_MEM_SET_S(&stMipiReadReq, (VOS_SIZE_T)sizeof(stMipiReadReq), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_MIPI_READ_REQ_STRU));

    /*参数检查*/
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*参数数目不正确*/
    if (7 != gucAtParaIndex )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*不是非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*参数长度检查*/
    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen)
      || (0 == gastAtParaList[4].usParaLen)
      || (0 == gastAtParaList[5].usParaLen)
      || (0 == gastAtParaList[6].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*填写消息参数*/
    stMipiReadReq.usReadType    = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stMipiReadReq.usMipiId      = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stMipiReadReq.usSlaveId     = (VOS_UINT16)gastAtParaList[2].ulParaValue;
    stMipiReadReq.usRegAddr     = (VOS_UINT16)gastAtParaList[3].ulParaValue;
    stMipiReadReq.usSpeedType   = (VOS_UINT16)gastAtParaList[4].ulParaValue;
    stMipiReadReq.usReadBitMask = (VOS_UINT16)gastAtParaList[5].ulParaValue;
    stMipiReadReq.usReserved1   = (VOS_UINT16)gastAtParaList[6].ulParaValue;


    /*发送消息给MTA*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_MIPIREAD_SET_REQ,
                                      &stMipiReadReq,
                                      (VOS_SIZE_T)sizeof(stMipiReadReq),
                                      I0_UEPS_PID_MTA);
    /*发送失败*/
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetMipiReadPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /*发送成功，设置当前操作模式*/
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_MIPIREAD_SET;

    /*等待异步处理时间返回*/
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetPhyMipiWritePara(VOS_UINT8 ucIndex)
{
    AT_MTA_PHY_MIPI_WRITE_REQ_STRU      stPhyMipiWriteReq;
    VOS_UINT32                          ulResult;

    /*局部变量初始化*/
    TAF_MEM_SET_S(&stPhyMipiWriteReq, (VOS_SIZE_T)sizeof(stPhyMipiWriteReq), 0x00, (VOS_SIZE_T)sizeof(AT_MTA_PHY_MIPI_WRITE_REQ_STRU));

    /*参数检查*/
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*参数数目不正确*/
    if (6 != gucAtParaIndex )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*不是非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*参数长度检查*/
    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen)
      || (0 == gastAtParaList[4].usParaLen)
      || (0 == gastAtParaList[5].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*填写消息参数*/
    stPhyMipiWriteReq.usWriteType   = (VOS_UINT16)gastAtParaList[0].ulParaValue;
    stPhyMipiWriteReq.usMipiId      = (VOS_UINT16)gastAtParaList[1].ulParaValue;
    stPhyMipiWriteReq.usSlaveId     = (VOS_UINT16)gastAtParaList[2].ulParaValue;
    stPhyMipiWriteReq.usRegAddr     = (VOS_UINT16)gastAtParaList[3].ulParaValue;
    stPhyMipiWriteReq.usMipiData    = (VOS_UINT16)gastAtParaList[4].ulParaValue;
    stPhyMipiWriteReq.usReserved1   = (VOS_UINT16)gastAtParaList[5].ulParaValue;

    /*发送消息给MTA*/
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_PHYMIPIWRITE_SET_REQ,
                                      &stPhyMipiWriteReq,
                                      (VOS_SIZE_T)sizeof(stPhyMipiWriteReq),
                                      I0_UEPS_PID_MTA);
    /*发送失败*/
    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetPhyMipiWritePara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /*发送成功，设置当前操作模式*/
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_PHYMIPIWRITE_SET;

    /*等待异步处理时间返回*/
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32  At_SetCltPara(VOS_UINT8 ucIndex)
{
    /* 状态检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不符合要求 */
    if (1 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    g_stAtDevCmdCtrl.enCltEnableFlg = (AT_DSP_CLT_ENABLE_ENUM_UINT8) gastAtParaList[0].ulParaValue;

    return AT_OK;    /* 返回命令处理处理成功 */

}


VOS_UINT32 AT_SetPdmCtrlPara(VOS_UINT8 ucIndex)
{
    AT_HPA_PDM_CTRL_REQ_STRU                *pstMsg;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if ( 4 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }
    /*lint -save -e516 */
    pstMsg   = (AT_HPA_PDM_CTRL_REQ_STRU *)AT_ALLOC_MSG_WITH_HDR( sizeof(AT_HPA_PDM_CTRL_REQ_STRU) );
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_SetPdmCtrlPara: alloc msg fail!");
        return AT_ERROR;
    }

    /* 填写消息头 */
    AT_CFG_MSG_HDR( pstMsg, DSP_PID_WPHY, ID_AT_HPA_PDM_CTRL_REQ );

    pstMsg->usMsgID                             = ID_AT_HPA_PDM_CTRL_REQ;
    pstMsg->usRsv                               = 0;
    pstMsg->usPdmRegValue                       = ( VOS_UINT16 )gastAtParaList[0].ulParaValue;
    pstMsg->usPaVbias                           = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    pstMsg->usPaVbias2                          = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;
    pstMsg->usPaVbias3                          = ( VOS_UINT16 )gastAtParaList[3].ulParaValue;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_SetPdmCtrlPara: Send msg fail!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt      = AT_CMD_PDM_CTRL;                /*设置当前操作模式 */
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;                                                /* 等待异步事件返回 */
}

/*****************************************************************************
  4 查询函数实现
*****************************************************************************/


VOS_UINT32  At_QryFChanPara(VOS_UINT8 ucIndex )
{
    TAF_UINT16                 usLength;

    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFCHANPara(ucIndex);
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前FCHAN的设置 */
    usLength =  (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%d,%d,%d,%d",
                                   g_stAtDevCmdCtrl.ucDeviceRatMode,
                                   g_stAtDevCmdCtrl.ucDeviceAtBand,
                                   g_stAtDevCmdCtrl.stDspBandArfcn.usUlArfcn,
                                   g_stAtDevCmdCtrl.stDspBandArfcn.usDlArfcn);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFTxonPara(VOS_UINT8 ucIndex )
{
    TAF_UINT16                 usLength;

    /*添加 LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFTXONPara(ucIndex);
    }

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }
    /* 查询当前DAC的设置 */
    usLength =  (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%d",
                                   g_stAtDevCmdCtrl.ucTxOnOff);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFRxonPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    /* 添加LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFRXONPara(ucIndex);
    }

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前接收机开关状态 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stAtDevCmdCtrl.ucRxOnOff);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 AT_QryFpowdetTPara(VOS_UINT8 ucIndex)
{
    AT_PHY_POWER_DET_REQ_STRU          *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucIsLteFlg;

    ucIsLteFlg = VOS_FALSE;

    /*判断当前接入模式，只支持W*/
    if ( (AT_RAT_MODE_WCDMA != g_stAtDevCmdCtrl.ucDeviceRatMode)
      && (AT_RAT_MODE_CDMA != g_stAtDevCmdCtrl.ucDeviceRatMode)
      && (AT_RAT_MODE_GSM != g_stAtDevCmdCtrl.ucDeviceRatMode)
      && (AT_RAT_MODE_FDD_LTE != g_stAtDevCmdCtrl.ucDeviceRatMode)
      && (AT_RAT_MODE_TDD_LTE != g_stAtDevCmdCtrl.ucDeviceRatMode) )
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 申请AT_PHY_POWER_DET_REQ_STRU消息 */
    ulLength = sizeof(AT_PHY_POWER_DET_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    pstMsg   = (AT_PHY_POWER_DET_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_QryFpowdetTPara: Alloc msg fail!");
        return AT_ERROR;
    }

    /* CDMA的话，发送给UPHY_PID_CSDR_1X_CM */
    if (AT_RAT_MODE_CDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        pstMsg->ulReceiverPid = UPHY_PID_CSDR_1X_CM;
    }
    else
    if (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
    }
    else if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
    }
    else
    {
        ucIsLteFlg = VOS_TRUE;
    }

    if (VOS_FALSE == ucIsLteFlg)
    {
        pstMsg->usMsgID       = ID_AT_PHY_POWER_DET_REQ;
        pstMsg->usRsv         = 0;

        /* 向对应PHY发送消息 */
        ulResult = PS_SEND_MSG(WUEPS_PID_AT, pstMsg);
    }
    else
    {
        /*lint --e{516,830} */
        PS_FREE_MSG(WUEPS_PID_AT, pstMsg);
        ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                          gastAtClientTab[ucIndex].opId,
                                          ID_AT_MTA_POWER_DET_QRY_REQ,
                                          VOS_NULL_PTR,
                                          0,
                                          I0_UEPS_PID_MTA);
    }

    if (VOS_OK != ulResult)
    {
        AT_WARN_LOG("AT_QryFpowdetTPara: Send msg fail!");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPOWDET_QRY;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryFPllStatusPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulReceiverPid;
    AT_PHY_RF_PLL_STATUS_REQ_STRU      *pstMsg;
    VOS_UINT32                          ulLength;
    VOS_UINT16                          usMsgId;
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
            ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFPllStatusPara(ucIndex);
    }
    /*判断当前接入模式，只支持G/W*/
    if (AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
        usMsgId       = ID_AT_WPHY_RF_PLL_STATUS_REQ;
    }
    else if ( (AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
            ||(AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode) )
    {
        ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
        usMsgId       = ID_AT_GPHY_RF_PLL_STATUS_REQ;
    }

    else
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 申请AT_PHY_RF_PLL_STATUS_REQ_STRU消息 */
    ulLength = sizeof(AT_PHY_RF_PLL_STATUS_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e516 */
    pstMsg   = (AT_PHY_RF_PLL_STATUS_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("AT_QryFPllStatusPara: Alloc msg fail!");
        return AT_ERROR;
    }

    /* 填充消息 */
    pstMsg->ulReceiverPid = ulReceiverPid;
    pstMsg->usMsgID       = usMsgId;
    pstMsg->usRsv1        = 0;
    pstMsg->usDspBand     = g_stAtDevCmdCtrl.stDspBandArfcn.usDspBand;
    pstMsg->usRsv2        = 0;

    /* 向对应PHY发送消息 */
    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("AT_QryFPllStatusPara: Send msg fail!");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_FPLLSTATUS_QRY;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryFrssiPara(
    VOS_UINT8                           ucIndex
)
{
    AT_HPA_RF_RX_RSSI_REQ_STRU          *pstMsg;
    VOS_UINT32                          ulLength;

    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFRSSIPara(ucIndex);
    }

    if(AT_RAT_MODE_TDSCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
    {
        return atQryFRSSIPara(ucIndex);
    }


    /*该命令需在非信令模式下使用*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /*该命令需在设置非信令信道后使用*/
    if (VOS_FALSE == g_stAtDevCmdCtrl.bDspLoadFlag)
    {
        return AT_CHANNEL_NOT_SET;
    }

    /*该命令需要在打开接收机后使用*/
    if (AT_DSP_RF_SWITCH_OFF == g_stAtDevCmdCtrl.ucRxOnOff)
    {
        return AT_FRSSI_RX_NOT_OPEN;
    }

    /* GDSP LOAD的情况下不支持接收机和发射机同时打开，需要判断最近一次执行的是打开接收机操作
    还是打开发射机操作，如果是打开发射机操作，则直接返回出错无需和GDSP 交互 */
    if ((AT_TXON_OPEN == g_stAtDevCmdCtrl.ucRxonOrTxon)
     && ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
      || (AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode)))
    {
        return AT_FRSSI_OTHER_ERR;
    }

    /* 申请AT_HPA_RF_RX_RSSI_REQ_STRU消息 */
    ulLength = sizeof(AT_HPA_RF_RX_RSSI_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    /*lint -save -e830 */
    pstMsg   = (AT_HPA_RF_RX_RSSI_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, ulLength);
    /*lint -restore */
    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_QryFrssiPara: alloc msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    if ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_EDGE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_GPHY);
        pstMsg->usMsgID       = ID_AT_GHPA_RF_RX_RSSI_REQ;
    }
    else
    {
        pstMsg->ulReceiverPid = AT_GetDestPid(ucIndex, I0_DSP_PID_WPHY);
        pstMsg->usMsgID       = ID_AT_HPA_RF_RX_RSSI_REQ;
    }

    pstMsg->usMeasNum  = AT_DSP_RSSI_MEASURE_NUM;
    pstMsg->usInterval = AT_DSP_RSSI_MEASURE_INTERVAL;
    pstMsg->usRsv      = 0;

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_QryFrssiPara: Send msg fail!");
        return AT_FRSSI_OTHER_ERR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_QUERY_RSSI;
    g_stAtDevCmdCtrl.ucIndex               = ucIndex;

    return AT_WAIT_ASYNC_RETURN;    /* 返回命令处理挂起状态 */

}


VOS_UINT32 AT_QryTSelRfPara(VOS_UINT8 ucIndex)
{

    return atQryTselrfPara(ucIndex);

}


VOS_UINT32  At_QryFpaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前发射机PA等级的设置 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stAtDevCmdCtrl.ucPaLevel);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32  At_QryFlnaPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    /* 添加 LTE 模的接口分支 */
    if ((AT_RAT_MODE_FDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode)
      ||(AT_RAT_MODE_TDD_LTE == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        return atQryFLNAPara(ucIndex);
    }

    /*当前不为非信令模式*/
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前发射机PA等级的设置 */
    usLength =  (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, "%s:%d",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       g_stAtDevCmdCtrl.ucLnaLevel);

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}

/*****************************************************************************
 函 数 名  : AT_QryDcxotempcompPara
 功能描述  : ^DCXOTEMPCOMP的查询命令处理函数
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32  AT_QryDcxotempcompPara(VOS_UINT8 ucIndex)
{

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /*  不是非信令模式下发返回错误 */
    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    g_stAtDevCmdCtrl.enDcxoTempCompEnableFlg);

    return AT_OK;
}


VOS_UINT32  AT_QryFDac(VOS_UINT8 ucIndex )
{
    TAF_UINT16                 usLength;

    if (AT_TMODE_FTM != g_stAtDevCmdCtrl.ucCurrentTMode)
    {
        return AT_DEVICE_MODE_ERROR;
    }

    /* 查询当前DAC的设置 */
    usLength =  (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                       (TAF_CHAR*)pgucAtSndCodeAddr, "%s:",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,(TAF_CHAR *)pgucAtSndCodeAddr,
                                  (TAF_CHAR *)pgucAtSndCodeAddr + usLength, "%d",
                                   g_stAtDevCmdCtrl.usFDAC);
    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32  At_QryCltInfo(VOS_UINT8 ucIndex)
{
    VOS_UINT16                 usLength;

    usLength = 0;

    /* 如果记录有效，则上报查询结果 */
    if (VOS_TRUE == g_stAtDevCmdCtrl.stCltInfo.ulInfoAvailableFlg)
    {

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                            "%s%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                            "^CLTINFO: ",
                                            g_stAtDevCmdCtrl.stCltInfo.shwGammaReal,          /* 反射系数实部 */
                                            g_stAtDevCmdCtrl.stCltInfo.shwGammaImag,          /* 反射系数虚部 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc0,       /* 驻波检测场景0反射系数幅度 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc1,       /* 驻波检测场景1反射系数幅度 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAmpUc2,       /* 驻波检测场景2反射系数幅度 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwGammaAntCoarseTune,/* 粗调格点位置 */
                                            g_stAtDevCmdCtrl.stCltInfo.ulwFomcoarseTune,      /* 粗调FOM值 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwCltAlgState,       /* 闭环算法收敛状态 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwCltDetectCount,    /* 闭环收敛总步数 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac0,              /* DAC0 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac1,              /* DAC1 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac2,              /* DAC2 */
                                            g_stAtDevCmdCtrl.stCltInfo.ushwDac3);             /* DAC3 */

        /* 上报后本地记录就无效 */
        g_stAtDevCmdCtrl.stCltInfo.ulInfoAvailableFlg = VOS_FALSE;
    }

    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


VOS_UINT32 At_TestFdacPara(VOS_UINT8 ucIndex)
{
    VOS_UINT16                          usLength;

    usLength  = 0;

    if ((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
     || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: (0-2047)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    else
    {
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                          (VOS_CHAR *)pgucAtSndCodeAddr,
                                          (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                          "%s: (0-1023)",
                                          g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    }
    gstAtSendData.usBufLen = usLength;

    return AT_OK;

}


/*****************************************************************************
  5 其它旧函数实现
*****************************************************************************/


VOS_UINT8 At_GetDspLoadMode(VOS_UINT32 ulRatMode)
{
    if ((AT_RAT_MODE_WCDMA == ulRatMode)
     || (AT_RAT_MODE_AWS == ulRatMode))
    {
        return VOS_RATMODE_WCDMA;
    }
    else if ((AT_RAT_MODE_GSM == ulRatMode)
          || (AT_RAT_MODE_EDGE == ulRatMode))
    {
        return VOS_RATMODE_GSM;
    }
    else if(AT_RAT_MODE_CDMA == ulRatMode)
    {
        return VOS_RATMODE_1X;
    }
    else
    {
        return VOS_RATMODE_BUTT;
    }

}


VOS_VOID AT_GetTseLrfLoadDspInfo(
    AT_TSELRF_PATH_ENUM_UINT32          enPath,
    VOS_BOOL                           *pbLoadDsp,
    DRV_AGENT_TSELRF_SET_REQ_STRU      *pstTseLrf
)
{
    /* ^TSELRF命令设置的射频通路编号为GSM且当前已经LOAD了该通路，无需LOAD */
    if (AT_TSELRF_PATH_GSM == enPath)
    {
        if ((AT_RAT_MODE_GSM == g_stAtDevCmdCtrl.ucDeviceRatMode)
         && (VOS_TRUE == g_stAtDevCmdCtrl.bDspLoadFlag))
        {
            *pbLoadDsp = VOS_FALSE;
        }
        else
        {
            pstTseLrf->ucLoadDspMode     = VOS_RATMODE_GSM;
            pstTseLrf->ucDeviceRatMode   = AT_RAT_MODE_GSM;
            *pbLoadDsp                   = VOS_TRUE;
        }
        return;
    }

    /* ^TSELRF命令设置的射频通路编号为WCDMA主集且当前已经LOAD了该通路，无需LOAD */
    if (AT_TSELRF_PATH_WCDMA_PRI == enPath)
    {
        if (((AT_RAT_MODE_WCDMA == g_stAtDevCmdCtrl.ucDeviceRatMode)
          || (AT_RAT_MODE_AWS == g_stAtDevCmdCtrl.ucDeviceRatMode))
         && (VOS_TRUE == g_stAtDevCmdCtrl.bDspLoadFlag))
        {
            *pbLoadDsp = VOS_FALSE;
        }
        else
        {
            pstTseLrf->ucLoadDspMode     = VOS_RATMODE_WCDMA;
            pstTseLrf->ucDeviceRatMode   = AT_RAT_MODE_WCDMA;
            *pbLoadDsp                   = VOS_TRUE;
        }
        return;
    }

    *pbLoadDsp = VOS_FALSE;

    AT_WARN_LOG("AT_GetTseLrfLoadDspInfo: enPath only support GSM or WCDMA primary.");

    return;
}


VOS_UINT32 AT_SetGlobalFchan(VOS_UINT8 ucRATMode)
{
    g_stAtDevCmdCtrl.ucDeviceRatMode = ucRATMode;

    return VOS_OK;
}


VOS_UINT32 AT_SetTlRficSsiRdPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ulResult;
    AT_MTA_RFICSSIRD_REQ_STRU           stRficSsiRdReq;

    /* 通道检查 */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    {
        return AT_ERROR;
    }

    /* 初始化 */
    TAF_MEM_SET_S(&stRficSsiRdReq, sizeof(stRficSsiRdReq), 0x00, sizeof(stRficSsiRdReq));

    stRficSsiRdReq.usChannelNo            = ( VOS_UINT16 )gastAtParaList[1].ulParaValue;
    stRficSsiRdReq.usRficReg              = ( VOS_UINT16 )gastAtParaList[2].ulParaValue;

    /* 发送消息 */
    ulResult = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                      gastAtClientTab[ucIndex].opId,
                                      ID_AT_MTA_RFICSSIRD_QRY_REQ,
                                      &stRficSsiRdReq,
                                      sizeof(stRficSsiRdReq),
                                      I0_UEPS_PID_MTA);

    if (TAF_SUCCESS != ulResult)
    {
        AT_WARN_LOG("AT_SetTlRficSsiRdPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_RFICSSIRD_SET;
    g_stAtDevCmdCtrl.ucIndex                    = ucIndex;

    return AT_WAIT_ASYNC_RETURN;
}




