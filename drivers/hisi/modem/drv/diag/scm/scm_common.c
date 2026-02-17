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
#include <linux/of_platform.h>
#include "scm_debug.h"
#include "scm_ind_src.h"
#include "scm_ind_dst.h"
#include "scm_cnf_src.h"
#include "scm_cnf_dst.h"
#include "scm_common.h"
#include <securec.h>


u64 g_dma_mask = (u64)(-1);
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
    void                           *pVirtAdd;
    dma_addr_t                          ulAddress = 0;

    struct device                       dev;

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

    memset_s(&dev, sizeof(dev), 0, sizeof(dev));
    dma_set_mask_and_coherent(&dev, g_dma_mask);	
    of_dma_configure(&dev, NULL);
    pVirtAdd = dma_alloc_coherent(&dev, ulSize, &ulAddress, GFP_KERNEL);

    *pulRealAddr = (unsigned long)ulAddress;

    return pVirtAdd;
}



void scm_FlushCpuWriteBuf(void)
{
    __asm(" DSB sy ");
    __asm(" ISB sy ");

    return ;
}



unsigned long scm_UncacheMemPhyToVirt(u8 *pucCurPhyAddr, u8 *pucPhyStart, u8 *pucVirtStart, u32 ulBufLen)
{
    if((pucCurPhyAddr < pucPhyStart) || (pucCurPhyAddr >= (pucPhyStart+ulBufLen)))
    {
        (void)scm_printf("\r\n VOS_UncacheMemPhyToVirt: The PHY Addr 0x%p, pucPhyStart: 0x%p, pucVirtStart: 0x%p, ulBufLen: 0x%x.\r\n",
                    pucCurPhyAddr,
                    pucPhyStart,
                    pucVirtStart,
                    ulBufLen);

        return (unsigned long)NULL;
    }

    return (unsigned long)((pucCurPhyAddr - pucPhyStart) + pucVirtStart);
}


