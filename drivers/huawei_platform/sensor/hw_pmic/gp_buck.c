/*
 * gp_buck.c
 *
 * gpio ctrl buck driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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
#include <hw_pmic.h>
#include "gp_buck.h"
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG huawei_pmic

#define GPIO_CTRL_PIN_ENABLE    1
#define GPIO_CTRL_PIN_DISABLE   0

HWLOG_REGIST();

struct buck_device_t {
	unsigned int buck_en_ctrl;
	unsigned int buck_volt_ctrl;
	unsigned int h_buck_volt;
	unsigned int l_buck_volt;
};

static struct buck_device_t buck_device;
static const struct of_device_id gp_buck_match_table[] = {
	{ .compatible = "huawei,gpio_buck", },
	{},
};
MODULE_DEVICE_TABLE(of, gp_buck_match_table);

static u32 pre_vol;
static u32 pre_state;

static void gp_buck_vote_ctrl(struct buck_device_t *buck_ctrl, u32 vol)
{
	if (!buck_ctrl)
		return;
	hwlog_info("%s set volt-%u\n", __func__, vol);
	if (buck_ctrl->buck_volt_ctrl == 0) {
		hwlog_warn("%s not support vote set\n", __func__);
		return;
	}
	// if vol is 1.15v set output l, 1.05 set input np
	if (vol == buck_device.h_buck_volt)
		gpio_direction_output(buck_ctrl->buck_volt_ctrl, 0);
	else if (vol == buck_device.l_buck_volt)
		gpio_direction_input(buck_ctrl->buck_volt_ctrl);
}

static void gp_buck_en_ctrl(struct buck_device_t *buck_ctrl, int state)
{
	if (!buck_ctrl)
		return;

	hwlog_info("%s state-%d\n", __func__, state);
	if (state)
		gpio_direction_output(buck_ctrl->buck_en_ctrl, 1);
	else
		gpio_direction_output(buck_ctrl->buck_en_ctrl, 0);
}

static void gp_buck_dis_ctrl(struct buck_device_t *buck_ctrl)
{
	if (!buck_ctrl)
		return;

	gpio_set_value(buck_ctrl->buck_en_ctrl, GPIO_CTRL_PIN_DISABLE);
}

int gp_buck_ctrl_en(u8 pmic_seq, u32 vol, int state)
{
	hwlog_info("pmic_seq-%u, vol-%u, state-%d\n",
		pmic_seq, vol, state);
	if (pmic_seq != VOUT_BUCK_1) {
		hwlog_err("%s pmic_seq ctrl err\n", __func__);
		return -EINVAL;
	}
	if ((state == pre_state) && (pre_vol == vol)) {
		hwlog_info("volt-%u state-%d not changed\n", vol, state);
		return 0;
	}
	if (state == 1) {
		gp_buck_vote_ctrl(&buck_device, vol);
		gp_buck_en_ctrl(&buck_device, state);
	} else {
		gp_buck_dis_ctrl(&buck_device);
	}
	return 0;
}

static int buck_ctrl_gpio_init(struct buck_device_t *buck_ctrl,
	struct device_node *node)
{
	int gpio_t;
	int ret;

	if (!buck_ctrl || !node)
		return -EINVAL;
	// read h_buck_volt
	ret = of_property_read_u32(node, "huawei,h_buck_volt",
		&buck_ctrl->h_buck_volt);
	if (ret) {
		hwlog_err("h_buck_volt parse error\n");
		return -EINVAL;
	}
	hwlog_info("h_buck_volt = %u\n", buck_ctrl->h_buck_volt);
	// read l_buck_volt
	ret = of_property_read_u32(node, "huawei,l_buck_volt",
		&buck_ctrl->l_buck_volt);
	if (ret) {
		hwlog_warn("l_buck_volt parse error\n");
		buck_ctrl->l_buck_volt = 0;
	}
	hwlog_info("l_buck_volt = %u\n", buck_ctrl->l_buck_volt);
	// get gpio num
	gpio_t = of_get_named_gpio(node, "huawei,buck_en_ctrl", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("weld pole gpio0 is invalid\n");
		return -EINVAL;
	}
	buck_ctrl->buck_en_ctrl = (unsigned int)gpio_t;
	hwlog_info("buck en ctrl = %u\n", buck_ctrl->buck_en_ctrl);
	// req gpio
	ret = gpio_request(buck_ctrl->buck_en_ctrl, "buck_en_ctrl");
	if (ret < 0) {
		hwlog_err("request buck_en_ctrl gpio err\n");
		return ret;
	}

	gpio_t = of_get_named_gpio(node, "huawei,buck_volt_ctrl", 0);
	if (!gpio_is_valid(gpio_t)) {
		hwlog_err("buck_volt_ctrl is invalid, not support\n");
		gpio_t = 0;
	}
	buck_ctrl->buck_volt_ctrl = (unsigned int)gpio_t;
	hwlog_info("buck volt ctrl = %u\n", buck_ctrl->buck_volt_ctrl);

	if (buck_ctrl->buck_volt_ctrl == 0) {
		hwlog_err("not support buck_volt_ctrl\n");
		return 0;
	}

	ret = gpio_request(buck_ctrl->buck_volt_ctrl, "buck_volt_ctrl");
	if (ret < 0) {
		hwlog_err("request buck_volt_ctrl gpio err\n");
		return ret;
	}

	return 0;
}

static int buck_probe(struct platform_device *pdev)
{
	struct device_node *temp = NULL;
	int ret;

	hwlog_info("%s in\n", __func__);
	temp = pdev->dev.of_node;

	ret = buck_ctrl_gpio_init(&buck_device, temp);
	if (ret) {
		hwlog_err("buck_ctrl_gpio_init failed\n");
		return ret;
	}

	hwlog_info("%s probe success\n", __func__);
	return 0;
}

static int buck_remove(struct platform_device *pdev)
{
	if (!pdev)
		return 0;

	if (buck_device.buck_en_ctrl != 0)
		gpio_free(buck_device.buck_en_ctrl);

	if (buck_device.buck_volt_ctrl != 0)
		gpio_free(buck_device.buck_volt_ctrl);
	return 0;
}

struct platform_driver buck_driver = {
	.probe = buck_probe,
	.remove = buck_remove,
	.driver = {
		.name = "gpio_ctrl_buck",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(gp_buck_match_table),
	},
};

static int __init gpio_buck_init(void)
{
	hwlog_info("init\n");
	return platform_driver_register(&buck_driver);
}

static void __exit gpio_buck_exit(void)
{
	hwlog_info("exit\n");
	platform_driver_unregister(&buck_driver);
}

module_init(gpio_buck_init);
module_exit(gpio_buck_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("gpio_buck driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
