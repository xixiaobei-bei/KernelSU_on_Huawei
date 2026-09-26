/*
 * lm3644.c
 *
 * driver for lm3644
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hw_flash.h"
#include "securec.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-label"
/* LM3644 Registers define */
#define LM3644_SLAVE_ADDRESS 0x63
#define REG_CHIP_ID 0x0c
#define KTD2687_CHIP_ID 0x02
#define LM3644TT_CHIP_ID 0x04
#define AW3644_CHIP_ID 0x02
#define REG_ENABLE 0x01
#define REG_L1_FL 0x03
#define REG_L2_FL 0x04
#define REG_L1_TORCH 0x05
#define REG_L2_TORCH 0x06
#define REG_FAULT_INDICATION_A 0x0a
#define REG_FAULT_INDICATION_B 0x0b
#define LED2_EN (0x1 << 1)
#define LED1_EN 0x1
#define REG_TIMING_CONFIG 0x08
#define MODE_STANDBY 0x00
#define FLASH_TIMEOUT_TI 0x0A /* 600ms */
#define FLASH_TIMEOUT_KTD 0x0F /* 400ms */
#define FLASH_TIMEOUT_AW 0x0A /* 600ms */
#define MODE_TORCH (0x02 << 2)
#define MODE_FLASH (0x03 << 2)
#define STROBE_PIN (0x01 << 5)
#define MODE_TEMP 0x09
#define DEF_TEMP_VAL 0x08
#define INVALID_GPIO 999


#define LM3644_FLASH_DEFAULT_CUR_LEV 63 /* 729mA */
#define LM3644_TORCH_DEFAULT_CUR_LEV 63 /* 178.6mA */
#define LM3644_FLASH_MAX_CUR 1500
#define LM3644_TORCH_MAX_CUR 360
#define LM3644_TORCH_MAX_LEV 128
#define MMI_TEST_BRIGHTNESS 48

#define FLASH_LED_MAX 128
#define TORCH_LED_MAX 128
#define FLASH_LED_LEVEL_INVALID 0xff

#define LM3644_VOLTAGE_PROTECT 0x02
#define LM3644_LED_OPEN_SHORT 0x70
#define LM3644_OVER_TEMP_PROTECT 0x04

#define flash_current_to_level_ti(x) (((x) * 1000 - 10900) / 11725)
#define torch_current_to_level_ti(x) (((x) * 1000 - 1954) / 2800)
#define flash_current_to_level_ktd2687(x) (((32 * (x)) / 375) - 1)
#define torch_current_to_level_ktd2687(x) (((256 * (x)) / 375) - 1)
#define flash_current_to_level_aw3644(x) (((x) * 100 - 1135) / 1172)
#define torch_current_to_level_aw3644(x) (((x) * 100 - 255) / 291)

enum lm3644_pin_type_t {
	STROBE = 0,
	FLASH_EN,
	TORCH,
	MAX_PIN,
};

enum lm3644_pin_status_t {
	FREED = 0,
	REQUESTED,
};

struct lm3644_pin_t {
	unsigned int pin_id;
	enum lm3644_pin_status_t pin_status;
};

/* Internal data struct define */
struct hw_lm3644_private_data_t {
	unsigned char flash_led[FLASH_LED_MAX];
	unsigned char torch_led[TORCH_LED_MAX];
	unsigned int flash_led_num;
	unsigned int torch_led_num;
	unsigned int flash_current;
	unsigned int torch_current;

	/* flash control pin */
	struct lm3644_pin_t pin[MAX_PIN];
	unsigned int strobe;

	unsigned int chipid;
	unsigned int flash_type;
	unsigned int timeout;
	int driver_ic_type;
};

enum ic_type {
	IC_TYPE_LM3644 = 1,
	IC_TYPE_KTD2687 = 2,
	IC_TYPE_AW3644 = 3,
	IC_TYPE_UNKONW = 4,
};

/* Internal varible define */
static struct hw_lm3644_private_data_t g_lm3644_pdata;
static struct hw_flash_ctrl_t g_lm3644_ctrl;
static struct i2c_driver g_lm3644_i2c_driver;
static unsigned int g_flash_type;
static unsigned int g_manual_flag = 0;
extern struct dsm_client *client_flash;

define_hisi_flash_mutex(lm3644);
#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif

static int hw_lm3644_clear_error_and_notify_dmd(struct hw_flash_ctrl_t
	*flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	unsigned char val_a = 0;
	unsigned char val_b = 0;

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	/* clear error flag, resume chip */
	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_A,
		&val_a) < 0);
	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_B,
		&val_b) < 0);
	if (val_a != 0 || val_b != 0) {
		if (!dsm_client_ocuppy(client_flash) && (
			(val_a & LM3644_VOLTAGE_PROTECT) ||
			(val_b & LM3644_VOLTAGE_PROTECT))) {
			dsm_client_record(client_flash,
				"UVLO FALUT or OVP FAULT\n");
			dsm_client_notify(client_flash,
				DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
			cam_err("[I/DSM] %s Undervoltage lockout or Overvoltage error",
				__func__);
		}
		if (!dsm_client_ocuppy(client_flash) &&
			(val_a & LM3644_LED_OPEN_SHORT)) {
			dsm_client_record(client_flash,
				"LED short or open circuit\n");
			dsm_client_notify(client_flash,
				DSM_FLASH_OPEN_SHOTR_ERROR_NO);
			cam_err("[I/DSM] %s LED short or open circuit error",
				__func__);
		}
		if (!dsm_client_ocuppy(client_flash) &&
			(val_a & LM3644_OVER_TEMP_PROTECT)) {
			dsm_client_record(client_flash,
				"LED over-Temperature\n");
			dsm_client_notify(client_flash,
				DSM_FLASH_OPEN_SHOTR_ERROR_NO);
			cam_err("[I/DSM] %s LED over-Temperature error",
				__func__);
		}
	}

	return 0;
}

static int hw_find_match_flash_current(int cur_flash, unsigned int ictype)
{
	int cur_level = 0;

	cam_info("%s ernter cur_flash %d, driveric is %d\n",
		__func__, cur_flash,
		ictype);
	if (cur_flash <= 0) {
		cam_err("%s current set is error", __func__);
		return -1;
	}

	if (cur_flash >= LM3644_FLASH_MAX_CUR) {
		cam_warn("%s current set is %d", __func__, cur_flash);
		return LM3644_FLASH_DEFAULT_CUR_LEV;
	}

	switch (ictype) {
	case IC_TYPE_KTD2687:
		cur_level = flash_current_to_level_ktd2687(cur_flash);
		break;
	case IC_TYPE_LM3644:
		cur_level = flash_current_to_level_ti(cur_flash);
		break;
	case IC_TYPE_AW3644:
		cur_level = flash_current_to_level_aw3644(cur_flash);
		break;
	default:
		break;
	};
	return cur_level;
}

static int hw_find_match_torch_current(int cur_torch, unsigned int ictype)
{
	int cur_level = 0;

	cam_info("%s ernter cur_torch %d, driveric is %d\n",
		__func__, cur_torch,
		ictype);
	if (cur_torch <= 0) {
		cam_err("%s current set is error", __func__);
		return -1;
	}

	switch (ictype) {
	case IC_TYPE_KTD2687:
		cur_level = torch_current_to_level_ktd2687(cur_torch);
		break;
	case IC_TYPE_LM3644:
		cur_level = torch_current_to_level_ti(cur_torch);
		break;
	case IC_TYPE_AW3644:
		cur_level = torch_current_to_level_aw3644(cur_torch);
		break;
	default:
		break;
	};

	if (cur_level > LM3644_TORCH_MAX_LEV)
		cur_level = LM3644_TORCH_MAX_LEV;

	return cur_level;
}

static int hw_lm3644_set_pin(struct hw_flash_ctrl_t *flash_ctrl,
	enum lm3644_pin_type_t pin_type, int state)
{
	struct hw_lm3644_private_data_t *pdata = NULL;
	int rc = 0;
	int i;
	const char *gpio_name[MAX_PIN] = { "flash-strobe", "flash-en",
		"flash-torch" };

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}
	if (pin_type < STROBE || pin_type >= MAX_PIN) {
		cam_err("%s error", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
	cam_info("%s pin-type=%d, state=%d", __func__, pin_type, state);

	for (i = STROBE; i < MAX_PIN; i++) {
		if ((pdata->pin[pin_type].pin_id != INVALID_GPIO) &&
			(pdata->pin[i].pin_status == REQUESTED)) {
			if (gpio_request(pdata->pin[i].pin_id, gpio_name[i]) != 0)
				cam_err("%s request gpio[%d] failed",
					__func__, pdata->pin[i].pin_id);
			if (gpio_direction_output(pdata->pin[pin_type].pin_id, state) != 0) {
				cam_err("%s gpio_direction_input failed",
					__func__);
				goto set_pin_fail;
			}
			if (rc < 0)
				cam_err("%s gpio output is err rc=%d", __func__, rc);
		}
	}
	return rc;

set_pin_fail:
	for (i = STROBE; i < MAX_PIN; i++) {
		if (pdata->pin[i].pin_id != INVALID_GPIO) {
			gpio_free(pdata->pin[i].pin_id);
			pdata->pin[i].pin_status = FREED;
		}
	}
	return rc;
}

static int hw_lm3644_init(struct hw_flash_ctrl_t *flash_ctrl)
{
	int i;
	struct hw_lm3644_private_data_t *pdata = NULL;

	cam_debug("%s ernter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	if (g_manual_flag == 1) {
		cam_info("%s g_manual_flag = 1", __func__);
		pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
		for (i = STROBE; i < MAX_PIN; i++) {
			if ((pdata->pin[i].pin_id != INVALID_GPIO) &&
				(pdata->pin[i].pin_status == FREED))
				pdata->pin[i].pin_status = REQUESTED;
		}
	}

	return 0;
}

static int hw_lm3644_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
	int i;
	struct hw_lm3644_private_data_t *pdata = NULL;

	cam_debug("%s ernter\n", __func__);
	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	if (g_manual_flag == 1) {
		cam_info("%s g_manual_flag = 1", __func__);
		pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
		for (i = STROBE; i < MAX_PIN; i++) {
			if ((pdata->pin[i].pin_id != INVALID_GPIO) &&
				(pdata->pin[i].pin_status == REQUESTED)) {
				gpio_free(pdata->pin[i].pin_id);
				pdata->pin[i].pin_status = FREED;
			}
		}
	}

	flash_ctrl->func_tbl->flash_off(flash_ctrl);

	return 0;
}

static int hw_lm3644_flash_mode(struct hw_flash_ctrl_t *flash_ctrl,
	struct hw_flash_cfg_data *cdata)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3644_private_data_t *pdata = NULL;
	unsigned char val_led1;
	unsigned char val_led2;
	int current_level;
	int rc;
	unsigned char regval;
	int data_level;

	if ((!flash_ctrl) || (!flash_ctrl->pdata) ||
		(!flash_ctrl->flash_i2c_client) || (!cdata)) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}
	cam_info("%s data=%d\n", __func__, cdata->data);

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = flash_ctrl->pdata;
	if (pdata->flash_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3644_FLASH_DEFAULT_CUR_LEV;
	} else {
		data_level = (pdata->flash_type == 1) ?
			(cdata->data) : (cdata->data / 2);
		current_level = hw_find_match_flash_current(data_level,
			pdata->driver_ic_type);
		if (current_level < 0)
			current_level = LM3644_FLASH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3644_clear_error_and_notify_dmd(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flashlight clear errorl", __func__);
		return -1;
	}

	/* set LED Flash current value, 7f: 0~6bit is available */
	val_led1 = (pdata->flash_type == 1) ? 0 :
		((unsigned int)current_level & 0x7f);
	val_led2 = ((unsigned int)current_level & 0x7f);
	cam_info("%s led flash current val_led1=0x%x, val_led2=0x%x, current level=%d, flash control time=0x%x",
		__func__, val_led1, val_led2, current_level, pdata->timeout);

	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L1_FL, val_led1) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L2_FL, val_led2) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TIMING_CONFIG,
		pdata->timeout) < 0);

	if (pdata->flash_type == 1)
		regval = LED2_EN | MODE_FLASH;
	else
		regval = LED2_EN | LED1_EN | MODE_FLASH;
	if (cdata->mode == FLASH_STROBE_MODE)
		regval = regval | STROBE_PIN;

	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, regval) < 0);

	return 0;
}

static int hw_lm3644_torch_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3644_private_data_t *pdata = NULL;
	unsigned char val_led1;
	unsigned char val_led2;
	int current_level;
	int rc;
	unsigned char regval;
	int data_level;

	cam_info("%s data=%d.\n", __func__, data);
	if ((!flash_ctrl) || (!flash_ctrl->pdata) ||
		(!flash_ctrl->flash_i2c_client)) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
	if (pdata->torch_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3644_TORCH_DEFAULT_CUR_LEV;
	} else {
		data_level = (pdata->flash_type == 1) ? (data) : (data / 2);
		current_level = hw_find_match_torch_current(data_level,
			pdata->driver_ic_type);
		if (current_level < 0)
			current_level = LM3644_TORCH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3644_clear_error_and_notify_dmd(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flashlight clear errorl", __func__);
		return -1;
	}

	/* set LED torch current value, 7f: 0~6bit is available */
	val_led1 = (pdata->flash_type == 1) ? 0 :
		((unsigned int)current_level & 0x7f);
	val_led2 = ((unsigned int)current_level & 0x7f);
	cam_info("%s the led torch current val_led1=0x%x, val_led2=0x%x, current_level=%d",
		__func__, val_led1, val_led2, current_level);

	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_L1_TORCH, val_led1) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client,
		REG_L2_TORCH, val_led2) < 0);
	if (pdata->flash_type == 1)
		regval = LED2_EN | MODE_TORCH;
	else
		regval = LED2_EN | LED1_EN | MODE_TORCH;

	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, regval) < 0);

	return 0;
}

static int hw_lm3644_torch_mode_mmi(struct hw_flash_ctrl_t *flash_ctrl,
	int mode)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3644_private_data_t *pdata = NULL;
	unsigned char val;
	int current_level = 0;
	int rc = 0;
	int current_torch = 97; // torch current

	cam_info("%s data=%d\n", __func__, mode);
	if ((!flash_ctrl) || (!flash_ctrl->pdata) ||
		(!flash_ctrl->flash_i2c_client)) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
	if (pdata->torch_current == FLASH_LED_LEVEL_INVALID) {
		current_level = LM3644_TORCH_DEFAULT_CUR_LEV;
	} else {
		current_level = hw_find_match_torch_current(current_torch,
			pdata->driver_ic_type);
		if (current_level < 0)
			current_level = LM3644_TORCH_DEFAULT_CUR_LEV;
	}

	rc = hw_lm3644_clear_error_and_notify_dmd(flash_ctrl);
	if (rc < 0) {
		cam_err("%s flashlight clear errorl", __func__);
		return -1;
	}

	/* set LED torch current value */
	val = ((unsigned int)current_level & 0x7f);
	cam_info("%s the led torch current val=0x%x, current_level=%d\n",
		__func__, val, current_level);

	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L1_TORCH, val) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L2_TORCH, val) < 0);
	if (mode == TORCH_LEFT_MODE) {
		loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE,
			MODE_TORCH | LED1_EN) < 0);
	} else {
		loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE,
			MODE_TORCH | LED2_EN) < 0);
	}
	return 0;
}

static int hw_lm3644_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
	int rc = -1;

	cam_debug("%s ernter\n", __func__);
	if ((!flash_ctrl) || (!cdata)) {
		cam_err("%s flash_ctrl or cdata is NULL", __func__);
		return -1;
	}

	cam_info("%s mode=%d, level=%d\n", __func__, cdata->mode, cdata->data);
	mutex_lock(flash_ctrl->hw_flash_mutex);
	if ((cdata->mode == FLASH_MODE) || (cdata->mode == FLASH_STROBE_MODE))
		rc = hw_lm3644_flash_mode(flash_ctrl, cdata);
	else
		rc = hw_lm3644_torch_mode(flash_ctrl, cdata->data);

	flash_ctrl->state.mode = cdata->mode;
	flash_ctrl->state.data = cdata->data;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_lm3644_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	unsigned char val;

	cam_info("%s ernter\n", __func__);
	if ((!flash_ctrl) || (!flash_ctrl->flash_i2c_client) ||
		(!flash_ctrl->flash_i2c_client->i2c_func_tbl)) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	mutex_lock(flash_ctrl->hw_flash_mutex);
	flash_ctrl->state.mode = STANDBY_MODE;
	flash_ctrl->state.data = 0;
	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	if (i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_A, &val) < 0 ||
		i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_B, &val) < 0)
		cam_err("%s %d", __func__, __LINE__);

	if (i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_STANDBY) < 0)
		cam_err("%s %d", __func__, __LINE__);

	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return 0;
}

static int hw_lm3644_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3644_private_data_t *pdata = NULL;
	struct device_node *of_node = NULL;
	int i;
	int rc = -1;
	unsigned int pin_tmp[MAX_PIN] = {0};

	cam_debug("%s enter\n", __func__);
	if ((!flash_ctrl) || (!flash_ctrl->pdata)) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return rc;
	}

	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
	of_node = flash_ctrl->dev->of_node;

	rc = of_property_read_u32(of_node, "huawei,manual-cfg-flag",
		&g_manual_flag);
	if (rc < 0)
		cam_info("%s hisi,manual-cfg-flag failed", __func__);

	cam_info("%s hisi,manual-cfg-flag %d, rc %d\n", __func__,
		g_manual_flag, rc);

	if (g_manual_flag == 1) {
		rc = of_property_read_u32_array(of_node, "huawei,flash-pin",
			pin_tmp, (unsigned long)MAX_PIN);
		if (rc < 0) {
			cam_err("%s get dt flash-pin failed line %d",
				__func__, __LINE__);
			/* if failed, init gpio num as invalid */
			for (i = 0; i < MAX_PIN; i++) {
				pdata->pin[i].pin_id = INVALID_GPIO;
				pdata->pin[i].pin_status = FREED;
			}
			return rc;
		}
		for (i = 0; i < MAX_PIN; i++) {
			pdata->pin[i].pin_id = pin_tmp[i];
			pdata->pin[i].pin_status = FREED;
			cam_info("%s pin[%d] = %u", __func__, i,
				pdata->pin[i].pin_id);
		}
	} else {
		rc = of_property_read_u32(of_node, "huawei,flash-pin",
			&pdata->strobe);
		cam_info("%s hisi,flash-pin %d, rc %d\n", __func__,
			pdata->strobe, rc);
		if (rc < 0)
			cam_err("%s failed %d\n", __func__, __LINE__);
	}

	rc = of_property_read_u32(of_node, "huawei,flash_current",
		&pdata->flash_current);
	cam_info("%s hisi,flash_current %d, rc %d\n", __func__,
		pdata->flash_current, rc);
	if (rc < 0) {
		cam_info("%s failed %d\n", __func__, __LINE__);
		pdata->flash_current = FLASH_LED_LEVEL_INVALID;
	}

	rc = of_property_read_u32(of_node, "huawei,torch_current",
		&pdata->torch_current);
	cam_info("%s hisi,torch_current %d, rc %d\n", __func__,
		pdata->torch_current, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		pdata->torch_current = FLASH_LED_LEVEL_INVALID;
	}
	rc = of_property_read_u32(of_node, "huawei,flash-chipid",
		&pdata->chipid);
	cam_info("%s hisi,chipid 0x%x, rc %d\n", __func__,
		pdata->chipid, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}
	rc = of_property_read_u32(of_node, "huawei,flash-type",
		&pdata->flash_type);
	cam_info("%s hisi,flash_type 0x%x, rc %d\n", __func__,
		pdata->flash_type, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}
	g_flash_type = pdata->flash_type;
	return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_lm3644_lightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;
	int ret;

	ret = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE-1,
		"mode=%d, data=%d\n",
		g_lm3644_ctrl.state.mode,
		g_lm3644_ctrl.state.mode);
	if (ret < 0)
		cam_err("%s %d::snprintf_s return error %d",
			__func__, __LINE__, ret);

	rc = strlen(buf) + 1;
	return rc;
}

static int hw_lm3644_param_check(char *buf, unsigned long *param,
	int num_of_par)
{
	char *token = NULL;
	int base;
	int cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16;
			else
				base = 10;

			if (strict_strtoul(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

static ssize_t hw_lm3644_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[2] = {0};
	int rc;

	rc = hw_lm3644_param_check((char *)buf, param, 2);
	if (rc < 0) {
		cam_err("%s failed to check param", __func__);
		return rc;
	}

	int flash_id = (int)param[0];

	cdata.mode = (int)param[1];
	cam_info("%s flash_id = %d, cdata.mode = %d", __func__,
		flash_id, cdata.mode);
	if (flash_id != 1) {
	/*
	 * bit[0]- rear first flash light
	 * bit[1]- rear sencond flash light
	 * bit[2]- front flash light
	 */
		cam_err("%s wrong flash_id=%d", __func__, flash_id);
		return -1;
	}

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_lm3644_off(&g_lm3644_ctrl);
		if (rc < 0) {
			cam_err("%s lm3644 flash off error", __func__);
			return rc;
		}
	} else if (cdata.mode == TORCH_MODE) {
		/* hardware test requiring the max torch mode current */
		cdata.data = LM3644_TORCH_MAX_CUR;
		cam_info("%s mode = %d, max_current = %d", __func__,
			cdata.mode, cdata.data);

		rc = hw_lm3644_on(&g_lm3644_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s lm3644 flash on error", __func__);
			return rc;
		}
	} else {
		cam_err("%s scharger wrong mode=%d", __func__, cdata.mode);
		return -1;
	}

	return count;
}
#endif

static ssize_t hw_lm3644_flash_mask_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;
	int ret;

	ret = snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE,
		MAX_ATTRIBUTE_BUFFER_SIZE-1,
		"flash_mask_disabled=%d\n",
		g_lm3644_ctrl.flash_mask_enable);
	if (ret < 0)
		cam_err("%s %d::snprintf_s return error %d",
			__func__, __LINE__, ret);

	rc = strlen(buf) + 1;
	return rc;
}

static ssize_t hw_lm3644_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (buf[0] == '0')
		g_lm3644_ctrl.flash_mask_enable = false;
	else
		g_lm3644_ctrl.flash_mask_enable = true;

	cam_debug("%s flash_mask_enable=%d", __func__,
		g_lm3644_ctrl.flash_mask_enable);
	return count;
}

static void hw_lm3644_torch_brightness_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	struct hw_flash_cfg_data cdata;
	int rc;
	unsigned int led_bright = brightness;
	int max_level = 3;

	cam_info("%s brightness=%d\n", __func__, brightness);
	if (led_bright == STANDBY_MODE) {
		rc = hw_lm3644_off(&g_lm3644_ctrl);
		if (rc < 0) {
			cam_err("%s pmu_led off error", __func__);
			return;
		}
	} else {
		/* cmp lm3642, just for mmi */
		cdata.data = MMI_TEST_BRIGHTNESS;
		if (g_flash_type == 1) {
			cdata.mode = TORCH_MODE;
			rc = hw_lm3644_on(&g_lm3644_ctrl, &cdata);
		} else {
			cdata.mode = (flash_mode)(((brightness - 1) /
				max_level) + TORCH_MODE);
			rc = hw_lm3644_torch_mode_mmi(&g_lm3644_ctrl,
				cdata.mode);
		}
		if (rc < 0) {
			cam_err("%s pmu_led on error", __func__);
			return;
		}
	}
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute g_lm3644_lightness =
	__ATTR(flash_lightness, 0664, hw_lm3644_lightness_show,
	hw_lm3644_lightness_store);
#endif

static struct device_attribute g_lm3644_flash_mask =
	__ATTR(flash_mask, 0664, hw_lm3644_flash_mask_show,
	hw_lm3644_flash_mask_store);

static int hw_lm3644_register_attribute(struct hw_flash_ctrl_t *flash_ctrl,
	struct device *dev)
{
	int rc;

	if ((!flash_ctrl) || (!dev)) {
		cam_err("%s flash_ctrl or dev is NULL", __func__);
		return -1;
	}

	flash_ctrl->cdev_torch.name = "torch";
	flash_ctrl->cdev_torch.max_brightness =
		((struct hw_lm3644_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
	flash_ctrl->cdev_torch.brightness_set = hw_lm3644_torch_brightness_set;
	rc = led_classdev_register((struct device *)dev,
		&flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev", __func__);
		goto err_out;
	}
#ifdef CAMERA_FLASH_FACTORY_TEST
	rc = device_create_file(dev, &g_lm3644_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat lightness attribute", __func__);
		goto err_create_lightness_file;
	}
#endif
	rc = device_create_file(dev, &g_lm3644_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute", __func__);
		goto err_create_flash_mask_file;
	}
	return 0;
err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
	device_remove_file(dev, &g_lm3644_lightness);
err_create_lightness_file:
#endif
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
	return rc;
}

static int hw_lm3644_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3644_private_data_t *pdata = NULL;
	unsigned char id = 0;
	unsigned char val = 0;

	cam_debug("%s ernter\n", __func__);
	if ((!flash_ctrl) || (!flash_ctrl->pdata) ||
		(!flash_ctrl->flash_i2c_client) ||
		(!flash_ctrl->flash_i2c_client->i2c_func_tbl) ||
		(!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)) {
		cam_err("%s flash_ctrl is NULL", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;

	if (g_manual_flag == 1) {
		/* change to AW3644tt actual slave address 0x63 */
		i2c_client->client->addr = LM3644_SLAVE_ADDRESS;
		hw_lm3644_set_pin(flash_ctrl, FLASH_EN, HIGH);
	}
	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0);
	switch (id) {
	case LM3644TT_CHIP_ID:
		pdata->driver_ic_type = IC_TYPE_LM3644;
		pdata->timeout = FLASH_TIMEOUT_TI;
		break;
	case KTD2687_CHIP_ID:
		(void)i2c_func->i2c_read(i2c_client, MODE_TEMP, &val);
		if (val == DEF_TEMP_VAL) {
			pdata->driver_ic_type = IC_TYPE_AW3644;
			pdata->timeout = FLASH_TIMEOUT_AW;
		} else {
			pdata->driver_ic_type = IC_TYPE_KTD2687;
			pdata->timeout = FLASH_TIMEOUT_KTD;
		}
		break;
	default:
		cam_info("%s match error 0x%x\n", __func__, id);
		return -1;
	}
	cam_info("%s match ok,driver type = 0x%x\n", __func__,
		pdata->driver_ic_type);

#ifdef CAMERA_FLASH_FACTORY_TEST
	register_camerafs_attr(&g_lm3644_lightness);
#endif
	return 0;
}

static int hw_lm3644_remove(struct i2c_client *client)
{
	cam_debug("%s enter", __func__);

	g_lm3644_ctrl.func_tbl->flash_exit(&g_lm3644_ctrl);

	client->adapter = NULL;
	return 0;
}

static void hw_lm3644_shutdown(struct i2c_client *client)
{
	int rc;

	rc = hw_lm3644_off(&g_lm3644_ctrl);
	cam_info("%s lm3644 shut down at %d", __func__, __LINE__);
	if (rc < 0)
		cam_err("%s lm3644 flash off error", __func__);
}

static const struct i2c_device_id g_lm3644_id[] = {
	{ "lm3644", (unsigned long)&g_lm3644_ctrl },
	{}
};

static const struct of_device_id g_lm3644_dt_match[] = {
	{ .compatible = "huawei,lm3644" },
	{}
};
MODULE_DEVICE_TABLE(of, lm3644_dt_match);

static struct i2c_driver g_lm3644_i2c_driver = {
	.probe = hw_flash_i2c_probe,
	.remove = hw_lm3644_remove,
	.shutdown = hw_lm3644_shutdown,
	.id_table = g_lm3644_id,
	.driver = {
		.name = "hw_lm3644",
		.of_match_table = g_lm3644_dt_match,
	},
};

static int __init hw_lm3644_module_init(void)
{
	cam_info("%s erter\n", __func__);
	return i2c_add_driver(&g_lm3644_i2c_driver);
}

static void __exit hw_lm3644_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&g_lm3644_i2c_driver);
}

static struct hw_flash_i2c_client g_lm3644_i2c_client;

static struct hw_flash_fn_t g_lm3644_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_lm3644_init,
	.flash_exit = hw_lm3644_exit,
	.flash_on = hw_lm3644_on,
	.flash_off = hw_lm3644_off,
	.flash_match = hw_lm3644_match,
	.flash_get_dt_data = hw_lm3644_get_dt_data,
	.flash_register_attribute = hw_lm3644_register_attribute,
};

static struct hw_flash_ctrl_t g_lm3644_ctrl = {
	.flash_i2c_client = &g_lm3644_i2c_client,
	.func_tbl = &g_lm3644_func_tbl,
	.hw_flash_mutex = &flash_mut_lm3644,
	.pdata = (void *)&g_lm3644_pdata,
	.flash_mask_enable = false,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_init(hw_lm3644_module_init);
module_exit(hw_lm3644_module_exit);
MODULE_DESCRIPTION("LM3644 FLASH");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
