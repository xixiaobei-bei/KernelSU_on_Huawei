
#include "venc_regulator.h"
#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/hisi-iommu.h>
#include "drv_venc_osal.h"
#include "drv_venc.h"

HI_U64 g_smmu_page_base_addr;
struct iommu_domain *g_hisi_mmu_domain = HI_NULL;
struct venc_config g_venc_config;
static struct venc_regulator g_venc_regulator;
static struct venc_clock g_venc_clock_manager[MAX_SUPPORT_CORE_NUM];

#ifdef VENC_QOS_CFG
#define VENC_QOS_MODE                0xE894000C
#define VENC_QOS_BANDWIDTH           0xE8940010
#define VENC_QOS_SATURATION          0xE8940014
#define VENC_QOS_EXTCONTROL          0xE8940018

static HI_U32 g_venc_qos_mode        = 0x1;
static HI_U32 g_venc_qos_bandwidth   = 0x1000;
static HI_U32 g_venc_qos_saturation  = 0x20;
static HI_U32 g_venc_qos_extcontrol  = 0x1;
#endif

#ifdef CONFIG_ES_VENC_LOW_FREQ
static HI_U32 g_venc_low_freq        = 480000000;
#endif

HI_S32 get_dts_config_info(struct platform_device *pdev)
{
	HI_S32 ret;
	HI_U32 i;
	HI_CHAR tmp_name[MAX_NAME_LEN] = {0};
	struct resource *res = NULL;
	struct device_node *np = NULL;
	struct device *dev = &pdev->dev;

	memset(&g_venc_config, 0, sizeof(g_venc_config));    /* unsafe_function_ignore: memset */
	memset(g_venc_context, 0, sizeof(g_venc_context));    /* unsafe_function_ignore: memset */
	memset(g_venc_clock_manager, 0, sizeof(g_venc_clock_manager));    /* unsafe_function_ignore: memset */

	if (!dev) {
		HI_FATAL_VENC("invalid argument, dev is NULL");
		return HI_FAILURE;
	}

	np = dev->of_node;

	if (!np) {
		HI_FATAL_VENC("invalid argument np is NULL");
		return HI_FAILURE;
	}

	/* 0 get venc core num */
	ret = of_property_read_u32(np, "core_num", &g_venc_config.venc_conf_com.core_num);

	if (ret) {
		HI_INFO_VENC("read property of core num fail set default");
		g_venc_config.venc_conf_com.core_num = 1;
	}

	if ((g_venc_config.venc_conf_com.core_num > MAX_SUPPORT_CORE_NUM) ||
		(g_venc_config.venc_conf_com.core_num <= 0)) {
		HI_FATAL_VENC("read property of core num[%d] invalid", g_venc_config.venc_conf_com.core_num);
		return HI_FAILURE;
	}

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		/* 1 read IRQ num from dts */
		g_venc_context[i].irq_num_normal = irq_of_parse_and_map(np, 3 * i);

		if (g_venc_context[i].irq_num_normal == 0) {
			HI_FATAL_VENC("parse and map irq VeduIrqNumNorm failed");
			return HI_FAILURE;
		}

		g_venc_context[i].irq_num_protect = irq_of_parse_and_map(np, 1 + 3 * i);

		if (g_venc_context[i].irq_num_protect == 0) {
			HI_FATAL_VENC("parse and map irq VeduIrqNumProt failed");
			return HI_FAILURE;
		}

		g_venc_context[i].irq_num_safe = irq_of_parse_and_map(np, 2 + 3 * i);

		if (g_venc_context[i].irq_num_safe == 0) {
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
		if (i == 0)
			strncpy(tmp_name, VENC_CLOCK_NAME, MAX_NAME_LEN - 1);    /* unsafe_function_ignore: strncpy */
		else
			snprintf(tmp_name, sizeof(tmp_name), "%s%d", VENC_CLOCK_NAME, i);    /* unsafe_function_ignore: snprintf */

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

	g_smmu_page_base_addr = (HI_U64)(hisi_domain_get_ttbr(&pdev->dev));

	if (g_smmu_page_base_addr == 0) {
		HI_ERR_VENC("get mmu addr failed");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 get_regulator_info(struct platform_device *pdev)
{
	HI_U32 i;
	HI_CHAR tmp_name[MAX_NAME_LEN] = {0};

	memset(&g_venc_regulator, 0, sizeof(g_venc_regulator));    // unsafe_function_ignore: memset

	g_venc_regulator.media_regulator = devm_regulator_get(&pdev->dev, MEDIA_REGULATOR_NAME);

	if (IS_ERR_OR_NULL(g_venc_regulator.media_regulator)) {
		HI_FATAL_VENC("get media regulator failed, error no is %ld", PTR_ERR(g_venc_regulator.media_regulator));
		g_venc_regulator.media_regulator = HI_NULL;
		return HI_FAILURE;
	}

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (i == 0)
			strncpy(tmp_name, VENC_REGULATOR_NAME, MAX_NAME_LEN - 1);    // unsafe_function_ignore: strncpy
		else
			snprintf(tmp_name, sizeof(tmp_name), "%s%d", VENC_REGULATOR_NAME, i);    // unsafe_function_ignore: snprintf

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
static HI_S32 config_qos(void)
{
	HI_U32 *qos_addr = HI_NULL;

	qos_addr = (HI_U32 *)ioremap(VENC_QOS_MODE, 4);

	if (!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_MODE reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_mode, qos_addr);
	iounmap(qos_addr);

	qos_addr = (HI_U32 *)ioremap(VENC_QOS_BANDWIDTH, 4);

	if (!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_BANDWIDTH reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_bandwidth, qos_addr);
	iounmap(qos_addr);

	qos_addr = (HI_U32 *)ioremap(VENC_QOS_SATURATION, 4);

	if (!qos_addr) {
		HI_FATAL_VENC("ioremap Venc_QOS_SATURATION reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_saturation, qos_addr);
	iounmap(qos_addr);

	qos_addr = (HI_U32 *)ioremap(VENC_QOS_EXTCONTROL, 4);

	if (!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_EXTCONTROL reg failed! ");
		return HI_FAILURE;
	}

	writel(g_venc_qos_extcontrol, qos_addr);
	iounmap(qos_addr);

	return HI_SUCCESS;
}
#endif

static HI_BOOL is_hardware_busy(HI_VOID)
{
	HI_U32 i;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (g_venc_context[i].status == VENC_BUSY)
			return HI_TRUE;
	}

	return HI_FALSE;
}

static HI_S32 wait_core_idle(HI_U32 core_id)
{
	HI_U32 ret;
	vedu_osal_event_t *event = venc_drv_get_encode_done_event_handle();

	if (event == NULL) {
		HI_ERR_VENC("get encode done event handle failed");
		return HI_FAILURE;
	}

	ret = OSAL_WAIT_EVENT_TIMEOUT(event,
			g_venc_context[core_id].status != VENC_BUSY,
			WAIT_CORE_IDLE_TIMEOUT_MS); /*lint !e578 !e666*/

	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("wait core idle timeout");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

static HI_S32 power_on_single_core(HI_S32 core_id)
{
	HI_S32 ret;
	HI_U32 low_rate;

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

	if (g_venc_context[core_id].status != VENC_POWER_OFF)
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
	HI_INFO_VENC("core %d, clk_set_rate lowRate:%u", core_id, low_rate);

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

	g_venc_context[core_id].reg_base = (HI_U32 *)hi_mmap(g_venc_config.venc_conf_priv[core_id].reg_base_addr,
			g_venc_config.venc_conf_priv[core_id].reg_range);

	if (!g_venc_context[core_id].reg_base) {
		HI_ERR_VENC("core %d, ioremap failed", core_id);
		goto on_error_set_base_addr;
	}

#ifndef HIVCODECV500
	venc_hal_set_smmu_addr((S_HEVC_AVC_REGS_TYPE *)(g_venc_context[core_id].reg_base));
#endif
	venc_hal_disable_all_int((S_HEVC_AVC_REGS_TYPE *)(g_venc_context[core_id].reg_base));
	venc_hal_clr_all_int((S_HEVC_AVC_REGS_TYPE *)(g_venc_context[core_id].reg_base));
#ifdef IRQ_EN
	if (venc_drv_osal_irq_init(g_venc_context[core_id].irq_num_normal, venc_drv_encode_done) == HI_FAILURE) {
		HI_ERR_VENC("core_id is %d, venc_drv_osal_irq_init failed", core_id);
		goto on_error_set_irq;
	}
#endif
	g_venc_context[core_id].status = VENC_IDLE;
	g_venc_context[core_id].first_cfg_flag = HI_TRUE;

#ifdef VENC_QOS_CFG
	ret = config_qos();  /* if config_qos fail, it only effects performance */
	if (ret != HI_SUCCESS)
		HI_ERR_VENC("%s config qos failed", __func__);
#endif

	HI_INFO_VENC("core_id is %d, power on++", core_id);
	return HI_SUCCESS;

on_error_set_irq:
	hi_munmap(g_venc_context[core_id].reg_base);
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

static HI_S32 power_off_single_core(HI_S32 core_id)
{
	HI_S32 ret;
	HI_U32 low_rate;

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

	if (g_venc_context[core_id].status == VENC_POWER_OFF)
		return HI_SUCCESS;

	if (g_venc_context[core_id].status == VENC_BUSY)
		HI_WARN_VENC("core %d: the going to power off core is busy now", core_id);

	venc_hal_disable_all_int((S_HEVC_AVC_REGS_TYPE *)(g_venc_context[core_id].reg_base));
	venc_hal_clr_all_int((S_HEVC_AVC_REGS_TYPE *)(g_venc_context[core_id].reg_base));

	/* there is not timer running nomally */
	if (osal_del_timer(&g_venc_context[core_id].timer, HI_TRUE) == HI_SUCCESS)
		HI_WARN_VENC("core %d: timer is pending, when power off", core_id);

#ifdef IRQ_EN
	venc_drv_osal_irq_free(g_venc_context[core_id].irq_num_normal);
#endif
	hi_munmap(g_venc_context[core_id].reg_base);

	ret = regulator_disable(g_venc_regulator.venc_regulator[core_id]);

	if (ret != 0)
		HI_ERR_VENC("core_id is %d, disable regulator failed", core_id);

	/* we need set lowest clk rate before power off */
#ifdef HIVCODECV500
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOWER];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOWER;
#else
	low_rate = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW];
	g_venc_clock_manager[core_id].curr_clk_type = VENC_CLK_RATE_LOW;
#endif
	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, low_rate);

	if (ret != 0)
		HI_ERR_VENC("core_id is %d, set clk lowrate:%u failed", core_id, low_rate);

	HI_INFO_VENC("core_id is %d, set clk rate: %u", core_id, low_rate);

	clk_disable_unprepare(g_venc_clock_manager[core_id].venc_clk);

	ret = regulator_disable(g_venc_regulator.media_regulator);

	if (ret != 0) {
		HI_ERR_VENC("disable media regulator failed");
	}

	g_venc_context[core_id].status = VENC_POWER_OFF;
	g_venc_context[core_id].first_cfg_flag = HI_TRUE;

	HI_INFO_VENC("core_id is %d, power off--", core_id);
	return HI_SUCCESS;
}

static HI_U32 power_on_multi_core(HI_U32 powr_on_core_num)
{
	HI_S32 ret;
	HI_U32 i;
	HI_U32 count = 0;

	for (i = 0; (i < g_venc_config.venc_conf_com.core_num) &&
			(count < powr_on_core_num); i++) {
		if (g_venc_context[i].status == VENC_POWER_OFF) {
			ret = power_on_single_core(i);
			if (ret != HI_SUCCESS) {
				HI_WARN_VENC("power on core %d failed", i);
				continue;
			}
			count++;
		}
	}

	if (count != powr_on_core_num) {
		HI_WARN_VENC("power on %d cores, but we need power on %d cores",
				count, powr_on_core_num);
	}

	return count;
}

static HI_U32 power_off_multi_core(HI_U32 power_off_core_num)
{
	HI_S32 ret;
	HI_S32 i;
	HI_U32 count = 0;

	for (i = g_venc_config.venc_conf_com.core_num - 1;
			(i >= 0) && (count < power_off_core_num); i--) {
		if (g_venc_context[i].status == VENC_POWER_OFF)
			continue;

		wait_core_idle(i);
		ret = power_off_single_core(i);
		if (ret != HI_SUCCESS) {
			HI_WARN_VENC("power off core %d failed", i);
			continue;
		}
		count++;
	}

	if (count != power_off_core_num) {
		HI_WARN_VENC("power off %d cores, but we need power off %d cores",
				count, power_off_core_num);
	}

	return count;
}

static HI_S32 switch_core_num(HI_U32 core_num)
{
	HI_U32 ret;
	HI_U32 i;
	HI_U32 count;
	HI_U32 cur_core_num = 0;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (g_venc_context[i].status != VENC_POWER_OFF)
			cur_core_num++;
	}

	if (cur_core_num == core_num)
		return HI_SUCCESS;

	HI_INFO_VENC("switch the working core from %d to %d", cur_core_num, core_num);

	if (cur_core_num < core_num) {
		count = core_num - cur_core_num;
		ret = power_on_multi_core(count);
	} else {
		count = cur_core_num - core_num;
		ret = power_off_multi_core(count);
	}

	return (ret == count) ? HI_SUCCESS : HI_FAILURE;
}

static HI_S32 set_clk_rate_single_core(venc_clk_t clk_type, HI_S32 core_id)
{
	HI_S32 ret;
	HI_U32 need_clk;
	HI_U32 current_clk;
	venc_clk_t need_clk_type;

	ret = venc_check_coreid(core_id);
	if (ret != HI_SUCCESS) {
		HI_ERR_VENC("CORE_ERROR:invalid core ID is %d", core_id);
		return HI_FAILURE;
	}

	if (g_venc_context[core_id].status == VENC_POWER_OFF)
		return HI_SUCCESS;

	if (g_venc_clock_manager[core_id].curr_clk_type == clk_type)
		return HI_SUCCESS;

	need_clk = g_venc_config.venc_conf_com.clk_rate[clk_type];
	current_clk = clk_get_rate(g_venc_clock_manager[core_id].venc_clk);

	if (need_clk == current_clk) {
		g_venc_clock_manager[core_id].curr_clk_type = current_clk;
		HI_WARN_VENC("core_id is %d, failed set clk to %u Hz", core_id, current_clk);
		return HI_SUCCESS;
	}

	HI_INFO_VENC("core %d: set clk type from %d to %d, clk rate from %u to %u",
			core_id, g_venc_clock_manager[core_id].curr_clk_type, clk_type, current_clk, need_clk);
	need_clk_type = clk_type;
	ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);

	if ((ret != HI_SUCCESS) && (need_clk == g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_HIGH]) &&
		(current_clk != g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_NORMAL])) {
		HI_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d, will reset high", core_id, need_clk, ret);
		need_clk = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_NORMAL];
		need_clk_type = VENC_CLK_RATE_NORMAL;
		ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);
	}

#ifdef VENC_AVS_LOW_CFG
	if ((ret != HI_SUCCESS) && (need_clk == g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_NORMAL]) &&
		(current_clk != g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW])) {
		HI_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d, will reset normal", core_id, need_clk, ret);
		need_clk = g_venc_config.venc_conf_com.clk_rate[VENC_CLK_RATE_LOW];
		need_clk_type = VENC_CLK_RATE_LOW;
		ret = clk_set_rate(g_venc_clock_manager[core_id].venc_clk, need_clk);
	}
#endif
	if (ret == HI_SUCCESS)
		g_venc_clock_manager[core_id].curr_clk_type = need_clk_type;
	else
		HI_WARN_VENC("core_id is %d, failed set clk to %u Hz,fail code is %d", core_id, need_clk, ret);
	return ret;
}

static HI_S32 set_clk_rate(venc_clk_t clk_type)
{
	HI_U32 i = 0;
	HI_S32 ret = HI_SUCCESS;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (set_clk_rate_single_core(clk_type, i) != HI_SUCCESS) {
			HI_ERR_VENC("set clock rate core %d failed", i);
			ret = HI_FAILURE;
		}
	}

	return ret;
}

static HI_S32 process_encode_timeout(HI_VOID)
{
	HI_U32 i;
	HI_S32 ret = HI_SUCCESS;
	venc_clk_t venc_curr_clk = 0;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (g_venc_context[i].status == VENC_TIME_OUT) {
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

static HI_S32 get_idle_core(HI_VOID)
{
	HI_U32 i;

	if (process_encode_timeout() != HI_SUCCESS)
		HI_WARN_VENC("time out reset reg fail");

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		if (g_venc_context[i].status == VENC_IDLE)
			break;
	}

	if (i == g_venc_config.venc_conf_com.core_num)
		return -1;

	return i;
}

HI_S32 venc_regulator_enable(HI_VOID)
{
	HI_S32 ret;
	ret = power_on_single_core(VENC_CORE_0);

	if (ret != HI_SUCCESS) {
		HI_INFO_VENC("core %d: enable regulator failed", VENC_CORE_0);
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 venc_regulator_disable(HI_VOID)
{
	HI_S32 ret;
	HI_U32 i;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) {
		ret = power_off_single_core(i);
		if (ret != HI_SUCCESS) {
			HI_ERR_VENC("core %d: disable regulator failed", i);
			return HI_FAILURE;
		}
	}

	return HI_SUCCESS;
}

HI_S32 venc_regulator_select_idle_core(vedu_osal_event_t *event)
{
	HI_U32 ret;
	HI_S32 core_id = -1;

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

HI_S32 venc_regulator_wait_hardware_idle(vedu_osal_event_t *event)
{
	HI_S32 ret;

	if (event == NULL) {
		HI_ERR_VENC("event input is NULL");
		return HI_FAILURE;
	}

	ret = OSAL_WAIT_EVENT_TIMEOUT(event, is_hardware_busy() == HI_FALSE,
			WAIT_CORE_IDLE_TIMEOUT_MS); /*lint !e666 !e578*/

	if (ret != HI_SUCCESS) {
		HI_WARN_VENC("wait hardware idle timeout");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_S32 venc_regulator_update(const struct clock_info *clock_info)
{
	HI_S32 ret;

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

HI_S32 venc_regulator_reset(HI_VOID)
{
	HI_S32 ret;
	HI_S32 i = 0;

	for (i = 0; i < g_venc_config.venc_conf_com.core_num; i++) { //lint !e574
		if (g_venc_context[i].status == VENC_POWER_OFF)
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

HI_BOOL venc_regulator_is_fpga(HI_VOID)
{
	return (HI_BOOL)g_venc_config.venc_conf_com.fpga_flag;
}

HI_U64 venc_get_smmu_ttb(HI_VOID)
{
    return g_smmu_page_base_addr;
}
