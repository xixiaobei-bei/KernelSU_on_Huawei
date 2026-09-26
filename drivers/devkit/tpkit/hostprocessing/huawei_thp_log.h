/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#ifndef _INPUT_THP_LOG_H_
#define _INPUT_THP_LOG_H_

#define THP_ROOT_UID 0
#define THP_SYSTEM_GID 1000
#define SH_THP_PATH_MAXLEN 128
#define SH_THP_CUR_PATH_MAXLEN 64
#define SH_THP_DIR_LIMIT 0770
#define SH_THP_FILE_AUTHORITY 0660
#define SH_THP_LOG_DIR "/data/log/sensorhub-log/"
#define SH_THP_LOG_FILE "/data/log/sensorhub-log/thplog.log"

#define THP_DAE_LOG_BUFF_LENGTH (1 * 1024)
#define THP_AFE_LOG_BUFF_LENGTH (1 * 1024)
#define THP_TSA_LOG_BUFF_LENGTH (2 * 1024)
#define THP_RAW_LOG_BUFF_LENGTH 2000
#define THP_DIF_LOG_BUFF_LENGTH 2000
#define THP_FRAME_DATA_LENGTH 2000
#define THP_LOG_RAW_DATA_LENGTH_MAX (5 * 1024)
#define THP_LOG_DIF_DATA_LENGTH_MAX (5 * 1024)

#define THP_LOG_AVAILABLE_DATA_LENGTH 16
#define THP_FS_WAIT_TIME 2

struct pkt_thp_frame_data_format_t {
	unsigned short col;
	unsigned short row;
	unsigned int len;
	unsigned char data[THP_FRAME_DATA_LENGTH];
};

struct pkt_thplog_report_data_t {
	unsigned char dae_log[THP_DAE_LOG_BUFF_LENGTH];
	unsigned int dae_logbuff_len;
	unsigned char afe_log[THP_AFE_LOG_BUFF_LENGTH];
	unsigned int afe_logbuff_len;
	unsigned char tsa_log[THP_TSA_LOG_BUFF_LENGTH];
	unsigned int tsa_logbuff_len;

	unsigned char frame_head[THP_FRAME_DATA_LENGTH];
	unsigned int frame_head_len;

	struct pkt_thp_frame_data_format_t raw_data;
	struct pkt_thp_frame_data_format_t dif_data;
};

struct thp_log_save {
	struct pkt_thplog_report_data_t *data;
	struct semaphore sem;
};

extern int thp_log_init(void);

#endif /* _INPUT_THP_LOG_H_ */

