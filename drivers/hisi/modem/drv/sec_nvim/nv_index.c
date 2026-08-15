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


#include <bsp_nvim.h>
#include "nv_comm.h"
#include "nv_ctrl.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "bsp_dump.h"

#define THIS_MODU mod_nv

/************************************************************************
 函 数 名  : nv_get_item_mdmbase
 功能描述  : 获取当前NV的在NV内存中的相对基地址
 输入参数  : modem_id: modem 编号
             item_info: NV reference data,包含NV的各种信息
             ctrl_info: NV 内存的信息头地址，即NV镜像在NV内存中的相对基地址
 输出参数  : 无
 返 回 值  :
*************************************************************************/
u8* nv_get_item_mdmbase(u32 modem_id, nv_item_info_s *item_info, nv_ctrl_info_s *ctrl_info)
{
    nv_file_info_s * file_info = (nv_file_info_s *)((unsigned long)ctrl_info + ctrl_info->file_offset);

	if((item_info->file_id > ctrl_info->file_num) || (item_info->file_id == 0))
	{
		nv_record("par err!fileid=%d,file_num=%d.\n", item_info->file_id, ctrl_info->file_num );
		return NULL;
	}

	if((modem_id > item_info->modem_num) || (modem_id == 0))
	{
		nv_record("par err!modeid=%d,mode_num=%d.\n", modem_id, item_info->modem_num);
		return NULL;
	}

    if((item_info->nv_off[modem_id - 1] + item_info->nv_len) > file_info[item_info->file_id - 1].file_size)
    {
        nv_printf("offset is invalid!offset=0x%x, file_size=0x%x.\n",
                item_info->nv_off[modem_id - 1],file_info[item_info->file_id - 1].file_size);
        return NULL;
    }

    return ((u8 *)((unsigned long)ctrl_info + file_info[item_info->file_id - 1].file_offset + item_info->nv_off[modem_id - 1]));
}

/************************************************************************
 函 数 名  : nv_search_iteminfo
 功能描述  : 查找需要的NV
 输入参数  : itemid: NV ID
             ctrl_info: NV 内存的信息头地址，即NV镜像在NV内存中的相对基地址
 输出参数  : item_info: NV reference data,包含NV的各种信息
 返 回 值  :
*************************************************************************/
u32 nv_search_iteminfo(u32 itemid, nv_ctrl_info_s* ctrl_info, nv_item_info_s** item_info)
{
    u32 low;
    u32 high;
    u32 mid;
    u32 offset;
    nv_item_info_s *item_table;

    offset = (u32)(NV_GLOBAL_CTRL_INFO_SIZE + ctrl_info->file_size);
    item_table = (nv_item_info_s*)((unsigned long)ctrl_info + offset);

    low  = 1;
    high = ctrl_info->ref_count;
    while(low <= high)
    {
        mid = (low+high)/2;

        *item_info = &item_table[mid-1];

        if(itemid < (*item_info)->itemid)
        {
            high = mid-1;
        }
        else if(itemid > (*item_info)->itemid)
        {
            low = mid+1;
        }
        else
        {
            return NV_OK;
        }
    }

    *item_info = NULL;

    return BSP_ERR_NV_NO_THIS_ID;
}

EXPORT_SYMBOL(nv_get_item_mdmbase);
EXPORT_SYMBOL(nv_search_iteminfo);



