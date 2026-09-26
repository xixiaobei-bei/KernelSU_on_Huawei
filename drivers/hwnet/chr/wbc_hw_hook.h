

#ifndef _WEB_HW_HOOK
#define _WEB_HW_HOOK

#include <linux/in6.h>
#include <linux/skbuff.h>

#define INIT_NUM 0
#define HASH_MAX 256
#define HASH_MAX_16_BIT 65535
#define HASH_MAX_24_BIT 16777216
#define HTTP_ACK_HEAD_NUM 18
#define HTTP_ACK_FROM_START 9
#define CHECK_TIME (15 * HZ)
#define EXPIRE_TIME (30 * HZ)
#define DELETE_TIME (30 * HZ)
#define REPORT_TIME (15 * 60 * HZ)
#define FORBID_TIME (60 * 60 * HZ)
#define RAT_TECH_CHANGE_NOTIFY_FORBIDDEN_TIME (60 * HZ)
#define HTTP_PORT 80
#define CHR_WEB_STAT_EVENT 9
#define RTT_THRESHOLD 600
#define RTT_THRESHOLD_L1 0
#define RTT_THRESHOLD_L2 50
#define RTT_THRESHOLD_L3 100
#define RTT_THRESHOLD_L4 200
#define RTT_THRESHOLD_L5 600

#define WEB_DELAY_THRESHOLD 3000
#define DELAY_THRESHOLD_L1 0
#define DELAY_THRESHOLD_L2 300
#define DELAY_THRESHOLD_L3 400
#define DELAY_THRESHOLD_L4 500
#define DELAY_THRESHOLD_L5 700
#define DELAY_THRESHOLD_L6 3000

#define HTTP_RESPONSE_NORMAL '2'
#define HTTP_RESPONSE_REDIRECT '3'
#define HTTP_RESPONSE_CLIENT_ERROR '4'
#define HTTP_RESPONSE_SERVER_ERROR '5'

#define SEQ_MAX_RTT (REPORT_TIME * 2 * HZ)

#define HTTP_RSP_LEN 16
#define HTTP_STR_LEN 4
#define GET_STR_LEN 3
#define POST_STR_LEN 4

#define MULTIPLE (1000 / HZ)
#ifndef CONFIG_CHR_MTK_NETLINK
#define WEB_DS_NET "rmnet"
#else
#define WEB_DS_NET "ccmni"
#endif

#define WEB_DS_NET_LEN 5

#define SYN_NO_ACK_REPORT_TIME (60 * HZ)
#define WEB_NO_ACK_REPORT_TIME (60 * HZ)
#define WEB_DELAY_REPORT_TIME (60 * HZ)
#define WEB_FAIL_REPORT_TIME (60 * HZ)
#define TCP_RTT_LARGE_REPORT_TIME (60 * HZ)
#define SYN_NO_ACK_MAX 3
#define WEB_NO_ACK_MAX 3
#define WEB_FAIL_MAX 3
#define WEB_DELAY_MAX 3
#define TCP_RTT_LARGE_MAX 3
#define IPV4ADDR_MASK 0x0000FFFF
#define RMNET_INTERFACE 0
#define WLAN_INTERFACE 1
#define IPV6_INTERFACE 2
#define IPV4_NETWORK 0
#define IPV6_NETWORK 1
#define RNT_STAT_SIZE 4
#define MAX_JIFFIES 0xFFFFFFFFFFFFFFFF
#define MAX_VALID_U16 65534
#define NS_CONVERT_TO_MS 1000000
#define FILTER_TIME_LIMIT (HZ / 4)
#define ALPHA_FILTER_PARA 8
#define CODE_LEN 3
#define DIF_JIF_LOW (-2 * HZ)
#define DIF_JIF_HIGH (2 * HZ)
#define IPV6_ADD_P1 0
#define IPV6_ADD_P2 1
#define IPV6_ADD_P3 2
#define IPV6_ADD_P4 3

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
#define TCP_MAX_REPORT_TIME (30 * 60 * HZ)
#define SMALL_WIN_STAMP_RATIO 10
#endif
#define MAX_VALID_NS 65534000000L
#define RIL_RADIO_TECHNOLOGY_LTE 13
#define RIL_RADIO_TECHNOLOGY_LTE_CA 19
#define RIL_RADIO_TECHNOLOGY_NR 20
#define RADIO_TECHNOLOGY_LTE_EN_DC 25
#define CHR_DATA_REG_TYPE_LTE 0
#define CHR_DATA_REG_TYPE_ENDC 1
#define CHR_DATA_REG_TYPE_NR 2
#define MIN_APP_UID 10000
#define APP_UID_INVALID (-1)

#define CHR_MAX_REPORT_APP_COUNT 10
#define DATA_REG_TECH_TAG 20
#define GET_AP_REPORT_TAG 21
#define SET_FOREGROUND_APP_UID 10

/*
 * enum - response and report type.
 * response and report type when report
 */
enum {
	WEB_STAT = 0,
	WEB_NO_ACK,
	WEB_FAIL,
	WEB_DELAY,
	TCP_RTT_LARGE,
	DNS_FAIL,
	DNS_BIG_LATENCY,
	SYN_NO_ACK,
	SYN_SUCC,
	WEB_SUCC,
	WEB_STAT_V6 = 20,
	WEB_NO_ACK_V6,
	WEB_FAIL_V6,
	WEB_DELAY_V6,
	TCP_RTT_LARGE_V6,
	DNS_FAIL_V6,
	DNS_BIG_LATENCY_V6,
	SYN_NO_ACK_V6,
	SYN_SUCC_V6,
	WEB_SUCC_V6,
	UN_KNOW,
};

enum {
	HTTP_GET = 0,
	HTTP_SYN,
	OTHER,
};

enum {
	IS_UNUSE = 0,
	IS_USE,
};

int web_chr_init(void);
void web_chr_exit(void);
void web_proc(void);
u32 http_timer(void);

/*
 * struct struct_name - http_stream.
 * Hook package for data storage structures.
 */
struct http_stream {
	u32 src_addr;
	u32 dst_addr;
	u16 tcp_port;
	u8 is_valid;
	u8 type;
	unsigned long time_stamp;
	unsigned long get_time_stamp;
	u32 uid;
	u32 resp_code;
	u8 interface;
	unsigned long ack_time_stamp;
	u8 proto;
	struct in6_addr src_v6_addr;
	struct in6_addr dst_v6_addr;
};

struct report_app_stat {
	u32 tcp_rtt;
	u32 web_delay;
	u32 succ_num;
	u32 fail_num;
	u32 no_ack_num;
	u32 total_num;
	u32 tcp_total_num;
	u32 tcp_succ_num;
	u32 delay_num_l1;
	u32 delay_num_l2;
	u32 delay_num_l3;
	u32 delay_num_l4;
	u32 delay_num_l5;
	u32 delay_num_l6;
	u32 rtt_num_l1;
	u32 rtt_num_l2;
	u32 rtt_num_l3;
	u32 rtt_num_l4;
	u32 rtt_num_l5;
};

struct ipv6_addr_info {
	u32 reason;
	u32 router_time;
	u32 managed;
	u32 other;
	u32 prefix_len;
	u32 prefer_time;
	u32 valid_time;
	u32 autoconf;
	u32 dns_option;
};

/*
 * struct struct_name - http_return.
 * Web page statistics structure.
 */
struct http_return {
	u32 tcp_rtt;
	u32 web_delay;
	u32 succ_num;
	u32 fail_num;
	u32 no_ack_num;
	u32 total_num;
	u32 report_type;
	u32 tcp_total_num;
	u32 delay_num_l1;
	u32 delay_num_l2;
	u32 delay_num_l3;
	u32 delay_num_l4;
	u32 delay_num_l5;
	u32 delay_num_l6;
	u32 rtt_num_l1;
	u32 rtt_num_l2;
	u32 rtt_num_l3;
	u32 rtt_num_l4;
	u32 rtt_num_l5;
	u32 tcp_succ_num;
	u32 uid;
	u32 http_resp;
	struct report_app_stat report_app_stat_list[CHR_MAX_REPORT_APP_COUNT];
	u32 highest_tcp_rtt;
	u32 lowest_tcp_rtt;
	u32 last_tcp_rtt;
	u32 highest_web_delay;
	u32 lowest_web_delay;
	u32 last_web_delay;
	u8 proto;
	u32 server_addr;
	struct in6_addr server_v6_addr;
	u32 rtt_abn_server_addr;
	struct in6_addr rtt_abn_srv_v6_addr;
	u32 vod_avg_speed;
	u32 vod_freez_num;
	u32 vod_time;
	u32 uvod_avg_speed;
	u32 uvod_freez_num;
	u32 uvod_time;
	u32 tcp_handshake_delay;
	u32 http_get_delay;
	u32 http_send_get_num;
	u32 exception_cnt;
	u32 data_direct;
	u32 transport_delay;
	u32 ip_delay;
	u32 hmac_delay;
	u32 driver_delay;
	u32 android_uid;
	u32 sock_uid;
	u32 sock_dura;
	u32 cur_win;
	u32 win_cnt;
	u32 free_space;
	u32 mime_type;
	u32 tcp_srtt;
	u32 data_reg_tech;
	struct ipv6_addr_info v6;
};

/*
 * struct struct_name - rtt_from_stack.
 * this is temporarily stores the RTT.
 */
struct rtt_from_stack {
	u32 tcp_rtt;
	u32 is_valid;
	u32 uid;
	u32 rtt_dst_addr;
	struct in6_addr rtt_dst_v6_addr;
};

struct delay_chr_report {
	u32 exception_cnt;
	u32 data_direct;
	u32 transport_delay;
	u32 ip_delay;
	u32 hmac_delay;
	u32 driver_delay;
	u32 android_uid;
};

struct chr_key_val {
	atomic64_t tcp_buf_time;
	atomic64_t udp_buf_time;

	volatile unsigned long tcp_last;
	volatile unsigned long udp_last;
};

void ipv6_spin_lock_init(void);
void try_upload_v6_info(void);
void chr_router_discovery(struct sk_buff *skb);
void wifi_disconnect_report(void);
int set_report_app_uid(int index, u32 uid);
extern void wifi_kernel_delay_report(struct delay_chr_report *delay_chr);
bool is_notify_chr_event(int old_data_reg_tech, int new_data_reg_tech);
void notify_chr_thread_to_update_rtt(u32 seq_rtt_us, struct sock *sk,
	u8 data_net_flag);
uid_t get_uid_from_sock(struct sock *sk);
extern int g_app_top_uid;

#endif /* WEB_HW_HOOK */
