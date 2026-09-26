/*
 * series_batt_speaker_charge_adapter.c
 *
 * series_batt_speaker_charge_adapter driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/speaker_charger/series_batt_speaker_charge_adapter.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_auth.h>

#define HWLOG_TAG series_batt_speaker_charge_adapter
HWLOG_REGIST();

static enum adapter_protocol_type g_prot = ADAPTER_PROTOCOL_SCP;

int series_batt_speaker_charge_init_adapter(void)
{
	struct adapter_init_data aid;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	aid.scp_mode_enable = 1;
	aid.vset_boundary = l_di->max_adapter_vset;
	aid.iset_boundary = l_di->max_adapter_iset;
	aid.init_voltage = l_di->init_adapter_vset;
	aid.watchdog_timer = l_di->adapter_watchdog_time;

	return adapter_set_init_data(g_prot, &aid);
}

int series_batt_speaker_charge_check_adapter_antifake(void)
{
	int ret;
	int max_cur;
	int max_pwr;
	struct direct_charge_device *l_di = direct_charge_get_di();

	if (!l_di)
		return -1;

	if (!l_di->adp_antifake_enable)
		return 0;

	max_cur = dc_get_adapter_max_current(
		l_di->bat_cell_num * BAT_RATED_VOLT * l_di->dc_volt_ratio / 100);
	if (max_cur == 0)
		return -1;

	max_pwr = (max_cur * BAT_RATED_VOLT / 1000) * l_di->dc_volt_ratio *
		l_di->bat_cell_num / 100; /* unit: 1000mw */

	hwlog_info("max_cur=%d, max_pwr=%d\n", max_cur, max_pwr);
	if (max_pwr <= POWER_TH_IGNORE_ANTIFAKE)
		return 0;

	dc_auth_clean_hash_data();

	ret = adapter_auth_encrypt_start(g_prot,
		l_di->adp_antifake_key_index,
		dc_auth_get_hash_data_header(), dc_auth_get_hash_data_size());
	if (ret)
		goto fail_check;

	ret = dc_auth_wait_completion();

fail_check:
	dc_auth_clean_hash_data();
	ret += adapter_auth_encrypt_release(g_prot);
	return ret;
}
