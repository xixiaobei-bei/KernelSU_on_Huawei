#ifndef _GTX8_BRL_H_
#define _GTX8_BRL_H_

#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/completion.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <asm/unaligned.h>

#ifdef CONFIG_OF
#include <linux/regulator/consumer.h>
#include <linux/of_gpio.h>
#endif
#include <huawei_ts_kit.h>


#define GTP_DRIVER_VERSION "v1.0<2020/07/03>"

#define LCD_PANEL_INFO_MAX_LEN 128
#define LCD_PANEL_TYPE_DEVICE_NODE_NAME "huawei,lcd_panel_type"

#define GOODIX_VENDER_NAME "goodix"
#define GTX8_OF_NAME "gtx8"
#define GTX8_9886_CHIP_ID "9886"
#define GTX8_FW_SD_NAME "ts/touch_screen_firmware.bin"
#define GTX8_CONFIG_FW_SD_NAME "ts/gtx8_cfg.bin"
#define GOODIX_9897_NOCODE "NOCODE"
#define TS_CFG_BIN_HEAD_LEN (sizeof(struct goodix_cfg_bin_head) + 6)
#define TS_PKG_CONST_INFO_LEN (sizeof(struct goodix_cfg_pkg_const_info))
#define TS_PKG_REG_INFO_LEN (sizeof(struct goodix_cfg_pkg_reg_info))
#define TS_PKG_HEAD_LEN (TS_PKG_CONST_INFO_LEN + TS_PKG_REG_INFO_LEN)
#define GTX8_PRODUCT_ID_LEN 4
#define SWITCH_OFF 0
#define SWITCH_ON 1
#define GOODIX_CFG_MAX_SIZE 4096
#define GOODIX_TRANSFER_MAX_SIZE 4096
#define GTX8_DOZE_ENABLE_DATA 0xCC
#define GTX8_DOZE_DISABLE_DATA 0xAA
#define GTX8_DOZE_CLOSE_OK_DATA 0xBB
#define GTX8_RETRY_NUM_3 3
#define GTX8_RETRY_NUM_10 10
#define GTX8_RETRY_NUM_20 20
#define GTX8_RETRY_NUM_100 100
#define GTX8_CFG_HEAD_LEN 4
#define GTX8_CFG_MAX_LEN 1024
#define GTX8_POWER_CONTRL_BY_SELF 1
#define GTX8_POWER_BY_LDO 1
#define GTX8_SMALL_OR_LARGE_CFG_LEN 32
#define GTX8_CFG_BAG_NUM_INDEX 2
#define GTX8_CFG_BAG_START_INDEX 4
#define GTX8_FW_NAME_LEN 64

#define GTX8_EXIST 1
#define GTX8_NOT_EXIST 0

#define GTX8_ABS_MAX_VALUE 255
#define GTP_MAX_TOUCH TS_MAX_FINGER
#define BYTES_PER_COORD 8
#define BYTES_PER_EDGE 6
#define GTX8_EDGE_DATA_SIZE 20
#define DATA_INFO_MAX_FINGER_NUM 5
#define DATA_INFO_BYTES_PER_FINGER 9

#define GTX8_RESET_SLEEP_TIME 100
#define GTX8_RESET_PIN_D_U_TIME 2000

#define GTX8_HOTKNOT_EVENT 0x10
#define GTX8_GESTURE_EVENT 0x20
#define GTX8_REQUEST_EVENT 0x40
#define GTX8_TOUCH_EVENT 0x80

#define REQUEST_HANDLED 0x00
#define REQUEST_CONFIG 0x01
#define REQUEST_BAKREF 0x02
#define REQUEST_RESET 0x03
#define REQUEST_MAINCLK 0x04
#define REQUEST_RELOADFW 0x05
#define REQUEST_IDLE 0x05
#define SYNC_VAL_WRITE_LEN 2

#define SLEEP_MODE 0x01
#define GESTURE_MODE 0x02

/*GTX8 CMD*/
#define GTX8_CMD_NORMAL 0x00
#define GTX8_CMD_RAWDATA 0x01
#define GTX8_CMD_SLEEP 0x05
#define GTX8_CMD_IDLE_PERMIT 0x11
#define GTX8_CMD_GESTURE 0x08
#define GTX8_CMD_GLOVE_ENBALE 0x20
#define GTX8_CMD_GLOVE_DISBALE 0x21
#define GTX8_CMD_HOLSTER_ENBALE 0x24
#define GTX8_CMD_HOLSTER_DISBALE 0x25
#define GTX8_CMD_START_SEND_CFG 0x80
#define GTX8_CMD_END_SEND_CFG 0x83
#define GTX8_CMD_SEND_SMALL_CFG 0x81
#define GTX8_CMD_READ_CFG 0x86
#define GTX8_CMD_CFG_READY 0x85
#define GTX8_CMD_COMPLETED 0xFF

/* Register define */
#define GTP_REG_ESD_TICK_W 0x30F3
#define GTP_REG_ESD_TICK_R 0x3103
#define GTP_REG_SENSOR_NUM 0x5473
#define GTP_REG_DRIVER_GROUP_A_NUM 0x5477
#define GTP_REG_DRIVER_GROUP_B_NUM 0x5478
#define GTP_RAWDATA_BASE_ADDR 0x8FA0
#define GTP_DIFFDATA_BASE_ADDR 0x9D20
#define GTP_REG_CFG_ADDR 0x6F78
#define GTP_REG_CMD 0x6F68
#define GTP_REG_COOR 0x4100
#define GTP_REG_PID 0x4535
#define GTP_REG_VID 0x453D
#define GTP_REG_SENSOR_ID 0x4541
#define GTP_REG_VERSION_BASE 0x452C
#define GTP_REG_FW_REQUEST 0x6F6D
#define GTP_REG_DATA_INFO 0x4190

#define GTP_PID_LEN 4
#define GTP_VID_LEN 4
#define GTP_VERSION_LEN 72
#define GTP_SENSOR_ID_MASK 0x0F

#define HW_REG_CPU_CTRL 0x2180
#define HW_REG_DSP_MCU_POWER 0x2010
#define HW_REG_RESET 0x2184
#define HW_REG_SCRAMBLE 0x2218
#define HW_REG_BANK_SELECT 0x2048
#define HW_REG_ACCESS_PATCH0 0x204D
#define HW_REG_EC_SRM_START 0x204F
#define HW_REG_ISP_RUN_FLAG 0x6006
#define HW_REG_ISP_ADDR 0xC000
#define HW_REG_SUBSYS_TYPE 0x6020
#define HW_REG_FLASH_FLAG 0x6022


/* cfg parse from bin */
#define CFG_BIN_SIZE_MIN 279
#define BIN_CFG_START_LOCAL 6
#define MODULE_NUM 22
#define CFG_NUM 23
#define CFG_INFO_BLOCK_BYTES 8
#define CFG_HEAD_BYTES 32

/* GTX8 cfg name */
#define GTX8_NORMAL_CONFIG "normal_config"
#define GTX8_TEST_CONFIG "tptest_config"
#define GTX8_NORMAL_NOISE_CONFIG "normal_noise_config"
#define GTX8_GLOVE_CONFIG "glove_config"
#define GTX8_GLOVE_NOISE_CONFIG "glove_noise_config"
#define GTX8_HOLSTER_CONFIG "holster_config"
#define GTX8_HOLSTER_NOISE_CONFIG "holster_noise_config"
#define GTX8_NOISE_TEST_CONFIG "tpnoise_test_config"

#define NORMAL_PKG 0x01
#define NORMAL_NOISE_PKG 0x02
#define GLOVE_PKG 0x03
#define GLOVE_NOISE_PKG 0x04
#define HOLSTER_PKG 0x05
#define HOLSTER_NOISE_PKG 0x06
#define TEST_PKG 0x07
#define NOISE_TEST_PKG 0x08

#define GTX8_NEED_SLEEP 1
#define GTX8_NOT_NEED_SLEEP 0

#define GTX8_CONFIG_REFRESH_DATA 0x01

/* send config delay time /ms */
#define SEND_CFG_FLASH 150
#define SEND_CFG_RAM 60

/* fw update */
#define FW_SUBSYS_MAX_NUM 28
#define FW_UPDATE_0_PERCENT 0
#define FW_UPDATE_10_PERCENT 10
#define FW_UPDATE_20_PERCENT 20
#define FW_UPDATE_100_PERCENT 100
#define FW_UPDATE_RETRY 2
#define FW_SUBSYS_INFO_OFFSET 36
#define FW_HEADER_SIZE 256
#define FW_SUBSYS_INFO_SIZE 10
#define ISP_MAX_BUFFERSIZE (1024 * 4)

/* roi data*/
#define ROI_STA_REG 0x141E0
#define ROI_HEAD_LEN 4
#define ROI_DATA_REG (ROI_STA_REG + ROI_HEAD_LEN)
#define ROI_READY_MASK 0x10
#define ROI_TRACKID_MASK 0x0f
#define GTX8_BIT_MASK 0x0f
#define GTX8_ROI_SRC_STATUS_INDEX 2
#define ROI_COLS 7
#define ROI_ROWS 7

/* gesture type */
#define GESTURE_DOUBLE_CLICK 0xCC
#define GESTURE_UP_SLIDE 0xBA
#define GESTURE_DOWN_SLIDE 0xAB
#define GESTURE_LEFT_SLIDE 0xBB
#define GESTURE_RIGHT_SLIDE 0xAA

/* edge info */
#define GOODIX_EDGE_ADDR 0x10392

/* Regiter for short  test*/
#define SHORT_STATUS_REG 0x5095
#define WATCH_DOG_TIMER_REG 0x20B0

#define TXRX_THRESHOLD_REG 0x8408
#define GNDVDD_THRESHOLD_REG 0x840A
#define ADC_DUMP_NUM_REG 0x840C

#define DIFFCODE_SHORT_THRESHOLD 0x880A

#define DRIVER_CH0_REG_9PT 0x8101
#define	SENSE_CH0_REG_9PT 0x80E1
#define DRIVER_CH0_REG_9P 0x80fc
#define	SENSE_CH0_REG_9P 0x80dc

#define DRV_CONFIG_REG 0x880E
#define SENSE_CONFIG_REG 0x882E
#define DRVSEN_CHECKSUM_REG 0x884E
#define CONFIG_REFRESH_REG 0x813E
#define	ADC_RDAD_VALUE 0x96
#define GNDAVDD_SHORT_VALUE 16
#define ADC_DUMP_NUM 200
#define	DIFFCODE_SHORT_VALUE 0x14

/* Regiter for rawdata test*/
#define GTP_RAWDATA_ADDR_9886 0x8FA0
#define GTP_NOISEDATA_ADDR_9886 0x9D20
#define GTP_BASEDATA_ADDR_9886 0xA980
#define GTP_SELF_RAWDATA_ADDR_9886 0x4C0C
#define GTP_SELF_NOISEDATA_ADDR_9886 0x4CA4

#define GTP_RAWDATA_ADDR_6861 0x9078
#define GTP_NOISEDATA_ADDR_6861 0x9B92
#define GTP_BASEDATA_ADDR_6861 0xB0DE

#define GTP_RAWDATA_ADDR_6862 0x9078
#define GTP_NOISEDATA_ADDR_6862 0x9B92
#define GTP_BASEDATA_ADDR_6862 0xB0DE

/* may used only in test.c */
#define SHORT_TESTEND_REG 0x8400
#define TEST_RESTLT_REG 0x8401
#define TX_SHORT_NUM 0x8402

#define DIFF_CODE_REG 0xA97A
#define DRV_SELF_CODE_REG 0xA8E0
#define TX_SHORT_NUM_REG 0x8802

#define MAX_DRV_NUM 21
#define MAX_SEN_NUM 42

#define MAX_DRV_NUM_9886 40
#define MAX_SEN_NUM_9886 36

#define MAX_DRV_NUM_6861 47
#define MAX_SEN_NUM_6861 36

#define MAX_DRV_NUM_6862 47
#define MAX_SEN_NUM_6862 29

/*  end  */

#define HOLD_MCU_STAT 0x01
#define ENABLE_MISCTL_CLOCK 0x08
#define OPEN_ESD_KEY 0x95
#define OPEN_ESD_OK 0x01
#define DISABLE_WATCHING_DOG 0x00
#define SET_BOOT_OPTION 0x55

/* errno define */
#define EBUS 1000
#define ETIMEOUT 1001
#define ECHKSUM 1002
#define EMEMCMP 1003

#define get_u32(a) ((u32)brl_get_uint((u8 *)(a), 4))
#define get_u16(a) ((u16)brl_get_uint((u8 *)(a), 2))

#define GOODIX_EXCHANGE_X 1
#define GOODIX_EXCHANGE_Y 0
#define GOODIX_REG_ESD 0x10168
#define FW_VERSION_INFO_ADDR 0x1000C

#define MAX_SCAN_FREQ_NUM 5
#define MAX_SCAN_RATE_NUM 5
#define MAX_FREQ_NUM_STYLUS 8
#define MAX_STYLUS_SCAN_FREQ_NUM 6

#define CONFIG_CMD_LEN 4
#define CONFIG_CMD_START 0x04
#define CONFIG_CMD_WRITE 0x05
#define CONFIG_CMD_EXIT 0x06
#define CONFIG_CMD_READ_START 0x07
#define CONFIG_CMD_READ_EXIT 0x08

#define GOODIX_SLEEP_CMD 0x84
#define GOODIX_GESTURE_CMD 0x12
#define GOODIX_FREQ_CMD 0xB1
#define GOODIX_UPDATE_REF_CMD 0x83
#define GOODIX_CMD_RAWDATA 0x02
#define GOODIX_CMD_DIFFDATA 0x01
#define GOODIX_CMD_COORD 0x00
#define HOLD_CPU_REG_W 0x0002
#define HOLD_CPU_REG_R 0x2000
#define MISCTL_REG 0xD807
#define ESD_KEY_REG 0xCC58
#define WATCH_DOG_REG 0xCC54
#define REG_W_BUFF_LEN 2

#define ISP_RAM_ADDR 0x18400
#define ISP_MBIST_ADDR 0x20400
#define HW_REG_CPU_RUN_FROM 0x10000
#define FLASH_CMD_REG 0x10400
#define HW_REG_ISP_BUFFER 0x10410

#define ISP_BANK0_SIZE 3072
#define ISP_BANK1_SIZE 1024

#define REG_MBIST_TEST_CTRL 0xD8B8

#define PATCH_VID_MAJOR 3
#define PATCH_VID_MINOR 2
#define NORMAL_CFG_DATA 34

#define CHECKSUM_LEN 2
#define USLEEP_START 5000
#define USLEEP_END 5100
#define VAL_LEN 1
#define GESTURE_CMD_LEN 6
#define DISABLE_DOUBLE_CLICK_CMD 0x80
#define DISABLE_SINGLE_CLICK_CMD 0x10

#pragma pack(1)
struct brl_ts_version {
	u8 rom_pid[6];               /* rom PID */
	u8 rom_vid[3];               /* Mask VID */
	u8 rom_vid_reserved;
	u8 patch_pid[8];              /* Patch PID */
	u8 patch_vid[4];              /* Patch VID */
	u8 patch_vid_reserved;
	u8 sensor_id;
	u8 reserved[2];
	u16 checksum;
};

struct brl_ic_info_version {
	u8 info_customer_id;
	u8 info_version_id;
	u8 ic_die_id;
	u8 ic_version_id;
	u32 config_id;
	u8 config_version;
	u8 frame_data_customer_id;
	u8 frame_data_version_id;
	u8 touch_data_customer_id;
	u8 touch_data_version_id;
	u8 reserved[3];
};

struct brl_ic_info_feature { /* feature info*/
	u16 freqhop_feature;
	u16 calibration_feature;
	u16 gesture_feature;
	u16 side_touch_feature;
	u16 stylus_feature;
};

struct brl_ic_info_param { /* param */
	u8 drv_num;
	u8 sen_num;
	u8 button_num;
	u8 force_num;
	u8 active_scan_rate_num;
	u16 active_scan_rate[MAX_SCAN_RATE_NUM];
	u8 mutual_freq_num;
	u16 mutual_freq[MAX_SCAN_FREQ_NUM];
	u8 self_tx_freq_num;
	u16 self_tx_freq[MAX_SCAN_FREQ_NUM];
	u8 self_rx_freq_num;
	u16 self_rx_freq[MAX_SCAN_FREQ_NUM];
	u8 stylus_freq_num;
	u16 stylus_freq[MAX_FREQ_NUM_STYLUS];
};

struct brl_ic_info_misc { /* other data */
	u32 cmd_addr;
	u16 cmd_max_len;
	u32 cmd_reply_addr;
	u16 cmd_reply_len;
	u32 fw_state_addr;
	u16 fw_state_len;
	u32 fw_buffer_addr;
	u16 fw_buffer_max_len;
	u32 frame_data_addr;
	u16 frame_data_head_len;
	u16 fw_attr_len;
	u16 fw_log_len;
	u8 pack_max_num;
	u8 pack_compress_version;
	u16 stylus_struct_len;
	u16 mutual_struct_len;
	u16 self_struct_len;
	u16 noise_struct_len;
	u32 touch_data_addr;
	u16 touch_data_head_len;
	u16 point_struct_len;
	u16 reserved1;
	u16 reserved2;
	u32 mutual_rawdata_addr;
	u32 mutual_diffdata_addr;
	u32 mutual_refdata_addr;
	u32 self_rawdata_addr;
	u32 self_diffdata_addr;
	u32 self_refdata_addr;
	u32 iq_rawdata_addr;
	u32 iq_refdata_addr;
	u32 im_rawdata_addr;
	u16 im_readata_len;
	u32 noise_rawdata_addr;
	u16 noise_rawdata_len;
	u32 stylus_rawdata_addr;
	u16 stylus_rawdata_len;
	u32 noise_data_addr;
	u32 esd_addr;
};

struct brl_ic_info {
	u16 length;
	struct brl_ic_info_version version;
	struct brl_ic_info_feature feature;
	struct brl_ic_info_param parm;
	struct brl_ic_info_misc misc;
};
#pragma pack()

#define MAX_CMD_DATA_LEN 10
#define MAX_CMD_BUF_LEN 16
#pragma pack(1)
struct brl_ts_cmd {
	union {
		struct {
			u8 state;
			u8 ack;
			u8 len;
			u8 cmd;
			u8 data[MAX_CMD_DATA_LEN];
		};
		u8 buf[MAX_CMD_BUF_LEN];
	};
};

struct brl_flash_cmd {
	union {
	struct {
		u8 status;
		u8 ack;
		u8 len;
		u8 cmd;
		u8 fw_type;
		u16 fw_len;
		u32 fw_addr;
	};
	u8 buf[16];
	};
};

struct brl_config_head {
	union {
		struct {
			u8 panel_name[8];
			u8 fw_pid[8];
			u8 fw_vid[4];
			u8 project_name[8];
			u8 file_ver[2];
			u32 cfg_id;
			u8 cfg_ver;
			u8 cfg_time[8];
			u8 reserved[15];
			u8 flag;
			u16 cfg_len;
			u8 cfg_num;
			u16 checksum;
		};
		u8 buf[64];
	};
};

struct test_result_t {
	uint8_t result;
	uint8_t drv_drv_num;
	uint8_t sen_sen_num;
	uint8_t drv_sen_num;
	uint8_t drv_gnd_avdd_num;
	uint8_t sen_gnd_avdd_num;
	uint16_t checksum;
};
#pragma pack()

u32 brl_get_uint(u8 *buffer, int len);

/*
 * brl_ts_feature - special touch feature
 * @TS_FEATURE_NONE: no special touch feature
 * @TS_FEATURE_GLOVE: glove feature
 * @TS_FEATURE_HOLSTER: holster feature
 */
enum brl_ts_feature {
	TS_FEATURE_NONE = 0,
	TS_FEATURE_GLOVE,
	TS_FEATURE_HOLSTER,
	TS_FEATURE_CHARGER,
};

struct roi_matrix {
	short *data;
	unsigned int row;
	unsigned int col;
};

/*
 * brl_ts_roi - finger sense data structure
 * @enabled: enable/disable finger sense feature
 * @data_ready: flag to indicate data ready state
 * @roi_rows: rows of roi data
 * @roi_clos: columes of roi data
 * @mutex: mutex
 * @rawdata: rawdata buffer
 */
struct brl_ts_roi {
	bool enabled;
	bool data_ready;
	int track_id;
	unsigned int roi_rows;
	unsigned int roi_cols;
	struct mutex mutex;
	short *rawdata;
	struct roi_matrix roi_mat_src;
	struct roi_matrix roi_mat_dst;
};

/**
 * fw_subsys_info - subsytem firmware infomation
 * @type: sybsystem type
 * @size: firmware size
 * @flash_addr: flash address
 * @data: firmware data
 */
struct fw_subsys_info {
	u8 type;
	u32 size;
	u32 flash_addr;
	const u8 *data;
};

#pragma pack(1)
/**
 * firmware_info
 * @size: fw total length
 * @checksum: checksum of fw
 * @hw_pid: mask pid string
 * @hw_pid: mask vid code
 * @fw_pid: fw pid string
 * @fw_vid: fw vid code
 * @subsys_num: number of fw subsystem
 * @chip_type: chip type
 * @protocol_ver: firmware packing
 *   protocol version
 * @subsys: sybsystem info
 */
struct firmware_info {
	u32 size;
	u32 checksum;
	u8 hw_pid[6];
	u8 hw_vid[3];
	u8 fw_pid[8];
	u8 fw_vid[4];
	u8 subsys_num;
	u8 chip_type;
	u8 protocol_ver;
	u8 bus_type;
	u8 flash_protect;
	u8 reserved[2];
	struct fw_subsys_info subsys[FW_SUBSYS_MAX_NUM];
};
#pragma pack()

/**
 * firmware_data - firmware data structure
 * @fw_info: firmware infomation
 * @firmware: firmware data structure
 */
struct firmware_data {
	struct firmware_info fw_info;
	const struct firmware *firmware;
};

enum update_status {
	UPSTA_NOTWORK = 0,
	UPSTA_PREPARING,
	UPSTA_UPDATING,
	UPSTA_ABORT,
	UPSTA_SUCCESS,
	UPSTA_FAILED
};

/**
 * fw_update_ctrl - sturcture used to control the
 *  firmware update process
 * @status: update status
 * @progress: indicate the progress of update
 * @fw_data: firmware data
 * @ts_dev: touch device
 * @fw_name: firmware name
 * @attr_fwimage: sysfs bin attrs, for storing fw image
 * @fw_from_sysfs: whether the firmware image is loadind
 *		from sysfs
 */
struct fw_update_ctrl {
	enum update_status  status;
	unsigned int progress;
	bool force_update;
	u32 take_time; /* ms */

	struct firmware_data fw_data;
	char fw_name[GTX8_FW_NAME_LEN + 1];
	struct bin_attribute attr_fwimage;
	bool fw_from_sysfs;
};

/*
 * struct brl_ts_config - chip config data
 * @initialized: whether intialized
 * @name: name of this config
 * @lock: mutex
 * @reg_base: register base of config data
 * @length: bytes of the config
 * @delay: delay time after sending config
 * @data: config data buffer
 */
struct brl_ts_config {
	bool initialized;
	char name[MAX_STR_LEN + 1];
	struct mutex lock;
	unsigned int reg_base;
	unsigned int length;
	unsigned int delay; /*ms*/
	unsigned char data[GOODIX_CFG_MAX_SIZE];
};

struct brl_ts_data;

struct brl_ts_ops {
	int (*i2c_write)(u32 addr, u8 *buffer, u32 len);
	int (*i2c_read)(u32 addr, u8 *buffer, u32 len);
	int (*chip_reset)(void);
	int (*send_cmd)(struct brl_ts_cmd *cmd);
	int (*send_cfg)(struct brl_ts_config *config);
	int (*read_cfg)(u8 *config_data, u32 config_len);
	int (*read_version)(struct brl_ts_version *version);
	int (*parse_cfg_data)(const struct firmware *cfg_bin,
				char *cfg_type, u8 *cfg, int *cfg_len, u8 sid);
	int (*feature_resume)(struct brl_ts_data *ts);
	u32 (*get_file_config_id)(u8 *data, u32 len);
};

struct brl_ts_data {
	struct platform_device *pdev;
	struct regulator *vci;
	struct regulator *vddio;
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_suspend;
	struct pinctrl_state *pins_gesture;
	struct ts_kit_device_data *dev_data;
	struct brl_ts_ops ops;
	struct brl_ts_version hw_info;
	struct brl_ts_roi roi;
	struct brl_ts_config normal_cfg;
	struct brl_ts_config normal_noise_cfg;
	struct brl_ts_config glove_cfg;
	struct brl_ts_config glove_noise_cfg;
	struct brl_ts_config holster_cfg;
	struct brl_ts_config holster_noise_cfg;
	struct brl_ts_config test_cfg;
	struct brl_ts_config noise_test_cfg;
	struct brl_ic_info ic_info; /* gt9897 info */

	bool flip_x;
	bool flip_y;
	bool noise_env;
	bool test_mode;
	bool rawdiff_mode;
	bool rawdiff_mode_res;
	bool suspend_status;
	int tool_esd_disable;
	int max_x;
	int max_y;
	int fw_only_depend_on_lcd;
	int gtx8_roi_fw_supported;
	int support_get_tp_color;
	u32 power_self_ctrl; /*0-LCD control, 1-tp controlled*/
	u32 vci_power_type; /*0 - gpio control  1 - ldo  2 - not used*/
	u32 vddio_power_type; /*0 - gpio control  1 - ldo  2 - not used*/
	u32 create_project_id_supported;
	u32 brl_edge_add;
	u32 brl_roi_data_add;
	u32 easy_wakeup_supported;
	u32 config_flag; /* 0 - normal config; 1 - extern config*/
	char project_id[MAX_STR_LEN + 1];
	char chip_name[GTX8_PRODUCT_ID_LEN + 1];
	char lcd_panel_info[LCD_PANEL_INFO_MAX_LEN];
	char lcd_module_name[MAX_STR_LEN];
	char *write_buf;
	char fw_name_prefix[MAX_STR_LEN * 4];
	u32 tools_support;
	bool after_resume_done;
	bool game_switch;
};

/*
 * checksum helper functions
 * checksum can be u8/le16/be16/le32/be32 format
 * NOTE: the caller shoule be responsible for the
 * legality of @data and @size parameters, so be
 * careful when call these functions.
 */
static inline u8 checksum_u8(u8 *data, u32 size)
{
	u8 checksum = 0;
	u32 i = 0;

	for (i = 0; i < size; i++)
		checksum += data[i];
	return checksum;
}

static inline u16 checksum_be16(u8 *data, u32 size)
{
	u16 checksum = 0;
	u32 i = 0;

	for (i = 0; i < size; i += 2)
		checksum += be16_to_cpup((__be16 *)(data + i));
	return checksum;
}

extern struct brl_ts_data *brl_ts;
extern struct fw_update_ctrl brl_update_ctrl;

enum CHECKSUM_MODE {
	CHECKSUM_MODE_U8_LE,
	CHECKSUM_MODE_U16_LE,
};
u32 goodix_append_checksum(u8 *data, int len, int mode);
int checksum_cmp(const u8 *data, int size, int mode);

int brl_update_firmware(void);


int brl_init_tool_node(void);
int brl_get_rawdata(struct ts_rawdata_info *info, struct ts_cmd_node *out_cmd);
#endif
