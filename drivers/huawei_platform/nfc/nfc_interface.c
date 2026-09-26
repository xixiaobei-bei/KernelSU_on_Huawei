/*
 * nfc_interface.c
 *
 * nfc interface
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/pm_wakeup.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <huawei_platform/nfc/nfc_interface.h>

static unsigned char g_nfc_dmd_probe_flag = 0;

int set_nfc_dmd_probe_flag(unsigned char probe_result)
{
	if (probe_result == NFC_PN547_PROBE_SUCCESS)
		g_nfc_dmd_probe_flag |= 0x01;
	else if (probe_result == NFC_PN547_PROBE_FAIL)
		g_nfc_dmd_probe_flag &= ~0x01;
	else if (probe_result == NFC_ST21NFC_PROBE_SUCCESS)
		g_nfc_dmd_probe_flag |= 0x02;
	else if (probe_result == NFC_ST21NFC_PROBE_FAIL)
		g_nfc_dmd_probe_flag &= ~0x02;
	pr_err("%s: nfc dmd probe:%d\n", __func__, g_nfc_dmd_probe_flag);
	return 0;
}

unsigned char get_nfc_dmd_probe_flag(void)
{
	pr_err("%s: nfc dmd probe:%d\n", __func__, g_nfc_dmd_probe_flag);
	return g_nfc_dmd_probe_flag;
}
