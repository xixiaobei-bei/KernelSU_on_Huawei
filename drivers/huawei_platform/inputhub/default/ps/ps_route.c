/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ps route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "ps_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/mtd/hisi_nve_interface.h>

#include <asm/io.h>
#include <asm/memory.h>
#include <securec.h>

#include "contexthub_route.h"
#include "ps_sysfs.h"

#define PS_GESTURE_ON                 1
#define PS_GESTURE_OFF                0

struct ps_ioctl_t {
	uint32_t sub_cmd;
	uint16_t ps_rcv_info;
};

struct ps_lcd_te_freq {
	struct work_struct ps_ud_work;
	uint32_t lcd_frequency;
};

struct workqueue_struct *ps_ud_workqueue = NULL;
static uint16_t ps_rcv_request;
static struct work_struct g_erase_ps_nv_work;
static struct workqueue_struct *g_erase_ps_nv_workqueue;

static int ps_commu(unsigned int cmd, unsigned int para, uint16_t action)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	struct ps_ioctl_t ps_ioctl;

	pkg_ap.tag = TAG_PS;
	pkg_ap.cmd = cmd;
	ps_ioctl.sub_cmd = para;
	ps_ioctl.ps_rcv_info = action;
	pkg_ap.wr_buf = &ps_ioctl;
	pkg_ap.wr_len = sizeof(ps_ioctl);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send ps cmd %d to mcu fail,ret=%d\n", cmd, ret);
		return ret;
	}
	return ret;
}

static int ps_rcv_send(void)
{
	unsigned int cmd;
	unsigned int sub_cmd;
	unsigned int ret;

	cmd = CMD_CMN_CONFIG_REQ;
	sub_cmd = SUB_CMD_PS_RCV_STATUS_REQ;
	ret = ps_commu(cmd, sub_cmd, ps_rcv_request);
	if (ret) {
		hwlog_err("%s: ps send rcv status fail\n", __FUNCTION__);
		return ret;
	}
	hwlog_info("%s: ps send rcv status succsess, %d\n", __FUNCTION__, ps_rcv_request);

	return ret;
}

/*
 * Description: ps request telephone call status to enable ps_gesture
 *              value:1--enable ps_gesture
 *              value:0--disable ps_gesture
 */
void ps_telecall_status_change(unsigned long value)
{
	if (value == PS_GESTURE_ON)
		ps_rcv_request = 1;
	else if (value == PS_GESTURE_OFF)
		ps_rcv_request = 0;

	ps_rcv_send();
}

int send_ps_lcd_freq_to_mcu(int tag, uint32_t subcmd, const void *data,
	int length, bool is_recovery)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = subcmd;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = length+SUBCMD_LEN;
	if (memcpy_s(cpkt.para, sizeof(cpkt.para), data, length) != EOK) {
		hwlog_err("%s: memcpy para fail\n", __func__);
		return -1;
	}

	if (is_recovery)
		return write_customize_cmd(&pkg_ap, NULL, false);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send als ud data to mcu fail,ret=%d\n", ret);
		return -1;
	}

	return 0;
}

void recv_lcd_freq_process(struct work_struct *ps_work)
{
	struct ps_lcd_te_freq *te_freq = NULL;

	te_freq = container_of(ps_work, struct ps_lcd_te_freq, ps_ud_work);
	if (te_freq == NULL) {
		hwlog_err("%s: NULL Pointer\n", __func__);
		return;
	}
	hwlog_info("%s: start process lcd_freq\n", __func__);
	send_ps_lcd_freq_to_mcu(TAG_PS, SUB_CMD_PS_LCD_FREQ_REQ,
		(const void *)&(te_freq->lcd_frequency),
		sizeof(te_freq->lcd_frequency), false);
	send_ps_lcd_freq_to_mcu(TAG_ALS, SUB_CMD_ALS_LCD_FREQ_REQ,
		(const void *)&(te_freq->lcd_frequency),
		sizeof(te_freq->lcd_frequency), false);
	kfree(te_freq);
}

void ps_lcd_freq_queue_work(uint32_t lcd_freq)
{
	struct ps_lcd_te_freq *ps_work = NULL;
	int ret;

	ps_work = kmalloc(sizeof(struct ps_lcd_te_freq), GFP_ATOMIC);
	if (!ps_work)
		return;
	ret = memset_s(ps_work, sizeof(struct ps_lcd_te_freq),
		0, sizeof(struct ps_lcd_te_freq));
	if (ret != EOK) {
		kfree(ps_work);
		ps_work = NULL;
		hwlog_err("%s: memset_s ps_work fail\n", __func__);
		return;
	}
	ps_work->lcd_frequency = lcd_freq;
	INIT_WORK(&ps_work->ps_ud_work, recv_lcd_freq_process);
	queue_work(ps_ud_workqueue, &ps_work->ps_ud_work);
}

void ps_send_lcd_freq_to_sensorhub(uint32_t lcd_freq)
{
	struct ps_platform_data *pf_data = NULL;

	pf_data = ps_get_platform_data(TAG_PS);
	if (pf_data == NULL)
		return;
	hwlog_info("%s: recv lcd_freq %d\n", __func__, lcd_freq);
	if (pf_data->is_need_lcd_freq)
		ps_lcd_freq_queue_work(lcd_freq);
}

ssize_t show_ps_sensorlist_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (memcpy_s(buf, MAX_STR_SIZE, get_sensorlist_info_by_index(PS),
		sizeof(struct sensorlist_info)) != EOK)
		return -1;
	return sizeof(struct sensorlist_info);
}

ssize_t show_ps_offset_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"ps OFFSET:%d %d %d\n",
		ps_sensor_offset[0], ps_sensor_offset[1], ps_sensor_offset[2]);
}

/* return ps_support_under_screen_cali to node file */
ssize_t attr_ps_calibrate_after_sale_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct ps_device_info *dev_info = NULL;

	if (!dev || !attr || !buf)
		return -1;
	dev_info = ps_get_device_info(TAG_PS);
	if (dev_info == NULL)
		return -1;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		dev_info->ps_support_cali_after_sale);
}

int sleeve_test_ps_prepare(int ps_config)
{
	int ret;
	struct write_info pkg_ap = { 0 };
	struct read_info pkg_mcu = { 0 };

	pkg_ap.tag = TAG_PS;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &ps_config;
	pkg_ap.wr_len = sizeof(ps_config);
	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret) {
		hwlog_err("send sleeve_test ps config cmd to mcu fail,ret=%d\n", ret);
		return ret;
	}
	if (pkg_mcu.errno != 0)
		hwlog_err("sleeve_test ps config fail %d\n", pkg_mcu.errno);
	return ret;
}

int ps_ud_workqueue_init(void)
{
	ps_ud_workqueue = create_freezable_workqueue("ps_ud_te_freq");
	if (ps_ud_workqueue == NULL) {
		hwlog_err("creat ps work queue failed!\n");
		return -EINVAL;
	}
	return 0;
}

static void ps_erase_nv_after_sale(struct work_struct *work)
{
	int32_t temp_buf[PS_CALIBRATE_DATA_LENGTH] = { 0 };
	int ret;

	if (read_calibrate_data_from_nv(PS_CALIDATA_NV_NUM,
		PS_CALIDATA_NV_SIZE, "PSENSOR"))
		hwlog_err("%s, nv read fail\n", __func__);
	if (memcpy_s(temp_buf, sizeof(temp_buf),
		user_info.nv_data, sizeof(temp_buf)) != EOK)
		hwlog_err("%s, memcpy temp_buf fail\n", __func__);
	hwlog_info("%s, nve_direct_access read ps_offset buf[0]~buf[3]%d,%d,%d,%d\n",
		__func__, temp_buf[PS_XTALK0], temp_buf[PS_5CM_PDATA0],
		temp_buf[PS_3CM_PDATA0], temp_buf[PS_OFFSET_NEW0]);
	hwlog_info("%s, nve_direct_access read ps_offset buf[6]~buf[9]%d,%d,%d,%d, buf[12]%d\n",
		__func__, temp_buf[PS_XTALK1], temp_buf[PS_5CM_PDATA1],
		temp_buf[PS_3CM_PDATA1], temp_buf[PS_OFFSET_NEW1],
		temp_buf[PS_PULSE_LEN]);
	if (memset_s(temp_buf, sizeof(temp_buf),
		0, sizeof(temp_buf)) != EOK)
		hwlog_err("%s, memset temp_buf\n", __func__);
	ret = write_ps_offset_to_nv(temp_buf, sizeof(temp_buf));
	if (ret)
		hwlog_err("nv write fail\n");
}

void identify_phone_type(int type)
{
	hwlog_info("%s:type %d\n", __func__, type);
	if (type == OCEAN_3718 || type == NOAH_NOAHP_3718 || type == JADE_3718) {
		if (!g_erase_ps_nv_workqueue) {
			g_erase_ps_nv_workqueue = create_freezable_workqueue("erase_ps_nv");
			if (!g_erase_ps_nv_workqueue) {
				hwlog_err("create erase ps nv work queue failed!\n");
				return;
			}
			INIT_WORK(&g_erase_ps_nv_work, ps_erase_nv_after_sale);
		}
		queue_work(g_erase_ps_nv_workqueue, &g_erase_ps_nv_work);
	}
}
