/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table liteon header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_LITEON_H__
#define __ALS_PARA_TABLE_LITEON_H__

#include "als_para_table.h"

ltr582_als_para_table *als_get_ltr582_table_by_id(uint32_t id);
ltr582_als_para_table *als_get_ltr582_first_table(void);
uint32_t als_get_ltr582_table_count(void);
pinhole_als_para_table *als_get_pinhole_table_by_id(uint32_t id);
pinhole_als_para_table *als_get_pinhole_first_table(void);
uint32_t als_get_pinhole_table_count(void);
als_para_normal_table *als_get_ltr2568_table_by_id(uint32_t id);
als_para_normal_table *als_get_ltr2568_first_table(void);
uint32_t als_get_ltr2568_table_count(void);
ltr2594_als_para_table *als_get_ltr2594_table_by_id(uint32_t id);
ltr2594_als_para_table *als_get_ltr2594_first_table(void);
uint32_t als_get_ltr2594_table_count(void);
als_para_normal_table *als_get_ltr311_table_by_id(uint32_t id);
als_para_normal_table *als_get_ltr311_first_table(void);
uint32_t als_get_ltr311_table_count(void);

#endif
