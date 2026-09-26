/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mpflow module implemention
 * Author: xialiang xialiang4@huawei.com
 * Create: 2020-06-08
 */
#include <asm/uaccess.h>
#include <linux/fdtable.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/version.h>
#include <net/inet_hashtables.h>
#include <net/inet6_hashtables.h>
#ifdef CONFIG_MPTCP
#include <net/mptcp.h>
#endif
#include <net/pkt_sched.h>
#include <net/sch_generic.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/udp.h>
#ifdef CONFIG_HUAWEI_BASTET
#include "huawei_platform/net/bastet/bastet_utils.h"
#endif
#include "securec.h"
#include "../emcom_netlink.h"
#include "../emcom_utils.h"
#include "emcom/emcom_mpdns.h"
#include "emcom/emcom_xengine.h"
#include "emcom/network_evaluation.h"
#ifdef CONFIG_HUAWEI_OPMP
#include "emcom/opmp_heartbeat.h"
#endif

#undef HWLOG_TAG
#define HWLOG_TAG emcom_mpflow
HWLOG_REGIST();

#define EMCOM_GOOD_RECV_RATE_THR_BYTE_PER_SEC 400000
#define EMCOM_GOOD_RTT_THR_MS 120

static spinlock_t g_mpflow_lock;
struct emcom_xengine_mpflow_info g_mpflow_uids[EMCOM_MPFLOW_MAX_APP];
static uint8_t g_mpflow_index;
static bool g_mpflow_tm_running;

static struct timer_list g_mpflow_tm;
static unsigned int g_mpflow_nf_hook = 0;

struct emcom_xengine_mpflow_stat g_mpflow_list[EMCOM_MPFLOW_MAX_LIST_NUM];
extern spinlock_t g_mpflow_ai_lock;
extern struct emcom_xengine_mpflow_ai_info g_mpflow_ai_uids[EMCOM_MPFLOW_AI_MAX_APP];
extern struct emcom_xengine_mpflow_stat g_mpflow_ai_list[EMCOM_MPFLOW_AI_MAX_LIST_NUM];

static unsigned int emcom_xengine_mpflow_hook_out_ipv4(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state);
static unsigned int emcom_xengine_mpflow_hook_out_ipv6(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state);
void emcom_mpflow_check_mpdns_addr(struct sock *sk, int reason, int result);

static inline bool invalid_uid(uid_t uid)
{
	/* if uid less than 10000, it is not an Android apk */
	return (uid < UID_APP);
}

char *strtok(char *string_org, const char *demial)
{
#define EMCOM_MPFLOW_FI_CHAR_MAP_NUM  32
#define EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT  3
#define EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK  7
	static unsigned char *last;
	unsigned char *str = NULL;
	const unsigned char *ctrl = (const unsigned char *)demial;
	unsigned char map[EMCOM_MPFLOW_FI_CHAR_MAP_NUM];
	int count;

	for (count = 0; count < EMCOM_MPFLOW_FI_CHAR_MAP_NUM; count++)
		map[count] = 0;

	do
		map[*ctrl >> EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT] |=
			(1 << (*ctrl & EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK));
	while (*ctrl++);

	if (string_org != NULL)
		str = (unsigned char *)string_org;
	else
		str = last;

	while ((map[*str >> EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT] &
		(1 << (*str & EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK))) && *str)
		str++;

	string_org = (char *)str;
	for (; *str; str++) {
		if (map[*str >> EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT] &
			(1 << (*str & EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK))) {
			*str++ = '\0';
			break;
		}
	}
	last = str;
	if (string_org == (char *)str)
		return NULL;
	else
		return string_org;

#undef EMCOM_MPFLOW_FI_CHAR_MAP_NUM
#undef EMCOM_MPFLOW_FI_CHAR_MAP_HIGH_BITS_SHIFT
#undef EMCOM_MPFLOW_FI_CHAR_MAP_LOW_BITS_MASK
}

static const struct nf_hook_ops emcom_xengine_mpflow_nfhooks[] = {
	{
		.hook        = emcom_xengine_mpflow_hook_out_ipv4,
		.pf          = PF_INET,
		.hooknum     = NF_INET_LOCAL_OUT,
		.priority    = NF_IP_PRI_FILTER + 1,
	},
	{
		.hook        = emcom_xengine_mpflow_hook_out_ipv6,
		.pf          = PF_INET6,
		.hooknum     = NF_INET_LOCAL_OUT,
		.priority    = NF_IP_PRI_FILTER + 1,
	},
};

static uint8_t *emcom_xengine_mpflow_make_skip(const uint8_t *ptrn, uint8_t plen)
{
	int i;
	uint8_t *skip = NULL;

	skip = (uint8_t *)kmalloc(EMCOM_MPFLOW_FI_ASCII_CODE_SIZE *
		 sizeof(uint8_t), GFP_ATOMIC);
	if (!skip)
		return NULL;

	for (i = 0; i < EMCOM_MPFLOW_FI_ASCII_CODE_SIZE; i++)
		*(skip + i) = (plen >= EMCOM_MPFLOW_FI_ASCII_CODE_MARK) ?
			EMCOM_MPFLOW_FI_ASCII_CODE_MARK : (plen + 1);
	while (plen != 0)
		*(skip + (uint8_t)*ptrn++) = plen--;
	return skip;
}

static uint8_t *emcom_xengine_mpflow_make_shift(const uint8_t *ptrn, uint8_t plen)
{
	uint8_t *sptr = NULL;
	const uint8_t *pptr = NULL;
	uint8_t c;
	uint8_t *shift = NULL;
	const uint8_t *p1 = NULL;
	const uint8_t *p2 = NULL;
	const uint8_t *p3 = NULL;

	shift = (uint8_t *)kmalloc(plen * sizeof(uint8_t), GFP_ATOMIC);
	if (!shift)
		return NULL;
	sptr = shift + plen - 1;
	pptr = ptrn + plen - 1;
	c = *(ptrn + plen - 1);
	*sptr = 1;

	while (sptr-- != shift) {
		p1 = ptrn + (plen - 1) - 1;
		do {
			while ((p1 >= ptrn) && (*p1-- != c))
				;
			p2 = ptrn + (plen - 1) - 1;
			p3 = p1;
			while ((p3 >= ptrn) && (*p3-- == *p2--) &&
				(p2 >= pptr))
				;
		} while ((p3 >= ptrn) && (p2 >= pptr));
		*sptr = shift + plen - sptr + p2 - p3;
		pptr--;
	}
	return shift;
}

static bool emcom_xengine_mpflow_bm_build(const uint8_t *ptn, uint32_t ptnlen,
	uint8_t **skip, uint8_t **shift)
{
	if ((ptn != NULL) && (ptnlen > 0) && (skip != NULL) && (shift != NULL)) {
		*skip = emcom_xengine_mpflow_make_skip(ptn, ptnlen);
		if (!*skip)
			return false;
		*shift = emcom_xengine_mpflow_make_shift(ptn, ptnlen);
		if (!*shift) {
			kfree(*skip);
			*skip = NULL;
			return false;
		}
		return true;
	}
	return false;
}


static int emcom_xengine_mpflow_split(char *src, const char *separator, char **dest, int *num)
{
	char *p = NULL;
	int count = 0;

	if ((src == NULL) || (strlen(src) == 0) || (separator == NULL) || (strlen(separator) == 0))
		return *num;

	p = strtok(src, separator);
	while (p != NULL) {
		*dest++ = p;
		++count;
		p = strtok(NULL, separator);
	}
	*num = count;
	return *num;
}

static void emcom_xengine_mpflow_hash_clear(const struct emcom_xengine_mpflow_app_priv *priv)
{
	int i;
	struct emcom_xengine_mpflow_ip *ip = NULL;
	struct hlist_node *tmp = NULL;

	for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++) {
		hlist_for_each_entry_safe(ip, tmp, &priv->hashtable[i], node) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}
}

void emcom_xengine_mpflow_register_nf_hook(uint8_t ver)
{
	int ret;

	if (g_mpflow_nf_hook == 0) {
		ret = nf_register_net_hooks(&init_net, emcom_xengine_mpflow_nfhooks, ARRAY_SIZE(emcom_xengine_mpflow_nfhooks));
		if (ret == 0) {
			g_mpflow_nf_hook |= (1 << ver);
			emcom_logd("start emcom_xengine_mpflow_nfhooks");
		}
	} else {
		g_mpflow_nf_hook |= (1 << ver);
	}
}

void emcom_xengine_mpflow_unregister_nf_hook(uint8_t ver)
{
	if (g_mpflow_nf_hook == 0)
		return;
	g_mpflow_nf_hook &= (~(1 << ver)); /*lint !e502*/
	if (g_mpflow_nf_hook > 0)
		return;

	nf_unregister_net_hooks(&init_net, emcom_xengine_mpflow_nfhooks, ARRAY_SIZE(emcom_xengine_mpflow_nfhooks));
	emcom_logd("stop emcom_xengine_mpflow_nfhooks");
}

static void emcom_xengine_mpflow_bm_free(uint8_t **skip, uint8_t **shift)
{
	if (*skip) {
		kfree(*skip);
		*skip = NULL;
	}
	if (*shift) {
		kfree(*shift);
		*shift = NULL;
	}
}

static void emcom_xengine_mpflow_apppriv_deinit(struct emcom_xengine_mpflow_info *uid)
{
	if (!uid->app_priv)
		return;

	emcom_xengine_mpflow_hash_clear(uid->app_priv);
	kfree(uid->app_priv);
	uid->app_priv = NULL;
}

static void emcom_xengine_mpflow_fi_init(struct emcom_xengine_mpflow_info *mpflow_uid)
{
	errno_t err;

	err = memset_s(&mpflow_uid->wifi, sizeof(struct emcom_xengine_mpflow_iface),
				   0, sizeof(struct emcom_xengine_mpflow_iface));
	if (err != EOK)
		emcom_loge("emcom_xengine_mpflow_fi_init memset failed");
	mpflow_uid->wifi.is_wifi = 1;
	err = memset_s(&mpflow_uid->lte, sizeof(struct emcom_xengine_mpflow_iface),
				   0, sizeof(struct emcom_xengine_mpflow_iface));
	if (err != EOK)
		emcom_loge("emcom_xengine_mpflow_fi_init memset failed");
	mpflow_uid->lte.is_wifi = 0;
	INIT_LIST_HEAD(&mpflow_uid->wifi.flows);
	INIT_LIST_HEAD(&mpflow_uid->lte.flows);
	mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	mpflow_uid->rst_jiffies = 0;
	mpflow_uid->is_downloading = 0;
	mpflow_uid->rst_to_another = 0;
}

static void emcom_xengine_mpflow_ptn_deinit(struct emcom_xengine_mpflow_ptn ptn[], uint8_t num)
{
	uint8_t i;

	for (i = 0; i < num; i++) {
		if (ptn[i].is_init) {
			emcom_xengine_mpflow_bm_free(&(ptn[i].skip), &(ptn[i].shift));
			ptn[i].is_init = false;
		}
	}
}

static void emcom_xengine_mpflow_download_flow_del(struct emcom_xengine_mpflow_iface *iface,
	struct emcom_xengine_mpflow_node *flow)
{
	struct sock *sk = (struct sock *)flow->tp;

	emcom_logd("remove sk: %pK sport: %u srtt_us: %u bytes_received: %ju duration: %u", flow->tp, sk->sk_num,
				flow->tp->srtt_us >> EMCOM_MPFLOW_SHIFT_THREE, flow->tp->bytes_received,
				jiffies_to_msecs(jiffies - flow->start_jiffies));
	sock_put(sk);
	list_del(&flow->list);
	kfree(flow);
	iface->flow_cnt--;
}

static bool emcom_xengine_mpflow_uid_empty(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid != UID_INVALID_APP)
			return false;
	}

	return true;
}

static bool emcom_xengine_mpflow_ptn_init(struct emcom_xengine_mpflow_ptn ptn[], uint8_t *num, const char *hex)
{
	bool ret = false;
	char *revbuf[EMCOM_MPFLOW_FI_PTN_MAX_NUM] = {0};
	int n = 0;
	int i;
	errno_t err;

	emcom_logd("hex %s\n", hex);
	if (emcom_xengine_mpflow_split((char *)hex, EMCOM_MPFLOW_FI_PTN_SEPERATE_CHAR, revbuf, &n) == 0) {
		emcom_loge("hex split fail\n");
		return false;
	}

	for (i = 0; i < n; i++) {
		ptn[i].skip = NULL;
		ptn[i].shift = NULL;
		err = memset_s(ptn[i].ptn, EMCOM_MPFLOW_FI_PTN_MAX_SIZE, 0, EMCOM_MPFLOW_FI_PTN_MAX_SIZE);
		if (err != EOK) {
			emcom_loge("emcom_xengine_mpflow_ptn_init memset failed!");
			ptn[i].is_init = false;
			return false;
		}
		ptn[i].ptnlen = strnlen(revbuf[i], EMCOM_MPFLOW_FI_PTN_MAX_SIZE) >> 1;
		if (hex2bin(ptn[i].ptn, revbuf[i], ptn[i].ptnlen)) {
			ptn[i].is_init = false;
			emcom_loge("hex2bin fail\n");
			return false;
		}
		ret = emcom_xengine_mpflow_bm_build((const uint8_t *)ptn[i].ptn, ptn[i].ptnlen,
											&(ptn[i].skip), &(ptn[i].shift));
		if (!ret) {
			emcom_loge("emcom_xengine_mpflow_bm_build failed!\n");
			ptn[i].is_init = false;
			return false;
		}
		ptn[i].is_init = true;
		emcom_logd("ptn %s init succ!\n", ptn[i].ptn);
	}
	*num = n;
	return true;
}

static bool emcom_xengine_mpflow_fi_start(bool is_new_uid_enable, uint8_t index, bool *ret,
	struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo)
{
	if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO ||
		g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI) {
		if (is_new_uid_enable)
			emcom_xengine_mpflow_fi_init(&g_mpflow_uids[index]);
		*ret = true;
		return true;
	}
	if (is_new_uid_enable) {
		int i;
		struct emcom_xengine_mpflow_app_priv *app_priv = NULL;

		emcom_xengine_mpflow_fi_init(&g_mpflow_uids[index]);
		app_priv = kzalloc(sizeof(struct emcom_xengine_mpflow_app_priv), GFP_ATOMIC);
		if (!app_priv)
			return false;

		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
			if (g_mpflow_uids[index].reserve_field & EMCOM_MPFLOW_ENABLEFLAG_LTE_FIRST) {
				app_priv->lte_thresh = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH;
				app_priv->lte_thresh_max = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX;
				app_priv->lte_thresh_min = EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MIN;
				app_priv->lte_first = 1;
			} else {
				app_priv->lte_thresh = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH;
				app_priv->lte_thresh_max = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MAX;
				app_priv->lte_thresh_min = EMCOM_MPFLOW_WIFI_FIRST_LTE_THREH_MIN;
				app_priv->lte_first = 0;
			}
		}

		for (i = 0; i < EMCOM_MPFLOW_HASH_SIZE; i++)
			INIT_HLIST_HEAD(&app_priv->hashtable[i]);

		g_mpflow_uids[index].app_priv = app_priv;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_80, &(g_mpflow_uids[index].ptn_80_num),
			mpflowstartinfo->ptn_80))
			*ret = true;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_443, &(g_mpflow_uids[index].ptn_443_num),
			mpflowstartinfo->ptn_443))
			*ret = true;
	} else {
		emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
		emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_80, &(g_mpflow_uids[index].ptn_80_num),
			mpflowstartinfo->ptn_80))
			*ret = true;
		if (emcom_xengine_mpflow_ptn_init(g_mpflow_uids[index].ptn_443, &(g_mpflow_uids[index].ptn_443_num),
			mpflowstartinfo->ptn_443))
			*ret = true;
	}
	return true;
}

uint8_t emcom_xengine_mpflow_ip_hash(__be32 addr)
{
#define EMCOM_MPFLOW_HASH_BIT_FOUR 4
#define EMCOM_MPFLOW_HASH_BIT_EIGHT (4 * 2)
#define EMCOM_MPFLOW_HASH_BIT_TWELVE (4 * 3)
#define EMCOM_MPFLOW_HASH_BIT_SIXTEEN (4 * 4)

	uint32_t hash;
	hash = (addr) << EMCOM_MPFLOW_HASH_BIT_EIGHT;
	hash ^= (addr) >> EMCOM_MPFLOW_HASH_BIT_FOUR;
	hash ^= (addr) >> EMCOM_MPFLOW_HASH_BIT_TWELVE;
	hash ^= (addr) >> EMCOM_MPFLOW_HASH_BIT_SIXTEEN;

#undef EMCOM_MPFLOW_HASH_BIT_FOUR
#undef EMCOM_MPFLOW_HASH_BIT_EIGHT
#undef EMCOM_MPFLOW_HASH_BIT_TWELVE
#undef EMCOM_MPFLOW_HASH_BIT_SIXTEEN
	return (uint8_t)(hash & (EMCOM_MPFLOW_HASH_SIZE - 1));
}

static struct emcom_xengine_mpflow_ip *emcom_xengine_mpflow_hash(__be32 addr,
	struct hlist_head *hashtable, uint32_t algorithm_type)
{
	struct emcom_xengine_mpflow_ip *ip = NULL;
	struct hlist_node *tmp = NULL;
	uint8_t hash;
	unsigned long aging;

	if (algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK)
		aging = EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH;
	else
		aging = EMCOM_MPFLOW_IP_AGING_THREH;

	hash = emcom_xengine_mpflow_ip_hash(addr);

	hlist_for_each_entry_safe(ip, tmp, &hashtable[hash], node) {
		if (ip->addr == addr)
			return ip;

		/* free too old entrys */
		if ((jiffies - ip->jiffies[(ip->tot_cnt - 1) % EMCOM_MPFLOW_FLOW_JIFFIES_REC]) >
			aging) {
			hlist_del(&ip->node);
			kfree(ip);
		}
	}

	ip = kzalloc(sizeof(struct emcom_xengine_mpflow_ip), GFP_ATOMIC);
	if (!ip)
		return NULL;

	ip->addr = addr;
	hlist_add_head(&ip->node, &hashtable[hash]);
	return ip;
}

static bool emcom_xengine_mpflow_ip_chk_bind_lte(
	struct emcom_xengine_mpflow_app_priv *priv,
	struct emcom_xengine_mpflow_ip *ip,
	uint32_t type)
{
	if (type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
		bool lte_first = priv->lte_first ? true : false;
		uint8_t index = ip->tot_cnt % EMCOM_MPFLOW_FLOW_JIFFIES_REC;
		unsigned long now = jiffies;

		if (ip->tot_cnt < EMCOM_MPFLOW_FLOW_JIFFIES_REC)
			return lte_first;

	/*lint -e666*/
		if (time_before(now, (ip->jiffies[index] + EMCOM_MPFLOW_NETDISK_DOWNLOAD_THREH))) {
			/* if (lte_cnt/tot_cnt < lte_thresh/10), then we need bind on lte */
			if ((ip->lte_cnt * EMCOM_MPFLOW_FI_NETDISK_FLOW_NUM) < (priv->lte_thresh * ip->tot_cnt))
				return true;
			else
				return false;
		}
	/*lint +e666*/

		return lte_first;
	} else {
		if (ip->tot_cnt % EMCOM_MPFLOW_DEV_NUM)
			return true;
		else
			return false;
	}
}

/* start index of ipv4 address which is mapped into ipv6 address */
#define EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX 3

static bool emcom_xengine_mpflow_get_addr_port(struct sockaddr *addr, __be32 *s_addr, uint16_t *port)
{
	if (addr->sa_family == AF_INET) {
		struct sockaddr_in *usin = (struct sockaddr_in *)addr;
		*s_addr = usin->sin_addr.s_addr;
		*port = ntohs(usin->sin_port);
		return true;
	}
#if IS_ENABLED(CONFIG_IPV6)
	else if (addr->sa_family == AF_INET6) {
		struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)addr;

		if (!ipv6_addr_v4mapped(&usin6->sin6_addr))
			return false;
		*s_addr = usin6->sin6_addr.s6_addr32[EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX];
		*port = ntohs(usin6->sin6_port);
		return true;
	}
#endif
	else {
		return false;
	}
}

#undef EMCOM_MPFLOW_FI_CLAT_IPV4_INDEX

static int emcom_xengine_mpflow_ip_bind(struct sockaddr *addr,
	const struct emcom_xengine_mpflow_info *uid)
{
	__be32 daddr;
	uint16_t dport;
	struct emcom_xengine_mpflow_ip *ip = NULL;
	int bind_dev;
	struct emcom_xengine_mpflow_app_priv *priv = uid->app_priv;
	uint32_t type = uid->algorithm_type;

	if (!emcom_xengine_mpflow_get_addr_port(addr, &daddr, &dport))
		return EMCOM_MPFLOW_BIND_WIFI;

	if ((dport == EMCOM_MPFLOW_FI_PORT_443) && !uid->ptn_443_num)
		return EMCOM_MPFLOW_BIND_WIFI;

	if ((dport == EMCOM_MPFLOW_FI_PORT_80) && !uid->ptn_80_num)
		return EMCOM_MPFLOW_BIND_WIFI;

	ip = emcom_xengine_mpflow_hash(daddr, priv->hashtable, type);
	if (!ip)
		return EMCOM_MPFLOW_BIND_WIFI;

	if (emcom_xengine_mpflow_ip_chk_bind_lte(priv, ip, type)) {
		ip->lte_cnt++;
		bind_dev = EMCOM_MPFLOW_BIND_LTE;
	} else {
		bind_dev = EMCOM_MPFLOW_BIND_WIFI;
	}
	ip->jiffies[ip->tot_cnt % EMCOM_MPFLOW_FLOW_JIFFIES_REC] = jiffies;
	ip->tot_cnt++;
	return bind_dev;
}

static void emcom_xengine_mpflow_netdisk_lte_thresh(struct emcom_xengine_mpflow_app_priv *priv, int add)
{
	if ((add > 0) && (priv->lte_thresh < priv->lte_thresh_max))
		priv->lte_thresh++;
	else if ((add < 0) && (priv->lte_thresh > priv->lte_thresh_min))
		priv->lte_thresh--;
}

static void emcom_xengine_mpflow_download_finish(const struct emcom_xengine_mpflow_info *uid)
{
#define EMCOM_MPFLOW_FI_RATE_RATIO_NUMERATOR 4
#define EMCOM_MPFLOW_FI_RATE_RATIO_DENOMINATOR 5
	struct emcom_xengine_mpflow_app_priv *priv = uid->app_priv;

	if (!priv)
		return;

	if (uid->algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
		int add = 0;

		emcom_logd("lte %u %u, wifi %u %u",
				   uid->lte.max_rate_received_flow,
				   uid->lte.bytes_received,
				   uid->wifi.max_rate_received_flow,
				   uid->wifi.bytes_received);

		if ((uid->lte.bytes_received + uid->wifi.bytes_received) < EMCOM_MPFLOW_LTE_THREH_ADJUST_BYTES)
			return;

		if (uid->lte.max_rate_received_flow && uid->wifi.max_rate_received_flow &&
			(uid->lte.max_rate_received_flow < uid->wifi.max_rate_received_flow)) {
			/* avg_speed_per_flow_LTE< avg_speed_per_flow_WIFI, lte_thresh-- */
			emcom_logd("case 1: lte_thresh--");
			add--;
		} else if ((uid->lte.max_rate_received_flow > EMCOM_MPFLOW_NETDISK_RATE_GOOD) &&
			((EMCOM_MPFLOW_FI_RATE_RATIO_DENOMINATOR * uid->wifi.max_rate_received_flow) <
			(EMCOM_MPFLOW_FI_RATE_RATIO_NUMERATOR * uid->lte.max_rate_received_flow))) {
			/* avg_speed_per_flow_LTE>1M && avg_speed_per_flow_WIFI < 0.8* avg_speed_per_flow_LTE, lte_thresh++ */
			emcom_logd("case 2: lte_thresh++");
			add++;
		} else if (!priv->lte_thresh && (uid->wifi.max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD)) {
			emcom_logd("case 3: lte_thresh++");
			add++;
		} else if ((priv->lte_thresh == EMCOM_MPFLOW_LTE_FIRST_LTE_THREH_MAX) &&
			(uid->lte.max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD)) {
			emcom_logd("case 4: lte_thresh--");
			add--;
		}

		emcom_xengine_mpflow_netdisk_lte_thresh(priv, add);
	}

	emcom_xengine_mpflow_hash_clear(priv);
#undef EMCOM_MPFLOW_FI_RATE_RATIO_NUMERATOR
#undef EMCOM_MPFLOW_FI_RATE_RATIO_DENOMINATOR
}

static bool emcom_xengine_get_mpflowlist(uint8_t ver, struct emcom_xengine_mpflow_stat **mpflow_list)
{
	switch (ver) {
	case EMCOM_MPFLOW_VER_V1:
		*mpflow_list = g_mpflow_list;
		break;
	case EMCOM_MPFLOW_VER_V2:
		*mpflow_list = g_mpflow_ai_list;
		break;
	default:
		emcom_loge("get mpflowlist failed.Version %u not supported", ver);
		return false;
	}

	return true;
}

void emcom_xengine_mpflow_reset(struct tcp_sock *tp)
{
	struct sock *sk = (struct sock *)tp;

	emcom_logd("reset sk %pK sport is %u, state[%u]", tp, sk->sk_num, sk->sk_state);
	if (sk->sk_state == TCP_ESTABLISHED || sk->sk_state == TCP_SYN_SENT) {
		local_bh_disable();
		bh_lock_sock(sk);

		if (!sock_flag(sk, SOCK_DEAD)) {
			sk->sk_err = ECONNABORTED;
			/* This barrier is coupled with smp_rmb() in tcp_poll() */
			smp_wmb();
			sk->sk_error_report(sk);
			if (tcp_need_reset(sk->sk_state))
				tcp_send_active_reset(sk, sk->sk_allocation);
			tcp_done(sk);
		}

		bh_unlock_sock(sk);
		local_bh_enable();
	}
}

static void emcom_xengine_mpflow_update_wifi_pri(struct emcom_xengine_mpflow_iface *flows,
	const struct emcom_xengine_mpflow_info *mpflow_uid, u16 flow_cnt)
{
	int i;
	if (mpflow_uid->algorithm_type != EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI)
		return;

	flows->is_slow = 1;    // initialize the interface as slow
	flows->is_sure_no_slow = 0;    // but not sure the interface is actually slow

	if (flow_cnt <= 1) {
		flows->is_slow = 0;  // no active flow, reconsider the interface as fast but not sure
	} else if (flows->mean_srtt_ms < EMCOM_GOOD_RTT_THR_MS) {  // low rtt, iface surely fast
		flows->is_slow = 0;
		flows->is_sure_no_slow = 1;
	}

	if (flows->rate_received[0] == 0 &&
		flows->rate_received[1] == 0 &&
		flows->rate_received[2] == 0)
		flows->is_slow = 0;  // no bytes recieved for 3s, reconsider the iface as fast but not sure

	for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i >= 0; i--) {
		if (flows->rate_received[i] > EMCOM_GOOD_RECV_RATE_THR_BYTE_PER_SEC) {
			flows->is_slow = 0;  // recieve rate is more than xx KBps, interface is surely fast
			flows->is_sure_no_slow = 1;
			break;
		}
	}
}

static void emcom_xengine_mpflow_update(struct emcom_xengine_mpflow_iface *flows,
	const struct emcom_xengine_mpflow_info *mpflow_uid)
{
#define EMCOM_MPFLOW_MICROSECOND_SMOOTH_RATE   8
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	struct sock *sk = NULL;
	u32 srtt_ms_sum = 0;
	u32 max_rate_received = 0;
	u16 flow_cnt = 0;
	int i;

	flows->is_slow = 1;
	flows->flow_valid_cnt = 0;
	for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i > 0; i--)
		flows->rate_received[i] = flows->rate_received[i - 1];
	flows->rate_received[0] = 0;
	list_for_each_entry_safe(node, tmp, &flows->flows, list) {
		sk = (struct sock *)node->tp;
		/* now update interval is 1s, rcv_bytes is the rate */
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i > 0; i--)
			node->rate_received[i] = node->rate_received[i - 1];
		node->rate_received[0] = (u32)(node->tp->bytes_received - node->last_bytes_received);
		/* srtt_us is smoothed round trip time << 3 in usecs */
		srtt_ms_sum += (node->tp->srtt_us / EMCOM_MPFLOW_MICROSECOND_SMOOTH_RATE) /
			USEC_PER_MSEC;
		flow_cnt++;
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i >= 0; i--) {
			if (node->rate_received[i] > EMCOM_MPFLOW_FLOW_SLOW_THREH) {
				flows->is_slow = 0;
				break;
			}
		}
		if (node->tp->bytes_received > EMCOM_MPFLOW_RATE_VALID_THREH)
			flows->flow_valid_cnt++;
		max_rate_received += node->rate_received[0];
		flows->bytes_received += (u32)(node->tp->bytes_received - node->last_bytes_received);
		flows->rate_received[0] += (u32)(node->tp->bytes_received - node->last_bytes_received);
		node->last_bytes_received = node->tp->bytes_received;
		if (sk->sk_state != TCP_ESTABLISHED)
			emcom_xengine_mpflow_download_flow_del(flows, node);
	}
	if (flow_cnt > 0)
		flows->mean_srtt_ms = srtt_ms_sum / flow_cnt;

	emcom_xengine_mpflow_update_wifi_pri(flows, mpflow_uid, flow_cnt);

	if (flows->flow_valid_cnt) {
		if (flows->max_rate_received_flow < max_rate_received / flows->flow_valid_cnt)
			flows->max_rate_received_flow = max_rate_received / flows->flow_valid_cnt;
	}

	if (flows->max_rate_received < max_rate_received)
		flows->max_rate_received = max_rate_received;
#undef EMCOM_MPFLOW_MICROSECOND_SMOOTH_RATE
}

static void emcom_xengine_mpflow_set_bind(struct emcom_xengine_mpflow_info *mpflow_uid, int bind_mode)
{
	switch (bind_mode) {
	case EMCOM_XENGINE_MPFLOW_BINDMODE_NORST:
		return;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST:
		mpflow_uid->rst_bind_mode = (mpflow_uid->wifi.max_rate_received >= mpflow_uid->lte.max_rate_received) ?
			EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI : EMCOM_XENGINE_MPFLOW_BINDMODE_LTE;
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI:
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI;
		break;
	case EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE:
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_LTE;
		break;
	default:
		return;
	}

	mpflow_uid->rst_jiffies = jiffies;
	emcom_logd("uid %u rst_bind_mode %u rst_jiffies %lu", mpflow_uid->uid,
			   mpflow_uid->rst_bind_mode, mpflow_uid->rst_jiffies);
}

static bool emcom_xengine_mpflow_unbalance_netdisk(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (iface1->max_rate_received && (iface1->max_rate_received < (iface2->max_rate_received >> 1)) &&
		iface2->is_slow && (iface1->max_rate_received_flow < EMCOM_MPFLOW_NETDISK_RATE_BAD) &&
		((iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		(!iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)))
		return true;
	else
		return false;
}

static bool emcom_xengine_mpflow_unbalance_market(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (iface1->max_rate_received &&
		((iface1->is_wifi && iface2->is_slow && (iface1->max_rate_received < (iface2->max_rate_received >> 1)) &&
		((g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM))) ||
		(!iface1->is_wifi && (iface1->max_rate_received < iface2->max_rate_received_flow) &&
		((g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)))))
		return true;
	else
		return false;
}

static bool emcom_xengine_mpflow_single_path(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	if (!g_mpflow_uids[index].rst_to_another && (iface1->flow_valid_cnt > 1) && !iface2->max_rate_received_flow &&
		((iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		(!iface1->is_wifi && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)))
		return true;
	else
		return false;
}

static int emcom_xengine_mpflow_chk_rst_market(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;

	list_for_each_entry_safe(node, tmp, &iface1->flows, list) {
	/*lint -e666*/
		if (time_before(jiffies, (node->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH)))
			continue;
	/*lint +e666*/

		/* all downloading flows are on same iface */
		if ((node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH) &&
			emcom_xengine_mpflow_single_path(iface1, iface2, index)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = iface1->is_wifi ? EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE :
				EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
			g_mpflow_uids[index].rst_to_another = 1;
			break;
		}
		if ((iface1->bytes_received + iface2->bytes_received) <= EMCOM_MPFLOW_RST_IFACE_GOOD)
			break;
		/* iface1 is slower than half of iface2 */
		if (iface1->max_rate_received && (iface1->max_rate_received < (iface2->max_rate_received >> 1)))
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		/* both wifi and lte download over */
		if (iface1->is_slow && (iface2->is_slow || !iface2->bytes_received)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
		/* wifi is slower than half of lte or lte is slower than wifi */
		if ((node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH) &&
			emcom_xengine_mpflow_unbalance_market(iface1, iface2, index)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
	}
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_netdisk(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2, uint8_t index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;

	list_for_each_entry_safe(node, tmp, &iface1->flows, list) {
	/*lint -e666*/
		if (time_before(jiffies, (node->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH)) ||
			((iface1->bytes_received + iface2->bytes_received) <= EMCOM_MPFLOW_RST_IFACE_GOOD))
			continue;
	/*lint +e666*/

		/* iface1 is slower than half of iface2 */
		if (iface1->max_rate_received &&
			(iface1->max_rate_received < (iface2->max_rate_received >> 1)))
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		/* both wifi and lte download over */
		if (iface1->is_slow && (iface2->is_slow || !iface2->bytes_received)) {
			emcom_xengine_mpflow_reset(node->tp);
			need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2FAST;
		}
		if ((node->last_bytes_received > EMCOM_MPFLOW_RST_RCV_BYTES_THREH) &&
			emcom_xengine_mpflow_unbalance_netdisk(iface1, iface2, index))
			emcom_xengine_mpflow_reset(node->tp);
	}
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_weibo(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2)
{
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	uint32_t wifi_download_time;
	uint32_t lte_download_time;

	/*lint -e666*/
	if (!iface1->max_rate_received_flow || !iface2->max_rate_received_flow ||
		time_before(jiffies, (iface1->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH)) ||
		time_before(jiffies, (iface2->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH)))
		return need_set_bind;
	/*lint +e666*/
	wifi_download_time = MSEC_PER_SEC * EMCOM_MPFLOW_WEIBO_SIZE / iface1->max_rate_received;
	lte_download_time =  MSEC_PER_SEC * EMCOM_MPFLOW_WEIBO_SIZE / iface2->max_rate_received;
	if ((iface1->mean_srtt_ms + wifi_download_time) <
		(iface2->mean_srtt_ms + (lte_download_time / EMCOM_MPFLOW_DEV_NUM)))
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
	else if ((iface2->mean_srtt_ms + lte_download_time) <
		(iface1->mean_srtt_ms + (wifi_download_time / EMCOM_MPFLOW_DEV_NUM)))
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE;
	return need_set_bind;
}

static int emcom_xengine_mpflow_chk_rst_wifi_pri(const struct emcom_xengine_mpflow_iface *iface1,
	const struct emcom_xengine_mpflow_iface *iface2, struct emcom_xengine_mpflow_info *mpflow_uid)
{
	int need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;

	if (!iface1->max_rate_received_flow || !iface2->max_rate_received_flow) {
		emcom_logd("emcom_xengine_mpflow_chk_rst_wifi_pri, no enough dl return NORST\n");
		return need_set_bind;
	}

	/*lint -e666*/
	if (time_before(jiffies, (iface1->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH)) ||
		time_before(jiffies, (iface2->start_jiffies + EMCOM_MPFLOW_FLOW_TIME_THREH))) {
		emcom_logd("emcom_xengine_mpflow_chk_rst_wifi_pri, not timeout, return NORST\n");
		return need_set_bind;
	}
	/*lint +e666*/

	if (!iface1->is_slow && iface1->is_sure_no_slow)
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2WIFI;
	else if (!iface2->is_slow && iface2->is_sure_no_slow)
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_RST2LTE;

	if (mpflow_uid->rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE &&
		(!iface1->is_slow && !iface1->is_sure_no_slow)) {
		mpflow_uid->rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		need_set_bind = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	}

	return need_set_bind;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
static void emcom_xengine_mpflow_timer(unsigned long arg)
#else
static void emcom_xengine_mpflow_timer(struct timer_list *t)
#endif
{
	uint8_t index;
	bool need_reset_timer = false;
	int bind_mode_wifi = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	int bind_mode_lte = EMCOM_XENGINE_MPFLOW_BINDMODE_NORST;
	int bind_mode;
	struct emcom_xengine_mpflow_info *mpflow_uid = NULL;

	spin_lock(&g_mpflow_lock);
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		mpflow_uid = &g_mpflow_uids[index];
		if ((mpflow_uid->uid == UID_INVALID_APP) || !mpflow_uid->is_downloading)
			continue;
		emcom_xengine_mpflow_update(&mpflow_uid->wifi, mpflow_uid);
		emcom_xengine_mpflow_update(&mpflow_uid->lte, mpflow_uid);
		if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_netdisk(&mpflow_uid->wifi, &mpflow_uid->lte, index);
			bind_mode_lte = emcom_xengine_mpflow_chk_rst_netdisk(&mpflow_uid->lte, &mpflow_uid->wifi, index);
		} else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_market(&mpflow_uid->wifi, &mpflow_uid->lte, index);
			bind_mode_lte = emcom_xengine_mpflow_chk_rst_market(&mpflow_uid->lte, &mpflow_uid->wifi, index);
		} else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_weibo(&mpflow_uid->wifi, &mpflow_uid->lte);
		} else if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI) {
			bind_mode_wifi = emcom_xengine_mpflow_chk_rst_wifi_pri(&mpflow_uid->wifi,
				&mpflow_uid->lte, mpflow_uid);
		}
		bind_mode = (bind_mode_wifi == EMCOM_XENGINE_MPFLOW_BINDMODE_NORST) ?
			bind_mode_lte : bind_mode_wifi;
		emcom_xengine_mpflow_set_bind(mpflow_uid, bind_mode);
		if (mpflow_uid->wifi.flow_cnt || mpflow_uid->lte.flow_cnt) {
			need_reset_timer = true;
		} else {
			emcom_logd("uid %u download is stop", mpflow_uid->uid);
			emcom_xengine_mpflow_download_finish(mpflow_uid);
			emcom_xengine_mpflow_fi_init(mpflow_uid);
		}
	}

	if (need_reset_timer) {
		mod_timer(&g_mpflow_tm, jiffies + HZ);
	} else {
		emcom_logd("stop mpflow timer");
		g_mpflow_tm_running = false;
	}
	spin_unlock(&g_mpflow_lock);
}

static void emcom_xengine_mpflow_add_node(struct sock *sk, struct emcom_xengine_mpflow_iface *iface,
	int index)
{
	struct emcom_xengine_mpflow_node *node = NULL;
	struct tcp_sock *tp = tcp_sk(sk);
	node = kmalloc(sizeof(struct emcom_xengine_mpflow_node), GFP_ATOMIC);
	if (node) {
		int i;
		node->last_bytes_received = tp->bytes_received;
		for (i = EMCOM_MPFLOW_FI_STAT_SECONDS - 1; i >= 0; i--)
			node->rate_received[i] = 0;
		node->tp = tp;
		node->start_jiffies = jiffies;
		sock_hold(sk);
		list_add(&node->list, &iface->flows);
		iface->flow_cnt++;
		if (!g_mpflow_uids[index].is_downloading) {
			emcom_logd("uid %u is_downloading", g_mpflow_uids[index].uid);
			g_mpflow_uids[index].is_downloading = 1;
			iface->start_jiffies = jiffies;
		}

		emcom_logd("sk %pK is a download flow", tp);
		if (!g_mpflow_tm_running) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
			init_timer(&g_mpflow_tm);
			g_mpflow_tm.function = emcom_xengine_mpflow_timer;
			g_mpflow_tm.data     = (unsigned long)NULL;
#else
			timer_setup(&g_mpflow_tm, emcom_xengine_mpflow_timer, 0);
#endif
			g_mpflow_tm_running = true;
			emcom_logd("start mpflow timer");
			mod_timer(&g_mpflow_tm, jiffies + HZ);
		}
	}
}

static void emcom_xengine_mpflow_download_flow_add(struct sock *sk)
{
	uid_t uid = sk->sk_uid.val;
	int index;
	struct dst_entry *dst = NULL;
	struct emcom_xengine_mpflow_iface *iface = NULL;
	uint16_t port = sk->sk_num;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(uid);
	if (index == INDEX_INVALID) {
		emcom_logd("emcom_xengine_mpflow_finduid fail");
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}
	dst = sk_dst_get(sk);
	if (dst) {
		if (strncmp(EMCOM_WLAN0_IFNAME, dst->dev->name, IFNAMSIZ) == 0) {
			iface = &g_mpflow_uids[index].wifi;
		} else if (strncmp(EMCOM_CELL0_IFNAME, dst->dev->name, IFNAMSIZ) == 0) {
			iface = &g_mpflow_uids[index].lte;
		} else {
			emcom_logd("sk port %u iface is %s", port, dst->dev->name);
			dst_release(dst);
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
		dst_release(dst);
	} else {
		emcom_logd("sk port %u dst is not found", port);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	sk->is_download_flow = 1;
	emcom_xengine_mpflow_add_node(sk, iface, index);
	spin_unlock_bh(&g_mpflow_lock);
}

static bool emcom_xengine_mpflow_bm_search(const uint8_t *buf, uint32_t buflen,
	const struct emcom_xengine_mpflow_ptn *sptn, uint32_t *offset)
{
	uint32_t pindex;
	uint8_t skip_stride;
	uint8_t shift_stride;
	uint32_t bindex = sptn->ptnlen;

	if ((buf == NULL) || (sptn->ptn == NULL) || (sptn->skip == NULL) || (sptn->shift == NULL)) {
		return false;
	}
	if ((sptn->ptnlen <= 0) || (buflen <= sptn->ptnlen)) {
		return false;
	}
	while (bindex <= buflen) {
		pindex = sptn->ptnlen;
		while (sptn->ptn[--pindex] == buf[--bindex]) {
			if (pindex != 0)
				continue;
			if (offset != NULL)
				*offset = bindex;
			return true;
		}
		skip_stride = sptn->skip[buf[bindex]];
		shift_stride = sptn->shift[pindex];
		bindex += ((skip_stride > shift_stride) ? skip_stride : shift_stride);
	}
	return false;
}

static bool emcom_xengine_mpflow_chk_download_flow(struct sk_buff *skb)
{
	int i;
	int index;
	uint16_t port;
	struct tcphdr *tcph = tcp_hdr(skb);
	uint16_t buflen = skb_headlen(skb);
	uint32_t offset = 0;

	port = ntohs(tcph->dest);
	/* download flow must be http(80) or https(443) */
	if ((port != EMCOM_MPFLOW_FI_PORT_80) && (port != EMCOM_MPFLOW_FI_PORT_443))
		return false;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(skb->sk->sk_uid.val);
	if (index == INDEX_INVALID) {
		emcom_loge("index is invalid\n");
		spin_unlock_bh(&g_mpflow_lock);
		return false;
	}
	if (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO ||
		g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI) {
		spin_unlock_bh(&g_mpflow_lock);
		return true;
	} else if ((port == EMCOM_MPFLOW_FI_PORT_80) && (g_mpflow_uids[index].ptn_80_num != 0)) {
		for (i = 0; i < g_mpflow_uids[index].ptn_80_num; i++) {
			if (emcom_xengine_mpflow_bm_search((const uint8_t *)skb->data, buflen,
				(const struct emcom_xengine_mpflow_ptn *)&(g_mpflow_uids[index].ptn_80[i]),
				&offset)) {
				emcom_logd("received a port 80 packet match ptn: %s\n",
						   g_mpflow_uids[index].ptn_80[i].ptn);
				spin_unlock_bh(&g_mpflow_lock);
				return true;
			}
		}
	} else if ((port == EMCOM_MPFLOW_FI_PORT_443) && (g_mpflow_uids[index].ptn_443_num != 0)) {
		for (i = 0; i < g_mpflow_uids[index].ptn_443_num; i++) {
			if (emcom_xengine_mpflow_bm_search((const uint8_t *)skb->data, buflen,
				(const struct emcom_xengine_mpflow_ptn *)&(g_mpflow_uids[index].ptn_443[i]),
				&offset)) {
				emcom_logd("received a port 443 packet match ptn: %s\n",
						   g_mpflow_uids[index].ptn_443[i].ptn);
				spin_unlock_bh(&g_mpflow_lock);
				return true;
			}
		}
	}
	spin_unlock_bh(&g_mpflow_lock);
	return false;
}

static unsigned int emcom_xengine_mpflow_hook_out_ipv4(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct sock *sk = skb->sk;
	struct iphdr *iph = ip_hdr(skb);
	struct tcphdr *tcph = tcp_hdr(skb);

	if (!sk || !iph || !tcph)
		return NF_ACCEPT;

	if ((sk->sk_state != TCP_ESTABLISHED) || (sk->sk_protocol != IPPROTO_TCP))
		return NF_ACCEPT;

	if (sk->snd_pkt_cnt > 0)
		return NF_ACCEPT;

	if (skb_headlen(skb) <= (tcp_hdrlen(skb) + ip_hdrlen(skb)))
		return NF_ACCEPT;

	if (sk->is_mp_flow && !sk->is_download_flow) {
		sk->snd_pkt_cnt++;
		if (emcom_xengine_mpflow_chk_download_flow(skb))
			emcom_xengine_mpflow_download_flow_add(sk);
	} else if (sk->is_mp_flow_bind) {
		sk->snd_pkt_cnt++;
	} else {
		return NF_ACCEPT;
	}
	emcom_xengine_process_http_req(skb, EMCOM_XENGINE_IP_TYPE_V4);
	return NF_ACCEPT;
}

static unsigned int emcom_xengine_mpflow_hook_out_ipv6(void *priv, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct sock *sk = skb->sk;
	struct ipv6hdr *iph = ipv6_hdr(skb);
	struct tcphdr *tcph = tcp_hdr(skb);

	if (!sk || !iph || !tcph)
		return NF_ACCEPT;

	if ((sk->sk_state != TCP_ESTABLISHED) || (sk->sk_protocol != IPPROTO_TCP))
		return NF_ACCEPT;

	if (sk->snd_pkt_cnt > 0)
		return NF_ACCEPT;

	if (skb_headlen(skb) <= (tcp_hdrlen(skb) + sizeof(struct ipv6hdr)))
		return NF_ACCEPT;

	if (sk->is_mp_flow && !sk->is_download_flow) {
		sk->snd_pkt_cnt++;
		if (emcom_xengine_mpflow_chk_download_flow(skb))
			emcom_xengine_mpflow_download_flow_add(sk);
	} else if (sk->is_mp_flow_bind) {
		sk->snd_pkt_cnt++;
	} else {
		return NF_ACCEPT;
	}
	emcom_xengine_process_http_req(skb, EMCOM_XENGINE_IP_TYPE_V6);
	return NF_ACCEPT;
}

void emcom_xengine_mpflow_init(void)
{
	uint8_t uindex;
	errno_t err;

	emcom_logd("mpflow init");
	spin_lock_init(&g_mpflow_lock);
	spin_lock_bh(&g_mpflow_lock);
	for (uindex = 0; uindex < EMCOM_MPFLOW_MAX_APP; uindex++) {
		g_mpflow_uids[uindex].uid = UID_INVALID_APP;
		g_mpflow_uids[uindex].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		g_mpflow_uids[uindex].enableflag = 0;
		g_mpflow_uids[uindex].protocol = 0;
		err = memset_s(g_mpflow_uids[uindex].dport_range,
					   sizeof(g_mpflow_uids[uindex].dport_range),
					   0,
					   sizeof(g_mpflow_uids[uindex].dport_range));
		if (err != EOK)
			emcom_logd("emcom_xengine_mpflow_init failed");
	}
	err = memset_s(g_mpflow_list, sizeof(g_mpflow_list), 0, sizeof(g_mpflow_list));
	if (err != EOK)
		emcom_logd("emcom_xengine_mpflow_init g_mpflow_list failed");
	spin_unlock_bh(&g_mpflow_lock);
	g_mpflow_index = 0;

	spin_lock_init(&g_mpflow_ai_lock);
	spin_lock_bh(&g_mpflow_ai_lock);
	err = memset_s(g_mpflow_ai_uids, sizeof(g_mpflow_ai_uids), 0, sizeof(g_mpflow_ai_uids));
	if (err != EOK)
		emcom_logd("emcom_xengine_mpflow_init ai failed");
	err = memset_s(g_mpflow_ai_list, sizeof(g_mpflow_ai_list), 0, sizeof(g_mpflow_ai_list));
	if (err != EOK)
		emcom_logd("g_mpflow_ai_list failed");
	spin_unlock_bh(&g_mpflow_ai_lock);
}

void emcom_xengine_mpflow_clear(void)
{
	uint8_t index;
	errno_t err;
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;

	spin_lock_bh(&g_mpflow_lock);
	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if ((g_mpflow_uids[index].uid != UID_INVALID_APP) &&
			((g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) ||
			 (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) ||
			 (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) ||
			 (g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI))) {
			list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].wifi.flows, list)
				emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].wifi, node);

			list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].lte.flows, list)
				emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].lte, node);

			emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
			emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
			emcom_xengine_mpflow_apppriv_deinit(&g_mpflow_uids[index]);
		}

		g_mpflow_uids[index].uid = UID_INVALID_APP;
		g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		g_mpflow_uids[index].enableflag = 0;
		g_mpflow_uids[index].protocol = 0;
		err = memset_s(&g_mpflow_uids[index].dport_range, /*lint !e545*/
					   sizeof(g_mpflow_uids[index].dport_range),
					   0,
					   sizeof(g_mpflow_uids[index].dport_range));
		if (err != EOK)
			emcom_logd("emcom_xengine_mpflow_clear failed");
	}
	spin_unlock_bh(&g_mpflow_lock);

	emcom_xengine_mpflow_unregister_nf_hook(EMCOM_MPFLOW_VER_V1);
	g_mpflow_index = 0;
}

int8_t emcom_xengine_mpflow_finduid(uid_t uid)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid == uid)
			return index;
	}

	return INDEX_INVALID;
}

int8_t emcom_xengine_mpflow_getfreeindex(void)
{
	int8_t index;

	for (index = 0; index < EMCOM_MPFLOW_MAX_APP; index++) {
		if (g_mpflow_uids[index].uid == UID_INVALID_APP)
			return index;
	}
	return INDEX_INVALID;
}

static void emcom_xengine_mpflow_fill_info(struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo,
	uint8_t index)
{
	/* Fill mpflow info. */
	g_mpflow_uids[index].uid = mpflowstartinfo->uid;
	g_mpflow_uids[index].enableflag = mpflowstartinfo->enableflag;
	g_mpflow_uids[index].protocol = mpflowstartinfo->protocol;
	g_mpflow_uids[index].bindmode = mpflowstartinfo->bindmode;
	g_mpflow_uids[index].algorithm_type = mpflowstartinfo->algorithm_type;
	g_mpflow_uids[index].reserve_field = mpflowstartinfo->reserve_field;
}

void emcom_xengine_mpflow_start(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_start_info *mpflowstartinfo = NULL;
	uint8_t index;
	int8_t tmp;
	bool ret = false;
	bool is_new_uid_enable = false;

	/* input param check */
	if (!pdata || (len != sizeof(struct emcom_xengine_mpflow_parse_start_info))) {
		emcom_loge("mpflow start data or length %d is error", len);
		return;
	}
	mpflowstartinfo = (struct emcom_xengine_mpflow_parse_start_info *)pdata;

	spin_lock_bh(&g_mpflow_lock);
	tmp = emcom_xengine_mpflow_finduid(mpflowstartinfo->uid);
	if (tmp < 0) {
		int8_t newindex;
		newindex = emcom_xengine_mpflow_getfreeindex();
		if (newindex == INDEX_INVALID) {
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
		index = newindex;
		is_new_uid_enable = true;
	} else {
		/* tmp >= 0 use as array index */
		index = (uint8_t)tmp;
	}

	/* redundant operation. new uid related mpflow list entry doesn't exist normally */
	if (is_new_uid_enable)
		emcom_xengine_mpflow_clear_blocked(mpflowstartinfo->uid, EMCOM_MPFLOW_VER_V1);

	emcom_xengine_mpflow_fill_info(mpflowstartinfo, index);
	if (mpflowstartinfo->enableflag & EMCOM_MPFLOW_ENABLEFLAG_DPORT) {
		errno_t err = memcpy_s(g_mpflow_uids[index].dport_range,
							   sizeof(g_mpflow_uids[index].dport_range),
							   mpflowstartinfo->dport_range,
							   sizeof(mpflowstartinfo->dport_range));
		if (err != EOK)
			emcom_loge("emcom_xengine_mpflow_start memcpy failed");
	}
	if ((g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) ||
		(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) ||
		(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) ||
		(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI)) {
		bool fi_start_ret = emcom_xengine_mpflow_fi_start(is_new_uid_enable, index, &ret, mpflowstartinfo);
		if (!fi_start_ret) {
			spin_unlock_bh(&g_mpflow_lock);
			return;
		}
	}

	emcom_xengine_mpflow_show();
	spin_unlock_bh(&g_mpflow_lock);
	emcom_xengine_mpflow_register_nf_hook(EMCOM_MPFLOW_VER_V1);
}

void emcom_xengine_mpflow_stop(const char *pdata, uint16_t len)
{
	struct emcom_xengine_mpflow_parse_stop_info *mpflowstopinfo = NULL;
	int8_t index;
	int32_t stop_reason;
	struct emcom_xengine_mpflow_node *node = NULL;
	struct emcom_xengine_mpflow_node *tmp = NULL;
	bool mpflow_uid_empty = false;

	/* input param check */
	if (!pdata || (len != sizeof(struct emcom_xengine_mpflow_parse_stop_info))) {
		emcom_loge("mpflow stop data or length %d is error", len);
		return;
	}

	mpflowstopinfo = (struct emcom_xengine_mpflow_parse_stop_info *)pdata;
	stop_reason = mpflowstopinfo->stop_reason;
	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(mpflowstopinfo->uid);
	if (index != INDEX_INVALID) {
		if ((stop_reason == EMCOM_MPFLOW_STOP_REASON_NETWORK_ROAMING) ||
			(stop_reason == EMCOM_MPFLOW_STOP_REASON_APPDIED)) {
			errno_t err;

			if ((g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_NET_DISK) ||
				(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_MARKET) ||
				(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WEIBO) ||
				(g_mpflow_uids[index].algorithm_type == EMCOM_MPFLOW_ENABLETYPE_WIFI_PRI)) {
				list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].wifi.flows, list)
					emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].wifi, node);

				list_for_each_entry_safe(node, tmp, &g_mpflow_uids[index].lte.flows, list)
					emcom_xengine_mpflow_download_flow_del(&g_mpflow_uids[index].lte, node);

				emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_80, g_mpflow_uids[index].ptn_80_num);
				emcom_xengine_mpflow_ptn_deinit(g_mpflow_uids[index].ptn_443, g_mpflow_uids[index].ptn_443_num);
				emcom_xengine_mpflow_apppriv_deinit(&g_mpflow_uids[index]);
			}

			g_mpflow_uids[index].uid = UID_INVALID_APP;
			g_mpflow_uids[index].enableflag = 0;
			g_mpflow_uids[index].protocol = 0;
			g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;

			err = memset_s(g_mpflow_uids[index].dport_range, sizeof(g_mpflow_uids[index].dport_range), 0,
						   sizeof(g_mpflow_uids[index].dport_range));
			if (err != EOK)
				emcom_loge("emcom_xengine_mpflow_stop memset failed");
		} else {
			g_mpflow_uids[index].bindmode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
		}
	}
	emcom_xengine_mpflow_show();
	emcom_xengine_mpflow_delete(mpflowstopinfo->uid, EMCOM_MPFLOW_VER_V1);

	mpflow_uid_empty = emcom_xengine_mpflow_uid_empty();
	spin_unlock_bh(&g_mpflow_lock);

	if (mpflow_uid_empty)
		emcom_xengine_mpflow_unregister_nf_hook(EMCOM_MPFLOW_VER_V1);
}

bool emcom_xengine_mpflow_checkvalid(struct sock *sk, struct sockaddr *uaddr, int8_t index, uint16_t *dport)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	bool isvalidaddr = false;

	if (!sk || !uaddr)
		return false;

	isvalidaddr = emcom_xengine_check_ip_addrss(uaddr) && (!emcom_xengine_check_ip_is_private(uaddr));
	if (isvalidaddr == false) {
		emcom_logd("mpflow check valid addr is not valid. uid: %u",
				   g_mpflow_uids[index].uid);
		return false;
	}

	emcom_logd("mpflow check valid uid: %u link famliy: %d, link proto: %d,"
			   "mpflow protocol: %d, bindmode: %u, ",
			   g_mpflow_uids[index].uid, sk->sk_family, sk->sk_protocol,
			   g_mpflow_uids[index].protocol,
			   g_mpflow_uids[index].bindmode);

	if (!(((sk->sk_protocol == IPPROTO_TCP) &&
		(EMCOM_MPFLOW_PROTOCOL_TCP & g_mpflow_uids[index].protocol))
		|| ((sk->sk_protocol == IPPROTO_UDP) &&
		(EMCOM_MPFLOW_PROTOCOL_UDP & g_mpflow_uids[index].protocol)))) {
		emcom_logd("mpflow check valid protocol not correct uid: %u, sk: %pK",
				   g_mpflow_uids[index].uid, sk);
		return false;
	}

	if (g_mpflow_uids[index].enableflag & EMCOM_MPFLOW_ENABLEFLAG_DPORT) {
		bool bfinddport = false;
		if (usin->sin_family == AF_INET) {
			*dport = ntohs(usin->sin_port);
		} else if (usin->sin_family == AF_INET6) {
			struct sockaddr_in6 *usin6 = (struct sockaddr_in6 *)uaddr;
			*dport = (uint16_t)ntohs(usin6->sin6_port);
		} else {
			emcom_logd("mpflow check valid not support family uid: %u,"
					   " sin_family: %d",
					   g_mpflow_uids[index].uid, usin->sin_family);
			return false;
		}
		bfinddport = emcom_xengine_mpflow_finddport(&g_mpflow_uids[index], *dport);
		if (bfinddport == false) {
			emcom_logd("mpflow check valid can not find uid: %u, dport: %d",
					   g_mpflow_uids[index].uid, *dport);
			return false;
		}
	}

	return true;
}

bool emcom_xengine_mpflow_getinetaddr(struct net_device *dev)
{
	struct in_device *in_dev = NULL;
	struct in_ifaddr *ifa = NULL;

	if (!dev) {
		emcom_logd("mpflow get inet addr dev is null");
		return false;
	}

	in_dev = __in_dev_get_rcu(dev);
	if (!in_dev) {
		emcom_logd("mpflow get inet addr in_dev is null dev: %s", dev->name);
		return false;
	}

	ifa = in_dev->ifa_list;
	if (ifa != NULL)
		return true;

	return false;
}

int emcom_xengine_mpflow_getmode_rand(int8_t index, uid_t uid, struct sockaddr *uaddr)
{
	bool is_wifi_block = false;
	bool is_lte_block = false;
	int bind_device;

	if (g_mpflow_uids[index].rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI) {
		bind_device = EMCOM_MPFLOW_BIND_WIFI;
	} else if (g_mpflow_uids[index].rst_bind_mode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE) {
		bind_device = EMCOM_MPFLOW_BIND_LTE;
	} else if (g_mpflow_uids[index].app_priv) {
		bind_device = emcom_xengine_mpflow_ip_bind(uaddr, &g_mpflow_uids[index]);
	} else {
		g_mpflow_index++;
		if (emcom_xengine_mpflow_connum(uid, EMCOM_WLAN0_IFNAME) == 0)
			bind_device = EMCOM_MPFLOW_BIND_WIFI;
		else if (g_mpflow_index % EMCOM_MPFLOW_DEV_NUM == 0)
			bind_device = EMCOM_MPFLOW_BIND_WIFI;
		else
			bind_device = EMCOM_MPFLOW_BIND_LTE;
	}

	is_wifi_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN0_IFNAME, EMCOM_MPFLOW_VER_V1);
	is_lte_block = emcom_xengine_mpflow_blocked(uid, EMCOM_CELL0_IFNAME, EMCOM_MPFLOW_VER_V1);
	if ((is_wifi_block == true) && (is_lte_block == true))
		bind_device = EMCOM_MPFLOW_BIND_NONE;
	else if ((is_wifi_block == true) && (bind_device == EMCOM_MPFLOW_BIND_WIFI))
		bind_device = EMCOM_MPFLOW_BIND_LTE;
	else if ((is_lte_block == true) && (bind_device == EMCOM_MPFLOW_BIND_LTE))
		bind_device = EMCOM_MPFLOW_BIND_WIFI;

	return bind_device;
}

int emcom_xengine_mpflow_getmode_spec(int8_t index, uid_t uid)
{
	bool is_wifi_block = false;
	bool is_lte_block = false;
	int bind_device = EMCOM_MPFLOW_BIND_NONE;

	is_wifi_block = emcom_xengine_mpflow_blocked(uid, EMCOM_WLAN0_IFNAME, EMCOM_MPFLOW_VER_V1);
	is_lte_block = emcom_xengine_mpflow_blocked(uid, EMCOM_CELL0_IFNAME, EMCOM_MPFLOW_VER_V1);

	if ((is_wifi_block && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)) ||
		(is_lte_block && (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)) ||
		((is_wifi_block == true) && (is_lte_block == true))) {
		emcom_logd("mpflow bind blocked uid: %u, bindmode: %d, blocked:%d, %d, connnum:%d",
				   uid, g_mpflow_uids[index].bindmode, is_wifi_block, is_lte_block,
				   emcom_xengine_mpflow_connum(uid, EMCOM_WLAN0_IFNAME));
		return bind_device;
	}

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_WIFI)
		bind_device = EMCOM_MPFLOW_BIND_WIFI;
	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_LTE)
		bind_device = EMCOM_MPFLOW_BIND_LTE;

	return bind_device;
}

int emcom_xengine_mpflow_getmode(int8_t index, uid_t uid, struct sockaddr *uaddr)
{
	int bind_device;

	/*lint -e666*/
	if (g_mpflow_uids[index].rst_bind_mode != EMCOM_XENGINE_MPFLOW_BINDMODE_NONE) {
		if (time_after(jiffies, (g_mpflow_uids[index].rst_jiffies + EMCOM_MPFLOW_RST_TIME_THREH)))
			g_mpflow_uids[index].rst_bind_mode = EMCOM_XENGINE_MPFLOW_BINDMODE_NONE;
	}
	/*lint +e666*/

	if (g_mpflow_uids[index].bindmode == EMCOM_XENGINE_MPFLOW_BINDMODE_RANDOM)
		bind_device = emcom_xengine_mpflow_getmode_rand(index, uid, uaddr);
	else
		bind_device = emcom_xengine_mpflow_getmode_spec(index, uid);

	return bind_device;
}

static void emcom_xengine_mpflow_bind2device_proc(struct sock *sk, struct sockaddr *uaddr,
	char *ifname, const uid_t uid)
{
	struct net_device *dev = NULL;

	rcu_read_lock();
	dev = dev_get_by_name_rcu(sock_net(sk), ifname);
	if (!dev || !emcom_xengine_mpflow_getinetaddr(dev)) {
		rcu_read_unlock();
		emcom_logd("mpflow bind2device dev not ready uid: %u, sk: %pK, dev: %pK, name: %s",
				   uid, sk, dev, (dev == NULL) ? "null" : dev->name);
		return;
	}
	rcu_read_unlock();
	sk->sk_bound_dev_if = dev->ifindex;
	if (emcom_mpdns_is_allowed(uid, NULL)) {
		int tar_net = emcom_xengine_get_net_type_by_name(ifname);
		int cur_net = emcom_xengine_get_net_type(sk->sk_mark & EMCOM_XENGINE_NET_ID_MASK);
		emcom_mpdns_update_dst_addr(sk, uaddr, cur_net, tar_net);
	}
	emcom_logd("mpflow bind2device success uid: %u, sk: %pK, "
			   "ifname: %s, ifindex: %d", uid, sk, ifname, sk->sk_bound_dev_if);
}

void emcom_xengine_mpflow_bind2device(struct sock *sk, struct sockaddr *uaddr)
{
	uid_t uid;
	int8_t index;
	char ifname[IFNAMSIZ] = {0};
	int bind_device;
	uint16_t dport;
	errno_t err = EOK;

	if (!sk || !uaddr)
		return;

	uid = sock_i_uid(sk).val;
	if (invalid_uid(uid))
		return;

	spin_lock_bh(&g_mpflow_lock);
	index = emcom_xengine_mpflow_finduid(uid);
	if (index == INDEX_INVALID) {
		emcom_xengine_mpflow_ai_bind2device(sk, uaddr);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	if (emcom_xengine_mpflow_checkvalid(sk, uaddr, index, &dport) == false) {
		emcom_logd("mpflow bind2device check valid fail uid: %u", uid);
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}

	bind_device = emcom_xengine_mpflow_getmode(index, uid, uaddr);
	if (bind_device == EMCOM_MPFLOW_BIND_WIFI) {
		err = memcpy_s(ifname, sizeof(char) * IFNAMSIZ, EMCOM_WLAN0_IFNAME, (strlen(EMCOM_WLAN0_IFNAME) + 1));
	} else if (bind_device == EMCOM_MPFLOW_BIND_LTE) {
		err = memcpy_s(ifname, sizeof(char) * IFNAMSIZ, EMCOM_CELL0_IFNAME, (strlen(EMCOM_CELL0_IFNAME) + 1));
	} else if (bind_device == EMCOM_MPFLOW_BIND_NONE) {
		spin_unlock_bh(&g_mpflow_lock);
		return;
	}
	if (err != EOK)
		emcom_loge("mpflow bind2device memcpy failed");

	sk->is_mp_flow = 1;
	spin_unlock_bh(&g_mpflow_lock);
	emcom_xengine_mpflow_bind2device_proc(sk, uaddr, ifname, uid);
}

bool emcom_xengine_mpflow_finddport(struct emcom_xengine_mpflow_info *mpflowinfo, uint16_t dport)
{
	int i;

	if (!mpflowinfo) {
		emcom_loge("mpflow finddport mpflow info is NULL");
		return false;
	}

	emcom_logd("mpflow finddport dport: %u", dport);

	for (i = 0; i < EMCOM_MPFLOW_PORT_RANGE_NUM_MAX; i++) {
		if ((mpflowinfo->dport_range[i].start_port <= dport) &&
			(mpflowinfo->dport_range[i].end_port >= dport))
			return true;
	}

	return false;
}

struct emcom_xengine_mpflow_stat *emcom_xengine_mpflow_get(int uid, const char *name, int ifindex, uint8_t ver)
{
	struct emcom_xengine_mpflow_stat *node = NULL;
	int8_t index;
	errno_t err;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return NULL;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		if ((node == NULL) && (mpflow_list[index].uid == UID_INVALID_APP))
			node = &mpflow_list[index];
		if ((mpflow_list[index].uid == uid) && (!strncmp(mpflow_list[index].name, name, strlen(name))))
			return &mpflow_list[index];
	}

	if (!node) {
		emcom_logd("emcom_xengine_mpflow_get list full\n");
		return NULL;
	}

	node->uid = uid;
	node->ifindex = ifindex;
	err = strncpy_s(node->name, IFNAMSIZ, name, IFNAMSIZ - 1);
	if (err != EOK) {
		emcom_loge("emcom_xengine_mpflow_get strncpy_s failed, errcode: %d", err);
		node->uid = UID_INVALID_APP;
		return NULL;
	}
	node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
	node->mpflow_fail_nopayload = 0;
	node->mpflow_fail_syn_rst = 0;
	node->mpflow_fail_syn_timeout = 0;
	node->mpflow_estlink = 0;
	node->start_jiffies = 0;
	err = memset_s(node->retrans_count, sizeof(node->retrans_count), 0, sizeof(node->retrans_count));
	if (err != EOK) {
		emcom_loge("emcom_xengine_mpflow_get memset failed, errcode: %d", err);
		node->uid = UID_INVALID_APP;
		return NULL;
	}

	return node;
}

void emcom_xengine_mpflow_delete(int uid, uint8_t ver)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &mpflow_list[index];
		if (node->uid == uid)
			node->uid = UID_INVALID_APP;
	}
}

void emcom_xengine_mpflow_clear_blocked(int uid, uint8_t ver)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	errno_t err;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &mpflow_list[index];
		if (node->uid == uid) {
			node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
			node->mpflow_fail_nopayload = 0;
			node->mpflow_fail_syn_rst = 0;
			node->mpflow_fail_syn_timeout = 0;
			node->start_jiffies = 0;
			err = memset_s(node->retrans_count, sizeof(node->retrans_count),
						   0, sizeof(node->retrans_count));
			if (err != EOK)
				emcom_logd("emcom_xengine_mpflow_clear_blocked memset failed");
		}
	}
}

bool emcom_xengine_mpflow_blocked(int uid, const char *name, uint8_t ver)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct emcom_xengine_mpflow_stat *mpflow_list = NULL;

	if (!emcom_xengine_get_mpflowlist(ver, &mpflow_list)) {
		return false;
	}

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &mpflow_list[index];
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return (node->mpflow_fallback == EMCOM_MPFLOW_FALLBACK_SET);
	}

	return false;
}

int16_t emcom_xengine_mpflow_connum(int uid, const char *name)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &g_mpflow_list[index];
		if ((node->uid == uid) && (!strncmp(node->name, name, strlen(name))))
			return node->mpflow_estlink;
	}
	return 0;
}


void emcom_xengine_mpflow_report(const void *data, int len)
{
	emcom_send_msg2daemon(NETLINK_EMCOM_KD_MPFLOW_FALLBACK, data, len);
}

void emcom_xengine_mpflow_show(void)
{
	int8_t index;
	struct emcom_xengine_mpflow_stat *node = NULL;

	for (index = 0; index < EMCOM_MPFLOW_MAX_LIST_NUM; index++) {
		node = &g_mpflow_list[index];
		if (node->uid == UID_INVALID_APP)
			continue;
		emcom_loge("MpFlow showinfo uid:%d inf:%d(%s) fail:%d estlink:%d "
				   "nodata,rst,tout:%d,%d,%d \n",
				   node->uid, node->ifindex, node->name,
				   node->mpflow_fallback, node->mpflow_estlink,
				   node->mpflow_fail_nopayload, node->mpflow_fail_syn_rst,
				   node->mpflow_fail_syn_timeout);
	}
	return;
}

bool emcom_xengine_mpflow_errlink(int reason, struct emcom_xengine_mpflow_stat *node)
{
	if (reason == EMCOM_MPFLOW_FALLBACK_NOPAYLOAD)
		node->mpflow_fail_nopayload++;
	else if (reason == EMCOM_MPFLOW_FALLBACK_SYN_RST)
		node->mpflow_fail_syn_rst++;
	else if ((reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT) ||
			(reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST))
		node->mpflow_fail_syn_timeout++;
	else
		return false;

	if ((node->mpflow_fail_nopayload >= EMCOM_MPFLOW_FALLBACK_NOPAYLOAD_THRH)
		|| (node->mpflow_fail_syn_rst >= EMCOM_MPFLOW_FALLBACK_SYN_RST_THRH)
		|| (node->mpflow_fail_syn_timeout >= EMCOM_MPFLOW_FALLBACK_SYN_TOUT_THRH))
		return true;

	return false;
}

bool emcom_xengine_mpflow_retrans(int reason, struct emcom_xengine_mpflow_stat *node)
{
	int i;
	errno_t err;

	if ((reason != EMCOM_MPFLOW_FALLBACK_RETRANS) &&
		(reason != EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST))
		return false;

	for (i = 1; (i <= node->retrans_count[0]) && (i <= EMCOM_MPFLOW_FALLBACK_RETRANS_TIME); i++) {
		if (time_before_eq(jiffies, node->start_jiffies + i * HZ)) {
			node->retrans_count[i]++;
			break;
		}
	}

	/* Time range matched */
	if (i <= node->retrans_count[0]) {
		/* expand time range */
		if (node->retrans_count[i] == EMCOM_MPFLOW_FALLBACK_RETRANS_THRH) {
			node->retrans_count[0]++;
			emcom_logd("MpFlow fallback uid:%d inf:%d(%s) count:%d, jiffies:%lu\n",
					   node->uid, node->ifindex, node->name, node->retrans_count[0], node->start_jiffies);
		}
		/* retransmission fallback */
		if (node->retrans_count[0] > EMCOM_MPFLOW_FALLBACK_RETRANS_TIME) {
			err = memset_s(node->retrans_count, sizeof(node->retrans_count), 0, sizeof(node->retrans_count));
			if (err != EOK)
				emcom_loge("emcom_xengine_mpflow_retrans memset failed");
			return true;
		}
	} else {
		err = memset_s(node->retrans_count, sizeof(node->retrans_count), 0, sizeof(node->retrans_count));
		if (err != EOK)
			emcom_loge("emcom_xengine_mpflow_retrans memset failed");
		node->retrans_count[0] = 1;
		node->retrans_count[1] = 1;
		node->start_jiffies = jiffies;
	}

	return false;
}

int8_t emcom_xengine_mpflow_checkstatus(const struct sock *sk, int *reason, int state,
	struct emcom_xengine_mpflow_stat *node)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct inet_sock *inet = inet_sk(sk);
	int8_t result = EMCOM_MPFLOW_FALLBACK_NONE;
	uint8_t oldstate = sk->sk_state;

	if (*reason == EMCOM_MPFLOW_FALLBACK_NOPAYLOAD) {
#ifdef CONFIG_MPTCP
		if (mptcp_meta_sk(sk) == sk)
			return result;
#endif
		/* EST->DOWN */
		if ((oldstate == TCP_ESTABLISHED) && (state != TCP_ESTABLISHED)) {
			if (node->mpflow_estlink > 0)
				node->mpflow_estlink--;
			if (state != TCP_FIN_WAIT1) {
				result = ((tp->bytes_received <= 1) && ((tp->bytes_acked > 1) ||
					(tp->snd_nxt - tp->snd_una > EMCOM_MPFLOW_SND_BYTE_THRESHOLD))) ?
					EMCOM_MPFLOW_FALLBACK_SET : EMCOM_MPFLOW_FALLBACK_CLR;
			} else {
				result = EMCOM_MPFLOW_FALLBACK_NONE;
			}
		/* DOWN->EST */
		} else if ((oldstate != TCP_ESTABLISHED) && (state == TCP_ESTABLISHED)) {
			result = EMCOM_MPFLOW_FALLBACK_SYNCLR;
			node->mpflow_estlink++;
		}
	} else if (*reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT) {
		result = ((1 << oldstate) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) ? EMCOM_MPFLOW_FALLBACK_SET :
			EMCOM_MPFLOW_FALLBACK_NONE;
	} else if (*reason == EMCOM_MPFLOW_FALLBACK_RETRANS) {
		result = EMCOM_MPFLOW_FALLBACK_SET;
		if (((1 << oldstate) & (TCPF_SYN_SENT | TCPF_SYN_RECV)) &&
			(tp->segs_out == EMCOM_MPFLOW_FALLBACK_SYN_TOUT_TIME))
			*reason = EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST;
	} else {
		result = EMCOM_MPFLOW_FALLBACK_SET;
	}

	if (result != EMCOM_MPFLOW_FALLBACK_NONE)
		emcom_logd("MpFlow checkinfo uid:%d sk:%pK src_addr:"IPV4_FMT":%d dst_addr:"IPV4_FMT":%d inf:%d[%s]"
					"R:%d ost:%d nst:%d P[%d->%d] ret:%d,snt,fly,ack,rcv:%u,%u,%llu,%llu\n",
					node->uid, sk, ipv4_info(sk->sk_rcv_saddr), sk->sk_num, ipv4_info(sk->sk_daddr),
					ntohs(sk->sk_dport), node->ifindex, node->name,
					*reason, oldstate, state, ntohs(inet->inet_sport), ntohs(inet->inet_dport),
					result, tp->segs_out, tp->snd_nxt - tp->snd_una, tp->bytes_acked, tp->bytes_received);

	return result;
}

static void emcom_xengine_mpflow_fallback_report(struct sock *sk, int reason, int state, const char *name,
	struct emcom_xengine_mpflow_stat *node)
{
	int8_t result = emcom_xengine_mpflow_checkstatus(sk, &reason, state, node);
	if (result == EMCOM_MPFLOW_FALLBACK_SET) {
		if (emcom_xengine_mpflow_errlink(reason, node) || emcom_xengine_mpflow_retrans(reason, node)) {
			struct emcom_xengine_mpflow_fallback fallback;
			emcom_loge("MpFlow fallback uid:%d inf:%d(%s) estlink:%d nodata,rst,tout:%d,%d,%d\n",
					   node->uid, node->ifindex, node->name, node->mpflow_estlink,
					   node->mpflow_fail_nopayload, node->mpflow_fail_syn_rst, node->mpflow_fail_syn_timeout);

			/* report connection unreachabled */
			if (!strncmp(EMCOM_WLAN0_IFNAME, name, strlen(name))) {
				fallback.reason = EMCOM_MPFLOW_FALLBACK_WLAN_OFFSET + reason;
			} else if (!strncmp(EMCOM_CELL0_IFNAME, name, strlen(name)) && (reason != EMCOM_MPFLOW_FALLBACK_RETRANS)) {
				fallback.reason = EMCOM_MPFLOW_FALLBACK_LTE_OFFSET + reason;
			}  else if (!strncmp(EMCOM_WLAN1_IFNAME, name, strlen(name))) {
				fallback.reason = EMCOM_MPFLOW_FALLBACK_WLAN1_OFFSET + reason;
			}  else if (!strncmp(EMCOM_CELL1_IFNAME, name, strlen(name)) && (reason != EMCOM_MPFLOW_FALLBACK_RETRANS)) {
				fallback.reason = EMCOM_MPFLOW_FALLBACK_LTE1_OFFSET + reason;
			} else {
				return;
			}

			fallback.uid = node->uid;
			node->report_jiffies = jiffies;
			emcom_xengine_mpflow_report(&fallback, sizeof(fallback));
			emcom_loge("MpFlow fallback report uid:%d reason: %d\n", fallback.uid, fallback.reason);
		}
	} else if (result == EMCOM_MPFLOW_FALLBACK_CLR) {
		node->mpflow_fallback = EMCOM_MPFLOW_FALLBACK_CLR;
		node->mpflow_fail_nopayload = 0;
		node->mpflow_fail_syn_rst = 0;
		node->mpflow_fail_syn_timeout = 0;
	} else if (result == EMCOM_MPFLOW_FALLBACK_SYNCLR) {
		node->mpflow_fail_syn_rst = 0;
		node->mpflow_fail_syn_timeout = 0;
	}

	emcom_mpflow_check_mpdns_addr(sk, reason, result);
}

void emcom_xengine_mpflow_fallback(struct sock *sk, int reason, int state)
{
	struct emcom_xengine_mpflow_stat *node = NULL;
	struct net *net = sock_net(sk);
	struct net_device *dev = NULL;
	char name[IFNAMSIZ];
	errno_t err;
	struct emcom_xengine_mpflow_fallback_ver uid_info;

	/* If the sk not bind yet, not need fallback check */
	if (!sk->sk_bound_dev_if || !sk->is_mp_flow_bind)
		return;

	uid_info.uid = sock_i_uid(sk).val;
	if (invalid_uid(uid_info.uid))
		return;

	uid_info.index = emcom_xengine_mpflow_finduid(uid_info.uid);
	if (uid_info.index != INDEX_INVALID) {
		uid_info.ver = EMCOM_MPFLOW_VER_V1;
	} else {
		uid_info.index = emcom_xengine_mpflow_ai_finduid(uid_info.uid);
		if (uid_info.index != INDEX_INVALID) {
			uid_info.ver = EMCOM_MPFLOW_VER_V2;
		} else {
			return;
		}
	}

	rcu_read_lock();
	dev = dev_get_by_index_rcu(net, sk->sk_bound_dev_if);
	if (!dev || (dev->name[0] == '\0')) {
		rcu_read_unlock();
		emcom_loge("get dev name failed.dev[%pK] sk[%pK] dev_if[%d]", dev, sk, sk->sk_bound_dev_if);
		return;
	}

	err = strncpy_s(name, IFNAMSIZ, dev->name, IFNAMSIZ - 1);
	if (err != 0) {
		rcu_read_unlock();
		emcom_loge("strncpy_s failed, errcode: %d", err);
		return;
	}
	rcu_read_unlock();

	node = emcom_xengine_mpflow_get(uid_info.uid, name, sk->sk_bound_dev_if, uid_info.ver);
	if (!node)
		return;

	emcom_xengine_mpflow_fallback_report(sk, reason, state, name, node);
	return;
}

void emcom_mpflow_check_mpdns_addr(struct sock *sk, int reason, int result)
{
	bool need_clear = false;
	int init_net_type;
	int bound_net_type;
	if ((reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT ||
		reason == EMCOM_MPFLOW_FALLBACK_SYN_TOUT_FAST) &&
		result == EMCOM_MPFLOW_FALLBACK_SET)
		need_clear = true;
	if (!need_clear)
		return;

	init_net_type = emcom_xengine_get_net_type(sk->sk_mark & 0x00FF);
	bound_net_type = emcom_xengine_get_sock_bound_net_type(sk);
	if (bound_net_type == EMCOM_XENGINE_NET_INVALID || bound_net_type == init_net_type)
		return;

	if (emcom_xengine_is_v6_sock(sk)) {
		struct in6_addr *v6_addr = &(sk->sk_v6_daddr);
		emcom_mpdns_clear_v6_addr(v6_addr, bound_net_type);
	} else {
		struct in_addr v4_addr = {.s_addr = sk->sk_daddr};
		emcom_mpdns_clear_v4_addr(&v4_addr, bound_net_type);
	}
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("xengine module driver");

