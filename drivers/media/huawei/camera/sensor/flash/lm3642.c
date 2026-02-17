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
/* LM3642 Registers define */
#define REG_CHIP_ID			0x00
#define REG_ENABLE			0x0a
#define REG_FLAGS			0x0b
#define REG_FLASH_FEATURES		0x08
#define REG_CURRENT_CONTROL		0x09
#define REG_IVFM			0x01


#define MODE_STANDBY			0x00
#define UVLO_VOLTAGE                   0x0C   //3.2v
#define FLASH_TIMEOUT			0x05   // 600ms
#define FLASH_RAMP_TIME             0x08   //512us
#define MODE_TORCH			0x02
#define MODE_FLASH			0x03
#define STROBE_PIN 0x20  // 0 = disable, 1=enable

#define IVFM_EN                       0x80
#define UVLO_EN                       0x80
#define TX_PIN				0x40

#define LM3642_FLASH_DEFAULT_CUR_LEV          7  //760mA
#define LM3642_TORCH_DEFAULT_CUR_LEV         2    //140mA
#define LM3642_FLASH_MAX_CUR                         1500
#define LM3642_TORCH_MAX_CUR                         375
#define LM3642_TORCH_MAX_RT_CUR                      190
#define LM3642_TORCH_MAX_LEV                         7

#define FLASH_LED_MAX			16
#define TORCH_LED_MAX			8
#define FLASH_LED_LEVEL_INVALID  0xff

#define LM3642_UNDER_VOLTAGE_LOCKOUT              0x10
#define LM3642_OVER_VOLTAGE_PROTECT               0x08
#define LM3642_LED_VOUT_SHORT                     0x04
#define LM3642_OVER_TEMP_PROTECT                  0x02

/* Internal data struct define */
struct hw_lm3642_private_data_t {
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

static int flash_arry[16] = {94, 188, 281, 375, 469, 563, 656, 750, 844, 938, 1031, 1125, 1219, 1313, 1406, 1500};
static int torch_arry[8] = {48, 94, 141, 188, 234, 281, 328, 375};

/* Internal varible define */
static struct hw_lm3642_private_data_t hw_lm3642_pdata;
static struct hw_flash_ctrl_t hw_lm3642_ctrl;
static struct i2c_driver hw_lm3642_i2c_driver;

extern struct dsm_client *client_flash;

DEFINE_HISI_FLASH_MUTEX(lm3642);
#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif
/* Function define */
extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);

static int hw_lm3642_clear_error_and_notify_dmd(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	unsigned char val = 0;
	int rc = 0;

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

	/* clear error flag,resume chip */
	rc = i2c_func->i2c_read(i2c_client, REG_FLAGS, &val);
	if(rc < 0) {
	    if(!dsm_client_ocuppy(client_flash)) {
                  dsm_client_record(client_flash, "flash i2c transfer fail\n");
                  dsm_client_notify(client_flash, DSM_FLASH_I2C_ERROR_NO);
                  cam_err("[I/DSM] %s flashlight i2c fail", __func__);
	    }
	    return -1;
	}

	if (val & LM3642_OVER_TEMP_PROTECT) {
            if(!dsm_client_ocuppy(client_flash)) {
                dsm_client_record(client_flash, "flash temperature is too hot! FlagReg[0x%x]\n", val);
                dsm_client_notify(client_flash, DSM_FLASH_HOT_DIE_ERROR_NO);
                cam_warn("[I/DSM] %s flash temperature is too hot! FlagReg[0x%x]", __func__, val);
            }
	}

	if (val & (LM3642_OVER_VOLTAGE_PROTECT | LM3642_LED_VOUT_SHORT)) {
            if(!dsm_client_ocuppy(client_flash)) {
                dsm_client_record(client_flash, "flash OVP, LED or VOUT short! FlagReg[0x%x]\n", val);
                dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
                cam_warn("[I/DSM] %s flash OVP, LED or VOUT short! FlagReg[0x%x]", __func__, val);
            }
	}

	if (val & LM3642_UNDER_VOLTAGE_LOCKOUT ) {
            if(!dsm_client_ocuppy(client_flash)) {
                dsm_client_record(client_flash, "flash UVLO! FlagReg[0x%x]\n", val);
                dsm_client_notify(client_flash, DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
                cam_warn("[I/DSM] %s flash UVLO! FlagReg[0x%x]", __func__, val);
            }
	}

	return 0;
}

static int hw_lm3642_find_match_flash_current(int cur_flash)
{
    int cur_level = 0;
    int i = 0;

    cam_info("%s ernter cur_flash %d.\n", __func__, cur_flash);
    if(cur_flash <= 0){
        cam_err("%s current set is error", __func__);
        return -1;
    }

    if(cur_flash >= LM3642_FLASH_MAX_CUR){
        cam_warn("%s current set is %d", __func__, cur_flash);
        return LM3642_FLASH_DEFAULT_CUR_LEV;
    }

    for(i=0; i < 16; i ++){
       if(cur_flash <= flash_arry[i]){
            cam_info("%s  i %d.\n", __func__, i);
            break;
       }
    }

    if( i==0 ){
          cur_level = i;
    }
    else if(i == 16){
        if((cur_flash - flash_arry[i-2]) < (flash_arry[i - 1] -cur_flash))
              cur_level = i -2;
        else
              cur_level = i -1;
    }
    else{
        if((cur_flash - flash_arry[i-1]) < (flash_arry[i] -cur_flash))
              cur_level = i -1;
        else
              cur_level = i;
    }

    return cur_level;
}

static int hw_lm3642_find_match_torch_current(int cur_torch)
{
    int cur_level = 0;
    int i = 0;

    cam_info("%s ernter cur_torch %d.\n", __func__, cur_torch);
    if(cur_torch <= 0){
        cam_err("%s current set is error", __func__);
        return -1;
    }

    if(cur_torch > LM3642_TORCH_MAX_CUR){
        cam_warn("%s current set is %d", __func__, cur_torch);
        return LM3642_TORCH_MAX_LEV;
    }

    for(i=0; i < 8; i ++){
       if(cur_torch <= torch_arry[i]){
            cam_info("%s  i %d.\n", __func__, i);
            break;
       }
    }

    if( i==0 ){
          cur_level = i;
    }
    else if(i == 8){
        if((cur_torch - torch_arry[i-2]) < (torch_arry[i - 1] -cur_torch))
              cur_level = i -2;
        else
              cur_level = i - 1;
   }
    else{
        if((cur_torch - torch_arry[i-1]) < (torch_arry[i] -cur_torch))
              cur_level = i -1;
        else
              cur_level = i;
    }

    return cur_level;
}

static int hw_lm3642_init(struct hw_flash_ctrl_t *flash_ctrl)
{

	cam_debug("%s ernter.\n", __func__);
	if (NULL == flash_ctrl) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}
	return 0;
}

static int hw_lm3642_exit(struct hw_flash_ctrl_t *flash_ctrl)
{

	cam_debug("%s ernter.\n", __func__);
	if (NULL == flash_ctrl) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	flash_ctrl->func_tbl->flash_off(flash_ctrl);
	return 0;
}

static int hw_lm3642_flash_mode(struct hw_flash_ctrl_t *flash_ctrl,
    struct hw_flash_cfg_data * cdata)
{
        struct hw_flash_i2c_client *i2c_client;
        struct hw_flash_i2c_fn_t *i2c_func;
        struct hw_lm3642_private_data_t *pdata;
        unsigned char val;
        int current_level = 0;
        int rc = 0;
        unsigned char regval = 0;

        if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client) || (NULL ==cdata)) {
            cam_err("%s flash_ctrl is NULL.", __func__);
            return -1;
        }
        cam_info("%s data=%d.\n", __func__, cdata->data);

        i2c_client = flash_ctrl->flash_i2c_client;
        i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
        pdata = flash_ctrl->pdata;
        if (FLASH_LED_LEVEL_INVALID == pdata->flash_current)
        {
            current_level = LM3642_FLASH_DEFAULT_CUR_LEV;
        }
        else
        {
            current_level = hw_lm3642_find_match_flash_current(cdata->data);
             if(current_level < 0){
                    current_level = LM3642_FLASH_DEFAULT_CUR_LEV;
              }
       }

       rc = hw_lm3642_clear_error_and_notify_dmd(flash_ctrl);
       if(rc < 0)
       {
              cam_err("%s flashlight clear errorl", __func__);
              return -1;
        }

       loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CURRENT_CONTROL, &val) < 0);

       /* set LED Flash current value */
       val = (val & 0xf0) | (current_level & 0x0f);
       cam_info("%s led flash current val=0x%x, current level=%d.\n", __func__, val, current_level);

       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_CURRENT_CONTROL, val) < 0);
       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_FEATURES, FLASH_RAMP_TIME|FLASH_TIMEOUT) < 0);  //set timout and ramp time
       if (flash_ctrl->flash_mask_enable)
       {
           regval = MODE_FLASH|TX_PIN;
           if(FLASH_STROBE_MODE == cdata->mode)
           {
               regval = MODE_FLASH|TX_PIN|STROBE_PIN;
           }
       }
       else
       {
            regval = MODE_FLASH|IVFM_EN;
            if(FLASH_STROBE_MODE == cdata->mode)
            {
                regval = MODE_FLASH|IVFM_EN|STROBE_PIN;
            }
       }

       loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, regval) < 0);

       return 0;
}

static int hw_lm3642_torch_mode(struct hw_flash_ctrl_t *flash_ctrl,
	int data)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	struct hw_lm3642_private_data_t *pdata;
	unsigned char val;
	int current_level = 0;
	int rc = 0;

	cam_info("%s data=%d.\n", __func__, data);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3642_private_data_t *)flash_ctrl->pdata;
	if (FLASH_LED_LEVEL_INVALID == pdata->torch_current)
	{
		current_level = LM3642_TORCH_DEFAULT_CUR_LEV;
	}
	else
	{
		current_level = hw_lm3642_find_match_torch_current(data);
		if(current_level < 0){
		      current_level = LM3642_TORCH_DEFAULT_CUR_LEV;
		}
	}

	rc = hw_lm3642_clear_error_and_notify_dmd(flash_ctrl);
	if(rc < 0)
	{
              cam_err("%s flashlight clear errorl", __func__);
              return -1;
	}

	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CURRENT_CONTROL, &val) < 0);

	/* set LED Flash current value */
	val = (val & 0x0f) | (current_level << 4);
	cam_info("%s the led torch current val=0x%x, current_level=%d.\n", __func__, val, current_level);

	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_CURRENT_CONTROL, val) < 0);
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_TORCH|IVFM_EN) < 0);

	return 0;
}

static int hw_lm3642_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
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
		rc = hw_lm3642_flash_mode(flash_ctrl, cdata);
	} else {
		rc = hw_lm3642_torch_mode(flash_ctrl, cdata->data);
	}
	flash_ctrl->state.mode = cdata->mode;
	flash_ctrl->state.data = cdata->data;
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return rc;
}

static int hw_lm3642_off(struct hw_flash_ctrl_t *flash_ctrl)
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

	if(i2c_func->i2c_read(i2c_client, REG_FLAGS, &val) < 0){
		cam_err("%s %d", __func__, __LINE__);
	}
	if(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_STANDBY) < 0){
		cam_err("%s %d", __func__, __LINE__);
	}
	mutex_unlock(flash_ctrl->hw_flash_mutex);

	return 0;
}

static int hw_lm3642_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_lm3642_private_data_t *pdata;
	struct device_node *of_node;
	int rc = -1;

	cam_debug("%s enter.\n", __func__);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return rc;
	}

	pdata = (struct hw_lm3642_private_data_t *)flash_ctrl->pdata;
	of_node = flash_ctrl->dev->of_node;

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
	rc = of_property_read_u32(of_node, "huawei,flash-chipid",
		&pdata->chipid);
	cam_info("%s hisi,chipid 0x%x, rc %d\n", __func__,
		pdata->chipid, rc);
	if (rc < 0) {
		cam_err("%s failed %d\n", __func__, __LINE__);
		return rc;
	}
	return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_lm3642_lightness_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
        int rc=0;

        snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE, "mode=%d, data=%d.\n",
		hw_lm3642_ctrl.state.mode, hw_lm3642_ctrl.state.mode);
        rc = strlen(buf)+1;
        return rc;
}

static int hw_lm3642_param_check(char *buf, unsigned long *param,
	int num_of_par)
{
	char *token;
	int base, cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++)
	{
		if (token != NULL)
		{
			if ((strlen(token) > 1)&& ((token[1] == 'x') || (token[1] == 'X'))) {
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

static ssize_t hw_lm3642_lightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hw_flash_cfg_data cdata = {0};
	unsigned long param[2]={0};
	int rc=0;

	rc = hw_lm3642_param_check((char *)buf, param, 2);
	if (rc < 0) {
		cam_err("%s failed to check param.", __func__);
		return rc;
	}

    int flash_id = (int)param[0];
    cdata.mode = (int)param[1];
    cam_info("%s flash_id=%d,cdata.mode=%d.", __func__, flash_id, cdata.mode);
    if(1 != flash_id){//bit[0]- rear first flash light.bit[1]- rear sencond flash light.bit[2]- front flash light; dallas product using only rear first flash light
        cam_err("%s wrong flash_id=%d.", __func__,flash_id);
        return -1;
    }

	if (cdata.mode == STANDBY_MODE) {
		rc = hw_lm3642_off(&hw_lm3642_ctrl);
		if (rc < 0) {
			cam_err("%s lm3642 flash off error.", __func__);
			return rc;
		}
	} else if(cdata.mode == TORCH_MODE){
        cdata.data = LM3642_TORCH_MAX_RT_CUR;//hardware test requiring the max torch mode current
        cam_info("%s mode=%d, max_current=%d.", __func__, cdata.mode, cdata.data);

		rc = hw_lm3642_on(&hw_lm3642_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s lm3642 flash on error.", __func__);
			return rc;
		}
    } else {
        cam_err("%s scharger wrong mode=%d.", __func__,cdata.mode);
        return -1;
    }

	return count;
}
#endif

static ssize_t hw_lm3642_flash_mask_show(struct device *dev,
	struct device_attribute *attr,char *buf)
{
        int rc=0;

        snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE-1, "flash_mask_disabled=%d.\n",
		hw_lm3642_ctrl.flash_mask_enable);
        rc = strlen(buf)+1;
        return rc;
}

static ssize_t hw_lm3642_flash_mask_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if ('0' == buf[0]) {
		hw_lm3642_ctrl.flash_mask_enable = false;
	} else {
		hw_lm3642_ctrl.flash_mask_enable = true;
	}
	cam_debug("%s flash_mask_enable=%d.", __func__,
			hw_lm3642_ctrl.flash_mask_enable);
	return count;
}

static void hw_lm3642_torch_brightness_set(struct led_classdev *cdev,
	enum led_brightness brightness)
{
	struct hw_flash_cfg_data cdata;
	int rc;
	unsigned int led_bright = brightness;

	if (STANDBY_MODE == led_bright) {
		rc = hw_lm3642_off(&hw_lm3642_ctrl);
		if (rc < 0) {
			cam_err("%s pmu_led off error.", __func__);
			return;
		}
	} else {
		cdata.mode = TORCH_MODE;
		cdata.data = brightness-1;
		rc = hw_lm3642_on(&hw_lm3642_ctrl, &cdata);
		if (rc < 0) {
			cam_err("%s pmu_led on error.", __func__);
			return;
		}
	}
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute hw_lm3642_lightness =
    __ATTR(flash_lightness, 0664, hw_lm3642_lightness_show, hw_lm3642_lightness_store);
#endif

static struct device_attribute hw_lm3642_flash_mask =
    __ATTR(flash_mask, 0664, hw_lm3642_flash_mask_show, hw_lm3642_flash_mask_store);

static int hw_lm3642_register_attribute(struct hw_flash_ctrl_t *flash_ctrl, struct device *dev)
{
	int rc = 0;

	if ((NULL == flash_ctrl) || (NULL == dev)) {
		cam_err("%s flash_ctrl or dev is NULL.", __func__);
		return -1;
	}

	flash_ctrl->cdev_torch.name = "torch";
	flash_ctrl->cdev_torch.max_brightness
		= ((struct hw_lm3642_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
	flash_ctrl->cdev_torch.brightness_set = hw_lm3642_torch_brightness_set;
	rc = led_classdev_register((struct device *)dev, &flash_ctrl->cdev_torch);
	if (rc < 0) {
		cam_err("%s failed to register torch classdev.", __func__);
		goto err_out;
	}
#ifdef CAMERA_FLASH_FACTORY_TEST
	rc = device_create_file(dev, &hw_lm3642_lightness);
	if (rc < 0) {
		cam_err("%s failed to creat lightness attribute.", __func__);
		goto err_create_lightness_file;
	}
#endif
	rc = device_create_file(dev, &hw_lm3642_flash_mask);
	if (rc < 0) {
		cam_err("%s failed to creat flash_mask attribute.", __func__);
		goto err_create_flash_mask_file;
	}
	return 0;
err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
	device_remove_file(dev, &hw_lm3642_lightness);
#endif
err_create_lightness_file:
	led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
	return rc;
}//lint !e563

static int hw_lm3642_match(struct hw_flash_ctrl_t *flash_ctrl)
{
	struct hw_flash_i2c_client *i2c_client;
	struct hw_flash_i2c_fn_t *i2c_func;
	struct hw_lm3642_private_data_t *pdata;
	unsigned char id;

	cam_debug("%s ernter.\n", __func__);
	if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)) {
		cam_err("%s flash_ctrl is NULL.", __func__);
		return -1;
	}

	i2c_client = flash_ctrl->flash_i2c_client;
	i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
	pdata = (struct hw_lm3642_private_data_t *)flash_ctrl->pdata;

	loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0);
	cam_info("%s id=0x%x.\n", __func__, id);
	if (id != pdata->chipid) {
		cam_err("%s match error, id(0x%x) != 0x%x.",
			__func__, (id&0x7), pdata->chipid);
		return -1;
	}
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, IVFM_EN) < 0);  //enable IVFM
	loge_if_ret(i2c_func->i2c_write(i2c_client, REG_IVFM, UVLO_EN|UVLO_VOLTAGE) < 0);    //enable UVLO, set voltage 3.2v
#ifdef CAMERA_FLASH_FACTORY_TEST
	register_camerafs_attr(&hw_lm3642_lightness);
#endif
	return 0;
}


static int hw_lm3642_remove(struct i2c_client *client)
{
	cam_debug("%s enter.", __func__);

	hw_lm3642_ctrl.func_tbl->flash_exit(&hw_lm3642_ctrl);

	client->adapter = NULL;
	return 0;
}

static void hw_lm3642_shutdown(struct i2c_client *client)
{
	int rc = -1;
	rc = hw_lm3642_off(&hw_lm3642_ctrl);
	cam_info("%s lm3642 shut down at (%d)",__func__,__LINE__);
	if (rc < 0) {
		cam_err("%s lm3642 flash off error.", __func__);
	}
}

static const struct i2c_device_id hw_lm3642_id[] = {
	{"lm3642", (unsigned long)&hw_lm3642_ctrl},
	{}
};

static const struct of_device_id hw_lm3642_dt_match[] = {
	{.compatible = "huawei,lm3642"},
	{}
};
MODULE_DEVICE_TABLE(of, lm3642_dt_match);

static struct i2c_driver hw_lm3642_i2c_driver = {
	.probe	= hw_flash_i2c_probe,
	.remove = hw_lm3642_remove,
	.shutdown = hw_lm3642_shutdown,
	.id_table	= hw_lm3642_id,
	.driver = {
		.name = "hw_lm3642",
		.of_match_table = hw_lm3642_dt_match,
	},
};

static int __init hw_lm3642_module_init(void)
{
	cam_info("%s erter.\n", __func__);
	return i2c_add_driver(&hw_lm3642_i2c_driver);
}

static void __exit hw_lm3642_module_exit(void)
{
	cam_info("%s enter.", __func__);
	i2c_del_driver(&hw_lm3642_i2c_driver);
	return;
}

static struct hw_flash_i2c_client hw_lm3642_i2c_client;

static struct hw_flash_fn_t hw_lm3642_func_tbl = {
	.flash_config = hw_flash_config,
	.flash_init = hw_lm3642_init,
	.flash_exit = hw_lm3642_exit,
	.flash_on = hw_lm3642_on,
	.flash_off = hw_lm3642_off,
	.flash_match = hw_lm3642_match,
	.flash_get_dt_data = hw_lm3642_get_dt_data,
	.flash_register_attribute = hw_lm3642_register_attribute,
};

static struct hw_flash_ctrl_t hw_lm3642_ctrl = {
	.flash_i2c_client = &hw_lm3642_i2c_client,
	.func_tbl = &hw_lm3642_func_tbl,
	.hw_flash_mutex = &flash_mut_lm3642,
	.pdata = (void*)&hw_lm3642_pdata,
	.flash_mask_enable = false,
	.state = {
		.mode = STANDBY_MODE,
	},
};

module_init(hw_lm3642_module_init);
module_exit(hw_lm3642_module_exit);
MODULE_DESCRIPTION("LM3642 FLASH");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
