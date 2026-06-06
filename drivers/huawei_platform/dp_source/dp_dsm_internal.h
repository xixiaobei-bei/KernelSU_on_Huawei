/*
 * dp_dsm_internal.h
 *
 * internal definition for dp dsm
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __DP_DSM_INTERNAL_H__
#define __DP_DSM_INTERNAL_H__

#ifdef CONFIG_HUAWEI_DSM
#ifndef DP_DSM_ENABLE
#define DP_DSM_ENABLE
#endif
#endif

#define DP_DSM_BUF_SIZE 1024

// for print debug log
//#define DP_DSM_DEBUG

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef MIN
#define MIN(x, y)  ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y)  ((x) > (y) ? (x) : (y))
#endif

#define DP_DSM_DATETIME_SIZE         20
#define DP_DSM_TIME_YEAR_OFFSET      1900
#define DP_DSM_TIME_MONTH_OFFSET     1
#define DP_DSM_TIME_US_OF_MS         1000 // 1ms = 1000us
#define DP_DSM_TIME_MS_OF_SEC        1000 // 1s = 1000ms
#define DP_DSM_TIME_SECOND_OF_MINUTE 60
#define DP_DSM_TIME_SECOND_OF_HOUR   (60 * DP_DSM_TIME_SECOND_OF_MINUTE)
#define DP_DSM_TIME_SECOND_OF_DAY    (24 * DP_DSM_TIME_SECOND_OF_HOUR)

// for datetime type
#define DP_DSM_TIMEVAL_WITH_TZ \
	(sys_tz.tz_minuteswest * DP_DSM_TIME_SECOND_OF_MINUTE)
#define DP_DSM_TIME_MS_PERCENT(t)    ((t) / 100) // unit: ms -> 100ms

// EDID 1.3 data format
// 18 EDID version, usually 1 (for 1.3)
// 19 EDID revision, usually 3 (for 1.3)
#define DP_DSM_EDID_VERSION_OFFSET     18
#define DP_DSM_EDID_REVISION_OFFSET    19
#define DP_DSM_EDID_VERSION_MAKE(a, b) (((a) << 8) | (b))

// DPCD data structure revision number.
// DPCD_REV: 00000h
// 10h = DPCD Rev. 1.0.
// 11h = DPCD Rev. 1.1.
// 12h = DPCD Rev. 1.2.
// 13h = DPCD Rev. 1.3 (for eDP v1.4 DPRX only).
// 14h = DPCD Rev. 1.4.
// 3:0 Minor Revision Number
// 7:4 Major Revision Number
#define DP_DSM_DPCD_REVISION_OFFSET 4
#define DP_DSM_DPCD_REVISION_MASK   0x0F

// Manufacturer ID.
// These IDs are assigned by Microsoft,
// they are PNP IDs "00001=A¡±; ¡°00010=B¡±; ... ¡°11010=Z¡±.
// Bit 7 (at address 08h) is 0
// the first character (letter) is located at bits 6 -> 2 (at address 08h),
// the second character (letter) is located at bits 1 & 0 (at address 08h)
// and bits 7 -> 5 (at address 09h),
// and the third character (letter) is located at bits 4 -> 0 (at address 09h).
#define DP_DSM_MID_LETTER_NUM    3
#define DP_DSM_MID_LETTER_OFFSET 5
#define DP_DSM_MID_LETTER_MASK   0x1F

#define DP_DSM_MID_INFO_SIZE     (DP_DSM_MID_LETTER_NUM + 1)
#define DP_DSM_MID_OFFSET        8 // offset in edid block

#define DP_DSM_MID_LETTER(a, b)  (((a) << 8) | (b))

// Descriptor blocks.
// Detailed timing descriptors, in decreasing preference order.
// After all detailed timing descriptors, additional descriptors are permitted:
// Monitor range limits (required)
// ASCII text(monitor name(required),monitor serial number or unstructured text)
// 6 Additional standard timing information blocks
// Colour point data
// 54  C71       Descriptor 1
// 72  C89       Descriptor 2
// 90  C107      Descriptor 3
// 108  C125     Descriptor 4
#define EDID_DESC_OFFSET         0x36 // from 54 bytes
#define EDID_DESC_NUM            4
#define EDID_DESC_SIZE           18

#define EDID_DESC_MASK_OFFSET    3
#define EDID_DESC_MONITOR_MASK   0xfc
#define EDID_DESC_MONITOR_OFFSET 5
#define EDID_DESC_MONITOR_TAIL   0x0a

#define DP_DSM_MONTIOR_INFO_SIZE 16
#define DP_DSM_TU_FAIL           65

#define DP_DSM_TYPEC_IN_OUT_NUM  8
#define DP_DSM_TCA_DEV_TYPE_NUM  32
#define DP_DSM_IRQ_VECTOR_NUM    32
#define DP_DSM_SOURCE_SWITCH_NUM 8
#define DP_DSM_EARLIEST_TIME_POINT(a, b) (((a) >= (b)) ? ((a) % (b)) : 0)

// error number:
// 1. hotplug retval
#define DP_DSM_ERRNO_DSS_PWRON_FAILED   0xFFFF
#define DP_DSM_ERRNO_HPD_NOT_EXISTED    (0xFFFF + 1)
// 2. link training retval
#define DP_DSM_ERRNO_DEVICE_SRS_FAILED  0xFFFF

// from tca.h (vendor\hisi\ap\kernel\include\linux\hisi\contexthub)
enum tca_irq_type {
	TCA_IRQ_HPD_OUT = 0,
	TCA_IRQ_HPD_IN = 1,
	TCA_IRQ_SHORT = 2,
	TCA_IRQ_MAX_NUM
};

enum tcpc_mux_ctrl_type {
	TCPC_NC = 0,
	TCPC_USB31_CONNECTED = 1,
	TCPC_DP = 2,
	TCPC_USB31_AND_DP_2LINE = 3,
	TCPC_MUX_MODE_MAX
};

enum tca_device_type {
	TCA_CHARGER_CONNECT_EVENT = 0, // usb device in
	TCA_CHARGER_DISCONNECT_EVENT,  // usb device out
	TCA_ID_FALL_EVENT,             // usb host in
	TCA_ID_RISE_EVENT,             // usb host out
	TCA_DP_OUT,
	TCA_DP_IN,
	TCA_DEV_MAX
};

enum typec_plug_orien {
	TYPEC_ORIEN_POSITIVE = 0,
	TYPEC_ORIEN_NEGATIVE = 1,
	TYPEC_ORIEN_MAX
};

// hpd or short irq
struct dp_connected_event {
	struct list_head list;
	struct timeval time;
	enum tca_irq_type irq_type;
	enum tcpc_mux_ctrl_type cur_mode;
	enum tcpc_mux_ctrl_type mode_type;
	enum tca_device_type dev_type;
	enum typec_plug_orien typec_orien;
};

enum dp_hdcp_key {
	HDCP_KEY_INVALID,
	HDCP_KEY_SUCCESS,
	HDCP_KEY_FAILED,
};

union dp_vs_pe {
	uint32_t vswing_preemp;
	struct {
		uint8_t preemp:4; // low 4 bits
		uint8_t vswing:4; // high 4 bits
	} vs_pe[DP_DSM_VS_PE_NUM];
};

enum dp_edid_check_result {
	EDID_CHECK_SUCCESS,
	EDID_CHECK_FAILED,
};

union dp_edid_check {
	struct {
		uint32_t header_num;
		uint32_t checksum_num;
		uint32_t header_flag;
		uint32_t checksum_flag;
		uint8_t ext_blocks_num;
		uint8_t ext_blocks;
	} u32;

	struct {
		uint8_t header_num[DP_PARAM_EDID_NUM];
		uint8_t checksum_num[DP_PARAM_EDID_NUM];
		uint8_t header_flag[DP_PARAM_EDID_NUM];
		uint8_t checksum_flag[DP_PARAM_EDID_NUM];
		uint8_t ext_blocks_num;
		uint8_t ext_blocks;
	} u8;
};

// dp hotplug info
struct dp_hotplug_info {
	struct list_head list;
	struct timeval time[DP_PARAM_TIME_NUM];

	// dp info
	char mid[DP_DSM_MID_INFO_SIZE]; // manufacturer id
	char monitor[DP_DSM_MONTIOR_INFO_SIZE];

	uint16_t width;
	uint16_t high;
	uint16_t max_width;
	uint16_t max_high;
	int pixel_clock;
	uint8_t fps;
	uint8_t max_rate;  // max_lanes_rate
	uint8_t max_lanes;
	uint8_t rate;      // link_lanes_rate
	uint8_t lanes;
	int tu;
	int tu_fail;
	union dp_vs_pe vp; // vs_pe

	bool source_mode;    // same_source or diff_source
	uint8_t user_mode;   // vesa_id
	uint8_t user_format; // vcea, cvt, dmt
	uint8_t basic_audio;
	uint16_t edid_version;
	uint8_t dpcd_revision;

	bool safe_mode;
	bool no_hotunplug;
	int read_edid_retval;
	int link_training_retval;
	int hotplug_retval;

	// extend info
	int current_edid_num;
	union dp_edid_check edid_check;
	uint8_t edid[DP_PARAM_EDID_NUM][DP_DSM_EDID_BLOCK_SIZE];

	// for debug
	uint8_t vs_force;
	uint8_t pe_force;
	union dp_vs_pe vp_force;
};

struct dp_timing_info {
	struct list_head list;
	uint16_t h_active;
	uint16_t v_active;
	uint32_t pixel_clock;
	uint8_t vesa_id;
};

struct dp_aux_rw {
	struct timeval time;
	bool rw;
	bool i2c;
	uint32_t addr;
	uint32_t len;
	int retval;
};

struct dp_typec_in_out {
	int in_out_num;
	struct timeval in_time[DP_DSM_TYPEC_IN_OUT_NUM];
	struct timeval out_time[DP_DSM_TYPEC_IN_OUT_NUM];
};


struct dp_dsm_priv {
	struct list_head list;
	enum dp_imonitor_type type;

	// typec cable connected
	struct timeval tpyec_in_time;
	struct timeval tpyec_out_time;
	struct timeval link_min_time;
	struct timeval link_max_time;
	struct timeval same_mode_time;
	struct timeval diff_mode_time;
	int event_num;
	enum tcpc_mux_ctrl_type tpyec_cable_type;
	struct list_head event_list_head;

	// dp in or out
	int irq_hpd_num;
	int irq_short_num;
	int dp_in_num;
	int dp_out_num;
	uint8_t tca_dev_type[DP_DSM_TCA_DEV_TYPE_NUM];

	// from handle_sink_request()
	int link_retraining_num;

	// dp hotplug
	bool need_add_hotplug_to_list;
	struct dp_hotplug_info *cur_hotplug_info;
	struct list_head hotplug_list_head;
	struct list_head timing_list_head;

	bool is_hotplug_running;
	uint32_t hotplug_state;
	uint32_t last_hotplug_state;
	uint16_t hotplug_width;
	uint16_t hotplug_high;

	bool cur_source_mode;
	bool dss_pwron_failed;
	int link_training_retval;
	int link_retraining_retval;
	int hotplug_retval;
	struct dp_aux_rw aux_rw;

	int irq_vector_num;
	uint8_t irq_vector[DP_DSM_IRQ_VECTOR_NUM];
	int source_switch_num;
	uint8_t source_switch[DP_DSM_SOURCE_SWITCH_NUM];

	// hdcp
	struct timeval hdcp_time;
	uint8_t hdcp_version; // hdcp1.3 or hdcp2.2
	uint8_t hdcp_key;

	// for report control flag
	bool rcf_basic_info_of_same_mode;
	bool rcf_basic_info_of_diff_mode;
	bool rcf_extend_info;
	bool rcf_link_training_info;
	bool rcf_link_retraining_info;
	bool rcf_hotplug_info;
	bool rcf_hdcp_info;

	// for repeated event recognition
	int hpd_repeated_num;

	// err_count: from dpcd regs in sink devices
	uint16_t lane0_err;
	uint16_t lane1_err;
	uint16_t lane2_err;
	uint16_t lane3_err;

#ifdef DP_DEBUG_ENABLE
	bool need_lanes_force;
	bool need_rate_force;
	bool need_resolution_force;
	uint8_t lanes_force;
	uint8_t rate_force;
	uint8_t user_mode;
	uint8_t user_format;
#endif
};

#ifndef DP_DSM_DEBUG
#ifndef DP_DEBUG_ENABLE
#define DP_DSM_REPORT_NUM_IN_ONE_DAY 5    // for user version
#else
#define DP_DSM_REPORT_NUM_IN_ONE_DAY 1000 // for eng version
#endif // DP_DEBUG_ENABLE
#define DP_DSM_REPORT_TIME_INTERVAL \
	(DP_DSM_TIME_SECOND_OF_DAY * DP_DSM_TIME_MS_OF_SEC)
#else
#define DP_DSM_REPORT_NUM_IN_ONE_DAY 5
#define DP_DSM_REPORT_TIME_INTERVAL  (300 * 1000) // 5 minutes
#endif // DP_DSM_DEBUG
#define DP_DSM_IMONITOR_DELAYED_TIME 0
#define DP_IMONITOR_REPORT_PRIV(g, d) (((d) == NULL) ? (g) : (d))

#define DP_DSM_HPD_DELAYED_TIME  (3 * 1000)  // 3s
#define DP_DSM_HPD_TIME_INTERVAL (10 * 1000) // 10s

#define DP_DSM_GET_LINK_MIN_TIME(m, t) do { \
	if (m.tv_usec == 0) \
		m.tv_usec = t.tv_usec; \
	else \
		m.tv_usec = MIN(m.tv_usec, t.tv_usec); \
} while (0)

#define DP_DSM_GET_LINK_MAX_TIME(m, t) \
	m.tv_usec = MAX(m.tv_usec, t.tv_usec)

struct dp_imonitor_report_info {
	struct list_head list_head;
	struct mutex lock;
	struct delayed_work imonitor_work;

	// check whether hpd existed or not
	struct delayed_work hpd_work;
	bool hpd_existed;
	unsigned long hpd_jiffies;
	uint32_t report_num[DP_IMONITOR_TYPE_NUM];
	unsigned long report_jiffies;
	// report-skip data of the last day
	bool report_skip_existed;

	uint32_t last_report_num[DP_IMONITOR_TYPE_NUM];
	struct timeval last_link_min_time;
	struct timeval last_link_max_time;
	struct timeval last_same_mode_time;
	struct timeval last_diff_mode_time;
	int last_source_switch_num;
};

typedef int (*imonitor_prepare_param_cb_t) (struct imonitor_eventobj *, void *);

struct dp_imonitor_event_id {
	enum dp_imonitor_type type;
	unsigned int event_id;
	imonitor_prepare_param_cb_t prepare_cb;
};

struct dp_dmd_error_no {
	enum dp_dmd_type type;
	int error_no;
};

// imonitor no.
#define DP_IMONITOR_BASIC_INFO_NO    936000102
#define DP_IMONITOR_TIME_INFO_NO     936000103
#define DP_IMONITOR_EXTEND_INFO_NO   936000104
#define DP_IMONITOR_HPD_NO           936000105
#define DP_IMONITOR_LINK_TRAINING_NO 936000106
#define DP_IMONITOR_HOTPLUG_NO       936000107
#define DP_IMONITOR_HDCP_NO          936000108

#define DP_IMONITOR_EVENT_ID(t, f)   { t, t##_NO, f }

#define DP_DSM_BIN_TO_STR_SHIFT4 4
#define DP_DSM_BIN_TO_STR_SHIFT8 8
#define DP_DSM_BIN_TO_STR_TIMES \
	(DP_DSM_BIN_TO_STR_SHIFT8 / DP_DSM_BIN_TO_STR_SHIFT4)

#define DP_DSM_EDID_HEADER_SIZE      8
#define DP_DSM_EDID_EXT_HEADER       0x02
#define DP_DSM_EDID_EXT_BLOCKS_INDEX 126
#define DP_DSM_EDID_EXT_BLOCKS_MAX   3
#define DP_DSM_PARAM_NAME_MAX        16

#define DP_DSM_FREE_HOTPLUG_PTR(p) do { \
	if (p->cur_hotplug_info != NULL) { \
		kfree(p->cur_hotplug_info); \
		p->cur_hotplug_info = NULL; \
	} \
} while (0)

#define DP_DSM_ADD_TO_IMONITOR_LIST(p, f, t) \
do { \
	if (p->f) { \
		p->f = false; \
		dp_add_info_to_imonitor_list(t, p); \
	} \
} while (0)

#define DP_CHECK_LINK_STATE(p, s) \
	(p->hotplug_state & (1 << s))
#define DP_REPORT_LINK_STATE(p, s) \
do { \
	dp_link_state_event(s); \
	p->last_hotplug_state |= (1 << s); \
} while (0)

#define DP_SET_PARAM_I(o, s, v) \
	imonitor_set_param_integer_v2(o, s, (long)v)

#define DP_SET_PARAM_S(o, s, v) \
	imonitor_set_param_string_v2(o, s, v)

#ifdef DP_DEBUG_ENABLE
struct dp_dsm_priv *dp_get_dp_dsm_priv(void);
struct dp_typec_in_out *dp_get_typec_in_out(void);
struct dp_imonitor_report_info *dp_get_imonitor_report(void);
int dp_timeval_to_time_str(struct timeval time, const char *comment,
	char *str, int size);
int dp_timeval_to_datetime_str(struct timeval time, const char *comment,
	char *str, int size);
#endif // DP_DEBUG_ENABLE

#endif // __DP_DSM_INTERNAL_H__
