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
#include <linux/atomic.h>
#include <linux/cpu.h>
#include <linux/notifier.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 10, 0))
#include <linux/cpuhotplug.h>
#endif
#include <linux/topology.h>
#include "securec.h"
#include "rnic_dev.h"
#include "rnic_dev_config.h"
#include "rnic_dev_debug.h"


/*
 * rnic_cpumasks_deinit - Optional function. Called when deinit load
 * balance cpumasks.
 * @policy: load balance policy addr
 *
 * This function is deinit load balance cpumasks, such as destory netdevice.
 */
void rnic_cpumasks_deinit(struct rnic_lb_policy_s *policy)
{
	if (policy->cap_valid) {
		policy->cap_valid = false;
		free_cpumask_var(policy->cpumask_curr_avail);
		free_cpumask_var(policy->cpumask_orig);
		free_cpumask_var(policy->cpumask_candidacy);
	}
}

/*
 * rnic_cpumasks_init - Optional function. Called when init load
 * balance cpumasks.
 * @policy: load balance policy addr
 *
 * This function is init load balance cpumasks, such as create netdevice.
 *
 * Returns negative errno on error, or zero on success.
 */
/*lint -save -e801*/
int rnic_cpumasks_init(struct rnic_lb_policy_s *policy)
{
	if (!alloc_cpumask_var(&policy->cpumask_curr_avail, GFP_KERNEL))
		goto err_alloc_cpumask_avail;

	if (!alloc_cpumask_var(&policy->cpumask_orig, GFP_KERNEL))
		goto err_alloc_cpumask_orig;

	if (!alloc_cpumask_var(&policy->cpumask_candidacy, GFP_KERNEL))
		goto err_alloc_cpumask_candidacy;

	cpumask_copy(policy->cpumask_curr_avail, cpu_online_mask);
	cpumask_clear(policy->cpumask_orig);
	cpumask_clear(policy->cpumask_candidacy);

	policy->cap_valid = true;

	return 0;

err_alloc_cpumask_candidacy:
	free_cpumask_var(policy->cpumask_orig);
err_alloc_cpumask_orig:
	free_cpumask_var(policy->cpumask_curr_avail);
err_alloc_cpumask_avail:
	policy->cap_valid = false;

	return -ENOMEM;
}
/*lint -restore +e801*/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
/*
 * rnic_cpu_hotplug_notify - Optional function. Called when cpu hot plug
 * notify cb.
 * @nfb: notifier block
 * @action: cpu status
 * @hcpu: hotplug cpu
 *
 * This function is called when cpu status change, such as cpu online.
 *
 * Returns zero on success.
 */
STATIC int rnic_cpu_hotplug_notify(struct notifier_block *nfb,
				   unsigned long action, void *hcpu)
{
	struct rnic_dev_priv_s *priv = container_of(nfb, struct rnic_dev_priv_s,
						    cpu_hotplug_notifier);
	struct rnic_lb_policy_s *policy = &priv->lb_policy;
	unsigned int cpu = (unsigned int)(uintptr_t)hcpu;
	unsigned long flags;

	RNIC_LOGI("cpu %d, action 0x%lx.", cpu, action);

	if (unlikely(cpu >= nr_cpu_ids))
		return NOTIFY_OK;

	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		spin_lock_irqsave(&policy->lb_lock, flags);
		cpumask_set_cpu(cpu, policy->cpumask_curr_avail);

		/*
		 * This cpu is the expected load balance cpu,
		 * then set it to lb_cpumask_orig
		 */
		if (test_bit((int)cpu, &policy->cpu_bitmask) &&
		    policy->level_cfg[policy->cur_level].cpu_weight[cpu])
			cpumask_set_cpu(cpu, policy->cpumask_orig);
		spin_unlock_irqrestore(&policy->lb_lock, flags);

		priv->lb_stats[cpu].hotplug_online_num++;
		break;
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		spin_lock_irqsave(&policy->lb_lock, flags);
		cpumask_clear_cpu((int)cpu, policy->cpumask_curr_avail);
		cpumask_clear_cpu((int)cpu, policy->cpumask_orig);
		spin_unlock_irqrestore(&policy->lb_lock, flags);
		priv->lb_stats[cpu].hotplug_down_num++;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

/*
 * rnic_cpu_hotplug_init - Optional function. Called when cpu hot plug init.
 *
 * This function is called when cpu hot plug init, such as create netdevice.
 */
void rnic_cpu_hotplug_init(void)
{
	struct rnic_dev_priv_s *priv = NULL;
	struct rnic_lb_policy_s *policy = NULL;
	struct notifier_block *notifier = NULL;
	uint8_t devid;

	RNIC_LOGH("enter");

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL) {
			policy = &priv->lb_policy;
			notifier = &priv->cpu_hotplug_notifier;
			if (policy->cap_valid) {
				notifier->notifier_call = rnic_cpu_hotplug_notify;
				register_cpu_notifier(notifier);
			} else {
				RNIC_LOGE("exist invalide cpumasks, devid: %d.",
					  devid);
				notifier->notifier_call = NULL;
			}
		}
	}
}

/*
 * rnic_cpu_hotplug_deinit - Optional function. Called when cpu hot plug deinit.
 *
 * This function is called when cpu hot plug deinit, such as destory netdevice.
 */
void rnic_cpu_hotplug_deinit(void)
{
	struct rnic_dev_priv_s *priv = NULL;
	uint8_t devid;

	RNIC_LOGH("enter");

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL &&
		    priv->cpu_hotplug_notifier.notifier_call != NULL)
			unregister_cpu_notifier(&priv->cpu_hotplug_notifier);
	}
}

#else
/*
 * rnic_cpuhp_online - Optional function. Called when cpu step into
 * RNIC_CPUHP_STATE from low state.
 * @cpu: cpu id which state change
 *
 * This function is called when cpu step into RNIC_CPUHP_STATE from
 * low state.
 *
 * Returns zero on success.
 */
STATIC int rnic_cpuhp_online(unsigned int cpu)
{
	struct rnic_dev_priv_s *priv = NULL;
	struct rnic_lb_policy_s *policy = NULL;
	unsigned long flags;
	uint8_t devid;

	RNIC_LOGH("cpuid: %d.", cpu);

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL) {
			policy = &priv->lb_policy;

			spin_lock_irqsave(&policy->lb_lock, flags);
			cpumask_set_cpu(cpu, policy->cpumask_curr_avail);

			/*
			 * This cpu is the expected load balance cpu,
			 * then set it to lb_cpumask_orig
			 */
			if (test_bit((int)cpu, &policy->cpu_bitmask) &&
			    policy->level_cfg[policy->cur_level].cpu_weight[cpu])
				cpumask_set_cpu(cpu, policy->cpumask_orig);
			spin_unlock_irqrestore(&policy->lb_lock, flags);

			priv->lb_stats[cpu].hotplug_online_num++;
		}
	}

	return 0;
}

/*
 * rnic_cpuhp_perpare_down - Optional function. Called when cpu step into
 * RNIC_CPUHP_STATE from high state.
 * @cpu: cpu id which state change
 *
 * This function is called when cpu step into RNIC_CPUHP_STATE from
 * high state.
 *
 * Returns zero on success.
 */
STATIC int rnic_cpuhp_perpare_down(unsigned int cpu)
{
	struct rnic_dev_priv_s *priv = NULL;
	unsigned long flags;
	uint8_t devid;

	RNIC_LOGH("cpuid: %d.", cpu);

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL) {
			spin_lock_irqsave(&priv->lb_policy.lb_lock, flags);
			cpumask_clear_cpu((int)cpu,
					  priv->lb_policy.cpumask_curr_avail);
			cpumask_clear_cpu((int)cpu,
					  priv->lb_policy.cpumask_orig);
			spin_unlock_irqrestore(&priv->lb_policy.lb_lock, flags);

			priv->lb_stats[cpu].hotplug_down_num++;
		}
	}

	return 0;
}

/*
 * rnic_cpuhp_init - Optional function. Called when cpu hot plug init.
 *
 * This function is called when cpu hot plug init, such as create netdevice.
 */
void rnic_cpuhp_init(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	struct rnic_dev_priv_s *priv = NULL;
	int ret;
	uint8_t devid;

	RNIC_LOGH("enter");
	dev_ctx->online_state = CPUHP_INVALID;

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL) {
			if (!priv->lb_policy.cap_valid) {
				RNIC_LOGE("exist invalid cpumasks, devid: %d.",
					  devid);
				return;
			}
		}
	}

	/*
	 * Care cpuhp_online_state, Currently it equel CPUHP_AP_ONLINE_DYN,
	 * so just assigned ret to online_state; When it equal a static STATE,
	 * please assigned the static STATE to online_state.
	 */
	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					"rnic:online",
					rnic_cpuhp_online,
					rnic_cpuhp_perpare_down);
	if (ret < 0)
		RNIC_LOGE("cpuhp_setup_state_nocalls fail, ret: %d.", ret);
	else
		dev_ctx->online_state = (enum cpuhp_state)ret;
}

/*
 * rnic_cpuhp_deinit - Optional function. Called when cpu hot plug deinit.
 *
 * This function is called when cpu hot plug deinit, such as destory netdevice.
 */
void rnic_cpuhp_deinit(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	RNIC_LOGH("enter");

	if (dev_ctx->online_state == CPUHP_INVALID)
		RNIC_LOGE("invalid online state");
	else
		cpuhp_remove_state(dev_ctx->online_state);
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0) */

void rnic_cluster_init(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	int cluster_id = 0;
	int cpu;

	for_each_online_cpu(cpu) {//lint !e713
		cluster_id = topology_physical_package_id(cpu);
		if (RNIC_VALID_CLUSTER(cluster_id)) {
			dev_ctx->cpufreq_req[cluster_id].last_cpu = (uint8_t)cpu;
			if (cluster_id != 0)
				set_bit(cpu, &dev_ctx->cpu_bitmask_mb);
		}
	}
	if (RNIC_VALID_CLUSTER(cluster_id))
		dev_ctx->act_cluster_num = (uint16_t)(cluster_id + 1);
}

/*
 * rnic_select_cpu_candidacy - Optional function. Called when load balance
 * target cpu select Algorithm.
 * @policy: load balance policy addr
 *
 * This function is select target cpu Algorithm.
 */
void rnic_select_cpu_candidacy(struct rnic_lb_policy_s *policy)
{
	unsigned long flags;
	unsigned int cpu;
	int napi_cpu = atomic_read(&policy->napi_cpu);

	spin_lock_irqsave(&policy->lb_lock, flags);

	if (cpumask_weight(policy->cpumask_candidacy) > 0) {
		napi_cpu = (int)cpumask_next(napi_cpu, policy->cpumask_candidacy);
		if (napi_cpu >= nr_cpu_ids)
			napi_cpu = (int)cpumask_first(policy->cpumask_candidacy);
	} else {
		/* restore lb_cpumask_candidacy, and lb_weight_remaind */
		for (cpu = 0; cpu < NR_CPUS; cpu++)
			policy->weight_remaind[cpu] = policy->weight_orig[cpu];

		cpumask_copy(policy->cpumask_candidacy, policy->cpumask_orig);
		napi_cpu = (int)cpumask_first(policy->cpumask_candidacy);
	}

	if (napi_cpu >= nr_cpu_ids)
		napi_cpu = 0;

	if (policy->weight_remaind[napi_cpu] > 0)
		policy->weight_remaind[napi_cpu]--;

	atomic_set(&policy->napi_cpu, napi_cpu);

	if (policy->weight_remaind[napi_cpu] == 0)
		cpumask_clear_cpu(napi_cpu, policy->cpumask_candidacy);

	spin_unlock_irqrestore(&policy->lb_lock, flags);
}

/*
 * rnic_set_lb_level - Optional function. Called when set load
 * balance level.
 * @devid: device id
 * @level: load balance level
 *
 * This function is set load balance level.
 *
 * Returns negative errno on error, or zero on success.
 */
int rnic_set_lb_level(uint8_t devid, uint8_t level)
{
	struct rnic_dev_priv_s *priv = NULL;
	struct rnic_lb_policy_s *policy = NULL;
	unsigned int cpu;
	unsigned long flags;

	priv = rnic_get_priv(devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", devid);
		return -ENODEV;
	}

	policy = &priv->lb_policy;
	if (!policy->cap_valid) {
		RNIC_LOGE("load balance cap is false: devid is %d.", devid);
		return -EPERM;
	}

	if (!policy->lb_enable)
		return -EFAULT;

	policy->cur_level = level >= RNIC_LB_MAX_LEVEL ? 0 : level;
	memcpy_s(&policy->weight_orig[0], NR_CPUS,
		 &policy->level_cfg[policy->cur_level].cpu_weight[0], NR_CPUS);
	memcpy_s(&policy->weight_remaind[0], NR_CPUS,
		 &policy->weight_orig[0], NR_CPUS);

	spin_lock_irqsave(&policy->lb_lock, flags);
	cpumask_clear(policy->cpumask_orig);
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (policy->weight_orig[cpu] &&
		    test_bit((int)cpu, &policy->cpu_bitmask) &&
		    cpumask_test_cpu((int)cpu, policy->cpumask_curr_avail))
			cpumask_set_cpu(cpu, policy->cpumask_orig);
	}
	cpumask_copy(policy->cpumask_candidacy, policy->cpumask_orig);
	spin_unlock_irqrestore(&policy->lb_lock, flags);

	return 0;
}

void rnic_set_lb_cpu_cfg(struct rnic_lb_policy_s *policy)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	int cluster_id = 0;
	int cpu, level;
	uint8_t cluster_last_cpu;

	for (cpu = 0; (cpu < nr_cpu_ids && cpu < RNIC_LB_MAX_CPUS); cpu++) {
		if (test_bit(cpu, &policy->cpu_bitmask) &&
		    cpumask_test_cpu(cpu, policy->cpumask_curr_avail))
			cpumask_set_cpu((unsigned int)cpu, policy->cpumask_orig);

		cluster_id = topology_physical_package_id(cpu);
		if (RNIC_VALID_CLUSTER(cluster_id)) {
			cluster_last_cpu = dev_ctx->cpufreq_req[cluster_id].last_cpu;

			for (level = 0; level < RNIC_LB_MAX_LEVEL; level++) {
				policy->level_cfg[level].cpu_weight[cpu] =
				policy->level_cfg[level].cpu_weight[cluster_last_cpu];
			}
		}
	}
}

/*
 * rnic_set_lb_config - Optional function. Called when set load balance config.
 * @lb_config: load balance config
 *
 * This function is set load balance config.
 *
 * Returns negative errno on error, or zero on success.
 */
int rnic_set_lb_config(const struct rnic_lb_config_s *lb_config)
{
	struct rnic_dev_priv_s *priv = NULL;
	struct rnic_lb_policy_s *policy = NULL;

	priv = rnic_get_priv(lb_config->devid);
	if (unlikely(priv == NULL)) {
		RNIC_LOGE("device not found: devid is %d.", lb_config->devid);
		return -ENODEV;
	}

	policy = &priv->lb_policy;
	if (!policy->cap_valid) {
		RNIC_LOGE("load balance cap is false: devid is %d.",
			  lb_config->devid);
		return -EFAULT;
	}

	policy->lb_enable = (lb_config->lb_enable == 0) ? false : true;
	policy->cpu_bitmask = lb_config->cpu_bitmask;

	memcpy_s(&policy->level_cfg[0],
		 sizeof(struct rnic_lb_level_config_s) * RNIC_LB_MAX_LEVEL,
		 &lb_config->level_cfg[0],
		 sizeof(struct rnic_lb_level_config_s) * RNIC_LB_MAX_LEVEL);

	rnic_set_lb_cpu_cfg(policy);

	/* init level to 0, and select the first napi cpu */
	rnic_set_lb_level(lb_config->devid, 0);
	rnic_select_cpu_candidacy(policy);

	return 0;
}

unsigned int rnic_napi_exp_weight(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned int ret = 0;

	if (dev_ctx->qcc.napi_wt_exp_enable &&
		priv->napi.weight < RNIC_NAPI_MAX_WT) {
		priv->napi.weight = RNIC_NAPI_MAX_WT;
		dev_ctx->qos_stats.napi_wt_exp_num++;
		ret = 1;
	}

	return ret;
}

STATIC void rnic_napi_rest_weight(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	if (dev_ctx->qcc.napi_wt_exp_enable &&
	    priv->napi.weight != priv->napi_weight) {
		priv->napi.weight = priv->napi_weight;
		dev_ctx->qos_stats.napi_wt_rst_num++;
	}
}

#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
unsigned int rnic_req_ddr_bd(int ddr_bd)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned int ret = 0;

	if (dev_ctx->qcc.ddr_req_enable &&
	    dev_ctx->qcc.ddr_req_bd_qcc != ddr_bd) {
		rnic_update_ddr_req_qcc(ddr_bd);
		dev_ctx->qcc.ddr_req_bd_qcc = ddr_bd;
		dev_ctx->qos_stats.ddr_req_mid_num++;
		ret = 1;
	}

	return ret;
}

STATIC void rnic_reset_ddr_bd(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	if (dev_ctx->qcc.ddr_req_enable &&
	    dev_ctx->qcc.ddr_req_bd_qcc > 0) {
		rnic_update_ddr_req_qcc(0);
		dev_ctx->qcc.ddr_req_bd_qcc = 0;
		dev_ctx->qos_stats.ddr_req_rst_num++;

		cancel_delayed_work(&dev_ctx->qos_rst_work);
	}
}
#endif

#ifdef CONFIG_HISI_RFS_RPS_MATCH
unsigned int rnic_upgrade_rps_cpumask(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned int ret = 0;

	if (dev_ctx->qcc.rps_mask_enable &&
	    dev_ctx->rcc.rps_bitmask != dev_ctx->cpu_bitmask_mb) {
		dev_ctx->qcc.rps_bitmask = dev_ctx->cpu_bitmask_mb;
		rnic_update_rps_cpumask(priv->devid, dev_ctx->qcc.rps_bitmask);
		dev_ctx->qos_stats.rps_mask_mb_num++;
		ret = 1;
	}

	return ret;
}

STATIC void rnic_rest_rps_cpumask(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	if (dev_ctx->qcc.rps_mask_enable &&
	    dev_ctx->qcc.rps_bitmask != dev_ctx->rcc.rps_bitmask) {
		dev_ctx->qcc.rps_bitmask = dev_ctx->rcc.rps_bitmask;
		rnic_update_rps_cpumask(priv->devid, dev_ctx->rcc.rps_bitmask);
		dev_ctx->qos_stats.rps_mask_rst_num++;
	}
}
#endif

/*
 * rnic_queue_low_load_ctrl - queue low load, reset control config.
 * @priv: private info of netdevice
 */
STATIC void rnic_queue_low_load_ctrl(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

	dev_ctx->qcc.queue_load = RNIC_LOW;

	if (delayed_work_pending(&dev_ctx->qos_rst_work))
		return;

	rnic_napi_rest_weight(priv);
#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
	rnic_reset_ddr_bd();
#endif
#ifdef CONFIG_HISI_RFS_RPS_MATCH
	rnic_rest_rps_cpumask(priv);
#endif

}

/*
 * rnic_queue_mid_load_ctrl - queue middle load control.
 * @priv: private info of netdevice
 */
STATIC void rnic_queue_mid_load_ctrl(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned int delay = dev_ctx->qcc.timeout;
	unsigned int reset = 0;

	dev_ctx->qcc.queue_load = RNIC_MID;

	reset |= rnic_napi_exp_weight(priv);
#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
	reset |= rnic_req_ddr_bd(dev_ctx->qcc.ddr_mid_bd);
#endif
#ifdef CONFIG_HISI_RFS_RPS_MATCH
	reset |= rnic_upgrade_rps_cpumask(priv);
#endif

	if (reset && (!delayed_work_pending(&dev_ctx->qos_rst_work)))
		schedule_delayed_work(&dev_ctx->qos_rst_work,
				      msecs_to_jiffies(delay));
}

/*
 * rnic_queue_high_load_ctrl - queue hight load control.
 * @priv: private info of netdevice
 */
STATIC void rnic_queue_high_load_ctrl(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	unsigned int delay = dev_ctx->qcc.timeout;
	unsigned int reset = 0;

	dev_ctx->qcc.queue_load = RNIC_HIGH;

	reset |= rnic_napi_exp_weight(priv);
#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
	reset |= rnic_req_ddr_bd(dev_ctx->qcc.ddr_high_bd);
#endif
#ifdef CONFIG_HISI_RFS_RPS_MATCH
	reset |= rnic_upgrade_rps_cpumask(priv);
#endif

	if (reset && (!delayed_work_pending(&dev_ctx->qos_rst_work)))
		schedule_delayed_work(&dev_ctx->qos_rst_work,
				      msecs_to_jiffies(delay));
}

/*
 * rinc_queue_cong_ctrl - napi queue congestion control.
 * @priv: private info of netdevice
 */
void rinc_queue_cong_ctrl(struct rnic_dev_priv_s *priv)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();
	uint32_t que_limit_len = priv->napi_queue_length;
	uint32_t total_que_len;

	total_que_len = skb_queue_len(&priv->input_queue) + \
			skb_queue_len(&priv->napi_queue) + \
			skb_queue_len(&priv->process_queue) + \
			skb_queue_len(&priv->rps_pend_queue);

	/* 队列总长度大于1/2,属于高档位;在[1/4, 1/2],属于中档位;其余为低档位 */
	if (total_que_len < (que_limit_len >> 2)) {
		if (dev_ctx->qcc.queue_load != RNIC_LOW)
			rnic_queue_low_load_ctrl(priv);
		dev_ctx->qos_stats.queue_low_load_num++;
	} else if (total_que_len > (que_limit_len >> 1)) {
		if (dev_ctx->qcc.queue_load != RNIC_HIGH)
			rnic_queue_high_load_ctrl(priv);
		dev_ctx->qos_stats.queue_high_load_num++;
	} else {
		if (dev_ctx->qcc.queue_load != RNIC_MID)
			rnic_queue_mid_load_ctrl(priv);
		dev_ctx->qos_stats.queue_mid_load_num++;
	}
}

/*
 * rnic_qos_rst_work_fn - queue congestion control reset work callback.
 * @work: work context
 */
void rnic_qos_rst_work_fn(struct work_struct *work)
{
	struct rnic_dev_priv_s *priv = NULL;
	uint32_t devid;

	for (devid = 0; devid < RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv((uint8_t)devid);
		if (priv->addr_family_mask != 0)
			rnic_queue_low_load_ctrl(priv);
	}
}

/*
 * rnic_pm_init - power management related policy init.
 */
void rnic_pm_init(void)
{
	struct rnic_dev_context_s *dev_ctx = RNIC_DEV_CTX();

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
	rnic_cpu_hotplug_init();
#else
	rnic_cpuhp_init();
#endif
	rnic_cluster_init();

#ifdef CONFIG_DEVFREQ_GOV_PM_QOS
	rnic_add_ddr_req(&dev_ctx->pm_qos_req_pps);
	rnic_add_ddr_req(&dev_ctx->pm_qos_req_qcc);
#endif
	INIT_DELAYED_WORK(&dev_ctx->qos_rst_work, rnic_qos_rst_work_fn);

}

