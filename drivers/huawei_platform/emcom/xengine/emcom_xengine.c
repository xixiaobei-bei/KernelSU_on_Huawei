/*
* emcom_xengine.c
*
*  xengine module implemention
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

#include <huawei_platform/emcom/emcom_xengine.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/inet6_hashtables.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/in.h>
#include <linux/fdtable.h>
#include <linux/inet.h>
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include <net/inet_hashtables.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include <huawei_platform/emcom/network_evaluation.h>
#ifdef CONFIG_HUAWEI_OPMP
#include <huawei_platform/emcom/opmp_heartbeat.h>
#endif
#ifdef CONFIG_MPTCP
#include <net/mptcp.h>
#endif

#ifdef CONFIG_HUAWEI_BASTET
#include <huawei_platform/net/bastet/bastet_utils.h>
#endif
#include <linux/version.h>
#include <asm/uaccess.h>
#include "securec.h"

#ifndef CONFIG_MPTCP
/* These states need RST on ABORT according to RFC793 */
static inline bool tcp_need_reset(int state)
{
	return (1 << state) &
		(TCPF_ESTABLISHED | TCPF_CLOSE_WAIT | TCPF_FIN_WAIT1 |
		TCPF_FIN_WAIT2 | TCPF_SYN_RECV);
}
#endif


#undef HWLOG_TAG
#define HWLOG_TAG emcom_xengine
HWLOG_REGIST();

#define EMCOM_MAX_ACC_APP 5
#define EMCOM_UID_ACC_AGE_MAX 1000
#define EMCOM_SPEED_CTRL_BASE_WIN_SIZE 10000
#define FAST_SYN_COUNT 5
#define UDPTIMER_DELAY 4
#define EMCOM_MAX_UDP_SKB 20
#define MIN_JIFFIE 1
#define EMCOM_MAX_MPIP_DEV_NUM 2
#define EMCOM_GOOD_RECV_RATE_THR_BYTE_PER_SEC 400000
#define EMCOM_GOOD_RTT_THR_MS 120

static spinlock_t g_mpflow_lock;
struct emcom_xengine_mpflow_info g_mpflow_uids[EMCOM_MPFLOW_MAX_APP];
static uint8_t g_mpflow_index;
static bool g_mpflow_tm_running;

static struct timer_list g_mpflow_tm;
static bool g_mpflow_nf_hook;

struct emcom_xengine_mpflow_stat g_mpflow_list[EMCOM_MPFLOW_MAX_LIST_NUM];
static spinlock_t g_mpflow_ai_lock;
struct emcom_xengine_mpflow_ai_info g_mpflow_ai_uids[EMCOM_MPFLOW_AI_MAX_APP];
struct emcom_xengine_mpflow_stat g_mpflow_ai_list[EMCOM_MPFLOW_AI_MAX_LIST_NUM];


#ifdef CONFIG_HUAWEI_BASTET_COMM
	extern int bastet_comm_keypsInfo_write(uint32_t state);
#endif

struct emcom_xengine_acc_app_info g_current_uids[EMCOM_MAX_ACC_APP];
struct emcom_xengine_speed_ctrl_info g_speedctrl_info;

struct sk_buff_head g_udp_skb_list;
struct timer_list g_udp_skb_timer;
uid_t  g_udp_retran_uid;
bool g_emcom_udptimer_on;
uid_t g_fastsyn_uid;

struct emcom_xengine_netem_skb_cb {
	psched_time_t time_to_send;
	ktime_t tstamp_save;
};

struct mutex g_mpip_mutex;

/* The uid of bind to Mpip Application */
struct  emcom_xengine_mpip_config g_mpip_uids[EMCOM_MAX_MPIP_APP];
bool g_mpip_start;
char g_ifacename[IFNAMSIZ];
static uint8_t g_socket_index;

static bool g_ccalg_start;
int8_t g_ccalg_uid_cnt;
/* The uid of bind to CCAlg Application */
struct emcom_xengine_ccalg_config g_ccalg_uids[EMCOM_MAX_CCALG_APP];

void emcom_xengine_mpip_init(void);
void emcom_xengine_ccalg_init(void);
static void emcom_xengine_mpflow_fi_init(struct emcom_xengine_mpflow_info *mpflow_uid);
static void emcom_xengine_mpflow_register_nf_hook(void);
static void emcom_xengine_mpflow_unregister_nf_hook(void);
static void emcom_xengine_mpflow_download_flow_del(struct emcom_xengine_mpflow_iface *iface,
	struct emcom_xengine_mpflow_node *flow);
static bool emcom_xengine_mpflow_ptn_init(struct emcom_xengine_mpflow_ptn ptn[], uint8_t *num, const char *hex);
static void emcom_xengine_mpflow_ptn_deinit(struct emcom_xengine_mpflow_ptn ptn[], uint8_t num);
static bool emcom_xengine_mpflow_bm_build(const uint8_t *ptn, uint32_t ptnlen,
	uint8_t **skip, uint8_t **shift);
static void emcom_xengine_mpflow_apppriv_deinit(struct emcom_xengine_mpflow_info *uid);
static inline bool invalid_uid(uid_t uid)
{
	/* if uid less than 10000, it is not an Android apk */
	return (uid < UID_APP);
}

static inline bool invalid_speedctrl_size(uint32_t grade)
{
	/* the speed control grade bigger than 10000 */
	return (grade < EMCOM_SPEED_CTRL_BASE_WIN_SIZE);
}
static inline struct emcom_xengine_netem_skb_cb *emcom_xengine_netem_skb_cb(const struct sk_buff *skb)
{
	/* we assume we can use skb next/prev/tstamp as storage for rb_node */
	qdisc_cb_private_validate(skb, sizeof(struct emcom_xengine_netem_skb_cb));
	return (struct emcom_xengine_netem_skb_cb *)qdisc_skb_cb(skb)->data;
}

int emcom_xengine_udpretran_clear(void)
{
	g_udp_retran_uid = UID_INVALID_APP;
	skb_queue_purge(&g_udp_skb_list);
	if (g_emcom_udptimer_on) {
		del_timer(&g_udp_skb_timer);
		g_emcom_udptimer_on = false;
	}
	return 0;
}

static void emcom_xengine_udptimer_handler(unsigned long pac)
{
	struct sk_buff *skb = NULL;
	unsigned long now;
	struct emcom_xengine_netem_skb_cb *cb = NULL;
	int jiffie_n;

	/* anyway, send out the first skb */
	if (!skb_queue_empty(&g_udp_skb_list)) {
		skb = skb_dequeue(&g_udp_skb_list);
		if (skb) {
			dev_queue_xmit(skb);
			EMCOM_LOGD("emcom_xengine_udptimer_handler send skb");
		}
	}

	skb = skb_peek(&g_udp_skb_list);
	if (!skb)
		goto timer_off;
	cb = emcom_xengine_netem_skb_cb(skb);
	now = jiffies;
	/* if remaining time is little than 1 jiffie, send out */
	while (cb->time_to_send <= (now + MIN_JIFFIE)) {
		EMCOM_LOGD("emcom_xengine_udptimer_handler send another skb");
		skb = skb_dequeue(&g_udp_skb_list);
		if (skb)
			dev_queue_xmit(skb);
		skb = skb_peek(&g_udp_skb_list);
		if (!skb)
			goto timer_off;
		cb = emcom_xengine_netem_skb_cb(skb);
		now = jiffies;
	}
	/* set timer based on next skb cb */
	now = jiffies;
	jiffie_n = cb->time_to_send - now;

	if (jiffie_n < MIN_JIFFIE)
		jiffie_n = MIN_JIFFIE;
	EMCOM_LOGD("emcom_xengine_udptimer_handler modify timer hz %d", jiffie_n);
	mod_timer(&g_udp_skb_timer, jiffies + jiffie_n);
	g_emcom_udptimer_on = true;
	return;

timer_off:
	g_emcom_udptimer_on = false;
}

void emcom_xengine_init(void)
{
	uint8_t index;

	for (index = 0; index < EMCOM_MAX_ACC_APP; index++) {
		g_current_uids[index].uid = UID_INVALID_APP;
		g_current_uids[index].age = 0;
	}
	g_speedctrl_info.uid = UID_INVALID_APP;
	g_speedctrl_info.size = 0;
	spin_lock_init(&g_speedctrl_info.stlocker);
	g_udp_retran_uid = UID_INVALID_APP;
	g_emcom_udptimer_on = false;
	skb_queue_head_init(&g_udp_skb_list);
	init_timer(&g_udp_skb_timer);
	g_udp_skb_timer.function = emcom_xengine_udptimer_handler;
	mutex_init(&g_mpip_mutex);
	emcom_xengine_mpip_init();
	emcom_xengine_mpflow_init();
	emcom_xengine_ccalg_init();
	g_fastsyn_uid = UID_INVALID_APP;
}


void emcom_xengine_mpip_init(void)
{
	uint8_t index;

	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < EMCOM_MAX_MPIP_APP; index++) {
		g_mpip_uids[index].uid = UID_INVALID_APP;
		g_mpip_uids[index].type = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	mutex_unlock(&g_mpip_mutex);
}

bool emcom_xengine_is_accuid(uid_t uid)
{
	uint8_t index;

	for (index = 0; index < EMCOM_MAX_ACC_APP; index++) {
		if (uid == g_current_uids[index].uid)
			return true;
	}

	return false;
}

bool emcom_xengine_hook_ul_stub(struct sock *pstsock)
{
	uid_t sock_uid;

	if (pstsock == NULL) {
		EMCOM_LOGD("Emcom_Xengine_Hook_Ul_Stub param invalid");
		return false;
	}

	/**
	 * if uid equals current acc uid, accelerate it,else stop it
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(pstsock).val;
#else
	sock_uid = sock_i_uid(pstsock);
#endif

	if (invalid_uid(sock_uid))
		return false;

	return emcom_xengine_is_accuid(sock_uid);
}

int emcom_xengine_clear(void)
{
	uint8_t index;
	errno_t err;

	for (index = 0; index < EMCOM_MAX_ACC_APP; index++) {
		g_current_uids[index].uid = UID_INVALID_APP;
		g_current_uids[index].age = 0;
	}
	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < EMCOM_MAX_MPIP_APP; index++) {
		g_mpip_uids[index].uid = UID_INVALID_APP;
		g_mpip_uids[index].type = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	err = memset_s(g_ifacename, sizeof(char) * IFNAMSIZ, 0, sizeof(char) * IFNAMSIZ);
	if (err != EOK)
		EMCOM_LOGE("emcom_xengine_clear memset failed");
	g_mpip_start = false;
	mutex_unlock(&g_mpip_mutex);
	for (index = 0; index < EMCOM_MAX_CCALG_APP; index++) {
		g_ccalg_uids[index].uid = UID_INVALID_APP;
		g_ccalg_uids[index].alg = EMCOM_XENGINE_CONG_ALG_INVALID;
		g_ccalg_uids[index].has_log = false;
	}
	g_ccalg_uid_cnt = 0;
	g_ccalg_start = false;
	emcom_xengine_mpflow_clear();
	emcom_xengine_udpretran_clear();
	EMCOM_XENGINE_SET_SPEEDCTRL(g_speedctrl_info, UID_INVALID_APP, 0);
	g_fastsyn_uid = UID_INVALID_APP;
	return 0;
}

uint8_t emcom_xengine_found_avaiable_accindex(uid_t uid)
{
	uint8_t index;
	uint8_t idle_index = EMCOM_MAX_ACC_APP;
	uint8_t old_index = EMCOM_MAX_ACC_APP;
	uint16_t old_age = 0;
	bool found = false;

	/* check whether has the same uid, and record the first idle position and the oldest position */
	for (index = 0; index < EMCOM_MAX_ACC_APP; index++) {
		if (g_current_uids[index].uid == UID_INVALID_APP) {
			if (idle_index == EMCOM_MAX_ACC_APP)
				idle_index = index;
		} else if (uid == g_current_uids[index].uid) {
			g_current_uids[index].age = 0;
			found = true;
		} else {
			g_current_uids[index].age++;
			if (g_current_uids[index].age > old_age) {
				old_age = g_current_uids[index].age;
				old_index = index ;
			}
		}
	}

	/* remove the too old acc uid */
	if (old_age > EMCOM_UID_ACC_AGE_MAX) {
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d added too long, remove it",
				   g_current_uids[old_index].uid);
		g_current_uids[old_index].age = 0;
		g_current_uids[old_index].uid  = UID_INVALID_APP;
	}

	EMCOM_LOGD("Emcom_Xengine_StartAccUid: idle_index=%d,old_index=%d,old_age=%d",
			   idle_index, old_index, old_age);

	/* if has already added, return */
	if (found)
		return index;

	/* if it is new uid, and has idle position , add it */
	if (idle_index < EMCOM_MAX_ACC_APP) {
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d added", uid);
		return idle_index;
	}

	/* if it is new uid, and acc list if full , replace the oldest */
	if (old_index < EMCOM_MAX_ACC_APP) {
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d replace the oldest uid:%d",
				   uid, g_current_uids[old_index].uid);
		return old_index;
	}

	return EMCOM_MAX_ACC_APP;
}

/*
 * start the special application use high priority queue
 */
int emcom_xengine_start_acc_uid(const uint8_t *pdata, uint16_t len)
{
	uid_t uid;
	uint8_t index;

	/* input param check */
	if (pdata == NULL) {
		EMCOM_LOGE("Emcom_Xengine_StartAccUid:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		EMCOM_LOGI("Emcom_Xengine_StartAccUid: len:%d is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)pdata;

	/* check uid */
	if (invalid_uid(uid))
		return -EINVAL;

	EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d ready to added", uid);

	index = emcom_xengine_found_avaiable_accindex(uid);
	/* if it is new uid, and has idle position , add it */
	if (index < EMCOM_MAX_ACC_APP) {
		EMCOM_LOGD("Emcom_Xengine_StartAccUid: uid:%d added", uid);
		g_current_uids[index].age = 0;
		g_current_uids[index].uid = uid;
		return 0;
	}

	EMCOM_LOGE("StartAccUid: not available index:%d, uid:%d", index, uid);
	return 0;
}


/*
 * stop the special application use high priority queue
 */
int emcom_xengine_stop_acc_uid(const uint8_t *pdata, uint16_t len)
{
	uid_t uid;
	uint8_t index;

	/* input param check */
	if (pdata == NULL) {
		EMCOM_LOGE("Emcom_Xengine_StopAccUid:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		EMCOM_LOGI("Emcom_Xengine_StopAccUid: len: %d is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)pdata;

	/* check uid */
	if (invalid_uid(uid))
		return -EINVAL;

	/* remove specify uid */
	for (index = 0; index < EMCOM_MAX_ACC_APP; index++) {
		if (uid == g_current_uids[index].uid) {
			g_current_uids[index].age = 0;
			g_current_uids[index].uid  = UID_INVALID_APP;
			EMCOM_LOGD("Emcom_Xengine_StopAccUid:lUid:%d", uid);
			break;
		}
	}

	return 0;
}

/*
 * confige the background application tcp window size
 */
int emcom_xengine_set_speedctrl_info(const uint8_t *data, uint16_t len)
{
	struct emcom_xengine_speed_ctrl_data *pspeedctrl_info = NULL;
	uid_t uid;
	uint32_t size;

	/* input param check */
	if (data == NULL) {
		EMCOM_LOGE("Emcom_Xengine_SetSpeedCtrlInfo:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(struct emcom_xengine_speed_ctrl_data)) {
		EMCOM_LOGI("Emcom_Xengine_SetSpeedCtrlInfo: len:%d is illegal", len);
		return -EINVAL;
	}

	pspeedctrl_info = (struct emcom_xengine_speed_ctrl_data *)data;
	uid = pspeedctrl_info->uid;
	size = pspeedctrl_info->size;

	/* if uid and size is zero, clear the speed control info */
	if (!uid && !size) {
		EMCOM_LOGD("Emcom_Xengine_SetSpeedCtrlInfo: clear speed ctrl state");
		EMCOM_XENGINE_SET_SPEEDCTRL(g_speedctrl_info, UID_INVALID_APP, 0);
		return 0;
	}

	/* check uid */
	if (invalid_uid(uid)) {
		EMCOM_LOGI("Emcom_Xengine_SetSpeedCtrlInfo: uid:%d is illegal", uid);
		return -EINVAL;
	}

	/* check size */
	if (invalid_speedctrl_size(size)) {
		EMCOM_LOGI("Emcom_Xengine_SetSpeedCtrlInfo: size:%d is illegal", size);
		return -EINVAL;
	}

	EMCOM_LOGD("Emcom_Xengine_SetSpeedCtrlInfo: uid:%d size:%d", uid, size);
	EMCOM_XENGINE_SET_SPEEDCTRL(g_speedctrl_info, uid, size);
	return 0;
}

/*
 * if the application is send packet, limit the other background  application
 * send pakcet rate according adjust the send wind
 */
void emcom_xengine_speedctrl_winsize(struct sock *pstsock, uint32_t *pstsize)
{
	uid_t sock_uid;
	uid_t uid;
	uint32_t size;

	if (pstsock == NULL) {
		EMCOM_LOGD("Emcom_Xengine_Hook_Ul_Stub param invalid\n");
		return;
	}

	if (pstsize == NULL) {
		EMCOM_LOGD(" Emcom_Xengine_SpeedCtrl_WinSize window size invalid\n");
		return;
	}

	EMCOM_XENGINE_GET_SPEEDCTRL_UID(g_speedctrl_info, uid);
	if (invalid_uid(uid))
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(pstsock).val;
#else
	sock_uid = sock_i_uid(pstsock);
#endif

	if (invalid_uid(sock_uid))
		return;

	EMCOM_XENGINE_GET_SPEEDCTRL_INFO(g_speedctrl_info, uid, size);
	/* check uid */
	if (sock_uid == uid)
		return;

	if (size)
		*pstsize = g_speedctrl_info.size < *pstsize ? g_speedctrl_info.size : *pstsize;
}

/*
 * clear the mpip configure information, only confige but not start
 */
int emcom_xengine_config_mpip(const uint8_t *data, uint16_t len)
{
	uint8_t index;
	const uint8_t *temp = data;
	uint8_t length;

	/* The empty updated list means clear the Mpip App Uid list */
	EMCOM_LOGD("The Mpip list will be update to empty");

	/* Clear the Mpip App Uid list */
	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < EMCOM_MAX_MPIP_APP; index++) {
		g_mpip_uids[index].uid = UID_INVALID_APP;
		g_mpip_uids[index].type = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	mutex_unlock(&g_mpip_mutex);
	/* pdata == NULL or len == 0 is ok, just return */
	if ((temp == NULL) || (len == 0))
		return 0;
	length = len / sizeof(struct emcom_xengine_mpip_config);
	if (length > EMCOM_MAX_MPIP_APP) {
		EMCOM_LOGE("The length of received MPIP APP uid list is error");
		return -EINVAL;
	}
	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < length; index++) {
		g_mpip_uids[index].uid = *(uid_t *)temp;
		g_mpip_uids[index].type = *(uint32_t *)(temp + sizeof(uid_t));
		EMCOM_LOGD("The Mpip config [%d] is: lUid %d and type %d", index,
				   g_mpip_uids[index].uid, g_mpip_uids[index].type);
		temp += sizeof(struct emcom_xengine_mpip_config);
	}
	mutex_unlock(&g_mpip_mutex);

	return 0;
}

/*
 * clear the mpip configure information
 */
int emcom_xengine_clear_mpip_config(const uint8_t *data, uint16_t len)
{
	uint8_t index;

	/* The empty updated list means clear the Mpip App Uid list */
	EMCOM_LOGD("The Mpip list will be update to empty");

	/* Clear the Mpip App Uid list */
	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < EMCOM_MAX_MPIP_APP; index++) {
		g_mpip_uids[index].uid = UID_INVALID_APP;
		g_mpip_uids[index].type = EMCOM_XENGINE_MPIP_TYPE_BIND_NEW;
	}
	mutex_unlock(&g_mpip_mutex);

	return 0;
}

/*
 * start  the application use mpip function
 * current support five application use this function in the same time
 */
int emcom_xengine_start_mpip(const char *data, uint16_t len)
{
	errno_t err;

	/* input param check */
	if ((data == NULL) || (len == 0) || (len > IFNAMSIZ)) {
		EMCOM_LOGE("MPIP interface name or length %d is error", len);
		return -EINVAL;
	}
	mutex_lock(&g_mpip_mutex);
	err = memcpy_s(g_ifacename, sizeof(char) * IFNAMSIZ, data, len);
	if (err != EOK)
		EMCOM_LOGE("emcom_xengine_start_mpip memcpy failed");
	g_mpip_start = true;
	mutex_unlock(&g_mpip_mutex);
	EMCOM_LOGD("Mpip is :%d to start", g_mpip_start);
	return 0;
}

/*
 * stop all the application use mpip function
 * current not support stop single application use this function
 */
int emcom_xengine_stop_mpip(const uint8_t *data, uint16_t len)
{
	mutex_lock(&g_mpip_mutex);
	g_mpip_start = false;
	mutex_unlock(&g_mpip_mutex);
	EMCOM_LOGD("MPIP function is :%d, ready to stop", g_mpip_start);

	return 0;
}


/*
 * check the application is support mpip function
 */
int emcom_xengine_is_mpip_binduid(uid_t uid)
{
	int ret = -1;
	uint8_t index;

	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < EMCOM_MAX_MPIP_APP; index++) {
		if (uid == g_mpip_uids[index].uid) {
			mutex_unlock(&g_mpip_mutex);
			ret = index;
			return ret;
		}
	}
	mutex_unlock(&g_mpip_mutex);

	return ret;
}

/*
 * bind special socket to suitable device
 */
void emcom_xengine_mpip_bind2device(struct sock *pstsock)
{
	int found;
	uid_t sock_uid;
	struct net *net = NULL;

	if (pstsock == NULL) {
		EMCOM_LOGE(" param invalid");
		return;
	}

	if (!g_mpip_start)
		return;
	/**
	 * if uid equals current bind uid, bind 2 device
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(pstsock).val;
#else
	sock_uid = sock_i_uid(pstsock);
#endif

	if (invalid_uid(sock_uid)) {
		return;
	}

	net = sock_net(pstsock);
	found = emcom_xengine_is_mpip_binduid(sock_uid);
	if (found != -1) {
		uint8_t index = 0;
		struct net_device *dev = NULL;

		rcu_read_lock();
		dev = dev_get_by_name_rcu(net, g_ifacename);
		if (dev)
			index = dev->ifindex;
		rcu_read_unlock();
		if (!dev || !test_bit(__LINK_STATE_START, &dev->state)) {
			mutex_lock(&g_mpip_mutex);
			g_mpip_start = false;
			mutex_unlock(&g_mpip_mutex);
			emcom_send_msg2daemon(NETLINK_EMCOM_KD_XENIGE_DEV_FAIL, NULL, 0);
			EMCOM_LOGE(" get dev fail or dev is not up");
			return;
		}

		if (g_mpip_uids[found].type == EMCOM_XENGINE_MPIP_TYPE_BIND_RANDOM) {
			if (g_socket_index % EMCOM_MAX_MPIP_DEV_NUM == 0) {
				lock_sock(pstsock);
				pstsock->sk_bound_dev_if = index;
				sk_dst_reset(pstsock);
				release_sock(pstsock);
			}
			g_socket_index++;
			g_socket_index = g_socket_index % EMCOM_MAX_MPIP_DEV_NUM;
		} else {
			lock_sock(pstsock);
			pstsock->sk_bound_dev_if = index;
			sk_dst_reset(pstsock);
			release_sock(pstsock);
		}
	}
}


int emcom_xengine_rrckeep(void)
{
#ifdef CONFIG_HUAWEI_BASTET
	post_indicate_packet(BST_IND_RRC_KEEP, NULL, 0);
#endif
	return 0;
}


/*
 * inform modem current application is high priority
 */
int emcom_send_keypsinfo(const uint8_t *data, uint16_t len)
{
	uint32_t state;

	/* input param check */
	if (data == NULL) {
		EMCOM_LOGE("Emcom_Send_KeyPsInfo:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len < sizeof(uint32_t)) {
		EMCOM_LOGE("Emcom_Send_KeyPsInfo: len: %d is illegal", len);
		return -EINVAL;
	}

	state = *(uint32_t *)data;

	if (true != emcom_is_modem_support()) {
		EMCOM_LOGI("Emcom_Send_KeyPsInfo: modem not support");
		return -EINVAL;
	}

#ifdef CONFIG_HUAWEI_BASTET_COMM
	bastet_comm_keypsInfo_write(state);
#endif
	return 0;
}

/*
 * judge current network is wifi
 */
static bool emcom_xengine_iswlan(const struct sk_buff *skb)
{
	const char *delim = "wlan0";
	int len = strlen(delim);

	if (!skb->dev)
		return false;

	if (strncmp(skb->dev->name, delim, len))
		return false;

	return true;
}


/*
 * when the application send packet ,we retran it immediately
 */
void emcom_xengine_udpenqueue(const struct sk_buff *skb)
{
	struct sock *sk = NULL;
	uid_t sock_uid;

	/* invalid g_udp_retran_uid means UDP retran is closed */
	if (invalid_uid(g_udp_retran_uid))
		return;

	if (!skb) {
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue skb null");
		return;
	}

	if (g_udp_skb_list.qlen >= EMCOM_MAX_UDP_SKB) {
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue max skb");
		return;
	}

	sk = skb_to_full_sk(skb);
	if (unlikely(!sk)) {
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue sk null");
		return;
	}

	if (unlikely(!sk->sk_socket)) {
		EMCOM_LOGE("Emcom_Xengine_UdpEnqueue sk_socket null");
		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(sk).val;
#else
	sock_uid = sock_i_uid(sk);
#endif
	if (sock_uid == g_udp_retran_uid) {
		if (!emcom_xengine_iswlan(skb)) {
			EMCOM_LOGD("Emcom_Xengine_UdpEnqueue not wlan");
			emcom_xengine_udpretran_clear();
			return;
		}

		if (sk->sk_socket->type == SOCK_DGRAM) {
			struct sk_buff *skb2 = skb_copy(skb, GFP_ATOMIC);
			if (unlikely(!skb2)) {
				EMCOM_LOGE("Emcom_Xengine_UdpEnqueue skb2 null");
				return;
			}
			dev_queue_xmit(skb2);
			return;
		}
	}
}

/*
 * indicate the  application in current condition need retran packets in wifi
 */
int emcom_xengine_start_udpretran(const uint8_t *data, uint16_t len)
{
	uid_t uid;

	/* input param check */
	if (data == NULL) {
		EMCOM_LOGE("Emcom_Xengine_StartUdpReTran:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		EMCOM_LOGI("Emcom_Xengine_StartUdpReTran: len: %d is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)data;
	/* check uid */
	if (invalid_uid(uid)) {
		EMCOM_LOGE("Emcom_Xengine_StartUdpReTran: uid is invalid %d", uid);
		return -EINVAL;
	}
	EMCOM_LOGI("Emcom_Xengine_StartUdpReTran: uid: %d ", uid);
	g_udp_retran_uid = uid;
	return 0;
}

/*
 * stop wifi retran function
 */
int emcom_xengine_stop_udpretran(const uint8_t *data, uint16_t len)
{
	emcom_xengine_udpretran_clear();
	return 0;
}

/*
 * when tcp need retrans sync packet, call this fucntion to
 * adjust the interval for the application
 */
void emcom_xengine_fastsyn(struct sock *pstsock)
{
	uid_t sock_uid;
	struct inet_connection_sock *icsk = NULL;

	if (pstsock == NULL) {
		EMCOM_LOGD(" Emcom_Xengine_FastSyn param invalid");
		return;
	}
	if (pstsock->sk_state != TCP_SYN_SENT)
		return;

	if (invalid_uid(g_fastsyn_uid))
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(pstsock).val;
#else
	sock_uid = sock_i_uid(pstsock);
#endif

	if (sock_uid != g_fastsyn_uid)
		return;

	icsk = inet_csk(pstsock);
	if (icsk->icsk_retransmits <= FAST_SYN_COUNT)
		icsk->icsk_rto = TCP_TIMEOUT_INIT;
}

/*
 * indicate spec application use fast sync function
 * current only support one application in the same time
 */
int emcom_xengine_start_fastsyn(const uint8_t *data, uint16_t len)
{
	uid_t uid;

	/* input param check */
	if (data == NULL) {
		EMCOM_LOGE(" emcom_xengine_start_fastsyn:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		EMCOM_LOGI(" emcom_xengine_start_fastsyn: len: %d is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)data;
	/* check uid */
	if (invalid_uid(uid)) {
		EMCOM_LOGE(" emcom_xengine_start_fastsyn: uid is invalid %d", uid);
		return -EINVAL;
	}
	EMCOM_LOGI(" emcom_xengine_start_fastsyn: uid: %d ", uid);
	g_fastsyn_uid = uid;
	return 0;
}

/*
 * stop every application use fast sync function
 * current not support stop single application
 */
int emcom_xengine_stop_fastsyn(const uint8_t *data, uint16_t len)
{
	g_fastsyn_uid = UID_INVALID_APP;
	return 0;
}

/*
 * init emcom_xengine congestion control algorithm and log flad for g_ccAlgUids
 */
void emcom_xengine_ccalg_init(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MAX_CCALG_APP; index++) {
		g_ccalg_uids[index].uid = UID_INVALID_APP;
		g_ccalg_uids[index].alg = EMCOM_XENGINE_CONG_ALG_INVALID;
		g_ccalg_uids[index].has_log = false;
	}
}

/*
 * find activated congestion control algorithm uid
 * return index of activated uid in g_ccalg_uids
 */
int8_t emcom_xengine_find_ccalg(uid_t uid)
{
	int8_t index;

	for (index = 0; index < EMCOM_MAX_CCALG_APP; index++) {
		if (g_ccalg_uids[index].uid == uid)
			return index;
	}
	return INDEX_INVALID;
}

/*
 * activating congestion control algorithm with uid and algorithm
 */
void emcom_xengine_active_ccalg(const uint8_t *data, uint16_t len)
{
	uid_t uid;
	uint32_t alg;
	int8_t index;
	struct emcom_xengine_ccalg_config_data *ccalg_config = NULL;

	/* input param check */
	if ((data == NULL) || (len == 0) || (len > IFNAMSIZ)) {
		EMCOM_LOGE("CCAlg interface name or length %d is error", len);
		return;
	}
	g_ccalg_start = true;
	ccalg_config = (struct emcom_xengine_ccalg_config_data *)data;
	uid = ccalg_config->uid;
	alg = ccalg_config->alg;

	index = emcom_xengine_find_ccalg(uid);
	if (index != INDEX_INVALID) {
		if (g_ccalg_uids[index].alg == alg) {
			/* activating again, do nothing */
			EMCOM_LOGD("alg: %u is activating again, uid: %u", alg, uid);
		} else {
			/* already activated, but change another algorithm */
			EMCOM_LOGD("CCAlg function is ready to change alg, uid: %u, alg from %u to %u",
					   uid, g_ccalg_uids[index].alg, alg);
			g_ccalg_uids[index].alg = alg;
			g_ccalg_uids[index].has_log = false;
		}
	} else {
		/* a new app to activate */
		int8_t indexNew = emcom_xengine_find_ccalg(UID_INVALID_APP);
		if (indexNew != INDEX_INVALID) {
			g_ccalg_uids[indexNew].uid = uid;
			g_ccalg_uids[indexNew].alg = alg;
			g_ccalg_uids[indexNew].has_log = false;
			g_ccalg_uid_cnt++;
			EMCOM_LOGD("CCAlg function is ready to start, uid: %u, alg: %u", uid, alg);
		} else if (g_ccalg_uid_cnt >= EMCOM_MAX_CCALG_APP) {
			EMCOM_LOGE("CCAlg has already activated %d apps, cannot activate more apps", g_ccalg_uid_cnt);
			return;
		} else {
			EMCOM_LOGE("not supposed to happend: CCAlg has already activated %d apps", g_ccalg_uid_cnt);
		}
	}
}

/*
 * deacvitating congestion control algorithm with uid
 */
void emcom_xengine_deactive_ccalg(const uint8_t *data, uint16_t len)
{
	uid_t uid;
	int8_t index;

	if ((data == NULL) || (len == 0) || (len > IFNAMSIZ)) {
		EMCOM_LOGE("CCAlg interface name or length %d is error", len);
		return;
	}

	uid = *((uid_t *)data);

	index = emcom_xengine_find_ccalg(uid);
	if (index != INDEX_INVALID) {
		EMCOM_LOGD("CCAlg function is ready to stop, uid: %u, alg: %u", uid, g_ccalg_uids[index].alg);
		g_ccalg_uids[index].uid = UID_INVALID_APP;
		g_ccalg_uids[index].alg = EMCOM_XENGINE_CONG_ALG_INVALID;
		g_ccalg_uids[index].has_log = false;
		g_ccalg_uid_cnt--;
	} else {
		EMCOM_LOGE("CCAlg function is not activated yet, cannot be deactivated, uid: %u", uid);
		return;
	}

	if (g_ccalg_uid_cnt <= 0) {
		EMCOM_LOGD(" no ccalg is activated now: CCAlg function is ready to stop, cnt: %u", g_ccalg_uid_cnt);
		g_ccalg_start = false;
		return;
	}
}

/*
 * change default congestion control algorithm to activated algorithm
 */
void emcom_xengine_change_default_ca(struct sock *sk, struct list_head tcp_cong_list)
{
	struct inet_connection_sock *icsk = NULL;
	struct tcp_congestion_ops *ca = NULL;
	int8_t index;
	char *app_alg = NULL;

	if (!g_ccalg_start)
		return;

	index = emcom_xengine_find_ccalg(sock_i_uid(sk).val);
	if (index == INDEX_INVALID)
		return;

	switch (g_ccalg_uids[index].alg) {
	case EMCOM_XENGINE_CONG_ALG_BBR:
		app_alg = EMCOM_CONGESTION_CONTROL_ALG_BBR;
		break;
	default:
		return;
	}

	icsk = inet_csk(sk);
	list_for_each_entry_rcu(ca, &tcp_cong_list, list) {
		if (likely(try_module_get(ca->owner))) {
			if (strcmp(ca->name, app_alg) == 0) {
				icsk->icsk_ca_ops = ca;
				if (g_ccalg_uids[index].has_log == false) {
					EMCOM_LOGD("app: %d change default congcontrol alg to :%s", sock_i_uid(sk).val, app_alg);
					g_ccalg_uids[index].has_log = true;
				}
				return;
			}
		}
	}

	if (g_ccalg_uids[index].has_log == false) {
		EMCOM_LOGE("Emcom_Xengine_change_default_ca failed to find algorithm %s", EMCOM_CONGESTION_CONTROL_ALG_BBR);
		g_ccalg_uids[index].has_log = true;
	}
}

bool emcom_xengine_check_ip_addrss(struct sockaddr *addr)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)addr;

	if (usin->sin_family == AF_INET) {
		return !ipv4_is_loopback(usin->sin_addr.s_addr) && !ipv4_is_multicast(usin->sin_addr.s_addr) &&
				!ipv4_is_zeronet(usin->sin_addr.s_addr) && !ipv4_is_lbcast(usin->sin_addr.s_addr);
	} else if (usin->sin_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;
		return !ipv6_addr_loopback(&usin6->sin6_addr) && !ipv6_addr_is_multicast(&usin6->sin6_addr);
	}

	return true;
}

/* start index of ipv4 address which is mapped into ipv6 address */
#define EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX 3

bool emcom_xengine_check_ip_is_private(struct sockaddr *addr)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)addr;

	if (usin->sin_family == AF_INET) {
		return (ipv4_is_linklocal_169(usin->sin_addr.s_addr) ||
			ipv4_is_private_10(usin->sin_addr.s_addr) ||
			ipv4_is_private_172(usin->sin_addr.s_addr) ||
			ipv4_is_private_192(usin->sin_addr.s_addr));
	} else if (usin->sin_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;
		int addr_type = ipv6_addr_type(&usin6->sin6_addr);
		if (addr_type & IPV6_ADDR_MAPPED) {
			__be32 s_addr = usin6->sin6_addr.s6_addr32[EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX];
			return (ipv4_is_linklocal_169(s_addr) ||
				ipv4_is_private_10(s_addr) ||
				ipv4_is_private_172(s_addr) ||
				ipv4_is_private_192(s_addr));
		}
	}

	return false;
}

void emcom_xengine_mpflow_init(void)
{
	uint8_t uindex;
	errno_t err;

	EMCOM_LOGD("mpflow init");
	spin_lock_init(&g_mpflow_lock);
	spin_lock_bh(&g_mpflow_lock);
	for (uindex = 0; uindex < EMCOM_MPFLOW_MAX_APP; uindex++) {
		g_mpflow_uids[uindex].uid = UID_INVALID_APP;
		g_mpflow_uids[uindex].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		g_mpflow_uids[uindex].enableflag = 0;
		g_mpflow_uids[uindex].protocol = 0;
		err = memset_s(&g_mpflow_uids[uindex].dport_range,
					   sizeof(g_mpflow_uids[uindex].dport_range),
					   0,
					   sizeof(g_mpflow_uids[uindex].dport_range));
		if (err != EOK)
			EMCOM_LOGD("emcom_xengine_mpflow_init failed");
	}
	err = memset_s(g_mpflow_list, sizeof(g_mpflow_list), 0, sizeof(g_mpflow_list));
	if (err != EOK)
		EMCOM_LOGD("emcom_xengine_mpflow_init g_mpflow_list failed");
	spin_unlock_bh(&g_mpflow_lock);
	g_mpflow_index = 0;

	spin_lock_init(&g_mpflow_ai_lock);
	spin_lock_bh(&g_mpflow_ai_lock);
	err = memset_s(g_mpflow_ai_uids, sizeof(g_mpflow_ai_uids), 0, sizeof(g_mpflow_ai_uids));
	if (err != EOK)
		EMCOM_LOGD("emcom_xengine_mpflow_init ai failed");
	err = memset_s(g_mpflow_ai_list, sizeof(g_mpflow_ai_list), 0, sizeof(g_mpflow_ai_list));
	if (err != EOK)
		EMCOM_LOGD("g_mpflow_ai_list failed");
	spin_unlock_bh(&g_mpflow_ai_lock);
}

void emcom_xengine_mpflow_clear(void)
{
	uint8_t index;
	errno_t err;
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;

	spin_lock_bh(&g_mpflow_lock);
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if ((g_mpflow_uids[index].uid != UID_INVALID_APP) &&
			((g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) ||
			 (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) ||
			 (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) ||
			 (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI))) {
			list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].wifi.flows, list)
				emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].wifi, node);

			list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].lte.flows, list)
				emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].lte, node);

			emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
			emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
			emcom_xengine_mpflow_apppriv_deinit(&g_mpflow_uids[index]);
		}

		g_mpflow_uids[index].uid = UID_INVALID_APP;
		g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		g_mpflow_uids[index].enableflag = 0;
		g_mpflow_uids[index].protocol = 0;
		err = memset_s(&g_mpflow_uids[index].dport_range,
					   sizeof(g_mpflow_uids[index].dport_range),
					   0,
					   sizeof(g_mpflow_uids[index].dport_range));
		if (err != EOK)
			EMCOM_LOGD("emcom_xengine_mpflow_clear failed");
	}
	spin_unlock_bh(&g_mpflow_lock);

	emcom_xengine_mpflow_unregister_nf_hook();
	g_mpflow_index = 0;
}

int8_t emcom_xengine_mpflow_finduid(uid_t uid)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid == uid)
			return index;
	}

	return INDEX_INVALID;
}

static bool emcom_xengine_mpflow_uid_empty(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid != UID_INVALID_APP)
			return false;
	}

	return true;
}

int8_t emcom_xengine_mpflow_getfreeindex(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid == UID_INVALID_APP)
			return index;
	}
	return INDEX_INVALID;
}

static bool emcom_xengine_mpflow_fi_start(bool is_new_uid_enable, uint8_t index, bool *ret,
	struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo)
{
	if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO ||
		g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI) {
		if (is_new_uid_enable)
			emcom_xengine_mpflow_fi_init(&g_mpflow_uids[index]);
		*ret = true;
		return true;
	}
	if (is_new_uid_enable) {
		int i;
		struct emcom_xengine_mpflow_app_priv *app_priv = NULL;

		emcom_xengine_mpflow_fi_init(&g_mpflow_uids[index]);
		app_priv = kzalloc(sizeof(struct emcom_xengine_mpflow_app_priv), GFP_ATOMIC);
		if (!app_priv)
			return false;

		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
			if (g_mpflow_uids[index].reserve_field & EMCOM_MPFLOW_ENABLEFLAG_LTE_FIRST) {
				app_priv->lte_thresh = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH;
				app_priv->lte_thresh_max = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX;
				app_priv->lte_thresh_min = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MIN;
				app_priv->lte_first = 1;
			} else {
				app_priv->lte_thresh = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH;
				app_priv->lte_thresh_max = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MAX;
				app_priv->lte_thresh_min = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MIN;
				app_priv->lte_first = 0;
			}
		}

		for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++)
			INIT_HLIST_HEAD(&app_priv->hashtable[i]);

		g_mpflow_uids[index].app_priv = app_priv;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_80, &(g_mpflow_uids[index].ptn_80_num),
			mpflowstartinfo->ptn_80))
			*ret = true;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_443, &(g_mpflow_uids[index].ptn_443_num),
			mpflowstartinfo->ptn_443))
			*ret = true;
	} else {
		emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
		emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_80, &(g_mpflow_uids[index].ptn_80_num),
			mpflowstartinfo->ptn_80))
			*ret = true;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_443, &(g_mpflow_uids[index].ptn_443_num),
			mpflowstartinfo->ptn_443))
			*ret = true;
	}
	return true;
}
void emcom_xengine_mpflow_start(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo = NULL;
	int8_t index;
	bool ret = false;
	bool is_new_uid_enable = false;

	/* input param check */
	if (!pdata || (len != sizeof(struct emcom_xengine_mpflow_parse_start_info))) {
		EMCOM_LOGE("mpflow start data or length %d is error", len);
		return;
	}

	mpflowstartinfo = (struct emcom_xengine_mpflow_parse_start_info *)pdata;

	EMCOM_LOGD("mpflow start uid: %u, enableflag: %d, "
			   "protocol: %d, bindmode: %d, algorithm: %d",
			   mpflowstartinfo->uid, mpflowstartinfo->enableflag,
			   mpflowstartinfo->protocol, mpflowstartinfo->bindmode, mpflowstartinfo->algorithm_type);

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(mpflowstartinfo->uid);
	if (index == INDEX_INVALID) {
		int8_t newindex;

		EMCOM_LOGD("mpflow add new mpinfo uid: %d", mpflowstartinfo->uid);
		newindex = emcom_xengine_mpflow_getfreeindex();
		if (newindex == INDEX_INVALID) {
			EMCOM_LOGE("mpflow start get free index exceed. uid: %d",
					   mpflowstartinfo->uid);
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
		index = newindex;
		is_new_uid_enable = true;
	}

	/* redundant operation. new uid related mpflow list entry doesn't exist normally */
	if (is_new_uid_enable)
		emcom_xengine_mpflow_clear_blocked(mpflowstartinfo->uid, EMCOM_MPFLOW_VER_V1);

	/* Fill mpflow info. */
	g_mpflow_uids[index].uid = mpflowstartinfo->uid;
	g_mpflow_uids[index].enableflag = mpflowstartinfo->enableflag;
	g_mpflow_uids[index].protocol = mpflowstartinfo->protocol;
	g_mpflow_uids[index].bindmode = mpflowstartinfo->bindmode;
	g_mpflow_uids[index].algorithm_type = mpflowstartinfo->algorithm_type;
	g_mpflow_uids[index].reserve_field = mpflowstartinfo->reserve_field;

	if (mpflowstartinfo->enableflag & EMCOM_MPFLOW_ENABLEFLAG_DPORT) {
		errno_t err = memcpy_s(g_mpflow_uids[index].dport_range,
							   sizeof(g_mpflow_uids[index].dport_range),
							   mpflowstartinfo->dport_range,
							   sizeof(mpflowstartinfo->dport_range));
		if (err != EOK)
			EMCOM_LOGE("emcom_xengine_mpflow_start memcpy failed");
	}
	if ((g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) ||
		(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) ||
		(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) ||
		(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI)) {
		bool fi_start_ret = emcom_xengine_mpflow_fi_start(is_new_uid_enable, index, &ret, mpflowstartinfo);
		if (!fi_start_ret) {
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
	}

	emcom_xengine_mpflow_show();
	spin_unlock_bh(&g_mpflow_lock);

	if (ret)
		emcom_xengine_mpflow_register_nf_hook();
	else
		emcom_xengine_mpflow_unregister_nf_hook();
}

void emcom_xengine_mpflow_stop(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_stop_info *mpflowstopinfo = NULL;
	int8_t index;
	int32_t stop_reason;
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	bool mpflow_uid_empty = false;


	/* input param check */
	if (!pdata || (len != sizeof(struct emcom_xengine_mpflow_parse_stop_info))) {
		EMCOM_LOGE("mpflow stop data or length %d is error", len);
		return;
	}

	mpflowstopinfo = (struct emcom_xengine_mpflow_parse_stop_info *)pdata;
	stop_reason = mpflowstopinfo->stop_reason;
	EMCOM_LOGD("mpflow stop uid: %u, stop reason: %u", mpflowstopinfo->uid, stop_reason);
	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(mpflowstopinfo->uid);
	if (index != INDEX_INVALID) {
		if ((stop_reason == EMCOM_MPFLOW_STOP_REASON_NETWORK_ROAMING) ||
			(stop_reason == EMCOM_MPFLOW_STOP_REASON_APPDIED)) {
			errno_t err;

			EMCOM_LOGD("mpflow stop clear info uid: %u, index: %d ", mpflowstopinfo->uid, index);

			if ((g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) ||
				(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) ||
				(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) ||
				(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI)) {
				list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].wifi.flows, list)
					emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].wifi, node);

				list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].lte.flows, list)
					emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].lte, node);

				emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
				emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
				emcom_xengine_mpflow_apppriv_deinit(&g_mpflow_uids[index]);
			}

			g_mpflow_uids[index].uid = UID_INVALID_APP;
			g_mpflow_uids[index].enableflag = 0;
			g_mpflow_uids[index].protocol = 0;
			g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
			err = memset_s(&g_mpflow_uids[index].dport_range, sizeof(g_mpflow_uids[index].dport_range), 0,
						   sizeof(g_mpflow_uids[index].dport_range));
			if (err != EOK)
				EMCOM_LOGE("emcom_xengine_mpflow_stop memset failed");
		} else {
			g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		}
	}
	emcom_xengine_mpflow_show();
	emcom_xengine_mpflow_delete(mpflowstopinfo->uid, EMCOM_MPFLOW_VER_V1);

	mpflow_uid_empty = emcom_xengine_mpflow_uid_empty();
	spin_unlock_bh(&g_mpflow_lock);

	if (mpflow_uid_empty)
		emcom_xengine_mpflow_unregister_nf_hook();
}

bool emcom_xengine_mpflow_checkvalid(struct sock *sk, struct sockaddr *uaddr, int8_t index, uint16_t *dport)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	bool isvalidaddr = false;

	if (!sk || !uaddr)
		return false;

	isvalidaddr = emcom_xengine_check_ip_addrss(uaddr) && (!emcom_xengine_check_ip_is_private(uaddr));
	if (isvalidaddr == false) {
		EMCOM_LOGD("mpflow check valid addr is not valid. uid: %u",
				   g_mpflow_uids[index].uid);
		return false;
	}

	EMCOM_LOGD("mpflow check valid uid: %u link famliy: %d, link proto: %d,"
			   "mpflow protocol: %d, bindmode: %u, ",
			   g_mpflow_uids[index].uid, sk->sk_family, sk->sk_protocol,
			   g_mpflow_uids[index].protocol,
			   g_mpflow_uids[index].bindmode);

	if (!(((sk->sk_protocol == IPPROTO_TCP) &&
		(EMCOM_MPFLOW_PROTOCOL_TCP & g_mpflow_uids[index].protocol))
		|| ((sk->sk_protocol == IPPROTO_UDP) &&
		(EMCOM_MPFLOW_PROTOCOL_UDP & g_mpflow_uids[index].protocol)))) {
		EMCOM_LOGD("mpflow check valid protocol not correct uid: %u, sk: %pK",
				   g_mpflow_uids[index].uid, sk);
		return false;
	}

	if (g_mpflow_uids[index].enableflag & EMCOM_MPFLOW_ENABLEFLAG_DPORT) {
		bool bfinddport = false;
		if (usin->sin_family == AF_INET) {
			*dport = ntohs(usin->sin_port);
		} else if (usin->sin_family == AF_INET6) {
			struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)uaddr;
			*dport = (uint16_t)ntohs(usin6->sin6_port);
		} else {
			EMCOM_LOGD("mpflow check valid not support family uid: %u,"
					   " sin_family: %d",
					   g_mpflow_uids[index].uid, usin->sin_family);
			return false;
		}
		bfinddport = emcom_xengine_mpflow_finddport(&g_mpflow_uids[index], *dport);
		if (bfinddport == false) {
			EMCOM_LOGD("mpflow check valid can not find uid: %u, dport: %d",
					   g_mpflow_uids[index].uid, *dport);
			return false;
		}
	}

	return true;
}

bool emcom_xengine_mpflow_getinetaddr(struct net_device *dev)
{
	struct in_device *in_dev = NULL;
	struct in_ifaddr *ifa = NULL;

	if (!dev) {
		EMCOM_LOGD("mpflow get inet addr dev is null");
		return false;
	}

	in_dev = __in_dev_get_rcu(dev);
	if (!in_dev) {
		EMCOM_LOGD("mpflow get inet addr in_dev is null dev: %s", dev->name);
		return false;
	}

	ifa = in_dev->ifa_list;
	if (ifa != NULL)
		return true;

	return false;
}

bool emcom_xengine_mpflow_dev_is_valid(struct sock *sk, char *ifname)
{
	bool ret = true;
	struct net_device *dev = NULL;
	rcu_read_lock();
	dev = dev_get_by_name_rcu(sock_net(sk), ifname);
	if (!dev || (emcom_xengine_mpflow_getinetaddr(dev) == false))
		ret = false;
	rcu_read_unlock();
	return ret;
}

static uint8_t emcom_xengine_mpflow_ip_hash(__be32 addr)
{
#define EMCOM_MPFLOW_HASH_BIT_FOUR 4
#define EMCOM_MPFLOW_HASH_BIT_EIGHT (4 * 2)
#define EMCOM_MPFLOW_HASH_BIT_TWELVE (4 * 3)
#define EMCOM_MPFLOW_HASH_BIT_SIXTEEN (4 * 4)

	uint32_t hash;
	hash = (addr) << EMCOM_MPFLOW_HASH_BIT_EIGHT;
	hash ^= (addr) >> EMCOM_MPFLOW_HASH_BIT_FOUR;
	hash ^= (addr) >> EMCOM_MPFLOW_HASH_BIT_TWELVE;
	hash ^= (addr) >> EMCOM_MPFLOW_HASH_BIT_SIXTEEN;

#undef EMCOM_MPFLOW_HASH_BIT_FOUR
#undef EMCOM_MPFLOW_HASH_BIT_EIGHT
#undef EMCOM_MPFLOW_HASH_BIT_TWELVE
#undef EMCOM_MPFLOW_HASH_BIT_SIXTEEN
	return (uint8_t)(hash & (EMCOM_MPFLOW_HASH_SIZE - 1));
}

static struct emcom_xengine_mpflow_ip *emcom_xengine_mpflow_hash(__be32 addr,
	struct hlist_head *hashtable, uint32_t algorithm_type)
{
	struct emcom_xengine_mpflow_ip *ip = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash;
	unsigned long aging;

	if (algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK)
		aging = EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH;
	else
		aging = EMCOM_MPFLOW_IP_AGING_THREH;

	hash = emcom_xengine_mpflow_ip_hash(addr);

	hlist_for_each_entry_safe(ip, tmp, &hashtable[hash], node) {
		if (ip->addr == addr)
			return ip;

		/* free too old entrys */
		if ((jiffies - ip->jiffies[(ip->tot_cnt - 1) % EMCOM_MPFLOW_FLOW_JIFFIES_REC]) >
			aging) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}

	ip = kzalloc(sizeof(struct emcom_xengine_mpflow_ip), GFP_ATOMIC);
	if (!ip)
		return NULL;

	ip->addr = addr;
	hlist_add_head(&ip->node, &hashtable[hash]);
	return ip;
}

static void emcom_xengine_mpflow_hash_clear(struct emcom_xengine_mpflow_app_priv *priv)
{
	int i;
	struct emcom_xengine_mpflow_ip *ip = NULL;
	struct hlist_node *tmp = NULL;

	for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[i], node) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}
}

static bool emcom_xengine_mpflow_ip_chk_bind_lte(
	struct emcom_xengine_mpflow_app_priv *priv,
	struct emcom_xengine_mpflow_ip *ip,
	uint32_t type)
{
	if (type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
		bool lte_first = priv->lte_first ? true : false;
		uint8_t index = ip->tot_cnt % EMCOM_MPFLOW_FLOW_JIFFIES_REC;
		unsigned long now = jiffies;

		if (ip->tot_cnt < EMCOM_MPFLOW_FLOW_JIFFIES_REC)
			return lte_first;

		if (time_before(now, ip->jiffies[index] + EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH)) {
			/* if (lte_cnt/tot_cnt < lte_thresh/10), then we need bind on lte */
			if ((ip->lte_cnt * EMCOM_MPFLOW_FI_NETDISK_FLOW_NUM) < (priv->lte_thresh * ip->tot_cnt))
				return true;
			else
				return false;
		}

		return lte_first;
	} else {
		if (ip->tot_cnt % EMCOM_MPFLOW_DEV_NUM)
			return true;
		else
			return false;
	}
}

static bool emcom_xengine_mpflow_get_addr_port(struct sockaddr *addr, __be32 *s_addr, uint16_t *port)
{
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *usin = (struct sockaddr_in *)addr;
		*s_addr = usin->sin_addr.s_addr;
		*port = ntohs(usin->sin_port);
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;

		if (!ipv6_addr_v4mapped(&usin6->sin6_addr))
			return false;
		*s_addr = usin6->sin6_addr.s6_addr32[EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX];
		*port = ntohs(usin6->sin6_port);
		return true;
	}
#endif
	else {
		return false;
	}
}

#undef EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX

static int emcom_xengine_mpflow_ip_bind(struct sockaddr *addr,
	struct emcom_xengine_mpflow_info *uid)
{
	__be32 daddr;
	uint16_t dport;
	struct emcom_xengine_mpflow_ip *ip = NULL;
	int bind_dev;
	struct emcom_xengine_mpflow_app_priv *priv = uid->app_priv;
	uint32_t type = uid->algorithm_type;

	if (!emcom_xengine_mpflow_get_addr_port(addr, &daddr, &dport))
		return EMCOM_MPFLOW_BIND_WIFI;

	if ((dport == EMCOM_MPFLOW_FI_PORT_443) && !uid->ptn_443_num)
		return EMCOM_MPFLOW_BIND_WIFI;

	if ((dport == EMCOM_MPFLOW_FI_PORT_80) && !uid->ptn_80_num)
		return EMCOM_MPFLOW_BIND_WIFI;

	ip = emcom_xengine_mpflow_hash(daddr, priv->hashtable, type);
	if (!ip)
		return EMCOM_MPFLOW_BIND_WIFI;

	if (emcom_xengine_mpflow_ip_chk_bind_lte(priv, ip, type)) {
		ip->lte_cnt++;
		bind_dev = EMCOM_MPFLOW_BIND_LTE;
	} else {
		bind_dev = EMCOM_MPFLOW_BIND_WIFI;
	}
	ip->jiffies[ip->tot_cnt % EMCOM_MPFLOW_FLOW_JIFFIES_REC] = jiffies;
	ip->tot_cnt++;
	return bind_dev;
}

static void emcom_xengine_mpflow_netdisk_lte_thresh(struct emcom_xengine_mpflow_app_priv *priv, int add)
{
	if ((add > 0) && (priv->lte_thresh < priv->lte_thresh_max))
		priv->lte_thresh++;
	else if ((add < 0) && (priv->lte_thresh > priv->lte_thresh_min))
		priv->lte_thresh--;
}

static void emcom_xengine_mpflow_download_finish(struct emcom_xengine_mpflow_info *uid)
{
#define EMCOM_MPFLOW_FI_RATE_RATIO_NUMERATOR 4
#define EMCOM_MPFLOW_FI_RATE_RATIO_DENOMINATOR 5
	struct emcom_xengine_mpflow_app_priv *priv = uid->app_priv;

	if (!priv)
		return;

	if (uid->algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
		int add = 0;

		EMCOM_LOGD("lte %u %u, wifi %u %u",
				   uid->lte.max_rate_received_flow,
				   uid->lte.bytes_received,
				   uid->wifi.max_rate_received_flow,
				   uid->wifi.bytes_received);

		if ((uid->lte.bytes_received + uid->wifi.bytes_received) < EMCOM_MPFLOW_LTE_THREH_ADJUST_BYTES)
			return;

		if (uid->lte.max_rate_received_flow && uid->wifi.max_rate_received_flow &&
			(uid->lte.max_rate_received_flow < uid->wifi.max_rate_received_flow)) {
			/* avg_speed_per_flow_LTE< avg_speed_per_flow_WIFI, lte_thresh-- */
			EMCOM_LOGD("case 1: lte_thresh--");
			add--;
		} else if ((uid->lte.max_rate_received_flow > EMCOM_MPFLOW_NETDISK_RATE_GOOD) &&
			((EMCOM_MPFLOW_FI_RATE_RATIO_DENOMINATOR * uid->wifi.max_rate_received_flow) <
			(EMCOM_MPFLOW_FI_RATE_RATIO_NUMERATOR * uid->lte.max_rate_received_flow))) {
			/* avg_speed_per_flow_LTE>1M && avg_speed_per_flow_WIFI < 0.8* avg_speed_per_flow_LTE, lte_thresh++ */
			EMCOM_LOGD("case 2: lte_thresh++");
			add++;
		} else if (!priv->lte_thresh && (uid->wifi.max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD)) {
			EMCOM_LOGD("case 3: lte_thresh++");
			add++;
		} else if ((priv->lte_thresh == EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX) &&
			(uid->lte.max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD)) {
			EMCOM_LOGD("case 4: lte_thresh--");
			add--;
		}

		emcom_xengine_mpflow_netdisk_lte_thresh(priv, add);
	}

	emcom_xengine_mpflow_hash_clear(priv);
#undef EMCOM_MPFLOW_FI_RATE_RATIO_NUMERATOR
#undef EMCOM_MPFLOW_FI_RATE_RATIO_DENOMINATOR
}


int emcom_xengine_mpflow_getmode_rand(int8_t index, uid_t uid, struct sockaddr *uaddr)
{
	bool is_wifi_block = false;
	bool is_lte_block = false;
	int bind_device = EMCOM_MPFLOW_BIND_NONE;

	if (g_mpflow_uids[index].rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI) {
		bind_device = EMCOM_MPFLOW_BIND_WIFI;
	} else if (g_mpflow_uids[index].rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE) {
		bind_device = EMCOM_MPFLOW_BIND_LTE;
	} else if (g_mpflow_uids[index].app_priv) {
		bind_device = emcom_xengine_mpflow_ip_bind(uaddr, &g_mpflow_uids[index]);
	} else {
		g_mpflow_index++;
		if (emcom_xengine_mpflow_connum(uid, EMCOM_WLAN_IFNAME) == 0)
			bind_device = EMCOM_MPFLOW_BIND_WIFI;
		else if (g_mpflow_index % EMCOM_MPFLOW_DEV_NUM == 0)
			bind_device = EMCOM_MPFLOW_BIND_WIFI;
		else
			bind_device = EMCOM_MPFLOW_BIND_LTE;
	}

	is_wifi_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN_IFNAME, EMCOM_MPFLOW_VER_V1);
	is_lte_block = emcom_xengine_mpflow_blocked(uid, EMCOM_LTE_IFNAME, EMCOM_MPFLOW_VER_V1);
	if ((is_wifi_block == true) && (is_lte_block == true))
		bind_device = EMCOM_MPFLOW_BIND_NONE;
	else if ((is_wifi_block == true) && (bind_device == EMCOM_MPFLOW_BIND_WIFI))
		bind_device = EMCOM_MPFLOW_BIND_LTE;
	else if ((is_lte_block == true) && (bind_device == EMCOM_MPFLOW_BIND_LTE))
		bind_device = EMCOM_MPFLOW_BIND_WIFI;

	return bind_device;
}

int emcom_xengine_mpflow_getmode_spec(int8_t index, uid_t uid)
{
	bool is_wifi_block = false;
	bool is_lte_block = false;
	int bind_device = EMCOM_MPFLOW_BIND_NONE;

	is_wifi_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN_IFNAME, EMCOM_MPFLOW_VER_V1);
	is_lte_block = emcom_xengine_mpflow_blocked(uid, EMCOM_LTE_IFNAME, EMCOM_MPFLOW_VER_V1);

	if ((is_wifi_block && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(is_lte_block && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		((is_wifi_block == true) && (is_lte_block == true))) {
		EMCOM_LOGD("mpflow bind blocked uid: %u, bindmode: %d, blocked:%d, %d, connnum:%d",
				   uid, g_mpflow_uids[index].bindmode, is_wifi_block, is_lte_block,
				   emcom_xengine_mpflow_connum(uid, EMCOM_WLAN_IFNAME));
		return bind_device;
	}

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)
		bind_device = EMCOM_MPFLOW_BIND_WIFI;

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)
		bind_device = EMCOM_MPFLOW_BIND_LTE;

	return bind_device;
}

int emcom_xengine_mpflow_getmode(int8_t index, uid_t uid, struct sockaddr *uaddr)
{
	int bind_device = EMCOM_MPFLOW_BIND_NONE;

	if (g_mpflow_uids[index].rst_bind_mode != EMCOM_XENGINE_MPFLOW_BINDMODE_NONE) {
		if (time_after(jiffies, g_mpflow_uids[index].rst_jiffies + EMCOM_MPFLOW_RST_TIME_THREH))
			g_mpflow_uids[index].rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	}

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)
		bind_device = emcom_xengine_mpflow_getmode_rand(index, uid, uaddr);
	else
		bind_device = emcom_xengine_mpflow_getmode_spec(index, uid);

	return bind_device;
}

void emcom_xengine_mpflow_bind2device(struct sock *sk, struct sockaddr *uaddr)
{
	uid_t uid;
	int8_t index;
	struct net_device *dev = NULL;
	char ifname[IFNAMSIZ] = {0};
	int bind_device;
	uint16_t dport;
	errno_t err;

	if (!sk || !uaddr)
		return;

	uid = sock_i_uid(sk).val;
	if (invalid_uid(uid))
		return;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(uid);
	if (index == INDEX_INVALID) {
		emcom_xengine_mpflow_ai_bind2device(sk, uaddr);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	if (emcom_xengine_mpflow_checkvalid(sk, uaddr, index, &dport) == false) {
		EMCOM_LOGD("mpflow bind2device check valid fail uid: %u", uid);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	bind_device = emcom_xengine_mpflow_getmode(index, uid, uaddr);

	if (bind_device == EMCOM_MPFLOW_BIND_WIFI) {
		err = memcpy_s(ifname, sizeof(char) * IFNAMSIZ, EMCOM_WLAN_IFNAME, (strlen(EMCOM_WLAN_IFNAME) + 1));
	} else if (bind_device == EMCOM_MPFLOW_BIND_LTE) {
		err = memcpy_s(ifname, sizeof(char) * IFNAMSIZ, EMCOM_LTE_IFNAME, (strlen(EMCOM_LTE_IFNAME) + 1));
	} else if (bind_device == EMCOM_MPFLOW_BIND_NONE) {
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}
	if (err != EOK)
		EMCOM_LOGE("mpflow bind2device memcpy failed");

	sk->is_mp_flow = 1;
	spin_unlock_bh(&g_mpflow_lock);

	rcu_read_lock();
	dev = dev_get_by_name_rcu(sock_net(sk), ifname);
	if (!dev || (emcom_xengine_mpflow_getinetaddr(dev) == false)) {
		rcu_read_unlock();
		EMCOM_LOGD("mpflow bind2device dev not ready uid: %u, sk: %pK, dev: %pK, name: %s",
				   uid, sk, dev, (dev == NULL ? "null" : dev->name));
		return;
	}
	rcu_read_unlock();
	sk->sk_bound_dev_if = dev->ifindex;
	EMCOM_LOGD("mpflow bind2device success uid: %u, sk: %pK, "
			   "ifname: %s, ifindex: %d",
			   uid, sk, ifname, sk->sk_bound_dev_if);
}


bool emcom_xengine_mpflow_finddport(struct emcom_xengine_mpflow_info *mpflowinfo, uint16_t dport)
{
	int i;

	if (!mpflowinfo) {
		EMCOM_LOGE("mpflow finddport mpflow info is NULL");
		return false;
	}

	EMCOM_LOGD("mpflow finddport dport: %d", dport);

	for (i = 0; i < EMCOM_MPFLOW_PORT_RANGE_NUM_MAX; i++) {
		if ((mpflowinfo->dport_range[i].start_port <= dport) &&
			(mpflowinfo->dport_range[i].end_port >= dport))
			return true;
	}

	return false;
}

static bool emcom_xengine_get_mpflowlist(uint8_t ver, struct emcom_xengine_mpflow_stat **mpflow_list)
{
	switch (ver) {
	case EMCOM_MPFLOW_VER_V1:
		*mpflow_list = g_mpflow_list;
		break;
	case EMCOM_MPFLOW_VER_V2:
		*mpflow_list = g_mpflow_ai_list;
		break;
	default:
		EMCOM_LOGE("get mpflowlist failed.Version %d not supported", ver);
		return false;;
	}

	return true;
}
struct emcom_xengine_mpflow_stat *emcom_xengine_mpflow_get(int uid, char *name, int ifindex, uint8_t ver)
{
	struct emcom_xengine_mpflow_stat *node = NULL;
	int8_t index;
	errno_t err;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return NULL;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		if ((node == NULL) && (mpflow_list[index].uid == UID_INVALID_APP))
			node = &mpflow_list[index];
		if ((mpflow_list[index].uid == uid) && (!strncmp(mpflow_list[index].name, name, strlen(name))))
			return &mpflow_list[index];
	}

	if (!node) {
		EMCOM_LOGD("emcom_xengine_mpflow_get list full\n");
		return NULL;
	}

	node->uid = uid;
	node->ifindex = ifindex;
	err = strncpy_s(node->name, IFNAMSIZ, name, IFNAMSIZ - 1);
	if (err != EOK) {
		EMCOM_LOGE("emcom_xengine_mpflow_get strncpy_s failed, errcode: %d", err);
		node->uid = UID_INVALID_APP;
		return NULL;
	}
	node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
	node->mpflow_fail_nopayload = 0;
	node->mpflow_fail_syn_rst = 0;
	node->mpflow_fail_syn_timeout = 0;
	node->mpflow_estlink = 0;
	node->start_jiffies = 0;
	err = memset_s(node->retrans_count, sizeof(node->retrans_count), 0, sizeof(node->retrans_count));
	if (err != EOK) {
		EMCOM_LOGE("emcom_xengine_mpflow_get memset failed, errcode: %d", err);
		node->uid = UID_INVALID_APP;
		return NULL;
	}

	return node;
}


void emcom_xengine_mpflow_delete(int uid, uint8_t ver)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &mpflow_list[index];
		if (node->uid == uid)
			node->uid = UID_INVALID_APP;
	}
}

void emcom_xengine_mpflow_clear_blocked(int uid, uint8_t ver)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	errno_t err;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &mpflow_list[index];
		if (node->uid == uid) {
			node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
			node->mpflow_fail_nopayload = 0;
			node->mpflow_fail_syn_rst = 0;
			node->mpflow_fail_syn_timeout = 0;
			node->start_jiffies = 0;
			err = memset_s(node->retrans_count, sizeof(node->retrans_count),
						   0, sizeof(node->retrans_count));
			if (err != EOK)
				EMCOM_LOGD("emcom_xengine_mpflow_clear_blocked memset failed");
		}
	}
}

bool emcom_xengine_mpflow_blocked(int uid, char *name, uint8_t ver)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return false;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &mpflow_list[index];
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return (node->mpflow_fallback == EMCOM_MPFLOW_FALLBACK_SET);
	}

	return false;
}

int16_t emcom_xengine_mpflow_connum(int uid, char *name)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &g_mpflow_list[index];
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return node->mpflow_estlink;
	}
	return 0;
}


void emcom_xengine_mpflow_report(void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPFLOW_FALLBACK, data, len);
}

void emcom_xengine_mpflow_show(void)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &g_mpflow_list[index];
		if (node->uid == UID_INVALID_APP)
			continue;
		EMCOM_LOGE("MpFlow showinfo uid:%d inf:%d(%s) fail:%d estlink:%d "
				   "nodata,rst,tout:%d,%d,%d \n",
				   node->uid, node->ifindex, node->name,
				   node->mpflow_fallback, node->mpflow_estlink,
				   node->mpflow_fail_nopayload, node->mpflow_fail_syn_rst,
				   node->mpflow_fail_syn_timeout);
	}
	return;
}

bool emcom_xengine_mpflow_errlink(int reason, struct emcom_xengine_mpflow_stat *node)
{
	if (reason == EMCOM_MPFLOW_FALLBACK_NOPAYLOAD)
		node->mpflow_fail_nopayload++;
	else if (reason == EMCOM_MPFLOW_FALLBACK_SYN_RST)
		node->mpflow_fail_syn_rst++;
	else if ((reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT) ||
			(reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST))
		node->mpflow_fail_syn_timeout++;
	else
		return false;

	if ((node->mpflow_fail_nopayload >= EMCOM_MPFLOW_FALLBACK_NOPAYLOAD_THRH)
		|| (node->mpflow_fail_syn_rst >= EMCOM_MPFLOW_FALLBACK_SYN_RST_THRH)
		|| (node->mpflow_fail_syn_timeout >= EMCOM_MPFLOW_FALLBACK_SYN_TOUT_THRH))
		return true;

	return false;
}

bool emcom_xengine_mpflow_retrans(int reason, struct emcom_xengine_mpflow_stat *node)
{
	int i;
	errno_t err;

	if ((reason != EMCOM_MPFLOW_FALLBACK_RETRANS) &&
		(reason != EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST))
		return false;

	for (i = 1; (i <= node->retrans_count[0]) && (i <= EMCOM_MPFLOW_FALLBACK_RETRANS_TIME); i++) {
		if (time_before_eq(jiffies, node->start_jiffies + i * HZ)) {
			node->retrans_count[i]++;
			break;
		}
	}

	/* Time range matched */
	if (i <= node->retrans_count[0]) {
		/* expand time range */
		if (node->retrans_count[i] == EMCOM_MPFLOW_FALLBACK_RETRANS_THRH) {
			node->retrans_count[0]++;
			EMCOM_LOGD("MpFlow fallback uid:%d inf:%d(%s) count:%d, jiffies:%lu\n",
					   node->uid, node->ifindex, node->name, node->retrans_count[0], node->start_jiffies);
		}
		/* retransmission fallback */
		if (node->retrans_count[0] > EMCOM_MPFLOW_FALLBACK_RETRANS_TIME) {
			err = memset_s(node->retrans_count, sizeof(node->retrans_count), 0, sizeof(node->retrans_count));
			if (err != EOK)
				EMCOM_LOGE("emcom_xengine_mpflow_retrans memset failed");
			return true;
		}
	} else {
		err = memset_s(node->retrans_count, sizeof(node->retrans_count), 0, sizeof(node->retrans_count));
		if (err != EOK)
			EMCOM_LOGE("emcom_xengine_mpflow_retrans memset failed");
		node->retrans_count[0] = 1;
		node->retrans_count[1] = 1;
		node->start_jiffies = jiffies;
	}

	return false;
}

int8_t emcom_xengine_mpflow_checkstatus(struct sock *sk, int *reason, int state, struct emcom_xengine_mpflow_stat *node)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct inet_sock *inet = inet_sk(sk);
	int8_t result = EMCOM_MPFLOW_FALLBACK_NONE;
	int oldstate = sk->sk_state;

	if (*reason == EMCOM_MPFLOW_FALLBACK_NOPAYLOAD) {
#ifdef CONFIG_MPTCP
		if (mptcp_meta_sk(sk) == sk)
			return result;
#endif
		/* EST->DOWN */
		if ((oldstate == TCP_ESTABLISHED) && (state != TCP_ESTABLISHED)) {
			result = ((tp->bytes_received <= 1) && ((tp->bytes_acked > 1) ||
				(tp->snd_nxt - tp->snd_una > EMCOM_MPFLOW_SND_BYTE_THRESHOLD))) ?
				EMCOM_MPFLOW_FALLBACK_SET : EMCOM_MPFLOW_FALLBACK_CLR;
			if (node->mpflow_estlink > 0)
				node->mpflow_estlink--;
		/* DOWN->EST */
		} else if ((oldstate != TCP_ESTABLISHED) && (state == TCP_ESTABLISHED)) {
			result = EMCOM_MPFLOW_FALLBACK_SYNCLR;
			node->mpflow_estlink++;
		}
	} else if (*reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT) {
		result = ((1 << oldstate) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) ? EMCOM_MPFLOW_FALLBACK_SET :
			EMCOM_MPFLOW_FALLBACK_NONE;
	} else if (*reason == EMCOM_MPFLOW_FALLBACK_RETRANS) {
		result = EMCOM_MPFLOW_FALLBACK_SET;
		if (((1 << oldstate) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) &&
			(tp->segs_out == EMCOM_MPFLOW_FALLBACK_SYN_TOUT_TIME))
			*reason = EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST;
	} else {
		result = EMCOM_MPFLOW_FALLBACK_SET;
	}

	if (result != EMCOM_MPFLOW_FALLBACK_NONE)
		EMCOM_LOGD("MpFlow checkinfo uid:%d sk:%pK src_addr:"IPV4_FMT":%d dst_addr:"IPV4_FMT":%d inf:%d[%s]"
					"R:%d ost:%d nst:%d P[%d->%d] ret:%d,snt,fly,ack,rcv:%u,%u,%llu,%llu\n",
					node->uid, sk, IPV4_INFO(sk->sk_rcv_saddr), sk->sk_num, IPV4_INFO(sk->sk_daddr),
					ntohs(sk->sk_dport), node->ifindex, node->name,
					*reason, oldstate, state, ntohs(inet->inet_sport), ntohs(inet->inet_dport),
					result, tp->segs_out, tp->snd_nxt - tp->snd_una, tp->bytes_acked, tp->bytes_received);

	return result;
}

void emcom_xengine_mpflow_fallback(struct sock *sk, int reason, int state)
{
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct net *net = sock_net(sk);
	struct net_device *dev = NULL;
	char name[IFNAMSIZ];
	int8_t result;
	errno_t err;
	struct emcom_xengine_mpflow_fallback_ver uid_info;

	/* If the sk not bind yet, not need fallback check */
	if (!sk->sk_bound_dev_if)
		return;

	uid_info.uid = sock_i_uid(sk).val;
	if (invalid_uid(uid_info.uid))
		return;

	uid_info.index = emcom_xengine_mpflow_finduid(uid_info.uid);
	if (uid_info.index != INDEX_INVALID) {
		uid_info.ver = EMCOM_MPFLOW_VER_V1;
	} else {
		uid_info.index = emcom_xengine_mpflow_ai_finduid(uid_info.uid);
		if (uid_info.index != INDEX_INVALID) {
			uid_info.ver = EMCOM_MPFLOW_VER_V2;
		} else {
			return;
		}
	}

	rcu_read_lock();
	dev = dev_get_by_index_rcu(net, sk->sk_bound_dev_if);
	if (!dev || (dev->name[0] == '\0')) {
		rcu_read_unlock();
		EMCOM_LOGE("get dev name failed.dev[%pK] sk[%pK] dev_if[%d]", dev, sk, sk->sk_bound_dev_if);
		return;
	}

	err = strncpy_s(name, IFNAMSIZ, dev->name, IFNAMSIZ - 1);
	if (err != 0) {
		rcu_read_unlock();
		EMCOM_LOGE("strncpy_s failed, errcode: %d", err);
		return;
	}
	rcu_read_unlock();

	node = emcom_xengine_mpflow_get(uid_info.uid, name, sk->sk_bound_dev_if, uid_info.ver);
	if (!node)
		return;

	result = emcom_xengine_mpflow_checkstatus(sk, &reason, state, node);
	if (result == EMCOM_MPFLOW_FALLBACK_SET) {
		if (emcom_xengine_mpflow_errlink(reason, node) || emcom_xengine_mpflow_retrans(reason, node)) {
			struct emcom_xengine_mpflow_fallback fallback;

			node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_SET;
			EMCOM_LOGE("MpFlow fallback uid:%d inf:%d(%s) estlink:%d nodata,rst,tout:%d,%d,%d\n",
					   node->uid, node->ifindex, node->name, node->mpflow_estlink,
					   node->mpflow_fail_nopayload, node->mpflow_fail_syn_rst, node->mpflow_fail_syn_timeout);

			/* report connection unreachabled */
			if (!strncmp(EMCOM_WLAN_IFNAME, name, strlen(name))) {
				fallback.uid = node->uid;
				fallback.reason = EMCOM_MPFLOW_FALLBACK_WLAN_OFFSET + reason;
				emcom_xengine_mpflow_report(&fallback, sizeof(fallback));
				EMCOM_LOGE("MpFlow fallback report uid:%d reason: %d\n", fallback.uid, fallback.reason);
			} else if (!strncmp(EMCOM_LTE_IFNAME, name, strlen(name)) && (reason != EMCOM_MPFLOW_FALLBACK_RETRANS)) {
				fallback.uid = node->uid;
				fallback.reason = EMCOM_MPFLOW_FALLBACK_LTE_OFFSET + reason;
				emcom_xengine_mpflow_report(&fallback, sizeof(fallback));
				EMCOM_LOGE("MpFlow fallback report uid:%d reason: %d\n", fallback.uid, fallback.reason);
			}
		}
	} else if (result == EMCOM_MPFLOW_FALLBACK_CLR) {
		node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
		node->mpflow_fail_nopayload = 0;
		node->mpflow_fail_syn_rst = 0;
		node->mpflow_fail_syn_timeout = 0;
	} else if (result == EMCOM_MPFLOW_FALLBACK_SYNCLR) {
		node->mpflow_fail_syn_rst = 0;
		node->mpflow_fail_syn_timeout = 0;
	}
	return;
}

char *strtok(char *string_org, const char *demial)
{
#define EMCOM_MPFLOW_FI_CHAR_MAP_NUM  32
#define EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT  3
#define EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK  7
	static unsigned char *last;
	unsigned char *str = NULL;
	const unsigned char *ctrl = (const unsigned char *)demial;
	unsigned char map[EMCOM_MPFLOW_FI_CHAR_MAP_NUM];
	int count;

	for (count = 0; count < EMCOM_MPFLOW_FI_CHAR_MAP_NUM; count++)
		map[count] = 0;

	do
		map[*ctrl >> EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT] |=
			(1 << (*ctrl & EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK));
	while (*ctrl++);

	if (string_org != NULL)
		str = (unsigned char *)string_org;
	else
		str = last;

	while ((map[*str >> EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT]
		& (1 << (*str & EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK)))
		&& *str)
		str++;

	string_org = (char *)str;
	for (; *str; str++) {
		if (map[*str >> EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT]
			& (1 << (*str & EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK))) {
			*str++ = '\0';
			break;
		}
	}
	last = str;
	if (string_org == (char *)str)
		return NULL;
	else
		return string_org;

#undef EMCOM_MPFLOW_FI_CHAR_MAP_NUM
#undef EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT
#undef EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK
}

static int emcom_xengine_mpflow_split(char *src, const char *separator, char **dest, int *num)
{
	char *p = NULL;
	int count = 0;

	if ((src == NULL) || (strlen(src) == 0) || (separator == NULL) || (strlen(separator) == 0))
		return *num;

	p = strtok(src, separator);
	while (p != NULL) {
		*dest++ = p;
		++count;
		p = strtok(NULL, separator);
	}
	*num = count;
	return *num;
}

static bool emcom_xengine_mpflow_ptn_init(struct emcom_xengine_mpflow_ptn ptn[], uint8_t *num, const char *hex)
{
	bool ret = false;
	char *revbuf[EMCOM_MPFLOW_FI_PTN_MAX_NUM] = {0};
	int n = 0;
	int i;
	errno_t err;

	EMCOM_LOGD("hex %s\n", hex);
	if (emcom_xengine_mpflow_split((char *)hex, EMCOM_MPFLOW_FI_PTN_SEPERATE_CHAR, revbuf, &n) == 0) {
		EMCOM_LOGE("hex split fail\n");
		return false;
	}

	for (i = 0; i < n; i++) {
		ptn[i].skip = NULL;
		ptn[i].shift = NULL;
		err = memset_s(ptn[i].ptn, EMCOM_MPFLOW_FI_PTN_MAX_SIZE, 0, EMCOM_MPFLOW_FI_PTN_MAX_SIZE);
		if (err != EOK) {
			EMCOM_LOGE("emcom_xengine_mpflow_ptn_init memset failed!");
			ptn[i].is_init = false;
			return false;
		}
		ptn[i].ptnlen = strnlen(revbuf[i], EMCOM_MPFLOW_FI_PTN_MAX_SIZE) >> 1;
		if (hex2bin(ptn[i].ptn, revbuf[i], ptn[i].ptnlen)) {
			ptn[i].is_init = false;
			EMCOM_LOGE("hex2bin fail\n");
			return false;
		}
		ret = emcom_xengine_mpflow_bm_build((const uint8_t *)ptn[i].ptn, ptn[i].ptnlen,
											&(ptn[i].skip), &(ptn[i].shift));
		if (!ret) {
			EMCOM_LOGE("emcom_xengine_mpflow_bm_build failed!\n");
			ptn[i].is_init = false;
			return false;
		}
		ptn[i].is_init = true;
		EMCOM_LOGD("ptn %s init succ!\n", ptn[i].ptn);
	}
	*num = n;
	return true;
}

static void emcom_xengine_mpflow_reset(struct tcp_sock *tp)
{
	struct sock *sk = (struct sock *)tp;

	EMCOM_LOGD("reset sk %pK sport is %u, state[%u]", tp, sk->sk_num, sk->sk_state);
	if (sk->sk_state == TCP_ESTABLISHED) {
		local_bh_disable();
		bh_lock_sock(sk);

		if (!sock_flag(sk, SOCK_DEAD)) {
			sk->sk_err = ECONNABORTED;
			/* This barrier is coupled with smp_rmb() in tcp_poll() */
			smp_wmb();
			sk->sk_error_report(sk);
			if (tcp_need_reset(sk->sk_state))
				tcp_send_active_reset(sk, sk->sk_allocation);
			tcp_done(sk);
		}

		bh_unlock_sock(sk);
		local_bh_enable();
	}
}

static void emcom_xengine_mpflow_download_flow_del(struct emcom_xengine_mpflow_iface *iface,
	struct emcom_xengine_mpflow_node *flow)
{
	struct sock *sk = (struct sock *)flow->tp;

	EMCOM_LOGD("remove sk: %pK sport: %u srtt_us: %u bytes_received: %u duration: %u", flow->tp, sk->sk_num,
			   flow->tp->srtt_us >> 3, flow->tp->bytes_received, jiffies_to_msecs(jiffies-flow->start_jiffies));
	sock_put(sk);
	list_del(&flow->list);
	kfree(flow);
	iface->flow_cnt--;
}

static void emcom_xengine_mpflow_update_wifi_pri(struct emcom_xengine_mpflow_iface *flows,
	struct emcom_xengine_mpflow_info *mpflow_uid, u16 flow_cnt)
{
	int i = 0;
	if (mpflow_uid->algorithm_type != EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI)
		return;

	flows->is_slow = 1;    // initialize the interface as slow
	flows->is_sure_no_slow = 0;    // but not sure the interface is actually slow

	if (flow_cnt <= 1) {
		flows->is_slow = 0;  // no active flow, reconsider the interface as fast but not sure
	} else if (flows->mean_srtt_ms < EMCOM_GOOD_RTT_THR_MS) {  // low rtt, iface surely fast
		flows->is_slow = 0;
		flows->is_sure_no_slow = 1;
	}

	if (flows->rate_received[0] == 0 &&
		flows->rate_received[1] == 0 &&
		flows->rate_received[2] == 0)
		flows->is_slow = 0;  // no bytes recieved for 3s, reconsider the iface as fast but not sure

	for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i >= 0; i--) {
		if (flows->rate_received[i] > EMCOM_GOOD_RECV_RATE_THR_BYTE_PER_SEC) {
			flows->is_slow = 0;  // recieve rate is more than xx KBps, interface is surely fast
			flows->is_sure_no_slow = 1;
			break;
		}
	}
}

static void emcom_xengine_mpflow_update(struct emcom_xengine_mpflow_iface *flows,
	struct emcom_xengine_mpflow_info *mpflow_uid)
{
#define EMCOM_MPFLOW_MICROSECOND_SMOOTH_RATE   8
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	struct sock *sk = NULL;
	u32 srtt_ms_sum = 0;
	u32 max_rate_received = 0;
	u16 flow_cnt = 0;
	int i;

	flows->is_slow = 1;
	flows->flow_valid_cnt = 0;
	for (i = EMCOM_MPFLOW_FI_STAT_SECONDS-1; i > 0; i--)
		flows->rate_received[i] = flows->rate_received[i-1];
	flows->rate_received[0] = 0;
	list_for_each_entry_safe(node, tmp, &flows->flows, list) {
		sk = (struct sock *)node->tp;
		/* now update interval is 1s, rcv_bytes is the rate */
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS-1; i > 0; i--)
			node->rate_received[i] = node->rate_received[i-1];
		node->rate_received[0] = (u32)(node->tp->bytes_received - node->last_bytes_received);
		/* srtt_us is smoothed round trip time << 3 in usecs */
		srtt_ms_sum += (node->tp->srtt_us / EMCOM_MPFLOW_MICROSECOND_SMOOTH_RATE) /
			USEC_PER_MSEC;
		flow_cnt++;
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i >= 0; i--) {
			if (node->rate_received[i] > EMCOM_MPFLOW_FLOW_SLOW_THREH) {
				flows->is_slow = 0;
				break;
			}
		}
		if (node->tp->bytes_received > EMCOM_MPFLOW_RATE_VALID_THREH)
			flows->flow_valid_cnt++;
		max_rate_received += node->rate_received[0];
		flows->bytes_received += (u32)(node->tp->bytes_received - node->last_bytes_received);
		flows->rate_received[0] += (u32)(node->tp->bytes_received - node->last_bytes_received);
		node->last_bytes_received = node->tp->bytes_received;
		if (sk->sk_state != TCP_ESTABLISHED)
			emcom_xengine_mpflow_download_flow_del(flows, node);
	}
	if (flow_cnt > 0)
		flows->mean_srtt_ms = srtt_ms_sum / flow_cnt;

	emcom_xengine_mpflow_update_wifi_pri(flows, mpflow_uid, flow_cnt);

	if (flows->flow_valid_cnt) {
		if (flows->max_rate_received_flow < max_rate_received / flows->flow_valid_cnt)
			flows->max_rate_received_flow = max_rate_received / flows->flow_valid_cnt;
	}

	if (flows->max_rate_received < max_rate_received)
		flows->max_rate_received = max_rate_received;
#undef EMCOM_MPFLOW_MICROSECOND_SMOOTH_RATE
}

static void emcom_xengine_mpflow_set_bind(struct emcom_xengine_mpflow_info *mpflow_uid, int bind_mode)
{
	switch (bind_mode) {
	case EMCOM_XENGINE_MPFLOW_BINDMODE_NORST:
		return;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST:
		mpflow_uid->rst_bind_mode = (mpflow_uid->wifi.max_rate_received >= mpflow_uid->lte.max_rate_received) ?
			EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI : EMCOM_XENGINE_MPFLOW_BINDMODE_LTE;
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI:
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI;
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE:
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_LTE;
		break;
	default:
		return;
	}

	mpflow_uid->rst_jiffies = jiffies;
	EMCOM_LOGD("uid %u rst_bind_mode %u rst_jiffies %lu", mpflow_uid->uid,
			   mpflow_uid->rst_bind_mode, mpflow_uid->rst_jiffies);
}

static bool emcom_xengine_mpflow_unbalance_netdisk(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (iface1->max_rate_received && (iface1->max_rate_received < (iface2->max_rate_received >> 1)) &&
		iface2->is_slow && (iface1->max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD) &&
		((iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		(!iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)))
		return true;
	else
		return false;
}

static bool emcom_xengine_mpflow_unbalance_market(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (iface1->max_rate_received &&
		((iface1->is_wifi && iface2->is_slow && (iface1->max_rate_received < (iface2->max_rate_received >> 1)) &&
		((g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM))) ||
		(!iface1->is_wifi && (iface1->max_rate_received < iface2->max_rate_received_flow) &&
		((g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)))))
		return true;
	else
		return false;
}

static bool emcom_xengine_mpflow_single_path(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (!g_mpflow_uids[index].rst_to_another && (iface1->flow_valid_cnt > 1) && !iface2->max_rate_received_flow &&
		((iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		(!iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)))
		return true;
	else
		return false;
}

static int emcom_xengine_mpflow_chk_rst_market(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;

	list_for_each_entry_safe(node, tmp, &iface1->flows, list) {
		if (time_before(jiffies, node->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH))
			continue;
		/* all downloading flows are on same iface */
		if ((node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH) &&
			emcom_xengine_mpflow_single_path(iface1, iface2, index)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = iface1->is_wifi ? EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE :
				EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
			g_mpflow_uids[index].rst_to_another = 1;
			break;
		}
		if ((iface1->bytes_received + iface2->bytes_received) <= EMCOM_MPFLOW_RST_IFACE_GOOD)
			break;
		/* iface1 is slower than half of iface2 */
		if (iface1->max_rate_received && (iface1->max_rate_received < (iface2->max_rate_received >> 1)))
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		/* both wifi and lte download over */
		if (iface1->is_slow && (iface2->is_slow || !iface2->bytes_received)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
		/* wifi is slower than half of lte or lte is slower than wifi */
		if ((node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH) &&
			emcom_xengine_mpflow_unbalance_market(iface1, iface2, index)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
	}
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_netdisk(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;

	list_for_each_entry_safe(node, tmp, &iface1->flows, list) {
		if (time_before(jiffies, node->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH) ||
			((iface1->bytes_received + iface2->bytes_received) <= EMCOM_MPFLOW_RST_IFACE_GOOD))
			continue;
		/* iface1 is slower than half of iface2 */
		if (iface1->max_rate_received &&
			(iface1->max_rate_received < (iface2->max_rate_received >> 1)))
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		/* both wifi and lte download over */
		if (iface1->is_slow && (iface2->is_slow || !iface2->bytes_received)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
		if ((node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH) &&
			emcom_xengine_mpflow_unbalance_netdisk(iface1, iface2, index))
			emcom_xengine_mpflow_reset(node->tp);
	}
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_weibo(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2)
{
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	uint32_t wifi_download_time;
	uint32_t lte_download_time;

	if (!iface1->max_rate_received_flow || !iface2->max_rate_received_flow ||
		time_before(jiffies, iface1->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH) ||
		time_before(jiffies, iface2->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH))
		return need_set_bind;
	wifi_download_time = MSEC_PER_SEC * EMCOM_MPFLOW_WEIBO_SIZE / iface1->max_rate_received;
	lte_download_time =  MSEC_PER_SEC * EMCOM_MPFLOW_WEIBO_SIZE / iface2->max_rate_received;
	if ((iface1->mean_srtt_ms + wifi_download_time) <
		(iface2->mean_srtt_ms + (lte_download_time / EMCOM_MPFLOW_DEV_NUM)))
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
	else if ((iface2->mean_srtt_ms + lte_download_time) <
		(iface1->mean_srtt_ms + (wifi_download_time / EMCOM_MPFLOW_DEV_NUM)))
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE;
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_wifi_pri(struct emcom_xengine_mpflow_iface *iface1,
	struct emcom_xengine_mpflow_iface *iface2, struct emcom_xengine_mpflow_info *mpflow_uid)
{
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;

	if (!iface1->max_rate_received_flow || !iface2->max_rate_received_flow) {
		EMCOM_LOGD("emcom_xengine_mpflow_chk_rst_wifi_pri, no enough dl return NORST\n");
		return need_set_bind;
	}

	if (time_before(jiffies, iface1->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH) ||
		time_before(jiffies, iface2->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH)) {
		EMCOM_LOGD("emcom_xengine_mpflow_chk_rst_wifi_pri, not timeout, return NORST\n");
		return need_set_bind;
	}

	if (!iface1->is_slow && iface1->is_sure_no_slow)
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
	else if (!iface2->is_slow && iface2->is_sure_no_slow)
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE;

	if (mpflow_uid->rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE &&
		(!iface1->is_slow && !iface1->is_sure_no_slow)) {
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	}

	return need_set_bind;
}

static void emcom_xengine_mpflow_timer(unsigned long arg)
{
	uint8_t index;
	bool need_reset_timer = false;
	int bind_mode_wifi = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	int bind_mode_lte = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	int bind_mode;
	struct emcom_xengine_mpflow_info *mpflow_uid = NULL;

	spin_lock(&g_mpflow_lock);
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		mpflow_uid = &g_mpflow_uids[index];
		if ((mpflow_uid->uid == UID_INVALID_APP) || !mpflow_uid->is_downloading)
			continue;
		emcom_xengine_mpflow_update(&mpflow_uid->wifi, mpflow_uid);
		emcom_xengine_mpflow_update(&mpflow_uid->lte, mpflow_uid);
		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_netdisk(&mpflow_uid->wifi, &mpflow_uid->lte, index);
			bind_mode_lte = emcom_xengine_mpflow_chk_rst_netdisk(&mpflow_uid->lte, &mpflow_uid->wifi, index);
		} else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_market(&mpflow_uid->wifi, &mpflow_uid->lte, index);
			bind_mode_lte = emcom_xengine_mpflow_chk_rst_market(&mpflow_uid->lte, &mpflow_uid->wifi, index);
		} else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_weibo(&mpflow_uid->wifi, &mpflow_uid->lte);
		} else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_wifi_pri(&mpflow_uid->wifi,
				&mpflow_uid->lte, mpflow_uid);
		}
		bind_mode = (bind_mode_wifi == EMCOM_XENGINE_MPFLOW_BINDMODE_NORST) ?
			bind_mode_lte : bind_mode_wifi;
		emcom_xengine_mpflow_set_bind(mpflow_uid, bind_mode);
		if (mpflow_uid->wifi.flow_cnt || mpflow_uid->lte.flow_cnt) {
			need_reset_timer = true;
		} else {
			EMCOM_LOGD("uid %u download is stop", mpflow_uid->uid);
			emcom_xengine_mpflow_download_finish(mpflow_uid);
			emcom_xengine_mpflow_fi_init(mpflow_uid);
		}
	}

	if (need_reset_timer) {
		mod_timer(&g_mpflow_tm, jiffies + HZ);
	} else {
		EMCOM_LOGD("stop mpflow timer");
		g_mpflow_tm_running = false;
	}
	spin_unlock(&g_mpflow_lock);
}

static void emcom_xengine_mpflow_download_flow_add(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	uid_t uid = sk->sk_uid.val;
	int index;
	struct dst_entry *dst = NULL;
	struct emcom_xengine_mpflow_iface *iface = NULL;
	struct emcom_xengine_mpflow_node *node = NULL;
	uint16_t port = sk->sk_num;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(uid);
	if (index == INDEX_INVALID) {
		EMCOM_LOGD("emcom_xengine_mpflow_finduid fail");
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}
	dst = sk_dst_get(sk);
	if (dst) {
		if (strncmp(EMCOM_WLAN_IFNAME, dst->dev->name, IFNAMSIZ) == 0) {
			iface = &g_mpflow_uids[index].wifi;
		} else if (strncmp(EMCOM_LTE_IFNAME, dst->dev->name, IFNAMSIZ) == 0) {
			iface = &g_mpflow_uids[index].lte;
		} else {
			EMCOM_LOGD("sk port %u iface is %s", port, dst->dev->name);
			dst_release(dst);
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
		dst_release(dst);
	} else {
		EMCOM_LOGD("sk port %u dst is not found", port);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	sk->is_download_flow = 1;

	node = kmalloc(sizeof(struct emcom_xengine_mpflow_node), GFP_ATOMIC);
	if (node) {
		int i;

		node->last_bytes_received = tp->bytes_received;
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i >= 0; i--)
			node->rate_received[i] = 0;
		node->tp = tp;
		node->start_jiffies = jiffies;
		sock_hold(sk);
		list_add(&node->list, &iface->flows);
		iface->flow_cnt++;
		if (!g_mpflow_uids[index].is_downloading) {
			EMCOM_LOGD("uid %u is_downloading", g_mpflow_uids[index].uid);
			g_mpflow_uids[index].is_downloading = 1;
			iface->start_jiffies = jiffies;
		}

		EMCOM_LOGD("sk %pK is a download flow sport %u ", tp, port);
		if (!g_mpflow_tm_running) {
			init_timer(&g_mpflow_tm);
			g_mpflow_tm.function = emcom_xengine_mpflow_timer;
			g_mpflow_tm.data     = (unsigned long)NULL;
			g_mpflow_tm_running = true;
			EMCOM_LOGD("start mpflow timer");
			mod_timer(&g_mpflow_tm, jiffies + HZ);
		}
	}
	spin_unlock_bh(&g_mpflow_lock);
}

static uint8_t *emcom_xengine_mpflow_make_skip(const uint8_t *ptrn, uint8_t plen)
{
	int i;
	uint8_t *skip = NULL;

	skip = (uint8_t *)kmalloc(EMCOM_MPFLOW_FI_ASCII_CODE_SIZE *
		 sizeof(uint8_t), GFP_ATOMIC);
	if (!skip)
		return NULL;

	for (i = 0; i < EMCOM_MPFLOW_FI_ASCII_CODE_SIZE; i++)
		*(skip + i) = (plen >= EMCOM_MPFLOW_FI_ASCII_CODE_MARK) ?
			EMCOM_MPFLOW_FI_ASCII_CODE_MARK : (plen + 1);
	while (plen != 0)
		*(skip + (uint8_t)*ptrn++) = plen--;
	return skip;
}

static uint8_t *emcom_xengine_mpflow_make_shift(const uint8_t *ptrn, uint8_t plen)
{
	uint8_t *sptr = NULL;
	const uint8_t *pptr = NULL;
	uint8_t c;
	uint8_t *shift = NULL;
	const uint8_t *p1 = NULL;
	const uint8_t *p2 = NULL;
	const uint8_t *p3 = NULL;

	shift = (uint8_t *)kmalloc(plen * sizeof(uint8_t), GFP_ATOMIC);
	if (!shift)
		return NULL;
	sptr = shift + plen - 1;
	pptr = ptrn + plen - 1;
	c = *(ptrn + plen - 1);
	*sptr = 1;

	while (sptr-- != shift) {
		p1 = ptrn + (plen - 1) - 1;
		do {
			while ((p1 >= ptrn) && (*p1-- != c))
				;
			p2 = ptrn + (plen - 1) - 1;
			p3 = p1;
			while ((p3 >= ptrn) && (*p3-- == *p2--) &&
				(p2 >= pptr))
				;
		} while ((p3 >= ptrn) && (p2 >= pptr));
		*sptr = shift + plen - sptr + p2 - p3;
		pptr--;
	}
	return shift;
}

static void emcom_xengine_mpflow_bm_free(uint8_t **skip, uint8_t **shift)
{
	if (*skip) {
		kfree(*skip);
		*skip = NULL;
	}
	if (*shift) {
		kfree(*shift);
		*shift = NULL;
	}
}

static bool emcom_xengine_mpflow_bm_build(const uint8_t *ptn, uint32_t ptnlen,
	uint8_t **skip, uint8_t **shift)
{
	if ((ptn != NULL) && (ptnlen > 0) && (skip != NULL) && (shift != NULL)) {
		*skip = emcom_xengine_mpflow_make_skip(ptn, ptnlen);
		if (!*skip)
			return false;
		*shift = emcom_xengine_mpflow_make_shift(ptn, ptnlen);
		if (!*shift) {
			kfree(*skip);
			*skip = NULL;
			return false;
		}
		return true;
	}
	return false;
}

static bool emcom_xengine_mpflow_bm_search(const uint8_t *buf, uint32_t buflen,
	const struct emcom_xengine_mpflow_ptn *sptn, uint32_t *offset)
{
	uint32_t pindex;
	uint8_t skip_stride;
	uint8_t shift_stride;
	uint32_t bindex = sptn->ptnlen;

	if ((buf == NULL) || (sptn->ptn == NULL) || (sptn->skip == NULL) || (sptn->shift == NULL)) {
		return false;
	}
	if ((sptn->ptnlen <= 0) || (buflen <= sptn->ptnlen)) {
		return false;
	}
	while (bindex <= buflen) {
		pindex = sptn->ptnlen;
		while (sptn->ptn[--pindex] == buf[--bindex]) {
			if (pindex != 0)
				continue;
			if (offset != NULL)
				*offset = bindex;
			return true;
		}
		skip_stride = sptn->skip[buf[bindex]];
		shift_stride = sptn->shift[pindex];
		bindex += ((skip_stride > shift_stride) ? skip_stride : shift_stride);
	}
	return false;
}

static void emcom_xengine_mpflow_ptn_deinit(struct emcom_xengine_mpflow_ptn ptn[], uint8_t num)
{
	uint8_t i;

	for (i = 0; i < num; i++) {
		if (ptn[i].is_init) {
			emcom_xengine_mpflow_bm_free(&(ptn[i].skip), &(ptn[i].shift));
			ptn[i].is_init = false;
		}
	}
}

static void emcom_xengine_mpflow_apppriv_deinit(struct emcom_xengine_mpflow_info *uid)
{
	if (!uid->app_priv)
		return;

	emcom_xengine_mpflow_hash_clear(uid->app_priv);
	kfree(uid->app_priv);
	uid->app_priv = NULL;
}

static void emcom_xengine_mpflow_fi_init(struct emcom_xengine_mpflow_info *mpflow_uid)
{
	errno_t err;

	err = memset_s(&mpflow_uid->wifi, sizeof(struct emcom_xengine_mpflow_iface),
				   0, sizeof(struct emcom_xengine_mpflow_iface));
	if (err != EOK)
		EMCOM_LOGE("emcom_xengine_mpflow_fi_init memset failed");
	mpflow_uid->wifi.is_wifi = 1;
	err = memset_s(&mpflow_uid->lte, sizeof(struct emcom_xengine_mpflow_iface),
				   0, sizeof(struct emcom_xengine_mpflow_iface));
	if (err != EOK)
		EMCOM_LOGE("emcom_xengine_mpflow_fi_init memset failed");
	mpflow_uid->lte.is_wifi = 0;
	INIT_LIST_HEAD(&mpflow_uid->wifi.flows);
	INIT_LIST_HEAD(&mpflow_uid->lte.flows);
	mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	mpflow_uid->rst_jiffies = 0;
	mpflow_uid->is_downloading = 0;
	mpflow_uid->rst_to_another = 0;
}

static bool emcom_xengine_mpflow_chk_download_flow(struct sk_buff *skb)
{
	int i;
	int index;
	uint16_t port;
	struct tcphdr *tcph = tcp_hdr(skb);
	uint16_t buflen = skb_headlen(skb);
	uint32_t offset = 0;

	port = ntohs(tcph->dest);
	/* download flow must be http(80) or https(443) */
	if ((port != EMCOM_MPFLOW_FI_PORT_80) && (port != EMCOM_MPFLOW_FI_PORT_443))
		return false;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(skb->sk->sk_uid.val);
	if (index == INDEX_INVALID) {
		EMCOM_LOGE("index is invalid\n");
		spin_unlock_bh(&g_mpflow_lock);
		return false;
	}
	if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO ||
		g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI) {
		spin_unlock_bh(&g_mpflow_lock);
		return true;
	} else if ((port == EMCOM_MPFLOW_FI_PORT_80) && (g_mpflow_uids[index].ptn_80_num != 0)) {
		for (i = 0; i < g_mpflow_uids[index].ptn_80_num; i++) {
			if (emcom_xengine_mpflow_bm_search((const uint8_t *)skb->data, buflen,
				(const struct emcom_xengine_mpflow_ptn *)&(g_mpflow_uids[index].ptn_80[i]),
				&offset)) {
				EMCOM_LOGD("received a port 80 packet match ptn: %s\n",
						   g_mpflow_uids[index].ptn_80[i].ptn);
				spin_unlock_bh(&g_mpflow_lock);
				return true;
			}
		}
	} else if ((port == EMCOM_MPFLOW_FI_PORT_443) && (g_mpflow_uids[index].ptn_443_num != 0)) {
		for (i = 0; i < g_mpflow_uids[index].ptn_443_num; i++) {
			if (emcom_xengine_mpflow_bm_search((const uint8_t *)skb->data, buflen,
				(const struct emcom_xengine_mpflow_ptn *)&(g_mpflow_uids[index].ptn_443[i]),
				&offset)) {
				EMCOM_LOGD("received a port 443 packet match ptn: %s\n",
						   g_mpflow_uids[index].ptn_443[i].ptn);
				spin_unlock_bh(&g_mpflow_lock);
				return true;
			}
		}
	}
	spin_unlock_bh(&g_mpflow_lock);
	return false;
}

static void emcom_xengine_mpflow_ai_udp_reset(struct sock *sk)
{
	local_bh_disable();
	bh_lock_sock(sk);
	sk->is_mp_flow_bind = 0;

	if (!sock_flag(sk, SOCK_DEAD)) {
		sk->sk_err = ECONNABORTED;
		smp_wmb();
		sk->sk_error_report(sk);
	}

	bh_unlock_sock(sk);
	local_bh_enable();
}

static void emcom_xengine_mpflow_ai_enable_selected_path(struct sock *sk, char *selected_path_iface)
{
	struct net_device *net_dev = dev_get_by_name(sock_net(sk), selected_path_iface);
	struct inet_sock *inet = NULL;
	if (net_dev) {
		unsigned int flags = dev_get_flags(net_dev);
		if (flags & IFF_RUNNING) {
			sk->sk_bound_dev_if = net_dev->ifindex;
			sk_dst_reset(sk);
			inet = inet_sk(sk);
			if (inet->inet_saddr)
				inet->inet_saddr = 0;
		}
		dev_put(net_dev);
	}
}

static bool emcom_xengine_mpflow_ai_rehash_sk(struct sock *sk)
{
	if (sk->sk_protocol == IPPROTO_UDP &&
#if IS_ENABLED(CONFIG_IPV6)
		(inet_sk(sk)->inet_rcv_saddr || (!ipv6_addr_any(&sk->sk_v6_rcv_saddr)))
#else
		(inet_sk(sk)->inet_rcv_saddr)
#endif
		) {
		if (sk->sk_prot && sk->sk_prot->rehash) {
			EMCOM_LOGD("UDP sock is already binded by user, rehash sk sk[%pK] sk_userlocks[%#x] "
				"dev_if[%d] inet_num[%d] hash[%d] inet_rcv_saddr: "IPV4_FMT" inet_saddr: "IPV4_FMT,
				sk, sk->sk_userlocks, sk->sk_bound_dev_if, inet_sk(sk)->inet_num, udp_sk(sk)->udp_portaddr_hash,
				IPV4_INFO(inet_sk(sk)->inet_rcv_saddr), IPV4_INFO(inet_sk(sk)->inet_saddr));
			inet_sk(sk)->inet_rcv_saddr = 0;
			inet_sk(sk)->inet_saddr = 0;

#if IS_ENABLED(CONFIG_IPV6)
			EMCOM_LOGD("[MPFlow_KERNEL] sk_v6_rcv_saddr: "IPV6_FMT, IPV6_INFO(sk->sk_v6_rcv_saddr));
			sk->sk_v6_rcv_saddr = in6addr_any;
#endif
			sk->sk_userlocks &= ~SOCK_BINDADDR_LOCK;
			sk->sk_prot->rehash(sk);
		} else {
			return false;
		}
	}
	return true;
}

static void emcom_xengine_mpflow_ai_path_handover(struct sock *sk)
{
	char *wifi = EMCOM_WLAN_IFNAME;
	char *lte = EMCOM_LTE_IFNAME;
	char *selected_path_iface = EMCOM_WLAN_IFNAME;

	if (!emcom_xengine_mpflow_ai_rehash_sk(sk))
		return;

	if (sk->sk_bound_dev_if) {
		struct net_device *net_dev = dev_get_by_index(sock_net(sk), sk->sk_bound_dev_if);
		if (net_dev) {
			if (!strncmp(net_dev->name, wifi, IFNAMSIZ))
				selected_path_iface = lte;
			dev_put(net_dev);
		}
	} else {
		selected_path_iface = lte;
	}
	EMCOM_LOGI("path_handover sk:%pK sport[%u] dev_if[%d] bind[%d] family[%u] dev[%s]",
		sk, sk->sk_num, sk->sk_bound_dev_if,
		sk->is_mp_flow_bind, sk->sk_family, selected_path_iface);
	emcom_xengine_mpflow_ai_enable_selected_path(sk, selected_path_iface);
}

extern struct inet_hashinfo tcp_hashinfo;
/*
 * mod flow local interface
 */
void emcom_xengine_mpflow_ai_reset_loc_intf(const uint8_t *data, uint16_t len)
{
	struct reset_flow_policy_info *reset = NULL;
	struct sock *sk = NULL;
	bool need_reset_device = false;
	int8_t app_index = INDEX_INVALID;

	if ((data == NULL) || (len < sizeof(struct reset_flow_policy_info))) {
		EMCOM_LOGE("mod flow pointer null or length %d error", len);
		return;
	}

	reset = (struct reset_flow_policy_info *)data;
	if (reset->flow.l3proto == ETH_P_IP) {
		EMCOM_LOGI("receive reset. SrcIP["IPV4_FMT"] SrcPort[%u] "
			"DstIP["IPV4_FMT"] DstPort[%u] l4proto[%u] l3proto[%u] intf[%u]",
			IPV4_INFO(reset->flow.ipv4_sip), reset->flow.src_port,
			IPV4_INFO(reset->flow.ipv4_dip), reset->flow.dst_port,
			reset->flow.l4proto, reset->flow.l3proto, reset->flow.sk_dev_itf);
	} else {
		EMCOM_LOGI("receive reset. SrcIP["IPV6_FMT"] SrcPort[%u] "
			"DstIP["IPV6_FMT"] DstPort[%u] l4proto[%u] l3proto[%u] intf[%u]",
			IPV6_INFO(reset->flow.ipv6_sip), reset->flow.src_port,
			IPV6_INFO(reset->flow.ipv6_dip), reset->flow.dst_port,
			reset->flow.l4proto, reset->flow.l3proto, reset->flow.sk_dev_itf);
	}
	EMCOM_LOGI("reset mode[%u] blinktime[%u]", reset->policy.rst_bind_mode, reset->policy.const_perid);

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(reset->uid);
	if (app_index == INDEX_INVALID) {
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	spin_unlock_bh(&g_mpflow_ai_lock);

	switch (reset->flow.l4proto) {
	case IPPROTO_TCP:
		if (reset->flow.l3proto == ETH_P_IP) {
			sk = inet_lookup_established(&init_net, &tcp_hashinfo,
				reset->flow.ipv4_dip, htons(reset->flow.dst_port),
				reset->flow.ipv4_sip, htons(reset->flow.src_port),
				reset->flow.sk_dev_itf);
		} else {
			sk = __inet6_lookup_established(&init_net, &tcp_hashinfo,
				&reset->flow.ipv6_dip, htons(reset->flow.dst_port),
				&reset->flow.ipv6_sip, reset->flow.src_port,
				reset->flow.sk_dev_itf, 0);
		}

		if (sk) {
			EMCOM_LOGI("reset sk:%pK sport[%u], state[%u] bind[%d] act[%u] family[%u] protocol[%u]",
				sk, sk->sk_num, sk->sk_state, sk->is_mp_flow_bind, reset->policy.act, sk->sk_family, sk->sk_protocol);
			if (reset->policy.act == SK_ERROR) {
				emcom_xengine_mpflow_reset(tcp_sk(sk));
				need_reset_device = true;
				EMCOM_LOGI("tcp reset completed");
			} else {
				EMCOM_LOGE("tcp reset action not support, action: %u", reset->policy.act);
			}
			sock_put(sk);
		} else {
			EMCOM_LOGE("tcp reset flow not found");
		}
		break;

	case IPPROTO_UDP:
		if (reset->flow.l3proto == ETH_P_IP) {
			sk = udp4_lib_lookup(&init_net,
				reset->flow.ipv4_dip, htons(reset->flow.dst_port),
				reset->flow.ipv4_sip, htons(reset->flow.src_port),
				reset->flow.sk_dev_itf);
		} else {
			sk = udp6_lib_lookup(&init_net,
				&reset->flow.ipv6_dip, htons(reset->flow.dst_port),
				&reset->flow.ipv6_sip, htons(reset->flow.src_port),
				reset->flow.sk_dev_itf);
		}

		if (sk) {
			EMCOM_LOGI("reset sk:%pK sport[%u], state[%u] bind[%d] act[%u] family[%u] protocol[%u]",
				sk, sk->sk_num, sk->sk_state, sk->is_mp_flow_bind, reset->policy.act, sk->sk_family, sk->sk_protocol);
			if (sk->sk_protocol == IPPROTO_UDP) {
				if (reset->policy.act == SK_ERROR) {
					emcom_xengine_mpflow_ai_udp_reset(sk);
					need_reset_device = true;
				} else if (reset->policy.act == INTF_CHANGE) {
					emcom_xengine_mpflow_ai_path_handover(sk);
				} else {
					EMCOM_LOGE("udp reset action not support, action: %u", reset->policy.act);
				}
				EMCOM_LOGI("udp reset completed");
			}
			sock_put(sk);
		} else {
			EMCOM_LOGE("udp flow not found");
		}
		break;
	default:
		EMCOM_LOGI("mpflow ai reset unsupport protocol: %d.\n", reset->flow.l4proto);
		break;
	}

	if (need_reset_device) {
		spin_lock_bh(&g_mpflow_ai_lock);
		app_index = emcom_xengine_mpflow_ai_finduid(reset->uid);
		if (app_index != INDEX_INVALID) {
			g_mpflow_ai_uids[app_index].rst_jiffies = jiffies;
			g_mpflow_ai_uids[app_index].rst_bind_mode = reset->policy.rst_bind_mode;
			g_mpflow_ai_uids[app_index].rst_duration = msecs_to_jiffies(reset->policy.const_perid);
			g_mpflow_ai_uids[app_index].rst_devif = reset->flow.sk_dev_itf;
		}
		spin_unlock_bh(&g_mpflow_ai_lock);
		if (reset->flow.l3proto == ETH_P_IP) {
			EMCOM_LOGI("[MPFlow_KERNEL]Reset Completed. SrcIP["IPV4_FMT"] SrcPort[%u] "
				"DstIP["IPV4_FMT"] DstPort[%u] l4proto[%u] intf[%u]",
				IPV4_INFO(reset->flow.ipv4_sip), reset->flow.src_port,
				IPV4_INFO(reset->flow.ipv4_dip), reset->flow.dst_port,
				reset->flow.l4proto, reset->flow.sk_dev_itf);
		} else {
			EMCOM_LOGI("[MPFlow_KERNEL]Reset Completed. SrcIP["IPV6_FMT"] SrcPort[%u] "
				"DstIP["IPV6_FMT"] DstPort[%u] l4proto[%u] intf[%u]",
				IPV6_INFO(reset->flow.ipv6_sip), reset->flow.src_port,
				IPV6_INFO(reset->flow.ipv6_dip), reset->flow.dst_port,
				reset->flow.l4proto, reset->flow.sk_dev_itf);
		}
	}
}

static unsigned int emcom_xengine_mpflow_hook_out_ipv4(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct sock *sk = skb->sk;
	struct iphdr *iph = ip_hdr(skb);
	struct tcphdr *tcph = tcp_hdr(skb);

	if (!sk || !iph || !tcph)
		return NF_ACCEPT;

	if ((sk->sk_state != TCP_ESTABLISHED) || (sk->sk_protocol != IPPROTO_TCP))
		return NF_ACCEPT;

	if (!sk->is_mp_flow || sk->is_download_flow || (sk->snd_pkt_cnt > 0))
		return NF_ACCEPT;

	if (skb_headlen(skb) <= (tcp_hdrlen(skb) + ip_hdrlen(skb)))
		return NF_ACCEPT;

	sk->snd_pkt_cnt++;
	if (emcom_xengine_mpflow_chk_download_flow(skb))
		emcom_xengine_mpflow_download_flow_add(sk);
	return NF_ACCEPT;
}

static unsigned int emcom_xengine_mpflow_hook_out_ipv6(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct sock *sk = skb->sk;
	struct ipv6hdr *iph = ipv6_hdr(skb);
	struct tcphdr *tcph = tcp_hdr(skb);

	if (!sk || !iph || !tcph)
		return NF_ACCEPT;

	if ((sk->sk_state != TCP_ESTABLISHED) || (sk->sk_protocol != IPPROTO_TCP))
		return NF_ACCEPT;

	if (!sk->is_mp_flow || sk->is_download_flow || (sk->snd_pkt_cnt > 0))
		return NF_ACCEPT;

	if (skb_headlen(skb) <= (tcp_hdrlen(skb) + sizeof(struct ipv6hdr)))
		return NF_ACCEPT;

	sk->snd_pkt_cnt++;
	if (emcom_xengine_mpflow_chk_download_flow(skb))
		emcom_xengine_mpflow_download_flow_add(sk);
	return NF_ACCEPT;
}

static const struct nf_hook_ops emcom_xengine_mpflow_nfhooks[] = {
	{
		.hook        = emcom_xengine_mpflow_hook_out_ipv4,
		.pf          = PF_INET,
		.hooknum     = NF_INET_LOCAL_OUT,
		.priority    = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook        = emcom_xengine_mpflow_hook_out_ipv6,
		.pf          = PF_INET6,
		.hooknum     = NF_INET_LOCAL_OUT,
		.priority    = NF_IP_PRI_FILTER + 1,
	},
};

static void emcom_xengine_mpflow_register_nf_hook(void)
{
	int ret;

	if (g_mpflow_nf_hook)
		return;

	ret = nf_register_net_hooks(&init_net, emcom_xengine_mpflow_nfhooks, ARRAY_SIZE(emcom_xengine_mpflow_nfhooks));
	if (!ret)
		g_mpflow_nf_hook = true;

	EMCOM_LOGD("start emcom_xengine_mpflow_nfhooks\n");
}

static void emcom_xengine_mpflow_unregister_nf_hook(void)
{
	if (!g_mpflow_nf_hook)
		return;

	nf_unregister_net_hooks(&init_net, emcom_xengine_mpflow_nfhooks, ARRAY_SIZE(emcom_xengine_mpflow_nfhooks));
	g_mpflow_nf_hook = false;
	EMCOM_LOGD("stop emcom_xengine_mpflow_nfhooks\n");
}

#ifdef CONFIG_MPTCP
void emcom_xengine_mptcp_socket_closed(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_SOCKET_CLOSED, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_socket_closed);

void emcom_xengine_mptcp_socket_switch(const  void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_SOCKET_SWITCH, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_socket_switch);

void emcom_xengine_mptcp_proxy_fallback(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_PROXY_FALLBACK, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_proxy_fallback);

void emcom_xengine_mptcp_fallback(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_FALLBACK, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_fallback);
#endif

/*
 * message proc , process every message for xengine module
 */
void emcom_xengine_evt_proc(int32_t event, const uint8_t *data, uint16_t len)
{
	switch (event) {
	case NETLINK_EMCOM_DK_START_ACC:
		EMCOM_LOGD("emcom netlink receive acc start");
		emcom_xengine_start_acc_uid(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_ACC:
		EMCOM_LOGD("emcom netlink receive acc stop");
		emcom_xengine_stop_acc_uid(data, len);
		break;
	case NETLINK_EMCOM_DK_CLEAR:
		EMCOM_LOGD("emcom netlink receive clear info");
		emcom_xengine_clear();
		break;
	case NETLINK_EMCOM_DK_RRC_KEEP:
		EMCOM_LOGD("emcom netlink receive rrc keep");
		emcom_xengine_rrckeep();
		break;
	case NETLINK_EMCOM_DK_KEY_PSINFO:
		EMCOM_LOGD("emcom netlink receive psinfo");
		emcom_send_keypsinfo(data, len);
		break;
	case NETLINK_EMCOM_DK_SPEED_CTRL:
		EMCOM_LOGD("emcom netlink receive speed control uid");
		emcom_xengine_set_speedctrl_info(data, len);
		break;
	case NETLINK_EMCOM_DK_START_UDP_RETRAN:
		EMCOM_LOGD("emcom netlink receive wifi udp start");
		emcom_xengine_start_udpretran(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_UDP_RETRAN:
		EMCOM_LOGD("emcom netlink receive wifi udp stop");
		emcom_xengine_stop_udpretran(data, len);
		break;
	case NETLINK_EMCOM_DK_CONFIG_MPIP:
		EMCOM_LOGD("emcom netlink receive btm config start");
		emcom_xengine_config_mpip(data, len);
		break;
	case NETLINK_EMCOM_DK_CLEAR_MPIP:
		EMCOM_LOGD("emcom netlink receive clear mpip config");
		emcom_xengine_clear_mpip_config(data, len);
		break;
	case NETLINK_EMCOM_DK_START_MPIP:
		EMCOM_LOGD("emcom netlink receive btm start");
		emcom_xengine_start_mpip(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_MPIP:
		EMCOM_LOGD("emcom netlink receive btm stop");
		emcom_xengine_stop_mpip(data, len);
		break;
	case NETLINK_EMCOM_DK_START_FAST_SYN:
		EMCOM_LOGD("emcom netlink receive fast syn start");
		emcom_xengine_start_fastsyn(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_FAST_SYN:
		EMCOM_LOGD("emcom netlink receive fast syn stop");
		emcom_xengine_stop_fastsyn(data, len);
		break;
#ifdef CONFIG_HUAWEI_OPMP
	case NETLINK_EMCOM_DK_OPMP_INIT_HEARTBEAT:
		EMCOM_LOGD("emcom netlink received opmp init heartbeat");
		opmp_event_process(event, data, len);
		break;
#endif
	case NETLINK_EMCOM_DK_ACTIVE_CCALG:
		EMCOM_LOGD(" emcom netlink active congestion control algorithm");
		emcom_xengine_active_ccalg(data, len);
		break;
	case NETLINK_EMCOM_DK_DEACTIVE_CCALG:
		EMCOM_LOGD(" emcom netlink deactive congestion control algorithm");
		emcom_xengine_deactive_ccalg(data, len);
		break;
	case NETLINK_EMCOM_DK_START_MPFLOW:
		EMCOM_LOGD(" emcom netlink start mpflow control algorithm");
		emcom_xengine_mpflow_start(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_MPFLOW:
		EMCOM_LOGD(" emcom netlink stop mpflow control algorithm");
		emcom_xengine_mpflow_stop(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_RST_LOC_INTF:
		EMCOM_LOGD(" emcom netlink mod local interface");
		emcom_xengine_mpflow_ai_reset_loc_intf(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_BIND_IP_POLICY:
		EMCOM_LOGD(" emcom netlink mpflow ip policy config");
		emcom_xengine_mpflow_ai_ip_config(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_INIT_BIND_CONFIG:
		EMCOM_LOGD(" emcom netlink mpflow init bind config");
		emcom_xengine_mpflow_ai_init_bind_config(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_BIND_PORT_POLICY:
		EMCOM_LOGD(" emcom netlink mpflow port policy config");
		emcom_xengine_mpflow_ai_iface_cfg(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_MPFLOW_V2:
		EMCOM_LOGD(" emcom netlink stop mpflow control algorithm v2");
		emcom_xengine_mpflow_ai_stop(data, len);
		break;
	default:
		EMCOM_LOGI("emcom Xengine unsupport packet, the type is %d.\n", event);
		break;
	}
}


int emcom_xengine_setproxyuid(struct sock *sk, const char __user *optval, int optlen)
{
	uid_t uid = 0;
	int ret;

	ret = -EINVAL;
	if (optlen != sizeof(uid_t))
		return ret;

	ret = -EFAULT;
	if (copy_from_user(&uid, optval, optlen))
		return ret;

	lock_sock(sk);
	sk->sk_uid.val = uid;
	release_sock(sk);
	EMCOM_LOGD("hicom set proxy uid, uid: %u", sk->sk_uid.val);
	ret = 0;

	return ret;
}

int emcom_xengine_setsockflag(struct sock *sk, const char __user *optval, int optlen)
{
	int ret;
	int hicom_flag = 0;

	ret = -EINVAL;
	if (optlen != sizeof(uid_t))
		return ret;

	ret = -EFAULT;
	if (copy_from_user(&hicom_flag, optval, optlen))
		return ret;

	lock_sock(sk);
	sk->hicom_flag = hicom_flag;
	release_sock(sk);

	EMCOM_LOGD(" hicom set proxy flag, uid: %u, flag: %x", sk->sk_uid.val, \
		sk->hicom_flag);
	ret = 0;

	return ret;
}

static LIST_HEAD(bind2device_list);
static int emcom_xengine_bind2device_uid(struct emcom_xengine_bind2device_node *cfg)
{
	struct emcom_xengine_bind2device_node *node = NULL;
	bool is_add_list = false;
	int err;

	rcu_read_lock();
	list_for_each_entry_rcu(node, &bind2device_list, list) {
		if (node->uid == cfg->uid) {
			if (cfg->add) {
				err = strncpy_s(node->iface, IFNAMSIZ, cfg->iface, IFNAMSIZ - 1);
				if (err != EOK)
					EMCOM_LOGE("strncpy_s failed");
				rcu_read_unlock();
				return is_add_list;
			} else {
				list_del_rcu(&node->list);
				rcu_read_unlock();
				kfree(node);
				return is_add_list;
			}
		}
	}

	if (cfg->add) {
		list_add_rcu(&cfg->list, &bind2device_list);
		is_add_list = true;
	}
	rcu_read_unlock();
	return is_add_list;
}

static struct file *emcom_xengine_fget_by_pid(pid_t pid, unsigned int fd, fmode_t mask)
{
	struct task_struct *task = NULL;
	struct file *file = NULL;
	struct files_struct *files = NULL;

	rcu_read_lock();

	task = find_task_by_vpid(pid);
	if (!task) {
		rcu_read_unlock();
		return NULL;
	}

	files = task->files;
	if (!files) {
		rcu_read_unlock();
		return NULL;
	}
loop:
	file = fcheck_files(files, fd);
	if (file) {
		/* File object ref couldn't be taken.
		 * dup2() atomicity guarantee is the reason
		 * we loop to catch the new file (or NULL pointer)
		 */
		if (file->f_mode & mask)
			file = NULL;
		else if (!get_file_rcu(file))
			goto loop;
	}
	rcu_read_unlock();

	return file;
}

static int emcom_xengine_bind2device_fd(struct emcom_xengine_bind2device_node *cfg)
{
	struct file *file = NULL;
	struct socket *sock = NULL;
	struct sock *sk = NULL;
	pid_t pid = cfg->pid;
	int fd = cfg->fd;
	struct net_device *dev = NULL;
	int err;

	EMCOM_LOGD("uid %u pid_fd:%d_%d\n", cfg->uid, pid, fd);
	file = emcom_xengine_fget_by_pid(pid, fd, 0);
	if (!file) {
		EMCOM_LOGD("get file fail by pid_fd:%d_%d\n", pid, fd);
		return -EBADF;
	}

	sock = sock_from_file(file, &err);
	if (!sock) {
		EMCOM_LOGD("get socket fail by pid_fd:%d_%d\n", pid, fd);
		fput_by_pid(pid, file);
		return -EBADF;
	}

	sk = sock->sk;
	if (!sk || sk->sk_uid.val != cfg->uid) {
		EMCOM_LOGD("get sk fail by pid_fd:%d_%d\n", pid, fd);
		fput_by_pid(pid, file);
		return -EBADF;
	}

	sock_hold(sk);
	rcu_read_lock();
	local_bh_disable();
	bh_lock_sock(sk);

	dev = dev_get_by_name_rcu(sock_net(sk), cfg->iface);
	if (dev) {
		unsigned int flags = dev_get_flags(dev);

		if (flags & IFF_RUNNING) {
			sk->sk_bound_dev_if = dev->ifindex;
			sk_dst_reset(sk);
		} else  {
			EMCOM_LOGD("iface %s is not on\n", cfg->iface);
		}
	} else {
		EMCOM_LOGD("iface %s is not exist\n", cfg->iface);
	}
	bh_unlock_sock(sk);
	local_bh_enable();
	sock_put(sk);
	fput_by_pid(pid, file);

	return 0;
}

int emcom_xengine_setbind2device(struct sock *sk, const char __user *optval, int optlen)
{
	int ret;
	bool need_free = true;
	struct emcom_xengine_bind2device_node *node = NULL;

	if (optlen != sizeof(struct emcom_xengine_bind2device_node))
		return -EINVAL;

	node = kmalloc(sizeof(struct emcom_xengine_bind2device_node), GFP_ATOMIC);
	if (!node)
		return -ENOMEM;

	if (copy_from_user(node, optval, optlen)) {
		kfree(node);
		return -EFAULT;
	}

	if (node->uid <= UID_APP || node->iface[0] == '\0' ||
	    strnlen(node->iface, IFNAMSIZ) >= IFNAMSIZ) {
		kfree(node);
		return -EINVAL;
	}

	if (!node->pid && !node->fd) {
		ret = emcom_xengine_bind2device_uid(node);
		if (ret) {
			need_free = false;
			ret = 0;
		}
	} else {
		ret = emcom_xengine_bind2device_fd(node);
	}

	if (ret)
		EMCOM_LOGD("uid: %u pid:%d fd:%d iface:%s", node->uid,
			   node->pid, node->fd, node->iface);

	if (need_free)
		kfree(node);
	return ret;
}

void emcom_xengine_bind(struct sock *sk)
{
	uid_t uid = sk->sk_uid.val;
	struct net_device *dev = NULL;
	struct emcom_xengine_bind2device_node *node = NULL;

	if (uid <= UID_APP)
		return;

	rcu_read_lock();
	list_for_each_entry_rcu(node, &bind2device_list, list) {
		if (node->uid == uid) {
			dev = dev_get_by_name_rcu(sock_net(sk), node->iface);
			if (dev) {
				unsigned int flags = dev_get_flags(dev);

				if (flags & IFF_RUNNING) {
					sk->sk_bound_dev_if = dev->ifindex;
					sk_dst_reset(sk);
				} else {
					EMCOM_LOGD("iface %s is not on\n", node->iface);
				}
			} else {
				EMCOM_LOGD("iface %s is not exist\n", node->iface);
			}
			break;
		}
	}
	rcu_read_unlock();
}

void emcom_xengine_notify_sock_error(struct sock *sk)
{
	if (sk->hicom_flag == HICOM_SOCK_FLAG_FINTORST) {
		EMCOM_LOGD(" hicom change fin to rst, uid: %u, flag: %x", sk->sk_uid.val, sk->hicom_flag);
		sk->sk_err = ECONNRESET;
		sk->sk_error_report(sk);
	}

	return;
}

void emcom_xengine_mpflow_ai_app_clear(int8_t index, uid_t uid)
{
	struct emcom_xengine_mpflow_ip_bind_policy *ip = NULL;
	struct hlist_node *tmp = NULL;
	struct emcom_xengine_mpflow_ai_priv *priv = NULL;
	int i;

	priv = g_mpflow_ai_uids[index].priv;
	if (!priv)
		return;

	for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[i], node) {
			if (ip->uid == uid) {
				hlist_del(&ip->node);
				kfree(ip);
			}
		}
	}

	kfree(g_mpflow_ai_uids[index].priv);
	g_mpflow_ai_uids[index].priv = NULL;
	g_mpflow_ai_uids[index].uid = 0;
	g_mpflow_ai_uids[index].enableflag = 0;
	g_mpflow_ai_uids[index].port_num = 0;
	g_mpflow_ai_uids[index].rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	g_mpflow_ai_uids[index].rst_jiffies = 0;
	g_mpflow_ai_uids[index].wifi_devif = 0;
	g_mpflow_ai_uids[index].lte_devif = 0;
	g_mpflow_ai_uids[index].rst_devif = 0;
}

int8_t emcom_xengine_mpflow_ai_getfreeindex(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_AI_MAX_APP; index++) {
		if (g_mpflow_ai_uids[index].uid == UID_INVALID_APP)
			return index;
	}
	return INDEX_INVALID;
}

int8_t emcom_xengine_mpflow_ai_finduid(uid_t uid)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_AI_MAX_APP; index++) {
		if (g_mpflow_ai_uids[index].uid == uid)
			return index;
	}
	return INDEX_INVALID;
}

static struct emcom_xengine_mpflow_ip_bind_policy*
emcom_xengine_mpflow_ai_hash(__be32 *addr, struct hlist_head *hashtable)
{
	struct emcom_xengine_mpflow_ip_bind_policy *policy = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t i, hash;

	hash = emcom_xengine_mpflow_ip_hash(*(addr + EMCOM_MPFLOW_AI_CLAT_IPV6 - 1));

	hlist_for_each_entry_safe(policy, tmp, &hashtable[hash], node) {
		if (memcmp((const void *)(policy->addr), (const void *)addr, EMCOM_MPFLOW_AI_CLAT_IPV6 * sizeof(__be32)) == 0)
			return policy;
	}

	policy = kzalloc(sizeof(struct emcom_xengine_mpflow_ip_bind_policy), GFP_ATOMIC);
	if (!policy)
		return NULL;

	for (i = 0; i < EMCOM_MPFLOW_AI_CLAT_IPV6; i++)
		policy->addr[i] = addr[i];
	policy->lte_cnt = 0;
	policy->pattern.tot_cnt = 0;
	policy->bind_mode = EMCOM_MPFLOW_BIND_NONE;

	hlist_add_head(&policy->node, &hashtable[hash]);
	return policy;
}

static void emcom_xengine_mpflow_ai_hash_delete(struct emcom_xengine_mpflow_ai_priv *priv, __be32 *addr)
{
	struct emcom_xengine_mpflow_ip_bind_policy *ip = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash;

	hash = emcom_xengine_mpflow_ip_hash(*(addr + EMCOM_MPFLOW_AI_CLAT_IPV6 - 1));
	hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[hash], node) {
		if (memcmp((const void *)(ip->addr), (const void *)addr, EMCOM_MPFLOW_AI_CLAT_IPV6 * sizeof(__be32)) == 0) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}
}

static void emcom_xengine_mpflow_ai_hash_clear(struct emcom_xengine_mpflow_ai_priv *priv)
{
	struct emcom_xengine_mpflow_ip_bind_policy *ip = NULL;
	struct hlist_node *tmp = NULL;
	int i;

	for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[i], node) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}
}

static bool emcom_xengine_mpflow_ai_get_addr(uint16_t sa_family, struct in_addr *addr, __be32 *s_addr)
{
	uint8_t i;

	if (sa_family == AF_INET) {
		struct in_addr *usin = (struct in_addr *)addr;
		*(s_addr + EMCOM_MPFLOW_AI_CLAT_IPV6 - 1) = usin->s_addr;
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (sa_family == AF_INET6) {
		struct in6_addr *usin6 = (struct in6_addr *)addr;
		for (i = 0; i < EMCOM_MPFLOW_AI_CLAT_IPV6; i++)
			*(s_addr + i) = usin6->s6_addr32[i];
		return true;
	}
#endif
	else {
		return false;
	}
}

void emcom_xengine_mpflow_ai_ip_config(const char *data, uint16_t len)
{
	struct emcom_xengine_mpflow_ai_ip_cfg *config = NULL;
	struct emcom_xengine_mpflow_ip_bind_policy *ip = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	__be32 daddr[EMCOM_MPFLOW_AI_CLAT_IPV6] = {0};
	int8_t app_index;

	if (!data || (len % sizeof(struct emcom_xengine_mpflow_ai_ip_cfg))) {
		EMCOM_LOGE("invalid data, length: %u expect: %u", len,
			sizeof(struct emcom_xengine_mpflow_ai_ip_cfg));
		return;
	}

	config = (struct emcom_xengine_mpflow_ai_ip_cfg *)data;

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(config->uid);
	if (app_index == INDEX_INVALID) {
		EMCOM_LOGE("get app fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	app = &g_mpflow_ai_uids[app_index];
	if (!emcom_xengine_mpflow_ai_get_addr(config->sa_family, &config->v4addr, daddr)) {
		EMCOM_LOGE("get address fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	ip = emcom_xengine_mpflow_ai_hash(daddr, app->priv->hashtable);
	if (!ip) {
		EMCOM_LOGE("hash fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	if (config->sa_family == AF_INET) {
		EMCOM_LOGI("ip policy config. dest ip:"IPV4_FMT", bindmode: %u, uid: %u",
			IPV4_INFO(config->v4addr), config->bind_mode, config->uid);
	} else if (config->sa_family == AF_INET6) {
		EMCOM_LOGI("ip policy config. dest ip:"IPV6_FMT", bindmode: %u, uid: %u",
			IPV6_INFO(config->v6addr), config->bind_mode, config->uid);
	}
	if (config->bind_mode != EMCOM_MPFLOW_BIND_NONE) {
		ip->uid = config->uid;
		ip->bind_mode = config->bind_mode;
	} else {
		emcom_xengine_mpflow_ai_hash_delete(app->priv, daddr);
	}

	spin_unlock_bh(&g_mpflow_ai_lock);
}

void emcom_xengine_mpflow_ai_init_bind_config(const char *data, uint16_t len)
{
	struct emcom_xengine_mpflow_ai_init_bind_cfg *config = NULL;
	struct emcom_xengine_mpflow_ai_init_bind_policy *policy = NULL;
	struct emcom_xengine_mpflow_ai_init_bind_policy *burst_policy = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	int8_t app_index;
	uint32_t index;
	uint32_t i, j;

	if (!data || (len != sizeof(struct emcom_xengine_mpflow_ai_init_bind_cfg))) {
		EMCOM_LOGE("input length error expect: %u, real: %u",
			sizeof(struct emcom_xengine_mpflow_ai_init_bind_cfg), len);
		return;
	}

	config = (struct emcom_xengine_mpflow_ai_init_bind_cfg *)data;
	if (!emcom_xengine_mpflow_ai_start(config->uid))
		return;

	EMCOM_LOGI("[MPFlow_KERNEL] Config received. policy num: %u", config->policy_num);

	if (config->policy_num > EMCOM_MPFLOW_BIND_PORT_CFG_SIZE) {
		EMCOM_LOGE("too many policy");
		return;
	}
	burst_policy = &config->burst_bind;
	EMCOM_LOGI("burst: proto[%u] mode[%u] ratio[%u:%u] portnum[%u]", burst_policy->l4_protocol, burst_policy->mode,
		burst_policy->ratio[WIFI_RATIO], burst_policy->ratio[LTE_RATIO], burst_policy->port_num);

	if (burst_policy->l4_protocol != 0) {
		if ((burst_policy->l4_protocol != IPPROTO_TCP) && (burst_policy->l4_protocol != IPPROTO_UDP)) {
			EMCOM_LOGE("burst: invalid protocol: %d", burst_policy->l4_protocol);
			return;
		}
	}

	if (burst_policy->mode > EMCOM_MPFLOW_IP_BURST_FIX) {
		EMCOM_LOGE("burst: invalid mode: %d", burst_policy->mode);
		return;
	}

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(config->uid);
	if (app_index == INDEX_INVALID) {
		EMCOM_LOGE("get app fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];
	app->burst_info.burst_port_num = 0;
	app->burst_info.burst_protocol = burst_policy->l4_protocol;
	app->burst_info.burst_mode = burst_policy->mode;
	app->burst_info.burst_ratio[WIFI_RATIO] = burst_policy->ratio[WIFI_RATIO];
	app->burst_info.burst_ratio[LTE_RATIO] = burst_policy->ratio[LTE_RATIO];

	j = 0;
	for (i = 0; i < burst_policy->port_num; i++) {
		EMCOM_LOGI("burst: port range[%d, %d]", burst_policy->port_range[i].start_port,
			burst_policy->port_range[i].end_port);

		if (burst_policy->port_range[i].start_port > burst_policy->port_range[i].end_port) {
			EMCOM_LOGE("burst: invalid port range");
			continue;
		}
		app->burst_info.burst_ports[j].start_port = burst_policy->port_range[i].start_port;
		app->burst_info.burst_ports[j].end_port = burst_policy->port_range[i].end_port;
		j++;
	}
	app->burst_info.burst_port_num = j;
	app->burst_info.launch_device = EMCOM_MPFLOW_BIND_WIFI;
	app->burst_info.burst_cnt = 0;
	app->burst_info.jiffies = 0;


	app->port_num = 0;
	for (index = 0; index < config->policy_num; index++) {
		policy = &config->scatter_bind[index];
		EMCOM_LOGI("scatter: proto[%u] mode[%u] ratio[%u:%u] portnum[%u]", policy->l4_protocol, policy->mode,
			policy->ratio[WIFI_RATIO], policy->ratio[LTE_RATIO], policy->port_num);

		if ((policy->l4_protocol != IPPROTO_TCP) && (policy->l4_protocol != IPPROTO_UDP)) {
			EMCOM_LOGE("invalid protocol: %d", policy->l4_protocol);
			continue;
		}
		if (policy->mode > EMCOM_MPFLOW_IP_BURST_FIX) {
			EMCOM_LOGE("invalid mode: %d", policy->mode);
			continue;
		}

		for (i = 0; i < policy->port_num; i++) {
			EMCOM_LOGI("port[%d, %d]", policy->port_range[i].start_port,
				policy->port_range[i].end_port);

			if (policy->port_range[i].start_port > policy->port_range[i].end_port) {
				EMCOM_LOGE("error port");
				continue;
			}
			app->ports[app->port_num].protocol = policy->l4_protocol;
			app->ports[app->port_num].range.start_port = policy->port_range[i].start_port;
			app->ports[app->port_num].range.end_port = policy->port_range[i].end_port;

			/* reserved for future logic */
			app->ports[app->port_num].pattern.select_mode = policy->mode;
			app->ports[app->port_num].pattern.ratio[WIFI_RATIO] = policy->ratio[WIFI_RATIO];
			app->ports[app->port_num].pattern.ratio[LTE_RATIO]  = policy->ratio[LTE_RATIO];

			app->ports[app->port_num].pattern.wifi_part = policy->ratio[WIFI_RATIO];
			app->ports[app->port_num].pattern.lte_part  = policy->ratio[LTE_RATIO];
			/* reserved for future logic end */

			if (policy->l4_protocol == IPPROTO_TCP) {
				app->ports[app->port_num].bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM;
			} else {
				app->ports[app->port_num].bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI;
			}
			app->port_num++;
			if (app->port_num > EMCOM_MPFLOW_BIND_PORT_SIZE) {
				EMCOM_LOGE("too many port: %d", app->port_num);
				spin_unlock_bh(&g_mpflow_ai_lock);
				return;
			}
		}
	}
	spin_unlock_bh(&g_mpflow_ai_lock);
}

int8_t emcom_xengine_mpflow_ai_get_port_index(int8_t index, struct emcom_xengine_mpflow_dport_range *range, uint8_t proto)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct emcom_xengine_mpflow_dport_range *exist = NULL;
	int8_t i, port_index = EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

	if (app->port_num == 0)
		return EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

	for (i = 0; i < app->port_num; i++) {
		exist = &app->ports[i].range;
		if ((range->start_port == exist->start_port) && (range->end_port == exist->end_port)
			&& (proto == app->ports[i].protocol)) {
			port_index = i;
			break;
		}
	}
	return port_index;
}

void emcom_xengine_mpflow_ai_iface_cfg(const char *data, uint16_t len)
{
	struct emcom_xengine_mpflow_ai_iface_config *config = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	int8_t app_index;
	int8_t port_index;

	if (!data || (len != sizeof(struct emcom_xengine_mpflow_ai_iface_config))) {
		EMCOM_LOGE("invalid data length %u expect: %u", len,
			sizeof(struct emcom_xengine_mpflow_ai_iface_config));
		return;
	}

	config = (struct emcom_xengine_mpflow_ai_iface_config *)data;
	if (config->port_range.start_port > config->port_range.end_port) {
		EMCOM_LOGE("invalid port range[%d, %d]",
			config->port_range.start_port, config->port_range.end_port);
		return;
	}

	spin_lock_bh(&g_mpflow_ai_lock);

	app_index = emcom_xengine_mpflow_ai_finduid(config->uid);
	if (app_index == INDEX_INVALID) {
		EMCOM_LOGE("get app fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];

	port_index = emcom_xengine_mpflow_ai_get_port_index(app_index, &config->port_range, config->l4protocol);
	if (port_index == EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX) {
		EMCOM_LOGE("port range[%d, %d] not exist",
			config->port_range.start_port, config->port_range.end_port);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	if (config->bind_mode != app->ports[port_index].bind_mode) {
		if (config->bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_NONE && app->priv)
			emcom_xengine_mpflow_ai_hash_clear(app->priv);
		if (config->bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)
			emcom_xengine_mpflow_clear_blocked(config->uid, EMCOM_MPFLOW_VER_V2);
		app->ports[port_index].pattern.tot_cnt = 0;
		EMCOM_LOGI("iface policy port[%d, %d] bind mode: %u",
			config->port_range.start_port, config->port_range.end_port, config->bind_mode);
	}
	app->ports[port_index].bind_mode = config->bind_mode;

	spin_unlock_bh(&g_mpflow_ai_lock);
}

int emcom_xengine_mpflow_ai_bind_random(struct emcom_xengine_mpflow_ai_bind_pattern *calc, uid_t uid)
{
	int bind_device;
	bool is_wifi_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_lte_block = emcom_xengine_mpflow_blocked(uid, EMCOM_LTE_IFNAME, EMCOM_MPFLOW_VER_V2);

	if (calc->last_device == EMCOM_MPFLOW_BIND_WIFI) {
		bind_device = EMCOM_MPFLOW_BIND_LTE;
	} else {
		bind_device = EMCOM_MPFLOW_BIND_WIFI;
	}
	EMCOM_LOGI("mpflow ai bind random to device: %d, blocked:%d, %d", bind_device, is_wifi_block, is_lte_block);

	if ((is_wifi_block == true) && (is_lte_block == true))
		bind_device = EMCOM_MPFLOW_BIND_NONE;
	else if ((is_wifi_block == true) && (bind_device == EMCOM_MPFLOW_BIND_WIFI))
		bind_device = EMCOM_MPFLOW_BIND_LTE;
	else if ((is_lte_block == true) && (bind_device == EMCOM_MPFLOW_BIND_LTE))
		bind_device = EMCOM_MPFLOW_BIND_WIFI;

	calc->last_device = bind_device;

	return bind_device;
}

bool emcom_xengine_mpflow_ai_start(uid_t uid)
{
	struct emcom_xengine_mpflow_ai_priv *priv = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	int i;
	int8_t index, newindex;
	bool is_new_uid = false;

	EMCOM_LOGD("mpflow ai start uid: %u", uid);

	spin_lock_bh(&g_mpflow_ai_lock);
	index = emcom_xengine_mpflow_ai_finduid(uid);
	if (index == INDEX_INVALID) {
		EMCOM_LOGD("new app uid: %d", uid);
		newindex = emcom_xengine_mpflow_ai_getfreeindex();
		if (newindex == INDEX_INVALID) {
			EMCOM_LOGE("mpflow ai start get free index exceed. uid: %d", uid);
			spin_unlock_bh(&g_mpflow_ai_lock);
			return false;
		}
		index = newindex;
		is_new_uid = true;
	}
	app = &g_mpflow_ai_uids[index];

	if (is_new_uid) {
		emcom_xengine_mpflow_clear_blocked(uid, EMCOM_MPFLOW_VER_V2);

		priv = kzalloc(sizeof(struct emcom_xengine_mpflow_ai_priv), GFP_ATOMIC);
		if (!priv) {
			EMCOM_LOGE("mpflow ai start alloc priv failed, uid: %d", uid);
			spin_unlock_bh(&g_mpflow_ai_lock);
			return false;
		}
		for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++)
			INIT_HLIST_HEAD(&priv->hashtable[i]);

		app->priv = priv;
		app->uid = uid;
		app->port_num = 0;
		app->enableflag = EMCOM_MPFLOW_AI_ENABLEFLAG_DPORT;
		app->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		app->rst_duration = EMCOM_MPFLOW_AI_RESET_DURATION;
		app->rst_jiffies = 0;
		app->wifi_devif = 0;
		app->lte_devif = 0;
		app->rst_devif = 0;
	}
	spin_unlock_bh(&g_mpflow_ai_lock);

	return true;
}

void emcom_xengine_mpflow_ai_stop(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_stop_info *stop = NULL;
	int8_t index;
	int32_t stop_reason;

	if (!pdata || (len != sizeof(struct emcom_xengine_mpflow_parse_stop_info))) {
		EMCOM_LOGE("mpflow ai stop data or length %d is error", len);
		return;
	}

	stop = (struct emcom_xengine_mpflow_parse_stop_info *)pdata;
	stop_reason = stop->stop_reason;
	EMCOM_LOGD("mpflow ai stop uid: %u, stop reason: %u", stop->uid, stop_reason);

	spin_lock_bh(&g_mpflow_ai_lock);
	index = emcom_xengine_mpflow_ai_finduid(stop->uid);
	if (index != INDEX_INVALID)
		emcom_xengine_mpflow_ai_app_clear(index, stop->uid);

	emcom_xengine_mpflow_delete(stop->uid, EMCOM_MPFLOW_VER_V2);
	spin_unlock_bh(&g_mpflow_ai_lock);
}

bool emcom_xengine_mpflow_ai_in_busrt_range(int8_t index, uint16_t dport, uint8_t proto)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct emcom_xengine_mpflow_dport_range *entry = NULL;
	int8_t i;

	if (app->burst_info.burst_protocol == 0 || app->burst_info.burst_port_num == 0) {
		return true;
	}

	if (app->burst_info.burst_protocol != proto) {
		return false;
	}

	for (i = 0; i < app->burst_info.burst_port_num; i++) {
		entry = &app->burst_info.burst_ports[i];
		if ((entry->start_port <= dport) && (dport <= entry->end_port)) {
			return true;
		}
	}

	return false;
}

int emcom_xengine_mpflow_ai_bind_burst(int8_t index, uint16_t dport, uint8_t proto,
	uint16_t qos_device, struct emcom_xengine_mpflow_ai_bind_pattern *pattern_calc, uid_t uid)
{
	bool is_wifi_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_lte_block = emcom_xengine_mpflow_blocked(uid, EMCOM_LTE_IFNAME, EMCOM_MPFLOW_VER_V2);
	if ((is_wifi_block && (qos_device == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(is_lte_block && (qos_device == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		((is_wifi_block == true) && (is_lte_block == true))) {
		EMCOM_LOGI("mpflow bind blocked uid: %u, bindmode: %d, blocked:%d, %d",
					uid, qos_device, is_wifi_block, is_lte_block);
		return EMCOM_MPFLOW_BIND_NONE;
	}

	if (emcom_xengine_mpflow_ai_in_busrt_range(index, dport, proto)) {
		int burst_device;
		struct emcom_xengine_mpflow_ai_burst_port *burst_info = &g_mpflow_ai_uids[index].burst_info;
		uint8_t wifi_ratio = burst_info->burst_ratio[WIFI_RATIO];
		uint8_t lte_ratio = burst_info->burst_ratio[LTE_RATIO];

		EMCOM_LOGI("burst current qos device: %u last burst qos device: %u, burst cnt: %u, ratio[%d, %d], mode[%u]",
			qos_device, burst_info->launch_device, burst_info->burst_cnt,
			wifi_ratio, lte_ratio, burst_info->burst_mode);

		if (burst_info->burst_mode != EMCOM_MPFLOW_IP_BURST_FIX)
			return qos_device;

		if (wifi_ratio == 0 && lte_ratio == 0)
			return qos_device;

		if (burst_info->launch_device != qos_device) {
			burst_info->burst_cnt = 0;
			burst_info->jiffies = jiffies;
			burst_info->launch_device = qos_device;
			return qos_device;
		}

		if (time_after(jiffies, burst_info->jiffies + EMCOM_MPFLOW_FLOW_BIND_BURST_TIME) ||
			(burst_info->jiffies == 0)) {
			burst_device = qos_device;
			burst_info->burst_cnt = 0;
			burst_info->jiffies = jiffies;
			burst_info->launch_device = burst_device; //update launch device every burst period begining
		} else {
			uint8_t launch_ratio, launch_cnt;
			uint16_t launch_device = burst_info->launch_device;
			uint16_t switch_device;

			if (launch_device == EMCOM_MPFLOW_BIND_WIFI) {
				launch_ratio = wifi_ratio;
				switch_device = EMCOM_MPFLOW_BIND_LTE;
			} else {
				launch_ratio = lte_ratio;
				switch_device = EMCOM_MPFLOW_BIND_WIFI;
			}

			launch_cnt = burst_info->burst_cnt % (wifi_ratio + lte_ratio);
			if (launch_cnt < launch_ratio) {
				burst_device = launch_device;
			} else {
				burst_device = switch_device;
				EMCOM_LOGD("burst to device: %u", switch_device);
			}
			burst_info->burst_cnt++;
		}

		return burst_device;
	}

	return qos_device;
}

int emcom_xengine_mpflow_ai_get_reset_device(struct emcom_xengine_mpflow_ai_info *app)
{
	int bind_device = EMCOM_MPFLOW_BIND_NONE;
	switch (app->rst_bind_mode) {
	case EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI:
		bind_device = EMCOM_MPFLOW_BIND_WIFI;
		break;

	case EMCOM_XENGINE_MPFLOW_BINDMODE_LTE:
		bind_device = EMCOM_MPFLOW_BIND_LTE;
		break;

	case EMCOM_XENGINE_MPFLOW_BINDMODE_SHIFT:
		if ((app->rst_devif == app->wifi_devif) || (app->rst_devif == 0))
			bind_device = EMCOM_MPFLOW_BIND_LTE;
		else if (app->rst_devif == app->lte_devif)
			bind_device = EMCOM_MPFLOW_BIND_WIFI;
		break;

	default:
		bind_device = EMCOM_MPFLOW_BIND_NONE;
		break;
	}
	EMCOM_LOGI("mpflow ai bind using reset mode: %u, device: %u rst_devif: %u wifi If: %u, lte If: %u",
		app->rst_bind_mode, bind_device, app->rst_devif, app->wifi_devif, app->lte_devif);
	return bind_device;
}

int8_t emcom_xengine_mpflow_ai_get_port_index_in_range(int8_t index, uint16_t dport, uint8_t proto)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct emcom_xengine_mpflow_dport_range *exist = NULL;
	int8_t i, port_index = EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

	if (app->port_num == 0) {
		return EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;
	}

	for (i = 0; i < app->port_num; i++) {
		exist = &app->ports[i].range;
		if ((exist->start_port <= dport) && (dport <= exist->end_port) &&
			(app->ports[i].protocol == proto)) {
			port_index = i;
			break;
		}
	}
	return port_index;
}

static struct emcom_xengine_mpflow_ip_bind_policy*
emcom_xengine_mpflow_ai_hash_get_policy(__be32 *addr, struct hlist_head *hashtable)
{
	struct emcom_xengine_mpflow_ip_bind_policy *policy = NULL;
	struct emcom_xengine_mpflow_ip_bind_policy *result = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash;

	hash = emcom_xengine_mpflow_ip_hash(*(addr + EMCOM_MPFLOW_AI_CLAT_IPV6 - 1));

	hlist_for_each_entry_safe(policy, tmp, &hashtable[hash], node) {
		if (memcmp((const void *)(policy->addr), (const void *)addr, EMCOM_MPFLOW_AI_CLAT_IPV6 * sizeof(__be32)) == 0) {
			result = policy;
			break;
		}
	}

	return result;
}
static bool emcom_xengine_mpflow_ai_get_addr_port(struct sockaddr *addr, __be32 *s_addr, uint16_t *port)
{
	uint8_t i;

	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *usin = (struct sockaddr_in *)addr;
		*(s_addr + EMCOM_MPFLOW_AI_CLAT_IPV6 - 1) = usin->sin_addr.s_addr;
		*port = ntohs(usin->sin_port);
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;

		for (i = 0; i < EMCOM_MPFLOW_AI_CLAT_IPV6; i++)
			*(s_addr + i) = usin6->sin6_addr.s6_addr32[i];

		*port = ntohs(usin6->sin6_port);
		return true;
	}
#endif
	else {
		EMCOM_LOGE("sa_family error, sa_family: %hu", addr->sa_family);
		return false;
	}
}

int emcom_xengine_mpflow_ai_getmode(int8_t index, uid_t uid, struct sockaddr *uaddr, uint8_t proto)
{
	struct emcom_xengine_mpflow_ip_bind_policy *ip_policy = NULL;
	struct emcom_xengine_mpflow_ai_bind_pattern *pattern_calc = NULL;
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	__be32 daddr[EMCOM_MPFLOW_AI_CLAT_IPV6] = {0};
	uint16_t dport;
	uint16_t bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	int8_t port_index;

	/* if reset control destination device exist, apply it first*/
	if (app->rst_bind_mode != EMCOM_XENGINE_MPFLOW_BINDMODE_NONE) {
		if (time_after(jiffies, app->rst_jiffies + app->rst_duration)) {
			app->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		} else {
			return emcom_xengine_mpflow_ai_get_reset_device(app);
		}
	}

	if (!app->priv) {
		EMCOM_LOGE("mpflow ai get mod priv error, uid: %u", app->uid);
		return EMCOM_MPFLOW_BIND_NONE;
	}

	if (!emcom_xengine_mpflow_ai_get_addr_port(uaddr, daddr, &dport))
		return EMCOM_MPFLOW_BIND_NONE;

	port_index = emcom_xengine_mpflow_ai_get_port_index_in_range(index, dport, proto);
	if (port_index >= EMCOM_MPFLOW_BIND_PORT_SIZE) {
		EMCOM_LOGI("no port match");
		return EMCOM_MPFLOW_BIND_NONE;
	}

	/* if ip policy matched, use ip policy, else use port policy as default */
	ip_policy = emcom_xengine_mpflow_ai_hash_get_policy(daddr, app->priv->hashtable);
	if (ip_policy) {
		bind_mode = ip_policy->bind_mode;
		pattern_calc = &ip_policy->pattern;
		EMCOM_LOGI("ip match, bindmode: %u", bind_mode);
	} else {
		bind_mode = app->ports[port_index].bind_mode;
		pattern_calc = &app->ports[port_index].pattern;
		EMCOM_LOGI("port match, bindmode: %u", bind_mode);
	}

	switch (bind_mode) {
	case  EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM:
		bind_mode = emcom_xengine_mpflow_ai_bind_random(pattern_calc, uid);
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI:
	case EMCOM_XENGINE_MPFLOW_BINDMODE_LTE:
		bind_mode = emcom_xengine_mpflow_ai_bind_burst(index, dport, proto, bind_mode, pattern_calc, uid);
		break;

	default:
		bind_mode = EMCOM_MPFLOW_BIND_NONE;
		EMCOM_LOGD("mpflow ai get spec mod error, mode: %u", bind_mode);
		break;
	}

	return bind_mode;
}

bool emcom_xengine_mpflow_ai_check_port(int8_t index, uint16_t dport)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct emcom_xengine_mpflow_dport_range *range = NULL;
	bool is_in_range = false;
	int i;

	if (app->port_num == 0)
		return true;

	for (i = 0; i < app->port_num; i++) {
		range = &app->ports[i].range;
		if (mpflow_ai_in_range(dport, range->start_port, range->end_port)) {
			is_in_range = true;
			break;
		}
	}
	return is_in_range;
}

bool emcom_xengine_mpflow_ai_checkvalid(struct sock *sk, struct sockaddr *uaddr, int8_t index, uint16_t *dport)
{
	struct emcom_xengine_mpflow_ai_info	*app = &g_mpflow_ai_uids[index];
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	bool isvalidaddr = false;
	int8_t port_index = EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

	if (!sk || !uaddr)
		return false;

	isvalidaddr = emcom_xengine_check_ip_addrss(uaddr) && (!emcom_xengine_check_ip_is_private(uaddr));
	if (isvalidaddr == false) {
		EMCOM_LOGD("invalid addr. uid: %u", app->uid);
		return false;
	}

	if (app->enableflag & EMCOM_MPFLOW_AI_ENABLEFLAG_DPORT) {
		if (usin->sin_family == AF_INET) {
			*dport = ntohs(usin->sin_port);
		} else if (usin->sin_family == AF_INET6) {
			struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)uaddr;
			*dport = (uint16_t)ntohs(usin6->sin6_port);
		} else {
			EMCOM_LOGD("unsupport family uid: %u, sin_family: %d",
						app->uid, usin->sin_family);
			return false;
		}

		port_index = emcom_xengine_mpflow_ai_get_port_index_in_range(index, *dport, sk->sk_protocol);
		if (port_index == EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX) {
				EMCOM_LOGD("port not in range uid: %u, dport: %d",
							app->uid, *dport);
				return false;
		}

		EMCOM_LOGD("mpflow ai check uid: %u sk: %pK, famliy: %d, sk_proto: %d, "
					"policy_proto: %d, port: %d bindmode: %u",
					app->uid, sk, sk->sk_family, sk->sk_protocol,
					app->ports[port_index].protocol, *dport,
					app->ports[port_index].bind_mode);

		if (sk->sk_protocol != app->ports[port_index].protocol) {
			EMCOM_LOGD("protocol not match uid: %u, sk: %pK", app->uid, sk);
			return false;
		}
	} else {
		EMCOM_LOGD("mpflow ai valid uid: %u enableflag: %u sk: %pK, famliy: %d, sk_proto: %d, "
					"policy_proto: %d, port: %d bindmode: %u",
					app->uid, app->enableflag, sk, sk->sk_family, sk->sk_protocol,
					app->ports[port_index].protocol, *dport,
					app->ports[port_index].bind_mode);
	}

	return true;
}

void emcom_xengine_mpflow_ai_bind2device(struct sock *sk, struct sockaddr *uaddr)
{
	struct net_device *dev = NULL;
	uint16_t dport;
	errno_t err;
	char ifname[IFNAMSIZ] = {0};
	int bind_device;
	uid_t uid;
	int8_t index;
	struct sockaddr_in *usin = NULL;
	struct inet_sock *inet = NULL;

	if (!sk || !uaddr)
		return;
	if (sk->is_mp_flow_bind)
		return;
	sk->is_mp_flow_bind = 1;

	uid = sock_i_uid(sk).val;
	if (invalid_uid(uid))
		return;

	spin_lock_bh(&g_mpflow_ai_lock);
	index = emcom_xengine_mpflow_ai_finduid(uid);
	if (index == INDEX_INVALID) {
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	if (emcom_xengine_mpflow_ai_checkvalid(sk, uaddr, index, &dport) == false) {
		EMCOM_LOGD("mpflow ai bind check invalid uid: %u", uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	bind_device = emcom_xengine_mpflow_ai_getmode(index, uid, uaddr, sk->sk_protocol);
	spin_unlock_bh(&g_mpflow_ai_lock);

	if (bind_device == EMCOM_MPFLOW_BIND_WIFI) {
		err = memcpy_s(ifname, sizeof(char) * IFNAMSIZ, EMCOM_WLAN_IFNAME, (strlen(EMCOM_WLAN_IFNAME) + 1));
	} else if (bind_device == EMCOM_MPFLOW_BIND_LTE) {
		err = memcpy_s(ifname, sizeof(char) * IFNAMSIZ, EMCOM_LTE_IFNAME, (strlen(EMCOM_LTE_IFNAME) + 1));
	} else if (bind_device == EMCOM_MPFLOW_BIND_NONE) {
		/* 1.wzry sgame stuck one min when shutdown LTE, because UDP not create a new sock when receive a reset,
		the old sock sk_bound_dev_if is LTE, so the UDP sock will continue use LTE to send packet
		2.as a wifi hotpoint, can not bind wlan, need reset sk_bound_dev_if to 0 */
		EMCOM_LOGD("EMCOM_MPFLOW_BIND_NONE uid: %u, sk: %pK, protocol: %u, DstPort: %u, dev_if: %u", uid, sk, sk->sk_protocol, dport, sk->sk_bound_dev_if);
		if (sk->sk_bound_dev_if && sk->sk_protocol == IPPROTO_UDP) {
			/* Fix the bug for weixin app bind wifi ip, then reset to lte, cannot find the sk */
			if (!emcom_xengine_mpflow_ai_rehash_sk(sk))
				return;
			sk->sk_bound_dev_if = 0;
			sk_dst_reset(sk);
			inet = inet_sk(sk);
			if (inet->inet_saddr)
				inet->inet_saddr = 0;
		} else {
			sk->sk_bound_dev_if = 0;
		}
		return;
	}

	if (err != EOK)
		EMCOM_LOGE("mpflow ai bind memcpy ifname failed");

	/* Fix the bug for weixin app bind wifi ip, then reset to lte, cannot find the sk */
	if (!emcom_xengine_mpflow_ai_rehash_sk(sk))
		return;

	rcu_read_lock();
	dev = dev_get_by_name_rcu(sock_net(sk), ifname);
	if (!dev || (emcom_xengine_mpflow_getinetaddr(dev) == false)) {
		rcu_read_unlock();
		EMCOM_LOGD("device not ready uid: %u, sk: %pK, dev: %pK, name: %s",
				uid, sk, dev, (dev == NULL ? "null" : dev->name));
		return;
	}
	rcu_read_unlock();

	sk->sk_bound_dev_if = dev->ifindex;
	if (sk->sk_protocol == IPPROTO_UDP) {
		sk_dst_reset(sk);
		inet = inet_sk(sk);
		if (inet->inet_saddr)
			inet->inet_saddr = 0;
	}

	spin_lock_bh(&g_mpflow_ai_lock);
	if (bind_device == EMCOM_MPFLOW_BIND_WIFI) {
		g_mpflow_ai_uids[index].wifi_devif = dev->ifindex;
	} else if (bind_device == EMCOM_MPFLOW_BIND_LTE) {
		g_mpflow_ai_uids[index].lte_devif = dev->ifindex;
	}
	spin_unlock_bh(&g_mpflow_ai_lock);
	EMCOM_LOGI("bind success uid: %u, ifname: %s, ifindex: %d",
			uid, ifname, sk->sk_bound_dev_if);

	usin = (struct sockaddr_in *)uaddr;
	if (usin->sin_family == AF_INET) {
		EMCOM_LOGI("[MPFlow_KERNEL] Bind Completed. SrcIP: *.*, SrcPort: *, DstIP:"IPV4_FMT", DstPort: %u ",
			IPV4_INFO(usin->sin_addr.s_addr), dport);
	} else if (usin->sin_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)uaddr;
		EMCOM_LOGI("[MPFlow_KERNEL] Bind Completed. SrcIP: *.*, SrcPort: *, DstIP:"IPV6_FMT", DstPort: %u ",
			IPV6_INFO(usin6->sin6_addr), dport);
	}
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("xengine module driver");

