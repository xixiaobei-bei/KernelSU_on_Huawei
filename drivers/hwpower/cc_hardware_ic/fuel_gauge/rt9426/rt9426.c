/*
 * rt9426.c
 *
 * driver for rt9426 battery fuel gauge
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "rt9426.h"

#define HWLOG_TAG rt9426
HWLOG_REGIST();

#define PRECISION_ENHANCE          5
#define RT9426_GAIN_DEFAULT_VAL    128
#define RT9426_GAIN_BASE_VAL       512
#define RT9426_COUL_DEFAULT_VAL    1000000
#define RT9426_TBATICAL_MIN_A      752000
#define RT9426_TBATICAL_MAX_A      1246000
#define RT9426_TBATCAL_MIN_A       752000
#define RT9426_TBATCAL_MAX_A       1246000
#define RT9426_EXTEND_REG          0x78
#define RT9426_FULL_CAPCACITY      100
#define RT9426_CAPACITY_TH         7
#define LV_SMOOTH_V_TH             3250   /* 3250mV */
#define LV_SMOOTH_S_TH             4      /* 4% */
#define LV_SMOOTH_T_MIN            100    /* 10oC */
#define LV_SMOOTH_T_MAX            500    /* 50oC */
#define LV_SMOOTH_I_TH             (-400) /* -400*2.5 = -1000 mA */
#define CC_THRESHOLD               450
#define NODE_NAME_MAX              20

struct rt9426_platform_data {
	int soc_offset_size[RT9426_SOC_OFFSET_SIZE];
	struct soc_offset_table soc_offset;
	int extreg_size;
	struct extreg_update_table extreg_update;
	int offset_interpolation_order[RT9426_OFFSET_INTERPLO_SIZE];
	struct fg_ocv_table ocv_table[RT9426_MAX_VTERM_SIZE * RT9426_OCV_ROW_SIZE];
	int ocv_vterm_size;

	char *bat_name;
	int boot_gpio;
	u32 intr_gpio;
	int chg_sts_gpio;
	int chg_inh_gpio;
	int chg_done_gpio;
	int design_capacity;
	int battery_type;
	u32 uv_ov_threshold;
	u32 us_threshold;
	u32 otc_tth;
	u32 otc_chg_ith;
	u32 otd_tth;
	u32 otd_dchg_ith;
	u32 curr_db;

	u32 dtsi_version[RT9426_DTSI_VER_SIZE];
	u32 op_config[RT9426_OP_CONFIG_SIZE];
	u32 fc_ith;
	u32 fc_sth;
	u32 fd_vth;
	u32 fcc[RT9426_MAX_VTERM_SIZE];
	u32 fc_vth[RT9426_MAX_VTERM_SIZE];

	u32 temp_source;
	u32 volt_source;
	u32 curr_source;

	u16 ocv_checksum_ic;
	u16 ocv_checksum_dtsi;
	u32 rs_ic_setting; /* in unit of 0.01 Ohm */
	u32 rs_schematic; /* in unit of 0.01 Ohm */

	u32 rt_config_ver;
	bool force_use_aux_cali_para;
	int ntc_compensation_is;
	struct compensation_para ntc_temp_compensation_para[NTC_PARA_LEVEL];
	int need_reset_version;
};

struct rt9426_chip {
	struct i2c_client *i2c;
	struct device *dev;
	struct rt9426_platform_data *pdata;
	struct power_supply *fg_psy;
#ifdef CONFIG_DEBUG_FS
	struct dentry *dir_dentry;
	struct dentry *file_dentries[RT9426FG_DENTRY_NR];
#endif /* CONFIG_DEBUG_FS */
	struct mutex var_lock;
	struct mutex update_lock;
	int alert_irq;
	int capacity;
	int soc_offset;
	u8 online : 1;
	int btemp;
	int bvolt;
	int bcurr;
	u16 ic_ver;
	u16 ocv_checksum_ic;
	u16 ocv_checksum_dtsi;
	bool calib_unlock;
	bool low_v_smooth_en;
	int rm;
	int avg_curr;
	int c_gain;
	int c_offset;
	int v_gain;
	int ocv_index;
};

struct rt9426_display_data {
	int temp;
	int vbat;
	int ibat;
	int avg_ibat;
	int rm;
	int soc;
	int fcc;
};

enum {
	RT9426_DISPLAY_TEMP = 0,
	RT9426_DISPLAY_VBAT,
	RT9426_DISPLAY_IBAT,
	RT9426_DISPLAY_AVG_IBAT,
	RT9426_DISPLAY_RM,
	RT9426_DISPLAY_SOC,
	RT9426_DISPLAY_DISIGN_FCC,
	RT9426_DISPLAY_FCC,
};

static struct rt9426_chip *g_rt9426_chip;

static int rt9426_read_device(void *client, u32 reg, int len, void *dst)
{
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret;

	if (len > 1) {
		ret = i2c_smbus_read_i2c_block_data(i2c, reg, len, dst);
	} else {
		ret = i2c_smbus_read_byte_data(i2c, reg);
		if (ret < 0)
			return ret;
		*(u8 *)dst = (u8)ret;
	}
	return ret;
}

static int rt9426_write_device(void *client, u32 reg, int len, const void *src)
{
	const u8 *data = NULL;
	struct i2c_client *i2c = (struct i2c_client *)client;
	int ret;

	if (len > 1) {
		ret = i2c_smbus_write_i2c_block_data(i2c, reg, len, src);
	} else {
		data = src;
		ret = i2c_smbus_write_byte_data(i2c, reg, *data);
	}
	return ret;
}

static int rt9426_block_read(struct i2c_client *i2c, u8 reg, int len, void *dst)
{
	struct rt9426_chip *chip = i2c_get_clientdata(i2c);
	int ret;

	ret = rt9426_read_device(i2c, reg, len, dst);
	if (ret < 0)
		dev_notice(chip->dev, "rt9426 block read 0x%02x fail\n", reg);
	return ret;
}

static int rt9426_block_write(struct i2c_client *i2c,
	u8 reg, int len, const void *src)
{
	struct rt9426_chip *chip = i2c_get_clientdata(i2c);
	int ret;

	ret = rt9426_write_device(i2c, reg, len, src);
	if (ret < 0)
		dev_notice(chip->dev, "rt9426 block write 0x%02x fail\n", reg);
	return ret;
}

static int rt9426_reg_read_word(struct i2c_client *i2c, u8 reg)
{
	u16 data = 0;
	int ret;

	ret = rt9426_block_read(i2c, reg, 2, &data);
	return (ret < 0) ? ret : (s32)le16_to_cpu(data);
}

static int rt9426_reg_write_word(struct i2c_client *i2c, u8 reg, u16 data)
{
	data = cpu_to_le16(data);
	return rt9426_block_write(i2c, reg, 2, (uint8_t *)&data);
}

static int __maybe_unused rt9426_reg_write_word_with_check(struct rt9426_chip *chip,
	u8 reg, u16 data)
{
	int retry_times = 2; /* sometimes need retry */
	int r_data = 0;

	while (retry_times) {
		rt9426_reg_write_word(chip->i2c, reg, data);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
		usleep_range(5000, 5100); /* delay 5ms */
		r_data = rt9426_reg_read_word(chip->i2c, reg);
		if (data == r_data) {
			dev_info(chip->dev,
				"TWrite REG_0x%.2x Successful\n", reg);
			break;
		}
		retry_times--;
		if (retry_times == 0)
			dev_notice(chip->dev, "Write REG_0x%.2x fail\n", reg);
	}
	return r_data;
}

static int rt9426_regmap_init(struct rt9426_chip *chip)
{
	return 0;
}

static void rt9426_read_page_cmd(struct rt9426_chip *chip, uint8_t page)
{
	uint16_t read_page_cmd = RT9426_READ_PAGE_CMD;

	read_page_cmd += page;
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, read_page_cmd);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, read_page_cmd);
	usleep_range(5000, 5100); /* delay 5ms */
}

static void rt9426_write_page_cmd(struct rt9426_chip *chip, uint8_t page)
{
	uint16_t write_page_cmd = RT9426_WRITE_PAGE_CMD;

	write_page_cmd += page;
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, write_page_cmd);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, write_page_cmd);
	usleep_range(5000, 5100); /* delay 5ms */
}

static int rt9426_unseal_wi_retry(struct rt9426_chip *chip)
{
	int i, regval, retry_times;

	retry_times = 3; /* sometimes need retry */
	for (i = 0; i < retry_times; i++) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
		if (regval < 0)
			return -1;
		if ((regval & RT9426_UNSEAL_MASK) == RT9426_UNSEAL_STATUS) {
			dev_info(chip->dev, "RT9426 Unseal Pass\n");
			return RT9426_UNSEAL_PASS;
		} else {
			dev_info(chip->dev, "RT9426 Unseal Fail Cnt = %d\n", i + 1);
			if (i >= 2) { /* 2: retry time */
				dev_info(chip->dev, "RT9426 Unseal Fail after 3 retries\n");
				return RT9426_UNSEAL_FAIL;
			} else if (i > 0) {
				dev_info(chip->dev, "delay 1000ms before next Unseal retry\n");
				mdelay(1000); /* delay 1s */
			}

			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
				(RT9426_UNSEAL_KEY & 0xffff)); /* lower 2 bytes */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
				(RT9426_UNSEAL_KEY >> 16)); /* higher 2 bytes */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
			usleep_range(5000, 5100); /* delay 5ms */
		}
	}
	return RT9426_UNSEAL_FAIL;
}

static void rt9426_hibernate_duty_set(struct rt9426_chip *chip, uint16_t data)
{
	int regval;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_read_page_cmd(chip, RT9426_PAGE_1);
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
		if (regval < 0)
			return;
		regval = ((regval & 0xfff8) | (data & 0x0007)); /* mask bit [2-0] and set */
		rt9426_write_page_cmd(chip, RT9426_PAGE_1);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7, regval);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
		mdelay(10); /* delay 10ms */
	}
}

static void rt9426_hibernate_duty_read(struct rt9426_chip *chip)
{
	int regval;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_read_page_cmd(chip, RT9426_PAGE_1);
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
		if (regval < 0)
			return;
		regval = (regval & 0x0007); /* set bit[2-0] */
		dev_info(chip->dev, "HIBERNATE_DUTTY = 2^%d sec)\n", regval);
	}
}

static void rt9426_enter_hibernate(struct rt9426_chip *chip)
{
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_ENTR_SLP_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
}

static void rt9426_exit_hibernate(struct rt9426_chip *chip)
{
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXIT_SLP_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
}

static void rt9426_temperature_set(struct rt9426_chip *chip, int data)
{
	dev_info(chip->dev, "%s: temp = %d oC\n", __func__, data);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_TEMP, ((data * 10) + 2732)); /* to K temp */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
}

static void rt9426_reset(struct rt9426_chip *chip)
{
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x0041); /* reset regs */
}

static int rt9426_get_volt(struct rt9426_chip *chip)
{
	if (chip->pdata->volt_source)
		chip->bvolt = rt9426_reg_read_word(chip->i2c, chip->pdata->volt_source);
	return chip->bvolt;
}

static int rt9426_get_temp(struct rt9426_chip *chip)
{
	if (chip->pdata->temp_source) {
		chip->btemp = rt9426_reg_read_word(chip->i2c, chip->pdata->temp_source);
		chip->btemp -= 2732; /* to K temp */
	}
	return chip->btemp;
}

static unsigned int rt9426_get_cyccnt(struct rt9426_chip *chip)
{
	int ret;
	unsigned int cyccnt = 0;

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_CYC);
	if (ret < 0)
		dev_notice(chip->dev, "%s: read cycle count fail\n", __func__);
	else
		cyccnt = ret;
	return cyccnt;
}

static unsigned int rt9426_get_avg_current(struct rt9426_chip *chip)
{
	chip->avg_curr = rt9426_reg_read_word(chip->i2c, RT9426_REG_AI);
	if (chip->avg_curr < 0)
		return -EIO;
	if (chip->avg_curr > 0x7FFF) { /* handle negative situation */
		chip->avg_curr = 0x10000 - chip->avg_curr;
		chip->avg_curr = 0 - chip->avg_curr;
	}

	return chip->avg_curr;
}

static unsigned int rt9426_get_current(struct rt9426_chip *chip)
{
	chip->bcurr = rt9426_reg_read_word(chip->i2c, RT9426_REG_CURR);
	if (chip->bcurr < 0)
		return -EIO;
	if (chip->bcurr > 0x7FFF) { /* handle negative situation */
		chip->bcurr = 0x10000 - chip->bcurr;
		chip->bcurr = 0 - chip->bcurr;
	}

	return chip->bcurr;
}

static unsigned int rt9426_get_rm(struct rt9426_chip *chip)
{
	chip->rm = rt9426_reg_read_word(chip->i2c, RT9426_REG_RM);

	return chip->rm;
}

static int rt9426_get_offset(struct rt9426_chip *chip, int soc, int temp);

static int rt9426_get_soc(struct rt9426_chip *chip)
{
	int regval;
	int capacity;
	int btemp;

	regval  = rt9426_reg_read_word(chip->i2c, RT9426_REG_SOC);
	if (regval < 0) {
		dev_notice(chip->dev, "read soc value fail\n");
		return -EIO;
	}
	capacity = (regval * 10); /* 10 precision */
	dev_dbg(chip->dev, "capacity before offset = %d\n", capacity);
	btemp = rt9426_get_temp(chip);
	dev_dbg(chip->dev, "TEMP = %d\n", btemp);
	chip->soc_offset = rt9426_get_offset(chip, capacity, btemp);
	dev_dbg(chip->dev, "SOC_OFFSET = %d\n", chip->soc_offset);
	capacity += chip->soc_offset;
	dev_dbg(chip->dev, "capacity after offset = %d\n", capacity);
	if (capacity > 0)
		capacity = DIV_ROUND_UP(capacity, 10); /* 10 precision */
	else
		capacity = 0;
	if (capacity > 100) /* full capacity */
		capacity = 100;
	dev_dbg(chip->dev, "SYS_SOC = %d\n", capacity);
	return capacity;
}

static void set_low_volt_smooth(struct rt9426_chip *chip)
{
	int regval;

	regval = rt9426_unseal_wi_retry(chip);
	if (regval == RT9426_UNSEAL_FAIL) {
		dev_info(chip->dev,
			"%s: Unseal failed. Unable to do lv smooth\n", __func__);
	} else {
		rt9426_write_page_cmd(chip, RT9426_PAGE_5);
		/* low voltage smooth, enable: 0x00FF, disable: 0x0032 */
		if (chip->low_v_smooth_en)
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, 0x00FF);
		else
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, 0x0032);
	}
}

static void check_for_lv_smooth(struct rt9426_chip *chip)
{
	int vbat = rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT);
	int temp = rt9426_get_temp(chip);
	int soc = rt9426_get_soc(chip);
	int curr = rt9426_get_current(chip);

	if ((temp >= LV_SMOOTH_T_MIN) && (temp <= LV_SMOOTH_T_MAX)) {
		if (curr <= LV_SMOOTH_I_TH) {
			/* dsg current <= -1000mA, disable lv_smooth */
			if (chip->low_v_smooth_en) {
				chip->low_v_smooth_en = false;
				set_low_volt_smooth(chip);
			}
		} else {
			/* dsg current > -1000mA, keep checking vbat & soc condition */
			if ((vbat <= LV_SMOOTH_V_TH) && (soc > LV_SMOOTH_S_TH)) {
				/* vbat & soc condition meet, enable lv_smooth */
				if (!chip->low_v_smooth_en) {
					chip->low_v_smooth_en = true;
					set_low_volt_smooth(chip);
				}
			} else {
				/* vbat & soc condition don't meet, disable lv_smooth */
				if (chip->low_v_smooth_en) {
					chip->low_v_smooth_en = false;
					set_low_volt_smooth(chip);
				}
			}
		}
	} else {
		/* temperature condition don't meet, disable lv_smooth */
		if (chip->low_v_smooth_en) {
			chip->low_v_smooth_en = false;
			set_low_volt_smooth(chip);
		}
	}
}

static int rt9426_get_display_data(struct rt9426_chip *di, int index)
{
	int val = 0;

	switch (index) {
	case RT9426_DISPLAY_TEMP:
		val = rt9426_get_temp(di);
		break;
	case RT9426_DISPLAY_VBAT:
		val = rt9426_get_volt(di);
		break;
	case RT9426_DISPLAY_IBAT:
		val = rt9426_get_current(di);
		if (di->pdata->rs_schematic)
			val = ((s64)val *  di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_AVG_IBAT:
		val = rt9426_get_avg_current(di);
			if (di->pdata->rs_schematic)
				val = ((s64)val *  di->pdata->rs_ic_setting) /
					di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_RM:
		val = rt9426_get_rm(di);
		if (di->pdata->rs_schematic)
			val = ((s64)val *  di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_SOC:
		val = rt9426_get_soc(di);
		check_for_lv_smooth(di);
		break;
	case RT9426_DISPLAY_DISIGN_FCC:
		val = di->pdata->design_capacity;
		if (di->pdata->rs_schematic)
			val = ((s64)val *  di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	case RT9426_DISPLAY_FCC:
		val = rt9426_reg_read_word(di->i2c, RT9426_REG_FCC);
		if (di->pdata->rs_schematic)
			val = ((s64)val * di->pdata->rs_ic_setting) /
				di->pdata->rs_schematic;
		break;
	default:
		break;
	}

	return val;
}

static int rt9426_get_log_head(char *buffer, int size, void *dev_data)
{
	struct rt9426_chip *di = dev_data;

	if (!buffer || !di)
		return -1;

	snprintf(buffer, size,
		"    Temp   Vbat   Ibat   AIbat   Rm   Soc   ");

	return 0;
}

static void rt9426_dump_register(struct rt9426_chip *di)
{
	int val;

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_RSVD_FLAG);
	if (val >= 0)
		dev_info(di->dev, "RSVD = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG1);
	if (val >= 0)
		dev_info(di->dev, "flag1 = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG2);
	if (val >= 0)
		dev_info(di->dev, "flag2 = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FLAG3);
	if (val >= 0)
		dev_info(di->dev, "flag3 = 0x%x\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_DSNCAP);
	if (val >= 0)
		dev_info(di->dev, "DC = %d\n", val);

	val = rt9426_reg_read_word(di->i2c, RT9426_REG_FCC);
	if (val >= 0)
		dev_info(di->dev, "FCC = %d\n", val);
}

static int rt9426_dump_log_data(char *buffer, int size, void *dev_data)
{
	struct rt9426_chip *di = dev_data;
	struct rt9426_display_data g_dis_data;

	if (!buffer || !di)
		return -1;

	g_dis_data.vbat = rt9426_get_display_data(di, RT9426_DISPLAY_VBAT);
	g_dis_data.ibat = rt9426_get_display_data(di, RT9426_DISPLAY_IBAT);
	g_dis_data.avg_ibat = rt9426_get_display_data(di, RT9426_DISPLAY_AVG_IBAT);
	g_dis_data.rm = rt9426_get_display_data(di, RT9426_DISPLAY_RM);
	g_dis_data.temp = rt9426_get_display_data(di, RT9426_DISPLAY_TEMP);
	g_dis_data.soc = rt9426_get_display_data(di, RT9426_DISPLAY_SOC);

	snprintf(buffer, size, "%-7d%-7d%-7d%-7d%-7d%-7d   ",
		g_dis_data.temp, g_dis_data.vbat, g_dis_data.ibat,
		g_dis_data.avg_ibat, g_dis_data.rm, g_dis_data.soc);

	return 0;
}

static struct power_log_ops rt9426_fg_ops = {
	.dev_name = "rt9426",
	.dump_log_head = rt9426_get_log_head,
	.dump_log_content = rt9426_dump_log_data,
};

/* add to get both ocv checksum and replace "qs_en" */
static unsigned int rt9426_get_ocv_checksum(struct rt9426_chip *chip)
{
	int regval = rt9426_unseal_wi_retry(chip);

	if (regval == RT9426_UNSEAL_FAIL) {
		chip->ocv_checksum_ic = 0xFFFF;
	} else {
		/* get ocv checksum from ic */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0xCA09);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0xCA09);
		mdelay(5); /* delay 5ms */
		chip->ocv_checksum_ic = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5);
	}
	return chip->ocv_checksum_ic;
}

static int rt9426_get_charging_current_setting(struct rt9426_chip *chip)
{
	return RT9426_CHG_CURR_VAL;
}

static int rt9426_update_ieoc_setting(struct rt9426_chip *chip)
{
	int icc_setting;
	int regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG1);

	dev_info(chip->dev, "%s\n", __func__);
	if (regval < 0)
		return 0;

	if (regval & BIT(0))
		return 0;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return -EINVAL;
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXT_READ_CMD_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXT_READ_CMD_PAGE_5);
	usleep_range(2000, 2100); /* delay 2ms */

	icc_setting = rt9426_get_charging_current_setting(chip);
	dev_info(chip->dev, "icc_setting = %d\n", icc_setting);
	if (icc_setting < CC_THRESHOLD) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
		if (regval != 0x109F) { /* opcfg reg */
			dev_info(chip->dev, "smaller than CC threshold, update\n", __func__);
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXT_WRITE_CMD_PAGE_5);
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXT_WRITE_CMD_PAGE_5);
			usleep_range(1000, 1100); /* delay 1ms */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, 0x109F); /* set opcfg */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
			usleep_range(1000, 1100); /* delay 1ms */
		}
	} else {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
		if (regval != 0x319F) { /* opcfg reg */
			dev_info(chip->dev, "larger than CC threshold, update\n", __func__);
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXT_WRITE_CMD_PAGE_5);
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_EXT_WRITE_CMD_PAGE_5);
			usleep_range(1000, 1100); /* delay 1ms */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, 0x319F); /* set opcfg */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
			usleep_range(1000, 1100); /* delay 1ms */
		}
	}

	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x0020); /* disable extend access mode */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	return 0;
}

static int rt9426_reg_write_ocv_table(struct rt9426_chip *chip, const u32 *pval)
{
	int i;
	int j;
	int regval;

	if (!pval)
		return -1;
	for (i = 0; i < (RT9426_OCV_ROW_SIZE - 1); i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
			RT9426_REG_OCV_WRITE + i);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
			RT9426_REG_OCV_WRITE + i);
		for (j = 0; j < RT9426_OCV_COL_SIZE; j++) {
			/* write ocv_table[index][i][j] */
			rt9426_reg_write_word(chip->i2c,
				RT9426_REG_OCV_COL_ADDRESS + j * 2,
				*(pval + i * RT9426_OCV_COL_SIZE + j));
			dev_info(chip->dev, "0x%x ",
				*(pval + i * RT9426_OCV_COL_SIZE + j));
			mdelay(1);  /* delay 1ms */
		}
		dev_info(chip->dev, "\n");
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
		RT9426_REG_OCV_WRITE + i);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL,
		RT9426_REG_OCV_WRITE + i);
	/* write ocv_table last line fine elements */
	for (i = 0; i < 5; i++) {
		/* write ocv_table[index][9][i] */
		rt9426_reg_write_word(chip->i2c,
			RT9426_REG_OCV_COL_ADDRESS + i * 2, *(pval +
			(RT9426_OCV_ROW_SIZE - 1) * RT9426_OCV_COL_SIZE + i));
		dev_info(chip->dev, "0x%x ",
			*(pval + (RT9426_OCV_ROW_SIZE - 1) *
			RT9426_OCV_COL_SIZE + i));
		mdelay(1); /* delay 1ms */
	}
	dev_info(chip->dev, "\n");
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(10); /* delay 10ms */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	if (regval & RT9426_USR_TBL_USED_MASK) {
		dev_info(chip->dev, "OCV Table Write Successful\n");
		return 0;
	}
	return -1;
}

static int rt9426_write_ocv_table(struct rt9426_chip *chip)
{
	int i;
	int retry_times = 3; /* try write 3 times */
	const u32 *pval = (u32 *)chip->pdata->ocv_table +
		chip->ocv_index * RT9426_OCV_DATA_TOTAL_SIZE;

	if (*pval != RT9426_OCV_TABLE_FIRST)
		return -1;

	dev_info(chip->dev, "Write NEW OCV Table\n");
	for (i = 0; i < retry_times; i++) {
		if (!rt9426_reg_write_ocv_table(chip, pval))
			return 0;
	}

	dev_notice(chip->dev, "Set OCV Table fail\n");
	return -1;
}

static void rt9426_update_info(struct rt9426_chip *chip)
{
	int regval;
	int ret;
	struct power_supply *batt_psy = NULL;

	dev_dbg(chip->dev, "%s\n", __func__);

	/* get battery temp from battery power supply */
	batt_psy = power_supply_get_by_name("battery");
	if (!batt_psy)
		return;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return;

	rt9426_read_page_cmd(chip, RT9426_PAGE_1);

	dev_dbg(chip->dev, "OPCFG(0x%x 0x%x 0x%x 0x%x 0x%x)\n",
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW2),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5));

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	rt9426_read_page_cmd(chip, RT9426_PAGE_2);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
	if (regval < 0)
		return;
	regval = (regval & 0x0300) >> 8; /* get defined state */
	if (((ret & 0x0800) >> 11) == 1) { /* get OCV */
		dev_dbg(chip->dev, "OCV table define by User\n");
	} else {
		if (regval == 0)
			dev_dbg(chip->dev, "OCV(4200) Zero_Point(3200)\n");
		else if (regval == 1)
			dev_dbg(chip->dev, "OCV(4350) Zero_Point(3200)\n");
		else if (regval == 2)
			dev_dbg(chip->dev, "OCV(4400) Zero_Point(3200)\n");
		else
			dev_dbg(chip->dev, "OCV(4350) Zero_Point(3400)\n");
	}
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW5);
	dev_dbg(chip->dev, "CSCOMP4(%d)\n", regval);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4);
	dev_dbg(chip->dev, "CSCOMP5(%d)\n", regval);

	dev_dbg(chip->dev, "DSNCAP(%d) FCC(%d)\n",
		rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC));

	dev_dbg(chip->dev,
		"VOLT_SOURCE(0x%x) INPUT_VOLT(%d) FG_VBAT(%d) FG_OCV(%d) FG_AV(%d)\n",
		chip->pdata->volt_source, rt9426_get_volt(chip),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_OCV),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_AV));
	dev_dbg(chip->dev,
		"CURR_SOURCE(0x%x) INPUT_CURR(%d) FG_CURR(%d) FG_AI(%d)\n",
		chip->pdata->curr_source, rt9426_get_current(chip),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_CURR),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_AI));
	dev_dbg(chip->dev, "TEMP_SOURCE(0x%x) INPUT_TEMP(%d) FG_TEMP(%d)\n",
		chip->pdata->temp_source, rt9426_get_temp(chip),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_TEMP));
	dev_dbg(chip->dev, "FG_FG_INTT(%d) FG_AT(%d)\n",
		rt9426_reg_read_word(chip->i2c, RT9426_REG_INTT),
		rt9426_reg_read_word(chip->i2c, RT9426_REG_AT));

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG1);
	dev_dbg(chip->dev, "FLAG1(0x%x)\n", regval);
	if (regval < 0)
		return;
	if (((regval & 0x0200) >> 9) == 1) /* get FC */
		dev_dbg(chip->dev, "FC = 1\n");
	else
		dev_dbg(chip->dev, "FC = 0\n");

	if (((regval & 0x0004) >> 2) == 1) /* get FD */
		dev_dbg(chip->dev, "FD = 1\n");
	else
		dev_dbg(chip->dev, "FD = 0\n");

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	dev_dbg(chip->dev, "FLAG2(0x%x)\n", regval);
	if (regval < 0)
		return;

	if (((regval & 0xE000) >> 13) == 0) /* get Active mode */
		dev_dbg(chip->dev, "Power_Mode (Active)\n");
	else if (((regval & 0xE000) >> 13) == 1) /* get FST_RSP_ACT mode */
		dev_dbg(chip->dev, "Power_Mode (FST_RSP_ACT)\n");
	else if (((regval & 0xE000) >> 13) == 2) /* get Shutdown mode */
		dev_dbg(chip->dev, "Power_Mode (Shutdown)\n");
	else
		dev_dbg(chip->dev, "Power_Mode (Sleep)\n");

	if (((regval & 0x0800) >> 11) == 1) /* get User_Define_Table state */
		dev_dbg(chip->dev, "User_Define_Table (IN USE)\n");
	else
		dev_dbg(chip->dev, "User_Define_Table (NOT IN USE)\n");
	if (((regval & 0x0040) >> 6) == 1) /* get Battery_Status state */
		dev_dbg(chip->dev, "Battery_Status (Inserted)\n");
	else
		dev_dbg(chip->dev, "Battery_Status (Removed)\n");

	if (((regval & 0x0001)) == 1) /* get RLX mode */
		dev_dbg(chip->dev, "RLX = 1\n");
	else
		dev_dbg(chip->dev, "RLX = 0\n");

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	dev_dbg(chip->dev, "FLAG3(0x%x)\n", regval);
	if (regval < 0)
		return;
	if (((regval & 0x0100) >> 8) == 1) /* get RI mode */
		dev_dbg(chip->dev, "RI = 1\n");
	else
		dev_dbg(chip->dev, "RI = 0\n");

	if (((regval & 0x0001)) == 1) /* get Seal mode */
		dev_dbg(chip->dev, "RT9426 (Unseal)\n");
	else
		dev_dbg(chip->dev, "RT9426 (Seal)\n");

	dev_dbg(chip->dev, "CYCCNT(%d)\n", rt9426_get_cyccnt(chip));

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_VGCOMP12);
	dev_dbg(chip->dev, "VGCOMP12(0x%x)\n", regval);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_VGCOMP34);
	dev_dbg(chip->dev, "VGCOMP34(0x%x)\n", regval);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC);
	dev_dbg(chip->dev, "FCC(%d)\n", regval);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP);
	dev_dbg(chip->dev, "DSNCAP(%d)\n", regval);

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_UN_FLT_SOC);
	dev_dbg(chip->dev, "UNFSOC(%d)\n", regval);
	chip->capacity = rt9426_get_soc(chip);

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC);
	regval = chip->capacity;
	regval = regval * ret;
	regval = DIV_ROUND_UP(regval, 100); /* rm: capacity * fcc / 100 */
	dev_dbg(chip->dev, "RM(%d)\n", regval);

	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FCC);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP);
	ret = ret * 100; /* full capacity */
	regval = DIV_ROUND_UP(ret, regval);
	dev_dbg(chip->dev, "SOH(%d)\n", regval);

	rt9426_update_ieoc_setting(chip);

	power_supply_changed(batt_psy);
	power_supply_put(batt_psy);
}

static int rt9426_apply_pdata(struct rt9426_chip *);
static irqreturn_t rt9426_irq_handler(int irqno, void *param)
{
	struct rt9426_chip *chip = (struct rt9426_chip *)param;
	u16 irq_val;

	if (!chip)
		return IRQ_HANDLED;

	irq_val = rt9426_reg_read_word(chip->i2c, RT9426_REG_IRQ);
	/* 0x02: bit flag for UV_IRQ */
	if (irq_val & 0x02)
		power_event_bnc_notify(POWER_BNT_COUL, POWER_NE_COUL_LOW_VOL, NULL);

	return IRQ_HANDLED;
}

static void new_vgcomp_soc_offset_datas(struct device *dev, int type,
	struct rt9426_platform_data *pdata, int size_x, int size_y, int size_z)
{
	switch (type) {
	case SOC_OFFSET:
		if (pdata->soc_offset.soc_offset_data) {
			devm_kfree(dev,
				pdata->soc_offset.soc_offset_data);
			pdata->soc_offset.soc_offset_data = NULL;
		}
		if ((size_x != 0) && (size_y != 0))
			pdata->soc_offset.soc_offset_data =
				devm_kzalloc(dev, size_x * size_y * sizeof(struct data_point),
					GFP_KERNEL);
		if (pdata->soc_offset.soc_offset_data) {
			pdata->soc_offset.soc_voltnr = size_x;
			pdata->soc_offset.tempnr = size_y;
		} else {
			pdata->soc_offset.soc_voltnr = 0;
			pdata->soc_offset.tempnr = 0;
		}
		break;
	case EXTREG_UPDATE:
		if (pdata->extreg_update.extreg_update_data) {
			devm_kfree(dev,
				pdata->extreg_update.extreg_update_data);
			pdata->extreg_update.extreg_update_data = NULL;
		}
		if (size_x != 0)
			pdata->extreg_update.extreg_update_data =
				devm_kzalloc(dev, size_x * sizeof(struct extreg_data_point),
					GFP_KERNEL);
		break;
	default:
		break;
	}
}

static inline const struct data_point *get_mesh_data(
	int i, int j, int k,
	const struct data_point *mesh, int xnr, int ynr)
{
	return mesh + k * ynr * xnr + j * xnr + i;
}

static int get_sub_mesh(int state, struct data_point *mesh_buffer,
	struct submask_condition *condition)
{
	int i, j, k;
	int x = condition->x;
	int y = condition->y;
	int z = condition->z;

	for (i = 0; i < condition->xnr; ++i) {
		if (get_mesh_data(i, 0, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->x >= x)
			break;
	}
	for (; i >= 0 && i < condition->xnr; --i) {
		if (get_mesh_data(i, 0, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->x <= x)
			break;
	}

	for (j = 0; j < condition->ynr; ++j) {
		if (get_mesh_data(0, j, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->y >= y)
			break;
	}
	for (; j >= 0 && j < condition->ynr; --j) {
		if (get_mesh_data(0, j, 0, condition->mesh_src,
			condition->xnr, condition->ynr)->y <= y)
			break;
	}

	if (state == FG_COMP) {
		for (k = 0; k < condition->znr; ++k) {
			if (get_mesh_data(0, 0, k, condition->mesh_src,
				condition->xnr, condition->ynr)->z >= z)
				break;
		}
		for (; k >= 0 && k < condition->znr; --k) {
			if (get_mesh_data(0, 0, k, condition->mesh_src,
				condition->xnr, condition->ynr)->z <= z)
				break;
		}
	}

	i -= ((condition->order_x - 1) / 2);
	j -= ((condition->order_y - 1) / 2);
	k -= ((condition->order_z - 1) / 2);

	if (i <= 0)
		i = 0;
	if (j <= 0)
		j = 0;
	if (k <= 0)
		k = 0;
	if ((i + condition->order_x) > condition->xnr)
		i = condition->xnr - condition->order_x;
	if ((j + condition->order_y) > condition->ynr)
		j = condition->ynr - condition->order_y;
	if ((k + condition->order_z) > condition->znr)
		k = condition->znr - condition->order_z;

	if (state == FG_COMP) {
		for (z = 0; z < condition->order_z; ++z) {
			for (y = 0; y < condition->order_y; ++y) {
				for (x = 0; x < condition->order_x; ++x)
					*(mesh_buffer + z * condition->order_y *
						condition->order_z +
						y * condition->order_x + x)
						= *get_mesh_data(i + x, j + y,
							k + z,
							condition->mesh_src,
							condition->xnr,
							condition->ynr);
			}
		}
	} else {
		for (y = 0; y < condition->order_y; ++y) {
			for (x = 0; x < condition->order_x; ++x)
				*(mesh_buffer + y * condition->order_x + x)
					= *get_mesh_data(i + x, j + y, 0,
						condition->mesh_src,
						condition->xnr,
						condition->ynr);
		}
	}
	return 0;
}

static int offset_li(int xnr, int ynr,
	const struct data_point *mesh, int x, int y)
{
	long long retval = 0;
	int i, j, k;
	long long wm, wd;
	const struct data_point *cache = NULL;

	for (i = 0; i < xnr; ++i) {
		for (j = 0; j < ynr; ++j) {
			wm = wd = 1;
			cache = get_mesh_data(i, j, 0, mesh, xnr, ynr);
			for (k = 0; k < xnr; ++k) {
				if (i != k) {
					wm *= (x - get_mesh_data(k, j, 0,
						mesh, xnr, ynr)->x);
					wd *= (cache->x - get_mesh_data(k, j, 0,
						mesh, xnr, ynr)->x);
				}
			}
			for (k = 0; k < ynr; ++k) {
				if (j != k) {
					wm *= (y - get_mesh_data(i, k, 0,
							mesh, xnr, ynr)->y);
					wd *= (cache->y - get_mesh_data(i, k, 0,
							mesh, xnr, ynr)->y);
				}
			}
			retval += div64_s64(
				((cache->w * wm) << PRECISION_ENHANCE), wd);
		}
	}
	return (int)((retval + (1 << (PRECISION_ENHANCE - 1)))
		>> PRECISION_ENHANCE);
}

static int rt9426_get_offset(struct rt9426_chip *chip, int soc_val, int temp)
{
	const int ip_x = chip->pdata->offset_interpolation_order[0];
	const int ip_y = chip->pdata->offset_interpolation_order[1];
	struct data_point *sub_mesh = NULL;
	int xnr, ynr;
	int offset;
	struct soc_offset_table *offset_table = NULL;
	struct submask_condition condition = {
		.x = soc_val,
		.y = temp,
	};

	sub_mesh = kzalloc(ip_x * ip_y * sizeof(struct data_point), GFP_KERNEL);
	if (!sub_mesh)
		return 0;
	mutex_lock(&chip->var_lock);
	offset_table = &chip->pdata->soc_offset;
	xnr = offset_table->soc_voltnr;
	ynr = offset_table->tempnr;
	if ((xnr == 0) || (ynr == 0)) {
		mutex_unlock(&chip->var_lock);
		kfree(sub_mesh);
		return 0;
	}
	condition.order_x = min(ip_x, xnr);
	condition.order_y = min(ip_y, ynr);
	condition.xnr = xnr;
	condition.ynr = ynr;
	condition.mesh_src = offset_table->soc_offset_data;
	get_sub_mesh(SOC_OFFSET, sub_mesh, &condition);
	offset = offset_li(condition.order_x, condition.order_y, sub_mesh,
		soc_val, temp);
	mutex_unlock(&chip->var_lock);
	kfree(sub_mesh);
	return offset;
}

static int rt9426_get_property(struct power_supply *psy,
	enum power_supply_property psp,
	union power_supply_propval *val)
{
	struct rt9426_chip *chip = power_supply_get_drvdata(psy);
	int rc = 0;

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = chip->online;
		dev_info(chip->dev, "psp_online = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = rt9426_get_volt(chip);
		dev_info(chip->dev, "psp_volt_now = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = chip->pdata->battery_type;
		dev_info(chip->dev, "psp_volt_max_design = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		chip->capacity = rt9426_get_display_data(chip, RT9426_DISPLAY_SOC);
		val->intval = chip->capacity;
		dev_info(chip->dev, "psp_capacity = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_DISIGN_FCC);
		dev_info(chip->dev,
			"psp_charge_full_design = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_IBAT);
		dev_info(chip->dev, "psp_curr_now = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = rt9426_get_display_data(chip, RT9426_DISPLAY_TEMP);
		dev_info(chip->dev, "psp_temp = %d\n", val->intval);
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		val->intval = chip->ocv_index;
		dev_info(chip->dev, "ocv index = %d\n", chip->ocv_index);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int rt9426_set_ocv_table(struct rt9426_chip *chip, int ocv_index)
{
	u32 regval;

	if (!chip)
		return -1;

	mutex_lock(&chip->update_lock);
	chip->ocv_index = ocv_index;
	rt9426_read_page_cmd(chip, RT9426_PAGE_5);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
	/* compare fc_vth */
	if ((regval & 0xff) == chip->pdata->fc_vth[chip->ocv_index]) {
		dev_info(chip->dev,
			"RT9426 ocv table is already write to reg\n");
		goto set_ocv_table_end;
	}

	/* write aging ocv table */
	(void)rt9426_write_ocv_table(chip);
	/* write aging fcc */
	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		chip->pdata->fcc[chip->ocv_index]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	mdelay(5); /* delay 5ms */
	/* write aging fc_vth */
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	/* write fc_vth and fc_ith */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3,
		chip->pdata->fc_vth[chip->ocv_index] |
		(chip->pdata->fc_ith << 8));

set_ocv_table_end:
	mutex_unlock(&chip->update_lock);
	return 0;
}

static int rt9426_set_property(struct power_supply *psy,
	enum power_supply_property psp,
	const union power_supply_propval *val)
{
	int rc = 0;
	struct rt9426_chip *chip = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		if ((val->intval < 0) ||
			(val->intval >= chip->pdata->ocv_vterm_size))
			return -EINVAL;
		if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
			return -EIO;
		rt9426_set_ocv_table(chip, val->intval);
		break;
	default:
		rc = -EINVAL;
		break;
	}
	return rc;
}

static int rt9426_property_is_writeable(struct power_supply *psy,
	enum power_supply_property psp)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_VOLTAGE_OCV:
		return true;
	default:
		return false;
	}
}

static enum power_supply_property rt9426_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_VOLTAGE_OCV,
};

static struct power_supply_desc fg_psy_desc = {
	.name = "rt-fuelgauge",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = rt9426_props,
	.num_properties = ARRAY_SIZE(rt9426_props),
	.get_property = rt9426_get_property,
	.set_property = rt9426_set_property,
	.property_is_writeable = rt9426_property_is_writeable,
};

static int get_parameters(char *buf, long int *param, int num_of_par)
{
	char *token = NULL;
	int base, cnt;

	token = strsep(&buf, " ");

	for (cnt = 0; cnt < num_of_par; cnt++) {
		if (token) {
			if ((token[1] == 'x') || (token[1] == 'X'))
				base = 16;
			else
				base = 10;

			if (kstrtol(token, base, &param[cnt]) != 0)
				return -EINVAL;

			token = strsep(&buf, " ");
		} else {
			return -EINVAL;
		}
	}
	return 0;
}

#ifdef CONFIG_DEBUG_FS
static int dentry_id_to_comp_offset_type[] = {
	[RT9426FG_SOC_OFFSET_SIZE] = SOC_OFFSET,
	[RT9426FG_SOC_OFFSET_DATA] = SOC_OFFSET,
	[RT9426FG_PARAM_LOCK] = -1, /* dummy */
	[RT9426FG_OFFSET_IP_ORDER] = -1, /* dummy */
	[RT9426FG_FIND_OFFSET_TEST] = -1, /* dummy */
	[RT9426FG_PARAM_CHECK] = -1, /* dummy */
};

struct rt9426_dbg_private_data {
	struct rt9426_chip *chip;
	int id;
	int counter;
	int temp;
	int volt;
	int curr;
	int soc_val;
	int temp2;
};

static ssize_t rt9426_debug_write(struct file *filp,
	const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	struct seq_file *s = filp->private_data;
	struct rt9426_dbg_private_data *prv_data = s->private;
	struct rt9426_dbg_private_data *prv_data_header = NULL;
	struct rt9426_chip *chip = prv_data->chip;
	struct data_point *data = NULL;
	char lbuf[RT9426_WRITE_BUF_LEN];
	int rc;
	int index;
	int comp_offset_type;
	long int param[RT9426_MAX_PARAMS];

	if (cnt > (sizeof(lbuf) - 1))
		return -EINVAL;
	rc = copy_from_user(lbuf, ubuf, cnt);
	if (rc)
		return -EFAULT;
	lbuf[cnt] = '\0';
	comp_offset_type = dentry_id_to_comp_offset_type[prv_data->id];
	prv_data_header = prv_data - prv_data->id;
	switch (prv_data->id) {
	case RT9426FG_SOC_OFFSET_SIZE:
		rc = get_parameters(lbuf, param, RT9426_SOC_OFFSET_SIZE);
		if (rc < 0)
			return rc;
		new_vgcomp_soc_offset_datas(chip->dev, SOC_OFFSET,
			chip->pdata, param[0], param[1], 0);
		prv_data_header[prv_data->id + 1].counter = 0;
		break;
	case RT9426FG_SOC_OFFSET_DATA:
		index = comp_offset_type - SOC_OFFSET;
		rc = get_parameters(lbuf, param, RT9426_SOC_OFFSET_DATA_SIZE);
		if (rc < 0)
			return rc;
		data = chip->pdata->soc_offset.soc_offset_data + prv_data->counter;
		data->voltage = param[0];
		data->temperature = param[1];
		data->offset = param[2];

		prv_data->counter++;
		break;
	case RT9426FG_PARAM_LOCK:
		rc = get_parameters(lbuf, param, RT9426FG_PARAM_LOCK_SIZE);
		if (rc < 0)
			return rc;
		if (param[0]) {
			if (prv_data->counter == 0)
				mutex_lock(&chip->var_lock);
			prv_data->counter = 1;
		} else {
			if (prv_data->counter == 1)
				mutex_unlock(&chip->var_lock);
			prv_data->counter = 0;
		}
		break;
	case RT9426FG_OFFSET_IP_ORDER:
		rc = get_parameters(lbuf, param, RT9426FG_OFFSET_IP_ORDER_SIZE);
		if (rc < 0)
			return rc;
		chip->pdata->offset_interpolation_order[0] = param[0];
		chip->pdata->offset_interpolation_order[1] = param[1];
		break;
	case RT9426FG_FIND_OFFSET_TEST:
		rc = get_parameters(lbuf, param, RT9426FG_FIND_OFFSET_TEST_SIZE);
		if (rc < 0)
			return rc;
		prv_data->soc_val = param[0];
		prv_data->temp2 = param[1];
		break;
	case RT9426FG_PARAM_CHECK:
		cnt = -ENOTSUPP;
		break;
	default:
		break;
	}

	return cnt;
}

static int rt9426_debug_read(struct seq_file *s, void *unused)
{
	struct rt9426_dbg_private_data *prv_data = s->private;
	struct rt9426_chip *chip = prv_data->chip;
	struct rt9426_dbg_private_data *prv_data_header;
	struct data_point *data = NULL;
	int i;
	int offset;
	int data_size;

	prv_data_header = prv_data - prv_data->id;
	switch (prv_data->id) {
	case RT9426FG_SOC_OFFSET_SIZE:
		seq_printf(s, "%d %d\n", chip->pdata->soc_offset.soc_voltnr,
			chip->pdata->soc_offset.tempnr);
		break;
	case RT9426FG_SOC_OFFSET_DATA:
		data_size = chip->pdata->soc_offset.soc_voltnr *
			chip->pdata->soc_offset.tempnr;
		if (data_size == 0)
			seq_puts(s, "no data\n");

		data = chip->pdata->soc_offset.soc_offset_data;
		for (i = 0; i < data_size; i++, data++)
			seq_printf(s, "%d %d %d\n", data->voltage,
				data->temperature, data->offset);
		break;
	case RT9426FG_PARAM_LOCK:
		seq_printf(s, "%d\n", prv_data->counter);
		break;
	case RT9426FG_OFFSET_IP_ORDER:
		seq_printf(s, "%d %d\n",
			chip->pdata->offset_interpolation_order[0],
			chip->pdata->offset_interpolation_order[1]);
		break;
	case RT9426FG_FIND_OFFSET_TEST:
		offset = rt9426_get_offset(chip, prv_data->soc_val,
			prv_data->temp2);
		seq_printf(s, "<%d %d> : offset = <%d>\n",
			prv_data->soc_val, prv_data->temp2, offset);
		break;
	case RT9426FG_PARAM_CHECK:
		break;
	default:
		break;
	}
	return 0;
}

static int rt9426_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, rt9426_debug_read, inode->i_private);
}

static const struct file_operations rt9426_debug_ops = {
	.open = rt9426_debug_open,
	.write = rt9426_debug_write,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const char * const dbgfs_names[] = {
	[RT9426FG_SOC_OFFSET_SIZE] = "soc_offset_size",
	[RT9426FG_SOC_OFFSET_DATA] = "soc_offset_data",
	[RT9426FG_PARAM_LOCK] = "param_lock",
	[RT9426FG_OFFSET_IP_ORDER] = "offset_ip_order",
	[RT9426FG_FIND_OFFSET_TEST] = "find_offset_test",
	[RT9426FG_PARAM_CHECK] = "param_check",
};

#define decl_rt9426fg_prv_data(_id) \
{ \
	.id = _id, \
	.counter = 0, \
}

static struct rt9426_dbg_private_data rt9426_dbg_private_data[] = {
	decl_rt9426fg_prv_data(RT9426FG_SOC_OFFSET_SIZE),
	decl_rt9426fg_prv_data(RT9426FG_SOC_OFFSET_DATA),
	decl_rt9426fg_prv_data(RT9426FG_PARAM_LOCK),
	decl_rt9426fg_prv_data(RT9426FG_OFFSET_IP_ORDER),
	decl_rt9426fg_prv_data(RT9426FG_FIND_OFFSET_TEST),
	decl_rt9426fg_prv_data(RT9426FG_PARAM_CHECK),
};

static void rt9426_create_debug_files(struct rt9426_chip *chip)
{
	int i;

	chip->dir_dentry = debugfs_create_dir("rt9426fg_table", 0);
	if (IS_ERR(chip->dir_dentry)) {
		dev_notice(chip->dev,
			"%s : cannot create rt9426fg_table\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(dbgfs_names); ++i) {
		rt9426_dbg_private_data[i].chip = chip;
		chip->file_dentries[i] =
			debugfs_create_file(dbgfs_names[i],
				S_IFREG | 0444,
				chip->dir_dentry,
				&rt9426_dbg_private_data[i],
				&rt9426_debug_ops);
	}
}
#else
static void rt9426_create_debug_files(struct rt9426_chip *chip)
{
	dev_notice(chip->dev, "Not support debugfs\n");
}
#endif /* CONFIG_DEBUG_FS */

/* Add for calibration */
static int rt9426_get_calibration_para(struct rt9426_chip *chip, u8 *curr_offs,
	u8 *curr_gain, u8 *volt_gain)
{
	return 0;
}

static void rt9426_check_system_tick(struct rt9426_chip *chip)
{
	int j;
	int tick_old;
	int tick_new;

	/* Check System Tick before entering Calibration */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SYS_TICK_ON_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);

	usleep_range(5000, 5100); /* delay 5ms */

	tick_old = rt9426_reg_read_word(chip->i2c, RT9426_REG_ADV);
	for (j = 0; j < 1000; j++) { /* try 1000 times */
		tick_new = rt9426_reg_read_word(chip->i2c, RT9426_REG_ADV);
		if (tick_old != tick_new) {
			mdelay(300); /* delay 300ms */
			break;
		}
		usleep_range(5000, 5100); /* delay 5ms */
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_CALI_ENTR_CMD);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SYS_TICK_OFF_CMD);
}

static int rt9426_enter_calibration_mode(struct rt9426_chip *chip)
{
	int i;
	int regval;
	int retry_times = 3; /* 3:calibration retry times */

	for (i = 0; i < retry_times; i++) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (regval < 0)
			return -1;
		if ((regval & RT9426_CALI_MODE_MASK) == RT9426_CALI_MODE_MASK) {
			dev_info(chip->dev, "RT9426 is in Calibration Mode\n");
			return RT9426_CALI_MODE_PASS;
		} else {
			if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
				rt9426_check_system_tick(chip);
				if (i >= (retry_times - 1)) {
					dev_info(chip->dev,
						"RT9426 Enter Calibration Mode Fail after 3 retries\n");
					return RT9426_CALI_MODE_FAIL;
				}
				usleep_range(5000, 5100); /* delay 5ms */
			}
		}
	}
	return RT9426_CALI_MODE_FAIL;
}

static void rt9426_exit_calibration_mode(struct rt9426_chip *chip)
{
	int regval;
	int retry_times = 3; /* retry 3 times */

	do {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_CALI_EXIT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);

		usleep_range(5000, 5100); /* delay 5ms */

		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (!(regval & RT9426_CALI_MODE_MASK)) {
			dev_info(chip->dev, "Success to exit calibration mode\n");
			break;
		}
	} while (--retry_times);
}

static void rt9426_apply_sense_resistor(struct rt9426_chip *chip)
{
	int op_config1;
	int rsense;

	switch (chip->pdata->rs_ic_setting) {
	case 250: /* 2.5mR */
		rsense = 0;
		break;
	case 500: /* 5mR */
		rsense = 1;
		break;
	case 2000: /* 20mR */
		rsense = 3;
		break;
	case 1000: /* 10mR */
	default:
		rsense = 2;
		break;
	}
	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		/* get op_config1 */
		rt9426_read_page_cmd(chip, RT9426_PAGE_1);
		op_config1 = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
		if (op_config1 < 0)
			return;
		/* update rsense to op_config1 */
		op_config1 = (op_config1 & 0xFF3F) | ((rsense & 0x3) << 6);
		/* apply op_config1 */
		rt9426_write_page_cmd(chip, RT9426_PAGE_1);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, op_config1);
		mdelay(5); /* 5: delay 5ms for write success */
	}
}

static void rt9426_apply_calibration_para(struct rt9426_chip *chip, u8 curr_offs,
	u8 curr_gain, u8 volt_gain)
{
	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_write_page_cmd(chip, RT9426_PAGE_0);
		/* set Current system gain & offset */
		if ((curr_gain != 0x00) && (curr_gain != 0xFF) &&
			(curr_offs != 0x00) && (curr_offs != 0xFF))
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1,
				curr_gain | (curr_offs << 8));
		else
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8080);
		/* set Voltage system gain */
		if ((volt_gain != 0x00) && (volt_gain != 0xFF))
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
				0x88 | (volt_gain << 8));
		else
			rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7, 0x8088);
		mdelay(5); /* 5: delay 5ms */
	}
}

static int rt9426_get_curr_by_conversion(struct rt9426_chip *chip)
{
	int regval = 0;

	if (rt9426_enter_calibration_mode(chip) == RT9426_CALI_MODE_PASS) {
		/* Start current convertion */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_CURR_CONVERT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(50);

		/* Get convert result */
		regval  = rt9426_reg_read_word(chip->i2c, RT9426_REG_CURR);
		if (regval < 0)
			return -EIO;
		if (regval > 0x7FFF) {
			regval = 0x10000 - regval;
			regval = 0 - regval;
		}
	}
	/* scaling for the current, 2020-08-07 */
	regval = regval * (int)chip->pdata->rs_ic_setting / (int)chip->pdata->rs_schematic;
	dev_info(chip->dev, "CALIB_CURRENT = %d mA\n", regval);

	return regval;
}

static int rt9426_get_volt_by_conversion(struct rt9426_chip *chip)
{
	int regval = 0;

	if (rt9426_enter_calibration_mode(chip) == RT9426_CALI_MODE_PASS) {
		/* Start current convertion */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_VOLT_CONVERT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(50); /* 50: delay 50ms */

		/* Get voltage result */
		regval  = rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT);
	}
	return regval;
}

/* Add subroutine to handle enter/exit shutdown mode, 2020-08-05 */
static int rt9426_request_charging_inhibit(bool need_inhibit)
{
	return 0;
}

static int rt9426_enter_shutdown_mode(struct rt9426_chip *chip)
{
	int regval, loop;

	for (loop = 0; loop < 5; loop++) { /* 5: retry 5 times */
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		dev_info(chip->dev, "flag2 = 0x%04x\n", regval);
		if (regval < 0)
			return -1;
		if (regval & RT9426_SHDN_MASK)
			break;

		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SHDN_ENTR_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(20); /* 20: delsy 20ms */
	}

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	if (regval < 0)
		return -1;
	if (!(regval & RT9426_SHDN_MASK)) {
		dev_info(chip->dev, "RT9426 Enter Shutdown Fail\n");
		return -1;
	}

	dev_info(chip->dev, "RT9426 Enter Shutdown Success\n");
	return 0;
}

static int rt9426_exit_shutdown_mode(struct rt9426_chip *chip)
{
	int regval, loop;
	int cmd_cnt = 0;

	for (loop = 0; loop < 5; loop++) { /* 5: retry 5 times */
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (regval < 0)
			return -1;
		if (!(regval & RT9426_SHDN_MASK))
			break;
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, RT9426_SHDN_EXIT_CMD);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(250); /* 250: delay 250ms */
		dev_info(chip->dev, "RT9426 Send Exit Shutdown Cmd Count = %d\n", ++cmd_cnt);
	}
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
	if (regval < 0)
		return -1;
	if (regval & RT9426_SHDN_MASK) {
		dev_info(chip->dev, "RT9426 is in Shutdown\n");
		return -1;
	}
	dev_info(chip->dev, "RT9426 is not in Shutdown\n");

	if (cmd_cnt == 0)
		return 0;

	/* Power path control check */
	regval = rt9426_get_current(chip);
	if (regval > 0) {
		rt9426_request_charging_inhibit(true); /* Disable power path */
		dev_info(chip->dev, "RT9426 request to enable charging inhibit\n");
		mdelay(1250); /* 1250: delay 1250ms */
	}
	/* Send QS Command to get INI SOC */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x4000);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
	dev_info(chip->dev, "Send QS after exiting Shutdown\n");
	mdelay(5); /* 5: delay 5ms */
	/* Power path recover check */
	if (regval > 0) {
		rt9426_request_charging_inhibit(false); /* Enable power path */
		dev_info(chip->dev, "RT9426 request to disable charging inhibit\n");
	}

	return 0;
}

static ssize_t rt9426_show_attrs(struct device *, struct device_attribute *,
	char *);
static ssize_t rt9426_store_attrs(struct device *, struct device_attribute *,
	const char *, size_t count);

#define rt9426_attr(_name) \
{ \
	.attr = {.name = #_name, .mode = 0664}, \
	.show = rt9426_show_attrs, \
	.store = rt9426_store_attrs, \
}

static struct device_attribute rt9426_fuelgauge_attrs[] = {
	rt9426_attr(fg_temp),
	rt9426_attr(volt),
	rt9426_attr(curr),
	rt9426_attr(update),
	rt9426_attr(ocv_table),
	rt9426_attr(enter_hibernate),
	rt9426_attr(exit_hibernate),
	rt9426_attr(set_hibernate_duty),
	rt9426_attr(DBP0),
	rt9426_attr(DBP1),
	rt9426_attr(DBP2),
	rt9426_attr(DBP3),
	rt9426_attr(DBP4),
	rt9426_attr(DBP5),
	rt9426_attr(DBP6),
	rt9426_attr(DBP7),
	rt9426_attr(DBP8),
	rt9426_attr(DBP9),
	rt9426_attr(WCNTL),
	rt9426_attr(WEXTCNTL),
	rt9426_attr(WSW1),
	rt9426_attr(WSW2),
	rt9426_attr(WSW3),
	rt9426_attr(WSW4),
	rt9426_attr(WSW5),
	rt9426_attr(WSW6),
	rt9426_attr(WSW7),
	rt9426_attr(WSW8),
	rt9426_attr(WTEMP),
	rt9426_attr(UNSEAL),
	rt9426_attr(FG_SET_TEMP),
	rt9426_attr(FG_RESET),
	rt9426_attr(CALIB_SECURE),
	rt9426_attr(CALIB_RSENSE),
	rt9426_attr(CALIB_CURRENT),
	rt9426_attr(CALIB_VOLTAGE),
	rt9426_attr(CALIB_FACTOR),
	rt9426_attr(FORCE_SHUTDOWN),
};

enum {
	FG_TEMP = 0,
	FG_VOLT,
	FG_CURR,
	FG_UPDATE,
	OCV_TABLE,
	ENTER_HIBERNATE,
	EXIT_HIBERNATE,
	SET_HIBERNATE_DUTY,
	DBP0,
	DBP1,
	DBP2,
	DBP3,
	DBP4,
	DBP5,
	DBP6,
	DBP7,
	DBP8,
	DBP9,
	WCNTL,
	WEXTCNTL,
	WSW1,
	WSW2,
	WSW3,
	WSW4,
	WSW5,
	WSW6,
	WSW7,
	WSW8,
	WTEMP,
	UNSEAL,
	FG_SET_TEMP,
	FG_RESET,
	CALIB_SECURE,
	CALIB_RSENSE,
	CALIB_CURRENT,
	CALIB_VOLTAGE,
	CALIB_FACTOR,
	FORCE_SHUTDOWN,
};

static ssize_t rt9426_show_attrs(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev->parent);
	const ptrdiff_t offset = attr - rt9426_fuelgauge_attrs;
	int i = 0;

	switch (offset) {
	case FG_TEMP:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", chip->btemp);
		break;
	case FG_VOLT:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", chip->bvolt);
		break;
	case FG_CURR:
		i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", chip->bcurr);
		break;
	case CALIB_SECURE:
		i = scnprintf(buf, PAGE_SIZE, chip->calib_unlock ? "Unlocked\n" : "Locked\n");
		break;
	case CALIB_CURRENT:
		if (!chip->calib_unlock)
			return -EACCES;
		rt9426_enter_calibration_mode(chip);
		i = scnprintf(buf, PAGE_SIZE, "%d\n", rt9426_get_curr_by_conversion(chip));
		rt9426_exit_calibration_mode(chip);
		break;
	case CALIB_VOLTAGE:
		if (!chip->calib_unlock)
			return -EACCES;
		rt9426_enter_calibration_mode(chip);
		i = scnprintf(buf, PAGE_SIZE, "%d\n", rt9426_get_volt_by_conversion(chip));
		rt9426_exit_calibration_mode(chip);
		break;
	default:
		i = -EINVAL;
		break;
	}
	return i;
}

static ssize_t rt9426_store_attrs(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev->parent);
	const ptrdiff_t offset = attr - rt9426_fuelgauge_attrs;
	int ret;
	int x;
	int y;
	int temp[RT9426_OCV_COL_SIZE]; /* para nums */
	char *org = NULL;
	char *token = NULL;
	char *cur = NULL;
	long int val;

	switch (offset) {
	case FG_TEMP:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		chip->btemp = val;
		ret = count;
		break;
	case FG_VOLT:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		chip->bvolt = val;
		ret = count;
		break;
	case FG_CURR:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		chip->bcurr = val;
		ret = count;
		break;
	case FG_UPDATE:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_update_info(chip);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case OCV_TABLE:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			for (x = 0; x < RT9426_OCV_ROW_SIZE; x++) {
				rt9426_reg_write_word(chip->i2c,
					RT9426_REG_EXTREGCNTL, 0xCA00 + x);
				rt9426_reg_write_word(chip->i2c,
					RT9426_REG_EXTREGCNTL, 0xCA00 + x);
				for (y = 0; y < RT9426_OCV_COL_SIZE; y++)
					temp[y] = rt9426_reg_read_word(chip->i2c,
						RT9426_REG_SWINDOW1 + y * 2);

				dev_info(dev, "fg_ocv_table_%d\n", x);
				dev_info(dev, "<0x%x 0x%x 0x%x 0x%x>\n",
					temp[0], temp[1], temp[2], temp[3]);
				dev_info(dev, "<0x%x 0x%x 0x%x 0x%x>\n",
					temp[4], temp[5], temp[6], temp[7]);
			}
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case ENTER_HIBERNATE:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_enter_hibernate(chip);
			dev_info(dev, "SLP_STS = %d\n", rt9426_reg_read_word(
				chip->i2c, RT9426_REG_FLAG2) >> 15);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case EXIT_HIBERNATE:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_exit_hibernate(chip);
			dev_info(dev, "SLP_STS = %d\n", rt9426_reg_read_word(
				chip->i2c, RT9426_REG_FLAG2) >> 15);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case SET_HIBERNATE_DUTY:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_hibernate_duty_set(chip, val);
		rt9426_hibernate_duty_read(chip);
		ret = count;
		break;
	case DBP0:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_0);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP1:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_1);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP2:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_2);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP3:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_3);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP4:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_4);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP5:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_5);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP6:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_6);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP7:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_7);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP8:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_8);
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case DBP9:
		if ((sscanf(buf, "%x\n", &x) == 1) && (x == 1)) {
			rt9426_read_page_cmd(chip, RT9426_PAGE_9);
			usleep_range(5000, 5100); /* delay 5ms */
			for (x = 0; x < RT9426_BLOCK_PAGE_SIZE; x++)
				temp[x] = rt9426_reg_read_word(chip->i2c,
					RT9426_REG_SWINDOW1 + x * 2);
			ret = count;
		} else {
			ret = -EINVAL;
		}
		break;
	case WCNTL:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WEXTCNTL:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, val);
		ret = count;
		break;
	case WSW1:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW2:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW3:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW4:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW5:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW6:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW7:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WSW8:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW8, val);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case WTEMP:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_TEMP, ((val * 10) + 2732)); /* get real temp */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		ret = count;
		break;
	case UNSEAL:
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
		if (ret < 0)
			break;
		if ((ret & 0x0001) == 0) {
			/* Unseal RT9426 */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
				(RT9426_UNSEAL_KEY & 0xffff)); /* get low 2 bytes */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL,
				(RT9426_UNSEAL_KEY >> 16)); /* get high 2 bytes */
			rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */

			mdelay(10); /* delay 10ms */

			ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
			if (ret < 0)
				break;
			if ((ret & 0x0001) == 0)
				dev_info(dev, "RT9426 Unseal Fail\n");
			else
				dev_info(dev, "RT9426 Unseal Pass\n");
		} else {
			dev_info(dev, "RT9426 Unseal Pass\n");
		}

		/* Unseal RT9426 */
		ret = count;
		break;
	case FG_SET_TEMP: /* New to set FG Temperature */
		ret = get_parameters((char *)buf, &val, 1);
		if (ret < 0) {
			dev_notice(dev, "get paremters fail\n");
			ret = -EINVAL;
		}
		rt9426_temperature_set(chip, val);
		ret = count;
		break;
	case FG_RESET: /* New to reset FG */
		rt9426_reset(chip);
		ret = count;
		break;
	case CALIB_SECURE:
		if (kstrtoint(buf, 0, &x))
			return -EINVAL;
		chip->calib_unlock = (x == 5526789);
		ret = count;
		break;
	case CALIB_RSENSE:
		if (!chip->calib_unlock)
			return -EACCES;
		rt9426_apply_sense_resistor(chip); /* set rsense according to dtsi */
		rt9426_apply_calibration_para(chip, 0x80, 0x80, 0x80);
		ret = count;
		break;
	case CALIB_FACTOR:
		if (!chip->calib_unlock)
			return -EACCES;
		org = kstrdup(buf, GFP_KERNEL);
		if (!org)
			return -ENOMEM;
		cur = org;
		x = 0;
		while ((token = strsep(&cur, ",")) != NULL) {
			if (kstrtoint(token, 0, &temp[x]))
				break;
			if (++x >= 3)
				break;
		}
		kfree(org);
		if (x != 3)
			return -EINVAL;
		rt9426_apply_calibration_para(chip, temp[0], temp[1], temp[2]);
		ret = count;
		break;
	case FORCE_SHUTDOWN:
		if (kstrtoint(buf, 0, &x) || x < 0 || x > 1)
			return -EINVAL;

		ret = x ? rt9426_enter_shutdown_mode(chip) : rt9426_exit_shutdown_mode(chip);
		if (ret)
			return ret;
		ret = count;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int rt9426_create_attrs(struct device *dev)
{
	int i;
	int rc;

	for (i = 0; i < ARRAY_SIZE(rt9426_fuelgauge_attrs); i++) {
		rc = device_create_file(dev, &rt9426_fuelgauge_attrs[i]);
		if (rc)
			goto create_attrs_failed;
	}
	goto create_attrs_succeed;

create_attrs_failed:
	dev_notice(dev, "%s: failed (%d)\n", __func__, rc);
	while (i--)
		device_remove_file(dev, &rt9426_fuelgauge_attrs[i]);
create_attrs_succeed:
	return rc;
}

static int rt9426_irq_enable(struct rt9426_chip *chip, bool enable)
{
	int regval;

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_PASS) {
		rt9426_write_page_cmd(chip, RT9426_PAGE_1);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2,
			enable ? chip->pdata->op_config[1] : 0);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
		usleep_range(5000, 5100); /* delay 5ms */
		/* if disable, force clear irq status */
		if (!enable) {
			regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_IRQ);
			dev_info(chip->dev, "previous irq status 0x%04x\n", regval);
		}
	}
	return 0;
}

static int rt9426_irq_deinit(struct rt9426_chip *chip)
{
	device_init_wakeup(chip->dev, false);
	return 0;
}

static bool rt9426_is_need_update_pdata(struct rt9426_chip *chip)
{
	int regval;
	int dc;
	int op_config[RT9426_OP_CONFIG_SIZE] = {0};

	/* 1st check version */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_RSVD_FLAG);
	/* mask high bits */
	if ((regval & 0x7FFF) != chip->pdata->rt_config_ver)
		return true;
	/* 2nd check dc */
	dc = rt9426_reg_read_word(chip->i2c, RT9426_REG_DSNCAP);
	if (dc != chip->pdata->design_capacity)
		return true;
	/* 3rd check op config */
	if (rt9426_unseal_wi_retry(chip) != RT9426_UNSEAL_PASS)
		return false;

	rt9426_read_page_cmd(chip, RT9426_PAGE_1);
	op_config[0] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1);
	op_config[1] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW2);
	op_config[2] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
	op_config[3] = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4);
	if (op_config[0] != chip->pdata->op_config[0] ||
		op_config[1] != chip->pdata->op_config[1] ||
		op_config[2] != chip->pdata->op_config[2] ||
		op_config[3] != chip->pdata->op_config[3])
		return true;

	return false;
}

static int rt9426_apply_pdata(struct rt9426_chip *chip)
{
	int i;
	int ret;
	int regval;
	int volt_now;
	int fd_vth_now;
	int fd_threshold;
	int retry_times_all;
	int fc_target1;
	int fc_target2;
	int fd_target1;
	u8 target_curr_offs = 0;
	u8 target_curr_gain = 0;
	u8 target_volt_gain = 0;
	int op_config_reading[RT9426_OP_CONFIG_SIZE] = { 0 };
	int retry_times = 30; /* sometimes need retry */
	bool need_update = false;

	dev_info(chip->dev, "%s\n", __func__);
	for (i = 0; i < retry_times; i++) {
		regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG2);
		if (regval < 0)
			return -1;
		if (regval & RT9426_RDY_MASK) {
			rt9426_exit_shutdown_mode(chip);
			rt9426_get_ocv_checksum(chip);
			regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
			if (regval < 0)
				return -1;
			if (!(regval & RT9426_RI_MASK)) {
				regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_RSVD_FLAG);
				need_update = rt9426_is_need_update_pdata(chip);
				if (!need_update) { /* mask high bits */
					dev_info(chip->dev, "RI=0, bypass initial phase\n");
					goto out_apply;
				} else {
					dev_info(chip->dev, "force init due to DRIVER_VER\n");
					mdelay(60); /* delay 60ms */
					if ((regval & 0x7FFF) <= chip->pdata->need_reset_version) {
						rt9426_reset(chip);
						mdelay(1000); /* delay 1000ms */
					}
					break;
				}
			} else {
				dev_info(chip->dev, "initialization due to RI\n");
				mdelay(60); /* delay 60ms */
				break;
			}
		}
		mdelay(10); /* delay 10ms */
	}

	/* add 3 times retry for overall flow */
	retry_times_all = 3;

init_start:
	if (--retry_times_all < 0) {
		dev_err(chip->dev, "Fail for the retry times 3\n");
		return -EFAULT;
	}

	if (rt9426_unseal_wi_retry(chip) == RT9426_UNSEAL_FAIL)
		return -EINVAL;

	rt9426_reg_read_word(chip->i2c,	RT9426_REG_IRQ);
	/* set OCV Table */
	(void)rt9426_write_ocv_table(chip);

	rt9426_get_calibration_para(chip, &target_curr_offs, &target_curr_gain, &target_volt_gain);
	rt9426_apply_calibration_para(chip, target_curr_offs, target_curr_gain, target_volt_gain);
	/* set alert threshold */
	rt9426_write_page_cmd(chip, RT9426_PAGE_3);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, chip->pdata->otc_tth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2, chip->pdata->otc_chg_ith);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, chip->pdata->otd_tth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4, chip->pdata->otd_dchg_ith);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5, chip->pdata->uv_ov_threshold);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, (0x4600 | (chip->pdata->us_threshold))); /* set us mask */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	/* set OCV type */
	rt9426_write_page_cmd(chip, RT9426_PAGE_2);
	if (chip->pdata->battery_type == 4400) { /* 4400: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8200);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8200\n",
			__func__, chip->pdata->battery_type);
	} else if (chip->pdata->battery_type == 4352) { /* 4352: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8100);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8100\n",
			__func__, chip->pdata->battery_type);
	} else if (chip->pdata->battery_type == 4354) { /* 4354: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8300);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8300\n",
			__func__, chip->pdata->battery_type);
	} else if (chip->pdata->battery_type == 4200) { /* 4200: ocv */
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8000);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8000\n",
			__func__, chip->pdata->battery_type);
	} else {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1, 0x8100);
		dev_info(chip->dev, "%s: ocv type = %d, set ocv type as = 0x8100\n",
			__func__, chip->pdata->battery_type);
	}

	/* set design capacity */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6,
		chip->pdata->design_capacity);

	/* set fcc */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		chip->pdata->fcc[chip->ocv_index]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	rt9426_read_page_cmd(chip, RT9426_PAGE_2);
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW6);
	if (ret == chip->pdata->design_capacity) {
		dev_info(chip->dev, "%s: design capacity matched\n", __func__);
	} else {
		dev_info(chip->dev,
			"%s: design capacity mismatched\n", __func__);
		goto init_start;
	}
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW7);
	dev_info(chip->dev, "%s: fcc setting = %d, fcc reading = %d\n",
		__func__, chip->pdata->fcc[chip->ocv_index], ret);
	if (ret == chip->pdata->fcc[chip->ocv_index]) {
		dev_info(chip->dev, "%s: fcc matched\n", __func__);
	} else {
		dev_info(chip->dev, "%s: fcc mismatched\n", __func__);
		goto init_start;
	}
	/* set all 5 op config */
	rt9426_write_page_cmd(chip, RT9426_PAGE_1);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW1,
		chip->pdata->op_config[0]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW2,
		chip->pdata->op_config[1]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3,
		chip->pdata->op_config[2]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4,
		chip->pdata->op_config[3]);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW5,
		chip->pdata->op_config[4]);
	/* set curr deadband */
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW7,
		(0x0012 | (chip->pdata->curr_db << 8)));
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000); /* reset */
	usleep_range(5000, 5100); /* delay 5ms */

	rt9426_read_page_cmd(chip, RT9426_PAGE_1);
	/* read all 5 op_config for check */
	for (i = 0; i < 5; i++)
		op_config_reading[i] =
			rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW1 + (i * 2));

	/* compare OPCFG1 & show result */
	if (op_config_reading[0] != chip->pdata->op_config[0]) {
		dev_info(chip->dev, "%s: OPCFG1 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG1 matched\n", __func__);

		if (((op_config_reading[0] & 0x00C0) >> 6) == 0)
			dev_info(chip->dev, "Rsense(2.5)mohm\n");
		else if (((op_config_reading[0] & 0x00C0) >> 6) == 1)
			dev_info(chip->dev, "Rsense(5.0)mohm\n");
		else if (((op_config_reading[0] & 0x00C0) >> 6) == 2)
			dev_info(chip->dev, "Rsense(10)mohm\n");
		else
			dev_info(chip->dev, "Rsense(20)mohm\n");

		if (((op_config_reading[0] & 0xC000) >> 14) == 0)
			dev_info(chip->dev, "Temperature (NTC)\n");
		else if (((op_config_reading[0] & 0xC000) >> 14) == 1)
			dev_info(chip->dev, "Temperature (AP)\n");
		else
			dev_info(chip->dev, "Temperature (INT_Temp)\n");
	}

	/* compare OPCFG2 & show result */
	if (op_config_reading[1] != chip->pdata->op_config[1]) {
		dev_info(chip->dev, "%s: OPCFG2 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG2 matched\n", __func__);

		/* 0x4000: voltage alert IRQ mask */
		if (op_config_reading[1] & 0x4000)
			dev_info(chip->dev, "voltage alert IRQ enable\n");
		else
			dev_info(chip->dev, "voltage alert IRQ disable\n");

		if (((op_config_reading[1] & 0x0080) >> 7) == 1)
			dev_info(chip->dev, "Auto_Sleep (Enable)\n");
		else
			dev_info(chip->dev, "Auto_Sleep (Disable)\n");

		if (((op_config_reading[1] & 0x0040) >> 6) == 1)
			dev_info(chip->dev, "Sleep_Mode (Enable)\n");
		else
			dev_info(chip->dev, "Sleep_Mode (Disable)\n");

		if (((op_config_reading[1] & 0x0020) >> 5) == 1)
			dev_info(chip->dev, "Shutdown_Mode (Enable)\n");
		else
			dev_info(chip->dev, "Shutdown_Mode (Disable)\n");

		if (((op_config_reading[1] & 0x0001)) == 1)
			dev_info(chip->dev, "Battery_Detection (Enable)\n");
		else
			dev_info(chip->dev, "Battery_Detection (Disable)\n");

		if (((op_config_reading[1] & 0x2000) >> 13) == 1)
			dev_info(chip->dev, "SOC_IRQ (Enable)\n");
		else
			dev_info(chip->dev, "SOC_IRQ (Disable)\n");

		if (((op_config_reading[1] & 0x1000) >> 12) == 1)
			dev_info(chip->dev, "Battery_Det_IRQ (Enable)\n");
		else
			dev_info(chip->dev, "Battery_Det_IRQ (Disable)\n");

		if (((op_config_reading[1] & 0x0200) >> 9) == 1)
			dev_info(chip->dev, "SC_IRQ (Enable)\n");
		else
			dev_info(chip->dev, "SC_IRQ (Disable)\n");
	}

	/* compare OPCFG3 & show result */
	if (op_config_reading[2] != chip->pdata->op_config[2]) {
		dev_info(chip->dev, "%s: OPCFG3 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG3 matched\n", __func__);

		if (((op_config_reading[2] & 0x00E0) >> 4) == 0xE)
			dev_info(chip->dev, "FC+FD+RLX_Det (Enable)\n");
		else
			dev_info(chip->dev, "FC+FD+RLX_Det (Disable)\n");

		if (((op_config_reading[2] & 0x0018) >> 3) == 0x3)
			dev_info(chip->dev, "TLCOMP (Enable)\n");
		else
			dev_info(chip->dev, "TLCOMP (Disable)\n");

		if (((op_config_reading[2] & 0x0007)) == 0x7) /* set bit[2-0] */
			dev_info(chip->dev, "BCCOMP (Enable)\n");
		else
			dev_info(chip->dev, "BCCOMP (Disable)\n");

		if (((op_config_reading[2] & 0x0200) >> 9) == 1)
			dev_info(chip->dev, "FC_LOCK (Enable)\n");
		else
			dev_info(chip->dev, "FC_LOCK (Disable)\n");
	}
	/* compare OPCFG4 & show result */
	if (op_config_reading[3] != chip->pdata->op_config[3]) {
		dev_info(chip->dev, "%s: OPCFG4 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG4 matched\n", __func__);
	}
	/* compare OPCFG5 & show result */
	if (op_config_reading[4] != chip->pdata->op_config[4]) {
		dev_info(chip->dev, "%s: OPCFG5 mismatched\n", __func__);
		goto init_start;
	} else {
		dev_info(chip->dev, "%s: OPCFG5 matched\n", __func__);
	}
	/* set fc_vth + fc_ith */
	fc_target1 = (chip->pdata->fc_vth[chip->ocv_index]) |
		(chip->pdata->fc_ith << 8);
	rt9426_write_page_cmd(chip, RT9426_PAGE_5);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW3, fc_target1);

	/* set fc_sth */
	fc_target2 = 0x4100 | (chip->pdata->fc_sth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW4, fc_target2);

	/* set fd_vth */
	fd_target1 = 0x1200 | (chip->pdata->fd_vth);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_SWINDOW6, fd_target1);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	usleep_range(5000, 5100); /* delay 5ms */
	/* add read back check for fc_vth/fc_ith/fc_sth/fd_vth */
	rt9426_read_page_cmd(chip, RT9426_PAGE_5);

	/* check fc_vth & fc_ith */
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW3);
	if (ret == fc_target1) {
		dev_info(chip->dev, "%s: fc_vth & fc_ith matched\n", __func__);
	} else {
		dev_info(chip->dev, "%s: fc_vth & fc_ith mismatched\n", __func__);
		/* add retry after read back check failed */
		goto init_start;
	}

	/* check fc_sth */
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW4);
	if (ret == fc_target2) {
		dev_info(chip->dev, "%s: fc_sth matched\n", __func__);
	} else {
		dev_info(chip->dev, "%s: fc_sth mismatched\n", __func__);
		/* add retry after read back check failed */
		goto init_start;
	}

	/* check fd_vth */
	ret = rt9426_reg_read_word(chip->i2c, RT9426_REG_SWINDOW6);
	if (ret == fd_target1) {
		dev_info(chip->dev, "%s: fd_vth matched\n", __func__);
	} else {
		dev_info(chip->dev, "%s: fd_vth mismatched\n", __func__);
		/* add retry after read back check failed */
		goto init_start;
	}
	/* set EXTREG */
	ret = chip->pdata->extreg_size;
	dev_info(chip->dev, "CHIP_INIT => EXTREG_SIZE=<%d>\n", ret);
	for (i = 0; i < ret; i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6550 +
		(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6550 +
		(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		usleep_range(1000, 1100); /* delay 1ms */
		rt9426_reg_write_word(chip->i2c,
			chip->pdata->extreg_update.extreg_update_data[i].extreg_addr,
			chip->pdata->extreg_update.extreg_update_data[i].extreg_data);
		usleep_range(5000, 5100); /* delay 5ms */
	}
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	/* check EXTREG: 0x6500 for write page cmd */
	ret = chip->pdata->extreg_size;
	for (i = 0; i < ret; i++) {
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6500 +
			(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		rt9426_reg_write_word(chip->i2c, RT9426_REG_EXTREGCNTL, 0x6500 +
			(chip->pdata->extreg_update.extreg_update_data[i].extreg_page));
		usleep_range(5000, 5100); /* delay 5ms */
		regval = rt9426_reg_read_word(chip->i2c,
			chip->pdata->extreg_update.extreg_update_data[i].extreg_addr);
		/* check the readback value */
		if (regval == chip->pdata->extreg_update.extreg_update_data[i].extreg_data) {
			dev_info(chip->dev, "%s: data[%d] matched\n", __func__, i);
		} else {
			dev_info(chip->dev, "%s: data[%d] mismatched\n", __func__, i);
			/* add retry after read back check failed */
			goto init_start;
		}
	}

	if (chip->ocv_checksum_ic != chip->ocv_checksum_dtsi) {
		if (rt9426_get_current(chip) > 0) {
			volt_now = rt9426_reg_read_word(chip->i2c, RT9426_REG_VBAT);
			fd_vth_now = chip->pdata->fd_vth;
			fd_threshold = RT9426_FD_BASE + 5 * (fd_vth_now);
			dev_info(chip->dev, "RT9426 VBAT = %d\n", volt_now);
			dev_info(chip->dev, "RT9426 FD_VTH = %d\n", fd_vth_now);
			dev_info(chip->dev, "RT9426 FD Threshold = %d\n", fd_threshold);

			if (volt_now > fd_threshold) {
				/* disable battery charging path before QS command */
				rt9426_request_charging_inhibit(true);
				dev_info(chip->dev, "Enable Charging Inhibit and delay 1250ms\n");
				mdelay(1250);
			}
		}
		rt9426_reg_write_word(chip->i2c, RT9426_REG_CNTL, 0x4000);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0x0000);
		mdelay(5);
		dev_info(chip->dev, "OCV checksum are different, QS is done.\n");
		/* Power path recover check */
		if (volt_now > fd_threshold) {
			/* enable battery charging path after QS command */
			rt9426_request_charging_inhibit(false);
			dev_info(chip->dev, "Disable Charging Inhibit\n");
		}
	} else {
		dev_info(chip->dev, "OCV checksum are the same, bypass QS.\n");
	}
	/* clear RI, set 0 to RI bits */
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	if (regval < 0)
		return -1;
	regval = (regval & ~RT9426_RI_MASK);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_FLAG3, regval);
	rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_FLAG3);
	if (regval < 0)
		return -1;
	if (((regval & RT9426_RI_MASK) >> 8) == 0)
		dev_info(chip->dev, "RT9426 RI=0\n");
	else
		dev_info(chip->dev, "RT9426 RI=1\n");

	regval = rt9426_reg_read_word(chip->i2c, RT9426_REG_RSVD_FLAG);
	if (regval < 0)
		return -1;
	if ((regval & 0x7FFF) != chip->pdata->rt_config_ver) {
		regval = (regval & 0x8000) | chip->pdata->rt_config_ver;
		rt9426_reg_write_word(chip->i2c, RT9426_REG_RSVD_FLAG, regval);
		rt9426_reg_write_word(chip->i2c, RT9426_REG_DUMMY, 0);
	}

out_apply:
	chip->online = 1;
	return 0;
}

static void rt9426_parse_batt_ntc(struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int array_len;
	int i;
	long idata = 0;
	const char *string = NULL;
	int ret;

	if (!np)
		return;
	if (of_property_read_u32(np, "ntc_compensation_is",
		&(pdata->ntc_compensation_is))) {
		hwlog_info("get ntc_compensation_is failed\n");
		return;
	}
	array_len = of_property_count_strings(np, "ntc_temp_compensation_para");
	if ((array_len <= 0) || (array_len % NTC_PARA_TOTAL != 0)) {
		hwlog_err("ntc is invaild,please check ntc_temp_para number\n");
		return;
	}
	if (array_len > NTC_PARA_LEVEL * NTC_PARA_TOTAL) {
		array_len = NTC_PARA_LEVEL * NTC_PARA_TOTAL;
		hwlog_err("temp is too long use only front %d paras\n", array_len);
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"ntc_temp_compensation_para", i, &string);
		if (ret) {
			hwlog_err("get ntc_temp_compensation_para failed\n");
			return;
		}
		/* 10 means decimalism */
		ret = kstrtol(string, 10, &idata);
		if (ret)
			break;

		switch (i % NTC_PARA_TOTAL) {
		case NTC_PARA_ICHG:
			pdata->ntc_temp_compensation_para[i / NTC_PARA_TOTAL]
				.refer = idata;
			break;
		case NTC_PARA_VALUE:
			pdata->ntc_temp_compensation_para[i / NTC_PARA_TOTAL]
				.comp_value = idata;
			break;
		default:
			hwlog_err("ntc_temp_compensation_para get failed\n");
		}
		hwlog_info("ntc_temp_compensation_para[%d][%d] = %ld\n",
			i / (NTC_PARA_TOTAL), i % (NTC_PARA_TOTAL), idata);
	}
}

struct dt_offset_params {
	int data[RT9426_DT_OFFSET_PARA_SIZE];
};

struct dt_extreg_params {
	int edata[RT9426_DT_EXTREG_PARA_SIZE];
};

static struct device_node *rt9426_get_child_node(struct device *dev,
	struct rt9426_platform_data *pdata)
{
	const char *battery_name = NULL;
	const char *batt_model_name = NULL;
	struct device_node *np = dev->of_node;
	struct device_node *child_node = NULL;
	struct device_node *default_node = NULL;

	batt_model_name = bat_model_name();
	for_each_child_of_node(np, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG),
			child_node, "batt_name", &battery_name)) {
			dev_info(dev, "childnode without batt_name property");
			continue;
		}
		if (!battery_name)
			continue;
		if (!default_node)
			default_node = child_node;
		dev_info(dev, "search battery data, battery_name: %s\n", battery_name);
		if (!batt_model_name || !strcmp(battery_name, batt_model_name))
			break;
	}

	if (!child_node) {
		if (default_node) {
			dev_info(dev, "cannt match childnode, use first\n");
			child_node = default_node;
		} else {
			dev_info(dev, "cannt find any childnode, use father\n");
			child_node = np;
		}
	}

	return child_node;
}

static void rt9426_parse_source(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,volt_source", &pdata->volt_source, RT9426_REG_AV);

	if (pdata->volt_source == RT9426_VOLT_SOURCE_NONE) {
		pdata->volt_source = 0;
	} else if (pdata->volt_source == RT9426_VOLT_SOURCE_VBAT) {
		pdata->volt_source = RT9426_REG_VBAT;
	} else if (pdata->volt_source == RT9426_VOLT_SOURCE_OCV) {
		pdata->volt_source = RT9426_REG_OCV;
	} else if (pdata->volt_source == RT9426_VOLT_SOURCE_AV) {
		pdata->volt_source = RT9426_REG_AV;
	} else {
		dev_notice(dev, "pdata->volt_source is out of range, use 3\n");
		pdata->volt_source = RT9426_REG_AV;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,temp_source", &pdata->temp_source, 0);

	if (pdata->temp_source == RT9426_TEMP_SOURCE_NONE)
		pdata->temp_source = 0;
	else if (pdata->temp_source == RT9426_TEMP_SOURCE_TEMP)
		pdata->temp_source = RT9426_REG_TEMP;
	else if (pdata->temp_source == RT9426_TEMP_SOURCE_INIT)
		pdata->temp_source = RT9426_REG_INTT;
	else if (pdata->temp_source == RT9426_TEMP_SOURCE_AT)
		pdata->temp_source = RT9426_REG_AT;
	else
		pdata->temp_source = 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,curr_source", &pdata->curr_source, 0);

	if (pdata->curr_source == RT9426_CURR_SOURCE_NONE)
		pdata->curr_source = 0;
	else if (pdata->curr_source == RT9426_CURR_SOURCE_CURR)
		pdata->curr_source = RT9426_REG_CURR;
	else if (pdata->curr_source == RT9426_CURR_SOURCE_AI)
		pdata->curr_source = RT9426_REG_AI;
	else
		pdata->curr_source = RT9426_REG_AI;
}

static void rt9426_parse_threshold(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otc_tth", &pdata->otc_tth, RT9426_OTC_TTH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otc_chg_ith", &pdata->otc_chg_ith, RT9426_OTC_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otd_tth", &pdata->otd_tth, RT9426_OTD_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_otd_dchg_ith", &pdata->otd_dchg_ith, RT9426_OTD_DCHG_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_uvov_threshold", &pdata->uv_ov_threshold, RT9426_UV_OV_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_us_threshold", &pdata->us_threshold, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_ith", &pdata->fc_ith, RT9426_FC_ITH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fc_sth", &pdata->fc_sth, RT9426_FC_STH_DEFAULT_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rt,fg_fd_vth", &pdata->fd_vth, RT9426_FD_VTH_DEFAULT_VAL);
}

static int rt9426_parse_soc_offset(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	struct dt_offset_params *offset_params = NULL;
	int sizes[RT9426_SOC_OFFSET_SIZE + 1] = { 0 }; /* 1:for boundary protection */

	ret = of_property_read_u32_array(np, "rt,offset_interpolation_order",
		pdata->offset_interpolation_order, RT9426_OFFSET_INTERPLO_SIZE);
	if (ret < 0)
		pdata->offset_interpolation_order[0] =
			pdata->offset_interpolation_order[1] = 2; /* 2: interpolation order */

	sizes[0] = sizes[1] = 0;
	ret = of_property_read_u32_array(np, "rt,soc_offset_size",
		sizes, RT9426_SOC_OFFSET_SIZE);
	if (ret < 0)
		dev_notice(dev, "cannt get prop soc_offset_size %d\n", ret);

	new_vgcomp_soc_offset_datas(dev, SOC_OFFSET, pdata, sizes[0], sizes[1], 0);
	if (!pdata->soc_offset.soc_offset_data)
		return 0;

	offset_params = devm_kzalloc(dev,
		sizes[0] * sizes[1] * sizeof(struct dt_offset_params), GFP_KERNEL);
	if (!offset_params)
		return -1;

	of_property_read_u32_array(np, "rt,soc_offset_data",
		(u32 *)offset_params,
		sizes[0] * sizes[1] * (RT9426_SOC_OFFSET_SIZE + 1));
	for (i = 0; i < sizes[0] * sizes[1]; i++) {
		pdata->soc_offset.soc_offset_data[i].x =
			offset_params[i].data[0];
		pdata->soc_offset.soc_offset_data[i].y =
			offset_params[i].data[1];
		pdata->soc_offset.soc_offset_data[i].offset =
			offset_params[i].data[2];
	}
	devm_kfree(dev, offset_params);

	return 0;
}

static int rt9426_parse_extreg(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	struct dt_extreg_params *extreg_params = NULL;

	ret = of_property_read_u32(np, "rt,extreg_size", &pdata->extreg_size);
	if (ret < 0)
		dev_notice(dev, "cannt get prop extreg_size %d\n", ret);

	dev_info(dev, "Parse EXTREG_SIZE=%d\n", pdata->extreg_size);
	new_vgcomp_soc_offset_datas(dev, EXTREG_UPDATE, pdata, pdata->extreg_size, 0, 0);
	if (!pdata->extreg_update.extreg_update_data)
		return 0;

	extreg_params = devm_kzalloc(dev,
		pdata->extreg_size * sizeof(struct dt_extreg_params), GFP_KERNEL);
	if (!extreg_params)
		return -1;

	of_property_read_u32_array(np, "rt,extreg_data",
		(u32 *)extreg_params, pdata->extreg_size * 3);
	for (i = 0; i < pdata->extreg_size;  ++i) {
		pdata->extreg_update.extreg_update_data[i].extreg_page =
			extreg_params[i].edata[0];
		pdata->extreg_update.extreg_update_data[i].extreg_addr =
			extreg_params[i].edata[1];
		pdata->extreg_update.extreg_update_data[i].extreg_data =
			extreg_params[i].edata[2];
	}
	devm_kfree(dev, extreg_params);

	dev_info(dev, "EXTREG_DATA Parse OK\n");

	return 0;
}

static void rt9426_parse_ocv_table(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int i;
	int ret;
	char prop_name[RT9426_WRITE_BUF_LEN + 1] = { 0 };

	if (pdata->ocv_vterm_size > 1)
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			"rt,fg_ocv_table0",
			(u32 *)pdata->ocv_table, RT9426_OCV_DATA_TOTAL_SIZE);
	else
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			"rt,fg_ocv_table",
			(u32 *)pdata->ocv_table, RT9426_OCV_DATA_TOTAL_SIZE);
	if (ret < 0) {
		memset32((u32 *)pdata->ocv_table, 0, RT9426_OCV_DATA_TOTAL_SIZE);
		return;
	}
	for (i = 1; i < pdata->ocv_vterm_size; i++) {
		snprintf(prop_name, RT9426_WRITE_BUF_LEN, "rt,fg_ocv_table%d", i);
		ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
			prop_name, (u32 *)pdata->ocv_table + i *
			RT9426_OCV_DATA_TOTAL_SIZE, RT9426_OCV_DATA_TOTAL_SIZE);
		if (ret < 0)
			memset32((u32 *)pdata->ocv_table + i *
				RT9426_OCV_DATA_TOTAL_SIZE, 0,
				RT9426_OCV_DATA_TOTAL_SIZE);
	}
}

static void rt9426_parse_op_config(struct device *dev, struct device_node *np,
	struct rt9426_platform_data *pdata)
{
	int ret;

	ret = of_property_read_u32_array(np, "rt,fg_op_config",
		(u32 *)pdata->op_config, RT9426_OP_CONFIG_SIZE);
	if (ret < 0) {
		dev_notice(dev, "no fg op config proeprty, use default\n");
		pdata->op_config[0] = RT9426_OP_CONFIG_0_DEFAULT_VAL;
		pdata->op_config[1] = RT9426_OP_CONFIG_1_DEFAULT_VAL;
		pdata->op_config[2] = RT9426_OP_CONFIG_2_DEFAULT_VAL;
		pdata->op_config[3] = RT9426_OP_CONFIG_3_DEFAULT_VAL;
		pdata->op_config[4] = RT9426_OP_CONFIG_4_DEFAULT_VAL;
	}
}

static int rt9426_parse_sub_param(struct device *dev,
	struct rt9426_platform_data *pdata)
{
	int ret;
	struct device_node *child_node = rt9426_get_child_node(dev, pdata);

	ret = of_property_count_u32_elems(child_node, "rt,fcc");
	if (ret < 1)
		ret = 1;
	pdata->ocv_vterm_size = ret;

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), child_node,
		"rt,fcc", pdata->fcc, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no fcc property, use default 2000\n");
		memset32(pdata->fcc, RT9426_DESIGN_FCC_VAL,
			pdata->ocv_vterm_size);
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), child_node,
		"rt,fg_fc_vth", pdata->fc_vth, pdata->ocv_vterm_size);
	if (ret < 0) {
		dev_notice(dev, "no fc_vth property, use default 4200mV\n");
		memset32(pdata->fc_vth, RT9426_FC_VTH_DEFAULT_VAL,
			pdata->ocv_vterm_size);
	}

	ret = of_property_read_u32_array(child_node, "rt,dtsi_version",
		pdata->dtsi_version, RT9426_DTSI_VER_SIZE);
	if (ret < 0)
		pdata->dtsi_version[0] = pdata->dtsi_version[1] = 0;

	of_property_read_u32(child_node, "rt,rs_ic_setting", &pdata->rs_ic_setting);
	of_property_read_u32(child_node, "rt,rs_schematic", &pdata->rs_schematic);
	dev_info(dev, "rs_ic_setting = %d\n", pdata->rs_ic_setting);
	dev_info(dev, "rs_schematic = %d\n", pdata->rs_schematic);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
		"rt,battery_type", &pdata->battery_type, 4352); /* 4352: default battery type */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
		"rt,design_capacity", &pdata->design_capacity, RT9426_DESIGN_CAP_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
		"rt,fg_curr_db", &pdata->curr_db, RT9426_CURR_DB_DEFAULT_VAL);

	rt9426_parse_threshold(dev, child_node, pdata);
	rt9426_parse_source(dev, child_node, pdata);
	rt9426_parse_ocv_table(dev, child_node, pdata);
	rt9426_parse_op_config(dev, child_node, pdata);

	ret = rt9426_parse_soc_offset(dev, child_node, pdata);
	ret += rt9426_parse_extreg(dev, child_node, pdata);

	return ret;
}

static int rt9426_parse_dt(struct device *dev, struct rt9426_platform_data *pdata)
{
	int ret;
	struct device_node *np = dev->of_node;
	const char *bat_name = "rt-fuelguage";

	dev_info(dev, "%s\n", __func__);

	pdata->intr_gpio = of_get_named_gpio(np, "rt9426,gpio-intb", 0);
	if (!gpio_is_valid(pdata->intr_gpio)) {
		dev_info(dev, "get intr_gpio fail\n");
		return -1;
	}
	dev_info(dev, "get intr_gpio %d\n", pdata->intr_gpio);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "rt_config_ver",
		&pdata->rt_config_ver, RT9426_DRIVER_VER);
	rt9426_parse_batt_ntc(np, pdata);
	pdata->force_use_aux_cali_para =
		of_property_read_bool(np, "force_use_aux_cali_para");

	ret = of_property_read_string(np, "rt,bat_name", (char const **)&bat_name);
	if (ret == 0)
		pdata->bat_name = kasprintf(GFP_KERNEL, "%s", bat_name);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "need_reset_version",
		&pdata->need_reset_version, 4); /* 4 default value */

	return rt9426_parse_sub_param(dev, pdata);
}

static int rt9426_i2c_chipid_check(struct i2c_client *i2c)
{
	u16 ver;
	int ret;

	ret = i2c_smbus_read_i2c_block_data(i2c, RT9426_REG_VER, 2, (u8 *)&ver);
	if (ret < 0)
		return ret;
	ver = le16_to_cpu(ver);
	if ((ver & 0xff00) != 0x0000) { /* get version */
		dev_notice(&i2c->dev, "chip id not match\n");
		return -ENODEV;
	}
	return ver;
}

static int rt9426_irq_register(struct rt9426_chip *chip)
{
	int ret;

	ret = devm_gpio_request_one(chip->dev, chip->pdata->intr_gpio,
		GPIOF_IN, "rt9426_irq_gpio");
	if (ret < 0) {
		hwlog_err("gpio request fail\n");
		return ret;
	}

	ret = gpio_to_irq(chip->pdata->intr_gpio);
	if (ret < 0) {
		hwlog_err("irq mapping fail\n");
		return ret;
	}
	chip->alert_irq = ret;
	hwlog_info("irq = %d\n", chip->alert_irq);

	/* Request threaded IRQ */
	ret = devm_request_threaded_irq(chip->dev, chip->alert_irq, NULL,
		rt9426_irq_handler, IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
		"rt9246_irq", chip);
	if (ret < 0) {
		hwlog_err("request thread irq fail\n");
		gpio_free(chip->pdata->intr_gpio);
		return ret;
	}
	device_init_wakeup(chip->dev, true);

	return 0;
}

static int rt9426_is_ready(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return 1;
}

static int rt9426_set_ntc_compensation_temp(struct rt9426_platform_data *pdata,
	int temp_val, int cur_temp)
{
	int temp_with_compensation = temp_val;
	struct common_comp_data comp_data;

	if (!pdata)
		return temp_with_compensation;

	comp_data.refer = abs(cur_temp);
	comp_data.para_size = NTC_PARA_LEVEL;
	comp_data.para = pdata->ntc_temp_compensation_para;
	if ((pdata->ntc_compensation_is == 1) &&
		(temp_val >= COMPENSATION_THRESHOLD))
		temp_with_compensation = power_get_compensation_value(temp_val,
			&comp_data);

	hwlog_debug("temp_with_compensation=%d temp_no_compensation=%d ichg=%d\n",
		temp_with_compensation, temp_val, cur_temp);
	return temp_with_compensation;
}

static int rt9426_read_battery_temperature(void *dev_data)
{
	int temp;
	int bat_curr;

	if (!g_rt9426_chip)
		return 0;

	temp = rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_TEMP);
	bat_curr = rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_IBAT);

	return rt9426_set_ntc_compensation_temp(g_rt9426_chip->pdata,
		temp, bat_curr);
}

#ifdef CONFIG_HLTHERM_RUNTEST
static int rt9426_is_battery_exist(void *dev_data)
{
	return 0;
}
#else
static int rt9426_is_battery_exist(void *dev_data)
{
	int temp;

	temp = rt9426_read_battery_temperature(dev_data);
	if ((temp <= RT9426_TEMP_ABR_LOW) || (temp >= RT9426_TEMP_ABR_HIGH))
		return 0;

	return 1;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int rt9426_read_battery_soc(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_SOC);
}

static int rt9426_read_battery_vol(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_VBAT);
}

static int rt9426_read_battery_current(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_IBAT);
}

static int rt9426_read_battery_avg_current(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_AVG_IBAT);
}

static int rt9426_read_battery_fcc(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_FCC);
}

static int rt9426_read_battery_cycle(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_cyccnt(g_rt9426_chip);
}

static int rt9426_read_battery_rm(void *dev_data)
{
	if (!g_rt9426_chip)
		return 0;

	return rt9426_get_display_data(g_rt9426_chip, RT9426_DISPLAY_RM);
}

static int rt9426_set_battery_low_voltage(int val, void *dev_data)
{
	int uv_set;
	int reg_val;
	u16 wr_val;

	if (!g_rt9426_chip)
		return -1;

	/* 2400: val = 2400mV + 10mV* uv_set */
	uv_set = (val - 2400) / 10;
	if (uv_set < 0)
		return -1;

	rt9426_read_page_cmd(g_rt9426_chip, RT9426_PAGE_3);
	reg_val = rt9426_reg_read_word(g_rt9426_chip->i2c, RT9426_REG_SWINDOW5);
	if (reg_val < 0)
		return -1;
	wr_val = ((unsigned int)uv_set << RT9426_BYTE_BITS) & RT9426_HIGH_BYTE_MASK;
	wr_val |= (unsigned int)reg_val & RT9426_LOW_BYTE_MASK;

	dev_notice(g_rt9426_chip->dev, "uv_set=0x%x, reg_val=0x%x, wr_val=0x%x\n",
		uv_set, reg_val, wr_val);
	rt9426_write_page_cmd(g_rt9426_chip, RT9426_PAGE_3);
	return rt9426_reg_write_word(g_rt9426_chip->i2c,
		RT9426_REG_SWINDOW5, wr_val);
}

static int rt9426_set_last_capacity(int capacity, void *dev_data)
{
	if ((capacity > RT9426_FULL_CAPCACITY) || (capacity < 0) || !g_rt9426_chip)
		return 0;

	return rt9426_reg_write_word(g_rt9426_chip->i2c,
		RT9426_EXTEND_REG, capacity);
}

static int rt9426_get_last_capacity(void *dev_data)
{
	int last_cap;
	int cap;

	if (!g_rt9426_chip)
		return last_cap;

	last_cap = rt9426_reg_read_word(g_rt9426_chip->i2c, RT9426_EXTEND_REG);
	cap = rt9426_read_battery_soc(dev_data);

	dev_notice(g_rt9426_chip->dev, "%s read cap=%d, last_cap=%d\n",
		__func__, cap, last_cap);

	if ((last_cap <= 0) || (cap <= 0))
		return cap;

	if (abs(last_cap - cap) >= RT9426_CAPACITY_TH)
		return cap;

	/* reset last capacity */
	rt9426_reg_write_word(g_rt9426_chip->i2c, RT9426_EXTEND_REG, 0);

	return last_cap;
}

static int rt9426_set_vterm_dec(int vterm, void *dev_data)
{
	int ocv_index;

	if (!g_rt9426_chip)
		return -1;

	ocv_index = (vterm + RT9426_VTERM_INCREASE - 1) / RT9426_VTERM_INCREASE;
	if (ocv_index >= g_rt9426_chip->pdata->ocv_vterm_size)
		ocv_index = g_rt9426_chip->pdata->ocv_vterm_size - 1;
	if (ocv_index < 0)
		ocv_index = 0;

	if (rt9426_unseal_wi_retry(g_rt9426_chip) == RT9426_UNSEAL_FAIL)
		return -EIO;
	return rt9426_set_ocv_table(g_rt9426_chip, ocv_index);
}

static struct coul_interface_ops rt9426_ops = {
	.type_name = "main",
	.is_coul_ready = rt9426_is_ready,
	.is_battery_exist = rt9426_is_battery_exist,
	.get_battery_capacity = rt9426_read_battery_soc,
	.get_battery_voltage = rt9426_read_battery_vol,
	.get_battery_current = rt9426_read_battery_current,
	.get_battery_avg_current = rt9426_read_battery_avg_current,
	.get_battery_temperature = rt9426_read_battery_temperature,
	.get_battery_fcc = rt9426_read_battery_fcc,
	.get_battery_cycle = rt9426_read_battery_cycle,
	.set_battery_low_voltage = rt9426_set_battery_low_voltage,
	.set_battery_last_capacity = rt9426_set_last_capacity,
	.get_battery_last_capacity = rt9426_get_last_capacity,
	.get_battery_rm = rt9426_read_battery_rm,
	.set_vterm_dec = rt9426_set_vterm_dec,
};

static int rt9426_get_calibration_curr(int *val, void *dev_data)
{
	if (!val || !g_rt9426_chip) {
		hwlog_err("invalid val or g_rt9426_chip\n");
		return -1;
	}

	*val = rt9426_get_curr_by_conversion(g_rt9426_chip);
	dev_notice(g_rt9426_chip->dev, "cali cur %d\n", *val);
	return 0;
}

static int rt9426_get_calibration_vol(int *val, void *dev_data)
{
	if (!val || !g_rt9426_chip) {
		hwlog_err("invalid val or g_rt9426_chip\n");
		return -1;
	}

	*val = rt9426_get_volt_by_conversion(g_rt9426_chip);
	*val *= 1000; /* mv to uv */
	dev_notice(g_rt9426_chip->dev, "cali vol %d\n", *val);
	return 0;
}

static int rt9426_set_current_gain(unsigned int val, void *dev_data)
{
	if (!g_rt9426_chip) {
		hwlog_err("invalid g_rt9426_chip\n");
		return -1;
	}

	val = RT9426_GAIN_DEFAULT_VAL + (((s64)(val) * RT9426_GAIN_BASE_VAL) /
		RT9426_COUL_DEFAULT_VAL - RT9426_GAIN_BASE_VAL);

	g_rt9426_chip->c_gain = val;
	rt9426_apply_calibration_para(g_rt9426_chip, RT9426_GAIN_DEFAULT_VAL,
		g_rt9426_chip->c_gain, g_rt9426_chip->v_gain);
	dev_notice(g_rt9426_chip->dev, "cur gain %d\n", val);
	return 0;
}

static int rt9426_set_voltage_gain(unsigned int val, void *dev_data)
{
	if (!g_rt9426_chip) {
		hwlog_err("invalid g_rt9426_chip\n");
		return -1;
	}

	val = RT9426_GAIN_DEFAULT_VAL + (((s64)(val) * RT9426_GAIN_BASE_VAL) /
		RT9426_COUL_DEFAULT_VAL - RT9426_GAIN_BASE_VAL);

	g_rt9426_chip->v_gain = val;
	rt9426_apply_calibration_para(g_rt9426_chip, RT9426_GAIN_DEFAULT_VAL,
		g_rt9426_chip->c_gain, g_rt9426_chip->v_gain);
	dev_notice(g_rt9426_chip->dev, "voltage gain %d\n", val);
	return 0;
}

static int rt9426_enable_cali_mode(int enable, void *dev_data)
{
	if (!g_rt9426_chip)
		return -1;

	if (enable)
		rt9426_enter_calibration_mode(g_rt9426_chip);
	else
		rt9426_exit_calibration_mode(g_rt9426_chip);
	return 0;
}

static struct coul_cali_ops rt9426_cali_ops = {
	.dev_name = "aux",
	.get_current = rt9426_get_calibration_curr,
	.get_voltage = rt9426_get_calibration_vol,
	.set_current_gain = rt9426_set_current_gain,
	.set_voltage_gain = rt9426_set_voltage_gain,
	.set_cali_mode = rt9426_enable_cali_mode,
};

static int rt9426_calibration_para_invalid(int c_gain, int v_gain)
{
	return ((c_gain < RT9426_TBATICAL_MIN_A) ||
		(c_gain > RT9426_TBATICAL_MAX_A) ||
		(v_gain < RT9426_TBATCAL_MIN_A) ||
		(v_gain > RT9426_TBATCAL_MAX_A));
}

static int rt9426_get_data_from_int(int val)
{
	return RT9426_GAIN_DEFAULT_VAL + (((s64)(val) * RT9426_GAIN_BASE_VAL) /
		RT9426_COUL_DEFAULT_VAL - RT9426_GAIN_BASE_VAL);
}

static void rt9426_init_calibration_para(struct rt9426_chip *chip)
{
	int c_a = 0;
	int v_a = 0;

	chip->c_gain = RT9426_GAIN_DEFAULT_VAL;
	chip->v_gain = RT9426_GAIN_DEFAULT_VAL;

	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &c_a);
	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_VOL_A, &v_a);

	if (chip->pdata->force_use_aux_cali_para && (c_a != 0) && (v_a != 0)) {
		dev_notice(g_rt9426_chip->dev, "force_use_aux_cali_para\n");

		if (c_a < RT9426_TBATICAL_MIN_A)
			c_a = RT9426_TBATICAL_MIN_A;
		else if (c_a > RT9426_TBATICAL_MAX_A)
			c_a = RT9426_TBATICAL_MAX_A;

		if (v_a < RT9426_TBATCAL_MIN_A)
			v_a = RT9426_TBATCAL_MIN_A;
		else if (v_a > RT9426_TBATCAL_MAX_A)
			v_a = RT9426_TBATCAL_MAX_A;
	}

	if (rt9426_calibration_para_invalid(c_a, v_a)) {
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_A, &c_a);
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_VOL_A, &v_a);
		if (rt9426_calibration_para_invalid(c_a, v_a))
			goto update;
	}

	chip->c_gain = rt9426_get_data_from_int(c_a);
	chip->v_gain = rt9426_get_data_from_int(v_a);

	dev_notice(chip->dev, "c_gain %d, v_gain %d\n",
		chip->c_gain, chip->v_gain);
update:
	rt9426_apply_calibration_para(chip, RT9426_GAIN_DEFAULT_VAL, chip->c_gain, chip->v_gain);
}

static int rt9426_i2c_probe(struct i2c_client *i2c,
	const struct i2c_device_id *id)
{
	struct rt9426_platform_data *pdata = i2c->dev.platform_data;
	struct rt9426_chip *chip = NULL;
	struct power_supply_config psy_config = {};
	u16 ic_ver;
	int ret;

	/* check chip id first */
	ret = rt9426_i2c_chipid_check(i2c);
	if (ret < 0) {
		dev_notice(&i2c->dev, "chip id check fail\n");
		return ret;
	}
	ic_ver = (u16)ret;
	/* alloc memory */
	chip = devm_kzalloc(&i2c->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	pdata = devm_kzalloc(&i2c->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	rt9426_parse_dt(&i2c->dev, pdata);
	chip->pdata = i2c->dev.platform_data = pdata;

	chip->i2c = i2c;
	chip->dev = &i2c->dev;
	chip->btemp = RT9426_BAT_TEMP_VAL;
	chip->bvolt = RT9426_BAT_VOLT_VAL;
	chip->bcurr = RT9426_BAT_CURR_VAL;
	chip->ic_ver = ic_ver;
	chip->ocv_checksum_ic = 0;
	chip->ocv_checksum_dtsi = pdata->ocv_table[9].data[4];
	chip->low_v_smooth_en = false;

	g_rt9426_chip = chip;

	mutex_init(&chip->var_lock);
	i2c_set_clientdata(i2c, chip);
	rt9426_apply_sense_resistor(chip);

	/* rt regmap init */
	ret = rt9426_regmap_init(chip);
	if (ret < 0) {
		dev_notice(chip->dev, "regmap init fail\n");
		return ret;
	}
	/* apply platform data */
	ret = rt9426_apply_pdata(chip);
	if (ret < 0) {
		dev_notice(chip->dev, "apply pdata fail\n");
		return ret;
	}
	/* fg psy register */
	psy_config.of_node = i2c->dev.of_node;
	psy_config.drv_data = chip;
	if (pdata->bat_name)
		fg_psy_desc.name = pdata->bat_name;
	chip->fg_psy = devm_power_supply_register(&i2c->dev,
		&fg_psy_desc, &psy_config);
	if (IS_ERR(chip->fg_psy)) {
		dev_notice(chip->dev, "register batt psy fail\n");
		return PTR_ERR(chip->fg_psy);
	}
	rt9426_create_attrs(&chip->fg_psy->dev);
	/* mask irq before irq register */
	ret = rt9426_irq_enable(chip, false);
	if (ret < 0) {
		dev_notice(chip->dev, "scirq mask fail\n");
		return ret;
	}

	ret = rt9426_irq_register(chip);
	if (ret < 0) {
		dev_notice(chip->dev, "irq init fail\n");
		return ret;
	}

	ret = rt9426_irq_enable(chip, true);
	if (ret < 0) {
		dev_notice(chip->dev, "scirq mask fail\n");
		free_irq(chip->alert_irq, chip);
		gpio_free(chip->pdata->intr_gpio);
		return ret;
	}

	rt9426_create_debug_files(chip);

	rt9426_fg_ops.dev_data = (void *)chip;
	power_log_ops_register(&rt9426_fg_ops);

	coul_interface_ops_register(&rt9426_ops);
	rt9426_init_calibration_para(chip);
	coul_cali_ops_register(&rt9426_cali_ops);
	rt9426_dump_register(chip);
	dev_info(chip->dev, "chip ver = 0x%04x\n", chip->ic_ver);
	return 0;
}

static int rt9426_i2c_remove(struct i2c_client *i2c)
{
	struct rt9426_chip *chip = i2c_get_clientdata(i2c);

	dev_info(chip->dev, "%s\n", __func__);
	g_rt9426_chip = NULL;
	rt9426_irq_enable(chip, false);
	rt9426_irq_deinit(chip);
	mutex_destroy(&chip->var_lock);
	return 0;
}

static int rt9426_i2c_suspend(struct device *dev)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev);

	dev_dbg(chip->dev, "%s\n", __func__);
	if (device_may_wakeup(dev))
		enable_irq_wake(chip->alert_irq);
	return 0;
}

static int rt9426_i2c_resume(struct device *dev)
{
	struct rt9426_chip *chip = dev_get_drvdata(dev);

	dev_dbg(chip->dev, "%s\n", __func__);
	if (device_may_wakeup(dev))
		disable_irq_wake(chip->alert_irq);
	return 0;
}

static SIMPLE_DEV_PM_OPS(rt9426_pm_ops, rt9426_i2c_suspend, rt9426_i2c_resume);

static const struct i2c_device_id rt9426_i2c_id[] = {
	{ "rt9426", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, rt9426_i2c_id);

static const struct of_device_id rt9426_match_table[] = {
	{ .compatible = "richtek,rt9426", },
	{},
};
MODULE_DEVICE_TABLE(of, rt9426_match_table);

static struct i2c_driver rt9426_i2c_driver = {
	.driver = {
		.name = "rt9426",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(rt9426_match_table),
		.pm = &rt9426_pm_ops,
	},
	.probe = rt9426_i2c_probe,
	.remove = rt9426_i2c_remove,
	.id_table = rt9426_i2c_id,
};
module_i2c_driver(rt9426_i2c_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("rt9426 Fuel Gauge Driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
