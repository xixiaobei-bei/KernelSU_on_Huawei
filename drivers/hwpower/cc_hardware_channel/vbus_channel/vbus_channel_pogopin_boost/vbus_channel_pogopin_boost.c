/*
 * vbus_channel_pogopin_boost.c
 *
 * pogopin boost for vbus channel driver
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

#include <chipset_common/hwpower/hardware_channel/vbus_channel_pogopin_boost.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG vbus_ch_pogopin
HWLOG_REGIST();

static struct pogopin_boost_dev *g_pogopin_boost_dev;

static struct pogopin_boost_dev *pogopin_boost_get_dev(void)
{
	if (!g_pogopin_boost_dev) {
		hwlog_err("g_pogopin_boost_dev is null\n");
		return NULL;
	}

	return g_pogopin_boost_dev;
}

static int pogopin_boost_get_fault_status(struct pogopin_boost_dev *l_dev)
{
	if (l_dev->otg_sc_check_enable && vbus_ch_eh_get_otg_sc_flag()) {
		hwlog_info("otg sc happen, can not open pogopin_boost\n");
		return -1;
	}

	if (l_dev->otg_ocp_check_enable &&
		(gpio_get_value(l_dev->otg_ocp_int) == 0)) {
		hwlog_err("otg ocp happen, can not open pogopin_boost\n");
		return -1;
	}

	return 0;
}

static void pogopin_boost_start_fault_check(struct pogopin_boost_dev *l_dev,
	unsigned int user)
{
	if (l_dev->otg_sc_check_enable && (user == VBUS_CH_USER_PD))
		power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_SC_CHECK_START,
			&l_dev->otg_sc_check_para);
}

static void pogopin_boost_stop_fault_check(struct pogopin_boost_dev *l_dev,
	unsigned int user)
{
	if (l_dev->otg_sc_check_enable && (user == VBUS_CH_USER_PD))
		power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_SC_CHECK_STOP, NULL);
}

/* fix a hardware issue, otg boost over current protect */
static void pogopin_boost_otg_ocp_work(struct work_struct *work)
{
	struct pogopin_boost_dev *l_dev = pogopin_boost_get_dev();

	if (!l_dev)
		return;

	power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_OCP_HANDLE, &l_dev->otg_ocp_int);
}

static irqreturn_t pogopin_boost_otg_ocp_irq_handler(int irq, void *_l_dev)
{
	struct pogopin_boost_dev *l_dev = _l_dev;

	if (!l_dev)
		return IRQ_HANDLED;

	schedule_work(&l_dev->otg_ocp_work);
	return IRQ_HANDLED;
}

static void pogopin_boost_otg_ocp_irq_request(struct device_node *np,
	struct pogopin_boost_dev *l_dev)
{
	int ret;

	if (power_gpio_config_interrupt(np, "otg_ocp_int", "otg_ocp_int",
		&l_dev->otg_ocp_int, &l_dev->otg_ocp_irq))
		return;

	ret = request_irq(l_dev->otg_ocp_irq,
		pogopin_boost_otg_ocp_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		"otg_ocp_irq", l_dev);
	if (ret) {
		hwlog_err("otg ocp irq request fail\n");
		gpio_free(l_dev->otg_ocp_int);
		return;
	}
}

static void pogopin_boost_5v_enable(struct pogopin_boost_dev *l_dev,
	bool enable)
{
	if (!l_dev)
		return;

	if (l_dev->otg_en_flag)
		boost_5v_enable(enable, BOOST_CTRL_BOOST_GPIO_OTG);
	else
		gpio_set_value(l_dev->buckboost_en, enable);
}

static int pogopin_boost_start_config(void)
{
	struct pogopin_boost_dev *l_dev = pogopin_boost_get_dev();

	if (!l_dev)
		return -EINVAL;

	/* open mos between charger and buckboost */
	gpio_set_value(l_dev->mos_en, MOS_GPIO_SWITCH_ENABLE);

	if (pogopin_boost_get_fault_status(l_dev))
		return 0;

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl) {
		if (power_platform_get_charger_type() == CHARGER_REMOVED) {
			hwlog_info("fix hw issue: open charger otg\n");
			vbus_ch_open(VBUS_CH_USER_WIRED_OTG, VBUS_CH_TYPE_CHARGER,
				false);
		}
	}
	/* open buckboost */
	pogopin_boost_5v_enable(l_dev, BOOST_5V_ENABLE);
	if (l_dev->otg_en_flag)
		gpio_set_value(l_dev->otg_en, OTG_GPIO_SWITCH_ENABLE);

	return 0;
}

static int pogopin_boost_stop_config(void)
{
	struct pogopin_boost_dev *l_dev = pogopin_boost_get_dev();

	if (!l_dev)
		return -EINVAL;

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl) {
		hwlog_info("fix hw issue: close charger otg\n");
		vbus_ch_close(VBUS_CH_USER_WIRED_OTG, VBUS_CH_TYPE_CHARGER,
			false, false);
	}

	if (l_dev->otg_en_flag)
		gpio_set_value(l_dev->otg_en, OTG_GPIO_SWITCH_DISABLE);

	/* close buckboost */
	pogopin_boost_5v_enable(l_dev, BOOST_5V_DISABLE);
	/* wait 160ms for buckboost real closed */
	msleep(160);
	/* open mos between charger and buckboost */
	gpio_set_value(l_dev->mos_en, MOS_GPIO_SWITCH_DISABLE);

	return 0;
}

static int pogopin_boost_open(unsigned int user, int flag)
{
	struct pogopin_boost_dev *l_dev = pogopin_boost_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (pogopin_boost_start_config())
		return -EINVAL;

	l_dev->user |= (1 << user);
	pogopin_boost_start_fault_check(l_dev, user);

	hwlog_info("user=%x open ok\n", l_dev->user);
	return 0;
}

static int pogopin_boost_close(unsigned int user, int flag, int force)
{
	struct pogopin_boost_dev *l_dev = pogopin_boost_get_dev();

	if (!l_dev)
		return -EINVAL;

	l_dev->user &= (~(unsigned int)(1 << user));

	if (l_dev->user == VBUS_CH_NO_OP_USER) {
		if (pogopin_boost_stop_config())
			return -EINVAL;
	}
	pogopin_boost_stop_fault_check(l_dev, user);

	hwlog_info("user=%x close ok\n", l_dev->user);
	return 0;
}

static int pogopin_boost_get_state(unsigned int user, int *state)
{
	struct pogopin_boost_dev *l_dev = pogopin_boost_get_dev();

	if (!l_dev || !state)
		return -EINVAL;

	if (l_dev->user == VBUS_CH_NO_OP_USER)
		*state = VBUS_CH_STATE_CLOSE;
	else
		*state = VBUS_CH_STATE_OPEN;

	return 0;
}

static struct vbus_ch_ops pogopin_boost_ops = {
	.type_name = "pogopin_boost",
	.open = pogopin_boost_open,
	.close = pogopin_boost_close,
	.get_state = pogopin_boost_get_state,
	.get_mode = NULL,
	.set_switch_mode = NULL,
	.set_voltage = NULL,
	.get_voltage = NULL,
};

static int pogopin_boost_parse_dts(struct device_node *np,
	struct pogopin_boost_dev *l_dev)
{
	if (power_gpio_config_output(np,
		"vbus_mos_ctrl_gpio", "vbus_mos_ctrl_gpio",
		&l_dev->mos_en, MOS_GPIO_SWITCH_DISABLE))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG),
		np, "pogopin_otg_en_flag", &l_dev->otg_en_flag, 0);

	if (!l_dev->otg_en_flag) {
		if (power_gpio_config_output(np,
			"buckboost_gpio", "buckboost_gpio",
			&l_dev->buckboost_en, BOOST_5V_DISABLE)) {
			gpio_free(l_dev->mos_en);
			return -EINVAL;
		}
	}

	if (l_dev->otg_en_flag && power_gpio_config_output(np,
		"pogopin_otg_en", "pogopin_otg_en",
		&l_dev->otg_en, OTG_GPIO_SWITCH_DISABLE)) {
		gpio_free(l_dev->mos_en);
		return -EINVAL;
	}

	/* fix a hardware issue, has leakage when open boost gpio */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG),
		np, "pogopin_charge_otg_ctl", &l_dev->charge_otg_ctl, 0);
	/* fix a hardware issue, has short circuit when open boost gpio */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_sc_check_enable", &l_dev->otg_sc_check_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_sc_vol_mv", &l_dev->otg_sc_check_para.vol_mv,
		BOOST_GPIO_OTG_SC_VOL_MV);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_sc_check_times", &l_dev->otg_sc_check_para.check_times,
		BOOST_GPIO_OTG_SC_CHECK_TIMES);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_sc_check_delayed_time",
		&l_dev->otg_sc_check_para.delayed_time,
		BOOST_GPIO_OTG_SC_CHECK_DELAYED_TIME);

	/* fix a hardware issue, otg boost over current protect */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_ocp_check_enable", &l_dev->otg_ocp_check_enable, 0);

	return 0;
}

static int pogopin_boost_probe(struct platform_device *pdev)
{
	int ret;
	struct pogopin_boost_dev *l_dev = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = devm_kzalloc(&pdev->dev, sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_pogopin_boost_dev = l_dev;
	l_dev->pdev = pdev;
	l_dev->dev = &pdev->dev;

	ret = pogopin_boost_parse_dts(l_dev->dev->of_node, l_dev);
	if (ret)
		goto fail_parse_dts;

	ret = vbus_ch_ops_register(&pogopin_boost_ops);
	if (ret)
		goto fail_register_ops;

	/* fix a hardware issue, otg boost over current protect */
	if (l_dev->otg_ocp_check_enable) {
		INIT_WORK(&l_dev->otg_ocp_work, pogopin_boost_otg_ocp_work);
		pogopin_boost_otg_ocp_irq_request(l_dev->dev->of_node, l_dev);
	}

	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_register_ops:
	gpio_free(l_dev->mos_en);
	if (!l_dev->otg_en_flag)
		gpio_free(l_dev->buckboost_en);
	if (l_dev->otg_en_flag)
		gpio_free(l_dev->otg_en);
fail_parse_dts:
	devm_kfree(&pdev->dev, l_dev);
	g_pogopin_boost_dev = NULL;
	return ret;
}

static int pogopin_boost_remove(struct platform_device *pdev)
{
	struct pogopin_boost_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -EINVAL;

	if (l_dev->mos_en)
		gpio_free(l_dev->mos_en);

	if (!l_dev->otg_en_flag && l_dev->buckboost_en)
		gpio_free(l_dev->buckboost_en);

	if (l_dev->otg_en_flag && l_dev->otg_en)
		gpio_free(l_dev->otg_en);

	if (l_dev->otg_ocp_check_enable && l_dev->otg_ocp_int)
		gpio_free(l_dev->otg_ocp_int);

	if (l_dev->otg_ocp_check_enable && l_dev->otg_ocp_irq)
		free_irq(l_dev->otg_ocp_irq, l_dev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, l_dev);
	g_pogopin_boost_dev = NULL;
	return 0;
}

static const struct of_device_id pogopin_boost_match_table[] = {
	{
		.compatible = "huawei,vbus_channel_pogopin_boost",
		.data = NULL,
	},
	{},
};

static struct platform_driver pogopin_boost_driver = {
	.probe = pogopin_boost_probe,
	.remove = pogopin_boost_remove,
	.driver = {
		.name = "huawei,vbus_channel_pogopin_boost",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pogopin_boost_match_table),
	},
};

static int __init pogopin_boost_init(void)
{
	return platform_driver_register(&pogopin_boost_driver);
}

static void __exit pogopin_boost_exit(void)
{
	platform_driver_unregister(&pogopin_boost_driver);
}

fs_initcall(pogopin_boost_init);
module_exit(pogopin_boost_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("pogopin boost for vbus channel driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
