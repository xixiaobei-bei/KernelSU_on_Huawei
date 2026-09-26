/*
 * slimbus is a kernel driver which is used to manager slimbus devices
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include "slimbus_utils.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include "linux/hisi/audio_log.h"

#include "asp_cfg.h"

/*lint -e838 -e715 -e573*/

#define SLIMBUS_CLK_24576 24576
#define SLIMBUS_CLK_21777 21777
#define CLK_ASP_SUBSYS_PERI 415000

#define LOG_TAG "slimbus_utils"

static int32_t slimbus_freq_cnt;
static bool slimbus_soc_div_freq_disable = false;
static struct mutex slimbus_freq_mutex;
static void __iomem *asp_reg_base_addr;
static int32_t clk_asp_subsys = 480000;

static void slimbus_reg_write(void __iomem *reg_addr, uint32_t value)
{
	writel(value, reg_addr);
}

static void slimbus_reg_set_bit(void __iomem *reg_addr, uint32_t offset)
{
	uint32_t value = readl(reg_addr);

	value |= (1 << offset);
	writel(value, reg_addr);
}

static void slimbus_reg_write_bits(void __iomem *reg_addr,
	uint32_t value, uint32_t mask)
{
	uint32_t val = readl(reg_addr);

	val &= (~mask);
	val |= (value & mask);
	writel(val, reg_addr);
}

uint32_t slimbus_utils_port_state_get(const void __iomem *slimbus_base_addr)
{
	uint32_t portstate;
	uint32_t result = 0;
	uint32_t port;

	if (!slimbus_base_addr) {
		AUDIO_LOGE("input addr is null");
		return 0;
	}

	for (port = 0; port < SLIMBUS_PORT_NUM; port++) {
		portstate = readl((slimbus_base_addr + SLIMBUS_PORT_BASE_ADDR) + (port*SLIMBUS_PORT_OFFSET));
		result |= ((portstate & SLIMBUS_PORT_ACTIVE) << port);
	}

	return result;
}

static uint32_t get_clk_asp_div(uint64_t target_freq)
{
	uint32_t param;

	/* target_freq = (param * clk_asp_subsys)  / (2 * pow(2, 27) */
	param = (unsigned int)((target_freq * 2 * 0x8000000) / clk_asp_subsys);

	return param;
}

void slimbus_utils_set_soc_div_freq_disable(bool div_disable)
{
	slimbus_soc_div_freq_disable = div_disable;
}

static bool get_slimbus_soc_div_freq_disable(void)
{
	return slimbus_soc_div_freq_disable;
}

void slimbus_utils_freq_request(void)
{
	uint32_t div_siobclk = get_clk_asp_div(SLIMBUS_CLK_24576);

	if (get_slimbus_soc_div_freq_disable())
		return;

	if (asp_reg_base_addr == NULL) {
		AUDIO_LOGE("slimbus_utils not init");
		return;
	}

	mutex_lock(&slimbus_freq_mutex);
	slimbus_freq_cnt++;

	/* When anyone request, Slimbus clk change to 24.576M */
	if (slimbus_freq_cnt == 1)
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK1_DIV_REG), div_siobclk);

	mutex_unlock(&slimbus_freq_mutex);
}

void slimbus_utils_freq_release(void)
{
	uint32_t div_siobclk = get_clk_asp_div(SLIMBUS_CLK_21777);

	if (get_slimbus_soc_div_freq_disable())
		return;

	if (asp_reg_base_addr == NULL) {
		AUDIO_LOGE("slimbus_utils not init");
		return;
	}

	mutex_lock(&slimbus_freq_mutex);
	if (slimbus_freq_cnt >= 1)
		slimbus_freq_cnt--;
	else
		AUDIO_LOGE("err, slimbus_freq_cnt: %d", slimbus_freq_cnt);

	/* When all req released, Slimbus clk change to 21.777 to avoid signal interference to GPS */
	if (slimbus_freq_cnt == 0)
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK1_DIV_REG), div_siobclk);

	mutex_unlock(&slimbus_freq_mutex);
}

uint32_t slimbus_utils_asp_state_get(const void __iomem *asppower_base_addr, uint32_t offset)
{
	uint32_t aspreg = 0;

	if (asppower_base_addr)
		aspreg = readl(asppower_base_addr + offset);

	return aspreg;
}

static void slimbus_io_config(const struct slimbus_device_info *dev, enum slimbus_rf_type rf, void __iomem *ioc_base_addr)
{
	/* For FPGA */
	if (rf == SLIMBUS_RF_6144) {
		/* IOMG slimbus clk */
		slimbus_reg_write((void __iomem *)((uintptr_t)ioc_base_addr + dev->slimbusclk_offset), 0x00000003);
		/* IOMG slimbus data */
		slimbus_reg_write((void __iomem *)((uintptr_t)ioc_base_addr + dev->slimbusdata_offset), 0x00000003);
		/* IOCG slimbus clk */
		slimbus_reg_write((void __iomem *)((uintptr_t)ioc_base_addr + dev->slimbusclk_cfg_offset), 0x12);
		/* IOCG slimbus data */
		slimbus_reg_write((void __iomem *)((uintptr_t)ioc_base_addr + dev->slimbusdata_cfg_offset), 0x13);
	} else {
		/* IOCG slimbus clk drv 6ma slew rate */
		slimbus_reg_write((void __iomem *)((uintptr_t)ioc_base_addr + dev->slimbusclk_cfg_offset), dev->slimbusclk_drv);
		/* IOCG slimbus data drv 6ma pu pd */
		slimbus_reg_write((void __iomem *)((uintptr_t)ioc_base_addr + dev->slimbusdata_cfg_offset), dev->slimbusdata_drv);
	}
}

static void slimbus_clk_config(enum slimbus_rf_type rf, uint32_t div_siobclk)
{
	/* clock select and div param set */
	if (rf == SLIMBUS_RF_24576) {
		/* SRC 480M divided to 43.55556M to avoid signal interference to GPS */
		if (!get_slimbus_soc_div_freq_disable())
			slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK1_DIV_REG), div_siobclk);

		if (clk_asp_subsys == CLK_ASP_SUBSYS_PERI) {
			slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK_SEL_REG), 0x700060);
		} else {
			/* Sel 480M_clk as clk src */
			slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK_SEL_REG), 0x700070);
		}

		/* bit24:gt_clk_ppll0_slimbus disable */
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_GATE_DIS_REG), 0x1000000);
	} else if (rf == SLIMBUS_RF_6144) {
		if (!get_slimbus_soc_div_freq_disable())
			slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK1_DIV_REG), 0x68db8c);
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CLK_SEL_REG), 0x600060);
	}
}

void slimbus_utils_module_enable(const struct slimbus_device_info *dev, bool enable)
{
	enum slimbus_rf_type rf = SLIMBUS_RF_MAX;
	void __iomem *ioc_base_addr = 0;
	uint32_t div_siobclk;

	if (!dev) {
		AUDIO_LOGE("dev is null");
		return;
	}

	if (asp_reg_base_addr == NULL) {
		AUDIO_LOGE("slimbus_utils not init");
		return;
	}

	rf = dev->rf;
	div_siobclk = get_clk_asp_div(SLIMBUS_CLK_21777);
	AUDIO_LOGI("get slimbus clk 21777, div is 0x%x", div_siobclk);

	if (enable) {
		ioc_base_addr = ioremap(IOC_BASE_ADDR, IOC_REG_SIZE);
		if (ioc_base_addr == NULL) {
			AUDIO_LOGE("ioc_base_addr remap error");
			return;
		}

		slimbus_io_config(dev, rf, ioc_base_addr);

		iounmap(ioc_base_addr);
		ioc_base_addr = NULL;

		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_RST_CTRLDIS_REG), 0xffffffff);
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_GATE_EN_REG), 0x1030800);
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_CG_EN_REG), 0x0fff0fff);

		/* bit2:gt_siobclk_div bit7:gt_asp_tcxo_div bit8:gt_slimbus_clk_div */
		slimbus_reg_write_bits((asp_reg_base_addr + ASP_CFG_R_GATE_CLKDIV_EN_REG), 0x184, 0x184);
		slimbus_clk_config(rf, div_siobclk);

		/* copyright information */
		slimbus_reg_write((asp_reg_base_addr + ASP_CFG_R_SLIMBUS_ID), 0xaaaa);

		/* interupt enable */
		slimbus_reg_set_bit((asp_reg_base_addr + ASP_CFG_R_INTR_NS_EN_REG), ASP_CFG_SLIMBUS_INTR_MASK_OFFSET);
	} else {
		slimbus_reg_write(asp_reg_base_addr + ASP_CFG_R_GATE_DIS_REG, 0x30000);
	}
}

void slimbus_utils_init(void __iomem *addr, int32_t src_freq)
{
	if (addr)
		asp_reg_base_addr = addr;

	clk_asp_subsys = src_freq;
	mutex_init(&slimbus_freq_mutex);
}

void slimbus_utils_deinit(void)
{
	asp_reg_base_addr = NULL;
	mutex_destroy(&slimbus_freq_mutex);
}

