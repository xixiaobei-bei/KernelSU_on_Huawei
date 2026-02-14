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
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "scm_cnf_src.h"
#include "scm_cnf_dst.h"
#include "soc_socp_adapter.h"
#include "diag_port_manager.h"
#include "soft_decode.h"
#include "OmCommonPpm.h"
#include "scm_debug.h"

SCM_INFODATA_STRU           g_stSCMInfoData = {};    /* 用于保存log信息 */

SCM_CODER_SRC_DEBUG_STRU g_astScmIndSrcDebugInfo = {};
SCM_CODER_SRC_DEBUG_STRU g_astScmCnfSrcDebugInfo = {};
extern SCM_CODER_SRC_CFG_STRU      g_astSCMIndCoderSrcCfg;
extern SCM_CODER_SRC_CFG_STRU      g_astSCMCnfCoderSrcCfg;
extern SCM_CODER_DEST_CFG_STRU g_astSCMIndCoderDstCfg;
extern SCM_CODER_DEST_CFG_STRU g_astSCMCnfCoderDstCfg;


void SCM_ChannelInfoShow(void)
{
    scm_printf("ind\n");

    scm_printf("\r\n The Channle 0x%x info is :", g_astSCMIndCoderSrcCfg.enChannelID);

    scm_printf("cnf\n");

    scm_printf("\r\n The Channle 0x%x info is :", g_astSCMCnfCoderSrcCfg.enChannelID);

    return;
}
EXPORT_SYMBOL(SCM_ChannelInfoShow);



void SCM_CoderSrcCHShow(u32 ulCfgNum)
{
    if(SCM_CODER_SRC_LOM_IND == ulCfgNum)
    {
        scm_printf("\r\n CH id         is 0x%x", g_astSCMIndCoderSrcCfg.enChannelID);
        scm_printf("\r\n CH init state is   %d", g_astSCMIndCoderSrcCfg.enInitState);
        scm_printf("\r\n CH type       is   %d", g_astSCMIndCoderSrcCfg.enCHMode);
        scm_printf("\r\n CH Dst        is 0x%x", g_astSCMIndCoderSrcCfg.enDstCHID);
        scm_printf("\r\n CH data type  is   %d", g_astSCMIndCoderSrcCfg.enDataType);
        scm_printf("\r\n CH Level      is   %d", g_astSCMIndCoderSrcCfg.enCHLevel);
        scm_printf("\r\n CH BD VirtAddris 0x%p", g_astSCMIndCoderSrcCfg.pucSrcBuf);
        scm_printf("\r\n CH BD PHYAddr is 0x%p", g_astSCMIndCoderSrcCfg.pucSrcPHY);
        scm_printf("\r\n CH BD BufLen  is 0x%x", g_astSCMIndCoderSrcCfg.ulSrcBufLen);
        scm_printf("\r\n CH RD VirtAddris 0x%p", g_astSCMIndCoderSrcCfg.pucRDBuf);
        scm_printf("\r\n CH RD PHYAddr is 0x%p", g_astSCMIndCoderSrcCfg.pucRDPHY);
        scm_printf("\r\n CH RD BufLen  is 0x%x", g_astSCMIndCoderSrcCfg.ulRDBufLen);

        scm_printf("\r\n CH SEND data   LENGTH          0x%x", g_astScmIndSrcDebugInfo.ulSendDataLen);
        scm_printf("\r\n CH SEND packet NUM             0x%x", g_astScmIndSrcDebugInfo.ulSendPacketNum);
        scm_printf("\r\n CH SEND addr   ERR             0x%x", g_astScmIndSrcDebugInfo.ulSendAddrErr);
        scm_printf("\r\n CH SEND head   ERR             0x%x", g_astScmIndSrcDebugInfo.ulSendHeaderErr);
        scm_printf("\r\n CH SEND write  ERR             0x%x", g_astScmIndSrcDebugInfo.ulSendWriteDoneErr);
        scm_printf("\r\n CH SEND first buff not enough  0x%x", g_astScmIndSrcDebugInfo.ulSendFirstNotEnough);

        scm_printf("\r\n CH SEND get buffer ERR         0x%x", g_astScmIndSrcDebugInfo.ulGetCoderBuffErr);
        scm_printf("\r\n CH SEND get coherent buff ERR  0x%x", g_astScmIndSrcDebugInfo.ulGetCoherentBuffErr);

        scm_printf("\r\n CH SEND fill max buff  len     0x%x", g_astScmIndSrcDebugInfo.ulFillFirstBuffMax);
        scm_printf("\r\n CH SEND fill num               0x%x", g_astScmIndSrcDebugInfo.ulFillFirstBuffNum);
        scm_printf("\r\n CH SEND after fill not enough  0x%x", g_astScmIndSrcDebugInfo.ulAfterFillNotEnough);
        scm_printf("\r\n CH SEND fill not enough        0x%x", g_astScmIndSrcDebugInfo.ulFillNotEnough);
    }
    if(SCM_CODER_SRC_LOM_CNF == ulCfgNum)
    {
        scm_printf("\r\n CH id         is 0x%x", g_astSCMCnfCoderSrcCfg.enChannelID);
        scm_printf("\r\n CH init state is   %d", g_astSCMCnfCoderSrcCfg.enInitState);
        scm_printf("\r\n CH type       is   %d", g_astSCMCnfCoderSrcCfg.enCHMode);
        scm_printf("\r\n CH Dst        is 0x%x", g_astSCMCnfCoderSrcCfg.enDstCHID);
        scm_printf("\r\n CH data type  is   %d", g_astSCMCnfCoderSrcCfg.enDataType);
        scm_printf("\r\n CH Level      is   %d", g_astSCMCnfCoderSrcCfg.enCHLevel);
        scm_printf("\r\n CH BD VirtAddris 0x%p", g_astSCMCnfCoderSrcCfg.pucSrcBuf);
        scm_printf("\r\n CH BD PHYAddr is 0x%p", g_astSCMCnfCoderSrcCfg.pucSrcPHY);
        scm_printf("\r\n CH BD BufLen  is 0x%x", g_astSCMCnfCoderSrcCfg.ulSrcBufLen);
        scm_printf("\r\n CH RD VirtAddris 0x%p", g_astSCMCnfCoderSrcCfg.pucRDBuf);
        scm_printf("\r\n CH RD PHYAddr is 0x%p", g_astSCMCnfCoderSrcCfg.pucRDPHY);
        scm_printf("\r\n CH RD BufLen  is 0x%x", g_astSCMCnfCoderSrcCfg.ulRDBufLen);

        scm_printf("\r\n CH SEND data   LENGTH          0x%x", g_astScmCnfSrcDebugInfo.ulSendDataLen);
        scm_printf("\r\n CH SEND packet NUM             0x%x", g_astScmCnfSrcDebugInfo.ulSendPacketNum);
        scm_printf("\r\n CH SEND addr   ERR             0x%x", g_astScmCnfSrcDebugInfo.ulSendAddrErr);
        scm_printf("\r\n CH SEND head   ERR             0x%x", g_astScmCnfSrcDebugInfo.ulSendHeaderErr);
        scm_printf("\r\n CH SEND write  ERR             0x%x", g_astScmCnfSrcDebugInfo.ulSendWriteDoneErr);
        scm_printf("\r\n CH SEND first buff not enough  0x%x", g_astScmCnfSrcDebugInfo.ulSendFirstNotEnough);

        scm_printf("\r\n CH SEND get buffer ERR         0x%x", g_astScmCnfSrcDebugInfo.ulGetCoderBuffErr);
        scm_printf("\r\n CH SEND get coherent buff ERR  0x%x", g_astScmCnfSrcDebugInfo.ulGetCoherentBuffErr);

        scm_printf("\r\n CH SEND fill max buff  len     0x%x", g_astScmCnfSrcDebugInfo.ulFillFirstBuffMax);
        scm_printf("\r\n CH SEND fill num               0x%x", g_astScmCnfSrcDebugInfo.ulFillFirstBuffNum);
        scm_printf("\r\n CH SEND after fill not enough  0x%x", g_astScmCnfSrcDebugInfo.ulAfterFillNotEnough);
        scm_printf("\r\n CH SEND fill not enough        0x%x", g_astScmCnfSrcDebugInfo.ulFillNotEnough);
    }

    return;
}
EXPORT_SYMBOL(SCM_CoderSrcCHShow);


void SCM_CoderDstCHShow(u32 ulCfgNum)
{
    scm_printf("\r\n CH id         is 0x%x", g_astSCMIndCoderDstCfg.enChannelID);
    scm_printf("\r\n CH init state is   %d", g_astSCMIndCoderDstCfg.enInitState);
    scm_printf("\r\n CH BD VirtAddris 0x%p", g_astSCMIndCoderDstCfg.pucBuf);
    scm_printf("\r\n CH BD PHYAddr is 0x%p", g_astSCMIndCoderDstCfg.pucBufPHY);
    scm_printf("\r\n CH BD BufLen  is 0x%x", g_astSCMIndCoderDstCfg.ulBufLen);
    scm_printf("\r\n CH threshold  is 0x%x", g_astSCMIndCoderDstCfg.ulThreshold);
    scm_printf("\r\n CH CB func    is 0x%p", g_astSCMIndCoderDstCfg.pfunc);

    scm_printf("\r\n CH id         is 0x%x", g_astSCMCnfCoderDstCfg.enChannelID);
    scm_printf("\r\n CH init state is   %d", g_astSCMCnfCoderDstCfg.enInitState);
    scm_printf("\r\n CH BD VirtAddris 0x%p", g_astSCMCnfCoderDstCfg.pucBuf);
    scm_printf("\r\n CH BD PHYAddr is 0x%p", g_astSCMCnfCoderDstCfg.pucBufPHY);
    scm_printf("\r\n CH BD BufLen  is 0x%x", g_astSCMCnfCoderDstCfg.ulBufLen);
    scm_printf("\r\n CH threshold  is 0x%x", g_astSCMCnfCoderDstCfg.ulThreshold);
    scm_printf("\r\n CH CB func    is 0x%p", g_astSCMCnfCoderDstCfg.pfunc);

    return;
}
EXPORT_SYMBOL(SCM_CoderDstCHShow);


SCM_INFODATA_STRU* SCM_GetDebugLogInfo(void)
{
    return &g_stSCMInfoData;
}
EXPORT_SYMBOL(SCM_GetDebugLogInfo);


u32 SCM_GetDebugLogInfoLen(void)
{
    return (u32)sizeof(g_stSCMInfoData);
}
EXPORT_SYMBOL(SCM_GetDebugLogInfoLen);


void SCM_SocpSendDataToUDISucc(
    SOCP_CODER_DST_ENUM_U32             enChanID,
    CPM_PHY_PORT_ENUM_UINT32            enPhyport,
    OM_SOCP_CHANNEL_DEBUG_INFO         *pstDebugInfo,
    u32                         *pulSendDataLen
)
{
    if ((SOCP_CODER_DST_OM_CNF == enChanID) && (CPM_CFG_PORT == enPhyport))
    {
        if ((0 == g_stUsbCfgPseudoSync.ulLen) || (*pulSendDataLen != g_stUsbCfgPseudoSync.ulLen))
        {
            pstDebugInfo->ulUSBSendCBAbnormalNum++;
            pstDebugInfo->ulUSBSendCBAbnormalLen += *pulSendDataLen;
        }

        *pulSendDataLen = g_stUsbCfgPseudoSync.ulLen;
    }
    else if ((SOCP_CODER_DST_OM_IND == enChanID) && (CPM_IND_PORT == enPhyport))
    {
        if ((0 == g_stUsbIndPseudoSync.ulLen) || (*pulSendDataLen != g_stUsbIndPseudoSync.ulLen))
        {
            pstDebugInfo->ulUSBSendCBAbnormalNum++;
            pstDebugInfo->ulUSBSendCBAbnormalLen += *pulSendDataLen;
        }

        *pulSendDataLen = g_stUsbIndPseudoSync.ulLen;
    }
    else
    {
        ;
    }

    return;
}


