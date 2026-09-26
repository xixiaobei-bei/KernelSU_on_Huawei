/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: define macros and structs for fi.c
* Author: liyouyong liyouyong@huawei.com
* Create: 2018-09-10
*/

#ifndef __FI_H_INCLUDED__
#define __FI_H_INCLUDED__

#include <linux/netfilter_ipv4.h>
#include <linux/types.h>
#include "../../emcom_netlink.h"
#include "../../emcom_utils.h"

#define FI_APPID_NULL 0
#define FI_APPID_HYXD 1     /* hyxd appid */
#define FI_APPID_WZRY 2     /* wzry appid */
#define FI_APPID_CJZC 3     /* cjzc appid */
#define FI_APPID_QJCJ 4     /* qjcj appid */
#define FI_APPID_CYHX 5     /* cyhx appid */
#define FI_APPID_QQFC 6     /* qqfc appid */
#define FI_APPID_BH3 7      /* bh3 appid */
#define FI_APPID_UU 11      /* uu appid */
#define FI_APPID_XUNYOU 12  /* xunyou appid */
#define FI_APPID_MAX 16     /* appid up limit */

#define FI_TIMER_INTERVAL 1
#define FI_IPPROTO_UDP 17
#define FI_IPPROTO_TCP 6
#define FI_IP_VER_4 4
#define FI_IP_HDR_LEN_BASE 4
#define FI_TCP_HDR_LEN_BASE 4
#define FI_LOOP_ADDR 127
#define FI_TRUE 1
#define FI_FALSE 0
#define FI_FAILURE (-1)
#define FI_SUCCESS 0

#define GAME_SDK_STATE_DEFAULT 0     /* game status: default */
#define GAME_SDK_STATE_BACKGROUND 3  /* game status: background */
#define GAME_SDK_STATE_FOREGROUND 4  /* game status: foreground */
#define GAME_SDK_STATE_DIE 5         /* game status: exit */
#define FI_STATUS_BATTLE_START 1     /* game status: battle start */
#define FI_STATUS_BATTLE_STOP 0      /* game status: battle stop */
#define FI_STATUS_BATTLE_RECONN 0x10 /* game status: reconnect */
#define FI_STATUS_DISCONN 0x01       /* game locat status: disconnect */

#define FI_APP_NAME_WZRY "com.tencent.tmgp.sgame"        /* app name: wzry */
#define FI_APP_NAME_CJZC "com.tencent.tmgp.pubgmhd"      /* app name: cjzc */
#define FI_APP_NAME_QJCJ "com.tencent.tmgp.pubgm"        /* app name: qjcj */
#define FI_APP_NAME_HYXD_HW "com.netease.hyxd.huawei"    /* app name: hyxd */
#define FI_APP_NAME_HYXD "com.netease.hyxd"              /* app name: hyxd */
#define FI_APP_NAME_CYHX "com.tencent.tmgp.cf"           /* app name: cyhx */
#define FI_APP_NAME_QQFC "com.tencent.tmgp.speedmobile"  /* app name: qqfc */
#define FI_APP_NAME_BH3 "com.miHoYo.bh3.huawei"          /* app name: bh3 */
#define FI_APP_NAME_BH3_2 "com.miHoYo.bh3.qihoo"         /* app name: bh3 */
#define FI_APP_NAME_BH3_3 "com.miHoYo.bh3.uc"            /* app name: bh3 */
#define FI_APP_NAME_BH3_4 "com.tencent.tmgp.bh3"         /* app name: bh3 */
#define FI_APP_NAME_BH3_5 "com.miHoYo.enterprise.NGHSoD" /* app name: bh3 */
#define FI_APP_NAME_UU "com.netease.uu"                  /* app name: uu */
#define FI_APP_NAME_XUNYOU "cn.wsds.gamemaster"          /* app name: xunyou */

#define FI_APP_TYPE_GAME 1
#define FI_TIMER_STR "fi_timer"

#define KMV_4 4   /* kernel main version number */
#define KSV_14 14 /* kernel subversion number */
#define KRV_1 1   /* kernel revision number */

/* mptcp DSS(Data Sequence Signal), 2 bytes */
struct fi_mptcp_dss {
#if defined(__BIG_ENDIAN_BITFIELD)
	uint16_t subtype : 4;
	uint16_t rev1 : 4;
	uint16_t rev2 : 3;
	uint16_t fin : 1;
	uint16_t seq8 : 1;   /* seq is 8 bytes or 4 bytes */
	uint16_t seqpre : 1; /* has seq or not */
	uint16_t ack8 : 1;   /* ack is 8 bytes or 4 bytes */
	uint16_t ackpre : 1; /* has ack or not */
#else
	uint16_t rev1 : 4;
	uint16_t subtype : 4;
	uint16_t ackpre : 1;
	uint16_t ack8 : 1;
	uint16_t seqpre : 1;
	uint16_t seq8 : 1;
	uint16_t fin : 1;
	uint16_t rev2 : 3;
#endif
};

enum fi_pkt_dir {
	SMFI_DIR_ANY = 0,
	SMFI_DIR_UP,
	SMFI_DIR_DOWN,
	SMFI_DIR_MAX
};

struct fi_msg_applaunch {
	uint32_t uid;
	uint32_t switches;
	char appname[0];
};

struct fi_msg_appstatus {
	uint32_t uid;
	uint32_t appstatus;
	char appname[0];
};

struct fi_report_status {
	uint32_t uid;
	uint32_t apptype;
	uint32_t status;
};

struct fi_app_info {
	uint32_t uid;
	uint16_t appid;
	uint8_t valid;
	uint8_t rev;
	uint32_t appstatus;
	uint32_t switches;
};

/* rtt cache data for every game */
struct fi_gamectx {
	uint8_t appid;
	uint8_t appstatus;
	uint8_t applocalstatus;
	uint8_t preuplinkpktnum;
	uint8_t predownlinkpktnum;
	uint8_t uplinkpktnum;
	uint8_t downlinkpktnum;
	uint8_t nouplinkpkt;
	uint8_t nodownlinkpkt;
	uint8_t rev1;
	int16_t rev2;
	int16_t hbrtt;
	int16_t hbsrtt;
	uint16_t battle_flow_port;
	uint16_t hb_flow_port;
	uint32_t cliip;
	int32_t rawrtt;
	int32_t rtt;
	int32_t srtt;
	int32_t battlertt;
	int32_t final_rtt;
	int64_t downlast;
	int64_t updatetime;
	int64_t rtt_integral;
	int64_t reconntime;
};

struct fi_mpctx {
	uint32_t seq;
	uint32_t preseq;
	int64_t uppkttime;
};

struct smfi_ctx {
	struct timer_list tm;
	struct mutex appinfo_lock; /* lock appinfo */
	struct mutex nf_mutex; /* lock nf nf_register */
	uint32_t memused;
	uint8_t enable;
	uint8_t appidmin;
	uint8_t appidmax;
	uint8_t nf_exist;
	struct fi_app_info appinfo[FI_APPID_MAX]; /* index by appid */
	struct fi_gamectx gamectx[FI_APPID_MAX];  /* index by appid */
	struct fi_mpctx mpctx[FI_APPID_MAX];      /* save mp info */
};

#define fi_battling(s) (((s) & FI_STATUS_BATTLE_START) != 0)

#define fi_logd(fmt, ...) do \
		hwlog_debug("%s" fmt "\n", __func__, ##__VA_ARGS__); \
	while (0)

#define fi_loginfo(fmt, ...) do \
		hwlog_info("%s" fmt "\n", __func__, ##__VA_ARGS__); \
	while (0)

#define fi_logerr(fmt, ...) do \
		hwlog_err("%s" fmt "\n", __func__, ##__VA_ARGS__); \
	while (0)

bool fi_appid_valid(uint32_t id);
void smfi_event_process(int32_t event, uint8_t *pdata, uint16_t len);
void smfi_init(void);
void smfi_deinit(void);
void *fi_malloc(uint32_t size);
void fi_free(const void *ptr, uint32_t size);
struct smfi_ctx *fi_get_ctx(void);
unsigned int fi_hook_out(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *state);
unsigned int fi_hook_in(void *priv, struct sk_buff *skb,
		const struct nf_hook_state *state);

#endif // __FI_H_INCLUDED__
