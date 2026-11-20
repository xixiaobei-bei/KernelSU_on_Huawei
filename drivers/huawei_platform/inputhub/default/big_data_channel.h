/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: big data channel header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef _BIG_DATA_CHANNEL_H_
#define _BIG_DATA_CHANNEL_H_

#include <linux/types.h>

typedef enum {
	INT_PARAM,
	STR_PARAM,
} big_data_param_type_t;

typedef enum {
	BIG_DATA_STR,
} big_data_str_tag_t;

typedef struct {
	const char *param_name;
	big_data_param_type_t param_type;
	int tag;
} big_data_param_detail_t;

typedef struct {
	int event_id;
	int param_num;
	big_data_param_detail_t *param_data;
} big_data_event_detail_t;

int iomcu_dubai_log_fetch(uint32_t event_type, void *data, uint32_t length);

#endif /* _BIG_DATA_CHANNEL_H_ */
