
#include <linux/printk.h>
#include "ipp.h"
#include "vbk_drv.h"
#include "vbk_drv_priv.h"
#include "vbk_reg_offset.h"
#include "vbk_reg_offset_field.h"

#define LOG_TAG LOG_MODULE_VBK_DRV

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static int vbk_basic_config(vbk_dev_t *dev, vbk_ctrl_cfg_t *vbk_basic_cfg)
{
	if ((NULL == dev) || (NULL == vbk_basic_cfg))
		return CPE_FW_ERR;

	union u_vbk_cfg temp_vbk_cfg;
	temp_vbk_cfg.u32 = 0;
	temp_vbk_cfg.bits.op_mode = vbk_basic_cfg->op_mode;
	temp_vbk_cfg.bits.y_ds16_gauss_en = vbk_basic_cfg->y_ds16_gauss_en;
	temp_vbk_cfg.bits.uv_ds16_gauss_en = vbk_basic_cfg->uv_ds16_gauss_en;
	temp_vbk_cfg.bits.sigma_gauss_en = vbk_basic_cfg->sigma_gauss_en;
	temp_vbk_cfg.bits.sigma2alpha_en = vbk_basic_cfg->sigma2alpha_en;
	temp_vbk_cfg.bits.in_width_ds4 = vbk_basic_cfg->in_width_ds4;
	temp_vbk_cfg.bits.in_height_ds4 = vbk_basic_cfg->in_height_ds4;
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_CFG_REG, temp_vbk_cfg.u32);
	return CPE_FW_OK;
}

static int vbk_y_gauss_config(vbk_dev_t *dev, vbk_yds16_sigma_gauss_coeff_t *gauss_coeff)
{
	if ((NULL == dev) || (NULL == gauss_coeff))
		return CPE_FW_ERR;

	union u_vbk_y_ds16_gauss_coeff0_cfg temp_gauss_coeff0;
	union u_vbk_y_ds16_gauss_coeff1_cfg temp_gauss_coeff1;
	temp_gauss_coeff0.u32 = 0;
	temp_gauss_coeff1.u32 = 0;
	temp_gauss_coeff0.bits.y_ds16_g00 = gauss_coeff->g00;
	temp_gauss_coeff0.bits.y_ds16_g01 = gauss_coeff->g01;
	temp_gauss_coeff0.bits.y_ds16_g02 = gauss_coeff->g02;
	temp_gauss_coeff0.bits.y_ds16_g03 = gauss_coeff->g03;
	temp_gauss_coeff1.bits.y_ds16_g04 = gauss_coeff->g04;
	temp_gauss_coeff1.bits.y_ds16_gauss_inv = gauss_coeff->inv;
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_Y_DS16_GAUSS_COEFF0_CFG_REG, temp_gauss_coeff0.u32);
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_Y_DS16_GAUSS_COEFF1_CFG_REG, temp_gauss_coeff1.u32);
	return CPE_FW_OK;
}

static int vbk_sigma_gauss_config(vbk_dev_t *dev, vbk_yds16_sigma_gauss_coeff_t *gauss_coeff)
{
	if ((NULL == dev) || (NULL == gauss_coeff))
		return CPE_FW_ERR;

	union u_vbk_sigma_gauss_coeff0_cfg temp_gauss_coeff0;
	union u_vbk_sigma_gauss_coeff1_cfg temp_gauss_coeff1;
	temp_gauss_coeff0.u32 = 0;
	temp_gauss_coeff1.u32 = 0;
	temp_gauss_coeff0.bits.sigma_g00 = gauss_coeff->g00;
	temp_gauss_coeff0.bits.sigma_g01 = gauss_coeff->g01;
	temp_gauss_coeff0.bits.sigma_g02 = gauss_coeff->g02;
	temp_gauss_coeff0.bits.sigma_g03 = gauss_coeff->g03;
	temp_gauss_coeff1.bits.sigma_g04 = gauss_coeff->g04;
	temp_gauss_coeff1.bits.sigma_gauss_inv = gauss_coeff->inv;
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_SIGMA_GAUSS_COEFF0_CFG_REG, temp_gauss_coeff0.u32);
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_SIGMA_GAUSS_COEFF1_CFG_REG, temp_gauss_coeff1.u32);
	return CPE_FW_OK;
}

static int vbk_uv_gauss_config(vbk_dev_t *dev, vbk_uvds16_gauss_coeff_t *gauss_coeff)
{
	if ((NULL == dev) || (NULL == gauss_coeff))
		return CPE_FW_ERR;

	union u_vbk_uv_ds16_gauss_coeff0_cfg temp_gauss_coeff0;
	union u_vbk_uv_ds16_gauss_coeff1_cfg temp_gauss_coeff1;
	temp_gauss_coeff0.u32 = 0;
	temp_gauss_coeff1.u32 = 0;
	temp_gauss_coeff0.bits.uv_ds16_g00 = gauss_coeff->g00;
	temp_gauss_coeff0.bits.uv_ds16_g01 = gauss_coeff->g01;
	temp_gauss_coeff0.bits.uv_ds16_g02 = gauss_coeff->g02;
	temp_gauss_coeff1.bits.uv_ds16_gauss_inv = gauss_coeff->inv;
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_UV_DS16_GAUSS_COEFF1_CFG_REG, temp_gauss_coeff1.u32);
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_UV_DS16_GAUSS_COEFF0_CFG_REG, temp_gauss_coeff0.u32);
	return CPE_FW_OK;
}

static int vbk_addnoise_config(vbk_dev_t *dev, vbk_addnoise_t *addnoise)
{
	if ((NULL == dev) || (NULL == addnoise))
		return CPE_FW_ERR;

	union u_vbk_addnoise_cfg temp;
	temp.u32 = 0;
	temp.bits.addnoise_th = addnoise->addnoise_th;
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_ADDNOISE_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

static int vbk_sigma2alpha_config(vbk_dev_t *dev, vbk_sigma2alpha_t *sigma2alpha)
{
	if ((NULL == dev) || (NULL == sigma2alpha))
		return CPE_FW_ERR;

	union u_vbk_sigma2alpha_lut temp;
	temp.u32 = 0;
	unsigned int i = 0;

	for (i = 0; i < SIGMA2ALPHA_NUM; i++) {
		temp.bits.sigma2alpha = sigma2alpha->sigma2alpha[i];
		CMDLST_SET_REG(dev->base_addr + VBK_VBK_SIGMA2ALPHA_LUT_0_REG + 4 * i, temp.u32);
	}

	return CPE_FW_OK;
}

static int vbk_foremask_config(vbk_dev_t *dev, vbk_foremask_cfg_t *foremask_cfg)
{
	if ((NULL == dev) || (NULL == foremask_cfg))
		return CPE_FW_ERR;

	union u_vbk_foremask_cfg temp;
	temp.u32 = 0;
	temp.bits.foremask_coeff = foremask_cfg->foremask_coeff;
	temp.bits.foremask_th = foremask_cfg->foremask_th;
	temp.bits.foremask_weighted_filter_en = foremask_cfg->foremask_weighted_filter_en;
	temp.bits.foremask_dilation_radius = foremask_cfg->foremask_dilation_radius;
	CMDLST_SET_REG(dev->base_addr + VBK_VBK_FOREMASK_CFG_REG, temp.u32);
	return CPE_FW_OK;
}

static int vbk_do_config(vbk_dev_t *dev, vbk_config_table_t *cfg_tab)
{
	if ((NULL == dev) || (NULL == cfg_tab))
		return CPE_FW_ERR;

	if (1 == cfg_tab->to_use) {
		cfg_tab->to_use = 0;

		if (1 == cfg_tab->vbk_ctrl.to_use) {
			cfg_tab->vbk_ctrl.to_use = 0;
			vbk_basic_config(dev, &cfg_tab->vbk_ctrl);
		}

		if (1 == cfg_tab->yds16_gauss_coeff.to_use) {
			cfg_tab->yds16_gauss_coeff.to_use = 0;
			vbk_y_gauss_config(dev, &cfg_tab->yds16_gauss_coeff);
		}

		if (1 == cfg_tab->uvds16_gauss_coeff.to_use) {
			cfg_tab->uvds16_gauss_coeff.to_use = 0;
			vbk_uv_gauss_config(dev, &cfg_tab->uvds16_gauss_coeff);
		}

		if (1 == cfg_tab->sigma_gauss_coeff.to_use) {
			cfg_tab->sigma_gauss_coeff.to_use = 0;
			vbk_sigma_gauss_config(dev, &cfg_tab->sigma_gauss_coeff);
		}

		if (1 == cfg_tab->sigma2alpha.to_use) {
			cfg_tab->sigma2alpha.to_use = 0;
			vbk_sigma2alpha_config(dev, &cfg_tab->sigma2alpha);
		}

		if (1 == cfg_tab->addnoise.to_use) {
			cfg_tab->addnoise.to_use = 0;
			vbk_addnoise_config(dev, &cfg_tab->addnoise);
		}

		if (1 == cfg_tab->foremask_cfg.to_use) {
			cfg_tab->foremask_cfg.to_use = 0;
			vbk_foremask_config(dev, &cfg_tab->foremask_cfg);
		}
	}

	return CPE_FW_OK;
}

int vbk_prepare_cmd(vbk_dev_t *dev, cmd_buf_t *cmd_buf, vbk_config_table_t *table)
{
	dev->cmd_buf = cmd_buf;
	loge_if_ret(vbk_do_config(dev, table));
	return 0;
}

static vbk_ops_t vbk_ops = {
	.prepare_cmd   = vbk_prepare_cmd,
};

vbk_dev_t g_vbk_devs[] = {
	[0] =
	{
		.base_addr = JPG_VBK_ADDR,
		.ops = &vbk_ops,
	},
};

#pragma GCC diagnostic pop

/********************************** END **********************************/
