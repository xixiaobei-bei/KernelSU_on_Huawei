/*
 * soc_decimal.c
 *
 * calculate soc with decimal driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "soc_decimal.h"
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG soc_decimal
HWLOG_REGIST();

static struct soc_decimal_dev *g_soc_decimal_dev;

static const char * const g_soc_decimal_op_user_table[SOC_DECIMAL_OP_USER_END] = {
	[SOC_DECIMAL_OP_USER_SYSTEM_UI] = "system_ui",
};

static int soc_decimal_get_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_soc_decimal_op_user_table); i++) {
		if (!strcmp(str, g_soc_decimal_op_user_table[i]))
			return i;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}

static struct soc_decimal_dev *soc_decimal_get_dev(void)
{
	if (!g_soc_decimal_dev) {
		hwlog_err("g_soc_decimal_dev is null\n");
		return NULL;
	}

	return g_soc_decimal_dev;
}

static u32 soc_decimal_get_soc(struct soc_decimal_dev *l_dev)
{
	int soc;

	if (!l_dev->info.start)
		soc = power_platform_get_filter_soc(SOC_DECIMAL_DEFAULT_BASE);
	else if (l_dev->info.level == SOC_DECIMAL_DEFAULT_LEVEL)
		soc = power_platform_get_filter_soc(SOC_DECIMAL_DEFAULT_BASE);
	else
		soc = l_dev->info.soc;

	return soc + l_dev->ui_offset;
}

static void soc_decimal_init_fifo(struct soc_decimal_dev *l_dev)
{
	int curr_soc;
	int i;
	int ui_soc;

	curr_soc = power_platform_get_filter_soc(l_dev->info.para.base);
	ui_soc = power_platform_get_battery_ui_capacity() * 10;
	curr_soc = curr_soc < ui_soc ? ui_soc : curr_soc;

	l_dev->info.fifo.sum = 0;
	for (i = 0; i < SOC_DECIMAL_WINDOW_LEN; i++) {
		l_dev->info.fifo.filter[i] = curr_soc;
		l_dev->info.fifo.sum += curr_soc;
	}

	l_dev->info.fifo.index = 0;
	l_dev->info.fifo.curr_samples = 0;
	l_dev->info.fifo.init_soc = curr_soc;
	l_dev->info.soc = curr_soc;
	hwlog_info("init decimal_soc=%d,ui_soc=%d\n", l_dev->info.fifo.init_soc, ui_soc);
}

static u32 soc_decimal_pulling_fifo(struct soc_decimal_dev *l_dev, u32 soc)
{
	int index;

	index = l_dev->info.fifo.index % SOC_DECIMAL_WINDOW_LEN;
	l_dev->info.fifo.sum -= l_dev->info.fifo.filter[index];
	l_dev->info.fifo.filter[index] = soc;
	l_dev->info.fifo.sum += soc;
	l_dev->info.fifo.index++;

	return l_dev->info.fifo.sum / SOC_DECIMAL_WINDOW_LEN;
}

static u32 soc_decimal_calculate_soc(struct soc_decimal_dev *l_dev)
{
	struct soc_decimal_calc_data calc;

	calc.base = l_dev->info.para.base;
	calc.samples = l_dev->info.para.samples;
	calc.curr_samples = l_dev->info.fifo.curr_samples;
	calc.init_soc = l_dev->info.fifo.init_soc;

	if (calc.base == 0) {
		hwlog_err("base is 0, use default base\n");
		calc.base = SOC_DECIMAL_DEFAULT_BASE;
	}

	/* first: get decimal soc */
	if (calc.curr_samples < calc.samples) {
		/*
		 * init_soc on (0, 80), soc = init_soc + (samples / 3) + 3
		 * init_soc on [80, 90),  soc = init_soc + (samples / 4)
		 * init_soc on [90, 100), soc = init_soc + (samples / 5)
		 */
		if (calc.init_soc < 80 * calc.base)
			calc.curr_soc = calc.init_soc + (calc.curr_samples / 3) + 3;
		else if ((calc.init_soc >= 80 * calc.base) &&
			(calc.init_soc < 90 * calc.base))
			calc.curr_soc = calc.init_soc + (calc.curr_samples / 4);
		else
			calc.curr_soc = calc.init_soc + (calc.curr_samples / 5);
		l_dev->info.fifo.curr_samples++;
	} else {
		return power_platform_get_filter_soc(calc.base);
	}

	/* second : filter decimal soc */
	calc.rep_soc = soc_decimal_pulling_fifo(l_dev, calc.curr_soc);
	calc.round_soc = calc.rep_soc / calc.base;
	calc.remain_soc = calc.rep_soc % calc.base;

	hwlog_info("samples[%d]: curr_soc=%d rep_soc=%d round=%d remain=%d\n",
		l_dev->info.fifo.curr_samples,
		calc.curr_soc, calc.rep_soc, calc.round_soc, calc.remain_soc);

	power_platform_sync_filter_soc(calc.rep_soc, calc.round_soc, calc.base);
	return calc.rep_soc;
}

static void soc_decimal_timer_start(struct soc_decimal_dev *l_dev)
{
	int t;

	if (!l_dev)
		return;

	t = l_dev->info.para.interval;
	hrtimer_start(&l_dev->timer,
		ktime_set(t / MSEC_PER_SEC, (t % MSEC_PER_SEC) * USEC_PER_SEC),
		HRTIMER_MODE_REL);
}

static enum hrtimer_restart soc_decimal_timer_func(struct hrtimer *timer)
{
	struct soc_decimal_dev *l_dev = soc_decimal_get_dev();

	if (!l_dev)
		return HRTIMER_NORESTART;

	if (!l_dev->info.start)
		goto restart_work;

	if (l_dev->info.level == SOC_DECIMAL_DEFAULT_LEVEL)
		goto restart_work;

	/* must exit timer when current samples is exceed specified samples */
	if (l_dev->info.fifo.curr_samples >= l_dev->info.para.samples)
		goto restart_work;

	l_dev->info.soc = soc_decimal_calculate_soc(l_dev);
	soc_decimal_timer_start(l_dev);
	return HRTIMER_NORESTART;

restart_work:
	power_platform_restart_capacity_work();
	return HRTIMER_NORESTART;
}

static void soc_decimal_select_para_level(struct soc_decimal_dev *l_dev,
	int type, const void *data)
{
	u32 power;
	int i;

	if (!data) {
		hwlog_info("data is null\n");
		goto fail_get_level;
	}

	power = *(u32 *)data;
	hwlog_info("power=%d, para_level=%d\n", power, l_dev->para_level);

	for (i = 0; i < l_dev->para_level; i++) {
		if ((type == l_dev->para[i].type) &&
			(power >= l_dev->para[i].min_pwr) &&
			(power < l_dev->para[i].max_pwr)) {
			l_dev->info.level = i;
			return;
		}
	}

fail_get_level:
	l_dev->info.level = SOC_DECIMAL_DEFAULT_LEVEL;
}

static void soc_decimal_reset_para(struct soc_decimal_dev *l_dev)
{
	memset(&l_dev->info, 0, sizeof(l_dev->info));
	l_dev->info.level = SOC_DECIMAL_DEFAULT_LEVEL;
}

static int soc_decimal_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct soc_decimal_dev *l_dev = soc_decimal_get_dev();

	if (!l_dev)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_SOC_DECIMAL_DC:
		soc_decimal_select_para_level(l_dev, SOC_DECIMAL_TYPE_DC, data);
		break;
	case POWER_NE_SOC_DECIMAL_WL_DC:
		soc_decimal_select_para_level(l_dev, SOC_DECIMAL_TYPE_WL_DC, data);
		break;
	case POWER_NE_STOP_CHARGING:
		soc_decimal_reset_para(l_dev);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

#ifdef CONFIG_SYSFS
static ssize_t soc_decimal_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t soc_decimal_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info soc_decimal_sysfs_field_tbl[] = {
	power_sysfs_attr_wo(soc_decimal, 0220, SOC_DECIMAL_SYSFS_START, start),
	power_sysfs_attr_ro(soc_decimal, 0440, SOC_DECIMAL_SYSFS_SOC, soc),
	power_sysfs_attr_ro(soc_decimal, 0440, SOC_DECIMAL_SYSFS_LEVEL, level),
	power_sysfs_attr_ro(soc_decimal, 0440, SOC_DECIMAL_SYSFS_PARA, para),
};

#define SOC_DECIMAL_SYSFS_ATTRS_SIZE  ARRAY_SIZE(soc_decimal_sysfs_field_tbl)

static struct attribute *soc_decimal_sysfs_attrs[SOC_DECIMAL_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group soc_decimal_sysfs_attr_group = {
	.attrs = soc_decimal_sysfs_attrs,
};

static ssize_t soc_decimal_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct soc_decimal_dev *l_dev = soc_decimal_get_dev();
	int i;
	char rd_buf[SOC_DECIMAL_RD_BUF_SIZE] = { 0 };

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		soc_decimal_sysfs_field_tbl, SOC_DECIMAL_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case SOC_DECIMAL_SYSFS_SOC:
		return scnprintf(buf, SOC_DECIMAL_RD_BUF_SIZE, "%d\n",
			soc_decimal_get_soc(l_dev));
	case SOC_DECIMAL_SYSFS_LEVEL:
		return scnprintf(buf, SOC_DECIMAL_RD_BUF_SIZE, "%d\n",
			l_dev->info.level);
	case SOC_DECIMAL_SYSFS_PARA:
		for (i = 0; i < l_dev->para_level; i++) {
			memset(rd_buf, 0, SOC_DECIMAL_RD_BUF_SIZE);
			scnprintf(rd_buf, SOC_DECIMAL_RD_BUF_SIZE,
				"%d %d %d %d %d %d\n",
				l_dev->para[i].type,
				l_dev->para[i].min_pwr,
				l_dev->para[i].max_pwr,
				l_dev->para[i].base,
				l_dev->para[i].samples,
				l_dev->para[i].interval);
			strncat(buf, rd_buf, strlen(rd_buf));
		}
		return strlen(buf);
	default:
		return 0;
	}
}

static int soc_decimal_sysfs_store_start(struct soc_decimal_dev *l_dev,
	const char *buf)
{
	char user[SOC_DECIMAL_WR_BUF_SIZE] = { 0 };
	int value;

	/* 2: the fields of "user value" */
	if (sscanf(buf, "%s %d", user, &value) != 2) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (soc_decimal_get_op_user(user) < 0)
		return -EINVAL;

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((value < 0) || (value > 1)) {
		hwlog_err("invalid start=%d\n", value);
		return -EINVAL;
	}
	hwlog_info("set: user=%s, start=%d, level=%d, para_level=%d\n",
		user, value, l_dev->info.level, l_dev->para_level);

	if (l_dev->info.start == value) {
		hwlog_info("ignore the same start event\n");
		return 0;
	}
	l_dev->info.start = value;

	if (!l_dev->info.start)
		return 0;

	/* delay 10 ms and wait for info.level to update */
	if (l_dev->info.level == SOC_DECIMAL_DEFAULT_LEVEL)
		msleep(SOC_DECIMAL_WAIT_TIME);

	if ((l_dev->info.level == SOC_DECIMAL_DEFAULT_LEVEL) ||
		(l_dev->info.level < 0) ||
		(l_dev->info.level >= l_dev->para_level))
		return 0;

	l_dev->info.para.type = l_dev->para[l_dev->info.level].type;
	l_dev->info.para.min_pwr = l_dev->para[l_dev->info.level].min_pwr;
	l_dev->info.para.max_pwr = l_dev->para[l_dev->info.level].max_pwr;
	l_dev->info.para.base = l_dev->para[l_dev->info.level].base;
	l_dev->info.para.samples = l_dev->para[l_dev->info.level].samples;
	l_dev->info.para.interval = l_dev->para[l_dev->info.level].interval;
	soc_decimal_init_fifo(l_dev);
	soc_decimal_timer_start(l_dev);
	power_platform_cancle_capacity_work();
	return 0;
}

static ssize_t soc_decimal_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct soc_decimal_dev *l_dev = soc_decimal_get_dev();
	int ret;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		soc_decimal_sysfs_field_tbl, SOC_DECIMAL_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (SOC_DECIMAL_WR_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	switch (info->name) {
	case SOC_DECIMAL_SYSFS_START:
		ret = soc_decimal_sysfs_store_start(l_dev, buf);
		if (ret)
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static struct device *soc_decimal_sysfs_create_group(void)
{
	power_sysfs_init_attrs(soc_decimal_sysfs_attrs,
		soc_decimal_sysfs_field_tbl, SOC_DECIMAL_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "soc_decimal",
		&soc_decimal_sysfs_attr_group);
}

static void soc_decimal_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &soc_decimal_sysfs_attr_group);
}
#else
static inline struct device *soc_decimal_sysfs_create_group(void)
{
	return NULL;
}

static inline void soc_decimal_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int soc_decimal_parse_dts(struct device_node *np,
	struct soc_decimal_dev *l_dev)
{
	int len, ret, i, j;
	u32 para[SOC_DECIMAL_PARA_LEVEL * SOC_DECIMAL_ITEM_TOTAL] = { 0 };

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ui_offset", &l_dev->ui_offset, POWER_PLATFOR_SOC_UI_OFFSET);

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
		"para", SOC_DECIMAL_PARA_LEVEL, SOC_DECIMAL_ITEM_TOTAL);
	if (len < 0)
		return -EINVAL;

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"para", para, len);
	if (ret < 0)
		return -EINVAL;

	l_dev->para_level = len / SOC_DECIMAL_ITEM_TOTAL;
	for (i = 0; i < len / SOC_DECIMAL_ITEM_TOTAL; i++) {
		j = SOC_DECIMAL_ITEM_TOTAL * i;
		l_dev->para[i].type = para[j + SOC_DECIMAL_ITEM_TYPE];
		l_dev->para[i].min_pwr = para[j + SOC_DECIMAL_ITEM_MIN_PWR];
		l_dev->para[i].max_pwr = para[j + SOC_DECIMAL_ITEM_MAX_PWR];
		l_dev->para[i].base = para[j + SOC_DECIMAL_ITEM_BASE];
		l_dev->para[i].samples = para[j + SOC_DECIMAL_ITEM_SAMPLES];
		l_dev->para[i].interval = para[j + SOC_DECIMAL_ITEM_INTERVAL];
	}

	return 0;
}

static int soc_decimal_probe(struct platform_device *pdev)
{
	int ret;
	struct soc_decimal_dev *l_dev = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_soc_decimal_dev = l_dev;
	np = pdev->dev.of_node;
	l_dev->info.level = SOC_DECIMAL_DEFAULT_LEVEL;

	ret = soc_decimal_parse_dts(np, l_dev);
	if (ret)
		goto fail_free_mem;

	l_dev->nb.notifier_call = soc_decimal_notifier_call;
	ret = power_event_bnc_register(POWER_BNT_SOC_DECIMAL, &l_dev->nb);
	if (ret)
		goto fail_free_mem;
	ret = power_event_bnc_register(POWER_BNT_CHARGING, &l_dev->nb);
	if (ret)
		goto fail_regist_event;

	l_dev->dev = soc_decimal_sysfs_create_group();
	hrtimer_init(&l_dev->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	l_dev->timer.function = soc_decimal_timer_func;
	return 0;

fail_regist_event:
	power_event_bnc_unregister(POWER_BNT_SOC_DECIMAL, &l_dev->nb);
fail_free_mem:
	kfree(l_dev);
	g_soc_decimal_dev = NULL;
	return ret;
}

static int soc_decimal_remove(struct platform_device *pdev)
{
	struct soc_decimal_dev *l_dev = g_soc_decimal_dev;

	if (!l_dev)
		return -ENODEV;

	power_event_bnc_unregister(POWER_BNT_SOC_DECIMAL, &l_dev->nb);
	power_event_bnc_unregister(POWER_BNT_CHARGING, &l_dev->nb);
	soc_decimal_sysfs_remove_group(l_dev->dev);
	hrtimer_cancel(&l_dev->timer);
	kfree(l_dev);
	g_soc_decimal_dev = NULL;
	return 0;
}

static const struct of_device_id soc_decimal_match_table[] = {
	{
		.compatible = "huawei,soc_decimal",
		.data = NULL,
	},
	{},
};

static struct platform_driver soc_decimal_driver = {
	.probe = soc_decimal_probe,
	.remove = soc_decimal_remove,
	.driver = {
		.name = "huawei,soc_decimal",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(soc_decimal_match_table),
	},
};

static int __init soc_decimal_init(void)
{
	return platform_driver_register(&soc_decimal_driver);
}

static void __exit soc_decimal_exit(void)
{
	platform_driver_unregister(&soc_decimal_driver);
}

fs_initcall_sync(soc_decimal_init);
module_exit(soc_decimal_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("calculate soc with decimal driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
