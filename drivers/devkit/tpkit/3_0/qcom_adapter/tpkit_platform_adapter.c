/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include "tpkit_platform_adapter.h"
#include <linux/notifier.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <hwmanufac/dev_detect/dev_detect.h>
#endif
#include <linux/mtd/hw_nve_interface.h>
#include <huawei_platform/log/hw_log.h>
#include "../huawei_ts_kit.h"
#ifdef CONFIG_HUAWEI_DEVKIT_QCOM_3_0
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#endif

#define NV_NUMBER 16
#define VALID_SIZE 15

unsigned int get_pd_charge_flag(void);
unsigned int get_boot_into_recovery_flag(void);

__attribute__((weak)) int power_event_bnc_register(unsigned int type,
	struct notifier_block *nb)
{
	return 0;
}
__attribute__((weak)) int power_event_bnc_unregister(unsigned int type,
	struct notifier_block *nb)
{
	return 0;
}

int charger_type_notifier_register(struct notifier_block *nb)
{
	int ret;

	ret = power_event_bnc_register(POWER_BNT_CONNECT, nb);
	return ret;
}

int charger_type_notifier_unregister(struct notifier_block *nb)
{
	int ret;

	ret = power_event_bnc_unregister(POWER_BNT_CONNECT, nb);
	return ret;
}

enum ts_charger_type get_charger_type(void)
{
	return TS_CHARGER_TYPE_NONE;
}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
void set_tp_dev_flag(void)
{
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
}
#endif

int read_tp_color_adapter(char *buf, int buf_size)
{
	int ret;
	struct hw_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = VALID_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hw_nve_direct_access(&user_info);
	if (ret) {
		TS_LOG_ERR("hw_nve_direct_access read error %d\n", ret);
		return ret;
	}
	/* buf_size value is NV_DATA_SIZE 104 */
	strncpy(buf, user_info.nv_data, buf_size - 1);
	TS_LOG_INFO("tp color from nv is %s\n", buf);
	return 0;
}

int write_tp_color_adapter(const char *buf)
{
	int ret;
	struct hw_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = VALID_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	strncpy(user_info.nv_data, buf, sizeof(user_info.nv_data) - 1);
	ret = hw_nve_direct_access(&user_info);
	if (ret) {
		TS_LOG_ERR("hw_nve_direct_access write error %d\n", ret);
		return ret;
	}
	return 0;
}

unsigned int get_into_recovery_flag_adapter(void)
{
	unsigned int touch_recovery_mode;

	touch_recovery_mode = get_boot_into_recovery_flag();
	return touch_recovery_mode;
}

unsigned int get_pd_charge_flag_adapter(void)
{
	unsigned int charge_flag;

	charge_flag = get_pd_charge_flag();
	return charge_flag;
}

int fb_esd_recover_disable(int value)
{
	return 0;
}
