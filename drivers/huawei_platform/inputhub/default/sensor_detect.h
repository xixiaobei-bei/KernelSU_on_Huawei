/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 * Description: sensor detect header file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#ifndef __SENSOR_DETECT_H
#define __SENSOR_DETECT_H

#include <linux/completion.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/workqueue.h>

#include "protocol.h"

#define AOD_FEATURE_NUM 2

#define AUX_FILE_LIST_ARGS 2
#define MAX_CHIP_INFO_LEN 50
#define MAX_STR_SIZE 1024
#define MAX_PHONE_COLOR_NUM 15
#define CYPRESS_CHIPS 2
#define SENSOR_PLATFORM_EXTEND_DATA_SIZE 50
#define SENSOR_PLATFORM_EXTEND_ALS_DATA_SIZE 68

#define KB_DEFAULT_UART_NUM 8
#define KB_DEFAULT_DETECT_ADC_NUM 7
#define KB_DEFAULT_DISCONNECT_ADC_VOL 1700

#define THP_MAX_PROJECT_ID_LENGTH 11

#define FP_IC_MAX_LEN   20
#define FP_MATCH_LEN    3
#define FP_IC_MATCH_LEN 6
#define SAR_USE_PH_NUM 2

#define DEF_SENSOR_COM_SETTING \
{\
	.bus_type = TAG_I2C,\
	.bus_num = 0,\
	.disable_sample_thread = 0,\
	{ .data = 0 } \
}

#define GPIO_NUM_TYPE uint16_t

enum sensor_detect_list {
	ACC,
	MAG,
	GYRO,
	ALS,
	PS,
	AIRPRESS,
	HANDPRESS,
	CAP_PROX,
	CONNECTIVITY,
	FINGERPRINT,
	KEY,
	MAGN_BRACKET,
	RPC,
	VIBRATOR,
	FINGERPRINT_UD,
	TOF,
	TP_UD,
	SH_AOD,
	ACC1,
	GYRO1,
	ALS1,
	MAG1,
	ALS2,
	CAP_PROX1,
	KB,
	MOTION,
	THP,
	IOMCU_MODEM,
	SOUND,
	THERMOMETER,
	TP_UD_EXTRA,
	SENSOR_MAX
};

#define SENSOR_DETECT_RETRY 2
enum detect_mode {
	BOOT_DETECT,
	DETECT_RETRY,
	BOOT_DETECT_END = DETECT_RETRY + SENSOR_DETECT_RETRY - 1,
	REDETECT_LATER
};

enum ALS_UD_TYPE_T
{
	ALS_UD_NONE,
	ALS_UD_MINUS_TP_RGB,
	ALS_UD_MINUS_DSS_NOISE,
};

struct sleeve_detect_pare {
	unsigned int tp_color;
	unsigned int sleeve_detect_threshhold;
};

struct sensor_combo_cfg {
	uint8_t bus_type;
	uint8_t bus_num;
	uint8_t disable_sample_thread;
	union {
		uint32_t data;
		uint32_t i2c_address;
		union spi_ctrl ctrl;
	};
} __packed;

struct tof_platform_data {
	struct sensor_combo_cfg cfg;
	int tof_calib_zero_threshold;
	int tof_calib_6cm_threshold;
	int tof_calib_10cm_threshold;
	int tof_calib_60cm_threshold;
};

#define STG_SUPPORTED_NUM 3
#define TO_MODEM_SUPPORTED_LEVEL_NUM 8
#define DEFAULT_THRESHOLD 0xC8
#define ADUX_REGS_NEED_INITIATED_NUM 16
#define SEMTECH_REGS_NEED_INITIATED_NUM 12

struct connectivity_platform_data {
	struct sensor_combo_cfg cfg;
	uint16_t poll_interval;
	GPIO_NUM_TYPE gpio1_gps_cmd_ap;
	GPIO_NUM_TYPE gpio1_gps_cmd_sh;
	GPIO_NUM_TYPE gpio2_gps_ready_ap;
	GPIO_NUM_TYPE gpio2_gps_ready_sh;
	GPIO_NUM_TYPE gpio3_wakeup_gps_ap;
	GPIO_NUM_TYPE gpio3_wakeup_gps_sh;
	uint32_t i3c_frequency;
	uint16_t gpio1_gps_cmd_pinmux;
	uint16_t gpio2_gps_ready_pinmux;
	uint16_t gpio3_wakeup_gps_pinmux;
};

struct fingerprint_platform_data {
	struct sensor_combo_cfg cfg;
	uint16_t reg;
	uint16_t chip_id;
	uint16_t product_id;
	GPIO_NUM_TYPE gpio_irq;
	GPIO_NUM_TYPE gpio_irq_sh;
	GPIO_NUM_TYPE gpio_cs;
	GPIO_NUM_TYPE gpio_reset;
	GPIO_NUM_TYPE gpio_reset_sh;
	uint16_t poll_interval;
	uint16_t tp_hover_support;
};

struct tp_ud_algo_config {
	uint16_t move_area_x_min;
	uint16_t move_area_x_max;
	uint16_t move_area_y_min;
	uint16_t move_area_y_max;
	uint16_t finger_area_x_min;
	uint16_t finger_area_x_max;
	uint16_t finger_area_y_min;
	uint16_t finger_area_y_max;
	uint16_t coor_scale;
};

struct tp_ud_platform_data {
	struct sensor_combo_cfg cfg;
	uint16_t reg;
	GPIO_NUM_TYPE gpio_irq;
	GPIO_NUM_TYPE gpio_irq_sh;
	GPIO_NUM_TYPE gpio_cs;
	uint16_t gpio_irq_pull_up_status;
	uint16_t pressure_support;
	uint16_t anti_forgery_support;
	uint32_t ic_type;
	uint32_t hover_enable;
	uint32_t i2c_max_speed_hz;
	uint32_t spi_max_speed_hz;
	uint8_t spi_mode;
	uint16_t fw_power_config_reg;
	uint16_t fw_touch_data_reg;
	uint16_t fw_touch_command_reg;
	uint16_t fw_addr_3;
	uint16_t fw_addr_4;
	uint16_t fw_addr_5;
	uint16_t fw_addr_6;
	uint16_t fw_addr_7;
	uint16_t tp_sensorhub_irq_flag;
	uint16_t tp_sensor_spi_sync_cs_low_delay_us;
	uint16_t soft_reset_support;
	struct tp_ud_algo_config algo_conf;
	uint16_t touch_report_restore_support;
	uint16_t tp_sensorhub_platform;
	uint16_t aod_display_support;
	uint16_t tsa_event_to_udfp;
	uint32_t multi_ic_id;
};

struct tp_multi_panel_info {
	uint32_t support_multi_panel_attach;
	uint32_t panel_index;
};

struct thp_gesture_capability {
	uint8_t support_fp_ud;
	uint8_t support_virtual_key;
	uint8_t support_double_click;
	uint8_t support_hw_m_pen;
};

struct fp_gpio_config {
	GPIO_NUM_TYPE gpio_reset;
	GPIO_NUM_TYPE gpio_cs;
	GPIO_NUM_TYPE gpio_irq;
};

struct fp_sensor_config {
	char ic_code[FP_IC_MAX_LEN];
	uint16_t length;
	void (*func)(struct fp_gpio_config *config);
};

struct thp_platform_data {
	struct sensor_combo_cfg cfg;
	uint16_t reg;
	GPIO_NUM_TYPE gpio_irq;
	GPIO_NUM_TYPE gpio_irq_sh;
	GPIO_NUM_TYPE gpio_cs;
	uint16_t gpio_irq_pull_up_status;
	uint16_t feature_config;
	uint16_t anti_forgery_support;
	uint32_t ic_type;
	uint32_t hover_enable;
	uint32_t i2c_max_speed_hz;
	uint32_t spi_max_speed_hz;
	uint8_t spi_mode;
	uint16_t fw_power_config_reg;
	uint16_t fw_touch_data_reg;
	uint16_t fw_touch_command_reg;
	uint16_t fw_addr_3;
	uint16_t fw_addr_4;
	uint16_t fw_addr_5;
	uint16_t fw_addr_6;
	uint16_t fw_addr_7;
	uint16_t tp_sensorhub_irq_flag;
	uint16_t tp_sensor_spi_sync_cs_low_delay_us;
	struct tp_ud_algo_config algo_conf;
	char project_id[THP_MAX_PROJECT_ID_LENGTH];
	uint8_t shb_thp_log;
	struct thp_gesture_capability gestures;
};

struct key_platform_data {
	struct sensor_combo_cfg cfg;
	uint8_t i2c_address_bootloader;
	GPIO_NUM_TYPE gpio_key_int;
	GPIO_NUM_TYPE gpio_key_int_sh;
	uint16_t poll_interval;
	uint8_t reserve[16];   /* 16 : default array length */
};

#define HUB_LRA_RATED_VOLTAGE 0x34
#define HUB_LRA_OVERDRIVE_CLAMP_VOLTAGE 0x76
#define HUB_REAL_TIME_PLAYBACK_STRENGTH 0x66
#define HUB_MAX_TIMEOUT 10000
#define VIB_FAC_LRALVILTAGE 0x48
#define VIB_RESET_GPIO_DELAY 5

#define SENSOR_COLD_LEVEL_COUNT   3

enum vibrator_chip_type {
	VIBRATOR_CHIP_UNKNOWN = 0,
	VIBRATOR_CHIP_TFA9874 = 0x36,
	VIBRATOR_CHIP_TAS2562 = 0x4E
};

struct vibrator_paltform_data {
	struct sensor_combo_cfg cfg;
	int gpio_enable;
	int gpio_pwm;
	int max_timeout_ms;
	int reduce_timeout_ms;
	int support_amplitude_control;
	char lra_rated_voltage;
	char lra_overdriver_voltage;
	char lra_rtp_strength;
	char skip_lra_autocal;
	int cold_level_count;
	int cold_level[SENSOR_COLD_LEVEL_COUNT];
	int battery_level[SENSOR_COLD_LEVEL_COUNT];
	uint32_t chip_type;
	GPIO_NUM_TYPE gpio_irq;
	GPIO_NUM_TYPE gpio_reset;
	uint8_t rst_value;
	uint8_t reg_addr_bytes;
	uint8_t reg_value_bytes;
	uint8_t ps_disable;
	uint8_t dma_channel;
	uint16_t dma_irq_gpio;
	uint32_t dma_irq_gpio_addr;
};

struct magn_bracket_platform_data {
	struct sensor_combo_cfg cfg;
	int mag_x_change_lower;
	int mag_x_change_upper;
	int mag_y_change_lower;
	int mag_y_change_upper;
	int mag_z_change_lower;
	int mag_z_change_upper;
};

struct motion_platform_data {
	uint8_t motion_horizontal_pickup_flag;
	uint8_t angle_gap;
};

struct aod_platform_data {
	struct sensor_combo_cfg cfg;
	uint32_t feature_set[AOD_FEATURE_NUM];
};

struct modem_platform_data {
	struct sensor_combo_cfg cfg;
	uint32_t mode;
};

struct rpc_platform_data {
	uint16_t table[32];
	uint16_t mask[32];
	uint16_t default_value;
	uint16_t mask_enable;
	uint16_t sar_choice;
	uint16_t sim_type_swtich_flag;
	uint16_t fusion_type;
};

struct kb_platform_data {
	uint16_t uart_num;
	uint16_t kb_detect_adc_num;
	uint16_t kb_disconnect_adc_min;
	uint16_t kb_disable_angle;
};

#define MAX_TX_RX_LEN 32
struct detect_word {
	struct sensor_combo_cfg cfg;
	uint32_t tx_len;
	uint8_t tx[MAX_TX_RX_LEN];
	uint32_t rx_len;
	uint8_t rx_msk[MAX_TX_RX_LEN];
	uint32_t exp_n;
	uint8_t rx_exp[MAX_TX_RX_LEN];
};

#define MAX_SENSOR_NAME_LENGTH 20
struct sensor_detect_manager {
	char sensor_name_str[MAX_SENSOR_NAME_LENGTH];
	enum sensor_detect_list sensor_id;
	uint8_t detect_result;
	int tag;
	const void *spara;
	int cfg_data_length;
};

#define MAX_REDETECT_NUM 100
struct sensor_redetect_state {
	uint8_t need_redetect_sensor;
	uint8_t need_recovery;
	uint8_t detect_fail_num;
	uint8_t redetect_num;
};

struct sensorlist_info {
	/*
	 * Name of this sensor.
	 * All sensors of the same "type" must have a different "name".
	 */
	char name[50];

	/* vendor of the hardware part */
	char vendor[50];
	/*
	 * version of the hardware part + driver. The value of this field
	 * must increase when the driver is updated in a way that changes the
	 * output of this sensor. This is important for fused sensors when the
	 * fusion algorithm is updated.
	 */
	int32_t version;

	/* maximum range of this sensor's value in SI units */
	int32_t max_range;

	/* smallest difference between two values reported by this sensor */
	int32_t resolution;

	/* rough estimate of this sensor's power consumption in mA */
	int32_t power;

	/*
	 * this value depends on the reporting mode:
	 * continuous: minimum sample period allowed in microseconds
	 * on-change : 0
	 * one-shot  :-1
	 * special   : 0, unless otherwise noted
	 */
	int32_t min_delay;

	/*
	 * number of events reserved for this sensor in the batch mode FIFO.
	 * If there is a dedicated FIFO for this sensor, then this is the
	 * size of this FIFO. If the FIFO is shared with other sensors,
	 * this is the size reserved for that sensor and it can be zero.
	 */
	uint32_t fifo_reserved_event_count;

	/*
	 * maximum number of events of this sensor that could be batched.
	 * This is especially relevant when the FIFO is shared between
	 * several sensors; this value is then set to the size of that FIFO.
	 */
	uint32_t fifo_max_event_count;
	/*
	 * This value is defined only for continuous mode and on-change sensors.
	 * It is the delay between two sensor events corresponding to the
	 * lowest frequency that this sensor supports.
	 * When lower frequencies are requested through batch()/setDelay()
	 * the events will be generated at this frequency instead.
	 * It can be used by the framework or applications to estimate
	 * when the batch FIFO may be full.
	 *
	 * @note
	 *   1) period_ns is in nanoseconds
	 *      where as min_delay/minDelay are in microseconds.
	 *         continuous, on-change: maximum sampling period
	 *                                allowed in microseconds.
	 *         one-shot, special : 0
	 *   2) max_delay should always fit within a 32 bit signed integer.
	 *      It is declared as 64 bit
	 *      on 64 bit architectures only for binary compatibility reasons.
	 * Availability: SENSORS_DEVICE_API_VERSION_1_3
	 */
	int32_t max_delay;

	/*
	 * Flags for sensor. See SENSOR_FLAG_* above.
	 * Only the least significant 32 bits are used here.
	 * It is declared as 64 bit on 64 bit architectures
	 * only for binary compatibility reasons.
	 * Availability: SENSORS_DEVICE_API_VERSION_1_3
	 */
	uint32_t flags;
};

void send_parameter_to_mcu(enum sensor_detect_list s_id, int cmd);
void read_aux_file_list(uint16_t fileid, uint16_t tag);
void read_dyn_file_list(uint16_t fileid);
void read_chip_info(struct device_node *dn, enum sensor_detect_list sname);
void read_sensorlist_info(struct device_node *dn, int sensor);
int _device_detect(struct device_node *dn, int index,
	struct sensor_combo_cfg *p_succ_ret);
int get_combo_bus_tag(const char *bus, uint8_t *tag);
int init_sensors_cfg_data_from_dts(void);
enum sensor_detect_list get_id_by_sensor_tag(int tag);
int sensor_set_cfg_data(void);
int send_fileid_to_mcu(void);
void sensor_redetect_enter(void);
void sensor_redetect_init(void);
int sensor_set_fw_load(void);
void resend_vibrator_parameters_to_mcu(void);
int is_power_off_charging_posture(void);
struct sensor_detect_manager *get_sensor_manager(void);
int get_hall_number(void);
int get_hall_sen_type(void);
int get_support_hall_hishow(void);
int get_support_hall_pen(void);
int get_support_hall_keyboard(void);
uint32_t get_hall_hishow_value(void);
uint32_t get_hall_pen_value(void);
uint32_t get_hall_keyboard_value(void);
struct vibrator_paltform_data *get_vibrator_data(void);
int get_sensor_tof_flag(void);
int get_hifi_supported(void);
uint16_t *get_sensorlist(void);
void add_sensor_list_info_id(uint16_t id);
struct sensorlist_info *get_sensorlist_info_by_index(enum sensor_detect_list index);
struct sleeve_detect_pare *get_sleeve_detect_parameter(void);
int get_support_hall_lightstrap(void);
uint32_t get_hall_lightstrap_value(void);
#endif /* __SENSOR_DETECT_H */
