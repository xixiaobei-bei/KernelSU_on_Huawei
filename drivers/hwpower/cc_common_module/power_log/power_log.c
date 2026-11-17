/*
 * power_log.c
 *
 * log for power module
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

#include <chipset_common/hwpower/common_module/power_log.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_log
HWLOG_REGIST();

static struct power_log_dev *g_power_log_dev;

static const char * const g_power_log_device_id_table[] = {
	[POWER_LOG_DEVICE_ID_SERIES_BATT] = "series_batt",
	[POWER_LOG_DEVICE_ID_SERIES_CAP] = "series_cap",
	[POWER_LOG_DEVICE_ID_BATT_INFO] = "batt_info",
	[POWER_LOG_DEVICE_ID_MULTI_BTB] = "multi_btb",
	[POWER_LOG_DEVICE_ID_BD99954] = "bd99954",
	[POWER_LOG_DEVICE_ID_BQ2419X] = "bq2419x",
	[POWER_LOG_DEVICE_ID_BQ2429X] = "bq2429x",
	[POWER_LOG_DEVICE_ID_BQ2560X] = "bq2560x",
	[POWER_LOG_DEVICE_ID_BQ25882] = "bq25882",
	[POWER_LOG_DEVICE_ID_BQ25892] = "bq25892",
	[POWER_LOG_DEVICE_ID_ETA6937] = "eta6937",
	[POWER_LOG_DEVICE_ID_HL7019] = "hl7019",
	[POWER_LOG_DEVICE_ID_RT9466] = "rt9466",
	[POWER_LOG_DEVICE_ID_RT9471] = "rt9471",
	[POWER_LOG_DEVICE_ID_BQ25970] = "bq25970",
	[POWER_LOG_DEVICE_ID_RT9759] = "rt9759",
	[POWER_LOG_DEVICE_ID_BQ25970_AUX] = "bq25970_aux",
	[POWER_LOG_DEVICE_ID_RT9759_AUX] = "rt9759_aux",
	[POWER_LOG_DEVICE_ID_AW32280] = "aw32280",
	[POWER_LOG_DEVICE_ID_AW32280_AUX] = "aw32280_aux",
	[POWER_LOG_DEVICE_ID_HI6522] = "hi6522",
	[POWER_LOG_DEVICE_ID_HI6523] = "hi6523",
	[POWER_LOG_DEVICE_ID_HI6526] = "hi6526",
	[POWER_LOG_DEVICE_ID_HI6526_AUX] = "hi6526_aux",
	[POWER_LOG_DEVICE_ID_SM5450] = "sm5450",
	[POWER_LOG_DEVICE_ID_SM5450_AUX] = "sm5450_aux",
	[POWER_LOG_DEVICE_ID_SGM41511H] = "sgm41511h",
	[POWER_LOG_DEVICE_ID_MT6360] = "mt6360",
	[POWER_LOG_DEVICE_ID_RT9426] = "rt9426",
	[POWER_LOG_DEVICE_ID_SC8545] = "sc8545",
	[POWER_LOG_DEVICE_ID_SC8545_AUX] = "sc8545_aux",
	[POWER_LOG_DEVICE_ID_BQ27Z561] = "bq27z561",
	[POWER_LOG_DEVICE_ID_BQ40Z50] = "bq40z50",
	[POWER_LOG_DEVICE_ID_BQ25713] = "bq25713",
};

static struct power_log_dev *power_log_get_dev(void)
{
	if (!g_power_log_dev) {
		hwlog_err("g_power_log_dev is null\n");
		return NULL;
	}

	return g_power_log_dev;
}

static int power_log_get_device_id(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_power_log_device_id_table); i++) {
		if (!strncmp(str, g_power_log_device_id_table[i], strlen(str)))
			return i;
	}

	return -1;
}

static int power_log_check_type(int type)
{
	if ((type >= POWER_LOG_TYPE_BEGIN) && (type < POWER_LOG_TYPE_END))
		return 0;

	hwlog_err("invalid type=%d\n", type);
	return -1;
}

static int power_log_check_dev_id(int dev_id)
{
	if ((dev_id >= POWER_LOG_DEVICE_ID_BEGIN) && (dev_id < POWER_LOG_DEVICE_ID_END))
		return 0;

	hwlog_err("invalid dev_id=%d\n", dev_id);
	return -EPERM;
}

static int power_log_set_dev_id(struct power_log_dev *l_dev, const char *str)
{
	int dev_id = power_log_get_device_id(str);

	if (dev_id < 0)
		return -EPERM;

	if (power_log_check_dev_id(dev_id))
		return -EPERM;

	if (!l_dev->ops[dev_id]) {
		hwlog_err("ops is null, dev_id=%d\n", dev_id);
		return -EPERM;
	}

	mutex_lock(&l_dev->devid_lock);
	l_dev->dev_id = dev_id;
	mutex_unlock(&l_dev->devid_lock);
	return 0;
}

static struct power_log_ops *power_log_get_ops(struct power_log_dev *l_dev)
{
	struct power_log_ops *ops = NULL;

	if (power_log_check_dev_id(l_dev->dev_id))
		return NULL;

	mutex_lock(&l_dev->devid_lock);
	ops = l_dev->ops[l_dev->dev_id];
	mutex_unlock(&l_dev->devid_lock);
	return ops;
}

static int power_log_operate_ops(struct power_log_ops *ops,
	int type, char *buf, int size)
{
	int ret = POWER_LOG_INVAID_OP;

	switch (type) {
	case POWER_LOG_DUMP_LOG_HEAD:
		if (!ops->dump_log_head)
			return ret;
		memset(buf, 0, size);
		return ops->dump_log_head(buf, size, ops->dev_data);
	case POWER_LOG_DUMP_LOG_CONTENT:
		if (!ops->dump_log_content)
			return ret;
		memset(buf, 0, size);
		return ops->dump_log_content(buf, size, ops->dev_data);
	default:
		return ret;
	}
}

static int power_log_common_operate(int type, char *buf, int size)
{
	int i, ret;
	int buf_size;
	int used = 0;
	int unused;
	struct power_log_dev *l_dev = NULL;

	if (!buf)
		return -EINVAL;

	ret = power_log_check_type(type);
	if (ret)
		return -EINVAL;

	l_dev = power_log_get_dev();
	if (!l_dev)
		return -EINVAL;

	mutex_lock(&l_dev->log_lock);

	for (i = 0; i < POWER_LOG_DEVICE_ID_END; i++) {
		if (!l_dev->ops[i])
			continue;

		ret = power_log_operate_ops(l_dev->ops[i], type,
			l_dev->log_buf, POWER_LOG_MAX_SIZE - 1);
		if (ret) {
			/* if op is invalid, must be skip output */
			if (ret == POWER_LOG_INVAID_OP)
				continue;
			hwlog_err("error type=%d, i=%d, ret=%d\n", type, i, ret);
			break;
		}

		unused = size - POWER_LOG_RESERVED_SIZE - used;
		buf_size = strlen(l_dev->log_buf);
		if (unused > buf_size) {
			strncat(buf, l_dev->log_buf, buf_size);
			used += buf_size;
		} else {
			strncat(buf, l_dev->log_buf, unused);
			used += unused;
			break;
		}
	}
	buf_size = strlen("\n\0");
	strncat(buf, "\n\0", buf_size);

	mutex_unlock(&l_dev->log_lock);
	return used + buf_size;
}

int power_log_ops_register(struct power_log_ops *ops)
{
	struct power_log_dev *l_dev = g_power_log_dev;
	int dev_id;

	if (!l_dev || !ops || !ops->dev_name) {
		hwlog_err("l_dev or ops is null\n");
		return -EINVAL;
	}

	dev_id = power_log_get_device_id(ops->dev_name);
	if (dev_id < 0) {
		hwlog_err("%s ops register fail\n", ops->dev_name);
		return -EINVAL;
	}

	l_dev->ops[dev_id] = ops;
	l_dev->total_ops++;

	hwlog_info("total_ops=%d %d:%s ops register ok\n",
		l_dev->total_ops, dev_id, ops->dev_name);
	return 0;
}

#ifdef CONFIG_SYSFS
static ssize_t power_log_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t power_log_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info power_log_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(power_log, 0660, POWER_LOG_SYSFS_DEV_ID, dev_id),
	power_sysfs_attr_ro(power_log, 0440, POWER_LOG_SYSFS_HEAD, head),
	power_sysfs_attr_ro(power_log, 0440, POWER_LOG_SYSFS_HEAD_ALL, head_all),
	power_sysfs_attr_ro(power_log, 0440, POWER_LOG_SYSFS_CONTENT, content),
	power_sysfs_attr_ro(power_log, 0440, POWER_LOG_SYSFS_CONTENT_ALL, content_all),
};

#define POWER_LOG_SYSFS_ATTRS_SIZE  ARRAY_SIZE(power_log_sysfs_field_tbl)

static struct attribute *power_log_sysfs_attrs[POWER_LOG_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group power_log_sysfs_attr_group = {
	.attrs = power_log_sysfs_attrs,
};

static ssize_t power_log_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_log_dev *l_dev = power_log_get_dev();
	struct power_log_ops *ops = NULL;
	int i;

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_log_sysfs_field_tbl, POWER_LOG_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case POWER_LOG_SYSFS_DEV_ID:
		for (i = 0; i < POWER_LOG_DEVICE_ID_END; i++) {
			if (!l_dev->ops[i])
				continue;

			if (strlen(buf) >= (PAGE_SIZE - POWER_LOG_RD_BUF_SIZE))
				break;

			scnprintf(buf + strlen(buf), POWER_LOG_RD_BUF_SIZE, "%s\n",
				l_dev->ops[i]->dev_name);
		}
		return strlen(buf);
	case POWER_LOG_SYSFS_HEAD:
		ops = power_log_get_ops(l_dev);
		if (!ops)
			return -EINVAL;

		if (power_log_operate_ops(ops, POWER_LOG_DUMP_LOG_HEAD,
			buf, PAGE_SIZE - 1))
			return -EINVAL;

		return strlen(buf);
	case POWER_LOG_SYSFS_CONTENT:
		ops = power_log_get_ops(l_dev);
		if (!ops)
			return -EINVAL;

		if (power_log_operate_ops(ops, POWER_LOG_DUMP_LOG_CONTENT,
			buf, PAGE_SIZE - 1))
			return -EINVAL;

		return strlen(buf);
	case POWER_LOG_SYSFS_HEAD_ALL:
		return power_log_common_operate(POWER_LOG_DUMP_LOG_HEAD,
			buf, PAGE_SIZE);
	case POWER_LOG_SYSFS_CONTENT_ALL:
		return power_log_common_operate(POWER_LOG_DUMP_LOG_CONTENT,
			buf, PAGE_SIZE);
	default:
		return 0;
	}
}

static ssize_t power_log_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct power_log_dev *l_dev = power_log_get_dev();
	char dev_id[POWER_LOG_WR_BUF_SIZE] = { 0 };

	if (!l_dev)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		power_log_sysfs_field_tbl, POWER_LOG_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (POWER_LOG_WR_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	switch (info->name) {
	case POWER_LOG_SYSFS_DEV_ID:
		if (sscanf(buf, "%s\n", dev_id) != 1) {
			hwlog_err("unable to parse input:%s\n", buf);
			return -EINVAL;
		}

		if (power_log_set_dev_id(l_dev, dev_id))
			return -EINVAL;
		break;
	default:
		break;
	}

	return count;
}

static struct device *power_log_sysfs_create_group(void)
{
	power_sysfs_init_attrs(power_log_sysfs_attrs,
		power_log_sysfs_field_tbl, POWER_LOG_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "power_log",
		&power_log_sysfs_attr_group);
}

static void power_log_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_log_sysfs_attr_group);
}
#else
static inline struct device *power_log_sysfs_create_group(void)
{
	return NULL;
}

static inline void power_log_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static int __init power_log_init(void)
{
	struct power_log_dev *l_dev = NULL;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	l_dev->total_ops = 0;
	mutex_init(&l_dev->log_lock);
	mutex_init(&l_dev->devid_lock);
	g_power_log_dev = l_dev;
	l_dev->dev = power_log_sysfs_create_group();
	return 0;
}

static void __exit power_log_exit(void)
{
	struct power_log_dev *l_dev = g_power_log_dev;

	if (!l_dev)
		return;

	mutex_destroy(&l_dev->log_lock);
	mutex_destroy(&l_dev->devid_lock);
	power_log_sysfs_remove_group(l_dev->dev);
	kfree(l_dev);
	g_power_log_dev = NULL;
}

subsys_initcall(power_log_init);
module_exit(power_log_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("power log module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
