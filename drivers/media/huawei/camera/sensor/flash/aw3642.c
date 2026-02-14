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
/* AW3642 Registers define */
#define AW3642_SLAVE_ADDRESS            (0x63)
#define REG_CHIP_ID                     (0x00)
#define REG_ENABLE                      (0x01)
#define REG_IVFM                        (0x02)
#define REG_OCP_FLAGS                   (0x0A)
#define REG_OVP_FLAGS                   (0x0B)
#define REG_TIMING_CONFIG               (0x08)
#define REG_FLASH_CURRENT_CONTROL       (0x03)
#define REG_TORCH_CURRENT_CONTROL       (0x05)
#define MODE_STANDBY                    (0x00)
#define MODE_TORCH                      (0x08)
#define MODE_FLASH                      (0x0c)
#define IVFM_EN                         (0x01)
#define UVLO_EN                         (0x40) //bit6 1:enable, 0 disable
#define IVFM_VOLTAGE                    (0x18) //3.2v
#define TX_PIN_EN                       (0x80)
#define TX_PIN_DISABLE                  (0x00)
#define LED_EN                          (0x03)
#define AW3642_FLASH_DEFAULT_CUR_LEV    (31) //754mA
#define AW3642_TORCH_DEFAULT_CUR_LEV    (23) //141mA
#define AW3642_FLASH_MAX_CUR            (1500)//1500mA
#define AW3642_TORCH_MAX_CUR            (376)
#define AW3642_TORCH_MAX_RT_CUR         (190)//190mA
#define AW3642_TORCH_MAX_MMI_CUR        (48)//48mA
#define AW3642_TORCH_MAX_LEV            (63)
#define FLASH_LED_MAX                   (64)
#define TORCH_LED_MAX                   (64)
#define FLASH_LED_LEVEL_INVALID         (0xff)
#define AW3642_UNDER_VOLTAGE_LOCKOUT    (0x02)
#define AW3642_OVER_VOLTAGE_PROTECT     (0x02)
#define AW3642_LED_SHORT                (0x10)
#define AW3642_VOUT_SHORT               (0x40)
#define AW3642_OVER_TEMP_PROTECT        (0x04)
#define PARAM_LEN                       (2)

/* Internal data struct define */
struct hw_aw3642_private_data_t {
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
/* flash: 23.4mA-1.5A, 64 levels */
static int flash_arry[FLASH_LED_MAX] = {24, 47, 71, 95, 118, 142, 165, 189, 212, 236, 260, 283, 307, 331, 354, 378,
                             401, 424, 448, 471, 495, 519, 542, 566, 589, 613, 636, 659, 683, 707, 730, 754,
                             775, 799, 822, 846, 870, 894, 917, 940, 964, 988, 1010, 1034, 1058, 1082, 1105, 1128,
                             1149, 1173, 1196, 1219, 1244, 1267, 1290, 1313, 1337, 1361, 1383, 1407, 1431, 1455, 1477, 1500};

/* torch: 5.4mA-376mA, 64 levels */
static int torch_arry[TORCH_LED_MAX] = {6, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 70, 76, 82, 88, 94,
                             100, 106, 111, 117, 123, 129, 135, 141, 147, 153, 159, 165, 170, 176, 182, 188,
                             193, 199, 205, 211, 217, 223, 229, 235, 241, 247, 252, 258, 264, 270, 276, 282,
                             288, 293, 299, 305, 311, 317, 323, 329, 335, 341, 346, 353, 358, 364, 370, 376};

/* Internal varible define */
static struct hw_aw3642_private_data_t hw_aw3642_pdata;
static struct hw_flash_ctrl_t hw_aw3642_ctrl;
static struct i2c_driver hw_aw3642_i2c_driver;
extern struct dsm_client *client_flash;

DEFINE_HISI_FLASH_MUTEX(aw3642);
#ifdef CAMERA_FLASH_FACTORY_TEST
extern int register_camerafs_attr(struct device_attribute *attr);
#endif

static int hw_aw3642_clear_error_and_notify_dmd(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    unsigned char ocp_reg_val = 0;
    unsigned char ovp_reg_val = 0;
    int rc_ocp = 0;
    int rc_ovp = 0;

    if (NULL == flash_ctrl || NULL == flash_ctrl->flash_i2c_client
        || NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl
        || NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

    /* clear error flag,resume chip */
    rc_ocp = i2c_func->i2c_read(i2c_client, REG_OCP_FLAGS, &ocp_reg_val);
    rc_ovp = i2c_func->i2c_read(i2c_client, REG_OVP_FLAGS, &ovp_reg_val);
    if((rc_ocp < 0)||(rc_ovp < 0)) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash i2c transfer fail\n");
            dsm_client_notify(client_flash, DSM_FLASH_I2C_ERROR_NO);
            cam_err("[I/DSM] %s flash light i2c fail, rc_ocp = %d, rc_ovp = %d", __func__,rc_ocp,rc_ovp);
        }
        return -EINVAL;
    }

    if (ocp_reg_val & AW3642_OVER_TEMP_PROTECT) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash temperature is too hot! FlagReg[0x%x]\n", ocp_reg_val);
            dsm_client_notify(client_flash, DSM_FLASH_HOT_DIE_ERROR_NO);
            cam_warn("[I/DSM] %s flash temperature is too hot! FlagReg[0x%x]", __func__, ocp_reg_val);
        }
    }

    if (ocp_reg_val & (AW3642_LED_SHORT | AW3642_VOUT_SHORT)) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "LED or VOUT short! FlagReg[0x%x]\n", ocp_reg_val);
            dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
            cam_warn("[I/DSM] %s LED or VOUT short! FlagReg[0x%x]", __func__, ocp_reg_val);
        }
    }

    if (ovp_reg_val & AW3642_OVER_VOLTAGE_PROTECT) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash OVP! FlagReg[0x%x]\n", ovp_reg_val);
            dsm_client_notify(client_flash, DSM_FLASH_OPEN_SHOTR_ERROR_NO);
            cam_warn("[I/DSM] %s flash OVP! FlagReg[0x%x]", __func__, ovp_reg_val);
        }
    }

    if (ocp_reg_val & AW3642_UNDER_VOLTAGE_LOCKOUT) {
        if(!dsm_client_ocuppy(client_flash)) {
            dsm_client_record(client_flash, "flash UVLO! FlagReg[0x%x]\n", ocp_reg_val);
            dsm_client_notify(client_flash, DSM_FLASH_UNDER_VOLTAGE_LOCKOUT_ERROR_NO);
            cam_warn("[I/DSM] %s flash UVLO! FlagReg[0x%x]", __func__, ocp_reg_val);
        }
    }

    return 0;
}

static int hw_aw3642_find_match_flash_current(int cur_flash)
{
    int cur_level = 0;
    int i = 0;

    cam_info("%s enter cur_flash %d.\n", __func__, cur_flash);
    if(cur_flash <= 0){
        cam_err("%s current set is error", __func__);
        return -EINVAL;
    }

    if(cur_flash >= AW3642_FLASH_MAX_CUR){
        cam_warn("%s current set is %d", __func__, cur_flash);
        return AW3642_FLASH_DEFAULT_CUR_LEV;
    }

    for(i=0; i < FLASH_LED_MAX; i++){
        if(cur_flash <= flash_arry[i]){
            cam_info("%s  i %d.\n", __func__, i);
            break;
        }
    }

    if( i==0 ){
          cur_level = i;
    }
    else if(i == FLASH_LED_MAX){
        if((cur_flash - flash_arry[i-2]) < (flash_arry[i-1] -cur_flash))/*calculate current near value*/
              cur_level = i-2;/*near smaller value*/
        else
              cur_level = i-1;/*near larger value*/
    }
    else{
        if((cur_flash - flash_arry[i-1]) < (flash_arry[i] -cur_flash))/*calculate current near value*/
              cur_level = i-1;/*near smaller value*/
        else
              cur_level = i;/*near larger value*/
    }

    return cur_level;
}

static int hw_aw3642_find_match_torch_current(int cur_torch)
{
    int cur_level = 0;
    int i = 0;

    cam_info("%s enter cur_torch %d.\n", __func__, cur_torch);
    if(cur_torch <= 0){
        cam_err("%s current set is error", __func__);
        return -EINVAL;
    }

    if(cur_torch > AW3642_TORCH_MAX_CUR){
        cam_warn("%s current set is %d", __func__, cur_torch);
        return AW3642_TORCH_MAX_LEV;
    }

    for(i=0; i < TORCH_LED_MAX; i ++){
       if(cur_torch <= torch_arry[i]){
            cam_info("%s  i %d.\n", __func__, i);
            break;
       }
    }

    if( i==0 ){
          cur_level = i;
    }
    else if(i == TORCH_LED_MAX){
        if((cur_torch - torch_arry[i-2]) < (torch_arry[i-1] -cur_torch))/*calculate current near value*/
              cur_level = i-2;/*near smaller value*/
        else
              cur_level = i-1;/*near larger value*/
   }
    else{
        if((cur_torch - torch_arry[i-1]) < (torch_arry[i] -cur_torch))/*calculate current near value*/
              cur_level = i-1;/*near smaller value*/
        else
              cur_level = i;/*near larger value*/
    }

    return cur_level;
}

static int hw_aw3642_init(struct hw_flash_ctrl_t *flash_ctrl)
{

    cam_debug("%s enter.\n", __func__);
    if (NULL == flash_ctrl) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }
    return 0;
}

static int hw_aw3642_exit(struct hw_flash_ctrl_t *flash_ctrl)
{

    cam_debug("%s enter.\n", __func__);
    if (NULL == flash_ctrl || NULL == flash_ctrl->func_tbl || NULL == flash_ctrl->func_tbl->flash_off) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    flash_ctrl->func_tbl->flash_off(flash_ctrl);
    return 0;
}

static int hw_aw3642_flash_mode(struct hw_flash_ctrl_t *flash_ctrl,
    int data)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_aw3642_private_data_t *pdata;
    unsigned char val;
    int current_level = 0;
    int rc = 0;

    cam_info("%s data=%d.\n", __func__, data);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = flash_ctrl->pdata;
    if (FLASH_LED_LEVEL_INVALID == pdata->flash_current)
    {
        current_level = AW3642_FLASH_DEFAULT_CUR_LEV;
    }
    else
    {
        current_level = hw_aw3642_find_match_flash_current(data);
        if(current_level < 0){
             current_level = AW3642_FLASH_DEFAULT_CUR_LEV;
        }
    }

    rc = hw_aw3642_clear_error_and_notify_dmd(flash_ctrl);
    if(rc < 0)
    {
            cam_err("%s flash light clear error", __func__);
            return -EINVAL;
    }

    val = 0x80 | (current_level & 0x3f);//flash brightness register 0x03, bit7:1,bit6:0,bit[0-5]:current level
    cam_info("%s led flash current val=0x%x, current_level =%d.\n", __func__, val, current_level);

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_FLASH_CURRENT_CONTROL, val) < 0);

    /*flash time-out time is 600ms(default), flash ramp-time is 1ms(fixed)*/
    if (flash_ctrl->flash_mask_enable) {
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_FLASH|TX_PIN_EN|LED_EN) < 0);
    } else {
        loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_FLASH|TX_PIN_DISABLE|LED_EN) < 0);
    }

    return 0;
}

static int hw_aw3642_torch_mode(struct hw_flash_ctrl_t *flash_ctrl,
    int data)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_aw3642_private_data_t *pdata;
    unsigned char val;
    int current_level = 0;
    int rc = 0;

    cam_info("%s data=%d.\n", __func__, data);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = (struct hw_aw3642_private_data_t *)flash_ctrl->pdata;
    if (FLASH_LED_LEVEL_INVALID == pdata->torch_current)
    {
        current_level = AW3642_TORCH_DEFAULT_CUR_LEV;
    }
    else
    {
        current_level = hw_aw3642_find_match_torch_current(data);
        if(current_level < 0){
              current_level = AW3642_TORCH_DEFAULT_CUR_LEV;
        }
    }

    rc = hw_aw3642_clear_error_and_notify_dmd(flash_ctrl);
    if(rc < 0)
    {
              cam_err("%s flash light clear error", __func__);
              return -EINVAL;
    }

    val = 0x80 | (current_level & 0x3f);//torch brightness register 0x05, bit7:1,bit6:0,bit[0-5]:current level
    cam_info("%s the led torch current val=0x%x, current_level=%d.\n", __func__, val, current_level);

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_TORCH_CURRENT_CONTROL, val) < 0);
    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_ENABLE, MODE_TORCH|LED_EN) < 0);

    return 0;
}

static int hw_aw3642_on(struct hw_flash_ctrl_t *flash_ctrl, void *data)
{
    struct hw_flash_cfg_data *cdata = (struct hw_flash_cfg_data *)data;
    int rc=-1;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == cdata)) {
        cam_err("%s flash_ctrl or cdata is NULL.", __func__);
        return -EINVAL;
    }

    cam_info("%s mode=%d, level=%d.\n", __func__, cdata->mode, cdata->data);
    mutex_lock(flash_ctrl->hw_flash_mutex);
    if (FLASH_MODE == cdata->mode) {
        rc = hw_aw3642_flash_mode(flash_ctrl, cdata->data);
    } else {
        rc = hw_aw3642_torch_mode(flash_ctrl, cdata->data);
    }
    flash_ctrl->state.mode = cdata->mode;
    flash_ctrl->state.data = cdata->data;
    mutex_unlock(flash_ctrl->hw_flash_mutex);

    return rc;
}

static int hw_aw3642_off(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;

    cam_info("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->flash_i2c_client)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    mutex_lock(flash_ctrl->hw_flash_mutex);
    flash_ctrl->state.mode = STANDBY_MODE;
    flash_ctrl->state.data = 0;
    i2c_client = flash_ctrl->flash_i2c_client;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;

    if(i2c_func->i2c_write(i2c_client, REG_ENABLE, TX_PIN_EN|MODE_STANDBY) < 0){
        cam_err("%s %d", __func__, __LINE__);
    }
    mutex_unlock(flash_ctrl->hw_flash_mutex);

    return 0;
}

static int hw_aw3642_get_dt_data(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_aw3642_private_data_t *pdata;
    struct device_node *of_node;
    int rc = -1;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->dev)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return rc;
    }

    pdata = (struct hw_aw3642_private_data_t *)flash_ctrl->pdata;
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

extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);
#ifdef CAMERA_FLASH_FACTORY_TEST
static ssize_t hw_aw3642_lightness_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }
    int rc=0;

    snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE-1, "mode=%d, data=%d.\n",
    hw_aw3642_ctrl.state.mode, hw_aw3642_ctrl.state.data);
    rc = strlen(buf)+1;
    return rc;
}

static int hw_aw3642_param_check(char *buf, unsigned long *param,
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
            if (strlen(token) < 2) { //add for check token[1] has valid element
                base = 10; //Decimal
            } else {
                if ((token[1] == 'x') || (token[1] == 'X')) {
                    base = 16;//Hex
                } else {
                    base = 10;//Decimal
                }
            }

            if (strict_strtoul(token, base, &param[cnt]) != 0) {
                return -EINVAL;
            }

            token = strsep(&buf, " ");
        }
        else
        {
            cam_err("%s the %d param is null\n", __func__, num_of_par);
            return -EINVAL;
        }
    }
    return 0;
}

static ssize_t hw_aw3642_lightness_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }

    struct hw_flash_cfg_data cdata = {0};
    unsigned long param[PARAM_LEN]={0};
    int rc=0;

    rc = hw_aw3642_param_check((char *)buf, param, PARAM_LEN);
    if (rc < 0) {
        cam_err("%s failed to check param.", __func__);
        return rc;
    }

    int flash_id = (int)param[0];
    cdata.mode = (int)param[1];
    cam_info("%s flash_id=%d,cdata.mode=%d.", __func__, flash_id, cdata.mode);
    if(1 != flash_id){//bit[0]- rear first flash light.bit[1]- rear sencond flash light.bit[2]- front flash light; dallas product using only rear first flash light
        cam_err("%s wrong flash_id=%d.", __func__,flash_id);
        return -EINVAL;
    }

    if (cdata.mode == STANDBY_MODE) {
        rc = hw_aw3642_off(&hw_aw3642_ctrl);
        if (rc < 0) {
            cam_err("%s aw3642 flash off error.", __func__);
            return rc;
        }
    } else if(cdata.mode == TORCH_MODE){
        cdata.data = AW3642_TORCH_MAX_RT_CUR;//hardware test requiring the max torch mode current
        cam_info("%s mode=%d, max_current=%d.", __func__, cdata.mode, cdata.data);

        rc = hw_aw3642_on(&hw_aw3642_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s aw3642 flash on error.", __func__);
            return rc;
        }
    } else {
        cam_err("%s scharger wrong mode=%d.", __func__,cdata.mode);
        return -EINVAL;
    }

    return count;
}
#endif

static ssize_t hw_aw3642_flash_mask_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    int rc=0;
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }

    snprintf_s(buf, MAX_ATTRIBUTE_BUFFER_SIZE, MAX_ATTRIBUTE_BUFFER_SIZE-1, "flash_mask_disabled=%d.\n", hw_aw3642_ctrl.flash_mask_enable);
    rc = strlen(buf)+1;
    return rc;
}

static ssize_t hw_aw3642_flash_mask_store(struct device *dev,
    struct device_attribute *attr, const char *buf, size_t count)
{
    if ((NULL == dev) || (NULL == attr) || (NULL == buf))
    {
        cam_err("%s invalid param.", __func__);
        return -EINVAL;
    }

    if ('0' == buf[0]) {
        hw_aw3642_ctrl.flash_mask_enable = false;
    } else {
        hw_aw3642_ctrl.flash_mask_enable = true;
    }
    cam_debug("%s flash_mask_enable=%d.", __func__,
            hw_aw3642_ctrl.flash_mask_enable);
    return count;
}

static void hw_aw3642_torch_brightness_set(struct led_classdev *cdev,
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
        rc = hw_aw3642_off(&hw_aw3642_ctrl);
        if (rc < 0) {
            cam_err("%s hw_aw3642_off error.", __func__);
            return;
        }
    } else {
        cdata.mode = TORCH_MODE;
        cdata.data = AW3642_TORCH_MAX_MMI_CUR;
        cam_info("%s current=%d.", __func__, cdata.data);
        rc = hw_aw3642_on(&hw_aw3642_ctrl, &cdata);
        if (rc < 0) {
            cam_err("%s hw_aw3642_on error.", __func__);
            return;
        }
    }
}

#ifdef CAMERA_FLASH_FACTORY_TEST
static struct device_attribute hw_aw3642_lightness =
    __ATTR(flash_lightness, 0660, hw_aw3642_lightness_show, hw_aw3642_lightness_store);
#endif

static struct device_attribute hw_aw3642_flash_mask =
    __ATTR(flash_mask, 0660, hw_aw3642_flash_mask_show, hw_aw3642_flash_mask_store);

static int hw_aw3642_register_attribute(struct hw_flash_ctrl_t *flash_ctrl, struct device *dev)
{
    int rc = 0;

    if ((NULL == flash_ctrl) || (NULL == dev) || (NULL == flash_ctrl->pdata)) {
        cam_err("%s flash_ctrl or dev is NULL.", __func__);
        return -EINVAL;
    }

    flash_ctrl->cdev_torch.name = "torch";
    flash_ctrl->cdev_torch.max_brightness
        = ((struct hw_aw3642_private_data_t *)(flash_ctrl->pdata))->torch_led_num;
    flash_ctrl->cdev_torch.brightness_set = hw_aw3642_torch_brightness_set;
    rc = led_classdev_register((struct device *)dev, &flash_ctrl->cdev_torch);
    if (rc < 0) {
        cam_err("%s failed to register torch classdev.", __func__);
        goto err_out;
    }
#ifdef CAMERA_FLASH_FACTORY_TEST
    rc = device_create_file(dev, &hw_aw3642_lightness);
    if (rc < 0) {
        cam_err("%s failed to creat lightness attribute.", __func__);
        goto err_create_lightness_file;
    }
#endif
    rc = device_create_file(dev, &hw_aw3642_flash_mask);
    if (rc < 0) {
        cam_err("%s failed to creat flash_mask attribute.", __func__);
        goto err_create_flash_mask_file;
    }
    return 0;
err_create_flash_mask_file:
#ifdef CAMERA_FLASH_FACTORY_TEST
    device_remove_file(dev, &hw_aw3642_lightness);
#endif
err_create_lightness_file:
    led_classdev_unregister(&flash_ctrl->cdev_torch);
err_out:
    return rc;
}//lint !e563

static int hw_aw3642_match(struct hw_flash_ctrl_t *flash_ctrl)
{
    struct hw_flash_i2c_client *i2c_client;
    struct hw_flash_i2c_fn_t *i2c_func;
    struct hw_aw3642_private_data_t *pdata;
    unsigned char id;

    cam_debug("%s enter.\n", __func__);
    if ((NULL == flash_ctrl) || (NULL == flash_ctrl->pdata) || (NULL == flash_ctrl->flash_i2c_client)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_read)
         || (NULL == flash_ctrl->flash_i2c_client->i2c_func_tbl->i2c_write)) {
        cam_err("%s flash_ctrl is NULL.", __func__);
        return -EINVAL;
    }

    i2c_client = flash_ctrl->flash_i2c_client;
    if ((NULL == i2c_client->client)) {
        cam_err("%s i2c client is NULL.", __func__);
        return -EINVAL;
    }
    /* change to AW3642 actual slave address 0x63*/
    i2c_client->client->addr = AW3642_SLAVE_ADDRESS;
    i2c_func = flash_ctrl->flash_i2c_client->i2c_func_tbl;
    pdata = (struct hw_aw3642_private_data_t *)flash_ctrl->pdata;

    loge_if_ret(i2c_func->i2c_read(i2c_client, REG_CHIP_ID, &id) < 0);
    cam_info("%s id=0x%x.\n", __func__, id);
    if (id != pdata->chipid) {
        cam_err("%s match error, id(0x%x) != 0x%x.", __func__, id, pdata->chipid);
        return -EINVAL;
    }

    loge_if_ret(i2c_func->i2c_write(i2c_client, REG_IVFM, UVLO_EN|IVFM_VOLTAGE|IVFM_EN) < 0);//enable UVLO, set voltage 3.2v, enable IVFM(mode:stop and hold mode)
#ifdef CAMERA_FLASH_FACTORY_TEST
    register_camerafs_attr(&hw_aw3642_lightness);
#endif
    return 0;
}

static int hw_aw3642_remove(struct i2c_client *client)
{
    cam_debug("%s enter.", __func__);
    if (NULL == client) {
        cam_err("%s invalid param", __func__);
        return -EINVAL;
    }

    hw_aw3642_ctrl.func_tbl->flash_exit(&hw_aw3642_ctrl);

    client->adapter = NULL;
    return 0;
}

static void hw_aw3642_shutdown(struct i2c_client *client)
{
    int rc = -1;
    if (NULL == client) {
        cam_err("%s invalid param", __func__);
        return;
    }
    rc = hw_aw3642_off(&hw_aw3642_ctrl);
    cam_info("%s aw3642 shut down at (%d)",__func__,__LINE__);
    if (rc < 0) {
        cam_err("%s aw3642 flash off error.", __func__);
    }
}

static const struct i2c_device_id hw_aw3642_id[] = {
    {"aw3642", (unsigned long)&hw_aw3642_ctrl},
    {}
};

static const struct of_device_id hw_aw3642_dt_match[] = {
    {.compatible = "huawei,aw3642"},
    {}
};
MODULE_DEVICE_TABLE(of, aw3642_dt_match);

static struct i2c_driver hw_aw3642_i2c_driver = {
    .probe = hw_flash_i2c_probe,
    .remove = hw_aw3642_remove,
    .shutdown = hw_aw3642_shutdown,
    .id_table = hw_aw3642_id,
    .driver = {
        .name = "hw_aw3642",
        .of_match_table = hw_aw3642_dt_match,
    },
};

static int __init hw_aw3642_module_init(void)
{
    cam_info("%s erter.\n", __func__);
    return i2c_add_driver(&hw_aw3642_i2c_driver);
}

static void __exit hw_aw3642_module_exit(void)
{
    cam_info("%s enter.", __func__);
    i2c_del_driver(&hw_aw3642_i2c_driver);
    return;
}

static struct hw_flash_i2c_client hw_aw3642_i2c_client;

static struct hw_flash_fn_t hw_aw3642_func_tbl = {
    .flash_config = hw_flash_config,
    .flash_init = hw_aw3642_init,
    .flash_exit = hw_aw3642_exit,
    .flash_on = hw_aw3642_on,
    .flash_off = hw_aw3642_off,
    .flash_match = hw_aw3642_match,
    .flash_get_dt_data = hw_aw3642_get_dt_data,
    .flash_register_attribute = hw_aw3642_register_attribute,
};

static struct hw_flash_ctrl_t hw_aw3642_ctrl = {
    .flash_i2c_client = &hw_aw3642_i2c_client,
    .func_tbl = &hw_aw3642_func_tbl,
    .hw_flash_mutex = &flash_mut_aw3642,
    .pdata = (void*)&hw_aw3642_pdata,
    .flash_mask_enable = false,
    .state = {
        .mode = STANDBY_MODE,
    },
};

module_init(hw_aw3642_module_init);
module_exit(hw_aw3642_module_exit);
MODULE_DESCRIPTION("AW3642 FLASH");
MODULE_LICENSE("GPL v2");
#pragma GCC diagnostic pop
