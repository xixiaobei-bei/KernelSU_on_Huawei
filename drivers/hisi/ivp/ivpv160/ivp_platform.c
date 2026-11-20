/*
 * This file implements ivp initialization and control functions
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "ivp_platform.h"
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include "ivp_reg.h"
#include "ivp_log.h"
#include "ivp_manager.h"

void ivp_fama_addr_32bit_2_64bit(struct ivp_device *ivp_devp)
{
	unsigned int ivp_remap_addr;
	unsigned int len;
	unsigned long ddr_addr;
	unsigned int icfg_addr_scr0;
	unsigned int icfg_addr_len0;
	unsigned int icfg_fama_addr_msb0;
	unsigned int icfg_fama_icfg_addr_dst0;
	unsigned int src_address_a;
	unsigned int dst_address_b;
	unsigned int reg_val;
	struct ivp_common *ivp_comm = NULL;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return;
	}
	ivp_comm = &ivp_devp->ivp_comm;
	ivp_remap_addr = (unsigned int)(ivp_comm->sects[DDR_SECTION_INDEX].acpu_addr <<
		IVP_MMAP_SHIFT);
	ddr_addr = ivp_comm->sects[DDR_SECTION_INDEX].acpu_addr << IVP_MMAP_SHIFT;
	len = (unsigned int)ivp_comm->ivp_meminddr_len;
	len = (len + SIZE_4K - 1) / SIZE_4K;

	icfg_addr_scr0 = ivp_remap_addr >> IVP_ADDR_SHIFT;
	icfg_addr_len0 = (ivp_remap_addr >> IVP_ADDR_SHIFT) + len;
	icfg_fama_addr_msb0 = (unsigned int)(ddr_addr >> IVP_DDR_ADDR_SHIFT);

	src_address_a = ivp_remap_addr >> IVP_ADDR_SHIFT;
	dst_address_b = (unsigned int)((ddr_addr & IVP_ADDR_AND_NUM) >> IVP_ADDR_SHIFT);
	if (src_address_a <= dst_address_b) {
		icfg_fama_icfg_addr_dst0 = dst_address_b - src_address_a;
	} else {
		ivp_warn("address a > address b");
		icfg_fama_icfg_addr_dst0 = ~(src_address_a - dst_address_b) + 1;
	}

	reg_val = icfg_fama_addr_msb0 << IVP_FAMA_ADDR_SHIFT | icfg_fama_icfg_addr_dst0;
	ivp_dbg("ivp:%x, %lx, 0x%x", ivp_remap_addr, ddr_addr, len);

	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_IVP_FAMA_CTRL0,
		IVP_FAMA_CTRL0_VAL);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_FAMA_ADDR_REMAP0_0,
		icfg_addr_scr0);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_FAMA_ADDR_REMAP0_1,
		icfg_addr_len0);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_FAMA_ADDR_REMAP0_2,
		reg_val);
}

int ivp_get_memory_section(struct platform_device *plat_dev,
	struct ivp_device *ivp_devp)
{
	int ret;
	int i;
	unsigned int size = 0;
	dma_addr_t dma_addr = 0;
	struct ivp_common *ivp_comm = NULL;

	loge_and_return_if_cond(-EINVAL, (!ivp_devp || !plat_dev),
		"invalid input param ivp_devp or plat_dev");
	ivp_comm = &ivp_devp->ivp_comm;
	ivp_comm->vaddr_memory = NULL;
	ret = of_property_read_u32(plat_dev->dev.of_node, OF_IVP_DYNAMIC_MEM, &size);
	if ((ret != 0) || (size == 0)) {
		ivp_err("get failed/not use dynamic mem, ret:%d, size:%u", ret, size);
		return -EINVAL;
	}
	ivp_comm->dynamic_mem_size = size;
	ivp_comm->ivp_meminddr_len = (int)size;

	ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_DYNAMIC_MEM_SEC_SIZE, &size);
	if ((ret != 0) || (size == 0)) {
		ivp_err("get failed/not use dynamic mem, ret:%d, size:%u", ret, size);
		return -EINVAL;
	}
	ivp_comm->dynamic_mem_section_size = size;
	if ((ivp_comm->dynamic_mem_section_size * (ivp_comm->sect_count - DDR_SECTION_INDEX)) !=
		ivp_comm->dynamic_mem_size) {
		ivp_err("dynamic_mem should be sect_count-3 times dynamic_mem_section");
		return -EINVAL;
	}

	/*lint -save -e598 -e648*/
	dma_set_mask_and_coherent(&ivp_comm->ivp_pdev->dev, DMA_BIT_MASK(64));
	/*lint -restore*/
	ivp_comm->vaddr_memory = dma_alloc_coherent(&ivp_comm->ivp_pdev->dev,
		ivp_comm->dynamic_mem_size, &dma_addr, GFP_KERNEL);
	if (!ivp_comm->vaddr_memory) {
		ivp_err("get vaddr_memory failed");
		return -ENOMEM;
	}

	for (i = DDR_SECTION_INDEX; i < ivp_comm->sect_count; i++) {
		if (i == DDR_SECTION_INDEX) {
			ivp_comm->sects[i].acpu_addr = dma_addr >> IVP_MMAP_SHIFT;
		} else {
			ivp_comm->sects[i].acpu_addr =
				((ivp_comm->sects[i - 1].acpu_addr << IVP_MMAP_SHIFT) +
				ivp_comm->sects[i - 1].len) >> IVP_MMAP_SHIFT;
			ivp_comm->sects[i].ivp_addr = ivp_comm->sects[i - 1].ivp_addr +
				ivp_comm->sects[i - 1].len;
		}
		ivp_comm->sects[i].len = ivp_comm->dynamic_mem_section_size;
		ivp_dbg("ivp sections 0x%lx", ivp_comm->sects[i].acpu_addr);
	}

	return ret;
}

void ivp_free_memory_section(struct ivp_device *ivp_devp)
{
	dma_addr_t dma_addr;
	struct ivp_common *ivp_comm = NULL;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return;
	}
	ivp_comm = &ivp_devp->ivp_comm;
	dma_addr = ivp_comm->sects[DDR_SECTION_INDEX].acpu_addr << IVP_MMAP_SHIFT;
	if (ivp_comm->vaddr_memory) {
		dma_free_coherent(&ivp_comm->ivp_pdev->dev,
			ivp_comm->dynamic_mem_size,
			ivp_comm->vaddr_memory, dma_addr);
		ivp_comm->vaddr_memory = NULL;
	}
}

static inline void ivp_hw_remap_ivp2ddr(struct ivp_device *ivp_devp,
	unsigned int ivp_addr, unsigned int len, unsigned long ddr_addr)
{
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_START_REMAP_ADDR,
		ivp_addr / SIZE_1MB);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_REMAP_LENGTH, len);
	ivp_reg_write(ivp_devp, ADDR_IVP_CFG_SEC_REG_DDR_REMAP_ADDR,
		(u32)(ddr_addr / SIZE_1MB)); /* reg is 32bit */
}

int ivp_remap_addr_ivp2ddr(struct ivp_device *ivp_devp,
	unsigned int ivp_addr, int len, unsigned long ddr_addr)
{
	loge_and_return_if_cond(-EINVAL, (!ivp_devp),
		"invalid input param ivp_devp");
	ivp_dbg("ivp_addr:%#x, len:%d, ddr_addr:%#lx",
		ivp_addr, len, ddr_addr);
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
	struct ivp_common *ivp_comm = &ivp_devp->ivp_comm;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -ENODEV;
	}
	/* 0.Enable the power */
	if ((ret = regulator_enable(ivp_devp->ivp_media1_regulator)) != 0) {
		ivp_err("ivp_media1_regulator enable failed %d", ret);
		return ret;
	}

	/* 1.Set Clock rate */
	if ((ret = clk_set_rate(ivp_comm->clk, (unsigned long)ivp_comm->clk_rate)) != 0) {
		ivp_err("set rate %#x fail, ret:%d", ivp_comm->clk_rate, ret);
		goto try_down_freq;
	}

	/* 2.Enable the clock */
	if ((ret = clk_prepare_enable(ivp_comm->clk)) != 0) {
		ivp_err("clk prepare enable failed, ret = %d", ret);
		goto try_down_freq;
	}
	goto normal_frq_success;

try_down_freq:
	ivp_info("try set core freq to: %ld", (unsigned long)ivp_comm->lowfrq_pd_clk_rate);
	if ((ret = clk_set_rate(ivp_comm->clk, (unsigned long)ivp_comm->lowfrq_pd_clk_rate)) != 0) {
		ivp_err("set low rate %#x fail, ret:%d", ivp_comm->lowfrq_pd_clk_rate, ret);
		goto err_clk_set_rate;
	}

	if ((ret = clk_prepare_enable(ivp_comm->clk)) != 0) {
		ivp_err("low clk prepare enable failed, ret = %d", ret);
		goto err_clk_prepare_enable;
	}

normal_frq_success:
	ivp_info("set core success to: %ld", clk_get_rate(ivp_comm->clk));
	/* 3.Enable the power */
	if ((ret = regulator_enable(ivp_comm->regulator)) != 0) {
		ivp_err("regularot enable failed %d", ret);
		goto err_regulator_enable_ivp;
	}
	ivp_reg_write(ivp_devp, IVP_REG_OFF_MEM_CTRL3, IVP_MEM_CTRL3_VAL);
	ivp_reg_write(ivp_devp, IVP_REG_OFF_MEM_CTRL4, IVP_MEM_CTRL4_VAL);

	return ret;

err_regulator_enable_ivp:
	clk_disable_unprepare(ivp_comm->clk);

err_clk_prepare_enable:
	if ((ret = clk_set_rate(ivp_comm->clk, (unsigned long)ivp_comm->lowfrq_pd_clk_rate)) != 0)
		ivp_err("err set lowfrq rate %#x fail %d", ivp_comm->lowfrq_pd_clk_rate, ret);

err_clk_set_rate:
	if ((ret = regulator_disable(ivp_devp->ivp_media1_regulator)) != 0)
		ivp_err("ivp_media1_regulator disable failed %d", ret);
	return -EINVAL;
}


int ivp_poweroff_pri(struct ivp_device *ivp_devp)
{
	int ret;
	struct ivp_common *ivp_comm = &ivp_devp->ivp_comm;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -EINVAL;
	}
	ivp_hw_runstall(ivp_devp, IVP_RUNSTALL_STALL);
	ivp_hw_enable_reset(ivp_devp);
	ret = regulator_disable(ivp_comm->regulator);
	if (ret)
		ivp_err("regulator power off failed %d", ret);

	clk_disable_unprepare(ivp_comm->clk);
	ret = clk_set_rate(ivp_comm->clk, (unsigned long)ivp_comm->lowfrq_pd_clk_rate);
	if (ret != 0)
		ivp_warn("set lfrq pd rate %#x fail, ret:%d", ivp_comm->lowfrq_pd_clk_rate, ret);

	ret = regulator_disable(ivp_devp->ivp_media1_regulator);
	if (ret)
		ivp_err("ivp_media1_regulator power off failed %d", ret);

	return ret;
}

int ivp_setup_regulator(struct platform_device *plat_dev,
	struct ivp_device *ivp_devp)
{
	struct regulator *ivp_media1_regulator = NULL;
	struct regulator *regulator = NULL;
	struct ivp_common *ivp_comm = NULL;

	loge_and_return_if_cond(-EINVAL, (!plat_dev || !ivp_devp),
		"invalid input param plat_dev or ivp_devp");
	ivp_comm = &ivp_devp->ivp_comm;
	regulator = devm_regulator_get(&plat_dev->dev, IVP_REGULATOR);
	if (IS_ERR(regulator)) {
		ivp_err("Get ivp regulator failed");
		return -ENODEV;
	} else {
		ivp_comm->regulator = regulator;
	}

	ivp_media1_regulator = devm_regulator_get(&plat_dev->dev,
		IVP_MEDIA_REGULATOR);
	if (IS_ERR(ivp_media1_regulator)) {
		ivp_err("Get ivp regulator failed");
		return -ENODEV;
	} else {
		ivp_devp->ivp_media1_regulator = ivp_media1_regulator;
	}

	return 0;
}

int ivp_setup_clk(struct platform_device *plat_dev,
	struct ivp_device *ivp_devp)
{
	int ret;
	u32 clk_rate;
	struct ivp_common *ivp_comm = NULL;

	loge_and_return_if_cond(-EINVAL, (!plat_dev || !ivp_devp),
		"invalid input param plat_dev or ivp_devp");
	ivp_comm = &ivp_devp->ivp_comm;
	ivp_comm->clk = devm_clk_get(&plat_dev->dev, OF_IVP_CLK_NAME);
	if (IS_ERR(ivp_comm->clk)) {
		ivp_err("get clk failed");
		return -ENODEV;
	}

	ret = of_property_read_u32(plat_dev->dev.of_node, OF_IVP_CLK_RATE_NAME,
		&clk_rate);
	if (ret) {
		ivp_err("get rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_comm->clk_rate = clk_rate;
	ivp_info("get clk rate: %u", clk_rate);

	ret = of_property_read_u32(plat_dev->dev.of_node,
		OF_IVP_LOWFREQ_CLK_RATE_NAME, &clk_rate);
	if (ret) {
		ivp_err("get rate failed, ret:%d", ret);
		return -ENOMEM;
	}
	ivp_comm->lowfrq_pd_clk_rate = clk_rate;
	ivp_info("get lowfrq pd clk rate: %u", clk_rate);

	return ret;
}

int ivp_change_clk(struct ivp_device *ivp_devp, unsigned int level)
{
	int ret;
	struct ivp_common *ivp_comm = &ivp_devp->ivp_comm;

	if (!ivp_devp) {
		ivp_err("invalid input param ivp_devp");
		return -EINVAL;
	}
	ivp_comm->clk_level = level;
	switch (ivp_comm->clk_level) {
	case IVP_CLK_LEVEL_LOW:
		ivp_info("ivp freq to low level");
		ivp_comm->clk_usrsetrate = IVP_CLKRATE_LOW;
		break;

	case IVP_CLK_LEVEL_MEDIUM:
		ivp_info("ivp freq to media level");
		ivp_comm->clk_usrsetrate = IVP_CLKRATE_MEDIUM;
		break;

	case IVP_CLK_LEVEL_HIGH:
		ivp_info("ivp freq to high level");
		ivp_comm->clk_usrsetrate = IVP_CLKRATE_HIGH;
		break;

	case IVP_CLK_LEVEL_DEFAULT:
	default:
		ivp_info("use default freq");
		ivp_comm->clk_usrsetrate = ivp_comm->clk_rate;
		break;
	}

	/* Set Clock rate */
	ivp_info("set clock rate");
	ret = clk_set_rate(ivp_comm->clk, (unsigned long)ivp_comm->clk_usrsetrate);
	if (ret == 0) {
		ivp_info("set core success to: %ld", clk_get_rate(ivp_comm->clk));
		return ret;
	}

	ivp_info("try set core freq to: %ld", (unsigned long)ivp_comm->lowfrq_pd_clk_rate);
	ret = clk_set_rate(ivp_comm->clk, (unsigned long)ivp_comm->lowfrq_pd_clk_rate);
	if (ret != 0)
		ivp_err("set low rate %#x fail, ret:%d", ivp_comm->lowfrq_pd_clk_rate, ret);

	return ret;
}

