/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: Sensor Hub Channel Bridge
 * Author: xiewengang
 * Create: 2013-10-12
 */

#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#define SUBCMD_LEN                         4
#define MAX_PKT_LENGTH                     128
#define MAX_PKT_LENGTH_AP                  2560
#define MAX_LOG_LEN                        100
#define MAX_PATTERN_SIZE                   16
#define MAX_ACCEL_PARAMET_LENGTH           100
#define MAX_MAG_PARAMET_LENGTH             100
#define MAX_GYRO_PARAMET_LENGTH            100
#define MAX_ALS_PARAMET_LENGTH             100
#define MAX_PS_PARAMET_LENGTH              100
#define MAX_I2C_DATA_LENGTH                50
#define MAX_SENSOR_CALIBRATE_DATA_LENGTH   60
#define MAX_MAG_CALIBRATE_DATA_LENGTH      12
#define MAX_GYRO_CALIBRATE_DATA_LENGTH     72
#define MAX_PS_CALIBRATE_DATA_LENGTH       12
#define MAX_GYRO_TEMP_OFFSET_LENGTH        56
#define MAX_CAP_PROX_CALIBRATE_DATA_LENGTH 16
#define MAX_MAG_AKM_CALIBRATE_DATA_LENGTH  28
#define DATA_FLAG_FLUSH_OFFSET             0
#define DATA_FLAG_VALID_TIMESTAMP_OFFSET   1
#define FLUSH_END                  (1 << DATA_FLAG_FLUSH_OFFSET)
#define DATA_FLAG_VALID_TIMESTAMP  (1 << DATA_FLAG_VALID_TIMESTAMP_OFFSET)
#define ACC_CALIBRATE_DATA_LENGTH          15
#define GYRO_CALIBRATE_DATA_LENGTH         18
#define PS_CALIBRATE_DATA_LENGTH           3
#define ALS_CALIBRATE_DATA_LENGTH          6

enum obj_tag {
	TAG_FLUSH_META,
	TAG_BEGIN = 0x01,
	TAG_SENSOR_BEGIN = TAG_BEGIN,
	TAG_ACCEL = TAG_SENSOR_BEGIN,
	TAG_GYRO,
	TAG_MAG,
	TAG_ALS,
	TAG_PS,
	TAG_LINEAR_ACCEL,
	TAG_GRAVITY,
	TAG_ORIENTATION,
	TAG_ROTATION_VECTORS,
	TAG_PRESSURE,
	TAG_HALL,
	TAG_MAG_UNCALIBRATED,
	TAG_GAME_RV,
	TAG_GYRO_UNCALIBRATED,
	TAG_SIGNIFICANT_MOTION,
	TAG_STEP_DETECTOR,
	TAG_STEP_COUNTER,
	TAG_GEOMAGNETIC_RV,
	TAG_HANDPRESS,
	TAG_FINGERSENSE,
	TAG_PHONECALL,
	TAG_GPS_4774_I2C,
	TAG_OIS,
	TAG_HINGE,
	TAG_RPC,
	TAG_CAP_PROX,
	TAG_MAGN_BRACKET,
	TAG_AGT,
	TAG_COLOR,
	TAG_ACCEL_UNCALIBRATED,
	TAG_TOF,
	TAG_DROP,
	TAG_SENSOR_END,
	TAG_HW_PRIVATE_APP_START = 45,
	TAG_AR = TAG_HW_PRIVATE_APP_START,
	TAG_MOTION,
	TAG_GPS,
	TAG_PDR,
	TAG_CA,
	TAG_FP,
	TAG_KEY,
	TAG_AOD,
	TAG_FLP,
	TAG_ENVIRONMENT,
	TAG_LOADMONITOR,
	TAG_APP_CHRE,
	TAG_FP_UD,
	TAG_HW_PRIVATE_APP_END,
	TAG_MODEM = 128,
	TAG_TP,
	TAG_SPI,
	TAG_I2C,
	TAG_UART,
	TAG_RGBLIGHT,
	TAG_BUTTONLIGHT,
	TAG_BACKLIGHT,
	TAG_VIBRATOR,
	TAG_SYS,
	TAG_LOG,
	TAG_LOG_BUFF,
	TAG_RAMDUMP,
	TAG_FAULT,
	TAG_SHAREMEM,
	TAG_SHELL_DBG,
	TAG_PD,
	TAG_I3C,
	TAG_DATA_PLAYBACK,
	TAG_CHRE,
	TAG_SENSOR_CALI,
	TAG_CELL,
	TAG_BIG_DATA,
	TAG_TIMESTAMP = 157,
	TAG_END = 0xFF
};

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
	CMD_DATA_REQ = 0x1f,
	CMD_DATA_RESP,
	CMD_SET_FAULT_TYPE_REQ,
	CMD_SET_FAULT_TYPE_RESP,
	CMD_SET_FAULT_ADDR_REQ,
	CMD_SET_FAULT_ADDR_RESP,

	/* SPI */
	CMD_SPI_BAUD_REQ,
	CMD_SPI_BAUD_RESP,
	CMD_SPI_TRANS_REQ,
	CMD_SPI_TRANS_RESP,

	/* I2C */
	CMD_I2C_TRANS_REQ,
	CMD_I2C_TRANS_RESP,

	/* system status */
	CMD_SYS_STATUSCHANGE_REQ,
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
	CMD_LOG_REPORT_REQ,
	CMD_LOG_REPORT_RESP,
	CMD_LOG_CONFIG_REQ,
	CMD_LOG_CONFIG_RESP,
	CMD_LOG_POWER_REQ,
	CMD_LOG_POWER_RESP,

	/* SHAREMEM */
	CMD_SHMEM_AP_RECV_REQ,
	CMD_SHMEM_AP_RECV_RESP,
	CMD_SHMEM_AP_SEND_REQ,
	CMD_SHMEM_AP_SEND_RESP,

	/* tag modem for cell info */
	CMD_MODEM_CELL_INFO_REQ,
	CMD_MODEM_CELL_INFO_RESP,
	CMD_MODEM_REBOOT_NOTIFY_REQ,
	CMD_MODEM_REBOOT_NOTIFY_RESP,

	/* SHELL_DBG */
	CMD_SHELL_DBG_REQ,
	CMD_SHELL_DBG_RESP,

	/* LoadMonitor */
	CMD_READ_AO_MONITOR_SENSOR,
	CMD_READ_AO_MONITOR_SENSOR_RESP,

	/* TAG_DATA_PLAYBACK */
	CMD_DATA_PLAYBACK_DATA_READY_REQ, /* BACKPLAY */
	CMD_DATA_PLAYBACK_DATA_READY_RESP,
	CMD_DATA_PLAYBACK_BUF_READY_REQ, /* RECORD */
	CMD_DATA_PLAYBACK_BUF_READY_RESP,

	/* CHRE */
	CMD_CHRE_AP_SEND_TO_MCU,
	CMD_CHRE_AP_SEND_TO_MCU_RESP,
	CMD_CHRE_MCU_SEND_TO_AP,
	CMD_CHRE_MCU_SEND_TO_AP_RESP,

	/* BIG DATA */
	CMD_BIG_DATA_REQUEST_DATA,
	CMD_BIG_DATA_REQUEST_DATA_RESP,
	CMD_BIG_DATA_SEND_TO_AP,
	CMD_BIG_DATA_SEND_TO_AP_RESP,

	/* tag sys for hall status */
	CMD_SYS_HALL_STATUS_REQ = 0x53,
	CMD_SYS_HALL_STATUS_RESP = 0x54,

	/* tag modem for phonecall status */
	CMD_MODEM_PHONECALL_INFO_REQ = 0x55,
	CMD_MODEM_PHONECALL_INFO_RESP = 0x56,

	/* log buff */
	CMD_LOG_SER_REQ = 0xf1,
	CMD_LOG_USEBUF_REQ,
	CMD_LOG_BUFF_ALERT,
	CMD_LOG_BUFF_FLUSH,
	CMD_LOG_BUFF_FLUSHP,
	CMD_EXT_LOG_FLUSH,
	CMD_EXT_LOG_FLUSHP,
	CMD_SYS_TIMEZONE_REQ,
	CMD_SYS_TIMEZONE_RESP,
	CMD_SYS_TIMESTAMP_REQ = 0xF8,
	CMD_SYS_TIMESTAMP_RESP = 0xF9,

	/* max cmd */
	CMD_ERR_RESP = 0xfe,
};

enum obj_sub_cmd {
	SUB_CMD_NULL_REQ = 0x0,
	SUB_CMD_SELFCALI_REQ = 0x01,
	SUB_CMD_SET_PARAMET_REQ,
	SUB_CMD_SET_OFFSET_REQ,
	SUB_CMD_SELFTEST_REQ,
	SUB_CMD_CALIBRATE_DATA_REQ,
	SUB_CMD_SET_SLAVE_ADDR_REQ,
	SUB_CMD_SET_RESET_PARAM_REQ,
	SUB_CMD_ADDITIONAL_INFO,
	SUB_CMD_FW_DLOAD_REQ,
	SUB_CMD_FLUSH_REQ,
	SUB_CMD_SET_ADD_DATA_REQ,
	SUB_CMD_SET_DATA_TYPE_REQ,
	SUB_CMD_SET_DATA_MODE = 0x0d,

	/* als backlight */
	SUB_CMD_UPDATE_BL_LEVEL = 0x21,

	/* motion */
	SUB_CMD_MOTION_ATTR_ENABLE_REQ = 0x20,
	SUB_CMD_MOTION_ATTR_DISABLE_REQ,
	SUB_CMD_MOTION_REPORT_REQ,
	SUB_CMD_MOTION_HORIZONTAL_PICKUP_REQ = 0x23,

	/* ca */
	SUB_CMD_CA_ATTR_ENABLE_REQ = 0x20,
	SUB_CMD_CA_ATTR_DISABLE_REQ,
	SUB_CMD_CA_REPORT_REQ,

	/* fingerprint */
	SUB_CMD_FINGERPRINT_OPEN_REQ = 0x20,
	SUB_CMD_FINGERPRINT_CLOSE_REQ,
	SUB_CMD_FINGERPRINT_CONFIG_SENSOR_DATA_REQ,

	/* gyro */
	SUB_CMD_GYRO_OIS_REQ = 0x20,
	SUB_CMD_GYRO_TMP_OFFSET_REQ,

	/* finger sense */
	SUB_CMD_ACCEL_FINGERSENSE_ENABLE_REQ = 0x20,
	SUB_CMD_ACCEL_FINGERSENSE_CLOSE_REQ,
	SUB_CMD_ACCEL_FINGERSENSE_REQ_DATA_REQ,
	SUB_CMD_ACCEL_FINGERSENSE_DATA_REPORT,

	/* tag pdr */
	SUB_CMD_FLP_PDR_START_REQ = 0x20,
	SUB_CMD_FLP_PDR_STOP_REQ,
	SUB_CMD_FLP_PDR_WRITE_REQ,
	SUB_CMD_FLP_PDR_UPDATE_REQ,
	SUB_CMD_FLP_PDR_FLUSH_REQ,
	SUB_CMD_FLP_PDR_UNRELIABLE_REQ,
	SUB_CMD_FLP_PDR_STEPCFG_REQ,

	/* tag ar */
	SUB_CMD_FLP_AR_START_REQ = 0x20,
	SUB_CMD_FLP_AR_CONFIG_REQ,
	SUB_CMD_FLP_AR_STOP_REQ,
	SUB_CMD_FLP_AR_UPDATE_REQ,
	SUB_CMD_FLP_AR_FLUSH_REQ,
	SUB_CMD_FLP_AR_GET_STATE_REQ,
	SUB_CMD_FLP_AR_SHMEM_STATE_REQ,
	SUB_CMD_CELL_INFO_DATA_REQ = 0x29,

	/* tag environment */
	SUB_CMD_ENVIRONMENT_INIT_REQ = 0x20,
	SUB_CMD_ENVIRONMENT_ENABLE_REQ,
	SUB_CMD_ENVIRONMENT_DISABLE_REQ,
	SUB_CMD_ENVIRONMENT_EXIT_REQ,
	SUB_CMD_ENVIRONMENT_DATABASE_REQ,
	SUB_CMD_ENVIRONMENT_GET_STATE_REQ,

	/* tag flp */
	SUB_CMD_FLP_START_BATCHING_REQ = 0x20,
	SUB_CMD_FLP_STOP_BATCHING_REQ,
	SUB_CMD_FLP_UPDATE_BATCHING_REQ,
	SUB_CMD_FLP_GET_BATCHED_LOCATION_REQ,
	SUB_CMD_FLP_FLUSH_LOCATION_REQ,
	SUB_CMD_FLP_INJECT_LOCATION_REQ,
	SUB_CMD_FLP_RESET_REQ,
	SUB_CMD_FLP_RESET_RESP,
	SUB_CMD_FLP_GET_BATCH_SIZE_REQ,
	SUB_CMD_FLP_BATCH_PUSH_GNSS_REQ,
	SUB_CMD_FLP_ADD_GEOF_REQ,
	SUB_CMD_FLP_REMOVE_GEOF_REQ,
	SUB_CMD_FLP_MODIFY_GEOF_REQ,
	SUB_CMD_FLP_LOCATION_UPDATE_REQ,
	SUB_CMD_FLP_GEOF_TRANSITION_REQ,
	SUB_CMD_FLP_GEOF_MONITOR_STATUS_REQ,
	SUB_CMD_FLP_GEOF_GET_TRANSITION_REQ,
	SUB_CMD_FLP_CELLFENCE_ADD_REQ,
	SUB_CMD_FLP_CELLFENCE_OPT_REQ,
	SUB_CMD_FLP_CELLFENCE_TRANSITION_REQ,
	SUB_CMD_FLP_CELLFENCE_INJECT_RESULT_REQ,
	SUB_CMD_FLP_CELLTRAJECTORY_CFG_REQ,
	SUB_CMD_FLP_CELLTRAJECTORY_REQUEST_REQ,
	SUB_CMD_FLP_CELLTRAJECTORY_REPORT_REQ,
	SUB_CMD_FLP_CELLDB_LOCATION_REPORT_REQ,
	SUB_CMD_FLP_COMMON_STOP_SERVICE_REQ,
	SUB_CMD_FLP_COMMON_WIFI_CFG_REQ,
	SUB_CMD_FLP_GEOF_GET_LOCATION_REQ,
	SUB_CMD_FLP_GEOF_GET_LOCATION_REPORT_REQ,
	SUB_CMD_FLP_ADD_WIFENCE_REQ,
	SUB_CMD_FLP_REMOVE_WIFENCE_REQ,
	SUB_CMD_FLP_PAUSE_WIFENCE_REQ,
	SUB_CMD_FLP_RESUME_WIFENCE_REQ,
	SUB_CMD_FLP_GET_WIFENCE_STATUS_REQ,
	SUB_CMD_FLP_WIFENCE_TRANSITION_REQ,
	SUB_CMD_FLP_WIFENCE_STATUS_REQ,
	SUB_CMD_FLP_COMMON_DEBUG_CONFIG_REQ,
	SUB_CMD_FLP_CELL_CELLBATCHING_CFG_REQ,
	SUB_CMD_FLP_CELL_CELLBATCHING_REQ,
	SUB_CMD_FLP_CELL_CELLBATCHING_REPORT_REQ,
	SUB_CMD_FLP_DIAG_SEND_CMD_REQ,
	SUB_CMD_FLP_DIAG_DATA_REPORT_REQ,
	SUB_CMD_FLP_BATCHING_MULT_LASTLOCATION_REQ,
	SUB_CMD_FLP_BATCHING_MULT_FLUSH_REQ,
	SUB_CMD_FLP_GEOF_GET_SIZE_REQ,
	SUB_CMD_FLP_GEOF_GET_SIZE_REPORT_REQ,
	SUB_CMD_FLP_CELLFENCE_GET_SIZE_REQ,
	SUB_CMD_FLP_CELLFENCE_GET_SIZE_REPORT_REQ,
	SUB_CMD_FLP_WIFENCE_GET_SIZE_REQ,
	SUB_CMD_FLP_WIFENCE_GET_SIZE_REPORT_REQ,

	/* Always On Display */
	SUB_CMD_AOD_START_REQ = 0x20,
	SUB_CMD_AOD_STOP_REQ,
	SUB_CMD_AOD_START_UPDATING_REQ,
	SUB_CMD_AOD_END_UPDATING_REQ,
	SUB_CMD_AOD_SET_TIME_REQ,
	SUB_CMD_AOD_SET_DISPLAY_SPACE_REQ,
	SUB_CMD_AOD_SETUP_REQ,
	SUB_CMD_AOD_DSS_ON_REQ,
	SUB_CMD_AOD_DSS_OFF_REQ,

	/* key */
	SUB_CMD_BACKLIGHT_REQ = 0x20,
	/* rpc */
	SUB_CMD_RPC_START_REQ = 0x20,
	SUB_CMD_RPC_STOP_REQ,
	SUB_CMD_RPC_UPDATE_REQ,
	SUB_CMD_RPC_LIBHAND_REQ,
	SUB_CMD_VIBRATOR_SINGLE_REQ = 0x20,
	SUB_CMD_VIBRATOR_REPEAT_REQ,
	SUB_CMD_VIBRATOR_ON_REQ,
	SUB_CMD_VIBRATOR_SET_AMPLITUDE_REQ,
	SUB_CMD_MAX = 0xff,
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
	MOTION_TYPE_EXT_LOG,
	MOTION_TYPE_HEAD_DOWN,
	MOTION_TYPE_PUT_DOWN,
	MOTION_TYPE_REMOVE,
	MOTION_TYPE_FALL,
	MOTION_TYPE_SIDEGRIP,
	MOTION_TYPE_MOVE,
	/* NOTE:add string in motion_type_str when add type */
	MOTION_TYPE_END,
} motion_type_t;

typedef enum {
	FINGERPRINT_TYPE_START = 0x0,
	FINGERPRINT_TYPE_HUB,
	FINGERPRINT_TYPE_END,
} fingerprint_type_t;

typedef enum {
	CA_TYPE_START,
	CA_TYPE_PICKUP,
	CA_TYPE_PUTDOWN,
	CA_TYPE_ACTIVITY,
	CA_TYPE_HOLDING,
	CA_TYPE_MOTION,
	CA_TYPE_PLACEMENT,
	/* NOTE:add string in ca_type_str when add type */
	CA_TYPE_END,
} ca_type_t;

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
	ST_SCREENON,
	ST_SCREENOFF,
	ST_SLEEP,
	ST_WAKEUP,
	ST_POWEROFF,
	ST_RECOVERY_BEGIN,
	ST_RECOVERY_FINISH,
	ST_END
} sys_status_t;

typedef enum  {
	DUBAI_EVENT_NULL = 0,
	DUBAI_EVENT_AOD_PICKUP = 3,
	DUBAI_EVENT_AOD_PICKUP_NO_FINGERDOWN = 4,
	DUBAI_EVENT_AOD_TIME_STATISTICS = 6,
	DUBAI_EVENT_FINGERPRINT_ICON_COUNT = 7,
	DUBAI_EVENT_ALL_SENSOR_STATISTICS = 8,
	DUBAI_EVENT_ALL_SENSOR_TIME = 9,
	DUBAI_EVENT_END
} dubai_event_type_t;

typedef enum {
	BIG_DATA_EVENT_MOTION_TYPE = 936005001,
	BIG_DATA_EVENT_DDR_INFO,
	BIG_DATA_EVENT_TOF_PHONECALL
} big_data_event_id_t;

typedef enum {
	TYPE_STANDARD,
	TYPE_EXTEND
} type_step_counter_t;

typedef struct {
	uint8_t tag;
	uint8_t partial_order;
} pkt_part_header_t;

struct pkt_header {
	uint8_t tag;
	uint8_t cmd;
	uint8_t resp; /* value CMD_RESP means need resp, CMD_NO_RESP means need not resp */
	uint8_t partial_order;
	uint16_t tranid;
	uint16_t length;
};

struct pkt_header_resp {
	uint8_t tag;
	uint8_t cmd;
	uint8_t resp;
	uint8_t partial_order;
	uint16_t tranid;
	uint16_t length;
	uint32_t errno; /* In win32, errno is function name and conflict */
};

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
} pkt_common_data_t;

typedef struct {
	pkt_common_data_t data_hd;
	struct sensor_data_xyz xyz[]; /* x,y,z,acc,time */
} pkt_batch_data_req_t;

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
	int16_t end;  /* 0: more additional info to be send  1:this pkt is last one */
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
	drop_info_t   data;
} pkt_drop_data_req_t;

typedef struct interval_param {
	uint32_t  period;
	uint32_t batch_count;
	uint8_t mode;
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

struct pkt_subcmd_resp {
	struct pkt_header_resp hd;
	uint32_t subcmd;
} __packed;

union spi_ctrl {
	uint32_t data;
	struct {
		uint32_t gpio_cs   : 16; /* bit0~8 is gpio NO., bit9~11 is gpio iomg set */
		uint32_t baudrate  : 5;  /* unit: MHz; 0 means default 5MHz */
		uint32_t mode      : 2;  /* low-bit: clk phase , high-bit: clk polarity convert, normally select:0 */
		uint32_t bits_per_word : 5; /* 0 means default: 8 */
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
	uint64_t current_app_mask;
} pkt_power_log_report_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t event_id;
} pkt_big_data_report_t;

typedef struct {
	struct pkt_header hd;
	uint8_t end;
	uint8_t file_count;
	uint16_t file_list[];
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

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_MAG_CALIBRATE_DATA_LENGTH];
} pkt_mag_calibrate_data_req_t;

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
	char calibrate_data[MAX_PS_CALIBRATE_DATA_LENGTH];
} pkt_ps_calibrate_data_req_t;

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	char calibrate_data[MAX_GYRO_TEMP_OFFSET_LENGTH];
} pkt_gyro_temp_offset_req_t;

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
	uint8_t res[2];
	uint8_t data[];
} chre_req_t;

typedef enum additional_info_type {
	AINFO_BEGIN = 0x0,
	AINFO_END   = 0x1,
	/* Basic information */
	AINFO_UNTRACKED_DELAY =  0x10000,
	AINFO_INTERNAL_TEMPERATURE,
	AINFO_VEC3_CALIBRATION,
	AINFO_SENSOR_PLACEMENT,
	AINFO_SAMPLING,
	/* Sampling channel modeling information */
	AINFO_CHANNEL_NOISE = 0x20000,
	AINFO_CHANNEL_SAMPLER,
	AINFO_CHANNEL_FILTER,
	AINFO_CHANNEL_LINEAR_TRANSFORM,
	AINFO_CHANNEL_NONLINEAR_MAP,
	AINFO_CHANNEL_RESAMPLER,
	/* Custom information */
	AINFO_CUSTOM_START =    0x10000000,
	/* Debugging */
	AINFO_DEBUGGING_START = 0x40000000,
} additional_info_type_t;

enum {
	FILE_BEGIN,
	FILE_BASIC_SENSOR_APP = FILE_BEGIN,
	FILE_FUSION,
	FILE_FUSION_GAME,
	FILE_FUSION_GEOMAGNETIC,
	FILE_MOTION,
	FILE_PEDOMETER,
	FILE_PDR,
	FILE_AR,
	FILE_GSENSOR_GATHER_FOR_GPS,
	FILE_PHONECALL,
	FILE_FINGERSENSE,
	FILE_SIX_FUSION,
	FILE_HANDPRESS,
	FILE_CA,
	FILE_OIS,
	FILE_FINGERPRINT,
	FILE_KEY,
	FILE_GSENSOR_GATHER_SINGLE_FOR_GPS,
	FILE_AOD,
	FILE_MODEM,
	FILE_CHARGING,
	FILE_MAGN_BRACKET,
	FILE_FLP,
	FILE_TILT_DETECTOR,
	FILE_RPC,
	FILE_FINGERPRINT_UD = 28,
	FILE_DROP,
	FILE_APP_ID_MAX = 31,
	FILE_AKM09911_DOE_MAG,
	FILE_BMI160_ACC,
	FILE_LSM6DS3_ACC,
	FILE_BMI160_GYRO,
	FILE_LSM6DS3_GYRO,
	FILE_AKM09911_MAG,
	FILE_BH1745_ALS,
	FILE_PA224_PS,
	FILE_ROHM_BM1383,
	FILE_APDS9251_ALS,
	FILE_LIS3DH_ACC,
	FILE_KIONIX_ACC,
	FILE_APDS993X_ALS,
	FILE_APDS993X_PS,
	FILE_TMD2620_PS,
	FILE_GPS_4774,
	FILE_ST_LPS22BH,
	FILE_APDS9110_PS,
	FILE_CYPRESS_HANDPRESS,
	FILE_LSM6DSM_ACC,
	FILE_LSM6DSM_GYRO,
	FILE_ICM20690_ACC,
	FILE_ICM20690_GYRO,
	FILE_LTR578_ALS,
	FILE_LTR578_PS,
	FILE_FPC1021_FP,
	FILE_CAP_PROX,
	FILE_CYPRESS_KEY,
	FILE_CYPRESS_SAR,
	FILE_GPS_4774_SINGLE,
	FILE_SX9323_CAP_PROX,
	FILE_BQ25892_CHARGER,
	FILE_FSA9685_SWITCH,
	FILE_SCHARGER_V300,
	FILE_YAS537_MAG,
	FILE_AKM09918_MAG,
	FILE_TMD2745_ALS,
	FILE_TMD2745_PS,
	FILE_YAS537_DOE_MAG = 73,
	FILE_FPC1268_FP,
	FILE_GOODIX8206_FP,
	FILE_FPC1075_FP,
	FILE_FPC1262_FP,
	FILE_GOODIX5296_FP,
	FILE_GOODIX3288_FP,
	FILE_SILEAD6185_FP,
	FILE_SILEAD6275_FP,
	FILE_BOSCH_BMP380,
	FILE_TMD3725_ALS,
	FILE_TMD3725_PS,
	FILE_DRV2605_DRV,
	FILE_LTR582_ALS,
	FILE_LTR582_PS,
	FILE_GOODIX_BAIKAL_FP,
	FILE_GOODIX5288_FP,
	FILE_QFP1500_FP,
	FILE_FPC1291_FP,
	FILE_FPC1028_FP,
	FILE_GOODIX3258_FP,
	FILE_SILEAD6152_FP,
	FILE_APDS9999_ALS,
	FILE_APDS9999_PS,
	FILE_TMD3702_ALS,
	FILE_TMD3702_PS,
	FILE_AMS8701_TOF,
	FILE_VCNL36658_ALS,
	FILE_VCNL36658_PS,
	FILE_SILEAD6165_FP,
	FILE_SYNA155A_FP,
	FILE_TSL2591_ALS = 105,
	FILE_BH1726_ALS = 106,
	FILE_GOODIX3658_FP = 107,
	FILE_LTR2568_PS = 108,
	FILE_STK3338_PS = 109,
	FILE_SILEAD6152B_FP = 110,
	FILE_VCNL36832_PS = 111,
	FILE_MMC5603_MAG,
	FILE_MMC5603_DOE_MAG,
	FILE_FPC1511_FP = 114,
	FILE_STK3338_ALS = 115,
	FILE_APDS9308_ALS = 116,
	FILE_LTR2568_ALS = 117,
	FILE_VCNL36832_ALS,
	FILE_TMD2702_ALS = 119,
	FILE_A96T3X6_CAP_PROX,
	FILE_LIS2DWL_ACC,
	FILE_BMA422_ACC,
	FILE_GOODIX3216_FP = 129,
	FILE_ET525_FP = 130,
	FILE_BMA2X2_ACC = 131,
	FILE_MC34XX_ACC = 132,
	FILE_SY3079_ALS = 133,
	FILE_STK3321_ALS = 134,
	FILE_STK3321_PS = 135,
	FILE_STK3235_ALS = 136,
	FILE_LTR578_039WA_PS = 137,
	FILE_MC3419_ACC = 138,
	FILE_DA718_ACC = 185,
	FILE_BU27006MUC_ALS = 186,
	FILE_VD6281_ALS = 187,
	FILE_AW9610XB_CAP_PROX = 188,
	FILE_TCS34083_ALS = 189,
	FILE_QMC6308_DOE_MAG = 190,
	FILE_ID_MAX, /* MAX VALID FILE ID */
};

#endif
