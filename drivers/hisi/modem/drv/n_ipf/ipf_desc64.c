#include <linux/dma-mapping.h>
#include <osl_malloc.h>
#include "ipf_balong.h"
#include <hi_ipf.h>
#include <bsp_ddr.h>
#include <mdrv_ipf_comm.h>
#include <osl_malloc.h>
#include <linux/dma-mapping.h>
#include <bsp_ipc.h>
#include <linux/skbuff.h>
#include <bsp_slice.h>
#include <securec.h>
#include <../../adrv/adrv.h>

extern struct ipf_ctx g_ipf_ctx;

const unsigned int ipf64_save_table[][2] =
{
    {HI_IPF64_CH0_CTRL_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_BDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_BDQ_BADDR_L_OFFSET, 0xffffffff},
    {HI_IPF64_CH0_BDQ_BADDR_H_OFFSET, 0xffffffff},
    /*
    {HI_IPF64_CH1_ADQ_CTRL_OFFSET, 0xfffffff0},
    {HI_IPF64_CH1_ADQ0_RPTR_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ1_RPTR_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ0_WPTR_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ1_WPTR_OFFSET, 0xffffffff},
    {HI_IPF64_CH1_ADQ_CTRL_OFFSET, 0xffffffff},
    */
};

/*
    Check whether downstream channel is busy.
    return 1: busy, 0: idle.
*/
static int ipf64_is_busy(void)
{
    HI_IPF_CH0_STATE_T state0;
    HI_IPF_CH1_STATE_T state1;
    int ret = 0;

    state0.u32 = ipf_readl(HI_IPF64_CH0_STATE_OFFSET);
    state1.u32 = ipf_readl(HI_IPF64_CH1_STATE_OFFSET);

    if(state0.bits.ul_busy){
        g_ipf_ctx.stax.ch0_busy++;
        ret = 1;
    }

    if(state1.bits.dl_busy){
        g_ipf_ctx.stax.ch1_busy++;
        ret = 1;
    }

    return ret;
}

static void _ipf64_reg_save(unsigned int* base)
{
    unsigned int i = 0;

    for(i=0; i<sizeof(ipf64_save_table)/sizeof(ipf64_save_table[0]); i++)
    {
       base[i] = ipf_readl(ipf64_save_table[i][0]);
    }
}

static void _ipf64_reg_load(unsigned int* base)
{
    unsigned int i=0;
    
    for(i=0; i<sizeof(ipf64_save_table)/sizeof(ipf64_save_table[0]); i++)
    {
       ipf_writel((base[i]&ipf64_save_table[i][1]), ipf64_save_table[i][0]);
    }
}

void ipf64_cd_en_set(void* bd_base, unsigned int index, unsigned short en)
{
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	ipf_desc_attr_t* attrp = (ipf_desc_attr_t*)&bd[index].attribute;
	attrp->bits.cd_en = en;
	return;
}

int ipf64_cd_en_get(void* bd_base, unsigned int index){
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	ipf_desc_attr_t* attrp = (ipf_desc_attr_t*)&bd[index].attribute;
	return attrp->bits.cd_en;
}

void ipf64_cd_clear(void* cd_base, unsigned int index)
{
	ipf64_cd_s* cd = (ipf64_cd_s*)cd_base;
	memset_s(&cd[index], sizeof(cd[index]), 0, sizeof(ipf64_cd_s));
}

void ipf64_bd_s2h(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index)
{
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	bd[index].attribute.u16 = 0;
    bd[index].attribute.bits.int_en		= param->int_en;
    bd[index].attribute.bits.fc_head	= param->fc_head;
	bd[index].attribute.bits.mode		= param->mode;
    bd[index].input_ptr.addr= (unsigned long)param->Data;
    bd[index].pkt_len		= param->u16Len;
    bd[index].user_field1	= param->u16UsrField1;
    bd[index].user_field2	= param->u32UsrField2;
    bd[index].user_field3	= param->u32UsrField3;
}

void ipf64_bd_h2s(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index)
{
	ipf64_bd_s* bd = (ipf64_bd_s*)bd_base;
	
    param->int_en	=	bd[index].attribute.bits.int_en;
    param->fc_head  =   bd[index].attribute.bits.fc_head;
	param->mode  	=   bd[index].attribute.bits.mode;
    param->Data		=	(modem_phy_addr)(unsigned long)bd[index].input_ptr.addr;
    param->u16Len		=	bd[index].pkt_len;
    param->u16UsrField1	=	bd[index].user_field1;
    param->u32UsrField2	=	bd[index].user_field2;
    param->u32UsrField3	=	bd[index].user_field3;

    g_ipf_ctx.last_bd = &bd[index];
}

void ipf64_rd_h2s(IPF_RD_DESC_S* param, void* rd_base, unsigned int index)
{
    static ipf64_rd_s tmp;
    ipf64_rd_s* rd = (ipf64_rd_s*)rd_base;

	memcpy_s(&tmp, sizeof(tmp), &rd[index], sizeof(ipf64_rd_s));
	param->u16Attribute	= tmp.attribute.u16;
	param->u16PktLen	= tmp.pkt_len;
	param->u16Result	= tmp.result;
	param->InPtr		= (modem_phy_addr)(unsigned long)tmp.input_ptr.addr;
	param->OutPtr	= (modem_phy_addr)(unsigned long)tmp.virt.ptr;
	param->u16UsrField1	= tmp.user_field1;
	param->u32UsrField2	= tmp.user_field2;
	param->u32UsrField3	= tmp.user_field3;

	g_ipf_ctx.last_rd = &rd[index];
	g_ipf_ctx.last_cd = (modem_phy_addr)(unsigned long)tmp.input_ptr.addr;
	
	tmp.attribute.bits.dl_direct_set?g_ipf_ctx.stax.direct_bd++:g_ipf_ctx.stax.indirect_bd++;
}


void ipf64_ad_h2s(IPF_AD_DESC_S* param, void* ad_base, unsigned int index)
{
	ipf64_ad_s* ad = (ipf64_ad_s*)ad_base;
	param->OutPtr0	= (modem_phy_addr)(unsigned long)ad[index].output_ptr0.bits.ptr;
	param->OutPtr1	= (modem_phy_addr)(unsigned long)ad[index].output_ptr1.bits.ptr;
}

unsigned int ipf64_last_get(void* cd_base, unsigned int index)
{
    ipf64_cd_s* cd = (ipf64_cd_s*)cd_base;
    return cd[index].cd_last;
}

void ipf64_config(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
    phy_addr_write(g_ipf_ctx.ul_info.pstIpfPhyBDQ, 
	                (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH0_BDQ_BADDR_H_OFFSET,
		            (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH0_BDQ_BADDR_L_OFFSET);

    phy_addr_write(g_ipf_ctx.dl_info.pstIpfPhyRDQ,
                    (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_RDQ_BADDR_H_OFFSET,
	                (unsigned char*)g_ipf_ctx.regs + HI_IPF64_CH1_RDQ_BADDR_L_OFFSET);
                
    sm->ipf_acore_reg_size = sizeof(ipf64_save_table)/sizeof(ipf64_save_table[0]);
	return;
}

unsigned int ipf64_get_ulbd_num(void)
{
	HI_IPF_CH0_BDQ_DEPTH_T dq_depth;
	
	/* 计算空闲BD数量 */
		dq_depth.u32 = ipf_readl(HI_IPF64_CH0_BDQ_DEPTH_OFFSET);
		return (unsigned int)(IPF_ULBD_DESC_SIZE - dq_depth.bits.ul_bdq_depth);
}

unsigned int ipf64_get_ulrd_num(void)
{
    HI_IPF_CH0_RDQ_DEPTH_T dq_depth;

    dq_depth.u32 = ipf_readl(HI_IPF64_CH0_RDQ_DEPTH_OFFSET);
    return dq_depth.bits.ul_rdq_depth;
}

void ipf64_config_bd(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlParam)
{
	HI_IPF_CH0_BDQ_WPTR_T bdq_wptr;
	unsigned int u32BD;
    unsigned int i;
	
	/* è¯»å‡ºBDå†™æŒ‡é’ˆ,å°†u32BdqWpträ½œä¸ºä¸´æ—¶å†™æŒ‡é’ˆä½¿ç”¨ */
    bdq_wptr.u32 = ipf_readl(HI_IPF64_CH0_BDQ_WPTR_OFFSET);
    u32BD = bdq_wptr.bits.ul_bdq_wptr;
    for(i = 0; i < u32Num; i++)
    {
		g_ipf_ctx.desc->bd_s2h(&pstUlParam[i], g_ipf_ctx.ul_info.pstIpfBDQ, u32BD);
		g_ipf_ctx.desc->cd_en_set(g_ipf_ctx.ul_info.pstIpfBDQ, u32BD, ipf_disable);

        u32BD = ((u32BD + 1) < IPF_ULBD_DESC_SIZE)? (u32BD + 1) : 0;
    }

	g_ipf_ctx.status->cfg_bd_cnt += u32Num;

    /* æ›´æ–°BDå†™æŒ‡é’ˆ*/
    ipf_writel(u32BD, HI_IPF64_CH0_BDQ_WPTR_OFFSET);
}

void __attribute__((weak)) ipf_get_waking_pkt(void* data, unsigned int len)
{
    return;
};

void ipf_waking_pkt_pick(void *buf, size_t len)
{
    struct sk_buff* skb = NULL;

    if (g_ipf_ctx.status->resume_with_intr){
        if (!virt_addr_valid(buf)){//lint !e648
            skb = phys_to_virt((unsigned long)buf);
            if(!virt_addr_valid(skb)){//lint !e648
                return;
            }
        }else
        {
            return;
        }

        if (!virt_addr_valid(skb->data)){/*lint !e644 !e648*/
            return;
        }

        dma_unmap_single(g_ipf_ctx.dev, (dma_addr_t)virt_to_phys(skb->data), len, DMA_FROM_DEVICE);

        

        g_ipf_ctx.stax.pkt_dbg_in = bsp_get_slice_value();
        ipf_get_waking_pkt(skb->data, skb->len);
        g_ipf_ctx.stax.pkt_dbg_out = bsp_get_slice_value();
        
        g_ipf_ctx.stax.wakeup_irq++;
        g_ipf_ctx.status->resume_with_intr = 0;
    }

    return;
}

void ipf64_get_dlrd(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd)
{
    unsigned int u32RdqRptr;
    unsigned int u32RdqDepth;
    unsigned int u32Num;
    unsigned int i;
    unsigned int u32CdqRptr;
    HI_IPF_CH1_RDQ_DEPTH_T dq_depth;

    /* 读取RD深度 */
    dq_depth.u32 = ipf_readl(HI_IPF64_CH1_RDQ_DEPTH_OFFSET);
    u32RdqDepth = dq_depth.bits.dl_rdq_depth;

    g_ipf_ctx.status->get_rd_times++;
    
    u32Num = (u32RdqDepth < *pu32Num)?u32RdqDepth:*pu32Num;
    if(0 == u32Num)
    {
        *pu32Num = 0;
        return;
    }

    u32RdqRptr = ipf_readl(HI_IPF64_CH1_RDQ_RPTR_OFFSET);
    for(i = 0; i < u32Num; i++)
    {
    /*
        dma_unmap_single(g_ipf_ctx.dev, (dma_addr_t)((char *)g_ipf_ctx.dl_info.pstIpfPhyRDQ+ u32RdqRptr*sizeof(ipf64_rd_s)),
                                                                sizeof(ipf64_rd_s), DMA_FROM_DEVICE);
    */
        g_ipf_ctx.desc->rd_h2s(&pstRd[i], g_ipf_ctx.dl_info.pstIpfRDQ, u32RdqRptr);
        if(ipf_enable == g_ipf_ctx.desc->cd_en_get(g_ipf_ctx.dl_info.pstIpfRDQ, u32RdqRptr)){
            /* 更新CD读指针 */
            u32CdqRptr = ((unsigned long)SHD_DDR_P2V((void *)pstRd[i].InPtr) - (unsigned 
            long)g_ipf_ctx.dl_info.pstIpfCDQ)/(unsigned long)sizeof(ipf64_cd_s);//lint !e712

            while(g_ipf_ctx.desc->cd_last_get(g_ipf_ctx.dl_info.pstIpfCDQ, u32CdqRptr) != 1)
            {
                u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
            }
            u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
            *(g_ipf_ctx.dl_info.u32IpfCdRptr) = u32CdqRptr;
        }
        ipf_waking_pkt_pick((void *)pstRd[i].OutPtr, (size_t)pstRd[i].u16PktLen);
        /* 更新RD读指针 */
        u32RdqRptr = ((u32RdqRptr+1) < IPF_DLRD_DESC_SIZE)?(u32RdqRptr+1):0;
        pstRd[i].u16PktLen > (g_ipf_ctx.status->ad_thred)? g_ipf_ctx.status->get_rd_cnt[IPF_AD_1]++:\
                                                       g_ipf_ctx.status->get_rd_cnt[IPF_AD_0]++;

        g_ipf_ctx.status->rd_len_update += pstRd[i].u16PktLen;
    /*
        dma_map_single(g_ipf_ctx.dev, g_ipf_ctx.dl_info.pstIpfRDQ + u32RdqRptr*sizeof(ipf64_rd_s), sizeof(ipf64_rd_s), DMA_FROM_DEVICE);
    */
    }

    ipf_writel(u32RdqRptr, HI_IPF64_CH1_RDQ_RPTR_OFFSET);
    *pu32Num = u32Num;
}

unsigned int ipf64_get_dlrd_num(void)
{
    HI_IPF_CH1_RDQ_DEPTH_T dq_depth;

    /* 读取RD深度 */
    dq_depth.u32 = ipf_readl(HI_IPF64_CH1_RDQ_DEPTH_OFFSET);
	g_ipf_ctx.status->get_rd_num_times++;
    return dq_depth.bits.dl_rdq_depth;
}

void ipf64_dump_callback(void)
{
    unsigned char* p = g_ipf_ctx.dump_area;
    unsigned int size = IPF_DUMP_SIZE;

    if(size<(sizeof(ipf64_rd_s)+sizeof(ipf64_bd_s)+sizeof(ipf64_ad_s)+sizeof(ipf64_ad_s)))
    {
        return;
    }
    
    if(!p) return;
    if(!g_ipf_ctx.last_rd) return;
    memcpy_s(p, size,  g_ipf_ctx.last_rd, sizeof(ipf64_rd_s));
    p += sizeof(ipf64_rd_s);
    size -= sizeof(ipf64_rd_s);

    if(!g_ipf_ctx.last_bd) return;
    memcpy_s(p, size, g_ipf_ctx.last_bd, sizeof(ipf64_bd_s));
    p += sizeof(ipf64_bd_s);
    size -= sizeof(ipf64_bd_s);

    if(!g_ipf_ctx.last_ad0) return;
    memcpy_s(p, size, g_ipf_ctx.last_ad0, sizeof(ipf64_ad_s));
    p += sizeof(ipf64_ad_s);
    size -= sizeof(ipf64_ad_s);

    if(!g_ipf_ctx.last_ad1) return;
    memcpy_s(p, size, g_ipf_ctx.last_ad1, sizeof(ipf64_ad_s));

    memcpy_s(&g_ipf_ctx.share_mem->drd, sizeof(g_ipf_ctx.share_mem->drd),
            g_ipf_ctx.dl_info.pstIpfRDQ, sizeof(ipf_drd_u));
}

void ipf64_acpu_wake_ccpu(void)
{
    unsigned int reg;
    unsigned long flags;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);

    if(IPF_PWR_DOWN == sm->init.status.ccore){
        UPDATE1(reg, HI_IPF_TIMER_LOAD,
                    timer_load, IPF_FAST_TIMEOUT);

        UPDATE3(reg, HI_IPF_TRANS_CNT_CTRL, 
                    timer_en, 1,
                    timer_clear, 1,
                    timer_prescaler, IPF_FAST_TIMEOUT);
        g_ipf_ctx.stax.timer_start++;
        g_ipf_ctx.stax.wake_time = bsp_get_slice_value();
    }

    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, flags);
}

struct ipf_desc_handler_s ipf_desc64_handler = {
    .name = "ipf64_desc",
    .dma_mask = 0xffffffffffffffffULL,
    .config = ipf64_config,
    .cd_en_set = ipf64_cd_en_set,
    .cd_en_get = ipf64_cd_en_get,
    .cd_clear = ipf64_cd_clear,
    .bd_s2h = ipf64_bd_s2h,
    .bd_h2s = ipf64_bd_h2s,
    .rd_h2s = ipf64_rd_h2s,
    .ad_h2s = ipf64_ad_h2s,
    .cd_last_get = ipf64_last_get,
    .get_bd_num = ipf64_get_ulbd_num,
    .get_ulrd_num = ipf64_get_ulrd_num,
    .config_bd = ipf64_config_bd,
    .get_rd = ipf64_get_dlrd,
    .get_dlrd_num = ipf64_get_dlrd_num,
    .is_busy = ipf64_is_busy,
    .save_reg = _ipf64_reg_save,
    .restore_reg = _ipf64_reg_load,
    .acpu_wake_ccpu = ipf64_acpu_wake_ccpu,
    .dump = ipf64_dump_callback,
};

EXPORT_SYMBOL(ipf_desc64_handler);

