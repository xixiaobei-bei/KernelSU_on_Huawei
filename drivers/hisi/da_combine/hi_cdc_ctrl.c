/*
 * hi_cdc_ctrl.c
 *
 * codec control driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hisi/da_combine/hi_cdc_ctrl.h>

#include <linux/module.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/types.h>
#include <linux/hisi/audio_log.h>
#include <linux/hisi/da_combine/hi_cdc_ssi.h>
#include <rdr_audio_adapter.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#define LOG_TAG "hi_cdc_ctrl"
#define LDO8_VOLTAGE 1800000
#define LDO8_ENABLE 1
#define SLIMBUS_CLK_DRV_FORCE 0x5
#define SLIMBUS_DATA_DRV_FORCE 0x5
#define REC_ITEM_NUM 128
#define GPIO_LOW 0
#define GPIO_HIGH 1

struct reg_ops {
	unsigned int (*read8)(unsigned int reg);
	unsigned int (*read32)(unsigned int reg);
	void (*write8)(unsigned int reg, unsigned int val);
	void (*write32)(unsigned int reg, unsigned int val);
};

struct reg_op_item {
	struct timespec tm;
	int rw;
	unsigned int reg;
	unsigned int val;
};

struct hi_cdc_ctrl_priv {
	struct hi_cdc_ctrl cdc_ctrl;
	struct mutex io_mutex;
	struct reg_ops reg_ops;
	struct clk *cdc_mclk;
	struct regulator *supplies[CDC_SUP_MAX];
	struct regulator *ldo8;
	struct reg_op_item op_rec[REC_ITEM_NUM];
	int rec_wr_idx;
	int irq;
	unsigned int reg_addr8_begin;
	unsigned int reg_addr8_end;
	unsigned int pmu_codec_mclk_addr;
};

enum record_op_type {
	REG_READ,
	REG_WRITE
};

static struct hi_cdc_ctrl_priv *g_cdc_ctrl_priv;

static void record_reg_op(struct hi_cdc_ctrl_priv *priv, int rw,
	unsigned int reg_addr, unsigned int reg_val)
{
	int idx = priv->rec_wr_idx;
	struct reg_op_item *item = &priv->op_rec[idx];

	get_monotonic_boottime(&item->tm);
	item->rw = rw;
	item->reg = reg_addr;
	item->val = reg_val;

	if (++priv->rec_wr_idx == ARRAY_SIZE(priv->op_rec))
		priv->rec_wr_idx = 0;
}

static void record_reg_dump(struct hi_cdc_ctrl_priv *priv)
{
	unsigned int i;
	int j;
	struct reg_op_item *item = NULL;

	AUDIO_LOGI("register operations dump begin");
	mutex_lock(&priv->io_mutex);

	for (i = priv->rec_wr_idx; i < ARRAY_SIZE(priv->op_rec); i++) {
		item = &priv->op_rec[i];
		AUDIO_LOGI("[%6ld.%09ld]%c: %08x, %08x", item->tm.tv_sec,
			item->tm.tv_nsec, item->rw ? 'w' : 'r', item->reg, item->val);
	}

	for (j = 0; j < priv->rec_wr_idx; j++) {
		item = &priv->op_rec[j];
		AUDIO_LOGI("[%6ld.%09ld]%c: %08x, %08x", item->tm.tv_sec,
			item->tm.tv_nsec, item->rw ? 'w' : 'r', item->reg, item->val);
	}
	mutex_unlock(&priv->io_mutex);
	AUDIO_LOGI("register operations dump end");
}

void hi_cdcctrl_dump(struct hi_cdc_ctrl *cdc_ctrl)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return;
	}

	record_reg_dump(priv);
}

static void ssi_check_dsm_report(struct hi_cdc_ctrl *cdc_ctrl,
	unsigned int val1, unsigned int val2)
{
	if (!cdc_ctrl->ssi_check_enable)
		return;

	if (val1 != val2) {
		cdc_ctrl->ssi_check_enable = SSI_STATE_CHECK_DISABLE;
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_SSI_READ_ONCE,
			"da combine ssi read once err, val1 is 0x%x, val2 is 0x%x\n",
			val1, val2);
#endif
		AUDIO_LOGW("ssi read once err, val1 is 0x%x, val2 is 0x%x", val1, val2);
	}
}

unsigned int hi_cdcctrl_reg_read(struct hi_cdc_ctrl *cdc_ctrl, unsigned int addr)
{
	int pm_ret;
	unsigned int val1;
	unsigned int val2;
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return 0;
	}

	mutex_lock(&priv->io_mutex);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_ret = pm_runtime_get_sync(cdc_ctrl->dev);
		if (pm_ret < 0) {
			AUDIO_LOGE("pm resume error, reg addr: 0x%pK pm ret: %d",
				(void *)(uintptr_t)addr, pm_ret);
			mutex_unlock(&priv->io_mutex);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return 0;
		}
	}
	/*
	 * avoid hardware noise disturbance ssi frame error, read twice and
	 * only use value of second reading
	 */
	if (addr >= priv->reg_addr8_begin &&
		addr <= priv->reg_addr8_end) {
		val1 = priv->reg_ops.read8(addr);
		val2 = priv->reg_ops.read8(addr);
	} else {
		val1 = priv->reg_ops.read32(addr);
		val2 = priv->reg_ops.read32(addr);
	}

	ssi_check_dsm_report(&priv->cdc_ctrl, val1, val2);
	record_reg_op(priv, REG_READ, addr, val2);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl->dev);
		pm_runtime_put_autosuspend(cdc_ctrl->dev);
	}

	mutex_unlock(&priv->io_mutex);

	return val2;
}

int hi_cdcctrl_reg_write(struct hi_cdc_ctrl *cdc_ctrl, unsigned int addr, unsigned int val)
{
	int ret;
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return 0;
	}

	mutex_lock(&priv->io_mutex);

	if (priv->cdc_ctrl.pm_runtime_support) {
		ret = pm_runtime_get_sync(cdc_ctrl->dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, reg addr: 0x%pK ret: %d",
				(void *)(uintptr_t)addr, ret);
			mutex_unlock(&priv->io_mutex);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return 0;
		}
	}

	record_reg_op(priv, REG_WRITE, addr, val);

	if (addr >= priv->reg_addr8_begin && addr <= priv->reg_addr8_end)
		priv->reg_ops.write8(addr, val);
	else
		priv->reg_ops.write32(addr, val);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl->dev);
		pm_runtime_put_autosuspend(cdc_ctrl->dev);
	}

	mutex_unlock(&priv->io_mutex);

	return 0;
}

void hi_cdcctrl_reg_update_bits(struct hi_cdc_ctrl *cdc_ctrl, unsigned int reg,
	unsigned int mask, unsigned int value)
{
	int ret;
	unsigned int old;
	unsigned int new;
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return;
	}

	if (priv->cdc_ctrl.pm_runtime_support) {
		ret = pm_runtime_get_sync(cdc_ctrl->dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, reg: 0x%pK ret: %d",
				(void *)(uintptr_t)reg, ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);

			return;
		}
	}
	old = hi_cdcctrl_reg_read(cdc_ctrl, reg);

	new = (old & ~mask) | (value & mask);
	hi_cdcctrl_reg_write(cdc_ctrl, reg, new);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(cdc_ctrl->dev);
		pm_runtime_put_autosuspend(cdc_ctrl->dev);
	}
}

int hi_cdcctrl_hw_reset(const struct hi_cdc_ctrl *cdc_ctrl)
{
	/* do nothing */
	return 0;
}

int hi_cdcctrl_get_irq(const struct hi_cdc_ctrl *cdc_ctrl)
{
	const struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return 0;
	}

	return priv->irq;
}

int hi_cdcctrl_enable_supply(struct hi_cdc_ctrl *cdc_ctrl,
	enum hi_cdcctrl_supply sup_type, bool enable)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGW("cdc ctrl priv is null");
		return 0;
	}

	if (sup_type >= CDC_SUP_MAX) {
		AUDIO_LOGW("sup type: %d is out of bounds", sup_type);
		return 0;
	}

	if (priv->supplies[sup_type] == NULL) {
		AUDIO_LOGW("cdc ctrl supplies is null");
		return 0;
	}

	if (enable)
		return regulator_enable(priv->supplies[sup_type]);

	return regulator_disable(priv->supplies[sup_type]);
}

int hi_cdcctrl_enable_clk(struct hi_cdc_ctrl *cdc_ctrl,
	enum hi_cdcctrl_clk clk_type, bool enable)
{
	struct hi_cdc_ctrl_priv *priv = (struct hi_cdc_ctrl_priv *)cdc_ctrl;

	if (priv == NULL) {
		AUDIO_LOGW("cdc ctrl priv is null");
		return 0;
	}

	if (clk_type == CDC_MCLK) {
		if (priv->cdc_mclk == NULL) {
			AUDIO_LOGW("cdc mclk is null");
			return 0;
		}

		if (enable)
			return clk_prepare_enable(priv->cdc_mclk);

		clk_disable_unprepare(priv->cdc_mclk);
	}

	return 0;
}

unsigned int hi_cdcctrl_get_pmu_mclk_status(void)
{
	if (g_cdc_ctrl_priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return 0;
	}

	if (g_cdc_ctrl_priv->pmu_codec_mclk_addr == 0) {
		AUDIO_LOGE("mclk addr get error");
		return 0;
	}

	return pmic_read_reg(g_cdc_ctrl_priv->pmu_codec_mclk_addr);
}

static const struct of_device_id of_codec_controller_child_match_tbl[] = {
	{ .compatible = "hisilicon,da_combine-irq", },
	{}
};

void hi_cdc_bus_type_select(struct hi_cdc_ctrl_priv *priv,
	struct device *dev)
{
	int ret;
	const char *str = NULL;

	if (dev == NULL) {
		AUDIO_LOGE("device dev is null");
		return;
	}

	ret = of_property_read_string(dev->of_node, "hisilicon,bus-sel", &str);
	if (ret == 0 && strncmp(str, "slimbus", 7) == 0) {
		priv->cdc_ctrl.bus_sel = BUSTYPE_SELECT_SLIMBUS;
		priv->reg_ops.read8 = slimbus_read_1byte;
		priv->reg_ops.read32 = slimbus_read_4byte;
		priv->reg_ops.write8 = slimbus_write_1byte;
		priv->reg_ops.write32 = slimbus_write_4byte;
	} else {
		priv->cdc_ctrl.bus_sel = BUSTYPE_SELECT_SSI;
		priv->reg_ops.read8 = ssi_reg_read8;
		priv->reg_ops.read32 = ssi_reg_read32;
		priv->reg_ops.write8 = ssi_reg_write8;
		priv->reg_ops.write32 = ssi_reg_write32;

		if (of_property_read_bool(dev->of_node, "pm_runtime_support"))
			priv->cdc_ctrl.pm_runtime_support = true;
	}
}

void hi_cdc_ioparam_read(struct platform_device *pdev, struct hi_cdc_ctrl_priv *priv)
{
	uint32_t slimbusclk_drv;
	uint32_t slimbusdata_drv;

	if (priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return;
	}

	priv->cdc_ctrl.slimbusclk_cdc_drv = SLIMBUS_CLK_DRV_FORCE;
	priv->cdc_ctrl.slimbusdata_cdc_drv = SLIMBUS_DATA_DRV_FORCE;

	if (pdev == NULL) {
		AUDIO_LOGE("platform device pedv is null");
		return;
	}

	if (of_property_read_u32(pdev->dev.of_node, "slimbusclk_io_driver",
		&slimbusclk_drv) == 0)
		priv->cdc_ctrl.slimbusclk_cdc_drv = slimbusclk_drv;

	if (of_property_read_u32(pdev->dev.of_node, "slimbusdata_io_driver",
		&slimbusdata_drv) == 0)
		priv->cdc_ctrl.slimbusdata_cdc_drv = slimbusdata_drv;
}

static int hi_cdc_get_regulator(struct device *dev, struct hi_cdc_ctrl_priv *priv)
{
	unsigned int supply_val;
	unsigned int set_val;
	bool flag = false;
	struct device_node *np = dev->of_node;
	int ret = 0;

	if (!of_property_read_u32(np, "codec_ldo8_set_voltage", &set_val)) {
		AUDIO_LOGI("ldo8 set voltage is %u", set_val);
		if (set_val == LDO8_ENABLE)
			flag = true;
	}

	if (of_property_read_u32(np, "hisilicon,ldo8_supply", &supply_val)) {
		AUDIO_LOGI("ldo8 is not support");
		return ret;
	}

	AUDIO_LOGI("ldo8 supply is %u", supply_val);

	if (supply_val != LDO8_ENABLE)
		return ret;

	priv->ldo8 = devm_regulator_get(dev, "codec_ldo8");
	if (IS_ERR(priv->ldo8)) {
		priv->ldo8 = NULL;
		AUDIO_LOGE("failed to get ldo8 regulator");
		return -EFAULT;
	}

	if (flag) {
		if (regulator_set_voltage(priv->ldo8, LDO8_VOLTAGE, LDO8_VOLTAGE)) {
			AUDIO_LOGE("failed to regulator set voltage");
			return -EFAULT;
		}
	}
	if (regulator_enable(priv->ldo8)) {
		AUDIO_LOGE("failed to enable ldo8 supply");
		return -EFAULT;
	}

	return ret;
}

static int irq_pinctrl_set(struct platform_device *pdev)
{
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);

	priv->cdc_ctrl.pctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (priv->cdc_ctrl.pctrl == NULL) {
		AUDIO_LOGE("irq pinctrl set default failed");
		return -EFAULT;
	}

	return 0;
}

static int chip_reset(struct platform_device *pdev)
{
	int ret;
	unsigned int gpio_irq;
	unsigned int gpio_reset;
	struct device *dev = &pdev->dev;
	struct device_node *node = pdev->dev.of_node;

	ret = of_property_read_u32(node, "gpio_irq", &gpio_irq);
	if (ret != 0) {
		AUDIO_LOGE("read irq gpio failed, errot: %d", ret);
		return ret;
	}

	ret = of_property_read_u32(node, "gpio_reset", &gpio_reset);
	if (ret != 0) {
		AUDIO_LOGE("read reset gpio failed, error: %d", ret);
		return ret;
	}

	AUDIO_LOGI("irq gpio: %u, reset gpio: %u", gpio_irq, gpio_reset);

	ret = devm_gpio_request(dev, gpio_irq, "gpio_irq");
	if (ret != 0) {
		AUDIO_LOGE("request irq gpio failed, error: %d", ret);
		return ret;
	}

	ret = gpio_direction_output(gpio_irq, GPIO_LOW);
	if (ret != 0) {
		AUDIO_LOGE("set irq gpio output failed, error: %d", ret);
		return ret;
	}

	ret = devm_gpio_request(dev, gpio_reset, "gpio_reset");
	if (ret != 0) {
		AUDIO_LOGE("request gpio reset failed, error: %d", ret);
		return ret;
	}

	ret = gpio_direction_output(gpio_reset, GPIO_LOW);
	if (ret != 0) {
		AUDIO_LOGE("reset gpio direction output failed, error: %d", ret);
		return ret;
	}

	mdelay(1);
	gpio_set_value(gpio_reset, GPIO_HIGH);
	devm_gpio_free(dev, gpio_reset);
	devm_gpio_free(dev, gpio_irq);

	AUDIO_LOGI("reset ok");

	return ret;
}

static int codec_reset(struct platform_device *pdev)
{
	int ret;
	struct device_node *node = pdev->dev.of_node;
	struct hi_cdc_ctrl_priv *priv = platform_get_drvdata(pdev);
	bool need_reset = of_property_read_bool(node, "need_reset_in_kernel");

	if (!need_reset)
		return 0;

	priv->cdc_ctrl.need_reset_in_kernel = true;

	ret = chip_reset(pdev);
	if (ret != 0) {
		AUDIO_LOGE("reset failed, error: %d", ret);
		return ret;
	}

	ret = irq_pinctrl_set(pdev);
	if (ret != 0) {
		AUDIO_LOGE("irq pinctrl set failed, error: %d", ret);
		return ret;
	}

	AUDIO_LOGI("reset ok");

	return 0;
}

static int get_resource(struct platform_device *pdev, struct hi_cdc_ctrl_priv *priv)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	(void)of_property_read_u32(np, "pmu_clkcodec_addr", &priv->pmu_codec_mclk_addr);

	ret = of_property_read_u32(np, "hisilicon,reg-8bit-begin-addr", &priv->reg_addr8_begin);
	if (ret != 0) {
		AUDIO_LOGE("read reg-8bit-begin-addr error");
		return ret;
	}

	ret = of_property_read_u32(np, "hisilicon,reg-8bit-end-addr", &priv->reg_addr8_end);
	if (ret != 0) {
		AUDIO_LOGE("read reg-8bit-end-addr error");
		return ret;
	}

	priv->supplies[CDC_SUP_MAIN] = devm_regulator_get(dev, "codec-main");
	if (IS_ERR(priv->supplies[CDC_SUP_MAIN]))
		priv->supplies[CDC_SUP_MAIN] = NULL;

	priv->supplies[CDC_SUP_ANLG] = devm_regulator_get(dev, "codec-anlg");
	if (IS_ERR(priv->supplies[CDC_SUP_ANLG]))
		priv->supplies[CDC_SUP_ANLG] = NULL;

	priv->cdc_mclk = devm_clk_get(dev, "clk_pmuaudioclk");
	if (IS_ERR(priv->cdc_mclk)) {
		ret = PTR_ERR(priv->cdc_mclk);
		AUDIO_LOGE("cdc mclk not found, error:%d", ret);
	}

	return ret;
}

static void enable_supply_main(struct hi_cdc_ctrl_priv *priv)
{
	if (priv->supplies[CDC_SUP_MAIN] != NULL) {
		if (regulator_enable(priv->supplies[CDC_SUP_MAIN]) != 0)
			AUDIO_LOGW("enable main supply failed");
	}
}

static void disable_supply_main(struct hi_cdc_ctrl_priv *priv)
{
	if (priv->supplies[CDC_SUP_MAIN] != NULL) {
		if (regulator_disable(priv->supplies[CDC_SUP_MAIN]) != 0)
			AUDIO_LOGW("disable main supply fail");
	}
}

static int enable_resource(struct hi_cdc_ctrl_priv *priv)
{
	int ret;

	enable_supply_main(priv);

	ret = clk_prepare_enable(priv->cdc_mclk);
	if (ret != 0) {
		AUDIO_LOGE("clk prepare enable failed");
		goto clk_enable_fail;
	}

	mdelay(1);
	return ret;

clk_enable_fail:
	disable_supply_main(priv);

	return ret;
}

static void disable_res(struct hi_cdc_ctrl_priv *priv)
{
	clk_disable_unprepare(priv->cdc_mclk);

	disable_supply_main(priv);
}

static int irq_request(struct platform_device * const pdev, struct hi_cdc_ctrl_priv *priv)
{
	int ret;
	int gpio;
	enum of_gpio_flags flags;
	struct device_node *np = pdev->dev.of_node;

	gpio = of_get_gpio_flags(np, 0, &flags);
	if (gpio < 0) {
		AUDIO_LOGE("get gpio flags failed");
		ret = gpio;
		return ret;
	}

	AUDIO_LOGI("irq gpio%d", gpio);

	if (!gpio_is_valid(gpio)) {
		AUDIO_LOGE("gpio%d is invalid", gpio);
		return -EINVAL;
	}

	ret = devm_gpio_request_one(&pdev->dev, (unsigned int)gpio, GPIOF_IN, "codec_interrupt");
	if (ret < 0) {
		AUDIO_LOGE("failed to request gpio%d", gpio);
		return ret;
	}

	priv->irq = gpio_to_irq((unsigned int)gpio);
	/* continue probe, just irq will request fail, but other function should work normally */
	if (priv->irq < 0)
		AUDIO_LOGE("gpio%d to irq%d fail", gpio, priv->irq);

	AUDIO_LOGI("irq is %d", priv->irq);

	return 0;
}

static void hi_cdc_pm_runtime_enable(struct platform_device *pdev, const struct hi_cdc_ctrl_priv *priv)
{
	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_use_autosuspend(&pdev->dev);
		pm_runtime_set_autosuspend_delay(&pdev->dev, 200); /* 200ms */
		pm_runtime_set_active(&pdev->dev);
		pm_runtime_enable(&pdev->dev);
	}
}

static void hi_cdc_pm_runtime_disable(struct platform_device *pdev, const struct hi_cdc_ctrl_priv *priv)
{
	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_resume(&pdev->dev);
		pm_runtime_disable(&pdev->dev);
		pm_runtime_set_suspended(&pdev->dev);
	}
}

static int hi_cdcctrl_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct hi_cdc_ctrl_priv *priv = NULL;

	AUDIO_LOGI("probe begin");
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		AUDIO_LOGE("malloc failed");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, priv);
	priv->cdc_ctrl.dev = dev;
	g_cdc_ctrl_priv = priv;

	hi_cdc_bus_type_select(priv, dev);

	hi_cdc_ioparam_read(pdev, priv);

	ret = get_resource(pdev, priv);
	if (ret != 0)
		goto res_err_exit;

	ret = enable_resource(priv);
	if (ret != 0)
		goto res_err_exit;

	ret = codec_reset(pdev);
	if (ret != 0)
		goto codec_reset_err_exit;

	ret = irq_request(pdev, priv);
	if (ret != 0)
		goto irq_request_err_exit;

	/* populate sub nodes */
	of_platform_populate(dev->of_node, of_codec_controller_child_match_tbl, NULL, dev);

	hi_cdc_pm_runtime_enable(pdev, priv);

	ret = hi_cdc_get_regulator(dev, priv);
	if (ret != 0)
		goto get_regulator_err_exit;

	mutex_init(&priv->io_mutex);

	AUDIO_LOGI("controller probe ok, slimbusclk drv: %d, slimbusdata drv: %d, pm runtime support: %d",
		priv->cdc_ctrl.slimbusclk_cdc_drv, priv->cdc_ctrl.slimbusdata_cdc_drv, priv->cdc_ctrl.pm_runtime_support);

	return 0;

get_regulator_err_exit:
	hi_cdc_pm_runtime_disable(pdev, priv);
codec_reset_err_exit:
irq_request_err_exit:
	disable_res(priv);
res_err_exit:
	AUDIO_LOGE("controller probe failed");

	return ret;
}

static int hi_cdcctrl_remove(struct platform_device *pdev)
{
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);

	mutex_destroy(&priv->io_mutex);

	hi_cdc_pm_runtime_disable(pdev, priv);

	disable_supply_main(priv);

	if (priv->ldo8 != NULL) {
		if (regulator_disable(priv->ldo8))
			AUDIO_LOGE("lodo8 regulator disable failed");
	}

	clk_disable_unprepare(priv->cdc_mclk);

	g_cdc_ctrl_priv = NULL;

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int hi_cdcctrl_suspend(struct device *device)
{
	int ret;
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);

	if (priv->cdc_ctrl.pm_runtime_support) {
		ret = pm_runtime_get_sync(priv->cdc_ctrl.dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
			return ret;
		}
	}

	AUDIO_LOGI("usage count: 0x%x status: 0x%x, disable depth: %u, clk: %d",
		atomic_read(&(device->power.usage_count)),
		device->power.runtime_status, device->power.disable_depth,
		clk_get_enable_count(priv->cdc_mclk));

	clk_disable_unprepare(priv->cdc_mclk);

	return 0;
}

static int hi_cdcctrl_resume(struct device *device)
{
	int ret;
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);

	ret = clk_prepare_enable(priv->cdc_mclk);
	if (ret != 0) {
		AUDIO_LOGE("clk prepare enable failed");
		return ret;
	}

	mdelay(1);

	if (priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(priv->cdc_ctrl.dev);
		pm_runtime_put_autosuspend(priv->cdc_ctrl.dev);

		pm_runtime_disable(priv->cdc_ctrl.dev);
		pm_runtime_set_active(priv->cdc_ctrl.dev);
		pm_runtime_enable(priv->cdc_ctrl.dev);
	}

	AUDIO_LOGI("usage count: 0x%x status: 0x%x disable depth: %u clk: %d",
		atomic_read(&(device->power.usage_count)),
		device->power.runtime_status, device->power.disable_depth,
		clk_get_enable_count(priv->cdc_mclk));

	return 0;
}
#endif

void hi_cdcctrl_pm_get(void)
{
#ifdef CONFIG_PM
	int ret;

	if (g_cdc_ctrl_priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return;
	}

	if (g_cdc_ctrl_priv->cdc_ctrl.pm_runtime_support) {
		ret = pm_runtime_get_sync(g_cdc_ctrl_priv->cdc_ctrl.dev);
		if (ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", ret);
			rdr_system_error(RDR_AUDIO_RUNTIME_SYNC_FAIL_MODID, 0, 0);
		}
	}
#endif
}

void hi_cdcctrl_pm_put(void)
{
#ifdef CONFIG_PM
	if (g_cdc_ctrl_priv == NULL) {
		AUDIO_LOGE("cdc ctrl priv is null");
		return;
	}

	if (g_cdc_ctrl_priv->cdc_ctrl.pm_runtime_support) {
		pm_runtime_mark_last_busy(g_cdc_ctrl_priv->cdc_ctrl.dev);
		pm_runtime_put_autosuspend(g_cdc_ctrl_priv->cdc_ctrl.dev);
	}
#endif
}

#ifdef CONFIG_PM
static int hi_cdcctrl_runtime_suspend(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);

	clk_disable_unprepare(priv->cdc_mclk);

	AUDIO_LOGI("usage count: 0x%x status: 0x%x disable depth: %u clk: %d",
		atomic_read(&(device->power.usage_count)),
		device->power.runtime_status, device->power.disable_depth,
		clk_get_enable_count(priv->cdc_mclk));

	return 0;
}

static int hi_cdcctrl_runtime_resume(struct device *device)
{
	struct platform_device *pdev = to_platform_device(device);
	struct hi_cdc_ctrl_priv *priv =
		(struct hi_cdc_ctrl_priv *)platform_get_drvdata(pdev);
	int ret;

	ret = clk_prepare_enable(priv->cdc_mclk);
	if (ret != 0)
		AUDIO_LOGE("clk prepare enable failed");

	mdelay(5);

	AUDIO_LOGI("usage count: 0x%x status: 0x%x disable depth: %u clk: %d",
		atomic_read(&(device->power.usage_count)),
		device->power.runtime_status, device->power.disable_depth,
		clk_get_enable_count(priv->cdc_mclk));

	return ret;
}
#endif

static const struct of_device_id of_hi_cdcctrl_match[] = {
	{ .compatible = "hisilicon,codec-controller", },
	{},
};

static const struct dev_pm_ops hi_cdcctrl_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hi_cdcctrl_suspend, hi_cdcctrl_resume)
	SET_RUNTIME_PM_OPS(hi_cdcctrl_runtime_suspend, hi_cdcctrl_runtime_resume, NULL)
};

static struct platform_driver hi_cdc_ctrl_driver = {
	.driver = {
		.name = "codec_controller",
		.owner = THIS_MODULE,
		.pm = &hi_cdcctrl_pm_ops,
		.of_match_table = of_hi_cdcctrl_match,
	},
	.probe = hi_cdcctrl_probe,
	.remove = hi_cdcctrl_remove,
};

static int __init hi_cdcctrl_init(void)
{
	return platform_driver_register(&hi_cdc_ctrl_driver);
}

static void __exit hi_cdcctrl_exit(void)
{
	platform_driver_unregister(&hi_cdc_ctrl_driver);
}
fs_initcall_sync(hi_cdcctrl_init);
module_exit(hi_cdcctrl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("codec controller");

