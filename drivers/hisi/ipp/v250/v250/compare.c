
#include <linux/printk.h>
#include <linux/string.h>
#include "ipp.h"
#include "compare_drv.h"
#include "compare_drv_priv.h"
#include "compare_reg_offset.h"
#include "compare_reg_offset_field.h"

#define LOG_TAG LOG_MODULE_COMPARE_DRV


/**********************************************************
function name: compare_ctrl_config

description:
    set the contrl register of COMPARE device.

input:
    dev  : the COMPARE device
    ctrl : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int compare_ctrl_config(compare_dev_t *dev, compare_ctrl_cfg_t  *ctrl)
{
	union u_compare_cfg temp;

	if (NULL == dev || NULL == ctrl) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.compare_en = ctrl->compare_en;
	temp.bits.descriptor_type = ctrl->descriptor_type;
	CMDLST_SET_REG(dev->base_addr + COMPARE_COMPARE_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: compare_block_config

description:
    set BLOCK_CFG register of COMPARE device.

input:
    dev  : the COMPARE device
    block_cfg : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int compare_block_config(compare_dev_t *dev, compare_block_cfg_t  *block_cfg)
{
	union u_compare_block_cfg temp;

	if (NULL == dev || NULL == block_cfg) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.blk_v_num = block_cfg->blk_v_num;
	temp.bits.blk_h_num = block_cfg->blk_h_num;
	temp.bits.blk_num = block_cfg->blk_num;
	CMDLST_SET_REG(dev->base_addr + COMPARE_BLOCK_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: compare_search_config

description:
    set search register of COMPARE device.

input:
    dev  : the COMPARE device
    search_cfg : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int compare_search_config(compare_dev_t *dev, compare_search_cfg_t *search_cfg)
{
	union u_compare_search_cfg temp;

	if (NULL == dev || NULL == search_cfg) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.v_radius = search_cfg->v_radius;
	temp.bits.h_radius = search_cfg->h_radius;
	temp.bits.dis_ratio = search_cfg->dis_ratio;
	temp.bits.dis_threshold = search_cfg->dis_threshold;
	CMDLST_SET_REG(dev->base_addr + COMPARE_SEARCH_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: compare_stat_config

description:
    set stat register of COMPARE device.

input:
    dev  : the COMPARE device
    stat_cfg : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int compare_stat_config(compare_dev_t *dev, compare_stat_cfg_t *stat_cfg)
{
	union u_compare_stat_cfg temp;

	if (NULL == dev || NULL == stat_cfg) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.stat_en = stat_cfg->stat_en;
	temp.bits.max3_ratio = stat_cfg->max3_ratio;
	temp.bits.bin_factor = stat_cfg->bin_factor;
	temp.bits.bin_num = stat_cfg->bin_num;
	CMDLST_SET_REG(dev->base_addr + COMPARE_STAT_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: compare_prefetch_config

description:
    set PREFETCH_CFG register of COMPARE device.

input:
    dev  : the COMPARE device
    prefetch_cfg : the value to be set

output:
    success : 0
    fail :  other
**********************************************************/
static int compare_prefetch_config(compare_dev_t *dev, compare_prefetch_cfg_t *prefetch_cfg)
{
	union u_compare_prefetch_cfg temp;

	if (NULL == dev || NULL == prefetch_cfg) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	temp.u32 = 0;
	temp.bits.prefetch_enable = prefetch_cfg->prefetch_enable;
	temp.bits.first_32k_page = prefetch_cfg->first_32k_page;
	CMDLST_SET_REG(dev->base_addr + COMPARE_PREFETCH_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

/**********************************************************
function name: compare_kpt_num_config

description:
    set kpt_num of each block register of COMPARE device.

input:
    dev  : the COMPARE device
    kptnum_cfg : kpt num

output:
    success : 0
    fail :  other
**********************************************************/
static int compare_kpt_num_config(compare_dev_t *dev, compare_kptnum_cfg_t *kptnum_cfg)
{
	unsigned int i = 0;
	union u_compare_ref_kpt_number temp_ref;
	union u_compare_cur_kpt_number temp_cur;

	if (NULL == dev || NULL == kptnum_cfg) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	temp_ref.u32 = 0;
	temp_cur.u32 = 0;

	for (i = 0; i < COMPARE_KPT_NUM; i++) {
		temp_ref.bits.ref_kpt_num = kptnum_cfg->compare_ref_kpt_num[i];
		CMDLST_SET_REG(dev->base_addr + COMPARE_REF_KPT_NUMBER_0_REG + 4 * i, temp_ref.u32);
	}

	for (i = 0; i < COMPARE_KPT_NUM; i++) {
		temp_cur.bits.cur_kpt_num = kptnum_cfg->compare_cur_kpt_num[i];
		CMDLST_SET_REG(dev->base_addr + COMPARE_CUR_KPT_NUMBER_0_REG + 4 * i, temp_cur.u32);
	}

	return CPE_FW_OK;
}

/**********************************************************
function name: compare_do_config

description:
     do_config interface of COMPARE device.

input:
    dev    : the COMPARE device
    table  : the configure table

output:
    success : 0
    fail : other
**********************************************************/
static int compare_do_config(compare_dev_t *dev, cfg_tab_compare_t *tab_compare)
{
	if (NULL == dev || NULL == tab_compare) {
		e("params is NULL!!");
		return CPE_FW_ERR;
	}

	if (tab_compare->compare_block_cfg.to_use)
		loge_if_ret(compare_block_config(dev, &(tab_compare->compare_block_cfg)));

	if (tab_compare->compare_search_cfg.to_use)
		loge_if_ret(compare_search_config(dev, &(tab_compare->compare_search_cfg)));

	if (tab_compare->compare_stat_cfg.to_use)
		loge_if_ret(compare_stat_config(dev, &(tab_compare->compare_stat_cfg)));

	if (tab_compare->compare_prefetch_cfg.to_use)
		loge_if_ret(compare_prefetch_config(dev, &(tab_compare->compare_prefetch_cfg)));

	if (tab_compare->compare_kptnum_cfg.to_use)
		loge_if_ret(compare_kpt_num_config(dev, &(tab_compare->compare_kptnum_cfg)));

	if (tab_compare->compare_ctrl_cfg.to_use)
		loge_if_ret(compare_ctrl_config(dev, &(tab_compare->compare_ctrl_cfg)));

	return CPE_FW_OK;
}


int compare_prepare_cmd(compare_dev_t *dev, cmd_buf_t *cmd_buf, cfg_tab_compare_t *table)
{
	dev->cmd_buf = cmd_buf;
	loge_if_ret(compare_do_config(dev, table));
	return 0;
}

static compare_ops_t compare_ops = {
	.prepare_cmd   = compare_prepare_cmd,
};

compare_dev_t g_compare_devs[] = {
	[0] =
	{
		.base_addr = JPG_COMPARE_ADDR,
		.ops = &compare_ops,
	},
};

/********************************** END **********************************/



