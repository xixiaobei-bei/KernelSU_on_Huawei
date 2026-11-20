/*
 * boost_5v.h
 *
 * boost with 5v driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _BOOST_5V_H_
#define _BOOST_5V_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/delay.h>

#define BOOST_5V_ENABLE            1
#define BOOST_5V_DISABLE           0

#define BOOST_5V_VOTE_OBJECT       "boost_5v"

#define BOOST_CTRL_BOOST_GPIO_OTG  "boost_gpio_otg"
#define BOOST_CTRL_PD_VCONN        "pd"
#define BOOST_CTRL_DC              "dc"
#define BOOST_CTRL_MOTOER          "motor"
#define BOOST_CTRL_AUDIO           "audio"
#define BOOST_CTRL_AT_CMD          "at_cmd"
#define BOOST_CTRL_FCP             "fcp"
#define BOOST_CTRL_WLDC            "wldc"
#define BOOST_CTRL_WLRX            "wlrx"
#define BOOST_CTRL_WLTX            "wltx"
#define BOOST_CTRL_WLC             "wlc"
#define BOOST_CTRL_WLRX_AUX        "wlrx_aux"
#define BOOST_CTRL_WLTX_AUX        "wltx_aux"
#define BOOST_CTRL_CAMERA          "camera"
#define BOOST_CTRL_NFC             "nfc"
#define BOOST_CTRL_SC_CHIP         "sc_chip"
#define BOOST_CTRL_SIM             "sim"
#define BOOST_CTRL_LCD             "lcd"
#define BOOST_CTRL_SD              "sd"

struct boost_5v_dev {
	struct device *dev;
	u32 use_gpio;
	u32 use_pmic;
	int gpio_no;
	u32 use_buckboost;
	u32 buckboost_vol;
};

#ifdef CONFIG_BOOST_5V
extern int boost_5v_enable(bool enable, const char *client_name);
extern bool boost_5v_status(const char *client_name);
#else
static inline int boost_5v_enable(bool enable, const char *client_name)
{
	return 0;
}

static inline bool boost_5v_status(const char *client_name)
{
	return false;
}
#endif /* CONFIG_BOOST_5V */

#endif /* _BOOST_5V_H_ */
