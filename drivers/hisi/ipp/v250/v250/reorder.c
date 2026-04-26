
#include <linux/printk.h>
#include <linux/string.h>
#include "ipp.h"
#include "reorder_drv.h"
#include "reorder_drv_priv.h"
#include "reorder_reg_offset.h"
#include "reorder_reg_offset_field.h"

#define LOG_TAG LOG_MODULE_REORDER_DRV

/**********************************************************
function name: reorder_ctrl_config

description:
    set the contrl register of REORDER device.

input:
    dev  : the REORDER device
    ctrl : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int reorder_ctrl_config(reorder_dev_t *dev, reorder_ctrl_cfg_t  *ctrl)
{
	union u_reorder_cfg temp;

	if (NULL == dev || NULL == ctrl) {
		e("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.reorder_en = ctrl->reorder_en;
	temp.bits.descriptor_type = ctrl->descriptor_type;
	temp.bits.total_kpts = 0x7ff;
	CMDLST_SET_REG(dev->base_addr + REORDER_REORDER_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: reorder_block_config

description:
    set BLOCK_CFG register of REORDER device.

input:
    dev  : the REORDER device
    block_cfg : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int reorder_block_config(reorder_dev_t *dev, reorder_block_cfg_t  *block_cfg)
{
	union u_reorder_block_cfg temp;

	if (NULL == dev || NULL == block_cfg) {
		e("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.blk_v_num = block_cfg->blk_v_num;
	temp.bits.blk_h_num = block_cfg->blk_h_num;
	temp.bits.blk_num = block_cfg->blk_num;
	CMDLST_SET_REG(dev->base_addr + REORDER_BLOCK_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: reorder_prefetch_config

description:
    set PREFETCH_CFG register of REORDER device.

input:
    dev  : the REORDER device
    prefetch_cfg : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int reorder_prefetch_config(reorder_dev_t *dev, reorder_prefetch_cfg_t *prefetch_cfg)
{
	union u_reorder_prefetch_cfg temp;

	if (NULL == dev || NULL == prefetch_cfg) {
		e("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.prefetch_enable = prefetch_cfg->prefetch_enable;
	temp.bits.first_32k_page = prefetch_cfg->first_32k_page;
	CMDLST_SET_REG(dev->base_addr + REORDER_PREFETCH_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: reorder_kpt_num_config

description:
    set kpt_num of each block register of REORDER device.

input:
    dev  : the REORDER device
    kptnum_cfg : kpt num

output:
    success : 0
    fail :  other
**********************************************************/
static int reorder_kpt_num_config(reorder_dev_t *dev, reorder_kptnum_cfg_t *kptnum_cfg)
{
	unsigned int i = 0;
	union u_reorder_kpt_number temp;

	if (NULL == dev || NULL == kptnum_cfg) {
		e("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	CMDLST_SET_REG_INCR(dev->base_addr + REORDER_KPT_NUMBER_0_REG, REORDER_KPT_NUM, 0, 0);

	for (i = 0; i < REORDER_KPT_NUM; i++) {
		temp.bits.kpt_num = kptnum_cfg->reorder_kpt_num[i];
		CMDLST_SET_REG_DATA(temp.u32);
	}

	return CPE_FW_OK;
}

/**********************************************************
function name: reorder_do_config

description:
     do_config interface of REORDER device.

input:
    dev    : the REORDER device
    table  : the configure table

output:
    success : 0
    fail : other
**********************************************************/
static int reorder_do_config(reorder_dev_t *dev, cfg_tab_reorder_t *tab_reorder)
{
	if (NULL == dev || NULL == tab_reorder) {
		e("params is NULL!!\n");
		return CPE_FW_ERR;
	}

	if (tab_reorder->reorder_block_cfg.to_use) {
		loge_if_ret(reorder_block_config(dev, &(tab_reorder->reorder_block_cfg)));
		tab_reorder->reorder_block_cfg.to_use = 0;
	}

	if (tab_reorder->reorder_prefetch_cfg.to_use) {
		loge_if_ret(reorder_prefetch_config(dev, &(tab_reorder->reorder_prefetch_cfg)));
		tab_reorder->reorder_prefetch_cfg.to_use = 0;
	}

	if (tab_reorder->reorder_kptnum_cfg.to_use) {
		tab_reorder->reorder_kpt_addr = dev->cmd_buf->data_addr + 4;
		loge_if_ret(reorder_kpt_num_config(dev, &(tab_reorder->reorder_kptnum_cfg)));
		tab_reorder->reorder_kptnum_cfg.to_use = 0;
	}

	if (tab_reorder->reorder_ctrl_cfg.to_use) {
		loge_if_ret(reorder_ctrl_config(dev, &(tab_reorder->reorder_ctrl_cfg)));
		tab_reorder->reorder_ctrl_cfg.to_use = 0;
	}

	return CPE_FW_OK;
}


int reorder_prepare_cmd(reorder_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_reorder_t *table)
{
	dev->cmd_buf = cmd_buf;
	loge_if_ret(reorder_do_config(dev, table));
	return 0;
}

static reorder_ops_t reorder_ops = {
	.prepare_cmd   = reorder_prepare_cmd,
};

reorder_dev_t g_reorder_devs[] = {
	[0] =
	{
		.base_addr = JPG_REORDER_ADDR,
		.ops = &reorder_ops,
	},
};

/********************************** END **********************************/
