/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: calculate rtt with packet information
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
 */
#include "fi_rtt.h"
#include "securec.h"

static char g_hyxd_data[] = {0x00, 0x00, 0x00, 0x00, 0x51, 0x00, 0x40, 0x00};

/* cal srtt */
uint32_t fi_get_srtt(uint32_t prev_srtt, uint32_t cur_rtt, int alpha)
{
	if (prev_srtt == 0)
		prev_srtt = cur_rtt;

	return (uint32_t)(((FI_PERCENT - alpha) * prev_srtt +
		(alpha * cur_rtt)) / FI_PERCENT);
}

/* send rtt to native */
void fi_rtt_send(uint32_t appid)
{
	struct smfi_ctx *fictx = fi_get_ctx();
	struct fi_report_rtt report = {0};
	struct fi_app_info *appinfo = fictx->appinfo + appid;
	struct fi_gamectx *gamectx = fictx->gamectx + appid;

	/* no rtt or game exit */
	if ((gamectx->final_rtt == 0) || (appinfo->valid == FI_FALSE))
		return;

	/* send rtt to native */
	if ((appinfo->switches & FI_SWITCH_RTT) != 0) {
		report.uid = appinfo->uid;
		report.rtt = gamectx->final_rtt;
		report.apptype = FI_APP_TYPE_GAME;

		emcom_send_msg2daemon(NETLINK_EMCOM_KD_SMARTCARE_FI_REPORT_RTT,
			&report, sizeof(report));

		fi_logd(" : FI rtt=%u uid=%u (btr=%d bts=%d hbr=%d hbst=%d)",
				report.rtt, report.uid, gamectx->rtt,
				gamectx->battlertt, gamectx->hbrtt,
				gamectx->hbsrtt);
	}
}

/* send app status to native */
void fi_rtt_status(uint32_t appid, uint32_t status)
{
	struct smfi_ctx *fictx = fi_get_ctx();
	struct fi_app_info *appinfo = NULL;
	struct fi_report_status report = {0};

	appinfo = fictx->appinfo + appid;
	if ((appinfo->switches & FI_SWITCH_STATUS) != 0) {
		report.uid = appinfo->uid;
		report.status = status;
		report.apptype = FI_APP_TYPE_GAME;

		emcom_send_msg2daemon(
			NETLINK_EMCOM_KD_SMARTCARE_FI_REPORT_APP_STATUS,
			&report, sizeof(report));

		fi_logd(" : FI appid=%u uid=%u status=%u",
			appid, report.uid, status);
	}
}

void fi_reset_gamectx(struct fi_gamectx *gamectx)
{
	struct smfi_ctx *fictx = fi_get_ctx();
	struct fi_gamectx tmpctx;
	struct fi_mpctx *mpctx = NULL;

	mpctx = fictx->mpctx + gamectx->appid;

	(void)memcpy_s(&tmpctx, sizeof(tmpctx), gamectx, sizeof(*gamectx));
	(void)memset_s(gamectx, sizeof(*gamectx), 0, sizeof(*gamectx));
	(void)memset_s(mpctx, sizeof(*mpctx), 0, sizeof(*mpctx));

	/* restore the values of some fields */
	gamectx->appid = tmpctx.appid;
	gamectx->battle_flow_port = tmpctx.battle_flow_port;
	gamectx->cliip = tmpctx.cliip;
	gamectx->hb_flow_port = tmpctx.hb_flow_port;
}

/* refresh the statistics of the number of battle flow */
void fi_rtt_battle_pktnum_cal(struct fi_gamectx *gamectx,
	struct fi_app_info *appinfo)
{
	/* do this only when game is in foreground */
	if (appinfo->appstatus != GAME_SDK_STATE_BACKGROUND) {
		if (gamectx->uplinkpktnum == 0)
			gamectx->nouplinkpkt++;
		else
			gamectx->nouplinkpkt = 0;

		if (gamectx->downlinkpktnum == 0)
			gamectx->nodownlinkpkt++;
		else
			gamectx->nodownlinkpkt = 0;
	}

	gamectx->preuplinkpktnum = gamectx->uplinkpktnum;
	gamectx->predownlinkpktnum = gamectx->downlinkpktnum;
	gamectx->uplinkpktnum = 0;
	gamectx->downlinkpktnum = 0;
}

/* timer: 1. report rtt, 2. judge battle stop or not */
void fi_rtt_timer(void)
{
	int i;
	struct fi_gamectx *gamectx = NULL;
	struct fi_app_info *appinfo = NULL;
	struct smfi_ctx *fictx = fi_get_ctx();

	for (i = 0; i < FI_APPID_MAX; i++) {
		gamectx = fictx->gamectx + i;
		if (!fi_appid_valid(gamectx->appid))
			continue;

		appinfo = fictx->appinfo + gamectx->appid;
		if (appinfo->valid == FI_FALSE)
			continue;

		fi_rtt_battle_pktnum_cal(gamectx, appinfo);

		fi_logd(" : FI appid=%d up=%u/%u down=%u/%u",
				i, gamectx->preuplinkpktnum,
				gamectx->nouplinkpkt,
				gamectx->predownlinkpktnum,
				gamectx->nodownlinkpkt);

		/* has no rtt or game is not in battle */
		if ((fi_battling(gamectx->appstatus) == FI_FALSE) ||
			(gamectx->updatetime == 0))
			continue;

		/* judge battle stop or not */
		if (fi_rtt_judge_battle_stop(gamectx, appinfo) == FI_TRUE)
			continue;

		/* report rtt */
		fi_rtt_send(gamectx->appid);
	}
}

/* get a little-endian uint32 from pkt data */
uint32_t fi_rtt_get_le_u32(uint8_t *data)
{
	uint32_t value;

	value = data[FI_OFFSET_3];
	value = (value << FI_SHIFT_8) + data[FI_OFFSET_2];
	value = (value << FI_SHIFT_8) + data[FI_OFFSET_1];
	value = (value << FI_SHIFT_8) + data[FI_OFFSET_0];

	return value;
}

/* calculate integral rtt, update final_rtt */
void fi_rtt_update_integral(struct fi_gamectx *gamectx,
	int32_t newrtt, int64_t curms)
{
	int64_t cursec = fi_ms_to_sec(curms);
	int32_t avgrtt;

	if (curms <= gamectx->updatetime)
		return;

	if (cursec == fi_ms_to_sec(gamectx->updatetime)) {
		gamectx->rtt_integral += gamectx->rtt *
		(curms - gamectx->updatetime);
	} else {
		gamectx->rtt_integral += gamectx->rtt *
			(cursec * FI_MS - gamectx->updatetime);
		avgrtt = gamectx->rtt_integral / FI_MS /
			(cursec - fi_ms_to_sec(gamectx->updatetime));
		/*
		 * Scenarios with few updates: between games,
		 * characters die, games in the background,
		 * When there are few upstream messages, there are usually
		 * few downstream messages. The RTT calculation will not
		 * be correct, and the RTT will not be updated at this time.
		 */
		if ((gamectx->preuplinkpktnum >= FI_BATTLE_ONGOING) ||
			(avgrtt < gamectx->battlertt)) {
			gamectx->srtt = fi_get_srtt(gamectx->srtt,
				fi_min(avgrtt, FI_MAX_ORG_RTT), FI_SRTT_VAR);
			gamectx->battlertt = fi_min(gamectx->srtt, FI_MAX_RTT);
		}

		gamectx->rtt_integral = gamectx->rtt * (curms - cursec * FI_MS);
	}

	gamectx->updatetime = curms;
	gamectx->rtt = fi_min(newrtt, FI_MAX_ORG_RTT);
}

void fi_rtt_reset_study(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx)
{
	flowctx->studystarttime = pktinfo->msec;
	flowctx->uppktnum = 0;
	flowctx->downpktnum = 0;
	flowctx->uppktbytes = 0;
}

int fi_rtt_study_by_port(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	uint16_t srvport = fi_srv_port(pktinfo);
	if (srvport == gamectx->battle_flow_port) {
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		fi_logd(" : FI learned a battle flow %u:%u by port %u",
				pktinfo->sport, pktinfo->dport,
				gamectx->battle_flow_port);

		/* found a new connection and judge whether it is reconnected */
		fi_rtt_judge_reconn(pktinfo, gamectx);
		gamectx->cliip = fi_cli_ip(pktinfo);

		return FI_SUCCESS;
	} else if (srvport == gamectx->hb_flow_port) {
		flowctx->flowtype = FI_FLOWTYPE_HB;
		fi_logd(" : FI learned a heartbeat flow %u:%u by port %u",
				pktinfo->sport, pktinfo->dport,
				gamectx->hb_flow_port);
		return FI_SUCCESS;
	} else {
		return FI_FAILURE;
	}

	return FI_FAILURE;
}

void fi_rtt_judge_reconn(struct fi_pkt *pktinfo, struct fi_gamectx *gamectx)
{
	struct smfi_ctx *fictx = fi_get_ctx();
	struct fi_app_info *appinfo = fictx->appinfo + gamectx->appid;
	int reconn = FI_FALSE;

	fi_logd(" : FI noup=%u nodown=%u appstatus=%u",
			gamectx->nouplinkpkt, gamectx->nodownlinkpkt,
			gamectx->appstatus);

	if (fi_battling(gamectx->appstatus) == FI_FALSE)
		return;

	/* prevent over-frequent disconnection */
	if (pktinfo->msec - gamectx->reconntime < FI_RECONN_LIMIT)
		return;

	do {
		/*
		 * if wifi-lte link switching occurs, reconnection will
		 * be reported regardless of game in foreground or
		 * in background.
		 */
		if ((gamectx->cliip != fi_cli_ip(pktinfo)) ||
			(gamectx->final_rtt > FI_RECONN_THRESH)) {
			reconn = FI_TRUE;
			gamectx->hbsrtt = 0;
			fi_logd(" : FI reconnect, reset srtt");
			break;
		}

		/*
		 * If the game is not in the background and there is
		 * an upstream message but no downstream message for
		 * several seconds, it is considered to be reconnected.
		 */
		if ((appinfo->appstatus != GAME_SDK_STATE_BACKGROUND) &&
			(gamectx->nouplinkpkt < FI_NO_DOWNLINK_TIME) &&
			(gamectx->nodownlinkpkt >= FI_NO_DOWNLINK_TIME)) {
			reconn = FI_TRUE;
			break;
		}
	} while (0);

	if (reconn) {
		fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_RECONN);
		gamectx->reconntime = pktinfo->msec;
	}
}

/* judge qqfc battle flow and battle start */
void fi_rtt_judge_qqfc(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	int64_t timediff; /* msec */
	uint16_t up_pps;

	timediff = fi_max(pktinfo->msec - flowctx->studystarttime, 1);

	up_pps = flowctx->uppktnum * FI_MS / timediff;
	fi_logd(" : FI up: %u/s down: %u/s flow: %u:%u times: %u",
			up_pps, flowctx->downpktnum, pktinfo->sport,
			pktinfo->dport, flowctx->btflow_times);

	if ((up_pps >= FI_QQFC_UP_PPS_MIN) && (up_pps <= FI_QQFC_UP_PPS_MAX))
		flowctx->btflow_times++;
	else
		flowctx->btflow_times = 0;

	if (flowctx->btflow_times >= FI_BATTLE_FLOW_CON) {
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		gamectx->battle_flow_port = fi_srv_port(pktinfo);
		gamectx->cliip = fi_cli_ip(pktinfo);
		fi_logd(" : FI learned qqfc battle flow %u:%u bt port %u",
				pktinfo->sport, pktinfo->dport,
				gamectx->battle_flow_port);
	}
}

int fi_rtt_judge_hb(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	uint32_t timediff; /* sec */
	int ret = FI_FAILURE;

	if (flowctx->hbstudytime == 0) {
		flowctx->hbstudytime = (uint32_t)(pktinfo->msec);
		return FI_FAILURE;
	}

	if (flowctx->hbdownnum < FI_HB_STUDY_NUM)
		return FI_FAILURE;

	timediff = ((uint32_t)(pktinfo->msec) - flowctx->hbstudytime) / FI_MS;

	fi_logd(" : FI hbup=%u hbdown=%u time=%u, flow %u:%u",
			flowctx->hbupnum, flowctx->hbdownnum, timediff,
			pktinfo->sport, pktinfo->dport);

	/* three conditions: the difference of uplink and downlink
	 * packet num; packet num per second; average packet payload len;
	 * packet interval
	 */
	if ((fi_abs_sub(flowctx->hbupnum, flowctx->hbdownnum) <
		FI_HB_PKT_SUB) &&
		(flowctx->hbupnum <= (FI_HB_MAX_PPS * timediff)) &&
		(timediff <= (FI_HB_MAX_TIME_GAP * flowctx->hbupnum))) {
		flowctx->flowtype = FI_FLOWTYPE_HB;
		gamectx->hb_flow_port = fi_srv_port(pktinfo);
		fi_logd(" : FI learned a bh flow %u:%u hb port %u",
				pktinfo->sport, pktinfo->dport,
				gamectx->hb_flow_port);
		ret = FI_SUCCESS;
	} else {
		/* used for heartbeat flow checking, reset flow type */
		flowctx->flowtype = FI_FLOWTYPE_INIT;
		gamectx->hb_flow_port = 0;
		fi_logd(" : FI hb flow verification failed flow %u:%u",
				pktinfo->sport, pktinfo->dport);
	}

	flowctx->hbstudytime = 0;
	flowctx->hbupnum = 0;
	flowctx->hbdownnum = 0;

	return ret;
}

void fi_rtt_judge_battle_flow(struct fi_pkt *pktinfo,
	struct fi_flowctx *flowctx, struct fi_gamectx *gamectx)
{
	int64_t timediff; /* msec */
	uint32_t avgpktlen;
	uint16_t uppktpersec;
	uint16_t downpktpersec;

	timediff = fi_max(pktinfo->msec - flowctx->studystarttime, 1);
	uppktpersec = flowctx->uppktnum * FI_MS / timediff;
	downpktpersec = flowctx->downpktnum * FI_MS / timediff;
	avgpktlen = flowctx->uppktbytes / fi_max(flowctx->uppktnum, 1);

	fi_logd(" : FI up=%u/s down=%u/s len=%u times=%u flow %u:%u",
			uppktpersec, downpktpersec, avgpktlen,
			flowctx->btflow_times, pktinfo->sport, pktinfo->dport);

	if ((uppktpersec >= FI_BATTLE_UP_PKT_PER_SEC) &&
		(downpktpersec >= FI_BATTLE_DOWN_PKT_PER_SEC) &&
		(avgpktlen < FI_AUDIO_BATTLE_BOUNDARY)) {
		flowctx->btflow_times++;
	} else {
		flowctx->btflow_times = 0;
	}

	if (flowctx->btflow_times >= FI_BATTLE_FLOW_CON) {
		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		gamectx->battle_flow_port = fi_srv_port(pktinfo);
		gamectx->cliip = fi_cli_ip(pktinfo);
		fi_logd(" : FI learned battle flow %u:%u battle port %u",
				pktinfo->sport, pktinfo->dport,
				gamectx->battle_flow_port);
	}
}

void fi_rtt_judge_battle_start(struct fi_pkt *pktinfo,
	struct fi_flowctx *flowctx, struct fi_gamectx *gamectx)
{
	int64_t timediff; /* msec */
	uint16_t uppktpersec;
	uint16_t downpktpersec;

	timediff = fi_max(pktinfo->msec - flowctx->studystarttime, 1);
	uppktpersec = flowctx->uppktnum * FI_MS / timediff;
	downpktpersec = flowctx->downpktnum * FI_MS / timediff;

	if ((fi_min(uppktpersec, downpktpersec) >= FI_BATTLE_UP_PKT_PER_SEC) &&
		(fi_max(uppktpersec, downpktpersec) >=
		FI_BATTLE_DOWN_PKT_PER_SEC)) {
		flowctx->battle_times++;
	} else {
		flowctx->battle_times = 0;
	}

	if (flowctx->battle_times >= FI_BATTLE_START_THRESH) {
		gamectx->appstatus |= FI_STATUS_BATTLE_START;
		fi_logd(" : FI learned battle start status by flow %u:%u",
				pktinfo->sport, pktinfo->dport);

		fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_START);
	}
}

int fi_rtt_judge_battle_stop(struct fi_gamectx *gamectx,
	struct fi_app_info *appinfo)
{
	if (appinfo->appstatus == GAME_SDK_STATE_BACKGROUND)
		return FI_FALSE;

	if (gamectx->appid == FI_APPID_QQFC) {
		if (gamectx->nodownlinkpkt >
			(gamectx->nouplinkpkt + FI_APP_DISCONN)) {
			gamectx->applocalstatus |= FI_STATUS_DISCONN;
			fi_logd(" : FI disconn nouppkt=%u nodownpkt=%u",
					gamectx->nouplinkpkt,
					gamectx->nodownlinkpkt);
		}

		if (gamectx->preuplinkpktnum >= FI_QQFC_UP_PPS_MIN) {
			gamectx->applocalstatus = 0;
			fi_logd(" : FI reconn preup=%u predown=%u",
					gamectx->preuplinkpktnum,
					gamectx->predownlinkpktnum);
		}

		if (fi_disconn(gamectx->applocalstatus) &&
			(gamectx->nouplinkpkt < FI_APP_DISCONN_STOP)) {
			return FI_FALSE;
		}
	}

	if ((gamectx->nouplinkpkt >= FI_BATTLE_STOP_THRESH) &&
		(gamectx->nodownlinkpkt >= FI_BATTLE_STOP_THRESH)) {
		fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_STOP);
		fi_flow_lock();
		fi_reset_gamectx(gamectx);
		fi_flow_unlock();

		return FI_TRUE;
	}

	return FI_FALSE;
}

void fi_rtt_judge_flow_type(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	int ret;

	do {
		ret = fi_rtt_study_by_port(pktinfo, flowctx, gamectx);
		if (ret == FI_SUCCESS)
			break;

		if (pktinfo->msec - flowctx->flowstarttime >
			FI_MAX_STUDY_TIME) {
			flowctx->flowtype = FI_FLOWTYPE_UNKNOWN;
			break;
		}

		switch (gamectx->appid) {
		case FI_APPID_QQFC:
			fi_rtt_judge_qqfc(pktinfo, flowctx, gamectx);
			break;

		case FI_APPID_HYXD:
		case FI_APPID_BH3:
			break;

		default:
			fi_rtt_judge_battle_flow(pktinfo, flowctx, gamectx);
			break;
		}
	} while (0);
}

void fi_rtt_flow_study(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	if (pktinfo->proto == FI_IPPROTO_TCP)
		return;

	if (pktinfo->dir == SMFI_DIR_UP) {
		flowctx->hbupnum++;
		flowctx->uppktnum++;
		flowctx->uppktbytes += pktinfo->len;
	} else {
		if (pktinfo->len < FI_HB_PKT_LEN)
			flowctx->hbdownnum++;
		flowctx->downpktnum++;
	}

	/* judge heartbeat flow */
	if (flowctx->flowtype == FI_FLOWTYPE_INIT) {
		/* If it's a heartbeat flow, don't judge for battle flow */
		if (fi_rtt_judge_hb(pktinfo, flowctx, gamectx) == FI_SUCCESS)
			return;
	}

	if ((flowctx->studystarttime != flowctx->flowstarttime) &&
		(pktinfo->msec - flowctx->studystarttime < FI_STUDY_INTERVAL))
		return;

	if (flowctx->flowtype == FI_FLOWTYPE_INIT)
		fi_rtt_judge_flow_type(pktinfo, flowctx, gamectx);

	/* judge battle start */
	if (fi_battling(gamectx->appstatus) == FI_FALSE) {
		/* qqfc bh3 found battle flow means battle start */
		if ((gamectx->appid == FI_APPID_QQFC) ||
			(gamectx->appid == FI_APPID_BH3)) {
			if (flowctx->flowtype == FI_FLOWTYPE_BATTLE) {
				gamectx->appstatus |= FI_STATUS_BATTLE_START;
				fi_logd(" : FI learned bt start bt flow %u:%u",
						pktinfo->sport, pktinfo->dport);

				fi_rtt_status(gamectx->appid,
					FI_STATUS_BATTLE_START);
			}
		} else {
			fi_rtt_judge_battle_start(pktinfo, flowctx, gamectx);
		}
	}

	fi_rtt_reset_study(pktinfo, flowctx);
}

int fi_rtt_cal_tcprtt(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	int32_t newrtt;
	char *logstr = "";

	if (pktinfo->dir == SMFI_DIR_UP) {
		if (flowctx->seq == 0) {
			if (pktinfo->len > 0) {
				flowctx->seq = pktinfo->seq + pktinfo->len;
				flowctx->uppkttime = pktinfo->msec;
				fi_logd(" : FI save seq=%u nextseq=%u",
						pktinfo->seq, flowctx->seq);
			}
			return FI_SUCCESS;
		}

		if (pktinfo->msec - gamectx->updatetime < FI_UPPKT_WAIT)
			return FI_FAILURE;

		if ((flowctx->uppkttime > 0) &&
			(pktinfo->msec - flowctx->uppkttime >
			FI_ACK_MAX_WAIT)) {
			fi_logd(" : FI reset tcprtt ctx");
			flowctx->seq = 0;
			flowctx->uppkttime = 0;
			return FI_FAILURE;
		}

		newrtt = fi_range(pktinfo->msec - flowctx->uppkttime,
			FI_MIN_RTT, FI_MAX_ORG_RTT);
		logstr = "uplink pkt";
	} else {
		int32_t rawrtt;

		if ((flowctx->uppkttime == 0) ||
			(pktinfo->ack - flowctx->seq > (uint32_t)FI_ACK_MAX))
			return FI_FAILURE;

		rawrtt = pktinfo->msec - flowctx->uppkttime;
		gamectx->rawrtt = fi_get_srtt(gamectx->rawrtt,
			rawrtt, FI_SRTT_VAR);
		newrtt = fi_range(rawrtt, FI_MIN_RTT, FI_MAX_ORG_RTT);

		fi_logd(" : FI get ack appid=%u rawrtt=%d %u:%u, %u:%u",
				gamectx->appid, rawrtt, flowctx->seq,
				pktinfo->ack, pktinfo->sport, pktinfo->dport);

		flowctx->seq = 0;
		flowctx->uppkttime = 0;
		logstr = "ack";
	}

	fi_rtt_update_integral(gamectx, newrtt, pktinfo->msec);
	fi_logd(" : FI update rtt by tcp %s appid=%u newrtt=%d battlertt=%d",
			logstr, gamectx->appid, newrtt, gamectx->battlertt);

	return FI_SUCCESS;
}

int fi_rtt_cal_mptcp(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	int32_t rawrtt;
	int32_t newrtt;
	struct smfi_ctx *fictx = fi_get_ctx();
	struct fi_mpctx *mpctx = fictx->mpctx + gamectx->appid;

	if (pktinfo->dir == SMFI_DIR_UP) {
		mpctx->preseq = fi_max(mpctx->preseq, pktinfo->seq);

		if ((mpctx->seq == 0) && (pktinfo->len > 0) &&
			((pktinfo->seq >= mpctx->preseq) ||
			(fi_abs_sub(pktinfo->seq, mpctx->preseq) >
			FI_RETRANS_PKT))) {
			mpctx->seq = pktinfo->seq + pktinfo->len;
			mpctx->uppkttime = pktinfo->msec;
			mpctx->preseq = mpctx->seq;
			fi_logd(" : FI save seq=%u nextseq=%u preseq=%u",
					pktinfo->seq, mpctx->seq,
					mpctx->preseq);
		}

		if ((mpctx->uppkttime > 0) &&
			(pktinfo->msec - mpctx->uppkttime > FI_ACK_MAX_WAIT)) {
			fi_logd(" : FI reset mptcp ctx");
			(void)memset_s(mpctx, sizeof(*mpctx), 0, sizeof(*mpctx));
		}

		return FI_SUCCESS;
	}

	mpctx->preseq = fi_max(mpctx->preseq, pktinfo->ack);
	if ((mpctx->uppkttime == 0) ||
		(pktinfo->ack - mpctx->seq > (uint32_t)FI_ACK_MAX))
		return FI_FAILURE;

	rawrtt = pktinfo->msec - mpctx->uppkttime;
	gamectx->rawrtt = fi_get_srtt(gamectx->rawrtt,
		rawrtt, FI_SRTT_VAR);
	newrtt = fi_range(rawrtt, FI_MIN_RTT, FI_MAX_ORG_RTT);

	fi_logd(" : FI get ack rawrtt=%d seq=%u ack=%u flow %u:%u",
			rawrtt, mpctx->seq, pktinfo->ack,
			pktinfo->sport, pktinfo->dport);

	mpctx->seq = 0;
	mpctx->uppkttime = 0;

	fi_rtt_update_integral(gamectx, newrtt, pktinfo->msec);
	fi_logd(" : FI update rtt by mptcp appid=%u newrtt=%d battlertt=%d",
			gamectx->appid, newrtt, gamectx->battlertt);

	return FI_SUCCESS;
}

int fi_rtt_cal_hyxd(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	int32_t newrtt;

	if (pktinfo->bufdatalen < FI_HYXD_MIN_LEN)
		return FI_FAILURE;

	/* uplink packet, get and save seq */
	if (pktinfo->dir == SMFI_DIR_UP) {
		if (memcmp(pktinfo->data, g_hyxd_data, sizeof(g_hyxd_data)))
			return FI_FAILURE;

		flowctx->flowtype = FI_FLOWTYPE_BATTLE;
		if (flowctx->seq == 0) {
			flowctx->seq = fi_rtt_get_le_u32(pktinfo->data +
				FI_HYXD_SEQ_OFFSET);
			flowctx->uppkttime = pktinfo->msec;
		}

		flowctx->uppktnum++;
		if ((pktinfo->msec - flowctx->uppkttime < FI_UPPKT_WAIT) ||
			(flowctx->uppktnum <= FI_HB_LOST_TOLERANCE))
			return FI_FAILURE;

		newrtt = fi_range(pktinfo->msec - flowctx->uppkttime,
			FI_MIN_RTT, FI_MAX_ORG_RTT);
	} else {
		uint32_t ack;

		if (flowctx->seq == 0)
			return FI_FAILURE;

		/* get ack */
		ack = fi_rtt_get_le_u32(pktinfo->data + FI_HYXD_ACK_OFFSET);
		if (ack - flowctx->seq > (uint32_t)FI_ACK_MAX)
			return FI_FAILURE;

		newrtt = fi_range(pktinfo->msec - flowctx->uppkttime,
			FI_MIN_RTT, FI_MAX_ORG_RTT);

		flowctx->seq = 0;
		flowctx->uppkttime = 0;
		flowctx->uppktnum = 0;
	}

	fi_rtt_update_integral(gamectx, newrtt, pktinfo->msec);
	fi_logd(" : FI update hyxd rtt, appid=%u rtt=%d",
			gamectx->appid, gamectx->battlertt);

	return FI_SUCCESS;
}

int fi_rtt_cal_battle(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	char *logstr = "";

	if (pktinfo->dir == SMFI_DIR_UP) {
		/* Wait for downlink data to start. */
		if (gamectx->downlast == 0)
			return FI_FAILURE;

		/* Use uplink pkt as a timer */
		if (pktinfo->msec - gamectx->downlast < FI_UPPKT_WAIT)
			return FI_FAILURE;

		/* For too long, there is no uplink pkt, update RTT. */
		gamectx->rtt = fi_range(pktinfo->msec - gamectx->downlast,
			FI_MIN_RTT, FI_MAX_ORG_RTT);
		logstr = "uplink pkt";
	} else {
		/* Save the timestamp of the first downlink pkt. */
		if (gamectx->downlast == 0) {
			gamectx->downlast = pktinfo->msec;
			return FI_FAILURE;
		}

		/* Calculate RTT by adjacent downlink pkts */
		gamectx->rtt = fi_range((pktinfo->msec - gamectx->downlast),
			FI_MIN_RTT, FI_MAX_ORG_RTT);
		gamectx->downlast = pktinfo->msec;
		logstr = "downlink pkt";
	}

	fi_rtt_update_integral(gamectx, gamectx->rtt, pktinfo->msec);
	fi_logd(" : FI update rtt by battle flow %s, rawrtt=%u, btrtt=%d",
		logstr, gamectx->rtt, gamectx->battlertt);

	return FI_SUCCESS;
}

void fi_rtt_cal_hb(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	if (pktinfo->dir == SMFI_DIR_UP) {
		flowctx->hbupnum++;
		flowctx->uppkttime = pktinfo->msec;
	} else {
		int16_t rawrtt;

		flowctx->hbdownnum++;
		if (flowctx->uppkttime == 0)
			return;

		rawrtt = pktinfo->msec - flowctx->uppkttime;
		if (rawrtt >= FI_MAX_ORG_RTT) {
			flowctx->uppkttime = 0;
			return;
		}

		gamectx->hbrtt = fi_range(rawrtt, FI_MIN_RTT, FI_MAX_HB_RTT);
		gamectx->hbsrtt = fi_get_srtt(gamectx->hbsrtt, gamectx->hbrtt,
			FI_HB_SRTT_VAR);
		fi_logd(" : FI update rtt by hb appid=%u hbrtt=%d hbsrtt=%d",
				gamectx->appid, gamectx->hbrtt,
				gamectx->hbsrtt);

		flowctx->uppkttime = 0;
	}

	if (fi_rtt_judge_hb(pktinfo, flowctx, gamectx) == FI_SUCCESS)
		fi_logd(" : FI verify hb flow success");

	if (flowctx->flowtype != FI_FLOWTYPE_HB) {
		gamectx->hbrtt = 0;
		gamectx->hbsrtt = 0;
		flowctx->uppkttime = 0;
	}
}

/* "uu" is an app on mobile phones for network acceleration */
void fi_rtt_cal_uu(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	static uint32_t uu_seq[FI_UU_CACHE_NUM] = {0};
	static int64_t uu_time[FI_UU_CACHE_NUM] = {0};
	static uint32_t curcache;
	uint32_t i;
	int32_t newrtt = 0;

	/* only needs tcp */
	if (pktinfo->proto != FI_IPPROTO_TCP)
		return;

	/* drop ack packet if its payload is 0 */
	if (pktinfo->len == 0)
		return;

	/* uplink packet, only save seq and timestamp */
	if (pktinfo->dir == SMFI_DIR_UP) {
		uu_seq[curcache] = pktinfo->seq + pktinfo->len;
		uu_time[curcache] = pktinfo->msec;
		curcache = (curcache + 1) % FI_UU_CACHE_NUM;
		return;
	}

	/* downlink packet, calculate rtt */
	for (i = 0; i < FI_UU_CACHE_NUM; i++) {
		if (uu_seq[curcache] && (pktinfo->ack - uu_seq[curcache] <
			(uint32_t)FI_ACK_MAX)) {
			newrtt = fi_range(pktinfo->msec - uu_time[curcache],
				FI_MIN_RTT, FI_MAX_ORG_RTT);
			uu_seq[curcache] = 0;
			uu_time[curcache] = 0;
		}

		curcache = (curcache + 1) % FI_UU_CACHE_NUM;
	}

	if (newrtt == 0)
		return;

	gamectx->rtt = newrtt;
	gamectx->srtt = fi_get_srtt(gamectx->srtt, newrtt, FI_SRTT_VAR);
	gamectx->battlertt = fi_min(gamectx->srtt, FI_MAX_RTT);

	fi_logd(" : FI update uu rtt newrtt=%d battlertt=%d",
		newrtt, gamectx->battlertt);
}

void fi_rtt_cal_bh3_uplink(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx)
{
	struct fi_flow_bh3 *bh3cache = NULL;
	uint16_t seq;
	uint16_t verify;
	int i;

	/* the payload len of packets with seq-number is fixed to 12 */
	if (pktinfo->bufdatalen != FI_BH3_UP_LEN)
		return;

	if (pktinfo->data[FI_BH3_KEY_OFFSET_UP] != FI_BH3_KEY_WORD)
		return;

	/* get seq and verify-number */
	seq = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_SEQ_OFFSET_UP));
	verify = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_VRF_OFFSET_UP));

	for (i = 0; i < FI_BH3_SEQ_CACHE_NUM; i++) {
		bh3cache = flowctx->flow_bh3 + i;
		if (seq > bh3cache->seq) {
			bh3cache->seq = seq;
			bh3cache->verify = verify;
			bh3cache->time = (uint32_t)pktinfo->msec;
			fi_logd(" : FI bh3 up seq=%04x verify=%04x time=%u",
					seq, verify, bh3cache->time);
			break;
		}
	}

	/*lint -e676*/
	for (i++; (i < FI_BH3_SEQ_CACHE_NUM) &&
		(flowctx->flow_bh3[i].seq > 0); i++)
		flowctx->flow_bh3[i].seq = 0;
	/*lint +e676*/
}

void fi_rtt_cal_bh3_downlink(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	struct fi_flow_bh3 *bh3cache = NULL;
	uint16_t ack;
	uint16_t verify;
	uint32_t newrtt = 0;
	int i;

	/* the payload len of packets with ack-number is fixed to 12 */
	if (pktinfo->bufdatalen != FI_BH3_DOWN_LEN)
		return;

	if (pktinfo->data[FI_BH3_KEY_OFFSET_DOWN] != FI_BH3_KEY_WORD)
		return;

	/* get ack and verify-number */
	ack = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_ACK_OFFSET_DOWN));
	verify = ntohs(*(uint16_t *)(pktinfo->data + FI_BH3_VRF_OFFSET_DOWN));

	fi_logd(" : FI down ack=%04x verify=%04x", ack, verify);

	for (i = 0; i < FI_BH3_SEQ_CACHE_NUM; i++) {
		bh3cache = flowctx->flow_bh3 + i;
		if ((ack == bh3cache->seq) && (verify == bh3cache->verify)) {
			newrtt = (uint32_t)pktinfo->msec - bh3cache->time;
			bh3cache->seq = 0;
			fi_logd(" : FI down ack=%04x verify=%04x rawrtt=%u",
				ack, verify, newrtt);
			break;
		}
	}

	if (i == FI_BH3_SEQ_CACHE_NUM)
		return;

	flowctx->flowtype = FI_FLOWTYPE_BATTLE;
	newrtt = fi_range(newrtt, FI_MIN_RTT, FI_MAX_ORG_RTT);

	gamectx->rtt = newrtt;
	gamectx->srtt = fi_get_srtt(gamectx->srtt, newrtt, FI_SRTT_VAR);
	gamectx->battlertt = fi_min(gamectx->srtt, FI_MAX_RTT);
	gamectx->updatetime = pktinfo->msec;

	fi_logd(" : FI bh3 rtt=%d srtt=%d btrtt=%d",
		gamectx->rtt, gamectx->srtt, gamectx->battlertt);
}

void fi_rtt_cal_bh3(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	struct fi_gamectx *gamectx)
{
	if (pktinfo->dir == SMFI_DIR_UP)
		fi_rtt_cal_bh3_uplink(pktinfo, flowctx);
	else
		fi_rtt_cal_bh3_downlink(pktinfo, flowctx, gamectx);
}

/* use heartbeat rtt to amend final rtt */
void fi_rtt_amend(struct fi_gamectx *gamectx, struct fi_gamectx *uuctx)
{
	switch (gamectx->appid) {
	case FI_APPID_CJZC:
	case FI_APPID_QJCJ:
		if (gamectx->hbsrtt) {
			gamectx->final_rtt = fi_range(
				gamectx->battlertt + gamectx->hbsrtt -
				FI_RTT_BASE, FI_MIN_RTT, FI_MAX_RTT);
		} else {
			gamectx->final_rtt = gamectx->battlertt;
		}
		break;

	case FI_APPID_QQFC:
		if ((gamectx->rawrtt < FI_GAME_UU_RTT) &&
			(uuctx->battlertt > 0))
			gamectx->final_rtt = uuctx->battlertt + FI_UU_BASE_RTT;
		else
			gamectx->final_rtt = gamectx->battlertt;
		break;

	default:
		gamectx->final_rtt = gamectx->battlertt;
		break;
	}
}

int fi_rtt_record_battle(struct fi_pkt *pktinfo, struct fi_gamectx *gamectx)
{
	if (pktinfo->dir == SMFI_DIR_UP)
		gamectx->uplinkpktnum++;
	else
		gamectx->downlinkpktnum++;

	return FI_SUCCESS;
}

int fi_rtt_check_para(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	uint32_t appid)
{
	static int64_t prepkttime;

	if ((pktinfo == NULL) || (flowctx == NULL) ||
		(!fi_appid_valid(appid))) {
		fi_logd(" : FI parameter error, appid=%u", appid);
		return FI_FAILURE;
	}

	if (pktinfo->msec < prepkttime)
		fi_logd(" : FI pkt timestamp error, %lld < %lld",
			pktinfo->msec, prepkttime);

	prepkttime = pktinfo->msec;

	if (flowctx->flowstarttime == 0)
		flowctx->flowstarttime = pktinfo->msec;

	if (flowctx->studystarttime == 0)
		flowctx->studystarttime = pktinfo->msec;

	return FI_SUCCESS;
}

int fi_rtt_filter_pkt(struct fi_pkt *pktinfo, uint32_t appid)
{
	int ret = FI_SUCCESS;

	switch (appid) {
	/* qqfc needs both tcp and udp */
	case FI_APPID_QQFC:
		break;

	case FI_APPID_UU:
		if (pktinfo->proto == FI_IPPROTO_UDP)
			ret = FI_FAILURE;
		break;

	default:
		if (pktinfo->proto == FI_IPPROTO_TCP)
			ret = FI_FAILURE;
		break;
	}

	return ret;
}

void fi_rtt_cal_common(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	uint32_t appid, struct fi_gamectx *gamectx)
{
	struct smfi_ctx *fictx = fi_get_ctx();

	switch (appid) {
	case FI_APPID_HYXD:
		fi_rtt_cal_hyxd(pktinfo, flowctx, gamectx);
		break;

	case FI_APPID_QQFC:
		if ((pktinfo->proto == FI_IPPROTO_TCP) &&
			(gamectx->battle_flow_port == fi_srv_port(pktinfo))) {
			if (pktinfo->mptcp)
				fi_rtt_cal_mptcp(pktinfo, flowctx, gamectx);
			else
				fi_rtt_cal_tcprtt(pktinfo, flowctx, gamectx);
		}
		break;

	default:
		if (flowctx->flowtype == FI_FLOWTYPE_BATTLE)
			fi_rtt_cal_battle(pktinfo, flowctx, gamectx);

		if (flowctx->flowtype == FI_FLOWTYPE_HB)
			fi_rtt_cal_hb(pktinfo, flowctx, gamectx);
		break;
	}

	fi_rtt_amend(gamectx, fictx->gamectx + FI_APPID_UU);
}

int fi_rtt_entrance(struct fi_pkt *pktinfo, struct fi_flowctx *flowctx,
	uint32_t appid)
{
	struct smfi_ctx *fictx = fi_get_ctx();
	struct fi_gamectx *gamectx = NULL;

	if (fictx->appinfo[FI_APPID_UU].valid ||
		fictx->appinfo[FI_APPID_XUNYOU].valid)
		return FI_FAILURE;

	if (fi_rtt_check_para(pktinfo, flowctx, appid) != FI_SUCCESS)
		return FI_FAILURE;

	if (fi_rtt_filter_pkt(pktinfo, appid) != FI_SUCCESS)
		return FI_FAILURE;

	gamectx = fictx->gamectx + appid;
	gamectx->appid = appid;

	if (appid == FI_APPID_UU) {
		fi_rtt_cal_uu(pktinfo, flowctx, gamectx);
		return FI_SUCCESS;
	}

	if ((flowctx->flowtype == FI_FLOWTYPE_INIT) ||
		(fi_battling(gamectx->appstatus) == FI_FALSE))
		fi_rtt_flow_study(pktinfo, flowctx, gamectx);

	if ((flowctx->flowtype == FI_FLOWTYPE_BATTLE) ||
		(gamectx->battle_flow_port == fi_srv_port(pktinfo)))
		fi_rtt_record_battle(pktinfo, gamectx);

	if (gamectx->appid == FI_APPID_BH3) {
		fi_rtt_cal_bh3(pktinfo, flowctx, gamectx);
		fi_rtt_amend(gamectx, fictx->gamectx + FI_APPID_UU);
		return FI_SUCCESS;
	}

	if (fi_battling(gamectx->appstatus) == FI_FALSE)
		return FI_SUCCESS;

	fi_rtt_cal_common(pktinfo, flowctx, appid, gamectx);

	return FI_SUCCESS;
}
