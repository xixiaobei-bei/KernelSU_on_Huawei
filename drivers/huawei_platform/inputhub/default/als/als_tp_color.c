/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para table ams source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */
#include "als_tp_color.h"

#include <linux/err.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_para_table.h"
#include "contexthub_route.h"

#define TP_COLOR_NV_NUM 16
#define TP_COLOR_NV_SIZE 15

static void read_tpcolor_select_als_data(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	if (dev_info->chip_type == ALS_CHIP_ROHM_RGB)
		select_rohm_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AVAGO_RGB)
		select_avago_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_BH1749)
		select_rohmbh1749_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_BU27006MUC)
		select_rohmbu27006muc_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3701_RGB)
		select_ams_tcs3701_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3708_RGB)
		select_ams_tcs3708_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3718_RGB)
		select_ams_tcs3718_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3707_RGB)
		select_ams_tcs3707_als_data(pf_data, dev_info);
	else if (dev_info->chip_type == ALS_CHIP_VD6281)
		set_vd6281_als_extend_prameters(pf_data, dev_info);
}

static int read_tpcolor_from_nv(struct als_platform_data *pf_data,
	struct als_device_info *dev_info)
{
	int ret;
	char nv_tp_color[TP_COLOR_NV_SIZE + 1] = "";
	struct hisi_nve_info_user user_info = { 0 };

	if (!pf_data || !dev_info)
		return -1;

	user_info.nv_operation = NV_READ_TAG;
	user_info.nv_number = TP_COLOR_NV_NUM;
	user_info.valid_size = TP_COLOR_NV_SIZE;
	if (memcpy_s(user_info.nv_name, sizeof(user_info.nv_name),
		"TPCOLOR", sizeof("TPCOLOR")) != EOK) {
		hwlog_err("%s memcpy_s nv_name error\n", __func__);
		return -1;
	}
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("nve_direct_access read error %d\n", ret);
		return -1;
	}
	if (memcpy_s(nv_tp_color, sizeof(nv_tp_color),
		user_info.nv_data, sizeof(nv_tp_color) - 1) != EOK)
		return -1;

	if (strstr(nv_tp_color, "white"))
		set_phone_color(WHITE);
	else if (strstr(nv_tp_color, "black"))
		set_phone_color(BLACK);
	else if (strstr(nv_tp_color, "gold"))
		set_phone_color(GOLD);
	else
		hwlog_info("other colors\n");

	hwlog_info("phone_color=0x%x, nv_tp_color=%s\n",
		get_phone_color(), nv_tp_color);

	pf_data->als_phone_tp_colour = get_phone_color();
	read_tpcolor_select_als_data(pf_data, dev_info);
	return 0;
}

int get_tpcolor_from_nv(int32_t tag)
{
	int ret;
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
	if (!pf_data || !dev_info)
		return -1;

	if (dev_info->tp_color_from_nv_flag) {
		hwlog_info(" %s: tp_color_from_nv_flag = %d\n",
			__func__, dev_info->tp_color_from_nv_flag);
		ret = read_tpcolor_from_nv(pf_data, dev_info);
		if (ret != 0) {
			hwlog_err("%s: read tp_color from NV fail\n", __func__);
			return -1;
		}
		resend_als_parameters_to_mcu(tag);
	}

	return 0;
}

