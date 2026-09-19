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
#include <icc_core.h>


#undef THIS_MODU
#define THIS_MODU mod_icc
extern struct icc_control g_icc_ctrl;

#define ICC_DEFAULT_SUB_CHANNEL   (0)


struct bsp_icc_cb_info 
{
        icc_read_cb         read_cb;
		unsigned int        channel;
};

struct icc_channel_map_logic2phy
{
    u32    logic_id;
    u32    channel_id;
};

static struct icc_channel_map_logic2phy g_icc_channel_map_logic2phy[] = {
    [MDRV_ICC_VT_VOIP] =                  {MDRV_ICC_VT_VOIP,                 (ICC_CHN_VT_VOIP            <<16)  | VT_VOIP_FUNC_VT_VOIP},
    [MDRV_ICC_RCS_SERV]=                {MDRV_ICC_RCS_SERV,              (ICC_CHN_VT_VOIP            <<16)  | VT_VOIP_FUNC_RCS_SERV},
    [MDRV_ICC_GUOM0]   =                 {MDRV_ICC_GUOM0,                   (ICC_CHN_GUOM0             <<16)  | ICC_DEFAULT_SUB_CHANNEL},
    [MDRV_ICC_GUOM1]   =                 {MDRV_ICC_GUOM1,                   (ICC_CHN_GUOM1             <<16)  | ICC_DEFAULT_SUB_CHANNEL},
    [MDRV_ICC_GUOM2]   =                 {MDRV_ICC_GUOM2,                   (ICC_CHN_GUOM2             <<16)  | ICC_DEFAULT_SUB_CHANNEL},
    [MDRV_ICC_GUOM3]   =                 {MDRV_ICC_GUOM3,                   (ICC_CHN_GUOM3             <<16)  | ICC_DEFAULT_SUB_CHANNEL},
    [MDRV_ICC_GUOM4]   =                 {MDRV_ICC_GUOM4,                   (ICC_CHN_GUOM4             <<16)  | ICC_DEFAULT_SUB_CHANNEL},
    [MDRV_ICC_GUOM5]   =                 {MDRV_ICC_GUOM5,                   (ICC_CHN_GUOM5             <<16)  | ICC_DEFAULT_SUB_CHANNEL},
    [MDRV_ICC_NRCCPU_APCPU_OSA] = {MDRV_ICC_NRCCPU_APCPU_OSA, (ICC_CHN_NRCCPU_APCPU_OSA  <<16)  | ICC_DEFAULT_SUB_CHANNEL},
};

s32 icc_read_cb_wraper(u32 channel_id , u32 len, void* context);
s32 icc_write_cb_wraper(u32 real_channel_id , void* context);

s32 icc_read_cb_wraper(u32 channel_id , u32 len, void* context)
{
    struct bsp_icc_cb_info *pICC_cb_info;
	pICC_cb_info = context;

	if (pICC_cb_info)
		return (s32)(pICC_cb_info->read_cb(pICC_cb_info->channel, (int)len));

	return ICC_OK;
}

s32 icc_write_cb_wraper(u32 real_channel_id , void* context)
{
	return ICC_OK;
}

/* 逻辑通道到物理通道的转换 */
int icc_channel_logic2phy(u32 u32ChanId, u32 *channel_id)
{
    int i = 0;
    int loop_cnt = sizeof(g_icc_channel_map_logic2phy)/sizeof(struct icc_channel_map_logic2phy);

    for (i = 0; i < loop_cnt; i++)
    {
        if (u32ChanId == g_icc_channel_map_logic2phy[i].logic_id)
        {
            *channel_id = g_icc_channel_map_logic2phy[i].channel_id;
            break;
        }
    }

    if (i >= loop_cnt)
    {
        icc_print_error("invalid logic id 0x%x\n", u32ChanId);
        return MDRV_ERROR;
    }

    return MDRV_OK;
}

int BSP_ICC_Open(unsigned int u32ChanId, ICC_CHAN_ATTR_S *pChanAttr)
{
    u32 channel_id = 0;
    u32 channel_index = 0;
    BSP_S32 ret = ICC_OK;
    struct bsp_icc_cb_info *pICC_cb_info;

    if (icc_channel_logic2phy(u32ChanId, &channel_id))
    {
        icc_print_error("icc_channel_logic2phy err logic id 0x%x\n", u32ChanId);
        goto out; /*lint !e801 */
    }

    channel_index = channel_id >> 16;
    if ((ICC_CHN_ID_MAX <= channel_index) || (!g_icc_ctrl.channels[channel_index]))
    {
        icc_print_error("invalid channel_index[%d]\n", channel_index);
        goto out; /*lint !e801 */
    }

    /* coverity[REVERSE_INULL] */
    if(!pChanAttr)
    {
        icc_print_error("pChanAttr is null!\n");
        goto out; /*lint !e801 */
    }
    else if(pChanAttr->u32FIFOOutSize != pChanAttr->u32FIFOInSize)
    {
        icc_print_error("invalid param u32ChanId[%d],pChanAttr[0x%p],fifo_in[0x%x],fifo_out[0x%x]\n",
            channel_index, pChanAttr, pChanAttr->u32FIFOInSize, pChanAttr->u32FIFOOutSize);
        goto out; /*lint !e801 */
    }

    if(pChanAttr->u32FIFOInSize > g_icc_ctrl.channels[channel_index]->fifo_send->size)/*lint !e574 */
    {
        icc_print_error("channel_id 0x%x user fifo_size(0x%x) > fifo_size(0x%x) defined in icc\n",
                        channel_id,
                        pChanAttr->u32FIFOInSize, g_icc_ctrl.channels[channel_index]->fifo_send->size);
        goto out; /*lint !e801 */
    }

    pICC_cb_info = osl_malloc(sizeof(struct bsp_icc_cb_info));
    if (pICC_cb_info == NULL) {
        icc_print_error("Fail to malloc memory \n");
        return ICC_MALLOC_MEM_FAIL;
    }

    pICC_cb_info->read_cb = pChanAttr->read_cb;
    pICC_cb_info->channel = u32ChanId;
    /* coverity[leaked_storage] */
    ret =  (BSP_S32)bsp_icc_event_register(channel_id, icc_read_cb_wraper, pICC_cb_info, icc_write_cb_wraper, (void*)pChanAttr->write_cb); /*lint !e611 */
    if(ret == ICC_OK)  
    {
    	return ret; /*lint !e429 */
    }
    else  
    {
    	icc_print_error(" failed to  bsp_icc_event_register ret=%d\n", ret);
	osl_free((void*)pICC_cb_info);
    }

out:
    return ICC_INVALID_PARA;
}

int BSP_ICC_Read(unsigned int u32ChanId, unsigned char* pData, int s32Size)
{
	u32 channel_id = 0;
    u32 channel_index = 0;

    if (icc_channel_logic2phy(u32ChanId, &channel_id))
    {
        icc_print_error("icc_channel_logic2phy err logic id 0x%x\n", u32ChanId);
        return ICC_INVALID_PARA;
    }

    channel_index = channel_id >> 16;
	
	if(!pData || channel_index >= ICC_CHN_ID_MAX)
	{
		icc_print_error("invalid param[%d], pData[0x%p]\n", channel_index, pData);
		return ICC_INVALID_PARA;
	}

	return (BSP_S32)bsp_icc_read(channel_id, (u8*)pData, (u32)s32Size);
}


int BSP_ICC_Write(unsigned int u32ChanId, unsigned char* pData, int s32Size)
{
    u32 channel_id = 0;
    u32 channel_index = 0;
    u32 dst_core;

    if (icc_channel_logic2phy(u32ChanId, &channel_id))
    {
        icc_print_error("icc_channel_logic2phy err logic id 0x%x\n", u32ChanId);
        return ICC_INVALID_PARA;
    }

    channel_index = channel_id >> 16;

    if(!pData || channel_index >= ICC_CHN_ID_MAX)
    {
        icc_print_error("invalid param[%d], pData[0x%p]\n", channel_index, pData);
        return ICC_INVALID_PARA;
    }
	
    if(u32ChanId == MDRV_ICC_NRCCPU_APCPU_OSA)
    {
        dst_core = ICC_CPU_NRCCPU;
    } 
    else
    {
        dst_core = ICC_SEND_CPU;
    }
    return (BSP_S32)bsp_icc_send(dst_core, channel_id, (u8*)pData, (u32)s32Size);
}

int mdrv_icc_register_resume_cb(unsigned int u32ChanId, FUNCPTR_1 debug_routine, int param)
{
    u32 channel_id = 0;

    if (icc_channel_logic2phy(u32ChanId, &channel_id))
    {
        icc_print_error("icc_channel_logic2phy err logic id 0x%x\n", u32ChanId);
        return ICC_INVALID_PARA;
    }

	return bsp_icc_debug_register(channel_id,debug_routine,param);
}

/*lint -save -e762 */
int mdrv_icc_open(unsigned int u32ChanId, ICC_CHAN_ATTR_S *pChanAttr)  __attribute__((alias("BSP_ICC_Open")));
int mdrv_icc_read(unsigned int u32ChanId, unsigned char* pData, int s32Size)   __attribute__((alias("BSP_ICC_Read")));
int mdrv_icc_write(unsigned int u32ChanId, unsigned char* pData, int s32Size)  __attribute__((alias("BSP_ICC_Write")));
/*lint -restore +e762 */


