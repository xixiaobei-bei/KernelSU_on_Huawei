/*
 * type.h
 *
 * type define
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

#ifndef __DA_COMBINE_V3_TYPE_H__
#define __DA_COMBINE_V3_TYPE_H__

#include <sound/soc.h>

#include <slimbus.h>

#include "linux/hisi/da_combine/da_combine_resmgr.h"
#include "linux/hisi/da_combine/da_combine_mbhc.h"

#define LOG_TAG "DA_combine_v3"

#define FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | \
			SNDRV_PCM_FMTBIT_S16_BE | \
			SNDRV_PCM_FMTBIT_S24_LE | \
			SNDRV_PCM_FMTBIT_S24_BE)

#define RATES SNDRV_PCM_RATE_8000_384000

/* res chain num */
#define IMP_RES_NUM           8
/* supersonic wave range 100mV */
#define IMP_RANGE_VALUE       100
/* fake res value 1000omh */
#define FAKE_RES_VALUE        1000
/* feedback res value 625omh */
#define FB_RES_VALUE          625
/* current ratio 32 */
#define CURRENT_RATIO_VALUE   32

#define HSD_CFG_DEFAULT_VALUE 0x37
#define HS_RES_DEGAULT_VALUE  32


/* HS 0.5dB PGA GAIN */
#define RES_THRESHOLD         100
#define HS_LOW_0_GAIN         0x3D    //-29.5dB
#define HS_LOW_1_GAIN         0x47    //-24.5dB
#define HS_HIGH_0_GAIN        0x6E    //-5dB
#define HS_HIGH_1_GAIN        0x78    //0dB

#define FS_8000		0x0
#define FS_16000	0x1
#define FS_32000	0x2
#define FS_48000	0x4
#define FS_96000	0x5

#define BT_TRI_BT                    0
#define BT_TRI_PA                    1
#define BT_TRI_INVALID               2

#ifdef CONFIG_HAC_SUPPORT
/*hac status*/
#define HAC_ENABLE                   1
#define HAC_DISABLE                  0
#endif

#ifdef CONFIG_RCV_TDD_SUPPORT
#define RCV_TDD_ENABLE               1
#define RCV_TDD_DISABLE              0
#endif

enum hs_det_func {
	CMP_FUNC = 0,
	SCHMIT_FUNC = 1,
};

enum track_state {
	TRACK_FREE = 0,
	TRACK_STARTUP = 1,
};

enum classh_state {
	HP_CLASSH_STATE  = 0x1u,  /* hp high mode(classAB) = 0 low mode(classh) = 1 */
	RCV_CLASSH_STATE = 0x2u,  /* classh_rcv_hp_switch true = 1 false =0 */
	HP_POWER_STATE   = 0x4u,  /* hp power on = 1 power off = 0 */
	RCV_POWER_STATE  = 0x8u,  /* rcv power on = 1 power off = 0 */
};

enum mic_type {
	MAIN_MIC  = 0,
	AUX_MIC  = 1,
	MIC3 = 2,
	MIC4 = 3,
};

union sc_adc_ctrl_reg {
	unsigned int value;
	struct {
		unsigned int r_din_sel : 4;
		unsigned int l_din_sel : 4;
	} reg;
};

struct classh_bit {
	unsigned char hp_classh : 1;
	unsigned char rcv_classh : 1;
	unsigned char hp_power : 1;
	unsigned char rcv_power : 1;
};

struct classh_bit_bind {
	unsigned char hp_rcv_classh : 2;
	unsigned char hp_rcv_power : 2;
};

union classh_bit_sel {
	unsigned char val;
	struct classh_bit classh;
	struct classh_bit_bind classh_bind;
};

enum hp_rcv_bind_state {
	HP_RCV_POWEROFF, /* rcv power on = 0 hp power on = 0 */
	HP_POWER_ONLY, /* rcv power on = 0 hp power on = 1 */
	RCV_POWER_ONLY, /* rcv power on = 1 hp power on = 0 */
	HP_RCV_POWEON, /* rcv power on = 1 hp power on = 1 */
};

struct board_cfg {
	/* board defination */
	int mic_num;
	unsigned int ir_gpio_id;
	unsigned int ear_ir_gpio_id;
	unsigned int hsd_cfg_value;
	bool use_stereo_smartpa;
	bool classh_rcv_hp_switch;
	bool hp_high_low_change_enable;
	bool hp_res_detect_enable;
	bool extern_hs_hifi_ak4376_I2S3;
	bool gpio_pd_enable;
	bool hsd_cfg_enable;
	int bt_tri_gpio;
	int hs_det_trigger_func;
#ifdef CONFIG_HAC_SUPPORT
	int hac_gpio;
#endif
	unsigned int pmu_mclk_addr;
	bool fm_enable;
	bool micbias_modify;
#ifdef CONFIG_RCV_TDD_SUPPORT
	int rcv_tdd_gpio;
#endif
	bool wakeup_audio_algo_support;
};

/* codec private data */
struct da_combine_v3_platform_data {
	struct snd_soc_component *codec;
	struct device_node *node;
	struct hi_cdc_ctrl *cdc_ctrl;
	struct da_combine_resmgr *resmgr;
	struct da_combine_irq *irqmgr;
	struct da_combine_mbhc *mbhc;
	struct da_combine_mbhc_config mbhc_config;
	struct board_cfg board_config;
	struct mutex rw_mutex;
	struct mutex impdet_dapm_mutex;
	unsigned int mm_reg;
	unsigned int virtul_dacl_gain;
	unsigned int virtul_dacr_gain;
	unsigned int res_value;
	unsigned int hs_high_pga_gain;
	unsigned int hs_low_pga_gain;
	struct slimbus_track_param voice_params;
	struct slimbus_track_param capture_params;
	struct slimbus_track_param play_params;
	struct slimbus_track_param soundtrigger_params;
	enum track_state voiceup_state;
	enum track_state audioup_4mic_state;
	int dp_clk_num;
	int cp1_num;
	int cp2_num;
	bool hsr_power_on;
	bool hsl_power_on;
	enum classh_state rcv_hp_classh_state;
	unsigned int mic_fixed_value[4];
#ifdef AUDIO_FACTORY_MODE
	int mainmicbias_val;
#endif
};

#endif

