/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#ifndef HISI_FB_PANEL_ENUM_H
#define HISI_FB_PANEL_ENUM_H

#include "hisi_fb_def.h"
#include "hisi_mipi_dsi.h"
#include "hisi_dss.h"
#include "hisi_dp.h"
#include "hisi_spr_dsc.h"

/* panel type list */
#define PANEL_NO BIT(0) /* No Panel */
#define PANEL_LCDC BIT(1) /* internal LCDC type */
#define PANEL_HDMI BIT(2) /* HDMI TV */
#define PANEL_MIPI_VIDEO BIT(3) /* MIPI */
#define PANEL_MIPI_CMD BIT(4) /* MIPI */
#define PANEL_DUAL_MIPI_VIDEO BIT(5) /* DUAL MIPI */
#define PANEL_DUAL_MIPI_CMD BIT(6) /* DUAL MIPI */
#define PANEL_DP BIT(7) /* DisplayPort */
#define PANEL_MIPI2RGB BIT(8) /* MIPI to RGB */
#define PANEL_RGB2MIPI BIT(9) /* RGB to MIPI */
#define PANEL_OFFLINECOMPOSER BIT(10) /* offline composer */
#define PANEL_WRITEBACK BIT(11) /* Wifi display */
#define PANEL_MEDIACOMMON BIT(12) /* mediacommon */

/* dts initial */
#define DTS_FB_RESOURCE_INIT_READY BIT(0)
#define DTS_PWM_READY BIT(1)
#define DTS_SPI_READY BIT(3)
#define DTS_PANEL_PRIMARY_READY BIT(4)
#define DTS_PANEL_EXTERNAL_READY BIT(5)
#define DTS_PANEL_OFFLINECOMPOSER_READY BIT(6)
#define DTS_PANEL_WRITEBACK_READY BIT(7)
#define DTS_PANEL_MEDIACOMMON_READY BIT(8)

/* device name */
#define DEV_NAME_DSS_DPE "dss_dpe"
#define DEV_NAME_SPI "spi_dev0"
#define DEV_NAME_HDMI "hdmi"
#define DEV_NAME_DP "dp"
#define DEV_NAME_MIPI2RGB "mipi2rgb"
#define DEV_NAME_RGB2MIPI "rgb2mipi"
#define DEV_NAME_MIPIDSI "mipi_dsi"
#define DEV_NAME_FB "hisi_fb"
#define DEV_NAME_PWM "hisi_pwm"
#define DEV_NAME_BLPWM "hisi_blpwm"
#define DEV_NAME_LCD_BKL "lcd_backlight0"

/* vcc name */
#define REGULATOR_PDP_NAME "regulator_dsssubsys"
#define REGULATOR_MEDIA_NAME "regulator_media_subsys"
#ifdef CONFIG_DSS_SMMU_V3
#define REGULATOR_SMMU_TCU_NAME "regulator_smmu_tcu"
#else /* mmbuf don't be used, stub smmuv3 regulator */
#define REGULATOR_SMMU_TCU_NAME "regulator_mmbuf"
#endif

/* irq name */
#define IRQ_PDP_NAME "irq_pdp"
#define IRQ_SDP_NAME "irq_sdp"
#define IRQ_ADP_NAME "irq_adp"
#define IRQ_MDC_NAME "irq_mdc"
#define IRQ_DSI0_NAME "irq_dsi0"
#define IRQ_DSI1_NAME "irq_dsi1"

/* dts compatible */
#define DTS_COMP_FB_NAME "hisilicon,hisifb"
#define DTS_COMP_PWM_NAME "hisilicon,hisipwm"
#define DTS_COMP_BLPWM_NAME "hisilicon,hisiblpwm"
#define DTS_PATH_LOGO_BUFFER "/reserved-memory/logo-buffer"

/* lcd resource name */
#define LCD_BL_TYPE_NAME "lcd-bl-type"
#define FPGA_FLAG_NAME "fpga_flag"
#define LCD_DISPLAY_TYPE_NAME "lcd-display-type"
#define LCD_IFBC_TYPE_NAME "lcd-ifbc-type"

/* backlight type */
#define BL_SET_BY_NONE BIT(0)
#define BL_SET_BY_PWM BIT(1)
#define BL_SET_BY_BLPWM BIT(2)
#define BL_SET_BY_MIPI BIT(3)
#define BL_SET_BY_SH_BLPWM BIT(4)

/* supported display effect type */
#define COMFORM_MODE BIT(0)
#define ACM_COLOR_ENHANCE_MODE BIT(1)
#define IC_COLOR_ENHANCE_MODE BIT(2)
#define CINEMA_MODE BIT(3)
#define VR_MODE BIT(4)
#define FPS_30_60_SENCE_MODE BIT(5)
#define LED_RG_COLOR_TEMP_MODE BIT(16)
#define GAMMA_MAP BIT(19)

#define DFR_METHOD_HPORCH BIT(0)
#define DFR_METHOD_VPORCH BIT(1)
#define DFR_METHOD_CLK BIT(2)

#define LCD_BL_IC_NAME_MAX 50

#define DEV_DSS_VOLTAGE_ID 20
#define DEV_LDI1_VOLTAGE_ID 23

#define PIPE_CLK_HPORCH_SET_ID 3

/* dsc type */
#define DSC_VERSION_1_1 0
#define DSC_VERSION_1_2 1

#define MIPI_DPHY_NUM 2
#define DE_EMPHASIS_REG_NUM 7

#define PANEL_10BIT_VIDEO_MODE_STR "10bitvid"
#define PANEL_8BIT_CMD_MODE_STR "08bitcmd"

/* frame rate type */
#define FPS_60HZ 60
#define FPS_HIGH_60HZ (FPS_60HZ + 1)  /* the real fps of hight60 mode is also 60 */
#define FPS_90HZ 90
#define FPS_120HZ 120

/* blpwm precision type */
enum BLPWM_PRECISION_TYPE {
	BLPWM_PRECISION_DEFAULT_TYPE = 0,
	BLPWM_PRECISION_10000_TYPE = 1,
	BLPWM_PRECISION_2048_TYPE = 2,
	BLPWM_PRECISION_4096_TYPE = 3,
};

/* LCD init step */
enum LCD_INIT_STEP {
	LCD_INIT_NONE = 0,
	LCD_INIT_POWER_ON,
	LCD_INIT_LDI_SEND_SEQUENCE,
	LCD_INIT_MIPI_LP_SEND_SEQUENCE,
	LCD_INIT_MIPI_HS_SEND_SEQUENCE,
};

/* LCD uninit step */
enum LCD_UNINIT_STEP {
	LCD_UNINIT_NONE = 0,
	LCD_UNINIT_POWER_OFF,
	LCD_UNINIT_LDI_SEND_SEQUENCE,
	LCD_UNINIT_MIPI_LP_SEND_SEQUENCE,
	LCD_UNINIT_MIPI_HS_SEND_SEQUENCE,
};

enum LCD_ESD_RECOVER_STEP {
	LCD_ESD_RECOVER_NONE = 0,
	LCD_ESD_RECOVER_POWER_OFF,
	LCD_ESD_RECOVER_POWER_ON,
};

/* LCD REFRESH DIRECTION */
enum LCD_REFRESH_DIRECTION {
	LCD_REFRESH_LEFT_TOP = 0,
	LCD_REFRESH_RIGHT_TOP,
	LCD_REFRESH_LEFT_BOTTOM,
	LCD_REFRESH_RIGHT_BOTTOM,
};

/* IFBC compress mode */
enum IFBC_TYPE {
	IFBC_TYPE_NONE = 0,
	IFBC_TYPE_ORISE2X,
	IFBC_TYPE_ORISE3X,
	IFBC_TYPE_HIMAX2X,
	IFBC_TYPE_RSP2X,
	IFBC_TYPE_RSP3X,
	IFBC_TYPE_VESA2X_SINGLE,
	IFBC_TYPE_VESA3X_SINGLE,
	IFBC_TYPE_VESA2X_DUAL,
	IFBC_TYPE_VESA3X_DUAL,
	IFBC_TYPE_VESA3_75X_DUAL,
	IFBC_TYPE_VESA4X_SINGLE_SPR,
	IFBC_TYPE_VESA4X_DUAL_SPR,
	IFBC_TYPE_VESA2X_SINGLE_SPR,
	IFBC_TYPE_VESA2X_DUAL_SPR,
	IFBC_TYPE_VESA3_75X_SINGLE,

	IFBC_TYPE_MAX
};

/* IFBC compress mode */
enum IFBC_COMP_MODE {
	IFBC_COMP_MODE_0 = 0,
	IFBC_COMP_MODE_1,
	IFBC_COMP_MODE_2,
	IFBC_COMP_MODE_3,
	IFBC_COMP_MODE_4,
	IFBC_COMP_MODE_5,
	IFBC_COMP_MODE_6,
};

// spr + dsc mode
enum SPR_DSC_MODE {
	SPR_DSC_MODE_NONE = 0,
	SPR_DSC_MODE_SPR_ONLY,
	SPR_DSC_MODE_DSC_ONLY,
	SPR_DSC_MODE_SPR_AND_DSC,
};

/* xres_div */
enum XRES_DIV {
	XRES_DIV_1 = 1,
	XRES_DIV_2,
	XRES_DIV_3,
	XRES_DIV_4,
	XRES_DIV_5,
	XRES_DIV_6,
};

/* yres_div */
enum YRES_DIV {
	YRES_DIV_1 = 1,
	YRES_DIV_2,
	YRES_DIV_3,
	YRES_DIV_4,
	YRES_DIV_5,
	YRES_DIV_6,
};

/* pxl0_divxcfg */
enum PXL0_DIVCFG {
	PXL0_DIVCFG_0 = 0,
	PXL0_DIVCFG_1,
	PXL0_DIVCFG_2,
	PXL0_DIVCFG_3,
	PXL0_DIVCFG_4,
	PXL0_DIVCFG_5,
	PXL0_DIVCFG_6,
	PXL0_DIVCFG_7,
};

/* pxl0_div2_gt_en */
enum PXL0_DIV2_GT_EN {
	PXL0_DIV2_GT_EN_CLOSE = 0,
	PXL0_DIV2_GT_EN_OPEN,
};

/* pxl0_div4_gt_en */
enum PXL0_DIV4_GT_EN {
	PXL0_DIV4_GT_EN_CLOSE = 0,
	PXL0_DIV4_GT_EN_OPEN,
};

/* used for 8bit 10bit switch */
enum PANEL_MODE {
	MODE_8BIT = 0,
	MODE_10BIT_VIDEO_3X,
};

enum PARA_UPDT_STATE {
	PARA_UPDT_END = 0,
	PARA_UPDT_DOING,
	PARA_UPDT_NEED,
};

/* pxl0_dsi_gt_en */
enum PXL0_DSI_GT_EN {
	PXL0_DSI_GT_EN_0 = 0,
	PXL0_DSI_GT_EN_1,
	PXL0_DSI_GT_EN_2,
	PXL0_DSI_GT_EN_3,
};

enum VSYNC_CTRL_TYPE {
	VSYNC_CTRL_NONE = 0x0,
	VSYNC_CTRL_ISR_OFF = BIT(0),
	VSYNC_CTRL_MIPI_ULPS = BIT(1),
	VSYNC_CTRL_CLK_OFF = BIT(2),
	VSYNC_CTRL_VCC_OFF = BIT(3),
};

enum {
	PANEL_NORMAL_TYPE = 0,
	PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE = 1,
};

/*
 * before the screen folding:               after the screen is folded:
 * connect(bit2)  inner(bit1)  outer(bit0)  connect(bit6)  inner(bit5)  outer(bit4)  mode
 *  inner(0)       on(1)        on(1)        inner(0)       on(1)        off(0)      0x23
 *  inner(0)       on(1)        on(1)        outer(1)       off(0)       on(1)       0x53
 *  inner(0)       on(1)        off(0)       outer(1)       off(0)       on(1)       0x52
 *  inner(0)       off(0)       off(0)       outer(1)       off(0)       off(0)      0x40
 *  outer(1)       on(1)        on(1)        outer(1)       off(0)       on(1)       0x57
 *  outer(1)       on(1)        on(1)        inner(0)       on(1)        off(0)      0x27
 *  outer(1)       off(0)       on(1)        inner(0)       on(1)        off(0)      0x25
 *  outer(1)       off(0)       off(0)       inner(0)       on(1)        off(0)      0x24
 *  outer(1)       off(0)       on(1)        inner(0)       on(1)        on(1)       0x35
 *  inner(0)       on(1)        off(0)       outer(1)       on(1)        on(1)       0x72
 *  inner(0)       on(1)        on(1)        outer(1)       on(1)        on(1)       0x73
 *  outer(1)       on(1)        on(1)        inner(0)       on(1)        on(1)       0x37
 */
enum {
	EN_DISPLAY_FOLD_MODE0 = 0x23,
	EN_DISPLAY_FOLD_MODE1 = 0x53,
	EN_DISPLAY_FOLD_MODE2 = 0x52,
	EN_DISPLAY_FOLD_MODE3 = 0x40,
	EN_DISPLAY_FOLD_MODE4 = 0x57,
	EN_DISPLAY_FOLD_MODE5 = 0x27,
	EN_DISPLAY_FOLD_MODE6 = 0x25,
	EN_DISPLAY_FOLD_MODE7 = 0x24,
	EN_DISPLAY_FOLD_MODE8 = 0x35,
	EN_DISPLAY_FOLD_MODE9 = 0x72,
	EN_DISPLAY_FOLD_MODE10 = 0x73,
	EN_DISPLAY_FOLD_MODE11 = 0x37,
};

enum {
	NORMAL_POWER_ON_OFF = 0x0,
	SKIP_POWER_ON_OFF = 0x1,
	FOLD_POWER_ON_OFF = 0x2,
	/* since FOLD_POWER_ON_OFF is used by lcd, we define FOLD_POWER_ON_OFF_FOR_EFFECT to redeclare it */
	FOLD_POWER_ON_OFF_FOR_EFFECT = 0x3,
};

enum {
	EN_INNER_OUTER_PANEL_OFF = 0x0,
	EN_INNER_PANEL_ON = 0x1,
	EN_OUTER_PANEL_ON = 0x2,
	EN_INNER_OUTER_PANEL_ON = 0x3,
};

/* dtype for vcc */
enum {
	DTYPE_VCC_GET,
	DTYPE_VCC_PUT,
	DTYPE_VCC_ENABLE,
	DTYPE_VCC_DISABLE,
	DTYPE_VCC_SET_VOLTAGE,
};

/* pinctrl operation */
enum {
	DTYPE_PINCTRL_GET,
	DTYPE_PINCTRL_STATE_GET,
	DTYPE_PINCTRL_SET,
	DTYPE_PINCTRL_PUT,
};

/* pinctrl state */
enum {
	DTYPE_PINCTRL_STATE_DEFAULT,
	DTYPE_PINCTRL_STATE_IDLE,
};

/* dtype for gpio */
enum {
	DTYPE_GPIO_REQUEST,
	DTYPE_GPIO_FREE,
	DTYPE_GPIO_INPUT,
	DTYPE_GPIO_OUTPUT,
};

enum {
	IFBC_ORISE_CTL_8LINE = 0,
	IFBC_ORISE_CTL_16LINE,
	IFBC_ORISE_CTL_32LINE,
	IFBC_ORISE_CTL_FRAME,
};

enum mipi_dsi_phy_mode {
	DPHY_MODE = 0,
	CPHY_MODE,
};

enum frame_rate_update_status {
	FRM_UPDT_DONE = 0,
	FRM_UPDT_CONFIG,
	FRM_UPDT_CONFIG_DONE,
	FRM_UPDT_NEED_DOING,
	FRM_UPDT_DOING,
	FRM_UPDT_SET_REG,
};

/* for display time info */
enum {
	EN_TIME_STATE_IDLE = 0,
	EN_TIME_STATE_WORK,
};

enum {
	EN_AGING_PANEL_MAIN = 0,
	EN_AGING_PANEL_SUB,
	EN_AGING_PANEL_SIDE,
	EN_AGING_PANEL_NUM
};
#endif /* HISI_FB_PANEL_ENUM_H */
