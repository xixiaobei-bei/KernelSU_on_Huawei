/*
 * sw_core.h
 *
 * single-wire driver
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

#ifndef _SW_CORE_H_
#define _SW_CORE_H_

/* include other head file */
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/completion.h>

/* define macro */
#define SUCCESS                     0
#define FAILURE                     1

#define DEBUG_SKB_BUFF_LEN          2048
#define PUBLIC_BUF_MAX              8192

#define INVALID_VALUE               (-1)

/* define for tx and rx packet queue */
#define TX_HIGH_QUEUE               1
#define TX_LOW_QUEUE                2

#define TX_DATA_QUEUE               3
#define RX_DATA_QUEUE               4

/* define Single-wire Protocol Command */
#define PROTO_CMD_KEY_NORAML        ((unsigned char)(3))
#define PROTO_CMD_MOUSE             ((unsigned char)(2))
#define PROTO_CMD_KEY_CONSUMER      ((unsigned char)(5))
#define PROTO_CMD_KEY_CONSUMER_1    ((unsigned char)(0))
#define PROTO_CMD_HANDSHAKE         ((unsigned char)(9))
#define PROTO_CMD_WORKMODE          ((unsigned char)(0xFE))
#define PROTO_CMD_DISCONNECT        ((unsigned char)(0xFF))

/*
 * PCMODE  -- keyborad HID devices attach
 * PADMODE -- keyboard HID devices detach
 */
#define KBMODE_PCMODE               0
#define KBMODE_PADMODE              1

/* define keyboard connect state */
#define KBSTATE_OFFLINE             0
#define KBSTATE_ONLINE              1
#define KBSTATE_NFC_INFO_REQ        2
#define KBSTATE_NFC_NEAR            3
#define KBSTATE_NFC_FAR             4

/* define single-wire protocol information */
#define KB_CRC_HIGH_BYTE            3
#define KB_CRC_LOW_BYTE             4
#define KB_VID_HIGH_BYTE            6
#define KB_VID_LOW_BYTE             7
#define KB_MAIN_VERSION_BYTE        8
#define KB_SUB_VERSION_BYTE         9

#define KB_MODE_BYTE                3
#define KBMODE_DATA_MIN_LEN         4

#define KB_KEY_DATA_HEAD_LEN        5

#define BITS_NUM_PER_BYTE           8
#define KB_PROTO_HEAD_LEN           5

/* define single-wire protocol hand-shake frame size */
#define KB_HANDSHAKE_LEN            10

#define KB_COMM_COMPLETE_TIMEOUT_MS 1000
#define KB_STATE_MAX                2
#define KB_STATE_CONNECT_MAX        4
#define KB_STATE_NFC_NEAR_MAX       4

#define DEFAULT_WAKE_TIMEOUT        (5 * HZ)

/* product id & vndor id define */
#define KIHITECH_KEYBOARD_VID       0x05AF /* kihitech vendor id */
#define KIHITECH_NFC_KEYBOARD_VID   0x12D1 /* kihitech nfc vendor id */

#define CMR_KEYBOARD_PID            0x12D1 /* cmr pad keyboard product id */
#define SCM_KEYBOARD_PID            0x1000 /* scm pad keyboard product id */
#define SCMR_KEYBOARD_PID           0x10AA /* scmr pad keyboard product id */

/* sw bus driver/device */
#define SW_ANY_ID                   (~0)
#define SW_BUS_ANY                  0xffff
#define SW_GROUP_ANY                0x0000
#define SW_TYPE_ANY                 0x0000

#define SW_HID_TYPE                 0x0001
#define SW_DEV_TYPE                 0x0002

#define SW_KB_EVENT_LED_INVALID     0xff
#define SW_REPORT_DATA_SIZE         16
#define KB_DISCONNECTED             0
#define KB_NFC_NOT_WRITE            2

/* struct define */
struct sw_device;

enum sw_hid_dev_num {
	KB_COMMON_KEY_DEV,
	KB_CUSTOM_KEY_DEV,
	KB_MOUSE_DEV,
	KB_HID_DEV_END,
};

enum sw_connect_dev_num {
	SW_DEV_BEGIN = 0,
	SW_DEV_CHARGER = SW_DEV_BEGIN,
	SW_DEV_KB,
	SW_DEV_END,
};

enum sw_kb_event_type {
	SW_KB_EVENT_TYPE_BEGIN = 0,
	SW_KB_EVENT_TYPE_CONNECT_DEV = SW_KB_EVENT_TYPE_BEGIN,
	SW_KB_EVENT_TYPE_END,
};

struct sw_device_id {
	u16 bus;
	u16 group;
	u32 vendor;
	u32 product;
	u32 type;
};

struct sw_driver {
	char *name;
	const struct sw_device_id *id_table;
	int (*probe)(struct sw_device *dev);
	void (*disconnect)(struct sw_device *dev);
	int (*pre_recvframe)(struct sw_device *dev,
		const char *data, int count);
	void (*recvframe)(struct sw_device *dev, struct sk_buff *skb);
	/* private: */
	struct device_driver driver;
};

struct sw_device {
	struct hw_sw_core_data *port;
	struct device dev; /* device */
	u16 bus;           /* bus id */
	u16 group;         /* report group */
	u8 mainver;
	u8 subver;
	u32 vendor;        /* vendor id */
	u32 product;       /* product id */
	u32 version;       /* hid version */
	u32 type;
	struct sw_driver *drv;
	/* protects sw->drv so attributes can pin driver */
	struct mutex drv_mutex;

	/* report descriptor */
	u8 *rd_data;
	u32 rd_size;
	/* (in) context for hid */
	void *context;
	/* worker for registering and unregistering hid devices */
	struct delayed_work uart_work;
};

/*
 * use these in module_init()/module_exit()
 * and don't forget MODULE_DEVICE_TABLE(sw, ...)
 */
int sw_register_driver(struct sw_driver *drv,
	struct module *owner, const char *mod_name);

/* use a define to avoid include chaining to get THIS_MODULE & friends */
#define sw_register(driver) \
	sw_register_driver(driver, THIS_MODULE, KBUILD_MODNAME)

void sw_deregister(struct sw_driver *drv);

/*
 * provide create methods for create for sw device and sw hid device
 * provide release method for all sw device
 */
struct sw_device *sw_create_new_device(u32 pid, u32 vid);
struct sw_device *sw_register_new_hiddevice(int devno,
	struct sw_device *dev, u8 *rd_data, u32 rd_size);
int sw_release_device(struct sw_device *device);

/* function from sw_hid */
void sw_hid_input_report(struct sw_device *dev, u8 *data, int size);
int swhid_init(void);
void swhid_exit(void);

/* function from sw_utils */
int sw_check_recv_data(const u8 *data, int count);
int hkadc_detect_value(int adc);

/* function from sw_hid_vendor */
int sw_drivers_init(void);
void sw_drivers_exit(void);

/* function from sw_device */
int sw_bus_init(void);
void sw_bus_exit(void);

int cmrkb_init(void);
void cmrkb_exit(void);

int scmkb_init(void);
void scmkb_exit(void);

extern void kb_extented_dmd_report(int connected, int nfcresult);

#endif /* _SW_CORE_H_ */
