/*
 * lm3642_ir.c
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
#include "aux_flash_ldo.h"
#include "hisi_adc.h" // for ito adc
#include "securec.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"

/* LM3642_IR Registers define */
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
#define IVFM_DIS 0x00
#define UVLO_EN 0x80
#define UVLO_DIS 0x00
#define TX_PIN 0x40
#define TORCH_PIN 0x10

#define FLASH_LED_MAX 16
#define TORCH_LED_MAX 15
#define FLASH_LED_LEVEL_INVALID 0xff
#define LM3642_FLASH_MAX_CUR 1500 // 1500ma
#define MAX_FLASH_ID 0xFF // used to check flash_lightness_sotre input param
#define MAX_LIGHTNESS_PARAM_NUM_RT 3
#define LIGHTNESS_PARAM_FD_INDEX_RT 0
#define LIGHTNESS_PARAM_MODE_INDEX_RT 1
#define LIGHTNESS_PARAM_CURRENT_INDEX_RT 2

#define LM3642_IR_FLASH_DEFAULT_CUR_LEV 7 // 760mA
#define LM3642_IR_TORCH_DEFAULT_CUR_LEV 3 // 187.5mA

#define LM3642_IR_UNDER_VOLTAGE_LOCKOUT 0x10
#define LM3642_IR_OVER_VOLTAGE_PROTECT 0x08
#define LM3642_IR_LED_VOUT_SHORT 0x04
#define LM3642_IR_OVER_TEMP_PROTECT 0x02
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

enum lm3642_ir_current_conf {
	CURRENT_TORCH_LEVEL_MMI = 0, // use for MMIE IR test
	CURRENT_TORCH_LEVEL_RT = 1, // use for RT current test
	CURRENT_MAX = 2,
};

/* ir flash RT self test param */
enum lm3642_ir_selftest_param {
	IR_SELFTEST_ON_TIME_INDEX = 0,
	IR_SELFTEST_OFF_TIME_INDEX = 1,
	IR_SELFTEST_CURRENT_INDEX = 2,
	IR_SELFTEST_TIMES_INDEX = 3,
	IR_SELFTEST_MAX_INDEX = 4,
};


/* ir ito threshold param */
enum lm3642_ito_threshold_param {
	ITO_THR_MIN = 0,
	ITO_THR_MAX = 1,
	ITO_THR_MAX_INDEX = 2,
};

/* Internal data struct define */
struct hw_lm3642_ir_private_data_t {
	unsigned char flash_led[FLASH_LED_MAX];
	unsigned char torch_led[TORCH_LED_MAX];
	unsigned int flash_led_num;
	unsigned int torch_led_num;
	unsigned int flash_current;
	unsigned int torch_current;
	/* flash electric current config */
	unsigned int ecurrent[CURRENT_MAX];
	unsigned int selftest_param[IR_SELFTEST_MAX_INDEX];

	/* flash control pin */
	unsigned int strobe;
	unsigned int chipid;
	/* flash ldo */
	unsigned int need_flash_ldo;
	unsigned int flash_ldo_voltage;
	unsigned int is_power_on;
	/* ito */
	unsigned int ito_enable;
	unsigned int ito_threshold[ITO_THR_MAX_INDEX];
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
	{ 188, MODE_TORCH, 3 }, { 234, MODE_TORCH, 4 },
	{ 281, MODE_TORCH, 5 }, { 328, MODE_TORCH, 6 },
	{ 375, MODE_TORCH, 7 }
};

/* Internal varible define */
static struct hw_lm3642_ir_private_data_t g_lm3642_ir_pdata;
static struct hw_flash_ctrl_t g_lm3642_ir_ctrl;
static struct i2c_driver g_lm3642_ir_i2c_driver;

extern struct dsm_client *client_flash;

define_hisi_flash_mutex(lm3642_ir);
#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif
/* Function define */

static int hw_lm3642_ir_clear_error_and_notify_dmd(struct hw_flash_ctrl_t *flash_ctrl)
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

	if (val & LM3642_IR_OVER_TEMP_PROTECT) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash temperature is too hot FlagReg[0x%x]\n",
				val);
			dsm_client_notify(client_flash,
				DSM_FLASH_HOT_DIE_ERROR_NO);
			cam_warn("[I/DSM] %s flash temperature is too hot FlagReg[0x%x]",
				__func__, val);
		}
	}

	if (val & (LM3642_IR_OVER_VOLTAGE_PROTECT | LM3642_IR_LED_VOUT_SHORT)) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash OVP, LED or VOUT short FlagReg[0x%x]\n",
				val);
			dsm_client_notify(client_flash,
				DSM_FLASH_OPEN_SHOTR_ERROR_NO);
			cam_warn("[I/DSM] %s flash OVP, LED or VOUT short FlagReg[0x%x]",
				__func__, val);
		}
	}

	if (val & LM3642_IR_UNDER_VOLTAGE_LOCKOUT) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash UVLO FlagReg[0x%x]\n", val);
			dsm_client_notify(client_flash,
				DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
			cam_warn("[I/DSM] %s flash UVLO FlagReg[0x%x]",
				__func__, val);
		}
	}

	return 0;
}

static int hw_lm3642_ir_find_match_flash_current(int cur_flash)
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
		return LM3642_IR_FLASH_DEFAULT_CUR_LEV;
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

static int hw_lm3642_ir_find_match_torch_current(int cur_torch,
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

static int hw_lm3642_ir_init(struct hw_flash_ctrl_t *flash_ctrl)
{
	cam_debug("%s enter\n", __func__);
	if (!flash_ctrl) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	return 0;
}

static int hw_lm3642_ir_exit(struct hw_flash_ctrl_t *flash_ctrl)
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

static int hw_lm3642_ir_flash_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3642_ir_private_data_t *pdata = NULL;
	unsigned char val = 0;
	int current_level = 0;
	int rc;

	cam_info("%s data = %d\n", __func__, data);
	if (!flash_ctrl || !flash_ctrl->pdata ||
		!flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = flash_ctrl->pdata;
	if (pdata->flash_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3642_IR_FLASH_DEFAULT_CUR_LEV;
	} else {
		current_level = hw_lm3642_ir_find_match_flash_current(data);
		if (current_level < 0)
			current_level = LM3642_IR_FLASH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3642_ir_clear_error_and_notify_dmd(flash_ctrl);
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
	if (pdata->need_flash_ldo == 1) {
		// disable UVLO
		loge_if_ret(i2c_func->i2c_write(i2c_client,
			REG_IVFM, UVLO_DIS) < 0);
		if (flash_ctrl->flash_mask_enable) {
			loge_if_ret(i2c_func->i2c_write(i2c_client,
				REG_ENABLE, MODE_FLASH | TX_PIN) < 0);
		} else {
			loge_if_ret(i2c_func->i2c_write(i2c_client,
				REG_ENABLE, MODE_FLASH) < 0);
		}
	} else {
		if (flash_ctrl->flash_mask_enable) {
			loge_if_ret(i2c_func->i2c_write(i2c_client,
				REG_ENABLE, MODE_FLASH | TX_PIN) < 0);
		} else {
			loge_if_ret(i2c_func->i2c_write(i2c_client,
				REG_ENABLE, MODE_FLASH | IVFM_EN) < 0);
		}
	}

	return 0;
}

static int hw_lm3642_ir_torch_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3642_ir_private_data_t *pdata = NULL;
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
	pdata = (struct hw_lm3642_ir_private_data_t *)flash_ctrl->pdata;
	if (pdata->torch_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3642_IR_TORCH_DEFAULT_CUR_LEV;
	} else {
		current_level = hw_lm3642_ir_find_match_torch_current(data,
			&mode);
		if (current_level < 0)
			current_level = LM3642_IR_TORCH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3642_ir_clear_error_and_notify_dmd(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flash light clear error", __func__);
		return -EINVAL;
	}

	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CURRENT_CONTROL, &val) < 0);

	/* set LED Flash current value */
	val = (val & 0x0f) | ((unsigned int)current_level << 4); // 16bits
	cam_info("%s the led torch current val=0x%x, current_level=%d, mode=%s",
		__func__, val, current_level,
		(mode == MODE_INDICATOR) ? "MODE_INDICATOR" : "MODE_TORCH");

	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_CURRENT_CONTROL, val) < 0);
	if (pdata->need_flash_ldo == 1) {
		// disable UVLO
		loge_if_ret(i2c_func->i2c_write(i2c_client,
			REG_IVFM, UVLO_DIS) < 0);
		loge_if_ret(i2c_func->i2c_write(i2c_client,
			REG_ENABLE, mode | TORCH_PIN) < 0);
	} else {
		loge_if_ret(i2c_func->i2c_write(i2c_client,
			REG_ENABLE, mode | IVFM_EN) < 0);
	}

	return 0;
}

static int hw_lm3642_ir_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
	int rc = -1;
	struct hw_lm3642_ir_private_data_t *pdata = NULL;

	cam_debug("%s enter\n", __func__);
	if (!flash_ctrl || !cdata || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3642_ir_private_data_t *)flash_ctrl->pdata;
	mutex_lock(flash_ctrl->hw_flash_mutex);
	if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 0)) {
		rc = aux_ldo_supply_enable(flash_ctrl,
			pdata->flash_ldo_voltage);
		if (rc < 0) {
			cam_err("%s power on fail", __func__);
			mutex_unlock(flash_ctrl->hw_flash_mutex);
			return rc;
		}
		pdata->is_power_on = 1;
	}
	cam_info("%s mode=%d, level=%d\n", __func__, cdata->mode, cdata->data);

	// check ITO protect
	if (pdata->ito_enable) {
		bool dsm_flag = false;
		int ret = hisi_adc_get_value(12); // ITO Use ADC 12
		if (ret >= 0) {
			if (((unsigned int)ret < pdata->ito_threshold[ITO_THR_MIN]) ||
				((unsigned int)ret > pdata->ito_threshold[ITO_THR_MAX])) {
				dsm_flag = true;
				cam_err("%s ito %d out of [%d,%d]", __func__,
					ret,
					pdata->ito_threshold[ITO_THR_MIN],
					pdata->ito_threshold[ITO_THR_MAX]);
			}
		} else {
			dsm_flag = true;
			cam_err("%s get ito fail ret %d", __func__, ret);
		}
		cam_info("%s ito adc ret %d", __func__, ret);
		if (dsm_flag) {
			mutex_unlock(flash_ctrl->hw_flash_mutex);
			// repo dsm
			if (!dsm_client_ocuppy(client_flash)) {
				dsm_client_record(client_flash,
					"ito %d out of [%d,%d]\n", ret,
					pdata->ito_threshold[ITO_THR_MIN],
					pdata->ito_threshold[ITO_THR_MAX]);
				dsm_client_notify(client_flash,
					DSM_ITO_OUT_OF_RANGE_ERROR_NO);
				cam_warn("[I/DSM] %s report[%d]",
					__func__,
					DSM_ITO_OUT_OF_RANGE_ERROR_NO);
			}
			return -EINVAL;
		}
	}

	if (cdata->mode == FLASH_MODE)
		rc = hw_lm3642_ir_flash_mode(flash_ctrl, cdata->data);
	else
		rc = hw_lm3642_ir_torch_mode(flash_ctrl, cdata->data);

	flash_ctrl->state.mode = cdata->mode;
	flash_ctrl->state.data = cdata->data;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_lm3642_ir_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	unsigned char val;
	int rc = -1;
	struct hw_lm3642_ir_private_data_t *pdata = NULL;

	cam_info("%s enter\n", __func__);

	if (!flash_ctrl || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write ||
		!flash_ctrl->pdata) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3642_ir_private_data_t *)flash_ctrl->pdata;

	mutex_lock(flash_ctrl->hw_flash_mutex);
	flash_ctrl->state.mode = STANDBY_MODE;
	flash_ctrl->state.data = 0;
	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	if ((pdata->need_flash_ldo == 0) || ((pdata->need_flash_ldo == 1) &&
		(pdata->is_power_on == 1))) {
		if (i2c_func->i2c_read(i2c_client, REG_FLAGS, &val) < 0)
			cam_err("%s %d", __func__, __LINE__);
		if (i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_STANDBY) < 0)
			cam_err("%s %d", __func__, __LINE__);
	}

	if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 1)) {
		rc = aux_ldo_supply_disable();
		if (rc < 0) {
			cam_err("%s power off fail", __func__);
			mutex_unlock(flash_ctrl->hw_flash_mutex);
			return rc;
		}
		pdata->is_power_on = 0;
	}

	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return 0;
}

static int hw_lm3642_ir_get_ito_protect_data(struct device_node *of_node,
	struct hw_lm3642_ir_private_data_t *pdata)
{
	int rc;
	int i;

	if (!of_node || !pdata) {
		cam_err("%s of_node or pdata is NULL", __func__);
		return -EINVAL;
	}
	pdata->ito_enable = 0;
	pdata->ito_threshold[ITO_THR_MIN] = 0;
	pdata->ito_threshold[ITO_THR_MAX] = 0;
	rc = of_property_read_u32(of_node, "huawei,ito_protect_enable",
		&pdata->ito_enable);
	cam_debug("%s ito_enable %d, rc %d", __func__, pdata->ito_enable, rc);
	if (rc < 0) {
		cam_info("%s failed %d", __func__, __LINE__);
		pdata->ito_enable = 0;
	}

	if (pdata->ito_enable) {
		rc = of_property_read_u32_array(of_node,
			"huawei,ito_protect_threshold",
			pdata->ito_threshold, ITO_THR_MAX_INDEX);
		if (rc < 0) {
			cam_info("%s read ito_protect_threshold failed line %d\n",
				__func__, __LINE__);
		} else {
			for (i = 0; i < ITO_THR_MAX_INDEX; i++)
				cam_info("%s ito_protect_threshold[%d]=%d\n",
					__func__, i,
					pdata->ito_threshold[i]);
		}
	}
	return rc;
}

static int hw_lm3642_ir_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3642_ir_private_data_t *pdata = NULL;
	struct device_node *of_node = NULL;
	int rc = -1;
	int i;

	cam_info("%s enter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->pdata || !flash_ctrl->dev) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3642_ir_private_data_t *)flash_ctrl->pdata;
	of_node = flash_ctrl->dev->of_node;

	pdata->is_power_on = 0;
	rc = of_property_read_u32(of_node, "huawei,need_flash_ldo",
		&pdata->need_flash_ldo);
	cam_debug("%s need_flash_ldo %d, rc %d", __func__,
		pdata->need_flash_ldo, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		pdata->need_flash_ldo = 0;
	}

	if (pdata->need_flash_ldo ==  1) {
		rc = of_property_read_u32(of_node, "huawei,flash_ldo_voltage",
			&pdata->flash_ldo_voltage);
		cam_info("%s flash_ldo_voltage %d, rc %d", __func__,
			pdata->flash_ldo_voltage, rc);
		if (rc < 0) {
			cam_err("%s failed %d", __func__, __LINE__);
			return rc;
		}
	}

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
	cam_info("%s hisi,chipid 0x%x, rc %d", __func__, pdata->chipid, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32_array(of_node, "huawei,flash-current",
		pdata->ecurrent, CURRENT_MAX);
	if (rc < 0) {
		cam_err("%s read flash-current failed line %d\n",
			__func__, __LINE__);
		return rc;
	}
	for (i = 0; i < CURRENT_MAX; i++)
		cam_info("%s ecurrent[%d]=%d.\n", __func__, i,
			pdata->ecurrent[i]);

	rc = of_property_read_u32_array(of_node, "huawei,selftest-param",
		pdata->selftest_param, IR_SELFTEST_MAX_INDEX);
	if (rc < 0) {
		cam_err("%s read selftest-param failed line %d\n",
			__func__, __LINE__);
		return rc;
	}
	for (i = 0; i < IR_SELFTEST_MAX_INDEX; i++)
		cam_info("%s selftest_param[%d]=%d\n", __func__, i,
			pdata->selftest_param[i]);
	rc = hw_lm3642_ir_get_ito_protect_data(of_node, pdata);
	return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_lm3642_ir_lightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}
	int rc;

	// show param for ir selftest for RT, PAGE_SIZE = 4096
	rc = scnprintf(buf, PAGE_SIZE, "%d %d %d %d",
		g_lm3642_ir_pdata.selftest_param[IR_SELFTEST_ON_TIME_INDEX],
		g_lm3642_ir_pdata.selftest_param[IR_SELFTEST_OFF_TIME_INDEX],
		g_lm3642_ir_pdata.selftest_param[IR_SELFTEST_CURRENT_INDEX],
		g_lm3642_ir_pdata.selftest_param[IR_SELFTEST_TIMES_INDEX]);

	return rc;
}

static int hw_lm3642_ir_param_check(char *buf, unsigned long *param,
	int num_of_par)
{
	if (!buf || !param) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}
	char *token = NULL;
	int base = 0;
	int cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			// add for check token[1] has valid element
			if (strlen(token) < 2) {
				base = 10; // decimal
			} else {
				// format 0x**
				if ((token[1] == 'x') || (token[1] == 'X'))
					base = 16; // hex
				else
					base = 10; // decimal
			}
			if (strict_strtoul(token, base, &param[cnt]) != 0)
				return -EINVAL;
			token = strsep(&buf, " ");
		} else {
			cam_info("%s the %d param is null\n",
				__func__, num_of_par);
		}
	}
	return 0;
}

static ssize_t hw_lm3642_ir_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (!buf || !dev || !attr) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[MAX_LIGHTNESS_PARAM_NUM_RT] = {0};
	int rc;
	int flash_id = 0;

	cam_info("%s enter, buf=%s", __func__, buf);
	rc = hw_lm3642_ir_param_check((char *)buf, param,
		MAX_LIGHTNESS_PARAM_NUM_RT);
	if (rc < 0) {
		cam_err("%s failed to check param", __func__);
		return rc;
	}

	// 0 - flash id
	flash_id = (int)param[LIGHTNESS_PARAM_FD_INDEX_RT];
	// 1 - mode
	cdata.mode = (int)param[LIGHTNESS_PARAM_MODE_INDEX_RT];
	// 2 - current
	cdata.data = (int)param[LIGHTNESS_PARAM_CURRENT_INDEX_RT];
	cam_info("%s flash_id=%d, cdata.mode=%d, cdata.data=%d",
		__func__, flash_id, cdata.mode, cdata.data);

	if ((cdata.mode < STANDBY_MODE) || (cdata.mode > MAX_MODE)) {
		cam_err("%s cdata.mode out of bounds", __func__);
		return -EINVAL;
	}
	// 0~1500 mA
	if ((cdata.data < 0) || (cdata.data > LM3642_FLASH_MAX_CUR)) {
		cam_err("%s cdata.data out of bounds", __func__);
		return -EINVAL;
	}
	// 0~0xFF, pass from HAL, but not use just print
	if ((flash_id < 0) || (flash_id > MAX_FLASH_ID)) {
		cam_err("%s flash_id out of bounds", __func__);
		return -EINVAL;
	}

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_lm3642_ir_off(&g_lm3642_ir_ctrl);
		if (rc < 0) {
			cam_err("%s lm3642_ir flash off error", __func__);
			return rc;
		}
	} else if (cdata.mode == TORCH_MODE) {
		cdata.data =
			g_lm3642_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_RT];
		cam_info("%s mode=%d, current=%d",
			__func__, cdata.mode, cdata.data);

		rc = hw_lm3642_ir_on(&g_lm3642_ir_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s lm3642_ir flash on error", __func__);
			return rc;
		}
	} else if (cdata.mode == FLASH_MODE) {
		cam_info("%s mode=%d, current=%d",
			__func__, cdata.mode, cdata.data);
		cdata.mode = TORCH_MODE;
		rc = hw_lm3642_ir_on(&g_lm3642_ir_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s flash on error", __func__);
			return rc;
		}
	} else {
		cam_err("%s wrong mode=%d", __func__, cdata.mode);
		return -EINVAL;
	}

	return count;
}
#endif

static ssize_t hw_lm3642_ir_flash_mask_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;

	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}

	rc = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE - 1,
		"flash_mask_disabled=%d.",
		g_lm3642_ir_ctrl.flash_mask_enable);
	if (rc < 0)
		cam_err("%s %d::snprintf_s return error %d",
			__func__, __LINE__, rc);
	rc = strlen(buf) + 1;
	return rc;
}

static ssize_t hw_lm3642_ir_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (!dev || !attr || !buf) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}

	if (buf[0] == '0')
		g_lm3642_ir_ctrl.flash_mask_enable = false;
	else
		g_lm3642_ir_ctrl.flash_mask_enable = true;

	cam_debug("%s flash_mask_enable=%d", __func__,
		g_lm3642_ir_ctrl.flash_mask_enable);
	return count;
}

static void hw_lm3642_ir_torch_brightness_set(struct led_classdev *cdev,
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
		rc = hw_lm3642_ir_off(&g_lm3642_ir_ctrl);
		if (rc < 0) {
			cam_err("%s pmu_led off error", __func__);
			return;
		}
	} else {
		cdata.mode = TORCH_MODE;
		cdata.data =
			g_lm3642_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_MMI];

		cam_info("%s current=%d", __func__, cdata.data);
		rc = hw_lm3642_ir_on(&g_lm3642_ir_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s pmu_led on error", __func__);
			return;
		}
	}
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute g_lm3642_ir_lightness =
	__ATTR(flash_lightness_f, 0660, hw_lm3642_ir_lightness_show,
	hw_lm3642_ir_lightness_store);
#endif

static struct device_attribute g_lm3642_ir_flash_mask =
	__ATTR(flash_mask, 0660, hw_lm3642_ir_flash_mask_show,
	hw_lm3642_ir_flash_mask_store);

static int hw_lm3642_ir_register_attribute(struct hw_flash_ctrl_t *flash_ctrl,
	struct device *dev)
{
	int rc = 0;

	if (!flash_ctrl || !dev || !flash_ctrl->pdata) {
		cam_err("%s flash_ctrl or dev is NULL", __func__);
		return -EINVAL;
	}

	flash_ctrl->cdev_torch.name = "torch_front";
	flash_ctrl->cdev_torch.max_brightness =
		((struct hw_lm3642_ir_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
	flash_ctrl->cdev_torch.brightness_set =
		hw_lm3642_ir_torch_brightness_set;
	rc = led_classdev_register((struct device *)dev,
		&flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev", __func__);
		goto err_out;
	}
#ifdef CAMERA_FLASH_FACTORY_TEST
	rc = device_create_file(dev, &g_lm3642_ir_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat lightness attribute", __func__);
		goto err_create_lightness_file;
	}
#endif
	rc = device_create_file(dev, &g_lm3642_ir_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute", __func__);
		goto err_create_flash_mask_file;
	}
	return 0;

err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
	device_remove_file(dev, &g_lm3642_ir_lightness);
#endif
err_create_lightness_file:
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
	return rc;
}

static int hw_lm3642_ir_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3642_ir_private_data_t *pdata = NULL;
	unsigned char id;
	int rc = -1;

	cam_info("%s enter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->pdata ||
		!flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3642_ir_private_data_t *)flash_ctrl->pdata;

	if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 0)) {
		rc = aux_ldo_supply_enable(flash_ctrl,
			pdata->flash_ldo_voltage);
		if (rc < 0) {
			cam_err("%s power on fail", __func__);
			return rc;
		}
		pdata->is_power_on = 1;

		if (i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0) {
			cam_err("%s i2c read REG_CHIP_ID error",
				__func__);
			goto err;
		}
		cam_info("%s id=0x%x\n", __func__, id);
		if (id != pdata->chipid) {
			cam_err("%s match error, id 0x%x != 0x%x",
				__func__, (id & 0x7), pdata->chipid);
			goto err;
		}

		rc = aux_ldo_supply_disable();
		if (rc < 0) {
			cam_err("%s power off fail", __func__);
			return rc;
		}
		pdata->is_power_on = 0;
	} else {
		loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0);
		cam_info("%s 0x%x\n", __func__, id);
		id = id & CHIP_ID_MASK;
		if (id != pdata->chipid) {
			cam_err("%s match error, 0x%x != 0x%x",
				__func__, id, pdata->chipid);
			return -EINVAL;
		}
		loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE,
			IVFM_EN) < 0); // enable IVFM
		// enable UVLO, set voltage 3.2v
		loge_if_ret(i2c_func->i2c_write(i2c_client, REG_IVFM,
			UVLO_EN | UVLO_VOLTAGE) < 0);
	}

#ifdef CAMERA_FLASH_FACTORY_TEST

	register_camerafs_attr(&g_lm3642_ir_lightness);

#endif
	return 0;

err:
	if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 1)) {
		rc = aux_ldo_supply_disable();
		if (rc < 0) {
			cam_err("%s power off fail", __func__);
			return rc;
		}
		pdata->is_power_on = 0;
	}
	return -EINVAL;
}

static int hw_lm3642_ir_remove(struct i2c_client *client)
{
	cam_debug("%s enter", __func__);

	if (!client) {
		cam_err("%s invalid param", __func__);
		return -EINVAL;
	}
	g_lm3642_ir_ctrl.func_tbl->flash_exit(&g_lm3642_ir_ctrl);

	client->adapter = NULL;
	return 0;
}

static void hw_lm3642_ir_shutdown(struct i2c_client *client)
{
	int rc = -1;

	if (!client) {
		cam_err("%s invalid param", __func__);
		return;
	}
	rc = hw_lm3642_ir_off(&g_lm3642_ir_ctrl);
	cam_info("%s lm3642_ir shut down at %d", __func__, __LINE__);
	if (rc < 0)
		cam_err("%s lm3642_ir flash off error", __func__);
}

static const struct i2c_device_id g_lm3642_ir_id[] = {
	{ "lm3642_ir", (unsigned long) &g_lm3642_ir_ctrl },
	{}
};

static const struct of_device_id g_lm3642_ir_dt_match[] = {
	{ .compatible = "huawei,lm3642_ir" },
	{}
};
MODULE_DEVICE_TABLE(of, lm3642_ir_dt_match);

static struct i2c_driver g_lm3642_ir_i2c_driver = {
	.probe  = hw_flash_i2c_probe,
	.remove = hw_lm3642_ir_remove,
	.shutdown = hw_lm3642_ir_shutdown,
	.id_table   = g_lm3642_ir_id,
	.driver = {
		.name = "hw_lm3642_ir",
		.of_match_table = g_lm3642_ir_dt_match,
	},
};

static int __init hw_lm3642_ir_module_init(void)
{
	cam_info("%s enter\n", __func__);
	return i2c_add_driver(&g_lm3642_ir_i2c_driver);
}

static void __exit hw_lm3642_ir_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&g_lm3642_ir_i2c_driver);
}

static struct hw_flash_i2c_client g_lm3642_ir_i2c_client;

static struct hw_flash_fn_t g_lm3642_ir_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_lm3642_ir_init,
	.flash_exit = hw_lm3642_ir_exit,
	.flash_on = hw_lm3642_ir_on,
	.flash_off = hw_lm3642_ir_off,
	.flash_match = hw_lm3642_ir_match,
	.flash_get_dt_data = hw_lm3642_ir_get_dt_data,
	.flash_register_attribute = hw_lm3642_ir_register_attribute,
};

static struct hw_flash_ctrl_t g_lm3642_ir_ctrl = {
	.flash_i2c_client = &g_lm3642_ir_i2c_client,
	.func_tbl = &g_lm3642_ir_func_tbl,
	.hw_flash_mutex = &flash_mut_lm3642_ir,
	.pdata = (void *)&g_lm3642_ir_pdata,
	.flash_mask_enable = false,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_init(hw_lm3642_ir_module_init);
module_exit(hw_lm3642_ir_module_exit);
MODULE_DESCRIPTION("LM3642_IR FLASH");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
