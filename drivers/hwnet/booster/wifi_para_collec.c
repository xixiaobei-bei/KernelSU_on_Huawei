/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This module is to collect wifi chip parameters
 * Author: tongxilin@huawei.com
 * Create: 2020-03-30.
 */

#include "wifi_para_collec.h"

#include <../net/wireless/rdev-ops.h>
#include <../net/wireless/nl80211.h>
#include <net/neighbour.h>
#include "ip_para_collec_ex.h"

static struct wifi_ctx *g_wifi_ctx;
static u32 g_gateway[MAX_WIFI_NUM];

static u32 turn_over(u32 cur, u32 past)
{
	if (cur < past)
		return MAX_U32 - past + cur + 1;
	else
		return cur - past;
}

static void get_wifi_para(struct wifi_stat *cur, int cnt)
{
	int i;
	struct wifi_info w_info[MAX_WIFI_NUM];

	memset(&w_info[0], 0, MAX_WIFI_NUM * sizeof(struct wifi_info));

	get_wifi_para_for_booster(&w_info[0], MAX_WIFI_NUM);

	for (i = 0; i < cnt; i++) {
		if (strstr(w_info[i].name, "wlan0") == NULL &&
			strstr(w_info[i].name, "wlan1") == NULL)
			continue;
		cur[i].dev_id = i + DEV_INDEX_OFFSET;
		cur[i].flag = w_info[i].flag;
		cur[i].info.rx_bytes = w_info[i].rx_bytes;
		cur[i].info.tx_bytes = w_info[i].tx_bytes;
		cur[i].info.phy_tx_rate = w_info[i].txrate;
		cur[i].info.phy_rx_rate = w_info[i].rxrate;
		cur[i].info.rx_packets = w_info[i].rx_packets;
		cur[i].info.tx_packets = w_info[i].tx_packets;
		cur[i].info.tx_failed = w_info[i].tx_failed;
		cur[i].info.rssi = w_info[i].rssi;

#ifdef CONFIG_HW_GET_EXT_SIG
		cur[i].info.noise = w_info[i].noise;
		cur[i].info.snr = w_info[i].snr;
		cur[i].info.chload = w_info[i].chload;
#else
		cur[i].info.noise = INVALID_NUM2;
		cur[i].info.snr = INVALID_NUM1;
		cur[i].info.chload = INVALID_NUM2;
#endif

#ifdef CONFIG_HW_GET_EXT_SIG_ULDELAY
		cur[i].info.ul_delay = w_info[i].ul_delay;
#else
		cur[i].info.ul_delay = INVALID_NUM1;
#endif
#ifdef CONFIG_HW_NETWORK_QOE
		cur[i].info.arp_state =
			get_wifi_arp_state(w_info[i].name, IFNAMSIZ, g_gateway[i]);
#endif
	}
}

static void check_wifi_para(struct wifi_stat *cur, int cnt)
{
	int i;

	if (cur == NULL)
		return;
	for (i = 0; i < cnt; i++) {
		if (cur[i].flag == 0)
			continue;
		if (cur[i].info.rssi < MIN_RSSI ||
			cur[i].info.rssi > MAX_RSSI)
			cur[i].info.rssi = MAX_RSSI;
		if (cur[i].info.noise < MIN_NOISE ||
			cur[i].info.noise > MAX_NOISE)
			cur[i].info.noise = MIN_NOISE;
		if (cur[i].info.snr < MIN_SNR ||
			cur[i].info.snr > MAX_SNR)
			cur[i].info.snr = MAX_SNR;
		if (cur[i].info.chload < MIN_CHLOAD ||
			cur[i].info.chload > MAX_CHLOAD)
			cur[i].info.chload = MIN_CHLOAD;
		if (cur[i].info.ul_delay < MIN_UL_DELAY ||
			cur[i].info.ul_delay > MAX_UL_DELAY)
			cur[i].info.ul_delay = MIN_UL_DELAY;
	}
}

static void update_wifi_res(struct wifi_stat *past,
	struct wifi_stat *cur, struct wifi_stat *res, int cnt)
{
	int i;

	if (past == NULL || res == NULL || cur == NULL)
		return;
	if (cur[WLAN0_IDX].flag == 0 && cur[WLAN1_IDX].flag == 0)
		return;
	for (i = 0; i < cnt; i++) {
		if (past[i].flag == 0 || cur[i].flag == 0) {
			res[i].flag = 0;
			continue;
		}
		res[i].flag = cur[i].flag;
		res[i].dev_id = cur[i].dev_id;
		res[i].info.rx_bytes =
			turn_over(cur[i].info.rx_bytes,
				past[i].info.rx_bytes);
		res[i].info.tx_bytes =
			turn_over(cur[i].info.tx_bytes,
				past[i].info.tx_bytes);
		res[i].info.rssi = cur[i].info.rssi;
		res[i].info.phy_tx_rate
			= cur[i].info.phy_tx_rate;
		res[i].info.phy_rx_rate
			= cur[i].info.phy_rx_rate;
		res[i].info.rx_packets =
			turn_over(cur[i].info.rx_packets,
				past[i].info.rx_packets);
		res[i].info.tx_packets =
			turn_over(cur[i].info.tx_packets,
				past[i].info.tx_packets);
		res[i].info.tx_failed =
			turn_over(cur[i].info.tx_failed,
				past[i].info.tx_failed);
		res[i].info.noise = cur[i].info.noise;
		res[i].info.snr = cur[i].info.snr;
		res[i].info.chload = cur[i].info.chload;
		res[i].info.ul_delay = cur[i].info.ul_delay;
		res[i].info.arp_state = cur[i].info.arp_state;
	}
}

static void update_wifi_para(struct wifi_stat *past,
	struct wifi_stat *cur, int cnt)
{
	if (past == NULL || cur == NULL)
		return;
	if (cur[WLAN0_IDX].flag == 0 && cur[WLAN1_IDX].flag == 0)
		return;
	memcpy(past, cur, cnt * sizeof(struct wifi_stat));
}

static void wifi_para_report()
{
	struct wifi_res_msg *res = NULL;
	struct wifi_stat cur[MAX_WIFI_NUM];
	u16 len;

	if (g_wifi_ctx == NULL)
		return;
	len = sizeof(struct wifi_res_msg) +
		MAX_WIFI_NUM * sizeof(struct wifi_stat);
	res = kmalloc(len, GFP_ATOMIC);
	if (res == NULL)
		return;
	memset(res, 0, len);
	memset(&cur[0], 0, MAX_WIFI_NUM * sizeof(struct wifi_stat));

	get_wifi_para(&cur[0], MAX_WIFI_NUM); // wlan0->cur[0], wlan1->cur[1],
	check_wifi_para(&cur[0], MAX_WIFI_NUM); // flag = 0,invalid
	update_wifi_res(g_wifi_ctx->past, &cur[0], res->wifi_res, MAX_WIFI_NUM);
	update_wifi_para(g_wifi_ctx->past, &cur[0], MAX_WIFI_NUM);

	res->type = WIFI_PARA_RPT;
	res->len = len;
	if (g_wifi_ctx->fn)
		g_wifi_ctx->fn((struct res_msg_head *)res);
	kfree(res);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
static void wifi_para_report_timer(unsigned long sync)
#else
static void wifi_para_report_timer(struct timer_list* sync)
#endif
{
	struct timer_msg msg;
	char *p = NULL;

	memset(&msg, 0, sizeof(struct timer_msg));
	p = (char *)&msg;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_wifi_ctx->timer.data = sync + 1; // number of reports
#endif
	g_wifi_ctx->timer.function = wifi_para_report_timer;
	mod_timer(&g_wifi_ctx->timer, jiffies + g_wifi_ctx->expires);

	msg.len = sizeof(struct timer_msg);
	msg.type = WIFI_RPT_TIMER;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	msg.sync = (u32)sync;
#endif
	if (g_wifi_ctx->fn)
		g_wifi_ctx->fn((struct res_msg_head *)p);
}

static void stop_wifi_collect(void)
{
	if (!timer_pending(&g_wifi_ctx->timer))
		return;
	del_timer(&g_wifi_ctx->timer);
}

static void start_wifi_collect(struct wifi_req_msg *msg)
{
	u32 expires;

	if (g_wifi_ctx == NULL)
		return;
	expires = msg->report_expires;
	g_gateway[0] = msg->wifi_gateway[0];
	g_gateway[1] = msg->wifi_gateway[1];
	pr_info("[WIFI_PARA]%s,expires=%u", __func__, expires);
	memset(g_wifi_ctx->past, 0, sizeof(struct wifi_stat) * MAX_WIFI_NUM);
	wifi_para_report();
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	g_wifi_ctx->timer.data = 0;
#endif
	g_wifi_ctx->timer.function = wifi_para_report_timer;
	g_wifi_ctx->expires = expires / JIFFIES_MS;
	mod_timer(&g_wifi_ctx->timer, jiffies + g_wifi_ctx->expires);
}

static void update_wifi_collect(struct wifi_req_msg *msg)
{
	g_gateway[0] = msg->wifi_gateway[0];
	g_gateway[1] = msg->wifi_gateway[1];
}

static void cmd_process(struct req_msg_head *msg)
{
	pr_info("[WIFI_PARA]%s,len=%u, type=%u", __func__, msg->len, msg->type);
	if (msg->len > MAX_REQ_DATA_LEN)
		return;
	if (msg->len < sizeof(struct req_msg_head))
		return;
	if (msg->type == WIFI_PARA_COLLECT_START)
		start_wifi_collect((struct wifi_req_msg *)msg);
	else if (msg->type == WIFI_PARA_COLLECT_STOP)
		stop_wifi_collect();
	else if (msg->type == WIFI_RPT_TIMER)
		wifi_para_report();
	else if (msg->type == WIFI_PARA_COLLECT_UPDATE)
		update_wifi_collect((struct wifi_req_msg *)msg);
}

/*
 * Initialize internal variables and external interfaces
 */
msg_process *wifi_para_collec_init(notify_event *fn)
{
	if (fn == NULL)
		return NULL;
	g_wifi_ctx = kmalloc(sizeof(struct wifi_ctx), GFP_KERNEL);
	if (g_wifi_ctx == NULL)
		return NULL;

	memset(g_wifi_ctx, 0, sizeof(struct wifi_ctx));
	g_wifi_ctx->fn = fn;
	g_wifi_ctx->past = kmalloc(sizeof(struct wifi_stat) * MAX_WIFI_NUM,
		GFP_KERNEL);
	if (g_wifi_ctx->past == NULL)
		goto init_error;
	memset(g_wifi_ctx->past, 0, sizeof(struct wifi_stat) * MAX_WIFI_NUM);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	init_timer(&g_wifi_ctx->timer);
#else
	timer_setup(&g_wifi_ctx->timer, wifi_para_report_timer, 0);
#endif

	return cmd_process;

init_error:
	if (g_wifi_ctx != NULL)
		kfree(g_wifi_ctx);
	g_wifi_ctx = NULL;
	return NULL;
}

void wifi_para_collec_exit(void)
{
	if (g_wifi_ctx->past != NULL)
		kfree(g_wifi_ctx->past);
	if (g_wifi_ctx != NULL)
		kfree(g_wifi_ctx);
	g_wifi_ctx = NULL;
}
