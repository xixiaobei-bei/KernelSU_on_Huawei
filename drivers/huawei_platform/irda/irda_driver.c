/*
 * irda_driver.c
 *
 * irda module init
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
 */

#include <irda_driver.h>

#define HWLOG_TAG irda
HWLOG_REGIST();

static struct class *irda_class;
static struct irda_irq_handler *g_ir_det; /* irda gpio detection */

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_client *g_irda_dsm_client;

static void irda_dsm_client_register(void)
{
	g_irda_dsm_client = dsm_find_client(DSM_CLIENT_NAME);
	if (!g_irda_dsm_client)
		hwlog_err("%s: fail\n", __func__);
	else
		hwlog_info("%s: succ\n", __func__);
}

static void irda_dsm_report(void)
{
	if (g_irda_dsm_client && !dsm_client_ocuppy(g_irda_dsm_client)) {
		dsm_client_record(g_irda_dsm_client,
			"irda element separated from board err\n");
		dsm_client_notify(g_irda_dsm_client,
			DSM_IRDA_SEPARATED_ERROR_NO);
	} else {
		hwlog_err("irda dsm report fail\n");
	}
}
#endif

struct class *get_irda_class()
{
	return irda_class;
}

static int get_chip_type(void)
{
	struct device_node *chip_np = NULL;
	int chip_type;
	int ret;

	chip_np = of_find_compatible_node(
				NULL, NULL, IRDA_DRIVER_COMPATIBLE_ID);
	if (!chip_np) {
		hwlog_err("not find node %s\n", IRDA_DRIVER_COMPATIBLE_ID);
		/* default type is maxim, for compatible with old products */
		return MAXIM_616;
	}

	ret = of_property_read_u32(chip_np, IRDA_CHIP_TYPE, &chip_type);
	if (ret) {
		hwlog_err("fail to get irda chip type\n");
		/* default type is maxim, for compatible with old products */
		return MAXIM_616;
	}
	return chip_type;
}

static void irda_detect_1st_read(int gpio, int err_target)
{
	int gpio_level = gpio_get_value(gpio);
	if (gpio_level == err_target) {
		hwlog_info("irda detect not in place for 1st read\n");
#ifdef CONFIG_HUAWEI_DSM
		irda_dsm_report();
#endif
	}
}

static irqreturn_t ir_detect_callback(int irq, void *dev_id)
{
	hwlog_info("irda detect interrupt\n");
#ifdef CONFIG_HUAWEI_DSM
	irda_dsm_report();
#endif
	return IRQ_HANDLED;
}

static int irda_irq_register(void)
{
	int ret;

	g_ir_det->irq = gpio_to_irq(g_ir_det->gpio);
	ret = request_irq(g_ir_det->irq, ir_detect_callback,
		((unsigned long)(g_ir_det->irq_flag) |
		IRQF_ONESHOT | IRQF_NO_SUSPEND | IRQF_NO_THREAD),
		"ir_detect_irq", NULL);
	if (ret < 0) {
		hwlog_err("%s: ir_detect_irq fail, ret=%d\n", __func__, ret);
		return -ENOENT;
	}
	hwlog_info("%s: ir detect irq request succ\n", __func__);

	return 0;
}

static int irda_detect_init(void)
{
	struct device_node *chip_np = NULL;
	int ret;

	chip_np = of_find_compatible_node(
				NULL, NULL, IRDA_DRIVER_COMPATIBLE_ID);
	if (!chip_np) {
		hwlog_err("not find node %s\n", IRDA_DRIVER_COMPATIBLE_ID);
		return -ENOENT;
	}

	chip_np = of_get_child_by_name(chip_np, "irq_handler");
	if (!chip_np) {
		/* unsupport */
		hwlog_info("%s: irq_handler not existed, skip\n", __func__);
		return -ENOENT;
	}

	g_ir_det = kzalloc(sizeof(*g_ir_det), GFP_KERNEL);
	if (!g_ir_det)
		return -ENOMEM;

	ret = of_property_read_u32(chip_np, "gpio_irq", &g_ir_det->gpio);
	if (ret < 0) {
		hwlog_err("%s: read gpio failed, ret=%d\n", __func__, ret);
		ret = -ENOENT;
		goto err_out;
	}

	ret = of_property_read_u32(chip_np, "irq_flag", &g_ir_det->irq_flag);
	if (ret < 0) {
		hwlog_err("%s: read irq_type failed, ret=%d\n", __func__, ret);
		ret = -ENOENT;
		goto err_out;
	}

	ret = of_property_read_u32(chip_np, "gpio_out_place",
		&g_ir_det->gpio_out_place);
	if (ret < 0) {
		hwlog_err("%s: read gpio_out_place failed, ret=%d\n",
			__func__, ret);
		ret = -ENOENT;
		goto err_out;
	}
#ifdef CONFIG_HUAWEI_DSM
	irda_dsm_client_register();
#endif
	irda_detect_1st_read(g_ir_det->gpio, g_ir_det->gpio_out_place);
	ret = irda_irq_register();
	if (ret < 0)
		goto err_out;

	return 0;

err_out:
	kfree(g_ir_det);
	g_ir_det = NULL;
	return ret;
}

static int irda_remote_init(void)
{
	int ret = 0;
	int chip_type;

	irda_class = class_create(THIS_MODULE, "irda");
	if (IS_ERR(irda_class)) {
		ret = PTR_ERR(irda_class);
		hwlog_err("Failed to create irda class; ret:%d\n", ret);
		return ret;
	}

	chip_type = get_chip_type();
	if ((chip_type == HI11XX) || (chip_type == HI64XX))
		ret = irda_chip_type_regist();
	else
		hwlog_err("undefined chip type:%d\n", chip_type);

	if (ret < 0)
		goto free_class;

	ret = irda_detect_init();
	if (ret < 0)
		hwlog_err("unsupport irda detected function\n");

	return 0;

free_class:
	hwlog_err("Failed to init irda driver");
	class_destroy(irda_class);
	irda_class = NULL;
	return ret;
}

static void irda_remote_exit(void)
{
	irda_chip_type_unregist();
	class_destroy(irda_class);
	irda_class = NULL;
}

late_initcall(irda_remote_init);
module_exit(irda_remote_exit);

MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("debug for irda module driver");
