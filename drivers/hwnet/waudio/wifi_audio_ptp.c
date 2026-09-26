/*
 * wifi_audio_ptp.c
 *
 * wifi audio ptp implementation
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "wifi_audio_ptp.h"
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/moduleparam.h>
#include <net/sock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/netlink.h>
#include <securec.h>
#include <linux/sort.h>
#include <securec.h>
#include <linux/version.h>

#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
#include <linux/io.h>
#endif
#include "wifi_audio_utils.h"

static void ptp_send_stop_timesync(struct ptp_dev *dev);
static struct wifi_audio_ptp *ptp_infor = NULL;
static DEFINE_SPINLOCK(ptp_infor_lock);
extern int do_settimeofday64(const struct timespec64 *ts);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
extern int timekeeping_inject_offset(struct timespec *ts);
#else
extern int timekeeping_inject_offset(const struct timespec64 *ts);
#endif
static void time_sync_status_report(const struct ptp_dev *dev,
	int status, int reason);
static void ptp_timesync_set_status(struct ptp_dev *dev,
	int status);

static void net_dev_status_report(int status)
{
	struct netlink_event_report_net_dev net_dev_status = {0};
	struct netlink_data netlink_event = {0};
	int ret;

	net_dev_status.status = status;
	wifi_audio_log_info("net dev status %d", status);

	netlink_event.id = NETLINK_EVENT_REPORT_NET_DEV;
	netlink_event.msg_len = sizeof(net_dev_status);
	ret = memcpy_s(netlink_event.msg_data, NETLINK_MSG_DATA_LEN_MAX,
		&net_dev_status, sizeof(net_dev_status));
	if (ret != 0) {
		wifi_audio_log_err("memcpy failed");
		return;
	}

	ret = netlink_event_report(&netlink_event);
	if (ret != 0)
		wifi_audio_log_err("netlink_event_report fail");
	return;
}

void ptp_clear_by_dev(const struct net_device *dev_net)
{
	int channel;
	struct ptp_dev *dev = NULL;
	int need_report = 0;

	spin_lock_bh(&ptp_infor_lock);
	if ((ptp_infor == NULL) || (dev_net == NULL)) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("ptp is not start!");
		return;
	}
	for (channel = 0; channel < CHANNEL_MAX; channel++) {
		if ((ptp_infor->dev[channel].status == PTP_START) &&
			(ptp_infor->dev[channel].net_dev == dev_net)) {
			dev_put(ptp_infor->dev[channel].net_dev);
			wifi_audio_log_info("channel %d", channel);
			dev = &(ptp_infor->dev[channel]);
			ptp_send_stop_timesync(dev);
			ptp_infor->dev[channel].net_dev = NULL;
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
			if ((dev->role == PTP_MASTER) &&
				(ptp_infor->write_time_reg_timer_started == 1)) {
				wifi_audio_log_info("write time reg timer cancel");
				hrtimer_cancel(&(ptp_infor->write_time_reg_timer));
				ptp_infor->write_time_reg_timer_started = 0;
			}
#endif
			ptp_timesync_set_status(dev, PTP_ABNORMAL);
			need_report = 1;
		}
	}
	if (need_report == 1)
		net_dev_status_report(NET_DEV_ABNORMAL);
	spin_unlock_bh(&ptp_infor_lock);
	return;
}

static int get_delay_event_id(const struct ptp_dev *dev,
	unsigned char data)
{
	int id;
	if (dev == NULL) {
		wifi_audio_log_err("dev is NULL Pointer");
		return PTP_EVENT_ID_MAX;
	}
	switch (data) {
	case DELAY_REQ: {
		if (dev->role == PTP_MASTER)
			id = PTP_RECEIVE_DELAY_REQUEST;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	case DELAY_RESP: {
		if (dev->role == PTP_SLAVE)
			id = PTP_RECEIVE_DELAY_RESPONSE;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	case SYNC_DONE: {
		if (dev->role == PTP_MASTER)
			id = PTP_RECEIVE_SYNC_DONE;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	default:
		id = PTP_EVENT_ID_MAX;
		break;
	}
	return id;
}

static int get_ptp_event_data_id(const struct ptp_dev *dev,
	unsigned char data)
{
	int id;
	if (dev == NULL) {
		wifi_audio_log_err("dev is NULL Pointer");
		return PTP_EVENT_ID_MAX;
	}
	switch (data) {
	case SYNC_REQ: {
		if (dev->role == PTP_MASTER)
			id = PTP_SEND_SYNC_REQUEST;
		else if (dev->role == PTP_SLAVE)
			id = PTP_RECEIVE_SYNC_REQUEST;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	case DELAY_REQ:
	case DELAY_RESP:
	case SYNC_DONE: {
		id = get_delay_event_id(dev, data);
		break;
	}
	case RESTART_TIME_SYNC: {
		if (dev->role == PTP_SLAVE)
			id = PTP_RECEIVE_RESTART_TIME_SYNC;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	case STOP_TIME_SYNC: {
		if ((dev->role == PTP_SLAVE) || (dev->role == PTP_MASTER))
			id = PTP_RECEIVE_STOP_TIME_SYNC;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	default:
		id = PTP_EVENT_ID_MAX;
		break;
	}
	return id;
}

static int get_ptp_event_data_id_udp(const struct ptp_dev *dev,
	const unsigned char *msg, unsigned int msg_len)
{
	int id;
	unsigned int cmd;
	if ((dev == NULL) || (msg == NULL)) {
		wifi_audio_log_err("dev is NULL Pointer");
		return PTP_EVENT_ID_MAX;
	}
	/* 12-15 is cmd code */
	cmd = msg[12] | (msg[13] << 8) | (msg[14] << 16) | (msg[15] << 24);
	switch (cmd) {
	case SYNC_REQ: {
		if (dev->role == PTP_SLAVE)
			id = PTP_RECEIVE_SYNC_REQUEST_UDP;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
 	case DELAY_REQ: {
		if (dev->role == PTP_SLAVE)
			id = PTP_RECEIVE_DELAY_REQUEST_UDP;
		else
			id = PTP_EVENT_ID_MAX;
		break;
	}
	default:
		id = PTP_EVENT_ID_MAX;
		break;
	}
	return id;
}

static int waudio_ip_match(int ip_dest, int ip_source,
	const struct wifi_audio_ptp *ptp, unsigned char protocol)
{
	int i;
	if (ptp == NULL) {
		wifi_audio_log_err("NULL Pointer");
		return 0;
	}
	for (i = 0; i < CHANNEL_MAX; i++) {
		if (ptp->dev[i].status == PTP_START) {
			if ((ptp->dev[i].ip_source == ip_dest) &&
				(ptp->dev[i].ip_dest == ip_source) &&
				(ptp->dev[i].protocol == protocol))
				return 1;
		}
	}
	return 0;
}

static int is_waudio_skb(const struct sk_buff *skb,
	const struct wifi_audio_ptp *ptp)
{
	struct ethhdr *ethdr = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	struct udphdr *uh = NULL;
	int ip_dset;
	int ip_source;
	int port_match = 0;

	if ((ptp == NULL) || (skb == NULL))
		return 0;

	ethdr = (struct ethhdr *)(skb->data);
	if (__constant_htons(ETH_P_IP) == ethdr->h_proto) {
		iph = (struct iphdr *)(skb->data + ETH_HLEN);
		if (iph->protocol == IPPROTO_TCP) {
			tcph = (struct tcphdr *)(skb->data + ETH_HLEN + sizeof(*iph));
			if ((tcph->source == SKB_TCP_SOURCE_PORT) ||
				(tcph->dest == SKB_TCP_DEST_PORT))
				port_match = 1;
		} else if (iph->protocol == IPPROTO_UDP) {
			uh = (struct udphdr *)(skb->data + ETH_HLEN + sizeof(*iph));
			if ((uh->source == __constant_htons(SKB_UDP_SOURCE_PORT)) ||
				(uh->dest == __constant_htons(SKB_UDP_DEST_PORT)))
				port_match = 1;
		}
		if (port_match == 1) {
			ip_dset = iph->daddr;
			ip_source = iph->saddr;
			if (waudio_ip_match(ip_dset, ip_source, ptp,
				iph->protocol))
				return 1;
		}
	}
	return 0;
}

static int pcm_skb_match(__be32 ip_dest, __be32 ip_source,
	__be16 port_dest, const struct wifi_audio_ptp *ptp)
{
	int i;
	if (ptp == NULL) {
		wifi_audio_log_err("NULL Pointer");
		return 0;
	}
	for (i = 0; i < CHANNEL_MAX; i++)
		if (ptp->dev[i].status == PTP_START)
			if ((ptp->dev[i].ip_source == ip_source) &&
				(ptp->dev[i].ip_dest == ip_dest) &&
				(ptp->dev[i].data_port ==
				__constant_htons(port_dest)))
				return 1;
	return 0;
}

static int is_pcm_skb_mark(const struct sk_buff *skb,
	const struct wifi_audio_ptp *ptp)
{
	struct ethhdr *ethdr = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	__be32 ip_dset;
	__be32 ip_source;
	__be16 port_dest;

	if ((ptp == NULL) || (skb == NULL))
		return 0;

	ethdr = eth_hdr(skb);
	iph = ip_hdr(skb);
	if ((ethdr->h_proto == __constant_htons(ETH_P_IP)) &&
		(iph->protocol == IPPROTO_UDP)) {
		udph = udp_hdr(skb);
		ip_dset = iph->daddr;
		ip_source = iph->saddr;
		port_dest = udph->dest;
		if (pcm_skb_match(ip_dset, ip_source, port_dest, ptp))
			return 1;
	}
	return 0;
}

static void skb_tx_handle(struct sk_buff *skb, struct net_device *dev)
{
	if (ptp_infor == NULL)
		return;

	if (ptp_infor->num == 0)
		return;

	if ((skb == NULL) || (dev == NULL))
		return;
	if (is_pcm_skb_mark(skb, ptp_infor)) {
		skb->mark = 0x5a; /* wifi vip mark */
		return;
	}
}

static int skb_rx_handle(struct sk_buff *skb)
{
	struct list_node *node = NULL;
	struct sk_buff *pskb = NULL;
	struct timespec64 tv;

	if (ptp_infor == NULL)
		return ERROR;

	if (ptp_infor->num == 0)
		return ERROR;

	if (skb == NULL)
		return ERROR;
	pskb = skb;
	skb_push(pskb, ETH_HLEN);
	if (is_waudio_skb(pskb, ptp_infor)) {
		getnstimeofday64(&tv);
		if (list_queue_is_empty(&(ptp_infor->waudio_skb_buffer)) == 0) {
			node = list_queue_dequeue(&(ptp_infor->waudio_skb_buffer));
			((struct waudio_skb *)(node->data))->skb = pskb;
			((struct waudio_skb *)(node->data))->stamp.tv_sec =
				tv.tv_sec;
			((struct waudio_skb *)(node->data))->stamp.tv_nsec =
				tv.tv_nsec;
			list_queue_enqueue(&(ptp_infor->waudio_skb_handle), node);
			tasklet_hi_schedule(&(ptp_infor->event_handle_work));
		} else {
			wifi_audio_log_err("waudio_skb_buffer is empty, error");
			dev_kfree_skb_any(pskb);
		}
		return SUCCESS;
	} else {
		skb_pull(pskb, ETH_HLEN);
		return ERROR;
	}
}

static void ptp_event_data_handle(struct wifi_audio_ptp *audio_ptp,
	struct ptp_event_data *event, int protocol)
{
	struct ptp_dev *dev = NULL;
	struct list_node *node = NULL;
	struct wifi_audio_ptp *ptp = NULL;
	int i;
	if ((audio_ptp == NULL) || (event == NULL)) {
		wifi_audio_log_err("NULL Pointer");
		return;
	}
	ptp = audio_ptp;

	for (i = 0; i < CHANNEL_MAX; i++) {
		if (ptp->dev[i].status == PTP_START) {
			if ((ptp->dev[i].ip_source == event->ip_dest) &&
				(ptp->dev[i].ip_dest == event->ip_source)) {
					dev = &(ptp->dev[i]);
				break;
			}
		}
	}
	if (dev == NULL) {
		wifi_audio_log_err("not find dev, error");
		return;
	}
	if ((event->id < PTP_SEND_SYNC_REQUEST) || (event->id >=
		PTP_EVENT_ID_MAX)) {
		if (protocol == IPPROTO_TCP)
			event->id = get_ptp_event_data_id(dev,
				event->msg[MSG_SUB_TYPE_INDEX]);
		else if (protocol == IPPROTO_UDP)
			event->id = get_ptp_event_data_id_udp(dev,
				event->msg, MSG_LEN);
	}
	node = alloc_list_node((void *)event, sizeof(*event));
	if (unlikely(node == NULL)) {
		wifi_audio_log_err("alloc node failed");
		return;
	}
	list_queue_enqueue(&(dev->dev_event_queue), node);
	tasklet_hi_schedule(&(dev->dev_work));
	return;
}

static void event_handle_work_handle(unsigned long data)
{
	struct wifi_audio_ptp *ptp = NULL;
	struct list_node *node = NULL;
	struct ptp_event_data event = {0};
	struct waudio_skb skb_data = {0};
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;
	char *pdata = NULL;
	int sec_ret;
	int protocol;

	spin_lock_bh(&ptp_infor_lock);
	ptp = (struct wifi_audio_ptp *)data;
	if (ptp == NULL) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	while (list_queue_is_empty(&(ptp->waudio_skb_handle)) == 0) {
		node = list_queue_dequeue(&(ptp->waudio_skb_handle));
		if (node == NULL) {
			wifi_audio_log_err("dequeue a null node");
			break;
		}
		sec_ret = memcpy_s(&skb_data, sizeof(skb_data), node->data,
			node->len);
		if (sec_ret != 0) {
			wifi_audio_log_err("memcpy node->data fail \n");
			dev_kfree_skb_any(((struct waudio_skb *)(node->data))->skb);
			list_queue_enqueue(&(ptp->waudio_skb_buffer), node);
			continue;
		}
		iph = (struct iphdr *)(skb_data.skb->data + ETH_HLEN);
		protocol = iph->protocol;
		if (protocol == IPPROTO_TCP) {
			tcph = (struct tcphdr *)(skb_data.skb->data + ETH_HLEN + sizeof(*iph));
			pdata = (char *)(skb_data.skb->data + ETH_HLEN + sizeof(*iph) +
				sizeof(*tcph));
		} else if (protocol == IPPROTO_UDP) {
			udph = (struct udphdr *)(skb_data.skb->data + ETH_HLEN + sizeof(*iph));
			pdata = (char *)(skb_data.skb->data + ETH_HLEN + sizeof(*iph) +
				sizeof(*udph));
		}
		sec_ret = memcpy_s(event.msg, MSG_LEN, pdata, MSG_LEN);
		if (sec_ret != 0) {
			wifi_audio_log_err("memcpy event.msg fail");
			dev_kfree_skb_any(((struct waudio_skb *)(node->data))->skb);
			list_queue_enqueue(&(ptp->waudio_skb_buffer), node);
			continue;
		}
		event.stamp.tv_sec = skb_data.stamp.tv_sec;
		event.stamp.tv_nsec = skb_data.stamp.tv_nsec;
		event.ip_dest = iph->daddr;
		event.ip_source = iph->saddr;
		event.id = PTP_EVENT_ID_MAX;
		dev_kfree_skb_any(((struct waudio_skb *)(node->data))->skb);
		list_queue_enqueue(&(ptp->waudio_skb_buffer), node);
		ptp_event_data_handle(ptp, &event, protocol);
	}
	spin_unlock_bh(&ptp_infor_lock);
	return;
}

static int ptp_send_data(struct ptp_dev *dev, const char *data,
	unsigned int len)
{
	struct sk_buff *skb = NULL;
	struct ethhdr *ethdr = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	char *pdata = NULL;
	int sec_ret;

	if ((dev == NULL) || (data == NULL) || (dev->net_dev == NULL)) {
		wifi_audio_log_err("NULL Pointer");
		return ERROR;
	}
	if (len > MSG_LEN) {
		wifi_audio_log_err("len is invalid");
		return ERROR;
	}
	skb = netdev_alloc_skb(dev->net_dev, SKB_DATA_LEN_MAX);
	if (skb == NULL) {
		wifi_audio_log_err("skb alloc failed, error");
		return ERROR;
	}
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = __constant_htons(ETH_P_IP);
	skb->ip_summed = CHECKSUM_NONE;
	skb->priority = 6; /* set priority VO */
	skb->mark = 0x5a; /* wifi vip mark */
	skb_reset_mac_header(skb);
	ethdr = (struct ethhdr *)skb_put(skb, sizeof(*ethdr));
	sec_ret = memcpy_s(ethdr->h_dest, ETH_ALEN, dev->mac_dest, ETH_ALEN);
	sec_ret += memcpy_s(ethdr->h_source, ETH_ALEN, dev->mac_source, ETH_ALEN);
	if (sec_ret != 0) {
		wifi_audio_log_err("memcpy_s failed, error");
		dev_kfree_skb_any(skb);
		return ERROR;
	}
	ethdr->h_proto = __constant_htons (ETH_P_IP);
	skb_set_network_header(skb, sizeof(*ethdr));
	iph = (struct iphdr*)skb_put(skb, sizeof(*iph));
	iph->version = 4; /* set IP version IPV4 */
	iph->ihl = sizeof(*iph) >> 2; /* Calculate and set IP header len */
	iph->frag_off = 0;
	iph->protocol = IPPROTO_TCP;
	iph->tos = 0xB8; /* set TOS voice type 0xB8 for SKB */
	iph->daddr = dev->ip_dest;
	iph->saddr = dev->ip_source;
	iph->ttl = 0x40; /* set TTL 0x40 for SKB */
	iph->check = 0;
	skb_set_transport_header(skb, sizeof(*ethdr) + sizeof(*iph));
	tcph = (struct tcphdr *)skb_put(skb, sizeof(*tcph));
	(void)memset_s(tcph, sizeof(*tcph), 0, sizeof(*tcph));
	tcph->source = SKB_TCP_SOURCE_PORT;
	tcph->dest = SKB_TCP_DEST_PORT;
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5;  /* set doff 5 for SKB */
	tcph->psh = 0;
	tcph->fin = 0;
	tcph->ack = 1;
	tcph->window = __constant_htons (65535); /* set window 65535 */
	skb->csum = 0;
	tcph->check = 0;
	pdata = skb_put (skb, len);
	sec_ret = memcpy_s(pdata, len, data, len);
	if (sec_ret != 0) {
		wifi_audio_log_err("memcpy_s failed, error");
		dev_kfree_skb_any(skb);
		return ERROR;
	}
	iph->tot_len = __constant_htons(skb->len);
	/* skb checksum */
	skb->csum = skb_checksum (skb, iph->ihl * 4, skb->len - (iph->ihl * 4),
		0);
	/* Calculate a partial checksum */
	tcph->check = csum_tcpudp_magic (dev->ip_source, dev->ip_dest,
		skb->len - (iph->ihl * 4), IPPROTO_TCP, skb->csum);
	ip_send_check(iph);
	if (unlikely(dev->net_dev->netdev_ops == NULL)) {
		wifi_audio_log_err("netdev_ops is NULL, error");
		dev_kfree_skb_any(skb);
		return ERROR;
	}
	dev->net_dev->netdev_ops->ndo_start_xmit(skb, dev->net_dev);
	return SUCCESS;
}

static void ptp_packet_udp_data(struct sk_buff *skb, struct ptp_dev *dev,
	unsigned int data_len)
{
	struct ethhdr *ethdr = NULL;
	struct iphdr *iph = NULL;
	struct udphdr *udph = NULL;
	int sec_ret;

	if ((dev == NULL) || (skb == NULL))
		return;

	if (data_len > UDP_MSG_LEN)
		return;

	udph = (struct udphdr *)skb_push(skb, sizeof(struct udphdr));
	(void)memset_s(udph, sizeof(struct udphdr), 0, sizeof(struct udphdr));
	udph->source = htons(SKB_UDP_SOURCE_PORT);
	udph->dest = htons(SKB_UDP_DEST_PORT);
	skb->csum = 0;
	udph->len = htons(sizeof(struct udphdr) + data_len);
	udph->check = 0;
	skb_reset_transport_header(skb);

	iph = (struct iphdr *)skb_push(skb, sizeof(struct iphdr));
	iph->version = 4;
	/* IP header calculation */
	iph->ihl = sizeof(struct iphdr) >> 2;
	dev->id_udp++;
	iph->id = htons(dev->id_udp);
	iph->frag_off = htons(0X4000);
	iph->protocol = IPPROTO_UDP;
	iph->tos = 0xB8; /* set TOS voice type 0xB8 for SKB */
	iph->saddr = dev->ip_source;
	iph->daddr = dev->ip_dest;
	iph->ttl = 0x40; /* set TTL 0x40 for SKB */
	iph->tot_len = htons(data_len + sizeof(struct iphdr) +
		sizeof(struct udphdr));
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
	skb_reset_network_header(skb);

	/* skb checksum */
	skb->csum = skb_checksum (skb, iph->ihl * 4, skb->len - (iph->ihl * 4), 0);
	/* Calculate a partial checksum */
	udph->check = csum_tcpudp_magic (dev->ip_source, dev->ip_dest,
		skb->len - (iph->ihl * 4), IPPROTO_UDP, skb->csum);

	ethdr = (struct ethhdr *)skb_push(skb, sizeof(struct ethhdr));
	sec_ret = memcpy_s(ethdr->h_dest, ETH_ALEN, dev->mac_dest, ETH_ALEN);
	sec_ret += memcpy_s(ethdr->h_source, ETH_ALEN, dev->mac_source, ETH_ALEN);
	if (sec_ret != 0) {
		wifi_audio_log_err("memcpy_s failed, error");
		dev_kfree_skb_any(skb);
	}
	ethdr->h_proto = __constant_htons (ETH_P_IP);
	skb_reset_mac_header(skb);

	if (unlikely(dev->net_dev->netdev_ops == NULL)) {
		wifi_audio_log_err("netdev_ops is NULL, error");
		dev_kfree_skb_any(skb);
	}
	return;
}

static int ptp_send_udp_data(struct ptp_dev *dev, const unsigned char *data,
	unsigned int data_len, unsigned int cmd)
{
	struct sk_buff *skb = NULL;
	char *pdata = NULL;
	int sec_ret = 0;
	unsigned int skb_size;

	if ((dev == NULL) || (data == NULL) || (dev->net_dev == NULL)) {
		wifi_audio_log_err("NULL Pointer");
		return ERROR;
	}
	if (data_len > UDP_MSG_LEN) {
		wifi_audio_log_err("len is invalid");
		return ERROR;
	}
	skb_size = data_len + sizeof(struct iphdr) + sizeof(struct udphdr) +
		LL_RESERVED_SPACE(dev->net_dev);
	skb = netdev_alloc_skb(dev->net_dev, skb_size);
	if (skb == NULL) {
		wifi_audio_log_err("skb alloc failed, error");
		return ERROR;
	}
	skb_reserve (skb, skb_size);
	skb->dev = dev->net_dev;
	skb->pkt_type = PACKET_OTHERHOST;
	skb->protocol = __constant_htons(ETH_P_IP);
	skb->ip_summed = CHECKSUM_NONE;
	skb->priority = 6; /* set priority VO */
	skb->mark = 0x5a; /* wifi vip mark */

	pdata = skb_push(skb, data_len);
	sec_ret += memcpy_s(pdata, data_len, data, data_len);
	if (sec_ret != 0) {
		wifi_audio_log_err("memcpy_s failed, error");
		dev_kfree_skb_any(skb);
		return ERROR;
	}
	ptp_packet_udp_data(skb, dev, data_len);

	if (cmd == SYNC_REQ)
		getnstimeofday64(&(dev->time_udp.t3));

	if (unlikely(dev->net_dev->netdev_ops == NULL)) {
		wifi_audio_log_err("netdev_ops is NULL, error");
		dev_kfree_skb_any(skb);
		return ERROR;
	}
	dev->net_dev->netdev_ops->ndo_start_xmit(skb, dev->net_dev);
	return SUCCESS;
}

static void ptp_set_time(long long offset_us)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0)
	struct timespec delta;
#else
	struct timespec64 delta;
#endif
	struct timeval tv;
	struct timespec64 new_ts;
	long offset = (long)offset_us;

	if (abs(offset) > PTP_SET_TIME_OFFSET_THREHOLD_USEC) {
		do_gettimeofday(&tv);
		tv.tv_sec -=  (offset / USEC_PER_SEC);
		tv.tv_usec -= (offset % USEC_PER_SEC);
		/* tv_usec overflow calculation */
		if (tv.tv_usec < 0) {
			tv.tv_sec -= 1;
			tv.tv_usec += USEC_PER_SEC;
		} else if (tv.tv_usec >= USEC_PER_SEC) {
			tv.tv_sec += 1;
			tv.tv_usec -= US_IN_SECOND;
		}
		new_ts.tv_sec = tv.tv_sec;
		new_ts.tv_nsec = tv.tv_usec * NSEC_PER_USEC;
		wifi_audio_log_info("set offset %ld sec %lld nsec %ld", offset,
			new_ts.tv_sec, new_ts.tv_nsec);
		if (do_settimeofday64(&new_ts) != 0)
			wifi_audio_log_err("set time of day, error");
	} else {
		offset *= (-1L); /* offset need Reverse calculation */
		delta.tv_sec  = offset / USEC_PER_SEC;
		delta.tv_nsec = (offset % USEC_PER_SEC) * NSEC_PER_USEC;
		/* tv_nsec valid check */
		if (offset < 0 && delta.tv_nsec) {
			delta.tv_sec -= 1;
			delta.tv_nsec += NSEC_PER_SEC;
		}
		wifi_audio_log_info("inject offset %ld sec %ld nsec %ld",
			offset, delta.tv_sec, delta.tv_nsec);
		if (timekeeping_inject_offset(&delta) != 0)
			wifi_audio_log_err("inject offset, error");
	}
	return;
}

static int ptp_time_cmp(const void *a,const void *b)
{
	const long long *r1 = NULL;
	const long long *r2 = NULL;

	if ((a == NULL) || (b == NULL))
		return 0;

	r1 = a;
	r2 = b;
	if (*r1 < *r2)
		return -1;
	if (*r1 > *r2)
		return 1;
	return 0;
}

static long long ptp_average(const long long *array, int array_len)
{
	long sum;
	int i;
	if ((array == NULL) || (array_len <= 0)) {
		wifi_audio_log_err("param is invalid, error");
		return 0;
	}

	sum = 0;
	for(i = 0; i < array_len; i++)
		sum += array[i];

	return sum / array_len;
}

static unsigned long ptp_standard_deviation(const long long *array,
	int array_len)
{
	long avg;
	long sqrt_sum;
	int i;

	if ((array == NULL) || (array_len <= 0)) {
		wifi_audio_log_err("array is NULL, error");
		return 0;
	}

	avg = ptp_average(array, array_len);
	wifi_audio_log_info("avg = %ld", avg);
	sqrt_sum = 0;
	for (i = 0; i < array_len; i++)
		sqrt_sum += (array[i] - avg) * (array[i] - avg);
	return  int_sqrt(sqrt_sum / array_len);
}

static void sync_offset_report(const struct ptp_dev *dev,
	long long offset_us)
{
	struct netlink_event_report_sync_offset sync_offset = {0};
	struct netlink_data netlink_event = {0};
	int ret;

	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return;
	}

	sync_offset.channel = dev->channel;
	sync_offset.offset_us = offset_us;
	wifi_audio_log_info("channel %d status %lld", dev->channel,
		sync_offset.offset_us);

	netlink_event.id = NETLINK_EVENT_REPORT_SYNC_OFFSET;
	netlink_event.msg_len = sizeof(sync_offset);
	ret = memcpy_s(netlink_event.msg_data, NETLINK_MSG_DATA_LEN_MAX,
		&sync_offset, sizeof(sync_offset));
	if (ret != 0) {
		wifi_audio_log_err("memcpy failed");
		return;
	}
	ret = netlink_event_report(&netlink_event);
	if (ret != 0)
		wifi_audio_log_err("netlink_event_report fail");
	return;
}

static void data_sync_status_report(const struct ptp_dev *dev,
	int status)
{
	struct netlink_event_report_data_sync sync_status = {0};
	struct netlink_data netlink_event = {0};
	int ret;

	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return;
	}

	sync_status.channel = dev->channel;
	sync_status.status = status;
	wifi_audio_log_info("channel %d status %d", dev->channel, status);

	netlink_event.id = NETLINK_EVENT_REPORT_DATA_SYNC;
	netlink_event.msg_len = sizeof(sync_status);
	ret = memcpy_s(netlink_event.msg_data, NETLINK_MSG_DATA_LEN_MAX,
		&sync_status, sizeof(sync_status));
	if (ret != 0) {
		wifi_audio_log_err("memcpy failed");
		return;
	}

	ret = netlink_event_report(&netlink_event);
	if (ret != 0)
		wifi_audio_log_err("netlink_event_report fail");
	return;
}

static void ptp_time_calculation_earlier_stage(struct ptp_dev *dev)
{
	long long offset;
	struct list_node *node = NULL;
	struct list_head *p = NULL;
	long long first_set_array[FIRST_SET_ARRAY_LEN] = {0};
	int i;

	if (dev == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	if ((list_queue_len(&(dev->ptp_time_queue)) <= FIRST_SET_ARRAY_LEN) &&
		(dev->cur_ptp_time.first_time_set_complete == 0)) {
		if (dev->cur_ptp_time.counter <= PTP_TIME_FIRST_SET_THRES) {
			offset = dev->cur_ptp_time.offset;
			wifi_audio_log_info("First set system time offset %lld offset_pre %lld",
				offset, dev->cur_ptp_time.offset_pre);
			if (abs(offset) < abs(dev->cur_ptp_time.offset_pre)) {
				ptp_set_time(offset);
				dev->cur_ptp_time.offset_pre = offset;
			} else {
				dev->cur_ptp_time.offset_pre = ((offset +
					dev->cur_ptp_time.offset_pre) /
					DOUBLE_SIZE);
				ptp_set_time(dev->cur_ptp_time.offset_pre);
			}
			if (dev->cur_ptp_time.counter ==
				PTP_TIME_FIRST_SET_THRES)
				sync_offset_report(dev,
					dev->cur_ptp_time.offset_pre);
			return;
		} else if (dev->cur_ptp_time.counter ==
			PTP_TIME_FIRST_SET_NUM_MAX) {
			i = 0;
			list_for_each(p, &(dev->ptp_time_queue.queue)) {
				node = list_entry(p, struct list_node, list);
				first_set_array[i] =
					((struct time_infor *)(node->data))->offset;
				i++;
			}
			sort(first_set_array, FIRST_SET_ARRAY_LEN, sizeof(long long),
				ptp_time_cmp, NULL);
			offset = ptp_average(&first_set_array[1],
				PTP_TIME_FIRST_SET_THRES);
			wifi_audio_log_info("i %d offset %lld", i, offset);
			ptp_set_time(offset);
			sync_offset_report(dev, offset);
		}
	} else {
		dev->cur_ptp_time.first_time_set_complete = 1;
		if ((abs(dev->cur_ptp_time.offset) >= STOP_DATA_SYNC_THRES)) {
			wifi_audio_log_info("stop_counter %d start_counter %d",
				dev->data_sync_stop_counter,
				dev->data_sync_start_counter);
			dev->data_sync_start_counter = 0;
			dev->data_sync_stop_counter++;
			if ((dev->data_sync_stop_counter > STOP_DATA_SYNC_TRIGGER) &&
			(dev->data_sync_status == DATA_SYNC_START)) {
				data_sync_status_report(dev, DATA_SYNC_STOP);
				dev->data_sync_status = DATA_SYNC_STOP;
			}
		} else {
			dev->data_sync_stop_counter = 0;
			dev->data_sync_start_counter++;
			if ((dev->data_sync_start_counter > START_DATA_SYNC_TRIGGER) &&
			(dev->data_sync_status == DATA_SYNC_STOP)) {
				data_sync_status_report(dev, DATA_SYNC_START);
				dev->data_sync_status = DATA_SYNC_START;
			}
		}
	}
	return;
}

static void ptp_time_calculation_later_stage(struct ptp_dev *dev)
{
	long long offset;
	struct list_node *node = NULL;
	struct list_head *p = NULL;
	long long ptp_time_array[PTP_TIME_QUEUE_LEN] = {0};
	int i;
	unsigned long stand;

	if (dev == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	if ((list_queue_len(&(dev->ptp_time_queue)) == PTP_TIME_QUEUE_LEN) &&
		((dev->cur_ptp_time.t4.tv_sec % PTP_TIME_SET_INTERVAL) == 0) &&
			(dev->cur_ptp_time.t4.tv_usec <
			PTP_TIME_SET_USEC_THRES)) {
		i = 0;
		list_for_each(p, &(dev->ptp_time_queue.queue)) {
			node = list_entry(p, struct list_node, list);
			ptp_time_array[i] =
				((struct time_infor *)(node->data))->offset;
			i++;
		}
		sort(ptp_time_array, PTP_TIME_QUEUE_LEN, sizeof(long long),
			ptp_time_cmp, NULL);
		stand = ptp_standard_deviation(&ptp_time_array[PTP_TIME_STANDARD_CAL_START],
			PTP_TIME_STANDARD_CAL_LEN);
		offset = ptp_average(&ptp_time_array[PTP_TIME_STANDARD_CAL_START],
			PTP_TIME_STANDARD_CAL_LEN);
		wifi_audio_log_info("stand %lu offset %lld", stand, offset);
		if ((stand < PTP_TIME_STANDARD_THRES) &&
			(abs(offset) > PTP_TIME_OFFSET_THRES)) {
			ptp_set_time(offset);
			sync_offset_report(dev, offset);
			list_queue_clean(&(dev->ptp_time_queue));
		}
	}
	return;
}

static void ptp_time_calculation(struct ptp_dev *dev)
{
	struct list_node *node = NULL;

	if (dev == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	/* Calculate offset */
	dev->cur_ptp_time.offset = ((((long long)(dev->cur_ptp_time.t2.tv_sec) * US_IN_SECOND) +
		(long long)dev->cur_ptp_time.t2.tv_usec) - (((long long)(dev->cur_ptp_time.t4.tv_sec) *
		US_IN_SECOND) + (long long)dev->cur_ptp_time.t4.tv_usec) +
		(((long long)(dev->cur_ptp_time.t3.tv_sec) * US_IN_SECOND) +
		(long long)dev->cur_ptp_time.t3.tv_usec) - (((long long)(dev->cur_ptp_time.t1.tv_sec) *
		US_IN_SECOND) + (long long)dev->cur_ptp_time.t1.tv_usec)) / 2;
	/* Calculate delay */
	dev->cur_ptp_time.delay = ((((long long)(dev->cur_ptp_time.t2.tv_sec) * US_IN_SECOND) +
		(long long)dev->cur_ptp_time.t2.tv_usec) + (((long long)(dev->cur_ptp_time.t4.tv_sec) *
		US_IN_SECOND) + (long long)dev->cur_ptp_time.t4.tv_usec) -
		(((long long)(dev->cur_ptp_time.t3.tv_sec) * US_IN_SECOND) +
		(long long)dev->cur_ptp_time.t3.tv_usec) - (((long long)(dev->cur_ptp_time.t1.tv_sec) *
		US_IN_SECOND) + (long long)dev->cur_ptp_time.t1.tv_usec)) / 2;

	wifi_audio_log_info("channel %u role %d counter %u offset %lld delay %lld t4 sec %u usec %u",
		dev->channel, dev->role, dev->cur_ptp_time.counter, dev->cur_ptp_time.offset,
		dev->cur_ptp_time.delay, dev->cur_ptp_time.t4.tv_sec, dev->cur_ptp_time.t4.tv_usec);
	ptp_time_calculation_earlier_stage(dev);
	node = alloc_list_node((void *)&(dev->cur_ptp_time),
		sizeof(dev->cur_ptp_time));
	if (unlikely(node == NULL)) {
		wifi_audio_log_err("alloc node failed");
		return;
	}
	list_queue_enqueue(&(dev->ptp_time_queue), node);
	if (list_queue_len(&(dev->ptp_time_queue)) > PTP_TIME_QUEUE_LEN) {
		node = list_queue_dequeue(&(dev->ptp_time_queue));
		if (node != NULL) {
			wifi_audio_log_info("free ptp time queue counter %d",
				((struct time_infor *)(node->data))->counter);
			free_list_node(node);
			node = NULL;
		}
	}
	ptp_time_calculation_later_stage(dev);
	return;
}

static int ptp_receive_delay_response(struct ptp_dev *dev,
	const struct ptp_event_data *event)
{
	int ret;
	unsigned int counter;
	unsigned char msg[MSG_LEN] = {0};

	if ((dev == NULL) || (event == NULL)) {
		wifi_audio_log_err("NULL Pointer, error");
		return ERROR;
	}
	msg[MSG_TYPE_INDEX] = MSG_PTP_SYNC;
	msg[MSG_SUB_TYPE_INDEX] = SYNC_DONE;

	ret = memcpy_s(&counter, sizeof(counter),
		&(event->msg[MSG_PTP_COUNTER_INDEX]), sizeof(counter));
	ret += memcpy_s(&(dev->cur_ptp_time.t4.tv_sec),
		sizeof(dev->cur_ptp_time.t4.tv_sec),
		&(event->msg[MSG_PTP_TV_SEC_INDEX]),
		sizeof(dev->cur_ptp_time.t4.tv_sec));
	ret += memcpy_s(&(dev->cur_ptp_time.t4.tv_usec),
		sizeof(dev->cur_ptp_time.t4.tv_usec),
		&(event->msg[MSG_PTP_TV_USEC_INDEX]),
		sizeof(dev->cur_ptp_time.t4.tv_usec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail");
		return ERROR;
	}
	if (counter != dev->cur_ptp_time.counter) {
		wifi_audio_log_err("counter != dev->cur_ptp_time.counter, error");
		return ERROR;
	}
	ptp_time_calculation(dev);
	ret = memcpy_s(&msg[MSG_PTP_COUNTER_INDEX],
		MSG_LEN - MSG_PTP_COUNTER_INDEX, &(dev->cur_ptp_time.counter),
		sizeof(dev->cur_ptp_time.counter));
	ret += memcpy_s(&msg[MSG_PTP_OFFSET_INDEX],
		MSG_LEN - MSG_PTP_OFFSET_INDEX, &(dev->cur_ptp_time.offset),
		sizeof(dev->cur_ptp_time.offset));
	ret += memcpy_s(&msg[MSG_PTP_DELAY_INDEX],
		MSG_LEN - MSG_PTP_DELAY_INDEX, &(dev->cur_ptp_time.delay),
		sizeof(dev->cur_ptp_time.delay));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail");
		return ERROR;
	}
	ret = ptp_send_data(dev, msg, MSG_LEN);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail \n");

	return ret;
}

static int ptp_receive_delay_request(struct ptp_dev *dev,
	const struct ptp_event_data *event)
{
	int ret;
	unsigned int counter;
	unsigned char msg[MSG_LEN] = {0};

	if ((dev == NULL) || (event == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}
	msg[MSG_TYPE_INDEX] = MSG_PTP_SYNC;
	msg[MSG_SUB_TYPE_INDEX] = DELAY_RESP;
	ret = memcpy_s(&counter, sizeof(counter), &(event->msg[MSG_PTP_COUNTER_INDEX]),
		sizeof(counter));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	if (counter != dev->cur_ptp_time.counter) {
		wifi_audio_log_err("counter != dev->cur_ptp_time.counter, error");
		return ERROR;
	}

	dev->cur_ptp_time.t4.tv_sec = event->stamp.tv_sec;
	dev->cur_ptp_time.t4.tv_usec = event->stamp.tv_nsec / 1000;

	ret = memcpy_s(&msg[MSG_PTP_COUNTER_INDEX],
		MSG_LEN - MSG_PTP_COUNTER_INDEX, &(dev->cur_ptp_time.counter),
		sizeof(dev->cur_ptp_time.counter));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = memcpy_s(&msg[MSG_PTP_TV_SEC_INDEX],
		MSG_LEN - MSG_PTP_TV_SEC_INDEX, &(dev->cur_ptp_time.t4.tv_sec),
		sizeof(dev->cur_ptp_time.t4.tv_sec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = memcpy_s(&msg[MSG_PTP_TV_USEC_INDEX],
		MSG_LEN - MSG_PTP_TV_USEC_INDEX, &(dev->cur_ptp_time.t4.tv_usec),
		sizeof(dev->cur_ptp_time.t4.tv_usec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}

	ret = ptp_send_data(dev, msg, MSG_LEN);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail \n");

	return ret;
}

static int ptp_receive_sync_request(struct ptp_dev *dev,
	const struct ptp_event_data *event)
{
	int ret;
	unsigned char msg[MSG_LEN] = {0};
	struct timeval tv;

	if ((dev == NULL) || (event == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}

	msg[MSG_TYPE_INDEX] = MSG_PTP_SYNC;
	msg[MSG_SUB_TYPE_INDEX] = DELAY_REQ;

	ret = memcpy_s(&(dev->cur_ptp_time.counter),
		sizeof(dev->cur_ptp_time.counter),
		&(event->msg[MSG_PTP_COUNTER_INDEX]),
		sizeof(dev->cur_ptp_time.counter));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}

	ret = memcpy_s(&(dev->cur_ptp_time.t1.tv_sec),
		sizeof(dev->cur_ptp_time.t1.tv_sec),
		&(event->msg[MSG_PTP_TV_SEC_INDEX]),
		sizeof(dev->cur_ptp_time.t1.tv_sec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = memcpy_s(&(dev->cur_ptp_time.t1.tv_usec),
		sizeof(dev->cur_ptp_time.t1.tv_usec),
		&(event->msg[MSG_PTP_TV_USEC_INDEX]),
		sizeof(dev->cur_ptp_time.t1.tv_usec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	dev->cur_ptp_time.t2.tv_sec = event->stamp.tv_sec;
	dev->cur_ptp_time.t2.tv_usec = event->stamp.tv_nsec / 1000;
	ret = memcpy_s(&msg[MSG_PTP_COUNTER_INDEX],
		MSG_LEN - MSG_PTP_COUNTER_INDEX, &(dev->cur_ptp_time.counter),
		sizeof(dev->cur_ptp_time.counter));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	do_gettimeofday(&tv);
	dev->cur_ptp_time.t3.tv_sec = tv.tv_sec;
	dev->cur_ptp_time.t3.tv_usec = tv.tv_usec;
	ret = memcpy_s(&msg[MSG_PTP_TV_SEC_INDEX],
		MSG_LEN - MSG_PTP_TV_SEC_INDEX, &(dev->cur_ptp_time.t3.tv_sec),
		sizeof(dev->cur_ptp_time.t3.tv_sec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = memcpy_s(&msg[MSG_PTP_TV_USEC_INDEX],
		MSG_LEN - MSG_PTP_TV_USEC_INDEX, &(dev->cur_ptp_time.t3.tv_usec),
		sizeof(dev->cur_ptp_time.t3.tv_usec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = ptp_send_data(dev, msg, MSG_LEN);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail \n");

	return ret;
}

static int ptp_receive_udp_request(struct ptp_dev *dev,
	const struct ptp_event_data *event)
{
	int ret;
	unsigned char msg[UDP_MSG_LEN] = {0};
	unsigned int cmd;
	unsigned int cmd_rsp;
	unsigned long long timestamp_rsp = 0;

	if ((dev == NULL) || (event == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}

	/* 12-15 is cmd code */
	cmd = event->msg[12] | (event->msg[13] << 8) | (event->msg[14] << 16) |
		(event->msg[15] << 24);
	/* cmd response need add one */
	cmd_rsp = cmd + 1;

	/* 0-3 is magic number, keep Consistent */
	msg[0] = event->msg[0];
	msg[1] = event->msg[1];
	msg[2] = event->msg[2];
	msg[3] = event->msg[3];

	/* 4-7 is ver, keep Consistent */
	msg[4] = event->msg[4];
	msg[5] = event->msg[5];
	msg[6] = event->msg[6];
	msg[7] = event->msg[7];

	/* 8-11 is Serial number, keep Consistent */
	msg[8] = event->msg[8];
	msg[9] = event->msg[9];
	msg[10] = event->msg[10];
	msg[11] = event->msg[11];

	/* 12-15 is cmd code */
	msg[12] = (cmd_rsp & 0xFF);
	msg[13] = ((cmd_rsp >> 8) & 0xFF);
	msg[14] = ((cmd_rsp >> 16) & 0xFF);
	msg[15] = ((cmd_rsp >> 24) & 0xFF);

	/* 16-19 is timestamp counter */
	msg[16] = 0x01;
	msg[17] = 0x00;
	msg[18] = 0x00;
	msg[19] = 0x00;
	if (cmd == SYNC_REQ)
		timestamp_rsp = event->stamp.tv_sec * NSEC_PER_SEC +
			event->stamp.tv_nsec;
	else if (cmd == DELAY_REQ)
		timestamp_rsp = dev->time_udp.t3.tv_sec * NSEC_PER_SEC +
			dev->time_udp.t3.tv_nsec;
	/* 20-27 is timestam */
	msg[20] = (timestamp_rsp & 0xFF);
	msg[21] = ((timestamp_rsp >> 8) & 0xFF);
	msg[22] = ((timestamp_rsp >> 16) & 0xFF);
	msg[23] = ((timestamp_rsp >> 24) & 0xFF);
	msg[24] = ((timestamp_rsp >> 32) & 0xFF);
	msg[25] = ((timestamp_rsp >> 40) & 0xFF);
	msg[26] = ((timestamp_rsp >> 48) & 0xFF);
	msg[27] = ((timestamp_rsp >> 56) & 0xFF);

	ret = ptp_send_udp_data(dev, msg, UDP_MSG_LEN, cmd);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail \n");

	return ret;
}

static int ptp_send_sync_request(struct ptp_dev *dev)
{
	int ret;
	unsigned char msg[MSG_LEN] = {0};
	struct timeval tv;
	if (dev == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return ERROR;
	}

	msg[MSG_TYPE_INDEX] = MSG_PTP_SYNC;
	msg[MSG_SUB_TYPE_INDEX] = SYNC_REQ;
	dev->cur_ptp_time.counter++;
	ret = memcpy_s(&msg[MSG_PTP_COUNTER_INDEX],
		MSG_LEN - MSG_PTP_COUNTER_INDEX, &(dev->cur_ptp_time.counter),
		sizeof(dev->cur_ptp_time.counter));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	do_gettimeofday(&tv);
	dev->cur_ptp_time.t1.tv_sec = tv.tv_sec;
	dev->cur_ptp_time.t1.tv_usec = tv.tv_usec;
	ret = memcpy_s(&msg[MSG_PTP_TV_SEC_INDEX],
		MSG_LEN - MSG_PTP_TV_SEC_INDEX, &(dev->cur_ptp_time.t1.tv_sec),
		sizeof(dev->cur_ptp_time.t1.tv_sec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = memcpy_s(&msg[MSG_PTP_TV_USEC_INDEX],
		MSG_LEN - MSG_PTP_TV_USEC_INDEX, &(dev->cur_ptp_time.t1.tv_usec),
		sizeof(dev->cur_ptp_time.t1.tv_usec));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	ret = ptp_send_data(dev, msg, MSG_LEN);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail \n");

	return ret;
}

static void ptp_offset_delay_report(const struct netlink_event_report_offset_delay *offset_delay)
{
	struct netlink_data netlink_event = {0};
	int ret;
	if (offset_delay == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}

	netlink_event.id = NETLINK_EVENT_REPORT_OFFSET_DELAY;
	netlink_event.msg_len = sizeof(*offset_delay);
	ret = memcpy_s(netlink_event.msg_data, NETLINK_MSG_DATA_LEN_MAX,
		offset_delay, sizeof(*offset_delay));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail");
		return;
	}

	ret = netlink_event_report(&netlink_event);
	if (ret != 0)
		wifi_audio_log_err("netlink_event_report fail \n");

	return;
}

static int ptp_receive_sync_done(struct ptp_dev *dev,
	const struct ptp_event_data *event)
{
	int ret;
	unsigned int counter;
	struct netlink_event_report_offset_delay offset_delay = {0};
	struct list_head *p = NULL;
	long long offset_array[PTP_TIME_QUEUE_LEN] = {0};
	long long delay_array[PTP_TIME_QUEUE_LEN] = {0};
	int i;
	struct list_node *node = NULL;

	if ((dev == NULL) || (event == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}
	ret = memcpy_s(&counter, sizeof(counter), &(event->msg[MSG_PTP_COUNTER_INDEX]),
		sizeof(counter));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}
	if (counter != dev->cur_ptp_time.counter) {
		wifi_audio_log_err("counter != dev->cur_ptp_time.counter, error");
		return ERROR;
	}
	if (dev->status == PTP_START)
		hrtimer_start(&(dev->start_ptp),
				ktime_set(0, PTP_START_SYNC_AGAIN_TIME_NSEC),
				HRTIMER_MODE_REL);
	hrtimer_cancel(&(dev->time_sync_timeout));
	dev->time_sync_timeout_counter = 0;

	ret = memcpy_s(&(dev->cur_ptp_time.offset),
		sizeof(dev->cur_ptp_time.offset), &(event->msg[MSG_PTP_OFFSET_INDEX]),
		sizeof(dev->cur_ptp_time.offset));
	ret += memcpy_s(&(dev->cur_ptp_time.delay),
		sizeof(dev->cur_ptp_time.delay), &(event->msg[MSG_PTP_DELAY_INDEX]),
		sizeof(dev->cur_ptp_time.delay));
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail \n");
		return ERROR;
	}

	if ((list_queue_is_empty(&(dev->ptp_time_queue)) != 0) &&
		(dev->cur_ptp_time.first_time_set_complete == 0)) {
		offset_delay.channel = dev->channel;
		offset_delay.offset_usec = dev->cur_ptp_time.offset;
		offset_delay.delay_usec = dev->cur_ptp_time.delay;
		wifi_audio_log_info("report offset %lld delay %lld",
			offset_delay.offset_usec, offset_delay.delay_usec);
		ptp_offset_delay_report(&offset_delay);
		dev->cur_ptp_time.first_time_set_complete = 1;
	}

	node = alloc_list_node((void *)&(dev->cur_ptp_time), sizeof(dev->cur_ptp_time));
	if (unlikely(node == NULL)) {
		wifi_audio_log_err("alloc node failed");
		return  ERROR;
	}
	list_queue_enqueue(&(dev->ptp_time_queue), node);

	if (list_queue_len(&(dev->ptp_time_queue)) > PTP_TIME_QUEUE_LEN) {
		node = list_queue_dequeue(&(dev->ptp_time_queue));
		if (node != NULL) {
			wifi_audio_log_info("free ptp time queue counter %d",
				((struct time_infor *)(node->data))->counter);
			free_list_node(node);
			node = NULL;
		}
	}
	if (list_queue_len(&(dev->ptp_time_queue)) == PTP_TIME_QUEUE_LEN) {
		i = 0;
		list_for_each(p, &(dev->ptp_time_queue.queue)) {
			node = list_entry(p, struct list_node, list);
			offset_array[i] =
				((struct time_infor *)(node->data))->offset;
			delay_array[i] =
				((struct time_infor *)(node->data))->delay;
			i++;
		}
		sort(offset_array, PTP_TIME_QUEUE_LEN, sizeof(long long), ptp_time_cmp,
			NULL);
		sort(delay_array, PTP_TIME_QUEUE_LEN, sizeof(long long), ptp_time_cmp,
			NULL);
		offset_delay.channel = dev->channel;
		offset_delay.offset_usec = ptp_average(&offset_array[PTP_TIME_STANDARD_CAL_START],
			PTP_TIME_STANDARD_CAL_LEN);
		offset_delay.delay_usec = ptp_average(&delay_array[PTP_TIME_STANDARD_CAL_START],
			PTP_TIME_STANDARD_CAL_LEN);
		ptp_offset_delay_report(&offset_delay);
		list_queue_clean(&(dev->ptp_time_queue));
	}
	return ret;
}

static int ptp_receive_restart_time_sync(struct ptp_dev *dev)
{
	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}

	wifi_audio_log_info("channel %d status %d role %d counter %d",
		dev->channel, dev->status, dev->role, dev->cur_ptp_time.counter);

	(void)memset_s(&(dev->cur_ptp_time), sizeof(dev->cur_ptp_time), 0,
		sizeof(dev->cur_ptp_time));
	return SUCCESS;
}

static void ptp_timesync_set_status(struct ptp_dev *dev,
	int status)
{
	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return;
	}
	wifi_audio_log_info("channel %d status %d", dev->channel, status);
	dev->status = status;
	if ((status == PTP_STOP) || (status == PTP_ABNORMAL)) {
		dev->channel = CHANNEL_MAX;
		dev->ip_dest = 0;
		dev->ip_source = 0;
		(void)memset_s(dev->mac_dest, ETH_ALEN, 0, ETH_ALEN);
		(void)memset_s(dev->mac_source, ETH_ALEN, 0, ETH_ALEN);
		if (dev->role == PTP_MASTER) {
			wifi_audio_log_info("role is PTP_MASTER, clear");
			hrtimer_cancel(&(dev->start_ptp));
			hrtimer_cancel(&(dev->time_sync_timeout));
			dev->time_sync_timeout_counter = 0;
		}
		list_queue_clean(&(dev->dev_event_queue));
		list_queue_clean(&(dev->ptp_time_queue));
		(void)memset_s(&(dev->cur_ptp_time), sizeof(dev->cur_ptp_time),
			0, sizeof(dev->cur_ptp_time));
		dev->role = PTP_ROLE_INVALID;
		hw_unregister_wifi_audio();
		if (dev->net_dev != NULL) {
			wifi_audio_log_info("dev_put the interface");
			dev_put(dev->net_dev);
			dev->net_dev = NULL;
		}
		dev->data_sync_stop_counter = 0;
		dev->data_sync_start_counter = 0;
		dev->data_sync_status = DATA_SYNC_START;
	}
	return;
}

static void time_sync_status_report(const struct ptp_dev *dev,
	int status, int reason)
{
	struct netlink_event_report_time_sync sync_status = {0};
	struct netlink_data netlink_event = {0};
	int ret;

	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return;
	}

	sync_status.channel = dev->channel;
	sync_status.status = status;
	sync_status.reason = reason;
	wifi_audio_log_info("channel %d status %d reason %d", dev->channel,
		status, reason);

	netlink_event.id = NETLINK_EVENT_REPORT_TIME_SYNC;
	netlink_event.msg_len = sizeof(sync_status);
	ret = memcpy_s(netlink_event.msg_data, NETLINK_MSG_DATA_LEN_MAX,
		&sync_status, sizeof(sync_status));
	if (ret != 0) {
		wifi_audio_log_err("memcpy failed");
		return;
	}

	ret = netlink_event_report(&netlink_event);
	if (ret != 0)
		wifi_audio_log_err("netlink_event_report fail");
	return;
}

static int ptp_receive_stop_time_sync(struct ptp_dev *dev)
{
	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}

	wifi_audio_log_info("channel %d status %d role %d counter %d",
		dev->channel, dev->status, dev->role, dev->cur_ptp_time.counter);

	if ((dev->status != PTP_START)) {
		wifi_audio_log_info("this channel is not PTP_START");
		return SUCCESS;
	}
	time_sync_status_report(dev, PTP_STOP, TIME_SYNC_STOP_BY_THE_OTHER_END);
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	if ((ptp_infor != NULL) && (dev->role == PTP_MASTER) &&
		(ptp_infor->write_time_reg_timer_started == 1)) {
		wifi_audio_log_info("write time reg timer cancel");
		hrtimer_cancel(&(ptp_infor->write_time_reg_timer));
		ptp_infor->write_time_reg_timer_started = 0;
	}
#endif
	ptp_timesync_set_status(dev, PTP_STOP);
	return  SUCCESS;
}

static void dev_event_dispatch(struct ptp_dev *dev,
	const struct ptp_event_data *event)
{
	int ret;

	if ((dev == NULL) || (event == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return;
	}
	switch (event->id) {
	case PTP_SEND_SYNC_REQUEST: {
		ret = ptp_send_sync_request(dev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp send sync request, error");
		break;
	}
	case PTP_RECEIVE_SYNC_REQUEST: {
		ret = ptp_receive_sync_request(dev, event);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive sync request, error");
		break;
	}
	case PTP_RECEIVE_DELAY_REQUEST: {
		ret = ptp_receive_delay_request(dev, event);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive delay request, error");
		break;
	}
	case PTP_RECEIVE_DELAY_RESPONSE: {
		ret = ptp_receive_delay_response(dev, event);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive delay response, error");
		break;
	}
	case PTP_RECEIVE_SYNC_DONE: {
		ret = ptp_receive_sync_done(dev, event);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive sync done, error");
		break;
	}
	case PTP_RECEIVE_RESTART_TIME_SYNC: {
		ret = ptp_receive_restart_time_sync(dev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive restart timesync, error");
		break;
	}
	case PTP_RECEIVE_STOP_TIME_SYNC: {
		ret = ptp_receive_stop_time_sync(dev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive stop timesync, error");
		break;
	}
	case PTP_RECEIVE_DELAY_REQUEST_UDP:
	case PTP_RECEIVE_SYNC_REQUEST_UDP: {
		ret = ptp_receive_udp_request(dev, event);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp receive udp request, error");
		break;
	}
	default:
		wifi_audio_log_err("event id is invalid, error");
		break;
	}
	return;
}

static void dev_event_handle(unsigned long data)
{
	struct ptp_dev *dev = NULL;
	struct list_node *node = NULL;
	struct ptp_event_data event = {0};
	int sec_ret;

	spin_lock_bh(&ptp_infor_lock);
	dev = (struct ptp_dev *)data;
	if (dev == NULL) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}

	while (list_queue_is_empty(&(dev->dev_event_queue)) == 0) {
		node = list_queue_dequeue(&(dev->dev_event_queue));
		if (node == NULL) {
			wifi_audio_log_err("dequeue a null node");
			break;
		}
		sec_ret = memcpy_s(&event, sizeof(event), node->data, node->len);
		if (sec_ret != 0) {
			wifi_audio_log_err("memcpy node->data fail");
			free_list_node(node);
			node = NULL;
			continue;
		}
		free_list_node(node);
		node = NULL;
		dev_event_dispatch(dev, &event);
	}
	spin_unlock_bh(&ptp_infor_lock);
	return;
}

static void start_ptp_work_handle(struct work_struct *work)
{
	struct ptp_dev *dev = NULL;
	struct list_node *node = NULL;
	struct ptp_event_data event = {0};

	if (work == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	spin_lock_bh(&ptp_infor_lock);
	dev = container_of(work, struct ptp_dev, start_ptp_work);
	event.id = PTP_SEND_SYNC_REQUEST;
	node = alloc_list_node((void *)&event, sizeof(event));
	if (unlikely(node == NULL)) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("alloc node failed");
		return;
	}
	list_queue_enqueue(&(dev->dev_event_queue), node);
	tasklet_hi_schedule(&(dev->dev_work));
	if (dev->status == PTP_START)
		hrtimer_start(&(dev->time_sync_timeout),
			ktime_set(PTP_TIME_SYNC_TIMEOUT_TIME_SEC,
			PTP_TIME_SYNC_TIMEOUT_TIME_NSEC),
			HRTIMER_MODE_REL);
	dev->time_sync_timeout_counter = 0;
	spin_unlock_bh(&ptp_infor_lock);
	return;
}

static void time_sync_timeout_handle(struct work_struct *work)
{
	struct ptp_event_data event = {0};
	struct list_node *node = NULL;
	struct ptp_dev *dev = NULL;
	unsigned char msg[MSG_LEN] = {0};
	int ret;

	if (work == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return;
	}
	spin_lock_bh(&ptp_infor_lock);
	dev = container_of(work, struct ptp_dev, time_sync_timeout_work);
	dev->time_sync_timeout_counter++;

	if (dev->time_sync_timeout_counter >= ptp_infor->time_sync_timeout_max_counter) {
		wifi_audio_log_err("ptp time sync tatus PTP_ABNORMAL, errer");
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
		if ((ptp_infor != NULL) && (dev->role == PTP_MASTER) &&
			(ptp_infor->write_time_reg_timer_started == 1)) {
			wifi_audio_log_info("write time reg timer cancel");
			hrtimer_cancel(&(ptp_infor->write_time_reg_timer));
			ptp_infor->write_time_reg_timer_started = 0;
		}
#endif
		ptp_send_stop_timesync(dev);
		time_sync_status_report(dev, PTP_ABNORMAL, TIME_SYNC_ABNORMAL_TIMEOUT);
		ptp_timesync_set_status(dev, PTP_ABNORMAL);
		spin_unlock_bh(&ptp_infor_lock);
		return;
	}

	wifi_audio_log_info("time sync timeout, restart the sync");

	msg[MSG_TYPE_INDEX] = MSG_PTP_SYNC;
	msg[MSG_SUB_TYPE_INDEX] = RESTART_TIME_SYNC;

	ret = ptp_send_data(dev, msg, MSG_LEN);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail \n");

	list_queue_clean(&(dev->dev_event_queue));
	list_queue_clean(&(dev->ptp_time_queue));
	(void)memset_s(&(dev->cur_ptp_time), sizeof(dev->cur_ptp_time), 0,
		sizeof(dev->cur_ptp_time));

	event.id = PTP_SEND_SYNC_REQUEST;
	node = alloc_list_node((void *)&event, sizeof(event));
	if (unlikely(node == NULL)) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("alloc node failed");
		return;
	}
	list_queue_enqueue(&(dev->dev_event_queue), node);
	tasklet_hi_schedule(&(dev->dev_work));
	if (dev->status == PTP_START)
		hrtimer_start(&(dev->time_sync_timeout),
			ktime_set(PTP_TIME_SYNC_TIMEOUT_TIME_SEC,
			PTP_TIME_SYNC_TIMEOUT_TIME_NSEC),
			HRTIMER_MODE_REL);
	spin_unlock_bh(&ptp_infor_lock);
	return;
}

static enum hrtimer_restart start_ptp_timer(struct hrtimer *timer)
{
	struct ptp_dev *dev = NULL;

	if (timer == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return HRTIMER_NORESTART;
	}
	dev = container_of(timer, struct ptp_dev, start_ptp);
	schedule_work(&(dev->start_ptp_work));
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart time_sync_timeout_timer(struct hrtimer *timer)
{
	struct ptp_dev *dev = NULL;
	if (timer == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return HRTIMER_NORESTART;
	}
	dev = container_of(timer, struct ptp_dev, time_sync_timeout);
	wifi_audio_log_info("time sync timeout ch %u status %d", dev->channel, dev->status);
	if (dev->status == PTP_START)
		schedule_work(&(dev->time_sync_timeout_work));
	return HRTIMER_NORESTART;
}

#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
static enum hrtimer_restart write_time_reg_handle(struct hrtimer *timer)
{
	struct wifi_audio_ptp *ptp = NULL;
	struct timeval tv;
	unsigned long long time_us;
	unsigned long long time_us_l;
	unsigned long long time_us_h;

	if (timer == NULL) {
		wifi_audio_log_err("NULL Pointer, error");
		return HRTIMER_NORESTART;
	}
	ptp = container_of(timer, struct wifi_audio_ptp, write_time_reg_timer);
	if (((ptp->dev[CHANNEL_LEFT].status == PTP_START) &&
		(ptp->dev[CHANNEL_RIGHT].status == PTP_START)) ||
		(ptp->dev[CHANNEL_ONE_SPEAKER].status == PTP_START)) {
		(void)memset_s(&tv, sizeof(tv), 0, sizeof(tv));
		do_gettimeofday(&tv);
		time_us = tv.tv_sec * US_IN_SECOND + tv.tv_usec;
		time_us_l = time_us & 0x00000000FFFFFFFF;  /* hanle low 32bit */
		time_us_h = (time_us >> 32) & 0x00000000FFFFFFFF; /* hanle high 32bit */
		__raw_writel(time_us_l, ptp->time_regs);
		__raw_writel(time_us_h, (void __iomem *)((unsigned char *)(ptp->time_regs) + 4));
		hrtimer_start(&(ptp->write_time_reg_timer),
			ktime_set(0, WRITE_TIME_REG_TIMEOUT_TIME_NSEC),
			HRTIMER_MODE_REL);
		ptp->write_time_reg_timer_started = 1;
	} else {
		wifi_audio_log_err("not PTP_START, stop write reg timer");
		ptp->write_time_reg_timer_started = 0;
	}
	return HRTIMER_NORESTART;
}
#endif

static int ptp_start_timesync(const struct netlink_data *msg_rev)
{
	int ret;
	struct netlink_msg_ptp_start_info *ptp_start = NULL;
	struct ptp_event_data event = {0};
	struct list_node *node = NULL;
	unsigned int channel;
	struct hw_wifi_audio_ops ops;

	if ((msg_rev == NULL) || (ptp_infor == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return ERROR;
	}

	ptp_start = (struct netlink_msg_ptp_start_info *)msg_rev->msg_data;
	if (ptp_start->channel >= CHANNEL_MAX) {
		wifi_audio_log_err("invalid channel, error");
		return ERROR;
	}
	channel = ptp_start->channel;
	ptp_infor->dev[channel].channel = channel;
	if (ptp_infor->dev[channel].status == PTP_START) {
		wifi_audio_log_info("this channel %d has started", channel);
		return SUCCESS;
	}
	ptp_infor->dev[channel].role = ptp_start->role;
	ptp_infor->dev[channel].ip_dest = ptp_start->ip_dest;
	ptp_infor->dev[channel].ip_source = ptp_start->ip_source;
	ptp_infor->dev[channel].cmd_port = ptp_start->cmd_port;
	ptp_infor->dev[channel].data_port = ptp_start->data_port;
	ret = memcpy_s(ptp_infor->dev[channel].mac_dest, ETH_ALEN,
		ptp_start->mac_dest, ETH_ALEN);
	ret += memcpy_s(ptp_infor->dev[channel].mac_source, ETH_ALEN,
		ptp_start->mac_source, ETH_ALEN);
	ret += memcpy_s(ptp_infor->dev[channel].interface_name, IFNAMSIZ,
		ptp_start->interface_name, IFNAMSIZ);
	if (ret != 0) {
		wifi_audio_log_err("memcpy fail");
		ptp_infor->dev[channel].status = PTP_NOT_ENABLED;
		return ERROR;
	}
	ptp_infor->dev[channel].time_sync_ver = ptp_start->time_sync_ver;
	ptp_infor->dev[channel].protocol = ptp_start->time_sync_ver ==
		TIME_SYNC_VER_0 ? IPPROTO_TCP : IPPROTO_UDP;
	ptp_infor->dev[channel].net_dev = dev_get_by_name(&init_net,
		ptp_infor->dev[channel].interface_name);
	if (ptp_infor->dev[channel].net_dev == NULL) {
		wifi_audio_log_err("dev_get_by_name fail");
		ptp_infor->dev[channel].status = PTP_NOT_ENABLED;
		return ERROR;
	}
	wifi_audio_log_info("channel %d role %d interface_name %s cmd_port %u"
		"data_port %u time_sync_ver %d", channel, ptp_start->role,
		ptp_infor->dev[channel].interface_name,
		ptp_infor->dev[channel].cmd_port,
		ptp_infor->dev[channel].data_port,
		ptp_infor->dev[channel].time_sync_ver);

	ptp_infor->dev[channel].status = PTP_START;
	if ((ptp_infor->dev[channel].role == PTP_MASTER) &&
		(ptp_infor->dev[channel].time_sync_ver == TIME_SYNC_VER_0)) {
		wifi_audio_log_info("role is PTP_MASTER, start time sync");
		event.id = PTP_SEND_SYNC_REQUEST;
		node = alloc_list_node((void *)&event, sizeof(event));
		if (unlikely(node == NULL)) {
			wifi_audio_log_err("alloc node failed");
			ptp_infor->dev[channel].status = PTP_NOT_ENABLED;
			return  ERROR;
		}
		list_queue_enqueue(&(ptp_infor->dev[channel].dev_event_queue),
			node);
		tasklet_hi_schedule(&(ptp_infor->dev[channel].dev_work));
		hrtimer_start(&(ptp_infor->dev[channel].time_sync_timeout),
			ktime_set(PTP_TIME_SYNC_TIMEOUT_TIME_SEC,
			PTP_TIME_SYNC_TIMEOUT_TIME_NSEC),
			HRTIMER_MODE_REL);
		ptp_infor->dev[channel].time_sync_timeout_counter = 0;
	}
	ptp_infor->dev[channel].data_sync_stop_counter = 0;
	ptp_infor->dev[channel].data_sync_start_counter = 0;
	ptp_infor->dev[channel].data_sync_status = DATA_SYNC_START;
	ptp_infor->dev[channel].id_udp = 0;
	ptp_infor->dev[channel].cur_ptp_time.first_time_set_complete = 0;
	ptp_infor->dev[channel].cur_ptp_time.offset_pre = SECOND_IN_HALF_HOUR *
		SECOND_IN_MINUTE * US_IN_SECOND;
	(void)memset_s(&ops, sizeof(ops), 0, sizeof(ops));
	ops.wifi_audio_skb_receive_handle = skb_rx_handle;
	ops.wifi_audio_skb_send_handle = skb_tx_handle;
	ret = hw_register_wifi_audio(&ops);
	if (ret != 0)
		wifi_audio_log_err("register wifi audio handle fail");

#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	if (ptp_infor->write_time_reg_timer_started == 0) {
		wifi_audio_log_info("write time reg timer start");
		hrtimer_start(&(ptp_infor->write_time_reg_timer),
			ktime_set(0, WRITE_TIME_REG_TIMEOUT_TIME_NSEC),
			HRTIMER_MODE_REL);
		ptp_infor->write_time_reg_timer_started = 1;
	}
#endif
	return  SUCCESS;
}

static int ptp_set_config_infor(const struct netlink_data *msg_rev)
{
	struct netlink_msg_config_info *config_infor = NULL;

	if ((msg_rev == NULL) || (ptp_infor == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return  ERROR;
	}

	config_infor = (struct netlink_msg_config_info *)(msg_rev->msg_data);
	wifi_audio_log_info("set config num %u mode %u", config_infor->num,
		config_infor->mode);

	ptp_infor->num = config_infor->num;
	ptp_infor->mode = config_infor->mode;
	ptp_infor->time_sync_timeout_max_counter =
		(ptp_infor->mode == WIFI_MODE_DBAC) ?
		TIME_SYNC_TIMEOUT_COUNTER_DBAC_MAX :
		TIME_SYNC_TIMEOUT_COUNTER_MAX;

	return  SUCCESS;
}

static void ptp_send_stop_timesync(struct ptp_dev *dev)
{
	unsigned char msg[MSG_LEN] = {0};
	int ret;

	if (dev == NULL) {
		wifi_audio_log_err("on NULL Pointer, error");
		return;
	}
	msg[MSG_TYPE_INDEX] = MSG_PTP_SYNC;
	msg[MSG_SUB_TYPE_INDEX] = STOP_TIME_SYNC;

	ret = ptp_send_data(dev, msg, MSG_LEN);
	if (ret != SUCCESS)
		wifi_audio_log_err("ptp_send_data fail");

	return;
}

static int ptp_stop_timesync(const struct netlink_data *msg_rev)
{
	struct netlink_msg_ptp_stop_info *ptp_stop = NULL;
	unsigned int channel;

	if ((msg_rev == NULL) || (ptp_infor == NULL)) {
		wifi_audio_log_err("on NULL Pointer, error");
		return  ERROR;
	}

	ptp_stop = (struct netlink_msg_ptp_stop_info *)(msg_rev->msg_data);
	channel = ptp_stop->channel;
	if (channel >= CHANNEL_MAX) {
		wifi_audio_log_err("invalid channel, error");
		return  ERROR;
	}
	wifi_audio_log_info("channel %d status %d", ptp_stop->channel,
		ptp_infor->dev[channel].status);
	if (ptp_infor->dev[channel].status != PTP_START) {
		wifi_audio_log_info("this channel %d is not started", channel);
		return SUCCESS;
	}
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	if ((ptp_infor->dev[channel].role == PTP_MASTER) &&
		(ptp_infor->write_time_reg_timer_started == 1)) {
		wifi_audio_log_info("write time reg timer cancel");
		hrtimer_cancel(&(ptp_infor->write_time_reg_timer));
		ptp_infor->write_time_reg_timer_started = 0;
	}
#endif
	ptp_send_stop_timesync(&(ptp_infor->dev[channel]));
	ptp_timesync_set_status(&(ptp_infor->dev[channel]), PTP_STOP);
	return  SUCCESS;
}

int netlink_msg_ptp_handle(const struct netlink_data *msg_rev)
{
	int ret;
	spin_lock_bh(&ptp_infor_lock);
	if ((msg_rev == NULL) || (ptp_infor == NULL)) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("on NULL Pointer, error");
		return  ERROR;
	}

	wifi_audio_log_info("id %d", msg_rev->id);
	switch (msg_rev->id) {
	case NETLINK_MSG_SET_CONFIG_INFOR: {
		ret = ptp_set_config_infor(msg_rev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp set config infor error");
		break;
	}
	case NETLINK_MSG_START_TIMESYNC: {
		ret = ptp_start_timesync(msg_rev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp start timesync error");
		break;
	}
	case NETLINK_MSG_STOP_TIMESYNC: {
		ret = ptp_stop_timesync(msg_rev);
		if (ret != SUCCESS)
			wifi_audio_log_err("ptp stop time sync error");
		break;
	}
	default:
		wifi_audio_log_err("not find msg id error");
		ret = -1;
		break;
	}
	spin_unlock_bh(&ptp_infor_lock);
	return ret;
}

int wifi_audio_time_sync_init(void)
{
	struct list_node *node = NULL;
	struct waudio_skb skb_data = {0};
	struct wifi_audio_ptp *ptp = NULL;
	int i;

	wifi_audio_log_info("wifi audio time sync init");

	spin_lock_bh(&ptp_infor_lock);
	if (ptp_infor != NULL) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_info("wifi audio time sync has inited");
		return SUCCESS;
	}

	ptp = (struct wifi_audio_ptp *)kmalloc(sizeof(*ptp), GFP_ATOMIC);
	if (ptp == NULL) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_err("kmalloc failed");
		return ERROR;
	}
	(void)memset_s(ptp, sizeof(*ptp), 0, sizeof(*ptp));
	ptp->num = 0;
	for (i = 0; i < CHANNEL_MAX; i++) {
		ptp->dev[i].status = PTP_NOT_ENABLED;
		tasklet_init(&(ptp->dev[i].dev_work), dev_event_handle,
			(unsigned long)(&(ptp->dev[i])));

		list_queue_init(&(ptp->dev[i].dev_event_queue));
		hrtimer_init(&(ptp->dev[i].start_ptp), CLOCK_MONOTONIC,
			HRTIMER_MODE_REL);
		ptp->dev[i].start_ptp.function = start_ptp_timer;
		INIT_WORK(&(ptp->dev[i].start_ptp_work),
			start_ptp_work_handle);

		hrtimer_init(&(ptp->dev[i].time_sync_timeout), CLOCK_MONOTONIC,
			HRTIMER_MODE_REL);
		ptp->dev[i].time_sync_timeout.function = time_sync_timeout_timer;
		INIT_WORK(&(ptp->dev[i].time_sync_timeout_work),
			time_sync_timeout_handle);
		ptp->dev[i].time_sync_timeout_counter = 0;
		list_queue_init(&(ptp->dev[i].ptp_time_queue));
	}
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	hrtimer_init(&(ptp->write_time_reg_timer), CLOCK_MONOTONIC,
		HRTIMER_MODE_REL);
	ptp->write_time_reg_timer.function = write_time_reg_handle;
	ptp->write_time_reg_timer_started = 0;
#endif

	tasklet_init(&(ptp->event_handle_work), event_handle_work_handle,
		(unsigned long)ptp);

	list_queue_init(&(ptp->waudio_skb_buffer));
	list_queue_init(&(ptp->waudio_skb_handle));

	for (i = 0; i < WAUDIO_SKB_BUFFER_LEN; i++) {
		node = alloc_list_node((void *)&skb_data, sizeof(skb_data));
		if (unlikely(node == NULL)) {
			wifi_audio_log_err("alloc node failed");
			continue;
		}
		list_queue_enqueue(&(ptp->waudio_skb_buffer), node);
	}
	ptp_infor = ptp;
	spin_unlock_bh(&ptp_infor_lock);
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	ptp_infor->time_regs = ioremap(WRITE_TIME_REG, WRITE_TIME_REG_LEN);
	wifi_audio_log_info("write time reg add %p", ptp_infor->time_regs);
#endif
	return SUCCESS;
}

void wifi_audio_time_sync_uninit(void)
{
	struct wifi_audio_ptp *ptp = NULL;
	int i;
	spin_lock_bh(&ptp_infor_lock);
	if (ptp_infor == NULL) {
		spin_unlock_bh(&ptp_infor_lock);
		wifi_audio_log_info("wifi audio time sync not started");
		return;
	}
	wifi_audio_log_info("wifi audio time sync uninit");

	ptp = ptp_infor;
	list_queue_clean(&(ptp->waudio_skb_buffer));
	list_queue_clean(&(ptp->waudio_skb_handle));
	for (i =0; i < CHANNEL_MAX; i++) {
		if (ptp->dev[i].status == PTP_NOT_ENABLED)
			continue;
		list_queue_clean(&(ptp->dev[i].dev_event_queue));
		list_queue_clean(&(ptp->dev[i].ptp_time_queue));
		cancel_work_sync(&(ptp->dev[i].start_ptp_work));
	}
	ptp_infor = NULL;
	spin_unlock_bh(&ptp_infor_lock);
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	iounmap(ptp->time_regs);
#endif
	kfree(ptp);
	ptp = NULL;
	return;
}

