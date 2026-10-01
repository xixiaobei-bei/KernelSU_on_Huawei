#include "psam_balong.h"

extern struct psam_device *g_psam_device;

void psam_help(void)
{
	bsp_err("psam_ad_info\n");
	bsp_err("psam_dump_ad_info\n");

	bsp_err("********\n");
	bsp_err("ad0 virtual addr  0x%x\n",g_psam_device->adq0_info.vir_addr);

	bsp_err("ad0 physical addr 0x%llx\n",g_psam_device->hal->get_ad0_base());
	bsp_err("ad0 lenth is %d\n",  	 g_psam_device->desc_num[0]);
	bsp_err("ad0 read index is %d\n",  readl(g_psam_device->regs + HI_PSAM_ADQ0_RPTR_OFFSET));
	bsp_err("ad0 write index is %d\n", readl(g_psam_device->regs + HI_PSAM_ADQ0_WPTR_OFFSET));

	bsp_err("ad1 virtual addr  0x%x\n",g_psam_device->adq1_info.vir_addr);

	bsp_err("ad1 physical addr 0x%llx\n",g_psam_device->hal->get_ad1_base());
	bsp_err("ad1 lenth is %d\n",  	 g_psam_device->desc_num[1]);
	bsp_err("ad1 read index is %d\n",  readl(g_psam_device->regs + HI_PSAM_ADQ1_RPTR_OFFSET));
	bsp_err("ad1 write index is %d\n\n", readl(g_psam_device->regs + HI_PSAM_ADQ1_WPTR_OFFSET));

	bsp_err("********ipf rd status\n");
	bsp_err("get_rd_num_times		%d\n", ((struct ipf_debug*)(g_psam_device->ipf_deb))->get_rd_num_times);
	bsp_err("get_rd_times			%d\n", ((struct ipf_debug*)(g_psam_device->ipf_deb))->get_rd_times);
	bsp_err("dlrd short cnt %d\n",((struct ipf_debug*)(g_psam_device->ipf_deb))->get_rd_cnt[0]);
	bsp_err("dlrd long cnt  %d\n",((struct ipf_debug*)(g_psam_device->ipf_deb))->get_rd_cnt[1]);
    bsp_err("free ad numbers:  %d\n", g_psam_device->debug.adq_free_cnt);

	bsp_err("********\n");
	bsp_err("adq0 empty count%d\n",g_psam_device->debug.adq0_epty_int1);
	bsp_err("adq1 empty count%d\n",g_psam_device->debug.adq1_epty_int1);
	bsp_err("upoverflow count%d\n",g_psam_device->debug.lbdq_upoverflow);
	bsp_err("lbdq_full count%d\n",g_psam_device->debug.lbdq_full);
	bsp_err("lbdq_empty count%d\n\n",g_psam_device->debug.lbdq_empty);

	bsp_err("********\n");
	bsp_err("call get_dlad_num cnt is: %d\n\n",g_psam_device->debug.get_dlad_num);

	bsp_err("adq0 success count %d\n",g_psam_device->debug.cfg_dl_ad_succ[0]);
	bsp_err("get_dlad0_num NULL %d\n",g_psam_device->debug.cfg_get_ad_para_null[0]);
	bsp_err("ad0 desc NULL %d\n",g_psam_device->debug.dlad_desc_null[0]);
	bsp_err("adq0 outpointer null %d\n\n",g_psam_device->debug.dlad_outptr_err[0]);

	bsp_err("adq1 success count%d\n",g_psam_device->debug.cfg_dl_ad_succ[1]);
	bsp_err("get_dlad0_num NULL %d\n",g_psam_device->debug.cfg_get_ad_para_null[1]);
	bsp_err("ad1 desc NULL %d\n",g_psam_device->debug.dlad_desc_null[1]);
	bsp_err("adq1 outpointer null %d\n\n",g_psam_device->debug.dlad_outptr_err[1]);

	bsp_err("ad wrong type %d\n",g_psam_device->debug.dlad_type_err);
	bsp_err("ad count error %d\n",g_psam_device->debug.cfg_dlad_num_err);
	bsp_err("scr ad error %d\n",g_psam_device->debug.scr_ad_addr_exc);
}

int psam_ad_info( unsigned int ad_type, unsigned int adq_ptr)
{
    if(adq_ptr >= PSAM_DLAD1_DESC_SIZE)
    {
        return -1;
    }

 	bsp_err("===========DL AD%d Info==========\n",ad_type);
 	bsp_err("AD pos:             %d\n",adq_ptr);
    bsp_err("************************\n");
    return 0;
}

int psam_dump_ad_info(unsigned int ad_type)
{
    unsigned int i;

    for(i = 0;i < PSAM_DLAD1_DESC_SIZE;i++)
    {
        psam_ad_info(i, ad_type);
    }

    return 0;
}

EXPORT_SYMBOL(psam_help);

