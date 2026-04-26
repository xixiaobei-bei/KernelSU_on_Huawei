/*
 * cpufreq_get_volt.c
 *
 * cpufreq get volt module
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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/types.h>
#include <linux/string.h>
#include <global_ddr_map.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/compiler.h>
#include <linux/arm-smccc.h>
#include <asm/compiler.h>
#include <linux/proc_fs.h>
#include <bl31_smc.h>
#include <linux/rdr_lpm3.h>
#include <securec.h>

#define AVS_VOLT_MAX_BYTE				192U
#define HPM_VOLT_MAX_BYTE				102U
#define DIEID_MAX_BYTE					20U

#ifdef CONFIG_LP_ENABLE_HPM_DATA_COLLECT
#define VOLT_SIZE	(AVS_VOLT_MAX_BYTE + HPM_VOLT_MAX_BYTE + DIEID_MAX_BYTE + 2U)
#else
#define VOLT_SIZE	(AVS_VOLT_MAX_BYTE)
#endif

struct tag_cpu_volt_data {
	phys_addr_t phy_addr;
	unsigned char *virt_addr;
	/* mutex lock for cpu_volt-data */
	struct mutex cpu_mutex;
};

static struct tag_cpu_volt_data g_cpu_volt_data;

static int get_volt_show(struct seq_file *m, void *v)
{
	struct arm_smccc_res res = {0};
	int i;
#ifdef CONFIG_HISI_LPMCU_BB
	u32 rdr_lpm3_buf_len;
	char *lpmcu_rdr_addr = NULL;
	int ret;
#endif
	mutex_lock(&g_cpu_volt_data.cpu_mutex);

	arm_smccc_1_1_smc(HISI_GET_CPU_VOLT_FID_VALUE,
			  g_cpu_volt_data.phy_addr, VOLT_SIZE, &res);
	if (res.a0 != 0) {
		(void)seq_printf(m, "get val failed\n");
		mutex_unlock(&g_cpu_volt_data.cpu_mutex);
		return -EAGAIN;
	}

	for (i = 0; i < VOLT_SIZE; i++) {
#ifdef CONFIG_LP_ENABLE_HPM_DATA_COLLECT
		if (i == AVS_VOLT_MAX_BYTE) {
			seq_printf(m, "\n hpm_volt:\n");
		} else if (i == (AVS_VOLT_MAX_BYTE + HPM_VOLT_MAX_BYTE)) {
			i += 2;
			seq_printf(m, "\n die_id:\n");
		}
#endif
		seq_printf(m, "0x%-2x ", g_cpu_volt_data.virt_addr[i]);
		if (i % 16 == 15)
			seq_printf(m, "\n");
	}

#ifdef CONFIG_HISI_LPMCU_BB
	rdr_lpm3_buf_len = rdr_lpm3_buf_len_get();
	if (rdr_lpm3_buf_len < AVS_VOLT_MAX_BYTE) {
		mutex_unlock(&g_cpu_volt_data.cpu_mutex);
		return -ENOSPC;
	}
	lpmcu_rdr_addr = lpmcu_rdr_ddr_addr_get();
	ret = memcpy_s((void *)(lpmcu_rdr_addr + rdr_lpm3_buf_len -
				AVS_VOLT_MAX_BYTE),
		       (size_t)AVS_VOLT_MAX_BYTE, g_cpu_volt_data.virt_addr,
		       (size_t)AVS_VOLT_MAX_BYTE);
	if (ret != EOK) {
		mutex_unlock(&g_cpu_volt_data.cpu_mutex);
		return -ENOMEM;
	}
#endif

	mutex_unlock(&g_cpu_volt_data.cpu_mutex);

	return 0;
}

static int get_volt_open(struct inode *inode, struct file *file)
{
	return single_open(file, get_volt_show, NULL);
}

static const struct file_operations get_volt_operations = {
	.open = get_volt_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init cpufreq_get_volt_init(void)
{
	int ret;
	struct device_node *np = NULL;
	u32 data[2] = {0};

	phys_addr_t bl31_smem_base =
		HISI_SUB_RESERVED_BL31_SHARE_MEM_PHYMEM_BASE;
	np = of_find_compatible_node(NULL, NULL, "hisilicon, get_val");
	if (np == NULL) {
		pr_err("%s: no compatible node found\n", __func__);
		return -EPERM;
	}

	ret = of_property_read_u32_array(np, "hisi,bl31-share-mem",
					 &data[0], 2UL);
	if (ret != 0) {
		pr_err("%s, get val mem compatible node err\n", __func__);
		goto err_out;
	}

	g_cpu_volt_data.phy_addr = bl31_smem_base + data[0];
	g_cpu_volt_data.virt_addr =
		(unsigned char *)ioremap(bl31_smem_base + data[0], (size_t)data[1]);
	if (g_cpu_volt_data.virt_addr == NULL) {
		pr_err("%s: %d: alloc memory for virt_addr failed\n",
		       __func__, __LINE__);
		goto err_out;
	}
	mutex_init(&g_cpu_volt_data.cpu_mutex);

	proc_create("param", 0440, NULL, &get_volt_operations);

	return 0;
err_out:
	of_node_put(np);
	return -EPERM;
}

module_init(cpufreq_get_volt_init);

