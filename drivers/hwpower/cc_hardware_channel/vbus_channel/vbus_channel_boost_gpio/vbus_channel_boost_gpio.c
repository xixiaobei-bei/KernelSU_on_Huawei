/*
 * vbus_channel_boost_gpio.c
 *
 * boost gpio for vbus channel driver
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

#include <chipset_common/hwpower/hardware_channel/vbus_channel_boost_gpio.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel.h>
#include <chipset_common/hwpower/common_module/power_gpio.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <chipset_common/hwpower/hardware_channel/wired_channel_switch.h>
#include <chipset_common/hwpower/common_module/power_event_ne.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <huawei_platform/power/wireless/wireless_tx_pwr_ctrl.h>
#include <chipset_common/hwpower/common_module/power_printk.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_dts.h>

#define HWLOG_TAG vbus_ch_boost_gpio
HWLOG_REGIST();

static struct boost_gpio_dev *g_boost_gpio_dev;

static struct boost_gpio_dev *boost_gpio_get_dev(void)
{
	if (!g_boost_gpio_dev) {
		hwlog_err("g_boost_gpio_dev is null\n");
		return NULL;
	}

	return g_boost_gpio_dev;
}

static int boost_gpio_get_fault_status(struct boost_gpio_dev *l_dev)
{
	if (l_dev->otg_sc_check_enable && vbus_ch_eh_get_otg_sc_flag()) {
		hwlog_info("otg sc happen, can not open boost_gpio\n");
		return -1;
	}

	if (l_dev->otg_ocp_check_enable &&
		(gpio_get_value(l_dev->otg_ocp_int) == 0)) {
		hwlog_err("otg ocp happen, can not open boost_gpio\n");
		return -1;
	}

	return 0;
}

static void boost_gpio_start_fault_check(struct boost_gpio_dev *l_dev,
	unsigned int user)
{
	if (l_dev->otg_sc_check_enable && (user == VBUS_CH_USER_PD))
		power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_SC_CHECK_START,
			&l_dev->otg_sc_check_para);
}

static void boost_gpio_stop_fault_check(struct boost_gpio_dev *l_dev,
	unsigned int user)
{
	if (l_dev->otg_sc_check_enable && (user == VBUS_CH_USER_PD))
		power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_SC_CHECK_STOP,
			NULL);
}

/* fix a hardware issue, has leakage when open boost gpio */
static void boost_gpio_charge_otg_close_work(struct work_struct *w)
{
	hwlog_info("fix hw issue: close charger otg on work\n");
	vbus_ch_close(VBUS_CH_USER_WIRED_OTG, VBUS_CH_TYPE_CHARGER,
		false, false);
}

/* fix a hardware issue, otg boost over current protect */
static void boost_gpio_otg_ocp_work(struct work_struct *work)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();

	if (!l_dev)
		return;

	power_event_bnc_notify(POWER_BNT_OTG, POWER_NE_OTG_OCP_HANDLE, &l_dev->otg_ocp_int);
}

static irqreturn_t boost_gpio_otg_ocp_irq_handler(int irq, void *_l_dev)
{
	struct boost_gpio_dev *l_dev = _l_dev;

	if (!l_dev)
		return IRQ_HANDLED;

	schedule_work(&l_dev->otg_ocp_work);
	return IRQ_HANDLED;
}

static void boost_gpio_otg_ocp_irq_request(struct device_node *np,
	struct boost_gpio_dev *l_dev)
{
	int ret;

	if (power_gpio_config_interrupt(np,
		"otg_ocp_int", "otg_ocp_int",
		&l_dev->otg_ocp_int, &l_dev->otg_ocp_irq))
		return;

	ret = request_irq(l_dev->otg_ocp_irq,
		boost_gpio_otg_ocp_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		"otg_ocp_irq", l_dev);
	if (ret) {
		hwlog_err("otg ocp irq request fail\n");
		gpio_free(l_dev->otg_ocp_int);
		return;
	}
}

static int boost_gpio_start_config(int flag)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (boost_gpio_get_fault_status(l_dev))
		return 0;

	l_dev->mode = VBUS_CH_IN_OTG_MODE;

	wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_CUTOFF);
	wireless_tx_cancel_work(PWR_SW_BY_OTG_ON);

	msleep(100); /* delay 100ms for hardware */
	boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_BOOST_GPIO_OTG);
	power_usleep(DT_USLEEP_10MS);
	gpio_set_value(l_dev->gpio_en, BOOST_GPIO_SWITCH_ENABLE);

	wireless_tx_restart_check(PWR_SW_BY_OTG_ON);

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl_flag) {
		hwlog_info("fix hw issue: open charger otg\n");
		vbus_ch_open(VBUS_CH_USER_WIRED_OTG, VBUS_CH_TYPE_CHARGER,
			false);
		cancel_delayed_work_sync(&l_dev->charge_otg_close_work);
		schedule_delayed_work(&l_dev->charge_otg_close_work,
			msecs_to_jiffies(CHARGE_OTG_CLOSE_WORK_TIMEOUT));
	}

	hwlog_info("start reverse_vbus flag=%d\n", flag);
	return 0;
}

static int boost_gpio_stop_config(int flag)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();
	struct wlrx_dts *dts = wlrx_get_dts();

	if (!l_dev)
		return -EINVAL;

	if (flag) {
		l_dev->mode = VBUS_CH_NOT_IN_OTG_MODE;
		wireless_tx_cancel_work(PWR_SW_BY_OTG_OFF);
	}

	gpio_set_value(l_dev->gpio_en, BOOST_GPIO_SWITCH_DISABLE);
	power_usleep(DT_USLEEP_10MS);
	boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_BOOST_GPIO_OTG);

	if (flag)
		wireless_tx_restart_check(PWR_SW_BY_OTG_OFF);

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl_flag) {
		hwlog_info("fix hw issue: close charger otg\n");
		vbus_ch_close(VBUS_CH_USER_WIRED_OTG, VBUS_CH_TYPE_CHARGER,
			false, false);
		cancel_delayed_work_sync(&l_dev->charge_otg_close_work);
	}

	if ((power_platform_get_charger_type() != CHARGER_TYPE_WIRELESS) &&
		dts && dts->wired_sw_dflt_on)
		wired_chsw_set_wired_channel(WIRED_CHANNEL_ALL, WIRED_CHANNEL_RESTORE);

	hwlog_info("stop reverse_vbus flag=%d\n", flag);
	return 0;
}

static int boost_gpio_open(unsigned int user, int flag)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (boost_gpio_start_config(flag))
		return -EINVAL;

	l_dev->user |= (1 << user);
	boost_gpio_start_fault_check(l_dev, user);

	hwlog_info("user=%x open ok\n", l_dev->user);
	return 0;
}

static int boost_gpio_close(unsigned int user, int flag, int force)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();

	if (!l_dev)
		return -EINVAL;

	if (force) {
		if (boost_gpio_stop_config(flag))
			return -EINVAL;

		hwlog_info("user=%x force close ok\n", l_dev->user);
		return 0;
	}

	l_dev->user &= (~(unsigned int)(1 << user));

	if (l_dev->user == VBUS_CH_NO_OP_USER) {
		if (boost_gpio_stop_config(flag))
			return -EINVAL;
	}

	boost_gpio_stop_fault_check(l_dev, user);

	hwlog_info("user=%x close ok\n", l_dev->user);
	return 0;
}

static int boost_gpio_get_state(unsigned int user, int *state)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();

	if (!l_dev || !state)
		return -EINVAL;

	if (l_dev->user == VBUS_CH_NO_OP_USER)
		*state = VBUS_CH_STATE_CLOSE;
	else
		*state = VBUS_CH_STATE_OPEN;

	return 0;
}

static int boost_gpio_get_mode(unsigned int user, int *mode)
{
	struct boost_gpio_dev *l_dev = boost_gpio_get_dev();

	if (!l_dev || !mode)
		return -EINVAL;

	*mode = l_dev->mode;

	return 0;
}

static struct vbus_ch_ops boost_gpio_ops = {
	.type_name = "boost_gpio",
	.open = boost_gpio_open,
	.close = boost_gpio_close,
	.get_state = boost_gpio_get_state,
	.get_mode = boost_gpio_get_mode,
	.set_switch_mode = NULL,
	.set_voltage = NULL,
	.get_voltage = NULL,
};

static int boost_gpio_parse_dts(struct device_node *np,
	struct boost_gpio_dev *l_dev)
{
	if (power_gpio_config_output(np,
		"gpio_otg_switch", "gpio_otg_switch",
		&l_dev->gpio_en, BOOST_GPIO_SWITCH_DISABLE))
		return -EINVAL;

	/* fix a hardware issue, has leakage when open boost gpio */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"charge_otg_ctl_flag", &l_dev->charge_otg_ctl_flag, 0);
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

static int boost_gpio_probe(struct platform_device *pdev)
{
	int ret;
	struct boost_gpio_dev *l_dev = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = devm_kzalloc(&pdev->dev, sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	g_boost_gpio_dev = l_dev;
	l_dev->pdev = pdev;
	l_dev->dev = &pdev->dev;

	ret = boost_gpio_parse_dts(l_dev->dev->of_node, l_dev);
	if (ret)
		goto fail_parse_dts;

	ret = vbus_ch_ops_register(&boost_gpio_ops);
	if (ret)
		goto fail_register_ops;

	/* fix a hardware issue, has leakage when open boost gpio */
	if (l_dev->charge_otg_ctl_flag)
		INIT_DELAYED_WORK(&l_dev->charge_otg_close_work,
			boost_gpio_charge_otg_close_work);

	/* fix a hardware issue, otg boost over current protect */
	if (l_dev->otg_ocp_check_enable) {
		INIT_WORK(&l_dev->otg_ocp_work, boost_gpio_otg_ocp_work);
		boost_gpio_otg_ocp_irq_request(l_dev->dev->of_node, l_dev);
	}

	platform_set_drvdata(pdev, l_dev);
	return 0;

fail_register_ops:
	gpio_free(l_dev->gpio_en);
fail_parse_dts:
	devm_kfree(&pdev->dev, l_dev);
	g_boost_gpio_dev = NULL;
	return ret;
}

static int boost_gpio_remove(struct platform_device *pdev)
{
	struct boost_gpio_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -EINVAL;

	if (l_dev->gpio_en)
		gpio_free(l_dev->gpio_en);

	if (l_dev->otg_ocp_check_enable && l_dev->otg_ocp_int)
		gpio_free(l_dev->otg_ocp_int);

	if (l_dev->otg_ocp_check_enable && l_dev->otg_ocp_irq)
		free_irq(l_dev->otg_ocp_irq, l_dev);

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, l_dev);
	g_boost_gpio_dev = NULL;
	return 0;
}

static const struct of_device_id boost_gpio_match_table[] = {
	{
		.compatible = "huawei,vbus_channel_boost_gpio",
		.data = NULL,
	},
	{},
};

static struct platform_driver boost_gpio_driver = {
	.probe = boost_gpio_probe,
	.remove = boost_gpio_remove,
	.driver = {
		.name = "huawei,vbus_channel_boost_gpio",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(boost_gpio_match_table),
	},
};

static int __init boost_gpio_init(void)
{
	return platform_driver_register(&boost_gpio_driver);
}

static void __exit boost_gpio_exit(void)
{
	platform_driver_unregister(&boost_gpio_driver);
}

fs_initcall(boost_gpio_init);
module_exit(boost_gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("boost gpio for vbus channel module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
