/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __HISI_DP_H__
#define __HISI_DP_H__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include "hisi_dss_ion.h"
#include "hisi_dss.h"
#include "hisi_fb_config.h"

#if CONFIG_DP_ENABLE
#include <linux/switch.h>
#endif
#include <drm/drm_dp_helper.h>
#include "dp/link/drm_dp_helper_additions.h"
#include "dp/dp_dbg.h"
#include "dp/link/dp_aux.h"
#include <drm/drm_fixed.h>
#include <drm/drm_dp_mst_helper.h>
#include "dsc/dsc_algorithm.h"

#define PARSE_EST_TIMINGS_FROM_BYTE3
#define DPTX_COMBO_PHY
#define DPTX_TYPE_C
#define DPTX_DEBUG_REG
#define CONFIG_DP_EDID_DEBUG
#define CONFIG_DP_SETTING_COMBOPHY 1
#define DP_TIME_INFO_SIZE		(24)
#define DP_MONTIOR_NAME_SIZE (11)

#define DP_PLUG_TYPE_NORMAL 0
#define DP_PLUG_TYPE_FLIPPED 1

#define DPTX_RECEIVER_CAP_SIZE	(0x100)
#define DPTX_SDP_NUM		(0x10)
#define DPTX_SDP_LEN	(0x9)
#define DPTX_SDP_SIZE	(9 * 4)
#define DPTX_DEFAULT_EDID_BUFLEN	(128UL)

#define INFOFRAME_PACKET_TYPE_HDR 0x07
#define MAX_INFOFRAME_LENGTH 27
#define INFOFRAME_REGISTER_SIZE 32
#define INFOFRAME_DATA_SIZE 8
#define DATA_NUM_PER_REG (INFOFRAME_REGISTER_SIZE / INFOFRAME_DATA_SIZE)
#define HDR_INFOFRAME_VERSION 0x01
#define HDR_INFOFRAME_LENGTH 26
#define HDR_INFOFRAME_EOTF_BYTE_NUM 0
#define HDR_INFOFRAME_SMPTE_ST_2084 2
#define STATIC_MATADATA_TYPE_1 0
#define HDR_INFOFRAME_METADATA_ID_BYTE_NUM 1
#define HDR_INFOFRAME_DISP_PRI_X_0_LSB 2
#define HDR_INFOFRAME_DISP_PRI_X_0_MSB 3
#define HDR_INFOFRAME_DISP_PRI_Y_0_LSB 4
#define HDR_INFOFRAME_DISP_PRI_Y_0_MSB 5
#define HDR_INFOFRAME_DISP_PRI_X_1_LSB 6
#define HDR_INFOFRAME_DISP_PRI_X_1_MSB 7
#define HDR_INFOFRAME_DISP_PRI_Y_1_LSB 8
#define HDR_INFOFRAME_DISP_PRI_Y_1_MSB 9
#define HDR_INFOFRAME_DISP_PRI_X_2_LSB 10
#define HDR_INFOFRAME_DISP_PRI_X_2_MSB 11
#define HDR_INFOFRAME_DISP_PRI_Y_2_LSB 12
#define HDR_INFOFRAME_DISP_PRI_Y_2_MSB 13
#define HDR_INFOFRAME_WHITE_POINT_X_LSB 14
#define HDR_INFOFRAME_WHITE_POINT_X_MSB 15
#define HDR_INFOFRAME_WHITE_POINT_Y_LSB 16
#define HDR_INFOFRAME_WHITE_POINT_Y_MSB 17
#define HDR_INFOFRAME_MAX_LUMI_LSB 18
#define HDR_INFOFRAME_MAX_LUMI_MSB 19
#define HDR_INFOFRAME_MIN_LUMI_LSB 20
#define HDR_INFOFRAME_MIN_LUMI_MSB 21
#define HDR_INFOFRAME_MAX_LIGHT_LEVEL_LSB 22
#define HDR_INFOFRAME_MAX_LIGHT_LEVEL_MSB 23
#define HDR_INFOFRAME_MAX_AVERAGE_LEVEL_LSB 24
#define HDR_INFOFRAME_MAX_AVERAGE_LEVEL_MSB 25
#define LSB_MASK 0x00FF
#define MSB_MASK 0xFF00
#define SHIFT_8BIT 8

#define DP_SYSFS_ATTRS_NUM	(10)

#if defined(CONFIG_DP_HDCP)
#define DPTX_HDCP_REG_DPK_CRC_ORIG	0x331c1169
#define DPTX_HDCP_MAX_AUTH_RETRY	10
#define DPTX_HDCP_MAX_REPEATER_AUTH_RETRY 5
#endif

#ifdef SUPPORT_HISI_DPTX
#define DPTX_COMBOPHY_PARAM_NUM				20
#define DPTX_COMBOPHY_SSC_PPM_PARAM_NUM		12
#else
#define DPTX_COMBOPHY_PARAM_NUM		21
#endif

#define DPTX_AUX_TIMEOUT	(2000)

#define DEFAULT_AUXCLK_DPCTRL_RATE	16000000UL
#define DEFAULT_ACLK_DPCTRL_RATE_ES	288000000UL

#define PIXELS_PER_GROUP 3
#define PIXEL_HOLD_DELAY 5
#define PIXEL_GROUP_DELAY 3
#define MUXER_INITIAL_BUFFERING_DELAY 9
#define DSC_MAX_AUX_ORIG_WIDTH 24
#define PIXEL_FLATNESSBUF_DELAY DSC_MAX_AUX_ORIG_WIDTH
#define INITIAL_SCALE_VALUE_SHIFT 3
#define DSC_DEFAULT_DECODER 2

struct rc_range_param {
	uint32_t minQP     :5;
	uint32_t maxQP    :5;
	uint32_t offset      :6;
};

/* Rounding up to the nearest multiple of a number */
#define ROUND_UP_TO_NEAREST(numToRound, mult) (((numToRound+(mult)-1) / (mult)) * (mult))

#ifdef SUPPORT_HISI_DPTX
#define DEFAULT_ACLK_DPCTRL_RATE 207500000UL
#define DEFAULT_MIDIA_PPLL7_CLOCK_FREQ 1290000000UL
#define KIRIN_VCO_MIN_FREQ_OUPUT         800000 /* dssv501: 800 * 1000 */
#define KIRIN_SYS_FREQ   38400 /* dssv501: 38.4 * 1000 */
#define MIDIA_PPLL7_CTRL0	0x838
#define MIDIA_PPLL7_CTRL1	0x83c
#define TX_VBOOST_ADDR		0x21
#define PERI_VOLTAGE_060V_CLK 207500000UL
#define PERI_VOLTAGE_065V_CLK 332000000UL
#define PERI_VOLTAGE_070V_CLK 415000000UL
#define PERI_VOLTAGE_080V_CLK 533330000UL
#define PPLL7_DIV_VOLTAGE_060V 7
#define PPLL7_DIV_VOLTAGE_065V 4
#define PPLL7_DIV_VOLTAGE_070V 4
#define PPLL7_DIV_VOLTAGE_080V 3

#define CTRL_TYPE_SELECT_OFFSET 0x54
#define CTRL_TYPE_SELECT_BIT BIT(3)
#define PHY_CLK_GATE_BIT BIT(4)

#define DPTX_CTRL_RESET_OFFSET 0x20
#define DPTX_CTRL_DIS_RESET_OFFSET 0x24
#define HIDPTX_CTRL_DIS_RESET_BIT BIT(16)
#define HIDPTX_CTRL_RESET_BIT BIT(16)
#define HIDPC_CTRL_DIS_RESET_BIT BIT(3)
#define HIDPC_CTRL_RESET_BIT BIT(3)
#define DPTX_CTRL_STATUS_OFFSET 0x28
#define HIDPTX_CTRL_RESET_STATUS_BIT BIT(16)
#define HIDPC_CTRL_RESET_STATUS_BIT BIT(3)

#define HIDPC_PCLK_HSDT1_GATE_OFFSET 0x10 /* pixel clock open gate */
#define HIDPC_PCLK_HSDT1_GATE_CLOSE_OFFSET 0x14 /* pixel clock close gate */
#define HIDPC_PCLK_HSDT1_GATE_MASK GENMASK(1, 0)
#endif

#ifdef CONFIG_HISI_FB_V510
#define DEFAULT_ACLK_DPCTRL_RATE 207500000UL
#define DEFAULT_MIDIA_PPLL7_CLOCK_FREQ 1290000000UL
#define KIRIN_VCO_MIN_FREQ_OUPUT         800000 /* dssv501: 800 * 1000 */
#define KIRIN_SYS_FREQ   38400 /* dssv501: 38.4 * 1000 */
#define MIDIA_PPLL7_CTRL0	0x530
#define MIDIA_PPLL7_CTRL1	0x534
#define MIDIA_PERI_CTRL4	0x350
#define TX_VBOOST_ADDR		0x21
#define PERI_VOLTAGE_060V_CLK 213000000UL
#define PERI_VOLTAGE_065V_CLK 332000000UL
#define PERI_VOLTAGE_070V_CLK 415000000UL
#define PERI_VOLTAGE_080V_CLK 600000000UL
#define PPLL7_DIV_VOLTAGE_060V 7
#define PPLL7_DIV_VOLTAGE_065V 4
#define PPLL7_DIV_VOLTAGE_070V 4
#define PPLL7_DIV_VOLTAGE_080V 3
#endif

#ifdef CONFIG_HISI_FB_V501
#define DEFAULT_ACLK_DPCTRL_RATE 207500000UL
#define DEFAULT_MIDIA_PPLL7_CLOCK_FREQ 1290000000UL
#define KIRIN_VCO_MIN_FREQ_OUPUT         800000 /* dssv501: 800 * 1000 */
#define KIRIN_SYS_FREQ   38400 /* dssv501: 38.4 * 1000 */
#define MIDIA_PPLL7_CTRL0	0x530
#define MIDIA_PPLL7_CTRL1	0x534
#define MIDIA_PERI_CTRL4	0x350
#define TX_VBOOST_ADDR		0x21
#define PERI_VOLTAGE_065V_CLK 300000000UL
#define PERI_VOLTAGE_070V_CLK 415000000UL
#define PERI_VOLTAGE_080V_CLK 594000000UL
#define PPLL7_DIV_VOLTAGE_065V 5
#define PPLL7_DIV_VOLTAGE_070V 4
#define PPLL7_DIV_VOLTAGE_080V 3
#endif

#ifdef CONFIG_HISI_FB_970
#define DEFAULT_ACLK_DPCTRL_RATE	208000000UL
#define DEFAULT_MIDIA_PPLL7_CLOCK_FREQ	1782000000UL
#define KIRIN_VCO_MIN_FREQ_OUPUT         1000000 /* Boston: 1000 * 1000 */
#define KIRIN_SYS_FREQ   19200 /* Boston: 19.2f * 1000 */
#define MIDIA_PPLL7_CTRL0	0x50c
#define MIDIA_PPLL7_CTRL1	0x510
#define MIDIA_PERI_CTRL4	0x350
#define TX_VBOOST_ADDR		0xf
#define PERI_VOLTAGE_065V_CLK 255000000UL
#define PERI_VOLTAGE_070V_CLK 415000000UL
#define PERI_VOLTAGE_080V_CLK 594000000UL
#define PPLL7_DIV_VOLTAGE_065V 7
#define PPLL7_DIV_VOLTAGE_070V 5
#define PPLL7_DIV_VOLTAGE_080V 3
#endif

#define DEFAULT_MIDIA_PPLL7_CLOCK_FREQ_SAVEMODE	223000000UL

#define MAX_DIFF_SOURCE_X_SIZE	1920

#define MIDIA_PPLL7_FREQ_DEVIDER_MASK	GENMASK(25, 2)
#define MIDIA_PPLL7_FRAC_MODE_MASK	GENMASK(25, 0)

#ifndef SUPPORT_HISI_DPTX
#define PMCTRL_PERI_CTRL4_TEMPERATURE_MASK		GENMASK(27, 26)
#define PMCTRL_PERI_CTRL4_TEMPERATURE_SHIFT		26
#define NORMAL_TEMPRATURE 0
#endif

#if defined(CONFIG_DP_HDCP)
#define ACCESS_REGISTER_FN_MAIN_ID_HDCP           0xc500aa01
#define ACCESS_REGISTER_FN_SUB_ID_HDCP_CTRL   (0x55bbccf1)
#define ACCESS_REGISTER_FN_SUB_ID_HDCP_INT   (0x55bbccf2)
#endif

/* #define DPTX_DEVICE_INFO(pdev) platform_get_drvdata(pdev)->panel_info->dp */

#define DPTX_CHANNEL_NUM_MAX 8
#define DPTX_DATA_WIDTH_MAX 24

enum dp_ctrl_type {
	HIDPTX = 0,
	HIDPC = 1,
};

enum dp_tx_hpd_states {
	HPD_OFF,
	HPD_ON,
};

enum pixel_enc_type {
	RGB = 0,
	YCBCR420 = 1,
	YCBCR422 = 2,
	YCBCR444 = 3,
	YONLY = 4,
	RAW = 5
};

enum color_depth {
	COLOR_DEPTH_INVALID = 0,
	COLOR_DEPTH_6 = 6,
	COLOR_DEPTH_8 = 8,
	COLOR_DEPTH_10 = 10,
	COLOR_DEPTH_12 = 12,
	COLOR_DEPTH_16 = 16
};

enum pattern_mode {
	TILE = 0,
	RAMP = 1,
	CHESS = 2,
	COLRAMP = 3
};

enum dynamic_range_type {
	CEA = 1,
	VESA = 2
};

enum colorimetry_type {
	ITU601 = 1,
	ITU709 = 2
};

enum video_format_type {
	VCEA = 0,
	CVT = 1,
	DMT = 2
};

enum established_timings {
	DMT_640x480_60hz,
	DMT_800x600_60hz,
	DMT_1024x768_60hz,
	NONE
};

enum iec_samp_freq_value {
	IEC_SAMP_FREQ_44K = 0,
	IEC_SAMP_FREQ_48K = 2,
	IEC_SAMP_FREQ_32K = 3,
	IEC_SAMP_FREQ_88K = 8,
	IEC_SAMP_FREQ_96K = 10,
	IEC_SAMP_FREQ_176K = 12,
	IEC_SAMP_FREQ_192K = 14,
};

enum iec_orig_samp_freq_value {
	IEC_ORIG_SAMP_FREQ_192K = 1,
	IEC_ORIG_SAMP_FREQ_176K = 3,
	IEC_ORIG_SAMP_FREQ_96K = 5,
	IEC_ORIG_SAMP_FREQ_88K = 7,
	IEC_ORIG_SAMP_FREQ_32K = 12,
	IEC_ORIG_SAMP_FREQ_48K = 13,
	IEC_ORIG_SAMP_FREQ_44K = 15,
};

enum dptx_hot_plug_type {
	HOT_PLUG_OUT = 0,
	HOT_PLUG_IN,
	HOT_PLUG_TEST,
	HOT_PLUG_IN_VR,
	HOT_PLUG_OUT_VR,
	HOT_PLUG_HDCP13_SUCCESS,
	HOT_PLUG_HDCP13_TIMEOUT,
	HOT_PLUG_HDCP13_FAIL,
	HOT_PLUG_HDCP13_POSTAUTH,
	HOT_PLUG_HDCP_ENABLE,
	HOT_PLUG_HDCP_DISABLE,
	HOT_PLUG_HDCP_OUT,
	HOT_PLUG_HDCP_CP_IRQ,
	HOT_PLUG_TEST_OUT,
	HOT_PLUG_MAINPANEL_UP,
	HOT_PLUG_MAINPANEL_DOWN,
};

enum dptx_sdp_config_type {
	SDP_CONFIG_TYPE_DSC,
	SDP_CONFIG_TYPE_HDR,
	SDP_CONFIG_TYPE_AUDIO,
};

/**
 * struct dptx_link - The link state.
 * @status: Holds the sink status register values.
 * @trained: True if the link is successfully trained.
 * @rate: The current rate that the link is trained at.
 * @lanes: The current number of lanes that the link is trained at.
 * @preemp_level: The pre-emphasis level used for each lane.
 * @vswing_level: The vswing level used for each lane.
 */
struct dptx_link {
	uint8_t status[DP_LINK_STATUS_SIZE];
	bool trained;
	uint8_t rate;
	uint8_t lanes;
	uint8_t preemp_level[4];
	uint8_t vswing_level[4];
};

/**
 * struct dptx_aux - The aux state
 * @sts: The AUXSTS register contents.
 * @data: The AUX data register contents.
 * @event: Indicates an AUX event ocurred.
 * @abort: Indicates that the AUX transfer should be aborted.
 */
struct dptx_aux {
	uint32_t sts;
	uint32_t data[4];
	atomic_t event;
	atomic_t abort;
};

struct sdp_header {
	uint8_t HB0;
	uint8_t HB1;
	uint8_t HB2;
	uint8_t HB3;
} __packed;

struct sdp_full_data {
	uint8_t en;
	uint32_t payload[9];
	uint8_t blanking;
	uint8_t cont;
} __packed;

struct hdr_infoframe {
	uint8_t type_code;
	uint8_t version_number;
	uint8_t length;
	uint8_t data[HDR_INFOFRAME_LENGTH];
};

#if defined(CONFIG_DP_HDCP)
enum _master_hdcp_op_type_ {
	DSS_HDCP13_ENABLE = 1,
	DSS_HDCP22_ENABLE,
	DSS_HDCP13_ENCRYPT_ENABLE,
	DSS_HDCP_OBS_SET,
	DSS_HDCP_INT_CLR,
	DSS_HDCP_INT_MASK,
	DSS_HDCP_CP_IRQ,
	DSS_HDCP_DPC_SEC_EN,
	DSS_HDCP_ENC_MODE_EN,
#ifdef SUPPORT_HISI_DPTX
	DSS_HDCP_DP_ENABLE,
#endif
	HDCP_OP_SECURITY_MAX,
};
#endif

enum audio_sample_freq {
	SAMPLE_FREQ_32 = 0,
	SAMPLE_FREQ_44_1 = 1,
	SAMPLE_FREQ_48 = 2,
	SAMPLE_FREQ_88_2 = 3,
	SAMPLE_FREQ_96 = 4,
	SAMPLE_FREQ_176_4 = 5,
	SAMPLE_FREQ_192 = 6
};

struct audio_short_desc {
	uint8_t max_num_of_channels;
	enum audio_sample_freq max_sampling_freq;
	uint8_t max_bit_per_sample;
};

struct audio_params {
	uint8_t iec_channel_numcl0;
	uint8_t iec_channel_numcr0;
	uint8_t use_lut;
	uint8_t iec_samp_freq;
	uint8_t iec_word_length;
	uint8_t iec_orig_samp_freq;
	uint8_t data_width;
	uint8_t num_channels;
	uint8_t inf_type;
	uint8_t mute;
	uint8_t ats_ver;
};

struct dtd {
	uint64_t pixel_clock;
	uint16_t pixel_repetition_input;
	uint16_t h_active;
	uint16_t h_blanking;
	uint16_t h_image_size;
	uint16_t h_sync_offset;
	uint16_t h_sync_pulse_width;
	uint8_t h_sync_polarity;
	uint16_t v_active;
	uint16_t v_blanking;
	uint16_t v_image_size;
	uint16_t v_sync_offset;
	uint16_t v_sync_pulse_width;
	uint8_t v_sync_polarity;
	uint8_t interlaced; /* 1 for interlaced, 0 progressive */
};

struct dp_dsc_info {
	uint32_t lsteer_xmit_delay;
	uint32_t wait_cnt_int;
	uint32_t wait_cnt_frac;
	uint32_t encoders;
	uint32_t h_active_new;
	struct dsc_info dsc_info;
};

struct video_params {
	enum pixel_enc_type pix_enc;
	enum pattern_mode pattern_mode;
	struct dtd mdtd;
	uint8_t mode;
	enum color_depth bpc;
	enum colorimetry_type colorimetry;
	enum dynamic_range_type dynamic_range;
	uint8_t aver_bytes_per_tu;
	uint8_t aver_bytes_per_tu_frac;
	uint8_t init_threshold;
	uint32_t refresh_rate;
	uint8_t video_format;
	uint8_t m_fps;

	struct dp_dsc_info dp_dsc_info;
};

/*edid info*/
struct timing_info {
	struct list_head list_node;

	uint8_t vSyncPolarity;
	uint8_t hSyncPolarity;

	uint64_t pixelClock;
	uint16_t hActivePixels;
	uint16_t hBlanking;
	uint16_t hSyncOffset;
	uint16_t hSyncPulseWidth;
	uint16_t hBorder;
	uint16_t hSize;
	uint16_t vActivePixels;
	uint16_t vBlanking;
	uint16_t vSyncOffset;
	uint16_t vSyncPulseWidth;
	uint16_t vBorder;
	uint16_t vSize;
	uint16_t inputType;
	uint16_t interlaced;
	uint16_t syncScheme;
	uint16_t schemeDetail;
};

struct ext_timing {
	uint16_t extFormatCode;
	uint16_t extHPixels;
	uint16_t extVPixels;
	uint16_t extVFreq;
};

struct edid_video {
	uint8_t mainVCount;
	uint8_t extVCount;

	uint16_t maxHPixels;
	uint16_t maxVPixels;
	uint16_t maxHImageSize;
	uint16_t maxVImageSize;
	uint16_t maxVFreq;
	uint16_t minVFreq;
	uint16_t maxHFreq;
	uint16_t minHFreq;
	uint16_t maxPixelClock;

	struct list_head *dptx_timing_list;
	struct ext_timing *extTiming;
	char *dp_monitor_descriptor;
};

struct edid_audio_info {
	uint16_t format;
	uint16_t channels;
	uint16_t sampling;
	uint16_t bitrate;
};

struct edid_audio {
	struct edid_audio_info *spec;
	uint8_t basicAudio;
	uint8_t extSpeaker;
	uint8_t extACount;
};

struct edid_information {
	struct edid_video Video;
	struct edid_audio Audio;
};

/**
 * struct dp_ctrl - The representation of the DP TX core
 * @mutex:
 * @base: Base address of the registers
 * @irq: IRQ number
 * @version: Contents of the IP_VERSION register
 * @max_rate: The maximum rate that the controller supports
 * @max_lanes: The maximum lane count that the controller supports
 * @dev: The struct device
 * @root: The debugfs root
 * @regset: The debugfs regset
 * @vparams: The video params to use
 * @aparams: The audio params to use
 * @waitq: The waitq
 * @shutdown: Signals that the driver is shutting down and that all
 *            operations should be aborted.
 * @c_connect: Signals that a HOT_PLUG or HOT_UNPLUG has occurred.
 * @sink_request: Signals the a HPD_IRQ has occurred.
 * @rx_caps: The sink's receiver capabilities.
 * @edid: The sink's EDID.
 * @aux: AUX channel state for performing an AUX transfer.
 * @link: The current link state.
 * @multipixel: Controls multipixel configuration. 0-Single, 1-Dual, 2-Quad.
 */
struct dp_ctrl {
	struct mutex dptx_mutex; /* generic mutex for dptx */

	struct {
		uint8_t multipixel;
		uint8_t streams;
		bool gen2phy;
		bool dsc;
	} hwparams;

	void __iomem *base;
	uint32_t irq;

	enum dp_ctrl_type ctrl_type;
	uint32_t version;
	uint8_t bstatus;
	uint8_t streams;
	uint8_t multipixel;
	uint8_t max_rate;
	uint8_t max_lanes;
	uint8_t dsc_decoders;
	uint8_t dsc_ifbc_type;

	/*mst*/
	uint8_t rad_port;
	uint8_t port[2];
	uint16_t pbn;
	int active_mst_links;

	int vcp_id;

	bool mst;
	bool fec;
	bool dsc;
	bool ycbcr420;
	bool cr_fail;
	bool ssc_en;
	bool efm_en;
	bool need_rad;
	bool logic_port;

	bool dummy_dtds_present;

#if defined(CONFIG_VR_DISPLAY)
	bool dptx_vr;
#endif
	bool dptx_gate;
	bool dptx_enable;
	bool dptx_plug_type;
	bool dptx_detect_inited;
	bool same_source;
	bool video_transfer_enable;

	struct device *dev;
#if CONFIG_DP_ENABLE
	struct switch_dev sdev;
	struct switch_dev dp_switch;
#endif
	struct dpu_fb_data_type *dpufd;

	struct video_params vparams;
	struct audio_params aparams;
	struct audio_short_desc audio_desc;
	struct hdr_metadata hdr_metadata;
	struct hdr_infoframe hdr_infoframe;

	struct edid_information edid_info;
	struct drm_dp_mst_topology_mgr mst_mgr;

	wait_queue_head_t dptxq;
	wait_queue_head_t waitq;

	atomic_t shutdown;
	atomic_t c_connect;
	atomic_t sink_request;

	uint8_t rx_caps[DPTX_RECEIVER_CAP_SIZE];

	uint8_t *edid;
	uint8_t *edid_second;
	uint32_t edid_try_count;
	uint32_t edid_try_delay; /* unit: ms */

	struct sdp_full_data sdp_list[DPTX_SDP_NUM];
	struct dptx_aux aux;
	struct dptx_link link;

	uint8_t detect_times;
	uint8_t current_link_rate;
	uint8_t current_link_lanes;
	uint32_t hpd_state;
	uint32_t user_mode;
	uint32_t combophy_pree_swing[DPTX_COMBOPHY_PARAM_NUM];
#ifdef SUPPORT_HISI_DPTX
	uint32_t combophy_ssc_ppm[DPTX_COMBOPHY_SSC_PPM_PARAM_NUM];
#endif
	uint32_t max_edid_timing_hactive;
	enum video_format_type user_mode_format;
	enum established_timings selected_est_timing;

	int sysfs_index;
	struct attribute *sysfs_attrs[DP_SYSFS_ATTRS_NUM];
	struct attribute_group sysfs_attr_group;

	struct hrtimer dptx_hrtimer;
	struct workqueue_struct *dptx_check_wq;
	struct work_struct dptx_check_work;

	/*
	 * aux read/write
	 */
	int (*aux_rw)(struct dp_ctrl *dptx, bool rw, bool i2c, bool mot,
		bool addr_only, uint32_t addr, uint8_t *bytes, uint32_t len);

	/*
	 * irq
	 */
	irqreturn_t (*dptx_irq)(int irq, void *dev);
	irqreturn_t (*dptx_threaded_irq)(int irq, void *dev);
	void (*dptx_hpd_handler)(struct dp_ctrl *dptx, bool plugin, uint8_t dp_lanes);
	void (*dptx_hpd_irq_handler)(struct dp_ctrl *dptx);

	/*
	 * link
	 */
	int (*handle_hotplug)(struct dpu_fb_data_type *dpufd);
	int (*handle_hotunplug)(struct dpu_fb_data_type *dpufd);
	int (*dptx_power_handler)(struct dp_ctrl *dptx, bool ublank, bool *bpresspowerkey);

	/*
	 * dsc
	 */
	int (*dptx_slice_height_limit)(struct dp_ctrl *dptx, uint32_t pic_height);
	int (*dptx_line_buffer_depth_limit)(uint8_t line_buf_depth);
	void (*dptx_dsc_check_rx_cap)(struct dp_ctrl *dptx);
	void (*dptx_dsc_para_init)(struct dp_ctrl *dptx);
	void (*dptx_dsc_sdp_manul_send)(struct dp_ctrl *dptx, bool enable);
	void (*dptx_dsc_cfg)(struct dp_ctrl *dptx);

	/*
	 * link training
	 */
	int (*dptx_link_set_lane_status)(struct dp_ctrl *dptx); /* set lane num/rate/ssc ... */
	void (*dptx_link_set_lane_after_training)(struct dp_ctrl *dptx);
	void (*dptx_phy_set_pattern)(struct dp_ctrl *dptx, uint32_t pattern);
	void (*dptx_link_set_preemp_vswing)(struct dp_ctrl *dptx);

	/*
	 * core config
	 */
	int (*dp_dis_reset)(struct dpu_fb_data_type *dpufd, bool benable);
	int (*dptx_core_on)(struct dp_ctrl *dptx);
	void (*dptx_core_off)(struct dpu_fb_data_type *dpufd, struct dp_ctrl *dptx);
	void (*dptx_core_remove)(struct dp_ctrl *dptx);
	void (*dptx_free_lanes)(struct dp_ctrl *dptx);
	void (*dptx_link_core_reset)(struct dp_ctrl *dptx);
	void (*dptx_default_params_from_core)(struct dp_ctrl *dptx); /* get default params decided by controller */
	bool (*dptx_sink_device_connected)(struct dp_ctrl *dptx);

	/*
	 * hdr infoframe
	 */
	void (*dptx_hdr_infoframe_set_reg)(struct dp_ctrl *dptx, uint8_t enable);

	/*
	 * stream config
	 */
	void (*dptx_video_core_config)(struct dp_ctrl *dptx, int stream); /* only video param config */
	void (*dptx_video_ts_change)(struct dp_ctrl *dptx, int stream); /* tu change config */
	void (*dptx_sdp_config)(struct dp_ctrl *dptx, int stream, enum dptx_sdp_config_type sdp_type);
	int (*dptx_video_ts_calculate)(struct dp_ctrl *dptx, int lane_num, int rate,
		int bpc, int encoding, int pixel_clock);
	/* include video/tu config and feature config like dsc */
	int (*dptx_link_timing_config)(struct dpu_fb_data_type *dpufd, struct dp_ctrl *dptx);
	/* disable stream when hotunplug */
	void (*dptx_link_close_stream)(struct dpu_fb_data_type *dpufd, struct dp_ctrl *dptx);
	/* enable/disable fec */
	int (*dptx_fec_enable)(struct dp_ctrl *dptx, bool fec_enable);

	/*
	 * audio config
	 */
	void (*dptx_audio_config)(struct dp_ctrl *dptx);

	/*
	 * media enable/disable
	 */
	void (*dptx_enable_default_video_stream)(struct dp_ctrl *dptx, int stream); /* stream enable */
	void (*dptx_disable_default_video_stream)(struct dp_ctrl *dptx, int stream); /* stream disable */
	/* enable/disable ldi and audio channel */
	int (*dptx_triger_media_transfer)(struct dp_ctrl *dptx, bool benable);
	int (*dptx_resolution_switch)(struct dpu_fb_data_type *dpufd, enum dptx_hot_plug_type etype);
	/* enable/disable phy transmitter on per lane */
	void (*dptx_phy_enable_xmit)(struct dp_ctrl *dptx, uint32_t lane, bool enable);

	/*
	 * phy config
	 */
	void (*dptx_combophy_set_preemphasis_swing)(struct dp_ctrl *dptx,
		uint32_t lane, uint32_t sw_level, uint32_t pe_level);
	int (*dptx_change_physetting_hbr2)(struct dp_ctrl *dptx); /* V510 Hardware patch */
	void (*dptx_combophy_set_rate)(struct dp_ctrl *dptx, int rate);
	/* V510 Hardware patch, phy set before and after set ssc */
	void (*dptx_combophy_set_ssc_addtions)(uint8_t link_rate, bool before);
	void (*dptx_combophy_set_ssc_dis)(struct dp_ctrl *dptx, bool ssc_disable);
	void (*dptx_aux_disreset)(struct dp_ctrl *dptx, bool enable);
	void (*dptx_combophy_set_lanes_power)(bool bopen);

	/*
	 * for test
	 */
	void (*dptx_audio_num_ch_change)(struct dp_ctrl *dptx);
	void (*dptx_video_timing_change)(struct dp_ctrl *dptx, int stream);
	void (*dptx_video_bpc_change)(struct dp_ctrl *dptx, int stream);
	void (*dptx_audio_infoframe_sdp_send)(struct dp_ctrl *dptx);
	int (*handle_test_link_phy_pattern)(struct dp_ctrl *dptx);
	void (*dptx_phy_set_ssc)(struct dp_ctrl *dptx, bool bswitchphy);
};

static inline uint32_t dptx_readl(struct dp_ctrl *dp, uint32_t offset)
{
	uint32_t data = readl(dp->base + offset);
	return data;
}

static inline void dptx_writel(struct dp_ctrl *dp, uint32_t offset, uint32_t data)
{
	writel(data, dp->base + offset);
}

/*
 * Wait functions
 */
#define dptx_wait(_dptx, _cond, _timeout)				\
	({								\
		int __retval;						\
		__retval = wait_event_interruptible_timeout(		\
			_dptx->waitq,					\
			((_cond) || (atomic_read(&_dptx->shutdown))),	\
			msecs_to_jiffies(_timeout));			\
		if (atomic_read(&_dptx->shutdown)) {			\
			__retval = -ESHUTDOWN;				\
		}							\
		else if (!__retval) {					\
			__retval = -ETIMEDOUT;				\
		}							\
		__retval;						\
	})

bool dptx_check_low_temperature(struct dp_ctrl *dptx);
void dptx_notify(struct dp_ctrl *dptx);
void dptx_notify_shutdown(struct dp_ctrl *dptx);
void dp_send_cable_notification(struct dp_ctrl *dptx, int val);
int hisi_dptx_switch_source(uint32_t user_mode, uint32_t user_format);

int dptx_phy_rate_to_bw(uint8_t rate);
int dptx_bw_to_phy_rate(uint8_t bw);
void dptx_audio_params_reset(struct audio_params *aparams);
void dptx_video_params_reset(struct video_params *params);
void dwc_dptx_dtd_reset(struct dtd *mdtd);

struct dpu_fb_data_type;
int hisi_dp_hpd_register(struct dpu_fb_data_type *dpufd);
void hisi_dp_hpd_unregister(struct dpu_fb_data_type *dpufd);

bool dp_get_dptx_feature_status(struct dp_ctrl *dptx);


#endif  /* HISI_DP_H */

