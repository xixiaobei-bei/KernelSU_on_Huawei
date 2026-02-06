/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox route header file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#ifndef __CAP_PROX_ROUTE_H__
#define __CAP_PROX_ROUTE_H__

#include "cap_prox_detect.h"

ssize_t attr_cap_prox_data_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_cap_prox_abov_data_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_abov_bootloader_verify(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_abov_bootloader_verify_result_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t attr_abov_bootloader_enter(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t attr_abov_bootloader_enter_show(struct device *dev,
	struct device_attribute *attr, char *buf);
ssize_t abov_reg_dump_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);
ssize_t abov_reg_dump_show(struct device *dev,
	struct device_attribute *attr, char *buf);

#endif
