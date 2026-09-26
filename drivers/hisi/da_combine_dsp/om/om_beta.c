/*
 * om_beta.c
 *
 * anc beta module for da_combine codec
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

#include "om_beta.h"

#include <linux/types.h>
#include <linux/time.h>
#include <linux/rtc.h>

#include "linux/hisi/audio_log.h"
#include <da_combine_dsp_interface.h>
#include <rdr_audio_adapter.h>
#include <rdr_audio_codec.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#include "da_combine_dsp_misc.h"
#include "huawei_platform/log/imonitor.h"
#include "huawei_platform/log/imonitor_keys.h"
#include "om_hook.h"

#define LOG_TAG "om_beta"

/*lint -e655 -e838 -e730 -e754 -e747 -e731*/
#define CODEC_DSP_ANC_ERR_BASE_ID 916000000
#define CODEC_DSP_SMARTPA_ERR_BASE_ID 916000100
#define VIRTUAL_BTN_INFO_EVENT_ID 931001002

#define HOOK_PATH_VIRTUAL_BTN "/data/log/codec_dsp/"
#define FULL_PERMISSION true
#define SAFE_PERMISSION false
#define EVENTLEVEL E916000001_EVENTLEVEL_INT
#define EVENTMODULE E916000001_EVENTMODULE_VARCHAR
#define DSM_LOG_STR_NUM 8
#define LOW_ACTTIME_RATE_START 24
#define PROCESS_PATH_ERR_START 28
#define FULL_PERMS_LEN 4
#define SAFE_PERMS_LEN 1

/*
 * This data structure holds statistics data of 5 adaptive parameters.
 * After reading, the statistics of the 5 adaptive parameters will reset.
 */
struct dsm_adp_statistics {
	int max_fc_q9; // Q9, in Hz, valid floating-point range = [0, 4194304)
	int min_fc_q9; // Q9, in Hz, valid floating-point range = [0, 4194304)
	int max_qc_q29; // Q29, valid floating-point range = [0, 4)
	int min_qc_q29; // Q29, valid floating-point range = [0, 4)
	int max_rdc_q27; // Q27, valid floating-point range = [0, 16)
	int min_rdc_q27; // Q27, valid floating-point range = [0, 16)
	int max_temp_q19; // Q19,
	int min_temp_q19; // Q19
	float avg_temp; // floating-point,
	int max_excur_q27; // Q27, valid floating-point range = [0, 16)
	int min_excur_q27; // Q27, valid floating-point range = [0, 16)
	int max_abs_input_q0; // Q0
	int max_abs_output_q0; // Q0
	int max_abs_idata_q0; // Q0
	int max_abs_vdata_q0; // Q0
	int spker_short_circuit_warning;
	int spker_open_circuit_warning;
	float elapsed_frames;
	int exc_limiter_overshoots;
};

static const struct om_hook_para g_anc_hook_para_full_perms[FULL_PERMS_LEN] = {
	{ HOOK_POS_ANC_PCM_REF, 2, 1, 16000 },
	{ HOOK_POS_ANC_PCM_ERR, 2, 1, 16000 },
	{ HOOK_POS_ANC_PCM_ANTI, 2, 1, 16000 },
	{ HOOK_POS_ANC_BETA_CSINFO, 2, 1, 8000 }
};

static const struct om_hook_para g_anc_hook_para_safe_perms[SAFE_PERMS_LEN] = {
	{ HOOK_POS_ANC_BETA_CSINFO, 2, 1, 8000 }
};

static const char * const wakeup_err_type_tab[WAKEUP_ERR_TYPE_BUTT] = {
	"model loading is abnormal",
	"failed to initialize the algorithm",
	"interrupt is abnormal",
	"performance is insufficient",
};

static const char integ_str[VIRTBTN_INTEGRATE_BUTT][VIRTUAL_BTN_STR_MAX_LENGTH] = {
	"BUTTON_NUM", "PASSIVE_NUM", "ACTIVE_NUM",
};

static const char algo_str[VIRTBTN_ALGO_BUTT][VIRTUAL_BTN_STR_MAX_LENGTH] = {
	"CNT_SOFT_LOG", "FORCE_MAX_GRAM", "TIMER_PZT_FORCE", "FORCE_MAX",
	"TIMER_ULTRA_PRESS", "CAL_ULT_MIN", "FITTING_LINEARITY",
	"TIMER_ULTRA_UP", "ULT_BASELINE_APPROX", "PRESS_DOWN",
	"PRESS_UP", "ULT_NOT_DET", "PZT_ULT_SLEEP", "DATA_ULT_BASELINE",
	"SLOPE_INTERCEPT", "FREQ_SCAN",
};

static char g_output_str[VIRTUAL_BTN_DUMP_MSG_MAX_LENGTH] = {0};

bool voice_record_permission = SAFE_PERMISSION;

struct da_combine_dsp_beta {
	struct timeval btn_upload_time;
	bool virtual_btn_create_dir;
};

static struct da_combine_dsp_beta *priv_beta;

static int virtual_btn_beta_log_upload(const struct vitrual_bnt_data_log *info)
{
	struct imonitor_eventobj *obj = NULL;
	unsigned int event_id;
	int ret;
	int i;

	event_id = VIRTUAL_BTN_INFO_EVENT_ID;
	obj = imonitor_create_eventobj(event_id);
	if (obj == NULL) {
		AUDIO_LOGE("virtual btn imonitor create eventobj error");
		return -EINVAL;
	}
	imonitor_set_param_integer_v2(obj, "btn_num", (long)info->btn_num);
	imonitor_set_param_integer_v2(obj, "passive_num", (long)info->passive_num);
	imonitor_set_param_integer_v2(obj, "active_num", (long)info->active_num);
	for (i = 0; i < VIRTUAL_BTN_MSG_NUM; i++) {
		if (info->data_log[i][VIRTBTN_PRESS_UP] ==
			VIRTUAL_BTN_PRESS_UP_FLAG)
			break;
	}
	if (i < VIRTUAL_BTN_MSG_NUM) {
		imonitor_set_param_integer_v2(obj, "fitting_linearity",
			(long)info->data_log[i][VIRTBTN_FITTING_LINEARITY]);
		imonitor_set_param_integer_v2(obj, "data_ult_baseline",
			(long)info->data_log[i][VIRTBTN_DATA_ULT_BASELINE]);
		imonitor_set_param_integer_v2(obj, "slope_intercept",
			(long)info->data_log[i][VIRTBTN_SLOPE_INTERCEPT]);
		imonitor_set_param_integer_v2(obj, "freq_scan",
			(long)info->data_log[i][VIRTBTN_FREQ_SCAN]);
	} else {
		imonitor_set_param_integer_v2(obj, "fitting_linearity", 0);
		imonitor_set_param_integer_v2(obj, "data_ult_baseline", 0);
		imonitor_set_param_integer_v2(obj, "slope_intercept", 0);
		imonitor_set_param_integer_v2(obj, "freq_scan", 0);
	}

	ret = imonitor_send_event(obj);
	if (ret <= 0) {
		AUDIO_LOGE("imonitor send event err, ret: %d", ret);
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	imonitor_destroy_eventobj(obj);

	return ret;
}

static void get_btn_time(char *output_str, unsigned int size)
{
	struct rtc_time tm;
	struct timeval tv;
	char buf[VIRTUAL_BTN_TERM_MAX_LENGTH] = {0};

	do_gettimeofday(&tv);
	tv.tv_sec -= (long)sys_tz.tz_minuteswest * 60;
	rtc_time_to_tm(tv.tv_sec, &tm);

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%04d%02d%02d%02d%02d%02d/",
		tm.tm_year + 1900, tm.tm_mon + 1,
		tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	if (strlen(buf) >= (size - strlen(output_str))) {
		AUDIO_LOGE("buf string exceed, len is: %zd", strlen(buf));
		return;
	}

	strncat(output_str, buf, size - strlen(output_str) - 1);
}

static void get_btn_algo(char *output_str, unsigned int size,
	const unsigned short *data_log, const char algo_string[][VIRTUAL_BTN_STR_MAX_LENGTH])
{
	int i;
	char buf[VIRTUAL_BTN_TERM_MAX_LENGTH] = {0};

	for (i = 0; i < VIRTBTN_ALGO_BUTT; i++) {
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%s:%d ", algo_string[i],
		(int)data_log[i]);

		if (strlen(buf) >= (size - strlen(output_str))) {
			AUDIO_LOGE("buf string exceed, len is: %zd", strlen(buf));
			return;
		}

		strncat(output_str, buf, size - strlen(output_str) - 1);
	}
}

static void get_btn_pressup(char *output_str, unsigned int size,
	const unsigned short *data_log, const char integ_string[][VIRTUAL_BTN_STR_MAX_LENGTH],
	const unsigned short *vir_num)
{
	int i;
	char buf[VIRTUAL_BTN_TERM_MAX_LENGTH] = {0};

	if (data_log[VIRTBTN_PRESS_UP] == VIRTUAL_BTN_PRESS_UP_FLAG) {
		for (i = 0; i < VIRTBTN_INTEGRATE_BUTT; i++) {
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf), "%s:%d ", integ_string[i],
				(int)vir_num[i]);

			if (strlen(buf) >= (size - strlen(output_str))) {
				AUDIO_LOGE("buf string exceed, len is: %zd", strlen(buf));
				return;
			}

			strncat(output_str, buf, size - strlen(output_str) - 1);
		}
	}
}

static int virtual_btn_param_dump(const struct vitrual_bnt_data_log *info)
{
	int i;
	unsigned short vir_num[VIRTBTN_INTEGRATE_BUTT] = {0};

	memset(g_output_str, 0, sizeof(g_output_str));
	memcpy(vir_num, &info->btn_num,
		VIRTBTN_INTEGRATE_BUTT * sizeof(unsigned short));

	for (i = 0; i < VIRTUAL_BTN_MSG_NUM; i++) {
		if (info->data_log[i][VIRTBTN_PZT_ULT_SLEEP]) {
			get_btn_time(g_output_str, sizeof(g_output_str));
			get_btn_algo(g_output_str, sizeof(g_output_str),
				info->data_log[i], algo_str);
			get_btn_pressup(g_output_str, sizeof(g_output_str),
				info->data_log[i], integ_str, vir_num);

			AUDIO_LOGI("virtual btn %s %d", g_output_str,
				(int)strlen(g_output_str));
			da_combine_virtual_btn_beta_dump_file(g_output_str,
				strlen(g_output_str),
				priv_beta->virtual_btn_create_dir);
			priv_beta->virtual_btn_create_dir = false;
		}
	}
	return 0;
}

void da_combine_set_voice_hook_switch(unsigned short permission)
{
	voice_record_permission =
		(permission == 1) ? FULL_PERMISSION : SAFE_PERMISSION;
}

int da_combine_report_pa_buffer_reverse(const void *data)
{
	const struct pa_buffer_reverse_msg *reverse_msg = NULL;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	reverse_msg = (struct pa_buffer_reverse_msg *)data;
	AUDIO_LOGE("pa count:%u, proc time:%u00us",
		reverse_msg->pa_count, reverse_msg->proc_interval);

#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, DSM_CODEC_HIFI_TIMEOUT,
	"pa count:%u, proc time:%u00us\n", reverse_msg->pa_count,
	reverse_msg->proc_interval);
#endif
	return 0;
}

int da_combine_anc_beta_start_hook(const void *data)
{
	struct da_combine_param_io_buf krn_param;
	struct om_start_hook_msg msg;
	const struct om_hook_para *anc_hook_para = NULL;
	unsigned char *buf_in = NULL;
	unsigned int buf_size_in;
	int ret;

	UNUSED_PARAMETER(data);
	AUDIO_LOGI("anc detection start hook, permission %d(1:inc voice)\n",
		voice_record_permission);
	msg.msg_id = ID_AP_DSP_HOOK_START;
	if (voice_record_permission) {
		msg.para_size = sizeof(g_anc_hook_para_full_perms);
		anc_hook_para = g_anc_hook_para_full_perms;
	} else {
		msg.para_size = sizeof(g_anc_hook_para_safe_perms);
		anc_hook_para = g_anc_hook_para_safe_perms;
	}

	buf_in = kzalloc(sizeof(msg) + msg.para_size, GFP_ATOMIC);
	if (buf_in == NULL) {
		AUDIO_LOGE("buf in alloc fail\n");
		return -ENOMEM;
	}
	memcpy(buf_in, &msg, sizeof(msg));
	memcpy(buf_in + sizeof(msg), anc_hook_para, (unsigned long)msg.para_size);

	buf_size_in = sizeof(msg) + msg.para_size;

	krn_param.buf_in = buf_in;
	krn_param.buf_size_in = buf_size_in;

	da_combine_set_dsp_hook_sponsor(OM_SPONSOR_BETA);
	da_combine_set_dsp_hook_bw(OM_BANDWIDTH_6144);
	ret = da_combine_start_hook(&krn_param);

	kfree(buf_in);

	return ret;
}

int da_combine_anc_beta_stop_hook(const void *data)
{
	struct da_combine_param_io_buf krn_param;
	struct om_start_hook_msg msg;
	const struct om_hook_para *anc_hook_para = NULL;
	unsigned char *buf_in = NULL;
	unsigned int buf_size_in;
	int ret;

	UNUSED_PARAMETER(data);
	AUDIO_LOGI("anc detection stop hook, permission %d(1:inc voice)",
		voice_record_permission);
	msg.msg_id = ID_AP_DSP_HOOK_START;
	if (voice_record_permission) {
		msg.para_size = sizeof(g_anc_hook_para_full_perms);
		anc_hook_para = g_anc_hook_para_full_perms;
	} else {
		msg.para_size = sizeof(g_anc_hook_para_safe_perms);
		anc_hook_para = g_anc_hook_para_safe_perms;
	}

	buf_in = kzalloc(sizeof(msg) + msg.para_size, GFP_ATOMIC);
	if (buf_in == NULL) {
		AUDIO_LOGE("buf in alloc fail");
		return -ENOMEM;
	}
	memcpy(buf_in, &msg, sizeof(msg));
	memcpy(buf_in + sizeof(msg), anc_hook_para, (unsigned long)msg.para_size);

	buf_size_in = sizeof(msg) + msg.para_size;

	krn_param.buf_in = buf_in;
	krn_param.buf_size_in = buf_size_in;

	ret = da_combine_stop_hook(&krn_param);

	kfree(buf_in);

	return ret;
}

void da_combine_anc_beta_generate_path(enum hook_pos pos,
	const char *base_path, char *full_path, unsigned long full_path_len)
{
	if (base_path == NULL || full_path == NULL) {
		AUDIO_LOGE("base path or full path is null");
		return;
	}

	switch (pos) {
	case HOOK_POS_ANC_PCM_REF:
		snprintf(full_path, full_path_len - 1,
			"%sanc_pcm_ref.data", base_path);
		break;
	case HOOK_POS_ANC_PCM_ERR:
		snprintf(full_path, full_path_len - 1,
			"%sanc_pcm_err.data", base_path);
		break;
	case HOOK_POS_ANC_PCM_ANTI:
		snprintf(full_path, full_path_len - 1,
			"%sanc_pcm_anti.data", base_path);
		break;
	case HOOK_POS_ANC_BETA_CSINFO:
		snprintf(full_path, full_path_len - 1,
			"%sanc_cs_info.log", base_path);
		break;
	default:
		break;
	}
}

int da_combine_anc_beta_upload_log(const void *data)
{
	const struct mlib_anc_dft_info *info = (struct mlib_anc_dft_info *)data;
	struct imonitor_eventobj *obj = NULL;
	unsigned int event_id;
	int ret;
	char rsn[DSM_LOG_STR_NUM] = {0};

	if (info == NULL) {
		AUDIO_LOGE("info is null");
		return -EINVAL;
	}

	event_id = CODEC_DSP_ANC_ERR_BASE_ID + info->err_class;
	obj = imonitor_create_eventobj(event_id);

	imonitor_set_param(obj, EVENTLEVEL, info->err_level);
	if (voice_record_permission)
		imonitor_set_param(obj, EVENTMODULE, (long)(uintptr_t)"ANC_inc_voice");
	else
		imonitor_set_param(obj, EVENTMODULE, (long)(uintptr_t)"ANC_no_voice");

	if (info->err_class == LOW_ACTTIME_RATE)
		imonitor_set_param(obj, E916000004_PROBABILITY_TINYINT,
			*(unsigned int *)(info->details + LOW_ACTTIME_RATE_START));

	if (info->err_class == PROCESS_PATH_ERR) {
		snprintf(rsn, DSM_LOG_STR_NUM - 1, "%d",
			*(unsigned int *)(info->details + PROCESS_PATH_ERR_START));
		imonitor_set_param(obj, E916000005_CAUSECASE_VARCHAR,
			(long)(uintptr_t)rsn);
	}
	ret = imonitor_send_event(obj);
	if (ret <= 0) {
		AUDIO_LOGE("imonitor send event err, ret: %d", ret);
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	imonitor_destroy_eventobj(obj);

	return ret;
}

int da_combine_dsm_beta_dump_file(const void *data, bool create_dir)
{
	const struct mlib_dsm_dft_info *info = (struct mlib_dsm_dft_info *)data;
	char fullname[HOOK_PATH_MAX_LENGTH] = {0};

	if (info == NULL) {
		AUDIO_LOGE("info is null");
		return -EINVAL;
	}

	if (create_dir)
		da_combine_dsp_create_hook_dir(HOOK_PATH_BETA_CLUB);

	snprintf(fullname, sizeof(fullname) - 1, "%s%s", HOOK_PATH_BETA_CLUB,
		"pa_om_info.log");

	if (info->msg_size > sizeof(struct mlib_dsm_dft_info)) {
		da_combine_dsp_dump_to_file((char *)info,
			sizeof(struct mlib_dsm_dft_info), fullname);
		AUDIO_LOGE("message size is wrong");
		return -EINVAL;
	}
	da_combine_dsp_dump_to_file((char *)info,  info->msg_size, fullname);

	return 0;
}

int da_combine_dsm_beta_log_upload(const void *data)
{
	const struct mlib_dsm_dft_info *info = (struct mlib_dsm_dft_info *)data;
	struct imonitor_eventobj *obj = NULL;
	unsigned int event_id;
	struct dsm_adp_statistics *dsm_info = NULL;
	int ret;
	char num_str[DSM_LOG_STR_NUM] = {0};

	if (info == NULL) {
		AUDIO_LOGE("info is null");
		return -EINVAL;
	}

	event_id = CODEC_DSP_SMARTPA_ERR_BASE_ID + info->err_class;
	obj = imonitor_create_eventobj(event_id);
	imonitor_set_param(obj, E916000101_EVENTLEVEL_INT, info->err_level);
	imonitor_set_param(obj, E916000101_EVENTMODULE_VARCHAR,
		(long)(uintptr_t)"SmartPa");
	if (info->err_class == DSM_OM_ERR_TYPE_PROC) {
		dsm_info = (struct dsm_adp_statistics *)info->err_info;
		imonitor_set_param(obj, E916000101_ERRCODE_INT, info->err_code);
		imonitor_set_param(obj, E916000101_MAXRDC_INT, dsm_info->max_rdc_q27);
		imonitor_set_param(obj, E916000101_MINRDC_INT, dsm_info->min_rdc_q27);
		imonitor_set_param(obj, E916000101_MAXTEMP_INT, dsm_info->max_temp_q19);
		imonitor_set_param(obj, E916000101_MINTEMP_INT, dsm_info->min_temp_q19);
		imonitor_set_param(obj, E916000101_MAXAMP_INT, dsm_info->max_excur_q27);
		imonitor_set_param(obj, E916000101_MINAMP_INT, dsm_info->min_excur_q27);
	} else {
		snprintf(num_str, DSM_LOG_STR_NUM - 1, "%d", info->err_line_num);
		imonitor_set_param(obj, E916000102_ERRPOSTAG_VARCHAR,
			(long)(uintptr_t)num_str);
	}
	ret = imonitor_send_event(obj);
	if (ret <= 0) {
		AUDIO_LOGE("imonitor send event err, ret: %d", ret);
		ret = -EINVAL;
	} else {
		ret = 0;
	}

	imonitor_destroy_eventobj(obj);

	return ret;
}

int da_combine_dsm_report_with_creat_dir(const void *data)
{
	int ret;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	ret = da_combine_dsm_beta_dump_file(data, true);
	if (ret != 0) {
		AUDIO_LOGE("dump file err");
		return ret;
	}

	ret = da_combine_dsm_beta_log_upload(data);
	if (ret != 0) {
		AUDIO_LOGE("log upload err");
		return ret;
	}

	return ret;
}

int da_combine_dsm_report_without_creat_dir(const void *data)
{
	int ret;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	ret = da_combine_dsm_beta_dump_file(data, false);
	if (ret != 0) {
		AUDIO_LOGE("dump file err");
		return ret;
	}

	ret = da_combine_dsm_beta_log_upload(data);
	if (ret != 0) {
		AUDIO_LOGE("log upload err");
		return ret;
	}

	return ret;
}

int da_combine_dsm_dump_file_without_creat_dir(const void *data)
{
	int ret;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	ret = da_combine_dsm_beta_dump_file(data, false);
	if (ret != 0) {
		AUDIO_LOGE("dump file err");
		return ret;
	}

	return ret;
}

int da_combine_virtual_btn_beta_dump_file(const void *data,
	unsigned int len, bool create_dir)
{
	char fullname[HOOK_PATH_MAX_LENGTH] = {0};

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	if (create_dir)
		da_combine_dsp_create_hook_dir(HOOK_PATH_VIRTUAL_BTN);

	snprintf(fullname, sizeof(fullname), "%s%s", HOOK_PATH_VIRTUAL_BTN,
		"virtual_btn_info");
	da_combine_dsp_dump_to_file(data, len, fullname);

	return 0;
}

int da_combine_upload_virtual_btn_beta(const void *data)
{
	struct timeval time_update;
	const struct vitrual_bnt_data_log *info = NULL;

	if (priv_beta == NULL) {
		AUDIO_LOGE("priv beta is null");
		return -EINVAL;
	}

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	info = (struct vitrual_bnt_data_log *)data;

	if (bbox_check_edition() == EDITION_INTERNAL_BETA)
		virtual_btn_param_dump(info);

	do_gettimeofday(&time_update);
	if (time_update.tv_sec - priv_beta->btn_upload_time.tv_sec >
		VIRTUAL_BTN_DAY_SECOND) {
		priv_beta->btn_upload_time.tv_sec = time_update.tv_sec;
		virtual_btn_beta_log_upload(info);
	}

	return 0;
}

int da_combine_wakeup_err_msg(const void *data)
{
	const struct wake_up_err_msg *msg = NULL;

	if (data == NULL) {
		AUDIO_LOGE("data is null");
		return -EINVAL;
	}

	msg = (struct wake_up_err_msg *)data;
	if (msg->type >= WAKEUP_ERR_TYPE_BUTT) {
		AUDIO_LOGE("wakeup err type overflow, type:%d", msg->type);
		return -EINVAL;
	}

	AUDIO_LOGW("wakeup err type:%s,content cmp: %d, content in: %d",
		wakeup_err_type_tab[msg->type], msg->content_cmp, msg->content_in);

	return 0;
}

int da_combine_dsp_beta_init(void)
{
	priv_beta = kzalloc(sizeof(*priv_beta), GFP_KERNEL);
	if (priv_beta == NULL) {
		AUDIO_LOGE("failed to kzalloc priv beta");
		return -ENOMEM;
	}

	do_gettimeofday(&priv_beta->btn_upload_time);
	priv_beta->virtual_btn_create_dir = true;

	return 0;
}

void da_combine_dsp_beta_deinit(void)
{
	kfree(priv_beta);
	priv_beta = NULL;
}

