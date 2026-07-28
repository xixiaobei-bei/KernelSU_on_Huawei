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

#include <stdarg.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/clk.h>
#include <linux/rtc.h>
#include <product_config.h>
#include <bsp_slice.h>
#include <bsp_blk.h>
#include <product_nv_id.h>
#include <product_nv_def.h>
#include <securec.h>
#include <mdrv.h>
#include "nv_comm.h"
#include "nv_file.h"
#include "nv_ctrl.h"
#include "nv_debug.h"
#include "nv_index.h"
#include "nv_partition_img.h"
#include "nv_partition_bakup.h"
#include "bsp_dump.h"
#include "nv_debug.h"
#include "nv_msg.h"

#define THIS_MODU mod_nv

struct nv_global_ctrl_info_stru  g_nv_ctrl = {};
struct nv_global_ctrl_stru * g_flash_emmc_info_ptr = NULL;

/*****************************************************************************
 函 数 名  : nv_ipc_sem_take
 功能描述  : take核间互斥信号量的接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_ipc_sem_take(u32 sem, u32 timeout)
{
    u32 start;
    u32 end;

    osl_sem_down(&g_nv_ctrl.rw_sem);
    start = bsp_get_slice_value();
    if(bsp_ipc_sem_take(sem, (int)timeout))
    {
        nv_record("[0x%x] 0x%x take ipc sem fail\n", bsp_get_slice_value(), start);
        return BSP_ERR_NV_TIME_OUT_ERR;
    }
    end = bsp_get_slice_value();
    nv_debug_record_delta_time(NV_DEBUG_DELTA_GET_IPC, start, end);
    return NV_OK;
}

/*****************************************************************************
 函 数 名  : nv_ipc_sem_take
 功能描述  : give核间互斥信号量的接口
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
void nv_ipc_sem_give(u32 sem)
{
    (void)bsp_ipc_sem_give(sem);
    osl_sem_up(&g_nv_ctrl.rw_sem);
    return;
}

/*****************************************************************************
 函 数 名  : nv_create_flag_file
 功能描述  : 创建文件标志
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
void nv_create_flag_file(const s8* path)
{
    FILE* fp;

    if(!mdrv_file_access(path,0))
        return;
    fp = mdrv_file_open((char*)path, (char*)NV_FILE_WRITE);

    if(fp){
        mdrv_file_close(fp);
        return;
    }
    else
        return;
}

/*****************************************************************************
 函 数 名  : nv_delete_flag_file
 功能描述  : 删除文件标志
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
void nv_delete_flag_file(const s8* path)
{
    if(mdrv_file_access((char*)path,0))
        return;
    else
        mdrv_file_remove((char*)path);
}

/*****************************************************************************
 函 数 名  : nv_flag_file_isExist
 功能描述  : 检查文件标志是否存在
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
bool nv_flag_file_isExist(const s8* path)
{
    return (mdrv_file_access((char*)path,0) == 0)?true:false;
}

/*****************************************************************************
 函 数 名  : nv_data_writeback
 功能描述  : 回写NV镜像到各分区，包括工作分区和备份区
 输入参数  : 无
 输出参数  : 无
 返 回 值  :
*****************************************************************************/
u32 nv_data_writeback(void)
{
    u32 ret;

    ret = nv_img_flush_carrier();
    if(ret)
    {
        nv_record("write carrier file back to [img] fail! ret :0x%x\n",ret);
        return ret;
    }

    nv_img_remove_all_file();
    ret = nv_img_flush();
    if(ret)
    {
        nv_record("write nv file back to [img] fail! ret :0x%x\n",ret);
        return ret;
    }

    ret = nv_bak_flush();
    if(ret)
    {
        nv_record("write nv file back to [back] fail! ret :0x%x\n",ret);
        return ret;
    }

    return NV_OK;
}

void nv_modify_print_sw(u32 arg)
{
    g_nv_ctrl.debug_sw = arg;
}

s32 nv_modify_pm_sw(s32 arg)
{
    g_nv_ctrl.pmSw = (bool)arg;
    return 0;
}

/*
 * get nv read right,check the nv init state or upgrade state to read nv,
 * A core may read nv data after kernel init ,C core read nv data must behine the phase of
 *       acore kernel init or acore init ok
 */
bool nv_read_right(void)
{
    nv_global_info_s* ddr_info= (nv_global_info_s*)NV_GLOBAL_INFO_ADDR;
    if( NV_KERNEL_INIT_DOING > ddr_info->acore_init_state)
    {
        return false;
    }
    return true;
}

u32 nv_read_from_mem(nv_rdwr_req *rreq, nv_item_info_s *item_info, nv_ctrl_info_s *ctrl_info)
{
    u8* mdm_base;

    mdm_base = nv_get_item_mdmbase(rreq->modemid, item_info, ctrl_info);
    if(NULL == mdm_base)
    {
        return NV_ERROR;
    }

    (void)memcpy_s((void*)rreq->pdata, rreq->size, (void*)(mdm_base+rreq->offset), rreq->size);

    return NV_OK;
}

/*
 *  acore callback of icc msg.only accept req message
 */
s32 nv_icc_msg_proc(u32 chanid ,u32 len,void* pdata)
{
    return (s32)nv_handle_icc_rmsg(chanid , len);
/*lint -save -e715*//*715表示pdata未使用*/
}
/*lint -restore*/

/*
 *  nv use this inter to send data through the icc channel
 */
u32 nv_icc_send(u32 chanid,u8* pdata,u32 len)
{
    s32  ret ;
    u32  fun_id = chanid & 0xffff;/*get fun id*/
    u32  core_type ;
    u32  i;

    if(fun_id == NV_RECV_FUNC_AC)
    {
        core_type = ICC_CPU_MODEM;
    }
    else
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }
    nv_debug_trace(pdata, len);
    for(i = 0;i<NV_ICC_SEND_COUNT;i++)
    {
        ret = bsp_icc_send(core_type,chanid,pdata,len);
        if(ICC_INVALID_NO_FIFO_SPACE == ret)/*消息队列满,则50ms之后重新发送*/
        {
            nv_taskdelay(50);
            continue;
        }
        else if(ret != (s32)len)
        {
            nv_printf("[%s]:ret :0x%x,len 0x%x\n",__FUNCTION__,ret,len);
            return BSP_ERR_NV_ICC_CHAN_ERR;
        }
        else
        {
            nv_debug_record(NV_DEBUG_SEND_ICC|(((nv_icc_msg_t*)(unsigned long)pdata)->msg_type<<16));
            return NV_OK;
        }
    }
    system_error(DRV_ERRNO_NV_ICC_FIFO_FULL,core_type,chanid,(char*)pdata,len);
    return NV_ERROR;
}

/*
 *  init icc channel used by nv module
 */
u32 nv_icc_chan_init(u32 fun_id)
{
    u32 chanid;
    if(fun_id == NV_RECV_FUNC_AC)
    {
        /*lint -save -e845*//*The right argument to operator '|' is certain to be 0，但是此处最好不要修改*/
        chanid = ICC_CHN_NV << 16 | fun_id;
        /*lint -restore*/
    }
    else
    {
        return BSP_ERR_NV_INVALID_PARAM;
    }

     /*reg icc debug proc*/
    (void)bsp_icc_debug_register(chanid,nv_modify_pm_sw,(s32)true);

    return (u32)bsp_icc_event_register(chanid,nv_icc_msg_proc,NULL,NULL,NULL);
}

EXPORT_SYMBOL(nv_data_writeback);
EXPORT_SYMBOL(nv_modify_print_sw);
EXPORT_SYMBOL(nv_modify_pm_sw);



