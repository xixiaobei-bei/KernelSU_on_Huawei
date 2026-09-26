

#include "wbc_hw_hook.h"
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <uapi/linux/netlink.h>
#include <net/sock.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <net/ndisc.h>
#include <net/addrconf.h>
#include <huawei_platform/log/hw_log.h>
#include "chr_netlink.h"

#define WLAN_NET "wlan0"
#define RA_DEFAULT 10
#define RA_PACKET_SHORT 11
#define RA_INVALID_ND 12
#define RA_PREFIX_SHORT 13
#define RA_NO_PREFIX 14
#define RA_HAS_DNS_OPTION 1
#define LEFT_OPT_LENGTH 3

static spinlock_t g_ipv6_addr_lock;
static struct http_return v6_addr_info[RNT_STAT_SIZE] = {{0}};

static inline int ndisc_is_useropt(const struct net_device *dev,
	struct nd_opt_hdr *opt)
{
	return opt->nd_opt_type == ND_OPT_RDNSS ||
		opt->nd_opt_type == ND_OPT_DNSSL ||
		ndisc_ops_is_useropt(dev, opt->nd_opt_type);
}

static struct nd_opt_hdr *ndisc_next_useropt(const struct net_device *dev,
	struct nd_opt_hdr *cur, struct nd_opt_hdr *end)
{
	if (cur == NULL || end == NULL || cur >= end)
		return NULL;

	/* nd_opt_len field of RA 's ratio is 8 */
	cur = ((void *)cur) + (cur->nd_opt_len << LEFT_OPT_LENGTH);
	while (cur < end && !ndisc_is_useropt(dev, cur))
		cur = ((void *)cur) + (cur->nd_opt_len << LEFT_OPT_LENGTH);
	return cur <= end && ndisc_is_useropt(dev, cur) ? cur : NULL;
}

static bool is_router_advertise(struct sk_buff *skb)
{
	struct ipv6hdr *ip6h = NULL;
	struct icmp6hdr *hdr = NULL;

	if (ntohs(skb->protocol) != ETH_P_IPV6)
		return false;

	ip6h = ipv6_hdr(skb);
	if (ip6h != NULL && ip6h->nexthdr == IPPROTO_ICMPV6) {
		hdr = icmp6_hdr(skb);
		if (hdr != NULL
			&& hdr->icmp6_type == NDISC_ROUTER_ADVERTISEMENT) {
			if (strncmp(skb->dev->name,
				WLAN_NET, WEB_DS_NET_LEN) == 0)
				return true;
		}
	}
	return false;
}

static bool parse_prefix(struct ndisc_options ndopts)
{
	struct prefix_info *pinfo = NULL;
	struct nd_opt_hdr *p = NULL;

	if (ndopts.nd_opts_pi != NULL) {
		u8 *opt = (u8 *)ndopts.nd_opts_pi;

		p = ndopts.nd_opts_pi;
		pinfo = (struct prefix_info *)opt;

		/* nd_opt_len field of RA 's ratio is 8 */
		if ((p->nd_opt_len << LEFT_OPT_LENGTH) <
			sizeof(struct prefix_info)) {
			v6_addr_info[WLAN_INTERFACE].v6.reason =
				RA_PREFIX_SHORT;
			return false;
		}
		v6_addr_info[WLAN_INTERFACE].v6.prefix_len =
			pinfo->prefix_len;
		v6_addr_info[WLAN_INTERFACE].v6.valid_time =
			ntohl(pinfo->valid);
		v6_addr_info[WLAN_INTERFACE].v6.prefer_time =
			ntohl(pinfo->prefered);
		v6_addr_info[WLAN_INTERFACE].v6.autoconf =
			pinfo->autoconf;
	} else {
		v6_addr_info[WLAN_INTERFACE].v6.reason = RA_NO_PREFIX;
	}
	return true;
}

void ipv6_spin_lock_init(void)
{
	spin_lock_init(&g_ipv6_addr_lock);
}

void try_upload_v6_info(void)
{
	if (v6_addr_info[WLAN_INTERFACE].v6.reason != 0) {
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			v6_addr_info);
		v6_addr_info[WLAN_INTERFACE].v6.reason = 0;
		v6_addr_info[WLAN_INTERFACE].v6.router_time = 0;
		v6_addr_info[WLAN_INTERFACE].v6.managed = 0;
		v6_addr_info[WLAN_INTERFACE].v6.other = 0;
		v6_addr_info[WLAN_INTERFACE].v6.prefix_len = 0;
		v6_addr_info[WLAN_INTERFACE].v6.prefer_time = 0;
		v6_addr_info[WLAN_INTERFACE].v6.valid_time = 0;
		v6_addr_info[WLAN_INTERFACE].v6.autoconf = 0;
		v6_addr_info[WLAN_INTERFACE].v6.dns_option = 0;
	}
}

static bool ndisc_suppress_frag_ndisc(struct sk_buff *skb)
{
	struct inet6_dev *idev = __in6_dev_get(skb->dev);

	if (idev == NULL)
		return true;
	if ((IP6CB(skb)->flags & IP6SKB_FRAGMENTED) &&
		idev->cnf.suppress_frag_ndisc)
		return true;
	return false;
}

static bool check_packet_succ(struct sk_buff *skb)
{
	struct nd_msg *msg = NULL;
	struct ipv6hdr *ip6h = NULL;

	if (skb == NULL || skb->dev == NULL || !is_router_advertise(skb))
		return false;

	if (ndisc_suppress_frag_ndisc(skb))
		return false;

	msg = (struct nd_msg *)skb_transport_header(skb);

	ip6h = ipv6_hdr(skb);
	if (ip6h == NULL || ip6h->hop_limit != 255) /* max hop limit is 255 */
		return false;

	if (msg == NULL || msg->icmph.icmp6_code != 0)
		return false;

	if (!((u32)ipv6_addr_type(&ip6h->saddr) & IPV6_ADDR_LINKLOCAL))
		return false;

	return true;
}

void chr_router_discovery(struct sk_buff *skb)
{
	struct ndisc_options ndopts;
	struct ra_msg *ra_msg = NULL;
	struct nd_opt_hdr *p = NULL;
	int optlen;

	if (!check_packet_succ(skb))
		return;
	if (!spin_trylock_bh(&g_ipv6_addr_lock))
		return;
	ra_msg = (struct ra_msg *)skb_transport_header(skb);
	optlen = (skb_tail_pointer(skb) - skb_transport_header(skb))
		- sizeof(struct ra_msg);
	v6_addr_info[WLAN_INTERFACE].v6.reason = RA_DEFAULT;
	if (optlen < 0) {
		v6_addr_info[WLAN_INTERFACE].v6.reason = RA_PACKET_SHORT;
		goto upload;
	}
	v6_addr_info[WLAN_INTERFACE].v6.router_time =
		ntohs(ra_msg->icmph.icmp6_rt_lifetime);
	v6_addr_info[WLAN_INTERFACE].v6.managed =
		ra_msg->icmph.icmp6_addrconf_managed;
	v6_addr_info[WLAN_INTERFACE].v6.other =
		ra_msg->icmph.icmp6_addrconf_other;
	if (!ndisc_parse_options(skb->dev,
		(__u8 *)(ra_msg + 1), optlen, &ndopts)) {
		v6_addr_info[WLAN_INTERFACE].v6.reason = RA_INVALID_ND;
		goto upload;
	}
	if (!parse_prefix(ndopts))
		goto upload;

	if (ndopts.nd_useropts != NULL) {
		for (p = ndopts.nd_useropts;
			p != NULL;
			p = ndisc_next_useropt(skb->dev,
				p, ndopts.nd_useropts_end)) {
			if (p->nd_opt_type == ND_OPT_RDNSS)
				v6_addr_info[WLAN_INTERFACE].v6.dns_option =
					RA_HAS_DNS_OPTION;
		}
	}
upload:
	v6_addr_info[WLAN_INTERFACE].report_type = WEB_STAT;
	spin_unlock_bh(&g_ipv6_addr_lock);
}
