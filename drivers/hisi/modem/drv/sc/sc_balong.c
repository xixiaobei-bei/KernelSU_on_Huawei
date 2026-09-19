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
#include <linux/err.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <mdrv.h>
#include "../../adrv/adrv.h"
#include "bsp_misc.h"
#include "sc_balong.h"
#include "nv_ctrl.h"
#include "bsp_blk.h"

#define SC_AREA_NAME PARTITION_MODEM_NVDEFAULT_NAME

/* func type */
s32  sc_bakup(s8 *pdata, u32 len);
s32  sc_restore(s8 *pdata, u32 len);


/*
* Function   : sc_bakup
* Discription: c core nv init,this phase build upon the a core kernel init,
*              this phase after icc init,this phase ensure to use all nv api normal
*              start at this phase ,ops global ddr need spinlock
* Parameter  : none
* Output     : result
* History    :
*/
s32  sc_bakup(s8 *pdata, u32 len)
{
    s32 wlen;
    u32 sc_mtd_len  = 0;
    u32 ret;

    if((NULL == pdata)||(0 == len))
    {
        sc_error_printf("pdata is NULL\n");
        return BSP_ERR_SC_INVALID_PARAM;
    }

    ret = (u32)bsp_blk_size(SC_AREA_NAME, &sc_mtd_len);
    if (ret) {
        sc_error_printf("bsp_blk_size error for device %s \n", SC_AREA_NAME);
        return BSP_ERR_READ_LGTH_FAIL;
    }

    sc_debug_printf("mtd len: 0x%x\n",sc_mtd_len);
    if((sc_mtd_len < SC_MTD_PTABLE_OFFSET) || (len >= SC_MTD_PTABLE_OFFSET))
    {
        sc_error_printf("mtd length err! sc_mtd_len: 0x%x, len: 0x%x\n",sc_mtd_len, len);
        return BSP_ERR_READ_LGTH_FAIL;
    }

    wlen = bsp_blk_write(SC_AREA_NAME, (loff_t)(sc_mtd_len - SC_MTD_PTABLE_OFFSET), pdata, (size_t)len);
    if(wlen != BSP_OK)
    {
        sc_error_printf("mtd length err! wlen 0x%x, len is 0x%x\n",wlen,len);
        return BSP_ERR_SC_WRITE_FILE_FAIL;
    }

    sc_debug_printf("sc write to nand ok, len is 0x%x!\n",(u32)(wlen));
    return SC_OK;
}

/*
* Function   : sc_restore
* Discription: c core nv init,this phase build upon the a core kernel init,
*              this phase after icc init,this phase ensure to use all nv api normal
*              start at this phase ,ops global ddr need spinlock
* Parameter  : none
* Output     : result
* History    :
*/
s32  sc_restore(s8 *pdata, u32 len)
{
    s32 rlen;
    u32 sc_mtd_len  = 0;
    u32 ret;

    if((NULL == pdata)||(0 == len))
    {
        sc_error_printf("pdata is NULL\n");
        return BSP_ERR_SC_INVALID_PARAM;
    }

    ret = (u32)bsp_blk_size(SC_AREA_NAME, &sc_mtd_len);
    if (ret) {
        sc_error_printf("bsp_blk_size error for device %s \n", SC_AREA_NAME);
        return BSP_ERR_READ_LGTH_FAIL;
    }

    if((sc_mtd_len < SC_MTD_PTABLE_OFFSET) || (len >= SC_MTD_PTABLE_OFFSET))
    {
        sc_error_printf("mtd length err! size 0x%x\n", sc_mtd_len);
        return BSP_ERR_READ_LGTH_FAIL;
    }

    rlen = bsp_blk_read(SC_AREA_NAME, (loff_t)(sc_mtd_len - SC_MTD_PTABLE_OFFSET), pdata, (size_t)len);
    if(rlen != BSP_OK)
    {
        sc_error_printf("mtd length err! read_ret 0x%x, len is 0x%x\n",rlen,len);
        return BSP_ERR_SC_READ_FILE_FAIL;
    }
    else
    {
        sc_debug_printf("mtd length read ok len is 0x%x\n",rlen);
    }

    return SC_OK;
}

/*
* Function   : bsp_sc_restore
* Parameter  : none
* Output     : result
* History    :
*/
s32 bsp_sc_restore(u8* pRamAddr,u32 len)
{
    return sc_restore((s8 *)pRamAddr,len);
}

/*
* Function   : bsp_sc_backup
* Parameter  : none
* Output     : result
* History    :
*/
s32 bsp_sc_backup(u8* pRamAddr,u32 len)
{
    return sc_bakup((s8 *)pRamAddr,len);
}

EXPORT_SYMBOL(bsp_sc_restore);
EXPORT_SYMBOL(bsp_sc_backup);



