

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/etherdevice.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include "hip2p_dc_interface.h"
#include "hip2p_dc_util.h"

static int dhcp_packet(const struct iphdr *iph)
{
	struct udphdr *udph = (void *)iph + (iph->ihl * FOUR_BYTES_ALIGNMENT);
	if ((udph != NULL) && (iph->protocol == IPPROTO_UDP) &&
		((ntohs(udph->dest) == DHCP_SERVER_PORT) ||
		(ntohs(udph->dest) == DHCP_CLIENT_PORT))) {
		dc_mlogd("dhcp packet");
		return HIP2P_TRUE;
	}
	return HIP2P_FALSE;
}

/* Game packet: TCP/UDP short packet(from real packet's analysis) */
static int packet_valid(const struct iphdr *iph, const struct sk_buff *skb)
{
	if ((skb->len < HIP2P_VALIDE_GAME_PACKET_LEN) &&
		((iph->protocol == IPPROTO_TCP) ||
		(iph->protocol == IPPROTO_UDP)))
		return HIP2P_SUCC;
	return HIP2P_FAIL;
}

static int game_packet(const struct sk_buff *skb)
{
	struct iphdr *iph = NULL;
	__be16 protocol;

	/* Unicast */
	if (!is_unicast_ether_addr(skb->data))
		return HIP2P_FAIL;

	/* data: dstMac[6] | srcMac[6] | protoType[2] */
	protocol = *(__be16 *)(&(skb->data[ETH_ALEN * MAC_ADDR_NUM]));
	if (protocol != htons(ETH_P_IP))
		return HIP2P_FAIL;
	iph = (struct iphdr *)(skb->data + ETH_ALEN * MAC_ADDR_NUM +
		LENGTH_OF_TYPE);
	if (dhcp_packet(iph))
		return HIP2P_FAIL;
	if (packet_valid(iph, skb))
		return HIP2P_SUCC;

	return HIP2P_FAIL;
}

static int send_check(const struct sk_buff *skb)
{
	struct dc_mngr *dcm = get_dcm();
	if (likely(atomic_read(&dcm->dc_enable) == DC_DISABLE))
		return HIP2P_FAIL;
	if (game_packet(skb) != HIP2P_SUCC)
		return HIP2P_FAIL;

	return HIP2P_SUCC;
}

static struct sk_buff *insert_dc_header(struct sk_buff *skb,
	const __be16 dc_proto, const u16 sequence)
{
	struct dc_ethhdr *dc_eth = NULL;

	if (unlikely(skb_cow_head(skb, DC_HLEN) < MINIMUM_SPACE)) {
		dc_mloge("no room for dc header");
		return NULL;
	}

	dc_eth = (struct dc_ethhdr *)skb_push(skb, DC_HLEN);
	if (dc_eth == NULL) {
		dc_mloge("skb_push failed, dc_eth is NULL");
		return NULL;
	}

	/* data: dstMac[6] | srcMac[6] | protoType[2] */
	memmove(skb->data, skb->data + DC_HLEN, ETH_ALEN * MAC_ADDR_NUM);
	skb->mac_header -= DC_HLEN;
	dc_eth->dc_proto = dc_proto;
	dc_eth->sequence = htons(sequence);
	return skb;
}

static void dc_header_rollback(struct sk_buff *skb)
{
	/* data: dstMac[6] | srcMac[6] | protoType[2] */
	memmove(skb->data + DC_HLEN, skb->data, ETH_ALEN * MAC_ADDR_NUM);
	skb_pull(skb, DC_HLEN);
}

static void delete_dc_header(struct sk_buff *skb, u16 *sequence)
{
	struct dc_hdr *dchdr = NULL;
	dchdr = (struct dc_hdr *)skb->data;
	skb_pull_rcsum(skb, DC_HLEN);
	skb->protocol = dchdr->encap_proto;
	*sequence = ntohs(dchdr->sequence);

	/* data: dstMac[6] | srcMac[6] | protoType[2] */
	memmove(skb->data - ETH_HLEN, skb->data - DC_ETH_HLEN,
		ETH_ALEN * MAC_ADDR_NUM);
	skb->mac_header += DC_HLEN;

	skb_reset_network_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_mac_len(skb);
}

static int encap_packet(struct sk_buff *skb, const struct net_device *dev,
	struct dc_ecap_info *encap)
{
	struct dc_mngr *dcm = get_dcm();
	struct dc_instance *dc = &dcm->dc;

	spin_lock_bh(&dcm->dc_lock);
	if (atomic_read(&dcm->dc_enable) == DC_DISABLE) {
		spin_unlock_bh(&dcm->dc_lock);
		return HIP2P_FAIL;
	}
	if (insert_dc_header(skb, htons(DC_PROTO), dc->sequence) == NULL) {
		spin_unlock_bh(&dcm->dc_lock);
		dc_mloge("add header failed\n");
		return HIP2P_FAIL;
	}
	encap->dev = get_pair_dev(dc, dev);
	if (encap->dev == NULL) {
		spin_unlock_bh(&dcm->dc_lock);
		dc_header_rollback(skb);
		return HIP2P_FAIL;
	}
	dc->sequence++;
	dev_hold(encap->dev);
	spin_unlock_bh(&dcm->dc_lock);

	return HIP2P_SUCC;
}

static inline void change_src_mac(struct sk_buff *skb,
	const struct net_device *dev)
{
	memcpy(skb->data + ETH_ALEN, dev->dev_addr, ETH_ALEN);
}

static void send_copy(struct sk_buff *skb, const struct net_device *dev)
{
	int rc;
	struct sk_buff *skb2 = NULL;
	struct dc_ecap_info encap = {0};

	if (encap_packet(skb, dev, &encap) == HIP2P_FAIL)
		return;

	skb2 = skb_copy(skb, GFP_ATOMIC);
	if (skb2 == NULL) {
		dc_header_rollback(skb);
		dev_put(encap.dev);
		return;
	}

	skb2->dev = encap.dev;
	change_src_mac(skb2, skb2->dev);
	rc = skb2->dev->netdev_ops->ndo_start_xmit(skb2, skb2->dev);
	if (rc != NETDEV_TX_OK) {
		dc_mloge("tx fail\n");
		dc_header_rollback(skb);
		dev_put(encap.dev);
		kfree_skb(skb2);
		skb2 = NULL;
		return;
	}
	dev_put(encap.dev);

	return;
}

static void hip2p_send_dc_copy(struct sk_buff *skb, struct net_device *dev)
{
	if ((skb == NULL) || (dev == NULL)) {
		dc_mloge("skb/dev is NULL");
		return;
	}
	if (send_check(skb) == HIP2P_SUCC)
		send_copy(skb, dev);
}

static int hip2p_check_dup_packet(struct sk_buff *skb)
{
	struct dc_instance *dc = NULL;
	unsigned short sequence = 0;
	struct dc_mngr *dcm = get_dcm();

	if (skb == NULL) {
		dc_mloge("skb is NULL");
		return NET_RX_DROP;
	}

	spin_lock_bh(&dcm->dc_lock);
	if (skb->protocol != cpu_to_be16(DC_PROTO)) {
		if (atomic_read(&dcm->dc_enable) == DC_ENABLE) {
			dc = &dcm->dc;
			if ((skb->dev == dc->dev[TYPE_SLAVE]) &&
				(skb->protocol == cpu_to_be16(ETH_P_ARP))) {
				spin_unlock_bh(&dcm->dc_lock);
				kfree_skb(skb);
				skb = NULL;
				return NET_RX_DROP;
			}
		}
		spin_unlock_bh(&dcm->dc_lock);
		return NET_RX_SUCCESS;
	}
	if (skb->len < DC_HLEN) {
		spin_unlock_bh(&dcm->dc_lock);
		kfree_skb(skb);
		skb = NULL;
		return NET_RX_DROP;
	}
	delete_dc_header(skb, &sequence);
	if (atomic_read(&dcm->dc_enable) == DC_DISABLE) {
		spin_unlock_bh(&dcm->dc_lock);
		dc_mloge("dc disable\n");
		return NET_RX_SUCCESS;
	}
	dc = &dcm->dc;
	if (check_packet_status(dc, sequence) == HIP2P_SUCC) {
		spin_unlock_bh(&dcm->dc_lock);
		kfree_skb(skb);
		skb = NULL;
		dc->seq.later_recv_seq = sequence;
		return NET_RX_DROP;
	} else {
		dc->seq.former_recv_seq = sequence;
	}

	/* Use master interface to receive */
	if (dc->dev[TYPE_MASTER] != NULL) {
		skb->dev = dc->dev[TYPE_MASTER];
		skb->skb_iif = dc->dev[TYPE_MASTER]->ifindex;
	}
	spin_unlock_bh(&dcm->dc_lock);

	return NET_RX_SUCCESS;
}

static struct hw_dc_ops g_dc_ops = {
	.dc_send_copy = hip2p_send_dc_copy,
	.dc_receive = hip2p_check_dup_packet,
};

static void netdev_down(const struct net_device *dev)
{
	dc_clear_by_dev(dev);
}

static int dc_netdev_event(struct notifier_block *this, unsigned long event,
	void *ptr)
{
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);
	switch (event) {
	case NETDEV_DOWN:
	case NETDEV_CHANGEMTU:
	case NETDEV_CHANGEADDR:
	case NETDEV_CHANGENAME:
		netdev_down(dev);
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block g_dc_netdev_notifier = {
	.notifier_call = dc_netdev_event,
};

static int __init hip2p_init(void)
{
	int ret;
	ret = dcm_init();
	if (ret != HIP2P_SUCC) {
		dc_mloge("dcm init failed\n");
		return HIP2P_NOK;
	}
	hip2p_proc_create();
	ret = register_netdevice_notifier(&g_dc_netdev_notifier);
	if (ret) {
		hip2p_proc_remove();
		return HIP2P_NOK;
	}

	ret = hw_register_dual_connection(&g_dc_ops);
	if (ret) {
		hip2p_proc_remove();
		unregister_netdevice_notifier(&g_dc_netdev_notifier);
		return HIP2P_NOK;
	}

	ret = netlink_init();
	if (ret != HIP2P_SUCC) {
		dc_mloge("proc create failed");
		hip2p_proc_remove();
		unregister_netdevice_notifier(&g_dc_netdev_notifier);
		hw_unregister_dual_connection();
		return HIP2P_NOK;
	}

	return HIP2P_OK;
}

static void __exit hip2p_exit(void)
{
	dcm_exit();
	hip2p_proc_remove();
	unregister_netdevice_notifier(&g_dc_netdev_notifier);
	hw_unregister_dual_connection();
}

module_init(hip2p_init);
module_exit(hip2p_exit);

MODULE_LICENSE("GPL");
