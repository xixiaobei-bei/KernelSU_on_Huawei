/*
 *
 * kernel AP maintenance test interface
 *
 * Copyright (c) 2001-2019 Huawei Technologies Co., Ltd.
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
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/threads.h>
#include <linux/kallsyms.h>
#include <linux/reboot.h>
#include <linux/input.h>
#include <linux/syscalls.h>
#include <linux/of.h>

#include <asm/ptrace.h>
#include <securec.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/powerkey_event.h>
#include "../rdr_print.h"
#include <pr_log.h>
#ifdef CONFIG_BOOT_DETECTOR
#include <hwbootfail/chipsets/bootfail_hisi.h>
#endif

#define PR_LOG_TAG BLACKBOX_TAG
#define POWERKEY_ONLY_PRODUCT 0x55aa
const char *g_dts_powerkey_only_status_name = "hisilicon,powerhold";

#ifdef CONFIG_HISI_BB
static u32 reboot_reason_flag;
#endif



#ifdef CONFIG_HISI_REENTRANT_EXCEPTION
#define REENTRANT_EXCEPTION_MAX 5

static int reentrant_exception_num;

void reentrant_exception(void)
{
	reentrant_exception_num++;

	if (reentrant_exception_num > REENTRANT_EXCEPTION_MAX)
		machine_restart("AP_S_PANIC");
}
#endif

static int g_powerkey_only_status;

static int hisi_pmic_powerkey_only_flag(void)
{
	return g_powerkey_only_status;
}

void hisi_pmic_powerkey_only_status_get(void)
{
	const char *press_to_fastboot_status = NULL;
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, g_dts_powerkey_only_status_name);
	if (!np) {
		BB_PRINT_ERR("%s: dts node(powerhold) not found\n", __func__);
		return;
	}
	ret = of_property_read_string(np, "press_6s_to_fastboot", &press_to_fastboot_status);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find powerkey_only_product in dts!\n", __func__);
		g_powerkey_only_status = 0;
		goto exit;
	}
	if (strncmp("ok", press_to_fastboot_status, sizeof("ok")) == 0) {
		g_powerkey_only_status = POWERKEY_ONLY_PRODUCT;
	} else {
		g_powerkey_only_status = 0;
		BB_PRINT_ERR("[%s], press_6s_to_fastboot status is not ok!\n", __func__);
	}
exit:
	of_node_put(np);
}

int rdr_press_key_to_fastboot(struct notifier_block *nb, unsigned long event, void *buf)
{
	if (hisi_pmic_powerkey_only_flag() == POWERKEY_ONLY_PRODUCT) {
		if (event != PRESS_KEY_6S)
			return -1;
	} else {
		if (event != PRESS_KEY_UP)
			return -1;
	}
	if (check_himntn(HIMNTN_PRESS_KEY_TO_FASTBOOT)) {
#ifdef CONFIG_KEYBOARD_HISI_GPIO_VOLUME_KEY
		if ((VOL_UPDOWN_PRESS & (unsigned int)gpio_key_vol_updown_press_get())
				== VOL_UPDOWN_PRESS) {
			gpio_key_vol_updown_press_set_zero();
			if (is_gpio_key_vol_updown_pressed()) {
				BB_PRINT_PN("[%s]Powerkey+VolUp_key+VolDn_key\n", __func__);
#ifdef CONFIG_HISI_BB
				rdr_syserr_process_for_ap(MODID_AP_S_COMBINATIONKEY, 0, 0);
#endif
			}
		}
#endif

		if (event == PRESS_KEY_6S) {
			BB_PRINT_PN("[%s]pmic Powerkey 6s\n", __func__);
#ifdef CONFIG_HISI_BB
			rdr_syserr_process_for_ap(MODID_AP_S_COMBINATIONKEY, 0, 0);
			return 0;
#endif
		}

	}
#ifdef CONFIG_HISI_BB
	if (get_reboot_reason() == AP_S_PRESS6S)
		set_reboot_reason(reboot_reason_flag);
#endif
	return 0;
}

void rdr_long_press_powerkey(void)
{
	u32 reboot_reason = get_reboot_reason();

	set_reboot_reason(AP_S_PRESS6S);
	if (get_boot_keypoint() != STAGE_BOOTUP_END) {
		BB_PRINT_PN("press6s in boot\n");
#ifdef CONFIG_BOOT_DETECTOR
		set_valid_long_press_flag();
#endif
		save_log_to_dfx_tempbuffer(AP_S_PRESS6S);
		SYS_SYNC();
	} else {
		reboot_reason_flag = reboot_reason;
		BB_PRINT_PN("press6s:reboot_reason_flag=%u\n", reboot_reason_flag);
	}
}
