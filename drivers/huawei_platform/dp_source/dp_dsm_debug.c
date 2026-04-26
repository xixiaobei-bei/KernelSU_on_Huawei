/*
 * dp_dsm_debug.c
 *
 * dsm debug for DP module
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/types.h>
#include <linux/list.h>
#include <dsm/dsm_pub.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/log/imonitor.h>
#include <huawei_platform/dp_source/dp_dsm.h>
#include <huawei_platform/dp_source/dp_debug.h>
#include <huawei_platform/dp_source/dp_factory.h>
#include <huawei_platform/dp_source/dp_source_switch.h>

#include "dp_dsm_internal.h"

#ifdef DP_DSM_ENABLE
#define HWLOG_TAG dp_dsm_debug
HWLOG_REGIST();

#ifdef DP_DEBUG_ENABLE
static bool dp_check_typec_cable_in_out_existed(struct dp_dsm_priv *priv)
{
	if (priv->tpyec_in_time.tv_sec == 0)
		return false;

	return true;
}

static int dp_get_pd_event_result_in_out(char *buffer, int size)
{
	struct dp_typec_in_out *typec_in_out = dp_get_typec_in_out();
	char datetime[DP_DSM_DATETIME_SIZE] = { 0 };
	int earlist_point;
	int count;
	int i;
	int ret = 0;

	if (typec_in_out->in_out_num <= 0)
		return 0;

	earlist_point = DP_DSM_EARLIEST_TIME_POINT(
		typec_in_out->in_out_num, DP_DSM_TYPEC_IN_OUT_NUM);
	count = MIN(typec_in_out->in_out_num, DP_DSM_TYPEC_IN_OUT_NUM);

	ret += dp_debug_append_info(buffer, size, "[**] %-19s %-19s\n",
		"typec_in_time", "typec_out_time");
	for (i = 0; i < count; i++) {
		dp_timeval_to_datetime_str(typec_in_out->in_time[i],
			"tpyec_in", datetime, DP_DSM_DATETIME_SIZE);
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-19s", i, datetime);

		dp_timeval_to_datetime_str(typec_in_out->out_time[i],
			"tpyec_out", datetime, DP_DSM_DATETIME_SIZE);
		if (i != earlist_point)
			ret += dp_debug_append_info(buffer, size,
				" %-19s\n", datetime);
		else
			ret += dp_debug_append_info(buffer, size,
				" %-19s*\n", datetime);
	}
	ret += dp_debug_append_info(buffer, size, "\n");

	return ret;
}

static int dp_get_pd_event_result_irq(struct dp_dsm_priv *priv,
	char *buffer, int size)
{
	char datetime[DP_DSM_DATETIME_SIZE] = { 0 };
	struct dp_connected_event *event = NULL;
	struct list_head *pos = NULL;
	int ret = 0;
	int i = 0;

	ret += dp_debug_append_info(buffer, size,
		"[**] %-19s %-12s %-12s %-12s %-12s %-12s\n",
		"time", "irq_type", "cur_mode",
		"mode_type", "dev_type", "typec_orien");

	list_for_each(pos, &(priv->event_list_head)) {
		event = list_entry(pos, struct dp_connected_event, list);

		dp_timeval_to_datetime_str(event->time,
			"pd_event", datetime, DP_DSM_DATETIME_SIZE);
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-19s %-12u %-12u %-12u %-12u %-12u\n",
			i, datetime, event->irq_type, event->cur_mode,
			event->mode_type, event->dev_type, event->typec_orien);
		i++;
	}

	return ret;
}

static int dp_get_pd_event_result_irq_vector(struct dp_dsm_priv *priv,
	char *buffer, int size)
{
	int earlist_point;
	int count;
	int i;
	int ret = 0;

	if (priv->irq_vector_num <= 0)
		return 0;

	earlist_point = DP_DSM_EARLIEST_TIME_POINT(
		priv->irq_vector_num, DP_DSM_IRQ_VECTOR_NUM);
	count = MIN(priv->irq_vector_num, DP_DSM_IRQ_VECTOR_NUM);

	ret += dp_debug_append_info(buffer, size,
		"irq vector(%d):", priv->irq_vector_num);
	for (i = 0; i < count; i++) {
		if (i != earlist_point)
			ret += dp_debug_append_info(buffer, size,
				" %u", priv->irq_vector[i]);
		else
			ret += dp_debug_append_info(buffer, size,
				" %u*", priv->irq_vector[i]);
	}
	ret += dp_debug_append_info(buffer, size, "\n");

	return ret;
}

static int dp_get_pd_event_result_hdcp(struct dp_dsm_priv *priv,
	char *buffer, int size)
{
	char datetime[DP_DSM_DATETIME_SIZE] = { 0 };
	int ret = 0;

	if (priv->hdcp_key == HDCP_KEY_INVALID)
		return 0;

	dp_timeval_to_datetime_str(priv->hdcp_time,
		"hdcp_auth", datetime, DP_DSM_DATETIME_SIZE);
	ret += dp_debug_append_info(buffer, size,
		"hdcp_auth_time: %s\n", datetime);

	ret += dp_debug_append_info(buffer, size, "hdcp_version: %s\n",
		(priv->hdcp_version == 3) ? "1.3" : "2.2 or other");
	ret += dp_debug_append_info(buffer, size, "hdcp_key: %s\n",
		(priv->hdcp_key == HDCP_KEY_FAILED) ? "failed" : "success");

	return ret;
}

int dp_get_pd_event_result(char *buffer, int size)
{
	struct dp_dsm_priv *priv = dp_get_dp_dsm_priv();
	char datetime[DP_DSM_DATETIME_SIZE] = { 0 };
	int ret = 0;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return -EINVAL;
	}

	ret += dp_get_pd_event_result_in_out(buffer, size);

	if (!dp_check_typec_cable_in_out_existed(priv)) {
		ret += dp_debug_append_info(buffer, size,
			"pd_event not found\n");
		goto err_out;
	}

	dp_timeval_to_datetime_str(priv->tpyec_in_time,
		"tpyec_in", datetime, DP_DSM_DATETIME_SIZE);
	ret += dp_debug_append_info(buffer, size,
		"tpyec_in_time: %s\n", datetime);
	dp_timeval_to_datetime_str(priv->tpyec_out_time,
		"tpyec_out", datetime, DP_DSM_DATETIME_SIZE);
	ret += dp_debug_append_info(buffer, size,
		"tpyec_out_time: %s\n", datetime);
	ret += dp_debug_append_info(buffer, size,
		"irq_hpd_num: %d\n", priv->irq_hpd_num);
	ret += dp_debug_append_info(buffer, size,
		"irq_short_num: %d\n", priv->irq_short_num);
	ret += dp_debug_append_info(buffer, size,
		"dp_in_num: %d\n", priv->dp_in_num);
	ret += dp_debug_append_info(buffer, size,
		"dp_out_num: %d\n", priv->dp_out_num);
	ret += dp_debug_append_info(buffer, size,
		"link_retraining_num: %d\n", priv->link_retraining_num);
	ret += dp_debug_append_info(buffer, size, "\n");

	ret += dp_get_pd_event_result_irq(priv, buffer, size);
	ret += dp_get_pd_event_result_irq_vector(priv, buffer, size);

	// hdcp authentication
	ret += dp_get_pd_event_result_hdcp(priv, buffer, size);

err_out:
	if (ret < 0)
		return -EFAULT;

	return strlen(buffer);
}
EXPORT_SYMBOL_GPL(dp_get_pd_event_result);

static int dp_get_hotplug_result_basic(struct dp_hotplug_info *hotplug,
	char *buffer, int size)
{
	int ret = 0;

	if (hotplug == NULL)
		return 0;

	if (hotplug->max_rate > 0) {
		ret += dp_debug_append_info(buffer, size,
			"sink max rate: %u\n", hotplug->max_rate);
		ret += dp_debug_append_info(buffer, size,
			"sink max lanes: %u\n", hotplug->max_lanes);
	}

	if (hotplug->max_width > 0) {
		ret += dp_debug_append_info(buffer, size,
			"mid: %s\n", hotplug->mid);
		ret += dp_debug_append_info(buffer, size,
			"monitor: %s\n", hotplug->monitor);
		ret += dp_debug_append_info(buffer, size,
			"max_width: %u\n", hotplug->max_width);
		ret += dp_debug_append_info(buffer, size,
			"max_high: %u\n", hotplug->max_high);
		ret += dp_debug_append_info(buffer, size,
			"pixel_clock: %d\n", hotplug->pixel_clock);
	}
	ret += dp_debug_append_info(buffer, size, "\n");

	return ret;
}
static int dp_get_hotplug_result_failed(struct dp_dsm_priv *priv,
	char *buffer, int size, char *datetime, int count)
{
	int ret = 0;

	if (priv->aux_rw.retval < 0)
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-8s %-10s(aux rw:%d, i2c:%d, addr:0x%x, len:%u, retval:%d)\n",
			count, "failed", datetime, priv->aux_rw.rw,
			priv->aux_rw.i2c, priv->aux_rw.addr,
			priv->aux_rw.len, priv->aux_rw.retval);
	else if (priv->link_training_retval < 0)
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-8s %-10s(link training failed %d)\n",
			count, "failed", datetime, priv->link_training_retval);
	else if (priv->dss_pwron_failed)
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-8s %-10s(dss pwron failed)\n",
			count, "failed", datetime);
	else
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-8s %-10s(hotplug failed %d)\n",
			count, "failed", datetime, priv->hotplug_retval);

	return ret;
}

static int dp_get_hotplug_result_success(struct dp_hotplug_info *hotplug,
	char *buffer, int size, char *datetime, int count)
{
	int index;
	int ret = 0;

	if (hotplug->hotplug_retval < 0) {
		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-8s %-10s(hotplug failed %d)\n",
			count, "failed", datetime, hotplug->hotplug_retval);
		return ret;
	}

	ret += dp_debug_append_info(buffer, size,
		"[%02d] %-8s %-10s", count, "success", datetime);

	index = DP_PARAM_TIME_LINK_SUCC - DP_PARAM_TIME;
	dp_timeval_to_time_str(hotplug->time[index],
		"link_succ_time", datetime, DP_DSM_DATETIME_SIZE);
	ret += dp_debug_append_info(buffer, size, " %-14s", datetime);

	index = DP_PARAM_TIME_STOP - DP_PARAM_TIME;
	dp_timeval_to_time_str(hotplug->time[index],
		"stop_time", datetime, DP_DSM_DATETIME_SIZE);
	ret += dp_debug_append_info(buffer, size, " %-9s", datetime);

	ret += dp_debug_append_info(buffer, size,
		" %-6s %-5u %-5u %-5u %-4u %-5u %-5u %-9s %-10s %-6s\n",
		hotplug->source_mode ? "same" : "diff",
		hotplug->width, hotplug->high, hotplug->fps,
		hotplug->rate, hotplug->lanes, hotplug->basic_audio,
		hotplug->safe_mode ? "safe" : "normal",
		(hotplug->read_edid_retval == 0) ? "success" : "failed",
		(hotplug->tu_fail >= DP_DSM_TU_FAIL) ? "reduce" : "normal");

	return ret;
}

static int dp_get_hotplug_result_full(struct dp_dsm_priv *priv,
	char *buffer, int size)
{
	char datetime[DP_DSM_DATETIME_SIZE] = { 0 };
	struct list_head *pos = NULL;
	struct dp_hotplug_info *hotplug = NULL;
	int index;
	int ret = 0;
	int i = 0;

	ret += dp_debug_append_info(buffer, size,
		"[**] %-8s %-10s %-14s %-9s %-6s %-5s %-5s %-5s %-4s %-5s %-5s %-9s %-10s %-6s\n",
		"status", "start_time", "link_succ_time", "stop_time",
		"source", "width", "high", "fps", "rate", "lanes",
		"audio", "safe_mode", "edid_state", "reduce");

	list_for_each(pos, &(priv->hotplug_list_head)) {
		hotplug = list_entry(pos, struct dp_hotplug_info, list);

		index = DP_PARAM_TIME_START - DP_PARAM_TIME;
		dp_timeval_to_time_str(hotplug->time[index],
			"start_time", datetime, DP_DSM_DATETIME_SIZE);
		if (priv->hotplug_retval < 0)
			ret += dp_get_hotplug_result_failed(priv,
				buffer, size, datetime, i);
		else
			ret += dp_get_hotplug_result_success(hotplug,
				buffer, size, datetime, i);

		i++;
	}

	return ret;
}

static int dp_get_hotplug_result_source_switch(struct dp_dsm_priv *priv,
	char *buffer, int size)
{
	int earlist_point;
	int count;
	int ret = 0;
	int i;

	if (priv->source_switch_num <= 0)
		return 0;

	earlist_point = DP_DSM_EARLIEST_TIME_POINT(
		priv->source_switch_num, DP_DSM_SOURCE_SWITCH_NUM);
	count = MIN(priv->source_switch_num, DP_DSM_SOURCE_SWITCH_NUM);

	ret += dp_debug_append_info(buffer, size,
		"source switch(%d):", priv->source_switch_num);
	for (i = 0; i < count; i++) {
		if (i != earlist_point)
			ret += dp_debug_append_info(buffer, size,
				" %u", priv->source_switch[i]);
		else
			ret += dp_debug_append_info(buffer, size,
				" %u*", priv->source_switch[i]);
	}
	ret += dp_debug_append_info(buffer, size, "\n");

	return ret;
}

int dp_get_hotplug_result(char *buffer, int size)
{
	struct dp_dsm_priv *priv = dp_get_dp_dsm_priv();
	struct dp_imonitor_report_info *report = dp_get_imonitor_report();
	int ret = 0;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return -EINVAL;
	}

	if (!dp_check_typec_cable_in_out_existed(priv)) {
		ret += dp_debug_append_info(buffer, size,
			"hotplug not found\n");
		goto err_out;
	}

	ret += dp_debug_append_info(buffer, size, "cable max rate: 2\n");
	if (priv->tpyec_cable_type == TCPC_DP)
		ret += dp_debug_append_info(buffer, size,
			"cable max lanes: 4\n");
	else // TCPC_USB31_AND_DP_2LINE
		ret += dp_debug_append_info(buffer, size,
			"cable max lanes: 2\n");

	ret += dp_get_hotplug_result_basic(priv->cur_hotplug_info,
		buffer, size);
	ret += dp_get_hotplug_result_full(priv, buffer, size);

	// source switch
	ret += dp_get_hotplug_result_source_switch(priv, buffer, size);

	// dp link failed in func dp_device_srs()
	if (priv->link_retraining_retval < 0)
		ret += dp_debug_append_info(buffer, size,
			"dp resume failed by press powerkey\n");

	// hpd not existed
	if (!report->hpd_existed)
		ret += dp_debug_append_info(buffer, size, "hpd not existed\n");

err_out:
	if (ret < 0)
		return -EFAULT;

	return strlen(buffer);
}
EXPORT_SYMBOL_GPL(dp_get_hotplug_result);

int dp_get_timing_info_result(char *buffer, int size)
{
	struct dp_dsm_priv *priv = dp_get_dp_dsm_priv();
	struct list_head *pos = NULL;
	struct dp_timing_info *timing = NULL;
	int ret = 0;
	int i = 0;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return -EINVAL;
	}

	// timing info
	ret += dp_debug_append_info(buffer, size, "[**] %-8s %-8s %-11s %-7s\n",
		"h_active", "v_active", "pixel_clock", "vesa_id");

	list_for_each(pos, &(priv->timing_list_head)) {
		timing = list_entry(pos, struct dp_timing_info, list);

		ret += dp_debug_append_info(buffer, size,
			"[%02d] %-8u %-8u %-11u %-7u\n", i, timing->h_active,
			timing->v_active, timing->pixel_clock, timing->vesa_id);
		i++;
	}

	if (ret < 0)
		return -EFAULT;

	return strlen(buffer);
}
EXPORT_SYMBOL_GPL(dp_get_timing_info_result);

static bool dp_is_vs_pe_force(struct dp_hotplug_info *hotplug)
{
	if ((hotplug->vs_force != DP_DSM_VS_PE_MASK) &&
		(hotplug->pe_force != DP_DSM_VS_PE_MASK))
		return true;

	return false;
}

int dp_get_vs_pe_result(char *buffer, int size)
{
	struct dp_dsm_priv *priv = dp_get_dp_dsm_priv();
	struct dp_hotplug_info *hotplug = NULL;
	int ret = 0;
	int i;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return -EINVAL;
	}

	if (priv->cur_hotplug_info == NULL) {
		ret += dp_debug_append_info(buffer, size,
			"hotplug not found\n");
		goto err_out;
	}
	hotplug = priv->cur_hotplug_info;

	if (priv->hotplug_retval < 0) {
		ret += dp_debug_append_info(buffer, size, "hotplug failed\n");
		goto err_out;
	}

	if (!dp_is_vs_pe_force(hotplug)) {
		ret += dp_debug_append_info(buffer, size,
			"vs_force: not set\n");
		ret += dp_debug_append_info(buffer, size,
			"pe_force: not set\n");
	} else {
		ret += dp_debug_append_info(buffer, size,
			"vs_force: %u\n", hotplug->vs_force);
		ret += dp_debug_append_info(buffer, size,
			"pe_force: %u\n", hotplug->pe_force);
	}

	// vs and pe
	ret += dp_debug_append_info(buffer, size, "[*] vs pe\n");
	for (i = 0; i < hotplug->lanes; i++)
		ret += dp_debug_append_info(buffer, size, "[%d] %-2u %-2u\n", i,
			hotplug->vp.vs_pe[i].vswing,
			hotplug->vp.vs_pe[i].preemp);

	// vs_force and pe_force
	if (dp_is_vs_pe_force(hotplug)) {
		ret += dp_debug_append_info(buffer, size,
			"[*] vs_force pe_force\n");
		for (i = 0; i < hotplug->lanes; i++)
			ret += dp_debug_append_info(buffer, size,
				"[%d] %-8u %-8u\n", i,
				hotplug->vp_force.vs_pe[i].vswing,
				hotplug->vp_force.vs_pe[i].preemp);
	}

err_out:
	if (ret < 0)
		return -EFAULT;

	return strlen(buffer);
}
EXPORT_SYMBOL_GPL(dp_get_vs_pe_result);
#endif // DP_DEBUG_ENABLE

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei dp dsm driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
#endif // DP_DSM_ENABLE
