/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_AMS_H__
#define __ALS_PARA_TABLE_AMS_H__

#include "als_para_table.h"

tcs3701_als_para_table *als_get_tcs3708_table_by_id(uint32_t id);
tcs3701_als_para_table *als_get_tcs3708_first_table(void);
uint32_t als_get_tcs3708_table_count(void);
tmd2745_als_para_table *als_get_tmd2745_table_by_id(uint32_t id);
tmd2745_als_para_table *als_get_tmd2745_first_table(void);
uint32_t als_get_tmd2745_table_count(void);
tcs3707_als_para_table *als_get_tcs3707_table_by_id(uint32_t id);
tcs3707_als_para_table *als_get_tcs3707_first_table(void);
uint32_t als_get_tcs3707_table_count(void);

#endif
