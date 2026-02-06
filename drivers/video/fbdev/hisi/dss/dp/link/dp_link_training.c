/*
 * Copyright (c) 2016 Synopsys, Inc.
 *
 * Synopsys DP TX Linux Software Driver and documentation (hereinafter,
 * "Software") is an Unsupported proprietary work of Synopsys, Inc. unless
 * otherwise expressly agreed to in writing between Synopsys and you.
 *
 * The Software IS NOT an item of Licensed Software or Licensed Product under
 * any End User Software License Agreement or Agreement for Licensed Product
 * with Synopsys or any supplement thereto. You are permitted to use and
 * redistribute this Software in source and binary forms, with or without
 * modification, provided that redistributions of source code must retain this
 * notice. You may not view, use, disclose, copy or distribute this file or
 * any information contained herein except pursuant to this license grant from
 * Synopsys. If you do not agree with this notice, including the disclaimer
 * below, then you are not authorized to use the Software.
 *
 * THIS SOFTWARE IS BEING DISTRIBUTED BY SYNOPSYS SOLELY ON AN "AS IS" BASIS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE HEREBY DISCLAIMED. IN NO EVENT SHALL SYNOPSYS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#include "dp_link_training.h"
#include "../controller/dp_core_interface.h"
#include "dp_link_layer_interface.h"
#include "dp_irq.h"
#include "hisi_dp.h"
#include "hisi_fb.h"
#include "hisi_fb_def.h"
#include "dp_dsc_algorithm.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

/*lint -save -e* */
static int dptx_link_read_status(struct dp_ctrl *dptx)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	return dptx_read_bytes_from_dpcd(dptx, DP_LANE0_1_STATUS,
					 dptx->link.status,
					 DP_LINK_STATUS_SIZE);
}

static int dptx_link_check_cr_done(struct dp_ctrl *dptx, bool *out_done)
{
	int retval;
	uint8_t byte = 0;
	uint32_t reg;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((out_done == NULL), -EINVAL, ERR, "[DP] out_done is NULL!\n");

	*out_done = false;

	retval = dptx_read_dpcd(dptx, DP_TRAINING_AUX_RD_INTERVAL, &byte);
	if (retval)
		return retval;

	reg = min_t(uint32_t, (byte & 0x7f), 4);
	reg *= 4000;
	if (!reg)
		reg = 400;

	/* DPTX spec acquire:
	 * TRAINING_AUX_RD_INTERVAL
	 * 00h = 400us.
	 * 01h = 4ms all.
	 * 02h = 8ms all.
	 * 03h = 12ms all.
	 * 04h = 16ms all.
	 */
	udelay(reg);

	mdelay(4);	/* Fix me: Reducing rate problem */

	retval = dptx_link_read_status(dptx);
	if (retval)
		return retval;

	*out_done = drm_dp_clock_recovery_ok(dptx->link.status,
						dptx->link.lanes);

	DPU_FB_INFO("[DP] CR_DONE = %d\n", *out_done);

	return 0;
}

static int dptx_link_check_ch_eq_done(struct dp_ctrl *dptx, bool *out_cr_done, bool *out_ch_eq_done)
{
	int retval;
	bool done = false;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((out_cr_done == NULL), -EINVAL, ERR, "[DP] out_cr_done is NULL!\n");
	dpu_check_and_return((out_ch_eq_done == NULL), -EINVAL, ERR, "[DP] out_ch_eq_done is NULL!\n");

	retval = dptx_link_check_cr_done(dptx, &done);
	if (retval)
		return retval;

	*out_cr_done = false;
	*out_ch_eq_done = false;

	if (!done)
		return 0;

	*out_cr_done = true;
	*out_ch_eq_done = drm_dp_channel_eq_ok(dptx->link.status,
					       dptx->link.lanes);

	DPU_FB_INFO("[DP] CH_EQ_DONE = %d\n", *out_ch_eq_done);

	return 0;
}

int dptx_link_training_lanes_set(struct dp_ctrl *dptx)
{
	int retval;
	uint32_t i;
	uint8_t bytes[4] = { 0xff, 0xff, 0xff, 0xff };
	uint8_t byte = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	for (i = 0; i < dptx->link.lanes; i++) {
		byte |= ((dptx->link.vswing_level[i] <<
			  DP_TRAIN_VOLTAGE_SWING_SHIFT) &
			 DP_TRAIN_VOLTAGE_SWING_MASK);

		if (dptx->link.vswing_level[i] == 3)
			byte |= DP_TRAIN_MAX_SWING_REACHED;

		byte |= ((dptx->link.preemp_level[i] <<
			  DP_TRAIN_PRE_EMPHASIS_SHIFT) &
			 DP_TRAIN_PRE_EMPHASIS_MASK);

		if (dptx->link.preemp_level[i] == 3)
			byte |= DP_TRAIN_MAX_PRE_EMPHASIS_REACHED;

		bytes[i] = byte;
	}

	retval = dptx_write_bytes_to_dpcd(dptx, DP_TRAINING_LANE0_SET, bytes,
					  dptx->link.lanes);
	if (retval)
		return retval;

	return 0;
}

int dptx_link_adjust_drive_settings(struct dp_ctrl *dptx, int *out_changed)
{
	int retval;
	uint32_t lanes;
	uint32_t i;
	uint8_t byte = 0;
	uint8_t adj[4] = { 0 };
	int changed = false;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	lanes = dptx->link.lanes;
	DPU_FB_INFO("[DP] Lane: %d\n", lanes);

	switch (lanes) {
	/* This case (value 4) is not terminated by a 'break' statement */
	case 4:
		retval = dptx_read_dpcd(dptx, DP_ADJUST_REQUEST_LANE2_3, &byte);
		if (retval)
			return retval;

		adj[2] = byte & 0x0f;
		adj[3] = (byte & 0xf0) >> 4;
	case 2:
	case 1:
		retval = dptx_read_dpcd(dptx, DP_ADJUST_REQUEST_LANE0_1, &byte);
		if (retval)
			return retval;

		adj[0] = byte & 0x0f;
		adj[1] = (byte & 0xf0) >> 4;
		break;
	default:
		DPU_FB_ERR("[DP] Invalid number of lanes %d\n", lanes);
		return -EINVAL;
	}

	/* Save the drive settings */
	for (i = 0; i < lanes; i++) {
		uint8_t vs = adj[i] & 0x3;
		uint8_t pe = (adj[i] & 0xc) >> 2;

		if (dptx->link.vswing_level[i] != vs)
			changed = true;

		dptx->link.vswing_level[i] = vs;
		dptx->link.preemp_level[i] = pe;
	}

	if (dptx->dptx_link_set_preemp_vswing)
		dptx->dptx_link_set_preemp_vswing(dptx);

	retval = dptx_link_training_lanes_set(dptx);
	if (retval)
		return retval;

	if (out_changed != NULL)
		*out_changed = changed;

	return 0;
}

static int dptx_link_training_init(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	uint8_t sink_max_rate;
	uint8_t sink_max_lanes;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	DPU_FB_INFO("[DP] lanes=%d, rate=%d\n", lanes, rate);

	if (rate > DPTX_PHYIF_CTRL_RATE_HBR3) {
		DPU_FB_ERR("[DP] Invalid rate %d\n", rate);
		rate = DPTX_PHYIF_CTRL_RATE_RBR;
	}

	if ((!lanes) || (lanes == 3) || (lanes > 4)) {
		DPU_FB_ERR("[DP] Invalid lanes %d\n", lanes);
		lanes = 1;
	}

	/* Initialize link parameters */
	memset(dptx->link.preemp_level, 0, sizeof(uint8_t) * 4);
	memset(dptx->link.vswing_level, 0, sizeof(uint8_t) * 4);
	memset(dptx->link.status, 0, DP_LINK_STATUS_SIZE);

	sink_max_lanes = drm_dp_max_lane_count(dptx->rx_caps);

	if (lanes > sink_max_lanes)
		lanes = sink_max_lanes;

	sink_max_rate = dptx->rx_caps[DP_MAX_LINK_RATE];
	sink_max_rate = dptx_bw_to_phy_rate(sink_max_rate);

	if (rate > sink_max_rate)
		rate = sink_max_rate;

	DPU_FB_INFO("[DP] Sink Device Capability: lanes=%d, rate=%d\n", sink_max_lanes, sink_max_rate);
	huawei_dp_imonitor_set_param(DP_PARAM_MAX_RATE,   &sink_max_rate);
	huawei_dp_imonitor_set_param(DP_PARAM_MAX_LANES,  &sink_max_lanes);
	huawei_dp_imonitor_set_param(DP_PARAM_LINK_RATE,  &rate);
	huawei_dp_imonitor_set_param(DP_PARAM_LINK_LANES, &lanes);

	dptx->link.lanes = lanes;
	dptx->link.rate = rate;
	dptx->link.trained = false;

	return 0;
}

int dptx_link_training_pattern_set(struct dp_ctrl *dptx, uint8_t pattern)
{
	int retval;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	retval = dptx_write_dpcd(dptx, DP_TRAINING_PATTERN_SET, pattern);
	if (retval)
		return retval;

	return 0;
}

int dptx_link_wait_cr_and_adjust(struct dp_ctrl *dptx, bool ch_eq)
{
	int i;
	int retval;
	int changed = 0;
	int param_repeat_times = 0;
	bool done = false;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	retval = dptx_link_check_cr_done(dptx, &done);
	if (retval)
		return retval;

	if (done)
		return 0;

	/* Try 10 times at most according to dp1.4 */
	for (i = 0; i < 10; i++) {
		retval = dptx_link_adjust_drive_settings(dptx, &changed);
		if (retval)
			return retval;

		/* Reset repeat count if we changed settings */
		if (changed)
			param_repeat_times = 0;

		retval = dptx_link_check_cr_done(dptx, &done);
		if (retval)
			return retval;

		if (done)
			return 0;

		param_repeat_times++;
		if (param_repeat_times >= 5) /* Try each adjustment setting 5 times */
			return -EPROTO;

		/* check for all lanes */
		/* Failed and reached the maximum voltage swing */
		if (dptx->link.vswing_level[0] == 3)
			return -EPROTO;
	}

	return -EPROTO;
}

int dptx_link_cr(struct dp_ctrl *dptx)
{
	int retval;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	DPU_FB_INFO("[DP] link training excute cr verify\n");

	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_1);

	retval = dptx_link_training_pattern_set(dptx, DP_TRAINING_PATTERN_1 | 0x20);

	if (retval)
		return retval;

	return dptx_link_wait_cr_and_adjust(dptx, false);
}

static int dptx_link_ch_eq_get_pattern(struct dp_ctrl *dptx, uint32_t *pattern, uint8_t *dp_pattern)
{
	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((pattern == NULL), -EINVAL, ERR, "[DP] pattern is NULL!\n");
	dpu_check_and_return((dp_pattern == NULL), -EINVAL, ERR, "[DP] dp_pattern is NULL!\n");

	switch (dptx->max_rate) {
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		if (drm_dp_tps4_supported(dptx->rx_caps)) {
			*pattern = DPTX_PHYIF_CTRL_TPS_4;
			*dp_pattern = DP_TRAINING_PATTERN_4;
			break;
		}
		/* Fall through */
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		if (drm_dp_tps3_supported(dptx->rx_caps)) {
			*pattern = DPTX_PHYIF_CTRL_TPS_3;
			*dp_pattern = DP_TRAINING_PATTERN_3;
			break;
		}
		/* Fall through */
	case DPTX_PHYIF_CTRL_RATE_RBR:
	case DPTX_PHYIF_CTRL_RATE_HBR:
		*pattern = DPTX_PHYIF_CTRL_TPS_2;
		*dp_pattern = DP_TRAINING_PATTERN_2;
		break;
	default:
		DPU_FB_ERR("[DP] Invalid rate %d\n", dptx->link.rate);
		return -EINVAL;
	}

	return 0;
}

int dptx_link_ch_eq(struct dp_ctrl *dptx)
{
	int retval;
	bool cr_done = false;
	bool ch_eq_done = false;
	uint32_t pattern;
	uint32_t i;
	uint8_t dp_pattern;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	DPU_FB_INFO("[DP] link training excute eq verify\n");

	retval = dptx_link_ch_eq_get_pattern(dptx, &pattern, &dp_pattern);
	if (retval)
		return retval;

	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, pattern);

	/* this needs to be different for other versions of
	 * DPRX
	 */
	retval = dptx_link_training_pattern_set(dptx,
				dp_pattern | 0x20);

	if (retval)
		return retval;

	for (i = 0; i < 6; i++) {
		retval = dptx_link_check_ch_eq_done(dptx,
						    &cr_done,
						    &ch_eq_done);

		if (retval)
			return retval;

		dptx->cr_fail = false;

		if (!cr_done) {
			dptx->cr_fail = true;
			return -EPROTO;
		}

		if (ch_eq_done)
			return 0;

		retval = dptx_link_adjust_drive_settings(dptx, NULL);

		if (retval)
			return retval;
	}

	return -EPROTO;
}

int dptx_link_reduce_rate(struct dp_ctrl *dptx)
{
	uint32_t rate = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	rate = dptx->link.rate;

	switch (rate) {
	case DPTX_PHYIF_CTRL_RATE_RBR:
		return -EPROTO;
	case DPTX_PHYIF_CTRL_RATE_HBR:
		rate = DPTX_PHYIF_CTRL_RATE_RBR;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR2:
		rate = DPTX_PHYIF_CTRL_RATE_HBR;
		break;
	case DPTX_PHYIF_CTRL_RATE_HBR3:
		rate = DPTX_PHYIF_CTRL_RATE_HBR2;
		break;
	}

	DPU_FB_INFO("[DP] Reducing rate from %d to %d\n",
					dptx->link.rate, rate);
	dptx->link.rate = rate;
	return 0;
}

int dptx_link_reduce_lanes(struct dp_ctrl *dptx)
{
	uint32_t lanes;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	switch (dptx->link.lanes) {
	case 4:
		lanes = 2;
		break;
	case 2:
		lanes = 1;
		break;
	case 1:
	default:
		return -EPROTO;
	}

	DPU_FB_INFO("[DP] Reducing lanes from %d to %d\n",
					dptx->link.lanes, lanes);
	dptx->link.lanes = lanes;
	return 0;
}

/*
 * When cr fail, try to redue lane num or lane rate
 */
static void dptx_link_cr_adjust_link_params(struct dp_ctrl *dptx, int *retval, bool *try_again)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_no_retval((retval == NULL), ERR, "[DP] retval is NULL!\n");
	dpu_check_and_no_retval((try_again == NULL), ERR, "[DP] try_again is NULL!\n");

	if (*retval == -EPROTO) {
		/* for factory test */
		if (huawei_dp_factory_mode_is_enable()) {
			huawei_dp_factory_link_cr_or_ch_eq_fail(true);
			DPU_FB_ERR("[DP] not support reduce rate by CR verify in factory mode!\n");
			*retval = -ECONNREFUSED;
			*try_again = false;
			return;
		}

		DPU_FB_INFO("[DP] Reduce rate by CR verify\n");
		if (dptx_link_reduce_rate(dptx)) {
			/*
			 * If CR_DONE bits for some lanes
			 * are set, we should reduce lanes to
			 * those lanes.
			 */
			if (dptx_link_reduce_lanes(dptx)) {
				*retval = -EPROTO;
				*try_again = false;
				return;
			}
			dptx->link.rate  = dptx->max_rate;
		}

		dptx_link_training_init(dptx,
					dptx->link.rate,
					dptx->link.lanes);
		*try_again = true;
		return;
	} else {
		*try_again = false;
		return;
	}
}

/*
 * When ch eq fail, try to redue lane num or lane rate
 */
static void dptx_link_ch_eq_adjust_link_params(struct dp_ctrl *dptx, int *retval, bool *try_again)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_no_retval((retval == NULL), ERR, "[DP] retval is NULL!\n");
	dpu_check_and_no_retval((try_again == NULL), ERR, "[DP] try_again is NULL!\n");

	if (*retval == -EPROTO) {
		/* for factory test */
		if (huawei_dp_factory_mode_is_enable()) {
			huawei_dp_factory_link_cr_or_ch_eq_fail(false);
			DPU_FB_ERR("[DP] not support reduce rate by EQ verify in factory mode!\n");
			*retval = -ECONNREFUSED;
			*try_again = false;
			return;
		}

		if (!dptx->cr_fail) {
			if (dptx->link.lanes == 1) {
				if (dptx_link_reduce_rate(dptx)) {
					*retval = -EPROTO;
					*try_again = false;
					return;
				}
				dptx->link.lanes = dptx->max_lanes;
			} else {
				(void)dptx_link_reduce_lanes(dptx);
			}
		} else {
			DPU_FB_INFO("[DP] Reduce rate by EQ verify\n");
			if (dptx_link_reduce_rate(dptx)) {
				if (dptx_link_reduce_lanes(dptx)) {
					*retval = -EPROTO;
					*try_again = false;
					return;
				}
				dptx->link.rate  = dptx->max_rate;
			}
		}

		dptx_link_training_init(dptx, dptx->link.rate, dptx->link.lanes);
		*try_again = true;
		return;
	} else {
		*try_again = false;
		return;
	}
}

int dptx_link_training(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	int retval, retval1;
	uint8_t byte;
	bool try_again = false;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, false);

	retval = dptx_link_training_init(dptx, rate, lanes);
	if (retval)
		goto fail;

again:
	DPU_FB_INFO("[DP] Link training start!\n");
	if (dptx->dptx_link_set_lane_status)
		retval = dptx->dptx_link_set_lane_status(dptx);
	if (retval)
		goto fail;

#if CONFIG_DP_ENABLE
	retval = dptx_link_cr(dptx);
	if (retval) {
		try_again = false;
		dptx_link_cr_adjust_link_params(dptx, &retval, &try_again);
		if (try_again)
			goto again;
		else
			goto fail;
	}

	retval = dptx_link_ch_eq(dptx);
	if (retval) {
		try_again = false;
		dptx_link_ch_eq_adjust_link_params(dptx, &retval, &try_again);
		if (try_again)
			goto again;
		else
			goto fail;
	}
#endif

	if (dptx->dptx_phy_set_pattern)
		dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);

	retval = dptx_link_training_pattern_set(dptx,
						DP_TRAINING_PATTERN_DISABLE);
	if (retval)
		goto fail;

	/* Branch device detection */
	retval = dptx_read_dpcd(dptx, DP_SINK_COUNT, &byte);
	if (retval)
		return retval;

	retval = dptx_read_dpcd(dptx, DP_SINK_COUNT_ESI, &byte);
	if (retval)
		return retval;

	if (dptx->dptx_link_set_lane_after_training)
		dptx->dptx_link_set_lane_after_training(dptx);


	if (dptx->dptx_triger_media_transfer)
		dptx->dptx_triger_media_transfer(dptx, true);

	dptx->detect_times = 0;

	return 0;

fail:
	if (dptx->dptx_sink_device_connected && dptx->dptx_sink_device_connected(dptx)) {
		if (dptx->dptx_phy_set_pattern)
			dptx->dptx_phy_set_pattern(dptx, DPTX_PHYIF_CTRL_TPS_NONE);
		retval1 = dptx_link_training_pattern_set(dptx, DP_TRAINING_PATTERN_DISABLE);
		if (retval1)
			return retval1;

		DPU_FB_ERR("[DP] Link training failed %d\n", retval);
	} else {
		DPU_FB_ERR("[DP]Link training failed  as sink is disconnected %d\n", retval);
	}
	return retval;
}

int dptx_link_check_status(struct dp_ctrl *dptx)
{
	int retval;
	int ret;
	uint8_t byte;
	uint8_t bytes[2];

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	retval = dptx_read_bytes_from_dpcd(dptx, DP_SINK_COUNT, bytes, 2);
	if (retval)
		return retval;

	retval = dptx_link_read_status(dptx);
	if (retval)
		return retval;

	byte = dptx->link.status[DP_LANE_ALIGN_STATUS_UPDATED - DP_LANE0_1_STATUS];

	if (!(byte & DP_LINK_STATUS_UPDATED))
		return 0;

	/* Check if need to retrain link */
	if (dptx->link.trained &&
		(!drm_dp_channel_eq_ok(dptx->link.status, dptx->link.lanes) ||
		!drm_dp_clock_recovery_ok(dptx->link.status, dptx->link.lanes))) {

		DPU_FB_INFO("[DP] Retraining link\n");
		huawei_dp_imonitor_set_param(DP_PARAM_LINK_RETRAINING, NULL);

		if (dptx->fec && dptx->dptx_fec_enable)
			dptx->dptx_fec_enable(dptx, false);

		retval = dptx_link_training(dptx, dptx->max_rate, dptx->max_lanes);
		if (retval < 0)
			huawei_dp_imonitor_set_param(DP_PARAM_LINK_TRAINING_FAILED, &retval);

		if (dptx->fec && dptx->dptx_fec_enable) {
			ret = dptx->dptx_fec_enable(dptx, true);
			if (ret)
				DPU_FB_ERR("[DP] fec enable failed!\n");
		}
		return retval;
	}

	return 0;
}

/*
 * dptx_link_retrain() - Retrain link
 * @dptx: The dptx struct
 * @rate: Link rate - Possible options: 0 - RBR, 1 - HBR, 2 - HBR2, 3 - HBR3
 * @lanes: Link lanes count - Possible options 1,2 or 4
 *
 * Returns 0 on success otherwise negative errno.
 */
int dptx_link_retraining(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes)
{
	struct video_params *vparams = NULL;
	struct dtd *mdtd = NULL;
	int i, retval;

	retval = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");

	if (dptx->dptx_sink_device_connected && !(dptx->dptx_sink_device_connected(dptx))) {
		DPU_FB_ERR("[DP] Not connected\n");
		return  -ENODEV;
	}

	if (lanes != 1 && lanes != 2 && lanes != 4) {
		DPU_FB_ERR("[DP] Invalid number of lanes %d\n",
			lanes);
		return  -EINVAL;
	}

	if (rate > DPTX_PHYIF_CTRL_RATE_HBR3) {
		DPU_FB_ERR("[DP] Invalid number of lane rate %d\n",
			rate);
		return  -EINVAL;
	}

	for (i = 0; i < 3; i++) {
		retval = dptx_link_training(dptx, rate, lanes);
		if (retval)
			return retval;

		if ((rate == dptx->link.rate) && (lanes == dptx->link.lanes))
			break;
		DPU_FB_INFO("[DP] link retrain reduce rate, try again !!!\n");
		mdelay(100);
	}

	if (i == 3) {
		DPU_FB_ERR("[DP] Link retrain reduce rate!!!\n");
		return -EINVAL;
	}

	vparams = &dptx->vparams;
	mdtd = &vparams->mdtd;
	if (dptx->dptx_video_ts_calculate)
		retval = dptx->dptx_video_ts_calculate(dptx, dptx->link.lanes, dptx->link.rate, vparams->bpc,
				vparams->pix_enc, mdtd->pixel_clock);
	if (retval)
		return retval;

	DPU_FB_INFO("[DP] Retraining link rate=%d lanes=%d\n",
		rate, lanes);

	return retval;
}

int dptx_power_handler(struct dp_ctrl *dptx, bool ublank, bool *bpresspowerkey)
{
	int retval = 0;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((bpresspowerkey == NULL), -EINVAL, ERR, "[DP] bpresspowerkey is NULL!\n");

	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_enable && dptx->video_transfer_enable) {
		if (ublank) {
			if (*bpresspowerkey) {
				retval = dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D0);
				if (retval) {
					DPU_FB_ERR("dptx_write_dpcd return value: %d.\n", retval);
					mutex_unlock(&dptx->dptx_mutex);
					return retval; //lint !e454
				}

				mdelay(10);

				if (dptx->fec && dptx->dptx_fec_enable)
					dptx->dptx_fec_enable(dptx, false);

				retval = dptx_link_retraining(dptx, dptx->current_link_rate, dptx->current_link_lanes);
				if (retval < 0)
					huawei_dp_imonitor_set_param(DP_PARAM_LINK_RETRAINING_FAILED, &retval);

				if (dptx->fec && dptx->dptx_fec_enable) {
					retval = dptx->dptx_fec_enable(dptx, true);
					if (retval)
						DPU_FB_ERR("[DP] fec enable failed!\n");
				}

				if (dptx->dptx_enable_default_video_stream)
					dptx->dptx_enable_default_video_stream(dptx, 0);
				*bpresspowerkey = false;
				DPU_FB_INFO("[DP] Retraining when blank on.\n");
			}
		} else {
			retval = dptx_write_dpcd(dptx, DP_SET_POWER, DP_SET_POWER_D3);
			if (retval) {
				DPU_FB_ERR("dptx_write_dpcd return value: %d.\n", retval);
				mutex_unlock(&dptx->dptx_mutex);
				return retval; //lint !e454
			}
			if (dptx->dptx_disable_default_video_stream)
				dptx->dptx_disable_default_video_stream(dptx, 0);
			*bpresspowerkey = true;
			DPU_FB_INFO("[DP] Disable stream when blank off.\n");
		}
	} else {
		DPU_FB_INFO("[DP] SRS never excute!\n");
		mutex_unlock(&dptx->dptx_mutex);
		return 0;
	}

	mutex_unlock(&dptx->dptx_mutex);
	return 0;
}

static void dptx_hdr_calculate_infoframe_data(struct dp_ctrl *dptx)
{
	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");

	dptx->hdr_infoframe.data[HDR_INFOFRAME_EOTF_BYTE_NUM] = dptx->hdr_metadata.electro_optical_transfer_function;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_METADATA_ID_BYTE_NUM] = dptx->hdr_metadata.static_metadata_descriptor_id;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_0_LSB] = dptx->hdr_metadata.red_primary_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_0_MSB]
		= (dptx->hdr_metadata.red_primary_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_0_LSB] = dptx->hdr_metadata.red_primary_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_0_MSB]
		= (dptx->hdr_metadata.red_primary_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_1_LSB] = dptx->hdr_metadata.green_primary_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_1_MSB]
		= (dptx->hdr_metadata.green_primary_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_1_LSB] = dptx->hdr_metadata.green_primary_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_1_MSB]
		= (dptx->hdr_metadata.green_primary_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_2_LSB] = dptx->hdr_metadata.blue_primary_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_X_2_MSB]
		= (dptx->hdr_metadata.blue_primary_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_2_LSB] = dptx->hdr_metadata.blue_primary_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_DISP_PRI_Y_2_MSB]
		= (dptx->hdr_metadata.blue_primary_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_X_LSB] = dptx->hdr_metadata.white_point_x & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_X_MSB]
		= (dptx->hdr_metadata.white_point_x & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_Y_LSB] = dptx->hdr_metadata.white_point_y & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_WHITE_POINT_Y_MSB]
		= (dptx->hdr_metadata.white_point_y & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LUMI_LSB]
		= dptx->hdr_metadata.max_display_mastering_luminance & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LUMI_MSB]
		= (dptx->hdr_metadata.max_display_mastering_luminance & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MIN_LUMI_LSB]
		= dptx->hdr_metadata.min_display_mastering_luminance & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MIN_LUMI_MSB]
		= (dptx->hdr_metadata.min_display_mastering_luminance & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LIGHT_LEVEL_LSB]
		= dptx->hdr_metadata.max_content_light_level & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_LIGHT_LEVEL_MSB]
		= (dptx->hdr_metadata.max_content_light_level & MSB_MASK) >> SHIFT_8BIT;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_AVERAGE_LEVEL_LSB]
		= dptx->hdr_metadata.max_frame_average_light_level & LSB_MASK;
	dptx->hdr_infoframe.data[HDR_INFOFRAME_MAX_AVERAGE_LEVEL_MSB]
		= (dptx->hdr_metadata.max_frame_average_light_level & MSB_MASK) >> SHIFT_8BIT;
}

int dptx_hdr_infoframe_sdp_send(struct dp_ctrl *dptx, const void __user *argp)
{
	int i;

	dpu_check_and_return((dptx == NULL), -EINVAL, ERR, "[DP] dptx is NULL!\n");
	dpu_check_and_return((argp == NULL), -EINVAL, ERR, "[DP] argp is NULL!\n");

	dptx->hdr_infoframe.type_code = INFOFRAME_PACKET_TYPE_HDR;
	dptx->hdr_infoframe.version_number = HDR_INFOFRAME_VERSION;
	dptx->hdr_infoframe.length = HDR_INFOFRAME_LENGTH;

	memset(dptx->hdr_infoframe.data, 0x00, HDR_INFOFRAME_LENGTH);

	if (copy_from_user(&(dptx->hdr_metadata), (void __user *)argp, sizeof(dptx->hdr_metadata))) {
		DPU_FB_ERR("[%s]: copy arg failed!\n", __func__);
		return -EFAULT;
	}

	dptx_hdr_calculate_infoframe_data(dptx);

	for (i = 0; i < HDR_INFOFRAME_LENGTH; i++)
		DPU_FB_DEBUG("[DP] hdr_infoframe->data[%d] = 0x%02x", i, dptx->hdr_infoframe.data[i]);

	mutex_lock(&dptx->dptx_mutex);

	if (dptx->dptx_hdr_infoframe_set_reg)
		dptx->dptx_hdr_infoframe_set_reg(dptx, 1);

	mutex_unlock(&dptx->dptx_mutex);

	return 0;
}

void dptx_link_layer_init(struct dp_ctrl *dptx)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_no_retval((dptx == NULL), ERR, "[DP] dptx is NULL!\n");

	dpufd = dptx->dpufd;
	dpu_check_and_no_retval((dpufd == NULL), ERR, "[DP] dpufd is NULL!\n");

	dpufd->dptx_hdr_infoframe_sdp_send = dptx_hdr_infoframe_sdp_send;

	dptx_link_params_reset(dptx);

	dptx->handle_hotplug = handle_hotplug;
	dptx->handle_hotunplug = handle_hotunplug;
	dptx->dptx_power_handler = dptx_power_handler;
	dptx->dptx_dsc_check_rx_cap = dptx_dsc_check_rx_cap;
	dptx->dptx_dsc_para_init = dptx_dsc_para_init;

	dptx_ctrl_layer_init(dptx);
}
/*lint -restore*/
#pragma GCC diagnostic pop
