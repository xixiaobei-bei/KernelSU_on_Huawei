/*
 * hisi_scharger_v200_common_irq.c
 *
 * HI6522 charger driver common config and irq handler
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "hisi_scharger_v200_common_irq.h"
#include <hisi_scharger_v200.h>
#include <linux/notifier.h>
#include <linux/workqueue.h>
#include <huawei_platform/power/huawei_charger.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/irq_work.h>

static int hi6522_write_block(struct hi6522_device_info *di, u8 *value,
			      u8 reg, unsigned int num_bytes)
{
	struct i2c_msg msg[1]; /* 1: number of msg struct is 1 */
	int ret;

	*value = reg;
	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = value;
	msg[0].len = num_bytes + 1;

	ret = i2c_transfer(di->client->adapter, msg, 1);
	/* i2c_transfer returns number of messages transferred */
	if (ret != 1) {
		scharger_err("i2c_write failed to transfer all messages\n");
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

int hi6522_read_block(struct hi6522_device_info *di, u8 *value, u8 reg,
		      unsigned int num_bytes)
{
	struct i2c_msg msg[2]; /* 2: the length of msg is 2 */
	u8 buf;
	int ret;

	buf = reg;
	msg[0].addr = di->client->addr;
	msg[0].flags = 0;
	msg[0].buf = &buf;
	msg[0].len = 1;

	msg[1].addr = di->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = value;
	msg[1].len = num_bytes;

	ret = i2c_transfer(di->client->adapter, msg, 2); /* 2: the length of msg */
	/* i2c_transfer returns number of messages transferred */
	if (ret != 2) {
		scharger_err("i2c_read failed to transfer all messages\n");
		if (ret < 0)
			return ret;
		else
			return -EIO;
	} else {
		return 0;
	}
}

int hi6522_write_byte(u8 reg, u8 value)
{
	struct hi6522_device_info *di = get_hi6522_dev();
	/* 2 bytes offset 1 contains the data offset 0 is used by i2c_write */
	u8 temp_buffer[2] = {0};
	/* offset 1 contains the data */
	if (di == NULL) {
		scharger_err("%s hi6522_device_info is NULL!\n", __func__);
		return -ENOMEM;
	}
	temp_buffer[1] = value;
	return hi6522_write_block(di, temp_buffer, reg, 1);
}

int hi6522_read_byte(u8 reg, u8 *value)
{
	struct hi6522_device_info *di = get_hi6522_dev();

	if (di == NULL || value == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}

	return hi6522_read_block(di, value, reg, 1);
}

int hi6522_write_mask(u8 reg, u8 mask, u8 shift, u8 value)
{
	int ret;
	u8 val = 0;

	ret = hi6522_read_byte(reg, &val);
	if (ret < 0)
		return ret;

	val &= ~mask;
	val |= ((value << shift) & mask);

	ret = hi6522_write_byte(reg, val);

	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
int hi6522_read_mask(u8 reg, u8 mask, u8 shift, u8 *value)
{
	int ret;
	u8 val = 0;

	if (value == NULL) {
		scharger_err("%s input is NULL!\n", __func__);
		return -ENOMEM;
	}
	ret = hi6522_read_byte(reg, &val);
	if (ret < 0)
		return ret;
	val &= mask;
	val >>= shift;
	*value = val;

	return 0;
}
#endif

static void chk_irq_status_0(unsigned char irq_stat_0)
{
	if (irq_stat_0 & CHG_IRQ0_VBUS_FAST_CHG_FAULT) {
		scharger_err("SCHARGER ERROR:FAST_CHG_FAULT!,reg[0x07]:0x%x\n",
			     irq_stat_0);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_0 & CHG_IRQ0_VBUS_PRE_CHG_FAULT) {
		scharger_err("SCHARGER ERROR:PRE_CHG_FAULT!,reg[0x07]:0x%x\n",
			     irq_stat_0);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_0 & CHG_IRQ0_VBUS_TRICKLE_CHG_FAULT)
		scharger_err("SCHARGER ERROR:TRICKLE_CHG_FAULT!,reg[0x07]:0x%x\n",
			     irq_stat_0);
}

static void chk_irq_status_1_scp(const unsigned char irq_stat_tmp_1,
				 struct hi6522_device_info *di)
{
	if (irq_stat_tmp_1 & CHG_IRQ1_WLED_SCP) {
		di->wled_en = CHG_POWER_DIS;
		(void)hi6522_config_wled_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ1_WLED_SCP,reg[0x08]:0x%x\n",
			irq_stat_tmp_1);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}
}

static void chk_irq_status_1(unsigned char irq_stat_1, struct hi6522_device_info *di)
{
	static int wled_ovp_count, wled_uvp_count;

	if (irq_stat_1 & CHG_IRQ1_BUCK_SCP) {
			scharger_err("SCHARGER ERROR:BUCK_SCP,reg[0x08]:0x%x\n",
				     irq_stat_1);
			atomic_notifier_call_chain(&fault_notifier_list,
						   CHARGE_FAULT_SCHARGER,
						   NULL);
	}

	if (irq_stat_1 & CHG_IRQ1_WLED_OVP_38V) {
		if (!(strstr(saved_command_line, "androidboot.swtype=factory"))) {
			wled_ovp_count++;
			/* 0: enable chopper */
			(void)hi6522_config_enable_chopper(di, 0);

			if (wled_ovp_count == WLED_IRQ_ONCE_COUNT) {
				(void)hi6522_config_enable_ifb1_ifb2(
					di, wled_ovp_count);
				scharger_err(
					"SCHARGER ERROR:CHG_IRQ1_WLED_OVP_38V,wled_ovp_count:%d\n",
					wled_ovp_count);
			} else if (wled_ovp_count == WLED_IRQ_SECOND_COUNT) {
				(void)hi6522_config_enable_ifb1_ifb2(
					di, wled_ovp_count);
				scharger_err(
					"SCHARGER ERROR:CHG_IRQ1_WLED_OVP_38V,wled_ovp_count:%d\n",
					wled_ovp_count);
			} else if (wled_ovp_count == WLED_IRQ_THIRD_COUNT) {
				(void)hi6522_config_enable_chopper(di, 1);
				(void)hi6522_config_enable_ifb1_ifb2(di, wled_ovp_count);
				wled_ovp_count = 0;
				di->wled_en = CHG_POWER_DIS;
				(void)hi6522_config_wled_enable(di);
				scharger_err(
					"SCHARGER ERROR:CHG_IRQ1_WLED_OVP_38V,wled_ovp_count:%d\n",
					wled_ovp_count);
			}
		} else {
			di->wled_en = CHG_POWER_DIS;
			(void)hi6522_config_wled_enable(di);
			scharger_err("SCHARGER ERROR:CHG_IRQ1_WLED_OVP_38V, reg[0x08]:0x%x\n",
				     irq_stat_1);
		}
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_1 & CHG_IRQ1_WLED_OVP_4P5V) {
		di->wled_en = CHG_POWER_DIS;
		(void)hi6522_config_wled_enable(di);
		scharger_err("SCHARGER ERROR:CHG_IRQ1_WLED_OVP_4P5V, reg[0x08]:0x%x\n",
			     irq_stat_1);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_1 & CHG_IRQ1_WLED_UVP) {
		if (!(strstr(saved_command_line,
			"androidboot.swtype=factory"))) {
			wled_uvp_count++;
			(void)hi6522_config_enable_chopper(di, 0);
			if (wled_uvp_count == WLED_IRQ_ONCE_COUNT) {
				(void)hi6522_config_enable_ifb1_ifb2(
					di, wled_uvp_count);
				scharger_err(
					"SCHARGER ERROR:CHG_IRQ1_WLED_UVP,wled_uvp_count:%d\n",
					wled_uvp_count);
			} else if (wled_uvp_count == WLED_IRQ_SECOND_COUNT) {
				(void)hi6522_config_enable_ifb1_ifb2(
					di, wled_uvp_count);
				scharger_err(
					"SCHARGER ERROR:CHG_IRQ1_WLED_UVP,wled_uvp_count:%d\n",
					wled_uvp_count);
			} else if (wled_uvp_count == WLED_IRQ_THIRD_COUNT) {
				(void)hi6522_config_enable_chopper(di, 1);
				(void)hi6522_config_enable_ifb1_ifb2(
					di, wled_uvp_count);
				wled_uvp_count = 0;
				di->wled_en = CHG_POWER_DIS;
				(void)hi6522_config_wled_enable(di);
				scharger_err(
					"SCHARGER ERROR:CHG_IRQ1_WLED_UVP,wled_uvp_count:%d\n",
					wled_uvp_count);
			}
		} else {
			di->wled_en = CHG_POWER_DIS;
			(void)hi6522_config_wled_enable(di);
		}
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ1_WLED_UVP,reg[0x08]:0x%x\n",
			irq_stat_1);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}
	chk_irq_status_1_scp(irq_stat_1, di);
}

static void chk_irq_status_2(unsigned char irq_stat_2, struct hi6522_device_info *di)
{
	static u8 flash_bst_scp_cnt;

	if (irq_stat_2 & CHG_IRQ2_FLASH_BST_UVP) {
		di->flash_bst_en = CHG_POWER_DIS;
		(void)hi6522_config_flash_boost_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ2_FLASH_BST_UVP,reg[0x09]:0x%x\n",
			irq_stat_2);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_2 & CHG_IRQ2_FLASH_BST_OVP) {
		di->flash_bst_en = CHG_POWER_DIS;
		(void)hi6522_config_flash_boost_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ2_FLASH_BST_OVP,reg[0x09]:0x%x\n",
			irq_stat_2);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_2 & CHG_IRQ2_FLASH_BST_SCP) {
		/* To avoid Scharger v210 chip problem */
		flash_bst_scp_cnt++;
		if (flash_bst_scp_cnt > 3) { /* 3: the count of scp trial */
			di->flash_bst_en = CHG_POWER_DIS;
			(void)hi6522_config_flash_boost_enable(di);
			scharger_err(
				"SCHARGER ERROR:CHG_IRQ2_FLASH_BST_SCP,reg[0x09]:0x%x\n",
				irq_stat_2);
			atomic_notifier_call_chain(&fault_notifier_list,
				CHARGE_FAULT_SCHARGER, NULL);
			flash_bst_scp_cnt = 0;
		}
	}

	if (irq_stat_2 & CHG_IRQ2_FLASH_LED_SHORT) {
		di->flash_led_flash_en = CHG_POWER_DIS;
		di->flash_led_torch_en = CHG_POWER_DIS;
		(void)hi6522_config_flash_led_flash_enable(di);
		(void)hi6522_config_flash_led_torch_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ2_FLASH_LED_SHORT,reg[0x09]:0x%x\n",
			irq_stat_2);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_2 & CHG_IRQ2_FLASH_LED_OPEN) {
		di->flash_led_flash_en = CHG_POWER_DIS;
		di->flash_led_torch_en = CHG_POWER_DIS;
		(void)hi6522_config_flash_led_flash_enable(di);
		(void)hi6522_config_flash_led_torch_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ2_FLASH_LED_OPEN,reg[0x09]:0x%x\n",
			irq_stat_2);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}
}

static void chk_irq_status_3(unsigned char irq_stat_3, struct hi6522_device_info *di)
{
	static u8 reg_read;

	if (irq_stat_3 & CHG_IRQ3_LCD_NCP_SCP) {
		di->lcd_bst_en = CHG_POWER_DIS;
		di->lcd_ncp_en = CHG_POWER_DIS;
		di->lcd_ldo_en = CHG_POWER_DIS;
		(void)hi6522_config_lcd_ncp_enable(di);
		(void)hi6522_config_lcd_ldo_enable(di);
		(void)hi6522_config_lcd_boost_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LCD_NCP_SCP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_3 & CHG_IRQ3_LCD_BST_UVP) {
		di->lcd_bst_en = CHG_POWER_DIS;
		di->lcd_ncp_en = CHG_POWER_DIS;
		di->lcd_ldo_en = CHG_POWER_DIS;
		(void)hi6522_config_lcd_ncp_enable(di);
		(void)hi6522_config_lcd_ldo_enable(di);
		(void)hi6522_config_lcd_boost_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LCD_BST_UVP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_3 & CHG_IRQ3_LCD_BST_OVP) {
		di->lcd_bst_en = CHG_POWER_DIS;
		di->lcd_ncp_en = CHG_POWER_DIS;
		di->lcd_ldo_en = CHG_POWER_DIS;
		(void)hi6522_config_lcd_ncp_enable(di);
		(void)hi6522_config_lcd_ldo_enable(di);
		(void)hi6522_config_lcd_boost_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LCD_BST_OVP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_3 & CHG_IRQ3_LCD_BST_SCP) {
		(void)hi6522_read_byte(SOC_SCHARGER_LCD_BOOST_REG10_ADDR(0),
				       &reg_read);
		if (reg_read == 0x55) { /* set according to nManager */
			hi6522_write_byte(SOC_SCHARGER_LCD_BOOST_REG10_ADDR(0),
					  0x54);
			di->lcd_bst_en = CHG_POWER_DIS;
			di->lcd_ncp_en = CHG_POWER_DIS;
			di->lcd_ldo_en = CHG_POWER_DIS;
			(void)hi6522_config_lcd_ncp_enable(di);
			(void)hi6522_config_lcd_ldo_enable(di);
			(void)hi6522_config_lcd_boost_enable(di);
			(void)hi6522_write_byte(SOC_SCHARGER_LCD_BOOST_REG10_ADDR(0),
						0x55);
		}
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LCD_BST_SCP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_3 & CHG_IRQ3_LCD_LDO_OCP) {
		di->lcd_bst_en = CHG_POWER_DIS;
		di->lcd_ncp_en = CHG_POWER_DIS;
		di->lcd_ldo_en = CHG_POWER_DIS;
		(void)hi6522_config_lcd_ncp_enable(di);
		(void)hi6522_config_lcd_ldo_enable(di);
		(void)hi6522_config_lcd_boost_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LCD_LDO_OCP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_3 & CHG_IRQ3_LDO1_200MA_OCP) {
		di->ldo1_200ma_en = CHG_POWER_DIS;
		(void)hi6522_config_ldo1_200ma_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LDO1_200MA_OCP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}

	if (irq_stat_3 & CHG_IRQ3_LDO2_200MA_OCP) {
		di->ldo2_200ma_en = CHG_POWER_DIS;
		(void)hi6522_config_ldo2_200ma_enable(di);
		scharger_err(
			"SCHARGER ERROR:CHG_IRQ3_LDO2_200MA_OCP,reg[0x0A]:0x%x\n",
			irq_stat_3);
		atomic_notifier_call_chain(&fault_notifier_list,
					   CHARGE_FAULT_SCHARGER, NULL);
	}
}

static void chk_irq_status_4(unsigned char irq_stat_4)
{
	static u8 otg_scp_cnt;
	static u8 reg_read;
	static u8 otg_uvp_cnt;

	if (irq_stat_4 & CHG_IRQ4_OTG_SCP) {
		otg_scp_cnt++;
		(void)hi6522_write_byte(CHG_IRQ_REG4,
					irq_stat_4 & CHG_IRQ4_OTG_SCP);
		msleep(600); /* 600: sleep 600mS for next read */
		(void)hi6522_read_byte(CHG_STATUS0, &reg_read);
		if ((reg_read & CHG_STATUS0_OTG_ON) == CHG_STATUS0_OTG_ON)
			otg_scp_cnt = 0;

		if (otg_scp_cnt > 3) { /* 3: threshold value for otg trial */
			otg_scp_cnt = 0;
			(void)hi6522_set_otg_enable(CHG_POWER_DIS);
			scharger_err(
				"SCHARGER ERROR:CHG_IRQ4_OTG_SCP,reg[0x0B]:0x%x\n",
				irq_stat_4);
			atomic_notifier_call_chain(&fault_notifier_list,
						   CHARGE_FAULT_SCHARGER, NULL);
		}
	}

	if (irq_stat_4 & CHG_IRQ4_OTG_UVP) {
		otg_uvp_cnt++;
		(void)hi6522_write_byte(CHG_IRQ_REG4,
					irq_stat_4 & CHG_IRQ4_OTG_UVP);
		msleep(600); /* 600: need 600mS for next read */
		(void)hi6522_read_byte(CHG_STATUS0, &reg_read);
		if ((reg_read & CHG_STATUS0_OTG_ON) == CHG_STATUS0_OTG_ON)
			otg_uvp_cnt = 0;

		if (otg_uvp_cnt > 3) { /* 3: the count of scp trial */
			otg_uvp_cnt = 0;
			(void)hi6522_set_otg_enable(CHG_POWER_DIS);
			scharger_err(
				"SCHARGER ERROR:CHG_IRQ4_OTG_UVP,reg[0x0B]:0x%x\n",
				irq_stat_4);
			atomic_notifier_call_chain(&fault_notifier_list,
						   CHARGE_FAULT_SCHARGER,
						   NULL);
		}
	}
}

void hi6522_irq_work_handle(struct work_struct *work)
{
	struct hi6522_device_info *di =
		container_of(work, struct hi6522_device_info, irq_work);
	if (work == NULL || di == NULL) {
		scharger_err("input ptr work is NULL\n");
		return;
	}
	unsigned char irq_status_0 = 0;
	unsigned char irq_status_1 = 0;
	unsigned char irq_status_2 = 0;
	unsigned char irq_status_3 = 0;
	unsigned char irq_status_4 = 0;
	/* save irq status */
	(void)hi6522_read_byte(CHG_IRQ_REG0, &irq_status_0);
	(void)hi6522_read_byte(CHG_IRQ_REG1, &irq_status_1);
	(void)hi6522_read_byte(CHG_IRQ_REG2, &irq_status_2);
	(void)hi6522_read_byte(CHG_IRQ_REG3, &irq_status_3);
	(void)hi6522_read_byte(CHG_IRQ_REG4, &irq_status_4);

	/* handle irq */
	/* charger mntn add start */
	chk_irq_status_0(irq_status_0);
	chk_irq_status_1(irq_status_1, di);
	chk_irq_status_2(irq_status_2, di);
	chk_irq_status_3(irq_status_3, di);
	chk_irq_status_4(irq_status_4);

	/* clean irq */
	(void)hi6522_write_byte(CHG_IRQ_REG0, irq_status_0);
	(void)hi6522_write_byte(CHG_IRQ_REG1, irq_status_1);
	(void)hi6522_write_byte(CHG_IRQ_REG2, irq_status_2);
	(void)hi6522_write_byte(CHG_IRQ_REG3, irq_status_3);
	(void)hi6522_write_byte(CHG_IRQ_REG4, irq_status_4);

	(void)enable_irq(di->irq_int);
#ifdef CONFIG_HUAWEI_OCP
	/* handle lcd wled */
	scharger_wled_ocp(irq_status_1);
	/* handle lcd vsp/vsn */
	scharger_lcd_ocp(irq_status_3);
#endif
}
