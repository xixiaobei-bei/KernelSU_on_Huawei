/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include <linux/module.h>
#include <linux/hisi/hitest_slt.h>
#include "dp_dbg.h"
#include "../hisi_fb_def.h"
#include "../hisi_dp.h"

/*******************************************************************************
 * DP GRAPHIC DEBUG TOOL
 */

/*
 * for debug, S_IRUGO
 * /sys/module/dpufb/parameters
 */
int g_dp_debug_mode_enable = 0;

int g_dp_lane_num_debug = 4;

int g_dp_lane_rate_debug = 2;

int g_dp_ssc_enable_debug = 0;

int g_dp_fec_enable_debug = 0;

int g_dp_dsc_enable_debug = 0;

int g_dp_mst_enable_debug = 0;

int g_dp_efm_enable_debug = 1;

int g_dp_aux_ronselp = 1;

int g_dp_user_format_debug = 0;

int g_dp_user_code_debug = 3;


/*******************************************************************************
 * DP CDC SLT TEST
 */

/*******************************************************************************
 * Interface provided by cbg, include data monitor, event report, and etc.
 */
#ifdef HUAWEI_DP_DSM_ENABLE
void huawei_dp_imonitor_set_param(enum dp_imonitor_param param, void *data)
{
	dp_imonitor_set_param(param, data);
}

void huawei_dp_imonitor_set_param_aux_rw(bool rw,
	bool i2c,
	uint32_t addr,
	uint32_t len,
	int retval)
{
	dp_imonitor_set_param_aux_rw(rw, i2c, addr, len, retval);
}

void huawei_dp_imonitor_set_param_timing(uint16_t h_active,
	uint16_t v_active,
	uint32_t pixel_clock,
	uint8_t vesa_id)
{
	dp_imonitor_set_param_timing(h_active, v_active, pixel_clock, vesa_id);
}

void huawei_dp_imonitor_set_param_err_count(uint16_t lane0_err,
	uint16_t lane1_err,
	uint16_t lane2_err,
	uint16_t lane3_err)
{
	dp_imonitor_set_param_err_count(lane0_err, lane1_err, lane2_err, lane3_err);
}

void huawei_dp_imonitor_set_param_vs_pe(int index, uint8_t *vs, uint8_t *pe)
{
	dp_imonitor_set_param_vs_pe(index, vs, pe);
}

void huawei_dp_imonitor_set_param_resolution(uint8_t *user_mode, uint8_t *user_format)
{
	dp_imonitor_set_param_resolution(user_mode, user_format);
}

#if defined(CONFIG_VR_DISPLAY)
void huawei_dp_set_dptx_vr_status(bool dptx_vr)
{
	dp_set_dptx_vr_status(dptx_vr);
}
#endif

bool huawei_dp_factory_mode_is_enable(void)
{
	return dp_factory_mode_is_enable();
}

void huawei_dp_factory_link_cr_or_ch_eq_fail(bool is_cr)
{
	dp_factory_link_cr_or_ch_eq_fail(is_cr);
}

bool huawei_dp_factory_is_4k_60fps(uint8_t rate,
	uint8_t lanes,
	uint16_t h_active,
	uint16_t v_active,
	uint8_t fps)
{
	return dp_factory_is_4k_60fps(rate, lanes, h_active, v_active, fps);
}

int huawei_dp_get_current_dp_source_mode(void)
{
	return get_current_dp_source_mode();
}

int huawei_dp_update_external_display_timming_info(uint32_t width,
	uint32_t high,
	uint32_t fps)
{
	return update_external_display_timming_info(width, high, fps);
}

void huawei_dp_send_event(enum dp_event_type event)
{
	dp_send_event(event);
}

void huawei_dp_debug_init_combophy_pree_swing(uint32_t *pv, int count)
{
	dp_debug_init_combophy_pree_swing(pv, count);
}

void huawei_dp_save_edid(uint8_t *edid_buf, uint32_t buf_len)
{
	(void)save_dp_edid(edid_buf, buf_len);
}

#endif /* HUAWEI_DP_DSM_ENABLE */
