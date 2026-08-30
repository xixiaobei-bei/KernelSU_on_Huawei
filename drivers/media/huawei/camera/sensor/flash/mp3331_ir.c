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
#include "aux_flash_ldo.h"
#include "hisi_adc.h" //for ito adc
//lint -save -e31 -e64 -e84 -e708 -e712 -e713 -e715 -e732 -e734 -e737 -e651
//lint -save -e747 -e753 -e778 -e785 -e838 -e846 -e866 -e514 -e528 -e570 -e574 -e578
/* MP3331_IR Registers define */
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
#define TORCH_MODE_CUR                          0x04    //Device mode setting bits assist light mode
#define TORCH_TX_MODE_CUR                       0x00    //Device mode setting bits torch mode
#define INDICATOR_MODE_CUR                      0x02    //Device mode setting bits indicator mode
#define FS_SD                                   0x03    // Disable switching frequency stretching down from 1Mhz if VIN is close to VOUT
#define FL_TIM                                  0xB0    // set Flash timer , 600ms
#define VBL_RUN                                 0x4F    // Low battery voltage setting, set 3.2v
#define VBL_RUN_DIS                             0xFF    // Disable low battery voltage setting
#define VBL_SD                                  0x1F   //Device disable when VIN is less than threshold set by VBL_RUN before flash
#define LED_OTAD                                0x80   //Adaptive thermal flash current control bit
#define TUP_I                                   0x70   // current ramp up time 64us/step
#define VTH_PAS                                 0x04   //VTS_PAS:450mV
#define INDI_PWM                                0x03   //PWM duty: 11=5/16
#define I_FL                                    0x18   //flash current:760.8mA
#define I_TX                                    0x08   // I_TX:253.6mA
#define I_TOR                                   0x05   // torch current:158.5mA
#define TORCH_PIN                               0x08
#define BASE                                    10
#define MAX_MATCH_TIME                          3

#define MP3331_IR_FLASH_DEFAULT_CUR_LEV      24  //760mA
#define MP3331_IR_TORCH_DEFAULT_CUR_LEV      6    //190.2mA
#define MP3331_IR_FLASH_MAX_CUR_LEV          47  //1490mA
#define MP3331_IR_TORCH_MAX_CUR_LEV          12   //380mA
#define MP3331_IR_CUR_STEP_LEV               317  //31.7mA * 10
#define MP3331_IR_TORCH_RT_CUR               48  //mA

#define FLASH_CHIP_ID_MASK                      0xF8
#define FLASH_LED_LEVEL_INVALID                 0xff

#define MP3331_IR_OVER_VOLTAGE_PROTECT       0x40
#define MP3331_IR_VOUT_SHORT                 0x20
#define MP3331_IR_LED_SHORT                  0x10
#define MP3331_IR_OVER_TEMP_PROTECT          0x08
#define MP3331_IR_LED_OPEN                   0x01

#define MP3331_FLASH_MAX_CUR                    1500 //1500ma
#define MAX_FLASH_ID                            0xFF //used to check flash_lightness_sotre input param
#define MAX_LIGHTNESS_PARAM_NUM_RT           3
#define LIGHTNESS_PARAM_FD_INDEX_RT          0
#define LIGHTNESS_PARAM_MODE_INDEX_RT        1
#define LIGHTNESS_PARAM_CURRENT_INDEX_RT     2

#define CONVERT2TORCH(x)                        ((x)*16/5)

#define RETURN_ERROR_IF(x) \
        if ((x)) { \
            cam_err("%s error %s.", __func__, #x); \
            return -EINVAL; \
        }

#define RETURN_ERROR_ON_NULL(x) \
        if ((x)==NULL) { \
            cam_err("%s invalid params %s.", __func__, #x); \
            return -EINVAL; \
        }

#define LIMITE_VAL_CELLING(x, celling) \
        if  ((x) > (celling)) { \
            (x) = (celling);     \
        }

#define RETURN_ERROR_OUT_OF_BOUNDS(x, min, max)     \
    if (((x) < (min)) || ((x) > (max))) {          \
            cam_err("%s out of bounds: %s.", __func__, #x); \
            return -EINVAL; \
        }

typedef enum {
    CURRENT_TORCH_LEVEL_MMI = 0,//use for MMIE IR test
    CURRENT_TORCH_LEVEL_RT = 1,//use for RT current test
    CURRENT_MAX = 2,
}mp3331_ir_current_conf;

/*ir flash RT self test param*/
typedef enum {
    IR_SELFTEST_ON_TIME_INDEX  = 0,
    IR_SELFTEST_OFF_TIME_INDEX = 1,
    IR_SELFTEST_CURRENT_INDEX  = 2,
    IR_SELFTEST_TIMES_INDEX    = 3,
    IR_SELFTEST_MAX_INDEX      = 4,
}mp3331_ir_selftest_param;


/*ir ito threshold param*/
typedef enum {
    ITO_THR_MIN  = 0,
    ITO_THR_MAX  = 1,
    ITO_THR_MAX_INDEX = 2,
}mp3331_ito_threshold_param;
/* Internal data struct define */
struct hw_mp3331_ir_private_data_t {
    unsigned int torch_led_num;
    unsigned int flash_current;
    unsigned int torch_current;
    /* flash electric current config */
    unsigned int ecurrent[CURRENT_MAX];
    unsigned int selftest_param[IR_SELFTEST_MAX_INDEX];

    unsigned int chipid;
    /* flash ldo*/
    unsigned int need_flash_ldo;
    unsigned int flash_ldo_voltage;
    unsigned int is_power_on;
    /* ito */
    unsigned int ito_enable;
    unsigned int ito_threshold[ITO_THR_MAX_INDEX];
};

/* Internal varible define */
static struct hw_mp3331_ir_private_data_t hw_mp3331_ir_pdata;
static struct hw_flash_ctrl_t hw_mp3331_ir_ctrl;
static struct i2c_driver hw_mp3331_ir_i2c_driver;

extern struct dsm_client *client_flash;

DEFINE_HISI_FLASH_MUTEX(mp3331_ir);

#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif

/****************************************************************************
* FunctionName: msm_flash_clear_err_and_unlock;
* Description : clear the error and unlock the IC ;
* NOTE: this funtion must be called before register is read and write
***************************************************************************/
static int hw_mp3331_ir_clear_err_and_unlock(struct hw_flash_ctrl_t *flash_ctrl)
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

    if (fault_h & MP3331_IR_OVER_TEMP_PROTECT) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash temperature is too hot! FlagReg_H[0x%x]\n", fault_h);
            dsm_client_notify(client_flash, DSM_FLASH_HOT_DIE_ERROR_NO);
            cam_warn("[I/DSM] %s flash temperature is too hot! FlagReg_H[0x%x]", __func__, fault_h);
        }
    }

    if (fault_h & (MP3331_IR_OVER_VOLTAGE_PROTECT | MP3331_IR_VOUT_SHORT | MP3331_IR_LED_SHORT)) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash OVP, VOUT or LED short! FlagReg_H[0x%x]\n", fault_h);
            dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
            cam_warn("[I/DSM] %s flash OVP, VOUT or LED short! FlagReg_H[0x%x]", __func__, (unsigned int)fault_h);
        }
    }

    if (fault_l & MP3331_IR_LED_OPEN) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash LED Open! FlagReg_L[0x%x]\n", fault_l);
            dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
            cam_warn("[I/DSM] %s flash LED Opent! FlagReg_L[0x%x]", __func__, (unsigned int)fault_l);
        }
    }

    cam_info("%s fault_h=0x%x, fault_l=0x%x \n", __func__, fault_h, fault_l);

    return 0;
}

static int hw_mp3331_ir_find_match_current(int cur_flash)
{
    int integer_flash = 0;
    int remainder_flash = 0;

    if(cur_flash <= 0){
        cam_err("%s current set is error", __func__);
        return -EINVAL;
    }

    integer_flash = (cur_flash * BASE) / MP3331_IR_CUR_STEP_LEV;
    remainder_flash = (cur_flash * BASE) % MP3331_IR_CUR_STEP_LEV;

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

static int hw_mp3331_ir_init(struct hw_flash_ctrl_t *flash_ctrl)
{
    //The chip init reg move to match id function for the hw_flash flow
    cam_debug("%s enter.\n", __func__);
    if (NULL == flash_ctrl) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    return 0;
}

static int hw_mp3331_ir_exit(struct hw_flash_ctrl_t *flash_ctrl)
{
    cam_debug("%s enter.\n", __func__);
    if (NULL == flash_ctrl || (NULL == flash_ctrl->func_tbl) || (NULL == flash_ctrl->func_tbl->flash_off)) {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }

    flash_ctrl->func_tbl->flash_off(flash_ctrl);

    return 0;
}

static int hw_mp3331_ir_flash_mode(struct hw_flash_ctrl_t *flash_ctrl, int data)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_mp3331_ir_private_data_t *pdata;
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
        current_level = MP3331_IR_FLASH_DEFAULT_CUR_LEV;
    }
    else
    {
        if( data > MP3331_IR_FLASH_MAX_CUR_LEV * MP3331_IR_CUR_STEP_LEV / BASE){
            current_level = MP3331_IR_FLASH_DEFAULT_CUR_LEV;
        }
        else{
            current_level = hw_mp3331_ir_find_match_current(data);
            if(current_level < 0){
                current_level = MP3331_IR_FLASH_DEFAULT_CUR_LEV;
            }
        }
    }
    /* clear error flag,resume chip */
    rc = hw_mp3331_ir_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
        cam_err("%s clear err and unlock error.", __func__);
        return rc;
    }

    /* set LED Flash current value */
    cam_info("%s led flash current  current level=%d.\n", __func__, current_level);

    if (pdata->need_flash_ldo == 1) {
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_PEAK_CUR_SET, FS_SD) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LOW_VLO_SET, VBL_RUN_DIS) < 0);//diable low voltage
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_INDICATOR_SET, LED_OTAD|TUP_I|VTH_PAS) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TXMASK_CUR_SET, I_TX) < 0);
    }
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|FLASH_MODE_CUR) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_FLASH_CUR_SET, current_level) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_TIMER_SET, FL_TIM) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|FLASH_MODE_CUR|LED1_EN) < 0);

    return rc;
}

static int hw_mp3331_ir_torch_mode(struct hw_flash_ctrl_t *flash_ctrl, int data)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_mp3331_ir_private_data_t *pdata;
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
    pdata = (struct hw_mp3331_ir_private_data_t *)flash_ctrl->pdata;
    if (FLASH_LED_LEVEL_INVALID == pdata->torch_current)
    {
        current_level = MP3331_IR_TORCH_DEFAULT_CUR_LEV;
    }
    else
    {
        if( data > MP3331_IR_TORCH_MAX_CUR_LEV * MP3331_IR_CUR_STEP_LEV / BASE){
            current_level = MP3331_IR_TORCH_MAX_CUR_LEV;
        }
        else{
            current_level = hw_mp3331_ir_find_match_current(data);
            if(current_level < 0){
                current_level = MP3331_IR_TORCH_DEFAULT_CUR_LEV;
            }
        }
    }
    /* clear error flag,resume chip */
    rc = hw_mp3331_ir_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
        cam_err("%s clear err and unlock error.", __func__);
        return rc;
    }

    /* set LED Flash current value */
    cam_info("%s the led torch current current_level=%d.\n", __func__, current_level);

    if (pdata->need_flash_ldo == 1) {
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_PEAK_CUR_SET, FS_SD) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_TIMER_SET, FL_TIM) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LOW_VLO_SET, VBL_RUN_DIS) < 0);//diable low voltage
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_INDICATOR_SET, LED_OTAD|TUP_I|VTH_PAS) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TXMASK_CUR_SET, I_TX) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|TORCH_TX_MODE_CUR|TORCH_PIN) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_TORCH_CUR_SET, current_level) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|TORCH_TX_MODE_CUR|TORCH_PIN|LED1_EN) < 0);
    } else {
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|TORCH_MODE_CUR) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_TORCH_CUR_SET, current_level) < 0);
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV|TORCH_MODE_CUR|LED1_EN) < 0);
    }

    return rc;
}

static int hw_mp3331_ir_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
    struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
    int rc = -1;
    struct hw_mp3331_ir_private_data_t *pdata = NULL;

    RETURN_ERROR_ON_NULL(flash_ctrl);
    RETURN_ERROR_ON_NULL(cdata);
    RETURN_ERROR_ON_NULL(flash_ctrl->pdata);

    pdata = (struct hw_mp3331_ir_private_data_t *)flash_ctrl->pdata;
    mutex_lock(flash_ctrl->hw_flash_mutex);
    if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 0)) {
        rc = aux_ldo_supply_enable(flash_ctrl, pdata->flash_ldo_voltage);

        if (rc < 0) {
            cam_err("%s power on fail.", __func__);
            mutex_unlock(flash_ctrl->hw_flash_mutex);
            return rc;
        }
        pdata->is_power_on = 1;
    }

    cam_info("%s mode=%d, level=%d.\n", __func__, cdata->mode, cdata->data);

    //check ITO protect
    if (pdata->ito_enable) {
        bool dsm_flag = false;
        int ret = hisi_adc_get_value(12);//ITO Use ADC 12
        if (ret >= 0)
        {
           if (((unsigned int)ret < pdata->ito_threshold[ITO_THR_MIN]) || ( (unsigned int)ret > pdata->ito_threshold[ITO_THR_MAX])) {
               dsm_flag = true;
               cam_err("%s ito %d out of [%d,%d]", __func__, ret, pdata->ito_threshold[ITO_THR_MIN], pdata->ito_threshold[ITO_THR_MAX]);
            }
        } else {
            dsm_flag = true;
            cam_err("%s get ito fail ret %d", __func__, ret);
        }
        cam_info("%s ito adc ret %d", __func__, ret);
        if (dsm_flag) {
               mutex_unlock(flash_ctrl->hw_flash_mutex);
               //repo dsm
               if (!dsm_client_ocuppy(client_flash)) {
                    dsm_client_record(client_flash, "ito %d out of [%d,%d]\n", ret, pdata->ito_threshold[ITO_THR_MIN], pdata->ito_threshold[ITO_THR_MAX]);
                    dsm_client_notify(client_flash, DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
                    cam_warn("[I/DSM] %s report[%d]", __func__, DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
               }
              return -EINVAL;
        }
    }


    if (FLASH_MODE == cdata->mode) {
        rc = hw_mp3331_ir_flash_mode(flash_ctrl, cdata->data);
    } else {
        rc = hw_mp3331_ir_torch_mode(flash_ctrl, cdata->data);
    }
    flash_ctrl->state.mode = cdata->mode;
    flash_ctrl->state.data = cdata->data;
    mutex_unlock(flash_ctrl->hw_flash_mutex);

    return rc;
}

static int hw_mp3331_ir_off(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    int rc = 0;
    struct hw_mp3331_ir_private_data_t *pdata;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->flash_i2c_client)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
        || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)
        || (NULL == flash_ctrl->pdata)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    pdata = (struct hw_mp3331_ir_private_data_t *)flash_ctrl->pdata;

    mutex_lock(flash_ctrl->hw_flash_mutex);
    flash_ctrl->state.mode = STANDBY_MODE;
    flash_ctrl->state.data = 0;
    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

    if ((pdata->need_flash_ldo == 0) || ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 1))) {
        /* clear error flag,resume chip */
        rc = hw_mp3331_ir_clear_err_and_unlock(flash_ctrl);
        if(rc < 0)
        {
            cam_err("%s clear err and unlock error.", __func__);
        }

        if(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV) < 0){
            cam_err("%s %d", __func__, __LINE__);
        }
    }

    if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 1))
    {
        rc = aux_ldo_supply_disable();
        if (rc < 0) {
            cam_err("%s power off fail.", __func__);
            mutex_unlock(flash_ctrl->hw_flash_mutex);
            return rc;
        }
        pdata->is_power_on = 0;
    }
    mutex_unlock(flash_ctrl->hw_flash_mutex);

    return 0;
}

static int hw_mp3331_ir_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_mp3331_ir_private_data_t *pdata;
    struct device_node *of_node;
    int rc = -1;
    int i = 0;

    cam_debug("%s enter.\n", __func__);
    RETURN_ERROR_ON_NULL(flash_ctrl);
    RETURN_ERROR_ON_NULL(flash_ctrl->pdata);
    RETURN_ERROR_ON_NULL(flash_ctrl->dev);

    pdata = (struct hw_mp3331_ir_private_data_t *)flash_ctrl->pdata;
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

    rc = of_property_read_u32_array(of_node, "huawei,flash-current",
            pdata->ecurrent, CURRENT_MAX);
    if (rc < 0) {
        cam_err("%s read flash-current failed line %d\n", __func__, __LINE__);
        return rc;
    } else {
        for (i = 0; i < CURRENT_MAX; i++) {
            cam_info("%s ecurrent[%d]=%d.\n", __func__, i,
                     pdata->ecurrent[i]);
        }
    }

    rc = of_property_read_u32_array(of_node, "huawei,selftest-param",
            pdata->selftest_param, IR_SELFTEST_MAX_INDEX);
    if (rc < 0) {
        cam_err("%s read selftest-param failed line %d\n", __func__, __LINE__);
        return rc;
    } else {
        for (i = 0; i < IR_SELFTEST_MAX_INDEX; i++) {
            cam_info("%s selftest_param[%d]=%d.\n", __func__, i,
                     pdata->selftest_param[i]);
        }
    }

    pdata->ito_enable = 0;
    pdata->ito_threshold[ITO_THR_MIN] = 0;
    pdata->ito_threshold[ITO_THR_MAX] = 0;
    rc = of_property_read_u32(of_node, "huawei,ito_protect_enable",
        &pdata->ito_enable);
    cam_debug("%s ito_enable %d, rc %d", __func__,
        pdata->ito_enable, rc);
    if (rc < 0) {
        cam_info("%s failed %d", __func__, __LINE__);
        pdata->ito_enable = 0;
    }

    if (pdata->ito_enable) {
        rc = of_property_read_u32_array(of_node, "huawei,ito_protect_threshold",
                pdata->ito_threshold, ITO_THR_MAX_INDEX);
        if (rc < 0) {
            cam_info("%s read ito_protect_threshold failed line %d\n", __func__, __LINE__);
        } else {
            for (i = 0; i < ITO_THR_MAX_INDEX; i++) {
                cam_info("%s ito_protect_threshold[%d]=%d.\n", __func__, i,
                         pdata->ito_threshold[i]);
            }
        }
    }

    return rc;
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_mp3331_ir_lightness_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }
    int rc=0;

    //show param for ir selftest for RT,PAGE_SIZE = 4096
    rc = scnprintf(buf, PAGE_SIZE, "%d %d %d %d",
                   hw_mp3331_ir_pdata.selftest_param[IR_SELFTEST_ON_TIME_INDEX],
                   hw_mp3331_ir_pdata.selftest_param[IR_SELFTEST_OFF_TIME_INDEX],
                   hw_mp3331_ir_pdata.selftest_param[IR_SELFTEST_CURRENT_INDEX],
                   hw_mp3331_ir_pdata.selftest_param[IR_SELFTEST_TIMES_INDEX]);

    return rc;
}

static int hw_mp3331_ir_param_check(char *buf, unsigned long *param,
    int num_of_par)
{
    if ((NULL == buf) || (NULL == param))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }
    char *token = NULL;
    int base = 0;
    int cnt = 0;

    token = strsep(&buf, " ");

    for (cnt = 0; cnt < num_of_par; cnt++)
    {
        if (token != NULL) {
            if (strlen(token) < 2) { //add for check token[1] has valid element
                base = 10; //decimal
            } else {
                //format 0x**
                if ((token[1] == 'x') || (token[1] == 'X')) {
                    base = 16;  //hex
                } else {
                    base = 10;  //decimal
                }
            }
            if (strict_strtoul(token, base, &param[cnt]) != 0) {
                return -EINVAL;
            }
            token = strsep(&buf, " ");
        } else {
            cam_info("%s the %d param is null\n", __func__, num_of_par);
        }
    }
    return 0;
}

static ssize_t hw_mp3331_ir_lightness_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    RETURN_ERROR_ON_NULL(buf);
    RETURN_ERROR_ON_NULL(dev);
    RETURN_ERROR_ON_NULL(attr);

    struct hw_flash_cfg_data cdata = {0};
    unsigned long param[MAX_LIGHTNESS_PARAM_NUM_RT]={0};
    int rc=0;
    int flash_id = 0;

    cam_info("%s enter,buf=%s.", __func__,buf);
    rc = hw_mp3331_ir_param_check((char *)buf, param, MAX_LIGHTNESS_PARAM_NUM_RT);
    if (rc < 0) {
        cam_err("%s failed to check param.", __func__);
        return rc;
    }

    flash_id = (int)param[LIGHTNESS_PARAM_FD_INDEX_RT]; //0 - flash id
    cdata.mode = (int)param[LIGHTNESS_PARAM_MODE_INDEX_RT];   //1 - mode
    cdata.data = (int)param[LIGHTNESS_PARAM_CURRENT_INDEX_RT];   //2 - current
    cam_info("%s flash_id=%d, cdata.mode=%d, cdata.data=%d.", __func__, flash_id, cdata.mode, cdata.data);

    RETURN_ERROR_OUT_OF_BOUNDS(cdata.mode, STANDBY_MODE, MAX_MODE);
    RETURN_ERROR_OUT_OF_BOUNDS(cdata.data, 0, MP3331_FLASH_MAX_CUR);//0~1500 mA
    RETURN_ERROR_OUT_OF_BOUNDS(flash_id, 0, MAX_FLASH_ID);//0~0xFF, pass from HAL, but not use just print

    if (cdata.mode == STANDBY_MODE) {
        rc = hw_mp3331_ir_off(&hw_mp3331_ir_ctrl);
        if (rc < 0) {
            cam_err("%s mp3331_ir flash off error.", __func__);
            return rc;
        }
    } else if(cdata.mode == TORCH_MODE){
        cdata.data = hw_mp3331_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_RT];
        cam_info("%s mode=%d, current=%d.", __func__, cdata.mode, cdata.data);

        rc = hw_mp3331_ir_on(&hw_mp3331_ir_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s mp3331_ir flash on error.", __func__);
            return rc;
        }
    } else if(cdata.mode == FLASH_MODE){
        cam_info("%s mode=%d, current=%d.", __func__, cdata.mode, cdata.data);
        cdata.mode = TORCH_MODE;
        rc = hw_mp3331_ir_on(&hw_mp3331_ir_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s flash on error.", __func__);
            return rc;
        }
    } else {
        cam_err("%s wrong mode=%d.", __func__,cdata.mode);
        return -EINVAL;
    }

    return count;
}
#endif

static void hw_mp3331_ir_torch_brightness_set(struct led_classdev *cdev,
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
        rc = hw_mp3331_ir_off(&hw_mp3331_ir_ctrl);
        if (rc < 0) {
            cam_err("%s pmu_led off error.", __func__);
            return;
        }
    } else {
        cdata.mode = TORCH_MODE;
        cdata.data = hw_mp3331_ir_pdata.ecurrent[CURRENT_TORCH_LEVEL_MMI];

        cam_info("%s current=%d.", __func__, cdata.data);
        rc = hw_mp3331_ir_on(&hw_mp3331_ir_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s pmu_led on error.", __func__);
            return;
        }
    }
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute hw_mp3331_ir_lightness =
    __ATTR(flash_lightness_f, 0660, hw_mp3331_ir_lightness_show, hw_mp3331_ir_lightness_store);
#endif

static int hw_mp3331_ir_register_attribute(struct hw_flash_ctrl_t *flash_ctrl, struct device *dev)
{
    int rc = 0;

    if ((NULL == flash_ctrl) || (NULL == dev) || (NULL == flash_ctrl->pdata)) {
        cam_err("%s flash_ctrl or dev is NULL.", __func__);
        return -EINVAL;
    }

    flash_ctrl->cdev_torch.name = "torch_front";
    flash_ctrl->cdev_torch.max_brightness
        = ((struct hw_mp3331_ir_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
    flash_ctrl->cdev_torch.brightness_set = hw_mp3331_ir_torch_brightness_set;
    rc = led_classdev_register((struct device *)dev, &flash_ctrl->cdev_torch);
    if (rc < 0) {
        cam_err("%s failed to register torch classdev.", __func__);
        goto err_out;
    }
#ifdef CAMERA_FLASH_FACTORY_TEST
    rc = device_create_file(dev, &hw_mp3331_ir_lightness);
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

static int hw_mp3331_ir_match(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_mp3331_ir_private_data_t *pdata;
    unsigned char id;
    int rc = 0;
    int i = 0;

    cam_debug("%s enter.\n", __func__);
    RETURN_ERROR_ON_NULL(flash_ctrl);
    RETURN_ERROR_ON_NULL(flash_ctrl->pdata);
    RETURN_ERROR_ON_NULL(flash_ctrl->flash_i2c_client);
    RETURN_ERROR_ON_NULL(flash_ctrl->flash_i2c_client->i2c_func_tbl);
    RETURN_ERROR_ON_NULL(flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read);
    RETURN_ERROR_ON_NULL(flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write);

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = (struct hw_mp3331_ir_private_data_t *)flash_ctrl->pdata;

    if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 0)) {
        loge_if_ret(aux_ldo_supply_enable(flash_ctrl, pdata->flash_ldo_voltage) < 0);
        pdata->is_power_on = 1;
    }
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
         goto err;
    }

    rc = hw_mp3331_ir_clear_err_and_unlock(flash_ctrl);
    if(rc < 0)
    {
         cam_err("%s clear err and unlock error.", __func__);
         goto err;
    }

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_MODE_SET, STB_LV) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_PEAK_CUR_SET, FS_SD) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_TIMER_SET, FL_TIM) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LOW_VLO_SET, VBL_RUN|VBL_SD) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_INDICATOR_SET, LED_OTAD|TUP_I|VTH_PAS|INDI_PWM) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_FLASH_CUR_SET, I_FL) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TXMASK_CUR_SET, I_TX) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_LED_TORCH_CUR_SET, I_TOR) < 0);

    if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 1)) {
        loge_if_ret(aux_ldo_supply_disable() < 0);
        pdata->is_power_on = 0;
    }

#ifdef CAMERA_FLASH_FACTORY_TEST

    rc = register_camerafs_attr(&hw_mp3331_ir_lightness);
    if (rc < 0) {
        cam_err("%s failed to creat lightness attribute.", __func__);
        return -EINVAL;
    }
#endif

    return 0;

err:
    if ((pdata->need_flash_ldo == 1) && (pdata->is_power_on == 1)) {
        loge_if_ret(aux_ldo_supply_disable() < 0);
        pdata->is_power_on = 0;
    }
    return -EINVAL;
}

static int hw_mp3331_ir_remove(struct i2c_client *client)
{
    cam_debug("%s enter.", __func__);
    if (NULL == client) {
        cam_err("%s invalid param", __func__);
        return -EINVAL;
    }
    hw_mp3331_ir_ctrl.func_tbl->flash_exit(&hw_mp3331_ir_ctrl);

    client->adapter = NULL;
    return 0;
}

static void hw_mp3331_ir_shutdown(struct i2c_client *client)
{
    int rc = -1;
    if (NULL == client) {
        cam_err("%s invalid param", __func__);
        return;
    }
    rc = hw_mp3331_ir_off(&hw_mp3331_ir_ctrl);
    cam_info("%s mp3331_ir shut down at (%d)",__func__,__LINE__);
    if (rc < 0) {
        cam_err("%s mp3331_ir flash off error.", __func__);
    }
}

static const struct i2c_device_id hw_mp3331_ir_id[] = {
    {"mp3331_ir", (unsigned long)&hw_mp3331_ir_ctrl},
    {}
};

static const struct of_device_id hw_mp3331_ir_dt_match[] = {
    {.compatible = "huawei,mp3331_ir"},
    {}
};
MODULE_DEVICE_TABLE(of, mp3331_ir_dt_match);

static struct i2c_driver hw_mp3331_ir_i2c_driver = {
    .probe  = hw_flash_i2c_probe,
    .remove = hw_mp3331_ir_remove,
    .shutdown = hw_mp3331_ir_shutdown,
    .id_table   = hw_mp3331_ir_id,
    .driver = {
        .name = "hw_mp3331_ir",
        .of_match_table = hw_mp3331_ir_dt_match,
    },
};

static int __init hw_mp3331_ir_module_init(void)
{
    cam_info("%s enter.\n", __func__);
    return i2c_add_driver(&hw_mp3331_ir_i2c_driver);
}

static void __exit hw_mp3331_ir_module_exit(void)
{
    cam_info("%s enter.", __func__);
    i2c_del_driver(&hw_mp3331_ir_i2c_driver);
    return;
}

static struct hw_flash_i2c_client hw_mp3331_ir_i2c_client;

static struct hw_flash_fn_t hw_mp3331_ir_func_tbl = {
    .flash_config = hw_flash_config,
    .flash_init = hw_mp3331_ir_init,
    .flash_exit = hw_mp3331_ir_exit,
    .flash_on = hw_mp3331_ir_on,
    .flash_off = hw_mp3331_ir_off,
    .flash_match = hw_mp3331_ir_match,
    .flash_get_dt_data = hw_mp3331_ir_get_dt_data,
    .flash_register_attribute = hw_mp3331_ir_register_attribute,
};

static struct hw_flash_ctrl_t hw_mp3331_ir_ctrl = {
    .flash_i2c_client = &hw_mp3331_ir_i2c_client,
    .func_tbl = &hw_mp3331_ir_func_tbl,
    .hw_flash_mutex = &flash_mut_mp3331_ir,
    .pdata = (void*)&hw_mp3331_ir_pdata,
    .flash_mask_enable = false,
    .state = {
        .mode = STANDBY_MODE,
    },
};

module_init(hw_mp3331_ir_module_init);
module_exit(hw_mp3331_ir_module_exit);
MODULE_DESCRIPTION("MP3331_IR FLASH");
MODULE_LICENSE("GPL v2");
