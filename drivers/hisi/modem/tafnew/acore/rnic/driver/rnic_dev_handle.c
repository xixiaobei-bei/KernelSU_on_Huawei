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
#include <linux/smp.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include "rnic_dev.h"
#include "rnic_dev_debug.h"
#include "rnic_dev_config.h"
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
 Prototype    : rnic_napi_check_skb_gro
 Description  : Check if skb can be handled by GRO engin.
 Input        : skb: sk buffer
 Output       : None
 Return Value : Return true if skb can be handled by GRO engin, otherwise false.
*****************************************************************************/
STATIC bool rnic_napi_check_skb_gro(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ipv6hdr *ipv6h;

	switch (skb->protocol) {
	case RNIC_NS_ETH_TYPE_IP:
		iph = (struct iphdr *)skb->data;
		if (iph->protocol == IPPROTO_TCP)
			return true;
		break;
	case RNIC_NS_ETH_TYPE_IPV6:
		ipv6h = (struct ipv6hdr *)skb->data;
		if (ipv6h->nexthdr == IPPROTO_TCP)
			return true;
		break;
	default:
		break;
	}

	return false;
}

/*****************************************************************************
 Prototype    : rnic_napi_rx_skb
 Description  : Rx handle function of packet.
 Input        : skb: sk buffer
                budget: max packets to be handled
 Output       : None
 Return Value : Number of packet handles.
*****************************************************************************/
/*lint -save -e801*/
STATIC int rnic_napi_rx_skb(struct rnic_dev_priv_s *priv, int budget)
{
	struct rnic_dev_ethhdr *eth;
	struct sk_buff *skb;
	unsigned long flags;
	bool again = true;
	int rx_packets = 0;
	uint32_t rx_bytes = 0;

	while (again) {
		while ((skb = __skb_dequeue(&priv->process_queue))) {
			rx_bytes += skb->len;
			eth = (skb->protocol == RNIC_NS_ETH_TYPE_IP) ?
							&priv->v4_eth_header : &priv->v6_eth_header;
			skb_push(skb, ETH_HLEN);
			memcpy(skb->data, eth, ETH_HLEN); /* unsafe_function_ignore: memcpy */
			skb->protocol = eth_type_trans(skb, priv->netdev);

			if (priv->gro_enable && rnic_napi_check_skb_gro(skb))
				napi_gro_receive(&priv->napi, skb);
			else
				netif_receive_skb(skb);

			if (++rx_packets >= budget)
				goto rx_completed;
		}

		spin_lock_irqsave(&priv->napi_queue.lock, flags);
		if (skb_queue_empty(&priv->napi_queue))
			again = false;
		else
			skb_queue_splice_tail_init(&priv->napi_queue,
					&priv->process_queue);
		spin_unlock_irqrestore(&priv->napi_queue.lock, flags);
	}

rx_completed:
	priv->stats.rx_packets += rx_packets;
	priv->stats.rx_bytes += rx_bytes;
	priv->data_stats.rx_packets += rx_packets;

	return rx_packets;
}
/*lint -restore +e801*/

/*****************************************************************************
 Prototype    : rnic_napi_poll
 Description  : Poll fuction of napi.
 Input        : napi: napi context
                budget: max packets to be handled
 Output       : None
 Return Value : Number of packet handles.
*****************************************************************************/
int rnic_napi_poll(struct napi_struct *napi, int budget)
{
	struct rnic_dev_priv_s *priv =
				container_of(napi, struct rnic_dev_priv_s, napi);
	uint32_t cur_cpu;
	int work = 0;

	if (unlikely(budget <= 0))
		return budget;

	cur_cpu = get_cpu();
	put_cpu();
	priv->lb_stats[cur_cpu].poll_on_cpu_num++;

	work = rnic_napi_rx_skb(priv, budget);

	/* If weight not fully consumed, exit the polling mode */
	if (work < budget)
		napi_complete(&priv->napi);

	return work;
}

/*****************************************************************************
 Prototype    : rnic_napi_schedule
 Description  : napi_schedule of rnic
 Input        : info: private info of netdeive
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_napi_schedule(void *info)
{
	struct rnic_dev_priv_s *priv = (struct rnic_dev_priv_s *)info;
	uint32_t cur_cpu;

	cur_cpu = get_cpu();
	put_cpu();

	if (napi_schedule_prep(&priv->napi)) {
		__napi_schedule(&priv->napi);
		priv->lb_stats[cur_cpu].schedul_on_cpu_num++;
	}
	else {
		priv->lb_stats[cur_cpu].schedule_fail_num++;
	}
}

/*****************************************************************************
 Prototype	  : rnic_napi_schedule_smp_on
 Description  : Buffer packet before polling.
 Input        : priv: private info of netdeive
                targ_cpu: target napi cpu
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_napi_schedule_smp_on(struct rnic_dev_priv_s *priv, int targ_cpu)
{
	int wait = 0; /* asynchronous IPI */

	if ((targ_cpu >= 0) && (targ_cpu < nr_cpu_ids)) {
		priv->lb_stats[targ_cpu].smp_on_cpu_num++;

		if (smp_call_function_single(targ_cpu, rnic_napi_schedule, priv, wait))
			priv->lb_stats[targ_cpu].smp_fail_num++;
	}
}

/*****************************************************************************
 Prototype    : rnic_napi_dispatcher_cb
 Description  : napi dispatch cb
 Input        : work: work for napi dispatch
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_napi_dispatcher_cb(struct work_struct * work)
{
	struct rnic_dev_priv_s *priv =
				container_of(work, struct rnic_dev_priv_s, napi_dispatcher_work);
	int napi_cpu;

	napi_cpu = atomic_read(&priv->napi_cpu);
	if ((napi_cpu >= 0) && (napi_cpu < nr_cpu_ids)) {
		get_online_cpus();
		if (!cpu_online(napi_cpu)) {
			rnic_napi_schedule(priv);
			priv->lb_stats[napi_cpu].dispatch_fail_num++;
		}
		else {
			rnic_napi_schedule_smp_on(priv, napi_cpu);
		}
		put_online_cpus();
	}
}

/*****************************************************************************
 Prototype    : rnic_select_cpu_candidacy
 Description  : Napi load balance napi_cpu select Algorithm.
 Input        : priv: private info of netdeive
 Output       : None
 Return Value : void
*****************************************************************************/
STATIC void rnic_select_cpu_candidacy(struct rnic_dev_priv_s *priv)
{
	unsigned int cpu = 0;
	int napi_cpu = atomic_read(&priv->napi_cpu);

	if (cpumask_weight(priv->lb_cpumask_candidacy) > 0) {
		napi_cpu = (int)cpumask_next(napi_cpu, priv->lb_cpumask_candidacy);
		if (napi_cpu >= nr_cpu_ids)
			napi_cpu = (int)cpumask_first(priv->lb_cpumask_candidacy);
	} else {
		/* restore lb_cpumask_candidacy, and lb_weight_remaind */
		for (cpu = 0; cpu < NR_CPUS; cpu++)
			priv->lb_weight_remaind[cpu] = priv->lb_weight_orig[cpu];

		cpumask_copy(priv->lb_cpumask_candidacy, priv->lb_cpumask_orig);
		napi_cpu = (int)cpumask_first(priv->lb_cpumask_candidacy);
	}

	if (napi_cpu >= nr_cpu_ids)
		napi_cpu = 0;

	if (priv->lb_weight_remaind[napi_cpu] > 0)
		priv->lb_weight_remaind[napi_cpu]--;

	atomic_set(&priv->napi_cpu, napi_cpu);

	if (priv->lb_weight_remaind[napi_cpu] == 0)
		cpumask_clear_cpu(napi_cpu, priv->lb_cpumask_candidacy);

	priv->lb_stats[napi_cpu].select_cpu_num++;
}

/*****************************************************************************
 Prototype    : rnic_net_rx_skb
 Description  : Packet reveive function of packet for non napi.
 Input        : skb: sk buffer
                priv: private info of netdevice
                in_intr: in interrupt
 Output       : None
 Return Value : void
*****************************************************************************/
STATIC void rnic_net_rx_skb(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv, bool in_intr)
{
	struct rnic_dev_ethhdr *eth;
	uint32_t data_len = skb->len;

	eth = (skb->protocol == RNIC_NS_ETH_TYPE_IP) ?
					&priv->v4_eth_header : &priv->v6_eth_header;
	skb_push(skb, ETH_HLEN);
	memcpy(skb->data, eth, ETH_HLEN); /* unsafe_function_ignore: memcpy */

	skb->protocol = eth_type_trans(skb, priv->netdev);

	if (in_intr)
		netif_rx(skb);
	else
		netif_rx_ni(skb);

	priv->stats.rx_packets++;
	priv->stats.rx_bytes += data_len;
	priv->data_stats.rx_packets++;
}

/*****************************************************************************
 Prototype    : rnic_net_rx_handle
 Description  : Host rx handle function.
 Input        : skb: sk buffer
                priv: private info of netdevice
 Output       : None
 Return Value : void
*****************************************************************************/
STATIC void rnic_net_rx_handle(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv)
{

	if (in_interrupt()) {
		if (priv->napi_enable)
			__skb_queue_tail(&priv->pend_queue, skb);
		else
			rnic_net_rx_skb(skb, priv, true);
	} else {
		rnic_net_rx_skb(skb, priv, false);
	}
}

/*****************************************************************************
 Prototype    : rnic_rx_handle
 Description  : Rx handle function of packet.
 Input        : devid: id of netdevice
                skb: sk buffer
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
/*lint -save -e801*/
int rnic_rx_handle(uint8_t devid, struct sk_buff *skb)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;

	dev = rnic_get_netdev_by_devid(devid);
	if (unlikely(!dev)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		rnic_kfree_skb(skb);
		return -ENODEV;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	priv->data_stats.rx_total_packets++;
	priv->dsflow_stats.rx_packets++;
	priv->dsflow_stats.rx_bytes += skb->len;

	if (unlikely(skb->len > ETH_DATA_LEN)) {
		priv->data_stats.rx_length_errors++;
		goto out_free_drop;
	}


	if (priv->state == RNIC_PHYS_LINK_DOWN) {
		priv->data_stats.rx_link_errors++;
		goto out_free_drop;
	}

	rnic_net_rx_handle(skb, priv);


	return 0;

out_free_drop:
	rnic_kfree_skb(skb);
	priv->stats.rx_dropped++;
	priv->data_stats.rx_dropped++;

	return -EFAULT;
}
/*lint -restore +e801*/

/*****************************************************************************
 Prototype    : rnic_rx_complete
 Description  : Rx complete function of packet.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : void
*****************************************************************************/
int rnic_rx_complete(uint8_t devid)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;
	struct sk_buff *skb;
	unsigned long flags;
	int napi_cpu;
	int cur_cpu;

	dev = rnic_get_netdev_by_devid(devid);
	if (unlikely(!dev)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return -ENODEV;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	if (priv->napi_enable) {
		if (skb_queue_len(&priv->napi_queue) < priv->napi_queue_length) {
			spin_lock_irqsave(&priv->napi_queue.lock, flags);
			skb_queue_splice_tail_init(&priv->pend_queue, &priv->napi_queue);
			spin_unlock_irqrestore(&priv->napi_queue.lock, flags);
		} else {
			while ((skb = __skb_dequeue(&priv->pend_queue))) {
				rnic_kfree_skb(skb);
				priv->stats.rx_dropped++;
				priv->data_stats.rx_dropped++;
				priv->data_stats.rx_enqueue_errors++;
			}
		}

		if (priv->napi_lb_enable && (priv->lb_cur_level != 0)) {
			napi_cpu = atomic_read(&priv->napi_cpu);
			cur_cpu = (int)get_cpu();
			put_cpu();

			if (!cpu_online(napi_cpu) || (cur_cpu == napi_cpu))
				rnic_napi_schedule(priv);
			else
				schedule_work(&priv->napi_dispatcher_work);
			rnic_select_cpu_candidacy(priv);
		} else {
			rnic_napi_schedule(priv);
		}
	}

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_tx_hanlde
 Description  : Tx handle function of packet.
 Input        : struct sk_buff *skb
                struct rnic_dev_priv_s *priv
 Output       : None
 Return Value : netdev_tx_t
*****************************************************************************/
/*lint -save -e801*/
netdev_tx_t rnic_tx_hanlde(struct sk_buff *skb,
					struct rnic_dev_priv_s *priv)
{
	rnic_ps_iface_data_tx_func data_tx_func;
	uint32_t data_len;
	int rc;

	priv->data_stats.tx_total_packets++;

	if (skb_linearize(skb)) {
		priv->data_stats.tx_linearize_errors++;
		goto out_free_drop;
	}

	if (skb->len < ETH_HLEN) {
		priv->data_stats.tx_length_errors++;
		goto out_free_drop;
	}

	skb_pull(skb, ETH_HLEN);
	priv->dsflow_stats.tx_bytes += skb->len;
	priv->dsflow_stats.tx_packets++;

	switch (ntohs(skb->protocol)) {
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
		if (ntohs(skb->protocol) == ETH_P_IP && priv->drop_notifier_func)
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
 Description  : Data stats function of netdevice.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : Return netdeivce data stats.
*****************************************************************************/
struct rnic_data_stats_s *rnic_get_data_stats(uint8_t devid)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;

	dev = rnic_get_netdev_by_devid(devid);
	if (unlikely(!dev)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return NULL;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);

	return &priv->data_stats;
}

/*****************************************************************************
 Prototype    : rnic_get_napi_stats
 Description  : Napi stats function of netdevice.
 Input        : devid: id of netdevice
                struct rnic_napi_stats_s *rnic_napi_stats
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_get_napi_stats(uint8_t devid, struct rnic_napi_stats_s *rnic_napi_stats)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;
	int act_cpu_nums = nr_cpu_ids;

	dev = rnic_get_netdev_by_devid(devid);
	if (unlikely(!dev)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return -EFAULT;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	rnic_napi_stats->napi_enable = priv->napi_enable;
	rnic_napi_stats->gro_enable = priv->gro_enable;
	rnic_napi_stats->napi_weight = priv->napi_weight;
	rnic_napi_stats->napi_lb_enable = priv->napi_lb_enable;
	rnic_napi_stats->lb_level = priv->lb_cur_level;
	rnic_napi_stats->act_cpu_nums = (uint8_t)act_cpu_nums;

	memcpy(rnic_napi_stats->lb_stats, priv->lb_stats, /* unsafe_function_ignore: memcpy */
		   sizeof(struct rnic_lb_stats_s) * \
		   ((act_cpu_nums > RNIC_NAPI_STATS_MAX_CPUS) ? RNIC_NAPI_STATS_MAX_CPUS : act_cpu_nums));

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_get_dsflow_stats
 Description  : Dsflow stats function of netdevice.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : Return netdeivce dsflow stats.
*****************************************************************************/
struct rnic_dsflow_stats_s *rnic_get_dsflow_stats(uint8_t devid)
{
	struct net_device *dev;
	struct rnic_dev_priv_s *priv;

	dev = rnic_get_netdev_by_devid(devid);
	if (unlikely(!dev)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return NULL;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);

	return &priv->dsflow_stats;
}

/*****************************************************************************
 Prototype    : rnic_clear_dsflow_stats
 Description  : Clear data flow stats.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_clear_dsflow_stats(uint8_t devid)
{
	struct net_device *dev;
	struct rnic_dev_priv_s *priv;

	dev = rnic_get_netdev_by_devid(devid);
	if (unlikely(!dev)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return -ENODEV;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	priv->dsflow_stats.rx_packets = 0;
	priv->dsflow_stats.tx_packets = 0;
	priv->dsflow_stats.rx_bytes   = 0;
	priv->dsflow_stats.tx_bytes   = 0;

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_show_data_stats
 Description  : Show data stats.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_show_data_stats(uint8_t devid)
{
	struct rnic_data_stats_s *data_stats;

	if (devid >= RNIC_DEV_ID_BUTT) {
		pr_err("devid %d overtop.\n", devid);
		return;
	}

	data_stats = rnic_get_data_stats(devid);
	if (!data_stats) {
		pr_err("get data stats fail, devid: %d.\n", devid);
		return;
	}

	pr_err("[RMNET%d] tx stats start:\n", devid);
	pr_err("[RMNET%d] tx_total_packets               %10u\n", devid, data_stats->tx_total_packets);
	pr_err("[RMNET%d] tx_packets                     %10u\n", devid, data_stats->tx_packets);
	pr_err("[RMNET%d] tx_dropped                     %10u\n", devid, data_stats->tx_dropped);
	pr_err("[RMNET%d] tx_length_errors               %10u\n", devid, data_stats->tx_length_errors);
	pr_err("[RMNET%d] tx_proto_errors                %10u\n", devid, data_stats->tx_proto_errors);
	pr_err("[RMNET%d] tx_carrier_errors              %10u\n", devid, data_stats->tx_carrier_errors);
	pr_err("[RMNET%d] tx_iface_errors                %10u\n", devid, data_stats->tx_iface_errors);
	pr_err("[RMNET%d] tx_linearize_errors            %10u\n", devid, data_stats->tx_linearize_errors);
	pr_err("[RMNET%d] tx stats end:\n", devid);
	pr_err("[RMNET%d] rx stats start:\n", devid);
	pr_err("[RMNET%d] rx_total_packets               %10u\n", devid, data_stats->rx_total_packets);
	pr_err("[RMNET%d] rx_packets                     %10u\n", devid, data_stats->rx_packets);
	pr_err("[RMNET%d] rx_dropped                     %10u\n", devid, data_stats->rx_dropped);
	pr_err("[RMNET%d] rx_length_errors               %10u\n", devid, data_stats->rx_length_errors);
	pr_err("[RMNET%d] rx_link_errors                 %10u\n", devid, data_stats->rx_link_errors);
	pr_err("[RMNET%d] rx_enqueue_errors              %10u\n", devid, data_stats->rx_enqueue_errors);
	pr_err("[RMNET%d] rx_trans_errors                %10u\n", devid, data_stats->rx_trans_errors);
	pr_err("[RMNET%d] rx stats end:\n", devid);
}

/*****************************************************************************
 Prototype    : rnic_show_napi_stats
 Description  : Show napi stats.
 Input        : devid: id of netdevice
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_show_napi_stats(uint8_t devid)
{
	struct rnic_napi_stats_s napi_stats;

	if (devid >= RNIC_DEV_ID_BUTT) {
		pr_err("devid %d overtop.\n", devid);
		return;
	}

	if (rnic_get_napi_stats(devid, &napi_stats)) {
		pr_err("get napi stats fail, devid: %d.\n", devid);
		return;
	}

	pr_err("[RMNET%d] napi enable        :(0:disable/1:enable)    %d\n", devid, napi_stats.napi_enable);
	pr_err("[RMNET%d] gro enable         :(0:disable/1:enable)    %d\n", devid, napi_stats.gro_enable);
	pr_err("[RMNET%d] napi weight        :                        %d\n", devid, napi_stats.napi_weight);
	pr_err("[RMNET%d] napi lb enable     :(0:disable/1:enable)    %d\n", devid, napi_stats.napi_lb_enable);
	pr_err("[RMNET%d] lb current levle   :                        %d\n", devid, napi_stats.lb_level);
	pr_err("[RMNET%d] select_cpu_num     : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].select_cpu_num,
		napi_stats.lb_stats[1].select_cpu_num,
		napi_stats.lb_stats[2].select_cpu_num,
		napi_stats.lb_stats[3].select_cpu_num,
		napi_stats.lb_stats[4].select_cpu_num,
		napi_stats.lb_stats[5].select_cpu_num,
		napi_stats.lb_stats[6].select_cpu_num,
		napi_stats.lb_stats[7].select_cpu_num);
	pr_err("[RMNET%d] schedul_on_cpu_num : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].schedul_on_cpu_num,
		napi_stats.lb_stats[1].schedul_on_cpu_num,
		napi_stats.lb_stats[2].schedul_on_cpu_num,
		napi_stats.lb_stats[3].schedul_on_cpu_num,
		napi_stats.lb_stats[4].schedul_on_cpu_num,
		napi_stats.lb_stats[5].schedul_on_cpu_num,
		napi_stats.lb_stats[6].schedul_on_cpu_num,
		napi_stats.lb_stats[7].schedul_on_cpu_num);

	pr_err("[RMNET%d] poll_on_cpu_num    : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].poll_on_cpu_num,
		napi_stats.lb_stats[1].poll_on_cpu_num,
		napi_stats.lb_stats[2].poll_on_cpu_num,
		napi_stats.lb_stats[3].poll_on_cpu_num,
		napi_stats.lb_stats[4].poll_on_cpu_num,
		napi_stats.lb_stats[5].poll_on_cpu_num,
		napi_stats.lb_stats[6].poll_on_cpu_num,
		napi_stats.lb_stats[7].poll_on_cpu_num);
	pr_err("[RMNET%d] smp_on_cpu_num     : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].smp_on_cpu_num,
		napi_stats.lb_stats[1].smp_on_cpu_num,
		napi_stats.lb_stats[2].smp_on_cpu_num,
		napi_stats.lb_stats[3].smp_on_cpu_num,
		napi_stats.lb_stats[4].smp_on_cpu_num,
		napi_stats.lb_stats[5].smp_on_cpu_num,
		napi_stats.lb_stats[6].smp_on_cpu_num,
		napi_stats.lb_stats[7].smp_on_cpu_num);
	pr_err("[RMNET%d] dispatch_fail_num  : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].dispatch_fail_num,
		napi_stats.lb_stats[1].dispatch_fail_num,
		napi_stats.lb_stats[2].dispatch_fail_num,
		napi_stats.lb_stats[3].dispatch_fail_num,
		napi_stats.lb_stats[4].dispatch_fail_num,
		napi_stats.lb_stats[5].dispatch_fail_num,
		napi_stats.lb_stats[6].dispatch_fail_num,
		napi_stats.lb_stats[7].dispatch_fail_num);
	pr_err("[RMNET%d] smp_fail_num       : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].smp_fail_num,
		napi_stats.lb_stats[1].smp_fail_num,
		napi_stats.lb_stats[2].smp_fail_num,
		napi_stats.lb_stats[3].smp_fail_num,
		napi_stats.lb_stats[4].smp_fail_num,
		napi_stats.lb_stats[5].smp_fail_num,
		napi_stats.lb_stats[6].smp_fail_num,
		napi_stats.lb_stats[7].smp_fail_num);
	pr_err("[RMNET%d] schedule_fail_num  : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].schedule_fail_num,
		napi_stats.lb_stats[1].schedule_fail_num,
		napi_stats.lb_stats[2].schedule_fail_num,
		napi_stats.lb_stats[3].schedule_fail_num,
		napi_stats.lb_stats[4].schedule_fail_num,
		napi_stats.lb_stats[5].schedule_fail_num,
		napi_stats.lb_stats[6].schedule_fail_num,
		napi_stats.lb_stats[7].schedule_fail_num);
	pr_err("[RMNET%d] hotplug_online_num : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].hotplug_online_num,
		napi_stats.lb_stats[1].hotplug_online_num,
		napi_stats.lb_stats[2].hotplug_online_num,
		napi_stats.lb_stats[3].hotplug_online_num,
		napi_stats.lb_stats[4].hotplug_online_num,
		napi_stats.lb_stats[5].hotplug_online_num,
		napi_stats.lb_stats[6].hotplug_online_num,
		napi_stats.lb_stats[7].hotplug_online_num);
	pr_err("[RMNET%d] hotplug_down_num   : %d %d %d %d %d %d %d %d\n",
		devid,
		napi_stats.lb_stats[0].hotplug_down_num,
		napi_stats.lb_stats[1].hotplug_down_num,
		napi_stats.lb_stats[2].hotplug_down_num,
		napi_stats.lb_stats[3].hotplug_down_num,
		napi_stats.lb_stats[4].hotplug_down_num,
		napi_stats.lb_stats[5].hotplug_down_num,
		napi_stats.lb_stats[6].hotplug_down_num,
		napi_stats.lb_stats[7].hotplug_down_num);
}



