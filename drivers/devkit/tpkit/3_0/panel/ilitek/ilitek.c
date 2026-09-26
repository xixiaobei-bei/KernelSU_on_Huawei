/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include "ilitek_fw.h"
#include "ilitek.h"
#if defined (CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
#endif

#define GESTURE_EVENT_RETRY_TIME 30
#define WAIT_FOR_SPI_BUS_READ_DELAY 5
#define FINGER_NUM_ZERO 0
/* Debug level */
bool debug_en = DEBUG_OUTPUT;
EXPORT_SYMBOL(debug_en);

static struct workqueue_struct *esd_wq;
static struct workqueue_struct *bat_wq;
static struct delayed_work esd_work;
static struct delayed_work bat_work;
extern char ts_test_failed_reason[ILITEK_TEST_FAILED_REASON_LEN];
void ili_irq_disable(void)
{
	return;
}

void ili_irq_enable(void)
{
	return;
}

#if RESUME_BY_DDI
static struct workqueue_struct *resume_by_ddi_wq;
static struct work_struct resume_by_ddi_work;


static void ilitek_resume_by_ddi_work(struct work_struct *work)
{
	mutex_lock(&ilits->touch_mutex);

	if (ilits->gesture)
		disable_irq_wake(ilits->irq_num);

	/* Set tp as demo mode and reload code if it's iram. */
	ilits->actual_tp_mode = P5_X_FW_AP_MODE;
	if (ilits->fw_upgrade_mode == UPGRADE_IRAM)
		ili_fw_upgrade_handler(NULL);
	else
		ili_reset_ctrl(ilits->reset);

	ili_irq_enable();
	TS_LOG_INFO("TP resume end by wq\n");
	ili_wq_ctrl(WQ_ESD, ENABLE);
	ili_wq_ctrl(WQ_BAT, ENABLE);
	ilits->tp_suspend = false;
	mutex_unlock(&ilits->touch_mutex);
}

void ili_resume_by_ddi(void)
{
	if (!resume_by_ddi_wq) {
		TS_LOG_INFO("resume_by_ddi_wq is null\n");
		return;
	}

	mutex_lock(&ilits->touch_mutex);

	TS_LOG_INFO("TP resume start called by ddi\n");

	/*
	 * To match the timing of sleep out, the first of mipi cmd must be sent within 10ms
	 * after TP reset. We then create a wq doing host download before resume.
	 */
	atomic_set(&ilits->fw_stat, ENABLE);
	ili_reset_ctrl(ilits->reset);
	ili_ice_mode_ctrl(ENABLE, OFF);
	ilits->ddi_rest_done = true;
	mdelay(5);
	queue_work(resume_by_ddi_wq, &(resume_by_ddi_work));

	mutex_unlock(&ilits->touch_mutex);
}
#endif

int ili_mp_test_handler(struct ts_rawdata_info *raw_info,u8 *apk, bool lcm_on)
{
	int ret = 0;

	if (!raw_info || !apk) {
		TS_LOG_ERR("raw_info or apk is null\n");
		return -EINVAL;
	}
	if (atomic_read(&ilits->fw_stat)) {
		TS_LOG_ERR("fw upgrade processing, ignore\n");
		return -EMP_FW_PROC;
	}

	atomic_set(&ilits->mp_stat, ENABLE);

	if (ilits->actual_tp_mode != P5_X_FW_TEST_MODE) {
		ret = ili_switch_tp_mode(P5_X_FW_TEST_MODE);
		if (ret < 0) {
			TS_LOG_ERR("Switch MP mode failed\n");
			ret = -EMP_MODE;
			strncpy(ts_test_failed_reason,
				"tp_initial_failed-", ILITEK_TEST_FAILED_REASON_LEN - 1);
			ilitek_result_init(raw_info,apk);
			goto out;
		}
	}

	ret = ili_mp_test_main(raw_info,apk, lcm_on);

out:
	/*
	 * If there's running mp test with lcm off, we suspose that
	 * users will soon call resume from suspend. TP mode will be changed
	 * from MP to AP mode until resume finished.
	 */
	if (!lcm_on) {
		atomic_set(&ilits->mp_stat, DISABLE);
		return ret;
	}

	ilits->actual_tp_mode = P5_X_FW_AP_MODE;
	if (ilits->fw_upgrade_mode == UPGRADE_IRAM) {
		if (ili_fw_upgrade_handler(NULL) < 0)
			TS_LOG_ERR("FW upgrade failed during mp test\n");
	} else {
		if (ili_reset_ctrl(ilits->reset) < 0)
			TS_LOG_ERR("TP Reset failed during mp test\n");
	}

	atomic_set(&ilits->mp_stat, DISABLE);
	return ret;
}

int ili_switch_tp_mode(u8 mode)
{
	int ret = 0;
	bool ges_dbg = false;

	atomic_set(&ilits->tp_sw_mode, START);

	ilits->actual_tp_mode = mode;

	/* able to see cdc data in gesture mode */
	if (ilits->tp_data_format == DATA_FORMAT_DEBUG &&
		ilits->actual_tp_mode == P5_X_FW_GESTURE_MODE)
		ges_dbg = true;

	switch (ilits->actual_tp_mode) {
	case P5_X_FW_AP_MODE:
		TS_LOG_INFO("Switch to AP mode\n");
		ilits->wait_int_timeout = AP_INT_TIMEOUT;
		if (ilits->fw_upgrade_mode == UPGRADE_IRAM) {
			if (ili_fw_upgrade_handler(NULL) < 0)
				TS_LOG_ERR("FW upgrade failed\n");
		} else {
			ret = ili_reset_ctrl(ilits->reset);
		}
		if (ret < 0)
			TS_LOG_ERR("TP Reset failed\n");

		break;
	case P5_X_FW_GESTURE_MODE:
		TS_LOG_INFO("Switch to Gesture mode\n");
		ilits->wait_int_timeout = AP_INT_TIMEOUT;
		ret = ilits->gesture_move_code(ilits->gesture_mode);
		if (ret < 0)
			TS_LOG_ERR("Move gesture code failed\n");
		if (ges_dbg) {
			TS_LOG_INFO("Enable gesture debug func\n");
			ili_set_tp_data_len(DATA_FORMAT_GESTURE_DEBUG, false, NULL);
		}
		break;
	case P5_X_FW_TEST_MODE:
		TS_LOG_INFO("Switch to Test mode\n");
		ilits->wait_int_timeout = MP_INT_TIMEOUT;
		ret = ilits->mp_move_code();
		break;
	default:
		TS_LOG_ERR("Unknown TP mode: %x\n", mode);
		ret = -1;
		break;
	}

	if (ret < 0)
		TS_LOG_ERR("Switch TP mode %d failed \n", mode);

	TS_LOG_DEBUG("Actual TP mode = %d\n", ilits->actual_tp_mode);
	atomic_set(&ilits->tp_sw_mode, END);
	return ret;
}

int ili_gesture_recovery(void)
{
	int ret = 0;
	bool lock = mutex_is_locked(&ilits->touch_mutex);

	atomic_set(&ilits->esd_stat, START);

	if (!lock)
		mutex_lock(&ilits->touch_mutex);

	TS_LOG_INFO("Doing gesture recovery\n");
	ret = ilits->ges_recover();

	if (!lock)
		mutex_unlock(&ilits->touch_mutex);

	atomic_set(&ilits->esd_stat, END);
	return ret;
}

void ili_spi_recovery(void)
{
	atomic_set(&ilits->esd_stat, START);

	TS_LOG_INFO("Doing spi recovery\n");
	if (ili_fw_upgrade_handler(NULL) < 0)
		TS_LOG_ERR("FW upgrade failed\n");

	atomic_set(&ilits->esd_stat, END);
}

int ili_wq_esd_spi_check(void)
{
	int ret;
	u8 tx;
	u8 rx;

	tx = SPI_WRITE;
	rx = 0;
	ret = ilits->spi_write_then_read(ilits->spi, &tx, 1, &rx, 1);
	TS_LOG_DEBUG("spi esd check = 0x%x\n", ret);
	if (ret == DO_SPI_RECOVER) {
		TS_LOG_ERR("ret = 0x%x\n", ret);
		return -1;
	}
	return 0;
}

int ili_wq_esd_i2c_check(void)
{
	TS_LOG_DEBUG("");
	return 0;
}

static void ilitek_tddi_wq_esd_check(struct work_struct *work)
{
	mutex_lock(&ilits->touch_mutex);
	if (ilits->esd_recover() < 0) {
#if defined (CONFIG_HUAWEI_DSM)
		ts_dmd_report(DSM_TP_ESD_ERROR_NO, "chip esd reset fail.\n");
#endif
		TS_LOG_ERR("SPI ACK failed, doing spi recovery\n");
		ili_spi_recovery();
	}
	mutex_unlock(&ilits->touch_mutex);
	complete_all(&ilits->esd_done);
	ili_wq_ctrl(WQ_ESD, ENABLE);
}

static int read_power_status(u8 *buf, int length)
{
	struct file *f = NULL;
	mm_segment_t old_fs;
	ssize_t byte = 0;

	old_fs = get_fs();
	set_fs(get_ds());

	f = filp_open(POWER_STATUS_PATH, O_RDONLY, 0);
	if (err_alloc_mem(f)) {
		TS_LOG_ERR("Failed to open %s\n", POWER_STATUS_PATH);
		set_fs(old_fs);
		return -1;
	}

	f->f_op->llseek(f, 0, SEEK_SET);
	byte = f->f_op->read(f, buf, 20, &f->f_pos);

	TS_LOG_DEBUG("Read %d bytes\n", (int)byte);

	set_fs(old_fs);
	filp_close(f, NULL);
	return 0;
}

static void ilitek_tddi_wq_bat_check(struct work_struct *work)
{
	u8 str[20] = {0};
	static int charge_mode;

	if (read_power_status(str, sizeof(str)/str[0]) < 0)
		TS_LOG_ERR("Read power status failed\n");

	TS_LOG_DEBUG("Batter Status: %s\n", str);

	if (strstr(str, "Charging") != NULL || strstr(str, "Full") != NULL
		|| strstr(str, "Fully charged") != NULL) {
		if (charge_mode != 1) {
			TS_LOG_DEBUG("Charging mode\n");
			if (ili_ic_func_ctrl("plug", DISABLE) < 0) // plug in
				TS_LOG_ERR("Write plug in failed\n");
			charge_mode = 1;
		}
	} else {
		if (charge_mode != 2) {
			TS_LOG_DEBUG("Not charging mode\n");
			if (ili_ic_func_ctrl("plug", ENABLE) < 0) // plug out
				TS_LOG_ERR("Write plug out failed\n");
			charge_mode = 2;
		}
	}
	ili_wq_ctrl(WQ_BAT, ENABLE);
}

void ili_wq_ctrl(int type, int ctrl)
{
	switch (type) {
	case WQ_ESD:
		if (ENABLE_WQ_ESD || ilits->wq_ctrl) {
			if (!esd_wq) {
				TS_LOG_ERR("wq esd is null\n");
				break;
			}
			ilits->wq_esd_ctrl = ctrl;
			if (ctrl == ENABLE) {
				TS_LOG_DEBUG("execute esd check\n");
				if (!queue_delayed_work(esd_wq, &esd_work, msecs_to_jiffies(WQ_ESD_DELAY)))
					TS_LOG_DEBUG("esd check was already on queue\n");
			} else {
				cancel_delayed_work_sync(&esd_work);
				flush_workqueue(esd_wq);
				TS_LOG_DEBUG("cancel esd wq\n");
			}
		}
		break;
	case WQ_BAT:
		if (ENABLE_WQ_BAT || ilits->wq_ctrl) {
			if (!bat_wq) {
				TS_LOG_ERR("WQ BAT is null\n");
				break;
			}
			ilits->wq_bat_ctrl = ctrl;
			if (ctrl == ENABLE) {
				TS_LOG_DEBUG("execute bat check\n");
				if (!queue_delayed_work(bat_wq, &bat_work, msecs_to_jiffies(WQ_BAT_DELAY)))
					TS_LOG_DEBUG("bat check was already on queue\n");
			} else {
				cancel_delayed_work_sync(&bat_work);
				flush_workqueue(bat_wq);
				TS_LOG_DEBUG("cancel bat wq\n");
			}
		}
		break;
	default:
		TS_LOG_ERR("Unknown WQ type, %d\n", type);
		break;
	}
}

void ilitek_tddi_wq_init(void)
{
	esd_wq = alloc_workqueue("esd_check", WQ_MEM_RECLAIM, 0);
	bat_wq = alloc_workqueue("bat_check", WQ_MEM_RECLAIM, 0);

	WARN_ON(!esd_wq);
	WARN_ON(!bat_wq);

	INIT_DELAYED_WORK(&esd_work,
		ilitek_tddi_wq_esd_check);
	INIT_DELAYED_WORK(&bat_work,
		ilitek_tddi_wq_bat_check);

#if RESUME_BY_DDI
	resume_by_ddi_wq = create_singlethread_workqueue("resume_by_ddi_wq");
	WARN_ON(!resume_by_ddi_wq);
	INIT_WORK(&resume_by_ddi_work, ilitek_resume_by_ddi_work);
#endif
}

int ili_sleep_handler(int mode)
{
	int ret = 0;
	bool sense_stop = true;

	mutex_lock(&ilits->touch_mutex);
	atomic_set(&ilits->tp_sleep, START);

	if (atomic_read(&ilits->fw_stat) ||
		atomic_read(&ilits->mp_stat)) {
		TS_LOG_INFO("fw upgrade or mp still running, ignore sleep requst\n");
		atomic_set(&ilits->tp_sleep, END);
		mutex_unlock(&ilits->touch_mutex);
		return 0;
	}

	ili_wq_ctrl(WQ_ESD, DISABLE);
	ili_wq_ctrl(WQ_BAT, DISABLE);
	ili_irq_disable();

	TS_LOG_INFO("Sleep Mode = %d\n", mode);

	if (ilits->ss_ctrl)
		sense_stop = true;
	else if ((ilits->chip->core_ver >= CORE_VER_1430))
		sense_stop = false;
	else
		sense_stop = true;

	switch (mode) {
	case TP_SUSPEND:
		TS_LOG_INFO("TP suspend start\n");
		if (sense_stop) {
			if (ili_ic_func_ctrl("sense", DISABLE) < 0)
				TS_LOG_ERR("Write sense stop cmd failed\n");

			if (ili_ic_check_busy(50, 20) < 0)
				TS_LOG_ERR("Check busy timeout during suspend\n");
		}

		if (ilits->gesture) {
			ili_switch_tp_mode(P5_X_FW_GESTURE_MODE);
			enable_irq_wake(ilits->irq_num);
			ili_irq_enable();
		} else {
			if (ili_ic_func_ctrl("sleep", DEEP_SLEEP_IN) < 0)
				TS_LOG_ERR("Write sleep in cmd failed\n");
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
			if (ilits->vcc_always_high_timing)
				gpio_set_value(ilits->tp_rst, 1);
			else
				gpio_set_value(ilits->tp_rst, 0);
#endif
		}
		TS_LOG_INFO("TP suspend end\n");
		ilits->tp_suspend = true;
		break;
	case TP_DEEP_SLEEP:
		TS_LOG_INFO("TP deep suspend start\n");
		if (sense_stop) {
			if (ili_ic_func_ctrl("sense", DISABLE) < 0)
				TS_LOG_ERR("Write sense stop cmd failed\n");

			if (ili_ic_check_busy(50, 20) < 0)
				TS_LOG_ERR("Check busy timeout during deep suspend\n");
		}

		if (ilits->gesture) {
			ili_switch_tp_mode(P5_X_FW_GESTURE_MODE);
			enable_irq_wake(ilits->irq_num);
			ili_irq_enable();
		} else {
			if (ili_ic_func_ctrl("sleep", DEEP_SLEEP_IN) < 0)
				TS_LOG_ERR("Write deep sleep in cmd failed\n");
#ifndef CONFIG_HUAWEI_DEVKIT_MTK_3_0
			if (ilits->vcc_always_high_timing)
				gpio_set_value(ilits->tp_rst, 1);
			else
				gpio_set_value(ilits->tp_rst, 0);
#endif
		}
		TS_LOG_INFO("TP deep suspend end\n");
		ilits->tp_suspend = true;
		break;
	case TP_RESUME:
#if !RESUME_BY_DDI
		TS_LOG_INFO("TP resume start\n");

		if (ilits->gesture)
			disable_irq_wake(ilits->irq_num);

		/* Set tp as demo mode and reload code if it's iram. */
		ilits->actual_tp_mode = P5_X_FW_AP_MODE;
		if (ilits->fw_upgrade_mode == UPGRADE_IRAM) {
			ret = ilitek_tddi_upgrade_get_fw_name();
			if (ret)
				TS_LOG_ERR("get firmware name failed");
			if (ili_fw_upgrade_handler(NULL) < 0)
				TS_LOG_ERR("FW upgrade failed during resume\n");
		} else {
			if (ili_reset_ctrl(ilits->reset) < 0)
				TS_LOG_ERR("TP Reset failed during resume\n");
		}
		ili_wq_ctrl(WQ_ESD, ENABLE);
		ili_wq_ctrl(WQ_BAT, ENABLE);
		ilits->tp_suspend = false;
		TS_LOG_INFO("TP resume end\n");
#endif
		ili_irq_enable();
		break;
	default:
		TS_LOG_ERR("Unknown sleep mode, %d\n", mode);
		ret = -EINVAL;
		break;
	}

	ili_touch_release_all_point();
	atomic_set(&ilits->tp_sleep, END);
	mutex_unlock(&ilits->touch_mutex);
	return ret;
}

int ili_fw_upgrade_handler(void *data)
{
	int ret = 0;

	atomic_set(&ilits->fw_stat, START);

	ilits->fw_update_stat = FW_STAT_INIT;
	ret = ili_fw_upgrade(ilits->fw_open);
	if (ret != 0) {
		TS_LOG_INFO("FW upgrade fail\n");
		ilits->fw_update_stat = FW_UPDATE_FAIL;
	} else {
		TS_LOG_INFO("FW upgrade pass\n");
		ilits->fw_update_stat = FW_UPDATE_PASS;
	}
	atomic_set(&ilits->fw_stat, END);
	return ret;
}

int ili_set_tp_data_len(int format, bool send, u8* data)
{
	u8 cmd[10] = {0}, ctrl = 0, debug_ctrl = 0;
	u16 self_key = 2;
	int ret = 0, tp_mode = ilits->actual_tp_mode, len = 0;

	switch (format) {
	case DATA_FORMAT_DEMO:
	case DATA_FORMAT_GESTURE_DEMO:
		len = P5_X_DEMO_MODE_PACKET_LEN;
		ctrl = DATA_FORMAT_DEMO_CMD;
		break;
	case DATA_FORMAT_DEBUG:
	case DATA_FORMAT_GESTURE_DEBUG:
		len = (2 * ilits->xch_num * ilits->ych_num) + (ilits->stx * 2) + (ilits->srx * 2);
		len += 2 * self_key + (8 * 2) + 1 + 35;
		ctrl = DATA_FORMAT_DEBUG_CMD;
		break;
	case DATA_FORMAT_DEMO_DEBUG_INFO:
		/*only suport SPI interface now, so defult use size 1024 buffer*/
		len = P5_X_DEMO_MODE_PACKET_LEN +
			P5_X_DEMO_DEBUG_INFO_ID0_LENGTH + P5_X_INFO_HEADER_LENGTH;
		ctrl = DATA_FORMAT_DEMO_DEBUG_INFO_CMD;
		break;
	case DATA_FORMAT_GESTURE_INFO:
		len = P5_X_GESTURE_INFO_LENGTH;
		ctrl = DATA_FORMAT_GESTURE_INFO_CMD;
		break;
	case DATA_FORMAT_GESTURE_NORMAL:
		len = P5_X_GESTURE_NORMAL_LENGTH;
		ctrl = DATA_FORMAT_GESTURE_NORMAL_CMD;
		break;
	case DATA_FORMAT_GESTURE_SPECIAL_DEMO:
		if (ilits->gesture_demo_ctrl == ENABLE) {
			if (ilits->gesture_mode == DATA_FORMAT_GESTURE_INFO)
				len = P5_X_GESTURE_INFO_LENGTH + P5_X_INFO_HEADER_LENGTH + P5_X_INFO_CHECKSUM_LENGTH;
			else
				len = P5_X_DEMO_MODE_PACKET_LEN + P5_X_INFO_HEADER_LENGTH + P5_X_INFO_CHECKSUM_LENGTH;
		} else {
			if (ilits->gesture_mode == DATA_FORMAT_GESTURE_INFO)
				len = P5_X_GESTURE_INFO_LENGTH;
			else
				len = P5_X_GESTURE_NORMAL_LENGTH;
		}
		TS_LOG_INFO("Gesture demo mode control = %d\n",  ilits->gesture_demo_ctrl);
		ili_ic_func_ctrl("gesture_demo_en", ilits->gesture_demo_ctrl);
		TS_LOG_INFO("knock_en setting\n");
		ili_ic_func_ctrl("knock_en", 0x8);
		break;
	case DATA_FORMAT_DEBUG_LITE_ROI:
		debug_ctrl = DATA_FORMAT_DEBUG_LITE_ROI_CMD;
		ctrl = DATA_FORMAT_DEBUG_LITE_CMD;
		break;
	case DATA_FORMAT_DEBUG_LITE_WINDOW:
		debug_ctrl = DATA_FORMAT_DEBUG_LITE_WINDOW_CMD;
		ctrl = DATA_FORMAT_DEBUG_LITE_CMD;
		break;
	case DATA_FORMAT_DEBUG_LITE_AREA:
		if(!data) {
			TS_LOG_ERR("DEBUG_LITE_AREA error cmd or data\n");
			return -1;
		}
		debug_ctrl = DATA_FORMAT_DEBUG_LITE_AREA_CMD;
		ctrl = DATA_FORMAT_DEBUG_LITE_CMD;
		cmd[3] = data[0];
		cmd[4] = data[1];
		cmd[5] = data[2];
		cmd[6] = data[3];
		break;
	default:
		TS_LOG_ERR("Unknow TP data format\n");
		return -1;
	}

	if (ctrl == DATA_FORMAT_DEBUG_LITE_CMD) {
		len = P5_X_DEBUG_LITE_LENGTH;
		cmd[0] = P5_X_MODE_CONTROL;
		cmd[1] = ctrl;
		cmd[2] = debug_ctrl;
		ret = ilits->wrapper(cmd, 10, NULL, 0, ON, OFF);

		if (ret < 0) {
			TS_LOG_ERR("switch to format %d failed\n", format);
			ili_switch_tp_mode(P5_X_FW_AP_MODE);
		}

	} else if (tp_mode == P5_X_FW_AP_MODE ||
		format == DATA_FORMAT_GESTURE_DEMO ||
		format == DATA_FORMAT_GESTURE_DEBUG) {
		cmd[0] = P5_X_MODE_CONTROL;
		cmd[1] = ctrl;
		ret = ilits->wrapper(cmd, 2, NULL, 0, ON, OFF);

		if (ret < 0) {
			TS_LOG_ERR("switch to format %d failed\n", format);
			ili_switch_tp_mode(P5_X_FW_AP_MODE);
		}
	} else if (tp_mode == P5_X_FW_GESTURE_MODE) {

		/*set gesture symbol*/
		ili_set_gesture_symbol();

		if (send) {
			ret = ili_ic_func_ctrl("lpwg", ctrl);
			if (ret < 0)
				TS_LOG_ERR("write gesture mode failed\n");
		}
	}

	ilits->tp_data_format = format;
	ilits->tp_data_len = len;
	TS_LOG_INFO("TP mode = %d, format = %d, len = %d\n",
		tp_mode, ilits->tp_data_format, ilits->tp_data_len);

	return ret;
}

int ilitek_read_roi_data(void)
{
	int i = 0;
	int ret = 0;
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;

	TS_LOG_DEBUG("roi_switch = %d, roi_support =%d\n",
		ts_dev_data->ts_platform_data->feature_info.roi_info.roi_switch,
		ts_dev_data->ts_platform_data->feature_info.roi_info.roi_supported);

	TS_LOG_DEBUG("fingers = %d, last_fingers =%d\n",
		ilits->finger,
		ilits->last_touch);

	if(ts_dev_data->ts_platform_data->feature_info.roi_info.roi_switch
		&& ts_dev_data->ts_platform_data->feature_info.roi_info.roi_supported) {
		if (ilits->last_touch != ilits->finger &&
			ilits->finger <= ILITEK_ROI_FINGERS ) {
			mutex_lock(&ilits->roi_mutex);
			ilits->roi_data_ready = false;
			mutex_unlock(&ilits->roi_mutex);
			ret = ili_ic_func_ctrl("roi_set",P5_X_GET_ROI_DATA);
			if (ret){
				TS_LOG_ERR("set roi cmd failed, ret = %d\n", ret);
				return ret;
			}
			mdelay(1);
			ret = ilits->wrapper(NULL, 0, ilits->roi_buf, ROI_DATA_READ_LENGTH+1, OFF, OFF);
			if (ret <0){
				TS_LOG_ERR("spi read roi data failed, ret = %d\n", ret);
				return ret;
			}
			mutex_lock(&ilits->roi_mutex);
			memcpy(ilits->roi_data,&ilits->roi_buf[1], ROI_DATA_READ_LENGTH);
			ilits->roi_data_ready = true;
			mutex_unlock(&ilits->roi_mutex);
			complete_all(&ilits->roi_completion);
			TS_LOG_DEBUG( "index = %d, fixed = %d, peak_row = %d, peak_colum = %d\n",
			ilits->roi_data[0], ilits->roi_data[1],
			ilits->roi_data[2], ilits->roi_data[3]);
			if (debug_en) {
				TS_LOG_INFO("=================== roi data in bytes ===================\n");
				for (i = 4; i < ROI_DATA_READ_LENGTH; i++) {
					TS_LOG_INFO(KERN_CONT "%3d ", ilits->roi_data[i]);
					if ((i - 3) % 14 == 0) /* 14 * 14  rawdata bytes */
						TS_LOG_INFO("\n");
				}
			}
		}
	}

	return 0;
}
static void ilitek_tddi_spi_recovery(struct ts_fingers *p_info)
{
	ili_ic_get_pc_counter(DO_SPI_RECOVER);
	if (ilits->actual_tp_mode == P5_X_FW_GESTURE_MODE &&
		ilits->gesture && !ilits->prox_near) {
		TS_LOG_ERR("Gesture failed, doing gesture recovery\n");
		if (ili_gesture_recovery() < 0)
			TS_LOG_ERR("Failed to recover gesture\n");
		ilits->irq_after_recovery = true;
	} else {
		TS_LOG_ERR("SPI ACK failed, doing spi recovery\n");
		ili_spi_recovery();
		if (ilits->game_switch)
			ili_ic_func_ctrl("lock_point", DISABLE);
		else
			ili_ic_func_ctrl("lock_point", ENABLE);
		p_info->cur_finger_number = FINGER_NUM_ZERO;
		ilits->irq_after_recovery = true;
	}
}
static int ilitek_tddi_spi_read(struct ts_fingers *p_info, int rlen)
{
	int ret;

	ret = ilits->wrapper(NULL, 0, ilits->tr_buf, rlen, OFF, OFF);
	if (ret < 0) {
		TS_LOG_ERR("actual_tp_mode = %d, gesture = %d, prox_near= %d\n",
			ilits->actual_tp_mode,
			ilits->gesture, ilits->prox_near);
		if ((ret == DO_SPI_RECOVER) && (!ilits->esd_debug))
			ilitek_tddi_spi_recovery(p_info);
		else
			msleep(WAIT_FOR_SPI_BUS_READ_DELAY);
	}

	return ret;
}
int ilitek_tddi_read_report_data(struct ts_fingers *p_info)
{
	int ret = 0, pid = 0;
	int i;
	u8  checksum = 0, pack_checksum = 0;
	u8 *trdata = NULL;
	int rlen = 0;
	int tmp = debug_en;
	/* Just in case these stats couldn't be blocked in top half context */
	struct ts_kit_device_data *ts_dev_data = ilits->ts_dev_data;
	struct ts_kit_platform_data *ts_platform_data = ts_dev_data->ts_platform_data;
	struct ts_easy_wakeup_info *gesture_report_info = &ts_dev_data->easy_wakeup_info;

	if (!ilits->report || atomic_read(&ilits->tp_reset) ||
		atomic_read(&ilits->fw_stat) || atomic_read(&ilits->tp_sw_mode) ||
		atomic_read(&ilits->mp_stat) || atomic_read(&ilits->tp_sleep)) {
		TS_LOG_INFO("ignore report request\n");
		return -EINVAL;
	}

	if (ilits->irq_after_recovery) {
		TS_LOG_INFO("ignore int triggered by recovery\n");
		ilits->irq_after_recovery = false;
		return -EINVAL;
	}
#if defined(CONFIG_HUAWEI_DEVKIT_QCOM_3_0)
	/*if the easy_wakeup_flag is false,status not in gesture;switch_value is false,gesture is no supported*/
	if ((true == ts_platform_data->feature_info.wakeup_gesture_enable_info.switch_value) &&
		(true == gesture_report_info->easy_wakeup_flag))
		msleep(100);
#endif
	if (ilits->esd_debug_delay)
		mdelay(ilits->esd_debug_delay);
	ili_wq_ctrl(WQ_ESD, DISABLE);
	ili_wq_ctrl(WQ_BAT, DISABLE);
	rlen = ilits->tp_data_len;
	TS_LOG_DEBUG("Packget length = %d\n", rlen);

	if (!rlen || rlen > TR_BUF_SIZE) {
		TS_LOG_ERR("Length of packet is invaild\n");
		goto out;
	}

	memset(ilits->tr_buf, 0x0, TR_BUF_SIZE);
	for (i = 0; i < GESTURE_EVENT_RETRY_TIME; i++) {
		ret = ilitek_tddi_spi_read(p_info, rlen);
		if (!ret)
			break;
		else if (ret == DO_SPI_RECOVER)
			goto out;
		else
			TS_LOG_ERR("spi not recovery, need retry\n");
	}

	ili_dump_data(ilits->tr_buf, 8, rlen, 0, "finger report");

	checksum = ili_calc_packet_checksum(ilits->tr_buf, rlen - 1);
	pack_checksum = ilits->tr_buf[rlen-1];
	trdata = ilits->tr_buf;
	pid = trdata[0];
	TS_LOG_DEBUG("Packet ID = %x\n", pid);

	if (checksum != pack_checksum && pid != P5_X_I2CUART_PACKET_ID) {
		TS_LOG_ERR("Checksum Error (0x%X)! Pack = 0x%X, len = %d\n", checksum, pack_checksum, rlen);
		debug_en = DEBUG_ALL;
		ili_dump_data(trdata, 8, rlen, 0, "finger report with wrong");
		debug_en = tmp;
		ret = -EINVAL;
		goto out;
	}

	if (pid == P5_X_INFO_HEADER_PACKET_ID) {
		trdata = ilits->tr_buf + P5_X_INFO_HEADER_LENGTH;
		pid = trdata[0];
	}
	switch (pid) {
	case P5_X_DEMO_PACKET_ID:
		ili_report_ap_mode(p_info,trdata, rlen);
		break;
	case P5_X_DEBUG_PACKET_ID:
		ili_report_debug_mode(p_info,trdata, rlen);
		break;
	case P5_X_DEBUG_LITE_PACKET_ID:
		ili_report_debug_lite_mode(p_info,trdata, rlen);
		break;
	case P5_X_I2CUART_PACKET_ID:
		ili_report_i2cuart_mode(trdata, rlen);
		ret = -EINVAL;
		break;
	case P5_X_GESTURE_PACKET_ID:
		ili_report_gesture_mode(p_info,trdata, rlen);
		break;
	case P5_X_GESTURE_FAIL_ID:
		TS_LOG_INFO("gesture fail reason code = 0x%02x", trdata[1]);
		break;
	case P5_X_DEMO_DEBUG_INFO_PACKET_ID:
		ili_demo_debug_info_mode(p_info,trdata, rlen);
		break;
	default:
		TS_LOG_ERR("Unknown packet id, %x\n", pid);
		break;
	}

out:
	if (ret == DO_SPI_RECOVER) {
		TS_LOG_DEBUG("after spi recovery, report up event\n");
		ret = NO_ERR;
	}
	if (ilits->actual_tp_mode != P5_X_FW_GESTURE_MODE) {
		ili_wq_ctrl(WQ_ESD, ENABLE);
		ili_wq_ctrl(WQ_BAT, ENABLE);
	}

	return ret;
}

int ili_reset_ctrl(int mode)
{
	int ret = 0;

	atomic_set(&ilits->tp_reset, START);

	switch (mode) {
	case TP_IC_CODE_RST:
		TS_LOG_INFO("TP IC Code RST \n");
		ret = ili_ic_code_reset();
		if (ret < 0)
			TS_LOG_ERR("IC Code reset failed\n");
		break;
	case TP_IC_WHOLE_RST:
		TS_LOG_INFO("TP IC whole RST\n");
		ret = ili_ic_whole_reset();
		if (ret < 0)
			TS_LOG_ERR("IC whole reset failed\n");
		break;
	case TP_HW_RST_ONLY:
		TS_LOG_INFO("TP HW RST\n");
		ili_tp_reset();
		break;
	default:
		TS_LOG_ERR("Unknown reset mode, %d\n", mode);
		ret = -EINVAL;
		break;
	}

	/*
	 * Since OTP must be folloing with reset, except for code rest,
	 * the stat of ice mode should be set as 0.
	 */
	if (mode != TP_IC_CODE_RST)
		atomic_set(&ilits->ice_stat, DISABLE);

	ilits->tp_data_format = DATA_FORMAT_DEMO;
	ilits->tp_data_len = P5_X_DEMO_MODE_PACKET_LEN;
	atomic_set(&ilits->tp_reset, END);
	return ret;
}

static int ilitek_get_tp_module(void)
{
	/*
	 * TODO: users should implement this function
	 * if there are various tp modules been used in projects.
	 */

	return 0;
}

void ili_update_tp_module_info(void)
{
	int module;

	module = ilitek_get_tp_module();

	switch (module) {
	case MODEL_CSOT:
		ilits->md_name = "CSOT";
		ilits->md_fw_filp_path = CSOT_FW_FILP_PATH;
		ilits->md_fw_rq_path = CSOT_FW_REQUEST_PATH;
		ilits->md_ini_path = CSOT_INI_NAME_PATH;
		ilits->md_ini_rq_path = CSOT_INI_REQUEST_PATH;
		break;
	case MODEL_AUO:
		ilits->md_name = "AUO";
		ilits->md_fw_filp_path = AUO_FW_FILP_PATH;
		ilits->md_fw_rq_path = AUO_FW_REQUEST_PATH;
		ilits->md_ini_path = AUO_INI_NAME_PATH;
		ilits->md_ini_rq_path = AUO_INI_REQUEST_PATH;
		break;
	case MODEL_BOE:
		ilits->md_name = "BOE";
		ilits->md_fw_filp_path = BOE_FW_FILP_PATH;
		ilits->md_fw_rq_path = BOE_FW_REQUEST_PATH;
		ilits->md_ini_path = BOE_INI_NAME_PATH;
		ilits->md_ini_rq_path = BOE_INI_REQUEST_PATH;
		break;
	case MODEL_INX:
		ilits->md_name = "INX";
		ilits->md_fw_filp_path = INX_FW_FILP_PATH;
		ilits->md_fw_rq_path = INX_FW_REQUEST_PATH;
		ilits->md_ini_path = INX_INI_NAME_PATH;
		ilits->md_ini_rq_path = INX_INI_REQUEST_PATH;
		break;
	case MODEL_DJ:
		ilits->md_name = "DJ";
		ilits->md_fw_filp_path = DJ_FW_FILP_PATH;
		ilits->md_fw_rq_path = DJ_FW_REQUEST_PATH;
		ilits->md_ini_path = DJ_INI_NAME_PATH;
		ilits->md_ini_rq_path = DJ_INI_REQUEST_PATH;
		break;
	case MODEL_TXD:
		ilits->md_name = "TXD";
		ilits->md_fw_filp_path = TXD_FW_FILP_PATH;
		ilits->md_fw_rq_path = TXD_FW_REQUEST_PATH;
		ilits->md_ini_path = TXD_INI_NAME_PATH;
		ilits->md_ini_rq_path = TXD_FW_REQUEST_PATH;
		break;
	case MODEL_TM:
		ilits->md_name = "TM";
		ilits->md_fw_filp_path = TM_FW_REQUEST_PATH;
		ilits->md_fw_rq_path = TM_FW_REQUEST_PATH;
		ilits->md_ini_path = TM_INI_NAME_PATH;
		ilits->md_ini_rq_path = TM_INI_REQUEST_PATH;
		break;
	default:
		break;
	}

	if (module == 0) {
		TS_LOG_ERR("Couldn't find any tp modules, applying default settings\n");
		ilits->md_name = "DEF";
		ilits->md_fw_filp_path = DEF_FW_FILP_PATH;
		ilits->md_fw_rq_path = DEF_FW_REQUEST_PATH;
		ilits->md_ini_path = DEF_INI_NAME_PATH;
		ilits->md_ini_rq_path = DEF_INI_REQUEST_PATH;
	}

	ilits->tp_module = module;
}


void ili_dev_remove(void)
{
	TS_LOG_INFO("remove ilitek dev\n");

	if (!ilits)
		return;

	if (esd_wq != NULL) {
		cancel_delayed_work_sync(&esd_work);
		flush_workqueue(esd_wq);
		destroy_workqueue(esd_wq);
	}
	if (bat_wq != NULL) {
		cancel_delayed_work_sync(&bat_work);
		flush_workqueue(bat_wq);
		destroy_workqueue(bat_wq);
	}

	if (ilits->ws)
		wakeup_source_unregister(ilits->ws);

	kfree(ilits->tr_buf);
	kfree(ilits->gcoord);
}

