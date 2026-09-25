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
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/topology.h>
#include "adrv.h"
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
 Prototype    : rnic_register_device_notifier
 Description  : Register netdevice ready notifer.
 Input        : dev_notifier: device notifier
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_register_device_notifier(
	const struct rnic_deivce_notifier_s *dev_notifier)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	if (dev_ctx->ready)
		if (dev_notifier->dev_notifier_func != NULL)
			dev_notifier->dev_notifier_func();

	dev_ctx->dev_notifier_func = dev_notifier->dev_notifier_func;
	RNIC_LOGH("succ.");

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_register_ps_iface_drop_notifier
 Description  : Register ps iface data drop notifier
 Input        : drop_notifer: drop notifier
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_register_ps_iface_drop_notifier(
	const struct rnic_ps_iface_drop_notifier_s *drop_notifier)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(drop_notifier->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", drop_notifier->devid);
		return -ENODEV;
	}

	priv->drop_notifier_func = drop_notifier->drop_notifier_func;

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_set_napi_config
 Description  : configure napi of netdevice.
 Input        : napi_config
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_set_napi_config(const struct rnic_napi_config_s *napi_config)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(napi_config->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", napi_config->devid);
		return -ENODEV;
	}

	priv->napi_enable = napi_config->napi_enable;
	priv->gro_enable  = napi_config->gro_enable;
	priv->napi_weight = napi_config->napi_weight;
	priv->napi.weight = napi_config->napi_weight;
	priv->napi_queue_length = napi_config->napi_queue_length;

	return 0;
}

/*****************************************************************************
 Prototype    : rnic_set_napi_weight
 Description  : Set napi weight.
 Input        : weight
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_set_napi_weight(uint8_t devid, uint8_t weight)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return -ENODEV;
	}

	priv->napi_weight = weight;
	priv->napi.weight = weight;

	return 0;
}

#ifdef CONFIG_HISI_CPUFREQ_DT
/*****************************************************************************
 Prototype    : rnic_init_cpu_freq
 Description  : init cpufreq req interface.
 Input        : void
 Output       : None
 Return Value : None
*****************************************************************************/
void rnic_init_cpu_freq(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	int first_cpu[RNIC_MAX_CLUSTERS]={0};
	int cluster_id = 0;
	int match_id = -1;
	int cpu, ret, i;

	/* find max cluster_id and the first cpu of each cluster. */
	for_each_online_cpu(cpu) {//lint !e713
		cluster_id = topology_physical_package_id(cpu);

		if (RNIC_VALID_CLUSTER(cluster_id) && (cluster_id != match_id)) {
			match_id = cluster_id;
			first_cpu[cluster_id] = cpu;
		}
	}

	RNIC_LOGH("cluster num=%d, first_cpu[0-2]=%d %d %d.", \
		cluster_id + 1, first_cpu[0], first_cpu[1], first_cpu[2]);

	if (RNIC_VALID_CLUSTER(cluster_id)) {
		dev_ctx->dev_stats.act_cluster_num = (uint16_t)(cluster_id + 1);

		for (i = 0; i <= cluster_id; i++) {
			ret = hisi_cpufreq_init_req(&dev_ctx->cluster[i], first_cpu[i]);
			if (unlikely(ret)) {
				RNIC_LOGE("cluster %d call hisi_cpufreq_init_req fail, ret %d.", i, ret);
			} else {
				dev_ctx->cpufreq_req[i].init_succ = true;
				dev_ctx->cpufreq_req[i].first_cpu = (uint16_t)first_cpu[i];
			}
		}
	}
}

/*****************************************************************************
 Prototype    : rnic_update_cpu_freq
 Description  : req update cpu freq.
 Input        : level
 Output       : None
 Return Value : None
*****************************************************************************/
void rnic_update_cpu_freq(uint8_t level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	uint32_t req_freq;
	unsigned int cluster_id;

	if (!dev_ctx->cpufreq_ctrl_cfg.enable) {
		RNIC_LOGE("cpufreq_ctrl disabled.");
		return;
	}

	for (cluster_id = 0; cluster_id < dev_ctx->dev_stats.act_cluster_num; cluster_id++) {
		if (!dev_ctx->cpufreq_req[cluster_id].init_succ)
			break;

		/* if requested freq changed, then update cpufreq. */
		req_freq = dev_ctx->cpufreq_ctrl_cfg.level_cfg[level].req_freq[cluster_id];
		if (req_freq != dev_ctx->cpufreq_req[cluster_id].last_req_freq) {
			hisi_cpufreq_update_req(&dev_ctx->cluster[cluster_id], req_freq);
			dev_ctx->cpufreq_req[cluster_id].last_req_freq = req_freq;
			dev_ctx->dev_stats.cpufreq_update_num++;
		}
	}
}
#endif

#ifdef CONFIG_HISI_CORE_CTRL
/*****************************************************************************
 Prototype    : rnic_disable_isolation
 Description  : disable isolation.
 Input        : level
 Output       : None
 Return Value : None
*****************************************************************************/
void rnic_disable_isolation(uint8_t level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	if (dev_ctx->cpufreq_ctrl_cfg.enable &&
		dev_ctx->cpufreq_ctrl_cfg.level_cfg[level].isolation_disable) {
		core_ctl_set_boost(RNIC_DIS_ISOLATION_TIMEOUT);
		dev_ctx->dev_stats.dis_isolation_num++;
	}
}
#endif

/*****************************************************************************
 Prototype    : rnic_set_cpufreq_ctrl_config
 Description  : Set cpu freq ctrl config.
 Input        : cpufreq_ctrl_config: cpu freq ctrl config parameter
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_set_cpufreq_ctrl_config(
	const struct rnic_cpufreq_ctrl_config_s *cpufreq_ctrl_config)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	dev_ctx->cpufreq_ctrl_cfg.enable = cpufreq_ctrl_config->enable;

	memcpy(&dev_ctx->cpufreq_ctrl_cfg.level_cfg[0], /* unsafe_function_ignore: memcpy */
	       &cpufreq_ctrl_config->level_cfg[0],
	       sizeof(struct rnic_cpufreq_ctrl_level_config_s) * RNIC_CPU_FREQ_CTRL_MAX_LEVEL);

#ifdef CONFIG_HISI_CPUFREQ_DT
	if (dev_ctx->cpufreq_ctrl_cfg.enable)
		rnic_init_cpu_freq();
#endif
}

/*****************************************************************************
 Prototype    : rnic_set_cpufreq_level
 Description  : Set cpufreq level.
 Input        : level
 Output       : None
 Return Value : void
*****************************************************************************/
void rnic_set_cpufreq_level(uint8_t level)
{
#ifdef CONFIG_HISI_CPUFREQ_DT
	rnic_update_cpu_freq(level);
#endif
#ifdef CONFIG_HISI_CORE_CTRL
	rnic_disable_isolation(level);
#endif
}



/*****************************************************************************
 Prototype    : rnic_set_ps_iface_up
 Description  : Set ps iface up.
 Input        : ps_iface
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_set_ps_iface_up(const struct rnic_ps_iface_config_s *iface_config)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(iface_config->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", iface_config->devid);
		return -ENODEV;
	}

	switch (iface_config->ip_family) {
	case RNIC_IPV4_ADDR:
		memcpy(&priv->v4_info, &iface_config->info, /* unsafe_function_ignore: memcpy */
		       sizeof(struct rnic_ps_iface_info_s));
		priv->v4_data_tx_func = iface_config->data_tx_func;
		break;
	case RNIC_IPV6_ADDR:
		memcpy(&priv->v6_info, &iface_config->info, /* unsafe_function_ignore: memcpy */
		       sizeof(struct rnic_ps_iface_info_s));
		priv->v6_data_tx_func = iface_config->data_tx_func;
		break;
	default:
		RNIC_LOGE("ip_family invalid: %d.", iface_config->ip_family);
		return -EINVAL;
	}

	set_bit(iface_config->ip_family, &priv->addr_family_mask);



	return 0;
}

/*****************************************************************************
 Prototype    : rnic_set_ps_iface_down
 Description  : Set ps iface down.
 Input        : ps_iface
 Output       : None
 Return Value : Return 0 on success, negative on failure.
*****************************************************************************/
int rnic_set_ps_iface_down(const struct rnic_ps_iface_config_s *iface_config)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(iface_config->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", iface_config->devid);
		return -ENODEV;
	}

	switch (iface_config->ip_family) {
	case RNIC_IPV4_ADDR:
		memcpy(&priv->v4_info, &iface_config->info, /* unsafe_function_ignore: memcpy */
		       sizeof(struct rnic_ps_iface_info_s));
		priv->v4_data_tx_func = NULL;
		break;
	case RNIC_IPV6_ADDR:
		memcpy(&priv->v6_info, &iface_config->info, /* unsafe_function_ignore: memcpy */
		       sizeof(struct rnic_ps_iface_info_s));
		priv->v6_data_tx_func = NULL;
		break;
	default:
		RNIC_LOGE("ip_family invalid: %d.", iface_config->ip_family);
		return -EINVAL;
	}

	clear_bit(iface_config->ip_family, &priv->addr_family_mask);



	return 0;
}







