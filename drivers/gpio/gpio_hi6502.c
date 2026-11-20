/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 * Description: provide pinctrl access function interfaces
 * Author: hisilicon
 * Create: 2019-11-28
 */
#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/hwspinlock.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <hisi_usb_phy_chip.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/hisi/hisi_hi6502_gpio.h>
#include "hisi_gpio.h"

#define GPIODIR 0x400
#define GPIOIS 0x404
#define GPIOIBE 0x408
#define GPIOIEV 0x40C
#define GPIOIE 0x410
#define GPIOMIS 0x418
#define GPIOIC 0x41C
#define HI6502_GPIO_NR 8
#define HI6502_GPIO_BANK 4
#define PL061_GPIO_BASE 296
#define HI6502_GPIO_MAX_NUM 328

struct hi6502_gpio {
	int sec_status;
	unsigned int base;
	unsigned int irqarray;
	struct gpio_chip gc;
	bool uses_pinctrl;
	struct delayed_work work;
	struct mutex irq_lock;
	struct mutex lock;
	u8 gpioie;
	u8 gpiois;
	u8 gpioiev;
	u8 gpioibe;
	u8 gpioic;
};

struct hi6502_gpio_collect {
	unsigned int sctrl_base;
	unsigned int sctrl_offset;
	unsigned int parent_irq;
	unsigned int virqs[32]; /* GPIO bank * GPIO NR */
	unsigned int base[HI6502_GPIO_BANK];
	struct irq_domain *domain[HI6502_GPIO_BANK];
};

static struct hi6502_gpio_collect hi6502_collect;

unsigned int phy_type;

static unsigned int hi6502_readb(struct hi6502_gpio *chip, unsigned int offset)
{
	unsigned int v = 0;

	hi6502_readl_u32(&v, chip->base + offset);

	return v;
}

static void hi6502_writeb(struct hi6502_gpio *chip, unsigned int v,
	unsigned int offset)
{
	hi6502_writel_u32(v, chip->base + offset);
}

static int hi6502_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct hi6502_gpio *hi6502 = NULL;

	if (!gc)
		return -EINVAL;

	hi6502 = gpiochip_get_data(gc);
	if (!hi6502)
		return -EINVAL;

	dev_dbg(gc->parent, "hi6502 get direct offset is %u gc base is 0x%x\n",
		offset, hi6502->base);

	return !(hi6502_readb(hi6502, GPIODIR) & BIT(offset));
}

static int hi6502_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct hi6502_gpio *hi6502 = NULL;
	unsigned char gpiodir;

	if (!gc || offset >= gc->ngpio)
		return -EINVAL;

	hi6502 = gpiochip_get_data(gc);
	if (!hi6502)
		return -EINVAL;

	dev_dbg(gc->parent, "offset is %u gc base is 0x%x\n", offset,
		hi6502->base);

	mutex_lock(&hi6502->lock);

	gpiodir = hi6502_readb(hi6502, GPIODIR);
	gpiodir &= ~(BIT(offset));
	hi6502_writeb(hi6502, gpiodir, GPIODIR);

	mutex_unlock(&hi6502->lock);

	return 0;
}

static int hi6502_direction_output(
	struct gpio_chip *gc, unsigned int offset, int value)
{
	struct hi6502_gpio *hi6502 = NULL;
	unsigned char gpiodir;

	if (!gc || offset >= gc->ngpio)
		return -EINVAL;

	hi6502 = gpiochip_get_data(gc);
	if (!hi6502)
		return -EINVAL;

	dev_dbg(gc->parent, "offset is %u gc base is 0x%x value is %d\n",
		offset, hi6502->base, value);

	mutex_lock(&hi6502->lock);

	hi6502_writeb(
		hi6502, !!(unsigned int)value << offset, (BIT(offset + 2))); /* 2: fixed offset */
	gpiodir = hi6502_readb(hi6502, GPIODIR);
	gpiodir |= BIT(offset);
	hi6502_writeb(hi6502, gpiodir, GPIODIR);

	/*
	 * gpio value is set again, because hi6502 doesn't allow to set value of
	 * a gpio pin before configuring it in OUT mode.
	 */
	hi6502_writeb(hi6502, !!value << offset, (BIT(offset + 2))); /* 2: fixed offset */

	mutex_unlock(&hi6502->lock);

	return 0;
}

static int hi6502_get_value(struct gpio_chip *gc, unsigned int offset)
{
	struct hi6502_gpio *hi6502 = NULL;

	if (!gc)
		return -EINVAL;

	hi6502 = gpiochip_get_data(gc);
	if (!hi6502)
		return -EINVAL;

	dev_dbg(gc->parent, "hi6502 get direct offset is %u gc base is 0x%x\n",
		offset, hi6502->base);

	return !!hi6502_readb(hi6502, (BIT(offset + 2))); /* 2: fixed offset */
}

static void hi6502_set_value(struct gpio_chip *gc, unsigned int offset,
							int value)
{
	struct hi6502_gpio *hi6502 = NULL;

	if (!gc || offset >= gc->ngpio)
		return;

	hi6502 = gpiochip_get_data(gc);
	if (!hi6502)
		return;

	dev_dbg(gc->parent,
		"hi6502 set value offset is %u gc base is 0x%x value is %d\n",
		offset, hi6502->base, value);

	hi6502_writeb(
		hi6502, !!(unsigned int)value << offset, (BIT(offset + 2))); /* 2: fixed offset */
}

static int hi6502_irq_type(struct irq_data *d, unsigned int trigger)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hi6502_gpio *hi6502 = gpiochip_get_data(gc);
	unsigned long offset = irqd_to_hwirq(d);
	if (offset >= HI6502_GPIO_NR)
		return -EINVAL;

	if (trigger & (IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW)) {
		hi6502->gpiois |= 1 << offset;
		if (trigger & IRQ_TYPE_LEVEL_HIGH)
			hi6502->gpioiev |= 1 << offset;
		else
			hi6502->gpioiev &= ~(1 << offset);
	} else {
		hi6502->gpiois &= ~(1 << offset);
	}

	if ((trigger & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH) {
		hi6502->gpioibe |= 1 << offset;
	} else {
		hi6502->gpioibe &= ~(1 << offset);
		if (trigger & IRQ_TYPE_EDGE_RISING)
			hi6502->gpioiev |= 1 << offset;
		else if (trigger & IRQ_TYPE_EDGE_FALLING)
			hi6502->gpioiev &= ~(1 << offset);
	}

	return 0;
}

static void hi6502_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hi6502_gpio *hi6502 = gpiochip_get_data(gc);
	unsigned long offset = irqd_to_hwirq(d);
	if (offset >= HI6502_GPIO_NR)
		return;

	hi6502->gpioie &= ~(1 << offset);
}

static void hi6502_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hi6502_gpio *hi6502 = gpiochip_get_data(gc);
	unsigned long offset = irqd_to_hwirq(d);
	if (offset >= HI6502_GPIO_NR)
		return;

	hi6502->gpioie |= 1 << offset;
}

static void hi6502_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hi6502_gpio *hi6502 = gpiochip_get_data(gc);
	unsigned long offset = irqd_to_hwirq(d);
	if (offset >= HI6502_GPIO_NR)
		return;

	hi6502->gpioic |= 1 << offset;
}

int hi6502_gpio_to_irq(unsigned int gpio)
{
	unsigned int offset, virq, group;

	if ((gpio >= PL061_GPIO_BASE) && (gpio < HI6502_GPIO_MAX_NUM)) {
		offset = (gpio - PL061_GPIO_BASE) % HI6502_GPIO_NR;
		group = (gpio - PL061_GPIO_BASE) / HI6502_GPIO_NR;

		virq = irq_create_mapping(hi6502_collect.domain[group], offset);
		if (!virq) {
			pr_err("%s: Failed mapping hwirq\n", __func__);
			return -1;
		}

		hi6502_collect.virqs[gpio - PL061_GPIO_BASE] = virq;

		return virq;
	}
	pr_err("%s: gpio num invald\n", __func__);
	return -1;
}

static void hi6502_delay_work(struct work_struct *work)
{
	unsigned long pending = 0;
	unsigned long sctrl_val = 0;
	int sctrl_offset = 0;
	int s;
	int p = 0;
	int loop = 200; /* max seleep loop num */

	while (hi6502_is_suspend() && (loop > 0)) {
		usleep_range(300, 500); /* seleep 300~500 us one time */
		loop--;
	}
	if (hi6502_is_suspend()) {
		pr_err("%s: hi6502_is_suspend timeout\n", __func__);
		return;
	}

	hi6502_readl_u32(&sctrl_val, hi6502_collect.sctrl_base +
		hi6502_collect.sctrl_offset);

	if (sctrl_val) {
		for_each_set_bit(sctrl_offset, &sctrl_val, HI6502_GPIO_BANK) {
			s = sctrl_offset;

			if ((s < 0) || (s >= HI6502_GPIO_BANK)) {
				pr_err("%s: sctrl is invalid\n", __func__);
				return;
			}

			hi6502_readl_u32(&pending, hi6502_collect.base[s] + GPIOMIS);
			if (pending) {
				for_each_set_bit(p, &pending, HI6502_GPIO_NR) {
					generic_handle_irq(
						hi6502_collect.virqs[s * HI6502_GPIO_NR + p]);
					hi6502_writel_u32(
						pending, hi6502_collect.base[s] + GPIOIC);
				}
			}
		}
	}

	enable_irq(hi6502_collect.parent_irq);
}

static irqreturn_t hi6502_irq_handle(int irq, void *data)
{
	struct hi6502_gpio *hi6502;

	hi6502 = (struct hi6502_gpio *)data;

	disable_irq_nosync(hi6502_collect.parent_irq);

	schedule_delayed_work(&hi6502->work, 0);

	return IRQ_HANDLED;
}

static int hi6502_gpio_irq_init(struct hi6502_gpio *hi6502, struct device *dev)
{
	int ret;
	int gpio;
	struct device_node *node =  dev->of_node;

	gpio = of_get_named_gpio(node, "hi6502-irq,gpios", 0);
	if (!gpio_is_valid(gpio)) {
		pr_err("%s: gpio is invalid\n", __func__);
		return -EINVAL;
	}

	ret = gpio_request(gpio, "hi6502");
	if (ret < 0) {
		pr_err("%s: Can not request hi6502_gpio %d\n", __func__, gpio);
		return -EIO;
	}

	ret = gpio_direction_input(gpio);
	if (ret < 0)
		goto out_free;

	ret = gpio_to_irq(gpio);
	if (ret < 0) {
		pr_err("%s: Could not set hi6502_gpio gpio = %d!, irq = %d\n",
			__func__, gpio, ret);
		goto out_free;
	}
	hi6502_collect.parent_irq = (unsigned int)ret;

	ret = request_irq(hi6502_collect.parent_irq,
			hi6502_irq_handle, IRQF_TRIGGER_LOW | IRQF_SHARED,
		"hi6502", hi6502);
	if (ret) {
		pr_err("%s: Request hi6502 parent_irq failed gpio = %d\n",
			__func__, gpio);
		goto out_free;
	}
	return ret;
out_free:
	gpio_free(gpio);
	return -EIO;
}

static void hi6502_gpio_irq_bus_lock(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hi6502_gpio *hi6502 = gpiochip_get_data(gc);

	mutex_lock(&hi6502->irq_lock);
}

static void hi6502_gpio_irq_bus_unlock(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct hi6502_gpio *hi6502 = gpiochip_get_data(gc);

	hi6502_writeb(hi6502, hi6502->gpiois, GPIOIS);
	hi6502_writeb(hi6502, hi6502->gpioibe, GPIOIBE);
	hi6502_writeb(hi6502, hi6502->gpioiev, GPIOIEV);
	hi6502_writeb(hi6502, hi6502->gpioie, GPIOIE);

	mutex_unlock(&hi6502->irq_lock);
}

static struct irq_chip hi6502_irqchip = {
	.name = "hi6502-gpio",
	.irq_ack = hi6502_irq_ack,
	.irq_mask = hi6502_irq_mask,
	.irq_unmask = hi6502_irq_unmask,
	.irq_disable = hi6502_irq_mask,
	.irq_enable = hi6502_irq_unmask,
	.irq_set_type = hi6502_irq_type,
	.irq_bus_lock = hi6502_gpio_irq_bus_lock,
	.irq_bus_sync_unlock = hi6502_gpio_irq_bus_unlock,
};

static int get_hi6502_device_tree_data(struct device_node *np,
	struct hi6502_gpio *hi6502)
{
	int ret, reg_prop_cnt;
	unsigned int *reg_prop = NULL;
	unsigned int sctrl_base, sctrl_offset;

	reg_prop_cnt = of_property_count_u32_elems(np, "reg");
	if (reg_prop_cnt < 0) {
		pr_err("%s: register reg not specified\n", __func__);
		return -ENOMEM;
	}
	reg_prop = kmalloc_array(reg_prop_cnt, sizeof(*reg_prop), GFP_KERNEL);
	if (!reg_prop)
		return -ENOMEM;

	ret = of_property_read_u32_array(np, "reg", reg_prop, reg_prop_cnt);
	if (ret) {
		pr_err("%s: register reg base not specified\n", __func__);
		kfree(reg_prop);
		return ret;
	}

	hi6502->base = reg_prop[1];

	/* Hook the request()/free() for pinctrl operation */
	if (of_get_property(np, "gpio-ranges", NULL)) {
		hi6502->uses_pinctrl = true;
		hi6502->gc.request = gpiochip_hi6502_request;
		hi6502->gc.free = gpiochip_hi6502_free;
	}
	kfree(reg_prop);

	if (!of_property_read_u32(np, "sctrl,base", &sctrl_base))
		hi6502_collect.sctrl_base = sctrl_base;

	if (!of_property_read_u32(np, "sctrl,offset", &sctrl_offset))
		hi6502_collect.sctrl_offset = sctrl_offset;

	/* get hi6502 irq array number */
	ret = of_property_read_u32_array(np, "hi6502-irq-array",
		&(hi6502->irqarray), 1);
	if (ret) {
		pr_err("%s: no hi6502-irq-array property set\n", __func__);
		return -ENODEV;
	}
	hi6502_collect.base[hi6502->irqarray] = hi6502->base;

	return ret;
}

static int hi6502_irq_map(struct irq_domain *d, unsigned int virq,
			  irq_hw_number_t hw)
{
	int ret;
	struct gpio_chip *chip = d->host_data;

	irq_set_chip_data(virq, chip);
	irq_set_chip_and_handler(virq, &hi6502_irqchip, handle_simple_irq);
	ret = irq_set_irq_type(virq, IRQ_TYPE_NONE);
	if (ret) {
		pr_err("%s: set irq type failed\n", __func__);
		return ret;
	}
	return 0;
}

static const struct irq_domain_ops hi6502_domain_ops = {
	.map	= hi6502_irq_map,
	.xlate	= irq_domain_xlate_twocell,
};

static void hi6502_setup_gpio(struct hi6502_gpio *hi6502, struct device *dev)
{
	/* clear sec-flag of the controller */
	hi6502->sec_status = 0;

	hi6502->gc.base = -1; /* base distribute by gpiolib core */
	hi6502->gc.get_direction = hi6502_get_direction;
	hi6502->gc.direction_input = hi6502_direction_input;
	hi6502->gc.direction_output = hi6502_direction_output;
	hi6502->gc.get = hi6502_get_value;
	hi6502->gc.set = hi6502_set_value;
	hi6502->gc.ngpio = HI6502_GPIO_NR;
	hi6502->gc.label = dev_name(dev);
	hi6502->gc.parent = dev;
	hi6502->gc.owner = THIS_MODULE;
}

static int hi6502_setup_irq(struct hi6502_gpio *hi6502, struct device *dev)
{
	int ret;
	struct device_node *np = NULL;
	struct irq_domain *domain = NULL;

	np = dev->of_node;
	if (!np)
		return -EINVAL;

	domain = irq_domain_add_simple(np, HI6502_GPIO_NR, 0,
		&hi6502_domain_ops, &hi6502->gc);
	if (!domain) {
		dev_err(dev, "%s: failed irq domain add simple!\n", __func__);
		return -ENODEV;
	}

	if (hi6502->irqarray >= HI6502_GPIO_BANK)
		return -EINVAL;

	hi6502_collect.domain[hi6502->irqarray] = domain;
	if (hi6502->irqarray == 0) {
		ret = hi6502_gpio_irq_init(hi6502, dev);
		if (ret) {
			dev_err(dev, "%s: hi6502_gpio_irq_init failed, fail err_no is %d!\n",
				__func__, ret);
			return -EINVAL;
		}

		INIT_DELAYED_WORK(&hi6502->work, hi6502_delay_work);
	}
	mutex_init(&hi6502->irq_lock);
	return 0;
}

static int hi6502_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct hi6502_gpio *hi6502 = NULL;
	struct device_node *np = NULL;

	np = pdev->dev.of_node;
	if (!np)
		return -EINVAL;

	if (!hi6502_is_ready()) {
		dev_err(dev, "%s: usb phy chip is not ready\n", __func__);
		return -EPROBE_DEFER;
	}

	hi6502 = devm_kzalloc(dev, sizeof(*hi6502), GFP_KERNEL);
	if (!hi6502)
		return -ENOMEM;

	ret = get_hi6502_device_tree_data(np, hi6502);
	if (ret) {
		dev_err(dev, "%s: Error get hi6502 dts\n", __func__);
		return ret;
	}

	hi6502_setup_gpio(hi6502, dev);

	ret = gpiochip_add_data(&hi6502->gc, hi6502);
	if (ret) {
		dev_err(dev, "%s: gpio 6502 add data failed, fail err_no is %d!\n",
			__func__, ret);
		return ret;
	}

	dev_err(dev, "%s: gpio 6502 gpio chip base is %u!\n",
		__func__, hi6502->gc.base);
	phy_type = hi6502_chip_type();

	/* irq */
	hi6502_writeb(hi6502, 0xff, GPIOIC);
	hi6502_writeb(hi6502, 0, GPIOIE); /* disable irqs */

	ret = hi6502_setup_irq(hi6502, dev);
	if (ret)
		dev_err(dev, "%s: hi6502 setup irq failed, err_no is %d!\n",
			__func__, ret);

	mutex_init(&hi6502->lock);
	platform_set_drvdata(pdev, hi6502);
	dev_err(dev, "%s: HI6502 GPIO chip @0x%x registered\n",
		__func__, hi6502->base);

	return 0;
}

#ifdef CONFIG_HI6502_GPIO_PM_SUPPORT
static int hi6502_suspend(struct device *dev)
{
	return 0;
}

static int hi6502_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops hi6502_dev_pm_ops = {
	.suspend = hi6502_suspend,
	.resume = hi6502_resume,
	.freeze = hi6502_suspend,
	.restore = hi6502_resume,
};
#endif

static const struct of_device_id hi6502_gpio_of_match[] = {
	{
		.compatible = "hisilicon,hi6502-gpio",
		.data = NULL
	},
	{},
};
MODULE_DEVICE_TABLE(of, hi6502_gpio_of_match);

static struct platform_driver hi6502_gpio_driver = {
	.probe = hi6502_probe,
	.driver = {
		.name = "hi6502-gpio",
		.owner = THIS_MODULE,
#ifdef CONFIG_HI6502_GPIO_PM_SUPPORT
		.pm = hi6502_dev_pm_ops,
#endif
		.of_match_table = hi6502_gpio_of_match,
	},
};

static int __init hi6502_gpio_init(void)
{
	return platform_driver_register(&hi6502_gpio_driver);
}

static void __exit hi6502_gpio_exit(void)
{
	platform_driver_unregister(&hi6502_gpio_driver);
}

fs_initcall(hi6502_gpio_init);
module_exit(hi6502_gpio_exit);

MODULE_DESCRIPTION("hi6502 GPIO driver");
MODULE_LICENSE("GPL");
