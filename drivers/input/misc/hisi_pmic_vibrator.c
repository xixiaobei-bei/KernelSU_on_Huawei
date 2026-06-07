/*
 * Hisi pmic vibrator driver for Hisilicon Hi64xx pmic vibrator.
 *
 * Copyright (c) 2017 Hisilicon Technologies CO.Ltd.
 *		http://www.hisilicon.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "hisi_pmic_vibrator.h"
#include <securec.h>
#include <asm/irq.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/hisi-spmi.h>
#include <linux/hisi/hisi_vibrator.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/leds.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_hisi_spmi.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/switch.h>
#include <linux/syscalls.h>
#include <linux/pm_wakeup.h>
#include <linux/workqueue.h>
#ifdef CONFIG_BOOST_5V
#include <huawei_platform/power/boost_5v.h>
#endif

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif


#if defined(CONFIG_HISI_VIBRATOR)
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif

struct hisi_pmic_vibrator_haptics_cfg {
	u32 ladd_para;
	u32 spel_duty;
};

struct hisi_pmic_vibrator_haptics_lib {
	struct hisi_pmic_vibrator_haptics_cfg
		cfg[HISI_PMIC_VIBRATOR_REG_CFG_NUM];
};

struct hisi_pmic_vibrator_irq {
	s32 num;
	s8 irq_name[16];
};

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_vibrator = {
	.name = "dsm_vibrator",
	.device_name = "hisi-pmic-vibrator",
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

static u32 pmic_vibrator_dmd_flage;
static s8 pmic_vibrator_happend_irq_name[16];

#define PMIC_VIBRATOR_DMD_NO_FLAGE 0x0
#define PMIC_VIBRATOR_DMD_IRQ_OCP_FLAGE (0x1 << 0)
#define PMIC_VIBRATOR_DMD_IRQ_OUT_FLAGE (0x1 << 1)
#define PMIC_VIBRATOR_DMD_IRQ_UNDERVOL_FLAGE (0x1 << 2)
#define PMIC_VIBRATOR_DMD_IRQ_OVERVOL_FLAGE (0x1 << 3)
#define PMIC_VIBRATOR_DMD_IRQ_ADC_FLAGE (0x1 << 4)
#define PMIC_VIBRATOR_DMD_FREQ_FLAGE (0x1 << 5)
#define PMIC_VIBRATOR_DMD_REPORT_NUM 2
#define PMIC_VIBRATOR_DMD_CLEAR 0
static u32 irq_ocp_time;
static u32 irq_out_time;
static u32 irq_undervol_time;
static u32 irq_overvol_time;
static u32 irq_adc_time;
struct dsm_client *vib_dclient;
#endif

struct hisi_pmic_vibrator_dev {
	struct device *dev;
	struct class *class;
	struct led_classdev led_dev;
	struct cdev cdev;
	struct switch_dev sw_dev;
	struct mutex lock;
	struct wakeup_source wakelock;
	dev_t version;
	struct hisi_pmic_vibrator_haptics_lib *haptics_lib;
	struct hisi_pmic_vibrator_irq lra_irq[HISI_PMIC_VIBRATOR_IRQ_COUNTS];
	struct work_struct hisi_pmic_vibrator_irq_work;
	struct work_struct vibrator_off_work;
	struct work_struct vibrator_enable_work;
	u32 haptics_counts;
	u32 pmic_vibrator_boost_power;
	u32 pmic_vibrator_freq;
	u32 pmic_vibrator_correct_freq;
	u32 pmic_vibrator_strength;
	u32 pmic_vibrator_vol;
	u32 state;
	s8 name[32];
};

static struct hisi_pmic_vibrator_dev *g_vdev;

static s8 hisi_pmic_vibrator_irq_type[HISI_PMIC_VIBRATOR_IRQ_COUNTS][16] = {
	"ocp", "out", "undervol", "overvol", "adc",
};

#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
#define MAX_INPUT_SIZE 63
#endif

/* read register  */
static u8 hisi_pmic_vibrator_read_u8(const u32 vibrator_address)
{
	return hisi_pmic_reg_read(vibrator_address);
}

/* write register  */
static void hisi_pmic_vibrator_write_u8(
	u8 vibrator_set, const u32 vibrator_address)
{
	hisi_pmic_reg_write(vibrator_address, vibrator_set);
}

static void hisi_pmic_vibrator_set_mode(u8 mode)
{
	hisi_pmic_vibrator_write_u8(mode, HISI_PMIC_VIBRATOR_ON);
}

static int hisi_pmic_vibrator_power_on_off(int pmic_vibrator_power_onoff)
{
	int retry = PMIC_VIBRATOR_BRAKE_TIME_OUT;

	if (g_vdev->pmic_vibrator_boost_power) {
		if (pmic_vibrator_power_onoff) {
			boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_MOTOER);
			mdelay(2);
			dev_info(g_vdev->dev, "boost 5v on\n");
		} else {
			while (hisi_pmic_vibrator_read_u8(
				       HISI_PMIC_VIBRATOR_STATUS) &&
				retry) {
				mdelay(1);
				retry--;
			}
			dev_info(g_vdev->dev,
				"vibraor status is 0x%x,retry is %d\n",
				hisi_pmic_vibrator_read_u8(
					HISI_PMIC_VIBRATOR_STATUS),
				retry);
			boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_MOTOER);
			mdelay(5);
			dev_info(g_vdev->dev, "boost 5v off\n");
		}
	}
	return 0;
}

static void hisi_pmic_vibrator_get_freq(void)
{
	unsigned int lra_nml_time_l;
	unsigned int lra_nml_time_m;
	unsigned int lra_nml_time_h;

	lra_nml_time_l =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_TIME_L);
	lra_nml_time_m =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_TIME_M);
	lra_nml_time_h =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_TIME_H);

	g_vdev->pmic_vibrator_freq =
		(lra_nml_time_h << 16) | (lra_nml_time_m << 8) | lra_nml_time_l;
	dev_info(g_vdev->dev, "frep is 0x%x\n", g_vdev->pmic_vibrator_freq);
}

static void hisi_pmic_vibrator_off(struct hisi_pmic_vibrator_dev *vdev)
{
#ifdef CONFIG_HUAWEI_DSM
	hisi_pmic_vibrator_get_freq();
	if ((vdev->pmic_vibrator_freq > PMIC_VIBRATOR_FREQ_MAX) ||
		(vdev->pmic_vibrator_freq < PMIC_VIBRATOR_FREQ_MIN)) {
		dev_info(vdev->dev,
			"this time will notify dsm_vibator_freq:0x%x\n",
			vdev->pmic_vibrator_freq);
		if (!dsm_client_ocuppy(vib_dclient)) {
			if (!(pmic_vibrator_dmd_flage &
				    PMIC_VIBRATOR_DMD_FREQ_FLAGE)) {
				dsm_client_record(vib_dclient,
					"PMU LRA driver frequency detect abnormal. 0x%x\n",
					vdev->pmic_vibrator_freq);
				dsm_client_notify(vib_dclient,
					DSM_PMIC_VIBRATOR_FREQ_CHECK_NO);
				pmic_vibrator_dmd_flage |=
					PMIC_VIBRATOR_DMD_FREQ_FLAGE;
			} else {
				dsm_client_unocuppy(vib_dclient);
			}
		}
	} else {
		vdev->pmic_vibrator_correct_freq = vdev->pmic_vibrator_freq;
	}
#endif
	hisi_pmic_vibrator_write_u8(
		HISI_PMIC_VIBRATOR_BRK_EN, HISI_PMIC_VIBRATOR_BRAKE);
	vdev->pmic_vibrator_strength = vdev->pmic_vibrator_vol;
	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);
}

static s32 hisi_pmic_vibrator_set_strength(void)
{
	uint64_t val;
	u32 duty_l, duty_h;
	u32 ovdr_l, ovdr_h;
	u32 boost_l, boost_h;
	u32 brake_l, brake_h;

	dev_info(g_vdev->dev, "pmic_vibrator strength  is %d.\n",
		g_vdev->pmic_vibrator_strength);
	// normal duty
	val = (uint64_t)g_vdev->pmic_vibrator_strength * DUTY_NUM /
	      VOLTAGE_NUM;
	if (val > DUTY_NUM)
		val = DUTY_NUM;

	duty_l = (val & 0xf) << 4; // low 8 bit
	duty_h = val >> 4;	 // high 8 bit
	hisi_pmic_vibrator_write_u8(
		duty_l, HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_L);
	hisi_pmic_vibrator_write_u8(
		duty_h, HISI_PMIC_VIBRATOR_DUTY_NORMAL_CFG_H);
	// ovdr duty
	val = ((uint64_t)g_vdev->pmic_vibrator_strength * OVDR_DUTY_A
		+ OVDR_DUTY_B) / 1000000;
	ovdr_l = duty_l; //(val & 0xf) << 4; // low 8 bit
	ovdr_h = duty_h; // val >> 4;  //high 8 bit
	hisi_pmic_vibrator_write_u8(ovdr_l, HISI_PMIC_VIBRATOR_OVDR_DUTY_L);
	hisi_pmic_vibrator_write_u8(ovdr_h, HISI_PMIC_VIBRATOR_OVDR_DUTY_H);
	// boost
	val = ((uint64_t)g_vdev->pmic_vibrator_strength *
			TH_BOOST_A + TH_BOOST_B) / 1000000;
	boost_l = val & 0xff; // low 8 bit
	boost_h = val >> 8;   // high 8 bit
	hisi_pmic_vibrator_write_u8(boost_l, HISI_PMIC_VIBRATOR_TH_BOOST_L);
	hisi_pmic_vibrator_write_u8(boost_h, HISI_PMIC_VIBRATOR_TH_BOOST_H);
	// drv num
	hisi_pmic_vibrator_write_u8(
		HISI_PMIC_VIBRATOR_DRV_NUM_VALUE, HISI_PMIC_VIBRATOR_DRV_NMU);
	// brake duty
	val = ((uint64_t)g_vdev->pmic_vibrator_strength * BRAKE_DUTY_A -
		      BRAKE_DUTY_B) /
	      1000000;
	if (val > DUTY_NUM)
		val = DUTY_NUM;

	brake_l = (val & 0xf) << 4; // low 8 bit
	brake_h = val >> 4;	 // high 8 bit
	hisi_pmic_vibrator_write_u8(brake_l, HISI_PMIC_VIBRATOR_BRAKE_DUTY_L);
	hisi_pmic_vibrator_write_u8(brake_h, HISI_PMIC_VIBRATOR_BRAKE_DUTY_H);
	// gate1
	hisi_pmic_vibrator_write_u8(HISI_PMIC_VIBRATOR_GATE_VOLTAGE1_VALUE_L,
		HISI_PMIC_VIBRATOR_GATE_VOLTAGE1_L);
	hisi_pmic_vibrator_write_u8(HISI_PMIC_VIBRATOR_GATE_VOLTAGE1_VALUE_H,
		HISI_PMIC_VIBRATOR_GATE_VOLTAGE1_H);
	// gate2
	hisi_pmic_vibrator_write_u8(HISI_PMIC_VIBRATOR_GATE_VOLTAGE2_VALUE_L,
		HISI_PMIC_VIBRATOR_GATE_VOLTAGE2_L);
	hisi_pmic_vibrator_write_u8(HISI_PMIC_VIBRATOR_GATE_VOLTAGE2_VALUE_H,
		HISI_PMIC_VIBRATOR_GATE_VOLTAGE2_H);
	// gate3
	hisi_pmic_vibrator_write_u8(HISI_PMIC_VIBRATOR_GATE_VOLTAGE3_VALUE_L,
		HISI_PMIC_VIBRATOR_GATE_VOLTAGE3_L);
	hisi_pmic_vibrator_write_u8(HISI_PMIC_VIBRATOR_GATE_VOLTAGE3_VALUE_L,
		HISI_PMIC_VIBRATOR_GATE_VOLTAGE3_H);

	return 0;
}

static void hisi_pmic_vibrator_off_work(struct work_struct *work)
{
	hisi_pmic_vibrator_off(g_vdev);
}

static void hisi_pmic_vibrator_enable(struct hisi_pmic_vibrator_dev *vdev)

{
	if (vdev == NULL)
		return;

	mutex_lock(&vdev->lock);

	if (vdev->state) {
		hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_ON);
		hisi_pmic_vibrator_set_strength();
		hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);
		hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_RTP);
		vibrator_shake = 1;
		dev_info(vdev->dev, "hisi_pmic_vibrator_RTP is running\n");
	} else {
		hisi_pmic_vibrator_off(vdev);
		vibrator_shake = 0;
	}

	mutex_unlock(&vdev->lock);
}

static void hisi_pmic_vibrator_enable_work(struct work_struct *work)
{
	hisi_pmic_vibrator_enable(g_vdev);
}

static void hisi_pmic_vibrator_enable_ctrl(
	struct led_classdev *led_dev, enum led_brightness state)
{
	struct hisi_pmic_vibrator_dev *vdev =
		container_of(led_dev, struct hisi_pmic_vibrator_dev, led_dev);

	vdev->state = state;
	schedule_work(&vdev->vibrator_enable_work);
}

static void haptics_num_set(u32 type)
{
	switch (type) {
	case HISI_PMIC_VIBRATOR_HAPTIC_1:
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE2_6,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG2);
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE3_6,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG3);
		dev_info(g_vdev->dev, "%s 6 cycle\n", __func__);
		break;
	case HISI_PMIC_VIBRATOR_HAPTIC_2:
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE3_8,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG3);
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE4_8,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG4);
		dev_info(g_vdev->dev, "%s 8 cycle\n",  __func__);
		break;
	case HISI_PMIC_VIBRATOR_HAPTIC_3:
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE4_10,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG4);
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE5_10,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG5);
		dev_info(g_vdev->dev, "%s 10 cycle\n",  __func__);
		break;
	case HISI_PMIC_VIBRATOR_HAPTIC_4:
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE2_6,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG2);
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE3_6,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG3);
		dev_info(g_vdev->dev, "%s 4 cycle\n",  __func__);
		break;
	case HISI_PMIC_VIBRATOR_HAPTIC_5:
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE2_6,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG2);
		hisi_pmic_vibrator_write_u8(
			HISI_PMIC_VIBRATOR_SPEL_TIME_VALUE3_6,
			HISI_PMIC_VIBRATOR_SPEL_TIME_REG3);
		dev_info(g_vdev->dev, "%s 5 cycle\n",  __func__);
		break;
	default:
		dev_err(g_vdev->dev, "input val is error!");
	}
}

static s32 hisi_pmic_vibrator_haptic_cfg(
	struct hisi_pmic_vibrator_dev *vdev, u32 type)
{
	u32 val;
	u32 val1;
	u32 ladd_reg, duty_reg, time_reg0;
	u32 i, idx;
	s32 ret;

	if (vdev == NULL)
		return -EINVAL;

	if (type > vdev->haptics_counts || !type) {
		dev_err(vdev->dev, "type:%d is invaild\n", type);
		return -EINVAL;
	}

	idx = type - 1;
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);

	for (i = 0; i < HISI_PMIC_VIBRATOR_REG_CFG_NUM; i++) {
		ladd_reg = HISI_PMIC_VIBRATOR_LADD_PARA_REG + i;
		duty_reg = HISI_PMIC_VIBRATOR_SPEL_DUTY_REG + i;

		/* configs time register and duty registers */
		hisi_pmic_vibrator_write_u8(
			vdev->haptics_lib[idx].cfg[i].ladd_para, ladd_reg);
		hisi_pmic_vibrator_write_u8(
			vdev->haptics_lib[idx].cfg[i].spel_duty, duty_reg);
	}

	hisi_pmic_vibrator_get_freq();
	if ((vdev->pmic_vibrator_freq > PMIC_VIBRATOR_FREQ_MAX) ||
		(vdev->pmic_vibrator_freq < PMIC_VIBRATOR_FREQ_MIN)) {
		vdev->pmic_vibrator_freq = vdev->pmic_vibrator_correct_freq;
	}
	val = vdev->pmic_vibrator_freq * LRA_NML_TIME / LRA_CYCLE_NUM /
	      LRA_TIME_NUM;
	val1 = vdev->pmic_vibrator_freq * LRA_NML_TIME / LRA_CYCLE_NUM /
	       LRA_TIME_NUM1;
	ret = val1 - val * LRA_HAPTIC_NUM;
	if (ret > LRA_HAPTIC_NUM1)
		val++;

	for (i = 0; i < HISI_PMIC_VIBRATOR_HAPTIC_NUM; i++) {
		time_reg0 = HISI_PMIC_VIBRATOR_SPEL_TIME_REG0 + i;
		hisi_pmic_vibrator_write_u8(val, time_reg0);
	}

	haptics_num_set(type);
	dev_info(vdev->dev, "%s complete\n",  __func__);

	return 0;
}

static ssize_t hisi_pmic_vibrator_haptics_write(
	struct file *filp, const char *buff, size_t len, loff_t *off)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;
	uint64_t type;
	s32 ret;
	char write_buf[PMIC_VIBRATOR_HAP_BUF_LENGTH] = {0};

	if (filp == NULL)
		return 0;

	vdev =(struct hisi_pmic_vibrator_dev *)filp->private_data;

	mutex_lock(&vdev->lock);
	vibrator_shake = 1;

	if (len > PMIC_VIBRATOR_HAP_BUF_LENGTH || buff == NULL) {
		dev_info(g_vdev->dev, "hisi pmic vibrator haptic buf is bad\n");
		goto out;
	}

	if (copy_from_user(write_buf, buff, len)) {
		dev_info(g_vdev->dev,
			"hisi pmic vibrator haptics copy from user failed\n");
		goto out;
	}

	if (kstrtoull(write_buf, 10, &type)) {
		dev_info(g_vdev->dev,
			"hisi pmic vibrator haptics read value error\n");
		goto out;
	}

	ret = hisi_pmic_vibrator_haptic_cfg(vdev, (u32)type);
	if (ret < 0) {
		dev_info(
			g_vdev->dev, "hisi pmic vibrator haptics cfg failed\n");
		goto out;
	}

	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_ON);
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_HAPTICS);
	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);

out:
	vibrator_shake = 0;
	mutex_unlock(&vdev->lock);

	return len;
}

void hisi_pmic_vibrator_haptics_set_type(int type)
{
	s32 ret;

	mutex_lock(&g_vdev->lock);

	ret = hisi_pmic_vibrator_haptic_cfg(g_vdev, (u32)type);
	if (ret) {
		dev_err(g_vdev->dev, "hisi_haptic_test error\n");
		mutex_unlock(&g_vdev->lock);
		return;
	}

	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_ON);
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_HAPTICS);
	dev_info(g_vdev->dev, "%s is running\n", __func__);
	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);

	mutex_unlock(&g_vdev->lock);
}
EXPORT_SYMBOL(hisi_pmic_vibrator_haptics_set_type);

#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG

/* calc value of voltage */
static s32 hisi_pmic_vibrator_set_rtp_val(
	struct hisi_pmic_vibrator_dev *vdev, u32 voltage_level)
{
	if (vdev == NULL)
		return -EINVAL;

	if (voltage_level < PMIC_VIBRATOR_VOL_LEVEL_1 ||
		voltage_level > PMIC_VIBRATOR_VOL_LEVEL_16) {
		dev_err(vdev->dev, "vibrator voltage level is invalid!\n");
		return -EINVAL;
	}

	if (voltage_level <= PMIC_VIBRATOR_VOL_LEVEL_10)
		vdev->pmic_vibrator_strength =
			voltage_level * PMIC_VIBRATOR_STRENGTH_STEP1;
	else
		vdev->pmic_vibrator_strength =
			(voltage_level - PMIC_VIBRATOR_VOL_LEVEL_10) *
				PMIC_VIBRATOR_STRENGTH_STEP2 +
			PMIC_VIBRATOR_VOL_LEVEL_10 *
				PMIC_VIBRATOR_STRENGTH_STEP1;

	return 0;
}

static ssize_t hisi_pmic_vibrator_voltage_change_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value;
	s32 ret;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, 10, &value))) {
		dev_err(g_vdev->dev, "vibrator voltage read value error\n");
		return count;
	}
	mutex_lock(&g_vdev->lock);
	ret = hisi_pmic_vibrator_set_rtp_val(g_vdev, value);
	if (ret < 0)
		dev_err(g_vdev->dev, "pmic_vibrator set rtp val faild\n");
	mutex_unlock(&g_vdev->lock);

	return count;
}

static ssize_t hisi_pmic_vibrator_change_mode_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t val;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, 10, &val))) {
		dev_err(g_vdev->dev, "invaild val\n");
		return -EINVAL;
	}

	mutex_lock(&g_vdev->lock);
	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_ON);

	switch (val) {
	case HISI_PMIC_VIBRATOR_MODE_STANDBY:
		hisi_pmic_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "hisi_pmic_vibrator mode is standby\n");
		break;
	case HISI_PMIC_VIBRATOR_MODE_RTP:
		hisi_pmic_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "hisi_pmic_vibrator mode is rtp\n");
		break;
	case HISI_PMIC_VIBRATOR_MODE_HAPTICS:
		hisi_pmic_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "hisi_pmic_vibrator mode is haptics\n");
		break;
	default:
		dev_err(g_vdev->dev, "input val is error!\n");
	}

	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);
	mutex_unlock(&g_vdev->lock);

	return count;
}

static ssize_t hisi_haptic_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	s32 ret;
	uint64_t val;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, 10, &val))) {
		dev_err(g_vdev->dev, "invaild parameters\n");
		return -EINVAL;
	}

	mutex_lock(&g_vdev->lock);

	ret = hisi_pmic_vibrator_haptic_cfg(g_vdev, val);
	if (ret) {
		dev_err(g_vdev->dev, "hisi_haptic_test error\n");
		mutex_unlock(&g_vdev->lock);
		return -EINVAL;
	}

	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_ON);
	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_HAPTICS);
	dev_info(g_vdev->dev, "hisi_pmic_vibrator_haptic is running\n");
	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);

	mutex_unlock(&g_vdev->lock);

	return count;
}

static DEVICE_ATTR(hisi_vibrator_voltage_change, 0600, NULL,
	hisi_pmic_vibrator_voltage_change_store);
static DEVICE_ATTR(vibrator_change_mode_test, 0600, NULL,
	hisi_pmic_vibrator_change_mode_test_store);
static DEVICE_ATTR(
	hisi_haptic_test, 0600, NULL, hisi_haptic_test_store);

static struct attribute *hisi_vb_attributes[] = {
	&dev_attr_hisi_vibrator_voltage_change.attr,
	&dev_attr_vibrator_change_mode_test.attr,
	&dev_attr_hisi_haptic_test.attr, NULL};

static const struct attribute_group hisi_vb_attr_group = {
	.attrs = hisi_vb_attributes,
};

#endif

static s32 hisi_pmic_vibrator_haptics_open(
	struct inode *i_node, struct file *filp)
{
	if (filp == NULL)
		return -1;
	filp->private_data = g_vdev;
	pr_err("%s:haptics open\n", __func__);
	return 0;
}

static const struct file_operations hisi_pmic_vibrator_fops = {
	.open = hisi_pmic_vibrator_haptics_open,
	.write = hisi_pmic_vibrator_haptics_write,
};

#ifdef CONFIG_HUAWEI_DSM
static void hisi_pmic_vibrator_dmd_log(void)
{
	int out_nml_vbemf_l, out_nml_vbemf_h;
	int ureal_data_l, ureal_data_h;
	int adc_zero_l, adc_zero_h;
	int kfit_data_l, kfit_data_h;
	int lra_nml_time_l, lra_nml_time_m, lra_nml_time_h;
	int lra_nml_beml_l, lra_nml_beml_h;

	out_nml_vbemf_l =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_OUT_NML_VBEMF_L);
	out_nml_vbemf_h =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_OUT_NML_VBEMF_H);
	ureal_data_l =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_UREAL_DATA_L);
	ureal_data_h =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_UREAL_DATA_H);
	adc_zero_l = hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_ADC_ZERO_L);
	adc_zero_h = hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_ADC_ZERO_H);
	kfit_data_l =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_KFIT_DATA_L);
	kfit_data_h =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_KFIT_DATA_H);
	lra_nml_time_l =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_TIME_L);
	lra_nml_time_m =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_TIME_M);
	lra_nml_time_h =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_TIME_H);
	lra_nml_beml_l =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_BEML_L);
	lra_nml_beml_h =
		hisi_pmic_vibrator_read_u8(HISI_PMIC_VIBRATOR_LRA_NML_BEML_H);

	dsm_client_record(vib_dclient,
				"PMU LRA driver irq: %s! out_nml_vbemf:%x,%x "
				"ureal_data:%x,%x adc_zero:%x,%x "
				"kfit_data:%x,%x lra_nml_time:%x,%x,%x "
				"lra_nml_beml:%x,%x\n", pmic_vibrator_happend_irq_name,
				out_nml_vbemf_l, out_nml_vbemf_h, ureal_data_l,
				ureal_data_h, adc_zero_l, adc_zero_h,
				kfit_data_l, kfit_data_h, lra_nml_time_l,
				lra_nml_time_m, lra_nml_time_h, lra_nml_beml_l,
				lra_nml_beml_h);
}

static void hisi_pmic_vibrator_dmd_notify(void)
{
	if (vib_dclient == NULL) {
		dev_err(g_vdev->dev, "there is not vib_dclient!\n");
		return;
	}
	if (dsm_client_ocuppy(vib_dclient)) {
		dev_err(g_vdev->dev, "buffer is busy!\n");
		return;
	}

	if (strncmp(pmic_vibrator_happend_irq_name, "ocp",
			 strlen("ocp")) == 0 &&
		(!(pmic_vibrator_dmd_flage &
			PMIC_VIBRATOR_DMD_IRQ_OCP_FLAGE))) {
		if (irq_ocp_time == PMIC_VIBRATOR_DMD_REPORT_NUM) {
			hisi_pmic_vibrator_dmd_log();
			dsm_client_notify(
				vib_dclient, DSM_PMIC_VIBRATOR_IRQ_OCP_NO);
			pmic_vibrator_dmd_flage |=
				PMIC_VIBRATOR_DMD_IRQ_OCP_FLAGE;
			irq_ocp_time = PMIC_VIBRATOR_DMD_CLEAR;
		} else {
			irq_ocp_time++;
		}
	} else if (strncmp(pmic_vibrator_happend_irq_name, "out",
				strlen("out")) == 0 &&
		   (!(pmic_vibrator_dmd_flage &
			   PMIC_VIBRATOR_DMD_IRQ_OUT_FLAGE))) {
		if (irq_out_time == PMIC_VIBRATOR_DMD_REPORT_NUM) {
			hisi_pmic_vibrator_dmd_log();
			dsm_client_notify(
				vib_dclient, DSM_PMIC_VIBRATOR_IRQ_OUT_NO);
			pmic_vibrator_dmd_flage |=
				PMIC_VIBRATOR_DMD_IRQ_OUT_FLAGE;
			irq_out_time = PMIC_VIBRATOR_DMD_CLEAR;
		} else {
			irq_out_time++;
		}
	} else if (strncmp(pmic_vibrator_happend_irq_name, "undervol",
				strlen("undervol")) == 0 &&
		   (!(pmic_vibrator_dmd_flage &
			   PMIC_VIBRATOR_DMD_IRQ_UNDERVOL_FLAGE))) {
		if (irq_undervol_time == PMIC_VIBRATOR_DMD_REPORT_NUM) {
			hisi_pmic_vibrator_dmd_log();
			dsm_client_notify(
				vib_dclient, DSM_PMIC_VIBRATOR_IRQ_UNDERVOL_NO);
			pmic_vibrator_dmd_flage |=
				PMIC_VIBRATOR_DMD_IRQ_UNDERVOL_FLAGE;
			irq_undervol_time = PMIC_VIBRATOR_DMD_CLEAR;
		} else {
			irq_undervol_time++;
		}
	} else if (strncmp(pmic_vibrator_happend_irq_name, "overvol",
				strlen("overvol")) == 0 &&
		   (!(pmic_vibrator_dmd_flage &
			   PMIC_VIBRATOR_DMD_IRQ_OVERVOL_FLAGE))) {
		if (irq_overvol_time == PMIC_VIBRATOR_DMD_REPORT_NUM) {
			hisi_pmic_vibrator_dmd_log();
			dsm_client_notify(
				vib_dclient, DSM_PMIC_VIBRATOR_IRQ_OVERVOL_NO);
			pmic_vibrator_dmd_flage |=
				PMIC_VIBRATOR_DMD_IRQ_OVERVOL_FLAGE;
			irq_overvol_time = PMIC_VIBRATOR_DMD_CLEAR;
		} else {
			irq_overvol_time++;
		}
	} else if (strncmp(pmic_vibrator_happend_irq_name, "adc",
				strlen("adc")) == 0 &&
		   (!(pmic_vibrator_dmd_flage &
			   PMIC_VIBRATOR_DMD_IRQ_ADC_FLAGE))) {
		if (irq_adc_time == PMIC_VIBRATOR_DMD_REPORT_NUM) {
			hisi_pmic_vibrator_dmd_log();
			dsm_client_notify(
				vib_dclient, DSM_PMIC_VIBRATOR_IRQ_ADC_NO);
			pmic_vibrator_dmd_flage |=
				PMIC_VIBRATOR_DMD_IRQ_ADC_FLAGE;
			irq_adc_time = PMIC_VIBRATOR_DMD_CLEAR;
		} else {
			irq_adc_time++;
		}
	} else {
		dev_err(g_vdev->dev, "there is no other irq or it has notified\n");
	}
	dsm_client_unocuppy(vib_dclient);
}
#endif


static void hisi_pmic_vibrator_irq_function(struct work_struct *work)
{
	struct led_classdev *led_cdev = &g_vdev->led_dev;
#ifdef CONFIG_HUAWEI_DSM
	hisi_pmic_vibrator_dmd_notify();
#endif
	led_set_brightness(led_cdev, LED_VIBRATOR_OFF);
}

static irqreturn_t hisi_pmic_vibrator_handler(int irq, void *data)
{
	struct hisi_pmic_vibrator_dev *vdev =
		(struct hisi_pmic_vibrator_dev *)data;
	u32 i;

	cancel_work_sync(&vdev->hisi_pmic_vibrator_irq_work);
	for (i = 0; i < HISI_PMIC_VIBRATOR_IRQ_COUNTS; i++) {
		if (irq == vdev->lra_irq[i].num) {
			dev_err(g_vdev->dev,
				"pmic vibrator interrupt happend[%s]\n",
				vdev->lra_irq[i].irq_name);
#ifdef CONFIG_HUAWEI_DSM
			if (strncpy_s(pmic_vibrator_happend_irq_name,
				sizeof(pmic_vibrator_happend_irq_name),
				vdev->lra_irq[i].irq_name,
				strlen(vdev->lra_irq[i].irq_name)) != 0) {
				dev_err(g_vdev->dev, "strncpy_s failed\n");
			}
#endif
			schedule_work(&vdev->hisi_pmic_vibrator_irq_work);

			goto irq_pending;
		} else {
			continue;
		}
	}

	dev_err(g_vdev->dev, "invalid irq %d!\n", irq);

irq_pending:
	return IRQ_HANDLED;
}

static s32 hisi_pmic_vibrator_interrupt_init(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;
	s32 i, ret;
	s8 *name = NULL;

	vdev = pdev->dev.driver_data;
	if (vdev == NULL)
		return -EINVAL;

	for (i = 0; i < HISI_PMIC_VIBRATOR_IRQ_COUNTS; i++) {
		name = hisi_pmic_vibrator_irq_type[i];
		vdev->lra_irq[i].num = spmi_get_irq_byname(pdev, NULL, name);
		if (vdev->lra_irq[i].num < 0) {
			dev_info(vdev->dev, "failed to get %s irq id\n", name);
		} else {
			ret = devm_request_irq(&pdev->dev, vdev->lra_irq[i].num,
				hisi_pmic_vibrator_handler, 0, name, vdev);
			if (ret < 0) {
				dev_err(vdev->dev, "failed to request %s irq\n",
					name);
			} else {
				if(strncpy_s(vdev->lra_irq[i].irq_name,
					sizeof(vdev->lra_irq[i].irq_name), name,
					strlen(name)) != 0) {
					dev_err(g_vdev->dev, "strncpy_s failed\n");
					return -EINVAL;
				}
			}
		}
	}

	return 0;
}

static s32 hisi_pmic_vibrator_parse_dt(struct hisi_pmic_vibrator_dev *vdev)
{
	struct device *dev = vdev->dev;
	const __be32 *mux = NULL;
	struct hisi_pmic_vibrator_haptics_lib *table = NULL;
	int ret, size, rows, temp = 0, params = 40;

	ret = of_property_read_u32(dev->of_node, "vibrator-boost-power", &temp);
	if (ret < 0) {
		dev_info(dev, "get gpio_en fail,not uesed 5v!\n");
	} else {
		vdev->pmic_vibrator_boost_power = temp;
		dev_info(dev, "boost_power is %d\n",
			vdev->pmic_vibrator_boost_power);
	}

	ret = of_property_read_u32(dev->of_node, "vibrator-voltage", &temp);
	if (ret < 0) {
		dev_err(dev, "get vibrator_vol fail\n");
		return -EINVAL;
	}

	vdev->pmic_vibrator_vol = temp;
	vdev->pmic_vibrator_strength = vdev->pmic_vibrator_vol;

	mux = of_get_property(dev->of_node, "haptics-cfg", &size);
	if (mux == NULL) {
		dev_info(dev, "could not support haptic lib\n");
		return 0;
	}

	if (size < (sizeof(*mux) * params)) {
		dev_err(dev, "haptic lib data is bad\n");
		return -EINVAL;
	}

	size /= sizeof(*mux); /* Number of elements in array */
	rows = size / params;

	dev_info(dev, "number of elements is %d, rows is %d\n", size, rows);

	table = devm_kzalloc(dev,
		sizeof(struct hisi_pmic_vibrator_haptics_lib) * rows,
		GFP_KERNEL);
	if (table == NULL) {
		dev_err(dev, "failed to allocate haptics cfg table\n");
		return -ENOMEM;
	}

	ret = of_property_read_u32_array(
		dev->of_node, "haptics-cfg", (u32 *)table, size);
	if (ret) {
		dev_err(dev, "could not read 'haptics-cfg' table\n");
		return ret;
	}

	vdev->haptics_counts = rows;
	vdev->haptics_lib = table;

	return 0;
}

static s32 hisi_pmic_vibrator_haptics_probe(struct hisi_pmic_vibrator_dev *vdev)
{
	s32 ret;

	vdev->version = MKDEV(0, 0);
	ret = alloc_chrdev_region(
		&vdev->version, 0, 1, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to alloc chrdev region, ret[%d]\n",
			ret);
		return ret;
	}

	vdev->class = class_create(THIS_MODULE, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (!vdev->class) {
		dev_err(vdev->dev, "failed to create class\n");
		ret = ENOMEM;
		goto unregister_cdev_region;
	}

	vdev->dev = device_create(vdev->class, NULL, vdev->version, NULL,
		HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (vdev->dev == NULL) {
		ret = ENOMEM;
		;
		dev_err(vdev->dev, "failed to create device\n");
		goto destory_class;
	}

	cdev_init(&vdev->cdev, &hisi_pmic_vibrator_fops);
	vdev->cdev.owner = THIS_MODULE;
	vdev->cdev.ops = &hisi_pmic_vibrator_fops;
	ret = cdev_add(&vdev->cdev, vdev->version, 1);
	if (ret) {
		dev_err(vdev->dev, "failed to add cdev\n");
		goto destory_device;
	}

	vdev->sw_dev.name = "haptics";
	ret = switch_dev_register(&vdev->sw_dev);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to register sw_dev\n");
		goto unregister_cdev;
	}

	dev_info(vdev->dev, "haptics setup ok\n");

	return 0;

unregister_cdev:
	cdev_del(&vdev->cdev);
destory_device:
	device_destroy(vdev->class, vdev->version);
destory_class:
	class_destroy(vdev->class);
unregister_cdev_region:
	unregister_chrdev_region(vdev->version, 1);
	return ret;
}

static void hisi_pmic_vibrator_haptics_remove(
	struct hisi_pmic_vibrator_dev *vdev)
{
	cdev_del(&vdev->cdev);
	device_destroy(vdev->class, vdev->version);
	class_destroy(vdev->class);
	unregister_chrdev_region(vdev->version, 1);
	switch_dev_unregister(&vdev->sw_dev);
}

static int hisi_pmic_vibrator_register_led_classdev(
	struct hisi_pmic_vibrator_dev *vdev)
{
	struct led_classdev *led_cdev = &vdev->led_dev;

	led_cdev->name = "vibrator";
	led_cdev->flags = LED_CORE_SUSPENDRESUME;
	led_cdev->brightness_set = hisi_pmic_vibrator_enable_ctrl;
	led_cdev->default_trigger = "transient";

	return devm_led_classdev_register(vdev->dev, led_cdev);
}

static int hisi_pmic_vibrator_probe(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;
	s32 ret;

	vdev = devm_kzalloc(
		&pdev->dev, sizeof(struct hisi_pmic_vibrator_dev), GFP_KERNEL);
	if (vdev == NULL)
		return -ENOMEM;

	vdev->dev = &pdev->dev;
	g_vdev = vdev;
	dev_set_drvdata(&pdev->dev, vdev);

	/* parse DT */
	ret = hisi_pmic_vibrator_parse_dt(vdev);
	if (ret) {
		dev_err(&pdev->dev, "DT parsing failed\n");
		return ret;
	}

	mutex_init(&vdev->lock);
	vdev->pmic_vibrator_correct_freq = LRA_DEFAULT_FREQ;
#ifdef CONFIG_HUAWEI_DSM
	if (vib_dclient == NULL)
		vib_dclient = dsm_register_client(&dsm_vibrator);
	pmic_vibrator_dmd_flage = PMIC_VIBRATOR_DMD_NO_FLAGE;
#endif

	INIT_WORK(&vdev->vibrator_enable_work, hisi_pmic_vibrator_enable_work);
	INIT_WORK(&vdev->hisi_pmic_vibrator_irq_work,
		hisi_pmic_vibrator_irq_function);
	INIT_WORK(&vdev->vibrator_off_work, hisi_pmic_vibrator_off_work);
	ret = hisi_pmic_vibrator_register_led_classdev(vdev);
	if (ret) {
		dev_err(&pdev->dev, "unable to register with timed_output\n");
		goto fail_register_led_classdev;
	}
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
	ret = sysfs_create_group(&vdev->led_dev.dev->kobj, &hisi_vb_attr_group);
	if (ret)
		dev_err(vdev->dev, "unable create vibrator's\n");
#endif
	ret = hisi_pmic_vibrator_haptics_probe(vdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to register haptics dev\n");
		goto haptics_fail_probe;
	}

	hisi_pmic_vibrator_set_mode(HISI_PMIC_VIBRATOR_MODE_STANDBY);

	/*init interrupts */
	ret = hisi_pmic_vibrator_interrupt_init(pdev);
	if (ret) {
		dev_err(&pdev->dev, "interrupts init failed,\n");
		return ret;
	}
	dev_info(&pdev->dev, "hisi_pmic_vibrator probe succeed\n");

	return 0;

haptics_fail_probe:
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
	sysfs_remove_group(&vdev->led_dev.dev->kobj, &hisi_vb_attr_group);
#endif
fail_register_led_classdev:
	cancel_work_sync(&vdev->vibrator_off_work);
	cancel_work_sync(&vdev->hisi_pmic_vibrator_irq_work);
	cancel_work_sync(&vdev->vibrator_enable_work);
	mutex_destroy(&vdev->lock);

	return ret;
}

static s32 hisi_pmic_vibrator_remove(struct spmi_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;

	vdev = dev_get_drvdata(&pdev->dev);
	if (vdev == NULL) {
		pr_err("%s:failed to get drvdata\n", __func__);
		return -ENODEV;
	}
	cancel_work_sync(&vdev->hisi_pmic_vibrator_irq_work);
	cancel_work_sync(&vdev->vibrator_off_work);
	cancel_work_sync(&vdev->vibrator_enable_work);

#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
	sysfs_remove_group(&vdev->led_dev.dev->kobj, &hisi_vb_attr_group);
#endif
	hisi_pmic_vibrator_haptics_remove(vdev);
	mutex_destroy(&vdev->lock);
	wakeup_source_trash(&vdev->wakelock);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static const struct of_device_id hisi_pmic_vibrator_match[] = {
	{
		.compatible = "hisilicon,pmic-vibrator",
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_pmic_vibrator_match);

static struct spmi_device_id pmic_vibrator_id[] = {
	{"hisilicon,pmic-vibrator", 0}, {},
};
static struct spmi_driver hisi_pmic_vibrator_driver = {
	.probe = hisi_pmic_vibrator_probe,
	.remove = hisi_pmic_vibrator_remove,
	.id_table = pmic_vibrator_id,
	.driver = {
			.name = "hisi-pmic-vibrator",
			.owner = THIS_MODULE,
			.of_match_table =
				of_match_ptr(hisi_pmic_vibrator_match),
		},
};

static int __init hisi_pmic_vibrator_init(void)
{

	return spmi_driver_register(&hisi_pmic_vibrator_driver);
}

static void __exit hisi_pmic_vibrator_exit(void)
{
	spmi_driver_unregister(&hisi_pmic_vibrator_driver);
}

module_init(hisi_pmic_vibrator_init);
module_exit(hisi_pmic_vibrator_exit);

MODULE_DESCRIPTION("HISI PMIC Vibrator driver");
MODULE_LICENSE("GPL");
