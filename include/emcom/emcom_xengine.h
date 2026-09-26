/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Xengine feature
 * Author: xialiang xialiang4@huawei.com
 * Create: 2019-05-15
 */
#ifndef __EMCOM_XENGINE_H__
#define __EMCOM_XENGINE_H__

#include "mpflow.h"

#define UID_APP 10000
#define UID_INVALID_APP 0
#define INDEX_INVALID (-1)
#define EMCOM_TRUE_VALUE  1
#define EMCOM_XENGINE_DEFAULT_NET_ID 0
#define EMCOM_XENGINE_NET_ID_MASK 0xff
#define EMCOM_MAX_MPIP_APP 5
#define EMCOM_MAX_CCALG_APP 5
#define EMCOM_CONGESTION_CONTROL_ALG_BBR "bbr"
#define INVALID_IFACE_NAME  "none"

#define EMCOM_WLAN0_IFNAME emcom_xengine_get_network_iface_name(EMCOM_XENGINE_NET_WLAN0)
#define EMCOM_CELL0_IFNAME emcom_xengine_get_network_iface_name(EMCOM_XENGINE_NET_CELL0)
#define EMCOM_WLAN1_IFNAME emcom_xengine_get_network_iface_name(EMCOM_XENGINE_NET_WLAN1)
#define EMCOM_CELL1_IFNAME emcom_xengine_get_network_iface_name(EMCOM_XENGINE_NET_CELL1)

#define IFNAMSIZ 16

#define EMCOM_XENGINE_SET_ACCSTATE(sk, value) \
	do { \
		(sk)->acc_state = (value); \
	} while (0)

#define emcom_xengine_set_speedctrl(speedctrl_info, uid_value, size_value) \
	do { \
		spin_lock_bh(&((speedctrl_info).stlocker)); \
		(speedctrl_info).uid = (uid_value); \
		(speedctrl_info).size = (size_value); \
		spin_unlock_bh(&((speedctrl_info).stlocker)); \
	} while (0)

#define emcom_xengine_get_speedctrl_uid(speedctrl_info, uid_value) \
	do { \
		uid = (speedctrl_info).uid_value; \
	} while (0)

#define emcom_xengine_get_speedctrl_info(speedctrl_info, uid_value, size_value) \
	do { \
		spin_lock_bh(&((speedctrl_info).stlocker)); \
		(uid_value) = (speedctrl_info).uid; \
		(size_value) = (speedctrl_info).size; \
		spin_unlock_bh(&((speedctrl_info).stlocker)); \
	} while (0)

#define IP_DEBUG 0

#define ip6_addr_block1(ip6_addr) (ntohs((ip6_addr).s6_addr16[0]) & 0xffff)
#define ip6_addr_block2(ip6_addr) (ntohs((ip6_addr).s6_addr16[1]) & 0xffff)
#define ip6_addr_block3(ip6_addr) (ntohs((ip6_addr).s6_addr16[2]) & 0xffff)
#define ip6_addr_block4(ip6_addr) (ntohs((ip6_addr).s6_addr16[3]) & 0xffff)
#define ip6_addr_block5(ip6_addr) (ntohs((ip6_addr).s6_addr16[4]) & 0xffff)
#define ip6_addr_block6(ip6_addr) (ntohs((ip6_addr).s6_addr16[5]) & 0xffff)
#define ip6_addr_block7(ip6_addr) (ntohs((ip6_addr).s6_addr16[6]) & 0xffff)
#define ip6_addr_block8(ip6_addr) (ntohs((ip6_addr).s6_addr16[7]) & 0xffff)

#if IP_DEBUG
#define IPV4_FMT "%u.%u.%u.%u"
#define ipv4_info(addr) \
	((unsigned char *)&(addr))[0], \
	((unsigned char *)&(addr))[1], \
	((unsigned char *)&(addr))[2], \
	((unsigned char *)&(addr))[3]

#define IPV6_FMT "%x:%x:%x:%x:%x:%x:%x:%x"
#define ipv6_info(addr) \
	ip6_addr_block1(addr), \
	ip6_addr_block2(addr), \
	ip6_addr_block3(addr), \
	ip6_addr_block4(addr), \
	ip6_addr_block5(addr), \
	ip6_addr_block6(addr), \
	ip6_addr_block7(addr), \
	ip6_addr_block8(addr)

#else
#define IPV4_FMT "%u.%u.*.*"
#define ipv4_info(addr) \
	((unsigned char *)&(addr))[0], \
	((unsigned char *)&(addr))[1]

#define IPV6_FMT "%x:***:%x"
#define ipv6_info(addr) \
	ip6_addr_block1(addr), \
	ip6_addr_block8(addr)
#endif

#define emcom_xengine_is_net_type_valid(net_type) \
	((net_type) > EMCOM_XENGINE_NET_INVALID && (net_type) < EMCOM_XENGINE_NET_MAX_NUM)

#define HICOM_SOCK_FLAG_FINTORST 0x00000001

typedef enum {
	EMCOM_XENGINE_ACC_NORMAL = 0,
	EMCOM_XENGINE_ACC_HIGH,
} emcom_xengine_acc_state;

typedef enum {
	EMCOM_XENGINE_MPIP_TYPE_BIND_NEW = 0,
	EMCOM_XENGINE_MPIP_TYPE_BIND_RANDOM,
} emcom_xengine_mpip_type;

typedef enum {
	EMCOM_XENGINE_CONG_ALG_INVALID = 0,
	EMCOM_XENGINE_CONG_ALG_BBR,
} emcom_xengine_ccalg_type;

typedef enum {
	EMCOM_XENGINE_NET_INVALID = -1,
	EMCOM_XENGINE_NET_WLAN0,
	EMCOM_XENGINE_NET_CELL0,
	EMCOM_XENGINE_NET_WLAN1,
	EMCOM_XENGINE_NET_CELL1,
	EMCOM_XENGINE_NET_MAX_NUM,
	EMCOM_XENGINE_NET_DEFAULT = 15,
} emcom_xengine_net_no;

typedef enum {
	EMCOM_XENGINE_IP_TYPE_INVALID,
	EMCOM_XENGINE_IP_TYPE_V4,
	EMCOM_XENGINE_IP_TYPE_V6,
	EMCOM_XENGINE_IP_TYPE_V4V6
} emcom_xengine_ip_type;

struct emcom_xengine_acc_app_info {
	uid_t     uid; /* The uid of accelerate Application */
	uint16_t  age;
	uint16_t  reverse;
};
struct emcom_xengine_speed_ctrl_info {
	uid_t     uid; /* The uid of foreground Application */
	uint32_t  size; /* The grade of speed control */
	spinlock_t stlocker; /* The Guard Lock of this unit */
};
struct emcom_xengine_speed_ctrl_data {
	uid_t     uid; /* The uid of foreground Application */
	uint32_t  size; /* The grade of speed control */
};
struct emcom_xengine_mpip_config {
	uid_t     uid; /* The uid of foreground Application */
	uint32_t  type; /* The type of mpip speed up */
};

struct emcom_xengine_ccalg_config {
	uid_t uid; /* The uid of foreground Application */
	uint32_t alg; /* The alg name of congestion control speed up */
	bool has_log; /* whether this app uid had printed log */
};

struct emcom_xengine_ccalg_config_data {
	uid_t uid; /* The uid of foreground Application */
	uint32_t alg; /* The alg name of congestion control speed up */
};

struct emcom_xengine_bind2device_node {
	struct list_head list;
	uid_t uid;
	pid_t pid;
	int fd;
	int add;
	char iface_name[IFNAMSIZ];
};

struct emcom_xengine_network_info {
	int32_t net_type;
	int32_t net_id;
	int32_t is_validated;
	int32_t is_default;
	char iface_name[IFNAMSIZ];
};

void emcom_xengine_init(void);
int emcom_xengine_clear(void);
bool emcom_xengine_hook_ul_stub(struct sock *pstSock);
void emcom_xengine_speedctrl_winsize(struct sock *pstSock, uint32_t *win);
void emcom_xengine_udpenqueue(const struct sk_buff *skb);
void emcom_xengine_fastsyn(struct sock *pstSock);
void emcom_xengine_evt_proc(int32_t event, const uint8_t *data, uint16_t len);
void emcom_xengine_mpip_bind2device(struct sock *pstSock);
void emcom_xengine_change_default_ca(struct sock *sk, struct list_head tcp_cong_list);
int emcom_xengine_setproxyuid(struct sock *sk, const char __user *optval, int optlen);
int emcom_xengine_setsockflag(struct sock *sk, const char __user *optval, int optlen);
int emcom_xengine_setbind2device(const struct sock *sk, const char __user *optval, int optlen);
void emcom_xengine_bind(struct sock *sk);
void fput_by_pid(pid_t pid, struct file *file);
void emcom_xengine_notify_sock_error(struct sock *sk);
bool emcom_xengine_check_ip_addrss(struct sockaddr *addr);
bool emcom_xengine_check_ip_is_private(struct sockaddr *addr);
bool emcom_xengine_transfer_sk_to_addr(struct sock *sk, struct sockaddr *addr);
bool emcom_xengine_is_private_v4_addr(const struct in_addr *v4_addr);
bool emcom_xengine_is_private_v6_addr(const struct in6_addr *v6_addr);
bool emcom_xengine_is_v6_sock(struct sock *sk);
int emcom_xengine_get_net_type(int net_id);
int emcom_xengine_get_net_type_by_name(const char *iface_name);
int emcom_xengine_get_sock_bound_net_type(const struct sock *sk);
void emcom_xengine_process_http_req(const struct sk_buff *skb, int ip_type);
char *emcom_xengine_get_network_iface_name(int net_type);

#ifdef CONFIG_MPTCP
void emcom_xengine_mptcp_socket_closed(const void *data, int len);
void emcom_xengine_mptcp_socket_switch(const void *data, int len);
void emcom_xengine_mptcp_proxy_fallback(const void *data, int len);
void emcom_xengine_mptcp_fallback(const void *data, int len);
#endif

#endif
