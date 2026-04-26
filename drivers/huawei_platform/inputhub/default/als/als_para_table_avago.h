/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table avago header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_AVAGO_H__
#define __ALS_PARA_TABLE_AVAGO_H__

#include "als_para_table.h"

apds9251_als_para_table *als_get_apds9251_table_by_id(uint32_t id);
apds9251_als_para_table *als_get_apds9251_first_table(void);
uint32_t als_get_apds9251_table_count(void);
apds9999_als_para_table *als_get_apds9999_table_by_id(uint32_t id);
apds9999_als_para_table *als_get_apds9999_first_table(void);
uint32_t als_get_apds9999_table_count(void);

#endif
