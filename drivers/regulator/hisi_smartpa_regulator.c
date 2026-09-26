/*
 * Copyright (c) Hisilicon technologies Co., Ltd.2019-2019. All rights reserved.
 * Description: provide smartpa regulator functions
 * Author: Hisilicon
 * Create: 2019-03-17
 */
#include <hisi_smartpa_regulator.h>

static struct smartpa_regulator_reg_ops g_spmi_regulator_reg_ops;

static struct smartpa_regs smartpa_regs_lookups_cfg_v0[] = {
	PMU_LOW(0x00, 0x1, 0, 0x0),
	PMU_LOW(0x01, 0xF0, 4, 0x9),
	PMU_LOW(0x01, 0x3, 0, 0x3),
	PMU_LOW(0x03, 0xFC, 2, 0xF),
	PMU_LOW(0x03, 0x1, 0, 0x0),
	PMU_LOW(0x04, 0xFF, 0, 0x7A),
	PMU_LOW(0x05, 0xFF, 0, 0x64),
	PMU_LOW(0x06, 0xFF, 0, 0xDA),
	PMU_LOW(0x07, 0xFF, 0, 0xDA),
	PMU_LOW(0x08, 0xFF, 0, 0x70),
	PMU_LOW(0x09, 0xFF, 0, 0x6D),
	PMU_LOW(0x0A, 0xFF, 0, 0xB),
	PMU_LOW(0x0B, 0xFF, 0, 0x19),
	PMU_LOW(0x0C, 0xFF, 0, 0xF),
	PMU_LOW(0x0D, 0xFF, 0, 0x19),
	PMU_LOW(0x0E, 0xFF, 0, 0xC),
	PMU_LOW(0x33, 0x80, 7, 0x1),
	PMU_LOW(0x83, 0xFF, 0, 0x3),
	PMU_LOW(0x02, 0xE0, 5, 0x0),
	PMU_LOW(0x02, 0x3, 0, 0x1),
};

static struct smartpa_regs smartpa_flash_clear_irq_regs[] = {
	PMU_LOW(0xA4, 0xFF, 0, 0x5),
	PMU_LOW(0xA5, 0xFF, 0, 0x38),
	PMU_LOW(0xA1, 0x1, 0, 0x0),
	PMU_LOW(0xA1, 0x4, 2, 0x0),
	PMU_LOW(0xA2, 0x18, 3, 0x0),
};

static struct smartpa_regs smartpa_flash_release_irq_mask_regs[] = {
	PMU_LOW(0xA1, 0x1, 0, 0x1),
	PMU_LOW(0xA1, 0x4, 2, 0x1),
	PMU_LOW(0xA2, 0x18, 3, 0x3),
};

int smartpa_flash_init()
{
	unsigned int ilen;
	unsigned int i;
	int ret = 0;

	ilen = ARRAY_SIZE(smartpa_regs_lookups_cfg_v0);
	for (i = 0; i < ilen; i++) {
		ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(I2C_ADDR,
			(int)smartpa_regs_lookups_cfg_v0[i].offset,
			(int)smartpa_regs_lookups_cfg_v0[i].mask,
			((int)smartpa_regs_lookups_cfg_v0[i].value <<
			(int)smartpa_regs_lookups_cfg_v0[i].shift));
		if (ret)
			return ret;
	}
	return ret;
}

int smartpa_flash_irq_enable(int enable)
{
	unsigned int ilen1;
	unsigned int ilen2;
	unsigned int i;
	int ret = 0;

	if (enable == 1) {
		ilen1 = ARRAY_SIZE(smartpa_flash_clear_irq_regs);
		/* clear the Flash interrupt status */
		for (i = 0; i < ilen1; i++) {
			ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
				I2C_ADDR,
				(int)smartpa_flash_clear_irq_regs[i].offset,
				(int)smartpa_flash_clear_irq_regs[i].mask,
				((int)smartpa_flash_clear_irq_regs[i].value <<
				(int)smartpa_flash_clear_irq_regs[i].shift));
			if (ret)
				return ret;
		}
	} else if (enable == 0) {
		ilen2 = ARRAY_SIZE(smartpa_flash_release_irq_mask_regs);
		/* Release the Flash interrupt mask status */
		for (i = 0; i < ilen2; i++) {
			ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
				I2C_ADDR,
				(int)smartpa_flash_release_irq_mask_regs[i].offset,
				(int)smartpa_flash_release_irq_mask_regs[i].mask,
				((int)smartpa_flash_release_irq_mask_regs[i].value <<
				(int)smartpa_flash_release_irq_mask_regs[i].shift));
			if (ret)
				return ret;
		}
	} else {
		ret = -1;
	}
	return ret;
}

static int smartpa_regulator_is_enabled(struct regulator_dev *dev)
{
	int ret;
	int value = 0;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}

	struct smartpa_regulator_ctrl_regs *ctrl_regs = &(sreg->ctrl_regs);
	struct smartpa_regulator_ctrl_data *ctrl_data = &(sreg->ctrl_data);

	ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, ctrl_regs->enable_reg2, &value);
	if (ret) {
		pr_err("[%s]: spmi_regulator_reg_read error", __func__);
		return ret;
	}
	pr_debug("[%s]:ctrl_reg2=0x%x, enable_statue=%d\n", __func__,
		ctrl_regs->enable_reg2, ((value & ctrl_data->mask2) != 0));
	return ((value & ctrl_data->mask2) != 0);
}

static int smartpa_regulator_enable(struct regulator_dev *dev)
{
	int ret;
	int ret1, ret2;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}
	struct smartpa_regulator_ctrl_regs *ctrl_regs = &(sreg->ctrl_regs);
	struct smartpa_regulator_ctrl_data *ctrl_data = &(sreg->ctrl_data);
	struct smartpa_regulator_time_out_regs *time_out_regs = &(sreg->time_out_regs);
	struct smartpa_regulator_vbat_protect_regs *vbat_protect_regs = &(sreg->vbat_protect_regs);

	if (sreg->time_out_flag) {
		ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(I2C_ADDR,
		time_out_regs->time_out_reg, time_out_regs->mask, time_out_regs->enable);
		if (ret)
			return ret;
	}
	if (sreg->vbat_protect_flag) {
		ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(I2C_ADDR,
			vbat_protect_regs->vbat_protect_reg, vbat_protect_regs->mask,
			(vbat_protect_regs->value << vbat_protect_regs->shift));
		if (ret)
			return ret;
	}
	ret1 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(I2C_ADDR,
		ctrl_regs->enable_reg1, ctrl_data->mask1, ctrl_data->shift1);
	ret2 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(I2C_ADDR,
		ctrl_regs->enable_reg2, ctrl_data->mask2, ctrl_data->shift2);
	ret = (ret1 || ret2);

	return ret;
}

static int smartpa_regulator_disable(struct regulator_dev *dev)
{
	int ret;
	int ret1, ret2;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}

	struct smartpa_regulator_ctrl_regs *ctrl_regs = &(sreg->ctrl_regs);
	struct smartpa_regulator_ctrl_data *ctrl_data = &(sreg->ctrl_data);
	struct smartpa_regulator_time_out_regs *time_out_regs = &(sreg->time_out_regs);

	if (sreg->time_out_flag) {
		ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(I2C_ADDR,
			time_out_regs->time_out_reg, time_out_regs->mask,
			time_out_regs->disable);
		if (ret)
			return ret;
	}
	ret1 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
		I2C_ADDR, ctrl_regs->enable_reg2, ctrl_data->mask2, 0);
	ret2 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
		I2C_ADDR, ctrl_regs->enable_reg1, ctrl_data->mask1, 0);
	ret = (ret1 || ret2);
	return ret;
}

static int smartpa_regulator_list_voltage_linear(
	struct regulator_dev *rdev, unsigned int selector)
{
	struct smartpa_regulator *sreg = rdev_get_drvdata(rdev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}
	if (selector >= sreg->vol_numb) {
		pr_err("selector err %s %d\n", __func__, __LINE__);
		return -1;
	}
	return sreg->vset_table[selector];
}

static int smartpa_regulator_get_voltage(struct regulator_dev *dev)
{
	int index;
	int ret1, ret2;
	int value1 = 0;
	int value2 = 0;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}
	struct smartpa_regulator_vset_reg1 *vset_reg1 = &(sreg->vset_reg1);
	struct smartpa_regulator_vset_reg2 *vset_reg2 = &(sreg->vset_reg2);
	struct smartpa_regulator_vset_data1 *vset_data1 = &(sreg->vset_data1);
	struct smartpa_regulator_vset_data2 *vset_data2 = &(sreg->vset_data2);

	ret1 = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, vset_reg1->vset_reg, &value1);
	ret2 = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, vset_reg2->vset_reg, &value2);
	if (ret1 || ret2)
		return -1;
	index = (value2 & (~vset_data2->mask)) |
		((value1 & (~vset_data1->mask)) >> vset_data1->shift);
	if (index < 0 || index >= (int)sreg->vol_numb) {
		pr_err("in [%s] index  %d outside\n", __func__, index);
		return -1;
	}
	return sreg->vset_table[index];
}

static int smartpa_regulator_set_voltage(
	struct regulator_dev *dev, int min_uv, int max_uv, unsigned int *selector)
{
	u32 vsel;
	int ret;
	int ret1, ret2, ret3, ret4;
	int uv;
	int value1;
	int value2;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}
	struct smartpa_regulator_vset_reg1 *vset_reg1 = &(sreg->vset_reg1);
	struct smartpa_regulator_vset_reg2 *vset_reg2 = &(sreg->vset_reg2);
	struct smartpa_regulator_vset_data1 *vset_data1 = &(sreg->vset_data1);
	struct smartpa_regulator_vset_data2 *vset_data2 = &(sreg->vset_data2);

	for (vsel = 0; vsel < sreg->rdesc.n_voltages; vsel++) {
		uv = sreg->vset_table[vsel];
		/* Break at the first in-range value */
		if (min_uv <= uv && uv <= max_uv)
			break;
	}

	/* unlikely to happen. sanity test done by regulator core */
	if (unlikely(vsel == sreg->rdesc.n_voltages))
		return -EINVAL;

	*selector = vsel;
	/* set voltage selector */
	ret1 = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, vset_reg1->vset_reg, &value1);
	ret2 = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, vset_reg2->vset_reg, &value2);
	value1 = (value1 & vset_data1->mask) |
		 ((vsel & vset_data1->bit) << vset_data1->shift);
	value2 = (value2 & vset_data2->mask) |
		 ((vsel & vset_data2->bit) << vset_data1->shift);
	ret3 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
		I2C_ADDR, vset_reg1->vset_reg, 0xFF, value1);
	ret4 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
		I2C_ADDR, vset_reg2->vset_reg, 0xFF, value2);
	ret = (ret1 || ret2 || ret3 || ret4);
	return ret;
}

static int smartpa_regulator_set_current_limit(
	struct regulator_dev *dev, int min_ua, int max_ua)
{
	u32 vsel;
	int ret;
	int ret1, ret2;
	int ua;
	int value = 0;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}
	struct smartpa_regulator_vset_reg1 *vset_reg1 = &(sreg->vset_reg1);
	struct smartpa_regulator_vset_data1 *vset_data1 = &(sreg->vset_data1);

	for (vsel = 0; vsel < sreg->rdesc.n_voltages; vsel++) {
		ua = sreg->vset_table[vsel];
		/* Break at the first in-range value */
		if (min_ua <= ua && ua <= max_ua)
			break;
	}

	/* unlikely to happen. sanity test done by regulator core */
	if (unlikely(vsel == sreg->rdesc.n_voltages))
		return -EINVAL;

	/* set voltage selector */
	ret1 = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, vset_reg1->vset_reg, &value);
	value = (value & vset_data1->mask) | (vsel << vset_data1->shift);
	ret2 = g_spmi_regulator_reg_ops.spmi_regulator_reg_write(
		I2C_ADDR, vset_reg1->vset_reg, 0xFF, value);
	ret = ret1 || ret2;
	return ret;
}

static int smartpa_regulator_get_current_limit(struct regulator_dev *dev)
{
	int i;
	int index;
	int ret;
	int value = 0;
	struct smartpa_regulator *sreg = rdev_get_drvdata(dev);

	if (sreg == NULL) {
		pr_err("[%s]: invalid input argument, sreg is NULL", __func__);
		return -EINVAL;
	}
	struct smartpa_regulator_vset_reg1 *vset_reg1 = &(sreg->vset_reg1);
	struct smartpa_regulator_vset_data1 *vset_data1 = &(sreg->vset_data1);

	ret = g_spmi_regulator_reg_ops.spmi_regulator_reg_read(
		I2C_ADDR, vset_reg1->vset_reg, &value);
	if (ret)
		return ret;
	index = (int)((value & (~vset_data1->mask)) >> vset_data1->shift);
	if (index < 0 || index >= (int)sreg->vol_numb) {
		pr_err("in [%s] index  %d outside\n", __func__, index);
		return -1;
	}
	return sreg->vset_table[index];
}

static struct regulator_ops smartpa_regulator_rops = {
	.is_enabled = smartpa_regulator_is_enabled,
	.enable = smartpa_regulator_enable,
	.disable = smartpa_regulator_disable,
	.list_voltage = smartpa_regulator_list_voltage_linear,
	.get_voltage = smartpa_regulator_get_voltage,
	.set_voltage = smartpa_regulator_set_voltage,
	.get_current_limit = smartpa_regulator_get_current_limit,
	.set_current_limit = smartpa_regulator_set_current_limit,
};

static const struct smartpa_regulator smartpa_regulator = {
	.rdesc = {
			.ops = &smartpa_regulator_rops,
			.type = REGULATOR_VOLTAGE,
			.owner = THIS_MODULE,
	},
};

static struct of_device_id of_smartpa_regulator_match_tbl[] = {
	{
		.compatible = "hisilicon,hisi-regulator-smartpa",
		.data = &smartpa_regulator,
	},
	{}
};
static int fake_of_get_regulator_constraint(
	struct regulation_constraints *constraints, struct device_node *np)
{
	const __be32 *min_uv = NULL;
	const __be32 *max_uv = NULL;
	unsigned int *valid_ops_mask = NULL;

	if ((!np) || (!constraints))
		return -1;

	constraints->name = of_get_property(np, "regulator-name", NULL);

	min_uv = of_get_property(np, "regulator-min-microvolt", NULL);
	if (min_uv) {
		constraints->min_uV = be32_to_cpu(*min_uv);
		constraints->min_uA = be32_to_cpu(*min_uv);
	}

	max_uv = of_get_property(np, "regulator-max-microvolt", NULL);
	if (max_uv) {
		constraints->max_uV = be32_to_cpu(*max_uv);
		constraints->max_uA = be32_to_cpu(*max_uv);
	}
	valid_ops_mask = (unsigned int *)of_get_property(
		np, "hisilicon,valid-ops-mask", NULL);
	if (valid_ops_mask)
		constraints->valid_ops_mask = be32_to_cpu(*valid_ops_mask);
	return 0;
}

static int fake_of_get_regulator_sreg(struct smartpa_regulator *sreg,
	struct device *dev, struct device_node *np)
{
	int *vol_numb = NULL;
	unsigned int *off_on_delay = NULL;
	unsigned int *time_out_flag = NULL;
	unsigned int *vbat_protect_flag = NULL;
	unsigned int *vset_table = NULL;
	int ret;

	off_on_delay = (unsigned int *)of_get_property(
		np, "hisilicon,off-on-delay", NULL);
	if (off_on_delay)
		sreg->off_on_delay = be32_to_cpu(*off_on_delay);

	time_out_flag = (unsigned int *)of_get_property(
		np, "hisilicon,time-out-flag", NULL);
	if (time_out_flag) {
		sreg->time_out_flag = be32_to_cpu(*time_out_flag);
		(void)of_property_read_u32_array(np, "hisilicon,time-out-regs",
		(unsigned int *)(&sreg->time_out_regs), 0x4);
	}

	vbat_protect_flag = (unsigned int *)of_get_property(
		np, "hisilicon,vbat-protect-flag", NULL);
	if (vbat_protect_flag) {
		sreg->vbat_protect_flag = be32_to_cpu(*vbat_protect_flag);
		(void)of_property_read_u32_array(np, "hisilicon,vbat-protect-regs",
		(unsigned int *)(&sreg->vbat_protect_regs), 0x4);
	}
	/* this parameters are not required, so do not judge the return value */
	(void)of_property_read_u32_array(np, "hisilicon,ctrl-regs",
		(unsigned int *)(&sreg->ctrl_regs), 0x2);
	(void)of_property_read_u32_array(np, "hisilicon,ctrl-data",
		(unsigned int *)(&sreg->ctrl_data), 0x4);
	(void)of_property_read_u32_array(np, "hisilicon,vset-reg1",
		(unsigned int *)(&sreg->vset_reg1), 0x1);
	(void)of_property_read_u32_array(np, "hisilicon,vset-data1",
		(unsigned int *)(&sreg->vset_data1), 0x4);
	(void)of_property_read_u32_array(np, "hisilicon,vset-reg2",
		(unsigned int *)(&sreg->vset_reg2), 0x1);
	(void)of_property_read_u32_array(np, "hisilicon,vset-data2",
		(unsigned int *)(&sreg->vset_data2), 0x4);

	vol_numb =
		(int *)of_get_property(np, "hisilicon,regulator-n-vol", NULL);
	if (vol_numb)
		sreg->vol_numb = be32_to_cpu(*vol_numb);
	vset_table =
		devm_kzalloc(dev, sreg->vol_numb * sizeof(int), GFP_KERNEL);
	if (!vset_table)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "hisilicon,vset-table",
		(unsigned int *)vset_table, sreg->vol_numb);
	if (ret) {
		devm_kfree(dev, vset_table);
		return ret;
	}
	sreg->vset_table = vset_table;

	return ret;
}

static int fake_read(int i2c_addr, int reg_addr, int *value)
{
	return 0;
}

static int fake_write(int i2c_addr, int reg_addr, int mask, int value)
{
	return 0;
}

int smartpa_regulator_reg_ops_register(struct smartpa_regulator_reg_ops *ops)
{
	if (ops != NULL && ops->spmi_regulator_reg_read != NULL \
		&& ops->spmi_regulator_reg_write != NULL) {
		g_spmi_regulator_reg_ops.spmi_regulator_reg_read =
			ops->spmi_regulator_reg_read;

		g_spmi_regulator_reg_ops.spmi_regulator_reg_write =
			ops->spmi_regulator_reg_write;
		return 0;
	} else {
		pr_err("[%s]: hisi regulator reg ops is NULL!!!\n", __func__);
		return -1;
	}
}

static int smartpa_regulator_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct regulator_dev *rdev = NULL;
	struct regulator_desc *rdesc = NULL;
	struct smartpa_regulator *sreg = NULL;
	struct regulator_init_data *initdata = NULL;
	const struct of_device_id *match = NULL;
	const struct smartpa_regulator *template = NULL;
	struct regulator_config config = {};
	/* Because read and write interface has not been successfully enabled */
	struct smartpa_regulator_reg_ops ops = {
		.spmi_regulator_reg_read = fake_read,
		.spmi_regulator_reg_write = fake_write,
	};

	if (!pdev) {
		pr_err("%s:pdev is NULL!\n", __func__);
		return -ENODEV;
	}
	match = of_match_device(
		of_smartpa_regulator_match_tbl, &pdev->dev);
	if (!match) {
		pr_err("get hisi smartpa regulator fail!\n");
		return -ENODEV;
	}
	template = match->data;

	initdata = of_get_regulator_init_data(dev, np, NULL);
	if (initdata == NULL) {
		pr_err("[%s]:get regulator init data error !\n", __func__);
		return -EINVAL;
	}
	ret = fake_of_get_regulator_constraint(&initdata->constraints, np);
	if (!!ret) {
		pr_err("[%s]:get regulator constraint error !\n", __func__);
		return -EINVAL;
	}

	sreg = kmemdup(template, sizeof(*sreg), GFP_KERNEL);
	if (!sreg)
		return -ENOMEM;

	if (fake_of_get_regulator_sreg(sreg, dev, np) != 0) {
		kfree(sreg);
		return -EINVAL;
	}

	rdesc = &sreg->rdesc;
	rdesc->n_voltages = sreg->vol_numb;
	rdesc->name = initdata->constraints.name;
	rdesc->min_uV = initdata->constraints.min_uV;

	/* to parse device tree data for regulator specific */
	config.dev = &pdev->dev;
	config.init_data = initdata;
	config.driver_data = sreg;
	config.of_node = pdev->dev.of_node;

	if (g_spmi_regulator_reg_ops.spmi_regulator_reg_read ==	NULL) {
		ret = smartpa_regulator_reg_ops_register(&ops);
		if (ret) {
			kfree(sreg);
			return -ENOMEM;
		};
	};
	/* register regulator */
	rdev = regulator_register(rdesc, &config);
	if (IS_ERR(rdev)) {
		pr_err("[%s]:regulator failed to register %s\n", __func__,
			rdesc->name);
		kfree(sreg);
		ret = PTR_ERR(rdev);
		return ret;
	}
	platform_set_drvdata(pdev, rdev);

	return ret;
}

static int smartpa_regulator_remove(struct platform_device *pdev)
{
	struct regulator_dev *rdev = NULL;
	struct smartpa_regulator *sreg = NULL;

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
	kfree(sreg);
	return 0;
}

static struct platform_driver smartpa_regulator_driver = {
	.driver = {
			.name = "smartpa_regulator",
			.owner = THIS_MODULE,
			.of_match_table = of_smartpa_regulator_match_tbl,
	},
	.probe = smartpa_regulator_probe,
	.remove = smartpa_regulator_remove,
};

static int __init smartpa_regulator_init(void)
{
	/* if there is some adapt layers so must be add its init-function */
	return platform_driver_register(&smartpa_regulator_driver);
}

static void __exit smartpa_regulator_exit(void)
{
	platform_driver_unregister(&smartpa_regulator_driver);
}

fs_initcall(smartpa_regulator_init);
module_exit(smartpa_regulator_exit);

MODULE_AUTHOR("Mengying Li <limengying7@hisilicon.com>");
MODULE_DESCRIPTION("Hisi hisi_smartpa regulator driver");
MODULE_LICENSE("GPL v2");
