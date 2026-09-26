/*
 * da_combine_v5_dsp_config.c
 *
 * dsp init
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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
#include "da_combine_v5_dsp_config.h"

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>
#include <linux/hisi/da_combine/da_combine_v5_regs.h>
#include <linux/hisi/da_combine/da_combine_v5_dsp_regs.h>
#include <linux/hisi/audio_log.h>
#include "download_image.h"
#include "dsp_utils.h"
#include "soundtrigger_dma_drv.h"

#define LOG_TAG "DA_combine_v5_config"

struct dspif_sc_fs {
	unsigned int reg_addr;
	int in_offset;
	int out_offset;
};

struct dspif_bypass {
	unsigned int id;
	unsigned int reg;
	unsigned int offset;
};

/* PLL frequency division 0x03: 4div, 0x03: 2div, 0x03: 1div */
const static unsigned int low_pll_div_tab[DSP_PLL_BUTT] = { 0x3, 0x1, 0x0 };

static struct dspif_sc_fs sc_fs_ctrl_regs[DA_COMBINE_DSP_IF_PORT_BUTT] = {
	{ SC_CODEC_MUX_CTRL4_REG, -1, DSPIF0_DOUT_VLD_SEL_OFFSET },
	{ DSPIF_VLD_CTRL0_REG, DSPIF1_DIN_VLD_SEL_OFFSET, DSPIF1_DOUT_VLD_SEL_OFFSET },
	{ SC_CODEC_MUX_CTRL5_REG, -1, DSPIF2_DOUT_VLD_SEL_OFFSET },
	{ DSPIF_VLD_CTRL1_REG, DSPIF3_DIN_VLD_SEL_OFFSET, DSPIF3_DOUT_VLD_SEL_OFFSET },
	{ SC_CODEC_MUX_CTRL6_REG, -1, DSPIF4_DOUT_VLD_SEL_OFFSET },
	{ DSPIF_VLD_CTRL2_REG, DSPIF5_DIN_VLD_SEL_OFFSET, DSPIF5_DOUT_VLD_SEL_OFFSET },
	{ SC_CODEC_MUX_CTRL7_REG, -1, DSPIF6_DOUT_VLD_SEL_OFFSET },
	{ DSPIF_VLD_CTRL3_REG, DSPIF7_DIN_VLD_SEL_OFFSET, DSPIF7_DOUT_VLD_SEL_OFFSET },
	{ DSPIF_VLD_CTRL4_REG, DSPIF8_DIN_VLD_SEL_OFFSET, DSPIF8_DOUT_VLD_SEL_OFFSET },
	{ DSPIF_VLD_CTRL5_REG, DSPIF9_DIN_VLD_SEL_OFFSET, DSPIF9_DOUT_VLD_SEL_OFFSET },
};

struct dspif_bypass if_bypass_regs[] = {
	{ DA_COMBINE_DSP_IF_PORT_0, SC_CODEC_MUX_CTRL8_REG,  S1_I_DSP_BYPASS_OFFSET },
	{ DA_COMBINE_DSP_IF_PORT_1, SC_CODEC_MUX_CTRL22_REG, S1_O_DSP_BYPASS_OFFSET },
	{ DA_COMBINE_DSP_IF_PORT_2, SC_CODEC_MUX_CTRL8_REG,  S2_I_DSP_BYPASS_OFFSET },
	{ DA_COMBINE_DSP_IF_PORT_3, SC_CODEC_MUX_CTRL22_REG, S2_O_DSP_BYPASS_OFFSET },
	{ DA_COMBINE_DSP_IF_PORT_4, SC_CODEC_MUX_CTRL8_REG,  S3_I_DSP_BYPASS_OFFSET },
	{ DA_COMBINE_DSP_IF_PORT_5, SC_CODEC_MUX_CTRL22_REG, S3_O_DSP_BYPASS_OFFSET },
	{ DA_COMBINE_DSP_IF_PORT_6, SC_CODEC_MUX_CTRL8_REG,  S4_I_DSP_BYPASS_OFFSET },
};

static enum low_pll_state g_low_pll_state;

static void set_runstall(bool pull_down)
{
	if (pull_down) {
		da_combine_dsp_reg_clr_bit(DSP_CTRL0_REG, HIFI_STATVECTORSEL_OFFSET);
		/* Pull down runstall of dsp */
		da_combine_dsp_reg_clr_bit(SC_DSP_CTRL0_REG, SC_DSP_SFT_RUNSTALL_OFFSET);
	} else {
		/* Pull up runstall of dsp */
		da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_DSP_SFT_RUNSTALL_OFFSET);
	}
}

static void set_watchdog_enable(bool enable)
{
	if (enable)
		da_combine_dsp_reg_set_bit(APB_CLK_CFG_REG, WD_PCLK_EN_OFFSET);
	else
		da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, WD_PCLK_EN_OFFSET);
}

static void set_i2c_enable(bool enable)
{
	if (enable)
		da_combine_dsp_reg_set_bit(CFG_CLK_CTRL_REG, INTF_I2C_CLK_EN_OFFSET);
	else
		da_combine_dsp_reg_clr_bit(CFG_CLK_CTRL_REG, INTF_I2C_CLK_EN_OFFSET);
}

static void notify_dsp(void)
{
	unsigned int wait_cnt = NOTIFY_DSP_WAIT_CNT;

	da_combine_dsp_reg_set_bit(DSP_NMI_REG, HIFI_IRQ_REQ_OFFSET);

	while (wait_cnt) {
		if ((da_combine_dsp_read_reg(DSP_NMI_REG) & 0xc) == 0x0)
			break;
		usleep_range(100, 110);
		wait_cnt--;
	}

	if (wait_cnt == 0)
		AUDIO_LOGE("dsp do not handle msg, DSP_NMI:0x%x",
			da_combine_dsp_read_reg(DSP_NMI_REG));
}

static void dsp_power_ctrl(bool enable)
{
	if (enable) {
		da_combine_dsp_reg_set_bit(DSP_CLK_CFG_REG, HIFI_DIV_CLK_EN_OFFSET);
		da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, HIFI_PERI_CLK_EN_OFFSET);
		da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_HIFI_CLK_EN_OFFSET);
		set_runstall(true);
	} else {
		da_combine_dsp_reg_clr_bit(DA_COMBINE_V5_DSP_CMD_STAT_VLD,
			DA_COMBINE_V5_DSP_CMD_STAT_VLD_OFFSET);
		set_runstall(false);
		da_combine_dsp_reg_clr_bit(SC_DSP_CTRL0_REG, SC_HIFI_CLK_EN_OFFSET);
		da_combine_dsp_reg_clr_bit(DSP_CTRL6_DMAC_REG, HIFI_PERI_CLK_EN_OFFSET);
		da_combine_dsp_reg_clr_bit(DSP_CLK_CFG_REG, HIFI_DIV_CLK_EN_OFFSET);
	}
}

static void dsp_autoclk_enable(void)
{
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, DW_AXI_X2P_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, DW_P2P_ASYNC_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, DMAC_ACLK0_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, DMAC_ACLK1_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, DW_X2X_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, UART_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, HIFI_PERI_CLK_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL6_DMAC_REG, PERI_DW_AXI_X2P_CG_EN_OFFSET);

	da_combine_dsp_reg_set_bit(APB_CLK_CFG_REG, APB_PD_PCLK_EN_OFFSET);
	da_combine_dsp_reg_set_bit(APB_CLK_CFG_REG, APB2RAM_PCLK_EN_OFFSET);
	da_combine_dsp_reg_set_bit(APB_CLK_CFG_REG, APB_CLK_EN_OFFSET);
}

static void peri_module_init(void)
{
	da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, WD_PCLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, TIMER0_PCLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, TIMER1_PCLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, GPIO_PCLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(DSP_PERI_CLKEN1_REG, I2C_MST_CLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(DSP_PERI_CLKEN1_REG, UART_CLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(I2C_TO_CTRL1_REG, UART_DIV_EN_OFFSET);

	da_combine_dsp_reg_set_bit(DSP_AXI_CLKEN1_REG, DW_AXI_GLB_CG_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_AXI_CLKEN1_REG, IOS_CG_EN_OFFSET);

	/* enable hifi auto clk */
	da_combine_dsp_write_reg(DSP_CG_CTRL0_REG, 0x0);
	da_combine_dsp_write_reg(DSP_CG_CTRL1_REG, 0x0);
}

static void dsp_rst_req_init(void)
{
	da_combine_dsp_reg_clr_bit(SW_RST_REQ_REG, DSP_PD_SRST_REQ_OFFSET);
	da_combine_dsp_reg_clr_bit(SW_RST_REQ_REG, HIFI_PERI_SRST_REQ_OFFSET);
	da_combine_dsp_reg_clr_bit(SW_RST_REQ_REG, HIFI_SRST_REQ_OFFSET);
	da_combine_dsp_reg_clr_bit(SW_RST_REQ_REG, HIFI_CORE_SRST_REQ_OFFSET);
}

static void debug_enable(void)
{
	da_combine_dsp_reg_set_bit(DSP_CTRL2_REG, HIFI_DEBUGRST_N_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_CTRL1_REG, HIFI_OCDHALTONRESET_N_OFFSET);

	da_combine_memset(DA_COMBINE_V5_AP_TO_DSP_MSG_ADDR, (size_t)DA_COMBINE_V5_AP_TO_DSP_MSG_SIZE);
	da_combine_memset(DA_COMBINE_V5_AP_DSP_CMD_ADDR, (size_t)DA_COMBINE_V5_AP_DSP_CMD_SIZE);
	da_combine_memset(DA_COMBINE_V5_DSP_TO_AP_MSG_ADDR, (size_t)DA_COMBINE_V5_DSP_TO_AP_MSG_SIZE);
	da_combine_memset(DA_COMBINE_V5_MLIBPARA_ADDR, (size_t)DA_COMBINE_V5_MLIB_PARA_MAX_SIZE);
}

static void dsp_init(void)
{
	/* reset dsp_pd_srst_req */
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, HIFI_CORE_SRST_REQ_OFFSET);
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, HIFI_SRST_REQ_OFFSET);
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, HIFI_PERI_SRST_REQ_OFFSET);
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, DSP_PD_SRST_REQ_OFFSET);

	/* close all dspif clocks */
	da_combine_dsp_write_reg(DA_COMBINE_V5_DSP_DSPIF_CLK_EN, 0x0);

	/* enable dsp debug */
	da_combine_dsp_reg_clr_bit(DSP_CTRL2_REG, HIFI_DEBUGRST_N_OFFSET);
	da_combine_dsp_reg_clr_bit(DSP_CTRL1_REG, HIFI_OCDHALTONRESET_N_OFFSET);

	/* power on dsp_top_mtcmos_ctrl */
	da_combine_dsp_reg_clr_bit(DSP_LP_CTRL0_REG, DSP_TOP_MTCMOS_CTRL_OFFSET);
	/* enable dsp_top_iso_ctrl */
	da_combine_dsp_reg_clr_bit(DSP_LP_CTRL0_REG, DSP_TOP_ISO_CTRL_OFFSET);
	/* enable axi lowpower bydefault */
	da_combine_dsp_reg_clr_bit(DSP_LP_CTRL0_REG, AXI_CSYSREQ_OFFSET);

	/* enable low power ctrl by default */
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_DSP_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(SC_DSP_CTRL0_REG, SC_DSP_BP_OFFSET);

	/* set runstall ctrl by software */
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_DSP_RUNSTALL_BP_OFFSET);
	/* set hifi clk ctrl by software */
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_DSP_HIFI_DIV_BP_OFFSET);

	/* enable hifi clk */
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_HIFI_ACLK_EN_OFFSET);
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_HIFI_CLK_EN_OFFSET);

	/* pull up runstall */
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_DSP_SFT_RUNSTALL_OFFSET);

	dsp_autoclk_enable();
	peri_module_init();
	dsp_rst_req_init();
	debug_enable();
}

static void dsp_deinit(void)
{
	/* Close watchdog clock */
	da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, WD_PCLK_EN_OFFSET);

	/* Close HIFI clock */
	da_combine_dsp_reg_clr_bit(SC_DSP_CTRL0_REG, SC_DSP_EN_OFFSET);

	/* Close APB clock */
	da_combine_dsp_reg_clr_bit(APB_CLK_CFG_REG, APB_PD_PCLK_EN_OFFSET);

	/* Close DSPIF clocks, and soft reset DSPIF */
	da_combine_dsp_write_reg(DA_COMBINE_V5_DSP_DSPIF_CLK_EN, 0x0);

	/* Enable isolation cell */
	da_combine_dsp_reg_set_bit(DSP_LP_CTRL0_REG, DSP_TOP_ISO_CTRL_OFFSET);

	/* Soft reset dsp */
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, HIFI_CORE_SRST_REQ_OFFSET);
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, HIFI_SRST_REQ_OFFSET);
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, HIFI_PERI_SRST_REQ_OFFSET);
	da_combine_dsp_reg_set_bit(SW_RST_REQ_REG, DSP_PD_SRST_REQ_OFFSET);

	/* Turn off power of power-off area */
	da_combine_dsp_reg_set_bit(DSP_LP_CTRL0_REG, DSP_TOP_MTCMOS_CTRL_OFFSET);

	/* Pull up runstall of dsp */
	da_combine_dsp_reg_set_bit(SC_DSP_CTRL0_REG, SC_DSP_SFT_RUNSTALL_OFFSET);
}

static void set_fasttrans_enable(bool enable)
{
	IN_FUNCTION;

	if (enable) {
		/* DSP IF 3 */
		da_combine_dsp_write_reg(DSPIF_VLD_CTRL1_REG, 0x36); /* 192K */
		/* DSP IF 1 */
		da_combine_dsp_write_reg(DSPIF_VLD_CTRL0_REG, 0xC); /* 16k in 48k out */

		da_combine_dsp_reg_write_bits(SLIM_UP_EN1_REG,
			0x1 << SLIM_UP1_EN_OFFSET | 0x1 << SLIM_UP2_EN_OFFSET,
			0x1 << SLIM_UP1_EN_OFFSET | 0x1 << SLIM_UP2_EN_OFFSET);
		da_combine_dsp_reg_write_bits(SLIM_UP_EN2_REG,
			0x1 << SLIM_UP9_EN_OFFSET,
			0x1 << SLIM_UP9_EN_OFFSET);

		da_combine_dsp_write_reg(SC_FS_SLIM_CTRL_7_REG,
			0x6 << FS_SLIM_UP9_OFFSET); /* 192K */
		da_combine_dsp_write_reg(SC_FS_SLIM_CTRL_3_REG, 0x44); /* 48K */
	} else {
		/* DSP IF 3 */
		da_combine_dsp_write_reg(DSPIF_VLD_CTRL1_REG, 0x24); /* 48K */

		da_combine_dsp_reg_write_bits(SLIM_UP_EN1_REG,
			0x0,
			0x1 << SLIM_UP1_EN_OFFSET | 0x1 << SLIM_UP2_EN_OFFSET);
		da_combine_dsp_reg_write_bits(SLIM_UP_EN2_REG,
			0x0,
			0x1 << SLIM_UP9_EN_OFFSET);
		da_combine_dsp_write_reg(SC_FS_SLIM_CTRL_7_REG,
			0x2 << FS_SLIM_UP9_OFFSET); /* 32K */
	}

	OUT_FUNCTION;
}

static void set_if_bypass(unsigned int dsp_if_id, bool bypass)
{
	unsigned int i;
	unsigned int bit_val;

	bit_val = bypass ? 1 : 0;

	if (dsp_if_id > DA_COMBINE_DSP_IF_PORT_7) {
		AUDIO_LOGI("dsp if %u do not need config bypass", dsp_if_id);
		return;
	}

	if (dsp_if_id == DA_COMBINE_DSP_IF_PORT_7) {
		if (bypass) {
			da_combine_dsp_reg_write_bits(SC_CODEC_MUX_CTRL37_REG,
				1 << SPA_OR_SRC_DIN_SEL_OFFSET,
				3 << SPA_OR_SRC_DIN_SEL_OFFSET);
			da_combine_dsp_reg_write_bits(SC_CODEC_MUX_CTRL37_REG,
				1 << SPA_OL_SRC_DIN_SEL_OFFSET,
				3 << SPA_OL_SRC_DIN_SEL_OFFSET);
		} else {
			da_combine_dsp_reg_write_bits(SC_CODEC_MUX_CTRL37_REG,
				2 << SPA_OR_SRC_DIN_SEL_OFFSET,
				3 << SPA_OR_SRC_DIN_SEL_OFFSET);
			da_combine_dsp_reg_write_bits(SC_CODEC_MUX_CTRL37_REG,
				2 << SPA_OL_SRC_DIN_SEL_OFFSET,
				3 << SPA_OL_SRC_DIN_SEL_OFFSET);
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(if_bypass_regs); i++) {
			if (if_bypass_regs[i].id == dsp_if_id)
				da_combine_dsp_reg_write_bits(if_bypass_regs[i].reg,
					bit_val << if_bypass_regs[i].offset,
					1 << if_bypass_regs[i].offset);
		}
	}
}

void mad_enable(void)
{
	/* mad_buf_clk ->en */
	da_combine_dsp_reg_set_bit(I2S_DSPIF_CLK_EN_REG, MAD_BUF_CLK_EN_OFFSET);
	/* mad_buffer ->en */
	da_combine_dsp_reg_set_bit(MAD_BUFFER_CTRL1_REG, MAD_BUF_EN_OFFSET);
	/* mad_vad_ao ->en */
	da_combine_dsp_reg_set_bit(MAD_CTRL_REG, MAD_VAD_AO_OFFSET);
	/* mad irq ->en */
	da_combine_dsp_reg_set_bit(MAD_CTRL_REG, MAD_INT_EN_OFFSET);
	/* mad ->en */
	da_combine_dsp_reg_set_bit(MAD_CTRL_REG, MAD_EN_OFFSET);
}

void mad_disable(void)
{
	/* mad_buf_clk ->en */
	da_combine_dsp_reg_clr_bit(I2S_DSPIF_CLK_EN_REG, MAD_BUF_CLK_EN_OFFSET);
	/* mad_buffer ->dis */
	da_combine_dsp_reg_clr_bit(MAD_BUFFER_CTRL1_REG, MAD_BUF_EN_OFFSET);
	/* mad_vad_ao ->dis */
	da_combine_dsp_reg_clr_bit(MAD_CTRL_REG, MAD_VAD_AO_OFFSET);
	/* mad irq ->dis */
	da_combine_dsp_reg_clr_bit(MAD_CTRL_REG, MAD_INT_EN_OFFSET);
	/* mad ->dis */
	da_combine_dsp_reg_clr_bit(MAD_CTRL_REG, MAD_EN_OFFSET);
}

static void mad_auto_clk_enable(void)
{
	da_combine_dsp_reg_clr_bit(SC_MAD_CTRL0_REG, SC_MAD_ANA_BP_OFFSET);
	da_combine_dsp_reg_clr_bit(SC_MAD_CTRL0_REG, SC_MAD_PLL_BP_OFFSET);
	da_combine_dsp_reg_clr_bit(SC_MAD_CTRL0_REG, SC_MAD_MIC_BP_OFFSET);
	da_combine_dsp_reg_clr_bit(SC_MAD_CTRL0_REG, SC_MAD_PGA_BP_OFFSET);
}

static void mad_auto_clk_disable(void)
{
	da_combine_dsp_reg_set_bit(SC_MAD_CTRL0_REG, SC_MAD_ANA_BP_OFFSET);
	da_combine_dsp_reg_set_bit(SC_MAD_CTRL0_REG, SC_MAD_PLL_BP_OFFSET);
	da_combine_dsp_reg_set_bit(SC_MAD_CTRL0_REG, SC_MAD_MIC_BP_OFFSET);
	da_combine_dsp_reg_set_bit(SC_MAD_CTRL0_REG, SC_MAD_PGA_BP_OFFSET);
}

static void set_dsp_clk_div(enum pll_state pll_state)
{
	switch (pll_state) {
	case PLL_HIGH_FREQ:
		mad_auto_clk_disable();
		da_combine_dsp_reg_clr_bit(DSP_CLK_CFG_REG, HIFI_DIV_CLK_EN_OFFSET);
		da_combine_dsp_reg_clr_bit(CLK_SOURCE_SW_REG, DSP_CLK_SW_OFFSET);
		da_combine_dsp_reg_write_bits(DSP_CLK_CFG_REG, 0x1, 0xF);
		da_combine_dsp_reg_write_bits(DSP_CLK_CFG_REG, 0x51, 0xFF);
		da_combine_dsp_reg_set_bit(DSP_CLK_CFG_REG, HIFI_DIV_CLK_EN_OFFSET);
		break;
	case PLL_LOW_FREQ:
		mad_auto_clk_enable();
		da_combine_dsp_reg_clr_bit(DSP_CLK_CFG_REG, HIFI_DIV_CLK_EN_OFFSET);
		da_combine_dsp_reg_set_bit(CLK_SOURCE_SW_REG, DSP_CLK_SW_OFFSET);
		da_combine_dsp_reg_write_bits(DSP_CLK_CFG_REG, low_pll_div_tab[g_low_pll_state], 0xF);
		da_combine_dsp_reg_set_bit(DSP_CLK_CFG_REG, HIFI_DIV_CLK_EN_OFFSET);
		break;
	default:
		break;
	}
}

static bool check_i2s2_clk(void)
{
	unsigned int s2_ctrl;

	s2_ctrl = da_combine_dsp_read_reg(SC_CODEC_MUX_CTRL26_REG);
	if (s2_ctrl & (1 << S2_CLK_IF_EN_OFFSET))
		return true;

	return false;
}

static int set_if_sample_rate(unsigned int dsp_if_id,
		unsigned int sample_rate_in, unsigned int sample_rate_out)
{
	unsigned char sample_rate_in_index = 0;
	unsigned char sample_rate_out_index = 0;

	if (dsp_if_id >= ARRAY_SIZE(sc_fs_ctrl_regs)) {
		AUDIO_LOGE("if id error, %u", dsp_if_id);
		return -1;
	}

	if (sc_fs_ctrl_regs[dsp_if_id].in_offset >= 0 &&
		da_combine_get_sample_rate_index(sample_rate_in, &sample_rate_in_index)) {
		da_combine_dsp_reg_write_bits(sc_fs_ctrl_regs[dsp_if_id].reg_addr,
			sample_rate_in_index << (unsigned int)sc_fs_ctrl_regs[dsp_if_id].in_offset,
			0x7 << (unsigned int)sc_fs_ctrl_regs[dsp_if_id].in_offset);
	} else {
		AUDIO_LOGW("invalid param dsp if id: %u, sample rate in: %u",
			dsp_if_id, sample_rate_in);
	}

	if (sc_fs_ctrl_regs[dsp_if_id].out_offset >= 0 &&
		da_combine_get_sample_rate_index(sample_rate_out, &sample_rate_out_index)) {
		da_combine_dsp_reg_write_bits(sc_fs_ctrl_regs[dsp_if_id].reg_addr,
			sample_rate_out_index << (unsigned int)sc_fs_ctrl_regs[dsp_if_id].out_offset,
			0x7 << (unsigned int)sc_fs_ctrl_regs[dsp_if_id].out_offset);
	} else {
		AUDIO_LOGW("invalid param dsp if id: %u, sample rate out: %u",
			dsp_if_id, sample_rate_out);
	}

	return 0;
}

static void config_usb_low_power(void)
{
	da_combine_dsp_reg_set_bit(DSP_PERI_CLKEN1_REG, USB_HCLK_EN_OFFSET);
	da_combine_dsp_reg_set_bit(DSP_AXI_CLKEN1_REG, USB_HRST_EN_OFFSET);
	da_combine_dsp_reg_write_bits(SOUNDWIRE_CTRL0_REG,
		0x1 << USB_PCLK_EN_OFFSET | 0x1 << USB_PRST_EN_OFFSET,
		0x1 << USB_PCLK_EN_OFFSET | 0x1 << USB_PRST_EN_OFFSET);
	da_combine_dsp_write_reg(USB20_CTRL_I_1, 0x400001);
	da_combine_dsp_reg_clr_bit(DSP_PERI_CLKEN1_REG, USB_HCLK_EN_OFFSET);
	da_combine_dsp_reg_clr_bit(DSP_AXI_CLKEN1_REG, USB_HRST_EN_OFFSET);
	da_combine_dsp_reg_write_bits(SOUNDWIRE_CTRL0_REG, 0x0,
		0x1 << USB_PCLK_EN_OFFSET);
}

static void config_dsp_param(struct da_combine_dsp_config *dsp_config,
	enum bustype_select bus_sel)
{
	dsp_config->para_addr = DA_COMBINE_V5_MLIBPARA_ADDR;
	dsp_config->msg_addr = DA_COMBINE_V5_AP_TO_DSP_MSG_ADDR;
	dsp_config->rev_msg_addr = DA_COMBINE_V5_DSP_TO_AP_MSG_ADDR;
	dsp_config->codec_type = DA_COMBINE_CODEC_TYPE_V5;
	dsp_config->cmd0_addr = DA_COMBINE_V5_CMD0_ADDR;
	dsp_config->cmd1_addr = DA_COMBINE_V5_CMD1_ADDR;
	dsp_config->cmd2_addr = DA_COMBINE_V5_CMD2_ADDR;
	dsp_config->cmd3_addr = DA_COMBINE_V5_CMD3_ADDR;
	dsp_config->wtd_irq_num = IRQ_WTD;
	dsp_config->vld_irq_num = IRQ_CMD_VALID;
	dsp_config->dump_ocram_addr = DA_COMBINE_V5_DUMP_PANIC_STACK_ADDR;
	dsp_config->dump_ocram_size = DA_COMBINE_V5_DUMP_PANIC_STACK_SIZE +
		DA_COMBINE_V5_DUMP_CPUVIEW_SIZE;
	dsp_config->dump_log_addr = DA_COMBINE_V5_SAVE_LOG_ADDR;
	dsp_config->dump_log_size = DA_COMBINE_V5_SAVE_LOG_SIZE;
	dsp_config->ocram_start_addr = DA_COMBINE_V5_OCRAM_BASE_ADDR;
	dsp_config->ocram_size = DA_COMBINE_V5_OCRAM_SIZE;
	dsp_config->itcm_start_addr = DA_COMBINE_V5_ITCM_BASE_ADDR;
	dsp_config->itcm_size = DA_COMBINE_V5_ITCM_SIZE;
	dsp_config->dtcm_start_addr = DA_COMBINE_V5_DTCM_BASE_ADDR;
	dsp_config->dtcm_size = DA_COMBINE_V5_DTCM_SIZE;
	dsp_config->msg_state_addr = DA_COMBINE_V5_DSP_MSG_STATE_ADDR;
	dsp_config->bus_sel = bus_sel;
	dsp_config->mlib_to_ap_msg_addr = DA_COMBINE_V5_MLIB_TO_AP_MSG_ADDR;
	dsp_config->mlib_to_ap_msg_size = DA_COMBINE_V5_MLIB_TO_AP_MSG_SIZE;
}

static void config_dsp_ops(struct da_combine_dsp_config *dsp_config)
{
	dsp_config->dsp_ops.init = dsp_init;
	dsp_config->dsp_ops.deinit = dsp_deinit;
	dsp_config->dsp_ops.clk_enable = NULL;
	dsp_config->dsp_ops.ram2axi = NULL;
	dsp_config->dsp_ops.runstall = set_runstall;
	dsp_config->dsp_ops.wtd_enable = set_watchdog_enable;
	dsp_config->dsp_ops.uart_enable = NULL;
	dsp_config->dsp_ops.i2c_enable = set_i2c_enable;
	dsp_config->dsp_ops.notify_dsp = notify_dsp;
	dsp_config->dsp_ops.dsp_power_ctrl = dsp_power_ctrl;
	dsp_config->dsp_ops.set_fasttrans_enable = set_fasttrans_enable;
	dsp_config->dsp_ops.set_if_bypass = set_if_bypass;
	dsp_config->dsp_ops.mad_enable = mad_enable;
	dsp_config->dsp_ops.mad_disable = mad_disable;
	dsp_config->dsp_ops.set_dsp_div = set_dsp_clk_div;
	dsp_config->dsp_ops.ir_path_clean = NULL;
	dsp_config->dsp_ops.check_i2s2_clk = check_i2s2_clk;
	dsp_config->dsp_ops.set_sample_rate = set_if_sample_rate;
	dsp_config->dsp_ops.config_usb_low_power = config_usb_low_power;
}

int da_combine_v5_dsp_config_init(struct snd_soc_component *codec,
	struct da_combine_v5_platform_data *data)
{
	int ret;
	struct da_combine_dsp_config dsp_config;
	struct da_combine_dsp_img_dl_config dl_config;

	if (codec == NULL || data == NULL || data->cdc_ctrl == NULL ||
		data->resmgr == NULL || data->irqmgr == NULL)
		return -EINVAL;

	if (data->cdc_ctrl->bus_sel <= BUSTYPE_SELECT_NULL ||
		data->cdc_ctrl->bus_sel >= BUSTYPE_SELECT_BUTT)
		return -EINVAL;

	memset(&dsp_config, 0, sizeof(dsp_config)); /* unsafe_function_ignore: memset */
	memset(&dl_config, 0, sizeof(dl_config)); /* unsafe_function_ignore: memset */

	config_dsp_param(&dsp_config, data->cdc_ctrl->bus_sel);
	config_dsp_ops(&dsp_config);

	dl_config.dspif_clk_en_addr = DA_COMBINE_V5_DSP_DSPIF_CLK_EN;

	if (of_property_read_u32(codec->dev->of_node, "pll_state", &g_low_pll_state) != 0)
		g_low_pll_state = DSP_PLL_24M;

	if (g_low_pll_state >= DSP_PLL_BUTT) {
		AUDIO_LOGW("low plL state too large: %d", g_low_pll_state);
		g_low_pll_state = DSP_PLL_24M;
	}

	dsp_config.low_pll_state = g_low_pll_state;
	AUDIO_LOGI("dsp low pll modle is %d", g_low_pll_state);

	ret = da_combine_dsp_misc_init(codec, data->resmgr, data->irqmgr, &dsp_config);
	if (ret)
		goto misc_init_err;

	if (data->board_config.dsp_soundtrigger_disable) {
		AUDIO_LOGI("soundtrigger init disable");
	} else {
		ret = soundtrigger_set_codec_type(CODEC_DA_COMBINE_V5);
		if (ret)
			goto soundtrigger_init_err;
	}

	ret = da_combine_dsp_img_dl_init(data->irqmgr, &dl_config, &dsp_config);
	if (ret)
		goto img_dl_init_err;


	return 0;

img_dl_init_err:
soundtrigger_init_err:
	da_combine_dsp_misc_deinit();
misc_init_err:
	AUDIO_LOGE("dsp config init failed");

	return ret;
}

void da_combine_v5_dsp_config_deinit(void)
{
	da_combine_dsp_misc_deinit();

	da_combine_dsp_img_dl_deinit();

	AUDIO_LOGI("dsp config deinit succ");
}

