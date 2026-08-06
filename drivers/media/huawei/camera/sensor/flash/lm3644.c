/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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
//lint -save -e31 -e64 -e84 -e708 -e712 -e713 -e715 -e732 -e734 -e737
//lint -save -e750 -e754 -e528 -e570 -e574 -e578 -e651 -e701
//lint -save -e747 -e753 -e778 -e785 -e838 -e846 -e866 -e514
/* LM3644 Registers define */
#define REG_CHIP_ID                     (0x0c)
#define KTD_CHIP_ID                     (0x02)
#define TI_CHIP_ID                      (0x04)
#define REG_ENABLE                      (0x01)
#define REG_L1_FL                       (0x03)
#define REG_L2_FL                       (0x04)
#define REG_L1_TORCH                    (0x05)
#define REG_L2_TORCH                    (0x06)
#define REG_FAULT_INDICATION_A          (0x0a)
#define REG_FAULT_INDICATION_B          (0x0b)
#define LED2_EN                         (0x1 << 1)
#define LED1_EN                         (0x1)
#define REG_TIMING_CONFIG               (0x08)
#define MODE_STANDBY                    (0x00)
#define FLASH_TIMEOUT_TI                (0x0A)   //600ms-TI
#define FLASH_TIMEOUT_KTD               (0x0F)   // 400ms-KTD
#define MODE_TORCH                      (0x02 << 2)
#define MODE_FLASH                      (0x03 << 2)

#define LM3644_FLASH_DEFAULT_CUR_LEV    (63) // 729mA
#define LM3644_TORCH_DEFAULT_CUR_LEV    (63) // 178.6mA
#define LM3644_FLASH_MAX_CUR            (1500)
#define LM3644_TORCH_MAX_CUR            (360)
#define LM3644_TORCH_MAX_LEV            (128)

#define FLASH_LED_MAX                   (128)
#define TORCH_LED_MAX                   (128)
#define FLASH_LED_LEVEL_INVALID         (0xff)

#define LM3644_VOLTAGE_PROTECT          (0x02)
#define LM3644_LED_OPEN_SHORT           (0x70)
#define LM3644_OVER_TEMP_PROTECT        (0x04)

#define FLASH_CURRENT_TO_LEVEL_TI(x)            (((x)*1000-10900)/11725) // reg value = 11.725/step
#define TORCH_CURRENT_TO_LEVEL_TI(x)            (((x)*1000-1954)/2800) // reg value = 2.8/step
#define FLASH_CURRENT_TO_LEVEL_KTD2687(x)            (((32*(x))/375)-1) // reg value = 1500/128step
#define TORCH_CURRENT_TO_LEVEL_KTD2687(x)            (((256*(x))/375)-1) //reg value = 187.5/128step

/* Internal data struct define */
struct hw_lm3644_private_data_t {
	unsigned char flash_led[FLASH_LED_MAX];
	unsigned char torch_led[TORCH_LED_MAX];
	unsigned int flash_led_num;
	unsigned int torch_led_num;
	unsigned int flash_current;
	unsigned int torch_current;

	/* flash control pin */
	unsigned int strobe;
	/* flash control time */
	unsigned int timeout;

	unsigned int chipid;
};

/* Internal varible define */
static struct hw_lm3644_private_data_t hw_lm3644_pdata;
static struct hw_flash_ctrl_t hw_lm3644_ctrl;
static struct i2c_driver hw_lm3644_i2c_driver;

extern struct dsm_client *client_flash;

DEFINE_HISI_FLASH_MUTEX(lm3644);
#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif
/* Function define */
extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);

static int hw_lm3644_clear_error_and_notify_dmd(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	unsigned char val_a = 0;
	unsigned char val_b = 0;

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	/* clear error flag, resume chip */
	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_A, &val_a) < 0);
	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_B, &val_b) < 0);
	if(val_a != 0 || val_b != 0) {
	    if(!dsm_client_ocuppy(client_flash) && (
                  (val_a & LM3644_VOLTAGE_PROTECT) ||
                  (val_b & LM3644_VOLTAGE_PROTECT))) {
                  dsm_client_record(client_flash, "UVLO FALUT or OVP FAULT\n");
                  dsm_client_notify(client_flash, DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
                  cam_err("[I/DSM] %s Undervoltage lockout or Overvoltage error", __func__);
	    }
		if(!dsm_client_ocuppy(client_flash) && (val_a & LM3644_LED_OPEN_SHORT)) {
                  dsm_client_record(client_flash, "LED short or open circuit\n");
                  dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
                  cam_err("[I/DSM] %s LED short or open circuit error", __func__);
	    }
		if(!dsm_client_ocuppy(client_flash) && (val_a & LM3644_OVER_TEMP_PROTECT)) {
                  dsm_client_record(client_flash, "LED over-Temperature\n");
                  dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
                  cam_err("[I/DSM] %s LED over-Temperature error", __func__);
	    }
	    return -1;
	}

	return 0;
}

static int hw_lm3644_find_match_flash_current(int cur_flash, unsigned int id)
{
    int cur_level = 0;

    cam_info("%s ernter cur_flash %d, driveric is %s\n", __func__, cur_flash,
        (id == KTD_CHIP_ID) ? "KTD2687" : "LM3644_TI");
    if(cur_flash <= 0){
        cam_err("%s current set is error", __func__);
        return -1;
    }

    if(cur_flash >= LM3644_FLASH_MAX_CUR){
        cam_warn("%s current set is %d", __func__, cur_flash);
        return LM3644_FLASH_DEFAULT_CUR_LEV;
    }

    switch (id) {
        case KTD_CHIP_ID:
            cur_level = FLASH_CURRENT_TO_LEVEL_KTD2687(cur_flash);
            break;
        case TI_CHIP_ID:
            cur_level = FLASH_CURRENT_TO_LEVEL_TI(cur_flash);
            break;
        default:
            break;
    }

    return cur_level;
}

static int hw_lm3644_find_match_torch_current(int cur_torch, unsigned int id)
{
    int cur_level = 0;

    cam_info("%s ernter cur_torch %d, driveric is %s\n", __func__, cur_torch,
        (id == KTD_CHIP_ID) ? "KTD2687" : "LM3644_TI");
    if(cur_torch <= 0){
        cam_err("%s current set is error", __func__);
        return -1;
    }

    if(cur_torch > LM3644_TORCH_MAX_CUR){
        cam_warn("%s current set is %d", __func__, cur_torch);
        return LM3644_TORCH_MAX_LEV;
    }

    switch (id) {
        case KTD_CHIP_ID:
            cur_level = TORCH_CURRENT_TO_LEVEL_KTD2687(cur_torch);
            break;
        case TI_CHIP_ID:
            cur_level = TORCH_CURRENT_TO_LEVEL_TI(cur_torch);
            break;
        default:
            break;
    }

    return cur_level;
}

static int hw_lm3644_init(struct hw_flash_ctrl_t *flash_ctrl)
{

	cam_debug("%s ernter.\n", __func__);
	if (NULL == flash_ctrl) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	return 0;
}

static int hw_lm3644_exit(struct hw_flash_ctrl_t *flash_ctrl)
{


	cam_debug("%s ernter.\n", __func__);
	if (NULL == flash_ctrl) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	flash_ctrl->func_tbl->flash_off(flash_ctrl);

	return 0;
}

static int hw_lm3644_flash_mode(struct hw_flash_ctrl_t *flash_ctrl,
    struct hw_flash_cfg_data * cdata)
{
        struct hw_flash_i2c_client *i2c_client;
        struct hw_flash_i2c_fn_t *i2c_func;
        struct hw_lm3644_private_data_t *pdata;
        unsigned char val;
        int current_level = 0;
        int rc = 0;
        unsigned char regval = 0;

        if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) ||
            (NULL == flash_ctrl->flash_i2c_client) || (NULL ==cdata)) {
            cam_err("%s flash_ctrl is NULL.", __func__);
            return -1;
        }
        cam_info("%s data=%d.\n", __func__, cdata->data);

        i2c_client = flash_ctrl->flash_i2c_client;
        i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
        pdata = flash_ctrl->pdata;
        if (FLASH_LED_LEVEL_INVALID == pdata->flash_current)
        {
            current_level = LM3644_FLASH_DEFAULT_CUR_LEV;
        }
        else
        {
            current_level = hw_lm3644_find_match_flash_current(cdata->data, pdata->chipid);
             if(current_level < 0){
                    current_level = LM3644_FLASH_DEFAULT_CUR_LEV;
              }
       }

       rc = hw_lm3644_clear_error_and_notify_dmd(flash_ctrl);
       if(rc < 0)
       {
              cam_err("%s flashlight clear errorl", __func__);
              return -1;
        }

       /* set LED Flash current value */
       val =  (current_level & 0x7f);
       cam_info("%s led flash current val=0x%x, current level=%d, flash control time =%d\n",
            __func__, val, current_level, pdata->timeout);

       /*0 = LED2 flash current is set by i2c register 0x04*/
       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L1_FL, 0) < 0);
       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L2_FL, val) < 0);
       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TIMING_CONFIG, pdata->timeout) < 0);  //set timout

       regval = LED2_EN | MODE_FLASH;
       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, regval) < 0);

       return 0;
}

static int hw_lm3644_torch_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	struct hw_lm3644_private_data_t *pdata;
	unsigned char val;
	int current_level = 0;
	int rc = 0;

	cam_info("%s data=%d.\n", __func__, data);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) ||
        (NULL == flash_ctrl->flash_i2c_client)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
	if (FLASH_LED_LEVEL_INVALID == pdata->torch_current)
	{
		current_level = LM3644_TORCH_DEFAULT_CUR_LEV;
	}
	else
	{
		current_level = hw_lm3644_find_match_torch_current(data, pdata->chipid);
		if(current_level < 0){
		      current_level = LM3644_TORCH_DEFAULT_CUR_LEV;
		}
	}


	rc = hw_lm3644_clear_error_and_notify_dmd(flash_ctrl);
	if(rc < 0)
	{
              cam_err("%s flashlight clear errorl", __func__);
              return -1;
	}

	/* set LED torch current value */
	val = (current_level & 0x7f);
	cam_info("%s the led torch current val=0x%x, current_level=%d.\n",
            __func__, val, current_level);

	/* 0 = LED2 flash current is set by i2c register 0x06 */
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L1_TORCH, 0) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_L2_TORCH, val) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_TORCH|LED2_EN) < 0);

	return 0;
}

static int hw_lm3644_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
	struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
	int rc=-1;

	cam_debug("%s ernter.\n", __func__);
	if ((NULL == flash_ctrl) || (NULL == cdata)) {
		cam_err("%s flash_ctrl or cdata is NULL.", __func__);
		return -1;
	}

	cam_info("%s mode=%d, level=%d.\n", __func__, cdata->mode, cdata->data);
	mutex_lock(flash_ctrl->hw_flash_mutex);
	if ((FLASH_MODE == cdata->mode) || (FLASH_STROBE_MODE == cdata->mode))// strobe is a trigger method of FLASH mode
       {
		rc = hw_lm3644_flash_mode(flash_ctrl, cdata);
	} else {
		rc = hw_lm3644_torch_mode(flash_ctrl, cdata->data);
	}
	flash_ctrl->state.mode = cdata->mode;
	flash_ctrl->state.data = cdata->data;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_lm3644_off(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	unsigned char val;

	cam_info("%s ernter.\n", __func__);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->flash_i2c_client) || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	mutex_lock(flash_ctrl->hw_flash_mutex);
	flash_ctrl->state.mode = STANDBY_MODE;
	flash_ctrl->state.data = 0;
	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	if(i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_A, &val) < 0 ||
        i2c_func->i2c_read(i2c_client, REG_FAULT_INDICATION_B, &val) < 0){
		cam_err("%s %d", __func__, __LINE__);
	}
	if(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_STANDBY) < 0){
		cam_err("%s %d", __func__, __LINE__);
	}
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return 0;
}

static int hw_lm3644_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3644_private_data_t *pdata;
	struct device_node *of_node;
	int rc = -1;

	cam_debug("%s enter.\n", __func__);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return rc;
	}

	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;
	of_node = flash_ctrl->dev->of_node;

	rc = of_property_read_u32(of_node, "huawei,flash-pin",
		&pdata->strobe);
	cam_info("%s hisi,flash-pin %d, rc %d\n", __func__,
		pdata->strobe, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		/* return rc; */
	}

	rc = of_property_read_u32(of_node, "huawei,flash_current",
		&pdata->flash_current);
	cam_info("%s hisi,flash_current %d, rc %d\n", __func__,
		pdata->flash_current, rc);
	if (rc < 0) {
		cam_info("%s failed %d\n", __func__, __LINE__);
		pdata->flash_current = FLASH_LED_LEVEL_INVALID;
		//return rc;
	}

	rc = of_property_read_u32(of_node, "huawei,torch_current",
		&pdata->torch_current);
	cam_info("%s hisi,torch_current %d, rc %d\n", __func__,
		pdata->torch_current, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		pdata->torch_current = FLASH_LED_LEVEL_INVALID;
		//TO FIX
		//return rc;
	}
	return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_lm3644_lightness_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
        int rc = 0;

        snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE, "mode=%d, data=%d.\n",
		hw_lm3644_ctrl.state.mode, hw_lm3644_ctrl.state.mode);
        rc = strlen(buf) + 1;
        return rc;
}

static int hw_lm3644_param_check(char *buf, unsigned long *param,
	int num_of_par)
{
	char *token;
	int base, cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++)
	{
		if (token != NULL)
		{
			if ((token[1] == 'x') || (token[1] == 'X')) {
				base = 16;
			} else {
				base = 10;
			}

			if (strict_strtoul(token, base, &param[cnt]) != 0) {
				return -EINVAL;
			}

			token = strsep(&buf, " ");
		}
		else
		{
			return -EINVAL;
		}
	}
	return 0;
}

static ssize_t hw_lm3644_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[2]={0};
	int rc=0;

	rc = hw_lm3644_param_check((char *)buf, param, 2);
	if (rc < 0) {
		cam_err("%s failed to check param.", __func__);
		return rc;
	}

    int flash_id = (int)param[0];
    cdata.mode = (int)param[1];
    cam_info("%s flash_id=%d, cdata.mode=%d.", __func__, flash_id, cdata.mode);
    if (1 != flash_id) {
    /*bit[0]- rear first flash light
      bit[1]- rear sencond flash light
      bit[2]- front flash light;*/
        cam_err("%s wrong flash_id=%d.", __func__,flash_id);
        return -1;
    }

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_lm3644_off(&hw_lm3644_ctrl);
		if (rc < 0) {
			cam_err("%s lm3644 flash off error.", __func__);
			return rc;
		}
	} else if (cdata.mode == TORCH_MODE) {
        /* hardware test requiring the max torch mode current */
        cdata.data = LM3644_TORCH_MAX_CUR;
        cam_info("%s mode=%d, max_current=%d.", __func__, cdata.mode, cdata.data);

		rc = hw_lm3644_on(&hw_lm3644_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s lm3644 flash on error.", __func__);
			return rc;
		}
    } else {
        cam_err("%s scharger wrong mode=%d.", __func__,cdata.mode);
        return -1;
    }

	return count;
}
#endif

static ssize_t hw_lm3644_flash_mask_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
        int rc=0;

        snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE-1,
            "flash_mask_disabled=%d.\n", hw_lm3644_ctrl.flash_mask_enable);
        rc = strlen(buf)+1;
        return rc;
}

static ssize_t hw_lm3644_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if ('0' == buf[0]) {
		hw_lm3644_ctrl.flash_mask_enable = false;
	} else {
		hw_lm3644_ctrl.flash_mask_enable = true;
	}
	cam_debug("%s flash_mask_enable=%d.", __func__,
			hw_lm3644_ctrl.flash_mask_enable);
	return count;
}

static void hw_lm3644_torch_brightness_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	struct hw_flash_cfg_data cdata;
	int rc;
	unsigned int led_bright = brightness;

	if (STANDBY_MODE == led_bright) {
		rc = hw_lm3644_off(&hw_lm3644_ctrl);
		if (rc < 0) {
			cam_err("%s pmu_led off error.", __func__);
			return;
		}
	} else {
		cdata.mode = TORCH_MODE;
		cdata.data = brightness-1;
		rc = hw_lm3644_on(&hw_lm3644_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s pmu_led on error.", __func__);
			return;
		}
	}
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute hw_lm3644_lightness =
    __ATTR(flash_lightness, 0664, hw_lm3644_lightness_show, hw_lm3644_lightness_store);
#endif

static struct device_attribute hw_lm3644_flash_mask =
    __ATTR(flash_mask, 0664, hw_lm3644_flash_mask_show, hw_lm3644_flash_mask_store);

static int hw_lm3644_register_attribute(struct hw_flash_ctrl_t *flash_ctrl, struct device *dev)
{
	int rc = 0;

	if ((NULL == flash_ctrl) || (NULL == dev)) {
		cam_err("%s flash_ctrl or dev is NULL.", __func__);
		return -1;
	}

	flash_ctrl->cdev_torch.name = "torch";
	flash_ctrl->cdev_torch.max_brightness
		= ((struct hw_lm3644_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
	flash_ctrl->cdev_torch.brightness_set = hw_lm3644_torch_brightness_set;
	rc = led_classdev_register((struct device *)dev, &flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev.", __func__);
		goto err_out;
	}
#ifdef CAMERA_FLASH_FACTORY_TEST
	rc = device_create_file(dev, &hw_lm3644_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat lightness attribute.", __func__);
		goto err_create_lightness_file;
	}
#endif
	rc = device_create_file(dev, &hw_lm3644_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute.", __func__);
		goto err_create_flash_mask_file;
	}
	return 0;
err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
	device_remove_file(dev, &hw_lm3644_lightness);
#endif
err_create_lightness_file:
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
	return rc;
}//lint !e563

static int hw_lm3644_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	struct hw_lm3644_private_data_t *pdata;
	unsigned char id;

	cam_debug("%s ernter.\n", __func__);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) ||
        (NULL == flash_ctrl->flash_i2c_client)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3644_private_data_t *)flash_ctrl->pdata;

	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0);
	cam_info("%s id = 0x%x\n", __func__, id);
	if (id != KTD_CHIP_ID && id != TI_CHIP_ID) {
		cam_err("%s match error, id(0x%x) != 0x%x and != 0x%x",
			__func__, (id & 0x7), KTD_CHIP_ID, TI_CHIP_ID);
		return -1;
	}

    pdata->chipid = id;
    pdata->timeout = (id == KTD_CHIP_ID) ? FLASH_TIMEOUT_KTD : FLASH_TIMEOUT_TI;

#ifdef CAMERA_FLASH_FACTORY_TEST
	register_camerafs_attr(&hw_lm3644_lightness);
#endif
	return 0;
}


static int hw_lm3644_remove(struct i2c_client *client)
{
	cam_debug("%s enter.", __func__);

	hw_lm3644_ctrl.func_tbl->flash_exit(&hw_lm3644_ctrl);

	client->adapter = NULL;
	return 0;
}

static void hw_lm3644_shutdown(struct i2c_client *client)
{
	int rc = -1;
	rc = hw_lm3644_off(&hw_lm3644_ctrl);
	cam_info("%s lm3644 shut down at (%d)",__func__,__LINE__);
	if (rc < 0) {
		cam_err("%s lm3644 flash off error.", __func__);
	}
}

static const struct i2c_device_id hw_lm3644_id[] = {
	{"lm3644", (unsigned long)&hw_lm3644_ctrl},
	{}
};

static const struct of_device_id hw_lm3644_dt_match[] = {
	{.compatible = "huawei,lm3644"},
	{}
};
MODULE_DEVICE_TABLE(of, lm3644_dt_match);

static struct i2c_driver hw_lm3644_i2c_driver = {
	.probe	= hw_flash_i2c_probe,
	.remove = hw_lm3644_remove,
	.shutdown = hw_lm3644_shutdown,
	.id_table	= hw_lm3644_id,
	.driver = {
		.name = "hw_lm3644",
		.of_match_table = hw_lm3644_dt_match,
	},
};

static int __init hw_lm3644_module_init(void)
{
	cam_info("%s erter.\n", __func__);
	return i2c_add_driver(&hw_lm3644_i2c_driver);
}

static void __exit hw_lm3644_module_exit(void)
{
	cam_info("%s enter.", __func__);
	i2c_del_driver(&hw_lm3644_i2c_driver);
	return;
}

static struct hw_flash_i2c_client hw_lm3644_i2c_client;

static struct hw_flash_fn_t hw_lm3644_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_lm3644_init,
	.flash_exit = hw_lm3644_exit,
	.flash_on = hw_lm3644_on,
	.flash_off = hw_lm3644_off,
	.flash_match = hw_lm3644_match,
	.flash_get_dt_data = hw_lm3644_get_dt_data,
	.flash_register_attribute = hw_lm3644_register_attribute,
};

static struct hw_flash_ctrl_t hw_lm3644_ctrl = {
	.flash_i2c_client = &hw_lm3644_i2c_client,
	.func_tbl = &hw_lm3644_func_tbl,
	.hw_flash_mutex = &flash_mut_lm3644,
	.pdata = (void*)&hw_lm3644_pdata,
	.flash_mask_enable = false,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_init(hw_lm3644_module_init);
module_exit(hw_lm3644_module_exit);
MODULE_DESCRIPTION("LM3644 FLASH");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
