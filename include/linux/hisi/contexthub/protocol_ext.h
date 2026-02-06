/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: protocol_ext.h.
 * Create: 2019/11/05
 */

#ifndef __PROTOCOL_AS_EXT_H
#define __PROTOCOL_AS_EXT_H

#define SUBCMD_LEN 4
#define MAX_PKT_LENGTH                       128
#define MAX_PKT_LENGTH_AP                    2560
#define MAX_LOG_LEN                          100
#define MAX_PATTERN_SIZE                     16
#define MAX_ACCEL_PARAMET_LENGTH             100
#define MAX_MAG_PARAMET_LENGTH               100
#define MAX_GYRO_PARAMET_LENGTH              100
#define MAX_ALS_PARAMET_LENGTH               100
#define MAX_PS_PARAMET_LENGTH                100
#define MAX_I2C_DATA_LENGTH                  50
#define MAX_SENSOR_CALIBRATE_DATA_LENGTH     60
#define MAX_VIB_CALIBRATE_DATA_LENGTH        3
#define MAX_MAG_CALIBRATE_DATA_LENGTH        12
#define MAX_GYRO_CALIBRATE_DATA_LENGTH       72
#define MAX_GYRO_TEMP_OFFSET_LENGTH          56
#define MAX_CAP_PROX_CALIBRATE_DATA_LENGTH   16
#define MAX_MAG_FOLDER_CALIBRATE_DATA_LENGTH 24
#define MAX_MAG_AKM_CALIBRATE_DATA_LENGTH    28
#define MAX_TOF_CALIBRATE_DATA_LENGTH        47
#define MAX_PS_CALIBRATE_DATA_LENGTH         24
#define MAX_ALS_CALIBRATE_DATA_LENGTH        24

/* data flag consts */
#define DATA_FLAG_FLUSH_OFFSET           0
#define DATA_FLAG_VALID_TIMESTAMP_OFFSET 1
#define FLUSH_END                        (1 << DATA_FLAG_FLUSH_OFFSET)
#define DATA_FLAG_VALID_TIMESTAMP        (1 << DATA_FLAG_VALID_TIMESTAMP_OFFSET)
#define ACC_CALIBRATE_DATA_LENGTH        15
#define PS_CALIBRATE_DATA_LENGTH         6
#define ALS_CALIBRATE_DATA_LENGTH        6
#define ACC1_CALIBRATE_DATA_LENGTH       60
#define ACC1_OFFSET_DATA_LENGTH          15
#define GYRO1_CALIBRATE_DATA_LENGTH      18
#define MAX_GYRO1_CALIBRATE_DATA_LENGTH  72
#define MAX_THP_SYNC_INFO_LEN            (2 * 1024)

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
	/* !!!NOTE:add string in ca_type_str when add type */
	CA_TYPE_END,
} ca_type_t;

typedef enum {
	TYPE_STANDARD,
	TYPE_EXTEND
} type_step_counter_t;

typedef enum {
	EN_OK = 0,
	EN_FAIL,
} err_no_t;

typedef enum {
	NO_RESP,
	RESP,
} obj_resp_t;

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

struct pkt_header {
	uint8_t tag;
	uint8_t cmd;
	uint8_t resp: 1;
	uint8_t hw_trans_mode: 2; /* 0:IPC 1:SHMEM 2:64bitsIPC */
	uint8_t rsv: 5;
	uint8_t partial_order;
	uint8_t tranid;
	uint8_t app_tag;
	uint16_t length;
};

struct pkt_header_resp {
	uint8_t tag;
	uint8_t cmd;
	uint8_t resp;
	uint8_t partial_order;
	uint8_t tranid;
	uint8_t app_tag;
	uint16_t length;
	uint32_t errno; /* In win32, errno is function name and conflict */
};

typedef struct {
	struct pkt_header hd;
	uint8_t wr;
	uint32_t fault_addr;
} __packed pkt_fault_addr_req_t;

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
} pkt_dft_report_t;

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


typedef enum {
	DFT_EVENT_CPU_USE = 1,
	DFT_EVENT_TASK_IFO,
	DFT_EVENT_MEM_USE,
	DFT_EVENT_FREQ_TIME,
	DFT_EVENT_SENSOR,
	DFT_EVENT_APP,
	DFT_EVENT_RELATION,
	DFT_EVENT_MAX,
} dft_event_id_t;


#define IOM3_ST_NORMAL      0
#define IOM3_ST_RECOVERY    1
#define IOM3_ST_REPEAT      2

#define MAX_STR_SIZE 1024

#define RET_SUCC  0
#define RET_FAIL  -1

#endif
