/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: This file implements ivp initialization and control functions
 * Author: chenweiyu
 * Create: 2019-02-18
 */

#include "ivp_platform.h"
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include "ivp_log.h"
#include "ivp_reg.h"
#include "ivp_rdr.h"
#include "ivp_manager.h"

#define REMAP_ADD                    0xE9100000
#define DEAD_FLAG                    0xDEADBEEF
#define SIZE_16K                     (16 * 1024)

#define IVP_CORE_RD_QOS_MODE         0x000C
#define IVP_CORE_RD_QOS_BANDWIDTH    0x0010
#define IVP_CORE_RD_QOS_SATURATION   0x0014
#define IVP_CORE_RD_QOS_EXTCONTROL   0x0018

#define IVP_CORE_WR_QOS_MODE         (IVP_CORE_RD_QOS_MODE + 0x80)
#define IVP_CORE_WR_QOS_BANDWIDTH    (IVP_CORE_RD_QOS_BANDWIDTH + 0x80)
#define IVP_CORE_WR_QOS_SATURATION   (IVP_CORE_RD_QOS_SATURATION + 0x80)
#define IVP_CORE_WR_QOS_EXTCONTROL   (IVP_CORE_RD_QOS_EXTCONTROL + 0x80)

#define IVP_IDMA_RD_QOS_MODE         (IVP_CORE_RD_QOS_MODE + 0x100)
#define IVP_IDMA_RD_QOS_BANDWIDTH    (IVP_CORE_RD_QOS_BANDWIDTH + 0x100)
#define IVP_IDMA_RD_QOS_SATURATION   (IVP_CORE_RD_QOS_SATURATION + 0x100)
#define IVP_IDMA_RD_QOS_EXTCONTROL   (IVP_CORE_RD_QOS_EXTCONTROL + 0x100)

#define IVP_IDMA_WR_QOS_MODE         (IVP_CORE_RD_QOS_MODE + 0x180)
#define IVP_IDMA_WR_QOS_BANDWIDTH    (IVP_CORE_RD_QOS_BANDWIDTH + 0x180)
#define IVP_IDMA_WR_QOS_SATURATION   (IVP_CORE_RD_QOS_SATURATION + 0x180)
#define IVP_IDMA_WR_QOS_EXTCONTROL   (IVP_CORE_RD_QOS_EXTCONTROL + 0x180)

#define IVP32_QOS_PRIORITY           0x0208
#define IVP32_QOS_MODE               0x020C

#define IVP_PLL0_TRANS_CLK           406000000
#define IVP_PLL3_TRANS_CLK           300000000

#define IVP_CONF_MID_ADDR            0x00696A74
#define IVP_CONF_MID_VALUE           0x0338

void ivp_set_qos_cfg(struct ivp_device *dev);
int ivp_set_transitional_clk(struct ivp_device *ivp_devp,
	unsigned int pre_level, unsigned int cur_level);

static void *g_iram = NULL;

static void noc_ivp_reg_write(const struct ivp_device *ivp_devp, unsigned int off, u32 val)
{
	char __iomem *reg = ivp_devp->io_res.noc_ivp_base_addr + off;
	writel(val, reg);
}

static int ivp_get_memory_section(struct platform_device *plat_dev,
	struct ivp_device *ivp_devp)
{
	int ret;
	int i;
	unsigned int size = 0;
	dma_addr_t dma_addr = 0;

	ivp_devp->vaddr_memory = NULL;
	ret = of_property_read_u32(plat_dev->dev.of_node, OF_IVP_DYNAMIC_MEM, &size);
	if (ret || !size) {
		ivp_err("get failed/not use dynamic mem section, ret:%d, size:%d", ret, size);
		return -EINVAL;
	}
	ivp_devp->dynamic_mem_size = size;
	ivp_devp->ivp_meminddr_len = (int)size;

	ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_DYNAMIC_MEM_SEC_SIZE, &size);
	if (ret || !size) {
		ivp_err("get failed/not use dynamic mem section, ret:%d, size:%d", ret, size);
		return -EINVAL;
	}

	ivp_devp->dynamic_mem_section_size = size;
	if ((ivp_devp->dynamic_mem_section_size * (ivp_devp->sect_count - DDR_SECTION_INDEX)) !=
		ivp_devp->dynamic_mem_size) {
		ivp_err("dynamic_mem should be sect_count-3 times dynamic_mem_section");
		return -EINVAL;
	}

	/*lint -save -e598 -e648*/
	dma_set_mask_and_coherent(&ivp_devp->ivp_pdev->dev, DMA_BIT_MASK(DMA_64BIT));
	/*lint -restore*/
	ivp_devp->vaddr_memory = dma_alloc_coherent(&ivp_devp->ivp_pdev->dev,
		ivp_devp->dynamic_mem_size, &dma_addr, GFP_KERNEL);
	if (!ivp_devp->vaddr_memory) {
		ivp_err("get vaddr_memory failed");
		return -EINVAL;
	}
	for (i = DDR_SECTION_INDEX; i < ivp_devp->sect_count; i++) {
		if (i == DDR_SECTION_INDEX) {
			ivp_devp->sects[i].acpu_addr = dma_addr >> IVP_MMAP_SHIFT;
		} else {
			ivp_devp->sects[i].acpu_addr =
				((ivp_devp->sects[i - 1].acpu_addr << IVP_MMAP_SHIFT) +
				ivp_devp->sects[i - 1].len) >> IVP_MMAP_SHIFT;
			ivp_devp->sects[i].ivp_addr = ivp_devp->sects[i - 1].ivp_addr +
				ivp_devp->sects[i - 1].len;
		}
		ivp_devp->sects[i].len = ivp_devp->dynamic_mem_section_size;
		ivp_dbg("ivp sections 0x%pK", ivp_devp->sects[i].acpu_addr);
	}

	return ret;
}

static void ivp_free_memory_section(struct ivp_device *ivp_devp)
{
	dma_addr_t dma_addr;

	dma_addr = ivp_devp->sects[DDR_SECTION_INDEX].acpu_addr << IVP_MMAP_SHIFT;
	if (ivp_devp->vaddr_memory) {
		dma_free_coherent(&ivp_devp->ivp_pdev->dev,
			ivp_devp->dynamic_mem_size,
			ivp_devp->vaddr_memory, dma_addr);
		ivp_devp->vaddr_memory = NULL;
	}
}

static void ivp_hw_remap_ivp2ddr(struct ivp_device *ivp_devp,
	unsigned int ivp_addr, unsigned int len, unsigned long ddr_addr)
{
	unsigned int ivp_addr_base = ((ivp_addr / SIZE_1MB) << IVP_ADDR_SHIFT) +
		(ivp_addr / SIZE_1MB);
	unsigned int ddr_addr_base = ((ddr_addr / SIZE_1MB) << IVP_ADDR_SHIFT) +
		(ddr_addr / SIZE_1MB);
	unsigned int remap_len = (len << IVP_REMP_LEN_SHIFT) + len;

	ivp_info("ivp remap ddr Enter");
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_START_REMAP_ADDR, ivp_addr_base);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_REMAP_LENGTH, remap_len);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_DDR_REMAP_ADDR, (u32)ddr_addr_base);
}

static inline int ivp_remap_addr_ivp2ddr(struct ivp_device *ivp_devp,
	unsigned int ivp_addr, int len, unsigned long ddr_addr)
{
	ivp_dbg("ivp_addr:%#pK, len:%#x, ddr_addr:%#pK", ivp_addr, len, ddr_addr);
	if ((ivp_addr & MASK_1MB) != 0 ||
		(ddr_addr & MASK_1MB) != 0 ||
		len >= MAX_DDR_LEN * SIZE_1MB) {
		ivp_err("not aligned");
		return -EINVAL;
	}
	len = (len + SIZE_1MB - 1) / SIZE_1MB - 1;
	ivp_hw_remap_ivp2ddr(ivp_devp, ivp_addr, (u32)len, ddr_addr);

	return 0;
}

int ivp_poweron_pri(struct ivp_device *ivp_devp)
{
	int ret;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -ENODEV;
	}
	/* 0.Enable the power */
	if ((ret = regulator_enable(ivp_devp->ivp_media2_regulator)) != 0) {
		ivp_err("ivp_media2_regulator enable failed %d", ret);
		return ret;
	}

	/* 1.Set Clock rate */
	if ((ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->clk_rate)) != 0) {
		ivp_err("set rate %#x fail, ret:%d", ivp_devp->clk_rate, ret);
		goto try_down_freq;
	}

	/* 2.Enable the clock */
	if ((ret = clk_prepare_enable(ivp_devp->clk)) != 0) {
		ivp_err("clk prepare enable failed, ret = %d", ret);
		goto try_down_freq;
	}
	goto normal_frq_success;

try_down_freq:
	ivp_info("try set core freq to: %ld", (unsigned long)ivp_devp->lowtemp_clk_rate);
	if ((ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->lowtemp_clk_rate)) != 0) {
		ivp_err("set low rate %#x fail, ret:%d", ivp_devp->lowtemp_clk_rate, ret);
		goto err_clk_set_rate;
	}

	if ((ret = clk_prepare_enable(ivp_devp->clk)) != 0) {
		ivp_err("low clk prepare enable failed, ret = %d", ret);
		goto err_clk_prepare_enable;
	}

normal_frq_success:
	ivp_info("set core success to: %ld", clk_get_rate(ivp_devp->clk));
	/* 3.Enable the power */
	if ((ret = regulator_enable(ivp_devp->regulator)) != 0) {
		ivp_err("regularot enable failed %d", ret);
		goto err_regulator_enable_ivp;
	}
	ivp_set_qos_cfg(ivp_devp);

	return ret;

err_regulator_enable_ivp:
	clk_disable_unprepare(ivp_devp->clk);

err_clk_prepare_enable:
	if ((ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->lowtemp_clk_rate)) != 0)
		ivp_err("err set lowfrq rate %#x fail %d", ivp_devp->lowtemp_clk_rate, ret);

err_clk_set_rate:
	if ((ret = regulator_disable(ivp_devp->ivp_media2_regulator)) != 0)
		ivp_err("ivp_media2_regulator disable failed %d", ret);
	return -EINVAL;
}

int ivp_poweron_remap(struct ivp_device *ivp_devp)
{
	int ret;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -EINVAL;
	}
	ret = ivp_remap_addr_ivp2ddr(ivp_devp,
		ivp_devp->sects[DDR_SECTION_INDEX].ivp_addr,
		ivp_devp->ivp_meminddr_len,
		ivp_devp->sects[DDR_SECTION_INDEX].acpu_addr << IVP_MMAP_SHIFT);
	if (ret)
		ivp_err("remap addr failed %d", ret);

	return ret;
}

int ivp_poweroff_pri(struct ivp_device *ivp_devp)
{
	int ret;
	int i = 0;
	unsigned int waiti;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -EINVAL;
	}
	waiti = ivp_reg_read(ivp_devp, IVP_REG_OFF_PWAITMODE);
	while (((waiti & IVP_DSP_PWAITMODE) == 0) && (i < 3)) { /* 3 times */
		udelay(100); /* 100us */
		ivp_err("ivp core is not in wfi mode, 0x%x", waiti);
		waiti = ivp_reg_read(ivp_devp, IVP_REG_OFF_PWAITMODE);
		i++;
	}

	ret = regulator_disable(ivp_devp->regulator);
	if (ret)
		ivp_err("regulator power off failed %d", ret);

	clk_disable_unprepare(ivp_devp->clk);
	ret = clk_set_rate(ivp_devp->clk,
		(unsigned long)ivp_devp->lowfrq_pd_clk_rate);
	if (ret)
		ivp_warn("set lfrq pd rate %#x fail, ret:%d",
			ivp_devp->lowfrq_pd_clk_rate, ret);

	ret = regulator_disable(ivp_devp->ivp_media2_regulator);
	if (ret)
		ivp_err("ivp_media2_regulator power off failed %d", ret);

	return ret;
}

static int ivp_setup_regulator(struct platform_device *plat_dev,
	struct ivp_device *ivp_devp)
{
	struct regulator *ivp_media2_regulator = NULL;
	struct regulator *regulator = NULL;

	regulator = devm_regulator_get(&plat_dev->dev, IVP_REGULATOR);
	if (IS_ERR(regulator)) {
		ivp_err("Get ivp regulator failed");
		return -ENODEV;
	} else {
		ivp_devp->regulator = regulator;
	}

	ivp_media2_regulator = devm_regulator_get(&plat_dev->dev,
		IVP_MEDIA_REGULATOR);
	if (IS_ERR(ivp_media2_regulator)) {
		ivp_err("Get ivp media regulator failed");
		return -ENODEV;
	} else {
		ivp_devp->ivp_media2_regulator = ivp_media2_regulator;
	}

	return 0;
}

static int ivp_setup_clk(struct platform_device *plat_dev,
	struct ivp_device *ivp_devp)
{
	int ret;

	ivp_devp->clk = devm_clk_get(&plat_dev->dev, OF_IVP_CLK_NAME);
	if (IS_ERR(ivp_devp->clk)) {
		ivp_err("get clk failed");
		return -ENODEV;
	}
	ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_CLK_RATE_NAME, &ivp_devp->clk_rate);
	if (ret) {
		ivp_err("get clk rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_info("get clk rate: %u", ivp_devp->clk_rate);

	if ((ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_MIDDLE_CLK_RATE_NAME, &ivp_devp->middle_clk_rate)) != 0) {
		ivp_err("get middle freq rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_info("get middle freq clk rate: %u", ivp_devp->middle_clk_rate);

	if ((ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_LOW_CLK_RATE_NAME, &ivp_devp->low_clk_rate)) != 0) {
		ivp_err("get low freq rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_info("get low freq clk rate: %u", ivp_devp->low_clk_rate);

	if ((ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_ULTRA_LOW_CLK_RATE_NAME, &ivp_devp->ultra_low_clk_rate)) != 0) {
		ivp_err("get ultra low freq clk rate, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_info("get ultra low freq clk rate: %u", ivp_devp->ultra_low_clk_rate);

	if ((ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_LOWFREQ_CLK_RATE_NAME, &ivp_devp->lowfrq_pd_clk_rate)) != 0) {
		ivp_err("get lowfreq pd clk rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_info("get lowfrq pd clk rate: %u", ivp_devp->lowfrq_pd_clk_rate);

	if ((ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_LOW_TEMP_RATE_NAME, &ivp_devp->lowtemp_clk_rate)) != 0) {
		ivp_err("get low temperature rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_info("get low temperature clk rate: %u", ivp_devp->lowtemp_clk_rate);

	return ret;
}

int ivp_change_clk(struct ivp_device *ivp_devp, unsigned int level)
{
	int ret;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -EINVAL;
	}
	ret = ivp_set_transitional_clk(ivp_devp, ivp_devp->clk_level, level);
	if (ret) {
		ivp_err("set transitional freq fail, ret:%d", ret);
		return ret;
	}

	ivp_devp->clk_level = level;
	switch (ivp_devp->clk_level) {
	case IVP_CLK_LEVEL_ULTRA_LOW:
		ivp_info("ivp freq to ultra low level");
		ivp_devp->clk_usrsetrate = ivp_devp->ultra_low_clk_rate;
		break;

	case IVP_CLK_LEVEL_LOW:
		ivp_info("ivp freq to low level");
		ivp_devp->clk_usrsetrate = ivp_devp->low_clk_rate;
		break;

	case IVP_CLK_LEVEL_MEDIUM:
		ivp_info("ivp freq to media level");
		ivp_devp->clk_usrsetrate = ivp_devp->middle_clk_rate;
		break;

	case IVP_CLK_LEVEL_HIGH:
		ivp_info("ivp freq to high level");
		ivp_devp->clk_usrsetrate = ivp_devp->clk_rate;
		break;

	default:
		ivp_info("use default freq");
		ivp_devp->clk_usrsetrate = ivp_devp->clk_rate;
		break;
	}

	/* Set Clock rate */
	ivp_info("set clock rate");
	ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->clk_usrsetrate);
	if (!ret) {
		ivp_info("set core success to: %ld", clk_get_rate(ivp_devp->clk));
		return ret;
	}

	ivp_info("try set core freq to: %ld", (unsigned long)ivp_devp->ultra_low_clk_rate);
	ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->ultra_low_clk_rate);
	if (ret)
		ivp_err("set low rate %#x fail, ret:%d", ivp_devp->ultra_low_clk_rate, ret);

	return ret;
}

int ivp_init_pri(struct platform_device *plat_dev, struct ivp_device *ivp_devp)
{
	int ret;

	if (!plat_dev || !ivp_devp) {
		ivp_err("invalid input param plat_dev or ivp_devp");
		return -EINVAL;
	}
	ret = ivp_setup_regulator(plat_dev, ivp_devp);
	if (ret) {
		ivp_err("setup regulator failed, ret:%d", ret);
		return ret;
	}

	ret = ivp_setup_clk(plat_dev, ivp_devp);
	if (ret) {
		ivp_err("setup clk failed, ret:%d", ret);
		return ret;
	}

	ret = ivp_get_memory_section(plat_dev, ivp_devp);
	if (ret) {
		ivp_err("get memory section failed, ret:%d", ret);
		return ret;
	}

	ret = ivp_rdr_init(ivp_devp);
	if (ret) {
		ivp_err("rdr init failed, ret:%d", ret);
		ivp_free_memory_section(ivp_devp);
	}

	return ret;
}

void ivp_deinit_pri(struct ivp_device *ivp_devp)
{
	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return;
	}
	ivp_free_memory_section(ivp_devp);
	ivp_rdr_deinit();
}

int ivp_init_resethandler(const struct ivp_device *ivp_devp __attribute__((unused)))
{
	/* init code to remap address */
	g_iram = ioremap(REMAP_ADD, SIZE_16K);
	if (!g_iram) {
		ivp_err("Can't map ivp base address");
		return -ENODEV;
	}
	iowrite32(DEAD_FLAG, g_iram);

	return 0;
}

int ivp_check_resethandler(const struct ivp_device *ivp_devp __attribute__((unused)))
{
	/* check init code in remap address */
	int inited = 0;
	u32 flag = 0;

	if (g_iram)
		flag = ioread32(g_iram);
	if (flag != DEAD_FLAG)
		inited = 1;

	return inited;
}

void ivp_deinit_resethandler(const struct ivp_device *ivp_devp __attribute__((unused)))
{
	/* deinit remap address */
	if (g_iram) {
		iounmap(g_iram);
		g_iram = NULL;
	}
}

int ivp_sec_loadimage(const struct ivp_device *ivp_devp __attribute__((unused)))
{
	ivp_err("not support sec ivp");
	return -EINVAL;
}

void ivp_dev_hwa_enable(struct ivp_device *ivp_devp)
{
	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return;
	}
	/* enable apb gate clock, watdog, timer */
	ivp_info("ivp will enable hwa");
	ivp_reg_write(ivp_devp, IVP_REG_OFF_APB_GATE_CLOCK, IVP_APB_GATE_CLOCK_VAL);
	ivp_reg_write(ivp_devp, IVP_REG_OFF_TIMER_WDG_RST_DIS, IVP_TIMER_WDG_RST_DIS_VAL);
}

void ivp_hw_enable_reset(struct ivp_device *ivp_devp)
{
	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return;
	}
	ivp_reg_write(ivp_devp, IVP_REG_OFF_DSP_CORE_RESET_EN, RST_IVP32_PROCESSOR_EN);
	ivp_reg_write(ivp_devp, IVP_REG_OFF_DSP_CORE_RESET_EN, RST_IVP32_DEBUG_EN);
	ivp_reg_write(ivp_devp, IVP_REG_OFF_DSP_CORE_RESET_EN, RST_IVP32_JTAG_EN);
}

void ivp_set_qos_cfg(struct ivp_device *ivp_devp)
{
	ivp_info("ivp set qos cfg enter");
	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return;
	}

	/* ivp QoS reg */
	ivp_reg_write(ivp_devp, IVP_REG_OFF_IVP_SYSTEM_QOS_CFG, IVP_DSP_INSTR_ARQOS);
	/* noc ivp QoS reg */
	noc_ivp_reg_write(ivp_devp, IVP_CORE_RD_QOS_MODE, IVP_CORE_RD_QOS_MODE_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_CORE_RD_QOS_BANDWIDTH,
		IVP_CORE_RD_QOS_BANDWIDTH_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_CORE_RD_QOS_SATURATION,
		IVP_CORE_RD_QOS_SATURATION_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_CORE_RD_QOS_EXTCONTROL,
		IVP_CORE_RD_QOS_EXTCONTROL_VAL);

	noc_ivp_reg_write(ivp_devp, IVP_CORE_WR_QOS_MODE, IVP_CORE_WR_QOS_MODE_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_CORE_WR_QOS_BANDWIDTH,
		IVP_CORE_WR_QOS_BANDWIDTH_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_CORE_WR_QOS_SATURATION,
		IVP_CORE_WR_QOS_SATURATION_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_CORE_WR_QOS_EXTCONTROL,
		IVP_CORE_WR_QOS_EXTCONTROL_VAL);

	noc_ivp_reg_write(ivp_devp, IVP_IDMA_RD_QOS_MODE, IVP_IDMA_RD_QOS_MODE_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_IDMA_RD_QOS_BANDWIDTH,
		IVP_IDMA_RD_QOS_BANDWIDTH_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_IDMA_RD_QOS_SATURATION,
		IVP_IDMA_RD_QOS_SATURATION_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_IDMA_RD_QOS_EXTCONTROL,
		IVP_IDMA_RD_QOS_EXTCONTROL_VAL);

	noc_ivp_reg_write(ivp_devp, IVP_IDMA_WR_QOS_MODE, IVP_IDMA_WR_QOS_MODE_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_IDMA_WR_QOS_BANDWIDTH,
		IVP_IDMA_WR_QOS_BANDWIDTH_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_IDMA_WR_QOS_SATURATION,
		IVP_IDMA_WR_QOS_SATURATION_VAL);
	noc_ivp_reg_write(ivp_devp, IVP_IDMA_WR_QOS_EXTCONTROL,
		IVP_IDMA_WR_QOS_EXTCONTROL_VAL);

	noc_ivp_reg_write(ivp_devp, IVP32_QOS_PRIORITY, IVP_QOS_PRIORITY_VAL);
	noc_ivp_reg_write(ivp_devp, IVP32_QOS_MODE, IVP32_QOS_MODE_VAL);
}

int ivp_set_transitional_clk(struct ivp_device *ivp_devp,
	unsigned int pre_level, unsigned int cur_level)
{
	int ret = 0;

	const unsigned int transitional_table[TRANSITIONAL_SIZE_MAX][TRANSITIONAL_SIZE_MAX] = {
		{ 0, 0, (IVP_PLL0_TRANS_CLK), (IVP_PLL3_TRANS_CLK) },
		{ 0, 0, (IVP_PLL0_TRANS_CLK), 0 },
		{ 0, 0, 0,                    0 },
		{ 0, 0, 0,                    0 }
	};
	int row = ((cur_level > TRANSITIONAL_SIZE_MAX) || (cur_level < 1)) ?
		IVP_CLK_LEVEL_HIGH : cur_level;
	int col = ((pre_level > TRANSITIONAL_SIZE_MAX) || (pre_level < 1)) ?
		IVP_CLK_LEVEL_HIGH : pre_level;

	unsigned int transitional_clk = transitional_table[row - 1][col - 1];
	if (transitional_clk) {
		ivp_info("need to change transitional_clk %u", transitional_clk);
		ret = clk_set_rate(ivp_devp->clk, transitional_clk);
		if (ret)
			ivp_err("set transitional_clk fail, ret = %d", ret);
	}

	return ret;
}
