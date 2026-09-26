/*
 *  Hisilicon K3 SOC camera driver source file
 *
 *  Copyright (C) Huawei Technology Co., Ltd.
 *
 * Author:    wangyaofeng
 * Email:
 * Date:      2017-12-08
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
//lint -save -e574
#include "laser_module.h"

#define DRIVER_NAME "huawei,vl53l0"

extern int stmvl53l0_laser_data_init(struct i2c_client *client,
                        const struct i2c_device_id *id);
extern int stmvl53l0_data_remove(struct i2c_client *client);
extern long stmvl53l0_ioctl(void *hw_data, unsigned int cmd, void  *p);
extern int stmvl53l1_probe(struct i2c_client *client,
                        const struct i2c_device_id *id);
extern int stmvl53l1_remove(struct i2c_client *client);
extern long stmvl53l1_laser_ioctl(void *hw_data, unsigned int cmd, void  *p);

extern int laser_probe(struct i2c_client *client, const struct i2c_device_id *id);
extern int laser_remove(struct i2c_client *client);

static laser_module_intf_t laser_devices [] = {
    {"VL53L0", &stmvl53l0_laser_data_init, &stmvl53l0_data_remove, &stmvl53l0_ioctl},
    {"VL53L1", &stmvl53l1_probe, &stmvl53l1_remove, &stmvl53l1_laser_ioctl},
};

static int laser_index = -1;

static long hw_laser_ioctl(void *hw_data, unsigned int cmd, void  *p)
{
    int rc = 0;
    if (hw_data == NULL)
    {
        laser_err("parameter error");
        return -EINVAL;
    }

    if (laser_index >= 0 && laser_index < ARRAY_SIZE(laser_devices))
    {
        rc = laser_devices[laser_index].laser_ioctl(hw_data, cmd, p);
    }
    return rc;
}

static int laser_data_remove(struct i2c_client *client)
{
    int rc = 0;

    if(client == NULL)
    {
        laser_err("parameter error");
        return -EINVAL;
    }

    if (laser_index >= 0 && laser_index < ARRAY_SIZE(laser_devices))
    {
        rc = laser_devices[laser_index].data_remove(client);
    }

    return rc;
}

static int laser_data_init(struct i2c_client *client,
                           const struct i2c_device_id *id)
{
    // try sub devices
    int rc = 0;
    int i = 0;

    if(client == NULL || id == NULL)
    {
        laser_err("parameter error");
        return -EINVAL;
    }

    for (i = 0; i < ARRAY_SIZE(laser_devices); i++)
    {
        rc = laser_devices[i].data_init(client, id);
        if (rc == 0) {
            break;
        }
    }

    laser_index = i;
    laser_info("sensor index = %d", i);
    return 0;
}

hw_laser_fn_t laser_module_fn = {
    .laser_ioctl = hw_laser_ioctl,
};

static hw_laser_ctrl_t laser_module_ctrl = {
    .func_tbl = &laser_module_fn,
    .data = NULL,
};

static const struct i2c_device_id laser_module_id[] = {
	{ "vl53l0_970",  (unsigned long)&laser_module_ctrl},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(i2c, laser_module_id);

static const struct of_device_id laser_module_of_match[] = {
	{ .compatible = "huawei,vl53l0_970", },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, laser_module_of_match);

static int laser_module_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    int rc = 0;
    if((client == NULL) || (id ==NULL))
        return -EINVAL;

    laser_info("module probe enter");
    rc = laser_data_init(client, id);
    if(0 == rc)
    {
        laser_info("laser_data_init success");
        rc = laser_probe(client, id);
    }

    return rc;
}

static int laser_module_remove(struct i2c_client *client)
{
    int rc = 0;
    if(client == NULL)
        return -EINVAL;

    laser_info("module remove enter");
    rc = laser_data_remove(client);
    if(0 == rc)
    {
        rc = laser_remove(client);
    }
    return rc;
}

static struct i2c_driver laser_module_i2c_driver = {
    .driver = {
        .name	= DRIVER_NAME,
        .of_match_table = of_match_ptr(laser_module_of_match),
    },
    .probe		= laser_module_probe,
    .remove     = laser_module_remove,
    .id_table	= laser_module_id,
};

static int __init laser_module_init(void)
{
    laser_err("%s enter.\n", __func__);
    return i2c_add_driver(&laser_module_i2c_driver);
}

static void __exit laser_module_exit(void)
{
    laser_err("%s enter.", __func__);
    i2c_del_driver(&laser_module_i2c_driver);
    return;
}

module_init(laser_module_init);
module_exit(laser_module_exit);
MODULE_DESCRIPTION("laser driver");
MODULE_LICENSE("GPL v2");
//lint -restore
