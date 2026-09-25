/*
 * onewire_common.c
 *
 * this is for all onewire phy
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
#define HWLOG_TAG onewire_commom
HWLOG_REGIST();

struct list_head ow_phy_reg_head = LIST_HEAD_INIT(ow_phy_reg_head);

void add_to_onewire_phy_list(struct ow_phy_list *entry)
{
	if (!entry)
		return;
	list_add_tail(&entry->node, &ow_phy_reg_head);
}

struct list_head *get_owi_phy_list_head(void)
{
	return &ow_phy_reg_head;
}

/* private delay -- almost same as kernel 4.4 __delay -- using it carefully */
void __hw_delay(unsigned long cycles)
{
	cycles_t start = get_cycles();

	while ((get_cycles() - start) < cycles)
		;
}

/* private delay v2 wmb may delay extra cycles() */
void __hw_delay_v2(unsigned long cycles)
{
	cycles_t start;

	/* make sure IO write finished */
	wmb();
	cycles = (cycles > 1) ? (cycles - 1) : cycles;
	start = get_cycles();
	while ((get_cycles() - start) < cycles)
		;
}

/*
 *It will set whole bank gpio to a state without checking current gpio bank
 *direction state.
 *Caution: It's only can be use in the safe environment & param in!
 *         Time sensitive!!!
 */
void gpio_direction_output_unsafe(unsigned char value, struct ow_gpio_des *gpio)
{
	writeb(gpio->gpio_dir_out_data, gpio->gpio_dir_addr);
	writeb(!!value << (gpio->offset), gpio->gpio_data_addr);
}

/*
 *It will set whole bank gpio to a state without checking current gpio
 *bank direction state.
 *Caution: It's only can be use in the safe environment & param in!
 *         Time sensitive!!!
 */
void gpio_direction_input_unsafe(struct ow_gpio_des *gpio)
{
	writeb(gpio->gpio_dir_in_data, gpio->gpio_dir_addr);
}

/*
 *Set value to gpio value register without checking current gpio direction state
 *Caution: It's only can be use in the safe environment & param in!
 *         Time sensitive!!!
 */
void gpio_set_value_unsafe(unsigned char value, struct ow_gpio_des *gpio)
{
	writeb(!!value << (gpio->offset), gpio->gpio_data_addr);
}

/*
 *Read value from gpio value register without checking current gpio
 *direction state.
 *Caution: It's only can be use in the safe environment & param in!
 *         Time sensitive!!!
 */
unsigned char gpio_get_value_unsafe(struct ow_gpio_des *gpio)
{
	return !!readb(gpio->gpio_data_addr);
}
/*
 *Get current gpio bank direction data
 *Caution: It's only can be use in the safe environment & param in!
 *         Time sensitive!!!
 */
void get_current_gpio_bank_dir(struct ow_gpio_des *gpio)
{
	unsigned char current_gpio_bank_dir;

	/* get current gpio bank in/out status */
	current_gpio_bank_dir = readb(gpio->gpio_dir_addr);
	/* get onewire gpio direction in/out data */
	gpio->gpio_dir_in_data = current_gpio_bank_dir & (~BIT(gpio->offset));
	gpio->gpio_dir_out_data = current_gpio_bank_dir | BIT(gpio->offset);
}

int get_gpio_phy_addr(struct platform_device *pdev,
		      struct ow_gpio_des *pl061_gpio)
{
	int ret;
	u32 gpio_reg_property[GPIO_REG_PROPERTY_SIZE];
	struct device_node *gpio_chip_np = NULL;

	if (!pdev || !pl061_gpio)
		return -1;
	/* get onewire gpio description from device tree */
	gpio_chip_np = of_parse_phandle(pdev->dev.of_node, "onewire-gpio",
					GPIO_CHIP_PHANDLE_INDEX);
	if (!gpio_chip_np) {
		hwlog_err("NULL: gpio_chip_np found in %s\n", __func__);
		return ONEWIRE_NULL_INPARA;
	}
	ret = of_property_read_u32_array(gpio_chip_np, "reg", gpio_reg_property,
					 GPIO_REG_PROPERTY_SIZE);
	if (ret) {
		hwlog_err("DTS:gpio reg property not found in %s\n", __func__);
		return ONEWIRE_DTS_FAIL;
	}
	pl061_gpio->gpio_phy_base_addr = gpio_reg_property[ADDRESS_HIGH32BIT];
	pl061_gpio->gpio_phy_base_addr <<= SHIFT_32;
	pl061_gpio->gpio_phy_base_addr += gpio_reg_property[ADDRESS_LOW32BIT];
	pl061_gpio->length = gpio_reg_property[LENGTH_HIGH32BIT];
	pl061_gpio->length <<= SHIFT_32;
	pl061_gpio->length += gpio_reg_property[LENGTH_LOW32BIT];
	ret = of_property_read_u32_index(pdev->dev.of_node, "onewire-gpio",
					 ONEWIRE_GPIO_OFFSET_INDEX,
					 &pl061_gpio->offset);
	if (ret) {
		hwlog_err("DTS:offset not found in %s\n", __func__);
		return ONEWIRE_DTS_FAIL;
	}

	return ONEWIRE_SUCCESS;
}

int init_gpio_iomem_addr(struct platform_device *pdev,
			 struct ow_gpio_des *pl061_gpio)
{
	void *gpio_virtual_base_addr = NULL;

	if (!pdev || !pl061_gpio)
		return -1;
	/* get virtual address from physic address */
	gpio_virtual_base_addr = devm_ioremap(&pdev->dev,
					      pl061_gpio->gpio_phy_base_addr,
					      pl061_gpio->length);
	if (!gpio_virtual_base_addr) {
		hwlog_err("NULL:virtual_base_addr found in %s\n", __func__);
		return ONEWIRE_NULL_INPARA;
	}
	pl061_gpio->gpio_data_addr = gpio_virtual_base_addr +
				    BIT(pl061_gpio->offset + PL061_DATA_OFFSET);
	pl061_gpio->gpio_dir_addr = gpio_virtual_base_addr + PL061_DIR_OFFSET;

	return ONEWIRE_SUCCESS;
}

int get_gpio_for_ow(struct platform_device *pdev,
		    struct ow_gpio_des *pl061_gpio, const char *gpio_name)
{
	int gpio_id;
	struct gpio_desc *desc = NULL;
	struct gpio_chip *chip = NULL;
#if (KERNEL_VERSION(4, 12, 0) > LINUX_VERSION_CODE)
	struct pl061_gpio *pl061_chip = NULL;
#else
	struct pl061 *pl061_chip = NULL;
#endif

	if (!pdev || !pl061_gpio)
		return -1;
	/* get gpio id */
	gpio_id = of_get_named_gpio(pdev->dev.of_node, "onewire-gpio",
				    GPIO_INDEX);
	if (gpio_id < 0) {
		hwlog_err("No gpio named onewire-gpio required by %s\n",
			  __func__);
		return ONEWIRE_GPIO_FAIL;
	}
	desc = gpio_to_desc(gpio_id);
	if (!desc) {
		hwlog_err("NULL: desc, found in %s\n", __func__);
		return ONEWIRE_NULL_INPARA;
	}
	chip = gpiod_to_chip(desc);
	if (!chip) {
		hwlog_err("NULL: chip, found in %s\n", __func__);
		return ONEWIRE_NULL_INPARA;
	}
#if (KERNEL_VERSION(4, 12, 0) > LINUX_VERSION_CODE)
	pl061_chip = container_of(chip, struct pl061_gpio, gc);
#else
	pl061_chip = container_of(chip, struct pl061, gc);
#endif
	if (!pl061_chip) {
		hwlog_err("NULL: pl061_chip, found in %s\n", __func__);
		return ONEWIRE_NULL_INPARA;
	}
#if (KERNEL_VERSION(4, 12, 0) > LINUX_VERSION_CODE)
	pl061_gpio->lock = &pl061_chip->lock.rlock;
#else
	pl061_gpio->lock = &pl061_chip->lock;
#endif
	if (!pl061_gpio->lock) {
		hwlog_err("NULL: pl061_gpio->lock, found in %s\n", __func__);
		return ONEWIRE_NULL_INPARA;
	}
	/* get the gpio */
	if (devm_gpio_request(&pdev->dev, gpio_id, gpio_name)) {
		hwlog_err("Gpio request failed(%d) in %s\n", gpio_id, __func__);
		return ONEWIRE_GPIO_FAIL;
	}
	gpio_direction_output(gpio_id, HIGH_VOLTAGE);

	return ONEWIRE_SUCCESS;
}
