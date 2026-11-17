/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: define macros and structs for fi_rtt.c
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
 */
#ifndef __HW_FI_RTT_H__
#define __HW_FI_RTT_H__

#include <linux/netlink.h>
#include <net/sock.h>
#include "fi.h"
#include "fi_flow.h"

#define FI_HB_STUDY_NUM 20      /* pkt num used for hb study */
#define FI_HB_PKT_SUB 4         /* hb flow: max pkt num differ */
#define FI_HB_MAX_PPS 2         /* hb flow: max pkt num per sec */
#define FI_HB_MAX_TIME_GAP 3    /* hb flow: max pkt interval */
#define FI_HB_LOST_TOLERANCE 2  /* hb pkt lost tolerance */
#define FI_HB_PKT_LEN 100       /* used for filtering hb pkt */
#define FI_PKT_LOST_PUNISH 50   /* cal punish = lost_ptk_num * 50 */

#define FI_BATTLE_UP_PKT_PER_SEC 3   /* battle flow: min uplink pkt num */
#define FI_BATTLE_DOWN_PKT_PER_SEC 8 /* battle flow: min downlink pkt num */
#define FI_BATTLE_START_THRESH 2     /* time of continuous satisfactions */
#define FI_AUDIO_BATTLE_BOUNDARY 120 /* distinguish between battle and hb */

#define FI_SRTT_VAR 25               /* srtt smoothing factor */
#define FI_HB_SRTT_VAR 25            /* hb srtt smoothing factor */
#define FI_STUDY_INTERVAL 1000
#define FI_MAX_STUDY_TIME (5 * 60 * 1000)
#define FI_MIN_IP_PKT_LEN 20
#define FI_BATTLE_STOP_THRESH 5   /* battle stop: seconds of no pkt time */
#define FI_BATTLE_FLOW_CON 4      /* battle flow: time of match */
#define FI_NO_DOWNLINK_TIME 3     /* reconnect: seconds of no downlink pkt */
#define FI_RECONN_LIMIT 10000     /* reconnect: min interval of to reconn */
#define FI_RECONN_THRESH 180      /* reconnect: rtt must gt 180 */

#define FI_QQFC_PKT_AVG_LEN 400   /* battle flow: avg pkt len of uplink pkt */
#define FI_QQFC_UP_PPS_MIN 14     /* battle flow: min pkt num of uplink */
#define FI_QQFC_UP_PPS_MAX 16     /* battle flow: min pkt num of downlink */
#define FI_APP_DISCONN 5          /* qqfc disconnect or not */
#define FI_APP_DISCONN_STOP 30    /* report game exit after n secs of disconn */

#define FI_HYXD_MIN_LEN 20        /* hyxd min pkt len */
#define FI_HYXD_SEQ_OFFSET 12     /* seq offset in payload */
#define FI_HYXD_ACK_OFFSET 16     /* ack offset in payload */
#define FI_UPPKT_WAIT 200         /* timer interval */
#define FI_ACK_MAX 60000U         /* used for judging ack reversal */
#define FI_RETRANS_PKT 2000       /* used for judging pkt retransmition */
#define FI_ACK_MAX_WAIT 10000     /* max ms of waiting ack */
#define FI_TCP_OPT_HDR_LEN 2      /* tcp option len */
#define FI_TCP_OPT_MPTCP 30       /* tcp option type mptcp */
#define FI_MPTCP_DSS_MINLEN 8     /* mptcp dss min len */
#define FI_MPTCP_SUBTYPE_DSS 2    /* mptcp dss type */
#define FI_GAME_UU_RTT 10         /* this rtt is used for judging uu */
#define FI_UU_BASE_RTT 25         /* uu base rtt */
#define FI_UU_CACHE_NUM 8         /* uu cache num */

#define FI_BH3_KEY_WORD 0xFF      /* bh3: key word */
#define FI_BH3_KEY_OFFSET_UP 9    /* bh3: keyword offset in uplink pkt */
#define FI_BH3_KEY_OFFSET_DOWN 7  /* bh3: keyword offset in downlink pkt */
#define FI_BH3_SEQ_OFFSET_UP 10   /* bh3: seq offset in uplink pkt */
#define FI_BH3_ACK_OFFSET_DOWN 8  /* bh3: ack offset in down link pkt */
#define FI_BH3_VRF_OFFSET_UP 2    /* bh3: ver-number offset in uplink pkt */
#define FI_BH3_VRF_OFFSET_DOWN 12 /* bh3: ver-number offset in downlink pkt */
#define FI_BH3_UP_LEN 12          /* bh3: filter uplink pkt of having seq */
#define FI_BH3_DOWN_LEN 14        /* bh3: filter downlink pkt of having ack */
#define FI_BATTLE_ONGOING 4       /* judge battle: min uplink pkt num per sec */
#define FI_BATTLE_START_PORT_MIN 1025

#define FI_MS 1000          /* ms to sec */
#define FI_PERCENT 100      /* 100 percent */
#define FI_MAX_ORG_RTT 700  /* limit the original value of rtt */
#define FI_MIN_RTT 40       /* limit the min value of rtt */
#define FI_MAX_RTT 500      /* limit the max value of rtt */
#define FI_MAX_HB_RTT 250   /* limit the max value of heartbeat rtt */
#define FI_RTT_BASE 50      /* base rtt */

#define FI_SWITCH_STATUS 0x80000000  /* report status switch */
#define FI_SWITCH_RTT 0x40000000     /* report rtt switch */

#define FI_OFFSET_0 0
#define FI_OFFSET_1 1
#define FI_OFFSET_2 2
#define FI_OFFSET_3 3

struct fi_report_rtt {
	uint32_t uid;
	uint32_t apptype;
	uint32_t rtt;
};

enum fi_rtt_flowtype_enum {
	FI_FLOWTYPE_INIT,
	FI_FLOWTYPE_HB,
	FI_FLOWTYPE_BATTLE,
	FI_FLOWTYPE_UNKNOWN
};

#define fi_min(a, b) (((a) <= (b)) ? (a) : (b))
#define fi_max(a, b) (((a) >= (b)) ? (a) : (b))
#define fi_range(v, l, r) ((v) < (l) ? (l) : ((v) > (r) ? (r) : (v)))
#define fi_abs_sub(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define fi_srv_port(p) (((p)->dir == SMFI_DIR_UP) ? (p)->dport : (p)->sport)
#define fi_cli_ip(p) (((p)->dir == SMFI_DIR_UP) ? (p)->sip : (p)->dip)
#define fi_ms_to_sec(ms) ((ms) / FI_MS)
#define fi_disconn(s) (((s) & FI_STATUS_DISCONN) != 0)

void fi_rtt_timer(void);
void fi_rtt_status(uint32_t appid, uint32_t status);
void fi_rtt_judge_reconn(struct fi_pkt *pktinfo,
		struct fi_gamectx *gamectx);
int fi_rtt_judge_battle_stop(struct fi_gamectx *gamectx,
		struct fi_app_info *appinfo);
void fi_rtt_cal_bh3(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
		struct fi_gamectx *gamectx);
int fi_rtt_entrance(struct fi_pkt *pktinfo,
		struct fi_flowctx *flowctx, uint32_t appid);

#endif /* __HW_FI_RTT_H__ */
