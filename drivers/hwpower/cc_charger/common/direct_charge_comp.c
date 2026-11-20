/*
 * direct_charge_comp.c
 *
 * compensation parameter interface for direct charger
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/math64.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_common.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_comp.h>

#define HWLOG_TAG direct_charge_comp
HWLOG_REGIST();

void dc_get_vbat_comp_para(const char *ic_name, struct dc_comp_para *info)
{
	int i;

	if (!ic_name || !info || !info->vbat_comp_group_size)
		return;

	for (i = 0; i < info->vbat_comp_group_size; i++) {
		if (!strstr(ic_name, info->vbat_comp_para[i].ic_name))
			continue;

		memcpy(info->vbat_comp, info->vbat_comp_para[i].vbat_comp,
			sizeof(info->vbat_comp));

		hwlog_info("[%d]: ic_name=%s, vbat_comp_main=%d, vbat_comp_aux=%d\n",
			i, info->vbat_comp_para[i].ic_name,
			info->vbat_comp[0], info->vbat_comp[1]);
		return;
	}

	memcpy(info->vbat_comp, info->vbat_comp_para[0].vbat_comp,
		sizeof(info->vbat_comp));
	hwlog_info("ic_name=%s is invalid, vbat_comp_main=%d, vbat_comp_aux=%d\n",
		ic_name, info->vbat_comp[0], info->vbat_comp[1]);
}

static void dc_parse_vbat_comp_para(struct device_node *np,
	struct dc_comp_para *info)
{
	int i, row, col, len, idata;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"vbat_comp_para", DC_VBAT_COMP_PARA_MAX, DC_VBAT_COMP_TOTAL);
	if (len < 0) {
		info->vbat_comp_group_size = 0;
		return;
	}

	info->vbat_comp_group_size = len / DC_VBAT_COMP_TOTAL;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "vbat_comp_para", i, &tmp_string))
			return;

		row = i / DC_VBAT_COMP_TOTAL;
		col = i % DC_VBAT_COMP_TOTAL;
		switch (col) {
		case DC_IC_NAME:
			strncpy(info->vbat_comp_para[row].ic_name,
				tmp_string, DC_IC_NAME_LEN_MAX - 1);
			break;
		case DC_VBAT_COMP_VALUE_MAIN:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return;
			info->vbat_comp_para[row].vbat_comp[0] = idata;
			break;
		case DC_VBAT_COMP_VALUE_AUX:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return;
			info->vbat_comp_para[row].vbat_comp[1] = idata;
			break;
		default:
			break;
		}
	}

	for (i = 0; i < info->vbat_comp_group_size; i++)
		hwlog_info("ic_name=%s,vbat_comp_main=%d,vbat_comp_aux=%d\n",
			info->vbat_comp_para[i].ic_name,
			info->vbat_comp_para[i].vbat_comp[0],
			info->vbat_comp_para[i].vbat_comp[1]);
}

void dc_comp_parse_dts(struct device_node *np, struct dc_comp_para *info)
{
	if (!np || !info)
		return;

	dc_parse_vbat_comp_para(np, info);
}
