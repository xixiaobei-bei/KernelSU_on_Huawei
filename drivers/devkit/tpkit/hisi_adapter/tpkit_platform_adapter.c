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
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <huawei_platform/devdetect/hw_dev_dec.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG tp_color
HWLOG_REGIST();

#define NV_NUMBER 16
#define VALID_SIZE 15
#define NV_WRITE_OPERATION 0
#define NV_READ_OPERATION 1

__attribute__((weak)) unsigned int get_boot_into_recovery_flag(void)
{
	return 0;
}

__attribute__((weak)) unsigned int get_pd_charge_flag(void)
{
	return 0;
}

__attribute__((weak)) int hisifb_esd_recover_disable(int value)
{
	return 0;
}

__attribute__((weak)) int set_hw_dev_flag(int dev_id)
{
	return 0;
}

__attribute__((weak)) int chip_charger_type_notifier_register(
	struct notifier_block *nb)
{
	return 0;
}

__attribute__((weak)) int chip_charger_type_notifier_unregister(
	struct notifier_block *nb)
{
	return 0;
}

__attribute__((weak)) enum hisi_charger_type chip_get_charger_type(void)
{
	return 0;
}

int charger_type_notifier_register(struct notifier_block *nb)
{
	return chip_charger_type_notifier_register(nb);
}

int charger_type_notifier_unregister(struct notifier_block *nb)
{
	return chip_charger_type_notifier_unregister(nb);
}

enum ts_charger_type get_charger_type(void)
{
	return (enum ts_charger_type)chip_get_charger_type();
}

void set_tp_dev_flag(void)
{
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
#endif
}

int read_tp_color_adapter(char *buf, int buf_size)
{
	int ret;
	struct hisi_nve_info_user user_info;

	if (buf == NULL) {
		hwlog_err("%s buf is null\n", __func__);
		return -1;
	}
	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_READ_OPERATION;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = VALID_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		hwlog_err("hisi_nve_direct_access read error:%d\n", ret);
		return -1;
	}
	/* buf_size value is NV_DATA_SIZE 104 */
	strncpy(buf, user_info.nv_data, buf_size - 1);
	return 0;
}
EXPORT_SYMBOL(read_tp_color_adapter);

int write_tp_color_adapter(const char *buf)
{
	int ret;
	struct hisi_nve_info_user user_info;

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_OPERATION;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = VALID_SIZE;
	strncpy(user_info.nv_name, "TPCOLOR", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	strncpy(user_info.nv_data, buf, sizeof(user_info.nv_data) - 1);
	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		hwlog_err("hisi_nve_direct_access write error:%d\n", ret);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(write_tp_color_adapter);

unsigned int get_into_recovery_flag_adapter(void)
{
	return get_boot_into_recovery_flag();
}

unsigned int get_pd_charge_flag_adapter(void)
{
	return get_pd_charge_flag();
}

int fb_esd_recover_disable(int value)
{
	return hisifb_esd_recover_disable(value);
}
EXPORT_SYMBOL(fb_esd_recover_disable);

