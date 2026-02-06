/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: implement hisee image upgrade function
 * Create: 2020-02-17
 */
#ifndef HISEE_UPGRADE_H
#define HISEE_UPGRADE_H
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include "hisee.h"
#include "hisee_fs.h"

#define HISEE_OLD_COS_IMAGE_ERROR        (-8000)
#define HISEE_IS_OLD_COS_IMAGE           (-8001)

#define HISEE_MISC_VERSION0              0x20
/* version=0 indicate there is not enable cos image upgrade sw version */
#define HISEE_DEFAULT_SW_UPGRADE_VERSION 0x0

#define HISEE_COS_UPGRADE_RUNNING_FLG   0x87654321u
#define HISEE_COS_UPGRADE_FINISH_FLG    0x0u

int cos_image_upgrade_func(const void *buf, int para);
int handle_cos_image_upgrade(const void *buf, int para);
int misc_image_upgrade_func(unsigned int cos_id);
int hisee_upgrade_image_read(unsigned int cos_id,
			     enum hisee_img_file_type img_type,
			     enum se_smc_cmd smc_cmd);
int cos_upgrade_image_read(unsigned int cos_id,
			   enum hisee_img_file_type img_type);

#ifdef CONFIG_HICOS_MISCIMG_PATCH
int hisee_cos_patch_read(enum hisee_img_file_type img_type);
#endif

int check_new_cosimage(unsigned int cos_id, int *is_new_cosimage);
ssize_t hisee_has_new_cos_show(struct device *dev,
			       struct device_attribute *attr, char *buf);
ssize_t hisee_check_upgrade_show(struct device *dev,
				 struct device_attribute *attr, char *buf);

#endif
