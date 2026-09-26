/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_PARA_TABLE_EMIN_MN78911_H__
#define __ALS_PARA_TABLE_EMIN_MN78911_H__

#include "als_detect.h"

mn78911_als_para_table *als_get_mn78911_table_by_id(uint32_t id);
mn78911_als_para_table *als_get_mn78911_first_table(void);
uint32_t als_get_mn78911_table_count(void);

#endif	//__ALS_PARA_TABLE_EMIN_MN78911_H__
