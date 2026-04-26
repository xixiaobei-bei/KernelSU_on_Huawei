/*
 * lm3642_front.c
 * Copyright (c) 2011-2020 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "hw_flash.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"

/* LM3642_FRONT Registers define */
#define REG_CHIP_ID 0x00
#define REG_ENABLE 0x0a
#define REG_FLAGS 0x0b
#define REG_FLASH_FEATURES 0x08
#define REG_CURRENT_CONTROL 0x09
#define REG_IVFM 0x01

#define MODE_STANDBY 0x00
#define UVLO_VOLTAGE 0x0C // 3.2v
#define FLASH_TIMEOUT 0x05 // 600ms
#define FLASH_RAMP_TIME 0x08 // 512us
#define IVFM_EN 0x80
#define UVLO_EN 0x80
#define TX_PIN 0x40

#define FLASH_LED_MAX 16
#define TORCH_LED_MAX 11
#define FLASH_LED_LEVEL_INVALID 0xff

#define LM3642_FRONT_FLASH_DEFAULT_CUR_LEV 7 // 760mA
#define LM3642_FRONT_TORCH_DEFAULT_CUR_LEV 1 // 94mA
#define LM3642_FRONT_TORCH_RT_CUR 48 // mA

#define LM3642_FRONT_UNDER_VOLTAGE_LOCKOUT 0x10
#define LM3642_FRONT_OVER_VOLTAGE_PROTECT 0x08
#define LM3642_FRONT_LED_VOUT_SHORT 0x04
#define LM3642_FRONT_OVER_TEMP_PROTECT 0x02
#define CHIP_ID_MASK 0x07
enum mode_e {
	MODE_INDICATOR = 0x01, // from spec
	MODE_TORCH = 0x02, // from spec
	MODE_FLASH = 0x03, // from spec
};

struct torch_data {
	int cur_val; // current value
	enum mode_e mode;
	int cur_level; // current level write to current control register
};

/* Internal data struct define */
struct hw_lm3642_front_private_data_t {
	unsigned char flash_led[FLASH_LED_MAX];
	unsigned char torch_led[TORCH_LED_MAX];
	unsigned int flash_led_num;
	unsigned int torch_led_num;
	unsigned int flash_current;
	unsigned int torch_current;

	/* flash control pin */
	unsigned int strobe;

	unsigned int chipid;
};

static int g_flash_arry[FLASH_LED_MAX] = { 94, 188, 281, 375, 469, 563, 656,
	750, 844, 938, 1031, 1125, 1219, 1313, 1406, 1500
};

static struct torch_data g_torch_arry[TORCH_LED_MAX] = {
	/* { cur_val, mode, cur_level } */
	{ 6, MODE_INDICATOR, 0 }, { 12, MODE_INDICATOR, 1 },
	{ 18, MODE_INDICATOR, 2 }, { 23, MODE_INDICATOR, 3 },
	{ 29, MODE_INDICATOR, 4 }, { 35, MODE_INDICATOR, 5 },
	{ 41, MODE_INDICATOR, 6 }, { 48, MODE_TORCH, 0 },
	{ 94, MODE_TORCH, 1 }, { 141, MODE_TORCH, 2 },
	{ 188, MODE_TORCH, 3 }
};

/* Internal varible define */
static struct hw_lm3642_front_private_data_t g_lm3642_front_pdata;
static struct hw_flash_ctrl_t g_lm3642_front_ctrl;
static struct i2c_driver g_lm3642_front_i2c_driver;

extern struct dsm_client *client_flash;

define_hisi_flash_mutex(lm3642_front);
#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif
/* Function define */

static int hw_lm3642_front_clear_error_and_notify_dmd(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	unsigned char val = 0;
	int rc;

	if (!flash_ctrl || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	/* clear error flag, resume chip */
	rc = i2c_func->i2c_read(i2c_client, REG_FLAGS, &val);
	if (rc < 0) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash i2c transfer fail\n");
			dsm_client_notify(client_flash,
				DSM_FLASH_I2C_ERROR_NO);
			cam_err("[I/DSM] %s flashlight i2c fail", __func__);
		}
		return -EINVAL;
	}

	if (val & LM3642_FRONT_OVER_TEMP_PROTECT) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash temperature is too hot! FlagReg[0x%x]\n",
				val);
			dsm_client_notify(client_flash,
				DSM_FLASH_HOT_DIE_ERROR_NO);
			cam_warn("[I/DSM] %s flash temperature is too hot FlagReg[0x%x]",
				__func__, val);
		}
	}

	if (val & (LM3642_FRONT_OVER_VOLTAGE_PROTECT |
		LM3642_FRONT_LED_VOUT_SHORT)) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash OVP, LED or VOUT short! FlagReg[0x%x]\n",
				val);
			dsm_client_notify(client_flash,
				DSM_FLASH_OPEN_SHOTR_ERROR_NO);
			cam_warn("[I/DSM] %s flash OVP, LED or VOUT short FlagReg[0x%x]",
				__func__, val);
		}
	}

	if (val & LM3642_FRONT_UNDER_VOLTAGE_LOCKOUT) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash UVLO! FlagReg[0x%x]\n", val);
			dsm_client_notify(client_flash,
				DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
			cam_warn("[I/DSM] %s flash UVLO FlagReg[0x%x]",
				__func__, val);
		}
	}

	return 0;
}

static int hw_lm3642_front_find_match_flash_current(int cur_flash)
{
	int cur_level = 0;
	int i;

	cam_info("%s enter cur_flash %d\n", __func__, cur_flash);
	if (cur_flash <= 0) {
		cam_err("%s current set is error", __func__);
		return -EINVAL;
	}

	if (cur_flash >= g_flash_arry[FLASH_LED_MAX - 1]) {
		cam_warn("%s current set is %d", __func__, cur_flash);
		return LM3642_FRONT_FLASH_DEFAULT_CUR_LEV;
	}

	for (i = 0; i < FLASH_LED_MAX; i++) {
		if (cur_flash <= g_flash_arry[i]) {
			cam_info("%s  i %d\n", __func__, i);
			break;
		}
	}

	if (i == 0) {
		cur_level = i;
	} else {
		if (i == FLASH_LED_MAX)
			i = FLASH_LED_MAX - 1; // find last valid data

		// find nearest level
		if ((cur_flash - g_flash_arry[i - 1]) <
			(g_flash_arry[i] - cur_flash))
			cur_level = i - 1;
		else
			cur_level = i;
	}

	return cur_level;
}

static int hw_lm3642_front_find_match_torch_current(int cur_torch,
	unsigned char *mode)
{
	int index = 0;
	int i;

	cam_info("%s enter cur_torch %d", __func__, cur_torch);
	if (cur_torch <= 0) {
		cam_err("%s current set is error", __func__);
		return -EINVAL;
	}

	if (cur_torch >= g_torch_arry[TORCH_LED_MAX - 1].cur_val) {
		cam_warn("%s current set is %d", __func__, cur_torch);
		*mode = g_torch_arry[TORCH_LED_MAX - 1].mode;
		// return max level
		return g_torch_arry[TORCH_LED_MAX - 1].cur_level;
	}

	for (i = 0; i < TORCH_LED_MAX; i++) {
		if (cur_torch <= g_torch_arry[i].cur_val) {
			cam_info("%s  i %d\n", __func__, i);
			break;
		}
	}

	if (i == 0) {
		index = i;
	} else {
		if (i == TORCH_LED_MAX)
			i = TORCH_LED_MAX - 1; // find last valid data

		// find nearest level
		if ((cur_torch - g_torch_arry[i - 1].cur_val) <
			(g_torch_arry[i].cur_val - cur_torch))
			index = i - 1;
		else
			index = i;
	}

	*mode = g_torch_arry[index].mode;

	return g_torch_arry[index].cur_level;
}

static int hw_lm3642_front_init(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_debug("%s enter\n", __func__);
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	return 0;
}

static int hw_lm3642_front_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_debug("%s enter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->func_tbl ||
		!flash_ctrl->func_tbl->flash_off) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	flash_ctrl->func_tbl->flash_off(flash_ctrl);

	return 0;
}

static int hw_lm3642_front_flash_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3642_front_private_data_t *pdata = NULL;
	unsigned char val = 0;
	int current_level = 0;
	int rc;

	cam_info("%s data = %d\n", __func__, data);
	if (!flash_ctrl || !flash_ctrl->pdata ||
		!flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = flash_ctrl->pdata;
	if (pdata->flash_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3642_FRONT_FLASH_DEFAULT_CUR_LEV;
	} else {
		current_level = hw_lm3642_front_find_match_flash_current(data);
		if (current_level < 0)
			current_level = LM3642_FRONT_FLASH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3642_front_clear_error_and_notify_dmd(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash light clear error", __func__);
		return -EINVAL;
	}

	loge_if_ret(i2c_func->i2c_read(i2c_client,
		REG_CURRENT_CONTROL, &val) < 0);

	/* set LED Flash current value */
	val = (val & 0xf0) | ((unsigned int)current_level & 0x0f); // 16bits
	cam_info("%s led flash current val = 0x%x, current level = %d\n",
		__func__, val, current_level);

	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_CURRENT_CONTROL, val) < 0);
	// set timout and ramp time
	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_FLASH_FEATURES, FLASH_RAMP_TIME | FLASH_TIMEOUT) < 0);
	if (flash_ctrl->flash_mask_enable) {
		loge_if_ret(i2c_func->i2c_write(i2c_client,
			REG_ENABLE, MODE_FLASH | TX_PIN) < 0);
	} else {
		loge_if_ret(i2c_func->i2c_write(i2c_client,
			REG_ENABLE, MODE_FLASH | IVFM_EN) < 0);
	}
	return 0;
}

static int hw_lm3642_front_torch_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3642_front_private_data_t *pdata = NULL;
	unsigned char val;
	int current_level = 0;
	int rc;
	unsigned char mode = MODE_TORCH;

	cam_info("%s data = %d", __func__, data);
	if (!flash_ctrl || !flash_ctrl->pdata ||
		!flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3642_front_private_data_t *)flash_ctrl->pdata;
	if (pdata->torch_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3642_FRONT_TORCH_DEFAULT_CUR_LEV;
	} else {
		current_level =
			hw_lm3642_front_find_match_torch_current(data, &mode);
		if (current_level < 0)
			current_level = LM3642_FRONT_TORCH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3642_front_clear_error_and_notify_dmd(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash light clear error", __func__);
		return -EINVAL;
	}

	loge_if_ret(i2c_func->i2c_read(i2c_client,
		REG_CURRENT_CONTROL, &val) < 0);

	/* set LED Flash current value */
	val = (val & 0x0f) | ((unsigned int)current_level << 4); // 16bits
	cam_info("%s the led torch current val=0x%x, current_level=%d, mode=%s",
		__func__, val, current_level,
		(mode == MODE_INDICATOR) ? "MODE_INDICATOR" : "MODE_TORCH");

	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_CURRENT_CONTROL, val) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_ENABLE, mode | IVFM_EN) < 0);

	return 0;
}

static int hw_lm3642_front_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
	int rc = -1;

	cam_debug("%s enter\n", __func__);
	if (!flash_ctrl || !cdata) {
		cam_err("%s flash_ctrl or cdata is NULL", __func__);
		return -EINVAL;
	}

	cam_info("%s mode=%d, level=%d\n", __func__, cdata->mode, cdata->data);
	mutex_lock(flash_ctrl->hw_flash_mutex);
	if (cdata->mode == FLASH_MODE)
		rc = hw_lm3642_front_flash_mode(flash_ctrl, cdata->data);
	else
		rc = hw_lm3642_front_torch_mode(flash_ctrl, cdata->data);

	flash_ctrl->state.mode = cdata->mode;
	flash_ctrl->state.data = cdata->data;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_lm3642_front_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	unsigned char val;

	cam_info("%s enter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	mutex_lock(flash_ctrl->hw_flash_mutex);
	flash_ctrl->state.mode = STANDBY_MODE;
	flash_ctrl->state.data = 0;
	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	if (i2c_func->i2c_read(i2c_client, REG_FLAGS, &val) < 0)
		cam_err("%s %d", __func__, __LINE__);

	if (i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_STANDBY) < 0)
		cam_err("%s %d", __func__, __LINE__);

	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return 0;
}

static int hw_lm3642_front_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3642_front_private_data_t *pdata = NULL;
	struct device_node *of_node = NULL;
	int rc = -1;

	cam_info("%s enter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->pdata || !flash_ctrl->dev) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return rc;
	}

	pdata = (struct hw_lm3642_front_private_data_t *)flash_ctrl->pdata;
	of_node = flash_ctrl->dev->of_node;

	rc = of_property_read_u32(of_node, "huawei,flash_current",
		&pdata->flash_current);
	cam_info("%s hisi,flash_current %d, rc %d", __func__,
		pdata->flash_current, rc);
	if (rc < 0) {
		cam_info("%s failed %d", __func__, __LINE__);
		pdata->flash_current = FLASH_LED_LEVEL_INVALID;
	}

	rc = of_property_read_u32(of_node, "huawei,torch_current",
		&pdata->torch_current);
	cam_info("%s hisi,torch_current %d, rc %d", __func__,
		pdata->torch_current, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		pdata->torch_current = FLASH_LED_LEVEL_INVALID;
	}
	rc = of_property_read_u32(of_node, "huawei,flash-chipid",
		&pdata->chipid);
	cam_info("%s hisi,chipid 0x%x, rc %d", __func__,
		pdata->chipid, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return rc;
	}
	return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_lm3642_front_lightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}

	ssize_t rc = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE - 1, "mode=%d, data=%d.",
		g_lm3642_front_ctrl.state.mode,
		g_lm3642_front_ctrl.state.data);

	return rc;
}

static int hw_lm3642_front_param_check(char *buf, unsigned long *param,
	int num_of_par)
{
	if (!buf || !param) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}
	char *token = NULL;
	int base;
	int cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16; // Hex
			else
				base = 10; // Decimal

			if (strict_strtoul(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

static ssize_t hw_lm3642_front_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}

	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[2] = {0};
	int rc;

	rc = hw_lm3642_front_param_check((char *)buf, param, 2); // 2 params
	if (rc < 0) {
		cam_err("%s failed to check param", __func__);
		return rc;
	}

	int flash_id = (int)param[0];

	cdata.mode = (int)param[1];
	cam_info("%s flash_id=%d, cdata.mode=%d", __func__,
		flash_id, cdata.mode);
	// bit[0]- rear first flash light.
	// bit[1]- rear sencond flash light.
	// bit[2]- front flash light;
	if (flash_id != 4) {
		cam_err("%s wrong flash_id = %d", __func__, flash_id);
		return -EINVAL;
	}

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_lm3642_front_off(&g_lm3642_front_ctrl);
		if (rc < 0) {
			cam_err("%s lm3642_front flash off error", __func__);
			return rc;
		}
	} else if (cdata.mode == TORCH_MODE) {
		cdata.data = LM3642_FRONT_TORCH_RT_CUR;
		cam_info("%s mode=%d, current=%d", __func__,
			cdata.mode, cdata.data);

		rc = hw_lm3642_front_on(&g_lm3642_front_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s lm3642_front flash on error", __func__);
			return rc;
		}
	} else {
		cam_err("%s wrong mode = %d", __func__, cdata.mode);
		return -EINVAL;
	}

	return count;
}
#endif

static ssize_t hw_lm3642_front_flash_mask_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc = 0;

	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}
	rc = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"flash_mask_disabled=%d.",
		g_lm3642_front_ctrl.flash_mask_enable);
	if (rc <= 0) {
		cam_err("%s snprintf_s failed", __func__);
		return -EINVAL;
	}
	rc = strlen(buf) + 1;
	return rc;
}

static ssize_t hw_lm3642_front_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}

	if (buf[0] == '0')
		g_lm3642_front_ctrl.flash_mask_enable = false;
	else
		g_lm3642_front_ctrl.flash_mask_enable = true;

	cam_debug("%s flash_mask_enable = %d", __func__,
		g_lm3642_front_ctrl.flash_mask_enable);
	return count;
}

static void hw_lm3642_front_torch_brightness_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	struct hw_flash_cfg_data cdata;
	int rc;
	unsigned int led_bright = brightness;

	if (!cdev) {
		cam_err("%s invalid param", __func__);
		return;
	}
	if (led_bright == STANDBY_MODE) {
		rc = hw_lm3642_front_off(&g_lm3642_front_ctrl);
		if (rc < 0) {
			cam_err("%s pmu_led off error", __func__);
			return;
		}
	} else {
		cdata.mode = TORCH_MODE;
		cdata.data = LM3642_FRONT_TORCH_RT_CUR;

		cam_info("%s current = %d", __func__, cdata.data);
		rc = hw_lm3642_front_on(&g_lm3642_front_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s pmu_led on error", __func__);
			return;
		}
	}
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute g_lm3642_front_lightness =
	__ATTR(flash_lightness_f, 0660, hw_lm3642_front_lightness_show,
	hw_lm3642_front_lightness_store);
#endif

static struct device_attribute g_lm3642_front_flash_mask =
	__ATTR(flash_mask, 0660, hw_lm3642_front_flash_mask_show,
	hw_lm3642_front_flash_mask_store);

static int hw_lm3642_front_register_attribute(struct hw_flash_ctrl_t
	*flash_ctrl, struct device *dev)
{
	int rc;

	if (!flash_ctrl || !dev || !flash_ctrl->pdata) {
		cam_err("%s flash_ctrl or dev is NULL", __func__);
		return -EINVAL;
	}

	flash_ctrl->cdev_torch.name = "torch_front";
	flash_ctrl->cdev_torch.max_brightness =
		((struct hw_lm3642_front_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
	flash_ctrl->cdev_torch.brightness_set =
		hw_lm3642_front_torch_brightness_set;
	rc = led_classdev_register((struct device *)dev,
		&flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev", __func__);
		goto err_out;
	}
#ifdef CAMERA_FLASH_FACTORY_TEST
	rc = device_create_file(dev, &g_lm3642_front_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat lightness attribute", __func__);
		goto err_create_lightness_file;
	}
#endif
	rc = device_create_file(dev, &g_lm3642_front_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute", __func__);
		goto err_create_flash_mask_file;
	}
	return 0;

err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
	device_remove_file(dev, &g_lm3642_front_lightness);
#endif
err_create_lightness_file:
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
	return rc;
}

static int hw_lm3642_front_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3642_front_private_data_t *pdata = NULL;
	unsigned char id;

	cam_err("%s enter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->pdata ||
		!flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3642_front_private_data_t *)flash_ctrl->pdata;

	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0);
	cam_info("%s 0x%x\n", __func__, id);
	id = id & CHIP_ID_MASK;
	if (id != pdata->chipid) {
		cam_err("%s match error, 0x%x != 0x%x",
			__func__, id, pdata->chipid);
		return -EINVAL;
	}
	// enable IVFM
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, IVFM_EN) < 0);
	// enable UVLO, set voltage 3.2v
	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_IVFM, UVLO_EN | UVLO_VOLTAGE) < 0);
#ifdef CAMERA_FLASH_FACTORY_TEST

	register_camerafs_attr(&g_lm3642_front_lightness);

#endif
	return 0;
}

static int hw_lm3642_front_remove(struct i2c_client *client)
{
	cam_debug("%s enter", __func__);

	if (!client) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}
	g_lm3642_front_ctrl.func_tbl->flash_exit(&g_lm3642_front_ctrl);

	client->adapter = NULL;
	return 0;
}

static void hw_lm3642_front_shutdown(struct i2c_client *client)
{
	int rc;

	if (!client) {
		cam_err("%s invalid param", __func__);
		return;
	}
	rc = hw_lm3642_front_off(&g_lm3642_front_ctrl);
	cam_info("%s lm3642_front shut down at %d", __func__, __LINE__);
	if (rc < 0)
		cam_err("%s lm3642_front flash off error", __func__);
}

static const struct i2c_device_id g_lm3642_front_id[] = {
	{ "lm3642_front", (unsigned long)&g_lm3642_front_ctrl },
	{}
};

static const struct of_device_id g_lm3642_front_dt_match[] = {
	{ .compatible = "huawei,lm3642_front" },
	{}
};
MODULE_DEVICE_TABLE(of, lm3642_front_dt_match);

static struct i2c_driver g_lm3642_front_i2c_driver = {
	.probe = hw_flash_i2c_probe,
	.remove = hw_lm3642_front_remove,
	.shutdown = hw_lm3642_front_shutdown,
	.id_table = g_lm3642_front_id,
	.driver = {
		.name = "hw_lm3642_front",
		.of_match_table = g_lm3642_front_dt_match,
	},
};

static int __init hw_lm3642_front_module_init(void)
{
	cam_info("%s enter\n", __func__);
	return i2c_add_driver(&g_lm3642_front_i2c_driver);
}

static void __exit hw_lm3642_front_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&g_lm3642_front_i2c_driver);
}

static struct hw_flash_i2c_client g_lm3642_front_i2c_client;

static struct hw_flash_fn_t g_lm3642_front_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_lm3642_front_init,
	.flash_exit = hw_lm3642_front_exit,
	.flash_on = hw_lm3642_front_on,
	.flash_off = hw_lm3642_front_off,
	.flash_match = hw_lm3642_front_match,
	.flash_get_dt_data = hw_lm3642_front_get_dt_data,
	.flash_register_attribute = hw_lm3642_front_register_attribute,
};

static struct hw_flash_ctrl_t g_lm3642_front_ctrl = {
	.flash_i2c_client = &g_lm3642_front_i2c_client,
	.func_tbl = &g_lm3642_front_func_tbl,
	.hw_flash_mutex = &flash_mut_lm3642_front,
	.pdata = (void *)&g_lm3642_front_pdata,
	.flash_mask_enable = false,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_init(hw_lm3642_front_module_init);
module_exit(hw_lm3642_front_module_exit);
MODULE_DESCRIPTION("LM3642_FRONT FLASH");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
