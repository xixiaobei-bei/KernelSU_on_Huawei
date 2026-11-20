/*
 * wireless_rx_auth.c
 *
 * authenticate for wireless rx charge
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/completion.h>
#include <chipset_common/hwpower/common_module/power_genl.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_auth.h>

#define HWLOG_TAG wireless_rx_auth
HWLOG_REGIST();

static struct completion g_wlrx_auth_completion;
static bool g_wlrx_auth_srv_state;
static int g_wlrx_auth_result;
static u8 g_wlrx_auth_hash[WLRX_AUTH_HASH_LEN];

bool wlrx_auth_get_srv_state(void)
{
	return g_wlrx_auth_srv_state;
}

void wlrx_auth_clean_hash_data(void)
{
	memset(g_wlrx_auth_hash, 0x00, WLRX_AUTH_HASH_LEN);
}

u8 *wlrx_auth_get_hash_data_header(void)
{
	return g_wlrx_auth_hash;
}

unsigned int wlrx_auth_get_hash_data_size(void)
{
	return WLRX_AUTH_HASH_LEN;
}

int wlrx_auth_wait_completion(void)
{
	g_wlrx_auth_result = 0;
	reinit_completion(&g_wlrx_auth_completion);

	/*
	 * if bms_auth service not ready, we assume the serivce is dead,
	 * return hash calculate ok anyway
	 */
	if (g_wlrx_auth_srv_state == false) {
		hwlog_err("service not ready\n");
		return -1;
	}

	power_genl_easy_send(POWER_GENL_TP_AF,
		POWER_GENL_CMD_WLRX_AUTH_HASH, 0,
		g_wlrx_auth_hash, WLRX_AUTH_HASH_LEN);

	/*
	 * if timeout happend, we assume the serivce is dead,
	 * return hash calculate ok anyway
	 */
	if (!wait_for_completion_timeout(&g_wlrx_auth_completion,
		msecs_to_jiffies(WLRX_AUTH_WAIT_TIMEOUT))) {
		hwlog_err("service wait timeout\n");
		return -1;
	}

	/*
	 * if not timeout,
	 * return the antifake result base on the hash calc result
	 */
	if (g_wlrx_auth_result == 0) {
		hwlog_err("hash calculate fail\n");
		return -1;
	}

	hwlog_info("hash calculate ok\n");
	return 0;
}

static int wlrx_auth_srv_on_cb(void)
{
	g_wlrx_auth_srv_state = true;
	hwlog_info("srv_on_cb ok\n");
	return 0;
}

static int wlrx_auth_cb(unsigned char version, void *data, int len)
{
	if (!data || (len != 1)) {
		hwlog_err("data is null or len invalid\n");
		return -1;
	}

	g_wlrx_auth_result = *(int *)data;
	complete(&g_wlrx_auth_completion);

	hwlog_info("version=%u auth_result=%d\n", version, g_wlrx_auth_result);
	return 0;
}

static const struct power_genl_easy_ops wlrx_auth_easy_ops[] = {
	{
		.cmd = POWER_GENL_CMD_WLRX_AUTH_HASH,
		.doit = wlrx_auth_cb,
	}
};

static struct power_genl_node wlrx_auth_genl_node = {
	.target = POWER_GENL_TP_AF,
	.name = "WLRX_AUTH",
	.easy_ops = wlrx_auth_easy_ops,
	.n_easy_ops = WLRX_AUTH_GENL_OPS_NUM,
	.srv_on_cb = wlrx_auth_srv_on_cb,
};

static int __init wlrx_auth_init(void)
{
	init_completion(&g_wlrx_auth_completion);
	power_genl_easy_node_register(&wlrx_auth_genl_node);
	return 0;
}

static void __exit wlrx_auth_exit(void)
{
}

subsys_initcall(wlrx_auth_init);
module_exit(wlrx_auth_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("auth for wireless rx charge module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
