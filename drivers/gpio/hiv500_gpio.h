/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 * Description: provide gpio access function interfaces
 * Author: hisilicon
 * Create: 2020-6-12
 */
#include <linux/hwspinlock.h>
#include <linux/gpio.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/of_address.h>
#include <linux/version.h>

#define HW_SW_SEL 4
#define HW_SW_SEL_MASK 20
#define GPIO_PER_REG 16
#define HIGH_LEVEL_ENABLE 0x00080008
#define LOW_LEVEL_ENABLE  0x00080000
#define RISING_EDGE_ENABLE  0x00060002
#define FALLING_EDGE_ENABLE  0x00060000
#define BOTH_EDGE_ENABLE  0x00060006
#define LEVEL_TRIGGER_ENABLE 0x00010001
#define EDGE_TRIGGER_ENABLE 0x00010000

void get_gpio_dump(void);

#ifdef CONFIG_PM
struct hiv500_context_save_regs {
	u8 gpio_data;
	u8 gpio_dir;
	u8 gpio_is;
	u8 gpio_ibe;
	u8 gpio_iev;
	u8 gpio_ie;
};
#endif

struct hiv500 {
	raw_spinlock_t lock;
	int sec_status;
	void __iomem *base;
	struct gpio_chip gc;
	int parent_irq;
	bool uses_pinctrl;
#ifdef CONFIG_PM
	struct hiv500_context_save_regs csave_regs;
#endif
	struct amba_device *adev;
};

int hiv500_check_security_status(struct hiv500 *chip);
int hiv500_parse_gpio_base(struct device *dev);
