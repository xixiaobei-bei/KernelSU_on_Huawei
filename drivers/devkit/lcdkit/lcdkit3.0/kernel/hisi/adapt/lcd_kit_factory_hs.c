/*
 * lcd_kit_factory_hs.c
 *
 * lcdkit hisi factory function for lcd driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include "lcd_kit_factory.h"
#include "lcd_kit_power.h"
#include "lcd_kit_disp.h"
#include "lcd_kit_parse.h"
#include "hisi_fb.h"
#include "lcd_kit_common.h"
#include "lcd_kit_ext_disp.h"

#if defined(CONFIG_HISI_HKADC)
#include <linux/hisi/hisi_adc.h>
#endif

static void checksum_set_mipi_clk(struct hisi_fb_data_type *hisifd,
	u32 mipi_clk)
{
	hisifd->panel_info.mipi.dsi_bit_clk_upt = mipi_clk;
}

static void checksum_set_vdd(u32 vdd)
{
	power_hdl->lcd_vdd.buf[POWER_VOL] = vdd;
}

static void checksum_stress_enable(struct hisi_fb_data_type *hisifd)
{
	int ret;
	u32 mipi_clk;
	u32 lcd_vdd;

	if (!FACT_INFO->checksum.stress_test_support) {
		LCD_KIT_ERR("not support checksum stress test\n");
		return;
	}
	if (FACT_INFO->checksum.mipi_clk) {
		mipi_clk = hisifd->panel_info.mipi.dsi_bit_clk_upt;
		FACT_INFO->checksum.rec_mipi_clk = mipi_clk;
		checksum_set_mipi_clk(hisifd, FACT_INFO->checksum.mipi_clk);
	}
	if (FACT_INFO->checksum.vdd) {
		if (power_hdl->lcd_vdd.buf == NULL) {
			LCD_KIT_ERR("lcd vdd buf is null\n");
			return;
		}
		lcd_vdd = power_hdl->lcd_vdd.buf[POWER_VOL];
		FACT_INFO->checksum.rec_vdd = lcd_vdd;
		checksum_set_vdd(FACT_INFO->checksum.vdd);
		ret = lcd_power_set_vol(LCD_KIT_VDD);
		if (ret)
			LCD_KIT_ERR("set voltage error\n");
	}
	lcd_kit_recovery_display(hisifd);
}

static void checksum_stress_disable(struct hisi_fb_data_type *hisifd)
{
	int ret;

	if (!FACT_INFO->checksum.stress_test_support) {
		LCD_KIT_ERR("not support checksum stress test\n");
		return;
	}
	if (FACT_INFO->checksum.mipi_clk)
		checksum_set_mipi_clk(hisifd,
			FACT_INFO->checksum.rec_mipi_clk);
	if (FACT_INFO->checksum.vdd) {
		if (power_hdl->lcd_vdd.buf == NULL) {
			LCD_KIT_ERR("lcd vdd buf is null\n");
			return;
		}
		checksum_set_vdd(FACT_INFO->checksum.rec_vdd);
		ret = lcd_power_set_vol(LCD_KIT_VDD);
		if (ret)
			LCD_KIT_ERR("set voltage error\n");
	}
	lcd_kit_recovery_display(hisifd);
}

static void lcd_enable_checksum(struct hisi_fb_data_type *hisifd)
{
	/* disable esd */
	lcd_esd_enable(hisifd, 0);
	FACT_INFO->checksum.status = LCD_KIT_CHECKSUM_START;
	if (!FACT_INFO->checksum.special_support) {
		if (FACT_INFO->checksum.clear_sram) {
			LCD_KIT_INFO("before checksum, clear sram\n");
			lcd_kit_dsi_cmds_tx(hisifd, &FACT_INFO->checksum.clear_sram_cmds);
		} else {
			lcd_kit_dsi_cmds_tx(hisifd, &FACT_INFO->checksum.enable_cmds);
		}
	}
	FACT_INFO->checksum.pic_index = INVALID_INDEX;
	LCD_KIT_INFO("Enable checksum\n");
}

static int lcd_kit_checksum_set(struct hisi_fb_data_type *hisifd,
	int pic_index)
{
	int ret = LCD_KIT_OK;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END) {
		if (pic_index == TEST_PIC_0) {
			LCD_KIT_INFO("start gram checksum\n");
			lcd_enable_checksum(hisifd);
			return ret;
		}
		LCD_KIT_INFO("pic index error\n");
		return LCD_KIT_FAIL;
	}
	if (pic_index == TEST_PIC_2)
		FACT_INFO->checksum.check_count++;
	if (FACT_INFO->checksum.check_count == CHECKSUM_CHECKCOUNT) {
		LCD_KIT_INFO("check 5 times, set checksum end\n");
		FACT_INFO->checksum.status = LCD_KIT_CHECKSUM_END;
		FACT_INFO->checksum.check_count = 0;
	}
	switch (pic_index) {
	case TEST_PIC_0:
	case TEST_PIC_1:
	case TEST_PIC_2:
		if (FACT_INFO->checksum.clear_sram)
			lcd_kit_dsi_cmds_tx(hisifd, &FACT_INFO->checksum.enable_cmds);
		if (FACT_INFO->checksum.special_support)
			lcd_kit_dsi_cmds_tx(hisifd,
				&FACT_INFO->checksum.enable_cmds);
		LCD_KIT_INFO("set pic [%d]\n", pic_index);
		FACT_INFO->checksum.pic_index = pic_index;
		break;
	default:
		LCD_KIT_INFO("set pic [%d] unknown\n", pic_index);
		FACT_INFO->checksum.pic_index = INVALID_INDEX;
		break;
	}
	return ret;
}

static int lcd_checksum_compare(uint8_t *read_value, uint32_t *value,
	int len)
{
	int i;
	int err_no = 0;
	uint8_t tmp;

	for (i = 0; i < len; i++) {
		tmp = (uint8_t)value[i];
		if (read_value[i] != tmp) {
			LCD_KIT_ERR("gram check error\n");
			LCD_KIT_ERR("read_value[%d]:0x%x\n", i, read_value[i]);
			LCD_KIT_ERR("expect_value[%d]:0x%x\n", i, tmp);
			err_no++;
		}
	}
	return err_no;
}

static int lcd_check_checksum(struct hisi_fb_data_type *hisifd)
{
	int ret;
	int err_cnt;
	int check_cnt;
	uint8_t read_value[LCD_KIT_CHECKSUM_SIZE + 1] = {0};
	uint32_t *checksum = NULL;
	uint32_t pic_index;

	if (FACT_INFO->checksum.value.arry_data == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	pic_index = FACT_INFO->checksum.pic_index;
	ret = lcd_kit_dsi_cmds_rx(hisifd, read_value, LCD_KIT_CHECKSUM_SIZE,
		&FACT_INFO->checksum.checksum_cmds);
	if (ret) {
		LCD_KIT_ERR("checksum read dsi0 error\n");
		return LCD_KIT_FAIL;
	}
	check_cnt = FACT_INFO->checksum.value.arry_data[pic_index].cnt;
	if (check_cnt > LCD_KIT_CHECKSUM_SIZE) {
		LCD_KIT_ERR("checksum count is larger than checksum size\n");
		return LCD_KIT_FAIL;
	}
	checksum = FACT_INFO->checksum.value.arry_data[pic_index].buf;
	err_cnt = lcd_checksum_compare(read_value, checksum, check_cnt);
	if (err_cnt) {
		LCD_KIT_ERR("err_cnt:%d\n", err_cnt);
		ret = LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_check_dsi1_checksum(struct hisi_fb_data_type *hisifd)
{
	int ret;
	int err_cnt;
	int check_cnt;
	uint8_t read_value[LCD_KIT_CHECKSUM_SIZE + 1] = {0};
	uint32_t *checksum = NULL;
	uint32_t pic_index;

	if (FACT_INFO->checksum.dsi1_value.arry_data == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	pic_index = FACT_INFO->checksum.pic_index;
	ret = lcd_kit_dsi1_cmds_rx(hisifd, read_value, LCD_KIT_CHECKSUM_SIZE,
		&FACT_INFO->checksum.checksum_cmds);
	if (ret) {
		LCD_KIT_ERR("checksum read dsi1 error\n");
		return LCD_KIT_FAIL;
	}
	check_cnt = FACT_INFO->checksum.dsi1_value.arry_data[pic_index].cnt;
	if (check_cnt > LCD_KIT_CHECKSUM_SIZE) {
		LCD_KIT_ERR("checksum count is larger than checksum size\n");
		return LCD_KIT_FAIL;
	}
	checksum = FACT_INFO->checksum.dsi1_value.arry_data[pic_index].buf;
	err_cnt = lcd_checksum_compare(read_value, checksum, check_cnt);
	if (err_cnt) {
		LCD_KIT_ERR("err_cnt:%d\n", err_cnt);
		ret = LCD_KIT_FAIL;
	}
	return ret;
}

static int lcd_kit_current_det(struct hisi_fb_data_type *hisifd)
{
	ssize_t current_check_result;
	uint8_t rd = 0;

	if (!FACT_INFO->current_det.support) {
		LCD_KIT_INFO("current detect is not support! return!\n");
		return LCD_KIT_OK;
	}
	lcd_kit_dsi_cmds_rx(hisifd, &rd, 1, &FACT_INFO->current_det.detect_cmds);
	LCD_KIT_INFO("current detect, read value = 0x%x\n", rd);
	/* buf[0] means current detect result */
	if ((rd & FACT_INFO->current_det.value.buf[0]) == LCD_KIT_OK) {
		current_check_result = LCD_KIT_OK;
		LCD_KIT_ERR("no current over\n");
	} else {
		current_check_result = LCD_KIT_FAIL;
		LCD_KIT_ERR("current over:0x%x\n", rd);
	}
	return current_check_result;
}

static int lcd_kit_lv_det(struct hisi_fb_data_type *hisifd)
{
	ssize_t lv_check_result;
	uint8_t rd = 0;

	if (!FACT_INFO->lv_det.support) {
		LCD_KIT_INFO("current detect is not support! return!\n");
		return LCD_KIT_OK;
	}
	lcd_kit_dsi_cmds_rx(hisifd, &rd, 1, &FACT_INFO->lv_det.detect_cmds);
	LCD_KIT_INFO("current detect, read value = 0x%x\n", rd);
	if ((rd & FACT_INFO->lv_det.value.buf[0]) == 0) {
		lv_check_result = LCD_KIT_OK;
		LCD_KIT_ERR("no current over\n");
	} else {
		lv_check_result = LCD_KIT_FAIL;
		LCD_KIT_ERR("current over:0x%x\n", rd);
	}
	return lv_check_result;
}

static int lcd_hor_line_test(struct hisi_fb_data_type *hisifd)
{
	int ret = LCD_KIT_OK;
	struct hisi_panel_info *pinfo = NULL;
	int count = HOR_LINE_TEST_TIMES;

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		LCD_KIT_ERR("pinfo is NULL!\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("horizontal line test start\n");
	LCD_KIT_INFO("FACT_INFO->hor_line.duration = %d\n",
			FACT_INFO->hor_line.duration);
	/* disable esd check */
	lcd_esd_enable(hisifd, 0);
	/* disable elvdd detect */
	disp_info->elvdd_detect.support = 0;
	while (count--) {
		/* hardware reset */
		if (!FACT_INFO->hor_line.hl_no_reset)
			lcd_hardware_reset();
		mdelay(30);
		/* test avdd */
		down(&hisifd->blank_sem);
		if (!hisifd->panel_power_on) {
			LCD_KIT_ERR("panel is power off\n");
			up(&hisifd->blank_sem);
			return LCD_KIT_FAIL;
		}
		hisifb_activate_vsync(hisifd);
		if (FACT_INFO->hor_line.hl_cmds.cmds != NULL) {
			ret = lcd_kit_dsi_cmds_tx(hisifd,
				&FACT_INFO->hor_line.hl_cmds);
			if (ret)
				LCD_KIT_ERR("send avdd cmd error\n");
		}
		hisifb_deactivate_vsync(hisifd);
		up(&hisifd->blank_sem);
		msleep(FACT_INFO->hor_line.duration * MILLISEC_TIME);
		/* recovery display */
		lcd_kit_recovery_display(hisifd);
	}
	/* enable esd */
	lcd_esd_enable(hisifd, 1);
	LCD_KIT_INFO("horizontal line test end\n");
	return ret;
}

static ssize_t lcd_inversion_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return lcd_kit_inversion_get_mode(buf);
}

static ssize_t lcd_inversion_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd inversion mode store buf NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	val = simple_strtoul(buf, NULL, 0);
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		hisifb_activate_vsync(hisifd);
		ret = lcd_kit_inversion_set_mode(hisifd, val);
		if (ret)
			LCD_KIT_ERR("inversion mode set failed\n");
		hisifb_deactivate_vsync(hisifd);
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_scan_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return lcd_kit_scan_get_mode(buf);
}

static ssize_t lcd_scan_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd scan mode store buf NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	val = simple_strtoul(buf, NULL, 0);
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		hisifb_activate_vsync(hisifd);
		ret = lcd_kit_scan_set_mode(hisifd, val);
		if (ret)
			LCD_KIT_ERR("scan mode set failed\n");
		hisifb_deactivate_vsync(hisifd);
	}
	up(&hisifd->blank_sem);
	return count;
}

static ssize_t lcd_check_reg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	pinfo = &(hisifd->panel_info);
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (pinfo->fps_updt != pinfo->fps && pinfo->fps_updt != 0) {
			ret = snprintf(buf, PAGE_SIZE, "OK\n");
			LCD_KIT_INFO("check_reg result:%s\n", buf);
			up(&hisifd->blank_sem);
			return ret;
		}
		hisifb_activate_vsync(hisifd);
		ret = lcd_kit_check_reg(hisifd, buf);
		hisifb_deactivate_vsync(hisifd);
	}
	up(&hisifd->blank_sem);
	return ret;
}

static int lcd_kit_checksum_check(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint32_t pic_index;

	pic_index = FACT_INFO->checksum.pic_index;
	LCD_KIT_INFO("checksum pic num:%d\n", pic_index);
	if (pic_index > TEST_PIC_2) {
		LCD_KIT_ERR("checksum pic num unknown:%d\n", pic_index);
		return LCD_KIT_FAIL;
	}
	ret = lcd_check_checksum(hisifd);
	if (ret)
		LCD_KIT_ERR("checksum error\n");
	if (is_dual_mipi_panel(hisifd)) {
		ret = lcd_check_dsi1_checksum(hisifd);
		if (ret)
			LCD_KIT_ERR("dsi1 checksum error\n");
	}
	if (ret && FACT_INFO->checksum.pic_index == TEST_PIC_2)
		FACT_INFO->checksum.status = LCD_KIT_CHECKSUM_END;

	if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END) {
		lcd_kit_dsi_cmds_tx(hisifd, &FACT_INFO->checksum.disable_cmds);
		LCD_KIT_INFO("gram checksum end, disable checksum\n");
		/* enable esd */
		lcd_esd_enable(hisifd, 1);
	}
	return ret;
}

static ssize_t lcd_gram_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int checksum_result;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->checksum.support) {
		down(&hisifd->blank_sem);
		if (hisifd->panel_power_on) {
			hisifb_activate_vsync(hisifd);
			checksum_result = lcd_kit_checksum_check(hisifd);
			hisifb_deactivate_vsync(hisifd);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", checksum_result);
		}
		up(&hisifd->blank_sem);
		/* disable checksum stress test, restore mipi clk and vdd */
		if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END) {
			checksum_stress_disable(hisifd);
			if (FACT_INFO->checksum.clear_sram) {
				lcd_kit_recovery_display(hisifd);
				LCD_KIT_INFO("checksum recovery display\n");
			}
		}
	}
	return ret;
}

static ssize_t lcd_gram_check_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;
	unsigned long val = 0;
	int index;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtoul(buf, 10, &val);
	if (ret)
		return ret;
	LCD_KIT_INFO("val=%ld\n", val);
	if (FACT_INFO->checksum.support) {
		/* enable checksum stress test, promote mipi clk and vdd */
		if (FACT_INFO->checksum.status == LCD_KIT_CHECKSUM_END)
			checksum_stress_enable(hisifd);
		down(&hisifd->blank_sem);
		if (hisifd->panel_power_on) {
			hisifb_activate_vsync(hisifd);
			index = val - INDEX_START;
			ret = lcd_kit_checksum_set(hisifd, index);
			hisifb_deactivate_vsync(hisifd);
		}
		up(&hisifd->blank_sem);
	}
	return count;
}

static ssize_t lcd_sleep_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return lcd_kit_get_sleep_mode(buf);
}

static ssize_t lcd_sleep_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long val = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = strict_strtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter!\n");
		return ret;
	}
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off!\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_set_sleep_mode(val);
	if (ret)
		LCD_KIT_ERR("set sleep mode fail!\n");
	return count;
}

static ssize_t lcd_poweric_det_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	ssize_t ret;
	int i;
	char str[LCD_PMIC_LENGTH_MAX] = {0};
	char tmp[LCD_PMIC_LENGTH_MAX] = {0};
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd)
		return LCD_KIT_FAIL;

	if (!hisifd->panel_info.poweric_num_length ||
		!FACT_INFO->poweric_detect.support) {
		ret += snprintf(tmp, sizeof(tmp), "poweric%d:%d", 1, 1);
		strncat(str, tmp, strlen(tmp));
		LCD_KIT_INFO("poweric_detect not support, default pass\n");
	} else {
		LCD_KIT_INFO("poweric num = %d\n", hisifd->panel_info.poweric_num_length);
		for (i = 0; i < hisifd->panel_info.poweric_num_length; i++) {
			LCD_KIT_INFO("i = %d, poweric_status = %d\n",
				i, hisifd->panel_info.poweric_status[i]);
			ret += snprintf(tmp, sizeof(tmp), "poweric%d:%d",
				(i + 1), hisifd->panel_info.poweric_status[i]);
			strncat(str, tmp, strlen(tmp));
			if (i != hisifd->panel_info.poweric_num_length - 1)
				strncat(str, "\r\n", strlen("\r\n"));
		}
	}
	LCD_KIT_INFO("%s\n", str);
	ret = snprintf(buf, PAGE_SIZE, "%s\n", str);
	return ret;
}

void poweric_gpio_ctl(int num, int pull_type)
{
	int gpio_temp;

	gpio_temp = FACT_INFO->poweric_detect.gpio_list.buf[num];
	if (pull_type) {
		if (FACT_INFO->poweric_detect.gpio_val.buf[num])
			lcd_poweric_gpio_operator(gpio_temp, GPIO_HIGH);
		else
			lcd_poweric_gpio_operator(gpio_temp, GPIO_LOW);
	} else {
		if (FACT_INFO->poweric_detect.gpio_val.buf[num])
			lcd_poweric_gpio_operator(gpio_temp, GPIO_LOW);
		else
			lcd_poweric_gpio_operator(gpio_temp, GPIO_HIGH);
	}
}

int poweric_get_elvdd(struct hisi_fb_data_type *hisifd, int i)
{
	int ret;
	int32_t gpio_value;

	lcd_poweric_gpio_operator(FACT_INFO->poweric_detect.detect_gpio,
		GPIO_REQ);
	ret = gpio_direction_input(FACT_INFO->poweric_detect.detect_gpio);
	if (ret) {
		gpio_free(FACT_INFO->poweric_detect.detect_gpio);
		LCD_KIT_ERR("poweric_gpio[%d] direction set fail\n",
			FACT_INFO->poweric_detect.detect_gpio);
		return LCD_KIT_FAIL;
	}
	gpio_value = gpio_get_value(FACT_INFO->poweric_detect.detect_gpio);
	LCD_KIT_INFO("poweric detect elvdd value is %d\n", gpio_value);

	if (gpio_value != FACT_INFO->poweric_detect.detect_val)
		hisifd->panel_info.poweric_status[i] = POWERIC_ERR;
	else
		hisifd->panel_info.poweric_status[i] = POWERIC_NOR;
	return LCD_KIT_OK;
}

int poweric_set_elvdd(struct hisi_fb_data_type *hisifd,
	int start, int end, int i)
{
	int j, k, ret, temp;
	int gpio_flag = 0;
	for (j = start; j < end; j++) {
		lcd_poweric_gpio_operator(FACT_INFO->poweric_detect.gpio_list.buf[j],
			GPIO_REQ);
		ret = gpio_direction_output(FACT_INFO->poweric_detect.gpio_list.buf[j],
			OUTPUT_TYPE);
		if (ret) {
			gpio_flag = POWERIC_NOR;
			for (k = start; k <= j; k++) {
				temp = FACT_INFO->poweric_detect.gpio_list.buf[k];
				lcd_poweric_gpio_operator(temp, GPIO_FREE);
			}
			LCD_KIT_ERR("poweric_gpio[%d] direction set fail\n",
				FACT_INFO->poweric_detect.gpio_list.buf[j]);
			hisifd->panel_info.poweric_status[i] = POWERIC_ERR;
			break;
		}
		LCD_KIT_INFO("disp_info->poweric_detect.gpio_list.buf[j] = %d\n",
			FACT_INFO->poweric_detect.gpio_list.buf[j]);
		/* enable gpio */
		poweric_gpio_ctl(j, PULL_TYPE_NOR);
		/* enable delay between gpio */
		msleep(10);
	}
	return gpio_flag;
}

void poweric_reverse_gpio(int start, int end)
{
	int j;
	for (j = start; j < end; j++) {
		poweric_gpio_ctl(j, PULL_TYPE_REV);
		/* disable delay between gpio */
		msleep(10);
	}
}

void poweric_free_gpio(int start, int end)
{
	int j;
	for (j = start; j < end; j++)
		lcd_poweric_gpio_operator(
			FACT_INFO->poweric_detect.gpio_list.buf[j],
			GPIO_FREE);
}

void poweric_gpio_detect_dbc(struct hisi_fb_data_type *hisifd)
{
	int i, gpio_temp;
	int32_t gpio_value;
	int gpio_pos = 0;
	int gpio_flag, temp;

	temp = FACT_INFO->poweric_detect.poweric_num;
	hisifd->panel_info.poweric_num_length = temp;
	LCD_KIT_INFO("poweric_num = %d\n",
		FACT_INFO->poweric_detect.poweric_num);

	for (i = 0; i < FACT_INFO->poweric_detect.poweric_num; i++) {
		temp = FACT_INFO->poweric_detect.detect_gpio;
		gpio_temp = gpio_pos;
		gpio_pos += FACT_INFO->poweric_detect.gpio_num_list.buf[i];
		/* operate gpio to detect elvdd */
		gpio_flag = poweric_set_elvdd(hisifd, gpio_temp, gpio_pos, i);
		if (!gpio_flag) {
			LCD_KIT_INFO("detect_gpio = %d\n", temp);
			if (poweric_get_elvdd(hisifd, i))
				LCD_KIT_INFO("get ELVDD fail %d\n", temp);
			/* reverse operate gpio to recovery elvdd */
			poweric_reverse_gpio(gpio_temp, gpio_pos);
			/* delay before obtaining ELVDD */
			msleep(1000);
			gpio_value = gpio_get_value(
				FACT_INFO->poweric_detect.detect_gpio);
			lcd_poweric_gpio_operator(
				FACT_INFO->poweric_detect.detect_gpio,
				GPIO_FREE);
			/* free gpio */
			poweric_free_gpio(gpio_temp, gpio_pos);
			LCD_KIT_INFO("second detect elvdd value is %d\n", gpio_value);
		} else {
			gpio_flag = POWERIC_ERR;
		}
	}
}

static ssize_t lcd_poweric_det_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_OK;
	}
	pdata = dev_get_platdata(&hisifd->pdev->dev);
	if (!pdata) {
		LCD_KIT_ERR("lcd poweric det store pdata NULL Pointer\n");
		return LCD_KIT_OK;
	}
	if (!buf) {
		LCD_KIT_ERR("lcd poweric det store buf NULL Pointer\n");
		return LCD_KIT_OK;
	}
	hisifd->panel_info.poweric_num_length = 0;
	if (FACT_INFO->poweric_detect.support) {
		LCD_KIT_INFO("poweric detect is support\n");
		poweric_gpio_detect_dbc(hisifd);
	} else {
		LCD_KIT_ERR("poweric detect is not support\n");
	}
	return count;
}

static ssize_t lcd_amoled_gpio_pcd_errflag(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_gpio_pcd_errflag_check();
	LCD_KIT_INFO("pcd_errflag result_value = %d\n", ret);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", ret);
	return ret;
}

static ssize_t lcd_amoled_cmds_pcd_errflag(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	int check_result;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (hisifd->panel_power_on) {
		if (disp_info->pcd_errflag.pcd_support ||
			disp_info->pcd_errflag.errflag_support) {
			check_result = lcd_kit_check_pcd_errflag_check(hisifd);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", check_result);
			LCD_KIT_INFO("pcd_errflag, the check_result = %d\n",
				check_result);
		}
	}
	return ret;
}

int lcd_kit_pcd_detect(struct hisi_fb_data_type *hisifd, uint32_t val)
{
	int ret = LCD_KIT_OK;
	static uint32_t pcd_mode;

	if (!hisifd) {
		LCD_KIT_ERR("hisifd is NULL\n");
		return LCD_KIT_FAIL;
	}

	if (!FACT_INFO->pcd_errflag_check.pcd_errflag_check_support) {
		LCD_KIT_ERR("pcd errflag not support!\n");
		return LCD_KIT_OK;
	}

	if (pcd_mode == val) {
		LCD_KIT_ERR("pcd detect already\n");
		return LCD_KIT_OK;
	}

	pcd_mode = val;
	if (pcd_mode == LCD_KIT_PCD_DETECT_OPEN)
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->pcd_errflag_check.pcd_detect_open_cmds);
	else if (pcd_mode == LCD_KIT_PCD_DETECT_CLOSE)
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->pcd_errflag_check.pcd_detect_close_cmds);
	LCD_KIT_INFO("pcd_mode %d, ret %d\n", pcd_mode, ret);
	return ret;
}

static ssize_t lcd_amoled_pcd_errflag_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = PCD_ERRFLAG_SUCCESS;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (disp_info->pcd_errflag.pcd_support ||
		disp_info->pcd_errflag.errflag_support)
		ret = lcd_amoled_cmds_pcd_errflag(dev, attr, buf);
	else
		ret = lcd_amoled_gpio_pcd_errflag(dev, attr, buf);
	return ret;
}

static ssize_t lcd_amoled_pcd_errflag_store(struct device *dev,
	struct device_attribute *attr, const char *buf)
{
	int ret;
	unsigned long val = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtoul(buf, 0, &val);
	if (ret) {
		LCD_KIT_ERR("invalid parameter\n");
		return ret;
	}
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	return lcd_kit_pcd_detect(hisifd, val);
}

static ssize_t lcd_test_config_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_get_test_config(buf);
	if (ret < 0) {
		LCD_KIT_ERR("not find test item\n");
		return ret;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (buf) {
			if (!strncmp(buf, "OVER_CURRENT_DETECTION", strlen("OVER_CURRENT_DETECTION"))) {
				hisifb_activate_vsync(hisifd);
				ret = lcd_kit_current_det(hisifd);
				hisifb_deactivate_vsync(hisifd);
				up(&hisifd->blank_sem);
				return snprintf(buf, PAGE_SIZE, "%d", ret);
			}
			if (!strncmp(buf, "OVER_VOLTAGE_DETECTION", strlen("OVER_VOLTAGE_DETECTION"))) {
				hisifb_activate_vsync(hisifd);
				ret = lcd_kit_lv_det(hisifd);
				hisifb_deactivate_vsync(hisifd);
				up(&hisifd->blank_sem);
				return snprintf(buf, PAGE_SIZE, "%d", ret);
			}
		}
	}
	up(&hisifd->blank_sem);
	return ret;
}

static ssize_t lcd_test_config_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	if (lcd_kit_set_test_config(buf) < 0)
		LCD_KIT_ERR("set_test_config failed\n");
	return count;
}

static ssize_t lcd_lv_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_lv_det(hisifd);
	return snprintf(buf, PAGE_SIZE, "%d", ret);
}

static ssize_t lcd_current_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	ret = lcd_kit_current_det(hisifd);
	return snprintf(buf, PAGE_SIZE, "%d", ret);
}

static void ldo_transform(struct lcd_ldo *ldo)
{
	int i;

	ldo->lcd_ldo_num = FACT_INFO->ldo_check.ldo_num;
	memcpy(ldo->lcd_ldo_name, FACT_INFO->ldo_check.ldo_name,
		sizeof(FACT_INFO->ldo_check.ldo_name));
	memcpy(ldo->lcd_ldo_channel, FACT_INFO->ldo_check.ldo_channel,
		sizeof(FACT_INFO->ldo_check.ldo_channel));
	memcpy(ldo->lcd_ldo_current, FACT_INFO->ldo_check.ldo_current,
		sizeof(FACT_INFO->ldo_check.ldo_current));
	memcpy(ldo->lcd_ldo_threshold, FACT_INFO->ldo_check.curr_threshold,
		sizeof(FACT_INFO->ldo_check.curr_threshold));
	for (i = 0; i < ldo->lcd_ldo_num; i++) {
		LCD_KIT_INFO("ldo[%d]:\n", i);
		LCD_KIT_INFO("name=%s,current=%d,channel=%d,threshold=%d!\n",
			ldo->lcd_ldo_name[i],
			ldo->lcd_ldo_current[i],
			ldo->lcd_ldo_channel[i],
			ldo->lcd_ldo_threshold[i]);
	}
}

static ssize_t lcd_ldo_check_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	int j;
	int cur_val = -1; // init invalid value
	int sum_current;
	int len = sizeof(struct lcd_ldo);
	int temp_max_value;
	struct hisi_fb_data_type *hisifd = NULL;
	struct lcd_ldo ldo_result;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!FACT_INFO->ldo_check.support) {
		LCD_KIT_INFO("ldo check not support\n");
		return len;
	}
	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off\n");
		up(&hisifd->blank_sem);
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < FACT_INFO->ldo_check.ldo_num; i++) {
		sum_current = 0;
		temp_max_value = 0;
		for (j = 0; j < LDO_CHECK_COUNT; j++) {
#if defined(CONFIG_HISI_HKADC)
			cur_val = hisi_adc_get_current(
				FACT_INFO->ldo_check.ldo_channel[i]);
#endif
			if (cur_val < 0) {
				sum_current = -1;
				break;
			}
			sum_current = sum_current + cur_val;
			if (temp_max_value < cur_val)
				temp_max_value = cur_val;
		}
		if (sum_current == -1)
			FACT_INFO->ldo_check.ldo_current[i] = -1;
		else
			FACT_INFO->ldo_check.ldo_current[i] =
				(sum_current - temp_max_value) / (LDO_CHECK_COUNT - 1);
	}
	up(&hisifd->blank_sem);
	ldo_transform(&ldo_result);
	memcpy(buf, &ldo_result, len);
	LCD_KIT_INFO("ldo check finished\n");
	return len;
}

static ssize_t lcd_general_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->hor_line.support)
		ret = lcd_hor_line_test(hisifd);

	if (ret == 0)
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	else
		ret = snprintf(buf, PAGE_SIZE, "FAIL\n");

	return ret;
}

static void lcd_vtc_line_set_bias_voltage(struct hisi_fb_data_type *hisifd)
{
	struct lcd_kit_bias_ops *bias_ops = NULL;
	int ret;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return;
	}

	bias_ops = lcd_kit_get_bias_ops();
	if (bias_ops == NULL) {
		LCD_KIT_ERR("can not get bias_ops\n");
		return;
	}
	/* set bias voltage */
	if ((bias_ops->set_vtc_bias_voltage) &&
		(FACT_INFO->vtc_line.vtc_vsp > 0) &&
		(FACT_INFO->vtc_line.vtc_vsn > 0)) {
		ret = bias_ops->set_vtc_bias_voltage(FACT_INFO->vtc_line.vtc_vsp,
			FACT_INFO->vtc_line.vtc_vsn, true);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("set vtc bias voltage error\n");
		if (bias_ops->set_bias_is_need_disable() == true)
			lcd_kit_recovery_display(hisifd);
	}
}

static void lcd_vtc_line_resume_bias_voltage(struct hisi_fb_data_type *hisifd)
{
	struct lcd_kit_bias_ops *bias_ops = NULL;
	int ret;

	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return;
	}

	bias_ops = lcd_kit_get_bias_ops();
	if (bias_ops == NULL) {
		LCD_KIT_ERR("can not get bias_ops\n");
		return;
	}
	/* set bias voltage */
	if ((bias_ops->set_vtc_bias_voltage) &&
		(bias_ops->set_bias_is_need_disable() == true)) {
		/* buf[2]:set bias voltage value */
		ret = bias_ops->set_vtc_bias_voltage(power_hdl->lcd_vsp.buf[2],
			power_hdl->lcd_vsn.buf[2], false);
		if (ret != LCD_KIT_OK)
			LCD_KIT_ERR("set vtc bias voltage error\n");
	}
}

static int lcd_vtc_line_test(struct hisi_fb_data_type *hisifd,
	unsigned long pic_index)
{
	int ret = LCD_KIT_OK;

	switch (pic_index) {
	case PIC1_INDEX:
		/* disable esd */
		lcd_esd_enable(hisifd, 0);
		/* disable elvdd detect */
		disp_info->elvdd_detect.support = 0;
		/* lcd panel set bias */
		lcd_vtc_line_set_bias_voltage(hisifd);
		/* hardware reset */
		if (!FACT_INFO->vtc_line.vtc_no_reset)
			lcd_hardware_reset();
		mdelay(20);
		if (FACT_INFO->vtc_line.vtc_cmds.cmds != NULL) {
			ret = lcd_kit_dsi_cmds_tx(hisifd,
				&FACT_INFO->vtc_line.vtc_cmds);
			if (ret)
				LCD_KIT_ERR("send vtc cmd error\n");
		}
		break;
	case PIC2_INDEX:
	case PIC3_INDEX:
	case PIC4_INDEX:
		LCD_KIT_INFO("picture:%lu display\n", pic_index);
		break;
	case PIC5_INDEX:
		/* lcd panel resume bias */
		lcd_vtc_line_resume_bias_voltage(hisifd);
		lcd_kit_recovery_display(hisifd);
		/* enable esd */
		lcd_esd_enable(hisifd, 1);
		break;
	default:
		LCD_KIT_ERR("pic number not support\n");
		break;
	}
	return ret;
}

static ssize_t lcd_vertical_line_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	struct hisi_fb_data_type *hisifd = NULL;

	hisifd = dev_get_hisifd(dev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!FACT_INFO->vtc_line.support) {
		ret = snprintf(buf, PAGE_SIZE, "NOT_SUPPORT\n");
		return ret;
	}
	ret = snprintf(buf, PAGE_SIZE, "OK\n");
	return ret;
}

static ssize_t lcd_vertical_line_test_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	unsigned long index = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (dev == NULL) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	if (buf == NULL) {
		LCD_KIT_ERR("NULL Pointer!\n");
		return LCD_KIT_FAIL;
	}
	hisifd = dev_get_hisifd(dev);
	if (hisifd == NULL) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	ret = kstrtoul(buf, 10, &index);
	if (ret) {
		LCD_KIT_ERR("strict_strtoul fail!\n");
		return ret;
	}
	LCD_KIT_INFO("index=%ld\n", index);
	if (FACT_INFO->vtc_line.support) {
		ret = lcd_vtc_line_test(hisifd, index);
		if (ret)
			LCD_KIT_ERR("vtc line test fail\n");
	}
	return count;
}

static ssize_t lcd_bl_self_test_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;
	struct lcd_kit_bl_ops *bl_ops = NULL;

	bl_ops = lcd_kit_get_bl_ops();
	if (!bl_ops) {
		LCD_KIT_ERR("bl_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}

	if (FACT_INFO->bl_open_short_support) {
		if (bl_ops->bl_self_test)
			ret = bl_ops->bl_self_test();
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t lcd_hkadc_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (FACT_INFO->hkadc.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", FACT_INFO->hkadc.value);

	return ret;
}

static ssize_t lcd_hkadc_debug_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int channel = 0;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}
	if (FACT_INFO->hkadc.support) {
		ret = sscanf(buf, "%u", &channel);
		if (ret) {
			LCD_KIT_ERR("ivalid parameter!\n");
			return ret;
		}
#if defined(CONFIG_HISI_HKADC)
		FACT_INFO->hkadc.value = hisi_adc_get_value(channel);
#endif
	}
	return count;
}

#if defined(CONFIG_HUAWEI_DSM) && defined(CONFIG_HUAWEI_DATA_ACQUISITION)

#define AVDD_PRODUCT_CODE     703026001

static int lcd_data_report_err(int err_no, const struct message *msg)
{
	int ret = LCD_KIT_OK;

	if (!msg) {
		LCD_KIT_ERR("msg is NULL Pointer\n");
		return LCD_KIT_FAIL;
	}

	if (lcd_dclient && !dsm_client_ocuppy(lcd_dclient)) {
		ret = dsm_client_copy_ext(lcd_dclient, msg,
			sizeof(struct message));
		if (ret > 0)
			dsm_client_notify(lcd_dclient, err_no);
		else
			LCD_KIT_ERR("dsm_client_notify failed");
	}
	return ret;
}

static int data_message_report(const struct event *event)
{
	int ret;
	struct message *msg = NULL;

	msg = kzalloc(sizeof(struct message), GFP_KERNEL);
	if (!msg) {
		LCD_KIT_ERR("alloc message failed\n");
		return -1;
	}

	msg->version = 1;
	msg->num_events = 1;
	msg->data_source = DATA_FROM_KERNEL;
	memcpy(&(msg->events[0]), event, sizeof(struct event));
	ret = lcd_data_report_err(DA_LCD_AVDD_ERROR_NO, msg);
	kfree(msg);
	return ret;
}

static void lcd_avdd_detect_data_report(uint16_t value)
{
	int ret;
	u32 min;
	u32 max;
	struct event event;

	min = FACT_INFO->avdd_detect.low_threshold;
	max = FACT_INFO->avdd_detect.high_threshold;
	memset(&event, 0, sizeof(struct event));
	event.item_id = AVDD_PRODUCT_CODE;
	memcpy(event.station, "NA", strlen("NA") + 1);
	memcpy(event.bsn, "NA", strlen("NA") + 1);
	memcpy(event.firmware, "NA", strlen("NA") + 1);
	memcpy(event.description, "NA", strlen("NA") + 1);
	memcpy(event.device_name, "LCD_DDIC", strlen("LCD_DDIC") + 1);
	memcpy(event.test_name, "AVDD_DETECT", strlen("AVDD_DETECT") + 1);
	if (value < min || value > max)
		memcpy(event.result, "fail", strlen("fail") + 1);
	else
		memcpy(event.result, "pass", strlen("pass") + 1);
	ret = snprintf(event.value, MAX_VAL_LEN, "%u", value);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error\n");
		return;
	}
	ret = snprintf(event.min_threshold, MAX_VAL_LEN, "%u", min);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error\n");
		return;
	}
	ret = snprintf(event.max_threshold, MAX_VAL_LEN, "%u", max);
	if (ret < 0) {
		LCD_KIT_ERR("snprintf error\n");
		return;
	}
	ret = data_message_report(&event);
	if (ret > 0)
		LCD_KIT_INFO("avdd data report succ\n");
	else
		LCD_KIT_ERR("avdd data report failed\n");
}
#endif
static int process_vb_value(void)
{
	int i;
	int j;
	int temp;
	int average;
	int vb_channel;
	int vb_arr[ADC_READ_TIMES] = {0};

	vb_channel = FACT_INFO->avdd_detect.vb_channel;
#if defined(CONFIG_HISI_HKADC)
	for (i = 0; i < ADC_READ_TIMES; i++) {
		vb_arr[i] = hisi_adc_get_value(vb_channel);
		LCD_KIT_ERR("vb_arr[%d] = %d\n", i, vb_arr[i]);
		mdelay(1);
	}
#else
	LCD_KIT_ERR("hisi hkadc is not support\n");
	return LCD_KIT_FAIL;
#endif
	/* array sort */
	for (i = 1; i < ADC_READ_TIMES; i++) {
		temp = vb_arr[i];
		for (j = i; j > 0 && vb_arr[j - 1] > temp; j--)
			vb_arr[j] = vb_arr[j - 1];
		vb_arr[j] = temp;
	}
	temp = 0;
	/* remove the max and min values, then calculate the average value */
	for (i = 1; i < ADC_READ_TIMES - 1; i++)
		temp += vb_arr[i];
	average = temp / (ADC_READ_TIMES - 2);
	LCD_KIT_ERR("average = %d\n", average);
	return average;
}
static void lcd_avdd_gpio_free(
	struct lcd_kit_array_data gpio_grp, int gpio_cnt)
{
	int i;

	for (i = 0; i < gpio_cnt; i++) {
		poweric_gpio = gpio_grp.buf[i];
		(void)lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC,
			GPIO_FREE);
	}
}
static int lcd_avdd_gpio_config(
	struct lcd_kit_array_data gpio_grp,
	struct lcd_kit_array_data gpio_ctrl)
{
	int i;
	int ret = LCD_KIT_OK;

	for (i = 0; i < gpio_grp.cnt; i++) {
		poweric_gpio = gpio_grp.buf[i];
		ret = lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC,
			gpio_ctrl.buf[i]);
		if (ret < 0) {
			(void)lcd_avdd_gpio_free(gpio_grp, gpio_grp.cnt);
			return ret;
		}
	}
	return ret;
}
static int lcd_avdd_gpio_request(struct lcd_kit_array_data gpio_grp)
{
	int i;
	int ret = LCD_KIT_OK;

	for (i = 0; i < gpio_grp.cnt; i++) {
		poweric_gpio = gpio_grp.buf[i];
		ret = lcd_kit_gpio_tx(LCD_KIT_POWERIC_DET_DBC,
			GPIO_REQ);
		if (ret < 0) {
			(void)lcd_avdd_gpio_free(gpio_grp, i);
			return ret;
		}
	}
	return ret;
}
static int lcd_get_gpio_adc_val(
	struct lcd_kit_array_data gpio_grp,
	struct lcd_kit_array_data gpio_ctrl)
{
	int i;
	int j;
	int ret;
	int adc_value;

	if (!gpio_grp.buf || !gpio_ctrl.buf) {
		LCD_KIT_ERR("buff is null.\n");
		return 0;
	}

	if ((gpio_grp.cnt != gpio_ctrl.cnt) ||
		(gpio_grp.cnt == 0)) {
		LCD_KIT_ERR("gpio_cnt %d, a_cnt %d, b_cnt %d.\n",
			gpio_grp.cnt, gpio_ctrl.cnt, gpio_ctrl.cnt);
		return 0;
	}

	ret = lcd_avdd_gpio_request(gpio_grp);
	if (ret < 0) {
		LCD_KIT_ERR("gpio requst fail.\n");
		return ret;
	}

	ret = lcd_avdd_gpio_config(gpio_grp, gpio_ctrl);
	if (ret < 0) {
		LCD_KIT_ERR("gpio requst fail.\n");
		return ret;
	}

	adc_value = process_vb_value();

	(void)lcd_avdd_gpio_free(gpio_grp, gpio_grp.cnt);

	return adc_value;
}
static int lcd_get_avdd_gpio_grp_det_result(char *buf)
{
	int det_ret = LCD_KIT_OK;
	int gpio_det_result;
	int gpio_adc_val_a;
	int gpio_adc_val_b;
	u32 low_threshold;
	u32 high_threshold;

	gpio_adc_val_a = lcd_get_gpio_adc_val(
		FACT_INFO->avdd_detect.gpio_grp,
		FACT_INFO->avdd_detect.gpio_ctrl_a);
	gpio_adc_val_b = lcd_get_gpio_adc_val(
		FACT_INFO->avdd_detect.gpio_grp,
		FACT_INFO->avdd_detect.gpio_ctrl_b);
	if ((gpio_adc_val_a < 0) ||
		(gpio_adc_val_b < 0) ||
		(gpio_adc_val_a < gpio_adc_val_b)) {
		LCD_KIT_INFO("avdd det err, val_a = %d, val_b = %d.\n",
			gpio_adc_val_a,
			gpio_adc_val_b);
		return det_ret;
	}
	gpio_det_result = gpio_adc_val_a - gpio_adc_val_b;
	low_threshold = FACT_INFO->avdd_detect.low_threshold;
	high_threshold = FACT_INFO->avdd_detect.high_threshold;
	if (gpio_det_result < low_threshold ||
		gpio_det_result > high_threshold) {
		LCD_KIT_ERR("avdd det err, value = %d, threshold : %d ~ %d.\n",
			gpio_det_result,
			low_threshold,
			high_threshold);
		/* err code is current panel number */
		return 1 << LCD_ACTIVE_PANEL;
	}
	LCD_KIT_INFO("panel %d: val_a %d, val_b %d, det_ret %d\n",
		LCD_ACTIVE_PANEL, gpio_adc_val_a, gpio_adc_val_b, det_ret);
	return det_ret;
}
static ssize_t lcd_avdd_detect_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	int gpio_avdd_ret;
	ssize_t ret;
	int vb_average;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	if (!FACT_INFO->avdd_detect.support) {
		LCD_KIT_ERR("avdd detect is not support\n");
		ret = snprintf(buf, PAGE_SIZE, "%d\n", 0);
		LCD_KIT_ERR("ret is %d\n", ret);
		return LCD_KIT_OK;
	}

	if (FACT_INFO->avdd_detect.gpio_grp_ctrl) {
		gpio_avdd_ret = lcd_get_avdd_gpio_grp_det_result(buf);
		return snprintf(buf, PAGE_SIZE, "%d\n", gpio_avdd_ret);
	}

	vb_average = process_vb_value();
#if defined(CONFIG_HUAWEI_DSM) && defined(CONFIG_HUAWEI_DATA_ACQUISITION)
	lcd_avdd_detect_data_report(vb_average);
#endif
	if (vb_average < FACT_INFO->avdd_detect.low_threshold ||
		vb_average > FACT_INFO->avdd_detect.high_threshold) {
		LCD_KIT_ERR("avdd exceeds the range, value = %d\n", vb_average);
		/* 1 is returned upon failure */
		ret = snprintf(buf, PAGE_SIZE, "%d\n", 1);
		return ret;
	}
	LCD_KIT_INFO("vb_average = %d, avdd detect success\n", vb_average);
	/* 0 is returned upon success */
	ret = snprintf(buf, PAGE_SIZE, "%d\n", 0);
	return ret;
}

static ssize_t lcd_oneside_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (FACT_INFO->oneside_mode.support ||
		FACT_INFO->color_aod_det.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n",
			FACT_INFO->oneside_mode.mode);

	return ret;
}

static int lcd_kit_oneside_setting(struct hisi_fb_data_type *hisifd,
	uint32_t mode)
{
	int ret = LCD_KIT_OK;

	if (!hisifd) {
		HISI_FB_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("oneside driver mode is %d\n", mode);
	if ((mode >= ONESIDE_DRIVER_LEFT) && (mode <= ONESIDE_MODE_EXIT))
		FACT_INFO->oneside_mode.mode = (int)mode;
	switch (mode) {
	case ONESIDE_DRIVER_LEFT:
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->oneside_mode.left_cmds);
		break;
	case ONESIDE_DRIVER_RIGHT:
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->oneside_mode.right_cmds);
		break;
	case ONESIDE_MODE_EXIT:
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->oneside_mode.exit_cmds);
		break;
	default:
		break;
	}
	return ret;
}

static int lcd_kit_aod_detect_setting(struct hisi_fb_data_type *hisifd,
	uint32_t mode)
{
	int ret = LCD_KIT_OK;

	LCD_KIT_INFO("color aod detect mode is %d\n", mode);
	if ((mode >= COLOR_AOD_DETECT_ENTER) &&
		(mode <= COLOR_AOD_DETECT_EXIT))
		FACT_INFO->color_aod_det.mode = (int)mode;
	if (mode == COLOR_AOD_DETECT_ENTER)
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->color_aod_det.enter_cmds);
	else if (mode == COLOR_AOD_DETECT_EXIT)
		ret = lcd_kit_dsi_cmds_tx(hisifd,
			&FACT_INFO->color_aod_det.exit_cmds);
	else
		HISI_FB_ERR("color aod detect mode not support\n");
	return ret;
}

static ssize_t lcd_oneside_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret;
	struct hisi_fb_data_type *hisifd = NULL;
	unsigned int mode = 0;

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off\n");
		return LCD_KIT_FAIL;
	}
	if (!buf) {
		LCD_KIT_ERR("oneside setting store buf null!\n");
		return LCD_KIT_FAIL;
	}
	if (strlen(buf) >= MAX_BUF) {
		LCD_KIT_ERR("buf overflow!\n");
		return LCD_KIT_FAIL;
	}
	ret = sscanf(buf, "%u", &mode);
	if (!ret) {
		LCD_KIT_ERR("sscanf return invaild:%zd\n", ret);
		return LCD_KIT_FAIL;
	}
	LCD_KIT_INFO("oneside set %d\n", mode);
	if (FACT_INFO->oneside_mode.support)
		lcd_kit_oneside_setting(hisifd, mode);
	if (FACT_INFO->color_aod_det.support)
		lcd_kit_aod_detect_setting(hisifd, mode);
	return count;
}

int lcd_kit_avdd_mipi_ctrl(void *hld, int enable)
{
	int ret = LCD_KIT_OK;

	if (hld == NULL) {
		LCD_KIT_ERR("NULL Pointer\n");
		return LCD_KIT_FAIL;
	}
	if (!lcd_kit_is_enter_sleep_mode()) {
		LCD_KIT_INFO("PT mode is not support\n");
		return LCD_KIT_OK;
	}
	if (enable && (FACT_INFO->pt.avdd_disable_cmds.cmds != NULL)) {
		ret = lcd_kit_dsi_cmds_tx(hld,
			&FACT_INFO->pt.avdd_disable_cmds);
		if (ret)
			LCD_KIT_ERR("avdd disable cmd error\n");
		LCD_KIT_INFO("pt avdd disable\n");
	} else if (!enable && (FACT_INFO->pt.avdd_enable_cmds.cmds != NULL)) {
		ret = lcd_kit_dsi_cmds_tx(hld,
			&FACT_INFO->pt.avdd_enable_cmds);
		if (ret)
			LCD_KIT_ERR("avdd enable cmd error\n");
		LCD_KIT_INFO("pt avdd enable\n");
	}
	return ret;
}

struct lcd_fact_ops g_fact_ops = {
	.inversion_mode_show = lcd_inversion_mode_show,
	.inversion_mode_store = lcd_inversion_mode_store,
	.scan_mode_show = lcd_scan_mode_show,
	.scan_mode_store = lcd_scan_mode_store,
	.check_reg_show = lcd_check_reg_show,
	.gram_check_show = lcd_gram_check_show,
	.gram_check_store = lcd_gram_check_store,
	.sleep_ctrl_show = lcd_sleep_ctrl_show,
	.sleep_ctrl_store = lcd_sleep_ctrl_store,
	.poweric_det_show = lcd_poweric_det_show,
	.poweric_det_store = lcd_poweric_det_store,
	.pcd_errflag_check_show = lcd_amoled_pcd_errflag_show,
	.pcd_errflag_check_store = lcd_amoled_pcd_errflag_store,
	.test_config_show = lcd_test_config_show,
	.test_config_store = lcd_test_config_store,
	.lv_detect_show = lcd_lv_detect_show,
	.current_detect_show = lcd_current_detect_show,
	.ldo_check_show = lcd_ldo_check_show,
	.general_test_show = lcd_general_test_show,
	.vtc_line_test_show = lcd_vertical_line_test_show,
	.vtc_line_test_store = lcd_vertical_line_test_store,
	.hkadc_debug_show = lcd_hkadc_debug_show,
	.hkadc_debug_store = lcd_hkadc_debug_store,
	.avdd_detect_show = lcd_avdd_detect_show,
	.oneside_mode_show = lcd_oneside_mode_show,
	.oneside_mode_store = lcd_oneside_mode_store,
};

static void parse_dt_poweric_det(struct device_node *np)
{
	/* poweric detect */
	lcd_kit_parse_u32(np, "lcd-kit,poweric-detect-support",
		&FACT_INFO->poweric_detect.support, 0);
	if (FACT_INFO->poweric_detect.support) {
		lcd_kit_parse_u32(np, "lcd-kit,poweric-number",
			&FACT_INFO->poweric_detect.poweric_num, 0);
		lcd_kit_parse_u32(np, "lcd-kit,poweric-detect-value",
			&FACT_INFO->poweric_detect.detect_val, 0);
		lcd_kit_parse_array_data(np, "lcd-kit,poweric-gpio-enable-list",
			&FACT_INFO->poweric_detect.gpio_list);
		lcd_kit_parse_array_data(np, "lcd-kit,poweric-gpio-enable-val",
			&FACT_INFO->poweric_detect.gpio_val);
		lcd_kit_parse_array_data(np, "lcd-kit,poweric-gpio-enable-number-list",
			&FACT_INFO->poweric_detect.gpio_num_list);
		FACT_INFO->poweric_detect.detect_gpio =
			power_hdl->lcd_elvdd_gpio.buf[0];
	}
}

static void parse_dt_pcd_errflag(struct device_node *np)
{
	/* pcd errflag check */
	lcd_kit_parse_u32(np, "lcd-kit,pcd-errflag-check-support",
		&FACT_INFO->pcd_errflag_check.pcd_errflag_check_support, 0);
	if (FACT_INFO->pcd_errflag_check.pcd_errflag_check_support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,pcd-detect-open-cmds",
			"lcd-kit,pcd-read-cmds-state",
			&FACT_INFO->pcd_errflag_check.pcd_detect_open_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,pcd-detect-close-cmds",
			"lcd-kit,pcd-read-cmds-state",
			&FACT_INFO->pcd_errflag_check.pcd_detect_close_cmds);
	}
}

static void parse_dt_bl_open_short(struct device_node *np)
{
	/* backlight open short test */
	lcd_kit_parse_u32(np, "lcd-kit,bkl-open-short-support",
		&FACT_INFO->bl_open_short_support, 0);
}

static void parse_dt_checksum(struct device_node *np)
{
	/* check sum */
	lcd_kit_parse_u32(np, "lcd-kit,checksum-support",
		&FACT_INFO->checksum.support, 0);
	if (FACT_INFO->checksum.support) {
		lcd_kit_parse_u32(np,
			"lcd-kit,checksum-special-support",
			&FACT_INFO->checksum.special_support, 0);
		lcd_kit_parse_u32(np, "lcd-kit,checksum-need-clear-sram",
			&FACT_INFO->checksum.clear_sram, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-enable-cmds",
			"lcd-kit,checksum-enable-cmds-state",
			&FACT_INFO->checksum.enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-disable-cmds",
			"lcd-kit,checksum-disable-cmds-state",
			&FACT_INFO->checksum.disable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-cmds",
			"lcd-kit,checksum-cmds-state",
			&FACT_INFO->checksum.checksum_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-clear-sram-cmds",
			"lcd-kit,checksum-clear-sram-cmds-state",
			&FACT_INFO->checksum.clear_sram_cmds);
		lcd_kit_parse_arrays_data(np, "lcd-kit,checksum-value",
			&FACT_INFO->checksum.value, CHECKSUM_VALUE_SIZE);
		lcd_kit_parse_arrays_data(np, "lcd-kit,checksum-dsi1-value",
			&FACT_INFO->checksum.dsi1_value, CHECKSUM_VALUE_SIZE);
		/* checksum stress test */
		lcd_kit_parse_u32(np,
			"lcd-kit,checksum-stress-test-support",
			&FACT_INFO->checksum.stress_test_support, 0);
		if (FACT_INFO->checksum.stress_test_support) {
			lcd_kit_parse_u32(np,
				"lcd-kit,checksum-stress-test-vdd",
				&FACT_INFO->checksum.vdd, 0);
			lcd_kit_parse_u32(np,
				"lcd-kit,checksum-stress-test-mipi",
				&FACT_INFO->checksum.mipi_clk, 0);
		}
	}
}

static void parse_dt_hkadc(struct device_node *np)
{
	/* hkadc */
	lcd_kit_parse_u32(np, "lcd-kit,hkadc-support",
		&FACT_INFO->hkadc.support, 0);
	if (FACT_INFO->hkadc.support)
		lcd_kit_parse_u32(np, "lcd-kit,hkadc-value",
			&FACT_INFO->hkadc.value, 0);
}

static void parse_dt_curr_det(struct device_node *np)
{
	/* current detect */
	lcd_kit_parse_u32(np, "lcd-kit,current-det-support",
		&FACT_INFO->current_det.support, 0);
	if (FACT_INFO->current_det.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,current-det-cmds",
			"lcd-kit,current-det-cmds-state",
			&FACT_INFO->current_det.detect_cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,current-det-value",
			&FACT_INFO->current_det.value);
	}
}

static void parse_dt_ldo_check(struct device_node *np)
{
	int ret;
	char *name[LDO_NUM_MAX] = {NULL};
	int i;

	/* ldo check */
	lcd_kit_parse_u32(np, "lcd-kit,ldo-check-support",
		&FACT_INFO->ldo_check.support, 0);
	if (FACT_INFO->ldo_check.support) {
		FACT_INFO->ldo_check.ldo_num = of_property_count_elems_of_size(
			np, "lcd-kit,ldo-check-channel", sizeof(u32));
		if (FACT_INFO->ldo_check.ldo_num > 0) {
			ret = of_property_read_u32_array(np,
				"lcd-kit,ldo-check-channel",
				FACT_INFO->ldo_check.ldo_channel,
				FACT_INFO->ldo_check.ldo_num);
			if (ret < 0)
				LCD_KIT_ERR("parse ldo channel fail\n");
			ret = of_property_read_u32_array(np,
				"lcd-kit,ldo-check-threshold",
				FACT_INFO->ldo_check.curr_threshold,
				FACT_INFO->ldo_check.ldo_num);
			if (ret < 0)
				LCD_KIT_ERR("parse current threshold fail\n");
			ret = of_property_read_string_array(np,
				"lcd-kit,ldo-check-name",
				(const char **)&name[0],
				FACT_INFO->ldo_check.ldo_num);
			if (ret < 0)
				LCD_KIT_ERR("parse ldo name fail\n");
			for (i = 0; i < (int)FACT_INFO->ldo_check.ldo_num; i++)
				strncpy(FACT_INFO->ldo_check.ldo_name[i],
					name[i], LDO_NAME_LEN_MAX - 1);
		}
	}
}

static void parse_dt_avdd_det(struct device_node *np)
{
	/* ddic low voltage detect test */
	lcd_kit_parse_u32(np, "lcd-kit,avdd-det-support",
		&FACT_INFO->avdd_detect.support, 0);
	if (FACT_INFO->avdd_detect.support) {
		lcd_kit_parse_u32(np, "lcd-kit,avdd-det-vb-channel",
			&FACT_INFO->avdd_detect.vb_channel, 0);
		lcd_kit_parse_u32(np, "lcd-kit,avdd-det-low-threshold",
			&FACT_INFO->avdd_detect.low_threshold, 0);
		lcd_kit_parse_u32(np, "lcd-kit,avdd-det-high-threshold",
			&FACT_INFO->avdd_detect.high_threshold, 0);
		lcd_kit_parse_u32(np, "lcd-kit,avdd-det-gpio-grp-ctrl",
			&FACT_INFO->avdd_detect.gpio_grp_ctrl, 0);
		if (FACT_INFO->avdd_detect.gpio_grp_ctrl) {
			(void)lcd_kit_parse_array_data(np,
				"lcd-kit,avdd-det-gpio-grp",
				&FACT_INFO->avdd_detect.gpio_grp);
			(void)lcd_kit_parse_array_data(np,
				"lcd-kit,avdd-det-gpio-ctrl-a",
				&FACT_INFO->avdd_detect.gpio_ctrl_a);
			(void)lcd_kit_parse_array_data(np,
				"lcd-kit,avdd-det-gpio-ctrl-b",
				&FACT_INFO->avdd_detect.gpio_ctrl_b);
		}
	}
}

static void parse_dt(struct device_node *np)
{
	/* parse pcd errflag */
	parse_dt_pcd_errflag(np);
	/* parse backlight open short */
	parse_dt_bl_open_short(np);
	/* parse checksum */
	parse_dt_checksum(np);
	/* parse hkadc */
	parse_dt_hkadc(np);
	/* parse current detect */
	parse_dt_curr_det(np);
	/* parse ldo check */
	parse_dt_ldo_check(np);
	/* parse poweric detect */
	parse_dt_poweric_det(np);
	/* parse avdd detect */
	parse_dt_avdd_det(np);
}

int lcd_factory_init(struct device_node *np)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;

	lcd_kit_fact_init(np);
	parse_dt(np);

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_fact_ops_register(&g_fact_ops);
	ret = lcd_create_fact_sysfs(&hisifd->fbi->dev->kobj);
	if (ret)
		LCD_KIT_ERR("create factory sysfs fail\n");
	return LCD_KIT_OK;
}

