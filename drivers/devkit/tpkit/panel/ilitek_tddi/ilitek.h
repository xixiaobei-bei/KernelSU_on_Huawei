/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef __ILI9881X_H
#define __ILI9881X_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/regulator/consumer.h>
#include <linux/power_supply.h>
#include <linux/fs.h>
#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif
#include <linux/uaccess.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <net/sock.h>

#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/time.h>

#include <linux/namei.h>
#include <linux/vmalloc.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>

#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/rtc.h>
#include <linux/syscalls.h>
#include <linux/security.h>
#include <linux/mount.h>
#include <linux/firmware.h>

#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif

#ifdef CONFIG_FB
#include <linux/notifier.h>
#include <linux/fb.h>
#else
#include <linux/earlysuspend.h>
#endif

#ifdef CONFIG_DRM_MSM
#include <linux/msm_drm_notify.h>
#endif

#ifdef CONFIG_MTK_SPI
#include "mt_spi.h"
#include "sync_write.h"
#endif

#include <huawei_platform/log/hw_log.h>
#include "huawei_ts_kit.h"
#include "huawei_ts_kit_algo.h"
#include "ilitek_dts.h"

#define DRIVER_VERSION "3.0.2.0.200410"
#define ILITEK_TEST_FAILED_REASON_LEN 24

typedef enum cmd_types{
	CMD_DISABLE = 0x00,
	CMD_ENABLE = 0x01,
	CMD_SEAMLESS = 0x02,
	CMD_STATUS = 0x03,
	CMD_PT_MODE = 0x04,
	CMD_ROI_DATA = 0x0E,
}cmd_types;
extern int pc_cnt_in;
#define ILITEK_ROI_FINGERS 2
#define P5_X_READ_ROI_CTRL 0x01
#define P5_X_ROI_CMD 0x0F
#define P5_X_READ_ROI_STUAUS 0x02
#define P5_X_WRITE_ROI_DISABLE 0x00
#define P5_X_WRITE_ROI_ENABLE 0x01
#define P5_X_READ_ROI_STATUS_LEN 0x03
#define P5_X_GET_ROI_DATA 0x03

/* Options */
#define TDDI_INTERFACE BUS_SPI /* BUS_I2C(0x18) or BUS_SPI(0x1C) */
#define VDD_VOLTAGE 1800000
#define VCC_VOLTAGE 1800000
#define SPI_CLK 9 /* follow by clk list */
#define SPI_RETRY 5
#define TR_BUF_SIZE (2*K) /* Buffer size of touch report */
#define TR_BUF_LIST_SIZE (1*K) /* Buffer size of touch report for debug data */
#define SPI_TX_BUF_SIZE 4096
#define SPI_RX_BUF_SIZE 4096
#define WQ_ESD_DELAY 4000
#define WQ_BAT_DELAY 2000
#define AP_INT_TIMEOUT 3000 /*3s*/
#define MP_INT_TIMEOUT 5000 /*5s*/
#define MT_B_TYPE ENABLE
#define TDDI_RST_BIND DISABLE
#define MT_PRESSURE ENABLE
#define ENABLE_WQ_ESD DISABLE
#define ENABLE_WQ_BAT DISABLE
#define ENABLE_GESTURE ENABLE
#define REGULATOR_POWER DISABLE
#define TP_SUSPEND_PRIO ENABLE
#define RESUME_BY_DDI DISABLE
#define BOOT_FW_UPDATE ENABLE
#define MP_INT_LEVEL ENABLE

#define ILITEK_TEST_MODULE_LEN 64

/*if current interface is spi, must to hostdownload */
#if (TDDI_INTERFACE == BUS_SPI)
#define HOST_DOWN_LOAD ENABLE
#else
#define HOST_DOWN_LOAD DISABLE
#endif
/* Plaform compatibility */
#define CONFIG_PLAT_SPRD DISABLE
#define I2C_DMA_TRANSFER DISABLE
#define SPI_DMA_TRANSFER_SPLIT ENABLE

/* Path */
#define DEBUG_DATA_FILE_SIZE (10*K)
#define DEBUG_DATA_FILE_PATH "/data/vendor/log/ILITEK_log.csv"
#define CSV_LCM_ON_PATH "/data/vendor/log/mp_lcm_on_log"
#define CSV_LCM_OFF_PATH "/data/vendor/log/ilitek_mp_lcm_off_log"
#define POWER_STATUS_PATH "/sys/class/power_supply/battery/status"
#define DUMP_FLASH_PATH "/data/vendor/log/flash_dump"
#define DUMP_IRAM_PATH "/data/vendor/log/iram_dump"

#define ILITEK_UPGRADE_FW_SD_NAME "ts/touch_screen_firmware.img"

#define ILITEK_INI_PATH_PERFIX "/odm/etc/firmware/ts/"

/* Debug messages */
#define DEBUG_NONE 0
#define DEBUG_ALL 1
#define DEBUG_OUTPUT DEBUG_NONE


#define ILITEK_PROJECT_ID_LEN 10 + 1
#define ILITEK_FILE_PATH_LEN 256
#define ILITEK_FILE_NAME_LEN 128

#define USB_CHARGER_IN (1)
#define USB_CHARGER_OUT (0)

extern bool debug_en;

#define err_alloc_mem(X) ((IS_ERR(X) || X == NULL) ? 1 : 0)
#define K (1024)
#define M (K * K)
#define ENABLE 1
#define START 1
#define ON 1
#define ILI_WRITE 1
#define ILI_READ 0
#define DISABLE 0
#define END 0
#define OFF 0
#define NONE -1
#define DO_SPI_RECOVER -2

enum TP_SPI_CLK_LIST {
	TP_SPI_CLK_1M = 1000000,
	TP_SPI_CLK_2M = 2000000,
	TP_SPI_CLK_3M = 3000000,
	TP_SPI_CLK_4M = 4000000,
	TP_SPI_CLK_5M = 5000000,
	TP_SPI_CLK_6M = 6000000,
	TP_SPI_CLK_7M = 7000000,
	TP_SPI_CLK_8M = 8000000,
	TP_SPI_CLK_9M = 9000000,
	TP_SPI_CLK_10M = 10000000,
	TP_SPI_CLK_11M = 11000000,
	TP_SPI_CLK_12M = 12000000,
	TP_SPI_CLK_13M = 13000000,
	TP_SPI_CLK_14M = 14000000,
	TP_SPI_CLK_15M = 15000000,
};

enum TP_PLAT_TYPE {
	TP_PLAT_MTK = 0,
	TP_PLAT_QCOM
};

enum TP_RST_METHOD {
	TP_IC_WHOLE_RST = 0,
	TP_IC_CODE_RST,
	TP_HW_RST_ONLY,
};

enum TP_FW_UPGRADE_TYPE {
	UPGRADE_FLASH = 0,
	UPGRADE_IRAM
};

enum TP_FW_UPGRADE_STATUS {
	FW_STAT_INIT = 0,
	FW_UPDATING = 90,
	FW_UPDATE_PASS = 100,
	FW_UPDATE_FAIL = -1
};

enum TP_FW_OPEN_METHOD {
	REQUEST_FIRMWARE = 0,
	FILP_OPEN
};

enum TP_SLEEP_STATUS {
	TP_SUSPEND = 0,
	TP_DEEP_SLEEP = 1,
	TP_RESUME = 2
};

enum TP_PROXIMITY_STATUS {
	DDI_POWER_OFF = 0,
	DDI_POWER_ON = 1,
	WAKE_UP_GESTURE_RECOVERY = 2,
	WAKE_UP_SWITCH_GESTURE_MODE = 3
};

enum TP_SLEEP_CTRL {
	SLEEP_IN = 0x0,
	SLEEP_OUT = 0x1,
	DEEP_SLEEP_IN = 0x3
};

enum TP_FW_BLOCK_NUM {
	AP = 1,
	DATA = 2,
	TUNING = 3,
	GESTURE = 4,
	MP = 5,
	DDI = 6,
	TAG = 7,
	PARA_BACKUP = 8,
	RESERVE_BLOCK3 = 9,
	RESERVE_BLOCK4 = 10,
	RESERVE_BLOCK5 = 11,
	RESERVE_BLOCK6 = 12,
	RESERVE_BLOCK7 = 13,
	RESERVE_BLOCK8 = 14,
	RESERVE_BLOCK9 = 15,
	RESERVE_BLOCK10 = 16,
};

enum TP_FW_BLOCK_TAG {
	BLOCK_TAG_AF = 0xAF,
	BLOCK_TAG_B0 = 0xB0
};

enum TP_WQ_TYPE {
	WQ_ESD = 0,
	WQ_BAT,
};

enum TP_RECORD_DATA {
	DISABLE_RECORD = 0,
	ENABLE_RECORD,
	DATA_RECORD
};

enum TP_DATA_FORMAT {
	DATA_FORMAT_DEMO = 0,
	DATA_FORMAT_DEBUG,
	DATA_FORMAT_DEMO_DEBUG_INFO,
	DATA_FORMAT_GESTURE_SPECIAL_DEMO,
	DATA_FORMAT_GESTURE_INFO,
	DATA_FORMAT_GESTURE_NORMAL,
	DATA_FORMAT_GESTURE_DEMO,
	DATA_FORMAT_GESTURE_DEBUG,
	DATA_FORMAT_DEBUG_LITE_ROI,
	DATA_FORMAT_DEBUG_LITE_WINDOW,
	DATA_FORMAT_DEBUG_LITE_AREA
};

enum NODE_MODE_SWITCH {
	AP_MODE = 0,
	TEST_MODE,
	DEBUG_MODE,
	DEBUG_LITE_ROI,
	DEBUG_LITE_WINDOW,
	DEBUG_LITE_AREA
};

enum TP_MODEL {
	MODEL_DEF = 0,
	MODEL_CSOT,
	MODEL_AUO,
	MODEL_BOE,
	MODEL_INX,
	MODEL_DJ,
	MODEL_TXD,
	MODEL_TM
};

enum TP_ERR_CODE {
	EMP_CMD = 100,
	EMP_PROTOCOL,
	EMP_FILE,
	EMP_INI,
	EMP_TIMING_INFO,
	EMP_INVAL,
	EMP_PARSE,
	EMP_NOMEM,
	EMP_GET_CDC,
	EMP_INT,
	EMP_CHECK_BUY,
	EMP_MODE,
	EMP_FW_PROC,
	EMP_FORMUL_NULL,
	EFW_CONVERT_FILE,
	EFW_ICE_MODE,
	EFW_CRC,
	EFW_REST,
	EFW_ERASE,
	EFW_PROGRAM,
};

enum TP_IC_TYPE {
	ILI_A = 0x0A,
	ILI_B,
	ILI_C,
	ILI_D,
	ILI_E,
	ILI_F,
	ILI_G,
	ILI_H,
	ILI_I,
	ILI_J,
	ILI_K,
	ILI_L,
	ILI_M,
	ILI_N,
	ILI_O,
	ILI_P,
	ILI_Q,
	ILI_R,
	ILI_S,
	ILI_T,
	ILI_U,
	ILI_V,
	ILI_W,
	ILI_X,
	ILI_Y,
	ILI_Z,
};

struct gesture_symbol {
	u8 double_tap : 1;
	u8 alphabet_line_2_top : 1;
	u8 alphabet_line_2_bottom : 1;
	u8 alphabet_line_2_left : 1;
	u8 alphabet_line_2_right : 1;
	u8 alphabet_m : 1;
	u8 alphabet_w : 1;
	u8 alphabet_c : 1;
	u8 alphabet_e : 1;
	u8 alphabet_v : 1;
	u8 alphabet_o : 1;
	u8 alphabet_s : 1;
	u8 alphabet_z : 1;
	u8 alphabet_v_down : 1;
	u8 alphabet_v_left : 1;
	u8 alphabet_v_right : 1;
	u8 alphabet_two_line_2_bottom : 1;
	u8 alphabet_f : 1;
	u8 alphabet_at : 1;
	u8 reserve0 : 5;
};

#define TDDI_I2C_ADDR 0x41
#define TDDI_DEV_ID "ILITEK_TDDI"

 /* define the width and heigth of a screen. */
#define TOUCH_SCREEN_X_MIN 0
#define TOUCH_SCREEN_Y_MIN 0
#define TOUCH_SCREEN_X_MAX 720
#define TOUCH_SCREEN_Y_MAX 1440
#define MAX_TOUCH_NUM 10

/* define the range on panel */
#define TPD_HEIGHT 2048
#define TPD_WIDTH 2048

/* Firmware upgrade */
#define CORE_VER_1410 0x01040100
#define CORE_VER_1420 0x01040200
#define CORE_VER_1430 0x01040300
#define CORE_VER_1460 0x01040600
#define MAX_HEX_FILE_SIZE (160*K)
#define ILI_FILE_HEADER 256
#define DLM_START_ADDRESS 0x20610
#define DLM_HEX_ADDRESS 0x10000
#define MP_HEX_ADDRESS 0x13000
#define DDI_RSV_BK_ST_ADDR 0x1E000
#define DDI_RSV_BK_END_ADDR 0x1FFFF
#define DDI_RSV_BK_SIZE (1*K)
#define RSV_BK_ST_ADDR 0x1E000
#define RSV_BK_END_ADDR 0x1E3FF
#define FW_VER_ADDR 0xFFE0
#define FW_BLOCK_INFO_NUM 17
#define SPI_UPGRADE_LEN 2048
#define SPI_BUF_SIZE MAX_HEX_FILE_SIZE
#define INFO_HEX_ST_ADDR 0x4F
#define INFO_MP_HEX_ADDR 0x1F

/* DMA Control Registers */
#define DMA_BASED_ADDR 0x72000
#define DMA48_ADDR (DMA_BASED_ADDR + 0xC0)
#define DMA48_REG_DMA_CH0_TRIGGER_SEL (BIT(16)|BIT(17)|BIT(18)|BIT(19))
#define DMA48_REG_DMA_CH0_START_CLEAR BIT(25)

#define DMA49_ADDR (DMA_BASED_ADDR + 0xC4)
#define DMA49_REG_DMA_CH0_SRC1_ADDR DMA49_ADDR

#define DMA50_ADDR (DMA_BASED_ADDR + 0xC8)
#define DMA50_REG_DMA_CH0_SRC1_STEP_INC DMA50_ADDR
#define DMA50_REG_DMA_CH0_SRC1_FORMAT (BIT(24)|BIT(25))
#define DMA50_REG_DMA_CH0_SRC1_EN BIT(31)

#define DMA52_ADDR (DMA_BASED_ADDR + 0xD0)
#define DMA52_REG_DMA_CH0_SRC2_EN BIT(31)

#define DMA53_ADDR (DMA_BASED_ADDR + 0xD4)
#define DMA53_REG_DMA_CH0_DEST_ADDR DMA53_ADDR

#define DMA54_ADDR (DMA_BASED_ADDR + 0xD8)
#define DMA54_REG_DMA_CH0_DEST_STEP_INC DMA54_ADDR
#define DMA54_REG_DMA_CH0_DEST_FORMAT (BIT(24)|BIT(25))
#define DMA54_REG_DMA_CH0_DEST_EN BIT(31)

#define DMA55_ADDR (DMA_BASED_ADDR + 0xDC)
#define DMA55_REG_DMA_CH0_TRAFER_COUNTS DMA55_ADDR
#define DMA55_REG_DMA_CH0_TRAFER_MODE (BIT(24)|BIT(25)|BIT(26)|BIT(27))

/* INT Function Registers */
#define INTR_BASED_ADDR 0x48000
#define INTR1_ADDR (INTR_BASED_ADDR + 0x4)
#define INTR1_REG_DMA_CH1_INT_FLAG BIT(16)

#define INTR2_ADDR (INTR_BASED_ADDR + 0x8)
#define INTR2_TDI_ERR_INT_FLAG_CLEAR BIT(18)
#define INTR32_ADDR (INTR_BASED_ADDR + 0x80)
#define INTR32_REG_T0_INT_EN BIT(24)
#define INTR32_REG_T1_INT_EN BIT(25)
#define INTR33_ADDR (INTR_BASED_ADDR + 0x84)
#define INTR33_REG_DMA_CH0_INT_EN BIT(17)

/* Flash */
#define FLASH_BASED_ADDR 0x41000
#define FLASH0_ADDR (FLASH_BASED_ADDR + 0x0)
#define FLASH0_REG_FLASH_CSB FLASH0_ADDR

#define FLASH1_ADDR (FLASH_BASED_ADDR + 0x4)
#define FLASH2_ADDR (FLASH_BASED_ADDR + 0x8)
#define FLASH3_ADDR (FLASH_BASED_ADDR + 0xC)
#define FLASH4_ADDR (FLASH_BASED_ADDR + 0x10)
#define FLASH4_REG_RCV_DATA FLASH4_ADDR

/* Dummy Registers */
#define WDT_DUMMY_BASED_ADDR 0x5101C
#define WDT7_DUMMY0 WDT_DUMMY_BASED_ADDR
#define WDT8_DUMMY1 (WDT_DUMMY_BASED_ADDR + 0x04)
#define WDT9_DUMMY2 (WDT_DUMMY_BASED_ADDR + 0x08)
#define INTR1_REG_FLASH_INT_FLAG BIT(25)


/* The example for the gesture virtual keys */
#define GESTURE_DOUBLECLICK 0x58
#define GESTURE_UP 0x60
#define GESTURE_DOWN 0x61
#define GESTURE_LEFT 0x62
#define GESTURE_RIGHT 0x63
#define GESTURE_M 0x64
#define GESTURE_W 0x65
#define GESTURE_C 0x66
#define GESTURE_E 0x67
#define GESTURE_V 0x68
#define GESTURE_O 0x69
#define GESTURE_S 0x6A
#define GESTURE_Z 0x6B
#define KEY_GESTURE_POWER KEY_POWER
#define KEY_GESTURE_UP KEY_UP
#define KEY_GESTURE_DOWN KEY_DOWN
#define KEY_GESTURE_LEFT KEY_LEFT
#define KEY_GESTURE_RIGHT KEY_RIGHT
#define KEY_GESTURE_O KEY_O
#define KEY_GESTURE_E KEY_E
#define KEY_GESTURE_M KEY_M
#define KEY_GESTURE_W KEY_W
#define KEY_GESTURE_S KEY_S
#define KEY_GESTURE_V KEY_V
#define KEY_GESTURE_C KEY_C
#define KEY_GESTURE_Z KEY_Z
#define KEY_GESTURE_F KEY_F
#define GESTURE_V_DOWN    0x6C
#define GESTURE_V_LEFT    0x6D
#define GESTURE_V_RIGHT   0x6E
#define GESTURE_TWOLINE_DOWN  0x6F
#define GESTURE_F 0x70
#define GESTURE_AT 0x71
#define ESD_GESTURE_PWD 0xF38A94EF
#define SPI_ESD_GESTURE_RUN 0xA67C9DFE
#define I2C_ESD_GESTURE_RUN 0xA67C9DFE
#define SPI_ESD_GESTURE_PWD_ADDR 0x25FF8
#define I2C_ESD_GESTURE_PWD_ADDR 0x40054

#define ESD_GESTURE_CORE146_PWD 0xF38A
#define SPI_ESD_GESTURE_CORE146_RUN 0xA67C
#define I2C_ESD_GESTURE_CORE146_RUN 0xA67C
#define SPI_ESD_GESTURE_CORE146_PWD_ADDR 0x4005C
#define I2C_ESD_GESTURE_CORE146_PWD_ADDR 0x4005C
#define DOUBLE_TAP ON

/* FW data format */
#define DATA_FORMAT_DEMO_CMD 0x00
#define DATA_FORMAT_DEBUG_CMD 0x02
#define DATA_FORMAT_DEMO_DEBUG_INFO_CMD 0x04
#define DATA_FORMAT_GESTURE_NORMAL_CMD  0x01
#define DATA_FORMAT_GESTURE_INFO_CMD 0x02
#define DATA_FORMAT_DEBUG_LITE_CMD 0x05
#define DATA_FORMAT_DEBUG_LITE_ROI_CMD 0x01
#define DATA_FORMAT_DEBUG_LITE_WINDOW_CMD 0x02
#define DATA_FORMAT_DEBUG_LITE_AREA_CMD 0x03
#define P5_X_DEMO_MODE_PACKET_LEN 43
#define P5_X_INFO_HEADER_LENGTH 3
#define P5_X_INFO_CHECKSUM_LENGTH 1
#define P5_X_DEMO_DEBUG_INFO_ID0_LENGTH 14
#define P5_X_DEBUG_MODE_PACKET_LENGTH 1280
#define P5_X_TEST_MODE_PACKET_LENGTH 1180
#define P5_X_GESTURE_NORMAL_LENGTH 8
#define P5_X_GESTURE_INFO_LENGTH 170
#define P5_X_DEBUG_LITE_LENGTH 300
#define P5_X_CORE_VER_THREE_LENGTH 5
#define P5_X_CORE_VER_FOUR_LENGTH 6

/* Protocol */
#define PROTOCOL_VER_500 0x050000
#define PROTOCOL_VER_510 0x050100
#define PROTOCOL_VER_520 0x050200
#define PROTOCOL_VER_530 0x050300
#define PROTOCOL_VER_540 0x050400
#define PROTOCOL_VER_550 0x050500
#define PROTOCOL_VER_560 0x050600
#define PROTOCOL_VER_570 0x050700
#define P5_X_READ_DATA_CTRL 0xF6
#define P5_X_GET_TP_INFORMATION 0x20
#define P5_X_GET_KEY_INFORMATION 0x27
#define P5_X_GET_PANEL_INFORMATION 0x29
#define P5_X_GET_FW_VERSION 0x21
#define P5_X_GET_PROTOCOL_VERSION 0x22
#define P5_X_GET_CORE_VERSION 0x23
#define P5_X_GET_CORE_VERSION_NEW 0x24
#define P5_X_MODE_CONTROL 0xF0
#define P5_X_SET_CDC_INIT 0xF1
#define P5_X_GET_CDC_DATA 0xF2
#define P5_X_CDC_BUSY_STATE 0xF3
#define P5_X_MP_TEST_MODE_INFO 0xFE
#define P5_X_I2C_UART 0x40
#define CMD_GET_FLASH_DATA 0x41
#define CMD_CTRL_INT_ACTION 0x1B
#define P5_X_FW_UNKNOWN_MODE 0xFF
#define P5_X_FW_AP_MODE 0x00
#define P5_X_FW_TEST_MODE 0x01
#define P5_X_FW_GESTURE_MODE 0x0F
#define P5_X_FW_DELTA_DATA_MODE 0x03
#define P5_X_FW_RAW_DATA_MODE 0x08
#define P5_X_DEMO_PACKET_ID 0x5A
#define P5_X_DEBUG_PACKET_ID 0xA7
#define P5_X_TEST_PACKET_ID 0xF2
#define P5_X_GESTURE_PACKET_ID 0xAA
#define P5_X_GESTURE_FAIL_ID 0xAE
#define P5_X_I2CUART_PACKET_ID 0x7A
#define P5_X_DEBUG_LITE_PACKET_ID 0x9A
#define P5_X_SLAVE_MODE_CMD_ID 0x5F
#define P5_X_INFO_HEADER_PACKET_ID 0xB7
#define P5_X_DEMO_DEBUG_INFO_PACKET_ID 0x5C
#define P5_X_EDGE_PLAM_CTRL_1 0x01
#define P5_X_EDGE_PLAM_CTRL_2 0x12
#define SPI_WRITE 0x82
#define SPI_READ 0x83
#define SPI_ACK  0xA3

/* Chips */
#define ILI9881_CHIP 0x9881
#define ILI7807_CHIP 0x7807
#define ILI9881N_AA 0x98811700
#define ILI9881O_AA 0x98811800
#define ILI9882_CHIP 0x9882
#define TDDI_PID_ADDR 0x4009C
#define TDDI_OTP_ID_ADDR 0x400A0
#define TDDI_ANA_ID_ADDR 0x400A4
#define TDDI_PC_COUNTER_ADDR 0x44008
#define TDDI_PC_LATCH_ADDR 0x51010
#define TDDI_CHIP_RESET_ADDR 0x40050
#define RAWDATA_NO_BK_SHIFT 8192

struct ilitek_ts_data {
	struct i2c_client *i2c;
	struct spi_device *spi;
	struct ts_kit_device_data *ts_dev_data;
	struct platform_device *pdev;
	struct input_dev *input;
	struct device *dev;
	struct wakeup_source *ws;

	struct ilitek_hwif_info *hwif;
	struct ilitek_ic_info *chip;
	struct ilitek_protocol_info *protocol;
	struct gesture_coordinate *gcoord;
	struct regulator *vdd;
	struct regulator *vcc;
	struct mutex wrong_touch_lock;

	struct pinctrl *pctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_idle;

#if defined (CONFIG_HUAWEI_DEVKIT_MTK_3_0)
	struct pinctrl_state *pinctrl_state_reset_high;
	struct pinctrl_state *pinctrl_state_reset_low;
	struct pinctrl_state *pinctrl_state_release;
	struct pinctrl_state *pinctrl_state_int_high;
	struct pinctrl_state *pinctrl_state_int_low;
	struct pinctrl_state *pinctrl_state_as_int;
#endif

#ifdef CONFIG_FB
	struct notifier_block notifier_fb;
#else
	struct early_suspend early_suspend;
#endif

	struct mutex touch_mutex;
	struct mutex debug_mutex;
	struct mutex debug_read_mutex;
	spinlock_t irq_spin;

	struct completion roi_completion;
	struct completion pm_completion;
	bool pm_suspend;
	bool game_switch;

	/* physical path to the input device in the system hierarchy */
	const char *phys;

	bool boot;
	u32 fw_pc;
	u32 fw_latch;

	u32 max_x;
	u32 max_y;
	u32 min_x;
	u32 min_y;
	u32 finger_nums;
	u32 use_ic_res;
	u32 support_roi;
	u32 support_pressure;
	u32 support_gesture;
	u32 support_get_tp_color;
	u32 only_open_once_captest_threshold;
	u32 project_id_length_control;

	bool open_threshold_status;
	u16 panel_wid;
	u16 panel_hei;
	u8 xch_num;
	u8 ych_num;
	u8 stx;
	u8 srx;
	u8 *update_buf;
	u8 *tr_buf;
	u8 *spi_tx;
	u8 *spi_rx;
	struct firmware tp_fw;

	int actual_tp_mode;
	int tp_data_format;
	int tp_data_len;

	int irq_num;
	int irq_gpio;
	int irq_tirgger_type;
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
	int tp_rst;
#endif
	int wait_int_timeout;

	int finger;
	int curt_touch[MAX_TOUCH_NUM];
	int prev_touch[MAX_TOUCH_NUM];
	int last_touch;

	u8 roi_data[ROI_DATA_READ_LENGTH];
	u8 roi_buf[ROI_DATA_READ_LENGTH+1];
	bool roi_switch_backup;
	struct mutex roi_mutex;
	bool roi_data_ready;
	int fw_retry;
	int fw_update_stat;
	int fw_open;
	u8  fw_info[75];
	u8  fw_mp_ver[4];
	bool wq_ctrl;
	bool wq_esd_ctrl;
	bool wq_bat_ctrl;

	bool netlink;
	bool report;
	bool gesture;
	bool mp_retry;
	int gesture_mode;
	int gesture_demo_ctrl;
	struct gesture_symbol ges_sym;

	u16 flash_mid;
	u16 flash_devid;
	int program_page;
	int flash_sector;

	/* Sending report data to users for the debug */
	bool dnp; //debug node open
	int dbf; //debug data frame
	int odi; //out data index
	wait_queue_head_t inq;
	struct debug_buf_list *dbl;
	int raw_count;
	int delta_count;
	int bg_count;
	int offset;
	int reset;
	int rst_edge_delay;
	int fw_upgrade_mode;
	int mp_ret_len;
	bool wtd_ctrl;
	bool force_fw_update;
	bool irq_after_recovery;
	bool ddi_rest_done;
	bool resume_by_ddi;
	bool tp_suspend;
	bool info_from_hex;
	bool prox_near;
	bool gesture_load_code;
	bool trans_xy;
	bool ss_ctrl;
	bool node_update;
	bool int_pulse;

	bool esd_debug;
	u8 esd_debug_delay;

	/* module info */
	int tp_module;
	char *md_name;
	char *md_fw_filp_path;
	char *md_fw_rq_path;
	char *md_ini_path;
	char *md_ini_rq_path;
	char ini_path[ILITEK_FILE_PATH_LEN];

	char project_id[ILITEK_PROJECT_ID_LEN];
	char fw_name[ILITEK_FILE_PATH_LEN];

	atomic_t irq_stat;
	atomic_t tp_reset;
	atomic_t ice_stat;
	atomic_t fw_stat;
	atomic_t mp_stat;
	atomic_t tp_sleep;
	atomic_t tp_sw_mode;
	atomic_t cmd_int_check;
	atomic_t esd_stat;

	/* Event for driver test */
	struct completion esd_done;

	int (*spi_write_then_read)(struct spi_device *spi, const void *txbuf,
				unsigned n_tx, void *rxbuf, unsigned n_rx);
	int  (*wrapper)(u8* wdata, u32 wlen, u8* rdata, u32 rlen, bool spi_irq, bool i2c_irq);
	int  (*mp_move_code)(void);
	int  (*gesture_move_code)(int mode);
	int  (*esd_recover)(void);
	int (*ges_recover)(void);
	void (*demo_debug_info[5])(u8 *, size_t);
	int (*detect_int_stat)(bool status);
	enum ts_bus_type btype;
};
extern struct ilitek_ts_data *ilits;

struct debug_buf_list {
	bool mark;
	unsigned char *data;
};

struct ilitek_touch_info {
	u16 id;
	u16 x;
	u16 y;
	u16 pressure;
};

struct gesture_coordinate {
	u16 code;
	u8 clockwise;
	int type;
	struct ilitek_touch_info pos_start;
	struct ilitek_touch_info pos_end;
	struct ilitek_touch_info pos_1st;
	struct ilitek_touch_info pos_2nd;
	struct ilitek_touch_info pos_3rd;
	struct ilitek_touch_info pos_4th;
};

struct ilitek_protocol_info {
	u32 ver;
	int fw_ver_len;
	int pro_ver_len;
	int tp_info_len;
	int key_info_len;
	int panel_info_len;
	int core_ver_len;
	int func_ctrl_len;
	int window_len;
	int cdc_len;
	int mp_info_len;
};

struct ilitek_ic_func_ctrl {
	const char *name;
	u8 cmd[6];
	int len;
};

struct ilitek_ic_info {
	u8 type;
	u8 ver;
	u16 id;
	u32 pid;
	u32 pid_addr;
	u32 pc_counter_addr;
	u32 pc_latch_addr;
	u32 reset_addr;
	u32 otp_addr;
	u32 ana_addr;
	u32 otp_id;
	u32 ana_id;
	u32 fw_ver;
	u32 core_ver;
	u32 fw_mp_ver;
	u32 max_count;
	u32 reset_key;
	u16 wtd_key;
	int no_bk_shift;
	bool dma_reset;
	int (*dma_crc)(u32 start_addr, u32 block_size);
};

struct ilitek_hwif_info {
	u8 bus_type;
	u8 plat_type;
	const char *name;
	struct module *owner;
	const struct of_device_id *of_match_table;
	const struct dev_pm_ops *pm;
	int (*plat_probe)(void);
	int (*plat_remove)(void);
	void *info;
};


/* Prototypes for tddi firmware/flash functions */
void ili_flash_dma_write(u32 start, u32 end, u32 len);
void ili_flash_clear_dma(void);
void ili_fw_read_flash_info(void);
u32 ili_fw_read_hw_crc(u32 start, u32 end);
int ili_fw_read_flash(u32 start, u32 end, u8 *data, int len);
int ili_fw_dump_iram_data(u32 start, u32 end, bool save);
int ili_fw_dump_flash_data(u32 start, u32 end, bool user);
int ili_fw_upgrade(int op);
int ilitek_tddi_upgrade_get_fw_name(void);

/* Prototypes for tddi mp test */
int ili_mp_test_main(struct ts_rawdata_info *raw_info,u8 *apk, bool lcm_on);

/* Prototypes for tddi core functions */
int ili_touch_esd_gesture_flash(void);
int ili_touch_esd_gesture_iram(void);
void ili_set_gesture_symbol(void);
int ili_move_gesture_code_flash(int mode);
int ili_move_gesture_code_iram(int mode);
int ili_move_mp_code_flash(void);
int ili_move_mp_code_iram(void);
void ili_touch_press(u16 x, u16 y, u16 pressure, u16 id);
void ili_touch_release(u16 x, u16 y, u16 id);
void ili_touch_release_all_point(void);
void ili_report_ap_mode(struct ts_fingers *ap_info,u8 *buf, int len);
void ili_report_debug_mode(struct ts_fingers *ap_info,u8 *buf, int len);
void ili_report_debug_lite_mode(struct ts_fingers *ap_info,u8 *buf, int rlen);
void ili_report_gesture_mode(struct ts_fingers *ap_info,u8 *buf, int rlen);
void ili_report_i2cuart_mode(u8 *buf, int rlen);
void ili_ic_set_ddi_reg_onepage(u8 page, u8 reg, u8 data);
void ili_ic_get_ddi_reg_onepage(u8 page, u8 reg, u8 *data);
int ili_ic_whole_reset(void);
int ili_ic_code_reset(void);
int ili_ic_func_ctrl(const char *name, int ctrl);
int ili_ic_int_trigger_ctrl(bool pulse);
void ili_ic_get_pc_counter(int stat);
int ili_ic_check_int_level(bool level);
int ili_ic_check_int_pulse(bool pulse);
int ili_ic_check_busy(int count, int delay);
int ili_ic_get_panel_info(void);
int ili_ic_get_tp_info(void);
int ili_ic_get_core_ver(void);
int ili_ic_get_protocl_ver(void);
int ili_ic_get_fw_ver(void);
int ili_ic_get_info(void);
int ili_ic_dummy_check(void);
int ili_ice_mode_bit_mask_write(u32 addr, u32 mask, u32 value);
int ili_ice_mode_write(u32 addr, u32 data, int len);
int ili_ice_mode_read(u32 addr, u32 *data, int len);
int ili_ice_mode_ctrl(bool enable, bool mcu);
void ili_ic_init(void);
void ili_fw_uart_ctrl(u8 ctrl);

/* Prototypes for tddi events */
#if RESUME_BY_DDI
void ili_resume_by_ddi(void);
#endif
int ili_proximity_far(int mode);
int ili_proximity_near(int mode);
int ili_switch_tp_mode(u8 data);
int ili_set_tp_data_len(int format, bool send, u8* data);
int ili_fw_upgrade_handler(void *data);
int ili_wq_esd_i2c_check(void);
int ili_wq_esd_spi_check(void);
int ili_gesture_recovery(void);
void ili_spi_recovery(void);
void ili_wq_ctrl(int type, int ctrl);
int ili_mp_test_handler(struct ts_rawdata_info *raw_info,u8 *apk, bool lcm_on);
int ili_report_handler(void);
int ili_sleep_handler(int mode);
int ili_reset_ctrl(int mode);
int ili_reset_ctrl_test(int low, int high);
void ili_dev_remove(void);

/* Prototypes for i2c/spi interface */
int ili_core_spi_setup(int num);

int ili_spi_write_then_read_split(struct spi_device *spi,
	const void *txbuf, unsigned n_tx,
	void *rxbuf, unsigned n_rx);
int ili_spi_write_then_read_direct(struct spi_device *spi,
	const void *txbuf, unsigned n_tx,
	void *rxbuf, unsigned n_rx);

/* Prototypes for platform level */

void ili_irq_disable(void);
void ili_irq_enable(void);
void ili_tp_reset(void);


/* Prototypes for miscs */
void ili_node_init(void);
void ili_dump_data(void *data, int type, int len, int row_len, const char *name);
u8 ili_calc_packet_checksum(u8 *packet, int len);
void ili_netlink_reply_msg(void *raw, int size);
int ili_katoi(char *str, int length);
int ili_str2hex(char *str);

int ili_mp_read_write(u8* wdata, u32 wlen, u8* rdata, u32 rlen);
/* Prototypes for additional functionalities */
void ili_gesture_fail_reason(bool enable);
int ili_get_tp_recore_ctrl(int data);
int ili_get_tp_recore_data(void);
void ili_demo_debug_info_mode(struct ts_fingers *ap_info,u8 *buf, size_t rlen);
void ili_demo_debug_info_id0(u8 *buf, size_t len);
int ilitek_bus_init(void);
void ilitek_bus_release(void);
int ilitek_tddi_read_report_data(struct ts_fingers *p_info);
void ilitek_tddi_wq_init(void);
void ili_update_tp_module_info(void);
int dev_mkdir(char *name, umode_t mode);
void ilitek_result_init(struct ts_rawdata_info *info,u8 *result_str);
int ilitek_tddi_rawdata_print(struct seq_file *m, struct ts_rawdata_info *info,int range_size, int row_size);
int ili_ic_get_roi_staus(void);
int ilitek_read_roi_data(void);
void input_kit_read_roi_data(void);
static inline void ipio_kfree(void **mem)
{
	if (*mem != NULL) {
		kfree(*mem);
		*mem = NULL;
	}
}

static inline void ipio_vfree(void **mem)
{
	if (*mem != NULL) {
		vfree(*mem);
		*mem = NULL;
	}
}

static inline int ipio_strcmp(const char *s1, const char *s2)
{
	return (strlen(s1) != strlen(s2)) ? -1 : strncmp(s1, s2, strlen(s1));
}

#endif /* __ILI9881X_H */
