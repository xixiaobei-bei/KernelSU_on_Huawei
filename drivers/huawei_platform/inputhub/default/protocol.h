/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: protocol header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <linux/types.h>
#include "protocol_ext.h"
#include "sub_cmd.h"
#ifdef CONFIG_INPUTHUB_30
#include <linux/hisi/contexthub/itf/ipc_ap_iomcu.h>
#endif

#define SUBCMD_LEN 4
#define MAX_PKT_LENGTH                       128
#define MAX_PKT_LENGTH_AP                    2560
#define MAX_LOG_LEN                          100
#define MAX_I2C_DATA_LENGTH                  50
#define MAX_SENSOR_CALIBRATE_DATA_LENGTH     60
#define MAX_VIB_CALIBRATE_DATA_LENGTH        12
#define MAX_MAG_CALIBRATE_DATA_LENGTH        12
#define MAX_GYRO_CALIBRATE_DATA_LENGTH       72
#define MAX_GYRO_TEMP_OFFSET_LENGTH          56
#define MAX_CAP_PROX_CALIBRATE_DATA_LENGTH   16
#define MAX_MAG_FOLDER_CALIBRATE_DATA_LENGTH 24
#define MAX_MAG_AKM_CALIBRATE_DATA_LENGTH    28
#define MAX_TOF_CALIBRATE_DATA_LENGTH        47
#define MAX_PS_CALIBRATE_DATA_LENGTH         52
#define MAX_ALS_CALIBRATE_DATA_LENGTH        24

/* data flag consts */
#define DATA_FLAG_FLUSH_OFFSET           0
#define DATA_FLAG_VALID_TIMESTAMP_OFFSET 1
#define FLUSH_END                        (1 << DATA_FLAG_FLUSH_OFFSET)
#define DATA_FLAG_VALID_TIMESTAMP        (1 << DATA_FLAG_VALID_TIMESTAMP_OFFSET)
#define ACC_CALIBRATE_DATA_LENGTH        15
#define GYRO_CALIBRATE_DATA_LENGTH       18
#define PS_CALIBRATE_DATA_LENGTH         13
#define ALS_CALIBRATE_DATA_LENGTH        6
#define ACC1_CALIBRATE_DATA_LENGTH       60
#define ACC1_OFFSET_DATA_LENGTH          15
#define GYRO1_CALIBRATE_DATA_LENGTH      18
#define MAX_GYRO1_CALIBRATE_DATA_LENGTH  72
#define MAX_THP_SYNC_INFO_LEN            (2 * 1024)
#ifndef CONFIG_INPUTHUB_30
/* --------------------------tag-------------------------- */
enum obj_tag {
	TAG_FLUSH_META,
	TAG_BEGIN = 0x01,
	TAG_SENSOR_BEGIN = TAG_BEGIN,
	TAG_ACCEL = TAG_SENSOR_BEGIN,
	TAG_GYRO,
	TAG_MAG,
	TAG_ALS,
	TAG_PS, /* 5 */
	TAG_LINEAR_ACCEL,
	TAG_GRAVITY,
	TAG_ORIENTATION,
	TAG_ROTATION_VECTORS,
	TAG_PRESSURE, /* 0x0a = 10 */
	TAG_HALL,
	TAG_MAG_UNCALIBRATED,
	TAG_GAME_RV,
	TAG_GYRO_UNCALIBRATED,
	TAG_SIGNIFICANT_MOTION, /* 0x0f = 15 */
	TAG_STEP_DETECTOR,
	TAG_STEP_COUNTER,
	TAG_GEOMAGNETIC_RV,
	TAG_HANDPRESS,
	TAG_FINGERSENSE, /* 0x14 = 20 */
	TAG_PHONECALL,
	TAG_CONNECTIVITY,
	TAG_OIS,
	TAG_HINGE,
	TAG_RPC, /* 0x19 = 25 should same with modem definition */
	TAG_CAP_PROX,
	TAG_MAGN_BRACKET,
	TAG_AGT,
	TAG_COLOR,
	TAG_ACCEL_UNCALIBRATED, /* 0x1e = 30 */
	TAG_TOF,
	TAG_DROP,
	TAG_POSTURE,
	TAG_EXT_HALL,
	TAG_ACC1 = 35, /* 0x23 = 35 */
	TAG_GYRO1,
	TAG_ALS1,
	TAG_MAG1,
	TAG_ALS2,
	TAG_PS1, /* 0x28 = 40 */
	TAG_CAP_PROX1,
	TAG_SOUND,
	TAG_AUX_END = TAG_SOUND,
	TAG_THERMOMETER,
	TAG_SENSOR_END = 44, /* sensor end should < 45 */
	TAG_HW_PRIVATE_APP_START = 45, /* 0x2d = 45 */
	TAG_AR = TAG_HW_PRIVATE_APP_START,
	TAG_MOTION,
	TAG_CONNECTIVITY_AGENT,
	TAG_PDR,
	TAG_CA,
	TAG_FP, /* 0x32 = 50 */
	TAG_KEY,
	TAG_AOD,
	TAG_FLP,
	TAG_ENVIRONMENT,
	TAG_LOADMONITOR, /* 0x37 = 55 */
	TAG_APP_CHRE,
	TAG_FP_UD,
	TAG_THP,
#ifdef CONFIG_CONTEXTHUB_IGS_20
	TAG_IGS = 63,
#endif
	/* APP_END should < 64, because power log used bitmap */
	TAG_HW_PRIVATE_APP_END,
	TAG_MODEM = 128, /* 0x80 = 128 */
	TAG_TP,
	TAG_SPI,
	TAG_I2C,
	TAG_UART,
	TAG_RGBLIGHT,
	TAG_BUTTONLIGHT,
	TAG_BACKLIGHT, /* 0x86 = 135 */
	TAG_VIBRATOR,
	TAG_SYS,
	TAG_LOG,
	TAG_LOG_BUFF,
	TAG_RAMDUMP, /* 0x8b = 140 */
	TAG_FAULT,
	TAG_SHAREMEM,
	TAG_SHELL_DBG,
	TAG_PD,
	TAG_I3C, /* 0x90 = 145 */
	TAG_DATA_PLAYBACK,
	TAG_CHRE,
	TAG_SENSOR_CALI,
	TAG_CELL,
	TAG_BIG_DATA,
#ifndef CONFIG_CONTEXTHUB_IGS_20
	TAG_IGS = 151, /* for igs1.0 */
#endif
	TAG_SWING_DBG = 152,
	TAG_SWING_CAM = 154,
	TAG_TIMESTAMP = 157,
	TAG_SWING_TOF = 158,
	TAG_KB = 180,
	TAG_UDI = 181, /* sensor dump & inject */
	TAG_END = 0xFF
};
#endif
/* --------------------------cmd-------------------------- */
enum obj_cmd {
	CMD_CMN_OPEN_REQ = 0x01,
	CMD_CMN_OPEN_RESP,
	CMD_CMN_CLOSE_REQ,
	CMD_CMN_CLOSE_RESP,
	CMD_CMN_INTERVAL_REQ,
	CMD_CMN_INTERVAL_RESP,
	CMD_CMN_CONFIG_REQ,
	CMD_CMN_CONFIG_RESP,
	CMD_CMN_FLUSH_REQ,
	CMD_CMN_FLUSH_RESP,
	CMD_CMN_IPCSHM_REQ,
	CMD_CMN_IPCSHM_RESP,

	CMD_DATA_REQ = 0x1f,
	CMD_DATA_RESP,

	CMD_SET_FAULT_TYPE_REQ, /* 0x21 */
	CMD_SET_FAULT_TYPE_RESP,
	CMD_SET_FAULT_ADDR_REQ,
	CMD_SET_FAULT_ADDR_RESP,

	/* SPI */
	CMD_SPI_BAUD_REQ, /* 0x25 */
	CMD_SPI_BAUD_RESP,
	CMD_SPI_TRANS_REQ,
	CMD_SPI_TRANS_RESP,

	/* I2C */
	CMD_I2C_TRANS_REQ, /* 0x29 */
	CMD_I2C_TRANS_RESP,

	/* system status */
	CMD_SYS_STATUSCHANGE_REQ, /* 0x2b */
	CMD_SYS_STATUSCHANGE_RESP,
	CMD_SYS_DYNLOAD_REQ,
	CMD_SYS_DYNLOAD_RESP,
	CMD_SYS_HEARTBEAT_REQ,
	CMD_SYS_HEARTBEAT_RESP,
	CMD_SYS_LOG_LEVEL_REQ,
	CMD_SYS_LOG_LEVEL_RESP,
	CMD_SYS_CTS_RESTRICT_MODE_REQ,
	CMD_SYS_CTS_RESTRICT_MODE_RESP,

	/* LOG */
	CMD_LOG_REPORT_REQ, /* 0x35 */
	CMD_LOG_REPORT_RESP,
	CMD_LOG_CONFIG_REQ,
	CMD_LOG_CONFIG_RESP,
	CMD_LOG_POWER_REQ,
	CMD_LOG_POWER_RESP,

	/* SHAREMEM */
	CMD_SHMEM_AP_RECV_REQ, /* 0x3b */
	CMD_SHMEM_AP_RECV_RESP,
	CMD_SHMEM_AP_SEND_REQ,
	CMD_SHMEM_AP_SEND_RESP,

	/* tag modem for cell info */
	CMD_MODEM_CELL_INFO_REQ, /* 0x3f */
	CMD_MODEM_CELL_INFO_RESP,
	CMD_MODEM_REBOOT_NOTIFY_REQ,
	CMD_MODEM_REBOOT_NOTIFY_RESP,

	/* SHELL_DBG */
	CMD_SHELL_DBG_REQ, /* 0x43 */
	CMD_SHELL_DBG_RESP,

	/* LoadMonitor */
	CMD_READ_AO_MONITOR_SENSOR, /* 0x45 */
	CMD_READ_AO_MONITOR_SENSOR_RESP,

	/* TAG_DATA_PLAYBACK */
	CMD_DATA_PLAYBACK_DATA_READY_REQ, /* 0x47 BACKPLAY */
	CMD_DATA_PLAYBACK_DATA_READY_RESP,
	CMD_DATA_PLAYBACK_BUF_READY_REQ, /* RECORD */
	CMD_DATA_PLAYBACK_BUF_READY_RESP,

	/* CHRE */
	CMD_CHRE_AP_SEND_TO_MCU, /* 0x4b */
	CMD_CHRE_AP_SEND_TO_MCU_RESP,
	CMD_CHRE_MCU_SEND_TO_AP,
	CMD_CHRE_MCU_SEND_TO_AP_RESP,

	/* BIG DATA */
	CMD_BIG_DATA_REQUEST_DATA, /* 0x4f */
	CMD_BIG_DATA_REQUEST_DATA_RESP,
	CMD_BIG_DATA_SEND_TO_AP, /* 0x51 */
	CMD_BIG_DATA_SEND_TO_AP_RESP, /* 0x52 */

	/* tag sys for hall status */
	CMD_SYS_HALL_STATUS_REQ = 0x53, /* 0x53 */
	CMD_SYS_HALL_STATUS_RESP = 0x54, /* 0x54 */

	/* tag modem for phonecall status */
	CMD_MODEM_PHONECALL_INFO_REQ = 0x55, /* 0x55 */
	CMD_MODEM_PHONECALL_INFO_RESP = 0x56, /* 0x56 */

	/* I3C */
	CMD_I3C_TRANS_REQ = 0x57,
	CMD_I3C_TRANS_RESP,

	/* tag als for ud */
	CMD_ALS_RUN_STOP_PARA_REQ = 0x59,
	CMD_ALS_RUN_STOP_PARA_RESP = 0x5a,
	CMD_ALS_BL_PARA_REQ = 0x5d,
	CMD_ALS_BL_PARA_RESP = 0x5e,
	CMD_ALS_COEF_BLOCK_REQ = 0x5f,
	CMD_ALS_COEF_BLOCK_RESP = 0x60,

	/* tag TAG_AIC_DBG for tiny pmu profile */
	CMD_DATA_PMU_PROFILE = 0x70,

	/* log buff */
	CMD_LOG_SER_REQ = 0xf1,
	CMD_LOG_USEBUF_REQ,
	CMD_LOG_BUFF_ALERT,
	CMD_LOG_BUFF_FLUSH,
	CMD_LOG_BUFF_FLUSHP,
	CMD_EXT_LOG_FLUSH,
	CMD_EXT_LOG_FLUSHP,

	CMD_SYS_TIMESTAMP_REQ = 0xF8,
	CMD_SYS_TIMESTAMP_RESP = 0xF9,

	/* modem mode */
	CMD_MODEM_MODE_REQ = 0xFA,
	CMD_MODEM_MODE_RESP,

	CMD_SYS_TIMEZONE_REQ,
	CMD_SYS_TIMEZONE_RESP,

	/* max cmd */
	CMD_ERR_RESP = 0xfe,
};

typedef enum {
	EN_OK = 0,
	EN_FAIL,
} err_no_t;

typedef enum {
	NO_RESP,
	RESP,
} obj_resp_t;

typedef enum {
	MOTION_TYPE_START,
	MOTION_TYPE_PICKUP,
	MOTION_TYPE_FLIP,
	MOTION_TYPE_PROXIMITY,
	MOTION_TYPE_SHAKE,
	MOTION_TYPE_TAP,
	MOTION_TYPE_TILT_LR,
	MOTION_TYPE_ROTATION,
	MOTION_TYPE_POCKET,
	MOTION_TYPE_ACTIVITY,
	MOTION_TYPE_TAKE_OFF,
	MOTION_TYPE_EXTEND_STEP_COUNTER,
	MOTION_TYPE_EXT_LOG, /* type 0xc */
	MOTION_TYPE_HEAD_DOWN,
	MOTION_TYPE_PUT_DOWN,
	MOTION_TYPE_REMOVE,
	MOTION_TYPE_FALL,
	MOTION_TYPE_SIDEGRIP,
	MOTION_TYPE_MOVE,
	MOTION_TYPE_FALL_DOWN,
	MOTION_TYPE_LF_END, /* 100hz sample type end */
	/* 500hz sample type start */
	MOTION_TYPE_TOUCH_LINK = 32,
	MOTION_TYPE_END, /* 500hz sample type end */
} motion_type_t;


typedef enum {
	FINGERPRINT_TYPE_START = 0x0,
	FINGERPRINT_TYPE_HUB,
	FINGERPRINT_TYPE_END,
} fingerprint_type_t;

typedef enum {
	AUTO_MODE = 0,
	FIFO_MODE,
	INTEGRATE_MODE,
	REALTIME_MODE,
	MODE_END
} obj_report_mode_t;

typedef enum {
	/* system status */
	ST_NULL = 0,
	ST_BEGIN,
	ST_POWERON = ST_BEGIN,
	ST_MINSYSREADY,
	ST_DYNLOAD,
	ST_MCUREADY,
	ST_TIMEOUTSCREENOFF,
	ST_SCREENON, /* 6 */
	ST_SCREENOFF, /* 7 */
	ST_SLEEP, /* 8 */
	ST_WAKEUP, /* 9 */
	ST_POWEROFF,
	ST_RECOVERY_BEGIN, /* for ar notify modem when iom3 recovery */
	ST_RECOVERY_FINISH, /* for ar notify modem when iom3 recovery */
	ST_END
} sys_status_t;

typedef enum {
	DUBAI_EVENT_NULL = 0,
	DUBAI_EVENT_AOD_PICKUP = 3,
	DUBAI_EVENT_AOD_PICKUP_NO_FINGERDOWN = 4,
	DUBAI_EVENT_AOD_TIME_STATISTICS = 6,
	DUBAI_EVENT_FINGERPRINT_ICON_COUNT = 7,
	DUBAI_EVENT_ALL_SENSOR_STATISTICS = 8,
	DUBAI_EVENT_ALL_SENSOR_TIME = 9,
	DUBAI_EVENT_SWING = 10,
	DUBAI_EVENT_TP = 11,
	DUBAI_EVENT_LCD_STATUS_TIMES = 14,
	DUBAI_EVENT_END
} dubai_event_type_t;

typedef enum {
	BIG_DATA_EVENT_MOTION_TYPE = 936005001,
	BIG_DATA_EVENT_DDR_INFO,
	BIG_DATA_EVENT_TOF_PHONECALL,
	BIG_DATA_EVENT_PHONECALL_SCREEN_STATUS,
	BIG_DATA_EVENT_PS_SOUND_INFO = 936005006,
	BIG_DATA_EVENT_VIB_RESP_TIME = 907400028,
	BIG_DATA_FOLD_TEMP = 936004016,
	BIG_DATA_EVENT_SYSLOAD_PERIOD = 936005007,
	BIG_DATA_EVENT_SYSLOAD_TRIGGER = 936005008,
	BIG_DATA_EVENT_AOD_INFO = 936005009,
	BIG_DATA_EVENT_BLPWM_USED_INFO = 936005011,
} big_data_event_id_t;

typedef enum {
	TYPE_STANDARD,
	TYPE_EXTEND
} type_step_counter_t;

typedef enum {
	THP_EVENT_NULL = 0,
	THP_EVENT_LOG,
	THP_EVENT_ALGO_SYNC_INFO,
	THP_EVENT_KEY,
	THP_EVENT_VOLUMN,
	THP_EVENT_END
} thp_shb_event_type_t;

typedef struct {
	uint8_t tag;
	uint8_t partial_order;
} pkt_part_header_t;

struct modem_pkt_header_t {
	uint8_t tag;
	uint8_t cmd;
	uint8_t core : 4;
	uint8_t resp : 1;
	uint8_t partial_order : 3;
	uint8_t length;
};
#ifndef CONFIG_INPUTHUB_30
struct pkt_header_resp {
	uint8_t tag;
	uint8_t cmd;
	uint8_t resp;
	uint8_t partial_order;
	uint16_t tranid;
	uint16_t length;
	uint32_t errno; /* In win32, errno is function name and conflict */
};
#endif
typedef struct {
	struct pkt_header hd;
	uint8_t wr;
	uint32_t fault_addr;
} __packed pkt_fault_addr_req_t;

typedef struct aod_display_pos {
	uint16_t x_start;
	uint16_t y_start;
} aod_display_pos_t;

typedef struct aod_start_config {
	aod_display_pos_t aod_pos;
	int32_t intelli_switching;
} aod_start_config_t;

typedef struct aod_time_config {
	uint64_t curr_time;
	int32_t time_zone;
	int32_t sec_time_zone;
	int32_t time_format;
} aod_time_config_t;

typedef struct aod_display_space {
	uint16_t x_start;
	uint16_t y_start;
	uint16_t x_size;
	uint16_t y_size;
} aod_display_space_t;

typedef struct aod_display_spaces {
	int32_t dual_clocks;
	int32_t display_type;
	int32_t display_space_count;
	aod_display_space_t display_spaces[5];
} aod_display_spaces_t;

typedef struct aod_screen_info {
	uint16_t xres;
	uint16_t yres;
	uint16_t pixel_format;
} aod_screen_info_t;

typedef struct aod_bitmap_size {
	uint16_t xres;
	uint16_t yres;
} aod_bitmap_size_t;

typedef struct aod_bitmaps_size {
	int32_t bitmap_type_count; /* 2, dual clock */
	aod_bitmap_size_t bitmap_size[2];
} aod_bitmaps_size_t;

typedef struct aod_config_info {
	uint32_t aod_fb;
	uint32_t aod_digits_addr;
	aod_screen_info_t screen_info;
	aod_bitmaps_size_t bitmap_size;
} aod_config_info_t;

typedef struct {
	struct pkt_header hd;
	uint32_t sub_cmd;
	union {
		aod_start_config_t start_param;
		aod_time_config_t time_param;
		aod_display_spaces_t display_param;
		aod_config_info_t config_param;
		aod_display_pos_t display_pos;
	};
} aod_req_t;

#define THERM_PARA_LEN 3
typedef struct {
	struct pkt_header hd;
	uint32_t sub_cmd;
	uint32_t para[THERM_PARA_LEN];
} therm_req_t;

typedef struct {
	struct pkt_header hd;
	int32_t x;
	int32_t y;
	int32_t z;
	uint32_t accracy;
} pkt_xyz_data_req_t;

struct sensor_data_xyz {
	int32_t x;
	int32_t y;
	int32_t z;
	uint32_t accracy;
};

typedef struct {
	struct pkt_header hd;
	uint16_t data_flag;
	uint16_t cnt;
	uint16_t len_element;
	uint16_t sample_rate;
	uint64_t timestamp;
}  pkt_common_data_t;

typedef struct {
	pkt_common_data_t data_hd;
	struct sensor_data_xyz xyz[]; /* x,y,z,acc,time */
} pkt_batch_data_req_t;

typedef struct {
	pkt_common_data_t hd;
	uint32_t status;
	uint32_t support;
} pkt_blpwm_sensor_req_t;

typedef struct {
	struct pkt_header hd;
	uint8_t sub_cmd;
	uint8_t pwm_cycle;
	uint8_t usage;
	uint8_t usage_stop;
	uint32_t color;
} pkt_blpwm_req_t;

typedef struct {
	struct pkt_header hd;
	s16 zaxis_data[];
} pkt_fingersense_data_report_req_t;

typedef struct {
	struct pkt_header hd;
	uint16_t data_flag;
	uint32_t step_count;
	uint32_t begin_time;
	uint16_t record_count;
	uint16_t capacity;
	uint32_t total_step_count;
	uint32_t total_floor_ascend;
	uint32_t total_calorie;
	uint32_t total_distance;
	uint16_t step_pace;
	uint16_t step_length;
	uint16_t speed;
	uint16_t touchdown_ratio;
	uint16_t reserved1;
	uint16_t reserved2;
	uint16_t action_record[];
} pkt_step_counter_data_req_t;

typedef struct {
	struct pkt_header hd;
	int32_t type;
	int16_t serial;
	/* 0: more additional info to be send  1:this pkt is last one */
	int16_t end;
	/*
	 * for each frame, a single data type,
	 * either int32_t or float, should be used
	 */
	union {
		int32_t data_int32[14];
	};
} pkt_additional_info_req_t;

typedef struct {
	pkt_common_data_t data_hd;
	int32_t status;
} pkt_magn_bracket_data_req_t;

typedef struct {
	unsigned int type;
	unsigned int initial_speed;
	unsigned int height;
	int angle_pitch;
	int angle_roll;
	unsigned int material;
} drop_info_t;

typedef struct {
	pkt_common_data_t data_hd;
	drop_info_t data;
} pkt_drop_data_req_t;

typedef struct interval_param {
	/* each group data of batch_count upload once, in & out */
	uint32_t period;
	/* input: expected value, out: the closest value actually supported */
	uint32_t batch_count;
	/*
	 * 0: auto mode, mcu reported according to the business characteristics
	 *    and system status
	 * 1: FIFO mode, maybe with multiple records
	 * 2: Integerate mode, update or accumulate the latest data, but do not
	 *    increase the record, and report it
	 * 3: real-time mode, real-time report no matter which status ap is
	 */
	uint8_t mode;
	/* reserved[0]: used by motion and pedometer now */
	uint8_t reserved[3];
} __packed interval_param_t;

typedef struct {
	struct pkt_header hd;
	interval_param_t param;
} __packed pkt_cmn_interval_req_t;

typedef struct {
	struct pkt_header hd;
	char app_config[16];
} __packed pkt_cmn_motion_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	uint8_t para[128];
} __packed pkt_parameter_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
} __packed pkt_subcmd_req_t;
#ifndef CONFIG_INPUTHUB_30
struct pkt_subcmd_resp {
	struct pkt_header_resp hd;
	uint32_t subcmd;
} __packed;
#endif
union spi_ctrl {
	uint32_t data;
	struct {
		/* bit0~8 is gpio NO., bit9~11 is gpio iomg set */
		uint32_t gpio_cs : 16;
		/* unit: MHz; 0 means default 5MHz */
		uint32_t baudrate : 5;
		/* low-bit: clk phase , high-bit: clk polarity convert, normally select:0 */
		uint32_t mode : 2;
		/* 0 means default: 8 */
		uint32_t bits_per_word : 5;
		uint32_t rsv_28_31 : 4;
	} b;
};

typedef struct {
	struct pkt_header hd;
	uint8_t busid;
	union {
		uint32_t i2c_address;
		union spi_ctrl ctrl;
	};
	uint16_t rx_len;
	uint16_t tx_len;
	uint8_t tx[];
} pkt_combo_bus_trans_req_t;

typedef struct {
	struct pkt_header_resp hd;
	uint8_t data[];
} pkt_combo_bus_trans_resp_t;

typedef struct {
	struct pkt_header hd;
	uint16_t status;
	uint16_t version;
} pkt_sys_statuschange_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t idle_time;
	uint32_t reserved;
	uint64_t current_app_mask;
} pkt_power_log_report_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t event_id;
} pkt_big_data_report_t;

typedef struct {
	struct pkt_header hd;
	/* 1:aux sensorlist 0:filelist 2: loading */
	uint8_t file_flg;
	/*
	 * num must less than
	 * (MAX_PKT_LENGTH-sizeof(PKT_HEADER)-sizeof(End))/sizeof(UINT16)
	 */
	uint8_t file_count; /* num of file or aux sensor */
	uint16_t file_list[]; /* fileid or aux sensor tag */
} pkt_sys_dynload_req_t;

typedef struct {
	struct pkt_header hd;
	uint8_t level;
	uint8_t dmd_case;
	uint8_t resv1;
	uint8_t resv2;
	uint32_t dmd_id;
	uint32_t info[5];
} pkt_dmd_log_report_req_t;

typedef struct{
	uint16_t rat;
	uint16_t band;
	int16_t power;
	uint16_t rsv;
}eda_dmd_tx_info;

typedef struct{
	uint16_t modem_id;
	uint8_t validflag;
	uint8_t channel;
	uint8_t tas_staus;
	uint8_t mas_staus;
	uint16_t trx_tas_staus;
	eda_dmd_tx_info tx_info[4]; /* 4 modem info */
}eda_dmd_mode_info;


typedef struct {
	struct pkt_header hd;
	uint8_t level;
	uint8_t dmd_case;
	uint8_t resv1;
	uint8_t resv2;
	uint32_t dmd_id;
	eda_dmd_mode_info sensorhub_dmd_mode_info[3]; /* 3 modem */
} pkt_mode_dmd_log_report_req_t;

struct pkt_vibrator_calibrate_data_req_t {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_VIB_CALIBRATE_DATA_LENGTH];
};

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
} pkt_mag_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
} pkt_mag1_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_MAG_AKM_CALIBRATE_DATA_LENGTH];
} pkt_akm_mag_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_GYRO_CALIBRATE_DATA_LENGTH];
} pkt_gyro_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_GYRO1_CALIBRATE_DATA_LENGTH];
} pkt_gyro1_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_GYRO_TEMP_OFFSET_LENGTH];
} pkt_gyro_temp_offset_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_TOF_CALIBRATE_DATA_LENGTH];
} pkt_tof_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_PS_CALIBRATE_DATA_LENGTH];
} pkt_ps_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_ALS_CALIBRATE_DATA_LENGTH];
} pkt_als_calibrate_data_req_t;

typedef struct {
	pkt_common_data_t fhd;
	int32_t data;
} fingerprint_upload_pkt_t;

typedef struct {
	uint32_t sub_cmd;
	uint8_t buf[7]; /* byte alignment */
	uint8_t len;
} fingerprint_req_t;

typedef struct {
	struct pkt_header hd;
	uint64_t app_id;
	uint16_t msg_type;
	uint8_t res[6];
	uint8_t data[];
} chre_req_t;
typedef struct {
	struct pkt_header hd;
	uint8_t core;
	uint8_t cmd;
	uint8_t data[];
} swing_req_t;

typedef struct{
	uint64_t sample_start_time;
	uint32_t sample_interval;
	uint32_t integ_time;
} als_run_stop_para_t;

typedef struct {
	struct pkt_header hd;
	uint8_t msg_type;
	uint8_t data[];
} touchpanel_report_pkt_t;

typedef struct {
	uint32_t size;
	char data[MAX_THP_SYNC_INFO_LEN];
} pkt_thp_sync_info_t; /* do not initialize in any function */

typedef enum additional_info_type {
	/* Marks the beginning of additional information frames */
	AINFO_BEGIN = 0x0,
	/* Marks the end of additional information frames */
	AINFO_END   = 0x1,
	/* Basic information */
	/*
	 * Estimation of the delay that is not tracked by sensor
	 * timestamps. This includes delay introduced by
	 * sensor front-end filtering, data transport, etc.
	 * float[2]: delay in seconds
	 * standard deviation of estimated value
	 */
	AINFO_UNTRACKED_DELAY =  0x10000,
	AINFO_INTERNAL_TEMPERATURE, /* float: Celsius temperature */
	/*
	 * First three rows of a homogeneous matrix, which
	 * represents calibration to a three-element vector
	 * raw sensor reading.
	 * float[12]: 3x4 matrix in row major order
	 */
	AINFO_VEC3_CALIBRATION,
	/*
	 * Location and orientation of sensor element in the
	 * device frame: origin is the geometric center of the
	 * mobile device screen surface; the axis definition
	 * corresponds to Android sensor definitions.
	 * float[12]: 3x4 matrix in row major order
	 */
	AINFO_SENSOR_PLACEMENT,
	/*
	 * float[2]: raw sample period in seconds,
	 * standard deviation of sampling period
	 */
	AINFO_SAMPLING,
	/* Sampling channel modeling information */
	/*
	 * int32_t: noise type
	 * float[n]: parameters
	 */
	AINFO_CHANNEL_NOISE = 0x20000,
	/*
	 * float[3]: sample period standard deviation of sample period,
	 * quantization unit
	 */
	AINFO_CHANNEL_SAMPLER,
	/*
	 * Represents a filter:
	 * \sum_j a_j y[n-j] == \sum_i b_i x[n-i]
	 * int32_t[3]: number of feedforward coefficients, M,
	 * number of feedback coefficients, N, for FIR filter, N=1.
	 * bit mask that represents which element to which the filter is applied
	 * bit 0 == 1 means this filter applies to vector element 0.
	 * float[M+N]: filter coefficients (b0, b1, ..., BM-1),
	 * then (a0, a1, ..., aN-1), a0 is always 1.
	 * Multiple frames may be needed for higher number of taps.
	 */
	AINFO_CHANNEL_FILTER,
	/*
	 * int32_t[2]: size in (row, column) ... 1st frame
	 * float[n]: matrix element values in row major order.
	 */
	AINFO_CHANNEL_LINEAR_TRANSFORM,
	/*
	 * int32_t[2]: extrapolate method interpolate method
	 * float[n]: mapping key points in pairs, (in, out)...
	 * (may be used to model saturation)
	 */
	AINFO_CHANNEL_NONLINEAR_MAP,
	/*
	 * int32_t: resample method (0-th order, 1st order...)
	 * float[1]: resample ratio (upsampling if < 1.0;
	 * downsampling if > 1.0).
	 */
	AINFO_CHANNEL_RESAMPLER,
	/* Custom information */
	AINFO_CUSTOM_START =    0x10000000,
	/* Debugging */
	AINFO_DEBUGGING_START = 0x40000000,
} additional_info_type_t;

#endif
