/*
 * hi6405_hifi_config.c -- adapt 64xx hifi misc to 6405
 *
 * Copyright (c) 2015 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hisi/hi64xx_hifi_misc.h>
#include <linux/hisi/hi64xx/hi6405_regs.h>
#include <linux/hisi/hi64xx/hi6405_dsp_regs.h>
#include "hi64xx_hifi_debug.h"
#include "hi64xx_hifi_img_dl.h"
#include "hi64xx_hifi_om.h"
#include "hi6405_hifi_config.h"
#include "soundtrigger_dma_drv.h"

struct dspif_sc_fs {
	unsigned int reg_addr;
	int in_offset;
	int out_offset;
};

static struct dspif_sc_fs hi6405_sc_fs_ctrl_regs[HI64XX_HIFI_DSP_IF_PORT_BUTT] = {
	{HI6405_SC_CODEC_MUX_CTRL4_REG, -1, HI6405_DSPIF0_DOUT_VLD_SEL_OFFSET},
	{HI6405_DSPIF_VLD_CTRL0_REG, HI6405_DSPIF1_DIN_VLD_SEL_OFFSET, HI6405_DSPIF1_DOUT_VLD_SEL_OFFSET},
	{HI6405_SC_CODEC_MUX_CTRL5_REG, -1, HI6405_DSPIF2_DOUT_VLD_SEL_OFFSET},
	{HI6405_DSPIF_VLD_CTRL1_REG, HI6405_DSPIF3_DIN_VLD_SEL_OFFSET, HI6405_DSPIF3_DOUT_VLD_SEL_OFFSET},
	{HI6405_SC_CODEC_MUX_CTRL6_REG, -1, HI6405_DSPIF4_DOUT_VLD_SEL_OFFSET},
	{HI6405_DSPIF_VLD_CTRL2_REG, HI6405_DSPIF5_DIN_VLD_SEL_OFFSET, HI6405_DSPIF5_DOUT_VLD_SEL_OFFSET},
	{HI6405_SC_CODEC_MUX_CTRL7_REG, -1, HI6405_DSPIF6_DOUT_VLD_SEL_OFFSET},
	{HI6405_DSPIF_VLD_CTRL3_REG, HI6405_DSPIF7_DIN_VLD_SEL_OFFSET, HI6405_DSPIF7_DOUT_VLD_SEL_OFFSET},
	{HI6405_DSPIF_VLD_CTRL4_REG, HI6405_DSPIF8_DIN_VLD_SEL_OFFSET, HI6405_DSPIF8_DOUT_VLD_SEL_OFFSET},
	{HI6405_DSPIF_VLD_CTRL5_REG, HI6405_DSPIF9_DIN_VLD_SEL_OFFSET, HI6405_DSPIF9_DOUT_VLD_SEL_OFFSET},
};

static void hi6405_hifi_runstall_cfg(bool pull_down)
{
	if (pull_down) {
		hi64xx_hifi_reg_clr_bit(HI6405_DSP_CTRL0_REG, HI6405_HIFI_STATVECTORSEL_OFFSET);
		/* Pull down runstall of HIFI*/
		hi64xx_hifi_reg_clr_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_SFT_RUNSTALL_OFFSET);
	} else {
		/* Pull up runstall of HIFI*/
		hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_SFT_RUNSTALL_OFFSET);
	}
}

static void hi6405_hifi_watchdog_enable(bool enable)
{
	if (enable) {
		hi64xx_hifi_reg_set_bit(HI6405_APB_CLK_CFG_REG, HI6405_WD_PCLK_EN_OFFSET);
	} else {
		hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_WD_PCLK_EN_OFFSET);
	}
}

static void hi6405_hifi_notify_dsp(void)
{
	unsigned int wait_cnt = 5;

	hi64xx_hifi_reg_set_bit(HI6405_DSP_NMI_REG, HI6405_HIFI_IRQ_REQ_OFFSET);

	while (wait_cnt) {
		if (0x0 == (hi64xx_hifi_read_reg(HI6405_DSP_NMI_REG) & 0xc))
			break;
		usleep_range((unsigned long)100, (unsigned long)110);
		wait_cnt--;
	}

	if (0 == wait_cnt)
		HI64XX_DSP_ERROR("dsp do not handle msg, DSP_NMI:0x%x\n",
			hi64xx_hifi_read_reg(HI6405_DSP_NMI_REG));

}

static void hi6405_dsp_power_ctrl(bool enable)
{
	if (enable) {
		hi64xx_hifi_reg_set_bit(HI6405_DSP_CLK_CFG_REG, HI6405_HIFI_DIV_CLK_EN_OFFSET);
		hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_HIFI_PERI_CLK_EN_OFFSET);
		hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_HIFI_CLK_EN_OFFSET);
		hi6405_hifi_runstall_cfg(true);
	} else {
		hi64xx_hifi_reg_clr_bit(HI6405_DSP_CMD_STAT_VLD, HI6405_DSP_CMD_STAT_VLD_OFFSET);
		hi6405_hifi_runstall_cfg(false);
		hi64xx_hifi_reg_clr_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_HIFI_CLK_EN_OFFSET);
		hi64xx_hifi_reg_clr_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_HIFI_PERI_CLK_EN_OFFSET);
		hi64xx_hifi_reg_clr_bit(HI6405_DSP_CLK_CFG_REG, HI6405_HIFI_DIV_CLK_EN_OFFSET);
	}
}

static void hi6405_hifi_init(void)
{
	/* reset dsp_pd_srst_req */
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_CORE_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_PERI_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_DSP_PD_SRST_REQ_OFFSET);

	/* close all dspif clocks */
	hi64xx_hifi_write_reg(HI6405_DSP_DSPIF_CLK_EN, 0x0);

	/* enable dsp debug */
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_CTRL2_REG, HI6405_HIFI_DEBUGRST_N_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_CTRL1_REG, HI6405_HIFI_OCDHALTONRESET_N_OFFSET);

	/* power on dsp_top_mtcmos_ctrl*/
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_LP_CTRL0_REG, HI6405_DSP_TOP_MTCMOS_CTRL_OFFSET);
	/* enable dsp_top_iso_ctrl */
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_LP_CTRL0_REG, HI6405_DSP_TOP_ISO_CTRL_OFFSET);
	/* enable axi lowpower bydefault */
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_LP_CTRL0_REG, HI6405_AXI_CSYSREQ_OFFSET);

	/* enable low power ctrl by default*/
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_BP_OFFSET);

	/* set runstall ctrl by software */
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_RUNSTALL_BP_OFFSET);
	/* set hifi clk ctrl by software */
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_HIFI_DIV_BP_OFFSET);

	/* enable hifi clk */
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_HIFI_ACLK_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_HIFI_CLK_EN_OFFSET);

	/* pull up runstall */
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_SFT_RUNSTALL_OFFSET);

	/* enable dsp autoclk  */
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_DW_AXI_X2P_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_DW_P2P_ASYNC_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_DMAC_ACLK0_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_DMAC_ACLK1_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_DW_X2X_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_UART_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_HIFI_PERI_CLK_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL6_DMAC_REG, HI6405_PERI_DW_AXI_X2P_CG_EN_OFFSET);

	hi64xx_hifi_reg_set_bit(HI6405_APB_CLK_CFG_REG, HI6405_APB_PD_PCLK_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_APB_CLK_CFG_REG, HI6405_APB2RAM_PCLK_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_APB_CLK_CFG_REG, HI6405_APB_CLK_EN_OFFSET);

	/* disable watchdog */
	hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_WD_PCLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_TIMER0_PCLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_TIMER1_PCLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_GPIO_PCLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_PERI_CLKEN1_REG, HI6405_I2C_MST_CLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_PERI_CLKEN1_REG, HI6405_UART_CLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_I2C_TO_CTRL1_REG, HI6405_UART_DIV_EN_OFFSET);

	hi64xx_hifi_reg_set_bit(HI6405_DSP_AXI_CLKEN1_REG, HI6405_DW_AXI_GLB_CG_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_AXI_CLKEN1_REG, HI6405_IOS_CG_EN_OFFSET);

	/* enable hifi auto clk */
	hi64xx_hifi_write_reg(HI6405_DSP_CG_CTRL0_REG, 0x0);
	hi64xx_hifi_write_reg(HI6405_DSP_CG_CTRL1_REG, 0x0);

	/* release dsp_pd_srst_req */
	hi64xx_hifi_reg_clr_bit(HI6405_SW_RST_REQ_REG, HI6405_DSP_PD_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_PERI_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_CORE_SRST_REQ_OFFSET);

	/* enable dsp debug */
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL2_REG, HI6405_HIFI_DEBUGRST_N_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_CTRL1_REG, HI6405_HIFI_OCDHALTONRESET_N_OFFSET);

	hi64xx_memset(HI6405_AP_TO_DSP_MSG_ADDR, (size_t)HI6405_AP_TO_DSP_MSG_SIZE);
	hi64xx_memset(HI6405_AP_DSP_CMD_ADDR, (size_t)HI6405_AP_DSP_CMD_SIZE);
	hi64xx_memset(HI6405_DSP_TO_AP_MSG_ADDR, (size_t)HI6405_DSP_TO_AP_MSG_SIZE);
	hi64xx_memset(HI6405_MLIBPARA_ADDR, (size_t)HI6405_MLIB_PARA_MAX_SIZE);
}

static void hi6405_hifi_deinit(void)
{
	/* Close watchdog clock */
	hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_WD_PCLK_EN_OFFSET);

	/* Close HIFI clock */
	hi64xx_hifi_reg_clr_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_EN_OFFSET);

	/* Close APB clock */
	hi64xx_hifi_reg_clr_bit(HI6405_APB_CLK_CFG_REG, HI6405_APB_PD_PCLK_EN_OFFSET);

	/* Close DSPIF clocks, and soft reset DSPIF */
	hi64xx_hifi_write_reg(HI6405_DSP_DSPIF_CLK_EN, 0x0);

	/* Enable isolation cell */
	hi64xx_hifi_reg_set_bit(HI6405_DSP_LP_CTRL0_REG, HI6405_DSP_TOP_ISO_CTRL_OFFSET);

	/* Soft reset HIFI */
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_CORE_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_HIFI_PERI_SRST_REQ_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SW_RST_REQ_REG, HI6405_DSP_PD_SRST_REQ_OFFSET);

	/* Turn off power of power-off area */
	hi64xx_hifi_reg_set_bit(HI6405_DSP_LP_CTRL0_REG, HI6405_DSP_TOP_MTCMOS_CTRL_OFFSET);

	/* Pull up runstall of HIFI */
	hi64xx_hifi_reg_set_bit(HI6405_SC_DSP_CTRL0_REG, HI6405_SC_DSP_SFT_RUNSTALL_OFFSET);
}

static void hi6405_soundtrigger_fasttrans_ctrl(bool enable, bool fm)
{
	IN_FUNCTION;

	if (enable) {
		/* DSP IF 5 */
		hi64xx_hifi_write_reg(HI6405_DSPIF_VLD_CTRL2_REG, 0x36);/* 192K */
		/* DSP IF 1 */
		hi64xx_hifi_write_reg(HI6405_DSPIF_VLD_CTRL0_REG, 0xC); /* 16k in 48k out */


		hi64xx_hifi_reg_write_bits(HI6405_SLIM_UP_EN1_REG,
			0x1<<HI6405_SLIM_UP1_EN_OFFSET | 0x1<<HI6405_SLIM_UP2_EN_OFFSET |
			0x1<<HI6405_SLIM_UP5_EN_OFFSET | 0x1<<HI6405_SLIM_UP6_EN_OFFSET,
			0x1<<HI6405_SLIM_UP1_EN_OFFSET | 0x1<<HI6405_SLIM_UP2_EN_OFFSET |
			0x1<<HI6405_SLIM_UP5_EN_OFFSET | 0x1<<HI6405_SLIM_UP6_EN_OFFSET);

		hi64xx_hifi_write_reg(HI6405_SC_FS_SLIM_CTRL_5_REG,
			0x6<<HI6405_FS_SLIM_UP6_OFFSET | 0x6<<HI6405_FS_SLIM_UP5_OFFSET); /* 192K */

		hi64xx_hifi_write_reg(HI6405_SC_FS_SLIM_CTRL_3_REG, 0x44); /* 48K */
	} else {
		/* DSP IF 5 */
		hi64xx_hifi_write_reg(HI6405_DSPIF_VLD_CTRL2_REG, 0x24);;/* 48K */

		hi64xx_hifi_reg_write_bits(HI6405_SLIM_UP_EN1_REG,
			0x0,
			0x1<<HI6405_SLIM_UP1_EN_OFFSET | 0x1<<HI6405_SLIM_UP2_EN_OFFSET |
			0x1<<HI6405_SLIM_UP5_EN_OFFSET | 0x1<<HI6405_SLIM_UP6_EN_OFFSET);

		hi64xx_hifi_write_reg(HI6405_SC_FS_SLIM_CTRL_5_REG,
			0x2<<HI6405_FS_SLIM_UP6_OFFSET | 0x2<<HI6405_FS_SLIM_UP5_OFFSET); /* 32K */
	}

	OUT_FUNCTION;
}

static void hi6405_dsp_if_set_bypass(unsigned int dsp_if_id, bool bypass)
{
	unsigned int bit_val = 0;

	bit_val = bypass ? 1 : 0;

	switch (dsp_if_id) {
	case HI64XX_HIFI_DSP_IF_PORT_0:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL8_REG,
				bit_val << HI6405_S1_I_DSP_BYPASS_OFFSET,
				1 << HI6405_S1_I_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_1:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL22_REG,
				bit_val << HI6405_S1_O_DSP_BYPASS_OFFSET,
				1 << HI6405_S1_O_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_2:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL8_REG,
				bit_val << HI6405_S2_I_DSP_BYPASS_OFFSET,
				1 << HI6405_S2_I_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_3:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL22_REG,
				bit_val << HI6405_S2_O_DSP_BYPASS_OFFSET,
				1 << HI6405_S2_O_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_4:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL8_REG,
				bit_val << HI6405_S3_I_DSP_BYPASS_OFFSET,
				1 << HI6405_S3_I_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_5:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL22_REG,
				bit_val << HI6405_S3_O_DSP_BYPASS_OFFSET,
				1 << HI6405_S3_O_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_6:
		hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL8_REG,
				bit_val << HI6405_S4_I_DSP_BYPASS_OFFSET,
				1 << HI6405_S4_I_DSP_BYPASS_OFFSET);
		break;
	case HI64XX_HIFI_DSP_IF_PORT_7:
		if (bypass) {
			hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL37_REG,
					1 << HI6405_SPA_OR_SRC_DIN_SEL_OFFSET,
					3 << HI6405_SPA_OR_SRC_DIN_SEL_OFFSET);
			hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL37_REG,
					1 << HI6405_SPA_OL_SRC_DIN_SEL_OFFSET,
					3 << HI6405_SPA_OL_SRC_DIN_SEL_OFFSET);
		} else {
			hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL37_REG,
					2 << HI6405_SPA_OR_SRC_DIN_SEL_OFFSET,
					3 << HI6405_SPA_OR_SRC_DIN_SEL_OFFSET);
			hi64xx_hifi_reg_write_bits(HI6405_SC_CODEC_MUX_CTRL37_REG,
					2 << HI6405_SPA_OL_SRC_DIN_SEL_OFFSET,
					3 << HI6405_SPA_OL_SRC_DIN_SEL_OFFSET);
		}
		break;
	default:
		HI64XX_DSP_INFO("dsp if %d do not need config bypass\n", dsp_if_id);
		break;
	}
}

void hi6405_mad_enable(void)
{
	/* mad_buf_clk ->en */
	hi64xx_hifi_reg_set_bit(HI6405_I2S_DSPIF_CLK_EN_REG, HI6405_MAD_BUF_CLK_EN_OFFSET);
	/* mad_buffer ->en */
	hi64xx_hifi_reg_set_bit(HI6405_MAD_BUFFER_CTRL1_REG, HI6405_MAD_BUF_EN_OFFSET);
	/* mad_vad_ao ->en */
	hi64xx_hifi_reg_set_bit(HI6405_MAD_CTRL_REG, HI6405_MAD_VAD_AO_OFFSET);
	/* mad irq ->en */
	hi64xx_hifi_reg_set_bit(HI6405_MAD_CTRL_REG, HI6405_MAD_INT_EN_OFFSET);
	/* mad ->en */
	hi64xx_hifi_reg_set_bit(HI6405_MAD_CTRL_REG, HI6405_MAD_EN_OFFSET);
}

void hi6405_mad_disable(void)
{
	/* mad_buf_clk ->en */
	hi64xx_hifi_reg_clr_bit(HI6405_I2S_DSPIF_CLK_EN_REG, HI6405_MAD_BUF_CLK_EN_OFFSET);
	/* mad_buffer ->dis */
	hi64xx_hifi_reg_clr_bit(HI6405_MAD_BUFFER_CTRL1_REG, HI6405_MAD_BUF_EN_OFFSET);
	/* mad_vad_ao ->dis */
	hi64xx_hifi_reg_clr_bit(HI6405_MAD_CTRL_REG, HI6405_MAD_VAD_AO_OFFSET);
	/* mad irq ->dis */
	hi64xx_hifi_reg_clr_bit(HI6405_MAD_CTRL_REG, HI6405_MAD_INT_EN_OFFSET);
	/* mad ->dis */
	hi64xx_hifi_reg_clr_bit(HI6405_MAD_CTRL_REG, HI6405_MAD_EN_OFFSET);
}

static void hi6405_enable_mad_auto_clk(void)
{
	hi64xx_hifi_reg_clr_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_ANA_BP_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_PLL_BP_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_MIC_BP_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_PGA_BP_OFFSET);
}

static void hi6405_disable_mad_auto_clk(void)
{
	hi64xx_hifi_reg_set_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_ANA_BP_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_PLL_BP_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_MIC_BP_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_SC_MAD_CTRL0_REG, HI6405_SC_MAD_PGA_BP_OFFSET);
}

static void hi6405_set_dsp_div(enum pll_state pll_state)
{
	switch(pll_state) {
	case PLL_HIGH_FREQ:
		hi6405_disable_mad_auto_clk();
		hi64xx_hifi_reg_clr_bit(HI6405_DSP_CLK_CFG_REG, HI6405_HIFI_DIV_CLK_EN_OFFSET);
		hi64xx_hifi_reg_clr_bit(HI6405_CLK_SOURCE_SW_REG, HI6405_DSP_CLK_SW_OFFSET);
		hi64xx_hifi_reg_write_bits(HI6405_DSP_CLK_CFG_REG, 0x1, 0xF);
		hi64xx_hifi_reg_write_bits(HI6405_DSP_CLK_CFG_REG, 0x51, 0xFF);
		hi64xx_hifi_reg_set_bit(HI6405_DSP_CLK_CFG_REG, HI6405_HIFI_DIV_CLK_EN_OFFSET);
		break;
	case PLL_LOW_FREQ:
		hi6405_enable_mad_auto_clk();
		hi64xx_hifi_reg_clr_bit(HI6405_DSP_CLK_CFG_REG, HI6405_HIFI_DIV_CLK_EN_OFFSET);
		hi64xx_hifi_reg_set_bit(HI6405_CLK_SOURCE_SW_REG, HI6405_DSP_CLK_SW_OFFSET);
		hi64xx_hifi_reg_write_bits(HI6405_DSP_CLK_CFG_REG, 0x3, 0xF);
		hi64xx_hifi_reg_set_bit(HI6405_DSP_CLK_CFG_REG, HI6405_HIFI_DIV_CLK_EN_OFFSET);
		break;
	default:
		break;
	}
	return;
}

static void hi6405_clean_ir_study_path(void)
{
	return;
}

static bool hi6405_check_dp_clk(void)
{
	unsigned int count = 1000;
	while(--count) {
		if(1 == hi64xx_hifi_read_reg(HI6405_CODEC_DP_CLKEN_REG)) {
			return true;
		} else {
			usleep_range(100, 110);
		}
	}

	return false;
}

static bool hi6405_check_i2s2_clk(void)
{
	unsigned int s2_ctrl = 0;

	s2_ctrl = hi64xx_hifi_read_reg(HI6405_SC_CODEC_MUX_CTRL26_REG);
	if (s2_ctrl & (1 << HI6405_S2_CLK_IF_EN_OFFSET))
		return true;

	return false;
}

static int hi6405_dsp_if_set_sample_rate(unsigned int dsp_if_id,
						unsigned int sample_rate_in, unsigned int sample_rate_out)
{
	unsigned char sample_rate_in_index = 0;
	unsigned char sample_rate_out_index = 0;

	if (hi6405_sc_fs_ctrl_regs[dsp_if_id].in_offset >= 0 && hi64xx_get_sample_rate_index(sample_rate_in, &sample_rate_in_index)) {
		hi64xx_hifi_reg_write_bits(hi6405_sc_fs_ctrl_regs[dsp_if_id].reg_addr,
			sample_rate_in_index<<hi6405_sc_fs_ctrl_regs[dsp_if_id].in_offset, 0x7<<hi6405_sc_fs_ctrl_regs[dsp_if_id].in_offset);
	} else {
		HI64XX_DSP_WARNING("invalid param dsp_if_id = %u, sample_rate_in = %u \n", dsp_if_id, sample_rate_in);
	}

	if (hi6405_sc_fs_ctrl_regs[dsp_if_id].out_offset >= 0 && hi64xx_get_sample_rate_index(sample_rate_out, &sample_rate_out_index)) {
		hi64xx_hifi_reg_write_bits(hi6405_sc_fs_ctrl_regs[dsp_if_id].reg_addr,
			sample_rate_out_index<<hi6405_sc_fs_ctrl_regs[dsp_if_id].out_offset, 0x7<<hi6405_sc_fs_ctrl_regs[dsp_if_id].out_offset);
	} else {
		HI64XX_DSP_WARNING("invalid param dsp_if_id = %u, sample_rate_out = %u \n", dsp_if_id, sample_rate_out);
	}

	return 0;
}

static void hi6405_dsp_config_usb_low_power(void)
{
	hi64xx_hifi_reg_set_bit(HI6405_DSP_PERI_CLKEN1_REG, HI6405_USB_HCLK_EN_OFFSET);
	hi64xx_hifi_reg_set_bit(HI6405_DSP_AXI_CLKEN1_REG, HI6405_USB_HRST_EN_OFFSET);
	hi64xx_hifi_reg_write_bits(HI6405_SOUNDWIRE_CTRL0_REG,
		0x1<<HI6405_USB_PCLK_EN_OFFSET | 0x1<<HI6405_USB_PRST_EN_OFFSET,
		0x1<<HI6405_USB_PCLK_EN_OFFSET | 0x1<<HI6405_USB_PRST_EN_OFFSET);
	hi64xx_hifi_write_reg(HI6405_USB20_CTRL_I_1, 0x400001);
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_PERI_CLKEN1_REG, HI6405_USB_HCLK_EN_OFFSET);
	hi64xx_hifi_reg_clr_bit(HI6405_DSP_AXI_CLKEN1_REG, HI6405_USB_HRST_EN_OFFSET);
	hi64xx_hifi_reg_write_bits(HI6405_SOUNDWIRE_CTRL0_REG, 0x0,
		0x1<<HI6405_USB_PCLK_EN_OFFSET);
}

int hi6405_hifi_config_init(struct snd_soc_codec *codec,
				struct hi64xx_resmgr *resmgr,
				struct hi64xx_irq *irqmgr,
				enum bustype_select bus_sel)
{
	struct hi64xx_dsp_config dsp_config;
	struct hi64xx_hifi_img_dl_config dl_config;
	int ret = 0;

	if (!codec || !resmgr|| !irqmgr)
		return -EINVAL;

	memset(&dsp_config, 0, sizeof(dsp_config));/* unsafe_function_ignore: memset */
	memset(&dl_config, 0, sizeof(dl_config));/* unsafe_function_ignore: memset */

	dsp_config.para_addr = HI6405_MLIBPARA_ADDR;
	dsp_config.msg_addr = HI6405_AP_TO_DSP_MSG_ADDR;
	dsp_config.rev_msg_addr = HI6405_DSP_TO_AP_MSG_ADDR;
	dsp_config.slimbus_load = false;
	dsp_config.codec_type = HI64XX_CODEC_TYPE_6405;
	dsp_config.cmd0_addr = HI6405_CMD0_ADDR;
	dsp_config.cmd1_addr = HI6405_CMD1_ADDR;
	dsp_config.cmd2_addr = HI6405_CMD2_ADDR;
	dsp_config.cmd3_addr = HI6405_CMD3_ADDR;
	dsp_config.wtd_irq_num = IRQ_WTD;
	dsp_config.vld_irq_num = IRQ_CMD_VALID;
	dsp_config.dump_ocram_addr = HI6405_DUMP_PANIC_STACK_ADDR;
	dsp_config.dump_ocram_size = HI6405_DUMP_PANIC_STACK_SIZE + HI6405_DUMP_CPUVIEW_SIZE;
	dsp_config.dump_log_addr = HI6405_SAVE_LOG_ADDR;
	dsp_config.dump_log_size = HI6405_SAVE_LOG_SIZE;
	dsp_config.ocram_start_addr = HI6405_OCRAM_BASE_ADDR;
	dsp_config.ocram_size = HI6405_OCRAM_SIZE;
	dsp_config.itcm_start_addr = HI6405_ITCM_BASE_ADDR;
	dsp_config.itcm_size = HI6405_ITCM_SIZE;
	dsp_config.dtcm_start_addr = HI6405_DTCM_BASE_ADDR;
	dsp_config.dtcm_size = HI6405_DTCM_SIZE;
	dsp_config.msg_state_addr = HI6405_DSP_MSG_STATE_ADDR;
	dsp_config.bus_sel = bus_sel;
	dsp_config.mlib_to_ap_msg_addr = HI6405_MLIB_TO_AP_MSG_ADDR;
	dsp_config.mlib_to_ap_msg_size = HI6405_MLIB_TO_AP_MSG_SIZE;

	dsp_config.dsp_ops.init = hi6405_hifi_init;
	dsp_config.dsp_ops.deinit = hi6405_hifi_deinit;
	dsp_config.dsp_ops.clk_enable = NULL;
	dsp_config.dsp_ops.ram2axi = NULL;
	dsp_config.dsp_ops.runstall = hi6405_hifi_runstall_cfg;
	dsp_config.dsp_ops.wtd_enable = hi6405_hifi_watchdog_enable;
	dsp_config.dsp_ops.uart_enable = NULL;
	dsp_config.dsp_ops.notify_dsp = hi6405_hifi_notify_dsp;
	dsp_config.dsp_ops.dsp_power_ctrl = hi6405_dsp_power_ctrl;
	dsp_config.dsp_ops.soundtrigger_fasttrans_ctrl = hi6405_soundtrigger_fasttrans_ctrl;
	dsp_config.dsp_ops.dsp_if_set_bypass = hi6405_dsp_if_set_bypass;
	dsp_config.dsp_ops.mad_enable = hi6405_mad_enable;
	dsp_config.dsp_ops.mad_disable = hi6405_mad_disable;
	dsp_config.dsp_ops.set_dsp_div = hi6405_set_dsp_div;
	dsp_config.dsp_ops.ir_path_clean = hi6405_clean_ir_study_path;
	dsp_config.dsp_ops.check_dp_clk = hi6405_check_dp_clk;
	dsp_config.dsp_ops.check_i2s2_clk = hi6405_check_i2s2_clk;
	dsp_config.dsp_ops.set_sample_rate = hi6405_dsp_if_set_sample_rate;
	dsp_config.dsp_ops.config_usb_low_power = hi6405_dsp_config_usb_low_power;

	dl_config.dspif_clk_en_addr = HI6405_DSP_DSPIF_CLK_EN;

	ret = hi64xx_hifi_misc_init(codec, resmgr, irqmgr, &dsp_config);
	if (ret)
		goto misc_init_err;

	ret = hi64xx_soundtrigger_init(CODEC_HI6405);
	if (ret)
		goto soundtrigger_init_err;

	ret = hi64xx_hifi_img_dl_init(irqmgr, &dl_config);
	if (ret)
		goto img_dl_init_err;

	ret = hi64xx_hifi_om_init(irqmgr, HI64XX_CODEC_TYPE_6405);
	if (ret)
		goto om_init_err;


	return 0;

om_init_err:
	hi64xx_hifi_img_dl_deinit();
img_dl_init_err:
soundtrigger_init_err:
	hi64xx_hifi_misc_deinit();
misc_init_err:
	HI64XX_DSP_ERROR("hi6405 dsp config init failed\n");

	return ret;
}
EXPORT_SYMBOL(hi6405_hifi_config_init);

void hi6405_hifi_config_deinit(void)
{

	hi64xx_hifi_misc_deinit();

	hi64xx_hifi_img_dl_deinit();

	hi64xx_hifi_om_deinit();

	HI64XX_DSP_INFO("%s--\n", __FUNCTION__);


}
EXPORT_SYMBOL(hi6405_hifi_config_deinit);

MODULE_DESCRIPTION("hi64xx hifi misc driver");
MODULE_LICENSE("GPL");
