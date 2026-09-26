/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * Description: sensor detect source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */
#include "sensor_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/types.h>

#include <chipset_common/hwpower/common_module/power_cmdline.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif
#include <huawei_platform/inputhub/sensorhub.h>
#ifdef CONFIG_HUAWEI_SHB_THP
#include <huawei_platform/inputhub/thphub.h>
#endif
#include <securec.h>

#include "acc_sysfs.h"
#include "airpress_sysfs.h"
#include "als_sysfs.h"
#include "cap_prox_sysfs.h"
#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "gyro_sysfs.h"
#include "handpress_detect.h"
#include "mag_sysfs.h"
#include "motion_detect.h"
#include "protocol.h"
#include "ps_sysfs.h"
#include "sensor_config.h"
#include "sensor_feima.h"
#include "sensor_sysfs.h"
#ifdef CONFIG_CONTEXTHUB_SHMEM
#include "shmem.h"
#endif
#include "therm_sysfs.h"
#include "vibrator_detect.h"

#define ADAPT_FILE_ID_NUM 32
#define ADAPT_SENSOR_LIST_NUM 32
#define WIA_NUM 6

#define GPIO_STAT_HIGH        1
#define GPIO_STAT_LOW         0
#define RESET_SHORT_SLEEP     5
#define RESET_LONG_SLEEP      10
#define FPC_NAME_LEN          3
#define SYNA_NAME_LEN         4
#define GOODIX_NAME_LEN       6
#define GF_G2_G3_NAME_LEN    15
#define SILEAD_NAME_LEN       6
#define QFP_NAME_LEN          3
#define EGIS_NAME_LEN         4
#define FP_SPI_NUM            2
#define RET_FAIL              (-1)
#define RET_SUCC              0

#define F03_WRITE_CMD           0xF0
#define F03_CHIP_ID_ADDR_HIGH   0x00
#define F03_CHIP_ID_ADDR_LOW    0x00
#define F03_WRITE_CMD_TX_LEN    3
#define F03003_WAKE_RETRY_TIMES 5
#define WORD_LEN_LOW_BIT  0x01
#define WORD_LEN_HIGH_BIT 0x00
#define F03_WRITE_WAKEUP_CMD_TX_LEN 7

#define PHONE_TYPE_LIST 2

#define I2C_ADDR_NXP 0x35
#define I2C_ADDR_AWINIC 0x31
#define I2C_ADDR_CIRRUS 0x50
#define NXP 1
#define AWINIC 2
#define CIRRUS 3
uint32_t spa_manufacture = NXP;

static struct sensor_redetect_state s_redetect_state;
static struct wakeup_source sensor_rd;
static struct work_struct redetect_work;
static char g_pkg_buf[MAX_PKT_LENGTH];
static char aux_buf[MAX_PKT_LENGTH];
static pkt_sys_dynload_req_t *dyn_req = (pkt_sys_dynload_req_t *)g_pkg_buf;
static pkt_sys_dynload_req_t *aux_req = (pkt_sys_dynload_req_t *)aux_buf;
struct sleeve_detect_pare sleeve_detect_paremeter[MAX_PHONE_COLOR_NUM];
struct sensorlist_info sensorlist_info[SENSOR_MAX];
uint8_t tof_register_value;
static uint16_t sensorlist[SENSOR_LIST_NUM];
static int sensor_tof_flag;
static int hifi_supported;
uint8_t is_close;
static int hall_number = 1;
static int hall_sen_type;
static int power_off_charging_posture = -1; // init invalid value
static int support_hall_hishow;
static int support_hall_pen;               /* defalut not support pen */
static int support_hall_keyboard;          /* defalut not support keyboard */
static int support_hall_lightstrap;
static uint32_t hall_hishow_value = 0x5;   /* hishow default trigger mask */
static uint32_t hall_pen_value = 0x10;     /* pen default trigger mask */
static uint32_t hall_keyboard_value = 0x4; /* keyboard default trigger mask */
static uint32_t hall_lightstrap_value = 0x2; /* 0010b, lightstrap trigger mask */
int gyro_detect_flag;

struct tof_platform_data tof_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.tof_calib_zero_threshold = 20000, /* zero threshold 20000 */
	.tof_calib_6cm_threshold = 10000, /* 6cm threshold 10000 */
	.tof_calib_10cm_threshold = 10000, /* 10cm threshold 10000 */
	.tof_calib_60cm_threshold = 10000, /* 60cm threshold 10000 */
};

static struct connectivity_platform_data connectivity_data = {
	.cfg = {
		.bus_type = TAG_I3C,
		.bus_num = 1,
		.disable_sample_thread = 1,
		{ .i2c_address = 0x6E },
	},
	.poll_interval = 50, /* interval 50 ms */
	.gpio1_gps_cmd_ap = 200, /* gpio1 gps cmd ap 200 */
	.gpio1_gps_cmd_sh = 230, /* gpio1 gps cmd sh 230 */
	.gpio2_gps_ready_ap = 213, /* gpio2 gps ready ap 213 */
	.gpio2_gps_ready_sh = 242, /* gpio2 gps ready sh 242 */
	.gpio3_wakeup_gps_ap = 214, /* gpio3 wakeup gps ap 214 */
	.gpio3_wakeup_gps_sh = 243, /* gpio3 wakeup gps sh 243 */
	.i3c_frequency = 0,
	.gpio1_gps_cmd_pinmux = 2, /* gpio1 gps cmd pinmux 2 */
	.gpio2_gps_ready_pinmux = 2, /* gpio2 gps ready pinmux 2 */
	.gpio3_wakeup_gps_pinmux = 4, /* gpio3 wakeup gps pinmux 4 */
};

static struct fingerprint_platform_data fingerprint_data = {
	.cfg = {
		.bus_type = TAG_SPI,
		.bus_num = 2, /* bus num 2 */
		.disable_sample_thread = 1,
		{ .ctrl = { .data = 218 } }, /* ctrl data 218 */
	},
	.reg = 0xFC,
	.chip_id = 0x021b,
	.product_id = 9, /* product id 9 */
	.gpio_irq = 207, /* gpio irq 207 */
	.gpio_irq_sh = 236, /* gpio irq sh 236 */
	.gpio_reset = 149, /* gpio reset 149 */
	.gpio_reset_sh = 1013, /* gpio reset sh 1013 */
	.gpio_cs = 218, /* gpio cs 218 */
	.poll_interval = 50, /* poll interval 50 ms */
};

static struct fingerprint_platform_data fingerprint_ud_data = {
	.cfg = {
		.bus_type = TAG_SPI,
		.bus_num = 2, /* bus num 2 */
		.disable_sample_thread = 1,
		{ .ctrl = { .data = 218 } }, /* ctrl data 218 */
	},
	.reg = 0xF1,
	.chip_id = 0x1204,
	.product_id = 35, /* product id 35 */
	.gpio_irq = 147, /* gpio irq 147 */
	.gpio_irq_sh = 1020, /* gpio irq sh 1020 */
	.gpio_reset = 211, /* gpio reset 211 */
	.gpio_cs = 216, /* gpio cs 216 */
	.poll_interval = 50, /* poll interval 50 ms */
	.tp_hover_support = 0,
};

static struct tp_ud_platform_data tp_ud_data = {
	.cfg = {
		.bus_type = TAG_I3C,
		.bus_num = 0,
		.disable_sample_thread = 1,
		{ .i2c_address = 0x70 },
	},
	.gpio_irq = 226, /* gpio irq 226 */
	.gpio_irq_sh = 1000, /* gpio irq sh 1000 */
	.gpio_cs  = 236, /* gpio cs 236 */
	.gpio_irq_pull_up_status = 0,
	.pressure_support = 1,
	.anti_forgery_support = 0,
	.spi_max_speed_hz = 10000000, /* spi max speed hz 10000000 */
	.spi_mode = 0,
	.ic_type = 0,
	.hover_enable = 1,
	.i2c_max_speed_hz = 0,
	.tp_sensorhub_platform = 4, /* default: orlando platform 4 */
	.aod_display_support = 0,
	.tsa_event_to_udfp = 0,
};

static struct tp_ud_platform_data tp_ud_data_extra = {
	.cfg = {
		.bus_type = TAG_I3C,
		.bus_num = 0,
		.disable_sample_thread = 1,
		{ .i2c_address = 0x70 },
	},
	.gpio_irq = 226, /* gpio irq 226 */
	.gpio_irq_sh = 1000, /* gpio irq sh 1000 */
	.gpio_cs  = 236, /* gpio cs 236 */
	.gpio_irq_pull_up_status = 0,
	.pressure_support = 1,
	.anti_forgery_support = 0,
	.spi_max_speed_hz = 10000000, /* spi max speed hz 10000000 */
	.spi_mode = 0,
	.ic_type = 0,
	.hover_enable = 1,
	.i2c_max_speed_hz = 0,
	.tp_sensorhub_platform = 4, /* default: orlando platform 4 */
	.aod_display_support = 0,
};

static struct key_platform_data key_data = {
	.cfg = {
		.bus_type = TAG_I2C,
		.bus_num = 0,
		.disable_sample_thread = 0,
		{ .i2c_address = 0x27 },
	},
	.i2c_address_bootloader = 0x28,
	.poll_interval = 30, /* poll interval 30 ms */
};

struct magn_bracket_platform_data magn_bracket_data = {
	.cfg = DEF_SENSOR_COM_SETTING, /* donot use, just give it a value */
	.mag_x_change_lower = 0,
	.mag_x_change_upper = 0,
	.mag_y_change_lower = 0,
	.mag_y_change_upper = 0,
	.mag_z_change_lower = 0,
	.mag_z_change_upper = 0,
};

struct aod_platform_data aod_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.feature_set = { 0 },
};

struct rpc_platform_data rpc_data = {
	.table = { 0 },
	.mask = { 0 },
	.default_value = 0,
	.mask_enable = 0,
	.sar_choice = 0,
	.sim_type_swtich_flag = 0,
	.fusion_type = 0,
};

struct kb_platform_data g_kb_data = {
	.uart_num = KB_DEFAULT_UART_NUM,
	.kb_detect_adc_num = KB_DEFAULT_DETECT_ADC_NUM,
	.kb_disconnect_adc_min = KB_DEFAULT_DISCONNECT_ADC_VOL,
	.kb_disable_angle = 0,
};

struct thp_platform_data thp_data = {
	.cfg = {
		.bus_type = TAG_SPI,
		.bus_num = 0,
		.disable_sample_thread = 1,
		{ .i2c_address = 0x70 },
	},
	.gpio_irq = 279, /* gpio irq 279 */
	.gpio_irq_sh = 1024, /* gpio irq sh 1024 */
	.gpio_cs  = 177, /* gpio cs 177 */
	.feature_config = 0,
	.anti_forgery_support = 0,
	.spi_max_speed_hz = 6000000, /* spi max speed h 6000000 */
	.spi_mode = 0,
	.ic_type = 0,
	.hover_enable = 1,
	.i2c_max_speed_hz = 0,
};

static struct modem_platform_data g_modem_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.mode = 0,
};

/*lint +e785*/
struct sensor_detect_manager sensor_manager[SENSOR_MAX] = {
	{ "acc", ACC, DET_INIT, TAG_ACCEL, NULL, 0 },
	{ "mag", MAG, DET_INIT, TAG_MAG, NULL, 0 },
	{ "gyro", GYRO, DET_INIT, TAG_GYRO, NULL, 0 },
	{ "als", ALS, DET_INIT, TAG_ALS, NULL, 0 },
	{ "ps", PS, DET_INIT, TAG_PS, NULL, 0 },
	{ "airpress", AIRPRESS, DET_INIT, TAG_PRESSURE, NULL, 0 },
	{ "handpress", HANDPRESS, DET_INIT, TAG_HANDPRESS, NULL, 0 },
	{ "cap_prox", CAP_PROX, DET_INIT, TAG_CAP_PROX, NULL, 0 },
	{ "connectivity", CONNECTIVITY, DET_INIT, TAG_CONNECTIVITY, &connectivity_data, sizeof(connectivity_data) },
	{ "fingerprint", FINGERPRINT, DET_INIT, TAG_FP, &fingerprint_data, sizeof(fingerprint_data) },
	{ "key", KEY, DET_INIT, TAG_KEY, &key_data, sizeof(key_data) },
	{ "hw_magn_bracket", MAGN_BRACKET, DET_INIT, TAG_MAGN_BRACKET, &magn_bracket_data, sizeof(magn_bracket_data) },
	{ "rpc", RPC, DET_INIT, TAG_RPC, &rpc_data, sizeof(rpc_data) },
	{ "vibrator", VIBRATOR, DET_INIT, TAG_VIBRATOR, NULL, 0 },
	{ "fingerprint_ud", FINGERPRINT_UD, DET_INIT, TAG_FP_UD, &fingerprint_ud_data, sizeof(fingerprint_ud_data) },
	{ "tof", TOF, DET_INIT, TAG_TOF, &tof_data, sizeof(tof_data) },
	{ "tp_ud", TP_UD, DET_INIT, TAG_TP, &tp_ud_data, sizeof(tp_ud_data) },
	{ "sh_aod", SH_AOD, DET_INIT, TAG_AOD, &aod_data, sizeof(aod_data) },
	{ "acc1", ACC1, DET_INIT, TAG_ACC1, NULL, 0 },
	{ "gyro1", GYRO1, DET_INIT, TAG_GYRO1, NULL, 0 },
	{ "als1", ALS1, DET_INIT, TAG_ALS1, NULL, 0 },
	{ "mag1", MAG1, DET_INIT, TAG_MAG1, NULL, 0 },
	{ "als2", ALS2, DET_INIT, TAG_ALS2, NULL, 0 },
	{ "cap_prox1", CAP_PROX1, DET_INIT, TAG_CAP_PROX1, NULL, 0 },
	{ "kb", KB, DET_INIT, TAG_KB, &g_kb_data, sizeof(g_kb_data) },
	{ "motion", MOTION, DET_INIT, TAG_MOTION, NULL, 0 },
	{ "thp", THP, DET_INIT, TAG_THP, &thp_data, sizeof(thp_data) },
	{ "iomcu_modem", IOMCU_MODEM, DET_INIT, TAG_MODEM, &g_modem_data, sizeof(g_modem_data) },
	{ "sound", SOUND, DET_INIT, TAG_SOUND, NULL, 0 },
	{ "thermometer", THERMOMETER, DET_INIT, TAG_THERMOMETER, NULL, 0 },
	{ "tp_ud_extra", TP_UD_EXTRA, DET_INIT, TAG_TP, &tp_ud_data_extra,
		sizeof(tp_ud_data_extra) },
};

static const struct app_link_info app_link_info_gyro[] = {
	{ SENSORHUB_TYPE_ACCELEROMETER, TAG_ACCEL, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_LIGHT, TAG_ALS, 1, { TAG_ALS } },
	{ SENSORHUB_TYPE_PROXIMITY, TAG_PS, 1, { TAG_PS } },
	{ SENSORHUB_TYPE_GYROSCOPE, TAG_GYRO, 1, { TAG_GYRO } },
	{ SENSORHUB_TYPE_GRAVITY, TAG_GRAVITY, 3, { TAG_ACCEL, TAG_GYRO, TAG_MAG } },
	{ SENSORHUB_TYPE_MAGNETIC, TAG_MAG, 2, { TAG_GYRO, TAG_MAG, } },
	{ SENSORHUB_TYPE_LINEARACCELERATE, TAG_LINEAR_ACCEL, 3, { TAG_ACCEL, TAG_GYRO, TAG_MAG } },
	{ SENSORHUB_TYPE_ORIENTATION, TAG_ORIENTATION, 3, { TAG_ACCEL, TAG_GYRO, TAG_MAG } },
	{ SENSORHUB_TYPE_ROTATEVECTOR, TAG_ROTATION_VECTORS, 3, { TAG_ACCEL, TAG_GYRO, TAG_MAG } },
	{ SENSORHUB_TYPE_PRESSURE, TAG_PRESSURE, 1, { TAG_PRESSURE } },
	{ SENSORHUB_TYPE_HALL, TAG_HALL, 0, { 0 } },
	{ SENSORHUB_TYPE_MAGNETIC_FIELD_UNCALIBRATED, TAG_MAG_UNCALIBRATED, 2, { TAG_MAG, TAG_GYRO } },
	{ SENSORHUB_TYPE_GAME_ROTATION_VECTOR, TAG_GAME_RV, 2, { TAG_ACCEL, TAG_GYRO } },
	{ SENSORHUB_TYPE_GYROSCOPE_UNCALIBRATED, TAG_GYRO_UNCALIBRATED, 1, { TAG_GYRO } },
	{ SENSORHUB_TYPE_SIGNIFICANT_MOTION, TAG_SIGNIFICANT_MOTION, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_STEP_DETECTOR, TAG_STEP_DETECTOR, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_STEP_COUNTER, TAG_STEP_COUNTER, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_GEOMAGNETIC_ROTATION_VECTOR, TAG_GEOMAGNETIC_RV, 3, { TAG_ACCEL, TAG_GYRO, TAG_MAG } },
	{ SENSORHUB_TYPE_HANDPRESS, TAG_HANDPRESS, 1, { TAG_HANDPRESS } },
	{ SENSORHUB_TYPE_CAP_PROX, TAG_CAP_PROX, 1, { TAG_CAP_PROX } },
	{ SENSORHUB_TYPE_PHONECALL, TAG_PHONECALL, 2, { TAG_ACCEL, TAG_PS } },
	{ SENSORHUB_TYPE_MAGN_BRACKET, TAG_MAGN_BRACKET, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_HINGE, TAG_HINGE, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_META_DATA, TAG_FLUSH_META, 0, { 0 } },
	{ SENSORHUB_TYPE_RPC, TAG_RPC, 4, { TAG_ACCEL, TAG_GYRO, TAG_CAP_PROX, TAG_CAP_PROX1 } },
	{ SENSORHUB_TYPE_AGT, TAG_AGT, 0, { 0 } },
	{ SENSORHUB_TYPE_COLOR, TAG_COLOR, 0, { 0 } },
	{ SENSORHUB_TYPE_ACCELEROMETER_UNCALIBRATED, TAG_ACCEL_UNCALIBRATED, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_TOF, TAG_TOF, 1, { TAG_TOF } },
	{ SENSORHUB_TYPE_DROP, TAG_DROP, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_POSTURE, TAG_POSTURE, 2, { TAG_ACCEL, TAG_GYRO } },
	{ SENSORHUB_TYPE_EXT_HALL, TAG_EXT_HALL, 1, { TAG_MAG1 } },
	{ SENSORHUB_TYPE_ACCELEROMETER1, TAG_ACC1, 1, { TAG_ACC1 } },
	{ SENSORHUB_TYPE_GYROSCOPE1, TAG_GYRO1, 1, { TAG_GYRO1 } },
	{ SENSORHUB_TYPE_PROXIMITY1, TAG_PS1, 0, { 0 } },
	{ SENSORHUB_TYPE_LIGHT1, TAG_ALS1, 1, { TAG_ALS1 } },
	{ SENSORHUB_TYPE_MAGNETIC1, TAG_MAG1, 0, { 0 } },
	{ SENSORHUB_TYPE_LIGHT2, TAG_ALS2, 1, { TAG_ALS2 } },
	{ SENSORHUB_TYPE_CAP_PROX1, TAG_CAP_PROX1, 1, { TAG_CAP_PROX1 } },
};

static const struct app_link_info app_link_info_no_gyro[] = {
	{ SENSORHUB_TYPE_ACCELEROMETER, TAG_ACCEL, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_LIGHT, TAG_ALS, 1, { TAG_ALS } },
	{ SENSORHUB_TYPE_PROXIMITY, TAG_PS, 1, { TAG_PS } },
	{ SENSORHUB_TYPE_GYROSCOPE, TAG_GYRO, 2, { TAG_MAG, TAG_ACCEL } },
	{ SENSORHUB_TYPE_GRAVITY, TAG_GRAVITY, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_MAGNETIC, TAG_MAG, 1, { TAG_MAG } },
	{ SENSORHUB_TYPE_LINEARACCELERATE, TAG_LINEAR_ACCEL, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_ORIENTATION, TAG_ORIENTATION, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_ROTATEVECTOR, TAG_ROTATION_VECTORS, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_PRESSURE, TAG_PRESSURE, 1, { TAG_PRESSURE } },
	{ SENSORHUB_TYPE_HALL, TAG_HALL, 0, {0} },
	{ SENSORHUB_TYPE_MAGNETIC_FIELD_UNCALIBRATED, TAG_MAG_UNCALIBRATED, 1, { TAG_MAG } },
	{ SENSORHUB_TYPE_GAME_ROTATION_VECTOR, TAG_GAME_RV, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_GYROSCOPE_UNCALIBRATED, TAG_GYRO_UNCALIBRATED, 0, { 0 } },
	{ SENSORHUB_TYPE_SIGNIFICANT_MOTION, TAG_SIGNIFICANT_MOTION, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_STEP_DETECTOR, TAG_STEP_DETECTOR, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_STEP_COUNTER, TAG_STEP_COUNTER, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_GEOMAGNETIC_ROTATION_VECTOR, TAG_GEOMAGNETIC_RV, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_HANDPRESS, TAG_HANDPRESS, 1, { TAG_HANDPRESS } },
	{ SENSORHUB_TYPE_CAP_PROX, TAG_CAP_PROX, 1, { TAG_CAP_PROX } },
	{ SENSORHUB_TYPE_PHONECALL, TAG_PHONECALL, 2, { TAG_ACCEL, TAG_PS } },
	{ SENSORHUB_TYPE_MAGN_BRACKET, TAG_MAGN_BRACKET, 2, { TAG_ACCEL, TAG_MAG } },
	{ SENSORHUB_TYPE_HINGE, TAG_HINGE, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_META_DATA, TAG_FLUSH_META, 0, { 0 } },
	{ SENSORHUB_TYPE_RPC, TAG_RPC, 3, { TAG_ACCEL, TAG_CAP_PROX, TAG_CAP_PROX1 } },
	{ SENSORHUB_TYPE_AGT, TAG_AGT, 0, { 0 } },
	{ SENSORHUB_TYPE_COLOR, TAG_COLOR, 0, { 0 } },
	{ SENSORHUB_TYPE_ACCELEROMETER_UNCALIBRATED, TAG_ACCEL_UNCALIBRATED, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_TOF, TAG_TOF, 1, { TAG_TOF } },
	{ SENSORHUB_TYPE_DROP, TAG_DROP, 1, { TAG_ACCEL } },
	{ SENSORHUB_TYPE_POSTURE, TAG_POSTURE, 0, { 0 } },
	{ SENSORHUB_TYPE_EXT_HALL, TAG_EXT_HALL, 1, { TAG_MAG1 } },
	{ SENSORHUB_TYPE_ACCELEROMETER1, TAG_ACC1, 1, { TAG_ACC1 } },
	{ SENSORHUB_TYPE_GYROSCOPE1, TAG_GYRO1, 0, { 0 } },
	{ SENSORHUB_TYPE_PROXIMITY1, TAG_PS1, 0, { 0 } },
	{ SENSORHUB_TYPE_LIGHT1, TAG_ALS1, 1, { TAG_ALS1 } },
	{ SENSORHUB_TYPE_MAGNETIC1, TAG_MAG1, 0, { 0 } },
	{ SENSORHUB_TYPE_LIGHT2, TAG_ALS2, 1, { TAG_ALS2 } },
	{ SENSORHUB_TYPE_CAP_PROX1, TAG_CAP_PROX1, 1, { TAG_CAP_PROX1 } },
};

struct f03_reg_info {
	uint16_t addr;
	uint16_t value;
	uint16_t delay;
};

struct sensor_detect_manager *get_sensor_manager(void)
{
	return sensor_manager;
}

int get_hall_number(void)
{
	return hall_number;
}

int get_hall_sen_type(void)
{
	return hall_sen_type;
}

int get_support_hall_hishow(void)
{
	return support_hall_hishow;
}

int get_support_hall_pen(void)
{
	return support_hall_pen;
}

int get_support_hall_keyboard(void)
{
	return support_hall_keyboard;
}

uint32_t get_hall_hishow_value(void)
{
	return hall_hishow_value;
}

uint32_t get_hall_pen_value(void)
{
	return hall_pen_value;
}

uint32_t get_hall_keyboard_value(void)
{
	return hall_keyboard_value;
}

int get_sensor_tof_flag(void)
{
	return sensor_tof_flag;
}

int get_hifi_supported(void)
{
	return hifi_supported;
}

uint16_t *get_sensorlist(void)
{
	return sensorlist;
}

void add_sensor_list_info_id(uint16_t id)
{
	sensorlist[++sensorlist[0]] = id;
}

struct sensorlist_info *get_sensorlist_info_by_index(enum sensor_detect_list index)
{
	return &sensorlist_info[index];
}

struct sleeve_detect_pare *get_sleeve_detect_parameter(void)
{
	return sleeve_detect_paremeter;
}

int get_support_hall_lightstrap(void)
{
	return support_hall_lightstrap;
}

uint32_t get_hall_lightstrap_value(void)
{
	return hall_lightstrap_value;
}

/* get app attach sensor info */
const struct app_link_info *get_app_link_info(int type)
{
	size_t i, size;
	const struct app_link_info *app_info = NULL;

	if (gyro_detect_flag) {
		app_info = app_link_info_gyro;
		size = sizeof(app_link_info_gyro) / sizeof(struct app_link_info);
	} else {
		app_info = app_link_info_no_gyro;
		size = sizeof(app_link_info_no_gyro) / sizeof(struct app_link_info);
	}

	for (i = 0; i < size; i++) {
		if (type == app_info[i].hal_sensor_type &&
			app_info[i].used_sensor_cnt > 0 &&
			app_info[i].used_sensor_cnt <= SENSORHUB_TAG_NUM_MAX)
			return &app_info[i];
	}

	return NULL;
}

enum sensor_detect_list get_id_by_sensor_tag(int tag)
{
	int i;

	for (i = 0; i < SENSOR_MAX; i++) {
		if (sensor_manager[i].tag == tag)
			break;
	}
	return i;
}

void read_sensorlist_info(struct device_node *dn, int sensor)
{
	int temp = 0;
	char *chip_info = NULL;

	if (of_property_read_string(dn, "sensorlist_name",
		(const char **)&chip_info) >= 0) {
		strncpy(sensorlist_info[sensor].name, chip_info,
			MAX_CHIP_INFO_LEN - 1);
		sensorlist_info[sensor].name[MAX_CHIP_INFO_LEN - 1] = '\0';
		hwlog_debug("sensor chip info name %s\n", chip_info);
		hwlog_debug("sensor SENSOR_DETECT_LIST %d get name %s\n",
			sensor, sensorlist_info[sensor].name);
	} else {
		sensorlist_info[sensor].name[0] = '\0';
	}
	if (of_property_read_string(dn, "vendor",
		(const char **)&chip_info) == 0) {
		strncpy(sensorlist_info[sensor].vendor, chip_info,
			MAX_CHIP_INFO_LEN - 1);
		sensorlist_info[sensor].vendor[MAX_CHIP_INFO_LEN - 1] = '\0';
		hwlog_debug("sensor SENSOR_DETECT_LIST %d get vendor %s\n",
			sensor, sensorlist_info[sensor].vendor);
	} else {
		sensorlist_info[sensor].vendor[0] = '\0';
	}
	if (of_property_read_u32(dn, "version", &temp) == 0) {
		sensorlist_info[sensor].version = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get version %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].version = -1;
	}
	if (of_property_read_u32(dn, "maxRange", &temp) == 0) {
		sensorlist_info[sensor].max_range = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get maxRange %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].max_range = -1;
	}
	if (of_property_read_u32(dn, "resolution", &temp) == 0) {
		sensorlist_info[sensor].resolution = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get resolution %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].resolution = -1;
	}
	if (of_property_read_u32(dn, "power", &temp) == 0) {
		sensorlist_info[sensor].power = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get power %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].power = -1;
	}
	if (of_property_read_u32(dn, "minDelay", &temp) == 0) {
		sensorlist_info[sensor].min_delay = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get minDelay %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].min_delay = -1;
	}
	if (of_property_read_u32(dn, "fifoReservedEventCount", &temp) == 0) {
		sensorlist_info[sensor].fifo_reserved_event_count = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get fifoReservedEventCount %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].fifo_reserved_event_count = -1;
	}
	if (of_property_read_u32(dn, "fifoMaxEventCount", &temp) == 0) {
		sensorlist_info[sensor].fifo_max_event_count = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get fifoMaxEventCount %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].fifo_max_event_count = -1;
	}
	if (of_property_read_u32(dn, "maxDelay", &temp) == 0) {
		sensorlist_info[sensor].max_delay = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get maxDelay %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].max_delay = -1;
	}
	if (of_property_read_u32(dn, "flags", &temp) == 0) {
		sensorlist_info[sensor].flags = temp;
		hwlog_info("sensor SENSOR_DETECT_LIST %d get flags %d\n",
			sensor, temp);
	} else {
		sensorlist_info[sensor].flags = -1;
	}
}

void read_chip_info(struct device_node *dn, enum sensor_detect_list sname)
{
	char *chip_info = NULL;
	int ret;

	ret = of_property_read_string(dn, "compatible",
		(const char **)&chip_info);
	if (ret) {
		hwlog_err("%s:read name_id:%d info fail\n", __func__, sname);
	} else {
		if (strncpy_s(get_sensor_chip_info_address(sname),
			MAX_CHIP_INFO_LEN, chip_info, MAX_CHIP_INFO_LEN - 1) !=
			EOK) {
			hwlog_err("%s strncpy_s fail\n", __func__);
			return;
		}
	}
	hwlog_info("get chip info from dts success\n");
}

void read_aux_file_list(uint16_t fileid, uint16_t tag)
{
	aux_req->file_list[aux_req->file_count * AUX_FILE_LIST_ARGS] = fileid;
	aux_req->file_list[aux_req->file_count * AUX_FILE_LIST_ARGS + 1] = tag;
	aux_req->file_count++;
}

void read_dyn_file_list(uint16_t fileid)
{
	dyn_req->file_list[dyn_req->file_count] = fileid;
	dyn_req->file_count++;
}

static void read_tof_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	int register_add = 0;
	int ret;
	int i2c_address = 0;
	int i2c_bus_num = 0;
	uint8_t detected_device_id = 0;

	read_chip_info(dn, TOF);
	if (!strncmp(get_sensor_chip_info_address(TOF), "huawei,ams_tmf8701",
		sizeof("huawei,ams_tmf8701"))) {
		sensor_tof_flag = 1;
		if (strncpy_s(get_sensor_chip_info_address(PS),
			MAX_CHIP_INFO_LEN, "huawei,ams_tmf8701",
			sizeof("huawei,ams_tmf8701")) != EOK)
			hwlog_err("%s strncpy_s fail\n", __func__);
		hwlog_info("%s:ams_tmf8701 i2c_address suc,%d\n", __func__, temp);
	} else if (!strncmp(get_sensor_chip_info_address(TOF),
		"huawei,sharp_gp2ap02", sizeof("huawei,sharp_gp2ap02"))) {
		sensor_tof_flag = 1;
		if (strncpy_s(get_sensor_chip_info_address(PS),
			MAX_CHIP_INFO_LEN, "huawei,sharp_gp2ap02",
			sizeof("huawei,sharp_gp2ap02")) != EOK)
			hwlog_err("%s strncpy_s fail\n", __func__);
		hwlog_info("%s:sharp_gp2ap02 i2c_address suc,%d\n", __func__, temp);
	}

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
		of_property_read_u32(dn, "reg", &i2c_address) ||
		of_property_read_u32(dn, "chip_id_register", &register_add))
		hwlog_err("%s:read i2c bus_number or bus address or chip_id_register from dts fail\n",
			__func__);

	if (of_property_read_u32(dn, "tof_calib_zero_threshold", &temp)) {
		hwlog_info("%s:read tof_calib_zero_threshold fail\n", __func__);
	} else {
		tof_data.tof_calib_zero_threshold = temp;
		hwlog_info("%s:read tof_calib_zero_threshold=%d\n",
			__func__, temp);
	}

	if (of_property_read_u32(dn, "tof_calib_6cm_threshold", &temp)) {
		hwlog_info("%s:read tof_calib_6cm_threshold fail\n", __func__);
	} else {
		tof_data.tof_calib_6cm_threshold = temp;
		hwlog_info("%s:read tof_calib_6cm_threshold=%d\n",
			__func__, temp);
	}

	if (of_property_read_u32(dn, "tof_calib_10cm_threshold", &temp)) {
		hwlog_info("%s:read tof_calib_10cm_threshold fail\n", __func__);
	} else {
		tof_data.tof_calib_10cm_threshold = temp;
		hwlog_info("%s:read tof_calib_10cm_threshold=%d\n",
			__func__, temp);
	}

	if (of_property_read_u32(dn, "tof_calib_60cm_threshold", &temp)) {
		hwlog_info("%s:read tof_calib_60cm_threshold fail\n", __func__);
	} else {
		tof_data.tof_calib_60cm_threshold = temp;
		hwlog_info("%s:read tof_calib_60cm_threshold=%d\n",
			__func__, temp);
	}

	ret = mcu_i2c_rw(TAG_I2C, (uint8_t)i2c_bus_num, (uint8_t)i2c_address,
		(uint8_t *)&register_add, 1, &detected_device_id, 1);
	if (ret)
		hwlog_err("%s:detect_i2c_device:send i2c read cmd to mcu fail,ret=%d\n",
			__func__, ret);

	if (of_property_read_u32(dn, "file_id", &temp)) {
		hwlog_err("%s:read tof file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read tof sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);

	read_sensorlist_info(dn, TOF);
}

static void read_connectivity_bus_type(struct device_node *dn, uint8_t *bus_type)
{
	const char *bus_string = NULL;
	int temp = (int)*bus_type;
	if (of_property_read_string(dn, "bus_type", &bus_string)) {
		hwlog_err("%s:connectivity bus_type not configured\n", __func__);
		return;
	}
	if (get_combo_bus_tag(bus_string, (uint8_t *)&temp)) {
		hwlog_warn("connectivity %s bus_type string invalid, next detect digit\n", bus_string);
		if (of_property_read_u32(dn, "bus_type", &temp)) {
			hwlog_err("%s:read bus_type digit fail\n", __func__);
			return;
		}

		if (temp >= TAG_END) {
			hwlog_err("%s:read bus_type %d invalid\n", __func__, temp);
			return;
		}
	}
	*bus_type = (uint8_t)temp;
}

static void read_connectivity_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, CONNECTIVITY);

	if (of_property_read_u32(dn, "gpio1_gps_cmd_ap", &temp))
		hwlog_err("%s:read gpio1_gps_cmd_ap fail\n", __func__);
	else
		connectivity_data.gpio1_gps_cmd_ap = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio1_gps_cmd_sh", &temp))
		hwlog_err("%s:read gpio1_gps_cmd_sh fail\n", __func__);
	else
		connectivity_data.gpio1_gps_cmd_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio1_gps_cmd_pinmux", &temp))
		hwlog_warn("%s:read gpio1_gps_cmd_pinmux fail\n", __func__);
	else
		connectivity_data.gpio1_gps_cmd_pinmux = (uint16_t)temp;

	if (of_property_read_u32(dn, "gpio2_gps_ready_ap", &temp))
		hwlog_err("%s:read gpio2_gps_ready_ap fail\n", __func__);
	else
		connectivity_data.gpio2_gps_ready_ap = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio2_gps_ready_sh", &temp))
		hwlog_err("%s:read gpio2_gps_ready_sh fail\n", __func__);
	else
		connectivity_data.gpio2_gps_ready_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio2_gps_ready_pinmux", &temp))
		hwlog_warn("%s:read gpio2_gps_ready_pinmux fail\n", __func__);
	else
		connectivity_data.gpio2_gps_ready_pinmux = (uint16_t)temp;

	if (of_property_read_u32(dn, "gpio3_wakeup_gps_ap", &temp))
		hwlog_err("%s:read gpio3_wakeup_gps_ap fail\n", __func__);
	else
		connectivity_data.gpio3_wakeup_gps_ap = (GPIO_NUM_TYPE) temp;

	if (of_property_read_u32(dn, "gpio3_wakeup_gps_sh", &temp))
		hwlog_err("%s:read gpio3_wakeup_gps_sh fail\n", __func__);
	else
		connectivity_data.gpio3_wakeup_gps_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio3_wakeup_gps_pinmux", &temp))
		hwlog_warn("%s:read gpio3_wakeup_gps_pinmux fail\n", __func__);
	else
		connectivity_data.gpio3_wakeup_gps_pinmux = (uint16_t)temp;

	read_connectivity_bus_type(dn, (uint8_t *)&connectivity_data.cfg.bus_type);

	if (of_property_read_u32(dn, "bus_number", &temp))
		hwlog_err("%s:read bus_number fail\n", __func__);
	else
		connectivity_data.cfg.bus_num = (uint8_t)temp;

	if (of_property_read_u32(dn, "i2c_address", &temp))
		hwlog_err("%s:read i2c_address fail\n", __func__);
	else
		connectivity_data.cfg.i2c_address = (uint32_t)temp;

	if (of_property_read_u32(dn, "i3c_frequency", &temp))
		hwlog_err("%s:read i3c_frequency fail\n", __func__);
	else
		connectivity_data.i3c_frequency = (uint32_t) temp;

	if (of_property_read_u32(dn, "file_id", &temp)) {
		hwlog_err("%s:read connectivity file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;
		dyn_req->file_count++;
	}

	hwlog_err("connectivity file id is %d\n", temp);
	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read connectivity sensor_list_info_id fail\n",
			__func__);
	else
		add_sensor_list_info_id((uint16_t)temp);
}

static void read_fingerprint_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, FINGERPRINT);

	if (of_property_read_u32(dn, "file_id", &temp)) {
		hwlog_err("%s:read fingerprint file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;
		dyn_req->file_count++;
	}
	hwlog_err("fingerprint  file id is %d\n", temp);

	if (of_property_read_u32(dn, "chip_id_register", &temp))
		hwlog_err("%s:read chip_id_register fail\n", __func__);
	else
		fingerprint_data.reg = (uint16_t)temp;

	if (of_property_read_u32(dn, "chip_id_value", &temp))
		hwlog_err("%s:read chip_id_value fail\n", __func__);
	else
		fingerprint_data.chip_id = (uint16_t)temp;

	if (of_property_read_u32(dn, "product_id_value", &temp))
		hwlog_err("%s:read product_id_value fail\n", __func__);
	else
		fingerprint_data.product_id = (uint16_t)temp;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
		hwlog_err("%s:read gpio_irq fail\n", __func__);
	else
		fingerprint_data.gpio_irq = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
		hwlog_err("%s:read gpio_irq_sh fail\n", __func__);
	else
		fingerprint_data.gpio_irq_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_reset", &temp))
		hwlog_err("%s:read gpio_reset fail\n", __func__);
	else
		fingerprint_data.gpio_reset = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_reset_sh", &temp))
		hwlog_err("%s:read gpio_reset_sh fail\n", __func__);
	else
		fingerprint_data.gpio_reset_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_cs", &temp))
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	else
		fingerprint_data.gpio_cs = (GPIO_NUM_TYPE)temp;
}

static void read_fingerprint_ud_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, FINGERPRINT_UD);

	if (of_property_read_u32(dn, "file_id", &temp)) {
		hwlog_err("%s:read fingerprint file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) temp;
		dyn_req->file_count++;
	}

	hwlog_err("fingerprint file id is %d\n", temp);

	if (of_property_read_u32(dn, "chip_id_register", &temp))
		hwlog_err("%s:read chip_id_register fail\n", __func__);
	else
		fingerprint_ud_data.reg = (uint16_t)temp;

	if (of_property_read_u32(dn, "chip_id_value", &temp))
		hwlog_err("%s:read chip_id_value fail\n", __func__);
	else
		fingerprint_ud_data.chip_id = (uint16_t)temp;

	if (of_property_read_u32(dn, "product_id_value", &temp))
		hwlog_err("%s:read product_id_value fail\n", __func__);
	else
		fingerprint_ud_data.product_id = (uint16_t)temp;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
		hwlog_err("%s:read gpio_irq fail\n", __func__);
	else
		fingerprint_ud_data.gpio_irq = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
		hwlog_err("%s:read gpio_irq_sh fail\n", __func__);
	else
		fingerprint_ud_data.gpio_irq_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_reset", &temp))
		hwlog_err("%s:read gpio_reset fail\n", __func__);
	else
		fingerprint_ud_data.gpio_reset = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_cs", &temp))
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	else
		fingerprint_ud_data.gpio_cs = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "tp_hover_support", &temp))
		hwlog_warn("%s:read tp_hover_support fail\n", __func__);
	else
		fingerprint_ud_data.tp_hover_support = (uint16_t)temp;
}

static void read_key_i2c_data_from_dts(struct device_node *dn)
{
	read_chip_info(dn, KEY);
	key_data.cfg.i2c_address = 0x27;
	dyn_req->file_list[dyn_req->file_count] = 59; /* key file_id 59 */
	dyn_req->file_count++;

	hwlog_info("key read dts\n");
}

static void read_sound_data_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = NULL;
	int temp = 0;

	hwlog_info("sound read dts\n");
	read_chip_info(dn, SOUND);
	if (of_property_read_u32(dn, "file_id", &temp)) {
		hwlog_err("%s:read sound file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;
		dyn_req->file_count++;
	}

	pf_data = ps_get_platform_data(TAG_PS);
	if (pf_data == NULL)
		return;

	if (of_property_read_u32(dn, "gpio_irq_soc", &temp))
		hwlog_warn("%s:read sound gpio_irq_soc fail\n", __func__);
	else
		pf_data->sound_gpio_irq_soc = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
		hwlog_warn("%s:read sound gpio_irq_sh fail\n", __func__);
	else
		pf_data->sound_gpio_irq_sh = (GPIO_NUM_TYPE)temp;

	hwlog_info("sound_gpio_irq_soc = %d, sound_gpio_irq_sh = %d\n",
		pf_data->sound_gpio_irq_soc, pf_data->sound_gpio_irq_sh);
}

static void read_aod_data_from_dts(struct device_node *dn)
{
	int i;
	int len = of_property_count_u32_elems(dn, "feature");

	read_chip_info(dn, SH_AOD);

	if (len <= 0) {
		hwlog_warn("%s:count u32 data fail\n", __func__);
		return;
	}

	if (of_property_read_u32_array(dn, "feature", aod_data.feature_set, len))
		hwlog_err("%s:read chip_id_value fail\n", __func__);

	for (i = 0; i < len; i++)
		hwlog_info("aod_data.feature_set[%d]= 0x%x\n",
			i, aod_data.feature_set[i]);
}

static void read_modem_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, IOMCU_MODEM);
	if (of_property_read_u32(dn, "mode", &temp))
		hwlog_warn("%s:read modem mode fail\n", __func__);
	else
		g_modem_data.mode = (uint32_t)temp;

	hwlog_info("g_modem_data.mode = 0x%x\n", g_modem_data.mode);
}

static void read_magn_bracket_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	u32 wia[6] = { 0 };
	struct property *prop = NULL;
	unsigned int len;

	read_chip_info(dn, MAGN_BRACKET);

	prop = of_find_property(dn, "mag_axis_change", NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL!\n", __func__);
		return;
	}
	if (!prop->value) {
		hwlog_err("%s! prop->value is NULL!\n", __func__);
		return;
	}
	len = prop->length / 4; /* 4: to word */
	if (of_property_read_u32_array(dn, "mag_axis_change", wia, len)) {
		hwlog_err("%s:read mag_axis_change from dts fail!\n",  __func__);
		return;
	}

	magn_bracket_data.mag_x_change_lower = (int)wia[0];
	magn_bracket_data.mag_x_change_upper = (int)wia[1];
	magn_bracket_data.mag_y_change_lower = (int)wia[2];
	magn_bracket_data.mag_y_change_upper = (int)wia[3];
	magn_bracket_data.mag_z_change_lower = (int)wia[4];
	magn_bracket_data.mag_z_change_upper = (int)wia[5];

	hwlog_info("read_magn_bracket_data_from_dts %d, %d, %d, %d, %d, %d\n",
		magn_bracket_data.mag_x_change_lower,
		magn_bracket_data.mag_x_change_upper,
		magn_bracket_data.mag_y_change_lower,
		magn_bracket_data.mag_y_change_upper,
		magn_bracket_data.mag_z_change_lower,
		magn_bracket_data.mag_z_change_upper);

	if (of_property_read_u32(dn, "file_id", &temp)) {
		hwlog_err("%s:read magn_bracket file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read magn_bracket sensor_list_info_id fail\n",
			__func__);
	else
		add_sensor_list_info_id((uint16_t)temp);

	read_sensorlist_info(dn, MAGN_BRACKET);
}

static void read_rpc_config_from_dts(struct device_node *dn)
{
	int t = 0;
	int *temp = &t;

	if (of_property_read_u32(dn, "file_id", (u32 *)temp)) {
		hwlog_err("%s:read rpc file_id fail\n", __func__);
	} else {
		dyn_req->file_list[dyn_req->file_count] = (uint16_t) t;
		dyn_req->file_count++;
	}

	if (of_property_read_u32(dn, "sensor_list_info_id", (u32 *)temp))
		hwlog_err("%s:read rpc sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)t);

	if (of_property_read_u32(dn, "default_value", (u32 *)temp))
		hwlog_err("%s:read default_value fail\n", __func__);
	else
		rpc_data.default_value = (uint16_t)t;

	if (of_property_read_u32(dn, "mask_enable", (u32 *)temp))
		hwlog_err("%s:read mask_enable fail\n", __func__);
	else
		rpc_data.mask_enable = (uint16_t)t;

	if (of_property_read_u32(dn, "sar_choice", (u32 *)temp))
		hwlog_err("%s:read mask_enable fail\n", __func__);
	else
		rpc_data.sar_choice = (uint16_t)t;

	if (of_property_read_u32(dn, "sim_type_swtich_flag", (u32 *)temp))
		hwlog_err("%s:read sim type swtich flag fail\n", __func__);
	else
		rpc_data.sim_type_swtich_flag = (uint16_t)t;

	if (of_property_read_u32(dn, "fusion_type", (u32 *)temp))
		hwlog_err("%s:read fusion type fail\n", __func__);
	else
		rpc_data.fusion_type = (uint16_t)t;
}

static int read_rpc_data_from_dts(struct device_node *dn)
{
	unsigned int i;
	u32 wia[32] = { 0 };
	struct property *prop = NULL;
	unsigned int len;

	memset(&rpc_data, 0, sizeof(rpc_data));

	read_chip_info(dn, RPC);

	prop = of_find_property(dn, "table", NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL\n", __func__);
		return -1;
	}

	len = (u32)(prop->length) / sizeof(u32);
	if (of_property_read_u32_array(dn, "table", wia, len)) {
		hwlog_err("%s:read rpc_table from dts fail\n", __func__);
		return -1;
	}
	for (i = 0; i < len; i++)
		rpc_data.table[i] = (u16)wia[i];

	memset(wia, 0, sizeof(wia));
	prop = of_find_property(dn, "mask", NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL\n", __func__);
		return -1;
	}

	len = (u32)(prop->length) / sizeof(u32);
	if (of_property_read_u32_array(dn, "mask", wia, len)) {
		hwlog_err("%s:read rpc_mask from dts fail\n", __func__);
		return -1;
	}
	for (i = 0; i < len; i++)
		rpc_data.mask[i] = (u16)wia[i];

	read_rpc_config_from_dts(dn);
	read_sensorlist_info(dn, RPC);
	return 0;
}

static void read_kb_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	if (!dn)
		return;

	read_chip_info(dn, KB);

	if (of_property_read_u32(dn, "uart_num", &temp))
		hwlog_err("%s:read uart_num fail\n", __func__);
	else
		g_kb_data.uart_num = (uint16_t) temp;

	if (of_property_read_u32(dn, "kb_detect_adc_num", &temp))
		hwlog_err("%s:read kb_detect_adc_num fail\n", __func__);
	else
		g_kb_data.kb_detect_adc_num = (uint16_t)temp;

	if (of_property_read_u32(dn, "kb_disconnect_adc_min", &temp))
		hwlog_err("%s:read kb_disconnect_adc_min fail\n", __func__);
	else
		g_kb_data.kb_disconnect_adc_min = (uint16_t)temp;

	if (of_property_read_u32(dn, "kb_disable_angle", &temp))
		hwlog_err("%s:read kb_disable_angle fail\n", __func__);
	else
		g_kb_data.kb_disable_angle = (uint16_t)temp;

	hwlog_info("%s: uart_num:%d, adc_num:%d, disconnect_adc_min:%d\n",
		__func__, g_kb_data.uart_num, g_kb_data.kb_detect_adc_num,
		g_kb_data.kb_disconnect_adc_min);
}

static int get_adapt_file_id_for_dyn_load(void)
{
	u32 wia[ADAPT_FILE_ID_NUM] = { 0 };
	struct property *prop = NULL;
	unsigned int i;
	unsigned int len;
	struct device_node *sensorhub_node = NULL;
	const char *name = "adapt_file_id";
	char *temp = NULL;

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}
	prop = of_find_property(sensorhub_node, name, NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL\n", __func__);
		return -EINVAL;
	}
	len = prop->length / sizeof(wia[0]);
	if ((!prop->value) || (len > ARRAY_SIZE(wia))) {
		hwlog_err("%s prop->value is NULL or len=%u too big\n",
			__func__, len);
		return -ENODATA;
	}
	if (of_property_read_u32_array(sensorhub_node, name, wia, len)) {
		hwlog_err("%s:read adapt_file_id from dts fail\n", name);
		return -1;
	}
	for (i = 0; i < len; i++)
		read_dyn_file_list((uint16_t)wia[i]);
	/* find hifi supported or not */
	if (of_property_read_u32(sensorhub_node, "hifi_support", &i) == 0) {
		if (i == 1) {
			hifi_supported = 1;
			hwlog_info("sensor get hifi support %d\n", i);
		}
	}

	if (of_property_read_string(sensorhub_node,
		"docom_step_counter", (const char **)&temp) == 0) {
		if (!strcmp("enabled", temp)) {
			g_config_on_ddr->reserved |= 1 << 0;
			hwlog_info("%s:docom_step_counter status is %s\n", __func__, temp);
		}
	}

	if (of_property_read_string(sensorhub_node,
		"homo_activity", (const char **)&temp) == 0) {
		if (!strcmp("enabled", temp)) {
			g_config_on_ddr->reserved |= 1 << 1;
			hwlog_info("%s:homo_activity status is %s\n", __func__, temp);
		}
	}

	return 0;
}

static int get_hall_config_from_dts(void)
{
	unsigned int i = 0;
	struct device_node *sensorhub_node = NULL;
	t_ap_sensor_ops_record *record = get_all_ap_sensor_operations();

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (sensorhub_node == NULL) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1; /* get huawei sensorhub node fail */
	}

	/* find number of the hall sensor */
	if (!of_property_read_u32(sensorhub_node, "hall_number", &i)) {
		hall_number = i;
		hwlog_info("sensor get hall number %d\n", hall_number);
	}

	if (!of_property_read_u32(sensorhub_node, "hall_sen_type", &i)) {
		hall_sen_type = i;
		hwlog_info("sensor get hall sensor type %d\n", i);
		if (hall_sen_type == HUB_FOLD_HALL_TYPE)
			record[TAG_EXT_HALL].work_on_ap = false;
	} else {
		hall_sen_type = 0; /* set hall sen type to default type */
	}

	if (!of_property_read_u32(sensorhub_node, "is_support_hall_hishow", &i)) {
		support_hall_hishow = i;
		hwlog_info("sensor get support_hall_hishow: %d\n", support_hall_hishow);
	}

	if (!of_property_read_u32(sensorhub_node, "hall_hishow_value", &i)) {
		hall_hishow_value = (uint32_t)i;
		hwlog_info("sensor get hall_hishow_value: %#x\n", hall_hishow_value);
	}

	if (!of_property_read_u32(sensorhub_node, "is_support_hall_pen", &i)) {
		support_hall_pen = i;
		hwlog_info("sensor get support_hall_pen: %d\n", support_hall_pen);
	}

	if (!of_property_read_u32(sensorhub_node, "hall_pen_value", &i)) {
		hall_pen_value = (uint32_t)i;
		hwlog_info("sensor get hall_pen_value: %#x\n", hall_pen_value);
	}

	if (!of_property_read_u32(sensorhub_node, "is_support_hall_keyboard", &i)) {
		support_hall_keyboard = i;
		hwlog_info("sensor get support_hall_keyboard: %d\n", support_hall_keyboard);
	}

	if (!of_property_read_u32(sensorhub_node, "hall_keyboard_value", &i)) {
		hall_keyboard_value = (uint32_t)i;
		hwlog_info("sensor get hall_keyboard_value: %#x\n", hall_keyboard_value);
	}

	if (!of_property_read_u32(sensorhub_node, "is_support_hall_lightstrap", &i)) {
		support_hall_lightstrap = i;
		hwlog_info("sensor get is_support_hall_lightstrap: %d\n", support_hall_lightstrap);
	}

	if (!of_property_read_u32(sensorhub_node, "hall_lightstrap_value", &i)) {
		hall_lightstrap_value = (uint32_t)i;
		hwlog_info("sensor get hall_lightstrap_value: %u\n", hall_lightstrap_value);
	}

	return 0;
}

static void swap1(uint16_t *left, uint16_t *right)
{
	uint16_t temp;

	temp = *left;
	*left = *right;
	*right = temp;
}

/* delete the repeated file id by map table */
static uint8_t check_file_list(uint8_t file_count, uint16_t *file_list)
{
	int i, j, k;

	if ((file_count == 0) || (!file_list)) {
		hwlog_err("%s, val invalid\n", __func__);
		return 0;
	}

	for (i = 0; i < file_count; i++) {
		for (j = i + 1; j < file_count; j++) {
			if (file_list[i] != file_list[j])
				continue;
			file_count -= 1;
			for (k = j; k < file_count; k++)
				file_list[k] = file_list[k+1];
			j -= 1;
		}
	}

	for (i = 0; i < file_count; i++) {
		for (j = 1; j < file_count - i; j++) {
			if (file_list[j - 1] > file_list[j])
				swap1(&file_list[j - 1], &file_list[j]);
		}
	}
	return file_count;
}

static int get_adapt_sensor_list_id(void)
{
	u32 wia[ADAPT_SENSOR_LIST_NUM] = { 0 };
	struct property *prop = NULL;
	unsigned int i;
	unsigned int len;
	struct device_node *sensorhub_node = NULL;
	const char *name = "adapt_sensor_list_id";

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}
	prop = of_find_property(sensorhub_node, name, NULL);
	if (!prop) {
		hwlog_err("%s! prop is NULL\n", __func__);
		return -EINVAL;
	}
	len = prop->length / sizeof(wia[0]);
	if ((!prop->value) || (len > ARRAY_SIZE(wia))) {
		hwlog_err("%s prop->value is NULL or len=%u too big\n",
			__func__, len);
		return -ENODATA;
	}
	if (of_property_read_u32_array(sensorhub_node, name, wia, len)) {
		hwlog_err("%s:read adapt_sensor_list_id from dts fail\n", name);
		return -1;
	}
	for (i = 0; i < len; i++)
		add_sensor_list_info_id((uint16_t)wia[i]);

	return 0;
}

int is_power_off_charging_posture(void)
{
	return power_off_charging_posture;
}

static void get_power_charging_status_from_dts(void)
{
	struct device_node *sensorhub_node = NULL;
	uint32_t charge_check = 0;
	bool power_mode = false;

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return;
	}
	if (!of_property_read_u32(sensorhub_node, "power_charging_posture",
		&charge_check))
		hwlog_info("get dts charge_check %d\n", charge_check);

	power_mode = power_cmdline_is_powerdown_charging_mode();
	hwlog_info("get charging_mode %d, charge_check = %u\n",
		power_mode, charge_check);
	if ((power_mode == true) && (charge_check == 1))
		power_off_charging_posture = 1; // suport power off get fold
}

static int get_sensors_id_from_dts(void)
{
	struct device_node *sensorhub_node = NULL;

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}

	acc_get_sensors_id_from_dts(sensorhub_node);
	als_get_sensors_id_from_dts(sensorhub_node);
	cap_prox_get_sensors_id_from_dts(sensorhub_node);
	gyro_get_sensors_id_from_dts(sensorhub_node);
	mag_get_sensors_id_from_dts(sensorhub_node);
	therm_get_sensors_id_from_dts(sensorhub_node);

	if(get_hall_config_from_dts())
		hwlog_err("get hall config from dts fail\n");

	return 0;
}

static int send_dyn_to_mcu(void *buf, int len)
{
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	int ret;

	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_DYNLOAD_REQ;
	pkg_ap.wr_buf = buf;
	pkg_ap.wr_len = len;

	if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
		(get_iomcu_power_state() == ST_SLEEP))
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	else
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);

	if (ret) {
		hwlog_err("send file id to mcu fail,ret=%d\n", ret);
		return -1;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("file id set fail\n");
		return -1;
	}

	return 0;
}

int send_fileid_to_mcu(void)
{
	int i;
	pkt_sys_dynload_req_t dynload_req;

	if (dyn_req->file_count) {
		hwlog_info("sensorhub after check, get dynload file id number = %d, fild id",
			dyn_req->file_count);
		for (i = 0; i < dyn_req->file_count; i++)
			hwlog_info("--%d", dyn_req->file_list[i]);
		hwlog_info("\n");
		dyn_req->file_flg = 0;
		if (send_dyn_to_mcu(&(dyn_req->file_flg),
		    dyn_req->file_count * sizeof(dyn_req->file_list[0]) +
		    sizeof(dyn_req->file_flg) + sizeof(dyn_req->file_count)))
			hwlog_err("%s send file_id to mcu failed\n", __func__);
	} else {
		hwlog_err("%s file_count = 0, not send file_id to mcu\n",
			__func__);
		return -EINVAL;
	}

	if (aux_req->file_count) {
		hwlog_info("sensorhub after check, get aux file id number = %d, aux file id and tag ",
			aux_req->file_count);
		for (i = 0; i < aux_req->file_count; i++)
			hwlog_info("--%d, %d",
				aux_req->file_list[AUX_FILE_LIST_ARGS * i],
				aux_req->file_list[AUX_FILE_LIST_ARGS * i + 1]);

		hwlog_info("\n");
		aux_req->file_flg = 1;
		if (send_dyn_to_mcu(&(aux_req->file_flg),
		    aux_req->file_count * sizeof(aux_req->file_list[0]) *
		    AUX_FILE_LIST_ARGS + sizeof(aux_req->file_flg) +
		    sizeof(aux_req->file_count)))
			hwlog_err("%s send aux file_id to mcu failed\n", __func__);
	} else {
		hwlog_err("%s aux count=0,not send file_id to mcu\n", __func__);
	}
	memset(&dynload_req, 0, sizeof(pkt_sys_dynload_req_t));
	/* 1:aux sensorlist 0:filelist 2: loading */
	dynload_req.file_flg = 2;

	return send_dyn_to_mcu(&(dynload_req.file_flg),
			sizeof(pkt_sys_dynload_req_t) - sizeof(struct pkt_header));
}

static int get_adapt_id_and_send(void)
{
	int ret, i;

	ret = get_adapt_file_id_for_dyn_load();
	if (ret < 0)
		hwlog_err("get_adapt_file_id_for_dyn_load() failed!\n");

	hwlog_info("get file id number = %d\n", dyn_req->file_count);

	ret = get_adapt_sensor_list_id();
	if (ret < 0)
		hwlog_err("get_adapt_sensor_list_id() failed\n");

	sensorlist[0] = check_file_list(sensorlist[0], &sensorlist[1]);
	if (sensorlist[0] > 0) {
		hwlog_info("sensorhub after check, get sensor list id number = %d, list id: ",
			sensorlist[0]);
		for (i = 0; i < sensorlist[0]; i++)
			hwlog_info("--%d", sensorlist[i + 1]);
		hwlog_info("\n");
	} else {
		hwlog_err("%s list num = 0, not send file_id to mcu\n", __func__);
		return -EINVAL;
	}
	dyn_req->file_count =
		check_file_list(dyn_req->file_count, dyn_req->file_list);

	return send_fileid_to_mcu();
}

static int get_key_chip_type(struct device_node *dn)
{
	int ret;
	int ctype = 0;

	ret = of_property_read_u32(dn, "chip_type", &ctype);
	if (ret < 0)
		hwlog_err("read chip type err. ret:%d\n", ret);

	return ctype;
}

static int key_sensor_detect(struct device_node *dn)
{
	int ret;
	int reg = 0;
	int bootloader_reg = 0;
	uint8_t values[12] = {0};
	int chip_id_register;

	ret = of_property_read_u32(dn, "reg", &reg);
	if (ret < 0) {
		hwlog_err("read reg err. ret:%d\n", ret);
		return -1;
	}
	ret = of_property_read_u32(dn, "reg_bootloader", &bootloader_reg);
	if (ret < 0) {
		hwlog_err("read reg_bootloader err. ret:%d\n", ret);
		return -1;
	}

	hwlog_info("[%s] debug key reg:%d, btld reg:%d\n",
		__func__, reg, bootloader_reg);
	msleep(50); /* sleep 50 ms */
	ret = mcu_i2c_rw(TAG_I2C, 0, (uint8_t)bootloader_reg, NULL, 0, values,
		(uint32_t)(sizeof(values)/sizeof(values[0])));
	if (ret < 0) {
		hwlog_info("[%s][28] %d %d %d %d %d %d %d %d\n", __func__,
			values[0], values[1], values[2], values[3],
			values[4], values[5], values[6], values[7]);
		msleep(10); /* sleep 10 ms */
		chip_id_register = 0x46;
		ret = mcu_i2c_rw(TAG_I2C, 0, (uint8_t)reg, (uint8_t *)&chip_id_register,
			1, values, 2); /* 2 bytes */
		if (ret < 0) {
			hwlog_err("i2c 27 read err\n");
			return -1;
		}
	}

	hwlog_info("[%s][28] %d %d %d %d %d %d %d %d\n", __func__,
			values[0], values[1], values[2], values[3],
			values[4], values[5], values[6], values[7]);
	return 0;
}

static int inputhub_key_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	int chip_type;
	struct sensor_detect_manager *p = sm + index;
	struct sensor_combo_cfg cfg = { 0 };

	chip_type = get_key_chip_type(dn);
	if (chip_type)
		return key_sensor_detect(dn);

	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}
	return ret;
}

#define FINGERPRINT_SENSOR_DETECT_SUCCESS 10
#define FINGERPRINT_WRITE_CMD_LEN 7
#define FINGERPRINT_READ_CONTENT_LEN 2

static int fingerprint_get_gpio_config(struct device_node *dn,
	struct fp_gpio_config *cfg)
{
	int temp = 0;
	int ret = RET_SUCC;

	if (of_property_read_u32(dn, "gpio_reset", &temp)) {
		hwlog_err("%s:read gpio_reset fail\n", __func__);
		ret = RET_FAIL;
	} else {
		cfg->gpio_reset = (GPIO_NUM_TYPE)temp;
	}

	if (of_property_read_u32(dn, "gpio_cs", &temp)) {
		hwlog_err("%s:read gpio_cs fail\n", __func__);
		ret = RET_FAIL;
	} else {
		cfg->gpio_cs = (GPIO_NUM_TYPE)temp;
	}

	if (of_property_read_u32(dn, "gpio_irq", &temp)) {
		hwlog_err("%s:read gpio_irq fail\n", __func__);
		ret = RET_FAIL;
	} else {
		cfg->gpio_irq = (GPIO_NUM_TYPE)temp;
	}

	return ret;
}

static void fingerprint_gpio_reset(GPIO_NUM_TYPE gpio_reset,
	unsigned int first_sleep, unsigned int second_sleep)
{
	gpio_direction_output(gpio_reset, GPIO_STAT_HIGH);
	msleep(first_sleep);
	gpio_direction_output(gpio_reset, GPIO_STAT_LOW);
	msleep(second_sleep);
	gpio_direction_output(gpio_reset, GPIO_STAT_HIGH);
}

static void set_f01_config(struct fp_gpio_config *gpio_cfg)
{
	gpio_direction_output(gpio_cfg->gpio_reset, GPIO_STAT_HIGH);
	msleep(RESET_LONG_SLEEP);
}

static void set_f02_config(struct fp_gpio_config *gpio_cfg)
{
	union spi_ctrl ctrl;
	uint8_t tx[2] = { 0 }; /* 2 : tx register length */
	uint32_t tx_len;

	fingerprint_gpio_reset(gpio_cfg->gpio_reset,
		RESET_LONG_SLEEP, RESET_LONG_SLEEP);

	/* send 2 byte 0xF0 cmd to software reset sensor */
	ctrl.data = gpio_cfg->gpio_cs;
	tx[0] = 0xF0;
	tx[1] = 0xF0;
	tx_len = 2; /* 2 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
	msleep(100); /* 100 : sleep time after soft reset */
}

static void set_f03_config(struct fp_gpio_config *gpio_cfg)
{
	uint8_t tx[FINGERPRINT_WRITE_CMD_LEN] = { 0 };
	uint8_t rx[FINGERPRINT_READ_CONTENT_LEN] = { 0 };
	uint32_t tx_len;
	uint32_t rx_len;
	union spi_ctrl ctrl;

	fingerprint_gpio_reset(gpio_cfg->gpio_reset,
		RESET_LONG_SLEEP, RESET_LONG_SLEEP);

	/* set sensor to idle mode, cmd is 0xC0, lenth is 1 */
	ctrl.data = gpio_cfg->gpio_cs;
	tx[0] = 0xC0;
	tx_len = 1;
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);

	/* write cmd 0xF0 & address 0x0126, length is 3 */
	tx[0] = 0xF0;
	tx[1] = 0x01;
	tx[2] = 0x26;
	tx_len = 3; /* 3 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);

	/* read cmd 0xF1, cmd length is 1, rx length is 2 */
	tx[0] = 0xF1;
	tx_len = 1;
	rx_len = 2; /* 2 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, rx, rx_len);

	/* write cmd 0xF0 & address 0x0124 and 0x0001 */
	/* clear irq, tx length is 7 */
	if ((rx[0] != 0x00) || (rx[1] != 0x00)) {
		tx[0] = 0xF0;
		tx[1] = 0x01;
		tx[2] = 0x24;
		tx[3] = 0x00;
		tx[4] = 0x01;
		tx[5] = rx[0];
		tx[6] = rx[1];
		tx_len = 7; /* 7 bytes */
		mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
	}

	/* set sensor to idle mode, cmd is 0xC0, lenth is 1 */
	tx[0] = 0xC0;
	tx_len = 1;
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);

	/* write cmd 0xF0 & address 0x0000, length is 3 */
	tx[0] = 0xF0;
	tx[1] = 0x00;
	tx[2] = 0x00;
	tx_len = 3; /* 3 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
}

static void set_f04_config(struct fp_gpio_config *gpio_cfg)
{
	fingerprint_gpio_reset(gpio_cfg->gpio_reset,
		RESET_LONG_SLEEP, RESET_LONG_SLEEP);
	msleep(RESET_SHORT_SLEEP);
}

static const struct fp_sensor_config g_fp_cfgs[] = {
	{ "F01", FP_MATCH_LEN, set_f01_config },
	{ "F02", FP_MATCH_LEN, set_f02_config },
	{ "F03", FP_MATCH_LEN, set_f03_config },
	{ "F04", FP_MATCH_LEN, set_f04_config },
	{ "fpc", FPC_NAME_LEN, set_f01_config },
	{ "syna", SYNA_NAME_LEN, set_f02_config },
	{ "goodix", GOODIX_NAME_LEN, set_f03_config },
	{ "silead", SILEAD_NAME_LEN, set_f04_config },
};

static int fingerprint_sensor_detect(struct device_node *dn, int index,
	struct sensor_combo_cfg *cfg)
{
	int ret;
	int i;
	int irq_value = 0;
	int reset_value = 0;
	char *sensor_vendor = NULL;
	struct fp_gpio_config gpio_cfg = { 0, 0, 0 };

	if (fingerprint_get_gpio_config(dn, &gpio_cfg) != 0)
		hwlog_err("%s:read fingerprint gpio fail\n", __func__);

	if (sensor_manager[index].detect_result == DET_FAIL) {
		reset_value = gpio_get_value(gpio_cfg.gpio_reset);
		irq_value = gpio_get_value(gpio_cfg.gpio_irq);
	}

	ret = of_property_read_string(dn, "compatible",
		(const char **)&sensor_vendor);
	if (!ret) {
		if (!strncmp(sensor_vendor, "qfp", QFP_NAME_LEN)) {
			_device_detect(dn, index, cfg);
			hwlog_info("%s: fingerprint device %s detect bypass\n",
				__func__, sensor_vendor);
			return FINGERPRINT_SENSOR_DETECT_SUCCESS;
		}

		for (i = 0; i < ARRAY_SIZE(g_fp_cfgs); i++) {
			if (!strncmp(sensor_vendor, g_fp_cfgs[i].ic_code,
				g_fp_cfgs[i].length)) {
				(*g_fp_cfgs[i].func)(&gpio_cfg);
				hwlog_info("%s: g_fp_cfgs index: %d\n",
					__func__, i);
				break;
			}
		}
		hwlog_info("%s: fingerprint device detecting\n", __func__);
	} else {
		hwlog_err("%s: get sensor_vendor err\n", __func__);
		ret = RET_FAIL;
	}

	if (sensor_manager[index].detect_result == DET_FAIL) {
		if (irq_value == GPIO_STAT_HIGH) {
			gpio_direction_output(gpio_cfg.gpio_reset, reset_value);
			gpio_direction_output(gpio_cfg.gpio_irq, irq_value);
		}
		hwlog_info("%s: irq_value = %d, reset_value = %d\n",
			__func__, irq_value, reset_value);
	}

	return ret;
}

static int inputhub_fingerprint_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };

	if (!dn || !sm)
		return -1;

	p = sm + index;
	ret = fingerprint_sensor_detect(dn, index, &cfg);
	if (ret == FINGERPRINT_SENSOR_DETECT_SUCCESS) {
		ret = 0;
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
		return ret;
	} else if (ret) {
		return ret;
	}

	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}
	return ret;
}

static void set_goodix_udg2_config(struct fp_gpio_config *gpio_cfg)
{
	uint8_t tx[3] = { 0 }; /* 3 : tx register length */
	uint32_t tx_len;
	union spi_ctrl ctrl;

	fingerprint_gpio_reset(gpio_cfg->gpio_reset,
		RESET_LONG_SLEEP, RESET_LONG_SLEEP);

	msleep(RESET_LONG_SLEEP);
	ctrl.data = gpio_cfg->gpio_cs;

	/* set sensor to idle mode, cmd is 0xC0, lenth is 1 */
	tx[0] = 0xC0;
	tx_len = 1;
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
	msleep(100); /* 100 : sleep 100ms after set sensor to idel */

	/* write cmd 0xF0 & address 0x4304, length is 3 */
	tx[0] = 0xF0;
	tx[1] = 0x43;
	tx[2] = 0x04;
	tx_len = 3; /* 3 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
}

static void set_f03001_cmd(GPIO_NUM_TYPE gpio_cs)
{
	uint8_t tx = 0xC0; /* set sensor to idle mode, cmd is 0xC0 */
	uint32_t tx_len = sizeof(tx);
	union spi_ctrl ctrl;

	ctrl.data = gpio_cs;
	mcu_spi_rw(FP_SPI_NUM, ctrl, &tx, tx_len, NULL, 0);
	msleep(4); /* 4 : sleep 4ms after set sensor to idle */
}

static bool compare_f03003_reg_value(GPIO_NUM_TYPE gpio_cs)
{
	uint8_t tx[FINGERPRINT_WRITE_CMD_LEN] = {0};
	uint8_t rx[FINGERPRINT_READ_CONTENT_LEN] = {0};
	uint8_t reg_value[2] = { 0xA5, 0x5A };
	uint32_t tx_len;
	uint32_t rx_len;
	union spi_ctrl ctrl;

	/* write cmd 0xF0 & address 0x0236, length is 3 */
	ctrl.data = gpio_cs;
	tx[0] = 0xF0;
	tx[1] = 0x02;
	tx[2] = 0x36;
	tx_len = 3; /* 3 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);

	/* read cmd 0xF1, cmd length is 1, rx length is 2 */
	tx[0] = 0xF1;
	tx_len = 1;
	rx_len = 2; /* 2 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, rx, rx_len);

	if (rx[0] == reg_value[0] && rx[1] == reg_value[1]) {
		hwlog_info("%s: goodix compare reg value success\n", __func__);
		return true;
	}

	hwlog_info("%s: goodix compare reg value failed\n", __func__);
	return false;
}

static void set_f03003_cmd(GPIO_NUM_TYPE gpio_cs)
{
	uint8_t tx = 0xC0; /* set sensor to idle mode, cmd is 0xC0 */
	uint32_t tx_len = sizeof(tx);
	union spi_ctrl ctrl;

	ctrl.data = gpio_cs;
	mcu_spi_rw(FP_SPI_NUM, ctrl, &tx, tx_len, NULL, 0);
	msleep(4); /* 4 : sleep 4ms after set sensor to idle */
}

static void f03_ud_common_handle(GPIO_NUM_TYPE gpio_cs, const uint32_t size,
	const struct f03_reg_info *reg_info)
{
	uint32_t tx_len;
	union spi_ctrl ctrl;
	uint32_t i;
	uint8_t tx[F03_WRITE_WAKEUP_CMD_TX_LEN] = {0};

	ctrl.data = gpio_cs;
	if (reg_info != NULL && size != 0) {
		tx[0] = F03_WRITE_CMD;
		tx[3] = WORD_LEN_HIGH_BIT;
		tx[4] = WORD_LEN_LOW_BIT;
		tx_len = F03_WRITE_WAKEUP_CMD_TX_LEN;
		for (i = 0; i < size; i++) {
			tx[1] = reg_info[i].addr >> 8; /* shift 8 bits */
			tx[2] = reg_info[i].addr;
			tx[5] = reg_info[i].value >> 8; /* shift 8 bits */
			tx[6] = reg_info[i].value;
			mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);

			if (reg_info[i].delay)
				msleep(reg_info[i].delay);
		}
	}

	tx[0] = F03_WRITE_CMD;
	tx[1] = F03_CHIP_ID_ADDR_HIGH;
	tx[2] = F03_CHIP_ID_ADDR_LOW;
	tx_len = F03_WRITE_CMD_TX_LEN;
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
}

static void set_f03006_config(struct fp_gpio_config *gpio_cfg)
{
	struct f03_reg_info reg_info[] = {
		{ 0xE500, 0x0000, 1 },
		{ 0x00E0, 0x0150, 1 },
		{ 0xE600, 0x0000, 1 },
		{ 0x00E0, 0x0010, 0 },
		{ 0x00E2, 0x00A8, 0 },
		{ 0x00E2, 0x0028, 0 },
		{ 0x00E0, 0x0000, 1 },
	};
	const uint32_t size = ARRAY_SIZE(reg_info);

	f03_ud_common_handle(gpio_cfg->gpio_cs, size, reg_info);
}

static void set_f03005_config(struct fp_gpio_config *gpio_cfg)
{
	struct f03_reg_info reg_info[] = {
		{ 0xE500, 0x0000, 2 },
		{ 0x00C2, 0xBFD1, 0 },
		{ 0xE600, 0x0000, 0 },
		{ 0x00C2, 0xBFD0, 0 },
		{ 0x00C2, 0xBED0, 0 },
		{ 0x00C2, 0x3ED0, 0 },
		{ 0x00C2, 0x7ED0, 5 },
		{ 0x00C2, 0x7E50, 2 },
		{ 0x00C2, 0x7E10, 0 },
		{ 0x0018, 0x0414, 0 },
		{ 0x001A, 0x0000, 0 },
		{ 0x001C, 0x0006, 0 },
	};
	const uint32_t size = ARRAY_SIZE(reg_info);

	f03_ud_common_handle(gpio_cfg->gpio_cs, size, reg_info);
}

static void set_f03001_config(struct fp_gpio_config *gpio_cfg)
{
	struct f03_reg_info reg_info[] = {
		{ 0x0130, 0x4101, 0 },
		{ 0x0132, 0x0000, 0 },
		{ 0x0092, 0x0AA3, 0 },
		{ 0x0094, 0x4370, 0 },
	};
	const uint32_t size = ARRAY_SIZE(reg_info);
	GPIO_NUM_TYPE gpio_cs = gpio_cfg->gpio_cs;

	set_f03001_cmd(gpio_cs);
	f03_ud_common_handle(gpio_cs, size, reg_info);
}

static void set_f03003_config(struct fp_gpio_config *gpio_cfg)
{
	uint32_t i;
	uint32_t index;
	uint32_t compare_times = 2;
	bool ret = true;
	GPIO_NUM_TYPE gpio_cs = gpio_cfg->gpio_cs;

	for (i = 0; i < F03003_WAKE_RETRY_TIMES; i++) {
		set_f03003_cmd(gpio_cs);

		/* compare reg value twice */
		for (index = 0; index < compare_times; index++)
			ret = ret && compare_f03003_reg_value(gpio_cs);

		if (ret)
			break;
	}

	f03_ud_common_handle(gpio_cs, 0, NULL);
}

static void set_f03_ud_config(struct fp_gpio_config *gpio_cfg)
{
	uint8_t tx[3] = { 0 }; /* 3 : tx register length */
	uint32_t tx_len;
	union spi_ctrl ctrl;

	fingerprint_gpio_reset(gpio_cfg->gpio_reset,
		RESET_LONG_SLEEP, RESET_LONG_SLEEP);

	ctrl.data = gpio_cfg->gpio_cs;

	/* set sensor to idle mode, cmd is 0xC0, lenth is 1 */
	tx[0] = 0xC0;
	tx_len = 1;
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
	msleep(100); /* 100 : sleep 100ms after set sensor to idel */

	/* write cmd 0xF0 & address 0x0004, length is 3 */
	tx[0] = 0xF0;
	tx[1] = 0x00;
	tx[2] = 0x04;
	tx_len = 3; /* 3 bytes */
	mcu_spi_rw(FP_SPI_NUM, ctrl, tx, tx_len, NULL, 0);
}

static void set_f09_ud_config(struct fp_gpio_config *gpio_cfg)
{
	fingerprint_gpio_reset(gpio_cfg->gpio_reset,
		RESET_SHORT_SLEEP, RESET_LONG_SLEEP);
	msleep(RESET_LONG_SLEEP);
}

static const struct fp_sensor_config g_fp_ud_cfgs[] = {
	{ "goodix,goodixG2", GF_G2_G3_NAME_LEN, set_goodix_udg2_config },
	{ "goodix,goodixG3", GF_G2_G3_NAME_LEN, set_f03006_config },
	{ "goodix", GOODIX_NAME_LEN, set_f03_ud_config },
	{ "syna", SYNA_NAME_LEN, set_f02_config },
	{ "silead", SILEAD_NAME_LEN, set_f04_config },
	{ "egis", EGIS_NAME_LEN, set_f09_ud_config },
	{ "F03006", FP_IC_MATCH_LEN, set_f03006_config },
	{ "F03005", FP_IC_MATCH_LEN, set_f03005_config },
	{ "F03001", FP_IC_MATCH_LEN, set_f03001_config },
	{ "F03003", FP_IC_MATCH_LEN, set_f03003_config },
	{ "F03004", FP_IC_MATCH_LEN, set_f03003_config },
	{ "F03", FP_MATCH_LEN, set_f03_ud_config },
	{ "F02", FP_MATCH_LEN, set_f02_config },
	{ "F04", FP_MATCH_LEN, set_f04_config },
	{ "F09", FP_MATCH_LEN, set_f09_ud_config },
	{ "F10", FP_MATCH_LEN, set_f09_ud_config },
};

static int fingerprint_ud_sensor_detect(struct device_node *dn, int index,
	struct sensor_combo_cfg *cfg)
{
	int ret;
	int i;
	int irq_value = 0;
	int reset_value = 0;
	char *sensor_vendor = NULL;
	struct fp_gpio_config gpio_cfg = { 0, 0, 0 };

	if (fingerprint_get_gpio_config(dn, &gpio_cfg) != 0)
		hwlog_err("%s:read fingerprint gpio fail\n", __func__);

	if (sensor_manager[index].detect_result == DET_FAIL) {
		reset_value = gpio_get_value(gpio_cfg.gpio_reset);
		irq_value = gpio_get_value(gpio_cfg.gpio_irq);
	}

	ret = of_property_read_string(dn,
		"compatible", (const char **)&sensor_vendor);
	if (!ret) {
		if (!strncmp(sensor_vendor, "qfp", QFP_NAME_LEN)) {
			_device_detect(dn, index, cfg);
			hwlog_info("%s: fingerprint device %s detect bypass\n",
				__func__, sensor_vendor);
			return FINGERPRINT_SENSOR_DETECT_SUCCESS;
		}

		for (i = 0; i < ARRAY_SIZE(g_fp_ud_cfgs); i++) {
			if (!strncmp(sensor_vendor, g_fp_ud_cfgs[i].ic_code,
				g_fp_ud_cfgs[i].length)) {
				(*g_fp_ud_cfgs[i].func)(&gpio_cfg);
				hwlog_info("%s: g_fp_ud_cfgs index: %d\n",
					__func__, i);
				break;
			}
		}
		hwlog_info("%s: fingerprint device detecting\n", __func__);
	} else {
		hwlog_err("%s: get sensor_vendor err\n", __func__);
		ret = RET_FAIL;
	}

	if (sensor_manager[index].detect_result == DET_FAIL) {
		if (irq_value == GPIO_STAT_HIGH)
			gpio_direction_output(gpio_cfg.gpio_reset, reset_value);
		hwlog_info("%s: irq_value = %d, reset_value = %d\n",
			__func__, irq_value, reset_value);
	}

	return ret;
}

static int inputhub_fingerprint_ud_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };

	if (!dn || !sm)
		return -1;

	p = sm + index;
	ret = fingerprint_ud_sensor_detect(dn, index, &cfg);
	if (ret == FINGERPRINT_SENSOR_DETECT_SUCCESS) {
		ret = 0;
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
		return ret;
	} else if (ret) {
		return ret;
	}

	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}
	return ret;
}

#define TP_VENDOR_NAME_LEN 50
static bool tp_multi_vendor_name_cmp(const char *name, const char *child_name,
	unsigned int index)
{
	char str[TP_VENDOR_NAME_LEN] = {0};
	char *token = NULL;
	char *save_ptr = NULL;
	unsigned int i = 0;

	if (memcpy_s(str, sizeof(str), name, strlen(name))) {
		hwlog_err("%s:memcpy_s failed\n", __func__);
		return false;
	}

	token = strtok_s(str, ",", &save_ptr);
	while ((token != NULL) && (i++ < index))
		token = strtok_s(NULL, ",", &save_ptr);
	if (token == NULL) {
		hwlog_err("%s:no vendor name of index:%u\n", __func__, index);
		goto err;
	}

	if (!strcmp(token, child_name)) {
		hwlog_info("%s:success, vendor: %s\n", __func__, child_name);
		return true;
	}
err:
	hwlog_err("%s:tp detect err, attached name: %s,ic:%s\n", __func__,
		name, child_name);
	return false;
}

__weak const char *ts_kit_get_vendor_name(void) { return NULL; }
static int tp_ud_sensor_detect(struct device_node *dn,
	enum sensor_detect_list sensor_id)
{
	int ret;
	struct tp_ud_platform_data *tp_data_config = NULL;
	struct tp_multi_panel_info info = {0};
	const char *configed_vendor_name = NULL;
	const char *attached_vendor_name = NULL;
	const char *bus_type = NULL;
	int temp = 0;

	if (sensor_id == TP_UD)
		tp_data_config = &tp_ud_data;
	else
		tp_data_config = &tp_ud_data_extra;

	ret = of_property_read_string(dn, "vendor_name", &configed_vendor_name);
	if (ret) {
		hwlog_err("%s:ic name not configed\n", __func__);
		return ret;
	}

	ret = of_property_read_string(dn, "bus_type", &bus_type);
	if (ret) {
		hwlog_err("%s:ic name not configed\n", __func__);
		return ret;
	}

	ret = get_combo_bus_tag(bus_type, (uint8_t *)&temp);
	if (ret) {
		hwlog_err("%s:bus_typeis invalid\n", __func__);
		return ret;
	}
	tp_data_config->cfg.bus_type = temp;

	ret = of_property_read_u32(dn, "bus_number", &temp);
	if (ret) {
		hwlog_err("%s:bus_number not configed\n", __func__);
		return ret;
	}
	tp_data_config->cfg.bus_num = temp;

	if (tp_data_config->cfg.bus_type == TAG_I2C  ||
		tp_data_config->cfg.bus_type == TAG_I3C) {
		ret = of_property_read_u32(dn, "i2c_address", &temp);
		if (ret) {
			hwlog_err("%s:i2c_address not configed\n", __func__);
			return ret;
		}
		tp_data_config->cfg.i2c_address = temp;
	}

	attached_vendor_name = ts_kit_get_vendor_name();
	if (!attached_vendor_name) {
		hwlog_err("%s:no attached ic\n", __func__);
		return -ENODEV;
	}
	ret = of_property_read_u32(dn, "support_multi_panel_attach",
		&(info.support_multi_panel_attach));
	if (ret) {
		hwlog_info("%s:not support multi panel attach\n", __func__);
		info.support_multi_panel_attach = 0;
	}
	if (info.support_multi_panel_attach) {
		ret = of_property_read_u32(dn, "multi_panel_index",
			&(info.panel_index));
		if (ret) {
			hwlog_err("%s:panel index not configed\n", __func__);
			return ret;
		}
		if (tp_multi_vendor_name_cmp(attached_vendor_name,
			configed_vendor_name, info.panel_index)) {
			hwlog_info("%s: %s detect success\n", __func__,
				configed_vendor_name);
			return 0;
		}
	} else if (!strcmp(configed_vendor_name, attached_vendor_name)) {
		hwlog_debug("%s:tp detect succ, ic:%s\n",
			__func__, configed_vendor_name);
		return 0;
	}

	return -ENODEV;
}

__weak const int thp_project_id_provider(char *project_id,
	unsigned int len)
{
	return 0;
}

__weak const int get_thp_unregister_ic_num(void)
{
	return 0;
}

static int thp_sensor_detect(struct device_node *dn)
{
	int ret;
	const char *configed_vendor_name = NULL;
	const char *attached_vendor_name = NULL;
	const char *bus_type = NULL;
	int temp = 0;
	int i;
	 /* retry 200 times waiting for tp register complete */
	unsigned int retry_times = 200;

	hwlog_info("%s: enter\n", __func__);
	while (retry_times--) {
		ret = thp_project_id_provider(thp_data.project_id,
			THP_MAX_PROJECT_ID_LENGTH);
		if (!ret) {
			hwlog_info("%s: project id is ok\n", __func__);
			break;
		}
		if (get_thp_unregister_ic_num() == 0) {
			hwlog_info("%s:tp have registered\n", __func__);
			break;
		}
		mdelay(5); /* delay 5ms */
	}
	ret = thp_project_id_provider(thp_data.project_id,
		THP_MAX_PROJECT_ID_LENGTH);
	hwlog_info("thp project_id = %s\n", thp_data.project_id);
	for (i = 0; i < THP_MAX_PROJECT_ID_LENGTH; i++)
		hwlog_info("project_id[%d]= %d\n", i, thp_data.project_id[i]);
	if (ret < 0) {
		hwlog_info("%s: project id is error\n", __func__);
		return ret;
	}

	ret = of_property_read_string(dn, "vendor_name", &configed_vendor_name);
	if (ret) {
		hwlog_err("%s:ic name not configed\n", __func__);
		return ret;
	}

	ret = of_property_read_string(dn, "bus_type", &bus_type);
	if (ret) {
		hwlog_err("%s:ic name not configed\n", __func__);
		return ret;
	}
	ret = get_combo_bus_tag(bus_type, (uint8_t *)&temp);
	if (ret) {
		hwlog_err("%s:bus_typeis invalid\n", __func__);
		return ret;
	}
	thp_data.cfg.bus_type = (uint8_t)temp;
	ret = of_property_read_u32(dn, "bus_number", &temp);
	if (ret) {
		hwlog_err("%s:bus_number not configed\n", __func__);
		return ret;
	}
	thp_data.cfg.bus_num = (uint8_t)temp;

	if (thp_data.cfg.bus_type == TAG_I2C ||
		thp_data.cfg.bus_type == TAG_I3C) {
		ret = of_property_read_u32(dn, "i2c_address", &temp);
		if (ret) {
			hwlog_err("%s:i2c_address not configed\n", __func__);
			return ret;
		}
		thp_data.cfg.i2c_address = (uint32_t)temp;
	}

	attached_vendor_name = ts_kit_get_vendor_name();
	if (!attached_vendor_name) {
		hwlog_err("%s:no attached ic\n", __func__);
		return -ENODEV;
	}

	if (!strcmp(configed_vendor_name, attached_vendor_name)) {
		hwlog_info("%s:thp detect succ\n", __func__);
		return 0;
	}

	return -ENODEV;
}

static void parse_tp_ud_algo_conf(struct tp_ud_platform_data *tp_data_config)
{
	int temp = 0;
	struct device_node *dn;

	dn = of_find_compatible_node(NULL, NULL, "up_tp,algo_config");
	if (!dn) {
		hwlog_err("%s:no config, skip\n", __func__);
		memset(&tp_data_config->algo_conf, 0,
			sizeof(struct tp_ud_algo_config));
		return;
	}

	if (of_property_read_u32(dn, "move_area_x_min", &temp))
		hwlog_err("%s:read move_area_x_min fail\n", __func__);
	else
		tp_data_config->algo_conf.move_area_x_min = temp;

	if (of_property_read_u32(dn, "move_area_x_max", &temp))
		hwlog_err("%s:read move_area_x_max fail\n", __func__);
	else
		tp_data_config->algo_conf.move_area_x_max = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "move_area_y_min", &temp))
		hwlog_err("%s:read move_area_y_min fail\n", __func__);
	else
		tp_data_config->algo_conf.move_area_y_min = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "move_area_y_max", &temp))
		hwlog_err("%s:read move_area_y_max fail\n", __func__);
	else
		tp_data_config->algo_conf.move_area_y_max = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_x_min", &temp))
		hwlog_err("%s:read finger_area_x_min fail\n", __func__);
	else
		tp_data_config->algo_conf.finger_area_x_min =
			(GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_x_max", &temp))
		hwlog_err("%s:read finger_area_x_max fail\n", __func__);
	else
		tp_data_config->algo_conf.finger_area_x_max =
			(GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_y_min", &temp))
		hwlog_err("%s:read finger_area_y_min fail\n", __func__);
	else
		tp_data_config->algo_conf.finger_area_y_min =
			(GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_y_max", &temp))
		hwlog_err("%s:read finger_area_y_max fail\n", __func__);
	else
		tp_data_config->algo_conf.finger_area_y_max =
			(GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "coor_scale", &temp))
		hwlog_err("%s:read coor_scale fail\n", __func__);
	else
		tp_data_config->algo_conf.coor_scale = (GPIO_NUM_TYPE)temp;
}

static void read_tp_ud_config1_from_dts(struct device_node *dn,
	struct tp_ud_platform_data *tp_data_config)
{
	int temp = 0;

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read tp ud file_id fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;

	dyn_req->file_count++;
	hwlog_info("tp ud file id is %d\n", temp);

	if (of_property_read_u32(dn, "spi_max_speed_hz", &temp))
		hwlog_err("%s:read spi_max_speed_hz fail\n", __func__);
	else
		tp_data_config->spi_max_speed_hz = temp;

	if (of_property_read_u32(dn, "spi_mode", &temp))
		hwlog_err("%s:read spi_mode fail\n", __func__);
	else
		tp_data_config->spi_mode = temp;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
		hwlog_err("%s:read gpio_irq fail\n", __func__);
	else
		tp_data_config->gpio_irq = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
		hwlog_err("%s:read gpio_irq_sh fail\n", __func__);
	else
		tp_data_config->gpio_irq_sh = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "gpio_cs", &temp))
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	else
		tp_data_config->gpio_cs = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "gpio_irq_pull_up_status", &temp)) {
		hwlog_err("%s:read gpio_irq_flag fail\n", __func__);
		tp_data_config->gpio_irq_pull_up_status = 0;
	} else {
		tp_data_config->gpio_irq_pull_up_status = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read gpio_irq_pull_up_status %d\n", __func__,
			tp_data_config->gpio_irq_pull_up_status);
	}
}

static void read_tp_ud_config2_from_dts(struct device_node *dn,
	struct tp_ud_platform_data *tp_data_config)
{
	int temp = 0;

	if (of_property_read_u32(dn, "pressure_support", &temp))
		hwlog_err("%s:read pressure_support fail\n", __func__);
	else
		tp_data_config->pressure_support = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "anti_forgery_support", &temp))
		hwlog_err("%s:read anti_forgery_support fail\n", __func__);
	else
		tp_data_config->anti_forgery_support = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "ic_type", &temp))
		hwlog_err("%s:read low_power_addr fail\n", __func__);
	else
		tp_data_config->ic_type = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "hover_enable", &temp))
		hwlog_err("%s:read event_info_addr fail  use default\n", __func__);
	else
		tp_data_config->hover_enable = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "i2c_max_speed_hz", &temp))
		hwlog_err("%s:read i2c_max_speed_hz fail\n", __func__);
	else
		tp_data_config->i2c_max_speed_hz = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "fw_power_config_reg", &temp)) {
		hwlog_err("%s:read fw_power_config_reg not config\n", __func__);
		tp_data_config->fw_power_config_reg = 0;
	} else {
		tp_data_config->fw_power_config_reg = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_power_config_reg = %d\n", __func__,
			tp_data_config->fw_power_config_reg);
	}
	if (of_property_read_u32(dn, "fw_touch_data_reg", &temp)) {
		hwlog_err("%s:read fw_touch_data_reg not config\n", __func__);
		tp_data_config->fw_touch_data_reg = 0;
	} else {
		tp_data_config->fw_touch_data_reg = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_touch_data_reg = %d\n", __func__,
			tp_data_config->fw_touch_data_reg);
	}
}

static void read_tp_ud_config3_from_dts(struct device_node *dn,
	struct tp_ud_platform_data *tp_data_config)
{
	int temp = 0;

	if (of_property_read_u32(dn, "fw_touch_command_reg", &temp)) {
		hwlog_err("%s:read fw_touch_command_reg not config\n", __func__);
		tp_data_config->fw_touch_command_reg = 0;
	} else {
		tp_data_config->fw_touch_command_reg = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_touch_command_reg = %d\n", __func__,
			tp_data_config->fw_touch_command_reg);
	}
	if (of_property_read_u32(dn, "fw_addr_3", &temp)) {
		hwlog_err("%s:read fw_addr_3 not config\n", __func__);
		tp_data_config->fw_addr_3 = 0;
	} else {
		tp_data_config->fw_addr_3 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_3 = %d\n", __func__,
			tp_data_config->fw_addr_3);
	}

	if (of_property_read_u32(dn, "fw_addr_4", &temp)) {
		hwlog_err("%s:read fw_addr_4 not config\n", __func__);
		tp_data_config->fw_addr_4 = 0;
	} else {
		tp_data_config->fw_addr_4 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_4 = %d\n", __func__,
			tp_data_config->fw_addr_4);
	}

	if (of_property_read_u32(dn, "fw_addr_5", &temp)) {
		hwlog_err("%s:read fw_addr_5 not config\n", __func__);
		tp_data_config->fw_addr_5 = 0;
	} else {
		tp_data_config->fw_addr_5 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_5 = %d\n", __func__,
			tp_data_config->fw_addr_5);
	}

	if (of_property_read_u32(dn, "fw_addr_6", &temp)) {
		hwlog_err("%s:read fw_addr_6 not config\n", __func__);
		tp_data_config->fw_addr_6 = 0;
	} else {
		tp_data_config->fw_addr_6 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_6 = %d\n", __func__,
			tp_data_config->fw_addr_6);
	}

	if (of_property_read_u32(dn, "fw_addr_7", &temp)) {
		hwlog_err("%s:read fw_addr_7 not config\n", __func__);
		tp_data_config->fw_addr_7 = 0;
	} else {
		tp_data_config->fw_addr_7 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_7 = %d\n", __func__,
			tp_data_config->fw_addr_7);
	}
}

static void read_tp_ud_config4_from_dts(struct device_node *dn,
	struct tp_ud_platform_data *tp_data_config)
{
	int temp = 0;

	if (of_property_read_u32(dn, "tp_sensorhub_irq_flag", &temp)) {
		hwlog_err("%s:read tp_sensorhub_irq_flag not config\n", __func__);
		tp_data_config->tp_sensorhub_irq_flag = 0;
	} else {
		tp_data_config->tp_sensorhub_irq_flag = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read tp_sensorhub_irq_flag = %d\n", __func__,
			tp_data_config->tp_sensorhub_irq_flag);
	}
	if (of_property_read_u32(dn, "touch_report_restore_support", &temp)) {
		hwlog_info("%s: read touch_report_restore_support not config\n",
			__func__);
		tp_data_config->touch_report_restore_support = 0;
	} else {
		tp_data_config->touch_report_restore_support = (uint16_t)temp;
		hwlog_info("%s: read touch_report_restore_support = %u\n",
			__func__, tp_data_config->touch_report_restore_support);
	}
	if (of_property_read_u32(dn,
		"tp_sensor_spi_sync_cs_low_delay_us", &temp)) {
		hwlog_err("%s:read tp_sensor_spi_sync_cs_low_delay_us not config\n",
			__func__);
		tp_data_config->tp_sensor_spi_sync_cs_low_delay_us = 0;
	} else {
		tp_data_config->tp_sensor_spi_sync_cs_low_delay_us =
							(GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read tp_sensor_spi_sync_cs_low_delay_us = %d\n",
			__func__,
			tp_data_config->tp_sensor_spi_sync_cs_low_delay_us);
	}
	if (of_property_read_u32(dn, "soft_reset_support", &temp)) {
		hwlog_info("%s:read soft_reset_support not config\n", __func__);
		tp_data_config->soft_reset_support = 0; /* default value */
	} else {
		tp_data_config->soft_reset_support = (uint16_t)temp;
		hwlog_info("%s:read soft_reset_support = %u\n",
			__func__, tp_data_config->soft_reset_support);
	}
	if (of_property_read_u32(dn, "tp_sensorhub_platform", &temp)) {
		hwlog_info("%s:read tp_sensorhub_platform not config\n", __func__);
		tp_data_config->tp_sensorhub_platform = 4; /* default value 4 */
	} else {
		tp_data_config->tp_sensorhub_platform = (uint16_t)temp;
		hwlog_info("%s:read tp_sensorhub_platform = %u\n",
			__func__, tp_data_config->tp_sensorhub_platform);
	}
}

static void read_tp_ud_config5_from_dts(struct device_node *dn,
	struct tp_ud_platform_data *tp_data_config)
{
	int temp = 0;

	if (of_property_read_u32(dn, "aod_display_support", &temp)) {
		hwlog_info("%s:read aod_display_support not config\n", __func__);
		tp_data_config->aod_display_support = 0; /* default value */
	} else {
		tp_data_config->aod_display_support = (uint16_t)temp;
		hwlog_info("%s:read aod_display_support = %u\n",
			__func__, tp_data_config->aod_display_support);
	}
	if (of_property_read_u32(dn, "tsa_event_to_udfp", &temp)) {
		hwlog_info("%s:read tsa_event_to_udfp not config\n", __func__);
		tp_data_config->tsa_event_to_udfp = 0; /* default value */
	} else {
		tp_data_config->tsa_event_to_udfp = (uint16_t)temp;
		hwlog_info("%s:read tsa_event_to_udfp = %u\n",
			__func__, tp_data_config->tsa_event_to_udfp);
	}
	if (of_property_read_u32(dn, "multi_ic_id", &temp)) {
		hwlog_info("%s:read multi_ic_id not config\n", __func__);
		tp_data_config->multi_ic_id = 0;
	} else {
		tp_data_config->multi_ic_id = (uint16_t)temp;
		hwlog_info("%s:read multi_ic_id = %u\n",
			__func__, tp_data_config->multi_ic_id);
	}
}

static void read_tp_ud_from_dts(struct device_node *dn,
	enum sensor_detect_list s_id)
{
	struct tp_ud_platform_data *tp_data_config = NULL;

	if (s_id == TP_UD) {
		tp_data_config = &tp_ud_data;
		read_chip_info(dn, TP_UD);
	} else {
		tp_data_config = &tp_ud_data_extra;
		read_chip_info(dn, TP_UD_EXTRA);
	}
	hwlog_info("tp ud s_id is %d\n", s_id);
	parse_tp_ud_algo_conf(tp_data_config);

	read_tp_ud_config1_from_dts(dn, tp_data_config);
	read_tp_ud_config2_from_dts(dn, tp_data_config);
	read_tp_ud_config3_from_dts(dn, tp_data_config);
	read_tp_ud_config4_from_dts(dn, tp_data_config);
	read_tp_ud_config5_from_dts(dn, tp_data_config);
}

static void parse_thp_algo_conf(void)
{
	int temp = 0;
	struct device_node *dn = NULL;

	dn = of_find_compatible_node(NULL, NULL, "thp,algo_config");
	if (!dn) {
		hwlog_err("%s:no config, skip\n", __func__);
		memset(&thp_data.algo_conf, 0, sizeof(struct tp_ud_algo_config));
		return;
	}

	if (of_property_read_u32(dn, "move_area_x_min", &temp))
		hwlog_err("%s:read move_area_x_min fail\n", __func__);
	else
		thp_data.algo_conf.move_area_x_min = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "move_area_x_max", &temp))
		hwlog_err("%s:read move_area_x_max fail\n", __func__);
	else
		thp_data.algo_conf.move_area_x_max = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "move_area_y_min", &temp))
		hwlog_err("%s:read move_area_y_min fail\n", __func__);
	else
		thp_data.algo_conf.move_area_y_min = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "move_area_y_max", &temp))
		hwlog_err("%s:read move_area_y_max fail\n", __func__);
	else
		thp_data.algo_conf.move_area_y_max = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_x_min", &temp))
		hwlog_err("%s:read finger_area_x_min fail\n", __func__);
	else
		thp_data.algo_conf.finger_area_x_min = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_x_max", &temp))
		hwlog_err("%s:read finger_area_x_max fail\n", __func__);
	else
		thp_data.algo_conf.finger_area_x_max = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_y_min", &temp))
		hwlog_err("%s:read finger_area_y_min fail\n", __func__);
	else
		thp_data.algo_conf.finger_area_y_min = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "finger_area_y_max", &temp))
		hwlog_err("%s:read finger_area_y_max fail\n", __func__);
	else
		thp_data.algo_conf.finger_area_y_max = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "coor_scale", &temp))
		hwlog_err("%s:read coor_scale fail\n", __func__);
	else
		thp_data.algo_conf.coor_scale = (GPIO_NUM_TYPE)temp;
}

static void parse_thp_dfx_conf(void)
{
	int temp = 0;
	struct device_node *dn = NULL;

	dn = of_find_compatible_node(NULL, NULL, "thp,dfx_config");
	if (!dn) {
		hwlog_err("%s:no config, skip\n", __func__);
		return;
	}

	if (of_property_read_u32(dn, "shb_thp_log", &temp))
		hwlog_err("%s:read shb_thp_log fail\n", __func__);
	else
		thp_data.shb_thp_log = (uint8_t)temp;
}

static void read_thp_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, THP);
	parse_thp_algo_conf();
	parse_thp_dfx_conf();

	if (of_property_read_u32(dn, "file_id0", &temp))
		hwlog_err("%s:read thp file_id0 fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;

	dyn_req->file_count++;
	hwlog_info("shb thp app file id is %d\n", temp);

	if (of_property_read_u32(dn, "file_id1", &temp))
		hwlog_err("%s:read thp file_id1 fail\n", __func__);
	else
		dyn_req->file_list[dyn_req->file_count] = (uint16_t)temp;

	dyn_req->file_count++;
	hwlog_info("shb thp driver file id is %d\n", temp);

	if (of_property_read_u32(dn, "support_fp_ud", &temp)) {
		hwlog_err("%s:read support_fp_ud fail\n", __func__);
	} else {
		thp_data.gestures.support_fp_ud = (uint8_t)temp;
		hwlog_err("%s:read support_fp_ud = %d.\n", __func__, temp);
	}

	if (of_property_read_u32(dn, "support_virtual_key", &temp)) {
		hwlog_err("%s:read support_virtual_key fail\n", __func__);
	} else {
		thp_data.gestures.support_virtual_key = (uint8_t)temp;
		hwlog_err("%s:read support_virtual_key = %d.\n", __func__, temp);
	}

	if (of_property_read_u32(dn, "support_double_click", &temp)) {
		hwlog_err("%s:read support_double_click fail\n", __func__);
	} else {
		thp_data.gestures.support_double_click = (uint8_t)temp;
		hwlog_err("%s:read support_double_click = %d.\n", __func__, temp);
	}

	if (of_property_read_u32(dn, "support_hw_m_pen", &temp)) {
		hwlog_err("%s:read support_hw_m_pen fail\n", __func__);
	} else {
		thp_data.gestures.support_hw_m_pen = (uint8_t)temp;
		hwlog_err("%s:read support_hw_m_pen = %d.\n", __func__, temp);
	}

	if (of_property_read_u32(dn, "spi_max_speed_hz", &temp))
		hwlog_err("%s:read spi_max_speed_hz fail\n", __func__);
	else
		thp_data.spi_max_speed_hz = (uint32_t)temp;

	if (of_property_read_u32(dn, "spi_mode", &temp))
		hwlog_err("%s:read spi_mode fail\n", __func__);
	else
		thp_data.spi_mode = (uint8_t)temp;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
		hwlog_err("%s:read gpio_irq fail\n", __func__);
	else
		thp_data.gpio_irq = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_irq_sh", &temp))
		hwlog_err("%s:read gpio_irq_sh fail\n", __func__);
	else
		thp_data.gpio_irq_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_cs", &temp))
		hwlog_err("%s:read gpio_cs fail\n", __func__);
	else
		thp_data.gpio_cs = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "feature_config", &temp))
		hwlog_err("%s:read feature_config fail\n", __func__);
	else
		thp_data.feature_config = (GPIO_NUM_TYPE)temp;
	hwlog_info("%s:read feature_config = %u\n",
		__func__, thp_data.feature_config);

	if (of_property_read_u32(dn, "anti_forgery_support", &temp))
		hwlog_err("%s:read anti_forgery_support fail\n", __func__);
	else
		thp_data.anti_forgery_support = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "ic_type", &temp))
		hwlog_err("%s:read low_power_addr fail\n", __func__);
	else
		thp_data.ic_type = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "hover_enable", &temp))
		hwlog_err("%s:read event_info_addr fail use default\n", __func__);
	else
		thp_data.hover_enable = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "i2c_max_speed_hz", &temp))
		hwlog_err("%s:read i2c_max_speed_hz fail\n", __func__);
	else
		thp_data.i2c_max_speed_hz = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "fw_power_config_reg", &temp)) {
		hwlog_err("%s:read fw_power_config_reg not config\n", __func__);
		thp_data.fw_power_config_reg = 0;
	} else {
		thp_data.fw_power_config_reg = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_power_config_reg = %d\n", __func__,
			thp_data.fw_power_config_reg);
	}

	if (of_property_read_u32(dn, "fw_touch_data_reg", &temp)) {
		hwlog_err("%s:read fw_touch_data_reg not config\n", __func__);
		thp_data.fw_touch_data_reg = 0;
	} else {
		thp_data.fw_touch_data_reg = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_touch_data_reg = %d\n", __func__,
			thp_data.fw_touch_data_reg);
	}

	if (of_property_read_u32(dn, "fw_touch_command_reg", &temp)) {
		hwlog_err("%s:read fw_touch_command_reg not config\n", __func__);
		thp_data.fw_touch_command_reg = 0;
	} else {
		thp_data.fw_touch_command_reg = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_touch_command_reg = %d\n", __func__,
			thp_data.fw_touch_command_reg);
	}

	if (of_property_read_u32(dn, "fw_addr_3", &temp)) {
		hwlog_err("%s:read fw_addr_3 not config\n", __func__);
		thp_data.fw_addr_3 = 0;
	} else {
		thp_data.fw_addr_3 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_3 = %d\n", __func__,
			thp_data.fw_addr_3);
	}

	if (of_property_read_u32(dn, "fw_addr_4", &temp)) {
		hwlog_err("%s:read fw_addr_4 not config\n", __func__);
		thp_data.fw_addr_4 = 0;
	} else {
		thp_data.fw_addr_4 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_4 = %d\n", __func__,
			thp_data.fw_addr_4);
	}

	if (of_property_read_u32(dn, "fw_addr_5", &temp)) {
		hwlog_err("%s:read fw_addr_5 not config\n", __func__);
		thp_data.fw_addr_5 = 0;
	} else {
		thp_data.fw_addr_5 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_5 = %d\n", __func__,
			thp_data.fw_addr_5);
	}

	if (of_property_read_u32(dn, "fw_addr_6", &temp)) {
		hwlog_err("%s:read fw_addr_6 not config\n", __func__);
		thp_data.fw_addr_6 = 0;
	} else {
		thp_data.fw_addr_6 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_6 = %d\n", __func__,
			thp_data.fw_addr_6);
	}

	if (of_property_read_u32(dn, "fw_addr_7", &temp)) {
		hwlog_err("%s:read fw_addr_7 not config\n", __func__);
		thp_data.fw_addr_7 = 0;
	} else {
		thp_data.fw_addr_7 = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_7 = %d\n", __func__,
			thp_data.fw_addr_7);
	}
	if (of_property_read_u32(dn, "tp_sensorhub_irq_flag", &temp)) {
		hwlog_err("%s:read tp_shb_irq_flag not config\n", __func__);
		thp_data.tp_sensorhub_irq_flag = 0;
	} else {
		thp_data.tp_sensorhub_irq_flag = (GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read fw_addr_7 = %d\n", __func__,
			thp_data.tp_sensorhub_irq_flag);
	}
	if (of_property_read_u32(dn,
		"tp_sensor_spi_sync_cs_low_delay_us", &temp)) {
		hwlog_err("%s:tp_sensor_spi_sync_cs_low_delay_us not config\n",
			__func__);
		thp_data.tp_sensor_spi_sync_cs_low_delay_us = 0;
	} else {
		thp_data.tp_sensor_spi_sync_cs_low_delay_us =
			(GPIO_NUM_TYPE)temp;
		hwlog_err("%s:read tp_sensor_spi_sync_cs_low_delay_us = %u\n",
			__func__, thp_data.tp_sensor_spi_sync_cs_low_delay_us);
	}
	hwlog_info("%s succ\n", __func__);
}

int detect_disable_sample_task_prop(struct device_node *dn, uint32_t *value)
{
	int ret;

	ret = of_property_read_u32(dn, "disable_sample_task", value);
	if (ret)
		return -1;

	return 0;
}

int get_combo_bus_tag(const char *bus, uint8_t *tag)
{
	enum obj_tag tag_tmp = TAG_END;

	if (!strcmp(bus, "i2c"))
		tag_tmp = TAG_I2C;
	else if (!strcmp(bus, "spi"))
		tag_tmp = TAG_SPI;
	else if (!strcmp(bus, "i3c"))
		tag_tmp = TAG_I3C;

	if (tag_tmp == TAG_END)
		return -1;
	*tag = (uint8_t)tag_tmp;
	return 0;
}

static int get_combo_prop(struct device_node *dn, struct detect_word *p_det_wd)
{
	int ret;
	struct property *prop = NULL;
	const char *bus_type = NULL;
	uint32_t u32_temp = 0;

	/* combo_bus_type */
	ret = of_property_read_string(dn, "combo_bus_type", &bus_type);
	if (ret) {
		hwlog_err("%s: get bus_type err\n", __func__);
		return ret;
	}
	if (get_combo_bus_tag(bus_type, &p_det_wd->cfg.bus_type)) {
		hwlog_err("%s: bus_type(%s) err\n", __func__, bus_type);
		return -1;
	}

	/* combo_bus_num */
	ret = of_property_read_u32(dn, "combo_bus_num", &u32_temp);
	if (ret) {
		hwlog_err("%s: get combo_data err\n", __func__);
		return ret;
	}
	p_det_wd->cfg.bus_num = (uint8_t)u32_temp;

	/* combo_data */
	ret = of_property_read_u32(dn, "combo_data", &u32_temp);
	if (ret) {
		hwlog_err("%s: get combo_data err\n", __func__);
		return ret;
	}
	p_det_wd->cfg.data = u32_temp;

	/* combo_tx */
	prop = of_find_property(dn, "combo_tx", NULL);
	if (!prop) {
		hwlog_err("%s: get combo_tx err\n", __func__);
		return -1;
	}
	p_det_wd->tx_len = (uint32_t)prop->length;
	if (p_det_wd->tx_len > sizeof(p_det_wd->tx)) {
		hwlog_err("%s: get combo_tx_len %d too big\n",
			__func__, p_det_wd->tx_len);
		return -1;
	}
	of_property_read_u8_array(dn, "combo_tx", p_det_wd->tx,
		(size_t)prop->length);

	/* combo_rx_mask */
	prop = of_find_property(dn, "combo_rx_mask", NULL);
	if (!prop) {
		hwlog_err("%s: get combo_rx_mask err\n", __func__);
		return -1;
	}
	p_det_wd->rx_len = (uint32_t)prop->length;
	if (p_det_wd->rx_len > sizeof(p_det_wd->rx_msk)) {
		hwlog_err("%s: get rx_len %d too big\n", __func__,
			p_det_wd->rx_len);
		return -1;
	}
	of_property_read_u8_array(dn, "combo_rx_mask", p_det_wd->rx_msk,
		(size_t)prop->length);

	/* combo_rx_exp */
	prop = of_find_property(dn, "combo_rx_exp", NULL);
	if (!prop) {
		hwlog_err("%s: get combo_rx_exp err\n", __func__);
		return -1;
	}
	prop->length = (uint32_t)prop->length;
	if ((ssize_t)prop->length > sizeof(p_det_wd->rx_exp) ||
	    ((uint32_t)prop->length % p_det_wd->rx_len)) {
		hwlog_err("%s: rx_exp_len %d not available\n",
			__func__, prop->length);
		return -1;
	}
	p_det_wd->exp_n = (uint32_t)prop->length / p_det_wd->rx_len;
	of_property_read_u8_array(dn, "combo_rx_exp", p_det_wd->rx_exp,
		(size_t)prop->length);

	return 0;
}

static int detect_i2c_device(struct device_node *dn, char *device_name)
{
	int i;
	int ret;
	int i2c_address = 0;
	int i2c_bus_num = 0;
	int register_add = 0;
	int len;
	u32 wia[10] = { 0 };
	u32 wia_mask = 0xffffffff;
	u32 temp = 0;
	uint8_t detected_device_id[4] = {0};
	uint8_t tag = TAG_I2C;
	uint32_t register_add_len;
	uint32_t rx_len;
	uint32_t device_id;
	struct property *prop = NULL;
	const char *bus_type = NULL;

	if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
		of_property_read_u32(dn, "reg", &i2c_address) ||
		of_property_read_u32(dn, "chip_id_register", &register_add)) {
		hwlog_err("%s:read i2c bus_number or bus address or chip_id_register from dts fail\n",
			device_name);
		return -1;
	}

	prop = of_find_property(dn, "chip_id_value", NULL);
	if (!prop)
		return -EINVAL;
	if (!prop->value)
		return -ENODATA;

	len = prop->length / 4; /* 4: to word */

	if (of_property_read_u32_array(dn, "chip_id_value", wia, len)) {
		hwlog_err("%s:read chip_id_value (id0=%x) from dts fail len=%d\n",
			device_name, wia[0], len);
		return -1;
	}

	if (of_property_read_u32(dn, "chip_id_mask", &temp)) {
		hwlog_err("%s:read err chip_id_mask:0x%x\n", __func__, wia_mask);
	} else {
		wia_mask = temp;
		hwlog_info("%s:read succ chip_id_mask:0x%x\n", __func__, wia_mask);
	}

	if (!of_property_read_string(dn, "bus_type", &bus_type))
		get_combo_bus_tag(bus_type, (uint8_t *)&tag);

	if (tag == TAG_I3C) {
		ret = mcu_i3c_rw((uint8_t)i2c_bus_num, (uint8_t)i2c_address,
			(uint8_t *)&register_add, 1, detected_device_id, 1);
	} else {
		if ((unsigned int)register_add & 0xFF00) {
			register_add_len = 2; /* address len 2 */
			rx_len = 4; /* rx len 4 */
		} else {
			register_add_len = 1;
			rx_len = 1;
		}
		ret = mcu_i2c_rw(TAG_I2C, (uint8_t)i2c_bus_num, (uint8_t)i2c_address,
			(uint8_t *)&register_add, register_add_len,
			detected_device_id, rx_len);
	}

	if (ret) {
		hwlog_err("%s:detect_i2c_device:send i2c read cmd to mcu fail,ret=%d\n",
			device_name, ret);
		return -1;
	}
	/* 4 bytes */
	memcpy(&device_id, detected_device_id, 4);
	if (!strncmp(device_name, "vibrator", strlen("vibrator"))) {
		if ((uint8_t)i2c_address == I2C_ADDR_NXP)
			spa_manufacture = NXP;
		else if ((uint8_t)i2c_address == I2C_ADDR_AWINIC)
			spa_manufacture = AWINIC;
		else
			spa_manufacture = CIRRUS;
		hwlog_info("virbator temp i2c detect success,chip_value:0x%x,len:%d,spa_manu %d\n",
			device_id, len, spa_manufacture);
		return 0;
	}
	if (!strncmp(device_name, "thermometer", strlen("thermometer")))
		device_id = (device_id & 0xff00) >> 8;

	for (i = 0; i < len; i++) {
		if (device_id == wia[i]) {
			hwlog_info("%s:i2c detect suc!chip_value:0x%x\n",
				device_name, device_id);
			return 0;
		} else if (temp != 0) {
			if ((device_id & wia_mask) == wia[i]) {
				hwlog_info("%s:i2c detect suc!chip_value_with_mask:0x%x\n",
					device_name, device_id & wia_mask);
				return 0;
			}
		}
	}
	hwlog_info("%s:i2c detect fail,chip_value:0x%x,len:%d\n",
		device_name, device_id, len);
	return -1;
}

static int device_detect_i2c(struct device_node *dn, int index,
	struct detect_word *p_det_wd)
{
	int ret;
	uint32_t i2c_bus_num = 0;
	uint32_t i2c_address = 0;
	uint32_t register_add = 0;

	ret = detect_i2c_device(dn, sensor_manager[index].sensor_name_str);
	if (!ret) {
		if (of_property_read_u32(dn, "bus_number", &i2c_bus_num) ||
			of_property_read_u32(dn, "reg", &i2c_address) ||
			of_property_read_u32(dn, "chip_id_register", &register_add)) {
			hwlog_err("%s:read i2c bus_number or bus address or chip_id_register from dts fail\n",
				sensor_manager[index].sensor_name_str);
			return -1;
		}
		p_det_wd->cfg.bus_type = TAG_I2C;
		p_det_wd->cfg.bus_num = (uint8_t)i2c_bus_num;
		p_det_wd->cfg.i2c_address = (uint8_t)i2c_address;
	}

	return ret;
}

int _device_detect(struct device_node *dn, int index,
	struct sensor_combo_cfg *p_succ_ret)
{
	int ret;
	struct detect_word det_wd = { { 0 }, 0 };
	struct property *prop = of_find_property(dn, "combo_bus_type", NULL);
	uint8_t r_buf[MAX_TX_RX_LEN] = { 0 };
	uint32_t i, n;
	int rx_exp_p;

	if (prop) {
		ret = get_combo_prop(dn, &det_wd);
		if (ret) {
			hwlog_err("%s:get_combo_prop fail\n", __func__);
			return ret;
		}

		hwlog_info("%s: combo detect bus type %d; num %d; data %d; txlen %d; tx[0] 0x%x; rxLen %d; rxmsk[0] 0x%x; n %d; rxexp[0] 0x%x",
			__func__, det_wd.cfg.bus_type, det_wd.cfg.bus_num,
			det_wd.cfg.data, det_wd.tx_len, det_wd.tx[0],
			det_wd.rx_len, det_wd.rx_msk[0], det_wd.exp_n,
			det_wd.rx_exp[0]);

		ret = combo_bus_trans(&det_wd.cfg, det_wd.tx,
			det_wd.tx_len, r_buf, det_wd.rx_len);
		hwlog_info("combo_bus_trans ret is %d; rx 0x%x;\n", ret, r_buf[0]);
		if (ret < 0)
			return ret;

		ret = -1; /* fail first */
		/* check expect value */
		for (n = 0; n < det_wd.exp_n; n++) {
			for (i = 0; i < det_wd.rx_len;) {
				rx_exp_p = n * det_wd.rx_len + i;
				/* check value */
				if ((r_buf[i] & det_wd.rx_msk[i]) !=
				     det_wd.rx_exp[rx_exp_p]) {
					break;
				}
				i++;
			}
			/* get the success device */
			if (i == det_wd.rx_len) {
				ret = 0;
				hwlog_info("%s: %s detect succ;\n", __func__,
					sensor_manager[index].sensor_name_str);
				break;
			}
		}
	} else {
		hwlog_info("%s: [%s] donot find combo prop\n", __func__,
			sensor_manager[index].sensor_name_str);
		ret = device_detect_i2c(dn, index, &det_wd);
	}

	if (!ret)
		*p_succ_ret = det_wd.cfg;

	return ret;
}

static int device_detect_ex(struct device_node *dn, int index)
{
	int ret = 0;
	struct sensor_combo_cfg cfg = { 0 };

	if (sensor_manager[index].sensor_id == HANDPRESS) {
		return handpress_sensor_detect(dn, sensor_manager, index);
	} else if (sensor_manager[index].sensor_id == FINGERPRINT) {
		return inputhub_fingerprint_sensor_detect(dn, sensor_manager,
			index);
	} else if (sensor_manager[index].sensor_id == FINGERPRINT_UD) {
		return inputhub_fingerprint_ud_sensor_detect(dn,
			sensor_manager, index);
	} else if (sensor_manager[index].sensor_id == CAP_PROX) {
		return cap_prox_sensor_detect(dn, sensor_manager, index);
	} else if (sensor_manager[index].sensor_id == KEY) {
		return inputhub_key_sensor_detect(dn, sensor_manager, index);
	} else if (sensor_manager[index].sensor_id == MOTION) {
		hwlog_info("%s:motion detect always ok\n", __func__);
		return ret;
	} else if (sensor_manager[index].sensor_id == PS) {
		return ps_sensor_detect(dn, sensor_manager, index);
	} else if (sensor_manager[index].sensor_id == TP_UD) {
		ret = tp_ud_sensor_detect(dn, TP_UD);
		if (ret)
			hwlog_err("tp ud sensor detect fail\n");
		return ret;
	} else if (sensor_manager[index].sensor_id == TP_UD_EXTRA) {
		ret = tp_ud_sensor_detect(dn, TP_UD_EXTRA);
		if (ret)
			hwlog_err("tp ud sensor detect fail\n");
		return ret;
	} else if (sensor_manager[index].sensor_id == THP) {
		ret = thp_sensor_detect(dn);
		if (ret)
			hwlog_err("thp sensor detect fail\n");
		return ret;
	} else if (sensor_manager[index].sensor_id == VIBRATOR) {
		ret = vibrator_sensor_detect(dn, sensor_manager, index);
		if (ret)
			hwlog_err("vibrator_sensor_detect fail\n");
		return ret;
	} else if (sensor_manager[index].sensor_id == AIRPRESS) {
		return airpress_sensor_detect(dn, sensor_manager, index);
	}
	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		if (memcpy_s((void *)sensor_manager[index].spara,
			sizeof(struct sensor_combo_cfg), (void *)&cfg,
			sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}
	return ret;
}

static int device_detect(struct device_node *dn, int index)
{
	int ret;
	struct sensor_combo_cfg *p_cfg = NULL;
	uint32_t disable = 0;

	if (sensor_manager[index].detect_result == DET_SUCC)
		return -1;

	ret = device_detect_ex(dn, index);
	if (ret) {
		sensor_manager[index].detect_result = DET_FAIL;
		return DET_FAIL;
	}

	/* check disable sensor task */
	p_cfg = (struct sensor_combo_cfg *)sensor_manager[index].spara;

	ret = detect_disable_sample_task_prop(dn, &disable);
	if (!ret)
		/* get disbale_sample_task property value */
		p_cfg->disable_sample_thread = (uint8_t)disable;

	sensor_manager[index].detect_result = DET_SUCC;
	return DET_SUCC;
}

static int get_sensor_index(const char *name_buf, int len)
{
	int i;

	for (i = 0; i < SENSOR_MAX; i++) {
		if (len != strlen(sensor_manager[i].sensor_name_str))
			continue;
		if (!strncmp(name_buf, sensor_manager[i].sensor_name_str, len))
			break;
	}
	if (i < SENSOR_MAX)
		return i;

	hwlog_err("get_sensor_detect_index fail\n");
	return -1;
}

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void __set_hw_dev_flag(enum sensor_detect_list s_id)
{
/* detect current device successful, set the flag as present */
	switch (s_id) {
	case ACC:
	case ACC1:
		set_hw_dev_flag(DEV_I2C_G_SENSOR);
		break;
	case MAG:
	case MAG1:
		set_hw_dev_flag(DEV_I2C_COMPASS);
		break;
	case GYRO:
	case GYRO1:
		set_hw_dev_flag(DEV_I2C_GYROSCOPE);
		break;
	case ALS:
	case ALS1:
	case ALS2:
	case PS:
		set_hw_dev_flag(DEV_I2C_L_SENSOR);
		break;
	case AIRPRESS:
		set_hw_dev_flag(DEV_I2C_AIRPRESS);
		break;
	case HANDPRESS:
		set_hw_dev_flag(DEV_I2C_HANDPRESS);
		break;
	case VIBRATOR:
		set_hw_dev_flag(DEV_I2C_VIBRATOR_LRA);
		break;
	case CAP_PROX:
	case CAP_PROX1:
	case FINGERPRINT:
	case KEY:
	case MOTION:
	case MAGN_BRACKET:
	case CONNECTIVITY:
	case TP_UD:
	default:
		hwlog_err("__set_hw_dev_flag:err id =%d\n", s_id);
		break;
	}
}
#endif

static int extend_config_before_sensor_detect(struct device_node *dn, int index)
{
	int ret = 0;
	enum sensor_detect_list s_id = sensor_manager[index].sensor_id;

	switch (s_id) {
	case CONNECTIVITY:
		sensor_manager[index].detect_result = DET_SUCC;
		read_connectivity_data_from_dts(dn);
		break;
	case MAGN_BRACKET:
		sensor_manager[index].detect_result = DET_SUCC;
		read_magn_bracket_data_from_dts(dn);
		break;
	case RPC:
		sensor_manager[index].detect_result = DET_SUCC;
		read_rpc_data_from_dts(dn);
		break;
	case SH_AOD:
		sensor_manager[index].detect_result = DET_SUCC;
		read_aod_data_from_dts(dn);
		break;
	case KB:
		sensor_manager[index].detect_result = DET_SUCC;
		read_kb_data_from_dts(dn);
		break;
	case IOMCU_MODEM:
		sensor_manager[index].detect_result = DET_SUCC;
		read_modem_data_from_dts(dn);
		break;
	case SOUND:
		sensor_manager[index].detect_result = DET_SUCC;
		read_sound_data_from_dts(dn);
		break;
	default:
		ret = -1;
		break;
	}
	return ret;
}

static void extend_config_after_sensor_detect(struct device_node *dn, int index)
{
	enum sensor_detect_list s_id = sensor_manager[index].sensor_id;

	switch (s_id) {
	case ACC:
	case ACC1:
		read_acc_data_from_dts(dn, &sensor_manager[index]);
		break;
	case MAG:
	case MAG1:
		read_mag_data_from_dts(dn, &sensor_manager[index]);
		break;
	case GYRO:
	case GYRO1:
		read_gyro_data_from_dts(dn, &sensor_manager[index]);
		break;
	case ALS:
	case ALS1:
	case ALS2:
		read_als_data_from_dts(dn, &sensor_manager[index]);
		break;
	case PS:
		read_ps_data_from_dts(dn);
		break;
	case AIRPRESS:
		read_airpress_data_from_dts(dn);
		break;
	case HANDPRESS:
		read_handpress_data_from_dts(dn, &sensor_manager[index]);
		break;
	case CAP_PROX:
	case CAP_PROX1:
		read_capprox_data_from_dts(dn, &sensor_manager[index]);
		break;
	case KEY:
		read_key_i2c_data_from_dts(dn);
		break;
	case FINGERPRINT:
		read_fingerprint_from_dts(dn);
		break;
	case VIBRATOR:
		read_vibrator_from_dts(dn, &sensor_manager[index]);
		break;
	case FINGERPRINT_UD:
		read_fingerprint_ud_from_dts(dn);
		break;
	case MOTION:
		read_motion_data_from_dts(dn, &sensor_manager[index]);
		break;
	case TOF:
		read_tof_data_from_dts(dn);
		break;
	case TP_UD:
		read_tp_ud_from_dts(dn, TP_UD);
		break;
	case THP:
		read_thp_from_dts(dn);
		break;
	case THERMOMETER:
		read_thermometer_data_from_dts(dn);
		break;
	case TP_UD_EXTRA:
		read_tp_ud_from_dts(dn, TP_UD_EXTRA);
		break;
	default:
		hwlog_err("%s:err id =%d\n", __func__, s_id);
		break;
	}
}

#ifdef CONFIG_HUAWEI_DSM
static void update_detectic_client_info(void)
{
	char sensor_name[DSM_MAX_IC_NAME_LEN] = { 0 };
	uint8_t i;
	int total_len = 0;
	struct dsm_dev *dsm = get_dsm_sensorhub();

	for (i = 0; i < SENSOR_MAX; i++) {
		if (sensor_manager[i].detect_result == DET_FAIL) {
			total_len += strlen(sensor_manager[i].sensor_name_str);
			if (total_len < DSM_MAX_IC_NAME_LEN)
				strcat(sensor_name,
					sensor_manager[i].sensor_name_str);
		}
	}
	sensor_name[DSM_MAX_IC_NAME_LEN - 1] = '\0';
	hwlog_debug("update_detectic_client_info %s.\n", sensor_name);
	dsm->ic_name = sensor_name;
	if (dsm_update_client_vendor_info(dsm))
		hwlog_info("dsm_update_client_vendor_info failed\n");
}

static void boot_detect_fail_dsm(char *title, char *detect_result,
	uint32_t size)
{
	int32_t update = 0;
	struct dsm_client *client = inputhub_get_shb_dclient();

	if (!dsm_client_ocuppy(client)) {
		update = 1;
	} else {
		hwlog_info("%s:dsm_client_ocuppy fail\n", __func__);
		dsm_client_unocuppy(client);
		if (!dsm_client_ocuppy(client))
			update = 1;
	}
	if (update == 1) {
		update_detectic_client_info();
		dsm_client_record(client, "[%s]%s", title, detect_result);
		dsm_client_notify(client, DSM_SHB_ERR_IOM7_DETECT_FAIL);
	}
}
#endif

static uint8_t check_detect_result(enum detect_mode mode)
{
	int i;
	uint8_t detect_fail_num = 0;
	uint8_t result;
	int total_len = 0;
	char detect_result[MAX_STR_SIZE] = { 0 };
	const char *sf = " detect fail!";

	for (i = 0; i < SENSOR_MAX; i++) {
		result = sensor_manager[i].detect_result;
		if (result == DET_FAIL) {
			detect_fail_num++;
			total_len += strlen(sensor_manager[i].sensor_name_str);
			total_len += 2; /* 2 bytes */
			if (total_len < MAX_STR_SIZE) {
				strcat(detect_result,
					sensor_manager[i].sensor_name_str);
				strcat(detect_result, "  ");
			}
			hwlog_info("%s: %s detect fail\n", __func__,
				sensor_manager[i].sensor_name_str);
		} else if (result == DET_SUCC) {
			hwlog_info("%s: %s detect success\n", __func__,
				sensor_manager[i].sensor_name_str);
			if (i == GYRO)
				gyro_detect_flag = 1;
		}
	}

	if (detect_fail_num > 0) {
		s_redetect_state.need_redetect_sensor = 1;
		total_len += strlen(sf);
		if (total_len < MAX_STR_SIZE)
			strcat(detect_result, sf);

#ifdef CONFIG_HUAWEI_DSM
		if (mode == BOOT_DETECT_END)
			boot_detect_fail_dsm((char *)__func__, detect_result,
				MAX_STR_SIZE);
#endif
	} else {
		s_redetect_state.need_redetect_sensor = 0;
	}

	if ((detect_fail_num < s_redetect_state.detect_fail_num) &&
		(mode == REDETECT_LATER)) {
		s_redetect_state.need_recovery = 1;
		hwlog_info("%s : %u sensors detect success after redetect\n",
			__func__, s_redetect_state.detect_fail_num - detect_fail_num);
	}
	s_redetect_state.detect_fail_num = detect_fail_num;
	return detect_fail_num;
}

static void show_last_detect_fail_sensor(void)
{
	int i;
	uint8_t result;

	for (i = 0; i < SENSOR_MAX; i++) {
		result = sensor_manager[i].detect_result;
		if (result == DET_FAIL)
			hwlog_err("last detect fail sensor: %s\n",
				sensor_manager[i].sensor_name_str);
	}
}

static void redetect_failed_sensors(enum detect_mode mode)
{
	int index;
	char *sensor_ty = NULL;
	char *sensor_st = NULL;
	struct device_node *dn = NULL;
	const char *st = "disabled";

	for_each_node_with_property(dn, "sensor_type") {
		/* sensor type */
		if (of_property_read_string(dn, "sensor_type",
		    (const char **)&sensor_ty)) {
			hwlog_err("redetect get sensor type fail\n");
			continue;
		}
		index = get_sensor_index(sensor_ty, strlen(sensor_ty));
		if (index < 0) {
			hwlog_err("redetect get sensor index fail\n");
			continue;
		}
		/* sensor status:ok or disabled */
		if (of_property_read_string(dn, "status",
		    (const char **)&sensor_st)) {
			hwlog_err("redetect get sensor status fail\n");
			continue;
		}
		if (!strcmp(st, sensor_st)) {
			hwlog_info("%s : sensor %s status is %s\n",
				__func__, sensor_ty, sensor_st);
			continue;
		}
		if (device_detect(dn, index) != DET_SUCC)
			continue;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
		__set_hw_dev_flag(sensor_manager[index].sensor_id);
#endif

		extend_config_after_sensor_detect(dn, index);
	}
	check_detect_result(mode);
}

static void sensor_detect_exception_process(uint8_t result)
{
	int i;

	if (result > 0) {
		for (i = 0; i < SENSOR_DETECT_RETRY; i++) {
			hwlog_info("%s :  %d redect sensor after detect all sensor, fail sensor num  %d\n",
				__func__, i, s_redetect_state.detect_fail_num);
			if (s_redetect_state.detect_fail_num > 0)
				redetect_failed_sensors(DETECT_RETRY+i);
		}
	}
}

static int get_phone_type_send(void)
{
	u32 temp[PHONE_TYPE_LIST] = { 0 };
	struct device_node *sensorhub_node = NULL;

	sensorhub_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return -1;
	}

	if (of_property_read_u32_array(
		sensorhub_node, "phone_type_info", temp, PHONE_TYPE_LIST) != 0) {
		hwlog_err("read phone type info from dts fail\n");
		return -1;
	}

	g_config_on_ddr->phone_type_info[0] = temp[0];
	g_config_on_ddr->phone_type_info[1] = temp[1];
	hwlog_info("%s: phone type is %d %d\n",
		__func__, g_config_on_ddr->phone_type_info[0],
		g_config_on_ddr->phone_type_info[1]);

	return 0;
}

static void init_sensors_cfg_data_each_node(void)
{
	int ret;
	int index;
	char *sensor_ty = NULL;
	char *sensor_st = NULL;
	struct device_node *dn = NULL;
	const char *st = "disabled";

	for_each_node_with_property(dn, "sensor_type") {
		/* sensor type */
		ret = of_property_read_string(dn, "sensor_type",
			(const char **)&sensor_ty);
		if (ret) {
			hwlog_err("get sensor type fail ret=%d\n", ret);
			continue;
		}
		hwlog_info("%s : get sensor type %s\n", __func__, sensor_ty);
		index = get_sensor_index(sensor_ty, strlen(sensor_ty));
		if (index < 0) {
			hwlog_err("get sensor index fail ret=%d\n", ret);
			continue;
		}
		if (sensor_manager[index].sensor_id == CAP_PROX)
			read_cap_prox_info(dn); /* for factory sar */

		if (sensor_manager[index].sensor_id == CAP_PROX1)
			read_cap_prox1_info(dn); /* for factory sar */

		/* sensor status:ok or disabled */
		ret = of_property_read_string(dn, "status",
			(const char **)&sensor_st);
		if (ret) {
			hwlog_err("get sensor status fail ret=%d\n", ret);
			continue;
		}

		ret = strcmp(st, sensor_st);
		if (!ret) {
			hwlog_info("%s : sensor %s status is %s\n", __func__,
				sensor_ty, sensor_st);
			continue;
		}
		if (!extend_config_before_sensor_detect(dn, index))
			continue;

		ret = device_detect(dn, index);
		if (ret != DET_SUCC)
			continue;

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
		__set_hw_dev_flag(sensor_manager[index].sensor_id);
#endif

		extend_config_after_sensor_detect(dn, index);
	}
}

int init_sensors_cfg_data_from_dts(void)
{
	int i;
	uint8_t sensor_detect_result;

	get_phone_type_send(); // get phone type info

	memset(&sensorlist_info, 0, SENSOR_MAX * sizeof(struct sensorlist_info));
	/* init sensorlist_info struct array */
	for (i = 0; i < SENSOR_MAX; i++) {
		sensorlist_info[i].version = -1;
		sensorlist_info[i].max_range = -1;
		sensorlist_info[i].resolution = -1;
		sensorlist_info[i].power = -1;
		sensorlist_info[i].min_delay = -1;
		sensorlist_info[i].max_delay = -1;
		sensorlist_info[i].fifo_reserved_event_count = 0xFFFFFFFF;
		sensorlist_info[i].fifo_max_event_count = 0xFFFFFFFF;
		sensorlist_info[i].flags = 0xFFFFFFFF;
	}

	init_sensors_cfg_data_each_node();

	if (sensor_manager[FINGERPRINT].detect_result != DET_SUCC) {
		hwlog_warn("fingerprint detect fail, bypass\n");
		sensor_manager[FINGERPRINT].detect_result = DET_SUCC;
	}
	if (sensor_manager[FINGERPRINT_UD].detect_result != DET_SUCC) {
		hwlog_warn("fingerprint_ud detect fail, bypass\n");
		sensor_manager[FINGERPRINT_UD].detect_result = DET_SUCC;
	}

	sensor_detect_result = check_detect_result(BOOT_DETECT);
	sensor_detect_exception_process(sensor_detect_result);

	get_sensors_id_from_dts();
	get_power_charging_status_from_dts();
	motion_get_fall_down_support_from_dts();
	if (get_adapt_id_and_send())
		return -EINVAL;

	return 0;
}

void send_parameter_to_mcu(enum sensor_detect_list s_id, int cmd)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	char buf[50] = { 0 }; /* buf size 50 */

	pkg_ap.tag = sensor_manager[s_id].tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = cmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sensor_manager[s_id].cfg_data_length + SUBCMD_LEN;
	memcpy(cpkt.para, sensor_manager[s_id].spara,
		sensor_manager[s_id].cfg_data_length);

	hwlog_info("%s g_iom3_state = %d,tag =%d ,cmd =%d\n",
		__func__, get_iom3_state(), sensor_manager[s_id].tag, cmd);

	if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
		(get_iomcu_power_state() == ST_SLEEP))
		ret = write_customize_cmd(&pkg_ap, NULL, false);
	else
		ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);

	if (ret) {
		hwlog_err("send tag %d cfg data to mcu fail,ret=%d\n",
			pkg_ap.tag, ret);
	} else {
		if (pkg_mcu.errno != 0) {
			/* buf size 50 */
			snprintf(buf, 50, "set %s cfg fail\n",
				sensor_manager[s_id].sensor_name_str);
			hwlog_err("%s\n", buf);
		} else {
			/* buf size 50 */
			snprintf(buf, 50, "set %s cfg to mcu success\n",
				sensor_manager[s_id].sensor_name_str);
			hwlog_info("%s\n", buf);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
			if (get_iom3_state() != IOM3_ST_RECOVERY)
				__set_hw_dev_flag(s_id);
#endif
		}
	}
}

static int tof_data_from_mcu(const struct pkt_header *head)
{
	switch (((pkt_tof_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SELFCALI_REQ:
		return write_tof_offset_to_nv(
			((pkt_tof_calibrate_data_req_t *)head)->calibrate_data,
			TOF_CALIDATA_NV_SIZE);
	default:
		hwlog_err("uncorrect subcmd 0x%x\n",
			((pkt_tof_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}


static void register_priv_notifier(enum sensor_detect_list s_id)
{
	switch (s_id) {
	case GYRO:
		register_mcu_event_notifier(TAG_GYRO, CMD_CMN_CONFIG_REQ,
			gyro_data_from_mcu);
		break;
	case MAG:
		register_mcu_event_notifier(TAG_MAG, CMD_CMN_CONFIG_REQ,
			mag_data_from_mcu);
		break;
	case TOF:
		register_mcu_event_notifier(TAG_TOF, CMD_CMN_CONFIG_REQ,
			tof_data_from_mcu);
		break;
	case ALS:
		register_mcu_event_notifier(TAG_ALS, CMD_CMN_CONFIG_REQ,
			als_data_from_mcu);
		break;
	case ALS1:
		register_mcu_event_notifier(TAG_ALS1, CMD_CMN_CONFIG_REQ,
			als_data_from_mcu);
		break;
	case ALS2:
		register_mcu_event_notifier(TAG_ALS2, CMD_CMN_CONFIG_REQ,
			als_data_from_mcu);
		break;
	case PS:
		register_mcu_event_notifier(TAG_PS, CMD_CMN_CONFIG_REQ,
			ps_data_from_mcu);
		break;
	case GYRO1:
		register_mcu_event_notifier(TAG_GYRO1, CMD_CMN_CONFIG_REQ,
			gyro1_data_from_mcu);
		break;
	case MAG1:
		register_mcu_event_notifier(TAG_MAG1, CMD_CMN_CONFIG_REQ,
			mag1_data_from_mcu);
		break;
	case VIBRATOR:
		register_mcu_event_notifier(TAG_VIBRATOR, CMD_CMN_CONFIG_REQ,
			vibrator_data_from_mcu);
		break;
	case THERMOMETER:
		register_mcu_event_notifier(TAG_THERMOMETER, CMD_CMN_CONFIG_REQ,
			thermometer_data_from_mcu);
		break;
	default:
		break;
	}
}

static void send_modem_mode_to_iomcu(uint32_t mode)
{
	struct write_info pkg_ap;
	int ret;

	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_MODEM_MODE_REQ;
	pkg_ap.wr_buf = &mode;
	pkg_ap.wr_len = sizeof(mode);

	ret = write_customize_cmd(&pkg_ap, NULL, false);
	hwlog_info("%s set mode %d, ret 0x%x\n", __func__, mode, ret);
}

int sensor_set_cfg_data(void)
{
	int32_t al_tag = 0;
	int ret = 0;
	enum sensor_detect_list s_id;

	for (s_id = ACC; s_id < SENSOR_MAX; s_id++) {
		if (s_id == PS)
			ps_set_config_to_mcu(TAG_PS);
		if (strlen(get_sensor_chip_info_address(s_id)) != 0) {
			if (s_id == IOMCU_MODEM) {
				hwlog_info("%s send modem mode\n", __func__);
				send_modem_mode_to_iomcu(g_modem_data.mode);
				continue;
			}
#ifdef CONFIG_CONTEXTHUB_SHMEM
			if (s_id != RPC) {
#endif
				send_parameter_to_mcu(s_id,
					SUB_CMD_SET_PARAMET_REQ);
				if (!als_get_tag_by_sensor_id(s_id, &al_tag)) {
					struct als_device_info *info = NULL;

					info = als_get_device_info(al_tag);
					if (info != NULL)
						info->send_para_flag = 1;
				}

#ifdef CONFIG_HUAWEI_SHB_THP
				if (s_id == THP) {
					hwlog_info("%s send thp open cmd\n", __func__);
					(void)send_thp_open_cmd();
				}
#endif
				if (get_iom3_state() != IOM3_ST_RECOVERY)
					register_priv_notifier(s_id);
#ifdef CONFIG_CONTEXTHUB_SHMEM
				if (s_id == MAG) {
					ret = mag_set_far_softiron(TAG_MAG);
					if (ret)
						hwlog_err("%s shmem_send mag error\n", __func__);
				}
			} else {
				ret = shmem_send(TAG_RPC, &rpc_data,
					sizeof(rpc_data));
				if (ret)
					hwlog_err("%s shmem_send error\n", __func__);
			}
#endif
		}
	}
	return ret;
}

static bool need_download_fw(uint8_t tag)
{
	return ((tag == TAG_KEY) || (tag == TAG_TOF) || (tag == TAG_CAP_PROX) || (tag == TAG_CAP_PROX1));
}

int sensor_set_fw_load(void)
{
	int val = 1;
	int ret;
	struct write_info pkg_ap;
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	enum sensor_detect_list s_id;

	hwlog_info("write  fw dload\n");
	for (s_id = ACC; s_id < SENSOR_MAX; s_id++) {
		if (strlen(get_sensor_chip_info_address(s_id)) != 0) {
			if (need_download_fw(sensor_manager[s_id].tag)) {
				pkg_ap.tag = sensor_manager[s_id].tag;
				pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
				cpkt.subcmd = SUB_CMD_FW_DLOAD_REQ;
				pkg_ap.wr_buf = &hd[1];
				pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
				memcpy(cpkt.para, &val, sizeof(val));
				ret = write_customize_cmd(&pkg_ap, NULL, false);
				hwlog_info("write %d fw dload\n", sensor_manager[s_id].tag);
			}
		}
	}
	return 0;
}

static void redetect_sensor_work_handler(struct work_struct *wk)
{
	__pm_stay_awake(&sensor_rd);
	redetect_failed_sensors(REDETECT_LATER);

	if (s_redetect_state.need_recovery == 1) {
		s_redetect_state.need_recovery = 0;
		hwlog_info("%s: some sensor detect success after %d redetect, begin recovery\n",
			__func__, s_redetect_state.redetect_num);
		iom3_need_recovery(SENSORHUB_USER_MODID, SH_FAULT_REDETECT);
	} else {
		hwlog_info("%s: no sensor redetect success\n", __func__);
	}
	__pm_relax(&sensor_rd);
}

void sensor_redetect_enter(void)
{
	if (get_iom3_state() == IOM3_ST_NORMAL) {
		if (s_redetect_state.need_redetect_sensor == 1) {
			if (s_redetect_state.redetect_num < MAX_REDETECT_NUM) {
				queue_work(system_power_efficient_wq,
					&redetect_work);
				s_redetect_state.redetect_num++;
			} else {
				hwlog_info("%s: some sensors detect fail, but the max redetect num is over flow\n",
					__func__);
				show_last_detect_fail_sensor();
			}
		}
	}
}

void sensor_redetect_init(void)
{
	memset(&s_redetect_state, 0, sizeof(s_redetect_state));
	acc_detect_init(sensor_manager, SENSOR_MAX);
	airpress_detect_init(sensor_manager, SENSOR_MAX);
	als_detect_init(sensor_manager, SENSOR_MAX);
	cap_prox_detect_init(sensor_manager, SENSOR_MAX);
	gyro_detect_init(sensor_manager, SENSOR_MAX);
	handpress_detect_init(sensor_manager, SENSOR_MAX);
	mag_detect_init(sensor_manager, SENSOR_MAX);
	motion_detect_init(sensor_manager, SENSOR_MAX);
	ps_detect_init(sensor_manager, SENSOR_MAX);
	therm_detect_init(sensor_manager, SENSOR_MAX);
	vibrator_detect_init(sensor_manager, SENSOR_MAX);
	wakeup_source_init(&sensor_rd, "sensorhub_redetect");
	INIT_WORK(&redetect_work, redetect_sensor_work_handler);
}

