/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __ALS_ROUTE_H__
#define __ALS_ROUTE_H__

#include "als_detect.h"

#define BLOCK_MID 45
#define get_reduce_data(a, b)           (((a) > (b)) ? ((a) - (b)) : 0)

ssize_t als_ud_rgbl_status_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_rgb_status_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t als_ud_rgbl_block_data_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_sensorlist_info_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_offset_data_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_always_on_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
ssize_t als_mmi_para_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_calibrate_under_tp_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t als_calibrate_under_tp_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_rgb_data_under_tp_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t als_rgb_data_under_tp_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_under_tp_calidata_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t als_under_tp_calidata_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t als_calibrate_after_sale_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf);

int send_als_ud_data_to_mcu(int32_t tag, uint32_t subcmd, const void *data,
	int length, bool is_recovery);
void send_dc_status_to_sensorhub(uint32_t dc_status);
void als_save_light_to_sensorhub(uint32_t mipi_level, uint32_t bl_level);
void wake_up_als_ud_block(void);
int als_ud_reset_immediately(int32_t tag);
int als_underscreen_calidata_save(int32_t tag);
void get_als_calibrate_data_by_tag(int32_t tag);
void get_als_under_tp_calibrate_data_by_tag(int32_t tag);
uint32_t als_ud_coef_block_calc_x(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);
uint32_t als_ud_coef_block_calc_y(struct als_platform_data *pf_data,
	struct als_device_info *dev_info);

#endif
