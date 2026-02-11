/*
 * Huawei Touchscreen Driver
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

#ifndef _THP_H_
#define _THP_H_

/* define THP_CHARGER_FB here to enable charger notify callback */
#define THP_CHARGER_FB
#if defined(THP_CHARGER_FB)
#ifndef CONFIG_HUAWEI_THP_QCOM
#include <linux/hisi/usb/hisi_usb.h>
#endif
#endif
#ifdef CONFIG_HUAWEI_SHB_THP
#include <huawei_platform/inputhub/thphub.h>
#endif
#include <linux/amba/pl022.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/platform_device.h>
#include <linux/ctype.h>
#include <linux/spi/spi.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/pm_wakeup.h>
#include <uapi/linux/sched/types.h>

#define THP_UNBLOCK 5
#define THP_TIMEOUT 6

#define THP_RESET_LOW 0
#define THP_RESET_HIGH 1

#define THP_IRQ_ENABLE 1
#define THP_IRQ_DISABLE 0
#define NO_ERR 0

#define THP_GET_FRAME_WAIT_COUNT 1
#define THP_GET_FRAME_BLOCK 1
#define THP_GET_FRAME_NONBLOCK 0

#define THP_SYNC_INFO_SIZE (2 * 1024)

#define MAX_INVALID_IRQ_NUM 200
#define THP_IRQ_STORM_FREQ 3 // 3 invalid irq per ms
#define MAX_PRINT_LOG_INVALID_IRQ_NUM 20

#define NEED_LOCK 1
#define DONOT_NEED_LOCK 0

#define RING_SWITCH_ON 1
#define RING_SWITCH_OFF 0

#define PNONE_STATUS_ON 1
#define PNONE_STATUS_OFF 0

#define PROJECTID_VALUE 0
#define PROJECTID_SV1 "D160970901"

#define DOUBLE_CLICK_ON 1
#define DOUBLE_CLICK_OFF 0

#define THP_POWER_ON 1

#define SINGLE_CLICK_ON 1
#define SINGLE_CLICK_OFF 0

enum thp_volume_side_status {
	VOLUME_SIDE_STATUS_DEFAULT = 0,
	VOLUME_SIDE_STATUS_LEFT,
	VOLUME_SIDE_STATUS_RIGHT,
	VOLUME_SIDE_STATUS_BOTH
};

struct thp_ioctl_get_frame_data {
	char __user *buf;
	char __user *tv; /* struct timeval */
	unsigned int size;
};
struct thp_ioctl_spi_sync_data {
	char __user *tx;
	char __user *rx;
	unsigned int size;
};
struct thp_sync_info {
	u32 size;
	char data[THP_SYNC_INFO_SIZE];
};

#define MAX_SPI_XFER_DATA_NUM 5

/* thp_ioctl_spi_xfer_data
 * delay_usecs: delay time(us) after each xfer
 * cs_change:
 * 1:cs will be pull up after each xfer;
 * 0:cs keep low state throughout the transmission process.
 */
struct thp_ioctl_spi_xfer_data {
	char __user *tx;
	char __user *rx;
	unsigned int len;
	unsigned short delay_usecs;
	unsigned char cs_change;
	unsigned char reserved[3];
};
/* thp_ioctl_spi_msg_package
 * speed_hz:
 * 0:no need to change transfer speed;
 * others: change spi speed to this value,and restore the original
 * speed at the end of the transfer
 */
struct thp_ioctl_spi_msg_package {
	unsigned int speed_hz;
	unsigned int xfer_num;
	unsigned int reserved[2];
	struct thp_ioctl_spi_xfer_data __user *xfer_data;
};

#define THP_IO_TYPE 0xB8
#define THP_IOCTL_CMD_GET_FRAME \
	_IOWR(THP_IO_TYPE, 0x01, struct thp_ioctl_get_frame_data)
#define THP_IOCTL_CMD_RESET _IOW(THP_IO_TYPE, 0x02, u32)
#define THP_IOCTL_CMD_SET_TIMEOUT _IOW(THP_IO_TYPE, 0x03, u32)
#define THP_IOCTL_CMD_SPI_SYNC \
	_IOWR(THP_IO_TYPE, 0x04, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_FINISH_NOTIFY _IO(THP_IO_TYPE, 0x05)
#define THP_IOCTL_CMD_SET_BLOCK _IOW(THP_IO_TYPE, 0x06, u32)
#define THP_IOCTL_CMD_SET_IRQ _IOW(THP_IO_TYPE, 0x07, u32)
#define THP_IOCTL_CMD_GET_FRAME_COUNT _IOW(THP_IO_TYPE, 0x08, u32)
#define THP_IOCTL_CMD_CLEAR_FRAME_BUFFER _IOW(THP_IO_TYPE, 0x09, u32)
#define THP_IOCTL_CMD_GET_IRQ_GPIO_VALUE _IOW(THP_IO_TYPE, 0x0A, u32)
#define THP_IOCTL_CMD_SET_SPI_SPEED _IOW(THP_IO_TYPE, 0x0B, u32)
#define THP_IOCTL_CMD_SPI_SYNC_SSL_BL \
	_IOWR(THP_IO_TYPE, 0x0c, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_SET_AFE_STATUS \
	_IOW(THP_IO_TYPE, 0x0d, struct thp_ioctl_set_afe_status)
#define THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC \
	_IOWR(THP_IO_TYPE, 0x0e, struct thp_ioctl_spi_msg_package)
#define THP_IOCTL_CMD_HW_LOCK _IOW(THP_IO_TYPE, 0x0f, u32)
#define THP_IOCTL_CMD_SPI_SYNC_NO_LOCK \
	_IOWR(THP_IO_TYPE, 0x10, struct thp_ioctl_spi_sync_data)
#define THP_IOCTL_CMD_MUILTIPLE_SPI_XFRE_SYNC_NO_LOCK \
	_IOWR(THP_IO_TYPE, 0x11, struct thp_ioctl_spi_msg_package)
#define THP_IOCTL_CMD_GET_WORK_STATUS _IOWR(THP_IO_TYPE, 0x12, u32)

#define GPIO_LOW 0
#define GPIO_HIGH 1
#define DUR_RESET_HIGH 1
#define DUR_RESET_LOW 0
#define WAITQ_WAIT 0
#define WAITQ_WAKEUP 1
#define THP_MAX_FRAME_SIZE (8 * 1024 + 16)
#define THP_DEFATULT_TIMEOUT_MS 200
#define THP_SPI_SPEED_DEFAULT (20 * 1000 * 1000)

#define THP_LIST_MAX_FRAMES 20

#define THP_PROJECT_ID_LEN 10
#define THP_SYNC_DATA_MAX (4096 * 32)

#define ROI_DATA_LENGTH 49
#define ROI_DATA_STR_LENGTH (ROI_DATA_LENGTH * 10)

#define THP_SUSPEND 0
#define THP_RESUME 1

#define PROX_NOT_SUPPORT 0
#define PROX_SUPPORT 1
#define THP_PROX_ENTER 1
#define THP_PROX_EXIT 0
#define AFTER_SUSPEND_TIME 2000

#define THP_PLATFORM_HISI 0

#define SPI_POLLING_MODE 1
#define SPI_DMA_MODE 2

#define OEM_INFO_DATA_LENGTH 32

#define NORMAL_WORK_MODE 2
#define FW_UPDATE_MODE 1

#define TP_DETECT_SUCC 0
#define TP_DETECT_FAIL (-1)


#define THP_CMD_QUEUE_SIZE 20

#define THP_POWER_ON 1
#define THP_POWER_OFF 0

#define IS_TMO(t) ((t) == 0)
#define THP_WAIT_MAX_TIME 2000u

#define thp_do_time_delay(x) do { \
	if (x) \
		msleep(x); \
} while (0)

#define THP_DEV_COMPATIBLE "huawei,thp"
#define THP_SPI_DEV_NODE_NAME "thp_spi_dev"
#define THP_TIMING_NODE_NAME "thp_timing"

#if (!defined CONFIG_LCD_KIT_DRIVER) && (!defined CONFIG_HUAWEI_THP_QCOM)
extern int g_tskit_pt_station_flag;
#endif

#define THP_GET_HARDWARE_TIMEOUT 100000
#define TP_HWSPIN_LOCK_CODE 28

#define THP_KEY_UP 0
#define THP_KEY_DOWN 1

#define KEY_VOLUME_UP 115
#define KEY_VOLUME_DOWN 114
#define KEY_POWER 116
#define KEY_VOLUME_MUTE 113
#define KEY_VOLUME_TRIG 252
#define KEY_F26 196 /* ESD Key Event */
#define KEY_F27 197
#define KEY_F28 198
#define TS_GESTURE_COMMAND 0x7ff
#define TS_GESTURE_INVALID_COMMAND 0xFFFF
#define TS_GESTURE_INVALID_CONTROL_NO 0xFF
#define MAX_POSITION_NUMS 6
#define MAX_STR_LEN 32
#define MAX_THP_CMD_INFO_LEN 40960
#define PROJECTID_BIT_MASK_NUM 7
#define DELAY_AFTER_NINE_BYTE 9

#define SUSPEND_DONE 0
#define RESUME_DONE 1

#define NO_SYNC_TIMEOUT 0
#define SHORT_SYNC_TIMEOUT 5
#define LONG_SYNC_TIMEOUT 10
#define LONG_LONG_SYNC_TIMEOUT 50

#define TS_SINGLE_CLICK 251
enum ts_gesture_num {
	/* 0.Double tap:KEY_F1 */
	TS_DOUBLE_CLICK = KEY_F1,
	/* 1.Single finger slide from left to right:KEY_F2 */
	TS_SLIDE_L2R = KEY_F2,
	/* 2.Single finger slide from right to left:KEY_F3 */
	TS_SLIDE_R2L = KEY_F3,
	/* 3.Single finger slide from top to bottom:KEY_F4 */
	TS_SLIDE_T2B = KEY_F4,
	/* 4.Single finger slide from bottom to top:KEY_F5 */
	TS_SLIDE_B2T = KEY_F5,
	/* 5.Single finger slide circle:KEY_F7 */
	TS_CIRCLE_SLIDE = KEY_F7,
	/* 6.Single finger write letter c*:KEY_F8 */
	TS_LETTER_c = KEY_F8,
	/* 7.Single finger write letter e:KEY_F9 */
	TS_LETTER_e = KEY_F9,
	/* 8.Single finger write letter m:KEY_F10 */
	TS_LETTER_m = KEY_F10,
	/* 9.Single finger write letter w:KEY_F11 */
	TS_LETTER_w = KEY_F11,
	/* 10.Palm off screen:KEY_F12 */
	TS_PALM_COVERED = KEY_F12,
	/* ESD_avoiding to report KEY_F26(196) */
	TS_KEY_IRON = KEY_F26,
	TS_STYLUS_WAKEUP_TO_MEMO = KEY_F27,
	TS_STYLUS_WAKEUP_SCREEN_ON = KEY_F28,
	TS_GESTURE_INVALID = 0xFF, /* FF.No gesture */
};

#define IS_APP_ENABLE_GESTURE(x)  ((u32)(1 << x))
enum ts_gesture_enable_bit {
	GESTURE_DOUBLE_CLICK = 0,
	GESTURE_SLIDE_L2R,
	GESTURE_SLIDE_R2L,
	GESTURE_SLIDE_T2B,
	GESTURE_SLIDE_B2T,
	GESTURE_CIRCLE_SLIDE = 6,
	GESTURE_LETTER_c,
	GESTURE_LETTER_e,
	GESTURE_LETTER_m,
	GESTURE_LETTER_w,
	GESTURE_PALM_COVERED,
	GESTURE_STYLUS_WAKE_UP,
	GESTURE_SINGLE_CLICK = 15,
	GESTURE_MAX,
	GESTURE_LETTER_ENABLE = 29,
	GESTURE_SLIDE_ENABLE = 30,
};

enum thp_status_type {
	THP_STATUS_POWER = 0,
	THP_STATUS_TUI = 1,
	THP_STATUS_CHARGER = 2,
	THP_STATUS_HOLSTER = 3,
	THP_STATUS_GLOVE = 4,
	THP_STATUS_ROI = 5,
	THP_STATUS_WINDOW_UPDATE = 6,
	THP_STATUS_TOUCH_SCENE = 7,
	THP_STATUS_UDFP = 8,
	THP_STATUS_TOUCH_APPROACH = 9,
	THP_STATUS_AFE_PROXIMITY = 10,
	THP_STATUS_UDFP_SWITCH = 11,
	THP_STATUS_STYLUS = 12,
	THP_STATUS_VOLUME_SIDE = 13,
	THP_STATUS_POWER_SWITCH_CTRL = 14,
	THP_STATUS_MAX,
};

enum thp_afe_notify_event_type {
	THP_AFE_NOTIFY_FW_UPDATE,
	THP_AFE_NOTIFY_EVENT_MAX,
};

enum thp_afe_status_type {
	THP_AFE_STATUS_NO = 0,
	THP_AFE_STATUS_FW_UPDATE = 1,
	THP_AFE_STATUS_MAX,
};
enum thp_afe_fw_update_type {
	THP_AFE_FW_UPDATE_NO = 0,
	THP_AFE_FW_UPDATE_SET_SPI_COM_MODE = 1,
	THP_AFE_FW_UPDATE_MAX,
};

enum thp_sub_event_type {
	THP_SUB_EVENT_NULL = 0,
	THP_SUB_EVENT_SINGLE_CLICK = 1,
	THP_SUB_EVENT_DOUBLE_CLICK = 2,
	THP_SUB_EVENT_STYLUS_SINGLE_CLICK = 3,
	/* user want to open notepad quickly */
	THP_SUB_EVENT_STYLUS_SINGLE_CLICK_AND_PRESS = 4,
};

struct thp_ioctl_set_afe_status {
	int type;
	int status;
	int parameter;
};

enum thp_oem_info_type {
	THP_OEM_INFO_NOT_SUPPOTRT = 0,
	THP_OEM_INFO_LCD_EFFECT_TYPE = 1,
	THP_OEM_INFO_MAX_TYPE = 255,
};

enum STYLUS_WAKEUP_CTRL {
	STYLUS_WAKEUP_DISABLE = 0,
	STYLUS_WAKEUP_NORMAL_STATUS = 1,
	STYLUS_WAKEUP_LOW_FREQENCY = 2,
	STYLUS_WAKEUP_TESTMODE_IN = 3,
	STYLUS_WAKEUP_TESTMODE_OUT = 4,
	MAX_STATUS = 255,
};

enum RING_STATUS_CTRL {
	RING_STAUS_DISABLE = 0,
	RING_STAUS_ENABLE = 1,
	RING_PHONE_STATUS = 2,
	VOLUME_SIDE_LEFT,
	VOLUME_SIDE_RIGHT,
	VOLUME_SIDE_BOTH,
	RING_MAX_STATUS = 255,
};

enum FWK_POWERONOFF_REASON {
	GO_TO_SLEEP_REASON_APPLICATION = 0,
	GO_TO_SLEEP_REASON_DEVICE_ADMIN = 1,
	GO_TO_SLEEP_REASON_TIMEOUT = 2,
	GO_TO_SLEEP_REASON_LID_SWITCH = 3,
	GO_TO_SLEEP_REASON_POWER_BUTTON = 4,
	GO_TO_SLEEP_REASON_HDMI = 5,
	GO_TO_SLEEP_REASON_SLEEP_BUTTON = 6,
	GO_TO_SLEEP_REASON_ACCESSIBILITY = 7,
	GO_TO_SLEEP_REASON_FORCE_SUSPEND = 8,
	GO_TO_SLEEP_REASON_PROX_SENSOR = 100,
	GO_TO_SLEEP_REASON_WAIT_BRIGHTNESS_TIMEOUT = 101,
	GO_TO_SLEEP_REASON_PHONE_CALL = 102,
	WAKE_REASON_UNKNOWN = 65536,
	WAKE_REASON_POWER_BUTTON = 1 + 65536,
	WAKE_REASON_APPLICATION = 2 + 65536,
	WAKE_REASON_PLUGGED_IN = 3 + 65536,
	WAKE_REASON_GESTURE = 4 + 65536,
	WAKE_REASON_CAMERA_LAUNCH = 5 + 65536,
	WAKE_REASON_WAKE_KEY = 6 + 65536,
	WAKE_REASON_WAKE_MOTION = 7 + 65536,
	WAKE_REASON_HDMI = 8 + 65536,
	WAKE_REASON_LID = 9 + 65536,
	WAKE_REASON_PROXIMITY = 100 + 65536,
	WAKE_REASON_FINGER_PRINT = 101 + 65536,
	WAKE_REASON_PICKUP = 102 + 65536,
};

enum POWER_STATUS_CTRL {
	POWER_STATUS_NULL = 0,
	POWER_KEY_ON_CTRL = 1,
	FINGERPRINT_ON_CTRL,
	PROXIMITY_ON_CTRL,
	AUTO_OR_OTHER_ON_CTRL,
	POWER_KEY_OFF_CTRL,
	PROXIMITY_OFF_CTRL,
	POWERON_TIMEOUT_OFF_CTRL,
	AUTO_OR_OTHER_OFF_CTRL,
	POWER_MAX_STATUS,
};

struct thp_frame {
	struct list_head list;
#ifdef THP_NOVA_ONLY
	u8 frame[NT_MAX_FRAME_SIZE];
#else
	u8 frame[THP_MAX_FRAME_SIZE];
#endif
	struct timeval tv;
};

#if defined(CONFIG_HUAWEI_DSM)
struct host_dsm_info {
	int constraints_SPI_status;
};
#endif

#if defined(CONFIG_TEE_TUI)
struct thp_tui_data {
	char project_id[THP_PROJECT_ID_LEN + 1];
	unsigned char enable;
	u16 frame_data_addr;
};

extern struct thp_tui_data thp_tui_info;
#endif

struct thp_device;

struct thp_device_ops {
	int (*init)(struct thp_device *tdev);
	int (*detect)(struct thp_device *tdev);
	int (*get_frame)(struct thp_device *tdev, char *buf, unsigned int len);
	int (*get_project_id)(struct thp_device *tdev,
			char *buf, unsigned int len);
	int (*resume)(struct thp_device *tdev);
	int (*after_resume)(struct thp_device *tdev);
	int (*suspend)(struct thp_device *tdev);
	void (*exit)(struct thp_device *tdev);
	int (*spi_transfer)(const char *tx_buf, char *rx_buf, unsigned int len);
	int (*afe_notify)(struct thp_device *tdev, unsigned long event);
	int (*set_fw_update_mode)(struct thp_device *tdev,
			struct thp_ioctl_set_afe_status set_afe_status);
	int (*chip_wakeup_gesture_enable_switch)(struct thp_device *tdev,
			u8 switch_value);
	int (*chip_wrong_touch)(struct thp_device *tdev);
	int (*chip_gesture_report)(struct thp_device *tdev,
			unsigned int *gesture_wakeup_value);
	int (*spi_transfer_one_byte_bootloader)(
			struct thp_device *tdev,
			const char *const tx_buf,
			char *const rx_buf,
			const unsigned int buf_len);
#ifdef CONFIG_HUAWEI_SHB_THP
	int (*switch_int_sh)(struct thp_device *tdev);
	int (*switch_int_ap)(struct thp_device *tdev);
#endif
};

struct thp_spi_config {
	u32 max_speed_hz;
	u16 mode;
	u8 bits_per_word;
	u8 bus_id;
	struct pl022_config_chip pl022_spi_config;
};

struct thp_timing_config {
	u32 boot_reset_hi_delay_ms;
	u32 boot_reset_low_delay_ms;
	u32 boot_reset_after_delay_ms;
	u32 resume_reset_after_delay_ms;
	u32 suspend_reset_after_delay_ms;
	u32 spi_sync_cs_hi_delay_ns;
	u32 spi_sync_cs_low_delay_ns;
	u32 spi_sync_cs_low_delay_us;
	u32 boot_vcc_on_after_delay_ms;
	u32 boot_vddio_on_after_delay_ms;
	u32 spi_transfer_delay_us;
};

struct thp_test_config {
	u32 pt_station_test;
};

struct thp_gpios {
	int irq_gpio;
	int rst_gpio;
	int cs_gpio;
};

enum thp_work_status_type {
	THP_WORK_STATUS_UNUSED = 0,
	THP_WORK_STATUS_SUSPENDED = 1,
	THP_WORK_STATUS_RESUMED = 2,
	THP_WORK_STATUS_INVALID_TYPE = 0xff,
};

struct thp_window_info {
	int x0;
	int y0;
	int x1;
	int y1;
};

enum thp_sensorhub_cmd {
	THP_SENSORHUB_CMD_NULL = 0,
	THP_FINGER_PRINT_EVENT = 1,
	THP_RING_EVENT = 2,
	THP_ALGO_SCREEN_OFF_INFO = 3,
	THP_AUXILIARY_DATA = 4,
};

struct thp_shb_info {
	enum thp_sensorhub_cmd cmd_type;
	unsigned int cmd_len;
	void *cmd_addr;
};

struct thp_scene_info {
	int type;
	int status;
	int parameter;
};

struct thp_udfp_status {
	unsigned int udfp_enable;
	unsigned int udfp_switch;
};

enum thp_power_type {
	THP_POWER_UNUSED = 0,
	THP_POWER_LDO = 1,
	THP_POWER_GPIO = 2,
	THP_POWER_PMIC = 3,
	THP_POWER_INVALID_TYPE,
};

enum thp_power_id {
	THP_IOVDD = 0,
	THP_VCC = 1,
	THP_POWER_ID_MAX,
};
enum ts_sleep_mode {
	TS_POWER_OFF_MODE = 0,
	TS_GESTURE_MODE,
};

struct thp_pmic_power_supply {
	int ldo_num;
	int value;
	int pmic_num;
};

struct thp_power_supply {
	int use_count;
	int type;
	int gpio;
	int ldo_value;
	struct thp_pmic_power_supply pmic_power;
	struct regulator *regulator;
};

struct thp_device {
	char *ic_name;
	char *dev_node_name;
	struct thp_device_ops *ops;
	struct spi_device *sdev;
	struct thp_core_data *thp_core;
	char *tx_buff;
	char *rx_buff;
	struct thp_timing_config timing_config;
	struct thp_test_config test_config;
	struct thp_gpios *gpios;
	void *private_data;
};


struct thp_easy_wakeup_info {
	enum ts_sleep_mode sleep_mode;
	int off_motion_on;
	unsigned int easy_wakeup_gesture;
	unsigned char easy_wakeup_fastrate;
	unsigned int easywake_position[MAX_POSITION_NUMS];
};

enum thp_timeout_flag {
	TS_TIMEOUT = 0,
	TS_NOT_TIMEOUT,
	TS_UNDEF = 255,
};

struct thp_cmd_param {
	union {
		struct thp_device *dev;
	} pub_params;
	void *prv_params;
};

enum thp_cmd {
	TS_INT_PROCESS = 0,
	TS_CHIP_DETECT,
	TS_INVAILD_CMD = 255,
};

struct thp_cmd_sync {
	atomic_t timeout_flag;
	struct completion done;
};

struct thp_cmd_node {
	enum thp_cmd command;
	struct thp_cmd_sync *sync;
	struct thp_cmd_param cmd_param;
};

struct thp_cmd_queue {
	int wr_index;
	int rd_index;
	int cmd_count;
	int queue_size;
	spinlock_t spin_lock;
	struct thp_cmd_node ring_buff[THP_CMD_QUEUE_SIZE];
};

struct thp_core_data {
	struct spi_device *sdev;
	struct task_struct *thp_task;
	struct thp_cmd_queue queue;
	struct platform_device *thp_platform_dev;
	struct thp_device *thp_dev;
	struct device_node *thp_node;
	struct notifier_block lcd_notify;
	struct thp_frame frame_list;
	struct thp_spi_config spi_config;
	struct kobject *thp_obj;
	struct platform_device *ts_dev;
	struct mutex mutex_frame;
	struct mutex irq_mutex;
	struct mutex thp_mutex;
	struct mutex spi_mutex;
	struct mutex status_mutex;
	struct hwspinlock *hwspin_lock;
	struct wakeup_source thp_wake_lock;
	struct thp_power_supply thp_powers[THP_POWER_ID_MAX];
	int project_in_tp;
	char *project_id_dummy;
	atomic_t register_flag;
	u32 status;
	int open_count;
	u32 frame_count;
	unsigned int frame_size;
	bool irq_enabled;
	unsigned int invalid_irq_num;
	int irq;
	unsigned int irq_flag;
	unsigned int fast_booting_solution;
	u8 *frame_read_buf;
	u8 frame_waitq_flag;
	wait_queue_head_t frame_waitq;
	u8 thp_ta_waitq_flag;
	wait_queue_head_t thp_ta_waitq;
	wait_queue_head_t suspend_resume_waitq;
	u8 suspend_resume_waitq_flag;
	u8 reset_flag;
	unsigned int timeout;
	struct thp_gpios gpios;
	bool suspended;
	bool early_suspended;
	bool work_status;
#if defined(CONFIG_HUAWEI_DSM)
	struct host_dsm_info dsm_info;
#endif
#if defined(THP_CHARGER_FB)
	struct notifier_block charger_detect_notify;
#endif
	char project_id[THP_PROJECT_ID_LEN + 1];
	const char *ic_name;
	const char *vendor_name;
	int get_frame_block_flag;
	short roi_data[ROI_DATA_LENGTH];
	bool is_udp;
	unsigned int need_huge_memory_in_spi;
	unsigned int self_control_power;
	struct thp_window_info window;
	struct thp_scene_info scene_info;
	struct thp_udfp_status udfp_status;
	struct pinctrl *pctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_idle;
	int support_pinctrl;
	u32 pinctrl_init_enable;
	u32 proximity_support;
	u32 platform_type;
	u32 supported_func_indicater;
	u32 use_hwlock;
	u32 delay_work_for_pm;
	int event;
	u8 thp_event_waitq_flag;
	wait_queue_head_t thp_event_waitq;
	bool event_flag;
	bool need_work_in_suspend;
	bool prox_cache_enable;
	struct timeval tp_suspend_record_tv;
	u32 support_gesture_mode;
	u32 support_ring_feature;
	u32 ring_switch;
	u32 power_switch; /* save power onoff command by any event */
	u32 double_click_switch;
	u32 single_click_switch;
	u32 phone_status;
	u32 volume_side_status;
	u32 hide_product_info_en;
	u32 support_vendor_ic_type;
	u32 protect_for_irq_storm;
	u32 support_oem_info;
	u32 support_fingerprint_switch;
	u32 edit_product_in_project_id;
	char *product;
	u32 support_extra_key_event_input;
	char oem_info_data[OEM_INFO_DATA_LENGTH + 1];
	u32 pen_supported;
	struct thp_easy_wakeup_info easy_wakeup_info;
	struct mutex thp_wrong_touch_lock;
	u16 frame_data_addr;
	u32 support_shb_thp;
	u32 support_shb_thp_log;
	int stylus_status;
	u32 poweroff_function_status;
	unsigned char sleep_mode; /* for record the switch command from framework */
};


extern u8 g_thp_log_cfg;

#define HWLOG_TAG THP
HWLOG_REGIST();
#define THP_LOG_INFO(x...) _hwlog_info(HWLOG_TAG, ##x)
#define THP_LOG_ERR(x...) _hwlog_err(HWLOG_TAG, ##x)
#define THP_LOG_DEBUG(x...) do { \
		if (g_thp_log_cfg) \
			_hwlog_info(HWLOG_TAG, ##x); \
	} while (0)

#ifndef CONFIG_HUAWEI_THP_QCOM
int hostprocessing_get_project_id(char *out);
#endif
int hostprocessing_get_project_id_for_udp(char *out);
int thp_register_dev(struct thp_device *dev);
int thp_parse_spi_config(struct device_node *spi_cfg_node,
		struct thp_core_data *cd);
struct thp_core_data *thp_get_core_data(void);
int thp_parse_timing_config(struct device_node *timing_cfg_node,
		struct thp_timing_config *timing);
int is_pt_test_mode(struct thp_device *tdev);
void thp_spi_cs_set(u32 control);
int thp_daemeon_suspend_resume_notify(int status);

int thp_set_status(int type, int status);
int thp_get_status(int type);
u32 thp_get_status_all(void);
int thp_parse_feature_config(struct device_node *thp_node,
		struct thp_core_data *cd);
int thp_parse_trigger_config(struct device_node *thp_node,
		struct thp_core_data *cd);

#if defined(CONFIG_TEE_TUI)
int spi_exit_secos(unsigned int spi_bus_id);
int spi_init_secos(unsigned int spi_bus_id);
#endif

int thp_spi_sync(struct spi_device *spi, struct spi_message *message);
int thp_power_supply_get(enum thp_power_id power_id);
int thp_power_supply_put(enum thp_power_id power_id);
int thp_power_supply_ctrl(enum thp_power_id power_id,
		int status, unsigned int delay_ms);
int thp_bus_lock(void);
void thp_bus_unlock(void);
int is_valid_project_id(const char *id);
int thp_set_spi_com_mode(struct thp_core_data *cd, u8 spi_com_mode);
void thp_inputkey_report(unsigned int gesture_wakeup_value);
void thp_input_pen_report(unsigned int pen_event_value);
int thp_set_spi_max_speed(unsigned int speed);
int thp_parse_pinctrl_config(struct device_node *spi_cfg_node,
		struct thp_core_data *cd);

int thp_init_sysfs(struct thp_core_data *cd);
void thp_sysfs_release(struct thp_core_data *cd);
int thp_put_one_cmd(struct thp_cmd_node *cmd, int timeout);
void thp_send_detect_cmd(struct thp_device *dev, int timeout);

#endif /* _THP_H_ */

