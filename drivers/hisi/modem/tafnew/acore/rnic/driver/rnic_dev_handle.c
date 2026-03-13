/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/*****************************************************************************
 1. Other files included
*****************************************************************************/

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include "rnic_dev.h"
#include "rnic_dev_debug.h"
#include "rnic_dev_handle.h"



/*****************************************************************************
 2. Global defintions
*****************************************************************************/


/*****************************************************************************
 3. Function defintions
*****************************************************************************/

/*****************************************************************************
 Prototype    : rnic_kfree_skb
 Description  : Free sk buffer.
 Input        : skb: sk buffer
 Output       : None
 Return Value : void
*****************************************************************************/
STATIC void rnic_kfree_skb(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}


/*****************************************************************************
 Prototype    : rnic_rx_skb
 Description  : Packet reveive function of packet for napi.
 Input        : struct sk_buff *skb
                struct rnic_dev_priv_s *priv
 Output       : None
 Return Value :
*****************************************************************************/
STATIC void rnic_napi_rx_skb(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv)
{
	if (priv->gro_enable)
		napi_gro_receive(&priv->napi, skb);
	else
		netif_receive_skb(skb);
}

/*****************************************************************************
 Prototype    : rnic_dev_napi_poll
 Description  : Poll fuction of napi.
 Input        : struct napi_struct *napi
                int budget
 Output       : None
 Return Value :
*****************************************************************************/
int rnic_napi_poll(struct napi_struct *napi, int budget)
{
	struct rnic_dev_priv_s *priv =
				container_of(napi, struct rnic_dev_priv_s, napi);
	struct sk_buff *skb;
	uint32_t data_len;
	int rx_num = 0;

	while (rx_num < budget) {
		skb = skb_dequeue(&priv->napi_queue);
		if (!skb)
			break;

		data_len = skb->len - ETH_HLEN;

		rnic_napi_rx_skb(skb, priv);
		priv->stats.rx_bytes += data_len;
		priv->stats.rx_packets++;
		priv->data_stats.rx_packets++;

		rx_num++;
	}

	/* If weight not fully consumed, exit the polling mode */
	if (rx_num < budget)
		napi_complete(&priv->napi);

	return rx_num;
}

/*****************************************************************************
 Prototype    : rnic_napi_enqueue
 Description  : Buffer packet before polling.
 Input        : skb: sk buffer
                priv: private info of netdeive
 Output       : None
 Return Value :
*****************************************************************************/
STATIC int rnic_napi_enqueue(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv)
{
	if (skb_queue_len(&priv->napi_queue) < priv->napi_queue_length) {
		skb_queue_tail(&priv->napi_queue, skb);
	} else {
		rnic_kfree_skb(skb);
		priv->stats.rx_dropped++;
		priv->data_stats.rx_dropped++;
		priv->data_stats.rx_enqueue_errors++;
	}

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_net_rx_handle
 Description  : Host rx handle function.
 Input        : skb: sk buffer
                priv: private info of netdevice
                ip_family: ip type (ipv4 or ipv6)
 Output       : None
 Return Value :
*****************************************************************************/
STATIC void rnic_net_rx_handle(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv,
					uint8_t ip_family)
{
	struct rnic_dev_ethhdr *eth;
	uint32_t data_len = skb->len;


	eth = (ip_family == RNIC_IPV4_ADDR) ?
					&priv->v4_eth_header : &priv->v6_eth_header;
	skb_push(skb, ETH_HLEN);
	memcpy(skb->data, eth, ETH_HLEN); /* unsafe_function_ignore: memcpy */

	skb->protocol = eth_type_trans(skb, priv->netdev);

	if (in_interrupt()) {
		if (priv->napi_enable) {
			rnic_napi_enqueue(skb, priv);
		}
		else {
			netif_rx(skb);
			priv->stats.rx_packets++;
			priv->stats.rx_bytes += data_len;
			priv->data_stats.rx_packets++;
		}
	} else {
		netif_rx_ni(skb);
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += data_len;
		priv->data_stats.rx_packets++;
	}
}

/*****************************************************************************
 Prototype    : rnic_rx_complete
 Description  : Rx complete function of packet.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_rx_complete(uint8_t devid)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;

	dev = rnic_get_netdev_by_devid(devid);
	if (!dev) {
		RNIC_LOGE("device not found.");
		return;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	if (priv->napi_enable)
		napi_schedule(&priv->napi);
}

/*****************************************************************************
 Prototype    : rnic_rx_handle
 Description  : Rx handle function of packet.
 Input        : devid: id of netdevice
                skb: sk buffer
                ip_family: ip type (ipv4 or ipv6)
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
/*lint -save -e801*/
int rnic_rx_handle(uint8_t devid, struct sk_buff *skb, uint8_t ip_family)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;

	dev = rnic_get_netdev_by_devid(devid);
	if (!dev) {
		RNIC_LOGE("device not found.");
		rnic_kfree_skb(skb);
		return -ENODEV;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	priv->data_stats.rx_total_packets++;
	priv->dsflow_stats.rx_bytes += skb->len;
	priv->dsflow_stats.rx_packets++;

	if (skb->len > ETH_DATA_LEN) {
		priv->data_stats.rx_length_errors++;
		goto out_free_drop;
	}

	if (priv->state == RNIC_PHYS_LINK_DOWN) {
		priv->data_stats.rx_phys_errors++;
		goto out_free_drop;
	}

	rnic_net_rx_handle(skb, priv, ip_family);

	return 0;

out_free_drop:
	rnic_kfree_skb(skb);
	priv->stats.rx_dropped++;
	priv->data_stats.rx_dropped++;

	return -EFAULT;
}
/*lint -restore +e801*/

/*****************************************************************************
 Prototype    : rnic_tx_hanlde
 Description  : Tx handle function of packet.
 Input        : struct sk_buff *skb
                struct rnic_dev_priv_s *priv
 Output       : None
 Return Value :
*****************************************************************************/
/*lint -save -e801*/
netdev_tx_t rnic_tx_hanlde(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv)
{
	rnic_ps_iface_data_tx_func data_tx_func;
	uint32_t data_len;
	int rc;
	uint16_t proto;

	priv->data_stats.tx_total_packets++;

	if (skb_linearize(skb)) {
		priv->data_stats.tx_linearize_errors++;
		goto out_free_drop;
	}

	if (skb->len < ETH_HLEN) {
		priv->data_stats.tx_length_errors++;
		goto out_free_drop;
	}

	proto = ((struct ethhdr *)skb->data)->h_proto;
	skb_pull(skb, ETH_HLEN);

	priv->dsflow_stats.tx_bytes += skb->len;
	priv->dsflow_stats.tx_packets++;

	switch (ntohs(proto)) {
	case ETH_P_IP:
		data_tx_func = priv->v4_data_tx_func;
		break;
	case ETH_P_IPV6:
		data_tx_func = priv->v6_data_tx_func;
		break;
	default:
		priv->data_stats.tx_proto_errors++;
		goto out_free_drop;
	}

	if (data_tx_func) {
		data_len = skb->len;
		rc = data_tx_func(skb, priv->devid);
	} else {
		if (ntohs(proto) == ETH_P_IP && priv->drop_notifier_func)
			priv->drop_notifier_func(priv->devid);

		priv->data_stats.tx_carrier_errors++;
		goto out_free_drop;
	}

	if (!rc) {
		priv->stats.tx_bytes += data_len;
		priv->stats.tx_packets++;
		priv->data_stats.tx_packets++;
	} else {
		priv->stats.tx_dropped++;
		priv->data_stats.tx_dropped++;
		priv->data_stats.tx_iface_errors++;
	}

	return NETDEV_TX_OK;

out_free_drop:
	rnic_kfree_skb(skb);
	priv->stats.tx_dropped++;
	priv->data_stats.tx_dropped++;

	return NETDEV_TX_OK;
}
/*lint -restore +e801*/

/*****************************************************************************
 Prototype    : rnic_get_data_stats
 Description  :
 Input        : uint8_t devid
 Output       : None
 Return Value :
*****************************************************************************/
struct rnic_data_stats_s *rnic_get_data_stats(uint8_t devid)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;

	dev = rnic_get_netdev_by_devid(devid);
	if (!dev) {
		RNIC_LOGE("device not found.");
		return NULL;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);

	return &priv->data_stats;
}

/*****************************************************************************
 Prototype    : rnic_get_dsflow_stats
 Description  :
 Input        : uint8_t devid
 Output       : None
 Return Value :
*****************************************************************************/
struct rnic_dsflow_stats_s *rnic_get_dsflow_stats(uint8_t devid)
{
	struct net_device *dev;
	struct rnic_dev_priv_s *priv;

	dev = rnic_get_netdev_by_devid(devid);
	if (!dev) {
		RNIC_LOGE("device not found.");
		return NULL;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);

	return &priv->dsflow_stats;
}

/*****************************************************************************
 Prototype    : rnic_clear_dsflow_stats
 Description  : Clear data flow stats.
 Input        : uint8_t devid
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_clear_dsflow_stats(uint8_t devid)
{
	struct net_device *dev;
	struct rnic_dev_priv_s *priv;

	dev = rnic_get_netdev_by_devid(devid);
	if (!dev) {
		RNIC_LOGE("device not found.");
		return -ENODEV;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	priv->dsflow_stats.rx_packets = 0;
	priv->dsflow_stats.tx_packets = 0;
	priv->dsflow_stats.rx_bytes   = 0;
	priv->dsflow_stats.tx_bytes   = 0;

	return 0;
}




