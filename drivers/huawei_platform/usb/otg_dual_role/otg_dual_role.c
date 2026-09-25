/*
 * otg_dual_role.c
 *
 * dummy otg dual role with micro-usb driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#ifdef CONFIG_DUAL_ROLE_USB_INTF
#include <linux/usb/class-dual-role.h>
#endif

#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG otg_dual_role
HWLOG_REGIST();

struct otg_dual_role_data {
	struct device *dev;
#ifdef CONFIG_DUAL_ROLE_USB_INTF
	struct dual_role_phy_instance *dual_role;
#endif /* CONFIG_DUAL_ROLE_USB_INTF */
	int dual_state;
};

#ifdef CONFIG_DUAL_ROLE_USB_INTF
static enum dual_role_property otg_dual_role_props[] = {
	DUAL_ROLE_PROP_MODE,
	DUAL_ROLE_PROP_PR,
	DUAL_ROLE_PROP_DR,
};

static int otg_dual_role_prop_is_writeable(struct dual_role_phy_instance *drp,
	enum dual_role_property prop)
{
	if (prop == DUAL_ROLE_PROP_MODE)
		return 1;
	else
		return 0;
}

static int otg_dual_role_get_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, unsigned int *val)
{
	struct otg_dual_role_data *dual_data = dual_role_get_drvdata(dual_role);
	int state;

	if (!dual_data) {
		hwlog_err("dual_data is null\n");
		return -EINVAL;
	}

	state = dual_data->dual_state;

	if (prop == DUAL_ROLE_PROP_MODE) {
		*val = state;
	} else if (prop == DUAL_ROLE_PROP_PR) {
		if (state == DUAL_ROLE_PROP_MODE_DFP)
			*val = DUAL_ROLE_PROP_PR_SRC;
		else if (state == DUAL_ROLE_PROP_MODE_UFP)
			*val = DUAL_ROLE_PROP_PR_SNK;
		else
			*val = DUAL_ROLE_PROP_PR_NONE;
	} else if (prop == DUAL_ROLE_PROP_DR) {
		if (state == DUAL_ROLE_PROP_MODE_DFP)
			*val = DUAL_ROLE_PROP_DR_HOST;
		else if (state == DUAL_ROLE_PROP_MODE_UFP)
			*val = DUAL_ROLE_PROP_DR_DEVICE;
		else
			*val = DUAL_ROLE_PROP_DR_NONE;
	} else {
		return -EINVAL;
	}

	hwlog_info("get_prop prop=%d,state=%d,val=%d\n", prop, state, *val);
	return 0;
}

static int otg_dual_role_set_prop(struct dual_role_phy_instance *dual_role,
	enum dual_role_property prop, const unsigned int *val)
{
	struct otg_dual_role_data *dual_data = dual_role_get_drvdata(dual_role);

	if (!dual_data) {
		hwlog_err("dual_data is null\n");
		return -EINVAL;
	}

	if (prop != DUAL_ROLE_PROP_MODE)
		return -EINVAL;

	if (*val != DUAL_ROLE_PROP_MODE_DFP &&
		*val != DUAL_ROLE_PROP_MODE_UFP)
		return -EINVAL;

	dual_data->dual_state = *val;
	dual_role_instance_changed(dual_role);

	hwlog_info("set_prop prop=%d,val=%d\n", prop, *val);
	return 0;
}
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

static int otg_dual_role_probe(struct platform_device *pdev)
{
	struct otg_dual_role_data *dual_data = NULL;
#ifdef CONFIG_DUAL_ROLE_USB_INTF
	struct dual_role_phy_desc *dual_desc = NULL;
	struct dual_role_phy_instance *dual_role = NULL;
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	dual_data = devm_kzalloc(&pdev->dev, sizeof(*dual_data), GFP_KERNEL);
	if (!dual_data)
		return -ENOMEM;

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	dual_desc = devm_kzalloc(&pdev->dev, sizeof(*dual_desc), GFP_KERNEL);
	if (!dual_desc) {
		devm_kfree(&pdev->dev, dual_data);
		return -ENOMEM;
	}

	dual_desc->name = "otg_default";
	dual_desc->supported_modes = DUAL_ROLE_SUPPORTED_MODES_DFP_AND_UFP;
	dual_desc->properties = otg_dual_role_props;
	dual_desc->num_properties = ARRAY_SIZE(otg_dual_role_props);
	dual_desc->get_property = otg_dual_role_get_prop;
	dual_desc->set_property = otg_dual_role_set_prop;
	dual_desc->property_is_writeable = otg_dual_role_prop_is_writeable;
	dual_role = devm_dual_role_instance_register(&pdev->dev, dual_desc);
	if (IS_ERR(dual_role)) {
		hwlog_err("otg dual role instance register fail\n");
		devm_kfree(&pdev->dev, dual_desc);
		devm_kfree(&pdev->dev, dual_data);
		return -EINVAL;
	}

	dual_data->dev = &pdev->dev;
	dual_data->dual_role = dual_role;
	dual_data->dual_state = DUAL_ROLE_PROP_MODE_NONE;
	dual_role->drv_data = dual_data;
#endif /* CONFIG_DUAL_ROLE_USB_INTF */

	dev_set_drvdata(&pdev->dev, dual_data);

	hwlog_info("probe end\n");
	return 0;
}

static int otg_dual_role_remove(struct platform_device *pdev)
{
	struct otg_dual_role_data *dual_data = dev_get_drvdata(&pdev->dev);

	hwlog_info("remove begin\n");

	if (!dual_data)
		return -EINVAL;

#ifdef CONFIG_DUAL_ROLE_USB_INTF
	devm_dual_role_instance_unregister(&pdev->dev, dual_data->dual_role);
#endif /* CONFIG_DUAL_ROLE_USB_INTF */
	dev_set_drvdata(&pdev->dev, NULL);

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id otg_dual_role_of_match[] = {
	{
		.compatible = "huawei,otg-dual-role",
		.data = NULL,
	},
	{},
};

static struct platform_driver otg_dual_role_drv = {
	.probe = otg_dual_role_probe,
	.remove = otg_dual_role_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "otg_dual_role",
		.of_match_table = otg_dual_role_of_match,
	},
};

static int __init otg_dual_role_init(void)
{
	return platform_driver_register(&otg_dual_role_drv);
}

static void __exit otg_dual_role_exit(void)
{
	platform_driver_unregister(&otg_dual_role_drv);
}

module_init(otg_dual_role_init);
module_exit(otg_dual_role_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("dummy otg dual role module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
