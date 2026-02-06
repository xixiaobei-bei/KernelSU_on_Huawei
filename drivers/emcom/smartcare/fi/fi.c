/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: fi module entrance, get and parse packet from netfilter
* Author: liyouyong liyouyong@huawei.com
* Create: 2018-09-10
*/

#include "fi.h"
#include <net/udp.h>
#include <linux/version.h>
#include "fi_flow.h"
#include "fi_rtt.h"
#include "securec.h"

static struct smfi_ctx g_fi_ctx;

/* netfilter hook */
#if KERNEL_VERSION(KMV_4, KSV_14, KRV_1) <= LINUX_VERSION_CODE
static const struct nf_hook_ops fi_nfhooks[] = {
#else
static struct nf_hook_ops fi_nfhooks[] = {
#endif
	{
		.hook     = fi_hook_out,
		.pf       = PF_INET,
		.hooknum  = NF_INET_LOCAL_OUT,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook     = fi_hook_in,
		.pf       = PF_INET,
		.hooknum  = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER - 1,
	},
};

bool fi_appid_valid(uint32_t id)
{
	if (id < FI_APPID_MAX && id > FI_APPID_NULL)
		return true;
	return false;
}

struct smfi_ctx *fi_get_ctx(void)
{
	return &g_fi_ctx;
}

/* calculate memory used by fi */
void fi_mem_used(uint32_t memlen)
{
	g_fi_ctx.memused += memlen;
}

/* reduce mem size of fi */
void fi_mem_de_used(uint32_t memlen)
{
	g_fi_ctx.memused -= memlen;
}

void fi_free(const void *ptr, uint32_t size)
{
	kfree(ptr);
	fi_mem_de_used(size);
}

void *fi_malloc(uint32_t size)
{
	void *ptr = NULL;

	if (size == 0)
		return NULL;

	ptr = kmalloc(size, GFP_ATOMIC);
	if (ptr == NULL)
		return NULL;

	fi_mem_used(size);
	(void)memset_s(ptr, size, 0, size);

	return ptr;
}

void fi_register_nf_hook(struct smfi_ctx *fictx)
{
	int ret;

	mutex_lock(&(fictx->nf_mutex));

	if (fictx->nf_exist == FI_TRUE) {
		mutex_unlock(&(fictx->nf_mutex));
		return;
	}

#if KERNEL_VERSION(KMV_4, KSV_14, KRV_1) <= LINUX_VERSION_CODE
	ret = nf_register_net_hooks(&init_net, fi_nfhooks,
		ARRAY_SIZE(fi_nfhooks));
#else
	ret = nf_register_hooks(fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#endif
	if (ret == 0) {
		fictx->nf_exist = FI_TRUE;

		/* register timer */
		fictx->tm.expires = jiffies + HZ / FI_TIMER_INTERVAL;
		mod_timer(&(fictx->tm), jiffies + HZ / FI_TIMER_INTERVAL);
	}

	mutex_unlock(&(fictx->nf_mutex));

	if (ret)
		fi_logerr(" : FI register nf hooks failed, ret=%d", ret);
	else
		fi_logd(" : FI register nf hooks successfully");
}

void fi_unregister_nf_hook(struct smfi_ctx *fictx)
{
	mutex_lock(&(fictx->nf_mutex));

	if (fictx->nf_exist == FI_FALSE) {
		mutex_unlock(&(fictx->nf_mutex));
		return;
	}

#if KERNEL_VERSION(KMV_4, KSV_14, KRV_1) <= LINUX_VERSION_CODE
	nf_unregister_net_hooks(&init_net, fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#else
	nf_unregister_hooks(fi_nfhooks, ARRAY_SIZE(fi_nfhooks));
#endif

	del_timer(&(fictx->tm));
	fictx->nf_exist = FI_FALSE;
	mutex_unlock(&(fictx->nf_mutex));

	fi_logd(" : FI unregister nf hooks successfully");
}

/* find appid by uid */
static uint32_t fi_find_appid(uint32_t uid, struct smfi_ctx *fictx)
{
	int i;

	if (uid == 0)
		return FI_APPID_NULL;

	for (i = fictx->appidmin; i <= fictx->appidmax; i++) {
		if ((fictx->appinfo[i].uid == uid) && (fictx->appinfo[i].valid))
			return fictx->appinfo[i].appid;
	}

	return FI_APPID_NULL;
}

static int fi_pkt_check(struct sk_buff *skb, int dir)
{
	struct iphdr *iph = ip_hdr(skb);

	/* need ip packet only */
	if ((iph == NULL) || (skb->len <= FI_MIN_IP_PKT_LEN))
		return FI_FAILURE;

	/* need ipv4 only */
	if (iph->version != FI_IP_VER_4)
		return FI_FAILURE;

	/* no need loop interface */
	if ((*(uint8_t *)&(iph->saddr) == FI_LOOP_ADDR) ||
		(*(uint8_t *)&(iph->daddr) == FI_LOOP_ADDR))
		return FI_FAILURE;

	if (iph->protocol == FI_IPPROTO_UDP) {
		if (skb->len < sizeof(struct udphdr) + sizeof(struct iphdr))
			return FI_FAILURE;
	} else if (iph->protocol == FI_IPPROTO_TCP) {
		if (skb->len < sizeof(struct tcphdr) + sizeof(struct iphdr))
			return FI_FAILURE;
	} else {
		return FI_FAILURE;
	}

	return FI_SUCCESS;
}

static struct fi_mptcp_dss *fi_mptcp_getdss(struct tcphdr *tcph,
	uint32_t hdrlen, uint32_t *retleftlen, uint8_t **retleftdata)
{
	uint8_t optkind = 0; /* tcp opt type */
	uint8_t optlen = 0; /* tcp opt len */
	uint32_t leftlen = hdrlen - sizeof(struct tcphdr);
	uint8_t *leftdata = (uint8_t *)tcph + sizeof(struct tcphdr);
	struct fi_mptcp_dss *mptcpdss = NULL;

	while (leftlen > FI_TCP_OPT_HDR_LEN) {
		optkind = *leftdata;
		optlen = *(leftdata + 1);

		leftdata += FI_TCP_OPT_HDR_LEN;
		leftlen -= FI_TCP_OPT_HDR_LEN;

		/* found mptcp opt */
		if (optkind == FI_TCP_OPT_MPTCP)
			break;
	}

	/* not mptcp pkt */
	if (optkind != FI_TCP_OPT_MPTCP)
		return NULL;

	/* get data len */
	if ((leftlen + sizeof(uint32_t) < optlen) ||
		(optlen < FI_MPTCP_DSS_MINLEN))
		return NULL;

	/* need dss type */
	mptcpdss = (struct fi_mptcp_dss *)leftdata;
	if (mptcpdss->subtype != FI_MPTCP_SUBTYPE_DSS)
		return NULL;

	/* subtract dss header len */
	leftdata += sizeof(*mptcpdss);
	leftlen -= sizeof(*mptcpdss);

	/* cal dss len */
	optlen = sizeof(uint32_t) * (mptcpdss->seq8 + mptcpdss->seqpre +
		mptcpdss->ack8 + mptcpdss->ackpre);
	if (leftlen < optlen)
		return NULL;

	*retleftlen = leftlen;
	*retleftdata = leftdata;

	return mptcpdss;
}

/* get ack and seq from mptcp header */
static int fi_parse_mptcp(struct fi_pkt *pktinfo,
	struct tcphdr *tcph, uint32_t hdrlen)
{
	uint32_t leftlen = 0;
	uint8_t *leftdata = NULL;
	uint32_t *seqack = NULL;
	struct fi_mptcp_dss *mptcpdss = NULL;

	mptcpdss = fi_mptcp_getdss(tcph, hdrlen, &leftlen, &leftdata);
	if (mptcpdss == NULL)
		return FI_SUCCESS;

	pktinfo->seq = 0;
	pktinfo->ack = 0;
	pktinfo->mptcp = FI_TRUE;

	/* get ack */
	if (mptcpdss->ackpre && (leftlen >= sizeof(uint32_t))) {
		seqack = (uint32_t *)leftdata;
		pktinfo->ack = ntohl(*seqack);

		leftdata += sizeof(uint32_t);
		leftlen -= sizeof(uint32_t);

		if (mptcpdss->ack8 && (leftlen >= sizeof(uint32_t))) {
			seqack = (uint32_t *)leftdata;
			pktinfo->ack = ntohl(*seqack);

			leftdata += sizeof(uint32_t);
			leftlen -= sizeof(uint32_t);
		}
	}

	/* get seq */
	if (mptcpdss->seqpre && (leftlen >= sizeof(uint32_t))) {
		seqack = (uint32_t *)leftdata;
		pktinfo->seq = ntohl(*seqack);

		leftdata += sizeof(uint32_t);
		leftlen -= sizeof(uint32_t);

		if (mptcpdss->seq8 && (leftlen >= sizeof(uint32_t))) {
			seqack = (uint32_t *)leftdata;
			pktinfo->seq = ntohl(*seqack);
		}
	}

	fi_logd(" : FI mptcp seq=%u ack=%u flow: %u:%u",
		pktinfo->seq, pktinfo->ack, pktinfo->sport, pktinfo->dport);

	return FI_SUCCESS;
}

static int fi_pkt_parse(struct fi_pkt *pktinfo,
	struct sk_buff *skb, int dir)
{
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	struct tcphdr *tcph = NULL;
	unsigned int iphlen; /* ip header len */
	unsigned int l4hlen; /* tcp/udp header len */

	/*
	 * Basic checks have been made above, the message is sure
	 * to be IP message, the length is sure to be more than 20
	 */
	iph = ip_hdr(skb);
	iphlen = iph->ihl * FI_IP_HDR_LEN_BASE;

	if (iph->protocol == FI_IPPROTO_UDP) {
		udph = udp_hdr(skb);
		l4hlen = sizeof(struct udphdr);

		if (skb->len < iphlen + l4hlen + skb->data_len)
			return FI_FAILURE;

		pktinfo->data = (uint8_t *)udph + l4hlen;
		pktinfo->len = skb->len - iphlen - l4hlen;
		pktinfo->bufdatalen = skb->len - skb->data_len -
			iphlen - l4hlen;
		pktinfo->sport = ntohs(udph->source);
		pktinfo->dport = ntohs(udph->dest);
	} else {
		tcph = tcp_hdr(skb);
		l4hlen = tcph->doff * FI_TCP_HDR_LEN_BASE;

		if (skb->len < iphlen + l4hlen + skb->data_len)
			return FI_FAILURE;

		pktinfo->data = (uint8_t *)tcph + l4hlen;
		pktinfo->len = skb->len - iphlen - l4hlen;
		pktinfo->bufdatalen = skb->len - skb->data_len -
			iphlen - l4hlen;
		pktinfo->sport = ntohs(tcph->source);
		pktinfo->dport = ntohs(tcph->dest);
		pktinfo->seq = ntohl(tcph->seq);
		pktinfo->ack = ntohl(tcph->ack_seq);

		if (fi_parse_mptcp(pktinfo, tcph, l4hlen) != FI_SUCCESS)
			return FI_FAILURE;
	}

	/* Focus only on ports larger than 1023 */
	if ((pktinfo->sport < FI_BATTLE_START_PORT_MIN) ||
		(pktinfo->dport < FI_BATTLE_START_PORT_MIN))
		return FI_FAILURE;

	pktinfo->proto = iph->protocol;
	pktinfo->dir = dir;
	pktinfo->sip = ntohl(iph->saddr);
	pktinfo->dip = ntohl(iph->daddr);

	pktinfo->msec = jiffies_to_msecs(jiffies);

	return FI_SUCCESS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
void fi_timer_callback(unsigned long arg)
#else
void fi_timer_callback(struct timer_list *t)
#endif
{
	fi_flow_age();
	fi_rtt_timer();
	if (g_fi_ctx.nf_exist != FI_TRUE) {
		return;
	}
	mod_timer(&g_fi_ctx.tm, jiffies + HZ / FI_TIMER_INTERVAL);
}

static bool fi_streq(const char *data, uint32_t datalen, const char *str)
{
	uint32_t len = strlen(str);
	if (datalen >= len + 1) {
		if (memcmp(data, str, len + 1) == 0)
			return FI_TRUE;
	} else if (datalen == len) {
		if (memcmp(data, str, len) == 0)
			return FI_TRUE;
	} else {
		return FI_FALSE;
	}

	return FI_FALSE;
}

static void fi_appid_add(uint32_t appid, struct smfi_ctx *fictx)
{
	if ((fictx->appidmin == 0) || (appid < fictx->appidmin))
		fictx->appidmin = appid;

	if ((fictx->appidmax == 0) || (appid > fictx->appidmax))
		fictx->appidmax = appid;
}

static void fi_appid_shrink(struct smfi_ctx *fictx)
{
	uint32_t i;

	for (i = fictx->appidmin;
		(i <= fictx->appidmax) && (i < FI_APPID_MAX);
		i++) {
		if (fictx->appinfo[i].valid == FI_FALSE)
			fictx->appidmin = i;
		else
			break;
	}

	for (i = fictx->appidmax; (i >= fictx->appidmin) && (i > 0); i--) {
		if (fictx->appinfo[i].valid == FI_FALSE)
			fictx->appidmax = i;
		else
			break;
	}

	i = fictx->appidmin;
	if ((fictx->appidmin == fictx->appidmax) &&
		(fictx->appinfo[i].valid == FI_FALSE)) {
		fictx->appidmin = 0;
		fictx->appidmax = 0;
	}
}

static uint32_t fi_appname_to_appid(const char *nameptr, uint32_t datalen)
{
	uint32_t appid;

	if (fi_streq(nameptr, datalen, FI_APP_NAME_WZRY)) {
		appid = FI_APPID_WZRY;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_CJZC)) {
		appid = FI_APPID_CJZC;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_QJCJ)) {
		appid = FI_APPID_QJCJ;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_HYXD)) {
		appid = FI_APPID_HYXD;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_HYXD_HW)) {
		appid = FI_APPID_HYXD;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_CYHX)) {
		appid = FI_APPID_CYHX;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_QQFC)) {
		appid = FI_APPID_QQFC;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_BH3) ||
		fi_streq(nameptr, datalen, FI_APP_NAME_BH3_2) ||
		fi_streq(nameptr, datalen, FI_APP_NAME_BH3_3) ||
		fi_streq(nameptr, datalen, FI_APP_NAME_BH3_4) ||
		fi_streq(nameptr, datalen, FI_APP_NAME_BH3_5)) {
		appid = FI_APPID_BH3;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_UU)) {
		appid = FI_APPID_UU;
	} else if (fi_streq(nameptr, datalen, FI_APP_NAME_XUNYOU)) {
		appid = FI_APPID_XUNYOU;
	} else {
		appid = FI_APPID_NULL;
	}

	return appid;
}

static void fi_proc_applaunch(uint8_t *data, uint32_t len, struct smfi_ctx *fictx)
{
	struct fi_msg_applaunch *msg = NULL;
	struct fi_app_info *appinfo = NULL;
	uint32_t appid;

	if ((data == NULL) || (len < sizeof(*msg))) {
		fi_logerr(" : FI illegal data length %d in FI app launch", len);
		return;
	}

	msg = (struct fi_msg_applaunch *)data;
	len -= sizeof(*msg);

	appid = fi_appname_to_appid(msg->appname, len);
	if (!fi_appid_valid(appid)) {
		fi_logerr(" : FI unknown app name %s, len %u",
			msg->appname, len);
		return;
	}

	fi_appid_add(appid, fictx);
	fi_register_nf_hook(fictx);

	appinfo = &(fictx->appinfo[appid]);
	appinfo->appid = appid;
	appinfo->uid = msg->uid;
	appinfo->valid = FI_TRUE;
	appinfo->switches = msg->switches;

	fi_loginfo(" : FI set app info appid=%u uid=%u switch=%08x",
			appinfo->appid, appinfo->uid, appinfo->switches);
}

static bool fi_has_no_appid(struct smfi_ctx *ctx)
{
	if (ctx->appidmin == 0 && ctx->appidmax == 0)
		return true;
	return false;
}

static void fi_proc_appstatus(uint8_t *data, uint32_t len, struct smfi_ctx *fictx)
{
	struct fi_msg_appstatus *msg = NULL;
	struct fi_app_info *appinfo = NULL;
	uint32_t appid;

	if ((data == NULL) || (len < sizeof(*msg))) {
		fi_logerr(" : FI illegal data length %d in FI set app status",
			len);
		return;
	}

	msg = (struct fi_msg_appstatus *)data;
	len -= sizeof(*msg);

	appid = fi_appname_to_appid(msg->appname, len);
	if (!fi_appid_valid(appid)) {
		fi_logerr(" : FI unknown app name %s, len %u",
			msg->appname, len);
		return;
	}

	fi_loginfo(" : FI recv app status appid=%u appstatus=%u",
		appid, msg->appstatus);

	appinfo = &(fictx->appinfo[appid]);
	appinfo->uid = msg->uid;
	appinfo->appstatus = msg->appstatus;

	if (msg->appstatus == GAME_SDK_STATE_DIE) {
		struct fi_gamectx *gamectx = NULL;

		appinfo->valid = FI_FALSE;
		gamectx = fictx->gamectx + appid;

		if (fi_battling(gamectx->appstatus))
			fi_rtt_status(gamectx->appid, FI_STATUS_BATTLE_STOP);

		(void)memset_s(gamectx, sizeof(*gamectx), 0, sizeof(*gamectx));
		fi_appid_shrink(fictx);

		if (fi_has_no_appid(fictx))
			fi_unregister_nf_hook(fictx);

		fi_loginfo(" : FI clear cache because of app exit, uid=%u",
			msg->uid);
	}
}

void fi_reflect_status(int32_t event, uint8_t *data, uint16_t len)
{
	struct fi_msg_appstatus *msg = NULL;
	struct fi_report_status report = {0};
	uint32_t status = 0;

	if ((data == NULL) || (len < sizeof(*msg)))
		return;

	msg = (struct fi_msg_appstatus *)data;
	if (event == NETLINK_EMCOM_DK_SMARTCARE_FI_APP_LAUNCH)
		status = GAME_SDK_STATE_FOREGROUND;
	else if (event == NETLINK_EMCOM_DK_SMARTCARE_FI_APP_STATUS)
		status = msg->appstatus;
	else
		return;

	report.uid = msg->uid;
	report.status = status;
	report.apptype = FI_APP_TYPE_GAME;
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_SMARTCARE_FI_REPORT_APP_STATUS,
		&report, sizeof(report));

	fi_logd(" : FI reflect status to daemon, uid=%u status=%u",
		report.uid, status);
}

static uint32_t fi_get_appid_from_sock(struct sk_buff *skb)
{
	struct sock *sk = NULL;
	struct iphdr *iph = ip_hdr(skb);
	uint32_t appid;
	kuid_t kuid = {0};

	sk = skb->sk;
	if (sk == NULL)
		return 0;

	if ((iph->protocol == FI_IPPROTO_TCP) &&
		(sk->sk_state != TCP_ESTABLISHED))
		return 0;

	kuid = sock_i_uid(sk);
	appid = fi_find_appid(kuid.val, &g_fi_ctx);
	if (!fi_appid_valid(appid))
		return 0;

	return appid;
}

static void fi_hook(struct sk_buff *skb, int dir)
{
	struct fi_flow_head *head = NULL;
	struct fi_flow_node *flow = NULL;
	struct fi_pkt pktinfo = {0};
	uint32_t hash;
	uint32_t appid = 0;
	uint32_t addflow = FI_FALSE;

	if (unlikely(skb == NULL))
		return;

	/* Filter out ipv4 udp&tcp */
	if (fi_pkt_check(skb, dir) != FI_SUCCESS)
		return;

	if (dir == SMFI_DIR_UP) {
		appid = fi_get_appid_from_sock(skb);
		if (appid == 0)
			return;
		addflow = FI_TRUE;
	}

	if (fi_pkt_parse(&pktinfo, skb, dir) != FI_SUCCESS)
		return;

	hash = fi_flow_hash(pktinfo.sip, pktinfo.dip, pktinfo.sport,
				pktinfo.dport, pktinfo.proto);
	head = fi_flow_header(hash);

	fi_flow_lock();
	flow = fi_flow_get(&pktinfo, head, addflow);
	if (flow == NULL) {
		fi_flow_unlock();
		return;
	}

	if (dir == SMFI_DIR_UP)
		flow->flowctx.appid = appid;
	else
		appid = flow->flowctx.appid;

	fi_rtt_entrance(&pktinfo, &(flow->flowctx), appid);
	fi_flow_unlock();
}

unsigned int fi_hook_out(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	fi_hook(skb, SMFI_DIR_UP);
	return NF_ACCEPT;
}

unsigned int fi_hook_in(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	fi_hook(skb, SMFI_DIR_DOWN);
	return NF_ACCEPT;
}

void fi_para_init(struct smfi_ctx *fictx)
{
	fi_flow_init();
	(void)memset_s(fictx, sizeof(*fictx), 0, sizeof(*fictx));
	mutex_init(&(fictx->appinfo_lock));
	mutex_init(&(fictx->nf_mutex));
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
	init_timer(&(fictx->tm));
	fictx->tm.function = fi_timer_callback;
	fictx->tm.data = (uintptr_t)FI_TIMER_STR;
#else
	timer_setup(&(fictx->tm), fi_timer_callback, 0);
#endif
}

void smfi_event_process(int32_t event, uint8_t *pdata, uint16_t len)
{
	struct smfi_ctx *fictx = &g_fi_ctx;

	fi_logd(" : FI received cmd=%d datalen=%u", event, len);

	switch (event) {
	case NETLINK_EMCOM_DK_SMARTCARE_FI_APP_LAUNCH:
		fi_proc_applaunch(pdata, len, fictx);
		break;

	case NETLINK_EMCOM_DK_SMARTCARE_FI_APP_STATUS:
		fi_proc_appstatus(pdata, len, fictx);
		break;

	default:
		fi_logerr(" : FI received unsupported message");
		break;
	}

	fi_reflect_status(event, pdata, len);
}
EXPORT_SYMBOL(smfi_event_process); /*lint !e580*/

void smfi_init(void)
{
	fi_para_init(&g_fi_ctx);
	fi_loginfo(" : FI init kernel module successfully");
}
EXPORT_SYMBOL(smfi_init); /*lint !e580*/

void smfi_deinit(void)
{
	fi_unregister_nf_hook(&g_fi_ctx);
	fi_flow_clear();
	fi_loginfo(" : FI deinit kernel module successfully");
}
EXPORT_SYMBOL(smfi_deinit); /*lint !e580*/
