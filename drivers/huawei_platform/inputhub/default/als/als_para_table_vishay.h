/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table vishay header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_VISHAY_H__
#define __ALS_PARA_TABLE_VISHAY_H__

#include "als_para_table.h"

vcnl36658_als_para_table *als_get_vcnl36658_table_by_id(uint32_t id);
vcnl36658_als_para_table *als_get_vcnl36658_first_table(void);
uint32_t als_get_vcnl36658_table_count(void);
als_para_normal_table *als_get_vcnl36832_table_by_id(uint32_t id);
als_para_normal_table *als_get_vcnl36832_first_table(void);
uint32_t als_get_vcnl36832_table_count(void);
als_para_normal_table *als_get_veml32185_table_by_id(uint32_t id);
als_para_normal_table *als_get_veml32185_first_table(void);
uint32_t als_get_veml32185_table_count(void);

#endif
