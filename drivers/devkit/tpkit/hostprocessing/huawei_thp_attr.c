/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "huawei_thp.h"
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/ctype.h>

#define SYSFS_PROPERTY_PATH "afe_properties"
#define SYSFS_TOUCH_PATH "touchscreen"
#define SYSFS_PLAT_TOUCH_PATH "huawei_touch"
#define THP_CHARGER_BUF_LEN 32
#define THP_BASE_DECIMAL 10

u8 g_thp_log_cfg;
EXPORT_SYMBOL(g_thp_log_cfg);

static ssize_t thp_tui_wake_up_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	ret = strncmp(buf, "open", sizeof("open"));
	if (ret == 0) {
		cd->thp_ta_waitq_flag = WAITQ_WAKEUP;
		wake_up_interruptible(&(cd->thp_ta_waitq));
		THP_LOG_ERR("%s wake up thp_ta_flag\n", __func__);
	}
	return count;
}

static ssize_t thp_tui_wake_up_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t thp_hostprocessing_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "hostprocessing\n");
}

static ssize_t thp_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "status=0x%x\n",
		thp_get_status_all());
}

/* If not config ic_name in dts, it will be "unknown" */
static ssize_t thp_chip_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (cd->hide_product_info_en)
		return snprintf(buf, PAGE_SIZE, "%s\n", cd->project_id);

	return snprintf(buf, PAGE_SIZE, "%s-%s-%s\n",
		cd->thp_dev->ic_name,
		cd->project_id,
		cd->vendor_name ? cd->vendor_name : "unknown");
}

static ssize_t thp_loglevel_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	u8 new_level = g_thp_log_cfg ? 0 : 1;

	int len = snprintf(buf, PAGE_SIZE, "%d -> %d\n",
		g_thp_log_cfg, new_level);

	g_thp_log_cfg = new_level;

	return len;
}

#if defined(THP_CHARGER_FB)
static ssize_t thp_host_charger_state_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	THP_LOG_DEBUG("%s called\n", __func__);

	return snprintf(buf, THP_CHARGER_BUF_LEN, "%d\n",
			thp_get_status(THP_STATUS_CHARGER));
}
static ssize_t thp_host_charger_state_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	/*
	 * get value of charger status from first byte of buf
	 */
	unsigned int value = buf[0] - '0';

	THP_LOG_INFO("%s: input value is %d\n", __func__, value);

	thp_set_status(THP_STATUS_CHARGER, value);

	return count;
}
#endif

static ssize_t thp_roi_data_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	short *roi_data = cd->roi_data;

	memcpy(buf, roi_data, ROI_DATA_LENGTH * sizeof(short));

	return ROI_DATA_LENGTH * sizeof(short);
}

static const char *move_to_next_number(const char *str_in)
{
	const char *str = str_in;
	const char *next_num = NULL;

	str = skip_spaces(str);
	next_num = strchr(str, ' ');
	if (next_num)
		return next_num;

	return str_in;
}

static ssize_t thp_roi_data_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int i = 0;
	int num;
	struct thp_core_data *cd = thp_get_core_data();
	short *roi_data = cd->roi_data;
	const char *str = buf;

	while (i < ROI_DATA_LENGTH && *str) {
		if (sscanf(str, "%7d", &num) < 0)
			break;

		str = move_to_next_number(str);
		roi_data[i++] = (short)num;
	}

	return count;
}

static ssize_t thp_roi_data_debug_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int count = 0;
	int i;
	int ret;
	struct thp_core_data *cd = thp_get_core_data();
	short *roi_data = cd->roi_data;

	for (i = 0; i < ROI_DATA_LENGTH; ++i) {
		ret = snprintf(buf + count, PAGE_SIZE - count, "%4d ", roi_data[i]);
		if (ret > 0)
			count += ret;
		/* every 7 data is a row */
		if (!((i + 1) % 7)) {
			ret = snprintf(buf + count, PAGE_SIZE - count, "\n");
			if (ret > 0)
				count += ret;
		}
	}

	return count;
}
static ssize_t thp_roi_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	long status = 0;
	int ret;

	ret = kstrtoul(buf, THP_BASE_DECIMAL, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_ROI, !!status);
	THP_LOG_INFO("%s: set roi enable status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_roi_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
		thp_get_status(THP_STATUS_ROI));
}

static ssize_t thp_holster_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	long status = 0;
	int ret;

	ret = kstrtoul(buf, THP_BASE_DECIMAL, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_HOLSTER, !!status);
	THP_LOG_INFO("%s: set holster status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_holster_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
				thp_get_status(THP_STATUS_HOLSTER));
}

static ssize_t thp_glove_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	long status = 0;
	int ret;

	ret = kstrtoul(buf, THP_BASE_DECIMAL, &status);
	if (ret) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return ret;
	}

	thp_set_status(THP_STATUS_GLOVE, !!status);
	THP_LOG_INFO("%s: set glove status to %d\n", __func__, !!status);

	return count;
}

static ssize_t thp_glove_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
			thp_get_status(THP_STATUS_GLOVE));
}


static ssize_t thp_holster_window_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;
	int window_enable;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;

	ret = sscanf(buf, "%4d %4d %4d %4d %4d",
		&window_enable, &x0, &y0, &x1, &y1);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EBUSY;
	}
	cd->window.x0 = x0;
	cd->window.y0 = y0;
	cd->window.x1 = x1;
	cd->window.y1 = y1;

	thp_set_status(THP_STATUS_HOLSTER, !!window_enable);
	thp_set_status(THP_STATUS_WINDOW_UPDATE,
		!thp_get_status(THP_STATUS_WINDOW_UPDATE));
	THP_LOG_INFO("%s: update window %d %d %d %d %d\n",
		__func__, window_enable, x0, y0, x1, y1);

	return count;
}

static ssize_t thp_holster_window_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_window_info *window = &cd->window;

	return snprintf(buf, PAGE_SIZE - 1, "%d %d %d %d %d\n",
			thp_get_status(THP_STATUS_HOLSTER),
			window->x0, window->y0, window->x1, window->y1);
}

static ssize_t thp_touch_switch_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	int ret;
	int type = 0;
	int status = 0;
	int parameter = 0;

	ret = sscanf(buf, "%4d,%4d,%4d", &type, &status, &parameter);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EBUSY;
	}
	cd->scene_info.type = type;
	cd->scene_info.status = status;
	cd->scene_info.parameter = parameter;
	thp_set_status(THP_STATUS_TOUCH_SCENE,
		!thp_get_status(THP_STATUS_TOUCH_SCENE));
	THP_LOG_INFO("%s:touch scene update %d %d %d\n",
		__func__, type, status, parameter);

	return count;
}

static ssize_t thp_touch_switch_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	unsigned int value = 0;

	THP_LOG_INFO("%s:value = %d\n", __func__, value);

	/*
	 * Inherit from tskit
	 * which will be use for ApsService to decide doze is support or not
	 * This feature is not supported by default in THP
	 */
	return snprintf(buf, PAGE_SIZE - 1, "%d\n", value);
}

static ssize_t thp_udfp_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE - 1, "udfp status : %d\n",
		thp_get_status(THP_STATUS_UDFP));
}

static ssize_t thp_udfp_enable_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count)
{
	unsigned int value = 0;
	int ret;

	ret = sscanf(buf, "%d", &value);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EINVAL;
	}

	thp_set_status(THP_STATUS_UDFP, value);
	THP_LOG_INFO("%s: ud fp status: %d\n", __func__, !!value);

	return count;
}

static ssize_t stylus_wakeup_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	int stylus_mode = STYLUS_WAKEUP_DISABLE;

	if ((dev == NULL) || (attr == NULL) || (buf == NULL) ||
		(cd == NULL)) {
		THP_LOG_ERR("%s:input null ptr\n", __func__);
		return -EINVAL;
	}
	if (cd->pen_supported == 0) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE - 1, "%d\n", stylus_mode);
}

static ssize_t stylus_wakeup_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int parameter = 0;
	int ret;
	struct thp_core_data *cd = thp_get_core_data();

	if ((dev == NULL) || (attr == NULL) || (buf == NULL) ||
		(cd == NULL)) {
		THP_LOG_ERR("%s:input null ptr\n", __func__);
		return -EINVAL;
	}
	if (cd->pen_supported == 0) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	ret = sscanf(buf, "%1d", &parameter);
	if (ret <= 0)
		THP_LOG_ERR("%s:read node error\n", __func__);
	THP_LOG_INFO("%s:parameter:%d\n", __func__, parameter);
	switch (parameter) {
	case STYLUS_WAKEUP_TESTMODE_IN:
		THP_LOG_INFO("%s:enter test mode\n", __func__);
		cd->stylus_status = thp_get_status(THP_STATUS_STYLUS);
		/* set bit 1 to turn on the pen switch */
		thp_set_status(THP_STATUS_STYLUS, 1);
		break;
	case STYLUS_WAKEUP_TESTMODE_OUT:
		THP_LOG_INFO("%s:exit test mode, recover last status\n",
			__func__);
		thp_set_status(THP_STATUS_STYLUS, cd->stylus_status);
		break;
	case STYLUS_WAKEUP_DISABLE:
		/* set bit 0 to turn off the pen switch */
		thp_set_status(THP_STATUS_STYLUS, 0);
		cd->stylus_status = parameter;
		break;
	case STYLUS_WAKEUP_NORMAL_STATUS:
	case STYLUS_WAKEUP_LOW_FREQENCY:
		/* set bit 1 to turn on the pen switch */
		thp_set_status(THP_STATUS_STYLUS, 1);
		cd->stylus_status = parameter;
		break;
	default:
		THP_LOG_INFO("%s:invalid  input\n", __func__);
		break;
	}
	return count;
}

static ssize_t thp_supported_func_indicater_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE - 1, "%d\n",
		cd->supported_func_indicater);
}

static ssize_t thp_easy_wakeup_gesture_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_easy_wakeup_info *info = &cd->easy_wakeup_info;
	ssize_t ret;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s\n", __func__);

	if (!dev) {
		THP_LOG_ERR("dev is null\n");
		return -EINVAL;
	}

	ret = snprintf(buf, MAX_STR_LEN, "0x%04X\n",
		info->easy_wakeup_gesture);

	return ret;
}

static ssize_t thp_easy_wakeup_gesture_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct thp_core_data *cd = thp_get_core_data();
	struct thp_easy_wakeup_info *info = NULL;
	unsigned int value = 0;
	int ret;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("thp_easy_wakeup_gesture_store called\n");

	if (!dev) {
		THP_LOG_ERR("dev is null\n");
		return -EINVAL;
	}
	ret = sscanf(buf, "%5u", &value);
	if ((ret <= 0) || (value > TS_GESTURE_INVALID_COMMAND)) {
		THP_LOG_ERR("invalid parm\n");
		return -EBUSY;
	}
	info = &cd->easy_wakeup_info;
	info->easy_wakeup_gesture = value;
	cd->double_click_switch = (IS_APP_ENABLE_GESTURE(GESTURE_DOUBLE_CLICK) &
		info->easy_wakeup_gesture) ? DOUBLE_CLICK_ON : DOUBLE_CLICK_OFF;
	cd->single_click_switch = (IS_APP_ENABLE_GESTURE(GESTURE_SINGLE_CLICK) &
		info->easy_wakeup_gesture) ? SINGLE_CLICK_ON : SINGLE_CLICK_OFF;
	THP_LOG_INFO("easy_wakeup_gesture=0x%x\n", info->easy_wakeup_gesture);
	THP_LOG_INFO("double_click_switch:%u, single_click_switch %u\n",
		cd->double_click_switch, cd->single_click_switch);
	if (info->easy_wakeup_gesture == false) {
		cd->sleep_mode = TS_POWER_OFF_MODE;
		THP_LOG_INFO("poweroff mode\n");
	} else {
		cd->sleep_mode = TS_GESTURE_MODE;
		THP_LOG_INFO("gesture mode\n");
	}

	THP_LOG_INFO("ts gesture wakeup done\n");
	return size;
}
static ssize_t thp_wakeup_gesture_enable_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	unsigned long tmp = 0;
	struct thp_core_data *cd = thp_get_core_data();
	int error;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s called\n", __func__);

	error = sscanf(buf, "%4lu", &tmp);
	if (error <= 0) {
		THP_LOG_ERR("sscanf return invaild :%d\n", error);
		return -EINVAL;
	}
	THP_LOG_INFO("%s:%lu\n", __func__, tmp);
	return count;
}

static ssize_t thp_wakeup_gesture_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s called\n", __func__);
	return 0;
}

static ssize_t thp_easy_wakeup_control_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct thp_core_data *cd = thp_get_core_data();

	unsigned long value = 0;
	int ret;
	int error = NO_ERR;

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s called\n", __func__);

	ret = sscanf(buf, "%4lu", &value);
	if (ret <= 0 || value > TS_GESTURE_INVALID_CONTROL_NO) {
		THP_LOG_INFO("%s->invalid parm\n", __func__);
		return -EINVAL;
	}

	value = (u8) value & TS_GESTURE_COMMAND;
	if (value == 1) {
		if (cd->thp_dev->ops->chip_wrong_touch) {
			error = cd->thp_dev->ops->chip_wrong_touch(cd->thp_dev);
			if (error < 0)
				THP_LOG_INFO("chip_wrong_touch error\n");
		} else {
			THP_LOG_INFO("chip_wrong_touch not init\n");
		}
	}
	THP_LOG_INFO("%s done\n", __func__);
	return size;
}

static ssize_t thp_easy_wakeup_position_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	if (!cd->support_gesture_mode) {
		THP_LOG_ERR("%s not support\n", __func__);
		return -EINVAL;
	}
	return 0;
}

static ssize_t thp_oem_info_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int error = NO_ERR;
	struct thp_core_data *cd = thp_get_core_data();

	THP_LOG_INFO("%s: called\n", __func__);

	if ((dev == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s: dev is null\n", __func__);
		return -EINVAL;
	}
	if (cd->support_oem_info == THP_OEM_INFO_LCD_EFFECT_TYPE) {
		error = snprintf(buf, OEM_INFO_DATA_LENGTH, "%s",
			cd->oem_info_data);
		if (error < 0) {
			THP_LOG_INFO("%s:oem info data:%s\n",
				__func__, cd->oem_info_data);
			return error;
		}
		THP_LOG_INFO("%s: oem info :%s\n", __func__, buf);
	}

	THP_LOG_DEBUG("%s done\n", __func__);
	return error;
}


static ssize_t thp_fingerprint_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned int udfp_enable = 0;
	unsigned int udfp_switch = 0;
	int ret;

	if (!cd->support_fingerprint_switch) {
		THP_LOG_DEBUG("%s:no need to report fingerprint switch\n",
			__func__);
		return count;
	}
	/*
	 * 0,0:the fingerprint doesn't be enrolled
	 * 1,1:begin enroll or authenticate
	 * 1,0:end enroll or authenticate
	 */
	ret = sscanf(buf, "%4u,%4u", &udfp_enable, &udfp_switch);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EINVAL;
	}

	cd->udfp_status.udfp_enable = udfp_enable;
	cd->udfp_status.udfp_switch = udfp_switch;

	thp_set_status(THP_STATUS_UDFP_SWITCH, !!udfp_switch);
	THP_LOG_INFO("%s: update udfp_status:%u %u\n",
		__func__, udfp_enable, udfp_switch);

	return count;
}

static ssize_t thp_fingerprint_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	return snprintf(buf, PAGE_SIZE - 1, "%u,%u\n",
		cd->udfp_status.udfp_enable, cd->udfp_status.udfp_switch);
}

static ssize_t thp_ring_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned int ring_status = 0;
	unsigned int volume_side;
	int ret;

	if ((dev == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s: dev or cd is null\n", __func__);
		return -EINVAL;
	}

	volume_side = cd->volume_side_status;
	ret = sscanf(buf, "%4u", &ring_status);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s:ring_status:%u\n", __func__, ring_status);
	if (!cd->support_ring_feature) {
		THP_LOG_ERR("%s: not support ring\n", __func__);
		return -EINVAL;
	}
	cd->ring_switch = RING_SWITCH_OFF;
	cd->phone_status = PNONE_STATUS_OFF;
	/*
	 * ring_status = 0:disbale report ring event
	 * ring_status = 1:enable report ring event
	 * ring_status = 2:on the phone
	 */
	switch (ring_status) {
	case RING_STAUS_ENABLE:
		cd->ring_switch = RING_SWITCH_ON;
		break;
	case RING_PHONE_STATUS:
		cd->ring_switch = RING_SWITCH_ON;
		cd->phone_status = PNONE_STATUS_ON;
		break;
	case VOLUME_SIDE_LEFT:
		cd->volume_side_status = VOLUME_SIDE_STATUS_LEFT;
		break;
	case VOLUME_SIDE_RIGHT:
		cd->volume_side_status = VOLUME_SIDE_STATUS_RIGHT;
		break;
	case VOLUME_SIDE_BOTH:
		cd->volume_side_status = VOLUME_SIDE_STATUS_BOTH;
		break;
	default:
		break;
	}

	if (cd->volume_side_status != volume_side)
		thp_set_status(THP_STATUS_VOLUME_SIDE,
			!thp_get_status(THP_STATUS_VOLUME_SIDE));

#ifdef CONFIG_HUAWEI_SHB_THP
	if ((cd->suspended || cd->support_ring_feature) &&
		(ring_status <= RING_PHONE_STATUS)) {
		ret = send_thp_driver_status_cmd(ring_status,
			0, ST_CMD_TYPE_SET_AUDIO_STATUS);
		THP_LOG_INFO("%s: %d\n", __func__, ret);
	}
#endif
	return count;
}

static ssize_t thp_ring_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	if ((dev == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s: dev or cd is null\n", __func__);
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE - 1, "%u\n", cd->ring_switch);
}

static ssize_t thp_power_switch_ctrl_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct thp_core_data *cd = thp_get_core_data();
	unsigned int power_status = 0;
	int ret;

	if ((dev == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s: dev or cd is null\n", __func__);
		return -EINVAL;
	}

	ret = sscanf(buf, "%u", &power_status);
	if (ret <= 0) {
		THP_LOG_ERR("%s: illegal input\n", __func__);
		return -EINVAL;
	}
	THP_LOG_INFO("%s:power_status:0x%x\n", __func__, power_status);
	switch (power_status) {
	case WAKE_REASON_POWER_BUTTON:
		cd->power_switch = POWER_KEY_ON_CTRL;
		break;
	case WAKE_REASON_FINGER_PRINT:
	case WAKE_REASON_PICKUP:
		cd->power_switch = FINGERPRINT_ON_CTRL;
		break;
	case WAKE_REASON_PROXIMITY:
		cd->power_switch = PROXIMITY_ON_CTRL;
		break;
	case WAKE_REASON_UNKNOWN:
	case WAKE_REASON_APPLICATION:
	case WAKE_REASON_PLUGGED_IN:
	case WAKE_REASON_GESTURE:
	case WAKE_REASON_CAMERA_LAUNCH:
	case WAKE_REASON_WAKE_KEY:
	case WAKE_REASON_WAKE_MOTION:
	case WAKE_REASON_HDMI:
	case WAKE_REASON_LID:
		cd->power_switch = AUTO_OR_OTHER_ON_CTRL;
		break;
	case GO_TO_SLEEP_REASON_POWER_BUTTON:
		cd->power_switch = POWER_KEY_OFF_CTRL;
		break;
	case GO_TO_SLEEP_REASON_PROX_SENSOR:
		cd->power_switch = PROXIMITY_OFF_CTRL;
		break;
	case GO_TO_SLEEP_REASON_TIMEOUT:
		cd->power_switch = POWERON_TIMEOUT_OFF_CTRL;
		break;
	case GO_TO_SLEEP_REASON_APPLICATION:
	case GO_TO_SLEEP_REASON_DEVICE_ADMIN:
	case GO_TO_SLEEP_REASON_LID_SWITCH:
	case GO_TO_SLEEP_REASON_HDMI:
	case GO_TO_SLEEP_REASON_ACCESSIBILITY:
	case GO_TO_SLEEP_REASON_FORCE_SUSPEND:
	case GO_TO_SLEEP_REASON_WAIT_BRIGHTNESS_TIMEOUT:
	case GO_TO_SLEEP_REASON_PHONE_CALL:
		cd->power_switch = AUTO_OR_OTHER_OFF_CTRL;
		break;
	default:
		cd->power_switch = POWER_STATUS_NULL;
		return -EINVAL;
	}
	THP_LOG_INFO("%s :power_switch = %u\n", __func__, cd->power_switch);
	thp_set_status(THP_STATUS_POWER_SWITCH_CTRL,
		!thp_get_status(THP_STATUS_POWER_SWITCH_CTRL));
	return count;
}

static ssize_t thp_power_switch_ctrl_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct thp_core_data *cd = thp_get_core_data();

	if ((dev == NULL) || (cd == NULL)) {
		THP_LOG_ERR("%s: dev or cd is null\n", __func__);
		return -EINVAL;
	}
	return snprintf(buf, PAGE_SIZE - 1, "%u\n", cd->power_switch);
}

static DEVICE_ATTR(thp_status, S_IRUGO, thp_status_show, NULL);
static DEVICE_ATTR(touch_chip_info, (S_IRUSR | S_IWUSR | S_IRGRP),
			thp_chip_info_show, NULL);
static DEVICE_ATTR(hostprocessing, S_IRUGO, thp_hostprocessing_show, NULL);
static DEVICE_ATTR(loglevel, S_IRUGO, thp_loglevel_show, NULL);
static DEVICE_ATTR(charger_state, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
		thp_host_charger_state_show, thp_host_charger_state_store);
static DEVICE_ATTR(roi_data, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			 thp_roi_data_show, thp_roi_data_store);
static DEVICE_ATTR(roi_data_internal, S_IRUGO, thp_roi_data_debug_show, NULL);
static DEVICE_ATTR(roi_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_roi_enable_show, thp_roi_enable_store);
static DEVICE_ATTR(touch_sensitivity, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_holster_enable_show, thp_holster_enable_store);
static DEVICE_ATTR(touch_glove, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_glove_enable_show, thp_glove_enable_store);
static DEVICE_ATTR(touch_window, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_holster_window_show, thp_holster_window_store);
static DEVICE_ATTR(touch_switch, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_touch_switch_show, thp_touch_switch_store);
static DEVICE_ATTR(udfp_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
			thp_udfp_enable_show, thp_udfp_enable_store);
static DEVICE_ATTR(stylus_wakeup_ctrl, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
		   stylus_wakeup_ctrl_show, stylus_wakeup_ctrl_store);
static DEVICE_ATTR(supported_func_indicater, (S_IRUSR),
			thp_supported_func_indicater_show, NULL);
static DEVICE_ATTR(tui_wake_up_enable, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
		thp_tui_wake_up_enable_show, thp_tui_wake_up_enable_store);
static DEVICE_ATTR(easy_wakeup_gesture, (S_IRUSR | S_IWUSR),
		thp_easy_wakeup_gesture_show, thp_easy_wakeup_gesture_store);
static DEVICE_ATTR(wakeup_gesture_enable, (S_IRUSR | S_IWUSR),
			thp_wakeup_gesture_enable_show,
			thp_wakeup_gesture_enable_store);
static DEVICE_ATTR(easy_wakeup_control, S_IWUSR, NULL,
			thp_easy_wakeup_control_store);
static DEVICE_ATTR(easy_wakeup_position, S_IRUSR, thp_easy_wakeup_position_show,
			NULL);
static DEVICE_ATTR(touch_oem_info, (S_IRUSR | S_IWUSR | S_IRGRP),
			thp_oem_info_show, NULL);
static DEVICE_ATTR(fingerprint_switch_ctrl, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
	thp_fingerprint_switch_ctrl_show, thp_fingerprint_switch_ctrl_store);
static DEVICE_ATTR(ring_switch_ctrl, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
	thp_ring_switch_ctrl_show, thp_ring_switch_ctrl_store);
static DEVICE_ATTR(power_switch_ctrl, (S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP),
	thp_power_switch_ctrl_show, thp_power_switch_ctrl_store);

static struct attribute *thp_ts_attributes[] = {
	&dev_attr_thp_status.attr,
	&dev_attr_touch_chip_info.attr,
	&dev_attr_hostprocessing.attr,
	&dev_attr_loglevel.attr,
#if defined(THP_CHARGER_FB)
	&dev_attr_charger_state.attr,
#endif
	&dev_attr_roi_data.attr,
	&dev_attr_roi_data_internal.attr,
	&dev_attr_roi_enable.attr,
	&dev_attr_touch_sensitivity.attr,
	&dev_attr_touch_glove.attr,
	&dev_attr_touch_window.attr,
	&dev_attr_touch_switch.attr,
	&dev_attr_udfp_enable.attr,
	&dev_attr_supported_func_indicater.attr,
	&dev_attr_tui_wake_up_enable.attr,
	&dev_attr_easy_wakeup_gesture.attr,
	&dev_attr_wakeup_gesture_enable.attr,
	&dev_attr_easy_wakeup_control.attr,
	&dev_attr_easy_wakeup_position.attr,
	&dev_attr_touch_oem_info.attr,
	&dev_attr_stylus_wakeup_ctrl.attr,
	&dev_attr_fingerprint_switch_ctrl.attr,
	&dev_attr_ring_switch_ctrl.attr,
	&dev_attr_power_switch_ctrl.attr,
	NULL,
};

static const struct attribute_group thp_ts_attr_group = {
	.attrs = thp_ts_attributes,
};


int thp_init_sysfs(struct thp_core_data *cd)
{
	int rc;

	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return -EINVAL;
	}

	cd->thp_platform_dev = platform_device_alloc("huawei_thp", -1);
	if (!cd->thp_platform_dev) {
		THP_LOG_ERR("%s: regist platform_device failed\n", __func__);
		return -ENODEV;
	}

	rc = platform_device_add(cd->thp_platform_dev);
	if (rc) {
		THP_LOG_ERR("%s: add platform_device failed\n", __func__);
		platform_device_unregister(cd->thp_platform_dev);
		return -ENODEV;
	}

	rc = sysfs_create_group(&cd->thp_platform_dev->dev.kobj,
			&thp_ts_attr_group);
	if (rc) {
		THP_LOG_ERR("%s:can't create ts's sysfs\n", __func__);
		goto err_create_group;
	}

	rc = sysfs_create_link(NULL, &cd->thp_platform_dev->dev.kobj,
			SYSFS_TOUCH_PATH);
	if (rc) {
		THP_LOG_ERR("%s: fail create link error = %d\n", __func__, rc);
		goto err_create_link;
	}

	return 0;

err_create_link:
	sysfs_remove_group(&cd->thp_platform_dev->dev.kobj, &thp_ts_attr_group);
err_create_group:
	platform_device_put(cd->thp_platform_dev);
	platform_device_unregister(cd->thp_platform_dev);
	return rc;
}


void thp_sysfs_release(struct thp_core_data *cd)
{
	if (!cd) {
		THP_LOG_ERR("%s: core data null\n", __func__);
		return;
	}
	platform_device_put(cd->thp_platform_dev);
	platform_device_unregister(cd->thp_platform_dev);
	sysfs_remove_group(&cd->sdev->dev.kobj, &thp_ts_attr_group);
}


