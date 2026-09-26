

#ifndef __HW_WIFI_SNIFFER_H
#define __HW_WIFI_SNIFFER_H

#include <linux/vmalloc.h>
#include <linux/uaccess.h>
#include <linux/jump_label.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/statfs.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <linux/errno.h>
#include <linux/kmod.h>
#include <asm/string.h>
#include <log/log_usertype.h>

#define PCAP_FILE_NUM 1
#define PCAP_FILE_NUM_MAX PCAP_FILE_NUM
#define PCAP_FILE_NUM_MIN 1
#define PCAP_FILE_LEN (30 * 1024 * 1024)
#define PCAP_FILE_LEN_MIN (500 * 1024)
#define PCAP_FILE_LEN_MAX (300 * 1024 * 1024)

#define PCAP_PKT_HDR_MAGIC 0xa1b2c3d4
#define PCAP_PKT_HDR_VER_MAJ 0x2
#define PCAP_PKT_HDR_VER_MIN 0x4
#define PCAP_PKT_HDR_THISZONE 0x0
#define PCAP_PKT_HDR_SIGFIGS 0x0
#define PCAP_PKT_HDR_SNAPLEN 0xffff
#define PCAP_PKT_HDR_LINKTYPE_802_11 0x69
#define PCAP_PKT_HDR_LINKTYPE_802_11_RADIOTAP 127

#define IEEE80211_MAC_HDR_LEN 26
#define TRIM_DATA_PAYLOAD_LEN 2000
#define IEEE80211_FRAME_TYPE_DATA 0x02
#define IEEE80211_FRAME_TYPE_MGMT 0x00
#define IEEE80211_FRAME_SUBTYPE_QOS 0x08
#define IEEE80211_FRAME_SUBTYPE_DISASSOC 0x0a

#define CMD_BUFF_MAGIC_NUM 0xaabbccdd
#define CMD_BUF_MAX_PARAM 12

#define MAC_ADDR_LEN 6
#define SNIFFER_START (static_key_enabled(&wifi_sniffer_enable))
#define SNIFFER_MAX_FILE_LEN (g_configs.max_file_len)
#define SNIFFER_MAX_FILE_NUM (g_configs.max_file_num)
#define SNIFFER_MAX_PACKET_LEN (g_configs.data_packet_max_len)
#define sniffer_enough_to_save(packet_len) \
	((g_configs.curpos[g_configs.cur_file] + (packet_len)) <= \
	g_configs.max_file_len)
#define sniffer_file_buffer(i) (g_configs.buffer[i])
#define sniffer_file_pos(i) (g_configs.curpos[i])
#define SNIFFER_CUR_FILE_BUFFER sniffer_file_buffer(g_configs.cur_file)
#define SNIFFER_CUR_FILE_POS sniffer_file_pos(g_configs.cur_file)
#define sniffer_file_gmttime(i)        (g_configs.gmttime[i])
#define sniffer_file_microtime(i)      (g_configs.microtime[i])
#define sniffer_file_packetcount(i)    (g_configs.packetcount[i])

#define SNIFFER_IS_CUR_FILE_EMPTY (SNIFFER_CUR_FILE_POS == 0)
#define SNIFFER_IS_CUR_FULL (g_configs.file_full[g_configs.cur_file])
#define SNIFFER_CUR_FILE_BUFF_POS (g_configs.buffer[g_configs.cur_file] + \
	g_configs.curpos[g_configs.cur_file])
#define SNIFFER_RESET_CUR_POS     g_configs.curpos[g_configs.cur_file] = 0;

#define FILE_READ 0
#define FILE_WRITE 1

#define DIRECTION_RX 0
#define DIRECTION_TX 1
#define DIRECTION_MONITOR_MODE 4

#define CMD_STR_LEN 52
#define CMD_MODE_INDEX 0
#define CMD_FREQ_INDEX 1
#define CMD_MAC_INDEX  2
#define CMD_ENABLE_MONITOR_MODE 2
#define CMD_ENABLE_MONITOR_MODE_PARANUM 8
#define CMD_CMD_OFFSET 1
#define TRIM_NEEDED 1
#define TRIM_NOT_NEEDED 0

#define EXPAND_NONE 0
#define EXPAND_LOW  1
#define EXPAND_HIGH 2
#define EXPAND_11B  3

#define PORT_LIST_SIZE 10
#define ADDR_LENGTH 16
#define FLOW_LABEL_SIZE 3
#define VEND_ID_NUM 3

/* frame control structure */
typedef struct {
	unsigned short bit_protocol_version : 2, /* protocol version */
	bit_type : 2, /* frame type */
	bit_sub_type : 4, /* subtype */
	bit_to_ds : 1, /* to DS */
	bit_from_ds : 1, /* from DS */
	bit_more_frag : 1, /* more fragment */
	bit_retry : 1, /* retransmit frame */
	bit_power_mgmt : 1, /* power management */
	bit_more_data : 1, /* more data flag */
	bit_protected_frame : 1, /* encrypt flag */
	bit_order : 1; /* order bit */
} mac_header_control_stru;

/* qos frame 2+2+ 6*3 + 2 +1 +1 = 26 */
typedef struct __attribute__((__packed__)) {
	mac_header_control_stru         st_frame_control;
	unsigned short                  bit_duration_value		: 15, /* duration/id */
					bit_duration_flag		: 1;
	unsigned char                   auc_address1[MAC_ADDR_LEN];
	unsigned char                   auc_address2[MAC_ADDR_LEN];
	unsigned char                   auc_address3[MAC_ADDR_LEN];
	unsigned short                  bit_frag_num			: 4, /* sequence control */
					bit_seq_num			: 12;
	unsigned char                   bit_qc_tid			: 4,
					bit_qc_eosp			: 1,
					bit_qc_ack_polocy		: 2,
					bit_qc_amsdu			: 1;
	union {
		unsigned char           bit_qc_txop_limit;
		unsigned char           bit_qc_ps_buf_state_resv        : 1,
					bit_qc_ps_buf_state_inducated   : 1,
					bit_qc_hi_priority_buf_ac       : 2,
					bit_qc_qosap_buf_load           : 4;
	} qos_control;
} mac_ieee80211_qos_stru; // __OAL_DECLARE_PACKED;

#define CMD_EXECUTE_TYPE_DRIVER 1
#define CMD_EXECUTE_TYPE_SHELL  2

#define CHIPTYPE_1102 2
#define CHIPTYPE_1103 5
#define CHIPTYPE_1105 6

typedef struct {
	char   cmds_str[CMD_STR_LEN];
	short  delay_time; // ms
	short  execute_type;
}proc_cmd_execution_struct;

typedef struct {
	unsigned int cur_file;
	unsigned int data_packet_max_len;
	unsigned int max_file_num;
	unsigned int max_file_len;

	int  file_full[PCAP_FILE_NUM];
	char *buffer[PCAP_FILE_NUM];
	unsigned int curpos[PCAP_FILE_NUM];
	unsigned int gmttime[PCAP_FILE_NUM];	 /* ms, between now and 1970-01-01 00:00:00 */
	unsigned int microtime[PCAP_FILE_NUM];	 /* us, between now and 1970-01-01 00:00:00 */
	unsigned int packetcount[PCAP_FILE_NUM]; /* us, between now and 1970-01-01 00:00:00 */
} proc_file_config_struct;

typedef enum {
	CMD_BUFF_CMDID_TOP = 0xaa000000,
	/* 1 parameter: 0 stop, 1 start */
	CMD_BUFF_CMDID_SNIFFER_ENABLE = 0xaa010001,
	/* 1 parameter: 0 Hybrid mode (my own), 1 Monitor mode (all) */
	CMD_BUFF_CMDID_SNIFFER_MODE = 0xaa020001,
	/* 1 parameter: channel number */
	CMD_BUFF_CMDID_CHANNDEL = 0xaa030001,
	/* 1 parameter: 20/40/80/160 */
	CMD_BUFF_CMDID_BANDWIDTH = 0xaa040001,
	/* 1 parameter: a/b/g/n/ac/ax(1~6 respectively) */
	CMD_BUFF_CMDID_PROTOTYPE = 0xaa050001,
	/* 1 parameter: file number */
	CMD_BUFF_CMDID_CURFILELEN = 0xaa060001,
	/* 2 parameters: file numbers(1~5); file length */
	CMD_BUFF_CMDID_FILESPEC = 0xaa070001,
	/* 1 parameter: 802.11 payload length after trimming */
	CMD_BUFF_CMDID_DATAPAYLOAD = 0xaa080001,
	/* 1 parameter: AID Number */
	CMD_BUFF_CMDID_MUMIMO_AID =  0xaa090001,
	/* 1 parameter: ENABLE WIFI6 */
	CMD_BUFF_CMDID_WIFI6_ENABLE =  0xaa0a0001,
	CMD_BUFF_CMDID_BUTTOM = 0xaaff0000,
} cmd_buff_cmdid_enum;

typedef enum {
	CMD_BUFF_RDWR_READ = 0x00000001,
	CMD_BUFF_RDWR_WRITE = 0x00000002,
} cmd_buff_read_write_enum;

typedef struct {
	int i_magic_num; /* fixed : CMD_BUFF_MAGIC_NUM */
	unsigned int i_cmd_id; /* cmd_buff_cmdid_enum (0xaa000000 ~ 0xaaff0000) */
	int i_rd_wr; /* 1 Read, 2 Write */
	/* parameter numbers, with max value CMD_BUF_MAX_PARAM */
	int i_para_num;
} ctl_cmd_hdr_stru;

typedef struct {
	ctl_cmd_hdr_stru ctl_cmd_hdr;
	int i_paras[CMD_BUF_MAX_PARAM];
} ctl_cmd_stru;

#pragma pack(1)
typedef struct {
	unsigned int magic;
	unsigned short version_major;
	unsigned short version_minor;
	unsigned int thiszone;
	unsigned int sigfigs;
	unsigned int snaplen;
	unsigned int linktype;
} oam_pcap_filehdr_stru;
#pragma pack()

/* 16 Bytes PCAP frame header's timestamp */
typedef struct {
	unsigned int gmt_time; /* ms, between now and 1970-01-01 00:00:00 */
	unsigned int micro_time; /* us, between now and 1970-01-01 00:00:00 */
} oam_pcap_time_stru;

/* PCAP frame header (16 Bytes) */
#pragma pack(1)
typedef struct {
	oam_pcap_time_stru time; /* PCAP Frame Header's timestamp */
	unsigned int caplen; /* length of (Frame Header + Frame Body) */
	unsigned int len; /* packet's actual length, len == caplen in common */
} oam_pcap_pkthdr_stru;
#pragma pack()

typedef struct {
	unsigned char dsap;
	unsigned char ssap;
	unsigned char cmd;
	unsigned char vend_id[VEND_ID_NUM];
	/*
	 * 0x0800 for IPv4, 0x0806 for ARP
	 * 0x888e for EAP (802.1X Authentication)
	 */
	unsigned short protocol;
} __attribute__((packed)) ieee802_2_llc_hdr_stru;

typedef struct {
	unsigned char version_ihl;
	unsigned char temp_tos_1;
	unsigned short temp_tot_len_1;
	unsigned short temp_id_1;
	unsigned short temp_frag_off_1;
	unsigned char temp_ttl_1;
	unsigned char protocol; /* 0x06 for TCP, 0x11 for UDP */
	unsigned short temp_check_1;
	unsigned int temp_saddr_1;
	unsigned int temp_daddr_1;
} __attribute__((packed)) eth_ipv4_hdr_stru;

typedef struct {
	unsigned char version;
	unsigned short traffic_class;
	unsigned char flow_label[FLOW_LABEL_SIZE];
	unsigned short payload_len;
	unsigned char next_hdr;
	unsigned char hop_limit;
	unsigned char saddr[ADDR_LENGTH];
	unsigned char daddr[ADDR_LENGTH];
} __attribute__((packed)) eth_ipv6_hdr_stru;

typedef struct {
	/* 0x0035 or others for DNS, 0x0043 or 0x0044 for DHCP */
	unsigned short sport;
	/* others or 0x0035 for DNS, 0x0044 or 0x0043 for DHCP */
	unsigned short dport;
	unsigned short len;
	unsigned short temp_check_1;
} __attribute__((packed)) eth_ipv4_udp_hdr_stru;

typedef struct {
	unsigned short s_port;
	unsigned short d_port;
} st_portpair;

typedef struct {
	unsigned short llc_type;
	unsigned char protocol_type;
	unsigned char port_count;
	st_portpair port_list[PORT_LIST_SIZE];
} st_packet_filter;

#define PORT_ANY 0xffff
#define PROTOCOL_ANY 0xff
#define get_llc_type(p_llc_hdr) \
	(((ieee802_2_llc_hdr_stru *)(p_llc_hdr))->protocol)
#define get_ipv4_protocol_type(p_llc_hdr) \
	(((eth_ipv4_hdr_stru *)((p_llc_hdr) + ETH_PKT_LLC_HDR_LEN))->protocol)

/*
 * thought we use UDP header structure here.
 * For TCP header, the first two short data are sport and dport too.
 */
#define get_ipv4_src_port(p_llc_hdr) \
	(((eth_ipv4_udp_hdr_stru *)((p_llc_hdr) + ETH_PKT_LLC_HDR_LEN + \
	ETH_PKT_IPV4_HDR_LEN))->sport)
#define get_ipv4_dst_port(p_llc_hdr) \
	(((eth_ipv4_udp_hdr_stru *)((p_llc_hdr) + ETH_PKT_LLC_HDR_LEN + \
	ETH_PKT_IPV4_HDR_LEN))->dport)

#define get_filter_sport(loop, port_loop) \
	(packet_filter[loop].port_list[port_loop].s_port)
#define get_filter_dport(loop, port_loop) \
	(packet_filter[loop].port_list[port_loop].d_port)

#define ETH_PKT_TYPE_IPV4 (ntohs(0x0800))
#define ETH_PKT_TYPE_IPV4_ICMP 0x01
#define ETH_PKT_TYPE_IPV4_TCP 0x06
#define ETH_PKT_TYPE_IPV4_UDP 0x11
#define ETH_PKT_TYPE_IPV4_UDP_DNS (ntohs(0x0035))
#define ETH_PKT_TYPE_IPV4_UDP_DHCP_A (ntohs(0x0043))
#define ETH_PKT_TYPE_IPV4_UDP_DHCP_B (ntohs(0x0044))

#define ETH_PKT_TYPE_IPV6 (ntohs(0x86dd))

#define ETH_PKT_TYPE_ARP (ntohs(0x0806))
#define ETH_PKT_TYPE_EAP (ntohs(0x888e))
#define ETH_PKT_LLC_HDR_LEN sizeof(ieee802_2_llc_hdr_stru)
#define ETH_PKT_IPV4_HDR_LEN sizeof(eth_ipv4_hdr_stru)

extern int proc_handle_commands(int file_index, char __user *buffer);

int proc_sniffer_read_01(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos);
int proc_sniffer_read_02(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos);
int proc_sniffer_read_03(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos);
int proc_sniffer_read_04(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos);
int proc_sniffer_read_05(struct ctl_table *ctl, int write, void __user *buffer,
	size_t *lenp, loff_t *ppos);

int proc_sniffer_write_file(const char *header_buff, unsigned int header_len,
	const char *frame_buff, unsigned int frame_len, int flag_rx_tx);

typedef unsigned int  (*drive_wal_hipriv_cmd_func)(signed char *p_cmd);
int proc_set_hipriv_func(void *p_func); // drive_wal_hipriv_cmd_func

#endif /* end __HW_WIFI_SNIFFER_H */
