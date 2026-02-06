/*
 * Linux kernel modules for VI5300 FlightSense TOF sensor
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/atomic.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/of_gpio.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <securec.h>
#include <media/huawei/laser_cfg.h>

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <huawei_platform/devdetect/hw_dev_dec.h>
#endif

#include "vi5300.h"
#include "vi5300_platform.h"
#include "vi5300_firmware.h"
#include "vi5300_api.h"

#define VI5300_DRV_NAME "vi5300"
#define VI5300_IOCTL_XTALK_CALIB _IOR('p', 0x02, struct VI5300_XTALK_Calib_Data)
#define VI5300_IOCTL_XTALK_CONFIG _IOW('p', 0x03, int8_t)
#define VI5300_IOCTL_POWER_ON _IO('p', 0x04)
#define VI5300_IOCTL_CHIP_INIT _IO('p', 0x05)
#define VI5300_IOCTL_CONTINUOUS_START _IO('p', 0x06)
#define VI5300_IOCTL_CONTINUOUS_STOP _IO('p', 0x07)
#define VI5300_IOCTL_POWER_OFF _IO('p', 0x08)
#define VI5300_IOCTL_SINGLE_RANGE _IO('p', 0x0b)
#define VI5300_IOCTL_GET_DATAS _IOR('p', 0x0c, struct VI5300_Measurement_Data)
#define VI5300_IOCTL_OFFSET_CONFIG _IOW('p', 0x0d, int16_t)
#define VI5300_IOCTL_OFFSET_CALIB _IOR('p', 0x09, struct VI5300_OFFSET_Calib_Data)
#define SWITCH_ON_LOG_LEN 1000
#define VER_MSB 3
#define VER_LSB 14

static int xtalk_mark;
static int offset_mark;

struct vi5300_api_fn_t {
	int32_t (*Power_ON)(VI5300_DEV dev);
	int32_t (*Power_OFF)(VI5300_DEV dev);
	int32_t (*Single_Measure)(VI5300_DEV dev);
	int32_t (*Start_Continuous_Measure)(VI5300_DEV dev);
	int32_t (*Stop_Continuous_Measure)(VI5300_DEV dev);
	int32_t (*Get_Measure_Data)(VI5300_DEV dev);
	int32_t (*Get_Interrupt_State)(VI5300_DEV dev);
	int32_t (*Chip_Init)(VI5300_DEV dev);
	int32_t (*Start_XTalk_Calibration)(VI5300_DEV dev);
	int32_t (*Start_Offset_Calibration)(VI5300_DEV dev);
	int32_t (*Get_XTalk_Parameter)(VI5300_DEV dev);
	int32_t (*Config_XTalk_Parameter)(VI5300_DEV dev);
};

static struct vi5300_api_fn_t vi5300_api_func_tbl = {
	.Power_ON = VI5300_Chip_PowerON,
	.Power_OFF = VI5300_Chip_PowerOFF,
	.Single_Measure = VI5300_Single_Measure,
	.Start_Continuous_Measure = VI5300_Start_Continuous_Measure,
	.Stop_Continuous_Measure = VI5300_Stop_Continuous_Measure,
	.Get_Measure_Data = VI5300_Get_Measure_Data,
	.Get_Interrupt_State = VI5300_Get_Interrupt_State,
	.Chip_Init = VI5300_Chip_Init,
	.Start_XTalk_Calibration = VI5300_Start_XTalk_Calibration,
	.Start_Offset_Calibration = VI5300_Start_Offset_Calibration,
	.Get_XTalk_Parameter = VI5300_Get_XTalk_Parameter,
	.Config_XTalk_Parameter = VI5300_Config_XTalk_Parameter,
};
struct vi5300_api_fn_t *vi5300_func_tbl;

static void vi5300_setupAPIFunctions(void)
{
	vi5300_func_tbl->Power_ON = VI5300_Chip_PowerON;
	vi5300_func_tbl->Power_OFF = VI5300_Chip_PowerOFF;
	vi5300_func_tbl->Single_Measure = VI5300_Single_Measure;
	vi5300_func_tbl->Start_Continuous_Measure = VI5300_Start_Continuous_Measure;
	vi5300_func_tbl->Stop_Continuous_Measure = VI5300_Stop_Continuous_Measure;
	vi5300_func_tbl->Get_Measure_Data = VI5300_Get_Measure_Data;
	vi5300_func_tbl->Get_Interrupt_State = VI5300_Get_Interrupt_State;
	vi5300_func_tbl->Chip_Init = VI5300_Chip_Init;
	vi5300_func_tbl->Start_XTalk_Calibration = VI5300_Start_XTalk_Calibration;
	vi5300_func_tbl->Start_Offset_Calibration = VI5300_Start_Offset_Calibration;
	vi5300_func_tbl->Get_XTalk_Parameter = VI5300_Get_XTalk_Parameter;
	vi5300_func_tbl->Config_XTalk_Parameter = VI5300_Config_XTalk_Parameter;
}

static irqreturn_t vi5300_irq_handler(int vec, void *info)
{
	struct vi5300_data *data = (struct vi5300_data *)info;
	VI5300_Error Status = VI5300_ERROR_NONE;

	vi5300_infomsg("enter vi5300_irq_handler:\n");

	if (!data || data->firmware_status == 0)
		return IRQ_HANDLED;

	if (data->irq == vec) {
		if (xtalk_mark) {
			Status = vi5300_func_tbl->Get_XTalk_Parameter(data);
			if (Status != VI5300_ERROR_NONE)
				vi5300_errmsg("%d vi5300_irq_handler: Get_XTalk_Parameter failed\n", __LINE__);
		}
		if (!xtalk_mark && !offset_mark) {
			data->RangData.status = vi5300_func_tbl->Get_Measure_Data(data);
			if (data->RangData.status != VI5300_ERROR_NONE) {
				data->RangData.status = VI5300_ERROR_GET_DATA;
				vi5300_errmsg("%d vi5300_irq_handler: Get_Measure_Data failed\n", __LINE__);
			}
			data->RangData.status = vi5300_func_tbl->Get_Interrupt_State(data);
			if (data->RangData.status != VI5300_ERROR_NONE) {
				data->RangData.status = VI5300_ERROR_IRQ_STATE;
				vi5300_errmsg("%d vi5300_irq_handler: Get_Interrupt_State failed\n", __LINE__);
			}
		}
	}
	return IRQ_HANDLED;
}

static int vi5300_parse_dt(struct device_node *np, struct vi5300_data *data)
{
	enum of_gpio_flags flags;
	uint32_t inte_counts = 0;
	uint8_t frame_counts = 0;

	if (!data || !np)
		return -EINVAL;

	data->irq_gpio = of_get_named_gpio_flags(np, "vi5300,irq-gpio", 0, &flags);
	if (data->irq_gpio < 0) {
		vi5300_errmsg("get irq gpio: %d error\n", data->irq_gpio);
		return -ENODEV;
	}
	vi5300_infomsg("INT GPIO %d\n", data->irq_gpio);

	data->xshut_gpio = of_get_named_gpio_flags(np, "vi5300,xshut-gpio", 0, &flags);
	if (data->xshut_gpio < 0) {
		vi5300_errmsg("get xshut gpio: %d error\n", data->xshut_gpio);
		return -ENODEV;
	}
	vi5300_infomsg("XSHUT GPIO %d\n", data->xshut_gpio);

	if (!of_property_read_u32(np, "vi5300,integral-counts", &inte_counts))
		data->integral_counts = inte_counts;
	else
		data->integral_counts = DEFAULT_INTEGRAL_COUNTS;
	if (!of_property_read_u8(np, "vi5300,frame-counts", &frame_counts))
		data->frame_counts = frame_counts;
	else
		data->frame_counts = DEFAULT_FRAME_COUNTS;
	return 0;
}

static int vi5300_open(struct inode *inode, struct file *file)
{
	return 0;
}

static void vi5300_enable_irq(struct vi5300_data* data)
{

	if (!data) {
		vi5300_errmsg("notify data event failed\n");
		return;
	}

	if (data->intr_status == VI5300_IRQ_DISABLE) {
		enable_irq(data->irq);
		data->intr_status = VI5300_IRQ_ENABLE;
		vi5300_infomsg("vi5300 enable irq\n");
	}

	return;
}

static void vi5300_disable_irq(struct vi5300_data* data)
{

	if (!data) {
		vi5300_errmsg("notify data event failed\n");
		return;
	}

	if (data->intr_status == VI5300_IRQ_ENABLE) {
		disable_irq(data->irq);
		data->intr_status = VI5300_IRQ_DISABLE;
		vi5300_infomsg("vi5300 diable irq\n");
	}

	return;
}

	/* match id */
static int vi5300_status(struct vi5300_data* data, hwlaser_status_t *laser_status)
{
	int rc = -EINVAL;
	if (!data || !laser_status)
		return rc;

	laser_status->status = VI5300_ERROR_NONE;

	return VI5300_ERROR_NONE;
}

static int vi5300_GetCalibrationData(struct vi5300_data *data, hwlaser_calibration_data_t *cal_data)
{
	int rc = 0;
	mutex_lock(&data->work_mutex);

	rc = memcpy_s(&(cal_data->u.dataL2.VI5300_XTALK_Calib_Data), sizeof(struct VI5300_XTALK_Calib_Data), &(data->XtalkData), sizeof(struct VI5300_XTALK_Calib_Data));
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}

	rc = memcpy_s(&(cal_data->u.dataL2.VI5300_OFFSET_Calib_Data), sizeof(struct VI5300_OFFSET_Calib_Data), &(data->OffsetData), sizeof(struct VI5300_OFFSET_Calib_Data));
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}

	vi5300_infomsg("kernel xtalk_cal(%d), kernel xtalk_peak(%u), kernel xtalk_tof(%d)",
		data->XtalkData.xtalk_cal, data->XtalkData.xtalk_peak, data->XtalkData.xtalk_tof);
	vi5300_infomsg("hal xtalk_cal(%d), hal xtalk_peak(%u), hal xtalk_tof(%d)",
		cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_cal, cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_peak,
		cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_tof);
	vi5300_infomsg("kernel offset_cal(%d)", data->OffsetData.offset_cal);
	vi5300_infomsg("hal offset_cal(%d)", cal_data->u.dataL2.VI5300_OFFSET_Calib_Data.offset_cal);

	mutex_unlock(&data->work_mutex);
	return rc;
}

static int vi5300_SetCalibrationData(struct vi5300_data *data, hwlaser_calibration_data_t *cal_data)
{
	int rc = 0;
	mutex_lock(&data->work_mutex);

	rc = memcpy_s(&(data->xtalk_config), sizeof(int8_t), &(cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_cal), sizeof(int8_t));
	vi5300_infomsg("hal xtalk_config(%d)", cal_data->u.dataL2.VI5300_XTALK_Calib_Data.xtalk_cal);
	vi5300_infomsg("kernel xtalk_config(%d)", data->xtalk_config);
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}
	rc = vi5300_func_tbl->Config_XTalk_Parameter(data);
	if (rc != VI5300_ERROR_NONE)
		data->status = VI5300_ERROR_XTALK_CONFIG;

	rc = memcpy_s(&(data->offset_config), sizeof(int16_t), &(cal_data->u.dataL2.VI5300_OFFSET_Calib_Data.offset_cal), sizeof(int16_t));
	vi5300_infomsg("hal offset_config(%d)", cal_data->u.dataL2.VI5300_OFFSET_Calib_Data.offset_cal);
	vi5300_infomsg("kernel offset_config(%d)", data->offset_config);
	if (rc) {
		mutex_unlock(&data->work_mutex);
		return -EFAULT;
	}

	mutex_unlock(&data->work_mutex);
	return rc;
}

static int vi5300_laser_get_set_cal_data(struct vi5300_data* data,
                                            hwlaser_calibration_data_t *cal_data)
{
	int rc = 0;

	if (cal_data->is_read) {
		vi5300_dbgmsg("vi5300 hal get calibration data\n");
		rc = vi5300_GetCalibrationData(data, cal_data);
		if (rc) {
			vi5300_errmsg("%d, hal get xtalk, offset fail\n", __LINE__);
			return -EFAULT;
		}
	} else {
		vi5300_dbgmsg("vi5300 hal set calibration data to kernel\n");
		rc = vi5300_SetCalibrationData(data, cal_data);
		if (rc) {
			vi5300_errmsg("%d, kernel get xtalk_config, offset_config fail\n", __LINE__);
			return -EFAULT;
		}
	}

	return rc;
}

static int vi5300_perform_cal(struct vi5300_data* data, hwlaser_ioctl_perform_calibration_t* cal)
{
	int rc = 0;

	mutex_lock(&data->work_mutex);
	switch (cal->calibration_type) {
	case HWLASER_CALIBRATION_CROSSTALK:
		xtalk_mark = 1;
		rc = vi5300_func_tbl->Start_XTalk_Calibration(data);
		if(rc != VI5300_ERROR_NONE) {
			data->status += VI5300_ERROR_XTALK_CALIB;
			vi5300_errmsg("vi5300 perform XTALK calibration fail");
		} else {
			msleep(1000); // delay 1000 ms for SoC cal data ready
		}
		xtalk_mark = 0;
		break;
	case HWLASER_CALIBRATION_OFFSET:
		offset_mark = 1;
		rc = vi5300_func_tbl->Start_Offset_Calibration(data);
		if(rc != VI5300_ERROR_NONE) {
			data->status += VI5300_ERROR_OFFSET_CALIB;
			vi5300_errmsg("vi5300 perform OFFSET calibration fail");
		}
		offset_mark = 0;
		break;
	default:
		vi5300_errmsg("vi5300_perform_cal %d, fail %d",
			cal->calibration_type, rc);
	}
	mutex_unlock(&data->work_mutex);

	return rc;
}

static int vi5300_set_params(struct vi5300_data *data, hwlaser_parameter_t *pParameter)
{
	int rc = 0;
	if (!pParameter) {
		rc = -EFAULT;
		return;
	}

	mutex_lock(&data->work_mutex);

	switch(pParameter->name) {
	case XTALKRATE_PAR:
		rc = memcpy_s(&(data->xtalk_config), sizeof(int8_t), &(pParameter->xtalk_config), sizeof(int8_t));
		vi5300_infomsg("kernel xtalk_config(%d)", pParameter->xtalk_config);
		vi5300_infomsg("hal xtalk_config(%d)", data->xtalk_config);
		if (rc) {
			vi5300_errmsg("%d, GET XTALK_CONFIG PARAM fail\n", __LINE__);
			mutex_unlock(&data->work_mutex);
			return -EFAULT;
		}
		rc = vi5300_func_tbl->Config_XTalk_Parameter(data);
		if (rc != VI5300_ERROR_NONE)
		data->status = VI5300_ERROR_XTALK_CONFIG;
		break;
	case OFFSET_PAR:
		rc = memcpy_s(&(data->offset_config), sizeof(int16_t), &(pParameter->offset_config), sizeof(int16_t));
		vi5300_infomsg("kernel offset_config(%d)", pParameter->offset_config);
		vi5300_infomsg("hal offset_config(%d)", data->offset_config);
		if (rc) {
			vi5300_errmsg("%d, GET OFFSET_CONFIG PARAM fail\n", __LINE__);
			mutex_unlock(&data->work_mutex);
			return -EFAULT;
		}
		break;
	default:
		vi5300_errmsg("unkown or unsupported cmd %d", pParameter->name);
		rc = -EINVAL;
	}

	mutex_unlock(&data->work_mutex);
	return rc;
}

long vi5300_ioctl(void *hw_data, unsigned int cmd, void *p)
{
	long rc = 0;
	struct vi5300_data *data = (struct vi5300_data*)hw_data;
	hwlaser_calibration_data_t *cal_data = NULL;
	hwlaser_ioctl_perform_calibration_t* cal_perform = NULL;
	hwlaser_status_t* status = NULL;
	hwlaser_info_t *pinfo = NULL;
	hwlaser_RangingData_t *pRangingData = NULL;
	hwlaser_parameter_t *pParameter = NULL;
	data->RangData.status = VI5300_ERROR_NONE;

	if (!data)
		return -EFAULT;

	switch (cmd) {
	case HWLASER_IOCTL_POWERON:
		vi5300_infomsg("HWLASER_IOCTL_POWERON\n");
		mutex_lock(&data->work_mutex);
		vi5300_enable_irq(data);
		rc = vi5300_func_tbl->Power_ON(data);
		if(rc != VI5300_ERROR_NONE)
			data->status += VI5300_ERROR_GPIO_ERROR;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_INIT:
		vi5300_infomsg("HWLASER_IOCTL_INIT\n");
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Chip_Init(data);
		if (rc != VI5300_ERROR_NONE)
			data->status += VI5300_ERROR_INIT_ERROR;
		else
			data->firmware_status = 1;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_GET_INFO:
		vi5300_infomsg("HWLASER_IOCTL_GET_INFO\n");
		mutex_lock(&data->work_mutex);
		pinfo = (hwlaser_info_t *)p;
		if (strncpy_s(pinfo->name, HWLASER_NAME_SIZE - 1 ,"vi5300", HWLASER_NAME_SIZE - 1))
			vi5300_errmsg("strncpy_s failed %d", __LINE__);
		if (strncpy_s(pinfo->product_name, HWLASER_NAME_SIZE - 1 ,"JADE_LASER", HWLASER_NAME_SIZE - 1))
			vi5300_errmsg("strncpy_s failed %d", __LINE__);
		pinfo->version = HWLASER_VI_I0_VERSION;
		pinfo->ap_pos = HWLASER_POS_AP;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_START:
		vi5300_infomsg("HWLASER_IOCTL_START\n");
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Start_Continuous_Measure(data);
		if (rc != VI5300_ERROR_NONE)
			data->status += VI5300_ERROR_CONTINUOUS_CMD;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_MZ_DATA:
		vi5300_infomsg("HWLASER_IOCTL_GET_DATA\n");
		mutex_lock(&data->work_mutex);
		pRangingData = (hwlaser_RangingData_t *)p;
		if (memcpy_s(&(pRangingData->v_data), sizeof(struct VI5300_Measurement_Data), &(data->RangData), sizeof(struct VI5300_Measurement_Data)) != EOK) {
			vi5300_errmsg("memcpy failed %d", __LINE__);
			mutex_unlock(&data->work_mutex);
			return -EFAULT;
		}
		mutex_unlock(&data->work_mutex);
		vi5300_infomsg("milimeter(%d), confidence(%u), status(%u)",
			pRangingData->v_data.milimeter,
			pRangingData->v_data.confidence,
			pRangingData->v_data.status);
		vi5300_infomsg("milimeter(%d), confidence(%u), status(%u)",
			data->RangData.milimeter,
			data->RangData.confidence,
			data->RangData.status);
		break;
	case HWLASER_IOCTL_STOP:
		vi5300_infomsg("HWLASER_IOCTL_STOP\n");
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Stop_Continuous_Measure(data);
		if (rc != VI5300_ERROR_NONE)
			data->status += VI5300_ERROR_STOP_CMD;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_POWEROFF:
		vi5300_infomsg("HWLASER_IOCTL_POWEROFF\n");
		vi5300_disable_irq(data);
		data->firmware_status = 0;
		mutex_lock(&data->work_mutex);
		rc = vi5300_func_tbl->Power_OFF(data);
		if (rc != VI5300_ERROR_NONE)
			data->status += VI5300_ERROR_GPIO_ERROR;
		mutex_unlock(&data->work_mutex);
		break;
	case HWLASER_IOCTL_CALIBRATION_DATA:
		vi5300_infomsg("HWLASER_IOCTL_CALIBRATION_DATA\n");
		cal_data = (hwlaser_calibration_data_t *)p;
		rc = vi5300_laser_get_set_cal_data(data, cal_data);
		break;
	case HWLASER_IOCTL_PERFORM_CALIBRATION:
		vi5300_infomsg("HWLASER_IOCTL_PERFORM_CALIBRATION\n");
		cal_perform = (hwlaser_ioctl_perform_calibration_t*) p;
		rc = vi5300_perform_cal(data, cal_perform);
		break;
	case HWLASER_IOCTL_PARAMETER:
		vi5300_infomsg("HWLASER_IOCTL_PARAMETER\n");
		pParameter = (hwlaser_parameter_t *)p;
		rc = vi5300_set_params(data, pParameter);
		break;
	case HWLASER_IOCTL_ROI:
		vi5300_infomsg("HWLASER_IOCTL_ROI\n");
		break;
	case HWLASER_IOCTL_CONFIG:
		vi5300_infomsg("HWLASER_IOCTL_CONFIG\n");
		break;
	case HWLASER_IOCTL_MATCHID:
		vi5300_infomsg("HWLASER_IOCTL_MATCHID");
		mutex_lock(&data->work_mutex);
		status = (hwlaser_status_t*) p;
		rc = vi5300_status(data, p);
		mutex_unlock(&data->work_mutex);
		break;
	default:
		vi5300_infomsg("cmd not support: %u", cmd);
		return -EFAULT;
	}
	return rc;
}
EXPORT_SYMBOL(vi5300_ioctl);

static int vi5300_flush(struct file *file, fl_owner_t id)
{
	return 0;
}

static int vi5300_setup(struct vi5300_data *data)
{
	int rc = 0;
	int irq = 0;
	uint8_t buf = 0;

	if (!data)
		return -EINVAL;

	if (!gpio_is_valid(data->irq_gpio) || !gpio_is_valid(data->xshut_gpio))
		return -ENODEV;

	gpio_request(data->xshut_gpio, "vi5300 xshut gpio");
	gpio_request(data->irq_gpio, "vi5300 int gpio");
	gpio_direction_input(data->irq_gpio);
	data->irq = gpio_to_irq(data->irq_gpio);
	if (data->irq < 0) {
		vi5300_errmsg("fail to map GPIO: %d to INT: %d\n", data->irq_gpio, irq);
		rc = -EINVAL;
		goto exit_free_gpio;
	} else {
		vi5300_dbgmsg("request irq: %d\n", irq);
		rc = request_threaded_irq(data->irq, NULL, vi5300_irq_handler,
			IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "vi5300_interrupt", (void *)data);
		if (rc) {
			vi5300_errmsg("%s(%d), Could not allocate VI5300_INT ! result:%d\n", __FUNCTION__, __LINE__, rc);
			goto exit_free_irq;
		}
	}
	data->intr_status = VI5300_IRQ_DISABLE;
	data->firmware_status = 0;
	disable_irq_nosync(data->irq);

	vi5300_func_tbl = &vi5300_api_func_tbl;
	vi5300_setupAPIFunctions();
	vi5300_func_tbl->Power_ON(data);
	vi5300_write_byte(data, VI5300_REG_MCU_CFG, 0x00);
	vi5300_write_byte(data, VI5300_REG_SYS_CFG, 0x0C);
	vi5300_write_byte(data, VI5300_REG_PW_CTRL, 0x0E);
	vi5300_write_byte(data, VI5300_REG_PW_CTRL, 0x0F);
	mdelay(5);
	vi5300_read_byte(data, VI5300_REG_DEV_ADDR, &buf);
	vi5300_func_tbl->Power_OFF(data);

	if (buf != VI5300_CHIP_ADDR) {
		vi5300_errmsg("VI5300 I2C Transfer Failed\n");
		rc = -EFAULT;
		goto exit_free_irq;
	}
	vi5300_infomsg("VI5300 I2C Transfer Successfully\n");
	data->RangData.status = 255;
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	set_hw_dev_flag(DEV_I2C_LASER);
#endif
	return 0;

exit_free_irq:
	free_irq(irq, data);
exit_free_gpio:
	gpio_free(data->xshut_gpio);
	gpio_free(data->irq_gpio);
	return rc;
}

int vi5300_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct vi5300_data *vi5300_data = NULL;
	struct device *dev = &client->dev;
	struct device_node *node = NULL;
	hw_laser_ctrl_t *ctrl = NULL;
	int ret  = 0;

	vi5300_infomsg("%s Enter\n", __FUNCTION__);
	vi5300_data = kzalloc(sizeof(struct vi5300_data), GFP_KERNEL);
	if (!vi5300_data) {
		vi5300_errmsg("devm_kzalloc error\n");
		return -ENOMEM;
	}
	if (!dev->of_node) {
		kfree(vi5300_data);
		return -EINVAL;
	}
	/* setup device data */
	vi5300_data->dev_name = dev_name(&client->dev);
	vi5300_data->client = client;
	vi5300_data->dev = dev;
	ctrl = (hw_laser_ctrl_t *)id->driver_data;
	vi5300_data->ctrl = ctrl;
	ctrl->data = (void *)vi5300_data;
	node = dev->of_node;
	i2c_set_clientdata(client, vi5300_data);
	mutex_init(&vi5300_data->update_lock);
	mutex_init(&vi5300_data->work_mutex);
	ret = vi5300_parse_dt(node, vi5300_data);
	if (ret) {
		vi5300_errmsg("VI5300 Parse DT Failed\n");
		goto exit_error;
	}
	ret = vi5300_setup(vi5300_data);
	if (ret) {
		vi5300_errmsg("VI5300 Setup Failed\n");
		goto exit_error;
	}
	vi5300_infomsg("End!\n");
	return 0;

exit_error:
	mutex_destroy(&vi5300_data->work_mutex);
	i2c_set_clientdata(client, NULL);
	kfree(vi5300_data);
	return ret;
}
EXPORT_SYMBOL(vi5300_probe);

int vi5300_remove(struct i2c_client *client)
{
	vi5300_dbgmsg("vi5300:%s\n", __func__);
	struct vi5300_data *data = i2c_get_clientdata(client);
	if (data->xshut_gpio) {
		gpio_direction_output(data->xshut_gpio, 0);
		gpio_free(data->xshut_gpio);
	}
	if (data->irq_gpio) {
		free_irq(data->irq, data);
		gpio_free(data->irq_gpio);
	}
	misc_deregister(&data->miscdev);
	i2c_set_clientdata(client, NULL);
	mutex_destroy(&data->work_mutex);
	kfree(data);
	return 0;
}
EXPORT_SYMBOL(vi5300_remove);

static const struct i2c_device_id vi5300_id[] = {
	{ VI5300_DRV_NAME, 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, vi5300_id);

static const struct of_device_id vi5300_dt_match[] = {
	{.compatible = "huawei, vi5300", },
	{},
};
MODULE_DEVICE_TABLE(of, vi5300_dt_match);

struct i2c_driver vi5300_driver = {
	.driver  = {
		.name = VI5300_DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = vi5300_dt_match,
	},
	.probe = vi5300_probe,
	.remove = vi5300_remove,
	.id_table = vi5300_id,
};

static int __init vi5300_init(void)
{
	return i2c_add_driver(&vi5300_driver);
}

static void __exit vi5300_exit(void)
{
	i2c_del_driver(&vi5300_driver);
}

module_init(vi5300_init);
module_exit(vi5300_exit);

MODULE_AUTHOR("William.li<william.li@vidar.ai>");
MODULE_DESCRIPTION("VI5300 FlightSense TOF  sensor Driver");
MODULE_LICENSE("GPL");
