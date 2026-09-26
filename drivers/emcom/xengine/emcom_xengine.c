/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2021. All rights reserved.
 * Description: utils of xengine module
 * Author: zhongwenguo zhongwenguo@huawei.com
 * Create: 2017-04-20
 */
#include "emcom/emcom_xengine.h"
#include <asm/uaccess.h>
#include <linux/fdtable.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/version.h>
#include <net/inet_hashtables.h>
#include <net/inet6_hashtables.h>
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include "emcom/network_evaluation.h"

#ifdef CONFIG_HUAWEI_OPMP
#include <huawei_platform/emcom/opmp_heartbeat.h>
#endif
#ifdef CONFIG_MPTCP
#include <net/mptcp.h>
#endif
#include <emcom/emcom_mpdns.h>

#ifdef CONFIG_HUAWEI_BASTET
#include <huawei_platform/net/bastet/bastet_utils.h>
#endif
#include "securec.h"

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

#ifdef CONFIG_HUAWEI_BASTET_COMM
extern int bastet_comm_key_ps_info_write(uint32_t state);
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

struct emcom_xengine_network_info g_network_infos[EMCOM_XENGINE_NET_MAX_NUM];
static spinlock_t g_network_infos_cache_lock;

void emcom_xengine_mpip_init(void);
void emcom_xengine_ccalg_init(void);
bool emcom_xengine_is_private_addr(__be32 addr);
void emcom_xengine_supply_mpdns_cache(const struct sk_buff *skb, const char* host, int ip_type);
void emcom_xengine_update_mpdns_cache(const char *data, uint16_t len);
void emcom_xengine_update_network_info(const char *data, uint16_t len);
void emcom_xengine_set_mpdns_config(const char *data, uint16_t len);

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
static void emcom_xengine_udptimer_handler(unsigned long pac)
#else
static void emcom_xengine_udptimer_handler(struct timer_list *t)
#endif
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
			emcom_logd("emcom_xengine_udptimer_handler send skb");
		}
	}

	skb = skb_peek(&g_udp_skb_list);
	if (!skb)
		goto timer_off;
	cb = emcom_xengine_netem_skb_cb(skb);
	now = jiffies;
	/* if remaining time is little than 1 jiffie, send out */
	while (cb->time_to_send <= (now + MIN_JIFFIE)) {
		emcom_logd("emcom_xengine_udptimer_handler send another skb");
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
	emcom_logd("emcom_xengine_udptimer_handler modify timer hz %d", jiffie_n);
	mod_timer(&g_udp_skb_timer, jiffies + jiffie_n);
	g_emcom_udptimer_on = true;
	return;

timer_off:
	g_emcom_udptimer_on = false;
}

void emcom_xengine_init(void)
{
	uint8_t index;
	spin_lock_init(&g_network_infos_cache_lock);

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
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	init_timer(&g_udp_skb_timer);
	g_udp_skb_timer.function = emcom_xengine_udptimer_handler;
#else
	timer_setup(&g_udp_skb_timer, emcom_xengine_udptimer_handler, 0);
#endif
	mutex_init(&g_mpip_mutex);
	emcom_xengine_mpip_init();
	emcom_xengine_mpflow_init();
	emcom_xengine_ccalg_init();
	g_fastsyn_uid = UID_INVALID_APP;
	(void)memset_s(g_network_infos, sizeof(g_network_infos), 0, sizeof(g_network_infos));
	emcom_mpdns_init();
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
		emcom_logd("emcom_xengine_hook_ul_stub param invalid");
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
		emcom_loge("emcom_xengine_clear memset failed");
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
	emcom_xengine_set_speedctrl(g_speedctrl_info, UID_INVALID_APP, 0);
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
				old_index = index;
			}
		}
	}

	/* remove the too old acc uid */
	if (old_age > EMCOM_UID_ACC_AGE_MAX) {
		emcom_logd("Emcom_Xengine_StartAccUid: uid:%d added too long, remove it",
				   g_current_uids[old_index].uid);
		g_current_uids[old_index].age = 0;
		g_current_uids[old_index].uid  = UID_INVALID_APP;
	}

	emcom_logd("Emcom_Xengine_StartAccUid: idle_index=%u,old_index=%u,old_age=%u",
			   idle_index, old_index, old_age);

	/* if has already added, return */
	if (found)
		return index;

	/* if it is new uid, and has idle position , add it */
	if (idle_index < EMCOM_MAX_ACC_APP) {
		emcom_logd("Emcom_Xengine_StartAccUid: uid:%u added", uid);
		return idle_index;
	}

	/* if it is new uid, and acc list if full , replace the oldest */
	if (old_index < EMCOM_MAX_ACC_APP) {
		emcom_logd("Emcom_Xengine_StartAccUid: uid:%u replace the oldest uid:%d",
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
		emcom_loge("Emcom_Xengine_StartAccUid:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		emcom_logi("Emcom_Xengine_StartAccUid: len:%u is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)pdata;

	/* check uid */
	if (invalid_uid(uid))
		return -EINVAL;

	emcom_logd("Emcom_Xengine_StartAccUid: uid:%u ready to added", uid);

	index = emcom_xengine_found_avaiable_accindex(uid);
	/* if it is new uid, and has idle position , add it */
	if (index < EMCOM_MAX_ACC_APP) {
		emcom_logd("Emcom_Xengine_StartAccUid: uid:%u added", uid);
		g_current_uids[index].age = 0;
		g_current_uids[index].uid = uid;
		return 0;
	}

	emcom_loge("StartAccUid: not available index:%u, uid:%u", index, uid);
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
		emcom_loge("Emcom_Xengine_StopAccUid:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		emcom_logi("Emcom_Xengine_StopAccUid: len: %u is illegal", len);
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
			emcom_logd("Emcom_Xengine_StopAccUid:lUid:%u", uid);
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
		emcom_loge("Emcom_Xengine_SetSpeedCtrlInfo:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(struct emcom_xengine_speed_ctrl_data)) {
		emcom_logi("Emcom_Xengine_SetSpeedCtrlInfo: len:%u is illegal", len);
		return -EINVAL;
	}

	pspeedctrl_info = (struct emcom_xengine_speed_ctrl_data *)data;
	uid = pspeedctrl_info->uid;
	size = pspeedctrl_info->size;

	/* if uid and size is zero, clear the speed control info */
	if (!uid && !size) {
		emcom_logd("Emcom_Xengine_SetSpeedCtrlInfo: clear speed ctrl state");
		emcom_xengine_set_speedctrl(g_speedctrl_info, UID_INVALID_APP, 0);
		return 0;
	}

	/* check uid */
	if (invalid_uid(uid)) {
		emcom_logi("Emcom_Xengine_SetSpeedCtrlInfo: uid:%u is illegal", uid);
		return -EINVAL;
	}

	/* check size */
	if (invalid_speedctrl_size(size)) {
		emcom_logi("Emcom_Xengine_SetSpeedCtrlInfo: size:%u is illegal", size);
		return -EINVAL;
	}

	emcom_logd("Emcom_Xengine_SetSpeedCtrlInfo: uid:%u size:%u", uid, size);
	emcom_xengine_set_speedctrl(g_speedctrl_info, uid, size);
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
		emcom_logd("emcom_xengine_speedctrl_winsize param invalid\n");
		return;
	}

	if (pstsize == NULL) {
		emcom_logd(" emcom_xengine_speedctrl_winsize window size invalid\n");
		return;
	}

	emcom_xengine_get_speedctrl_uid(g_speedctrl_info, uid);
	if (invalid_uid(uid))
		return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(pstsock).val;
#else
	sock_uid = sock_i_uid(pstsock);
#endif
	if (invalid_uid(sock_uid))
		return;

	emcom_xengine_get_speedctrl_info(g_speedctrl_info, uid, size);
	/* check uid */
	if (sock_uid == uid)
		return;

	if (size)
		*pstsize = (g_speedctrl_info.size < *pstsize) ? g_speedctrl_info.size : *pstsize;
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
	emcom_logd("The Mpip list will be update to empty");

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
		emcom_loge("The length of received MPIP APP uid list is error");
		return -EINVAL;
	}
	mutex_lock(&g_mpip_mutex);
	for (index = 0; index < length; index++) {
		g_mpip_uids[index].uid = *(uid_t *)temp;
		g_mpip_uids[index].type = *(uint32_t *)(temp + sizeof(uid_t));
		emcom_logd("The Mpip config [%u] is: lUid %u and type %d", index,
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
	emcom_logd("The Mpip list will be update to empty");

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
		emcom_loge("MPIP interface name or length %u is error", len);
		return -EINVAL;
	}
	mutex_lock(&g_mpip_mutex);
	err = memcpy_s(g_ifacename, sizeof(char) * IFNAMSIZ, data, len);
	if (err != EOK)
		emcom_loge("emcom_xengine_start_mpip memcpy failed");
	g_mpip_start = true;
	mutex_unlock(&g_mpip_mutex);
	emcom_logd("Mpip is :%d to start", g_mpip_start);
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
	emcom_logd("MPIP function is :%d, ready to stop", g_mpip_start);

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
		emcom_loge(" param invalid");
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
			emcom_loge(" get dev fail or dev is not up");
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
		emcom_loge("Emcom_Send_KeyPsInfo:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len < sizeof(uint32_t)) {
		emcom_loge("Emcom_Send_KeyPsInfo: len: %u is illegal", len);
		return -EINVAL;
	}

	state = *(uint32_t *)data;

	if (!emcom_is_modem_support()) {
		emcom_logi("Emcom_Send_KeyPsInfo: modem not support");
		return -EINVAL;
	}

#ifdef CONFIG_HUAWEI_BASTET_COMM
bastet_comm_key_ps_info_write(state);
#endif
	return 0;
}

/*
 * judge current network is wifi
 */
static bool emcom_xengine_iswlan(const struct sk_buff *skb)
{
	const char *delim = EMCOM_WLAN0_IFNAME;
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
		emcom_loge("Emcom_Xengine_UdpEnqueue skb null");
		return;
	}

	if (g_udp_skb_list.qlen >= EMCOM_MAX_UDP_SKB) {
		emcom_loge("Emcom_Xengine_UdpEnqueue max skb");
		return;
	}

	sk = skb_to_full_sk(skb);
	if (unlikely(!sk)) {
		emcom_loge("Emcom_Xengine_UdpEnqueue sk null");
		return;
	}

	if (unlikely(!sk->sk_socket)) {
		emcom_loge("Emcom_Xengine_UdpEnqueue sk_socket null");
		return;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 10)
	sock_uid = sock_i_uid(sk).val;
#else
	sock_uid = sock_i_uid(sk);
#endif
	if (sock_uid == g_udp_retran_uid) {
		if (!emcom_xengine_iswlan(skb)) {
			emcom_logd("Emcom_Xengine_UdpEnqueue not wlan");
			emcom_xengine_udpretran_clear();
			return;
		}

		if (sk->sk_socket->type == SOCK_DGRAM) {
			struct sk_buff *skb2 = skb_copy(skb, GFP_ATOMIC);
			if (unlikely(!skb2)) {
				emcom_loge("Emcom_Xengine_UdpEnqueue skb2 null");
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
		emcom_loge("Emcom_Xengine_StartUdpReTran:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		emcom_logi("Emcom_Xengine_StartUdpReTran: len: %u is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)data;
	/* check uid */
	if (invalid_uid(uid)) {
		emcom_loge("Emcom_Xengine_StartUdpReTran: uid is invalid %u", uid);
		return -EINVAL;
	}
	emcom_logi("Emcom_Xengine_StartUdpReTran: uid: %u ", uid);
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
		emcom_logd(" Emcom_Xengine_FastSyn param invalid");
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
		emcom_loge(" emcom_xengine_start_fastsyn:data is null");
		return -EINVAL;
	}

	/* check len is invalid */
	if (len != sizeof(uid_t)) {
		emcom_logi(" emcom_xengine_start_fastsyn: len: %u is illegal", len);
		return -EINVAL;
	}

	uid = *(uid_t *)data;
	/* check uid */
	if (invalid_uid(uid)) {
		emcom_loge(" emcom_xengine_start_fastsyn: uid is invalid %u", uid);
		return -EINVAL;
	}
	emcom_logi(" emcom_xengine_start_fastsyn: uid: %u ", uid);
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
		emcom_loge("CCAlg interface name or length %u is error", len);
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
			emcom_logd("alg: %u is activating again, uid: %u", alg, uid);
		} else {
			/* already activated, but change another algorithm */
			emcom_logd("CCAlg function is ready to change alg, uid: %u, alg from %u to %u",
					   uid, g_ccalg_uids[index].alg, alg);
			g_ccalg_uids[index].alg = alg;
			g_ccalg_uids[index].has_log = false;
		}
	} else {
		/* a new app to activate */
		int8_t index_new = emcom_xengine_find_ccalg(UID_INVALID_APP);
		if (index_new != INDEX_INVALID) {
			g_ccalg_uids[index_new].uid = uid;
			g_ccalg_uids[index_new].alg = alg;
			g_ccalg_uids[index_new].has_log = false;
			g_ccalg_uid_cnt++;
			emcom_logd("CCAlg function is ready to start, uid: %u, alg: %u", uid, alg);
		} else if (g_ccalg_uid_cnt >= EMCOM_MAX_CCALG_APP) {
			emcom_loge("CCAlg has already activated %d apps, cannot activate more apps", g_ccalg_uid_cnt);
			return;
		} else {
			emcom_loge("not supposed to happend: CCAlg has already activated %d apps", g_ccalg_uid_cnt);
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
		emcom_loge("CCAlg interface name or length %u is error", len);
		return;
	}

	uid = *((uid_t *)data);

	index = emcom_xengine_find_ccalg(uid);
	if (index != INDEX_INVALID) {
		emcom_logd("CCAlg function is ready to stop, uid: %u, alg: %u", uid, g_ccalg_uids[index].alg);
		g_ccalg_uids[index].uid = UID_INVALID_APP;
		g_ccalg_uids[index].alg = EMCOM_XENGINE_CONG_ALG_INVALID;
		g_ccalg_uids[index].has_log = false;
		g_ccalg_uid_cnt--;
	} else {
		emcom_loge("CCAlg function is not activated yet, cannot be deactivated, uid: %u", uid);
		return;
	}

	if (g_ccalg_uid_cnt <= 0) {
		emcom_logd(" no ccalg is activated now: CCAlg function is ready to stop, cnt: %u", g_ccalg_uid_cnt);
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
					emcom_logd("app: %d change default congcontrol alg to :%s", sock_i_uid(sk).val, app_alg);
					g_ccalg_uids[index].has_log = true;
				}
				return;
			}
		}
	}

	if (g_ccalg_uids[index].has_log == false) {
		emcom_loge("Emcom_Xengine_change_default_ca failed to find algorithm %s", EMCOM_CONGESTION_CONTROL_ALG_BBR);
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

bool emcom_xengine_transfer_sk_to_addr(struct sock *sk, struct sockaddr *addr)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)addr;
	if (sk->sk_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;
		usin->sin_family = AF_INET6;
		usin6->sin6_addr = sk->sk_v6_daddr;
		return true;
	} else if (sk->sk_family == AF_INET) {
		usin->sin_family = AF_INET;
		usin->sin_addr.s_addr = sk->sk_daddr;
		return true;
	}

	return false;
}

bool emcom_xengine_is_v6_sock(struct sock *sk)
{
	if (sk->sk_family == AF_INET6 &&
		!(ipv6_addr_type(&sk->sk_v6_daddr) & IPV6_ADDR_MAPPED))
		return true;
	return false;
}

bool emcom_xengine_is_private_addr(__be32 addr)
{
	return (ipv4_is_linklocal_169(addr) ||
		ipv4_is_private_10(addr) ||
		ipv4_is_private_172(addr) ||
		ipv4_is_private_192(addr));
}

bool emcom_xengine_is_private_v4_addr(const struct in_addr *v4_addr)
{
	__be32 addr = v4_addr->s_addr;
	return emcom_xengine_is_private_addr(addr);
}

bool emcom_xengine_is_private_v6_addr(const struct in6_addr *v6_addr)
{
	const unsigned int clat_ipv4_index = 3;
	int addr_type = ipv6_addr_type(v6_addr);
	if (addr_type & IPV6_ADDR_MAPPED) {
		__be32 s_addr = v6_addr->s6_addr32[clat_ipv4_index];
		return emcom_xengine_is_private_addr(s_addr);
	}
	return false;
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

#undef EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX

#ifdef CONFIG_MPTCP
void emcom_xengine_mptcp_socket_closed(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_SOCKET_CLOSED, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_socket_closed); /*lint !e580*/

void emcom_xengine_mptcp_socket_switch(const  void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_SOCKET_SWITCH, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_socket_switch); /*lint !e580*/

void emcom_xengine_mptcp_proxy_fallback(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_PROXY_FALLBACK, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_proxy_fallback); /*lint !e580*/

void emcom_xengine_mptcp_fallback(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPTCP_FALLBACK, data, len);
}
EXPORT_SYMBOL(emcom_xengine_mptcp_fallback); /*lint !e580*/
#endif

/*
 * message proc , process every message for xengine module
 */
void emcom_xengine_evt_proc(int32_t event, const uint8_t *data, uint16_t len)
{
	switch (event) {
	case NETLINK_EMCOM_DK_START_ACC:
		emcom_logd("emcom netlink receive acc start");
		emcom_xengine_start_acc_uid(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_ACC:
		emcom_logd("emcom netlink receive acc stop");
		emcom_xengine_stop_acc_uid(data, len);
		break;
	case NETLINK_EMCOM_DK_CLEAR:
		emcom_logd("emcom netlink receive clear info");
		emcom_xengine_clear();
		break;
	case NETLINK_EMCOM_DK_RRC_KEEP:
		emcom_logd("emcom netlink receive rrc keep");
		emcom_xengine_rrckeep();
		break;
	case NETLINK_EMCOM_DK_KEY_PSINFO:
		emcom_logd("emcom netlink receive psinfo");
		emcom_send_keypsinfo(data, len);
		break;
	case NETLINK_EMCOM_DK_SPEED_CTRL:
		emcom_logd("emcom netlink receive speed control uid");
		emcom_xengine_set_speedctrl_info(data, len);
		break;
	case NETLINK_EMCOM_DK_START_UDP_RETRAN:
		emcom_logd("emcom netlink receive wifi udp start");
		emcom_xengine_start_udpretran(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_UDP_RETRAN:
		emcom_logd("emcom netlink receive wifi udp stop");
		emcom_xengine_stop_udpretran(data, len);
		break;
	case NETLINK_EMCOM_DK_CONFIG_MPIP:
		emcom_logd("emcom netlink receive btm config start");
		emcom_xengine_config_mpip(data, len);
		break;
	case NETLINK_EMCOM_DK_CLEAR_MPIP:
		emcom_logd("emcom netlink receive clear mpip config");
		emcom_xengine_clear_mpip_config(data, len);
		break;
	case NETLINK_EMCOM_DK_START_MPIP:
		emcom_logd("emcom netlink receive btm start");
		emcom_xengine_start_mpip(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_MPIP:
		emcom_logd("emcom netlink receive btm stop");
		emcom_xengine_stop_mpip(data, len);
		break;
	case NETLINK_EMCOM_DK_START_FAST_SYN:
		emcom_logd("emcom netlink receive fast syn start");
		emcom_xengine_start_fastsyn(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_FAST_SYN:
		emcom_logd("emcom netlink receive fast syn stop");
		emcom_xengine_stop_fastsyn(data, len);
		break;
#ifdef CONFIG_HUAWEI_OPMP
	case NETLINK_EMCOM_DK_OPMP_INIT_HEARTBEAT:
		emcom_logd("emcom netlink received opmp init heartbeat");
		opmp_event_process(event, data, len);
		break;
#endif
	case NETLINK_EMCOM_DK_ACTIVE_CCALG:
		emcom_logd(" emcom netlink active congestion control algorithm");
		emcom_xengine_active_ccalg(data, len);
		break;
	case NETLINK_EMCOM_DK_DEACTIVE_CCALG:
		emcom_logd(" emcom netlink deactive congestion control algorithm");
		emcom_xengine_deactive_ccalg(data, len);
		break;
	case NETLINK_EMCOM_DK_START_MPFLOW:
		emcom_logd(" emcom netlink start mpflow control algorithm");
		emcom_xengine_mpflow_start(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_MPFLOW:
		emcom_logd(" emcom netlink stop mpflow control algorithm");
		emcom_xengine_mpflow_stop(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_RST_LOC_INTF:
		emcom_logd(" emcom netlink mod local interface");
		emcom_xengine_mpflow_ai_reset_loc_intf(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_BIND_IP_POLICY:
		emcom_logd(" emcom netlink mpflow ip policy config");
		emcom_xengine_mpflow_ai_ip_config(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_INIT_BIND_CONFIG:
		emcom_logd(" emcom netlink mpflow init bind config");
		emcom_xengine_mpflow_ai_init_bind_config(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_BIND_PORT_POLICY:
		emcom_logd(" emcom netlink mpflow port policy config");
		emcom_xengine_mpflow_ai_iface_cfg(data, len);
		break;
	case NETLINK_EMCOM_DK_STOP_MPFLOW_V2:
		emcom_logd(" emcom netlink stop mpflow control algorithm v2");
		emcom_xengine_mpflow_ai_stop(data, len);
		break;
	case NETLINK_EMCOM_DK_NOTIFY_NETWORK_INFO:
		emcom_logd("emcom netlink receive network info");
		emcom_xengine_update_network_info(data, len);
		break;
	case NETLINK_EMCOM_DK_CONFIG_MPDNS:
		emcom_logd("emcom netlink receive mpdns config");
		emcom_xengine_set_mpdns_config(data, len);
		break;
	case NETLINK_EMCOM_DK_NOTIFY_MPDNS_RESULT:
		emcom_xengine_update_mpdns_cache(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_RST_ALL_FLOW:
		emcom_logd(" emcom netlink transfer mpflow to one");
		emcom_xengine_mpflow_ai_close_all_flow(data, len);
		break;
	case NETLINK_EMCOM_DK_MPF_CHANGE_BURST_RATIO:
		emcom_logd(" emcom netlink change burst ratio");
		emcom_xengine_mpflow_ai_change_burst_ratio(data, len);
		break;
	default:
		emcom_logi("emcom Xengine unsupport packet, the type is %d.\n", event);
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
	emcom_logd("hicom set proxy uid, uid: %u", sk->sk_uid.val);
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

	emcom_logd(" hicom set proxy flag, uid: %u, flag: %x", sk->sk_uid.val, \
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
				err = strncpy_s(node->iface_name, IFNAMSIZ, cfg->iface_name, IFNAMSIZ - 1);
				if (err != EOK)
					emcom_loge("strncpy_s failed");
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

	/* File object ref couldn't be taken.
	 * dup2() atomicity guarantee is the reason
	 * we loop to catch the new file (or NULL pointer)
	 */
	while ((file = fcheck_files(files, fd)) != NULL) {
		if (file->f_mode & mask) {
			file = NULL;
			break;
		} else if (get_file_rcu(file)) {
			break;
		}
	}
	rcu_read_unlock();
	return file;
}

static int emcom_xengine_bind2device_fd(const struct emcom_xengine_bind2device_node *cfg)
{
	struct file *file = NULL;
	struct socket *sock = NULL;
	struct sock *sk = NULL;
	pid_t pid = cfg->pid;
	int fd = cfg->fd;
	struct net_device *dev = NULL;
	int err;

	emcom_logd("uid %u pid_fd:%d_%d\n", cfg->uid, pid, fd);
	file = emcom_xengine_fget_by_pid(pid, fd, 0);
	if (!file) {
		emcom_logd("get file fail by pid_fd:%d_%d\n", pid, fd);
		return -EBADF;
	}

	sock = sock_from_file(file, &err);
	if (!sock) {
		emcom_logd("get socket fail by pid_fd:%d_%d\n", pid, fd);
		fput_by_pid(pid, file);
		return -EBADF;
	}

	sk = sock->sk;
	if (!sk || sk->sk_uid.val != cfg->uid) {
		emcom_logd("get sk fail by pid_fd:%d_%d\n", pid, fd);
		fput_by_pid(pid, file);
		return -EBADF;
	}

	sock_hold(sk);
	rcu_read_lock();
	local_bh_disable();
	bh_lock_sock(sk);

	dev = dev_get_by_name_rcu(sock_net(sk), cfg->iface_name);
	if (dev) {
		unsigned int flags = dev_get_flags(dev);
		if (flags & IFF_RUNNING) {
			sk->sk_bound_dev_if = dev->ifindex;
			sk_dst_reset(sk);
		} else  {
			emcom_logd("iface %s is not on\n", cfg->iface_name);
		}
	} else {
		emcom_logd("iface %s is not exist\n", cfg->iface_name);
	}
	bh_unlock_sock(sk);
	local_bh_enable();
	rcu_read_unlock();
	sock_put(sk);
	fput_by_pid(pid, file);
	return 0;
}

int emcom_xengine_setbind2device(const struct sock *sk, const char __user *optval, int optlen)
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

	if (node->iface_name[0] == '\0' || strnlen(node->iface_name, IFNAMSIZ) >= IFNAMSIZ) {
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
		emcom_logd("uid: %u pid:%d fd:%d iface:%s", node->uid,
			node->pid, node->fd, node->iface_name);

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
			dev = dev_get_by_name_rcu(sock_net(sk), node->iface_name);
			if (dev) {
				unsigned int flags = dev_get_flags(dev);
				if (flags & IFF_RUNNING) {
					sk->sk_bound_dev_if = dev->ifindex;
					sk_dst_reset(sk);
				} else {
					emcom_logd("iface %s is not on\n", node->iface_name);
				}
			} else {
				emcom_logd("iface %s is not exist\n", node->iface_name);
			}
			break;
		}
	}
	rcu_read_unlock();
}

void emcom_xengine_notify_sock_error(struct sock *sk)
{
	if (sk->hicom_flag == HICOM_SOCK_FLAG_FINTORST) {
		emcom_logd(" hicom change fin to rst, uid: %u, flag: %x", sk->sk_uid.val, sk->hicom_flag);
		sk->sk_err = ECONNRESET;
		sk->sk_error_report(sk);
	}

	return;
}

void emcom_xengine_process_http_req(const struct sk_buff *skb, int ip_type)
{
	uid_t uid;
	unsigned int payloadlen;
	unsigned char *payload = NULL;
	char host[EMCOM_MAX_HOST_LEN] = {0};
	struct sock *sk = skb->sk;
	uint16_t port = ntohs(tcp_hdr(skb)->dest); /*lint !e666*/
	/* http port must be 80 */
	if (port != EMCOM_MPFLOW_FI_PORT_80)
		return;

	payloadlen = skb->len - skb_transport_offset(skb) - tcp_hdrlen(skb);
	if (payloadlen < EMCOM_MIN_HTTP_LEN)
		return;
	payload = skb_transport_header(skb) + tcp_hdrlen(skb);
	if (!emcom_parse_httpget_host(payload, host, EMCOM_MAX_HOST_LEN) || !emcom_is_alp_host(host))
		return;

	uid = sock_i_uid(sk).val;
	if (!emcom_mpdns_is_allowed(uid, host))
		return;

	emcom_xengine_supply_mpdns_cache(skb, host, ip_type);
}

void emcom_xengine_supply_mpdns_cache(const struct sk_buff *skb, const char* host, int ip_type)
{
	struct sock *sk = skb->sk;
	uid_t uid = sock_i_uid(sk).val;
	int init_net_type = emcom_xengine_get_net_type(sk->sk_mark & 0x00FF);
	int bound_net_type = emcom_xengine_get_sock_bound_net_type(sk);
	bool dst_reliable = (bound_net_type == EMCOM_XENGINE_NET_INVALID ||
		bound_net_type == init_net_type);
	bool act_net_type = (bound_net_type == EMCOM_XENGINE_NET_INVALID) ?
		init_net_type : bound_net_type;
	bool need_add_proc = false;

	if (ip_type == EMCOM_XENGINE_IP_TYPE_V4) {  // no use sk->sk_family
		uint32_t ndest = ip_hdr(skb)->daddr;
		struct in_addr v4_addr = {.s_addr = ndest};
		struct in_addr *act_addr = NULL;
		if (emcom_xengine_is_private_v4_addr(&v4_addr))
			dst_reliable = false;
		act_addr = dst_reliable ? &v4_addr : NULL;
		need_add_proc = emcom_mpdns_supply_v4_cache(host, act_addr, act_net_type);
	} else if (ip_type == EMCOM_XENGINE_IP_TYPE_V6) {
		struct in6_addr *v6_addr = &(ipv6_hdr(skb)->daddr);
		struct in6_addr *act_addr = NULL;
		if (emcom_xengine_is_private_v6_addr(v6_addr))
			dst_reliable = false;
		act_addr = dst_reliable ? v6_addr : NULL;
		need_add_proc = emcom_mpdns_supply_v6_cache(host, act_addr, act_net_type);
	} else {
		// do nothing
	}
	if (need_add_proc)
		emcom_mpdns_host_send(uid, act_net_type, ip_type, host);
}

void emcom_xengine_update_network_info(const char *data, uint16_t len)
{
	struct emcom_xengine_network_info *net_info = NULL;
	errno_t err;

	if (data == NULL || (len != sizeof(struct emcom_xengine_network_info))) {
		emcom_loge("data or length %u is invalid", len);
		return;
	}

	net_info = (struct emcom_xengine_network_info *)data;
	emcom_logi("net type:%d, net id:%d, validated:%d, default:%d, iface: %s", net_info->net_type, net_info->net_id,
		net_info->is_validated, net_info->is_default, net_info->iface_name);
	if (!emcom_xengine_is_net_type_valid(net_info->net_type))
		return;

	if (g_network_infos[net_info->net_type].is_validated != net_info->is_validated &&
		net_info->is_validated != EMCOM_TRUE_VALUE)
		emcom_mpdns_flush_cache(net_info->net_type);  // flush cache for unavailable net
	spin_lock_bh(&g_network_infos_cache_lock);
	err = memcpy_s(&(g_network_infos[net_info->net_type]), sizeof(struct emcom_xengine_network_info), net_info, len);
	spin_unlock_bh(&g_network_infos_cache_lock);
	if (err != EOK)
		emcom_loge("copy network info fail");
}

void emcom_xengine_set_mpdns_config(const char *data, uint16_t len)
{
	struct emcom_mpdns_config *mpdns_config = NULL;

	if (data == NULL || (len != sizeof(struct emcom_mpdns_config))) {
		emcom_loge("data or length %u is invalid", len);
		return;
	}

	mpdns_config = (struct emcom_mpdns_config *)data;
	emcom_logd("uid:%d, act:%d", mpdns_config->uid, mpdns_config->act);
	emcom_mpdns_set_app_config(mpdns_config);
}

void emcom_xengine_update_mpdns_cache(const char *data, uint16_t len)
{
	struct emcom_mpdns_result *mpdns_result = NULL;

	if (data == NULL || (len != sizeof(struct emcom_mpdns_result))) {
		emcom_loge("data or length %u is invalid", len);
		return;
	}

	mpdns_result = (struct emcom_mpdns_result *)data;
	emcom_mpdns_update_cache(mpdns_result);
}

int emcom_xengine_get_net_type(int net_id)
{
	int net_type;
	if (net_id <= EMCOM_XENGINE_DEFAULT_NET_ID) {
		spin_lock_bh(&g_network_infos_cache_lock);
		for (net_type = EMCOM_XENGINE_NET_WLAN0; net_type < EMCOM_XENGINE_NET_MAX_NUM; net_type++) {
			if (g_network_infos[net_type].is_default) {
				spin_unlock_bh(&g_network_infos_cache_lock);
				return net_type;
			}
		}
		spin_unlock_bh(&g_network_infos_cache_lock);
		return EMCOM_XENGINE_NET_INVALID;
	}
	spin_lock_bh(&g_network_infos_cache_lock);
	for (net_type = EMCOM_XENGINE_NET_WLAN0; net_type < EMCOM_XENGINE_NET_MAX_NUM; net_type++) {
		if (g_network_infos[net_type].net_id == net_id) {
			spin_unlock_bh(&g_network_infos_cache_lock);
			return net_type;
		}
	}
	spin_unlock_bh(&g_network_infos_cache_lock);
	return EMCOM_XENGINE_NET_INVALID;
}

int emcom_xengine_get_net_type_by_name(const char *iface_name)
{
	int net_type;
	if (iface_name == NULL)
		return EMCOM_XENGINE_NET_INVALID;
	for (net_type = EMCOM_XENGINE_NET_WLAN0; net_type < EMCOM_XENGINE_NET_MAX_NUM; net_type++) {
		char *cur_iface_name = emcom_xengine_get_network_iface_name(net_type);
		if (strcmp(cur_iface_name, iface_name) == 0)
			return net_type;
	}
	return EMCOM_XENGINE_NET_INVALID;
}

int emcom_xengine_get_sock_bound_net_type(const struct sock *sk)
{
	char ifname[IFNAMSIZ] = {0};
	int bound_net_type = EMCOM_XENGINE_NET_INVALID;

	if (sk->sk_bound_dev_if > 0 &&
		netdev_get_name(sock_net(sk), ifname, sk->sk_bound_dev_if) == 0)
		bound_net_type = emcom_xengine_get_net_type_by_name(ifname);

	return bound_net_type;
}

char *emcom_xengine_get_network_iface_name(int net_type)
{
	unsigned int iface_len;
	if (net_type < 0 || net_type >= EMCOM_XENGINE_NET_MAX_NUM) {
		emcom_loge("net_type is invalid: %d", net_type);
		return INVALID_IFACE_NAME;
	}
	spin_lock_bh(&g_network_infos_cache_lock);
	iface_len = strlen(g_network_infos[net_type].iface_name);
	if (iface_len == 0 || iface_len >= IFNAMSIZ) {
		spin_unlock_bh(&g_network_infos_cache_lock);
		return INVALID_IFACE_NAME;
	}
	spin_unlock_bh(&g_network_infos_cache_lock);
	return g_network_infos[net_type].iface_name;
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("xengine module driver");

