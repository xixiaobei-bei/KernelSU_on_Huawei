/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/bitops.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/topology.h>
#include "adrv.h"
#if (defined(CONFIG_BALONG_ESPE))
#include "mdrv_wan.h"
#endif /* CONFIG_BALONG_ESPE */
#include "securec.h"
#include "rnic_dev.h"
#include "rnic_dev_debug.h"
#include "rnic_dev_config.h"
#include "rnic_dev_handle.h"
#include "rnic_policy_manage.h"


/*
 * rnic_register_device_notifier - Register netdevice ready notifer.
 * @dev_notifier: notifer block
 *
 * Returns 0 on success, negative on failure.
 */
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

/*
 * rnic_register_ps_iface_drop_notifier - Register iface data drop notifier.
 * @drop_notifier: notifer block
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_register_ps_iface_drop_notifier(
	const struct rnic_ps_iface_drop_notifier_s *drop_notifier)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(drop_notifier->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.",
			  drop_notifier->devid);
		return -ENODEV;
	}

	priv->drop_notifier_func = drop_notifier->drop_notifier_func;

	return 0;
}

/*
 * rnic_set_napi_config - Configure napi feature for specific netdevice.
 * @napi_config: configuration of napi
 *
 * Returns 0 on success, negative on failure.
 */
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

/*
 * rnic_set_napi_weight - Configure napi weight for specific netdevice.
 * @weight: poll weight
 *
 * Returns 0 on success, negative on failure.
 */
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
/*
 * rnic_init_cpu_freq - Initialize cpufreq request interface.
 *
 * Returns 0 on success, negative on failure.
 */
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
		for (i = 0; i <= cluster_id; i++) {
			ret = hisi_cpufreq_init_req(&dev_ctx->cluster[i], first_cpu[i]);
			if (unlikely(ret)) {
				RNIC_LOGE("cluster %d call hisi_cpufreq_init_req fail, ret %d.", i, ret);
			} else {
				dev_ctx->cpufreq_req[i].init_succ = true;
				dev_ctx->cpufreq_req[i].first_cpu = (uint8_t)first_cpu[i];
			}
		}
	}
}

/*
 * rnic_update_cpu_freq - Update request frequency.
 * @level: rhc levle
 */
void rnic_update_cpu_freq(uint8_t level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	uint32_t req_freq;
	unsigned int cluster_id;

	dev_ctx->act_cluster_num = dev_ctx->act_cluster_num < RNIC_MAX_CLUSTERS ? dev_ctx->act_cluster_num : RNIC_MAX_CLUSTERS;
	for (cluster_id = 0; cluster_id < dev_ctx->act_cluster_num; cluster_id++) {
		if (!dev_ctx->cpufreq_req[cluster_id].init_succ)
			break;

		/* if requested freq changed, then update cpufreq. */
		req_freq = dev_ctx->rhc_cfg.level_cfg[level].req_freq[cluster_id];
		if (req_freq != dev_ctx->cpufreq_req[cluster_id].last_req_freq) {
			hisi_cpufreq_update_req(&dev_ctx->cluster[cluster_id], req_freq);
			dev_ctx->cpufreq_req[cluster_id].last_req_freq = req_freq;
			dev_ctx->qos_stats.cpufreq_update_num++;
		}
	}
}
#endif


#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
/*
 * rnic_add_ddr_req - add pm qos request interface.
 * @pm_qos_req: pointer to a preallocated handle
 */
void rnic_add_ddr_req(struct pm_qos_request *pm_qos_req)
{
	pm_qos_add_request(pm_qos_req, PM_QOS_MEMORY_LATENCY, 0);
}

/*
 * rnic_update_ddr_req_pps - update PM_QOS_MEMORY_LATENCY req by pps level.
 * @level: rcc config level
 */
void rnic_update_ddr_req_pps(uint8_t level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	int req_bd = dev_ctx->rhc_cfg.level_cfg[level].ddr_bd;

	if (req_bd != dev_ctx->ddr_req_bd_pps) {
		pm_qos_update_request(&dev_ctx->pm_qos_req_pps, req_bd);
		dev_ctx->ddr_req_bd_pps = req_bd;
		dev_ctx->qos_stats.ddr_req_pps_num++;
	}
}

/*
 * rnic_update_ddr_req_qcc - update PM_QOS_MEMORY_LATENCY req.
 * @pm_qos_req: pointer to a preallocated handle
 * @bandwidth: DDR bandwidth
 */
void rnic_update_ddr_req_qcc(int bandwidth)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	pm_qos_update_request(&dev_ctx->pm_qos_req_qcc, bandwidth);
}

/*
 * rnic_remove_ddr_req - remove pm qos request interface.
 * @pm_qos_req: pointer to a preallocated handle
 */
void rnic_remove_ddr_req(struct pm_qos_request *pm_qos_req)
{
	pm_qos_remove_request(pm_qos_req);
}
#endif

/*
 * rnic_init_rcc - Initialize configration ctrl.
 */
void rnic_init_rcc(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned long rps_bitmask = 0;
#ifdef CONFIG_HISI_RFS_RPS_MATCH
	uint32_t devid;
#endif

	rps_bitmask = dev_ctx->rhc_cfg.level_cfg[0].rps_bitmask;
	dev_ctx->rcc.enable = dev_ctx->rhc_cfg.level_cfg[0].rcc_enable;
	dev_ctx->rcc.rps_bitmask = rps_bitmask;

#ifdef CONFIG_HISI_RFS_RPS_MATCH
	for (devid = 0; devid < RNIC_DEV_ID_DATA_MAX; devid++)
		rnic_update_rps_cpumask((uint8_t)devid, rps_bitmask);
#endif
}

#ifdef CONFIG_HISI_RFS_RPS_MATCH
/*
 * rnic_update_lb_cpumask - Update LB cpumask for specific netdevice.
 * @devid: id of netdevice.
 * @rps_bitmask: rps bitmask of specific netdevice.
 */
void rnic_update_lb_cpumask(uint8_t devid, unsigned long rps_bitmask)
{
	struct rnic_dev_priv_s *priv;
	struct rnic_lb_policy_s *policy;
	int cpu;

	priv = rnic_get_priv(devid);
	policy = &priv->lb_policy;

	policy->cpu_bitmask = 0;
	for (cpu = 0; (cpu < NR_CPUS) && (rps_bitmask > 0); cpu++) {
		rps_bitmask = rps_bitmask >> 1;
		set_bit(cpu, &policy->cpu_bitmask);
	}
	clear_bit(0, &policy->cpu_bitmask);
}

/*
 * rnic_update_rps_cpumask - Update rps cpumask for specific netdevice.
 * @devid: id of netdevice.
 * @rps_bitmask: rps bitmask of specific netdevice.
 */
void rnic_update_rps_cpumask(uint8_t devid, unsigned long rps_bitmask)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	struct rnic_dev_priv_s *priv = NULL;
	struct netdev_rx_queue *rxqueue = NULL;
	struct rps_map *map = NULL;
	unsigned int cpu = 0;

	if (rps_bitmask < RNIC_RPS_BITMASK_MIN ) {
		dev_ctx->qos_stats.rpsmap_invalid_num++;
		return;
	}

	cpumask_clear(dev_ctx->rcc.rps_cpumask);
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (test_bit((int)cpu, &rps_bitmask))
			cpumask_set_cpu(cpu, dev_ctx->rcc.rps_cpumask);
	}
	rnic_update_lb_cpumask(devid, rps_bitmask);

	priv = rnic_get_priv(devid);

	rxqueue = priv->netdev->_rx;
	if (unlikely(rxqueue == NULL)) {
		dev_ctx->qos_stats.rxqueue_null++;
		return;
	}

	preempt_disable();
	rcu_read_lock();
	map = rcu_dereference(rxqueue->rps_map);
	if (unlikely(map == NULL)) {
		dev_ctx->qos_stats.rxqueue_rpsmap_null++;
		rcu_read_unlock();
		preempt_enable();
		return;
	}

	cpumask_copy(&map->cpus_mask, dev_ctx->rcc.rps_cpumask);

	for (cpu = 0; (cpu < NR_CPUS) && (rps_bitmask > 0); cpu++)
		rps_bitmask = rps_bitmask >> 1;

	map->len = cpu - 1;
	rcu_read_unlock();
	preempt_enable();
}
#endif

/*
 * rnic_set_rcc_config - Set configuration of rcc.
 * @level: rcc config level
 */
void rnic_set_rcc_config(uint8_t level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	struct rnic_rhc_level_config_s *level_cfg = NULL;

	level_cfg = &dev_ctx->rhc_cfg.level_cfg[level];
	dev_ctx->rcc.enable = level_cfg->rcc_enable;
	dev_ctx->rcc.backlog_que_limit_len = level_cfg->backlog_que_limit_len;

#ifdef CONFIG_HISI_RFS_RPS_MATCH
	if (dev_ctx->rcc.rps_bitmask != level_cfg->rps_bitmask) {
		struct rnic_dev_priv_s *priv = NULL;
		uint8_t devid;
		dev_ctx->rcc.rps_bitmask = level_cfg->rps_bitmask;

		for (devid = 0; devid < RNIC_DEV_ID_DATA_MAX; devid++) {
			priv = rnic_get_priv(devid);
			if (priv->addr_family_mask != 0)
				rnic_update_rps_cpumask(devid, dev_ctx->rcc.rps_bitmask);
		}
	}
#endif
}

void rnic_update_level_rps_config(int level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned long rps_bitmask = 0;
	int cluster = 0;
	uint32_t cpu;
	uint8_t last_cpu;

	rps_bitmask = dev_ctx->rhc_cfg.level_cfg[level].rps_bitmask;

	for (cpu = 1; cpu < nr_cpu_ids; cpu++) {
		cluster = topology_physical_package_id(cpu);
		if (RNIC_VALID_CLUSTER(cluster)) {
			last_cpu = dev_ctx->cpufreq_req[cluster].last_cpu;
			if (test_bit(last_cpu, &rps_bitmask))
				rps_bitmask |= (unsigned long)(unsigned)(1 << cpu);
			else
				rps_bitmask &= ~(unsigned long)(unsigned)(1 << cpu);
		}
	}

	dev_ctx->rhc_cfg.level_cfg[level].rps_bitmask = (uint8_t)rps_bitmask;
}

void rnic_adapt_rhc_rps_config(void)
{
	int level;

	for (level = 0; level < RNIC_LB_MAX_LEVEL; level++) {
		rnic_update_level_rps_config(level);
	}
}

/*
 * rnic_set_rhc_config - Set configuration of rhc.
 * @config: rhc config
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_set_rhc_config(const struct rnic_rhc_config_s *config)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	int ret;

	dev_ctx->rhc_cfg.rhc_feature = config->rhc_feature;
	dev_ctx->qcc.enable = config->qcc_enable;
	dev_ctx->qcc.napi_wt_exp_enable = config->napi_wt_exp_enable;
	dev_ctx->qcc.ddr_req_enable = config->ddr_req_enable;
	dev_ctx->qcc.rps_mask_enable = config->rps_mask_enable;
	dev_ctx->qcc.ddr_mid_bd = config->ddr_mid_bd;
	dev_ctx->qcc.ddr_high_bd = config->ddr_high_bd;
	/* qcc_timeout 100ms unit to ms */
	dev_ctx->qcc.timeout = config->qcc_timeout * 100;
	dev_ctx->qcc.napi_poll_max = config->napi_poll_max;

	ret = memcpy_s(&dev_ctx->rhc_cfg.level_cfg[0],
		 sizeof(struct rnic_rhc_level_config_s) * RNIC_RHC_MAX_LEVEL,
		 &config->level_cfg[0],
		 sizeof(struct rnic_rhc_level_config_s) * RNIC_RHC_MAX_LEVEL);
	RNIC_MEMCPY_RET_CHECK(ret, sizeof(struct rnic_rhc_level_config_s) * RNIC_RHC_MAX_LEVEL,
			      sizeof(struct rnic_rhc_level_config_s) * RNIC_RHC_MAX_LEVEL);

	rnic_adapt_rhc_rps_config();
	if (dev_ctx->rhc_cfg.rhc_feature) {
#ifdef CONFIG_HISI_CPUFREQ_DT
		rnic_init_cpu_freq();
#endif
		rnic_init_rcc();
	}

	return 0;
}

/*
 * rnic_set_rhc_level - Set rhc config level.
 * @level: rhc level
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_set_rhc_level(uint8_t rhc_level)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	if (dev_ctx->rhc_cfg.rhc_feature) {
#ifdef CONFIG_HISI_CPUFREQ_DT
		rnic_update_cpu_freq(rhc_level);
#endif
#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
		rnic_update_ddr_req_pps(rhc_level);
#endif
		rnic_set_rcc_config(rhc_level);
		dev_ctx->qos_stats.cur_rhc_level = rhc_level;
	}
	return 0;
}


#if (defined(CONFIG_BALONG_ESPE))
/*
 * rnic_config_espe_wan_info - Set wan information for specific netdevice.
 * @priv: private info of netdeive
 *
 * Returns 0 on success, negative on failure.
 */
STATIC int rnic_config_espe_wan_info(struct rnic_dev_priv_s *priv)
{
	struct wan_dev_info_s wan_info;

	memset_s(&wan_info, sizeof(wan_info), 0, sizeof(wan_info));

	wan_info.dev = priv->netdev;
	wan_info.v4.info.pdu_session_id = priv->v4_info.pdu_session_id;
	wan_info.v4.info.fc_head = priv->v4_info.fc_head;
	wan_info.v4.info.modem_id = priv->v4_info.modem_id;
	wan_info.v4.userfield0 = priv->v4_info.iface_id;

	wan_info.v6.info.pdu_session_id = priv->v6_info.pdu_session_id;
	wan_info.v6.info.fc_head = priv->v6_info.fc_head;
	wan_info.v6.info.modem_id = priv->v6_info.modem_id;
	wan_info.v6.userfield0 = priv->v6_info.iface_id;

	return mdrv_wan_dev_info_register(&wan_info);
}

/*
 * rnic_add_espe_netdev - Associate netdevice to espe port.
 * @priv: private info of netdeive
 *
 * Returns 0 on success, negative on failure.
 */
STATIC int rnic_add_espe_netdev(struct rnic_dev_priv_s *priv)
{
	return mdrv_lan_add_netdev(priv->netdev, rnic_espe_xmit, NULL);
}

/*
 * rnic_rm_espe_netdev - Unassociate netdevice from espe port.
 * @priv: private info of netdeive
 *
 * Returns 0 on success, negative on failure.
 */
STATIC int rnic_rm_espe_netdev(struct rnic_dev_priv_s *priv)
{
	return mdrv_lan_rm_netdev(priv->netdev);
}

/*
 * rnic_init_espe_port - Set configration of espe port.
 * @port_config: port config
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_init_espe_port(const struct rnic_espe_port_config_s *port_config)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	struct rnic_dev_priv_s *priv = NULL;
	uint32_t devid;
	int ret;

	for (devid = 0; devid < RNIC_DEV_ID_BUTT; devid++) {
		priv = dev_ctx->priv[devid];
		ret = memcpy_s(&priv->v4_info, sizeof(struct rnic_ps_iface_info_s),
			       &port_config->info, sizeof(struct rnic_ps_iface_info_s));
		RNIC_MEMCPY_RET_CHECK(ret, sizeof(struct rnic_ps_iface_info_s),
				      sizeof(struct rnic_ps_iface_info_s));
		ret = memcpy_s(&priv->v6_info, sizeof(struct rnic_ps_iface_info_s),
			       &port_config->info, sizeof(struct rnic_ps_iface_info_s));
		RNIC_MEMCPY_RET_CHECK(ret, sizeof(struct rnic_ps_iface_info_s),
				      sizeof(struct rnic_ps_iface_info_s));
		if (rnic_config_espe_wan_info(priv))
			RNIC_LOGE("set espe wan info failed: %d.", devid);
	}

	return 0;
}

/*
 * rnic_set_espe_config - Set espe ip cfg.
 * @iface_config: iface config
 *
 * Returns 0 on success, negative on failure.
 */
void rnic_set_espe_config(struct rnic_dev_priv_s *priv,
			  const struct rnic_ps_iface_config_s *config)
{
	if (config->addr_family == RNIC_ETH_ADDR) {
		if (rnic_add_espe_netdev(priv))
			RNIC_LOGE("add espe netdev failed: %d.", priv->devid);
	} else {
		if (rnic_config_espe_wan_info(priv))
			RNIC_LOGE("set espe wan info failed: %d.", priv->devid);
	}
}

/*
 * rnic_clear_espe_config - Clear espe ip cfg.
 * @iface_config: iface config
 *
 * Returns 0 on success, negative on failure.
 */
void rnic_clear_espe_config(struct rnic_dev_priv_s *priv,
			    const struct rnic_ps_iface_config_s *config)
{
	if (config->addr_family == RNIC_ETH_ADDR) {
		if (rnic_rm_espe_netdev(priv))
			RNIC_LOGE("remove espe netdev failed: %d.", priv->devid);
	} else {
		if (rnic_config_espe_wan_info(priv))
			RNIC_LOGE("set espe wan info failed: %d.", priv->devid);

	}
}
#endif /* CONFIG_BALONG_ESPE */

/*
 * rnic_set_iface_addr_family_info - Set iface addr_family info.
 * @iface_config: iface config
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_set_iface_addr_family_info(struct rnic_dev_priv_s *priv,
				    const struct rnic_ps_iface_config_s *config)
{
	switch (config->addr_family) {
	case RNIC_IPV4_ADDR:
		priv->v4_info = config->info;
		priv->v4_data_tx_func = config->data_tx_func;
		break;
	case RNIC_IPV6_ADDR:
		priv->v6_info = config->info;
		priv->v6_data_tx_func = config->data_tx_func;
		break;
	case RNIC_ETH_ADDR:
		priv->eth_info = config->info;
		priv->eth_data_tx_func = config->data_tx_func;
		break;
	default:
		RNIC_LOGE("addr_family invalid: %d.", config->addr_family);
		return -EINVAL;
	}

	return 0;
}

/*
 * rnic_set_ps_iface_up - Set ps iface up.
 * @iface_config: iface config
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_set_ps_iface_up(const struct rnic_ps_iface_config_s *iface_config)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(iface_config->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", iface_config->devid);
		return -ENODEV;
	}

	if (rnic_set_iface_addr_family_info(priv, iface_config))
		return -EINVAL;

	set_bit(iface_config->addr_family, &priv->addr_family_mask);

#if (defined(CONFIG_BALONG_ESPE))
	rnic_set_espe_config(priv, iface_config);
#endif

	return 0;
}
/*
 * rnic_clear_iface_addr_family_info - clear iface addr_family info.
 * @iface_config: iface config
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_clear_iface_addr_family_info(struct rnic_dev_priv_s *priv,
				      const struct rnic_ps_iface_config_s *config)
{
	int ret;

	switch (config->addr_family) {
	case RNIC_IPV4_ADDR:
		ret = memcpy_s(&priv->v4_info, sizeof(struct rnic_ps_iface_info_s),
			       &config->info, sizeof(struct rnic_ps_iface_info_s));
		RNIC_MEMCPY_RET_CHECK(ret, sizeof(struct rnic_ps_iface_info_s),
				      sizeof(struct rnic_ps_iface_info_s));
		priv->v4_data_tx_func = NULL;
		break;
	case RNIC_IPV6_ADDR:
		ret = memcpy_s(&priv->v6_info, sizeof(struct rnic_ps_iface_info_s),
			       &config->info, sizeof(struct rnic_ps_iface_info_s));
		RNIC_MEMCPY_RET_CHECK(ret, sizeof(struct rnic_ps_iface_info_s),
				      sizeof(struct rnic_ps_iface_info_s));
		priv->v6_data_tx_func = NULL;
		break;
	case RNIC_ETH_ADDR:
		ret = memcpy_s(&priv->eth_info, sizeof(struct rnic_ps_iface_info_s),
			       &config->info, sizeof(struct rnic_ps_iface_info_s));
		RNIC_MEMCPY_RET_CHECK(ret, sizeof(struct rnic_ps_iface_info_s),
				      sizeof(struct rnic_ps_iface_info_s));
		priv->eth_data_tx_func = NULL;
		break;
	default:
		RNIC_LOGE("addr_family invalid: %d.", config->addr_family);
		return -EINVAL;
	}

	return 0;
}

/*
 * rnic_set_ps_iface_up - Set ps iface down.
 * @iface_config: iface config
 *
 * Returns 0 on success, negative on failure.
 */
int rnic_set_ps_iface_down(const struct rnic_ps_iface_config_s *config)
{
	struct rnic_dev_priv_s *priv = NULL;

	priv = rnic_get_priv(config->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", config->devid);
		return -ENODEV;
	}

	if (rnic_clear_iface_addr_family_info(priv, config) != 0) {
		return -EINVAL;
	}

	clear_bit(config->addr_family, &priv->addr_family_mask);

#if (defined(CONFIG_BALONG_ESPE))
	rnic_clear_espe_config(priv, config);
#endif

	return 0;
}

