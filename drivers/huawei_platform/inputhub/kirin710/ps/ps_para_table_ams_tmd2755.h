/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __PS_PARA_TABLE_AMS_TMD2755_H__
#define __PS_PARA_TABLE_AMS_TMD2755_H__

#include "sensor_config.h"

enum {
	TMD2755_PS_PARA_PPULSES_IDX = 0,
	TMD2755_PS_PARA_BINSRCH_TARGET_IDX,
	TMD2755_PS_PARA_THRESHOLD_L_IDX,
	TMD2755_PS_PARA_THRESHOLD_H_IDX,
	TMD2755_PS_PARA_BUTT,
};

tmd2755_ps_para_table *ps_get_tmd2755_table_by_id(uint32_t id);
tmd2755_ps_para_table *ps_get_tmd2755_first_table(void);
uint32_t ps_get_tmd2755_table_count(void);

#endif
