/*
 * dsp misc.c
 *
 * head of dsp misc.c
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#ifndef __DSP_MISC_H__
#define __DSP_MISC_H__

#include <linux/list.h>
#include "global_ddr_map.h"
#include "soc_acpu_baseaddr_interface.h"

/* mailbox mail_len max */
#define MAIL_LEN_MAX 512

#ifndef OK
#define OK    0
#endif

#define ERROR  (-1)
#define BUSY  (-2)
#define NOMEM  (-3)
#define INVAILD  (-4)
#define COPYFAIL (-0xFFF)

#define NVPARAM_COUNT        600         /* HIFI NV size is 600 */
#define NVPARAM_NUMBER       258         /* 256+2, nv_data(256) + nv_id(2) */
#define NVPARAM_START        2           /* head protect_number 0x5a5a5a5a */
#define NVPARAM_TAIL         2           /* tail protect_number 0x5a5a5a5a */
#define NVPARAM_TOTAL_SIZE   ((NVPARAM_NUMBER * NVPARAM_COUNT + NVPARAM_START + NVPARAM_TAIL) * sizeof(unsigned short))

#define DSP_UNSEC_BASE_ADDR   (HISI_RESERVED_HIFI_DATA_PHYMEM_BASE)
#define ASP_FAMA_PHY_ADDR_DIFF 0

#ifdef CONFIG_HIFI_IPC_3660
#define DSP_UNSEC_REGION_SIZE              (HISI_RESERVED_HIFI_DATA_PHYMEM_SIZE)
#define DSP_MUSIC_DATA_SIZE                (0x132000 + 0x32000)
#ifdef CONFIG_HIFI_MEMORY_15M
#define PCM_PLAY_BUFF_SIZE                  0x6F000
#else
#define PCM_PLAY_BUFF_SIZE                 (0x200000 - 0x32000)
#endif
#else
#define DSP_UNSEC_REGION_SIZE              0x380000
#define DSP_MUSIC_DATA_SIZE                0x132000
#define PCM_PLAY_BUFF_SIZE                 0x100000
#endif
#define PCM_PLAY_BUFF_USED_SIZE            0x100000
#define DSP_AP_NV_DATA_SIZE                0x4BC00
#define DRV_DSP_UART_TO_MEM_SIZE           0x7f000
#define DRV_DSP_UART_TO_MEM_RESERVE_SIZE   0x100
#define DRV_DSP_STACK_TO_MEM_SIZE          0x1000
#define DSP_ICC_DEBUG_SIZE                 0x11000
#define DSP_SOCP_BUFFER_SIZE               0x2000
#define DSP_FLAG_DATA_SIZE                 0x1000
#define DSP_SEC_HEAD_SIZE                  0x1000

#define DSP_AP_MAILBOX_TOTAL_SIZE          0x10000
#define CODEC_DSP_OM_DMA_BUFFER_SIZE       0xF000
#define CODEC_DSP_OM_DMA_CONFIG_SIZE       0x80
#define CODEC_DSP_SOUNDTRIGGER_TOTAL_SIZE  0xF000
#define DSP_PCM_UPLOAD_BUFFER_SIZE         0x2000
#define DSP_PARA_DOWNLOAD_BUFFER_SIZE      0x2000
#define DSP_AUDIO_EFFECT_PARAM_BUFF_SIZE   0xC000
#ifdef CONFIG_HIFI_MEMORY_15M
#define HIFI_USB_DRIVER_SHARE_MEM_SIZE     0x0
#else
#define HIFI_USB_DRIVER_SHARE_MEM_SIZE     0x20000
#endif
#ifdef CONFIG_HIFI_IPC_3660
#ifdef CONFIG_HIFI_MEMORY_15M
#define AUDIO_DP_DATA_BUFF_SIZE       0x0
#else
#define AUDIO_DP_DATA_BUFF_SIZE       0x40000
#endif
#else
#define AUDIO_DP_DATA_BUFF_SIZE       0x0
#endif
#define AP_AUDIO_PA_BUFF_SIZE              0x4000
#ifdef CONFIG_DSP_VAD_WAKEUP
#define AP_AUDIO_WAKEUP_RINGBUFEER_SIZE    0x1D000
#define AP_AUDIO_WAKEUP_CAPTURE_SIZE       0x3000
#else
#define AP_AUDIO_WAKEUP_RINGBUFEER_SIZE    0x1E000
#define AP_AUDIO_WAKEUP_CAPTURE_SIZE       0x2000
#endif
#define AP_AUDIO_WAKEUP_MODEL_SIZE         0x1000
#define AP_AUDIO_LP_WAKEUP_RINGBUFEER_SIZE AP_AUDIO_WAKEUP_RINGBUFEER_SIZE
#define AP_AUDIO_LP_WAKEUP_CAPTURE_SIZE    AP_AUDIO_WAKEUP_CAPTURE_SIZE
#define DSP_UNSEC_RESERVE_SIZE                  0x48C00
#define DSP_PCM_THREAD_DATA_SIZE                0x48
#define DRV_DSP_SCREEN_STATUS_SIZE              4
#ifdef HISI_EXTERNAL_MODEM
#define AP_B5000_NV_SIZE  0x32c00
#define AP_OM_BUFFER_SIZE 0x50000
#define AP_B5000_MSG_SIZE 0x200
#else
#define AP_B5000_NV_SIZE  0x0
#define AP_OM_BUFFER_SIZE 0x0
#endif
#define AP_VIRTUAL_CALL_DOWNLINK_SIZE  0x1000
#define AP_VIRTUAL_CALL_UPLINK_SIZE    0x1000

#ifdef CONFIG_HIFI_SUPPORT_VIBRATOR
#define AP_AUDIO_VIBRATOR_DATA_SIZE (0xF000)
#else
#define AP_AUDIO_VIBRATOR_DATA_SIZE (0x0)
#endif

#define DSP_SOCP_BUFFER_SIZE 0x2000

#define DSP_MUSIC_DATA_LOCATION        (DSP_UNSEC_BASE_ADDR)
#define PCM_PLAY_BUFF_LOCATION         (DSP_MUSIC_DATA_LOCATION + DSP_MUSIC_DATA_SIZE)
#define PCM_OM_BUFF_LOCATION           (PCM_PLAY_BUFF_LOCATION + PCM_PLAY_BUFF_USED_SIZE)
#define DRV_DSP_UART_TO_MEM            (PCM_PLAY_BUFF_LOCATION + PCM_PLAY_BUFF_SIZE)
#define DRV_DSP_STACK_TO_MEM           (DRV_DSP_UART_TO_MEM + DRV_DSP_UART_TO_MEM_SIZE)
#define DSP_ICC_DEBUG_LOCATION         (DRV_DSP_STACK_TO_MEM + DRV_DSP_STACK_TO_MEM_SIZE)
#define DSP_SOCP_BUFFER_ADDR           (DSP_ICC_DEBUG_LOCATION + DSP_ICC_DEBUG_SIZE)
#define DSP_FLAG_DATA_ADDR             (DSP_SOCP_BUFFER_ADDR + DSP_SOCP_BUFFER_SIZE)
#define DSP_SEC_HEAD_BACKUP            (DSP_FLAG_DATA_ADDR + DSP_FLAG_DATA_SIZE)
#define DSP_AP_NV_DATA_ADDR            (DSP_SEC_HEAD_BACKUP + DSP_SEC_HEAD_SIZE)
#define DSP_AP_MAILBOX_BASE_ADDR       (DSP_AP_NV_DATA_ADDR + DSP_AP_NV_DATA_SIZE)
#define CODEC_DSP_OM_DMA_BUFFER_ADDR    (DSP_AP_MAILBOX_BASE_ADDR + DSP_AP_MAILBOX_TOTAL_SIZE)
#define CODEC_DSP_OM_DMA_CONFIG_ADDR    (CODEC_DSP_OM_DMA_BUFFER_ADDR + CODEC_DSP_OM_DMA_BUFFER_SIZE)
#define CODEC_DSP_SOUNDTRIGGER_BASE_ADDR (CODEC_DSP_OM_DMA_CONFIG_ADDR + CODEC_DSP_OM_DMA_CONFIG_SIZE)
#define DSP_PCM_UPLOAD_BUFFER_ADDR      (CODEC_DSP_SOUNDTRIGGER_BASE_ADDR + CODEC_DSP_SOUNDTRIGGER_TOTAL_SIZE)
#define DSP_AUDIO_EFFECT_PARAM_ADDR     (DSP_PCM_UPLOAD_BUFFER_ADDR + DSP_PCM_UPLOAD_BUFFER_SIZE)
#define HIFI_USB_DRIVER_SHARE_MEM_ADDR  (DSP_AUDIO_EFFECT_PARAM_ADDR + DSP_AUDIO_EFFECT_PARAM_BUFF_SIZE)
#define AUDIO_DP_DATA_LOCATION     (HIFI_USB_DRIVER_SHARE_MEM_ADDR + HIFI_USB_DRIVER_SHARE_MEM_SIZE)
#define AP_AUDIO_PA_ADDR           (AUDIO_DP_DATA_LOCATION + AUDIO_DP_DATA_BUFF_SIZE)
#define AP_AUDIO_WAKEUP_RINGBUFFER_ADDR (AP_AUDIO_PA_ADDR + AP_AUDIO_PA_BUFF_SIZE)
#define AP_AUDIO_WAKEUP_CAPTURE_ADDR    (AP_AUDIO_WAKEUP_RINGBUFFER_ADDR + AP_AUDIO_WAKEUP_RINGBUFEER_SIZE)
#define AP_AUDIO_WAKEUP_MODEL_ADDR         (AP_AUDIO_WAKEUP_CAPTURE_ADDR + AP_AUDIO_WAKEUP_CAPTURE_SIZE)
#define AP_AUDIO_LP_WAKEUP_RINGBUFFER_ADDR (AP_AUDIO_WAKEUP_MODEL_ADDR + AP_AUDIO_WAKEUP_MODEL_SIZE)
#define AP_AUDIO_LP_WAKEUP_CAPTURE_ADDR    (AP_AUDIO_LP_WAKEUP_RINGBUFFER_ADDR + AP_AUDIO_LP_WAKEUP_RINGBUFEER_SIZE)
#define AP_B5000_NV_ADDR   (AP_AUDIO_LP_WAKEUP_CAPTURE_ADDR + AP_AUDIO_LP_WAKEUP_CAPTURE_SIZE)
#define AP_OM_BUFFER_ADDR  (AP_B5000_NV_ADDR + AP_B5000_NV_SIZE)
#define AP_VIRTUAL_CALL_DOWNLINK_ADDR (AP_OM_BUFFER_ADDR + AP_OM_BUFFER_SIZE)
#define AP_VIRTUAL_CALL_UPLINK_ADDR   (AP_VIRTUAL_CALL_DOWNLINK_ADDR + AP_VIRTUAL_CALL_DOWNLINK_SIZE)
#define AP_AUDIO_VIBRATOR_ADDR (AP_VIRTUAL_CALL_UPLINK_ADDR + AP_VIRTUAL_CALL_UPLINK_SIZE)
#define HIFI_WIRED_HEADSET_PARA_ADDR (AP_AUDIO_VIBRATOR_ADDR + AP_AUDIO_VIBRATOR_DATA_SIZE)
#define DSP_UNSEC_RESERVE_ADDR (HIFI_WIRED_HEADSET_PARA_ADDR + DSP_PARA_DOWNLOAD_BUFFER_SIZE)

#define DSP_OM_LOG_SIZE                0xA000
#define DSP_OM_LOG_ADDR                (DRV_DSP_UART_TO_MEM - DSP_OM_LOG_SIZE)
#define DSP_DUMP_BIN_SIZE              (DSP_AP_MAILBOX_BASE_ADDR + DSP_AP_MAILBOX_TOTAL_SIZE - DSP_OM_LOG_ADDR)
#define DSP_DUMP_BIN_ADDR              (DSP_OM_LOG_ADDR)

#define DRV_DSP_PANIC_MARK              (DSP_FLAG_DATA_ADDR)
#define DRV_DSP_UART_LOG_LEVEL          (DRV_DSP_PANIC_MARK + 4)
#define DRV_DSP_UART_TO_MEM_CUR_ADDR    (DRV_DSP_UART_LOG_LEVEL + 4)
#define DRV_DSP_EXCEPTION_NO            (DRV_DSP_UART_TO_MEM_CUR_ADDR + 4)
#define DRV_DSP_IDLE_COUNT_ADDR         (DRV_DSP_EXCEPTION_NO + 4)
#define DRV_DSP_LOADED_INDICATE         (DRV_DSP_IDLE_COUNT_ADDR + 4)
#define DRV_DSP_KILLME_ADDR             (DRV_DSP_LOADED_INDICATE + 4)
#define DRV_DSP_WRITE_MEM_PRINT_ADDR    (DRV_DSP_KILLME_ADDR + 4)
#define DRV_DSP_POWER_STATUS_ADDR       (DRV_DSP_WRITE_MEM_PRINT_ADDR + 4)
#define DRV_DSP_NMI_FLAG_ADDR           (DRV_DSP_POWER_STATUS_ADDR + 4)
#define DRV_DSP_SOCP_FAMA_CONFIG_ADDR   (DRV_DSP_NMI_FLAG_ADDR + 4)
#define DRV_DSP_FLAG_ALP_NOC_CHECK      (DRV_DSP_SOCP_FAMA_CONFIG_ADDR + sizeof(struct drv_fama_config))
#define DRV_DSP_SLT_FLAG_ADDR           (DRV_DSP_FLAG_ALP_NOC_CHECK + 4)
#define DRV_DSP_PCM_THREAD_DATA_ADDR    (DRV_DSP_SLT_FLAG_ADDR + 4)
#define DRV_DSP_SCREEN_STATUS           (DRV_DSP_PCM_THREAD_DATA_ADDR + DSP_PCM_THREAD_DATA_SIZE)
#define DRV_DSP_FLAG_DATA_RESERVED      (DRV_DSP_SCREEN_STATUS + DRV_DSP_SCREEN_STATUS_SIZE)

#define DRV_DSP_POWER_ON                0x55AA55AA
#define DRV_DSP_POWER_OFF               0x55FF55FF
#define DRV_DSP_KILLME_VALUE            0xA5A55A5A
#define DRV_DSP_NMI_COMPLETE            0xB5B5B5B5
#define DRV_DSP_NMI_INIT                0xA5A5A5A5
#define DRV_DSP_SOCP_FAMA_HEAD_MAGIC    0x5A5A5A5A
#define DRV_DSP_SOCP_FAMA_REAR_MAGIC    0xA5A5A5A5
#define DRV_DSP_FAMA_ON     0x1
#define DRV_DSP_FAMA_OFF    0x0

#define DSP_IMAGE_OCRAMBAK_SIZE        0x30000
#define DSP_SEC_HEAD_SIZE              0x1000
#ifdef CONFIG_HIFI_IPC_3660
#define DSP_SEC_REGION_SIZE            (HISI_RESERVED_HIFI_PHYMEM_SIZE)
#ifdef CONFIG_HIFI_MEMORY_21M
#define DSP_RUN_SIZE                   0xF00000
#elif defined(CONFIG_2MIC_LP_WAKEUP)
#define DSP_RUN_SIZE                   0xD00000
#else
#define DSP_RUN_SIZE                   0xB00000
#endif
#define DSP_IMAGE_TCMBAK_SIZE          0x34000

#define DSP_RUN_ITCM_SIZE              0x9000
#define DSP_RUN_DTCM_SIZE              0x28000

#define DSP_SEC_BASE_ADDR              (HISI_RESERVED_HIFI_PHYMEM_BASE)

#define DSP_RUN_LOCATION               (DSP_SEC_BASE_ADDR)
#define DSP_IMAGE_OCRAMBAK_LOCATION    (DSP_RUN_LOCATION + DSP_RUN_SIZE)
#define DSP_IMAGE_TCMBAK_LOCATION      (DSP_IMAGE_OCRAMBAK_LOCATION + DSP_IMAGE_OCRAMBAK_SIZE)
#define DSP_SEC_HEAD_LOCATION          (DSP_IMAGE_TCMBAK_LOCATION + DSP_IMAGE_TCMBAK_SIZE)
#define DSP_SEC_RESERVE_ADDR           (DSP_SEC_HEAD_LOCATION + DSP_SEC_HEAD_SIZE)

#define DRV_WATCHDOG_BASE_ADDR          (SOC_ACPU_ASP_WD_BASE_ADDR)
#else
#define DSP_SEC_REGION_SIZE            0xB80000
#define DSP_RUN_SIZE                   0xB00000
#define DSP_IMAGE_TCMBAK_SIZE          0x1E000
#define DSP_SEC_RESERVE_SIZE           0x31000

#define DSP_RUN_ITCM_SIZE              0x6000
#define DSP_RUN_DTCM_SIZE              0x18000

#define DSP_SEC_BASE_ADDR              (DSP_UNSEC_BASE_ADDR + DSP_UNSEC_REGION_SIZE)
#define DSP_IMAGE_OCRAMBAK_LOCATION    (DSP_SEC_BASE_ADDR)
#define DSP_IMAGE_TCMBAK_LOCATION      (DSP_IMAGE_OCRAMBAK_LOCATION + DSP_IMAGE_OCRAMBAK_SIZE)
#define DSP_SEC_HEAD_LOCATION          (DSP_IMAGE_TCMBAK_LOCATION + DSP_IMAGE_TCMBAK_SIZE)
#define DSP_SEC_RESERVE_ADDR           (DSP_SEC_HEAD_LOCATION + DSP_SEC_HEAD_SIZE)
#define DSP_RUN_LOCATION               (DSP_SEC_RESERVE_ADDR + DSP_SEC_RESERVE_SIZE)

#define DRV_WATCHDOG_BASE_ADDR          (SOC_ACPU_ASP_Watchdog_BASE_ADDR)
#endif

#define SIZE_PARAM_PRIV       206408 /* refer from function dsp_nv_init in dsp_soc_para_ctl.c */
#define HIFI_SYS_MEM_ADDR     (DSP_RUN_LOCATION)
#define SYS_TIME_STAMP_REG    (SOC_ACPU_SCTRL_BASE_ADDR + 0x534)

#define SIZE_CMD_ID 8

/* notice buffer for reporting data once */
#define REV_MSG_NOTICE_ID_MAX 2

#define ACPU_TO_HIFI_ASYNC_CMD 0xFFFFFFFF

#define BUFFER_NUM 8
#define MAX_NODE_COUNT 10

#define SYSCACHE_QUOTA_SIZE_MAX   0x100000
#define SYSCACHE_QUOTA_SIZE_ALIGN 0x40000
#define CMD_FUNC_NAME_LEN         50
#define RETRY_COUNT               3

enum dsp_msg_id {
	/* DTS command id from ap */
	ID_AP_AUDIO_SET_DTS_ENABLE_CMD  = 0xDD36,
	ID_AP_AUDIO_SET_DTS_DEV_CMD     = 0xDD38,
	ID_AP_AUDIO_SET_DTS_GEQ_CMD     = 0xDD39,
	ID_AP_AUDIO_SET_AUDIO_EFFECT_PARAM_CMD = 0xDD3A,
	ID_AP_AUDIO_SET_DTS_GEQ_ENABLE_CMD     = 0xDD3B,
	ID_AP_AUDIO_SET_EXCODEC_ENABLE_CMD     = 0xDD3D,

	/* Voice Record */
	ID_AP_DSP_VOICE_RECORD_START_CMD = 0xDD40,
	ID_AP_DSP_VOICE_RECORD_STOP_CMD  = 0xDD41,

	/* voicePP MSG_ID */
	ID_AP_VOICEPP_START_REQ           = 0xDD42, /* start VOICEPP */
	ID_VOICEPP_MSG_START              = ID_AP_VOICEPP_START_REQ,
	ID_VOICEPP_AP_START_CNF           = 0xDD43,
	ID_AP_VOICEPP_STOP_REQ            = 0xDD44, /* stop VOICEPP */
	ID_VOICEPP_AP_STOP_CNF            = 0xDD45,
	ID_VOICEPP_MSG_END                = 0xDD4A,

	ID_AP_AUDIO_PLAY_START_REQ        = 0xDD51, /* AP start up dsp audio player request */
	ID_AUDIO_AP_PLAY_START_CNF        = 0xDD52, /* dsp start up audio player call back AP confirm */
	ID_AP_AUDIO_PLAY_PAUSE_REQ        = 0xDD53, /* AP stop dsp audio player request */
	ID_AUDIO_AP_PLAY_PAUSE_CNF        = 0xDD54, /* dsp stop audio player call back AP confirm */
	ID_AUDIO_AP_PLAY_DONE_IND         = 0xDD56, /* dsp notifies AP audio player data playback or interrupt indication */
	ID_AP_AUDIO_PLAY_UPDATE_BUF_CMD   = 0xDD57, /* AP notifies dsp sync data command */
	ID_AP_AUDIO_PLAY_QUERY_TIME_REQ   = 0xDD59, /* AP search dsp audio player play progress request */
	ID_AP_AUDIO_PLAY_WAKEUPTHREAD_REQ = 0xDD5A,
	ID_AUDIO_AP_PLAY_QUERY_TIME_CNF   = 0xDD60, /* dsp reply AP audio player play progress confirm */
	ID_AP_AUDIO_PLAY_QUERY_STATUS_REQ = 0xDD61, /* AP search dsp audio player play state request */
	ID_AUDIO_AP_PLAY_QUERY_STATUS_CNF = 0xDD62, /* dsp reply AP audio player play state confirm */
	ID_AP_AUDIO_PLAY_SEEK_REQ         = 0xDD63, /* AP seek dsp audio player anywhere request */
	ID_AUDIO_AP_PLAY_SEEK_CNF         = 0xDD64, /* dsp reply AP seek result confirm */
	ID_AP_AUDIO_PLAY_SET_VOL_CMD      = 0xDD70, /* AP setting the volume */
	ID_AP_AUDIO_RECORD_PCM_HOOK_CMD   = 0xDD7A, /* AP notifies dsp catch PCM data */
	ID_AUDIO_AP_UPDATE_PCM_BUFF_CMD      = 0xDD7C,
	ID_AP_AUDIO_DYN_EFFECT_GET_PARAM     = 0xDD7D,
	ID_AP_AUDIO_DYN_EFFECT_GET_PARAM_CNF = 0xDD7E,
	ID_AP_AUDIO_DYN_EFFECT_TRIGGER       = 0xDD7F,
	/* enhance msgid between ap and hifi */
	ID_AP_DSP_ENHANCE_START_REQ  = 0xDD81,
	ID_DSP_AP_ENHANCE_START_CNF  = 0xDD82,
	ID_AP_DSP_ENHANCE_STOP_REQ   = 0xDD83,
	ID_DSP_AP_ENHANCE_STOP_CNF   = 0xDD84,
	ID_AP_DSP_ENHANCE_SET_DEVICE_REQ = 0xDD85,
	ID_DSP_AP_ENHANCE_SET_DEVICE_CNF = 0xDD86,

	/* audio enhance msgid between ap and hifi */
	ID_AP_AUDIO_ENHANCE_SET_DEVICE_IND = 0xDD91,
	ID_AP_AUDIO_MLIB_SET_PARA_IND      = 0xDD92,
	ID_AP_AUDIO_CMD_SET_SOURCE_CMD     = 0xDD95,
	ID_AP_AUDIO_CMD_SET_DEVICE_CMD     = 0xDD96,
	ID_AP_AUDIO_CMD_SET_MODE_CMD       = 0xDD97,
	ID_AP_AUDIO_CMD_SET_ANGLE_CMD      = 0xDD99,
	ID_AP_AUDIO_CMD_SET_TYPEC_DEVICE_MASK_CMD = 0xDD9D,

	/* for 3mic */
	ID_AP_AUDIO_ROUTING_COMPLETE_REQ   = 0xDDC0, /* AP notifies dsp 3Mic/4Mic path was build*/
	ID_AUDIO_AP_DP_CLK_EN_IND          = 0xDDC1, /* dsp notifies AP open/close Codec DP clk */
	ID_AP_AUDIO_DP_CLK_STATE_IND       = 0xDDC2, /* AP notifies dsp £¬Codec DP clk state */
	ID_AUDIO_AP_OM_DUMP_CMD            = 0xDDC3, /* dsp notifies AP dump log */
	ID_AUDIO_AP_FADE_OUT_REQ           = 0xDDC4, /* dsp notifies AP fadeout */
	ID_AP_AUDIO_FADE_OUT_IND           = 0xDDC5, /* AP notifies dsp fadeout completed */

	ID_AUDIO_AP_OM_CMD                 = 0xDDC9,
	ID_AP_AUDIO_STR_CMD                = 0xDDCB, /* AP send string to dsp,resolve in dsp */
	ID_AUDIO_AP_VOICE_BSD_PARAM_CMD    = 0xDDCC, /* VOICE BSD reported patameter */

	ID_AP_ENABLE_MODEM_LOOP_REQ        = 0xDDCD, /* the audio hal notify dsp to start/stop  MODEM LOOP*/
	ID_AUDIO_AP_3A_CMD                 = 0xDDCE,
	ID_AP_ENABLE_AT_DSP_LOOP_REQ       = 0xDDCF, /* notify dsp to start/stop dsp LOOP from slimbus to i2s */
	ID_DSP_AP_HOOK_DATA_CMD            = 0xDDD1, /* dsp send pcm data to ap for om */
	ID_AP_DSP_REQUEST_VOICE_PARA_REQ   = 0xDF00, /* ap request voice msg */
	ID_DSP_AP_REQUEST_VOICE_PARA_CNF   = 0xDF01, /* dsp replay voice msg */
	ID_AP_DSP_REQUEST_SET_PARA_CMD     = 0xDF08, /* dsp set param msg */
	ID_AP_DSP_REQUEST_GET_PARA_CMD     = 0xDF09, /* dsp get param msg */
	ID_AP_DSP_REQUEST_GET_PARA_CNF     = 0xDF0A, /* dsp get param msg */
	ID_AP_DSP_SET_FM_CMD               = 0xDF0F,
	ID_DSP_AP_BIGDATA_CMD              = 0xDF10, /* bigdata */
	ID_DSP_AP_SMARTPA_DFT              = 0xDF11,
	ID_DSP_AP_AUDIO_DB                 = 0xDF12,
	ID_AP_AUDIO_CMD_SET_COMBINE_RECORD_FUNC_CMD = 0xDF22,/* hal notify dsp combine record cmd */
	ID_AP_SOCDSP_MADTEST_START         = 0xDF23, /* mmi mad test start cmd */
	ID_AP_SOCDSP_MADTEST_STOP          = 0xDF24, /* mmi mad test stop cmd */
	ID_DSP_AP_SYSCACHE_QUOTA_CMD       = 0xDF32, /* syscache quota MSG */
#ifdef HISI_EXTERNAL_MODEM
	ID_AP_DSP_NV_REFRESH_IND  = 0xDF33,
	ID_DSP_AP_OM_DATA_IND     = 0xDF34,
	ID_DSP_AP_PCIE_REOPEN_IND = 0xDF35,
	ID_DSP_AP_PCIE_CLOSE_IND  = 0xDF37,
#endif
	ID_AP_DSP_I2S_TEST_POWER_REQ = 0xDF38, /* AP notifies dsp to power on for i2s test */
	ID_DSP_AP_I2S_TEST_POWER_CNF = 0xDF39, /* dsp returns to the AP to power off the result */
	ID_DSP_AP_AUXHEAR_CMD = 0xDF44,
	ID_AP_DSP_SET_SCENE_DENOISE_CMD = 0xDF54,
	ID_AP_HIFI_SET_VALUE_METER_SWITCH_CMD = 0xDF58, /* ap hal set hearing protection switch */
	ID_AP_HIFI_SET_WIRED_HEADSET_PARA_CMD = 0xDF59, /* ap hal set wired headset para */
	ID_AP_HIFI_GET_VALUE_DATA_CMD = 0xDF60, /* ap hal get hearing protection volume data info */
	ID_HIFI_AP_GET_VALUE_DATA_CNF = 0xDF61, /* ap hal get hearing protection volume data feedback */
};

enum dsp_platform_type {
	DSP_PLATFORM_ASIC,
	DSP_PLATFORM_FPGA
};

struct rev_msg_buff {
	unsigned char *mail_buff;
	unsigned int mail_buff_len;
	unsigned int cmd_id;         /* 4 byte */
	unsigned char *out_buff_ptr; /* point to behind of cmd_id */
	unsigned int out_buff_len;
};

struct recv_request {
	struct list_head recv_node;
	struct rev_msg_buff rev_msg;
};

struct misc_recmsg_param {
	unsigned short msg_id;
	unsigned short play_status;
};

struct common_dsp_cmd {
	unsigned short msg_id;
	unsigned short reserve;
	unsigned int   value;
};

struct audio_vote_ddr_freq_stru {
	unsigned short down_cpu_utilization; /* DDR down cpu occupancy level */
	unsigned short up_cpu_utilization;   /* DDR up cpu occupancy level */
	unsigned short ddr_freq;             /* setup ddr freq */
	unsigned short is_vote_ddr;          /* is current vote DDR */
};

struct audio_cpu_load_cfg_stru {
	unsigned short enable_vote_ddr;    /* 0: only print cpu occupancy rate not vote */
	unsigned short ddr_freq_count;     /* ddr FM length */
	unsigned short check_interval;     /* check occupancy rate interval, ms*/
	unsigned short report_interval;    /* report occupancy rate interval, s */
	struct audio_vote_ddr_freq_stru *pst_vote_ddr_freq; /* length is ddr_freq_count */
};

struct drv_fama_config {
	unsigned int head_magic;
	unsigned int flag;
	unsigned int rear_magic;
};

enum usbaudio_ioctl_type {
	USBAUDIO_QUERY_INFO = 0,
	USBAUDIO_USB_POWER_RESUME,
	USBAUDIO_NV_ISREADY,
	USBAUDIO_MSG_MAX
};

struct usbaudio_ioctl_input {
	unsigned int msg_type;
	unsigned int input1;
	unsigned int input2;
};

#ifdef HISI_EXTERNAL_MODEM
struct hifi_ap_om_data_notify {
	unsigned int chunk_index;
	unsigned int len;
};
#endif

enum syscache_quota_type {
	SYSCACHE_QUOTA_RELEASE = 0,
	SYSCACHE_QUOTA_REQUEST
};

enum syscache_session {
	SYSCACHE_SESSION_AUDIO = 0,
	SYSCACHE_SESSION_VOICE,
	SYSCACHE_SESSION_CNT
};

struct syscache_quota_msg {
	unsigned int msg_type;
	unsigned int session;
};

typedef int (*cmd_proc_func)(uintptr_t arg);

struct dsp_ioctl_cmd {
	unsigned int id;
	cmd_proc_func func;
	char name[CMD_FUNC_NAME_LEN];
};

struct dsp_misc_proc {
	const struct dsp_ioctl_cmd *cmd_table;
	cmd_proc_func sync_msg_proc;
	unsigned int size;
};

long dsp_msg_process_cmd(unsigned int cmd, uintptr_t data32);
int dsp_send_msg(unsigned int mailcode, const void *data, unsigned int length);
#ifdef ENABLE_AUDIO_KCOV
void dsp_misc_msg_process(void *cmd);
#endif
void dsp_get_log_signal(void);
void dsp_release_log_signal(void);
void dsp_watchdog_send_event(void);
unsigned long try_copy_from_user(void *to, const void __user *from, unsigned long n);
unsigned long try_copy_to_user(void __user *to, const void *from, unsigned long n);
#ifdef HISI_EXTERNAL_MODEM
unsigned char *get_dsp_viradr(void);
#endif
enum dsp_platform_type dsp_misc_get_platform_type(void);
void dsp_reset_release_syscache(void);

#endif /* __DSP_MISC_H__ */

