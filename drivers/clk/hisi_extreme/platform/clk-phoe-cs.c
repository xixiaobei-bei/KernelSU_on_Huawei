/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include "../clk.h"
#include "clk-phoe-cs.h"
#include "../clk-mclk.h"
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>

static const struct fixed_rate_clock fixed_clks[] = {
	{ CLKIN_SYS, "clkin_sys", NULL, 0, 38400000 },
	{ CLKIN_REF, "clkin_ref", NULL, 0, 32764 },
	{ CLK_FLL_SRC, "clk_fll_src", NULL, 0, 180000000 },
	{ CLK_PPLL0, "clk_ppll0", NULL, 0, 1660000000 },
	{ CLK_PPLL1, "clk_ppll1", NULL, 0, 1866000000 },
	{ CLK_PPLL2, "clk_ppll2", NULL, 0, 1920000000 },
	{ CLK_PPLL3, "clk_ppll3", NULL, 0, 1200000000 },
	{ CLK_PPLL5, "clk_ppll5", NULL, 0, 1500000000 },
	{ CLK_SCPLL, "clk_scpll", NULL, 0, 393216000 },
	{ CLK_PPLL6, "clk_ppll6", NULL, 0, 1720000000 },
	{ CLK_PPLL7, "clk_ppll7", NULL, 0, 1290000000 },
	{ CLK_SPLL, "clk_spll", NULL, 0, 1572864000 },
	{ CLK_MODEM_BASE, "clk_modem_base", NULL, 0, 49152000 },
	{ CLK_LBINTPLL, "clk_lbintpll", NULL, 0, 100000000 },
	{ CLK_LBINTPLL_1, "clk_lbintpll_1", NULL, 0, 49146757 },
	{ CLK_PPLL_PCIE, "clk_ppll_pcie", NULL, 0, 100000000 },
	{ PCLK, "pclk", NULL, 0, 20000000 },
	{ CLK_UART0_DBG, "clk_uart0_dbg", NULL, 0, 19200000 },
	{ CLK_UART6, "clk_uart6", NULL, 0, 19200000 },
	{ OSC32K, "osc32k", NULL, 0, 32764 },
	{ OSC19M, "osc19m", NULL, 0, 19200000 },
	{ CLK_480M, "clk_480m", NULL, 0, 480000000 },
	{ CLK_INVALID, "clk_invalid", NULL, 0, 10000000 },
	{ CLK_FPGA_1P92, "clk_fpga_1p92", NULL, 0, 1920000 },
	{ CLK_FPGA_2M, "clk_fpga_2m", NULL, 0, 2000000 },
	{ CLK_FPGA_10M, "clk_fpga_10m", NULL, 0, 10000000 },
	{ CLK_FPGA_19M, "clk_fpga_19m", NULL, 0, 19200000 },
	{ CLK_FPGA_20M, "clk_fpga_20m", NULL, 0, 20000000 },
	{ CLK_FPGA_24M, "clk_fpga_24m", NULL, 0, 24000000 },
	{ CLK_FPGA_26M, "clk_fpga_26m", NULL, 0, 26000000 },
	{ CLK_FPGA_27M, "clk_fpga_27m", NULL, 0, 27000000 },
	{ CLK_FPGA_32M, "clk_fpga_32m", NULL, 0, 32000000 },
	{ CLK_FPGA_40M, "clk_fpga_40m", NULL, 0, 40000000 },
	{ CLK_FPGA_48M, "clk_fpga_48m", NULL, 0, 48000000 },
	{ CLK_FPGA_50M, "clk_fpga_50m", NULL, 0, 50000000 },
	{ CLK_FPGA_57M, "clk_fpga_57m", NULL, 0, 57000000 },
	{ CLK_FPGA_60M, "clk_fpga_60m", NULL, 0, 60000000 },
	{ CLK_FPGA_64M, "clk_fpga_64m", NULL, 0, 64000000 },
	{ CLK_FPGA_80M, "clk_fpga_80m", NULL, 0, 80000000 },
	{ CLK_FPGA_100M, "clk_fpga_100m", NULL, 0, 100000000 },
	{ CLK_FPGA_160M, "clk_fpga_160m", NULL, 0, 160000000 },
};

static const struct fixed_factor_clock fixed_factor_clks[] = {
	{ CLK_SYS_INI, "clk_sys_ini", "clkin_sys", 1, 2, 0 },
	{ CLK_DIV_SYSBUS, "clk_div_sysbus", "clk_gate_ppll0", 1, 7, 0 },
	{ CLK_PPLL_EPS, "clk_ppll_eps", "clk_gate_ppll2", 1, 1, 0 },
	{ CLK_GATE_WD0_HIGH, "clk_gate_wd0_high", "clk_div_cfgbus", 1, 1, 0 },
	{ PCLK_GATE_DSI0, "pclk_gate_dsi0", "clk_div_cfgbus", 1, 1, 0 },
	{ PCLK_GATE_DSI1, "pclk_gate_dsi1", "clk_div_cfgbus", 1, 1, 0 },
	{ CLK_FACTOR_TCXO, "clk_factor_tcxo", "clk_sys_ini", 1, 4, 0 },
	{ CLK_GATE_TIMER5_A, "clk_gate_timer5_a", "clk_mux_timer5_a", 1, 1, 0 },
	{ ATCLK, "atclk", "clk_atdvfs", 1, 1, 0 },
	{ CLK_DIV_CSSYSDBG, "clk_div_cssysdbg", "autodiv_sysbus", 1, 1, 0 },
	{ CLK_GATE_CSSYSDBG, "clk_gate_cssysdbg", "clk_div_dmabus", 1, 1, 0 },
	{ NPU_PLL6_FIX, "npu_pll6_fix", "clk_gate_ppll6", 1, 4, 0 },
	{ CLK_GATE_DMA_IOMCU, "clk_gate_dma_iomcu", "clk_fll_src", 1, 4, 0 },
	{ CLK_SD_SYS, "clk_sd_sys", "clk_sd_sys_gt", 1, 6, 0 },
	{ CLK_SDIO_SYS, "clk_sdio_sys", "clk_sdio_sys_gt", 1, 6, 0 },
	{ CLK_DIV_A53HPM, "clk_div_a53hpm", "clk_a53hpm_andgt", 1, 4, 0 },
	{ CLK_DIV_320M, "clk_div_320m", "clk_320m_pll_gt", 1, 5, 0 },
	{ PCLK_GATE_UART0, "pclk_gate_uart0", "clk_gate_uart0", 1, 1, 0 },
	{ CLK_FACTOR_UART0, "clk_factor_uart0", "clk_mux_uart0", 1, 1, 0 },
	{ CLK_FACTOR_USB3PHY_PLL, "clk_factor_usb3phy_pll", "clk_gate_ppll0", 1, 60, 0 },
	{ CLKIN_SYS_DIV, "clkin_sys_div", "clkin_sys", 1, 2, 0 },
	{ CLK_GATE_ABB_USB, "clk_gate_abb_usb", "clk_gate_usb_tcxo_en", 1, 1, 0 },
	{ CLK_GATE_UFSPHY_REF, "clk_gate_ufsphy_ref", "clkin_sys", 1, 1, 0 },
	{ CLK_GATE_UFSIO_REF, "clk_gate_ufsio_ref", "clkin_sys", 1, 1, 0 },
	{ CLK_GATE_BLPWM, "clk_gate_blpwm", "clk_fll_src", 1, 2, 0 },
	{ CLK_SYSCNT_DIV, "clk_syscnt_div", "clk_sys_ini", 1, 10, 0 },
	{ CLK_GATE_GPS_REF, "clk_gate_gps_ref", "clk_mux_gps_ref", 1, 1, 0 },
	{ CLK_GATE_LDI0, "clk_gate_ldi0", "clk_sys_ini", 1, 1, 0 },
	{ EPS_VOLT_HIGH, "eps_volt_high", "peri_volt_hold", 1, 1, 0 },
	{ EPS_VOLT_MIDDLE, "eps_volt_middle", "peri_volt_middle", 1, 1, 0 },
	{ EPS_VOLT_LOW, "eps_volt_low", "peri_volt_low", 1, 1, 0 },
	{ VENC_VOLT_HOLD, "venc_volt_hold", "peri_volt_hold", 1, 1, 0 },
	{ VDEC_VOLT_HOLD, "vdec_volt_hold", "peri_volt_hold", 1, 1, 0 },
	{ EDC_VOLT_HOLD, "edc_volt_hold", "peri_volt_hold", 1, 1, 0 },
	{ EFUSE_VOLT_HOLD, "efuse_volt_hold", "peri_volt_middle", 1, 1, 0 },
	{ LDI0_VOLT_HOLD, "ldi0_volt_hold", "peri_volt_hold", 1, 1, 0 },
	{ HISE_VOLT_HOLD, "hise_volt_hold", "peri_volt_hold", 1, 1, 0 },
	{ CLK_FIX_DIV_DPCTRL, "clk_fix_div_dpctrl", "clk_gt_dpctrl_16m", 1, 10, 0 },
	{ CLK_ISP_SNCLK_FAC, "clk_isp_snclk_fac", "clk_isp_snclk_angt", 1, 14, 0 },
	{ CLK_FACTOR_RXDPHY, "clk_factor_rxdphy", "clk_andgt_rxdphy", 1, 6, 0 },
	{ CLK_GATE_I2C0, "clk_gate_i2c0", "clk_fll_src", 1, 2, 0 },
	{ CLK_GATE_I2C1, "clk_gate_i2c1", "clk_i2c1_gate_iomcu", 1, 2, 0 },
	{ CLK_GATE_I2C2, "clk_gate_i2c2", "clk_fll_src", 1, 2, 0 },
	{ CLK_GATE_SPI0, "clk_gate_spi0", "clk_fll_src", 1, 2, 0 },
	{ CLK_FAC_180M, "clk_fac_180m", "clk_ppll0", 1, 8, 0 },
	{ CLK_GATE_IOMCU_PERI0, "clk_gate_iomcu_peri0", "clk_spll", 1, 1, 0 },
	{ CLK_GATE_SPI2, "clk_gate_spi2", "clk_fll_src", 1, 2, 0 },
	{ CLK_GATE_UART3, "clk_gate_uart3", "clk_gate_iomcu_peri0", 1, 8, 0 },
	{ CLK_GATE_UART8, "clk_gate_uart8", "clk_gate_iomcu_peri0", 1, 8, 0 },
	{ CLK_GATE_UART7, "clk_gate_uart7", "clk_gate_iomcu_peri0", 1, 8, 0 },
	{ AUTODIV_ISP_DVFS, "autodiv_isp_dvfs", "autodiv_sysbus", 1, 1, 0 },
};

static const struct gate_clock crgctrl_gate_clks[] = {
	{ CLK_GATE_PPLL0_MEDIA, "clk_gate_ppll0_media", "clk_gate_ppll0", 0x410, 0x40000000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ppll0_media" },
	{ CLK_GATE_PPLL2_MEDIA, "clk_gate_ppll2_media", "clk_gate_ppll2", 0x410, 0x8000000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ppll2_media" },
	{ CLK_GATE_PPLL3_MEDIA, "clk_gate_ppll3_media", "clk_gate_ppll3", 0x410, 0x10000000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ppll3_media" },
	{ CLK_GATE_PPLL7_MEDIA, "clk_gate_ppll7_media", "clk_gate_ppll7", 0x410, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ppll7_media" },
	{ PCLK_GPIO0, "pclk_gpio0", "clk_div_cfgbus", 0x10, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio0" },
	{ PCLK_GPIO1, "pclk_gpio1", "clk_div_cfgbus", 0x10, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio1" },
	{ PCLK_GPIO2, "pclk_gpio2", "clk_div_cfgbus", 0x10, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio2" },
	{ PCLK_GPIO3, "pclk_gpio3", "clk_div_cfgbus", 0x10, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio3" },
	{ PCLK_GPIO4, "pclk_gpio4", "clk_div_cfgbus", 0x10, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio4" },
	{ PCLK_GPIO5, "pclk_gpio5", "clk_div_cfgbus", 0x10, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio5" },
	{ PCLK_GPIO6, "pclk_gpio6", "clk_div_cfgbus", 0x10, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio6" },
	{ PCLK_GPIO7, "pclk_gpio7", "clk_div_cfgbus", 0x10, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio7" },
	{ PCLK_GPIO8, "pclk_gpio8", "clk_div_cfgbus", 0x10, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio8" },
	{ PCLK_GPIO9, "pclk_gpio9", "clk_div_cfgbus", 0x10, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio9" },
	{ PCLK_GPIO10, "pclk_gpio10", "clk_div_cfgbus", 0x10, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio10" },
	{ PCLK_GPIO11, "pclk_gpio11", "clk_div_cfgbus", 0x10, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio11" },
	{ PCLK_GPIO12, "pclk_gpio12", "clk_div_cfgbus", 0x10, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio12" },
	{ PCLK_GPIO13, "pclk_gpio13", "clk_div_cfgbus", 0x10, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio13" },
	{ PCLK_GPIO14, "pclk_gpio14", "clk_div_cfgbus", 0x10, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio14" },
	{ PCLK_GPIO15, "pclk_gpio15", "clk_div_cfgbus", 0x10, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio15" },
	{ PCLK_GPIO16, "pclk_gpio16", "clk_div_cfgbus", 0x10, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio16" },
	{ PCLK_GPIO17, "pclk_gpio17", "clk_div_cfgbus", 0x10, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio17" },
	{ PCLK_GPIO18, "pclk_gpio18", "clk_div_cfgbus", 0x10, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio18" },
	{ PCLK_GPIO19, "pclk_gpio19", "clk_div_cfgbus", 0x10, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio19" },
	{ PCLK_GATE_WD0_HIGH, "pclk_gate_wd0_high", "clk_div_cfgbus", 0x20, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_wd0_high" },
	{ PCLK_GATE_WD0, "pclk_gate_wd0", "clk_mux_wd0", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_wd0" },
	{ PCLK_GATE_WD1, "pclk_gate_wd1", "clk_div_cfgbus", 0x20, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_wd1" },
	{ CLK_GATE_CODECSSI, "clk_gate_codecssi", "codeccssi_mux", 0x020, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_codecssi" },
	{ PCLK_GATE_CODECSSI, "pclk_gate_codecssi", "clk_div_cfgbus", 0x020, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_codecssi" },
	{ CLK_GATE_MMC_USBDP, "clk_gate_mmc_usbdp", "clk_div_mmc_usbdp", 0x040, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mmc_usbdp" },
	{ PCLK_GATE_IOC, "pclk_gate_ioc", "clk_sys_ini", 0x20, 0x2000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ioc" },
	{ PCLK_GATE_MMC1_PCIE, "pclk_gate_mmc1_pcie", "pclk_div_mmc1_pcie", 0x30, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_mmc1_pcie" },
	{ CLK_ATDVFS, "clk_atdvfs", "clk_div_cssysdbg", 0, 0x0, 0,
		NULL, 1, { 0, 0, 0 }, { 0, 1, 2, 3 }, 3, 19, 0, "clk_atdvfs" },
	{ ACLK_GATE_PERF_STAT, "aclk_gate_perf_stat", "clk_div_dmabus", 0x040, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_perf_stat" },
	{ PCLK_GATE_PERF_STAT, "pclk_gate_perf_stat", "clk_div_dmabus", 0x040, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_perf_stat" },
	{ CLK_GATE_PERF_STAT, "clk_gate_perf_stat", "clk_div_perf_stat", 0x040, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_perf_stat" },
	{ CLK_GATE_DMAC, "clk_gate_dmac", "clk_div_sysbus", 0x30, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dmac" },
	{ CLK_GATE_SOCP_ACPU, "clk_gate_socp_acpu", "clk_div_dmabus", 0x10, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_socp_acpu" },
	{ CLK_GATE_SOCP_DEFLATE, "clk_gate_socp_deflate", "clk_div_socp_deflate", 0, 0x0, 0,
		"clk_socp_acpu", 0, {0}, {0}, 0, 0, 0, "clk_socp_deflat" },
	{ CLK_GATE_TCLK_SOCP, "clk_gate_tclk_socp", "clk_sys_ini", 0, 0x0, 0,
		"clk_socp_acpu", 0, {0}, {0}, 0, 0, 0, "tclk_socp" },
	{ CLK_GATE_IPF, "clk_gate_ipf", "clk_div_sysbus", 0x0, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ipf" },
	{ CLK_GATE_IPF_PSAM, "clk_gate_ipf_psam", "clk_div_sysbus", 0x0, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ipf_psam" },
	{ CLK_GATE_VCODECBUS2DDR, "clk_gate_vcodecbus2ddr", "clk_div_vcodecbus", 0x0, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vcodecbus2" },
	{ CLK_GATE_SD, "clk_gate_sd", "clk_mux_sd_sys", 0x40, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 2, 1, "clk_sd" },
	{ CLK_GATE_UART1, "clk_gate_uart1", "clk_mux_uarth", 0x20, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart1" },
	{ CLK_GATE_UART4, "clk_gate_uart4", "clk_mux_uarth", 0x20, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart4" },
	{ PCLK_GATE_UART1, "pclk_gate_uart1", "clk_mux_uarth", 0x20, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart1" },
	{ PCLK_GATE_UART4, "pclk_gate_uart4", "clk_mux_uarth", 0x20, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart4" },
	{ CLK_GATE_UART2, "clk_gate_uart2", "clk_mux_uartl", 0x20, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart2" },
	{ CLK_GATE_UART5, "clk_gate_uart5", "clk_mux_uartl", 0x20, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart5" },
	{ PCLK_GATE_UART2, "pclk_gate_uart2", "clk_mux_uartl", 0x20, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart2" },
	{ PCLK_GATE_UART5, "pclk_gate_uart5", "clk_mux_uartl", 0x20, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart5" },
	{ CLK_GATE_UART0, "clk_gate_uart0", "clk_mux_uart0", 0x20, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart0" },
	{ CLK_GATE_I2C3, "clk_gate_i2c3", "clk_mux_i2c", 0x20, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c3" },
	{ CLK_GATE_I2C4, "clk_gate_i2c4", "clk_mux_i2c", 0x20, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c4" },
	{ CLK_GATE_I2C6_ACPU, "clk_gate_i2c6_acpu", "clk_mux_i2c", 0x10, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c6_acpu" },
	{ CLK_GATE_I2C7, "clk_gate_i2c7", "clk_mux_i2c", 0x10, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c7" },
	{ PCLK_GATE_I2C3, "pclk_gate_i2c3", "clk_mux_i2c", 0x20, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c3" },
	{ PCLK_GATE_I2C4, "pclk_gate_i2c4", "clk_mux_i2c", 0x20, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c4" },
	{ PCLK_GATE_I2C6_ACPU, "pclk_gate_i2c6_acpu", "clk_mux_i2c", 0x10, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c6_acpu" },
	{ PCLK_GATE_I2C7, "pclk_gate_i2c7", "clk_mux_i2c", 0x10, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c7" },
	{ CLK_GATE_I3C4, "clk_gate_i3c4", "clk_div_i3c4", 0x470, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i3c4" },
	{ PCLK_GATE_I3C4, "pclk_gate_i3c4", "clk_div_i3c4", 0x470, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i3c4" },
	{ CLK_GATE_SPI1, "clk_gate_spi1", "clk_mux_spi", 0x20, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spi1" },
	{ CLK_GATE_SPI4, "clk_gate_spi4", "clk_mux_spi", 0x40, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spi4" },
	{ PCLK_GATE_SPI1, "pclk_gate_spi1", "clk_mux_spi", 0x20, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi1" },
	{ PCLK_GATE_SPI4, "pclk_gate_spi4", "clk_mux_spi", 0x40, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi4" },
	{ CLK_GATE_USB3OTG_REF, "clk_gate_usb3otg_ref", "clk_sys_ini", 0x050, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_usb3otg_ref" },
	{ CLK_GATE_AO_ASP, "clk_gate_ao_asp", "clk_div_ao_asp", 0x0, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ao_asp" },
	{ PCLK_GATE_PCTRL, "pclk_gate_pctrl", "clk_div_ptp", 0x20, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pctrl" },
	{ CLK_GATE_PWM, "clk_gate_pwm", "clk_div_ptp", 0x20, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pwm" },
	{ PCLK_GATE_BLPWM_PERI, "pclk_gate_blpwm_peri", "clk_blpwm_peri_div", 0x20, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_blpwm_peri" },
	{ CLK_GATE_MDM2GPS0, "clk_gate_mdm2gps0", "clk_modem_base", 0x30, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mdm2gps0" },
	{ CLK_GATE_MDM2GPS1, "clk_gate_mdm2gps1", "clk_modem_base", 0x40, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mdm2gps1" },
	{ CLK_GATE_MDM2GPS2, "clk_gate_mdm2gps2", "clk_modem_base", 0x420, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mdm2gps2" },
	{ ABB_AUDIO_EN0, "abb_audio_en0", "clk_gate_abb_192", 0x30, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "abb_audio_en0" },
	{ ABB_AUDIO_EN1, "abb_audio_en1", "clk_gate_abb_192", 0x30, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "abb_audio_en1" },
	{ ABB_AUDIO_GT_EN0, "abb_audio_gt_en0", "abb_audio_en0", 0x30, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "abb_audio_gt0" },
	{ ABB_AUDIO_GT_EN1, "abb_audio_gt_en1", "abb_audio_en1", 0x40, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "abb_audio_gt1" },
	{ PERI_VOLT_HOLD, "peri_volt_hold", "clk_sys_ini", 0, 0x0, 0,
		NULL, 1, { 0, 0, 0 }, { 0, 1, 2, 3 }, 3, 18, 0, "peri_volt_hold" },
	{ PERI_VOLT_MIDDLE, "peri_volt_middle", "clk_sys_ini", 0, 0x0, 0,
		NULL, 1, { 0, 0 }, { 0, 1, 2 }, 2, 22, 0, "peri_volt_middle" },
	{ PERI_VOLT_LOW, "peri_volt_low", "clk_sys_ini", 0, 0x0, 0,
		NULL, 1, {0}, { 0, 1 }, 1, 28, 0, "peri_volt_low" },
	{ CLK_GATE_DPCTRL_16M, "clk_gate_dpctrl_16m", "clk_div_dpctrl_16m", 0x470, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dpctrl_16m" },
	{ CLK_GATE_ISP_I2C_MEDIA, "clk_gate_isp_i2c_media", "clk_div_isp_i2c", 0x30, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_i2c_media" },
	{ CLK_GATE_ISP_SNCLK0, "clk_gate_isp_snclk0", "clk_isp_snclk_mux0", 0x50, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk0" },
	{ CLK_GATE_ISP_SNCLK1, "clk_gate_isp_snclk1", "clk_isp_snclk_mux1", 0x50, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk1"},
	{ CLK_GATE_ISP_SNCLK2, "clk_gate_isp_snclk2", "clk_isp_snclk_mux2", 0x50, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk2" },
	{ CLK_GATE_ISP_SNCLK3, "clk_gate_isp_snclk3", "clk_isp_snclk_mux3", 0x40, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk3" },
	{ CLK_GATE_RXDPHY0_CFG, "clk_gate_rxdphy0_cfg", "clk_mux_rxdphy_cfg", 0x030, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy0_cfg" },
	{ CLK_GATE_RXDPHY1_CFG, "clk_gate_rxdphy1_cfg", "clk_mux_rxdphy_cfg", 0x030, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy1_cfg" },
	{ CLK_GATE_RXDPHY2_CFG, "clk_gate_rxdphy2_cfg", "clk_mux_rxdphy_cfg", 0x030, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy2_cfg" },
	{ CLK_GATE_RXDPHY3_CFG, "clk_gate_rxdphy3_cfg", "clk_mux_rxdphy_cfg", 0x030, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy3_cfg" },
	{ CLK_GATE_TXDPHY0_CFG, "clk_gate_txdphy0_cfg", "clk_sys_ini", 0x030, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_txdphy0_cfg" },
	{ CLK_GATE_TXDPHY0_REF, "clk_gate_txdphy0_ref", "clk_sys_ini", 0x030, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_txdphy0_ref" },
	{ CLK_GATE_TXDPHY1_CFG, "clk_gate_txdphy1_cfg", "clk_sys_ini", 0x030, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_txdphy1_cfg" },
	{ CLK_GATE_TXDPHY1_REF, "clk_gate_txdphy1_ref", "clk_sys_ini", 0x030, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_txdphy1_ref" },
	{ PCLK_GATE_LOADMONITOR, "pclk_gate_loadmonitor", "clk_div_cfgbus", 0x20, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_loadmonitor" },
	{ CLK_GATE_LOADMONITOR, "clk_gate_loadmonitor", "clk_div_loadmonitor", 0x20, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor" },
	{ PCLK_GATE_LOADMONITOR_L, "pclk_gate_loadmonitor_l", "clk_div_cfgbus", 0x20, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_loadmonitor_l" },
	{ CLK_GATE_LOADMONITOR_L, "clk_gate_loadmonitor_l", "clk_div_loadmonitor", 0x20, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor_l" },
	{ CLK_GATE_MEDIA_TCXO, "clk_gate_media_tcxo", "clk_sys_ini", 0x40, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_media_tcxo" },
	{ CLK_GATE_AO_HIFD, "clk_gate_ao_hifd", "clk_div_ao_hifd", 0x50, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ao_hifd" },
	{ AUTODIV_ISP, "autodiv_isp", "autodiv_isp_dvfs", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "autodiv_isp" },
	{ CLK_GATE_ATDIV_MMC0, "clk_gate_atdiv_mmc0", "clk_div_mmc0bus", 0x410, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_mmc0" },
	{ CLK_GATE_ATDIV_DMA, "clk_gate_atdiv_dma", "clk_div_dmabus", 0x410, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_dma" },
	{ CLK_GATE_ATDIV_CFG, "clk_gate_atdiv_cfg", "clk_div_cfgbus", 0x410, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_cfg" },
	{ CLK_GATE_ATDIV_SYS, "clk_gate_atdiv_sys", "clk_div_sysbus", 0x410, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_sys" },
};

static const struct scgt_clock crgctrl_scgt_clks[] = {
	{ CLK_ANDGT_MMC_USBDP, "clk_andgt_mmc_usbdp", "clk_gate_ppll0",
		0x70C, 2, CLK_GATE_HIWORD_MASK, "clk_mmc_usbdp_andgt" },
	{ AUTODIV_SYSBUS, "autodiv_sysbus", "clk_div_sysbus",
		0x404, 5, CLK_GATE_HIWORD_MASK, "autodiv_sysbus" },
	{ AUTODIV_EMMC0BUS, "autodiv_emmc0bus", "autodiv_sysbus",
		0x404, 1, CLK_GATE_HIWORD_MASK, "autodiv_emmc0bus" },
	{ PCLK_ANDGT_MMC1_PCIE, "pclk_andgt_mmc1_pcie", "clk_div_320m",
		0xf8, 13, CLK_GATE_HIWORD_MASK, "pclk_pcie_andgt" },
	{ CLK_PERF_DIV_GT, "clk_perf_div_gt", "clk_div_320m",
		0xF0, 3, CLK_GATE_HIWORD_MASK, "clk_perf_gt" },
	{ CLK_SOCP_DEFLATE_GT, "clk_socp_deflate_gt", "clk_gate_ppll0",
		0x0fc, 9, CLK_GATE_HIWORD_MASK, "clk_socp_def_gt" },
	{ CLK_GATE_TIME_STAMP_GT, "clk_gate_time_stamp_gt", "clk_sys_ini",
		0xF0, 1, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_timestp_gt" },
	{ CLK_GATE_VCODECBUS_GT, "clk_gate_vcodecbus_gt", "clk_mux_vcodecbus",
		0x0F0, 8, CLK_GATE_HIWORD_MASK, "clk_vcodbus_gt" },
	{ CLK_ANDGT_SD, "clk_andgt_sd", "clk_mux_sd_pll",
		0xF4, 3, CLK_GATE_HIWORD_MASK, "clk_sd_gt" },
	{ CLK_SD_SYS_GT, "clk_sd_sys_gt", "clk_sys_ini",
		0xF4, 5, CLK_GATE_HIWORD_MASK, "clk_sd_sys_gt" },
	{ CLK_A53HPM_ANDGT, "clk_a53hpm_andgt", "clk_mux_a53hpm",
		0x0F4, 7, CLK_GATE_HIWORD_MASK, "clk_a53hpm_gt" },
	{ CLK_320M_PLL_GT, "clk_320m_pll_gt", "clk_mux_320m",
		0xF8, 10, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "gt_clk_320m_pll" },
	{ CLK_ANDGT_UARTH, "clk_andgt_uarth", "clk_div_320m",
		0xF4, 11, CLK_GATE_HIWORD_MASK, "clkgt_uarth" },
	{ CLK_ANDGT_UARTL, "clk_andgt_uartl", "clk_div_320m",
		0xF4, 10, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clkgt_uartl" },
	{ CLK_ANDGT_UART0, "clk_andgt_uart0", "clk_div_320m",
		0xF4, 9, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clkgt_uart0" },
	{ CLK_ANDGT_I3C4, "clk_andgt_i3c4", "clk_div_320m",
		0x70C, 5, CLK_GATE_HIWORD_MASK, "clkgt_i3c4" },
	{ CLK_ANDGT_SPI, "clk_andgt_spi", "clk_div_320m",
		0xF4, 13, CLK_GATE_HIWORD_MASK, "clkgt_spi" },
	{ CLK_DIV_AO_ASP_GT, "clk_div_ao_asp_gt", "clk_mux_ao_asp",
		0xF4, 4, CLK_GATE_HIWORD_MASK, "clk_ao_asp_gt" },
	{ CLK_ANDGT_PTP, "clk_andgt_ptp", "clk_div_320m",
		0xF8, 5, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_ptp_gt"},
	{ CLK_BLPWM_PERI_GT, "clk_blpwm_peri_gt", "clk_blpwm_peri_mux",
		0xFC, 1, CLK_GATE_HIWORD_MASK, "clk_blpwm_peri_gt" },
	{ CLK_ANDGT_VDEC, "clk_andgt_vdec", "clk_mux_vdec",
		0xF0, 13, CLK_GATE_HIWORD_MASK, "clkgt_vdec" },
	{ CLK_ANDGT_VENC, "clk_andgt_venc", "clk_mux_venc",
		0xF0, 9, CLK_GATE_HIWORD_MASK, "clkgt_venc" },
	{ CLK_GT_DPCTRL_16M, "clk_gt_dpctrl_16m", "clk_div_a53hpm",
		0xfc, 12, CLK_GATE_HIWORD_MASK, "clk_gt_16m_dp" },
	{ CLK_GT_ISP_I2C, "clk_gt_isp_i2c", "clk_div_320m",
		0xf8, 4, CLK_GATE_HIWORD_MASK, "clk_gt_isp_i2c" },
	{ CLK_ANDGT_VENC2, "clk_andgt_venc2", "clk_mux_venc2",
		0x0F0, 14, CLK_GATE_HIWORD_MASK, "clkgt_venc2" },
	{ CLK_ISP_SNCLK_ANGT, "clk_isp_snclk_angt", "clk_div_320m",
		0x108, 2, CLK_GATE_HIWORD_MASK, "clk_ispsn_gt" },
	{ CLK_ANDGT_RXDPHY, "clk_andgt_rxdphy", "clk_div_a53hpm",
		0x0F0, 12, CLK_GATE_HIWORD_MASK, "clk_rxdcfg_gt" },
	{ CLK_GT_LOADMONITOR, "clk_gt_loadmonitor", "clk_div_320m",
		0x0f0, 5, CLK_GATE_HIWORD_MASK, "clk_loadmonitor_gt" },
	{ CLK_ANDGT_IVP32DSP_CORE, "clk_andgt_ivp32dsp_core", "clk_mux_ivp32dsp_core",
		0x0F0, 7, CLK_GATE_HIWORD_MASK, "clkgt_ivp32dsp" },
	{ CLK_ANDGT_AO_HIFD, "clk_andgt_ao_hifd", "clk_ao_hifd_mux",
		0x100, 10, CLK_GATE_HIWORD_MASK, "clkgt_ao_hifd" },
	{ AUTODIV_CFGBUS, "autodiv_cfgbus", "autodiv_sysbus",
		0x404, 4, CLK_GATE_HIWORD_MASK, "autodiv_cfgbus" },
	{ AUTODIV_DMABUS, "autodiv_dmabus", "autodiv_sysbus",
		0x404, 3, CLK_GATE_HIWORD_MASK, "autodiv_dmabus" },
};

static const struct pll_clock pll_clks[] = {
	{ CLK_GATE_PPLL0, "clk_gate_ppll0", "clk_ppll0", 0x0,
		{0x96C, 0}, {0x970, 0}, {0x974, 0}, {0x4F0, 26}, HS_PMCTRL, 0, "clk_ap_ppll0" },
	{ CLK_GATE_PPLL1, "clk_gate_ppll1", "clk_ppll1", 0x1,
		{0x954, 0}, {0x958, 0}, {0x95C, 0}, {0x500, 26}, HS_PMCTRL, 0, "clk_ap_ppll1" },
	{ CLK_GATE_PPLL2, "clk_gate_ppll2", "clk_ppll2", 0x2,
		{0x954, 5}, {0x958, 5}, {0x95C, 5}, {0x508, 26}, HS_PMCTRL, 0, "clk_ap_ppll2" },
	{ CLK_GATE_PPLL3, "clk_gate_ppll3", "clk_ppll3", 0x3,
		{0x954, 10}, {0x958, 10}, {0x95C, 10}, {0x510, 26}, HS_PMCTRL, 0, "clk_ap_ppll3" },
	{ CLK_GATE_PPLL5, "clk_gate_ppll5", "clk_ppll5", 0x5,
		{0x96C, 5}, {0x970, 5}, {0x974, 5}, {0x520, 26}, HS_PMCTRL, 0, "clk_ap_ppll5" },
	{ CLK_GATE_PPLL6, "clk_gate_ppll6", "clk_ppll6", 0x6,
		{0x960, 5}, {0x964, 5}, {0x968, 5}, {0x528, 26}, HS_PMCTRL, 0, "clk_ap_ppll6" },
	{ CLK_GATE_PPLL7, "clk_gate_ppll7", "clk_ppll7", 0x7,
		{0x960, 10}, {0x964, 10}, {0x968, 10}, {0x530, 26}, HS_PMCTRL, 0, "clk_ap_ppll7" },
};

static const struct div_clock crgctrl_div_clks[] = {
	{ CLK_DIV_CFGBUS, "clk_div_cfgbus", "clk_div_sysbus",
		0xEC, 0x3, 4, 1, 0, "sc_div_cfgbus" },
	{ CLK_DIV_MMC_USBDP, "clk_div_mmc_usbdp", "clk_andgt_mmc_usbdp",
		0x710, 0xf, 16, 1, 0, "div_mmc_usbdp" },
	{ PCLK_DIV_MMC1_PCIE, "pclk_div_mmc1_pcie", "pclk_andgt_mmc1_pcie",
		0xb4, 0x3c0, 16, 1, 0, "pclk_pcie_div" },
	{ CLK_DIV_PERF_STAT, "clk_div_perf_stat", "clk_perf_div_gt",
		0x0D0, 0xf000, 16, 1, 0, "clk_perf_div" },
	{ CLK_DIV_DMABUS, "clk_div_dmabus", "autodiv_dmabus",
		0x0EC, 0x8000, 2, 1, 0, "clk_dmabus_div" },
	{ CLK_DIV_SOCP_DEFLATE, "clk_div_socp_deflate", "clk_socp_deflate_gt",
		0x700, 0x3e00, 32, 1, 0, "clk_socpdef_div" },
	{ CLK_DIV_VCODECBUS, "clk_div_vcodecbus", "clk_gate_vcodecbus_gt",
		0x0BC, 0x3f, 64, 1, 0, "clk_vcodbus_div" },
	{ CLK_DIV_MMC0BUS, "clk_div_mmc0bus", "clk_div_sysbus",
		0x0EC, 0x4000, 2, 1, 0, "clk_div_mmc0bus" },
	{ CLK_DIV_SD, "clk_div_sd", "clk_andgt_sd",
		0xB8, 0xf, 16, 1, 0, "clk_sd_div" },
	{ CLK_DIV_UARTH, "clk_div_uarth", "clk_andgt_uarth",
		0xB0, 0xf000, 16, 1, 0, "clkdiv_uarth" },
	{ CLK_DIV_UARTL, "clk_div_uartl", "clk_andgt_uartl",
		0xB0, 0xf00, 16, 1, 0, "clkdiv_uartl" },
	{ CLK_DIV_UART0, "clk_div_uart0", "clk_andgt_uart0",
		0xB0, 0xf0, 16, 1, 0, "clkdiv_uart0" },
	{ CLK_DIV_I2C, "clk_div_i2c", "clk_div_320m",
		0xE8, 0xf0, 16, 1, 0, "clkdiv_i2c" },
	{ CLK_DIV_I3C4, "clk_div_i3c4", "clk_andgt_i3c4",
		0x710, 0xc00, 4, 1, 0, "clkdiv_i3c4" },
	{ CLK_DIV_SPI, "clk_div_spi", "clk_andgt_spi",
		0xC4, 0xf000, 16, 1, 0, "clkdiv_spi" },
	{ CLK_DIV_AO_ASP, "clk_div_ao_asp", "clk_div_ao_asp_gt",
		0x108, 0x3c0, 16, 1, 0, "clk_ao_asp_div" },
	{ CLK_DIV_PTP, "clk_div_ptp", "clk_andgt_ptp",
		0xD8, 0xf, 16, 1, 0, "clk_ptp_div" },
	{ CLK_BLPWM_PERI_DIV, "clk_blpwm_peri_div", "clk_blpwm_peri_gt",
		0xEC, 0x30, 4, 1, 0, "div_blpwm_peri" },
	{ CLK_DIV_VDEC, "clk_div_vdec", "clk_andgt_vdec",
		0xC4, 0x3f, 64, 1, 0, "clkdiv_vdec" },
	{ CLK_DIV_VENC, "clk_div_venc", "clk_andgt_venc",
		0xC0, 0x3f00, 64, 1, 0, "clkdiv_venc" },
	{ CLK_DIV_DPCTRL_16M, "clk_div_dpctrl_16m", "clk_fix_div_dpctrl",
		0x700, 0xc000, 4, 1, 0, "div_dpctrl16m" },
	{ CLK_DIV_ISP_I2C, "clk_div_isp_i2c", "clk_gt_isp_i2c",
		0xb8, 0x780, 16, 1, 0, "clk_div_isp_i2c" },
	{ CLK_DIV_VENC2, "clk_div_venc2", "clk_andgt_venc2",
		0x0C0, 0x3f, 64, 1, 0, "clkdiv_venc2" },
	{ CLK_ISP_SNCLK_DIV0, "clk_isp_snclk_div0", "clk_isp_snclk_fac",
		0x108, 0x3, 4, 1, 0, "clk_div_ispsn0" },
	{ CLK_ISP_SNCLK_DIV1, "clk_isp_snclk_div1", "clk_isp_snclk_fac",
		0x10C, 0xc000, 4, 1, 0, "clk_div_ispsn1" },
	{ CLK_ISP_SNCLK_DIV2, "clk_isp_snclk_div2", "clk_isp_snclk_fac",
		0x10C, 0x1800, 4, 1, 0, "clk_div_ispsn2" },
	{ CLK_ISP_SNCLK_DIV3, "clk_isp_snclk_div3", "clk_isp_snclk_fac",
		0x100, 0x300, 4, 1, 0, "clk_div_ispsn3" },
	{ CLK_DIV_LOADMONITOR, "clk_div_loadmonitor", "clk_gt_loadmonitor",
		0x0b8, 0xc000, 4, 1, 0, "clk_loadmonitor_div" },
	{ CLK_DIV_IVP32DSP_CORE, "clk_div_ivp32dsp_core", "clk_andgt_ivp32dsp_core",
		0x0B4, 0x3f, 64, 1, 0, "clkdiv_ivpdsp" },
	{ CLK_DIV_AO_HIFD, "clk_div_ao_hifd", "clk_andgt_ao_hifd",
		0x104, 0xc000, 4, 1, 0, "clk_ao_hifd_div" },
};

static const char * const clk_mux_wd0_p [] = { "clkin_ref", "pclk_gate_wd0_high" };
static const char * const codeccssi_mux_p [] = { "clk_sys_ini", "clk_sys_ini" };
static const char * const clk_mux_vcodecbus_p [] = {
		"clk_invalid", "clk_gate_ppll0", "clk_sys_ini", "clk_invalid",
		"clk_gate_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_sd_sys_p [] = { "clk_sd_sys", "clk_div_sd" };
static const char * const clk_mux_sd_pll_p [] = { "clk_gate_ppll0", "clk_gate_ppll3", "clk_gate_ppll2", "clk_gate_ppll2" };
static const char * const clk_mux_a53hpm_p [] = { "clk_gate_ppll0", "clk_gate_ppll2" };
static const char * const clk_mux_320m_p [] = { "clk_gate_ppll2", "clk_gate_ppll0" };
static const char * const clk_mux_uarth_p [] = { "clk_sys_ini", "clk_div_uarth" };
static const char * const clk_mux_uartl_p [] = { "clk_sys_ini", "clk_div_uartl" };
static const char * const clk_mux_uart0_p [] = { "clk_sys_ini", "clk_div_uart0" };
static const char * const clk_mux_i2c_p [] = { "clk_sys_ini", "clk_div_i2c" };
static const char * const clk_mux_spi_p [] = { "clk_sys_ini", "clk_div_spi" };
static const char * const clk_mux_ao_asp_p [] = { "clk_gate_ppll2", "clk_gate_ppll0", "clk_gate_ppll3", "clk_gate_ppll3" };
static const char * const clk_blpwm_peri_mux_p [] = { "clk_div_320m", "clk_fll_src" };
static const char * const clk_mux_gps_ref_p [] = { "clk_gate_mdm2gps0", "clk_gate_mdm2gps1", "clk_gate_mdm2gps2", "clk_gate_mdm2gps2" };
static const char * const clk_mux_vdec_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0", "clk_invalid",
		"clk_gate_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_venc_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0", "clk_invalid",
		"clk_gate_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_venc2_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0", "clk_invalid",
		"clk_gate_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_isp_snclk_mux0_p [] = { "clk_sys_ini", "clk_isp_snclk_div0" };
static const char * const clk_isp_snclk_mux1_p [] = { "clk_sys_ini", "clk_isp_snclk_div1" };
static const char * const clk_isp_snclk_mux2_p [] = { "clk_sys_ini", "clk_isp_snclk_div2" };
static const char * const clk_isp_snclk_mux3_p [] = { "clk_sys_ini", "clk_isp_snclk_div3" };
static const char * const clk_mux_rxdphy_cfg_p [] = { "clk_factor_rxdphy", "clk_sys_ini" };
static const char * const clk_mux_ivp32dsp_core_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0", "clk_invalid",
		"clk_gate_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_ao_hifd_mux_p [] = { "clk_gate_ppll0", "clk_gate_ppll2" };
static const struct mux_clock crgctrl_mux_clks[] = {
	{ CLK_MUX_WD0, "clk_mux_wd0", clk_mux_wd0_p,
		ARRAY_SIZE(clk_mux_wd0_p), 0x140, 0x20000, 0, "clk_wd0_mux" },
	{ CODECCSSI_MUX, "codeccssi_mux", codeccssi_mux_p,
		ARRAY_SIZE(codeccssi_mux_p), 0x0AC, 0x80, CLK_MUX_HIWORD_MASK, "sel_codeccssi" },
	{ CLK_MUX_VCODECBUS, "clk_mux_vcodecbus", clk_mux_vcodecbus_p,
		ARRAY_SIZE(clk_mux_vcodecbus_p), 0x0C8, 0xf, CLK_MUX_HIWORD_MASK, "clk_vcodbus_mux" },
	{ CLK_MUX_SD_SYS, "clk_mux_sd_sys", clk_mux_sd_sys_p,
		ARRAY_SIZE(clk_mux_sd_sys_p), 0x0B8, 0x40, CLK_MUX_HIWORD_MASK, "clk_sd_muxsys" },
	{ CLK_MUX_SD_PLL, "clk_mux_sd_pll", clk_mux_sd_pll_p,
		ARRAY_SIZE(clk_mux_sd_pll_p), 0x0B8, 0x30, CLK_MUX_HIWORD_MASK, "clk_sd_muxpll" },
	{ CLK_MUX_A53HPM, "clk_mux_a53hpm", clk_mux_a53hpm_p,
		ARRAY_SIZE(clk_mux_a53hpm_p), 0x0D4, 0x200, CLK_MUX_HIWORD_MASK, "clk_a53hpm_mux" },
	{ CLK_MUX_320M, "clk_mux_320m", clk_mux_320m_p,
		ARRAY_SIZE(clk_mux_320m_p), 0x100, 0x1, CLK_MUX_HIWORD_MASK, "sc_sel_320m_pll" },
	{ CLK_MUX_UARTH, "clk_mux_uarth", clk_mux_uarth_p,
		ARRAY_SIZE(clk_mux_uarth_p), 0xAC, 0x10, CLK_MUX_HIWORD_MASK, "clkmux_uarth" },
	{ CLK_MUX_UARTL, "clk_mux_uartl", clk_mux_uartl_p,
		ARRAY_SIZE(clk_mux_uartl_p), 0xAC, 0x8, CLK_MUX_HIWORD_MASK, "clkmux_uartl" },
	{ CLK_MUX_UART0, "clk_mux_uart0", clk_mux_uart0_p,
		ARRAY_SIZE(clk_mux_uart0_p), 0xAC, 0x4, CLK_MUX_HIWORD_MASK, "clkmux_uart0" },
	{ CLK_MUX_I2C, "clk_mux_i2c", clk_mux_i2c_p,
		ARRAY_SIZE(clk_mux_i2c_p), 0xAC, 0x2000, CLK_MUX_HIWORD_MASK, "clkmux_i2c" },
	{ CLK_MUX_SPI, "clk_mux_spi", clk_mux_spi_p,
		ARRAY_SIZE(clk_mux_spi_p), 0xAC, 0x100, CLK_MUX_HIWORD_MASK, "clkmux_spi" },
	{ CLK_MUX_AO_ASP, "clk_mux_ao_asp", clk_mux_ao_asp_p,
		ARRAY_SIZE(clk_mux_ao_asp_p), 0x100, 0x30, CLK_MUX_HIWORD_MASK, "clkmux_ao_asp" },
	{ CLK_BLPWM_PERI_MUX, "clk_blpwm_peri_mux", clk_blpwm_peri_mux_p,
		ARRAY_SIZE(clk_blpwm_peri_mux_p), 0x10C, 0x20, CLK_MUX_HIWORD_MASK, "sel_blpwm" },
	{ CLK_MUX_GPS_REF, "clk_mux_gps_ref", clk_mux_gps_ref_p,
		ARRAY_SIZE(clk_mux_gps_ref_p), 0xAC, 0x60, CLK_MUX_HIWORD_MASK, "clkmux_gps_ref" },
	{ CLK_MUX_VDEC, "clk_mux_vdec", clk_mux_vdec_p,
		ARRAY_SIZE(clk_mux_vdec_p), 0xC8, 0xf00, CLK_MUX_HIWORD_MASK, "clkmux_vdec" },
	{ CLK_MUX_VENC, "clk_mux_venc", clk_mux_venc_p,
		ARRAY_SIZE(clk_mux_venc_p), 0xC8, 0xf0, CLK_MUX_HIWORD_MASK, "clkmux_venc" },
	{ CLK_MUX_VENC2, "clk_mux_venc2", clk_mux_venc2_p,
		ARRAY_SIZE(clk_mux_venc2_p), 0x0C8, 0xf000, CLK_MUX_HIWORD_MASK, "clkmux_venc2" },
	{ CLK_ISP_SNCLK_MUX0, "clk_isp_snclk_mux0", clk_isp_snclk_mux0_p,
		ARRAY_SIZE(clk_isp_snclk_mux0_p), 0x108, 0x8, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn0" },
	{ CLK_ISP_SNCLK_MUX1, "clk_isp_snclk_mux1", clk_isp_snclk_mux1_p,
		ARRAY_SIZE(clk_isp_snclk_mux1_p), 0x10C, 0x2000, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn1" },
	{ CLK_ISP_SNCLK_MUX2, "clk_isp_snclk_mux2", clk_isp_snclk_mux2_p,
		ARRAY_SIZE(clk_isp_snclk_mux2_p), 0x10C, 0x400, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn2" },
	{ CLK_ISP_SNCLK_MUX3, "clk_isp_snclk_mux3", clk_isp_snclk_mux3_p,
		ARRAY_SIZE(clk_isp_snclk_mux3_p), 0x100, 0x80, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn3" },
	{ CLK_MUX_RXDPHY_CFG, "clk_mux_rxdphy_cfg", clk_mux_rxdphy_cfg_p,
		ARRAY_SIZE(clk_mux_rxdphy_cfg_p), 0x0C4, 0x100, CLK_MUX_HIWORD_MASK, "clk_rxdcfg_mux" },
	{ CLK_MUX_IVP32DSP_CORE, "clk_mux_ivp32dsp_core", clk_mux_ivp32dsp_core_p,
		ARRAY_SIZE(clk_mux_ivp32dsp_core_p), 0x708, 0xf000, CLK_MUX_HIWORD_MASK, "clkmux_ivp32dsp" },
	{ CLK_AO_HIFD_MUX, "clk_ao_hifd_mux", clk_ao_hifd_mux_p,
		ARRAY_SIZE(clk_ao_hifd_mux_p), 0x100, 0x40, CLK_MUX_HIWORD_MASK, "clkmux_ao_hifd" },
};

static const struct div_clock hsdtctrl_div_clks[] = {
	{ CLK_DIV_SDIO, "clk_div_sdio", "clk_andgt_sdio",
		0x0A8, 0xf, 16, 1, 0, "clk_sdio_div" },
	{ CLKDIV_DP_AUDIO_PLL_AO, "clkdiv_dp_audio_pll_ao", "clkgt_dp_audio_pll_ao",
		0xB0, 0xf, 16, 1, 0, "clk_dp_audio_pll_ao_div" },
};

static const char * const clk_mux_sdio_sys_p [] = { "clk_sdio_sys", "clk_div_sdio" };
static const struct mux_clock hsdtctrl_mux_clks[] = {
	{ CLK_MUX_SDIO_SYS, "clk_mux_sdio_sys", clk_mux_sdio_sys_p,
		ARRAY_SIZE(clk_mux_sdio_sys_p), 0x0A8, 0x2000, CLK_MUX_HIWORD_MASK, "clk_sdio_muxsy" },
};

static const struct gate_clock hsdtctrl_gate_clks[] = {
	{ CLK_GATE_PCIEPHY_REF, "clk_gate_pciephy_ref", "clk_ppll_pcie", 0x010, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pciephy_ref" },
	{ CLK_GATE_PCIE1PHY_REF, "clk_gate_pcie1phy_ref", "clk_ppll_pcie", 0x010, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcie1phy_ref" },
	{ PCLK_GATE_PCIE_SYS, "pclk_gate_pcie_sys", "clk_spll", 0x010, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pcie_sys" },
	{ PCLK_GATE_PCIE1_SYS, "pclk_gate_pcie1_sys", "clk_spll", 0x010, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pcie1_sys" },
	{ PCLK_GATE_PCIE_PHY, "pclk_gate_pcie_phy", "pclk_gate_mmc1_pcie", 0x010, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pcie_phy" },
	{ PCLK_GATE_PCIE1_PHY, "pclk_gate_pcie1_phy", "pclk_gate_mmc1_pcie", 0x010, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pcie1_phy" },
	{ HCLK_GATE_SDIO, "hclk_gate_sdio", "clk_spll", 0x000, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_sdio" },
	{ CLK_GATE_SDIO, "clk_gate_sdio", "clk_mux_sdio_sys", 0x00, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 2, "clk_sdio" },
	{ CLK_GATE_PCIEAUX, "clk_gate_pcieaux", "clk_sys_ini", 0x010, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcieaux" },
	{ CLK_GATE_PCIEAUX1, "clk_gate_pcieaux1", "clk_sys_ini", 0x010, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcieaux1" },
	{ ACLK_GATE_PCIE, "aclk_gate_pcie", "clk_spll", 0x010, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_pcie" },
	{ ACLK_GATE_PCIE1, "aclk_gate_pcie1", "clk_spll", 0x010, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_pcie1" },
	{ CLK_GATE_DP_AUDIO_PLL_AO, "clk_gate_dp_audio_pll_ao", "clkdiv_dp_audio_pll_ao", 0x00, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dp_audio_pll_ao" },
};

static const struct pll_clock hsdtctrl_pll_clks[] = {
	{ CLK_GATE_SCPLL, "clk_gate_scpll", "clk_scpll", 0x8,
		{0x200, 0}, {0x204, 26}, {0x200, 1}, {0x208, 0}, HS_HSDTCRG, 1, "clk_ap_scpll" },
};

static const struct scgt_clock hsdtctrl_scgt_clks[] = {
	{ CLK_ANDGT_SDIO, "clk_andgt_sdio", "clk_spll",
		0x0AC, 1, CLK_GATE_HIWORD_MASK, "clk_sdio_gt" },
	{ CLK_SDIO_SYS_GT, "clk_sdio_sys_gt", "clk_sys_ini",
		0x0AC, 0, CLK_GATE_HIWORD_MASK, "clk_sdio_sys_gt" },
	{ CLKGT_DP_AUDIO_PLL_AO, "clkgt_dp_audio_pll_ao", "clk_gate_scpll",
		0xAC, 4, CLK_GATE_HIWORD_MASK, "clk_dp_audio_pll_ao_gt" },
};

static const struct div_clock mmc0ctrl_div_clks[] = {
};

static const struct mux_clock mmc0ctrl_mux_clks[] = {
};

static const struct gate_clock mmc0ctrl_gate_clks[] = {
	{ HCLK_GATE_USB3OTG, "hclk_gate_usb3otg", "clk_gate_mmc_usbdp", 0x0, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_usb3otg" },
	{ ACLK_GATE_USB3OTG, "aclk_gate_usb3otg", "clk_gate_mmc_usbdp", 0x0, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_usb3otg" },
	{ HCLK_GATE_SD, "hclk_gate_sd", "clk_div_mmc0bus", 0x0, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_sd" },
	{ PCLK_GATE_DPCTRL, "pclk_gate_dpctrl", "clk_div_mmc0bus", 0x00, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_dpctrl" },
	{ ACLK_GATE_DPCTRL, "aclk_gate_dpctrl", "clk_gate_mmc_usbdp", 0x00, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_dpctrl" },
};

static const struct scgt_clock mmc0ctrl_scgt_clks[] = {
};

static const struct div_clock sctrl_div_clks[] = {
	{ CLK_DIV_AOBUS, "clk_div_aobus", "clk_spll",
		0x254, 0x3f, 64, 1, 0, "sc_div_aobus" },
	{ CLK_DIV_IOPERI, "clk_div_ioperi", "clk_andgt_ioperi",
		0x270, 0x3f, 64, 1, 0, "sc_div_ioperi" },
	{ CLKDIV_ASP_CODEC, "clkdiv_asp_codec", "clkgt_asp_codec",
		0x254, 0xf80, 32, 1, 0, "clkdiv_asp_codec" },
	{ CLK_DIV_AO_LOADMONITOR, "clk_div_ao_loadmonitor", "clk_gt_ao_loadmonitor",
		0x26c, 0x3c00, 16, 1, 0, "clk_ao_loadmonitor_div" },
	{ CLK_DIV_HIFD_FLL, "clk_div_hifd_fll", "clk_andgt_hifd_fll",
		0x284, 0x30, 4, 1, 0, "clk_hifd_fll_div" },
	{ CLK_DIV_HIFD_PLL, "clk_div_hifd_pll", "clk_andgt_hifd_pll",
		0x284, 0xf, 16, 1, 0, "clk_hifd_pll_div" },
};

static const char * const clk_mux_timer5_a_p [] = { "clkin_ref", "pclk", "clk_factor_tcxo", "pclk" };
static const char * const clk_mux_timer5_b_p [] = { "clkin_ref", "pclk", "clk_factor_tcxo", "pclk" };
static const char * const clk_usb2phy_ref_mux_p [] = { "clkin_sys_div", "clk_lbintpll" };
static const char * const clkmux_syscnt_p [] = { "clk_syscnt_div", "clkin_ref" };
static const char * const clk_mux_asp_codec_p [] = { "clkdiv_asp_codec", "clk_asp_backup" };
static const char * const clk_mux_asp_pll_p [] = {
		"clk_invalid", "clk_spll", "clk_fll_src", "clk_invalid",
		"clk_ao_asp_mux", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_sys_ini", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_ao_asp_mux_p [] = { "clk_gate_ao_asp", "clk_lbintpll_1" };
static const char * const clk_sw_ao_loadmonitor_p [] = { "clk_fll_src", "clk_spll" };
static const char * const clk_hifd_pll_mux_p [] = { "clk_spll", "clk_gate_ao_hifd" };
static const struct mux_clock sctrl_mux_clks[] = {
	{ CLK_MUX_TIMER5_A, "clk_mux_timer5_a", clk_mux_timer5_a_p,
		ARRAY_SIZE(clk_mux_timer5_a_p), 0x3c4, 0x30, 0, "clkmux_timer5_a" },
	{ CLK_MUX_TIMER5_B, "clk_mux_timer5_b", clk_mux_timer5_b_p,
		ARRAY_SIZE(clk_mux_timer5_b_p), 0x3c4, 0xc0, 0, "clkmux_timer5_b" },
	{ CLK_USB2PHY_REF_MUX, "clk_usb2phy_ref_mux", clk_usb2phy_ref_mux_p,
		ARRAY_SIZE(clk_usb2phy_ref_mux_p), 0x264, 0x4000, CLK_MUX_HIWORD_MASK, "clk_usb2phy_ref_mux" },
	{ CLKMUX_SYSCNT, "clkmux_syscnt", clkmux_syscnt_p,
		ARRAY_SIZE(clkmux_syscnt_p), 0x264, 0x2000, CLK_MUX_HIWORD_MASK, "clkmux_syscnt" },
	{ CLK_MUX_ASP_CODEC, "clk_mux_asp_codec", clk_mux_asp_codec_p,
		ARRAY_SIZE(clk_mux_asp_codec_p), 0x260, 0x8000, CLK_MUX_HIWORD_MASK, "sel_asp_codec" },
	{ CLK_MUX_ASP_PLL, "clk_mux_asp_pll", clk_mux_asp_pll_p,
		ARRAY_SIZE(clk_mux_asp_pll_p), 0x280, 0xf, CLK_MUX_HIWORD_MASK, "clk_asp_pll_sel" },
	{ CLK_AO_ASP_MUX, "clk_ao_asp_mux", clk_ao_asp_mux_p,
		ARRAY_SIZE(clk_ao_asp_mux_p), 0x250, 0x1000, CLK_MUX_HIWORD_MASK, "sel_ao_asp" },
	{ CLK_SW_AO_LOADMONITOR, "clk_sw_ao_loadmonitor", clk_sw_ao_loadmonitor_p,
		ARRAY_SIZE(clk_sw_ao_loadmonitor_p), 0x26c, 0x100, CLK_MUX_HIWORD_MASK, "clk_ao_loadmonitor_sw" },
	{ CLK_HIFD_PLL_MUX, "clk_hifd_pll_mux", clk_hifd_pll_mux_p,
		ARRAY_SIZE(clk_hifd_pll_mux_p), 0x284, 0x800, CLK_MUX_HIWORD_MASK, "clkmux_hifd_pll" },
};

static const struct gate_clock sctrl_gate_clks[] = {
	{ PCLK_GPIO20, "pclk_gpio20", "clk_div_aobus", 0x1B0, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio20" },
	{ PCLK_GPIO21, "pclk_gpio21", "clk_div_aobus", 0x1B0, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio21" },
	{ CLK_GATE_TIMER5_B, "clk_gate_timer5_b", "clk_mux_timer5_b", 0, 0x0, 0,
		"clk_timer5", 0, {0}, {0}, 0, 0, 0, "clk_timer5_b" },
	{ CLK_GATE_TIMER5, "clk_gate_timer5", "clk_div_aobus", 0x170, 0x3000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_timer5" },
	{ CLK_GATE_SPI, "clk_gate_spi", "clk_div_ioperi", 0x1B0, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 84, 0, "clk_spi3" },
	{ PCLK_GATE_SPI, "pclk_gate_spi", "clk_div_ioperi", 0x1B0, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi3" },
	{ CLK_GATE_USB2PHY_REF, "clk_gate_usb2phy_ref", "clk_usb2phy_ref_mux", 0x1B0, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_usb2phy_ref" },
	{ PCLK_GATE_RTC, "pclk_gate_rtc", "clk_div_aobus", 0x160, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_rtc" },
	{ PCLK_GATE_RTC1, "pclk_gate_rtc1", "clk_div_aobus", 0x160, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_rtc1" },
	{ PCLK_AO_GPIO0, "pclk_ao_gpio0", "clk_div_aobus", 0x160, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio0" },
	{ PCLK_AO_GPIO1, "pclk_ao_gpio1", "clk_div_aobus", 0x160, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio1" },
	{ PCLK_AO_GPIO2, "pclk_ao_gpio2", "clk_div_aobus", 0x160, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio2" },
	{ PCLK_AO_GPIO3, "pclk_ao_gpio3", "clk_div_aobus", 0x160, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio3" },
	{ PCLK_AO_GPIO4, "pclk_ao_gpio4", "clk_div_aobus", 0x160, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio4" },
	{ PCLK_AO_GPIO5, "pclk_ao_gpio5", "clk_div_aobus", 0x160, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio5" },
	{ PCLK_AO_GPIO6, "pclk_ao_gpio6", "clk_div_aobus", 0x160, 0x2000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio6" },
	{ PCLK_AO_GPIO29, "pclk_ao_gpio29", "clk_div_aobus", 0x1B0, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio29" },
	{ PCLK_AO_GPIO30, "pclk_ao_gpio30", "clk_div_aobus", 0x1B0, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio30" },
	{ PCLK_AO_GPIO31, "pclk_ao_gpio31", "clk_div_aobus", 0x1B0, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio31" },
	{ PCLK_AO_GPIO32, "pclk_ao_gpio32", "clk_div_aobus", 0x1B0, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio32" },
	{ PCLK_AO_GPIO33, "pclk_ao_gpio33", "clk_div_aobus", 0x1B0, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio33" },
	{ PCLK_AO_GPIO34, "pclk_ao_gpio34", "clk_div_aobus", 0x1B0, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio34" },
	{ PCLK_AO_GPIO35, "pclk_ao_gpio35", "clk_div_aobus", 0x1B0, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio35" },
	{ PCLK_AO_GPIO36, "pclk_ao_gpio36", "clk_div_aobus", 0x1B0, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio36" },
	{ PCLK_GATE_SYSCNT, "pclk_gate_syscnt", "clk_div_aobus", 0x160, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_syscnt" },
	{ CLK_GATE_SYSCNT, "clk_gate_syscnt", "clkmux_syscnt", 0x160, 0x100000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_syscnt" },
	{ CLK_ASP_BACKUP, "clk_asp_backup", "clk_lbintpll_1", 0x160, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_backup" },
	{ CLK_ASP_CODEC, "clk_asp_codec", "clk_mux_asp_codec", 0x170, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_codec" },
	{ CLK_GATE_ASP_SUBSYS, "clk_gate_asp_subsys", "clk_mux_asp_pll", 0x170, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_subsys" },
	{ CLK_GATE_ASP_TCXO, "clk_gate_asp_tcxo", "clk_sys_ini", 0x160, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_tcxo" },
	{ CLK_GATE_DP_AUDIO_PLL, "clk_gate_dp_audio_pll", "clk_gate_dp_audio_pll_ao", 0x1B0, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dp_audio_pll" },
	{ PCLK_GATE_AO_LOADMONITOR, "pclk_gate_ao_loadmonitor", "clk_div_aobus", 0x1b0, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_loadmonitor" },
	{ CLK_GATE_AO_LOADMONITOR, "clk_gate_ao_loadmonitor", "clk_div_ao_loadmonitor", 0x1b0, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ao_loadmonitor" },
	{ CLK_GATE_HIFD_TCXO, "clk_gate_hifd_tcxo", "clk_sys_ini", 0x190, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_hifd_tcxo" },
	{ CLK_GATE_HIFD_FLL, "clk_gate_hifd_fll", "clk_div_hifd_fll", 0x170, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_hifd_fll" },
	{ CLK_GATE_HIFD_PLL, "clk_gate_hifd_pll", "clk_div_hifd_pll", 0x170, 0x800000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_hifd_pll" },
};

static const struct scgt_clock sctrl_scgt_clks[] = {
	{ CLK_ANDGT_IOPERI, "clk_andgt_ioperi", "clk_spll",
		0x270, 6, CLK_GATE_HIWORD_MASK, "clk_gt_ioperi" },
	{ CLKGT_ASP_CODEC, "clkgt_asp_codec", "clk_spll_vote",
		0x274, 14, CLK_GATE_HIWORD_MASK, "clkgt_asp_codec" },
	{ CLK_GT_AO_LOADMONITOR, "clk_gt_ao_loadmonitor", "clk_sw_ao_loadmonitor",
		0x26c, 9, CLK_GATE_HIWORD_MASK, "clk_ao_loadmonitor_gt" },
	{ CLK_ANDGT_HIFD_FLL, "clk_andgt_hifd_fll", "clk_fll_src",
		0x254, 15, CLK_GATE_HIWORD_MASK, "clkgt_hifd_fll" },
	{ CLK_ANDGT_HIFD_PLL, "clk_andgt_hifd_pll", "clk_hifd_pll_mux",
		0x258, 5, CLK_GATE_HIWORD_MASK, "clkgt_hifd_pll" },
};

static const struct gate_clock iomcu_gate_clks[] = {
	{ CLK_I2C1_GATE_IOMCU, "clk_i2c1_gate_iomcu", "clk_fll_src", 0x10, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c1_gt" },
};

static const struct div_clock media1crg_div_clks[] = {
	{ CLK_DIV_VIVOBUS, "clk_div_vivobus", "clk_gate_vivobus_andgt",
		0x74, 0x3f, 64, 1, 0, "clk_vivobus_div" },
	{ CLK_DIV_EDC0, "clk_div_edc0", "clk_andgt_edc0",
		0x68, 0x3f, 64, 1, 0, "clk_edc0_div" },
	{ CLK_DIV_LDI1, "clk_div_ldi1", "clk_andgt_ldi1",
		0x64, 0x3f, 64, 1, 0, "clkdiv_ldi1" },
	{ CLK_DIV_ISPCPU, "clk_div_ispcpu", "clk_andgt_ispcpu",
		0x70, 0x3f, 64, 1, 0, "clk_ispcpu_div" },
	{ CLK_DIV_ISPFUNC, "clk_div_ispfunc", "clk_andgt_ispfunc",
		0x6c, 0x3f, 64, 1, 0, "clkdiv_ispfunc" },
	{ CLK_DIV_ISPFUNC2, "clk_div_ispfunc2", "clk_andgt_ispfunc2",
		0x88, 0x3f0, 64, 1, 0, "clkdiv_ispfunc2" },
	{ CLK_DIV_ISPFUNC3, "clk_div_ispfunc3", "clk_andgt_ispfunc3",
		0x88, 0xfc00, 64, 1, 0, "clkdiv_ispfunc3" },
	{ CLK_DIV_ISPFUNC4, "clk_div_ispfunc4", "clk_andgt_ispfunc4",
		0x74, 0xfc00, 64, 1, 0, "clkdiv_ispfunc4" },
	{ CLK_DIV_ISP_I3C, "clk_div_isp_i3c", "clk_andgt_isp_i3c",
		0x064, 0xfc00, 64, 1, 0, "clkdiv_isp_i3c" },
	{ CLK_MEDIA_COMMON_DIV, "clk_media_common_div", "clkandgt_media_common",
		0x80, 0xfc0, 64, 1, 0, "clkdiv_media_common" },
	{ CLK_DIV_FD_FUNC, "clk_div_fd_func", "clk_andgt_fd_func",
		0x7C, 0x3f0, 64, 1, 0, "clk_fd_func_div" },
	{ CLK_DIV_JPG_FUNC, "clk_div_jpg_func", "clk_andgt_jpg_func",
		0x80, 0x3f, 64, 1, 0, "clk_jpg_func_div" },
	{ CLK_DIV_FDAI_FUNC, "clk_div_fdai_func", "clk_andgt_fdai_func",
		0x98, 0x3f, 64, 1, 0, "clk_fdai_func_div" },
	{ ACLK_DIV_MMBUF, "aclk_div_mmbuf", "clk_mmbuf_pll_andgt",
		0x7C, 0xfc00, 64, 1, 0, "aclk_mmbuf_div" },
	{ PCLK_DIV_MMBUF, "pclk_div_mmbuf", "pclk_mmbuf_andgt",
		0x78, 0x3, 4, 1, 0, "pclk_mmbuf_div" },
};

static const char * const clk_mux_vivobus_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_edc0_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ldi1_p [] = {
		"clk_invalid", "clk_gate_ppll7_media", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispi2c_p [] = {"clk_gate_media_tcxo", "clk_gate_isp_i2c_media"};
static const char * const clk_mux_ispcpu_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc2_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc3_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc4_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_media_common_sw_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_fd_func_mux_p [] = {
		"clk_invalid", "clk_gate_ppll0_media", "clk_gate_ppll2_media", "clk_gate_ppll3_media"
};
static const char * const clk_jpg_func_mux_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_fdai_func_mux_p [] = {
		"clk_invalid", "clk_gate_ppll0_media", "clk_gate_ppll2_media", "clk_gate_ppll3_media"
};
static const char * const clk_sw_mmbuf_p [] = {
		"clk_invalid", "clk_invalid", "clk_gate_ppll0_media", "clk_invalid",
		"clk_gate_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_gate_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const struct mux_clock media1crg_mux_clks[] = {
	{ CLK_MUX_VIVOBUS, "clk_mux_vivobus", clk_mux_vivobus_p,
		ARRAY_SIZE(clk_mux_vivobus_p), 0x74, 0x3c0, CLK_MUX_HIWORD_MASK, "clk_vivobus_mux" },
	{ CLK_MUX_EDC0, "clk_mux_edc0", clk_mux_edc0_p,
		ARRAY_SIZE(clk_mux_edc0_p), 0x68, 0x3c0, CLK_MUX_HIWORD_MASK, "sel_edc0_pll" },
	{ CLK_MUX_LDI1, "clk_mux_ldi1", clk_mux_ldi1_p,
		ARRAY_SIZE(clk_mux_ldi1_p), 0x64, 0x3c0, CLK_MUX_HIWORD_MASK, "clkmux_ldi1" },
	{ CLK_MUX_ISPI2C, "clk_mux_ispi2c", clk_mux_ispi2c_p,
		ARRAY_SIZE(clk_mux_ispi2c_p), 0x78, 0x4, CLK_MUX_HIWORD_MASK, "clk_ispi2c_mux" },
	{ CLK_MUX_ISPCPU, "clk_mux_ispcpu", clk_mux_ispcpu_p,
		ARRAY_SIZE(clk_mux_ispcpu_p), 0x60, 0x3c00, CLK_MUX_HIWORD_MASK, "sel_ispcpu" },
	{ CLK_MUX_ISPFUNC, "clk_mux_ispfunc", clk_mux_ispfunc_p,
		ARRAY_SIZE(clk_mux_ispfunc_p), 0x6c, 0x3c0, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc" },
	{ CLK_MUX_ISPFUNC2, "clk_mux_ispfunc2", clk_mux_ispfunc2_p,
		ARRAY_SIZE(clk_mux_ispfunc2_p), 0x6c, 0x3c00, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc2" },
	{ CLK_MUX_ISPFUNC3, "clk_mux_ispfunc3", clk_mux_ispfunc3_p,
		ARRAY_SIZE(clk_mux_ispfunc3_p), 0x70, 0x3c00, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc3" },
	{ CLK_MUX_ISPFUNC4, "clk_mux_ispfunc4", clk_mux_ispfunc4_p,
		ARRAY_SIZE(clk_mux_ispfunc4_p), 0x78, 0x78, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc4" },
	{ CLK_MEDIA_COMMON_SW, "clk_media_common_sw", clk_media_common_sw_p,
		ARRAY_SIZE(clk_media_common_sw_p), 0x80, 0xf000, CLK_MUX_HIWORD_MASK, "sel_media_common_pll" },
	{ CLK_FD_FUNC_MUX, "clk_fd_func_mux", clk_fd_func_mux_p,
		ARRAY_SIZE(clk_fd_func_mux_p), 0x70, 0x300, CLK_MUX_HIWORD_MASK, "clkmux_fd_func" },
	{ CLK_JPG_FUNC_MUX, "clk_jpg_func_mux", clk_jpg_func_mux_p,
		ARRAY_SIZE(clk_jpg_func_mux_p), 0x60, 0x3c0, CLK_MUX_HIWORD_MASK, "clkmux_jpg_func" },
	{ CLK_FDAI_FUNC_MUX, "clk_fdai_func_mux", clk_fdai_func_mux_p,
		ARRAY_SIZE(clk_fdai_func_mux_p), 0x98, 0xc0, CLK_MUX_HIWORD_MASK, "clkmux_fdai_func" },
	{ CLK_SW_MMBUF, "clk_sw_mmbuf", clk_sw_mmbuf_p,
		ARRAY_SIZE(clk_sw_mmbuf_p), 0x88, 0xf, CLK_MUX_HIWORD_MASK, "aclk_mmbuf_sw" },
};

static const struct gate_clock media1crg_gate_clks[] = {
	{ PCLK_GATE_ISP_NOC_SUBSYS, "pclk_gate_isp_noc_subsys", "clk_div_sysbus", 0x10, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_isp_noc_subsys" },
	{ ACLK_GATE_ISP_NOC_SUBSYS, "aclk_gate_isp_noc_subsys", "clk_ppll0", 0x10, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_isp_noc_subsys" },
	{ ACLK_GATE_MEDIA_COMMON, "aclk_gate_media_common", "aclk_gate_disp_noc_subsys", 0x10, 0x800000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_media_common" },
	{ ACLK_GATE_NOC_DSS, "aclk_gate_noc_dss", "aclk_gate_disp_noc_subsys", 0x10, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_noc_dss" },
	{ PCLK_GATE_MEDIA_COMMON, "pclk_gate_media_common", "pclk_gate_disp_noc_subsys", 0x10, 0x1000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_media_common" },
	{ PCLK_GATE_NOC_DSS_CFG, "pclk_gate_noc_dss_cfg", "pclk_gate_disp_noc_subsys", 0x10, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_noc_dss_cfg" },
	{ PCLK_GATE_MMBUF_CFG, "pclk_gate_mmbuf_cfg", "pclk_gate_disp_noc_subsys", 0x20, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_mmbuf_cfg" },
	{ PCLK_GATE_DISP_NOC_SUBSYS, "pclk_gate_disp_noc_subsys", "clk_div_sysbus", 0x10, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_disp_noc_subsys" },
	{ ACLK_GATE_DISP_NOC_SUBSYS, "aclk_gate_disp_noc_subsys", "clk_ppll0", 0x10, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_disp_noc_subsys" },
	{ PCLK_GATE_DSS, "pclk_gate_dss", "pclk_gate_disp_noc_subsys", 0x00, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_dss" },
	{ACLK_GATE_DSS, "aclk_gate_dss", "aclk_gate_disp_noc_subsys", 0x00, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 8, 0, "aclk_dss" },
	{ ACLK_GATE_ISP, "aclk_gate_isp", "aclk_gate_isp_noc_subsys", 0x00, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 7, 0, "aclk_isp" },
	{ CLK_GATE_VIVOBUS_TO_DDRC, "clk_gate_vivobus_to_ddrc", "clk_div_vivobus", 0x00, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vivobus2ddr" },
	{ CLK_GATE_EDC0, "clk_gate_edc0", "clk_div_edc0", 0x00, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 20, 0, "clk_edc0" },
	{ CLK_GATE_LDI1, "clk_gate_ldi1", "clk_div_ldi1", 0x00, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 23, 0, "clk_ldi1" },
	{ CLK_GATE_ISPI2C, "clk_gate_ispi2c", "clk_mux_ispi2c", 0x00, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispi2c" },
	{ CLK_GATE_ISP_SYS, "clk_gate_isp_sys", "clk_gate_media_tcxo", 0x00, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_sys" },
	{ CLK_GATE_ISPCPU, "clk_gate_ispcpu", "clk_div_ispcpu", 0x00, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispcpu" },
	{ CLK_GATE_ISPFUNCFREQ, "clk_gate_ispfuncfreq", "clk_div_ispfunc", 0x00, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 26, 0, "clk_ispfuncfreq" },
	{ CLK_GATE_ISPFUNC2FREQ, "clk_gate_ispfunc2freq", "clk_div_ispfunc2", 0x00, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 38, 0, "clk_ispfunc2freq" },
	{ CLK_GATE_ISPFUNC3FREQ, "clk_gate_ispfunc3freq", "clk_div_ispfunc3", 0x00, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 39, 0, "clk_ispfunc3freq" },
	{ CLK_GATE_ISPFUNC4FREQ, "clk_gate_ispfunc4freq", "clk_div_ispfunc4", 0x00, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 40, 0, "clk_ispfunc4freq" },
	{ CLK_GATE_ISP_I3C, "clk_gate_isp_i3c", "clk_div_isp_i3c", 0x010, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_i3c" },
	{ CLK_GATE_MEDIA_COMMONFREQ, "clk_gate_media_commonfreq", "clk_media_common_div", 0x10, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 34, 0, "clk_media_commonfreq" },
	{ CLK_GATE_BRG, "clk_gate_brg", "clk_media_common_div", 0x00, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_brg" },
	{ PCLK_GATE_MEDIA1_LM, "pclk_gate_media1_lm", "clk_div_ptp", 0x00, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_media1_lm" },
	{ CLK_GATE_LOADMONITOR_MEDIA1, "clk_gate_loadmonitor_media1", "clk_div_ptp", 0x00, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor_media1" },
	{ CLK_GATE_FD_FUNC, "clk_gate_fd_func", "clk_div_fd_func", 0x10, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_fd_func" },
	{ CLK_GATE_JPG_FUNCFREQ, "clk_gate_jpg_funcfreq", "clk_div_jpg_func", 0x10, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 36, 0, "clk_jpg_funcfreq" },
	{ ACLK_GATE_JPG, "aclk_gate_jpg", "aclk_gate_isp_noc_subsys", 0x10, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_jpg" },
	{ PCLK_GATE_JPG, "pclk_gate_jpg", "pclk_gate_isp_noc_subsys", 0x10, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_jpg" },
	{ ACLK_GATE_NOC_ISP, "aclk_gate_noc_isp", "aclk_gate_isp_noc_subsys", 0x10, 0x2000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_noc_isp" },
	{ PCLK_GATE_NOC_ISP, "pclk_gate_noc_isp", "pclk_gate_isp_noc_subsys", 0x10, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_noc_isp" },
	{ CLK_GATE_FDAI_FUNCFREQ, "clk_gate_fdai_funcfreq", "clk_div_fdai_func", 0x10, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 35, 0, "clk_fdai_funcfreq" },
	{ ACLK_GATE_ASC, "aclk_gate_asc", "clk_gate_mmbuf", 0x20, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_asc" },
	{ CLK_GATE_DSS_AXI_MM, "clk_gate_dss_axi_mm", "clk_gate_mmbuf", 0x20, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dss_axi_mm" },
	{ CLK_GATE_MMBUF, "clk_gate_mmbuf", "aclk_div_mmbuf", 0x20, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mmbuf" },
	{ PCLK_GATE_MMBUF, "pclk_gate_mmbuf", "pclk_div_mmbuf", 0x20, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 58, 0, "pclk_mmbuf" },
	{ CLK_GATE_ATDIV_VIVO, "clk_gate_atdiv_vivo", "clk_div_vivobus", 0x010, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_vivo" },
	{ CLK_GATE_ATDIV_ISPCPU, "clk_gate_atdiv_ispcpu", "clk_div_ispcpu", 0x010, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_ispcpu" },
};

static const struct scgt_clock media1crg_scgt_clks[] = {
	{ CLK_GATE_VIVOBUS_ANDGT, "clk_gate_vivobus_andgt", "clk_mux_vivobus",
		0x84, 3, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_vivobus_gt" },
	{ CLK_ANDGT_EDC0, "clk_andgt_edc0", "clk_mux_edc0",
		0x84, 7, CLK_GATE_HIWORD_MASK, "clk_edc0_gt" },
	{ CLK_ANDGT_LDI1, "clk_andgt_ldi1", "clk_mux_ldi1",
		0x84, 8, CLK_GATE_HIWORD_MASK, "clkgt_ldi1" },
	{ CLK_ANDGT_ISPCPU, "clk_andgt_ispcpu", "clk_mux_ispcpu",
		0x84, 5, CLK_GATE_HIWORD_MASK, "clk_ispcpu_gt" },
	{ CLK_ANDGT_ISPFUNC, "clk_andgt_ispfunc", "clk_mux_ispfunc",
		0x84, 6, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc" },
	{ CLK_ANDGT_ISPFUNC2, "clk_andgt_ispfunc2", "clk_mux_ispfunc2",
		0x84, 2, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc2" },
	{ CLK_ANDGT_ISPFUNC3, "clk_andgt_ispfunc3", "clk_mux_ispfunc3",
		0x84, 1, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc3" },
	{ CLK_ANDGT_ISPFUNC4, "clk_andgt_ispfunc4", "clk_mux_ispfunc4",
		0x84, 0, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc4" },
	{ CLK_ANDGT_ISP_I3C, "clk_andgt_isp_i3c", "clk_gate_ppll0_media",
		0x084, 9, CLK_GATE_HIWORD_MASK, "clkgt_isp_i3c" },
	{ CLKANDGT_MEDIA_COMMON, "clkandgt_media_common", "clk_media_common_sw",
		0x84, 11, CLK_GATE_HIWORD_MASK, "clkgt_media_common" },
	{ CLK_ANDGT_FD_FUNC, "clk_andgt_fd_func", "clk_fd_func_mux",
		0x84, 13, CLK_GATE_HIWORD_MASK, "clkgt_fd_func" },
	{ CLK_ANDGT_JPG_FUNC, "clk_andgt_jpg_func", "clk_jpg_func_mux",
		0x84, 12, CLK_GATE_HIWORD_MASK, "clkgt_jpg_func" },
	{ CLK_ANDGT_FDAI_FUNC, "clk_andgt_fdai_func", "clk_fdai_func_mux",
		0x84, 10, CLK_GATE_HIWORD_MASK, "clkgt_fdai_func" },
	{ CLK_MMBUF_PLL_ANDGT, "clk_mmbuf_pll_andgt", "clk_sw_mmbuf",
		0x84, 14, CLK_GATE_HIWORD_MASK, "clk_mmbuf_gt" },
	{ PCLK_MMBUF_ANDGT, "pclk_mmbuf_andgt", "aclk_div_mmbuf",
		0x84, 15, CLK_GATE_HIWORD_MASK, "pclk_mmbuf_gt" },
};

static const struct div_clock media2crg_div_clks[] = {
};

static const struct mux_clock media2crg_mux_clks[] = {
};

static const struct gate_clock media2crg_gate_clks[] = {
	{ CLK_GATE_VDECFREQ, "clk_gate_vdecfreq", "clk_div_vdec", 0x00, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 21, 0, "clk_vdecfreq" },
	{ PCLK_GATE_VDEC, "pclk_gate_vdec", "clk_div_sysbus", 0x00, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_vdec" },
	{ ACLK_GATE_VDEC, "aclk_gate_vdec", "clk_div_vcodecbus", 0x00, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_vdec" },
	{ CLK_GATE_VENCFREQ, "clk_gate_vencfreq", "clk_div_venc", 0x00, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 24, 0, "clk_vencfreq" },
	{ PCLK_GATE_VENC, "pclk_gate_venc", "clk_div_sysbus", 0x00, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_venc" },
	{ ACLK_GATE_VENC, "aclk_gate_venc", "clk_div_vcodecbus", 0x00, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_venc" },
	{ ACLK_GATE_VENC2, "aclk_gate_venc2", "clk_div_vcodecbus", 0x00, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_venc2" },
	{ CLK_GATE_VENC2FREQ, "clk_gate_venc2freq", "clk_div_venc2", 0x00, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 25, 0, "clk_venc2freq" },
	{ PCLK_GATE_MEDIA2_LM, "pclk_gate_media2_lm", "clk_div_ptp", 0x00, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_media2_lm" },
	{ CLK_GATE_LOADMONITOR_MEDIA2, "clk_gate_loadmonitor_media2", "clk_div_ptp", 0x00, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor_media2" },
	{ CLK_GATE_IVP32DSP_TCXO, "clk_gate_ivp32dsp_tcxo", "clk_gate_media_tcxo", 0x00, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ivpdsp_tcxo" },
	{ CLK_GATE_IVP32DSP_COREFREQ, "clk_gate_ivp32dsp_corefreq", "clk_div_ivp32dsp_core", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 27, 0, "clk_ivpdsp_corefreq" },
	{ CLK_GATE_AUTODIV_VCODECBUS, "clk_gate_autodiv_vcodecbus", "clk_div_vcodecbus", 0x00, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_vcbus" },
	{ CLK_GATE_ATDIV_VDEC, "clk_gate_atdiv_vdec", "clk_div_vdec", 0x00, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_vdec" },
	{ CLK_GATE_ATDIV_VENC, "clk_gate_atdiv_venc", "clk_div_venc", 0x00, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_venc" },
};

static const struct scgt_clock media2crg_scgt_clks[] = {
};

static const struct div_clock pctrl_div_clks[] = {
};

static const struct mux_clock pctrl_mux_clks[] = {
};

static const struct gate_clock pctrl_gate_clks[] = {
};

static const struct scgt_clock pctrl_scgt_clks[] = {
	{ CLK_GATE_USB_TCXO_EN, "clk_gate_usb_tcxo_en", "clk_gate_abb_192",
		0x10, 0, CLK_GATE_HIWORD_MASK, "clk_usb_tcxo_en" },
};

static const struct xfreq_clock xfreq_clks[] = {
	{ CLK_CLUSTER0, "clk_cluster0", 0, 0, 0, 0x41C, {0x0001030A, 0x0}, {0x0001020A, 0x0}, "cpu-cluster.0" },
	{ CLK_CLUSTER1, "clk_cluster1", 1, 0, 0, 0x41C, {0x0002030A, 0x0}, {0x0002020A, 0x0}, "cpu-cluster.1" },
	{ CLK_G3D, "clk_g3d", 2, 0, 0, 0x41C, {0x0003030A, 0x0}, {0x0003020A, 0x0}, "clk_g3d" },
	{ CLK_DDRC_FREQ, "clk_ddrc_freq", 3, 0, 0, 0x41C, {0x00040309, 0x0}, {0x0004020A, 0x0}, "clk_ddrc_freq" },
	{ CLK_DDRC_MAX, "clk_ddrc_max", 5, 1, 0x8000, 0x250, {0x00040308, 0x0}, {0x0004020A, 0x0}, "clk_ddrc_max" },
	{ CLK_DDRC_MIN, "clk_ddrc_min", 4, 1, 0x8000, 0x270, {0x00040309, 0x0}, {0x0004020A, 0x0}, "clk_ddrc_min" },
};

static const struct pmu_clock pmu_clks[] = {
	{ CLK_GATE_ABB_192, "clk_gate_abb_192", "clkin_sys", 0x03C, 0, 9, 0, "clk_abb_192" },
	{ CLK_PMU32KA, "clk_pmu32ka", "clkin_ref", 0x47, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmu32ka" },
	{ CLK_PMU32KB, "clk_pmu32kb", "clkin_ref", 0x46, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmu32kb" },
	{ CLK_PMU32KC, "clk_pmu32kc", "clkin_ref", 0x45, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmu32kc" },
	{ CLK_PMUAUDIOCLK, "clk_pmuaudioclk", "clk_sys_ini", 0x42, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmuaudioclk" },
};

static const struct mailbox_clock mailbox_clks[] = {
	{ CLK_GATE_VIVOBUS, "clk_gate_vivobus", "clk_ppll0", 0,
		{0x000D0002, 0xC}, {0x000D0102, 0xC}, 0, "clk_vivobus" },
	{ CLK_GATE_VCODECBUS, "clk_gate_vcodecbus", "clk_div_vcodecbus", 0,
		{0x000D0002, 0xE}, {0x000D0102, 0xE}, 0, "clk_vcodecbus" },
	{ CLK_SPLL_VOTE, "clk_spll_vote", "clk_spll", 0,
		{0x000D0002, 0x0}, {0x000D0102, 0x0}, 0, "clk_spll_vote" },
};

static const struct dvfs_clock dvfs_clks[] = {
	{ CLK_GATE_VDEC, "clk_gate_vdec", "clk_gate_vdecfreq", 21, 5, 3, 1,
		{185000, 277000, 332000}, {0, 1, 2, 3}, 332000, 1, "clk_vdec" },
	{ CLK_GATE_VENC, "clk_gate_venc", "clk_gate_vencfreq", 24, 4, 3, 1,
		{207500, 332000, 480000}, {0, 1, 2, 3}, 480000, 1, "clk_venc" },
	{ CLK_GATE_VENC2, "clk_gate_venc2", "clk_gate_venc2freq", 25, 7, 3, 1,
		{207500, 332000, 480000}, {0, 1, 2, 3}, 480000, 1, "clk_venc2" },
	{ CLK_GATE_ISPFUNC, "clk_gate_ispfunc", "clk_gate_ispfuncfreq", 26, 2, 3, 1,
		{207500, 332000, 480000}, {0, 1, 2, 3}, 480000, 1, "clk_ispfunc" },
	{ CLK_GATE_JPG_FUNC, "clk_gate_jpg_func", "clk_gate_jpg_funcfreq", 36, -1, 3, 1,
		{238000, 332000, 415000}, {0, 1, 2, 3}, 415000, 1, "clk_jpg_func" },
	{ CLK_GATE_FDAI_FUNC, "clk_gate_fdai_func", "clk_gate_fdai_funcfreq", 35, -1, 3, 1,
		{384000, 480000, 554000}, {0, 1, 2, 3}, 554000, 1, "clk_fdai_func" },
	{ CLK_GATE_MEDIA_COMMON, "clk_gate_media_common", "clk_gate_media_commonfreq", 34, -1, 3, 1,
		{238000, 332000, 415000}, {0, 1, 2, 3}, 415000, 1, "clk_media_common" },
	{ CLK_GATE_IVP32DSP_CORE, "clk_gate_ivp32dsp_core", "clk_gate_ivp32dsp_corefreq", 27, 6, 3, 1,
		{480000, 554000, 640000}, {0, 1, 2, 3}, 640000, 1, "clk_ivpdsp_core" },
	{ CLK_GATE_ISPFUNC2, "clk_gate_ispfunc2", "clk_gate_ispfunc2freq", 38, -1, 3, 1,
		{207500, 332000, 480000}, {0, 1, 2, 3}, 480000, 1, "clk_ispfunc2" },
	{ CLK_GATE_ISPFUNC3, "clk_gate_ispfunc3", "clk_gate_ispfunc3freq", 39, -1, 3, 1,
		{166000, 300000, 400000}, {0, 1, 2, 3}, 400000, 1, "clk_ispfunc3" },
	{ CLK_GATE_ISPFUNC4, "clk_gate_ispfunc4", "clk_gate_ispfunc4freq", 40, -1, 3, 1,
		{185000, 300000, 400000}, {0, 1, 2, 3}, 400000, 1, "clk_ispfunc4" },
	{ CLK_GATE_HIFACE, "clk_gate_hiface", "pclk", 41, -1, 3, 1,
		{600000, 650000, 700000}, {0, 1, 2, 3}, 0, 0, "clk_hiface" },
};

static void clk_crgctrl_init(struct device_node *np)
{
	struct clock_data *clk_crgctrl_data;
	int nr = ARRAY_SIZE(fixed_clks) +
		ARRAY_SIZE(pll_clks) +
		ARRAY_SIZE(crgctrl_scgt_clks) +
		ARRAY_SIZE(fixed_factor_clks) +
		ARRAY_SIZE(crgctrl_div_clks) +
		ARRAY_SIZE(crgctrl_mux_clks) +
		ARRAY_SIZE(crgctrl_gate_clks);

	clk_crgctrl_data = clk_init(np, nr);
	if (!clk_crgctrl_data)
		return;

	plat_clk_register_fixed_rate(fixed_clks,
		ARRAY_SIZE(fixed_clks), clk_crgctrl_data);

	plat_clk_register_pll(pll_clks,
		ARRAY_SIZE(pll_clks), clk_crgctrl_data);

	plat_clk_register_scgt(crgctrl_scgt_clks,
		ARRAY_SIZE(crgctrl_scgt_clks), clk_crgctrl_data);

	plat_clk_register_fixed_factor(fixed_factor_clks,
		ARRAY_SIZE(fixed_factor_clks), clk_crgctrl_data);

	plat_clk_register_divider(crgctrl_div_clks,
		ARRAY_SIZE(crgctrl_div_clks), clk_crgctrl_data);

	plat_clk_register_mux(crgctrl_mux_clks,
		ARRAY_SIZE(crgctrl_mux_clks), clk_crgctrl_data);

	plat_clk_register_gate(crgctrl_gate_clks,
		ARRAY_SIZE(crgctrl_gate_clks), clk_crgctrl_data);
}

CLK_OF_DECLARE_DRIVER(clk_peri_crgctrl,
	"hisilicon,clk-extreme-crgctrl", clk_crgctrl_init);

static void clk_hsdt_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(hsdtctrl_pll_clks) +
		ARRAY_SIZE(hsdtctrl_scgt_clks) +
		ARRAY_SIZE(hsdtctrl_div_clks) +
		ARRAY_SIZE(hsdtctrl_mux_clks) +
		ARRAY_SIZE(hsdtctrl_gate_clks);

	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_pll(hsdtctrl_pll_clks,
		ARRAY_SIZE(hsdtctrl_pll_clks), clk_data);

	plat_clk_register_scgt(hsdtctrl_scgt_clks,
		ARRAY_SIZE(hsdtctrl_scgt_clks), clk_data);

	plat_clk_register_divider(hsdtctrl_div_clks,
		ARRAY_SIZE(hsdtctrl_div_clks), clk_data);

	plat_clk_register_mux(hsdtctrl_mux_clks,
		ARRAY_SIZE(hsdtctrl_mux_clks), clk_data);

	plat_clk_register_gate(hsdtctrl_gate_clks,
		ARRAY_SIZE(hsdtctrl_gate_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_hsdtcrg,
	"hisilicon,clk-extreme-hsdt-crg", clk_hsdt_init);

static void clk_mmc0_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(mmc0ctrl_gate_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_gate(mmc0ctrl_gate_clks,
		ARRAY_SIZE(mmc0ctrl_gate_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_mmc0crg,
	"hisilicon,clk-extreme-mmc0-crg", clk_mmc0_init);

static void clk_sctrl_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(sctrl_scgt_clks) +
		ARRAY_SIZE(sctrl_div_clks) +
		ARRAY_SIZE(sctrl_mux_clks) +
		ARRAY_SIZE(sctrl_gate_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_scgt(sctrl_scgt_clks,
		ARRAY_SIZE(sctrl_scgt_clks), clk_data);

	plat_clk_register_divider(sctrl_div_clks,
		ARRAY_SIZE(sctrl_div_clks), clk_data);

	plat_clk_register_mux(sctrl_mux_clks,
		ARRAY_SIZE(sctrl_mux_clks), clk_data);

	plat_clk_register_gate(sctrl_gate_clks,
		ARRAY_SIZE(sctrl_gate_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_sctrl,
	"hisilicon,clk-extreme-sctrl", clk_sctrl_init);

static void clk_pctrl_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(pctrl_scgt_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_scgt(pctrl_scgt_clks,
		ARRAY_SIZE(pctrl_scgt_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_pctrl,
	"hisilicon,clk-extreme-pctrl", clk_pctrl_init);

static void clk_media1_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(media1crg_scgt_clks) +
		ARRAY_SIZE(media1crg_div_clks) +
		ARRAY_SIZE(media1crg_mux_clks) +
		ARRAY_SIZE(media1crg_gate_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_scgt(media1crg_scgt_clks,
		ARRAY_SIZE(media1crg_scgt_clks), clk_data);

	plat_clk_register_divider(media1crg_div_clks,
		ARRAY_SIZE(media1crg_div_clks), clk_data);

	plat_clk_register_mux(media1crg_mux_clks,
		ARRAY_SIZE(media1crg_mux_clks), clk_data);

	plat_clk_register_gate(media1crg_gate_clks,
		ARRAY_SIZE(media1crg_gate_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_media1crg,
	"hisilicon,clk-extreme-media1-crg", clk_media1_init);

static void clk_media2_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(media2crg_gate_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_gate(media2crg_gate_clks,
		ARRAY_SIZE(media2crg_gate_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_media2crg,
	"hisilicon,clk-extreme-media2-crg", clk_media2_init);

static void clk_iomcu_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(iomcu_gate_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_gate(iomcu_gate_clks,
		ARRAY_SIZE(iomcu_gate_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_iomcu,
	"hisilicon,clk-extreme-iomcu-crg", clk_iomcu_init);

static void clk_xfreq_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(xfreq_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_xfreq(xfreq_clks,
		ARRAY_SIZE(xfreq_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_xfreq,
	"hisilicon,clk-extreme-xfreq", clk_xfreq_init);

static void clk_pmu_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(pmu_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_clkpmu(pmu_clks,
		ARRAY_SIZE(pmu_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_pmu,
	"hisilicon,clk-extreme-pmu", clk_pmu_init);

static void clk_mailbox_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(mailbox_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_mclk(mailbox_clks,
		ARRAY_SIZE(mailbox_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_mailbox,
	"hisilicon,clk-extreme-mailbox", clk_mailbox_init);

static void clk_dvfs_init(struct device_node *np)
{
	struct clock_data *clk_data;
	int nr = ARRAY_SIZE(dvfs_clks);
	clk_data = clk_init(np, nr);
	if (!clk_data)
		return;

	plat_clk_register_dvfs_clk(dvfs_clks,
		ARRAY_SIZE(dvfs_clks), clk_data);
}

CLK_OF_DECLARE_DRIVER(clk_dvfs,
	"hisilicon,clk-extreme-dvfs", clk_dvfs_init);

