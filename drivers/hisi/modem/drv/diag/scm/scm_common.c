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
#include <linux/of_platform.h>
#include <mdrv.h>
#include "scm_debug.h"
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "scm_cnf_src.h"
#include "scm_cnf_dst.h"
#include "scm_common.h"
#include <securec.h>
#include <bsp_shared_ddr.h>
#include "diag_system_debug.h"

extern struct platform_device *modem_socp_pdev;
/*****************************************************************************
 Function   : VOS_UnCacheMemAlloc
 Description: allocate uncached memory.
 Input      : size & real address
            :
 Return     : null or vir address
 Other      :
 *****************************************************************************/
void *scm_UnCacheMemAlloc(u32 ulSize, unsigned long *pulRealAddr)
{
    void                           *pVirtAdd = NULL;
    dma_addr_t                      ulAddress = 0;

    if ( 0 == ulSize )
    {
        return NULL;
    }

    if ( NULL == pulRealAddr )
    {
        return NULL;
    }

    /* keep lint happy */
    *pulRealAddr = 0;
    pVirtAdd     = 0;


    pVirtAdd = dma_alloc_coherent(&modem_socp_pdev->dev, ulSize, &ulAddress, GFP_KERNEL);

    *pulRealAddr = (unsigned long)ulAddress;

    return pVirtAdd;
}


unsigned long scm_UncacheMemPhyToVirt(u8 *pucCurPhyAddr, u8 *pucPhyStart, u8 *pucVirtStart, u32 ulBufLen)
{
    if((pucCurPhyAddr < pucPhyStart) || (pucCurPhyAddr >= (pucPhyStart+ulBufLen)))
    {
        diag_error("Phy to Virt error\n");

        return (unsigned long)NULL;
    }

    return (unsigned long)((pucCurPhyAddr - pucPhyStart) + pucVirtStart);
}

/*****************************************************************************
 函 数 名  : diag_shared_mem_write
 功能描述  : diag子系统对共享内存写接口
 输入参数  : eType:  操作共享内存的类型，如开机A核log标志
             len:    写操作len不允许为0
             pData:  写入共享内存的数据

 输出参数  : 无
 返 回 值  : 错误码

**************************************************************************** */
u32 diag_shared_mem_write(u32 eType, u32 len, char *pData)
{
    u8 *ptr;

    switch(eType)
    {
        case POWER_ON_LOG_A:
            if((pData == NULL) || (len == 0))
            {
                diag_error("para error, len=0x%x\n", len);
                return ERR_MSP_FAILURE;
            }
            ptr = (u8 *)(((unsigned long)SHM_BASE_ADDR)+SHM_OFFSET_DIAG_POWER_ON_LOG);
            memcpy_s(&(((SHM_POWER_ON_LOG_FLAG_STRU *)ptr)->cPowerOnlogA),
                sizeof(((SHM_POWER_ON_LOG_FLAG_STRU *)ptr)->cPowerOnlogA), pData, len);
            return ERR_MSP_SUCCESS;

        case DS_DATA_BUFFER_STATE:
            if((pData == NULL) || (len == 0))
            {
                diag_error("para error, len=0x%x\n", len);
                return ERR_MSP_FAILURE;
            }
            ptr = (u8 *)(((unsigned long)SHM_BASE_ADDR)+SHM_OFFSET_DIAG_POWER_ON_LOG);
            memcpy_s(&(((SHM_POWER_ON_LOG_FLAG_STRU *)ptr)->cDsSocpBuffer),
                sizeof(((SHM_POWER_ON_LOG_FLAG_STRU *)ptr)->cDsSocpBuffer), pData, len);
            return ERR_MSP_SUCCESS;

        default:
            return ERR_MSP_INVALID_ID;
    }
}

/*****************************************************************************
 函 数 名  : diag_shared_mem_read
 功能描述  : diag子系统对共享内存读接口
 输入参数  : eType:  操作共享内存的类型，如开机A核log标志

 输出参数  : 无
 返 回 值  : 共享内存中的数据

**************************************************************************** */
u32 diag_shared_mem_read(u32 eType)
{
    u8 *ptr;
    u32 val;

    switch(eType)
    {
        case POWER_ON_LOG_A:
                ptr = (u8 *)(SHM_BASE_ADDR+SHM_OFFSET_DIAG_POWER_ON_LOG);
                val = ((SHM_POWER_ON_LOG_FLAG_STRU *)ptr)->cPowerOnlogA;
                return val;

        case DS_DATA_BUFFER_STATE:
                ptr = (u8 *)(((unsigned long)SHM_BASE_ADDR)+SHM_OFFSET_DIAG_POWER_ON_LOG);
                val = (u32)(((SHM_POWER_ON_LOG_FLAG_STRU *)ptr)->cDsSocpBuffer);//lint !e571
                return val;                
        default:
            return ERR_MSP_INVALID_ID;
    }
}



