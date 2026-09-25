/*
 * heartbeat.c
 *
 * opmp heartbeat function
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include <huawei_platform/emcom/opmp_heartbeat.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <net/sock.h>
#include <net/mptcp_heartbeat.h>
#include <linux/inet.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/emcom/emcom_netlink.h>

#undef HWLOG_TAG
#define HWLOG_TAG opmp_heartbeat
HWLOG_REGIST();

/* LOG related */
#define LOGE(msg, ...) \
	do { \
		hwlog_err("[%s:%d %s]: "msg"\n", kbasename(__FILE__), \
			  __LINE__, __func__, ## __VA_ARGS__); \
	} while (0)

#define LOGI(msg, ...) \
	do { \
		hwlog_info("[%s:%d %s]: "msg"\n", kbasename(__FILE__), \
			   __LINE__, __func__, ## __VA_ARGS__); \
	} while (0)

#define LOGD(msg, ...) \
	do { \
		hwlog_debug("[%s:%d %s]: "msg"\n", kbasename(__FILE__), \
			    __LINE__, __func__, ## __VA_ARGS__); \
	} while (0)

#define INVALID_VALUE (-1);
#define CHECK_TAG(buf, tag) (strstr(buf, tag) != NULL)
#define IFNAME_SIZE 32
#define MAX_BUF 128

static struct sockaddr_in g_link_addrs[LINK_NUM]; // LINK_NUM definied in mptcp_heartbeat.h
static int32_t g_period = DEFAULT_PERIOD;
static int32_t g_timeout = DEFAULT_TIMEOUT;
static int8_t g_retry_count = DEFAULT_RETRY_COUNT;
static int32_t g_teid = 0;
static bool g_mpheartbeat_enabled = false;

static u64 g_previous_tx_bytes = 0;
static u64 g_previous_rx_bytes = 0;

static struct mutp_header_packet g_payload;

static char* g_ifname_array[LINK_NUM] = IF_NAME_LIST;

/* definitions */

/*
 *	init mptcp_heartbeat, and start a timer to check heartbeat condition
 *	input param : link_addrs the addresses of all links, size LINK_NUM
 *	            period
 *	            retry_count,
 */
static void enable_mpheartbeat(struct sockaddr_in* link_addrs, int32_t teid,
		int32_t period, int8_t retry_count)
{
	if (g_mpheartbeat_enabled) {
		LOGE("try to re-enable an enabled mpheartbeat, abort!");
		return;
	}
	if (link_addrs == NULL) {
		LOGE("null link_addrs in enable_mpheartbeat(), abort!");
		return;
	}

	/* first record these parameters */
	g_period = period;
	g_retry_count = retry_count;
	g_teid = teid;
	memcpy(g_link_addrs, link_addrs, sizeof(g_link_addrs));
	/* init mpheartbeat */
	heartbeat_init(link_addrs);
	/* fill the payload, which is mutp header */

	g_payload.version = 1;
	g_payload.icheck = 0x00;
	g_payload.spare = 0x00;
	g_payload.type = HEARTBEAT_REQUEST_CODE;
	g_payload.len = htons(HEARTBEAT_PAYLOAD_LENGTH);
	g_payload.tunnel_id = teid;

	g_mpheartbeat_enabled = true;
	LOGI("mpheartbeat enabled, teid %d, period %d, retry_count %d", teid, period, retry_count);
}

/*
 *	deinit mptcp_heartbeat and stop the timer
 */
static void disable_mpheartbeat(void)
{
	if (!g_mpheartbeat_enabled) {
		LOGE("try to disable an NOT enabled mpheartbeat, abort!");
		return;
	}

	/* reset the global parameters */
	g_period = DEFAULT_PERIOD;
	g_retry_count = DEFAULT_RETRY_COUNT;
	g_teid = 0;
	memset(g_link_addrs, 0, sizeof(g_link_addrs));
	memset(&g_payload, 0, sizeof(g_payload));
	heartbeat_deinit();
	g_mpheartbeat_enabled = false;
	LOGI("mpheartbeat disabled");
}

static void get_line_safely(const char* in, char* out, int max_len)
{
	int i;
	if ((in == NULL) || (out == NULL) || (max_len <= 0)) {
		LOGE("illegal parameters in get_line_safely");
	}
	memset(out, 0, max_len);
	/* without -1, out of boundary error might occur */
	for (i = 0; i < max_len - 1; i++) {
		if (in[i] == '\n')
			break;
	}
	memcpy(out, in, i);
	out[i] = '\0';
}

/*
 *	read the traffic file and get the total tx/rx data bytes
 *	output: total_rx_bytes, total_tx_bytes
 *	return 0 if no error occurs, otherwise return INVALID_VALUE
 */
static int get_total_traffic(u64* total_rx_bytes, u64* total_tx_bytes)
{
	struct file* fp;
	mm_segment_t fs;
	loff_t pos;
	char buffer[MAX_BUF];
	char iface[IFNAME_SIZE];
	u64 rx_bytes;
	u64 rx_packets;
	u64 tx_bytes;
	char search_buf[MAX_BUF];
	int bytes_read;
	int line_len;
	int index;

	if ((total_rx_bytes == NULL) || (total_tx_bytes == NULL)) {
		LOGE("null pointer in get_total_traffic");
		return INVALID_VALUE;
	}

	fp = filp_open(TRAFFIC_FILE, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		LOGE("failed to open file, err code %ld", PTR_ERR(fp));
		return INVALID_VALUE;
	}

	fs = get_fs(); // file operation in kernel requires this
	set_fs(KERNEL_DS);

	pos = 0;
	bytes_read = 0;
	line_len = 0;
	/* read the data of the entire file, to get traffic of all uids */

	while (true) {
		/* vfs_read will automatically set the value of pos */
		bytes_read = vfs_read(fp, buffer, sizeof(buffer), &pos);
		if (bytes_read <= 0)
			break;
		get_line_safely(buffer, search_buf, sizeof(search_buf));
		line_len = strlen(search_buf) + 1;	// the '\0' of the end included
		for (index = 0; index < LINK_NUM; index++) {
			if (CHECK_TAG(search_buf, g_ifname_array[index])) {
				sscanf(buffer, "%31s %llu %llu%llu", iface, &rx_bytes, &rx_packets,&tx_bytes);
				LOGD("traffic for interface %s, rx_bytes %llu, tx_bytes %llu, tx_pkt %llu",
						iface, rx_bytes, tx_bytes, rx_packets);
				*total_rx_bytes += rx_bytes;
				*total_tx_bytes += tx_bytes;
			}
		}
		pos = pos - bytes_read + line_len; //remember to move the pos to the start of next line
	}

	filp_close(fp,NULL);
	set_fs(fs);
	return 0;
}

/*
 *	read the node and determine whether heartbeat is needed
 */
static bool check_heartbeat_needed(void)
{
	u64 total_rx_bytes = 0;
	u64 total_tx_bytes = 0;
	bool ret = false;
	/* get tx/rx data bytes */
	if (get_total_traffic(&total_rx_bytes, &total_tx_bytes) != 0) {
		LOGE("failed to get_total_traffic");
		return false;
	}

	/* compare the latest bytes with the previous ones */
	if (total_tx_bytes > g_previous_tx_bytes && total_rx_bytes == g_previous_rx_bytes) {
		LOGD("heartbeat condition is met");
		ret = true;
	} else {
		LOGD("heartbeat condition is NOT met, g_previous_tx_bytes is %llu, \
g_previous_rx_bytes is %llu, total_tx_bytes is %llu, total_rx_bytes is %llu",
				g_previous_tx_bytes, g_previous_rx_bytes, total_tx_bytes, total_rx_bytes);
		ret = false;
	}

	/* update the tx/rx bytes */
	g_previous_tx_bytes = total_tx_bytes;
	g_previous_rx_bytes = total_rx_bytes;

	return ret;
}

/*
 *	report heartbeat detecion result to daemon
 */
static void opmp_heartbeat_dection_callback(int type, int result)
{
	struct heartbeat_report report;

	LOGD("the detection result for link %d is %d", type, result);
	if (g_mpheartbeat_enabled) {
		report.type = type;
		report.result = result;
		emcom_send_msg2daemon(NETLINK_EMCOM_KD_OPMP_HEARTBEAT_REPORT,
				(void*)&report, sizeof(report));
	}
}

static void trigger_mpheartbeat(void)
{
	bool trigger = false;
	unsigned int detection_type = WIFI_DETECTION + LTE_DETECTION;  // both paths need detection
	struct detection_para para;

	para.retry_count = g_retry_count;
	para.timeout = g_timeout;
	para.payload = (char*)&g_payload;
	para.payload_len = sizeof(g_payload);
	para.cb_fun = opmp_heartbeat_dection_callback;

	LOGD("timer triggered!");
	trigger = check_heartbeat_needed();
	if (trigger) {
		LOGD("mpheartbeat is needed at the moment, trigger it");
		heartbeat_trigger(detection_type, &para);
	}
}

static void handle_init_info(struct heartbeat_init_info* data)
{
	struct sockaddr_in link_addrs[LINK_NUM];
	int32_t teid;

	if (data == NULL) {
		LOGE("null data in handle_init_info");
		return;
	}

	/* first one is wifi */
	link_addrs[0].sin_family = AF_INET;
	link_addrs[0].sin_addr.s_addr = htonl(data->wifi_addr);
	link_addrs[0].sin_port = htons(data->port);
	/* second is LTE */
	link_addrs[1].sin_family = AF_INET;
	link_addrs[1].sin_addr.s_addr = htonl(data->lte_addr);
	link_addrs[1].sin_port = htons(data->port);
	/* teid */
	teid = htonl(data->tunnel_id);

	/*enable heartbeat */
	enable_mpheartbeat(link_addrs, teid, data->interval, data->count);
	LOGD("wifi %d, lte %d, p %d, teid %d, interval %d, count %d",
			data->wifi_addr, data->lte_addr, data->port, teid, data->interval, data->count);
}

/*
 *	handle messages from daemon
 */
void opmp_event_process(int32_t event, uint8_t* pdata, uint16_t len)
{
#ifdef CONFIG_MPTCP_HEARTBEAT
	switch(event) {
		case NETLINK_EMCOM_DK_OPMP_INIT_HEARTBEAT:
			LOGD(" : received NETLINK_EMCOM_DK_OPMP_INIT_HEARTBEAT");
			handle_init_info((struct heartbeat_init_info*)pdata);
			break;
		case NETLINK_EMCOM_DK_OPMP_DEINIT_HEARTBEAT:
			LOGD(" : received NETLINK_EMCOM_DK_OPMP_DEINIT_HEARTBEAT");
			disable_mpheartbeat();
			break;
		case NETLINK_EMCOM_DK_OPMP_TRIGGER_HEARTBEAT:
			LOGD(" : received NETLINK_EMCOM_DK_OPMP_TRIGGER_HEARTBEAT");
			trigger_mpheartbeat();
			break;
		default:
			LOGE(" unsupported event in opmp_event_process");
			break;
	}
#else
	LOGI("mptcp heartbeat feature is not enabled, heartbeat cannot continue");
#endif
}
EXPORT_SYMBOL(opmp_event_process);