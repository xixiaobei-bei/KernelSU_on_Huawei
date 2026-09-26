/*
 * hwqos_fork.c
 *
 * Qos schedule implementation
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/atomic.h>
#include <linux/sched.h>
#include <linux/slab.h>

struct set_qos g_init_qos = {
	.dynamic_qos = ATOMIC_INIT(VALUE_QOS_NORMAL),
	.usage = ATOMIC_INIT(1),
};

/*
 * Notes: There is no sleepable task in init_task_qos_info, for it is called
 * between get_cpu() and put_cpu(), which will preempt_disable()
 */
void init_task_qos_info(struct task_struct *p)
{
	if (unlikely(!p))
		return;
	p->proc_qos = &g_init_qos;
	atomic_set(&p->thread_qos.dynamic_qos, VALUE_QOS_NORMAL);
	atomic_set(&p->thread_qos.usage, 0);
	atomic_set(&p->trans_flags, 0);
	p->trans_allowed = NULL;
#ifdef CONFIG_HUAWEI_SCHED_VIP
	p->vip_params.trans_flag = false;
	p->vip_params.value = 0;
#endif
#ifdef CONFIG_SCHED_HISI_TASK_MIN_UTIL
	p->min_util_params.trans_flag = false;
	p->min_util_params.value = 0;
#endif
}

void release_task_qos_info(struct task_struct *p)
{
	if (unlikely(!p))
		return;
	if ((p->exit_signal >= 0) &&
		(p->proc_qos) && (p->proc_qos != &g_init_qos))
		kfree(p->proc_qos);
	p->proc_qos = NULL;
	if (unlikely(p->trans_allowed)) {
		memset(p->trans_allowed,
			0,
			sizeof(*p->trans_allowed));
		p->trans_allowed = NULL;
	}
}
