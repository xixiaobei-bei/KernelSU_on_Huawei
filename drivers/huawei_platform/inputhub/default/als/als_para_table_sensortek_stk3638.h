/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table sensortek stk3638 header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_SENSORTEK_STK3638_H__
#define __ALS_PARA_TABLE_SENSORTEK_STK3638_H__

#include "als_para_table.h"

stk3638_als_para_table *als_get_stk3638_table_by_id(uint32_t id);
stk3638_als_para_table *als_get_stk3638_first_table(void);
uint32_t als_get_stk3638_table_count(void);

#endif
