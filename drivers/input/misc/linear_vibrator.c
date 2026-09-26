/*
 * Linear vibrator driver for pmic vibrator.
 *
 * Copyright (c) 2017 Hisilicon Technologies CO.Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifdef CONFIG_HISI_HI6421V900_PMU
#include "linear_vibrator_hi6423.h"
#else
#include "linear_vibrator.h"
#endif
#include <securec.h>
#include <asm/irq.h>
#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/hisi-spmi.h>
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
#include <chipset_common/hwpower/hardware_ic/boost_5v.h>
#include <huawei_platform/sensor/hw_comm_pmic.h>
#endif

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#if defined(CONFIG_HISI_VIBRATOR)
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif

struct linear_vibrator_haptics_cfg {
	u32 ladd_para;
	u32 spel_duty;
};

struct linear_vibrator_haptics_lib {
	struct linear_vibrator_haptics_cfg
		cfg[LINEAR_VIBRATOR_REG_CFG_NUM];
};

struct linear_vibrator_irq {
	s32 num;
	s8 irq_name[16];
};

#ifdef CONFIG_HUAWEI_DSM

struct vibrator_dmd_table {
	char *irq_name;
	unsigned int dmd_flg;
	unsigned int dmd_no;
	unsigned int times;
	unsigned int happened;
};

struct dmd_timer {
	int lra_nml_time_l;
	int lra_nml_time_m;
	int lra_nml_time_h;
	int lra_nml_beml_l;
	int lra_nml_beml_h;
};

struct dmd_info {
	int out_nml_vbemf_l;
	int out_nml_vbemf_h;
	int ureal_data_l;
	int ureal_data_h;
	int adc_zero_l;
	int adc_zero_h;
	int kfit_data_l;
	int kfit_data_h;
	struct dmd_timer timer;
};

static struct dsm_dev dsm_vibrator = {
	.name = "dsm_vibrator",
	.device_name = "linear-vibrator",
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

static u32 linear_vibrator_dmd_flage;

#define LINEAR_VIBRATOR_DMD_NO_FLAGE 0x0
#define LINEAR_VIBRATOR_DMD_IRQ_OCP_FLAGE (0x1 << 0)
#define LINEAR_VIBRATOR_DMD_IRQ_OUT_FLAGE (0x1 << 1)
#define LINEAR_VIBRATOR_DMD_IRQ_UNDERVOL_FLAGE (0x1 << 2)
#define LINEAR_VIBRATOR_DMD_IRQ_OVERVOL_FLAGE (0x1 << 3)
#define LINEAR_VIBRATOR_DMD_IRQ_ADC_FLAGE (0x1 << 4)
#define LINEAR_VIBRATOR_DMD_FREQ_FLAGE (0x1 << 5)
#define LINEAR_VIBRATOR_DMD_REPORT_NUM 2
#define LINEAR_VIBRATOR_DMD_CLEAR 0
struct dsm_client *vib_dclient;
#endif

struct linear_vibrator_dev {
	struct device *dev;
	struct class *class;
	struct led_classdev led_dev;
	struct cdev cdev;
	struct switch_dev sw_dev;
	struct mutex lock;
	struct wakeup_source wakelock;
	dev_t version;
	struct linear_vibrator_haptics_lib *haptics_lib;
	struct linear_vibrator_irq lra_irq[LINEAR_VIBRATOR_IRQ_COUNTS];
	struct work_struct linear_vibrator_irq_work;
	struct work_struct vibrator_off_work;
	struct work_struct vibrator_enable_work;
	u32 haptics_counts;
	u32 linear_vibrator_boost_power;
	u32 linear_vibrator_freq;
	u32 linear_vibrator_correct_freq;
	u32 linear_vibrator_strength;
	u32 linear_vibrator_vol;
	u32 state;
	u32 linear_erm_vibrator;
	s8 name[32];
#ifdef CONFIG_BOOST_5V
	u32 boost_load_enable;
#endif
};

static struct linear_vibrator_dev *g_vdev;

static s8 linear_vibrator_irq_type[LINEAR_VIBRATOR_IRQ_COUNTS][16] = {
	"ocp", "out", "undervol", "overvol", "adc",
};

struct vib_weak_gear {
	int weak_id;
	unsigned int weak_strength;
};

struct vib_weak_gear vib_weak_res[] = {
	{LINEAR_WEAK_TYPE_DEFULT, LINEAR_STRENGTH_DEFULT},
	{LINEAR_WEAK_TYPE_1, LINEAR_WEAK_STRENGTH_1},
	{LINEAR_WEAK_TYPE_2, LINEAR_WEAK_STRENGTH_2},
};

#define MAX_INPUT_SIZE 63
#define HAPTICS_CFG_PARAMS 40

/* read register  */
static u8 linear_vibrator_read_u8(const u32 vibrator_address)
{
#ifdef CONFIG_HISI_HI6421V900_PMU
	return hisi_sub_pmic_reg_read(vibrator_address);
#else
	return pmic_read_reg(vibrator_address);
#endif
}

/* write register  */
static void linear_vibrator_write_u8(
	u8 vibrator_set, const u32 vibrator_address)
{
#ifdef CONFIG_HISI_HI6421V900_PMU
	hisi_sub_pmic_reg_write(vibrator_address, vibrator_set);
#else
	pmic_write_reg(vibrator_address, vibrator_set);
#endif
}

static void linear_vibrator_set_mode(u8 mode)
{
	linear_vibrator_write_u8(mode, LINEAR_VIBRATOR_ON);
}

#ifdef CONFIG_HISI_HI6421V900_PMU
static void linear_vibrator_set_clock(int pmic_vibrator_clk_onoff)
{
	pmic_write_reg(LINEAR_VIBRATOR_CLK_GATE,
		pmic_vibrator_clk_onoff);
	if (pmic_vibrator_clk_onoff)
		hisi_sub_pmic_reg_write(LINEAR_VIBRATOR_LRA_ANA_CFG2,
			LINEAR_VIBRATOR_LRA_ANA_VALUE);
}
#endif

static int linear_vibrator_power_on_off(int pmic_vibrator_power_onoff)
{
	int retry = LINEAR_VIBRATOR_BRAKE_TIME_OUT;

	if (g_vdev->linear_vibrator_boost_power) {
		if (pmic_vibrator_power_onoff) {
#ifdef CONFIG_BOOST_5V
			if (g_vdev->boost_load_enable)
				set_boost_load_enable_cfg(VIB_PMIC_REQ,
					0, LINEAR_VIBRATOR_POWER_ON);
			boost_5v_enable(BOOST_5V_ENABLE, BOOST_CTRL_MOTOER);
			mdelay(2);
			if (g_vdev->boost_load_enable) {
				set_force_pwm_mode_cfg(VIB_PMIC_REQ,
					0, LINEAR_VIBRATOR_POWER_ON);
				dev_info(g_vdev->dev, "force pwm mode\n");
			}
#else
			mdelay(2);
			dev_info(g_vdev->dev, "boost 5v on\n");
#endif
		} else {
			while (linear_vibrator_read_u8(
				LINEAR_VIBRATOR_STATUS) && retry) {
				mdelay(1);
				retry--;
			}
			dev_info(g_vdev->dev,
				"vibraor status is 0x%x,retry is %d\n",
				linear_vibrator_read_u8(
				LINEAR_VIBRATOR_STATUS), retry);
#ifdef CONFIG_HISI_HI6421V900_PMU
			linear_vibrator_set_clock(pmic_vibrator_power_onoff);
#endif
#ifdef CONFIG_BOOST_5V
			if (g_vdev->boost_load_enable) {
				set_force_pwm_mode_cfg(VIB_PMIC_REQ,
					0, LINEAR_VIBRATOR_POWER_OFF);
				mdelay(2);
				set_boost_load_enable_cfg(VIB_PMIC_REQ,
					0, LINEAR_VIBRATOR_POWER_OFF);
			}
			boost_5v_enable(BOOST_5V_DISABLE, BOOST_CTRL_MOTOER);
#endif
			mdelay(5);
			dev_info(g_vdev->dev, "boost 5v off\n");
		}
	}
	return 0;
}

static void linear_vibrator_get_freq(void)
{
	unsigned int lra_nml_time_l;
	unsigned int lra_nml_time_m;
	unsigned int lra_nml_time_h;

	lra_nml_time_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_TIME_L);
	lra_nml_time_m = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_TIME_M);
	lra_nml_time_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_TIME_H);

	g_vdev->linear_vibrator_freq = (lra_nml_time_h << 16) |
		(lra_nml_time_m << 8) | lra_nml_time_l;
	dev_info(g_vdev->dev, "frep is 0x%x\n", g_vdev->linear_vibrator_freq);
}

static void linear_vibrator_freq_dmd(struct linear_vibrator_dev *vdev)
{
#ifdef CONFIG_HUAWEI_DSM
	linear_vibrator_get_freq();
	if ((vdev->linear_vibrator_freq > LINEAR_VIBRATOR_FREQ_MAX) ||
		(vdev->linear_vibrator_freq < LINEAR_VIBRATOR_FREQ_MIN)) {
		dev_info(vdev->dev,
			"this time will notify dsm_vibator_freq:0x%x\n",
			vdev->linear_vibrator_freq);
		if (!dsm_client_ocuppy(vib_dclient)) {
			if (!(linear_vibrator_dmd_flage &
				LINEAR_VIBRATOR_DMD_FREQ_FLAGE)) {
				dsm_client_record(vib_dclient,
					"PMU LRA driver frequency detect abnormal. 0x%x\n",
					vdev->linear_vibrator_freq);
				dsm_client_notify(vib_dclient,
					DSM_PMIC_VIBRATOR_FREQ_CHECK_NO);
				linear_vibrator_dmd_flage |=
					LINEAR_VIBRATOR_DMD_FREQ_FLAGE;
			} else {
				dsm_client_unocuppy(vib_dclient);
			}
		}
	} else {
		vdev->linear_vibrator_correct_freq = vdev->linear_vibrator_freq;
	}
#endif
}

static void linear_vibrator_off(struct linear_vibrator_dev *vdev)
{
	if (!vdev->linear_erm_vibrator)
		linear_vibrator_freq_dmd(vdev);

	linear_vibrator_write_u8(LINEAR_VIBRATOR_BRK_EN,
		LINEAR_VIBRATOR_BRAKE);
	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_OFF);
}

static void linear_vibrator_erm_setup(void)
{
	uint64_t val = 0;
	u32 duty_l, duty_h;

	dev_info(g_vdev->dev, "linear_vibrator strength  is %d.\n",
		g_vdev->linear_vibrator_strength);

	/* erm unlock */
	linear_vibrator_write_u8(ERM_UNLOCK, LINEAR_VIBRATOR_ERM_LOCK);
	/* erm mode set */
	linear_vibrator_write_u8(ERM_MODE_NO_SAMPL,
		LINEAR_VIBRATOR_ERM_MODE_CFG);
	/* erm pwm set */
	linear_vibrator_write_u8(ERM_PWM_600K,
		LINEAR_VIBRATOR_FRQC_SEL_CFG);
	/* erm normal duty */
	val = (uint64_t)g_vdev->linear_vibrator_strength *
		ERM_DUTY_NUM / ERM_VOLTAGE_IN;
	if (val > ERM_DUTY_NUM)
		val = ERM_DUTY_NUM;

	duty_l = (val & 0xf) << 4; /* low 8 bit */
	duty_h = val >> 4; /* high 8 bit */
	linear_vibrator_write_u8(duty_l,
		LINEAR_VIBRATOR_ERM_DUTY_NML_CFG_L);
	linear_vibrator_write_u8(duty_h,
		LINEAR_VIBRATOR_ERM_DUTY_NML_CFG_H);
	/* overdrv duty */
	linear_vibrator_write_u8(ERM_OVDR_VAL_L,
		LINEAR_VIBRATOR_ERM_DUTY_OVDR_CFG_L);
	linear_vibrator_write_u8(ERM_OVDR_VAL_H,
		LINEAR_VIBRATOR_ERM_DUTY_OVDR_CFG_H);
	/* break duty */
	linear_vibrator_write_u8(duty_l,
		LINEAR_VIBRATOR_ERM_DUTY_BRK_CFG_L);
	linear_vibrator_write_u8(duty_h,
		LINEAR_VIBRATOR_ERM_DUTY_BRK_CFG_H);
}

static void linear_vibrator_set_gate(void)
{
	// gate1
	linear_vibrator_write_u8(LINEAR_VIBRATOR_GATE_VOLTAGE1_VALUE_L,
		LINEAR_VIBRATOR_GATE_VOLTAGE1_L);
	linear_vibrator_write_u8(LINEAR_VIBRATOR_GATE_VOLTAGE1_VALUE_H,
		LINEAR_VIBRATOR_GATE_VOLTAGE1_H);
	// gate2
	linear_vibrator_write_u8(LINEAR_VIBRATOR_GATE_VOLTAGE2_VALUE_L,
		LINEAR_VIBRATOR_GATE_VOLTAGE2_L);
	linear_vibrator_write_u8(LINEAR_VIBRATOR_GATE_VOLTAGE2_VALUE_H,
		LINEAR_VIBRATOR_GATE_VOLTAGE2_H);
	// gate3
	linear_vibrator_write_u8(LINEAR_VIBRATOR_GATE_VOLTAGE3_VALUE_L,
		LINEAR_VIBRATOR_GATE_VOLTAGE3_L);
	linear_vibrator_write_u8(LINEAR_VIBRATOR_GATE_VOLTAGE3_VALUE_L,
		LINEAR_VIBRATOR_GATE_VOLTAGE3_H);
}

static s32 linear_vibrator_set_strength(void)
{
	uint64_t val;
	u32 duty_l, duty_h;
	u32 ovdr_l, ovdr_h;
	u32 boost_l, boost_h;
	u32 brake_l, brake_h;

	dev_info(g_vdev->dev, "linear_vibrator strength  is %d.\n",
		g_vdev->linear_vibrator_strength);
	// normal duty
	val = (uint64_t)g_vdev->linear_vibrator_strength * DUTY_NUM /
	      VOLTAGE_NUM;
	if (val > DUTY_NUM)
		val = DUTY_NUM;

	duty_l = (val & 0xf) << 4; // low 8 bit
	duty_h = val >> 4;         // high 8 bit
	linear_vibrator_write_u8(
		duty_l, LINEAR_VIBRATOR_DUTY_NORMAL_CFG_L);
	linear_vibrator_write_u8(
		duty_h, LINEAR_VIBRATOR_DUTY_NORMAL_CFG_H);
	// ovdr duty
	val = ((uint64_t)g_vdev->linear_vibrator_strength * OVDR_DUTY_A +
		OVDR_DUTY_B) / 1000000;
	ovdr_l = duty_l; //(val & 0xf) << 4,low 8 bit
	ovdr_h = duty_h; // val >> 4,high 8 bit
	linear_vibrator_write_u8(ovdr_l, LINEAR_VIBRATOR_OVDR_DUTY_L);
	linear_vibrator_write_u8(ovdr_h, LINEAR_VIBRATOR_OVDR_DUTY_H);
	// boost
	val = ((uint64_t)g_vdev->linear_vibrator_strength *
		TH_BOOST_A + TH_BOOST_B) / 1000000;
	boost_l = val & 0xff; // low 8 bit
	boost_h = val >> 8;   // high 8 bit
	linear_vibrator_write_u8(boost_l, LINEAR_VIBRATOR_TH_BOOST_L);
	linear_vibrator_write_u8(boost_h, LINEAR_VIBRATOR_TH_BOOST_H);
	// drv num
	linear_vibrator_write_u8(
		LINEAR_VIBRATOR_DRV_NUM_VALUE, LINEAR_VIBRATOR_DRV_NMU);
	// brake duty
	val = ((uint64_t)g_vdev->linear_vibrator_strength * BRAKE_DUTY_A -
		BRAKE_DUTY_B) / 1000000;
	if (val > DUTY_NUM)
		val = DUTY_NUM;

	brake_l = (val & 0xf) << 4; // low 8 bit
	brake_h = val >> 4;         // high 8 bit
	linear_vibrator_write_u8(brake_l, LINEAR_VIBRATOR_BRAKE_DUTY_L);
	linear_vibrator_write_u8(brake_h, LINEAR_VIBRATOR_BRAKE_DUTY_H);

	linear_vibrator_set_gate();

	return 0;
}

static void linear_vibrator_off_work(struct work_struct *work)
{
	linear_vibrator_off(g_vdev);
}

static void linear_vibrator_enable(struct linear_vibrator_dev *vdev)

{
	if (vdev == NULL)
		return;

	mutex_lock(&vdev->lock);

	if (vdev->state) {
		linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_ON);
		if (vdev->linear_erm_vibrator)
			linear_vibrator_erm_setup();
		else
			linear_vibrator_set_strength();
		linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_STANDBY);
		linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_RTP);
		vibrator_shake = 1;
		dev_info(vdev->dev, "linear_vibrator_RTP is running\n");
	} else {
		linear_vibrator_off(vdev);
		vibrator_shake = 0;
	}

	mutex_unlock(&vdev->lock);
}

static void linear_vibrator_enable_work(struct work_struct *work)
{
#ifdef CONFIG_HISI_HI6421V900_PMU
	linear_vibrator_set_clock(LINEAR_VIBRATOR_POWER_ON);
#endif
	linear_vibrator_enable(g_vdev);
}

static void linear_vibrator_enable_ctrl(
	struct led_classdev *led_dev, enum led_brightness state)
{
	struct linear_vibrator_dev *vdev =
		container_of(led_dev, struct linear_vibrator_dev, led_dev);

	vdev->state = state;
	schedule_work(&vdev->vibrator_enable_work);
}

static void haptics_num_set(u32 type)
{
	switch (type) {
	case LINEAR_VIBRATOR_HAPTIC_1:
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE2_6,
			LINEAR_VIBRATOR_SPEL_TIME_REG2);
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE3_6,
			LINEAR_VIBRATOR_SPEL_TIME_REG3);
		dev_info(g_vdev->dev, "%s 6 cycle\n", __func__);
		break;
	case LINEAR_VIBRATOR_HAPTIC_2:
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE3_8,
			LINEAR_VIBRATOR_SPEL_TIME_REG3);
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE4_8,
			LINEAR_VIBRATOR_SPEL_TIME_REG4);
		dev_info(g_vdev->dev, "%s 8 cycle\n",  __func__);
		break;
	case LINEAR_VIBRATOR_HAPTIC_3:
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE4_10,
			LINEAR_VIBRATOR_SPEL_TIME_REG4);
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE5_10,
			LINEAR_VIBRATOR_SPEL_TIME_REG5);
		dev_info(g_vdev->dev, "%s 10 cycle\n",  __func__);
		break;
	case LINEAR_VIBRATOR_HAPTIC_4:
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE2_6,
			LINEAR_VIBRATOR_SPEL_TIME_REG2);
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE3_6,
			LINEAR_VIBRATOR_SPEL_TIME_REG3);
		dev_info(g_vdev->dev, "%s 4 cycle\n",  __func__);
		break;
	case LINEAR_VIBRATOR_HAPTIC_5:
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE2_6,
			LINEAR_VIBRATOR_SPEL_TIME_REG2);
		linear_vibrator_write_u8(
			LINEAR_VIBRATOR_SPEL_TIME_VALUE3_6,
			LINEAR_VIBRATOR_SPEL_TIME_REG3);
		dev_info(g_vdev->dev, "%s 5 cycle\n",  __func__);
		break;
	default:
		dev_err(g_vdev->dev, "input val is error!");
	}
}

static s32 linear_vibrator_haptic_cfg(
	struct linear_vibrator_dev *vdev, u32 type)
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
	linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_STANDBY);

	for (i = 0; i < LINEAR_VIBRATOR_REG_CFG_NUM; i++) {
		ladd_reg = LINEAR_VIBRATOR_LADD_PARA_REG + i;
		duty_reg = LINEAR_VIBRATOR_SPEL_DUTY_REG + i;

		/* configs time register and duty registers */
		linear_vibrator_write_u8(
			vdev->haptics_lib[idx].cfg[i].ladd_para, ladd_reg);
		linear_vibrator_write_u8(
			vdev->haptics_lib[idx].cfg[i].spel_duty, duty_reg);
	}

	linear_vibrator_get_freq();
	if ((vdev->linear_vibrator_freq > LINEAR_VIBRATOR_FREQ_MAX) ||
		(vdev->linear_vibrator_freq < LINEAR_VIBRATOR_FREQ_MIN))
		vdev->linear_vibrator_freq = vdev->linear_vibrator_correct_freq;

	val = vdev->linear_vibrator_freq * LRA_NML_TIME / LRA_CYCLE_NUM /
	      LRA_TIME_NUM;
	val1 = vdev->linear_vibrator_freq * LRA_NML_TIME / LRA_CYCLE_NUM /
	       LRA_TIME_NUM1;
	ret = val1 - val * LRA_HAPTIC_NUM;
	if (ret > LRA_HAPTIC_NUM1)
		val++;

	for (i = 0; i < LINEAR_VIBRATOR_HAPTIC_NUM; i++) {
		time_reg0 = LINEAR_VIBRATOR_SPEL_TIME_REG0 + i;
		linear_vibrator_write_u8(val, time_reg0);
	}

	haptics_num_set(type);
	dev_info(vdev->dev, "%s complete\n",  __func__);

	return 0;
}

static ssize_t linear_vibrator_haptics_write(
	struct file *filp, const char *buff, size_t len, loff_t *off)
{
	struct linear_vibrator_dev *vdev = NULL;
	uint64_t type;
	s32 ret;
	char write_buf[LINEAR_VIBRATOR_HAP_BUF_LENGTH] = {0};

	if (filp == NULL)
		return 0;

	vdev = (struct linear_vibrator_dev *)filp->private_data;

	mutex_lock(&vdev->lock);
	vibrator_shake = 1;

	if ((len > LINEAR_VIBRATOR_HAP_BUF_LENGTH - 1) || buff == NULL) {
		dev_info(g_vdev->dev, "linear vibrator haptic buf is bad\n");
		goto out;
	}

	if (copy_from_user(write_buf, buff, len)) {
		dev_info(g_vdev->dev,
			"linear vibrator haptics copy from user failed\n");
		goto out;
	}

	if (kstrtoull(write_buf, 10, &type)) {
		dev_info(g_vdev->dev,
			"linear vibrator haptics read value error\n");
		goto out;
	}
#ifdef CONFIG_HISI_HI6421V900_PMU
	linear_vibrator_set_clock(LINEAR_VIBRATOR_POWER_ON);
#endif
	ret = linear_vibrator_haptic_cfg(vdev, (u32)type);
	if (ret < 0) {
		dev_info(g_vdev->dev, "linear vibrator haptics cfg fail\n");
		goto out;
	}

	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_ON);
	linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_HAPTICS);
	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_OFF);
out:
	vibrator_shake = 0;
	mutex_unlock(&vdev->lock);

	return len;
}

void linear_vibrator_haptics_set_type(int type)
{
	s32 ret;

	mutex_lock(&g_vdev->lock);

#ifdef CONFIG_HISI_HI6421V900_PMU
	linear_vibrator_set_clock(LINEAR_VIBRATOR_POWER_ON);
#endif
	ret = linear_vibrator_haptic_cfg(g_vdev, (u32)type);
	if (ret) {
		dev_err(g_vdev->dev, "haptic_test error\n");
		mutex_unlock(&g_vdev->lock);
		return;
	}

	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_ON);
	linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_HAPTICS);
	dev_info(g_vdev->dev, "%s is running\n", __func__);
	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_OFF);

	mutex_unlock(&g_vdev->lock);
}
EXPORT_SYMBOL(linear_vibrator_haptics_set_type);

static ssize_t linear_vibrator_min_timeout_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 val = LINEAR_VIBRATOR_TIMEOUT_MIN;

	UNUSED(dev);
	UNUSED(attr);
	return snprintf_s(buf, VIBRATOR_TIMEOUT_MIN_SIZE,
		VIBRATOR_TIMEOUT_MIN_SIZE - 1, "%d\n", val);
}

static ssize_t linear_vibrator_erm_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u32 val = 1;

	UNUSED(dev);
	UNUSED(attr);
	return snprintf_s(buf, VIBRATOR_TIMEOUT_MIN_SIZE,
		VIBRATOR_TIMEOUT_MIN_SIZE - 1, "%d\n", val);
}

static ssize_t linear_vibrator_duty_set_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int64_t value = 0;
	u32 i;

	UNUSED(dev);
	UNUSED(attr);
	if ((count > MAX_INPUT_SIZE) || (kstrtoll(buf, 10, &value))) {
		dev_err(g_vdev->dev, "vibrator voltage read value error\n");
		return count;
	}

	dev_info(g_vdev->dev, "%s input value is %lld\n", __func__, value);
	for (i = 0; i < ARRAY_SIZE(vib_weak_res); i++) {
		if (value == vib_weak_res[i].weak_id) {
			g_vdev->linear_vibrator_strength =
				vib_weak_res[i].weak_strength;
			break;
		}
	}

	return count;
}

static DEVICE_ATTR(vibrator_min_timeout, 0644, linear_vibrator_min_timeout_show,
	NULL);
static DEVICE_ATTR(vibrator_erm, 0644, linear_vibrator_erm_show, NULL);
static DEVICE_ATTR(set_amplitude, 0644, NULL, linear_vibrator_duty_set_store);

static struct attribute *linear_erm_vb_attributes[] = {
	&dev_attr_vibrator_min_timeout.attr,
	&dev_attr_vibrator_erm.attr,
	NULL
};
static const struct attribute_group linear_erm_vb_attr_group = {
	.attrs = linear_erm_vb_attributes,
};

static struct attribute *linear_vb_duty_set_attributes[] = {
	&dev_attr_set_amplitude.attr,
	NULL
};

static const struct attribute_group linear_duty_attr_group = {
	.attrs = linear_vb_duty_set_attributes,
};

#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG

/* calc value of voltage */
static s32 linear_vibrator_set_rtp_val(
	struct linear_vibrator_dev *vdev, u32 voltage_level)
{
	if (vdev == NULL)
		return -EINVAL;

	if (voltage_level < LINEAR_VIBRATOR_VOL_LEVEL_1 ||
		voltage_level > LINEAR_VIBRATOR_VOL_LEVEL_16) {
		dev_err(vdev->dev, "vibrator voltage level is invalid!\n");
		return -EINVAL;
	}

	if (voltage_level <= LINEAR_VIBRATOR_VOL_LEVEL_10)
		vdev->linear_vibrator_strength = voltage_level *
			LINEAR_VIBRATOR_STRENGTH_STEP1;
	else
		vdev->linear_vibrator_strength = (voltage_level -
			LINEAR_VIBRATOR_VOL_LEVEL_10) *
			LINEAR_VIBRATOR_STRENGTH_STEP2 +
			LINEAR_VIBRATOR_VOL_LEVEL_10 *
			LINEAR_VIBRATOR_STRENGTH_STEP1;
	return 0;
}

static ssize_t linear_vibrator_voltage_change_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t value = 0;
	s32 ret;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, 10, &value))) {
		dev_err(g_vdev->dev, "vibrator voltage read value error\n");
		return count;
	}
	mutex_lock(&g_vdev->lock);
	ret = linear_vibrator_set_rtp_val(g_vdev, value);
	if (ret < 0)
		dev_err(g_vdev->dev, "linear_vibrator set rtp val faild\n");
	mutex_unlock(&g_vdev->lock);

	return count;
}

static ssize_t linear_vibrator_change_mode_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	uint64_t val = 0;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, 10, &val))) {
		dev_err(g_vdev->dev, "invaild val\n");
		return -EINVAL;
	}

	mutex_lock(&g_vdev->lock);
	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_ON);

	switch (val) {
	case LINEAR_VIBRATOR_MODE_STANDBY:
		linear_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "linear_vibrator mode is standby\n");
		break;
	case LINEAR_VIBRATOR_MODE_RTP:
		linear_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "linear_vibrator mode is rtp\n");
		break;
	case LINEAR_VIBRATOR_MODE_HAPTICS:
		linear_vibrator_set_mode(val);
		dev_info(g_vdev->dev, "linear_vibrator mode is haptics\n");
		break;
	default:
		dev_err(g_vdev->dev, "input val is error!\n");
	}

	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_OFF);
	mutex_unlock(&g_vdev->lock);

	return count;
}

static ssize_t lineari_haptic_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	s32 ret;
	uint64_t val = 0;

	if ((count > MAX_INPUT_SIZE) || (kstrtoull(buf, 10, &val))) {
		dev_err(g_vdev->dev, "invaild parameters\n");
		return -EINVAL;
	}

	mutex_lock(&g_vdev->lock);
#ifdef CONFIG_HISI_HI6421V900_PMU
	linear_vibrator_set_clock(LINEAR_VIBRATOR_POWER_ON);
#endif
	ret = linear_vibrator_haptic_cfg(g_vdev, val);
	if (ret) {
		dev_err(g_vdev->dev, "linear_haptic_test error\n");
		mutex_unlock(&g_vdev->lock);
		return -EINVAL;
	}

	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_ON);
	linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_HAPTICS);
	dev_info(g_vdev->dev, "linear_vibrator_haptic is running\n");
	linear_vibrator_power_on_off(LINEAR_VIBRATOR_POWER_OFF);

	mutex_unlock(&g_vdev->lock);

	return count;
}

static DEVICE_ATTR(linear_vibrator_voltage_change, 0600, NULL,
	linear_vibrator_voltage_change_store);
static DEVICE_ATTR(vibrator_change_mode_test, 0600, NULL,
	linear_vibrator_change_mode_test_store);
static DEVICE_ATTR(linear_haptic_test, 0600, NULL, lineari_haptic_test_store);

static struct attribute *linear_vb_attributes[] = {
	&dev_attr_linear_vibrator_voltage_change.attr,
	&dev_attr_vibrator_change_mode_test.attr,
	&dev_attr_linear_haptic_test.attr,
	NULL
};

static const struct attribute_group linear_vb_attr_group = {
	.attrs = linear_vb_attributes,
};

#endif

static s32 linear_vibrator_haptics_open(struct inode *i_node,
	struct file *filp)
{
	if (filp == NULL)
		return -1;
	filp->private_data = g_vdev;
	pr_err("%s:haptics open\n", __func__);
	return 0;
}

static const struct file_operations linear_vibrator_fops = {
	.open = linear_vibrator_haptics_open,
	.write = linear_vibrator_haptics_write,
};

#ifdef CONFIG_HUAWEI_DSM

struct vibrator_dmd_table g_vib_dmd_table[] = {
	{ "ocp", LINEAR_VIBRATOR_DMD_IRQ_OCP_FLAGE,
		DSM_PMIC_VIBRATOR_IRQ_OCP_NO, 0, 0},
	{ "out", LINEAR_VIBRATOR_DMD_IRQ_OUT_FLAGE,
		DSM_PMIC_VIBRATOR_IRQ_OUT_NO, 0, 0},
	{ "undervol", LINEAR_VIBRATOR_DMD_IRQ_UNDERVOL_FLAGE,
		DSM_PMIC_VIBRATOR_IRQ_UNDERVOL_NO, 0, 0},
	{ "overvol", LINEAR_VIBRATOR_DMD_IRQ_OVERVOL_FLAGE,
		DSM_PMIC_VIBRATOR_IRQ_OVERVOL_NO, 0, 0},
	{ "adc", LINEAR_VIBRATOR_DMD_IRQ_ADC_FLAGE,
		DSM_PMIC_VIBRATOR_IRQ_ADC_NO, 0, 0}
};

static void linear_vibrator_dmd_log(struct vibrator_dmd_table *dmd_cfg)
{
	struct dmd_info dmd;

	if (g_vdev->linear_erm_vibrator) {
		dsm_client_record(vib_dclient, "PMU LRA driver irq: %s!\n",
			dmd_cfg->irq_name);
		return;
	}

	dmd.out_nml_vbemf_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_OUT_NML_VBEMF_L);
	dmd.out_nml_vbemf_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_OUT_NML_VBEMF_H);
	dmd.ureal_data_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_UREAL_DATA_L);
	dmd.ureal_data_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_UREAL_DATA_H);
	dmd.adc_zero_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_ADC_ZERO_L);
	dmd.adc_zero_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_ADC_ZERO_H);
	dmd.kfit_data_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_KFIT_DATA_L);
	dmd.kfit_data_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_KFIT_DATA_H);
	dmd.timer.lra_nml_time_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_TIME_L);
	dmd.timer.lra_nml_time_m = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_TIME_M);
	dmd.timer.lra_nml_time_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_TIME_H);
	dmd.timer.lra_nml_beml_l = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_BEML_L);
	dmd.timer.lra_nml_beml_h = linear_vibrator_read_u8(
		LINEAR_VIBRATOR_LRA_NML_BEML_H);

	dsm_client_record(vib_dclient,
		"PMU LRA driver irq: %s! out_nml_vbemf:%x,%x ureal_data:%x,%x adc_zero:%x,%x "
		"kfit_data:%x,%x lra_nml_time:%x,%x,%x lra_nml_beml:%x,%x\n",
		dmd_cfg->irq_name, dmd.out_nml_vbemf_l, dmd.out_nml_vbemf_h,
		dmd.ureal_data_l, dmd.ureal_data_h, dmd.adc_zero_l,
		dmd.adc_zero_h, dmd.kfit_data_l, dmd.kfit_data_h,
		dmd.timer.lra_nml_time_l, dmd.timer.lra_nml_time_m,
		dmd.timer.lra_nml_time_h, dmd.timer.lra_nml_beml_l,
		dmd.timer.lra_nml_beml_h);
}

static void linear_vibrator_dmd_handle(struct vibrator_dmd_table *dmd_cfg)
{
	if (dmd_cfg->times == LINEAR_VIBRATOR_DMD_REPORT_NUM) {
		linear_vibrator_dmd_log(dmd_cfg);
		dsm_client_notify(vib_dclient, dmd_cfg->dmd_no);
		linear_vibrator_dmd_flage |= dmd_cfg->dmd_flg;
		dmd_cfg->times = LINEAR_VIBRATOR_DMD_CLEAR;
	} else {
		dmd_cfg->times++;
	}
}

static void linear_vibrator_irq_dmd_match(const char *irq_name)
{
	unsigned int i;
	struct vibrator_dmd_table *dmd_cfg = NULL;

	for (i = 0; i < ARRAY_SIZE(g_vib_dmd_table); i++) {
		dmd_cfg = &g_vib_dmd_table[i];
		if (!strncmp(irq_name, dmd_cfg->irq_name, strlen(irq_name))) {
			dmd_cfg->happened = 1;
			return;
		}
	}
}

static void linear_vibrator_dmd_notify(void)
{
	unsigned int i;
	struct vibrator_dmd_table *dmd_cfg = NULL;

	if (vib_dclient == NULL) {
		dev_err(g_vdev->dev, "there is not vib_dclient!\n");
		return;
	}

	if (dsm_client_ocuppy(vib_dclient)) {
		dev_err(g_vdev->dev, "buffer is busy!\n");
		return;
	}

	for (i = 0; i < ARRAY_SIZE(g_vib_dmd_table); i++) {
		dmd_cfg = &g_vib_dmd_table[i];
		if (dmd_cfg->happened &&
			!(linear_vibrator_dmd_flage & dmd_cfg->dmd_flg))
			linear_vibrator_dmd_handle(dmd_cfg);
		dmd_cfg->happened = 0;
	}

	if (i >= ARRAY_SIZE(g_vib_dmd_table))
		dev_err(g_vdev->dev,
			"there is no other irq or it has notified\n");

	dsm_client_unocuppy(vib_dclient);
}
#endif

static void linear_vibrator_irq_function(struct work_struct *work)
{
	struct led_classdev *led_cdev = &g_vdev->led_dev;
#ifdef CONFIG_HUAWEI_DSM
	linear_vibrator_dmd_notify();
#endif
	led_set_brightness(led_cdev, LED_VIBRATOR_OFF);
}

static irqreturn_t linear_vibrator_handler(int irq, void *data)
{
	struct linear_vibrator_dev *vdev =
		(struct linear_vibrator_dev *)data;
	u32 i;

	for (i = 0; i < LINEAR_VIBRATOR_IRQ_COUNTS; i++) {
		if (irq == vdev->lra_irq[i].num) {
			dev_err(g_vdev->dev,
				"linear vibrator interrupt happend[%s]\n",
				vdev->lra_irq[i].irq_name);
#ifdef CONFIG_HUAWEI_DSM
			linear_vibrator_irq_dmd_match(
				vdev->lra_irq[i].irq_name);
#endif
			schedule_work(&vdev->linear_vibrator_irq_work);

			goto irq_pending;
		} else {
			continue;
		}
	}

	dev_err(g_vdev->dev, "invalid irq %d!\n", irq);

irq_pending:
	return IRQ_HANDLED;
}

static s32 linear_vibrator_interrupt_init(struct spmi_device *pdev)
{
	struct linear_vibrator_dev *vdev = NULL;
	s32 i, ret;
	s8 *name = NULL;

	vdev = pdev->dev.driver_data;
	if (vdev == NULL)
		return -EINVAL;

	for (i = 0; i < LINEAR_VIBRATOR_IRQ_COUNTS; i++) {
		name = linear_vibrator_irq_type[i];
		vdev->lra_irq[i].num = spmi_get_irq_byname(pdev, NULL, name);
		if (vdev->lra_irq[i].num < 0) {
			dev_info(vdev->dev, "failed to get %s irq id\n", name);
		} else {
			ret = devm_request_irq(&pdev->dev, vdev->lra_irq[i].num,
				linear_vibrator_handler, 0, name, vdev);
			if (ret < 0) {
				dev_err(vdev->dev, "failed to request %s irq\n",
					name);
			} else {
				if (strncpy_s(vdev->lra_irq[i].irq_name,
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

static s32 linear_vibrator_haptics_cfg(struct linear_vibrator_dev *vdev)
{
	struct device *dev = vdev->dev;
	const __be32 *mux = NULL;
	struct linear_vibrator_haptics_lib *table = NULL;
	int ret;
	unsigned int size, rows;

	mux = of_get_property(dev->of_node, "haptics-cfg", &size);
	if (mux == NULL) {
		dev_info(dev, "could not support haptic lib\n");
		return 0;
	}

	if (size < (sizeof(*mux) * HAPTICS_CFG_PARAMS)) {
		dev_err(dev, "haptic lib data is bad\n");
		return -EINVAL;
	}

	size /= sizeof(*mux); /* Number of elements in array */
	rows = size / HAPTICS_CFG_PARAMS;

	dev_info(dev, "number of elements is %d, rows is %d\n", size, rows);

	table = devm_kzalloc(dev,
		sizeof(struct linear_vibrator_haptics_lib) * rows,
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

static s32 linear_vibrator_parse_dt(struct linear_vibrator_dev *vdev)
{
	struct device *dev = vdev->dev;
	unsigned int temp = 0;
	int ret;

	ret = of_property_read_u32(dev->of_node, "vibrator-boost-power", &temp);
	if (ret < 0) {
		dev_info(dev, "get gpio_en fail,not uesed 5v!\n");
	} else {
		vdev->linear_vibrator_boost_power = temp;
		dev_info(dev, "boost_power is %d\n",
			vdev->linear_vibrator_boost_power);
	}

#ifdef CONFIG_BOOST_5V
	ret = of_property_read_u32(dev->of_node, "boost-load-enable", &temp);
	if (ret < 0)
		vdev->boost_load_enable = 0;
	else
		vdev->boost_load_enable = temp;
#endif

	ret = of_property_read_u32(dev->of_node, "vibrator-voltage", &temp);
	if (ret < 0) {
		dev_err(dev, "get vibrator_vol fail\n");
		return -EINVAL;
	}
	vdev->linear_vibrator_vol = temp;
	vdev->linear_vibrator_strength = vdev->linear_vibrator_vol;

	ret = of_property_read_u32(dev->of_node, "vibrator-erm", &temp);
	if (ret < 0) {
		dev_info(dev, "get vibrator-erm fail\n");
		vdev->linear_erm_vibrator = 0;
	} else {
		vdev->linear_erm_vibrator = temp;
		dev_info(dev, "vibrator is erm\n");
		return 0;
	}

	ret = linear_vibrator_haptics_cfg(vdev);
	if (ret)
		return ret;

	return 0;
}

static s32 linear_vibrator_haptics_probe(struct linear_vibrator_dev *vdev)
{
	s32 ret;

	vdev->version = MKDEV(0, 0);
	ret = alloc_chrdev_region(
		&vdev->version, 0, 1, LINEAR_VIBRATOR_CDEVIE_NAME);
	if (ret < 0) {
		dev_err(vdev->dev, "failed to alloc chrdev region, ret[%d]\n",
			ret);
		return ret;
	}

	vdev->class = class_create(THIS_MODULE, LINEAR_VIBRATOR_CDEVIE_NAME);
	if (!vdev->class) {
		dev_err(vdev->dev, "failed to create class\n");
		ret = ENOMEM;
		goto unregister_cdev_region;
	}

	vdev->dev = device_create(vdev->class, NULL, vdev->version, NULL,
		LINEAR_VIBRATOR_CDEVIE_NAME);
	if (vdev->dev == NULL) {
		ret = ENOMEM;
		dev_err(vdev->dev, "failed to create device\n");
		goto destory_class;
	}

	cdev_init(&vdev->cdev, &linear_vibrator_fops);
	vdev->cdev.owner = THIS_MODULE;
	vdev->cdev.ops = &linear_vibrator_fops;
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

static void linear_vibrator_haptics_remove(
	struct linear_vibrator_dev *vdev)
{
	cdev_del(&vdev->cdev);
	device_destroy(vdev->class, vdev->version);
	class_destroy(vdev->class);
	unregister_chrdev_region(vdev->version, 1);
	switch_dev_unregister(&vdev->sw_dev);
}

static int linear_vibrator_register_led_classdev(
	struct linear_vibrator_dev *vdev)
{
	struct led_classdev *led_cdev = &vdev->led_dev;

	led_cdev->name = "vibrator";
	led_cdev->flags = LED_CORE_SUSPENDRESUME;
	led_cdev->brightness_set = linear_vibrator_enable_ctrl;
	led_cdev->default_trigger = "transient";

	return devm_led_classdev_register(vdev->dev, led_cdev);
}

static int linear_vibrator_init_cfg(struct linear_vibrator_dev *vdev)
{
	s32 ret;

	ret = linear_vibrator_register_led_classdev(vdev);
	if (ret) {
		dev_err(vdev->dev, "unable to register with timed_output\n");
		return 0;
	}

	if (vdev->linear_erm_vibrator) {
		ret = sysfs_create_group(&vdev->led_dev.dev->kobj,
			&linear_erm_vb_attr_group);
		if (ret) {
			dev_err(vdev->dev, "unable create erm min_timeout sys\n");
			return 0;
		}
	} else {
		vdev->linear_vibrator_correct_freq = LRA_DEFAULT_FREQ;
		ret = sysfs_create_group(&vdev->led_dev.dev->kobj,
			&linear_duty_attr_group);
		if (ret) {
			dev_err(vdev->dev, "unable create duty set sys\n");
			return 0;
		}
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
		ret = sysfs_create_group(&vdev->led_dev.dev->kobj,
			&linear_vb_attr_group);
		if (ret)
			dev_err(vdev->dev, "unable create vibrator's\n");
#endif
		ret = linear_vibrator_haptics_probe(vdev);
		if (ret) {
			dev_err(vdev->dev, "failed to register haptics dev\n");
			goto haptics_fail_probe;
		}
	}

	return 0;

haptics_fail_probe:
	sysfs_remove_group(&vdev->led_dev.dev->kobj, &linear_duty_attr_group);
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
	sysfs_remove_group(&vdev->led_dev.dev->kobj, &linear_vb_attr_group);
#endif
	return ret;
}

static int linear_vibrator_probe(struct spmi_device *pdev)
{
	struct linear_vibrator_dev *vdev = NULL;
	s32 ret;

	vdev = devm_kzalloc(&pdev->dev, sizeof(struct linear_vibrator_dev),
		GFP_KERNEL);
	if (vdev == NULL)
		return -ENOMEM;

	vdev->dev = &pdev->dev;
	g_vdev = vdev;
	dev_set_drvdata(&pdev->dev, vdev);

	/* parse DT */
	ret = linear_vibrator_parse_dt(vdev);
	if (ret) {
		dev_err(&pdev->dev, "DT parsing failed\n");
		return ret;
	}

	mutex_init(&vdev->lock);
#ifdef CONFIG_HUAWEI_DSM
	if (vib_dclient == NULL)
		vib_dclient = dsm_register_client(&dsm_vibrator);
	linear_vibrator_dmd_flage = LINEAR_VIBRATOR_DMD_NO_FLAGE;
#endif

	INIT_WORK(&vdev->vibrator_enable_work, linear_vibrator_enable_work);
	INIT_WORK(&vdev->linear_vibrator_irq_work,
		linear_vibrator_irq_function);
	INIT_WORK(&vdev->vibrator_off_work, linear_vibrator_off_work);

	ret = linear_vibrator_init_cfg(vdev);
	if (ret)
		goto fail_init_cfg;

	linear_vibrator_set_mode(LINEAR_VIBRATOR_MODE_STANDBY);

	/* init interrupts */
	ret = linear_vibrator_interrupt_init(pdev);
	if (ret)
		dev_err(&pdev->dev, "interrupts init failed\n");

	dev_info(&pdev->dev, "linear_vibrator probe succeed\n");

	return 0;

fail_init_cfg:
	cancel_work_sync(&vdev->vibrator_off_work);
	cancel_work_sync(&vdev->linear_vibrator_irq_work);
	cancel_work_sync(&vdev->vibrator_enable_work);
	mutex_destroy(&vdev->lock);
	return ret;
}

static s32 linear_vibrator_remove(struct spmi_device *pdev)
{
	struct linear_vibrator_dev *vdev = NULL;

	vdev = dev_get_drvdata(&pdev->dev);
	if (vdev == NULL) {
		pr_err("%s:failed to get drvdata\n", __func__);
		return -ENODEV;
	}
	cancel_work_sync(&vdev->linear_vibrator_irq_work);
	cancel_work_sync(&vdev->vibrator_off_work);
	cancel_work_sync(&vdev->vibrator_enable_work);
	sysfs_remove_group(&vdev->led_dev.dev->kobj, &linear_duty_attr_group);
	if (vdev->linear_erm_vibrator) {
		sysfs_remove_group(&vdev->led_dev.dev->kobj,
			&linear_erm_vb_attr_group);
	} else {
#ifdef CONFIG_HISI_PMIC_VIBRATOR_DEBUG
		sysfs_remove_group(&vdev->led_dev.dev->kobj,
			&linear_vb_attr_group);
#endif
		linear_vibrator_haptics_remove(vdev);
	}
	mutex_destroy(&vdev->lock);
	wakeup_source_trash(&vdev->wakelock);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static const struct of_device_id linear_vibrator_match[] = {
	{
		.compatible = "hisilicon,pmic-vibrator",
	},
	{},
};
MODULE_DEVICE_TABLE(of, linear_vibrator_match);

static struct spmi_device_id linear_vibrator_id[] = {
	{"hisilicon,pmic-vibrator", 0}, {},
};
static struct spmi_driver linear_vibrator_driver = {
	.probe = linear_vibrator_probe,
	.remove = linear_vibrator_remove,
	.id_table = linear_vibrator_id,
	.driver = {
		.name = "linear-vibrator",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(linear_vibrator_match),
	},
};

static int __init linear_vibrator_init(void)
{
	return spmi_driver_register(&linear_vibrator_driver);
}

static void __exit linear_vibrator_exit(void)
{
	spmi_driver_unregister(&linear_vibrator_driver);
}

module_init(linear_vibrator_init);
module_exit(linear_vibrator_exit);

MODULE_DESCRIPTION("Linear Vibrator driver");
MODULE_LICENSE("GPL");
