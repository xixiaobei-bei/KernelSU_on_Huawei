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
#include "socp_ind_delay.h"
#include <securec.h>

#include "socp_balong.h"


#include <linux/vmalloc.h>
#include <linux/device.h>
#include <linux/of_platform.h>
#include "bsp_socp.h"

#include <linux/of_reserved_mem.h>
#include <linux/of_fdt.h>
#include <nv_stru_drv.h>

#define  THIS_MODU mod_socp
struct socp_enc_dst_log_cfg g_stEncDstBufLogConfig = {NULL,0,0,10,false,10,false,0,0};
struct socp_enc_dst_log_cfg g_stDeflateDstBufLogConfig = {NULL,0,0,10,false,10,false,0,0};
u32 g_stDeflateState =0;
socp_early_cfg_stru         g_stSocpEarlyCfg    = {NULL,0,0,0,0,0};
socp_mem_reserve_stru       g_stSocpMemReserve  = {NULL,0,0,0,0};
DRV_DEFLATE_CFG_STRU  g_deflate_nv_ctrl={0,0};
extern u32 socp_version;
extern u32 g_strSocpDeflateStatus;
s32 deflate_set_compress_mode(SOCP_IND_MODE_ENUM eMode);

extern struct platform_device *modem_socp_pdev;

/*****************************************************************************
* 函 数 名  : socp_get_logbuffer_sizeparse
*
* 功能描述  : 获取socp buffer大小
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
s32  socp_get_logbuffer_sizeparse(void)
{
    u32      ulBufferSize;

    /* Buffer的大小以Byte为单位，原则上不大于200M，不小于1M */
    ulBufferSize = bsp_socp_get_logbuffer_size();

    if ((ulBufferSize > SOCP_MAX_MEM_SIZE )
      || (ulBufferSize < SOCP_MIN_MEM_SIZE))
    {
        socp_crit("cmdline: BuffSize=0x%x\n",ulBufferSize);
        g_stSocpEarlyCfg.ulBufUsable = BSP_FALSE;
        return -1;
    }

    /* 为了保持ulBufferSize的长度8字节对齐,如果长度不是8字节对齐地址也不会 */
    if (0 != (ulBufferSize % 8))
    {
        socp_error("BuffSize no 8 byte allignment,BuffSize: 0x%x\n",ulBufferSize);
        g_stSocpEarlyCfg.ulBufUsable = BSP_FALSE;
        return -1;
    }
    g_stSocpEarlyCfg.ulBufferSize = ulBufferSize;
    socp_crit("BufferSize 0x%x, adapt buffer_size: 0x%x\n",
                ulBufferSize,
                g_stSocpEarlyCfg.ulBufferSize);

    g_stSocpEarlyCfg.ulBufUsable  =  BSP_TRUE;
    return 0;
}


/*****************************************************************************
* 函 数 名  : socp_get_logbuffer_timeparse
*
* 功能描述  : 获取socp时间
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
s32  socp_get_logbuffer_timeparse(void)
{
    u32      ulTimeout;

    /* 输入字符串以秒为单位，需要再转换成毫秒，至少为1秒，不大于20分钟 */
    ulTimeout = bsp_socp_get_logbuffer_time();

    if (SOCP_MAX_TIMEOUT < ulTimeout)
    {

        ulTimeout = SOCP_MAX_TIMEOUT;
    }

    ulTimeout *= 1000;

    g_stSocpEarlyCfg.ulTimeout  = ulTimeout;

    socp_crit("early_cfg: timeout=0x%x\n", g_stSocpEarlyCfg.ulTimeout);

    return 0;
}


/*****************************************************************************
* 函 数 名  : socp_get_logbuffer_addrparse
*
* 功能描述  : 获取内存基地址
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
s32 socp_get_logbuffer_addrparse(void)
{
    unsigned long  ulBaseAddr;

    ulBaseAddr = bsp_socp_get_logbuffer_addr();

    /* 物理地址是32位的实地址并且是8字节对齐的 */
    if ((0 != (ulBaseAddr % 8))
        || (0 == ulBaseAddr))
    {
        return -1;
    }

    g_stSocpEarlyCfg.ulPhyBufferAddr = ulBaseAddr;

    return 0;
}
/*****************************************************************************
* 函 数 名  : socp_get_cmdline_param
*
* 功能描述  : 获取cmdline参数
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void socp_get_cmdline_param(void)
{
    (void)socp_get_logbuffer_sizeparse();
    (void)socp_get_logbuffer_timeparse();
    (void)socp_get_logbuffer_addrparse();
    (void)socp_get_logbuffer_logcfg();
    return ;

}
/*****************************************************************************
* 函 数 名  : socp_get_logbuffer_logcfg
*
* 功能描述  : 获取50M buffer使能位
*
* 输入参数  : 解析字符串
*
* 输出参数  : 无
*
* 返 回 值  : 成功与否
*****************************************************************************/
s32  socp_get_logbuffer_logcfg(void)
{
    unsigned int  ulMemLogCfg;

    ulMemLogCfg= bsp_socp_get_logbuffer_logcfg();

    g_stSocpEarlyCfg.ulLogCfg = ulMemLogCfg;

    socp_crit("early_cfg: memLogCfg=0x%x\n", g_stSocpEarlyCfg.ulLogCfg);

    return 0;
}


/*lint -save -e785*/
static const struct of_device_id socp_dev_of_memalloc_match[] = {
        {.compatible = "hisilicon,socp_balong_memalloc"},
	    {},
    };
/*lint -restore +e785*/
/*****************************************************************************
* 函 数 名  : socp_memalloc_probe
*
* 功能描述  : 50M内存申请驱动设备的probe函数
*
* 输入参数  : 驱动设备pdev
*
* 输出参数  : 无
*
* 返 回 值  : 成功与否
*****************************************************************************/

static int __init socp_memalloc_probe(struct platform_device *pdev)
{
    u32 ulTimeout=0;
    s32 ret;
    struct device_node* dev ;
    struct device *pdevice = &(pdev->dev);

    if(g_stSocpEarlyCfg.ulLogCfg)
    {
        ret=of_reserved_mem_device_init(pdevice);
        if(ret<0)
        {
             socp_error("mem_device_init fail\n" );
             return -1;
        }
		socp_crit( "[init]mem_device_init sucess!\n");
    }
	else
	{
		socp_crit("socp_memalloc_probe: mdmlog not open!\n" );
		return -1;
	}

    dev = of_find_node_by_name(NULL, "hisi_mdmlog");
    if(NULL == dev)
    {
        socp_error("of_find_node_by_name failed!\n");
        return -1;
    }

    ret = of_property_read_u32_index(dev,"time",0, &ulTimeout);
    if(ret)
    {
        socp_error("of_property_read_u32_index failed!\n");
        return -1;
    }

    socp_crit("of_property_read_u32_index get time 0x%x!\n",ulTimeout);

    if (SOCP_MAX_TIMEOUT < ulTimeout)
    {
        ulTimeout = SOCP_MAX_TIMEOUT;
    }

    ulTimeout *= 1000;

    g_stSocpMemReserve.ulTimeout = ulTimeout;

    return 0;

}
/*lint -save -e785 -e64*/
static struct platform_driver socp_mem_driver = {
    .driver = {
               .name = "modem_socp_memalloc",
               .owner = THIS_MODULE,
               .of_match_table = socp_dev_of_memalloc_match,
    },
    .probe = socp_memalloc_probe,
};
/*lint -restore +e785 +e64*/

void *socp_logbuffer_memremap(unsigned long phys_addr, size_t size)
{
    unsigned long i;
    u8* vaddr;
    unsigned long npages = (PAGE_ALIGN((phys_addr & (PAGE_SIZE - 1)) + size) >> PAGE_SHIFT);
    unsigned long offset = phys_addr & (PAGE_SIZE - 1);
    struct page **pages;

    pages = vmalloc(sizeof(struct page *) * npages);
    if (!pages)
    {
        return NULL;
    }

    pages[0] = phys_to_page(phys_addr);

    for (i = 0; i < npages - 1 ; i++)
    {
        pages[i + 1] = pages[i] + 1;
    }

    vaddr = (u8*)vmap(pages, (unsigned int)npages, (unsigned long)VM_MAP, pgprot_writecombine(PAGE_KERNEL));

    if ( NULL != vaddr )

    {
        vaddr += offset;
    }

    vfree(pages);

    return (void *)vaddr;
}

/*****************************************************************************
* 函 数 名  : socp_logbuffer_mmap
*
* 功能描述  : 物理地址转换成虚拟内存
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
s32 socp_logbuffer_mmap(void)
{
    /*step1: if logcfg is on, mmap kernel reserved memory */
    if(SOCP_RESERVED_TRUE == g_stSocpEarlyCfg.ulLogCfg)
    {
        if(BSP_TRUE == g_stSocpMemReserve.ulBufUsable)
        {
            /* 映射虚拟地址 */
            g_stSocpMemReserve.pVirBuffer = socp_logbuffer_memremap(g_stSocpMemReserve.ulPhyBufferAddr,
                                                                    (size_t)g_stSocpMemReserve.ulBufferSize);
            if(NULL == g_stSocpMemReserve.pVirBuffer)
            {
                g_stSocpMemReserve.ulBufUsable = BSP_FALSE;
                return BSP_ERROR;
            }

            socp_crit("kernel reserved buffer mmap success\n");
            return BSP_OK;
        }
        else
        {
            return BSP_ERROR;
        }
    }

    /*step2: if buffer addr is invalid or buffer size is invalide,
             it means fastboot do not reserve memory to us,
             or memory reserved is invalid, do nothing*/
    if(BSP_FALSE == g_stSocpEarlyCfg.ulBufUsable)
    {
        return BSP_OK;
    }

    /*step3: memmap socp buffer, and record virtual buffer address */
    g_stSocpEarlyCfg.pVirBuffer = socp_logbuffer_memremap(g_stSocpEarlyCfg.ulPhyBufferAddr,
                                                          (size_t)g_stSocpEarlyCfg.ulBufferSize);
    if(NULL == g_stSocpEarlyCfg.pVirBuffer)
    {
        g_stSocpEarlyCfg.ulBufUsable = BSP_FALSE;
        return BSP_ERROR;
    }

    g_stSocpEarlyCfg.ulBufUsable = BSP_TRUE;

    socp_crit("fastboot resered buffer is valid\n");

    return BSP_OK;
}

/* log2.0 2014-03-19 Begin:*/
/*****************************************************************************
* 函 数 名  : bsp_socp_get_log_cfg
*
* 功能描述  : 获取LOG2.0 SOCP水线、超时配置信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : SOCP_ENC_DST_BUF_LOG_CFG_STRU指针
*****************************************************************************/
struct socp_enc_dst_log_cfg * bsp_socp_get_log_cfg(void)
{
    return &g_stEncDstBufLogConfig;
}
/*****************************************************************************
* 函 数 名  : bsp_socp_get_log_ind_mode
*
* 功能描述  : 上报模式接口
*
* 输入参数  : 模式参数
*
* 输出参数  : 无
*
* 返 回 值  : BSP_S32 BSP_OK:成功 BSP_ERROR:失败
*****************************************************************************/
s32  bsp_socp_get_log_ind_mode(u32 *LofgIndMode)
{
    *LofgIndMode = g_stEncDstBufLogConfig.currentMode;
    return  BSP_OK;
}

u32 bsp_socp_get_sd_logcfg(SOCP_ENC_DST_BUF_LOG_CFG_STRU* cfg)
{
    struct socp_enc_dst_log_cfg* LogCfg;
    u32  deflate_stat;
    if(NULL == cfg)
        return BSP_ERR_SOCP_INVALID_PARA;

    LogCfg = bsp_socp_get_log_cfg();

    cfg->logOnFlag       = LogCfg->logOnFlag;
    cfg->BufferSize      = LogCfg->BufferSize;

    deflate_stat =bsp_socp_compress_status();
    if(1 == deflate_stat)
    {
        cfg->overTime        = g_stDeflateDstBufLogConfig.overTime;
    }
    else
    {

        cfg->overTime        = LogCfg->overTime;
    }
    cfg->pVirBuffer      = LogCfg->pVirBuffer;
    cfg->ulCurTimeout    = LogCfg->ulCurTimeout;
    cfg->ulPhyBufferAddr = LogCfg->ulPhyBufferAddr;

    return BSP_OK;
}
/*****************************************************************************
* 函 数 名  : socp_logbuffer_dmalloc
* 功能描述  : 动态申请内存
* 输入参数  : 设备节点
* 输出参数  : 无
* 返 回 值  : 设置成功与否标志
*****************************************************************************/
s32 socp_logbuffer_dmalloc(struct device_node* dev)
{
    dma_addr_t      ulAddress = 0;
    u8              *pucBuf;
    int             ret;
    u32             aulDstChan[SOCP_DST_CHAN_CFG_BUTT]={0};
    u32             size;

    ret = of_property_read_u32_array(dev, "dst_chan_cfg", aulDstChan, (size_t)SOCP_DST_CHAN_CFG_BUTT);
    if(ret)
    {
        socp_crit("dts don't config dmalloc logbuffer size, use default size 1M!\n");
        size = 1*1024*1024;
    }
    else
    {
        socp_crit("of_property_read_u32_array get size 0x%x!\n", aulDstChan[SOCP_DST_CHAN_CFG_SIZE]);
        size = aulDstChan[SOCP_DST_CHAN_CFG_SIZE];
    }
    pucBuf =(u8 *) dma_alloc_coherent(&modem_socp_pdev->dev, (size_t)size, &ulAddress, GFP_KERNEL);

    if(BSP_NULL == pucBuf)
    {
        socp_error("logbuffer_dmalloc buffer failed\n");
        return BSP_ERROR;
    }

    socp_crit("socp_logbuffer_dmalloc success\n");

    g_stEncDstBufLogConfig.ulPhyBufferAddr  = (unsigned long)ulAddress;
    g_stEncDstBufLogConfig.pVirBuffer       = pucBuf;
    g_stEncDstBufLogConfig.ulCurTimeout     = 10; /* 使用默认值 */
    g_stEncDstBufLogConfig.BufferSize       = size;
    g_stEncDstBufLogConfig.logOnFlag        = SOCP_DST_CHAN_DTS;

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_socp_timeout_init
*
* 功能描述  : 模块初始化目的buffer上报超时配置的函数
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 初始化成功的标识码
*****************************************************************************/
s32 bsp_socp_logbuffer_init(struct device_node* dev)
{
    s32 ret;

    /*step1: if modlog config enabled, use kernel reserved buffer */
    if(SOCP_RESERVED_TRUE ==g_stSocpEarlyCfg.ulLogCfg)
    {
        /*step1.1: if kernel reserved buffer is valid, enable ind delay */
        if(BSP_TRUE == g_stSocpMemReserve.ulBufUsable)
        {
            g_stEncDstBufLogConfig.pVirBuffer       = g_stSocpMemReserve.pVirBuffer;
            g_stEncDstBufLogConfig.ulPhyBufferAddr  = g_stSocpMemReserve.ulPhyBufferAddr;
            g_stEncDstBufLogConfig.BufferSize       = g_stSocpMemReserve.ulBufferSize;
            g_stEncDstBufLogConfig.overTime         = g_stSocpMemReserve.ulTimeout;
            g_stEncDstBufLogConfig.logOnFlag        = SOCP_DST_CHAN_DELAY;
        }
        /*step1.2: if kernel reserved buffer is invalid, disable ind delay, use 1M malloc buffer */
        else
        {
            g_stEncDstBufLogConfig.logOnFlag        = SOCP_DST_CHAN_NOT_CFG;

        }
    }
    /*step2: if modlog config is not parsed, use fastboot reserved memory*/
    else
    {
        /*step2.1: if fastboot reserved memory is valid, enable ind delay*/
        if(BSP_TRUE == g_stSocpEarlyCfg.ulBufUsable)
        {
            g_stEncDstBufLogConfig.pVirBuffer       = g_stSocpEarlyCfg.pVirBuffer;
            g_stEncDstBufLogConfig.ulPhyBufferAddr  = g_stSocpEarlyCfg.ulPhyBufferAddr;
            g_stEncDstBufLogConfig.BufferSize       = g_stSocpEarlyCfg.ulBufferSize;
            g_stEncDstBufLogConfig.overTime         = g_stSocpEarlyCfg.ulTimeout;
            g_stEncDstBufLogConfig.logOnFlag        = SOCP_DST_CHAN_DELAY;
        }
        /*step2.2: if fastboot reserved memory is invalid, use 1M malloc buffer, disable ind delay*/
        else
        {
             g_stEncDstBufLogConfig.logOnFlag=SOCP_DST_CHAN_NOT_CFG;
        }
    }

    /*step3: use dmalloc buffer, disable ind delay*/
    /* 延迟上报没打开时判断DTSI中是否配置目的通道的buffer size */
    if(SOCP_DST_CHAN_NOT_CFG == g_stEncDstBufLogConfig.logOnFlag)
    {
        ret = socp_logbuffer_dmalloc(dev);
        if(ret)
        {
            socp_error("of_property_read_u32_array failed!\n");
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_socp_timeout_init
*
* 功能描述  : 模块初始化目的buffer上报超时配置的函数
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 初始化成功的标识码
*****************************************************************************/
void bsp_socp_timeout_init(void)
{
    bsp_socp_set_timeout(SOCP_TIMEOUT_TRF,  SOCP_MIN_TIMEOUT*2289/1000);

    return;
}

void bsp_socp_set_mode_direct(void)
{
    u32 ret;

    if(SOCP_IND_MODE_DIRECT == g_stEncDstBufLogConfig.currentMode)
    {
        socp_crit("the ind mode direct is already config!\n");
        return;
    }

    if(socp_version < SOCP_206_VERSION)
    {
        (void)bsp_socp_set_timeout(SOCP_TIMEOUT_TRF, 0x17);
    }
    else
    {
        (void)bsp_socp_set_timeout(SOCP_TIMEOUT_TRF_LONG, SOCP_TIMEOUT_TRF_LONG_MIN);
    }
    g_stEncDstBufLogConfig.ulCurTimeout = 10;
    bsp_socp_set_enc_dst_threshold((bool)FALSE,SOCP_CODER_DST_OM_IND);
    ret = bsp_socp_encdst_set_cycle(SOCP_CODER_DST_OM_IND, SOCP_IND_MODE_DIRECT);
    if(BSP_OK != ret)
    {
        socp_error("direct mode config failed!\n");
    }
    else
    {
        g_stEncDstBufLogConfig.currentMode = SOCP_IND_MODE_DIRECT;
        socp_crit("direct mode config sucess!\n");
    }
}

void bsp_socp_set_mode_delay(void)
{
    u32 time;
    u32 ret;

    if(SOCP_IND_MODE_DELAY == g_stEncDstBufLogConfig.currentMode)
    {
        socp_crit("socp:the ind mode delay is already config!\n");
        return;
    }

    /* if logbuffer is not configed, can't enable delay mode*/
    if(g_stEncDstBufLogConfig.logOnFlag == SOCP_DST_CHAN_DELAY)
    {
        if(socp_version < SOCP_206_VERSION)
        {
            time = (g_stEncDstBufLogConfig.overTime * 2289)/1000;
            (void)bsp_socp_set_timeout(SOCP_TIMEOUT_TRF, time);
            g_stEncDstBufLogConfig.ulCurTimeout = g_stEncDstBufLogConfig.overTime;
        }
        else
        {
           (void)bsp_socp_set_timeout(SOCP_TIMEOUT_TRF_LONG, SOCP_TIMEOUT_TRF_LONG_MAX);
        }
        g_stEncDstBufLogConfig.ulCurTimeout = SOCP_TIMEOUT_TRF_LONG_MAX;
        bsp_socp_set_enc_dst_threshold((bool)TRUE,SOCP_CODER_DST_OM_IND);
        ret = bsp_socp_encdst_set_cycle(SOCP_CODER_DST_OM_IND, SOCP_IND_MODE_DELAY);
        if(BSP_OK != ret)
        {
            socp_error("delay mode config failed!\n");
        }
        else
        {
            g_stEncDstBufLogConfig.currentMode = SOCP_IND_MODE_DELAY;
            socp_crit("delay mode config sucess!\n");
        }
    }
    else
    {
        socp_crit("delay mode can't config:mem can't be setted!\n");
    }
}

void bsp_socp_set_mode_cycle(void)
{
    u32 ret;

    if(SOCP_IND_MODE_CYCLE == g_stEncDstBufLogConfig.currentMode)
    {
        socp_crit("the ind mode cycle is already config!\n");
        return;
    }

    /* if logbuffer is not configed, can't enable cycle mode*/
    /* 如果dst方式malloc 10M内存,也支持配置循环模式，支持商用10Mlog抓取*/
    if(((g_stEncDstBufLogConfig.logOnFlag == SOCP_DST_CHAN_DELAY)&&(BSP_TRUE == g_stSocpMemReserve.ulBufUsable))||
        (g_stEncDstBufLogConfig.logOnFlag == SOCP_DST_CHAN_DTS))
    {
        ret = bsp_socp_encdst_set_cycle(SOCP_CODER_DST_OM_IND, SOCP_IND_MODE_CYCLE);
        if(BSP_OK != ret)
        {
            socp_error("cycle mode config failed!\n");
        }
        else
        {
            g_stEncDstBufLogConfig.currentMode = SOCP_IND_MODE_CYCLE;
            socp_crit("the ind cycle mode config sucess!\n");
        }
    }
    else
    {
        socp_crit("ind delay can't config:mem can't be setted!\n");
    }
}

/*****************************************************************************
* 函 数 名  : bsp_socp_set_ind_mode
*
* 功能描述  : 上报模式接口
*
* 输入参数  : 模式参数
*
* 输出参数  : 无
*
* 返 回 值  : BSP_S32 BSP_OK:成功 BSP_ERROR:失败
*****************************************************************************/
s32 bsp_socp_set_ind_mode(SOCP_IND_MODE_ENUM eMode)
{
    switch (eMode)
    {
        case SOCP_IND_MODE_DIRECT:
        {
            bsp_socp_set_mode_direct();
            break;
        }
        case SOCP_IND_MODE_DELAY:
        {
            bsp_socp_set_mode_delay();
            break;

        }
        case SOCP_IND_MODE_CYCLE:
        {
            bsp_socp_set_mode_cycle();
            break;
        }
        default:
        {
            socp_error("set invalid mode: %d!\n",g_stEncDstBufLogConfig.currentMode);
            return  BSP_ERROR;
        }
    }

    return BSP_OK ;
 }



/*****************************************************************************
  函 数 名  : bsp_socp_compress_status
  功能描述  : 查询当前压缩模式是否开启
  输入参数  : 无
  输出参数  : 无
  返 回 值  : 无

*****************************************************************************/

s32 bsp_socp_compress_status(void)
{
    return g_strSocpDeflateStatus;
}
/*****************************************************************************
  函 数 名  : bsp_socp_get_cfg_ind_mode
  功能描述  : 查询获取当前上报模式
  输入参数  : 无
  输出参数  : 无
  返 回 值  : 无

*****************************************************************************/

s32 bsp_socp_get_cfg_ind_mode(u32 *CfgIndMode)
{
    u32 ulDeflateState;
    ulDeflateState = bsp_socp_compress_status();
    if(SOCP_COMPRESS == ulDeflateState)
    {
        bsp_deflate_get_log_ind_mode(CfgIndMode);
        socp_crit("deflate channel is open!\n");
        return BSP_OK;
    }
    else
    {
        bsp_socp_get_log_ind_mode(CfgIndMode);
        socp_crit("socp channel is open!\n");
        return BSP_OK;
    }
}
/*****************************************************************************
  函 数 名  : bsp_socp_set_cfg_ind_mode
  功能描述  : 设置当前上报模式
  输入参数  : 无
  输出参数  : 无
  返 回 值  : 无

*****************************************************************************/

s32 bsp_socp_set_cfg_ind_mode(SOCP_IND_MODE_ENUM eMode)
{
    u32 ulDeflateState;
    ulDeflateState = bsp_socp_compress_status();
    if(SOCP_NO_COMPRESS==ulDeflateState)
    {
        bsp_socp_set_ind_mode(eMode);
        socp_crit("socp channel is open!\n");
        return BSP_OK;
    }
    else
    {
        bsp_deflate_set_ind_mode(eMode);
        socp_crit("deflate channel is open!\n");
    }
    return   BSP_OK;
}
/*****************************************************************************
  函 数 名  : bsp_socp_get_cps_ind_mode
  功能描述  : 获取当前压缩模式
  输入参数  : 无
  输出参数  : 无
  返 回 值  : 无

*****************************************************************************/
s32 bsp_socp_get_cps_ind_mode(u32 *CpsIndMode)
{
    u32 ulDeflateState;
    ulDeflateState = bsp_socp_compress_status();
    if(SOCP_COMPRESS==ulDeflateState)
    {
        *CpsIndMode = DEFLATE_IND_COMPRESS ;
        socp_crit("deflate channel is open!\n");
        return BSP_OK;
    }
    else
    {
        *CpsIndMode = DEFLATE_IND_NO_COMPRESS;
        socp_crit("socp channel is open!\n");
        return BSP_OK;
    }

}
/*****************************************************************************
  函 数 名  : bsp_socp_set_cps_ind_mode
  功能描述  : 设置当前压缩模式
  输入参数  : 无
  输出参数  : 无
  返 回 值  : 无

*****************************************************************************/
s32 bsp_socp_set_cps_ind_mode(DEFLATE_IND_COMPRESSS_ENUM eMode)
{
    return BSP_ERROR;

}
/*****************************************************************************
* 函 数 名  : socp_ind_delay_init
*
* 功能描述  : 模块初始化函数
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 初始化成功的标识码
*****************************************************************************/
s32 bsp_socp_ind_delay_init(void)
{
    s32 ret;
    struct device_node* dev;

    dev = of_find_compatible_node(NULL,NULL,"hisilicon,socp_balong_app");
    if(NULL == dev)
    {
        socp_error("Socp dev find failed\n");
        return BSP_ERROR;
    }
    /*获取cmdline参数*/
    (void)socp_get_cmdline_param();

    /*获取SOCP预留内存信息*/
    bsp_socp_get_mem_reserve_stru(&g_stSocpMemReserve);

    /*获取的内存虚拟物理转换*/
    (void)socp_logbuffer_mmap();
	ret = platform_driver_register(&socp_mem_driver); /*lint !e64*/
    if(ret)
    {
        socp_error("platform driver register failed!\n");
        return BSP_ERROR;
    }

    ret = bsp_socp_logbuffer_init(dev);
    if(ret)
    {
        socp_error("socp dst logbuffer init faield!\n");
        return ret;
    }

    /* init timeout */
    bsp_socp_timeout_init();

    socp_crit("socp_ind_delay init sucess!\n");

    return BSP_OK;
}


/*****************************************************************************
* 函 数 名  : bsp_deflate_get_log_ind_mode
*
* 功能描述  : 上报模式接口
*
* 输入参数  : 模式参数
*
* 输出参数  : 无
*
* 返 回 值  : BSP_S32 BSP_OK:成功 BSP_ERROR:失败
*****************************************************************************/

s32  bsp_deflate_get_log_ind_mode(u32 *CfgIndMode)
{

    *CfgIndMode = g_stDeflateDstBufLogConfig.currentMode;

    return  BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_deflate_get_compress_mode
*
* 功能描述  : 上报模式接口
*
* 输入参数  : 模式参数
*
* 输出参数  : 无
*
* 返 回 值  : BSP_S32 BSP_OK:成功 BSP_ERROR:失败
*****************************************************************************/

s32  bsp_deflate_get_compress_mode(u32 *CpsIndMode)
{
    *CpsIndMode = g_stDeflateDstBufLogConfig.cpsMode;

    return  BSP_OK;
}

/*****************************************************************************
* 函 数 名  : bsp_deflate_set_ind_mode
*
* 功能描述  : 上报模式接口
*
* 输入参数  : 模式参数
*
* 输出参数  : 无
*
* 返 回 值  : BSP_S32 BSP_OK:成功 BSP_ERROR:失败
*****************************************************************************/
s32 bsp_deflate_set_ind_mode(SOCP_IND_MODE_ENUM eMode)
{
    return BSP_OK ;
 }

/*****************************************************************************
* 函 数 名  : bsp_socp_read_cur_mode
*
* 功能描述  : 循环模式查询接口
*
* 输入参数  : 通道ID
*
* 输出参数  : 无
*
* 返 回 值  : 初始化成功的标识码
*****************************************************************************/
u32 bsp_socp_read_cur_mode(u32 u32DestChanID)
{
    u32 u32modestate;
    u32 u32ChanID = SOCP_REAL_CHAN_ID(u32DestChanID);

    /*lint -save -e647*/
    u32modestate = SOCP_REG_GETBITS(SOCP_REG_ENCDEST_SBCFG(u32ChanID),1,1);
    /*lint -restore +e647*/
    return u32modestate;
}

/*****************************************************************************
* 函 数 名  : bsp_socp_logbuffer_cfgshow
*
* 功能描述  : 打印延时写入的配置信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_logbuffer_cfgshow(void)
{

    socp_crit("socp_logbuffer_cfgshow: overTime           %d\n", g_stEncDstBufLogConfig.overTime);
    socp_crit("socp_logbuffer_cfgshow: Current Time Out   %d\n", g_stEncDstBufLogConfig.ulCurTimeout);
    socp_crit("socp_logbuffer_cfgshow: BufferSize         %d\n", g_stEncDstBufLogConfig.BufferSize);
    socp_crit("socp_logbuffer_cfgshow: logOnFlag          %d\n", g_stEncDstBufLogConfig.logOnFlag);
    socp_crit("socp_logbuffer_cfgshow: PhyBufferAddr      0x%lx\n", g_stEncDstBufLogConfig.ulPhyBufferAddr);
    socp_crit("socp_logbuffer_cfgshow: currentmode        %d\n", g_stEncDstBufLogConfig.currentMode);

    return;
}
EXPORT_SYMBOL(bsp_socp_logbuffer_cfgshow);

/*****************************************************************************
* 函 数 名  : bsp_deflate_logbuffer_cfgshow
*
* 功能描述  : 打印延时写入的配置信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_deflate_logbuffer_cfgshow(void)
{

    socp_crit("deflate_logbuffer_cfgshow: overTime           %d\n", g_stDeflateDstBufLogConfig.overTime);
    socp_crit("deflate_logbuffer_cfgshow: Current Time Out   %d\n", g_stDeflateDstBufLogConfig.ulCurTimeout);
    socp_crit("deflate_logbuffer_cfgshow: BufferSize         %d\n", g_stDeflateDstBufLogConfig.BufferSize);
    socp_crit("deflate_logbuffer_cfgshow: logOnFlag          %d\n", g_stDeflateDstBufLogConfig.logOnFlag);
    socp_crit("deflate_logbuffer_cfgshow: PhyBufferAddr      0x%lx\n", g_stDeflateDstBufLogConfig.ulPhyBufferAddr);
    socp_crit("deflate_logbuffer_cfgshow: currentmode        %d\n", g_stDeflateDstBufLogConfig.currentMode);

    return;
}
EXPORT_SYMBOL(bsp_deflate_logbuffer_cfgshow);

/*****************************************************************************
* 函 数 名  : socp_logbuffer_early_cfgshow
*
* 功能描述  : cmdline方式申请内存信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_logbuffer_early_cfgshow(void)
{

    socp_crit("socp_logbuffer_early_cfgshow: BufferSize         %d\n", g_stSocpEarlyCfg.ulBufferSize);
    socp_crit("socp_logbuffer_early_cfgshow: BufUsable          %d\n", g_stSocpEarlyCfg.ulBufUsable);
    socp_crit("socp_logbuffer_early_cfgshow: logCfg             %d\n", g_stSocpEarlyCfg.ulLogCfg);
    socp_crit("socp_logbuffer_early_cfgshow: PhyBufferAddr      0x%lx\n", g_stSocpEarlyCfg.ulPhyBufferAddr);
    socp_crit("socp_logbuffer_early_cfgshow: time               %d\n", g_stSocpEarlyCfg.ulTimeout);


    return;
}
EXPORT_SYMBOL(bsp_socp_logbuffer_early_cfgshow);

/*****************************************************************************
* 函 数 名  : socp_logbuffer_memreseve_cfgshow
*
* 功能描述  : kernel预留内存配置信息
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : 无
*****************************************************************************/
void bsp_socp_logbuffer_memreserve_cfgshow(void)
{

    socp_crit("socp_logbuffer_memreserve_cfgshow: BufferSize         %d\n", g_stSocpMemReserve.ulBufferSize);
    socp_crit("socp_logbuffer_memreserve_cfgshow: BufUsable          %d\n", g_stSocpMemReserve.ulBufUsable);
    socp_crit("socp_logbuffer_memreserve_cfgshow: PhyBufferAddr      0x%lx\n", g_stSocpMemReserve.ulPhyBufferAddr);
    socp_crit("socp_logbuffer_memreserve_cfgshow: time               %d\n", g_stSocpMemReserve.ulTimeout);


    return;
}
EXPORT_SYMBOL(bsp_socp_logbuffer_memreserve_cfgshow);

EXPORT_SYMBOL(bsp_socp_get_log_cfg);






