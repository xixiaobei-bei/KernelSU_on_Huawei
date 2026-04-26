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
#include "clk-bur.h"
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk-provider.h>

static const struct fixed_rate_clock fixed_clks[] = {
	{ CLKIN_SYS, "clkin_sys", NULL, 0, 38400000, "clkin_sys" },
	{ CLKIN_REF, "clkin_ref", NULL, 0, 32764, "clkin_ref" },
	{ CLK_FLL_SRC, "clk_fll_src", NULL, 0, 184000000, "clk_fll_src" },
	{ CLK_PPLL1, "clk_ppll1", NULL, 0, 10000000, "clk_ppll1" },
	{ CLK_PPLL2, "clk_ppll2", NULL, 0, 1920000000, "clk_ppll2" },
	{ CLK_PPLL3, "clk_ppll3", NULL, 0, 1200000000, "clk_ppll3" },
	{ CLK_PPLL4, "clk_ppll4", NULL, 0, 10000000, "clk_ppll4" },
	{ CLK_FNPLL1, "clk_fnpll1", NULL, 0, 1440000000, "clk_fnpll1" },
	{ CLK_FNPLL4, "clk_fnpll4", NULL, 0, 1720000000, "clk_fnpll4" },
	{ CLK_SPLL, "clk_spll", NULL, 0, 1622016000, "clk_spll" },
	{ CLK_MODEM_BASE, "clk_modem_base", NULL, 0, 49152000, "clk_modem_base" },
	{ CLK_LBINTPLL_1, "clk_lbintpll_1", NULL, 0, 49146757, "clk_lbintpll_1" },
	{ CLK_PPLL_PCIE, "clk_ppll_pcie", NULL, 0, 100000000, "clk_ppll_pcie" },
	{ PCLK, "apb_pclk", NULL, 0, 20000000, "apb_pclk" },
	{ CLK_UART0_DBG, "uart0clk_dbg", NULL, 0, 19200000, "uart0clk_dbg" },
	{ CLK_UART6, "uart6clk", NULL, 0, 19200000, "uart6clk" },
	{ OSC32K, "osc32khz", NULL, 0, 32764, "osc32khz" },
	{ OSC19M, "osc19mhz", NULL, 0, 19200000, "osc19mhz" },
	{ CLK_480M, "clk_480m", NULL, 0, 480000000, "clk_480m" },
	{ CLK_INVALID, "clk_invalid", NULL, 0, 10000000, "clk_invalid" },
	{ CLK_FPGA_1P92, "clk_fpga_1p92", NULL, 0, 1920000, "clk_fpga_1p92" },
	{ CLK_FPGA_2M, "clk_fpga_2m", NULL, 0, 2000000, "clk_fpga_2m" },
	{ CLK_FPGA_10M, "clk_fpga_10m", NULL, 0, 10000000, "clk_fpga_10m" },
	{ CLK_FPGA_19M, "clk_fpga_19m", NULL, 0, 19200000, "clk_fpga_19m" },
	{ CLK_FPGA_20M, "clk_fpga_20m", NULL, 0, 20000000, "clk_fpga_20m" },
	{ CLK_FPGA_24M, "clk_fpga_24m", NULL, 0, 24000000, "clk_fpga_24m" },
	{ CLK_FPGA_26M, "clk_fpga_26m", NULL, 0, 26000000, "clk_fpga_26m" },
	{ CLK_FPGA_27M, "clk_fpga_27m", NULL, 0, 27000000, "clk_fpga_27m" },
	{ CLK_FPGA_32M, "clk_fpga_32m", NULL, 0, 32000000, "clk_fpga_32m" },
	{ CLK_FPGA_40M, "clk_fpga_40m", NULL, 0, 40000000, "clk_fpga_40m" },
	{ CLK_FPGA_48M, "clk_fpga_48m", NULL, 0, 48000000, "clk_fpga_48m" },
	{ CLK_FPGA_50M, "clk_fpga_50m", NULL, 0, 50000000, "clk_fpga_50m" },
	{ CLK_FPGA_57M, "clk_fpga_57m", NULL, 0, 57000000, "clk_fpga_57m" },
	{ CLK_FPGA_60M, "clk_fpga_60m", NULL, 0, 60000000, "clk_fpga_60m" },
	{ CLK_FPGA_64M, "clk_fpga_64m", NULL, 0, 64000000, "clk_fpga_64m" },
	{ CLK_FPGA_80M, "clk_fpga_80m", NULL, 0, 80000000, "clk_fpga_80m" },
	{ CLK_FPGA_100M, "clk_fpga_100m", NULL, 0, 100000000, "clk_fpga_100m" },
	{ CLK_FPGA_160M, "clk_fpga_160m", NULL, 0, 160000000, "clk_fpga_160m" },
};

static const struct fixed_factor_clock fixed_factor_clks[] = {
	{ CLK_SYS_INI, "clk_sys_ini", "clkin_sys", 1, 2, 0, "clk_sys_ini" },
	{ CLK_DIV_SYSBUS, "div_sysbus_pll", "clk_spll", 1, 7, 0, "div_sysbus_pll" },
	{ CLK_GATE_WD0_HIGH, "clk_wd0_high", "sc_div_cfgbus", 1, 1, 0, "clk_wd0_high" },
	{ CLK_FACTOR_TCXO, "clk_factor_tcxo", "clk_sys_ini", 1, 4, 0, "clk_factor_tcxo" },
	{ CLK_GATE_TIMER5_A, "clk_timer5_a", "clkmux_timer5_a", 1, 1, 0, "clk_timer5_a" },
	{ ATCLK, "clk_at", "clk_atdvfs", 1, 1, 0, "clk_at" },
	{ TRACKCLKIN, "clk_track", "clkdiv_track", 1, 1, 0, "clk_track" },
	{ PCLK_DBG, "pclk_dbg", "pclkdiv_dbg", 1, 1, 0, "pclk_dbg" },
	{ CLK_DIV_CSSYSDBG, "clk_cssys_div", "autodiv_sysbus", 1, 1, 0, "clk_cssys_div" },
	{ CLK_GATE_CSSYSDBG, "clk_cssysdbg", "clk_dmabus_div", 1, 1, 0, "clk_cssysdbg" },
	{ CLK_GATE_DMA_IOMCU, "clk_dma_iomcu", "clk_fll_src", 1, 1, 0, "clk_dma_iomcu" },
	{ CLK_SD_SYS, "clk_sd_sys", "clk_sd_sys_gt", 1, 6, 0, "clk_sd_sys" },
	{ CLK_SDIO_SYS, "clk_sdio_sys", "clk_sdio_sys_gt", 1, 6, 0, "clk_sdio_sys" },
	{ CLK_DIV_A53HPM, "clk_a53hpm_div", "clk_a53hpm_gt", 1, 4, 0, "clk_a53hpm_div" },
	{ CLK_DIV_320M, "sc_div_320m", "gt_clk_320m_pll", 1, 5, 0, "sc_div_320m" },
	{ CLK_FACTOR_UART0, "clk_uart0_fac", "clkmux_uart0", 1, 1, 0, "clk_uart0_fac" },
	{ CLK_USB2PHY_REF_DIV, "clkdiv_usb2phy_ref", "clkgt_usbphy_38d4m_ref", 1, 2, 0, "clkdiv_usb2phy_ref" },
	{ CLK_GATE_UFSPHY_REF, "clk_ufsphy_ref", "clkin_sys", 1, 1, 0, "clk_ufsphy_ref" },
	{ CLK_GATE_UFSIO_REF, "clk_ufsio_ref", "clkin_sys", 1, 1, 0, "clk_ufsio_ref" },
	{ CLK_GATE_BLPWM, "clk_blpwm", "clk_fll_src", 1, 2, 0, "clk_blpwm" },
	{ CLK_SYSCNT_DIV, "clk_syscnt_div", "clk_sys_ini", 1, 10, 0, "clk_syscnt_div" },
	{ CLK_GATE_GPS_REF, "clk_gps_ref", "clk_sys_ini", 1, 1, 0, "clk_gps_ref" },
	{ CLK_MUX_GPS_REF, "clkmux_gps_ref", "clk_sys_ini", 1, 1, 0, "clkmux_gps_ref" },
	{ CLK_GATE_MDM2GPS0, "clk_mdm2gps0", "clk_modem_base", 1, 1, 0, "clk_mdm2gps0" },
	{ CLK_GATE_MDM2GPS1, "clk_mdm2gps1", "clk_modem_base", 1, 1, 0, "clk_mdm2gps1" },
	{ CLK_GATE_MDM2GPS2, "clk_mdm2gps2", "clk_modem_base", 1, 1, 0, "clk_mdm2gps2" },
	{ PCLK_GATE_DSI0, "pclk_dsi0", "clk_spll", 1, 1, 0, "pclk_dsi0" },
	{ PCLK_GATE_DSI1, "pclk_dsi1", "sc_div_cfgbus", 1, 1, 0, "pclk_dsi1" },
	{ CLK_GATE_LDI0, "clk_ldi0", "clk_invalid", 1, 1, 0, "clk_ldi0" },
	{ CLK_GATE_LDI1, "clk_ldi1", "clk_invalid", 1, 1, 0, "clk_ldi1" },
	{ EPS_VOLT_HIGH, "eps_volt_high", "peri_volt_hold", 1, 1, 0, "eps_volt_high" },
	{ EPS_VOLT_MIDDLE, "eps_volt_middle", "peri_volt_middle", 1, 1, 0, "eps_volt_middle" },
	{ EPS_VOLT_LOW, "eps_volt_low", "peri_volt_low", 1, 1, 0, "eps_volt_low" },
	{ VENC_VOLT_HOLD, "venc_volt_hold", "peri_volt_hold", 1, 1, 0, "venc_volt_hold" },
	{ VDEC_VOLT_HOLD, "vdec_volt_hold", "peri_volt_hold", 1, 1, 0, "vdec_volt_hold" },
	{ EDC_VOLT_HOLD, "edc_volt_hold", "peri_volt_hold", 1, 1, 0, "edc_volt_hold" },
	{ EFUSE_VOLT_HOLD, "efuse_volt_hold", "peri_volt_middle", 1, 1, 0, "efuse_volt_hold" },
	{ LDI0_VOLT_HOLD, "ldi0_volt_hold", "peri_volt_hold", 1, 1, 0, "ldi0_volt_hold" },
	{ HISE_VOLT_HOLD, "hise_volt_hold", "peri_volt_hold", 1, 1, 0, "hise_volt_hold" },
	{ CLK_GATE_DPCTRL_16M, "clk_dpctrl_16m", "clk_invalid", 1, 1, 0, "clk_dpctrl_16m" },
	{ PCLK_GATE_DPCTRL, "pclk_dpctrl", "clk_invalid", 1, 1, 0, "pclk_dpctrl" },
	{ ACLK_GATE_DPCTRL, "aclk_dpctrl", "clk_invalid", 1, 1, 0, "aclk_dpctrl" },
	{ CLK_ISP_SNCLK_FAC, "clk_fac_ispsn", "clk_ispsn_gt", 1, 14, 0, "clk_fac_ispsn" },
	{ CLK_GATE_TXDPHY1_CFG, "clk_txdphy1_cfg", "clk_invalid", 1, 1, 0, "clk_txdphy1_cfg" },
	{ CLK_GATE_TXDPHY1_REF, "clk_txdphy1_ref", "clk_invalid", 1, 1, 0, "clk_txdphy1_ref" },
	{ CLK_FACTOR_RXDPHY, "clk_rxdcfg_fac", "clk_rxdcfg_gt", 1, 6, 0, "clk_rxdcfg_fac" },
	{ CLK_GATE_FD_FUNC, "clk_fd_func", "clk_invalid", 1, 1, 0, "clk_fd_func" },
	{ CLK_GATE_I2C0, "clk_i2c0", "clk_fll_src", 1, 2, 0, "clk_i2c0" },
	{ CLK_GATE_I2C1, "clk_i2c1", "clk_i2c1_gt", 1, 2, 0, "clk_i2c1" },
	{ CLK_GATE_I2C2, "clk_i2c2", "clk_fll_src", 1, 2, 0, "clk_i2c2" },
	{ CLK_GATE_SPI0, "clk_spi0", "clk_fll_src", 1, 2, 0, "clk_spi0" },
	{ CLK_FAC_180M, "clkfac_180m", "clk_spll", 1, 8, 0, "clkfac_180m" },
	{ CLK_GATE_IOMCU_PERI0, "clk_gate_peri0", "clk_spll", 1, 1, 0, "clk_gate_peri0" },
	{ CLK_GATE_SPI2, "clk_spi2", "clk_fll_src", 1, 2, 0, "clk_spi2" },
	{ CLK_GATE_UART3, "clk_uart3", "clk_gate_peri0", 1, 6, 0, "clk_uart3" },
	{ CLK_GATE_UART8, "clk_uart8", "clk_gate_peri0", 1, 6, 0, "clk_uart8" },
	{ CLK_GATE_UART7, "clk_uart7", "clk_gate_peri0", 1, 6, 0, "clk_uart7" },
	{ CLK_GNSS_ABB, "clk_gnss_abb", "clk_abb_192", 1, 1, 0, "clk_gnss_abb" },
	{ AUTODIV_ISP_DVFS, "autodiv_isp_dvfs", "autodiv_sysbus", 1, 1, 0, "autodiv_isp_dvfs" },
};

static const struct gate_clock crgctrl_gate_clks[] = {
	{ CLK_GATE_SPLL_MEDIA, "clk_spll_media", "clk_spll", 0x410, 0x40000000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spll_media" },
	{ CLK_GATE_PPLL2_MEDIA, "clk_ppll2_media", "clk_ap_ppll2", 0x410, 0x8000000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ppll2_media" },
	{ CLK_GATE_PPLL3_MEDIA, "clk_ppll3_media", "clk_ap_ppll3", 0x410, 0x10000000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ppll3_media" },
	{ PCLK_GPIO0, "pclk_gpio0", "sc_div_cfgbus", 0x010, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio0" },
	{ PCLK_GPIO1, "pclk_gpio1", "sc_div_cfgbus", 0x010, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio1" },
	{ PCLK_GPIO2, "pclk_gpio2", "sc_div_cfgbus", 0x010, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio2" },
	{ PCLK_GPIO3, "pclk_gpio3", "sc_div_cfgbus", 0x010, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio3" },
	{ PCLK_GPIO4, "pclk_gpio4", "sc_div_cfgbus", 0x010, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio4" },
	{ PCLK_GPIO5, "pclk_gpio5", "sc_div_cfgbus", 0x010, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio5" },
	{ PCLK_GPIO6, "pclk_gpio6", "sc_div_cfgbus", 0x010, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio6" },
	{ PCLK_GPIO7, "pclk_gpio7", "sc_div_cfgbus", 0x010, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio7" },
	{ PCLK_GPIO8, "pclk_gpio8", "sc_div_cfgbus", 0x010, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio8" },
	{ PCLK_GPIO9, "pclk_gpio9", "sc_div_cfgbus", 0x010, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio9" },
	{ PCLK_GPIO10, "pclk_gpio10", "sc_div_cfgbus", 0x010, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio10" },
	{ PCLK_GPIO11, "pclk_gpio11", "sc_div_cfgbus", 0x010, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio11" },
	{ PCLK_GPIO12, "pclk_gpio12", "sc_div_cfgbus", 0x010, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio12" },
	{ PCLK_GPIO13, "pclk_gpio13", "sc_div_cfgbus", 0x010, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio13" },
	{ PCLK_GPIO14, "pclk_gpio14", "sc_div_cfgbus", 0x010, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio14" },
	{ PCLK_GPIO15, "pclk_gpio15", "sc_div_cfgbus", 0x010, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio15" },
	{ PCLK_GPIO16, "pclk_gpio16", "sc_div_cfgbus", 0x010, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio16" },
	{ PCLK_GPIO17, "pclk_gpio17", "sc_div_cfgbus", 0x010, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio17" },
	{ PCLK_GPIO18, "pclk_gpio18", "sc_div_cfgbus", 0x010, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio18" },
	{ PCLK_GPIO19, "pclk_gpio19", "sc_div_cfgbus", 0x010, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio19" },
	{ PCLK_GATE_WD0_HIGH, "pclk_wd0_high", "sc_div_cfgbus", 0x020, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_wd0_high" },
	{ PCLK_GATE_WD0, "pclk_wd0", "clk_wd0_mux", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_wd0" },
	{ PCLK_GATE_WD1, "pclk_wd1", "sc_div_cfgbus", 0x020, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_wd1" },
	{ CLK_GATE_CODECSSI, "clk_codecssi", "sel_codeccssi", 0x020, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_codecssi" },
	{ PCLK_GATE_CODECSSI, "pclk_codecssi", "sc_div_cfgbus", 0x020, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_codecssi" },
	{ CLK_ATDVFS, "clk_atdvfs", "clk_cssys_div", 0, 0x0, 0,
		NULL, 1, { 0, 0, 0 }, { 0, 1, 2, 3 }, 3, 19, 0, "clk_atdvfs" },
	{ ACLK_GATE_PERF_STAT, "aclk_perf_stat", "clk_dmabus_div", 0x040, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_perf_stat" },
	{ PCLK_GATE_PERF_STAT, "pclk_perf_stat", "clk_dmabus_div", 0x040, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_perf_stat" },
	{ CLK_GATE_PERF_STAT, "clk_perf_stat", "clk_perf_div", 0x040, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_perf_stat" },
	{ CLK_GATE_DMAC, "clk_dmac", "div_sysbus_pll", 0x030, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dmac" },
	{ CLK_GATE_SOCP_ACPU, "clk_socp_acpu", "clk_dmabus_div", 0x010, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_socp_acpu" },
	{ CLK_GATE_TCLK_SOCP, "tclk_socp", "clk_sys_ini", 0, 0x0, 0,
		"clk_socp_acpu", 0, {0}, {0}, 0, 0, 0, "tclk_socp" },
	{ CLK_GATE_IPF, "clk_ipf", "div_sysbus_pll", 0x000, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ipf" },
	{ CLK_GATE_IPF_PSAM, "clk_ipf_psam", "div_sysbus_pll", 0x000, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ipf_psam" },
	{ ACLK_GATE_MAA, "aclk_maa", "div_sysbus_pll", 0x470, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_maa" },
	{ CLK_GATE_MAA_REF, "clk_maa_ref", "clkin_ref", 0x470, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_maa_ref" },
	{ CLK_GATE_SPE, "clk_spe", "clk_spe_div", 0x470, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spe" },
	{ HCLK_GATE_SPE, "hclk_spe", "div_sysbus_pll", 0x470, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_spe" },
	{ CLK_GATE_SPE_REF, "clk_spe_ref", "clkin_ref", 0x470, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spe_ref" },
	{ ACLK_GATE_AXI_MEM, "aclk_axi_mem", "div_sysbus_pll", 0x470, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_axi_mem" },
	{ CLK_GATE_AXI_MEM, "clk_axi_mem", "div_sysbus_pll", 0x470, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_axi_mem" },
	{ CLK_GATE_AXI_MEM_GS, "clk_axi_mem_gs", "div_sysbus_pll", 0x470, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_axi_mem_gs" },
	{ CLK_GATE_SD, "clk_sd", "clk_sd_muxsys", 0x040, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 1, "clk_sd" },
	{ CLK_GATE_UART1, "clk_uart1", "clkmux_uarth", 0x020, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart1" },
	{ CLK_GATE_UART4, "clk_uart4", "clkmux_uarth", 0x020, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart4" },
	{ PCLK_GATE_UART1, "pclk_uart1", "clkmux_uarth", 0x020, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart1" },
	{ PCLK_GATE_UART4, "pclk_uart4", "clkmux_uarth", 0x020, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart4" },
	{ CLK_GATE_UART2, "clk_uart2", "clkmux_uartl", 0x020, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart2" },
	{ CLK_GATE_UART5, "clk_uart5", "clkmux_uartl", 0x020, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart5" },
	{ PCLK_GATE_UART2, "pclk_uart2", "clkmux_uartl", 0x020, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart2" },
	{ PCLK_GATE_UART5, "pclk_uart5", "clkmux_uartl", 0x020, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart5" },
	{ CLK_GATE_UART0, "clk_uart0", "clkmux_uart0", 0x020, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_uart0" },
	{ PCLK_GATE_UART0, "pclk_uart0", "clkmux_uart0", 0x020, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_uart0" },
	{ CLK_GATE_I2C3, "clk_i2c3", "clkmux_i2c", 0x020, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c3" },
	{ CLK_GATE_I2C4, "clk_i2c4", "clkmux_i2c", 0x020, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c4" },
	{ CLK_GATE_I2C6_ACPU, "clk_i2c6_acpu", "clkmux_i2c", 0x010, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c6_acpu" },
	{ CLK_GATE_I2C7, "clk_i2c7", "clkmux_i2c", 0x010, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c7" },
	{ CLK_GATE_I2C9, "clk_i2c9", "clkmux_i2c", 0x030, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c9" },
	{ PCLK_GATE_I2C3, "pclk_i2c3", "clkmux_i2c", 0x020, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c3" },
	{ PCLK_GATE_I2C4, "pclk_i2c4", "clkmux_i2c", 0x020, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c4" },
	{ PCLK_GATE_I2C6_ACPU, "pclk_i2c6_acpu", "clkmux_i2c", 0x010, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c6_acpu" },
	{ PCLK_GATE_I2C7, "pclk_i2c7", "clkmux_i2c", 0x010, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c7" },
	{ PCLK_GATE_I2C9, "pclk_i2c9", "clkmux_i2c", 0x030, 0x40000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i2c9" },
	{ CLK_GATE_I3C4, "clk_i3c4", "clkdiv_i3c4", 0x470, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i3c4" },
	{ PCLK_GATE_I3C4, "pclk_i3c4", "clkdiv_i3c4", 0x470, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_i3c4" },
	{ CLK_GATE_SPI1, "clk_spi1", "clkmux_spi1", 0x020, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spi1" },
	{ CLK_GATE_SPI4, "clk_spi4", "clkmux_spi4", 0x040, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spi4" },
	{ PCLK_GATE_SPI1, "pclk_spi1", "clkmux_spi1", 0x020, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi1" },
	{ PCLK_GATE_SPI4, "pclk_spi4", "clkmux_spi4", 0x040, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi4" },
	{ CLK_GATE_MMC_USBDP, "clk_mmc_usbdp", "clk_div_usbdp", 0x040, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mmc_usbdp" },
	{ CLK_USB2_DRD_32K, "clk_usb2_drd_32k", "clkin_ref", 0x050, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_usb2_drd_32k" },
	{ CLK_USB2DRD_REF, "clk_usb2drd_ref", "clk_sys_ini", 0x050, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_usb2drd_ref" },
	{ PCLK_GATE_PCTRL, "pclk_pctrl", "clk_ptp_div", 0x020, 0x80000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pctrl" },
	{ CLK_GATE_PWM, "clk_pwm", "clk_ptp_div", 0x020, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pwm" },
	{ CLK_GATE_AO_ASP, "clk_ao_asp", "clk_ao_asp_div", 0x000, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ao_asp" },
	{ PERI_VOLT_HOLD, "peri_volt_hold", "clk_sys_ini", 0, 0x0, 0,
		NULL, 1, { 0, 0, 0 }, { 0, 1, 2, 3 }, 3, 18, 0, "peri_volt_hold" },
	{ PERI_VOLT_MIDDLE, "peri_volt_middle", "clk_sys_ini", 0, 0x0, 0,
		NULL, 1, { 0, 0 }, { 0, 1, 2 }, 2, 22, 0, "peri_volt_middle" },
	{ PERI_VOLT_LOW, "peri_volt_low", "clk_sys_ini", 0, 0x0, 0,
		NULL, 1, {0}, { 0, 1 }, 1, 28, 0, "peri_volt_low" },
	{ CLK_GATE_ISP_I2C_MEDIA, "clk_isp_i2c_media", "clk_div_isp_i2c", 0x030, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_i2c_media" },
	{ CLK_GATE_ISP_SNCLK0, "clk_isp_snclk0", "clk_mux_ispsn0", 0x050, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk0" },
	{ CLK_GATE_ISP_SNCLK1, "clk_isp_snclk1", "clk_mux_ispsn1", 0x050, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk1" },
	{ CLK_GATE_ISP_SNCLK2, "clk_isp_snclk2", "clk_mux_ispsn2", 0x050, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk2" },
	{ CLK_GATE_ISP_SNCLK3, "clk_isp_snclk3", "clk_mux_ispsn3", 0x040, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_snclk3" },
	{ CLK_GATE_RXDPHY0_CFG, "clk_rxdphy0_cfg", "clk_rxdcfg_mux", 0x030, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy0_cfg" },
	{ CLK_GATE_RXDPHY1_CFG, "clk_rxdphy1_cfg", "clk_rxdcfg_mux", 0x030, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy1_cfg" },
	{ CLK_GATE_RXDPHY2_CFG, "clk_rxdphy2_cfg", "clk_rxdcfg_mux", 0x030, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy2_cfg" },
	{ CLK_GATE_RXDPHY3_CFG, "clk_rxdphy3_cfg", "clk_rxdcfg_mux", 0x030, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_rxdphy3_cfg" },
	{ CLK_GATE_TXDPHY0_CFG, "clk_txdphy0_cfg", "clk_sys_ini", 0x030, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_txdphy0_cfg" },
	{ CLK_GATE_TXDPHY0_REF, "clk_txdphy0_ref", "clk_sys_ini", 0x030, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_txdphy0_ref" },
	{ PCLK_GATE_LOADMONITOR, "pclk_loadmonitor", "sc_div_cfgbus", 0x020, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_loadmonitor" },
	{ CLK_GATE_LOADMONITOR, "clk_loadmonitor", "clk_loadmonitor_div", 0x020, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor" },
	{ PCLK_GATE_LOADMONITOR_L, "pclk_loadmonitor_l", "sc_div_cfgbus", 0x020, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_loadmonitor_l" },
	{ CLK_GATE_LOADMONITOR_L, "clk_loadmonitor_l", "clk_loadmonitor_div", 0x020, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor_l" },
	{ CLK_GATE_MEDIA_TCXO, "clk_media_tcxo", "clk_sys_ini", 0x040, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_media_tcxo" },
	{ CLK_PMU32KC, "clk_pmu32kc", "clkin_ref", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pmu32kc" },
	{ AUTODIV_ISP, "autodiv_isp", "autodiv_isp_dvfs", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "autodiv_isp" },
	{ CLK_GATE_ATDIV_MMC0, "clk_atdiv_mmc0", "clk_div_mmc0bus", 0x410, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_mmc0" },
	{ CLK_GATE_ATDIV_DMA, "clk_atdiv_dma", "clk_dmabus_div", 0x410, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_dma" },
	{ CLK_GATE_ATDIV_CFG, "clk_atdiv_cfg", "sc_div_cfgbus", 0x410, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_cfg" },
	{ CLK_GATE_ATDIV_SYS, "clk_atdiv_sys", "div_sysbus_pll", 0x410, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_sys" },
};

static const struct pll_clock pll_clks[] = {
	{ CLK_GATE_PPLL2, "clk_ap_ppll2", "clk_ppll2", 0x2,
		{ 0x954, 5 }, { 0x958, 5 }, { 0x95C, 5 }, { 0x808, 26 }, 2, 0, "clk_ap_ppll2" },
	{ CLK_GATE_PPLL3, "clk_ap_ppll3", "clk_ppll3", 0x3,
		{ 0x954, 10 }, { 0x958, 10 }, { 0x95C, 10 }, { 0x810, 26 }, 2, 0, "clk_ap_ppll3" },
	{ CLK_GATE_FNPLL4, "clk_ap_fnpll4", "clk_fnpll4", 0xB,
		{ 0x960, 0 }, { 0x964, 0 }, { 0x968, 0 }, { 0x818, 26 }, 2, 0, "clk_ap_fnpll4" },
};

static const struct scgt_clock crgctrl_scgt_clks[] = {
	{ AUTODIV_SYSBUS, "autodiv_sysbus", "div_sysbus_pll",
		0x404, 5, CLK_GATE_HIWORD_MASK, "autodiv_sysbus" },
	{ AUTODIV_EMMC0BUS, "autodiv_emmc0bus", "autodiv_sysbus",
		0x404, 1, CLK_GATE_HIWORD_MASK, "autodiv_emmc0bus" },
	{ CLK_PERF_DIV_GT, "clk_perf_gt", "sc_div_320m",
		0x0F0, 3, CLK_GATE_HIWORD_MASK, "clk_perf_gt" },
	{ CLK_GATE_CSSYS_ATCLK, "clk_cssys_atclk", "clk_dmabus_div",
		0x1A0, 9, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_cssys_atclk" },
	{ CLK_GATE_TIME_STAMP_GT, "clk_timestp_gt", "clk_sys_ini",
		0x0F0, 1, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_timestp_gt" },
	{ CLK_GATE_TIME_STAMP, "clk_time_stamp", "clk_timestp_div",
		0x1A0, 11, CLK_GATE_HIWORD_MASK, "clk_time_stamp" },
	{ CLK_GATE_SPE_GT, "clk_spe_gt", "sc_div_320m",
		0x70C, 9, CLK_GATE_HIWORD_MASK, "clk_spe_gt" },
	{ CLK_GATE_VCODECBUS_GT, "clk_vcodbus_gt", "clk_vcodbus_mux",
		0x0F0, 8, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_vcodbus_gt" },
	{ CLK_ANDGT_SD, "clk_sd_gt", "clk_sd_muxpll",
		0x0F4, 3, CLK_GATE_HIWORD_MASK, "clk_sd_gt" },
	{ CLK_SD_SYS_GT, "clk_sd_sys_gt", "clk_sys_ini",
		0x0F4, 5, CLK_GATE_HIWORD_MASK, "clk_sd_sys_gt" },
	{ CLK_SDIO_PERI_GT, "clk_sdio_peri_gt", "clk_ap_ppll2",
		0x0F0, 10, CLK_GATE_HIWORD_MASK, "clk_sdio_peri_gt" },
	{ CLK_A53HPM_ANDGT, "clk_a53hpm_gt", "clk_a53hpm_mux",
		0x0F4, 7, CLK_GATE_HIWORD_MASK, "clk_a53hpm_gt" },
	{ CLK_320M_PLL_GT, "gt_clk_320m_pll", "sc_sel_320m_pll",
		0x0F8, 10, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "gt_clk_320m_pll" },
	{ CLK_ANDGT_UARTH, "clkgt_uarth", "sc_div_320m",
		0x0F4, 11, CLK_GATE_HIWORD_MASK, "clkgt_uarth" },
	{ CLK_ANDGT_UARTL, "clkgt_uartl", "sc_div_320m",
		0x0F4, 10, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clkgt_uartl" },
	{ CLK_ANDGT_UART0, "clkgt_uart0", "sc_div_320m",
		0x0F4, 9, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clkgt_uart0" },
	{ CLK_ANDGT_I3C4, "clkgt_i3c4", "sc_div_320m",
		0x70C, 5, CLK_GATE_HIWORD_MASK, "clkgt_i3c4" },
	{ CLK_ANDGT_SPI1, "clkgt_spi1", "sc_div_320m",
		0x0F4, 13, CLK_GATE_HIWORD_MASK, "clkgt_spi1" },
	{ CLK_ANDGT_SPI4, "clkgt_spi4", "sc_div_320m",
		0x0F4, 14, CLK_GATE_HIWORD_MASK, "clkgt_spi4" },
	{ CLKANDGT_MMC_USBDP, "clkgt_mmc_usbdp", "clk_spll",
		0x70C, 2, CLK_GATE_HIWORD_MASK, "clkgt_mmc_usbdp" },
	{ CLK_ANDGT_PTP, "clk_ptp_gt", "sc_div_320m",
		0x0F8, 5, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_ptp_gt" },
	{ CLK_DIV_AO_ASP_GT, "clk_ao_asp_gt", "clkmux_ao_asp",
		0x0F4, 4, CLK_GATE_HIWORD_MASK, "clk_ao_asp_gt" },
	{ CLK_ANDGT_VDEC, "clkgt_vdec", "clkmux_vdec",
		0x0F0, 13, CLK_GATE_HIWORD_MASK, "clkgt_vdec" },
	{ CLK_ANDGT_VENC, "clkgt_venc", "clkmux_venc",
		0x0F0, 9, CLK_GATE_HIWORD_MASK, "clkgt_venc" },
	{ CLK_GT_ISP_I2C, "clk_gt_isp_i2c", "sc_div_320m",
		0x0F8, 4, CLK_GATE_HIWORD_MASK, "clk_gt_isp_i2c" },
	{ CLK_ISP_SNCLK_ANGT, "clk_ispsn_gt", "sc_div_320m",
		0x108, 2, CLK_GATE_HIWORD_MASK, "clk_ispsn_gt" },
	{ CLK_ANDGT_RXDPHY, "clk_rxdcfg_gt", "clk_a53hpm_div",
		0x0F0, 12, CLK_GATE_HIWORD_MASK, "clk_rxdcfg_gt" },
	{ CLK_GT_LOADMONITOR, "clk_loadmonitor_gt", "sc_div_320m",
		0x0F0, 5, CLK_GATE_HIWORD_MASK, "clk_loadmonitor_gt" },
	{ CLK_ANDGT_IVP32DSP_CORE, "clkgt_ivp32dsp", "clkmux_ivp32dsp",
		0x0F0, 7, CLK_GATE_HIWORD_MASK, "clkgt_ivp32dsp" },
	{ AUTODIV_CFGBUS, "autodiv_cfgbus", "autodiv_sysbus",
		0x404, 4, CLK_GATE_HIWORD_MASK, "autodiv_cfgbus" },
	{ AUTODIV_DMABUS, "autodiv_dmabus", "autodiv_sysbus",
		0x404, 3, CLK_GATE_HIWORD_MASK, "autodiv_dmabus" },
};

static const struct div_clock crgctrl_div_clks[] = {
	{ CLK_DIV_CFGBUS, "sc_div_cfgbus", "div_sysbus_pll",
		0x0EC, 0x3, 4, 1, 0, 0, 0, "sc_div_cfgbus" },
	{ PCLK_DIV_DBG, "pclkdiv_dbg", "clk_cssys_div",
		0x1A0, 0x6000, 4, 1, 0, 0, 0, "pclkdiv_dbg" },
	{ TRACKCLKIN_DIV, "clkdiv_track", "clk_cssys_div",
		0x128, 0x3000, 4, 1, 0, 0, 0, "clkdiv_track" },
	{ CLK_DIV_PERF_STAT, "clk_perf_div", "clk_perf_gt",
		0x0D0, 0xf000, 16, 1, 0, 0, 0, "clk_perf_div" },
	{ CLK_DIV_DMABUS, "clk_dmabus_div", "autodiv_dmabus",
		0x0EC, 0x8000, 2, 1, 0, 0, 0, "clk_dmabus_div" },
	{ CLK_DIV_TIME_STAMP, "clk_timestp_div", "clk_timestp_gt",
		0x1A0, 0x1c0, 8, 1, 0, 0, 0, "clk_timestp_div" },
	{ CLK_DIV_SPE, "clk_spe_div", "clk_spe_gt",
		0x710, 0xf0, 16, 1, 0, 0, 0, "clk_spe_div" },
	{ CLK_DIV_VCODECBUS, "clk_vcodbus_div", "clk_vcodbus_gt",
		0x0BC, 0x3f, 64, 1, 0, 0, 0, "clk_vcodbus_div" },
	{ CLK_DIV_MMC0BUS, "clk_div_mmc0bus", "div_sysbus_pll",
		0x0EC, 0x4000, 2, 1, 0, 0, 0, "clk_div_mmc0bus" },
	{ CLK_DIV_SD, "clk_sd_div", "clk_sd_gt",
		0x0B8, 0xf, 16, 1, 0, 0, 0, "clk_sd_div" },
	{ CLK_SDIO_PERI_DIV, "clk_sdio_peri_div", "clk_sdio_peri_gt",
		0x0B0, 0xf, 16, 1, 0, 0, 0, "clk_sdio_peri_div" },
	{ CLK_DIV_UARTH, "clkdiv_uarth", "clkgt_uarth",
		0x0B0, 0xf000, 16, 1, 0, 0, 0, "clkdiv_uarth" },
	{ CLK_DIV_UARTL, "clkdiv_uartl", "clkgt_uartl",
		0x0B0, 0xf00, 16, 1, 0, 0, 0, "clkdiv_uartl" },
	{ CLK_DIV_UART0, "clkdiv_uart0", "clkgt_uart0",
		0x0B0, 0xf0, 16, 1, 0, 0, 0, "clkdiv_uart0" },
	{ CLK_DIV_I2C, "clkdiv_i2c", "sc_div_320m",
		0x0E8, 0xf0, 16, 1, 0, 0, 0, "clkdiv_i2c" },
	{ CLK_DIV_I3C4, "clkdiv_i3c4", "clkgt_i3c4",
		0x710, 0xc00, 4, 1, 0, 0, 0, "clkdiv_i3c4" },
	{ CLK_DIV_SPI1, "clkdiv_spi1", "clkgt_spi1",
		0x0C4, 0xf000, 16, 1, 0, 0, 0, "clkdiv_spi1" },
	{ CLK_DIV_SPI4, "clkdiv_spi4", "clkgt_spi4",
		0x0D0, 0xf0, 16, 1, 0, 0, 0, "clkdiv_spi4" },
	{ CLK_DIV_USBDP, "clk_div_usbdp", "clkgt_mmc_usbdp",
		0x710, 0xf, 16, 1, 0, 0, 0, "clk_div_usbdp" },
	{ CLK_DIV_PTP, "clk_ptp_div", "clk_ptp_gt",
		0x0D8, 0xf, 16, 1, 0, 0, 0, "clk_ptp_div" },
	{ CLK_DIV_AO_ASP, "clk_ao_asp_div", "clk_ao_asp_gt",
		0x108, 0x3c0, 16, 1, 0, 0, 0, "clk_ao_asp_div" },
	{ CLK_DIV_VDEC, "clkdiv_vdec", "clkgt_vdec",
		0x0C4, 0x3f, 64, 1, 0, 0x118, 13, "clkdiv_vdec" },
	{ CLK_DIV_VENC, "clkdiv_venc", "clkgt_venc",
		0x0C0, 0x3f00, 64, 1, 0, 0x118, 12, "clkdiv_venc" },
	{ CLK_DIV_ISP_I2C, "clk_div_isp_i2c", "clk_gt_isp_i2c",
		0x0B8, 0x780, 16, 1, 0, 0, 0, "clk_div_isp_i2c" },
	{ CLK_ISP_SNCLK_DIV0, "clk_div_ispsn0", "clk_fac_ispsn",
		0x108, 0x3, 4, 1, 0, 0, 0, "clk_div_ispsn0" },
	{ CLK_ISP_SNCLK_DIV1, "clk_div_ispsn1", "clk_fac_ispsn",
		0x10C, 0xc000, 4, 1, 0, 0, 0, "clk_div_ispsn1" },
	{ CLK_ISP_SNCLK_DIV2, "clk_div_ispsn2", "clk_fac_ispsn",
		0x10C, 0x1800, 4, 1, 0, 0, 0, "clk_div_ispsn2" },
	{ CLK_ISP_SNCLK_DIV3, "clk_div_ispsn3", "clk_fac_ispsn",
		0x100, 0x300, 4, 1, 0, 0, 0, "clk_div_ispsn3" },
	{ CLK_DIV_LOADMONITOR, "clk_loadmonitor_div", "clk_loadmonitor_gt",
		0x0B8, 0xc000, 4, 1, 0, 0, 0, "clk_loadmonitor_div" },
	{ CLK_DIV_IVP32DSP_CORE, "clkdiv_ivpdsp", "clkgt_ivp32dsp",
		0x0B4, 0x3f, 64, 1, 0, 0x500, 24, "clkdiv_ivpdsp" },
};

static const char * const clk_mux_wd0_p [] = { "clkin_ref", "pclk_wd0_high" };
static const char * const codeccssi_mux_p [] = { "clk_sys_ini", "clk_sys_ini" };
static const char * const clk_mux_vcodecbus_p [] = {
		"clk_invalid", "clk_spll", "clk_sys_ini", "clk_invalid",
		"clk_ap_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ap_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_sd_sys_p [] = { "clk_sd_sys", "clk_sd_div" };
static const char * const clk_mux_sd_pll_p [] = { "clk_spll", "clk_ap_ppll3", "clk_ap_ppll2", "clk_ap_ppll2" };
static const char * const clk_mux_a53hpm_p [] = { "clk_spll", "clk_ap_ppll2" };
static const char * const clk_mux_320m_p [] = { "clk_ap_ppll2", "clk_spll" };
static const char * const clk_mux_uarth_p [] = { "clk_sys_ini", "clkdiv_uarth" };
static const char * const clk_mux_uartl_p [] = { "clk_sys_ini", "clkdiv_uartl" };
static const char * const clk_mux_uart0_p [] = { "clk_sys_ini", "clkdiv_uart0" };
static const char * const clk_mux_i2c_p [] = { "clk_sys_ini", "clkdiv_i2c" };
static const char * const clk_mux_spi1_p [] = { "clk_sys_ini", "clkdiv_spi1" };
static const char * const clk_mux_spi4_p [] = { "clk_sys_ini", "clkdiv_spi4" };
static const char * const clk_mux_ao_asp_p [] = { "clk_ap_ppll2", "clk_spll", "clk_ap_ppll3", "clk_ap_ppll3" };
static const char * const clk_mux_vdec_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll", "clk_invalid",
		"clk_ap_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ap_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_venc_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll", "clk_invalid",
		"clk_ap_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ap_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_isp_snclk_mux0_p [] = { "clk_sys_ini", "clk_div_ispsn0" };
static const char * const clk_isp_snclk_mux1_p [] = { "clk_sys_ini", "clk_div_ispsn1" };
static const char * const clk_isp_snclk_mux2_p [] = { "clk_sys_ini", "clk_div_ispsn2" };
static const char * const clk_isp_snclk_mux3_p [] = { "clk_sys_ini", "clk_div_ispsn3" };
static const char * const clk_mux_rxdphy_cfg_p [] = { "clk_rxdcfg_fac", "clk_sys_ini" };
static const char * const clk_mux_ivp32dsp_core_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll", "clk_invalid",
		"clk_ap_ppll2", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ap_ppll3", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const struct mux_clock crgctrl_mux_clks[] = {
	{ CLK_MUX_WD0, "clk_wd0_mux", clk_mux_wd0_p,
		ARRAY_SIZE(clk_mux_wd0_p), 0x140, 0x20000, 0, "clk_wd0_mux" },
	{ CODECCSSI_MUX, "sel_codeccssi", codeccssi_mux_p,
		ARRAY_SIZE(codeccssi_mux_p), 0x0AC, 0x80, CLK_MUX_HIWORD_MASK, "sel_codeccssi" },
	{ CLK_MUX_VCODECBUS, "clk_vcodbus_mux", clk_mux_vcodecbus_p,
		ARRAY_SIZE(clk_mux_vcodecbus_p), 0x0C8, 0xf, CLK_MUX_HIWORD_MASK, "clk_vcodbus_mux" },
	{ CLK_MUX_SD_SYS, "clk_sd_muxsys", clk_mux_sd_sys_p,
		ARRAY_SIZE(clk_mux_sd_sys_p), 0x0B8, 0x40, CLK_MUX_HIWORD_MASK, "clk_sd_muxsys" },
	{ CLK_MUX_SD_PLL, "clk_sd_muxpll", clk_mux_sd_pll_p,
		ARRAY_SIZE(clk_mux_sd_pll_p), 0x0B8, 0x30, CLK_MUX_HIWORD_MASK, "clk_sd_muxpll" },
	{ CLK_MUX_A53HPM, "clk_a53hpm_mux", clk_mux_a53hpm_p,
		ARRAY_SIZE(clk_mux_a53hpm_p), 0x0D4, 0x200, CLK_MUX_HIWORD_MASK, "clk_a53hpm_mux" },
	{ CLK_MUX_320M, "sc_sel_320m_pll", clk_mux_320m_p,
		ARRAY_SIZE(clk_mux_320m_p), 0x100, 0x1, CLK_MUX_HIWORD_MASK, "sc_sel_320m_pll" },
	{ CLK_MUX_UARTH, "clkmux_uarth", clk_mux_uarth_p,
		ARRAY_SIZE(clk_mux_uarth_p), 0x0AC, 0x10, CLK_MUX_HIWORD_MASK, "clkmux_uarth" },
	{ CLK_MUX_UARTL, "clkmux_uartl", clk_mux_uartl_p,
		ARRAY_SIZE(clk_mux_uartl_p), 0x0AC, 0x8, CLK_MUX_HIWORD_MASK, "clkmux_uartl" },
	{ CLK_MUX_UART0, "clkmux_uart0", clk_mux_uart0_p,
		ARRAY_SIZE(clk_mux_uart0_p), 0x0AC, 0x4, CLK_MUX_HIWORD_MASK, "clkmux_uart0" },
	{ CLK_MUX_I2C, "clkmux_i2c", clk_mux_i2c_p,
		ARRAY_SIZE(clk_mux_i2c_p), 0x0AC, 0x2000, CLK_MUX_HIWORD_MASK, "clkmux_i2c" },
	{ CLK_MUX_SPI1, "clkmux_spi1", clk_mux_spi1_p,
		ARRAY_SIZE(clk_mux_spi1_p), 0x0AC, 0x100, CLK_MUX_HIWORD_MASK, "clkmux_spi1" },
	{ CLK_MUX_SPI4, "clkmux_spi4", clk_mux_spi4_p,
		ARRAY_SIZE(clk_mux_spi4_p), 0xAC, 0x1, CLK_MUX_HIWORD_MASK, "clkmux_spi4" },
	{ CLK_MUX_AO_ASP, "clkmux_ao_asp", clk_mux_ao_asp_p,
		ARRAY_SIZE(clk_mux_ao_asp_p), 0x100, 0x30, CLK_MUX_HIWORD_MASK, "clkmux_ao_asp" },
	{ CLK_MUX_VDEC, "clkmux_vdec", clk_mux_vdec_p,
		ARRAY_SIZE(clk_mux_vdec_p), 0x0C8, 0xf00, CLK_MUX_HIWORD_MASK, "clkmux_vdec" },
	{ CLK_MUX_VENC, "clkmux_venc", clk_mux_venc_p,
		ARRAY_SIZE(clk_mux_venc_p), 0x0C8, 0xf0, CLK_MUX_HIWORD_MASK, "clkmux_venc" },
	{ CLK_ISP_SNCLK_MUX0, "clk_mux_ispsn0", clk_isp_snclk_mux0_p,
		ARRAY_SIZE(clk_isp_snclk_mux0_p), 0x108, 0x8, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn0" },
	{ CLK_ISP_SNCLK_MUX1, "clk_mux_ispsn1", clk_isp_snclk_mux1_p,
		ARRAY_SIZE(clk_isp_snclk_mux1_p), 0x10C, 0x2000, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn1" },
	{ CLK_ISP_SNCLK_MUX2, "clk_mux_ispsn2", clk_isp_snclk_mux2_p,
		ARRAY_SIZE(clk_isp_snclk_mux2_p), 0x10C, 0x400, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn2" },
	{ CLK_ISP_SNCLK_MUX3, "clk_mux_ispsn3", clk_isp_snclk_mux3_p,
		ARRAY_SIZE(clk_isp_snclk_mux3_p), 0x100, 0x80, CLK_MUX_HIWORD_MASK, "clk_mux_ispsn3" },
	{ CLK_MUX_RXDPHY_CFG, "clk_rxdcfg_mux", clk_mux_rxdphy_cfg_p,
		ARRAY_SIZE(clk_mux_rxdphy_cfg_p), 0x0C4, 0x100, CLK_MUX_HIWORD_MASK, "clk_rxdcfg_mux" },
	{ CLK_MUX_IVP32DSP_CORE, "clkmux_ivp32dsp", clk_mux_ivp32dsp_core_p,
		ARRAY_SIZE(clk_mux_ivp32dsp_core_p), 0x708, 0xf000, CLK_MUX_HIWORD_MASK, "clkmux_ivp32dsp" },
};

static const struct div_clock hsdtctrl_div_clks[] = {
	{ CLK_DIV_SDIO, "clk_sdio_div", "clk_sdio_gt",
		0x0A8, 0xf, 16, 1, 0, 0, 0, "clk_sdio_div" },
};

static const char * const clk_mux_sdio_sys_p [] = { "clk_sdio_sys", "clk_sdio_div", "clk_sdio_peri_div", "clk_sdio_peri_div" };
static const struct mux_clock hsdtcrg_mux_clks[] = {
	{ CLK_MUX_SDIO_SYS, "clk_sdio_muxsy", clk_mux_sdio_sys_p,
		ARRAY_SIZE(clk_mux_sdio_sys_p), 0x0A8, 0x6000, CLK_MUX_HIWORD_MASK, "clk_sdio_muxsy" },
};

static const struct pll_clock hsdtcrg_pll_clks[] = {
	{ CLK_GATE_PCIEPLL, "clk_ap_pciepll", "clk_ppll_pcie", 0xD,
		{ 0x23C, 0 }, { 0, 0 }, { 0x23C, 1 }, { 0x208, 4 }, 10, 1, "clk_ap_pciepll" },
};

static const struct gate_clock hsdtctrl_gate_clks[] = {
	{ CLK_GATE_PCIEPHY_REF, "clk_pciephy_ref", "clk_ppll_pcie", 0x010, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pciephy_ref" },
	{ PCLK_GATE_PCIE_SYS, "pclk_pcie_sys", "clk_spll", 0x010, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pcie_sys" },
	{ PCLK_GATE_PCIE_PHY, "pclk_pcie_phy", "clk_spll", 0x010, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_pcie_phy" },
	{ ACLK_GATE_PCIE, "aclk_pcie", "clk_spll", 0x010, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_pcie" },
	{ CLK_GATE_HSDT_TBU, "clk_hsdt_tbu", "clk_spll", 0x010, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_hsdt_tbu" },
	{ CLK_GATE_HSDT_TCU, "clk_hsdt_tcu", "clk_spll", 0x010, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_hsdt_tcu" },
	{ CLK_GATE_PCIE0_MCU, "clk_pcie0_mcu", "clk_spll", 0x010, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcie0_mcu" },
	{ CLK_GATE_PCIE0_MCU_BUS, "clk_pcie0_mcu_bus", "clk_spll", 0x010, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcie0_mcu_bus" },
	{ CLK_GATE_PCIE0_MCU_19P2, "clk_pcie0_mcu_19p2", "clk_sys_ini", 0x010, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcie0_mcu_19p2" },
	{ CLK_GATE_PCIE0_MCU_32K, "clk_pcie0_mcu_32k", "clkin_ref", 0x010, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcie0_mcu_32k" },
	{ CLK_GATE_SDIO, "clk_sdio", "clk_sdio_muxsy", 0x000, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_sdio" },
	{ HCLK_GATE_SDIO, "hclk_sdio", "clk_spll", 0x000, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_sdio" },
};

static const struct scgt_clock hsdtctrl_scgt_clks[] = {
	{ CLK_ANDGT_SDIO, "clk_sdio_gt", "clk_spll",
		0x0AC, 1, CLK_GATE_HIWORD_MASK, "clk_sdio_gt" },
	{ CLK_SDIO_SYS_GT, "clk_sdio_sys_gt", "clk_sys_ini",
		0x0AC, 0, CLK_GATE_HIWORD_MASK, "clk_sdio_sys_gt" },
};

static const char * const clk_mux_usb19d2phy_p [] = { "clkin_sys", "clkdiv_usb2phy_ref" };
static const struct mux_clock mmc0crg_mux_clks[] = {
	{ CLK_MUX_USB19D2PHY, "clk_mux_usb19d2phy", clk_mux_usb19d2phy_p,
		ARRAY_SIZE(clk_mux_usb19d2phy_p), 0x010, 0x4, CLK_MUX_HIWORD_MASK, "clk_mux_usb19d2phy" },
};

static const struct gate_clock mmc0crg_gate_clks[] = {
	{ HCLK_GATE_SD, "hclk_sd", "clk_div_mmc0bus", 0x000, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_sd" },
	{ CLK_USB2PHY_REF, "clk_usb2phy_ref", "clk_mux_usb19d2phy", 0x000, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_usb2phy_ref" },
	{ PCLK_HISI_USB20PHY, "pclk_hisi_usb20phy", "clk_div_mmc0bus", 0x000, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_hisi_usb20phy" },
	{ PCLK_USB20_SYSCTRL, "pclk_usb20_sysctrl", "clk_div_mmc0bus", 0x000, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_usb20_sysctrl" },
	{ HCLK_USB2DRD, "hclk_usb2drd", "clk_mmc_usbdp", 0x000, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "hclk_usb2drd" },
};

static const struct scgt_clock hisi_mmc0crg_scgt_clks[] = {
	{ CLKANDGT_USB2PHY_REF, "clkgt_usbphy_38d4m_ref", "clkin_sys",
		0x010, 1, CLK_GATE_HIWORD_MASK, "clkgt_usbphy_38d4m_ref" },
};

static const struct div_clock sctrl_div_clks[] = {
	{ CLK_DIV_AOBUS, "sc_div_aobus", "clk_spll",
		0x254, 0x3f, 64, 1, 0, 0, 0, "sc_div_aobus" },
	{ CLK_PCIE_AUX_DIV, "clk_pcie_aux_div", "clk_pcie_aux_andgt",
		0x254, 0xf00, 16, 1, 0, 0, 0, "clk_pcie_aux_div" },
	{ CLK_DIV_IOPERI, "sc_div_ioperi", "clk_gt_ioperi",
		0x270, 0x3f, 64, 1, 0, 0, 0, "sc_div_ioperi" },
	{ CLK_DIV_SPI5, "clkdiv_spi5", "clkgt_spi5",
		0x294, 0x3f00, 64, 1, 0, 0, 0, "clkdiv_spi5" },
	{ CLKDIV_ASP_CODEC, "clkdiv_asp_codec", "clkgt_asp_codec",
		0x298, 0xfc0, 64, 1, 0, 0, 0, "clkdiv_asp_codec" },
	{ CLK_DIV_AO_LOADMONITOR, "clk_ao_loadmonitor_div", "clk_ao_loadmonitor_gt",
		0x26C, 0x3c00, 16, 1, 0, 0, 0, "clk_ao_loadmonitor_div" },
};

static const char * const clk_mux_timer5_a_p [] = { "clkin_ref", "apb_pclk", "clk_factor_tcxo", "apb_pclk" };
static const char * const clk_mux_timer5_b_p [] = { "clkin_ref", "apb_pclk", "clk_factor_tcxo", "apb_pclk" };
static const char * const clk_pcie_aux_mux_p [] = { "clk_sys_ini", "clk_pcie_aux_div" };
static const char * const clk_mux_ioperi_p [] = { "clk_fll_src", "clk_spll" };
static const char * const clk_mux_spi5_p [] = { "clk_fll_src", "clk_spll" };
static const char * const clkmux_syscnt_p [] = { "clk_syscnt_div", "clkin_ref" };
static const char * const clk_mux_asp_codec_p [] = { "clkdiv_asp_codec", "clk_asp_backup" };
static const char * const clk_mux_asp_pll_p [] = {
		"clk_invalid", "clk_spll", "clk_fll_src", "clk_invalid",
		"sel_ao_asp", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_sys_ini", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_ao_asp_mux_p [] = { "clk_ao_asp", "clkin_ref" };
static const char * const clk_sw_ao_loadmonitor_p [] = { "clk_fll_src", "clk_spll" };
static const struct mux_clock sctrl_mux_clks[] = {
	{ CLK_MUX_TIMER5_A, "clkmux_timer5_a", clk_mux_timer5_a_p,
		ARRAY_SIZE(clk_mux_timer5_a_p), 0x3C4, 0x30, 0, "clkmux_timer5_a" },
	{ CLK_MUX_TIMER5_B, "clkmux_timer5_b", clk_mux_timer5_b_p,
		ARRAY_SIZE(clk_mux_timer5_b_p), 0x3C4, 0xc0, 0, "clkmux_timer5_b" },
	{ CLK_PCIE_AUX_MUX, "clk_pcie_aux_mux", clk_pcie_aux_mux_p,
		ARRAY_SIZE(clk_pcie_aux_mux_p), 0x264, 0x4000, CLK_MUX_HIWORD_MASK, "clk_pcie_aux_mux" },
	{ CLK_MUX_IOPERI, "clk_ioperi_mux", clk_mux_ioperi_p,
		ARRAY_SIZE(clk_mux_ioperi_p), 0x294, 0x4000, CLK_MUX_HIWORD_MASK, "clk_ioperi_mux" },
	{ CLK_MUX_SPI5, "clkmux_spi5", clk_mux_spi5_p,
		ARRAY_SIZE(clk_mux_spi5_p), 0x294, 0x8000, CLK_MUX_HIWORD_MASK, "clkmux_spi5" },
	{ CLKMUX_SYSCNT, "clkmux_syscnt", clkmux_syscnt_p,
		ARRAY_SIZE(clkmux_syscnt_p), 0x264, 0x2000, CLK_MUX_HIWORD_MASK, "clkmux_syscnt" },
	{ CLK_MUX_ASP_CODEC, "sel_asp_codec", clk_mux_asp_codec_p,
		ARRAY_SIZE(clk_mux_asp_codec_p), 0x260, 0x8000, CLK_MUX_HIWORD_MASK, "sel_asp_codec" },
	{ CLK_MUX_ASP_PLL, "clk_asp_pll_sel", clk_mux_asp_pll_p,
		ARRAY_SIZE(clk_mux_asp_pll_p), 0x280, 0xf, CLK_MUX_HIWORD_MASK, "clk_asp_pll_sel" },
	{ CLK_AO_ASP_MUX, "sel_ao_asp", clk_ao_asp_mux_p,
		ARRAY_SIZE(clk_ao_asp_mux_p), 0x250, 0x1000, CLK_MUX_HIWORD_MASK, "sel_ao_asp" },
	{ CLK_SW_AO_LOADMONITOR, "clk_ao_loadmonitor_sw", clk_sw_ao_loadmonitor_p,
		ARRAY_SIZE(clk_sw_ao_loadmonitor_p), 0x26C, 0x100, CLK_MUX_HIWORD_MASK, "clk_ao_loadmonitor_sw" },
};

static const struct gate_clock sctrl_gate_clks[] = {
	{ PCLK_GPIO20, "pclk_gpio20", "sc_div_aobus", 0x1B0, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio20" },
	{ PCLK_GPIO21, "pclk_gpio21", "sc_div_aobus", 0x1B0, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_gpio21" },
	{ CLK_GATE_TIMER5_B, "clk_timer5_b", "clkmux_timer5_b", 0, 0x0, 0,
		"clk_timer5", 0, {0}, {0}, 0, 0, 0, "clk_timer5_b" },
	{ CLK_GATE_TIMER5, "clk_timer5", "sc_div_aobus", 0x170, 0x3000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_timer5" },
	{ CLK_GATE_PCIEAUX, "clk_pcieaux", "clk_pcie_aux_mux", 0x1B0, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_pcieaux" },
	{ CLK_GATE_SPI, "clk_spi3", "sc_div_ioperi", 0x1B0, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spi3" },
	{ PCLK_GATE_SPI, "pclk_spi3", "sc_div_ioperi", 0x1B0, 0x400, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi3" },
	{ CLK_GATE_SPI5, "clk_spi5", "clkdiv_spi5", 0x1B0, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_spi5" },
	{ PCLK_GATE_SPI5, "pclk_spi5", "clkdiv_spi5", 0x1B0, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_spi5" },
	{ PCLK_GATE_RTC, "pclk_rtc", "sc_div_aobus", 0x160, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_rtc" },
	{ PCLK_GATE_RTC1, "pclk_rtc1", "sc_div_aobus", 0x160, 0x4, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_rtc1" },
	{ PCLK_AO_GPIO0, "pclk_ao_gpio0", "sc_div_aobus", 0x160, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio0" },
	{ PCLK_AO_GPIO1, "pclk_ao_gpio1", "sc_div_aobus", 0x160, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio1" },
	{ PCLK_AO_GPIO2, "pclk_ao_gpio2", "sc_div_aobus", 0x160, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio2" },
	{ PCLK_AO_GPIO3, "pclk_ao_gpio3", "sc_div_aobus", 0x160, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio3" },
	{ PCLK_AO_GPIO4, "pclk_ao_gpio4", "sc_div_aobus", 0x160, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio4" },
	{ PCLK_AO_GPIO5, "pclk_ao_gpio5", "sc_div_aobus", 0x160, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio5" },
	{ PCLK_AO_GPIO6, "pclk_ao_gpio6", "sc_div_aobus", 0x160, 0x2000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio6" },
	{ PCLK_AO_GPIO29, "pclk_ao_gpio29", "sc_div_aobus", 0x1B0, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio29" },
	{ PCLK_AO_GPIO30, "pclk_ao_gpio30", "sc_div_aobus", 0x1B0, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio30" },
	{ PCLK_AO_GPIO31, "pclk_ao_gpio31", "sc_div_aobus", 0x1B0, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio31" },
	{ PCLK_AO_GPIO32, "pclk_ao_gpio32", "sc_div_aobus", 0x1B0, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio32" },
	{ PCLK_AO_GPIO33, "pclk_ao_gpio33", "sc_div_aobus", 0x1B0, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio33" },
	{ PCLK_AO_GPIO34, "pclk_ao_gpio34", "sc_div_aobus", 0x1B0, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio34" },
	{ PCLK_AO_GPIO35, "pclk_ao_gpio35", "sc_div_aobus", 0x1B0, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio35" },
	{ PCLK_AO_GPIO36, "pclk_ao_gpio36", "sc_div_aobus", 0x1B0, 0x20000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_gpio36" },
	{ PCLK_GATE_SYSCNT, "pclk_syscnt", "sc_div_aobus", 0x160, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_syscnt" },
	{ CLK_GATE_SYSCNT, "clk_syscnt", "clkmux_syscnt", 0x160, 0x100000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_syscnt" },
	{ CLK_ASP_BACKUP, "clk_asp_backup", "clkin_ref", 0x160, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_backup" },
	{ CLK_ASP_CODEC, "clk_asp_codec", "sel_asp_codec", 0x170, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_codec" },
	{ CLK_GATE_ASP_SUBSYS, "clk_asp_subsys", "clk_asp_pll_sel", 0x170, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_subsys" },
	{ CLK_GATE_ASP_TCXO, "clk_asp_tcxo", "clk_sys_ini", 0x160, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_asp_tcxo" },
	{ PCLK_GATE_AO_LOADMONITOR, "pclk_ao_loadmonitor", "sc_div_aobus", 0x1B0, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_ao_loadmonitor" },
	{ CLK_GATE_AO_LOADMONITOR, "clk_ao_loadmonitor", "clk_ao_loadmonitor_div", 0x1B0, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ao_loadmonitor" },
};

static const struct scgt_clock sctrl_scgt_clks[] = {
	{ CLK_PCIE_AUX_ANDGT, "clk_pcie_aux_andgt", "clk_fll_src",
		0x254, 7, CLK_GATE_HIWORD_MASK, "clk_pcie_aux_andgt" },
	{ CLK_ANDGT_IOPERI, "clk_gt_ioperi", "clk_ioperi_mux",
		0x270, 6, CLK_GATE_HIWORD_MASK, "clk_gt_ioperi" },
	{ CLK_ANDGT_SPI5, "clkgt_spi5", "clkmux_spi5",
		0x298, 12, CLK_GATE_HIWORD_MASK, "clkgt_spi5" },
	{ CLKGT_ASP_CODEC, "clkgt_asp_codec", "clk_spll_vote",
		0x274, 14, CLK_GATE_HIWORD_MASK, "clkgt_asp_codec" },
	{ CLK_GT_AO_LOADMONITOR, "clk_ao_loadmonitor_gt", "clk_ao_loadmonitor_sw",
		0x26C, 9, CLK_GATE_HIWORD_MASK, "clk_ao_loadmonitor_gt" },
};

static const struct gate_clock iomcu_gate_clks[] = {
	{ CLK_I2C1_GATE_IOMCU, "clk_i2c1_gt", "clk_fll_src", 0x010, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_i2c1_gt" },
};

static const struct div_clock media1crg_div_clks[] = {
	{ CLK_DIV_VIVOBUS, "clk_vivobus_div", "clk_vivobus_gt",
		0x074, 0x3f, 64, 1, 0, 0, 0, "clk_vivobus_div" },
	{ CLK_DIV_EDC0, "clk_edc0_div", "clk_edc0_gt",
		0x068, 0x3f, 64, 1, 0, 0x0A0, 7, "clk_edc0_div" },
	{ CLK_DIV_ISP_I3C, "clkdiv_isp_i3c", "clkgt_isp_i3c",
		0x064, 0xfc00, 64, 1, 0, 0, 0, "clkdiv_isp_i3c" },
	{ CLK_DIV_ISPCPU, "clk_ispcpu_div", "clk_ispcpu_gt",
		0x070, 0x3f, 64, 1, 0, 0x0A0, 5, "clk_ispcpu_div" },
	{ CLK_DIV_ISPFUNC, "clkdiv_ispfunc", "clkgt_ispfunc",
		0x06C, 0x3f, 64, 1, 0, 0x0A0, 6, "clkdiv_ispfunc" },
	{ CLK_DIV_ISPFUNC2, "clkdiv_ispfunc2", "clkgt_ispfunc2",
		0x088, 0x3f0, 64, 1, 0, 0x0A0, 2, "clkdiv_ispfunc2" },
	{ CLK_DIV_ISPFUNC3, "clkdiv_ispfunc3", "clkgt_ispfunc3",
		0x088, 0xfc00, 64, 1, 0, 0x0A0, 1, "clkdiv_ispfunc3" },
	{ CLK_DIV_JPG_FUNC, "clk_jpg_func_div", "clkgt_jpg_func",
		0x080, 0x3f, 64, 1, 0, 0x0A0, 12, "clk_jpg_func_div" },
	{ CLK_DIV_FDAI_FUNC, "clk_fdai_func_div", "clkgt_fdai_func",
		0x098, 0x3f, 64, 1, 0, 0, 0, "clk_fdai_func_div" },
	{ ACLK_DIV_MMBUF, "aclk_mmbuf_div", "clk_mmbuf_gt",
		0x07C, 0xfc00, 64, 1, 0, 0x0A0, 14, "aclk_mmbuf_div" },
	{ PCLK_DIV_MMBUF, "pclk_mmbuf_div", "pclk_mmbuf_gt",
		0x078, 0x3, 4, 1, 0, 0x0A0, 15, "pclk_mmbuf_div" },
};

static const char * const clk_mux_vivobus_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_edc0_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispi2c_p [] = { "clk_media_tcxo", "clk_isp_i2c_media" };
static const char * const clk_mux_ispcpu_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc2_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_mux_ispfunc3_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_jpg_func_mux_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const char * const clk_fdai_func_mux_p [] = { "clk_invalid", "clk_spll_media", "clk_ppll2_media", "clk_ppll3_media" };
static const char * const clk_sw_mmbuf_p [] = {
		"clk_invalid", "clk_invalid", "clk_spll_media", "clk_invalid",
		"clk_ppll2_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_ppll3_media", "clk_invalid", "clk_invalid", "clk_invalid",
		"clk_invalid", "clk_invalid", "clk_invalid", "clk_invalid"
};
static const struct mux_clock media1crg_mux_clks[] = {
	{ CLK_MUX_VIVOBUS, "clk_vivobus_mux", clk_mux_vivobus_p,
		ARRAY_SIZE(clk_mux_vivobus_p), 0x074, 0x3c0, CLK_MUX_HIWORD_MASK, "clk_vivobus_mux" },
	{ CLK_MUX_EDC0, "sel_edc0_pll", clk_mux_edc0_p,
		ARRAY_SIZE(clk_mux_edc0_p), 0x068, 0x3c0, CLK_MUX_HIWORD_MASK, "sel_edc0_pll" },
	{ CLK_MUX_ISPI2C, "clk_ispi2c_mux", clk_mux_ispi2c_p,
		ARRAY_SIZE(clk_mux_ispi2c_p), 0x078, 0x4, CLK_MUX_HIWORD_MASK, "clk_ispi2c_mux" },
	{ CLK_MUX_ISPCPU, "sel_ispcpu", clk_mux_ispcpu_p,
		ARRAY_SIZE(clk_mux_ispcpu_p), 0x060, 0x3c00, CLK_MUX_HIWORD_MASK, "sel_ispcpu" },
	{ CLK_MUX_ISPFUNC, "clkmux_ispfunc", clk_mux_ispfunc_p,
		ARRAY_SIZE(clk_mux_ispfunc_p), 0x06C, 0x3c0, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc" },
	{ CLK_MUX_ISPFUNC2, "clkmux_ispfunc2", clk_mux_ispfunc2_p,
		ARRAY_SIZE(clk_mux_ispfunc2_p), 0x06C, 0x3c00, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc2" },
	{ CLK_MUX_ISPFUNC3, "clkmux_ispfunc3", clk_mux_ispfunc3_p,
		ARRAY_SIZE(clk_mux_ispfunc3_p), 0x070, 0x3c00, CLK_MUX_HIWORD_MASK, "clkmux_ispfunc3" },
	{ CLK_JPG_FUNC_MUX, "clkmux_jpg_func", clk_jpg_func_mux_p,
		ARRAY_SIZE(clk_jpg_func_mux_p), 0x060, 0x3c0, CLK_MUX_HIWORD_MASK, "clkmux_jpg_func" },
	{ CLK_FDAI_FUNC_MUX, "clkmux_fdai_func", clk_fdai_func_mux_p,
		ARRAY_SIZE(clk_fdai_func_mux_p), 0x098, 0xc0, CLK_MUX_HIWORD_MASK, "clkmux_fdai_func" },
	{ CLK_SW_MMBUF, "aclk_mmbuf_sw", clk_sw_mmbuf_p,
		ARRAY_SIZE(clk_sw_mmbuf_p), 0x088, 0xf, CLK_MUX_HIWORD_MASK, "aclk_mmbuf_sw" },
};

static const struct gate_clock media1crg_gate_clks[] = {
	{ CLK_GATE_VIVOBUS, "clk_vivobus", "clk_vivobus_div", 0x000, 0x40000, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vivobus" },
	{ PCLK_GATE_ISP_NOC_SUBSYS, "pclk_isp_noc_subsys", "div_sysbus_pll", 0x010, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_isp_noc_subsys" },
	{ ACLK_GATE_ISP_NOC_SUBSYS, "aclk_isp_noc_subsys", "clk_spll", 0x010, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_isp_noc_subsys" },
	{ PCLK_GATE_DISP_NOC_SUBSYS, "pclk_disp_noc_subsys", "div_sysbus_pll", 0x010, 0x40000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_disp_noc_subsys" },
	{ ACLK_GATE_DISP_NOC_SUBSYS, "aclk_disp_noc_subsys", "clk_spll", 0x010, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_disp_noc_subsys" },
	{ PCLK_GATE_DSS, "pclk_dss", "pclk_disp_noc_subsys", 0x000, 0x4000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_dss" },
	{ ACLK_GATE_DSS, "aclk_dss", "aclk_disp_noc_subsys", 0x000, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_dss" },
	{ CLK_GATE_EDC0FREQ, "clk_edc0freq", "clk_edc0_div", 0x000, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_edc0freq" },
	{ ACLK_GATE_ISP, "aclk_isp", "aclk_isp_noc_subsys", 0x000, 0x100000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_isp" },
	{ CLK_GATE_ISPI2C, "clk_ispi2c", "clk_ispi2c_mux", 0x000, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispi2c" },
	{ CLK_GATE_ISP_SYS, "clk_isp_sys", "clk_media_tcxo", 0x000, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_sys" },
	{ CLK_GATE_ISP_I3C, "clk_isp_i3c", "clkdiv_isp_i3c", 0x010, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_isp_i3c" },
	{ CLK_GATE_ISPCPU, "clk_ispcpu", "clk_ispcpu_div", 0x000, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispcpu" },
	{ CLK_GATE_ISPFUNCFREQ, "clk_ispfuncfreq", "clkdiv_ispfunc", 0x000, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispfuncfreq" },
	{ CLK_GATE_ISPFUNC2FREQ, "clk_ispfunc2freq", "clkdiv_ispfunc2", 0x000, 0x400000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispfunc2freq" },
	{ CLK_GATE_ISPFUNC3FREQ, "clk_ispfunc3freq", "clkdiv_ispfunc3", 0x000, 0x200000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispfunc3freq" },
	{ PCLK_GATE_MEDIA1_LM, "pclk_media1_lm", "clk_ptp_div", 0x000, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_media1_lm" },
	{ CLK_GATE_LOADMONITOR_MEDIA1, "clk_loadmonitor_media1", "clk_ptp_div", 0x000, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor_media1" },
	{ CLK_GATE_JPG_FUNCFREQ, "clk_jpg_funcfreq", "clk_jpg_func_div", 0x010, 0x8000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_jpg_funcfreq" },
	{ ACLK_GATE_JPG, "aclk_jpg", "aclk_isp_noc_subsys", 0x010, 0x8000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_jpg" },
	{ PCLK_GATE_JPG, "pclk_jpg", "pclk_isp_noc_subsys", 0x010, 0x10000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_jpg" },
	{ ACLK_GATE_NOC_ISP, "aclk_noc_isp", "aclk_isp_noc_subsys", 0x010, 0x2000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_noc_isp" },
	{ PCLK_GATE_NOC_ISP, "pclk_noc_isp", "pclk_isp_noc_subsys", 0x010, 0x4000000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_noc_isp" },
	{ CLK_GATE_FDAI_FUNCFREQ, "clk_fdai_funcfreq", "clk_fdai_func_div", 0x010, 0x200, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_fdai_funcfreq" },
	{ ACLK_GATE_ASC, "aclk_asc", "clk_mmbuf", 0x020, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_asc" },
	{ CLK_GATE_DSS_AXI_MM, "clk_dss_axi_mm", "clk_mmbuf", 0x020, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_dss_axi_mm" },
	{ CLK_GATE_MMBUF, "clk_mmbuf", "aclk_mmbuf_div", 0x020, 0x1, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_mmbuf" },
	{ PCLK_GATE_MMBUF, "pclk_mmbuf", "pclk_mmbuf_div", 0x020, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_mmbuf" },
	{ CLK_GATE_ATDIV_VIVO, "clk_atdiv_vivo", "clk_vivobus_div", 0x010, 0x2, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_vivo" },
	{ CLK_GATE_ATDIV_ISPCPU, "clk_atdiv_ispcpu", "clk_ispcpu_div", 0x010, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_ispcpu" },
};

static const struct scgt_clock media1crg_scgt_clks[] = {
	{ CLK_GATE_VIVOBUS_ANDGT, "clk_vivobus_gt", "clk_vivobus_mux",
		0x084, 3, CLK_GATE_HIWORD_MASK | CLK_GATE_ALWAYS_ON_MASK, "clk_vivobus_gt" },
	{ CLK_ANDGT_EDC0, "clk_edc0_gt", "sel_edc0_pll",
		0x084, 7, CLK_GATE_HIWORD_MASK, "clk_edc0_gt" },
	{ CLK_ANDGT_ISP_I3C, "clkgt_isp_i3c", "clk_spll_media",
		0x084, 9, CLK_GATE_HIWORD_MASK, "clkgt_isp_i3c" },
	{ CLK_ANDGT_ISPCPU, "clk_ispcpu_gt", "sel_ispcpu",
		0x084, 5, CLK_GATE_HIWORD_MASK, "clk_ispcpu_gt" },
	{ CLK_ANDGT_ISPFUNC, "clkgt_ispfunc", "clkmux_ispfunc",
		0x084, 6, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc" },
	{ CLK_ANDGT_ISPFUNC2, "clkgt_ispfunc2", "clkmux_ispfunc2",
		0x084, 2, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc2" },
	{ CLK_ANDGT_ISPFUNC3, "clkgt_ispfunc3", "clkmux_ispfunc3",
		0x084, 1, CLK_GATE_HIWORD_MASK, "clkgt_ispfunc3" },
	{ CLK_ANDGT_JPG_FUNC, "clkgt_jpg_func", "clkmux_jpg_func",
		0x084, 12, CLK_GATE_HIWORD_MASK, "clkgt_jpg_func" },
	{ CLK_ANDGT_FDAI_FUNC, "clkgt_fdai_func", "clkmux_fdai_func",
		0x084, 10, CLK_GATE_HIWORD_MASK, "clkgt_fdai_func" },
	{ CLK_MMBUF_PLL_ANDGT, "clk_mmbuf_gt", "aclk_mmbuf_sw",
		0x084, 14, CLK_GATE_HIWORD_MASK, "clk_mmbuf_gt" },
	{ PCLK_MMBUF_ANDGT, "pclk_mmbuf_gt", "aclk_mmbuf_div",
		0x084, 15, CLK_GATE_HIWORD_MASK, "pclk_mmbuf_gt" },
};

static const struct gate_clock media2crg_gate_clks[] = {
	{ CLK_GATE_VCODECBUS, "clk_vcodecbus", "clk_vcodbus_div", 0x000, 0x200, ALWAYS_ON,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vcodecbus" },
	{ CLK_GATE_VDECFREQ, "clk_vdecfreq", "clkdiv_vdec", 0x000, 0x100, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vdecfreq" },
	{ PCLK_GATE_VDEC, "pclk_vdec", "div_sysbus_pll", 0x000, 0x40, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_vdec" },
	{ ACLK_GATE_VDEC, "aclk_vdec", "clk_vcodbus_div", 0x000, 0x80, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_vdec" },
	{ CLK_GATE_VENCFREQ, "clk_vencfreq", "clkdiv_venc", 0x000, 0x20, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vencfreq" },
	{ PCLK_GATE_VENC, "pclk_venc", "div_sysbus_pll", 0x000, 0x8, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_venc" },
	{ ACLK_GATE_VENC, "aclk_venc", "clk_vcodbus_div", 0x000, 0x10, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_venc" },
	{ PCLK_GATE_MEDIA2_LM, "pclk_media2_lm", "clk_ptp_div", 0x000, 0x20000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_media2_lm" },
	{ CLK_GATE_LOADMONITOR_MEDIA2, "clk_loadmonitor_media2", "clk_ptp_div", 0x000, 0x10000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_loadmonitor_media2" },
	{ CLK_GATE_IVP32DSP_TCXO, "clk_ivpdsp_tcxo", "clk_media_tcxo", 0x000, 0x80000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ivpdsp_tcxo" },
	{ CLK_GATE_IVP32DSP_COREFREQ, "clk_ivpdsp_corefreq", "clkdiv_ivpdsp", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ivpdsp_corefreq" },
	{ CLK_GATE_AUTODIV_VCODECBUS, "clk_atdiv_vcbus", "clk_vcodbus_div", 0x000, 0x800, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_vcbus" },
	{ CLK_GATE_ATDIV_VDEC, "clk_atdiv_vdec", "clkdiv_vdec", 0x000, 0x1000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_vdec" },
	{ CLK_GATE_ATDIV_VENC, "clk_atdiv_venc", "clkdiv_venc", 0x000, 0x2000, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_atdiv_venc" },
};

static const char * const clk_usb2phy_ref_mux_p [] = { "clkin_sys", "clk_abb_192" };
static const struct mux_clock pctrl_mux_clks[] = {
	{ CLK_USB2PHY_REF_MUX, "clk_usb2phy_mux", clk_usb2phy_ref_mux_p,
		ARRAY_SIZE(clk_usb2phy_ref_mux_p), 0x064, 0x2000000, 0, "clk_usb2phy_mux" },
};

static const struct xfreq_clock xfreq_clks[] = {
	{ CLK_CLUSTER0, "cpu-cluster.0", 0, 0, 0, 0x41C, { 0x0001030A, 0x0 }, { 0x0001020A, 0x0 }, "cpu-cluster.0" },
	{ CLK_CLUSTER1, "cpu-cluster.1", 1, 0, 0, 0x41C, { 0x0002030A, 0x0 }, { 0x0002020A, 0x0 }, "cpu-cluster.1" },
	{ CLK_G3D, "clk_g3d", 2, 0, 0, 0x41C, { 0x0003030A, 0x0 }, { 0x0003020A, 0x0 }, "clk_g3d" },
	{ CLK_DDRC_FREQ, "clk_ddrc_freq", 3, 0, 0, 0x41C, { 0x00040309, 0x0 }, { 0x0004020A, 0x0 }, "clk_ddrc_freq" },
	{ CLK_DDRC_MAX, "clk_ddrc_max", 5, 1, 0x8000, 0x250, { 0x00040308, 0x0 }, { 0x0004020A, 0x0 }, "clk_ddrc_max" },
	{ CLK_DDRC_MIN, "clk_ddrc_min", 4, 1, 0x8000, 0x270, { 0x00040309, 0x0 }, { 0x0004020A, 0x0 }, "clk_ddrc_min" },
	{ CLK_DMSS_MIN, "clk_dmss_min", 6, 1, 0x8000, 0x230, { 0x00040309, 0x0 }, { 0x0004020A, 0x0 }, "clk_dmss_min" },
};

static const struct pmu_clock pmu_clks[] = {
	{ CLK_GATE_ABB_192, "clk_abb_192", "clkin_sys", 0x049, 0, 9, 0, "clk_abb_192" },
	{ CLK_PMU32KA, "clk_pmu32ka", "clkin_ref", 0x051, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmu32ka" },
	{ CLK_PMU32KB, "clk_pmu32kb", "clkin_ref", 0x050, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmu32kb" },
	{ CLK_PMUAUDIOCLK, "clk_pmuaudioclk", "clk_sys_ini", 0x04E, 0, INVALID_HWSPINLOCK_ID, 0, "clk_pmuaudioclk" },
	{ CLK_GATE_NFC, "clk_nfc", "clk_sys_ini", 0x04B, 0, INVALID_HWSPINLOCK_ID, 0, "clk_nfc" },
};

static const struct mailbox_clock mailbox_clks[] = {
	{ CLK_SPLL_VOTE, "clk_spll_vote", "clk_spll", 0,
		{ 0x000D0002, 0x0 }, { 0x000D0102, 0x0 }, 0, "clk_spll_vote" },
};

static const struct dvfs_clock dvfs_clks[] = {
	{ CLK_GATE_EDC0, "clk_edc0", "clk_edc0freq", 20, -1, 3, 1,
		{ 214000, 325000, 406000 }, { 0, 1, 2, 3 }, 406000, 1, "clk_edc0" },
	{ CLK_GATE_VDEC, "clk_vdec", "clk_vdecfreq", 21, -1, 3, 1,
		{ 181000, 271000, 325000 }, { 0, 1, 2, 3 }, 325000, 1, "clk_vdec" },
	{ CLK_GATE_VENC, "clk_venc", "clk_vencfreq", 24, -1, 3, 1,
		{ 203000, 325000, 480000 }, { 0, 1, 2, 3 }, 480000, 1, "clk_venc" },
	{ CLK_GATE_ISPFUNC, "clk_ispfunc", "clk_ispfuncfreq", 26, -1, 3, 1,
		{ 181000, 271000, 325000 }, { 0, 1, 2, 3 }, 325000, 1, "clk_ispfunc" },
	{ CLK_GATE_JPG_FUNC, "clk_jpg_func", "clk_jpg_funcfreq", 36, -1, 3, 1,
		{ 271000, 384000, 406000 }, { 0, 1, 2, 3 }, 406000, 1, "clk_jpg_func" },
	{ CLK_GATE_FDAI_FUNC, "clk_fdai_func", "clk_fdai_funcfreq", 35, -1, 3, 1,
		{ 384000, 480000, 541000 }, { 0, 1, 2, 3 }, 541000, 1, "clk_fdai_func" },
	{ CLK_GATE_IVP32DSP_CORE, "clk_ivp32dsp_core", "clk_ivpdsp_corefreq", 27, -1, 3, 1,
		{ 480000, 541000, 640000 }, { 0, 1, 2, 3 }, 640000, 1, "clk_ivpdsp_core" },
	{ CLK_GATE_ISPFUNC2, "clk_ispfunc2", "clk_ispfunc2freq", 38, -1, 3, 1,
		{ 163000, 203000, 325000 }, { 0, 1, 2, 3 }, 325000, 1, "clk_ispfunc2" },
	{ CLK_GATE_ISPFUNC3, "clk_ispfunc3", "clk_ispfunc3freq", 39, -1, 3, 1,
		{ 163000, 203000, 325000 }, { 0, 1, 2, 3 }, 325000, 1, "clk_ispfunc3" },
	{ CLK_GATE_HIFACE, "clk_hiface", "clk_sys_ini", 41, -1, 3, 1,
		{ 138000, 192000, 240000 }, { 0, 1, 2, 3 }, 240000, 1, "clk_hiface" },
	{ CLK_GATE_NPU_DVFS, "clk_npu_dvfs", "clk_sys_ini", 62, -1, 3, 1,
		{ 384000, 480000, 640000 }, { 0, 1, 2, 3 }, 640000, 1, "clk_npu_dvfs" },
};

static const struct gate_clock media1_stub_clks[] = {
	{ CLK_MUX_VIVOBUS, "clk_vivobus_mux", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vivobus_mux" },
	{ CLK_DIV_VIVOBUS, "clk_vivobus_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_vivobus_div" },
	{ CLK_DIV_EDC0, "clk_edc0_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_edc0_div" },
	{ CLK_MUX_EDC0, "sel_edc0_pll", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "sel_edc0_pll" },
	{ CLK_MUX_ISPI2C, "clk_ispi2c_mux", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispi2c_mux" },
	{ CLK_DIV_ISP_I3C, "clkdiv_isp_i3c", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkdiv_isp_i3c" },
	{ CLK_MUX_ISPCPU, "sel_ispcpu", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "sel_ispcpu" },
	{ CLK_DIV_ISPCPU, "clk_ispcpu_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_ispcpu_div" },
	{ CLK_MUX_ISPFUNC, "clkmux_ispfunc", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkmux_ispfunc" },
	{ CLK_DIV_ISPFUNC, "clkdiv_ispfunc", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkdiv_ispfunc" },
	{ CLK_MUX_ISPFUNC2, "clkmux_ispfunc2", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkmux_ispfunc2" },
	{ CLK_DIV_ISPFUNC2, "clkdiv_ispfunc2", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkdiv_ispfunc2" },
	{ CLK_MUX_ISPFUNC3, "clkmux_ispfunc3", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkmux_ispfunc3" },
	{ CLK_DIV_ISPFUNC3, "clkdiv_ispfunc3", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkdiv_ispfunc3" },
	{ CLK_JPG_FUNC_MUX, "clkmux_jpg_func", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkmux_jpg_func" },
	{ CLK_DIV_JPG_FUNC, "clk_jpg_func_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_jpg_func_div" },
	{ CLK_FDAI_FUNC_MUX, "clkmux_fdai_func", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clkmux_fdai_func" },
	{ CLK_DIV_FDAI_FUNC, "clk_fdai_func_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "clk_fdai_func_div" },
	{ CLK_SW_MMBUF, "aclk_mmbuf_sw", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_mmbuf_sw" },
	{ ACLK_DIV_MMBUF, "aclk_mmbuf_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "aclk_mmbuf_div" },
	{ PCLK_DIV_MMBUF, "pclk_mmbuf_div", "clk_sys_ini", 0, 0x0, 0,
		NULL, 0, {0}, {0}, 0, 0, 0, "pclk_mmbuf_div" },
};

static void clk_crgctrl_init(struct device_node *np)
{
	struct clock_data *clk_crgctrl_data = NULL;
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
	struct clock_data *clk_hsdtcrg_data = NULL;
	int nr = ARRAY_SIZE(hsdtcrg_pll_clks) +
		 ARRAY_SIZE(hsdtctrl_div_clks) +
		 ARRAY_SIZE(hsdtcrg_mux_clks) +
		 ARRAY_SIZE(hsdtctrl_scgt_clks) +
		 ARRAY_SIZE(hsdtctrl_gate_clks);

	clk_hsdtcrg_data = clk_init(np, nr);
	if (!clk_hsdtcrg_data)
		return;

	plat_clk_register_pll(hsdtcrg_pll_clks,
		ARRAY_SIZE(hsdtcrg_pll_clks), clk_hsdtcrg_data);

	plat_clk_register_divider(hsdtctrl_div_clks,
		ARRAY_SIZE(hsdtctrl_div_clks), clk_hsdtcrg_data);

	plat_clk_register_mux(hsdtcrg_mux_clks,
		ARRAY_SIZE(hsdtcrg_mux_clks), clk_hsdtcrg_data);

	plat_clk_register_scgt(hsdtctrl_scgt_clks,
		ARRAY_SIZE(hsdtctrl_scgt_clks), clk_hsdtcrg_data);

	plat_clk_register_gate(hsdtctrl_gate_clks,
		ARRAY_SIZE(hsdtctrl_gate_clks), clk_hsdtcrg_data);
}

CLK_OF_DECLARE_DRIVER(clk_hsdtcrg,
	"hisilicon,clk-extreme-hsdt-crg", clk_hsdt_init);

static void clk_mmc0crg_init(struct device_node *np)
{
	struct clock_data *clk_mmc0crg_data = NULL;
	int nr = ARRAY_SIZE(mmc0crg_mux_clks) +
		 ARRAY_SIZE(mmc0crg_scgt_clks) +
		 ARRAY_SIZE(mmc0crg_gate_clks);

	clk_mmc0crg_data = clk_init(np, nr);
	if (!clk_mmc0crg_data)
		return;

	plat_clk_register_mux(mmc0crg_mux_clks,
		ARRAY_SIZE(mmc0crg_mux_clks), clk_mmc0crg_data);

	plat_clk_register_scgt(mmc0crg_scgt_clks,
		ARRAY_SIZE(mmc0crg_scgt_clks), clk_mmc0crg_data);

	plat_clk_register_gate(mmc0crg_gate_clks,
		ARRAY_SIZE(mmc0crg_gate_clks), clk_mmc0crg_data);
}

CLK_OF_DECLARE_DRIVER(clk_mmc0crg,
	"hisilicon,clk-extreme-mmc0-crg", clk_mmc0crg_init);

static void clk_sctrl_init(struct device_node *np)
{
	struct clock_data *clk_sctrl_data = NULL;
	int nr = ARRAY_SIZE(sctrl_div_clks) +
		 ARRAY_SIZE(sctrl_mux_clks) +
		 ARRAY_SIZE(sctrl_scgt_clks) +
		 ARRAY_SIZE(sctrl_gate_clks);

	clk_sctrl_data = clk_init(np, nr);
	if (!clk_sctrl_data)
		return;

	plat_clk_register_divider(sctrl_div_clks,
		ARRAY_SIZE(sctrl_div_clks), clk_sctrl_data);

	plat_clk_register_mux(sctrl_mux_clks,
		ARRAY_SIZE(sctrl_mux_clks), clk_sctrl_data);

	hisi_clk_register_scgt(sctrl_scgt_clks,
		ARRAY_SIZE(sctrl_scgt_clks), clk_sctrl_data);

	plat_clk_register_gate(sctrl_gate_clks,
		ARRAY_SIZE(sctrl_gate_clks), clk_sctrl_data);
}

CLK_OF_DECLARE_DRIVER(clk_sctrl,
	"hisilicon,clk-extreme-sctrl", clk_sctrl_init);

static void clk_pctrl_init(struct device_node *np)
{
	struct clock_data *clk_pctrl_data = NULL;
	int nr = ARRAY_SIZE(pctrl_mux_clks);

	clk_pctrl_data = clk_init(np, nr);
	if (!clk_pctrl_data)
		return;

	plat_clk_register_mux(pctrl_mux_clks,
		ARRAY_SIZE(pctrl_mux_clks), clk_pctrl_data);
}
CLK_OF_DECLARE_DRIVER(clk_pctrl,
	"hisilicon,clk-extreme-pctrl", clk_pctrl_init);

static void clk_iomcu_init(struct device_node *np)
{
	struct clock_data *clk_iomcu_data = NULL;
	int nr = ARRAY_SIZE(iomcu_gate_clks);

	clk_iomcu_data = clk_init(np, nr);
	if (!clk_iomcu_data)
		return;

	plat_clk_register_gate(iomcu_gate_clks,
		ARRAY_SIZE(iomcu_gate_clks), clk_iomcu_data);
}
CLK_OF_DECLARE_DRIVER(clk_iomcu,
	"hisilicon,clk-extreme-iomcu-crg", clk_iomcu_init);

static void clk_media1_init(struct device_node *np)
{
	int nr;
	struct clock_data *clk_media1_data = NULL;
#ifdef CONFIG_HISI_CLK_DEBUG
	int no_media = is_no_media();
#else
	int no_media = 0;
#endif

	if (no_media)
		nr = ARRAY_SIZE(media1_stub_clks) +
			ARRAY_SIZE(media1crg_scgt_clks) +
			ARRAY_SIZE(media1crg_gate_clks);
	else
		nr = ARRAY_SIZE(media1crg_div_clks) +
			ARRAY_SIZE(media1crg_mux_clks) +
			ARRAY_SIZE(media1crg_scgt_clks) +
			ARRAY_SIZE(media1crg_gate_clks);

	clk_media1_data = clk_init(np, nr);
	if (!clk_media1_data)
		return;

	if (no_media) {
		pr_err("[%s] mux/sw and div won't initialize!\n", __func__);
		plat_clk_register_gate(media1_stub_clks,
			ARRAY_SIZE(media1_stub_clks), clk_media1_data);
	} else {
		plat_clk_register_divider(media1crg_div_clks,
			ARRAY_SIZE(media1crg_div_clks), clk_media1_data);
		plat_clk_register_mux(media1crg_mux_clks,
			ARRAY_SIZE(media1crg_mux_clks), clk_media1_data);
	}

	plat_clk_register_scgt(media1crg_scgt_clks,
		ARRAY_SIZE(media1crg_scgt_clks), clk_media1_data);

	plat_clk_register_gate(media1crg_gate_clks,
		ARRAY_SIZE(media1crg_gate_clks), clk_media1_data);
}
CLK_OF_DECLARE_DRIVER(clk_media1crg,
	"hisilicon,clk-extreme-media1-crg", clk_media1_init);

static void clk_media2_init(struct device_node *np)
{
	struct clock_data *clk_media2_data = NULL;
	int nr = ARRAY_SIZE(media2crg_gate_clks);

	clk_media2_data = clk_init(np, nr);
	if (!clk_media2_data)
		return;

	plat_clk_register_gate(media2crg_gate_clks,
		ARRAY_SIZE(media2crg_gate_clks), clk_media2_data);
}
CLK_OF_DECLARE_DRIVER(clk_media2crg,
	"hisilicon,clk-extreme-media2-crg", clk_media2_init);

static void clk_xfreq_init(struct device_node *np)
{
	struct clock_data *clk_xfreq_data = NULL;
	int nr = ARRAY_SIZE(xfreq_clks);

	clk_xfreq_data = clk_init(np, nr);
	if (!clk_xfreq_data)
		return;

	plat_clk_register_xfreq(xfreq_clks,
		ARRAY_SIZE(xfreq_clks), clk_xfreq_data);
}
CLK_OF_DECLARE_DRIVER(clk_xfreq,
	"hisilicon,clk-extreme-xfreq", clk_xfreq_init);

static void clk_pmu_init(struct device_node *np)
{
	struct clock_data *clk_pmu_data = NULL;
	int nr = ARRAY_SIZE(pmu_clks);

	clk_pmu_data = clk_init(np, nr);
	if (!clk_pmu_data)
		return;

	plat_clk_register_clkpmu(pmu_clks,
		ARRAY_SIZE(pmu_clks), clk_pmu_data);
}
CLK_OF_DECLARE_DRIVER(clk_pmu,
	"hisilicon,clk-extreme-pmu", clk_pmu_init);

static void clk_interactive_init(struct device_node *np)
{
	struct clock_data *clk_interactive_data = NULL;
	int nr = ARRAY_SIZE(mailbox_clks);

	clk_interactive_data = clk_init(np, nr);
	if (!clk_interactive_data)
		return;

	plat_clk_register_mclk(mailbox_clks,
		ARRAY_SIZE(mailbox_clks), clk_interactive_data);
}
CLK_OF_DECLARE_DRIVER(clk_interactive,
	"hisilicon,interactive-clk", clk_interactive_init);


static void clk_dvfs_init(struct device_node *np)
{
	struct clock_data *clk_dvfs_data = NULL;
	int nr = ARRAY_SIZE(dvfs_clks);

	clk_dvfs_data = clk_init(np, nr);
	if (!clk_dvfs_data)
		return;

	plat_clk_register_dvfs_clk(dvfs_clks,
		ARRAY_SIZE(dvfs_clks), clk_dvfs_data);
}
CLK_OF_DECLARE_DRIVER(clk_dvfs,
	"hisilicon,clk-extreme-dvfs", clk_dvfs_init);

