/*
 * wireless_tx_alarm.c
 *
 * tx alarm for wireless reverse charging
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
#include <linux/mutex.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/protocol/wireless_protocol_qi.h>
#include <chipset_common/hwpower/protocol/wireless_protocol.h>
#include <chipset_common/hwpower/wireless_charge/wireless_tx_alarm.h>
#include <chipset_common/hwpower/wireless_charge/wireless_trx_ic_intf.h>

#define HWLOG_TAG wireless_tx_alarm
HWLOG_REGIST();

static u8 g_wltx_alarm[QI_TX_ALARM_LEN];
static struct mutex g_alarm_update_lock;

void wltx_reset_alarm_data(void)
{
	memset(g_wltx_alarm, 0, sizeof(g_wltx_alarm));
}

static void wltx_update_tx_alarm_src(u8 *prot_buff, u8 src, u8 type)
{
	if (!src)
		prot_buff[QI_TX_ALARM_SRC] &= ~BIT(type);
	else
		prot_buff[QI_TX_ALARM_SRC] |= BIT(type);
}

static void wltx_update_tx_alarm_plim(u8 *prot_buff, u8 plim)
{
	if (plim <= 0)
		return;

	if (prot_buff[QI_TX_ALARM_PLIM] <= 0)
		prot_buff[QI_TX_ALARM_PLIM] = plim;
	else if (plim < prot_buff[QI_TX_ALARM_PLIM])
		prot_buff[QI_TX_ALARM_PLIM] = plim;
}

static void wltx_update_tx_alarm_vlim(u8 *prot_buff, u8 vlim)
{
	if (vlim <= 0)
		return;

	if (prot_buff[QI_TX_ALARM_VLIM] <= 0)
		prot_buff[QI_TX_ALARM_VLIM] = vlim;
	else if (vlim < prot_buff[QI_TX_ALARM_VLIM])
		prot_buff[QI_TX_ALARM_VLIM] = vlim;
}

static void wltx_send_tx_alarm(u8 *alarm_buff, u8 *alarm_new, int len)
{
	int i;
	int ret;

	if (!memcmp(alarm_buff, alarm_new, len))
		return;

	hwlog_info("[send_tx_alarm] 0x%x %u %u %u | 0x%x %u %u %u\n",
		alarm_buff[QI_TX_ALARM_SRC], alarm_buff[QI_TX_ALARM_PLIM],
		alarm_buff[QI_TX_ALARM_VLIM], alarm_buff[QI_TX_ALARM_RSVD],
		alarm_new[QI_TX_ALARM_SRC], alarm_new[QI_TX_ALARM_PLIM],
		alarm_new[QI_TX_ALARM_VLIM], alarm_new[QI_TX_ALARM_RSVD]);
	memcpy(alarm_buff, alarm_new, len);
	for (i = 0; i < TX_ALARM_SEND_RETRY_CNT; i++) {
		ret = wireless_send_tx_alarm(WLTRX_IC_MAIN, WIRELESS_PROTOCOL_QI,
			alarm_buff, len);
		if (!ret)
			break;
	}
}

void wltx_update_alarm_data(struct wltx_alarm *alarm, u8 src_type)
{
	static u8 tmp[QI_TX_ALARM_LEN];

	mutex_lock(&g_alarm_update_lock);
	memcpy(tmp, g_wltx_alarm, QI_TX_ALARM_LEN);
	wltx_update_tx_alarm_src(tmp, (u8)alarm->src_state, src_type);
	wltx_update_tx_alarm_plim(tmp,
		(u8)(alarm->plim / QI_TX_ALARM_PLIM_STEP));
	wltx_update_tx_alarm_vlim(tmp,
		(u8)(alarm->vlim / QI_TX_ALARM_VLIM_STEP));
	wltx_send_tx_alarm(g_wltx_alarm, tmp, QI_TX_ALARM_LEN);
	mutex_unlock(&g_alarm_update_lock);
}

static int __init wltx_alarm_module_init(void)
{
	mutex_init(&g_alarm_update_lock);
	return 0;
}

static void __exit wltx_alarm_module_exit(void)
{
}

fs_initcall(wltx_alarm_module_init);
module_exit(wltx_alarm_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wireless tx_alarm driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
