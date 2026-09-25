/*
 * hi64xx_dsp_interface.h
 *
 * codecdsp interface info
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

#ifndef __HI64XX_DSP_INTERFACE_H__
#define __HI64XX_DSP_INTERFACE_H__

#define HIFI_AUDIO_PCM_COMP_COEF_LEN_MAX 128
#define HOOK_PATH_MAX_LENGTH 200
#define CODECDSP_FW_NAME_MAX_LENGTH 64
#define VIRTUAL_BTN_DAY_SECOND (24 * 60 * 60)
#define VIRTUAL_BTN_MSG_NUM 3
#define VIRTUAL_BTN_PRESS_UP_FLAG 3
#define VIRTUAL_BTN_DUMP_MSG_MAX_LENGTH 1000
#define VIRTUAL_BTN_TERM_MAX_LENGTH 40
#define VIRTUAL_BTN_STR_MAX_LENGTH 22
#define VIRTUAL_BTN_STR_END_MAX_LENGTH 5
#define ANC_DFT_INFO_DETAILS_LEN 64
#define DSM_DFT_ERRINFO_LEN 96

enum hook_pos {
	/* codec dspif pos begin */
	HOOK_POS_IF0               = 0x00000001u,
	HOOK_POS_IF1               = 0x00000002u,
	HOOK_POS_IF2               = 0x00000004u,
	HOOK_POS_IF3               = 0x00000008u,
	HOOK_POS_IF4               = 0x00000010u,
	HOOK_POS_IF5               = 0x00000020u,
	HOOK_POS_IF6               = 0x00000040u,
	HOOK_POS_IF7               = 0x00000080u,
	HOOK_POS_IF8               = 0x00000100u,
	/* codec dspif pos end */

	/* codec dsp inner data pos begin */
	HOOK_POS_LOG               = 0x00000200u,
	HOOK_POS_ANC_CORE          = 0x00000400u,
	HOOK_POS_ANC_PCM_RX_VOICE  = 0x00000800u,
	HOOK_POS_ANC_PCM_REF       = 0x00001000u,
	HOOK_POS_ANC_PCM_ERR       = 0x00002000u,
	HOOK_POS_ANC_PCM_ANTI      = 0x00004000u,
	HOOK_POS_ANC_PCM_TX_VOICE  = 0x00008000u,
	HOOK_POS_ANC_ALG_INDICATE  = 0x00010000u,
	HOOK_POS_ANC_COEF          = 0x00020000u,
	HOOK_POS_PA_EFFECTIN       = 0x00040000u,
	HOOK_POS_PA_EFFECTOUT      = 0x00080000u,
	HOOK_POS_WAKEUP_MICIN      = 0x00100000u,
	HOOK_POS_WAKEUP_EFFECTOUT  = 0x00200000u,
	HOOK_POS_PA1_I             = 0x00400000u,
	HOOK_POS_PA1_V             = 0x00800000u,
	HOOK_POS_PA2_I             = 0x01000000u,
	HOOK_POS_PA2_V             = 0x02000000u,
	HOOK_POS_ANC_BETA_CSINFO   = 0x04000000u,
	HOOK_POS_ULTRASONIC        = 0x08000000u,
	/* codec dsp inner data pos end */

	HOOK_POS_BUTT,
	HOOK_POS_NONE_TEST         = 0x10000000u,
};

enum {
	OM_BANDWIDTH_6144 = 0,
	OM_BANDWIDTH_12288,
	OM_BANDWIDTH_BUTT,
};

enum {
	OM_SPONSOR_DEFAULT = 0,
	OM_SPONSOR_BETA,
	OM_SPONSOR_TOOL,
	OM_SPONSOR_BUTT,
};

enum  {
	ANC_START_HOOK      = 0xAA00,
	ANC_STOP_HOOK       = 0xAA01,
	ANC_TRIGGER_DFT     = 0xAA02,
	DSM_DUAL_STATIC0    = 0xAA03,
	DSM_DUAL_STATIC1    = 0xAA04,
	DSM_MONO_STATIC0    = 0xAA05,
	DSM_PARAM           = 0xAA06,
	PA_BUFFER_REVERSE   = 0xAA07,
	VIRTUAL_BTN_MONITOR = 0xAA08,
	WAKEUP_SUSPEND      = 0xAA09,
	SAVE_DSP_LOG        = 0xAA0A,
	WAKEUP_ERR_MSG      = 0xAA0B,
	OM_BUTT
};

enum {
	ANC_MODULE = 0,
	SMARTPA_MODULE,
};

enum error_class {
	NO_ERR                = 0,
	HOWLING_ERR           = 1,
	WIND_NOISE_ERR        = 2,
	COEF_DIVERGENCE_ERR   = 3,
	LOW_ACTTIME_RATE      = 4,
	PROCESS_PATH_ERR      = 5,
	OTD_ERR               = 6,
};

enum {
	DSM_OM_ERR_TYPE_PROC = 1, // imonitor required this start from 1
	DSM_OM_ERR_TYPE_PARA_SET,
	DSM_OM_ERR_TYPE_MALLOC,
	DSM_OM_ERR_TYPE_STATUS
};

enum error_level {
	ERR_LEVEL_WARNING = 0,
	ERR_LEVEL_ERROR,
};

enum dsp_power_state_enum {
	HIFI_POWER_ON = 0xA0,
	HIFI_POWER_CLK_ON = 0xB0,
	HIFI_POWER_CLK_OFF,
};

enum mlib_path_enum {
	MLIB_PATH_DATA_HOOK = 0,
	MLIB_PATH_DEFAULT,
	MLIB_PATH_WAKEUP,
	MLIB_PATH_SMARTPA,
	MLIB_PATH_ANC,
	MLIB_PATH_ANC_DEBUG,
	MLIB_PATH_IR_LEARN,
	MLIB_PATH_IR_TRANS,
	MLIB_PATH_VIRTUAL_BTN,
	MLIB_PATH_ULTRASONIC,
	MLIB_PATH_BUTT
};

enum mlib_module_enum {
	MLIB_MODULE_DEFAULT = 0,
	MLIB_MODULE_IVW,
	MLIB_MODULE_SWU,
	MLIB_MODULE_DSM,
	MLIB_MODULE_ELVIS,
	MLIB_MODULE_TSLICE,
	MLIB_MODULE_OEM,
	MLIB_MODULE_GOOGLE,
	MLIB_MODULE_DSM_STEREO,
	MLIB_MODULE_ANC,
	MLIB_MODULE_TFADSP,
	MLIB_MODULE_TFADSP_STEREO,
	MLIB_MODULE_TAS,
	MLIB_MODULE_TAS_STEREO,
	MLIB_MODULE_PA_BYPASS,
	MLIB_MODULE_CR_4PA_WITH_DSP,
	MLIB_MODULE_VIRTUAL_BTN,
	MLIB_MODULE_4PA_TDM_AUDIO_LOOP,
	MLIB_MODULE_ULTRA_CALIB,
	MLIB_MODULE_ULTRA_DUMP,
	MLIB_MODULE_BUTT,

	MLIB_MODULE_BROADCAST   = 0xFFFFFFFF,
};

enum apdsp_msg_enum {
	ID_AP_DSP_IF_OPEN          = 0xDD10,
	ID_AP_DSP_IF_CLOSE         = 0xDD11,
	ID_AP_DSP_PARAMETER_SET    = 0xDD12,
	ID_AP_DSP_PARAMETER_GET    = 0xDD13,
	ID_AP_DSP_OM               = 0xDD14,

	ID_AP_DSP_IF_ST_OPEN       = 0xDD15,
	ID_AP_DSP_IF_ST_CLOSE      = 0xDD16,

	ID_AP_DSP_FASTTRANS_OPEN   = 0XDD17,
	ID_AP_DSP_FASTTRANS_CLOSE  = 0XDD18,
	ID_AP_DSP_WAKEUP_TEST      = 0XDD19,

	ID_DSP_UPDATE_BUF          = 0xDD20,
	ID_DSP_LOG_HOOK            = 0xDD21,
	ID_DSP_UPDATE_FASTBUF      = 0XDD22,
	ID_DSP_UPDATE_RECORDBUF    = 0XDD23,
	ID_AP_DSP_MADTEST_START    = 0xDD31,
	ID_AP_DSP_MADTEST_STOP     = 0xDD32,
	ID_AP_DSP_UARTMODE         = 0xDD33,
	ID_AP_DSP_FASTMODE         = 0xDD34,
	ID_AP_DSP_ANCTEST_START    = 0xDD37,
	ID_AP_DSP_ANCTEST_STOP     = 0xDD38,
	ID_AP_DSP_ANCTEST_STRESS   = 0xDD39,
	ID_AP_IMGAE_DOWNLOAD       = 0xDD40,
	ID_AP_DSP_PLL_SWITCH       = 0xDD41,
	ID_DSP_WAKE_UP_MAD_ISR     = 0xDD42,

	ID_AP_DSP_HOOK_START       = 0xDD62,
	ID_AP_DSP_HOOK_STOP        = 0xDD63,
	ID_AP_DSP_SET_OM_BW        = 0xDD64,
	ID_AP_AP_SET_HOOK_SPONSOR  = 0xDD65,
	ID_AP_AP_SET_HOOK_PATH     = 0xDD66,
	ID_AP_AP_SET_DIR_COUNT     = 0xDD67,

	ID_AP_DSP_DEBUG_MSG        = 0xDD74,
	ID_AP_DSP_OCRAM_TCM_CHECK  = 0xDD75,
	ID_AP_AP_SET_WAKEUP_HOOK   = 0xDD76,
	ID_FAULTINJECT             = 0xDDA0,
};

enum uart_mode_enum {
	UART_MODE_115200 = 0, /* for pc uart */
	UART_MODE_500K,       /* for phone in mad mode */
	UART_MODE_6M,         /* for phone in normal mode */
	UART_MODE_OFF,        /* force writing log to 36K om buffer */
};

enum ir_para_id_enum {
	IR_PARA_ID_CODE      = 0,
	IR_PARA_ID_FREQUENCY,
	IR_PARA_ID_STATUS,
};

enum virtual_btn_algo_enum {
	VIRTBTN_CNT_SOFT_LOG,
	VIRTBTN_FORCE_MAX_GRAM,
	VIRTBTN_TIMER_PZT_FORCE,
	VIRTBTN_FORCE_MAX,
	VIRTBTN_TIMER_ULTRA_PRESS,
	VIRTBTN_CAL_ULT_MIN,
	VIRTBTN_FITTING_LINEARITY,
	VIRTBTN_TIMER_ULTRA_UP,
	VIRTBTN_ULT_BASELINE_APPROX,
	VIRTBTN_PRESS_DOWN,
	VIRTBTN_PRESS_UP,
	VIRTBTN_ULT_NOT_DET,
	VIRTBTN_PZT_ULT_SLEEP,
	VIRTBTN_DATA_ULT_BASELINE,
	VIRTBTN_SLOPE_INTERCEPT,
	VIRTBTN_FREQ_SCAN,
	VIRTBTN_ALGO_BUTT,
};

enum virtual_btn_integrate_enum {
	VIRTBTN_BUTTON_NUM,
	VIRTBTN_PASSIVE_NUM,
	VIRTBTN_ACTIVE_NUM,
	VIRTBTN_INTEGRATE_BUTT,
};

/* om control */
enum pcm_hook_point_enum {
	TRACK_LOG_NONE                 = 0x00000000,
	TRACK_LOG_MAINMIC              = 0x00000001,
	TRACK_LOG_SUBMIC               = 0x00000002,
	TRACK_LOG_THIRDMIC             = 0x00000004,
	TRACK_LOG_FORTHMIC             = 0x00000008,
	TRACK_LOG_UPLINK_AFTERPRO      = 0x00000010,
	TRACK_LOG_UPLINK_AFTERCODE     = 0x00000020,
	TRACK_LOG_DOWNLINK_AFTERPRO    = 0x00000100,
	TRACK_LOG_DOWNLINK_AFTERCODE   = 0x00000200,
	TRACK_LOG_DOWNLINK_BEFORECODE  = 0x00000400,
	TRACK_LOG_SMARTPA_IN           = 0x00001000,
	TRACK_LOG_SMARTPA_OUT          = 0x00002000,
	TRACK_LOG_SMARTPA_I            = 0x00004000,
	TRACK_LOG_SMARTPA_V            = 0x00008000,
	TRACK_LOG_AEC                  = 0x00010000,
	TRACK_LOG_SOUNDTRIGGER_KEYWORD = 0x00020000,
	TRACK_LOG_UPDATA_LEFT          = 0X00100000,
	TRACK_LOG_UPDATA_RIGHT         = 0X00200000,
	TRACK_LOG_ALGORITHM_FAILURE    = 0x00400000,
};

enum wakeup_hook_pcm_status {
	WAKEUP_HOOK_PCM_START,
	WAKEUP_HOOK_PCM_STOP,
};

enum check_memory_type {
	CHECK_MEMORY_NORMALLY,
	CHECK_MEMORY_BY_MARCH_IC,
	CHECK_TYPE_BUTT,
};

enum hook_type_enum {
	HOOK_OFF = 0,
	HOOK_LOG,
	HOOK_LOG_PCM,
};

enum hook_force_output_log_enum {
	HOOK_DEFAULT = 0,
	HOOK_FORCE_OUTPUT_LOG,
};

enum wake_up_err_type{
	WAKEUP_LOADING_ERR = 0,
	WAKEUP_INIT_ALG_ERR,
	WAKEUP_ISR_ERR,
	WAKEUP_PERFORMANCE_INSUFFICIENT,
	WAKEUP_ERR_TYPE_BUTT,
};

struct vitrual_bnt_data_log {
	unsigned short data_log[VIRTUAL_BTN_MSG_NUM][VIRTBTN_ALGO_BUTT];
	unsigned short btn_num;
	unsigned short passive_num;
	unsigned short active_num;
};

struct om_hook_para {
	enum hook_pos pos;
	unsigned short resolution;
	unsigned short channel_num;
	unsigned int sample_rate;
};

struct om_set_dir_count_msg {
	unsigned short msg_id;
	unsigned int count;
};

struct om_set_hook_sponsor_msg {
	unsigned short msg_id;
	unsigned short sponsor;
};

struct om_set_hook_path_msg {
	unsigned short msg_id;
	char hook_path[HOOK_PATH_MAX_LENGTH];
	unsigned int size;
};

struct om_start_hook_msg {
	unsigned short msg_id;
	unsigned short para_size;
};

struct om_stop_hook_msg {
	unsigned short msg_id;
	unsigned short reserve;
};

struct om_set_bandwidth_msg {
	unsigned short msg_id;
	unsigned short bw;
};

struct om_anc_stress_msg {
	unsigned short msg_id;
	unsigned short stress_id;
};

struct pa_buffer_reverse_msg {
	unsigned int pa_count;
	unsigned int proc_interval;
};

struct mlib_anc_dft_info {
	unsigned int err_module; // ANC or smartPa
	unsigned int err_class;  // err classification
	unsigned int err_level;  // err serious level
	unsigned char details[ANC_DFT_INFO_DETAILS_LEN]; // upload information
};

struct mlib_dsm_dft_info {
	unsigned int msg_size;
	unsigned int err_module;
	unsigned int err_class;
	unsigned int err_level;
	int err_code;
	unsigned int err_line_num;
	unsigned char err_info[DSM_DFT_ERRINFO_LEN];
};

struct fast_trans_msg_stru {
	unsigned short  msg_id;
	unsigned short  fast_trans_enable;
};

struct pcm_if_msg_stru {
	unsigned int if_id;
	unsigned int if_direct;
	unsigned int sample_ratein;
	unsigned int sample_rateout;
	unsigned int channel_count;
	unsigned int dma_set_id;
	unsigned int uc_resolution;
};

struct pcm_process_dma_msg_stru {
	unsigned int process_id;
	unsigned int if_count;
	unsigned int module_id;
	struct pcm_if_msg_stru if_cfg_list[0];
};

struct dsp_if_open_req_stru {
	unsigned short msg_id;  /* ID_AP_DSP_IF_OPEN 0xDD10 */
	unsigned short perms;
	struct pcm_process_dma_msg_stru process_dma;
};

struct fast_trans_cfg_req_stru {
	unsigned short msg_id;  /* ID_AP_DSP_FASTTRANS_CONFIG 0xDD17 */
	unsigned short reserve;
	int status;
};

/* mlib para control */
struct mlib_para_set_req_stru {
	unsigned short msg_id;
	unsigned short reserve;
	unsigned int   process_id;
	unsigned int   module_id;
	unsigned int   size;
	unsigned char  auc_data[0];
};

struct mlib_para_get_req_stru {
	unsigned short msg_id;
	unsigned short reserve;
	unsigned int   process_id;
	unsigned int   module_id;
	unsigned int   size;
	unsigned char  auc_data[0];
};

struct mlib_para_get_data {
	int result;
	unsigned char para_data[0];
};

struct om_ctrl_block_stru {
	unsigned short msg_id;
	unsigned short reserve;
	unsigned int   hook_type;
	unsigned int   output_mode;
	unsigned int   port_num;
	unsigned int   force_output_log;
	unsigned int   hook_points;
	unsigned int   log_mode;
	unsigned int   log_level;
};

struct fw_download_stru {
	unsigned short msg_id;
	unsigned short reserve;
	char           chw_name[CODECDSP_FW_NAME_MAX_LENGTH];
};

struct mad_test_stru {
	unsigned short msg_id;
	unsigned short reserve;
};

struct anc_test_stru {
	unsigned short msg_id;
	unsigned short reserve;
};

struct uartmode_stru {
	unsigned short msg_id;
	unsigned short reserve;
	unsigned int   uart_mode; /* 0--115200, 1--500K, 2--6M */
};

struct wakeup_hook_msg {
	unsigned short msg_id;
	unsigned short reserve;
	enum wakeup_hook_pcm_status status;
};

struct fault_inject {
	unsigned short msg_id;
	unsigned short reserve;
	unsigned int   fault_type;
};

struct sync_msg_no_params {
	unsigned short msg_id;
	unsigned short reserve;
};

struct wake_up_err_msg {
	unsigned int type;
	unsigned int content_in;
	unsigned int content_cmp;
};

#endif /* end of hi64xx_dsp_interface.h */

