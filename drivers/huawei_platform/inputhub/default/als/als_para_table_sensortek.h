/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sensortek header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_SENSORTEK_H__
#define __ALS_PARA_TABLE_SENSORTEK_H__

#include "als_para_table.h"

als_para_normal_table *als_get_stk3338_table_by_id(uint32_t id);
als_para_normal_table *als_get_stk3338_first_table(void);
uint32_t als_get_stk3338_table_count(void);
als_para_normal_table *als_get_stk32670_table_by_id(uint32_t id);
als_para_normal_table *als_get_stk32670_first_table(void);
uint32_t als_get_stk32670_table_count(void);
als_para_normal_table *als_get_stk32671_table_by_id(uint32_t id);
als_para_normal_table *als_get_stk32671_first_table(void);
uint32_t als_get_stk32671_table_count(void);

#endif
