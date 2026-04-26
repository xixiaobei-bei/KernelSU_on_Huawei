/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: mpflow module implemention
 * Author: xialiang xialiang4@huawei.com
 * Create: 2020-06-08
 */
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
#ifdef CONFIG_MPTCP
#include <net/mptcp.h>
#endif
#ifdef CONFIG_HUAWEI_BASTET
#include "huawei_platform/net/bastet/bastet_utils.h"
#endif
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include "emcom/emcom_xengine.h"
#include "emcom/network_evaluation.h"
#ifdef CONFIG_HUAWEI_OPMP
#include "emcom/opmp_heartbeat.h"
#endif
#include "emcom/emcom_mpdns.h"
#include "securec.h"

#undef HWLOG_TAG
#define HWLOG_TAG emcom_mpflow
HWLOG_REGIST();

#define EMCOM_GOOD_RECV_RATE_THR_BYTE_PER_SEC 400000
#define EMCOM_GOOD_RTT_THR_MS 120
#define SKARR_SZ 200

spinlock_t g_mpflow_ai_lock;
struct emcom_xengine_mpflow_ai_info g_mpflow_ai_uids[EMCOM_MPFLOW_AI_MAX_APP];
struct emcom_xengine_mpflow_stat g_mpflow_ai_list[EMCOM_MPFLOW_AI_MAX_LIST_NUM];

static bool emcom_xengine_mpflow_ai_is_excluded_port(const struct emcom_xengine_mpflow_ai_info *app,
	uint16_t dport, uint8_t proto);
static bool emcom_xengine_mpflow_ai_get_port(struct sockaddr *addr, uint16_t *port);

static inline bool invalid_uid(uid_t uid)
{
	/* if uid less than 10000, it is not an Android apk */
	return (uid < UID_APP);
}

static bool emcom_xengine_mpflow_ai_uid_empty(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_AI_MAX_APP; index++) {
		if (g_mpflow_ai_uids[index].uid != UID_INVALID_APP)
			return false;
	}

	return true;
}

static struct emcom_xengine_mpflow_ip_bind_policy *
emcom_xengine_mpflow_ai_hash_get_policy(__be32 *addr, uint8_t addr_len, const struct hlist_head *hashtable)
{
	struct emcom_xengine_mpflow_ip_bind_policy *policy = NULL;
	struct emcom_xengine_mpflow_ip_bind_policy *result = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash;

	if (addr_len > EMCOM_MPFLOW_AI_CLAT_IPV6)
		return result;

	hash = emcom_xengine_mpflow_ip_hash(*(addr + addr_len - 1));

	hlist_for_each_entry_safe(policy, tmp, &hashtable[hash], node) {
		if (memcmp((const void *)(policy->addr), (const void *)addr, addr_len * sizeof(__be32)) == 0) {
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
		emcom_loge("sa_family error, sa_family: %hu", addr->sa_family);
		return false;
	}
}

static bool emcom_xengine_mpflow_ai_get_port(struct sockaddr *addr, uint16_t *port)
{
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *usin = (struct sockaddr_in *)addr;
		*port = ntohs(usin->sin_port);
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;
		*port = ntohs(usin6->sin6_port);
		return true;
	}
#endif
	else {
		emcom_loge("sa_family error, sa_family: %hu", addr->sa_family);
		return false;
	}
}

static struct emcom_xengine_mpflow_ip_bind_policy *
emcom_xengine_mpflow_ai_hash(const __be32 *addr, uint8_t addr_len, struct hlist_head *hashtable)
{
	struct emcom_xengine_mpflow_ip_bind_policy *policy = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t i, hash;

	if (addr_len > EMCOM_MPFLOW_AI_CLAT_IPV6)
		return NULL;

	hash = emcom_xengine_mpflow_ip_hash(*(addr + addr_len - 1));

	hlist_for_each_entry_safe(policy, tmp, &hashtable[hash], node) {
		if (memcmp((const void *)(policy->addr), (const void *)addr, addr_len * sizeof(__be32)) == 0)
			return policy;
	}

	policy = kzalloc(sizeof(struct emcom_xengine_mpflow_ip_bind_policy), GFP_ATOMIC);
	if (!policy)
		return NULL;

	for (i = 0; i < addr_len; i++)
		policy->addr[i] = addr[i];
	policy->lte_cnt = 0;
	policy->pattern.tot_cnt = 0;
	policy->bind_mode = 0;

	hlist_add_head(&policy->node, &hashtable[hash]);
	return policy;
}

static void emcom_xengine_mpflow_ai_hash_delete(const struct emcom_xengine_mpflow_ai_priv *priv,
	const __be32 *addr, uint8_t addr_len)
{
	struct emcom_xengine_mpflow_ip_bind_policy *ip = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash;

	if (addr_len > EMCOM_MPFLOW_AI_CLAT_IPV6)
		return;

	hash = emcom_xengine_mpflow_ip_hash(*(addr + addr_len - 1));
	hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[hash], node) {
		if (memcmp((const void *)(ip->addr), (const void *)addr, addr_len * sizeof(__be32)) == 0) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}
}

static bool emcom_xengine_mpflow_ai_get_addr(uint16_t sa_family,
	struct in_addr *addr, __be32 *s_addr, uint8_t s_addr_len)
{
	uint8_t i;

	if (s_addr_len > EMCOM_MPFLOW_AI_CLAT_IPV6)
		return false;

	if (sa_family == AF_INET) {
		struct in_addr *usin = (struct in_addr *)addr;
		*(s_addr + s_addr_len - 1) = usin->s_addr;
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (sa_family == AF_INET6) {
		struct in6_addr *usin6 = (struct in6_addr *)addr;
		for (i = 0; i < s_addr_len; i++)
			*(s_addr + i) = usin6->s6_addr32[i];
		return true;
	}
#endif
	else {
		return false;
	}
}

static void emcom_xengine_mpflow_ai_udp_reset(struct sock *sk)
{
	emcom_logd("reset sk %pK sport is %u, state[%u]", sk, sk->sk_num, sk->sk_state);
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

static void emcom_xengine_mpflow_ai_enable_selected_path(struct sock *sk,
	const char *selected_path_iface)
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
			/*lint -e666*/
			emcom_logd("UDP sock is already binded by user, rehash sk sk[%pK] sk_userlocks[%#x] "
				"dev_if[%d] inet_num[%d] hash[%d] inet_rcv_saddr: "IPV4_FMT" inet_saddr: "IPV4_FMT,
				sk, sk->sk_userlocks, sk->sk_bound_dev_if, inet_sk(sk)->inet_num, udp_sk(sk)->udp_portaddr_hash,
				ipv4_info(inet_sk(sk)->inet_rcv_saddr), ipv4_info(inet_sk(sk)->inet_saddr));
			inet_sk(sk)->inet_rcv_saddr = 0;
			inet_sk(sk)->inet_saddr = 0;
			/*lint +e666*/

#if IS_ENABLED(CONFIG_IPV6)
			emcom_logd("[MPFlow_KERNEL] sk_v6_rcv_saddr: "IPV6_FMT, ipv6_info(sk->sk_v6_rcv_saddr));
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

static void emcom_xengine_mpflow_ai_path_handover(struct sock *sk, uint16_t bind_mode)
{
	char *selected_path_iface = EMCOM_WLAN0_IFNAME;

	if (!emcom_xengine_mpflow_ai_rehash_sk(sk))
		return;

	if (bind_mode == EMCOM_MPFLOW_BIND_NONE) {
		emcom_logi("path_handover to default route, sk:%pK sport[%u] dev_if[%d] bind[%d] family[%u]",
			sk, sk->sk_num, sk->sk_bound_dev_if, sk->is_mp_flow_bind, sk->sk_family);
		if (sk->sk_bound_dev_if) {
			struct inet_sock *inet = inet_sk(sk);
			sk->sk_bound_dev_if = 0;
			sk_dst_reset(sk);
			if (inet->inet_saddr)
				inet->inet_saddr = 0;
		}
		return;
	}

	switch (bind_mode) {
	case EMCOM_MPFLOW_BIND_WIFI0:
		selected_path_iface = EMCOM_WLAN0_IFNAME;
		break;
	case EMCOM_MPFLOW_BIND_LTE0:
		selected_path_iface = EMCOM_CELL0_IFNAME;
		break;
	case EMCOM_MPFLOW_BIND_WIFI1:
		selected_path_iface = EMCOM_WLAN1_IFNAME;
		break;
	case EMCOM_MPFLOW_BIND_LTE1:
		selected_path_iface = EMCOM_CELL1_IFNAME;
		break;
	default:
		break;
	}

	emcom_logi("path_handover sk:%pK sport[%u] dev_if[%d] bind[%d] family[%u] dev[%s]",
		sk, sk->sk_num, sk->sk_bound_dev_if,
		sk->is_mp_flow_bind, sk->sk_family, selected_path_iface);
	emcom_xengine_mpflow_ai_enable_selected_path(sk, selected_path_iface);
}

extern struct inet_hashinfo tcp_hashinfo;

static void emcom_xengine_mpflow_ai_tcp_intf_reset(const struct emcom_xengine_mpflow_ai_info *app,
	const struct reset_flow_policy_info *reset,
	bool *need_reset_device)
{
	struct sock *sk = NULL;
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
	if (sk == NULL) {
		emcom_loge("tcp reset flow not found");
		return;
	}
	if ((!sk->is_mp_flow_bind && sk->sk_bound_dev_if) ||
		emcom_xengine_mpflow_ai_is_excluded_port(app, ntohs(sk->sk_dport), IPPROTO_TCP)) {
		sock_put(sk);
		return;
	}
	emcom_logi("reset sk:%pK sport[%u], state[%u] bind[%d] act[%u] family[%u] protocol[%u]",
		sk, sk->sk_num, sk->sk_state, sk->is_mp_flow_bind, reset->policy.act, sk->sk_family, sk->sk_protocol);
	if (reset->policy.act == SK_ERROR) {
		emcom_xengine_mpflow_reset(tcp_sk(sk));
		*need_reset_device = true;
		emcom_logi("tcp reset completed");
	} else {
		emcom_loge("tcp reset action not support, action: %u", reset->policy.act);
	}
	sock_put(sk);
}

static void emcom_xengine_mpflow_ai_udp_intf_reset(const struct reset_flow_policy_info *reset,
	bool *need_reset_device)
{
	struct sock *sk = NULL;
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
		emcom_logi("reset sk:%pK sport[%u], state[%u] bind[%d] act[%u] family[%u] protocol[%u]",
			sk, sk->sk_num, sk->sk_state, sk->is_mp_flow_bind, reset->policy.act, sk->sk_family, sk->sk_protocol);
		if (sk->sk_protocol == IPPROTO_UDP) {
			if (reset->policy.act == SK_ERROR) {
				emcom_xengine_mpflow_ai_udp_reset(sk);
				*need_reset_device = true;
			} else if (reset->policy.act == INTF_CHANGE) {
				emcom_xengine_mpflow_ai_path_handover(sk, reset->policy.rst_bind_mode);
			} else {
				emcom_loge("udp reset action not support, action: %u", reset->policy.act);
			}
			emcom_logi("udp reset completed");
		}
		sock_put(sk);
	} else {
		emcom_loge("udp flow not found");
	}
}

/*
 * mod flow local interface
 */
void emcom_xengine_mpflow_ai_reset_loc_intf(const uint8_t *data, uint16_t len)
{
	struct reset_flow_policy_info *reset = NULL;
	bool need_reset_device = false;
	int8_t app_index;
	struct emcom_xengine_mpflow_ai_info *app = NULL;

	if ((data == NULL) || (len < sizeof(struct reset_flow_policy_info))) {
		emcom_loge("mod flow pointer null or length %d error", len);
		return;
	}

	reset = (struct reset_flow_policy_info *)data;
	if (reset->flow.l3proto == ETH_P_IP) {
		emcom_logi("receive reset. SrcIP["IPV4_FMT"] SrcPort[%u] "
			"DstIP["IPV4_FMT"] DstPort[%u] l4proto[%u] l3proto[%u] intf[%u]",
			ipv4_info(reset->flow.ipv4_sip), reset->flow.src_port,
			ipv4_info(reset->flow.ipv4_dip), reset->flow.dst_port,
			reset->flow.l4proto, reset->flow.l3proto, reset->flow.sk_dev_itf);
	} else {
		emcom_logi("receive reset. SrcIP["IPV6_FMT"] SrcPort[%u] "
			"DstIP["IPV6_FMT"] DstPort[%u] l4proto[%u] l3proto[%u] intf[%u]",
			ipv6_info(reset->flow.ipv6_sip), reset->flow.src_port,
			ipv6_info(reset->flow.ipv6_dip), reset->flow.dst_port,
			reset->flow.l4proto, reset->flow.l3proto, reset->flow.sk_dev_itf);
	}
	emcom_logi("reset mode[%u] blinktime[%u]", reset->policy.rst_bind_mode, reset->policy.const_perid);

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(reset->uid);
	if (app_index == INDEX_INVALID) {
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];
	spin_unlock_bh(&g_mpflow_ai_lock);

	switch (reset->flow.l4proto) {
	case IPPROTO_TCP:
		emcom_xengine_mpflow_ai_tcp_intf_reset(app, reset, &need_reset_device);
		break;

	case IPPROTO_UDP:
		emcom_xengine_mpflow_ai_udp_intf_reset(reset, &need_reset_device);
		break;
	default:
		emcom_logi("mpflow ai reset unsupport protocol: %d.\n", reset->flow.l4proto);
		break;
	}

	if (need_reset_device) {
		spin_lock_bh(&g_mpflow_ai_lock);
		app->rst_jiffies = jiffies;
		app->rst_bind_mode = reset->policy.rst_bind_mode;
		app->rst_duration = msecs_to_jiffies(reset->policy.const_perid);
		app->rst_devif = reset->flow.sk_dev_itf;
		spin_unlock_bh(&g_mpflow_ai_lock);
		if (reset->flow.l3proto == ETH_P_IP) {
			emcom_logi("[MPFlow_KERNEL]Reset Completed. SrcIP["IPV4_FMT"] SrcPort[%u] "
				"DstIP["IPV4_FMT"] DstPort[%u] l4proto[%u] intf[%u]",
				ipv4_info(reset->flow.ipv4_sip), reset->flow.src_port,
				ipv4_info(reset->flow.ipv4_dip), reset->flow.dst_port,
				reset->flow.l4proto, reset->flow.sk_dev_itf);
		} else {
			emcom_logi("[MPFlow_KERNEL]Reset Completed. SrcIP["IPV6_FMT"] SrcPort[%u] "
				"DstIP["IPV6_FMT"] DstPort[%u] l4proto[%u] intf[%u]",
				ipv6_info(reset->flow.ipv6_sip), reset->flow.src_port,
				ipv6_info(reset->flow.ipv6_dip), reset->flow.dst_port,
				reset->flow.l4proto, reset->flow.sk_dev_itf);
		}
	}
}

static uint16_t emcom_xengine_transfer_bindmode(uint16_t bind_mode)
{
	uint16_t qos_device = EMCOM_MPFLOW_BIND_NONE;
	switch (bind_mode) {
	case EMCOM_MPFLOW_WIFI0_MASK:
		qos_device = EMCOM_MPFLOW_BIND_WIFI0;
		break;
	case EMCOM_MPFLOW_LTE0_MASK:
		qos_device = EMCOM_MPFLOW_BIND_LTE0;
		break;
	case EMCOM_MPFLOW_WIFI1_MASK:
		qos_device = EMCOM_MPFLOW_BIND_WIFI1;
		break;
	case EMCOM_MPFLOW_LTE1_MASK:
		qos_device = EMCOM_MPFLOW_BIND_LTE1;
		break;
	default:
		break;
	}

	return qos_device;
}

static uint8_t emcom_xengine_transfer_ifname(const char* ifname)
{
	if (strncmp(ifname, EMCOM_WLAN0_IFNAME, strlen(EMCOM_WLAN0_IFNAME)) == 0) {
		return EMCOM_MPFLOW_BIND_WIFI0;
	}
	if (strncmp(ifname, EMCOM_CELL0_IFNAME, strlen(EMCOM_CELL0_IFNAME)) == 0) {
		return EMCOM_MPFLOW_BIND_LTE0;
	}
	if (strncmp(ifname, EMCOM_WLAN1_IFNAME, strlen(EMCOM_WLAN1_IFNAME)) == 0) {
		return EMCOM_MPFLOW_BIND_WIFI1;
	}
	if (strncmp(ifname, EMCOM_CELL1_IFNAME, strlen(EMCOM_CELL1_IFNAME)) == 0) {
		return EMCOM_MPFLOW_BIND_LTE1;
	}

	emcom_logd("ifname is wrong!");
	return EMCOM_MPFLOW_BIND_NONE;
}

static void emcom_xengine_net_type_transfer_ifname(int net_type, char* ifname, int len)
{
	int ret;
	switch (net_type) {
	case EMCOM_XENGINE_NET_WLAN0:
		ret = strcpy_s(ifname, len, EMCOM_WLAN0_IFNAME);
		break;
	case EMCOM_XENGINE_NET_CELL0:
		ret = strcpy_s(ifname, len, EMCOM_CELL0_IFNAME);
		break;
	case EMCOM_XENGINE_NET_WLAN1:
		ret = strcpy_s(ifname, len, EMCOM_WLAN1_IFNAME);
		break;
	case EMCOM_XENGINE_NET_CELL1:
		ret = strcpy_s(ifname, len, EMCOM_CELL1_IFNAME);
		break;
	default:
		ret = memset_s(ifname, len, 0, len);
		break;
	}
	if (ret)
		emcom_loge("strcpy_s failed ret=%d, net_type=%d", ret, net_type);

	return;
}

static bool emcom_xengine_mpflow_ai_check_loop_sk(struct sock *sk)
{
	struct sockaddr_in6 addr;
	if (emcom_xengine_is_v6_sock(sk)) {
		if (ipv6_addr_equal(&sk->sk_v6_daddr, &sk->sk_v6_rcv_saddr)) {
			if (ipv6_addr_any(&sk->sk_v6_daddr))
				return false;
			emcom_logi("ipv6 loop sk_v6_daddr:"IPV6_FMT, ipv6_info(sk->sk_v6_daddr));
			return true;
		}
	} else {
		if (sk->sk_daddr == sk->sk_rcv_saddr) {
			if (sk->sk_daddr == 0)
				return false;
			emcom_logi("ipv4 loop sk_daddr:"IPV4_FMT, ipv4_info(sk->sk_daddr));
			return true;
		}
	}

	if (!emcom_xengine_transfer_sk_to_addr(sk, (struct sockaddr *)&addr)) {
		emcom_logi("transfer sk to addr fail");
		return true;
	}
	if (!emcom_xengine_check_ip_addrss((struct sockaddr *)&addr) ||
		emcom_xengine_check_ip_is_private((struct sockaddr *)&addr)) {
		emcom_logi("ip is illegal");
		return true;
	}

	return false;
}

static bool emcom_xengine_mpflow_ai_need_reset(struct sock *sk,
	uint16_t bindmode, uint8_t cur_net_type, uint16_t ori_dev)
{
	if (bindmode == EMCOM_MPFLOW_BIND_NONE && ((sk->sk_bound_dev_if == EMCOM_MPFLOW_BIND_NONE) ||
		cur_net_type == emcom_xengine_get_net_type(EMCOM_XENGINE_DEFAULT_NET_ID) +
		EMCOM_MPFLOW_BIND_WIFI0 - EMCOM_XENGINE_NET_WLAN0)) {
		emcom_logi("ignore flow already in defult dev, port: %u, state: %u devif: %d cur_net_type: %u",
			sk->sk_num, sk->sk_state, sk->sk_bound_dev_if, cur_net_type);
		return false;
	}
	if (cur_net_type == bindmode || cur_net_type == EMCOM_MPFLOW_BIND_NONE ||
		(ori_dev && cur_net_type != ori_dev) ||
		unlikely(!refcount_inc_not_zero(&sk->sk_refcnt))) {
		return false;
	}
	return true;
}

/*
 * transfer flow to one
 */
static void emcom_xengine_mpflow_ai_close_tcp_flow(const struct emcom_xengine_mpflow_ai_info *app,
	const uint16_t bindmode,
	const uint16_t ori_dev)
{
	uint32_t uid = app->uid;
	struct sock *sk = NULL;
	uint32_t bucket = 0;
	struct sock *sk_arr[SKARR_SZ];
	int accum = 0;
	int idx = 0;

	for (; bucket <= tcp_hashinfo.ehash_mask; bucket++) {
		struct inet_ehash_bucket *head = &tcp_hashinfo.ehash[bucket];
		spinlock_t *lock = inet_ehash_lockp(&tcp_hashinfo, bucket);
		struct hlist_nulls_node *node = NULL;

		if (hlist_nulls_empty(&head->chain))
			continue;

		spin_lock_bh(lock);
		sk_nulls_for_each(sk, node, &head->chain) {
			if (sk->sk_state == TCP_ESTABLISHED || sk->sk_state == TCP_SYN_SENT) {
				uid_t sock_uid = sock_i_uid(sk).val;
				char ifname[IFNAMSIZ] = {0};
				uint8_t bind_type;
				if (uid != sock_uid)
					continue;
				if (emcom_xengine_mpflow_ai_check_loop_sk(sk))
					continue;
				if ((!sk->is_mp_flow_bind && sk->sk_bound_dev_if) ||
					emcom_xengine_mpflow_ai_is_excluded_port(app, ntohs(sk->sk_dport), IPPROTO_TCP))
					continue;

				if (netdev_get_name(sock_net(sk), ifname, sk->sk_bound_dev_if) != 0) {
					if (sk->sk_bound_dev_if == EMCOM_MPFLOW_BIND_NONE) {
						int net_type = emcom_xengine_get_net_type(sk->sk_mark & EMCOM_XENGINE_NET_ID_MASK);
						emcom_xengine_net_type_transfer_ifname(net_type, ifname, sizeof(ifname));
					} else {
						continue;
					}
				}
				bind_type = emcom_xengine_transfer_ifname(ifname);
				if (!emcom_xengine_mpflow_ai_need_reset(sk, bindmode, bind_type, ori_dev)) {
					continue;
				}
				sk_arr[accum] = sk;
				accum++;
			}

			if (accum >= SKARR_SZ)
				break;
		}
		spin_unlock_bh(lock);
		if (accum >= SKARR_SZ)
			break;
	}
	emcom_logi("reset tcp flow, uid: %u bindmode: %hu ori: %hu flow_num: %d", uid, bindmode, ori_dev, accum);

	for (; idx < accum; idx++) {
		emcom_xengine_mpflow_reset(tcp_sk(sk_arr[idx]));
		sock_gen_put(sk_arr[idx]);
	}
}

static void emcom_xengine_mpflow_ai_close_udp_flow(const uint32_t uid, const uint16_t bindmode,
	const uint16_t ori_dev, const reset_act act)
{
	struct sock *sk = NULL;
	uint32_t bucket = 0;
	struct sock *sk_arr[SKARR_SZ];
	int accum = 0;
	int idx = 0;

	for (; bucket <= udp_table.mask; bucket++) {
		struct udp_hslot *hslot = &udp_table.hash[bucket];

		if (hlist_empty(&hslot->head))
			continue;

		spin_lock_bh(&hslot->lock);
		sk_for_each(sk, &hslot->head) {
			if (sk->sk_state != TCP_SYN_RECV && sk->sk_state != TCP_TIME_WAIT &&
				sk->sk_state != TCP_NEW_SYN_RECV) {
				uid_t sock_uid = sock_i_uid(sk).val;
				char ifname[IFNAMSIZ] = {0};
				uint8_t bind_type;
				if (uid != sock_uid) {
					continue;
				}
				if (emcom_xengine_mpflow_ai_check_loop_sk(sk))
					continue;
				if (netdev_get_name(sock_net(sk), ifname, sk->sk_bound_dev_if) != 0) {
					if (sk->sk_bound_dev_if == EMCOM_MPFLOW_BIND_NONE) {
						int net_type = emcom_xengine_get_net_type(sk->sk_mark & EMCOM_XENGINE_NET_ID_MASK);
						emcom_xengine_net_type_transfer_ifname(net_type, ifname, sizeof(ifname));
						emcom_logi("sk:%pK port: %u, state: %u devif: %d ifname: %s mark: %#x",
							sk, sk->sk_num, sk->sk_state, sk->sk_bound_dev_if, ifname, sk->sk_mark);
					} else {
						continue;
					}
				}
				bind_type = emcom_xengine_transfer_ifname(ifname);
				if (!emcom_xengine_mpflow_ai_need_reset(sk, bindmode, bind_type, ori_dev)) {
					continue;
				}
				sk_arr[accum] = sk;
				accum++;
			}
			if (accum >= SKARR_SZ)
				break;
		}
		spin_unlock_bh(&hslot->lock);
		if (accum >= SKARR_SZ)
			break;
	}
	emcom_logi("reset udp flow, uid: %u action: %u bindmode: %hu ori: %hu flow_num: %d",
		uid, act, bindmode, ori_dev, accum);

	for (; idx < accum; idx++) {
		if (act == SK_ERROR) {
			emcom_xengine_mpflow_ai_udp_reset(sk_arr[idx]);
		} else if (act == INTF_CHANGE) {
			emcom_xengine_mpflow_ai_path_handover(sk_arr[idx], bindmode);
		} else {
			emcom_loge("close udp flow not support, action: %u", act);
		}
		sock_gen_put(sk_arr[idx]);
	}
}

void emcom_xengine_mpflow_ai_close_all_flow(const uint8_t *data, uint16_t len)
{
	struct transfer_flow_info *transfer = NULL;
	uint16_t low_bit;
	uint16_t ori_dev;
	uint32_t uid;
	uint16_t bindmode;
	int8_t app_index;
	reset_act act;
	struct emcom_xengine_mpflow_ai_info *app = NULL;

	if ((data == NULL) || (len < sizeof(struct transfer_flow_info))) {
		emcom_loge("mod flow pointer null or length %d error", len);
		return;
	}

	transfer = (struct transfer_flow_info *)data;
	low_bit = transfer->policy.rst_bind_mode & EMCOM_MPFLOW_BINDMODE_ORI_DEV_MASK;
	ori_dev = transfer->policy.rst_bind_mode >> EMCOM_MPFLOW_BINDMODE_ORI_DEV_OFFSET;
	uid = transfer->uid;
	bindmode = emcom_xengine_transfer_bindmode(low_bit);
	ori_dev = emcom_xengine_transfer_bindmode(ori_dev);
	act = transfer->policy.act;
	emcom_logi("reset all flow:uid[%u] low_bit[%#x] bindmode[%hu] ori_dev[%hu] act[%d] time_ms[%u]",
		uid, low_bit, bindmode, ori_dev, act, transfer->policy.const_perid);

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(uid);
	if (app_index == INDEX_INVALID) {
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];
	if (!transfer->policy.const_perid) {
		app->rst_all_flow = false;
		app->all_flow_duration = 0;
		emcom_logi("clear reset all flow");
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app->all_flow_mode = bindmode;
	app->rst_all_flow = true;
	app->all_flow_jiffies = jiffies;
	app->all_flow_duration = msecs_to_jiffies(transfer->policy.const_perid);
	spin_unlock_bh(&g_mpflow_ai_lock);

	emcom_xengine_mpflow_ai_close_tcp_flow(app, bindmode, ori_dev);
	emcom_xengine_mpflow_ai_close_udp_flow(uid, bindmode, ori_dev, act);
}

void emcom_xengine_mpflow_ai_change_burst_ratio(const uint8_t *data, uint16_t len)
{
	struct change_burst_ratio *new_burst_ratio = NULL;
	uint32_t uid;
	int8_t app_index;
	struct emcom_xengine_mpflow_ai_info *app = NULL;

	if ((data == NULL) || (len < sizeof(struct change_burst_ratio))) {
		emcom_loge("change burst ratio pointer null or length %u error", len);
		return;
	}
	new_burst_ratio = (struct change_burst_ratio *)data;
	uid = new_burst_ratio->uid;

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(uid);
	if (app_index == INDEX_INVALID) {
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];
	app->burst_info.burst_ratio[WLAN0_RATIO] = new_burst_ratio->ratio[WLAN0_RATIO];
	app->burst_info.burst_ratio[CELL0_RATIO] = new_burst_ratio->ratio[CELL0_RATIO];
	app->burst_info.burst_ratio[WLAN1_RATIO] = new_burst_ratio->ratio[WLAN1_RATIO];
	app->burst_info.burst_ratio[CELL1_RATIO] = new_burst_ratio->ratio[CELL1_RATIO];
	spin_unlock_bh(&g_mpflow_ai_lock);
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
	g_mpflow_ai_uids[index].rst_bind_mode = EMCOM_XENGINE_MPFLOW_AI_BINDMODE_NONE;
	g_mpflow_ai_uids[index].rst_jiffies = 0;
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

static void emcom_xengine_mpflow_ai_hash_clear(const struct emcom_xengine_mpflow_ai_priv *priv)
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

void emcom_xengine_mpflow_ai_ip_config(const char *data, uint16_t len)
{
	struct emcom_xengine_mpflow_ai_ip_cfg *config = NULL;
	struct emcom_xengine_mpflow_ip_bind_policy *ip = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	__be32 daddr[EMCOM_MPFLOW_AI_CLAT_IPV6] = {0};
	int8_t app_index;

	if (!data || (len % sizeof(struct emcom_xengine_mpflow_ai_ip_cfg))) {
		emcom_loge("invalid data, length: %u expect: %zu", len,
			sizeof(struct emcom_xengine_mpflow_ai_ip_cfg));
		return;
	}

	config = (struct emcom_xengine_mpflow_ai_ip_cfg *)data;

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(config->uid);
	if (app_index == INDEX_INVALID) {
		emcom_loge("get app fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	app = &g_mpflow_ai_uids[app_index];
	if (!emcom_xengine_mpflow_ai_get_addr(config->sa_family, &config->v4addr,
			daddr, sizeof(daddr) / sizeof(__be32))) {
		emcom_loge("get address fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	ip = emcom_xengine_mpflow_ai_hash(daddr, sizeof(daddr) / sizeof(__be32), app->priv->hashtable);
	if (!ip) {
		emcom_loge("hash fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	if (config->sa_family == AF_INET) {
		emcom_logi("ip policy config. dest ip:"IPV4_FMT", bindmode: %#x, uid: %u",
			ipv4_info(config->v4addr), config->bind_mode, config->uid);
	} else if (config->sa_family == AF_INET6) {
		emcom_logi("ip policy config. dest ip:"IPV6_FMT", bindmode: %#x, uid: %u",
			ipv6_info(config->v6addr), config->bind_mode, config->uid);
	}
	if (config->bind_mode != EMCOM_MPFLOW_BIND_NONE) {
		ip->uid = config->uid;
		ip->bind_mode = config->bind_mode;
	} else {
		emcom_xengine_mpflow_ai_hash_delete(app->priv, daddr, sizeof(daddr) / sizeof(__be32));
	}

	spin_unlock_bh(&g_mpflow_ai_lock);
}

static void emcom_xengine_mpflow_ai_policy_init(
	const struct emcom_xengine_mpflow_ai_init_bind_cfg *config,
	struct emcom_xengine_mpflow_ai_info *app)
{
	const struct emcom_xengine_mpflow_ai_init_bind_policy *policy = NULL;
	uint32_t i;
	uint32_t index;

	for (index = 0; index < config->policy_num; index++) {
		policy = &config->scatter_bind[index];

		if ((policy->l4_protocol != IPPROTO_TCP) && (policy->l4_protocol != IPPROTO_UDP)) {
			emcom_loge("invalid protocol: %d", policy->l4_protocol);
			continue;
		}
		if (policy->mode > EMCOM_MPFLOW_IP_BURST_FIX) {
			emcom_loge("invalid mode: %d", policy->mode);
			continue;
		}

		for (i = 0; (i < policy->port_num) && (i < EMCOM_MPFLOW_BIND_PORT_CFG_SIZE); i++) {
			emcom_logi("port[%d, %d]", policy->port_range[i].start_port,
				policy->port_range[i].end_port);

			if (policy->port_range[i].start_port > policy->port_range[i].end_port) {
				emcom_loge("error port");
				continue;
			}
			app->ports[app->port_num].protocol = policy->l4_protocol;
			app->ports[app->port_num].range.start_port = policy->port_range[i].start_port;
			app->ports[app->port_num].range.end_port = policy->port_range[i].end_port;

			/* reserved for future logic */
			app->ports[app->port_num].pattern.select_mode = policy->mode;
			app->ports[app->port_num].pattern.ratio[WLAN0_RATIO] = policy->ratio[WLAN0_RATIO];
			app->ports[app->port_num].pattern.ratio[CELL0_RATIO]  = policy->ratio[CELL0_RATIO];
			app->ports[app->port_num].pattern.ratio[WLAN1_RATIO] = policy->ratio[WLAN1_RATIO];
			app->ports[app->port_num].pattern.ratio[CELL1_RATIO]  = policy->ratio[CELL1_RATIO];
			/* reserved for future logic end */

			if (policy->l4_protocol == IPPROTO_TCP) {
				app->ports[app->port_num].bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM;
			} else {
				app->ports[app->port_num].bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI;
			}
			app->port_num++;
			if (app->port_num >= EMCOM_MPFLOW_BIND_PORT_SIZE) {
				emcom_loge("too many port: %d", app->port_num);
				return;
			}
		}
	}
}

void emcom_xengine_mpflow_ai_init_bind_config(const char *data, uint16_t len)
{
	struct emcom_xengine_mpflow_ai_init_bind_cfg *config = NULL;
	struct emcom_xengine_mpflow_ai_init_bind_policy *burst_policy = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	int8_t app_index;
	uint32_t i, j;

	if (!data || (len != sizeof(struct emcom_xengine_mpflow_ai_init_bind_cfg))) {
		emcom_loge("input length error expect: %zu, real: %u",
			sizeof(struct emcom_xengine_mpflow_ai_init_bind_cfg), len);
		return;
	}

	config = (struct emcom_xengine_mpflow_ai_init_bind_cfg *)data;
	if (!emcom_xengine_mpflow_ai_start(config->uid))
		return;

	emcom_logi("[MPFlow_KERNEL] Config received. policy num: %u", config->policy_num);

	if (config->policy_num > EMCOM_MPFLOW_BIND_PORT_CFG_SIZE) {
		emcom_loge("too many policy");
		return;
	}
	burst_policy = &config->burst_bind;
	emcom_logi("burst: proto[%u] mode[%u] portnum[%u]", burst_policy->l4_protocol, burst_policy->mode,
		burst_policy->port_num);

	if (burst_policy->l4_protocol != 0) {
		if ((burst_policy->l4_protocol != IPPROTO_TCP) && (burst_policy->l4_protocol != IPPROTO_UDP)) {
			emcom_loge("burst: invalid protocol: %d", burst_policy->l4_protocol);
			return;
		}
	}

	if (burst_policy->mode > EMCOM_MPFLOW_IP_BURST_FIX) {
		emcom_loge("burst: invalid mode: %d", burst_policy->mode);
		return;
	}

	spin_lock_bh(&g_mpflow_ai_lock);
	app_index = emcom_xengine_mpflow_ai_finduid(config->uid);
	if (app_index == INDEX_INVALID) {
		emcom_loge("get app fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];
	app->burst_info.burst_port_num = 0;
	app->burst_info.burst_protocol = burst_policy->l4_protocol;
	app->burst_info.burst_mode = burst_policy->mode;
	app->burst_info.burst_ratio[WLAN0_RATIO] = burst_policy->ratio[WLAN0_RATIO];
	app->burst_info.burst_ratio[CELL0_RATIO] = burst_policy->ratio[CELL0_RATIO];
	app->burst_info.burst_ratio[WLAN1_RATIO] = burst_policy->ratio[WLAN1_RATIO];
	app->burst_info.burst_ratio[CELL1_RATIO] = burst_policy->ratio[CELL1_RATIO];

	j = 0;
	for (i = 0; i < burst_policy->port_num; i++) {
		emcom_logi("burst: port range[%d, %d]", burst_policy->port_range[i].start_port,
			burst_policy->port_range[i].end_port);

		if (burst_policy->port_range[i].start_port > burst_policy->port_range[i].end_port) {
			emcom_loge("burst: invalid port range");
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
	emcom_xengine_mpflow_ai_policy_init(config, app);

	for (i = 0; i < EMCOM_MPFLOW_BIND_PORT_CFG_SIZE; i++)
		app->excluded_tcp_ports[i] = config->excluded_tcp_ports[i];
	spin_unlock_bh(&g_mpflow_ai_lock);
}

int8_t emcom_xengine_mpflow_ai_get_port_index(int8_t index,
	const struct emcom_xengine_mpflow_dport_range *range, uint8_t proto)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct emcom_xengine_mpflow_dport_range *exist = NULL;
	int8_t i;
	int8_t port_index = EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

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

static void emcom_xengine_mpflow_ai_clear_blocked(int uid, uint16_t net_bit)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	errno_t err;

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &g_mpflow_ai_list[index];
		if (node->uid == uid && node->mpflow_fallback != EMCOM_MPFLOW_FALLBACK_CLR) {
			uint8_t bind_device = emcom_xengine_transfer_ifname(node->name);
			if (bind_device == EMCOM_MPFLOW_BIND_NONE) {
				continue;
			}
			if (((net_bit >> bind_device) & 0x1) != 0x1) {
				continue;
			}
			if (time_before_eq(jiffies, node->report_jiffies + EMCOM_MPFLOW_FALLBACK_WAIT_TIME * HZ)) {
				continue;
			}
			node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
			node->mpflow_fail_nopayload = 0;
			node->mpflow_fail_syn_rst = 0;
			node->mpflow_fail_syn_timeout = 0;
			node->start_jiffies = 0;
			err = memset_s(node->retrans_count, sizeof(node->retrans_count),
						   0, sizeof(node->retrans_count));
			if (err != EOK)
				emcom_logd("emcom_xengine_mpflow_clear_blocked memset failed");
		}
	}
}


void emcom_xengine_mpflow_ai_iface_cfg(const char *data, uint16_t len)
{
	struct emcom_xengine_mpflow_ai_iface_config *config = NULL;
	struct emcom_xengine_mpflow_ai_info *app = NULL;
	int8_t app_index;
	int8_t port_index;
	uint16_t config_high_bit;
	uint16_t app_high_bit;

	if (!data || (len != sizeof(struct emcom_xengine_mpflow_ai_iface_config))) {
		emcom_loge("invalid data length %u expect: %zu", len,
			sizeof(struct emcom_xengine_mpflow_ai_iface_config));
		return;
	}

	config = (struct emcom_xengine_mpflow_ai_iface_config *)data;
	if (config->port_range.start_port > config->port_range.end_port) {
		emcom_loge("invalid port range[%d, %d]",
			config->port_range.start_port, config->port_range.end_port);
		return;
	}

	spin_lock_bh(&g_mpflow_ai_lock);

	app_index = emcom_xengine_mpflow_ai_finduid(config->uid);
	if (app_index == INDEX_INVALID) {
		emcom_loge("get app fail, uid: %d", config->uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}
	app = &g_mpflow_ai_uids[app_index];

	port_index = emcom_xengine_mpflow_ai_get_port_index(app_index, &config->port_range, config->l4protocol);
	if (port_index == EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX) {
		emcom_loge("port range[%d, %d] not exist",
			config->port_range.start_port, config->port_range.end_port);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	config_high_bit = (config->bind_mode >> EMCOM_MPFLOW_BINDMODE_SHIFT) & EMCOM_MPFLOW_BINDMODE_MASK;
	app_high_bit = (app->ports[port_index].bind_mode >> EMCOM_MPFLOW_BINDMODE_SHIFT) & EMCOM_MPFLOW_BINDMODE_MASK;
	if (config->bind_mode != app->ports[port_index].bind_mode) {
		if ((config->bind_mode & EMCOM_MPFLOW_BINDMODE_MASK) == 0 && app->priv)
			emcom_xengine_mpflow_ai_hash_clear(app->priv);
		if (config_high_bit != app_high_bit)
			emcom_xengine_mpflow_ai_clear_blocked(config->uid, (config_high_bit ^ app_high_bit) & config_high_bit);
		app->ports[port_index].pattern.tot_cnt = 0;
		emcom_logi("iface policy port[%d, %d] bind mode: %#x",
			config->port_range.start_port, config->port_range.end_port, config->bind_mode);
	}
	app->ports[port_index].bind_mode = config->bind_mode;

	spin_unlock_bh(&g_mpflow_ai_lock);
}

static bool emcom_xengine_mpflow_ai_is_block(const int bind_device, const bool is_wifi0_block,
	const bool is_lte0_block, const bool is_wifi1_block, const bool is_lte1_block)
{
	bool res = true;

	switch (bind_device) {
	case EMCOM_MPFLOW_BIND_WIFI0:
		res = is_wifi0_block;
		break;
	case EMCOM_MPFLOW_BIND_LTE0:
		res = is_lte0_block;
		break;
	case EMCOM_MPFLOW_BIND_WIFI1:
		res = is_wifi1_block;
		break;
	case EMCOM_MPFLOW_BIND_LTE1:
		res = is_lte1_block;
		break;
	default:
		break;
	}

	return res;
}

int emcom_xengine_mpflow_ai_bind_random(struct emcom_xengine_mpflow_ai_bind_pattern *calc,
	uid_t uid, uint16_t high_bit)
{
	uint16_t bind_device = calc->last_device;
	int cnt = 0;
	bool is_wifi0_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN0_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_lte0_block = emcom_xengine_mpflow_blocked(uid, EMCOM_CELL0_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_wifi1_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN1_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_lte1_block = emcom_xengine_mpflow_blocked(uid, EMCOM_CELL1_IFNAME, EMCOM_MPFLOW_VER_V2);
	if ((is_wifi0_block == true) && (is_lte0_block == true) &&
		(is_wifi1_block == true) && (is_lte1_block == true)) {
		calc->last_device = EMCOM_MPFLOW_BIND_NONE;
		return EMCOM_MPFLOW_BIND_NONE;
	}

	do {
		// find no suitable device, then break
		if (++cnt > EMCOM_MPFLOW_DEV_NUM) {
			emcom_loge("mpflow ai can't find bind_device");
			break;
		}
		if (bind_device == EMCOM_MPFLOW_BIND_LTE1) {
			bind_device = EMCOM_MPFLOW_BIND_WIFI0;
		} else {
			bind_device++;
		}
	} while ((((high_bit >> (bind_device - 1)) & 0x1) == 0x0) ||
		emcom_xengine_mpflow_ai_is_block(bind_device, is_wifi0_block, is_lte0_block, is_wifi1_block, is_lte1_block));

	emcom_logi("mpflow ai bind random to device: %d", bind_device);
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

	emcom_logd("mpflow ai start uid: %u", uid);

	spin_lock_bh(&g_mpflow_ai_lock);
	index = emcom_xengine_mpflow_ai_finduid(uid);
	if (index == INDEX_INVALID) {
		emcom_logd("new app uid: %d", uid);
		newindex = emcom_xengine_mpflow_ai_getfreeindex();
		if (newindex == INDEX_INVALID) {
			emcom_loge("mpflow ai start get free index exceed. uid: %d", uid);
			spin_unlock_bh(&g_mpflow_ai_lock);
			return false;
		}
		index = newindex;
		is_new_uid = true;
	}
	app = &g_mpflow_ai_uids[index]; /*lint !e676*/

	if (is_new_uid) {
		emcom_xengine_mpflow_clear_blocked(uid, EMCOM_MPFLOW_VER_V2);

		priv = kzalloc(sizeof(struct emcom_xengine_mpflow_ai_priv), GFP_ATOMIC);
		if (!priv) {
			emcom_loge("mpflow ai start alloc priv failed, uid: %d", uid);
			spin_unlock_bh(&g_mpflow_ai_lock);
			return false;
		}
		for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++)
			INIT_HLIST_HEAD(&priv->hashtable[i]);

		app->priv = priv;
		app->uid = uid;
		app->port_num = 0;
		app->enableflag = EMCOM_MPFLOW_AI_ENABLEFLAG_DPORT;
		app->rst_bind_mode = EMCOM_XENGINE_MPFLOW_AI_BINDMODE_NONE;
		app->rst_duration = EMCOM_MPFLOW_AI_RESET_DURATION;
		app->rst_jiffies = 0;
		app->all_flow_mode = EMCOM_XENGINE_MPFLOW_AI_BINDMODE_NONE;
		app->all_flow_duration = EMCOM_MPFLOW_AI_RESET_DURATION;
		app->all_flow_jiffies = 0;
		app->rst_all_flow = false;
		app->rst_devif = 0;
	}
	spin_unlock_bh(&g_mpflow_ai_lock);

	emcom_xengine_mpflow_register_nf_hook(EMCOM_MPFLOW_VER_V2);
	return true;
}

void emcom_xengine_mpflow_ai_stop(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_stop_info *stop = NULL;
	int8_t index;
	int32_t stop_reason;
	bool mpflow_ai_uids_empty = false;

	if (!pdata || (len != sizeof(struct emcom_xengine_mpflow_parse_stop_info))) {
		emcom_loge("mpflow ai stop data or length %d is error", len);
		return;
	}

	stop = (struct emcom_xengine_mpflow_parse_stop_info *)pdata;
	stop_reason = stop->stop_reason;
	emcom_logd("mpflow ai stop uid: %u, stop reason: %u", stop->uid, stop_reason);

	spin_lock_bh(&g_mpflow_ai_lock);
	index = emcom_xengine_mpflow_ai_finduid(stop->uid);
	if (index != INDEX_INVALID)
		emcom_xengine_mpflow_ai_app_clear(index, stop->uid);

	emcom_xengine_mpflow_delete(stop->uid, EMCOM_MPFLOW_VER_V2);
	mpflow_ai_uids_empty = emcom_xengine_mpflow_ai_uid_empty();
	spin_unlock_bh(&g_mpflow_ai_lock);

	if (mpflow_ai_uids_empty)
		emcom_xengine_mpflow_unregister_nf_hook(EMCOM_MPFLOW_VER_V2);
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

static bool emcom_xengine_mpflow_ai_bind_block(uid_t uid, uint16_t qos_device)
{
	bool is_wifi0_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN0_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_lte0_block = emcom_xengine_mpflow_blocked(uid, EMCOM_CELL0_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_wifi1_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN1_IFNAME, EMCOM_MPFLOW_VER_V2);
	bool is_lte1_block = emcom_xengine_mpflow_blocked(uid, EMCOM_CELL1_IFNAME, EMCOM_MPFLOW_VER_V2);
	if ((is_wifi0_block && (qos_device == EMCOM_MPFLOW_BIND_WIFI0)) ||
		(is_lte0_block && (qos_device == EMCOM_MPFLOW_BIND_LTE0)) ||
		(is_wifi1_block && (qos_device == EMCOM_MPFLOW_BIND_WIFI1)) ||
		(is_lte1_block && (qos_device == EMCOM_MPFLOW_BIND_LTE1)) ||
		((is_wifi0_block == true) && (is_lte0_block == true) &&
			(is_wifi1_block == true) && (is_lte1_block == true))) {
		emcom_logi("mpflow bind blocked uid: %u, bindmode: %#x, blocked:%d, %d, %d, %d",
					uid, qos_device, is_wifi0_block, is_lte0_block, is_wifi1_block, is_lte1_block);
		return true;
	}

	return false;
}

static void emcom_xengine_mpflow_ai_get_ratio(struct emcom_xengine_mpflow_ai_burst_port *burst_info,
	uint8_t* cur_burst_ratio, uint16_t high_bit)
{
	cur_burst_ratio[WLAN0_RATIO] = burst_info->burst_ratio[WLAN0_RATIO];
	cur_burst_ratio[CELL0_RATIO] = burst_info->burst_ratio[CELL0_RATIO];
	cur_burst_ratio[WLAN1_RATIO] = burst_info->burst_ratio[WLAN1_RATIO];
	cur_burst_ratio[CELL1_RATIO] = burst_info->burst_ratio[CELL1_RATIO];

	if ((high_bit & 0x1) == 0x0)
		cur_burst_ratio[WLAN0_RATIO] = 0;
	if (((high_bit >> CELL0_RATIO) & 0x1) == 0x0)
		cur_burst_ratio[CELL0_RATIO] = 0;
	if (((high_bit >> WLAN1_RATIO) & 0x1) == 0x0)
		cur_burst_ratio[WLAN1_RATIO] = 0;
	if (((high_bit >> CELL1_RATIO) & 0x1) == 0x0)
		cur_burst_ratio[CELL1_RATIO] = 0;
}

int emcom_xengine_mpflow_ai_bind_burst(int8_t index, uint16_t dport, uint8_t proto,
	uint32_t bind_mode, struct emcom_xengine_mpflow_ai_bind_pattern *pattern_calc, uid_t uid)
{
	uint16_t low_bit = bind_mode & EMCOM_MPFLOW_BINDMODE_MASK;
	uint16_t high_bit = (bind_mode >> EMCOM_MPFLOW_BINDMODE_SHIFT) & EMCOM_MPFLOW_BINDMODE_MASK;
	uint16_t qos_device = emcom_xengine_transfer_bindmode(low_bit);
	uint16_t launch_device;
	uint8_t launch_cnt;
	uint8_t cur_pos;

	if (qos_device == EMCOM_MPFLOW_BIND_NONE) {
		return EMCOM_MPFLOW_BIND_NONE;
	}

	if (emcom_xengine_mpflow_ai_bind_block(uid, qos_device)) {
		return EMCOM_MPFLOW_BIND_NONE;
	}

	if (emcom_xengine_mpflow_ai_in_busrt_range(index, dport, proto)) {
		int burst_device;
		uint8_t cur_burst_ratio[BURST_SIZE];
		struct emcom_xengine_mpflow_ai_burst_port *burst_info = &g_mpflow_ai_uids[index].burst_info;
		emcom_xengine_mpflow_ai_get_ratio(burst_info, cur_burst_ratio, high_bit);
		emcom_logi("burst current qos device: %u last burst qos device: %u, burst cnt: %lu, mode[%u]",
		qos_device, burst_info->launch_device, burst_info->burst_cnt, burst_info->burst_mode);

		if (burst_info->burst_mode != EMCOM_MPFLOW_IP_BURST_FIX)
			return qos_device;

		if (!cur_burst_ratio[WLAN0_RATIO] && !cur_burst_ratio[CELL0_RATIO] &&
			!cur_burst_ratio[WLAN1_RATIO] && !cur_burst_ratio[CELL1_RATIO])
			return qos_device;

		if (burst_info->launch_device != qos_device) {
			burst_info->burst_cnt = 0;
			burst_info->jiffies = jiffies;
			burst_info->launch_device = qos_device;
			return qos_device;
		}

		/*lint -e666*/
		if (time_after(jiffies, (burst_info->jiffies + EMCOM_MPFLOW_FLOW_BIND_BURST_TIME)) ||
			(burst_info->jiffies == 0)) {
			burst_device = qos_device;
			burst_info->burst_cnt = 0;
			burst_info->jiffies = jiffies;
			burst_info->launch_device = burst_device; // update launch device every burst period begining
		} else {
			// base device
			launch_device = burst_info->launch_device;
			launch_cnt = burst_info->burst_cnt % (cur_burst_ratio[WLAN0_RATIO] + cur_burst_ratio[CELL0_RATIO] +
				cur_burst_ratio[WLAN1_RATIO] + cur_burst_ratio[CELL1_RATIO]);
			// last device postion, bindmode - 1
			cur_pos = launch_device - 1;
			// find this burst device
			while (launch_cnt >= cur_burst_ratio[cur_pos]) {
				launch_cnt = launch_cnt - cur_burst_ratio[cur_pos];
				if (cur_pos != (EMCOM_MPFLOW_BIND_LTE1 - 1)) {
					// jump to next device
					cur_pos++;
				} else {
					// need restart
					cur_pos = EMCOM_MPFLOW_BIND_WIFI0 - 1;
				}
			}
			burst_device = cur_pos + 1;
			burst_info->burst_cnt++;
		}
		/*lint +e666*/

		return burst_device;
	}

	return qos_device;
}

int emcom_xengine_mpflow_ai_get_reset_device(uint16_t mode)
{
	int bind_device;
	switch (mode) {
	case EMCOM_XENGINE_MPFLOW_AI_BINDMODE_WIFI0:
		bind_device = EMCOM_MPFLOW_BIND_WIFI0;
		break;

	case EMCOM_XENGINE_MPFLOW_AI_BINDMODE_LTE0:
		bind_device = EMCOM_MPFLOW_BIND_LTE0;
		break;

	case EMCOM_XENGINE_MPFLOW_AI_BINDMODE_WIFI1:
		bind_device = EMCOM_MPFLOW_BIND_WIFI1;
		break;

	case EMCOM_XENGINE_MPFLOW_AI_BINDMODE_LTE1:
		bind_device = EMCOM_MPFLOW_BIND_LTE1;
		break;

	default:
		bind_device = EMCOM_MPFLOW_BIND_NONE;
		break;
	}
	emcom_logi("mpflow ai bind using reset mode: %u, device: %u", mode, bind_device);
	return bind_device;
}

int8_t emcom_xengine_mpflow_ai_get_port_index_in_range(int8_t index, uint16_t dport, uint8_t proto)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct emcom_xengine_mpflow_dport_range *exist = NULL;
	int8_t i;
	int8_t port_index = EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

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

int emcom_xengine_mpflow_ai_getmode(int8_t index, uid_t uid, struct sockaddr *uaddr, uint8_t proto)
{
	struct emcom_xengine_mpflow_ip_bind_policy *ip_policy = NULL;
	struct emcom_xengine_mpflow_ai_bind_pattern *pattern_calc = NULL;
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	__be32 daddr[EMCOM_MPFLOW_AI_CLAT_IPV6] = {0};
	uint16_t dport = 0;
	uint32_t bind_mode = 0;
	uint8_t port_index;
	uint16_t low_bit;
	uint16_t high_bit;

	/* apply all flow reset first */
	if (app->rst_all_flow)
		return emcom_xengine_mpflow_ai_get_reset_device(app->all_flow_mode);

	/* apply reset mode second */
	if (time_after(jiffies, app->rst_jiffies + app->rst_duration))
		app->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	else
		return emcom_xengine_mpflow_ai_get_reset_device(app->rst_bind_mode);

	if (!app->priv) {
		emcom_loge("mpflow ai get mod priv error, uid: %u", app->uid);
		return EMCOM_MPFLOW_BIND_NONE;
	}

	if (!emcom_xengine_mpflow_ai_get_addr_port(uaddr, daddr, &dport))
		return EMCOM_MPFLOW_BIND_NONE;

	port_index = emcom_xengine_mpflow_ai_get_port_index_in_range(index, dport, proto);
	if (port_index >= EMCOM_MPFLOW_BIND_PORT_SIZE) {
		emcom_logi("no port match");
		return EMCOM_MPFLOW_BIND_NONE;
	}

	/* if ip policy matched, use ip policy, else use port policy as default */
	ip_policy = emcom_xengine_mpflow_ai_hash_get_policy(daddr, sizeof(daddr) / sizeof(__be32), app->priv->hashtable);
	if (ip_policy) {
		bind_mode = ip_policy->bind_mode;
		pattern_calc = &ip_policy->pattern;
		emcom_logi("ip match, bindmode: %#x", bind_mode);
	} else {
		bind_mode = app->ports[port_index].bind_mode;
		pattern_calc = &app->ports[port_index].pattern;
		emcom_logi("port match, bindmode: %#x", bind_mode);
	}

	low_bit = bind_mode & EMCOM_MPFLOW_BINDMODE_MASK;
	high_bit = (bind_mode >> EMCOM_MPFLOW_BINDMODE_SHIFT) & EMCOM_MPFLOW_BINDMODE_MASK;

	if (low_bit == EMCOM_MPFLOW_BINDMODE_MASK) {
		bind_mode = emcom_xengine_mpflow_ai_bind_random(pattern_calc, uid, high_bit);
	} else if (low_bit == 0) {
		bind_mode = EMCOM_MPFLOW_BIND_NONE;
		emcom_logd("mpflow ai get spec mod error, mode: %u", bind_mode);
	} else {
		bind_mode = emcom_xengine_mpflow_ai_bind_burst(index, dport, proto, bind_mode, pattern_calc, uid);
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

bool emcom_xengine_mpflow_ai_is_excluded_port(const struct emcom_xengine_mpflow_ai_info *app,
	uint16_t dport, uint8_t proto)
{
	unsigned int i;
	if (proto == IPPROTO_TCP) {
		for (i = 0; i < EMCOM_MPFLOW_BIND_PORT_CFG_SIZE; i++) {
			if (app->excluded_tcp_ports[i] != 0 &&
				app->excluded_tcp_ports[i] == (int32_t)dport)
				return true;
		}
	}
	return false;
}

bool emcom_xengine_mpflow_ai_checkvalid(struct sock *sk, struct sockaddr *uaddr, int8_t index, uint16_t dport)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	bool isvalidaddr = false;
	int8_t port_index = EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX;

	if (!sk || !uaddr)
		return false;

	if (emcom_xengine_mpflow_ai_is_excluded_port(app, dport, sk->sk_protocol))
		return false;

	isvalidaddr = emcom_xengine_check_ip_addrss(uaddr) && (!emcom_xengine_check_ip_is_private(uaddr));
	if (isvalidaddr == false) {
		emcom_logd("invalid addr. uid: %u", app->uid);
		return false;
	}

	if (app->enableflag & EMCOM_MPFLOW_AI_ENABLEFLAG_DPORT) {
		if (usin->sin_family != AF_INET && usin->sin_family != AF_INET6) {
			emcom_logd("unsupport family uid: %u, sin_family: %d",
						app->uid, usin->sin_family);
			return false;
		}

		if (time_after(jiffies, app->all_flow_jiffies + app->all_flow_duration)) {
			app->all_flow_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
			app->rst_all_flow = false;
		} else if (app->rst_all_flow) {
			emcom_logd("reset all flow");
			return true;
		}

		port_index = emcom_xengine_mpflow_ai_get_port_index_in_range(index, dport, sk->sk_protocol);
		if (port_index == EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX) {
			emcom_logd("port not in range uid: %u, dport: %d",
						app->uid, dport);
			return false;
		}

		emcom_logd("mpflow ai check uid: %u sk: %pK, famliy: %d, sk_proto: %d, "
					"policy_proto: %d, port: %d bindmode: %#x",
					app->uid, sk, sk->sk_family, sk->sk_protocol,
					app->ports[port_index].protocol, dport,
					app->ports[port_index].bind_mode);

		if (sk->sk_protocol != app->ports[port_index].protocol) {
			emcom_logd("protocol not match uid: %u, sk: %pK", app->uid, sk);
			return false;
		}
	} else {
		emcom_logd("mpflow ai valid uid: %u enableflag: %u sk: %pK, famliy: %d, sk_proto: %d, "
					"policy_proto: %d, port: %d bindmode: %#x",
					app->uid, app->enableflag, sk, sk->sk_family, sk->sk_protocol,
					app->ports[port_index].protocol, dport,
					app->ports[port_index].bind_mode);
	}

	return true;
}

static int emcom_xengine_update_bind_iface_name(char *ifname, unsigned int len, int bind_device)
{
	const char *src_ifname = emcom_xengine_get_network_iface_name(bind_device - EMCOM_MPFLOW_BIND_WIFI0);
	if (src_ifname == NULL || strcmp(src_ifname, INVALID_IFACE_NAME) == 0)
		return -1;
	return memcpy_s(ifname, len, src_ifname, (strlen(src_ifname) + 1));
}

static int emcom_xengine_mpflow_rehash(int bind_device, struct sock *sk, int8_t index,
	struct inet_sock *inet, uid_t uid, uint16_t dport, struct sockaddr *uaddr)
{
	struct emcom_xengine_mpflow_ai_info *app = &g_mpflow_ai_uids[index];
	char ifname[IFNAMSIZ] = {0};
	errno_t err = EOK;
	struct net_device *dev = NULL;

	if (bind_device == EMCOM_MPFLOW_BIND_NONE) {
		/* 1.wzry sgame stuck one min when shutdown LTE, because UDP not create a new sock when receive a reset,
		the old sock sk_bound_dev_if is LTE, so the UDP sock will continue use LTE to send packet
		2.as a wifi hotpoint, can not bind wlan, need reset sk_bound_dev_if to 0 */
		emcom_logd("EMCOM_MPFLOW_BIND_NONE uid: %u, sk: %pK, protocol: %u, DstPort: %u, dev_if: %u", uid, sk, sk->sk_protocol, dport, sk->sk_bound_dev_if);
		if (sk->sk_bound_dev_if && sk->sk_protocol == IPPROTO_UDP) {
			/* Fix the bug for weixin app bind wifi ip, then reset to lte, cannot find the sk */
			if (!emcom_xengine_mpflow_ai_rehash_sk(sk))
				return MPFLOW_ERROR;
			sk->sk_bound_dev_if = 0;
			sk_dst_reset(sk);
			inet = inet_sk(sk);
			if (inet->inet_saddr)
				inet->inet_saddr = 0;
		} else {
			sk->sk_bound_dev_if = 0;
		}
		return MPFLOW_ERROR;
	} else {
		err = emcom_xengine_update_bind_iface_name(ifname, IFNAMSIZ - 1, bind_device);
	}

	if (err != EOK) {
		emcom_loge("mpflow ai bind memcpy ifname failed");
		return MPFLOW_ERROR;
	}

	/* Fix the bug for weixin app bind wifi ip, then reset to lte, cannot find the sk */
	if (!emcom_xengine_mpflow_ai_rehash_sk(sk))
		return MPFLOW_ERROR;

	rcu_read_lock();
	dev = dev_get_by_name_rcu(sock_net(sk), ifname);
	if (!dev || (emcom_xengine_mpflow_getinetaddr(dev) == false)) {
		rcu_read_unlock();
		emcom_logd("device not ready uid: %u, sk: %pK, dev: %pK, name: %s",
				uid, sk, dev, (dev == NULL) ? "null" : dev->name);
		return MPFLOW_ERROR;
	}
	rcu_read_unlock();

	sk->sk_bound_dev_if = dev->ifindex;
	if (sk->sk_protocol == IPPROTO_UDP) {
		sk_dst_reset(sk);
		inet = inet_sk(sk);
		if (inet->inet_saddr)
			inet->inet_saddr = 0;
	}

	if (sk->sk_protocol == IPPROTO_TCP && emcom_mpdns_is_allowed(uid, NULL) && !app->rst_all_flow) {
		int tar_net = emcom_xengine_get_net_type_by_name(ifname);
		int cur_net = emcom_xengine_get_net_type(sk->sk_mark & 0x00FF);
		emcom_mpdns_update_dst_addr(sk, uaddr, cur_net, tar_net);
	}
	emcom_logi("bind success uid: %u, ifname: %s, ifindex: %d, srcPort: %u",
			uid, ifname, sk->sk_bound_dev_if, sk->sk_num);

	return MPFLOW_OK;
}

void emcom_xengine_mpflow_ai_bind2device(struct sock *sk, struct sockaddr *uaddr)
{
	uint16_t dport = 0;
	int bind_device;
	uid_t uid;
	int8_t index;
	struct sockaddr_in *usin = NULL;
	struct inet_sock *inet = NULL;

	if (!sk || !uaddr)
		return;

	if (sk->is_mp_flow_bind || (sk->sk_bound_dev_if && sk->sk_protocol == IPPROTO_TCP))
		return;
	else if (sk->sk_protocol == IPPROTO_UDP)
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

	emcom_xengine_mpflow_ai_get_port(uaddr, &dport);  // not check return value
	if (emcom_xengine_mpflow_ai_checkvalid(sk, uaddr, index, dport) == false) {
		emcom_logd("mpflow ai bind check invalid. uid: %u", uid);
		spin_unlock_bh(&g_mpflow_ai_lock);
		return;
	}

	bind_device = emcom_xengine_mpflow_ai_getmode(index, uid, uaddr, sk->sk_protocol);
	spin_unlock_bh(&g_mpflow_ai_lock);

	sk->is_mp_flow_bind = 1;

	if (emcom_xengine_mpflow_rehash(bind_device, sk, index, inet, uid, dport, uaddr) != MPFLOW_OK)
		return;

	usin = (struct sockaddr_in *)uaddr;
	if (usin->sin_family == AF_INET) {
		emcom_logi("[MPFlow_KERNEL] Bind Completed. SrcIP: *.*, SrcPort: %u, DstIP:"IPV4_FMT", DstPort: %u ",
			sk->sk_num, ipv4_info(usin->sin_addr.s_addr), dport);
	} else if (usin->sin_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)uaddr;
		emcom_logi("[MPFlow_KERNEL] Bind Completed. SrcIP: *.*, SrcPort: %u, DstIP:"IPV6_FMT", DstPort: %u ",
			sk->sk_num, ipv6_info(usin6->sin6_addr), dport);
	}
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("xengine module driver");

