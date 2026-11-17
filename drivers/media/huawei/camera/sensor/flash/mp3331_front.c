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
//lint -save -e31 -e64 -e84 -e708 -e712 -e713 -e715 -e732 -e734 -e737 -e651
//lint -save -e747 -e753 -e778 -e785 -e838 -e846 -e866 -e514 -e528 -e570 -e574 -e578
/* MP3331_FRONT Registers define */
#define REG_CHIP_ID                             0x00
#define REG_MODE_SET                            0x01
#define REG_PEAK_CUR_SET                        0x02
#define REG_FLASH_TIMER_SET                     0x03
#define REG_LOW_VLO_SET                         0x04
#define REG_INDICATOR_SET                       0x05
#define REG_LED_FLASH_CUR_SET                   0x06
#define REG_TXMASK_CUR_SET                      0x07
#define REG_LED_TORCH_CUR_SET                   0x0A
#define REG_FLASH_FAULT_H                       0x0B
#define REG_FLASH_FAULT_L                       0x0C

#define STB_LV                                  0x80     //Flash mode trigger mode with strobe pin signal
#define LED1_EN                                 0x10    //LED1 current source enable bit
#define FLASH_MODE_CUR                          0x06    //Device mode setting bits flash mode
#define TORCH_MODE_CUR                          0x04    //Device mode setting bits torch mode
#define INDICATOR_MODE_CUR                      0x02    //Device mode setting bits indicator mode
#define FS_SD                                   0x03    // Disable switching frequency stretching down from 1Mhz if VIN is close to VOUT
#define FL_TIM                                  0xB0    // set Flash timer , 600ms
#define VBL_RUN                                 0x4F    // Low battery voltage setting, set 3.2v
#define VBL_SD                                  0x1F   //Device disable when VIN is less than threshold set by VBL_RUN before flash
#define LED_OTAD                                0x80   //Adaptive thermal flash current control bit
#define TUP_I                                   0x70   // current ramp up time 64us/step
#define VTH_PAS                                 0x04   //VTS_PAS:450mV
#define INDI_PWM                                0x03   //PWM duty: 11=5/16
#define I_FL                                    0x18   //flash current:760.8mA
#define I_TX                                    0x08   // I_TX:253.6mA
#define I_TOR                                   0x05   // torch current:158.5mA
#define BASE                                    10
#define MAX_MATCH_TIME                          3

#define MP3331_FRONT_FLASH_DEFAULT_CUR_LEV      24  //760mA
#define MP3331_FRONT_TORCH_DEFAULT_CUR_LEV      9    //89mA
#define MP3331_FRONT_FLASH_MAX_CUR_LEV          47  //1490mA
#define MP3331_FRONT_TORCH_MAX_CUR_LEV          16   //158.5mA
#define MP3331_FRONT_CUR_STEP_LEV               317  //31.7mA * 10
#define MP3331_FRONT_TORCH_RT_CUR               48  //mA

#define FLASH_CHIP_ID_MASK                      0xF8
#define FLASH_LED_LEVEL_INVALID                 0xff

#define MP3331_FRONT_OVER_VOLTAGE_PROTECT       0x40
#define MP3331_FRONT_VOUT_SHORT                 0x20
#define MP3331_FRONT_LED_SHORT                  0x10
#define MP3331_FRONT_OVER_TEMP_PROTECT          0x08
#define MP3331_FRONT_LED_OPEN                   0x01

#define CONVERT2TORCH(x)                        ((x)*16/5)

/* Internal data struct define */
struct hw_mp3331_front_private_data_t {
    unsigned int torch_led_num;
    unsigned int flash_current;
    unsigned int torch_current;
    unsigned int chipid;
};

/* Internal varible define */
static struct hw_mp3331_front_private_data_t hw_mp3331_front_pdata;
static struct hw_flash_ctrl_t hw_mp3331_front_ctrl;
static struct i2c_driver hw_mp3331_front_i2c_driver;

extern struct dsm_client *client_flash;

DEFINE_HISI_FLASH_MUTEX(mp3331_front);

#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif

/****************************************************************************
* FunctionName: msm_flash_clear_err_and_unlock;
* Description : clear the error and unlock the IC ;
* NOTE: this funtion must be called before register is read and write
***************************************************************************/
static int hw_mp3331_front_clear_err_and_unlock(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    unsigned char fault_h = 0;
    unsigned char fault_l = 0;
    int rc = 0;

    cam_debug("%s enter.\n", __func__);

    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->flash_i2c_client)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

    loge_if_ret(i2c_func->i2c_read(i2c_client, REG_FLASH_FAULT_H, &fault_h) < 0);
    rc = i2c_func->i2c_read(i2c_client, REG_FLASH_FAULT_L, &fault_l);
    if(rc < 0) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash i2c transfer fail\n");
            dsm_client_notify(client_flash, DSM_FLASH_I2C_ERROR_NO);
            cam_err("[I/DSM] %s flashlight i2c fail", __func__);
        }
        return -EINVAL;
    }

    if (fault_h & MP3331_FRONT_OVER_TEMP_PROTECT) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash temperature is too hot! FlagReg_H[0x%x]\n", fault_h);
            dsm_client_notify(client_flash, DSM_FLASH_HOT_DIE_ERROR_NO);
            cam_warn("[I/DSM] %s flash temperature is too hot! FlagReg_H[0x%x]", __func__, fault_h);
        }
    }

    if (fault_h & (MP3331_FRONT_OVER_VOLTAGE_PROTECT | MP3331_FRONT_VOUT_SHORT | MP3331_FRONT_LED_SHORT)) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash OVP, VOUT or LED short! FlagReg_H[0x%x]\n", fault_h);
            dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
            cam_warn("[I/DSM] %s flash OVP, VOUT or LED short! FlagReg_H[0x%x]", __func__, (unsigned int)fault_h);
        }
    }

    if (fault_l & MP3331_FRONT_LED_OPEN) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash LED Open! FlagReg_L[0x%x]\n", fault_l);
            dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
            cam_warn("[I/DSM] %s flash LED Opent! FlagReg_L[0x%x]", __func__, (unsigned int)fault_l);
        }
    }

    cam_info("%s fault_h=0x%x, fault_l=0x%x \n", __func__, fault_h, fault_l);

    return 0;
}

static int hw_mp3331_front_find_match_current(int cur_flash)
{
    int integer_flash = 0;
    int remainder_flash = 0;

    if(cur_flash <= 0){
        cam_err("%s current set is error", __func__);
        return -EINVAL;
    }

    integer_flash = (cur_flash * BASE) / MP3331_FRONT_CUR_STEP_LEV;
    remainder_flash = (cur_flash * BASE) % MP3331_FRONT_CUR_STEP_LEV;

    cam_info("%s current=%d integer=%d remainder=%d", __func__, cur_flash, integer_flash, remainder_flash);
    if(integer_flash == 0){
         integer_flash = 1; //level =1
    }
    else{
       if(remainder_flash >= 158)  //158 = 317/2, go to next level
           integer_flash = integer_flash + 1;
    }

    return integer_flash;
}

static int hw_mp3331_front_init(struct hw_flash_ctrl_t *flash_ctrl)
{
    //The chip init reg move to match id function for the hw_flash flow
    cam_debug("%s enter.\n", __func__);
    if (NULL == flash_ctrl) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    return 0;
}

static int hw_mp3331_front_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
    cam_debug("%s enter.\n", __func__);
    if (NULL == flash_ctrl || (NULL == flash_ctrl->func_tbl) || (NULL == flash_ctrl->func_tbl->flash_off)) {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }

    flash_ctrl->func_tbl->flash_off(flash_ctrl);

    return 0;
}

static int hw_mp3331_front_flash_mode(struct hw_flash_ctrl_t *flash_ctrl, int data)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_mp3331_front_private_data_t *pdata;
    int current_level = 0;
    int rc = 0;

    cam_info("%s data=%d.\n", __func__, data);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = flash_ctrl->pdata;
    if (FLASH_LED_LEVEL_INVALID == pdata->flash_current)
    {
        current_level = MP3331_FRONT_FLASH_DEFAULT_CUR_LEV;
    }
    else
    {
        if( data > MP3331_FRONT_FLASH_MAX_CUR_LEV * MP3331_FRONT_CUR_STEP_LEV / BASE){
            current_level = MP3331_FRONT_FLASH_DEFAULT_CUR_LEV;
        }
        else{
            current_level = hw_mp3331_front_find_match_current(data);
            if(current_level < 0){
                current_level = MP3331_FRONT_FLASH_DEFAULT_CUR_LEV;
            }
        }
    }
    /* clear error flag,resume chip */
    rc = hw_mp3331_front_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
        cam_err("%s clear err and unlock error.", __func__);
        return rc;
    }

    /* set LED Flash current value */
    cam_info("%s led flash current  current level=%d.\n", __func__, current_level);

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|FLASH_MODE_CUR) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_FLASH_CUR_SET, current_level) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_TIMER_SET, FL_TIM) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|FLASH_MODE_CUR|LED1_EN) < 0);

    return rc;
}

static int hw_mp3331_front_torch_mode(struct hw_flash_ctrl_t *flash_ctrl, int data)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_mp3331_front_private_data_t *pdata;
    int current_level = 0;
    int rc = 0;

    cam_info("%s data=%d.\n", __func__, data);
    if ((NULL == flash_ctrl) || (NULL ==flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = (struct hw_mp3331_front_private_data_t *)flash_ctrl->pdata;
    if (FLASH_LED_LEVEL_INVALID == pdata->torch_current)
    {
        current_level = MP3331_FRONT_TORCH_DEFAULT_CUR_LEV;
    }
    else
    {
        /*indicator mode current step is 31.7mA*5/16,
          flash mode current step is 31.7mA,
          use CONVERT2TORCH(x) to normalize current step*/
        data = CONVERT2TORCH(data);
        if( data > MP3331_FRONT_TORCH_MAX_CUR_LEV * MP3331_FRONT_CUR_STEP_LEV / BASE){
            current_level = MP3331_FRONT_TORCH_MAX_CUR_LEV;
        }
        else{
            current_level = hw_mp3331_front_find_match_current(data);
            if(current_level < 0){
                current_level = MP3331_FRONT_TORCH_DEFAULT_CUR_LEV;
            }
        }
    }
    /* clear error flag,resume chip */
    rc = hw_mp3331_front_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
        cam_err("%s clear err and unlock error.", __func__);
        return rc;
    }

    /* set LED Flash current value */
    cam_info("%s the led torch current current_level=%d.\n", __func__, current_level);

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|INDICATOR_MODE_CUR) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_TORCH_CUR_SET, current_level) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|INDICATOR_MODE_CUR|LED1_EN) < 0);

    return rc;
}

static int hw_mp3331_front_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
    struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
    int rc = -1;

    if ((NULL == flash_ctrl) || (NULL == cdata)) {
        cam_err("%s flash_ctrl or cdata is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("%s mode=%d, level=%d.\n", __func__, cdata->mode, cdata->data);
    mutex_lock(flash_ctrl->hw_flash_mutex);
    if (FLASH_MODE == cdata->mode) {
        rc = hw_mp3331_front_flash_mode(flash_ctrl, cdata->data);
    } else {
        rc = hw_mp3331_front_torch_mode(flash_ctrl, cdata->data);
    }
    flash_ctrl->state.mode = cdata->mode;
    flash_ctrl->state.data = cdata->data;
    mutex_unlock(flash_ctrl->hw_flash_mutex);

    return rc;
}

static int hw_mp3331_front_off(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    int rc = 0;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->flash_i2c_client)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    mutex_lock(flash_ctrl->hw_flash_mutex);
    flash_ctrl->state.mode = STANDBY_MODE;
    flash_ctrl->state.data = 0;
    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

    /* clear error flag,resume chip */
    rc = hw_mp3331_front_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
        cam_err("%s clear err and unlock error.", __func__);
        mutex_unlock(flash_ctrl->hw_flash_mutex);
        return rc;
    }

    if(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV) < 0){
        cam_err("%s %d", __func__, __LINE__);
    }

    mutex_unlock(flash_ctrl->hw_flash_mutex);

    return 0;
}

static int hw_mp3331_front_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_mp3331_front_private_data_t *pdata;
    struct device_node *of_node;
    int rc = -1;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->dev)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return rc;
    }

    pdata = (struct hw_mp3331_front_private_data_t *)flash_ctrl->pdata;
    of_node = flash_ctrl->dev->of_node;

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
    return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_mp3331_front_lightness_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }
    int rc=0;

    snprintf(buf, MAX_ATTRIBUTE_BUFFER_SIZE, "mode=%d, data=%d.\n",
    hw_mp3331_front_ctrl.state.mode, hw_mp3331_front_ctrl.state.data);
    rc = strlen(buf)+1;
    return rc;
}

static int hw_mp3331_front_param_check(char *buf, unsigned long *param,
    int num_of_par)
{
    if ((NULL == buf) || (NULL == param))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }
    char *token;
    int base, cnt;

    token = strsep(&buf, " ");

    for (cnt = 0; cnt < num_of_par; cnt++)
    {
        if (token != NULL)
        {
            if ((token[1] == 'x') || (token[1] == 'X')) {
                base = 16;//Hex
            } else {
                base = 10;//Decimal
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

static ssize_t hw_mp3331_front_lightness_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }
    struct hw_flash_cfg_data cdata = {0};
    unsigned long param[2]={0};
    int rc=0;

    cam_info("%s enter,buf=%s.", __func__,buf);
    rc = hw_mp3331_front_param_check((char *)buf, param, 2);
    if (rc < 0) {
        cam_err("%s failed to check param.", __func__);
        return rc;
    }

    int flash_id = (int)param[0];
    cdata.mode = (int)param[1];
    cam_info("%s flash_id=%d,cdata.mode=%d.", __func__, flash_id, cdata.mode);

    if(4 != flash_id){//bit[0]- rear first flash light.bit[1]- rear sencond flash light.bit[2]- front flash light;
        cam_err("%s wrong flash_id=%d.", __func__,flash_id);
        return -EINVAL;
    }

    if (cdata.mode == STANDBY_MODE) {
        rc = hw_mp3331_front_off(&hw_mp3331_front_ctrl);
        if (rc < 0) {
            cam_err("%s mp3331_front flash off error.", __func__);
            return rc;
        }
    } else if(cdata.mode == TORCH_MODE){
        cdata.data = MP3331_FRONT_TORCH_RT_CUR;
        cam_info("%s mode=%d, current=%d.", __func__, cdata.mode, cdata.data);

        rc = hw_mp3331_front_on(&hw_mp3331_front_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s mp3331_front flash on error.", __func__);
            return rc;
        }
    } else {
        cam_err("%s wrong mode=%d.", __func__,cdata.mode);
        return -EINVAL;
    }

    return count;
}
#endif

static void hw_mp3331_front_torch_brightness_set(struct led_classdev *cdev,
    enum led_brightness brightness)
{
    struct hw_flash_cfg_data cdata;
    int rc;
    unsigned int led_bright = brightness;

    if (NULL == cdev)
    {
        cam_err("%s invalid param.", __func__);
        return;
    }
    if (STANDBY_MODE == led_bright) {
        rc = hw_mp3331_front_off(&hw_mp3331_front_ctrl);
        if (rc < 0) {
            cam_err("%s pmu_led off error.", __func__);
            return;
        }
    } else {
        cdata.mode = TORCH_MODE;
        cdata.data = MP3331_FRONT_TORCH_RT_CUR;

        cam_info("%s current=%d.", __func__, cdata.data);
        rc = hw_mp3331_front_on(&hw_mp3331_front_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s pmu_led on error.", __func__);
            return;
        }
    }
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute hw_mp3331_front_lightness =
    __ATTR(flash_lightness_f, 0660, hw_mp3331_front_lightness_show, hw_mp3331_front_lightness_store);
#endif

static int hw_mp3331_front_register_attribute(struct hw_flash_ctrl_t *flash_ctrl, struct device *dev)
{
    int rc = 0;

    if ((NULL == flash_ctrl) || (NULL == dev) || (NULL == flash_ctrl->pdata)) {
        cam_err("%s flash_ctrl or dev is NULL.", __func__);
        return -EINVAL;
    }

    flash_ctrl->cdev_torch.name = "torch_front";
    flash_ctrl->cdev_torch.max_brightness
        = ((struct hw_mp3331_front_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
    flash_ctrl->cdev_torch.brightness_set = hw_mp3331_front_torch_brightness_set;
    rc = led_classdev_register((struct device *)dev, &flash_ctrl->cdev_torch);
    if (rc < 0) {
        cam_err("%s failed to register torch classdev.", __func__);
        goto err_out;
    }
#ifdef CAMERA_FLASH_FACTORY_TEST
    rc = device_create_file(dev, &hw_mp3331_front_lightness);
    if (rc < 0) {
        cam_err("%s failed to creat lightness attribute.", __func__);
        goto err_create_lightness_file;
    }
#endif
    return 0;
#ifdef CAMERA_FLASH_FACTORY_TEST
err_create_lightness_file:
    led_classdev_unregister(&flash_ctrl->cdev_torch);
#endif
err_out:
    return rc;
}

static int hw_mp3331_front_match(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_mp3331_front_private_data_t *pdata;
    unsigned char id;
    int rc = 0;
    int i = 0;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = (struct hw_mp3331_front_private_data_t *)flash_ctrl->pdata;

    for(i = 0; i < MAX_MATCH_TIME; i++){
         rc = i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id);
         if(rc < 0){
             cam_err("%s read flash chip id error i = %d .", __func__,i);
             continue;
         }

         if( pdata->chipid == (id & FLASH_CHIP_ID_MASK) ){
             cam_info("%s match success, id:0x%x\n", __func__, id);
             break;
         }
    }

    if(i >= MAX_MATCH_TIME){
         cam_err("%s match fail\n", __func__);
         return -EINVAL;
    }

    rc = hw_mp3331_front_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
         cam_err("%s clear err and unlock error.", __func__);
         return rc;
    }

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_PEAK_CUR_SET, FS_SD) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_TIMER_SET, FL_TIM) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LOW_VLO_SET, VBL_RUN|VBL_SD) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_INDICATOR_SET, LED_OTAD|TUP_I|VTH_PAS|INDI_PWM) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_FLASH_CUR_SET, I_FL) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TXMASK_CUR_SET, I_TX) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_TORCH_CUR_SET, I_TOR) < 0);
#ifdef CAMERA_FLASH_FACTORY_TEST

    rc = register_camerafs_attr(&hw_mp3331_front_lightness);
    if (rc < 0) {
        cam_err("%s failed to creat lightness attribute.", __func__);
        return -EINVAL;
    }
#endif
    return 0;
}

static int hw_mp3331_front_remove(struct i2c_client *client)
{
    cam_debug("%s enter.", __func__);
    if (NULL == client) {
        cam_err("%s invalid param", __func__);
        return -EINVAL;
    }
    hw_mp3331_front_ctrl.func_tbl->flash_exit(&hw_mp3331_front_ctrl);

    client->adapter = NULL;
    return 0;
}

static void hw_mp3331_front_shutdown(struct i2c_client *client)
{
    int rc = -1;
    if (NULL == client) {
        cam_err("%s invalid param", __func__);
        return;
    }
    rc = hw_mp3331_front_off(&hw_mp3331_front_ctrl);
    cam_info("%s mp3331_front shut down at (%d)",__func__,__LINE__);
    if (rc < 0) {
        cam_err("%s mp3331_front flash off error.", __func__);
    }
}

static const struct i2c_device_id hw_mp3331_front_id[] = {
    {"mp3331_front", (unsigned long)&hw_mp3331_front_ctrl},
    {}
};

static const struct of_device_id hw_mp3331_front_dt_match[] = {
    {.compatible = "huawei,mp3331_front"},
    {}
};
MODULE_DEVICE_TABLE(of, mp3331_front_dt_match);

static struct i2c_driver hw_mp3331_front_i2c_driver = {
    .probe  = hw_flash_i2c_probe,
    .remove = hw_mp3331_front_remove,
    .shutdown = hw_mp3331_front_shutdown,
    .id_table   = hw_mp3331_front_id,
    .driver = {
        .name = "hw_mp3331_front",
        .of_match_table = hw_mp3331_front_dt_match,
    },
};

static int __init hw_mp3331_front_module_init(void)
{
    cam_info("%s enter.\n", __func__);
    return i2c_add_driver(&hw_mp3331_front_i2c_driver);
}

static void __exit hw_mp3331_front_module_exit(void)
{
    cam_info("%s enter.", __func__);
    i2c_del_driver(&hw_mp3331_front_i2c_driver);
    return;
}

static struct hw_flash_i2c_client hw_mp3331_front_i2c_client;

static struct hw_flash_fn_t hw_mp3331_front_func_tbl = {
    .flash_config = hw_flash_config,
    .flash_init = hw_mp3331_front_init,
    .flash_exit = hw_mp3331_front_exit,
    .flash_on = hw_mp3331_front_on,
    .flash_off = hw_mp3331_front_off,
    .flash_match = hw_mp3331_front_match,
    .flash_get_dt_data = hw_mp3331_front_get_dt_data,
    .flash_register_attribute = hw_mp3331_front_register_attribute,
};

static struct hw_flash_ctrl_t hw_mp3331_front_ctrl = {
    .flash_i2c_client = &hw_mp3331_front_i2c_client,
    .func_tbl = &hw_mp3331_front_func_tbl,
    .hw_flash_mutex = &flash_mut_mp3331_front,
    .pdata = (void*)&hw_mp3331_front_pdata,
    .flash_mask_enable = false,
    .state = {
        .mode = STANDBY_MODE,
    },
};

module_init(hw_mp3331_front_module_init);
module_exit(hw_mp3331_front_module_exit);
MODULE_DESCRIPTION("MP3331_FRONT FLASH");
MODULE_LICENSE("GPL v2");
