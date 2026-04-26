/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams tcs3718 header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_AMS_TCS3718_H__
#define __ALS_PARA_TABLE_AMS_TCS3718_H__

#include "als_para_table.h"

tcs3701_als_para_table *als_get_tcs3718_table_by_id(uint32_t id);
tcs3701_als_para_table *als_get_tcs3718_first_table(void);
uint32_t als_get_tcs3718_table_count(void);

#endif
