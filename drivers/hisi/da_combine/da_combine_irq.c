/*
 * da_combine_irq.c
 *
 * Interrupt controller support for DA_COMBINE
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.
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

#include <linux/hisi/da_combine/da_combine_irq.h>

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/pm_wakeup.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/version.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>
#include <linux/hisi/audio_log.h>

#ifndef NO_IRQ
#define NO_IRQ 0
#endif

#define LOG_TAG "DA_combine_irq"

#define TRY_LOCK_TIMEOUT_MS 1000
#define PHY_IRQ_INDEX 3

struct irq_platform_data {
	/* public data */
	struct da_combine_irq da_combine_irq;
	/* irq controller */
	struct irq_domain *domain;
	unsigned int irq_id;
	unsigned int sub_irq_id[DA_COMBINE_MAX_IRQS_NUM];
	/* mutex for irq thread handler */
	struct mutex handler_mutex;
	/* mutex for irq_mask & irq_source */
	struct mutex irq_lock;
	/* mutex for sr */
	struct mutex sr_lock;
	/* wake lock for irq thread handler */
	struct wakeup_source wake_lock;
	/* used to mask sub irqs */
	unsigned char irq_mask[DA_COMBINE_MAX_IRQ_REGS_NUM];
	/* used to clear sub irqs */
	unsigned char irq_source[DA_COMBINE_MAX_IRQ_REGS_NUM];
	struct da_combine_irq_map phy_irq_map;
	struct hi_cdc_ctrl *hi_cdc;
};

static const struct of_device_id g_irq_match_tbl[] = {
	{ .compatible = "hisilicon,da_combine-irq", },
	{},
};
MODULE_DEVICE_TABLE(of, g_irq_match_tbl);

static const struct of_device_id g_irq_child_match_tbl[] = {
	{ .compatible = "hisilicon,da_combine_v2-codec", },
	{},
};

static irqreturn_t irq_handler(int irq, void *data)
{
	struct irq_platform_data *priv = data;

	if (priv == NULL) {
		AUDIO_LOGE("irq platform data is null");
		return IRQ_NONE;
	}

	__pm_stay_awake(&priv->wake_lock);
	disable_irq_nosync(irq);

	return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_handler_thread(int irq, void *data)
{
	struct irq_platform_data *priv = data;
	unsigned long pending;
	unsigned int status;
	int index = 0; /* irq index in single irq register */
	int offset; /* offset of different irq registers */
	int size;
	int i;

	if (priv == NULL) {
		AUDIO_LOGE("irq platform data is null");
		return IRQ_HANDLED;
	}

	size = priv->phy_irq_map.irq_num / DA_COMBINE_IRQ_REG_BITS_NUM;

	mutex_lock(&priv->sr_lock);

	for (i = 0; i < size; i++) {
		/* 8 irqs in each irq register */
		offset = i * DA_COMBINE_IRQ_REG_BITS_NUM;
		/* get irq status */
		status = hi_cdcctrl_reg_read(priv->hi_cdc,
			priv->phy_irq_map.irq_regs[i]);
		/* get irq status with irq mask status */
		status &= (~hi_cdcctrl_reg_read(priv->hi_cdc,
			priv->phy_irq_map.irq_mask_regs[i]));
		/* clr all unmask irqs */
		hi_cdcctrl_reg_write(priv->hi_cdc,
			priv->phy_irq_map.irq_regs[i], status);

		pending = (unsigned long)status;
		/* handle each irq */
		if (pending != 0) {
			for_each_set_bit(index, &pending, DA_COMBINE_IRQ_REG_BITS_NUM)
				handle_nested_irq(priv->sub_irq_id[index + offset]);
		}
	}

	mutex_unlock(&priv->sr_lock);
	enable_irq(irq);
	__pm_relax(&priv->wake_lock);

	return IRQ_HANDLED;
}

static void irq_mask(struct irq_platform_data *data, unsigned long phy_irq)
{
	int index = phy_irq >> PHY_IRQ_INDEX;

	if (index >= (data->phy_irq_map.irq_num / DA_COMBINE_IRQ_REG_BITS_NUM)) {
		AUDIO_LOGE("phy irq %lu out of range", phy_irq);
		return;
	}

	data->irq_mask[index] |= (unsigned char)BIT(phy_irq & 0x07);
}

static void irq_unmask(struct irq_platform_data *data, unsigned long phy_irq)
{
	int index = phy_irq >> PHY_IRQ_INDEX;

	if (index >= (data->phy_irq_map.irq_num / DA_COMBINE_IRQ_REG_BITS_NUM)) {
		AUDIO_LOGE("phy irq %lu out of range", phy_irq);
		return;
	}

	data->irq_mask[index] &= ~(unsigned char)BIT(phy_irq & 0x07);
	data->irq_source[index] |= (unsigned char)BIT(phy_irq & 0x07);
}

static void irq_state_sync(struct irq_platform_data *data)
{
	int size;
	int i;

	size = data->phy_irq_map.irq_num / DA_COMBINE_IRQ_REG_BITS_NUM;
	for (i = 0; i < size; i++) {
		hi_cdcctrl_reg_write(data->hi_cdc,
			data->phy_irq_map.irq_regs[i],
			(unsigned int)data->irq_source[i]);

		hi_cdcctrl_reg_write(data->hi_cdc,
			data->phy_irq_map.irq_mask_regs[i],
			(unsigned int)data->irq_mask[i]);

		data->irq_source[i] = 0;
	}
}

static void da_combine_irq_mask(struct irq_data *data)
{
	struct irq_platform_data *priv = irq_data_get_irq_chip_data(data);
	unsigned long phy_irq = irqd_to_hwirq(data);

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	irq_mask(priv, phy_irq);
}

static void da_combine_irq_unmask(struct irq_data *data)
{
	struct irq_platform_data *priv = irq_data_get_irq_chip_data(data);
	unsigned long phy_irq = irqd_to_hwirq(data);

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	irq_unmask(priv, phy_irq);
}

static void da_combine_irq_bus_lock(struct irq_data *data)
{
	struct irq_platform_data *priv = irq_data_get_irq_chip_data(data);

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	mutex_lock(&priv->irq_lock);
}

static void da_combine_irq_bus_unlock(struct irq_data *data)
{
	struct irq_platform_data *priv = irq_data_get_irq_chip_data(data);

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	irq_state_sync(priv);

	mutex_unlock(&priv->irq_lock);
}

static struct irq_chip g_irq_chip = {
	.name = "da_combine_irq",
	.irq_mask = da_combine_irq_mask,
	.irq_unmask = da_combine_irq_unmask,
	.irq_disable = da_combine_irq_mask,
	.irq_enable = da_combine_irq_unmask,
	.irq_bus_lock = da_combine_irq_bus_lock,
	.irq_bus_sync_unlock = da_combine_irq_bus_unlock,
};

static int da_combine_irq_map(struct irq_domain *domain, unsigned int virq,
	irq_hw_number_t hw)
{
	struct irq_platform_data *irq = domain->host_data;

	irq_set_chip_data(virq, irq);
	irq_set_chip(virq, &g_irq_chip);
	irq_set_nested_thread(virq, true);
#if KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE
	set_irq_flags(virq, IRQF_VALID);
#endif

	return 0;
}

static const struct irq_domain_ops g_domain_ops = {
	.map = da_combine_irq_map,
	.xlate = irq_domain_xlate_twocell
};

static int irq_domain_init(struct device *dev, struct irq_platform_data *data)
{
	int ret = 0;
	int i;

	/* create irq domain */
	data->domain = irq_domain_add_linear(dev->of_node,
		data->phy_irq_map.irq_num, &g_domain_ops, data);
	if (data->domain == NULL) {
		AUDIO_LOGE("create irq domain error");
		return -ENODEV;
	}

	/* get gpio map */
	for (i = 0; i < data->phy_irq_map.irq_num; i++) {
		data->sub_irq_id[i] = irq_create_mapping(data->domain, i);
		if (data->sub_irq_id[i] == NO_IRQ) {
			AUDIO_LOGE("failed mapping irq ids");
			ret = -ENOSPC;
		}
	}

	return ret;
}

int da_combine_irq_init_irq(struct da_combine_irq *irq_data,
	const struct da_combine_irq_map *irq_map)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;
	struct hi_cdc_ctrl *cdc_data = NULL;
	int size;
	int ret;
	int i;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if (irq_map == NULL) {
		AUDIO_LOGE("irq map is null");
		return -EINVAL;
	}

	cdc_data = data->hi_cdc;

	memcpy(&data->phy_irq_map, irq_map, sizeof(*irq_map));

	size = data->phy_irq_map.irq_num / DA_COMBINE_IRQ_REG_BITS_NUM;
	for (i = 0; i < size; i++) {
		hi_cdcctrl_reg_write(cdc_data,
			data->phy_irq_map.irq_mask_regs[i], 0xFF);
		data->irq_mask[i] = 0xFF;

		hi_cdcctrl_reg_write(cdc_data,
			data->phy_irq_map.irq_regs[i], 0xFF);
		data->irq_source[i] = 0;
	}

	/* get irq_id from gpio */
	data->irq_id = hi_cdcctrl_get_irq(cdc_data);

	/* init irq domain */
	ret = irq_domain_init(data->da_combine_irq.dev, data);
	if (ret < 0) {
		AUDIO_LOGE("request irq domain fail, return is %d", ret);
		return ret;
	}

	/* request gpio irq for codec */
	ret = request_threaded_irq(data->irq_id, irq_handler,
		irq_handler_thread,
		IRQF_TRIGGER_LOW | IRQF_NO_SUSPEND | IRQF_ONESHOT,
		dev_name(data->da_combine_irq.dev), data);
	if (ret < 0) {
		AUDIO_LOGE("register irq fail, return is %d", ret);
		if (data->domain != NULL)
			irq_domain_remove(data->domain);
	}

	return ret;
}

void da_combine_irq_deinit_irq(struct da_combine_irq *irq_data)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return;
	}

	free_irq(data->irq_id, data);

	if (data->domain != NULL)
		irq_domain_remove(data->domain);
}

int da_combine_irq_request_irq(struct da_combine_irq *irq_data, int phy_irq,
	irq_handler_t handler, const char *name, void *priv)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if (name == NULL) {
		AUDIO_LOGE("name is null");
		return -EINVAL;
	}

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return -EINVAL;
	}

	if ((phy_irq >= data->phy_irq_map.irq_num) || (phy_irq < 0)) {
		AUDIO_LOGE("physical irq id %d error", phy_irq);
		return -EPERM;
	}

	return request_threaded_irq(data->sub_irq_id[phy_irq], NULL, handler,
		IRQF_NO_SUSPEND | IRQF_ONESHOT, name, priv);
}

void da_combine_irq_free_irq(struct da_combine_irq *irq_data, int phy_irq, void *priv)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return;
	}

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if ((phy_irq >= data->phy_irq_map.irq_num) || (phy_irq < 0)) {
		AUDIO_LOGE("physical irq id %d out of range", phy_irq);
		return;
	}

	free_irq(data->sub_irq_id[phy_irq], priv);
}

int da_combine_irq_enable_irq(struct da_combine_irq *irq_data, int phy_irq)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if ((phy_irq >= data->phy_irq_map.irq_num) || (phy_irq < 0)) {
		AUDIO_LOGE("physical irq id %d out of range", phy_irq);
		return -EINVAL;
	}

	enable_irq(data->sub_irq_id[phy_irq]);

	return 0;
}

int da_combine_irq_disable_irq(struct da_combine_irq *irq_data, int phy_irq)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if ((phy_irq >= data->phy_irq_map.irq_num) || (phy_irq < 0)) {
		AUDIO_LOGE("physical irq id %d out of range", phy_irq);
		return -EINVAL;
	}

	disable_irq(data->sub_irq_id[phy_irq]);

	return 0;
}

int da_combine_irq_enable_irqs(struct da_combine_irq *irq_data,
	int irq_num, const int *phy_irqs)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;
	int i;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if (phy_irqs == NULL) {
		AUDIO_LOGE("phy irqs is null");
		return -EINVAL;
	}

	mutex_lock(&data->irq_lock);

	for (i = 0; i < irq_num; i++)
		irq_unmask(data, phy_irqs[i]);

	irq_state_sync(data);

	mutex_unlock(&data->irq_lock);

	return 0;
}

int da_combine_irq_disable_irqs(struct da_combine_irq *irq_data,
	int irq_num, const int *phy_irqs)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;
	int i;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if (phy_irqs == NULL) {
		AUDIO_LOGE("phy irqs is null");
		return -EINVAL;
	}

	mutex_lock(&data->irq_lock);

	for (i = 0; i < irq_num; i++)
		irq_mask(data, phy_irqs[i]);

	irq_state_sync(data);

	mutex_unlock(&data->irq_lock);

	return 0;
}

void da_combine_irq_resume_wait(struct da_combine_irq *irq_data)
{
	struct irq_platform_data *data = (struct irq_platform_data *)irq_data;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return;
	}

	mutex_lock(&data->sr_lock);
	mutex_unlock(&data->sr_lock);
}

static int da_combine_irq_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct irq_platform_data *data = NULL;
	struct hi_cdc_ctrl *cdc_data = NULL;
	const struct of_device_id *match = NULL;

	AUDIO_LOGI("enter");

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (data == NULL) {
		AUDIO_LOGE("devm kzalloc error");
		return -ENOMEM;
	}

	match = of_match_device(g_irq_match_tbl, dev);
	if (match == NULL) {
		AUDIO_LOGE("get device info err");
		return -ENOENT;
	}

	mutex_init(&data->irq_lock);
	mutex_init(&data->sr_lock);
	mutex_init(&data->handler_mutex);
	wakeup_source_init(&data->wake_lock, "da_combine-irq");

	data->da_combine_irq.dev = dev;

	platform_set_drvdata(pdev, data);
	/* get parent data */
	cdc_data = (struct hi_cdc_ctrl *)dev_get_drvdata(pdev->dev.parent);
	data->hi_cdc = cdc_data;

	/* populate sub nodes */
	of_platform_populate(np, g_irq_child_match_tbl, NULL, dev);

	AUDIO_LOGI("ok");

	return 0;
}

static int da_combine_irq_remove(struct platform_device *pdev)
{
	struct irq_platform_data *data = platform_get_drvdata(pdev);

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	free_irq(data->irq_id, data);

	wakeup_source_trash(&data->wake_lock);
	mutex_destroy(&data->handler_mutex);
	mutex_destroy(&data->sr_lock);
	mutex_destroy(&data->irq_lock);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static int da_combine_irq_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct irq_platform_data *data = platform_get_drvdata(pdev);
	int ret;
	unsigned long cur_time;
	unsigned long timeout;

	AUDIO_LOGI("enter");

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	timeout = jiffies + msecs_to_jiffies(TRY_LOCK_TIMEOUT_MS);
	while (mutex_trylock(&data->sr_lock) == 0) {
		cur_time = jiffies;
		if (time_after(cur_time, timeout)) {
			AUDIO_LOGE("mutex trylock timeout fail");
			return -EAGAIN;
		}

		usleep_range(1000, 2000);
	}

	ret = da_combine_dsp_misc_suspend();
	if (ret != 0)
		mutex_unlock(&data->sr_lock);

	return ret;
}

static int da_combine_irq_resume(struct platform_device *pdev)
{
	struct irq_platform_data *data = platform_get_drvdata(pdev);
	int ret;

	AUDIO_LOGI("enter");

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	ret = da_combine_dsp_misc_resume();
	if (ret != 0)
		AUDIO_LOGE("hifi misc resume failed");

	mutex_unlock(&data->sr_lock);

	return ret;
}

static struct platform_driver g_irq_driver = {
	.driver = {
		.name = "da_combine_irq",
		.owner = THIS_MODULE,
		.of_match_table = g_irq_match_tbl,
	},
	.probe = da_combine_irq_probe,
	.remove = da_combine_irq_remove,
	.suspend = da_combine_irq_suspend,
	.resume = da_combine_irq_resume,
};

static int __init da_combine_irq_init(void)
{
	return platform_driver_register(&g_irq_driver);
}

static void __exit da_combine_irq_exit(void)
{
	platform_driver_unregister(&g_irq_driver);
}

fs_initcall_sync(da_combine_irq_init);
module_exit(da_combine_irq_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("da_combine irq controller driver");

