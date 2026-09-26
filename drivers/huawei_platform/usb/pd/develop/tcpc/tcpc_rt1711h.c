/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * Mediatek MT6370 Type-C Port Control Driver
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "rt1711h.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/pm_runtime.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/cpu.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>
#include <linux/sched/clock.h>
#include <linux/sched/rt.h>
#include <uapi/linux/sched/types.h>

#ifdef CONFIG_RT_REGMAP
#include <mt-plat/rt-regmap.h>
#endif /* CONFIG_RT_REGMAP */

#include "inc/tcpci.h"
#include "inc/tcpm.h"

#define RT1711H_DRV_VERSION	"2.0.1_MTK"

#define RT1711H_IRQ_WAKE_TIME 500 /* ms */
#define RT1711H_RETRY_TIMES   5
#define RICHTEK_1711_VID      0x29cf
#define RICHTEK_1711_PID      0x1711

struct rt1711_chip {
	struct i2c_client *client;
	struct device *dev;
#ifdef CONFIG_RT_REGMAP
	struct rt_regmap_device *m_dev;
#endif /* CONFIG_RT_REGMAP */
	struct semaphore io_lock;
	struct semaphore suspend_lock;
	struct tcpc_device *tcpc;
	struct kthread_worker irq_worker;
	struct kthread_work irq_work;
	struct task_struct *irq_worker_task;
	struct wakeup_source irq_wake_lock;

	atomic_t poll_count;
	struct delayed_work	poll_work;

	int irq_gpio;
	int irq;
	int chip_id;
	bool typec_polarity;
};

#ifdef CONFIG_RT_REGMAP
RT_REG_DECL(TCPC_V10_REG_VID, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_PID, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_DID, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TYPEC_REV, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_PD_REV, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_PDIF_REV, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_ALERT, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_ALERT_MASK, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_POWER_STATUS_MASK, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_FAULT_STATUS_MASK, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TCPC_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_ROLE_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_FAULT_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_POWER_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_CC_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_POWER_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_FAULT_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_COMMAND, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_MSG_HDR_INFO, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_DETECT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_BYTE_CNT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_BUF_FRAME_TYPE, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_HDR, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_RX_DATA, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TRANSMIT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TX_BYTE_CNT, 1, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TX_HDR, 2, RT_VOLATILE, {});
RT_REG_DECL(TCPC_V10_REG_TX_DATA, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_PHY_CTRL1, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_CLK_CTRL2, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_CLK_CTRL3, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_BMC_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_BMCIO_RXDZSEL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_VCONN_CLIMITEN, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_RT_STATUS, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_RT_INT, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_RT_MASK, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_IDLE_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_INTRST_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_WATCHDOG_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_I2CRST_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_SWRESET, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_TTCPC_FILTER, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_DRP_TOGGLE_CYCLE, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_DRP_DUTY_CTRL, 1, RT_VOLATILE, {});
RT_REG_DECL(RT1711H_REG_BMCIO_RXDZEN, 1, RT_VOLATILE, {});

static const rt_register_map_t rt1711_chip_regmap[] = {
	RT_REG(TCPC_V10_REG_VID),
	RT_REG(TCPC_V10_REG_PID),
	RT_REG(TCPC_V10_REG_DID),
	RT_REG(TCPC_V10_REG_TYPEC_REV),
	RT_REG(TCPC_V10_REG_PD_REV),
	RT_REG(TCPC_V10_REG_PDIF_REV),
	RT_REG(TCPC_V10_REG_ALERT),
	RT_REG(TCPC_V10_REG_ALERT_MASK),
	RT_REG(TCPC_V10_REG_POWER_STATUS_MASK),
	RT_REG(TCPC_V10_REG_FAULT_STATUS_MASK),
	RT_REG(TCPC_V10_REG_TCPC_CTRL),
	RT_REG(TCPC_V10_REG_ROLE_CTRL),
	RT_REG(TCPC_V10_REG_FAULT_CTRL),
	RT_REG(TCPC_V10_REG_POWER_CTRL),
	RT_REG(TCPC_V10_REG_CC_STATUS),
	RT_REG(TCPC_V10_REG_POWER_STATUS),
	RT_REG(TCPC_V10_REG_FAULT_STATUS),
	RT_REG(TCPC_V10_REG_COMMAND),
	RT_REG(TCPC_V10_REG_MSG_HDR_INFO),
	RT_REG(TCPC_V10_REG_RX_DETECT),
	RT_REG(TCPC_V10_REG_RX_BYTE_CNT),
	RT_REG(TCPC_V10_REG_RX_BUF_FRAME_TYPE),
	RT_REG(TCPC_V10_REG_RX_HDR),
	RT_REG(TCPC_V10_REG_RX_DATA),
	RT_REG(TCPC_V10_REG_TRANSMIT),
	RT_REG(TCPC_V10_REG_TX_BYTE_CNT),
	RT_REG(TCPC_V10_REG_TX_HDR),
	RT_REG(TCPC_V10_REG_TX_DATA),
	RT_REG(RT1711H_REG_PHY_CTRL1),
	RT_REG(RT1711H_REG_CLK_CTRL2),
	RT_REG(RT1711H_REG_CLK_CTRL3),
	RT_REG(RT1711H_REG_BMC_CTRL),
	RT_REG(RT1711H_REG_BMCIO_RXDZSEL),
	RT_REG(RT1711H_REG_VCONN_CLIMITEN),
	RT_REG(RT1711H_REG_RT_STATUS),
	RT_REG(RT1711H_REG_RT_INT),
	RT_REG(RT1711H_REG_RT_MASK),
	RT_REG(RT1711H_REG_IDLE_CTRL),
	RT_REG(RT1711H_REG_INTRST_CTRL),
	RT_REG(RT1711H_REG_WATCHDOG_CTRL),
	RT_REG(RT1711H_REG_I2CRST_CTRL),
	RT_REG(RT1711H_REG_SWRESET),
	RT_REG(RT1711H_REG_TTCPC_FILTER),
	RT_REG(RT1711H_REG_DRP_TOGGLE_CYCLE),
	RT_REG(RT1711H_REG_DRP_DUTY_CTRL),
	RT_REG(RT1711H_REG_BMCIO_RXDZEN),
};
#define RT1711_CHIP_REGMAP_SIZE ARRAY_SIZE(rt1711_chip_regmap)
#endif /* CONFIG_RT_REGMAP */

static int rt1711_read_device(void *client, u32 reg, int len, void *dst)
{
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret;
	int count = RT1711H_RETRY_TIMES;

	while (count) {
		if (len > 1) {
			ret = i2c_smbus_read_i2c_block_data(i2c, reg, len, dst);
			if (ret < 0)
				count--;
			else
				return ret;
		} else {
			ret = i2c_smbus_read_byte_data(i2c, reg);
			if (ret < 0) {
				count--;
			} else {
				*(u8 *)dst = (u8)ret;
				return ret;
			}
		}
		/* delay 100us */
		udelay(100);
	}
	return ret;
}

static int rt1711_write_device(void *client, u32 reg, int len, const void *src)
{
	const u8 *data = NULL;
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret;
	int count = RT1711H_RETRY_TIMES;

	while (count) {
		if (len > 1) {
			ret = i2c_smbus_write_i2c_block_data(i2c, reg, len, src);
			if (ret < 0)
				count--;
			else
				return ret;
		} else {
			data = src;
			ret = i2c_smbus_write_byte_data(i2c, reg, *data);
			if (ret < 0)
				count--;
			else
				return ret;
		}
		/* delay 100us */
		udelay(100);
	}
	return ret;
}

static int rt1711_reg_read(struct i2c_client *i2c, u8 reg)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	u8 val = 0;
	int ret;

#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_read(chip->m_dev, reg, 1, &val);
#else
	ret = rt1711_read_device(chip->client, reg, 1, &val);
#endif /* CONFIG_RT_REGMAP */
	if (ret < 0) {
		dev_err(chip->dev, "rt1711 reg read fail\n");
		return ret;
	}
	return val;
}

static int rt1711_reg_write(struct i2c_client *i2c, u8 reg, const u8 data)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	int ret;

#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_write(chip->m_dev, reg, 1, &data);
#else
	ret = rt1711_write_device(chip->client, reg, 1, &data);
#endif /* CONFIG_RT_REGMAP */
	if (ret < 0)
		dev_err(chip->dev, "rt1711 reg write fail\n");
	return ret;
}

static int rt1711_block_read(struct i2c_client *i2c, u8 reg, int len, void *dst)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	int ret;

#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_read(chip->m_dev, reg, len, dst);
#else
	ret = rt1711_read_device(chip->client, reg, len, dst);
#endif /* #ifdef CONFIG_RT_REGMAP */
	if (ret < 0)
		dev_err(chip->dev, "rt1711 block read fail\n");
	return ret;
}

static int rt1711_block_write(struct i2c_client *i2c, u8 reg, int len,
	const void *src)
{
	struct rt1711_chip *chip = i2c_get_clientdata(i2c);
	int ret;

#ifdef CONFIG_RT_REGMAP
	ret = rt_regmap_block_write(chip->m_dev, reg, len, src);
#else
	ret = rt1711_write_device(chip->client, reg, len, src);
#endif /* #ifdef CONFIG_RT_REGMAP */
	if (ret < 0)
		dev_err(chip->dev, "rt1711 block write fail\n");
	return ret;
}

static inline int32_t rt1711_write_word(struct i2c_client *client,
	uint8_t reg_addr, uint16_t data)
{
	return rt1711_block_write(client, reg_addr, sizeof(uint16_t),
		(uint8_t *)&data);
}

static inline int32_t rt1711_read_word(struct i2c_client *client,
	uint8_t reg_addr, uint16_t *data)
{
	return rt1711_block_read(client, reg_addr, sizeof(uint16_t),
		(uint8_t *)data);
}

static inline int rt1711_i2c_write8(struct rt1711_chip *chip, u8 reg,
	const u8 data)
{
	return rt1711_reg_write(chip->client, reg, data);
}

static inline int rt1711_i2c_write16(struct rt1711_chip *chip, u8 reg,
	const u16 data)
{
	return rt1711_write_word(chip->client, reg, data);
}

static inline int rt1711_i2c_read8(struct rt1711_chip *chip, u8 reg)
{
	return rt1711_reg_read(chip->client, reg);
}

static int rt1711_i2c_read16(struct rt1711_chip *chip, u8 reg)
{
	u16 data;
	int ret;

	ret = rt1711_read_word(chip->client, reg, &data);
	if (ret < 0)
		return ret;

	return data;
}

#ifdef CONFIG_RT_REGMAP
static struct rt_regmap_fops rt1711_regmap_fops = {
	.read_device = rt1711_read_device,
	.write_device = rt1711_write_device,
};

static int rt1711_regmap_init(struct rt1711_chip *chip)
{
	struct rt_regmap_properties *props = NULL;
	/* 32: max name size */
	char name[32];
	int len;

	props = devm_kzalloc(chip->dev, sizeof(*props), GFP_KERNEL);
	if (!props)
		return -ENOMEM;

	props->register_num = RT1711_CHIP_REGMAP_SIZE;
	props->rm = rt1711_chip_regmap;
	props->rt_regmap_mode = RT_MULTI_BYTE | RT_CACHE_DISABLE |
		RT_IO_PASS_THROUGH | RT_DBG_GENERAL;
	snprintf(name, sizeof(name), "rt1711-%02x", chip->client->addr);

	len = strlen(name);
	props->name = kzalloc(len + 1, GFP_KERNEL);
	if (!props->name)
		goto fail_alloc_name;
	props->aliases = kzalloc(len + 1, GFP_KERNEL);
	if (!props->aliases)
		goto fail_alloc_aliases;

	strlcpy((char *)props->name, name, len + 1);
	strlcpy((char *)props->aliases, name, len + 1);
	props->io_log_en = 0;

	chip->m_dev = rt_regmap_device_register(props, &rt1711_regmap_fops,
		chip->dev, chip->client, chip);
	if (!chip->m_dev) {
		dev_err(chip->dev, "rt1711 chip rt_regmap register fail\n");
		goto fail_register_regmap;
	}
	return 0;

fail_register_regmap:
	kfree(props->aliases);
fail_alloc_aliases:
	kfree(props->name);
fail_alloc_name:
	kfree(props);
	return -ENOMEM;
}

static int rt1711_regmap_deinit(struct rt1711_chip *chip)
{
	rt_regmap_device_unregister(chip->m_dev);
	return 0;
}
#else
static inline int rt1711_regmap_init(struct rt1711_chip *chip)
{
	return 0;
}

static inline int rt1711_regmap_deinit(struct rt1711_chip *chip)
{
	return 0;
}
#endif /* CONFIG_RT_REGMAP */

static int rt1711_software_reset(struct rt1711_chip *chip)
{
	int ret = rt1711_i2c_write8(chip, RT1711H_REG_SWRESET, 1);

	if (ret < 0)
		return ret;
	/* 1ms */
	usleep_range(1000, 2000);
	return 0;
}

static inline int rt1711_command(struct rt1711_chip *chip, uint8_t cmd)
{
	return rt1711_i2c_write8(chip, TCPC_V10_REG_COMMAND, cmd);
}

static int rt1711_init_alert_mask(struct rt1711_chip *chip)
{
	uint16_t mask;

	mask = TCPC_V10_REG_ALERT_CC_STATUS;
	mask |= TCPC_V10_REG_ALERT_POWER_STATUS;

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	/* Need to handle RX overflow */
	mask |= TCPC_V10_REG_ALERT_TX_SUCCESS;
	mask |= TCPC_V10_REG_ALERT_TX_DISCARDED;
	mask |= TCPC_V10_REG_ALERT_TX_FAILED;
	mask |= TCPC_V10_REG_ALERT_RX_HARD_RST;
	mask |= TCPC_V10_REG_ALERT_RX_STATUS;
	mask |= TCPC_V10_REG_RX_OVERFLOW;
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	mask |= TCPC_REG_ALERT_FAULT;
	return rt1711_write_word(chip->client, TCPC_V10_REG_ALERT_MASK, mask);
}

static int rt1711_init_power_status_mask(struct rt1711_chip *chip)
{
	return rt1711_i2c_write8(chip, TCPC_V10_REG_POWER_STATUS_MASK,
		TCPC_V10_REG_POWER_STATUS_VBUS_PRES);
}

static int rt1711_init_fault_mask(struct rt1711_chip *chip)
{
	const uint8_t mask = TCPC_V10_REG_FAULT_STATUS_VCONN_OV |
		TCPC_V10_REG_FAULT_STATUS_VCONN_OC;

	return rt1711_i2c_write8(chip, TCPC_V10_REG_FAULT_STATUS_MASK, mask);
}

static int rt1711_init_rt_mask(struct rt1711_chip *chip)
{
	uint8_t rt_mask = 0;

#ifdef CONFIG_TYPEC_CAP_RA_DETACH
	if (tcpci_is_support(chip->tcpc, TCPC_FLAGS_CHECK_RA_DETACHE))
		rt_mask |= RT1711H_REG_M_RA_DETACH;
#endif /* CONFIG_TYPEC_CAP_RA_DETACH */

	if (tcpci_is_support(chip->tcpc, TCPC_FLAGS_LPM_WAKEUP_WATCHDOG))
		rt_mask |= RT1711H_REG_M_WAKEUP;

	return rt1711_i2c_write8(chip, RT1711H_REG_RT_MASK, rt_mask);
}

static inline void rt1711_poll_ctrl(struct rt1711_chip *chip)
{
	cancel_delayed_work_sync(&chip->poll_work);

	if (atomic_read(&chip->poll_count) == 0) {
		atomic_inc(&chip->poll_count);
		cpu_idle_poll_ctrl(true);
	}

	/* 40ms */
	schedule_delayed_work(&chip->poll_work, msecs_to_jiffies(40));
}

static void rt1711_irq_work_handler(struct kthread_work *work)
{
	struct rt1711_chip *chip = container_of(work, struct rt1711_chip,
		irq_work);
	int regval;
	int gpio_val;

	rt1711_poll_ctrl(chip);

	/* make sure I2C bus had resumed */
	down(&chip->suspend_lock);
	do {
		regval = tcpci_alert(chip->tcpc);
		if (regval)
			break;
		gpio_val = gpio_get_value(chip->irq_gpio);
	} while (gpio_val == 0);

	up(&chip->suspend_lock);
}

static void rt1711_poll_work(struct work_struct *work)
{
	struct rt1711_chip *chip = container_of(
		work, struct rt1711_chip, poll_work.work);

	if (atomic_dec_and_test(&chip->poll_count))
		cpu_idle_poll_ctrl(false);
}

static irqreturn_t rt1711_intr_handler(int irq, void *data)
{
	struct rt1711_chip *chip = data;

	__pm_wakeup_event(&chip->irq_wake_lock, RT1711H_IRQ_WAKE_TIME);

	kthread_queue_work(&chip->irq_worker, &chip->irq_work);
	return IRQ_HANDLED;
}

static int rt1711_init_alert(struct rt1711_chip *chip)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
	int ret;

	/* Clear Alert Mask & Status */
	rt1711_write_word(chip->client, TCPC_V10_REG_ALERT_MASK, 0);
	rt1711_write_word(chip->client, TCPC_V10_REG_ALERT, 0xffff);

	ret = devm_gpio_request(chip->dev, chip->irq_gpio, "tcpc_rt1711h-IRQ");
	if (ret < 0) {
		pr_err("failed to request GPIO%d ret=%d\n", chip->irq_gpio, ret);
		goto init_alert_err;
	}

	ret = gpio_direction_input(chip->irq_gpio);
	if (ret < 0) {
		pr_err("failed to set GPIO%d as input pin ret=%d)\n",
			chip->irq_gpio, ret);
		goto init_alert_err;
	}

	chip->irq = gpio_to_irq(chip->irq_gpio);
	if (chip->irq <= 0) {
		pr_err("%s gpio to irq fail, chip->irq%d\n", __func__, chip->irq);
		goto init_alert_err;
	}
	pr_info("%s : IRQ number=%d\n", __func__, chip->irq);

	kthread_init_worker(&chip->irq_worker);
	chip->irq_worker_task = kthread_run(kthread_worker_fn, &chip->irq_worker,
		"tcpc_rt1711h");
	if (IS_ERR(chip->irq_worker_task)) {
		pr_err("could not create tcpc task\n");
		goto init_alert_err;
	}

	sched_setscheduler(chip->irq_worker_task, SCHED_FIFO, &param);
	kthread_init_work(&chip->irq_work, rt1711_irq_work_handler);

	ret = request_irq(chip->irq, rt1711_intr_handler,
		IRQF_TRIGGER_FALLING | IRQF_NO_THREAD |
		IRQF_NO_SUSPEND, "tcpc_rt1711h-IRQ", chip);
	if (ret < 0) {
		pr_err("failed to request irq%d gpio=%d, ret=%d\n", chip->irq,
			chip->irq_gpio, ret);
		goto init_alert_err;
	}

	enable_irq_wake(chip->irq);
	return 0;

init_alert_err:
	return -EINVAL;

}

int rt1711_alert_status_clear(void *chip, uint32_t mask)
{
	uint16_t mask_t1;

	if (!chip)
		return -ENODEV;

	/* Write 1 clear */
	mask_t1 = (uint16_t) mask;
	if (mask_t1)
		return rt1711_i2c_write16(chip, TCPC_V10_REG_ALERT, mask_t1);
}

static int rt1711h_set_clock_gating(struct rt1711_chip *chip, bool en)
{
	int ret = 0;
	uint8_t clk2 = RT1711H_REG_CLK_DIV_600K_EN
		| RT1711H_REG_CLK_DIV_300K_EN | RT1711H_REG_CLK_CK_300K_EN;

	uint8_t clk3 = RT1711H_REG_CLK_DIV_2P4M_EN;

	if (!en) {
		clk2 |= RT1711H_REG_CLK_BCLK2_EN | RT1711H_REG_CLK_BCLK_EN;
		clk3 |= RT1711H_REG_CLK_CK_24M_EN | RT1711H_REG_CLK_PCLK_EN;
	}

	if (en)
		ret = rt1711_alert_status_clear(chip, TCPC_REG_ALERT_RX_STATUS |
			TCPC_REG_ALERT_RX_HARD_RST | TCPC_REG_ALERT_RX_BUF_OVF);

	if (ret == 0)
		ret = rt1711_i2c_write8(chip, RT1711H_REG_CLK_CTRL2, clk2);
	if (ret == 0)
		ret = rt1711_i2c_write8(chip, RT1711H_REG_CLK_CTRL3, clk3);

	return ret;
}

static int rt1711h_init_cc_params(struct rt1711_chip *chip, uint8_t cc_res)
{
#ifdef CONFIG_HW_USB_POWER_DELIVERY
#ifdef CONFIG_USB_PD_SNK_DFT_NO_GOOD_CRC
	int rv;
	uint8_t en, sel;

	if (cc_res == TYPEC_CC_VOLT_SNK_DFT) { /* 0.55 */
		en = 0;
		sel = 0x81;
	} else if (chip->chip_id >= RT1715_DID_D) { /* 0.35 & 0.75 */
		en = 1;
		sel = 0x81;
	} else { /* 0.4 & 0.7 */
		en = 1;
		sel = 0x80;
	}

	rv = rt1711_i2c_write8(chip, RT1711H_REG_BMCIO_RXDZEN, en);
	if (rv == 0)
		rv = rt1711_i2c_write8(chip, RT1711H_REG_BMCIO_RXDZSEL, sel);
	return rv;
#endif /* CONFIG_USB_PD_SNK_DFT_NO_GOOD_CRC */
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

	return 0;
}

static int rt1711_tcpc_init(void *chip, bool sw_reset)
{
	int ret;
	bool retry_discard_old = false;
	struct rt1711_chip *rt_chip = chip;

	if (!rt_chip)
		return -ENODEV;

	RT1711_INFO("\n");

	if (sw_reset) {
		ret = rt1711_software_reset(rt_chip);
		if (ret < 0)
			return ret;
	}

	/* CK_300K from 320K, SHIPPING off, AUTOIDLE enable, TIMEOUT=32ms */
	rt1711_i2c_write8(rt_chip, RT1711H_REG_IDLE_CTRL,
		RT1711H_REG_IDLE_SET(0, 1, 1, 2));

	rt1711_i2c_write8(rt_chip, RT1711H_REG_I2CRST_CTRL,
		RT1711H_REG_I2CRST_SET(true, 0x0f));

	/* DRP = 0, RpVal = 0 (Default), Rd, Rd */
	rt1711_i2c_write8(rt_chip, TCPC_V10_REG_ROLE_CTRL,
		TCPC_V10_REG_ROLE_CTRL_RES_SET(0, 0, CC_RD, CC_RD));

	if (rt_chip->chip_id == RT1711H_DID_A)
		rt1711_i2c_write8(rt_chip, TCPC_V10_REG_FAULT_CTRL,
			TCPC_V10_REG_FAULT_CTRL_DIS_VCONN_OV);

	/*
	 * CC Detect Debounce : 26.7*val us
	 * Transition window count : spec 12~20us, based on 2.4MHz
	 * DRP Toggle Cycle : 51.2 + 6.4*val ms
	 * DRP Duyt Ctrl : dcSRC: /1024
	 */
	rt1711_i2c_write8(rt_chip, RT1711H_REG_TTCPC_FILTER, 10);
	rt1711_i2c_write8(rt_chip, RT1711H_REG_DRP_TOGGLE_CYCLE, 4);
	rt1711_i2c_write16(rt_chip, RT1711H_REG_DRP_DUTY_CTRL,
		TCPC_NORMAL_RP_DUTY);

	/* Vconn OC */
	rt1711_i2c_write8(rt_chip, RT1711H_REG_VCONN_CLIMITEN, 1);

	/* RX/TX Clock Gating (Auto Mode)*/
	if (!sw_reset)
		rt1711h_set_clock_gating(rt_chip, true);
	/* toggle_cnt=7,bus_idle_cnt=0,rx_filte=1 */
	if (!tcpci_is_support(rt_chip->tcpc, TCPC_FLAGS_RETRY_CRC_DISCARD))
		retry_discard_old = true;

	rt1711_i2c_write8(rt_chip, RT1711H_REG_PHY_CTRL1,
		RT1711H_REG_PHY_CTRL1_SET(retry_discard_old, 7, 0, 1));

	rt1711_alert_status_clear(rt_chip, 0xffffffff);
	rt1711_init_power_status_mask(rt_chip);
	rt1711_init_alert_mask(rt_chip);
	rt1711_init_fault_mask(rt_chip);
	rt1711_init_rt_mask(rt_chip);

	return 0;
}

static int rt1711_fault_status_vconn_ov(struct rt1711_chip *chip)
{
	int ret;

	ret = rt1711_i2c_read8(chip, RT1711H_REG_BMC_CTRL);
	if (ret < 0)
		return ret;

	ret &= ~RT1711H_REG_DISCHARGE_EN;
	return rt1711_i2c_write8(chip, RT1711H_REG_BMC_CTRL, ret);
}

int rt1711_fault_status_clear(void *chip, uint8_t status)
{
	if (!chip)
		return -ENODEV;

	if (status & TCPC_V10_REG_FAULT_STATUS_VCONN_OV)
		rt1711_fault_status_vconn_ov(chip);

	rt1711_i2c_write8(chip, TCPC_V10_REG_FAULT_STATUS, status);
	return 0;
}

int rt1711_get_alert_status(void *chip, uint32_t *alert)
{
	int ret;

	if (!chip)
		return -ENODEV;

	ret = rt1711_i2c_read16(chip, TCPC_V10_REG_ALERT);
	if (ret < 0)
		return ret;

	*alert = (uint16_t) ret;
	return 0;
}

static bool rt1711_is_vsafe0v(void *chip)
{
	int ret;

	if (!chip)
		return -ENODEV;

	ret = rt1711_i2c_read8(chip, RT1711H_REG_RT_STATUS);
	if (ret < 0)
		return false;

	return (ret & RT1711H_REG_VBUS_80) ? true : false;
}

static int rt1711_get_power_status(void *chip, uint16_t *pwr_status)
{
	int ret;

	if (!chip)
		return -ENODEV;

	ret = rt1711_i2c_read8(chip, TCPC_V10_REG_POWER_STATUS);
	if (ret < 0)
		return ret;

	*pwr_status = 0;
	if (ret & TCPC_V10_REG_POWER_STATUS_VBUS_PRES)
		*pwr_status |= TCPC_REG_POWER_STATUS_VBUS_PRES;

	return 0;
}

int rt1711_get_fault_status(void *chip, uint8_t *status)
{
	int ret;

	if (!chip)
		return -ENODEV;

	ret = rt1711_i2c_read8(chip, TCPC_V10_REG_FAULT_STATUS);
	if (ret < 0)
		return ret;

	*status = (uint8_t) ret;
	return 0;
}

static int rt1711_get_cc(void *chip, int *cc1, int *cc2)
{
	int status, role_ctrl, cc_role;
	bool act_as_sink = false;
	bool act_as_drp = false;
	struct rt1711_chip *rt_chip = chip;

	if (!rt_chip)
		return -ENODEV;

	status = rt1711_i2c_read8(rt_chip, TCPC_V10_REG_CC_STATUS);
	if (status < 0)
		return status;

	role_ctrl = rt1711_i2c_read8(rt_chip, TCPC_V10_REG_ROLE_CTRL);
	if (role_ctrl < 0)
		return role_ctrl;

	if (status & TCPC_V10_REG_CC_STATUS_DRP_TOGGLING) {
		*cc1 = TYPEC_CC_DRP_TOGGLING;
		*cc2 = TYPEC_CC_DRP_TOGGLING;
		return 0;
	}
	*cc1 = TCPC_V10_REG_CC_STATUS_CC1(status);
	*cc2 = TCPC_V10_REG_CC_STATUS_CC2(status);

	act_as_drp = TCPC_V10_REG_ROLE_CTRL_DRP & role_ctrl;
	if (act_as_drp) {
		act_as_sink = TCPC_V10_REG_CC_STATUS_DRP_RESULT(status);
	} else {
		cc_role =  TCPC_V10_REG_CC_STATUS_CC1(role_ctrl);
		if (cc_role == TYPEC_CC_RP)
			act_as_sink = false;
		else
			act_as_sink = true;
	}

	/*
	 * If status is not open, then OR in termination to convert to
	 * enum tcpc_cc_voltage_status.
	 */
	if (*cc1 != TYPEC_CC_VOLT_OPEN)
		*cc1 |= (act_as_sink << 2);

	if (*cc2 != TYPEC_CC_VOLT_OPEN)
		*cc2 |= (act_as_sink << 2);

	rt1711h_init_cc_params(rt_chip, (uint8_t)rt_chip->typec_polarity ?
		*cc2 : *cc1);
	return 0;
}

static int rt1711_set_cc(void *chip, int pull)
{
	int ret;
	uint8_t data;
	int rp_lvl = typec_cc_pull_get_rp_lvl(pull);
	struct rt1711_chip *rt_chip = chip;

	if (!rt_chip)
		return -ENODEV;

	RT1711_INFO("\n");
	pull = typec_cc_pull_get_res(pull);
	if (pull == TYPEC_CC_DRP) {
		data = TCPC_V10_REG_ROLE_CTRL_RES_SET(
				1, rp_lvl, TYPEC_CC_RD, TYPEC_CC_RD);

		ret = rt1711_i2c_write8(rt_chip, TCPC_V10_REG_ROLE_CTRL, data);
		if (ret == 0)
			ret = rt1711_command(rt_chip, TCPM_CMD_LOOK_CONNECTION);
	} else {
#ifdef CONFIG_HW_USB_POWER_DELIVERY
		if ((pull == TYPEC_CC_RD) && tcpci_is_pr_swaping(rt_chip->tcpc))
			rt1711h_init_cc_params(rt_chip, TYPEC_CC_VOLT_SNK_DFT);
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

		data = TCPC_V10_REG_ROLE_CTRL_RES_SET(0, rp_lvl, pull, pull);
		ret = rt1711_i2c_write8(rt_chip, TCPC_V10_REG_ROLE_CTRL, data);
	}

	return 0;
}

static int rt1711_set_polarity(void *chip, int polarity, uint8_t remote_cc)
{
	int data;
	struct rt1711_chip *rt_chip = chip;

	if (!rt_chip)
		return -ENODEV;

	rt_chip->typec_polarity = polarity;
	data = rt1711h_init_cc_params(rt_chip, remote_cc);
	if (data)
		return data;

	data = rt1711_i2c_read8(rt_chip, TCPC_V10_REG_TCPC_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_TCPC_CTRL_PLUG_ORIENT;
	data |= polarity ? TCPC_V10_REG_TCPC_CTRL_PLUG_ORIENT : 0;

	return rt1711_i2c_write8(rt_chip, TCPC_V10_REG_TCPC_CTRL, data);
}

static int rt1711_set_low_rp_duty(void *chip, bool low_rp)
{
	uint16_t duty = low_rp ? TCPC_LOW_RP_DUTY : TCPC_NORMAL_RP_DUTY;

	if (!chip)
		return -ENODEV;

	return rt1711_i2c_write16(chip, RT1711H_REG_DRP_DUTY_CTRL, duty);
}

static int rt1711_set_vconn(void *chip, int enable)
{
	int data;

	if (!chip)
		return -ENODEV;

	data = rt1711_i2c_read8(chip, TCPC_V10_REG_POWER_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_POWER_CTRL_VCONN;
	data |= enable ? TCPC_V10_REG_POWER_CTRL_VCONN : 0;

	return rt1711_i2c_write8(chip, TCPC_V10_REG_POWER_CTRL, data);
}

static int rt1711_is_low_power_mode(void *chip)
{
	int rv;

	if (!chip)
		return -ENODEV;

	rv = rt1711_i2c_read8(chip, RT1711H_REG_BMC_CTRL);
	if (rv < 0)
		return rv;

	return (rv & RT1711H_REG_BMCIO_LPEN) != 0;
}

static int rt1711_set_low_power_mode(void *chip, bool en, int pull)
{
	uint8_t data;

	if (!chip)
		return -ENODEV;

	if (en) {
		data = RT1711H_REG_BMCIO_LPEN;

		if (pull & TYPEC_CC_RP)
			data |= RT1711H_REG_BMCIO_LPRPRD;

		data |= (RT1711H_REG_VBUS_DET_EN | RT1711H_REG_BMCIO_BG_EN);
	} else {
		data = RT1711H_REG_BMCIO_BG_EN | RT1711H_REG_VBUS_DET_EN |
			RT1711H_REG_BMCIO_OSC_EN;
	}

	return rt1711_i2c_write8(chip, RT1711H_REG_BMC_CTRL, data);
}

static int rt1711_tcpc_deinit(void *chip)
{
	if (!chip)
		return -ENODEV;

	rt1711_set_cc(chip, TYPEC_CC_DRP);
	rt1711_set_cc(chip, TYPEC_CC_OPEN);

	/* tout=4 */
	rt1711_i2c_write8(chip, RT1711H_REG_I2CRST_CTRL,
		RT1711H_REG_I2CRST_SET(true, 4));
	rt1711_i2c_write8(chip, RT1711H_REG_INTRST_CTRL,
		RT1711H_REG_INTRST_SET(true, 0));

	return 0;
}

#ifdef CONFIG_HW_USB_POWER_DELIVERY
static int rt1711_set_msg_header(void *chip, uint8_t power_role,
	uint8_t data_role)
{
	uint8_t msg_hdr = TCPC_V10_REG_MSG_HDR_INFO_SET(data_role, power_role);

	if (!chip)
		return -ENODEV;

	return rt1711_i2c_write8(chip, TCPC_V10_REG_MSG_HDR_INFO, msg_hdr);
}

static int rt1711_protocol_reset(void *chip)
{
	if (!chip)
		return -ENODEV;

	rt1711_i2c_write8(chip, RT1711H_REG_PRL_FSM_RESET, 0);
	mdelay(1);
	rt1711_i2c_write8(chip, RT1711H_REG_PRL_FSM_RESET, 1);
	return 0;
}

static int rt1711_set_rx_enable(void *chip, uint8_t enable)
{
	int ret = 0;

	if (!chip)
		return -ENODEV;

	if (enable)
		ret = rt1711h_set_clock_gating(chip, false);

	if (ret == 0)
		ret = rt1711_i2c_write8(chip, TCPC_V10_REG_RX_DETECT, enable);

	if ((ret == 0) && !enable)
		ret = rt1711h_set_clock_gating(chip, true);

	if (!enable)
		rt1711_protocol_reset(chip);

	return ret;
}

static int rt1711_get_message(void *chip, uint32_t *payload, uint16_t *msg_head,
	enum tcpc_pd_transmit_type *frame_type)
{
	struct rt1711_chip *rt_chip = chip;
	int rv;
	uint8_t type;
	uint8_t cnt;
	uint8_t buf[4]; /* buf size is 4 */
	const uint16_t alert_rx = TCPC_V10_REG_ALERT_RX_STATUS |
		TCPC_V10_REG_RX_OVERFLOW;

	if (!rt_chip)
		return -ENODEV;

	rv = rt1711_block_read(rt_chip->client, TCPC_V10_REG_RX_BYTE_CNT,
		ARRAY_SIZE(buf), buf);
	cnt = buf[0];
	type = buf[1];
	*msg_head = *(uint16_t *)&buf[2];

	/* TCPC 1.0 ==> no need to subtract the size of msg_head */
	if ((rv >= 0) && (cnt > 3)) {
		cnt -= 3; /* 3: msg_hdr */
		rv = rt1711_block_read(rt_chip->client, TCPC_V10_REG_RX_DATA,
			cnt, (uint8_t *) payload);
	}
	*frame_type = (enum tcpc_pd_transmit_type)type;

	/* Read complete, clear RX status alert bit */
	rt1711_alert_status_clear(rt_chip, alert_rx);
	return rv;
}

static int rt1711_set_bist_carrier_mode(void *chip, uint8_t pattern)
{
	return 0;
}

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
static int rt1711_retransmit(void *chip, uint8_t retrys)
{
	if (!chip)
		return -ENODEV;

	return rt1711_i2c_write8(chip, TCPC_V10_REG_TRANSMIT,
		TCPC_V10_REG_TRANSMIT_SET(retrys, TCPC_TX_SOP));
}
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */

#pragma pack(push, 1)
struct tcpc_transmit_packet {
	uint8_t cnt;
	uint16_t msg_header;
	/* 7: max data block num */
	uint8_t data[sizeof(uint32_t) * 7];
};
#pragma pack(pop)

static int rt1711_transmit(void *chip, enum tcpc_pd_transmit_type type,
	uint16_t header, const uint32_t *data, uint8_t retrys)
{
	struct rt1711_chip *rt_chip = chip;
	int rv;
	int data_cnt;
	struct tcpc_transmit_packet packet;

	if (!rt_chip)
		return -ENODEV;

	if (type < TCPC_TX_HARD_RESET) {
		data_cnt = sizeof(uint32_t) * tcpc_pd_header_cnt(header);

		packet.cnt = data_cnt + sizeof(uint16_t);
		packet.msg_header = header;

		if (data_cnt > 0)
			memcpy(packet.data, (uint8_t *)data, data_cnt);

		rv = rt1711_block_write(rt_chip->client, TCPC_V10_REG_TX_BYTE_CNT,
			packet.cnt + 1, (uint8_t *)&packet);
		if (rv < 0)
			return rv;
	}

	return rt1711_i2c_write8(rt_chip, TCPC_V10_REG_TRANSMIT,
		TCPC_V10_REG_TRANSMIT_SET(retrys, type));
}

static int rt1711_set_bist_test_mode(void *chip, bool en)
{
	int data;

	if (!chip)
		return -ENODEV;

	data = rt1711_i2c_read8(chip, TCPC_V10_REG_TCPC_CTRL);
	if (data < 0)
		return data;

	data &= ~TCPC_V10_REG_TCPC_CTRL_BIST_TEST_MODE;
	data |= en ? TCPC_V10_REG_TCPC_CTRL_BIST_TEST_MODE : 0;

	return rt1711_i2c_write8(chip, TCPC_V10_REG_TCPC_CTRL, data);
}
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

static rt1711_get_dummy(void *chip, uint32_t *mask)
{
	return 0;
}

static struct tcpc_ops rt1711_tcpc_ops = {
	.init = rt1711_tcpc_init,
	.alert_status_clear = rt1711_alert_status_clear,
	.fault_status_clear = rt1711_fault_status_clear,
	.get_alert_status = rt1711_get_alert_status,
	.get_alert_mask = rt1711_get_dummy,
	.get_power_status = rt1711_get_power_status,
	.get_fault_status = rt1711_get_fault_status,
	.get_cc = rt1711_get_cc,
	.set_cc = rt1711_set_cc,
	.set_polarity = rt1711_set_polarity,
	.set_low_rp_duty = rt1711_set_low_rp_duty,
	.set_vconn = rt1711_set_vconn,
	.deinit = rt1711_tcpc_deinit,
	.is_vsafe0v = rt1711_is_vsafe0v,
	.is_low_power_mode = rt1711_is_low_power_mode,
	.set_low_power_mode = rt1711_set_low_power_mode,

#ifdef CONFIG_HW_USB_POWER_DELIVERY
	.set_msg_header = rt1711_set_msg_header,
	.set_rx_enable = rt1711_set_rx_enable,
	.protocol_reset = rt1711_protocol_reset,
	.get_message = rt1711_get_message,
	.transmit = rt1711_transmit,
	.set_bist_test_mode = rt1711_set_bist_test_mode,
	.set_bist_carrier_mode = rt1711_set_bist_carrier_mode,
#endif /* CONFIG_HW_USB_POWER_DELIVERY */

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	.retransmit = rt1711_retransmit,
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */
};

static int rt_parse_dt(struct rt1711_chip *chip, struct device *dev)
{
	struct device_node *np = NULL;
	int ret;

	pr_info("%s+\n", __func__);

	np = of_find_node_by_name(NULL, "type_c_port0");
	if (!np) {
		pr_err("%s find node type_c_port0 fail\n", __func__);
		return -ENODEV;
	}

#if (!defined(CONFIG_MTK_GPIO) || defined(CONFIG_MTK_GPIOLIB_STAND))
	ret = of_get_named_gpio(np, "rt1711pd,intr_gpio", 0);
	if (ret < 0) {
		pr_err("%s no intr_gpio info\n", __func__);
		return ret;
	}
	chip->irq_gpio = ret;
#else
	ret = of_property_read_u32(np, "rt1711pd,intr_gpio_num", &chip->irq_gpio);
	if (ret < 0)
		pr_err("%s no intr_gpio info\n", __func__);
#endif /* CONFIG_MTK_GPIO || CONFIG_MTK_GPIOLIB_STAND */
	return ret;
}

static int rt1711_tcpcdev_init(struct rt1711_chip *chip, struct device *dev)
{
	uint32_t tcpc_flags;

	chip->tcpc = tcpc_device_register(dev, "rt-tcpc", &rt1711_tcpc_ops, chip);
	if (IS_ERR(chip->tcpc))
		return -EINVAL;

	tcpc_flags = TCPC_FLAGS_LPM_WAKEUP_WATCHDOG;

	if (chip->chip_id > RT1711H_DID_B)
		tcpc_flags |= TCPC_FLAGS_CHECK_RA_DETACHE;

#ifdef CONFIG_USB_PD_RETRY_CRC_DISCARD
	if (chip->chip_id > RT1715_DID_D)
		tcpc_flags |= TCPC_FLAGS_RETRY_CRC_DISCARD;
#endif /* CONFIG_USB_PD_RETRY_CRC_DISCARD */

#ifdef CONFIG_HW_USB_PD_REV30
	if (chip->chip_id >= RT1715_DID_D)
		tcpc_flags |= TCPC_FLAGS_PD_REV30;
	tcpci_set_flags(chip->tcpc, tcpc_flags);

	if (tcpci_is_support(chip->tcpc, TCPC_FLAGS_PD_REV30))
		dev_info(dev, "PD_REV30\n");
	else
		dev_info(dev, "PD_REV20\n");
#endif /* CONFIG_HW_USB_PD_REV30 */

	return 0;
}

static inline int rt1711h_check_revision(struct i2c_client *client)
{
	u16 vid, pid, did;
	int ret;
	u8 data = 1;

	ret = rt1711_read_device(client, TCPC_V10_REG_VID, 2, &vid);
	if (ret < 0) {
		dev_err(&client->dev, "read chip ID fail\n");
		return -EIO;
	}

	if (vid != RICHTEK_1711_VID) {
		pr_info("%s failed, VID=0x%04x\n", __func__, vid);
		return -ENODEV;
	}

	ret = rt1711_read_device(client, TCPC_V10_REG_PID, sizeof(u16), &pid);
	if (ret < 0) {
		dev_err(&client->dev, "read product ID fail\n");
		return -EIO;
	}

	if (pid != RICHTEK_1711_PID) {
		pr_info("%s failed, PID=0x%04x\n", __func__, pid);
		return -ENODEV;
	}

	ret = rt1711_write_device(client, RT1711H_REG_SWRESET, 1, &data);
	if (ret < 0)
		return ret;

	usleep_range(1000, 2000); /* 1ms */

	ret = rt1711_read_device(client, TCPC_V10_REG_DID, sizeof(u16), &did);
	if (ret < 0) {
		dev_err(&client->dev, "read device ID fail\n");
		return -EIO;
	}

	return did;
}

static int rt1711_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct rt1711_chip *chip = NULL;
	int ret = 0;
	int chip_id;
	bool use_dt = client->dev.of_node;

	pr_info("%s+\n", __func__);
	if (i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK |
		I2C_FUNC_SMBUS_BYTE_DATA))
		pr_info("I2C functionality : OK...\n");
	else
		pr_info("I2C functionality check : failuare...\n");

	chip_id = rt1711h_check_revision(client);
	if (chip_id < 0)
		return chip_id;

	chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	if (use_dt)
		rt_parse_dt(chip, &client->dev);
	else {
		dev_err(&client->dev, "no dts node\n");
		return -ENODEV;
	}
	chip->dev = &client->dev;
	chip->client = client;
	sema_init(&chip->io_lock, 1);
	sema_init(&chip->suspend_lock, 1);
	i2c_set_clientdata(client, chip);
	INIT_DELAYED_WORK(&chip->poll_work, rt1711_poll_work);
	wakeup_source_init(&chip->irq_wake_lock, "rt1711h_irq_wakelock");

	chip->chip_id = chip_id;
	pr_info("rt1711h_chipID = 0x%0x\n", chip_id);

	ret = rt1711_regmap_init(chip);
	if (ret < 0) {
		dev_err(chip->dev, "rt1711 regmap init fail\n");
		return -EINVAL;
	}

	ret = rt1711_tcpcdev_init(chip, &client->dev);
	if (ret < 0) {
		dev_err(&client->dev, "rt1711 tcpc dev init fail\n");
		goto err_tcpc_reg;
	}

	ret = rt1711_init_alert(chip->tcpc);
	if (ret < 0) {
		pr_err("rt1711 init alert fail\n");
		goto err_irq_init;
	}

	pr_info("%s probe ok\n", __func__);
	return 0;

err_irq_init:
	tcpc_device_unregister(chip->dev, chip->tcpc);
err_tcpc_reg:
	rt1711_regmap_deinit(chip);
	wakeup_source_trash(&chip->irq_wake_lock);
	return ret;
}

static int rt1711_i2c_remove(struct i2c_client *client)
{
	struct rt1711_chip *chip = i2c_get_clientdata(client);

	if (chip) {
		cancel_delayed_work_sync(&chip->poll_work);
		tcpc_device_unregister(chip->dev, chip->tcpc);
		rt1711_regmap_deinit(chip);
	}

	return 0;
}

#ifdef CONFIG_PM
static int rt1711_i2c_suspend(struct device *dev)
{
	struct rt1711_chip *chip = NULL;
	struct i2c_client *client = to_i2c_client(dev);

	if (client) {
		chip = i2c_get_clientdata(client);
		if (chip)
			down(&chip->suspend_lock);
	}

	return 0;
}

static int rt1711_i2c_resume(struct device *dev)
{
	struct rt1711_chip *chip = NULL;
	struct i2c_client *client = to_i2c_client(dev);

	if (client) {
		chip = i2c_get_clientdata(client);
		if (chip)
			up(&chip->suspend_lock);
	}

	return 0;
}

static void rt1711_shutdown(struct i2c_client *client)
{
	struct rt1711_chip *chip = i2c_get_clientdata(client);

	/* Please reset IC here */
	if (chip) {
		if (chip->irq)
			disable_irq(chip->irq);
		tcpm_shutdown(chip->tcpc);
	} else {
		i2c_smbus_write_byte_data(client, RT1711H_REG_SWRESET, 0x01);
	}
}

#ifdef CONFIG_PM_RUNTIME
static int rt1711_pm_suspend_runtime(struct device *device)
{
	dev_dbg(device, "pm_runtime: suspending...\n");
	return 0;
}

static int rt1711_pm_resume_runtime(struct device *device)
{
	dev_dbg(device, "pm_runtime: resuming...\n");
	return 0;
}
#endif /* #ifdef CONFIG_PM_RUNTIME */

static const struct dev_pm_ops rt1711_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(
		rt1711_i2c_suspend,
		rt1711_i2c_resume)
#ifdef CONFIG_PM_RUNTIME
	SET_RUNTIME_PM_OPS(
		rt1711_pm_suspend_runtime,
		rt1711_pm_resume_runtime,
		NULL)
#endif /* CONFIG_PM_RUNTIME */
};
#define RT1711_PM_OPS    (&rt1711_pm_ops)
#else
#define RT1711_PM_OPS    NULL
#endif /* CONFIG_PM */

static const struct i2c_device_id rt1711_id_table[] = {
	{"rt1711", 0},
	{"rt1715", 0},
	{"rt1716", 0},
	{},
};
MODULE_DEVICE_TABLE(i2c, rt1711_id_table);

static const struct of_device_id rt_match_table[] = {
	{.compatible = "mediatek,usb_type_c",},
	{},
};

static struct i2c_driver rt1711_driver = {
	.driver = {
		.name = "rt_usb_type_c",
		.owner = THIS_MODULE,
		.of_match_table = rt_match_table,
		.pm = RT1711_PM_OPS,
	},
	.probe = rt1711_i2c_probe,
	.remove = rt1711_i2c_remove,
	.shutdown = rt1711_shutdown,
	.id_table = rt1711_id_table,
};

static int __init rt1711_init(void)
{
	struct device_node *np = NULL;

	pr_info("rt1711h_init (%s): initializing...\n", RT1711H_DRV_VERSION);
	np = of_find_node_by_name(NULL, "usb_type_c");
	if (np)
		pr_info("usb_type_c node found...\n");
	else
		pr_info("usb_type_c node not found...\n");

	return i2c_add_driver(&rt1711_driver);
}
subsys_initcall(rt1711_init);

static void __exit rt1711_exit(void)
{
	i2c_del_driver(&rt1711_driver);
}
module_exit(rt1711_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RT1711 TCPC Driver");
MODULE_VERSION(RT1711H_DRV_VERSION);
