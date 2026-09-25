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
		logw("Smart PA dft module assert\n"); \
		return; \
	} \
} while (0)

#define SOC_SMARTPA_ERR_BASE_ID          916000100
#define SOC_SMARTPA_ERR_INFO_MAX_LEN     64
#define MLIB_SMARTPA_DFT_ERRINFO_MAX_LEN 96
#define IMEDIA_SMARTPA_MAX_CHANNEL       4
#define SMARTPAKIT_NAME_MAX              64
#define SMARTPA_CHANNEL_STEREO           2
#define SMARTPA_DFT_UPLOAD_CHIP_NAME_MAX_LENGTH 16
#define MSG_SIZE_64_BYTES                64
#define MSG_SIZE_128_BYTES               128
#define HIFI_TIMESTAMP_CYCLES_PER_SECOND 32768

enum
{
	SMARTPA_DFT_PROCESS_ERROR_ID = 1,
	SMARTPA_DFT_F0_ERROR_ID = 2,
	SMARTPA_DFT_RESISTANCE_ERROR_ID = 3,
	SMARTPA_DFT_OVERHEAT_ERROR_ID = 4,
};


enum {
	SMARTPA_OM_ERR_TYPE_PROC = 1,  // imonitor required this start from 1
	SMARTPA_OM_ERR_TYPE_PARA_SET,
	SMARTPA_OM_ERR_TYPE_MALLOC,
	SMARTPA_OM_ERR_TYPE_STATUS
};

typedef enum {
	ERR_LEVEL_INFO,
	ERR_LEVEL_WARNING,
	ERR_LEVEL_ERROR,
	ERR_LEVEL_CRITICAL,
	ERR_LEVEL_BUTT,
} error_level;

enum {
	DFT_PROCESS_ERROR_REPORT_MSG_DATA_SET_SIZE = 10,
	DFT_F0_ERROR_REPORT_MSG_DATA_SET_SIZE = 10,
	DFT_RESISTANCE_ERROR_REPORT_MSG_DATA_SET_SIZE = 10,
	DFT_OVERHEAT_ERROR_REPORT_MSG_DATA_SET_SIZE = 5,
};

typedef enum {
	DFT_UPLOAD_PROCESS_ERROR = 1,
	DFT_UPLOAD_F0_ERROR,
	DFT_UPLOAD_RESISTANCE_ERROR,
	DFT_UPLOAD_OVERHEAT_ERROR,
	DFT_UPLOAD_ALL,
} smartpa_dft_upload_function_select;

typedef struct {
	unsigned int timestamp;
	short error_code;
	unsigned char scene_id;
} dft_process_error_data_set;

typedef struct {
	unsigned int timestamp_at_upload;
	char chip_name[SMARTPA_DFT_UPLOAD_CHIP_NAME_MAX_LENGTH];
	dft_process_error_data_set data_set[DFT_PROCESS_ERROR_REPORT_MSG_DATA_SET_SIZE];
} dft_process_error_report_data;

typedef struct {
	bool is_upload_pending;
	unsigned short upload_count;
	unsigned int err_module;
	unsigned int err_level;
	int err_code;
	unsigned short data_set_index;
	dft_process_error_report_data raw_data;
} smartpa_dft_data_process_error;

typedef struct {
	unsigned int timestamp;
	unsigned char channel_index;
	unsigned char scene_id;
	unsigned short f0_value;
} dft_f0_error_data_set;

typedef struct {
	unsigned int timestamp_at_upload;
	char chip_name[SMARTPA_DFT_UPLOAD_CHIP_NAME_MAX_LENGTH];
	dft_f0_error_data_set data_set[DFT_F0_ERROR_REPORT_MSG_DATA_SET_SIZE];
	unsigned char box_vendor;
} dft_f0_error_report_data;

typedef struct {
	bool is_upload_pending;
	unsigned short upload_count;
	unsigned int err_level;
	int err_code;
	unsigned short data_set_index;
	dft_f0_error_report_data raw_data;
} smartpa_dft_data_f0_error;

typedef struct {
	unsigned int timestamp;
	unsigned char channel_index;
	unsigned char scene_id;
	unsigned short resistance_value;
	unsigned short calibration_value;
} dft_resistance_error_data_set;

typedef struct {
	unsigned int timestamp_at_upload;
	char chip_name[SMARTPA_DFT_UPLOAD_CHIP_NAME_MAX_LENGTH];
	unsigned char box_vendor;
	dft_resistance_error_data_set data_set[DFT_RESISTANCE_ERROR_REPORT_MSG_DATA_SET_SIZE];
} dft_resistance_error_report_data;

typedef struct {
	bool is_upload_pending;
	unsigned short upload_count;
	unsigned int err_level;
	int err_code;
	unsigned short data_set_index;
	dft_resistance_error_report_data raw_data;
} smartpa_dft_data_resistance_error;

typedef struct {
	unsigned int timestamp;
	unsigned char channel_index;
	unsigned char scene_id;
	short temperature[SMARTPA_CHANNEL_STEREO];
	short current_gain[SMARTPA_CHANNEL_STEREO];
	short calibration_value[SMARTPA_CHANNEL_STEREO];
} dft_overheat_error_data_set;

typedef struct {
	unsigned int timestamp_at_upload;
	char chip_name[SMARTPA_DFT_UPLOAD_CHIP_NAME_MAX_LENGTH];
	unsigned char box_vendor;
	dft_overheat_error_data_set data_set[DFT_OVERHEAT_ERROR_REPORT_MSG_DATA_SET_SIZE];
} dft_overheat_error_report_data;

typedef struct {
	bool is_upload_pending;
	unsigned short upload_count;
	unsigned int err_level;
	int err_code;
	unsigned short data_set_index;
	dft_overheat_error_report_data raw_data;
} smartpa_dft_data_overheat_error;

void smartpa_dft_report_process_error(const char*);
void smartpa_dft_report_f0_error(const char*);
void smartpa_dft_report_resistance_error(const char*);
void smartpa_dft_report_overheat_error(const char*);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


