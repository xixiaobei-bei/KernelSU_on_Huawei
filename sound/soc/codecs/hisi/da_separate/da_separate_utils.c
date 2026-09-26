/*
 * da_separate_utils.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_utils.h"
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pm_runtime.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/clk.h>
#include "da_separate_type.h"

#include "linux/hisi/audio_log.h"

#define LOG_TAG "DA_separate_utils"

static void __iomem *reg_base_addr[REG_TYPE_CNT];

static const struct reg_page reg_page_array[] = {
	{ PAGE_SoCCODEC,  SoCCODEC_START,    SoCCODEC_END,    "PAGE_SoCCODEC" },
	{ PAGE_ASPCFG,    ASPCFG_START,      ASPCFG_END,      "PAGE_ASPCFG" },
	{ PAGE_AO_IOC,    AOIOC_START,       AOIOC_END,       "PAGE_AO_IOC" },
	{ PAGE_PMU_CODEC, PMUCODEC_START,    PMUCODEC_END,    "PAGE_PMU_CODEC" },
	{ PAGE_PMU_CTRL,  PMUCTRL_START,     PMUCTRL_END,     "PAGE_PMU_CTRL" },
	{ PAGE_PMU_HKADC, PMUHKADC_START,    PMUHKADC_END,    "PAGE_PMU_HKADC" },
	{ PAGE_VIRCODEC,  VIRCODEC_START,    VIRCODEC_END,    "PAGE_VIRCODEC" },
};

#define DBG_SOC_CODEC_START (ASP_CODEC_REG_BASE + SoCCODEC_START)
#define DBG_SOC_CODEC_END   (ASP_CODEC_REG_BASE + SoCCODEC_END)
#define DBG_PMU_CODEC_START (PAGE_PMU_CODEC + PMUCODEC_START)
#define DBG_PMU_CODEC_END   (PAGE_PMU_CODEC + PMUCODEC_END)
#define DBG_PMU_HKADC_START (PAGE_PMU_HKADC + PMUHKADC_START)
#define DBG_PMU_HKADC_END   (PAGE_PMU_HKADC + PMUHKADC_END)
#define DBG_PMU_CTRL_CLASSD_START (PAGE_PMU_CTRL_REGB + PMUCLASSD_START)
#define DBG_PMU_CTRL_CLASSD_END   (PAGE_PMU_CTRL_REGB + PMUCLASSD_END)

#define SOC_CODEC_REG_SIZE 4
#define PMU_CODEC_SIZE 1
#define PMU_HKADC_SIZE 1
#define PMU_CLASSD_SIZE 1

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
static struct hicodec_dump_reg_entry dump_table[] = {
	{ "SOC CODEC",  DBG_SOC_CODEC_START, DBG_SOC_CODEC_END,             SOC_CODEC_REG_SIZE },
	{ "PMU CODEC",  DBG_PMU_CODEC_START, DBG_PMU_CODEC_END,             PMU_CODEC_SIZE },
	{ "PMU HKADC",  DBG_PMU_HKADC_START, DBG_PMU_HKADC_END,             PMU_HKADC_SIZE },
	{ "PMU CLASSD", DBG_PMU_CTRL_CLASSD_START, DBG_PMU_CTRL_CLASSD_END, PMU_CLASSD_SIZE },
};

static struct hicodec_dump_reg_info dump_info = {
	.entry = dump_table,
	.count = ARRAY_SIZE(dump_table),
};

struct hicodec_dump_reg_info *da_separate_reg_get_dump_info(void)
{
	return &dump_info;
}
#endif

int da_separate_reg_addr_map(struct platform_device *pdev)
{
	struct resource *res = NULL;
	unsigned int i;

	IN_FUNCTION;

	if (pdev == NULL) {
		AUDIO_LOGE("pdev paramrter is null");
		return -EINVAL;
	}

	for (i = 0; i < REG_TYPE_CNT; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (res == NULL) {
			AUDIO_LOGE("platform_get_resource %d err", i);
			return -ENOENT;
		}

		reg_base_addr[i] = (char * __force)(ioremap(res->start, resource_size(res))); //lint !e446
		if (!reg_base_addr[i]) {
			AUDIO_LOGE("cannot map register memory");
			return -ENOMEM;
		}
	}

	OUT_FUNCTION;

	return 0;
}

void da_separate_reg_addr_unmap(void)
{
	unsigned int i;

	IN_FUNCTION;
	for (i = 0; i < REG_TYPE_CNT; i++) {
		if (reg_base_addr[i]) {
			iounmap(reg_base_addr[i]);
			reg_base_addr[i] = NULL;
		}
	}
	OUT_FUNCTION;
}

/* get a unused register to lock common resource */
#define RESERVED_LOCK_OFFSET 0x90
#define INTER_LOCK_TIMEOUT_CNT 12

static void intercore_try_lock()
{
	uint32_t time_cnt = INTER_LOCK_TIMEOUT_CNT;
	while(time_cnt) {
		if (readl(reg_base_addr[ASPCFG] + RESERVED_LOCK_OFFSET) == 0) {
			break;
		}
		udelay(250);
		time_cnt--;
	}

	if (time_cnt == 0) {
		AUDIO_LOGE("try lock timeout");
		return;
	}

	writel(1, reg_base_addr[ASPCFG] + RESERVED_LOCK_OFFSET);
}

static void intercore_unlock()
{
	writel(0, reg_base_addr[ASPCFG] + RESERVED_LOCK_OFFSET);
}

static bool _reg_value_valid(const struct platform_data *priv,
	unsigned int reg_type, unsigned int reg_value)
{
	bool is_valid = false;
	unsigned int reg_page_array_size = ARRAY_SIZE(reg_page_array);
	unsigned int i = 0;

	for (i = 0; i < reg_page_array_size; i++) {
		if ((reg_type == reg_page_array[i].page_tag) &&
			((reg_value >= reg_page_array[i].page_reg_begin) &&
			(reg_value <= reg_page_array[i].page_reg_end))) {
			is_valid = true;
			break;
		}
	}

	if (is_valid) {
		if ((reg_type == PAGE_ASPCFG || reg_type == PAGE_SoCCODEC)
			&& priv->pm.asp_pd) {
			AUDIO_LOGE("asp power down");
			is_valid = false;
		}
	} else {
		AUDIO_LOGE("reg_type: %d, reg_value: 0x%x is invalid",
			reg_type, reg_value);
	}

	return is_valid;
}

static void runtime_info_print(const struct platform_data *priv)
{
	AUDIO_LOGD("get suspend usage_count: 0x%x child_count: 0x%x status: 0x%x disable_depth: %d asp_subsys_clk: %d, asp_codec_clk: %d",
			atomic_read(&(dev->power.usage_count)),
			atomic_read(&(dev->power.child_count)),
			priv->codec->dev->power.runtime_status,
			priv->codec->dev->power.disable_depth,
			clk_get_enable_count(priv->resource.asp_subsys_clk),
			clk_get_enable_count(priv->resource.asp_codec_clk));
}

static int runtime_get_sync(const struct platform_data *priv, unsigned int reg_type)
{
	bool runtime_flag = false;
	struct device *dev = priv->codec->dev;
	int pm_ret = 0;

	IN_FUNCTION;

	runtime_flag = priv->pm.pm_runtime_support &&
		(reg_type == PAGE_SoCCODEC || reg_type == PAGE_ASPCFG);

	if (runtime_flag) {
		pm_ret = pm_runtime_get_sync(dev);
		AUDIO_LOGD("get pm resume  ret: %d", pm_ret);
		if (pm_ret < 0) {
			AUDIO_LOGE("pm resume error, ret: %d", pm_ret);
			return pm_ret;
		}

		runtime_info_print(priv);
	}

	OUT_FUNCTION;

	return pm_ret;
}

static void runtime_put_sync(struct platform_data *priv, unsigned int reg_type)
{
	bool runtime_flag = false;
	struct device *dev = priv->codec->dev;

	IN_FUNCTION;

	runtime_flag = priv->pm.pm_runtime_support &&
		(reg_type == PAGE_SoCCODEC || reg_type == PAGE_ASPCFG);

	if (runtime_flag) {
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);

		runtime_info_print(priv);
	}

	OUT_FUNCTION;
}

static unsigned int read_soc_codec_reg(unsigned int reg_value)
{
	unsigned int ret;

	if (reg_value == (CODEC_CLK_EN0_REG & PAGE_VALUE_MASK) ||
		reg_value == (CODEC_CLK_EN1_REG & PAGE_VALUE_MASK)) {
		intercore_try_lock();
		ret = readl(reg_base_addr[SOCCODEC] + reg_value);
		intercore_unlock();
	} else {
		ret = readl(reg_base_addr[SOCCODEC] + reg_value);
	}

	return ret;
}

static unsigned int page_reg_read(struct platform_data *priv, unsigned int reg)
{
	volatile unsigned int ret;
	unsigned int reg_type;
	unsigned int reg_value;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	switch (reg_type) {
	case PAGE_SoCCODEC:
		ret = read_soc_codec_reg(reg_value);
		AUDIO_LOGD("PAGE_SoCCODEC: offset = 0x%x, value = 0x%x", reg_value, ret);
		break;
	case PAGE_PMU_CTRL:
	case PAGE_PMU_CODEC:
	case PAGE_PMU_HKADC:
		if (priv->board_type == FPGA_BOARD && priv->pmu_invalid) {
			AUDIO_LOGD("pmu invalid, read offset = 0x%x", reg_value);
			ret = 0;
			break;
		}
		ret = pmic_read_reg(reg);
		AUDIO_LOGD("PAGE_PMUCODEC: offset = 0x%x, value = 0x%x", reg, ret);
		break;
	case PAGE_ASPCFG:
		ret = readl(reg_base_addr[ASPCFG] + reg_value);
		AUDIO_LOGD("PAGE_ASPCFG: offset = 0x%x, value = 0x%x", reg_value, ret);
		break;
	case PAGE_AO_IOC:
		ret = readl(reg_base_addr[AOIOC] + reg_value);
		AUDIO_LOGD("PAGE_AO_IOC: offset = 0x%x, value = 0x%x", reg_value, ret);
		break;
	case PAGE_VIRCODEC:
		ret = priv->virtual_reg[reg_value];
		AUDIO_LOGD("PAGE_VIRCODEC: offset = 0x%x, ret = 0x%x", reg_value, ret);
		break;
	default:
		AUDIO_LOGE("reg=0x%pK", (void *)(unsigned long)reg);
		ret = INVALID_REG_VALUE;
		break;
	}

	return ret;
}

static unsigned int reg_read(struct platform_data *priv, unsigned int reg)
{
	volatile unsigned int ret;
	unsigned int reg_type;
	unsigned int reg_value;
	unsigned long flags = 0;
	int pm_ret;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	if (!_reg_value_valid(priv, reg_type, reg_value)) {
		AUDIO_LOGE("invalid reg:0x%pK", (void *)(unsigned long)reg);
		return INVALID_REG_VALUE;
	}

	pm_ret = runtime_get_sync(priv, reg_type);
	if (pm_ret < 0) {
		AUDIO_LOGE("runtime resume fail");
		return INVALID_REG_VALUE;
	}

	spin_lock_irqsave(&priv->reg_access_lock, flags);

	ret = page_reg_read(priv, reg);

	spin_unlock_irqrestore(&priv->reg_access_lock, flags);

	runtime_put_sync(priv, reg_type);

	return ret;
}

static void write_soc_codec_reg(unsigned int reg_value, unsigned int value)
{
	if (reg_value == (CODEC_CLK_EN0_REG & PAGE_VALUE_MASK) ||
		reg_value == (CODEC_CLK_EN1_REG & PAGE_VALUE_MASK)) {
		intercore_try_lock();
		writel(value, reg_base_addr[SOCCODEC] + reg_value);
		intercore_unlock();
	} else {
		writel(value, reg_base_addr[SOCCODEC] + reg_value);
	}
}

static void page_reg_write(struct platform_data *priv, unsigned int reg,
	unsigned int value)
{
	unsigned int reg_type;
	unsigned int reg_value;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;
	switch (reg_type) {
	case PAGE_SoCCODEC:
		AUDIO_LOGD("PAGE_SoCCODEC: offset = 0x%x, value = 0x%x", reg_value, value);
		write_soc_codec_reg(reg_value, value);
		break;
	case PAGE_PMU_CTRL:
	case PAGE_PMU_CODEC:
	case PAGE_PMU_HKADC:
		if (priv->board_type == FPGA_BOARD && priv->pmu_invalid) {
			AUDIO_LOGD("pmu invalid, write offset = 0x%x", reg_value);
			break;
		}
		AUDIO_LOGD("PAGE_PMUCODEC: offset = 0x%x, value = 0x%x", reg, value);
		pmic_write_reg(reg, value);
		break;
	case PAGE_ASPCFG:
		AUDIO_LOGD("PAGE_ASPCFG: offset = 0x%x, value = 0x%x", reg_value, value);
		writel(value, reg_base_addr[ASPCFG] + reg_value);
		break;
	case PAGE_AO_IOC:
		AUDIO_LOGD("PAGE_AO_IOC: offset = 0x%x, value = 0x%x", reg_value, value);
		writel(value, reg_base_addr[AOIOC] + reg_value);
		break;
	case PAGE_VIRCODEC:
		AUDIO_LOGD("PAGE_VIRCODEC: offset = 0x%x, value = 0x%x", reg_value, value);
		priv->virtual_reg[reg_value] = value;
		break;
	default:
		AUDIO_LOGE("reg=0x%pK, value=0x%x", (void *)(unsigned long)reg, value);
		break;
	}

}

static void reg_write(struct platform_data *priv, unsigned int reg,
	unsigned int value)
{
	unsigned int reg_type;
	unsigned int reg_value;
	unsigned long flags = 0;
	int ret;

	reg_type  = reg & PAGE_TYPE_MASK;
	reg_value = reg & PAGE_VALUE_MASK;

	if (!_reg_value_valid(priv, reg_type, reg_value)) {
		AUDIO_LOGE("invalid reg: 0x%pK, value: %d", (void *)(unsigned long)reg, value);
		return;
	}

	ret = runtime_get_sync(priv, reg_type);
	if (ret < 0) {
		AUDIO_LOGE("runtime resume fail");
		return;
	}

	spin_lock_irqsave(&priv->reg_access_lock, flags);

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_reg_rw_cache(reg, value, HICODEC_DEBUG_FLAG_WRITE);
#endif

	page_reg_write(priv, reg, value);

	spin_unlock_irqrestore(&priv->reg_access_lock, flags);

	runtime_put_sync(priv, reg_type);
}

static struct platform_data *get_private_data(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return NULL;
	}

	return snd_soc_component_get_drvdata(codec);
}

unsigned int da_separate_reg_read(struct snd_soc_component *codec, unsigned int reg)
{
	volatile unsigned int ret;
	struct platform_data *priv = get_private_data(codec);
	if (priv == NULL) {
		AUDIO_LOGE("private data is null");
		return INVALID_REG_VALUE;
	}

	ret = reg_read(priv, reg);
	if (ret == INVALID_REG_VALUE)
		AUDIO_LOGE("reg 0x%pK read value 0x%x is invalid",
			(void *)(unsigned long)reg, ret);

	return ret;
}

int da_separate_reg_write(struct snd_soc_component *codec, unsigned int reg,
	unsigned int value)
{
	int ret = 0;
	struct platform_data *priv = get_private_data(codec);
	if (priv == NULL) {
		AUDIO_LOGE("private data is null");
		return -EINVAL;
	}

	reg_write(priv, reg, value);

	return ret;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
unsigned int da_separate_reg_read_by_codec(struct snd_soc_codec *codec, unsigned int reg)
{
	return da_separate_reg_read(&codec->component, reg);
}

int da_separate_reg_write_by_codec(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	return da_separate_reg_write(&codec->component, reg, value);
}
#endif

void da_separate_reg_seq_write_array(struct snd_soc_component *codec,
	const struct reg_seq_config *reg_seq_array, size_t len)
{
	unsigned int i;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	if (reg_seq_array == NULL) {
		AUDIO_LOGE("reg array is null");
		return;
	}

	for (i = 0; i < len; i++) {
		if (reg_seq_array[i].cfg.update_bits)
			snd_soc_component_update_bits(codec, reg_seq_array[i].cfg.addr,
				reg_seq_array[i].cfg.mask, reg_seq_array[i].cfg.val);
		else
			 snd_soc_component_write(codec, reg_seq_array[i].cfg.addr,
				reg_seq_array[i].cfg.val);

		switch (reg_seq_array[i].type) {
		case RANGE_SLEEP:
			usleep_range(reg_seq_array[i].us,
				reg_seq_array[i].us + reg_seq_array[i].us / 10);
			break;
		case MSLEEP:
			msleep(reg_seq_array[i].us / 1000);
			break;
		case MDELAY:
			mdelay((unsigned long)(reg_seq_array[i].us / 1000));
			break;
		default:
			break;
		}
	}
}

int da_separate_asp_regulator_enable(struct snd_soc_component *codec)
{
	int ret;
	struct platform_data *priv = get_private_data(codec);
	if (priv == NULL) {
		AUDIO_LOGE("private data is null");
		return -EINVAL;
	}

	IN_FUNCTION;
	if (priv->pm.regulator_pu_count > 0) {
		priv->pm.regulator_pu_count++;
		AUDIO_LOGI("asp regulator is already enable, regulator_pu_count %d",
			priv->pm.regulator_pu_count);
		return 0;
	}

	ret = regulator_enable(priv->resource.asp_regulator);
	if (ret) {
		AUDIO_LOGE("couldn't enable asp regulator, ret = %d", ret);
		return ret; /* lint !e527 */
	}

	priv->pm.regulator_pu_count++;

	AUDIO_LOGI("asp regulator enable done, regulator_pu_count %d",
		priv->pm.regulator_pu_count);

	OUT_FUNCTION;

	return ret;
}

int da_separate_asp_clk_enable(struct snd_soc_component *codec)
{
	int ret;
	struct platform_data *priv = get_private_data(codec);
	if (priv == NULL) {
		AUDIO_LOGE("private data is null");
		return -EINVAL;
	}

	IN_FUNCTION;
	if (priv->pm.clk_pu_count > 0) {
		priv->pm.clk_pu_count++;
		AUDIO_LOGI("asp_subsys_clk is already enable, clk_pu_count %d",
			priv->pm.clk_pu_count);
		return 0;
	}

	ret = clk_prepare_enable(priv->resource.asp_subsys_clk);
	if (ret) {
		AUDIO_LOGE("asp_subsys_clk :clk prepare enable failed, ret = %d", ret);
		return ret;/*lint !e527*/
	}

	ret = clk_prepare_enable(priv->resource.asp_codec_clk);
	if (ret) {
		AUDIO_LOGE("clk en fail, ret = %d", ret);
		clk_disable_unprepare(priv->resource.asp_subsys_clk);
		return ret;/*lint !e527*/
	}

	priv->pm.clk_pu_count++;
	AUDIO_LOGI("asp clk enable ok, clk_pu_count: %d", priv->pm.clk_pu_count);

	OUT_FUNCTION;

	return ret;
}

int da_separate_asp_resource_enable(struct snd_soc_component *codec)
{
	int ret;

	IN_FUNCTION;

	ret = da_separate_asp_regulator_enable(codec);
	if (ret) {
		AUDIO_LOGE("couldn't enable asp regulator, ret = %d", ret);
		goto end;
	}

	ret = da_separate_asp_clk_enable(codec);
	if (ret) {
		AUDIO_LOGE("asp clk enable failed, ret = %d", ret);
		goto clk_enable_err;
	}

	AUDIO_LOGI("asp resource enable ok");

	goto end;

clk_enable_err:
	da_separate_asp_regulator_disable(codec);

end:
	OUT_FUNCTION;

	return ret;
}

void da_separate_asp_regulator_disable(struct snd_soc_component *codec)
{
	struct platform_data *priv = get_private_data(codec);
	if (priv == NULL) {
		AUDIO_LOGE("private data is null");
		return;
	}

	IN_FUNCTION;
	if (priv->pm.regulator_pu_count > 1) {
		priv->pm.regulator_pu_count--;
		AUDIO_LOGI("asp regulator others in use, regulator_pu_count %d",
			priv->pm.regulator_pu_count);
		return;
	}

	if (!IS_ERR(priv->resource.asp_regulator)) {
		if (regulator_disable(priv->resource.asp_regulator))
			AUDIO_LOGE("couldn't disable asp regulator");

		AUDIO_LOGI("asp_regulator disable ok");
	}

	priv->pm.regulator_pu_count--;

	AUDIO_LOGI("asp regulator disable done, regulator_pu_count %d",
		priv->pm.regulator_pu_count);

	OUT_FUNCTION;
}

void da_separate_asp_clk_disable(struct snd_soc_component *codec)
{
	struct platform_data *priv = get_private_data(codec);
	if (priv == NULL) {
		AUDIO_LOGE("private data is null");
		return;
	}

	IN_FUNCTION;
	if (priv->pm.clk_pu_count > 1) {
		priv->pm.clk_pu_count--;
		AUDIO_LOGI("asp_subsys_clk others in use, clk_pu_count %d",
			priv->pm.clk_pu_count);
		return;
	}

	if (!IS_ERR_OR_NULL(priv->resource.asp_codec_clk)) {
		clk_disable_unprepare(priv->resource.asp_codec_clk);
		AUDIO_LOGI("asp clk disable ok");
	}

	if (!IS_ERR_OR_NULL(priv->resource.asp_subsys_clk)) {
		clk_disable_unprepare(priv->resource.asp_subsys_clk);
		AUDIO_LOGI("asp_subsys_clk disable ok");
	}

	priv->pm.clk_pu_count--;
	AUDIO_LOGI("asp clk disable ok, clk_pu_count: %d", priv->pm.clk_pu_count);

	OUT_FUNCTION;
}

void da_separate_asp_resource_disable(struct snd_soc_component *codec)
{
	IN_FUNCTION;

	da_separate_asp_regulator_disable(codec);
	da_separate_asp_clk_disable(codec);

	OUT_FUNCTION;
}

int da_separate_add_codec_resource(struct snd_soc_component *codec, const struct snd_codec_resource *resource)
{
	int ret;
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	if (resource == NULL) {
		AUDIO_LOGE("resource is null");
		return -EINVAL;
	}

	ret = snd_soc_add_component_controls(codec, resource->controls, resource->num_controls);
	if (ret != 0) {
		AUDIO_LOGE("add kcontrols failed, ret: %d", ret);
		return ret;
	}

	dapm = snd_soc_component_get_dapm(codec);
	ret = snd_soc_dapm_new_controls(dapm, resource->widgets, resource->num_widgets);
	if (ret != 0) {
		AUDIO_LOGE("add widgets failed, ret: %d", ret);
		return ret;
	}

	ret = snd_soc_dapm_add_routes(dapm, resource->routes, resource->num_routes);
	if (ret != 0) {
		AUDIO_LOGE("add routes failed, ret: %d", ret);
		return ret;
	}

	return ret;
}

