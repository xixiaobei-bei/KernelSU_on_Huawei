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
#include <linux/of_platform.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <osl_malloc.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <of.h>
#include <osl_thread.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <osl_bio.h>
#include <securec.h>
#include "mdrv_diag_system_common.h"
#include "bsp_hds_service.h"
#include "bsp_hds_ind.h"
#include "bsp_slice.h"

#undef THIS_MODU
#define THIS_MODU mod_hds

print_send_buff* g_print_sendbuf = NULL;
trans_send_buff* g_trans_sendbuf = NULL;
LOG_SRC_CFG_STRU g_logSrcCfg = {0};
u32 g_print_init_state=PRINTLOG_CHN_UNINIT;
u32 g_trans_init_state=TRANSLOG_CHN_UNINIT;
unsigned long long g_log_dma_test_mask=0xffffffffffffffffULL;
u32 g_printlog_pkgnum = 0;
u32 g_Translog_pkgnum = 0;
u32 g_printlog_transId = 0;
u32 g_translog_transId = 0;
print_report_hook g_bsp_print_hook = NULL;
hds_lock_ctrl_info g_hds_lock_ctrl;

static inline void * bsp_MemPhyToVirt(u8 *pucCurPhyAddr, u8 *pucPhyStart, u8 *pucVirtStart, u32 ulBufLen)
{
    if((pucCurPhyAddr < pucPhyStart) || (pucCurPhyAddr >= (pucPhyStart+ulBufLen)))
    {
        hds_debug("bsp_MemPhyToVirt fail!\n");
        return HDS_NULL;
    }
    return (void *)((pucCurPhyAddr - pucPhyStart) + pucVirtStart);
}

void bsp_print_level_cfg(u32 *level)
{
    if((*level) >= BSP_LOG_LEVEL_ERROR)
    {
        (*level) = 1;
    }
    else if(BSP_LOG_LEVEL_WARNING == (*level))
    {
        (*level) =  2;
    }
    else if(BSP_LOG_LEVEL_NOTICE == (*level))
    {
        (*level) =  3;
    }
    else if((BSP_LOG_LEVEL_INFO == (*level)) || (BSP_LOG_LEVEL_DEBUG == (*level)))
    {
        (*level) =  4;
    }
}


s32 bsp_log_write_socp_chan(u8* data,u32 len)
{

    SOCP_BUFFER_RW_STRU wbuf = {NULL,};
    if( HDS_OK != bsp_socp_get_write_buff(SOCP_CODER_SRC_LOG_IND, &wbuf))
    {
       hds_debug("fail to get print buffer,chan=0x%x,len=0x%x\n", SOCP_CODER_SRC_LOG_IND, len);
       return HDS_ERR;
    }

    wbuf.pBuffer = (char *)bsp_MemPhyToVirt((u8*)(wbuf.pBuffer), g_logSrcCfg.pucPhyStart, g_logSrcCfg.pucVirtStart, g_logSrcCfg.ulBufLen);

    if((wbuf.u32Size >= len) && (NULL != wbuf.pBuffer))
    {
        (void)memcpy_s((void*)(wbuf.pBuffer), wbuf.u32Size, data, (unsigned long)len);
        //LOG_FLUSH_CACHE(wbuf.pBuffer, (unsigned long)len);
        if(HDS_OK != bsp_socp_write_done(SOCP_CODER_SRC_LOG_IND, len))
        {
            hds_debug("socp write done fail!\n");
            return HDS_ERR;
        }
    }
    else if(wbuf.u32Size + wbuf.u32RbSize >= len)
    {
        wbuf.pRbBuffer = (char *)bsp_MemPhyToVirt((u8*)(wbuf.pRbBuffer), g_logSrcCfg.pucPhyStart, g_logSrcCfg.pucVirtStart, g_logSrcCfg.ulBufLen);
        if((NULL != wbuf.pBuffer) && (NULL != wbuf.pRbBuffer))
        {
            (void)memcpy_s((void*)(wbuf.pBuffer),(unsigned long)(wbuf.u32Size), data,(unsigned long)(wbuf.u32Size));
            //LOG_FLUSH_CACHE(wbuf.pBuffer, (unsigned long)(wbuf.u32Size));
            (void)memcpy_s((void*)(wbuf.pRbBuffer), wbuf.u32RbSize, data+wbuf.u32Size, (unsigned long)(len-wbuf.u32Size));
            //LOG_FLUSH_CACHE(wbuf.pRbBuffer, (unsigned long)(len-wbuf.u32Size));

            if(HDS_OK != bsp_socp_write_done(SOCP_CODER_SRC_LOG_IND, len))
            {
                hds_debug("socp rb write done fail!\n");
                return HDS_ERR;
            }
        }
    }
    else
    {
        hds_debug("socp buffer not enough!\n");
        return HDS_ERR;
    }

    return HDS_OK;
}

/*lint -save -e550 */
s32 bsp_transreport(TRANS_IND_STRU *pstData)
{
    s32 ret;
    u64 auctime;
    u32 socp_packet_len;
    u32 diag_packet_len;
    u32 trans_packet_len;
    diag_trans_head_stru *trans_head;
    diag_frame_head_stru *diag_head;
    diag_socp_head_stru  *socp_head;
    unsigned long lock_flag = 0;

    /*判断工具连接状态、开关状态*/
    if(0 == g_translog_conn)
    {
       hds_info("hids not conn(%d)!\n",g_translog_conn);
       return HDS_TRANS_SW_ERR;
    }

    /*入参检查*/
    if((NULL == pstData)||(NULL == pstData->pData)||(0 == pstData->ulLength) || ((pstData->ulLength) > (TRANSLOG_MAX_HIDS_BUFF_LEN - 1)))
    {
       hds_info("pstdata err!\n");
       return HDS_ERR;
    }

    trans_head = &g_trans_sendbuf->trans_head;
    diag_head  = &g_trans_sendbuf->diag_head;
    socp_head  = &g_trans_sendbuf->socp_head;

    trans_packet_len = pstData->ulLength + DIAG_TRANS_HEAD_SIZE;
    diag_packet_len  = trans_packet_len + DIAG_FRAME_HEAD_SIZE;
    socp_packet_len  = diag_packet_len  + DIAG_SOCP_HEAD_SIZE;

    spin_lock_irqsave(&g_hds_lock_ctrl.trans_lock,lock_flag);

    (void)memcpy_s(g_trans_sendbuf->data, TRANSLOG_MAX_HIDS_BUFF_LEN, pstData->pData, (unsigned long)(pstData->ulLength));

    /*fill trans head*/
    trans_head->ulModule   = 0x8003;            /*Pid,0x8003代表BSP*/
    trans_head->ulMsgId    = pstData->ulMsgId;
    trans_head->ulNo       = g_Translog_pkgnum++;
    /*fill diag head*/
    bsp_slice_getcurtime(&auctime);
    (void)memcpy_s(diag_head->stService.aucTimeStamp, sizeof(diag_head->stService.aucTimeStamp), &auctime, sizeof(auctime));
    /*pstData->ulModule:31-24bit,代表mdmid3b,主副卡*/
    diag_head->stService.mdmid3b = ((pstData->ulModule) & 0xff000000)>>24;
    diag_head->stService.MsgTransId  = g_translog_transId;

    diag_head->stID.pri4b    = DIAG_FRAME_MSG_TYPE_BSP;
    diag_head->stID.mode4b   = ((pstData->ulModule) & 0x00ff0000)>>16;
    diag_head->stID.sec5b    = DIAG_FRAME_MSG_STRUCT;
    diag_head->stID.cmdid19b = ((pstData->ulMsgId) & 0x7ffff);       /*pstData->ulMsgId:18-0bit,代表cmdid19b*/
    diag_head->u32MsgLen     = trans_packet_len;

    /*fill socp head*/
    socp_head->u32DataLen = diag_packet_len;

    ret = bsp_log_write_socp_chan((u8*)g_trans_sendbuf, socp_packet_len);
    if(ret)
    {
        hds_info("write transdata to socp fail!\n");
        spin_unlock_irqrestore(&g_hds_lock_ctrl.trans_lock,lock_flag);
        return ret;
    }

    /* trans_id++, only if send data successfully*/
    g_translog_transId++;

    spin_unlock_irqrestore(&g_hds_lock_ctrl.trans_lock,lock_flag);
    return HDS_TRANS_RE_SUCC;
}
/*lint -restore +e550 */


s32 bsp_printreport(char *logdata,u32 level,u32 module_id)
{
    s32 ret = 0;
    u32 datalen = 0;
    u32 socp_packet_len  = 0;
    u32 diag_packet_len  = 0;
    u32 print_packet_len = 0;
    diag_print_head_stru *print_head = NULL;
    diag_frame_head_stru *diag_head = NULL;
    diag_socp_head_stru  *socp_head = NULL;

    /*入参检查*/
    if(NULL == logdata)
    {
       hds_printf("logdata err!\n");
       return HDS_ERR;
    }

    datalen = (u32)snprintf_s((char *)g_print_sendbuf->data, PRINTLOG_MAX_BUFF_LEN, (PRINTLOG_MAX_BUFF_LEN - 1),"ap[0]%s", logdata);
    if(datalen > PRINTLOG_MAX_BUFF_LEN)
    {
       hds_info("print datalen err!\n");
       return HDS_ERR;
    }

    print_head = &g_print_sendbuf->print_head;
    diag_head = &g_print_sendbuf->diag_head;
    socp_head = &g_print_sendbuf->socp_head;

    print_packet_len = datalen + DIAG_PRINT_HEAD_SIZE;
    diag_packet_len  = print_packet_len + DIAG_FRAME_HEAD_SIZE;
    socp_packet_len  = diag_packet_len  + DIAG_SOCP_HEAD_SIZE;

    bsp_print_level_cfg(&level);

    /*fill print head*/
    /* 1:error, 2:warning, 3:normal, 4:info */
    /* (0|ERROR|WARNING|NORMAL|INFO|0|0|0) */
    print_head->u32level = (0x80000000) >> level;
    print_head->u32module = 0x8003;
    print_head->u32no = g_printlog_pkgnum++;

    /*fill diag head*/
    bsp_slice_getcurtime((u64*)diag_head->stService.aucTimeStamp);
    diag_head->stService.MsgTransId  = g_printlog_transId;
    diag_head->stID.sec5b    = DIAG_FRAME_MSG_PRINT;
    diag_head->stID.mode4b   = DIAG_FRAME_MODE_COMM;
    diag_head->stID.pri4b    = DIAG_FRAME_MSG_TYPE_BSP;
    diag_head->u32MsgLen     = print_packet_len;

    /*fill socp head*/
    socp_head->u32DataLen   = diag_packet_len;

    ret = bsp_log_write_socp_chan((u8*)g_print_sendbuf, socp_packet_len);
    if(ret)
    {
        hds_info("write printdata to socp fail!\n");
        return ret;
    }

    /* trans_id++, only if send data successfully*/
    g_printlog_transId++;

    return HDS_PRINT_RE_SUCC;
}
/*****************************************************************************
new start
******************************************************************************/
void match_type(u32 *level)
{
	switch(*level){
		case BSP_P_FATAL:	*level = BSP_LOG_LEVEL_FATAL;break;
		case BSP_P_ERR: 	*level = BSP_LOG_LEVEL_ERROR;break;
		case BSP_P_WRN: 	*level = BSP_LOG_LEVEL_WARNING;break;
		case BSP_P_INFO:	*level = BSP_LOG_LEVEL_INFO;break;
		default:			*level = BSP_LOG_LEVEL_DEBUG;
		}
	return;
}
/*****************************************************************************
new start
******************************************************************************/

/*****************************************************************************
* 函 数 名  : bsp_trace
*
* 功能描述  : 底软打印输出处理接口
*
* 输入参数  :  mod_id: 输出模块
*              print_level: 打印级别
*              fmt :打印输入参数
*
* 输出参数  : 无
*
* 返 回 值  : 成功返回0
*****************************************************************************/
/*lint -save -e530 -e830 -e64*/
int bsp_trace_to_hids(u32 module_id, u32 level, u32 sel, char* print_buff)
{
    unsigned long lock_flag;
    int ret = HDS_PRINT_SW_ERR;
    static bool print_flag = false;
	if(0 == sel)		
		match_type(&level);
	if(level < g_printlog_level)
		return HDS_PRINT_SW_ERR;
    spin_lock_irqsave(&g_hds_lock_ctrl.trace_lock,lock_flag);
    if (print_flag)
    {
        hds_info("print data report recursion!\n");
        spin_unlock_irqrestore(&g_hds_lock_ctrl.trace_lock,lock_flag);
        return HDS_PRINT_RECURSION;
    }
    print_flag = true;

    /*判断工具连接状态、开关状态*/
    if((1 == g_printlog_conn)&&(1 == g_printlog_enable))
    {
        ret = bsp_printreport(print_buff,level,module_id);

        if (HDS_PRINT_RE_SUCC != ret)
        {
            hds_info("fail to print data,ret = 0x%x!\n",ret);
        }
        print_flag = false;
        spin_unlock_irqrestore(&g_hds_lock_ctrl.trace_lock,lock_flag);
        return ret;
    }
    else
    {
        //hds_printf("hids disconnect(0x%x) or print switch off(0x%x)!\n",g_printlog_conn, g_printlog_enable);
        print_flag = false;
        spin_unlock_irqrestore(&g_hds_lock_ctrl.trace_lock,lock_flag);
        return ret;
    }
}
/*lint -restore +e550 */
/*lint -save -e429 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
struct platform_device *g_modem_hds_pdev=NULL;

static int hds_driver_probe(struct platform_device *pdev)
{
/*lint -save -e598 -e648 */
    dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));

    g_modem_hds_pdev = pdev;

    return HDS_OK;
/*lint -restore */
}

static const struct of_device_id hds_dev_of_match[] = {
    {
        .compatible = "hisilicon,hds_balong",
        .data = NULL,
    },
    {},
};

static struct platform_driver hds_driver = {
    .probe  = hds_driver_probe,
    .driver = {
        .name = "hds_balong",
        .of_match_table = hds_dev_of_match,
    },
};
#else
u64 g_dma_hds_mask = (u64)(-1);
#endif
s32 bsp_socp_log_chan_cfg(void)
{
    SOCP_CODER_SRC_CHAN_S EncSrcAttr;
    dma_addr_t  ulAddress = 0;
    u8 *p;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    int ret = platform_driver_register(&hds_driver);
    if(ret != 0)
    {
        hds_printf("init failed\n");
        return HDS_ERR;
    }
    if(NULL == g_modem_hds_pdev)
    {
        hds_printf("null pdev");
        return HDS_ERR;
    }
    p =(u8 *) dma_alloc_coherent(&g_modem_hds_pdev->dev, (unsigned long)LOG_SRC_BUF_LEN, &ulAddress, GFP_KERNEL);
#else
    struct device dev;
    /* coverity[secure_coding] */
    memset_s(&dev, sizeof(dev), 0, sizeof(dev));
    dma_set_mask_and_coherent(&dev, g_dma_hds_mask);
    of_dma_configure(&dev, NULL);
    p =(u8 *) dma_alloc_coherent(&dev, (unsigned long)LOG_SRC_BUF_LEN, &ulAddress, GFP_KERNEL);
#endif

    if(HDS_NULL == p)
    {
        hds_printf("log src chan malloc fail!\n");
        return HDS_ERR;
    }

    EncSrcAttr.eDataType   = SOCP_DATA_TYPE_0;
    EncSrcAttr.eDataTypeEn = SOCP_DATA_TYPE_EN;
    EncSrcAttr.eDebugEn    = SOCP_ENC_DEBUG_DIS;
    EncSrcAttr.eMode       = SOCP_ENCSRC_CHNMODE_CTSPACKET;
    EncSrcAttr.ePriority   = SOCP_CHAN_PRIORITY_2;
    EncSrcAttr.u32BypassEn = 0;
    EncSrcAttr.u32DestChanID = SOCP_CODER_DST_OM_IND;

    EncSrcAttr.sCoderSetSrcBuf.pucInputStart = (u8*)ulAddress;
    EncSrcAttr.sCoderSetSrcBuf.pucInputEnd = (u8*)(ulAddress +LOG_SRC_BUF_LEN - 1);


    /*调用SOCP接口进行编码源通道配置*/
    if(HDS_OK != bsp_socp_coder_set_src_chan(SOCP_CODER_SRC_LOG_IND, &EncSrcAttr))
    {

        hds_printf("log src channel set failed!\n");
        return HDS_ERR;
    }

    g_logSrcCfg.pucPhyStart = (u8*)ulAddress;
    g_logSrcCfg.pucVirtStart = (void*)p;
    g_logSrcCfg.ulBufLen = LOG_SRC_BUF_LEN;

    /*启动编码*/
    bsp_socp_start(SOCP_CODER_SRC_LOG_IND);
    return HDS_OK;
}
/*lint -restore +e429 */



int __init bsp_hds_init(void)
{
    int ret;
	hds_printf("[init]start\n");

    bsp_hds_service_init();

    /*上报LOG的SOCP通道*/
    ret=bsp_socp_log_chan_cfg();
    if(ret)
    {
        hds_printf("bsplog src chan fail!\n");
        return HDS_ERR;
    }

    spin_lock_init(&g_hds_lock_ctrl.trace_lock);
    g_bsp_print_hook = (print_report_hook)bsp_trace_to_hids;

    /*为print数据申请buffer*/
    g_print_sendbuf = (print_send_buff*)osl_malloc((unsigned int)sizeof(print_send_buff));
    if(NULL == g_print_sendbuf)
    {
        hds_printf("printbuf malloc fail!\n");
        return HDS_ERR;
    }
    bsp_diag_frame_head_init(&g_print_sendbuf->diag_head);
    bsp_diag_fill_socp_head(&g_print_sendbuf->socp_head,0);

    g_print_init_state=PRINTLOG_CHN_INIT;

    spin_lock_init(&g_hds_lock_ctrl.trans_lock);
    /*为结构化消息数据申请buffer*/

    g_trans_sendbuf = (trans_send_buff*)osl_malloc((unsigned int)sizeof(trans_send_buff));
    if(NULL == g_trans_sendbuf)
    {
        hds_printf("transbuf malloc fail!\n");
        return HDS_ERR;
    }
    bsp_diag_frame_head_init(&g_trans_sendbuf->diag_head);
    bsp_diag_fill_socp_head(&g_trans_sendbuf->socp_head,0);

    g_trans_init_state=TRANSLOG_CHN_INIT;

    hds_printf("[init] ok!\n");
    return HDS_OK;
}

/*lint --e{528}*/
module_init(bsp_hds_init);
EXPORT_SYMBOL(bsp_transreport);
