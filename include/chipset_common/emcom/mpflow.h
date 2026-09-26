/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Mpflow feature
 * Author: xialiang xialiang4@huawei.com
 * Create: 2019-05-15
 */
#ifndef __MPFLOW_H__
#define __MPFLOW_H__

#include <net/sock.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/netdevice.h>

#define EMCOM_MPFLOW_MAX_APP  10
#define EMCOM_MPFLOW_AI_MAX_APP  10
#define EMCOM_MPFLOW_BIND_PORT_SIZE 16
#define EMCOM_MPFLOW_BIND_PORT_CFG_SIZE 8
#define EMCOM_MPFLOW_BIND_INVALID_PORT_INDEX EMCOM_MPFLOW_BIND_PORT_SIZE
#define EMCOM_MPFLOW_AI_MAX_LIST_NUM (EMCOM_MPFLOW_DEV_NUM * EMCOM_MPFLOW_AI_MAX_APP)

#define WLAN0_RATIO 0
#define CELL0_RATIO  1
#define WLAN1_RATIO 2
#define CELL1_RATIO  3
#define BURST_SIZE 4

#define MPFLOW_ERROR (-1)
#define MPFLOW_OK 0

#define EMCOM_MPFLOW_IP_BURST_OFF 0
#define EMCOM_MPFLOW_IP_BURST_AUTO 1
#define EMCOM_MPFLOW_IP_BURST_FIX 2

#define EMCOM_MPFLOW_VER_V1 1
#define EMCOM_MPFLOW_VER_V2 2

#define EMCOM_MPFLOW_AI_RESET_DURATION msecs_to_jiffies(500)

#define EMCOM_MPFLOW_DEV_NUM  4
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
#define EMCOM_MPFLOW_BIND_WIFI0 1
#define EMCOM_MPFLOW_BIND_LTE0 2
#define EMCOM_MPFLOW_BIND_WIFI1 3
#define EMCOM_MPFLOW_BIND_LTE1 4

#define EMCOM_MPFLOW_WIFI0_MASK 1
#define EMCOM_MPFLOW_LTE0_MASK 2
#define EMCOM_MPFLOW_WIFI1_MASK 4
#define EMCOM_MPFLOW_LTE1_MASK 8

#define EMCOM_MPFLOW_BIND_WIFI 1
#define EMCOM_MPFLOW_BIND_LTE 2

#define EMCOM_MPFLOW_BINDMODE_ORI_DEV_OFFSET 8
#define EMCOM_MPFLOW_BINDMODE_ORI_DEV_MASK ((1 << EMCOM_MPFLOW_BINDMODE_ORI_DEV_OFFSET) - 1)

#define EMCOM_MPFLOW_SHIFT_TWO 2
#define EMCOM_MPFLOW_SHIFT_THREE 3

#define EMCOM_MPFLOW_AI_CLAT_IPV6 4

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
#define EMCOM_MPFLOW_FLOW_BIND_BURST_TIME msecs_to_jiffies(500)

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

#define mpflow_ai_in_range(value, svalue, evalue) ((value) >= (svalue) && (value) <= (evalue))

#define EMCOM_MPFLOW_FALLBACK_LTE_OFFSET 300
#define EMCOM_MPFLOW_FALLBACK_WLAN_OFFSET 400
#define EMCOM_MPFLOW_FALLBACK_LTE1_OFFSET 600
#define EMCOM_MPFLOW_FALLBACK_WLAN1_OFFSET 700


#define EMCOM_MPFLOW_FALLBACK_NOPAYLOAD 0
#define EMCOM_MPFLOW_FALLBACK_SYN_RST 1
#define EMCOM_MPFLOW_FALLBACK_SYN_TOUT 2
#define EMCOM_MPFLOW_FALLBACK_RETRANS 3
#define EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST 4

#define EMCOM_MPFLOW_FALLBACK_NOPAYLOAD_THRH 5
#define EMCOM_MPFLOW_FALLBACK_SYN_RST_THRH 3
#define EMCOM_MPFLOW_FALLBACK_SYN_TOUT_THRH 3
#define EMCOM_MPFLOW_FALLBACK_RETRANS_THRH 3
#define EMCOM_MPFLOW_FALLBACK_RETRANS_TIME 5
#define EMCOM_MPFLOW_FALLBACK_SYN_TOUT_TIME 3

#define EMCOM_MPFLOW_FALLBACK_CLR 0
#define EMCOM_MPFLOW_FALLBACK_SET 1
#define EMCOM_MPFLOW_FALLBACK_SYNCLR 2
#define EMCOM_MPFLOW_FALLBACK_NONE 3

#define EMCOM_MPFLOW_SND_BYTE_THRESHOLD 8
#define EMCOM_MPFLOW_BINDMODE_MASK 0xFFFF
#define EMCOM_MPFLOW_BINDMODE_SHIFT 16
#define EMCOM_MPFLOW_FALLBACK_WAIT_TIME 10

/* Definition of mpflow xengine configuration information */
typedef enum {
	EMCOM_XENGINE_MPFLOW_AI_BINDMODE_NONE = 0,
	EMCOM_XENGINE_MPFLOW_AI_BINDMODE_WIFI0,
	EMCOM_XENGINE_MPFLOW_AI_BINDMODE_LTE0,
	EMCOM_XENGINE_MPFLOW_AI_BINDMODE_WIFI1,
	EMCOM_XENGINE_MPFLOW_AI_BINDMODE_LTE1,
	BIND_MODE_HIGH_BIT_START = 16,
	EMCOM_XENGINE_MPFLOW_AI_BINDMODE_RANDOM = (1 << BIND_MODE_HIGH_BIT_START) - 1
} emcom_xengine_mpflow_ai_bindmode;

/* Definition of mpflow xengine configuration information */
typedef enum {
	EMCOM_XENGINE_MPFLOW_BINDMODE_NONE = 0,
	EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI,
	EMCOM_XENGINE_MPFLOW_BINDMODE_LTE,
	EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM,
	EMCOM_XENGINE_MPFLOW_BINDMODE_SHIFT,
} emcom_xengine_mpflow_bindmode;

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
	uint16_t is_wifi : 1,
			 is_slow : 1,
			 is_sure_no_slow : 1;
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
	uint16_t is_downloading : 1,
		rst_to_another : 1;
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
	unsigned long report_jiffies;
	uint16_t retrans_count[EMCOM_MPFLOW_FALLBACK_RETRANS_TIME + 1];
};

struct emcom_xengine_mpflow_fallback_ver {
	uid_t uid;
	uint8_t ver;
	int8_t index;
};

struct emcom_xengine_mpflow_fallback {
	int32_t uid;
	int32_t reason;
};

typedef enum {
	IMMEDIATE = 1,
	ON_SAME_TYPE_FIN,
	ON_OTH_DEV_SAME_TYPE_FIN
} reset_trigger;

typedef enum {
	SK_ERROR = 1,
	INTF_CHANGE
} reset_act;

/* Reset related definition: FlowInfo */
struct flow_info {
	uint16_t l3proto;
	uint8_t l4proto;
	union {
		struct {
			uint32_t ipv4_sip;
			uint32_t ipv4_dip;
		};
#if defined(CONFIG_IPV6)
		struct {
			struct in6_addr ipv6_sip;
			struct in6_addr ipv6_dip;
		};
#endif
	};
	uint16_t src_port;
	uint16_t dst_port;
	int sk_dev_itf;
};

/* policy to kernel: RstPolicy2K */
struct reset_policy {
	reset_trigger    trigger;
	reset_act        act;
	uint32_t      const_perid;
	uint16_t      rst_bind_mode;
};

struct reset_flow_policy_info {
	uint32_t uid; /* The uid of application */
	struct flow_info flow;
	struct reset_policy policy;
};

struct transfer_flow_info {
	uint32_t uid; /* The uid of application */
	struct reset_policy policy;
};

struct change_burst_ratio {
	int uid; /* The uid of application */
	uint8_t ratio[4];
};

// interface daemon to kernel: BindPolicy
struct emcom_xengine_mpflow_ai_ip_cfg {
	uid_t    uid;            /* The uid of application */
	uint32_t bind_mode;       /* Bind device mode */
	uint16_t sa_family;

	/* destination */
	union {
		struct in_addr v4addr;
#if defined(CONFIG_IPV6)
		struct in6_addr v6addr;
#endif
	};
	uint8_t l4protocol;
};

// interface daemon to kernel: InitBindConfig app start
struct emcom_xengine_mpflow_ai_port_cfg {
	uid_t   uid;            /* The uid of application */
	uint8_t l4_protocol;
	uint8_t port_num;
	uint8_t ip_burst[BURST_SIZE];    /* wifi vs. lte */
	struct emcom_xengine_mpflow_dport_range port_range[EMCOM_MPFLOW_BIND_PORT_CFG_SIZE];
};

struct emcom_xengine_mpflow_ai_init_bind_policy {
	uint8_t l4_protocol;
	uint8_t mode; /* fix auto off */
	uint8_t ratio[BURST_SIZE]; /* wifi vs. lte */
	uint8_t port_num;
	struct emcom_xengine_mpflow_dport_range port_range[EMCOM_MPFLOW_BIND_PORT_CFG_SIZE];
};

struct emcom_xengine_mpflow_ai_init_bind_cfg {
	uid_t   uid;            /* The uid of application */
	uint8_t policy_num;
	int32_t excluded_tcp_ports[EMCOM_MPFLOW_BIND_PORT_CFG_SIZE];
	struct emcom_xengine_mpflow_ai_init_bind_policy burst_bind;
	struct emcom_xengine_mpflow_ai_init_bind_policy scatter_bind[EMCOM_MPFLOW_BIND_PORT_CFG_SIZE];
};

// interface daemon to kernel: IfaceBindInfoCalc
struct emcom_xengine_mpflow_ai_iface_config {
	uid_t    uid;            /* The uid of application */
	uint32_t bind_mode;       /* Bind device mode */
	uint8_t l4protocol;
	struct emcom_xengine_mpflow_dport_range port_range;
};

struct emcom_xengine_mpflow_ai_bind_pattern {
	uint8_t wifi_part;
	uint8_t lte_part;
	uint8_t select_mode;
	uint32_t tot_cnt;
	uint8_t ratio[BURST_SIZE];
	uint16_t last_device;     /* last match device */
	unsigned long jiffies;
};

struct emcom_xengine_mpflow_ip_bind_policy {
	struct hlist_node node;
	uid_t    uid;            /* The uid of application */
	__be32   addr[EMCOM_MPFLOW_AI_CLAT_IPV6];
	uint16_t lastdevice;     /* last match device */
	uint32_t bind_mode;       /* Bind device mode */
	uint32_t lte_cnt;
	unsigned long jiffies;
	struct emcom_xengine_mpflow_ai_bind_pattern pattern;
};

struct emcom_xengine_mpflow_ai_port {
	uint32_t bind_mode;     /* Bind device mode */
	uint16_t protocol;      /* tcp or udp */
	struct emcom_xengine_mpflow_ai_bind_pattern pattern;
	struct emcom_xengine_mpflow_dport_range range;
};
struct emcom_xengine_mpflow_ai_priv {
	struct hlist_head hashtable[EMCOM_MPFLOW_HASH_SIZE];
};

struct emcom_xengine_mpflow_ai_burst_port {
	uint8_t burst_port_num;
	uint8_t burst_protocol; /* tcp udp */
	uint8_t burst_mode; /* off auto fix */
	uint8_t burst_ratio[BURST_SIZE];
	uint16_t launch_device;     /* last match device */
	unsigned long jiffies;
	unsigned long burst_cnt;
	struct emcom_xengine_mpflow_dport_range burst_ports[EMCOM_MPFLOW_BIND_PORT_CFG_SIZE];
};

#define EMCOM_MPFLOW_AI_ENABLEFLAG_PROTOCOL  0x01
#define EMCOM_MPFLOW_AI_ENABLEFLAG_DPORT     0x02

struct emcom_xengine_mpflow_ai_info {
	uid_t uid; /* The uid of Acc Application */
	uint8_t enableflag; /* enable protocol/enable dport */
	uint8_t port_num; /* config port number */
	uint16_t rst_bind_mode;
	uint16_t all_flow_mode;
	int rst_devif;
	unsigned long rst_duration; /* ms */
	unsigned long rst_jiffies;
	unsigned long all_flow_duration; /* ms */
	unsigned long all_flow_jiffies;
	bool rst_all_flow;

	struct emcom_xengine_mpflow_ai_port ports[EMCOM_MPFLOW_BIND_PORT_SIZE];
	struct emcom_xengine_mpflow_ai_priv *priv;
	struct emcom_xengine_mpflow_ai_burst_port burst_info;
	int32_t excluded_tcp_ports[EMCOM_MPFLOW_BIND_PORT_CFG_SIZE];
};

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
struct emcom_xengine_mpflow_stat *emcom_xengine_mpflow_get(int uid, const char *name, int ifindex, uint8_t ver);
void emcom_xengine_mpflow_delete(int uid, uint8_t ver);
void emcom_xengine_mpflow_clear_blocked(int uid, uint8_t ver);
bool emcom_xengine_mpflow_blocked(int uid, const char *name, uint8_t ver);
int16_t emcom_xengine_mpflow_connum(int uid, const char *name);
void emcom_xengine_mpflow_report(const void *data, int len);
void emcom_xengine_mpflow_show(void);
bool emcom_xengine_mpflow_errlink(int reason, struct emcom_xengine_mpflow_stat *node);
bool emcom_xengine_mpflow_retrans(int reason, struct emcom_xengine_mpflow_stat *node);
int8_t emcom_xengine_mpflow_checkstatus(const struct sock *sk, int *reason, int state, struct emcom_xengine_mpflow_stat *node);
void emcom_xengine_mpflow_fallback(struct sock *sk, int reason, int state);
void emcom_xengine_mpflow_ai_reset_loc_intf(const uint8_t *data, uint16_t len);
int8_t emcom_xengine_mpflow_ai_finduid(uid_t uid);
bool emcom_xengine_mpflow_ai_start(uid_t uid);
void emcom_xengine_mpflow_ai_stop(const char *pdata, uint16_t len);
void emcom_xengine_mpflow_ai_bind2device(struct sock *sk, struct sockaddr *uaddr);
void emcom_xengine_mpflow_ai_ip_config(const char *data, uint16_t len);
void emcom_xengine_mpflow_ai_init_bind_config(const char *data, uint16_t len);
void emcom_xengine_mpflow_ai_iface_cfg(const char *data, uint16_t len);
void emcom_xengine_mpflow_ai_close_all_flow(const uint8_t *data, uint16_t len);
uint8_t emcom_xengine_mpflow_ip_hash(__be32 addr);
void emcom_xengine_mpflow_reset(struct tcp_sock *tp);
void emcom_xengine_mpflow_register_nf_hook(uint8_t ver);
void emcom_xengine_mpflow_unregister_nf_hook(uint8_t ver);
void emcom_xengine_mpflow_ai_change_burst_ratio(const uint8_t *data, uint16_t len);

#endif
