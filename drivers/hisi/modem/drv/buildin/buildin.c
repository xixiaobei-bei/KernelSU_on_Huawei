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
#include "buildin.h"
#include <bsp_socp.h>



socp_cmdline_parse     g_stSocpParse={0,0,0,0,0};
socp_mem_reserve_stru  g_stSocpMemReserveParse  = {NULL,0,0,0,0};

extern unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base);
/*****************************************************************************
* 函 数 名  : socp_logbuffer_sizeparse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的BUFFER大小参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/

static int __init socp_logbuffer_sizeparse(char *pucChar)
{

    /* Buffer的大小以Byte为单位，原则上不大于200M，不小于1M */
    g_stSocpParse.ulBufferSize = (u32)simple_strtoul(pucChar, NULL, 0);
    return 0;
}
early_param("mdmlogsize", socp_logbuffer_sizeparse);

/*****************************************************************************
* 函 数 名  : socp_logbuffer_timeparse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的TIMEOUT大小参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
static int __init socp_logbuffer_timeparse(char *pucChar)
{

    /* 输入字符串以秒为单位，需要再转换成毫秒，至少为1秒，不大于20分钟 */
    g_stSocpParse.ulTimeout = (u32)simple_strtoul(pucChar, NULL,0);

    return 0;
}
early_param("mdmlogtime", socp_logbuffer_timeparse);
/*****************************************************************************
* 函 数 名  : socp_logbuffer_addrparse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的基地址参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
static int __init socp_logbuffer_addrparse(char *pucChar)
{

    g_stSocpParse.ulBaseAddr = simple_strtoul(pucChar, NULL, 0);

    return 0;
}
early_param("mdmlogbase", socp_logbuffer_addrparse);

/*****************************************************************************
* 函 数 名  : socp_logbuffer_logcfg
*
* 功能描述  : 在代码编译阶段将CMD LINE中的50M申请使能参数解析出来
*
* 输入参数  : 解析字符串
*
* 输出参数  : 无
*
* 返 回 值  : 成功与否
*****************************************************************************/
static int __init socp_logbuffer_logcfg(char *pucChar)
{
  
    g_stSocpParse.ulMemReservedEnable = (unsigned int )simple_strtoul(pucChar, NULL, 0);

    return 0;
}
early_param("modemlog_enable", socp_logbuffer_logcfg);


/*****************************************************************************
* 函 数 名  : socp_logbuffer_sizeparse
*
* 功能描述  : 在代码编译阶段将CMD LINE中的BUFFER大小参数解析出来
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
static int __init socp_bbpmemenable(char *pucChar)
{
   
    g_stSocpParse.ulBbpMemFlag = (u32)simple_strtoul(pucChar, NULL, 0);
    return 0;
}

early_param("modem_socp_enable",socp_bbpmemenable);

/*****************************************************************************
* 函 数 名  : bsp_socp_get_logbuffer_size
*
* 功能描述  : 获取buffer大小接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/

u32 bsp_socp_get_logbuffer_size(void)
{
    return g_stSocpParse.ulBufferSize;    
}
EXPORT_SYMBOL(bsp_socp_get_logbuffer_size);
/*****************************************************************************
* 函 数 名  : bsp_socp_get_logbuffer_time
*
* 功能描述  : 获取time大小接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
u32 bsp_socp_get_logbuffer_time(void)
{
    return g_stSocpParse.ulTimeout;
    
}
EXPORT_SYMBOL(bsp_socp_get_logbuffer_time);
/*****************************************************************************
* 函 数 名  : bsp_socp_get_logbuffer_addr
*
* 功能描述  : 获取基地址大小接口
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
u32 bsp_socp_get_logbuffer_addr(void)
{
    return g_stSocpParse.ulBaseAddr;
    
}
EXPORT_SYMBOL(bsp_socp_get_logbuffer_addr);
/*****************************************************************************
* 函 数 名  : bsp_socp_get_logbuffer_logcfg
*
* 功能描述  : 获取预留内存使能开关
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
u32 bsp_socp_get_logbuffer_logcfg(void)
{
    return g_stSocpParse.ulMemReservedEnable;
    
}
EXPORT_SYMBOL(bsp_socp_get_logbuffer_logcfg);

/*****************************************************************************
* 函 数 名  : bsp_socp_get_mee_reserve_stru
*
* 功能描述  : 获取预留内存信息结构
*
* 输入参数  : socp_mem_reserve_stru *pSocpMemStru
*
* 输出参数  : socp_mem_reserve_stru *pSocpMemStru
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_get_mem_reserve_stru(socp_mem_reserve_stru *pSocpMemStru)
{
    pSocpMemStru->ulPhyBufferAddr = g_stSocpMemReserveParse.ulPhyBufferAddr;
    pSocpMemStru->ulBufferSize    = g_stSocpMemReserveParse.ulBufferSize;
    pSocpMemStru->ulBufUsable     = g_stSocpMemReserveParse.ulBufUsable;
}
EXPORT_SYMBOL(bsp_socp_get_mem_reserve_stru);

/*****************************************************************************
* 函 数 名  : bsp_socp_bbpmemenable
*
* 功能描述  : 获取采数内存预留开关
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
u32 bsp_socp_bbpmemenable(void)
{
    return g_stSocpParse.ulBbpMemFlag;
    
}
EXPORT_SYMBOL(bsp_socp_bbpmemenable);
u32 mdrv_socp_bbpmemenable(void)
{
    return bsp_socp_bbpmemenable();
}
EXPORT_SYMBOL(mdrv_socp_bbpmemenable);

/*****************************************************************************
* 函 数 名  : socp_reserve_area
*
* 功能描述  : 获取系统预留的base和size
*
* 输入参数  : 内存结构体参数
*
* 输出参数  : 无
*
* 返 回 值  : 成功与否
*****************************************************************************/
static int socp_reserve_area(struct reserved_mem *rmem)
{
    char *status  ;

    status = (char *)of_get_flat_dt_prop(rmem->fdt_node, "status", NULL);
    if (status && (strncmp(status, "ok", strlen("ok")) != 0))
    {
        printk(KERN_ERR"[%s]:status is %s!\n", __FUNCTION__,status);
		return 0;
    }

    g_stSocpMemReserveParse.ulBufferSize     = (unsigned int)rmem->size;
    g_stSocpMemReserveParse.ulPhyBufferAddr  = rmem->base;

    /* if reserved buffer is too small, set kernel reserved buffer is not usable */
    if((0 == g_stSocpMemReserveParse.ulPhyBufferAddr)
        || (0 != g_stSocpMemReserveParse.ulPhyBufferAddr % 8))
    {
        printk(KERN_ERR"[%s]: kernel reserved addr is null, , is not 8bits align, base 0x%llx!\n",
                    __FUNCTION__,rmem->base);
        g_stSocpMemReserveParse.ulBufUsable = BSP_FALSE;
        return -1;
    }

    if((SOCP_MAX_MEM_SIZE > g_stSocpMemReserveParse.ulBufferSize)
        || (0 != g_stSocpMemReserveParse.ulBufferSize % 8))
    {
        printk(KERN_ERR"[%s]: kernel reserved buffer size is too small, is not 8bits align, size 0x%llx!\n",
                    __FUNCTION__,rmem->size);
        g_stSocpMemReserveParse.ulBufUsable = BSP_FALSE;
        return -1;
    }

    g_stSocpMemReserveParse.ulBufUsable = BSP_TRUE;
    printk(KERN_ERR"[%s]:kernel reserved buffer is useful, base 0x%llx, size is 0x%llx\n",
                 __FUNCTION__, rmem->base, rmem->size);

    return 0;
}

/*lint -save -e611*/
RESERVEDMEM_OF_DECLARE(hisilicon, "hisi_mdmlog", (reservedmem_of_init_fn)socp_reserve_area);
/*lint -restore +e611*/

