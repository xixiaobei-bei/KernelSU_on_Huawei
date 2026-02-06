/*
 * lpcpu_cpuddr_freq_link.c
 *
 * cpuddr freq link module
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/cpufreq.h>
#include <linux/of_platform.h>
#include <linux/pm_opp.h>
#include <linux/pm_qos.h>
#include <linux/cpumask.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/slab.h>

static int set_cpu_ddr_link_level(unsigned int ddr_lvl)
{
	unsigned int *msg = NULL;
	int rproc_id = HISI_ACPU_LPM3_MBX_1;
	int ret;

	msg = kzalloc(sizeof(u32) * MAX_MAIL_SIZE, GFP_KERNEL);
	if (msg == NULL) {
		pr_err("%s: cannot allocate msg space\n", __func__);
		return -ENOMEM;
	}
	msg[0] = IPC_CMD(OBJ_AP, OBJ_LITTLE_CLUSTER, CMD_SETTING, TYPE_DNLIMIT);
	msg[1] = ddr_lvl;
	ret = RPROC_ASYNC_SEND((rproc_id_t)rproc_id, (mbox_msg_t *)msg,
			       MAX_MAIL_SIZE);
	if (ret != 0)
		pr_err("%s, line %d, send error\n", __func__, __LINE__);

	kfree(msg);
	msg = NULL;

	return ret;
}

static int cpuddr_freq_link_notify(struct notifier_block *nb,
				   unsigned long val, void *v)
{
	int ret;

	ret = set_cpu_ddr_link_level((unsigned int)val);
	if (ret != 0)
		return NOTIFY_BAD;

	return NOTIFY_OK;
}

static struct notifier_block cpuddr_freq_link_notifier = {
	.notifier_call = cpuddr_freq_link_notify,
};

static __init int hisi_cpuddr_freq_link_init(void)
{
	int ret;

	ret = pm_qos_add_notifier(PM_QOS_FREQ_LINK_LEVEL, &cpuddr_freq_link_notifier);
	if (ret != 0)
		pr_err("cpuddr_freq_link init fail%d\n", ret);

	return ret;
}
module_init(hisi_cpuddr_freq_link_init);
