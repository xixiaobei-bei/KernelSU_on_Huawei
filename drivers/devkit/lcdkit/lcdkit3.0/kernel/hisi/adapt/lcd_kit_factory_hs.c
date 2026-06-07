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

	if (!g_fact_info.checksum.stress_test_support) {
		LCD_KIT_ERR("not support checksum stress test\n");
		return;
	}
	if (g_fact_info.checksum.mipi_clk) {
		mipi_clk = hisifd->panel_info.mipi.dsi_bit_clk_upt;
		g_fact_info.checksum.rec_mipi_clk = mipi_clk;
		checksum_set_mipi_clk(hisifd, g_fact_info.checksum.mipi_clk);
	}
	if (g_fact_info.checksum.vdd) {
		if (power_hdl->lcd_vdd.buf == NULL) {
			LCD_KIT_ERR("lcd vdd buf is null\n");
			return;
		}
		lcd_vdd = power_hdl->lcd_vdd.buf[POWER_VOL];
		g_fact_info.checksum.rec_vdd = lcd_vdd;
		checksum_set_vdd(g_fact_info.checksum.vdd);
		ret = lcd_power_set_vol(LCD_KIT_VDD);
		if (ret)
			LCD_KIT_ERR("set voltage error\n");
	}
	lcd_kit_recovery_display(hisifd);
}

static void checksum_stress_disable(struct hisi_fb_data_type *hisifd)
{
	int ret;

	if (!g_fact_info.checksum.stress_test_support) {
		LCD_KIT_ERR("not support checksum stress test\n");
		return;
	}
	if (g_fact_info.checksum.mipi_clk)
		checksum_set_mipi_clk(hisifd,
			g_fact_info.checksum.rec_mipi_clk);
	if (g_fact_info.checksum.vdd) {
		if (power_hdl->lcd_vdd.buf == NULL) {
			LCD_KIT_ERR("lcd vdd buf is null\n");
			return;
		}
		checksum_set_vdd(g_fact_info.checksum.rec_vdd);
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
	g_fact_info.checksum.status = LCD_KIT_CHECKSUM_START;
	lcd_kit_dsi_cmds_tx(hisifd, &g_fact_info.checksum.enable_cmds);
	g_fact_info.checksum.pic_index = INVALID_INDEX;
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
	if (g_fact_info.checksum.status == LCD_KIT_CHECKSUM_END) {
		if (pic_index == TEST_PIC_0) {
			LCD_KIT_INFO("start gram checksum\n");
			lcd_enable_checksum(hisifd);
			return ret;
		}
		LCD_KIT_INFO("pic index error\n");
		return LCD_KIT_FAIL;
	}
	if (pic_index == TEST_PIC_2)
		g_fact_info.checksum.check_count++;
	if (g_fact_info.checksum.check_count == CHECKSUM_CHECKCOUNT) {
		LCD_KIT_INFO("check 5 times, set checksum end\n");
		g_fact_info.checksum.status = LCD_KIT_CHECKSUM_END;
		g_fact_info.checksum.check_count = 0;
	}
	switch (pic_index) {
	case TEST_PIC_0:
	case TEST_PIC_1:
	case TEST_PIC_2:
		LCD_KIT_INFO("set pic [%d]\n", pic_index);
		g_fact_info.checksum.pic_index = pic_index;
		break;
	default:
		LCD_KIT_INFO("set pic [%d] unknown\n", pic_index);
		g_fact_info.checksum.pic_index = INVALID_INDEX;
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

	if (g_fact_info.checksum.value.arry_data == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	pic_index = g_fact_info.checksum.pic_index;
	ret = lcd_kit_dsi_cmds_rx(hisifd, read_value,
		&g_fact_info.checksum.checksum_cmds);
	if (ret) {
		LCD_KIT_ERR("checksum read dsi0 error\n");
		return LCD_KIT_FAIL;
	}
	check_cnt = g_fact_info.checksum.value.arry_data[pic_index].cnt;
	if (check_cnt > LCD_KIT_CHECKSUM_SIZE) {
		LCD_KIT_ERR("checksum count is larger than checksum size\n");
		return LCD_KIT_FAIL;
	}
	checksum = g_fact_info.checksum.value.arry_data[pic_index].buf;
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

	if (g_fact_info.checksum.dsi1_value.arry_data == NULL) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	pic_index = g_fact_info.checksum.pic_index;
	ret = lcd_kit_dsi1_cmds_rx(hisifd, read_value,
		&g_fact_info.checksum.checksum_cmds);
	if (ret) {
		LCD_KIT_ERR("checksum read dsi1 error\n");
		return LCD_KIT_FAIL;
	}
	check_cnt = g_fact_info.checksum.dsi1_value.arry_data[pic_index].cnt;
	if (check_cnt > LCD_KIT_CHECKSUM_SIZE) {
		LCD_KIT_ERR("checksum count is larger than checksum size\n");
		return LCD_KIT_FAIL;
	}
	checksum = g_fact_info.checksum.dsi1_value.arry_data[pic_index].buf;
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

	if (!g_fact_info.current_det.support) {
		LCD_KIT_INFO("current detect is not support! return!\n");
		return LCD_KIT_OK;
	}
	lcd_kit_dsi_cmds_rx(hisifd, &rd, &g_fact_info.current_det.detect_cmds);
	LCD_KIT_INFO("current detect, read value = 0x%x\n", rd);
	/* buf[0] means current detect result */
	if ((rd & g_fact_info.current_det.value.buf[0]) == LCD_KIT_OK) {
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

	if (!g_fact_info.lv_det.support) {
		LCD_KIT_INFO("current detect is not support! return!\n");
		return LCD_KIT_OK;
	}
	lcd_kit_dsi_cmds_rx(hisifd, &rd, &g_fact_info.lv_det.detect_cmds);
	LCD_KIT_INFO("current detect, read value = 0x%x\n", rd);
	if ((rd & g_fact_info.lv_det.value.buf[0]) == 0) {
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
	LCD_KIT_INFO("g_fact_info.hor_line.duration = %d\n",
			g_fact_info.hor_line.duration);
	/* disable esd check */
	lcd_esd_enable(hisifd, 0);
	while (count--) {
		/* hardware reset */
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
		if (g_fact_info.hor_line.hl_cmds.cmds != NULL) {
			ret = lcd_kit_dsi_cmds_tx(hisifd,
				&g_fact_info.hor_line.hl_cmds);
			if (ret)
				LCD_KIT_ERR("send avdd cmd error\n");
		}
		hisifb_deactivate_vsync(hisifd);
		up(&hisifd->blank_sem);
		msleep(g_fact_info.hor_line.duration * MILLISEC_TIME);
		/* recovery display */
		lcd_kit_recovery_display(hisifd);
	}
	/* enable esd */
	lcd_esd_enable(hisifd, 1);
	LCD_KIT_INFO("horizontal line test end\n");
	return ret;
}

static int gpio_pcd_pro(u8 *result_value, int *pcd_gpio)
{
	int ret;

	ret = gpio_request(g_fact_info.gpio_pcd, GPIO_LCDKIT_PCD_NAME);
	if (ret != 0) {
		*result_value = PCD_ERRFLAG_FAIL;
		LCD_KIT_ERR("pcd__gpio[%d] request fail!\n",
			g_fact_info.gpio_pcd);
		return ret;
	}
	ret = gpio_direction_input(g_fact_info.gpio_pcd);
	if (ret != 0) {
		gpio_free(g_fact_info.gpio_pcd);
		*result_value = PCD_ERRFLAG_FAIL;
		LCD_KIT_ERR("pcd__gpio[%d] direction set fail!\n",
			g_fact_info.gpio_pcd);
		return ret;
	}
	*pcd_gpio = gpio_get_value(g_fact_info.gpio_pcd);
	gpio_free(g_fact_info.gpio_pcd);
	return ret;
}

static int gpio_errflag_pro(u8 *result_value, int *errflag_gpio)
{
	int ret;

	ret = gpio_request(g_fact_info.gpio_errflag,
		GPIO_LCDKIT_ERRFLAG_NAME);
	if (ret != 0) {
		*result_value = PCD_ERRFLAG_FAIL;
		LCD_KIT_ERR("pcd_errflag_gpio[%d] request fail!\n",
			g_fact_info.gpio_errflag);
		return ret;
	}
	ret = gpio_direction_input(g_fact_info.gpio_errflag);
	if (ret != 0) {
		gpio_free(g_fact_info.gpio_errflag);
		*result_value = PCD_ERRFLAG_FAIL;
		LCD_KIT_ERR("pcd_errflag_gpio[%d] direction set fail!\n",
			g_fact_info.gpio_errflag);
		return ret;
	}
	*errflag_gpio = gpio_get_value(g_fact_info.gpio_errflag);
	gpio_free(g_fact_info.gpio_errflag);
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

	hisifd = dev_get_hisifd(dev);
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
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

	pic_index = g_fact_info.checksum.pic_index;
	LCD_KIT_INFO("checksum pic num:%d\n", pic_index);
	if (pic_index > TEST_PIC_2) {
		LCD_KIT_ERR("checksum pic num unknown:%d\n", pic_index);
		return LCD_KIT_FAIL;
	}
	ret = lcd_check_checksum(hisifd);
	if (ret)
		LCD_KIT_ERR("checksum error\n");
	if (lcd_is_dual_mipi()) {
		ret = lcd_check_dsi1_checksum(hisifd);
		if (ret)
			LCD_KIT_ERR("dsi1 checksum error\n");
	}
	if (ret && g_fact_info.checksum.pic_index == TEST_PIC_2)
		g_fact_info.checksum.status = LCD_KIT_CHECKSUM_END;

	if (g_fact_info.checksum.status == LCD_KIT_CHECKSUM_END) {
		lcd_kit_dsi_cmds_tx(hisifd, &g_fact_info.checksum.disable_cmds);
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
	if (g_fact_info.checksum.support) {
		down(&hisifd->blank_sem);
		if (hisifd->panel_power_on) {
			hisifb_activate_vsync(hisifd);
			checksum_result = lcd_kit_checksum_check(hisifd);
			hisifb_deactivate_vsync(hisifd);
			ret = snprintf(buf, PAGE_SIZE, "%d\n", checksum_result);
		}
		up(&hisifd->blank_sem);
		/* disable checksum stress test, restore mipi clk and vdd */
		if (g_fact_info.checksum.status == LCD_KIT_CHECKSUM_END)
			checksum_stress_disable(hisifd);
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
	if (g_fact_info.checksum.support) {
		/* enable checksum stress test, promote mipi clk and vdd */
		if (g_fact_info.checksum.status == LCD_KIT_CHECKSUM_END)
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

static ssize_t lcd_amoled_gpio_pcd_errflag(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = PCD_ERRFLAG_SUCCESS;
	int pcd_gpio = 0;
	int errflag_gpio = 0;
	u8 result_value = 0;

	if (buf == NULL) {
		LCD_KIT_ERR("buf is null\n");
		return LCD_KIT_FAIL;
	}

	if (!g_fact_info.pcd_errflag_check_support) {
		LCD_KIT_INFO("pcd_errflag check default pass\n");
		return LCD_KIT_OK;
	}
	if (g_fact_info.gpio_pcd != 0) {
		ret = gpio_pcd_pro(&result_value, &pcd_gpio);
		if (ret != 0) {
			LCD_KIT_ERR("gpio_pcd_pro fail!\n");
			return ret;
		}
	}
	if (g_fact_info.gpio_errflag != 0) {
		ret = gpio_errflag_pro(&result_value, &errflag_gpio);
		if (ret != 0) {
			LCD_KIT_ERR("gpio_errflag_pro fail!\n");
			return ret;
		}
	}
	LCD_KIT_INFO("pcd_gpio[%d]=%d,errflag_gpio[%d]=%d\n", g_fact_info.gpio_pcd,
		pcd_gpio, g_fact_info.gpio_errflag, errflag_gpio);
	if ((pcd_gpio == GPIO_LOW_PCDERRFLAG) &&
		(errflag_gpio == GPIO_LOW_PCDERRFLAG))
		result_value = PCD_ERRFLAG_SUCCESS; // PCD_ERR_FLAG_SUCCESS
	if ((pcd_gpio == GPIO_HIGH_PCDERRFLAG) &&
		(errflag_gpio == GPIO_LOW_PCDERRFLAG))
		result_value = PCD_FAIL; // only  PCD_FAIL
	if ((pcd_gpio == GPIO_LOW_PCDERRFLAG) &&
		(errflag_gpio == GPIO_HIGH_PCDERRFLAG))
		result_value = ERRFLAG_FAIL; // only ERRFLAG FAIL
	if ((pcd_gpio == GPIO_HIGH_PCDERRFLAG) &&
		(errflag_gpio == GPIO_HIGH_PCDERRFLAG))
		result_value = PCD_ERRFLAG_FAIL; // PCD_ERR_FLAG_FAIL
	else
		result_value = PCD_ERRFLAG_SUCCESS;
err_out:
	LCD_KIT_INFO("pcd_errflag result_value = %d\n", result_value);
	ret = snprintf(buf, PAGE_SIZE, "%d\n", result_value);
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
	if (ret) {
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

	ldo->lcd_ldo_num = g_fact_info.ldo_check.ldo_num;
	memcpy(ldo->lcd_ldo_name, g_fact_info.ldo_check.ldo_name,
		sizeof(g_fact_info.ldo_check.ldo_name));
	memcpy(ldo->lcd_ldo_channel, g_fact_info.ldo_check.ldo_channel,
		sizeof(g_fact_info.ldo_check.ldo_channel));
	memcpy(ldo->lcd_ldo_current, g_fact_info.ldo_check.ldo_current,
		sizeof(g_fact_info.ldo_check.ldo_current));
	memcpy(ldo->lcd_ldo_threshold, g_fact_info.ldo_check.curr_threshold,
		sizeof(g_fact_info.ldo_check.curr_threshold));
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
	if (!g_fact_info.ldo_check.support) {
		LCD_KIT_INFO("ldo check not support\n");
		return len;
	}
	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		LCD_KIT_ERR("panel is power off\n");
		up(&hisifd->blank_sem);
		return LCD_KIT_FAIL;
	}
	for (i = 0; i < g_fact_info.ldo_check.ldo_num; i++) {
		sum_current = 0;
		temp_max_value = 0;
		for (j = 0; j < LDO_CHECK_COUNT; j++) {
#if defined(CONFIG_HISI_HKADC)
			cur_val = hisi_adc_get_current(g_fact_info.ldo_check.ldo_channel[i]);
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
			g_fact_info.ldo_check.ldo_current[i] = -1;
		else
			g_fact_info.ldo_check.ldo_current[i] =
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
	if (g_fact_info.hor_line.support)
		ret = lcd_hor_line_test(hisifd);

	if (ret == 0)
		ret = snprintf(buf, PAGE_SIZE, "OK\n");
	else
		ret = snprintf(buf, PAGE_SIZE, "FAIL\n");

	return ret;
}

static int lcd_vtc_line_test(struct hisi_fb_data_type *hisifd,
	unsigned long pic_index)
{
	int ret = LCD_KIT_OK;

	switch (pic_index) {
	case PIC1_INDEX:
		/* disable esd */
		lcd_esd_enable(hisifd, 0);
		/* hardware reset */
		lcd_hardware_reset();
		mdelay(20);
		if (g_fact_info.vtc_line.vtc_cmds.cmds != NULL) {
			ret = lcd_kit_dsi_cmds_tx(hisifd,
				&g_fact_info.vtc_line.vtc_cmds);
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
	if (g_fact_info.vtc_line.support) {
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

	if (g_fact_info.bl_open_short_support) {
		if (bl_ops->bl_self_test)
			ret = bl_ops->bl_self_test();
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", ret);
}

static ssize_t lcd_hkadc_debug_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret = LCD_KIT_OK;

	if (g_fact_info.hkadc.support)
		ret = snprintf(buf, PAGE_SIZE, "%d\n", g_fact_info.hkadc.value);

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
	if (g_fact_info.hkadc.support) {
		ret = sscanf(buf, "%u", &channel);
		if (ret) {
			LCD_KIT_ERR("ivalid parameter!\n");
			return ret;
		}
#if defined(CONFIG_HISI_HKADC)
		g_fact_info.hkadc.value = hisi_adc_get_value(channel);
#endif
	}
	return count;
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
	.pcd_errflag_check_show = lcd_amoled_pcd_errflag_show,
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
};

static void parse_dt_pcd_errflag(struct device_node *np)
{
	/* pcd errflag check */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,pcd-errflag-check-support",
		&g_fact_info.pcd_errflag_check_support, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,gpio-pcd",
		&g_fact_info.gpio_pcd, 0);
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,gpio-errflag",
		&g_fact_info.gpio_errflag, 0);
}

static void parse_dt_bl_open_short(struct device_node *np)
{
	/* backlight open short test */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,bkl-open-short-support",
		&g_fact_info.bl_open_short_support, 0);
}

static void parse_dt_checksum(struct device_node *np)
{
	/* check sum */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,checksum-support",
		&g_fact_info.checksum.support, 0);
	if (g_fact_info.checksum.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-enable-cmds",
			"lcd-kit,checksum-enable-cmds-state",
			&g_fact_info.checksum.enable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-disable-cmds",
			"lcd-kit,checksum-disable-cmds-state",
			&g_fact_info.checksum.disable_cmds);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,checksum-cmds",
			"lcd-kit,checksum-cmds-state",
			&g_fact_info.checksum.checksum_cmds);
		lcd_kit_parse_arrays_data(np, "lcd-kit,checksum-value",
			&g_fact_info.checksum.value, CHECKSUM_VALUE_SIZE);
		if (lcd_is_dual_mipi())
			lcd_kit_parse_arrays_data(np,
				"lcd-kit,checksum-dsi1-value",
				&g_fact_info.checksum.dsi1_value,
				CHECKSUM_VALUE_SIZE);
		/* checksum stress test */
		OF_PROPERTY_READ_U32_DEFAULT(np,
			"lcd-kit,checksum-stress-test-support",
			&g_fact_info.checksum.stress_test_support, 0);
		if (g_fact_info.checksum.stress_test_support) {
			OF_PROPERTY_READ_U32_DEFAULT(np,
				"lcd-kit,checksum-stress-test-vdd",
				&g_fact_info.checksum.vdd, 0);
			OF_PROPERTY_READ_U32_DEFAULT(np,
				"lcd-kit,checksum-stress-test-mipi",
				&g_fact_info.checksum.mipi_clk, 0);
		}
	}
}

static void parse_dt_hkadc(struct device_node *np)
{
	/* hkadc */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,hkadc-support",
		&g_fact_info.hkadc.support, 0);
	if (g_fact_info.hkadc.support)
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,hkadc-value",
			&g_fact_info.hkadc.value, 0);
}

static void parse_dt_curr_det(struct device_node *np)
{
	/* current detect */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,current-det-support",
		&g_fact_info.current_det.support, 0);
	if (g_fact_info.current_det.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,current-det-cmds",
			"lcd-kit,current-det-cmds-state",
			&g_fact_info.current_det.detect_cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,current-det-value",
			&g_fact_info.current_det.value);
	}
}

static void parse_dt_lv_det(struct device_node *np)
{
	/* low voltage detect */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,lv-det-support",
		&g_fact_info.lv_det.support, 0);
	if (g_fact_info.lv_det.support) {
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,lv-det-cmds",
			"lcd-kit,lv-det-cmds-state",
			&g_fact_info.lv_det.detect_cmds);
		lcd_kit_parse_array_data(np, "lcd-kit,lv-det-value",
			&g_fact_info.lv_det.value);
	}
}

static void parse_dt_ldo_check(struct device_node *np)
{
	int ret;
	char *name[LDO_NUM_MAX] = {NULL};
	int i;

	/* ldo check */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,ldo-check-support",
		&g_fact_info.ldo_check.support, 0);
	if (g_fact_info.ldo_check.support) {
		g_fact_info.ldo_check.ldo_num = of_property_count_elems_of_size(
			np, "lcd-kit,ldo-check-channel", sizeof(u32));
		if (g_fact_info.ldo_check.ldo_num > 0) {
			ret = of_property_read_u32_array(np,
				"lcd-kit,ldo-check-channel",
				g_fact_info.ldo_check.ldo_channel,
				g_fact_info.ldo_check.ldo_num);
			if (ret < 0)
				LCD_KIT_ERR("parse ldo channel fail\n");
			ret = of_property_read_u32_array(np,
				"lcd-kit,ldo-check-threshold",
				g_fact_info.ldo_check.curr_threshold,
				g_fact_info.ldo_check.ldo_num);
			if (ret < 0)
				LCD_KIT_ERR("parse current threshold fail\n");
			ret = of_property_read_string_array(np,
				"lcd-kit,ldo-check-name",
				(const char **)&name[0],
				g_fact_info.ldo_check.ldo_num);
			if (ret < 0)
				LCD_KIT_ERR("parse ldo name fail\n");
			for (i = 0; i < (int)g_fact_info.ldo_check.ldo_num; i++)
				strncpy(g_fact_info.ldo_check.ldo_name[i],
					name[i], LDO_NAME_LEN_MAX - 1);
		}
	}
}

static void parse_dt_vtc_line(struct device_node *np)
{
	/* vertical line test */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,vtc-line-support",
		&g_fact_info.vtc_line.support, 0);
	if (g_fact_info.vtc_line.support)
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,vtc-line-cmds",
			"lcd-kit,vtc-line-cmds-state",
			&g_fact_info.vtc_line.vtc_cmds);
}

static void parse_dt_hor_line(struct device_node *np)
{
	/* horizontal line test */
	OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,hor-line-support",
		&g_fact_info.hor_line.support, 0);
	if (g_fact_info.hor_line.support) {
		OF_PROPERTY_READ_U32_DEFAULT(np, "lcd-kit,hor-line-duration",
			&g_fact_info.hor_line.duration, 0);
		lcd_kit_parse_dcs_cmds(np, "lcd-kit,hor-line-cmds",
			"lcd-kit,hor-line-cmds-state",
			&g_fact_info.hor_line.hl_cmds);
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
	/* parse low voltage detect */
	parse_dt_lv_det(np);
	/* parse ldo check */
	parse_dt_ldo_check(np);
	/* parse vertical line test */
	parse_dt_vtc_line(np);
	/* parse horizontal line test */
	parse_dt_hor_line(np);
}

int lcd_factory_init(struct device_node *np)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret;

	hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	if (!hisifd) {
		LCD_KIT_ERR("hisifd is null\n");
		return LCD_KIT_FAIL;
	}
	lcd_fact_ops_register(&g_fact_ops);
	ret = lcd_create_fact_sysfs(&hisifd->fbi->dev->kobj);
	if (ret)
		LCD_KIT_ERR("create factory sysfs fail\n");
	lcd_kit_fact_init(np);
	parse_dt(np);
	return LCD_KIT_OK;
}

