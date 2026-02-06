/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: analog headset switch driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "ana_hs_core.h"
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/workqueue.h>
#include <linux/io.h>
#include <huawei_platform/log/hw_log.h>
#include "huawei_platform/audio/ana_hs_extern_ops.h"

#define HWLOG_TAG ana_hs_i2c
HWLOG_REGIST();

#define REGS_CTRL_DELAY    1
#define RETRY_TIMES    3

static int ana_hs_regmap_read(struct regmap *regmap,
	unsigned int reg_addr, unsigned int *value)
{
	int ret = 0;
	int i;

	if (!regmap) {
		hwlog_err("%s regmap is null\n", __func__);
		return ret;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_read(regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(REGS_CTRL_DELAY);
	}
	hwlog_info("%s: reg 0x%x  value 0x%x\n", __func__, reg_addr, *value);
	return ret;
}

static int ana_hs_regmap_write(struct regmap *regmap,
	unsigned int reg_addr, unsigned int value)
{
	int ret = 0;
	int i;

	if (!regmap) {
		hwlog_err("%s regmap is null\n", __func__);
		return ret;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_write(regmap, reg_addr, value);
		if (ret == 0)
			break;

		mdelay(REGS_CTRL_DELAY);
	}
	hwlog_info("%s: reg 0x%x  value 0x%x\n", __func__, reg_addr, value);
	return ret;
}

static int ana_hs_regmap_update_bits(struct regmap *regmap,
	unsigned int reg_addr, unsigned int mask, unsigned int value)
{
	int ret = 0;
	int i;

	if (!regmap) {
		hwlog_err("%s regmap is null\n", __func__);
		return ret;
	}

	for (i = 0; i < RETRY_TIMES; i++) {
		ret = regmap_update_bits(regmap, reg_addr, mask, value);
		if (ret == 0)
			break;

		mdelay(REGS_CTRL_DELAY);
	}
	hwlog_info("%s: reg 0x%x value 0x%x mask 0x%x\n", __func__, reg_addr, value, mask);
	return ret;
}

static void ana_hs_i2c_free_regmap_cfg(struct ana_hs_regmap_cfg *cfg)
{
	if (cfg == NULL)
		return;

	ana_hs_kfree_ops(cfg->reg_writeable);
	ana_hs_kfree_ops(cfg->reg_unwriteable);
	ana_hs_kfree_ops(cfg->reg_readable);
	ana_hs_kfree_ops(cfg->reg_unreadable);
	ana_hs_kfree_ops(cfg->reg_volatile);
	ana_hs_kfree_ops(cfg->reg_unvolatile);
	ana_hs_kfree_ops(cfg->reg_defaults);

	kfree(cfg);
	cfg = NULL;
}

static unsigned int ana_hs_i2c_get_reg_value_mask(int val_bits)
{
	unsigned int mask;

	if (val_bits == ANA_HS_REG_VALUE_B16)
		mask = ANA_HS_REG_VALUE_M16;
	else if (val_bits == ANA_HS_REG_VALUE_B24)
		mask = ANA_HS_REG_VALUE_M24;
	else if (val_bits == ANA_HS_REG_VALUE_B32)
		mask = ANA_HS_REG_VALUE_M32;
	else /* ANA_HS_REG_VALUE_B8 or other */
		mask = ANA_HS_REG_VALUE_M8;

	return mask;
}

static int ana_hs_i2c_parse_reg_defaults(struct device_node *node,
	struct ana_hs_regmap_cfg *cfg_info)
{
	const char *reg_defaults_str = "reg_defaults";

	return ana_hs_get_u32_array(node, reg_defaults_str,
		(u32 **)&cfg_info->reg_defaults, &cfg_info->num_defaults);
}

static int ana_hs_i2c_parse_special_regs_range(struct device_node *node,
	struct ana_hs_regmap_cfg *cfg_info)
{
	const char *reg_writeable_str   = "reg_writeable";
	const char *reg_unwriteable_str = "reg_unwriteable";
	const char *reg_readable_str    = "reg_readable";
	const char *reg_unreadable_str  = "reg_unreadable";
	const char *reg_volatile_str    = "reg_volatile";
	const char *reg_unvolatile_str  = "reg_unvolatile";
	int ret;

	cfg_info->num_writeable   = 0;
	cfg_info->num_unwriteable = 0;
	cfg_info->num_readable    = 0;
	cfg_info->num_unreadable  = 0;
	cfg_info->num_volatile    = 0;
	cfg_info->num_unvolatile  = 0;
	cfg_info->num_defaults    = 0;

	ret = ana_hs_get_u32_array(node, reg_writeable_str,
		&cfg_info->reg_writeable, &cfg_info->num_writeable);
	ret += ana_hs_get_u32_array(node, reg_unwriteable_str,
		&cfg_info->reg_unwriteable, &cfg_info->num_unwriteable);
	ret += ana_hs_get_u32_array(node, reg_readable_str,
		&cfg_info->reg_readable, &cfg_info->num_readable);
	ret += ana_hs_get_u32_array(node, reg_unreadable_str,
		&cfg_info->reg_unreadable, &cfg_info->num_unreadable);
	ret += ana_hs_get_u32_array(node, reg_volatile_str,
		&cfg_info->reg_volatile, &cfg_info->num_volatile);
	ret += ana_hs_get_u32_array(node, reg_unvolatile_str,
		&cfg_info->reg_unvolatile, &cfg_info->num_unvolatile);
	ret += ana_hs_i2c_parse_reg_defaults(node, cfg_info);
	return ret;
}

static int ana_hs_i2c_parse_regmap_cfg(struct device_node *node,
	struct ana_hs_regmap_cfg **cfg)
{
	struct ana_hs_regmap_cfg *cfg_info = NULL;
	const char *reg_bits_str     = "reg_bits";
	const char *val_bits_str     = "val_bits";
	const char *cache_type_str   = "cache_type";
	const char *max_register_str = "max_register";
	int ret;

	cfg_info = kzalloc(sizeof(*cfg_info), GFP_KERNEL);
	if (cfg_info == NULL)
		return -ENOMEM;

	ret = of_property_read_u32(node, reg_bits_str,
		(u32 *)&cfg_info->cfg.reg_bits);
	if (ret < 0) {
		hwlog_err("%s: get reg_bits failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = of_property_read_u32(node, val_bits_str,
		(u32 *)&cfg_info->cfg.val_bits);
	if (ret < 0) {
		hwlog_err("%s: get val_bits failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	cfg_info->value_mask = ana_hs_i2c_get_reg_value_mask(
		cfg_info->cfg.val_bits);

	ret = of_property_read_u32(node, cache_type_str,
		(u32 *)&cfg_info->cfg.cache_type);
	if ((ret < 0) || (cfg_info->cfg.cache_type > REGCACHE_FLAT)) {
		hwlog_err("%s: get cache_type failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = of_property_read_u32(node, max_register_str,
		&cfg_info->cfg.max_register);
	if (ret < 0) {
		hwlog_err("%s: get max_register failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = ana_hs_i2c_parse_special_regs_range(node, cfg_info);
	if (ret < 0)
		goto err_out;

	*cfg = cfg_info;
	return 0;

err_out:
	ana_hs_i2c_free_regmap_cfg(cfg_info);
	return ret;
}

static int ana_hs_i2c_parse_irq_handler(struct device_node *node,
	struct ana_hs_irq_handler **handler)
{
	struct ana_hs_irq_handler *handler_info = NULL;
	const char *gpio_irq_str = "gpio_irq";
	const char *irq_flag_str = "irq_flag";
	const char *func_id_str = "func_id";
	int ret;

	handler_info = kzalloc(sizeof(*handler_info), GFP_KERNEL);
	if (handler_info == NULL)
		return -ENOMEM;

	handler_info->gpio = of_get_named_gpio(node, gpio_irq_str, 0);
	if (handler_info->gpio < 0) {
		hwlog_info("%s: get named irq gpio failed, %d\n", __func__,
			handler_info->gpio);
		ret = of_property_read_u32(node, gpio_irq_str,
			(u32 *)&handler_info->gpio);
		if (ret < 0) {
			hwlog_err("%s: read irq gpio failed, %d\n",
				__func__, ret);
			ret = -EFAULT;
			goto err_out;
		}
	}

	ret = of_property_read_u32(node, irq_flag_str,
		(u32 *)&handler_info->irq_flag);
	if (ret < 0) {
		hwlog_err("%s: get irq flag failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	ret = of_property_read_u32(node, func_id_str,
		(u32 *)&handler_info->func_id);
	if (ret < 0) {
		hwlog_err("%s: get irq func id failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}

	*handler = handler_info;
	return 0;

err_out:
	kfree(handler_info);
	handler_info = NULL;
	return ret;
}

static bool ana_hs_i2c_is_reg_in_special_range(unsigned int reg_addr,
	int num, unsigned int *reg)
{
	int i;

	if ((num == 0) || (reg == NULL)) {
		hwlog_err("%s: invalid arg\n", __func__);
		return false;
	}

	for (i = 0; i < num; i++) {
		if (reg[i] == reg_addr)
			return true;
	}
	return false;
}

static struct ana_hs_regmap_cfg *ana_hs_i2c_get_regmap_cfg(
	struct device *dev)
{
	struct ana_hs_i2c_data *i2c_priv = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return NULL;
	}
	i2c_priv = dev_get_drvdata(dev);
	if ((i2c_priv == NULL) || (i2c_priv->regmap_cfg == NULL)) {
		hwlog_err("%s: regmap_cfg invalid argument\n", __func__);
		return NULL;
	}
	return i2c_priv->regmap_cfg;
}

static bool ana_hs_i2c_writeable_reg(struct device *dev, unsigned int reg)
{
	struct ana_hs_regmap_cfg *cfg = NULL;

	cfg = ana_hs_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config writable or unwritable, can not config in dts at same time */
	if (cfg->num_writeable > 0)
		return ana_hs_i2c_is_reg_in_special_range(reg,
			cfg->num_writeable, cfg->reg_writeable);
	if (cfg->num_unwriteable > 0)
		return !ana_hs_i2c_is_reg_in_special_range(reg,
			cfg->num_unwriteable, cfg->reg_unwriteable);

	return true;
}

static bool ana_hs_i2c_readable_reg(struct device *dev, unsigned int reg)
{
	struct ana_hs_regmap_cfg *cfg = NULL;

	cfg = ana_hs_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config readable or unreadable, can not config in dts at same time */
	if (cfg->num_readable > 0)
		return ana_hs_i2c_is_reg_in_special_range(reg,
			cfg->num_readable, cfg->reg_readable);
	if (cfg->num_unreadable > 0)
		return !ana_hs_i2c_is_reg_in_special_range(reg,
			cfg->num_unreadable, cfg->reg_unreadable);

	return true;
}

static bool ana_hs_i2c_volatile_reg(struct device *dev, unsigned int reg)
{
	struct ana_hs_regmap_cfg *cfg = NULL;

	cfg = ana_hs_i2c_get_regmap_cfg(dev);
	if (cfg == NULL)
		return false;
	/* config volatile or unvolatile, can not config in dts at same time */
	if (cfg->num_volatile > 0)
		return ana_hs_i2c_is_reg_in_special_range(reg,
			cfg->num_volatile, cfg->reg_volatile);
	if (cfg->num_unvolatile > 0)
		return !ana_hs_i2c_is_reg_in_special_range(reg,
			cfg->num_unvolatile, cfg->reg_unvolatile);

	return true;
}

static int ana_hs_i2c_regmap_init(struct i2c_client *i2c,
	struct ana_hs_i2c_data *i2c_priv)
{
	const char *regmap_cfg_str = "regmap_cfg";
	struct ana_hs_regmap_cfg *cfg = NULL;
	struct device_node *node = NULL;
	int ret;
	int val_num;

	node = of_get_child_by_name(i2c->dev.of_node, regmap_cfg_str);
	if (node == NULL) {
		hwlog_debug("%s: regmap_cfg not existed, skip\n", __func__);
		return 0;
	}

	ret = ana_hs_i2c_parse_regmap_cfg(node, &i2c_priv->regmap_cfg);
	if (ret < 0)
		return ret;

	cfg = i2c_priv->regmap_cfg;
	val_num = sizeof(struct reg_default) / sizeof(unsigned int);
	if (cfg->num_defaults > 0) {
		if ((cfg->num_defaults % val_num) != 0) {
			hwlog_err("%s: reg_defaults %d%%%d != 0\n",
				__func__, cfg->num_defaults, val_num);
			ret = -EFAULT;
			goto err_out;
		}
	}

	/* set num_reg_defaults */
	if (cfg->num_defaults > 0) {
		cfg->num_defaults /= val_num;
		cfg->cfg.reg_defaults = cfg->reg_defaults;
		cfg->cfg.num_reg_defaults = (unsigned int)cfg->num_defaults;
	}

	cfg->cfg.writeable_reg = ana_hs_i2c_writeable_reg;
	cfg->cfg.readable_reg  = ana_hs_i2c_readable_reg;
	cfg->cfg.volatile_reg  = ana_hs_i2c_volatile_reg;

	cfg->regmap = regmap_init_i2c(i2c, &cfg->cfg);
	if (IS_ERR(cfg->regmap)) {
		hwlog_err("%s: regmap_init_i2c regmap failed\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	return 0;
err_out:
	ana_hs_i2c_free_regmap_cfg(i2c_priv->regmap_cfg);
	return ret;
}

static int ana_hs_i2c_irq_init(struct i2c_client *i2c,
	struct ana_hs_i2c_data *i2c_priv)
{
	const char *irq_handler_str = "irq_handler";
	struct device_node *node = NULL;
	irq_handler_t handler_func = NULL;
	int ret;

	node = of_get_child_by_name(i2c->dev.of_node, irq_handler_str);
	if (node == NULL) {
		hwlog_debug("%s: irq_handler not existed, skip\n", __func__);
		return 0;
	}

	ret = ana_hs_i2c_parse_irq_handler(node, &i2c_priv->handler);
	if (ret < 0)
		return ret;

	handler_func = ana_hs_irq_select_func(i2c_priv->handler->func_id);
	if (handler_func == NULL) {
		hwlog_err("%s ana_hs_irq_select_func is null\n", __func__);
		return 0;
	}

	i2c_priv->handler->irq = gpio_to_irq((unsigned int)i2c_priv->handler->gpio);
	hwlog_info("%s i2c_priv->handler->irq is %d\n", __func__, i2c_priv->handler->irq);

	ret = request_threaded_irq(i2c_priv->handler->irq, NULL,
		handler_func, i2c_priv->handler->irq_flag | IRQF_ONESHOT | IRQF_NO_SUSPEND,
		"ana_hs_irq", NULL);
	if (ret < 0) {
		hwlog_err("ana_hs_irq request fail, ret = %d\n", ret);
		goto err_out;
	}

	return 0;
err_out:
	kfree(i2c_priv->handler);
	i2c_priv->handler = NULL;
	return ret;
}

static int ana_hs_i2c_extern_init(struct i2c_client *i2c,
	struct ana_hs_i2c_data *i2c_priv)
{
	ana_hs_init_func init_func = NULL;
	const char *init_id_str = "init_id";
	int ret;

	ret = of_property_read_u32(i2c->dev.of_node, init_id_str,
		(u32 *)&i2c_priv->init_id);
	if (ret < 0) {
		hwlog_info("%s: read init id failed, %d\n",
			__func__, ret);
		ret = -EFAULT;
		goto err_out;
	}

	init_func = ana_hs_init_select_func(i2c_priv->init_id);
	if (init_func == NULL) {
		hwlog_err("%s ana_hs_init_select_func is null\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	init_func();

	return 0;
err_out:
	return ret;
}

struct ana_hs_bus_ctl_ops ana_hs_i2c_ctl_ops = {
	.read_regs   = ana_hs_regmap_read,
	.write_regs  = ana_hs_regmap_write,
	.update_bits = ana_hs_regmap_update_bits,
};

static int ana_hs_i2c_probe(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	struct ana_hs_i2c_data *data = NULL;
	int ret;

	if (ana_hs_get_misc_init_flag() == 0) {
		hwlog_err("%s: need probe defer\n", __func__);
		return -EPROBE_DEFER;
	}

	if (i2c == NULL) {
		hwlog_err("%s invalid argument\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s: device %s, addr = 0x%x, flags = 0x%x\n",
		__func__, id->name, i2c->addr, i2c->flags);

	if (!i2c_check_functionality(i2c->adapter, I2C_FUNC_I2C)) {
		hwlog_err("%s: i2c check functionality error\n", __func__);
		return -ENODEV;
	}

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		hwlog_err("%s: kzalloc data failed \n", __func__);
		return -ENOMEM;
	}

	data->dev = &i2c->dev;
	data->i2c = i2c;

	i2c_set_clientdata(i2c, data);
	dev_set_drvdata(&i2c->dev, data);

	ret = ana_hs_i2c_regmap_init(i2c, data);
	if (ret < 0) {
		hwlog_err("%s: regmap init fail\n", __func__);
		goto err_out;
	}

	ret = ana_hs_register_bus_ops(&ana_hs_i2c_ctl_ops, data);
	if (ret < 0) {
		hwlog_err("%s: registe bus ops fail\n", __func__);
		goto regmap_destroy;
	}

	ret = ana_hs_i2c_irq_init(i2c, data);
	if (ret < 0)
		hwlog_err("%s: irq init fail\n", __func__);

	ret = ana_hs_i2c_extern_init(i2c, data);
	if (ret < 0)
		hwlog_err("%s: extern init fail\n", __func__);
	return 0;

regmap_destroy:
	if (data->regmap_cfg != NULL) {
		regmap_exit(data->regmap_cfg->regmap);
		data->regmap_cfg->regmap = NULL;
		ana_hs_i2c_free_regmap_cfg(data->regmap_cfg);
	}
err_out:
	i2c_set_clientdata(i2c, NULL);
	dev_set_drvdata(&i2c->dev, NULL);
	kfree(data);
	data = NULL;

	return ret;
}

static int ana_hs_i2c_extern_remove(struct i2c_client *i2c,
	struct ana_hs_i2c_data *i2c_priv)
{
	ana_hs_remove_func remove_func = NULL;
	const char *remove_id_str = "remove_id";
	int ret;

	ret = of_property_read_u32(i2c->dev.of_node, remove_id_str,
		(u32 *)&i2c_priv->remove_id);
	if (ret < 0) {
		hwlog_info("%s: read remove id failed, %d\n",
			__func__, ret);
		ret = -EFAULT;
		goto err_out;
	}

	remove_func = ana_hs_remove_select_func(i2c_priv->remove_id);
	if (remove_func == NULL) {
		hwlog_err("%s ana_hs_remove_select_func is null\n", __func__);
		ret = -EFAULT;
		goto err_out;
	}
	remove_func();

	return 0;
err_out:
	return ret;
}

static int ana_hs_i2c_remove(struct i2c_client *i2c)
{
	struct ana_hs_regmap_cfg *cfg = NULL;
	struct ana_hs_i2c_data *data = NULL;

	if (i2c == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	data = i2c_get_clientdata(i2c);
	if (data == NULL) {
		hwlog_err("%s: data invalid\n", __func__);
		return -EINVAL;
	}

	ana_hs_i2c_extern_remove(i2c, data);

	i2c_set_clientdata(i2c, NULL);
	dev_set_drvdata(&i2c->dev, NULL);

	cfg = data->regmap_cfg;
	if (cfg != NULL) {
		regmap_exit(cfg->regmap);
		cfg->regmap = NULL;
		ana_hs_i2c_free_regmap_cfg(cfg);
	}

	kfree(data);

	return 0;
}

static void ana_hs_i2c_shutdown(struct i2c_client *i2c)
{
}

#ifdef CONFIG_PM
static int ana_hs_i2c_suspend(struct device *dev)
{
	struct ana_hs_i2c_data *data = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	data = dev_get_drvdata(dev);
	if (data == NULL)
		return 0;

	if ((data->regmap_cfg != NULL) &&
		(data->regmap_cfg->regmap != NULL) &&
		(data->regmap_cfg->cfg.cache_type == REGCACHE_RBTREE))
		regcache_cache_only(data->regmap_cfg->regmap, (bool)true);

	return 0;
}

static int ana_hs_i2c_resume(struct device *dev)
{
	struct ana_hs_i2c_data *data = NULL;

	if (dev == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		return -EINVAL;
	}

	data = dev_get_drvdata(dev);
	if (data == NULL)
		return 0;

	if ((data->regmap_cfg != NULL) &&
		(data->regmap_cfg->regmap != NULL) &&
		(data->regmap_cfg->cfg.cache_type == REGCACHE_RBTREE)) {
		regcache_cache_only(data->regmap_cfg->regmap, (bool)false);
		regcache_sync(data->regmap_cfg->regmap);
	}

	return 0;
}
#else
#define ana_hs_i2c_suspend NULL /* function pointer */
#define ana_hs_i2c_resume  NULL /* function pointer */
#endif /* CONFIG_PM */

static const struct dev_pm_ops ana_hs_i2c_pm_ops = {
	.suspend = ana_hs_i2c_suspend,
	.resume  = ana_hs_i2c_resume,
};

static const struct i2c_device_id ana_hs_i2c_id[] = {
	{ "ana_hs_i2c", 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, ana_hs_i2c_id);

static const struct of_device_id ana_hs_i2c_match[] = {
	{ .compatible = "huawei,ana_hs_i2c", },
	{},
};
MODULE_DEVICE_TABLE(of, ana_hs_i2c_match);

static struct i2c_driver ana_hs_i2c_driver = {
	.driver = {
		.name           = "ana_hs_i2c",
		.owner          = THIS_MODULE,
		.pm             = &ana_hs_i2c_pm_ops,
		.of_match_table = of_match_ptr(ana_hs_i2c_match),
	},
	.probe    = ana_hs_i2c_probe,
	.remove   = ana_hs_i2c_remove,
	.shutdown = ana_hs_i2c_shutdown,
	.id_table = ana_hs_i2c_id,
};

static int __init ana_hs_i2c_init(void)
{
	return i2c_add_driver(&ana_hs_i2c_driver);
}

static void __exit ana_hs_i2c_exit(void)
{
	i2c_del_driver(&ana_hs_i2c_driver);
}

subsys_initcall_sync(ana_hs_i2c_init);
module_exit(ana_hs_i2c_exit);

MODULE_DESCRIPTION("analog headset i2c driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
