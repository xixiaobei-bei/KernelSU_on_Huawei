/*
 * om_bigdata.c
 *
 * audio socdsp report current state
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#include "om_bigdata.h"

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/time.h>

#include "dsp_misc.h"
#include "audio_hifi.h"
#include "algorithm_dft.h"
#include "huawei_platform/log/imonitor.h"
#include "huawei_platform/log/imonitor_keys.h"
#include "drv_mailbox_msg.h"
#include "om_debug.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#define BLKMIC_SECONE     17
#define BLKMIC_SECTWO     32
#define BLKMIC_SECTHR     64
#define BLKMIC_SECFOUR    128
#define HIFI_AP_MESG_CNT  127
#define IMONITOR_UPDATIME 96400 /* seconds number of one day */
#define AUXHEAR_IMONITOR_UPDATIME 60
#define BIGDATA_VOICE_HSEVENTID   916200001
#define BIGDATA_VOICE_HFEVENTID   916200002
#define BIGDATA_VOICE_HESEVENTID  916200003
#define BIGDATA_VOICE_BTEVENTID   916200004
#define BIGDATA_AUXHEAR_EVENTID   916200009
#define BIGDATA_VOICE_BLOCKMIC    20
#define BIGDATA_VOIP_BLKEVENTID   916200101

#define VOICE_BIGDATA_NOISESIZE   16
#define VOICE_BIGDATA_VOICESIZE   16
#define VOICE_BIGDATA_CHARACTSIZE 32
#define VOICE_BIGDATA_NOISE_VOICE_SIZE  (VOICE_BIGDATA_NOISESIZE + VOICE_BIGDATA_VOICESIZE)

#define DSP_DUMP_MAX_EFFECTS_CNT  10
#define LOG_PATH_HIFI_REQUEST_LOG "/data/hisi_logs/running_trace/hifi_log/hifi_requset/"

#define VOICECHARACTER_DEVICE_MAX 3
#define BLOCKMIC_DEVICE_MAX       1

enum mlib_device {
	MLIB_DEVICE_HANDSET = 0,
	MLIB_DEVICE_HANDFREE,
	MLIB_DEVICE_CARFREE,
	MLIB_DEVICE_HEADSET,
	MLIB_DEVICE_BLUETOOTH,
	MLIB_DEVICE_PCVOICE,
	MLIB_DEVICE_HEADPHONE,
	MLIB_DEVICE_USBVOICE,
	MLIB_DEVICE_USBHEADSET,
	MLIB_DEVICE_USBHEADPHONE,
	MLIB_DEVICE_BUTT
};

enum micblk_info {
	MIC_BLK_TIMES = 0,
	HS_TOP_MIC_BLK_TIMES,
	HF_TOP_MIC_BLK_TIMES,
	HS_RIGHT_MIC_BLK_TIMES,
	HF_RIGHT_MIC_BLK_TIMES,
	HS_LEFT_MIC_BLK_TIMES,
	HF_LEFT_MIC_BLK_TIMES,
	HS_HEAD_MIC_BLK_TIMES,
	HF_HEAD_MIC_BLK_TIMES,
	HS_MIC_BLK_CHANGE_NVID,
	HF_MIC_BLK_CHANGE_NVID,
	MIC_ERR_TIMES,
	HS_MIC_DIS_CHANGE_NVID,
	HF_MIC_DIS_CHANGE_NVID,
	LEFT_MIC_DIS_TIMES,
	HEAD_MIC_DIS_TIMES,
	TOP_MIC_RMS,
	RIGHT_MIC_RMS,
	LEFT_MIC_RMS,
	HEAD_MIC_RMS,
	MIC_BLK_RATE,
	MIC_ERR_RATE,
	MIC_BLK_BUT
};

enum {
	HS_DEV = 0,
	HF_DEV = 1,
	MICBLK_DEV_BUTT
};

struct socdsp_effect_mcps {
	unsigned short stream_id;
	unsigned short algo_id;
	unsigned int   algo_mcps;
};

struct socdsp_cpu_load_info {
	unsigned int cpu_load;
	unsigned int avg_cpu_load;
	unsigned int ddr_freq;
};

struct socdsp_om_load_info {
	unsigned int recv_msg_type; /* CPU load reporting message type */
	struct socdsp_cpu_load_info cpu_load_info;
	unsigned int info_type;
	unsigned int report_interval;
};

struct socdsp_om_effect_mcps {
	unsigned int recv_msg_type;
	struct socdsp_cpu_load_info cpu_load_info;
	struct socdsp_effect_mcps effect_mcps_info[DSP_DUMP_MAX_EFFECTS_CNT];
};

struct socdsp_om_update_buff_delay_info {
	unsigned int recv_msg_type;
	unsigned short pcm_mode;
	unsigned short pcm_device;
};

struct socdsp_om_dump_log_info {
	unsigned int recv_msg_type;
	unsigned int value;
};

struct voice_3a_om {
	unsigned int   recv_msg_type;
	unsigned short recv_msg;
	unsigned short nvid;
	unsigned short data1;
	unsigned short data2;
};

typedef struct {
	unsigned int mic_blk_times;
	unsigned int top_mic_blk_times[MICBLK_DEV_BUTT];
	unsigned int right_mic_blk_times[MICBLK_DEV_BUTT];
	unsigned int left_mic_blk_times[MICBLK_DEV_BUTT];
	unsigned int head_mic_blk_times[MICBLK_DEV_BUTT];
	unsigned int mic_blk_change_nvid[MICBLK_DEV_BUTT];
	unsigned int mic_err_times;
	unsigned int hs_mic_dis_change_nvid;
	unsigned int hf_mic_dis_change_nvid;
	unsigned int left_mic_dis_times;
	unsigned int head_mic_dis_times;
	signed short top_mic_rms;
	signed short right_mic_rms;
	signed short left_mic_rms;
	signed short head_mic_rms;
	signed short mic_blk_rate;
	signed short mic_err_rate;
} voice_bsd_3a_micblk_stru;

typedef struct {
	unsigned int recv_msg_type;
	unsigned int msg_len;
	voice_bsd_3a_micblk_stru data;
} voip_micblk_3a_om;

struct imedia_voice_bigdata {
	unsigned char bigdata_device : 4;
	unsigned char bigdata_flag : 4;
	unsigned char bigdata_noise : 4;
	unsigned char bigdata_voice : 4;
	unsigned char bigdata_miccheck;
	unsigned char bigdata_reserve;
	unsigned int bigdata_charact;
};

struct imedia_auxhear_bigdata {
	char imedia_auxhear_mode;
	short imedia_auxhear_error;
	unsigned int imedia_auxhear_device;
};

struct imedia_voice_bigdata_to_imonitor {
	char noise[VOICE_BIGDATA_NOISESIZE];     /* 0-15 noise level */
	char voice[VOICE_BIGDATA_VOICESIZE];     /* 0-15 voice level */
	char charact[VOICE_BIGDATA_CHARACTSIZE]; /* 32 voice charact, such as whisper, ave and so on */
};

struct om_priv_bigdata {
	struct device *dev;
	bool is_inited;
};

struct socdsp_effect_info {
	unsigned int effect_id;
	char *effect_name;
};

static struct socdsp_effect_info effect_algo[] = {
	{ ID_EFFECT_ALGO_FORMATER, "FORMATER" },

	{ ID_EFFECT_ALGO_FORTE_VOICE_SPKOUT, "FORTE_VOICE_SPKOUT" },
	{ ID_EFFECT_ALGO_FORTE_VOICE_MICIN, "FORTE_VOICE_MICIN" },
	{ ID_EFFECT_ALGO_FORTE_VOICE_SPKOUT_BWE, "FORTE_VOICE_SPKOUT_BWE" },

	{ ID_EFFECT_ALGO_FORTE_VOIP_MICIN, "FORTE_VOIP_MICIN" },
	{ ID_EFFECT_ALGO_FORTE_VOIP_SPKOUT, "FORTE_VOIP_SPKOUT" },

	{ ID_EFFECT_ALGO_IN_CONVERT_I2S_GENERAL, "IN_CONVERT_I2S_GENERAL" },
	{ ID_EFFECT_ALGO_IN_CONVERT_I2S_HI363X, "IN_CONVERT_I2S_HI363X" },

	{ ID_EFFECT_ALGO_INTERLACE, "INTERLACE" },

	{ ID_EFFECT_ALGO_OUT_CONVERT_I2S_GENERAL, "OUT_CONVERT_I2S_GENERAL" },
	{ ID_EFFECT_ALGO_OUT_CONVERT_I2S_HI363X, "OUT_CONVERT_I2S_HI363X" },

	{ ID_EFFECT_ALGO_SWAP, "SWAP" },

	{ ID_EFFECT_ALGO_IMEDIA_WNR_MICIN, "IMEDIA_WNR_MICIN" },
	{ ID_EFFECT_ALGO_IMEDIA_WNR_SPKOUT, "IMEDIA_WNR_SPKOUT" },

	{ ID_EFFECT_ALGO_SWS_INTERFACE, "SWS_INTERFACE" },
	{ ID_EFFECT_ALGO_DTS, "DTS" },
	{ ID_EFFECT_ALGO_DRE, "DRE" },
	{ ID_EFFECT_ALGO_CHC, "CHC" },
	{ ID_EFFECT_ALGO_SRC, "SRC" },
	{ ID_EFFECT_ALGO_TTY, "TTY" },

	{ ID_EFFECT_ALGO_KARAOKE_RECORD, "KARAOKE_RECORD" },
	{ ID_EFFECT_ALGO_KARAOKE_PLAY, "KARAOKE_PLAY" },

	{ ID_EFFECT_ALGO_MLIB_CS_VOICE_CALL_MICIN, "MLIB_CS_VOICE_CALL_MICIN" },
	{ ID_EFFECT_ALGO_MLIB_CS_VOICE_CALL_SPKOUT, "MLIB_CS_VOICE_CALL_SPKOUT" },
	{ ID_EFFECT_ALGO_MLIB_VOIP_CALL_MICIN, "MLIB_VOIP_CALL_MICIN" },
	{ ID_EFFECT_ALGO_MLIB_VOIP_CALL_SPKOUT, "MLIB_VOIP_CALL_MICIN" },
	{ ID_EFFECT_ALGO_MLIB_AUDIO_PLAY, "MLIB_AUDIO_PLAY" },
	{ ID_EFFECT_ALGO_MLIB_AUDIO_RECORD, "MLIB_AUDIO_RECORD" },
	{ ID_EFFECT_ALGO_MLIB_SIRI_MICIN, "MLIB_SIRI_MICIN" },
	{ ID_EFFECT_ALGO_MLIB_SIRI_SPKOUT, "MLIB_SIRI_SPKOUT" },

	{ ID_EFFECT_ALGO_EQ, "EQ" },
	{ ID_EFFECT_ALGO_MBDRC6402, "MBDRC6402" },

	{ ID_EFFECT_ALGO_IMEDIA_VOIP_MICIN, "IMEDIA_VOIP_MICIN" },
	{ ID_EFFECT_ALGO_IMEDIA_VOIP_SPKOUT, "IMEDIA_VOIP_SPKOUT" },
	{ ID_EFFECT_ALGO_IMEDIA_VOICE_CALL_MICIN, "IMEDIA_VOICE_CALL_MICIN" },
	{ ID_EFFECT_ALGO_IMEDIA_VOICE_CALL_SPKOUT, "IMEDIA_VOICE_CALL_SPKOUT" },
	{ ID_EFFECT_ALGO_IMEDIA_VOICE_CALL_SPKOUT_BWE, "IMEDIA_VOICE_CALL_SPKOUT_BWE" },
};

static struct om_priv_bigdata g_priv_bigdata;

static int get_workqueue_data(enum socdsp_om_work_id work_id, unsigned char *data, unsigned int *len);
static unsigned int dsm_notify_limit = 0x10;

int socdsp_om_get_voice_bsd_param(uintptr_t uaddr)
{
	unsigned char data[MAIL_LEN_MAX] = {'\0'};
	unsigned int data_len = 0;
	struct voice_bsd_param_hsm param;
	int ret;

	memset(&param, 0, sizeof(param));
	if (try_copy_from_user(&param, (void __user *)uaddr, sizeof(param))) {
		loge("copy from user failed\n");
		return -EFAULT;
	}

	if (!param.pdata) {
		loge("user buffer is null\n");
		return -EINVAL;
	}

	ret = get_workqueue_data(SOCDSP_OM_WORK_VOICE_BSD, data, &data_len);
	if (ret != 0)
		return ret;

	if (param.data_len < data_len) {
		loge("userspace len: %u is less than data len: %u\n",
			param.data_len, data_len);
		return -EINVAL;
	}

	if (try_copy_to_user((void __user *)param.pdata, data, data_len)) {
		loge("copy to user failed\n");
		return -EFAULT;
	}
	logd("size: %u copy to user success\n", data_len);

	return 0;
}

static void socdsp_om_cpu_load_info_show(const struct socdsp_om_load_info *socdsp_om_info)
{
	switch (socdsp_om_info->info_type) {
	case SOCDSP_CPU_LOAD_VOTE_UP:
	case SOCDSP_CPU_LOAD_VOTE_DOWN:
		logi("cpu utilization: %u%%, vote ddr to %um\n",
			socdsp_om_info->cpu_load_info.cpu_load,
			socdsp_om_info->cpu_load_info.ddr_freq);
		break;

	case SOCDSP_CPU_LOAD_LACK_PERFORMANCE:
		logi("ddr freq: %um, cpu utilization: %u%%, lack of performance\n",
			socdsp_om_info->cpu_load_info.ddr_freq,
			socdsp_om_info->cpu_load_info.cpu_load);
#ifdef CONFIG_HUAWEI_DSM
		if (unlikely((dsm_notify_limit % 0x10) == 0))
			audio_dsm_report_info(AUDIO_CODEC, DSM_SOC_HIFI_HIGH_CPU,
				"DSM_SOC_HIFI_HIGH_CPU\n");
#endif
		dsm_notify_limit++;
		break;

	default:
		break;
	}
}

static void socdsp_om_effect_mcps_info_show(struct socdsp_om_effect_mcps *socdsp_mcps_info)
{
	unsigned int i = 0;
	unsigned short id;
	static const char * const map[] =  {
		[AUDIO_STREAM_PCM_OUTPUT]     = "pcm output",
		[AUDIO_STREAM_PLAYER_OUTPUT]  = "player output",
		[AUDIO_STREAM_MIXER_OUTPUT]   = "mixer output",
		[AUDIO_STREAM_VOICE_OUTPUT]   = "voice output",
		[AUDIO_STREAM_VOICEPP_OUTPUT] = "voicepp output",
		[AUDIO_STREAM_PCM_INPUT]      = "pcm input",
		[AUDIO_STREAM_VOICE_INPUT]    = "voice input",
		[AUDIO_STREAM_VOICEPP_INPUT]  = "voicepp input"
	};

	logi("ddr freq: %um, cpu utilization: %u%%\n", socdsp_mcps_info->cpu_load_info.ddr_freq,
		socdsp_mcps_info->cpu_load_info.cpu_load);
	id = socdsp_mcps_info->effect_mcps_info[i].algo_id;
	for (i = 0; i < ARRAY_SIZE(socdsp_mcps_info->effect_mcps_info); i++) {
		if (id < ID_EFFECT_ALGO_BUTT && id > ID_EFFECT_ALGO_START) {
			switch (socdsp_mcps_info->effect_mcps_info[i].stream_id) {
			case AUDIO_STREAM_PCM_OUTPUT:
			case AUDIO_STREAM_PLAYER_OUTPUT:
			case AUDIO_STREAM_MIXER_OUTPUT:
			case AUDIO_STREAM_VOICE_OUTPUT:
			case AUDIO_STREAM_VOICEPP_OUTPUT:
			case AUDIO_STREAM_PCM_INPUT:
			case AUDIO_STREAM_VOICE_INPUT:
			case AUDIO_STREAM_VOICEPP_INPUT:
				logi("algorithm: %s, mcps: %u, stream: %s\n",
					effect_algo[id - 1].effect_name,
					socdsp_mcps_info->effect_mcps_info[i].algo_mcps,
					map[socdsp_mcps_info->effect_mcps_info[i].stream_id]);
				break;
			default:
				break;
			}
		}
	}
}

static void socdsp_om_update_buff_delay_info_show(struct socdsp_om_update_buff_delay_info *info)
{
	logi("socdsp continuous update buff delay: mode: %u 0-play, 1-capture, device: %u 0-primary, 1-direct\n",
		info->pcm_mode, info->pcm_device);
}

static void socdsp_om_show_audio_detect_info(struct work_struct *work)
{
	unsigned char data[MAIL_LEN_MAX] = {'\0'};
	unsigned int data_len = 0;
	unsigned int socdsp_msg_type;
	union {
		struct socdsp_om_load_info load_info;
		struct socdsp_om_effect_mcps mcps_info;
		struct socdsp_om_update_buff_delay_info delay_info;
		struct socdsp_om_dump_log_info log_info;
	} info;
	long info_size[SOCDSP_CPU_OM_INFO_BUTT] = {
		sizeof(info.load_info), sizeof(info.mcps_info),
		sizeof(info.delay_info), sizeof(info.log_info)
	};

	memset(&info, 0, sizeof(info));
	get_workqueue_data(SOCDSP_OM_WORK_AUDIO_OM_DETECTION, data, &data_len);
	socdsp_msg_type = *(unsigned int *)data;
	if (socdsp_msg_type >= SOCDSP_CPU_OM_INFO_BUTT) {
		logi("type %u, not support\n", socdsp_msg_type);
		return;
	}
	if (data_len != info_size[socdsp_msg_type]) {
		logw("unavailable data from socdsp, type: %u, len: %u\n", socdsp_msg_type, data_len);
		return;
	}
	memcpy(&info, data, data_len);
	switch (socdsp_msg_type) {
	case SOCDSP_CPU_OM_LOAD_INFO:
		socdsp_om_cpu_load_info_show(&info.load_info);
		break;
	case SOCDSP_CPU_OM_ALGO_MCPS_INFO:
		socdsp_om_effect_mcps_info_show(&info.mcps_info);
		break;
	case SOCDSP_CPU_OM_UPDATE_BUFF_DELAY_INFO:
		socdsp_om_update_buff_delay_info_show(&info.delay_info);
		break;
	case SOCDSP_CPU_OM_DUMP_LOG_INFO:
		logi("socdsp request dump log, value: %u\n", info.log_info.value);
		socdsp_dump_dsp(NORMAL_LOG, LOG_PATH_HIFI_REQUEST_LOG);
		break;
	default:
		break;
	}
}

static void socdsp_om_voice_bsd_work_handle(struct work_struct *work)
{
	int retval;
	char *envp[2] = {"hifi_voice_bsd_param", NULL};

	retval = kobject_uevent_env(&g_priv_bigdata.dev->kobj, KOBJ_CHANGE, envp);
	if (retval != 0) {
		loge("send uevent failed, retval: %d\n", retval);
		return;
	}
	logi("report uevent success\n");
}

static void voip_bigdata_micblk_param_update(unsigned short paramid,
	unsigned int voip_micblk_bigdata_buff[], struct imonitor_eventobj *voip_bigdata_obj)
{
	int i;
	static const char * const bigdata_voip_param[MIC_BLK_BUT] = {
		"mic_blk_times",
		"hs_top_mic_blk_times",
		"hf_top_mic_blk_times",
		"hs_right_mic_blk_times",
		"hf_right_mic_blk_times",
		"hs_left_mic_blk_times",
		"hf_left_mic_blk_times",
		"hs_head_mic_blk_times",
		"hf_head_mic_blk_times",
		"hs_mic_blk_change_nvid",
		"hf_mic_blk_change_nvid",
		"mic_err_times",
		"hs_mic_dis_change_nvid",
		"hf_mic_dis_change_nvid",
		"left_mic_dis_times",
		"head_mic_dis_times",
		"top_mic_rms",
		"right_mic_rms",
		"left_mic_rms",
		"head_mic_rms",
		"mic_blk_rate",
		"mic_err_rate"
	};

	for (i = paramid; i < MIC_BLK_BUT; i++) {
		imonitor_set_param_integer_v2(voip_bigdata_obj,
			bigdata_voip_param[i], voip_micblk_bigdata_buff[i]);
	}
}

static void voip_blkmic_bigdata_update_imonitor(unsigned int eventid,
	unsigned short paramid,
	voip_micblk_3a_om *voip_micblk_bigdata_buff)
{
	struct imonitor_eventobj *voip_bigdata_obj = NULL;
	unsigned int voip_micblk_bigdata_array[MIC_BLK_BUT] = {0};

	/* creat imonitor obj */
	voip_bigdata_obj = imonitor_create_eventobj(eventid);
	if (voip_bigdata_obj == NULL) {
		logw("imonitor obj create for handset or handfree fail\n");
		return;
	}

	/* carry voip micblk data from kerenl to imonitor */
	voip_micblk_bigdata_array[MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.mic_blk_times;
	voip_micblk_bigdata_array[HS_TOP_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.top_mic_blk_times[HS_DEV];
	voip_micblk_bigdata_array[HF_TOP_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.top_mic_blk_times[HF_DEV];
	voip_micblk_bigdata_array[HS_RIGHT_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.right_mic_blk_times[HS_DEV];
	voip_micblk_bigdata_array[HF_RIGHT_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.right_mic_blk_times[HF_DEV];
	voip_micblk_bigdata_array[HS_LEFT_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.left_mic_blk_times[HS_DEV];
	voip_micblk_bigdata_array[HF_LEFT_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.left_mic_blk_times[HF_DEV];
	voip_micblk_bigdata_array[HS_HEAD_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.head_mic_blk_times[HS_DEV];
	voip_micblk_bigdata_array[HF_HEAD_MIC_BLK_TIMES] = voip_micblk_bigdata_buff->data.head_mic_blk_times[HF_DEV];
	voip_micblk_bigdata_array[HS_MIC_BLK_CHANGE_NVID] = voip_micblk_bigdata_buff->data.mic_blk_change_nvid[HS_DEV];
	voip_micblk_bigdata_array[HF_MIC_BLK_CHANGE_NVID] = voip_micblk_bigdata_buff->data.mic_blk_change_nvid[HF_DEV];
	voip_micblk_bigdata_array[MIC_ERR_TIMES] = voip_micblk_bigdata_buff->data.mic_err_times;
	voip_micblk_bigdata_array[HS_MIC_DIS_CHANGE_NVID] = voip_micblk_bigdata_buff->data.hs_mic_dis_change_nvid;
	voip_micblk_bigdata_array[HF_MIC_DIS_CHANGE_NVID] = voip_micblk_bigdata_buff->data.hf_mic_dis_change_nvid;
	voip_micblk_bigdata_array[LEFT_MIC_DIS_TIMES] = voip_micblk_bigdata_buff->data.left_mic_dis_times;
	voip_micblk_bigdata_array[HEAD_MIC_DIS_TIMES] = voip_micblk_bigdata_buff->data.head_mic_dis_times;
	voip_micblk_bigdata_array[TOP_MIC_RMS] = voip_micblk_bigdata_buff->data.top_mic_rms;
	voip_micblk_bigdata_array[RIGHT_MIC_RMS] = voip_micblk_bigdata_buff->data.right_mic_rms;
	voip_micblk_bigdata_array[LEFT_MIC_RMS] = voip_micblk_bigdata_buff->data.left_mic_rms;
	voip_micblk_bigdata_array[HEAD_MIC_RMS] = voip_micblk_bigdata_buff->data.head_mic_rms;
	voip_micblk_bigdata_array[MIC_BLK_RATE] = voip_micblk_bigdata_buff->data.mic_blk_rate;
	voip_micblk_bigdata_array[MIC_ERR_RATE] = voip_micblk_bigdata_buff->data.mic_err_rate;
	voip_bigdata_micblk_param_update(0, voip_micblk_bigdata_array, voip_bigdata_obj);

	imonitor_send_event(voip_bigdata_obj);
	imonitor_destroy_eventobj(voip_bigdata_obj);
}

static void socdsp_om_show_voice_3a_info(struct work_struct *work)
{
	unsigned char data[MAIL_LEN_MAX] = {'\0'};
	unsigned int data_len = 0;
	unsigned int socdsp_msg_type;
	struct voice_3a_om  voice_3a_om_info;

	get_workqueue_data(SOCDSP_OM_WORK_VOICE_3A, data, &data_len);

	socdsp_msg_type = *(unsigned int *)data;

	switch (socdsp_msg_type) {
	case SOCDSP_3A_INFO_MSG:
		if ((sizeof(voice_3a_om_info)) != data_len) {
			logw("unavailable data from socdsp, type: %u, len: %u\n",
				socdsp_msg_type, data_len);
			return;
		}
		memcpy(&voice_3a_om_info, data, sizeof(voice_3a_om_info));
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_CODEC, DSM_SOC_HIFI_3A_ERROR,
			"3a error type: %u error: %u nvid: %u dat1: %u dat2: %u\n",
			socdsp_msg_type, voice_3a_om_info.recv_msg, voice_3a_om_info.nvid,
			voice_3a_om_info.data1, voice_3a_om_info.data2);
#endif
		break;
	case SOCDSP_VOIP_MICBLK_INFO_MSG:
		logd("received SOCDSP_HIFI_VOIP_MICBLK_INFO_MSG:%u \n", socdsp_msg_type);
		if ((sizeof(voip_micblk_3a_om)) != data_len) {
			logw("unavailable data from socdsp, type: %u, len: %u\n",
				socdsp_msg_type, data_len);
			return;
		}
		voip_blkmic_bigdata_update_imonitor(BIGDATA_VOIP_BLKEVENTID, 0, (voip_micblk_3a_om *)&data);
		break;
	default:
		logi("type: %u, not support\n", socdsp_msg_type);
		break;
	}
}

static void socdsp_om_work_smartpa_dft_handle(struct work_struct *work)
{
	unsigned char data[MAIL_LEN_MAX] = {0};

	get_workqueue_data(SOCDSP_OM_WORK_SMARTPA_DFT, data, NULL);

	hifi_om_work_smartpa_dft_report((const enum smartpa_dft_id *)data);
}

static void socdsp_om_work_audio_db_handle(struct work_struct *work)
{
	unsigned char data[MAIL_LEN_MAX] = {0};

	get_workqueue_data(SOCDSP_OM_WORK_AUDIO_DB, data, NULL);

	hifi_om_work_audio_db((const enum smartpa_dft_id *)data);
}

/* carry data from kernel to imonitor,for headset mode and bluetooth mode */
static void auxhear_bigdata_update_imonitor(unsigned int event_id, char *data, int len)
{
	struct imonitor_eventobj *auxhear_bigdata_obj = NULL;
	struct imedia_auxhear_bigdata *bigdata_mesg = (struct imedia_auxhear_bigdata *)data;

	if (!bigdata_mesg) {
		logw("imonitor data from kernel is empty\n");
		return;
	}

	/* creat imonitor obj */
	auxhear_bigdata_obj = imonitor_create_eventobj(event_id);
	if (!auxhear_bigdata_obj) {
		logw("imonitor obj create fail\n");
		return;
	}
	/* carry  data from kerenl to imonitor */
	imonitor_set_param_integer_v2(auxhear_bigdata_obj,
		"AuxhearMode", bigdata_mesg->imedia_auxhear_mode);
	imonitor_set_param_integer_v2(auxhear_bigdata_obj,
		"AuxhearDevice", bigdata_mesg->imedia_auxhear_device);
	imonitor_set_param_integer_v2(auxhear_bigdata_obj,
		"AuxhearError", bigdata_mesg->imedia_auxhear_error);

	imonitor_send_event(auxhear_bigdata_obj);
	imonitor_destroy_eventobj(auxhear_bigdata_obj);
}

static void socdsp_om_auxhear_bigdata_handle(struct work_struct *work)
{
	unsigned char data[MAIL_LEN_MAX] = {'\0'};
	unsigned int data_len = 0;

	static struct timespec bigdata_time1;
	static struct timespec bigdata_time2;

	get_workqueue_data(SOCDSP_OM_WORK_AUXHEAR_BIGDATA, data, &data_len);

	if (data_len != (sizeof(struct imedia_auxhear_bigdata))) {
		logw("unavailable data from socdsp, data len: %u\n", data_len);
		return;
	}

	bigdata_time2 = current_kernel_time();

	if ((bigdata_time2.tv_sec - bigdata_time1.tv_sec) > AUXHEAR_IMONITOR_UPDATIME) {
		/* carry data from kernel to imonitor */
		auxhear_bigdata_update_imonitor(BIGDATA_AUXHEAR_EVENTID, data, MAIL_LEN_MAX);

		bigdata_time1 = current_kernel_time();
	}
}

static void voice_bigdata_voice_noise_param_update(unsigned short paramid,
	struct imedia_voice_bigdata_to_imonitor *voice_bigdata_buff,
	struct imonitor_eventobj *voice_bigdata_obj)
{
	int i, j;
	static const char * const bigdata_voice_param[VOICE_BIGDATA_NOISE_VOICE_SIZE] = {
		"NoiseCnt0",  "NoiseCnt1",  "NoiseCnt2",  "NoiseCnt3",
		"NoiseCnt4",  "NoiseCnt5",  "NoiseCnt6",  "NoiseCnt7",
		"NoiseCnt8",  "NoiseCnt9",  "NoiseCnt10", "NoiseCnt11",
		"NoiseCnt12", "NoiseCnt13", "NoiseCnt14", "NoiseCnt15",
		"VoiceCnt0",  "VoiceCnt1",  "VoiceCnt2",  "VoiceCnt3",
		"VoiceCnt4",  "VoiceCnt5",  "VoiceCnt6",  "VoiceCnt7",
		"VoiceCnt8",  "VoiceCnt9",  "VoiceCnt10", "VoiceCnt11",
		"VoiceCnt12", "VoiceCnt13", "VoiceCnt14", "VoiceCnt15"
	};

	for (i = paramid; i < VOICE_BIGDATA_NOISESIZE; i++) {
		imonitor_set_param_integer_v2(voice_bigdata_obj,
			bigdata_voice_param[i], voice_bigdata_buff->noise[i]);
		j = i + VOICE_BIGDATA_NOISESIZE;
		imonitor_set_param_integer_v2(voice_bigdata_obj,
			bigdata_voice_param[j], voice_bigdata_buff->voice[i]);
	}
}

static void voice_bigdata_miccheck_param_update(unsigned short paramid, const char *blockmic,
	struct imonitor_eventobj *voice_bigdata_obj)
{
	int i;
	static const char * const miccheck_param[BIGDATA_VOICE_BLOCKMIC] = {
		"BlockmicCnt0",  "BlockmicCnt1",  "BlockmicCnt2",  "BlockmicCnt3",
		"BlockmicCnt4",  "BlockmicCnt5",  "BlockmicCnt6",  "BlockmicCnt7",
		"BlockmicCnt8",  "BlockmicCnt9",  "BlockmicCnt10", "BlockmicCnt11",
		"BlockmicCnt12", "BlockmicCnt13", "BlockmicCnt14", "BlockmicCnt15",
		"BlockmicCnt16", "BlockmicCnt17", "BlockmicCnt18", "BlockmicCnt19"
	};

	for (i = paramid; i < BIGDATA_VOICE_BLOCKMIC; i++) {
		imonitor_set_param_integer_v2(voice_bigdata_obj,
			miccheck_param[i], *blockmic);
		blockmic++;
	}
}

static void voice_bigdata_charact_param_hs_update(
	struct imedia_voice_bigdata_to_imonitor *voice_bigdata_buff,
	struct imonitor_eventobj *voice_bigdata_obj)
{
	int i, j;
	int size;
	static const char * const charact_param_hs[] = {
		"WhisperCnt", "AveCnt",   "BweCnt", "AutolvmCnt",
		"WindCnt",    "AngleCnt", "SvmCnt", "SvmoutCnt"
	};

	size = ARRAY_SIZE(charact_param_hs);
	for (i = 0; i < size; i++) {
		j = (VOICE_BIGDATA_CHARACTSIZE - 1) - i;
		imonitor_set_param_integer_v2(voice_bigdata_obj,
			charact_param_hs[i],
			voice_bigdata_buff->charact[j]);
	}
}

static void voice_bigdata_charact_param_update(
	struct imedia_voice_bigdata_to_imonitor *voice_bigdata_buff,
	struct imonitor_eventobj *voice_bigdata_obj)
{
	int i, j;
	int size;
	static const char * const charact_param[] = {
		"WhisperCnt", "AveCnt", "BweCnt", "AutolvmCnt", "WindCnt"
	};

	size = ARRAY_SIZE(charact_param);
	for (i = 0; i < size; i++) {
		j = (VOICE_BIGDATA_CHARACTSIZE - 1) - i;
		imonitor_set_param_integer_v2(voice_bigdata_obj,
			charact_param[i],
			voice_bigdata_buff->charact[j]);
	}
}

/* carry data from kernel to imonitor,for handset mode and handfree mode */
static void voice_bigdata_update_imonitor_inc_blkmic(unsigned int eventid,
	unsigned short paramid,
	struct imedia_voice_bigdata_to_imonitor *voice_bigdata_buff,
	const char *blockmic)
{
	struct imonitor_eventobj *voice_bigdata_obj = NULL;

	/* creat imonitor obj */
	voice_bigdata_obj = imonitor_create_eventobj(eventid);
	if (!voice_bigdata_obj) {
		logw("imonitor obj create for handset or handfree fail\n");
		return;
	}

	/* carry noise and voice data from kerenl to imonitor */
	voice_bigdata_voice_noise_param_update(paramid, voice_bigdata_buff, voice_bigdata_obj);

	/* carry blockmic data from kerenl to imonitor */
	voice_bigdata_miccheck_param_update(paramid, blockmic, voice_bigdata_obj);

	/* carry charact data from kerenl to imonitor */
	if (eventid == BIGDATA_VOICE_HSEVENTID)
		voice_bigdata_charact_param_hs_update(voice_bigdata_buff, voice_bigdata_obj);
	else
		voice_bigdata_charact_param_update(voice_bigdata_buff, voice_bigdata_obj);

	imonitor_send_event(voice_bigdata_obj);
	imonitor_destroy_eventobj(voice_bigdata_obj);
}

/* carry data from kernel to imonitor,for headset mode and bluetooth mode */
static void voice_bigdata_update_imonitor(unsigned int eventid, unsigned short paramid,
	struct imedia_voice_bigdata_to_imonitor *voice_bigdata_buff)
{
	struct imonitor_eventobj *voice_bigdata_obj = NULL;

	/*creat imonitor obj*/
	voice_bigdata_obj = imonitor_create_eventobj(eventid);
	if (!voice_bigdata_obj) {
		logw("imonitor obj create for headset or bluetooth fail\n");
		return;
	}

	/* carry charact data from kerenl to imonitor */
	voice_bigdata_charact_param_update(voice_bigdata_buff, voice_bigdata_obj);
	/* carry noise and voice data from kerenl to imonitor */
	voice_bigdata_voice_noise_param_update(paramid, voice_bigdata_buff, voice_bigdata_obj);

	imonitor_send_event(voice_bigdata_obj);
	imonitor_destroy_eventobj(voice_bigdata_obj);
}

static void voice_bigdata_blockmic(char array[][BIGDATA_VOICE_BLOCKMIC],
	unsigned char device, unsigned char data)
{
	if (device > BLOCKMIC_DEVICE_MAX) {
		logw("unavailable bigdata blockmic data\n");

		return;
	}
	if ((data > 0) && (data < BLKMIC_SECONE))
		array[device][data-1]++;
	else if ((data >= BLKMIC_SECONE) && (data < BLKMIC_SECTWO))
		array[device][16]++;
	else if ((data >= BLKMIC_SECTWO) && (data < BLKMIC_SECTHR))
		array[device][17]++;
	else if ((data >= BLKMIC_SECTHR) && (data < BLKMIC_SECFOUR))
		array[device][18]++;
	else if (data >= BLKMIC_SECFOUR)
		array[device][19]++;
}

static void voice_bigdata_voicecharact(
	struct imedia_voice_bigdata_to_imonitor array[4],
	unsigned char device, unsigned int data)
{
	unsigned int shi_var;
	unsigned int datatran;
	unsigned int index;
	int i;

	if (device > VOICECHARACTER_DEVICE_MAX) {
		logw("unavailable bigdata voicecharact data\n");
		return;
	}

	datatran = data;
	shi_var = 0x80000000;
	for (i = 0; i < VOICE_BIGDATA_CHARACTSIZE; i++) {
		index = !!(datatran & shi_var);
		array[device].charact[i] += index;
		shi_var = shi_var>>1;
	}
}

static void voice_bigdata_decode(char arrayblock[][BIGDATA_VOICE_BLOCKMIC],
	struct imedia_voice_bigdata_to_imonitor arraybigdata[4],
	unsigned char *data)
{
	struct imedia_voice_bigdata *bigdata_mesg = NULL;
	int index;

	if (!data) {
		logw("data from socdsp is empty\n");
		return;
	}
	bigdata_mesg = (struct imedia_voice_bigdata *)data;
	switch (bigdata_mesg->bigdata_device) {
	case MLIB_DEVICE_HANDSET:
	case MLIB_DEVICE_HANDFREE:
		voice_bigdata_blockmic(arrayblock,
			bigdata_mesg->bigdata_device,
			bigdata_mesg->bigdata_miccheck);
		break;
	case MLIB_DEVICE_USBHEADSET:
	case MLIB_DEVICE_HEADSET:
		bigdata_mesg->bigdata_device = MLIB_DEVICE_CARFREE;
		break;
	case MLIB_DEVICE_BLUETOOTH:
		bigdata_mesg->bigdata_device = MLIB_DEVICE_HEADSET;
		break;
	default:
		return;
	}

	index = bigdata_mesg->bigdata_device;
	arraybigdata[index].noise[bigdata_mesg->bigdata_noise]++;
	arraybigdata[index].voice[bigdata_mesg->bigdata_voice]++;
	voice_bigdata_voicecharact(arraybigdata, bigdata_mesg->bigdata_device,
		bigdata_mesg->bigdata_charact);
}

static void voice_bigdata_update(char arrayblock[][BIGDATA_VOICE_BLOCKMIC],
	struct imedia_voice_bigdata_to_imonitor arraybigdata[4])
{
	char *blockmic = NULL;

	/* carry data from kernel to imonitor,handset mode */
	blockmic = (char *)arrayblock[0];
	voice_bigdata_update_imonitor_inc_blkmic(BIGDATA_VOICE_HSEVENTID,
		E916200001_NOISECNT0_TINYINT, &arraybigdata[0], blockmic);

	/* carry data from kernel to imonitor,handfree mode */
	blockmic = (char *)arrayblock[1];
	voice_bigdata_update_imonitor_inc_blkmic(BIGDATA_VOICE_HFEVENTID,
		E916200002_NOISECNT0_TINYINT, &arraybigdata[1], blockmic);

	/* carry data from kernel to imonitor,headset mode */
	voice_bigdata_update_imonitor(BIGDATA_VOICE_HESEVENTID,
		E916200003_NOISECNT0_TINYINT, &arraybigdata[2]);

	/* carry data from kernel to imonitor,bluetooth mode */
	voice_bigdata_update_imonitor(BIGDATA_VOICE_BTEVENTID,
		E916200004_NOISECNT0_TINYINT, &arraybigdata[3]);
}
static void socdsp_om_voice_bigdata_handle(struct work_struct *work)
{
	unsigned char data[MAIL_LEN_MAX] = {'\0'};
	unsigned int data_len = 0;
	static struct imedia_voice_bigdata_to_imonitor voice_bigdata[4];
	static char bigdata_miccheck[2][BIGDATA_VOICE_BLOCKMIC];
	static struct timespec bigdata_time1;
	static struct timespec bigdata_time2;
	static int socdsp_ap_count;

	get_workqueue_data(SOCDSP_OM_WORK_VOICE_BIGDATA, data, &data_len);

	if (sizeof(struct imedia_voice_bigdata) != data_len) {
		logw("unavailable data from socdsp, data len: %u\n", data_len);
		return;
	}

	socdsp_ap_count++;
	bigdata_time2 = current_kernel_time();
	if (socdsp_ap_count < HIFI_AP_MESG_CNT)
		voice_bigdata_decode(bigdata_miccheck, voice_bigdata, data);

	if (IMONITOR_UPDATIME < (bigdata_time2.tv_sec - bigdata_time1.tv_sec)) {
		voice_bigdata_update(bigdata_miccheck, voice_bigdata);
		memset(voice_bigdata, 0, sizeof(voice_bigdata));
		memset(bigdata_miccheck, 0, sizeof(bigdata_miccheck));

		bigdata_time1 = current_kernel_time();
		socdsp_ap_count = 0;
	}
}

static struct socdsp_om_work_info work_info[SOCDSP_OM_WORK_MAX] = {
	{ SOCDSP_OM_WORK_VOICE_BSD, "hifi_om_work_voice_bsd",
		socdsp_om_voice_bsd_work_handle, {0} },
	{ SOCDSP_OM_WORK_AUDIO_OM_DETECTION, "hifi_om_work_audio_om_detect",
		socdsp_om_show_audio_detect_info, {0} },
	{ SOCDSP_OM_WORK_VOICE_3A, "hifi_om_work_voice_3a",
		socdsp_om_show_voice_3a_info, {0} },
	{ SOCDSP_OM_WORK_VOICE_BIGDATA, "hifi_om_work_voice_bigdata",
		socdsp_om_voice_bigdata_handle, {0} },
	{ SOCDSP_OM_WORK_SMARTPA_DFT, "hifi_om_work_smartpa_dft_report",
		socdsp_om_work_smartpa_dft_handle, {0} },
	{ SOCDSP_OM_WORK_AUDIO_DB, "hifi_om_work_audio_db",
		socdsp_om_work_audio_db_handle, {0} },
	{ SOCDSP_OM_WORK_AUXHEAR_BIGDATA, "hifi_om_work_auxhear_bigdata",
		socdsp_om_auxhear_bigdata_handle, {0} },
};

static int get_workqueue_data(enum socdsp_om_work_id work_id,
	unsigned char *data, unsigned int *len)
{
	unsigned int data_len = 0;
	struct socdsp_om_work *om_work = NULL;

	spin_lock_bh(&work_info[work_id].ctl.lock);
	if (!list_empty(&work_info[work_id].ctl.list)) {
		om_work = list_entry(work_info[work_id].ctl.list.next,
			struct socdsp_om_work, om_node);

		data_len = om_work->data_len;
		if (len != NULL)
			*len = data_len;
		memcpy(data, om_work->data, om_work->data_len);

		list_del(&om_work->om_node);
		kzfree(om_work);
	} else {
		spin_unlock_bh(&work_info[work_id].ctl.lock);
		if (len != NULL)
			*len = data_len;
		return -EAGAIN;
	}
	spin_unlock_bh(&work_info[work_id].ctl.lock);

	return 0;
}

void socdsp_om_rev_data_handle(enum socdsp_om_work_id work_id,
	const unsigned char *addr, unsigned int len)
{
	struct socdsp_om_work *work = NULL;

	if (!addr || len == 0 || len > MAIL_LEN_MAX) {
		loge("addr is null or len is invaled, len: %u", len);
		return;
	}

	if (!g_priv_bigdata.is_inited) {
		loge("om bigdata is not init\n");
		return;
	}

	work = kzalloc(sizeof(*work) + len, GFP_ATOMIC);
	if (!work) {
		loge("malloc size %zu failed\n", sizeof(*work) + len);
		return;
	}
	memcpy(work->data, addr, len); /*lint !e419 */
	work->data_len = len;

	spin_lock_bh(&work_info[work_id].ctl.lock);
	list_add_tail(&work->om_node, &work_info[work_id].ctl.list);
	spin_unlock_bh(&work_info[work_id].ctl.lock);

	if (!queue_work(work_info[work_id].ctl.wq, &work_info[work_id].ctl.work))
		loge("work id: %d, this work was already on the queue\n", work_id);
}

static int socdsp_init_workqueue(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(work_info); i++) {
		work_info[i].ctl.wq = create_singlethread_workqueue(work_info[i].work_name);
		if (!work_info[i].ctl.wq) {
			loge("workqueue create failed\n");
			return -EPERM;
		}
		INIT_WORK(&work_info[i].ctl.work, work_info[i].func);
		spin_lock_init(&work_info[i].ctl.lock);
		INIT_LIST_HEAD(&work_info[i].ctl.list);
	}

	return 0;
}

static void socdsp_deinit_workqueue(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(work_info); i++) {
		if (!work_info[i].ctl.wq)
			continue;
		flush_workqueue(work_info[i].ctl.wq);
		destroy_workqueue(work_info[i].ctl.wq);
		work_info[i].ctl.wq = NULL;
	}
}

int om_bigdata_init(struct platform_device *pdev)
{
	int ret;

	memset(&g_priv_bigdata, 0, sizeof(g_priv_bigdata));

	g_priv_bigdata.dev = &pdev->dev;

	ret = socdsp_init_workqueue();
	if (ret != 0)
		return -EPERM;

	g_priv_bigdata.is_inited = true;

	return 0;
}

void om_bigdata_deinit(void)
{
	g_priv_bigdata.is_inited = false;

	socdsp_deinit_workqueue();
}
