/*
 * hi6403_dsp_config.c
 *
 * dsp init
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#include "hi6403_dsp_config.h"

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hisi/hi64xx_dsp/hi64xx_dsp_misc.h>
#include <linux/hisi/hi64xx/hi6403_regs.h>
#include <linux/hisi/hi64xx/hi6403_dsp_regs.h>
#include <linux/hisi/audio_log.h>
#include "download_image.h"
#include "dsp_utils.h"
#include "soundtrigger_dma_drv.h"

#define LOG_TAG "hi6403_dsp_config"

static unsigned int sc_fs_ctrls_h[] = {
	HI64XX_SC_FS_S1_CTRL_H,
	HI64XX_SC_FS_S2_CTRL_H,
	HI64XX_SC_FS_S3_CTRL_H,
	HI64XX_SC_FS_S4_CTRL_H,
	HI64XX_SC_FS_MISC_CTRL,
};

static void set_runstall(bool pull_down)
{
	IN_FUNCTION;

	if (pull_down)
		/* Pull down runstall of HIFI */
		hi64xx_dsp_reg_clr_bit(HI6403_SC_DSP_CTRL0,
			HI6403_SC_DSP_SFT_RUNSTALL_BIT);
	else
		/* Pull up runstall of HIFI */
		hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0,
			HI6403_SC_DSP_SFT_RUNSTALL_BIT);

	OUT_FUNCTION;
}

static void set_watchdog_enable(bool enable)
{
	IN_FUNCTION;

	if (enable)
		hi64xx_dsp_reg_set_bit(HI6403_APB_CLK_CFG,
			HI6403_WD_PCLK_EN_BIT);
	else
		hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG,
			HI6403_WD_PCLK_EN_BIT);

	OUT_FUNCTION;
}

static void notify_dsp(void)
{
	unsigned int wait_cnt = NOTIFY_DSP_WAIT_CNT;

	hi64xx_dsp_reg_set_bit(HI64XX_DSP_DSP_NMI, 0x2);

	while (wait_cnt) {
		if ((hi64xx_dsp_read_reg(HI64XX_DSP_DSP_NMI) & 0xc) == 0x0)
			break;
		usleep_range(100, 110);
		wait_cnt--;
	}

	if (wait_cnt == 0)
		AUDIO_LOGE("dsp do not handle msg, DSP_NMI:0x%x",
			hi64xx_dsp_read_reg(HI64XX_DSP_DSP_NMI));

}

static void set_uart_enable(bool enable)
{
	IN_FUNCTION;

	if (enable) {
		hi64xx_dsp_reg_set_bit(HI6403_UART_DIV_EN,
			HI6403_UART_DIV_EN_BIT);
		hi64xx_dsp_reg_set_bit(HI6403_DSP_PERI_CLKEN1,
			HI6403_UART_CLK_EN_BIT);
	} else {
		hi64xx_dsp_reg_clr_bit(HI6403_UART_DIV_EN,
			HI6403_UART_DIV_EN_BIT);
		hi64xx_dsp_reg_clr_bit(HI6403_DSP_PERI_CLKEN1,
			HI6403_UART_CLK_EN_BIT);
	}

	OUT_FUNCTION;
}

static void dsp_power_ctrl(bool enable)
{
	IN_FUNCTION;

	if (enable) {
		hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0,
			HI6403_SC_HIFI_CLK_EN_BIT);
		/* bit6:hifi_div_clk_en 0:disable 1:enable */
		hi64xx_dsp_reg_set_bit(HI6403_DSP_CLK_CFG,
			HI6403_HIFI_DIV_CLK_EN_BIT);
		hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC,
			HI6403_HIFI_PREI_CLK_EN);

		set_runstall(true);
	} else {
		hi64xx_dsp_reg_clr_bit(HI6403_DSP_CMD_STAT_VLD, 0);

		set_runstall(false);

		hi64xx_dsp_reg_clr_bit(HI6403_SC_DSP_CTRL0,
			HI6403_SC_HIFI_CLK_EN_BIT);
		/* bit6:hifi_div_clk_en 0:disable 1:enable */
		hi64xx_dsp_reg_clr_bit(HI6403_DSP_CLK_CFG,
			HI6403_HIFI_DIV_CLK_EN_BIT);
		hi64xx_dsp_reg_clr_bit(HI6403_DSP_CTRL6_DMAC,
			HI6403_HIFI_PREI_CLK_EN);
	}

	OUT_FUNCTION;
}

static void dsp_autoclk_enable(void)
{
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_DW_AXI_X2P_CG_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_DW_AXI_X2P_ASYNC_CG_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_DMAC_ACLK0_CG_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_DMAC_ACLK1_CG_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_DW_X2X_CG_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_UART_CG_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_HIFI_PREI_CLK_EN);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL6_DMAC, HI6403_PERI_DW_AXI_X2P_CG_EN);

	hi64xx_dsp_reg_set_bit(HI6403_APB_CLK_CFG, HI6403_APB_PD_PCLK_EN_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_APB_CLK_CFG, HI6403_APB2RAM_PCLK_EN_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_APB_CLK_CFG, HI6403_APB_CLK_EN_BIT);
}

static void peri_module_init(void)
{
	hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG, HI6403_WD_PCLK_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG, HI6403_TIMER0_PCLK_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG, HI6403_TIMER1_PCLK_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG, HI6403_GPIO_PCLK_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_PERI_CLKEN1, HI6403_I2C_MST_CLK_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_PERI_CLKEN1, HI6403_UART_CLK_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_UART_DIV_EN, HI6403_UART_DIV_EN_BIT);

	hi64xx_dsp_reg_set_bit(HI6403_DSP_AXI_CLKEN1, HI6403_DW_AXI_GLB_CG_EN_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_AXI_CLKEN1, HI6403_IOS_CG_EN_BIT);

	hi64xx_dsp_write_reg(HI6403_DSP_CG_CTRL0, 0x0);
	hi64xx_dsp_write_reg(HI6403_DSP_CG_CTRL1, 0x0);
}

static void dsp_rst_req_init(void)
{
	hi64xx_dsp_reg_clr_bit(HI6403_SW_RST_REQ, HI6403_DSP_PD_SRST_REQ_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SW_RST_REQ, HI6403_HIFI_PERI_SRST_REQ_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SW_RST_REQ, HI6403_HIFI_SRST_REQ_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SW_RST_REQ, HI6403_HIFI_CORE_SRST_REQ_BIT);
}

static void debug_enable(void)
{
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL2, HI6403_HIFI_DEBUGRST_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_DSP_CTRL1, HI6403_HIFI_OCDHALTONRST_BIT);
	hi64xx_memset(HI6403_AP_TO_DSP_MSG_ADDR, (size_t)HI6403_AP_TO_DSP_MSG_SIZE);
	hi64xx_memset(HI6403_AP_DSP_CMD_ADDR, (size_t)HI6403_AP_DSP_CMD_SIZE);
	hi64xx_memset(HI6403_DSP_TO_AP_MSG_ADDR, (size_t)HI6403_DSP_TO_AP_MSG_SIZE);
	hi64xx_memset(HI6403_MLIBPARA_ADDR, (size_t)HI6403_MLIB_PARA_MAX_SIZE);
}

static void dsp_init(void)
{
	IN_FUNCTION;

	/* reset dsp_pd_srst_req */
	hi64xx_dsp_reg_set_bit(HI6403_SW_RST_REQ, HI6403_HIFI_CORE_SRST_REQ_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SW_RST_REQ, HI6403_HIFI_SRST_REQ_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SW_RST_REQ, HI6403_HIFI_PERI_SRST_REQ_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SW_RST_REQ, HI6403_DSP_PD_SRST_REQ_BIT);

	/* close dspif clocks */
	hi64xx_dsp_write_reg(HI6403_DSP_I2S_DSPIF_CLK_EN, 0x0);

	/* reset dsp debug */
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_CTRL2, HI6403_HIFI_DEBUGRST_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_CTRL1, HI6403_HIFI_OCDHALTONRST_BIT);

	/* power on dsp_top_mtcmos_ctrl */
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_LP_CTRL0, HI6403_DSP_TOP_MTCMOS_CTRL_BIT);
	/* enable dsp_top_iso_ctrl */
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_LP_CTRL0, HI6403_DSP_TOP_ISO_CTRL_BIT);
	/* enable axi lowpower bydefault */
	hi64xx_dsp_reg_clr_bit(HI6403_DSP_LP_CTRL0, HI6403_AXI_CSYSREQ_BIT);

	/* enable low power ctrl by default */
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_EN_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_BP_BIT);

	/* set runstall ctrl by software */
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_RUNSTALL_BP_BIT);
	/* set hifi clk ctrl by software */
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_HIFI_DIV_BP_BIT);

	/* enable hifi clk */
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_HIFI_ACLK_EN_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_HIFI_CLK_EN_BIT);

	/* pull up runstall */
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_SFT_RUNSTALL_BIT);

	dsp_autoclk_enable();
	peri_module_init();
	dsp_rst_req_init();
	debug_enable();

	OUT_FUNCTION;
}

static void dsp_deinit(void)
{
	IN_FUNCTION;

	/* Close watchdog clock */
	hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG, HI6403_WD_PCLK_EN_BIT);

	/* Close HIFI clock */
	hi64xx_dsp_reg_clr_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_EN_BIT);

	/* Close APB clock */
	hi64xx_dsp_reg_clr_bit(HI6403_APB_CLK_CFG, HI6403_APB_PD_PCLK_EN_BIT);

	/* Close DSPIF clocks, and soft reset DSPIF */
	hi64xx_dsp_write_reg(HI6403_DSP_I2S_DSPIF_CLK_EN, 0x0);

	/* Enable isolation cell */
	hi64xx_dsp_reg_set_bit(HI64XX_DSP_LP_CTRL, 1);

	/* Soft reset HIFI */
	hi64xx_dsp_reg_set_bit(HI6403_SW_RST_REQ, HI6403_DSP_PD_SRST_REQ_BIT);

	/* Turn off power of power-off area */
	hi64xx_dsp_reg_set_bit(HI64XX_DSP_LP_CTRL, 0);

	/* Pull up runstall of HIFI */
	hi64xx_dsp_reg_set_bit(HI6403_SC_DSP_CTRL0, HI6403_SC_DSP_SFT_RUNSTALL_BIT);

	OUT_FUNCTION;
}

static void set_fasttrans_enable(bool enable)
{
	IN_FUNCTION;

	if (enable) {
		/* set S3 CTRL */
		hi64xx_dsp_reg_write_bits(HI6403_DSP_S3_CTRL_L, 0x06, 0x07); /* 192K */
		/* DSP IF 5 */
		hi64xx_dsp_write_reg(HI6403_DSP_S3_CTRL_H, 0x66); /* 192K */

		/* Slimbus u5,u6,u1,u2 */
		hi64xx_dsp_write_reg(HI6403_SLIM_UP_EN1, 0xFF); /* Open u5,u6 */
		/* Slimbus u5,u6 sample rate */
		hi64xx_dsp_write_reg(HI6403_SLIM_CTRL_5, 0x66); /* 192K */

		/* set S1 CTRL */
		hi64xx_dsp_write_reg(HI6403_DSP_S1_CTRL_L, 0x04); /* 48K */
		/* DSP IF 1 */
		hi64xx_dsp_write_reg(HI6403_DSP_S1_CTRL_H, 0x14); /* 16K */
		/* Slimbus u1,u2 sample rate */
		hi64xx_dsp_write_reg(HI6403_SLIM_CTRL_3, 0x44); /* 48K */
	} else {
		/* set S3 CTRL */
		hi64xx_dsp_reg_write_bits(HI6403_DSP_S3_CTRL_L, 0x04, 0x07); /* 48K */
		/* DSP IF 5 */
		hi64xx_dsp_write_reg(HI6403_DSP_S3_CTRL_H, 0x00); /* 8K */
		/* Slimbus u5,u6,u1,u2 */
		hi64xx_dsp_write_reg(HI6403_SLIM_UP_EN1, 0x00); /* close u5,u6,u1,u2 */
		/* Slimbus u5,u6 sample rate */
		hi64xx_dsp_write_reg(HI6403_SLIM_CTRL_5, 0x11); /* 16K */

		/* set S1 CTRL */
		hi64xx_dsp_write_reg(HI6403_DSP_S1_CTRL_L, 0x04); /* 48K */
		/* DSP IF 1 */
		hi64xx_dsp_write_reg(HI6403_DSP_S1_CTRL_H, 0x14); /* 16K */
		/* Slimbus u1,u2 sample rate */
		hi64xx_dsp_write_reg(HI6403_SLIM_CTRL_3, 0x44); /* 48K */
	}

	OUT_FUNCTION;
}

/* dsp_if bypass config bit 6,7 */
static unsigned int src_lr_ctrls[] = {
	HI6403_SC_S1_SRC_LR_CTRL_M,
	HI6403_SC_S2_SRC_LR_CTRL_M,
	HI6403_SC_S3_SRC_LR_CTRL_M,
	HI6403_SC_S4_SRC_LR_CTRL_M,
	HI6403_SC_CODEC_MUX_SEL3_1,
};

static void set_if_bypass(unsigned int dsp_if_id, bool enable)
{
	unsigned int addr;
	unsigned int bit = 0;

	unsigned int i2s_id = dsp_if_id / 2;
	unsigned int direct =
		(dsp_if_id & 0x1) ? HI64XX_DSP_PCM_OUT : HI64XX_DSP_PCM_IN;

	IN_FUNCTION;

	if (i2s_id >= ARRAY_SIZE(src_lr_ctrls)) {
		AUDIO_LOGE("i2s id error, %d\n", i2s_id);
		return;
	}
	addr = src_lr_ctrls[i2s_id];

	if (dsp_if_id < HI64XX_DSP_IF_PORT_8) {
		bit = (direct == HI64XX_DSP_PCM_IN) ? 6 : 7;
		if (enable)
			hi64xx_dsp_reg_set_bit(addr, bit);
		else
			hi64xx_dsp_reg_clr_bit(addr, bit);
	} else if (dsp_if_id == HI64XX_DSP_IF_PORT_8) {
		if (enable) {
			hi64xx_dsp_reg_set_bit(addr, 7);
			hi64xx_dsp_reg_set_bit(addr, 3);
		} else {
			hi64xx_dsp_reg_clr_bit(addr, 7);
			hi64xx_dsp_reg_clr_bit(addr, 3);
		}
	}

	OUT_FUNCTION;
}

static void mad_enable(void)
{
	/* mad_buf_clk ->en */
	hi64xx_dsp_reg_set_bit(HI6403_I2S_DSPIF_CLK_EN, HI6403_MAD_BUF_CLK_EN_BIT);
	/* mad_buffer ->en */
	hi64xx_dsp_reg_set_bit(HI6403_MAD_BUFFER_CTRL1, HI6403_MAD_BUF_EN_BIT);
	/* mad_vad_ao ->en */
	hi64xx_dsp_reg_set_bit(HI6403_MAD_CTRL, HI6403_MAD_VAD_AO_BIT);
	/* mad irq ->en */
	hi64xx_dsp_reg_set_bit(HI6403_MAD_CTRL, HI6403_MAD_INT_EN_BIT);
	/* mad ->en */
	hi64xx_dsp_reg_set_bit(HI6403_MAD_CTRL, HI6403_MAD_EN_BIT);
}

static void mad_disable(void)
{
	/* mad_buf_clk ->en */
	hi64xx_dsp_reg_clr_bit(HI6403_I2S_DSPIF_CLK_EN, HI6403_MAD_BUF_CLK_EN_BIT);
	/* mad_buffer ->dis */
	hi64xx_dsp_reg_clr_bit(HI6403_MAD_BUFFER_CTRL1, HI6403_MAD_BUF_EN_BIT);
	/* mad_vad_ao ->dis */
	hi64xx_dsp_reg_clr_bit(HI6403_MAD_CTRL, HI6403_MAD_VAD_AO_BIT);
	/* mad irq ->dis */
	hi64xx_dsp_reg_clr_bit(HI6403_MAD_CTRL, HI6403_MAD_INT_EN_BIT);
	/* mad ->dis */
	hi64xx_dsp_reg_clr_bit(HI6403_MAD_CTRL, HI6403_MAD_EN_BIT);
}

static void mad_auto_clk_enable(void)
{
	hi64xx_dsp_reg_clr_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_ANA_BP_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_PLL_BP_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_MIC_BP_BIT);
	hi64xx_dsp_reg_clr_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_PGA_BP_BIT);
}

static void mad_auto_clk_disable(void)
{
	hi64xx_dsp_reg_set_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_ANA_BP_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_PLL_BP_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_MIC_BP_BIT);
	hi64xx_dsp_reg_set_bit(HI6403_SC_MAD_CTRL0, HI6403_SC_MAD_PGA_BP_BIT);
}

static void set_dsp_clk_div(enum pll_state pll_state)
{
	switch (pll_state) {
	case PLL_HIGH_FREQ:
		mad_auto_clk_disable();
		/* disable hifi_div_clk_en */
		hi64xx_dsp_reg_clr_bit(HI6403_DSP_CLK_CFG,
			HI6403_HIFI_DIV_CLK_EN_BIT);
		hi64xx_dsp_reg_clr_bit(HI6403_12M288_CLK_SEL_REG,
			HI6403_DSP_CLK_SW_BIT);
		hi64xx_dsp_reg_write_bits(HI6403_DSP_CLK_CFG, 0x1, 0xF);
		/* enable hifi_div_clk_en */
		hi64xx_dsp_reg_set_bit(HI6403_DSP_CLK_CFG,
			HI6403_HIFI_DIV_CLK_EN_BIT);
		break;
	case PLL_LOW_FREQ:
		mad_auto_clk_enable();
		/* disable hifi_div_clk_en */
		hi64xx_dsp_reg_clr_bit(HI6403_DSP_CLK_CFG,
			HI6403_HIFI_DIV_CLK_EN_BIT);
		hi64xx_dsp_reg_set_bit(HI6403_12M288_CLK_SEL_REG,
			HI6403_DSP_CLK_SW_BIT);
		hi64xx_dsp_reg_write_bits(HI6403_DSP_CLK_CFG, 0x3, 0xF);
		/* enable hifi_div_clk_en */
		hi64xx_dsp_reg_set_bit(HI6403_DSP_CLK_CFG,
			HI6403_HIFI_DIV_CLK_EN_BIT);
		break;
	default:
		break;
	}
}

static void clean_ir_study_path(void)
{
	hi64xx_dsp_reg_write_bits(HI6403_SC_FS_ADC_REG, 0x0, 0x30);
	hi64xx_dsp_reg_write_bits(HI6403_SC_CODEC_MUX_SEL1_3_REG, 0x0, 0x80);
	hi64xx_dsp_reg_write_bits(HI64XX_SC_FS_S4_CTRL_H, 0x40, 0x70);
	hi64xx_dsp_reg_write_bits(HI6403_DSPIF8_MUX_REG, 0x0, 0xF);
	hi64xx_dsp_reg_write_bits(HI64XX_SC_FS_MISC_CTRL, 0x0, 0xE0);

	/* HPF bypass for mic abnormal */
	hi64xx_dsp_reg_write_bits(HI6403_ADC1L_R_REG, 0x0, 0x1);
}

static bool check_i2s2_clk(void)
{
	unsigned int s2_ctrl;

	s2_ctrl = hi64xx_dsp_read_reg(HI6403_DSP_S2_CTRL_L);
	if (s2_ctrl & (1 << HI6403_DSP_S2_CLK_EN_BIT))
		return true;

	return false;
}

static int set_if_sample_rate(unsigned int dsp_if_id,
	unsigned int sample_rate_in, unsigned int sample_rate_out)
{
	unsigned int addr;
	unsigned char mask = 0;
	unsigned char sample_rate_index = 0;

	unsigned int i2s_id = dsp_if_id / 2;
	unsigned int direct =
		(dsp_if_id & 0x1) ? HI64XX_DSP_PCM_OUT : HI64XX_DSP_PCM_IN;

	IN_FUNCTION;

	if (i2s_id >= ARRAY_SIZE(sc_fs_ctrls_h)) {
		AUDIO_LOGE("i2s id error, %d", i2s_id);
		return -1;
	}

	addr = sc_fs_ctrls_h[i2s_id];

	if (!hi64xx_get_sample_rate_index(sample_rate_in, &sample_rate_index)) {
		AUDIO_LOGE("sample_rate_in is invalid %d", sample_rate_in);
		return 0;
	}

	if (dsp_if_id != HI64XX_DSP_IF_PORT_8) {
		mask = (direct == HI64XX_DSP_PCM_IN) ? 0xf : 0xf0;

		if (direct != HI64XX_DSP_PCM_IN)
			sample_rate_index <<= 4;
	} else {
		mask = (direct == HI64XX_DSP_PCM_IN) ? 0xe0 : 0x1c;
		if (sample_rate_index < HI64XX_SAMPLE_RATE_INDEX_48K) {
			AUDIO_LOGE("unsupport sample rate in %d", sample_rate_in);
			return 0;
		}
		sample_rate_index = sample_rate_index - HI64XX_SAMPLE_RATE_INDEX_48K;

		if (direct == HI64XX_DSP_PCM_IN)
			sample_rate_index <<= 5;
		else
			sample_rate_index <<= 2;
	}

	hi64xx_dsp_reg_write_bits(addr, sample_rate_index, mask);

	OUT_FUNCTION;

	return 0;
}

static void config_dsp_param(struct hi64xx_dsp_config *dsp_config,
	enum bustype_select bus_sel)
{
	dsp_config->para_addr = HI6403_MLIBPARA_ADDR;
	dsp_config->msg_addr = HI6403_AP_TO_DSP_MSG_ADDR;
	dsp_config->rev_msg_addr = HI6403_DSP_TO_AP_MSG_ADDR;
	dsp_config->codec_type = HI64XX_CODEC_TYPE_6403;
	dsp_config->cmd0_addr = HI6403_CMD0_ADDR;
	dsp_config->cmd1_addr = HI6403_CMD1_ADDR;
	dsp_config->cmd2_addr = HI6403_CMD2_ADDR;
	dsp_config->cmd3_addr = HI6403_CMD3_ADDR;
	dsp_config->cmd4_addr = HI6403_CMD4_ADDR;
	dsp_config->wtd_irq_num = IRQ_WTD;
	dsp_config->vld_irq_num = IRQ_CMD_VALID;
	dsp_config->dump_ocram_addr = HI6403_DUMP_PANIC_STACK_ADDR;
	dsp_config->dump_ocram_size = HI6403_DUMP_PANIC_STACK_SIZE + HI6403_DUMP_CPUVIEW_SIZE;
	dsp_config->dump_log_addr = HI6403_SAVE_LOG_ADDR;
	dsp_config->dump_log_size = HI6403_SAVE_LOG_SIZE;
	dsp_config->ocram_start_addr = HI6403_OCRAM_BASE_ADDR;
	dsp_config->ocram_size = HI6403_OCRAM_SIZE;
	dsp_config->itcm_start_addr = HI6403_ITCM_BASE_ADDR;
	dsp_config->itcm_size = HI6403_ITCM_SIZE;
	dsp_config->dtcm_start_addr = HI6403_DTCM_BASE_ADDR;
	dsp_config->dtcm_size = HI6403_DTCM_SIZE;
	dsp_config->msg_state_addr = HI6403_DSP_MSG_STATE_ADDR;
	dsp_config->bus_sel = bus_sel;
	dsp_config->mlib_to_ap_msg_addr = HI6403_MLIB_TO_AP_MSG_ADDR;
	dsp_config->mlib_to_ap_msg_size = HI6403_MLIB_TO_AP_MSG_SIZE;
}

static void config_dsp_ops(struct hi64xx_dsp_config *dsp_config)
{
	dsp_config->dsp_ops.init = dsp_init;
	dsp_config->dsp_ops.deinit = dsp_deinit;
	dsp_config->dsp_ops.clk_enable = NULL;
	dsp_config->dsp_ops.ram2axi = NULL;
	dsp_config->dsp_ops.runstall = set_runstall;
	dsp_config->dsp_ops.wtd_enable = set_watchdog_enable;
	dsp_config->dsp_ops.uart_enable = set_uart_enable;
	dsp_config->dsp_ops.i2c_enable = NULL;
	dsp_config->dsp_ops.notify_dsp = notify_dsp;
	dsp_config->dsp_ops.dsp_power_ctrl = dsp_power_ctrl;
	dsp_config->dsp_ops.set_fasttrans_enable = set_fasttrans_enable;
	dsp_config->dsp_ops.set_if_bypass = set_if_bypass;
	dsp_config->dsp_ops.mad_enable = mad_enable;
	dsp_config->dsp_ops.mad_disable = mad_disable;
	dsp_config->dsp_ops.set_dsp_div = set_dsp_clk_div;
	dsp_config->dsp_ops.ir_path_clean = clean_ir_study_path;
	dsp_config->dsp_ops.check_i2s2_clk = check_i2s2_clk;
	dsp_config->dsp_ops.set_sample_rate = set_if_sample_rate;
	dsp_config->dsp_ops.config_usb_low_power = NULL;
}

int hi6403_dsp_config_init(struct snd_soc_codec *codec,
	struct hi64xx_resmgr *resmgr, struct hi64xx_irq *irqmgr,
	enum bustype_select bus_sel)
{
	int ret;
	struct hi64xx_dsp_config dsp_config;
	struct hi64xx_dsp_img_dl_config dl_config;

	if (!codec || !resmgr || !irqmgr)
		return -EINVAL;

	if (bus_sel <= BUSTYPE_SELECT_NULL || bus_sel >= BUSTYPE_SELECT_BUTT)
		return -EINVAL;

	AUDIO_LOGI("hi6403 dsp config init begin");

	memset(&dsp_config, 0, sizeof(dsp_config));
	memset(&dl_config, 0, sizeof(dl_config));

	config_dsp_param(&dsp_config, bus_sel);
	config_dsp_ops(&dsp_config);

	dl_config.dspif_clk_en_addr = HI6403_DSP_I2S_DSPIF_CLK_EN;

	ret = hi64xx_dsp_misc_init(codec, resmgr, irqmgr, &dsp_config);
	if (ret)
		goto misc_init_err;

	ret = hi64xx_soundtrigger_init(CODEC_HI6403);
	if (ret)
		goto soundtrigger_init_err;

	ret = hi64xx_dsp_img_dl_init(irqmgr, &dl_config, &dsp_config);
	if (ret)
		goto img_dl_init_err;


	AUDIO_LOGI("hi6403 dsp config init end");

	return 0;

img_dl_init_err:
soundtrigger_init_err:
	hi64xx_dsp_misc_deinit();
misc_init_err:
	AUDIO_LOGE("hi6403 dsp config init failed");

	return ret;
}

void hi6403_hifi_config_deinit(void)
{
	hi64xx_dsp_misc_deinit();

	hi64xx_dsp_img_dl_deinit();
}

