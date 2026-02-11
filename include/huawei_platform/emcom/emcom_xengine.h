/*
 * emcom_xengine.h
 *
 * xengine module implemention
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

#ifndef __EMCOM_XENGINE_H__
#define __EMCOM_XENGINE_H__

#include <net/sock.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/netdevice.h>

#define UID_APP 10000
#define UID_INVALID_APP 0
#define INDEX_INVALID (-1)
#define EMCOM_MAX_MPIP_APP 5
#define EMCOM_MAX_CCALG_APP 5
#define EMCOM_CONGESTION_CONTROL_ALG_BBR "bbr"

#define EMCOM_MPFLOW_MAX_APP  10

#define EMCOM_MPFLOW_DEV_NUM  2
#define EMCOM_MPFLOW_MAX_LIST_NUM (EMCOM_MPFLOW_DEV_NUM * EMCOM_MPFLOW_MAX_APP)
#define EMCOM_MPFLOW_PORT_RANGE_MAX_LEN  100
#define EMCOM_MPFLOW_PORT_RANGE_NUM_MAX  5
#define EMCOM_MPFLOW_DELIMITER_COMMA  ","
#define EMCOM_MPFLOW_DELIMITER_COLON  ':'

#define EMCOM_MPFLOW_ENABLEFLAG_PROTOCOL  0x00000001
#define EMCOM_MPFLOW_ENABLEFLAG_DPORT     0x00000002

#define EMCOM_MPFLOW_ENABLETYPE_NONE 0x00000000
#define EMCOM_MPFLOW_ENABLETYPE_NET_DISK 0x00000001
#define EMCOM_MPFLOW_ENABLETYPE_MARKET 0x00000002
#define EMCOM_MPFLOW_ENABLETYPE_WEIBO 0x00000003
#define EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI 0x00000004

#define EMCOM_MPFLOW_ENABLEFLAG_LTE_FIRST 0x00000001

#define EMCOM_MPFLOW_PROTOCOL_TCP    0x0001
#define EMCOM_MPFLOW_PROTOCOL_UDP    0x0002

#define EMCOM_WLAN_IFNAME   "wlan0"
#define EMCOM_LTE_IFNAME    "rmnet0"

#define EMCOM_MPFLOW_STOP_REASON_TRAFFIC_OVERRUN 1
#define EMCOM_MPFLOW_STOP_REASON_WIFI_DISCONNECTED 2
#define EMCOM_MPFLOW_STOP_REASON_THERMAL_OVERRUN 3
#define EMCOM_MPFLOW_STOP_REASON_NETWORK_ROAMING 4
#define EMCOM_MPFLOW_STOP_REASON_DESKTOP 5
#define EMCOM_MPFLOW_STOP_REASON_DUAL_BLOCK 6
#define EMCOM_MPFLOW_STOP_REASON_SWITCH_OFF 7
#define EMCOM_MPFLOW_STOP_REASON_POWER_SAVING 8
#define EMCOM_MPFLOW_STOP_REASON_SWITCH_TO_WLAN 9
#define EMCOM_MPFLOW_STOP_REASON_NOT_USER_OWNER 10
#define EMCOM_MPFLOW_STOP_REASON_APPDIED 11

#define EMCOM_MPFLOW_BIND_NONE 0
#define EMCOM_MPFLOW_BIND_WIFI 1
#define EMCOM_MPFLOW_BIND_LTE 2

#define EMCOM_MPFLOW_FLOW_SLOW_THREH (20*1024u)
#define EMCOM_MPFLOW_RST_RCV_BYTES_THREH (30*1024u)
#define EMCOM_MPFLOW_RATE_VALID_THREH (30*1024u)

#define EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH 2
#define EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MIN 0
#define EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MAX 5
#define EMCOM_MPFLOW_LTE_FIRST_LTE_THREH 8
#define EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MIN 5
#define EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX 10

#define EMCOM_MPFLOW_LTE_THREH_ADJUST_BYTES (20*1024*1024u)

#define EMCOM_MPFLOW_RST_IFACE_GOOD (2*1024*1024u)
#define EMCOM_MPFLOW_RST_TIME_THREH msecs_to_jiffies(2000)
#define EMCOM_MPFLOW_FLOW_TIME_THREH msecs_to_jiffies(5000)

#define EMCOM_XENGINE_MPFLOW_BINDMODE_NORST 0
#define EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST 1
#define EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI 2
#define EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE 3

#define EMCOM_MPFLOW_FI_PTN_MAX_SIZE 64
#define EMCOM_MPFLOW_FI_PTN_SEPERATE_CHAR "|"
#define EMCOM_MPFLOW_FI_PTN_MAX_NUM 16
#define EMCOM_MPFLOW_FI_ASCII_CODE_SIZE 256
#define EMCOM_MPFLOW_FI_ASCII_CODE_MARK 0xff
#define EMCOM_MPFLOW_FI_STAT_SECONDS 3
#define EMCOM_MPFLOW_FI_PORT_80 80
#define EMCOM_MPFLOW_FI_PORT_443 443
#define EMCOM_MPFLOW_FI_NETDISK_FLOW_NUM 10

#define EMCOM_MPFLOW_FLOW_JIFFIES_REC 5
#define EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH msecs_to_jiffies(500)

#define EMCOM_MPFLOW_NETDISK_RATE_GOOD (1*1024*1024)
#define EMCOM_MPFLOW_NETDISK_RATE_BAD (512*1024)
#define EMCOM_MPFLOW_WEIBO_SIZE (256*1024u)

#define EMCOM_MPFLOW_HASH_SIZE 64
#define EMCOM_MPFLOW_IP_AGING_THREH msecs_to_jiffies(5*1000)

#define EMCOM_XENGINE_IS_UID_VALID(uid) ((uid) > 10000)
#define EMCOM_XENGINE_SET_ACCSTATE(sk, value) \
	do { \
		(sk)->acc_state = (value); \
	} while (0)

#define EMCOM_XENGINE_SET_SPEEDCTRL(speedctrl_info, uid_value, size_value) \
	do { \
		spin_lock_bh(&((speedctrl_info).stlocker)); \
		(speedctrl_info).uid = (uid_value); \
		(speedctrl_info).size = (size_value); \
		spin_unlock_bh(&((speedctrl_info).stlocker)); \
	} while (0)

#define EMCOM_XENGINE_GET_SPEEDCTRL_UID(speedctrl_info, uid_value) \
	do { \
		uid = (speedctrl_info).uid_value; \
	} while (0)

#define EMCOM_XENGINE_GET_SPEEDCTRL_INFO(speedctrl_info, uid_value, size_value) \
	do { \
		spin_lock_bh(&((speedctrl_info).stlocker)); \
		(uid_value) = (speedctrl_info).uid; \
		(size_value) = (speedctrl_info).size; \
		spin_unlock_bh(&((speedctrl_info).stlocker)); \
	} while (0)

#define HICOM_SOCK_FLAG_FINTORST 0x00000001

#define EMCOM_MPFLOW_FALLBACK_LTE_OFFSET 300
#define EMCOM_MPFLOW_FALLBACK_WLAN_OFFSET 400

#define EMCOM_MPFLOW_FALLBACK_NOPAYLOAD 0
#define EMCOM_MPFLOW_FALLBACK_SYN_RST 1
#define EMCOM_MPFLOW_FALLBACK_SYN_TOUT 2
#define EMCOM_MPFLOW_FALLBACK_RETRANS 3

#define EMCOM_MPFLOW_FALLBACK_NOPAYLOAD_THRH 5
#define EMCOM_MPFLOW_FALLBACK_SYN_RST_THRH 3
#define EMCOM_MPFLOW_FALLBACK_SYN_TOUT_THRH 7
#define EMCOM_MPFLOW_FALLBACK_RETRANS_THRH 3
#define EMCOM_MPFLOW_FALLBACK_RETRANS_TIME 5

#define EMCOM_MPFLOW_FALLBACK_CLR 0
#define EMCOM_MPFLOW_FALLBACK_SET 1
#define EMCOM_MPFLOW_FALLBACK_SYNCLR 2
#define EMCOM_MPFLOW_FALLBACK_NONE 3

#define EMCOM_MPFLOW_SND_BYTE_THRESHOLD 8

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
	EMCOM_XENGINE_MPFLOW_BINDMODE_NONE = 0,
	EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI,
	EMCOM_XENGINE_MPFLOW_BINDMODE_LTE,
	EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM,
} emcom_xengine_mpflow_bindmode;

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

struct emcom_xengine_mpflow_dport_range {
	uint16_t start_port;    /* start port */
	uint16_t end_port;      /* end port */
};

struct emcom_xengine_mpflow_parse_start_info {
	uid_t uid;              /* The uid of Acc Application */
	uint32_t enableflag;    /* enable protocol/enable dport */
	uint16_t protocol;      /* tcp or udp */
	uint16_t bindmode;      /* Bind device mode */
	uint32_t algorithm_type;
	uint32_t reserve_field;
	struct emcom_xengine_mpflow_dport_range dport_range[EMCOM_MPFLOW_PORT_RANGE_NUM_MAX]; /* prot range */
	char ptn_80[EMCOM_MPFLOW_FI_PTN_MAX_SIZE]; /* fi pattern for tcp port 80 */
	char ptn_443[EMCOM_MPFLOW_FI_PTN_MAX_SIZE]; /* fi pattern for tcp port 443 */
};

struct emcom_xengine_mpflow_parse_stop_info {
	uid_t uid;              /* The uid of Acc Application */
	int32_t stop_reason;    /* The reason of mpflow_stop */
};

struct emcom_xengine_mpflow_node {
	struct list_head list;
	uint64_t last_bytes_received;
	uint32_t rate_received[EMCOM_MPFLOW_FI_STAT_SECONDS];
	unsigned long start_jiffies;
	struct tcp_sock *tp;
};

struct emcom_xengine_mpflow_iface {
	struct list_head flows;
	uint16_t flow_cnt;
	uint16_t is_wifi:1,
			 is_slow:1,
			 is_sure_no_slow:1;
	uint32_t bytes_received;
	uint32_t max_rate_received;
	uint32_t max_rate_received_flow;
	uint32_t rate_received[EMCOM_MPFLOW_FI_STAT_SECONDS];
	uint16_t flow_valid_cnt;
	uint32_t mean_srtt_ms;
	unsigned long start_jiffies;
};

struct emcom_xengine_mpflow_ptn {
	uint8_t ptn[EMCOM_MPFLOW_FI_PTN_MAX_SIZE];
	uint16_t ptnlen;
	uint8_t *skip;
	uint8_t *shift;
	bool is_init;
};

struct emcom_xengine_mpflow_ip {
	struct hlist_node node;
	uint8_t lte_cnt;
	uint8_t tot_cnt;
	__be32 addr;
	unsigned long jiffies[EMCOM_MPFLOW_FLOW_JIFFIES_REC];
};

struct emcom_xengine_mpflow_app_priv {
	struct hlist_head hashtable[EMCOM_MPFLOW_HASH_SIZE];

	/* the below field is only used by netdisk */
	uint8_t lte_thresh; /* the flow bind to lte in each 10 flows on same ip */
	uint8_t lte_thresh_min;
	uint8_t lte_thresh_max;
	uint8_t lte_first;
};

struct emcom_xengine_mpflow_info {
	uid_t uid;             /* The uid of Acc Application */
	uint32_t enableflag;   /* enable protocol/enable dport */
	uint16_t protocol;     /* tcp or udp */
	uint16_t bindmode;     /* Bind device mode */
	uint32_t algorithm_type;
	uint32_t reserve_field;
	struct emcom_xengine_mpflow_dport_range dport_range[EMCOM_MPFLOW_PORT_RANGE_NUM_MAX];  /* port range */
	struct emcom_xengine_mpflow_iface wifi;
	struct emcom_xengine_mpflow_iface lte;
	uint16_t is_downloading:1,
		rst_to_another:1;
	uint16_t rst_bind_mode;
	unsigned long rst_jiffies;
	struct emcom_xengine_mpflow_ptn ptn_80[EMCOM_MPFLOW_FI_PTN_MAX_NUM];
	uint8_t ptn_80_num;
	struct emcom_xengine_mpflow_ptn ptn_443[EMCOM_MPFLOW_FI_PTN_MAX_NUM];
	uint8_t ptn_443_num;
	struct emcom_xengine_mpflow_app_priv *app_priv;
};

struct emcom_xengine_mpflow_stat {
	struct list_head list;
	int32_t uid;
	int ifindex;
	char name[IFNAMSIZ];
	int16_t mpflow_estlink;          /* ESTABLISHED link */
	uint8_t mpflow_fallback;         /* fallback flag */
	uint8_t mpflow_fail_nopayload;   /* failure count from app server no payload */
	uint8_t mpflow_fail_syn_rst;     /* failure count from syn rst */
	uint8_t mpflow_fail_syn_timeout; /* failure count from syn timeout */
	unsigned long start_jiffies;
	uint16_t retrans_count[EMCOM_MPFLOW_FALLBACK_RETRANS_TIME + 1];
};

struct emcom_xengine_mpflow_fallback {
	int32_t uid;
	int32_t reason;
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
void emcom_xengine_notify_sock_error(struct sock *sk);

bool emcom_xengine_check_ip_addrss(struct sockaddr *addr);
bool emcom_xengine_check_ip_is_private(struct sockaddr *addr);
void emcom_xengine_mpflow_init(void);
void emcom_xengine_mpflow_clear(void);
int8_t emcom_xengine_mpflow_finduid(uid_t uid);
int8_t emcom_xengine_mpflow_getfreeindex(void);
void emcom_xengine_mpflow_start(const char *pdata, uint16_t len);
void emcom_xengine_mpflow_stop(const char *pdata, uint16_t len);
bool emcom_xengine_mpflow_getinetaddr(struct net_device *dev);
int emcom_xengine_mpflow_getmode_rand(int8_t index, uid_t uid, struct sockaddr *uaddr);
int emcom_xengine_mpflow_getmode_spec(int8_t index, uid_t uid);
int emcom_xengine_mpflow_getmode(int8_t index, uid_t uid, struct sockaddr *uaddr);
void emcom_xengine_mpflow_bind2device(struct sock *sk, struct sockaddr *uaddr);
bool emcom_xengine_mpflow_finddport(struct emcom_xengine_mpflow_info *mpflowinfo, uint16_t dport);
struct emcom_xengine_mpflow_stat *emcom_xengine_mpflow_get(int uid, char *name, int ifindex);
void emcom_xengine_mpflow_delete(int uid);
void emcom_xengine_mpflow_clear_blocked(int uid);
bool emcom_xengine_mpflow_blocked(int uid, char *name);
int16_t emcom_xengine_mpflow_connum(int uid, char *name);
void emcom_xengine_mpflow_report(void *data, int len);
void emcom_xengine_mpflow_show(void);
bool emcom_xengine_mpflow_errlink(int reason, struct emcom_xengine_mpflow_stat *node);
bool emcom_xengine_mpflow_retrans(int reason, struct emcom_xengine_mpflow_stat *node);
int8_t emcom_xengine_mpflow_checkstatus(struct sock *sk, int reason, int state, struct emcom_xengine_mpflow_stat *node);
void emcom_xengine_mpflow_fallback(struct sock *sk, int reason, int state);

#ifdef CONFIG_MPTCP
void emcom_xengine_mptcp_socket_closed(const void *data, int len);
void emcom_xengine_mptcp_socket_switch(const void *data, int len);
void emcom_xengine_mptcp_proxy_fallback(const void *data, int len);
void emcom_xengine_mptcp_fallback(const void *data, int len);
#endif

#endif
