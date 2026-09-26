/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: IPC AP IOMCU
 * Author: Huawei
 * Create: 2020-08-26
 */
#ifndef __IPC_AP_IOMCU_H__
#define __IPC_AP_IOMCU_H__

struct pkt_header {
	uint8_t tag;
	uint8_t cmd;
	uint8_t resp:1;
	uint8_t hw_trans_mode:2; /* 0:IPC 1:SHMEM 2:64bitsIPC */
	uint8_t rsv:5; /* 5 bits */
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
	uint16_t tranid;
	uint16_t length;
	uint32_t errno;
};

struct pkt_subcmd_resp {
	struct pkt_header_resp hd;
	uint32_t subcmd;
} __packed;

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
	TAG_BT,
	TAG_BLPWM,
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
