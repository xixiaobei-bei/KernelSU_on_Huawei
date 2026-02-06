/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table silergy header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_SILERGY_H__
#define __ALS_PARA_TABLE_SILERGY_H__

#include "als_detect.h"

#define SYH399_PARA_SIZE 17

sy3079_als_para_table *als_get_sy3079_table_by_id(uint32_t id);
sy3079_als_para_table *als_get_sy3079_first_table(void);
uint32_t als_get_sy3079_table_count(void);
als_para_normal_table *als_get_syh399_table_by_id(uint32_t id);
als_para_normal_table *als_get_syh399_first_table(void);
uint32_t als_get_syh399_table_count(void);
#endif
