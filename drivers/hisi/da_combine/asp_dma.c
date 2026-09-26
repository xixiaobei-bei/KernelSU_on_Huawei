/*
 * asp_dma.c
 *
 * asp dma driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#include <linux/hisi/da_combine/asp_dma.h>

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/of_device.h>
#include <linux/hwspinlock.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/hisi/audio_log.h>

#define LOG_TAG "asp_dma"

#define HWLOCK_WAIT_TIME 50
#define ASP_DMA_HWLOCK_ID 6
#define ASP_DMA_STOP_RETRY_TIMES 40
#define REG_BIT_CLR 0
#define REG_BIT_SET 1

enum {
	IRQ_ERR1 = 0,
	IRQ_ERR2,
	IRQ_ERR3,
	IRQ_TC1,
	IRQ_TC2,
	IRQ_TYPE_BUTT
};

struct dma_callback {
	callback_t callback;
	unsigned long para;
};

struct dma_priv {
	struct device *dev;
	int irq;
	spinlock_t lock;
	struct resource *res;
	struct hwspinlock *hwlock;
	struct wakeup_source wake_lock;
	void __iomem *reg_base_addr;
	struct dma_callback callback[ASP_DMA_MAX_CHANNEL_NUM];
};

static struct dma_priv *g_dma_priv;

unsigned int dmac_reg_read(unsigned int reg)
{
	struct dma_priv *priv = g_dma_priv;
	unsigned long flag = 0;
	unsigned int ret;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return 0;
	}

	if (hwspin_lock_timeout_irqsave(priv->hwlock,
		HWLOCK_WAIT_TIME, &flag) != 0) {
		AUDIO_LOGE("hwspinlock timeout");
		return 0;
	}

	ret = readl(priv->reg_base_addr + reg);

	hwspin_unlock_irqrestore(priv->hwlock, &flag);

	return ret;
}

static void dmac_reg_write(unsigned int reg, unsigned int value)
{
	struct dma_priv *priv = g_dma_priv;
	unsigned long flag = 0;

	if (hwspin_lock_timeout_irqsave(priv->hwlock,
		HWLOCK_WAIT_TIME, &flag) != 0) {
		AUDIO_LOGE("hwspinlock timeout");
		return;
	}

	writel(value, priv->reg_base_addr + reg);

	hwspin_unlock_irqrestore(priv->hwlock, &flag);
}

static void dmac_reg_write_bit(unsigned int reg, unsigned int offset,
	unsigned int event)
{
	struct dma_priv *priv = g_dma_priv;
	unsigned int value;
	unsigned long flag_hw = 0;
	unsigned long flag_sft = 0;

	if (hwspin_lock_timeout_irqsave(priv->hwlock,
		HWLOCK_WAIT_TIME, &flag_hw) != 0) {
		AUDIO_LOGE("hwspinlock timeout");
		return;
	}

	spin_lock_irqsave(&priv->lock, flag_sft);

	value = readl(priv->reg_base_addr + reg);

	if (event == REG_BIT_CLR)
		value &= ~(BIT(offset));
	else
		value |= BIT(offset);

	writel(value, priv->reg_base_addr + reg);

	spin_unlock_irqrestore(&priv->lock, flag_sft);

	hwspin_unlock_irqrestore(priv->hwlock, &flag_hw);
}

static void dmac_dump(unsigned int channel)
{
	AUDIO_LOGD("a count: %u", dmac_reg_read(ASP_DMA_CX_CNT0(channel)));
	AUDIO_LOGD("src addr: %u", dmac_reg_read(ASP_DMA_CX_SRC_ADDR(channel)));
	AUDIO_LOGD("des addr: %u", dmac_reg_read(ASP_DMA_CX_DES_ADDR(channel)));
	AUDIO_LOGD("lli: 0x%x", dmac_reg_read(ASP_DMA_CX_LLI(channel)));
	AUDIO_LOGD("config: %u", dmac_reg_read(ASP_DMA_CX_CONFIG(channel)));

	AUDIO_LOGD("c count: %u", dmac_reg_read(ASP_DMA_CX_CNT1(channel)));
	AUDIO_LOGD("b index: %u", dmac_reg_read(ASP_DMA_CX_BINDX(channel)));
	AUDIO_LOGD("c index: %u", dmac_reg_read(ASP_DMA_CX_CINDX(channel)));
}

static irqreturn_t irq_handler(int irq, void *data)
{
	struct dma_priv *priv = g_dma_priv;
	unsigned int mask;
	unsigned int type;
	unsigned int state;
	unsigned int i;
	unsigned int val[IRQ_TYPE_BUTT] = {0};

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return IRQ_HANDLED;
	}

	state = dmac_reg_read(ASP_DMA_INT_STAT_AP);
	if (state == 0)
		return IRQ_HANDLED;

	val[IRQ_ERR1] = dmac_reg_read(ASP_DMA_INT_ERR1_AP);
	val[IRQ_ERR2] = dmac_reg_read(ASP_DMA_INT_ERR2_AP);
	val[IRQ_ERR3] = dmac_reg_read(ASP_DMA_INT_ERR3_AP);
	val[IRQ_TC1] = dmac_reg_read(ASP_DMA_INT_TC1_AP);
	val[IRQ_TC2] = dmac_reg_read(ASP_DMA_INT_TC2_AP);

	/* clr interupt states */
	dmac_reg_write(ASP_DMA_INT_TC1_RAW, state);
	dmac_reg_write(ASP_DMA_INT_TC2_RAW, state);
	dmac_reg_write(ASP_DMA_INT_ERR1_RAW, state);
	dmac_reg_write(ASP_DMA_INT_ERR2_RAW, state);
	dmac_reg_write(ASP_DMA_INT_ERR3_RAW, state);

	for (i = 0; i < ASP_DMA_MAX_CHANNEL_NUM; i++) {
		mask = BIT(i);
		if (!(state & mask))
			continue;

		if (priv->callback[i].callback == NULL)
			continue;

		if (val[IRQ_ERR1] & mask)
			type = ASP_DMA_INT_TYPE_ERR1;
		else if (val[IRQ_ERR2] & mask)
			type = ASP_DMA_INT_TYPE_ERR2;
		else if (val[IRQ_ERR3] & mask)
			type = ASP_DMA_INT_TYPE_ERR3;
		else if (val[IRQ_TC1] & mask)
			type = ASP_DMA_INT_TYPE_TC1;
		else if (val[IRQ_TC2] & mask)
			type = ASP_DMA_INT_TYPE_TC2;
		else
			type = ASP_DMA_INT_TYPE_BUTT;

		priv->callback[i].callback(type, priv->callback[i].para, i);
	}

	return IRQ_HANDLED;
}

unsigned int asp_dma_get_des(unsigned short channel)
{
	unsigned int addr;

	if (g_dma_priv == NULL) {
		AUDIO_LOGE("asp dma priv is null");
		return 0;
	}

	if (channel >= ASP_DMA_MAX_CHANNEL_NUM) {
		AUDIO_LOGE("channel %hu is err", channel);
		return 0;
	}

	addr = dmac_reg_read(ASP_DMA_CX_DES_ADDR(channel));

	return addr;
}

unsigned int asp_dma_get_src(unsigned short channel)
{
	if (g_dma_priv == NULL) {
		AUDIO_LOGE("asp dma priv is null");
		return 0;
	}

	if (channel >= ASP_DMA_MAX_CHANNEL_NUM) {
		AUDIO_LOGE("channel %hu is err", channel);
		return 0;
	}

	return dmac_reg_read(ASP_DMA_CX_SRC_ADDR(channel));
}

static int check_dma_para(const struct dma_priv *priv,
	unsigned short channel, const struct dma_lli_cfg *lli_cfg)
{
	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return -EINVAL;
	}

	if (channel >= ASP_DMA_MAX_CHANNEL_NUM) {
		AUDIO_LOGE("channel %hu is err", channel);
		return -EINVAL;
	}

	if (lli_cfg == NULL) {
		AUDIO_LOGE("lli cfg is null");
		return -EINVAL;
	}

	return 0;
}

int asp_dma_config(unsigned short channel, const struct dma_lli_cfg *lli_cfg,
	callback_t callback, unsigned long para)
{
	struct dma_priv *priv = g_dma_priv;
	unsigned int channel_mask = BIT(channel);

	if (check_dma_para(priv, channel, lli_cfg) != 0)
		return -EINVAL;

	/* disable dma channel */
	dmac_reg_write_bit(ASP_DMA_CX_CONFIG(channel), 0, REG_BIT_CLR);
	dmac_reg_write(ASP_DMA_CX_CNT0(channel), lli_cfg->a_count);

	/* c count */
	dmac_reg_write(ASP_DMA_CX_CNT1(channel), 0);
	dmac_reg_write(ASP_DMA_CX_BINDX(channel), 0);
	dmac_reg_write(ASP_DMA_CX_CINDX(channel), 0);

	/* set dma src/des addr */
	dmac_reg_write(ASP_DMA_CX_SRC_ADDR(channel), lli_cfg->src_addr);
	dmac_reg_write(ASP_DMA_CX_DES_ADDR(channel), lli_cfg->des_addr);

	/* set dma lli config */
	dmac_reg_write(ASP_DMA_CX_LLI(channel), lli_cfg->lli);

	/* clr irq status of dma channel */
	dmac_reg_write(ASP_DMA_INT_TC1_RAW, channel_mask);
	dmac_reg_write(ASP_DMA_INT_TC2_RAW, channel_mask);
	dmac_reg_write(ASP_DMA_INT_ERR1_RAW, channel_mask);
	dmac_reg_write(ASP_DMA_INT_ERR2_RAW, channel_mask);
	dmac_reg_write(ASP_DMA_INT_ERR3_RAW, channel_mask);

	if (callback != NULL) {
		priv->callback[channel].callback = callback;
		priv->callback[channel].para = para;

		/* release irq mask */
		dmac_reg_write_bit(ASP_DMA_INT_ERR1_MASK_AP, channel, REG_BIT_SET);
		dmac_reg_write_bit(ASP_DMA_INT_ERR2_MASK_AP, channel, REG_BIT_SET);
		dmac_reg_write_bit(ASP_DMA_INT_ERR3_MASK_AP, channel, REG_BIT_SET);
		dmac_reg_write_bit(ASP_DMA_INT_TC1_MASK_AP, channel, REG_BIT_SET);
		dmac_reg_write_bit(ASP_DMA_INT_TC2_MASK_AP, channel, REG_BIT_SET);
	}

	AUDIO_LOGI("dma config succ");

	return 0;
}

int asp_dma_start(unsigned short channel, const struct dma_lli_cfg *lli_cfg)
{
	unsigned int lli_reg;

	if (check_dma_para(g_dma_priv, channel, lli_cfg) != 0)
		return -EINVAL;

	lli_reg = dmac_reg_read(ASP_DMA_CX_LLI(channel));
	if (lli_reg != lli_cfg->lli) {
		AUDIO_LOGE("lli is changed, lli reg: %u, lli cfg: %u",
			lli_reg, lli_cfg->lli);
		return -EINVAL;
	}

	dmac_reg_write(ASP_DMA_CX_CONFIG(channel), lli_cfg->config);

	dmac_dump(channel);

	AUDIO_LOGI("dma start succ");

	return 0;
}

void asp_dma_suspend(unsigned short channel)
{
	unsigned int mask = BIT(channel);
	unsigned int i = ASP_DMA_STOP_RETRY_TIMES;
	unsigned int state;

	if (g_dma_priv == NULL) {
		AUDIO_LOGE("asp dma priv is null");
		return;
	}

	if (channel >= ASP_DMA_MAX_CHANNEL_NUM) {
		AUDIO_LOGE("dma channel err: %hu", channel);
		return;
	}

	/* disable dma channel */
	dmac_reg_write_bit(ASP_DMA_CX_CONFIG(channel), 0, REG_BIT_CLR);

	do {
		state = dmac_reg_read(ASP_DMA_CH_STAT) & mask;
		if (state == 0)
			break;

		AUDIO_LOGI("stopping dma channel: %hu", channel);
		udelay(250);
	} while (--i);

	if (i == 0) {
		AUDIO_LOGE("dma channel: %hu suspend fail, channel state: %u",
			channel, state);
		return;
	}

	AUDIO_LOGI("dma channel: %hu suspend succ", channel);
}

void asp_dma_stop(unsigned short channel)
{
	struct dma_priv *priv = g_dma_priv;
	unsigned int channel_mask = BIT(channel);
	unsigned int i = ASP_DMA_STOP_RETRY_TIMES;
	unsigned int state;

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if (channel >= ASP_DMA_MAX_CHANNEL_NUM) {
		AUDIO_LOGE("channel err: %hu", channel);
		return;
	}

	/* disable dma channel */
	dmac_reg_write_bit(ASP_DMA_CX_CONFIG(channel), 0, REG_BIT_CLR);

	do {
		state = dmac_reg_read(ASP_DMA_CH_STAT) & channel_mask;
		if (state == 0)
			break;

		AUDIO_LOGI("stopping channel: %hu", channel);
		udelay(250);
	} while (--i);

	dmac_reg_write_bit(ASP_DMA_INT_ERR1_MASK_AP, channel, REG_BIT_CLR);
	dmac_reg_write_bit(ASP_DMA_INT_ERR2_MASK_AP, channel, REG_BIT_CLR);
	dmac_reg_write_bit(ASP_DMA_INT_ERR3_MASK_AP, channel, REG_BIT_CLR);
	dmac_reg_write_bit(ASP_DMA_INT_TC1_MASK_AP, channel, REG_BIT_CLR);
	dmac_reg_write_bit(ASP_DMA_INT_TC2_MASK_AP, channel, REG_BIT_CLR);

	memset(&priv->callback[channel], 0, sizeof(priv->callback[channel]));

	if (i == 0) {
		AUDIO_LOGE("channel: %hu stop fail, channel state: %u",
			channel, state);
		return;
	}

	AUDIO_LOGI("channel: %hu stop succ", channel);
}

static int remap_base_addr(struct platform_device *pdev,
	struct dma_priv *priv, struct device *dev)
{
	priv->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (priv->res == NULL) {
		AUDIO_LOGE("get resource failed");
		return -ENOENT;
	}

	if (!devm_request_mem_region(dev, priv->res->start,
		resource_size(priv->res), "asp dma")) {
		AUDIO_LOGE("request mem region failed");
		return -ENOMEM;
	}

	priv->reg_base_addr =
		devm_ioremap(dev, priv->res->start, resource_size(priv->res));
	if (priv->reg_base_addr == NULL) {
		AUDIO_LOGE("asp dma reg addr ioremap failed");
		return -ENOMEM;
	}

	return 0;
}

static int dma_irq_init(struct platform_device *pdev,
	struct dma_priv *priv, struct device *dev)
{
	int ret;

	priv->irq = platform_get_irq_byname(pdev, "asp_dma_irq");
	if (priv->irq < 0) {
		AUDIO_LOGE("get asp dma irq failed");
		return -ENOENT;
	}

	if (of_property_read_bool(dev->of_node, "oneshot_irq")) {
		AUDIO_LOGI("oneshot irq mode enable");
		ret = request_threaded_irq(priv->irq, NULL, irq_handler,
			  IRQF_NO_SUSPEND | IRQF_ONESHOT, "asp_dma_irq", dev);
	} else {
		AUDIO_LOGI("shared irq mode enable");
		ret = request_irq(priv->irq, irq_handler,
			  IRQF_SHARED | IRQF_NO_SUSPEND, "asp_dma_irq", dev);
	}

	if (ret != 0) {
		AUDIO_LOGE("request asp dma irq failed");
		return -ENOENT;
	}

	return 0;
}

static void unremap_base_addr(struct dma_priv *priv)
{
	if (priv->reg_base_addr != NULL) {
		devm_iounmap(priv->dev, priv->reg_base_addr);
		priv->reg_base_addr = NULL;
		devm_release_mem_region(priv->dev, priv->res->start,
			resource_size(priv->res));
	}
}

static int asp_dma_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct dma_priv *priv = NULL;

	AUDIO_LOGI("enter");

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		AUDIO_LOGE("malloc failed");
		return -ENOMEM;
	}

	ret = remap_base_addr(pdev, priv, dev);
	if (ret != 0) {
		AUDIO_LOGE("remap base addr failed");
		goto remap_err;
	}

	ret = dma_irq_init(pdev, priv, dev);
	if (ret != 0) {
		AUDIO_LOGE("rdma irq init failed");
		goto irq_init_err;
	}

	priv->hwlock = hwspin_lock_request_specific(ASP_DMA_HWLOCK_ID);
	if (priv->hwlock == NULL) {
		AUDIO_LOGE("request hw spinlok failed");
		ret = -ENOENT;
		goto hwspin_lock_err;
	}

	wakeup_source_init(&priv->wake_lock, "asp_dma");

	spin_lock_init(&priv->lock);

	priv->dev = dev;

	platform_set_drvdata(pdev, priv);

	g_dma_priv = priv;

	AUDIO_LOGI("exit");

	return 0;

hwspin_lock_err:
	free_irq(priv->irq, priv);
irq_init_err:
	unremap_base_addr(priv);
remap_err:
	AUDIO_LOGE("probe failed");

	return ret;
}

static int asp_dma_remove(struct platform_device *pdev)
{
	struct dma_priv *priv =
		(struct dma_priv *)platform_get_drvdata(pdev);

	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return 0;
	}

	wakeup_source_trash(&priv->wake_lock);

	if (hwspin_lock_free(priv->hwlock))
		AUDIO_LOGE("hwspinlock free failed");

	free_irq(priv->irq, priv);

	unremap_base_addr(priv);

	g_dma_priv = NULL;

	return 0;
}

static const struct of_device_id g_dma_match_tbl[] = {
	{ .compatible = "hisilicon,da_combine-asp-dma", },
	{},
};

MODULE_DEVICE_TABLE(of, g_dma_match_tbl);

static struct platform_driver g_dma_driver = {
	.driver = {
		.name = "da_combine_asp_dma_drv",
		.owner = THIS_MODULE,
		.of_match_table = g_dma_match_tbl,
	},
	.probe = asp_dma_probe,
	.remove = asp_dma_remove,
};

static int __init asp_dma_init(void)
{
	return platform_driver_register(&g_dma_driver);
}
module_init(asp_dma_init);

static void __exit asp_dma_exit(void)
{
	platform_driver_unregister(&g_dma_driver);
}
module_exit(asp_dma_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisilicon (R) ASP DMA Driver");

