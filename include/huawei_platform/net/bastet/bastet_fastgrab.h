/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015. All rights reserved.
 * Description: process the event for hongbao fastgrab in the kernel
 *
 * Author: zhuweichen@huawei.com
 * Create: 2015-09-30
 */

#ifndef __BST_FAST_GRAB_H__
#define __BST_FAST_GRAB_H__

#include <net/sock.h>
#ifdef CONFIG_CHR_NETLINK_MODULE
#include <hwnet/booster/netlink_handle.h>
#endif

#define BST_FG_MAX_APP_NUMBER	(0x01U)
#define BST_FG_MAX_KW_LEN	(0xFFU) /* Current Max keyword lenght is 16 */
#define BST_FG_MAX_KW_NUMBER	(0x04U)
#define BST_FG_MAX_PID_NUMBER	(0x200U)
#define BST_FG_MAX_US_COPY_NUM	(4096)
#define BST_FG_ROLE_SNDER	(0x01U)
#define BST_FG_ROLE_RCVER	(0x02U)

#define BST_FG_IDX_WECHAT	(0x00U)
#define BST_FG_IDX_ALIPAY	(0x01U)
#define BST_FG_IDX_GTGJ	(0x02U)
#define BST_FG_IDX_INVALID_APP	(0xFFU)
#define BST_FG_FLAG_APP_INFO	(0x01U)

#define BST_FG_FLAG_WECHAT_PUSH	(0x00U)
#define BST_FG_FLAG_WECHAT_RCVD	(0x01U)
#define BST_FG_FLAG_WECHAT_GET	(0x02U)
#define BST_FG_FLAG_WECHAT_PUSH_NEW (0x03U)


#define BST_FG_INVALID_UID	(0)
#define BST_FG_INVALID_INET_ADDR	(0)
#define BST_FG_FLAG_WECHAT_VALID \
	((BST_FG_FLAG_APP_INFO) |\
	(BST_FG_FLAG_WECHAT_RCVD) |\
	(BST_FG_FLAG_WECHAT_GET) |\
	(BST_FG_FLAG_WECHAT_VALID))

#define BST_FG_MAX_CUSTOM_APP   (10)
#define BST_FG_TCP_BITMAP   (0x01)
#define BST_FG_UDP_BITMAP   (0x02)

#define BST_FG_ACC_BITMAP           (0x01)
#define BST_FG_CONGESTION_BITMAP   (0x02)

#define bst_fg_set_flag(var, idx)	((var) |= (0x01<<(idx)))
#define bst_fg_clr_flag(var)	((var)  = 0U)
#define bst_fg_is_uid_valid(uid) ((uid) > 0)
#define bst_fg_is_flag_valid(var, flag)	((var) == (flag))
#define bst_fg_is_app_idx_valid(idx)	((idx) < (BST_FG_MAX_APP_NUMBER))
#define bst_fg_is_kwd_idx_valid(idx)	((idx) < (BST_FG_MAX_KW_NUMBER))
#define bst_fg_get_app_ins(idx)	(&g_fast_grab_app_info[idx])
#define bst_fg_is_cared_sk_state(state) \
	((TCP_CLOSING == (state)) || (TCP_ESTABLISHED == (state)) || \
	(TCP_FIN_WAIT1 == (state)) || (TCP_CLOSE_WAIT == (state)))

#define bst_fg_init_sock_spec(pSock) \
	{ \
		(pSock)->fg_Spec = BST_FG_NO_SPEC; \
		(pSock)->fg_Step = BST_FG_INIT_STEP; \
	}

#define bst_fg_set_sock_special(sk, spec) \
	{ \
		bh_lock_sock(sk); \
		(sk)->fg_Spec = (spec); \
		bh_unlock_sock(sk); \
	}

#define bst_fg_set_acc_state(sk, value) \
	{ \
		(sk)->acc_state = (value); \
	}

#define bst_fg_hook_dl_stub(sk, skb, hrd_len) \
	do { \
		bastet_mark_hb_reply(sk, skb, hrd_len); \
		{ \
			if (BST_FG_NO_SPEC != (sk)->fg_Spec) { \
				if ((skb)->len > (hrd_len)) { \
					bst_fg_hook_packet((sk), \
					(skb)->data+(hrd_len), \
					(skb)->len-(hrd_len), \
					BST_FG_ROLE_RCVER); \
				} \
			} \
		} \
	} while (0)

#define bst_min(x, y) ((x) < (y) ? (x) : (y))
#define bst_max(x, y) ((x) > (y) ? (x) : (y))

#define bst_fg_set_discard_timer(sk, value) \
	{ \
		(sk)->discard_duration = (value); \
	}

#ifdef CONFIG_HW_DPIMARK_MODULE
#define bst_fg_set_app_type(sk, value) \
	{ \
		(sk)->__sk_common.skc_hwdpi_mark |= (value << 8); \
	}

#define bst_fg_init_app_type(sk) \
	{ \
		(sk)->__sk_common.skc_hwdpi_mark &= (0xFFFF00FF); \
	}
#endif

typedef void(*BST_FG_PKT_PROC_T)(struct sock *, uint8_t*, uint32_t uint32_t);

enum fastgrab_cmd {
	CMD_LOAD_KEYWORDS = 0,
	CMD_UPDATE_UID,
	CMD_UPDATE_TID,
	CMD_UPDATE_DSCP,
	CMD_UPDATE_ACC_UID,
	CMD_UPDATE_CUSTOM,
};

enum bastet_fg_sock_prop {
	BST_FG_NO_SPEC = 0,
	BST_FG_WECHAT_PUSH,
	BST_FG_WECHAT_WAIT_HONGBAO,
	BST_FG_WECHAT_HONGBAO,
	BST_FG_SOCK_BUTT,
};

enum bastet_fg_sock_step {
	BST_FG_INIT_STEP = 0,
	BST_FG_STEP_BUTT,
};

enum bastet_fg_acc_state {
	BST_FG_ACC_NORMAL = 0,
	BST_FG_ACC_HIGH,
};

/**
 * struct bst_fg_packet_len_stru - define the packet length.
 * @us_copy:The length should be copied from Packet.
 * @us_min:The min length of this key packet.
 * @us_max:The max length of this key packet.
 *
 * used to record the packet length information.
 */
struct bst_fg_packet_len_stru {
	uint16_t us_copy;
	uint16_t us_min;
	uint16_t us_max;
};

/**
 * struct bst_fg_key_words_stru - key words stru.
 * @us_ofst:The offset address of the keyworks.
 * @us_tot_len:The totle length of this keyworks.
 * @auc_data[0]:The content.
 *
 * used to record the key words structer informations.
 */
struct bst_fg_key_words_stru {
	uint16_t us_ofst;
	uint16_t us_tot_len;
	uint8_t auc_data[0];
};

/**
 * struct bst_fg_keyword_stru - key words context in the packet.
 * @us_role:The comm role of this packet.
 * @us_index:The Keyword Index Name, co. with Kernel.
 * @bst_fg_packet_len_stru:Packet length message.
 * @bst_fg_key_words_stru:The keywords of this one.
 *
 * used to record the key words context informations within packets.
 */
struct bst_fg_keyword_stru {
	uint16_t us_role;
	uint16_t us_index;
	struct bst_fg_packet_len_stru st_len_porp;
	struct bst_fg_key_words_stru st_key_word;
};

/**
 * struct bst_fg_app_info_stru - record app info.
 * @l_uid:The uid of Wechat Application.
 * @us_index:The App Index Name, co. with Kernel.
 * @*pst_kws[BST_FG_MAX_KW_NUMBER]:Keywords arry.
 * @st_locker:The Guard Lock of this unit.
 * @pf_proc:The Packet Process Function.
 *
 * used to record the app key information and keywords array.
 */
struct bst_fg_app_info_stru {
	uid_t l_uid;
	uint16_t us_index;
	struct bst_fg_keyword_stru *pst_kws[BST_FG_MAX_KW_NUMBER];
	spinlock_t st_locker;
	BST_FG_PKT_PROC_T pf_proc;
};

/**
 * struct bst_fg_kws_comm_stru - comm info of keywords
 * @us_index:The App Index Name, co. with Kernel.
 * @us_key_length:The Number of key words.
 * @auc_key_words[0]:The Arry of keywords.
 *
 * used to record the comm infomation of keywords.
 */
struct bst_fg_kws_comm_stru {
	uint16_t us_index;
	uint16_t us_key_length;
	uint8_t auc_key_words[0];
};

/**
 * struct bst_fg_uid_comm_stru - uid comm info.
 * @us_index:The App Index Name, co. with Kernel.
 * @l_uid:The uid of Wechat Application.
 *
 * used to record the uid common informations such as
 * index and its id.
 */
struct bst_fg_uid_comm_stru {
	uint16_t us_index;
	uid_t l_uid;
};

/**
 * struct bst_fg_tid_comm_stru - tid comm info.
 * @us_index:The App Index Name, co. with Kernel.
 * @us_totle:The numberof tids.
 * @l_tids[0]:Tid List
 *
 * used to record the tid common informations such as
 * index and its length
 */
struct bst_fg_tid_comm_stru {
	uint16_t us_index;
	int16_t us_totle;
	int32_t l_tids[0];
};

/**
 * struct bst_fg_custom_info - bastet custom fg information.
 * @l_uid:The uid of Application.
 * @uc_protocol_bitmap:BIT0:tcp; BIT1:udp
 * @uc_tcp_retran_discard:Bit7 is switch flag, bit0~bit6 is value.
 * @uc_discard_timer:discard timer value.
 * @uc_app_type:app type
 *
 * used to record the tid bastet app common informations.
 */
struct bst_fg_custom_info {
	uid_t l_uid;
	uint8_t uc_protocol_bitmap;
	uint8_t uc_tcp_retran_discard;
	uint8_t uc_discard_timer;
	uint8_t uc_app_type;
};

/**
 * struct bst_fg_custom_stru - bastet custom fg information.
 * @uc_hongbao_duration_time:grab hongbao time, unit is second.
 * @rsv[2]: reserve segment.
 * @uc_app_num:app number.
 * @ast_custom_info[BST_FG_MAX_CUSTOM_APP]:custom app infomations.
 *
 * used to record the several custom apps infomations
 */
struct bst_fg_custom_stru {
	uint8_t uc_hongbao_duration_time; /* unit: second*/
	uint8_t rsv[2];
	uint8_t uc_app_num;
	struct bst_fg_custom_info ast_custom_info[BST_FG_MAX_CUSTOM_APP];
};

/**
 * struct bst_fg_hongbao_stru - hongbao information.
 * @uc_congestion_proc_flag: judge if the congestion happen.
 * @uc_duration_time: hongbao duration time. unit is second.
 * @ul_start_time:hongbao start time,
 *
 * used to record the hongbao congestion info and send/recv info.
 */
struct bst_fg_hongbao_stru {
	uint8_t uc_congestion_proc_flag;
	uint8_t uc_duration_time;
	long ul_start_time;
};

/**
 * struct fastgrab_info - fastgrab information.
 * @cmd: different type of cmd such as Update/Load.
 * @len: command length.
 * @p_data[0]:the content of command.
 *
 * used to record the fastgrab command info.
 */
struct fastgrab_info {
	enum fastgrab_cmd cmd;
	uint16_t len;
	uint8_t p_data[0];
};

/**
 * struct fastgrab_info - fastgrab information.
 * @cmd: different type of cmd such as Update/Load.
 * @len: command length.
 * @p_data[0]:the content of command.
 *
 * used to record the fastgrab command info.
 */
struct bastet_fg_sock_stru {
	enum bastet_fg_sock_prop en_spec;
	enum bastet_fg_sock_step en_step;
};

void bst_fg_init(void);
void bst_fg_io_ctrl(unsigned long arg);
void bst_fg_check_sock_uid(struct sock *pst_sock, int state);
uint8_t bst_fg_hook_packet(struct sock *pst_sock, uint8_t *p_data,
		uint32_t ul_length, uint32_t ul_role);
#ifdef CONFIG_HUAWEI_XENGINE
bool bst_fg_hook_ul_stub(struct sock *pst_sock, struct msghdr *msg);
#else
void bst_fg_hook_ul_stub(struct sock *pst_sock, struct msghdr *msg);
#endif
#ifdef CONFIG_HUAWEI_BASTET
void bst_fg_custom_process(struct sock *pst_sock,
	struct msghdr *msg, uint8_t uc_protocol_bitmap);
#endif
uint8_t bst_fg_encode_discard_timer(unsigned long ul_timer);
#ifdef CONFIG_CHR_NETLINK_MODULE
void regist_msg_fun(notify_event *notify);
#endif

#endif
