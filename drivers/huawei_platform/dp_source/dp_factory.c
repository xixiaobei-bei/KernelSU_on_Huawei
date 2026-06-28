/* Copyright (c) 2008-2019, Huawei Tech. Co., Ltd. All rights reserved.
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
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/dp_source/dp_factory.h>
#include <huawei_platform/dp_source/dp_dsm.h>

#define HWLOG_TAG dp_factory
HWLOG_REGIST();

#ifndef DP_FACTORY_MODE_ENABLE // not defined
bool dp_factory_mode_is_enable(void)
{
	//hwlog_info("factory_mode disable!\n");
	return false;
}
EXPORT_SYMBOL_GPL(dp_factory_mode_is_enable);

void dp_factory_link_cr_or_ch_eq_fail(bool is_cr)
{
}
EXPORT_SYMBOL_GPL(dp_factory_link_cr_or_ch_eq_fail);

bool dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes, uint16_t h_active, uint16_t v_active, uint8_t fps)
{
	// not check 4k@60fps
	hwlog_info("%s: not check 4k@60fps, skip!\n", __func__);
	return true;
}
EXPORT_SYMBOL_GPL(dp_factory_is_4k_60fps);

int dp_factory_get_test_result(char *buffer, int size)
{
	if ((buffer != NULL) && (size > 0)) {
		return snprintf(buffer, (unsigned long)size, "not support factory test!\n");
	}

	return 0;
}
EXPORT_SYMBOL_GPL(dp_factory_get_test_result);

#else // DP_FACTORY_MODE_ENABLE defined

#define DP_FACTORY_RATE_HBR2 (2)
#define DP_FACTORY_LANES     (4)
#define DP_FACTORY_H_ACTIVE  (3840)
#define DP_FACTORY_V_ACTIVE  (2160)
#define DP_FACTORY_FPS       (60)

#define DP_FACTORY_CHECK_RET(a, b) (a), (((a) >= (b)) ? "ok" : "error")

typedef enum {
	DP_FACTORY_LINK_SUCC,
	DP_FACTORY_LINK_CR_FAILED,
	DP_FACTORY_LINK_CH_EQ_FAILED,
} dp_factory_link_state_t;

typedef struct
{
	bool     check_lanes_rate;
	bool     is_4k_60fps;;
	uint8_t  rate;
	uint8_t  lanes;
	uint16_t h_active;
	uint16_t v_active;
	uint8_t  fps;

	dp_factory_link_state_t link_state;
} dp_factory_priv_t;

dp_factory_priv_t *g_dp_factory_priv = NULL;

bool dp_factory_mode_is_enable(void)
{
	hwlog_info("factory_mode enable!\n");
	return true;
}
EXPORT_SYMBOL_GPL(dp_factory_mode_is_enable);

void dp_factory_link_cr_or_ch_eq_fail(bool is_cr)
{
	dp_factory_priv_t *priv = g_dp_factory_priv;

	if (priv != NULL) {
		if (is_cr) {
			priv->link_state = DP_FACTORY_LINK_CR_FAILED;
		} else {
			priv->link_state = DP_FACTORY_LINK_CH_EQ_FAILED;
		}
		dp_link_state_event(DP_LINK_STATE_LINK_REDUCE_RATE);
	}
}
EXPORT_SYMBOL_GPL(dp_factory_link_cr_or_ch_eq_fail);

bool dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes, uint16_t h_active, uint16_t v_active, uint8_t fps)
{
	dp_factory_priv_t *priv = g_dp_factory_priv;

	if (priv != NULL) {
		priv->rate     = rate;
		priv->lanes    = lanes;
		priv->h_active = h_active;
		priv->v_active = v_active;
		priv->fps      = fps;

		priv->link_state = DP_FACTORY_LINK_SUCC;
	}

	if ((rate < DP_FACTORY_RATE_HBR2) || (lanes < DP_FACTORY_LANES)
		|| (h_active < DP_FACTORY_H_ACTIVE) || (v_active < DP_FACTORY_V_ACTIVE)
		|| (fps < DP_FACTORY_FPS)) {
		hwlog_info("current link config, rate=%u, lanes=%u, h_active=%u, v_active=%u, fps=%u.\n",
			rate, lanes, h_active, v_active, fps);
		if (priv != NULL) {
			priv->is_4k_60fps = false;
		}

		// false: need to check external connection combinations
		// true:  not need to check
		if (priv->check_lanes_rate) {
			dp_link_state_event(DP_LINK_STATE_INVALID_COMBINATIONS);
			return false;
		} else {
			return true;
		}
	}

	if (priv != NULL) {
		priv->is_4k_60fps = true;
	}
	hwlog_info("%s: current dp config is 4k@60fps!\n", __func__);
	return true;
}
EXPORT_SYMBOL_GPL(dp_factory_is_4k_60fps);

int dp_factory_get_test_result(char *buffer, int size)
{
	dp_factory_priv_t *priv = g_dp_factory_priv;

	if ((priv != NULL) && (buffer != NULL) && (size > 0)) {
		if (DP_FACTORY_LINK_CR_FAILED == priv->link_state) {
			return snprintf(buffer, (unsigned long)size, "link cr failed!\n");
		}

		if (DP_FACTORY_LINK_CH_EQ_FAILED == priv->link_state) {
			return snprintf(buffer, (unsigned long)size, "link ch_eq failed!\n");
		}

		if ((0 == priv->rate) && (0 == priv->lanes)) {
			return snprintf(buffer, (unsigned long)size, "Not yet tested!\n");
		}

		return snprintf(buffer, (unsigned long)size, "test %s:\nrate=%u[%s]\nlanes=%u[%s]\nh_active=%u[%s]\nv_active=%u[%s]\nfps=%u[%s]\n",
			(priv->is_4k_60fps || !priv->check_lanes_rate) ? "success" : "failed",
			DP_FACTORY_CHECK_RET(priv->rate,     DP_FACTORY_RATE_HBR2),
			DP_FACTORY_CHECK_RET(priv->lanes,    DP_FACTORY_LANES),
			DP_FACTORY_CHECK_RET(priv->h_active, DP_FACTORY_H_ACTIVE),
			DP_FACTORY_CHECK_RET(priv->v_active, DP_FACTORY_V_ACTIVE),
			DP_FACTORY_CHECK_RET(priv->fps,      DP_FACTORY_FPS));
	}

	return 0;
}
EXPORT_SYMBOL_GPL(dp_factory_get_test_result);

static void dp_factory_parse_dts(dp_factory_priv_t *priv)
{
	struct device_node *np = NULL;

	np = of_find_compatible_node(NULL, NULL, "huawei,dp_source_switch");
	if (NULL == np) {
		hwlog_info("%s: dts node dp_source_switch not existed.\n", __func__);
		return;
	}

	if (!of_device_is_available(np)) {
		hwlog_info("%s: dts node dp_source_switch not available.\n", __func__);
		return;
	}

	if ((priv != NULL) && of_property_read_bool(np, "check_lanes_rate")) {
		priv->check_lanes_rate = true;
		hwlog_info("%s: need to check lanes and rate.\n", __func__);
	} else {
		hwlog_info("%s: not need to check lanes and rate.\n", __func__);
	}
}

static void dp_factory_release(dp_factory_priv_t *priv)
{
	if (priv != NULL) {
		kfree(priv);
	}
}

static int __init dp_factory_init(void)
{
	dp_factory_priv_t *priv = NULL;
	int ret = 0;

	hwlog_info("%s: enter...\n", __func__);
	priv = (dp_factory_priv_t *)kzalloc(sizeof(dp_factory_priv_t), GFP_KERNEL);
	if (priv == NULL) {
		hwlog_err("%s: kzalloc priv failed!!!\n", __func__);
		ret = -ENOMEM;
		goto err_out;
	}

	priv->check_lanes_rate = false;
	priv->is_4k_60fps = true;
	priv->link_state  = DP_FACTORY_LINK_SUCC;
	dp_factory_parse_dts(priv);

	g_dp_factory_priv = priv;
	hwlog_info("%s: init success!!!\n", __func__);
	return 0;

err_out:
	dp_factory_release(priv);
	return ret;
}

static void __exit dp_factory_exit(void)
{
	hwlog_info("%s: enter...\n", __func__);
	dp_factory_release(g_dp_factory_priv);
	g_dp_factory_priv = NULL;
}

module_init(dp_factory_init);
module_exit(dp_factory_exit);

#endif // DP_FACTORY_MODE_ENABLE end.

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei dp factory driver");
MODULE_AUTHOR("<wangping48@huawei.com>");

