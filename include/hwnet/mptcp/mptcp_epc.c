/*
 * mptcp_epc.c
 *
 * EPC Feature depending on MPTCP
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <net/checksum.h>
#include <net/mptcp_epc.h>

struct mptcp_hw_ext_hag_enable_all_app_info {
	bool enable_all_app;
	struct mptcp_hw_ext_hag_gw_info hag_gw_info;
	struct mptcp_hw_ext_hag_config hag_config;
};

struct mptcp_hw_ext_hag_enable_all_app_info mptcp_hag_proxy = {
	.enable_all_app = false,
	.hag_config.black_list_num = 0,
	.hag_gw_info.udp_enable = 0,
};

bool mptcp_hw_is_enable_hag_config(void)
{
	return mptcp_hag_proxy.enable_all_app;
}

static bool mptcp_hw_is_hag_gw_black_list(kuid_t uid)
{
	uint8_t index;

	if (uid.val == INVALID_UID.val)
		return false;

	for (index = 0; index < mptcp_hag_proxy.hag_config.black_list_num;
	     index++)
		if (mptcp_hag_proxy.hag_config.black_app_list[index] == uid.val)
			return true;

	return false;
}

bool mptcp_hw_is_enable_hag_gw(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (!mptcp_hag_proxy.enable_all_app)
		return false;

	return !mptcp_hw_is_hag_gw_black_list(sock_i_uid(tp->meta_sk ?
							tp->meta_sk : sk));
}

void mptcp_hw_ext_set_hag_gw_info(struct mptcp_hw_ext *ext)
{
	switch (ext->cmd) {
	case MPTCP_HW_EXT_SET_CONF_HAG_GW_INFO: {
		mptcp_hag_proxy.hag_gw_info = ext->conf.hag_gw_info;
		mptcp_hag_proxy.enable_all_app =
					ext->conf.hag_gw_info.lte_ip != 0 &&
					ext->conf.hag_gw_info.wifi_ip != 0 &&
					ext->conf.hag_gw_info.port != 0 &&
					ext->conf.hag_gw_info.tunnel_id != 0;
		break;
	}

	case MPTCP_HW_EXT_SET_CONF_HAG_CONFIG: {
		(void)memcpy(&mptcp_hag_proxy.hag_config,
				&ext->conf.hag_config,
				sizeof(struct mptcp_hw_ext_hag_config));
		break;
	}

	default:
		break;
	}
}

static bool mutp_is_enable_hag_gw(struct sock *sk)
{
	if (!mptcp_hag_proxy.hag_gw_info.udp_enable)
		return false;

	return !mptcp_hw_is_hag_gw_black_list(sock_i_uid(sk));
}

static bool check_ip_addrss_for_mutp_available(struct sockaddr *addr)
{
	return check_ip_addrss_for_mptcp_available(addr) && check_ip_addrss_public_available(addr);
}

static struct mutp_header *mutp_hdr(struct sk_buff *skb)
{
	return (struct mutp_header *)(skb_transport_header(skb) +
					sizeof(struct udphdr));
}

static struct iphdr *mutp_ip_hdr(const struct sk_buff *skb)
{
	return (struct iphdr *)(skb_transport_header(skb) +
				sizeof(struct udphdr) +
				MUTP_HEADER_LEN);
}

static struct udphdr *mutp_udp_hdr(const struct sk_buff *skb)
{
	return (struct udphdr *)(skb_transport_header(skb) +
				sizeof(struct udphdr) +
				MUTP_HEADER_LEN +
				(skb->transport_header - skb->network_header));
}

static bool is_net_dev_running(struct net *net, const char *name)
{
	bool result = false;
	struct net_device *net_dev = dev_get_by_name(net, name);

	if (net_dev) {
		result = dev_get_flags(net_dev) & IFF_RUNNING ? true : false;
		dev_put(net_dev);
	}

	return result;
}

static bool is_rmnet_net_dev_running(struct net *net)
{
	bool result = false;
	struct net_device *dev;

	rcu_read_lock();
	for_each_netdev_rcu(net, dev) {
		if ((dev_get_flags(dev) & IFF_RUNNING) &&
			!memcmp(dev->name, "rmnet", RMNET_STR_LEN)) {
			result = true;
			break;
		}
	}
	rcu_read_unlock();

	return result;
}

static int get_sock_dev_if(struct sock *sk)
{
	int dev_if = DEV_IF_WIFI;
	struct net *net = sock_net(sk);

	if (!sk->sk_bound_dev_if) {
		if (!is_net_dev_running(net, "wlan0"))
			dev_if = is_rmnet_net_dev_running(net) ? DEV_IF_RMNET :
								DEV_IF_OTHER;
	} else {
		struct net_device *net_dev = dev_get_by_index(net,
							sk->sk_bound_dev_if);

		if (net_dev) {
			if (memcmp(net_dev->name, "rmnet", 5) == 0)
				dev_if = DEV_IF_RMNET;
			else if (memcmp(net_dev->name, "wlan0", 5) != 0)
				dev_if = DEV_IF_OTHER;

			dev_put(net_dev);
		}
	}
	return dev_if;
}

void mutp_udp_send_check(struct udphdr *udph, struct iphdr *iph)
{
	int udp_len = ntohs(udph->len);

	udph->check = 0;
	udph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, udp_len,
					IPPROTO_UDP,
					csum_partial(udph, udp_len, 0));
	if (udph->check == 0) {
		mptcp_info("udph->check zeor, must be set CSUM_MANGLED_0\n");
		udph->check = CSUM_MANGLED_0;
	}
}

void mptcp_teid_syn_options(const struct sock *sk, struct tcp_out_options *opts,
	unsigned int *remaining)
{
	if (*remaining < MPTCP_SUB_LEN_TEID_ALIGN) {
		tcp_write_err((struct sock *)sk);

		mptcp_debug("%s: not enough space to add new teid option.\n",
				__func__);
		return;
	}
	opts->mptcp_options |= OPTION_MP_OPT_TEID;
	*remaining -= MPTCP_SUB_LEN_TEID_ALIGN;

	mptcp_debug("%s: add options OPTION_MP_OPT_TEID.\n", __func__);
}

void mptcp_teid_options_write(__be32 *ptr, struct tcp_sock *tp,
			 const struct tcp_out_options *opts,
			 struct sk_buff *skb)
{
	if (unlikely(OPTION_MP_OPT_TEID & opts->mptcp_options)) {
		struct mp_option_teid *mp_teid = (struct mp_option_teid *)ptr;
		u8 *align = (u8 *)(mp_teid + 1);

		mp_teid->kind = TCPOPT_MPTCP;
		mp_teid->len = MPTCP_SUB_LEN_TEID;
		mp_teid->sub = MPTCP_SUB_TEID;
		mp_teid->rsv = 0;
		mp_teid->teid = ntohl(mptcp_hag_proxy.hag_gw_info.tunnel_id);
		ptr += MPTCP_SUB_LEN_TEID_ALIGN >> 2;

		/* Fill the last align with NOP's */
		*align = TCPOPT_NOP;
	}
}

void mptcp_gw_rewrite_dst_addr(struct sock *sk, struct sockaddr *uaddr)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;
	int dev_if = get_sock_dev_if(sk);

	if (dev_if != DEV_IF_OTHER)
		(void)memcpy((void *)&(tp->server_addr), (const void *)uaddr,
			     sizeof(struct sockaddr));

	switch (dev_if) {
	case DEV_IF_WIFI:
		usin->sin_addr.s_addr =
				htonl(mptcp_hag_proxy.hag_gw_info.wifi_ip);
		usin->sin_port = htons(mptcp_hag_proxy.hag_gw_info.port);
		mptcp_debug("%s: sk_bound_dev_if == 0, write wifi ip.\n",
				__func__);
		break;

	case DEV_IF_RMNET:
		usin->sin_addr.s_addr =
				htonl(mptcp_hag_proxy.hag_gw_info.lte_ip);
		usin->sin_port = htons(mptcp_hag_proxy.hag_gw_info.port);
		mptcp_debug("%s: sk_bound_dev_if == lte, write lte ip.\n",
				__func__);
		break;

	default:
		mptcp_debug("%s: not rewrite dst addr.\n", __func__);
		break;
	}
}

int mptcp_hw_socks_recv(struct sock *sk, struct sk_buff *skb)
{
	struct sock *meta_sk = mptcp_meta_sk(sk);
	struct iphdr *iph = ip_hdr(skb);
	unsigned char *payload = skb->data;
	unsigned int ip_hlen = ip_hdrlen(skb);
	unsigned int tcp_hlen = tcp_hdrlen(skb);
	unsigned int len = ntohs(iph->tot_len);

	if ((len < ip_hlen) || ((len - ip_hlen - tcp_hlen) != SOCKS4_ECHO_LEN))
		return -EFAULT;

	if (!meta_sk || !tcp_sk(meta_sk)->mpcb ||
	    (tcp_sk(meta_sk)->mpcb->socks_sate == 1))
		return -EFAULT;

	if (payload[1] == SOCKS4_REQUEST_GRANTED)
		tcp_sk(meta_sk)->mpcb->socks_sate = 1;
	else
		return -EFAULT;

	if (!sock_flag(meta_sk, SOCK_DEAD))
		sk_wake_async(meta_sk, SOCK_WAKE_IO, POLL_OUT);

	return 0;
}

void mptcp_hw_init_data_skb(struct sk_buff *skb, u32 seq, u32 data_len)
{
	skb->ip_summed = CHECKSUM_PARTIAL;
	skb->csum = 0;

	TCP_SKB_CB(skb)->tcp_flags = TCPHDR_PSH | TCPHDR_ACK;
	TCP_SKB_CB(skb)->sacked = 0;

	tcp_skb_pcount_set(skb, 0);

	TCP_SKB_CB(skb)->seq = seq;
	TCP_SKB_CB(skb)->end_seq = seq + data_len;
}

void mptcp_hw_socks4_send(struct sock *sk)
{
	int data_len = sizeof(struct socks4_request);
	struct sk_buff *skb;
	struct socks4_request *request;
	struct tcp_sock *tp = tcp_sk(sk);
	struct sock *meta_sk;
	struct tcp_sock *meta_tp;
	struct sockaddr_in *server_addr;

	if (!mptcp_hag_proxy.enable_all_app) {
		mptcp_debug("%s EPC disable.\n", __func__);
		return;
	}

	if (!mptcp(tp)) {
		mptcp_debug("%s not enable mptcp.\n", __func__);
		return;
	}

	meta_sk = mptcp_meta_sk(sk);
	meta_tp = tcp_sk(meta_sk);

	if (meta_tp->mptcp_cap_flag != MPTCP_CAP_ALL_APP) {
		mptcp_debug("%s: disable all app.\n", __func__);
		return;
	}

	server_addr = (struct sockaddr_in *) (&(tp->server_addr));
	if (server_addr->sin_addr.s_addr == 0 || server_addr->sin_port == 0) {
		mptcp_debug("%s: addr == null.\n", __func__);
		return;
	}

	skb = alloc_skb(data_len + MAX_TCP_HEADER, GFP_ATOMIC);
	if (IS_ERR_OR_NULL(skb)) {
		mptcp_debug("%s: buff alloc failed.\n", __func__);
		return;
	}

	/* Reserve space for headers and prepare control bits. */
	skb_reserve(skb, MAX_TCP_HEADER);
	mptcp_hw_init_data_skb(skb, meta_tp->write_seq, data_len);

	meta_tp->write_seq += data_len;
	meta_tp->pushed_seq = meta_tp->write_seq;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	skb_mstamp_get(&skb->skb_mstamp);
#endif

	request = (struct socks4_request *) skb_push(skb,
						sizeof(struct socks4_request));
	request->ver = SOCKS4_VERSON;
	request->cmd = SOCKS4_CMD_CONNECT;
	request->port = server_addr->sin_port;
	request->addr = server_addr->sin_addr.s_addr;
	request->user = 0x00;

	tcp_queue_skb(meta_sk, skb);
	__tcp_push_pending_frames(meta_sk, mptcp_current_mss(meta_sk),
				TCP_NAGLE_PUSH);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	skb->skb_mstamp = meta_tp->tcp_mstamp;
#endif
}

bool mutp_udp_csum_check(struct udphdr *udph, struct iphdr *iph)
{
	__sum16 sum;
	__sum16 received_sum = udph->check;

	mutp_udp_send_check(udph, iph);
	sum = udph->check;

	udph->check = received_sum;

	return received_sum == sum;
}

bool mutp_ip_csum_check(struct iphdr *iph)
{
	__sum16 sum;
	__sum16 received_sum = iph->check;

	ip_send_check(iph);
	sum = iph->check;

	iph->check = received_sum;

	return received_sum == sum;
}

bool mutp_check_mutp_message(struct sk_buff *skb)
{
	struct mutp_header *mutph;
	int mutp_offset = MUTP_HEADER_LEN + sizeof(struct iphdr) +
				sizeof(struct udphdr);

	if (skb_headlen(skb) < mutp_offset) {
		mptcp_debug("%s data len error\n", __func__);
		return false;
	}

	mutph = mutp_hdr(skb);
	if ((mutph->version != 1) || (mutph->type != MUTP_MSGTYPE_DATA) ||
	    (ntohs(mutph->len) != skb_headlen(skb))) {
		mptcp_debug("%s mutph format error\n", __func__);
		return false;
	}

	if (!mutp_ip_csum_check(mutp_ip_hdr(skb))) {
		mptcp_debug("%s mutp ip check csum error\n", __func__);
		return false;
	}

	if (!mutp_udp_csum_check(mutp_udp_hdr(skb), mutp_ip_hdr(skb))) {
		mptcp_debug("%s mutp udp check csum error\n", __func__);
		return false;
	}

	return true;
}

void mutp_rewrite_dst_addr(struct sock *sk, struct sockaddr *uaddr)
{
	struct sockaddr_in *usin = (struct sockaddr_in *)uaddr;

	if (mutp_is_enable_hag_gw(sk) &&
	    check_ip_addrss_for_mutp_available(uaddr) &&
	    usin->sin_port != 0) {
		struct udp_sock *up = udp_sk(sk);
		int dev_if = get_sock_dev_if(sk);

		if (dev_if != DEV_IF_OTHER)
			(void)memcpy((void *)&(up->server_addr),
				(const void *)uaddr, sizeof(struct sockaddr));

		switch (dev_if) {
		case DEV_IF_WIFI:
			usin->sin_addr.s_addr =
				htonl(mptcp_hag_proxy.hag_gw_info.wifi_ip);
			usin->sin_port =
				htons(mptcp_hag_proxy.hag_gw_info.port);
			mptcp_debug("%s: dev_if is wifi, write wifi ip.\n",
					__func__);
			break;

		case DEV_IF_RMNET:
			usin->sin_addr.s_addr =
				htonl(mptcp_hag_proxy.hag_gw_info.lte_ip);
			usin->sin_port =
				htons(mptcp_hag_proxy.hag_gw_info.port);
			mptcp_debug("%s: dev_if is lte, write lte ip.\n",
					__func__);
			break;

		default:
			mptcp_debug("%s: not rewrite dst addr.\n", __func__);
			break;
		}
	}
}

void mutp_fill_mutp_header(struct sock *sk, struct sk_buff *skb)
{
	struct udp_sock *up = udp_sk(sk);
	struct sockaddr_in *server_addr =
				(struct sockaddr_in *)&up->server_addr;

	if (!IS_ERR_OR_NULL(skb) && server_addr->sin_addr.s_addr != 0 &&
	    server_addr->sin_port != 0) {
		struct inet_sock *inet = inet_sk(sk);
		struct iphdr *iph = ip_hdr(skb);
		struct udphdr *udph = udp_hdr(skb);
		struct mutp_header *mutph = mutp_hdr(skb);
		struct iphdr *mutp_iph = mutp_ip_hdr(skb);
		struct udphdr *mutp_udph = mutp_udp_hdr(skb);
		uint32_t mutp_csum = 0;

		mutph->version = MUTP_VERSION;
		mutph->icheck = 0x00;
		mutph->spare = 0x00;
		mutph->type = MUTP_MSGTYPE_DATA;
		mutph->len = htons(skb->len - skb_transport_offset(skb) -
					sizeof(struct udphdr));
		mutph->tunnel_id = ntohl(mptcp_hag_proxy.hag_gw_info.tunnel_id);

		memcpy(mutp_iph, iph, iph->ihl * 4);
		mutp_iph->tot_len = htons(ntohs(mutph->len) - MUTP_HEADER_LEN);
		mutp_iph->saddr = htonl(mptcp_hag_proxy.hag_gw_info.ue_ip);
		mutp_iph->daddr = server_addr->sin_addr.s_addr;

		memcpy(mutp_udph, udph, sizeof(struct udphdr));
		mutp_udph->len = htons(ntohs(mutp_iph->tot_len) - iph->ihl * 4);
		mutp_udph->source = inet->inet_sport;
		mutp_udph->dest = server_addr->sin_port;

		mutp_udp_send_check(mutp_udph, mutp_iph);
		ip_send_check(mutp_iph);

		mutp_csum = csum_partial(mutph, MUTP_HEADER_LEN +
					mutp_iph->ihl * 4 +
					sizeof(struct udphdr), 0);
		skb->csum = csum_block_add(mutp_csum, skb->csum,
					MUTP_HEADER_LEN + mutp_iph->ihl * 4 +
					sizeof(struct udphdr));
	}
}

bool mutp_decode_recv(struct sk_buff *skb, bool is_udp4, int *off)
{
	if (!mptcp_hag_proxy.hag_gw_info.udp_enable) {
		mptcp_debug("%s udp disable.\n", __func__);
		return false;
	}

	if (!is_udp4) {
		mptcp_debug("%s not support ipv6\n", __func__);
		return false;
	}

	if (mutp_check_mutp_message(skb)) {
		*off += (MUTP_HEADER_LEN + mutp_ip_hdr(skb)->ihl * 4 +
			sizeof(struct udphdr));
		mptcp_debug("%s check csum right\n", __func__);
		return true;
	}

	return false;
}

void mutp_rewrite_msg_addr(bool is_mutp, struct sk_buff *skb,
			struct sockaddr_in *sin)
{
	if (!is_mutp) {
		sin->sin_port = udp_hdr(skb)->source;
		sin->sin_addr.s_addr = ip_hdr(skb)->saddr;
	} else {
		sin->sin_port = mutp_udp_hdr(skb)->source;
		sin->sin_addr.s_addr = mutp_ip_hdr(skb)->saddr;
	}
}

void mutp_rewrite_msg_addrv6(bool is_mutp, struct sk_buff *skb,
			struct sockaddr_in6 *sin6)
{
	if (!is_mutp) {
		ipv6_addr_set_v4mapped(ip_hdr(skb)->saddr, &sin6->sin6_addr);
	} else {
		ipv6_addr_set_v4mapped(mutp_ip_hdr(skb)->saddr,
					&sin6->sin6_addr);
		sin6->sin6_port = mutp_udp_hdr(skb)->source;
	}
}
