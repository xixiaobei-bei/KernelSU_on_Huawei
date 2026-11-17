/*
 * venc_regulator.h
 *
 * This is venc drv.
 *
 * Copyright (c) 2009-2020 Huawei Technologies CO., Ltd.
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

#include "venc_regulator.h"
#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/hisi-iommu.h>
#include "drv_venc_dpm.h"
#include "hal_venc.h"

uint64_t g_smmu_page_base_addr;
struct iommu_domain *g_hisi_mmu_domain = HI_NULL;
struct venc_config g_venc_config;
static struct venc_regulator g_venc_regulator;
static struct venc_clock g_venc_clock_manager[MAX_SUPPORT_CORE_NUM];

#ifdef VENC_QOS_CFG
#define VENC_QOS_MODE                0xE894000C
#define VENC_QOS_BANDWIDTH           0xE8940010
#define VENC_QOS_SATURATION          0xE8940014
#define VENC_QOS_EXTCONTROL          0xE8940018

static uint32_t g_venc_qos_mode        = 0x1;
static uint32_t g_venc_qos_bandwidth   = 0x1000;
static uint32_t g_venc_qos_saturation  = 0x20;
static uint32_t g_venc_qos_extcontrol  = 0x1;
#endif

#ifdef CONFIG_ES_VENC_LOW_FREQ
static uint32_t g_venc_low_freq        = 480000000;
#endif
static bool g_need_transient_freq_rate;
static bool g_support_power_control_per_frame;

static void get_transient_freq_cfg(struct device_node *np)
{
	int32_t ret = of_property_read_u32(np, "transit_clk_rate", &g_venc_config.venc_conf_com.transit_clk_rate);
	if (ret) {
		HI_INFO_VENC("read property of transit clk fail set default");
		g_venc_config.venc_conf_com.transit_clk_rate = g_venc_config.venc_conf_com.clk_rate[0];
		g_need_transient_freq_rate = false;
	} else {
		g_need_transient_freq_rate = true;
	}

	return;
}

#ifdef VENC_DPM_ENABLE
static int32_t get_dpm_reg_address_range_cfg(struct device_node *np)
{
	int32_t ret;
	uint32_t i;

	for (i = 0; i < MAX_INNER_MODULE; i++) {
		ret = of_property_read_u32_index(np, "reg_address", i,
				&g_venc_config.venc_conf_dpm[i].reg_base_addr);
		if (ret) {
			HI_FATAL_VENC("get dpm reg %d failed, ret is %d", i, ret);
			return HI_FAILURE;
		}

		ret = of_property_read_u32(np, "reg_range", &g_venc_config.venc_conf_dpm[i].reg_range);
		if (ret) {
			HI_FATAL_VENC("get dpm reg %d range failed, ret is %d", i, ret);
			return HI_FAILURE;
		}
	}
	return HI_SUCCESS;
}
#endif

static int32_t get_core_cfg(struct device_node *np)
{
	int32_t ret;
	int32_t single_core_mode_value;

	ret = of_property_read_u32(np, "single_core_mode", &single_core_mode_value);
	if (ret) {
		HI_INFO_VENC("can not get singel core mode value, default use false");
		g_venc_config.venc_conf_com.is_single_core = false;
	} else {
		g_venc_config.venc_conf_com.is_single_core = (bool)single_core_mode_value;
	}

	ret = of_property_read_u32(np, "valid_core_id", &g_venc_config.venc_conf_com.valid_core_id);
	if (ret) {
		HI_INFO_VENC("can not get valid core id, default use core 0");
		g_venc_config.venc_conf_com.valid_core_id = VENC_CORE_0;
	} else if (g_venc_config.venc_conf_com.valid_core_id >= MAX_SUPPORT_CORE_NUM ||
		g_venc_config.venc_conf_com.valid_core_id < VENC_CORE_0) {
		HI_FATAL_VENC("get valid core id [%d] is invalid", g_venc_config.venc_conf_com.valid_core_id);
		return HI_FAILURE;
	}

	ret = of_property_read_u32(np, "core_num", &g_venc_config.venc_conf_com.core_num);
	if (ret) {
		HI_INFO_VENC("read property of core num fail set default");
		g_venc_config.venc_conf_com.core_num = 1;
	} else if ((g_venc_config.venc_conf_com.core_num > MAX_SUPPORT_CORE_NUM) ||
		(g_venc_config.venc_conf_com.core_num <= 0)) {
		HI_FATAL_VENC("read property of core num[%d] invalid", g_venc_config.venc_conf_com.core_num);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

int32_t get_dts_config_info(struct platform_device *pdev)
{
	int32_t ret;
	uint32_t i;
	char tmp_name[MAX_NAME_LEN] = {0};
	struct resource *res = NULL;
	struct device_node *np = NULL;
	struct device *dev = &pdev->dev;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	(void)memset_s(&g_venc_config, sizeof(g_venc_config), 0, sizeof(g_venc_config));
	(void)memset_s(g_venc_clock_manager, sizeof(g_venc_clock_manager), 0, sizeof(g_venc_clock_manager));

	if (!dev) {
		HI_FATAL_VENC("invalid argument, dev is NULL");
		return HI_FAILURE;
	}

	np = dev->of_node;
	if (!np) {
		HI_FATAL_VENC("invalid argument np is NULL");
		return HI_FAILURE;
	}

	/* 0 get venc core cfg */
	ret = get_core_cfg(np);
	if (ret) {
		HI_FATAL_VENC("get core num cfg failed");
		return HI_FAILURE;
	}

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		/* 1 read IRQ num from dts */
		venc->ctx[i].irq_num_normal = irq_of_parse_and_map(np, 3 * i);

		if (venc->ctx[i].irq_num_normal == 0) {
			HI_FATAL_VENC("parse and map irq VeduIrqNumNorm failed");
			return HI_FAILURE;
		}

		venc->ctx[i].irq_num_protect = irq_of_parse_and_map(np, 1 + 3 * i);

		if (venc->ctx[i].irq_num_protect == 0) {
			HI_FATAL_VENC("parse and map irq VeduIrqNumProt failed");
			return HI_FAILURE;
		}

		venc->ctx[i].irq_num_safe = irq_of_parse_and_map(np, 2 + 3 * i);

		if (venc->ctx[i].irq_num_safe == 0) {
			HI_FATAL_VENC("parse and map irq VeduIrqNumSafe failed");
			return HI_FAILURE;
		}

		/* 2 read venc register start address, range */
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (IS_ERR_OR_NULL(res)) {
			HI_FATAL_VENC("failed to get instruction resource!");
			return HI_FAILURE;
		}

		g_venc_config.venc_conf_priv[i].reg_base_addr = res->start; /*lint !e712*/
		g_venc_config.venc_conf_priv[i].reg_range    = resource_size(res); /*lint !e712*/

		/* 3 read venc clk rate [low, high], venc clock */
		if (i == 0) {
			ret = strcpy_s(tmp_name, MAX_NAME_LEN, VENC_CLOCK_NAME);
			if (ret) {
				HI_FATAL_VENC("call strcpy_s failed");
				return HI_FAILURE;
			}
		} else {
			ret = sprintf_s(tmp_name, sizeof(tmp_name), "%s%d", VENC_CLOCK_NAME, i);
			if (ret < 0) {
				HI_FATAL_VENC("call sprintf_s failed");
				return HI_FAILURE;
			}
		}

		g_venc_clock_manager[i].venc_clk  = devm_clk_get(dev, tmp_name);

		if (IS_ERR_OR_NULL(g_venc_clock_manager[i].venc_clk)) {
			HI_FATAL_VENC("can not get core_id %d clock", i);
			return HI_FAILURE;
		}
	}

	for (i = 0; i < VENC_CLK_BUTT; i++) {
		/* the clk rate is from high to low in dts */
		ret = of_property_read_u32_index(np, VENC_CLK_RATE, VENC_CLK_BUTT - i - 1,
				&g_venc_config.venc_conf_com.clk_rate[i]);
		if (ret) {
			HI_FATAL_VENC("get venc rate type %d failed, ret is %d", i, ret);
			return HI_FAILURE;
		}

		HI_INFO_VENC("venc clock type %d: clock rate is %d",
				i, g_venc_config.venc_conf_com.clk_rate[i]);
	}

#ifdef VENC_DPM_ENABLE
	/* get reg address used by dpm in dts */
	ret = get_dpm_reg_address_range_cfg(np);
	if (ret) {
		HI_FATAL_VENC("get dpm reg address range cfg failed");
		return HI_FAILURE;
	}
#endif
	ret = of_property_read_u32(np, "power_off_clk_rate", &g_venc_config.venc_conf_com.power_off_clk_rate);
	if (ret) {
		HI_INFO_VENC("read property of power off clk fail set default");
		g_venc_config.venc_conf_com.power_off_clk_rate = g_venc_config.venc_conf_com.clk_rate[0];
	}
	get_transient_freq_cfg(np);
#ifdef CONFIG_ES_VENC_LOW_FREQ
	g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_HIGH] = g_venc_low_freq;
#endif

	/* 4 fpga platform */
	ret = of_property_read_u32(np, "venc_fpga", &g_venc_config.venc_conf_com.fpga_flag);
	if (ret)
		HI_INFO_VENC("can not get the venc fpga flag, maybe not fpga");

	/* 5 get venc qos mode */
	ret = of_property_read_u32(np, "venc_qos_mode", &g_venc_config.venc_conf_com.qos_mode);
	if (ret)
		HI_ERR_VENC("can not get venc qos mode, use default value %d",
				g_venc_config.venc_conf_com.qos_mode);

	ret = of_property_read_u32(np, "support_power_control_per_frame", (uint32_t *)&g_support_power_control_per_frame);
	if (ret) {
		g_support_power_control_per_frame = false;
		HI_INFO_VENC("can not get venc power control flag, use default value 0");
	}
	HI_INFO_VENC("venc power control flag %d", g_support_power_control_per_frame);

	g_smmu_page_base_addr = (uint64_t)(hisi_domain_get_ttbr(&pdev->dev));
#ifndef SMMU_V3
	if (g_smmu_page_base_addr == 0) {
		HI_ERR_VENC("get mmu addr failed");
		return HI_FAILURE;
	}
#endif
	return HI_SUCCESS;
}

int32_t get_regulator_info(struct platform_device *pdev)
{
	uint32_t i;
	int32_t ret;
	char tmp_name[MAX_NAME_LEN] = {0};

	(void)memset_s(&g_venc_regulator, sizeof(g_venc_regulator), 0, sizeof(g_venc_regulator));

	g_venc_regulator.media_regulator = devm_regulator_get(&pdev->dev, MEDIA_REGULATOR_NAME);

	if (IS_ERR_OR_NULL(g_venc_regulator.media_regulator)) {
		HI_FATAL_VENC("get media regulator failed, error no is %ld", PTR_ERR(g_venc_regulator.media_regulator));
		g_venc_regulator.media_regulator = HI_NULL;
		return HI_FAILURE;
	}

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (i == 0) {
			ret = strcpy_s(tmp_name, MAX_NAME_LEN, VENC_REGULATOR_NAME);
			if (ret) {
				HI_FATAL_VENC("call strcpy_s failed!");
				return HI_FAILURE;
			}
		} else {
			ret = sprintf_s(tmp_name, sizeof(tmp_name), "%s%d", VENC_REGULATOR_NAME, i);
			if (ret < 0) {
				HI_FATAL_VENC("call sprintf_s failed! ");
				return HI_FAILURE;
			}
		}

		g_venc_regulator.venc_regulator[i] = devm_regulator_get(&pdev->dev, tmp_name);

		if (IS_ERR_OR_NULL(g_venc_regulator.venc_regulator[i])) {
			HI_FATAL_VENC("get regulator failed, error no is %ld", PTR_ERR(g_venc_regulator.venc_regulator[i]));
			g_venc_regulator.venc_regulator[i] = HI_NULL;
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

#ifdef VENC_QOS_CFG
static int32_t config_qos(void)
{
	uint32_t *qos_addr = HI_NULL;

	qos_addr = (uint32_t *)ioremap(VENC_QOS_MODE, 4);
	if (!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_MODE reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_mode, qos_addr);
	iounmap(qos_addr);

	qos_addr = (uint32_t *)ioremap(VENC_QOS_BANDWIDTH, 4);
	if (!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_BANDWIDTH reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_bandwidth, qos_addr);
	iounmap(qos_addr);

	qos_addr = (uint32_t *)ioremap(VENC_QOS_SATURATION, 4);
	if (!qos_addr) {
		HI_FATAL_VENC("ioremap Venc_QOS_SATURATION reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_saturation, qos_addr);
	iounmap(qos_addr);

	qos_addr = (uint32_t *)ioremap(VENC_QOS_EXTCONTROL, 4);
	if (!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_EXTCONTROL reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_extcontrol, qos_addr);
	iounmap(qos_addr);

	return HI_SUCCESS;
}
#endif

static bool is_hardware_busy(void)
{
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status == VENC_BUSY)
			return true;
	}

	return false;
}

static int32_t wait_core_idle(uint32_t core_id)
{
	uint32_t ret;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	/*lint -e578 -e666*/
	ret = OSAL_WAIT_EVENT_TIMEOUT(&venc->event,
		venc->ctx[core_id].status != VENC_BUSY,
		WAIT_CORE_IDLE_TIMEOUT_MS);
	/*lint +e578 +e666*/
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("wait core idle timeout");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

static int32_t check_param_valid(int32_t core_id)
{
	int32_t ret;

	ret = venc_check_coreid(core_id);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		return HI_FAILURE;
	}

	if (IS_ERR_OR_NULL(g_venc_clock_manager[core_id].venc_clk) ||
		IS_ERR_OR_NULL(g_venc_regulator.venc_regulator[core_id]) ||
		IS_ERR_OR_NULL(g_venc_regulator.media_regulator)) {
		HI_FATAL_VENC("core %d: regulator param error", core_id);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

static int32_t power_on_single_core(int32_t core_id)
{
	int32_t ret;
	uint32_t low_rate;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	struct venc_context *ctx = NULL;

	if (check_param_valid(core_id) != HI_SUCCESS)
		return HI_FAILURE;

	ctx = &venc->ctx[core_id];

	if (ctx->status != VENC_POWER_OFF)
		return HI_SUCCESS;

	ret = regulator_enable(g_venc_regulator.media_regulator);
	if (ret != 0) {
		HI_FATAL_VENC("core %d, enable media regulator failed", core_id);
		return HI_FAILURE;
	}

	ret = clk_prepare_enable(g_venc_clock_manager[core_id].venc_clk);
	if (ret != 0) {
		HI_FATAL_VENC("core %d, prepare clk enable failed", core_id);
		goto on_error_regulator;
	}

	/* we need set lowest clk rate before power on */
#ifdef HIVCODECV500
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOWER];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#else
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOW;
#endif

	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, low_rate);
	if (ret != 0) {
		HI_FATAL_VENC("core %d, set clk low rate failed", core_id);
		goto on_error_prepare_clk;
	}

	ret = regulator_enable(g_venc_regulator.venc_regulator[core_id]);
	if (ret != 0) {
		HI_FATAL_VENC("core %d, enable regulator failed", core_id);
		goto on_error_prepare_clk;
	}

	ctx->reg_base = (uint32_t *)hi_mmap(g_venc_config.venc_conf_priv[core_id].reg_base_addr,
			g_venc_config.venc_conf_priv[core_id].reg_range);

	if (!ctx->reg_base) {
		HI_ERR_VENC("core %d, ioremap failed", core_id);
		goto on_error_set_base_addr;
	}

#ifndef HIVCODECV500
	venc_hal_set_smmu_addr((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
#endif

	venc_hal_disable_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
	venc_hal_clr_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
#ifdef IRQ_EN
	if (venc_drv_osal_irq_init(ctx->irq_num_normal, venc_drv_encode_done) == HI_FAILURE) {
		HI_ERR_VENC("core_id is %d, venc_drv_osal_irq_init failed", core_id);
		goto on_error_set_irq;
	}
#endif
	osal_init_timer(&ctx->timer, venc->ops.encode_timeout, (unsigned long)core_id);//lint !e571

#ifdef VENC_QOS_CFG
	ret = config_qos();  /* if config_qos fail, it only effects performance */
	if (ret != HI_SUCCESS)
		HI_ERR_VENC("%s config qos failed", __func__);
#endif

#ifdef SMMU_V3
	ret = venc_smmu_init_tbu(core_id);
	if (ret != SMMU_OK) {
#ifdef IRQ_EN
		venc_drv_osal_irq_free(ctx->irq_num_normal);
#endif
		goto on_error_set_irq;
	}
#endif
#ifdef VENC_DPM_ENABLE
	venc_dpm_init(&g_venc_config);
#endif

	ctx->status = VENC_IDLE;
	ctx->first_cfg_flag = true;

	pm_hardware_power_on_enter(&ctx->pm);

	HI_DBG_VENC("core_id is %d, power on++", core_id);
	return HI_SUCCESS;

on_error_set_irq:
	hi_munmap(ctx->reg_base);
on_error_set_base_addr:
	if (regulator_disable(g_venc_regulator.venc_regulator[core_id]))
		HI_ERR_VENC("core_id is %d, disable media regulator failed", core_id);
on_error_prepare_clk:
	clk_disable_unprepare(g_venc_clock_manager[core_id].venc_clk);
on_error_regulator:
	if (regulator_disable(g_venc_regulator.media_regulator))
		HI_ERR_VENC("core_id is %d, disable media regulator failed", core_id);

	return HI_FAILURE;
}

static int32_t power_off_single_core(int32_t core_id)
{
	int32_t ret;
	uint32_t low_rate;
	struct venc_context *ctx = NULL;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	if (check_param_valid(core_id) != HI_SUCCESS)
		return HI_FAILURE;

	ctx = &venc->ctx[core_id];

	if (ctx->status == VENC_POWER_OFF)
		return HI_SUCCESS;

	if (ctx->status == VENC_BUSY || ctx->status == VENC_TIME_OUT) {
		HI_WARN_VENC("The current power-off core %d status is %d", core_id, ctx->status);
#ifdef HIVCODECV500
		vedu_hal_request_bus_idle(ctx->reg_base);
#endif
	}

#ifdef SMMU_V3
	venc_smmu_deinit();
#endif
	venc_hal_disable_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));
	venc_hal_clr_all_int((S_HEVC_AVC_REGS_TYPE *)(ctx->reg_base));

	/* there is not timer running nomally */
	if (osal_del_timer(&ctx->timer, true) == HI_SUCCESS)
		HI_WARN_VENC("core %d: timer is pending, when power off", core_id);

#ifdef IRQ_EN
	venc_drv_osal_irq_free(ctx->irq_num_normal);
#endif

#ifdef VENC_DPM_ENABLE
	venc_dpm_deinit();
#endif

	hi_munmap(ctx->reg_base);

	ret = regulator_disable(g_venc_regulator.venc_regulator[core_id]);
	if (ret != 0)
		HI_ERR_VENC("core_id is %d, disable regulator failed", core_id);

	/* we need set lowest clk rate before power off */
#if ((defined HIVCODECV600) || (defined HIVCODECV520))
	low_rate = g_venc_config.venc_conf_com.power_off_clk_rate;
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#elif defined HIVCODECV500
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOWER];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#else
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOW;
#endif

	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, low_rate);
	if (ret != 0)
		HI_ERR_VENC("core_id is %d, set lowest clk rate failed before power off", core_id);

	clk_disable_unprepare(g_venc_clock_manager[core_id].venc_clk);

	ret = regulator_disable(g_venc_regulator.media_regulator);
	if (ret != 0)
		HI_ERR_VENC("disable media regulator failed");

	ctx->status = VENC_POWER_OFF;
	ctx->first_cfg_flag = true;

	pm_hardware_power_on_exit(&ctx->pm);

	return HI_SUCCESS;
}

static uint32_t power_on_multi_core(uint32_t powr_on_core_num)
{
	int32_t ret;
	uint32_t i;
	uint32_t count = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; (i < g_venc_config.venc_conf_com.core_num) &&
			(count < powr_on_core_num); i++) {
		if (g_venc_config.venc_conf_com.is_single_core
			&& g_venc_config.venc_conf_com.valid_core_id != i)
			continue;

		if (venc->ctx[i].status == VENC_POWER_OFF) {
			ret = power_on_single_core(i);
			if (ret != HI_SUCCESS) {
				HI_WARN_VENC("power on core %d failed", i);
				continue;
			}
			count++;
		}
	}

	if (count != powr_on_core_num)
		HI_WARN_VENC("power on %d cores, but we need power on %d cores",
				count, powr_on_core_num);

	return count;
}

static uint32_t power_off_multi_core(uint32_t power_off_core_num)
{
	int32_t ret;
	int32_t i;
	uint32_t count = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = g_venc_config.venc_conf_com.core_num - 1;
			(i >= 0) && (count < power_off_core_num); i--) {
		if (venc->ctx[i].status == VENC_POWER_OFF)
			continue;

		wait_core_idle(i);
		ret = power_off_single_core(i);
		if (ret != HI_SUCCESS) {
			HI_WARN_VENC("power off core %d failed", i);
			continue;
		}
		count++;
	}

	if (count != power_off_core_num)
		HI_WARN_VENC("power off %d cores, but we need power off %d cores",
				count, power_off_core_num);

	return count;
}

static int32_t switch_core_num(uint32_t core_num)
{
	uint32_t ret;
	uint32_t i;
	uint32_t count;
	uint32_t cur_core_num = 0;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status != VENC_POWER_OFF)
			cur_core_num++;
	}

	if (cur_core_num == core_num)
		return HI_SUCCESS;

	HI_DBG_VENC("switch the working core from %d to %d", cur_core_num, core_num);

	if (cur_core_num < core_num) {
		count = core_num - cur_core_num;
		ret = power_on_multi_core(count);
	} else {
		count = cur_core_num - core_num;
		ret = power_off_multi_core(count);
	}

	return (ret == count) ? HI_SUCCESS : HI_FAILURE;
}

#if ((defined HIVCODECV600) || (defined HIVCODECV310) || (defined HIVCODECV520))
static int32_t set_tmp_clk_rate(int32_t core_id, venc_clk_t level, bool is_v310, bool need_transient_freq_rate)
{
	int32_t ret = 0;
	uint32_t current_clk;

	if (is_v310 && !need_transient_freq_rate) {
		return ret;
	}

	current_clk = clk_get_rate(g_venc_clock_manager[core_id].venc_clk);
	if (current_clk == g_venc_config.venc_conf_com.clk_rate[level]) {
		ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk,
			g_venc_config.venc_conf_com.transit_clk_rate);
		if (ret)
			HI_WARN_VENC("core_id is %d, failed set transit clk,fail code is %d", core_id, ret);
	}

	return ret;
}
#endif

static int32_t set_clk_rate_single_core(venc_clk_t clk_type, int32_t core_id)
{
	int32_t ret;
	uint32_t need_clk;
	uint32_t current_clk;
	venc_clk_t need_clk_type;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	ret = venc_check_coreid(core_id);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		return HI_FAILURE;
	}

	if (venc->ctx[core_id].status == VENC_POWER_OFF)
		return HI_SUCCESS;

#ifdef SUPPORT_VENC_FREQ_CHANGE
	mutex_lock(&g_venc_freq_mutex);
	if (clk_type < g_venc_freq)
		clk_type = g_venc_freq;
	mutex_unlock(&g_venc_freq_mutex);
#endif

	if (g_venc_clock_manager[core_id].curr_clk_type == clk_type)
		return HI_SUCCESS;

	need_clk = g_venc_config.venc_conf_com.clk_rate[clk_type];
	current_clk = clk_get_rate(g_venc_clock_manager[core_id].venc_clk);
	if (need_clk == current_clk) {
		g_venc_clock_manager[core_id].curr_clk_type = current_clk;
		HI_WARN_VENC("core_id is %d, failed set clk to %u Hz", core_id, current_clk);
		return HI_SUCCESS;
	}

	HI_DBG_VENC("core %d: set clk type from %d to %d, clk rate from %u to %u",
			core_id, g_venc_clock_manager[core_id].curr_clk_type, clk_type, current_clk, need_clk);

#if ((defined HIVCODECV600) || (defined HIVCODECV520))
	ret = set_tmp_clk_rate(core_id, VENC_CLK_RATE_NORMAL, false, g_need_transient_freq_rate);
#elif defined HIVCODECV310
	ret = set_tmp_clk_rate(core_id, VENC_CLK_RATE_HIGH, true, g_need_transient_freq_rate);
#endif
	if (ret)
		HI_WARN_VENC("core_id is %d, temprate set failed, clk to %u Hz", core_id, current_clk);

	need_clk_type = clk_type;
	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);

	while ((ret != HI_SUCCESS) && (need_clk_type > 0)) {
		need_clk_type--;
		if (current_clk != g_venc_config.venc_conf_com.clk_rate[need_clk_type]) {
			HI_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d", core_id, need_clk, ret);
			need_clk = g_venc_config.venc_conf_com.clk_rate[need_clk_type];
			ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);
		} else {
			break;
		}
	}

	if (ret == HI_SUCCESS) {
		g_venc_clock_manager[core_id].curr_clk_type = need_clk_type;
#ifdef VENC_DPM_ENABLE
		venc_dpm_freq_select(g_venc_clock_manager[core_id].curr_clk_type);
#endif
	} else {
		HI_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d", core_id, need_clk, ret);
	}
	return ret;
}

static int32_t set_clk_rate(venc_clk_t clk_type)
{
	uint32_t i;
	int32_t ret = HI_SUCCESS;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (set_clk_rate_single_core(clk_type, i) != HI_SUCCESS) {
			HI_ERR_VENC("set clock rate core %d failed", i);
			ret = HI_FAILURE;
		}
	}

	return ret;
}

static int32_t process_encode_timeout(void)
{
	uint32_t i;
	int32_t ret = HI_SUCCESS;
	venc_clk_t venc_curr_clk;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status == VENC_TIME_OUT) {
			HI_WARN_VENC("core_id: %d reset", i);
			venc_curr_clk = g_venc_clock_manager[i].curr_clk_type;

			if (power_off_single_core(i) != HI_SUCCESS) {
				HI_ERR_VENC("power off core %d failed", i);
				ret = HI_FAILURE;
				continue;
			}

			if (power_on_single_core(i) != HI_SUCCESS) {
				HI_ERR_VENC("power on core %d failed", i);
				ret = HI_FAILURE;
				continue;
			}

			if (set_clk_rate_single_core(venc_curr_clk, i) != HI_SUCCESS) {
				HI_ERR_VENC("set clock rate core %d failed", i);
				ret = HI_FAILURE;
			}
		}
	}

	return ret;
}

static int32_t get_idle_core(void)
{
	uint32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	if (process_encode_timeout() != HI_SUCCESS)
		HI_WARN_VENC("time out reset reg fail");

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (venc->ctx[i].status == VENC_IDLE)
			break;
	}

	if (i == g_venc_config.venc_conf_com.core_num)
		return -1;

	return i;
}

int32_t venc_regulator_enable(void)
{
	int32_t ret;
	ret = power_on_single_core(g_venc_config.venc_conf_com.valid_core_id);
	if (ret != HI_SUCCESS) {
		HI_INFO_VENC("core %d: enable regulator failed", g_venc_config.venc_conf_com.valid_core_id);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

int32_t venc_regulator_disable(void)
{
	int32_t ret;
	uint32_t i;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		ret = power_off_single_core(i);
		if (ret != HI_SUCCESS) {
			HI_ERR_VENC("core %d: disable regulator failed", i);
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

int32_t venc_regulator_disable_by_low_power(void)
{
	uint32_t i;
	int32_t ret = HI_SUCCESS;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (!pm_if_need_power_off(&venc->ctx[i].pm))
			continue;

		if (power_off_single_core(i) != HI_SUCCESS) {
			HI_ERR_VENC("core %d: disable regulator failed", i);
			ret = HI_FAILURE;
		}
	}

	return ret;
}

int32_t venc_regulator_select_idle_core(vedu_osal_event_t *event)
{
	int32_t ret;
	int32_t core_id = -1;

	if (event == NULL) {
		HI_ERR_VENC("event input is NULL");
		return -1;
	}

	ret = OSAL_WAIT_EVENT_TIMEOUT(event, (core_id = get_idle_core()) >= 0, ENCODE_DONE_TIMEOUT_MS); /*lint !e666 !e578*/
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("wait idle core timeout");
		return -1;
	}

	return core_id;
}

int32_t venc_regulator_wait_hardware_idle(vedu_osal_event_t *event)
{
	int32_t ret;

	if (event == NULL) {
		HI_ERR_VENC("event input is NULL");
		return HI_FAILURE;
	}

	/*lint -e578 -e666*/
	ret = OSAL_WAIT_EVENT_TIMEOUT(event, is_hardware_busy() == false,
			WAIT_CORE_IDLE_TIMEOUT_MS);
	/*lint +e578 +e666*/
	if (ret != HI_SUCCESS) {
		HI_WARN_VENC("wait hardware idle timeout");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

int32_t venc_regulator_update(struct clock_info *clock_info)
{
	int32_t ret;

	if (clock_info == NULL) {
		HI_ERR_VENC("clock info input is NULL");
		return HI_FAILURE;
	}

	if (clock_info->core_num > g_venc_config.venc_conf_com.core_num) {
		HI_ERR_VENC("core num %d is more than the total %d",
				clock_info->core_num, g_venc_config.venc_conf_com.core_num);
		return HI_FAILURE;
	}

	if (clock_info->clock_type >= VENC_CLK_BUTT) {
		HI_ERR_VENC("clock type %d invalid", clock_info->clock_type);
		return HI_FAILURE;
	}

	ret = switch_core_num(clock_info->core_num);
	if (ret != HI_SUCCESS) {
		HI_WARN_VENC("switch encode core num %d failed", clock_info->core_num);
		return HI_FAILURE;
	}

	ret = set_clk_rate(clock_info->clock_type);
	if (ret != HI_SUCCESS) {
		HI_WARN_VENC("set venc clkrate failed, ret: %d", ret);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

int32_t venc_regulator_reset(void)
{
	int32_t ret;
	int32_t i;
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) { //lint !e574
		if (venc->ctx[i].status == VENC_POWER_OFF)
			continue;

		wait_core_idle(i);

		ret = power_off_single_core(i);
		if (ret != HI_SUCCESS) {
			HI_ERR_VENC("power off core %d failed", i);
			return HI_FAILURE;
		}

		ret = power_on_single_core(i);
		if (ret != HI_SUCCESS) {
			HI_ERR_VENC("power on core %d failed", i);
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

bool venc_regulator_is_fpga(void)
{
	return (bool)g_venc_config.venc_conf_com.fpga_flag;
}

uint64_t venc_get_smmu_ttb(void)
{
	return g_smmu_page_base_addr;
}

bool is_support_power_control_per_frame(void)
{
	return g_support_power_control_per_frame;
}

uint32_t venc_get_reg_base_addr(int32_t core_id)
{
	if (venc_check_coreid(core_id) != HI_SUCCESS)
		return 0;

	return g_venc_config.venc_conf_priv[core_id].reg_base_addr;
}

uint32_t venc_get_core_num(void)
{
	return g_venc_config.venc_conf_com.core_num;
}

void venc_set_clock_info(struct clock_info *info)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	mutex_lock(&venc->backup_info.lock);
	venc->backup_info.info = *info;
	mutex_unlock(&venc->backup_info.lock);
}

void venc_get_clock_info(struct clock_info *info)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	mutex_lock(&venc->backup_info.lock);
	*info = venc->backup_info.info;
	mutex_unlock(&venc->backup_info.lock);
}
