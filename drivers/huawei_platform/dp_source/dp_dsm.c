/*
 * dp_dsm.c
 *
 * dsm for DP module
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
#define HWLOG_TAG dp_dsm
HWLOG_REGIST();

static const unsigned char data_bin2ascii[17] = "0123456789ABCDEF";
#define CONV_BIN2ASCII(a) (data_bin2ascii[(a) & 0xF])
#define CONV_BIN2ASCII_HIGH(a) CONV_BIN2ASCII(((a) >> 4) & 0xF)
#define CONV_BIN2ASCII_LOW(a)  CONV_BIN2ASCII((a) & 0xF)

static const uint8_t edid_v1_header[DP_DSM_EDID_HEADER_SIZE] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

static int dp_imonitor_basic_info(struct imonitor_eventobj *obj, void *data);
static int dp_imonitor_time_info(struct imonitor_eventobj *obj, void *data);
static int dp_imonitor_extend_info(struct imonitor_eventobj *obj, void *data);
static int dp_imonitor_hpd_info(struct imonitor_eventobj *obj, void *data);
static int dp_imonitor_link_training_info(struct imonitor_eventobj *obj,
	void *data);
static int dp_imonitor_hotplug_info(struct imonitor_eventobj *obj, void *data);
static int dp_imonitor_hdcp_info(struct imonitor_eventobj *obj, void *data);

static struct dp_imonitor_event_id imonitor_event_id[] = {
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_BASIC_INFO, dp_imonitor_basic_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_TIME_INFO, dp_imonitor_time_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_EXTEND_INFO, dp_imonitor_extend_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_HPD, dp_imonitor_hpd_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_LINK_TRAINING,
		dp_imonitor_link_training_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_HOTPLUG, dp_imonitor_hotplug_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_HDCP, dp_imonitor_hdcp_info),
};

static struct dp_dmd_error_no dmd_error_no[] = {
	// NOTE: 936000100 and 936000101 have been discarded
};

static struct dsm_dev dp_dsm = {
	.name = "dsm_dp",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = DP_DSM_BUF_SIZE,
};

static struct dp_dsm_priv *g_dp_dsm_priv;
static struct dsm_client *g_dp_dsm_client;
static struct dp_imonitor_report_info g_imonitor_report;

#ifdef DP_DEBUG_ENABLE
static struct dp_typec_in_out g_typec_in_out;

struct dp_dsm_priv *dp_get_dp_dsm_priv(void)
{
	return g_dp_dsm_priv;
}

struct dp_typec_in_out *dp_get_typec_in_out(void)
{
	return &g_typec_in_out;
}

struct dp_imonitor_report_info *dp_get_imonitor_report(void)
{
	return &g_imonitor_report;
}
#endif // DP_DEBUG_ENABLE

// param time:
// true:  only time
// false: date and time
static int dp_timeval_to_str(struct timeval time, const char *comment,
	char *str, int size, bool only_time)
{
	struct rtc_time tm;
	int ret = 0;

	if ((comment == NULL) || (str == NULL)) {
		hwlog_err("%s: comment or str is NULL\n", __func__);
		return -EINVAL;
	}
	memset(str, 0, size);

	time.tv_sec -= sys_tz.tz_minuteswest * DP_DSM_TIME_SECOND_OF_MINUTE;
	rtc_time_to_tm(time.tv_sec, &tm);

	if (only_time)
		ret = snprintf(str, size, "%02d:%02d:%02d", tm.tm_hour,
			tm.tm_min, tm.tm_sec);
	else
		ret = snprintf(str, size, "%04d-%02d-%02d %02d:%02d:%02d",
			tm.tm_year + DP_DSM_TIME_YEAR_OFFSET,
			tm.tm_mon + DP_DSM_TIME_MONTH_OFFSET, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);

	if (ret < 0)
		hwlog_info("%s: %s failed %d\n", __func__, comment, ret);
#ifdef DP_DSM_DEBUG
	else
		hwlog_info("%s: %s is %s %d\n", __func__, comment, str, ret);
#endif

	return 0;
}

#ifdef DP_DEBUG_ENABLE
int dp_timeval_to_time_str(struct timeval time, const char *comment,
	char *str, int size)
{
	return dp_timeval_to_str(time, comment, str, size, true);
}
#endif

int dp_timeval_to_datetime_str(struct timeval time, const char *comment,
	char *str, int size)
{
	return dp_timeval_to_str(time, comment, str, size, false);
}

#ifdef DP_LINK_TIMEVAL_FORMAT
static int dp_timeval_sec_to_str(unsigned long time, const char *comment,
	char *str, int size)
{
	struct rtc_time tm;
	int ret = 0;

	if ((comment == NULL) || (str == NULL)) {
		hwlog_err("%s: comment or str is NULL\n", __func__);
		return -EINVAL;
	}
	memset(str, 0, size);

	tm.tm_mday = time / DP_DSM_TIME_SECOND_OF_DAY;
	time -= (unsigned long)tm.tm_mday * DP_DSM_TIME_SECOND_OF_DAY;

	tm.tm_hour = time / DP_DSM_TIME_SECOND_OF_HOUR;
	time -= (unsigned long)tm.tm_hour * DP_DSM_TIME_SECOND_OF_HOUR;

	tm.tm_min = time / DP_DSM_TIME_SECOND_OF_MINUTE;
	tm.tm_sec = time - (unsigned long)tm.tm_min *
		DP_DSM_TIME_SECOND_OF_MINUTE;

	ret = snprintf(str, size, "%d %02d:%02d:%02d",
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef DP_DSM_DEBUG
	if (ret < 0)
		hwlog_info("%s: %s failed %d\n", __func__, comment, ret);
	else
		hwlog_info("%s: %s is %s %d\n", __func__, comment, str, ret);
#endif

	return 0;
}
#endif // DP_LINK_TIMEVAL_FORMAT

static int dp_data_bin_to_str(char *dst, int dst_len,
	uint8_t *src, int src_len, int shift)
{
	int remaining = src_len;

	if ((dst == NULL) || (src == NULL)) {
		hwlog_err("%s: dst or src is NULL\n", __func__);
		return -EINVAL;
	}

	if (shift == DP_DSM_BIN_TO_STR_SHIFT8) {
		if (dst_len <= src_len) {
			hwlog_err("%s: invalid params %d<=%d\n",
				__func__, dst_len, src_len);
			return -EINVAL;
		}
	} else { // DP_DSM_BIN_TO_STR_SHIFT4
		if (dst_len <= (src_len * DP_DSM_BIN_TO_STR_TIMES)) {
			hwlog_err("%s: invalid params %d<=%d*2\n",
				__func__, dst_len, src_len);
			return -EINVAL;
		}
	}

	memset(dst, 0, dst_len);
	while (remaining) {
		if (shift == DP_DSM_BIN_TO_STR_SHIFT8) {
			*(dst++) = CONV_BIN2ASCII(*(src++));
		} else { // DP_DSM_BIN_TO_STR_SHIFT4
			*(dst++) = CONV_BIN2ASCII_HIGH(*src);
			*(dst++) = CONV_BIN2ASCII_LOW(*src);
			src++;
		}

		remaining--;
	}

	return 0;
}

void dp_set_hotplug_state(enum dp_link_state state)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;
	int old_state = 0;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	if (priv->last_hotplug_state & (1 << state)) {
		hwlog_info("%s: last_hotplug_state=0x%x, state=%d, skip\n",
			__func__, priv->last_hotplug_state, state);
		return;
	}

	old_state = priv->hotplug_state;
	priv->hotplug_state |= (1 << state);
	hwlog_info("%s: last_state=0x%x, hotplug_state old=0x%x, new=0x%x\n",
		__func__, priv->last_hotplug_state,
		old_state, priv->hotplug_state);
}
EXPORT_SYMBOL_GPL(dp_set_hotplug_state);

static void dp_report_hotplug_fail_state(struct dp_dsm_priv *priv)
{
	bool mmie_flag = false;

	// for factory MMIE test
	if (dp_factory_mode_is_enable()) {
		if (DP_CHECK_LINK_STATE(priv, DP_LINK_STATE_EDID_FAILED)) {
			// 1024*768
			DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_EDID_FAILED);
			mmie_flag = true;
		} else if (DP_CHECK_LINK_STATE(priv, DP_LINK_STATE_SAFE_MODE) &&
			(priv->hotplug_width == DP_SAFE_MODE_DISPLAY_WIDTH) &&
			(priv->hotplug_high == DP_SAFE_MODE_DISPLAY_HIGH)) {
			// safe mode, 640*480
			DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_SAFE_MODE);
			mmie_flag = true;
		} else if (DP_CHECK_LINK_STATE(priv,
			DP_LINK_STATE_INVALID_COMBINATIONS)) {
			DP_REPORT_LINK_STATE(priv,
				DP_LINK_STATE_INVALID_COMBINATIONS);
			mmie_flag = true;
		} else if (DP_CHECK_LINK_STATE(priv,
			DP_LINK_STATE_LINK_REDUCE_RATE)) {
			DP_REPORT_LINK_STATE(priv,
				DP_LINK_STATE_LINK_REDUCE_RATE);
			mmie_flag = true;
		}
	}

	if (mmie_flag) {
		hwlog_info("%s: report state for mmie\n", __func__);
	} else if (DP_CHECK_LINK_STATE(priv, DP_LINK_STATE_LINK_FAILED)) {
		DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_LINK_FAILED);
	} else if (DP_CHECK_LINK_STATE(priv, DP_LINK_STATE_AUX_FAILED)) {
		DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_AUX_FAILED);
	} else {
		hwlog_info("%s: hotplug failed, hotplug_state=0x%x, skip\n",
			__func__, priv->hotplug_state);
	}
}

static void dp_report_hotplug_success_state(struct dp_dsm_priv *priv)
{
	if (DP_CHECK_LINK_STATE(priv, DP_LINK_STATE_EDID_FAILED)) {
		// 1024*768
		DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_EDID_FAILED);
	} else if (DP_CHECK_LINK_STATE(priv, DP_LINK_STATE_SAFE_MODE) &&
		(priv->hotplug_width == DP_SAFE_MODE_DISPLAY_WIDTH) &&
		(priv->hotplug_high == DP_SAFE_MODE_DISPLAY_HIGH)) {
		// safe mode, 640*480
		DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_SAFE_MODE);
	} else {
		hwlog_info("%s: hotplug success, hotplug_state=0x%x, skip\n",
			__func__, priv->hotplug_state);
	}
}

static void dp_report_hotplug_state(struct dp_dsm_priv *priv)
{
	hwlog_info("%s: hotplug_state=0x%x\n", __func__, priv->hotplug_state);
	// hotplug_retval < 0 means hotplug failed, otherwise hotplug success
	if (priv->hotplug_retval < 0)
		dp_report_hotplug_fail_state(priv);
	else
		dp_report_hotplug_success_state(priv);

	priv->hotplug_state = 0;
	dp_factory_set_link_event_no(priv->last_hotplug_state,
		false, NULL, priv->hotplug_retval);
}

static int dp_add_info_to_imonitor_list(enum dp_imonitor_type type,
	struct dp_dsm_priv *data)
{
	struct dp_dsm_priv *priv = NULL;
	struct dp_hotplug_info *hotplug = NULL;
	int ret = 0;

	if (data == NULL) {
		hwlog_err("%s: data is NULL\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&(g_imonitor_report.lock));
	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	hotplug = kzalloc(sizeof(*hotplug), GFP_KERNEL);
	if ((priv == NULL) || (hotplug == NULL)) {
		hwlog_err("%s: priv or hotplug is NULL\n", __func__);
		ret = -ENOMEM;
		goto err_out;
	}

	memcpy(priv, data, sizeof(*priv));
	switch (type) {
	case DP_IMONITOR_BASIC_INFO:
	case DP_IMONITOR_EXTEND_INFO:
	case DP_IMONITOR_LINK_TRAINING:
		if (data->cur_hotplug_info != NULL) {
			memcpy(hotplug, data->cur_hotplug_info, sizeof(*hotplug));
		} else {
			hwlog_err("%s: cur_hotplug_info is NULL\n", __func__);
			ret = -EFAULT;
			goto err_out;
		}
		break;
	case DP_IMONITOR_TIME_INFO:
	case DP_IMONITOR_HPD:
	case DP_IMONITOR_HOTPLUG:
	case DP_IMONITOR_HDCP:
	default:
		kfree(hotplug);
		hotplug = NULL;
		break;
	}

	priv->type = type;
	priv->cur_hotplug_info = hotplug;
	list_add_tail(&(priv->list), &(g_imonitor_report.list_head));

err_out:
	if (ret < 0) {
		if (priv != NULL)
			kfree(priv);

		if (hotplug != NULL)
			kfree(hotplug);
	}
	mutex_unlock(&(g_imonitor_report.lock));

	return ret;
}

static void dp_imonitor_report_info_one(struct dp_dsm_priv *priv)
{
	struct dp_imonitor_report_info *report = &g_imonitor_report;
	int count = report->report_num[priv->type];

	if (count < DP_DSM_REPORT_NUM_IN_ONE_DAY) {
		hwlog_info("%s: imonitor report %d %d/%d\n",
			__func__, priv->type, count + 1,
			DP_DSM_REPORT_NUM_IN_ONE_DAY);
		dp_imonitor_report(priv->type, priv);
	} else {
		hwlog_info("%s: imonitor report %d %d/%d, skip\n",
			__func__, priv->type, count + 1,
			DP_DSM_REPORT_NUM_IN_ONE_DAY);

		// record the key information of the last day
		report->report_skip_existed = true;
		report->last_report_num[priv->type]++;

		if (priv->type == DP_IMONITOR_TIME_INFO) {
			DP_DSM_GET_LINK_MIN_TIME(report->last_link_min_time,
				priv->link_min_time);
			DP_DSM_GET_LINK_MAX_TIME(report->last_link_max_time,
				priv->link_max_time);

			report->last_same_mode_time.tv_sec +=
				priv->same_mode_time.tv_sec;
			report->last_diff_mode_time.tv_sec +=
				priv->diff_mode_time.tv_sec;

			report->last_source_switch_num +=
				priv->source_switch_num;
		}
	}

	report->report_num[priv->type]++;
}

static void dp_imonitor_report_work(struct work_struct *work)
{
	struct dp_imonitor_report_info *report = NULL;
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct dp_dsm_priv *priv = NULL;
	int list_count = 0;

	report = container_of(work, struct dp_imonitor_report_info,
		imonitor_work.work);

	mutex_lock(&(report->lock));
	if (!time_is_after_jiffies(report->report_jiffies)) {
		memset(report->report_num, 0,
			sizeof(uint32_t) * DP_IMONITOR_TYPE_NUM);
		report->report_jiffies = jiffies +
			msecs_to_jiffies(DP_DSM_REPORT_TIME_INTERVAL);
	}

	list_for_each_safe(pos, q, &(report->list_head)) {
		priv = list_entry(pos, struct dp_dsm_priv, list);
		list_del(pos);
		list_count++;

		dp_imonitor_report_info_one(priv);
		DP_DSM_FREE_HOTPLUG_PTR(priv);
		kfree(priv);
	}
	hwlog_info("%s: imonitor report success %d\n", __func__, list_count);
	mutex_unlock(&(report->lock));
}

static void dp_hpd_check_work(struct work_struct *work)
{
	struct dp_imonitor_report_info *report = NULL;
	struct dp_dsm_priv *priv = g_dp_dsm_priv;

	if (priv == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return;
	}

	report = container_of(work,
		struct dp_imonitor_report_info, hpd_work.work);
	if (!report->hpd_existed) {
		hwlog_err("%s: hpd not existed\n", __func__);
		priv->hotplug_retval = -DP_DSM_ERRNO_HPD_NOT_EXISTED;
		DP_REPORT_LINK_STATE(priv, DP_LINK_STATE_HPD_NOT_EXISTED);
		dp_factory_set_link_event_no(priv->last_hotplug_state,
			false, NULL, priv->hotplug_retval);
	}
}

static void dp_clear_timing_info(struct dp_dsm_priv *priv)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct dp_timing_info *timing = NULL;

	list_for_each_safe(pos, q, &(priv->timing_list_head)) {
		timing = list_entry(pos, struct dp_timing_info, list);
		list_del(pos);
		kfree(timing);
	}
}

static void dp_clear_priv_data(struct dp_dsm_priv *priv)
{
	struct list_head *pos = NULL;
	struct list_head *q = NULL;
	struct dp_connected_event *event = NULL;
	struct dp_hotplug_info *hotplug = NULL;

	list_for_each_safe(pos, q, &(priv->event_list_head)) {
		event = list_entry(pos, struct dp_connected_event, list);
		list_del(pos);
		kfree(event);
	}

	list_for_each_safe(pos, q, &(priv->hotplug_list_head)) {
		hotplug = list_entry(pos, struct dp_hotplug_info, list);
		list_del(pos);
		kfree(hotplug);
	}
	dp_clear_timing_info(priv);

#ifndef DP_DEBUG_ENABLE
	DP_DSM_FREE_HOTPLUG_PTR(priv);
#endif
}

static void dp_reinit_priv_data(struct dp_dsm_priv *priv)
{
	memset(priv, 0, sizeof(*priv));
	priv->rcf_basic_info_of_same_mode = true;
	priv->rcf_basic_info_of_diff_mode = true;
	priv->rcf_extend_info = true;
	priv->rcf_link_training_info = true;
	priv->rcf_link_retraining_info = true;
	priv->rcf_hotplug_info = true;
	priv->rcf_hdcp_info = true;

	INIT_LIST_HEAD(&(priv->event_list_head));
	INIT_LIST_HEAD(&(priv->hotplug_list_head));
	INIT_LIST_HEAD(&(priv->timing_list_head));
}

static void dp_add_hotplug_info_to_list(struct dp_dsm_priv *priv, bool stop)
{
	struct dp_hotplug_info *hotplug = NULL;
	int start_index = DP_PARAM_TIME_START - DP_PARAM_TIME;
	int stop_index = DP_PARAM_TIME_STOP - DP_PARAM_TIME;
	int link_succ_index = DP_PARAM_TIME_LINK_SUCC - DP_PARAM_TIME;

	if ((priv->cur_hotplug_info == NULL) || !priv->need_add_hotplug_to_list)
		return;
	hotplug = priv->cur_hotplug_info;
	priv->need_add_hotplug_to_list = false;

	// stop time, no hotunplug
	if (!stop) {
		hotplug->no_hotunplug = true;
		do_gettimeofday(&(hotplug->time[stop_index]));
	}

	hotplug->hotplug_retval = priv->hotplug_retval;
#ifdef DP_DEBUG_ENABLE
	list_add_tail(&(hotplug->list), &(priv->hotplug_list_head));
#endif

	if (priv->hotplug_retval != 0)
		return;

	if (stop) {
		struct timeval sec;

		// same or diff mode time
		sec.tv_sec = hotplug->time[stop_index].tv_sec -
			hotplug->time[link_succ_index].tv_sec;
		if (hotplug->source_mode)
			priv->same_mode_time.tv_sec += sec.tv_sec;
		else
			priv->diff_mode_time.tv_sec += sec.tv_sec;

		// link succ time
		sec.tv_sec = hotplug->time[link_succ_index].tv_sec -
			hotplug->time[start_index].tv_sec;
		sec.tv_usec = hotplug->time[link_succ_index].tv_usec -
			hotplug->time[start_index].tv_usec;
		sec.tv_usec = (sec.tv_sec * DP_DSM_TIME_MS_OF_SEC) +
			(sec.tv_usec / DP_DSM_TIME_US_OF_MS);

		DP_DSM_GET_LINK_MIN_TIME(priv->link_min_time, sec);
		DP_DSM_GET_LINK_MAX_TIME(priv->link_max_time, sec);
	}

	if (hotplug->read_edid_retval != 0)
		return;

#ifdef DP_DEBUG_ENABLE
	// invalid edid
	if ((hotplug->edid_check.u32.header_flag != EDID_CHECK_SUCCESS) ||
	    (hotplug->edid_check.u32.checksum_flag != EDID_CHECK_SUCCESS) ||
	    (hotplug->edid_check.u32.ext_blocks > DP_DSM_EDID_EXT_BLOCKS_MAX))
		dp_add_info_to_imonitor_list(DP_IMONITOR_EXTEND_INFO, priv);
	else // valid edid
#endif
		DP_DSM_ADD_TO_IMONITOR_LIST(priv,
			rcf_extend_info, DP_IMONITOR_EXTEND_INFO);
}

static bool dp_connected_by_typec(uint8_t type)
{
	if ((type == TCPC_DP) || (type == TCPC_USB31_AND_DP_2LINE))
		return true;

	return false;
}

#ifdef DP_DEBUG_ENABLE
static void dp_set_typec_in_out_time(struct dp_dsm_priv *priv, bool in)
{
	int index = g_typec_in_out.in_out_num % DP_DSM_TYPEC_IN_OUT_NUM;

	if (in) {
		do_gettimeofday(&(priv->tpyec_in_time));
		g_typec_in_out.in_time[index] = priv->tpyec_in_time;
	} else { // out
		do_gettimeofday(&(priv->tpyec_out_time));
		g_typec_in_out.out_time[index] = priv->tpyec_out_time;
		g_typec_in_out.in_out_num++;
	}
}
#endif

static void dp_typec_cable_in(struct dp_dsm_priv *priv, uint8_t mode_type)
{
	struct dp_imonitor_report_info *report = &g_imonitor_report;

	hwlog_info("typec cable in\n");
	dp_link_state_event(DP_LINK_STATE_CABLE_IN);
	dp_factory_set_link_event_no(0, true, NULL, 0);
	dp_clear_priv_data(priv);
	dp_reinit_priv_data(priv);
#ifdef DP_DEBUG_ENABLE
	dp_set_typec_in_out_time(priv, true);
#else
	do_gettimeofday(&(priv->tpyec_in_time));
#endif
	priv->tpyec_cable_type = mode_type;

	// delayed work to check whether hpd existed or not
	if (dp_factory_mode_is_enable()) {
		if ((report->hpd_jiffies == 0) ||
			!time_is_after_jiffies(report->hpd_jiffies)) {
			cancel_delayed_work_sync(&(report->hpd_work));
			report->hpd_jiffies = 0;
			report->hpd_existed = false;
			schedule_delayed_work(&(report->hpd_work),
				msecs_to_jiffies(DP_DSM_HPD_DELAYED_TIME));
		}
	}
}

static void dp_typec_cable_out(struct dp_dsm_priv *priv)
{
	bool hpd_report = false;

	hwlog_info("typec cable out\n");
	dp_link_state_event(DP_LINK_STATE_CABLE_OUT);
	if (dp_factory_mode_is_enable())
		cancel_delayed_work_sync(&(g_imonitor_report.hpd_work));
#ifdef DP_DEBUG_ENABLE
	dp_set_typec_in_out_time(priv, false);
#else
	do_gettimeofday(&(priv->tpyec_out_time));
#endif
	// if hotunplug not existed
	dp_add_hotplug_info_to_list(priv, false);

	// dp in event(>1) after typec cable in
	if (priv->dp_in_num > 1) {
		int delta = 0;

		if (priv->source_switch_num > 0)
			delta = (priv->dp_in_num > priv->source_switch_num) ?
				(priv->dp_in_num - priv->source_switch_num) : 0;
		else
			delta = priv->dp_in_num;

		if (delta > 1) {
			hpd_report = true;
			dp_add_info_to_imonitor_list(DP_IMONITOR_HPD, priv);
		}
	}

	// dp time info report
	if (!hpd_report && (priv->hotplug_retval == 0) &&
	   ((priv->same_mode_time.tv_sec > 0) ||
	   (priv->diff_mode_time.tv_sec > 0)))
		dp_add_info_to_imonitor_list(DP_IMONITOR_TIME_INFO, priv);

	// delayed work to avoid report info in irq handler
	cancel_delayed_work_sync(&(g_imonitor_report.imonitor_work));
	schedule_delayed_work(&(g_imonitor_report.imonitor_work),
		msecs_to_jiffies(DP_DSM_IMONITOR_DELAYED_TIME));
}

static bool dp_is_irq_hpd(uint8_t type)
{
	if ((type == TCA_IRQ_HPD_OUT) || (type == TCA_IRQ_HPD_IN))
		return true;

	return false;
}

static void dp_set_first_or_second_dev_type(struct dp_dsm_priv *priv,
	uint8_t dev_type)
{
	int index = priv->irq_hpd_num % DP_DSM_TCA_DEV_TYPE_NUM;

	priv->tca_dev_type[index] = dev_type;
	priv->irq_hpd_num++;
}

static void dp_set_irq_info(struct dp_dsm_priv *priv,
	uint8_t irq_type, uint8_t dev_type)
{
	priv->event_num++;
	if (!dp_is_irq_hpd(irq_type)) {
		priv->irq_short_num++;
		return;
	}

	if (dev_type == TCA_DP_IN) { // hotplug
		priv->dp_in_num++;
		// report hpd repeatedly from PD module
		// 1. only IN event: reached six times, and then report
		// 2. IN/OUT event: the total number reached six times
		priv->hpd_repeated_num++;
		if (priv->hpd_repeated_num >= DP_HPD_REPEATED_THRESHOLD) {
			if (!dp_factory_mode_is_enable())
				dp_link_state_event(DP_LINK_STATE_MULTI_HPD);
			priv->hpd_repeated_num = 0;
		}

		dp_set_first_or_second_dev_type(priv, dev_type);
	} else if (dev_type == TCA_DP_OUT) { // hotunplug
		priv->dp_out_num++;
		// If the first time is pulled out, not counting
		if (priv->hpd_repeated_num > 0)
			priv->hpd_repeated_num++;
		dp_set_first_or_second_dev_type(priv, dev_type);
	}

}

void dp_imonitor_set_pd_event(uint8_t irq_type, uint8_t cur_mode,
	uint8_t mode_type, uint8_t dev_type, uint8_t typec_orien)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;
#ifdef DP_DEBUG_ENABLE
	struct dp_connected_event *event = NULL;
#endif

	// not dp cable in
	if (!dp_connected_by_typec(cur_mode) &&
	   !dp_connected_by_typec(mode_type))
		return;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	// dp cable in, start
	if (!dp_connected_by_typec(cur_mode) &&
	   dp_connected_by_typec(mode_type))
		dp_typec_cable_in(priv, mode_type);
	else
		g_imonitor_report.hpd_existed = true;
	dp_set_irq_info(priv, irq_type, dev_type);

#ifdef DP_DEBUG_ENABLE
	event = kzalloc(sizeof(*event), GFP_KERNEL);
	if (event == NULL)
		return;

	do_gettimeofday(&(event->time));
	event->irq_type = (enum tca_irq_type)irq_type;
	event->cur_mode = (enum tcpc_mux_ctrl_type)cur_mode;
	event->mode_type = (enum tcpc_mux_ctrl_type)mode_type;
	event->dev_type = (enum tca_device_type)dev_type;
	event->typec_orien = (enum typec_plug_orien)typec_orien;

	list_add_tail(&(event->list), &(priv->event_list_head));
#endif
	// dp cable out, stop
	if (dp_connected_by_typec(cur_mode) &&
	   !dp_connected_by_typec(mode_type))
		dp_typec_cable_out(priv);
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_pd_event);

static void dp_set_edid_version(struct dp_hotplug_info *hotplug,
	uint8_t *edid)
{
	uint8_t version = edid[DP_DSM_EDID_VERSION_OFFSET];
	uint8_t revision = edid[DP_DSM_EDID_REVISION_OFFSET];

	hotplug->edid_version = DP_DSM_EDID_VERSION_MAKE(version, revision);
#ifdef DP_DSM_DEBUG
	hwlog_info("edid_version is %x.%x\n", version, revision);
#endif
}

static void dp_set_manufacturer_id(struct dp_hotplug_info *hotplug,
	uint8_t *edid)
{
	uint16_t id;
	int index; // letter index, 1 ~ 26
	int i;

	// byte 8 and byte 9
	id = DP_DSM_MID_LETTER(edid[DP_DSM_MID_OFFSET],
		edid[DP_DSM_MID_OFFSET + 1]);
	if (id == 0) {
		hwlog_err("%s: id is 0, invalid params\n", __func__);
		return;
	}

	// If index = 0, the letter is '@'('A' + (-1)).
	for (i = 0; i < DP_DSM_MID_LETTER_NUM; i++) {
		index = id >> ((DP_DSM_MID_LETTER_NUM - i - 1) *
			DP_DSM_MID_LETTER_OFFSET);
		index &= DP_DSM_MID_LETTER_MASK;
		index -= 1; // letter start 1, 00001=A

		hotplug->mid[i] = 'A' + index;
	}
	hotplug->mid[i] = '\0';
#ifdef DP_DEBUG_ENABLE
	hwlog_info("manufacturer id is %s\n", hotplug->mid);
#endif
}

static void dp_set_monitor_info(struct dp_hotplug_info *hotplug,
	uint8_t *edid)
{
	unsigned char *block = NULL;
	int offset = EDID_DESC_MONITOR_OFFSET;
	int i, j;

	block = edid + EDID_DESC_OFFSET;
	for (i = 0; i < EDID_DESC_NUM; i++, block += EDID_DESC_SIZE) {
		if (block[EDID_DESC_MASK_OFFSET] == EDID_DESC_MONITOR_MASK) {
			// 00 00 00 fc 00 53 6b 79 20 4c 43 44 20 2a 2a 2a 0a 20
			for (j = offset; j < EDID_DESC_SIZE; j++) {
				if (block[j] == EDID_DESC_MONITOR_TAIL)
					break;
				hotplug->monitor[j - offset] = block[j];
			}
			hotplug->monitor[j - offset] = '\0';
			break;
		}
	}

#ifdef DP_DEBUG_ENABLE
	hwlog_info("monitor name is %s\n", hotplug->monitor);
#endif
}

static int dp_check_edid(struct dp_hotplug_info *hotplug,
	uint8_t *edid, int index)
{
	union dp_edid_check *check = &hotplug->edid_check;
	bool verify_header = true;
	uint32_t checksum = 0;
	uint8_t ext_blocks;
	int i;

	// verify (checksum == 0)
	for (i = 0; i < DP_DSM_EDID_BLOCK_SIZE; i++)
		checksum += edid[i];

	if (checksum & 0xFF) {
		check->u8.checksum_num[index]++;
		check->u8.checksum_flag[index] = EDID_CHECK_FAILED;
	} else {
		check->u8.checksum_flag[index] = EDID_CHECK_SUCCESS;
	}

	if (index == 0) {
		// verify header
		for (i = 0; i < DP_DSM_EDID_HEADER_SIZE; i++) {
			if (edid[i] != edid_v1_header[i]) {
				check->u8.header_num[index]++;
				verify_header = false;
				break;
			}
		}

		ext_blocks = edid[DP_DSM_EDID_EXT_BLOCKS_INDEX];
		check->u8.ext_blocks = ext_blocks;
		if (ext_blocks > DP_DSM_EDID_EXT_BLOCKS_MAX)
			check->u8.ext_blocks_num++;
	} else {
		// Check Extension Tag
		// Header Tag stored in the first uint8_t
		if (edid[0] != DP_DSM_EDID_EXT_HEADER) {
			check->u8.header_num[index]++;
			verify_header = false;
		}
	}

	if (verify_header)
		check->u8.header_flag[index] = EDID_CHECK_SUCCESS;
	else
		check->u8.header_flag[index] = EDID_CHECK_FAILED;

#ifdef DP_DSM_DEBUG
	hwlog_info("%d header_flag=0x%08x, checksum_flag=0x%08x\n",
		index, check->u32.header_flag,
		check->u32.checksum_flag);
#endif
	return 0;
}

static void dp_set_edid_block(struct dp_dsm_priv *priv,
	enum dp_imonitor_param param, uint8_t *data)
{
	struct dp_hotplug_info *hotplug = priv->cur_hotplug_info;
	uint8_t *edid = NULL;
	int index = (int)(param - DP_PARAM_EDID);
#ifdef DP_DSM_DEBUG
	int i;
#endif

	if (hotplug == NULL || data == NULL) {
		hwlog_err("%s: hotplug or data is NULL\n", __func__);
		return;
	}

	if ((index < 0) || (index >= (int)DP_PARAM_EDID_NUM)) {
		hwlog_err("%s: invalid index %d\n", __func__, index);
		return;
	}

	edid = hotplug->edid[index];
	dp_check_edid(hotplug, data, index);

	// edid block0
	if (index == 0) {
		hotplug->current_edid_num = 0;
		dp_clear_timing_info(priv);
#ifdef DP_DEBUG_ENABLE
		if (dp_debug_get_lanes_rate_force(&priv->lanes_force,
		    &priv->rate_force) == 0) {
			priv->need_lanes_force = true;
			priv->need_rate_force = true;
		}

		if (dp_debug_get_resolution_force(&priv->user_mode,
		    &priv->user_format) == 0)
			priv->need_resolution_force = true;
#endif

		// parse edid: version, mid, monitor
		dp_set_edid_version(hotplug, data);
		dp_set_manufacturer_id(hotplug, data);
		dp_set_monitor_info(hotplug, data);
	}

	memcpy(edid, data, DP_DSM_EDID_BLOCK_SIZE);
	hotplug->current_edid_num++;
#ifdef DP_DEBUG_ENABLE
	if (priv->need_resolution_force)
		data[DP_DSM_EDID_BLOCK_SIZE - 1] =
			~data[DP_DSM_EDID_BLOCK_SIZE - 1];
#endif

#ifdef DP_DSM_DEBUG
	for (i = 0; i < DP_DSM_EDID_BLOCK_SIZE; i += 16) {
		hwlog_info("EDID%d[%02x]:%02x %02x %02x %02x %02x %02x %02x "
			"%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			index, i,
			edid[i], edid[i + 1], edid[i + 2], edid[i + 3],
			edid[i + 4], edid[i + 5], edid[i + 6], edid[i + 7],
			edid[i + 8], edid[i + 9], edid[i + 10], edid[i + 11],
			edid[i + 12], edid[i + 13], edid[i + 14], edid[i + 15]);
	}
#endif
}

static void dp_set_dpcd_rx_caps(struct dp_dsm_priv *priv, uint8_t *rx_caps)
{
	struct dp_hotplug_info *hotplug = priv->cur_hotplug_info;
#ifdef DP_DSM_DEBUG
	int i;
#endif

	if (hotplug == NULL || rx_caps == NULL) {
		hwlog_err("%s: hotplug or rx_caps is NULL\n", __func__);
		return;
	}
	hotplug->dpcd_revision = rx_caps[0];

#ifdef DP_DSM_DEBUG
	hwlog_info("dpcd_revision is %x.%x\n",
		(hotplug->dpcd_revision >> DP_DSM_DPCD_REVISION_OFFSET) &
		DP_DSM_DPCD_REVISION_MASK,
		hotplug->dpcd_revision & DP_DSM_DPCD_REVISION_MASK);

	for (i = 0; i < DP_DSM_DPCD_RX_CAPS_SIZE; i += 16) {
		hwlog_info("DPCD[%02x]:%02x %02x %02x %02x %02x %02x %02x %02x "
			"%02x %02x %02x %02x %02x %02x %02x %02x\n",
			i, rx_caps[i], rx_caps[i + 1], rx_caps[i + 2],
			rx_caps[i + 3], rx_caps[i + 4], rx_caps[i + 5],
			rx_caps[i + 6], rx_caps[i + 7], rx_caps[i + 8],
			rx_caps[i + 9], rx_caps[i + 10], rx_caps[i + 11],
			rx_caps[i + 12], rx_caps[i + 13], rx_caps[i + 14],
			rx_caps[i + 15]);
	}
#endif
}

static void dp_set_link_tu(struct dp_hotplug_info *hotplug, int tu)
{
	if (tu >= DP_DSM_TU_FAIL)
		hotplug->tu_fail = tu;
	else
		hotplug->tu = tu;
}

static void dp_set_irq_vector(struct dp_dsm_priv *priv, uint8_t *vector)
{
	int index = 0;

	if (vector == NULL) {
		hwlog_err("%s: priv or vector is NULL\n", __func__);
		return;
	}

	index = priv->irq_vector_num % DP_DSM_IRQ_VECTOR_NUM;
	priv->irq_vector[index] = (*vector);
	priv->irq_vector_num++;
}

static void dp_set_source_switch(struct dp_dsm_priv *priv, int *source)
{
	int index = 0;

	if (source == NULL) {
		hwlog_err("%s: source is NULL\n", __func__);
		return;
	}

	index = priv->source_switch_num % DP_DSM_SOURCE_SWITCH_NUM;
	priv->source_switch[index] = (uint8_t)(*source);

	priv->source_switch_num++;
	priv->hpd_repeated_num = 0;
}

static void dp_set_param_time_info(struct dp_dsm_priv *priv,
	enum dp_imonitor_param param)
{
	int index = (int)(param - DP_PARAM_TIME);

	if ((index < 0) || (index >= (int)DP_PARAM_TIME_NUM)) {
		hwlog_err("%s: invalid index %d\n", __func__, index);
		return;
	}

	// hotplug, dp start
	if (param == DP_PARAM_TIME_START) {
		struct dp_hotplug_info *hotplug = NULL;

		// if hotunplug not existed
		dp_add_hotplug_info_to_list(priv, false);

#ifdef DP_DEBUG_ENABLE
		hotplug = kzalloc(sizeof(*hotplug), GFP_KERNEL);
#else
		if (priv->cur_hotplug_info == NULL) {
			hotplug = kzalloc(sizeof(*hotplug), GFP_KERNEL);
		} else {
			hotplug = priv->cur_hotplug_info;
			memset(hotplug, 0, sizeof(*hotplug));
		}
#endif
		if (hotplug == NULL) {
			hwlog_err("%s: kzalloc hotplug failed\n", __func__);
			return;
		}
		// init data
		priv->cur_source_mode = false; // false: diff source
		priv->dss_pwron_failed = false;
		priv->link_training_retval = 0;
		priv->hotplug_retval = 0;
#ifdef DP_DEBUG_ENABLE
		priv->need_lanes_force = false;
		priv->need_rate_force = false;
		priv->need_resolution_force = false;
#endif
		memset(&(priv->aux_rw), 0, sizeof(priv->aux_rw));

		hotplug->vs_force = DP_DSM_VS_PE_MASK;
		hotplug->pe_force = DP_DSM_VS_PE_MASK;

		priv->cur_hotplug_info = hotplug;
		priv->need_add_hotplug_to_list = true;

		priv->is_hotplug_running = true;
		priv->hotplug_state = 0;
		priv->hotplug_width = 0;
		priv->hotplug_high = 0;
	}

	if (priv->cur_hotplug_info != NULL)
		do_gettimeofday(&(priv->cur_hotplug_info->time[index]));
	// hotunplug, dp stop
	if (param == DP_PARAM_TIME_STOP)
		dp_add_hotplug_info_to_list(priv, true);
}

#ifdef DP_DEBUG_ENABLE
static void dp_reset_lanes_rate_force(struct dp_dsm_priv *priv,
	uint8_t *lanes_force, uint8_t *rate_force)
{
	if (priv->need_lanes_force && (lanes_force != NULL)) {
		priv->need_lanes_force = false;
		*lanes_force = priv->lanes_force;
		hwlog_info("%s: set lanes_force %d force\n", __func__,
			priv->lanes_force);
	}

	if (priv->need_rate_force && (rate_force != NULL)) {
		priv->need_rate_force = false;
		*rate_force = priv->rate_force;
		hwlog_info("%s: set rate_force %d force\n", __func__,
			priv->rate_force);
	}
}
#endif

static void dp_set_param_basic_info(struct dp_dsm_priv *priv,
	enum dp_imonitor_param param, void *data)
{
	struct dp_hotplug_info *hotplug = priv->cur_hotplug_info;

	if (hotplug == NULL || data == NULL) {
		hwlog_err("%s: hotplug or param is NULL\n", __func__);
		return;
	}

	switch (param) {
	case DP_PARAM_WIDTH:
		hotplug->width = *(uint16_t *)data;
		priv->hotplug_width = hotplug->width;
		break;
	case DP_PARAM_HIGH:
		hotplug->high = *(uint16_t *)data;
		priv->hotplug_high = hotplug->high;
		break;
	case DP_PARAM_MAX_WIDTH:
		hotplug->max_width = *(uint16_t *)data;
		break;
	case DP_PARAM_MAX_HIGH:
		hotplug->max_high = *(uint16_t *)data;
		break;
	case DP_PARAM_PIXEL_CLOCK:
		hotplug->pixel_clock = *(int *)data;
		break;
	case DP_PARAM_FPS:
		hotplug->fps = *(uint8_t *)data;
		break;
	case DP_PARAM_MAX_RATE:
		hotplug->max_rate = *(uint8_t *)data;
		break;
	case DP_PARAM_MAX_LANES:
		hotplug->max_lanes = *(uint8_t *)data;
		break;
	case DP_PARAM_LINK_RATE:
#ifdef DP_DEBUG_ENABLE
		dp_reset_lanes_rate_force(priv, NULL, data);
#endif
		hotplug->rate = *(uint8_t *)data;
		break;
	case DP_PARAM_LINK_LANES:
#ifdef DP_DEBUG_ENABLE
		dp_reset_lanes_rate_force(priv, data, NULL);
#endif
		hotplug->lanes = *(uint8_t *)data;
		if (priv->is_hotplug_running)
			dp_factory_set_link_rate_lanes(hotplug->rate,
				hotplug->lanes, hotplug->max_rate,
				hotplug->max_lanes);
		break;
	case DP_PARAM_TU:
		dp_set_link_tu(hotplug, *(int *)data);
		break;
	case DP_PARAM_SOURCE_MODE:
		priv->cur_source_mode = *(bool *)data;
		hotplug->source_mode = priv->cur_source_mode;
		break;
	case DP_PARAM_USER_MODE:
		hotplug->user_mode = *(uint8_t *)data;
		break;
	case DP_PARAM_USER_FORMAT:
		hotplug->user_format = *(uint8_t *)data;
		break;
	case DP_PARAM_BASIC_AUDIO:
		hotplug->basic_audio = *(uint8_t *)data;
		break;
	case DP_PARAM_SAFE_MODE:
		hotplug->safe_mode = *(bool *)data;
		dp_set_hotplug_state(DP_LINK_STATE_SAFE_MODE);
		break;
	case DP_PARAM_READ_EDID_FAILED:
		hotplug->read_edid_retval = *(int *)data;
		dp_set_hotplug_state(DP_LINK_STATE_EDID_FAILED);
		break;
	case DP_PARAM_LINK_TRAINING_FAILED:
		priv->link_training_retval = *(int *)data;
		hotplug->link_training_retval = priv->link_training_retval;
		if (priv->is_hotplug_running) {
			dp_set_hotplug_state(DP_LINK_STATE_LINK_FAILED);
		} else {
			// 1. link retraining failed
			// 2. aux rw failed
			if (!dp_factory_mode_is_enable())
				dp_link_state_event(DP_LINK_STATE_LINK_FAILED);
		}
		break;
	default:
		break;
	}
}

static void dp_set_param_extend_info(struct dp_dsm_priv *priv,
	enum dp_imonitor_param param, void *data)
{
	switch (param) {
	case DP_PARAM_EDID_BLOCK0:
	case DP_PARAM_EDID_BLOCK1:
	case DP_PARAM_EDID_BLOCK2:
	case DP_PARAM_EDID_BLOCK3:
		dp_set_edid_block(priv, param, data);
		break;
	case DP_PARAM_DPCD_RX_CAPS:
		dp_set_dpcd_rx_caps(priv, data);
		break;
	default:
		break;
	}
}

static void dp_set_param_hdcp(struct dp_dsm_priv *priv,
	enum dp_imonitor_param param, void *data)
{
	switch (param) {
	case DP_PARAM_HDCP_VERSION:
		if (data != NULL)
			priv->hdcp_version = *(uint8_t *)data;
		break;
	case DP_PARAM_HDCP_KEY_S:
		do_gettimeofday(&(priv->hdcp_time));
		priv->hdcp_key = HDCP_KEY_SUCCESS;
		break;
	case DP_PARAM_HDCP_KEY_F:
		do_gettimeofday(&(priv->hdcp_time));
		priv->hdcp_key = HDCP_KEY_FAILED;
		if (!dp_factory_mode_is_enable())
			dp_link_state_event(DP_LINK_STATE_HDCP_FAILED);
		// hdcp authentication failed
#ifndef DP_DEBUG_ENABLE
		DP_DSM_ADD_TO_IMONITOR_LIST(priv,
			rcf_hdcp_info, DP_IMONITOR_HDCP);
#else
		dp_add_info_to_imonitor_list(DP_IMONITOR_HDCP, priv);
#endif
		break;
	default:
		break;
	}
}

static void dp_set_hotplug(struct dp_dsm_priv *priv, int *retval)
{
	if (retval == NULL) {
		hwlog_err("%s: retval is NULL\n", __func__);
		return;
	}

	priv->is_hotplug_running = false;
	priv->hotplug_retval = (*retval);
	if (priv->dss_pwron_failed)
		priv->hotplug_retval = -DP_DSM_ERRNO_DSS_PWRON_FAILED;

	if (priv->hotplug_retval < 0) {
		// if hotplug failed, not process hotunplug
		dp_add_hotplug_info_to_list(priv, false);

		if (priv->link_training_retval < 0) {
			// link training failed
#ifndef DP_DEBUG_ENABLE
			DP_DSM_ADD_TO_IMONITOR_LIST(priv,
				rcf_link_training_info,
				DP_IMONITOR_LINK_TRAINING);
#else
			dp_add_info_to_imonitor_list(DP_IMONITOR_LINK_TRAINING,
				priv);
#endif
		} else {
			// hotplug failed
#ifndef DP_DEBUG_ENABLE
			DP_DSM_ADD_TO_IMONITOR_LIST(priv,
				rcf_hotplug_info, DP_IMONITOR_HOTPLUG);
#else
			dp_add_info_to_imonitor_list(DP_IMONITOR_HOTPLUG, priv);
#endif
		}
	} else {
		// link training success
		dp_set_param_time_info(priv, DP_PARAM_TIME_LINK_SUCC);

		// hotplug success
		if (priv->cur_source_mode) { // same mode
			DP_DSM_ADD_TO_IMONITOR_LIST(priv,
				rcf_basic_info_of_same_mode,
				DP_IMONITOR_BASIC_INFO);
		} else { // diff mode
			DP_DSM_ADD_TO_IMONITOR_LIST(priv,
				rcf_basic_info_of_diff_mode,
				DP_IMONITOR_BASIC_INFO);
		}
	}

	// report link failed event
	dp_report_hotplug_state(priv);
}

static void dp_set_link_retraining(struct dp_dsm_priv *priv, int *retval)
{
	if (retval == NULL) {
		hwlog_err("%s: retval is NULL\n", __func__);
		return;
	}

	priv->link_retraining_retval = (*retval);
	hwlog_info("%s: link retraining failed %d in dp_device_srs()\n",
		__func__, priv->link_retraining_retval);

	// dp link failed in func dp_device_srs()
	if (priv->link_retraining_retval < 0) {
		if (!dp_factory_mode_is_enable())
			dp_link_state_event(DP_LINK_STATE_LINK_FAILED);
#ifndef DP_DEBUG_ENABLE
		DP_DSM_ADD_TO_IMONITOR_LIST(priv,
			rcf_link_retraining_info, DP_IMONITOR_LINK_TRAINING);
#else
		dp_add_info_to_imonitor_list(DP_IMONITOR_LINK_TRAINING, priv);
#endif
	}
}

static void dp_set_param_diag(struct dp_dsm_priv *priv,
	enum dp_imonitor_param param, void *data)
{
	switch (param) {
	case DP_PARAM_LINK_RETRAINING:
		priv->link_retraining_num++;
		priv->link_training_retval = 0;
		// maybe cause user's doubts by occasional interruptions,
		// don't send event DP_LINK_STATE_LINK_RETRAINING
		break;
	case DP_PARAM_SAFE_MODE:
	case DP_PARAM_READ_EDID_FAILED:
	case DP_PARAM_LINK_TRAINING_FAILED:
		dp_set_param_basic_info(priv, param, data);
		break;
	case DP_PARAM_LINK_RETRAINING_FAILED:
		dp_set_link_retraining(priv, data);
		break;
	case DP_PARAM_HOTPLUG_RETVAL:
		dp_set_hotplug(priv, data);
		break;
	case DP_PARAM_IRQ_VECTOR:
		dp_set_irq_vector(priv, data);
		break;
	case DP_PARAM_SOURCE_SWITCH:
		dp_set_source_switch(priv, data);
		break;
	case DP_PARAM_DSS_PWRON_FAILED:
		priv->dss_pwron_failed = true;
		break;
	default:
		break;
	}
}

void dp_imonitor_set_param(enum dp_imonitor_param param, void *data)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	if (param <= DP_PARAM_TIME_MAX)
		dp_set_param_time_info(priv, param);
	else if ((param >= DP_PARAM_BASIC) && (param <= DP_PARAM_BASIC_MAX))
		dp_set_param_basic_info(priv, param, data);
	else if ((param >= DP_PARAM_EXTEND) && (param <= DP_PARAM_EXTEND_MAX))
		dp_set_param_extend_info(priv, param, data);
	else if ((param >= DP_PARAM_HDCP) && (param <= DP_PARAM_HDCP_MAX))
		dp_set_param_hdcp(priv, param, data);
	else if ((param >= DP_PARAM_DIAG) && (param <= DP_PARAM_DIAG_MAX))
		dp_set_param_diag(priv, param, data);
	else // other params
		hwlog_info("%s: unknown param(%d), skip\n", __func__, param);
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_param);

void dp_imonitor_set_param_aux_rw(bool rw, bool i2c,
	uint32_t addr, uint32_t len, int retval)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	do_gettimeofday(&(priv->aux_rw.time));

	priv->aux_rw.rw = rw;
	priv->aux_rw.i2c = i2c;
	priv->aux_rw.addr = addr;
	priv->aux_rw.len = len;
	priv->aux_rw.retval = retval;

	if (priv->is_hotplug_running)
		dp_set_hotplug_state(DP_LINK_STATE_AUX_FAILED);
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_param_aux_rw);

void dp_imonitor_set_param_timing(uint16_t h_active, uint16_t v_active,
	uint32_t pixel_clock, uint8_t vesa_id)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;
	struct dp_timing_info *timing = NULL;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	timing = kzalloc(sizeof(*timing), GFP_KERNEL);
	if (timing == NULL)
		return;

	timing->h_active = h_active;
	timing->v_active = v_active;
	timing->pixel_clock = pixel_clock;
	timing->vesa_id = vesa_id;
	list_add_tail(&(timing->list), &(priv->timing_list_head));
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_param_timing);

void dp_imonitor_set_param_err_count(uint16_t lane0_err, uint16_t lane1_err,
	uint16_t lane2_err, uint16_t lane3_err)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	// 15-bit value storing the symbol error count of Lane 0~3
	priv->lane0_err += lane0_err;
	priv->lane1_err += lane1_err;
	priv->lane2_err += lane2_err;
	priv->lane3_err += lane3_err;
	hwlog_info("%s: err_count(0x%x,0x%x,0x%x,0x%x) happened\n", __func__,
		priv->lane0_err, priv->lane1_err,
		priv->lane2_err, priv->lane3_err);
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_param_err_count);

// NOTE:
// 1. record vs and pe for imonitor
// 2. reset vs and pe force when to debug(eng build)
void dp_imonitor_set_param_vs_pe(int index, uint8_t *vs, uint8_t *pe)
{
	struct dp_dsm_priv *priv = g_dp_dsm_priv;
	struct dp_hotplug_info *hotplug = NULL;
#ifdef DP_DEBUG_ENABLE
	uint8_t vs_force = 0;
	uint8_t pe_force = 0;
#endif

	if ((priv == NULL) || (priv->cur_hotplug_info == NULL) ||
	   (vs == NULL) || (pe == NULL)) {
		hwlog_err("%s: priv, hotplug, vs or pe is NULL\n", __func__);
		return;
	}
	hotplug = priv->cur_hotplug_info;

	if ((index < 0) || (index >= (int)DP_DSM_VS_PE_NUM)) {
		hwlog_err("%s: invalid index %d\n", __func__, index);
		return;
	}

	hotplug->vp.vs_pe[index].vswing = *vs;
	hotplug->vp.vs_pe[index].preemp = *pe;

	// for debug: set vs or pe force
#ifdef DP_DEBUG_ENABLE
	if (dp_debug_get_vs_pe_force(&vs_force, &pe_force) == 0) {
		hotplug->vs_force = vs_force;
		hotplug->pe_force = pe_force;

		hotplug->vp_force.vs_pe[index].vswing = vs_force;
		hotplug->vp_force.vs_pe[index].preemp = pe_force;

		*vs = vs_force;
		*pe = pe_force;
		if (index == 0)
			hwlog_info("%s: set vs %d or pe %d force\n",
				__func__, vs_force, pe_force);
	}
#endif // DP_DEBUG_ENABLE
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_param_vs_pe);

// for debug: reset resolution force
void dp_imonitor_set_param_resolution(uint8_t *user_mode, uint8_t *user_format)
{
#ifndef DP_DEBUG_ENABLE
	UNUSED(user_mode);
	UNUSED(user_format);
#else
	struct dp_dsm_priv *priv = g_dp_dsm_priv;

	if (priv == NULL) {
		hwlog_err("%s: priv is NULL\n", __func__);
		return;
	}

	if (priv->need_resolution_force) {
		*user_mode = priv->user_mode;
		*user_format = priv->user_format;
		hwlog_info("%s: set user_mode %d and user_format %d force\n",
			__func__, priv->user_mode, priv->user_format);
	}
#endif // DP_DEBUG_ENABLE
}
EXPORT_SYMBOL_GPL(dp_imonitor_set_param_resolution);

static int dp_imonitor_basic_info(struct imonitor_eventobj *obj, void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	struct dp_hotplug_info *hotplug = priv->cur_hotplug_info;
	int ret = 0;

	if (hotplug == NULL)
		return -EINVAL;

	ret += DP_SET_PARAM_I(obj, "CableType", priv->tpyec_cable_type);

	ret += DP_SET_PARAM_S(obj, "Mid", hotplug->mid);
	ret += DP_SET_PARAM_S(obj, "Monitor", hotplug->monitor);
	ret += DP_SET_PARAM_I(obj, "Width", hotplug->width);
	ret += DP_SET_PARAM_I(obj, "High", hotplug->high);
	ret += DP_SET_PARAM_I(obj, "MaxWidth", hotplug->max_width);
	ret += DP_SET_PARAM_I(obj, "MaxHigh", hotplug->max_high);
	ret += DP_SET_PARAM_I(obj, "PixelClk", hotplug->pixel_clock);
	ret += DP_SET_PARAM_I(obj, "Fps", hotplug->fps);

	ret += DP_SET_PARAM_I(obj, "MaxRate", hotplug->max_rate);
	ret += DP_SET_PARAM_I(obj, "MaxLanes", hotplug->max_lanes);
	ret += DP_SET_PARAM_I(obj, "LinkRate", hotplug->rate);
	ret += DP_SET_PARAM_I(obj, "LinkLanes", hotplug->lanes);
	ret += DP_SET_PARAM_I(obj, "Tu", hotplug->tu);
	ret += DP_SET_PARAM_I(obj, "TuFail", hotplug->tu_fail);
	ret += DP_SET_PARAM_I(obj, "VsPe", hotplug->vp.vswing_preemp);

	ret += DP_SET_PARAM_I(obj, "SrcMode", hotplug->source_mode);
	ret += DP_SET_PARAM_I(obj, "UserMode", hotplug->user_mode);
	ret += DP_SET_PARAM_I(obj, "UserFmt", hotplug->user_format);
	ret += DP_SET_PARAM_I(obj, "Audio", hotplug->basic_audio);

	ret += DP_SET_PARAM_I(obj, "EdidVer", hotplug->edid_version);
	ret += DP_SET_PARAM_I(obj, "DpcdVer", hotplug->dpcd_revision);

	ret += DP_SET_PARAM_I(obj, "SafeMode", hotplug->safe_mode);
	ret += DP_SET_PARAM_I(obj, "RedidRet", hotplug->read_edid_retval);

	ret += DP_SET_PARAM_I(obj, "EdidNum", hotplug->current_edid_num);
	ret += DP_SET_PARAM_I(obj, "HeadNum",
		hotplug->edid_check.u32.header_num);
	ret += DP_SET_PARAM_I(obj, "ChksNum",
		hotplug->edid_check.u32.checksum_num);
	ret += DP_SET_PARAM_I(obj, "HeadFlag",
		hotplug->edid_check.u32.header_flag);
	ret += DP_SET_PARAM_I(obj, "ChksFlag",
		hotplug->edid_check.u32.checksum_flag);
	ret += DP_SET_PARAM_I(obj, "ExtBlkNum",
		hotplug->edid_check.u32.ext_blocks_num);
	ret += DP_SET_PARAM_I(obj, "ExtBlks",
		hotplug->edid_check.u32.ext_blocks);

	return ret;
}

static int dp_imonitor_time_info(struct imonitor_eventobj *obj,
				 void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	struct dp_imonitor_report_info *report = &g_imonitor_report;
	char in_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char out_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char link_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char same_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char diff_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char source_switch[DP_DSM_SOURCE_SWITCH_NUM + 1] = { 0 };
	int count = 0;
	int ret = 0;

	// typec in & out time
	dp_timeval_to_datetime_str(priv->tpyec_in_time,
		"tpyec_in", in_time, DP_DSM_DATETIME_SIZE);
	dp_timeval_to_datetime_str(priv->tpyec_out_time,
		"tpyec_out", out_time, DP_DSM_DATETIME_SIZE);
	priv->same_mode_time.tv_sec += DP_DSM_TIMEVAL_WITH_TZ;
	priv->diff_mode_time.tv_sec += DP_DSM_TIMEVAL_WITH_TZ;
	dp_timeval_to_datetime_str(priv->same_mode_time,
		"same_mode_time", same_time, DP_DSM_DATETIME_SIZE);
	dp_timeval_to_datetime_str(priv->diff_mode_time,
		"diff_mode_time", diff_time, DP_DSM_DATETIME_SIZE);
	priv->link_max_time.tv_sec =
		DP_DSM_TIME_MS_PERCENT(priv->link_max_time.tv_usec);
	priv->link_max_time.tv_sec += DP_DSM_TIMEVAL_WITH_TZ;
	dp_timeval_to_datetime_str(priv->link_max_time,
		"link_max_time", link_time, DP_DSM_DATETIME_SIZE);

	count = MIN(priv->source_switch_num, DP_DSM_SOURCE_SWITCH_NUM);
	dp_data_bin_to_str(source_switch, DP_DSM_SOURCE_SWITCH_NUM + 1,
		priv->source_switch, count, DP_DSM_BIN_TO_STR_SHIFT8);

	ret += DP_SET_PARAM_S(obj, "InTime", in_time);
	ret += DP_SET_PARAM_S(obj, "OutTime", out_time);
	ret += DP_SET_PARAM_S(obj, "LinkTime", link_time);
	ret += DP_SET_PARAM_S(obj, "SameTime", same_time);
	ret += DP_SET_PARAM_S(obj, "DiffTime", diff_time);

	ret += DP_SET_PARAM_I(obj, "SwNum", priv->source_switch_num);
	ret += DP_SET_PARAM_S(obj, "SrcSw", source_switch);

	// the key information of the last day
	if (report->report_skip_existed) {
		int i;

		for (i = 0; i < DP_IMONITOR_TYPE_NUM; i++) {
			hwlog_info("%s: last_report_num[%d]=%u\n", __func__,
				i, report->last_report_num[i]);
		}
		hwlog_info("%s: last_link_min_time=%ld\n", __func__,
			report->last_link_min_time.tv_usec);
		hwlog_info("%s: last_link_max_time=%ld\n", __func__,
			report->last_link_max_time.tv_usec);
		hwlog_info("%s: last_same_mode_time=%ld\n", __func__,
			report->last_same_mode_time.tv_sec);
		hwlog_info("%s: last_diff_mode_time=%ld\n", __func__,
			report->last_diff_mode_time.tv_sec);
		hwlog_info("%s: last_source_switch_num=%d\n", __func__,
			report->last_source_switch_num);

		report->report_skip_existed = false;
		memset(report->last_report_num, 0,
			sizeof(uint32_t) * DP_IMONITOR_TYPE_NUM);
	}

	return ret;
}

static int dp_imonitor_extend_info(struct imonitor_eventobj *obj, void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	struct dp_hotplug_info *hotplug = priv->cur_hotplug_info;
	char edid[DP_DSM_EDID_BLOCK_SIZE * DP_DSM_BIN_TO_STR_TIMES + 1] = { 0 };
	char param[DP_DSM_PARAM_NAME_MAX] = { 0 };
	int ret = 0;
	int i = 0;

	if (hotplug == NULL)
		return -EINVAL;

	ret += DP_SET_PARAM_I(obj, "EdidNum", hotplug->current_edid_num);
	ret += DP_SET_PARAM_I(obj, "HeadFlag",
		hotplug->edid_check.u32.header_flag);
	ret += DP_SET_PARAM_I(obj, "ChksFlag",
		hotplug->edid_check.u32.checksum_flag);

	// dp edid block info
	for (i = 0; i < hotplug->current_edid_num; i++) {
		dp_data_bin_to_str(edid,
			DP_DSM_EDID_BLOCK_SIZE * DP_DSM_BIN_TO_STR_TIMES + 1,
			hotplug->edid[i], DP_DSM_EDID_BLOCK_SIZE,
			DP_DSM_BIN_TO_STR_SHIFT4);

		snprintf(param, (unsigned long)DP_DSM_PARAM_NAME_MAX,
			"EdidBlk%d", i);
		ret += DP_SET_PARAM_S(obj, param, edid);
	}

	return ret;
}

static int dp_imonitor_hpd_info(struct imonitor_eventobj *obj, void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	char in_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char out_time[DP_DSM_DATETIME_SIZE] = { 0 };
	char dev_type[DP_DSM_TCA_DEV_TYPE_NUM + 1] = { 0 };
	char irq_vector[DP_DSM_IRQ_VECTOR_NUM + 1] = { 0 };
	char source_switch[DP_DSM_SOURCE_SWITCH_NUM + 1] = { 0 };
	int count = 0;
	int ret = 0;

	dp_timeval_to_datetime_str(priv->tpyec_in_time,
		"tpyec_in", in_time, DP_DSM_DATETIME_SIZE);
	dp_timeval_to_datetime_str(priv->tpyec_out_time,
		"tpyec_out", out_time, DP_DSM_DATETIME_SIZE);

	count = MIN(priv->irq_hpd_num, DP_DSM_TCA_DEV_TYPE_NUM);
	dp_data_bin_to_str(dev_type, DP_DSM_TCA_DEV_TYPE_NUM + 1,
		priv->tca_dev_type, count, DP_DSM_BIN_TO_STR_SHIFT8);

	count = MIN(priv->irq_vector_num, DP_DSM_IRQ_VECTOR_NUM);
	dp_data_bin_to_str(irq_vector, DP_DSM_IRQ_VECTOR_NUM + 1,
		priv->irq_vector, count, DP_DSM_BIN_TO_STR_SHIFT8);

	count = MIN(priv->source_switch_num, DP_DSM_SOURCE_SWITCH_NUM);
	dp_data_bin_to_str(source_switch, DP_DSM_SOURCE_SWITCH_NUM + 1,
		priv->source_switch, count, DP_DSM_BIN_TO_STR_SHIFT8);

	ret += DP_SET_PARAM_S(obj, "InTime", in_time);
	ret += DP_SET_PARAM_S(obj, "OutTime", out_time);

	ret += DP_SET_PARAM_I(obj, "EvtNum", priv->event_num);
	ret += DP_SET_PARAM_I(obj, "CableType", priv->tpyec_cable_type);
	ret += DP_SET_PARAM_I(obj, "HpdNum", priv->irq_hpd_num);
	ret += DP_SET_PARAM_I(obj, "IrqNum", priv->irq_short_num);
	ret += DP_SET_PARAM_I(obj, "DpInNum", priv->dp_in_num);
	ret += DP_SET_PARAM_I(obj, "DpOutNum", priv->dp_out_num);
	ret += DP_SET_PARAM_I(obj, "LinkNum", priv->link_retraining_num);
	ret += DP_SET_PARAM_S(obj, "DevType", dev_type);

	ret += DP_SET_PARAM_I(obj, "IrqVecNum", priv->irq_vector_num);
	ret += DP_SET_PARAM_S(obj, "IrqVec", irq_vector);
	ret += DP_SET_PARAM_I(obj, "SwNum", priv->source_switch_num);
	ret += DP_SET_PARAM_S(obj, "SrcSw", source_switch);

	return ret;
}

static int dp_imonitor_link_training_info(struct imonitor_eventobj *obj,
	void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	struct dp_hotplug_info *hotplug = priv->cur_hotplug_info;
	int ret = 0;

	if (hotplug == NULL)
		return -EINVAL;

	if ((hotplug->link_training_retval == 0) &&
	   (priv->link_retraining_retval < 0)) {
		hotplug->link_training_retval = priv->link_retraining_retval +
			(-DP_DSM_ERRNO_DEVICE_SRS_FAILED);
	}

	ret += DP_SET_PARAM_I(obj, "CableType", priv->tpyec_cable_type);
	ret += DP_SET_PARAM_I(obj, "MaxWidth", hotplug->max_width);
	ret += DP_SET_PARAM_I(obj, "MaxHigh", hotplug->max_high);
	ret += DP_SET_PARAM_I(obj, "PixelClk", hotplug->pixel_clock);
	ret += DP_SET_PARAM_I(obj, "MaxRate", hotplug->max_rate);
	ret += DP_SET_PARAM_I(obj, "MaxLanes", hotplug->max_lanes);
	ret += DP_SET_PARAM_I(obj, "LinkRate", hotplug->rate);
	ret += DP_SET_PARAM_I(obj, "LinkLanes", hotplug->lanes);
	ret += DP_SET_PARAM_I(obj, "VsPe", hotplug->vp.vswing_preemp);
	ret += DP_SET_PARAM_I(obj, "SafeMode", hotplug->safe_mode);
	ret += DP_SET_PARAM_I(obj, "RedidRet", hotplug->read_edid_retval);
	ret += DP_SET_PARAM_I(obj, "LinkRet", hotplug->link_training_retval);
	ret += DP_SET_PARAM_I(obj, "AuxRw", priv->aux_rw.rw);
	ret += DP_SET_PARAM_I(obj, "AuxI2c", priv->aux_rw.i2c);
	ret += DP_SET_PARAM_I(obj, "AuxAddr", priv->aux_rw.addr);
	ret += DP_SET_PARAM_I(obj, "AuxLen", priv->aux_rw.len);
	ret += DP_SET_PARAM_I(obj, "AuxRet", priv->aux_rw.retval);
	return ret;
}

static int dp_imonitor_hotplug_info(struct imonitor_eventobj *obj, void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	int ret = 0;

	// unknown error
	if (!(priv->aux_rw.retval < 0 ||
	    priv->hotplug_retval <= -DP_DSM_ERRNO_DSS_PWRON_FAILED))
		return -EFAULT;

	// priv->aux_rw.retval < 0: edid/dpcd read or write error
	ret += DP_SET_PARAM_I(obj, "HpRet", priv->hotplug_retval);
	ret += DP_SET_PARAM_I(obj, "AuxRw", priv->aux_rw.rw);
	ret += DP_SET_PARAM_I(obj, "AuxI2c", priv->aux_rw.i2c);
	ret += DP_SET_PARAM_I(obj, "AuxAddr", priv->aux_rw.addr);
	ret += DP_SET_PARAM_I(obj, "AuxLen", priv->aux_rw.len);
	ret += DP_SET_PARAM_I(obj, "AuxRet", priv->aux_rw.retval);
	return ret;
}

static int dp_imonitor_hdcp_info(struct imonitor_eventobj *obj, void *data)
{
	struct dp_dsm_priv *priv = DP_IMONITOR_REPORT_PRIV(g_dp_dsm_priv, data);
	int ret = 0;

	ret += DP_SET_PARAM_I(obj, "HdcpVer", priv->hdcp_version);
	ret += DP_SET_PARAM_I(obj, "HdcpKey", priv->hdcp_key);
	return ret;
}

static unsigned int dp_imonitor_get_event_id(enum dp_imonitor_type type,
	imonitor_prepare_param_cb_t *prepare)
{
	int count = ARRAY_SIZE(imonitor_event_id);
	unsigned int event_id = 0;
	int i;

	if (type >= DP_IMONITOR_TYPE_NUM)
		goto err_out;

	for (i = 0; i < count; i++) {
		if (imonitor_event_id[i].type == type) {
			event_id = imonitor_event_id[i].event_id;
			*prepare = imonitor_event_id[i].prepare_cb;
			break;
		}
	}

err_out:
	return event_id;
}

void dp_imonitor_report(enum dp_imonitor_type type, void *data)
{
	struct imonitor_eventobj *obj = NULL;
	imonitor_prepare_param_cb_t prepare = NULL;
	unsigned int event_id = 0;
	int ret = 0;

	hwlog_info("%s enter\n", __func__);
	event_id = dp_imonitor_get_event_id(type, &prepare);
	if ((event_id == 0) || (prepare == NULL)) {
		hwlog_err("%s: invalid type %d, event_id %u or prepare %pK\n",
			__func__, type, event_id, prepare);
		return;
	}

	obj = imonitor_create_eventobj(event_id);
	if (obj == NULL) {
		hwlog_err("%s: imonitor_create_eventobj %u failed\n",
			__func__, event_id);
		return;
	}

	ret = prepare(obj, data);
	if (ret < 0) {
		hwlog_info("%s: prepare param %u skip %d\n",
			__func__, event_id, ret);
		goto err_out;
	}

	ret = imonitor_send_event(obj);
	if (ret < 0) {
		hwlog_err("%s: imonitor_send_event %u failed %d\n",
			__func__, event_id, ret);
		goto err_out;
	}
	hwlog_info("%s event_id %u success\n", __func__, event_id);

err_out:
	if (obj != NULL)
		imonitor_destroy_eventobj(obj);
}
EXPORT_SYMBOL_GPL(dp_imonitor_report);

static int dp_dmd_get_error_no(enum dp_dmd_type type)
{
	int count = ARRAY_SIZE(dmd_error_no);
	int error_no = 0;
	int i;

	if (type >= DP_DMD_TYPE_NUM)
		goto err_out;

	for (i = 0; i < count; i++) {
		if (dmd_error_no[i].type == type) {
			error_no = dmd_error_no[i].error_no;
			break;
		}
	}

err_out:
	return error_no;
}

void dp_dmd_report(enum dp_dmd_type type, const char *fmt, ...)
{
	struct dsm_client *client = g_dp_dsm_client;
	char report_buf[DP_DSM_BUF_SIZE] = { 0 };
	va_list args;
	int error_no = 0;
	int ret = 0;

	hwlog_info("%s enter\n", __func__);
	if ((client == NULL) || (fmt == NULL)) {
		hwlog_err("%s: client or fmt is NULL\n", __func__);
		return;
	}

	error_no = dp_dmd_get_error_no(type);
	if (error_no <= 0) {
		hwlog_err("%s: invalid type %d or error_no %d\n",
			__func__, type, error_no);
		return;
	}

	va_start(args, fmt);
	ret = vsnprintf(report_buf, sizeof(report_buf) - 1, fmt, args);
	va_end(args);
	if (ret < 0) {
		hwlog_err("%s: vsnprintf failed %d\n", __func__, ret);
		return;
	}

	ret = dsm_client_ocuppy(client);
	if (!ret) {
		ret = dsm_client_record(client, report_buf);
		dsm_client_notify(client, error_no);
		hwlog_info("%s: report %d success %d\n",
			__func__, error_no, ret);
	} else {
		hwlog_info("%s: report %d skip %d\n", __func__, error_no, ret);
	}
}
EXPORT_SYMBOL_GPL(dp_dmd_report);

static void dp_dsm_release(struct dp_dsm_priv *priv, struct dsm_client *client)
{
	cancel_delayed_work_sync(&(g_imonitor_report.imonitor_work));
	cancel_delayed_work_sync(&(g_imonitor_report.hpd_work));

	if (client != NULL)
		dsm_unregister_client(client, &dp_dsm);

	if (priv != NULL) {
		dp_clear_priv_data(priv);
		kfree(priv);
	}
}

static int __init dp_dsm_init(void)
{
	struct dp_imonitor_report_info *report = &g_imonitor_report;
	struct dp_dsm_priv *priv = NULL;
	struct dsm_client *client = NULL;
	int ret = 0;

	hwlog_info("%s: enter\n", __func__);
	client = dsm_register_client(&dp_dsm);
	if (client == NULL) {
		client = dsm_find_client((char *)dp_dsm.name);
		if (client == NULL) {
			hwlog_err("%s:dsm_register_client failed\n", __func__);
			ret = -EFAULT;
			goto err_out;
		} else {
			hwlog_info("%s:dsm_find_client %pK\n",
				__func__, client);
		}
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	dp_reinit_priv_data(priv);
#ifdef DP_DEBUG_ENABLE
	memset(&g_typec_in_out, 0, sizeof(g_typec_in_out));
#endif

	// init report info
	memset(report, 0, sizeof(*report));
	INIT_LIST_HEAD(&(report->list_head));
	mutex_init(&(report->lock));

	INIT_DELAYED_WORK(&(report->imonitor_work), dp_imonitor_report_work);
	INIT_DELAYED_WORK(&(report->hpd_work), dp_hpd_check_work);
	report->hpd_jiffies = jiffies +
		msecs_to_jiffies(DP_DSM_HPD_TIME_INTERVAL);
	report->report_jiffies = jiffies +
		msecs_to_jiffies(DP_DSM_REPORT_TIME_INTERVAL);

	g_dp_dsm_priv = priv;
	g_dp_dsm_client = client;
	hwlog_info("%s: init success\n", __func__);
	return 0;

err_out:
	dp_dsm_release(priv, client);
	return ret;
}

static void __exit dp_dsm_exit(void)
{
	hwlog_info("%s: enter\n", __func__);
	dp_dsm_release(g_dp_dsm_priv, g_dp_dsm_client);
	g_dp_dsm_priv = NULL;
	g_dp_dsm_client = NULL;
}

module_init(dp_dsm_init);
module_exit(dp_dsm_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei dp dsm driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
#endif // DP_DSM_ENABLE
