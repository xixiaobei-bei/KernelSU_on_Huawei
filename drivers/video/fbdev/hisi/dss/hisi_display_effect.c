/*
 * Copyright (c) 2013-2021, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_display_effect.h"
#include "overlay/hisi_overlay_utils.h"
#include "hisi_dpe_utils.h"
#include "product/hisi_displayengine_utils.h"
#include "product/hisi_displayengine_effect.h"
#include <linux/hisi/hw_cmdline_parse.h>

/* lcd_kit 3.0 */

#include <linux/miscdevice.h>
#include <linux/of_reserved_mem.h>

#include <linux/dma-mapping.h>
#include <linux/memory.h>
#include <uapi/linux/sched/types.h>

#include "dbv_curve_map/dbv_map_lg_ea9151_6_39__cmd_tft_1440_x_3120.h"
#include "dbv_curve_map/dbv_map_lg_nt37280_lion_6_53__cmd_tft_1176_x_2400.h"


#define MMAP_DEVICE_NAME "display_sharemem_map"
#define DTS_COMP_SHAREMEM_NAME "hisilicon,hisisharemem"
#define DEV_NAME_SHARE_MEM "display_share_mem"
#define XCC_COEF_LENGTH 12
#define DEBUG_EFFECT_LOG DPU_FB_ERR
#define round(x, y)  (((x) + (y) - 1) / (y))

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-larger-than="

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

int g_factory_gamma_enable = 0;
struct mutex g_rgbw_lock;
uint8_t* share_mem_virt = NULL;
phys_addr_t share_mem_phy = 0;
const int JDI_TD4336_RT8555_RGBW_ID = 14;
const int SHARP_TD4336_RT8555_RGBW_ID = 15;
const int LG_NT36772A_RT8555_RGBW_ID = 16;
const int g_bl_timeinterval = 16670000;
static struct dss_display_effect_xcc last_xcc_param = {
	0,
	{
		0x0, 0x8000, 0x0, 0x0,
		0x0, 0x0, 0x8000, 0x0,
		0x0, 0x0, 0x0, 0x8000
	}
};
const int blc_xcc_buf_count = 18;
const int display_effect_flag_max = 4;

/*lint -e838 -e778 -e845 -e712 -e527 -e30 -e142 -e715 -e655 -e550 +e559*/
#if defined(CONFIG_EFFECT_HIACE)
void hisi_display_effect_hiace_trigger_wq(struct dpu_fb_data_type *dpufd, bool is_idle_display)
{
	/* Check whether the system enters the idle state. If no, need queue work
	 * if yes, clear the current hiace interrupt and no need queue work
	 */
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is null\n");
	if (!is_idle_display) {
		queue_work(dpufd->hiace_end_wq, &dpufd->hiace_end_work);
	} else {
		/* clear hiace interrupt */
		outp32(dpufd->dss_base + DSS_HI_ACE_OFFSET + HIACE_INT_STAT, 0x1);
	}
}

void hisi_effect_hiace_trigger_wq(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is null\n");
	hisi_display_effect_hiace_trigger_wq(dpufd, dpufd->is_idle_display);

}
#endif

static void hisi_effect_module_support(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dss_effect *effect_ctrl = NULL;

	if (NULL == dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return ;
	}

	pinfo = &(dpufd->panel_info);
	effect_ctrl = &(dpufd->effect_ctl);

	memset(effect_ctrl, 0, sizeof(struct dss_effect));

	effect_ctrl->acm_support = (pinfo->acm_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_ACM) != 0));

	effect_ctrl->ace_support = (pinfo->acm_ce_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_ACE) != 0));

	effect_ctrl->dither_support = (pinfo->dither_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_DITHER) != 0));

	effect_ctrl->lcp_xcc_support = (pinfo->xcc_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_LCP_XCC) != 0));

	effect_ctrl->lcp_gmp_support = (pinfo->gmp_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_LCP_GMP) != 0));

	effect_ctrl->lcp_igm_support = (pinfo->gamma_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_LCP_IGM) != 0));

	effect_ctrl->gamma_support = (pinfo->gamma_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_GAMA) != 0));

	effect_ctrl->hiace_support = (pinfo->hiace_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_HIACE) != 0));

	effect_ctrl->post_hihdr_support = pinfo->post_hihdr_support;

	effect_ctrl->arsr1p_sharp_support = (pinfo->arsr1p_sharpness_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF) != 0));

	effect_ctrl->arsr2p_sharp_support = (pinfo->prefix_sharpness2D_support);

	effect_ctrl->post_xcc_support = (pinfo->post_xcc_support
		&& (HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_XCC) != 0));
	effect_ctrl->dss_ready = true;
}

static int dpufb_effect_module_init_handler(void __user *argp)
{
	int ret;
	struct dpu_fb_data_type *dpufd_primary = NULL;

	dpufd_primary = dpufd_list[PRIMARY_PANEL_IDX];
	if (NULL == dpufd_primary) {
		DPU_FB_ERR("fb0 is not existed, return!\n");
		ret = -ENODEV;
		goto err_out;
	}

	if (argp == NULL) {
		DPU_FB_ERR("argp is null pointer\n");
		return -1;
	}
	ret = copy_to_user(argp, &(dpufd_primary->effect_ctl), sizeof(struct dss_effect));
	DPU_FB_INFO("fb0 effect_ctl:dss_ready=%d, arsr2p=%d, arsr1p=%d, acm=%d,"
		" ace=%d, hiace=%d, hihdr=%d, igm=%d, gmp=%d, xcc=%d, gamma=%d, dither=%d,"
		" post_xcc=%d\n",
		dpufd_primary->effect_ctl.dss_ready,
		dpufd_primary->effect_ctl.arsr2p_sharp_support,
		dpufd_primary->effect_ctl.arsr1p_sharp_support,
		dpufd_primary->effect_ctl.acm_support,
		dpufd_primary->effect_ctl.ace_support,
		dpufd_primary->effect_ctl.hiace_support,
		dpufd_primary->effect_ctl.post_hihdr_support,
		dpufd_primary->effect_ctl.lcp_igm_support,
		dpufd_primary->effect_ctl.lcp_gmp_support,
		dpufd_primary->effect_ctl.lcp_xcc_support,
		dpufd_primary->effect_ctl.gamma_support,
		dpufd_primary->effect_ctl.dither_support,
		dpufd_primary->effect_ctl.post_xcc_support);
	if (ret) {
		DPU_FB_ERR("failed to copy result of ioctl to user space.\n");
		goto err_out;
	}

err_out:
	return ret;
}

static int dpufb_effect_module_deinit_handler(const void __user *argp)
{
	int ret;
	struct dss_effect init_status;

	if (argp == NULL) {
		DPU_FB_ERR("argp is null pointer\n");
		return -1;
	}

	ret = copy_from_user(&init_status, argp, sizeof(struct dss_effect));
	if (ret) {
		DPU_FB_ERR("failed to copy data to kernel space.\n");
		goto err_out;
	}

err_out:
	return ret;
}

static int dpufb_effect_info_get_handler(void __user *argp)
{
	int ret = -EINVAL;
	struct dss_effect_info effect_info;
	struct dpu_fb_data_type *dpufd_primary = NULL;

	if (argp == NULL) {
		DPU_FB_ERR("argp is null pointer\n");
		return -1;
	}

	ret = copy_from_user(&effect_info, argp, sizeof(struct dss_effect_info));
	if (ret) {
		DPU_FB_ERR("failed to copy data from user.\n");
		goto err_out;
	}

	dpufd_primary = dpufd_list[PRIMARY_PANEL_IDX];
	if (dpufd_primary == NULL) {
		DPU_FB_ERR("fb0 is not existed, return!\n");
		ret = -ENODEV;
		goto err_out;
	}

	if (!dpufd_primary->panel_power_on) {
		DPU_FB_ERR("panel is power down, return!\n");
		ret = -EBADRQC;
		goto err_out;
	}

	if (!dpufd_primary->effect_ctl.dss_ready) {
		DPU_FB_ERR("dss is not ready\n");
		ret = -EBADRQC;
		goto err_out;
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_ARSR2P) {
		ret = hisi_effect_arsr2p_info_get(dpufd_primary, effect_info.arsr2p);
		if (ret) {
			DPU_FB_ERR("failed to get arsr2p info\n");
			goto err_out;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_ARSR1P) {
		ret = hisi_effect_arsr1p_info_get(dpufd_primary, effect_info.arsr1p);
		if (ret) {
			DPU_FB_ERR("failed to get arsr1p info\n");
			goto err_out;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_ACM) {
		ret = hisi_effect_acm_info_get(dpufd_primary, &effect_info.acm);
		if (ret) {
			DPU_FB_ERR("failed to get acm info\n");
			goto err_out;
		}
	}

	if (effect_info.modules & (DSS_EFFECT_MODULE_LCP_GMP | DSS_EFFECT_MODULE_LCP_IGM |
		DSS_EFFECT_MODULE_LCP_XCC | DSS_EFFECT_MODULE_POST_XCC)) {
		ret = hisi_effect_lcp_info_get(dpufd_primary, &effect_info.lcp);
		if (ret) {
			DPU_FB_ERR("failed to get lcp info\n");
			goto err_out;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_GAMMA) {
		ret = hisi_effect_gamma_info_get(dpufd_primary, &effect_info.gamma);
		if (ret) {
			DPU_FB_ERR("failed to get gamma info\n");
			goto err_out;
		}
	}

	ret = copy_to_user(argp, &effect_info, sizeof(struct dss_effect_info));
	if (ret) {
		DPU_FB_ERR("failed to copy result of ioctl to user space.\n");
		goto err_out;
	}

err_out:
	return ret;;
}

static int dpufb_effect_info_set_handler(const void __user *argp)
{
	int ret;
	struct dss_effect_info effect_info;
	struct dpu_fb_data_type *dpufd_primary = NULL;

	if (argp == NULL) {
		DPU_FB_ERR("argp is null pointer\n");
		return -1;
	}

	ret = copy_from_user(&effect_info, argp, sizeof(struct dss_effect_info));
	if (ret) {
		DPU_FB_ERR("failed to copy data to kernel space.\n");
		goto err_out;
	}

	if ((effect_info.disp_panel_id >= DISP_PANEL_NUM) ||
		(effect_info.disp_panel_id < 0)) {
		DPU_FB_ERR("disp_panel_id = %d is overflow.\n", effect_info.disp_panel_id);
		goto err_out;
	}
	dpufd_primary = dpufd_list[PRIMARY_PANEL_IDX];
	if (dpufd_primary == NULL) {
		DPU_FB_ERR("dpufd_primary is null or unexpected input fb\n");
		ret = -EBADRQC;
		goto err_out;
	}

	mutex_lock(&dpufd_primary->effect_lock);

	dpufd_primary->effect_info[effect_info.disp_panel_id].modules = effect_info.modules;

	if (effect_info.modules & DSS_EFFECT_MODULE_ARSR2P) {
		ret = hisi_effect_save_arsr2p_info(dpufd_primary, &effect_info);
		if (ret) {
			DPU_FB_ERR("failed to set arsr2p info\n");
			goto err_out_spin;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_ARSR1P) {
		ret = hisi_effect_save_arsr1p_info(dpufd_primary, &effect_info);
		if (ret) {
			DPU_FB_ERR("failed to set arsr1p info\n");
			goto err_out_spin;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_ACM) {
		ret = hisi_effect_save_acm_info(dpufd_primary, &effect_info);
		if (ret) {
			DPU_FB_ERR("failed to set acm info\n");
			goto err_out_spin;
		}
	}

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_V330) && \
	!defined(CONFIG_HISI_FB_V501) && !defined(CONFIG_HISI_FB_970)
	if (effect_info.modules & DSS_EFFECT_MODULE_POST_XCC) {
		ret = hisi_effect_save_post_xcc_info(dpufd_primary, &effect_info);
		if (ret) {
			DPU_FB_ERR("failed to set post_xcc info\n");
			goto err_out_spin;
		}
	}

	if ((effect_info.modules & DSS_EFFECT_MODULE_LCP_GMP) ||
		(effect_info.modules & DSS_EFFECT_MODULE_LCP_IGM) ||
		(effect_info.modules & DSS_EFFECT_MODULE_LCP_XCC) ||
		(effect_info.modules & DSS_EFFECT_MODULE_GAMMA)) {
		ret = hisi_effect_dpp_buf_info_set(dpufd_primary, &effect_info, false);
		if (ret) {
			DPU_FB_ERR("failed to set dpp info\n");
			goto err_out_spin;
		}
	}
#else
	if (effect_info.modules & DSS_EFFECT_MODULE_LCP_GMP) {
		ret = hisi_effect_gmp_info_set(dpufd_primary, &effect_info.lcp);
		if (ret) {
			DPU_FB_ERR("failed to set GMP info\n");
			goto err_out_spin;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_LCP_IGM) {
		ret = hisi_effect_igm_info_set(dpufd_primary, &effect_info.lcp);
		if (ret) {
			DPU_FB_ERR("failed to set IGM info\n");
			goto err_out_spin;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_LCP_XCC) {
		ret = hisi_effect_xcc_info_set(dpufd_primary, &effect_info.lcp);
		if (ret) {
			DPU_FB_ERR("failed to set XCC info\n");
			goto err_out_spin;
		}
	}

	if (effect_info.modules & DSS_EFFECT_MODULE_GAMMA) {
		ret = hisi_effect_gamma_info_set(dpufd_primary, &effect_info.gamma);
		if (ret) {
			DPU_FB_ERR("failed to set gama info\n");
			goto err_out_spin;
		}
	}
#endif
	/* the display effect is not allowed to set reg when the partical update */
	if (dpufd_primary->display_effect_flag < 5)
		dpufd_primary->display_effect_flag = 4;

err_out_spin:
	mutex_unlock(&dpufd_primary->effect_lock);

	DPU_FB_DEBUG("fb%d, modules = 0x%x, -.\n", dpufd_primary->index, effect_info.modules);

err_out:
	return ret;;
}

static int hisi_display_effect_ioctl_handler(struct dpu_fb_data_type *dpufd, unsigned int cmd, void __user *argp)
{
	int ret = -EINVAL;

	if (argp == NULL || dpufd == NULL) {
		DPU_FB_ERR("[effect]NULL pointer of argp or dpufd\n");
		goto err_out;
	}

	DPU_FB_DEBUG("[effect]fb%d, +\n", dpufd->index);

	switch (cmd) {
	case HISIFB_EFFECT_MODULE_INIT:
		ret = dpufb_effect_module_init_handler(argp);
		break;
	case HISIFB_EFFECT_MODULE_DEINIT:
		ret = dpufb_effect_module_deinit_handler(argp);
		break;
	case HISIFB_EFFECT_INFO_GET:
		ret = dpufb_effect_info_get_handler(argp);
		break;
	case HISIFB_EFFECT_INFO_SET:
		ret = dpufb_effect_info_set_handler(argp);
		break;
	default:
		DPU_FB_ERR("[effect]unknown cmd id\n");
		ret = -ENOSYS;
		break;
	};

	DPU_FB_DEBUG("[effect]fb%d, -\n", dpufd->index);

err_out:
	return ret;
}

static int effect_thread_fn(void *data)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;
	DPU_FB_INFO("[effect] thread + \n");

	dpufd = (struct dpu_fb_data_type *)data;
	if (!dpufd) {
		DPU_FB_ERR("dpufd is null pointer\n");
		return -1;
	}

	while(!kthread_should_stop()) {
		ret = wait_event_interruptible(dpufd->effect_wq, dpufd->effect_update); //lint !e578
		if (ret) {
			DPU_FB_ERR("wait_event_interruptible wrong \n");
			continue;
		}
		dpufd->effect_update = false;
	}

	DPU_FB_INFO("[effect] thread - \n");

	return 0;
}

static int hisi_display_effect_thread_init(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->effect_thread)
		return 0;

	struct sched_param param = { .sched_priority = 1};
	dpufd->effect_thread = kthread_create(effect_thread_fn, dpufd, "effect_thread");

	if (IS_ERR(dpufd->effect_thread)) {
		DPU_FB_ERR("Unable to start kernel effect_thread.\n");
		dpufd->effect_thread = NULL;
		return -EINVAL;
	}

	init_waitqueue_head(&dpufd->effect_wq);
	sched_setscheduler_nocheck(dpufd->effect_thread, SCHED_FIFO, &param);
	wake_up_process(dpufd->effect_thread);
	return 0;
}

void hisi_display_effect_init(struct dpu_fb_data_type *dpufd)
{
	if (NULL == dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return ;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	if (PRIMARY_PANEL_IDX == dpufd->index) {
		if (dpufd->panel_info.p3_support)
			hisi_display_effect_thread_init(dpufd);

		dpufd->display_effect_ioctl_handler = NULL;
		memset(dpufd->effect_updated_flag, 0, sizeof(dpufd->effect_updated_flag));
		mutex_init(&dpufd->effect_lock);

		hisi_effect_module_support(dpufd);
	} else if (AUXILIARY_PANEL_IDX == dpufd->index) {
		dpufd->display_effect_ioctl_handler = hisi_display_effect_ioctl_handler;
	} else {
		dpufd->display_effect_ioctl_handler = NULL;
	}

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);
}

void hisi_dss_effect_set_reg(struct dpu_fb_data_type *dpufd)
{
	if (NULL == dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return ;
	}

	if (PRIMARY_PANEL_IDX != dpufd->index) {
		return;
	}

	if (mutex_trylock(&dpufd->effect_lock) != 0) {
		if (dpufd->panel_info.smart_color_mode_support == 0) {
			hisi_effect_acm_set_reg(dpufd);
			hisi_effect_lcp_set_reg(dpufd);
		}
#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_V330) && \
	!defined(CONFIG_HISI_FB_V501) && !defined(CONFIG_HISI_FB_970)
		hisi_effect_post_xcc_set_reg(dpufd);
#endif
		hisi_effect_gamma_set_reg(dpufd);
		mutex_unlock(&dpufd->effect_lock);
	} else {
		DPU_FB_DEBUG("dss effect param is being updated, delay set reg to next frame!\n");
	}
	return;
}

static inline struct dpu_fb_data_type *display_engine_check_input_and_get_dpufd(struct fb_info *info, void __user *argp)
{
	struct dpu_fb_data_type *dpufd = NULL;

	if (NULL == info) {
		DPU_FB_ERR("[effect] info is NULL\n");
		return NULL;
	}

	if (NULL == argp) {
		DPU_FB_ERR("[effect] argp is NULL\n");
		return NULL;
	}

	dpufd = (struct dpu_fb_data_type *)info->par;
	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return NULL;
	}

	return dpufd;
}

static void display_engine_amoled_mod_set(
	struct dpu_fb_data_type *dpufd,
	display_engine_param_t *de_param)
{
	uint32_t dpufd_modules = 0;

	mutex_lock(&dpufd->de_info.param_lock);
	dpufd_modules = dpufd->de_param.modules;
	dpufd->de_param = *de_param;
	dpufd_modules |= (*de_param).modules;
	dpufd->de_param.modules = dpufd_modules;
	mutex_unlock(&dpufd->de_info.param_lock);
}

static int display_engine_copy_map_from_user(
	uint16_t *param_map, uint32_t param_map_size,
	uint16_t *buf_map, uint32_t buf_size)
{
	int ret = 0;
	int num = 0;
	int num_of_elements = buf_size / sizeof(uint16_t);

	if (param_map == NULL || param_map_size != buf_size)
		goto OUT;
	ret = copy_from_user(buf_map, param_map, param_map_size);
	if (ret != 0) {
		DPU_FB_ERR("[effect] copy map fail\n");
		goto OUT;
	}
	for (num = 1; num < num_of_elements; num++) {
		if (buf_map[num] == 0)
			goto OUT;
	}
	return ret;
OUT:
	DPU_FB_INFO("[effect] use default map\n");
	for (num = 0; num < num_of_elements; num++)
		buf_map[num] = num;
	return ret;
}

static int display_engine_amoled_sync_param_set(
	struct dpu_fb_data_type *dpufd,
	display_engine_amoled_param_sync_t *param)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}
	if (dpufd->pdev == NULL) {
		DPU_FB_ERR("[effect] pdev is NULL Pointer\n");
		return -1;
	}
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}
	if (param == NULL) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}
	if (display_engine_copy_map_from_user(param->high_dbv_map, param->map_size,
		dpufd->de_info.dbv_map, sizeof(dpufd->de_info.dbv_map)) < 0)
		ret = -1;
	if (display_engine_copy_map_from_user(param->alpha_map, param->alpha_map_size,
		dpufd->de_info.alpha_map, sizeof(dpufd->de_info.alpha_map)) < 0)
		ret = -1;
	return ret;
}

static int display_engine_ud_fp_backlight_param_set(
	struct dpu_fb_data_type *dpufd,
	display_engine_sync_ud_fingerprint_backlight_t *param)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	if (dpufd->pdev == NULL) {
		DPU_FB_ERR("[effect] pdev is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	if ((int)param->current_level == ((int)param->hbm_level + 1)) {
		dpufd->hbm_need_to_open = true;
		dpufd->ud_fp_hbm_level = (int)param->hbm_level;
		DPU_FB_INFO("dpufd->hbm_need_to_open = true\n");
		return ret;
	}

	if ((param->hbm_level < dpufd->panel_info.bl_min) ||
		(param->hbm_level > dpufd->panel_info.bl_max) ||
		(param->current_level < dpufd->panel_info.bl_min) ||
		(param->current_level > dpufd->panel_info.bl_max)) {
		DPU_FB_ERR("invalid data, hbm_level %d current_level %d\n",
			param->hbm_level, param->current_level);
		return -EINVAL;
	}

	dpufd->ud_fp_scene = (int)param->scene_info;
	dpufd->ud_fp_hbm_level = (int)param->hbm_level;
	dpufd->ud_fp_current_level = (int)param->current_level;

	DPU_FB_INFO("[effect] scene:%d hbm_level:%d current_level:%d\n",
		dpufd->ud_fp_scene, dpufd->ud_fp_hbm_level,
		dpufd->ud_fp_current_level);

	return ret;
}

static int display_engine_blc_param_set(struct dpu_fb_data_type *dpufd, display_engine_blc_param_t *param)
{
	struct dpu_fb_panel_data *pdata = NULL;
	ktime_t current_bl_timestamp = ktime_get();
	int ret = 0;

	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return -EINVAL;
	}

	if (NULL == param) {
		DPU_FB_ERR("[effect] param is NULL\n");
		return -EINVAL;
	}

	dpufd->de_info.blc_enable = (param->enable == 1) ? true : false;
	dpufd->de_info.blc_delta = param->delta;

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (NULL == pdata) {
		DPU_FB_ERR("pdata is NULL");
		return -EINVAL;
	}
	down(&dpufd->blank_sem_effect);

	if (abs((int)(ktime_to_ns(dpufd->backlight.bl_timestamp) - ktime_to_ns(current_bl_timestamp))) > g_bl_timeinterval) {
		DPU_FB_DEBUG("[effect] delta:%d bl:%d enable:%d set\n", dpufd->de_info.blc_delta, dpufd->bl_level, dpufd->de_info.blc_enable);
		if (dpufd->bl_level > 0) {
			down(&dpufd->brightness_esd_sem);
			dpufb_set_backlight(dpufd, dpufd->bl_level, true);
			up(&dpufd->brightness_esd_sem);
		}
	} else {
		DPU_FB_DEBUG("[effect] delta:%d bl:%d enable:%d skip\n", dpufd->de_info.blc_delta, dpufd->bl_level, dpufd->de_info.blc_enable);
		ret = 2; /* This delta has been skipped, and hal may resent the command. */
	}

	up(&dpufd->blank_sem_effect);
	return ret;
}

int display_engine_ddic_color_param_set(struct dpu_fb_data_type *dpufd, display_engine_ddic_color_param_t *param)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	if (dpufd->pdev == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	dpufd->de_info.ddic_color_mode = param->ddic_color_mode;

	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -1;
		goto err_out;
	}

	if (pdata->lcd_ce_mode_store != NULL) {
		char buf[8];
		int count = 0;
		dpufb_activate_vsync(dpufd);
		count = snprintf(buf, sizeof(buf), "%d", param->ddic_color_mode);
		pdata->lcd_ce_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);

	return ret;
}

int display_engine_ddic_cabc_param_set(struct dpu_fb_data_type *dpufd, display_engine_ddic_cabc_param_t *param)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	if (dpufd->pdev == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (NULL == pdata) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	dpufd->de_info.ddic_cabc_mode = param->ddic_cabc_mode;
	down(&dpufd->blank_sem);

	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("fb%d, panel power off!\n", dpufd->index);
		ret = -1;
		goto err_out;
	}

	if (pdata->lcd_cabc_mode_store != NULL) {
		char buf[8];
		int count = 0;
		dpufb_activate_vsync(dpufd);
		count = snprintf(buf, sizeof(buf), "%d", param->ddic_cabc_mode);
		pdata->lcd_cabc_mode_store(dpufd->pdev, buf, count);
		dpufb_deactivate_vsync(dpufd);
	}

err_out:
	up(&dpufd->blank_sem);
	return ret;
}

int display_engine_ddic_rgbw_param_set(struct dpu_fb_data_type *dpufd, display_engine_ddic_rgbw_param_t *param)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (NULL == pdata) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (param == NULL) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	mutex_lock(&g_rgbw_lock);
	dpufd->de_info.ddic_panel_id = param->ddic_panel_id;
	dpufd->de_info.ddic_rgbw_mode = param->ddic_rgbw_mode;
	dpufd->de_info.rgbw_saturation_control = param->rgbw_saturation_control;
	dpufd->de_info.frame_gain_speed = param->frame_gain_speed;
	dpufd->de_info.pixel_gain_speed = param->pixel_gain_speed;
	if((param->ddic_panel_id != LG_NT36772A_RGBW_ID) && (param->ddic_panel_id != LG_NT36772A_RGBW_ID_HMA)
		&& (param->ddic_panel_id != BOE_HX83112E_RGBW_ID_HMA) && (param->ddic_panel_id != JDI_TD4336_RT8555_RGBW_ID)
		&& (param->ddic_panel_id != SHARP_TD4336_RT8555_RGBW_ID) && (param->ddic_panel_id != LG_NT36772A_RT8555_RGBW_ID)) {
		dpufd->de_info.frame_gain_limit = param->frame_gain_limit;
		dpufd->de_info.pwm_duty_gain = param->pwm_duty_gain;
		dpufd->de_info.color_distortion_allowance = param->color_distortion_allowance;
		dpufd->de_info.pixel_gain_limit = param->pixel_gain_limit;
	}
	mutex_unlock(&g_rgbw_lock);

	return ret;
}

int display_engine_amoled_algo_param_set(struct dpu_fb_data_type *dpufd, display_engine_amoled_param_t *param)
{
	int ret = 0;

	if(NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL Pointer\n");
		return -1;
	}
	if(NULL == param) {
		DPU_FB_ERR("[effect] param is NULL Pointer\n");
		return -1;
	}

	down(&dpufd->blank_sem);
	dpufd->de_info.amoled_param.HBMEnable = param->HBMEnable;
	dpufd->de_info.amoled_param.amoled_diming_enable = param->amoled_diming_enable;
	dpufd->de_info.amoled_param.HBM_Threshold_BackLight = param->HBM_Threshold_BackLight;
	dpufd->de_info.amoled_param.HBM_Min_BackLight = param->HBM_Min_BackLight;
	dpufd->de_info.amoled_param.HBM_Max_BackLight = param->HBM_Max_BackLight;
	dpufd->de_info.amoled_param.HBM_MinLum_Regvalue = param->HBM_MinLum_Regvalue;
	dpufd->de_info.amoled_param.HBM_MaxLum_Regvalue = param->HBM_MaxLum_Regvalue;
	dpufd->de_info.amoled_param.Hiac_DBVThres = param->Hiac_DBVThres;
	dpufd->de_info.amoled_param.Hiac_DBV_XCCThres = param->Hiac_DBV_XCCThres;
	dpufd->de_info.amoled_param.Hiac_DBV_XCC_MinThres = param->Hiac_DBV_XCC_MinThres;
	dpufd->de_info.amoled_param.Lowac_DBVThres = param->Lowac_DBVThres;
	dpufd->de_info.amoled_param.Lowac_DBV_XCCThres = param->Lowac_DBV_XCCThres;
	dpufd->de_info.amoled_param.Lowac_DBV_XCC_MinThres = param->Lowac_DBV_XCC_MinThres;
	dpufd->de_info.amoled_param.Lowac_Fixed_DBVThres= param->Lowac_Fixed_DBVThres;
	dpufd->de_info.amoled_param.dc_lowac_dbv_thre = param->dc_lowac_dbv_thre;
	dpufd->de_info.amoled_param.dc_lowac_fixed_dbv_thres = param->dc_lowac_fixed_dbv_thres;
	dpufd->de_info.amoled_param.dc_backlight_delay_us = param->dc_backlight_delay_us;
	dpufd->de_info.amoled_param.dc_lowac_dbv_thres_low =
		param->dc_lowac_dbv_thres_low;
	up(&dpufd->blank_sem);
	DPU_FB_DEBUG("[effect] first screen on ! HBM_Max_BackLight:%d Hiac_DBVThres: %d HBMEnable: %d amoled_diming_enable : %d\n",param->HBM_Max_BackLight,param->Hiac_DBVThres,param->HBMEnable,param->amoled_diming_enable);
	DPU_FB_DEBUG("[effect] first screen on ! Lowac_DBVThres:%d Lowac_DBV_XCCThres: %d Lowac_DBV_XCC_MinThres: %d Lowac_Fixed_DBVThres : %d\n",param->Lowac_DBVThres,param->Lowac_DBV_XCCThres,param->Lowac_DBV_XCC_MinThres,param->Lowac_Fixed_DBVThres);
	DPU_FB_DEBUG("[effect] dc_backlight_delay_us = %d", dpufd->de_info.amoled_param.dc_backlight_delay_us);
	DPU_FB_DEBUG("[effect] dc_lowac_dbv_thres_low = %d",
		dpufd->de_info.amoled_param.dc_lowac_dbv_thres_low);

	return ret;
}

int dpufb_display_engine_blank(int blank_mode, struct fb_info *info)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	if (NULL == info) {
		DPU_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	dpufd = (struct dpu_fb_data_type *)info->par;
	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (NULL == pdata) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (blank_mode == FB_BLANK_UNBLANK) {
			if (pdata->lcd_cabc_mode_store && dpufd->de_info.ddic_cabc_mode) {
				char buf[8];
				int count = 0;
				dpufb_activate_vsync(dpufd);
				count = snprintf(buf, sizeof(buf), "%d", dpufd->de_info.ddic_cabc_mode);
				pdata->lcd_cabc_mode_store(dpufd->pdev, buf, count);
				dpufb_deactivate_vsync(dpufd);
			}
		}
	}

	return 0;
}

void dpufb_dbv_curve_mapped_init(struct dpu_fb_data_type *dpufd)
{
	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}

	if (!dpufd->panel_info.dbv_curve_mapped_support) {
		DPU_FB_INFO("[effect] no need dbv_curve_mapped");
		return;
	}

	if (!dpufd->panel_info.panel_name)
		return;

	if (!strcmp(dpufd->panel_info.panel_name, "LG_EA9151 6.39' CMD TFT 1440 x 3120")) {
		if (!strcmp(dpufd->panel_info.lcd_panel_version, " VER:V4")) {
			dpufd->panel_info.dbv_map_index = 0;
			dpufd->panel_info.is_dbv_need_mapped = 1;
		} else if (!strcmp(dpufd->panel_info.lcd_panel_version, " VER:VN1")) {
			dpufd->panel_info.dbv_map_index = 1;
			dpufd->panel_info.is_dbv_need_mapped = 1;
		} else if (!strcmp(dpufd->panel_info.lcd_panel_version, " VER:VN2")) {
			dpufd->panel_info.dbv_map_index= 2;
			dpufd->panel_info.is_dbv_need_mapped = 1;
		}
		dpufd->panel_info.dbv_map_points_num = DBV_MAP_10BIT;
		dpufd->panel_info.dbv_map_curve_pointer =
			&dbv_curve_nolinear_to_linear_map_laya[0][0];
	}
	if (!strcmp(dpufd->panel_info.panel_name, "LG_NT37280_LION 6.53' CMD TFT 1176 x 2400")) {
		if (!strcmp(dpufd->panel_info.lcd_panel_version, " VER:VN1")) {
			dpufd->panel_info.dbv_map_index = 0;
			dpufd->panel_info.is_dbv_need_mapped = 1;
		} else if (!strcmp(dpufd->panel_info.lcd_panel_version, " VER:VN2")) {
			dpufd->panel_info.dbv_map_index= 1;
			dpufd->panel_info.is_dbv_need_mapped = 1;
		}
		dpufd->panel_info.dbv_map_points_num = DBV_MAP_12BIT;
		dpufd->panel_info.dbv_map_curve_pointer =
			&dbv_curve_nolinear_to_linear_map_lion[0][0];
	}
	DPU_FB_INFO("[effect] dpufd index is %d need_map = %d\n",
		dpufd->panel_info.dbv_map_index,
		dpufd->panel_info.is_dbv_need_mapped);
}

int dpufb_display_engine_init(struct fb_info *info, void __user *argp)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_fb_panel_data *pdata = NULL;
	display_engine_t de;
	int ret = 0;

	if (NULL == info) {
		DPU_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		DPU_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	dpufd = (struct dpu_fb_data_type *)info->par;
	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return -EINVAL;
	}

	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (NULL == pdata) {
		DPU_FB_ERR("[effect] pdata is NULL Pointer\n");
		return -1;
	}

	if (!dpufd->de_info.is_ready) {
		dpufd->de_info.is_ready = true;
	}

	dpufb_dbv_curve_mapped_init(dpufd);

	de.blc_support = 1;
	de.ddic_cabc_support = pdata->lcd_cabc_mode_store == NULL ? 0 : 1;
	de.ddic_rgbw_support = pdata->lcd_rgbw_set_func == NULL ? 0 : 1;
	de.ddic_hbm_support = pdata->lcd_hbm_set_func == NULL ? 0 : 1;
	de.ddic_color_support = pdata->lcd_ce_mode_store == NULL ? 0 : 1;
	de.ddic_color_rectify_support = pdata->lcd_color_param_get_func == NULL ? 0 : 1;

	ret = (int)copy_to_user(argp, &de, sizeof(display_engine_t));
	if (ret) {
		DPU_FB_ERR("[effect] copy_to_user(display_engine_t) failed! ret=%d.\n", ret);
		goto ERR_OUT;
	}

ERR_OUT:
	return ret;
}

int dpufb_display_engine_deinit(struct fb_info *info, void __user *argp)
{
	struct dpu_fb_data_type *dpufd = display_engine_check_input_and_get_dpufd(info, argp);
	if (NULL == dpufd) {
		return -EINVAL;
	}

	if (dpufd->de_info.is_ready) {
		dpufd->de_info.is_ready = false;
	}

	return 0;
}

int dpufb_display_engine_param_get(struct fb_info *info, void __user *argp)
{
	return display_engine_param_get(info, argp);
}

static int dpufb_param_check(struct fb_info *info, void __user *argp)
{
	struct dpu_fb_data_type *dpufd = display_engine_check_input_and_get_dpufd(info, argp);
	if (NULL == dpufd) {
		return -EINVAL;
	}

	if (!dpufd->de_info.is_ready) {
		DPU_FB_ERR("[effect] display engine has not been initialized!\n");
		return -EINVAL;
	}

	return 0;
}

int dpufb_runmode_check(display_engine_param_t* de_param)
{
	if(de_param == NULL){
		DPU_FB_ERR("de_param is null pointer\n");
		return -1;
	}
	if (de_param->modules & DISPLAY_ENGINE_DDIC_RGBW ||
		de_param->modules & DISPLAY_ENGINE_HBM ||
		de_param->modules & DISPLAY_ENGINE_BLC ||
		de_param->modules & DISPLAY_ENGINE_DDIC_LOCAL_HBM) {
		DPU_FB_DEBUG("dpufb_runmode_check ok\n");
		return 0;
	} else if (runmode_is_factory()) {
		return -EINVAL;
	}

	return 0;
}

int dpufb_display_engine_param_set(struct fb_info *info, void __user *argp)
{
	struct dpu_fb_data_type *dpufd = NULL;
	display_engine_param_t de_param;
	int ret = 0;

	if(argp == NULL){
		DPU_FB_ERR("argp is null pointer\n");
		return -EINVAL;
	}
	if (dpufb_param_check(info, argp)) {
		return -EINVAL;
	}

	dpufd = (struct dpu_fb_data_type *)info->par;
	if(dpufd == NULL){
		DPU_FB_ERR("dpufd is null pointer\n");
		return -1;
	}

	ret = (int)copy_from_user(&de_param, argp, sizeof(display_engine_param_t));
	if (ret) {
		DPU_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return ret;
	}

	if (dpufb_runmode_check(&de_param)) {
		return -EINVAL;
	}
	display_engine_amoled_mod_set(dpufd, &de_param);
	if (de_param.modules & DISPLAY_ENGINE_AMOLED_SYNC) {
		ret = display_engine_amoled_sync_param_set(dpufd,
			&de_param.amoled_param_sync);
		if (ret < 0) {
			DPU_FB_ERR("AMOLED_SYNC set fail");
			return ret;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_UD_FINGERPRINT_BACKLIGHT) {
		ret = display_engine_ud_fp_backlight_param_set(dpufd,
                        &de_param.ud_fingerprint_backlight);
		if (ret < 0) {
			DPU_FB_ERR("UD_FP_BACKLIGHT failed, ret=%d\n", ret);
			return ret;
		}
	}

	if (de_param.modules & DISPLAY_ENGINE_MAINTENANCE)
		dpufd->de_info.is_maintaining = de_param.is_maintaining;

	if (dpufd->display_engine_wq != NULL) {
		queue_work(dpufd->display_engine_wq, &dpufd->display_engine_work);
	}

	return ret;
}

void dpufb_display_engine_workqueue_handler(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	display_engine_param_t de_param;
	int ret = 0;

	if (NULL == work) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}

	dpufd = container_of(work, struct dpu_fb_data_type, display_engine_work);
	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return;
	}

	mutex_lock(&dpufd->de_info.param_lock);
	de_param = dpufd->de_param;
	dpufd->de_param.modules = 0;
	mutex_unlock(&dpufd->de_info.param_lock);

	if (de_param.modules & DISPLAY_ENGINE_BLC) {
		ret = display_engine_blc_param_set(dpufd, &de_param.blc);
		if (ret < 0)
			DPU_FB_ERR("[effect] failed to set BLC, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_CABC) {
		ret = display_engine_ddic_cabc_param_set(dpufd, &de_param.ddic_cabc);
		if (ret)
			DPU_FB_ERR("[effect] failed to set DDIC_CABC, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_RGBW) {
		ret = display_engine_ddic_rgbw_param_set(dpufd, &de_param.ddic_rgbw);
		if (ret)
			DPU_FB_ERR("[effect] failed to set DDIC_RGBW, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_COLOR) {
		ret = display_engine_ddic_color_param_set(dpufd, &de_param.ddic_color);
		if (ret)
			DPU_FB_ERR("[effect] failed to set DDIC_COLOR, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_HBM) {
		ret = display_engine_hbm_param_set(dpufd, &de_param.hbm);
		if (ret)
			DPU_FB_INFO("[effect] failed to set HBM, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_AMOLED_ALGO) {
		ret = display_engine_amoled_algo_param_set(dpufd, &de_param.amoled_param);
		if (ret)
			DPU_FB_ERR("[effect] failed to set Amoled, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_FLICKER_DETECTOR) {
		bl_flicker_detector_init(de_param.flicker_detector_config);
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_IRC) {
		ret = display_engine_ddic_irc_set(dpufd, &de_param.ddic_irc);
		if (ret)
			DPU_FB_INFO("[effect] failed to set IRC, ret=%d\n", ret);
	}

	if (de_param.modules & DISPLAY_ENGINE_DDIC_LOCAL_HBM) {
		DPU_FB_INFO("[effect] DISPLAY_ENGINE_DDIC_LOCAL_HBM\n");
		ret = display_engine_local_hbm_mmie_set(dpufd, &de_param.ddic_local_hbm);
		if (ret) {
			DPU_FB_ERR("[effect] failed to set LOCAL_HBM, ret=%d\n", ret);
			return;
		}
	}
}

#ifndef DISPLAY_ENGINE_DEBUG
#define DISPLAY_ENGINE_DEBUG(product_type, msg)
#endif

static int dpufb_display_effect_resolve_bl_buf(const char *buf,
	struct effect_bl_buf *resolved_buf)
{
	int ret_count;
	ret_count = sscanf(buf,
		"%u:%d:%u:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%u:%u:%d",
		&(resolved_buf->blc_enable),
		&(resolved_buf->delta),
		&(resolved_buf->xcc_param.xcc_enable),
		&(resolved_buf->xcc_param.xcc_table[0]),
		&(resolved_buf->xcc_param.xcc_table[1]),
		&(resolved_buf->xcc_param.xcc_table[2]),
		&(resolved_buf->xcc_param.xcc_table[3]),
		&(resolved_buf->xcc_param.xcc_table[4]),
		&(resolved_buf->xcc_param.xcc_table[5]),
		&(resolved_buf->xcc_param.xcc_table[6]),
		&(resolved_buf->xcc_param.xcc_table[7]),
		&(resolved_buf->xcc_param.xcc_table[8]),
		&(resolved_buf->xcc_param.xcc_table[9]),
		&(resolved_buf->xcc_param.xcc_table[10]),
		&(resolved_buf->xcc_param.xcc_table[11]),
		&(resolved_buf->dc_enable),
		&(resolved_buf->dimming_enable),
		&(resolved_buf->panel_id));
	DPU_FB_DEBUG("[effect] ret_count = %d\n", ret_count);
	return ret_count;
}

static void dpufb_display_effect_handle_xcc(struct dpu_fb_data_type *dpufd,
	struct effect_bl_buf *resolved_buf, const bool dc_enable_changed)
{
	resolved_buf->ddic_alpha = ALPHA_DEFAULT;
	if ((display_engine_alpha_get_support() == 1) && resolved_buf->dc_enable)
		dpufb_display_effect_separate_alpha_from_xcc(resolved_buf);

	DPU_FB_DEBUG("resolved_buf->alpha:%d\n", resolved_buf->ddic_alpha);
	if ((memcmp(&last_xcc_param, &(resolved_buf->xcc_param),
		sizeof(last_xcc_param)) == 0) && !dc_enable_changed)
		return;

	DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type,
		"[DE] xcc_info_set_kernel +\n");
	if (dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) {
	} else {
#if !defined(CONFIG_HISI_FB_970)
		if (hisi_effect_xcc_info_set_kernel(dpufd,
			&(resolved_buf->xcc_param)) != 0) {
			dpufd->de_info.amoled_param.dc_brightness_dimming_enable = 0;
			dpufd->de_info.amoled_param.amoled_enable_from_hal = 0;
			DPU_FB_ERR("[effect] hisi_effect_xcc_info_set error\n");
		}
#endif
	}
	DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type,
		"[DE] xcc_info_set_kernel -\n");

	(void)memcpy(&last_xcc_param, &(resolved_buf->xcc_param),
		sizeof(last_xcc_param));
}

static void dpufb_display_effect_handle_dc_sync(
	struct dpu_fb_data_type *dpufd,
	struct effect_bl_buf *resolved_buf, const bool dc_enable_changed)
{
	struct dpu_fb_data_type *dpufd_primary =
		dpufd_list[PRIMARY_PANEL_IDX];
	struct dpu_panel_info *pinfo = NULL;

	if (dpufd_primary == NULL) {
		DPU_FB_ERR("dpufd_primary is NULL pointer, return!\n");
		return;
	}
	pinfo = get_panel_info(dpufd, resolved_buf->panel_id);
	if (pinfo == NULL) {
		DPU_FB_ERR("pinfo is NULL pointer, return!\n");
		return;
	}
	dpufd->de_info.amoled_param.dc_brightness_dimming_enable = resolved_buf->dc_enable;
	dpufd->de_info.amoled_param.amoled_enable_from_hal = resolved_buf->dimming_enable;
	if (dc_enable_changed) {
		dpufd_primary->display_effect_flag = display_effect_flag_max;
		if (dpufd->bl_level > 0 || resolved_buf->dc_enable > 0 ||
			resolved_buf->dimming_enable > 0) {
			pinfo->delta_bl_delayed = resolved_buf->delta;
			pinfo->blc_enable_delayed = resolved_buf->blc_enable;
			pinfo->ddic_alpha_delayed = resolved_buf->ddic_alpha;
		} else {
			dpufd->de_info.amoled_param.
				dc_brightness_dimming_enable_real =
				resolved_buf->dc_enable;
			dpufd->de_info.amoled_param.amoled_diming_enable =
				resolved_buf->dimming_enable;
			dpufd->de_info.blc_enable =
				(bool)resolved_buf->blc_enable;
			dpufd->de_info.blc_delta = resolved_buf->delta;
		}
	} else {
		dpufd->de_info.blc_enable = (bool)resolved_buf->blc_enable;
		dpufd->de_info.blc_delta = resolved_buf->delta;
		dpufd->de_info.ddic_alpha = resolved_buf->ddic_alpha;
	}
}

static ssize_t display_engine_effect_set_delta(struct dpu_fb_data_type *dpufd,
	ssize_t ret, bool dc_enable_changed, bool delta_changed)
{
	static bool need_trigger_next_time = false;

	DPU_FB_DEBUG("force_delta_bl_update_support:%d\n", display_engine_force_delta_bl_update_get_support());
	if ((display_engine_force_delta_bl_update_get_support() == 1) || (abs((int)(ktime_to_ns(dpufd->backlight.bl_timestamp) -
		ktime_to_ns(ktime_get()))) > g_bl_timeinterval)) {
		DPU_FB_DEBUG("[effect] delta:%d bl:%d enable:%d, return %d\n",
			dpufd->de_info.blc_delta, dpufd->bl_level,
			dpufd->de_info.blc_enable, (uint32_t)ret);
		if (dpufd->bl_level > 0 && !dc_enable_changed &&
			(delta_changed || need_trigger_next_time)) {
			need_trigger_next_time = false;
			DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type,
				"[DE] brightness_sem +\n");
			down(&dpufd->brightness_esd_sem);
			DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type
				, "[DE] brightness_sem -\n");
			dpufb_set_backlight(dpufd, dpufd->bl_level, true);
			DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type,
				"[DE] set_backlight -\n");
			up(&dpufd->brightness_esd_sem);
		}
	} else {
		DPU_FB_DEBUG("[effect] delta:%d bl:%d skip,return 0\n",
			dpufd->de_info.blc_delta, dpufd->bl_level);
		need_trigger_next_time = true;
		ret = 0;
		/* This delta has been skipped, and hal may resent the command. */
	}
	return ret;
}

static ssize_t display_engine_effect_handle_delta(
	struct dpu_fb_data_type *dpufd, ssize_t ret_count,
	struct effect_bl_buf *resolved_buf, ssize_t ret)
{
	bool delta_changed = dpufd->de_info.blc_delta != resolved_buf->delta;
	bool dc_enable_changed = false;
	if (ret_count == blc_xcc_buf_count) {
		dc_enable_changed = (resolved_buf->dc_enable !=
			dpufd->de_info.amoled_param.
			dc_brightness_dimming_enable_real) ||
			(resolved_buf->dimming_enable !=
			dpufd->de_info.amoled_param.
			amoled_diming_enable);
		dpufb_display_effect_handle_xcc(dpufd,
			resolved_buf, dc_enable_changed);
		dpufb_display_effect_handle_dc_sync(dpufd,
			resolved_buf, dc_enable_changed);
	} else {
		dpufd->de_info.blc_enable = (bool)resolved_buf->blc_enable;
		dpufd->de_info.blc_delta = resolved_buf->delta;
		dpufd->de_info.amoled_param.dc_brightness_dimming_enable = 0;
	}
	DPU_FB_DEBUG("[effect] blc_enable = %d, blc_delta = %d",
		dpufd->de_info.blc_enable, dpufd->de_info.blc_delta);
	return display_engine_effect_set_delta(dpufd, ret, dc_enable_changed,
		delta_changed);
}

ssize_t dpufb_display_effect_bl_ctrl_store(struct fb_info *info,
	const char *buf, size_t count)
{
	struct dpu_fb_data_type *dpufd = NULL;
	ssize_t ret = (ssize_t)count;
	uint32_t len = 0;
	ssize_t ret_count = 0;
	struct effect_bl_buf resolved_buf = { 0 };

	if (NULL == info) {
		DPU_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	dpufd = (struct dpu_fb_data_type *)info->par;
	if (NULL == dpufd) {
		DPU_FB_ERR("[effect] dpufd is NULL\n");
		return -1;
	}

	if (NULL == buf) {
		DPU_FB_ERR("[effect] buf is NULL\n");
		return -1;
	}

	len = (uint32_t)strlen(buf);
	if (len < 3) {
		DPU_FB_ERR("[effect] invalid input buf, length=%d\n", len);
		return -1;
	}
	if (buf[1] != ':') {
		DPU_FB_ERR("[effect] invalid input buf:%s\n", buf);
		return -1;
	}

	DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type,
		"[DE] blank_sem_effect +\n");
	down(&dpufd->blank_sem_effect);
	DISPLAY_ENGINE_DEBUG(dpufd->panel_info.product_type,
		"[DE] blank_sem_effect -\n");

	ret_count = dpufb_display_effect_resolve_bl_buf(buf, &resolved_buf);
	DPU_FB_DEBUG("[effect]buf is %s, ret_count is %d\n",
		buf, (int)ret_count);
	ret = display_engine_effect_handle_delta(dpufd, ret_count,
		&resolved_buf, ret);
	up(&dpufd->blank_sem_effect);
	return ret;
}

uint32_t get_fixed_point_offset(uint32_t half_block_size)
{
	uint32_t num = 2;
	uint32_t len = 2;

	while (len < half_block_size) {
		num++;
		len <<= 1;
	}
	return num;
}


static int share_mmap_map(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long start = 0;
	unsigned long size = 0;

	if (vma == NULL) {
		DPU_FB_ERR("[shmmap] vma is null!\n");
		return -1;
	}

	if (share_mem_virt == NULL || share_mem_phy == 0) {
		DPU_FB_ERR("[shmmap] share memory is not alloced!\n");
		return 0;
	}

	start = (unsigned long)vma->vm_start;
	size = (unsigned long)(vma->vm_end - vma->vm_start);
	if (size > SHARE_MEMORY_SIZE) {
		return -1;
	}
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	DPU_FB_INFO("[shmmap] my_map size = 0x%lx\n", size);
	if(remap_pfn_range(vma, start, __phys_to_pfn(share_mem_phy), size, vma->vm_page_prot)) {
		DPU_FB_ERR("[shmmap] remap_pfn_range error!\n");
		return -1;
	}
	return 0;
}

static struct file_operations mmap_dev_fops = {
	.owner = THIS_MODULE,
	.mmap = share_mmap_map,
};

static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MMAP_DEVICE_NAME,
	.fops = &mmap_dev_fops,
};

static int __init mmap_dev_init(void)
{
	int ret = 0;

	DPU_FB_INFO("[shmmap] dev_init \n");
	ret = misc_register(&misc);
	if (ret) {
		DPU_FB_ERR("[shmmap] misc_register ret = %d \n", ret);
		return ret;
	}

	return ret;
}

static int hisi_share_mem_probe(struct platform_device *pdev)
{
	DPU_FB_DEBUG("[shmmap] hisi_share_mem_probe \n");

	if (NULL == pdev) {
		DPU_FB_ERR("[shmmap] pdev is NULL");
		return -EINVAL;
	}

	share_mem_virt = (void *)dma_alloc_coherent(&pdev->dev, SHARE_MEMORY_SIZE, &share_mem_phy, GFP_KERNEL);
	if (share_mem_virt == NULL || share_mem_phy == 0) {
		DPU_FB_ERR("[shmmap] dma_alloc_coherent error! ");
		return -EINVAL;
	}

	mmap_dev_init();

	return 0;
}

static int hisi_share_mem_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id hisi_share_match_table[] = {
	{
		.compatible = DTS_COMP_SHAREMEM_NAME,
		.data = NULL,
	},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_share_match_table);

static struct platform_driver this_driver = {
	.probe = hisi_share_mem_probe,
	.remove = hisi_share_mem_remove,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_SHARE_MEM,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_share_match_table),
	},
};

static int __init hisi_share_mem_init(void)
{
	int ret = 0;
	DPU_FB_INFO("[shmmap] hisi_share_mem_init\n");
	ret = platform_driver_register(&this_driver);
	if (ret) {
		DPU_FB_ERR("[shmmap] platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(hisi_share_mem_init);

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
/*lint +e838 +e778 +e845 +e712 +e527 +e30 +e142 +e715 +e655 +e550 +e559*/
