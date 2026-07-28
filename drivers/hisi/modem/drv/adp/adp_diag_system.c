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


#include <mdrv.h>
#include <scm_ind_src.h>
#include <scm_cnf_src.h>
#include <scm_ind_dst.h>
#include <scm_debug.h>
#include <diag_system_debug.h>
#include <OmCommonPpm.h>
#include <scm_init.h>
#include <scm_ind_dst.h>
#include <diag_port_manager.h>
#include <OmVcomPpm.h>
#include <scm_common.h>
#include <OmSocketPpm.h>
#include <soft_cfg.h>


void mdrv_diag_PTR(DIAG_PTR_ID_ENUM enType, u32 paraMark, u32 para0, u32 para1)
{
    diag_PTR(enType, paraMark, para0, para1);
}
void mdrv_PPM_RegDisconnectCb(PPM_DisconnectTLPortFuc cb)
{
    PPM_RegDisconnectCb(cb);
}
unsigned int mdrv_GetThrputInfo(DIAG_THRPUT_ID_ENUM type)
{
    return DIAG_GetThrputInfo(type);
}

void mdrv_scm_reg_ind_coder_dst_send_fuc(void)
{
    scm_reg_ind_coder_dst_send_fuc();
}
unsigned int mdrv_PPM_LogPortSwitch(u32  ulPhyPort, unsigned int ulEffect)
{
    return PPM_LogPortSwitch(ulPhyPort, (bool)ulEffect);
}
unsigned int mdrv_PPM_QueryLogPort(unsigned int  *pulLogPort)
{
    return PPM_QueryLogPort(pulLogPort);
}
unsigned int mdrv_CPM_ComSend(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, unsigned char *pucVirData, unsigned char *pucPHYData, unsigned int ulLen)
{
    return CPM_ComSend(enLogicPort, pucVirData, pucPHYData, ulLen);
}
void mdrv_CPM_LogicRcvReg(CPM_LOGIC_PORT_ENUM_UINT32 enLogicPort, CPM_RCV_FUNC pRcvFunc)
{
    PPM_SocketRevFunReg(enLogicPort, pRcvFunc);
}
unsigned int mdrv_SCM_RegDecoderDestProc(SOCP_DECODER_DST_ENUM_U32 enChanlID, SCM_DECODERDESTFUCN func)
{
    return SCM_RegDecoderDestProc(enChanlID, func);
}
void mdrv_scm_ind_dst_read_cb(void)
{
    scm_ind_dst_read_cb();
}
void mdrv_diag_get_dst_mntn_info(DIAG_MNTN_DST_INFO_STRU * dst_mntn)
{
    diag_debug_get_dst_mntn_info(dst_mntn);
}
void mdrv_diag_reset_dst_mntn_info(void)
{
    diag_reset_dst_mntn_info();
}
void  mdrv_scm_set_power_on_log(void)
{
    scm_set_power_on_log();
}

unsigned int mdrv_diag_shared_mem_write(unsigned int eType, unsigned int len, char *pData)
{
    return diag_shared_mem_write(eType, len, pData);
}

unsigned int mdrv_diag_shared_mem_read(unsigned int eType)
{
    return diag_shared_mem_read(eType);
}
unsigned long mdrv_scm_ind_src_phy_to_virt(u8 * phyAddr)
{
    return scm_ind_src_phy_to_virt(phyAddr);
}
unsigned long mdrv_scm_cnf_src_phy_to_virt(u8 * phyAddr)
{
    return scm_cnf_src_phy_to_virt(phyAddr);
}


