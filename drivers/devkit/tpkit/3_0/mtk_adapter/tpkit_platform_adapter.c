/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/notifier.h>
#include "tpkit_platform_adapter.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif

#include <mt-plat/mtk_boot.h>

unsigned int get_pd_charge_flag(void);
unsigned int get_boot_into_recovery_flag(void);

int charger_type_notifier_register(struct notifier_block *nb)
{
	return 0;
}
int charger_type_notifier_unregister(struct notifier_block *nb)
{
	return 0;
}
enum ts_charger_type get_charger_type(void)
{
	return TS_CHARGER_TYPE_NONE;
}

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
void set_tp_dev_flag(void)
{
	set_hw_dev_detect_result(DEV_DETECT_TOUCH_PANEL);
}
#endif

int read_tp_color_adapter(char *buf, int buf_size)
{
	return -1;
}

int write_tp_color_adapter(const char *buf)
{
	return -1;
}
unsigned int get_into_recovery_flag_adapter(void)
{
	if (get_boot_mode() == RECOVERY_BOOT)
		return 1;
	else
		return 0;
}
unsigned int get_pd_charge_flag_adapter(void)
{
	if ((get_boot_mode() == LOW_POWER_OFF_CHARGING_BOOT) ||
		(get_boot_mode() == KERNEL_POWER_OFF_CHARGING_BOOT))
		return 1;
	else
		return 0;
}
int fb_esd_recover_disable(int value)
{
	return 0;
}
