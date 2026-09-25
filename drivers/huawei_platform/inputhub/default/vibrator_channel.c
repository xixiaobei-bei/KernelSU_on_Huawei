/*
 * drivers/inputhub/vibrator_channel.c
 *
 * Vibrator Channel driver
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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/pm_wakeup.h>
#include "contexthub_route.h"
#include "contexthub_boot.h"
#include "protocol.h"
#include <linux/cdev.h>
#include<linux/leds.h>
#include <linux/device.h>
#include "sensor_config.h"
#include "sensor_feima.h"
#include "sensor_sysfs.h"
#include "sensor_detect.h"
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/switch.h>
#include <linux/timer.h>
#include <linux/delay.h>
#ifdef CONFIG_HISI_COUL
#include <linux/power/hisi/coul/hisi_coul_drv.h>
#endif
#include "vibrator_channel.h"
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

static struct wakeup_source vib_wl;

struct drv2605_data {
	struct led_classdev cclassdev;
	int gpio_enable;
	int gpio_pwm;
	int irq_int;
	uint32_t irq_user_bits;
	bool irq_requested;
	bool irq_enabled;
	unsigned int irq_count;
	unsigned int reset_count;
	int max_timeout_ms;
	int reduce_timeout_ms;
	int play_effect_time;
	struct hrtimer timer;
	struct mutex lock;
	struct work_struct work;
	struct work_struct work_play_eff;
	struct work_struct irq_work;
	unsigned int vib_cmd;
	struct class* class;
	struct device* device;
	dev_t version;
	struct cdev cdev;
	struct switch_dev sw_dev;
};

struct vib_cmd_t {
	unsigned int cmd;
	int level;
};

enum vib_abnormity_index {
	VIB_ABNORMITY_INDEX_VDDS = 0,
	VIB_ABNORMITY_INDEX_FREQ,
	VIB_ABNORMITY_INDEX_OTDS,
	VIB_ABNORMITY_INDEX_OCPR,
	VIB_ABNORMITY_INDEX_UVDS,
	VIB_ABNORMITY_INDEX_MANALARM,
	VIB_ABNORMITY_INDEX_TDMER,
	VIB_ABNORMITY_INDEX_NOCLK,
	VIB_ABNORMITY_INDEX_COUNT_MAX
};

#define vib_abnormity_to_bit(x) (0x1 << x)

static struct drv2605_data *data;
static int vib_calib_result = 0;
static uint32_t reg_add;
static uint32_t reg_value;
volatile static bool vibrator_in_irq_work;
extern struct vibrator_paltform_data vibrator_data;
static int vib_time = 0;
#if defined(CONFIG_HISI_VIBRATOR)
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif
static char rtp_strength = 0x7F;
#define VIB_REG_WRITE 0
#define VIB_REG_READ 1
#define DEVICE_NAME "drv2605"
#define DRIVER_VERSION "130"
#define CDEVIE_NAME  "haptics"
#define VIBRA_NAME "vibrator"
#define MODE_STANDBY        0x40
#define MODE_DEVICE_READY   0x00
#define MAX_HAP_BUF_SIZE 100
#define MIN_HAP_BUF_SIZE  2
#define DRV2605_REG_ADDR 0x5A
#define DRV2605_BUS_NUM 0
#define DRV2605_RW_LEN 1
#define MAX_BUF_LEGTH    16
#define MAX_BUF_RD_LEGTH 20
#define MODE_REG 0x01
#define VIB_CA_DIAG_RST 0x08
#define HAPTIC_STOP 0
#define CONVERT_TO_16 16
#define CONVERT_TO_10 10
#define VIB_TEST_CMD_LEN 20
/* sw_dev state */
#define SW_STATE_IDLE				0x00
#define SW_STATE_AUDIO2HAPTIC			0x01
#define SW_STATE_SEQUENCE_PLAYBACK		0x02
#define SW_STATE_RTP_PLAYBACK			0x04
#define YES 1
#define NO  0
#define MAX_TIMEOUT 10000	/* 10s */
#define MIN_REDUCE_TIMEOUT 10   /* 10ms */
#define MAX_REDUCE_TIMEOUT 50   /* 50ms */
#define VIB_OFF 0

#define F0_COEF 8192
#define F0_UPPER_LIMIT 182 /* Hz */
#define F0_LOWER_LIMIT 158 /* Hz */

#define VIB_I2C_BUF_MAX             16

#define VIB_IRQ_USER_BIT_SWITCH     0x1
#define VIB_IRQ_USER_BIT_EFFECT     0x2

#define TFA9874_INT_OUT_REG1        0x40
#define TFA9874_INT_IN_REG1         0x44
#define TFA9874_INT_ENABLE_REG1     0x48
#define TFA9874_STATUS_POL_REG1     0x4C

#define TFA9874_INT_VDDS            0x01
#define TFA9874_INT_OTDS            0x04
#define TFA9874_INT_OCPR            0x08
#define TFA9874_INT_UVDS            0x10
#define TFA9874_INT_MANALARM        0x20
#define TFA9874_INT_TDMER           0x40
#define TFA9874_INT_NOCLK           0x80

#ifdef CONFIG_HUAWEI_DSM
struct vib_dmd_param {
	int32_t error_no;
	char *fmt;
};
#endif

struct reg_operate_param {
	uint8_t addr_bytes;
	uint8_t value_bytes;
	uint8_t is_read;
	uint8_t reserved;
	uint32_t reg_addr;
	uint32_t write_value;
	uint32_t read_value;
};

enum VIB_TEST_TYPE {
	VIB_MMI_CALI = 1,
	VIB_DBC_CALI = 2,
	VIB_HAPTIC_TEST = 3,
};

#ifdef CONFIG_HISI_COUL
#define VIB_COLD_CHECK_SHORT_DELAY    2
#define VIB_COLD_CHECK_LONG_DELAY     5

enum EN_VIB_BAT_COLD_LEVEL {
	EN_VIB_BAT_COLD_LEVEL_0, /* > -5 */
	EN_VIB_BAT_COLD_LEVEL_1, /* <=-5 */
	EN_VIB_BAT_COLD_LEVEL_2, /* <=-10 */
	EN_VIB_BAT_COLD_LEVEL_3, /* <--15 */
	EN_VIB_BAT_COLD_LEVEL_4,
	EN_VIB_BAT_COLD_LEVEL_NUM,
};

enum EN_VIB_BAT_LEVEL {
	EN_VIB_BAT_LEVEL_0, /* normal scene */
	EN_VIB_BAT_LEVEL_1, /* -5, 20%, -10, 30%, -15, 50% */
	EN_VIB_BAT_LEVEL_NUM,
};
#endif
static int vib_cold_level;
static int vib_cold_check_delay;
static time_t vib_cold_check_timestamp;

struct {
	int haptics_type;
	int id;
	int time;
} haptics_table_hub[] = {
	{ 0, 32, 20 },
	{ 1, 33, 20 },
	{ 2, 34, 20 },
	{ 3, 35, 20 },
	{ 4, 36, 20 },
	{ 5, 37, 20 },
	{ 6, 38, 20 },
	{ 7, 39, 20 },
	{ 8, 40, 25 },
	{ 9, 41, 25 },
	{ 10, 42, 25 },
	{ 11, 43, 25 },
	{ 12, 44, 15 },
	{ 13, 45, 15 },
	{ 14, 46, 15 },
	{ 15, 47, 15 },
	{ 16, 48, 30 },
	{ 17, 49, 30 },
	{ 18, 50, 30 },
	{ 19, 51, 30 },
	{ 20, 52, 30 },
	{ 21, 53, 30 },
	{ 22, 54, 30 },
	{ 23, 55, 30 },
	{ 24, 56, 30 },
	{ 25, 57, 30 },
	{ 26, 58, 30 },
	{ 27, 59, 30 },
	{ 28, 60, 25 },
	{ 29, 61, 25 },
	{ 30, 62, 25 },
	{ 31, 63, 25 },
	{ 32, 64, 15 },
	{ 33, 65, 15 },
	{ 34, 66, 15 },
	{ 35, 67, 15 },
	{ 36, 68, 20 },
	{ 37, 69, 20 },
	{ 38, 70, 20 },
	{ 39, 71, 20 },
	{ 40, 72, 20 },
	{ 41, 73, 20 },
	{ 42, 74, 20 },
	{ 43, 75, 20 },
	{ 44, 76, 30 },
	{ 45, 77, 30 },
	{ 46, 78, 30 },
	{ 47, 79, 30 },
	{ 48, 80, 30 },
	{ 49, 81, 30 },
	{ 50, 82, 30 },
	{ 51, 83, 30 },
	{ 52, 84, 30 },
	{ 53, 85, 30 },
	{ 54, 86, 30 },
	{ 55, 87, 30 },
	{ 56, 88, 30 },
	{ 57, 89, 30 },
	{ 58, 90, 30 },
	{ 59, 91, 30 },
	{ 60, 92, 30 },
	{ 61, 93, 30 },
	{ 62, 94, 30 },
	{ 63, 95, 30 },
	{ 64, 96, 28 },
	{ 65, 97, 28 },
	{ 66, 98, 28 },
	{ 67, 99, 28 },
	{ 68, 100, 21 },
	{ 69, 101, 21 },
	{ 70, 102, 21 },
	{ 71, 103, 21 },
	{ 72, 104, 30 },
	{ 73, 105, 30 },
	{ 74, 106, 30 },
	{ 75, 107, 30 },
	/* tone */
	{ 77, 108, 40 },
	{ 78, 109, 45 },
	{ 79, 110, 50 },
	{ 80, 111, 55 },
	{ 81, 112, 60 },
	{ 82, 113, 65 },
	{ 83, 114, 70 },
	{ 84, 115, 75 },
	{ 85, 116, 80 },
	{ 86, 117, 85 },
	{ 87, 118, 90 },
	{ 88, 119, 95 },
	{ 89, 120, 100 },
	{ 90, 121, 105 },
	{ 91, 122, 110 },
	{ 92, 123, 115 },
	{ 93, 124, 120 },
	{ 94, 125, 125 },
	{ 95, 126, 130 },
	{ 96, 127, 135 },
};

#ifdef CONFIG_HUAWEI_DSM
static struct vib_dmd_param g_dmd_param[VIB_ABNORMITY_INDEX_COUNT_MAX] = {
	{ DSM_SHB_VIBRATOR_OVP_ERROR, "SmartPA %s Over Voltage Protection(OVP)\n" },
	{ 0, NULL },
	{ DSM_SHB_VIBRATOR_OTP_ERROR, "SmartPA %s Over Temperature Protection(OTP)\n" },
	{ DSM_SHB_VIBRATOR_OCP_ERROR, "SmartPA %s Over Current Protection(OCP)\n" },
	{ DSM_SHB_VIBRATOR_UVP_ERROR, "SmartPA %s Under Voltage Protection(UVP)\n" },
	{ DSM_SHB_VIBRATOR_ABNORMAL_ERROR, "SmartPA %s manager alarm\n" },
	{ DSM_SHB_VIBRATOR_ABNORMAL_ERROR, "SmartPA %s TDM error\n" },
	{ DSM_SHB_VIBRATOR_ABNORMAL_ERROR, "SmartPA %s lost clock\n" }
};
#endif

static int vibrator_hardware_reset(void);
static int vibrator_operate_reg_ex(struct reg_operate_param *param);
static int vibrator_reg_read(uint32_t reg_addr, uint32_t *read_value);
static int vibrator_reg_write(uint32_t reg_addr, uint32_t write_value);
static void vibrator_enable(struct led_classdev *cdev, enum led_brightness value);
static void vibrator_set_time(int val);

static int64_t get_timestamp(void)
{
	struct timespec ts;

	get_monotonic_boottime(&ts);
	/* timevalToNano */
	return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

#ifdef CONFIG_HUAWEI_DSM
static void vibrator_dmd_freq(uint32_t freq)
{
	uint32_t dmd_bit;
	struct dsm_client *client = NULL;
	char *chip_str = NULL;

	client = inputhub_get_shb_dclient();
	if (!client) {
		hwlog_err("%s: there is no shb_dclient\n", __func__);
		return;
	}
	if (vibrator_data.chip_type == VIBRATOR_CHIP_TFA9874)
		chip_str = "TFA9874";
	else
		chip_str = "unknown";
	dmd_bit = vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_FREQ);
	if (chip_str) {
		if (dsm_client_ocuppy(client)) {
			hwlog_err("%s: shb_dclient buffer is busy\n", __func__);
			return;
		}
		dsm_client_record(client,
			"vibrator %s frequency detect abnormal %u\n",
			chip_str,
			freq);
		dsm_client_notify(client, DSM_SHB_VIBRATOR_F0_ERROR);
	}
}

static void vibrator_dmd_abnormity(uint32_t abnormity, char *chip_str)
{
	struct dsm_client *client = NULL;
	int32_t i;
	int32_t error_no;
	uint32_t dmd_bit;
	uint32_t record_count;

	if (!chip_str) {
		hwlog_err("%s: chip_str is NULL\n", __func__);
		return;
	}
	client = inputhub_get_shb_dclient();
	if (!client) {
		hwlog_err("%s: there is no shb_dclient\n", __func__);
		return;
	}
	error_no = DSM_SHB_VIBRATOR_ABNORMAL_ERROR;
	record_count = 0;
	for (i = 0; i < VIB_ABNORMITY_INDEX_COUNT_MAX; i++) {
		dmd_bit = vib_abnormity_to_bit(i);
		if ((abnormity & dmd_bit) && g_dmd_param[i].fmt) {
			if (record_count == 0) {
				if (dsm_client_ocuppy(client)) {
					hwlog_err("%s: shb_dclient buffer is busy\n",
						__func__);
					return;
				}
				error_no = g_dmd_param[i].error_no;
			}
			record_count++;
			dsm_client_record(client, g_dmd_param[i].fmt, chip_str);
		}
	}
	if (record_count > 0) {
		/* more then 1, error_no default */
		if (record_count > 1)
			error_no = DSM_SHB_VIBRATOR_ABNORMAL_ERROR;

		dsm_client_notify(client, error_no);
	}
}
#endif

static void vibrator_irq_abnormity(
	bool is_reset,
	bool reset_ok,
	bool i2c_ok,
	uint32_t calc_bits,
	uint32_t out_reg_addr)
{
	int32_t ret;
	uint32_t read_value;
	uint32_t abnormity;
	char *chip_str = NULL;

	if (is_reset && !reset_ok) /* reset failed */
		hwlog_err("%s error reset\n", __func__);
	if (!i2c_ok) {
		/* i2c abnormity, read again */
		read_value = 0;
		ret = vibrator_reg_read(out_reg_addr, &read_value);
		if (ret != 0)
			hwlog_err("%s error i2c\n", __func__);
	}

	if (!calc_bits) {
		hwlog_info("%s calc_bits is 0\n", __func__);
		return;
	}

	abnormity = 0;
	if (vibrator_data.chip_type == VIBRATOR_CHIP_TFA9874) {
		chip_str = "TFA9874";
		if (calc_bits & TFA9874_INT_VDDS)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_VDDS);
		if (calc_bits & TFA9874_INT_OTDS)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_OTDS);
		if (calc_bits & TFA9874_INT_OCPR)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_OCPR);
		if (calc_bits & TFA9874_INT_UVDS)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_UVDS);
		if (calc_bits & TFA9874_INT_MANALARM)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_MANALARM);
		if (calc_bits & TFA9874_INT_TDMER)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_TDMER);
		if (calc_bits & TFA9874_INT_NOCLK)
			abnormity |= vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_NOCLK);
	} else {
		return;
	}

	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_VDDS))
		hwlog_err("%s Over Voltage Protection(OVP)\n",
			__func__);
	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_OTDS))
		hwlog_err("%s Over Temperature Protection(OTP)\n",
			__func__);
	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_OCPR))
		hwlog_err("%s Over Current Protection(OCP)\n",
			__func__);
	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_UVDS))
		hwlog_err("%s Under Voltage Protection(UVP)\n",
			__func__);
	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_MANALARM))
		hwlog_err("%s Status manager alarm state\n",
			__func__);
	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_TDMER))
		hwlog_err("%s Status TDM error\n",
			__func__);
	if (abnormity & vib_abnormity_to_bit(VIB_ABNORMITY_INDEX_NOCLK))
		hwlog_err("%s lost clock status\n",
			__func__);

#ifdef CONFIG_HUAWEI_DSM
	vibrator_dmd_abnormity(abnormity, chip_str);
#endif
}

static void sync_mcu_status(void)
{
	int wait_count = 0;

	hwlog_debug("start sync mcu status\n");
	/* wait until mcu status is not ST_SLEEP
	 * or wait time is over 100ms
	 */
	while (get_iomcu_power_state() == ST_SLEEP && wait_count < 50) {
		mdelay(2);
		wait_count++;
	}
	hwlog_info("sync mcu wait_count is %d\n", wait_count);
}

static void vibrator_irq_work(struct work_struct *work)
{
	int32_t ret;
	uint32_t enable_reg_addr, out_reg_addr, clear_reg_addr;
	uint32_t enable_bits, irq_out_bits, irq_calc_bits;
	bool i2c_ok = false;
	bool is_reset = false;
	bool reset_ok = false;
	bool is_high_default = false;

	if (vibrator_data.chip_type == VIBRATOR_CHIP_TFA9874) {
		out_reg_addr = TFA9874_INT_OUT_REG1;
		clear_reg_addr = TFA9874_INT_IN_REG1;
		enable_reg_addr = TFA9874_INT_ENABLE_REG1;
		is_high_default = true; /* gpio_irq level high default */
	} else {
		hwlog_err("%s chip_type=%x\n",
			__func__, vibrator_data.chip_type);
		return;
	}

	if (vibrator_in_irq_work) {
		hwlog_info("%s vibrator irq in processing\n", __func__);
		return;
	}
	vibrator_in_irq_work = true;

	__pm_stay_awake(&vib_wl);
	sync_mcu_status();

	enable_bits = 0;
	irq_out_bits = 0;
	irq_calc_bits = 0;
	ret = vibrator_reg_read(enable_reg_addr, &enable_bits);
	ret |= vibrator_reg_read(out_reg_addr, &irq_out_bits);
	if (ret == 0) {
		i2c_ok = true;
		irq_calc_bits = irq_out_bits & enable_bits;
	}
	if ((!i2c_ok) || irq_calc_bits) {
		int32_t gpio_value;

		is_reset = true;
		vibrator_set_time(0);
		if (i2c_ok)
			vibrator_reg_write(clear_reg_addr, irq_out_bits);
		vibrator_hardware_reset();
		resend_vibrator_parameters_to_mcu();
		gpio_value = gpio_get_value(vibrator_data.gpio_irq);
		if (((!is_high_default) && (!gpio_value)) ||
			(is_high_default && gpio_value))
			reset_ok = true;
	}
	if (data) {
		data->irq_count++;
		hwlog_info("%s chip_type=%x irq_count=%u irq_calc_bits=%x is_reset=%d reset_ok=%d i2c_ok=%d\n",
			__func__, vibrator_data.chip_type, data->irq_count, irq_calc_bits,
			(int32_t)is_reset, (int32_t)reset_ok, (int32_t)i2c_ok);
	}
	vibrator_irq_abnormity(is_reset, reset_ok, i2c_ok, irq_calc_bits,
		out_reg_addr);

	__pm_relax(&vib_wl);

	vibrator_in_irq_work = false;
}

static void vibrator_irq_set_enable(uint32_t user_bits, bool enable)
{
	if (!data || !data->irq_requested)
		return;

	if (enable) {
		data->irq_user_bits |= user_bits;
		if (!data->irq_enabled) {
			data->irq_enabled = true;
			enable_irq(data->irq_int);
			hwlog_info("%s enable user_bits=%x irq_user_bits=%x\n",
				__func__, user_bits, data->irq_user_bits);
			return;
		}
		hwlog_info("%s already enable user_bits=%x irq_user_bits=%x\n",
			__func__, user_bits, data->irq_user_bits);
	} else {
		data->irq_user_bits &= (~user_bits);
		if (data->irq_user_bits != 0) {
			hwlog_info("%s no need disable user_bits=%x irq_user_bits=%x\n",
				__func__, user_bits, data->irq_user_bits);
			return;
		}
		if (data->irq_enabled) {
			disable_irq(data->irq_int);
			data->irq_enabled = false;
			hwlog_info("%s disable user_bits=%x irq_user_bits=%x\n",
				__func__, user_bits, data->irq_user_bits);
			return;
		}
		hwlog_info("%s already disable user_bits=%x irq_user_bits=%x\n",
			__func__, user_bits, data->irq_user_bits);
	}
}

static irqreturn_t vibrator_irq_handler(int irq, void *dev_id)
{
	if (!data || !data->irq_requested)
		return IRQ_HANDLED;

	disable_irq_nosync(data->irq_int);
	data->irq_enabled = false;

	schedule_work(&data->irq_work);
	return IRQ_HANDLED;
}

static void vibrator_irq_request(struct drv2605_data *data)
{
	int ret;
	unsigned long trigger;

	if (vibrator_data.chip_type == VIBRATOR_CHIP_TFA9874) {
		trigger = IRQF_TRIGGER_LOW;
	} else {
		hwlog_err("%s chip_type=%x\n",
			__func__, vibrator_data.chip_type);
		return;
	}

	gpio_direction_input(vibrator_data.gpio_irq);
	data->irq_int = gpio_to_irq(vibrator_data.gpio_irq);
	if (data->irq_int < 0) {
		hwlog_err("%s gpio_to_irq error\n", __func__);
		return;
	}

	ret = request_irq(data->irq_int, vibrator_irq_handler,
		trigger | IRQF_ONESHOT | IRQF_NO_SUSPEND,
		"vibrator_int_irq", NULL);
	if (ret != 0) {
		hwlog_err("%s request_irq error\n", __func__);
		return;
	}
	enable_irq_wake(data->irq_int);
	disable_irq(data->irq_int);
	data->irq_enabled = false;
	data->irq_requested = true;
	hwlog_info("%s chip_type=%x trigger=%x\n",
		__func__, vibrator_data.chip_type,
		trigger);
}

static int vibrator_hardware_reset(void)
{
	gpio_direction_output(vibrator_data.gpio_reset,
		(~vibrator_data.rst_value) & 0x1);
	msleep(VIB_RESET_GPIO_DELAY);
	gpio_set_value(vibrator_data.gpio_reset, vibrator_data.rst_value);
	msleep(VIB_RESET_GPIO_DELAY);
	if (data) {
		data->reset_count++;
		hwlog_info("%s chip_type=%x reset_count=%u\n",
		__func__, vibrator_data.chip_type, data->reset_count);
	}
	return 0;
}

static int vibrator_operate_reg_ex(struct reg_operate_param *param)
{
	uint8_t bus_num, i2c_address, i, param_invalid;
	int ret = 0;
	uint8_t *ptemp = NULL;
	uint8_t buf[VIB_I2C_BUF_MAX] = { 0 };

	param_invalid = 0;
	param_invalid |= (param->addr_bytes == 0);
	param_invalid |= (param->addr_bytes > sizeof(uint32_t));
	param_invalid |= (param->value_bytes == 0);
	param_invalid |= (param->value_bytes > sizeof(uint32_t));
	param_invalid |=
		((param->addr_bytes + param->value_bytes) > VIB_I2C_BUF_MAX);
	if (param_invalid) {
		hwlog_err("%s param_invalid addr_bytes=%d value_bytes=%d\n",
			__func__, param->addr_bytes, param->value_bytes);
		return -1;
	}

	bus_num = vibrator_data.cfg.bus_num;
	i2c_address = vibrator_data.cfg.i2c_address;
	ptemp = (uint8_t *)&param->reg_addr;
	ptemp += (param->addr_bytes - 1);
	for (i = 0; i < param->addr_bytes; i++)
		buf[i] = *ptemp--;
	if (param->is_read) {
		ret = mcu_i2c_rw(bus_num, i2c_address,
			buf, param->addr_bytes,
			&buf[param->addr_bytes], param->value_bytes);
	} else {
		ptemp = (uint8_t *)&param->write_value;
		ptemp += (param->value_bytes - 1);
		for (i = 0; i < param->value_bytes; i++)
			buf[param->addr_bytes + i] = *ptemp--;
		ret = mcu_i2c_rw(bus_num, i2c_address,
			buf, param->addr_bytes + param->value_bytes,
			NULL, 0);
	}
	if (ret < 0) {
		hwlog_err("%s error is_read=%d ret=%d\n",
			__func__, param->is_read, ret);
		return -1;
	}
	if (param->is_read) {
		param->read_value = 0;
		ptemp = (uint8_t *)&param->read_value;
		ptemp += (param->value_bytes - 1);
		for (i = 0; i < param->value_bytes; i++)
			*ptemp-- = buf[param->addr_bytes + i];
		hwlog_info("%s reg_addr=%x read_value=%x\n",
			__func__, param->reg_addr, param->read_value);
	} else {
		hwlog_info("%s reg_addr=%x write_value=%x\n",
			__func__, param->reg_addr, param->write_value);
	}
	return 0;
}

static int vibrator_reg_read(uint32_t reg_addr, uint32_t *read_value)
{
	struct reg_operate_param param = { 0 };
	int ret;

	param.addr_bytes = vibrator_data.reg_addr_bytes;
	param.value_bytes = vibrator_data.reg_value_bytes;
	param.reg_addr = reg_addr;
	param.is_read = 1;
	ret = vibrator_operate_reg_ex(&param);
	if (ret == 0 && read_value)
		*read_value = param.read_value;
	return ret;
}

static int vibrator_reg_write(uint32_t reg_addr, uint32_t write_value)
{
	struct reg_operate_param param = { 0 };
	int ret;

	param.addr_bytes = vibrator_data.reg_addr_bytes;
	param.value_bytes = vibrator_data.reg_value_bytes;
	param.reg_addr = reg_addr;
	param.write_value = write_value;
	ret = vibrator_operate_reg_ex(&param);
	return ret;
}
int write_vibrator_calib_value_to_nv(const char *temp, uint16_t length)
{
	int ret = 0;
	struct hisi_nve_info_user user_info;

	if (!temp) {
		hwlog_err("write_vibrator_calib_value_to_nv fail, invalid para\n");
		return -1;
	}

	if (length > VIB_CALIDATA_NV_SIZE) {
		hwlog_err("vibrator write nv size is out of range %d\n", length);
		return -1;
	}

	memset(&user_info, 0, sizeof(user_info));
	user_info.nv_operation = NV_WRITE_TAG;
	user_info.nv_number = VIB_CALIDATA_NV_NUM;
	user_info.valid_size = VIB_CALIDATA_NV_SIZE;
	strncpy(user_info.nv_name, "VIBCAL", sizeof(user_info.nv_name));
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	user_info.nv_data[0] = temp[0];
	user_info.nv_data[1] = temp[1];
	user_info.nv_data[2] = temp[2];
	ret = hisi_nve_direct_access(&user_info);
	if (ret != 0) {
		hwlog_err("vibrator nve_direct_access write error %d\n", ret);
		return -1;
	}
	hwlog_info("vibrator nve_direct_access write nv_data 0x%x 0x%x 0x%x\n",
		 user_info.nv_data[0], user_info.nv_data[1],
		 user_info.nv_data[2]);

	return ret;
}

static read_info_t vibrator_send_cali_test_cmd(const char* cmd, int len,
	RET_TYPE *rtype)
{
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_VIBRATOR;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	spkt.subcmd = SUB_CMD_SELFCALI_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = len + SUBCMD_LEN;
	memcpy(spkt.para, cmd, len);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		*rtype = COMMU_FAIL;
		hwlog_err("drv2605 calibrate cmd to mcu fail,ret=%d\n", ret);
		return pkg_mcu;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("drv2605 calibrate fail, %d\n", pkg_mcu.errno);
		*rtype = EXEC_FAIL;
	} else {
		hwlog_info("drv2605 calibrate return success\n");
		*rtype = SUC;
	}
	return pkg_mcu;
}

static read_info_t vibrator_send_cfg_cmd(unsigned int sub_cmd, char *cmd_para,
		unsigned int len, RET_TYPE *rtype)
{
	int ret;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;

	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	if ((sub_cmd > SUB_CMD_MAX) || (!cmd_para)) {
		*rtype = COMMU_FAIL;
		return pkg_mcu;
	}
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_VIBRATOR;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	spkt.subcmd = sub_cmd;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = len + SUBCMD_LEN;
	memcpy(spkt.para, cmd_para, len);
	hwlog_info("tag %d calibrator cmd_para is %d, len is %d\n",
			TAG_VIBRATOR, *(unsigned int *)cmd_para, len);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		*rtype = COMMU_FAIL;
		hwlog_err("calibrate cmd to mcu fail, ret=%d\n", ret);
		return pkg_mcu;
	}
	if (pkg_mcu.errno != 0) {
		hwlog_err("calibrate fail, %d\n", pkg_mcu.errno);
		*rtype = EXEC_FAIL;
	} else {
		hwlog_info("calibrate return success\n");
		*rtype = SUC;
	}
	return pkg_mcu;
}

static void vibrator_off(void)
{
	vibrator_shake = 0;
}

static void vibra_set_work(struct work_struct *work)
{
	int on_time = vib_time;

	if (on_time != 0)
		vibrator_irq_set_enable(VIB_IRQ_USER_BIT_SWITCH, true);

	__pm_stay_awake(&vib_wl);
	sync_mcu_status();
	vibrator_set_time(vib_time);
	__pm_relax(&vib_wl);

	if (on_time == 0)
		vibrator_irq_set_enable(VIB_IRQ_USER_BIT_SWITCH, false);
}

__weak void switch_set_state(struct switch_dev *sdev, int state)
{
	return;
}

__weak int switch_dev_register(struct switch_dev *sdev)
{
	return 0;
}

__weak void switch_dev_unregister(struct switch_dev *sdev)
{
	return;
}

#ifdef CONFIG_HISI_COUL
static void vibrator_agent_cold_level_proc(void)
{
	struct timespec ts;
	int temp;
	int capacity;
	int i;
	int cold_level = EN_VIB_BAT_COLD_LEVEL_0;
	int cold_cnt = vibrator_data.cold_level_count;

	if (cold_cnt <= 0)
		return;
	get_monotonic_boottime(&ts);
	if (ts.tv_sec >= vib_cold_check_timestamp) {
		if ((ts.tv_sec - vib_cold_check_timestamp) < vib_cold_check_delay)
			return;
	} else {
		if ((vib_cold_check_timestamp - ts.tv_sec) < vib_cold_check_delay)
			return;
	}

	vib_cold_check_timestamp = ts.tv_sec;
	temp = hisi_battery_temperature();
	capacity = hisi_battery_capacity();
	for (i = cold_cnt - 1; i >= 0; i--) {
		if (temp > vibrator_data.cold_level[i]) {
			cold_level = EN_VIB_BAT_COLD_LEVEL_0 + cold_cnt - 1 - i;
			break;
		}
	}
	if (temp <= vibrator_data.cold_level[0])
		cold_level = EN_VIB_BAT_COLD_LEVEL_0 + cold_cnt;
	if (cold_level == EN_VIB_BAT_COLD_LEVEL_0) {
		vib_cold_level = EN_VIB_BAT_LEVEL_0;
		vib_cold_check_delay = VIB_COLD_CHECK_LONG_DELAY;
	} else {
		vib_cold_check_delay = VIB_COLD_CHECK_SHORT_DELAY;
		if (capacity <= vibrator_data.battery_level[cold_cnt - cold_level])
			vib_cold_level = EN_VIB_BAT_LEVEL_1;
	}
	hwlog_info("vibrator_agent_cold_level_proc: %d, %d, %d, %d\n",
		cold_level, vib_cold_level, temp, capacity);
}
#endif

static void haptics_play_effect(struct work_struct *work)
{
	RET_TYPE vib_return_calibration = RET_INIT;
	read_info_t read_pkg;
	struct vib_cmd_t vib_data;

	__pm_stay_awake(&vib_wl);
	sync_mcu_status();
	vibrator_shake = 1;
	switch_set_state(&data->sw_dev, SW_STATE_SEQUENCE_PLAYBACK);
	vibrator_irq_set_enable(VIB_IRQ_USER_BIT_EFFECT, true);
	memset(&read_pkg, 0, sizeof(read_pkg));
	vib_data.cmd = data->vib_cmd;
#ifdef CONFIG_HISI_COUL
	vibrator_agent_cold_level_proc();
#endif
	vib_data.level = vib_cold_level;
	read_pkg = vibrator_send_cfg_cmd(SUB_CMD_VIBRATOR_GEN_WAVE_TONE,
		(char *)&vib_data, sizeof(vib_data), &vib_return_calibration);

	if ((vib_return_calibration == COMMU_FAIL) ||
	    (vib_return_calibration == EXEC_FAIL))
		hwlog_err("%s: commu fail\n", __func__);
	else if (read_pkg.errno == 0)
		hwlog_info("%s: commu succ\n", __func__);

	if (data->play_effect_time > 0)
		msleep(data->play_effect_time);

	vibrator_irq_set_enable(VIB_IRQ_USER_BIT_EFFECT, false);
	switch_set_state(&data->sw_dev, SW_STATE_IDLE);
	vibrator_shake = 0;
	__pm_relax(&vib_wl);
	return;
}

static ssize_t vibrator_dbc_test_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	uint64_t value = 0;
	char test_case = 0;
	RET_TYPE vib_return_calibration = RET_INIT;
	read_info_t read_pkg;
	char test_cmd[VIB_TEST_CMD_LEN] = { 0 };

	hwlog_info("%s\n", __func__);
	if (strict_strtoull(buf, CONVERT_TO_16, &value)) {
		hwlog_err("vibrator dbc test read value error\n");
		return -EINVAL;
	}

	test_case = ((char)value)&0xff;
	if ((test_case != 0) && (test_case != 1) && (test_case != 2)) {
		hwlog_err("vibrator dbc test read value para not defiend error\n");
		return -EINVAL;
	}

	test_cmd[0] = VIB_DBC_CALI;
	test_cmd[1] = test_case;
	hwlog_info("vibrator dbc test read value test cmd= %d, %d\n",
		test_cmd[0], test_cmd[1]);
	memset(&read_pkg, 0, sizeof(read_pkg));
	read_pkg = vibrator_send_cali_test_cmd(test_cmd, sizeof(test_cmd), &vib_return_calibration);
	if ((vib_return_calibration == COMMU_FAIL) ||
	    (vib_return_calibration == EXEC_FAIL))
		hwlog_err("%s: commu fail\n", __func__);
	else if (read_pkg.errno == 0)
		hwlog_err("%s: commu succ\n", __func__);

	return count;
}

static ssize_t vibrator_calib_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	read_info_t read_pkg;
	char status = 0;
	RET_TYPE vib_return_calibration = RET_INIT;
	char calib_value[VIB_CALIDATA_NV_SIZE] = { 0 };
	int i = 0;
	int ret = 0;
	char test_case[VIB_TEST_CMD_LEN] = { 0 };
	unsigned int calib_val = 0;

	hwlog_info("%s\n", __func__);
	test_case[0] = VIB_MMI_CALI;
	memset(&read_pkg, 0, sizeof(read_pkg));
	read_pkg = vibrator_send_cali_test_cmd(test_case,sizeof(test_case),
		&vib_return_calibration);
	if ((vib_return_calibration == COMMU_FAIL) ||
	    (vib_return_calibration == EXEC_FAIL)) {
		hwlog_err("%s: cali fail\n", __func__);
		vib_calib_result = 0;
		return count;
	} else if (read_pkg.errno == 0) {
		hwlog_info("%s: commu succ\n", __func__);
	}
	hwlog_info("calib result= 0x%x, 0x%x, 0x%x, 0x%x\n", read_pkg.data[0],
		read_pkg.data[1], read_pkg.data[2], read_pkg.data[3]);
	status = VIB_CA_DIAG_RST & read_pkg.data[0];
	if (status != 0) {
		vib_calib_result = 0;
		hwlog_err("drv2605 vibrator calibration fail!\n");
		return count;
	} else {
		hwlog_info("drv2605 vibrator calibration success!\n");
		vib_calib_result = 1;
		calib_val = (read_pkg.data[0] << 24) +
					(read_pkg.data[1] << 16) +
					(read_pkg.data[2] << 8) +
					read_pkg.data[3];
	}
	/* F0=F0/8192, should be in [158,182] */
	if ((calib_val < (F0_COEF * F0_LOWER_LIMIT)) ||
				(calib_val > (F0_COEF * F0_UPPER_LIMIT))) {
		hwlog_err("vibrator f0 is out of range, 0x%x\n", calib_val);
		vib_calib_result = 0;
#ifdef CONFIG_HUAWEI_DSM
		vibrator_dmd_freq(calib_val / F0_COEF);
#endif
	}
	for(i = 0; i < VIB_CALIDATA_NV_SIZE; i++)
		calib_value[i] = read_pkg.data[i + 1];

	ret = write_vibrator_calib_value_to_nv(calib_value, VIB_CALIDATA_NV_SIZE);
	if (ret) {
		vib_calib_result = 0;
		hwlog_err("vibrator calibration result write to nv fail!\n");
	}
	return count;
}

static ssize_t vibrator_calib_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	int val = vib_calib_result;
	RET_TYPE vib_return_calibration = RET_INIT;
	read_info_t read_pkg;

	memset(&read_pkg, 0, sizeof(read_pkg));
	read_pkg = vibrator_send_cfg_cmd(SUB_CMD_CALIBRATE_DATA_REQ,
			(char *)&data->vib_cmd, sizeof(unsigned int), &vib_return_calibration);
	if ((vib_return_calibration == COMMU_FAIL) ||
			(vib_return_calibration == EXEC_FAIL))
		hwlog_err("%s: commu fail\n", __func__);
	else if (read_pkg.errno == 0)
		hwlog_info("%s: commu succ\n", __func__);

	hwlog_info("vibrator f0 result = 0x%x, 0x%x, 0x%x, 0x%x\n", read_pkg.data[0],
		read_pkg.data[1], read_pkg.data[2], read_pkg.data[3]);
	return snprintf(buf, PAGE_SIZE, "%d", val);
}

static ssize_t vibrator_get_reg_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	uint64_t value = 0;

	if (!buf) {
		hwlog_err("%s bad parameter error\n", __func__);
		return count;
	}

	if (strict_strtoull(buf, CONVERT_TO_16, &value)) {
		hwlog_info("%s read value error\n", __func__);
		goto out;
	}

	vibrator_reg_read((uint32_t)value, &reg_value);
out:
	return count;
}

static ssize_t vibrator_get_reg_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	hwlog_info("%s, reg_value is 0x%x\n", __func__, reg_value);
	return snprintf(buf, PAGE_SIZE, "%x\n", reg_value);
}

static ssize_t vibrator_set_reg_address_store(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf, size_t count)
{
	uint64_t value = 0;

	if (!buf) {
		hwlog_err("%s bad parameter error\n", __func__);
		return count;
	}
	if (strict_strtoull(buf, CONVERT_TO_16, &value)) {
		hwlog_info("%s read value error\n", __func__);
		goto out;
	}
	reg_add = (uint32_t)value;
	hwlog_info("%s, reg_addr is 0x%x\n", __func__, reg_add);

out:
	return count;
}

static ssize_t vibrator_set_reg_value_store(struct device *dev,
					    struct device_attribute *attr,
					    const char *buf, size_t count)
{
	uint32_t val = 0;
	uint64_t value = 0;

	if (!buf){
		hwlog_err("%s bad parameter error\n", __func__);
		return count;
	}
	if (strict_strtoull(buf, CONVERT_TO_16, &value)) {
		hwlog_err("%s read value error\n", __func__);
		return count;
	}
	val = (uint32_t)value;
	hwlog_info("%s, reg_add is 0x%x, reg_value is 0x%x\n",
		__func__, reg_add, val);

	vibrator_reg_write(reg_add, val);
	return count;
}

static ssize_t haptic_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char a[2] = { 0 };
	char haptic_value[100] = { 0 };
	int i, j, m, n;

	if (count < MIN_HAP_BUF_SIZE || count > MAX_HAP_BUF_SIZE || !buf) {
		hwlog_info("-----> haptic_test bad value\n");
		return -1;
	}

	/* get haptic value, the buf max length is count -2 */
	for (i = 0, j = 0; i < 100 && i < count - MIN_HAP_BUF_SIZE;) {
		memcpy(&a[0], &buf[i], 2);
		i = i + 2;
		hwlog_info("-----> haptic_test1 is %d %d\n", a[0],a[1]);

		if ((a[0] == 57) && (a[1] == 57)) {
			haptic_value[j] = 0;
			break;
		} else {
			/* haptic test process */
			m = ((a[0] > 57) ? (a[0] - 97 + 10) : (a[0] - 48)) * 16;
			n =  ((a[1] > 57) ? (a[1] - 97 + 10) : (a[1] - 48));
			haptic_value[j] = m + n;
		}
		hwlog_info("-----> haptic_test2 is 0x%x, m = %d, n=%d\n",
			haptic_value[j],m,n);
		j++;
	}
	vibrator_off();
	memcpy(&data->vib_cmd, haptic_value, sizeof(unsigned int));
	data->play_effect_time = 0;
	haptics_play_effect(NULL);
	hwlog_info("%s\n", __func__);
	return count;
}

static ssize_t vibrator_set_rtp_value_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;

	if (!buf) {
		hwlog_err("drv2605 vibrator_set_rtp_value_store error buf\n");
		goto out;
	}
	if (strict_strtoull(buf, CONVERT_TO_16, &value)) {
		hwlog_info("vibrator_set_rtp_value_store read value error\n");
		goto out;
	}
	rtp_strength = (char)value;
	hwlog_info("vibrator_set_rtp_value_store is 0x%x\n", rtp_strength);

out:
	return count;
}

static ssize_t vibrator_reg_value_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	uint32_t read_regval = 0;

	vibrator_reg_read(reg_add, &read_regval);
	hwlog_info("%s, read_regval is 0x%x\n", __func__, read_regval);
	return snprintf(buf, PAGE_SIZE, "%x\n", (uint32_t)read_regval);
}

static ssize_t vibrator_reg_value_store(struct device *dev,
			struct device_attribute *attr, const char *buf, size_t count)
{
	uint32_t val = 0;
	uint64_t value = 0;

	if (!buf) {
		hwlog_err("drv2605 vibrator_reg_value_store error buf\n");
		goto out;
	}
	if (strict_strtoull(buf, CONVERT_TO_16, &value)) {
		hwlog_info("vibrator_reg_value_store read value error\n");
		goto out;
	}
	val = (uint32_t)value;
	hwlog_info("reg_add is 0x%x, reg_value is 0x%x\n", reg_add, val);
	vibrator_reg_write(reg_add, val);
out:
	return (ssize_t)count;
}

static ssize_t set_amplitude_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	char val = 0;
	uint64_t value = 0;
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;

	if (!buf) {
		hwlog_err("drv2605 set_amplitude_store error buf\n");
		goto out;
	}

	if (strict_strtoull(buf, CONVERT_TO_10, &value)) {
		hwlog_err("drv2605 set_amplitude_store read value error\n");
		goto out;
	}

	val = (char)value;
	hwlog_info("drv2605 set_amplitude_store: reg_values = 0x%x\n", val);

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_VIBRATOR;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	spkt.subcmd = SUB_CMD_VIBRATOR_SET_AMPLITUDE_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(val)+SUBCMD_LEN;
	memcpy(spkt.para, &val, sizeof(val));
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret)
		hwlog_err("drv2605 set_amplitude_store cmd to mcu fail,ret=%d\n",
			ret);

	if (pkg_mcu.errno != 0)
		hwlog_err("drv2605 set_amplitude_store fail, %d\n", pkg_mcu.errno);
	else
		hwlog_info("drv2605 set_amplitude_store  success\n");

out:
	return (ssize_t)count;
}

static ssize_t support_amplitude_control_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n",
		vibrator_data.support_amplitude_control); /* not support */
}

static DEVICE_ATTR(vibrator_dbc_test, S_IRUSR | S_IWUSR, NULL,
		   vibrator_dbc_test_store);
static DEVICE_ATTR(vibrator_calib, S_IRUSR | S_IWUSR, vibrator_calib_show,
		   vibrator_calib_store);
static DEVICE_ATTR(vibrator_get_reg, S_IRUSR | S_IWUSR, vibrator_get_reg_show,
		   vibrator_get_reg_store);
static DEVICE_ATTR(vibrator_set_reg_address, S_IRUSR | S_IWUSR, NULL,
		   vibrator_set_reg_address_store);
static DEVICE_ATTR(vibrator_set_reg_value, S_IRUSR | S_IWUSR, NULL,
		   vibrator_set_reg_value_store);
static DEVICE_ATTR(haptic_test, S_IRUSR|S_IWUSR, NULL, haptic_test_store);
static DEVICE_ATTR(vibrator_set_rtp_value, S_IRUSR|S_IWUSR, NULL,
		   vibrator_set_rtp_value_store);
static DEVICE_ATTR(vibrator_reg_value, S_IRUSR|S_IWUSR, vibrator_reg_value_show,
		   vibrator_reg_value_store);
static DEVICE_ATTR(set_amplitude, S_IRUSR|S_IWUSR, NULL,
		   set_amplitude_store);
static DEVICE_ATTR(support_amplitude_control, S_IRUSR|S_IWUSR,
		   support_amplitude_control_show, NULL);

static struct attribute *vb_attributes[] = {
	&dev_attr_vibrator_dbc_test.attr,
	&dev_attr_vibrator_calib.attr,
	&dev_attr_vibrator_get_reg.attr,
	&dev_attr_vibrator_set_reg_address.attr,
	&dev_attr_vibrator_set_reg_value.attr,
	&dev_attr_haptic_test.attr,
	&dev_attr_vibrator_set_rtp_value.attr,
	&dev_attr_vibrator_reg_value.attr,
	&dev_attr_set_amplitude.attr,
	&dev_attr_support_amplitude_control.attr,
	NULL
};

static const struct attribute_group vb_attr_group = {
	.attrs = vb_attributes,
};

static void vibrator_set_time(int val){
	int ret = 0;
	int64_t time_start;
	int64_t time_end;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	pkt_parameter_req_t spkt;
	pkt_header_t *shd = (pkt_header_t *)&spkt;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_VIBRATOR;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	spkt.subcmd = SUB_CMD_VIBRATOR_ON_REQ;
	pkg_ap.wr_buf = &shd[1];
	pkg_ap.wr_len = sizeof(val) + SUBCMD_LEN;
	memcpy(spkt.para, &val, sizeof(val));
	time_start = get_timestamp();
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret)
		hwlog_err("send tag %d vibrator_set_time cmd to mcu fail,ret=%d\n",
			TAG_VIBRATOR, ret);

	if (pkg_mcu.errno != 0)
		hwlog_err("send tag %d vibrator_set_time fail, %d\n",
			TAG_VIBRATOR, pkg_mcu.errno);

	time_end = get_timestamp();
	hwlog_info("ipc time diff is %lld\n", time_end - time_start);
}
static void vibrator_enable(struct led_classdev *cdev, enum led_brightness value)
{
	int val = value;

	hwlog_info("%s time=%d\n", __func__, val);
	if (val > 0) {
		vibrator_shake = 1;
		if (vibrator_data.reduce_timeout_ms) {
			if ((val > MIN_REDUCE_TIMEOUT) &&
			    (val <= MAX_REDUCE_TIMEOUT))
				val = vibrator_data.reduce_timeout_ms;
		}
		if (val > vibrator_data.max_timeout_ms)
			val = vibrator_data.max_timeout_ms;
		vib_time = val;
	} else {
		vib_time = 0;
		vibrator_shake = 0;
	}
	schedule_work(&data->work);
}

static int haptics_open(struct inode * i_node, struct file * filp)
{
	if (!data)
		return -ENODEV;
	hwlog_info("haptics_open");
	filp->private_data = data;
	return 0;
}

static ssize_t haptics_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
	uint64_t type = 0;
	char write_buf[MAX_BUF_LEGTH] = { 0 };
	struct drv2605_data *data = NULL;

	if (len > MAX_BUF_LEGTH || !buff || !filp || !off) {
		hwlog_err("[haptics_write] bad value\n");
		return len;
	}

	data = (struct drv2605_data *)filp->private_data;

	mutex_lock(&data->lock);

	if (copy_from_user(write_buf, buff, len)) {
		hwlog_err("[haptics_write] copy_from_user failed\n");
		goto out;
	}
	if (strict_strtoull(write_buf, CONVERT_TO_10, &type)) {
		hwlog_err("[haptics_write] read value error\n");
		goto out;
	}
	/* haptics basic ID is 32 */
	if ((type < 32) || (type - 32) >= ARRAY_SIZE(haptics_table_hub)) {
		hwlog_err("[haptics_write] type is out of range: %d\n", type);
		goto out;
	}

	data->vib_cmd = type;
	vibrator_off();
	hwlog_info("[haptics write] vib_cmd is 0x%x\n",
			data->vib_cmd);

	/* base id is 32(0x20) */
	data->play_effect_time = haptics_table_hub[type - 32].time;
	schedule_work(&data->work_play_eff);
out:
	mutex_unlock(&data->lock);
	return len;
}

static ssize_t haptics_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
	ssize_t count = 0;
	struct drv2605_data *data = NULL;
	char read_buf[MAX_BUF_RD_LEGTH] = { 0 };

	if ((len > MAX_BUF_RD_LEGTH) || !buff || !filp || !off ||
		!filp->private_data) {
		hwlog_err("[haptics_read] bad value\n");
		goto out;
	}
	data = (struct drv2605_data *)filp->private_data;
	count = snprintf(read_buf, PAGE_SIZE, "%d", data->vib_cmd);
	if (copy_to_user(buff, read_buf, len)) {
		hwlog_err("[haptics_read] copy_to_user failed\n");
		goto out;
	}
out:
	return count;
}

static struct file_operations fops = {
	.open = haptics_open,
	.write = haptics_write,
	.read = haptics_read,
};

static int haptics_probe(struct drv2605_data *data)
{
	int ret = -ENOMEM;

	data->version = MKDEV(0, 0);
	ret = alloc_chrdev_region(&data->version, 0, 1, CDEVIE_NAME);
	if (ret < 0) {
		hwlog_info("drv2605: error getting major number %d\n", ret);
		return ret;
	}

	data->class = class_create(THIS_MODULE, CDEVIE_NAME);
	if (!data->class) {
		hwlog_info("drv2605: error creating class\n");
		goto unregister_cdev_region;
	}

	data->device = device_create(data->class, NULL, data->version,
		NULL, CDEVIE_NAME);
	if (!data->device) {
		hwlog_info("drv2605: error creating device 2605\n");
		goto destory_class;
	}

	cdev_init(&data->cdev, &fops);
	data->cdev.owner = THIS_MODULE;
	data->cdev.ops = &fops;
	ret = cdev_add(&data->cdev, data->version, 1);
	if (ret) {
		hwlog_info("drv2605: fail to add cdev\n");
		goto destory_device;
	}

	data->sw_dev.name = "haptics";
	ret = switch_dev_register(&data->sw_dev);
	if (ret < 0) {
		hwlog_info("drv2605: fail to register switch\n");
		goto destory_device;
	}

	vibrator_irq_request(data);
	wakeup_source_init(&vib_wl, "haptic_vib_wakelock");
	return 0;

destory_device:
	device_destroy(data->class, data->version);
destory_class:
	class_destroy(data->class);
unregister_cdev_region:
	unregister_chrdev_region(data->version, 1);

	return ret;
}

static int support_vibratorhub(void)
{
	struct device_node *dn = NULL;
	char *sensor_ty = NULL;
	char *sensor_st = NULL;
	const char *st = "ok";
	int ret = -1;

	for_each_node_with_property(dn, "sensor_type") {
		/* sensor type */
		ret = of_property_read_string(dn, "sensor_type",
			(const char **)&sensor_ty);
		if (ret) {
			hwlog_err("get sensor type fail ret=%d\n", ret);
			continue;
		}
		ret = strncmp(sensor_ty, VIBRA_NAME, sizeof(VIBRA_NAME));
		if(!ret){
			ret = of_property_read_string(dn, "status",
				(const char **)&sensor_st);
			if (ret) {
				hwlog_err("get sensor status fail ret=%d\n", ret);
				return -1;
			}
			ret = strcmp(st,sensor_st);
			if( !ret){
				hwlog_info("%s : vibrator status is %s\n",
					__func__, sensor_st);
				return 0;
			}
		}
	}
	return -1;
}

/*
 * Function:       vibratorhub_init
 * Description:    apply kernel buffer, register vibratorhub timeout_dev
 * Data Accessed:  no
 * Data Updated:   no
 * Input:          void
 * Output:         void
 * Return:         result of function, 0 successed, else false
 */
static int __init vibratorhub_init(void)
{
	int ret, rc;
	char vb_name[50]= "vibrator";
	struct timespec ts;

	if (is_sensorhub_disabled()) {
		return -1;
	}
	if (support_vibratorhub()) {
		return - 1;
	}
	data = kzalloc(sizeof(struct drv2605_data), GFP_KERNEL);
	if (!data) {
		hwlog_err("unable to allocate memory\n");
		return -ENOMEM;
	}
	mutex_init(&data->lock);
	data->cclassdev.name = vb_name;
	data->cclassdev.flags = 0;
	data->cclassdev.brightness_set = vibrator_enable;
	data->cclassdev.default_trigger = "transient";
	rc = led_classdev_register(NULL, &data->cclassdev);
	if (rc) {
		hwlog_err("%s, unable to register with timed_output\n", __func__);
		kfree(data);
		data = NULL;
		return -ENOMEM;
	}
	ret = sysfs_create_group(&data->cclassdev.dev->kobj, &vb_attr_group);
	if (ret) {
		hwlog_err("%s, unable create vibrator's sysfs,DBC check IC fail\n",
			__func__);
		led_classdev_unregister(&data->cclassdev);
		kfree(data);
		data = NULL;
		return -ENOMEM;
	}

	INIT_WORK(&data->work_play_eff, haptics_play_effect);
	INIT_WORK(&data->work, vibra_set_work);
	INIT_WORK(&data->irq_work, vibrator_irq_work);

	haptics_probe(data);

	get_monotonic_boottime(&ts);
	vib_cold_check_timestamp = ts.tv_sec;
#ifdef CONFIG_HISI_COUL
	vibrator_agent_cold_level_proc();
#endif

	hwlog_info("vibrator_agent_cold_level_proc: %d, %d",
		vib_cold_level, vib_cold_check_delay);
	hwlog_info("%s success!", __func__);
	return rc;

}

/*
 * Function:       fingerprinthub_exit
 * Description:    release kernel buffer, deregister fingerprinthub_miscdev
 * Data Accessed:  no
 * Data Updated:   no
 * Input:          void
 * Output:         void
 * Return:         void
 */
static void __exit vibratorhub_exit(void)
{
	if (!data)
		return;
	if (data->irq_requested) {
		if (data->irq_enabled) {
			disable_irq(data->irq_int);
			data->irq_enabled = false;
		}
		free_irq(data->irq_int, NULL);
		data->irq_requested = false;
	}
	mutex_destroy(&data->lock);
	sysfs_remove_group(&data->cclassdev.dev->kobj, &vb_attr_group);
	led_classdev_unregister(&data->cclassdev);
	cancel_work_sync(&data->work_play_eff);
	cancel_work_sync(&data->work);
	cancel_work_sync(&data->irq_work);
	kfree(data);
	data = NULL;
	hwlog_info("exit %s\n", __func__);
}

late_initcall_sync(vibratorhub_init);
module_exit(vibratorhub_exit);

MODULE_AUTHOR("VBHub <hujianglei@huawei.com>");
MODULE_DESCRIPTION("VBHub driver");
MODULE_LICENSE("GPL");
