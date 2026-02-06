/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sensortek header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_SENSORTEK_H__
#define __ALS_PARA_TABLE_SENSORTEK_H__

#include "als_detect.h"

stk3321_als_para_table *als_get_stk3321_table_by_id(uint32_t id);
stk3321_als_para_table *als_get_stk3321_first_table(void);
uint32_t als_get_stk3321_table_count(void);
stk3235_als_para_table *als_get_stk3235_table_by_id(uint32_t id);
stk3235_als_para_table *als_get_stk3235_first_table(void);
uint32_t als_get_stk3235_table_count(void);
als_para_normal_table *als_get_stk3338_table_by_id(uint32_t id);
als_para_normal_table *als_get_stk3338_first_table(void);
uint32_t als_get_stk3338_table_count(void);

#endif
