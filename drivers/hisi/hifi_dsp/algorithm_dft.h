/*
 * Audio Algorithm DFT Module
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ALGORITHM_DFT_H__
#define __ALGORITHM_DFT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SMARTPA_DFT_ASSERT(bool_expression) \
do { \
	if (!(bool_expression)) { \
		logw("Smart PA dft module assert, %s\n", #bool_expression); \
		return; \
	} \
} while (0)

#define IMEDIA_SMARTPA_MAX_CHANNEL       4
#define SMARTPA_CHANNEL_STEREO           2
#define SMARTPA_DFT_UPLOAD_CHIP_NAME_MAX_LENGTH 16
#define MSG_SIZE_65_BYTES                65
#define EVENT_MODULE_LEN                 16
#define ERR_POS_LEN                      16
#define BITS_TO_PRINT_EACH_NUMBER        4

enum smartpa_dft_id {
	DFT_ID_BASE = 916000100,
	DFT_ID_PROCESS_ERROR = 916000101,
	DFT_ID_FRES_ERROR = 916000102,
	DFT_ID_RDC_ERROR = 916000103,
	DFT_ID_TEMP_ERROR = 916000104,
	DFT_ID_AUDIO_DB = 916000105,
	DFT_ID_BUTT,
};

struct smartpa_dft_process_error {
	enum smartpa_dft_id dft_id;
	int32_t err_code;
	uint16_t event_module[EVENT_MODULE_LEN];
};

struct smartpa_dft_gain_protection {
	enum smartpa_dft_id dft_id;
	int32_t err_code;
	uint16_t event_module[EVENT_MODULE_LEN];
	uint16_t err_pos[ERR_POS_LEN];
};

void hifi_om_work_smartpa_dft_report(const enum smartpa_dft_id *data);
void hifi_om_work_audio_db(const enum smartpa_dft_id *data);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


