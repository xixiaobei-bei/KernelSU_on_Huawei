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
#include "AtCmdPacketProc.h"
#include "AtTafAgentInterface.h"
#include "TafIfaceApi.h"

#include "product_config.h"


/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_AT_CMD_PACKET_PROC_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/


/*****************************************************************************
  3 函数实现
*****************************************************************************/


AT_APP_CONN_STATE_ENUM_U32 AT_AppConvertPdpStateToConnStatus(
    TAF_IFACE_STATE_ENUM_U8                enPdpState
)
{
    AT_APP_CONN_STATE_ENUM_U32          enConnStatus;

    switch (enPdpState)
    {
        case TAF_IFACE_STATE_ACTING:
            enConnStatus = AT_APP_DIALING;
            break;

        case TAF_IFACE_STATE_ACTED:
        case TAF_IFACE_STATE_DEACTING:
            enConnStatus = AT_APP_DIALED;
            break;

        case TAF_IFACE_STATE_IDLE:
        default:
            enConnStatus = AT_APP_UNDIALED;
            break;
    }

    return enConnStatus;
}


TAF_IFACE_DYNAMIC_INFO_STRU *AT_GetDynamicInfoBaseAddr(
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU     *pstEvt
)
{
    return &(pstEvt->astDynamicInfo[0]);
}


VOS_UINT32 AT_GetCidByCidMask(
    VOS_UINT32                          ulCidMask,
    VOS_UINT8                          *pucCid
)
{
    VOS_UINT32                          ulCid;

    for (ulCid = 1; ulCid <= TAF_MAX_CID_NV; ulCid++)
    {
        /* 当前Bit位为1，则此位为对应的Cid */
        if (0 != (ulCidMask & ((VOS_UINT32)0x00000001 << ulCid)))
        {
            *pucCid = (VOS_UINT8)ulCid;
            return VOS_OK;
        }
    }

    *pucCid = TAF_INVALID_CID;

    return VOS_ERR;
}


VOS_UINT32 AT_SetChdataPara_AppUser(VOS_UINT8 ucIndex)
{

    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx   = VOS_NULL_PTR;
    CONST AT_CHDATA_RNIC_RMNET_ID_STRU *pstChDataCfg    = VOS_NULL_PTR;
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId;
    VOS_UINT8                           ucLoop;
    VOS_UINT8                           ucCid;

    ucCid = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(ucIndex);

    /* 指定CID的PDP若已经激活，则不允许删除或修改该CID的通道映射关系，直接返回ERROR */
    if ((VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulUsed)
      && (VOS_TRUE == pstPsModemCtx->astChannelCfg[ucCid].ulIfaceActFlg))
    {
        return AT_ERROR;
    }

    /* 第二个参数为空，则表示删除配置关系 */
    if (0 == gastAtParaList[1].usParaLen)
    {
        pstPsModemCtx->astChannelCfg[ucCid].ulUsed     = VOS_FALSE;
        pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId  = AT_PS_INVALID_RMNET_ID;
        return AT_OK;
    }

    enDataChannelId = gastAtParaList[1].ulParaValue;

    /* 获取网卡ID */
    pstChDataCfg = AT_PS_GetChDataCfgByChannelId(ucIndex,enDataChannelId);
    if (VOS_NULL_PTR == pstChDataCfg)
    {
        return AT_ERROR;
    }

    /* 查找是否有不同的<CID>配置了相同的<enRnicRmNetId> */
    for (ucLoop = 1; ucLoop <= TAF_MAX_CID; ucLoop++)
    {
        if ( (ucLoop != ucCid)
          && (VOS_TRUE == pstPsModemCtx->astChannelCfg[ucLoop].ulUsed)
          && (pstChDataCfg->enRnicRmNetId == (RNIC_DEV_ID_ENUM_UINT8)pstPsModemCtx->astChannelCfg[ucLoop].ulRmNetId))
        {
            /* 不同的<CID>配置了相同的<enRnicRmNetId>，直接返回ERROR */
            return AT_ERROR;
        }
    }

    /* 配置数传通道映射表 */
    pstPsModemCtx->astChannelCfg[ucCid].ulUsed    = VOS_TRUE;
    pstPsModemCtx->astChannelCfg[ucCid].ulRmNetId = pstChDataCfg->enRnicRmNetId;
    pstPsModemCtx->astChannelCfg[ucCid].ulIfaceId = pstChDataCfg->enIfaceId;

    return AT_OK;
}


VOS_UINT32 AT_SetChdataPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucUserIndex;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数为空 */
    if ((0 == gastAtParaList[0].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ucUserIndex = ucIndex;

    /* PCUI口且已设置PCUI口模拟NDISDUP拨号 */
    if (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)
    {
        if (VOS_TRUE == AT_GetPcuiPsCallFlag())
        {
            ucUserIndex = AT_GetPcuiUsertId();
        }
    }

    /* CTRL口且已设置CTRL口模拟NDISDUP拨号 */
    if (AT_CTR_USER == gastAtClientTab[ucIndex].UserType)
    {
        if (VOS_TRUE == AT_GetCtrlPsCallFlag())
        {
            ucUserIndex = AT_GetCtrlUserId();
        }
    }

    /* PCUI2口且已设置PCUI2口模拟NDISDUP拨号 */
    if (AT_PCUI2_USER == gastAtClientTab[ucIndex].UserType)
    {
        if (VOS_TRUE == AT_GetPcui2PsCallFlag())
        {
            ucUserIndex = AT_GetPcui2UserId();
        }
    }

    /* APP通道的处理 */
    if (VOS_TRUE == AT_CheckAppUser(ucUserIndex))
    {
        return AT_SetChdataPara_AppUser(ucUserIndex);
    }

    return AT_ERROR;
}


VOS_UINT32 AT_QryChdataPara_AppUser(TAF_UINT8 ucIndex)
{
    VOS_UINT8                           ucLoop;
    VOS_UINT8                           ucNum;
    VOS_UINT16                          usLength;
    AT_CH_DATA_CHANNEL_ENUM_UINT32      enDataChannelId;
    AT_MODEM_PS_CTX_STRU               *pstPsModemCtx = VOS_NULL_PTR;
    VOS_UINT32                          ulRslt;

    ucNum    = 0;
    usLength = 0;

    pstPsModemCtx = AT_GetModemPsCtxAddrFromClientId(ucIndex);

    /* 输出结果 */
    for (ucLoop = 1; ucLoop <= TAF_MAX_CID; ucLoop++)
    {
        if ( (VOS_TRUE == pstPsModemCtx->astChannelCfg[ucLoop].ulUsed)
          && (AT_PS_INVALID_RMNET_ID != pstPsModemCtx->astChannelCfg[ucLoop].ulRmNetId) )
        {
            if (0 != ucNum)
            {
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%s",gaucAtCrLf);
            }

            /* ^CHDATA:  */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"^CHDATA: ");

            /* <cid> */
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,"%d", ucLoop);

            /* <datachannel> */
            ulRslt = AT_PS_GetChDataValueFromRnicRmNetId((RNIC_DEV_ID_ENUM_UINT8)(pstPsModemCtx->astChannelCfg[ucLoop].ulRmNetId), &enDataChannelId);

            if (VOS_OK != ulRslt)
            {
                return AT_ERROR;
            }

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR*)pgucAtSndCodeAddr,(VOS_CHAR*)pgucAtSndCodeAddr + usLength,",%d", enDataChannelId);

            ucNum++;
        }
    }

    gstAtSendData.usBufLen = usLength;

    return AT_OK;
}


VOS_UINT32 AT_QryChdataPara(TAF_UINT8 ucIndex)
{
    VOS_UINT8                           ucUserId;

    /*命令状态类型检查*/
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    ucUserId = ucIndex;

    /* PCUI口且已设置PCUI口模拟NDISDUP拨号 */
    if (AT_USBCOM_USER == gastAtClientTab[ucIndex].UserType)
    {
        if (VOS_TRUE == AT_GetPcuiPsCallFlag())
        {
            ucUserId = AT_GetPcuiUsertId();
        }
    }

    /* CTRL口且已设置CTRL口模拟NDISDUP拨号 */
    if (AT_CTR_USER == gastAtClientTab[ucIndex].UserType)
    {
        if (VOS_TRUE == AT_GetCtrlPsCallFlag())
        {
            ucUserId = AT_GetCtrlUserId();
        }
    }

    /* PCUI2口且已设置PCUI2口模拟NDISDUP拨号 */
    if (AT_PCUI2_USER == gastAtClientTab[ucIndex].UserType)
    {
        if (VOS_TRUE == AT_GetPcui2PsCallFlag())
        {
            ucUserId = AT_GetPcui2UserId();
        }
    }

    /* APP通道的处理 */
    if (VOS_TRUE == AT_CheckAppUser(ucUserId))
    {
        return AT_QryChdataPara_AppUser(ucUserId);
    }

    return AT_ERROR;
}


VOS_UINT32 AT_TestChdataPara(VOS_UINT8 ucIndex)
{
    /* 通道检查 */
    if (VOS_FALSE == AT_IsApPort(ucIndex))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: %s",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                       CHDATA_TEST_CMD_PARA_STRING);

    return AT_OK;
}


VOS_UINT32 At_SetIfaceDynamicParaComCheck(
    const VOS_UINT8                     ucIndex,
    TAF_CTRL_STRU                      *pstCtrl
)
{
    /* 参数检查 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 构造控制结构体 */
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, pstCtrl) == VOS_ERR)
    {
        return AT_ERROR;
    }

    return VOS_OK;
}


VOS_UINT32 At_QryIfaceDynamicParaComCheck(
    const VOS_UINT8                     ucIndex,
    TAF_CTRL_STRU                      *pstCtrl
)
{
    /* 参数检查 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_READ_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 构造控制结构体 */
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, pstCtrl) == VOS_ERR)
    {
        return AT_ERROR;
    }

    return VOS_OK;
}



VOS_UINT32 At_SetDhcpPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    
    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("At_SetDhcpPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DHCP_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryDhcpPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("At_QryDhcpPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DHCP_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestDhcpPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-11)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetDhcpv6Para(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    
    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetDhcpv6Para: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DHCPV6_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryDhcpv6Para(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_QryDhcpv6Para: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DHCPV6_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestDhcpv6Para(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-11)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetApRaInfoPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    
    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetApRaInfoPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APRAINFO_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryApRaInfoPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_QryApRaInfoPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APRAINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestApRaInfoPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-11)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetApLanAddrPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    /* 构造控制结构体 */
    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    
    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetApLanAddrPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APLANADDR_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryApLanAddrPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_QryApLanAddrPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APLANADDR_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestApLanAddrPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-11)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetIPv6TempAddrPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    /* 构造控制结构体 */
    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    
    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    if (AT_GetPsIPv6IIDTestModeConfig() != IPV6_ADDRESS_TEST_MODE_ENABLE)
    {
        AT_ERR_LOG("AT_PS_ReportIPv6TempAddr: Test mode is disabled!");
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    /* BitCid 构造*/
    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetIPv6TempAddrPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_IPV6TEMPADDR_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestIPv6TempAddrPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: (1-11)",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetCgpiafPara(VOS_UINT8 ucIndex)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;

    pstCommPsCtx    = AT_GetCommPsCtxAddr();

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (4 < gucAtParaIndex)
    {
        return AT_TOO_MANY_PARA;
    }

    /* 参数<IPv6_AddressFormat> */
    if (0 != gastAtParaList[0].usParaLen)
    {
        pstCommPsCtx->bitOpIpv6AddrFormat = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    }

    /* 参数<IPv6_SubnetNotation> */
    if (0 != gastAtParaList[1].usParaLen)
    {
        pstCommPsCtx->bitOpIpv6SubnetNotation = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    }

    /* 参数<IPv6_LeadingZeros> */
    if (0 != gastAtParaList[2].usParaLen)
    {
        pstCommPsCtx->bitOpIpv6LeadingZeros = (VOS_UINT8)gastAtParaList[2].ulParaValue;
    }

    /* 参数<IPv6_CompressZeros> */
    if (0 != gastAtParaList[2].usParaLen)
    {
        pstCommPsCtx->bitOpIpv6CompressZeros = (VOS_UINT8)gastAtParaList[3].ulParaValue;
    }

    return AT_OK;
}


VOS_UINT32 AT_QryCgpiafPara(VOS_UINT8 ucIndex)
{
    AT_COMM_PS_CTX_STRU                *pstCommPsCtx        = VOS_NULL_PTR;

    pstCommPsCtx    = AT_GetCommPsCtxAddr();

    /* 命令操作类型检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 输出查询结果 */
    /* +CGPIAF: <IPv6_AddressFormat>,<IPv6_SubnetNotation>,<IPv6_LeadingZeros>,<IPv6_CompressZeros> */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d,%d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      pstCommPsCtx->bitOpIpv6AddrFormat,
                                      pstCommPsCtx->bitOpIpv6SubnetNotation,
                                      pstCommPsCtx->bitOpIpv6LeadingZeros,
                                      pstCommPsCtx->bitOpIpv6CompressZeros);

    return AT_OK;
}


VOS_UINT32 At_SetApConnStPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    /* 构造控制结构体 */
    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("At_SetApConnStPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APCONNST_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_QryApConnStPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* BitCid 构造*/
    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("At_SetApConnStPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APCONNST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestApConnStPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (1-11)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetApDsFlowRptCfgPara(VOS_UINT8 ucIndex)
{
    TAF_APDSFLOW_RPT_CFG_STRU           stRptCfg;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 命令参数个数检查 */
    if ((gucAtParaIndex < 1) || (gucAtParaIndex > 4))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT^APDSFLOWRPTCFG=, */
    if (0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<enable> */
    stRptCfg.ulRptEnabled = gastAtParaList[0].ulParaValue;

    if (VOS_TRUE == stRptCfg.ulRptEnabled)
    {
        /* AT^APDSFLOWRPTCFG=1 */
        if (1 == gucAtParaIndex)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* AT^APDSFLOWRPTCFG=1, */
        if (0 == gastAtParaList[1].usParaLen)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* AT^APDSFLOWRPTCFG=1,<threshold> */
        stRptCfg.ulFluxThreshold = gastAtParaList[1].ulParaValue;

        /* AT^APDSFLOWRPTCFG=1,0 */
        if (0 == stRptCfg.ulFluxThreshold)
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        /* AT^APDSFLOWRPTCFG=0 */
        stRptCfg.ulFluxThreshold = 0;
    }

    /* 执行命令操作 */
    if (VOS_OK != TAF_PS_SetApDsFlowRptCfg(WUEPS_PID_AT,
                                           AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                           0,
                                           &stRptCfg))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APDSFLOWRPTCFG_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryApDsFlowRptCfgPara(VOS_UINT8 ucIndex)
{
    /* 执行命令操作 */
    if (VOS_OK != TAF_PS_GetApDsFlowRptCfg(WUEPS_PID_AT,
                                           AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                           0))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APDSFLOWRPTCFG_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetDsFlowNvWriteCfgPara(VOS_UINT8 ucIndex)
{
    TAF_DSFLOW_NV_WRITE_CFG_STRU        stWriteNvCfg;

    TAF_MEM_SET_S(&stWriteNvCfg, sizeof(stWriteNvCfg), 0x00, sizeof(TAF_DSFLOW_NV_WRITE_CFG_STRU));

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 命令参数个数检查 */
    if (gucAtParaIndex > 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT^DSFLOWNVWRCFG= */
    if (0 == gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT^DSFLOWNVWRCFG=,<interval> */
    if (0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取<enable> */
    stWriteNvCfg.ucEnabled          = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 获取<interval> */
    if (gucAtParaIndex > 1)
    {
        if (0 != gastAtParaList[1].usParaLen)
        {
            /* AT^DSFLOWNVWRCFG=<enable>,<interval> */
            stWriteNvCfg.ucInterval = (VOS_UINT8)gastAtParaList[1].ulParaValue;
        }
        else
        {
            /* AT^DSFLOWNVWRCFG=<enable>, */
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    else
    {
        /* AT^DSFLOWNVWRCFG=<enable> */
        stWriteNvCfg.ucInterval     = TAF_DEFAULT_DSFLOW_NV_WR_INTERVAL;
    }

    /* AT^DSFLOWNVWRCFG=1,0 */
    if ((VOS_TRUE == stWriteNvCfg.ucEnabled) && (0 == stWriteNvCfg.ucInterval))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (VOS_OK != TAF_PS_SetDsFlowNvWriteCfg(WUEPS_PID_AT,
                                             AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                             0,
                                             &stWriteNvCfg))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DSFLOWNVWRCFG_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryDsFlowNvWriteCfgPara(VOS_UINT8 ucIndex)
{
    /* 执行命令操作 */
    if (VOS_OK != TAF_PS_GetDsFlowNvWriteCfg(WUEPS_PID_AT,
                                             AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                             0))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DSFLOWNVWRCFG_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetImsPdpCfg(VOS_UINT8 ucIndex)
{
    TAF_IMS_PDP_CFG_STRU                stImsPdpCfg;

    /* 参数过多 */
    if (gucAtParaIndex != 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    TAF_MEM_SET_S(&stImsPdpCfg, sizeof(stImsPdpCfg), 0x00, sizeof(stImsPdpCfg));

    /* 参数赋值 */
    stImsPdpCfg.ucCid           = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stImsPdpCfg.ucImsFlag       = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* 发送跨核消息 */
    if ( VOS_OK != TAF_PS_SetImsPdpCfg(WUEPS_PID_AT,
                                       AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                       0,
                                       &stImsPdpCfg) )
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_IMSPDPCFG_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetMipPara(VOS_UINT8 ucIndex)
{

    /* 参数错误 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (0 == gastAtParaList[0].usParaLen)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息 */
    if ( VOS_OK != TAF_PS_SetMipMode(WUEPS_PID_AT,
                                     AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                     0,
                                     (VOS_UINT8)gastAtParaList[0].ulParaValue) )
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMIP_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryMipPara(VOS_UINT8 ucIndex)
{

    /* 发送跨核消息 */
    if ( VOS_OK != TAF_PS_GetMipMode(WUEPS_PID_AT,
                                     AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                     0) )
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CMIP_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetVzwApnePara(VOS_UINT8 ucIndex)
{
    TAF_PS_VZWAPNE_INFO_STRU            stVzwapneInfo;
    VOS_UINT8                           ucIsCustomCmd;

    TAF_MEM_SET_S(&stVzwapneInfo, sizeof(stVzwapneInfo), 0, sizeof(TAF_PS_VZWAPNE_INFO_STRU));
    ucIsCustomCmd = VOS_FALSE;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* +VZWAPNE命令标记 */
    if (AT_CMD_VZWAPNE == g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex)
    {
        ucIsCustomCmd = VOS_TRUE;
    }

    /* 设置<wapn>/<cid> */
    stVzwapneInfo.ucCid     = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 设置<apncl> */
    stVzwapneInfo.ucClass   = (VOS_UINT8)gastAtParaList[1].ulParaValue;

    /* 设置<apnni> */
    if (gastAtParaList[2].usParaLen > TAF_MAX_APN_LEN)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (0 != gastAtParaList[2].usParaLen)
    {
        /* 检查 */
        if ( VOS_OK != AT_CheckApnFormat(gastAtParaList[2].aucPara,
                                         gastAtParaList[2].usParaLen,
                                         gastAtClientTab[ucIndex].usClientId) )
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stVzwapneInfo.stApn.ucLength = (VOS_UINT8)gastAtParaList[2].usParaLen;
        TAF_MEM_CPY_S(stVzwapneInfo.stApn.aucValue,
                      sizeof(stVzwapneInfo.stApn.aucValue),
                      gastAtParaList[2].aucPara,
                      gastAtParaList[2].usParaLen);
    }

    /* 设置<apntype> */
    stVzwapneInfo.enIpType  = (VOS_UINT8)gastAtParaList[3].ulParaValue + 1;

    /* 设置<apned> */
    stVzwapneInfo.ucEnable  = (VOS_UINT8)gastAtParaList[5].ulParaValue;

    /* 设置<apntime> */
    stVzwapneInfo.usInactiveTimer  = (VOS_UINT8)gastAtParaList[6].ulParaValue;

    /* 发送跨核消息 */
    if (VOS_OK != TAF_PS_SetVzwApneInfo(WUEPS_PID_AT,
                                        AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                        gastAtClientTab[ucIndex].opId,
                                        ucIsCustomCmd,
                                        &stVzwapneInfo))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VZWAPNE_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryVzwApnePara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucIsCustomCmd;

    ucIsCustomCmd = VOS_FALSE;

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* +VZWAPNE命令标记 */
    if (AT_CMD_VZWAPNE == g_stParseContext[ucIndex].pstCmdElement->ulCmdIndex)
    {
        ucIsCustomCmd = VOS_TRUE;
    }

    /* 发送跨核消息 */
    if (VOS_OK != TAF_PS_GetVzwApneInfo(WUEPS_PID_AT,
                                        AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                        gastAtClientTab[ucIndex].opId,
                                        ucIsCustomCmd))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_VZWAPNE_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestVzwApnePara(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
        (VOS_CHAR *)pgucAtSndCodeAddr,
        (VOS_CHAR *)pgucAtSndCodeAddr,
        "%s: (1-8),(1-8),(\"VZWIMS\",\"VZWADMIN\",\"VZWINTERNET\",\"VZWAPP\"),(\"IPv4\",\"IPv6\",\"IPv4v6\"),(\"LTE\"),(\"Enabled\",\"Disabled\"),(0-255)",
        g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetCgerepPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_NET_CTX_STRU                  *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* +CGEREP=[<mode>[,<bfr>]] */
    if (0 == gucAtParaIndex)
    {
        pstNetCtx->stCgerepCfg.ucMode   = 0;
        pstNetCtx->stCgerepCfg.ucBfr    = 0;
    }
    else if (1 == gucAtParaIndex)
    {
        if (0 == gastAtParaList[0].usParaLen)
        {
             return AT_CME_INCORRECT_PARAMETERS;
        }

        pstNetCtx->stCgerepCfg.ucMode   = (VOS_UINT8)gastAtParaList[0].ulParaValue;
        pstNetCtx->stCgerepCfg.ucBfr    = 0;
    }
    else
    {
        if ( (0 == gastAtParaList[0].usParaLen)
          || (0 == gastAtParaList[1].usParaLen))
        {
             return AT_CME_INCORRECT_PARAMETERS;
        }

        pstNetCtx->stCgerepCfg.ucMode   = (VOS_UINT8)gastAtParaList[0].ulParaValue;
        pstNetCtx->stCgerepCfg.ucBfr    = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    }

    return AT_OK;
}


VOS_UINT32 AT_QryCgerepPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d,%d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    pstNetCtx->stCgerepCfg.ucMode,
                                                    pstNetCtx->stCgerepCfg.ucBfr);


    return AT_OK;
}


VOS_UINT32 AT_SetCindPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;
    TAF_START_INFO_IND_STRU             stStartInfoInd;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);
    TAF_MEM_SET_S(&stStartInfoInd, sizeof(stStartInfoInd), 0x00, sizeof(stStartInfoInd));

    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (1 < gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (0 == gucAtParaIndex)
    {
        return AT_OK;
    }

    /* +CIND=[<ind>] 目前只支持signal的设置 */
    if (0 != gastAtParaList[0].usParaLen)
    {
        pstNetCtx->ucCerssiReportType = (AT_CERSSI_REPORT_TYPE_ENUM_UINT8)gastAtParaList[0].ulParaValue;

        stStartInfoInd.ucActionType             = TAF_START_EVENT_INFO_FOREVER;
        stStartInfoInd.ucSignThreshold          = (VOS_UINT8)gastAtParaList[0].ulParaValue;
        stStartInfoInd.ucRrcMsgType             = TAF_EVENT_INFO_CELL_SIGN;
        stStartInfoInd.ucMinRptTimerInterval    = pstNetCtx->ucCerssiMinTimerInterval;

        if (VOS_TRUE == TAF_MMA_SetCindReq(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stStartInfoInd))
        {
            gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CIND_SET;
            return AT_WAIT_ASYNC_RETURN;
        }
        else
        {
            return AT_ERROR;
        }
    }

    return AT_OK;
}


VOS_UINT32 AT_QryCindPara(VOS_UINT8 ucIndex)
{
    AT_MODEM_NET_CTX_STRU              *pstNetCtx = VOS_NULL_PTR;

    pstNetCtx = AT_GetModemNetCtxAddrFromClientId(ucIndex);

    /* 参数检查 */
    if (AT_CMD_OPT_READ_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* +CIND: <ind> 目前只支持signal的查询 */
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                    "%s: %d",
                                                    g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                                    pstNetCtx->ucCerssiReportType);


    return AT_OK;
}


VOS_UINT32 AT_TestCindPara(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
        (VOS_CHAR *)pgucAtSndCodeAddr,
        (VOS_CHAR *)pgucAtSndCodeAddr,
        "%s: \"signal\",(0-5)",
        g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_SetDataSwitchStatus( VOS_UINT8 ucIndex )
{
    VOS_UINT8                           ucDataSwitchAT;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetDataSwitchStatus: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (1 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetDataSwitchStatus: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (1 != gastAtParaList[0].usParaLen)
    {
        AT_WARN_LOG("AT_SetDataSwitchStatus: usParaLen  is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;

    }

    ucDataSwitchAT=(VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 给TAF APS发送通知消息 */
    if (VOS_OK != TAF_PS_Set_DataSwitchStatus(WUEPS_PID_AT,
                                              AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                              0,
                                              ucDataSwitchAT))
    {
        AT_WARN_LOG("AT_SetDataSwitchStatus():TAF_PS_Set_DataSwitchStatus fail");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DATASWITCH_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetDataRoamSwitchStatus( VOS_UINT8 ucIndex )
{
    VOS_UINT8                           ucDataRoamSwitchAT;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        AT_WARN_LOG("AT_SetDataRoamSwitchStatus: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (1 != gucAtParaIndex)
    {
        AT_WARN_LOG("AT_SetDataRoamSwitchStatus: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (1 != gastAtParaList[0].usParaLen)
    {
        AT_WARN_LOG("AT_SetDataRoamSwitchStatus: usParaLen  is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;

    }

    ucDataRoamSwitchAT=(VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 给TAF APS发送通知消息 */
    if (VOS_OK != TAF_PS_Set_DataRoamSwitchStatus(WUEPS_PID_AT,
                                                  AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                                  0,
                                                  ucDataRoamSwitchAT) )
    {
        AT_WARN_LOG("AT_SetDataRoamSwitchStatus():TAF_PS_Set_DataRoamSwitchStatus fail");
        return AT_ERROR;
    }


    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DATAROAMSWITCH_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryDataSwitchStatus(  VOS_UINT8 ucIndex )
{
    /* 发送跨核消息 */
    if ( VOS_OK != TAF_PS_Get_DataSwitchStatus(WUEPS_PID_AT,
                                               AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                               0))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DATASWITCH_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}



VOS_UINT32 AT_QryDataRoamSwitchStatus(  VOS_UINT8 ucIndex )
{
    /* 发送跨核消息 */
    if ( VOS_OK != TAF_PS_Get_DataRoamSwitchStatus(WUEPS_PID_AT,
                                                   AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                                   0))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DATAROAMSWITCH_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetApnThrotInfoPara(VOS_UINT8 ucIndex)
{
    TAF_PS_APN_THROT_INFO_STRU                stApnThrotInfo;

    TAF_MEM_SET_S(&stApnThrotInfo, sizeof(TAF_PS_APN_THROT_INFO_STRU), 0, sizeof(TAF_PS_APN_THROT_INFO_STRU));

    /* 指令类型检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数目错误 */
    if (5 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ( (0 == gastAtParaList[0].usParaLen)
      || (0 == gastAtParaList[1].usParaLen)
      || (0 == gastAtParaList[2].usParaLen)
      || (0 == gastAtParaList[3].usParaLen)
      || (0 == gastAtParaList[4].usParaLen) )
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* CID与ApnClassType 必须一一对应 */
    if (gastAtParaList[0].ulParaValue != gastAtParaList[1].ulParaValue)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置参数 */
    stApnThrotInfo.ucCid               = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    stApnThrotInfo.ucApnClassType      = (VOS_UINT8)gastAtParaList[1].ulParaValue;
    stApnThrotInfo.ulWaitTime          = gastAtParaList[2].ulParaValue;
    stApnThrotInfo.ulPdnMaxConnTime    = gastAtParaList[3].ulParaValue;
    stApnThrotInfo.ulPdnMaxConnCount   = gastAtParaList[4].ulParaValue;

    /* 发送跨核消息 */
    if (VOS_OK != TAF_PS_SetApnThrotInfo(WUEPS_PID_AT,
                                         AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                         gastAtClientTab[ucIndex].opId,
                                         &stApnThrotInfo))
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_APN_THROT_INFO_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 At_SetUsbTetherInfo(VOS_UINT8 ucIndex)
{
    AT_RNIC_USB_TETHER_INFO_IND_STRU   *pstMsg = VOS_NULL_PTR;

    /* 指令类型检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数目错误 */
    if (2 != gucAtParaIndex)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((0 == gastAtParaList[0].usParaLen)
     || (0 == gastAtParaList[1].usParaLen)
     || (RNIC_RMNET_NAME_MAX_LEN < gastAtParaList[1].usParaLen))
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    AT_WARN_LOG1("At_SetUsbTetherInfo: para2 len is ",gastAtParaList[1].usParaLen);

    pstMsg = (AT_RNIC_USB_TETHER_INFO_IND_STRU *)AT_ALLOC_MSG_WITH_HDR(sizeof(AT_RNIC_USB_TETHER_INFO_IND_STRU));

    if (VOS_NULL_PTR == pstMsg)
    {
        AT_WARN_LOG("At_SetUsbTetherInfo: alloc msg fail!");
        return AT_ERROR;
    }

    /* 初始化消息体 */
    TAF_MEM_SET_S((VOS_INT8 *)pstMsg + VOS_MSG_HEAD_LENGTH,
                  (VOS_SIZE_T)sizeof(AT_RNIC_USB_TETHER_INFO_IND_STRU) - VOS_MSG_HEAD_LENGTH,
                  0x00,
                  (VOS_SIZE_T)sizeof(AT_RNIC_USB_TETHER_INFO_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    /* 填写消息头和消息ID */
    AT_CFG_MSG_HDR( pstMsg, ACPU_PID_RNIC, ID_AT_RNIC_USB_TETHER_INFO_IND );

    /* 填写消息体 */
    pstMsg->enTetherConnStat = (VOS_UINT8)gastAtParaList[0].ulParaValue;
    TAF_MEM_CPY_S(pstMsg->aucRmnetName, RNIC_RMNET_NAME_MAX_LEN, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);

    if (VOS_OK != PS_SEND_MSG(WUEPS_PID_AT, pstMsg))
    {
        AT_WARN_LOG("At_SetUsbTetherInfo: Send msg fail!");
        return AT_ERROR;
    }

    return AT_OK;
}


VOS_UINT32 At_TestUsbTetherInfo(VOS_UINT8 ucIndex)
{
    /* 参数检查 */
    if (AT_CMD_OPT_TEST_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
        (VOS_CHAR *)pgucAtSndCodeAddr,
        (VOS_CHAR *)pgucAtSndCodeAddr,
        "%s: (\"0:disconn\",\"1:connect\"),(\"rmnet0 - rmnet6\")",
        g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 AT_PS_Ipv6IfaceDynamicRstCheck(
    const VOS_UINT8                             ucIndex,
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU        *pstIfaceDynamicInfo,
    VOS_UINT8                                  *pucCid
)
{
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;

    if (AT_GetCidByCidMask(pstIfaceDynamicInfo->ulBitCid, &ucCid) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck:ERROR: ulBitCid is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if (pstDynamicInfo[ucCid].bitOpIpv6Valid == VOS_FALSE)
    {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck:ERROR: bitOpIpv6Valid is FALSE");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    if (pstDynamicInfo[ucCid].stIpv6Info.enState != TAF_IFACE_STATE_ACTED)
    {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck: PDP is not actived in cellular.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    if (pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.bitOpPrefixAddr != VOS_TRUE)
    {
        AT_WARN_LOG("AT_PS_Ipv6IfaceDynamicRstCheck: Prefix address is not received.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    *pucCid = ucCid;

    return VOS_OK;
}


VOS_UINT32 At_QryDconnStatPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("At_QryDconnStatPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DCONNSTAT_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_TestDconnStatPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;

    /* 参数检查 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_TEST_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 构造控制结构体 */
    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));
    if (AT_PS_BuildIfaceCtrl(WUEPS_PID_AT, gastAtClientTab[ucIndex].usClientId, 0, &stCtrl) == VOS_ERR)
    {
        return AT_ERROR;
    }

    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_TestDconnStatPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_DCONNSTAT_TEST;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_QryNdisStatPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_QryIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    /* 支持1-11的cid */
    ulBitCid = TAF_IFACE_ALL_BIT_NV_CID;

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_QryNdisStatPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_NDISSTATQRY_QRY;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetCgmtuPara(VOS_UINT8 ucIndex)
{
    TAF_CTRL_STRU                       stCtrl;
    VOS_UINT32                          ulBitCid;
    VOS_UINT32                          ulRst;

    TAF_MEM_SET_S(&stCtrl, sizeof(stCtrl), 0x00, sizeof(TAF_CTRL_STRU));

    ulRst = At_SetIfaceDynamicParaComCheck(ucIndex, &stCtrl);

    if (ulRst != VOS_OK)
    {
        return ulRst;
    }

    ulBitCid = (VOS_UINT32)(0x01UL << gastAtParaList[0].ulParaValue);

    /* 发送查询消息 */
    if (TAF_IFACE_GetDynamicPara(&stCtrl, ulBitCid) != VOS_OK)
    {
        AT_ERR_LOG("AT_SetCgmtuPara: TAF_IFACE_GetDynamicPara return ERROR!");
        return AT_ERROR;
    }

    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_CGMTU_SET;

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_PS_ReportSetIpv6TempAddrRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;

    if (AT_PS_Ipv6IfaceDynamicRstCheck(ucIndex, pstIfaceDynamicInfo, &ucCid) != VOS_OK)
    {
        return VOS_ERR;
    }
    
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if (!AT_PS_IS_IPV6_ADDR_IID_VALID(pstDynamicInfo[ucCid].stIpv6Info.stDhcpInfo.aucTmpAddr))
    {
        AT_WARN_LOG("AT_PS_ReportSetIpv6TempAddrRst: IID is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      pstDynamicInfo[ucCid].stIpv6Info.stDhcpInfo.aucTmpAddr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "\"%s\"",
                                       aucIpv6AddrStr);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportSetDhcpRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_DISPLAY_RATE_STRU                    stSpeed;
    AT_DHCP_PARA_STRU                       stDhcpConfig;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;

    if (AT_GetCidByCidMask(pstIfaceDynamicInfo->ulBitCid, &ucCid) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: ulBitCid is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstDynamicInfo = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if (pstDynamicInfo[ucCid].bitOpIpv4Valid == VOS_FALSE)
    {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: bitOpIpv4Valid is FALSE");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    if (pstDynamicInfo[ucCid].stIpv4Info.enState != TAF_IFACE_STATE_ACTED)
    {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: PDP is not actived.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    /* 获取接入理论带宽*/
    TAF_MEM_SET_S(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));
    if (AT_GetDisplayRate(ucIndex, &stSpeed) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetDhcpRst: AT_GetDisplayRate Error!");
    }

    /* 获取DHCP参数(网络序) */
    TAF_MEM_SET_S(&stDhcpConfig, sizeof(stDhcpConfig), 0x00, (VOS_SIZE_T)(sizeof(AT_DHCP_PARA_STRU)));
    AT_GetDhcpPara(&stDhcpConfig, &(pstDynamicInfo[ucCid].stIpv4Info.stDhcpInfo));

    usLength = 0;
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulIPAddr);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulSubNetMask);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulGateWay);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulGateWay);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulPrimDNS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulSndDNS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s,",stSpeed.ucDlSpeed);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s",stSpeed.ucUlSpeed);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryDhcpRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_DISPLAY_RATE_STRU                    stSpeed;
    AT_DHCP_PARA_STRU                       stDhcpConfig;
    VOS_UINT32                              i;
    VOS_UINT32                              ulRst;
    VOS_UINT16                              usLength;

    usLength            = 0;
    ulRst               = AT_ERROR;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));
    TAF_MEM_SET_S(&stDhcpConfig, sizeof(stDhcpConfig), 0x00, (VOS_SIZE_T)(sizeof(AT_DHCP_PARA_STRU)));

    /* 目前DHCP的查询只支持1-11的cid */
    for (i = 1; i <= TAF_MAX_CID_NV; i++)
    {
        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & (0x01UL << i)) == 0)
        {
            continue;
        }

        if (pstDynamicInfo[i].bitOpIpv4Valid == VOS_FALSE)
        {
            continue;
        }

        if (pstDynamicInfo[i].stIpv4Info.enState != TAF_IFACE_STATE_ACTED)
        {
            continue;
        }

        /* 获取接入理论带宽*/
        if (AT_GetDisplayRate(ucIndex, &stSpeed) == VOS_ERR)
        {
            AT_WARN_LOG("AT_PS_ReportQryDhcpRst: AT_GetDisplayRate Error!");
        }

        /* 获取DHCP参数(网络序) */
        AT_GetDhcpPara(&stDhcpConfig, &(pstDynamicInfo[i].stIpv4Info.stDhcpInfo));

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulIPAddr);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulSubNetMask);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulGateWay);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulGateWay);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulPrimDNS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%08X,",stDhcpConfig.stDhcpCfg.ulSndDNS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s,",stSpeed.ucDlSpeed);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,(VOS_CHAR *)pgucAtSndCodeAddr,(VOS_CHAR *)pgucAtSndCodeAddr + usLength,"%s",stSpeed.ucUlSpeed);

        ulRst    = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRst);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportSetDhcpv6Rst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_DISPLAY_RATE_STRU                    stSpeed;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;

    if (AT_PS_Ipv6IfaceDynamicRstCheck(ucIndex, pstIfaceDynamicInfo, &ucCid) != VOS_OK)
    {
        return VOS_ERR;
    }

    /* 获取接入理论带宽*/
    TAF_MEM_SET_S(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));
    if (AT_GetDisplayRate(ucIndex, &stSpeed) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetDhcpv6Rst: AT_GetDisplayRate Error!");
    }

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr, "%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    /* 填写IPV6地址 */
    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      pstDynamicInfo[ucCid].stIpv6Info.stDhcpInfo.aucAddr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%s", aucIpv6AddrStr);

    /* 填写IPV6掩码, 该字段填全0 */
    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      aucInvalidIpv6Addr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

    /* 填写IPV6网关, 该字段填全0 */
    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      aucInvalidIpv6Addr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

    /* 填写DHCP IPV6, 该字段填全0 */
    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      aucInvalidIpv6Addr,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

    /* 填写IPV6 Primary DNS */
    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      pstDynamicInfo[ucCid].stIpv6Info.stDhcpInfo.aucPrimDns,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

    /* 填写IPV6 Secondary DNS */
    AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                      pstDynamicInfo[ucCid].stIpv6Info.stDhcpInfo.aucSecDns,
                                      TAF_IPV6_STR_RFC2373_TOKENS);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

    /* 填写MAX RX/TX Rate */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", stSpeed.ucDlSpeed);
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", stSpeed.ucUlSpeed);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryDhcpv6Rst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_DISPLAY_RATE_STRU                    stSpeed;
    VOS_UINT32                              i;
    VOS_UINT32                              ulRst;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    usLength            = 0;
    ulRst               = AT_ERROR;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));
    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);

    for ( i = 1; i <= TAF_MAX_CID_NV; i++ )
    {
        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & 0x01UL << i) == 0)
        {
            continue;
        }

        if (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE)
        {
             continue;
        }

        if (pstDynamicInfo[i].stIpv6Info.enState != TAF_IFACE_STATE_ACTED)
        {
            continue;
        }

        if (pstDynamicInfo[i].stIpv6Info.stRaInfo.bitOpPrefixAddr != VOS_TRUE)
        {
            continue;
        }

        /* 获取接入理论带宽*/
        if (AT_GetDisplayRate(ucIndex, &stSpeed) == VOS_ERR)
        {
            AT_WARN_LOG("AT_PS_ReportQryDhcpv6Rst: AT_GetDisplayRate Error!");
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%s: ",g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

        /* 填写IPV6地址 */
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          pstDynamicInfo[i].stIpv6Info.stDhcpInfo.aucAddr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, "%s", aucIpv6AddrStr);

        /* 填写IPV6掩码, 该字段填全0 */
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          aucInvalidIpv6Addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

        /* 填写IPV6网关, 该字段填全0 */
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          aucInvalidIpv6Addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

        /* 填写DHCP IPV6, 该字段填全0 */
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          aucInvalidIpv6Addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

        /* 填写IPV6 Primary DNS */
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          pstDynamicInfo[i].stIpv6Info.stDhcpInfo.aucPrimDns,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

        /* 填写IPV6 Secondary DNS */
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          pstDynamicInfo[i].stIpv6Info.stDhcpInfo.aucSecDns,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", aucIpv6AddrStr);

        /* 填写MAX RX/TX Rate */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", stSpeed.ucDlSpeed);
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength, ",%s", stSpeed.ucUlSpeed);

        ulRst    = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRst);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportSetApRaInfoRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    VOS_UINT32                              ulMtuSize;
    VOS_UINT32                              ulPrefixBitLen;
    VOS_UINT32                              ulPreferredLifetime;
    VOS_UINT32                              ulValidLifetime;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;

    if (AT_GetCidByCidMask(pstIfaceDynamicInfo->ulBitCid, &ucCid) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetApRaInfoRst: ulBitCid is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if (pstDynamicInfo[ucCid].bitOpIpv6Valid == VOS_FALSE)
    {
        AT_WARN_LOG("AT_PS_ReportSetApRaInfoRst: bitOpIpv6Valid is FALSE");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    if (pstDynamicInfo[ucCid].stIpv6Info.enState != TAF_IFACE_STATE_ACTED)
    {
        AT_WARN_LOG("AT_PS_ReportSetApRaInfoRst: PDP is not actived in cellular.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    /* 填写MTU */
    if (pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.bitOpMtuSize == VOS_FALSE)
    {
        ulMtuSize = 0;
    }
    else
    {
        ulMtuSize = pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.ulMtuSize;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "%d",
                                       ulMtuSize);
    /* 填写Prefix */
    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);

    if (pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.bitOpPrefixAddr == VOS_FALSE)
    {
        ulPrefixBitLen = 0;
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          aucInvalidIpv6Addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
    }
    else
    {
        ulPrefixBitLen = pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.ulPrefixBitLen;
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.aucPrefixAddr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       ",\"%s/%d\"",
                                       aucIpv6AddrStr,
                                       ulPrefixBitLen);

    /* 填写Preferred Lifetime */
    if (pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.bitOpPreferredLifetime == VOS_FALSE)
    {
        ulPreferredLifetime = 0;
    }
    else
    {
        ulPreferredLifetime = pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.ulPreferredLifetime;
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       ",%u",
                                       ulPreferredLifetime);

    if (pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.bitOpValidLifetime == VOS_FALSE)
    {
        ulValidLifetime = 0;
    }
    else
    {
        ulValidLifetime = pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.ulValidLifetime;
    }

    /* 填写Valid Lifetime */
    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       ",%u",
                                       ulValidLifetime);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryApRaInfoRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    TAF_IFACE_IPV6_RA_INFO_STRU            *pstAppRaInfoAddr    = VOS_NULL_PTR;
    VOS_UINT32                              ulMtuSize;
    VOS_UINT32                              ulPrefixBitLen;
    VOS_UINT32                              ulPreferredLifetime;
    VOS_UINT32                              ulValidLifetime;
    VOS_UINT32                              i;
    VOS_UINT32                              ulRst;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    usLength            = 0;
    ulRst               = AT_ERROR;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);

    for (i = 1; i <= TAF_MAX_CID_NV; i++)
    {
        pstAppRaInfoAddr = &(pstDynamicInfo[i].stIpv6Info.stRaInfo);

        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & (VOS_UINT32)1 << i) == 0)
        {
            continue;
        }

        if (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE)
        {
            continue;
        }

        if (pstDynamicInfo[i].stIpv6Info.enState != TAF_IFACE_STATE_ACTED)
        {
            continue;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%s: %d,",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                           i);
        /* 填写MTU */
        if (pstAppRaInfoAddr->bitOpMtuSize == VOS_FALSE)
        {
            ulMtuSize = 0;
        }
        else
        {
            ulMtuSize = pstAppRaInfoAddr->ulMtuSize;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           ulMtuSize);

        /* 填写Prefix */
        if (pstAppRaInfoAddr->bitOpPrefixAddr == VOS_FALSE)
        {
            ulPrefixBitLen = 0;
            AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                              aucInvalidIpv6Addr,
                                              TAF_IPV6_STR_RFC2373_TOKENS);
        }
        else
        {
            ulPrefixBitLen = pstAppRaInfoAddr->ulPrefixBitLen;
            AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                              pstAppRaInfoAddr->aucPrefixAddr,
                                              (VOS_UINT8)ulPrefixBitLen/16);
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",\"%s/%d\"",
                                           aucIpv6AddrStr,
                                           ulPrefixBitLen);

        /* 填写Preferred Lifetime */
        if (pstAppRaInfoAddr->bitOpPreferredLifetime == VOS_FALSE)
        {
            ulPreferredLifetime = 0;
        }
        else
        {
            ulPreferredLifetime = pstAppRaInfoAddr->ulPreferredLifetime;
        }

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",%u",
                                           ulPreferredLifetime);

        if (pstAppRaInfoAddr->bitOpValidLifetime == VOS_FALSE)
        {
            ulValidLifetime = 0;
        }
        else
        {
            ulValidLifetime = pstAppRaInfoAddr->ulValidLifetime;
        }

        /* 填写Valid Lifetime */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",%u%s",
                                           ulValidLifetime,
                                           gaucAtCrLf);
    }

    if (usLength < (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf))
    {
        AT_ERR_LOG("AT_PS_ReportQryApRaInfoRst: CID INFO is invalid.");
    }
    else
    {
        usLength -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
        ulRst     = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRst);

    return VOS_OK;

}


VOS_UINT32 AT_PS_ReportSetApLanAddrRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    TAF_IFACE_IPV6_RA_INFO_STRU            *pstIpv6RaInfo       = VOS_NULL_PTR;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;

    if (AT_GetCidByCidMask(pstIfaceDynamicInfo->ulBitCid, &ucCid) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetApLanAddrRst: ulBitCid is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if (pstDynamicInfo[ucCid].bitOpIpv6Valid == VOS_FALSE)
    {
        AT_WARN_LOG("AT_PS_ReportSetApLanAddrRst: bitOpIpv6Valid is FALSE");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }


    /* 当前未激活，直接返回error */
    if (pstDynamicInfo[ucCid].stIpv6Info.enState != TAF_IFACE_STATE_ACTED)
    {
        AT_WARN_LOG("AT_PS_ReportSetApLanAddrRst: PDP is not actived in cellular.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstIpv6RaInfo = &(pstDynamicInfo[ucCid].stIpv6Info.stRaInfo);
    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);

    usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr,
                                       "%s: ",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    if (pstIpv6RaInfo->bitOpLanAddr == VOS_FALSE)
    {
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          aucInvalidIpv6Addr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
    }
    else
    {
        AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                          pstIpv6RaInfo->aucLanAddr,
                                          TAF_IPV6_STR_RFC2373_TOKENS);
    }

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       "\"%s\"",
                                       aucIpv6AddrStr);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                       ",%d",
                                       pstIpv6RaInfo->ulPrefixBitLen);

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryApLanAddrRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    TAF_IFACE_IPV6_RA_INFO_STRU            *pstAppRaInfoAddr    = VOS_NULL_PTR;
    VOS_UINT32                              i;
    VOS_UINT32                              ulRst;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    usLength            = 0;
    ulRst               = AT_ERROR;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);

    if (AT_APP_USER == gastAtClientTab[ucIndex].UserType)
    {
        for (i = 1; i <= TAF_MAX_CID_NV; i++)
        {
            pstAppRaInfoAddr = &(pstDynamicInfo[i].stIpv6Info.stRaInfo);

            /* 判断此CID是否有效*/
            if ((pstIfaceDynamicInfo->ulBitCid & 0x01UL << i) == 0)
            {
                continue;
            }

            if (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE)
            {
                continue;
            }

            /* 当前未激活*/
            if (pstDynamicInfo[i].stIpv6Info.enState != TAF_IFACE_STATE_ACTED)
            {
                continue;
            }

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s: %d,",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                               i);

            if (pstAppRaInfoAddr->bitOpLanAddr == VOS_FALSE)
            {
                AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                                  aucInvalidIpv6Addr,
                                                  TAF_IPV6_STR_RFC2373_TOKENS);
            }
            else
            {
                AT_ConvertIpv6AddrToCompressedStr(aucIpv6AddrStr,
                                                  pstAppRaInfoAddr->aucLanAddr,
                                                  TAF_IPV6_STR_RFC2373_TOKENS);
            }

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "\"%s\"",
                                               aucIpv6AddrStr);

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr, (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               ",%d%s",
                                               pstAppRaInfoAddr->ulPrefixBitLen,
                                               gaucAtCrLf);
        }
    }

    if (usLength < (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf))
    {
        AT_ERR_LOG("AT_PS_ReportQryApLanAddrRst: CID INFO is invalid.");
    }
    else
    {
        usLength -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
        ulRst     = AT_OK;
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, ulRst);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportSetApConnStRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_APP_CONN_STATE_ENUM_U32              enIpv4ConnStatus;
    AT_APP_CONN_STATE_ENUM_U32              enIpv6ConnStatus;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU*)pstEvtInfo;

    if (AT_GetCidByCidMask(pstIfaceDynamicInfo->ulBitCid, &ucCid) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetApConnStRst: ulBitCid is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if ( (pstDynamicInfo[ucCid].bitOpIpv4Valid == VOS_FALSE)
      && (pstDynamicInfo[ucCid].bitOpIpv6Valid == VOS_FALSE))
    {
        AT_WARN_LOG("AT_PS_ReportSetApConnStRst: bitOpIpv4v6Valid both FALSE");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    usLength         = 0;
    enIpv4ConnStatus = AT_APP_UNDIALED;
    enIpv6ConnStatus = AT_APP_UNDIALED;

    switch (AT_GetIpv6Capability())
    {
        case AT_IPV6_CAPABILITY_IPV4_ONLY:
            enIpv4ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[ucCid].stIpv4Info.enState);

            usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s: ",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d,\"IPV4\"",
                                               enIpv4ConnStatus);
            break;

        case AT_IPV6_CAPABILITY_IPV6_ONLY:
            enIpv6ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[ucCid].stIpv6Info.enState);

            usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s: ",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d,\"IPV6\"",
                                               enIpv6ConnStatus);
            break;

        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
            enIpv4ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[ucCid].stIpv4Info.enState);
            enIpv6ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[ucCid].stIpv6Info.enState);

            usLength  = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               "%s: ",
                                               g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d,\"IPV4\"",
                                               enIpv4ConnStatus);
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               ",%d,\"IPV6\"",
                                               enIpv6ConnStatus);
            break;

        default:
            break;
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryApConnStRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_APP_CONN_STATE_ENUM_U32              enIpv4ConnStatus;
    AT_APP_CONN_STATE_ENUM_U32              enIpv6ConnStatus;
    VOS_UINT32                              i;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucIpv6AddrStr[TAF_MAX_IPV6_ADDR_COLON_STR_LEN];
    VOS_UINT8                               aucInvalidIpv6Addr[TAF_IPV6_ADDR_LEN];

    usLength            = 0;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(aucIpv6AddrStr, sizeof(aucIpv6AddrStr), 0x00, TAF_MAX_IPV6_ADDR_COLON_STR_LEN);
    TAF_MEM_SET_S(aucInvalidIpv6Addr, sizeof(aucInvalidIpv6Addr), 0x00, TAF_IPV6_ADDR_LEN);

    for ( i = 1; i <= TAF_MAX_CID_NV; i++ )
    {
        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & (VOS_UINT32)1 << i) == 0)
        {
            continue;
        }

        if ( (pstDynamicInfo[i].bitOpIpv4Valid == VOS_FALSE)
          && (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE))
        {
            continue;
        }

        switch (AT_GetIpv6Capability())
        {
            case AT_IPV6_CAPABILITY_IPV4_ONLY:
                enIpv4ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[i].stIpv4Info.enState);

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s: ",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%d,%d,\"IPV4\"%s",
                                                   i,
                                                   enIpv4ConnStatus,
                                                   gaucAtCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV6_ONLY:
                enIpv6ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[i].stIpv6Info.enState);

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s: ",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%d,%d,\"IPV6\"%s",
                                                   i,
                                                   enIpv6ConnStatus,
                                                   gaucAtCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
            case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                enIpv4ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[i].stIpv4Info.enState);
                enIpv6ConnStatus = AT_AppConvertPdpStateToConnStatus(pstDynamicInfo[i].stIpv6Info.enState);

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%s: ",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   "%d,%d,\"IPV4\"",
                                                   i,
                                                   enIpv4ConnStatus);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr,
                                                   (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                                   ",%d,\"IPV6\"%s",
                                                   enIpv6ConnStatus,
                                                   gaucAtCrLf);
                break;

            default:
                break;
        }
    }

    if (usLength >= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf))
    {
        usLength -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
    }

    gstAtSendData.usBufLen = usLength;

    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryDconnStatRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    TAF_PDP_PRIM_CONTEXT_STRU               stPdpPriPara;
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status;
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status;
    AT_DIALTYPE_ENUM_UINT32                 enDialType;
    VOS_UINT32                              i;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucStr[TAF_MAX_APN_LEN + 1];

    usLength        = 0;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(&stPdpPriPara, sizeof(stPdpPriPara), 0x00, (VOS_SIZE_T)(sizeof(TAF_PDP_PRIM_CONTEXT_STRU)));

    /* 目前DHCP的查询只支持1-11的cid */
    for ( i = 1; i <= TAF_MAX_CID_NV; i++ )
    {
        /* ^DCONNSTAT: */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s: ",
                                           g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
        /* CID */
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           "%d",
                                           i);

        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & 0x01UL << i) == 0)
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s", gaucAtCrLf);
            continue;
        }

        if ( (pstDynamicInfo[i].bitOpIpv4Valid == VOS_FALSE)
          && (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s", gaucAtCrLf);
            continue;
        }

        enIpv4Status    = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv4Info.enState);
        enIpv6Status    = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv6Info.enState);

        if ((enIpv4Status == AT_PDP_STATUS_DEACT)
         && (enIpv6Status == AT_PDP_STATUS_DEACT))
        {
            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucAtSndCodeAddr,
                                               (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                               "%s", gaucAtCrLf);
            continue;
        }

        if (pstDynamicInfo[i].enUserType == TAF_IFACE_USER_TYPE_NDIS)
        {
            enDialType = AT_DIALTYPE_NDIS;
        }
        else if (pstDynamicInfo[i].enUserType == TAF_IFACE_USER_TYPE_APP)
        {
            enDialType = AT_DIALTYPE_APP;
        }
        else
        {
            enDialType = AT_DIALTYPE_BUTT;
        }

        /* APN */
        TAF_MEM_SET_S(aucStr, sizeof(aucStr), 0x00, sizeof(aucStr));
        TAF_MEM_CPY_S(aucStr, sizeof(aucStr), pstDynamicInfo[i].aucApn, 
                      TAF_MIN(pstDynamicInfo[i].ucApnLen, TAF_MAX_APN_LEN));

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           ",\"%s\"",
                                           aucStr);
        /* IPV4 IPV6 STATUS*/
        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                           ",%d,%d,%d%s",
                                           enIpv4Status, enIpv6Status, enDialType, gaucAtCrLf);
    }

    if (usLength >= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf))
    {
        usLength -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
    }

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportTestDconnStatRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status;
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status;
    VOS_UINT32                              i;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucHasDialedFlg;

    usLength            = 0;
    ucHasDialedFlg      = VOS_FALSE;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo      = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                       "%s: %s",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName, "(");

    /* 目前DHCP的查询只支持1-11的cid */
    for ( i = 1; i <= TAF_MAX_CID_NV; i++ )
    {
        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & 0x01UL << i) == 0)
        {
            continue;
        }

        if ( (pstDynamicInfo[i].bitOpIpv4Valid == VOS_FALSE)
          && (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE))
        {
            continue;
        }

        enIpv4Status    = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv4Info.enState);
        enIpv6Status    = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv6Info.enState);

        if ( (enIpv4Status == AT_PDP_STATUS_ACT)
          || (enIpv6Status == AT_PDP_STATUS_ACT))
        {
            ucHasDialedFlg = VOS_TRUE;

            usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucAtSndCodeAddr,
                                               (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                               "%d,",i);
        }
    }

    if (ucHasDialedFlg == VOS_TRUE)
    {
        /* 删除最后一个逗号 */
        usLength--;

        usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR *)pgucAtSndCodeAddr + usLength,
                                           "%s", ")");
    }
    else
    {
        /* 若无连接态CID,直接返回OK */
        usLength = 0;
    }

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportQryNdisStatRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    TAF_PDP_PRIM_CONTEXT_STRU               pstPdpPriPara;
    AT_DISPLAY_RATE_STRU                    stSpeed;
    AT_PDP_STATUS_ENUM_UINT32               enIpv4Status;
    AT_PDP_STATUS_ENUM_UINT32               enIpv6Status;
    VOS_UINT32                              i;
    VOS_UINT16                              usLength;
    VOS_UINT8                               aucAtStrIpv4[] = "IPV4";
    VOS_UINT8                               aucAtStrIpv6[] = "IPV6";

    usLength        = 0;
    enIpv4Status    = AT_PDP_STATUS_DEACT;
    enIpv6Status    = AT_PDP_STATUS_DEACT;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;
    pstDynamicInfo = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    TAF_MEM_SET_S(&stSpeed, sizeof(stSpeed), 0x00, (VOS_SIZE_T)(sizeof(AT_DISPLAY_RATE_STRU)));
    TAF_MEM_SET_S(&pstPdpPriPara, sizeof(pstPdpPriPara), 0x00, (VOS_SIZE_T)(sizeof(TAF_PDP_PRIM_CONTEXT_STRU)));

    for (i = 1; i <= TAF_MAX_CID_NV; i++)
    {
        /* 判断此CID是否有效*/
        if ((pstIfaceDynamicInfo->ulBitCid & 0x01UL << i) == 0)
        {
            continue;
        }

        if ( (pstDynamicInfo[i].bitOpIpv4Valid == VOS_FALSE)
          && (pstDynamicInfo[i].bitOpIpv6Valid == VOS_FALSE))
        {
            continue;
        }

        switch (AT_GetIpv6Capability())
        {
            case AT_IPV6_CAPABILITY_IPV4_ONLY:
                enIpv4Status = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv4Info.enState);

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   "%s: ",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   "%d,%d,,,\"%s\"%s",
                                                   i,
                                                   enIpv4Status,
                                                   aucAtStrIpv4,
                                                   gaucAtCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV6_ONLY:
                enIpv6Status    = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv6Info.enState);

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   "%s: ",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   "%d,%d,,,\"%s\"%s",
                                                   i,
                                                   enIpv6Status,
                                                   aucAtStrIpv6,
                                                   gaucAtCrLf);
                break;

            case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
            case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP:
                enIpv4Status = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv4Info.enState);
                enIpv6Status = AT_NdisGetConnStatus(pstDynamicInfo[i].stIpv6Info.enState);

                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   "%s: ",
                                                   g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   "%d,%d,,,\"%s\"",
                                                   i,
                                                   enIpv4Status,
                                                   aucAtStrIpv4);
                usLength += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr,
                                                   (VOS_CHAR*)pgucAtSndCodeAddr + usLength,
                                                   ",%d,,,\"%s\"%s",
                                                   enIpv6Status,
                                                   aucAtStrIpv6,
                                                   gaucAtCrLf);
                break;

            default:
                break;
        }
    }

    if (usLength >= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf))
    {
        usLength -= (VOS_UINT16)VOS_StrLen((VOS_CHAR *)gaucAtCrLf);
    }

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_PS_ReportSetCgmtuRst(
    VOS_UINT8                           ucIndex,
    VOS_VOID                           *pstEvtInfo
)
{
    TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU    *pstIfaceDynamicInfo = VOS_NULL_PTR;
    TAF_IFACE_DYNAMIC_INFO_STRU            *pstDynamicInfo      = VOS_NULL_PTR;
    VOS_UINT16                              usLength;
    VOS_UINT8                               ucCid;

    ucCid               = TAF_INVALID_CID;
    pstIfaceDynamicInfo = (TAF_IFACE_GET_DYNAMIC_PARA_CNF_STRU *)pstEvtInfo;

    if (AT_GetCidByCidMask(pstIfaceDynamicInfo->ulBitCid, &ucCid) == VOS_ERR)
    {
        AT_WARN_LOG("AT_PS_ReportSetCgmtuRst: ulBitCid is invalid.");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    pstDynamicInfo = AT_GetDynamicInfoBaseAddr(pstIfaceDynamicInfo);

    if ( (pstDynamicInfo[ucCid].bitOpIpv4Valid == VOS_FALSE)
      && (pstDynamicInfo[ucCid].bitOpIpv6Valid == VOS_FALSE))
    {
        AT_WARN_LOG("AT_PS_ReportSetCgmtuRst: bitOpIpv4Valid and bitOpIpv6Valid is FALSE");
        AT_STOP_TIMER_CMD_READY(ucIndex);
        At_FormatResultData(ucIndex, AT_ERROR);
        return VOS_ERR;
    }

    /* 上报查询结果 */
    usLength = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      (VOS_CHAR *)pgucAtSndCodeAddr,
                                      "%s: %d,%d,%d",
                                      g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                      ucCid,
                                      pstDynamicInfo[ucCid].stIpv4Info.usMtu,
                                      pstDynamicInfo[ucCid].stIpv6Info.stRaInfo.ulMtuSize);

    gstAtSendData.usBufLen = usLength;
    AT_STOP_TIMER_CMD_READY(ucIndex);
    At_FormatResultData(ucIndex, AT_OK);

    return VOS_OK;
}


VOS_UINT32 AT_SetRoamPdpTypePara(VOS_UINT8 ucIndex)
{
    TAF_PS_ROAMING_PDP_TYPE_INFO_STRU   stPdpCxtInfo;
    VOS_UINT32                          ulRslt;

    /* 初始化 */
    TAF_MEM_SET_S(&stPdpCxtInfo, sizeof(stPdpCxtInfo),
                  0, sizeof(TAF_PS_ROAMING_PDP_TYPE_INFO_STRU));

    /* 指令类型检查 */
    if (g_stATParseCmd.ucCmdOptType != AT_CMD_OPT_SET_PARA_CMD)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 2)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<CID> */
    stPdpCxtInfo.ucCid                  = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    /* 如果参数只有<CID>, 无需检查其它参数 */
    if (gucAtParaIndex == 1)
    {
        stPdpCxtInfo.ucDefined          = VOS_FALSE;
    }
    else
    {
        /* 不设置ip类型 或类型错误直接返回 */
        if ( (gastAtParaList[1].usParaLen == 0)
          || (gastAtParaList[1].ulParaValue >= TAF_PDP_IPV4V6))
        {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        stPdpCxtInfo.ucDefined          = VOS_TRUE;

        /* 设置<PDP_type> IP:1, IPV6:2, IPV4V6:3*/
        stPdpCxtInfo.enRoamingPdpType   = (VOS_UINT8)(gastAtParaList[1].ulParaValue + 1);

    }

    ulRslt = TAF_PS_SetRoamPdpType(WUEPS_PID_AT,
                                   AT_PS_BuildExClientId(gastAtClientTab[ucIndex].usClientId),
                                   0,
                                   &stPdpCxtInfo);
    /* 执行命令操作 */
    if (ulRslt != VOS_OK)
    {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    gastAtClientTab[ucIndex].CmdCurrentOpt = AT_CMD_ROAMPDPTYPE_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}




