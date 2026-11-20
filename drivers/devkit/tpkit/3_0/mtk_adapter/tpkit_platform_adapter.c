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
#include <linux/slab.h>
#include <mt-plat/mtk_boot.h>
#include <huawei_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <hwmanufac/dev_detect/dev_detect.h>
#endif

#ifdef CONFIG_MTK_CHARGER
#include <mt-plat/mtk_charger.h>
#endif

#include "../huawei_ts_kit.h"
#ifdef CONFIG_HUAWEI_OEMINFO
#include "oeminfo_def.h"
#endif
#define HWLOG_TAG tp_color
HWLOG_REGIST();
#define OEMINFO_TPCOLOR_NUMBER 65
#define VALID_SIZE 15

unsigned int get_pd_charge_flag(void);
unsigned int get_boot_into_recovery_flag(void);

int charger_type_notifier_register(struct notifier_block *nb)
{
	return 0;
}

int charger_type_notifier_unregister(struct notifier_block *nb)
{
	int error;

	if (g_ts_kit_platform_data.chr_consumer == NULL)
		return -EINVAL;

	error = unregister_charger_manager_notifier(
		g_ts_kit_platform_data.chr_consumer, nb);
	if (error < 0) {
		TS_LOG_ERR("unregister charger notify failed\n");
		return error;
	}

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
	int ret;
	struct oeminfo_info_user *user_info = NULL;

	if (!buf) {
		hwlog_err("%s:buf is null\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_HUAWEI_OEMINFO
	user_info = kzalloc(sizeof(struct oeminfo_info_user),
		GFP_KERNEL);
	if (!user_info) {
		hwlog_err("%s:request memory fail\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: read_tp_color_adapter enter\n", __func__);

	user_info->oeminfo_operation = OEMINFO_READ;
	user_info->oeminfo_id = OEMINFO_TPCOLOR_NUMBER;
	user_info->valid_size = VALID_SIZE;
	ret = oeminfo_direct_access(user_info);
	if (ret) {
		hwlog_err("%s: oeminfo_direct_access read error:%d\n",
			__func__, ret);
		kfree(user_info);
		user_info = NULL;
		return ret;
	}

	memcpy(buf, user_info->oeminfo_data, (buf_size < user_info->valid_size) ?
		buf_size : user_info->valid_size);
	hwlog_info("%s:read tp color from oeminfo is:%s\n", __func__, buf);

	kfree(user_info);
	user_info = NULL;
#endif

	return 0;
}
EXPORT_SYMBOL(read_tp_color_adapter);

int write_tp_color_adapter(const char *buf)
{
	int ret;
	struct oeminfo_info_user *user_info = NULL;

	if (!buf) {
		hwlog_err("%s:buf is null\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_HUAWEI_OEMINFO
	user_info = kzalloc(sizeof(struct oeminfo_info_user),
		GFP_KERNEL);
	if (!user_info) {
		hwlog_err("%s:request memory fail\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: write_tp_color_adapter enter\n", __func__);

	user_info->oeminfo_operation = OEMINFO_WRITE;
	user_info->oeminfo_id = OEMINFO_TPCOLOR_NUMBER;
	user_info->valid_size = VALID_SIZE;
	memcpy(user_info->oeminfo_data, buf,
		(sizeof(user_info->oeminfo_data) < user_info->valid_size) ?
		sizeof(user_info->oeminfo_data) : user_info->valid_size);
	ret = oeminfo_direct_access(user_info);
	if (ret) {
		hwlog_err("%s: oeminfo_direct_access write error:%d\n",
			__func__, ret);
		kfree(user_info);
		user_info = NULL;
		return ret;
	}
	hwlog_info("%s: write tp color to oeminfo is:%s\n",
		__func__, user_info->oeminfo_data);

	kfree(user_info);
	user_info = NULL;
#endif

	return 0;
}
EXPORT_SYMBOL(write_tp_color_adapter);

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
