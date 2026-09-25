/*
 * onewire_phy_pl061_gpio_v2.c
 *
 * this is for nxp onewire ic
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

#include "onewire_common.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG hw_onewire_phy_v2
HWLOG_REGIST();

static const char gpio_name[] = "onewire_v2";

/* onewire reset operation */
static void onewire_reset(struct ow_gpio_des *gpio, struct ow_treq_v2 *ow_trq)
{
	unsigned long flags;

	if (!gpio || !ow_trq)
		return;
	raw_spin_lock_irqsave(gpio->lock, flags);
	get_current_gpio_bank_dir(gpio);
	gpio_direction_output_unsafe(LOW_VOLTAGE, gpio);
	__hw_delay(ow_trq->reset_low_cycs);
	gpio_set_value_unsafe(HIGH_VOLTAGE, gpio);
	raw_spin_unlock_irqrestore(gpio->lock, flags);
	mdelay(ow_trq->reset_us / 1000);
}

/* onewire stop operation */
static inline void onewire_stop(struct ow_gpio_des *gpio,
				struct ow_treq_v2 *ow_trq)
{
	gpio_direction_output_unsafe(LOW_VOLTAGE, gpio);
	__hw_delay_v2(ow_trq->stop_low_cycs);
	gpio_set_value_unsafe(HIGH_VOLTAGE, gpio);
	__hw_delay(ow_trq->stop_high_cycs);
}

/* onewire bit operation */
static unsigned char onewire_read_bit(unsigned char *value,
				      struct ow_gpio_des *gpio,
				      struct ow_treq_v2 *ow_trq)
{
	gpio_direction_output_unsafe(LOW_VOLTAGE, gpio);
	__hw_delay_v2(ow_trq->read_sample_cycs);
	gpio_direction_input_unsafe(gpio);
	*value = gpio_get_value_unsafe(gpio);
	__hw_delay(ow_trq->read_rest_cycs);

	return 0;
}

static void onewire_write_bit(const unsigned char val,
			      struct ow_gpio_des *gpio,
			      struct ow_treq_v2 *ow_trq)
{
	gpio_direction_output_unsafe(LOW_VOLTAGE, gpio);
	__hw_delay_v2(val ? ow_trq->write_high_cycs : ow_trq->write_low_cycs);
	gpio_set_value_unsafe(HIGH_VOLTAGE, gpio);
	__hw_delay(ow_trq->write_bit_cycs -
		   (val ? ow_trq->write_high_cycs : ow_trq->write_low_cycs));
}

static inline void onewire_start_bit(struct ow_gpio_des *gpio,
				     struct ow_treq_v2 *ow_trq)
{
	onewire_write_bit(0, gpio, ow_trq);
}

/* onewire byte operation */
static unsigned char onewire_read_byte(const unsigned char end,
				       struct ow_gpio_des *gpio,
				       struct ow_treq_v2 *ow_trq,
				       unsigned char *data)
{
	unsigned char temp = 0;
	unsigned long flags;
	char i;

	if (!gpio || !ow_trq || !data)
		return 1;
	raw_spin_lock_irqsave(gpio->lock, flags);
	get_current_gpio_bank_dir(gpio);
	/* data from IC bit7 bit6 ... bit0 */
	*data = 0;
	for (i = 8; i > 0; i--) {
		onewire_read_bit(&temp, gpio, ow_trq);
		*data |= temp << (i - 1);
	}
	/* A1007 need write an low ack bit at end of read byte */
	onewire_write_bit(end, gpio, ow_trq);
	if (end)
		onewire_stop(gpio, ow_trq);
	raw_spin_unlock_irqrestore(gpio->lock, flags);

	return 0;
}

static inline void __onewire_writeb(const unsigned char val,
				    struct ow_gpio_des *gpio,
				    struct ow_treq_v2 *ow_trq)
{
	/* date to IC bit7 bit6 bit5...bit0 */
	onewire_write_bit(val & BIT_7, gpio, ow_trq);
	onewire_write_bit(val & BIT_6, gpio, ow_trq);
	onewire_write_bit(val & BIT_5, gpio, ow_trq);
	onewire_write_bit(val & BIT_4, gpio, ow_trq);
	onewire_write_bit(val & BIT_3, gpio, ow_trq);
	onewire_write_bit(val & BIT_2, gpio, ow_trq);
	onewire_write_bit(val & BIT_1, gpio, ow_trq);
	onewire_write_bit(val & BIT_0, gpio, ow_trq);
}

static unsigned char onewire_start_byte(const unsigned char val,
					struct ow_gpio_des *gpio,
					struct ow_treq_v2 *ow_trq)
{
	unsigned long flags;
	unsigned char nack = 0;

	if (!gpio || !ow_trq)
		return 1;
	raw_spin_lock_irqsave(gpio->lock, flags);
	get_current_gpio_bank_dir(gpio);
	onewire_stop(gpio, ow_trq);
	onewire_start_bit(gpio, ow_trq);
	__onewire_writeb(val, gpio, ow_trq);
	/* A1007 need read an low ack bit at end of read byte */
	if (onewire_read_bit(&nack, gpio, ow_trq))
		nack = 1;
	if (nack)
		onewire_stop(gpio, ow_trq);
	raw_spin_unlock_irqrestore(gpio->lock, flags);

	return nack;
}

static unsigned char onewire_write_byte(const unsigned char end,
					const unsigned char val,
					struct ow_gpio_des *gpio,
					struct ow_treq_v2 *ow_trq)
{
	unsigned long flags;
	unsigned char nack = 0;

	if (!gpio || !ow_trq)
		return 1;
	raw_spin_lock_irqsave(gpio->lock, flags);
	get_current_gpio_bank_dir(gpio);
	__onewire_writeb(val, gpio, ow_trq);
	/* A1007 need read an low ack bit at end of read byte */
	if (onewire_read_bit(&nack, gpio, ow_trq))
		nack = 1;
	if (nack || end)
		onewire_stop(gpio, ow_trq);
	raw_spin_unlock_irqrestore(gpio->lock, flags);

	return nack;
}

/* wait for onewire IC process  */
static void onewire_wait_for_ic(unsigned int ms)
{
	msleep(ms);
}

/* wait for onewire IC to idle  */
static void onewire_soft_reset(struct ow_gpio_des *gpio,
			       struct ow_treq_v2 *ow_trq)
{
	unsigned long flags;

	if (!gpio || !ow_trq)
		return;
	raw_spin_lock_irqsave(gpio->lock, flags);
	get_current_gpio_bank_dir(gpio);
	gpio_direction_output_unsafe(HIGH_VOLTAGE, gpio);
	__hw_delay(ow_trq->soft_reset_cycs / 2);
	gpio_set_value_unsafe(LOW_VOLTAGE, gpio);
	__hw_delay_v2(ow_trq->soft_reset_cycs);
	gpio_set_value_unsafe(HIGH_VOLTAGE, gpio);
	__hw_delay(ow_trq->soft_reset_cycs / 2);
	raw_spin_unlock_irqrestore(gpio->lock, flags);
}

static int pl061_gpio_register_v2(struct ow_phy_ops_v2 *ow_phy_ops)
{
	ow_phy_ops->reset = onewire_reset;
	ow_phy_ops->read_byte = onewire_read_byte;
	ow_phy_ops->write_byte = onewire_write_byte;
	ow_phy_ops->start = onewire_start_byte;
	ow_phy_ops->wait_for_ic = onewire_wait_for_ic;
	ow_phy_ops->soft_reset = onewire_soft_reset;

	return ONEWIRE_PHY_SUCCESS;
}

static int ow_driver_probe(struct platform_device *pdev)
{
	int ret;
	unsigned int myid;
	struct ow_gpio_des *pl061_gpio = NULL;
	struct ow_phy_list *reg_node = NULL;

	if (!pdev)
		return ONEWIRE_GPIO_FAIL;
	hwlog_info("onewire_phy: pl061 gpio is probing\n");
	pl061_gpio = devm_kzalloc(&pdev->dev, sizeof(*pl061_gpio), GFP_KERNEL);
	if (!pl061_gpio) {
		hwlog_err("%s alloc for pl061_gpio failed\n", pdev->name);
		return ONEWIRE_GPIO_FAIL;
	}
	reg_node = devm_kzalloc(&pdev->dev, sizeof(*reg_node), GFP_KERNEL);
	if (!reg_node)
		return ONEWIRE_GPIO_FAIL;
	platform_set_drvdata(pdev, pl061_gpio);

	ret = get_gpio_phy_addr(pdev, pl061_gpio);
	if (ret)
		return ret;
	ret = init_gpio_iomem_addr(pdev, pl061_gpio);
	if (ret)
		return ret;
	ret = get_gpio_for_ow(pdev, pl061_gpio, gpio_name);
	if (ret)
		return ret;

	ret = of_property_read_u32(pdev->dev.of_node, "phandle", &myid);
	if (ret) {
		hwlog_err("Can't find device dts node phandle in %s\n",
			  __func__);
		return ONEWIRE_GPIO_FAIL;
	}
	reg_node->onewire_phy_register_v2 = pl061_gpio_register_v2;
	reg_node->gpio = pl061_gpio;
	reg_node->dev_phandle = myid;
	add_to_onewire_phy_list(reg_node);
	hwlog_info("onewire_phy: pl061 gpio probed success\n");

	return ONEWIRE_PHY_SUCCESS;
}

static int ow_driver_remove(struct platform_device *pdev)
{
	return ONEWIRE_PHY_SUCCESS;
}

static const struct of_device_id ow_v2_match_table[] = {
	{ .compatible = "onewire-phy-v2,pl061,gpio", },
	{ /* end */ },
};

static struct platform_driver onewire_v2_driver = {
	.probe = ow_driver_probe,
	.remove = ow_driver_remove,
	.driver = {
		.name = "onewire_phy_pl061_gpio_v2",
		.owner = THIS_MODULE,
		.of_match_table = ow_v2_match_table,
	},
};

int __init ow_v2_driver_init(void)
{
	return platform_driver_register(&onewire_v2_driver);
}

void __exit ow_v2_driver_exit(void)
{
	platform_driver_unregister(&onewire_v2_driver);
}

subsys_initcall_sync(ow_v2_driver_init);
module_exit(ow_v2_driver_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("onewire physic controller V2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
