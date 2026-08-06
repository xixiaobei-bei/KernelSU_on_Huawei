/*
 * ite83201-regulator.c
 *
 * Regulator driver for ITE83201 PMIC
 *
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/of.h>
#include <linux/slab.h>
#include "../echub_i2c.h"
#include <linux/regulator/of_regulator.h>


/* LDOs */
#define EC_LDO_1                1
#define EC_LDO_2                2
#define EC_LDO_3                3
#define EC_LDO_4                4
#define EC_LDO_5                5


#define EC_MAX_REG_ID           EC_LDO_5


/* Number of LDO voltage regulators  available */
#define EC_NUM_LDO              5
/* Number of total regulators available */
#define EC_NUM_REGULATOR        EC_NUM_LDO

#define ECHUB_IIC_WRITE_SUCC    0x0
#define ECHUB_IIC_WRITE_TIMES_MAX 0x3
#define COMMAND_TO_EC           0x02B2
#define EC_1V2_EN_REG           0x50
#define EC_DSI_VCCIO_REG        0x52
#define EC_HDMI_VDD_ON_REG      0x54
#define EC_1V8_EN_REG           0x56
#define EC_1V1_EN_REG           0x58
#define EC_3V3_EN_REG           0x5A
#define EC_CAM_PWR_REG          0x5C

#define ENABLE                  1
#define DISABLE                 0
#define RE_COUNTS               3
enum {
	EC_1V2_EN = 1,
	EC_DSI_VCCIO_ON,
	EC_HDMI_VDD_ON,
	EC_1V8_EN,
	EC_1V1_EN,
	EC_3V3_EN,
	EC_CAM_PWR_EN,
};


struct ec_data {
	struct device       *dev;
	struct echub_i2c_dev    *iodev;
	int         num_regulators;
};

static int ec_get_register(struct regulator_dev *rdev, int *shift, int status)
{
	int offset = DISABLE;
	int ldo = rdev_get_id(rdev);

	if (DISABLE == status) {
		offset = DISABLE;
	} else if (ENABLE == status) {
		offset = ENABLE;
	} else {
		pr_err("[%s]int status para can only be zero or one!\n", __func__);
		return -EINVAL;
	}

	switch (ldo) {
	case EC_1V2_EN:
		*shift = EC_1V2_EN_REG + offset;
		break;
	case EC_DSI_VCCIO_ON:
		*shift = EC_DSI_VCCIO_REG + offset;
		break;
	case EC_HDMI_VDD_ON:
		*shift = EC_HDMI_VDD_ON_REG + offset;
		break;
	case EC_1V8_EN:
		*shift = EC_1V8_EN_REG + offset;
		break;
	case EC_1V1_EN:
		*shift = EC_1V1_EN_REG + offset;
		break;
	case EC_3V3_EN:
		*shift = EC_3V3_EN_REG + offset;
		break;
	case EC_CAM_PWR_EN:
		*shift = EC_CAM_PWR_REG + offset;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ec_regulator_is_enabled(struct regulator_dev *rdev)
{
	printk("<[%s]:regulator_id=%d>\n", __func__, rdev_get_id(rdev));
	return 0;
}

static int ec_regulator_enable(struct regulator_dev *rdev)
{
	struct ec_data *edata = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = edata->iodev->client;
	int ret = 0;
	int shift = 0;
	int i = 0;
	int rslt = 1;

	if (edata == NULL || i2c == NULL) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}
	ret = ec_get_register(rdev, &shift, ENABLE);
	if (ret) {
		pr_err("[%s] could not get EC enable register\n", __func__);
		return ret;
	}
	printk("<[%s]:regulator_id=%d>\n", __func__, rdev_get_id(rdev));

	for (i = 0; i < RE_COUNTS; i++) {
		rslt = edata->iodev->write_func(edata->iodev, COMMAND_TO_EC, shift);
		if (rslt == 0) {
			printk("regulator_enable_ok,re_counts= %d.\n", i);
			break;
		}
	}
	return rslt;
}

static int ec_regulator_disable(struct regulator_dev *rdev)
{
	struct ec_data *edata = rdev_get_drvdata(rdev);
	struct i2c_client *i2c = edata->iodev->client;
	int ret = 0;
	int shift = 0;
	int i = 0;
	int rslt = 1;

	if (edata == NULL || i2c == NULL) {
		pr_err("[%s]regulator get  para is err!\n", __func__);
		return -EINVAL;
	}
	ret = ec_get_register(rdev, &shift, DISABLE);
	if (ret) {
		pr_err("[%s] could not get EC disable register\n", __func__);
		return ret;
	}
	printk("<[%s]:regulator_id=%d>\n", __func__, rdev_get_id(rdev));

	for (i = 0; i < RE_COUNTS; i++) {
		rslt = edata->iodev->write_func(edata->iodev, COMMAND_TO_EC, shift);
		if (rslt == 0) {
			printk("regulator_disable_ok,re_counts= %d.\n", i);
			break;
		}
	}
	return rslt;

}

static struct regulator_ops ec_regulator_ops = {
	.is_enabled     = ec_regulator_is_enabled,
	.enable         = ec_regulator_enable,
	.disable        = ec_regulator_disable,
};

static struct regulator_desc regulators[] = {
	{
		.name       = "LDO0",
		.id         = EC_1V2_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO1",
		.id         = EC_DSI_VCCIO_ON,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO2",
		.id         = EC_HDMI_VDD_ON,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO3",
		.id         = EC_1V8_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO4",
		.id         = EC_1V1_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO5",
		.id         = EC_3V3_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	},
	{
		.name       = "LDO6",
		.id         = EC_CAM_PWR_EN,
		.ops        = &ec_regulator_ops,
		.type       = REGULATOR_VOLTAGE,
		.owner      = THIS_MODULE,
	}
};

static int ec_regulator_dt_parse_pdata(struct echub_i2c_dev *iodev,
					struct ec_platform_data *pdata)
{
	struct device_node *pmic_np = iodev->dev->of_node;
	struct device_node *power = NULL;
	struct device_node *regulators_np = NULL;
	struct device_node *reg_np = NULL;
	struct ec_regulator_data *rdata = NULL;
	unsigned int i;
	int ret;

	power = of_get_child_by_name(pmic_np, "power");
	if (!power) {
		dev_err(iodev->dev, "could not find regulators sub-node_power\n");
		return -EINVAL;
	}

	regulators_np = of_get_child_by_name(power, "regulators");
	if (!regulators_np) {
		dev_err(iodev->dev, "could not find regulators sub-node\n");
		return -EINVAL;
	}

	/* count the number of regulators to be supported in pmic */
	pdata->num_regulators = of_get_child_count(regulators_np);

	rdata = devm_kzalloc(iodev->dev, sizeof(*rdata) *
				pdata->num_regulators, GFP_KERNEL);
	if (!rdata) {
		of_node_put(regulators_np);
		return -ENOMEM;
	}

	pdata->regulators = rdata;
	for (i = 0; i < ARRAY_SIZE(regulators); ++i) {
		reg_np = of_get_child_by_name(regulators_np,
						regulators[i].name);
		if (!reg_np)
			continue;

		rdata->id = regulators[i].id;
		rdata->initdata = of_get_regulator_init_data(iodev->dev,
								reg_np,
								&regulators[i]);
		rdata->reg_node = reg_np;
		++rdata;
	}
	pdata->num_regulators = rdata - pdata->regulators;
	of_node_put(reg_np);
	of_node_put(regulators_np);

	return 0;
}

static int EchubRegulatorProbe(struct platform_device *pdev)
{
	struct echub_i2c_dev *iodev = dev_get_drvdata(pdev->dev.parent);
	if (iodev == NULL) {
		printk("echub Regulator dev null get echub_dev form tmp");
		iodev = get_echub_i2c_dev_data();
		if (iodev == NULL) {
			printk("[Echub-Regulator] iodev and g_echub_i2c_data is NULL return");
			return 0;
		}
	}
	struct ec_platform_data *pdata = iodev->pdata;
	struct regulator_config config = { };
	struct regulator_dev *rdev = NULL;
	struct ec_data *edata = NULL;
	struct i2c_client *i2c = NULL;
	int i;
	int ret;

	if (!pdata) {
		dev_err(pdev->dev.parent, "No platform init data supplied\n");
		return -ENODEV;
	}

	if (IS_ENABLED(CONFIG_OF) && iodev->dev->of_node) {
		ret = ec_regulator_dt_parse_pdata(iodev, pdata);
		if (ret)
		return ret;
	}

	edata = devm_kzalloc(&pdev->dev, sizeof(struct ec_data),
				GFP_KERNEL);
	if (!edata)
	return -ENOMEM;

	edata->dev = &pdev->dev;
	edata->iodev = iodev;
	edata->num_regulators = pdata->num_regulators;
	platform_set_drvdata(pdev, edata);
	i2c = edata->iodev->client;

	for (i = 0; i < pdata->num_regulators; i++) {
		config.dev = edata->dev;
		config.of_node = pdata->regulators[i].reg_node;
		config.init_data = pdata->regulators[i].initdata;
		config.driver_data = edata;
		rdev = devm_regulator_register(&pdev->dev, &regulators[i], &config);

		if (IS_ERR(rdev)) {
			ret = PTR_ERR(rdev);
			dev_err(&pdev->dev, "regulator register failed\n");
			return ret;
		}

	}

	return 0;
}

static struct of_device_id of_hw_echub_regulator_match_tbl[] = {
	{
		.compatible = "huawei,echub-power",
	},
	{ /* end */ }
};

static struct platform_driver g_echubRegulatorDriver = {
	.driver = {
		.name = "echub-power",
		.owner  = THIS_MODULE,
		.of_match_table = of_hw_echub_regulator_match_tbl,
	},
	.probe = EchubRegulatorProbe,
};

static int __init EC_regulator_init(void)
{
	return platform_driver_register(&g_echubRegulatorDriver);
}
module_init(EC_regulator_init);

static void __exit EC_regulator_cleanup(void)
{
	platform_driver_unregister(&g_echubRegulatorDriver);
}
module_exit(EC_regulator_cleanup);


MODULE_DESCRIPTION("EC voltage regulator driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:EC-LDO");
