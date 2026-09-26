/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
/*
 * 1 Include HeadFile
 */

#include "ndserver_at_proc.h"
#include "vos.h"
#include "securec.h"
#include "at_ndserver_interface.h"
#include "ps_common_def.h"
#include "ps_type_def.h"
#include "ndis_entity.h"
#include "ps_iface_global_def.h"
#include "ip_nd_server.h"
#include "ip_comm.h"
#include "ip_ipm_global.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERATPROC_C
/*lint +e767*/

/* 统计信息 */


/*
 * 2 Declare the Global Variable
 */
STATIC VOS_UINT32 NDSERVER_RecvMsgParaCheck(VOS_UINT32 length, const NDSERVER_MsgTypeStruLen *msgStruLen,
                                       VOS_UINT32 msgIdNum, VOS_UINT32 msgId)
{
    VOS_UINT32  msgNumFloop;
    for (msgNumFloop = 0; msgNumFloop < msgIdNum; msgNumFloop++) {
        if (msgId == msgStruLen[msgNumFloop].msgId) {
            if (length < (msgStruLen[msgNumFloop].msTypeLenth - VOS_MSG_HEAD_LENGTH)) {
                return PS_FAIL;
            }
            return PS_SUCC;
        }
    }
    return PS_SUCC;
}

STATIC VOS_UINT32 NDSERVER_ChkRabIdValid(VOS_UINT8 ifaceId)
{
   /* B5000及以后的版本下，Ndis的索引为IFACE ID */
   if (ifaceId >= PS_IFACE_ID_BUTT) {
       return PS_FAIL;
   }

   return PS_SUCC;
}

STATIC VOS_VOID NDSERVER_PPPIpv6CfgToPdnInfoCfg(const AT_NDSERVER_PPPIpv6CfgInd *ipv6CfgInfo,
                                                AT_NDIS_PdnInfoCfgReq *ndisPdnInfo)
{
    errno_t rlt;
    (VOS_VOID)memset_s((VOS_UINT8 *)ndisPdnInfo, sizeof(AT_NDIS_PdnInfoCfgReq), 0,
                       sizeof(AT_NDIS_PdnInfoCfgReq));

    ndisPdnInfo->msgId          = ipv6CfgInfo->msgId;
    ndisPdnInfo->opIpv4PdnInfo  = PS_FALSE;
    ndisPdnInfo->opIpv6PdnInfo  = PS_TRUE;
    ndisPdnInfo->modemId        = 0;
    ndisPdnInfo->rabId          = ipv6CfgInfo->ifaceId;
    ndisPdnInfo->handle         = 0;

    rlt = memcpy_s(&ndisPdnInfo->ipv6PdnInfo, sizeof(AT_NDIS_Ipv6PdnInfo),
                   &ipv6CfgInfo->ipv6Info, sizeof(AT_NDSERVER_Ipv6PdnInfo));
    IP_CHK_SEC_RETURN_NO_VAL((rlt != EOK));

    return;
}

/*
 * 功能描述: PDN IPV6地址信息检查
 * 修改历史:
 *  1.日    期: 2011年12月11日
 *    修改内容: 新生成函数
 *  2.日    期: 2013年1月15日
 *    修改内容: DSDA开发，使用ExRabId来映射NdSer实体
 */
VOS_UINT32 NdSer_CheckIpv6PdnInfo(const AT_NDIS_Ipv6PdnInfo *ipv6PdnInfo)
{
    VOS_UINT32        prefixIndex;
    ND_IP_Ipv6Prefix *ipv6Prefix = IP_NULL_PTR;

    if (ipv6PdnInfo->prefixNum >= AT_NDIS_MAX_PREFIX_NUM_IN_RA) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "NdSer_CheckIpv6PdnInfo: Invalid IPV6 PrefixNum!",
                        ipv6PdnInfo->prefixNum);
        return IP_FAIL;
    }

    /* 遍历前缀列表，查找A标识为1，前缀长度为64的前缀，prefixLen单位为bit */
    for (prefixIndex = IP_NULL; prefixIndex < ipv6PdnInfo->prefixNum; prefixIndex++) {
        ipv6Prefix = (ND_IP_Ipv6Prefix *)&ipv6PdnInfo->prefixList[prefixIndex];
        if ((ipv6Prefix->prefixAValue == PS_TRUE) &&
            ((ND_IP_IPV6_ADDR_LENGTH - ND_IP_IPV6_IFID_LENGTH) * 8 == ipv6Prefix->prefixLen) && /* 8:bit */
            (!IP_IPV6_64BITPREFIX_EQUAL_ZERO(ipv6Prefix->prefix)) &&
            (!IP_IPV6_IS_LINKLOCAL_ADDR(ipv6Prefix->prefix)) &&
            (!IP_IPV6_IS_MULTICAST_ADDR(ipv6Prefix->prefix))) {
            return PS_SUCC;
        }
    }

    return PS_FAIL;
}

STATIC VOS_VOID NDSERVER_PPPIpv6CfgProc(const AT_NDSERVER_PPPIpv6CfgInd *ipv6CfgInfo)
{
   VOS_UINT8                     ifaceId;
   AT_NDIS_PdnInfoCfgReq         pdnInfo = {0};

   ifaceId = ipv6CfgInfo->ifaceId;
   /* ifaceId参数范围有效性检查。若检查失败，则直接向AT回复配置失败 */
   if (NDSERVER_ChkRabIdValid(ifaceId) != PS_SUCC) {
       IPND_ERROR_LOG(NDIS_TASK_PID, "NDSERVER_PPPIpv6CfgProc,  NDSERVER_ChkRabIdValid fail!");
       return;
   }

   NDSERVER_PPPIpv6CfgToPdnInfoCfg(ipv6CfgInfo, &pdnInfo);

   if (NdSer_CheckIpv6PdnInfo(&(pdnInfo.ipv6PdnInfo)) != PS_SUCC) {
       IPND_ERROR_LOG(NDIS_TASK_PID, "NDSERVER_PPPIpv6CfgProc,  NdSer_CheckIpv6PdnInfo fail!");
       return;
   }

   /* 配置IPV6地址信息 */
   NdSer_Ipv6PdnInfoCfg(ifaceId, &(pdnInfo.ipv6PdnInfo));

   return;
}

STATIC VOS_VOID NDSERVER_PPPIpv6RelProc(const AT_NDSERVER_PPPIpv6RelInd *ipv6RelInfo)
{
   VOS_UINT8            ifaceId;

   ifaceId = ipv6RelInfo->ifaceId;
   /* ifaceId参数范围有效性检查。若检查失败，则直接向AT回复配置失败 */
   if (NDSERVER_ChkRabIdValid(ifaceId) != PS_SUCC) {
       IPND_ERROR_LOG(NDIS_TASK_PID, "NDSERVER_PPPIpv6RelProc,  NDSERVER_ChkRabIdValid fail!");
       return;
   }

   NdSer_Ipv6PdnRel(ifaceId);

   return;
}

/*
 * 修改历史:
 * 1.日    期: 2020年7月20日
 *   修改内容: 新生成函数
 */
VOS_VOID NDSERVER_AtMsgProc(const MsgBlock *msgBlock)
{
    AT_NDSERVER_MsgIdUint32 msgId;

    NDSERVER_MsgTypeStruLen msgStruLen[] = {
        { ID_AT_NDSERVER_PPP_IPV6_CFG_IND, sizeof(AT_NDSERVER_PPPIpv6CfgInd) },
        { ID_AT_NDSERVER_PPP_IPV6_REL_IND, sizeof(AT_NDSERVER_PPPIpv6RelInd) },
    };
    VOS_UINT32 msgIdNum = sizeof(msgStruLen) / sizeof(NDSERVER_MsgTypeStruLen);
    COVERITY_TAINTED_SET(msgBlock->value); /* 鹰眼插桩 */

    /* 长度异常保护 */
    if (sizeof(MSG_Header) - VOS_MSG_HEAD_LENGTH > msgBlock->ulLength) {
        IPND_ERROR_LOG1(NDIS_TASK_PID, "NDSERVER_AtMsgProc: input msg length less than struc MSG_Header",
                        msgBlock->ulLength);
        return;
    }

    msgId = ((MSG_Header *)(VOS_VOID *)msgBlock)->msgName;
    if (NDSERVER_RecvMsgParaCheck(msgBlock->ulLength, msgStruLen, msgIdNum, msgId) == PS_FAIL) {
        IPND_ERROR_LOG1(NDIS_TASK_PID, "NDSERVER_AtMsgProc: input msg length less than struc MSG_Header",
                        msgId);
        return;
    }
    switch (msgId) {
        case ID_AT_NDSERVER_PPP_IPV6_CFG_IND:
            NDSERVER_PPPIpv6CfgProc((AT_NDSERVER_PPPIpv6CfgInd *)(VOS_VOID *)msgBlock);
            break;

        case ID_AT_NDSERVER_PPP_IPV6_REL_IND:
            NDSERVER_PPPIpv6RelProc((AT_NDSERVER_PPPIpv6RelInd *)(VOS_VOID *)msgBlock);
            break;

        default:
            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "NDSERVER_AtMsgProc:Error Msg!");
            break;
    }

    return;
}




