/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 * Description: provide pinctrl  access function interfaces
 * Author: hisilicon
 * Create: 2019-11-28
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <hisi_usb_phy_chip.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <securec.h>
#include "core.h"
#include "devicetree.h"
#include "pinconf.h"
#include "pinmux.h"

#define DRIVER_NAME "pinctrl-hi6502"
#define PCH_OFF_DISABLED ~0U
#define PCH_HAS_PINCONF (pch->flags & PCH_FEAT_PINCONF)

/*
 * struct pch_func_vals - mux function register offset and value pair
 * @reg: register virtual address
 * @val: register value
 */
struct pch_func_vals {
	unsigned int reg;
	unsigned int val;
	unsigned int mask;
};

/*
 * struct pch_conf_vals - pinconf parameter, pinconf register offset
 * and value, enable, disable, mask
 * @param: config parameter
 * @val: user input bits in the pinconf register
 * @enable: enable bits in the pinconf register
 * @disable: disable bits in the pinconf register
 * @mask: mask bits in the register value
 */
struct pch_conf_vals {
	enum pin_config_param param;
	unsigned int val;
	unsigned int enable;
	unsigned int disable;
	unsigned int mask;
};

/*
 * struct pch_conf_type - pinconf property name, pinconf param pair
 * @name: property name in DTS file
 * @param: config parameter
 */
struct pch_conf_type {
	const char *name;
	enum pin_config_param param;
};

struct pch_conf_type prop2[] = {
	{ "pinctrl-hi6502,drive-strength", PIN_CONFIG_DRIVE_STRENGTH, },
	{ "pinctrl-hi6502,slew-rate", PIN_CONFIG_SLEW_RATE, },
	{ "pinctrl-hi6502,input-schmitt", PIN_CONFIG_INPUT_SCHMITT, },
	{ "pinctrl-hi6502,low-power-mode", PIN_CONFIG_LOW_POWER_MODE, },
};
struct pch_conf_type prop4[] = {
	{ "pinctrl-hi6502,bias-pullup", PIN_CONFIG_BIAS_PULL_UP, },
	{ "pinctrl-hi6502,bias-pulldown", PIN_CONFIG_BIAS_PULL_DOWN, },
	{ "pinctrl-hi6502,input-schmitt-enable",
		PIN_CONFIG_INPUT_SCHMITT_ENABLE, },
};
/*
 * struct pch_function - pinctrl function
 * @name: pinctrl function name
 * @vals: egister and vals array
 * @nvals: number of entries in vals array
 * @pgnames: array of pingroup names the function uses
 * @npgnames: number of pingroup names the function uses
 * @node: list node
 */
struct pch_function {
	const char *name;
	struct pch_func_vals *vals;
	unsigned int nvals;
	const char **pgnames;
	int npgnames;
	struct pch_conf_vals *conf;
	int nconfs;
	struct list_head node;
};

/*
 * struct pch_gpiofunc_range - pin ranges with same mux value of gpio function
 * @offset: offset base of pins
 * @npins: number pins with the same mux value of gpio function
 * @gpiofunc: mux value of gpio function
 * @node: list node
 */
struct pch_gpiofunc_range {
	unsigned int offset;
	unsigned int npins;
	unsigned int gpiofunc;
	struct list_head node;
};

/*
 * struct pch_data - wrapper for data needed by pinctrl framework
 * @pa: pindesc array
 * @cur: index to current element
 * REVISIT: We should be able to drop this eventually by adding
 * support for registering pins individually in the pinctrl
 * framework for those drivers that don't need a static array.
 */
struct pch_data {
	struct pinctrl_pin_desc *pa;
	int cur;
};

/*
 * struct pch_soc_data - SoC specific settings
 * @flags: initial SoC specific pch_FEAT_xxx values
 */
struct pch_soc_data {
	unsigned int flags;
};

struct pch_parse_info {
	struct pch_func_vals *vals;
	int rows;
	int *pins;
};
/*
 * struct pch_device - pinctrl device instance
 * @base: virtual address of the controller
 * @size:size of the ioremapped area
 * @dev:device entry
 * @np: device tree node
 * @pctl: pin controller device
 * @flags: mask of pch_FEAT_xxx values
 * @missing_nr_pinctrl_cells: for legacy binding, may go away
 * @socdata: soc specific data
 * @lock:spinlock for register access
 * @mutex: mutex protecting the lists
 * @width: bits per mux register
 * @fmask: function register mask
 * @fshift: function register shift
 * @fmax: max number of functions in fmask
 * @pins: physical pins on the SoC
 * @gpiofuncs: list of gpio functions
 * @desc: pin controller descriptor
 * @read: register read function to use
 * @write: register write function to use
 */
struct pch_device {
	unsigned int base;
	unsigned int size;
	struct device *dev;
	struct device_node *np;
	struct pinctrl_dev *pctl;
	unsigned int flags;
#define PCH_FEAT_PINCONF (1 << 0)
	struct property *missing_nr_pinctrl_cells;
	struct pch_soc_data socdata;
	raw_spinlock_t lock;
	struct mutex mutex;
	unsigned int width;
	unsigned int fmask;
	unsigned int fshift;
	unsigned int fmax;
	struct pch_data pins;
	struct list_head gpiofuncs;
	struct pinctrl_desc desc;
	unsigned int (*read)(unsigned int reg);
	void (*write)(unsigned int val, unsigned int reg);
};
struct pch_para {
	unsigned int i;
	unsigned int pin;
	u32 arg;
};

static int pch_pinconf_get(struct pinctrl_dev *pctldev, unsigned int pin,
	unsigned long *config);
static int pch_pinconf_set(struct pinctrl_dev *pctldev, unsigned int pin,
	unsigned long *configs, unsigned int num_configs);

static enum pin_config_param pch_bias[] = {
	PIN_CONFIG_BIAS_PULL_DOWN,
	PIN_CONFIG_BIAS_PULL_UP,
};

/*
 * REVISIT: Reads and writes could eventually use regmap or something
 * generic. But at least on omaps, some mux registers are performance
 * critical as they may need to be remuxed every time before and after
 * idle. Adding tests for register access width for every read and
 * write like regmap is doing is not desired, and caching the registers
 * does not help in this case.
 */

static unsigned int pch_readl(unsigned int reg)
{
	unsigned int  v = 0;

	hi6502_readl_u32(&v, reg);

	return v;
}

static void pch_writel(unsigned int val, unsigned int reg)
{
	hi6502_writel_u32(val, reg);

}

static void pch_pin_dbg_show(
	struct pinctrl_dev *pctldev, struct seq_file *s, unsigned int pin)
{
	struct pch_device *pch = NULL;
	unsigned int val, mux_bytes;
	unsigned int offset;
	size_t pa;
#if defined CONFIG_HISI_PINCRTL_INFO
	struct pch_gpiofunc_range *frange = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;
#endif
	pch = pinctrl_dev_get_drvdata(pctldev);
	if (!pch)
		return;

#if defined CONFIG_HISI_PINCRTL_INFO
	list_for_each_safe(pos, tmp, &pch->gpiofuncs) {
		frange = list_entry(
			pos, struct pch_gpiofunc_range, node);
		if (pin >= frange->offset + frange->npins ||
			pin < frange->offset)
			continue;

		mux_bytes = pch->width / BITS_PER_BYTE;
		val = (unsigned int)pch->read(
			pch->base + pin * mux_bytes);
		seq_printf(s, "%08x %s ", val, DRIVER_NAME);
		break;
	}
#else

	mux_bytes = pch->width / BITS_PER_BYTE;
	offset = pin * mux_bytes;
	val = pch->read(pch->base + offset);
	pa = pch->base + offset;

	seq_printf(s, "%zx %08x %s ", pa, val, DRIVER_NAME);
#endif
}

static void pch_dt_free_map(struct pinctrl_dev *pctldev,
	struct pinctrl_map *map, unsigned int num_maps)
{
	struct pch_device *pch = NULL;

	if (!pctldev || !map)
		return;
	pch = pinctrl_dev_get_drvdata(pctldev);
	if (!pch)
		return;

	devm_kfree(pch->dev, map);
}

static int pch_dt_node_to_map(struct pinctrl_dev *pctldev,
	struct device_node *np_config, struct pinctrl_map **map,
	unsigned int *num_maps);

static const struct pinctrl_ops pch_pinctrl_ops = {
	.get_groups_count = pinctrl_generic_get_group_count,
	.get_group_name = pinctrl_generic_get_group_name,
	.get_group_pins = pinctrl_generic_get_group_pins,
	.pin_dbg_show = pch_pin_dbg_show,
	.dt_node_to_map = pch_dt_node_to_map,
	.dt_free_map = pch_dt_free_map,
};

static int pch_get_function(struct pinctrl_dev *pctldev,
	unsigned int pin, struct pch_function **func)
{
	struct pch_device *pch = pinctrl_dev_get_drvdata(pctldev);
	struct pin_desc *pdesc = pin_desc_get(pctldev, pin);
	const struct pinctrl_setting_mux *setting = NULL;
	struct function_desc *function = NULL;
	unsigned int fselector;

	/* If pin is not described in DTS & enabled, mux_setting is NULL. */
	if (!pdesc)
		return -EINVAL;
	setting = pdesc->mux_setting;
	if (!setting)
		return -ENOTSUPP;
	fselector = setting->func;
	function = pinmux_generic_get_function(pctldev, fselector);
	*func = function->data;
	if (!(*func)) {
		dev_err(pch->dev, "%s could not find function%ui\n", __func__,
			fselector);
		return -ENOTSUPP;
	}
	return 0;
}

static int pch_set_mux(
	struct pinctrl_dev *pctldev, unsigned int fselector, unsigned int group)
{
	struct pch_device *pch = NULL;
	struct function_desc *function = NULL;
	struct pch_function *func = NULL;
	struct pch_func_vals *vals = NULL;
	int i;
	unsigned int val, mask;

	pch = pinctrl_dev_get_drvdata(pctldev);
	/* If function mask is null, needn't enable it. */
	if (!pch->fmask)
		return 0;

	function = pinmux_generic_get_function(pctldev, fselector);
	if (!function)
		return -EINVAL;
	func = function->data;
	if (!func)
		return -EINVAL;

	dev_err(pch->dev, "enabling %s function%u\n", func->name, fselector);

	for (i = 0; i < func->nvals; i++) {
		vals = &func->vals[i];
		pr_err("pch base is %x, , regoffset is %x\n", pch->base,
			(unsigned int)vals->reg);
		val = pch->read(vals->reg);

		mask = pch->fmask;
		val &= ~mask;
		val |= (vals->val & mask);
		pr_err("mask is %d, val is %d, regoffset is %u\n", mask, val,
			(unsigned int)vals->reg);
		pch->write(val, vals->reg);
	}

	return 0;
}

static int pch_request_gpio(struct pinctrl_dev *pctldev,
	struct pinctrl_gpio_range *range, unsigned int pin)
{
	struct pch_device *pch = pinctrl_dev_get_drvdata(pctldev);
	struct pch_gpiofunc_range *frange = NULL;
	struct list_head *pos = NULL;
	struct list_head *tmp = NULL;
	int mux_bytes;
	unsigned int data;

	/* If function mask is null, return directly. */
	if (!pch->fmask) {
		pr_err("Fmask is null!\n");
		return -ENOTSUPP;
	}

	list_for_each_safe(pos, tmp, &pch->gpiofuncs) {
		frange = list_entry(pos, struct pch_gpiofunc_range, node);
		if (pin >= frange->offset + frange->npins ||
			pin < frange->offset)
			continue;

		mux_bytes = pch->width / BITS_PER_BYTE;
		data = pch->read(pch->base + pin * mux_bytes) & ~pch->fmask;
		data |= frange->gpiofunc;
		pch->write(data, pch->base + pin * mux_bytes);
		break;
	}
	return 0;
}

static const struct pinmux_ops pch_pinmux_ops = {
	.get_functions_count = pinmux_generic_get_function_count,
	.get_function_name = pinmux_generic_get_function_name,
	.get_function_groups = pinmux_generic_get_function_groups,
	.set_mux = pch_set_mux,
	.gpio_request_enable = pch_request_gpio,
};

/* Clear BIAS value */
static void pch_pinconf_clear_bias(struct pinctrl_dev *pctldev,
	unsigned int pin)
{
	unsigned long config;
	int i;

	for (i = 0; i < ARRAY_SIZE(pch_bias); i++) {
		config = pinconf_to_config_packed(pch_bias[i], 0);
		pch_pinconf_set(pctldev, pin, &config, 1);
	}
}

/*
 * Check whether PIN_CONFIG_BIAS_DISABLE is valid
 * It's depend on that PULL_DOWN & PULL_UP configs are all invalid
 */
static bool pch_pinconf_bias_disable(struct pinctrl_dev *pctldev,
	unsigned int pin)
{
	unsigned long config;
	int i;

	for (i = 0; i < ARRAY_SIZE(pch_bias); i++) {
		config = pinconf_to_config_packed(pch_bias[i], 0);
		if (!pch_pinconf_get(pctldev, pin, &config))
			goto out;
	}
	return true;
out:
	return false;
}

static int pch_pinconf_get_param(struct pch_function *func,
	 unsigned long *config, unsigned int data,unsigned int i)
{
	unsigned int j;

	switch (func->conf[i].param) {
	/* 4 parameters */
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_UP:
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		if ((data != func->conf[i].enable) ||
			(data == func->conf[i].disable))
			return -ENOTSUPP;
		*config = 0;
		break;
	/* 2 parameters */
	case PIN_CONFIG_INPUT_SCHMITT:
		for (j = 0; j < func->nconfs; j++) {
			switch (func->conf[j].param) {
			case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
				if (data != func->conf[j].enable)
					return -ENOTSUPP;
				break;
			default:
				break;
			}
		}
		*config = data;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
	case PIN_CONFIG_SLEW_RATE:
	case PIN_CONFIG_LOW_POWER_MODE:
	default:
		*config = data;
		break;
	}
	return 0;
}

static int pch_pinconf_get(
	struct pinctrl_dev *pctldev, unsigned int pin, unsigned long *config)
{
	struct pch_device *pch = pinctrl_dev_get_drvdata(pctldev);
	struct pch_function *func = NULL;
	enum pin_config_param param;
	unsigned int offset, data, i, ret;

	ret = pch_get_function(pctldev, pin, &func);
	if (ret)
		return ret;

	for (i = 0; i < func->nconfs; i++) {
		param = pinconf_to_config_param(*config);
		if (param == PIN_CONFIG_BIAS_DISABLE) {
			if (pch_pinconf_bias_disable(pctldev, pin)) {
				*config = 0;
				return 0;
			} else {
				return -ENOTSUPP;
			}
		} else if (param != func->conf[i].param) {
			continue;
		}

		offset = pin * (pch->width / BITS_PER_BYTE);
		data = pch->read(pch->base + offset) & func->conf[i].mask;
		ret = pch_pinconf_get_param(func, config, data,i);

		if (!ret) {
			pr_err("pinconf set param failed!\n");
			return ret;
		}

		return 0;
	}
	return -ENOTSUPP;
}

static int pch_pinconf_set_param(struct pinctrl_dev *pctldev,
	struct pch_function *func,
	struct pch_para *para, unsigned int *pdata)
{
	unsigned int i , pin;
	unsigned int shift = 0;
	unsigned int data = 0;
	u32 arg;

	arg= para->arg;
	i = para->i;
	pin = para->pin;
	switch (func->conf[i].param) {
	/* 2 parameters */
	case PIN_CONFIG_INPUT_SCHMITT:
	case PIN_CONFIG_DRIVE_STRENGTH:
	case PIN_CONFIG_SLEW_RATE:
	case PIN_CONFIG_LOW_POWER_MODE:
		shift = ffs(func->conf[i].mask) - 1;
		data &= ~func->conf[i].mask;
		data |= (arg << shift) & func->conf[i].mask;
		pr_err("Func conf mask is %d, arg is %d\n",
			func->conf[i].mask, arg);
		break;
	/* 4 parameters */
	case PIN_CONFIG_BIAS_DISABLE:
		pr_err("Pin disable bias pin is %d\n", pin);
		pch_pinconf_clear_bias(pctldev, pin);
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_UP:
		if (arg) {
			pr_err("Pin disable bias arg is %d\n",
				arg);
			pch_pinconf_clear_bias(pctldev, pin);
		}
	/* fall through */
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		data &= ~func->conf[i].mask;
		if (arg)
			data |= func->conf[i].enable;
		else
			data |= func->conf[i].disable;
		break;
	default:
		return -ENOTSUPP;
	}

	*pdata = data;
	return 0;
}


static int pch_pinconf_set(struct pinctrl_dev *pctldev, unsigned int pin,
	unsigned long *configs, unsigned int num_configs)
{
	struct pch_device *pch = pinctrl_dev_get_drvdata(pctldev);
	struct pch_function *func = NULL;
	struct pch_para para = {0};
	unsigned int offset = 0, i, data, ret;
	u32 arg;
	int j;

	pr_err("PIN IS %d, num_configs %d\n", pin, num_configs);

	ret = pch_get_function(pctldev, pin, &func);
	if (ret) {
		pr_err("get pch func failed!\n");
		return ret;
	}

	pr_err("Func names is %s, nvals is %d, npgnames is %d, nconfs is %d\n",
		func->name, func->nvals, func->npgnames, func->nconfs);

	for (j = 0; j < num_configs; j++) {
		for (i = 0; i < func->nconfs; i++) {
			if (pinconf_to_config_param(configs[j]) !=
				func->conf[i].param)
				continue;

			pr_err("Func conf param is %d\n", func->conf[i].param);

			offset = pin * (pch->width / BITS_PER_BYTE);
			data = pch->read(pch->base + offset);
			arg = pinconf_to_config_argument(configs[j]);
			para.pin = pin;
			para.i = i;
			para.arg = arg;
			ret = pch_pinconf_set_param(pctldev, func,
				&para, &data);

			if (ret) {
				pr_err("pinconf set param failed!\n");
				return ret;
			}

			pch->write(data, pch->base + offset);

			break;
		}
		if (i >= func->nconfs)
			return -ENOTSUPP;
	}

	return 0;
}

static int pch_pinconf_group_get(
	struct pinctrl_dev *pctldev, unsigned int group, unsigned long *config)
{
	const unsigned int *pins = NULL;
	unsigned int npins, old = 0;
	int i, ret;

	ret = pinctrl_generic_get_group_pins(pctldev, group, &pins, &npins);
	if (ret)
		return ret;
	for (i = 0; i < npins; i++) {
		if (pch_pinconf_get(pctldev, pins[i], config))
			return -ENOTSUPP;
		/* configs do not match between two pins */
		if (i && (old != *config))
			return -ENOTSUPP;
		old = *config;
	}
	return 0;
}

static int pch_pinconf_group_set(struct pinctrl_dev *pctldev,
	unsigned int group, unsigned long *configs, unsigned int num_configs)
{
	const unsigned int *pins = NULL;
	unsigned int npins;
	int i, ret;

	ret = pinctrl_generic_get_group_pins(pctldev, group, &pins, &npins);
	if (ret)
		return ret;
	for (i = 0; i < npins; i++) {
		if (pch_pinconf_set(pctldev, pins[i], configs, num_configs))
			return -ENOTSUPP;
	}
	return 0;
}

static void pch_pinconf_dbg_show(
	struct pinctrl_dev *pctldev, struct seq_file *s, unsigned int pin)
{
}

static void pch_pinconf_group_dbg_show(
	struct pinctrl_dev *pctldev, struct seq_file *s, unsigned int selector)
{
}

static void pch_pinconf_config_dbg_show(
	struct pinctrl_dev *pctldev, struct seq_file *s, unsigned long config)
{
	pinconf_generic_dump_config(pctldev, s, config);
}

static const struct pinconf_ops pch_pinconf_ops = {
	.pin_config_get = pch_pinconf_get,
	.pin_config_set = pch_pinconf_set,
	.pin_config_group_get = pch_pinconf_group_get,
	.pin_config_group_set = pch_pinconf_group_set,
	.pin_config_dbg_show = pch_pinconf_dbg_show,
	.pin_config_group_dbg_show = pch_pinconf_group_dbg_show,
	.pin_config_config_dbg_show = pch_pinconf_config_dbg_show,
	.is_generic = true,
};

/*
 * pch_add_pin() - add a pin to the static per controller pin array
 * @pch: pch driver instance
 * @offset: register offset from base
 */
static int pch_add_pin(struct pch_device *pch)
{
	struct pinctrl_pin_desc *pin = NULL;
	int i;

	i = pch->pins.cur;
	if (i >= pch->desc.npins) {
		dev_err(pch->dev, "too many pins, max %i\n", pch->desc.npins);
		return -ENOMEM;
	}

	pin = &pch->pins.pa[i];
	pin->number = i;
	pch->pins.cur++;

	return i;
}

/*
 * pch_allocate_pin_table() - adds all the pins for the pinctrl driver
 * @pch: pch driver instance
 * If your hardware needs holes in the address space, then just set
 * up multiple driver instances
 */
static int pch_allocate_pin_table(struct pch_device *pch)
{
	int mux_bytes, nr_pins, i, res;

	mux_bytes = pch->width / BITS_PER_BYTE;
	nr_pins = pch->size / mux_bytes;

	dev_dbg(pch->dev, "allocating %i pins\n", nr_pins);
	pch->pins.pa = devm_kzalloc(
		pch->dev, sizeof(*pch->pins.pa) * nr_pins, GFP_KERNEL);
	if (!pch->pins.pa)
		return -ENOMEM;

	pch->desc.pins = pch->pins.pa;
	pch->desc.npins = nr_pins;

	for (i = 0; i < pch->desc.npins; i++) {

		res = pch_add_pin(pch);
		if (res < 0) {
			dev_err(pch->dev, "error adding pins: %i\n", res);
			return res;
		}
	}

	return 0;
}

/*
 * pch_add_function() - adds a new function to the function list
 * @pch: pch driver instance
 * @np: device node of the mux entry
 * @name: name of the function
 * @vals: array of mux register value pairs used by the function
 * @nvals: number of mux register value pairs
 * @pgnames: array of pingroup names for the function
 * @npgnames: number of pingroup names
 */
static struct pch_function *pch_add_function(struct pch_device *pch,
	const char *name, struct pch_func_vals *vals,
	unsigned int nvals, const char **pgnames)
{
	struct pch_function *function = NULL;
	int res;

	function = devm_kzalloc(pch->dev, sizeof(*function), GFP_KERNEL);
	if (!function)
		return NULL;

	function->vals = vals;
	function->nvals = nvals;

	res = pinmux_generic_add_function(
		pch->pctl, name, pgnames, 1, function);
	if (res)
		return NULL;

	return function;
}

/*
 * pch_get_pin_by_offset() - get a pin index based on the register offset
 * @pch: pch driver instance
 * @offset: register offset from the base
 * Note that this is OK as long as the pins are in a static array
 */
static int pch_get_pin_by_offset(struct pch_device *pch, unsigned int offset)
{
	unsigned int index;

	if (offset >= pch->size) {
		dev_err(pch->dev, "mux offset out of range: 0x%x (0x%x)\n",
			offset, pch->size);
		return -EINVAL;
	}

	index = offset / (pch->width / BITS_PER_BYTE);

	return index;
}

/*
 * check whether data matches enable bits or disable bits
 * Return value: 1 for matching enable bits, 0 for matching disable bits,
 *               and negative value for matching failure.
 */
static int pch_config_match(unsigned int data, unsigned int enable,
	unsigned int disable)
{
	int ret = -EINVAL;

	if (data == enable)
		ret = 1;
	else if (data == disable)
		ret = 0;
	return ret;
}

static void add_config(struct pch_conf_vals **conf,
	struct pch_conf_vals cfparam)
{
	(*conf)->param = cfparam.param;
	(*conf)->val = cfparam.val;
	(*conf)->enable = cfparam.enable;
	(*conf)->disable = cfparam.disable;
	(*conf)->mask = cfparam.mask;
	(*conf)++;
}

static void add_setting(
	unsigned long **setting, enum pin_config_param param, unsigned int arg)
{
	**setting = pinconf_to_config_packed(param, arg);
	(*setting)++;
}

/* add pinconf setting with 2 parameters */
static void pch_add_conf2(struct pch_device *pch, struct device_node *np,
	struct pch_conf_type *pchcnf,
	struct pch_conf_vals **conf, unsigned long **settings)
{
	unsigned int value[2] = {0};
	unsigned int shift;
	int ret;
	struct pch_conf_vals cfparam;

	ret = of_property_read_u32_array(np, pchcnf->name, value, 2);
	if (ret)
		return;
	/* set value & mask */
	value[0] &= value[1];
	shift = ffs(value[1]) - 1;
	/* skip enable & disable */
	cfparam.param = pchcnf->param;
	cfparam.val = value[0];
	cfparam.enable = 0;
	cfparam.disable = 0;
	cfparam.mask = value[1];
	add_config(conf, cfparam);
	add_setting(settings, pchcnf->param, value[0]>>shift);
}

/* add pinconf setting with 4 parameters */
static void pch_add_conf4(struct pch_device *pch, struct device_node *np,
	struct pch_conf_type *pchcnf,
	struct pch_conf_vals **conf, unsigned long **settings)
{
	unsigned int value[4] = {0};
	int ret;
	struct pch_conf_vals cfparam;

	/* value to set, enable, disable, mask */
	ret = of_property_read_u32_array(np, pchcnf->name, value, 4);
	if (ret)
		return;
	if (!value[3]) {
		dev_err(pch->dev, "mask field of the property can't be 0\n");
		return;
	}
	value[0] &= value[3];
	value[1] &= value[3];
	value[2] &= value[3];
	ret = pch_config_match(value[0], value[1], value[2]);
	if (ret < 0)
		dev_dbg(pch->dev, "failed to match enable or disable bits\n");

	cfparam.param = pchcnf->param;
	cfparam.val = value[0];
	cfparam.enable = 0;
	cfparam.disable = 0;
	cfparam.mask = value[1];
	add_config(conf, cfparam);
	add_setting(settings, pchcnf->param, ret);
}

static int pch_parse_pinconf(struct pch_device *pch, struct device_node *np,
	struct pch_function *func, struct pinctrl_map **map)

{
	struct pinctrl_map *m = *map;
	int i, nconfs = 0;
	unsigned long *settings = NULL, *s = NULL;
	struct pch_conf_vals *conf = NULL;

	/* If pinconf isn't supported, don't parse properties in below. */
	if (!PCH_HAS_PINCONF)
		return 0;

	/* cacluate how much properties are supported in current node */
	for (i = 0; i < ARRAY_SIZE(prop2); i++) {
		if (of_find_property(np, prop2[i].name, NULL))
			nconfs++;
	}
	for (i = 0; i < ARRAY_SIZE(prop4); i++) {
		if (of_find_property(np, prop4[i].name, NULL))
			nconfs++;
	}
	if (!nconfs)
		return 0;

	func->conf = devm_kzalloc(
		pch->dev, sizeof(struct pch_conf_vals) * nconfs, GFP_KERNEL);
	if (!func->conf)
		return -ENOMEM;
	func->nconfs = nconfs;
	conf = &(func->conf[0]);
	m++;
	settings = devm_kzalloc(
		pch->dev, sizeof(unsigned long) * nconfs, GFP_KERNEL);
	if (!settings)
		return -ENOMEM;
	s = &settings[0];

	for (i = 0; i < ARRAY_SIZE(prop2); i++)
		pch_add_conf2(
			pch, np, &prop2[i], &conf, &s);
	for (i = 0; i < ARRAY_SIZE(prop4); i++)
		pch_add_conf4(
			pch, np, &prop4[i], &conf, &s);
	m->type = PIN_MAP_TYPE_CONFIGS_GROUP;
	m->data.configs.group_or_pin = np->name;
	m->data.configs.configs = settings;
	m->data.configs.num_configs = nconfs;
	return 0;
}
static int pch_parse_vals_pins(struct pch_device *pch, struct device_node *np,
	int rows, struct pch_func_vals *vals, int *pins)
{
	int ret = 0, found = 0;
	int i;
	const char *name = "pinctrl-hi6502,pins";

	for (i = 0; i < rows; i++) {
		struct of_phandle_args pinctrl_spec;
		unsigned int offset;
		int pin;

		ret = pinctrl_parse_index_with_args(np, name, i, &pinctrl_spec);
		if (ret) {
			dev_err(pch->dev, "pch parse args failed\n");
			return -ENOMEM;
		}

		if (pinctrl_spec.args_count < 2) {
			dev_err(pch->dev, "invalid args_count for spec: %i\n",
				pinctrl_spec.args_count);
			break;
		}

		/* Index plus one value cell */
		offset = pinctrl_spec.args[0];
		pr_err("%s offset is %d base is %d\n", __func__,
			offset, pch->base);
		vals[found].reg = pch->base + offset;
		pr_err("%s offset reg %d\n", __func__,
			vals[found].reg);
		vals[found].val = pinctrl_spec.args[1];
		pr_err("%s val val is %d\n", __func__,
			vals[found].val);

		dev_dbg(pch->dev, "%s index: 0x%x value: 0x%x\n",
			pinctrl_spec.np->name, offset, pinctrl_spec.args[1]);

		pin = pch_get_pin_by_offset(pch, offset);
		pr_err("%s get pin offset is %d\n", __func__,
			pin);
		if (pin < 0) {
			dev_err(pch->dev,
				"could not add functions for %s %ux\n",
				np->name, offset);
			break;
		}
		pins[found++] = pin;
	}

	return found;
}
/*
 * smux_parse_one_pinctrl_entry() - parses a device tree mux entry
 * @pctldev: pin controller device
 * @pch: pinctrl driver instance
 * @np: device node of the mux entry
 * @map: map entry
 * @num_maps: number of map
 * @pgnames: pingroup names
 */
static int pch_parse_one_pinctrl_entry(struct pch_device *pch,
	struct device_node *np, struct pinctrl_map **map,
	unsigned int *num_maps, const char **pgnames)
{
	const char *name = "pinctrl-hi6502,pins";
	struct pch_func_vals *vals = NULL;
	int rows, *pins = NULL, found = 0, res = -ENOMEM;
	struct pch_function *function = NULL;

	rows = pinctrl_count_index_with_args(np, name);
	if (rows <= 0)
		return -EINVAL;

	vals = devm_kzalloc(pch->dev, sizeof(*vals) * rows, GFP_KERNEL);
	if (!vals)
		return -ENOMEM;

	pins = devm_kzalloc(pch->dev, sizeof(*pins) * rows, GFP_KERNEL);
	if (!pins)
		goto free_vals;

	found = pch_parse_vals_pins(pch, np, rows, vals, pins);
	if (found < 0)
		return found;

	pgnames[0] = np->name;
	function = pch_add_function(pch, np->name, vals, found, pgnames);
	if (!function) {
		res = -ENOMEM;
		goto free_pins;
	}

	res = pinctrl_generic_add_group(pch->pctl, np->name, pins, found, pch);
	if (res < 0)
		goto free_function;

	(*map)->type = PIN_MAP_TYPE_MUX_GROUP;
	(*map)->data.mux.group = np->name;
	(*map)->data.mux.function = np->name;

	if (PCH_HAS_PINCONF) {
		res = pch_parse_pinconf(pch, np, function, map);
		if (res)
			goto free_pingroups;
		*num_maps = 2;
	} else {
		*num_maps = 1;
	}
	return 0;

free_pingroups:
	pinctrl_generic_remove_last_group(pch->pctl);
	*num_maps = 1;
free_function:
	pinmux_generic_remove_last_function(pch->pctl);
free_pins:
	devm_kfree(pch->dev, pins);
free_vals:
	devm_kfree(pch->dev, vals);

	return res;
}

/*
 * pch_dt_node_to_map() - allocates and parses pinctrl maps
 * @pctldev: pinctrl instance
 * @np_config: device tree pinmux entry
 * @map: array of map entries
 * @num_maps: number of maps
 */
static int pch_dt_node_to_map(struct pinctrl_dev *pctldev,
	struct device_node *np_config, struct pinctrl_map **map,
	unsigned int *num_maps)
{
	struct pch_device *pch = NULL;
	const char **pgnames = NULL;
	int ret;

	pch = pinctrl_dev_get_drvdata(pctldev);

	/* create 2 maps. One is for pinmux, and the other is for pinconf. */
	*map = devm_kzalloc(pch->dev, sizeof(**map) * 2, GFP_KERNEL);
	if (!*map)
		return -ENOMEM;

	*num_maps = 0;

	pgnames = devm_kzalloc(pch->dev, sizeof(*pgnames), GFP_KERNEL);
	if (!pgnames) {
		ret = -ENOMEM;
		goto free_map;
	}

	ret = pch_parse_one_pinctrl_entry(
		pch, np_config, map, num_maps, pgnames);
	if (ret < 0) {
		dev_err(pch->dev, "no pins entries for %s\n", np_config->name);
		goto free_pgnames;
	}

	return 0;

free_pgnames:
	devm_kfree(pch->dev, pgnames);
free_map:
	devm_kfree(pch->dev, *map);

	return ret;
}

/*
 * pch_free_resources() - free memory used by this driver
 * @pch: pch driver instance
 */
static void pch_free_resources(struct pch_device *pch)
{
	pinctrl_unregister(pch->pctl);

	if (pch->missing_nr_pinctrl_cells)
		of_remove_property(pch->np, pch->missing_nr_pinctrl_cells);
}

static int pch_add_gpio_func(struct device_node *node, struct pch_device *pch)
{
	const char *propname = "pinctrl-hi6502,gpio-range";
	const char *cellname = "#pinctrl-hi6502,gpio-range-cells";
	struct of_phandle_args gpiospec;
	struct pch_gpiofunc_range *range = NULL;
	int ret, i;

	for (i = 0;; i++) {
		ret = of_parse_phandle_with_args(
			node, propname, cellname, i, &gpiospec);
		/* Do not treat it as error. Only treat it as end condition. */
		if (ret) {
			ret = 0;
			break;
		}
		range = devm_kzalloc(pch->dev, sizeof(*range), GFP_KERNEL);
		if (!range) {
			ret = -ENOMEM;
			break;
		}
		range->offset = gpiospec.args[0];
		range->npins = gpiospec.args[1];
		range->gpiofunc = gpiospec.args[2];
		mutex_lock(&pch->mutex);
		list_add_tail(&range->node, &pch->gpiofuncs);
		mutex_unlock(&pch->mutex);
	}
	return ret;
}

#ifdef CONFIG_PM
static int pinctrl_hi6502_suspend(
	struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int pinctrl_hi6502_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

/*
 * pch_quirk_missing_pinctrl_cells - handle legacy binding
 * @pch: pinctrl driver instance
 * @np: device tree node
 * @cells: number of cells
 * Handle legacy binding with no #pinctrl-cells
 */
static int pch_quirk_missing_pinctrl_cells(
	struct pch_device *pch, struct device_node *np, int cells)
{
	struct property *p = NULL;
	const char *name = "#pinctrl-cells";
	int error;
	u32 val;

	error = of_property_read_u32(np, name, &val);
	if (!error)
		return 0;

	dev_warn(pch->dev, "please update dts to use %s = <%i>\n", name, cells);

	p = devm_kzalloc(pch->dev, sizeof(*p), GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	p->length = sizeof(__be32);
	p->value = devm_kzalloc(pch->dev, sizeof(__be32), GFP_KERNEL);
	if (!p->value)
		return -ENOMEM;
	*(__be32 *)p->value = cpu_to_be32(cells);

	p->name = devm_kstrdup(pch->dev, name, GFP_KERNEL);
	if (!p->name)
		return -ENOMEM;

	pch->missing_nr_pinctrl_cells = p;

	error = of_add_property(np, pch->missing_nr_pinctrl_cells);

	return error;
}
static int get_pinctrl_hi6502_device_data(struct device_node *np,
	struct pch_device *pch)
{
	int ret, reg_prop_cnt;
	u32 *reg_prop = NULL;

	ret = of_property_read_u32(
		np, "pinctrl-hi6502,register-width", &pch->width);
	if (ret) {
		pr_err("register width not specified\n");
		return ret;
	}

	ret = of_property_read_u32(
		np, "pinctrl-hi6502,function-mask", &pch->fmask);
	if (!ret) {
		pch->fshift = __ffs(pch->fmask);
		pch->fmax = pch->fmask >> pch->fshift;
	} else {
		/* If mask property doesn't exist, function mux is invalid. */
		pch->fmask = 0;
		pch->fshift = 0;
		pch->fmax = 0;
	}

	reg_prop_cnt = of_property_count_u32_elems(np, "reg");
	if (reg_prop_cnt < 0) {
		pr_err("register reg not specified\n");
		return -1;
	}

	reg_prop = kmalloc_array(reg_prop_cnt, sizeof(*reg_prop), GFP_KERNEL);
	if(!reg_prop){
		pr_err("kmalloc_array failed\n");
		return -1;
	}
	ret = of_property_read_u32_array(np, "reg", reg_prop, reg_prop_cnt);
	if (ret) {
		pr_err("register reg base not specified\n");
		kfree(reg_prop);
		return ret;
	}

	/* Hi6502 not in soc address space, needn't get ioremap */
	pch->size = reg_prop[3]; /* get from node size */
	pch->base = reg_prop[1]; /* get from node base addr */
	ret = pch_quirk_missing_pinctrl_cells(pch, np, 1);
	if (ret) {
		pr_err("unable to patch #pinctrl-cells\n");
		kfree(reg_prop);
		return ret;
	}
	kfree(reg_prop);
	return ret;
}
static int pch_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct pch_device *pch = NULL;
	const struct pch_soc_data *soc = NULL;
	int ret, ret1;

	soc = of_device_get_match_data(&pdev->dev);
	if (WARN_ON(!soc))
		return -EINVAL;

	pch = devm_kzalloc(&pdev->dev, sizeof(*pch), GFP_KERNEL);
	if (!pch)
		return -ENOMEM;

	pch->dev = &pdev->dev;
	pch->np = np;
	raw_spin_lock_init(&pch->lock);
	mutex_init(&pch->mutex);
	INIT_LIST_HEAD(&pch->gpiofuncs);
	pch->flags = soc->flags;

	ret = get_pinctrl_hi6502_device_data(np, pch);
	if (ret) {
		dev_err(&pdev->dev, "Error get hi6502 pinctrl dts\n");
		return ret;
	}

	platform_set_drvdata(pdev, pch);

	pch->read = pch_readl;
	pch->write = pch_writel;
	pch->desc.name = DRIVER_NAME;
	pch->desc.pctlops = &pch_pinctrl_ops;
	pch->desc.pmxops = &pch_pinmux_ops;
	if (PCH_HAS_PINCONF)
		pch->desc.confops = &pch_pinconf_ops;
	pch->desc.owner = THIS_MODULE;

	ret = pch_allocate_pin_table(pch);
	if (ret < 0)
		goto free;

	ret = pinctrl_register_and_init(&pch->desc, pch->dev, pch, &pch->pctl);
	if (ret) {
		dev_err(pch->dev, "could not register single pinctrl driver\n");
		goto free;
	}

	ret = pch_add_gpio_func(np, pch);
	if (ret < 0)
		goto free;

	ret1 = pinctrl_enable(pch->pctl);
	return ret1;

free:
	pch_free_resources(pch);

	return ret;
}

static int pch_remove(struct platform_device *pdev)
{
	struct pch_device *pch = platform_get_drvdata(pdev);

	if (!pch)
		return 0;

	pch_free_resources(pch);

	return 0;
}

static const struct pch_soc_data pinctrl_hi6502 = {};

static const struct pch_soc_data pinconf_hi6502 = {
	.flags = PCH_FEAT_PINCONF,
};

static const struct of_device_id pch_of_match[] = {
	{ .compatible = "pinctrl-hi6502", .data = &pinctrl_hi6502 },
	{ .compatible = "pinconf-hi6502", .data = &pinconf_hi6502 },
	{ },
};
MODULE_DEVICE_TABLE(of, pch_of_match);

static struct platform_driver pch_driver = {
	.probe = pch_probe,
	.remove = pch_remove,
	.driver = {
	.name = DRIVER_NAME,
	.of_match_table = pch_of_match,
	},
#ifdef CONFIG_PM
	.suspend = pinctrl_hi6502_suspend,
	.resume = pinctrl_hi6502_resume,
#endif
};

static int __init pinctrl_hi6502_init(void)
{
	return platform_driver_register(&pch_driver);
}
static void __exit pinctrl_hi6502_exit(void)
{
	platform_driver_unregister(&pch_driver);
}
arch_initcall(pinctrl_hi6502_init);
module_exit(pinctrl_hi6502_exit);

MODULE_DESCRIPTION(
	"One-register-per-pin type device tree based pinctrl driver");
MODULE_LICENSE("GPL v2");
