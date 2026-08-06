/*
 * lm3646_mix_ir.c
 *
 * driver for lm3646_mix_ir
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

/* lm3646 Registers define */
#define REG_CHIPID 0x00
#define REG_MODE 0x01
#define REG_FLAGS1 0x08
#define REG_FLAGS2 0x09
#define REG_FLASH_TIMEOUT 0x04
#define REG_MAX_CURRENT 0x05
#define REG_FLASH_LED1 0x06
#define REG_TORCH_LED1 0x07

#define CHIP_ID 0x10
#define CHIP_ID_MASK 0x38
#define MODE_STANDBY 0x00
#define MODE_TORCH 0x02
#define MODE_FLASH 0x03
#define STROBE_ENABLE 0x80
#define STROBE_DISABLE 0x00
#define TX_ENABLE 0x08
#define INDUCTOR_CURRENT_LIMMIT 0xe0
#define FLASH_TIMEOUT_TIME 0x47 // 400ms

#define REG_MASK_0XFF 0xFF
#define LM3646_TORCH_MAX_VAL_SHIFT 4
#define LM3646_FLASH_MAX_VAL_MASK 0x0F // (reg 0x05)max flash cur mask
#define LM3646_TORCH_MAX_VAL_MASK 0x70 // (reg 0x05)max torch cur mask
#define LM3646_FLASH_LED1_VAL_MASK 0x7F
#define LM3646_TORCH_LED1_VAL_MASK 0x7F

#define LM3646_TORCH_MAX_CUR 188 // mA
#define LM3646_FLASH_MAX_CUR 1500 // mA
#define MAX_LIGHTNESS_PARAM_NUM_RT 3
#define LIGHTNESS_PARAM_FD_INDEX_RT 0
#define LIGHTNESS_PARAM_MODE_INDEX_RT 1
#define LIGHTNESS_PARAM_CURRENT_INDEX_RT 2
#define MAX_FLASH_ID 0xFF // used to check flash_lightness_sotre input param

#define TOTAL_STEPS_OF_TORCH 8
#define TOTAL_STEPS_OF_FLASH 16

#define MAX_BRIGHTNESS_FORMMI 0x09
#define ENABLE_SHORT_PROTECT 0x80
#define LM3646_RESET_HOLD_TIME 2 // 2ms

#define OVER_VOLTAGE_PROTECT 0x80
#define OVER_CURRENT_PROTECT 0x10
#define OVER_TEMP_PROTECT 0x20
#define LED_SHORT 0x0C
#define UNDER_VOLTAGE_LOCKOUT 0x04

#define COUNT_BASE 100
#define LM3646_LED1_BASE_CUR_OF_FLASH 2304 // 100
#define LM3646_LED1_STEP_CUR_OF_FLASH 1172 // 100
#define LM3646_LED1_BASE_CUR_OF_TORCH 253 // 100
#define LM3646_LED1_STEP_CUR_OF_TORCH 146 // 100
#define LM3646_STANDBY_MODE_CUR 0

/* 1 means I2C control, 0 means GPIO control */
#define LM3646_CTRL_TYPE_GPIO 0
#define LM3646_CTRL_TYPE_I2C 1

#define INVALID_GPIO 999

#define WAKE_LOCK_ENABLE 1
#define WAKE_LOCK_DISABLE 0

#define BACK_FLASH_USE_LED2 1

enum lm3646_mix_ir_pin_type_t {
	RESET = 0,
	STROBE,
	TORCH,
	MAX_PIN,
};

enum lm3646_mix_ir_pin_status_t {
	FREED = 0,
	REQUESTED,
};

struct lm3646_mix_ir_pin_t {
	unsigned int pin_id;
	enum lm3646_mix_ir_pin_status_t pin_status;
};

enum lm3646_mix_ir_current_conf {
	CURRENT_TORCH_LEVEL_MMI_BACK = 0,
	CURRENT_TORCH_LEVEL_MMI_FRONT = 1,
	CURRENT_TORCH_LEVEL_RT_BACK = 2,
	CURRENT_TORCH_LEVEL_RT_FRONT = 3,
	CURRENT_MIX_MAX = 4,
};

/* flash mode for single front or back */
enum lm3646_mix_ir_single_mode_t {
	SINGLE_STANDBY_MODE = 0x00,
	SINGLE_FLASH_MODE = 0x01,
	SINGLE_TORCH_MODE = 0x02,
};

/*
 * flash mode for entire IC which depends on current flash mode of
 * single front and back
 */
enum lm3646_mix_ir_entire_ic_mode_t {
	ENTIRE_IC_STANDBY_MODE = 0,
	ENTIRE_IC_FLASH_MODE = 1,
	ENTIRE_IC_TORCH_MODE = 2,
};

/* ir flash RT self test param */
enum lm3646_mix_ir_selftest_param {
	IR_SELFTEST_ON_TIME_INDEX = 0,
	IR_SELFTEST_OFF_TIME_INDEX = 1,
	IR_SELFTEST_CURRENT_INDEX = 2,
	IR_SELFTEST_TIMES_INDEX = 3,
	IR_SELFTEST_MAX_INDEX = 4,
};

/* Internal data struct define */
struct hw_lm3646_mix_ir_private_data_t {
	struct wakeup_source lm3646_mix_ir_wakelock;
	unsigned int need_wakelock;
	/* flash control pin */
	struct lm3646_mix_ir_pin_t pin[MAX_PIN];
	/* flash electric current config */
	unsigned int ecurrent[CURRENT_MIX_MAX];
	unsigned int selftest_param[IR_SELFTEST_MAX_INDEX];
	unsigned int chipid;
	unsigned int ctrltype;
	unsigned int led_type;
	unsigned int front_cur;
	unsigned int back_cur;
	enum lm3646_mix_ir_single_mode_t front_mode;
	enum lm3646_mix_ir_single_mode_t back_mode;
	enum lm3646_mix_ir_entire_ic_mode_t entire_ic_mode;
};

static unsigned int g_torch_arry[TOTAL_STEPS_OF_TORCH] = {
	24, 47, 70, 94, 117, 141, 164, 188
};

static unsigned int g_flash_arry[TOTAL_STEPS_OF_FLASH] = {
	94, 188, 281, 375,
	469, 563, 656, 750,
	844, 938, 1031, 1125,
	1219, 1313, 1406, 1500
};

/* Internal varible define */
static struct hw_lm3646_mix_ir_private_data_t g_lm3646_mix_ir_pdata;
struct hw_flash_ctrl_t g_lm3646_mix_ir_ctrl;
extern struct dsm_client *client_flash;

define_hisi_flash_mutex(lm3646_mix_ir);

/* Function define */
static int hw_lm3646_mix_ir_param_check(char *buf, unsigned long *param,
	int num_of_par);
static int hw_lm3646_mix_ir_set_pin(struct hw_flash_ctrl_t *flash_ctrl,
	enum lm3646_mix_ir_pin_type_t  pin_type,
	int state)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	int rc = 0;

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	if (pin_type < RESET || pin_type >= MAX_PIN) {
		cam_err("%s total level error", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	cam_debug("%s pin-type = %d, state = %d", __func__, pin_type, state);

	if ((pdata->pin[pin_type].pin_id != INVALID_GPIO)
		&& (pdata->pin[pin_type].pin_status == REQUESTED)) {
		rc = gpio_direction_output(pdata->pin[pin_type].pin_id, state);
		if (rc < 0)
			cam_err("%s gpio output is err rc = %d", __func__, rc);
	}

	return rc;
}

static int gpio_request_assemble(struct hw_lm3646_mix_ir_private_data_t *pdata)
{
	int i;
	int rc = 0;
	const char *gpio_name[MAX_PIN] = {
		"flash-reset",
		"flash-strobe",
		"flash-torch"
	};

	if (!pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	for (i = RESET; i < MAX_PIN; i++) {
		if ((pdata->pin[i].pin_id != INVALID_GPIO)
		    && (pdata->pin[i].pin_status == FREED)) {
			rc = gpio_request(pdata->pin[i].pin_id, gpio_name[i]);
			if (rc < 0) {
				cam_err("%s failed to request pin %s",
					__func__, gpio_name[i]);
				break;
			}
			pdata->pin[i].pin_status = REQUESTED;
		}
	}

	return rc;
}

static int gpio_free_assemble(struct hw_lm3646_mix_ir_private_data_t *pdata)
{
	int i;

	if (!pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	for (i = RESET; i < MAX_PIN; i++) {
		if ((pdata->pin[i].pin_id != INVALID_GPIO)
			&& (pdata->pin[i].pin_status == REQUESTED)) {
			gpio_free(pdata->pin[i].pin_id);
			pdata->pin[i].pin_status = FREED;
		}
	}

	return 0;
}

static int hw_lm3646_mix_ir_init(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	int rc;

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	// init data
	flash_ctrl->flash_type = FLASH_MIX; // mix flash
	flash_ctrl->pctrl = devm_pinctrl_get_select(flash_ctrl->dev,
		PINCTRL_STATE_DEFAULT);

	if (!flash_ctrl->pctrl) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	// stub:check other params which need to init
	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	pdata->front_cur = LM3646_STANDBY_MODE_CUR; // init current as 0
	pdata->back_cur = LM3646_STANDBY_MODE_CUR; // init current as 0
	pdata->front_mode = SINGLE_STANDBY_MODE;
	pdata->back_mode = SINGLE_STANDBY_MODE;
	pdata->entire_ic_mode = ENTIRE_IC_STANDBY_MODE;
	flash_ctrl->state.mode = STANDBY_MODE;
	flash_ctrl->state.data = LM3646_STANDBY_MODE_CUR; // set init value 0

	rc = gpio_request_assemble(pdata);
	if (rc < 0) {
		cam_err("%s failde to request gpio", __func__);
		goto gpio_fail_handler;
	}

	rc = hw_lm3646_mix_ir_set_pin(flash_ctrl, RESET, LOW);
	if (rc < 0) {
		cam_err("%s failed to set reset pin", __func__);
		goto gpio_fail_handler;
	}

	if (pdata->need_wakelock == WAKE_LOCK_ENABLE)
		wakeup_source_init(&pdata->lm3646_mix_ir_wakelock,
		"lm3646_mix_ir");

	return rc;

gpio_fail_handler:
	gpio_free_assemble(pdata);
	return rc;
}

static int hw_lm3646_mix_ir_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;

	cam_debug("%s enter", __func__);

	if (!flash_ctrl || !flash_ctrl->pdata || !flash_ctrl->func_tbl ||
		!flash_ctrl->func_tbl->flash_off) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	flash_ctrl->func_tbl->flash_off(flash_ctrl);
	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	gpio_free_assemble(pdata);

	flash_ctrl->pctrl = devm_pinctrl_get_select(flash_ctrl->dev,
		PINCTRL_STATE_IDLE);

	return 0;
}

static void report_dsm(void)
{
	if (!dsm_client_ocuppy(client_flash)) {
		dsm_client_record(client_flash, "flash i2c transfer fail\n");
		dsm_client_notify(client_flash, DSM_FLASH_I2C_ERROR_NO);
		cam_warn("[I/DSM] %s dsm_client_notify",
		client_flash->client_name);
	}
}

static void check_fault_and_report_dsm(unsigned char fault_val)
{
	if (fault_val & (OVER_VOLTAGE_PROTECT | OVER_CURRENT_PROTECT)) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash OVP or OCP FlagReg1[0x%x]\n",
				fault_val);
			dsm_client_notify(client_flash,
				DSM_FLASH_OPEN_SHOTR_ERROR_NO);
			cam_warn("[I/DSM] %s dsm_client_notify",
				client_flash->client_name);
		}
	}
	if (fault_val & OVER_TEMP_PROTECT) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash temperature is too hot FlagReg1[0x%x]\n",
				fault_val);
			dsm_client_notify(client_flash,
				DSM_FLASH_HOT_DIE_ERROR_NO);
			cam_warn("[I/DSM] %s dsm_client_notify",
				client_flash->client_name);
		}
	}
	if (fault_val & UNDER_VOLTAGE_LOCKOUT) {
		if (!dsm_client_ocuppy(client_flash)) {
			dsm_client_record(client_flash,
				"flash UVLO FlagReg1[0x%x]\n",
				fault_val);
			dsm_client_notify(client_flash,
				DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
			cam_warn("[I/DSM] %s dsm_client_notify",
				client_flash->client_name);
		}
	}
}

static int hw_lm3646_mix_ir_flash_step_match(unsigned int cur, unsigned int *level)
{
	int i;

	if (!level) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	*level = 0;
	for (i = 0; i < TOTAL_STEPS_OF_FLASH - 1; i++) {
		if (cur <= g_flash_arry[i])
			break;
	}
	*level = i;

	cam_info("%s cur = %d, max flash level = 0x%02x",
		__func__, cur, *level);
	return 0;
}

static int hw_lm3646_mix_ir_flash_led1_step_calculate(unsigned int cur,
	unsigned int *level)
{
	if (!level) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	*level = 0;
	if (cur * COUNT_BASE < LM3646_LED1_BASE_CUR_OF_FLASH)
		*level = 0; // level 0, cur is 0
	else
		*level = (cur * 100 - LM3646_LED1_BASE_CUR_OF_FLASH) /
			LM3646_LED1_STEP_CUR_OF_FLASH + 1; // idx from 1

	cam_info("%s cur = %d, led1 flash level = 0x%02x",
		__func__, cur, *level);
	return 0;
}

static int hw_lm3646_mix_ir_flash_mode_cfg_calculate(unsigned char *led1_val,
	unsigned char *max_val, unsigned char *reg_mode,
	struct hw_lm3646_mix_ir_private_data_t *pdata)
{
	unsigned int led1_cur;
	unsigned int led2_cur;
	unsigned int front_cur;
	unsigned int back_cur;
	unsigned int total_cur;
	unsigned int led1_level = 0;
	unsigned int total_level = 0;

	if (!led1_val || !max_val || !reg_mode || !pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	front_cur = pdata->front_cur;
	back_cur  = pdata->back_cur;
	total_cur = front_cur + back_cur;
	hw_lm3646_mix_ir_flash_step_match(total_cur, &total_level);

	if (total_level >= TOTAL_STEPS_OF_FLASH) {
		cam_err("%s total level error", __func__);
		return -EINVAL;
	}

	*max_val  = (unsigned char)(total_level & REG_MASK_0XFF);

	/* both front and back active */
	if ((front_cur != LM3646_STANDBY_MODE_CUR)
		&& (back_cur != LM3646_STANDBY_MODE_CUR)) {
		// 1.try to keep back cur same,
		// update front cur to avoid back flicker
		total_cur = g_flash_arry[total_level];
		front_cur = total_cur - back_cur;
	}

	if (pdata->led_type == BACK_FLASH_USE_LED2) {
		led1_cur = front_cur;
		led2_cur = back_cur;
	} else {
		led1_cur = back_cur;
		led2_cur = front_cur;
	}

	if (led2_cur == LM3646_STANDBY_MODE_CUR) {
		/*
		 * If MAX Current Control Value < LED1 Current Control Value
		 * LED1 = MAX Current Control Value
		 * LED2 = Off
		 */
		*led1_val = LM3646_FLASH_LED1_VAL_MASK;
	} else {
		/*
		 * If LED1 and LED2 Active, Or only LED2 active:
		 * LED1 = LED1 Flash Current Control Value
		 * LED2 = MAX Flash Current Control Value -
		 * LED1 Flash Current Control Value
		 */
		hw_lm3646_mix_ir_flash_led1_step_calculate(led1_cur,
			&led1_level);
		*led1_val = (unsigned char)(led1_level & REG_MASK_0XFF);
	}

	if (pdata->ctrltype == LM3646_CTRL_TYPE_I2C)
		*led1_val = (*led1_val & LM3646_FLASH_LED1_VAL_MASK) |
			STROBE_DISABLE;
	else
		*led1_val = (*led1_val & LM3646_FLASH_LED1_VAL_MASK) |
			STROBE_ENABLE;

	*max_val = (*max_val & LM3646_FLASH_MAX_VAL_MASK) |
		ENABLE_SHORT_PROTECT;
	*reg_mode = INDUCTOR_CURRENT_LIMMIT | TX_ENABLE | MODE_FLASH;

	return 0;
}

static int hw_lm3646_mix_ir_torch_step_match(unsigned int cur,
	unsigned int *level)
{
	int i;

	if (!level) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	*level = 0;
	for (i = 0; i < TOTAL_STEPS_OF_TORCH - 1; i++) {
		if (cur <= g_torch_arry[i])
			break;
	}
	*level = i;

	cam_info("%s cur = %d, max torch level = 0x%02x",
		__func__, cur, *level);

	return 0;
}

static int hw_lm3646_mix_ir_torch_led1_step_calculate(unsigned int cur,
	unsigned int *level)

{
	if (!level) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	*level = 0;
	if (cur * COUNT_BASE < LM3646_LED1_BASE_CUR_OF_TORCH)
		*level = 0;
	else
		*level = (cur * 100 - LM3646_LED1_BASE_CUR_OF_TORCH) /
			LM3646_LED1_STEP_CUR_OF_TORCH + 1;

	cam_info("%s cur = %d, led1 torch level = 0x%02x",
		__func__, cur, *level);

	return 0;
}

static int hw_lm3646_mix_ir_torch_mode_cfg_calculate(unsigned char *led1_val,
	unsigned char *max_val, unsigned char *reg_mode,
	struct hw_lm3646_mix_ir_private_data_t *pdata)
{
	unsigned int led1_cur;
	unsigned int led2_cur;
	unsigned int front_cur;
	unsigned int back_cur;
	unsigned int total_cur;
	unsigned int led1_level = 0;
	unsigned int total_level = 0;

	if (!led1_val || !max_val || !reg_mode || !pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	front_cur = pdata->front_cur;
	back_cur  = pdata->back_cur;
	total_cur = front_cur + back_cur;
	hw_lm3646_mix_ir_torch_step_match(total_cur, &total_level);

	if (total_level >= TOTAL_STEPS_OF_TORCH) {
		cam_err("%s total level error", __func__);
		return -EINVAL;
	}

	*max_val  = (unsigned char)(total_level & REG_MASK_0XFF);

	/* both front and back active */
	if ((front_cur != LM3646_STANDBY_MODE_CUR)
		&& (back_cur != LM3646_STANDBY_MODE_CUR)) {
		/*
		 * 1.try to keep back cur same,
		 * update front cur to avoid back flicker
		 */
		total_cur = g_torch_arry[total_level];
		front_cur = total_cur - back_cur;
	}

	if (pdata->led_type == BACK_FLASH_USE_LED2) {
		led1_cur = front_cur;
		led2_cur = back_cur;
	} else {
		led1_cur = back_cur;
		led2_cur = front_cur;
	}

	if (led2_cur == LM3646_STANDBY_MODE_CUR) {
		/*
		 * If MAX Torch Current Control Value < LED1
		 * Torch Current Control Value
		 * LED1 = MAX Torch Current Control Value
		 * LED2 = Off
		 */
		*led1_val = LM3646_TORCH_LED1_VAL_MASK;
	} else {
		/*
		 * If LED1 and LED2 Active, Or only LED2 active:
		 * LED1 = LED1 Torch Current Control Value
		 * LED2 = MAX Torch Current Control Value -
		 * LED1 Torch Current Control Value
		 */
		hw_lm3646_mix_ir_torch_led1_step_calculate(led1_cur,
			&led1_level);
		*led1_val = (unsigned char)(led1_level & REG_MASK_0XFF);
	}

	if (pdata->ctrltype == LM3646_CTRL_TYPE_I2C)
		*led1_val = (*led1_val & LM3646_TORCH_LED1_VAL_MASK) |
			STROBE_DISABLE;
	else
		*led1_val = (*led1_val & LM3646_TORCH_LED1_VAL_MASK) |
			STROBE_ENABLE;

	*max_val  = ((*max_val << LM3646_TORCH_MAX_VAL_SHIFT) &
		LM3646_TORCH_MAX_VAL_MASK) | ENABLE_SHORT_PROTECT;
	*reg_mode = INDUCTOR_CURRENT_LIMMIT | TX_ENABLE | MODE_TORCH;

	return 0;
}

static int hw_lm3646_mix_ir_flash_mode_regs_cfg(struct hw_flash_ctrl_t
	*flash_ctrl, unsigned char led1_val, unsigned char max_val,
	unsigned char reg_mode)
{
	int rc = 0;
	unsigned char reg_fault_clean = 0;
	unsigned char reg_max_val_tmp = 0;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_flash_i2c_client *i2c_client = NULL;

	if (!flash_ctrl || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = i2c_client->i2c_func_tbl;

	/* clear error status */
	i2c_func->i2c_read(i2c_client, REG_FLAGS1, &reg_fault_clean);
	check_fault_and_report_dsm(reg_fault_clean);
	i2c_func->i2c_read(i2c_client, REG_FLAGS2, &reg_fault_clean);

	rc += i2c_func->i2c_read(i2c_client, REG_MAX_CURRENT, &reg_max_val_tmp);
	/*
	 * keep the current value of flash mode to avoid flicker
	 * when mode convert from flash to torch
	 */
	max_val = (reg_max_val_tmp & LM3646_TORCH_MAX_VAL_MASK) | max_val;

	rc += i2c_func->i2c_write(i2c_client, REG_FLASH_TIMEOUT,
		FLASH_TIMEOUT_TIME);
	rc += i2c_func->i2c_write(i2c_client, REG_MAX_CURRENT, max_val);
	rc += i2c_func->i2c_write(i2c_client, REG_FLASH_LED1, led1_val);
	rc += i2c_func->i2c_write(i2c_client, REG_MODE, reg_mode);

	cam_info("%s config flash1 = 0x%02x, max_val = 0x%02x, reg_mode = 0x%02x",
		__func__, led1_val, max_val, reg_mode);

	if (rc < 0)
		report_dsm();

	return rc;
}

static int hw_lm3646_mix_ir_torch_mode_regs_cfg(struct hw_flash_ctrl_t
	*flash_ctrl, unsigned char led1_val, unsigned char max_val,
	unsigned char reg_mode)
{
	int rc = 0;
	unsigned char reg_fault_clean = 0;
	unsigned char reg_max_val_tmp = 0;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_flash_i2c_client *i2c_client = NULL;

	if (!flash_ctrl || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = i2c_client->i2c_func_tbl;

	/* clear error status */
	i2c_func->i2c_read(i2c_client, REG_FLAGS1, &reg_fault_clean);
	check_fault_and_report_dsm(reg_fault_clean);
	i2c_func->i2c_read(i2c_client, REG_FLAGS2, &reg_fault_clean);

	rc += i2c_func->i2c_read(i2c_client, REG_MAX_CURRENT, &reg_max_val_tmp);
	/*
	 * keep the current value of flash mode to avoid flicker
	 * when mode convert from flash to torch
	 */
	max_val = (reg_max_val_tmp & LM3646_FLASH_MAX_VAL_MASK) | max_val;

	rc += i2c_func->i2c_write(i2c_client, REG_MAX_CURRENT, max_val);
	rc += i2c_func->i2c_write(i2c_client, REG_TORCH_LED1, led1_val);
	rc += i2c_func->i2c_write(i2c_client, REG_MODE, reg_mode);

	cam_info("%s config torch1 = 0x%02x, max_val = 0x%02x, reg_mode = 0x%02x",
		__func__, led1_val, max_val, reg_mode);

	if (rc < 0)
		report_dsm();

	return rc;
}

static int hw_lm3646_mix_ir_action_ic_standby(struct hw_flash_ctrl_t *flash_ctrl)
{
	int rc = 0;
	unsigned char reg_fault_clean = 0;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;

	if (!flash_ctrl || !flash_ctrl->pdata || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = i2c_client->i2c_func_tbl;

	/* read back error status */
	i2c_func->i2c_read(i2c_client, REG_FLAGS1, &reg_fault_clean);
	check_fault_and_report_dsm(reg_fault_clean);
	i2c_func->i2c_read(i2c_client, REG_FLAGS2, &reg_fault_clean);

	rc += i2c_func->i2c_write(i2c_client, REG_MODE, MODE_STANDBY);
	if (rc < 0)
		report_dsm();

	hw_lm3646_mix_ir_set_pin(flash_ctrl, STROBE, LOW);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, TORCH, LOW);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, RESET, LOW);

	if (pdata->need_wakelock == WAKE_LOCK_ENABLE)
		/*
		 * wake_unlock() will check whether the wakeup source is active
		 * before doing the unlock operation, so there is no dangers
		 * for the mutex being unlocked before locked
		 */
		__pm_relax(&pdata->lm3646_mix_ir_wakelock);

	return rc;
}

static int hw_lm3646_mix_ir_action_ic_flash_mode(struct hw_flash_ctrl_t *flash_ctrl)
{
	int rc;
	unsigned char reg_mode = 0;
	unsigned char led1_val = 0;
	unsigned char max_val = 0; // led2_val = max_val - led1_val
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;

	hw_lm3646_mix_ir_set_pin(flash_ctrl, RESET, HIGH);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, STROBE, LOW);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, TORCH, LOW);

	rc = hw_lm3646_mix_ir_flash_mode_cfg_calculate(&led1_val, &max_val,
		&reg_mode, pdata);
	if (rc < 0) {
		cam_err("%s flash mode cfg calculate faild", __func__);
		return rc;
	}

	rc = hw_lm3646_mix_ir_flash_mode_regs_cfg(flash_ctrl, led1_val,
		max_val, reg_mode);
	if (rc < 0) {
		cam_err("%s flash mode regs cfg faild", __func__);
		return rc;
	}

	hw_lm3646_mix_ir_set_pin(flash_ctrl, STROBE, HIGH);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, TORCH,  HIGH);

	return rc;
}

static int hw_lm3646_mix_ir_action_ic_torch_mode(struct hw_flash_ctrl_t *flash_ctrl)
{
	int rc;
	unsigned char led1_val = 0;
	unsigned char max_val = 0;
	unsigned char reg_mode = 0;
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;

	hw_lm3646_mix_ir_set_pin(flash_ctrl, RESET, HIGH);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, STROBE, LOW);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, TORCH, LOW);

	rc = hw_lm3646_mix_ir_torch_mode_cfg_calculate(&led1_val, &max_val,
		&reg_mode, pdata);
	if (rc < 0) {
		cam_err("%s torch mode cfg calculate faild", __func__);
		return rc;
	}

	rc = hw_lm3646_mix_ir_torch_mode_regs_cfg(flash_ctrl, led1_val,
		max_val, reg_mode);
	if (rc < 0) {
		cam_err("%s torch mode reg cfg faild", __func__);
		return rc;
	}

	hw_lm3646_mix_ir_set_pin(flash_ctrl, STROBE, HIGH);
	hw_lm3646_mix_ir_set_pin(flash_ctrl, TORCH, HIGH);

	return rc;
}

static int hw_lm3646_mix_ir_update_inner_status(struct hw_flash_ctrl_t *flash_ctrl,
	struct hw_flash_cfg_data *cdata)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	enum lm3646_mix_ir_single_mode_t inner_mode  = SINGLE_STANDBY_MODE;
	unsigned int cur;

	if (!flash_ctrl || !flash_ctrl->pdata || !cdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	cur = (unsigned int)(cdata->data);

	/*
	 * limit the max value when we calculate levels
	 * in the corresponding mode
	 */
	switch (cdata->mode) {
	case STANDBY_MODE:
		inner_mode = SINGLE_STANDBY_MODE;
		/*
		 * flash_ctrl->state will be update
		 * when ic standby mode cfg is finished
		 */
		break;
	case FLASH_MODE:
		inner_mode = SINGLE_FLASH_MODE;
		/*
		 * flash_ctrl->state will be update here,
		 * and may be used in thermal protect
		 */
		flash_ctrl->state.mode = cdata->mode;
		flash_ctrl->state.data = cdata->data;
		break;
	default:
		inner_mode = SINGLE_TORCH_MODE; // set as default
		/*
		 * flash_ctrl->state will be update here,
		 * and may be used in thermal protect
		 */
		flash_ctrl->state.mode = cdata->mode;
		flash_ctrl->state.data = cdata->data;
		break;
	}

	if (flash_ctrl->mix_pos == HWFLASH_POSITION_FORE) {
		pdata->front_mode = inner_mode;
		pdata->front_cur = cur;
		cam_info("%s update front mode:%d, front cur %u", __func__,
			pdata->front_mode, pdata->front_cur);
	} else {
		pdata->back_mode = inner_mode;
		pdata->back_cur = cur;
		cam_info("%s update back mode:%d, back cur %u",  __func__,
			pdata->back_mode, pdata->back_cur);
	}

	return 0;
}

static int hw_lm3646_mix_ir_get_work_mode_strategy(struct hw_flash_ctrl_t *flash_ctrl,
	enum lm3646_mix_ir_entire_ic_mode_t *ic_work_mode)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	unsigned int front_mode = SINGLE_STANDBY_MODE;
	unsigned int back_mode  = SINGLE_STANDBY_MODE;

	if (!flash_ctrl || !flash_ctrl->pdata || !ic_work_mode) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	front_mode = pdata->front_mode;
	back_mode  = pdata->back_mode;
	*ic_work_mode = ENTIRE_IC_STANDBY_MODE; // set as default

	if ((front_mode | back_mode) == SINGLE_STANDBY_MODE)
		*ic_work_mode = ENTIRE_IC_STANDBY_MODE;
	else if (((front_mode | back_mode) & SINGLE_FLASH_MODE) == SINGLE_FLASH_MODE)
		*ic_work_mode = ENTIRE_IC_FLASH_MODE;
	else if (((front_mode | back_mode) & SINGLE_TORCH_MODE) == SINGLE_TORCH_MODE)
		*ic_work_mode = ENTIRE_IC_TORCH_MODE;

	if (pdata->front_cur == LM3646_STANDBY_MODE_CUR &&
		pdata->back_cur == LM3646_STANDBY_MODE_CUR)
		*ic_work_mode = ENTIRE_IC_STANDBY_MODE;

	return 0;
}

static int hw_lm3646_mix_ir_update_work_mode_dispatch(struct hw_flash_ctrl_t *flash_ctrl,
	enum lm3646_mix_ir_entire_ic_mode_t ic_work_mode)
{
	int rc = 0;
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;

	switch (ic_work_mode) {
	case ENTIRE_IC_STANDBY_MODE:
		if (pdata->entire_ic_mode != ENTIRE_IC_STANDBY_MODE) {
			// only when current mode is not in standby,
			// action the standby mode
			rc = hw_lm3646_mix_ir_action_ic_standby(flash_ctrl);
			pdata->entire_ic_mode = ENTIRE_IC_STANDBY_MODE;
			flash_ctrl->state.mode = STANDBY_MODE;
			flash_ctrl->state.data = LM3646_STANDBY_MODE_CUR;
		}
		break;
	case ENTIRE_IC_FLASH_MODE:
		rc = hw_lm3646_mix_ir_action_ic_flash_mode(flash_ctrl);
		pdata->entire_ic_mode = ENTIRE_IC_FLASH_MODE;
		break;
	case ENTIRE_IC_TORCH_MODE:
		rc = hw_lm3646_mix_ir_action_ic_torch_mode(flash_ctrl);
		pdata->entire_ic_mode = ENTIRE_IC_TORCH_MODE;
		break;
	default:
		cam_err("%s invalid ic work mode", __func__);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int hw_lm3646_mix_ir_update_work_mode(struct hw_flash_ctrl_t *flash_ctrl,
	struct hw_flash_cfg_data *cdata)
{
	int rc;
	enum lm3646_mix_ir_entire_ic_mode_t ic_work_mode = ENTIRE_IC_STANDBY_MODE;

	if (!flash_ctrl || !cdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	rc = hw_lm3646_mix_ir_update_inner_status(flash_ctrl, cdata);
	if (rc < 0) {
		cam_err("%s update inner status failed", __func__);
		goto work_mode_err;
	}

	rc = hw_lm3646_mix_ir_get_work_mode_strategy(flash_ctrl,
		&ic_work_mode);
	if (rc < 0) {
		cam_err("%s get work mode strategy failed", __func__);
		goto work_mode_err;
	}

	rc = hw_lm3646_mix_ir_update_work_mode_dispatch(flash_ctrl,
		ic_work_mode);
	if (rc < 0) {
		cam_err("%s update work mode dispatch failed", __func__);
		goto work_mode_err;
	}

	return rc;

work_mode_err:
	// error handler may need to add here
	return rc;
}

static int hw_lm3646_mix_ir_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	struct hw_flash_cfg_data *cdata = NULL;
	int rc;

	if (!flash_ctrl || !flash_ctrl->pdata || !data) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	cdata = (struct hw_flash_cfg_data *)data;
	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;

	cam_info("%s mode = %d, cur = %d", __func__, cdata->mode, cdata->data);

	mutex_lock(flash_ctrl->hw_flash_mutex);

	if (pdata->need_wakelock == WAKE_LOCK_ENABLE)
		/*
		 * if the wake lock source has already been active,
		 * repeat call of
		 * 'wake_lock' will just add the event_count
		 * of the wake lock source,
		 * and will not cause evil effects.
		 */
		__pm_stay_awake(&pdata->lm3646_mix_ir_wakelock);

	rc = hw_lm3646_mix_ir_update_work_mode(flash_ctrl, cdata);

	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_lm3646_mix_ir_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_cfg_data cdata = {0};
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	int rc = 0;

	if (!flash_ctrl || !flash_ctrl->pdata) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	cam_debug("%s enter", __func__);

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	cdata.mode = SINGLE_STANDBY_MODE;
	cdata.data = LM3646_STANDBY_MODE_CUR; // set standby current as 0

	mutex_lock(flash_ctrl->hw_flash_mutex);
	rc = hw_lm3646_mix_ir_update_work_mode(flash_ctrl, &cdata);
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	cam_info("%s end", __func__);

	return rc;
}

static int hw_lm3646_mix_ir_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	struct device_node *node = NULL;
	int i;
	int rc = -EINVAL;
	unsigned int pin_tmp[MAX_PIN] = {0};

	if (!flash_ctrl || !flash_ctrl->pdata || !flash_ctrl->dev ||
		!flash_ctrl->dev->of_node) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;
	node = flash_ctrl->dev->of_node;

	rc = of_property_read_u32_array(node, "huawei,flash-pin",
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
		cam_info("%s pin[%d] = %u", __func__,
			i, pdata->pin[i].pin_id);
	}

	rc = of_property_read_u32(node, "huawei,flash-chipid",
		&pdata->chipid);
	cam_info("%s hisi, chipid 0x%x, rc %d", __func__,
		pdata->chipid, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(node, "huawei,flash-ctrltype",
		&pdata->ctrltype);
	cam_info("%s hisi,ctrltype 0x%x, rc %d", __func__,
		pdata->ctrltype, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(node, "huawei,led-type",
		&pdata->led_type);
	cam_info("%s huawei,led-type %d, rc %d\n", __func__,
		pdata->led_type, rc);
	if (rc < 0) {
		cam_err("%s read led-type failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32(node, "huawei,need-wakelock",
		(u32 *)&pdata->need_wakelock);
	cam_info("%s huawei, need-wakelock %d, rc %d\n",
		__func__, pdata->need_wakelock, rc);
	if (rc < 0) {
		pdata->need_wakelock = WAKE_LOCK_DISABLE;
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}

	rc = of_property_read_u32_array(node, "huawei,flash-current",
		pdata->ecurrent, CURRENT_MIX_MAX);
	if (rc < 0) {
		cam_err("%s read flash-current failed line %d\n",
			__func__, __LINE__);
		return rc;
	}
	for (i = 0; i < CURRENT_MIX_MAX; i++)
		cam_info("%s ecurrent[%d] = %d\n", __func__, i,
			pdata->ecurrent[i]);

	rc = of_property_read_u32_array(node, "huawei,selftest-param",
		pdata->selftest_param, IR_SELFTEST_MAX_INDEX);
	if (rc < 0) {
		cam_err("%s read selftest-param failed line %d\n",
			__func__, __LINE__);
		return rc;
	}
	for (i = 0; i < IR_SELFTEST_MAX_INDEX; i++)
		cam_info("%s selftest_param[%d] = %d\n", __func__, i,
			pdata->selftest_param[i]);

	return rc;
}

static ssize_t hw_lm3646_mix_ir_flash_lightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;

	if (!buf) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	// PAGE_SIZE = 4096
	rc = scnprintf(buf, PAGE_SIZE,
		"front_mode=%d, data=%d; back_mode=%d, data=%d\n",
		g_lm3646_mix_ir_pdata.front_mode,
		g_lm3646_mix_ir_pdata.front_cur,
		g_lm3646_mix_ir_pdata.back_mode,
		g_lm3646_mix_ir_pdata.back_cur);

	return rc;
}

static ssize_t hw_lm3646_mix_ir_flash_lightness_show_f(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;

	if (!buf) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	// show for ir selftest in RT, PAGE_SIZE = 4096
	rc = scnprintf(buf, PAGE_SIZE, "%d %d %d %d",
		g_lm3646_mix_ir_pdata.selftest_param[IR_SELFTEST_ON_TIME_INDEX],
		g_lm3646_mix_ir_pdata.selftest_param[IR_SELFTEST_OFF_TIME_INDEX],
		g_lm3646_mix_ir_pdata.selftest_param[IR_SELFTEST_CURRENT_INDEX],
		g_lm3646_mix_ir_pdata.selftest_param[IR_SELFTEST_TIMES_INDEX]);

	return rc;
}

static int hw_lm3646_mix_ir_param_check(char *buf, unsigned long *param,
	int num_of_par)
{
	char *token = NULL;
	unsigned int base;
	int cnt;

	if (!buf || !param) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

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

static ssize_t hw_lm3646_mix_ir_flash_lightness_store_imp(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[MAX_LIGHTNESS_PARAM_NUM_RT] = {0};
	int rc;
	int flash_id;

	if (!buf) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	rc = hw_lm3646_mix_ir_param_check((char *)buf, param,
		MAX_LIGHTNESS_PARAM_NUM_RT);
	if (rc < 0) {
		cam_err("%s failed to check param", __func__);
		return rc;
	}

	flash_id = (int)param[LIGHTNESS_PARAM_FD_INDEX_RT]; // 0 - flash id
	cdata.mode = (int)param[LIGHTNESS_PARAM_MODE_INDEX_RT]; // 1 - mode
	// 2 - current
	cdata.data = (int)param[LIGHTNESS_PARAM_CURRENT_INDEX_RT];
	cam_info("%s flash_id = %d, cdata.mode = %d, cdata.data = %d",
		__func__, flash_id, cdata.mode, cdata.data);

	if ((cdata.mode < STANDBY_MODE) || (cdata.mode > MAX_MODE)) {
		cam_err("%s out of bounds: %d", __func__, cdata.mode);
		return -EINVAL;
	}
	// 0~1500 mA
	if ((cdata.data < 0) || (cdata.data > LM3646_FLASH_MAX_CUR)) {
		cam_err("%s out of bounds: %d", __func__, cdata.data);
		return -EINVAL;
	}
	// 0~0xFF, pass from HAL, but not use just print
	if ((flash_id < 0) || (flash_id > MAX_FLASH_ID)) {
		cam_err("%s out of bounds: %d", __func__, flash_id);
		return -EINVAL;
	}

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_lm3646_mix_ir_off(&g_lm3646_mix_ir_ctrl);
		if (rc < 0) {
			cam_err("%s flash off error", __func__);
			return rc;
		}
	} else if (cdata.mode == TORCH_MODE) {
		if (g_lm3646_mix_ir_ctrl.mix_pos == HWFLASH_POSITION_FORE)
			cdata.data =
				g_lm3646_mix_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_RT_FRONT];
		else
			cdata.data =
				g_lm3646_mix_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_RT_BACK];

		cam_info("%s mode = %d, max_current = %d",
			__func__, cdata.mode, cdata.data);

		rc = hw_lm3646_mix_ir_on(&g_lm3646_mix_ir_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s flash on error", __func__);
			return rc;
		}
	} else if (cdata.mode == FLASH_MODE) {
		cam_info("%s mode = %d, current = %d",
			__func__, cdata.mode, cdata.data);
		rc = hw_lm3646_mix_ir_on(&g_lm3646_mix_ir_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s flash on error", __func__);
			return rc;
		}
	} else {
		cam_err("%s wrong mode = %d", __func__, cdata.mode);
		return -EINVAL;
	}

	return count;
}

static ssize_t hw_lm3646_mix_ir_flash_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	// call back flash
	g_lm3646_mix_ir_ctrl.mix_pos = HWFLASH_POSITION_REAR;
	return hw_lm3646_mix_ir_flash_lightness_store_imp(dev, attr,
		buf, count);
}

static ssize_t hw_lm3646_mix_ir_flash_lightness_f_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	// call front flash
	g_lm3646_mix_ir_ctrl.mix_pos = HWFLASH_POSITION_FORE;
	return hw_lm3646_mix_ir_flash_lightness_store_imp(dev, attr,
		buf, count);
}

static ssize_t hw_lm3646_mix_ir_flash_mask_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int rc;

	if (!buf) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	rc = scnprintf(buf, PAGE_SIZE, "flash_mask_disabled = %d\n",
		g_lm3646_mix_ir_ctrl.flash_mask_enable);

	return rc;
}

static ssize_t hw_lm3646_mix_ir_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (!buf) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	if (buf[0] == '0') // char '0' for mask disable
		g_lm3646_mix_ir_ctrl.flash_mask_enable = false;
	else
		g_lm3646_mix_ir_ctrl.flash_mask_enable = true;

	cam_info("%s flash_mask_enable = %d", __func__,
		g_lm3646_mix_ir_ctrl.flash_mask_enable);
	return (ssize_t)count;
}

static void hw_lm3646_mix_ir_torch_brightness_set_imp(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	int rc;
	struct hw_flash_cfg_data cdata = {0};
	unsigned int led_bright = brightness;

	cam_info("%s brightness = %d", __func__, brightness);

	if (led_bright == LED_OFF) {
		rc = hw_lm3646_mix_ir_off(&g_lm3646_mix_ir_ctrl);
		if (rc < 0) {
			cam_err("%s pmu_led off error", __func__);
			return;
		}
	} else {
		cdata.mode = TORCH_MODE;
		if (g_lm3646_mix_ir_ctrl.mix_pos == HWFLASH_POSITION_FORE)
			cdata.data =
				g_lm3646_mix_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_MMI_FRONT];
		else
			cdata.data =
				g_lm3646_mix_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_MMI_BACK];

		cam_info("%s brightness = 0x%x, mode = %d, data = %d",
			__func__, brightness, cdata.mode, cdata.data);
		rc = hw_lm3646_mix_ir_on(&g_lm3646_mix_ir_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s flash on error", __func__);
			return;
		}
	}
}

static void hw_lm3646_mix_ir_torch_brightness_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	g_lm3646_mix_ir_ctrl.mix_pos = HWFLASH_POSITION_REAR;
	hw_lm3646_mix_ir_torch_brightness_set_imp(cdev, brightness);
}

static void hw_lm3646_mix_ir_torch_brightness_f_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	g_lm3646_mix_ir_ctrl.mix_pos = HWFLASH_POSITION_FORE;
	hw_lm3646_mix_ir_torch_brightness_set_imp(cdev, brightness);
}

// for RT
// 660:-wr-wr---
static struct device_attribute g_lm3646_mix_ir_flash_lightness =
	__ATTR(flash_lightness,
	0660, hw_lm3646_mix_ir_flash_lightness_show,
	hw_lm3646_mix_ir_flash_lightness_store);

// 660:-wr-wr---
static struct device_attribute g_lm3646_mix_ir_flash_lightness_f =
	__ATTR(flash_lightness_f,
	0660, hw_lm3646_mix_ir_flash_lightness_show_f,
	hw_lm3646_mix_ir_flash_lightness_f_store);

// 660:-wr-wr---
static struct device_attribute g_lm3646_mix_ir_flash_mask = __ATTR(flash_mask,
	0660, hw_lm3646_mix_ir_flash_mask_show,
	hw_lm3646_mix_ir_flash_mask_store);

extern int register_camerafs_attr(struct device_attribute *attr);
static int hw_lm3646_mix_ir_register_attribute(struct hw_flash_ctrl_t *flash_ctrl,
	struct device *dev)
{
	int rc;

	if (!flash_ctrl || !dev) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	register_camerafs_attr(&g_lm3646_mix_ir_flash_lightness);
	register_camerafs_attr(&g_lm3646_mix_ir_flash_lightness_f);

	flash_ctrl->cdev_torch.name = "torch";
	flash_ctrl->cdev_torch.max_brightness =
		(enum led_brightness)MAX_BRIGHTNESS_FORMMI;
	flash_ctrl->cdev_torch.brightness_set =
		hw_lm3646_mix_ir_torch_brightness_set;
	rc = led_classdev_register((struct device *)dev,
		&flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev", __func__);
		goto err_out;
	}

	flash_ctrl->cdev_torch1.name = "torch_front";
	flash_ctrl->cdev_torch1.max_brightness
		= (enum led_brightness)MAX_BRIGHTNESS_FORMMI;
	flash_ctrl->cdev_torch1.brightness_set =
		hw_lm3646_mix_ir_torch_brightness_f_set;
	rc = led_classdev_register((struct device *)dev,
		&flash_ctrl->cdev_torch1);
	if (rc < 0) {
		cam_err("%s failed to register torch_front classdev", __func__);
		goto err_create_torch_front_file;
	}

	rc = device_create_file(dev, &g_lm3646_mix_ir_flash_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat flash_lightness attribute",
			__func__);
		goto err_create_flash_lightness_file;
	}

	rc = device_create_file(dev, &g_lm3646_mix_ir_flash_lightness_f);
	if (rc < 0) {
		cam_err("%s failed to creat flash_f_lightness attribute",
			__func__);
		goto err_create_flash_f_lightness_file;
	}

	rc = device_create_file(dev, &g_lm3646_mix_ir_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute", __func__);
		goto err_create_flash_mask_file;
	}

	return 0;

err_create_flash_mask_file:
	device_remove_file(dev, &g_lm3646_mix_ir_flash_lightness_f);
err_create_flash_f_lightness_file:
	device_remove_file(dev, &g_lm3646_mix_ir_flash_lightness);
err_create_flash_lightness_file:
	led_classdev_unregister(&flash_ctrl->cdev_torch1);
err_create_torch_front_file:
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
	return rc;
}

static int hw_lm3646_mix_ir_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client = NULL;
	struct hw_flash_i2c_fn_t *i2c_func = NULL;
	struct hw_lm3646_mix_ir_private_data_t *pdata = NULL;
	unsigned char id = 0;

	if (!flash_ctrl || !flash_ctrl->pdata || !flash_ctrl->flash_i2c_client ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl ||
		!flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read) {
		cam_err("%s invalid params", __func__);
		return -EINVAL;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3646_mix_ir_private_data_t *)flash_ctrl->pdata;

	/*
	 * Enable lm3646_mix_ir switch to standby current is 10ua,
	 * if match id success, reset pin will always be enabled.
	 */
	hw_lm3646_mix_ir_set_pin(flash_ctrl, RESET, HIGH);
	i2c_func->i2c_read(i2c_client, REG_CHIPID, &id);
	cam_info("%s id = 0x%x", __func__, id);
	id = id & CHIP_ID_MASK;
	if (id != CHIP_ID) {
		cam_err("%s match error, id 0x%x != 0x%x",
			__func__, id, CHIP_ID);

		/*
		 * Enable lm3646_mix_ir switch to shutdown
		 * when matchid fail, current is 1.3ua
		 */
		hw_lm3646_mix_ir_set_pin(flash_ctrl, RESET, LOW);
		return -EINVAL;
	}

	return 0;
}

static int hw_lm3646_mix_ir_remove(struct i2c_client *client)
{
	cam_debug("%s enter", __func__);
	if (!client) {
		cam_err("%s client is NULL", __func__);
		return -EINVAL;
	}
	g_lm3646_mix_ir_ctrl.func_tbl->flash_exit(&g_lm3646_mix_ir_ctrl);

	client->adapter = NULL;
	return 0;
}

static const struct i2c_device_id g_lm3646_mix_ir_id[] = {
	{ "lm3646_mix_ir", (unsigned long) &g_lm3646_mix_ir_ctrl },
	{}
};

static const struct of_device_id g_lm3646_mix_ir_dt_match[] = {
	{ .compatible = "huawei,lm3646_mix_ir" },
	{}
};
MODULE_DEVICE_TABLE(of, lm3646_mix_ir_dt_match);

static struct i2c_driver g_lm3646_mix_ir_i2c_driver = {
	.probe = hw_flash_i2c_probe,
	.remove = hw_lm3646_mix_ir_remove,
	.id_table = g_lm3646_mix_ir_id,
	.driver = {
		.name = "hw_lm3646_mix_ir",
		.of_match_table = g_lm3646_mix_ir_dt_match,
	},
};

static int __init hw_lm3646_mix_ir_module_init(void)
{
	cam_info("%s erter", __func__);
	return i2c_add_driver(&g_lm3646_mix_ir_i2c_driver);
}

static void __exit hw_lm3646_mix_ir_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&g_lm3646_mix_ir_i2c_driver);
}

static struct hw_flash_i2c_client g_lm3646_mix_ir_i2c_client;

static struct hw_flash_fn_t g_lm3646_mix_ir_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_lm3646_mix_ir_init,
	.flash_exit = hw_lm3646_mix_ir_exit,
	.flash_on = hw_lm3646_mix_ir_on,
	.flash_off = hw_lm3646_mix_ir_off,
	.flash_match = hw_lm3646_mix_ir_match,
	.flash_get_dt_data = hw_lm3646_mix_ir_get_dt_data,
	.flash_register_attribute = hw_lm3646_mix_ir_register_attribute,
};

struct hw_flash_ctrl_t g_lm3646_mix_ir_ctrl = {
	.flash_i2c_client = &g_lm3646_mix_ir_i2c_client,
	.func_tbl = &g_lm3646_mix_ir_func_tbl,
	.hw_flash_mutex = &flash_mut_lm3646_mix_ir,
	.pdata = (void *)&g_lm3646_mix_ir_pdata,
	.flash_mask_enable = true,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_init(hw_lm3646_mix_ir_module_init);
module_exit(hw_lm3646_mix_ir_module_exit);
MODULE_DESCRIPTION("lm3646 FLASH");
MODULE_LICENSE("GPL v2");
