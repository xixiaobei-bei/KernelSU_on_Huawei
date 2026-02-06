/*
 * Copyright (c) Hisilicon technologies Co., Ltd.2013-2019. All rights reserved.
 * Description: provide regulator functions
 * Create: 2013-03-17
 */
#include <linux/mfd/hisi_pmic.h>
#ifdef CONFIG_HISI_PMIC_DEBUG
#include <linux/debugfs.h>
#include "regulator_debug.h"
#endif
#include <linux/regulator/of_regulator.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <securec.h>
#include <linux/version.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/hisi-spmi.h>
#include <linux/of_hisi_spmi.h>
#include <pr_log.h>

#define PR_LOG_TAG PMIC_REGULATOR_TAG

#define BRAND_DEBUG(args...) pr_debug(args)

#define STATIC static

#define TIME_CHANGE_US_MS 1000

struct regulator_register_info {
	u32 ctrl_reg;
	u32 enable_mask;
	u32 eco_mode_mask;
	u32 vset_reg;
	u32 vset_mask;
};

struct spmi_regulator {
	const char *name;
	struct regulator_register_info register_info;
	struct timeval last_off_time;
	u32 off_on_delay;
	u32 eco_uA;
	struct regulator_desc rdesc;
	int (*dt_parse)(struct spmi_regulator *, struct spmi_device *);
};
static DEFINE_MUTEX(enable_mutex);
static struct timeval g_last_enabled;

static inline struct vendor_pmic *rdev_to_pmic(struct regulator_dev *dev)
{
	/*
	 * regulator_dev parent to->
	 * regulator platform device_dev parent to->
	 * pmic platform device_dev
	 */

	if (rdev_get_dev(dev) == NULL)
		return NULL;

	return dev_get_drvdata(rdev_get_dev(dev)->parent->parent);
}
/*
 * helper function to ensure when it returns it is at least 'delay_us'
 * microseconds after 'since'.
 */
static void ensured_time_after(struct timeval since, u32 delay_us)
{
	struct timeval now;
	u64 elapsed_ns64, delay_ns64;
	u32 actual_us32;

	delay_ns64 = delay_us * NSEC_PER_USEC;
	do_gettimeofday(&now);
	elapsed_ns64 = timeval_to_ns(&now) - timeval_to_ns(&since);
	if (delay_ns64 > elapsed_ns64) {
		actual_us32 = ((u32)(delay_ns64 - elapsed_ns64) /
							NSEC_PER_USEC);
		if (actual_us32 >= TIME_CHANGE_US_MS) {
			mdelay((u32)(actual_us32 / TIME_CHANGE_US_MS));
			udelay(actual_us32 % TIME_CHANGE_US_MS);
		} else if (actual_us32 > 0) {
			udelay(actual_us32);
		}
	}
}

static int spmi_regulator_is_enabled(struct regulator_dev *dev)
{
	u32 reg_val;
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);

	if (!sreg)
		return 0;

	reg_val = main_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x,enable_state=%d>\n", __func__,
			sreg->register_info.ctrl_reg,
			(reg_val & sreg->register_info.enable_mask));
	return ((reg_val & sreg->register_info.enable_mask) != 0);
}

static int spmi_regulator_enable(struct regulator_dev *dev)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);

	if (!sreg)
		return -EINVAL;

	/* keep a distance of off_on_delay from last time disabled */
	ensured_time_after(sreg->last_off_time, sreg->off_on_delay);

	BRAND_DEBUG("<[%s]: off_on_delay = %d us>\n", __func__, sreg->off_on_delay);

	/* cannot enable more than one regulator at one time */
	mutex_lock(&enable_mutex);
	ensured_time_after(g_last_enabled, PMIC_REGS_ENA_PROTECT_TIME);

	/* set enable register */
	hisi_pmic_rmw(pmic, sreg->register_info.ctrl_reg,
				sreg->register_info.enable_mask,
				sreg->register_info.enable_mask);
	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x,enable_mask=0x%x>\n",
			__func__, sreg->register_info.ctrl_reg,
			sreg->register_info.enable_mask);

	do_gettimeofday(&g_last_enabled);
	mutex_unlock(&enable_mutex);

	return 0;
}

static int spmi_regulator_disable(struct regulator_dev *dev)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);

	if (!sreg)
		return -EINVAL;
	/* set enable register to 0 */
	hisi_pmic_rmw(pmic, sreg->register_info.ctrl_reg,
				sreg->register_info.enable_mask, 0);

	do_gettimeofday(&sreg->last_off_time);

	return 0;
}

static int spmi_regulator_get_voltage(struct regulator_dev *dev)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val;
	u32 selector;

	if (!sreg)
		return -EINVAL;

	/* get voltage selector */
	reg_val = main_pmic_read(pmic, sreg->register_info.vset_reg);
	BRAND_DEBUG("<[%s]: vset_reg = 0x%x>\n", __func__,
				sreg->register_info.vset_reg);

	selector = (reg_val & sreg->register_info.vset_mask) >>
			((unsigned int)ffs(sreg->register_info.vset_mask) - 1);

	return sreg->rdesc.ops->list_voltage(dev, selector);
}

static int spmi_regulator_set_voltage(struct regulator_dev *dev,
				int min_uv, int max_uv, unsigned int *selector)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);
	u32 vsel;
	int uv;

	if (!sreg)
		return -EINVAL;

	for (vsel = 0; vsel < sreg->rdesc.n_voltages; vsel++) {
		uv = sreg->rdesc.volt_table[vsel];
		/* Break at the first in-range value */
		if (min_uv <= uv && uv <= max_uv)
			break;
	}

	/* unlikely to happen. sanity test done by regulator core */
	if (unlikely(vsel == sreg->rdesc.n_voltages))
		return -EINVAL;

	*selector = vsel;
	/* set voltage selector */
	hisi_pmic_rmw(pmic, sreg->register_info.vset_reg,
		sreg->register_info.vset_mask,
		vsel << ((unsigned int)ffs(sreg->register_info.vset_mask) - 1));

	BRAND_DEBUG("<[%s]: vset_reg=0x%x, vset_mask=0x%x, value=0x%x>\n",
			__func__, sreg->register_info.vset_reg,
			sreg->register_info.vset_mask,
			vsel << (ffs(sreg->register_info.vset_mask) - 1));

	return 0;
}

static unsigned int spmi_regulator_get_mode(struct regulator_dev *dev)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);
	u32 reg_val;

	if (!sreg)
		return 0;

	reg_val = main_pmic_read(pmic, sreg->register_info.ctrl_reg);
	BRAND_DEBUG("<[%s]: reg_val=%d, ctrl_reg=0x%x, eco_mode_mask=0x%x>\n",
			__func__, reg_val,
			sreg->register_info.ctrl_reg,
			sreg->register_info.eco_mode_mask);

	if (reg_val & sreg->register_info.eco_mode_mask)
		return REGULATOR_MODE_IDLE;
	else
		return REGULATOR_MODE_NORMAL;
}

static int spmi_regulator_set_mode(struct regulator_dev *dev,
						unsigned int mode)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);
	struct vendor_pmic *pmic = rdev_to_pmic(dev);
	u32 eco_mode;

	if (!sreg)
		return -EINVAL;

	switch (mode) {
	case REGULATOR_MODE_NORMAL:
		eco_mode = PMIC_ECO_MODE_DISABLE;
		break;
	case REGULATOR_MODE_IDLE:
		eco_mode = PMIC_ECO_MODE_ENABLE;
		break;
	default:
		return -EINVAL;
	}

	/* set mode */
	hisi_pmic_rmw(pmic, sreg->register_info.ctrl_reg,
		sreg->register_info.eco_mode_mask,
		eco_mode << ((unsigned int)ffs(sreg->register_info.eco_mode_mask) - 1));

	BRAND_DEBUG("<[%s]: ctrl_reg=0x%x, eco_mode_mask=0x%x, value=0x%x>\n",
		__func__, sreg->register_info.ctrl_reg,
		sreg->register_info.eco_mode_mask,
		eco_mode << (ffs(sreg->register_info.eco_mode_mask) - 1));
	return 0;
}

static unsigned int spmi_regulator_get_optimum_mode(struct regulator_dev *dev,
			int input_uV, int output_uV, int load_uA)
{
	struct spmi_regulator *sreg = rdev_get_drvdata(dev);

	if (!sreg)
		return 0;

	if ((load_uA == 0) || ((unsigned int)load_uA > sreg->eco_uA))
		return REGULATOR_MODE_NORMAL;
	else
		return REGULATOR_MODE_IDLE;
}

static int spmi_dt_parse_common(struct spmi_regulator *sreg,
					struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_desc *rdesc = &sreg->rdesc;
	unsigned int register_info[3] = {0}; /* the length is determined by dts parameter */
	int ret;

	/* parse .register_info.ctrl_reg */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-ctrl",
						register_info, 3); /* the length of register_info is 3 */
	if (ret) {
		dev_err(dev, "no spmi-ctrl property set\n");
		return ret;
	}
	sreg->register_info.ctrl_reg = register_info[0];
	sreg->register_info.enable_mask = register_info[1];
	sreg->register_info.eco_mode_mask = register_info[2];

	/* parse .register_info.vset_reg */
	ret = of_property_read_u32_array(np, "hisilicon,hisi-vset",
						register_info, 2); /* the length of register_info is 2 */
	if (ret) {
		dev_err(dev, "no spmi-vset property set\n");
		return ret;
	}
	sreg->register_info.vset_reg = register_info[0];
	sreg->register_info.vset_mask = register_info[1];

	/* parse .off-on-delay */
	ret = of_property_read_u32(np, "hisilicon,hisi-off-on-delay-us",
						&sreg->off_on_delay);
	if (ret) {
		dev_err(dev, "no spmi-off-on-delay-us property set\n");
		return ret;
	}

	/* parse .enable_time */
	ret = of_property_read_u32(np, "hisilicon,hisi-enable-time-us",
				   &rdesc->enable_time);
	if (ret) {
		dev_err(dev, "no spmi-enable-time-us property set\n");
		return ret;
	}

	/* parse .eco_uA */
	ret = of_property_read_u32(np, "hisilicon,hisi-eco-microamp",
				   &sreg->eco_uA);
	if (ret) {
		sreg->eco_uA = 0;
		ret = 0;
	}
	return ret;
}
static int fake_of_get_regulator_constraint(struct regulation_constraints *constraints,
						struct device_node *np)
{
	unsigned int temp_modes = 0;
	int ret;

	/* regulator supports two modes */
	ret = of_property_read_u32_array(np, "hisilicon,valid-modes-mask",
					&(constraints->valid_modes_mask), 1);
	if (ret) {
		pr_err("no spmi-valid-modes-mask property set\n");
		ret = -ENODEV;
		return ret;
	}
	ret = of_property_read_u32_array(np, "hisilicon,valid-idle-mask",
						&temp_modes, 1);
	if (ret) {
		pr_err("no spmi-valid-modes-mask property set\n");
		ret = -ENODEV;
		return ret;
	}
	constraints->valid_ops_mask |= temp_modes;
	return ret;
}
static int spmi_dt_parse_ldo(struct spmi_regulator *sreg,
				struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_desc *rdesc = &sreg->rdesc;
	unsigned int *v_table = NULL;
	int ret;

	/* parse .n_voltages, and .volt_table */
	ret = of_property_read_u32(np, "hisilicon,hisi-n-voltages",
				   &rdesc->n_voltages);
	if (ret) {
		dev_err(dev, "no spmi-n-voltages property set\n");
		return ret;
	}
	/* alloc space for .volt_table */
	v_table = devm_kzalloc(
		dev, sizeof(unsigned int) * rdesc->n_voltages, GFP_KERNEL);
	if (!v_table) {
		ret = -ENOMEM;
		dev_err(dev, "no memory for .volt_table\n");
		return ret;
	}
	ret = of_property_read_u32_array(np, "hisilicon,hisi-vset-table",
						v_table, rdesc->n_voltages);
	if (ret) {
		dev_err(dev, "no spmi-vset-table property set\n");
		devm_kfree(dev, v_table);
		return ret;
	}
	rdesc->volt_table = v_table;

	/* parse regulator's dt common part */
	ret = spmi_dt_parse_common(sreg, pdev);
	if (ret) {
		dev_err(dev, "failure in dt_parse_common\n");
		devm_kfree(dev, v_table);
		return ret;
	}
	return ret;
}

static struct regulator_ops pmic_ldo_rops = {
	.is_enabled = spmi_regulator_is_enabled,
	.enable = spmi_regulator_enable,
	.disable = spmi_regulator_disable,
	.list_voltage = regulator_list_voltage_table,
	.get_voltage = spmi_regulator_get_voltage,
	.set_voltage = spmi_regulator_set_voltage,
	.get_mode = spmi_regulator_get_mode,
	.set_mode = spmi_regulator_set_mode,
	.get_optimum_mode = spmi_regulator_get_optimum_mode,
};

static const struct spmi_regulator spmi_regulator_ldo = {
	.rdesc = {
	.ops = &pmic_ldo_rops,
		.type = REGULATOR_VOLTAGE,
		.owner = THIS_MODULE,
	},
	.dt_parse = spmi_dt_parse_ldo,
};

static const struct of_device_id of_spmi_regulator_match_tbl[] = {
	{
		.compatible = "hisilicon-hisi-ldo",
		.data = &spmi_regulator_ldo,
	},
	{   }   /* end */
};

static int spmi_regulator_probe(struct spmi_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_desc *rdesc = NULL;
	struct regulator_dev *rdev = NULL;
	struct spmi_regulator *sreg = NULL;
	struct regulator_init_data *initdata = NULL;
	struct regulator_config config;
	const struct of_device_id *match = NULL;
	const struct spmi_regulator *template = NULL;
	struct regulation_constraints *constraint = NULL;
	const char *supplyname = NULL;
	int ret;

	/* to check which type of regulator this is */
	match = of_match_device(of_spmi_regulator_match_tbl, &pdev->dev);
	if (!match) {
		pr_err("get regulator fail!\n");
		return -EINVAL;
	}
	template = match->data;
	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (!initdata) {
		pr_err("get regulator init data error!\n");
		return -EINVAL;
	}
	ret = memset_s(&config, sizeof(struct regulator_config),
				0, sizeof(struct regulator_config));
	if (ret != EOK)
		pr_err("memset_s fail!\n");
	/* regulator supports two modes */
	constraint = &initdata->constraints;
	ret = fake_of_get_regulator_constraint(constraint, np);
	if (ret) {
		pr_err("get regulator mode error!\n");
		return ret;
	}
	sreg = kmemdup(template, sizeof(*sreg), GFP_KERNEL);
	if (!sreg) {
		pr_err("template kememdup is fail\n");
		return -ENOMEM;
	}
	sreg->name = initdata->constraints.name;
	rdesc = &sreg->rdesc;
	rdesc->name = sreg->name;
	rdesc->min_uV = initdata->constraints.min_uV;
	supplyname = of_get_property(np, "hisilicon,supply_name", NULL);
	if (supplyname != NULL)
		initdata->supply_regulator = supplyname;
	/* to parse device tree data for regulator specific */
	ret = sreg->dt_parse(sreg, pdev);
	if (ret) {
		dev_err(dev, "device tree parameter parse error!\n");
		goto regulator_probe_end;
	}
	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = sreg;
	config.of_node = pdev->dev.of_node;

	/* register regulator */
	rdev = regulator_register(rdesc, &config);
	if (IS_ERR(rdev)) {
		dev_err(dev, "failed to register %s\n",
			rdesc->name);
		ret = PTR_ERR(rdev);
		goto regulator_probe_end;
	}
	BRAND_DEBUG("[%s]:valid_modes_mask[0x%x], valid_ops_mask[0x%x]\n",
		rdesc->name, constraint->valid_modes_mask, constraint->valid_ops_mask);

	dev_set_drvdata(dev, rdev);
#ifdef CONFIG_HISI_PMIC_DEBUG
	ret = regulator_debugfs_create(dev);
	if (ret)
		regulator_unregister(rdev);
#endif
regulator_probe_end:
	if (ret)
		kfree(sreg);
	return ret;
}

static int spmi_regulator_remove(struct spmi_device *pdev)
{
	struct regulator_dev *rdev = NULL;
	struct spmi_regulator *sreg = NULL;

	rdev = dev_get_drvdata(&pdev->dev);
	if (!rdev) {
		pr_err("%s:rdev is NULL\n", __func__);
		return -ENOMEM;
	}

	sreg = rdev_get_drvdata(rdev);
	if (!sreg) {
		pr_err("%s:sreg is NULL\n", __func__);
		return -ENOMEM;
	}

	regulator_unregister(rdev);

	if (sreg->rdesc.volt_table)
		devm_kfree(&pdev->dev, (unsigned int *)sreg->rdesc.volt_table);

	kfree(sreg);
	return 0;
}

static const struct spmi_device_id regulator_spmi_id[] = {
	{ "hisilicon-hisi-ldo", 0 },
	{}
};

static struct spmi_driver spmi_pmic_driver = {
	.driver = {
		.name	= "spmi_regulator",
		.owner  = THIS_MODULE,
		.of_match_table = of_spmi_regulator_match_tbl,
	},
	.id_table = regulator_spmi_id,
	.probe	= spmi_regulator_probe,
	.remove	= spmi_regulator_remove,
};

STATIC int __init spmi_regulator_init(void)
{
	return spmi_driver_register(&spmi_pmic_driver);
}

STATIC void __exit spmi_regulator_exit(void)
{
	spmi_driver_unregister(&spmi_pmic_driver);
}

fs_initcall(spmi_regulator_init);
module_exit(spmi_regulator_exit);

MODULE_DESCRIPTION("regulator driver");
MODULE_LICENSE("GPL v2");
