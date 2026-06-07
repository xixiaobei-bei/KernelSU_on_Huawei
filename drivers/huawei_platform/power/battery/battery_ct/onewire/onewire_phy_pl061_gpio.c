/*
 * onewire_phy_pl061_gpio_v2.c
 *
 * this is for maxim onewire ic
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

#include <huawei_platform/power/battery_type_identify.h>

#include "onewire_common.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_onewire_phy
HWLOG_REGIST();

static struct ow_gpio_des pl061_gpio;

static struct ow_treq *ow_trq;

static const char gpio_name[] = "onewire_phy_pl061_gpio";

/* onewire reset operation */
static unsigned char onewire_reset(void)
{
	unsigned char presence;
	unsigned long flags;

	raw_spin_lock_irqsave(pl061_gpio.lock, flags);
	get_current_gpio_bank_dir(&pl061_gpio);
	gpio_direction_output_unsafe(LOW_VOLTAGE, &pl061_gpio);
	__hw_delay(ow_trq->reset_init_low_cycles);
	gpio_set_value_unsafe(HIGH_VOLTAGE, &pl061_gpio);
	gpio_direction_input_unsafe(&pl061_gpio);
	__hw_delay(ow_trq->reset_slave_response_delay_cycles);
	presence = gpio_get_value_unsafe(&pl061_gpio);
	__hw_delay(ow_trq->reset_hold_high_cycles);
	raw_spin_unlock_irqrestore(pl061_gpio.lock, flags);

	return presence;
}

/* onewire bit operation */
static inline void onewire_read_bit(unsigned char *value)
{
	gpio_direction_output_unsafe(LOW_VOLTAGE, &pl061_gpio);
	__hw_delay(ow_trq->read_init_low_cycles);
	gpio_direction_input_unsafe(&pl061_gpio);
	*value = (*value) | (gpio_get_value_unsafe(&pl061_gpio) << SHIFT_7);
	__hw_delay(ow_trq->read_residual_cycles);
}

static inline void onewire_write_bit(const unsigned char bitval)
{
	gpio_direction_output_unsafe(LOW_VOLTAGE, &pl061_gpio);
	__hw_delay(ow_trq->write_init_low_cycles);
	gpio_set_value_unsafe(bitval, &pl061_gpio);
	__hw_delay(ow_trq->write_hold_cycles);
	gpio_set_value_unsafe(HIGH_VOLTAGE, &pl061_gpio);
	__hw_delay(ow_trq->write_residual_cycles);
}

/* onewire byte operation */
static unsigned char onewire_read_byte(void)
{
	unsigned char value = 0;
	unsigned long flags;

	raw_spin_lock_irqsave(pl061_gpio.lock, flags);
	get_current_gpio_bank_dir(&pl061_gpio);
	/* data from IC bit0 bit1 ... bit7 */
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	value >>= SHIFT_1;
	onewire_read_bit(&value);
	/* set gpio for powering eeprom program & hmac computation */
	gpio_direction_output_unsafe(HIGH_VOLTAGE, &pl061_gpio);
	raw_spin_unlock_irqrestore(pl061_gpio.lock, flags);

	return value;
}

static void onewire_write_byte(const unsigned char val)
{
	unsigned long flags;

	raw_spin_lock_irqsave(pl061_gpio.lock, flags);
	get_current_gpio_bank_dir(&pl061_gpio);
	onewire_write_bit(val & BIT_0);
	onewire_write_bit(val & BIT_1);
	onewire_write_bit(val & BIT_2);
	onewire_write_bit(val & BIT_3);
	onewire_write_bit(val & BIT_4);
	onewire_write_bit(val & BIT_5);
	onewire_write_bit(val & BIT_6);
	onewire_write_bit(val & BIT_7);
	raw_spin_unlock_irqrestore(pl061_gpio.lock, flags);
}

/* wait for onewire IC process  */
static void onewire_wait_for_ic(unsigned int ms)
{
	msleep(ms);
}

/* set up onewire time request */
static void onewire_set_time_request(struct ow_treq *ic_ow_trq)
{
	if (ic_ow_trq)
		ow_trq = ic_ow_trq;
	else
		hwlog_err("NULL point ic_ow_trq passed to %s\n", __func__);
}

static int pl061_gpio_register(struct ow_phy_ops *ow_phy_ops)
{
	ow_phy_ops->reset = onewire_reset;
	ow_phy_ops->read_byte = onewire_read_byte;
	ow_phy_ops->write_byte = onewire_write_byte;
	ow_phy_ops->set_time_request = onewire_set_time_request;
	ow_phy_ops->wait_for_ic = onewire_wait_for_ic;

	return ONEWIRE_PHY_SUCCESS;
}

static struct ow_phy_list pl061_gpio_reg_node;

static int onewire_phy_pl061_open_ic(void)
{
	unsigned long flags;

	raw_spin_lock_irqsave(pl061_gpio.lock, flags);
	get_current_gpio_bank_dir(&pl061_gpio);
	gpio_direction_output_unsafe(HIGH_VOLTAGE, &pl061_gpio);
	raw_spin_unlock_irqrestore(pl061_gpio.lock, flags);

	return 0;
}

static int onewire_phy_pl061_close_ic(void)
{
	unsigned long flags;

	raw_spin_lock_irqsave(pl061_gpio.lock, flags);
	get_current_gpio_bank_dir(&pl061_gpio);
	gpio_direction_input_unsafe(&pl061_gpio);
	raw_spin_unlock_irqrestore(pl061_gpio.lock, flags);

	return 0;
}

static const struct security_ic_ops onewire_phy_pl061_ops = {
	.open_ic = onewire_phy_pl061_open_ic,
	.close_ic = onewire_phy_pl061_close_ic,
};

static int onewire_phy_pl061_gpio_driver_probe(struct platform_device *pdev)
{
	unsigned int myid;
	int ret;

	if (!pdev)
		return ONEWIRE_GPIO_FAIL;
	hwlog_info("onewire_phy: pl061 gpio is probing\n");
	ret = get_gpio_phy_addr(pdev, &pl061_gpio);
	if (ret)
		return ret;
	ret = init_gpio_iomem_addr(pdev, &pl061_gpio);
	if (ret)
		return ret;
	ret = get_gpio_for_ow(pdev, &pl061_gpio, gpio_name);
	if (ret)
		return ret;


	ret = of_property_read_u32(pdev->dev.of_node, "phandle", &myid);
	if (ret) {
		hwlog_err("Can't find dts node phandle in %s\n", __func__);
		return ONEWIRE_GPIO_FAIL;
	}
	pl061_gpio_reg_node.onewire_phy_register = pl061_gpio_register;
	pl061_gpio_reg_node.gpio = &pl061_gpio;
	pl061_gpio_reg_node.dev_phandle = myid;
	add_to_onewire_phy_list(&pl061_gpio_reg_node);
	register_security_ic_ops(&onewire_phy_pl061_ops);
	hwlog_info("onewire_phy: pl061 gpio probed success\n");

	return ONEWIRE_PHY_SUCCESS;
}

static int onewire_phy_pl061_gpio_driver_remove(struct platform_device *pdev)
{
	unregister_security_ic_ops(&onewire_phy_pl061_ops);
	return ONEWIRE_PHY_SUCCESS;
}

static const struct of_device_id onewire_phy_pl061_gpio_match_table[] = {
	{ .compatible = "onewire-phy,pl061,gpio", },
	{ /* end */ },
};

static struct platform_driver onewire_phy_pl061_gpio_driver = {
	.probe = onewire_phy_pl061_gpio_driver_probe,
	.remove = onewire_phy_pl061_gpio_driver_remove,
	.driver = {
		.name = "onewire_phy_pl061_gpio",
		.owner = THIS_MODULE,
		.of_match_table = onewire_phy_pl061_gpio_match_table,
	},
};

int __init onewire_phy_pl061_gpio_driver_init(void)
{
	return platform_driver_register(&onewire_phy_pl061_gpio_driver);
}

void __exit onewire_phy_pl061_gpio_driver_exit(void)
{
	platform_driver_unregister(&onewire_phy_pl061_gpio_driver);
}

subsys_initcall_sync(onewire_phy_pl061_gpio_driver_init);
module_exit(onewire_phy_pl061_gpio_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("onewire physic controller");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
