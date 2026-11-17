/*
 * power_gpio.c
 *
 * gpio interface for power module
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_sysfs.h>
#include <chipset_common/hwpower/common_module/power_interface.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_gpio
HWLOG_REGIST();

static struct device *g_power_gpio_dev;
static int g_power_gpio_enable;

#if (defined(CONFIG_HUAWEI_POWER_DEBUG) && defined(CONFIG_SYSFS))
static const char * const g_power_gpio_op_user_table[POWER_GPIO_OP_USER_END] = {
	[POWER_GPIO_OP_USER_SHELL] = "shell",
	[POWER_GPIO_OP_USER_KERNEL] = "kernel",
};

static int power_gpio_check_op_user(const char *str)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(g_power_gpio_op_user_table); i++) {
		if (!strcmp(str, g_power_gpio_op_user_table[i]))
			return 0;
	}

	hwlog_err("invalid user_str=%s\n", str);
	return -1;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG && CONFIG_SYSFS */

static struct device_node *power_gpio_get_compatible(const char *compatible)
{
	struct device_node *np = NULL;

	if (!compatible) {
		hwlog_err("compatible is null\n");
		return NULL;
	}

	np = of_find_compatible_node(NULL, NULL, compatible);
	if (!np) {
		hwlog_err("compatible %s get fail\n", compatible);
		return NULL;
	}

	return np;
}

int power_gpio_request(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio)
{
	if (!np || !prop || !label || !gpio) {
		hwlog_err("np or prop or label or gpio is null\n");
		return -EINVAL;
	}

	*gpio = of_get_named_gpio(np, prop, 0);
	hwlog_info("%s: %s=%d\n", label, prop, *gpio);

	if (!gpio_is_valid(*gpio)) {
		hwlog_err("gpio %d is not valid\n", *gpio);
		return -EINVAL;
	}

	if (gpio_request(*gpio, label)) {
		hwlog_err("gpio %d request fail\n", *gpio);
		return -EINVAL;
	}

	return 0;
}

int power_gpio_config_input(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio)
{
	if (power_gpio_request(np, prop, label, gpio))
		return -EINVAL;

	if (gpio_direction_input(*gpio)) {
		gpio_free(*gpio);
		hwlog_err("gpio %d set input fail\n", *gpio);
		return -EINVAL;
	}

	return 0;
}

int power_gpio_config_output(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio, int value)
{
	if (power_gpio_request(np, prop, label, gpio))
		return -EINVAL;

	if (gpio_direction_output(*gpio, value)) {
		gpio_free(*gpio);
		hwlog_err("gpio %d set output fail\n", *gpio);
		return -EINVAL;
	}

	return 0;
}

int power_gpio_config_interrupt(struct device_node *np,
	const char *prop, const char *label, unsigned int *gpio, int *irq)
{
	if (!irq) {
		hwlog_err("irq is null\n");
		return -EINVAL;
	}

	if (power_gpio_request(np, prop, label, gpio))
		return -EINVAL;

	if (gpio_direction_input(*gpio)) {
		gpio_free(*gpio);
		hwlog_err("gpio %d set input fail\n", *gpio);
		return -EINVAL;
	}

	*irq = gpio_to_irq(*gpio);
	if (*irq < 0) {
		gpio_free(*gpio);
		hwlog_err("gpio %d map to irq fail\n", *gpio);
		return -EINVAL;
	}

	hwlog_info("%s: irq=%d\n", label, *irq);
	return 0;
}

int power_gpio_config_output_compatible(const char *comp,
	const char *prop, const char *label, unsigned int *gpio, int value)
{
	struct device_node *np = power_gpio_get_compatible(comp);

	if (!np)
		return -EINVAL;

	return power_gpio_config_output(np, prop, label, gpio, value);
}

#if (defined(CONFIG_HUAWEI_POWER_DEBUG) && defined(CONFIG_SYSFS))
static ssize_t power_gpio_set_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char user[POWER_GPIO_RD_BUF_SIZE] = { 0 };
	unsigned int gpio;
	unsigned int value;

	if (!g_power_gpio_enable)
		return -EINVAL;

	/* reserve 2 bytes to prevent buffer overflow */
	if (count >= (POWER_GPIO_RD_BUF_SIZE - 2)) {
		hwlog_err("input too long\n");
		return -EINVAL;
	}

	/* 3: the fields of "user gpio value" */
	if (sscanf(buf, "%s %d %d", user, &gpio, &value) != 3) {
		hwlog_err("unable to parse input:%s\n", buf);
		return -EINVAL;
	}

	if (power_gpio_check_op_user(user))
		return -EINVAL;

	/* gpio must be (1, 300) */
	if ((gpio < 1) || (gpio > 300)) {
		hwlog_err("invalid gpio=%d\n", gpio);
		return -EINVAL;
	}

	/* value must be 0 or 1, 0: low, 1: high */
	if ((value < 0) || (value > 1)) {
		hwlog_err("invalid value=%d\n", value);
		return -EINVAL;
	}

	hwlog_info("set: user=%s gpio=%d value=%d\n", user, gpio, value);

	if (gpio_direction_output(gpio, value)) {
		hwlog_err("gpio %d set output fail\n", gpio);
		return -EINVAL;
	}

	return count;
}

static DEVICE_ATTR(info, 0220, NULL, power_gpio_set_info);

static struct attribute *power_gpio_attributes[] = {
	&dev_attr_info.attr,
	NULL,
};

static const struct attribute_group power_gpio_attr_group = {
	.attrs = power_gpio_attributes,
};

static struct device *power_gpio_create_group(void)
{
	return power_sysfs_create_group("hw_power", "power_gpio",
		&power_gpio_attr_group);
}

static void power_gpio_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &power_gpio_attr_group);
}
#else
static inline struct device *power_gpio_create_group(void)
{
	return NULL;
}

static inline void power_gpio_remove_group(struct device *dev)
{
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG && CONFIG_SYSFS */

static int power_gpio_set_enable(unsigned int val)
{
	if (!g_power_gpio_dev)
		return -1;

	g_power_gpio_enable = val;
	return 0;
}

static int power_gpio_get_enable(unsigned int *val)
{
	if (!val)
		return -1;

	*val = g_power_gpio_enable;
	return 0;
}

static struct power_if_ops power_gpio_if_ops = {
	.set_enable = power_gpio_set_enable,
	.get_enable = power_gpio_get_enable,
	.type_name = "power_gpio",
};

static int __init power_gpio_init(void)
{
	g_power_gpio_dev = power_gpio_create_group();
	power_if_ops_register(&power_gpio_if_ops);
	return 0;
}

static void __exit power_gpio_exit(void)
{
	power_gpio_remove_group(g_power_gpio_dev);
	g_power_gpio_dev = NULL;
}

device_initcall_sync(power_gpio_init);
module_exit(power_gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("gpio for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
