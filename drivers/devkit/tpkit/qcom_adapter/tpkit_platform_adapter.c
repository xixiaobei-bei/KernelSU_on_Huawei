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
#include <linux/hw_dev_dec.h>
#endif

__attribute__((weak)) unsigned int get_pd_charge_flag(void)
{
	return 0;
}

__attribute__((weak)) unsigned int get_boot_into_recovery_flag(void)
{
	return 0;
}

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

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
void set_tp_dev_flag(void)
{
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
}
#endif

int read_tp_color_adapter(char *buf, int buf_size)
{
	return -1;
}

int write_tp_color_adapter(char *buf)
{
	return -1;
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
