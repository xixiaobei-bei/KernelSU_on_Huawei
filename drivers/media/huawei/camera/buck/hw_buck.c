/*
 * hw_buck.c
 *
 * common fun for buck
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hw_buck.h"
#include "../cam_log.h"
#include "../platform/sensor_commom.h"
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

struct hw_buck_ctrl_t *hw_buck_ctrl;

void hw_set_buck_ctrl(struct hw_buck_ctrl_t *buck_ctrl)
{
	hw_buck_ctrl = buck_ctrl;
}

struct hw_buck_ctrl_t *hw_get_buck_ctrl(void)
{
	return hw_buck_ctrl;
}

int hw_buck_i2c_read(struct hw_buck_i2c_client *client,
			u8 reg, u8 *data)
{
	int rc;
	struct i2c_msg msgs[2]; /* two bytes */

	cam_debug("%s enter", __func__);
	if (!client || !data)
		return -1;

	msgs[0].addr = client->client->addr;
	msgs[0].flags = 0; /* default value */
	msgs[0].len = 1; /* one byte */
	msgs[0].buf = &reg;

	msgs[1].addr = client->client->addr;
	msgs[1].flags = I2C_M_RD; /* read data, from slave to master */
	msgs[1].len = 1; /* one byte */
	msgs[1].buf = data;

	rc = i2c_transfer(client->client->adapter, msgs, 2); /* msg len is 2 */
	if (rc < 0)
		cam_err("%s transfer error, reg=0x%x, data=0x%x, addr=0x%x",
		__func__, reg, *data, msgs[0].addr);
	else
		cam_debug("%s reg=0x%x, data=0x%x", __func__, reg, *data);

	return rc;
}

int hw_buck_i2c_write(struct hw_buck_i2c_client *client,
			u8 reg, u8 data)
{
	int rc;
	u8 buf[2]; /* two bytes */
	struct i2c_msg msg;

	if (!client)
		return -1;

	cam_debug("%s reg=0x%x, data=0x%x", __func__, reg, data);

	buf[0] = reg;
	buf[1] = data;
	msg.addr = client->client->addr;
	msg.flags = 0; /* default value */
	msg.len = 2; /* the msg len is 2 */
	msg.buf = buf;

	rc = i2c_transfer(client->client->adapter, &msg, 1);

	if (rc < 0)
		cam_err("%s transfer error, reg=0x%x, data=0x%x, addr=0x%x",
		__func__, reg, data, msg.addr);
	else
		cam_debug("%s reg=0x%x, data=0x%x", __func__, reg, data);

	return rc;
}


int hw_buck_get_dt_data(struct hw_buck_ctrl_t *buck_ctrl)
{
	struct device_node *dev_node = NULL;
	struct hw_buck_info *buck_info = NULL;
	int rc;

	if (!buck_ctrl) {
		cam_err("%s buck ctrl is NULL", __func__);
		return -1;
	}
	buck_info = &buck_ctrl->buck_info;
	dev_node = buck_ctrl->dev->of_node;

	rc = of_property_read_string(dev_node,
				"huawei,buck_name",
				&buck_info->name);
	cam_info("%s hw,buck_name %s, rc %d",
		__func__, buck_info->name, rc);
	if (rc < 0) {
		cam_err("%s failed %d", __func__, __LINE__);
		return -1;
	}

	rc = of_property_read_u32(dev_node,
			"huawei,slave_address",
			&buck_info->slave_address);
	cam_info("%s slave_address %d, rc %d", __func__,
		buck_info->slave_address, rc);
	if (rc < 0)
		cam_err("%s failed %d", __func__, __LINE__);

	rc = of_property_read_u32(dev_node, "huawei,en-pin", &buck_info->en_pin);
	cam_info("%s huawei,en-pin %d", __func__, buck_info->en_pin);
	if (rc < 0)
		cam_err("%s, get en-pin failed %d", __func__, __LINE__);

	rc = of_property_read_u32(dev_node, "huawei,vsel-pin", &buck_info->pin_vsel);
	cam_info("%s huawei,vsel-pin %d", __func__, buck_info->pin_vsel);
	if (rc < 0)
		cam_err("%s, get vsel-pin failed %d", __func__, __LINE__);

	return 0;
}

int hw_buck_remove(struct i2c_client *client)
{
	cam_info("%s enter", __func__);
	if (!client)
		return -1;
	client->adapter = NULL;

	return 0;
}

struct hw_buck_i2c_fn_t buck_i2c_func_tbl = {
	.i2c_read = hw_buck_i2c_read,
	.i2c_write = hw_buck_i2c_write,
};

int32_t hw_buck_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = NULL;
	struct hw_buck_ctrl_t *buck_ctrl = NULL;
	int32_t rc = 0;

	if (!client || !id)
		return -1;
	cam_info("%s client name = %s\n", __func__, client->name);

	adapter = client->adapter;
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
		cam_err("%s i2c_check_functionality failed\n", __func__);
		return -1;
	}

	buck_ctrl = (struct hw_buck_ctrl_t *)id->driver_data;
	buck_ctrl->buck_i2c_client->client = client;
	buck_ctrl->dev = &client->dev;
	buck_ctrl->buck_i2c_client->i2c_func_tbl = &buck_i2c_func_tbl;

	rc = buck_ctrl->func_tbl->buck_get_dt_data(buck_ctrl);
	if (rc < 0) {
		cam_err("%s buck_get_dt_data failed", __func__);
		return -1;
	}

	rc = buck_ctrl->func_tbl->buck_init(buck_ctrl);
	if (rc < 0) {
		cam_err("%s buck init failed\n", __func__);
		return -1;
	}

	rc = buck_ctrl->func_tbl->buck_match(buck_ctrl);
	if (rc < 0) {
		cam_err("%s buck match failed\n", __func__);
		return -1;
	}

	hw_set_buck_ctrl(buck_ctrl);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_BUCK);
#endif

	return rc;
}

