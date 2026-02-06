/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als channel header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_CHANNEL_H__
#define __ALS_CHANNEL_H__

#include "als_detect.h"

int send_laya_als_calibrate_data_to_mcu(int32_t tag);
int send_als_calibrate_data_to_mcu(int32_t tag);
void als_ud_minus_dss_noise_send(int32_t tag,
	struct als_platform_data *pf_data, struct als_device_info *dev_info);
int als_under_tp_nv_read(int32_t tag);
int send_als_under_tp_calibrate_data_to_mcu(int32_t tag);
int als_get_calidata_nv_addr(int32_t tag, uint32_t *addr);
int als_get_sensor_id_by_tag(int32_t tag, uint32_t *s_id);
int als_get_tag_by_sensor_id(uint32_t s_id, int32_t *tag);
int als_flag_result_cali(struct als_device_info *dev_info);
void reset_als_calibrate_data(void);
void reset_als1_calibrate_data(void);
void reset_als2_calibrate_data(void);

void select_als_para_pinhole(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_als_para_rpr531(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_als_para_tmd2745(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
void select_als_para_normal(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);

void select_als_para(struct als_platform_data *pf_data,
	struct als_device_info *dev_info,
	struct device_node *dn);
void als_on_read_tp_module(unsigned long action, void *data);

#endif
