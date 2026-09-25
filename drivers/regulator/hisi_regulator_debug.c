/*
 * hisi_regulator_debug.c
 *
 * regulator debug API
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_regulator_debug.h"
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/string.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#endif

#ifdef CONFIG_HISI_REGULATOR_TRACE
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#endif

#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG HISI_PMIC_REGULATOR_DEBUG_TAG

#ifdef CONFIG_HISI_REGULATOR_TRACE

#define REGULATOR_NAME_LEN 16
#define REGULATOR_MAGIC_NUM 0x16022602U

#ifdef CONFIG_GCOV_KERNEL
#define STATIC
#else
#define STATIC static
#endif

struct pc_record_info {
	unsigned int   dump_magic;
	unsigned int   buffer_size;
	unsigned char *buffer_addr;
	unsigned char *percpu_addr[NR_CPUS];
	unsigned int   percpu_length[NR_CPUS];
};

struct regulator_record_info {
	u64 current_time;
	/* 0: enable 1: set vol  2: mode */
	u8 item;
	u8 mode;
	u16 enalbe_count;
	int max_uv;
	int min_uv;
	char comm[REGULATOR_NAME_LEN];
};

enum BUF_TYPE_EN {
	SINGLE_BUF = 0,
	MULTI_BUF,
};

static u64 g_regulator_rdr_core_id = RDR_REGULATOR;
static struct pc_record_info *g_regulator_track_addr;
static unsigned char g_regulator_hook_on;
static enum BUF_TYPE_EN g_regulator_sel_buf_type = MULTI_BUF;
#endif

#ifdef CONFIG_HISI_PMIC_DEBUG
void get_current_regulator_dev(struct seq_file *s)
{
	struct regulator_dev *rdev = NULL;
	const struct regulator_ops *ops = NULL;
	int enabled = 0;
	struct list_head *regulator_list = get_regulator_list();

	if (regulator_list == NULL)
		return;

	list_for_each_entry(rdev, regulator_list, list) {
		if (rdev->constraints->name == NULL)
			break;
		seq_printf(s, "%-15s", rdev->constraints->name);
		ops = rdev->desc->ops;
		if (ops == NULL)
			return;
		if (ops->is_enabled)
			enabled = ops->is_enabled(rdev);

		if (enabled)
			seq_printf(s, "%-20s", "ON");
		else
			seq_printf(s, "%-20s", "OFF");

		seq_printf(s, "%u\t\t", rdev->use_count);
		seq_printf(s, "%u\t\t", rdev->open_count);
		if (rdev->constraints->always_on)
			seq_printf(s, "%-17s\n\r", "ON");
		else
			seq_printf(s, "%-17s\n\r", "OFF");
	}
}

static int regulator_dev_check(struct regulator_dev *rdev)
{
	if ((rdev == NULL)              ||
	    (rdev->desc == NULL)        ||
	    (rdev->desc->ops == NULL)   ||
	    (rdev->constraints == NULL) ||
	    (rdev->constraints->name == NULL))
		return -1;

	if (rdev->constraints->name == NULL) {
		pr_info("Couldnot find your ldo name\n");
		return -1;
	}

	return 0;
}

void set_regulator_state(const char *ldo_name, int value)
{
	struct regulator_dev *rdev = NULL;
	const struct regulator_ops *ops = NULL;
	int len;
	struct list_head *regulator_list = get_regulator_list();

	if ((regulator_list == NULL) ||
	    (ldo_name == NULL)       ||
	    ((value != 0) && (value != 1)))
		return;

	pr_info("ldo_name = %s,value = %d\n", ldo_name, value);
	list_for_each_entry(rdev, regulator_list, list) {
		if (regulator_dev_check(rdev))
			return;

		ops = rdev->desc->ops;
		len = strlen(rdev->constraints->name) > strlen(ldo_name) ?
			strlen(rdev->constraints->name) : strlen(ldo_name);
		if (strncmp(rdev->constraints->name, ldo_name, len) == 0) {
			if ((value == 0) && ops->disable) {
				pr_info("close the %s\n", ldo_name);
				ops->disable(rdev);
			} else if ((value == 1) && ops->enable) {
				pr_info("open the %s\n", ldo_name);
				ops->enable(rdev);
			}
			return;
		}
	}
}

int set_regulator_voltage(const char *ldo_name, unsigned int vol_value)
{
	struct regulator_dev *rdev = NULL;
	const struct regulator_ops *ops = NULL;
	int len;
	unsigned int selector;
	struct list_head *regulator_list = get_regulator_list();

	if ((regulator_list == NULL) || (ldo_name == NULL)) {
		pr_info("regulator_list is NULL or voltage value is invalid\n");
		return -1;
	}
	pr_info("ldo_name=%s,vol_value=%u\n", ldo_name, vol_value);
	list_for_each_entry(rdev, regulator_list, list) {
		if (regulator_dev_check(rdev))
			return -1;
		ops = rdev->desc->ops;
		len = strlen(rdev->constraints->name) > strlen(ldo_name) ?
			strlen(rdev->constraints->name) : strlen(ldo_name);
		if (strncmp(rdev->constraints->name, ldo_name, len) == 0) {
			if (ops->set_voltage && ops->set_voltage(rdev, vol_value,
				vol_value, &selector)) {
				pr_info("voltage set fail\n");
				return -1;
			}
			pr_info("voltage set succ, selector is %u\n", selector);
			return 0;
		}
	}
	pr_info("Could not find your ldo\n");
	return -1;
}

void get_regulator_state(const char *ldo_name, int length)
{
	struct regulator_dev *rdev = NULL;
	const struct regulator_ops *ops = NULL;
	int state_value;
	struct list_head *regulator_list = get_regulator_list();

	if ((regulator_list == NULL) || (ldo_name == NULL))
		return;

	pr_info("ldo_name=%s\n", ldo_name);
	list_for_each_entry(rdev, regulator_list, list) {
		if (regulator_dev_check(rdev))
			return;

		ops = rdev->desc->ops;
		if (strncmp(rdev->constraints->name, ldo_name, length) == 0) {
			if (ops->is_enabled) {
				state_value = ops->is_enabled(rdev);
				pr_info("enabled_state is %d, ", state_value);
			}
			if (ops->get_voltage) {
				state_value = ops->get_voltage(rdev);
				pr_info("voltage_value the %d\n", state_value);
			}
			return;
		}
	}
}

#endif
#ifdef CONFIG_HISI_SR_DEBUG
void get_ip_regulator_state(void)
{
	struct regulator_dev *rdev = NULL;
	const struct regulator_ops *ops = NULL;
	int enabled = 0;
	struct list_head *regulator_list = get_regulator_list();

	if (regulator_list == NULL)
		return;

	pr_info("Get IP Regulator State!\n");

	list_for_each_entry(rdev, regulator_list, list) {
		if ((rdev == NULL)              ||
		    (rdev->desc == NULL)        ||
		    (rdev->desc->ops == NULL)   ||
		    (rdev->constraints == NULL) ||
		    (rdev->constraints->name == NULL))
			return;

		if ((strncmp(rdev->constraints->name, "ldo", strlen("ldo")) == 0) ||
		    (strncmp(rdev->constraints->name, "schg", strlen("schg")) == 0))
			continue;

		ops = rdev->desc->ops;
		if (ops->is_enabled) {
			enabled = ops->is_enabled(rdev);
			if (enabled)
				pr_err("N:%s, S:%d, C:%u\n",
					rdev->constraints->name, enabled,
					rdev->use_count);
		}
	}
}
#endif

#ifdef CONFIG_HISI_REGULATOR_TRACE
static const char *track_regulator_rdev_get_name(struct regulator_dev *rdev)
{
	if (rdev->constraints && rdev->constraints->name)
		return rdev->constraints->name;
	else if (rdev->desc->name)
		return rdev->desc->name;
	else
		return "";
}

static void __track_regulator(struct regulator_dev *rdev,
	enum track_regulator_type track_item, u8 mode, int max_uv, int min_uv)
{
	struct regulator_record_info info = {0};
	u8 cpu;

	if (!g_regulator_hook_on)
		return;

	cpu = (u8)raw_smp_processor_id();
	if (g_regulator_sel_buf_type == SINGLE_BUF)
		cpu = 0;

	info.current_time = hisi_getcurtime();
	info.item = track_item;
	info.mode = mode;
	info.enalbe_count = rdev->use_count;
	info.max_uv = max_uv;
	info.min_uv = min_uv;
	strncpy(info.comm, track_regulator_rdev_get_name(rdev),
		sizeof(info.comm) - 1);
	info.comm[REGULATOR_NAME_LEN - 1] = '\0';
	pr_debug("######%s!\n", info.comm);
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)g_regulator_track_addr->percpu_addr[cpu],
		(u8 *)&info);
}

void track_regulator_onoff(struct regulator_dev *rdev,
	enum track_regulator_type track_item)
{
	if (IS_ERR_OR_NULL(rdev)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	if (track_item >= TRACK_REGULATOR_MAX) {
		pr_err("[%s], track_type [%d] is invalid!\n", __func__,
			track_item);
		return;
	}
	__track_regulator(rdev, TRACK_ON_OFF, 0, 0, 0);
}

void track_regulator_set_vol(struct regulator_dev *rdev,
	enum track_regulator_type track_item, int max_uv, int min_uv)
{
	if (IS_ERR_OR_NULL(rdev)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	if (track_item >= TRACK_REGULATOR_MAX) {
		pr_err("[%s], track_type [%d] is invalid!\n", __func__,
			track_item);
		return;
	}
	__track_regulator(rdev, TRACK_VOL, 0, max_uv, min_uv);
}

void track_regulator_set_mode(struct regulator_dev *rdev,
	enum track_regulator_type track_item, u8 mode)
{
	if (IS_ERR_OR_NULL(rdev)) {
		pr_err("%s param is null!\n", __func__);
		return;
	}
	if (track_item >= TRACK_REGULATOR_MAX) {
		pr_err("[%s], track_type [%d] is invalid!\n", __func__,
			track_item);
		return;
	}
	__track_regulator(rdev, TRACK_VOL, mode, 0, 0);
}

static void track_regulator_reset(u32 modid, u32 etype, u64 coreid)
{
}

static void track_regulator_dump(u32 modid, u32 etype, u64 coreid,
	char *pathname, pfn_cb_dump_done pfn_cb)
{
	if (pfn_cb)
		pfn_cb(modid, coreid);

	pr_info("%s dump!\n", __func__);
}

STATIC int track_regulator_rdr_register(
	struct rdr_register_module_result *result)
{
	struct rdr_module_ops_pub s_module_ops = {
		.ops_dump = NULL,
		.ops_reset = NULL
	};

	int ret = -1;

	if (result == NULL) {
		pr_err("%s para null!\n", __func__);
		return ret;
	}

	pr_info("%s start!\n", __func__);

	s_module_ops.ops_dump = track_regulator_dump;
	s_module_ops.ops_reset = track_regulator_reset;
	ret = rdr_register_module_ops(g_regulator_rdr_core_id, &s_module_ops,
		result);

	pr_info("%s end!\n", __func__);

	return ret;
}

#define ALIGN8(size) ((size / BITS_PER_BYTE) * BITS_PER_BYTE)
static int regulator_percpu_buffer_init(u8 *addr, u32 size, u32 fieldcnt,
	u32 magic_number, u32 ratio[][BITS_PER_BYTE], u32 max_cpu_nums,
	enum BUF_TYPE_EN buf_type)
{
	int i, ret;
	u32 cpu_num = num_possible_cpus();

	if (cpu_num > max_cpu_nums) {
		pr_err("[%s] cpu number error!\n", __func__);
		return -1;
	}

	if (IS_ERR_OR_NULL(addr)) {
		pr_err("[%s], buffer_addr [0x%pK], buffer_size [0x%x]\n",
			__func__, addr, size);
		return -1;
	}
	if (buf_type == SINGLE_BUF)
		cpu_num = 1;
	pr_info("[%s], num_online_cpus [%d]!\n", __func__, num_online_cpus());

	/* set pc info for parse */
	g_regulator_track_addr = (struct pc_record_info *)addr;
	g_regulator_track_addr->buffer_addr = addr;
	g_regulator_track_addr->buffer_size = size - sizeof(struct pc_record_info);
	g_regulator_track_addr->dump_magic = magic_number;

	/* set per cpu buffer */
	for (i = 0; i < cpu_num; i++) {
		pr_info("[%s], ratio[%u][%d] = [%u]\n", __func__, (cpu_num - 1),
			i, ratio[cpu_num - 1][i]);
		/* 16: elements for per line */
		g_regulator_track_addr->percpu_length[i] =
			g_regulator_track_addr->buffer_size / 16 * ratio[cpu_num - 1][i];
		g_regulator_track_addr->percpu_length[i] =
			ALIGN8(g_regulator_track_addr->percpu_length[i]);

		if (i == 0)
			g_regulator_track_addr->percpu_addr[0] =
				g_regulator_track_addr->buffer_addr +
				sizeof(struct pc_record_info);
		else
			g_regulator_track_addr->percpu_addr[i] =
				g_regulator_track_addr->percpu_addr[i - 1] +
				g_regulator_track_addr->percpu_length[i - 1];

		pr_info("[%s], [%d]: percpu_addr [0x%pK], percpu_length [0x%x], fieldcnt [%u]\n",
			__func__, i, g_regulator_track_addr->percpu_addr[i],
			g_regulator_track_addr->percpu_length[i], fieldcnt);

		ret = hisiap_ringbuffer_init((struct hisiap_ringbuffer_s *)g_regulator_track_addr->percpu_addr[i],
			g_regulator_track_addr->percpu_length[i], fieldcnt,
			"regulator");
		if (ret) {
			pr_err("[%s], cpu [%d] ringbuffer init failed!\n",
				__func__, i);
			return ret;
		}
	}
	return 0;
}

static int regulator_buffer_init(u8 *addr, u32 size, enum BUF_TYPE_EN buf_type)
{
	unsigned int record_ratio[BITS_PER_BYTE][BITS_PER_BYTE] = {
		{ 16, 0, 0, 0, 0, 0, 0, 0 },
		{  8, 8, 0, 0, 0, 0, 0, 0 },
		{  6, 5, 5, 0, 0, 0, 0, 0 },
		{  4, 4, 4, 4, 0, 0, 0, 0 },
		{  4, 4, 4, 3, 1, 0, 0, 0 },
		{  4, 4, 3, 3, 1, 1, 0, 0 },
		{  4, 3, 3, 3, 1, 1, 1, 0 },
		{  3, 3, 3, 3, 1, 1, 1, 1 }
	};

	return regulator_percpu_buffer_init(addr, size,
		sizeof(struct regulator_record_info), REGULATOR_MAGIC_NUM,
		record_ratio, BITS_PER_BYTE, buf_type);
}

STATIC int __init track_regulator_record_init(void)
{
	int ret;
	struct rdr_register_module_result regulator_rdr_info;
	unsigned char *vir_addr = NULL;

	/* alloc rdr memory and init */
	ret = track_regulator_rdr_register(&regulator_rdr_info);
	if (ret)
		return ret;

	if (regulator_rdr_info.log_len == 0) {
		pr_err("%s clk_rdr_len is 0x0!\n", __func__);
		return 0;
	}
	vir_addr = (unsigned char *)hisi_bbox_map(
		(phys_addr_t)regulator_rdr_info.log_addr,
		regulator_rdr_info.log_len);
	pr_info("%s log_addr is 0x%llx, log_len is 0x%x!\n", __func__,
		regulator_rdr_info.log_addr, regulator_rdr_info.log_len);
	if (IS_ERR_OR_NULL(vir_addr)) {
		pr_err("%s vir_addr err!\n", __func__);
		return -1;
	}

	memset(vir_addr, 0, regulator_rdr_info.log_len);

	ret = regulator_buffer_init(vir_addr, regulator_rdr_info.log_len,
		g_regulator_sel_buf_type);
	if (ret) {
		pr_err("%s buffer init err!\n", __func__);
		return -1;
	}

	if (check_himntn(HIMNTN_TRACE_CLK_REGULATOR))
		g_regulator_hook_on = 1;

	pr_err("%s: hook_on = %u,rdr_phy_addr = 0x%llx, rdr_len = 0x%x, rdr_virt_add = 0x%pK\n",
		__func__, g_regulator_hook_on, regulator_rdr_info.log_addr,
		regulator_rdr_info.log_len, vir_addr);

	return 0;
}

STATIC void __exit track_regulator_record_exit(void)
{
}

module_init(track_regulator_record_init);
module_exit(track_regulator_record_exit);
#endif

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisi regulator debug func");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
