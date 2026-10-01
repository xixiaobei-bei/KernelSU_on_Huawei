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

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <net/sock.h>
#include "rnic_dev.h"
#include "rnic_dev_handle.h"
#include "rnic_dev_debug.h"



/*****************************************************************************
 2. Function declarations
*****************************************************************************/


/*****************************************************************************
 3. Global defintions
*****************************************************************************/

struct rnic_dev_context_s rnic_dev_context;
unsigned int rnic_dev_log_level = RNIC_DEV_LOG_LVL_HIGH | RNIC_DEV_LOG_LVL_ERR;

static const uint8_t rnic_dev_dst_mac_base[ETH_ALEN] = {
	0x58,0x02,0x03,0x04,0x05,0x06
};

static const uint8_t rnic_dev_src_mac_base[ETH_ALEN] = {
	0x00,0x11,0x09,0x64,0x01,0x01
};

STATIC const struct rnic_dev_name_param_s rnic_dev_name_param_table[] = {
	RNIC_DEV_NAME_ELEMENT(RMNET0),
	RNIC_DEV_NAME_ELEMENT(RMNET1),
	RNIC_DEV_NAME_ELEMENT(RMNET2),
	RNIC_DEV_NAME_ELEMENT(RMNET3),
	RNIC_DEV_NAME_ELEMENT(RMNET4),
	RNIC_DEV_NAME_ELEMENT(RMNET5),
	RNIC_DEV_NAME_ELEMENT(RMNET6),
	RNIC_DEV_NAME_ELEMENT(RMNET_IMS00),
	RNIC_DEV_NAME_ELEMENT(RMNET_IMS10),
	RNIC_DEV_NAME_ELEMENT(RMNET_EMC0),
	RNIC_DEV_NAME_ELEMENT(RMNET_EMC1),
	RNIC_DEV_NAME_ELEMENT(RMNET_R_IMS00),
	RNIC_DEV_NAME_ELEMENT(RMNET_R_IMS01),
	RNIC_DEV_NAME_ELEMENT(RMNET_R_IMS10),
	RNIC_DEV_NAME_ELEMENT(RMNET_R_IMS11),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN00),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN01),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN02),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN03),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN04),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN10),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN11),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN12),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN13),
	RNIC_DEV_NAME_ELEMENT(RMNET_TUN14)
};


/*****************************************************************************
 4. Function defintions
*****************************************************************************/

/*****************************************************************************
 Prototype    : rnic_dev_open
 Description  : Open function of netdevice.
 Input        : struct net_device *dev
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
STATIC int rnic_dev_open(struct net_device *dev)
{
	struct rnic_dev_priv_s *priv =
				(struct rnic_dev_priv_s *)netdev_priv(dev);

	if (priv->napi_enable)
		napi_enable(&priv->napi);

	netif_start_queue(dev);
	priv->state = RNIC_PHYS_LINK_UP;

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_dev_stop
 Description  : Stop function of netdevice.
 Input        : dev: netdevice
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
STATIC int rnic_dev_stop(struct net_device *dev)
{
	struct rnic_dev_priv_s *priv =
				(struct rnic_dev_priv_s *)netdev_priv(dev);

	priv->state = RNIC_PHYS_LINK_DOWN;
	netif_stop_queue(dev);

	if (priv->napi_enable) {
		skb_queue_purge(&priv->napi_queue);
		napi_disable(&priv->napi);
	}

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_dev_start_xmit
 Description  : Xmit function of netdeivce.
 Input        : skb: sk buffer
                dev: netdevice
 Output       : None
 Return Value : netdev_tx_t
*****************************************************************************/
STATIC netdev_tx_t rnic_dev_start_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct rnic_dev_priv_s *priv =
				(struct rnic_dev_priv_s *)netdev_priv(dev);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
	/* The value is a bit-shift of 1 second,
	 * so 4 is ~64ms of queued data. Only affects local TCP
	 * sockets.
	 */
	sk_pacing_shift_update(skb->sk, 4);
#endif

	rnic_tx_hanlde(skb, priv);

	return NETDEV_TX_OK;
}

/*****************************************************************************
 Prototype    : rnic_dev_change_mtu
 Description  : Mtu function of netdeivce.
 Input        : dev: netdevice
                new_mtu: mtu
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
STATIC int rnic_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	struct rnic_dev_priv_s *priv =
				(struct rnic_dev_priv_s *)netdev_priv(dev);

	if (RNIC_RMNET_R_IMS_IS_VALID(priv->devid)){
		if (new_mtu > RNIC_R_IMS_ETH_DATA_LEN)
			return -EINVAL;
	} else {
		if (new_mtu > ETH_DATA_LEN)
			return -EINVAL;
	}

	dev->mtu = (unsigned int)new_mtu;

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_dev_get_stats
 Description  : Stats function of netdevice.
 Input        : dev: netdevice
 Output       : None
 Return Value : Return netdeivce stats.
*****************************************************************************/
STATIC struct net_device_stats *rnic_dev_get_stats(struct net_device *dev)
{
	struct rnic_dev_priv_s *priv =
				(struct rnic_dev_priv_s *)netdev_priv(dev);

	return &priv->stats;
}

STATIC const struct net_device_ops rnic_dev_ops = {
	.ndo_open			= rnic_dev_open,
	.ndo_stop			= rnic_dev_stop,
	.ndo_start_xmit		= rnic_dev_start_xmit,
	.ndo_change_mtu		= rnic_dev_change_mtu,
	.ndo_get_stats		= rnic_dev_get_stats,
};

/*****************************************************************************
 Prototype    : rnic_get_netdev_by_devid
 Description  : Get netdevice.
 Output       : devid: id of netdeivce
 Output       : None
 Return Value : net_device
*****************************************************************************/
struct net_device *rnic_get_netdev_by_devid(uint8_t devid)
{
	return rnic_dev_context.netdev[devid];
}

/*****************************************************************************
 Prototype    : rnic_get_devid_by_name
 Description  : Get id of netdeivce.
 Input        : name: name of netdevice
 Output       : devid: id of netdeivce
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_get_devid_by_name(char *name, uint8_t *devid)
{
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;

	if (!name || !devid) {
		return -EINVAL;
	}

	dev = dev_get_by_name(&init_net, name);
	if (!dev) {
		RNIC_LOGE("invalid param.");
		return -ENODEV;
	}

	priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
	*devid = priv->devid;
	dev_put(dev);

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_get_name_param
 Description  : Get name parameter.
 Input        : devid: id of netdeivce
 Output       : None
 Return Value : rnic_dev_name_param_s
*****************************************************************************/
STATIC const struct rnic_dev_name_param_s *rnic_get_name_param(uint8_t devid)
{
	const struct rnic_dev_name_param_s *name_param = NULL;
	uint8_t i;

	for (i = 0; i < ARRAY_SIZE(rnic_dev_name_param_table); i++) {
		if (rnic_dev_name_param_table[i].devid == devid) {
			name_param = &rnic_dev_name_param_table[i];
			break;
		}
	}

	return name_param;
}

/*****************************************************************************
 Prototype    : rnic_cleanup
 Description  : Destory netdevice.
 Input        : void
 Output       : None
 Return Value : None
*****************************************************************************/
STATIC void rnic_cleanup(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	struct net_device *dev;
	uint32_t devid;

	for (devid = 0; devid < RNIC_DEV_ID_BUTT; devid++) {
		dev = dev_ctx->netdev[devid];
		if (dev) {
			unregister_netdev(dev);
			free_netdev(dev);
		}

		dev_ctx->netdev[devid] = NULL;
		dev_ctx->priv[devid] = NULL;
	}

	dev_ctx->ready = false;
}

/*****************************************************************************
 Prototype    : rnic_create_netdev
 Description  : Create netdevice.
 Input        : void
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
/*lint -save -e801*/
int rnic_create_netdev(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	struct rnic_dev_priv_s *priv;
	struct net_device *dev;
	const struct rnic_dev_name_param_s *name_param;
	uint8_t dst_mac[ETH_ALEN], src_mac[ETH_ALEN];
	int rc = 0;
	uint8_t devid;

	memset(dev_ctx->netdev, 0, /* unsafe_function_ignore: memset */
		sizeof(struct net_device *) * RNIC_DEV_ID_BUTT);

	for (devid = 0; devid < RNIC_DEV_ID_BUTT; devid++) {
		name_param = rnic_get_name_param(devid);
		if (!name_param) {
			rc = -ENODEV;
			goto err_name_param;
		}

		memcpy(dst_mac, rnic_dev_dst_mac_base, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		memcpy(src_mac, rnic_dev_src_mac_base, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		dst_mac[ETH_ALEN - 1] += devid;
		src_mac[ETH_ALEN - 1] += devid;

		dev = alloc_etherdev(sizeof(struct rnic_dev_priv_s));
		if (!dev) {
			RNIC_LOGE("alloc etherdev fail, devid: %d.", devid);
			rc = -ENOMEM;
			goto err_alloc_netdev;
		}

		scnprintf(dev->name, IFNAMSIZ, "%s%s",
			rnic_dev_name_param_table[devid].prefix,
			rnic_dev_name_param_table[devid].suffix);
		dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
		dev->mtu = ETH_DATA_LEN;
		memcpy(dev->dev_addr, dst_mac, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		dev->netdev_ops = &rnic_dev_ops;

		priv = (struct rnic_dev_priv_s *)netdev_priv(dev);
		priv->netdev = dev;
		priv->devid = devid;

		priv->napi_enable = true;
		priv->gro_enable  = true;
		priv->napi_weight = 16;
		priv->napi_queue_length = 10000;
		skb_queue_head_init(&priv->napi_queue);
		netif_napi_add(dev, &priv->napi, rnic_napi_poll, 16);

		memcpy(priv->v4_eth_header.h_dest, dst_mac, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		memcpy(priv->v4_eth_header.h_source, src_mac, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		priv->v4_eth_header.h_proto = htons(ETH_P_IP);/*lint !e778*/

		memcpy(priv->v6_eth_header.h_dest, dst_mac, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		memcpy(priv->v6_eth_header.h_source, src_mac, ETH_ALEN); /* unsafe_function_ignore: memcpy */
		priv->v6_eth_header.h_proto = htons(ETH_P_IPV6);/*lint !e778*/

		dev->features |= NETIF_F_SG;
		dev->hw_features |= NETIF_F_SG;

		rc = register_netdev(dev);
		if (rc) {
			RNIC_LOGE("register netdev fail, devid: %d.", devid);
			goto err_register_netdev;
		}

		dev_ctx->netdev[devid] = dev;
		dev_ctx->priv[devid] = priv;
	}

	dev_ctx->ready = true;
	if (dev_ctx->dev_notifier_func)
		dev_ctx->dev_notifier_func();

	return 0;

err_register_netdev:
	free_netdev(dev);
err_alloc_netdev:
err_name_param:
	rnic_cleanup();

	return rc;
}
/*lint -restore +e801*/

/*****************************************************************************
 Prototype    : rnic_init.
 Description  : Init function of rnic.
 Input        : void
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int __init rnic_init(void)
{
	int rc;

	rc = rnic_create_netdev();
	RNIC_LOGH("%s.", (!rc)? "succ" : "fail");

	return rc;
}

/*****************************************************************************
 Prototype    : rnic_exit.
 Description  : Exit function of rnic.
 Input        : void
 Output       : None
 Return Value : None
*****************************************************************************/
STATIC void __exit rnic_exit(void)
{
	rnic_cleanup();
	RNIC_LOGH("succ.");
}

module_param(rnic_dev_log_level, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(rnic_dev_log_level, "rnic device log level");
module_init(rnic_init);
module_exit(rnic_exit);


