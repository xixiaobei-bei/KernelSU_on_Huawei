#include <linux/dma-mapping.h>
#include <osl_malloc.h>
#include "ipf_balong.h"
#include <hi_ipf.h>
#include <bsp_ddr.h>
#include <mdrv_ipf_comm.h>
#include <osl_malloc.h>
#include <linux/dma-mapping.h>
#include <securec.h>

extern struct ipf_ctx g_ipf_ctx;

const unsigned int ipf32_save_table[][2] =
{
    {HI_IPF32_CH0_CTRL_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_BDQ_SIZE_OFFSET, 0xffffffff},
    {HI_IPF32_CH0_BDQ_BADDR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ_CTRL_OFFSET, 0xfffffff0},
    {HI_IPF32_CH1_ADQ0_RPTR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ1_RPTR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ0_WPTR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ1_WPTR_OFFSET, 0xffffffff},
    {HI_IPF32_CH1_ADQ_CTRL_OFFSET, 0xffffffff},
};

/*
    Check whether downstream channel is busy.
    return 1: busy, 0: idle.
*/
static int ipf32_is_busy(void)
{
    HI_IPF_CH1_STATE_T state;

    state.u32 = ipf_readl(HI_IPF32_CH1_STATE_OFFSET);

    return state.bits.dl_busy;
}

static void _ipf32_reg_save(unsigned int* base)
{
    unsigned int i = 0;
  
    for(i=0; i<sizeof(ipf32_save_table)/sizeof(ipf32_save_table[0]); i++)
    {
       base[i] = ipf_readl(ipf32_save_table[i][0]);
    }
}

static void _ipf32_reg_load(unsigned int* base)
{
    unsigned int i=0;

    for(i=0; i<sizeof(ipf32_save_table)/sizeof(ipf32_save_table[0]); i++)
    {
       ipf_writel((base[i]&ipf32_save_table[i][1]), ipf32_save_table[i][0]);
    }
}

void ipf_cd_en_set(void* bd_base, unsigned int index, unsigned short en)
{
	ipf_bd_s* bd = (ipf_bd_s*)bd_base;
	ipf_desc_attr_t* attrp = (ipf_desc_attr_t*)&bd[index].u16Attribute;
	attrp->bits.cd_en = en;
	return;
}

int ipf_cd_en_get(void* bd_base, unsigned int index){
	ipf_bd_s* bd = (ipf_bd_s*)bd_base;
	ipf_desc_attr_t* attrp = (ipf_desc_attr_t*)&bd[index].u16Attribute;
	return attrp->bits.cd_en;
}


void ipf_cd_clear(void* cd_base, unsigned int index)
{
	ipf_cd_s* cd = (ipf_cd_s*)cd_base;
	memset_s(&cd[index], sizeof(cd[index]), 0, sizeof(ipf_cd_s));
}

void ipf_bd_s2h(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index)
{
	ipf_bd_s* bd = (ipf_bd_s*)bd_base;
    bd[index].u16Attribute.u16 = 0;
    bd[index].u16Attribute.bits.int_en = param->int_en;
    bd[index].u16Attribute.bits.fc_head = param->fc_head;
	bd[index].u16Attribute.bits.mode = param->mode;
    bd[index].u32InPtr		= ADDR_TO_U32(param->Data);
    bd[index].u16PktLen		= param->u16Len;
    bd[index].u16UsrField1	= param->u16UsrField1;
    bd[index].u32UsrField2	= param->u32UsrField2;
    bd[index].u32UsrField3	= param->u32UsrField3;
}

void ipf_bd_h2s(IPF_CONFIG_PARAM_S* param, void* bd_base, unsigned int index)
{
	ipf_bd_s* bd = (ipf_bd_s*)bd_base;

	param->int_en	=	bd[index].u16Attribute.bits.int_en;
	param->fc_head  =   bd[index].u16Attribute.bits.fc_head;
	param->mode  =   bd[index].u16Attribute.bits.mode;
    param->Data		=	U32_TO_ADDR(bd[index].u32InPtr);
    param->u16Len		=	bd[index].u16PktLen;
    param->u16UsrField1	=	bd[index].u16UsrField1;
    param->u32UsrField2	=	bd[index].u32UsrField2;
    param->u32UsrField3	=	bd[index].u32UsrField3;
    g_ipf_ctx.last_bd = &bd[index];
}

void ipf_rd_h2s(IPF_RD_DESC_S* param, void* rd_base, unsigned int index)
{
	ipf_rd_s* rd = (ipf_rd_s*)rd_base;
	
	param->u16Attribute	= rd[index].u16Attribute.u16;
	param->u16PktLen	= rd[index].u16PktLen;
	param->u16Result	= rd[index].u16Result;
	param->InPtr		= (modem_phy_addr)(unsigned long)rd[index].u32InPtr;
	param->OutPtr	= U32_TO_ADDR(rd[index].u32UsrField3);
	param->u16UsrField1	= rd[index].u16UsrField1;
	param->u32UsrField2	= rd[index].u32UsrField2;
	param->u32UsrField3	= rd[index].u32UsrField3;
	g_ipf_ctx.last_rd = &rd[index];
	g_ipf_ctx.last_cd = (modem_phy_addr)(unsigned long)rd[index].u32InPtr;
	rd[index].u16Attribute.bits.dl_direct_set?g_ipf_ctx.stax.direct_bd++:g_ipf_ctx.stax.indirect_bd++;
}


void ipf_ad_h2s(IPF_AD_DESC_S* param, void* ad_base, unsigned int index)
{
	ipf_ad_s* ad = (ipf_ad_s*)ad_base;
	param->OutPtr0	= U32_TO_ADDR(ad[index].u32OutPtr0);
	param->OutPtr1	= U32_TO_ADDR(ad[index].u32OutPtr1);
}

unsigned int ipf_last_get(void* cd_base, unsigned int index)
{
    ipf_cd_s* cd = (ipf_cd_s*)cd_base;
    return cd[index].u16Attribute;
}

void ipf32_config(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
	ipf_writel((unsigned int)g_ipf_ctx.ul_info.pstIpfPhyBDQ, HI_IPF32_CH0_BDQ_BADDR_OFFSET);

	ipf_writel((unsigned int)g_ipf_ctx.dl_info.pstIpfPhyRDQ, HI_IPF32_CH1_RDQ_BADDR_OFFSET);
				

    sm->ipf_acore_reg_size = sizeof(ipf32_save_table)/sizeof(ipf32_save_table[0]);
	return;
}

unsigned int ipf32_get_ulbd_num(void)
{
	HI_IPF_CH0_DQ_DEPTH_T dq_depth;

	dq_depth.u32 = ipf_readl(HI_IPF32_CH0_DQ_DEPTH_OFFSET);
	return (unsigned int)(IPF_ULBD_DESC_SIZE - (dq_depth.bits.ul_bdq_depth));
}

unsigned int ipf32_get_ulrd_num(void)
{
    HI_IPF_CH0_DQ_DEPTH_T dq_depth;

    dq_depth.u32 = ipf_readl(HI_IPF32_CH0_DQ_DEPTH_OFFSET);
    return dq_depth.bits.ul_rdq_depth;
}


void ipf32_config_bd(unsigned int u32Num, IPF_CONFIG_ULPARAM_S* pstUlParam)
{
	HI_IPF_CH0_BDQ_WPTR_T bdq_wptr;
	unsigned int u32BD;
    unsigned int i;
	
    bdq_wptr.u32 = ipf_readl(HI_IPF32_CH0_BDQ_WPTR_OFFSET);
    u32BD = bdq_wptr.bits.ul_bdq_wptr;
    for(i = 0; i < u32Num; i++)
    {
		g_ipf_ctx.desc->bd_s2h(&pstUlParam[i], g_ipf_ctx.ul_info.pstIpfBDQ, u32BD);
		g_ipf_ctx.desc->cd_en_set(g_ipf_ctx.ul_info.pstIpfBDQ, u32BD, ipf_disable);

        u32BD = ((u32BD + 1) < IPF_ULBD_DESC_SIZE)? (u32BD + 1) : 0;
    }

	g_ipf_ctx.status->cfg_bd_cnt += u32Num;

    ipf_writel(u32BD, HI_IPF32_CH0_BDQ_WPTR_OFFSET);
}

void ipf32_get_dlrd(unsigned int* pu32Num, IPF_RD_DESC_S *pstRd)
{
    unsigned int u32RdqRptr;
    unsigned int u32RdqDepth;
    unsigned int u32Num;
    unsigned int i;
    unsigned int u32CdqRptr;
    HI_IPF_CH1_DQ_DEPTH_T dq_depth;

    dq_depth.u32 = ipf_readl(HI_IPF32_CH1_DQ_DEPTH_OFFSET);
    u32RdqDepth = dq_depth.bits.dl_rdq_depth;

	g_ipf_ctx.status->get_rd_times++;
	
    u32Num = (u32RdqDepth < *pu32Num)?u32RdqDepth:*pu32Num;
    if(0 == u32Num)
    {
        *pu32Num = 0;
        return;
    }

    u32RdqRptr = ipf_readl(HI_IPF32_CH1_RDQ_RPTR_OFFSET);
    for(i = 0; i < u32Num; i++)
    {
    /*
        dma_unmap_single(g_ipf_ctx.dev, (dma_addr_t)((char *)g_ipf_ctx.dl_info.pstIpfPhyRDQ + u32RdqRptr*sizeof(ipf_rd_s)), 
									sizeof(ipf_rd_s), DMA_FROM_DEVICE);
	*/
		g_ipf_ctx.desc->rd_h2s(&pstRd[i], g_ipf_ctx.dl_info.pstIpfRDQ, u32RdqRptr);
        if(ipf_enable == g_ipf_ctx.desc->cd_en_get(g_ipf_ctx.dl_info.pstIpfRDQ, u32RdqRptr)){
        	u32CdqRptr = ((unsigned long)SHD_DDR_P2V((void *)(unsigned long)MDDR_FAMA(pstRd[i].InPtr)) - (unsigned 
        	long)g_ipf_ctx.dl_info.pstIpfCDQ)/(unsigned long)sizeof(ipf_cd_s); //lint !e712

            while(g_ipf_ctx.desc->cd_last_get(g_ipf_ctx.dl_info.pstIpfCDQ, u32CdqRptr) != 1)
            {
                u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
            }
            u32CdqRptr = ((u32CdqRptr+1) < IPF_DLCD_DESC_SIZE)?(u32CdqRptr+1):0;
            *(g_ipf_ctx.dl_info.u32IpfCdRptr) = u32CdqRptr;
        }
		ipf_pm_print_packet((void *)(unsigned long)MDDR_FAMA(pstRd[i].OutPtr), (unsigned int)pstRd[i].u16PktLen);//lint !e747

        u32RdqRptr = ((u32RdqRptr+1) < IPF_DLRD_DESC_SIZE)?(u32RdqRptr+1):0;
		pstRd[i].u16PktLen > (g_ipf_ctx.status->ad_thred)? g_ipf_ctx.status->get_rd_cnt[IPF_AD_1]++:\
						   		  					   g_ipf_ctx.status->get_rd_cnt[IPF_AD_0]++;

        g_ipf_ctx.status->rd_len_update += pstRd[i].u16PktLen;
    /*
        dma_map_single(g_ipf_ctx.dev, g_ipf_ctx.dl_info.pstIpfRDQ + u32RdqRptr*sizeof(ipf_rd_s), sizeof(ipf_rd_s), DMA_FROM_DEVICE);
    */
    }

    ipf_writel(u32RdqRptr, HI_IPF32_CH1_RDQ_RPTR_OFFSET);
    *pu32Num = u32Num;
}

unsigned int ipf32_get_dlrd_num(void)
{
    HI_IPF_CH1_DQ_DEPTH_T dq_depth;

    /* 读取RD深度 */
    dq_depth.u32 = ipf_readl(HI_IPF32_CH1_DQ_DEPTH_OFFSET);
	g_ipf_ctx.status->get_rd_num_times++;
    return dq_depth.bits.dl_rdq_depth;
}

void ipf32_dump_callback(void)
{
    unsigned char* p = g_ipf_ctx.dump_area;
    unsigned int size = IPF_DUMP_SIZE;

    if(size<(sizeof(ipf64_rd_s)+sizeof(ipf64_bd_s)+sizeof(ipf64_ad_s)+sizeof(ipf64_ad_s)))
    {
        return;
    }
    if(!p) return;
    if(!g_ipf_ctx.last_rd) return;
    memcpy_s(p, size, g_ipf_ctx.last_rd, sizeof(ipf_rd_s));
    p += sizeof(ipf_rd_s);
    size -= sizeof(ipf_rd_s);

    if(!g_ipf_ctx.last_bd) return;
    memcpy_s(p, size, g_ipf_ctx.last_bd, sizeof(ipf_bd_s));
    p += sizeof(ipf_bd_s);
    size -= sizeof(ipf_bd_s);

    if(!g_ipf_ctx.last_ad0) return;
    memcpy_s(p, size, g_ipf_ctx.last_ad0, sizeof(ipf_ad_s));
    p += sizeof(ipf_ad_s);
    size -= sizeof(ipf_ad_s);

    if(!g_ipf_ctx.last_ad1) return;
    memcpy_s(p, size, g_ipf_ctx.last_ad1, sizeof(ipf_ad_s));
}

void ipf_acpu_wake_ccpu(void)
{
    unsigned int reg;
    UPDATE1(reg, HI_IPF_INT_MASK0, dl_rdq_downoverflow_mask0, 1);
    ipf_writel(0, HI_IPF32_CH1_RDQ_RPTR_OFFSET);
}

struct ipf_desc_handler_s ipf_desc_handler = {
    .name = "ipf_desc",
    .dma_mask = 0xffffffffULL,
	.config = ipf32_config,
	.cd_en_set = ipf_cd_en_set,
	.cd_en_get = ipf_cd_en_get,
	.cd_clear = ipf_cd_clear,
	.bd_s2h = ipf_bd_s2h,
	.bd_h2s = ipf_bd_h2s,
	.rd_h2s = ipf_rd_h2s,
	.ad_h2s = ipf_ad_h2s,
	.cd_last_get = ipf_last_get,
	.get_bd_num = ipf32_get_ulbd_num,
	.get_ulrd_num = ipf32_get_ulrd_num,
	.config_bd = ipf32_config_bd,
	.get_rd = ipf32_get_dlrd,
	.get_dlrd_num = ipf32_get_dlrd_num,
	.is_busy = ipf32_is_busy,
	.save_reg = _ipf32_reg_save,
    .restore_reg = _ipf32_reg_load,
    .acpu_wake_ccpu = ipf_acpu_wake_ccpu,
    .dump = ipf32_dump_callback,
};

EXPORT_SYMBOL(ipf_desc_handler);


