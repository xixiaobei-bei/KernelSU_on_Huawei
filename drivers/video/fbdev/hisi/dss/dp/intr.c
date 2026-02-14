/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#include "hisi_fb.h"
#include "hisi_dp.h"
#include "hisi_fb_def.h"

#include "avgen.h"
#include "dp_aux.h"
#include "link.h"
#include "intr.h"
#include "core.h"
#include "edid.h"
#include "hdcp22/hdcp13.h"


/*lint -save -e* */
#define EDID_NUM 256
#define SAFE_MODE_TIMING_HACTIVE 640
#define SAFE_MODE_TIMING_PIXEL_CLOCK 2517  /*The pixel clock of 640 * 480 = 25175. The saving pixel clock need 1/10.*/
#define DPTX_CHECK_TIME_PERIOD 2000

#define VBLANKING_MAX 255

uint32_t g_bit_hpd_status;

enum dp_event_type
{
	DP_LINK_STATE_BAD = 0,
	DP_LINK_STATE_GOOD = 1
};

extern void dp_send_event(enum dp_event_type event);

static enum hrtimer_restart dptx_detect_hrtimer_fnc(struct hrtimer *timer)
{
	struct dp_ctrl *dptx = NULL;

	if (NULL == timer) {
		HISI_FB_ERR("[DP] timer is NULL");
		return HRTIMER_NORESTART;
	}

	dptx = container_of(timer, struct dp_ctrl, dptx_hrtimer);

	if (NULL == dptx) {
		HISI_FB_ERR("[DP] dptx is NULL");
		return HRTIMER_NORESTART;
	}

	if (dptx->dptx_check_wq) {
		queue_work(dptx->dptx_check_wq, &(dptx->dptx_check_work));
	}

	hrtimer_start(&dptx->dptx_hrtimer, ktime_set(DPTX_CHECK_TIME_PERIOD / 1000,
		(DPTX_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);

	return HRTIMER_NORESTART;
}

static bool dptx_check_vr_err_count(struct dp_ctrl *dptx)
{
	uint8_t vector, vector1;
	uint16_t Lane0_err;
	uint16_t Lane1_err;
	uint16_t Lane2_err;
	uint16_t Lane3_err;
	int retval;

	retval = 0;
	vector = vector1 =0;
	Lane0_err = Lane1_err = Lane2_err = Lane3_err = 0;

	if ((dptx->video_transfer_enable) && (dptx->dptx_vr)) {
		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE0_L, &vector);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE0_H, &vector1);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}
		vector1 &= 0x7F;
		Lane0_err = ((vector1 << 8) | vector);

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE1_L, &vector);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE1_H, &vector1);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}
		vector1 &= 0x7F;
		Lane1_err = ((vector1 << 8) | vector);

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE2_L, &vector);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE2_H, &vector1);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}
		vector1 &= 0x7F;
		Lane2_err = ((vector1 << 8) | vector);

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE3_L, &vector);
		if (retval) {
			HISI_FB_ERR("Read DPCD error\n");
			return FALSE;
		}

		retval = dptx_read_dpcd(dptx, DP_SYMBOL_ERROR_COUNT_LANE3_H, &vector1);
		if (retval) {
			HISI_FB_ERR("[DP] Read DPCD error\n");
			return FALSE;
		}
		vector1 &= 0x7F;
		Lane3_err = ((vector1 << 8) | vector);

		if (Lane0_err || Lane1_err || Lane2_err || Lane3_err) {
			HISI_FB_ERR("[DP] Lane x ERR count: (0x%x); (0x%x); (0x%x); (0x%x).\n",
			Lane0_err, Lane1_err, Lane2_err, Lane3_err);
			dp_imonitor_set_param_err_count(Lane0_err, Lane1_err, Lane2_err, Lane3_err);
			return FALSE;
		}
	}

	return TRUE;
}

static void dptx_err_count_check_wq_handler(struct work_struct *work)
{
	struct dp_ctrl *dptx = NULL;
	bool berr;

	dptx = container_of(work, struct dp_ctrl, dptx_check_work);

	if (NULL== dptx) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	if ((!dptx->dptx_enable) || (!dptx->video_transfer_enable)) {
		return;
	}

	berr = dptx_check_vr_err_count(dptx);

	/* dptx need check err count when video has been transmited on wallex.
	The first checking result should be discarded, and then, we need report
	the bad message when the err count has been detected by 3 times in
	a row. The time interval of detecting is 3 second.*/

	if((!berr) && (dptx->detect_times >= 1)) {
		dptx->detect_times ++;
	} else if (berr) {
		dptx->detect_times = 1;
	} else {
		dptx->detect_times ++;
	}

	if (dptx->detect_times == 4) {
		dp_send_event(DP_LINK_STATE_BAD);
		dptx->detect_times = 1;
		HISI_FB_INFO("\n [DP] ERR count upload!");
	}

	return;
}

static int dptx_init_detect_work(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] Init Detect work \n");

	if (!dptx->dptx_detect_inited) {
		dptx->dptx_check_wq = create_singlethread_workqueue("dptx_check");
		if (!dptx->dptx_check_wq) {
			HISI_FB_ERR("[DP] create dptx_check_wq failed\n");
			return -1;
		}

		INIT_WORK(&dptx->dptx_check_work, dptx_err_count_check_wq_handler);

		/* hrtimer for detecting error count*/
		hrtimer_init(&dptx->dptx_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		dptx->dptx_hrtimer.function = dptx_detect_hrtimer_fnc;
		hrtimer_start(&dptx->dptx_hrtimer, ktime_set(DPTX_CHECK_TIME_PERIOD / 1000,
			(DPTX_CHECK_TIME_PERIOD % 1000) * 1000000), HRTIMER_MODE_REL);

		dptx->dptx_detect_inited = true;
	}
	return 0;
}

static int dptx_cancel_detect_work(struct dp_ctrl *dptx)
{
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] Cancel Detect work \n");

	if (dptx->dptx_detect_inited) {
		if (dptx->dptx_check_wq) {
			destroy_workqueue(dptx->dptx_check_wq);
			dptx->dptx_check_wq = NULL;
		}

		hrtimer_cancel(&dptx->dptx_hrtimer);

		dptx->dptx_detect_inited = false;
	}

	return 0;
}

static int dptx_resolution_switch(struct hisi_fb_data_type *hisifd, enum dptx_hot_plug_type etype)
{
	struct dtd *mdtd;
	struct dp_ctrl *dptx;
	struct hisi_panel_info *pinfo;
	struct video_params *vparams;
	int ret;

	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	dptx = &(hisifd->dp);
	vparams= &(dptx->vparams);
	mdtd = &(dptx->vparams.mdtd);

	pinfo = &(hisifd->panel_info);

	pinfo->xres = mdtd->h_active;
	pinfo->yres = mdtd->v_active;
	pinfo->ldi.h_back_porch =
		(mdtd->h_blanking - mdtd->h_sync_offset -mdtd->h_sync_pulse_width);
	pinfo->ldi.h_front_porch = mdtd->h_sync_offset;
	pinfo->ldi.h_pulse_width = mdtd->h_sync_pulse_width;
	pinfo->ldi.v_back_porch =
		(mdtd->v_blanking - mdtd->v_sync_offset - mdtd->v_sync_pulse_width);
	pinfo->ldi.v_front_porch = mdtd->v_sync_offset;
	pinfo->ldi.v_pulse_width = mdtd->v_sync_pulse_width;
	pinfo->ldi.hsync_plr = 1 - mdtd->h_sync_polarity;
	pinfo->ldi.vsync_plr = 1 - mdtd->v_sync_polarity;
	pinfo->pxl_clk_rate_div = 1;
	pinfo->width = 530;
	pinfo->height = 300;
	if (dptx->edid_info.Video.maxHImageSize != 0) {
		pinfo->width = dptx->edid_info.Video.maxHImageSize * 10;
		pinfo->height = dptx->edid_info.Video.maxVImageSize * 10;
	} else {
		HISI_FB_ERR("[DP] The size of display device cannot be got from edid information.\n");
	}

	pinfo->pxl_clk_rate = mdtd->pixel_clock * 1000;

	hisifd->fbi->var.xres = pinfo->xres;
	hisifd->fbi->var.yres = pinfo->yres;
	hisifd->fbi->var.width = pinfo->width;
	hisifd->fbi->var.height = pinfo->height;

	hisifd->fbi->var.pixclock = pinfo->pxl_clk_rate;

	if(((mdtd->h_active + mdtd->h_blanking) * (mdtd->v_active + mdtd->v_blanking)) > 0) {
		vparams->m_fps = (uint32_t)(pinfo->pxl_clk_rate / ((mdtd->h_active + mdtd->h_blanking)
					* (mdtd->v_active + mdtd->v_blanking)));
	} else {
		return -1;
	}

	HISI_FB_INFO("[DP] xres=%d\n"
		"yres=%d\n"
		"h_back_porch=%d\n"
		"h_front_porch=%d\n"
		"h_pulse_width=%d\n"
		"v_back_porch=%d\n"
		"v_front_porch=%d\n"
		"v_pulse_width=%d\n"
		"hsync_plr=%d\n"
		"vsync_plr=%d\n"
		"pxl_clk_rate_div=%d\n"
		"pxl_clk_rate=%llu\n"
		"var.xres=%d\n"
		"var.yres=%d\n"
		"var.width=%d\n"
		"var.height=%d\n"
		"m_fps = %d",
		pinfo->xres,
		pinfo->yres,
		pinfo->ldi.h_back_porch,
		pinfo->ldi.h_front_porch,
		pinfo->ldi.h_pulse_width,
		pinfo->ldi.v_back_porch,
		pinfo->ldi.v_front_porch,
		pinfo->ldi.v_pulse_width,
		pinfo->ldi.hsync_plr,
		pinfo->ldi.vsync_plr,
		pinfo->pxl_clk_rate_div,
		pinfo->pxl_clk_rate,
		hisifd->fbi->var.xres,
		hisifd->fbi->var.yres,
		hisifd->fbi->var.width,
		hisifd->fbi->var.height,
		vparams->m_fps);

	dp_imonitor_set_param(DP_PARAM_WIDTH, &(mdtd->h_active));
	dp_imonitor_set_param(DP_PARAM_HIGH,  &(mdtd->v_active));
	dp_imonitor_set_param(DP_PARAM_FPS,   &(vparams->m_fps));

	if(hisifd->hpd_open_sub_fnc) {
		ret = hisifd->hpd_open_sub_fnc(hisifd->fbi);
		if (ret) {
			HISI_FB_ERR("[DP] dss pwr on failed.\n");
			dp_imonitor_set_param(DP_PARAM_DSS_PWRON_FAILED, NULL);
			return -1;
		} else {
			dptx->video_transfer_enable = true;
			dp_send_cable_notification(dptx, etype);
			return 0;
		}
	}

	return -1;
}

static int handle_test_link_training(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t lanes;
	uint8_t rate;
	struct video_params *vparams;
	struct dtd *mdtd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_read_dpcd(dptx, DP_TEST_LINK_RATE, &rate);
	if (retval)
		return retval;

	retval = dptx_bw_to_phy_rate(rate);
	if (retval < 0)
		return retval;

	rate = retval;

	retval = dptx_read_dpcd(dptx, DP_TEST_LANE_COUNT, &lanes);
	if (retval)
		return retval;

	HISI_FB_INFO("[DP] %s: Strating link training rate=%d, lanes=%d\n",
		 __func__, rate, lanes);

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;

	retval = dptx_video_ts_calculate(dptx, lanes, rate,
					 vparams->bpc, vparams->pix_enc,
					 mdtd->pixel_clock);
	if (retval)
		return retval;

	retval = dptx_link_training(dptx,
				    rate,
				    lanes);
	if (retval)
		HISI_FB_ERR("[DP] Link training failed %d\n", retval);
	else
		HISI_FB_INFO("[DP] Link training succeeded\n");

	return retval;
}

static int handle_test_link_video_timming(struct dp_ctrl *dptx)
{
	int retval, i;
	uint8_t test_h_total_lsb, test_h_total_msb, test_v_total_lsb,
	   test_v_total_msb, test_h_start_lsb, test_h_start_msb,
	   test_v_start_lsb, test_v_start_msb, test_hsync_width_lsb,
	   test_hsync_width_msb, test_vsync_width_lsb, test_vsync_width_msb,
	   test_h_width_lsb, test_h_width_msb, test_v_width_lsb,
	   test_v_width_msb;
	uint32_t h_total, v_total, h_start, v_start, h_width, v_width,
	    hsync_width, vsync_width, h_sync_pol, v_sync_pol, refresh_rate;
	enum video_format_type video_format;
	uint8_t vmode;
	uint8_t test_refresh_rate;
	struct video_params *vparams;
	struct dtd mdtd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	retval = 0;
	h_total = 0;
	v_total = 0;
	h_start = 0;
	v_start = 0;
	v_width = 0;
	h_width = 0;
	hsync_width = 0;
	vsync_width = 0;
	h_sync_pol = 0;
	v_sync_pol = 0;
	test_refresh_rate = 0;
	i = 0;

	/* H_TOTAL */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_TOTAL_LSB, &test_h_total_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_TOTAL_MSB, &test_h_total_msb);
	if (retval)
		return retval;
	h_total |= test_h_total_lsb;
	h_total |= test_h_total_msb << 8;
	HISI_FB_INFO("[DP] h_total = %d\n", h_total);

	/* V_TOTAL */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_TOTAL_LSB, &test_v_total_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_TOTAL_MSB, &test_v_total_msb);
	if (retval)
		return retval;
	v_total |= test_v_total_lsb;
	v_total |= test_v_total_msb << 8;
	HISI_FB_INFO("[DP] v_total = %d\n", v_total);

	/*  H_START */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_START_LSB, &test_h_start_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_START_MSB, &test_h_start_msb);
	if (retval)
		return retval;
	h_start |= test_h_start_lsb;
	h_start |= test_h_start_msb << 8;
	HISI_FB_INFO("[DP] h_start = %d\n", h_start);

	/* V_START */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_START_LSB, &test_v_start_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_START_MSB, &test_v_start_msb);
	if (retval)
		return retval;
	v_start |= test_v_start_lsb;
	v_start |= test_v_start_msb << 8;
	HISI_FB_INFO("[DP] v_start = %d\n", v_start);

	/* TEST_HSYNC */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_SYNC_WIDTH_LSB,
				&test_hsync_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_SYNC_WIDTH_MSB,
				&test_hsync_width_msb);
	if (retval)
		return retval;
	hsync_width |= test_hsync_width_lsb;
	hsync_width |= (test_hsync_width_msb & (~(1 << 7))) << 8;
	h_sync_pol |= (test_hsync_width_msb & (1 << 7)) >> 8;
	HISI_FB_INFO("[DP] hsync_width = %d\n", hsync_width);
	HISI_FB_INFO("[DP] h_sync_pol = %d\n", h_sync_pol);

	/* TEST_VSYNC */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_SYNC_WIDTH_LSB,
				&test_vsync_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_SYNC_WIDTH_MSB,
				&test_vsync_width_msb);
	if (retval)
		return retval;
	vsync_width |= test_vsync_width_lsb;
	vsync_width |= (test_vsync_width_msb & (~(1 << 7))) << 8;
	v_sync_pol |= (test_vsync_width_msb & (1 << 7)) >> 8;
	HISI_FB_INFO("[DP] vsync_width = %d\n", vsync_width);
	HISI_FB_INFO("[DP] v_sync_pol = %d\n", v_sync_pol);

	/* TEST_H_WIDTH */
	retval = dptx_read_dpcd(dptx, DP_TEST_H_WIDTH_LSB, &test_h_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_H_WIDTH_MSB, &test_h_width_msb);
	if (retval)
		return retval;
	h_width |= test_h_width_lsb;
	h_width |= test_h_width_msb << 8;
	HISI_FB_INFO("[DP] h_width = %d\n", h_width);

	/* TEST_V_WIDTH */
	retval = dptx_read_dpcd(dptx, DP_TEST_V_WIDTH_LSB, &test_v_width_lsb);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_V_WIDTH_MSB, &test_v_width_msb);
	if (retval)
		return retval;
	v_width |= test_v_width_lsb;
	v_width |= test_v_width_msb << 8;
	HISI_FB_INFO("[DP] v_width = %d\n", v_width);

	retval = dptx_read_dpcd(dptx, 0x234, &test_refresh_rate);
	if (retval)
		return retval;
	HISI_FB_INFO("[DP] test_refresh_rate = %d\n", test_refresh_rate);

	video_format = DMT;
	refresh_rate =  test_refresh_rate * 1000;

	if (h_total == 1056 && v_total == 628 && h_start == 216 &&
	    v_start == 27 && hsync_width == 128 && vsync_width == 4 &&
	    h_width == 800 && v_width == 600) {
		vmode = 9;
	} else if (h_total == 1088 && v_total == 517 && h_start == 224 &&
		   v_start == 31 && hsync_width == 112 && vsync_width == 8 &&
		   h_width == 848 && v_width == 480) {
		vmode = 14;
	} else if (h_total == 1344 && v_total == 806 && h_start == 296 &&
		   v_start == 35 && hsync_width == 136 && vsync_width == 6 &&
		   h_width == 1024 && v_width == 768) {
		vmode = 16;
	} else if (h_total == 1440 && v_total == 790 && h_start == 112 &&
		   v_start == 19 && hsync_width == 32 && vsync_width == 7 &&
		   h_width == 1280 && v_width == 768) {
		vmode = 22;
	} else if (h_total == 1664 && v_total == 798 && h_start == 320 &&
		   v_start == 27 && hsync_width == 128 && vsync_width == 7 &&
		   h_width == 1280 && v_width == 768) {
		vmode = 23;
	} else if (h_total == 1440 && v_total == 823 && h_start == 112 &&
		   v_start == 20 && hsync_width == 32 && vsync_width == 6 &&
		   h_width == 1280 && v_width == 800) {
		vmode = 27;
	} else if (h_total == 1800 && v_total == 1000 && h_start == 424 &&
		   v_start == 39 && hsync_width == 112 && vsync_width == 3 &&
		   h_width == 1280 && v_width == 960) {
		vmode = 32;
	} else if (h_total == 1688 && v_total == 1066 && h_start == 360 &&
		   v_start == 41 && hsync_width == 112 && vsync_width == 3 &&
		   h_width == 1280 && v_width  == 1024) {
		vmode = 35;
	} else if (h_total == 1792 && v_total == 795 && h_start == 368 &&
		   v_start == 24 && hsync_width == 112 && vsync_width == 6 &&
		   h_width == 1360 && v_width == 768) {
		vmode = 39;
	} else if (h_total == 1560 && v_total == 1080  && h_start == 112  &&
		   v_start == 27 && hsync_width == 32 && vsync_width == 4 &&
		   h_width == 1400 && v_width == 1050) {
		vmode = 41;
	} else if (h_total == 2160 && v_total == 1250 && h_start == 496 &&
		   v_start == 49 && hsync_width == 192 && vsync_width == 3 &&
		   h_width == 1600 && v_width == 1200) {
		vmode = 51;
	} else if (h_total == 2448 && v_total == 1394 && h_start == 528 &&
		   v_start == 49 && hsync_width == 200 && vsync_width == 3 &&
		   h_width == 1792 && v_width == 1344) {
		vmode = 62;
	} else if (h_total == 2600 && v_total == 1500 && h_start == 552 &&
		   v_start == 59  && hsync_width == 208 && vsync_width == 3 &&
		   h_width == 1920 && v_width == 1440) {
		vmode = 73;
	} else if (h_total == 2200 && v_total == 1125 && h_start == 192 &&
		   v_start == 41 && hsync_width == 44 && vsync_width == 5 &&
		   h_width == 1920 && v_width == 1080) {
		if (refresh_rate == 120000) {
			vmode = 63;
			video_format = VCEA;
		} else {
			vmode = 82;
		}
	} else if (h_total == 800 && v_total == 525 && h_start == 144 &&
		   v_start == 35 && hsync_width == 96 && vsync_width == 2 &&
		   h_width == 640 && v_width == 480) {
		vmode = 1;
		video_format = VCEA;
	} else if (h_total == 1650 && v_total == 750 && h_start == 260 &&
		   v_start == 25 && hsync_width == 40 && vsync_width == 5 &&
		   h_width == 1280  && v_width == 720) {
		vmode = 4;
		video_format = VCEA;
	} else if (h_total == 1680 && v_total == 831 && h_start == 328 &&
		   v_start == 28 && hsync_width == 128 && vsync_width == 6 &&
		   h_width == 1280 && v_width == 800) {
		vmode = 28;
		video_format = CVT;
	} else if (h_total == 1760 && v_total == 1235 && h_start == 112 &&
		   v_start == 32 && hsync_width == 32 && vsync_width == 4 &&
		   h_width == 1600 && v_width == 1200) {
		vmode = 40;
		video_format = CVT;
	} else if (h_total == 2208 && v_total == 1580 && h_start == 112 &&
		   v_start == 41 && hsync_width == 32 &&  vsync_width == 4 &&
		   h_width == 2048 && v_width == 1536) {
		vmode = 41;
		video_format = CVT;
	} else {
		HISI_FB_INFO("[DP] Unknown video mode\n");
		return -EINVAL;
	}

	if (!dptx_dtd_fill(&mdtd, vmode, refresh_rate, video_format)) {
		HISI_FB_INFO("[DP] %s: Invalid video mode value %d\n",
			 __func__, vmode);
		retval = -EINVAL;
		goto fail;
	}
	vparams->mdtd = mdtd;
	vparams->refresh_rate = refresh_rate;
	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
					 dptx->link.rate, vparams->bpc,
					 vparams->pix_enc, mdtd.pixel_clock);
	if (retval)
		return retval;
	/* MMCM */
	dptx_resolution_switch(dptx->hisifd, Hot_Plug_TEST);
	dptx_video_timing_change(dptx);
fail:
	return retval;

}

static int handle_test_link_audio_pattern(struct dp_ctrl *dptx)
{
	int retval, freq_id;
	uint8_t test_audio_mode, test_audio_smaple_range, test_audio_ch_count,
	   audio_ch_count, orig_sample_freq, sample_freq;
	struct audio_params *aparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	aparams = &dptx->aparams;
	retval = dptx_read_dpcd(dptx, DP_TEST_AUDIO_MODE, &test_audio_mode);
	if (retval)
		return retval;

	HISI_FB_INFO("[DP] test_audio_mode= %d\n", test_audio_mode);

	test_audio_smaple_range = test_audio_mode &
		DP_TEST_AUDIO_SAMPLING_RATE_MASK;
	test_audio_ch_count = (test_audio_mode & DP_TEST_AUDIO_CH_COUNT_MASK)
		>> DP_TEST_AUDIO_CH_COUNT_SHIFT;

	switch (test_audio_ch_count) {
	case DP_TEST_AUDIO_CHANNEL1:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL1\n");
		audio_ch_count = 1;
		break;
	case DP_TEST_AUDIO_CHANNEL2:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL2\n");
		audio_ch_count = 2;
		break;
	case DP_TEST_AUDIO_CHANNEL3:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL3\n");
		audio_ch_count = 3;
		break;
	case DP_TEST_AUDIO_CHANNEL4:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL4\n");
		audio_ch_count = 4;
		break;
	case DP_TEST_AUDIO_CHANNEL5:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL5\n");
		audio_ch_count = 5;
		break;
	case DP_TEST_AUDIO_CHANNEL6:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL6\n");
		audio_ch_count = 6;
		break;
	case DP_TEST_AUDIO_CHANNEL7:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL7\n");
		audio_ch_count = 7;
		break;
	case DP_TEST_AUDIO_CHANNEL8:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_CHANNEL8\n");
		audio_ch_count = 8;
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_AUDIO_CHANNEL_COUNT\n");
		return -EINVAL;
	}
	HISI_FB_INFO("[DP] test_audio_ch_count = %d\n", audio_ch_count);
	aparams->num_channels = audio_ch_count;

	switch (test_audio_smaple_range) {
	case DP_TEST_AUDIO_SAMPLING_RATE_32:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_32\n");
		orig_sample_freq = 12;
		sample_freq = 3;
		freq_id = 0;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_44_1:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_44_1\n");
		orig_sample_freq = 15;
		sample_freq = 0;
		freq_id = 1;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_48:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_48\n");
		orig_sample_freq = 13;
		sample_freq = 2;
		freq_id = 2;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_88_2:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_88_2\n");
		orig_sample_freq = 7;
		sample_freq = 8;
		freq_id = 3;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_96:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_96\n");
		orig_sample_freq = 5;
		sample_freq = 10;
		freq_id = 4;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_176_4:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_176_4\n");
		orig_sample_freq = 3;
		sample_freq = 12;
		freq_id = 5;
		break;
	case DP_TEST_AUDIO_SAMPLING_RATE_192:
		HISI_FB_INFO("[DP] DP_TEST_AUDIO_SAMPLING_RATE_192\n");
		orig_sample_freq = 1;
		sample_freq = 14;
		freq_id = 6;
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_AUDIO_SAMPLING_RATE\n");
		return -EINVAL;
	}
	HISI_FB_INFO("[DP] sample_freq = %d\n", sample_freq);
	HISI_FB_INFO("[DP] orig_sample_freq = %d\n", orig_sample_freq);
	aparams->iec_samp_freq = sample_freq;
	aparams->iec_orig_samp_freq = orig_sample_freq;

	dptx_audio_num_ch_change(dptx);
	dptx_audio_infoframe_sdp_send(dptx);

	return retval;
}

static int handle_test_link_video_pattern(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t misc, pattern, bpc, bpc_map, dynamic_range,
	   dynamic_range_map, color_format, color_format_map,
	   ycbcr_coeff,  ycbcr_coeff_map;
	struct video_params *vparams;
	struct dtd *mdtd;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	retval = 0;

	retval = dptx_read_dpcd(dptx, DP_TEST_PATTERN, &pattern);
	if (retval)
		return retval;
	retval = dptx_read_dpcd(dptx, DP_TEST_MISC, &misc);
	if (retval)
		return retval;

	dynamic_range = (misc & DP_TEST_DYNAMIC_RANGE_MASK)
			>> DP_TEST_DYNAMIC_RANGE_SHIFT;
	switch (dynamic_range) {
	case DP_TEST_DYNAMIC_RANGE_VESA:
		HISI_FB_INFO("[DP] DP_TEST_DYNAMIC_RANGE_VESA\n");
		dynamic_range_map = VESA;
		break;
	case DP_TEST_DYNAMIC_RANGE_CEA:
		HISI_FB_INFO("[DP] DP_TEST_DYNAMIC_RANGE_CEA\n");
		dynamic_range_map = CEA;
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}

	ycbcr_coeff = (misc & DP_TEST_YCBCR_COEFF_MASK)
			>> DP_TEST_YCBCR_COEFF_SHIFT;

	switch (ycbcr_coeff) {
	case DP_TEST_YCBCR_COEFF_ITU601:
		HISI_FB_INFO("[DP] DP_TEST_YCBCR_COEFF_ITU601\n");
		ycbcr_coeff_map = ITU601;
		break;
	case DP_TEST_YCBCR_COEFF_ITU709:
		HISI_FB_INFO("[DP] DP_TEST_YCBCR_COEFF_ITU709:\n");
		ycbcr_coeff_map = ITU709;
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}
	color_format = misc & DP_TEST_COLOR_FORMAT_MASK;

	switch (color_format) {
	case DP_TEST_COLOR_FORMAT_RGB:
		HISI_FB_INFO("[DP] DP_TEST_COLOR_FORMAT_RGB\n");
		color_format_map = RGB;
		break;
	case DP_TEST_COLOR_FORMAT_YCBCR422:
		HISI_FB_INFO("[DP] DP_TEST_COLOR_FORMAT_YCBCR422\n");
		color_format_map = YCBCR422;
		break;
	case DP_TEST_COLOR_FORMAT_YCBCR444:
		HISI_FB_INFO("[DP] DP_TEST_COLOR_FORMAT_YCBCR444\n");
		color_format_map = YCBCR444;
		break;
	default:
		HISI_FB_INFO("[DP] Invalid  DP_TEST_COLOR_FORMAT\n");
		return -EINVAL;
	}

	bpc = (misc & DP_TEST_BIT_DEPTH_MASK)
		>> DP_TEST_BIT_DEPTH_SHIFT;

	switch (bpc) {
	case DP_TEST_BIT_DEPTH_6:
		bpc_map = COLOR_DEPTH_6;
		HISI_FB_INFO("[DP] TEST_BIT_DEPTH_6\n");
		break;
	case DP_TEST_BIT_DEPTH_8:
		bpc_map = COLOR_DEPTH_8;
		HISI_FB_INFO("[DP] TEST_BIT_DEPTH_8\n");
		break;
	case DP_TEST_BIT_DEPTH_10:
		bpc_map = COLOR_DEPTH_10;
		HISI_FB_INFO("[DP] TEST_BIT_DEPTH_10\n");
		break;
	case DP_TEST_BIT_DEPTH_12:
		bpc_map = COLOR_DEPTH_12;
		HISI_FB_INFO("[DP] TEST_BIT_DEPTH_12\n");
		break;
	case DP_TEST_BIT_DEPTH_16:
		bpc_map = COLOR_DEPTH_16;
		HISI_FB_INFO("[DP] TEST_BIT_DEPTH_16\n");
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_BIT_DEPTH\n");
		return -EINVAL;
	}

	vparams->dynamic_range = dynamic_range_map;
	HISI_FB_INFO("[DP] Change video dynamic range to %d\n", dynamic_range_map);

	vparams->colorimetry = ycbcr_coeff_map;
	HISI_FB_INFO("[DP] Change video colorimetry to %d\n", ycbcr_coeff_map);

	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
					 dptx->link.rate,
					 bpc_map, color_format_map,
					 mdtd->pixel_clock);
	if (retval)
		return retval;

	vparams->pix_enc = color_format_map;
	HISI_FB_INFO("[DP] Change pixel encoding to %d\n", color_format_map);

	vparams->bpc = bpc_map;
	dptx_video_bpc_change(dptx);
	HISI_FB_INFO("[DP] Change bits per component to %d\n", bpc_map);
	dptx_video_ts_change(dptx);

	switch (pattern) {
	case DP_TEST_PATTERN_NONE:
		HISI_FB_INFO("[DP] TEST_PATTERN_NONE %d\n", pattern);
		break;
	case DP_TEST_PATTERN_COLOR_RAMPS:
		HISI_FB_INFO("[DP] TEST_PATTERN_COLOR_RAMPS %d\n", pattern);
		vparams->pattern_mode = RAMP;
		HISI_FB_INFO("[DP] Change video pattern to RAMP\n");
		break;
	case DP_TEST_PATTERN_BW_VERITCAL_LINES:
		HISI_FB_INFO("[DP] TEST_PATTERN_BW_VERTICAL_LINES %d\n", pattern);
		break;
	case DP_TEST_PATTERN_COLOR_SQUARE:
		HISI_FB_INFO("[DP] TEST_PATTERN_COLOR_SQUARE %d\n", pattern);
		vparams->pattern_mode = COLRAMP;
		HISI_FB_INFO("[DP] Change video pattern to COLRAMP\n");
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_PATTERN %d\n", pattern);
		return -EINVAL;
	}

	retval = handle_test_link_video_timming(dptx);
	if (retval)
		return retval;

	return 0;
}

static int handle_test_link_phy_pattern(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t pattern;
	retval = 0;
	pattern = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_read_dpcd(dptx, DP_TEST_PHY_PATTERN, &pattern);
	if (retval)
		return retval;

	pattern &= DPTX_PHY_TEST_PATTERN_SEL_MASK;

	switch (pattern) {
	case DPTX_NO_TEST_PATTERN_SEL:
		HISI_FB_INFO("[DP] DPTX_D102_WITHOUT_SCRAMBLING\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);
		break;
	case DPTX_D102_WITHOUT_SCRAMBLING:
		HISI_FB_INFO("[DP] DPTX_D102_WITHOUT_SCRAMBLING\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_1);
		break;
	case DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT:
		HISI_FB_INFO("[DP] DPTX_SYMBOL_ERROR_MEASUREMENT_COUNT\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_SYM_ERM);
		break;
	case DPTX_TEST_PATTERN_PRBS7:
		HISI_FB_INFO("[DP] DPTX_TEST_PATTERN_PRBS7\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_PRBS7);
		break;
	case DPTX_80BIT_CUSTOM_PATTERN_TRANS:
		HISI_FB_INFO("[DP] DPTX_80BIT_CUSTOM_PATTERN_TRANS\n");
		dptx_writel(dptx, DPTX_CUSTOMPAT0, 0x3e0f83e0);
		dptx_writel(dptx, DPTX_CUSTOMPAT1, 0x3e0f83e0);
		dptx_writel(dptx, DPTX_CUSTOMPAT2, 0xf83e0);
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_CUSTOM80);
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN:
		HISI_FB_INFO("[DP] DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_CP2520_1);
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_2:
		HISI_FB_INFO("[DP] DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_CP2520_2);
		break;
	case DPTX_CP2520_HBR2_COMPLIANCE_PATTERN_3:
		HISI_FB_INFO("[DP] DPTX_CP2520_HBR2_COMPLIANCE_EYE_PATTERN\n");
		dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_4);
		break;
	default:
		HISI_FB_INFO("[DP] Invalid TEST_PATTERN\n");
		return -EINVAL;
	}

	return 0;
}

static int handle_automated_test_request(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t test;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_read_dpcd(dptx, DP_TEST_REQUEST, &test);
	if (retval)
		return retval;

	if (test & DP_TEST_LINK_TRAINING) {
		HISI_FB_INFO("[DP] %s: DP_TEST_LINK_TRAINING\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_training(dptx);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_VIDEO_PATTERN) {
		HISI_FB_INFO("[DP] %s:DP_TEST_LINK_VIDEO_PATTERN\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		dptx->hisifd->hpd_release_sub_fnc(dptx->hisifd->fbi);
		dp_send_cable_notification(dptx, Hot_Plug_TEST_OUT);

		retval = handle_test_link_video_pattern(dptx);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_AUDIO_PATTERN) {
		HISI_FB_INFO("[DP] %s:DP_TEST_LINK_AUDIO_PATTERN\n", __func__);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		retval = handle_test_link_audio_pattern(dptx);
		if (retval)
			return retval;
	}

	if (test & DP_TEST_LINK_EDID_READ) {
		/* Invalid, this should happen on HOTPLUG */
		HISI_FB_INFO("[DP] %s:DP_TEST_LINK_EDID_READ\n", __func__);
		return -ENOTSUPP;
	}

	if (test & DP_TEST_LINK_PHY_TEST_PATTERN) {
		/* TODO */
		HISI_FB_INFO("[DP] %s:DP_TEST_LINK_PHY_TEST_PATTERN\n", __func__);

		dptx_triger_media_transfer(dptx, false);

		retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_ACK);
		if (retval)
			return retval;

		dptx_link_set_ssc(dptx, true);

		dptx_link_adjust_drive_settings(dptx, NULL);   //Test only

		retval = handle_test_link_phy_pattern(dptx);
		if (retval)
			return retval;
	}

	return 0;
}

static int handle_sink_request(struct dp_ctrl *dptx)
{
	int retval;
	uint8_t vector;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	retval = dptx_link_check_status(dptx);
	if (retval)
		return retval;

	retval = dptx_read_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &vector);
	if (retval)
		return retval;

	HISI_FB_INFO("[DP] %s: IRQ_VECTOR: 0x%02x\n", __func__, vector);
	dp_imonitor_set_param(DP_PARAM_IRQ_VECTOR, &vector);

	/* TODO handle sink interrupts */
	if (!vector)
		return 0;

	if (vector & DP_REMOTE_CONTROL_COMMAND_PENDING) {
		/* TODO */
		HISI_FB_WARNING(
			  "[DP] %s: DP_REMOTE_CONTROL_COMMAND_PENDING: Not yet implemented",
			  __func__);
	}

	if (vector & DP_AUTOMATED_TEST_REQUEST) {
		HISI_FB_INFO("[DP] %s: DP_AUTOMATED_TEST_REQUEST", __func__);
		retval = handle_automated_test_request(dptx);
		if (retval) {
			HISI_FB_ERR("[DP] Automated test request failed\n");
			if (retval == -ENOTSUPP) {
				retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE,
							 DP_TEST_NAK);
				if (retval)
					return retval;
			}
		}
	}

	if (vector & DP_CP_IRQ) {
		HISI_FB_WARNING("[DP] %s: DP_CP_IRQ", __func__);
		//retval = dptx_write_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, DP_CP_IRQ);
        dp_imonitor_set_param(DP_PARAM_HDCP_VERSION, &g_hdcp_mode);
        if (g_hdcp_mode == 3)   //hdcp1.3
        {
            uint8_t bstatus=0;
            uint32_t temp_value = 0;
            int ret;

            if(dptx->hisifd->secure_ctrl.hdcp_reg_get)
                temp_value = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_OBS);
            //HISI_FB_INFO("DPTX_HDCP_OBS: 0x%x\n", temp_value);

            if ((temp_value & 0xFFFF) == REPEATER_STATE)
            {
                HISI_FB_INFO("[DP] CP_IRQ in REPEATER_STATE!\n");
                ret = dptx_read_dpcd(dptx, 0x68029, &bstatus); //1B-02 need
                if (ret)
                    return ret;
                HISI_FB_INFO("[DP] bstatus is 0x%x\n", bstatus);
                if (bstatus & 0x4)
                {
                    link_error_count ++;
                    HISI_FB_ERR("[DP] link_error %d times!!\n", link_error_count);
                    if(link_error_count >= MAX_LINK_ERROR_COUNT)
                    {
                        HISI_FB_ERR("[DP] re-auth HDCP1.3!\n");
                        link_error_count=0;
                        //dptx_writel(dptx, 0xE00, 0);
                        HDCP13_enable(dptx, 0);
                        msleep(10);
                        //dptx_writel(dptx, 0xE00, 0x106);    //patch for auto re-auth
                        HDCP13_enable(dptx, 1);
                    }
                }
            }
            else if ((temp_value & 0xF0) == 0x40)
            {
                if(dptx->hisifd->secure_ctrl.hdcp_cp_irq)
                    dptx->hisifd->secure_ctrl.hdcp_cp_irq();
            }
        }
        else    //hdcp2.2 ot other
        {
            if(dptx->hisifd->secure_ctrl.hdcp_cp_irq)
				dptx->hisifd->secure_ctrl.hdcp_cp_irq();
        }
		if (retval)
			return retval;
		/* TODO Check Re-authentication Request and Link integrity
		 * Failure bits in Bstatus
		 */
	}

	if (vector & DP_MCCS_IRQ) {
		/* TODO */
		HISI_FB_WARNING(
			  "[DP] %s: DP_MCCS_IRQ: Not yet implemented", __func__);
		retval = -ENOTSUPP;
	}

	if (vector & DP_DOWN_REP_MSG_RDY) {
		/* TODO */
		HISI_FB_WARNING("[DP] %s: DP_DOWN_REP_MSG_RDY: Not yet implemented",
			  __func__);
		retval = -ENOTSUPP;
	}

	if (vector & DP_UP_REQ_MSG_RDY) {
		/* TODO */
		HISI_FB_WARNING("[DP] %s: DP_UP_REQ_MSG_RDY: Not yet implemented",
			  __func__);
		retval = -ENOTSUPP;
	}

	if (vector & DP_SINK_SPECIFIC_IRQ) {
		/* TODO */
		HISI_FB_WARNING("[DP] %s: DP_SINK_SPECIFIC_IRQ: Not yet implemented",
			  __func__);
		retval = -ENOTSUPP;
	}

	return retval;
}
static void dptx_notify(struct dp_ctrl *dptx)
{
	if (!dptx) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return;
	}

	wake_up_interruptible(&dptx->waitq);
}

void dptx_notify_shutdown(struct dp_ctrl *dptx)
{
	if (!dptx) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return;
	}

	atomic_set(&dptx->shutdown, 1);
	wake_up_interruptible(&dptx->waitq);
}

int dptx_triger_media_transfer(struct dp_ctrl *dptx, bool benable)
{
	struct audio_params *aparams;
	struct video_params *vparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	aparams = &(dptx->aparams);
	vparams = &(dptx->vparams);

	if (dptx->video_transfer_enable) {
		if (benable) {
			enable_ldi(dptx->hisifd);
			dptx_en_audio_channel(dptx, aparams->num_channels, 1);
		} else {
			dptx_en_audio_channel(dptx, aparams->num_channels, 0);
			disable_ldi(dptx->hisifd);
			if (vparams->m_fps > 0)
				mdelay(1000 / vparams->m_fps + 10);
			else
				return -1;
		}
	}
	return 0;
}

int handle_hotunplug(struct hisi_fb_data_type *hisifd)
{
	struct dp_ctrl *dptx;
	int ret = 0;

	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] +.\n");

	dptx = &(hisifd->dp);
	dptx->video_transfer_enable = false;
	dptx->max_edid_timing_hactive = 0;

	dptx_cancel_detect_work(dptx);
	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);
	dp_send_cable_notification(dptx, (dptx->dptx_vr) ? Hot_Plug_OUT_VR : Hot_Plug_OUT);

	/*Disable DSS and HWC first*/
	if(hisifd->hpd_release_sub_fnc)
		hisifd->hpd_release_sub_fnc(hisifd->fbi);

	/*Disable DPTX*/
	dptx_disable_default_video_stream(dptx);
	/* Clear xmit enables */
	dptx_phy_enable_xmit(dptx, 4, false);
	/* Power down all lanes */
	/* TODO */
	dptx_phy_set_lanes_status(dptx, false);
	/* Disable AUX Block */
	dptx_aux_disreset(dptx, false);
	release_edid_info(dptx);
	atomic_set(&dptx->sink_request, 0);
	atomic_set(&dptx->aux.event, 0);
	dptx->link.trained = false;

	dp_imonitor_set_param(DP_PARAM_TIME_STOP, NULL);
	HISI_FB_INFO("[DP] -.\n");

	return 0;
}

static int dptx_read_edid_block(struct dp_ctrl *dptx,
	unsigned int block)
{
	int retval;
	uint8_t offset = block * 128;
	uint8_t segment = block >> 1;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	HISI_FB_INFO("[DP] block=%d.\n", block);
	if (segment != 0) {
		retval = dptx_write_bytes_to_i2c(dptx, 0x30, &segment, 1);
		if (retval) {
			HISI_FB_ERR("[DP] failed to  dptx_write_bytes_to_i2c 1!\n");
			return retval;
		}
	}
	/* TODO Skip if no E-DDC */

	retval = dptx_write_bytes_to_i2c(dptx, 0x50, &offset, 1);
	if (retval) {
		HISI_FB_ERR("[DP] failed to  dptx_write_bytes_to_i2c 2!\n");
		return retval;
	}

	retval = dptx_read_bytes_from_i2c(dptx, 0x50,
		&dptx->edid[block * 128], 128);
	if (retval) {
		HISI_FB_ERR("[DP] failed to  dptx_read_bytes_from_i2c 2!\n");
		return retval;
	}
	dp_imonitor_set_param(DP_PARAM_EDID + block, &(dptx->edid[block * DP_DSM_EDID_BLOCK_SIZE]));

	return 0;
}

bool dptx_check_edid_header(struct dp_ctrl *dptx)
{
	int i;
	uint8_t* edid_t;
	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (!(dptx->edid)) {
		HISI_FB_ERR("[DP] edid is NULL!\n");
		return -EINVAL;
	}

	edid_t = dptx->edid;
	for(i = 0; i < EDID_HEADER_END + 1; i++)
	{
		if(edid_t[i] != edid_v1_header[i]) {
			HISI_FB_INFO("[DP] Invalide edid header\n");
			return false;
		}
	}

	return true;
}

static int dptx_read_edid(struct dp_ctrl *dptx)
{
	int i;
	int retval = 0;
	unsigned int ext_blocks = 0;
	uint8_t *first_edid_block;
	unsigned int edid_buf_size = 0;
	int edid_try_count = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (!(dptx->edid)) {
		HISI_FB_ERR("[DP] edid is NULL!\n");
		return -EINVAL;
	}

edid_retry:
	memset(dptx->edid, 0, DPTX_DEFAULT_EDID_BUFLEN);
	retval = dptx_read_edid_block(dptx, 0);
	/*will try to read edid block again when ready edid block failed*/
	if(retval) {
		if(edid_try_count <= MAX_AUX_RETRY_COUNT) {
			HISI_FB_INFO("[DP] Read edid block failed, try %d times \n", edid_try_count);
			mdelay(AUX_RETRY_DELAY_TIME);
			edid_try_count += 1;
			goto edid_retry;
		}else{
			HISI_FB_ERR("[DP] failed to dptx_read_edid_block!\n");
			edid_try_count = 0;
			return -EINVAL;
		}
	}

	ext_blocks = dptx->edid[126];
	if((ext_blocks > MAX_EXT_BLOCKS) || !dptx_check_edid_header(dptx)){
		edid_try_count += 1;
		if(edid_try_count <= MAX_AUX_RETRY_COUNT) {
			mdelay(AUX_RETRY_DELAY_TIME);
			HISI_FB_INFO("[DP] Read edid data is not correct, try %d times \n", edid_try_count);
			goto edid_retry;
		}else{
			edid_try_count = 0;
			if(ext_blocks > MAX_EXT_BLOCKS)
				ext_blocks = MAX_EXT_BLOCKS;
		}
	}

	first_edid_block = kmalloc(128, GFP_KERNEL);
	if (first_edid_block == NULL) {
		HISI_FB_ERR("[DP] Allocate buffer error\n");
		return -EINVAL;
	}
	memcpy(first_edid_block, dptx->edid, 128);

	if (dptx->edid) {
		kfree(dptx->edid);
		dptx->edid = NULL;
	}

	dptx->edid = kzalloc(128 * ext_blocks + 128, GFP_KERNEL);
	if (!dptx->edid) {
		HISI_FB_ERR("[DP] Allocate edid buffer error!\n");
		return -EINVAL;
	}

	memcpy(dptx->edid, first_edid_block, 128);
	for (i = 1; i <= ext_blocks; i++) {
		retval = dptx_read_edid_block(dptx, i);
		if (retval) {
			goto fail;
		}
	}

	edid_buf_size = 128 * ext_blocks + 128;
	retval = edid_buf_size;

fail:
	if (first_edid_block) {
		kfree(first_edid_block);
		first_edid_block = NULL;
	}

	return retval;
}
/*
static int dptx_check_edid(struct dp_ctrl *dptx)
{
	int i;
	uint32_t edid_sum = 0;

	if (dptx == NULL) {
		HISI_FB_ERR("NULL Pointer\n");
		return -EINVAL;
	}

	if (!(dptx->edid)) {
		HISI_FB_ERR("edid is NULL!\n");
		return -EINVAL;
	}

	for (i = 0; i < 128; i++)
		edid_sum += dptx->edid[i];

	if (edid_sum & 0xFF) {
		HISI_FB_ERR("Invalid EDID checksum\n");
		return -EINVAL;
	}

	return 0;
}
*/

int dptx_choose_edid_timing(struct dp_ctrl *dptx, bool *bsafemode)
{
	struct timing_info *per_timing_info;
	struct timing_info *dptx_timing_node, *_node_;
	struct dtd *mdtd;

	uint32_t default_hactive;
	uint64_t default_pixel_clock;

	mdtd = &dptx->vparams.mdtd;
	per_timing_info = dptx_timing_node = _node_ = NULL;

	if (dptx->edid_info.Video.dptx_timing_list != NULL) {
		default_hactive = SAFE_MODE_TIMING_HACTIVE;
		default_pixel_clock = SAFE_MODE_TIMING_PIXEL_CLOCK;
		list_for_each_entry_safe(dptx_timing_node, _node_, dptx->edid_info.Video.dptx_timing_list, list_node) {
			if ((dptx_timing_node->hActivePixels >= default_hactive) &&
				(dptx_timing_node->pixelClock > default_pixel_clock) &&
				(dptx_timing_node->interlaced != 1) &&
				(dptx_timing_node->vBlanking <= VBLANKING_MAX) &&
				(!dptx_video_ts_calculate(dptx, dptx->link.lanes,
						 dptx->link.rate, dptx->vparams.bpc,
						 dptx->vparams.pix_enc, (dptx_timing_node->pixelClock * 10)))) {
					default_hactive = dptx_timing_node->hActivePixels;
					default_pixel_clock = dptx_timing_node->pixelClock;
					per_timing_info = dptx_timing_node;
				}
		}

		if ((SAFE_MODE_TIMING_HACTIVE == default_hactive) || (per_timing_info == NULL)) {
			*bsafemode = true;
			return 0;
		}

		mdtd->pixel_repetition_input = 0;
		mdtd->pixel_clock = per_timing_info->pixelClock;

		mdtd->h_active = per_timing_info->hActivePixels;
		mdtd->h_blanking = per_timing_info->hBlanking;
		mdtd->h_sync_offset = per_timing_info->hSyncOffset;
		mdtd->h_sync_pulse_width = per_timing_info->hSyncPulseWidth;

		mdtd->h_image_size = per_timing_info->hSize;

		mdtd->v_active = per_timing_info->vActivePixels;
		mdtd->v_blanking = per_timing_info->vBlanking;
		mdtd->v_sync_offset = per_timing_info->vSyncOffset;
		mdtd->v_sync_pulse_width = per_timing_info->vSyncPulseWidth;

		mdtd->v_image_size = per_timing_info->vSize;

		mdtd->interlaced = per_timing_info->interlaced;

		mdtd->v_sync_polarity = per_timing_info->vSyncPolarity;
		mdtd->h_sync_polarity = per_timing_info->hSyncPolarity;

		mdtd->pixel_clock *= 10;
		if (mdtd->interlaced == 1)
			mdtd->v_active /= 2;

		dptx->max_edid_timing_hactive = per_timing_info->hActivePixels;

		HISI_FB_INFO("[DP] The choosed DTD: pixel_clock is %llu, interlaced is %d, h_active is %d, v_active is %d\n",
				mdtd->pixel_clock, mdtd->interlaced, mdtd->h_active, mdtd->v_active);
		HISI_FB_DEBUG("[DP] DTD pixel_clock: %llu interlaced: %d\n",
			 mdtd->pixel_clock, mdtd->interlaced);
		HISI_FB_DEBUG("[DP] h_active: %d h_blanking: %d h_sync_offset: %d\n",
			 mdtd->h_active, mdtd->h_blanking, mdtd->h_sync_offset);
		HISI_FB_DEBUG("[DP] h_sync_pulse_width: %d h_image_size: %d h_sync_polarity: %d\n",
			 mdtd->h_sync_pulse_width, mdtd->h_image_size,
			 mdtd->h_sync_polarity);
		HISI_FB_DEBUG("[DP] v_active: %d v_blanking: %d v_sync_offset: %d\n",
			 mdtd->v_active, mdtd->v_blanking, mdtd->v_sync_offset);
		HISI_FB_DEBUG("[DP] v_sync_pulse_width: %d v_image_size: %d v_sync_polarity: %d\n",
			 mdtd->v_sync_pulse_width, mdtd->v_image_size,
			 mdtd->v_sync_polarity);

		dp_imonitor_set_param(DP_PARAM_MAX_WIDTH,   &(mdtd->h_active));
		dp_imonitor_set_param(DP_PARAM_MAX_HIGH,    &(mdtd->v_active));
		dp_imonitor_set_param(DP_PARAM_PIXEL_CLOCK, &(mdtd->pixel_clock));
		*bsafemode = false;
	} else {
		*bsafemode = true;
	}

	return 0;
}

int handle_hotplug(struct hisi_fb_data_type *hisifd)
{
	uint8_t rev = 0;
	int retval;
	uint8_t vector;
	uint8_t checksum = 0;
	uint8_t blocks = 0;
	//uint8_t preferred_vic[18] = {0};
	uint8_t test = 0;
	uint32_t edid_info_size = 0;
	struct video_params *vparams;
	struct hdcp_params *hparams;
	struct dtd mdtd;
	struct dp_ctrl *dptx;
	char *monitor_name_info;
	bool bsafe_mode;

	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("+.\n");

	dptx = &(hisifd->dp);

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return -EINVAL;
	}

	if (!dptx->dptx_enable) {
		HISI_FB_ERR("[DP] dptx has already off.\n");
		return -EINVAL;
	}

	dp_imonitor_set_param(DP_PARAM_TIME_START, NULL);
	bsafe_mode = false;
	vparams = &dptx->vparams;
	hparams = &dptx->hparams;

	dptx_video_params_reset(&dptx->vparams);
	dptx_audio_params_reset(&dptx->aparams);
	dptx_video_config(dptx);

	dptx_soft_reset(dptx,
		DPTX_SRST_CTRL_PHY | DPTX_SRST_CTRL_HDCP | DPTX_SRST_CTRL_AUX);

	/* Enable AUX Block */
	dptx_aux_disreset(dptx, true);
	dptx_core_init_phy(dptx);

	retval = dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D0);
	if (retval) {
		HISI_FB_ERR("[DP] failed to  dptx_write_dpcd DP_SET_POWER, DP_SET_POWER_D0 %d", retval);
		return retval;
	}
	mdelay(1);

	retval = dptx_read_dpcd(dptx, DP_DEVICE_SERVICE_IRQ_VECTOR, &vector);
	if (retval) {
		HISI_FB_ERR("[DP] failed to  dptx_read_dpcd DP_DEVICE_SERVICE_IRQ_VECTOR, retval=%d.", retval);
		return retval;
	}

	retval = dptx_read_edid(dptx);
	if (retval < 128) {
		HISI_FB_ERR("[DP] failed to  dptx_read_edid, retval=%d.", retval);
		dp_imonitor_set_param(DP_PARAM_READ_EDID_FAILED, &retval);
		edid_info_size = 0;
		bsafe_mode = true;
	} else {
		edid_info_size = retval;
	}
/*
	retval = dptx_check_edid(dptx);
	if (retval) {
		vparams->video_format = VCEA;
		dptx_dtd_fill(&mdtd, 1, vparams->refresh_rate,
			      vparams->video_format);
	} else {
		memcpy(preferred_vic, dptx->edid + 0x36, 0x12);
		retval = dptx_dtd_parse(dptx, &mdtd, preferred_vic);
		if (retval) {
			vparams->video_format = VCEA;
			dptx_dtd_fill(&mdtd, 1, vparams->refresh_rate,
				      vparams->video_format);
		}
	}
*/
	if (g_fpga_flag == 1) {
		retval = -1;
	} else {
		retval = parse_edid(dptx, edid_info_size);
	}

	if (retval) {
		HISI_FB_ERR("[DP] EDID Parser fail, display safe mode\n");
		bsafe_mode = true;
	}

	if(dptx->edid_info.Video.dp_monitor_descriptor != NULL) {
		monitor_name_info = dptx->edid_info.Video.dp_monitor_descriptor;
		if(!(strncmp("HUAWEIAV02", monitor_name_info, strlen("HUAWEIAV02")))) {
			dptx->dptx_vr = true;
			HISI_FB_INFO("[DP] The display is VR.\n");
		}
	}

	/* Link Train start */
	retval = dptx_read_dpcd(dptx, DP_DPCD_REV, &rev);
	if (retval) {
		/* Abort bringup */
		/* Reset core and try again */
		/* Abort all aux, and other work, reset the core */
		HISI_FB_ERR("[DP] failed to dptx_read_dpcd DP_DPCD_REV, retval=%d.\n", retval);
		return retval;
	}
	HISI_FB_DEBUG("[DP] DP Revision %x.%x .\n", (rev & 0xf0) >> 4, rev & 0xf);

	memset(dptx->rx_caps, 0, DPTX_RECEIVER_CAP_SIZE);
	retval = dptx_read_bytes_from_dpcd(dptx, DP_DPCD_REV,
		dptx->rx_caps, DPTX_RECEIVER_CAP_SIZE);
	if (retval) {
		HISI_FB_ERR("[DP] failed to dptx_read_bytes_from_dpcd DP_DPCD_REV, retval=%d.\n", retval);
		return retval;
	}
	dp_imonitor_set_param(DP_PARAM_DPCD_RX_CAPS, dptx->rx_caps);

	/*
	* The TEST_EDID_READ is asserted on HOTPLUG. Check for it and
	* handle it here.
	*/
	if (vector & DP_AUTOMATED_TEST_REQUEST) {
		HISI_FB_INFO("[DP] DP_AUTOMATED_TEST_REQUEST");
		retval = dptx_read_dpcd(dptx, DP_TEST_REQUEST, &test);
		if (retval) {
			HISI_FB_ERR("[DP] failed to dptx_read_dpcd DP_TEST_REQUEST, retval=%d.\n", retval);
			return retval;
		}

		if (test & DP_TEST_LINK_EDID_READ) {
			blocks = dptx->edid[126];
			checksum = dptx->edid[127 + 128 * blocks];

			retval = dptx_write_dpcd(dptx, DP_TEST_EDID_CHECKSUM, checksum);
			if (retval) {
				HISI_FB_ERR("[DP] failed to dptx_write_dpcd DP_TEST_EDID_CHECKSUM, retval=%d.\n", retval);
				return retval;
			}

			retval = dptx_write_dpcd(dptx, DP_TEST_RESPONSE, DP_TEST_EDID_CHECKSUM_WRITE);
			if (retval) {
				HISI_FB_ERR("[DP] failed to dptx_write_dpcd DP_TEST_RESPONSE, retval=%d.\n", retval);
				return retval;
			}
		}
	}

	/* TODO No other IRQ should be set on hotplug */
	retval = dptx_link_training(dptx, dptx->max_rate, dptx->max_lanes);
	if (retval) {
		HISI_FB_ERR("[DP] failed to  dptx_link_training, retval=%d.\n", retval);
		dp_imonitor_set_param(DP_PARAM_LINK_TRAINING_FAILED, &retval);
		return retval;
	}

	if (!bsafe_mode) {
		dptx_choose_edid_timing(dptx, &bsafe_mode);
	}

	if (bsafe_mode) {
		dp_imonitor_set_param(DP_PARAM_SAFE_MODE, &bsafe_mode);
		if (edid_info_size) {
			uint8_t code = 1; // resolution: 640*480
			vparams->video_format = VCEA;
			dp_imonitor_set_param_resolution(&code, &(vparams->video_format));
			dptx_dtd_fill(&mdtd, code, vparams->refresh_rate, vparams->video_format);
		} else {
			vparams->video_format = DMT;
			dptx_dtd_fill(&mdtd, 16, vparams->refresh_rate, vparams->video_format); /*If edid can't be got, DP transfer 1024*768 firstly!*/

			retval = dptx_read_dpcd(dptx, DP_DOWNSTREAMPORT_PRESENT, &rev);
			if (retval) {
				HISI_FB_ERR("[DP] failed to dptx_read_dpcd DP_DOWNSTREAMPORT_PRESENT, retval=%d.\n", retval);
				return retval;
			}

			if (((rev & DP_DWN_STRM_PORT_TYPE_MASK) >> 1) != 0x01) {
				dptx->edid_info.Audio.basicAudio = 0x1;
				HISI_FB_INFO("[DP] If DFP port don't belong to analog(VGA/DVI-I), update audio capabilty.\n");
				dp_imonitor_set_param(DP_PARAM_BASIC_AUDIO, &(dptx->edid_info.Audio.basicAudio));
			}
		}
		memcpy(&(dptx->vparams.mdtd), &mdtd, sizeof(mdtd));
	}

	retval = dptx_video_ts_calculate(dptx, dptx->link.lanes,
		dptx->link.rate, vparams->bpc, vparams->pix_enc, vparams->mdtd.pixel_clock);
	if (retval) {
		HISI_FB_INFO("[DP] Can't change to the preferred video mode: frequency = %llu\n",
						vparams->mdtd.pixel_clock);
	} else {
		//vparams->mdtd = mdtd;
		HISI_FB_DEBUG("[DP] pixel_frequency=%llu.\n", vparams->mdtd.pixel_clock);

		/* MMCM */
	}

	/*DP update device to HWC and configue DSS*/
	if (dptx->dptx_vr) {
		if (dptx_check_low_temperature(dptx)) {
			HISI_FB_ERR("[DP] VR device can't work on low temperature!\n");
			return 0;
		}

		retval = dptx_resolution_switch(hisifd, Hot_Plug_IN_VR);
		if (retval) {
			HISI_FB_ERR("[DP] Hot_Plug_IN_VR DSS init fail !!!\n");
		}
	} else {
		retval = dptx_change_video_mode_user(dptx);
		if (retval) {
			HISI_FB_ERR("[DP] Change mode by user setting error!\n");
		}

		retval = dptx_resolution_switch(hisifd, Hot_Plug_IN);
		if (retval) {
			HISI_FB_ERR("[DP] Hot_Plug_IN DSS init fail !!!\n");
		}
	}

	// for factory test
	if (dp_factory_mode_is_enable()) {
		if (!dp_factory_is_4k_60fps(dptx->max_rate, dptx->max_lanes,
			dptx->vparams.mdtd.h_active, dptx->vparams.mdtd.v_active, dptx->vparams.m_fps)) {
			HISI_FB_ERR("[DP] not support hotplug when not 4k@60fps in factory mode!\n");
			return -ECONNREFUSED;
		}
	}

	/*Update DP reg configue*/
	dptx_video_timing_change(dptx);	/*dptx video reg depends on dss pixel clock.*/
	dptx_audio_config(dptx);	/*dptx audio reg depends on phy status(P0)*/

	dptx->current_link_rate = dptx->link.rate;
	dptx->current_link_lanes= dptx->link.lanes;

	if (dptx->dptx_vr) {
		dptx_init_detect_work(dptx);
	}

	hparams->auth_fail_count = 0;

	HISI_FB_INFO("[DP] -.\n");

	return 0;
}

static void handle_hdcp22_gpio_intr(struct dp_ctrl *dptx)
{
	uint32_t hdcpobs;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	//hdcpobs = dptx_readl(dptx, DPTX_HDCP_OBS);
	if(dptx->hisifd->secure_ctrl.hdcp_reg_get)
		hdcpobs = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_OBS);
	else
		HISI_FB_ERR("[HDCP] ATF:hdcp_reg_get is NULL\n");
	HISI_FB_DEBUG("[HDCP22] DPTX_HDCP_OBS = 0x%x.\n", hdcpobs);

	if (hdcpobs & DPTX_HDCP22_BOOTED)
		HISI_FB_DEBUG("[HDCP22] ESM has booted.\n");

	if (hdcpobs & DPTX_HDCP22_CAP_CHECK_COMPLETE) {
		if (hdcpobs & DPTX_HDCP22_CAPABLE_SINK)
			HISI_FB_DEBUG("[HDCP22] sink is HDCP22 capable\n");
		else
			HISI_FB_NOTICE("[HDCP22] sink is not HDCP22 capable\n");
	}

	if (hdcpobs & DPTX_HDCP22_AUTH_SUCCESS) {
		HISI_FB_NOTICE("[HDCP22] the authentication is succeded.\n");
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_S, NULL);
	}

	if (hdcpobs & DPTX_HDCP22_AUTH_FAILED) {
		HISI_FB_NOTICE("[HDCP22] the authentication is failed.\n");
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_F, NULL);
	}

	if (hdcpobs & DPTX_HDCP22_RE_AUTH_REQ)
		HISI_FB_WARNING("[HDCP22] the sink has requested a re-authentication.\n");

	//dptx_writel(dptx, DPTX_HDCP_OBS, hdcpobs);
	if(dptx->hisifd->secure_ctrl.hdcp_obs_set)
		dptx->hisifd->secure_ctrl.hdcp_obs_set(hdcpobs);
	else
		HISI_FB_ERR("[HDCP] ATF:hdcp_obs_set is NULL\n");
}

static void handle_hdcp_intr(struct dp_ctrl *dptx)
{
	uint32_t hdcpintsts;
	uint32_t hdcpobs;
	struct hdcp_params *hparams;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	hparams = &dptx->hparams;

	//hdcpintsts = dptx_readl(dptx, DPTX_HDCP_INT_STS);
	if(dptx->hisifd->secure_ctrl.hdcp_reg_get)
		hdcpintsts = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_INT_STS);
	else
		HISI_FB_ERR("[HDCP] ATF:hdcp_reg_get is NULL\n");
	HISI_FB_DEBUG("DPTX_HDCP_INT_STS=0x%08x.\n", hdcpintsts);

	if(dptx->hisifd->secure_ctrl.hdcp_reg_get)
		hdcpobs = dptx->hisifd->secure_ctrl.hdcp_reg_get(DPTX_HDCP_OBS);
	else
		HISI_FB_ERR("[HDCP] ATF:hdcp_reg_get is NULL\n");
	HISI_FB_DEBUG("[HDCP] DPTX_HDCP_OBS = 0x%x.\n", hdcpobs);

	if (hdcpintsts & DPTX_HDCP_KSV_ACCESS_INT)
		HISI_FB_DEBUG("KSV memory access guaranteed for read, write access\n");

	if (hdcpintsts & DPTX_HDCP_KSV_SHA1)
	{
	    HISI_FB_DEBUG("SHA1 verification has been done\n");
	}

	if (hdcpintsts & DPTX_HDCP_AUX_RESP_TIMEOUT) {
		HDCP_Stop_Polling_task(1);
		HDCP13_enable(dptx, 0);
		if(dptx->hisifd->secure_ctrl.hdcp_enc_mode)
			dptx->hisifd->secure_ctrl.hdcp_enc_mode(1);
		HISI_FB_WARNING("DPTX_HDCP_AUX_RESP_TIMEOUT\n");
	}

	if (hdcpintsts & DPTX_HDCP_FAILED) {
		if(!(hdcpobs & BIT(18)))    //TE is receiver, not repeater
			HDCP_Stop_Polling_task(1);
		hparams->auth_fail_count++;
		if (hparams->auth_fail_count > DPTX_HDCP_MAX_AUTH_RETRY) {
			HDCP13_enable(dptx, 0);
			if(dptx->hisifd->secure_ctrl.hdcp_enc_mode)
				dptx->hisifd->secure_ctrl.hdcp_enc_mode(1);
			HISI_FB_ERR("Reach max allowed retries count=%d.\n", hparams->auth_fail_count);
		}
		else
		{
		    link_error_count = MAX_LINK_ERROR_COUNT;   //flag to re-auth
		    HISI_FB_INFO("HDCP authentication process was failed!\n");
			dp_imonitor_set_param(DP_PARAM_HDCP_KEY_F, NULL);
		}
	}

	if (hdcpintsts & DPTX_HDCP_ENGAGED) {
		hparams->auth_fail_count = 0;
		HDCP_Stop_Polling_task(1);
		HISI_FB_NOTICE("HDCP authentication process was successful.\n");
		dp_imonitor_set_param(DP_PARAM_HDCP_KEY_S, NULL);
	}

	if (hdcpintsts & DPTX_HDCP22_GPIOINT) {
		HISI_FB_DEBUG("HDCP22_GPIOINT\n");
		handle_hdcp22_gpio_intr(dptx);
	}

	//dptx_writel(dptx, DPTX_HDCP_INT_CLR, hdcpintsts);
	if(dptx->hisifd->secure_ctrl.hdcp_int_clr)
		dptx->hisifd->secure_ctrl.hdcp_int_clr(hdcpintsts);
	else
		HISI_FB_ERR("[HDCP] ATF:hdcp_int_clr is NULL\n");
}

static void handle_aux_reply(struct dp_ctrl *dptx)
{
	uint32_t auxsts;
	uint32_t status;
	uint32_t auxm;
	uint32_t br;

	if (dptx == NULL) {
		HISI_FB_ERR("[DP] NULL Pointer\n");
		return;
	}

	auxsts = dptx_readl(dptx, DPTX_AUX_STS);

	status = (auxsts & DPTX_AUX_STS_STATUS_MASK) >> DPTX_AUX_STS_STATUS_SHIFT;
	auxm = (auxsts & DPTX_AUX_STS_AUXM_MASK) >> DPTX_AUX_STS_AUXM_SHIFT;
	br = (auxsts & DPTX_AUX_STS_BYTES_READ_MASK) >> DPTX_AUX_STS_BYTES_READ_SHIFT;

	HISI_FB_DEBUG("[DP] DPTX_AUX_STS=0x%08x: sts=%d, auxm=%d, br=%d, "
		"replyrcvd=%d, replyerr=%d, timeout=%d, disconn=%d.\n",
		auxsts, status, auxm, br,
		!!(auxsts & DPTX_AUX_STS_REPLY_RECEIVED),
		!!(auxsts & DPTX_AUX_STS_REPLY_ERR),
		!!(auxsts & DPTX_AUX_STS_TIMEOUT),
		!!(auxsts & DPTX_AUX_STS_SINK_DWA));

	switch (status) {
	case DPTX_AUX_STS_STATUS_ACK:
		HISI_FB_DEBUG("[DP] DPTX_AUX_STS_STATUS_ACK!\n");
		break;
	case DPTX_AUX_STS_STATUS_NACK:
		HISI_FB_DEBUG("[DP] DPTX_AUX_STS_STATUS_NACK!\n");
		break;
	case DPTX_AUX_STS_STATUS_DEFER:
		HISI_FB_DEBUG("[DP] DPTX_AUX_STS_STATUS_DEFER!\n");
		break;
	case DPTX_AUX_STS_STATUS_I2C_NACK:
		HISI_FB_DEBUG("[DP] DPTX_AUX_STS_STATUS_I2C_NACK!\n");
		break;
	case DPTX_AUX_STS_STATUS_I2C_DEFER:
		HISI_FB_DEBUG("[DP] DPTX_AUX_STS_STATUS_I2C_DEFER!\n");
		break;
	default:
		HISI_FB_ERR("[DP] Invalid AUX status 0x%x.\n", status);
		break;
	}

	dptx->aux.data[0] = dptx_readl(dptx, DPTX_AUX_DATA0);
	dptx->aux.data[1] = dptx_readl(dptx, DPTX_AUX_DATA1);
	dptx->aux.data[2] = dptx_readl(dptx, DPTX_AUX_DATA2);
	dptx->aux.data[3] = dptx_readl(dptx, DPTX_AUX_DATA3);
	dptx->aux.sts = auxsts;

	atomic_set(&dptx->aux.event, 1);
	dptx_notify(dptx);
}

irqreturn_t dptx_threaded_irq(int irq, void *dev)
{
	int retval = 0;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx;
	uint32_t hpdsts;

	hisifd = dev;
	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return IRQ_HANDLED;
	}

	dptx = &(hisifd->dp);

	mutex_lock(&dptx->dptx_mutex);
	if (!dptx->dptx_enable) {
		HISI_FB_WARNING("[DP] dptx has already off!\n");
		mutex_unlock(&dptx->dptx_mutex);
		return IRQ_HANDLED;
	}

	g_bit_hpd_status = DPTX_HPDSTS_STATUS_GA;

	/*
	 * TODO this should be set after all AUX transactions that are
	 * queued are aborted. Currently we don't queue AUX and AUX is
	 * only started from this function.
	 */
	atomic_set(&dptx->aux.abort, 0);
	if (atomic_read(&dptx->c_connect)) {
		atomic_set(&dptx->c_connect, 0);

		hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
		HISI_FB_INFO("[DP] HPDSTS = 0x%08x.\n", hpdsts);

		if (hpdsts & g_bit_hpd_status) {
			retval = handle_hotplug(hisifd);
			dp_imonitor_set_param(DP_PARAM_HOTPLUG_RETVAL, &retval);
			if (retval) {
				HISI_FB_ERR("[DP] DP Device Hotplug error %d\n", retval);
			}
		} else {
			retval = handle_hotunplug(hisifd);
			if (retval) {
				HISI_FB_ERR("[DP] DP Device Hotplug error %d\n", retval);
			}
		}
	}

	if (atomic_read(&dptx->sink_request)) {
		atomic_set(&dptx->sink_request, 0);
		retval = handle_sink_request(dptx);
		if (retval) {
			HISI_FB_ERR("[DP] Unable to handle sink request %d\n", retval);
		}
	}

	mutex_unlock(&dptx->dptx_mutex);

	return IRQ_HANDLED;
}

void dptx_hpd_handler(struct dp_ctrl *dptx, bool plugin, uint8_t dp_lanes)
{
	uint32_t reg = 0;

	if (!dptx) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return;
	}

	HISI_FB_INFO("[DP] DP Plug Type:(%d), Lanes:(%d)\n", plugin, dp_lanes);
	/* need to  check dp lanes */

	dptx->max_lanes = dp_lanes;

	reg = dptx_readl(dptx, DPTX_CCTL);
	if (plugin) {
		reg |= DPTX_CCTL_FORCE_HPD;
	} else {
		reg &= ~DPTX_CCTL_FORCE_HPD;
	}

	dptx_writel(dptx, DPTX_CCTL, reg);
}

void dptx_hpd_irq_handler(struct dp_ctrl *dptx)
{
	int retval = 0;
	if (!dptx) {
		HISI_FB_ERR("[DP] dptx is NULL!\n");
		return;
	}

	HISI_FB_INFO("[DP] DP Shot Plug!\n");

	if (!dptx->video_transfer_enable) {
		HISI_FB_ERR("[DP] DP never link train success, shot plug is useless!\n");
		return;
	}

	dptx_notify(dptx);
	retval = handle_sink_request(dptx);
	if (retval) {
		HISI_FB_ERR("[DP] Unable to handle sink request %d\n", retval);
	}
}

irqreturn_t dptx_irq(int irq, void *dev)
{
	irqreturn_t retval = IRQ_HANDLED;
	struct hisi_fb_data_type *hisifd;
	struct dp_ctrl *dptx = NULL;
	uint32_t ists;
	uint32_t ien;
	uint32_t hpdsts;

	hisifd = (struct hisi_fb_data_type *)dev;
	if (!hisifd) {
		HISI_FB_ERR("[DP] hisifd is NULL!\n");
		return IRQ_HANDLED;
	}

	dptx = &(hisifd->dp);

	ists = dptx_readl(dptx, DPTX_ISTS);
	ien = dptx_readl(dptx, DPTX_IEN);
	hpdsts = dptx_readl(dptx, DPTX_HPDSTS);
	HISI_FB_DEBUG("[DP] DPTX_ISTS=0x%08x, DPTX_IEN=0x%08x, DPTX_HPDSTS=0x%08x.\n",
		ists, ien, hpdsts);

	if (!(ists & DPTX_ISTS_ALL_INTR)) {
		HISI_FB_INFO("[DP] IRQ_NONE, DPTX_ISTS=0x%08x.\n", ists);
		retval = IRQ_NONE;
		return retval;
	}

	if (ists & DPTX_ISTS_AUX_REPLY) {
		ists &= ~DPTX_ISTS_AUX_REPLY;
		handle_aux_reply(dptx);
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUX_REPLY);
	}

	if (ists & DPTX_ISTS_AUX_CMD_INVALID) {
		/* TODO abort AUX */
		/* handle_aux_reply(dptx); */
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUX_CMD_INVALID);
	}

	if (ists & DPTX_ISTS_HDCP) {
		handle_hdcp_intr(dptx);
		dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_HDCP);
	}

	if (ists & DPTX_ISTS_SDP) {
		/* TODO Handle and clear */
	}

	if (ists & DPTX_ISTS_AUDIO_FIFO_OVERFLOW) {
		if (ien & DPTX_IEN_AUDIO_FIFO_OVERFLOW) {
			HISI_FB_INFO("[DP] DPTX_ISTS_AUDIO_FIFO_OVERFLOW!\n");
			dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_AUDIO_FIFO_OVERFLOW);
		}
	}

	if (ists & DPTX_ISTS_VIDEO_FIFO_OVERFLOW) {
		if (ien & DPTX_IEN_VIDEO_FIFO_OVERFLOW) {
			HISI_FB_ERR("[DP] DPTX_ISTS_VIDEO_FIFO_OVERFLOW!\n");
			//dptx_triger_media_transfer(dptx, false);
			dptx_writel(dptx, DPTX_ISTS, DPTX_ISTS_VIDEO_FIFO_OVERFLOW);
		}
	}

	if (ists & DPTX_ISTS_TYPE_C) {
		/* TODO Handle and clear */
		dptx_writel(dptx, DPTX_TYPE_C_CTRL, DPTX_TYPEC_INTERRUPT_STATUS);

		HISI_FB_DEBUG("\n [DP] DPTX_TYPE_C_CTRL: [%02x] PRE", dptx_readl(dptx, DPTX_TYPE_C_CTRL));
		dptx_typec_reset_ack(dptx);
		HISI_FB_DEBUG("\n [DP] DPTX_TYPE_C_CTRL: [%02x] AFT", dptx_readl(dptx, DPTX_TYPE_C_CTRL));
	}

	if (ists & DPTX_ISTS_HPD) {
		if (hpdsts & DPTX_HPDSTS_IRQ) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_IRQ);
			atomic_set(&dptx->sink_request, 1);
			dptx_notify(dptx);
			retval = IRQ_WAKE_THREAD;
		}

		if (hpdsts & DPTX_HPDSTS_HOT_PLUG) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_HOT_PLUG);

			/* if (hpdsts & DPTX_HPDSTS_STATUS) { */
			if (1) {
				atomic_set(&dptx->aux.abort, 1);
				atomic_set(&dptx->c_connect, 1);
				dptx_notify(dptx);
				retval = IRQ_WAKE_THREAD;
			} else {
				HISI_FB_INFO("[DP] Hot plug - not connected\n");
			}
		}

		if (hpdsts & DPTX_HPDSTS_HOT_UNPLUG) {
			dptx_writel(dptx, DPTX_HPDSTS, DPTX_HPDSTS_HOT_UNPLUG);

			/* if (!(hpdsts & DPTX_HPDSTS_STATUS)) { */
			if (1) {
				atomic_set(&dptx->aux.abort, 1);
				atomic_set(&dptx->c_connect, 1);
				dptx_notify(dptx);
				retval = IRQ_WAKE_THREAD;
			} else {
				HISI_FB_INFO("[DP] Hot unplug - not disconnected\n");
			}
		}

		if (hpdsts & 0x80) {
			HISI_FB_INFO("[DP] DPTX_HPDSTS[7] HOTPLUG DEBUG INTERRUPT!\n");
			dptx_writel(dptx, DPTX_HPDSTS, 0x80 | DPTX_HPDSTS_HOT_UNPLUG);
		}
	}

	return retval;
}
/*lint -restore*/
