/*
 * pd_dpm_pdo_select.c
 * PDO/APDO Select Driver
 *
 * Copyright (C) 2015 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#include <linux/kernel.h>

#include "include/pd_core.h"
#include "include/pd_tcpm.h"
#include "include/pd_dpm_pdo_select.h"
#include <securec.h>

#ifdef CONFIG_USB_POWER_DELIVERY_SUPPORT
struct dpm_select_info_t {
	uint8_t pos;
	int max_uw;
	int cur_mv;
	uint8_t policy;
};

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
static void dpm_extract_apdo_info(uint32_t pdo, struct dpm_pdo_info_t *info)
{
	switch (APDO_TYPE(pdo)) {
	case APDO_TYPE_PPS:
		info->apdo_type = DPM_APDO_TYPE_PPS;

		if (pdo & APDO_PPS_CURR_FOLDBACK)
			info->apdo_type |= DPM_APDO_TYPE_PPS_CF;

		info->ma = APDO_PPS_EXTRACT_CURR(pdo);
		info->vmin = APDO_PPS_EXTRACT_MIN_VOLT(pdo);
		info->vmax = APDO_PPS_EXTRACT_MAX_VOLT(pdo);
		info->uw = info->ma * info->vmax;
		return;

	default:
		D("Reserved\n");
	}

	info->type = TCPM_POWER_CAP_VAL_TYPE_UNKNOWN;
}
#endif

void hisi_dpm_extract_pdo_info(uint32_t pdo, struct dpm_pdo_info_t *info)
{
	if (memset_s(info, sizeof(struct dpm_pdo_info_t),
			0, sizeof(struct dpm_pdo_info_t)))
		E("clr pdo info\n");
	info->type = DPM_PDO_TYPE(pdo);

	switch (PDO_TYPE(pdo)) {
	case PDO_TYPE_FIXED:
		info->ma = PDO_FIXED_EXTRACT_CURR(pdo);
		info->vmax = PDO_FIXED_EXTRACT_VOLT(pdo);
		info->vmin = PDO_FIXED_EXTRACT_VOLT(pdo);
		info->uw = info->ma * info->vmax;
		break;
	case PDO_TYPE_VARIABLE:
		info->ma = PDO_VAR_EXTRACT_CURR(pdo);
		info->vmin = PDO_VAR_EXTRACT_MIN_VOLT(pdo);
		info->vmax = PDO_VAR_EXTRACT_MAX_VOLT(pdo);
		info->uw = info->ma * info->vmax;
		break;

	case PDO_TYPE_BATTERY:
		info->uw = PDO_BATT_EXTRACT_OP_POWER(pdo) * PDO_POWER_MW_2_UW;
		info->vmin = PDO_BATT_EXTRACT_MIN_VOLT(pdo);
		info->vmax = PDO_BATT_EXTRACT_MAX_VOLT(pdo);
		info->ma = info->uw / info->vmin;
		break;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	case PDO_TYPE_APDO:
		dpm_extract_apdo_info(pdo, info);
		break;
#endif

	default:
		E("Pdo type err\n");
		break;
	}
}

static int dpm_calc_src_cap_power_uw(const struct dpm_pdo_info_t *source,
		const struct dpm_pdo_info_t *sink)
{
	int uw, ma;

	if (source->type == DPM_PDO_TYPE_BAT) {
		uw = source->uw;

		if (sink->type == DPM_PDO_TYPE_BAT)
			uw = min_t(int, uw, sink->uw);
	} else {
		ma = source->ma;

		if (sink->type != DPM_PDO_TYPE_BAT)
			ma = min_t(int, ma, sink->ma);

		uw = ma * source->vmax;
	}

	return uw;
}

static bool dpm_select_pdo_from_vsafe5v(struct dpm_select_info_t *select_info,
		const struct dpm_pdo_info_t *sink,
		const struct dpm_pdo_info_t *source)
{
	int uw;

	if ((sink->vmax != TCPC_VBUS_SINK_5V) ||
		(sink->vmin != TCPC_VBUS_SINK_5V) ||
		(source->vmax != TCPC_VBUS_SINK_5V) ||
		(source->vmin != TCPC_VBUS_SINK_5V))
		return false;

	uw = dpm_calc_src_cap_power_uw(source, sink);
	if (uw > select_info->max_uw) {
		select_info->max_uw = uw;
		select_info->cur_mv = source->vmax;
		return true;
	}

	return false;
}

static bool dpm_is_valid_pdo_pair(const struct dpm_pdo_info_t *sink,
		const struct dpm_pdo_info_t *source, uint32_t policy)
{
	if (sink->vmax < source->vmax)
		return false;

	if (sink->vmin > source->vmin)
		return false;

	if (policy & DPM_CHARGING_POLICY_IGNORE_MISMATCH_CURR)
		return sink->ma <= source->ma;

	return true;
}

static bool dpm_select_pdo_from_max_power(struct dpm_select_info_t *select_info,
		const struct dpm_pdo_info_t *sink,
		const struct dpm_pdo_info_t *source)
{
	bool overload = false;
	int uw;

	if (!dpm_is_valid_pdo_pair(sink, source, select_info->policy))
		return false;

	uw = dpm_calc_src_cap_power_uw(source, sink);

	overload = (uw > select_info->max_uw);

	if ((!overload) && (uw == select_info->max_uw)) {
		if (select_info->policy &
			DPM_CHARGING_POLICY_PREFER_LOW_VOLTAGE)
			overload = (source->vmax < select_info->cur_mv);
		else if (select_info->policy &
			DPM_CHARGING_POLICY_PREFER_HIGH_VOLTAGE)
			overload = (source->vmax > select_info->cur_mv);
	}

	if (overload) {
		select_info->max_uw = uw;
		select_info->cur_mv = source->vmax;
		return true;
	}

	return false;
}

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
static bool dpm_select_pdo_from_pps(struct dpm_select_info_t *select_info,
		const struct dpm_pdo_info_t *sink,
		const struct dpm_pdo_info_t *source)
{
	bool overload = false;
	int uw, diff_mv;
	const int tolerance = 300;

	if (sink->type != DPM_PDO_TYPE_FIXED ||
			source->type != DPM_PDO_TYPE_APDO)
		return false;

	if (!(source->apdo_type & DPM_APDO_TYPE_PPS))
		return false;

	if (sink->vmax > source->vmax)
		return false;

	if (sink->vmin < source->vmin)
		return false;

	uw = sink->vmax * source->ma;
	diff_mv = source->vmax - sink->vmax;

	if (uw > select_info->max_uw)
		overload = true;
	else if (uw < select_info->max_uw)
		overload = false;
	else if ((select_info->cur_mv < tolerance) && (diff_mv > tolerance))
		overload = true;
	else if (diff_mv < select_info->cur_mv)
		overload = true;
	else
		overload = false;

	if (overload) {
		select_info->max_uw = uw;
		select_info->cur_mv = diff_mv;
	}

	return true;
}
#endif

typedef bool (*dpm_select_pdo_fun)(struct dpm_select_info_t *select_info,
		const struct dpm_pdo_info_t *sink,
		const struct dpm_pdo_info_t *source);

bool dpm_find_match_req_info(struct dpm_rdo_info_t *req_info,
		uint32_t snk_pdo, int cnt, uint32_t *src_pdos,
		int min_uw, uint8_t policy)
{
	int i;
	struct dpm_select_info_t select;
	struct dpm_pdo_info_t sink, source;
	dpm_select_pdo_fun select_pdo_fun;

	hisi_dpm_extract_pdo_info(snk_pdo, &sink);

	select.pos = 0;
	select.cur_mv = 0;
	select.max_uw = min_uw;
	select.policy = policy;

	switch (policy & DPM_CHARGING_POLICY_MASK) {
	case DPM_CHARGING_POLICY_MAX_POWER:
		select_pdo_fun = dpm_select_pdo_from_max_power;
		break;

	case DPM_CHARGING_POLICY_CUSTOM:
		select_pdo_fun = dpm_select_pdo_from_vsafe5v;
		break;

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
	case DPM_CHARGING_POLICY_PPS:
		select_pdo_fun = dpm_select_pdo_from_pps;
		break;
#endif

	default: /* DPM_CHARGING_POLICY_VSAFE5V */
		select_pdo_fun = dpm_select_pdo_from_vsafe5v;
		break;
	}

	for (i = 0; i < cnt; i++) {
		hisi_dpm_extract_pdo_info(src_pdos[i], &source);

		if (select_pdo_fun(&select, &sink, &source))
			select.pos = i + 1;
	}

	if (select.pos > 0) {
		hisi_dpm_extract_pdo_info(src_pdos[select.pos - 1], &source);

		req_info->pos = select.pos;
		req_info->type = source.type;
		req_info->vmax = source.vmax;
		req_info->vmin = source.vmin;

		if (sink.type == DPM_PDO_TYPE_BAT)
			req_info->mismatch = select.max_uw < sink.uw;
		else
			req_info->mismatch = source.ma < sink.ma;

		if (source.type == DPM_PDO_TYPE_BAT) {
			req_info->max_uw = sink.uw;
			req_info->oper_uw = select.max_uw;
		} else {
			req_info->max_ma = sink.ma;
			req_info->oper_ma = min_t(int, sink.ma, source.ma);
		}

#ifdef CONFIG_USB_PD_REV30_PPS_SINK
		if (source.type == DPM_PDO_TYPE_APDO) {
			req_info->vmax = sink.vmax;
			req_info->vmin = sink.vmin;
		}
#endif

		return true;
	}

	return false;
}
#endif /* CONFIG_USB_POWER_DELIVERY */
