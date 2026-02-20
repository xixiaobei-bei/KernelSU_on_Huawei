#include "psam_balong.h"
#include "n_bsp_psam.h"

extern struct psam_device *g_psam_device;

const unsigned int g_psam32_setting_table[][2] ={
    {HI_PSAM_ADQ_CTRL_OFFSET, 0xfffffff0},
    {HI_PSAM32_ADQ0_BASE_OFFSET, 0xffffffff},
    {HI_PSAM_ADQ0_RPTR_OFFSET, 0xffffffff}, 
    {HI_PSAM_ADQ0_WPTR_OFFSET, 0xffffffff}, 
    {HI_PSAM32_ADQ1_BASE_OFFSET, 0xffffffff},
    {HI_PSAM_ADQ1_WPTR_OFFSET, 0xffffffff},
    {HI_PSAM_ADQ1_RPTR_OFFSET, 0xffffffff},
    {HI_PSAM_ADQ_CTRL_OFFSET, 0xffffffff},
};

void psam32_save_reg(unsigned int *base)
{    
    unsigned int i = 0;

    for(i=0; i<sizeof(g_psam32_setting_table)/sizeof(g_psam32_setting_table[0]); i++)
    {
       base[i] = psam_readl(g_psam32_setting_table[i][0]);
    }
}

void psam32_restore_reg(unsigned int *base)
{
    unsigned int i=0;

    for(i=0; i<sizeof(g_psam32_setting_table)/sizeof(g_psam32_setting_table[0]); i++)
    {
       psam_writel((base[i]&g_psam32_setting_table[i][1]), g_psam32_setting_table[i][0]);
    }
}

u64 psam32_get_ad0_base(void)
{
	return MDDR_FAMA_FROM(psam_readl(HI_PSAM32_ADQ0_BASE_OFFSET));//lint !e571
}

u64 psam32_get_ad1_base(void)
{
	return MDDR_FAMA_FROM(psam_readl(HI_PSAM32_ADQ1_BASE_OFFSET));//lint !e571
}

void psam32_set_ad_base(void)
{
    HI_PSAM_INT0_MASK_T mask;
	HI_PSAM_ADQ_CTRL_T ctrl;
	struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
	psam_writel(g_psam_device->dma[0], HI_PSAM32_ADQ0_BASE_OFFSET);
	psam_writel(g_psam_device->dma[1], HI_PSAM32_ADQ1_BASE_OFFSET);

	mask.u32 = 0;
    mask.bits.adq0_epty_mask0 = 1;
    mask.bits.adq1_epty_mask0 = 1;
	writel(mask.u32, g_psam_device->regs + HI_PSAM_INT0_MASK_OFFSET);
	writel(0, g_psam_device->regs + HI_PSAM_INT1_MASK_OFFSET);
	writel(0, g_psam_device->regs + HI_PSAM_INT2_MASK_OFFSET);
	writel(0xffffffff, g_psam_device->regs + HI_PSAM_INT0_STAT_OFFSET);

	ctrl.u32 = 0;
    ctrl.bits.adq_en = 0x3;
    ctrl.bits.adq0_size_sel = IPF_DL_ADQ_LEN_EXP;
    ctrl.bits.adq1_size_sel = IPF_DL_ADQ_LEN_EXP;
    ctrl.bits.adq_plen_th = 404;
	writel(ctrl.u32, g_psam_device->regs + HI_PSAM_ADQ_CTRL_OFFSET);

	sm->psam_acore_reg_size = sizeof(g_psam32_setting_table)/sizeof(g_psam32_setting_table[0]);
}

int psam32_config_ad(IPF_AD_TYPE_E type, unsigned int n, IPF_AD_DESC_S * param)
{
	unsigned int i;
	unsigned int wptr;
	unsigned int offset;
	unsigned int size;
	ipf_ad_s* ad;

	ad = 	g_psam_device->desc[type];
			
	offset = (IPF_AD_0==type)?
			 HI_PSAM_ADQ0_WPTR_OFFSET:
			 HI_PSAM_ADQ1_WPTR_OFFSET;

	size = (IPF_AD_0==type)?
			 IPF_DLAD0_DESC_SIZE:
			 IPF_DLAD1_DESC_SIZE;

	wptr = psam_readl(offset);
	for(i=0; i < n; i++)
	{
		if(0 == param->OutPtr1)
		{
			return -1;
		}
		ad[wptr].u32OutPtr0	= ADDR_TO_U32(param[i].OutPtr0);
		ad[wptr].u32OutPtr1	= ADDR_TO_U32(param[i].OutPtr1);
		wptr = ((wptr + 1) < size)? (wptr + 1) : 0;
	}
	psam_writel(wptr, offset);

	(void)offset;
	return 0;
}

struct psam_hal_handle psam32_hal = {
	.get_ad0_base = psam32_get_ad0_base,
	.get_ad1_base = psam32_get_ad1_base,
	.set_ad_base = psam32_set_ad_base,
	.config_ad = psam32_config_ad,
	.save_reg = psam32_save_reg,
	.restore_reg = psam32_restore_reg,
};

