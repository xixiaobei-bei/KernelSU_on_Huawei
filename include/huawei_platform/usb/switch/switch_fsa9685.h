/*
 * switch_fsa9685.h
 *
 * header file for switch_fsa9685 driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _SWITCH_FSA9685_H_
#define _SWITCH_FSA9685_H_

#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/usb/switch/usbswitch_common.h>

#define FSA9685_RD_BUF_SIZE               (32)
#define FSA9685_WR_BUF_SIZE               (64)

/* usb state */
#define FSA9685_OPEN                      (0)
#define FSA9685_USB1_ID_TO_IDBYPASS       (1)
#define FSA9685_USB2_ID_TO_IDBYPASS       (2)
#define FSA9685_UART_ID_TO_IDBYPASS       (3)
#define FSA9685_USB1_ID_TO_VBAT           (5)

enum err_oprt_reg_num {
	ERR_FSA9685_REG_MANUAL_SW_1 = 1,
	ERR_FSA9685_READ_REG_CONTROL = 2,
	ERR_FSA9685_WRITE_REG_CONTROL = 3,
};

struct fsa9685_device_info {
	struct device *dev;
	struct i2c_client *client;
	int device_id;

	struct mutex accp_detect_lock;
	struct mutex accp_adaptor_reg_lock;
	struct wakeup_source usb_switch_lock;

	struct work_struct g_intb_work;
	struct delayed_work detach_delayed_work;

#ifdef CONFIG_BOOST_5V
	struct notifier_block usb_nb;
#endif

	u32 fcp_support;
	u32 scp_support;
	u32 two_switch_flag; /* disable for two switch */
	u32 pd_support;
	u32 dcd_timeout_force_enable;
	u32 power_by_5v;
};

struct fsa9685_device_ops {
	int (*dump_regs)(char *buf);
	int (*switchctrl_store)(struct i2c_client *client, int action);
	int (*switchctrl_show)(char *buf);
	int (*manual_switch)(int input_select);
	void (*detach_work)(void);
};

extern struct fsa9685_device_ops *usbswitch_fsa9685_get_device_ops(void);
extern struct fsa9685_device_ops *usbswitch_rt8979_get_device_ops(void);
struct fsa9685_device_info *switch_get_dev_info(void);

#define ADAPTOR_BC12_TYPE_MAX_CHECK_TIME 100
#define WAIT_FOR_BC12_DELAY 5
#define ACCP_NOT_PREPARE_OK -1
#define ACCP_PREPARE_OK 0
#define BOOST_5V_CLOSE_FAIL -1
#define SET_DCDTOUT_FAIL -1
#define SET_DCDTOUT_SUCC 0

#endif /* end of _SWITCH_FSA9685_H_ */
