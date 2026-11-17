 /* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License version 2 and
  * only version 2 as published by the Free Software Foundation.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  */

#include "hisi_hiace_single_mode.h"
#include <linux/fb.h>

/* Definiton */
#define HIACE_LHIST_BLOCK_ERROR 95
#define HIACE_LHIST_BLOCK_WARNING 98
#define HIACE_LHIST_BLOCK_RATIO 100

struct position_block {
	uint32_t center;
	uint32_t side;
	uint32_t vertex;
	uint32_t error;
	uint32_t warning;
};

/* Declaration */
static struct hiace_single_mode_info g_single_mode_info;
static bool g_single_mode_init;

/* Internal API */
static bool hiace_lhist_block_vertex_block(uint32_t ptr)
{
	if (ptr == 0)  /* left-top */
		return true;
	if (ptr == (XBLOCKNUM - 1))  /* right-top */
		return true;
	if (ptr == (XBLOCKNUM * (YBLOCKNUM - 1)))  /* left-bottom */
		return true;
	if (ptr == (XBLOCKNUM * YBLOCKNUM - 1))  /* right-bottom */
		return true;

	return false;
}

static bool hiace_lhist_block_side_block(uint32_t ptr)
{
	if ((ptr % XBLOCKNUM) == 0)  /* left */
		return true;
	if (ptr < XBLOCKNUM)  /* top */
		return true;
	if ((ptr % XBLOCKNUM) == (XBLOCKNUM - 1))  /* right */
		return true;
	if (ptr >= (XBLOCKNUM * (YBLOCKNUM - 1)))  /* bottom */
		return true;

	return false;
}

static int hiace_get_lhist_band(const char __iomem *hiace_base)
{
	uint32_t lhist_en;
	int lhist_quant;
	int lhist_band = 16;  /* default value */

	lhist_en = inp32(hiace_base + DPE_LHIST_EN);

	lhist_quant = (lhist_en >> 10) & 0x1;

	if (lhist_quant == 0)
		lhist_band = 16;
	else
		lhist_band = 8;

	return lhist_band;
}


static void hiace_single_mode_calculate_block_pixels(struct dpu_fb_data_type *dpufd,
	uint32_t *center_block, uint32_t *side_block, uint32_t *vertex_block)
{
	char __iomem *hiace_base = NULL;
	struct dpu_panel_info *pinfo = NULL;
	uint32_t reg;
	uint32_t width;
	uint32_t height;
	uint32_t lshift;
	uint32_t lhist_block_pixel_num;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;
	pinfo = &(dpufd->panel_info);

	/* calculate block pixels according to local hist algo */
	reg = inp32(hiace_base + DPE_HALF_BLOCK_INFO);
	width = (reg & 0x1ff) * 2;
	height = ((reg >> 16) & 0x1ff) * 2;

	reg = inp32(hiace_base + DPE_LHIST_SFT);
	lshift = 1 << (reg & 0x7);

	lhist_block_pixel_num = width * height / lshift;
	if (lhist_block_pixel_num == 0) {
		DPU_FB_ERR("error param: width=%u, height=%u, lshift=%u!\n", width, height, lshift);
		lhist_block_pixel_num = pinfo->xres * pinfo->yres / (YBLOCKNUM * XBLOCKNUM) / lshift;
	}

	/* calculater different type block typical number according to algorithem:
	 * center: 4 times of one block pixels;
	 * side: 3 times of one block pixels;
	 * vertex: 9/4 times of one block pixels
	 */
	*center_block = lhist_block_pixel_num * 4;
	*side_block = lhist_block_pixel_num * 3;
	*vertex_block = lhist_block_pixel_num * 9 / 4;
}

static bool hiace_single_mode_verify_hist_block(struct dpu_fb_data_type *dpufd)
{
	bool ret = true;
	char __iomem *hiace_base = NULL;
	struct position_block block = {0};
	uint32_t i;
	uint32_t j;
	uint32_t *local_hist_ptr = NULL;
	uint32_t start;
	uint32_t end;
	uint32_t lhist_band;
	uint32_t cur_block_pixel_num;

	start = g_single_mode_info.hist_block_v_ptr * XBLOCKNUM + g_single_mode_info.hist_block_h_ptr;
	end = start + g_single_mode_info.block_once_num;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	local_hist_ptr = &g_single_mode_info.hist[HIACE_GHIST_RANK + HIACE_GHIST_RANK];
	lhist_band = hiace_get_lhist_band(hiace_base);

	hiace_single_mode_calculate_block_pixels(dpufd, &block.center, &block.side, &block.vertex);

	for (i = start; i < end; i++) {
		cur_block_pixel_num = 0;

		for (j = 0; j < lhist_band; j++)
			cur_block_pixel_num += local_hist_ptr[i * lhist_band + j];

		if (hiace_lhist_block_vertex_block(i)) {
			block.error = block.vertex * HIACE_LHIST_BLOCK_ERROR / HIACE_LHIST_BLOCK_RATIO;
			block.warning = block.vertex * HIACE_LHIST_BLOCK_WARNING / HIACE_LHIST_BLOCK_RATIO;
		} else if (hiace_lhist_block_side_block(i)) {
			block.error = block.side * HIACE_LHIST_BLOCK_ERROR / HIACE_LHIST_BLOCK_RATIO;
			block.warning = block.side * HIACE_LHIST_BLOCK_WARNING / HIACE_LHIST_BLOCK_RATIO;
		} else {
			block.error = block.center * HIACE_LHIST_BLOCK_ERROR / HIACE_LHIST_BLOCK_RATIO;
			block.warning = block.center * HIACE_LHIST_BLOCK_WARNING / HIACE_LHIST_BLOCK_RATIO;
		}

		if (cur_block_pixel_num < block.error) {
			DPU_FB_WARNING("lhist block:%d is wrong num = %d!\n", i, cur_block_pixel_num);
			ret = false;
		} else if (cur_block_pixel_num < block.warning) {
			DPU_FB_WARNING("lhist block:%d is not good num = %d!\n", i, cur_block_pixel_num);
		}
	}

	return ret;
}

static void hiace_single_mode_state_init(void)
{
	if (g_single_mode_info.ioctl_info.info_type & EN_HIACE_INFO_TYPE_GLOBAL_HIST) {
		g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_WORKING_GLOBAL_HIST;
	} else if (g_single_mode_info.ioctl_info.info_type & EN_HIACE_INFO_TYPE_LOCAL_HIST) {
		g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_WORKING_LOCAL_HIST;
	} else if (g_single_mode_info.ioctl_info.info_type & EN_HIACE_INFO_TYPE_FNA) {
		if (g_single_mode_info.ioctl_info.isr_handle)
			g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_WORKING_ISR_FNA;
		else
			g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_WORKING_FNA;
	}
}

static void hiace_single_mode_reset_read_point(char __iomem *hiace_base)
{
	/* reset hist read poit */
	g_single_mode_info.hist_block_h_ptr = 0;
	g_single_mode_info.hist_block_v_ptr = 0;
	set_reg(hiace_base + DPE_LHIST_EN, 0, 6, 4);

	/* reset fna read point */
	set_reg(hiace_base + DPE_FNA_ADDR, 0, 6, 4);
}

static void hiace_single_mode_init(struct dpu_fb_data_type *dpufd)
{
	char __iomem *hiace_base = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;
	ce_ctrl = &(dpufd->ce_ctrl);

	/* initial state */
	dpufd->hiace_info[dpufd->panel_info.disp_panel_id].hiace_enable = true;
	ce_ctrl->ctrl_ce_mode = CE_MODE_SINGLE;
	hiace_single_mode_state_init();

	hiace_single_mode_reset_read_point(hiace_base);

	/* only enable statistic read module is enough */
	if (g_single_mode_info.ioctl_info.info_type & EN_HIACE_INFO_TYPE_LOCAL_HIST) {
		set_reg(hiace_base + DPE_UPDATE_LOCAL, 0x1, 1, 0);
		set_reg(hiace_base + DPE_UPDATE_FNA, 0x0, 1, 0);
	}
	if (g_single_mode_info.ioctl_info.info_type & EN_HIACE_INFO_TYPE_FNA) {
		set_reg(hiace_base + DPE_UPDATE_LOCAL, 0x0, 1, 0);
		set_reg(hiace_base + DPE_UPDATE_FNA, 0x1, 1, 0);
	}

	/* close no need module */
	outp32(hiace_base + DPE_ROI_MODE_CTRL, 0);
	set_reg(hiace_base + DPE_BYPASS_NR, 1, 1, 0);
	outp32(hiace_base + DPE_HDR10_EN, 0);
#ifndef CONFIG_HISI_FB_V501
	set_reg(dpufd->dss_base + DSS_DISP_CH0_OFFSET + HIACE_V3_HIGHLIGHT, 0, 1, 16);
#endif

	/* enable hiace */
	set_reg(hiace_base + DPE_BYPASS_ACE, 0x0, 1, 0);
}

static void hiace_single_mode_deinit(struct dpu_fb_data_type *dpufd, bool is_done)
{
	char __iomem *hiace_base = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	int disp_panel_id = dpufd->panel_info.disp_panel_id;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;
	ce_ctrl = &(dpufd->ce_ctrl);

	/* disable hiace */
	set_reg(hiace_base + DPE_BYPASS_ACE, 0x1, 1, 0);

	ce_ctrl->ctrl_ce_mode = CE_MODE_DISABLE;
	dpufd->hiace_info[dpufd->panel_info.disp_panel_id].hiace_enable = false;

	if (is_done)
		g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_DONE;
	else
		g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_IDLE;

	/* restore the setting of each module */
	set_reg(hiace_base + DPE_UPDATE_LOCAL, 0x1, 1, 0);
	outp32(hiace_base + DPE_UPDATE_FNA, dpufd->effect_info[disp_panel_id].hiace.update_fna);
	outp32(hiace_base + DPE_ROI_MODE_CTRL, dpufd->effect_info[disp_panel_id].hiace.roi_mode_ctrl);
	outp32(hiace_base + DPE_BYPASS_NR, (dpufd->effect_info[disp_panel_id].hiace.bypass_nr |
		(dpufd->effect_info[disp_panel_id].hiace.bypass_nr_gain << 1)));
	outp32(hiace_base + DPE_HDR10_EN, dpufd->effect_info[disp_panel_id].hiace.hdr10_en);

	DPU_FB_INFO("alreay read all data, close hiace!");
}

static int hiace_single_mode_get_read_point(struct dpu_fb_data_type *dpufd, uint32_t band,
	uint32_t *start_ptr, uint32_t *end_ptr)
{
	uint32_t start;
	uint32_t end;

	start = g_single_mode_info.hist_block_v_ptr * XBLOCKNUM + g_single_mode_info.hist_block_h_ptr;
	end = start + g_single_mode_info.block_once_num;

	DPU_FB_DEBUG("start:%d/stop:%d!\n", start, end);

	if (start >= XBLOCKNUM * YBLOCKNUM) {
		/* reset state machine, close hiace */
		hiace_single_mode_deinit(dpufd, false);
		return -1;
	}

	/* check if end_ptr over the maximum which will occur the total blocks not exact divided by num_once */
	if (end > XBLOCKNUM * YBLOCKNUM)
		end = XBLOCKNUM * YBLOCKNUM;

	start *= band;
	end *= band;

	*start_ptr = start;
	*end_ptr = end;

	return 0;
}

static void hiace_single_mode_update_read_point(struct dpu_fb_data_type *dpufd, uint32_t end_ptr)
{
	/* update the read point */
	g_single_mode_info.hist_block_v_ptr = end_ptr / XBLOCKNUM;
	g_single_mode_info.hist_block_h_ptr = end_ptr % XBLOCKNUM;

	/* update the state machine */
	if (end_ptr == XBLOCKNUM * YBLOCKNUM) {
		/* close the hiace, no need read again */
		/* not call enable_hiace function, because already gotten the blank_sem */
		hiace_single_mode_deinit(dpufd, true);
		wake_up_interruptible(&g_single_mode_info.wq_hist);
	}
}

static int hiace_single_mode_wait_state(struct fb_info *info)
{
	long wait_ret;
	unsigned long timeout = msecs_to_jiffies(HIACE_TIMEOUT_PER_FRAME);

	timeout = timeout * XBLOCKNUM * YBLOCKNUM / g_single_mode_info.block_once_num;

	if (g_single_mode_info.ioctl_info.blocking_mode != 0) {
		/* blocking mode */
		DPU_FB_INFO("enter sleep to wait single mode working done\n");
		unlock_fb_info(info);
		wait_ret = wait_event_interruptible_timeout(g_single_mode_info.wq_hist, /*lint !e578*/
			((g_single_mode_info.single_mode_state & EN_HIACE_SINGLE_MODE_WORKING) == 0),
			timeout);
		(void)lock_fb_info(info);
		DPU_FB_INFO("exit sleep from waiting single mode working done\n");

		/* check if success */
		if (g_single_mode_info.single_mode_state != EN_HIACE_SINGLE_MODE_DONE) {
			/* which means timeout, power off, or hiace normal mode... */
			if (wait_ret == -ERESTARTSYS)
				DPU_FB_WARNING("single mode wait system err\n");
			else if (wait_ret == 0)
				DPU_FB_WARNING("single mode wait done timeout\n");
			else
				DPU_FB_INFO("single mode waiting has been interruputl\n");

			return -EINVAL;
		}

	} else {
		/* non-blocking mode */
		if (g_single_mode_info.single_mode_state & EN_HIACE_SINGLE_MODE_WORKING) {
			DPU_FB_INFO("still in single mode working state:%d, wait some while and try again\n",
				g_single_mode_info.single_mode_state);
			return -EINVAL;
		} else if (g_single_mode_info.single_mode_state != EN_HIACE_SINGLE_MODE_DONE) {
			DPU_FB_WARNING("error state:%d, can not get info\n", g_single_mode_info.single_mode_state);
			return -EINVAL;
		}
	}

	return 0;
}

static void hiace_single_mode_global_hist_get(struct dpu_fb_data_type *dpufd)
{
	char __iomem *hiace_base = NULL;
	uint32_t *global_hist_ptr = NULL;
	uint32_t *sat_global_hist_ptr = NULL;
	uint32_t i;
	uint32_t global_hist_ab_shadow;
	uint32_t global_hist_ab_work;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	global_hist_ab_shadow = inp32(hiace_base + DPE_GLOBAL_HIST_AB_SHADOW);
	global_hist_ab_work = inp32(hiace_base + DPE_GLOBAL_HIST_AB_WORK);
	if (global_hist_ab_shadow == global_hist_ab_work) {
		/* read global hist */
		global_hist_ptr = &g_single_mode_info.hist[0];
		for (i = 0; i < HIACE_GHIST_RANK; i++)
			/* 4 byte one time */
			global_hist_ptr[i] = inp32(hiace_base + DPE_GLOBAL_HIST_LUT_ADDR + i * 4);

		/* read sat_global hist */
		sat_global_hist_ptr = &g_single_mode_info.hist[HIACE_GHIST_RANK]; /* HIACE_GHIST_RANK */
		for (i = 0; i < HIACE_GHIST_RANK; i++)
			/* 4 bytes one time */
			sat_global_hist_ptr[i] = inp32(hiace_base + DPE_SAT_GLOBAL_HIST_LUT_ADDR + i * 4);

		outp32(hiace_base + DPE_GLOBAL_HIST_AB_SHADOW, global_hist_ab_shadow ^ 1);

		/* check if need get local hist further */
		if (g_single_mode_info.ioctl_info.info_type & EN_HIACE_INFO_TYPE_LOCAL_HIST) {
			g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_WORKING_LOCAL_HIST;
		} else {
			hiace_single_mode_deinit(dpufd, true);
			wake_up_interruptible(&g_single_mode_info.wq_hist);
		}
	}
}

static int hiace_single_mode_local_hist_get(struct dpu_fb_data_type *dpufd)
{
	char __iomem *hiace_base = NULL;
	uint32_t *local_hist_ptr = NULL;
	uint32_t i;
	uint32_t lhist_band;
	uint32_t local_valid;
	uint32_t hist_reg;
	uint32_t start_ptr, end_ptr;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	local_valid = inp32(hiace_base + DPE_LOCAL_VALID);

	lhist_band = hiace_get_lhist_band(hiace_base);

	if (local_valid == 1) {
		/* read local hist */
		if (hiace_single_mode_get_read_point(dpufd, lhist_band, &start_ptr, &end_ptr) != 0) {
			wake_up_interruptible(&g_single_mode_info.wq_hist);
			DPU_FB_ERR("should not occur, read point is over limit!\n");
			return -1;
		}
		local_hist_ptr = &g_single_mode_info.hist[HIACE_GHIST_RANK + HIACE_GHIST_RANK];

		/* enable the hist read en, and configure the start block point */
		hist_reg = (g_single_mode_info.hist_block_h_ptr << 3) | g_single_mode_info.hist_block_v_ptr;
		hist_reg <<= 4;  /* low 4 bits is reserved */
		hist_reg |= 0x80000000;  /* BIT31 */
		outp32(hiace_base + DPE_LHIST_EN, hist_reg);

		/* read the hist register */
		for (i = start_ptr; i < end_ptr; i++)
			local_hist_ptr[i] = inp32(hiace_base + DPE_LOCAL_HIST_VxHy_2z_2z1);

		set_reg(hiace_base + DPE_LHIST_EN, 0, 1, 31);  /* BIT31 */

		/* protect: allow loacal hist refresh */
#ifndef CONFIG_HISI_FB_V501
		set_reg(dpufd->dss_base + DSS_DISP_CH0_OFFSET + HIACE_V3_DBUG_HIACE, 0x3, 2, 3);
#endif
		outp32(hiace_base + DPE_UPDATE_LOCAL, 1);

		/* verify the blocks of hist, discard if not reasonable */
		if (!hiace_single_mode_verify_hist_block(dpufd)) {
			DPU_FB_WARNING("There are some errors in these %d blocks, which start from hist block:%d/%d!\n",
				g_single_mode_info.block_once_num,
				g_single_mode_info.hist_block_h_ptr,
				g_single_mode_info.hist_block_v_ptr);
			return -1;
		}

		/* update the read point */
		hiace_single_mode_update_read_point(dpufd, (end_ptr / lhist_band));
	}

	return 0;
}

static int hiace_single_mode_fna_data_get(struct dpu_fb_data_type *dpufd)
{
	char __iomem *hiace_base = NULL;
	uint32_t *fna_data_ptr = NULL;
	uint32_t i;
	uint32_t fna_valid;
	uint32_t hist_reg;
	uint32_t start_ptr;
	uint32_t end_ptr;

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	fna_valid = inp32(hiace_base + DPE_FNA_VALID);
	if (fna_valid == 1) {
		/* read fna data */
		if (hiace_single_mode_get_read_point(dpufd, HIACE_FNA_RANK, &start_ptr, &end_ptr) != 0) {
			wake_up_interruptible(&g_single_mode_info.wq_hist);
			DPU_FB_ERR("should not occur, read point is over limit!");
			return -1;
		}
		fna_data_ptr = &g_single_mode_info.fna[0];

		/* enable the hist read en, and configure the start block point */
		hist_reg = (g_single_mode_info.hist_block_h_ptr << 3) | g_single_mode_info.hist_block_v_ptr;
		hist_reg <<= 4;  /* low 4 bits is reserved */
		hist_reg |= 0x80000000;  /* BIT31 */
		outp32(hiace_base + DPE_FNA_EN, hist_reg);

		for (i = start_ptr; i < end_ptr; i++)
			fna_data_ptr[i] = inp32(hiace_base + DPE_FNA_VxHy);

		set_reg(hiace_base + DPE_FNA_EN, 0, 1, 31);  /* BIT31 */

		/* protect: allow loacal hist refresh */
#ifndef CONFIG_HISI_FB_V501
		set_reg(dpufd->dss_base + DSS_DISP_CH0_OFFSET + HIACE_V3_DBUG_HIACE, 0x3, 2, 3);
#endif
		outp32(hiace_base + DPE_UPDATE_FNA, 1);

		/* update the read point */
		hiace_single_mode_update_read_point(dpufd, (end_ptr / HIACE_FNA_RANK));
	}

	return 0;
}

/* check input parameter and get dpufb ptr */
static int hiace_single_mode_check_input(struct fb_info *info, const void __user *argp)
{
	struct dpu_fb_data_type *dpufd = NULL;

	if ((info == NULL) || (argp == NULL)) {
		DPU_FB_ERR("info or argp is NULL\n");
		return -EINVAL;
	}

	if (!g_single_mode_init) {
		DPU_FB_ERR("hiace single mode is not init\n");
		return -EINVAL;
	}

	dpufd = (struct dpu_fb_data_type *)info->par;
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_ERR("fb%d, not support!\n", dpufd->index);
		return -EINVAL;
	}
	return 0;
}

static int hiace_single_mode_wq_check_input(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;

	if (work == NULL) {
		DPU_FB_ERR("work is NULL\n");
		return -EINVAL;
	}

	if (!g_single_mode_init) {
		DPU_FB_ERR("hiace single mode is not init\n");
		return -EINVAL;
	}

	dpufd = container_of(work, struct dpu_fb_data_type, hiace_end_work);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_ERR("fb%d, not support!\n", dpufd->index);
		return -EINVAL;
	}
	return 0;
}

/* External API */
void dpufb_hiace_single_mode_init(void)
{
	if (!g_single_mode_init) {
		memset(&g_single_mode_info, 0, sizeof(g_single_mode_info));
		init_waitqueue_head(&g_single_mode_info.wq_hist);
		sema_init(&g_single_mode_info.wq_sem, 1);
		mutex_init(&g_single_mode_info.hist_lock);
		g_single_mode_info.block_once_num = YBLOCKNUM * XBLOCKNUM;  /* use the max block num default */

		g_single_mode_init = true;
		DPU_FB_INFO("hiace single mode init done\n");
	}
}

void dpufb_hiace_single_mode_terminate(struct dpu_fb_data_type *dpufd, bool disable)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (g_single_mode_init && (dpufd->ce_ctrl.ctrl_ce_mode == CE_MODE_SINGLE)) {
		mutex_lock(&g_single_mode_info.hist_lock);
		if (disable)
			dpufd->ce_ctrl.ctrl_ce_mode = CE_MODE_DISABLE;

		g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_IDLE;
		mutex_unlock(&g_single_mode_info.hist_lock);
		wake_up_interruptible(&g_single_mode_info.wq_hist);
	}
}

int dpufb_hiace_single_mode_trigger(struct fb_info *info, const void __user *argp)
{
	int ret;
	char __iomem *hiace_base = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	dss_ce_info_t *ce_info = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (hiace_single_mode_check_input(info, argp) != 0)
		return -EINVAL;

	dpufd = (struct dpu_fb_data_type *)info->par;
	ce_ctrl = &(dpufd->ce_ctrl);
	ce_info = &(dpufd->hiace_info[dpufd->panel_info.disp_panel_id]);

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	/* check if already in hiace normal mode */
	if ((ce_ctrl->ctrl_ce_mode == CE_MODE_VIDEO) || (ce_ctrl->ctrl_ce_mode == CE_MODE_IMAGE)) {
		DPU_FB_WARNING("already in hiace normal mode, should not call this function\n");
		return -EINVAL;
	}

	DPU_FB_INFO("+\n");

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_WARNING("panel power off!\n");
		up(&dpufd->blank_sem);
		return -EINVAL;
	}

	dpufb_activate_vsync(dpufd);

	/* need to synchronize with isr routine */
	mutex_lock(&g_single_mode_info.hist_lock);

	ret = (int)copy_from_user(&g_single_mode_info.ioctl_info, argp, sizeof(struct dss_hiace_single_mode_ctrl_info));
	if (ret) {
		mutex_unlock(&g_single_mode_info.hist_lock);

		dpufb_deactivate_vsync(dpufd);
		up(&dpufd->blank_sem);
		DPU_FB_ERR("copy_from_user(param) failed! ret=%d\n", ret);
		return -EINVAL;
	}

	/* check if the last single moe is still working */
	if (ce_ctrl->ctrl_ce_mode == CE_MODE_SINGLE) {
		if (g_single_mode_info.single_mode_state & EN_HIACE_SINGLE_MODE_WORKING) {
			/* the last working not done, reset state machine */
			hiace_single_mode_state_init();

			hiace_single_mode_reset_read_point(hiace_base);
		}
	} else if (ce_ctrl->ctrl_ce_mode == CE_MODE_DISABLE) {
		/* start new single mode */
		hiace_single_mode_init(dpufd);
	}

	mutex_unlock(&g_single_mode_info.hist_lock);

	dpufb_deactivate_vsync(dpufd);
	up(&dpufd->blank_sem);

	DPU_FB_INFO("-\n");

	return 0;
}

int dpufb_hiace_single_mode_block_once_set(struct fb_info *info, const void __user *argp)
{
	int ret;
	uint32_t block_once_num;
	dss_ce_info_t *ce_info = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (hiace_single_mode_check_input(info, argp) != 0)
		return -EINVAL;

	dpufd = (struct dpu_fb_data_type *)info->par;
	ce_ctrl = &(dpufd->ce_ctrl);
	ce_info = &(dpufd->hiace_info[dpufd->panel_info.disp_panel_id]);

	DPU_FB_INFO("+\n");

	/* check if still in single mode working state */
	if ((ce_ctrl->ctrl_ce_mode == CE_MODE_SINGLE) &&
		(g_single_mode_info.single_mode_state & EN_HIACE_SINGLE_MODE_WORKING)) {
		DPU_FB_ERR("can not set in single mode working state!\n");
		return -EINVAL;
	}

	ret = (int)copy_from_user(&block_once_num, argp, sizeof(uint32_t));
	if (ret) {
		DPU_FB_ERR("copy_from_user(param) failed! ret=%d.\n", ret);
		return -EINVAL;
	}

	if ((block_once_num > XBLOCKNUM * YBLOCKNUM) || (block_once_num == 0)) {
		g_single_mode_info.block_once_num = XBLOCKNUM * YBLOCKNUM;
		DPU_FB_WARNING("block_num=%d is not in limit, use the default value\n", block_once_num);
	} else {
		g_single_mode_info.block_once_num = block_once_num;
	}

	DPU_FB_INFO("-\n");
	return 0;
}

int dpufb_hiace_hist_get(struct fb_info *info, void __user *argp)
{
	dss_display_effect_ce_t *ce_ctrl = NULL;
	uint32_t *hist_ptr = NULL;
	uint32_t hist_size;
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (hiace_single_mode_check_input(info, argp) != 0)
		return -EINVAL;

	dpufd = (struct dpu_fb_data_type *)info->par;
	ce_ctrl = &(dpufd->ce_ctrl);

	if (g_single_mode_info.ioctl_info.info_type > EN_HIACE_INFO_TYPE_HIST) {
		DPU_FB_ERR("mis match with trigger info type[%d]!\n", g_single_mode_info.ioctl_info.info_type);
		return -EINVAL;
	}

	DPU_FB_INFO("+\n");

	ret = hiace_single_mode_wait_state(info);
	if (ret != 0) {
		DPU_FB_WARNING("fail to get loacl_hist due to wait state fail!\n");
		return ret;
	}

	/* next, means succeed to wait single mode working done */
	mutex_lock(&g_single_mode_info.hist_lock);

	/* copy the complete info to user */
	if (g_single_mode_info.ioctl_info.info_type == EN_HIACE_INFO_TYPE_GLOBAL_HIST) {
		hist_ptr = &g_single_mode_info.hist[0];
		hist_size = HIACE_GHIST_RANK * 2 * sizeof(uint32_t);
		DPU_FB_INFO("copy global hist to user!\n");
	} else if (g_single_mode_info.ioctl_info.info_type == EN_HIACE_INFO_TYPE_LOCAL_HIST) {
		hist_ptr = &g_single_mode_info.hist[HIACE_GHIST_RANK * 2];
		hist_size = YBLOCKNUM * XBLOCKNUM * HIACE_LHIST_RANK * sizeof(uint32_t);
		DPU_FB_INFO("copy local hist to user!\n");
	} else {  /* means EN_HIACE_INFO_TYPE_HIST case */
		hist_ptr = &g_single_mode_info.hist[0];
		hist_size = sizeof(g_single_mode_info.hist);
		DPU_FB_INFO("copy global & local hist to user!\n");
	}
	ret = (int)copy_to_user(argp, hist_ptr, hist_size);
	if (ret) {
		DPU_FB_ERR("copy_to_user failed(param)! ret=%d\n", ret);
		ret = -1;
	}

	g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_IDLE;
	mutex_unlock(&g_single_mode_info.hist_lock);

	DPU_FB_INFO("-\n");
	return ret;
}

int dpufb_hiace_fna_get(struct fb_info *info, void __user *argp)
{
	dss_display_effect_ce_t *ce_ctrl = NULL;
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (hiace_single_mode_check_input(info, argp) != 0)
		return -EINVAL;

	dpufd = (struct dpu_fb_data_type *)info->par;
	ce_ctrl = &(dpufd->ce_ctrl);

	if (g_single_mode_info.ioctl_info.info_type != EN_HIACE_INFO_TYPE_FNA) {
		DPU_FB_ERR("mis-match with trigger info type[%d]!\n", g_single_mode_info.ioctl_info.info_type);
		return -EINVAL;
	}

	DPU_FB_INFO("+\n");

	ret = hiace_single_mode_wait_state(info);
	if (ret) {
		DPU_FB_WARNING("fail to get loacl_hist due to wait state fail!\n");
		return ret;
	}

	/* next, means succeed to wait single mode working done */
	mutex_lock(&g_single_mode_info.hist_lock);

	// copy the complete info to user
	ret = (int)copy_to_user(argp, g_single_mode_info.fna, sizeof(g_single_mode_info.fna));
	if (ret) {
		DPU_FB_ERR("copy_to_user failed(param)! ret=%d\n", ret);
		ret = -1;
	}

	g_single_mode_info.single_mode_state = EN_HIACE_SINGLE_MODE_IDLE;
	mutex_unlock(&g_single_mode_info.hist_lock);

	DPU_FB_INFO("-\n");
	return ret;
}


void hisi_hiace_single_mode_wq_handler(struct work_struct *work)
{
	char __iomem *hiace_base = NULL;
	struct dpu_fb_data_type *dpufd = NULL;

	if (hiace_single_mode_wq_check_input(work) != 0)
		return;

	DPU_FB_INFO("+\n");

	dpufd = container_of(work, struct dpu_fb_data_type, hiace_end_work);
	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	down(&g_single_mode_info.wq_sem);  /* one entry work queue handler */

	down(&dpufd->blank_sem);
	if (!dpufd->panel_power_on) {
		DPU_FB_DEBUG("panel power off!\n");
		up(&dpufd->blank_sem);

		up(&g_single_mode_info.wq_sem);
		return;
	}

	dpufb_activate_vsync(dpufd);

	if (!(g_single_mode_info.single_mode_state & EN_HIACE_SINGLE_MODE_WORKING)) {
		DPU_FB_WARNING("not in working state, do nothing!");
		goto error_exit;
	}

	/* start working to get info */
	mutex_lock(&g_single_mode_info.hist_lock);

	if (g_single_mode_info.single_mode_state == EN_HIACE_SINGLE_MODE_WORKING_GLOBAL_HIST)
		/* get global hist */
		hiace_single_mode_global_hist_get(dpufd);
	else if (g_single_mode_info.single_mode_state == EN_HIACE_SINGLE_MODE_WORKING_LOCAL_HIST)
		/* get specified blocks of local hist */
		hiace_single_mode_local_hist_get(dpufd);
	else if (g_single_mode_info.single_mode_state == EN_HIACE_SINGLE_MODE_WORKING_FNA)
		/* get fna */
		hiace_single_mode_fna_data_get(dpufd);

	mutex_unlock(&g_single_mode_info.hist_lock);

error_exit:
	/* clear INT */
	outp32(hiace_base + DPE_INT_STAT, 0x1);

	dpufb_deactivate_vsync(dpufd);
	up(&dpufd->blank_sem);

	up(&g_single_mode_info.wq_sem);

	DPU_FB_INFO("-\n");
}

bool hisi_hiace_single_mode_handle_isr(struct dpu_fb_data_type *dpufd)
{
	bool ret = false;
	char __iomem *hiace_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	hiace_base = dpufd->dss_base + DSS_HI_ACE_OFFSET;

	if (dpufd->ce_ctrl.ctrl_ce_mode != CE_MODE_SINGLE)
		return false;

	if (g_single_mode_info.single_mode_state == EN_HIACE_SINGLE_MODE_WORKING_ISR_FNA) {
		hiace_single_mode_fna_data_get(dpufd);

		/* clear INT */
		outp32(hiace_base + DPE_INT_STAT, 0x1);
		ret = true;
	}

	return ret;
}
