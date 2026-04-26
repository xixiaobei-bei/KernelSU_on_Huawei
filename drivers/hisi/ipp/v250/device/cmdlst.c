

#define LOG_TAG LOG_MODULE_CMDLST_DRV

#include "linux/printk.h"

#include "include/cmdlst_drv.h"
#include "include/cmdlst_drv_priv.h"
#include "include/cmdlst_reg_offset.h"

static int cmdlst_set_config(cmdlst_dev_t *dev, cmdlst_cfg_t *cfg)
{
	union u_cmdlst_cfg tmp_cfg;
	tmp_cfg.u32 = hispcpe_reg_get(CMDLIST_REG, CMDLST_CFG_REG);
	tmp_cfg.bits.slowdown_nrt_channel = cfg->slowdown_nrt_channel;
	tmp_cfg.bits.prefetch = cfg->prefetch;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_CFG_REG, tmp_cfg.u32);
	return CPE_FW_OK;
}

static int cmdlst_set_sw_ch(cmdlst_dev_t *dev, cmdlst_ch_cfg_t *cfg, unsigned int channel_id)
{
	union u_cmdlst_ch_cfg tmp_cfg;
	tmp_cfg.u32 = hispcpe_reg_get(CMDLIST_REG, CMDLST_CH_CFG_0_REG + 0x80 * channel_id);
	tmp_cfg.bits.active_token_nbr = cfg->active_token_nbr;
	tmp_cfg.bits.active_token_nbr_enable = cfg->active_token_nbr_en;
	tmp_cfg.bits.nrt_channel = cfg->nrt_channel;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_CH_CFG_0_REG + 0x80 * channel_id, tmp_cfg.u32);
	return CPE_FW_OK;
}

static int cmdlst_set_sw_ch_mngr(cmdlst_dev_t *dev, cmdlst_sw_ch_mngr_cfg_t *cfg, unsigned int channel_id)
{
	union u_cmdlst_sw_ch_mngr tmp_cfg;
	tmp_cfg.u32 = hispcpe_reg_get(CMDLIST_REG, CMDLST_SW_CH_MNGR_0_REG + 0x80 * channel_id);
	tmp_cfg.bits.sw_link_channel = cfg->sw_link_channel;
	tmp_cfg.bits.sw_link_token_nbr = cfg->sw_link_token_nbr;
	tmp_cfg.bits.sw_resource = cfg->sw_ressource_request;
	tmp_cfg.bits.sw_priority = cfg->sw_priority;
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CH_MNGR_0_REG + 0x80 * channel_id, tmp_cfg.u32);
	return CPE_FW_OK;
}

int cmdlst_set_vp_rd(cmdlst_dev_t *dev, cmdlst_vp_rd_cfg_t *cfg)
{
	loge_if_ret(NULL == cfg);
	hispcpe_reg_set(CVDR_REG, 0xac4, 0x00000000);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_ADDR_0_REG + 0x80 * cfg->vp_rd_id, 0x03006AB0);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_0_0_REG + 0x80 * cfg->vp_rd_id,
					(((cfg->rd_addr + cfg->rd_size) >> 2) & 0xffffE000) | 0x0000012E);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_1_0_REG + 0x80 * cfg->vp_rd_id, (cfg->rd_size >> 3) - 1);
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_CVDR_RD_DATA_3_0_REG + 0x80 * cfg->vp_rd_id, cfg->rd_addr >> 2);
	return CPE_FW_OK;
}

int cmdlst_do_config(cmdlst_dev_t *dev, cfg_tab_cmdlst_t *config_table)
{
	loge_if_ret(cmdlst_set_config(dev, &config_table->cfg));
	loge_if_ret(cmdlst_set_sw_ch_mngr(dev, &config_table->sw_ch_mngr_cfg, config_table->vp_rd_cfg.vp_rd_id));
	loge_if_ret(cmdlst_set_vp_rd(dev, &config_table->vp_rd_cfg));
	return CPE_FW_OK;
}

static int cmdlst_set_branch(cmdlst_dev_t *dev, int ch_id)
{
	hispcpe_reg_set(CMDLIST_REG, CMDLST_SW_BRANCH_0_REG + 0x80 * ch_id, 0x00000001);
	return CPE_FW_OK;
}

int cmdlst_get_state(cmdlst_dev_t *dev, cmdlst_state_t *st)
{
	st->ch_state = hispcpe_reg_get(CMDLIST_REG, CMDLST_CHANNEL_DEBUG_0_REG + 0x80 * st->ch_id);
	st->last_exec = hispcpe_reg_get(CMDLIST_REG, CMDLST_LAST_EXEC_RD_DATA_3_0_REG + 0x80 * st->ch_id);
	return CPE_FW_OK;
}

static cmdlst_ops_t cmdlst_ops = {
	.set_config = cmdlst_set_config,
	.set_sw_ch  = cmdlst_set_sw_ch,
	.set_sw_ch_mngr  = cmdlst_set_sw_ch_mngr,
	.set_vp_rd = cmdlst_set_vp_rd,
	.do_config = cmdlst_do_config,
	.set_branch = cmdlst_set_branch,
	.get_state = cmdlst_get_state,
};

cmdlst_dev_t g_cmdlst_devs[] = {
	[0] =
	{
		.base_addr = JPG_CMDLST_ADDR,
		.ops = &cmdlst_ops,
	},
};

/********************************** END **********************************/


