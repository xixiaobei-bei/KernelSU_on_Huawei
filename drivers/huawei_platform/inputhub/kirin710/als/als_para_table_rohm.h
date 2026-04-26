/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table rohm header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_ROHM_H__
#define __ALS_PARA_TABLE_ROHM_H__

#include "als_detect.h"

bh1745_als_para_table *als_get_bh1745_table_by_id(uint32_t id);
bh1745_als_para_table *als_get_bh1745_first_table(void);
uint32_t als_get_bh1745_table_count(void);
rpr531_als_para_table *als_get_rpr531_table_by_id(uint32_t id);
rpr531_als_para_table *als_get_rpr531_first_table(void);
uint32_t als_get_rpr531_table_count(void);
bh1726_als_para_table *als_get_bh1726_table_by_id(uint32_t id);
bh1726_als_para_table *als_get_bh1726_first_table(void);
uint32_t als_get_bh1726_table_count(void);

#endif
