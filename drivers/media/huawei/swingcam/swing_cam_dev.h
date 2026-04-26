/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Declaration of swing driver
 * Author: lidenghui
 * Create: 2019-03-26
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
 */

#ifndef __LINUX_CONTEXTHUB_SWING_CAM_H__
#define __LINUX_CONTEXTHUB_SWING_CAM_H__

#define NAME_LEN 32

#ifdef MAX_STREAM
#undef MAX_STREAM
#endif

#define MAX_STREAM 2
#define PARA_MAX_LEN 96 /* max len, can't be increased */
#define BUS_TYPE_NAME_LEN 8
#define MAX_SWING_PAYLOAD 112

enum {
	MINIISP_NAME_SIZE = 32,
	MINIISP_V4L2_EVENT_TYPE = V4L2_EVENT_PRIVATE_START + 0x00100000,
	MINIISP_HIGH_PRIO_EVENT = 0x1500,
	MINIISP_MIDDLE_PRIO_EVENT = 0x2000,
	MINIISP_LOW_PRIO_EVENT = 0x3000,
};

enum mini_api_id {
	/* request */
	COMMAND_QUERY_SWING_CAM,
	COMMAND_ACQUIRE_SWING_CAM,
	COMMAND_USECASE_CONFIG_SWING_CAM,
	COMMAND_MINIISP_REQUEST,
	COMMAND_RELEASE_SWING_CAM,
	COMMAND_MINIISP_EXTEND_SET,
	COMMAND_GET_SWING_CAM_OTP,

	/* response */
	QUERY_SWING_CAM_RESPONSE,
	ACQUIRE_SWING_CAM_RESPONSE,
	USECASE_CONFIG_SWING_CAM_RESPONSE,
	REQUEST_MINIISP_RESPONSE,
	RELEASE_SWING_CAM_RESPONSE,
	EXTEND_SET_MINIISP_RESPONSE,

	/* event sent to ap */
	MINI_EVENT_SENT,
};

enum {
	NORMAL_SIZE = 0 << 0,
	FULL_SIZE = 1 << 1,
};

enum {
	SWING_SINGLE = 0,
	SWING_CONTINUOUS,
};

enum miniisp_event_kind {
	MINIISP_MSG_CB,
	MINIISP_MSG_REQUEST,
};

enum mini_event_info {
	EVENT_MINI_RECOVER_CODE = 0,
	EVENT_AO_CAMERA_OPEN,
	EVENT_AO_CAMERA_CLOSE,
	EVENT_AO_TURN_ON_IR,
	EVENT_AO_TURN_OFF_IR,
	EVENT_AO_POWER_ON_TOF,
	EVENT_AO_POWER_OFF_TOF,
};

typedef enum {
	SENSOR_PIXEL_IR,
	SENSOR_PIXEL_RGB,
	SENSOR_PIXEL_MONO,
	SENSOR_PIXEL_MAX,
} process_pattern_e;

enum {
	SWING_PIXEL_FMT_XRGB8888,
	SWING_PIXEL_FMT_RGB888,   // RGB
	SWING_PIXEL_FMT_Y8,       // MONO
	SWING_PIXEL_FMT_YUV422I,  // reserved for baltimore
};

struct miniisp_event {
	enum miniisp_event_kind kind;
};

struct msg_req_get_ao_otp_t {
	unsigned int cam_id;
	char         sensor_name[NAME_LEN];
	unsigned int input_otp_buffer;
	unsigned int buffer_size;
};

struct msg_ack_get_ao_otp_t {
	unsigned int cam_id;
	char         sensor_name[NAME_LEN];
	unsigned int output_otp_buffer;
	unsigned int buffer_size;
	int          status;
};

struct msg_req_query_swing_cam {
	unsigned int i2c_index;
	unsigned int mount_position;
	unsigned int csi_index;
	char         product_name[NAME_LEN];
	char         name[NAME_LEN];
	char         bus_type[BUS_TYPE_NAME_LEN];
};

struct mini_stream_config {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int format; /* XRGB8888/RGB888/Y8/YUV422I */
};

struct msg_req_config_swing_cam {
	unsigned int extension;
	unsigned int flow_mode;
	unsigned int process_pattern;
	unsigned int is_secure[MAX_STREAM];
	struct mini_stream_config stream_cfg;
};

struct msg_ack_config_swing_cam {
	unsigned int extension;
};

struct msg_ack_swing_cam {
	char          name[NAME_LEN];
	unsigned char version;
	int           status;
};

struct stream_info_swing_cam {
	unsigned int buffer;
	unsigned int width;
	unsigned int height;
	unsigned int format;
	unsigned int valid;
	unsigned int frame_num;
};

struct miniisp_req_request {
	unsigned int map_fd;
	unsigned int target_map;
	/* [0] represents miniisp_out_buf, [1] represets out_metadata */
	unsigned int buf_pa[MAX_STREAM];
	unsigned int buf_size[MAX_STREAM];
	unsigned int frame_num;
};

struct miniisp_ack_request {
	unsigned int buf_pa[MAX_STREAM];
	struct stream_info_swing_cam stream_info[MAX_STREAM];
	unsigned int timestamp_low;
	unsigned int timestamp_high;
};

struct awb_t {
	unsigned int r_gain;
	unsigned int g_gain;
	unsigned int b_gain;
};

struct crop_region {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
};

struct ae_t {
	unsigned int expo;
	unsigned int gain;
};

struct ae_md_t {
	struct ae_t  cur_ae;
	unsigned int ae_stable;
	unsigned int stat_stable; /* used to decide to enter MD mode or not */
	unsigned int ave_luma;
	unsigned int current_luma; /* center weighted luma */
	unsigned int face_luma;
	unsigned int lv;
	unsigned int fps; /* only for continuous mode */
};

enum sensor_work_mode {
	SENSOR_MODE_PPI,
	SENSOR_MODE_MIPI_ONLINE,
	SENSOR_MODE_MIPI_OFFLINE,
	SENSOR_WORK_MODE_MAX,
};

enum sensor_test_pattern {
	SENSOR_TEST_PATTERN_NORMAL,
	/* miniisp bypass all algos, include BLC */
	SENSOR_TEST_PATTERN_COLORBAR,
	SENSOR_TEST_PATTERN_FULL_SIZE,
	SENSOR_TEST_PATTERN_MAX,
};

struct miniisp_phy_info {
	unsigned int    is_master_sensor;
	phy_id_e        phy_id;
	phy_mode_e      phy_mode;
	phy_work_mode_e phy_work_mode;
};

struct sensor_spec {
	enum sensor_work_mode sensor_mode;
	enum sensor_test_pattern test_pattern; /* reserve */
	struct miniisp_phy_info phy_info;
};

struct msg_req_acquire_swing_cam {
	char sensor_name[NAME_LEN];
	char i2c_bus_type[BUS_TYPE_NAME_LEN];
	unsigned int i2c_index;
	unsigned int chrom_addr;
	struct sensor_spec sensor_spec;
};

struct msg_ack_acquire_swing_cam {
	unsigned int csi_index;
	int init_ret;
};

struct msg_req_release_swing_cam {
	unsigned int cam_id;
};

struct msg_ack_release_swing_cam {
	unsigned int cam_id;
};

enum miniisp_stream_pos {
	STREAM_MINIISP_OUT_PREVIEW = 0,
	STREAM_MINIISP_OUT_META = 1,
	STREAM_MINIISP_POS_MAX,
};

enum miniisp_extendset_info {
	/* below subcmd should be called before config */
	SUBCMD_BYPASS_ISP_ALGO,      /* bypass miniISP algos */
	/* Shutter priority, for motion cases, such as gesture detection */
	SUBCMD_SET_SHUTTER_PRIORITY,

	/* below subcmd can be called dynamically */
	/* IR enable: 0-off, 1-on */
	/* AE use this flag to decide the init AE value */
	SUBCMD_SET_IR_MODE,
	SUBCMD_SET_BANDINGTYPE,      /* 0-50Hz; 1-60Hz */
	/* set sensorhub's work mode, 0-normal;1-suspend */
	SUBCMD_SET_SYS_MODE,
	SUBCMD_SET_MANUAL_EXPOSURE, /* expo&gain */
	SUBCMD_SET_ROI,             /* face ROI */
	SUBCMD_SET_CROP_REGION,     /* crop region */

	MINIISP_SUBCMD_MAX,
};

struct msg_req_extend_swing_set {
	unsigned int extend_cmd;
	char         paras[PARA_MAX_LEN];
};

struct miniisp_event_sent_t {
	enum mini_event_info event_id;
	char event_params[PARA_MAX_LEN];
};

struct msg_ack_extend_swing_set {
	unsigned int extend_cmd;
	union {
		struct msg_ack_swing_cam query_ods;
	} ext_u;
};

struct miniisp_msg_t {
	unsigned int message_size;
	unsigned int api_name;
	unsigned int message_id;

	/* Alert   All the union member should not be greater than 100 */
	union {
		/* Request items. */
		/* match id */
		struct msg_req_query_swing_cam   req_query_swing_cam;
		/* capture */
		struct miniisp_req_request       req_miniisp_request;
		struct msg_req_config_swing_cam  req_usecase_cfg_swing_cam;
		struct msg_req_acquire_swing_cam req_acquire_swing_cam;
		struct msg_req_release_swing_cam req_release_swing_cam;
		struct msg_req_extend_swing_set  req_subcmd_swing_cam;
		struct msg_req_get_ao_otp_t      req_get_ao_otp;

		/* ack items */
		struct msg_ack_swing_cam         ack_query_swing_cam;
		struct miniisp_ack_request       ack_miniisp_request;
		struct msg_ack_config_swing_cam  ack_swing_cam_usecase_cfg;
		struct msg_ack_acquire_swing_cam ack_acquire_swing_cam;
		struct msg_ack_release_swing_cam ack_release_swing_cam;
		struct msg_ack_extend_swing_set  ack_extend_swing_set;
		struct msg_ack_get_ao_otp_t      ack_get_ao_otp;

		/* sent to ap */
		struct miniisp_event_sent_t      event_sent;
	} u;
};


#define SWING_CAM_DRV_NAME "hisilicon,swing-cam"
#define swing_cam_log_info(msg...) pr_info("[I/SWINGCAM]" msg)
#define swing_cam_log_warn(msg...) pr_warn("[W/SWINGCAM]" msg)
#define swing_cam_log_err(msg...) pr_err("[E/SWINGCAM]" msg)

struct swingcam_data {
	unsigned int recv_len;
	void *p_recv;
};

struct swingcam_priv {
	int ref_cnt;
	struct mutex swing_mutex; /* Used to protect ops on ioctl & open */
	struct kfifo read_kfifo;
	struct device *self; /* self device. */
};

struct miniisp_t {
	char const *name;
	struct platform_device *pdev;
	struct v4l2_subdev subdev;
	struct mutex lock;
};

#define SWING_READ_CACHE_COUNT 32

/* ioctl cmd define */
#define SWCAM_IO                   0xC1
#define SWING_IOCTL_CAM_OPEN       _IO(SWCAM_IO, 0xD1)
#define SWING_IOCTL_CAM_CLOSE      _IO(SWCAM_IO, 0xD2)
#define SWING_IOCTL_CAM_CONFIG     _IOWR(SWCAM_IO, 0xD3, struct miniisp_msg_t)
#define SWING_IOCTL_CAM_CAPTURE    _IOWR(SWCAM_IO, 0xD4, struct miniisp_msg_t)
#define SWING_IOCTL_CAM_MATCH_ID   _IOWR(SWCAM_IO, 0xD5, struct miniisp_msg_t)
#define SWING_IOCTL_CAM_RECV_RPMSG _IOWR(SWCAM_IO, 0xD6, struct miniisp_msg_t)
#define SWING_IOCTL_CAM_ACQUIRE    _IOWR(SWCAM_IO, 0xD7, struct miniisp_msg_t)
#define SWING_IOCTL_CAM_SET_EXTEND _IOWR(SWCAM_IO, 0xD8, struct miniisp_msg_t)
#define SWING_IOCTL_CAM_GET_OTP    _IOWR(SWCAM_IO, 0xD9, struct miniisp_msg_t)

#endif /* __LINUX_CONTEXTHUB_SWING_CAM_H__ */
