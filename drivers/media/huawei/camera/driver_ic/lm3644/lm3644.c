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
#include "../hwdriver_ic.h"
#include "cam_log.h"

//lint -save -e429

#define INVALID_GPIO        (999)
#define INVALID_I2C_INDEX   (-1)
#define INVALID_IC_POSITION (-1)
#define LM3644_GPIO_ENABLE  (1)
#define LM3644_GPIO_DISABLE (0)

#define DRIVER_IC_ENABLE_PIN (10)
#define BUCKBOOST_ENABLE_PIN (8)
#define DRV_IC1_NAME "drv_ic_lm3644_1"
#define DRV_IC2_NAME "drv_ic_lm3644_2"

#define INTF2DRIVERIC(i) container_of(i, driveric_t, intf)

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
    RESET = 1,
    LM3644_MAX_GPIOPIN,
}drv_ic_LM3644_pin_type_t;

typedef enum {
    FREED = 0,
    REQUESTED,
}drv_ic_LM3644_pin_status_t;

typedef enum{
    DISABLE = 0,
    ENABLE = 1,
}powertype_t;

typedef struct _drv_ic_LM3644_pin_t {
    unsigned int pin_id;
    drv_ic_LM3644_pin_status_t pin_status;
}drv_ic_LM3644_pin_t;

typedef struct _driver_ic_LM3644_private_data_t {
    drv_ic_LM3644_pin_t pin[LM3644_MAX_GPIOPIN];
}drv_ic_LM3644_private_data_t;

extern int snprintf_s(char* strDest, size_t destMax, size_t count, const char* format, ...);
static int LM3644_get_dt_data(const hwdriveric_intf_t *intf, struct device_node *dev_node)
{
    driveric_t *drv_ic = NULL;
    drv_ic_LM3644_private_data_t *pdata = NULL;
    unsigned int pin_tmp[LM3644_MAX_GPIOPIN] = {0};
    int rc = 0;
    int i  = 0;
    unsigned int position, i2c_index;

    RETURN_ERROR_ON_NULL(intf);
    RETURN_ERROR_ON_NULL(dev_node);

    drv_ic = INTF2DRIVERIC(intf);
    RETURN_ERROR_ON_NULL(drv_ic);

    pdata = (drv_ic_LM3644_private_data_t *)kzalloc(sizeof(drv_ic_LM3644_private_data_t),GFP_KERNEL);
    if (pdata) {
        drv_ic->pdata = (void*)pdata;
    } else {
        cam_err("fail to alloc drv_ic_LM3644_private_data_t,out of memory");
        return -1;
    }

    rc = of_property_read_u32(dev_node, "huawei,position", (u32 *)&position);
    if (rc < 0) {
        cam_err("%s get dt position failed", __func__);
        drv_ic->position = INVALID_IC_POSITION;
        goto get_dt_error_handler;
    } else {
        drv_ic->position = position;
        cam_info("%s position(%u)", __func__, drv_ic->position);
    }

    rc = of_property_read_u32(dev_node, "huawei,i2c_index", (u32 *)&i2c_index);
    if (rc < 0) {
        cam_err("%s get dt i2c_index failed", __func__);
        drv_ic->i2c_index = INVALID_I2C_INDEX;
        goto get_dt_error_handler;
    } else {
        drv_ic->i2c_index = i2c_index;
        cam_info("%s i2c_index(%u)", __func__, drv_ic->i2c_index);
    }

    rc = of_property_read_u32_array(dev_node, "huawei,gpio_pin",
                pin_tmp, LM3644_MAX_GPIOPIN);
    if (rc < 0) {
        cam_err("%s get dt gpio-pin failed", __func__);
        for (i = 0; i < LM3644_MAX_GPIOPIN; i++) {
            pdata->pin[i].pin_id = INVALID_GPIO;
            pdata->pin[i].pin_status = FREED;
        }
        goto get_dt_error_handler;
    } else {
        for (i = 0; i < LM3644_MAX_GPIOPIN; i++) {
            pdata->pin[i].pin_id = pin_tmp[i];
            pdata->pin[i].pin_status = FREED;
            cam_info("%s gpio-pin[%d] = %u.", __func__, i, pdata->pin[i].pin_id);
        }
    }

    return rc;
get_dt_error_handler:
    kfree(drv_ic->pdata);
    drv_ic->pdata= NULL;
    return rc;
}


static int drv_ic_LM3644_set_pin(drv_ic_LM3644_private_data_t *pdata, int state)
{
    int rc = 0;
    int i=0;

    RETURN_ERROR_ON_NULL(pdata);

    for(i=0;i<LM3644_MAX_GPIOPIN;i++)
    {
        cam_debug("%s pin_id=%d, state=%d.", __func__, pdata->pin[i].pin_id, state);

        if(INVALID_GPIO == pdata->pin[i].pin_id)
        {
            cam_err("%s gpio pin is err rc=%d.", __func__, pdata->pin[i].pin_id);
            return -1;
        }
        rc = gpio_request(pdata->pin[i].pin_id, NULL);
        if (rc < 0) {
            cam_err("%s, gpio-request %d", __func__, pdata->pin[i].pin_id);
            return -1;
        }

        pdata->pin[i].pin_status = REQUESTED;
        rc = gpio_direction_output(pdata->pin[i].pin_id, state);
        if (rc < 0) {
            cam_err("%s gpio output is err rc=%d.", __func__, rc);
        }

        gpio_free(pdata->pin[i].pin_id);
    }

    return rc;
}


static int LM3644_init(const hwdriveric_intf_t *intf)
{
    driveric_t *drv_ic = NULL;
    int rc = 0;
    cam_info("%s init enter", __func__);

    RETURN_ERROR_ON_NULL(intf);

    drv_ic = INTF2DRIVERIC(intf);
    RETURN_ERROR_ON_NULL(drv_ic);
    RETURN_ERROR_ON_NULL(drv_ic->pdata);

    cam_info("%s init success", __func__);
    return rc;
}

//static void LM3644_notify_error(uint32_t id);

static int LM3644_power_on(const hwdriveric_intf_t* intf)
{
    driveric_t *drv_ic = NULL;
    drv_ic_LM3644_private_data_t *pdata = NULL;
    int rc = 0;

    cam_info("%s power on enter", __func__);
    RETURN_ERROR_ON_NULL(intf);

    drv_ic = INTF2DRIVERIC(intf);
    RETURN_ERROR_ON_NULL(drv_ic);
    RETURN_ERROR_ON_NULL(drv_ic->pdata);

    pdata = (drv_ic_LM3644_private_data_t *)drv_ic->pdata;

    rc = drv_ic_LM3644_set_pin(pdata, ENABLE);

    cam_info("%s power on end,rc=%d", __func__,rc);
    return rc;
}

static int LM3644_power_off(const hwdriveric_intf_t* intf)
{
    driveric_t *drv_ic = NULL;
    drv_ic_LM3644_private_data_t *pdata = NULL;
    int rc = 0;

    cam_info("%s power off enter", __func__);
    RETURN_ERROR_ON_NULL(intf);

    drv_ic = INTF2DRIVERIC(intf);
    RETURN_ERROR_ON_NULL(drv_ic);
    RETURN_ERROR_ON_NULL(drv_ic->pdata);

    pdata = (drv_ic_LM3644_private_data_t *)drv_ic->pdata;

    rc = drv_ic_LM3644_set_pin(pdata, DISABLE);

    cam_info("%s power off end", __func__);
    return rc;
}

char const* LM3644_get_name(const hwdriveric_intf_t* intf)
{
    driveric_t* LM3644 = NULL;
    char* name = NULL;
    LM3644 = INTF2DRIVERIC(intf);

    if (NULL == LM3644) {
        cam_err("%s. LM3644 is NULL.", __func__);
        return NULL;
    }
    if (LM3644->position == 0) {
        name = DRV_IC1_NAME;
    }
    else
    {
        name = DRV_IC2_NAME;
    }
    cam_info("%s position = %d get name = %s", __func__,LM3644->position,name);
    return name;
}
/*
void LM3644_notify_error( uint32_t id)
{
    hwdriveric_event_t LM3644_ev;
    LM3644_ev.kind = HWDRIVERIC_INFO_ERROR;
    LM3644_ev.data.error.id = id;
    cam_info("%s id = %x",__func__,id);
    hwdriveric_intf_notify_error(s_LM3644.notify, &LM3644_ev);
}
*/
static hwdriveric_vtbl_t
s_vtbl_LM3644 =
{
    .get_name = LM3644_get_name,
    .power_on = LM3644_power_on,
    .power_off = LM3644_power_off,
    .driveric_get_dt_data = LM3644_get_dt_data,
    .init = LM3644_init,
};

static driveric_t
s_LM3644 =
{
    .name  = "drv_ic_LM3644",
    .intf  = { .vtbl = &s_vtbl_LM3644, },
    .pdata = NULL,
};

static const struct of_device_id
s_LM3644_dt_match[] =
{
    {
        .compatible = "huawei,drv_ic_LM3644_1",
        .data = &s_LM3644.intf,
    },
    {
        .compatible = "huawei,drv_ic_LM3644_2",
        .data = &s_LM3644.intf,
    },
    {
    },
};

MODULE_DEVICE_TABLE(of, s_driveric_dt_match);

static struct platform_driver
s_LM3644_driver =
{
    .driver =
    {
        .name = "huawei,drv_ic_LM3644",
        .owner = THIS_MODULE,
        .of_match_table = s_LM3644_dt_match,
    },
};

static int32_t
LM3644_platform_probe(
        struct platform_device* pdev)
{
    driveric_t *drv_ic = NULL;
    char* name1 = "drv_ic_LM3644_1";
    char* name2 = "drv_ic_LM3644_2";
    char* name = NULL;
    unsigned int position = 0;
    int rc = 0;
    cam_notice("%s enter", __func__);

    rc = of_property_read_u32(pdev->dev.of_node, "huawei,position", (u32 *)&position);
    if (rc !=0) {
        printk("%s fail to read huawei,position",__func__);
        return rc;
    }
    printk("%s to read huawei,position %d ",__func__,position);

    if (position == 0) {
        name = name1;
    } else {
        name = name2;
    }
    drv_ic = (driveric_t*)kzalloc(sizeof(driveric_t),GFP_KERNEL);
    if (drv_ic != NULL) {
        drv_ic->position = position;
        snprintf_s(&(drv_ic->name[0]),HWDRIVERIC_NAME_SIZE, HWDRIVERIC_NAME_SIZE-1,"%s",name);
        drv_ic->intf.vtbl = &s_vtbl_LM3644;
        if (hwdriveric_register(pdev, &(drv_ic->intf), &(drv_ic->notify))!=0) {
            cam_err("fail to do hwdriveric_register");
            kfree(drv_ic);
            return -1;
        }
    } else {
        cam_err("fail to alloc driveric_t,out of memory");
        return -1;
    }
    return rc;
}

static int __init
LM3644_init_module(void)
{
    int ret = 0;

    cam_notice("%s enter", __func__);

    ret = platform_driver_probe(&s_LM3644_driver,
        LM3644_platform_probe);

    return ret;
}

static void __exit
LM3644_exit_module(void)
{
    platform_driver_unregister(&s_LM3644_driver);
    //hwdriveric_unregister(&s_LM3644.intf);
}

module_init(LM3644_init_module);
module_exit(LM3644_exit_module);
MODULE_DESCRIPTION("LM3644");
MODULE_LICENSE("GPL v2");

