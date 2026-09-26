/* Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_FB_CONFIG_H
#define HISI_FB_CONFIG_H

#if defined(CONFIG_VIDEO_IDLE)
#define VIDEO_IDLE_GPU_COMPOSE_ENABLE 1
#else
#define VIDEO_IDLE_GPU_COMPOSE_ENABLE 0
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
#define SUPPORT_HEBC_ROT_CAP
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
#define SUPPORT_SPR_DSC1_2
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V346)
#define SUPPORT_SPR_DSC1_2_SEPARATE
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
#define MIPI_RESOURCE_PROTECTION
/* mipi_dsi_bit_clk_update wait count, unit is ms */
#define MIPI_CLK_UPDT_TIMEOUT 30
/* esd check wait for mipi_resource_available count, unit is ms */
#define ESD_WAIT_MIPI_AVAILABLE_TIMEOUT 64
#endif

#if defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V360) || \
	defined(CONFIG_HISI_FB_V346)
#define SUPPORT_FPGA_SUSPEND_RESUME
#endif

#if defined(CONFIG_HISI_FB_V320) || defined(CONFIG_HISI_FB_V330) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
#define LOW_TEMP_VOLTAGE_LIMIT
#endif

#if defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346) || \
	defined(CONFIG_HISI_FB_V360)
/* support dynamic clk adapter (DCA) without voltage vote */
#define SUPPORT_DCA_WITHOUT_VOLT
#endif

#if defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
/* support copybit offline compose use wch scaler */
#define SUPPORT_COPYBIT_WCH_SCL
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
/* after 990, dsi instead ldi */
#define SUPPORT_DSI_VER_2_0

/* after 990, if dsc configure to single pipe, pic_width must
 * consistent with DDIC,when dsc configure to double pipe,pic_width
 * must be half the size of the panel
 */
#define SUPPORT_DSC_PIC_WIDTH

/* after 990, supports two dpp channels */
#define SUPPORT_DISP_CHANNEL
#endif

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346) || \
	defined(CONFIG_HISI_FB_V360)
#define SUPPORT_DSS_BUS_IDLE_CTRL
#endif

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
#define SUPPORT_FULL_CHN_10BIT_COLOR
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
/* after 990, supports copybit vote(core clock,voltage) */
#define SUPPORT_COPYBIT_VOTE_CTRL
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V345) ||  defined(CONFIG_HISI_FB_V346) || \
	defined(CONFIG_HISI_FB_V360)
/* after 990, dump disp dbuf dsi info */
#define SUPPORT_DUMP_REGISTER_INFO
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
#define SUPPORT_DSS_HEBCE
#define SUPPORT_DSS_HEBCD
#endif

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V360) || \
	defined(CONFIG_HISI_FB_V346)
#define SUPPORT_DSS_HFBCD
#endif

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
#define SUPPORT_DSS_HFBCE
#endif

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_V330)
#define SUPPORT_DSS_AFBC_ENHANCE
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V345) || \
	defined(CONFIG_HISI_FB_V346) || defined(CONFIG_HISI_FB_V360)
#define ESD_RESTORE_DSS_VOLTAGE_CLK
#endif

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346) || \
	defined(CONFIG_HISI_FB_V360)
/* low power cmds trans under video mode */
#define MIPI_DSI_HOST_VID_LP_MODE
#endif

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346) || \
	defined(CONFIG_HISI_FB_V360)
/* after 501, mmbuf's clock is configurable */
#define SUPPORT_SET_MMBUF_CLK
#endif

#if defined(CONFIG_HISI_FB_V410)
/* The mmbuf clock of the 970 is not dynamically calculated and adjusted.
* To solve the performance problems caused by insufficient mmbuf clocks in some scenarios,
* the mmbuf clock is adjusted to the same level as the current core clock.
* In this way, power consumption does not increase.
*/
#define ADJUST_MMBUF_BY_PRI_CLK
#endif

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_V330)
/* after 970, no need to manually clean cmdlist */
#define SUPPORT_CLR_CMDLIST_BY_HARDWARE
#endif

#if defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346) || \
	defined(CONFIG_HISI_FB_V360)
/* after 350, 600, supports afbcd online rot */
#define SUPPORT_AFBCD_ROT
#endif

#if defined(CONFIG_HISI_FB_V320) || defined(CONFIG_HISI_FB_V330) || \
	defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501)
#define PANEL_MODE_SWITCH_SUPPORT
#endif

#if defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V345) || defined(CONFIG_HISI_FB_V346)
/* when switching the frequency, some platforms switch the clock source first and then the frequency division ratio.
 * when the core clk frequency is switched across clock sources, after the clock source is switched successfully
 * and before the frequency division ratio switching takes effect, for a moment,
 * the old division ratio will possiblily generate a higher frequency than expected on the new clock source.
 * to solve this problem, we need to choose a lower frequency on the old clock source,
 * its division ratio produces a lower temporary frequency on the new clock source
 */
#define NEED_SWITCH_TEMP_CORE_CLK
#endif

#if defined(CONFIG_HISI_FB_V600)
#define SUPPORT_LOW_TEMPERATURE_PROTECT
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V360)
#define SUPPORT_DP_VER_1_4_LATER
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
#define SUPPORT_DP_DSC
#endif

#if defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V360)
#define SUPPORT_HISI_DPTX
#endif

#if defined(CONFIG_HISI_FB_V600)
#define SUPPORT_BALANCED_AIF_DISTRIBUTION
#endif

#endif /* HISI_FB_CONFIG_H */

