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

#ifndef __RNIC_DEV_H__
#define __RNIC_DEV_H__

/*****************************************************************************
  1. Other file included
*****************************************************************************/

#include <linux/types.h>
#include <linux/etherdevice.h>
#include "product_config.h"
#include "rnic_dev_def.h"
#include "rnic_dev_i.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
  2. Macro definitions
*****************************************************************************/

#if (defined(LLT_OS_VER))
#define STATIC
#else
#define STATIC static
#endif

#define RNIC_DEV_CTX()		(&rnic_dev_context)

#if (defined(CONFIG_BALONG_SPE))
#if (defined(CONFIG_BALONG_SPE_WAN))
#define RNIC_SPE_IPF_PORT_FLAG	(0)
#else
#define RNIC_SPE_IPF_PORT_FLAG	(1)
#endif
#endif

#if (FEATURE_ON == FEATURE_RMNET_CUSTOM)
#define RNIC_DEV_NAME_PREFIX	"eth_x"
#else
#define RNIC_DEV_NAME_PREFIX	"rmnet"
#endif

#if (FEATURE_ON == FEATURE_RMNET_CUSTOM)
#define RNIC_DEV_NAME_SUFFIX_RMNET0			""
#else
#define RNIC_DEV_NAME_SUFFIX_RMNET0			"0"
#endif
#define RNIC_DEV_NAME_SUFFIX_RMNET1			"1"
#define RNIC_DEV_NAME_SUFFIX_RMNET2			"2"
#define RNIC_DEV_NAME_SUFFIX_RMNET3			"3"
#define RNIC_DEV_NAME_SUFFIX_RMNET4			"4"
#define RNIC_DEV_NAME_SUFFIX_RMNET5			"5"
#define RNIC_DEV_NAME_SUFFIX_RMNET6			"6"

#define RNIC_DEV_NAME_SUFFIX_RMNET_IMS00	"_ims00"
#define RNIC_DEV_NAME_SUFFIX_RMNET_IMS10	"_ims10"
#define RNIC_DEV_NAME_SUFFIX_RMNET_EMC0		"_emc0"
#define RNIC_DEV_NAME_SUFFIX_RMNET_EMC1		"_emc1"

#define RNIC_DEV_NAME_SUFFIX_RMNET_R_IMS00	"_r_ims00"
#define RNIC_DEV_NAME_SUFFIX_RMNET_R_IMS01	"_r_ims01"
#define RNIC_DEV_NAME_SUFFIX_RMNET_R_IMS10	"_r_ims10"
#define RNIC_DEV_NAME_SUFFIX_RMNET_R_IMS11	"_r_ims11"

#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN00	"_tun00"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN01	"_tun01"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN02	"_tun02"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN03	"_tun03"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN04	"_tun04"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN10	"_tun10"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN11	"_tun11"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN12	"_tun12"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN13	"_tun13"
#define RNIC_DEV_NAME_SUFFIX_RMNET_TUN14	"_tun14"

#define RNIC_DEV_NAME_ELEMENT(ifname)\
			{\
				RNIC_DEV_ID_##ifname,\
				RNIC_DEV_NAME_PREFIX,\
				RNIC_DEV_NAME_SUFFIX_##ifname,\
			}

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_DELAY_MODEM_INIT))
#define RNIC_R_IMS_ETH_DATA_LEN			(2000)
#define RNIC_RMNET_R_IMS_IS_VALID(RmNetId) \
			((RNIC_DEV_ID_RMNET_R_IMS00 == (RmNetId)) \
			|| (RNIC_DEV_ID_RMNET_R_IMS10 == (RmNetId)) \
			|| (RNIC_DEV_ID_RMNET_R_IMS01 == (RmNetId)) \
			|| (RNIC_DEV_ID_RMNET_R_IMS11 == (RmNetId)))
#endif

/*****************************************************************************
  3. Enumerations declatations
*****************************************************************************/

enum rnic_phys_link_state_e {
	RNIC_PHYS_LINK_DOWN = 0,
	RNIC_PHYS_LINK_UP
};


/*****************************************************************************
  4. STRUCT and UNION declatations
*****************************************************************************/

struct rnic_dev_name_param_s {
	uint8_t devid;
	char *prefix;
	char *suffix;
};

struct rnic_dev_ethhdr {
	uint8_t h_dest[ETH_ALEN];
	uint8_t h_source[ETH_ALEN];
	uint16_t h_proto;
};

struct rnic_dev_context_s {
	uint32_t ready;
	rnic_device_notifier_func dev_notifier_func;

	struct net_device *netdev[RNIC_DEV_ID_BUTT];
	struct rnic_dev_priv_s *priv[RNIC_DEV_ID_BUTT];

#if (defined(CONFIG_BALONG_SPE))
	int spe_port;
	int ipf_port_flag;
#endif
};

struct rnic_dev_priv_s {
	struct net_device *netdev;
	struct net_device_stats stats;
	struct rnic_data_stats_s data_stats;
	struct rnic_dsflow_stats_s dsflow_stats;

	unsigned long state;

	uint8_t devid;
	uint8_t napi_enable;
	uint8_t gro_enable;
	uint8_t napi_weight;
	uint32_t napi_queue_length;

	struct sk_buff_head napi_queue ____cacheline_aligned;
	struct napi_struct napi ____cacheline_aligned;

	struct rnic_dev_ethhdr v4_eth_header ____cacheline_aligned;
	struct rnic_dev_ethhdr v6_eth_header;

#if (defined(CONFIG_BALONG_SPE))
	int spe_port;
	int ipf_port_flag;
#endif

	struct rnic_ps_iface_info_s v4_info;
	struct rnic_ps_iface_info_s v6_info;

	rnic_ps_iface_data_tx_func v4_data_tx_func;
	rnic_ps_iface_data_tx_func v6_data_tx_func;
	rnic_ps_iface_drop_notifier_func drop_notifier_func;
};


/*****************************************************************************
 5. Global declaration
*****************************************************************************/

extern struct rnic_dev_context_s rnic_dev_context;


/*******************************************************************************
  6. Function declarations
*******************************************************************************/

struct net_device *rnic_get_netdev_by_devid(uint8_t devid);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_DEV_H__ */
