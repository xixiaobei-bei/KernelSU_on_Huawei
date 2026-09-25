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
#include "rnic_policy_manage.h"
#include "rnic_dev.h"
#include "rnic_dev_debug.h"


/*****************************************************************************
 2. Global defintions
*****************************************************************************/


/*****************************************************************************
 3. Function defintions
*****************************************************************************/

/**
 * rnic_cpumasks_deinit - Optional function. Called when deinit load
 * balance cpumasks.
 * @policy: load balance policy addr
 *
 * This function is deinit load balance cpumasks, such as destory netdevice.
 *
 * Returns void.
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

/**
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
/**
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

	RNIC_LOGI("cpu %d, action 0x%lx.", cpu, action);

	if (unlikely(cpu >= nr_cpu_ids))
		return NOTIFY_OK;

	switch (action) {
	case CPU_ONLINE:
	case CPU_ONLINE_FROZEN:
		cpumask_set_cpu(cpu, policy->cpumask_curr_avail);

		/*
		 * This cpu is the expected load balance cpu,
		 * then set it to lb_cpumask_orig
		 */
		if (test_bit((int)cpu, &policy->cpu_bitmask) &&
		    policy->level_cfg[policy->cur_level].cpu_weight[cpu])
			cpumask_set_cpu(cpu, policy->cpumask_orig);

		priv->lb_stats[cpu].hotplug_online_num++;
		break;
	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		cpumask_clear_cpu((int)cpu, policy->cpumask_curr_avail);
		cpumask_clear_cpu((int)cpu, policy->cpumask_orig);
		priv->lb_stats[cpu].hotplug_down_num++;
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

/**
 * rnic_cpu_hotplug_init - Optional function. Called when cpu hot plug init.
 *
 * This function is called when cpu hot plug init, such as create netdevice.
 *
 * Returns void.
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

/**
 * rnic_cpu_hotplug_deinit - Optional function. Called when cpu hot plug deinit.
 *
 * This function is called when cpu hot plug deinit, such as destory netdevice.
 *
 * Returns void.
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
/**
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
	uint8_t devid;

	RNIC_LOGH("cpuid: %d.", cpu);

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL) {
			policy = &priv->lb_policy;
			cpumask_set_cpu(cpu, policy->cpumask_curr_avail);

			/*
			 * This cpu is the expected load balance cpu,
			 * then set it to lb_cpumask_orig
			 */
			if (test_bit((int)cpu, &policy->cpu_bitmask) &&
			    policy->level_cfg[policy->cur_level].cpu_weight[cpu])
				cpumask_set_cpu(cpu, policy->cpumask_orig);

			priv->lb_stats[cpu].hotplug_online_num++;
		}
	}

	return 0;
}

/**
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
	uint8_t devid;

	RNIC_LOGH("cpuid: %d.", cpu);

	/* only data netcard need care cpu hotplug */
	for (devid = 0; devid <= RNIC_DEV_ID_DATA_MAX; devid++) {
		priv = rnic_get_priv(devid);
		if (priv != NULL) {
			cpumask_clear_cpu((int)cpu,
					  priv->lb_policy.cpumask_curr_avail);
			cpumask_clear_cpu((int)cpu,
					  priv->lb_policy.cpumask_orig);
			priv->lb_stats[cpu].hotplug_down_num++;
		}
	}

	return 0;
}

/**
 * rnic_cpuhp_init - Optional function. Called when cpu hot plug init.
 *
 * This function is called when cpu hot plug init, such as create netdevice.
 *
 * Returns void.
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

	ret = cpuhp_setup_state_nocalls(CPUHP_AP_ONLINE_DYN,
					"rnic:online",
					rnic_cpuhp_online,
					rnic_cpuhp_perpare_down);
	if (ret < 0) {
		RNIC_LOGE("cpuhp_setup_state_nocalls fail, ret: %d.", ret);
	} else {
		/*
		 * Care cpuhp_online_state, Currently it equel
		 * CPUHP_AP_ONLINE_DYN, so just assigned ret to
		 * online_state; When it equel a static STATE,
		 * please assigned the static STATE to online_state;
		 */
		dev_ctx->online_state = (enum cpuhp_state)ret;
	}
}

/**
 * rnic_cpuhp_deinit - Optional function. Called when cpu hot plug deinit.
 *
 * This function is called when cpu hot plug deinit, such as destory netdevice.
 *
 * Returns void.
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

/**
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

	if (!policy->enable) {
		RNIC_LOGE("load balance is disabled: devid is %d.", devid);
		return -EFAULT;
	}

	policy->cur_level = (level >= RNIC_LB_MAX_LEVEL) ? 0 : level;
	memcpy(&policy->weight_orig[0], /* unsafe_function_ignore: memcpy */
	       &policy->level_cfg[policy->cur_level].cpu_weight[0],
	       sizeof(uint8_t) * NR_CPUS);
	memcpy(&policy->weight_remaind[0], /* unsafe_function_ignore: memcpy */
	       &policy->weight_orig[0],
	       sizeof(uint8_t) * NR_CPUS);

	cpumask_clear(policy->cpumask_orig);
	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		if (policy->weight_orig[cpu] &&
		    test_bit((int)cpu, &policy->cpu_bitmask) &&
		    cpumask_test_cpu((int)cpu, policy->cpumask_curr_avail))
			cpumask_set_cpu(cpu, policy->cpumask_orig);
	}

	cpumask_copy(policy->cpumask_candidacy, policy->cpumask_orig);

	return 0;
}

/**
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
	int cpu;

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

	policy->enable = (lb_config->enable == 0) ? false : true;
	policy->cpu_bitmask = lb_config->cpu_bitmask;

	for (cpu = 0; (cpu < NR_CPUS && cpu < RNIC_LB_MAX_CPUS); cpu++) {
		if (test_bit(cpu, &policy->cpu_bitmask) &&
		    cpumask_test_cpu(cpu, policy->cpumask_curr_avail))
			cpumask_set_cpu((unsigned int)cpu,
					policy->cpumask_orig);

		policy->weight_orig[cpu] =
			lb_config->level_cfg[0].cpu_weight[cpu]; //lint !e661 !e662
	}

	memcpy(&policy->level_cfg[0], /* unsafe_function_ignore: memcpy */
	       &lb_config->level_cfg[0],
	       sizeof(policy->level_cfg[0]) * RNIC_LB_MAX_LEVEL);

	return 0;
}

/**
 * rnic_select_cpu_candidacy - Optional function. Called when load balance
 * target cpu select Algorithm.
 * @policy: load balance policy addr
 *
 * This function is select target cpu Algorithm.
 *
 * Returns void.
 */
void rnic_select_cpu_candidacy(struct rnic_lb_policy_s *policy)
{
	unsigned int cpu;
	int napi_cpu = atomic_read(&policy->target_cpu);

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

	atomic_set(&policy->target_cpu, napi_cpu);

	if (policy->weight_remaind[napi_cpu] == 0)
		cpumask_clear_cpu(napi_cpu, policy->cpumask_candidacy);
}



