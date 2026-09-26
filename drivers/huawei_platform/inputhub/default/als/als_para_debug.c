/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als para debug source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_para_debug.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "als_para_table.h"
#include "als_para_table_ams.h"
#include "als_para_table_ams_tcs3701.h"
#include "als_para_table_ams_tmd3702.h"
#include "als_para_table_ams_tmd3725.h"
#include "als_para_table_ams_tsl2591.h"
#include "als_para_table_avago.h"
#include "als_para_table_liteon.h"
#include "als_para_table_rohm.h"
#include "als_para_table_sensortek.h"
#include "als_para_table_vishay.h"
#include "contexthub_route.h"

#define ALS_DBG_PARA_SIZE             8
#define BUF_SIZE                      128

static void show_als_debug_bh745(uint32_t id, short *dbg, uint32_t num)
{
	bh1745_als_para_table *table = NULL;

	table = als_get_bh1745_table_by_id(id);
	if (!table)
		return;

	/* cofficient_red[0] */
	dbg[0] = table->bh745_para[0];
	/* cofficient_red[0] */
	dbg[1] = table->bh745_para[1];
	/* cofficient_green[0] */
	dbg[2] = table->bh745_para[3];
	/* cofficient_red[1] */
	dbg[3] = table->bh745_para[2];
	/* cofficient_green[1] */
	dbg[4] = table->bh745_para[4];
}

static void show_als_debug_apds251(uint32_t id, short *dbg, uint32_t num)
{
	apds9251_als_para_table *table = NULL;

	table = als_get_apds9251_table_by_id(id);
	if (!table)
		return;

	/* apds251_para or apds9253_para */
	/* avago_cofficient[1] */
	dbg[1] = table->apds251_para[9];
	/* avago_cofficient[2] */
	dbg[2] = table->apds251_para[10];
	/* avago_cofficient[3] */
	dbg[3] = table->apds251_para[20];
	/* LUX_P */
	dbg[4] = table->apds251_para[2];
	/* LUX_R */
	dbg[5] = table->apds251_para[4];
	/* LUX_Q */
	dbg[6] = table->apds251_para[3];
	/* lux_mix */
	dbg[7] = table->apds251_para[19];
}

static void show_als_debug_apds9999(uint32_t id, short *dbg, uint32_t num)
{
	apds9999_als_para_table *table = NULL;

	table = als_get_apds9999_table_by_id(id);
	if (!table)
		return;

	/* avago_cofficient[1] */
	dbg[1] = table->apds9999_para[9];
	/* avago_cofficient[2] */
	dbg[2] = table->apds9999_para[10];
	/* avago_cofficient[3] */
	dbg[3] = table->apds9999_para[20];
	/* LUX_P */
	dbg[4] = table->apds9999_para[2];
	/* LUX_R */
	dbg[5] = table->apds9999_para[4];
	/* LUX_Q */
	dbg[6] = table->apds9999_para[3];
	/* lux_mix */
	dbg[7] = table->apds9999_para[19];
}

static void show_als_debug_tmd3725(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	tmd3725_als_para_table *table = NULL;

	table = als_get_tmd3725_table_by_id(id);
	if (!table)
		return;

	/*
	 * tmd3725_para:
	 * 0: atime
	 * 1: again
	 * 2: dgf
	 * 3: c_coef
	 * 4: r_coef
	 * 5: g_coef
	 * 6: b_coef
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		dbg[1 + k] = table->tmd3725_para[k];
}

static void show_als_debug_tmd3702(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	tmd3702_als_para_table *table = NULL;

	table = als_get_tmd3702_table_by_id(id);
	if (!table)
		return;

	/*
	 * tmd3702_para:
	 * 0: atime
	 * 1: again
	 * 2: dgf
	 * 3: c_coef
	 * 4: r_coef
	 * 5: g_coef
	 * 6: b_coef
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		dbg[1 + k] = table->tmd3702_para[k];
}

static void show_als_debug_tcs3707(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	tcs3707_als_para_table *table = NULL;

	table = als_get_tcs3707_table_by_id(id);
	if (!table)
		return;

	/*
	 * tcs3707_para:
	 * 0: atime
	 * 1: again
	 * 2: dgf
	 * 3: c_coef
	 * 4: r_coef
	 * 5: g_coef
	 * 6: b_coef
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		dbg[1 + k] = table->tcs3707_para[k];
}

static void show_als_debug_tcs3701(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	tcs3701_als_para_table *table = NULL;

	table = als_get_tcs3701_table_by_id(id);
	if (!table)
		return;

	/*
	 * tcs3701_para:
	 * 0: atime
	 * 1: again
	 * 2: dgf
	 * 3: c_coef
	 * 4: r_coef
	 * 5: g_coef
	 * 6: b_coef
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		dbg[1 + k] = table->tcs3701_para[k];
}

static void show_als_debug_vcnl36658(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	vcnl36658_als_para_table *table = NULL;

	table = als_get_vcnl36658_table_by_id(id);
	if (!table)
		return;

	for (k = 0; k < 7; k++) /* 7 parameters */
		dbg[1 + k] = table->vcnl36658_para[k];
}

static void show_als_debug_ltr582(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	ltr582_als_para_table *table = NULL;

	table = als_get_ltr582_table_by_id(id);
	if (!table)
		return;

	/*
	 * ltr582_para:
	 * 3: ad_radio
	 * 4: dc_radio
	 * 5: a_winfac
	 * 6: d_winfac
	 * 7: c_winfac
	 * 8: slope
	 * 9: slope_offset
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		dbg[1 + k] = table->ltr582_para[3 + k]; /* from 3 */
}

static void show_als_debug_tmd2745(uint32_t id, short *dbg, uint32_t num)
{
	uint32_t k;
	tmd2745_als_para_table *table = NULL;

	table = als_get_tmd2745_table_by_id(id);
	if (!table)
		return;

	/*
	 * als_para:
	 * 0: D_factor
	 * 1: B_Coef
	 * 2: C_Coef
	 * 3: D_Coef
	 * 4: is_min_algo
	 * 5: is_auto_gain
	 */
	for (k = 0; k < 6; k++) /* 6 parameters */
		dbg[k] = table->als_para[k];
}

static void show_als_debug_stk32670(uint32_t id, short *dbg, uint32_t num)
{
	int8_t i;
	als_para_normal_table *table = NULL;

	table = als_get_stk32670_table_by_id(id);
	if (!table)
		return;

	for (i = 0; i < num; i++)
		dbg[i] = table->als_para[i + 1];
}

ssize_t als_debug_data_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;
	uint32_t id;
	short dbg[ALS_DBG_PARA_SIZE] = { 0 }; /* als debug para */
	uint32_t num = ARRAY_SIZE(dbg);

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
	if (!pf_data || !dev_info || (dev_info->chip_type == ALS_CHIP_NONE))
		return -1;

	if (!buf)
		return 0;

	id = dev_info->table_id;

	hwlog_info("%s chip_type=%d\n", __func__, dev_info->chip_type);

	if (dev_info->chip_type == ALS_CHIP_ROHM_RGB) /* bh745_para */
		show_als_debug_bh745(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_AVAGO_RGB ||
		dev_info->chip_type == ALS_CHIP_APDS9253_RGB)
		show_als_debug_apds251(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_APDS9999_RGB)
		show_als_debug_apds9999(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TMD3725_RGB)
		show_als_debug_tmd3725(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TMD3702_RGB)
		show_als_debug_tmd3702(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3707_RGB)
		show_als_debug_tcs3707(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_AMS_TCS3701_RGB)
		show_als_debug_tcs3701(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_VISHAY_VCNL36658)
		show_als_debug_vcnl36658(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_LITEON_LTR582)
		show_als_debug_ltr582(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_TMD2745)
		show_als_debug_tmd2745(id, dbg, num);
	else if (dev_info->chip_type == ALS_CHIP_STK32670)
		show_als_debug_stk32670(id, dbg, num);

	return snprintf_s(buf, BUF_SIZE, BUF_SIZE - 1,
		"%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd\n",
		dbg[0], dbg[1], dbg[2], dbg[3], dbg[4], dbg[5],
		dbg[6], dbg[7]);
}

static ssize_t store_als_debug_bh745(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	bh1745_als_para_table *table = NULL;

	table = als_get_bh1745_table_by_id(id);
	if (!table)
		return -1;

	/* cofficient_judge */
	table->bh745_para[0] = dbg[0];
	/* cofficient_red[0] */
	table->bh745_para[1] = dbg[1];
	/* cofficient_green[0] */
	table->bh745_para[3] = dbg[2];
	/* cofficient_red[1] */
	table->bh745_para[2] = dbg[3];
	/* cofficient_green[1] */
	table->bh745_para[4] = dbg[4];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->bh745_para, sizeof(table->bh745_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_apds251(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	apds9251_als_para_table *table = NULL;

	table = als_get_apds9251_table_by_id(id);
	if (!table)
		return -1;

	/* avago_cofficient[1] */
	table->apds251_para[9] = dbg[1];
	/* avago_cofficient[2] */
	table->apds251_para[10] = dbg[2];
	/* avago_cofficient[3] */
	table->apds251_para[20] = dbg[3];
	/* LUX_P */
	table->apds251_para[2] = dbg[4];
	/* LUX_R */
	table->apds251_para[4] = dbg[5];
	/* LUX_Q */
	table->apds251_para[3] = dbg[6];
	/* lux_mix */
	table->apds251_para[19] = dbg[7];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->apds251_para, sizeof(table->apds251_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_apds9999(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	apds9999_als_para_table *table = NULL;

	table = als_get_apds9999_table_by_id(id);
	if (!table)
		return -1;

	/* avago_cofficient[1] */
	table->apds9999_para[9] = dbg[1];
	/* avago_cofficient[2] */
	table->apds9999_para[10] = dbg[2];
	/* avago_cofficient[3] */
	table->apds9999_para[20] = dbg[3];
	/* LUX_P */
	table->apds9999_para[2] = dbg[4];
	/* LUX_R */
	table->apds9999_para[4] = dbg[5];
	/* LUX_Q */
	table->apds9999_para[3] = dbg[6];
	/* lux_mix */
	table->apds9999_para[19] = dbg[7];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->apds9999_para, sizeof(table->apds9999_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_tmd3725(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	uint32_t k;
	tmd3725_als_para_table *table = NULL;

	table = als_get_tmd3725_table_by_id(id);
	if (!table)
		return -1;

	/*
	 * tmd3725_para:
	 * 0: atime
	 * 1: again
	 * 2: dgf
	 * 3: c_coef
	 * 4: r_coef
	 * 5: g_coef
	 * 6: b_coef
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		table->tmd3725_para[k] = dbg[1 + k];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->tmd3725_para, sizeof(table->tmd3725_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_tmd3702(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	uint32_t k;
	tmd3702_als_para_table *table = NULL;

	table = als_get_tmd3702_table_by_id(id);
	if (!table)
		return -1;

	/*
	 * tmd3702_para:
	 * 0: atime
	 * 1: again
	 * 2: dgf
	 * 3: c_coef
	 * 4: r_coef
	 * 5: g_coef
	 * 6: b_coef
	 */
	for (k = 0; k < 7; k++) /* 7 parameters */
		table->tmd3702_para[k] = dbg[1 + k];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->tmd3702_para, sizeof(table->tmd3702_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_tcs3707(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	tcs3707_als_para_table *table = NULL;

	table = als_get_tcs3707_table_by_id(id);
	if (!table)
		return -1;

	/* atime */
	table->tcs3707_para[0] = dbg[1];
	/* again */
	table->tcs3707_para[1] = dbg[2];
	/* dgf */
	table->tcs3707_para[2] = dbg[3];
	/* c_coef */
	table->tcs3707_para[3] = dbg[4];
	/* r_coef */
	table->tcs3707_para[4] = dbg[5];
	/* g_coef */
	table->tcs3707_para[5] = dbg[6];
	/* b_coef */
	table->tcs3707_para[6] = dbg[7];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->tcs3707_para, sizeof(table->tcs3707_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_tcs3701(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	tcs3701_als_para_table *table = NULL;

	table = als_get_tcs3701_table_by_id(id);
	if (!table)
		return -1;

	/* atime */
	table->tcs3701_para[0] = dbg[1];
	/* again */
	table->tcs3701_para[1] = dbg[2];
	/* dgf */
	table->tcs3701_para[2] = dbg[3];
	/* c_coef */
	table->tcs3701_para[3] = dbg[4];
	/* r_coef */
	table->tcs3701_para[4] = dbg[5];
	/* g_coef */
	table->tcs3701_para[5] = dbg[6];
	/* b_coef */
	table->tcs3701_para[6] = dbg[7];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->tcs3701_para, sizeof(table->tcs3701_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_vcnl36658(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	vcnl36658_als_para_table *table = NULL;

	table = als_get_vcnl36658_table_by_id(id);
	if (!table)
		return -1;

	table->vcnl36658_para[0] = dbg[1];
	table->vcnl36658_para[1] = dbg[2];
	table->vcnl36658_para[2] = dbg[3];
	table->vcnl36658_para[3] = dbg[4];
	table->vcnl36658_para[4] = dbg[5];
	table->vcnl36658_para[5] = dbg[6];
	table->vcnl36658_para[6] = dbg[7];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->vcnl36658_para, sizeof(table->vcnl36658_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_ltr582(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	ltr582_als_para_table *table = NULL;

	table = als_get_ltr582_table_by_id(id);
	if (!table)
		return -1;

	/* ad_radio */
	table->ltr582_para[3] = dbg[1];
	/* dc_radio */
	table->ltr582_para[4] = dbg[2];
	/* a_winfac */
	table->ltr582_para[5] = dbg[3];
	/* d_winfac */
	table->ltr582_para[6] = dbg[4];
	/* c_winfac */
	table->ltr582_para[7] = dbg[5];
	/* slope */
	table->ltr582_para[8] = dbg[6];
	/* slope_offset */
	table->ltr582_para[9] = dbg[7];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->ltr582_para, sizeof(table->ltr582_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_tmd2745(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	tmd2745_als_para_table *table = NULL;

	table = als_get_tmd2745_table_by_id(id);
	if (!table)
		return -1;

	/* D_factor */
	table->als_para[0] = dbg[0];
	/* B_Coef */
	table->als_para[1] = dbg[1];
	/* C_Coef */
	table->als_para[2] = dbg[2];
	/* D_Coef */
	table->als_para[3] = dbg[3];
	/* is_min_algo */
	table->als_para[4] = dbg[4];
	/* is_auto_gain */
	table->als_para[5] = dbg[5];
	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->als_para, sizeof(table->als_para)) != EOK)
		return -1;

	return size;
}

static ssize_t store_als_debug_stk32670(struct als_platform_data *pf_data,
	uint32_t id, const short *dbg, uint32_t num, size_t size)
{
	int8_t i;
	als_para_normal_table *table = NULL;

	table = als_get_stk32670_table_by_id(id);
	if (!table)
		return -1;

	for (i = 0; i < num; i++)
		table->als_para[i + 1] = dbg[i];

	if (sizeof(table->als_para) > SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE)
		return -1;

	if (memcpy_s(pf_data->als_extend_data, sizeof(pf_data->als_extend_data),
		table->als_para, sizeof(table->als_para)) != EOK)
		return -1;

	return size;
}

ssize_t als_debug_data_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;
	uint32_t id;
	short dbg[ALS_DBG_PARA_SIZE] = { 0 }; /* als debug para */
	uint32_t num = ARRAY_SIZE(dbg);

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
	if (!pf_data || !dev_info || (dev_info->chip_type == ALS_CHIP_NONE))
		return -1;

	if (!buf)
		return 0;

	id = dev_info->table_id;
	hwlog_info("%s:chip_type=%d\n", __func__, dev_info->chip_type);

	if (sscanf_s(buf, "%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd", &dbg[0], &dbg[1],
		&dbg[2], &dbg[3], &dbg[4], &dbg[5], &dbg[6],
		&dbg[7]) > 0)
		hwlog_info("%s: get parameter success\n", __func__);
	else
		hwlog_info("%s: get parameter fail\n", __func__);

	switch (dev_info->chip_type) {
	case ALS_CHIP_ROHM_RGB: /* bh745 */
		return store_als_debug_bh745(pf_data, id, dbg, num, size);
	case ALS_CHIP_AVAGO_RGB: /* apds251 */
		return store_als_debug_apds251(pf_data, id, dbg, num, size);
	case ALS_CHIP_APDS9999_RGB:
		return store_als_debug_apds9999(pf_data, id, dbg, num, size);
	case ALS_CHIP_AMS_TMD3725_RGB:
		return store_als_debug_tmd3725(pf_data, id, dbg, num, size);
	case ALS_CHIP_AMS_TMD3702_RGB:
		return store_als_debug_tmd3702(pf_data, id, dbg, num, size);
	case ALS_CHIP_AMS_TCS3707_RGB:
		return store_als_debug_tcs3707(pf_data, id, dbg, num, size);
	case ALS_CHIP_AMS_TCS3701_RGB:
		return store_als_debug_tcs3701(pf_data, id, dbg, num, size);
	case ALS_CHIP_VISHAY_VCNL36658:
		return store_als_debug_vcnl36658(pf_data, id, dbg, num, size);
	case ALS_CHIP_LITEON_LTR582:
		return store_als_debug_ltr582(pf_data, id, dbg, num, size);
	case ALS_CHIP_TMD2745:
		return store_als_debug_tmd2745(pf_data, id, dbg, num, size);
	case ALS_CHIP_STK32670:
		return store_als_debug_stk32670(pf_data, id, dbg, num, size);
	default:
		return size;
	}

	return size;
}

