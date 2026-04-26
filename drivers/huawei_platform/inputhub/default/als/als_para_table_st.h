/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table st header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_ST_H__
#define __ALS_PARA_TABLE_ST_H__

#include "als_para_table.h"

vd6281_als_para_table *als_get_vd6281_table_by_id(uint32_t id);
vd6281_als_para_table *als_get_vd6281_first_table(void);
uint32_t als_get_vd6281_table_count(void);

#endif
