/*
 *  stmvl53l0_module-i2c.c - Linux kernel modules for STM VL53L0 FlightSense TOF
 *							sensor
 *
 *  Copyright (C) 2016 STMicroelectronics Imaging Division.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/media.h>
#include <linux/module.h>
#include <media/huawei/camera.h>
#include <hwcam_intf.h> 
#include <linux/mutex.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/time.h>

#include "hw_laser_i2c.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include "../laser_common.h"
#include <media/huawei/laser_cfg.h>
//lint -save -e715 -e578 -e644 -e530 -e774 -e383 -e749 -e838 -e421
//lint -save -e826 -e715 -e785 -e64 -e528 -e551 -e753 -e753 -esym(528,*) -esym(753,*) -esym(551,*)
#include "gp2ap01vt.h"
#include "stmvl53l0.h"

enum{
    XSDN,
    INTR,
    GPIO_MAX,
};

enum{
    LASER_IOVDD,
    LASER_AVDD,
    VDD_MAX,
};

enum{
    LASER_UNUSED = -1,
    LASER_ST = 0,
    LASER_SHARP = 1,
};
static int  laser_type = LASER_UNUSED;
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
//#define DBG_EN_STUB
extern int laser_remove(struct i2c_client *client);
extern int laser_probe(struct i2c_client *client, const struct i2c_device_id *id);

static int laser_i2c_read(struct i2c_client *client, u8 reg, u8* data)
{
    int err = -1;
    struct i2c_msg  i2cMsg[2];
    if (client == NULL) {
    	return -ENODEV;
    }

    i2cMsg[0].addr = client->addr;
    i2cMsg[0].flags = 0;
    i2cMsg[0].len = 1;
    i2cMsg[0].buf = &reg;

    i2cMsg[1].addr = client->addr;
    i2cMsg[1].flags = I2C_M_RD;
    i2cMsg[1].len = 1;
    i2cMsg[1].buf = data;

    err = i2c_transfer(client->adapter, i2cMsg, 2);
    if (err < 0) {
    	laser_err("i2c transfer error(%d)", err);
    }
    return err;
}

static void put_intr(struct i2c_data *i2c_data)
{
    if(NULL == i2c_data)
    {
        laser_err("i2c_data is NULL");
        return;
    }
    if (i2c_data->io_flag.intr_owned) {
        if (i2c_data->io_flag.intr_started) {
            free_irq((unsigned int)i2c_data->irq, i2c_data);
            i2c_data->io_flag.intr_started = 0;
        }
        laser_info("release intr_gpio %d", i2c_data->intr_gpio);
        gpio_free((unsigned int)i2c_data->intr_gpio);
        i2c_data->io_flag.intr_owned = 0;
    }
    i2c_data->intr_gpio = 0;
    return;
}

static void put_xsdn(struct i2c_data *i2c_data)
{
    if(NULL == i2c_data)
    {
        laser_err("i2c_data is NULL");
        return;
    }
    if (i2c_data->io_flag.xsdn_owned) {
        laser_info("release xsdn_gpio %d", i2c_data->xsdn_gpio);
        gpio_free(i2c_data->xsdn_gpio);
        i2c_data->io_flag.xsdn_owned = 0;
        i2c_data->xsdn_gpio = 0;
    }
    i2c_data->xsdn_gpio = 0;
    return;
}

static int get_xsdn(struct device *dev, struct i2c_data *i2c_data)
{
    int rc = 0;

    if(NULL == i2c_data)
    {
        laser_err("i2c_data is NULL");
        return -EINVAL;
    }
    i2c_data->io_flag.xsdn_owned = 0;
    if (i2c_data->xsdn_gpio == 0) {
        laser_err("reset gpio is required");
        rc = -ENODEV;
        goto no_gpio;
    }

    laser_info("request xsdn_gpio %d", i2c_data->xsdn_gpio);
    rc = gpio_request(i2c_data->xsdn_gpio, "vl53l0_xsdn");
    if (rc) {
        laser_err("fail to acquire xsdn %d", rc);
        goto request_failed;
    }

    rc = gpio_direction_output(i2c_data->xsdn_gpio, 0);
    if (rc) {
        laser_err("fail to configure xsdn as output %d", rc);
        goto direction_failed;
    }
    i2c_data->io_flag.xsdn_owned = 1;

    return rc;

direction_failed:
    gpio_free(i2c_data->xsdn_gpio);

request_failed:
no_gpio:
    return rc;
}

static int get_intr(struct device *dev, struct i2c_data *i2c_data)
{
    int rc = 0;
    if(NULL == i2c_data)
    {
        laser_err("i2c_data is NULL");
        return -EINVAL;
    }
    i2c_data->io_flag.intr_owned = 0;
    if (i2c_data->intr_gpio == 0) {
        laser_err("no interrupt gpio");
        goto no_gpio;
    }

    laser_info("request intr_gpio %d", i2c_data->intr_gpio);
    rc = gpio_request((unsigned int)(i2c_data->intr_gpio), "vl53l0_intr");
    if (rc) {
        laser_err("fail to acquire intr %d", rc);
        goto request_failed;
    }

    rc = gpio_direction_input((unsigned int)(i2c_data->intr_gpio));
    if (rc) {
        laser_err("fail to configure intr as input %d", rc);
        goto direction_failed;
    }

    i2c_data->irq = gpio_to_irq((unsigned int)(i2c_data->intr_gpio));
    if (i2c_data->irq < 0) {
        laser_err("fail to map GPIO: %d to interrupt:%d\n",
                i2c_data->intr_gpio, i2c_data->irq);
        goto irq_failed;
    }
    i2c_data->io_flag.intr_owned = 1;
    return rc;
irq_failed:
direction_failed:
    gpio_free((unsigned int)(i2c_data->intr_gpio));
request_failed:
no_gpio:
    return rc;
}

static int laser_power_on_off(struct i2c_data *i2cdata, int is_on)
{
    int rc = 0;
    if(NULL != i2cdata) {
        laser_info("laser power, on_off=%d\n", is_on);
        rc = gpio_direction_output(i2cdata->xsdn_gpio, is_on);
        if (rc) {
            laser_err("fail to power control laser %d\n", rc);
        }
        msleep(POWER_SLEEP_TIME);
    }else {
        laser_err("param is NULL");
        rc = -EINVAL;
    }
    return rc;
}

static int laser_parse_tree(struct device *dev, struct i2c_data *i2c_data)
{
    struct device_node *of_node = NULL;
    int rc = 0;
    char *gpio_tag = NULL;
    const char *gpio_ctrl_types[GPIO_MAX] = {"xsdn","intr"};
    int gpio[GPIO_MAX];
    int index = 0;
    int i = 0;
    int gpio_num = 0;
    if(dev == NULL) {
        return -EFAULT;
    }
    of_node = dev->of_node;
    if(of_node == NULL) {
        laser_err("%s of node get fail", __func__);
        return -EINVAL;
    }

    laser_info("enter %s", __func__);
    i2c_data->ldo_avdd.supply = "avdd";
    rc = devm_regulator_bulk_get(dev, 1, &(i2c_data->ldo_avdd));
    if (rc < 0) {
        laser_err("%s failed %d", __func__, __LINE__);
        i2c_data->ldo_avdd.consumer = NULL; /* always on */
    }

    gpio_num = of_gpio_count(of_node);
    if(gpio_num < 0 ) {
        laser_err("%s failed %d, ret is %d", __func__, __LINE__, gpio_num);
        goto fail;
    }
    laser_info("laser gpio num = %d", gpio_num);
    for(index = 0;index < gpio_num;index++)
    {
        rc = of_property_read_string_index(of_node, "huawei,gpio-ctrl-types", index, (const char **)&gpio_tag);
        if(rc < 0) {
        laser_err("%s failed %d", __func__, __LINE__);
        goto fail;
        }
        for(i = 0; i < GPIO_MAX; i++)
        {
            if(!strcmp(gpio_ctrl_types[i], gpio_tag))
            gpio[index] = of_get_gpio(of_node, index);
        }
        laser_info("gpio ctrl types: %s gpio = %d", gpio_tag, gpio[index]);
    }

    i2c_data->pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR(i2c_data->pinctrl)) {
        laser_err("could not get pinctrl");
        goto fail;
    }
    i2c_data->pins_default = pinctrl_lookup_state(i2c_data->pinctrl, PINCTRL_STATE_DEFAULT);
    if (IS_ERR(i2c_data->pins_default))
        laser_err("could not get default pinstate");
    i2c_data->pins_idle = pinctrl_lookup_state(i2c_data->pinctrl, PINCTRL_STATE_IDLE);
    if (IS_ERR(i2c_data->pins_idle))
        laser_err("could not get idle pinstate");
   
    i2c_data->xsdn_gpio = gpio[XSDN];
    i2c_data->intr_gpio = gpio[INTR];
    rc = get_xsdn(dev, i2c_data);
    if (rc)
        goto no_xsdn;
    rc = get_intr(dev, i2c_data);
    if (rc)
        goto no_intr;
    return rc;
fail:
    laser_err("%s can not read laser info exit.", __func__);
    return rc;
no_intr:
    put_intr(i2c_data);
no_xsdn:
    put_xsdn(i2c_data);
return rc;
}

int laser_match_id(struct i2c_data * i2cdata)
{
    int rc = 0;
    u8 chipid = 0;

    if(NULL == i2cdata)
        return -EINVAL;

    rc = laser_i2c_read(i2cdata->client, GP2AP01_CHIP_ID, &chipid); 
    if(rc < 0){
        laser_err("%s i2c fail", __func__);
        return rc;
    }
    laser_info("laser matchid = 0x%x", chipid);
    if (chipid == 0x0F) {        
        //strncpy_s(i2cdata->chip_name, CHIP_NAME_LEN-1, laser_sharp_name, strlen(laser_sharp_name));
        laser_type = LASER_SHARP;
    } else {
        rc = laser_i2c_read(i2cdata->client, VL53L0_CHIP_ID, &chipid);
        laser_info("laser matchid = 0x%x", chipid);
        if (chipid == 0xEE) {
            //strncpy_s(i2cdata->chip_name, CHIP_NAME_LEN-1, laser_st_name, strlen(laser_st_name));       
            laser_type = LASER_ST;
        }else{
            laser_type = LASER_UNUSED;
            return -1;
        }
    }
    #ifdef CONFIG_HUAWEI_HW_DEV_DCT
        set_hw_dev_flag(DEV_I2C_LASER);
    #endif
    return 0; 
}

int stmvl53l0_laser_data_init(struct i2c_client *client,
                        const struct i2c_device_id *id)
{
    int rc = 0;
    struct i2c_data *i2c_object = NULL;
    hw_laser_ctrl_t *ctrl = NULL;
    if(client == NULL || id == NULL)
            return -EINVAL;

    ctrl = (hw_laser_ctrl_t*)id->driver_data;
    laser_info("enter %s", __func__);
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE)) {
        rc = -EIO;
        return rc;
    }
    i2c_object = kzalloc(sizeof(struct i2c_data), GFP_KERNEL);
    if (!i2c_object) {
        rc = -ENOMEM;
        return rc;
    }
    i2c_object->client = client;
    i2c_object->ctrl = ctrl;
	/* setup regulator */
    rc = laser_parse_tree(&client->dev, i2c_object);
    if (rc) {
        laser_err("vl53l0 parse device tree error");
        goto free_client;
    }

    if (i2c_object->ldo_avdd.consumer != NULL)
    {
        rc = regulator_set_voltage(i2c_object->ldo_avdd.consumer, LDO_AVDD_3P3V, LDO_AVDD_3P3V);
        if(rc < 0){
             laser_err("set ldo_avdd error %d", rc);
        }
        rc =  regulator_bulk_enable(1, &i2c_object->ldo_avdd);
        if (rc) {
            laser_err("failed to enable ldo_avdd %d\n", rc);
        }
    }
    msleep(1);
    rc = laser_power_on_off(i2c_object, 1);
    if(rc) {
        laser_err("laser power up fail");
        goto free_client;
    }
    rc = laser_match_id(i2c_object);
    if(rc) {
        laser_err("laser match id fail");
        goto free_client;
    }

    if (laser_type == LASER_SHARP) {
        rc = gp2ap01vt00f_data_init((void*)&(ctrl->data), i2c_object);
        if(rc){
            laser_err("gp2ap01vt00f_data_init fail");
            goto free_client;
        }
    }else if(laser_type == LASER_ST) {
        rc = stmvl53l0_data_init((void*)&(ctrl->data), i2c_object);
        if(rc){
            laser_err("stmvl53l0_data_init fail");
            goto free_client;
        }
    }else{
        laser_err("laser_detect fail");
        ctrl->data = NULL;
        goto free_client;
    }

    rc = laser_power_on_off(i2c_object, 0);
    i2c_object->power_up = 0;
    laser_info("end\n");
    return rc;

free_client:
    laser_power_on_off(i2c_object, 0);
    i2c_object->power_up = 0;
    laser_info("end\n");
    put_intr(i2c_object);
    put_xsdn(i2c_object);
    kfree(i2c_object);
    return rc;
}
EXPORT_SYMBOL(stmvl53l0_laser_data_init);

long stmvl53l0_ioctl(void *hw_data, unsigned int cmd, void  *p)
{
    int rc = 0;
    if (hw_data == NULL)
        return -EINVAL;

    if (laser_type == LASER_SHARP) {
        rc = gp2ap01vt00f_laser_ioctl(hw_data, cmd, p);
    }else if(laser_type == LASER_ST) {
        rc = stmvl53l0_laser_ioctl(hw_data, cmd, p);
    }else{
        laser_err("hw_laser_ioctl fail");
        return -EINVAL;
    }
    return rc;
}
EXPORT_SYMBOL(stmvl53l0_ioctl);

int stmvl53l0_data_remove(struct i2c_client *client)
{
    int rc = 0;
    if(client == NULL)
        return -EINVAL;

    if (laser_type == LASER_SHARP) {
        rc = gp2ap01vt00f_laser_remove(client);
    }else if(laser_type == LASER_ST) {
        rc = stmvl53l0_laser_remove(client);
    }else{
        laser_err("laser type is error\n");
        return -EINVAL;
    }
    return rc;
}
EXPORT_SYMBOL(stmvl53l0_data_remove);



//lint -restore
