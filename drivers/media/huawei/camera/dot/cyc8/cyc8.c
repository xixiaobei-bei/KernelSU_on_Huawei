/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
//lint -save -e529 -e542 -e429 -e501 -e527 -e559 -e568 -e570 -e563 -e593 -e613 -e668 -e679
#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/videodev2.h>
#include <media/huawei/camera.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-core.h>

#include "hwcam_intf.h"
#include "../hwdot.h"
#include "cam_log.h"
#include "../../platform/sensor_commom.h"


#define INVALID_GPIO        (999)
#define INVALID_I2C_INDEX   (-1)
#define CYC8_GPIO_ENABLE    (1)
#define CYC8_GPIO_DISABLE   (0)

#define INTF2DOT(i) container_of(i, dot_t, intf)

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

typedef enum {
    POWER_ENABLE = 0,
    MAX_PIN,
}dot_cyc8_pin_type_t;

/*static const char* gpio_name[MAX_PIN]= {"dot-power-en"};*/ /*maybe use later*/

typedef enum {
    FREED = 0,
    REQUESTED,
}dot_cyc8_pin_status_t;

typedef struct _dot_cyc8_private_data_t {
    unsigned int pin[MAX_PIN];
}dot_cyc8_private_data_t;

static dot_t s_cyc8;
static dot_cyc8_private_data_t s_cyc8_pdata;

static int cyc8_get_dt_data(const hwdot_intf_t *intf, struct device_node *dev_node)
{
    dot_t *dot = NULL;
    dot_cyc8_private_data_t *pdata = NULL;
    unsigned int pin_tmp[MAX_PIN] = {0};
    int rc = 0;
    int i  = 0;
    unsigned int i2c_index;

    RETURN_ERROR_ON_NULL(intf);
    RETURN_ERROR_ON_NULL(dev_node);

    dot = INTF2DOT(intf);
    RETURN_ERROR_ON_NULL(dot);
    RETURN_ERROR_ON_NULL(dot->pdata);

    pdata = (dot_cyc8_private_data_t *)dot->pdata;

    rc = of_property_read_u32(dev_node, "huawei,i2c_index", (u32 *)&i2c_index);
    if (rc < 0) {
        cam_err("%s get dt i2c_index failed", __func__);
        dot->i2c_index = INVALID_I2C_INDEX;
        goto get_dt_error_handler;
    } else {
        dot->i2c_index = i2c_index;
        cam_info("%s i2c_index(%u)", __func__, dot->i2c_index);
    }

    rc = of_property_read_u32_array(dev_node, "huawei,gpio_pin",
                pin_tmp, MAX_PIN);
    if (rc < 0) {
        cam_err("%s get dt dot-pin failed", __func__);
        for (i = 0; i < MAX_PIN; i++) {
            pdata->pin[i] = INVALID_GPIO;
        }
        goto get_dt_error_handler;
    } else {
        for (i = 0; i < MAX_PIN; i++) {
            pdata->pin[i]= pin_tmp[i];
            cam_info("%s dot-pin[%d] = %u.", __func__, i, pdata->pin[i]);
        }
    }

    return rc;

get_dt_error_handler:

    return rc;
}



int cyc8_get_thermal(const hwdot_intf_t *i,void* data)
{
    int rc=0;
    dot_thermal_data *dotdata = (dot_thermal_data *)data;

    if(NULL == i || NULL == data)
    {
        cam_err("%s cyc8 null pointer", __func__);
        return -1;
    }

    rc = hw_sensor_get_thermal("dot",&(dotdata->data));
    if( -1 == rc){
        cam_err("%s cyc8 get thermal fail", __func__);
        return -1;}

    return rc;
}


static int cyc8_init(const hwdot_intf_t *intf)
{
    dot_t *dot = NULL;
    int rc = 0;

    RETURN_ERROR_ON_NULL(intf);
    dot = INTF2DOT(intf);
    RETURN_ERROR_ON_NULL(dot);
    RETURN_ERROR_ON_NULL(dot->pdata);

    return rc;
}

void cyc8_notify_error(uint32_t id);

int cyc8_power_on(const hwdot_intf_t* intf)
{
    dot_t *dot = NULL;
    dot_cyc8_private_data_t *pdata = NULL;
    int rc = 0;

    RETURN_ERROR_ON_NULL(intf);

    dot = INTF2DOT(intf);
    RETURN_ERROR_ON_NULL(dot);
    RETURN_ERROR_ON_NULL(dot->pdata);

    pdata = (dot_cyc8_private_data_t *)dot->pdata;

    cam_info("%s enter", __func__);
    /*rc = dot_cyc8_set_pin(pdata, POWER_ENABLE, CYC8_GPIO_ENABLE);*/ /*control by fpga*/

    return rc;
}

int cyc8_power_off(const hwdot_intf_t* intf)
{
    dot_t *dot = NULL;
    int rc = 0;

    RETURN_ERROR_ON_NULL(intf);

    dot = INTF2DOT(intf);
    RETURN_ERROR_ON_NULL(dot);
    RETURN_ERROR_ON_NULL(dot->pdata);

    cam_info("%s do nothings just return", __func__);
    return rc;
}

char const* cyc8_get_name(const hwdot_intf_t* intf)
{
    dot_t* cyc8 = NULL;

    if(NULL == intf) {
        cam_err("%s. intf is NULL.", __func__);
        return NULL;
    }

    cyc8 = INTF2DOT(intf);

    if (NULL == cyc8) {
        cam_err("%s. cyc8 is NULL.", __func__);
        return NULL;
    }

    return cyc8->name;
}

void cyc8_notify_error( uint32_t id)
{
    hwdot_event_t cyc8_ev;
    cyc8_ev.kind = HWDOT_INFO_ERROR;
    cyc8_ev.data.error.id = id;
    cam_info("%s id = %x",__func__,id);
    hwdot_intf_notify_error(s_cyc8.notify, &cyc8_ev);
}


static ssize_t cyc8_thermal_show(struct device *dev,
    struct device_attribute *attr,char *buf)
{
    int rc=0;
    int tmper =0;
    rc = hw_sensor_get_thermal("dot",&tmper);
    if( -1 == rc){
        cam_err("%s cyc8 get thermal fail", __func__);
        return -1;}

    rc = scnprintf(buf, PAGE_SIZE, "%d", tmper);

    return rc;
}
extern int register_camerafs_attr(struct device_attribute *attr);
static struct device_attribute dev_attr_cyc8thermal =
    __ATTR(cyc8thermal, 0440, cyc8_thermal_show, NULL);

 int cyc8_register_attribute(const hwdot_intf_t *intf, struct device * dev)
{
    int rc = 0;

    if (NULL == intf) {
        cam_err("%s intf is NULL.", __func__);
        return -1;
    }

    rc = device_create_file(dev, &dev_attr_cyc8thermal);
    if (rc < 0) {
        cam_err("%s failed to creat cyc8 thermal attribute.", __func__);
        goto err_out;
    }

    return 0;
err_out:
    return rc;
}

static hwdot_vtbl_t
s_vtbl_cyc8 =
{
    .get_name = cyc8_get_name,
    .power_on = cyc8_power_on,
    .power_off = cyc8_power_off,
    .dot_get_dt_data = cyc8_get_dt_data,
    .init = cyc8_init,
    .dot_register_attribute = cyc8_register_attribute,
    .get_thermal = cyc8_get_thermal,
};

static dot_t
s_cyc8 =
{
    .name = "cyc8",
    .intf = { .vtbl = &s_vtbl_cyc8, },
    .pdata = (void*)&s_cyc8_pdata,
};

static const struct of_device_id
s_cyc8_dt_match[] =
{
    {
        .compatible = "huawei,dot_cyc8",
        .data = &s_cyc8.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_dot_dt_match);

static struct platform_driver
s_cyc8_driver =
{
    .driver =
    {
        .name = "huawei,cyc8",
        .owner = THIS_MODULE,
        .of_match_table = s_cyc8_dt_match,
    },
};

static int32_t
cyc8_platform_probe(
        struct platform_device* pdev)
{
    cam_notice("%s enter", __func__);
    register_camerafs_attr(&dev_attr_cyc8thermal);
    return hwdot_register(pdev, &s_cyc8.intf, &s_cyc8.notify);
}

static int __init
cyc8_init_module(void)
{
    int ret = 0;

    cam_notice("%s enter", __func__);

    ret = platform_driver_probe(&s_cyc8_driver,
        cyc8_platform_probe);

    return ret;
}

static void __exit
cyc8_exit_module(void)
{
    platform_driver_unregister(&s_cyc8_driver);
    hwdot_unregister(&s_cyc8.intf);
}

module_init(cyc8_init_module);
module_exit(cyc8_exit_module);
MODULE_DESCRIPTION("cyc8");
MODULE_LICENSE("GPL v2");

