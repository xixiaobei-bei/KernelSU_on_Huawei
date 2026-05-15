/*
 *  stmvl53l0_module.c - Linux kernel modules for STM VL53L0 FlightSense TOF
 *						 sensor
 *
 *  Copyright (C) 2016 STMicroelectronics Imaging Division.
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

//lint -save -e550 -e731 -e747 -e826 -e30 -e142 -e845 -e747 -e774 -e734 -e563 -e715 -e750 -e754 -e438 -e732 -e838
//lint -save -esym(750,*) -esym(754,*)
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/kthread.h>
#include <linux/videodev2.h>

//#include "hw_vl53l0_cfg.h"
#include <media/huawei/laser_cfg.h>

#include "vl53l0_api.h"
#include "hw_laser_i2c.h"
/*#define USE_INT */
#define IRQ_NUM	   59
#define POLL_DELAY_TIME	        35 //ms

static char *stmvl53l0_name = "vl53l0";
extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern int strncpy_s(char *strDest, size_t destMax, const char *strSrc, size_t count);
extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);
struct stmvl53l0_api_fn_t {
	int8_t (*GetVersion)(VL53L0_Version_t *pVersion);
	int8_t (*GetPalSpecVersion)(VL53L0_Version_t *pPalSpecVersion);

	int8_t (*GetProductRevision)(VL53L0_DEV Dev,
					uint8_t *pProductRevisionMajor,
					uint8_t *pProductRevisionMinor);
	int8_t (*GetDeviceInfo)(VL53L0_DEV Dev,
				VL53L0_DeviceInfo_t *pVL53L0_DeviceInfo);
	int8_t (*GetDeviceErrorStatus)(VL53L0_DEV Dev,
				VL53L0_DeviceError *pDeviceErrorStatus);
	int8_t (*GetRangeStatusString)(uint8_t RangeStatus,
				char *pRangeStatusString);
	int8_t (*GetDeviceErrorString)(VL53L0_DeviceError ErrorCode,
				char *pDeviceErrorString);
	int8_t (*GetPalErrorString)(VL53L0_Error PalErrorCode,
				char *pPalErrorString);
	int8_t (*GetPalStateString)(VL53L0_State PalStateCode,
				char *pPalStateString);
	int8_t (*GetPalState)(VL53L0_DEV Dev,	VL53L0_State *pPalState);
	int8_t (*SetPowerMode)(VL53L0_DEV Dev,
				VL53L0_PowerModes PowerMode);
	int8_t (*GetPowerMode)(VL53L0_DEV Dev,
				VL53L0_PowerModes *pPowerMode);
	int8_t (*SetOffsetCalibrationDataMicroMeter)(VL53L0_DEV Dev,
				int32_t OffsetCalibrationDataMicroMeter);
	int8_t (*GetOffsetCalibrationDataMicroMeter)(VL53L0_DEV Dev,
				int32_t *pOffsetCalibrationDataMicroMeter);
	int8_t (*SetLinearityCorrectiveGain)(VL53L0_DEV Dev,
				int16_t LinearityCorrectiveGain);
	int8_t (*GetLinearityCorrectiveGain)(VL53L0_DEV Dev,
				uint16_t *pLinearityCorrectiveGain);
	int8_t (*SetGroupParamHold)(VL53L0_DEV Dev,
				uint8_t GroupParamHold);
	int8_t (*GetUpperLimitMilliMeter)(VL53L0_DEV Dev,
				uint16_t *pUpperLimitMilliMeter);
	int8_t (*SetDeviceAddress)(VL53L0_DEV Dev,
				uint8_t DeviceAddress);
	int8_t (*DataInit)(VL53L0_DEV Dev);
	int8_t (*SetTuningSettingBuffer)(VL53L0_DEV Dev,
				uint8_t *pTuningSettingBuffer,
				uint8_t UseInternalTuningSettings);
	int8_t (*GetTuningSettingBuffer)(VL53L0_DEV Dev,
				uint8_t **pTuningSettingBuffer,
				uint8_t *pUseInternalTuningSettings);
	int8_t (*StaticInit)(VL53L0_DEV Dev);
	int8_t (*WaitDeviceBooted)(VL53L0_DEV Dev);
	int8_t (*ResetDevice)(VL53L0_DEV Dev);
	int8_t (*SetDeviceParameters)(VL53L0_DEV Dev,
			const VL53L0_DeviceParameters_t *pDeviceParameters);
	int8_t (*GetDeviceParameters)(VL53L0_DEV Dev,
				VL53L0_DeviceParameters_t *pDeviceParameters);
	int8_t (*SetDeviceMode)(VL53L0_DEV Dev,
				VL53L0_DeviceModes DeviceMode);
	int8_t (*GetDeviceMode)(VL53L0_DEV Dev,
				VL53L0_DeviceModes *pDeviceMode);
	int8_t (*SetHistogramMode)(VL53L0_DEV Dev,
				VL53L0_HistogramModes HistogramMode);
	int8_t (*GetHistogramMode)(VL53L0_DEV Dev,
				VL53L0_HistogramModes *pHistogramMode);
	int8_t (*SetMeasurementTimingBudgetMicroSeconds)(VL53L0_DEV Dev,
				uint32_t  MeasurementTimingBudgetMicroSeconds);
	int8_t (*GetMeasurementTimingBudgetMicroSeconds)(
				VL53L0_DEV Dev,
				uint32_t *pMeasurementTimingBudgetMicroSeconds);
	int8_t (*GetVcselPulsePeriod)(VL53L0_DEV Dev,
				VL53L0_VcselPeriod VcselPeriodType,
				uint8_t	*pVCSELPulsePeriod);
	int8_t (*SetVcselPulsePeriod)(VL53L0_DEV Dev,
				VL53L0_VcselPeriod VcselPeriodType,
				uint8_t VCSELPulsePeriod);
	int8_t (*SetSequenceStepEnable)(VL53L0_DEV Dev,
				VL53L0_SequenceStepId SequenceStepId,
				uint8_t SequenceStepEnabled);
	int8_t (*GetSequenceStepEnable)(VL53L0_DEV Dev,
				VL53L0_SequenceStepId SequenceStepId,
				uint8_t *pSequenceStepEnabled);
	int8_t (*GetSequenceStepEnables)(VL53L0_DEV Dev,
		VL53L0_SchedulerSequenceSteps_t *pSchedulerSequenceSteps);
	int8_t (*SetSequenceStepTimeout)(VL53L0_DEV Dev,
				VL53L0_SequenceStepId SequenceStepId,
				FixPoint1616_t TimeOutMilliSecs);
	int8_t (*GetSequenceStepTimeout)(VL53L0_DEV Dev,
				VL53L0_SequenceStepId SequenceStepId,
				FixPoint1616_t *pTimeOutMilliSecs);
	int8_t (*GetNumberOfSequenceSteps)(VL53L0_DEV Dev,
				uint8_t *pNumberOfSequenceSteps);
	int8_t (*GetSequenceStepsInfo)(
				VL53L0_SequenceStepId SequenceStepId,
				char *pSequenceStepsString);
	int8_t (*SetInterMeasurementPeriodMilliSeconds)(
				VL53L0_DEV Dev,
				uint32_t InterMeasurementPeriodMilliSeconds);
	int8_t (*GetInterMeasurementPeriodMilliSeconds)(
				VL53L0_DEV Dev,
				uint32_t *pInterMeasurementPeriodMilliSeconds);
	int8_t (*SetXTalkCompensationEnable)(VL53L0_DEV Dev,
				uint8_t XTalkCompensationEnable);
	int8_t (*GetXTalkCompensationEnable)(VL53L0_DEV Dev,
				uint8_t *pXTalkCompensationEnable);
	int8_t (*SetXTalkCompensationRateMegaCps)(
				VL53L0_DEV Dev,
				FixPoint1616_t XTalkCompensationRateMegaCps);
	int8_t (*GetXTalkCompensationRateMegaCps)(
				VL53L0_DEV Dev,
				FixPoint1616_t *pXTalkCompensationRateMegaCps);
	int8_t (*GetNumberOfLimitCheck)(
				uint16_t *pNumberOfLimitCheck);
	int8_t (*GetLimitCheckInfo)(VL53L0_DEV Dev,
				uint16_t LimitCheckId, char *pLimitCheckString);
	int8_t (*SetLimitCheckEnable)(VL53L0_DEV Dev,
				uint16_t LimitCheckId,
				uint8_t LimitCheckEnable);
	int8_t (*GetLimitCheckEnable)(VL53L0_DEV Dev,
		uint16_t LimitCheckId, uint8_t *pLimitCheckEnable);
	int8_t (*SetLimitCheckValue)(VL53L0_DEV Dev,
				uint16_t LimitCheckId,
				FixPoint1616_t LimitCheckValue);
	int8_t (*GetLimitCheckValue)(VL53L0_DEV Dev,
				uint16_t LimitCheckId,
				FixPoint1616_t *pLimitCheckValue);
	int8_t (*GetLimitCheckCurrent)(VL53L0_DEV Dev,
		uint16_t LimitCheckId, FixPoint1616_t *pLimitCheckCurrent);
	int8_t (*SetWrapAroundCheckEnable)(VL53L0_DEV Dev,
				uint8_t WrapAroundCheckEnable);
	int8_t (*GetWrapAroundCheckEnable)(VL53L0_DEV Dev,
				uint8_t *pWrapAroundCheckEnable);
	int8_t (*PerformSingleMeasurement)(VL53L0_DEV Dev);
	int8_t (*PerformRefCalibration)(VL53L0_DEV Dev,
				uint8_t *pVhvSettings, uint8_t *pPhaseCal);
	int8_t (*SetRefCalibration)(VL53L0_DEV Dev,
			uint8_t VhvSettings,
			uint8_t PhaseCal);
	int8_t (*GetRefCalibration)(VL53L0_DEV Dev,
			uint8_t *pVhvSettings,
			uint8_t *pPhaseCal);
	int8_t (*PerformXTalkCalibration)(VL53L0_DEV Dev,
				FixPoint1616_t XTalkCalDistance,
				FixPoint1616_t *pXTalkCompensationRateMegaCps);
	int8_t (*PerformOffsetCalibration)(VL53L0_DEV Dev,
				FixPoint1616_t CalDistanceMilliMeter,
				int32_t *pOffsetMicroMeter);
	int8_t (*StartMeasurement)(VL53L0_DEV Dev);
	int8_t (*StopMeasurement)(VL53L0_DEV Dev);
	int8_t (*GetMeasurementDataReady)(VL53L0_DEV Dev,
				uint8_t *pMeasurementDataReady);
	int8_t (*WaitDeviceReadyForNewMeasurement)(VL53L0_DEV Dev,
				uint32_t MaxLoop);
	int8_t (*GetRangingMeasurementData)(VL53L0_DEV Dev,
		VL53L0_RangingMeasurementData_t *pRangingMeasurementData);
	int8_t (*GetHistogramMeasurementData)(VL53L0_DEV Dev,
		VL53L0_HistogramMeasurementData_t *pHistogramMeasurementData);
	int8_t (*PerformSingleRangingMeasurement)(VL53L0_DEV Dev,
		VL53L0_RangingMeasurementData_t *pRangingMeasurementData);
	int8_t (*PerformSingleHistogramMeasurement)(VL53L0_DEV Dev,
		VL53L0_HistogramMeasurementData_t *pHistogramMeasurementData);
	int8_t (*SetNumberOfROIZones)(VL53L0_DEV Dev,
				uint8_t NumberOfROIZones);
	int8_t (*GetNumberOfROIZones)(VL53L0_DEV Dev,
				uint8_t *pNumberOfROIZones);
	int8_t (*GetMaxNumberOfROIZones)(VL53L0_DEV Dev,
				uint8_t *pMaxNumberOfROIZones);
	int8_t (*SetGpioConfig)(VL53L0_DEV Dev,
				uint8_t Pin,
				VL53L0_DeviceModes DeviceMode,
				VL53L0_GpioFunctionality Functionality,
				VL53L0_InterruptPolarity Polarity);
	int8_t (*GetGpioConfig)(VL53L0_DEV Dev,
				uint8_t Pin,
				VL53L0_DeviceModes *pDeviceMode,
				VL53L0_GpioFunctionality *pFunctionality,
				VL53L0_InterruptPolarity *pPolarity);
	int8_t (*SetInterruptThresholds)(VL53L0_DEV Dev,
				VL53L0_DeviceModes DeviceMode,
				FixPoint1616_t ThresholdLow,
				FixPoint1616_t ThresholdHigh);
	int8_t (*GetInterruptThresholds)(VL53L0_DEV Dev,
				VL53L0_DeviceModes DeviceMode,
				FixPoint1616_t *pThresholdLow,
				FixPoint1616_t *pThresholdHigh);
	int8_t (*ClearInterruptMask)(VL53L0_DEV Dev,
				uint32_t InterruptMask);
	int8_t (*GetInterruptMaskStatus)(VL53L0_DEV Dev,
				uint32_t *pInterruptMaskStatus);
	int8_t (*EnableInterruptMask)(VL53L0_DEV Dev, uint32_t InterruptMask);
	int8_t (*SetSpadAmbientDamperThreshold)(VL53L0_DEV Dev,
				uint16_t SpadAmbientDamperThreshold);
	int8_t (*GetSpadAmbientDamperThreshold)(VL53L0_DEV Dev,
				uint16_t *pSpadAmbientDamperThreshold);
	int8_t (*SetSpadAmbientDamperFactor)(VL53L0_DEV Dev,
				uint16_t SpadAmbientDamperFactor);
	int8_t (*GetSpadAmbientDamperFactor)(VL53L0_DEV Dev,
				uint16_t *pSpadAmbientDamperFactor);
	int8_t (*PerformRefSpadManagement)(VL53L0_DEV Dev,
		uint32_t *refSpadCount, uint8_t *isApertureSpads);
	int8_t (*SetReferenceSpads)(VL53L0_DEV Dev,
			 uint32_t count, uint8_t isApertureSpads);
	int8_t (*GetReferenceSpads)(VL53L0_DEV Dev,
			uint32_t *pSpadCount, uint8_t *pIsApertureSpads);
	int8_t (*GetStopCompletedStatus)(VL53L0_DEV Dev,
					 uint32_t *pStopStatus);
};

static struct stmvl53l0_api_fn_t stmvl53l0_api_func_tbl = {
	.GetVersion = VL53L0_GetVersion,
	.GetPalSpecVersion = VL53L0_GetPalSpecVersion,
	.GetProductRevision = VL53L0_GetProductRevision,
	.GetDeviceInfo = VL53L0_GetDeviceInfo,
	.GetDeviceErrorStatus = VL53L0_GetDeviceErrorStatus,
	.GetRangeStatusString = VL53L0_GetRangeStatusString,
	.GetDeviceErrorString = VL53L0_GetDeviceErrorString,
	.GetPalErrorString = VL53L0_GetPalErrorString,
	.GetPalState = VL53L0_GetPalState,
	.SetPowerMode = VL53L0_SetPowerMode,
	.GetPowerMode = VL53L0_GetPowerMode,
	.SetOffsetCalibrationDataMicroMeter =
		VL53L0_SetOffsetCalibrationDataMicroMeter,
	.SetLinearityCorrectiveGain =
		VL53L0_SetLinearityCorrectiveGain,
	.GetLinearityCorrectiveGain =
		VL53L0_GetLinearityCorrectiveGain,
	.GetOffsetCalibrationDataMicroMeter =
		VL53L0_GetOffsetCalibrationDataMicroMeter,
	.SetGroupParamHold = VL53L0_SetGroupParamHold,
	.GetUpperLimitMilliMeter = VL53L0_GetUpperLimitMilliMeter,
	.SetDeviceAddress = VL53L0_SetDeviceAddress,
	.DataInit = VL53L0_DataInit,
	.SetTuningSettingBuffer = VL53L0_SetTuningSettingBuffer,
	.GetTuningSettingBuffer = VL53L0_GetTuningSettingBuffer,
	.StaticInit = VL53L0_StaticInit,
	.WaitDeviceBooted = VL53L0_WaitDeviceBooted,
	.ResetDevice = VL53L0_ResetDevice,
	.SetDeviceParameters = VL53L0_SetDeviceParameters,
	.SetDeviceMode = VL53L0_SetDeviceMode,
	.GetDeviceMode = VL53L0_GetDeviceMode,
	.SetHistogramMode = VL53L0_SetHistogramMode,
	.GetHistogramMode = VL53L0_GetHistogramMode,
	.SetMeasurementTimingBudgetMicroSeconds =
		VL53L0_SetMeasurementTimingBudgetMicroSeconds,
	.GetMeasurementTimingBudgetMicroSeconds =
		VL53L0_GetMeasurementTimingBudgetMicroSeconds,
	.GetVcselPulsePeriod = VL53L0_GetVcselPulsePeriod,
	.SetVcselPulsePeriod = VL53L0_SetVcselPulsePeriod,
	.SetSequenceStepEnable = VL53L0_SetSequenceStepEnable,
	.GetSequenceStepEnable = VL53L0_GetSequenceStepEnable,
	.GetSequenceStepEnables = VL53L0_GetSequenceStepEnables,
	.SetSequenceStepTimeout = VL53L0_SetSequenceStepTimeout,
	.GetSequenceStepTimeout = VL53L0_GetSequenceStepTimeout,
	.GetNumberOfSequenceSteps = VL53L0_GetNumberOfSequenceSteps,
	.GetSequenceStepsInfo = VL53L0_GetSequenceStepsInfo,
	.SetInterMeasurementPeriodMilliSeconds =
		VL53L0_SetInterMeasurementPeriodMilliSeconds,
	.GetInterMeasurementPeriodMilliSeconds =
		VL53L0_GetInterMeasurementPeriodMilliSeconds,
	.SetXTalkCompensationEnable = VL53L0_SetXTalkCompensationEnable,
	.GetXTalkCompensationEnable = VL53L0_GetXTalkCompensationEnable,
	.SetXTalkCompensationRateMegaCps =
		VL53L0_SetXTalkCompensationRateMegaCps,
	.GetXTalkCompensationRateMegaCps =
		VL53L0_GetXTalkCompensationRateMegaCps,
	.GetNumberOfLimitCheck = VL53L0_GetNumberOfLimitCheck,
	.GetLimitCheckInfo = VL53L0_GetLimitCheckInfo,
	.SetLimitCheckEnable = VL53L0_SetLimitCheckEnable,
	.GetLimitCheckEnable = VL53L0_GetLimitCheckEnable,
	.SetLimitCheckValue = VL53L0_SetLimitCheckValue,
	.GetLimitCheckValue = VL53L0_GetLimitCheckValue,
	.GetLimitCheckCurrent = VL53L0_GetLimitCheckCurrent,
	.SetWrapAroundCheckEnable = VL53L0_SetWrapAroundCheckEnable,
	.GetWrapAroundCheckEnable = VL53L0_GetWrapAroundCheckEnable,
	.PerformSingleMeasurement = VL53L0_PerformSingleMeasurement,
	.PerformRefCalibration = VL53L0_PerformRefCalibration,
	.SetRefCalibration = VL53L0_SetRefCalibration,
	.GetRefCalibration = VL53L0_GetRefCalibration,
	.PerformXTalkCalibration = VL53L0_PerformXTalkCalibration,
	.PerformOffsetCalibration = VL53L0_PerformOffsetCalibration,
	.StartMeasurement = VL53L0_StartMeasurement,
	.StopMeasurement = VL53L0_StopMeasurement,
	.GetMeasurementDataReady = VL53L0_GetMeasurementDataReady,
	.WaitDeviceReadyForNewMeasurement =
		VL53L0_WaitDeviceReadyForNewMeasurement,
	.GetRangingMeasurementData = VL53L0_GetRangingMeasurementData,
	.GetHistogramMeasurementData = VL53L0_GetHistogramMeasurementData,
	.PerformSingleRangingMeasurement =
		VL53L0_PerformSingleRangingMeasurement,
	.PerformSingleHistogramMeasurement =
		VL53L0_PerformSingleHistogramMeasurement,
	.SetNumberOfROIZones = VL53L0_SetNumberOfROIZones,
	.GetNumberOfROIZones = VL53L0_GetNumberOfROIZones,
	.GetMaxNumberOfROIZones = VL53L0_GetMaxNumberOfROIZones,
	.SetGpioConfig = VL53L0_SetGpioConfig,
	.GetGpioConfig = VL53L0_GetGpioConfig,
	.SetInterruptThresholds = VL53L0_SetInterruptThresholds,
	.GetInterruptThresholds = VL53L0_GetInterruptThresholds,
	.ClearInterruptMask = VL53L0_ClearInterruptMask,
	.GetInterruptMaskStatus = VL53L0_GetInterruptMaskStatus,
	.EnableInterruptMask = VL53L0_EnableInterruptMask,
	.SetSpadAmbientDamperThreshold = VL53L0_SetSpadAmbientDamperThreshold,
	.GetSpadAmbientDamperThreshold = VL53L0_GetSpadAmbientDamperThreshold,
	.SetSpadAmbientDamperFactor = VL53L0_SetSpadAmbientDamperFactor,
	.GetSpadAmbientDamperFactor = VL53L0_GetSpadAmbientDamperFactor,
	.PerformRefSpadManagement = VL53L0_PerformRefSpadManagement,
	.SetReferenceSpads = VL53L0_SetReferenceSpads,
	.GetReferenceSpads = VL53L0_GetReferenceSpads,
	.GetStopCompletedStatus = VL53L0_GetStopCompletedStatus,

};
struct stmvl53l0_api_fn_t *papi_func_tbl;



/* Mask fields to indicate Offset and Xtalk Comp
 * values have been set by application
 */
#define SET_OFFSET_CALIB_DATA_MICROMETER_MASK 0x1
#define SET_XTALK_COMP_RATE_MCPS_MASK         0x2

/* Macros used across different functions */
#define USE_CASE_LONG_DISTANCE	1
#define USE_CASE_HIGH_ACCURACY	2
#define USE_CASE_HIGH_SPEED		3
#define USE_CASE_CUSTOM			4

#define LONG_DISTANCE_TIMING_BUDGET			33000
#define LONG_DISTANCE_SIGNAL_RATE_LIMIT		(25 * 65536 / 100) /* 0.1 * 65536  */
#define LONG_DISTANCE_SIGMA_LIMIT			(60*65536)
#define LONG_DISTANCE_PRE_RANGE_PULSE_PERIOD	18
#define LONG_DISTANCE_FINAL_RANGE_PULSE_PERIOD	14



#define HIGH_ACCURACY_TIMING_BUDGET				200000
#define HIGH_ACCURACY_SIGNAL_RATE_LIMIT	 (25 * 65536 / 100) /* 0.25 * 65536 */
#define HIGH_ACCURACY_SIGMA_LIMIT				(18*65536)
#define HIGH_ACCURACY_PRE_RANGE_PULSE_PERIOD	14
#define HIGH_ACCURACY_FINAL_RANGE_PULSE_PERIOD	10



#define HIGH_SPEED_TIMING_BUDGET				20000
#define HIGH_SPEED_SIGNAL_RATE_LIMIT	(25 * 65536 / 100) /* 0.25 * 65536 */
#define HIGH_SPEED_SIGMA_LIMIT					(32*65536)
#define HIGH_SPEED_PRE_RANGE_PULSE_PERIOD		14
#define HIGH_SPEED_FINAL_RANGE_PULSE_PERIOD		10

#define LASER_PRINTCYLE 30 //for print laser data

static int stmvl53l0_laser_init(struct stmvl53l0_data *data);

static int stmvl53l0_laser_parameter(struct stmvl53l0_data *data, hwlaser_parameter_t* param);

static int stmvl53l0_init_client(struct stmvl53l0_data *data);

static int stmvl53l0_config_use_case(struct stmvl53l0_data *data);

static int stmvl53l0_laser_get_data(struct stmvl53l0_data *data, void* p);

static int stmvl53l0_stop(struct stmvl53l0_data *data);

static int stmvl53l0_start(struct stmvl53l0_data *data);

extern void laser_notify_data_event(hw_laser_ctrl_t *ctrl, const struct v4l2_event *ev);

static void stmvl53l0_setupAPIFunctions(struct stmvl53l0_data *data)
{
	uint8_t revision = 0;
	VL53L0_DEV vl53l0_dev = data;

	/* Read Revision ID */
    if(NULL == data)
    {
        vl53l0_errmsg("input data is NULL !!");
        return;
    }
	VL53L0_RdByte(vl53l0_dev,
				VL53L0_REG_IDENTIFICATION_REVISION_ID,
				&revision);
	vl53l0_errmsg("read REVISION_ID: 0x%x\n", revision);
	revision = (revision & 0xF0) >> 4;
	if (revision == 1) {
		/*cut 1.1*/
		vl53l0_errmsg("to setup API cut 1.1\n");
		papi_func_tbl->GetVersion = VL53L0_GetVersion;
		papi_func_tbl->GetPalSpecVersion = VL53L0_GetPalSpecVersion;
		papi_func_tbl->GetProductRevision = VL53L0_GetProductRevision;
		papi_func_tbl->GetDeviceInfo = VL53L0_GetDeviceInfo;
		papi_func_tbl->GetDeviceErrorStatus =
			 VL53L0_GetDeviceErrorStatus;
		papi_func_tbl->GetRangeStatusString =
			 VL53L0_GetRangeStatusString;
		papi_func_tbl->GetDeviceErrorString =
			 VL53L0_GetDeviceErrorString;
		papi_func_tbl->GetPalErrorString =
			 VL53L0_GetPalErrorString;
		papi_func_tbl->GetPalState =
			 VL53L0_GetPalState;
		papi_func_tbl->SetPowerMode =
			 VL53L0_SetPowerMode;
		papi_func_tbl->GetPowerMode =
			 VL53L0_GetPowerMode;
		papi_func_tbl->SetOffsetCalibrationDataMicroMeter =
			 VL53L0_SetOffsetCalibrationDataMicroMeter;
		papi_func_tbl->GetOffsetCalibrationDataMicroMeter =
			 VL53L0_GetOffsetCalibrationDataMicroMeter;
		papi_func_tbl->SetLinearityCorrectiveGain =
			 VL53L0_SetLinearityCorrectiveGain;
		papi_func_tbl->GetLinearityCorrectiveGain =
			 VL53L0_GetLinearityCorrectiveGain;
		papi_func_tbl->SetGroupParamHold = VL53L0_SetGroupParamHold;
		papi_func_tbl->GetUpperLimitMilliMeter =
			 VL53L0_GetUpperLimitMilliMeter;
		papi_func_tbl->SetDeviceAddress =
			 VL53L0_SetDeviceAddress;
		papi_func_tbl->DataInit =
			 VL53L0_DataInit;
		papi_func_tbl->SetTuningSettingBuffer =
			 VL53L0_SetTuningSettingBuffer;
		papi_func_tbl->GetTuningSettingBuffer =
			 VL53L0_GetTuningSettingBuffer;
		papi_func_tbl->StaticInit =
			 VL53L0_StaticInit;
		papi_func_tbl->WaitDeviceBooted =
			 VL53L0_WaitDeviceBooted;
		papi_func_tbl->ResetDevice =
			 VL53L0_ResetDevice;
		papi_func_tbl->SetDeviceParameters =
			 VL53L0_SetDeviceParameters;
		papi_func_tbl->SetDeviceMode = VL53L0_SetDeviceMode;
		papi_func_tbl->GetDeviceMode = VL53L0_GetDeviceMode;
		papi_func_tbl->SetHistogramMode = VL53L0_SetHistogramMode;
		papi_func_tbl->GetHistogramMode = VL53L0_GetHistogramMode;
		papi_func_tbl->SetMeasurementTimingBudgetMicroSeconds =
			 VL53L0_SetMeasurementTimingBudgetMicroSeconds;
		papi_func_tbl->GetMeasurementTimingBudgetMicroSeconds =
			 VL53L0_GetMeasurementTimingBudgetMicroSeconds;
		papi_func_tbl->GetVcselPulsePeriod = VL53L0_GetVcselPulsePeriod;
		papi_func_tbl->SetVcselPulsePeriod = VL53L0_SetVcselPulsePeriod;
		papi_func_tbl->SetSequenceStepEnable =
				 VL53L0_SetSequenceStepEnable;
		papi_func_tbl->GetSequenceStepEnable =
				 VL53L0_GetSequenceStepEnable;
		papi_func_tbl->GetSequenceStepEnables =
				 VL53L0_GetSequenceStepEnables;
		papi_func_tbl->SetSequenceStepTimeout =
				 VL53L0_SetSequenceStepTimeout;
		papi_func_tbl->GetSequenceStepTimeout =
				 VL53L0_GetSequenceStepTimeout;
		papi_func_tbl->GetNumberOfSequenceSteps =
				 VL53L0_GetNumberOfSequenceSteps;
		papi_func_tbl->GetSequenceStepsInfo =
				 VL53L0_GetSequenceStepsInfo;
		papi_func_tbl->SetInterMeasurementPeriodMilliSeconds =
				 VL53L0_SetInterMeasurementPeriodMilliSeconds;
		papi_func_tbl->GetInterMeasurementPeriodMilliSeconds =
				VL53L0_GetInterMeasurementPeriodMilliSeconds;
		papi_func_tbl->SetXTalkCompensationEnable =
				 VL53L0_SetXTalkCompensationEnable;
		papi_func_tbl->GetXTalkCompensationEnable =
				 VL53L0_GetXTalkCompensationEnable;
		papi_func_tbl->SetXTalkCompensationRateMegaCps =
				 VL53L0_SetXTalkCompensationRateMegaCps;
		papi_func_tbl->GetXTalkCompensationRateMegaCps =
				 VL53L0_GetXTalkCompensationRateMegaCps;
		papi_func_tbl->GetNumberOfLimitCheck =
				 VL53L0_GetNumberOfLimitCheck;
		papi_func_tbl->GetLimitCheckInfo =
				 VL53L0_GetLimitCheckInfo;
		papi_func_tbl->SetLimitCheckEnable =
				 VL53L0_SetLimitCheckEnable;
		papi_func_tbl->GetLimitCheckEnable =
				 VL53L0_GetLimitCheckEnable;
		papi_func_tbl->SetLimitCheckValue =
				 VL53L0_SetLimitCheckValue;
		papi_func_tbl->GetLimitCheckValue =
				 VL53L0_GetLimitCheckValue;
		papi_func_tbl->GetLimitCheckCurrent =
				 VL53L0_GetLimitCheckCurrent;
		papi_func_tbl->SetWrapAroundCheckEnable =
				 VL53L0_SetWrapAroundCheckEnable;
		papi_func_tbl->GetWrapAroundCheckEnable =
				 VL53L0_GetWrapAroundCheckEnable;
		papi_func_tbl->PerformSingleMeasurement =
				 VL53L0_PerformSingleMeasurement;
		papi_func_tbl->PerformRefCalibration =
				 VL53L0_PerformRefCalibration;
		papi_func_tbl->SetRefCalibration =
				 VL53L0_SetRefCalibration;
		papi_func_tbl->GetRefCalibration =
					 VL53L0_GetRefCalibration;
		papi_func_tbl->PerformXTalkCalibration =
				 VL53L0_PerformXTalkCalibration;
		papi_func_tbl->PerformOffsetCalibration =
				 VL53L0_PerformOffsetCalibration;
		papi_func_tbl->StartMeasurement =
				 VL53L0_StartMeasurement;
		papi_func_tbl->StopMeasurement =
				 VL53L0_StopMeasurement;
		papi_func_tbl->GetMeasurementDataReady =
				 VL53L0_GetMeasurementDataReady;
		papi_func_tbl->WaitDeviceReadyForNewMeasurement =
			VL53L0_WaitDeviceReadyForNewMeasurement;
		papi_func_tbl->GetRangingMeasurementData =
			VL53L0_GetRangingMeasurementData;
		papi_func_tbl->GetHistogramMeasurementData =
			VL53L0_GetHistogramMeasurementData;
		papi_func_tbl->PerformSingleRangingMeasurement =
			VL53L0_PerformSingleRangingMeasurement;
		papi_func_tbl->PerformSingleHistogramMeasurement =
			VL53L0_PerformSingleHistogramMeasurement;
		papi_func_tbl->SetNumberOfROIZones =
			 VL53L0_SetNumberOfROIZones;
		papi_func_tbl->GetNumberOfROIZones =
			VL53L0_GetNumberOfROIZones;
		papi_func_tbl->GetMaxNumberOfROIZones =
			 VL53L0_GetMaxNumberOfROIZones;
		papi_func_tbl->SetGpioConfig =
			 VL53L0_SetGpioConfig;
		papi_func_tbl->GetGpioConfig =
			 VL53L0_GetGpioConfig;
		papi_func_tbl->SetInterruptThresholds =
			 VL53L0_SetInterruptThresholds;
		papi_func_tbl->GetInterruptThresholds =
			 VL53L0_GetInterruptThresholds;
		papi_func_tbl->ClearInterruptMask =
			 VL53L0_ClearInterruptMask;
		papi_func_tbl->GetInterruptMaskStatus =
				 VL53L0_GetInterruptMaskStatus;
		papi_func_tbl->EnableInterruptMask = VL53L0_EnableInterruptMask;
		papi_func_tbl->SetSpadAmbientDamperThreshold =
			VL53L0_SetSpadAmbientDamperThreshold;
		papi_func_tbl->GetSpadAmbientDamperThreshold =
			VL53L0_GetSpadAmbientDamperThreshold;
		papi_func_tbl->SetSpadAmbientDamperFactor =
			VL53L0_SetSpadAmbientDamperFactor;
		papi_func_tbl->GetSpadAmbientDamperFactor =
			VL53L0_GetSpadAmbientDamperFactor;
		papi_func_tbl->PerformRefSpadManagement =
			 VL53L0_PerformRefSpadManagement;
		papi_func_tbl->SetReferenceSpads = VL53L0_SetReferenceSpads;
		papi_func_tbl->GetReferenceSpads = VL53L0_GetReferenceSpads;
	} else if (revision == 0) {
		/*cut 1.0*/
		vl53l0_errmsg("API cut 1.0 NOT SUPPORTED\n");
	}

}



static void stmvl53l0_cancel_handler(struct stmvl53l0_data *data)
{
    unsigned long flags;
    bool ret;

    if(NULL == data)
    {
        vl53l0_errmsg("input data is NULL!!");
        return;
    }
    spin_lock_irqsave(&data->update_lock.wait_lock, flags);
    /*
     * If work is already scheduled then subsequent schedules will not
     * change the scheduled time that's why we have to cancel it first.
     */
    ret = cancel_delayed_work(&data->dwork);
    if (ret == 0)
        vl53l0_errmsg("cancel_delayed_work return FALSE\n");

    spin_unlock_irqrestore(&data->update_lock.wait_lock, flags);

}

void stmvl53l0_schedule_handler(struct stmvl53l0_data *data)
{
    unsigned long flags;
    if(NULL == data)
    {
        vl53l0_errmsg("input data is NULL!!");
        return;
    }
    spin_lock_irqsave(&data->update_lock.wait_lock, flags);
    /*
     * If work is already scheduled then subsequent schedules will not
     * change the scheduled time that's why we have to cancel it first.
     */
    cancel_delayed_work(&data->dwork);
    schedule_delayed_work(&data->dwork, 0);
    spin_unlock_irqrestore(&data->update_lock.wait_lock, flags);

}

#ifdef USE_INT
static irqreturn_t stmvl53l0_interrupt_handler(int vec, void *info)
{

	struct stmvl53l0_data *data;
    if(NULL == info)
        return IRQ_NONE; 
	data = (struct stmvl53l0_data *)info;

	if (data->irq == vec) {
		data->interrupt_received = 1;
		schedule_delayed_work(&data->dwork, 0);
	}
	return IRQ_HANDLED;
}
#else
/* Flag used to exit the thread when kthread_stop() is invoked */
static int poll_thread_exit = 0;
int stmvl53l0_poll_thread(void *data)
{
	VL53L0_DEV vl53l0_dev = data;
	VL53L0_Error Status = VL53L0_ERROR_NONE;
	uint32_t sleep_time = 0;
	uint32_t interruptStatus = 0;
	pr_err("%s(%d) : Starting Polling thread\n", __func__, __LINE__);
	while (!kthread_should_stop()) {
		if (poll_thread_exit) {
			pr_err(
		"%s(%d) : Exiting the poll thread\n", __func__, __LINE__);
			break;
		}
		wait_event(vl53l0_dev->poll_thread_wq,
			(vl53l0_dev->enable_ps_sensor));
		mutex_lock(&vl53l0_dev->work_mutex);
		sleep_time = vl53l0_dev->delay_ms;
		Status = VL53L0_GetInterruptMaskStatus(vl53l0_dev,
					 &interruptStatus);
            if(vl53l0_dev->print_count%LASER_PRINTCYLE == 0)
            {
                vl53l0_dbgmsg("laser poll status=%d, old status =%d, ret=%d\n",
                    interruptStatus, vl53l0_dev->interruptStatus, Status);
            }
		if (Status == VL53L0_ERROR_NONE &&
			interruptStatus &&
			interruptStatus != vl53l0_dev->interruptStatus) {
			vl53l0_dev->interruptStatus = interruptStatus;
			vl53l0_dev->noInterruptCount = 0;
			stmvl53l0_schedule_handler(vl53l0_dev);
		} else {
			vl53l0_dev->noInterruptCount++;
		}
		if ((vl53l0_dev->noInterruptCount * vl53l0_dev->delay_ms) >
			 (vl53l0_dev->timingBudget * 2)) {
			pr_err("No interrupt after (%u) msec(TimingBudget = %u) . Clear Interrupt Mask and restart\n",
				(vl53l0_dev->noInterruptCount *
					 vl53l0_dev->delay_ms),
					vl53l0_dev->timingBudget);
			Status = papi_func_tbl->ClearInterruptMask(vl53l0_dev,
								   0);
			vl53l0_dev->noInterruptCount = 0;
			if (vl53l0_dev->deviceMode ==
			 VL53L0_DEVICEMODE_SINGLE_RANGING) {
				Status = papi_func_tbl->StartMeasurement(
							vl53l0_dev);
				if (Status != VL53L0_ERROR_NONE) {
					pr_err("%s(%d) : Status = %d\n",
						 __func__, __LINE__, Status);
				}
			}
		}
		mutex_unlock(&vl53l0_dev->work_mutex);
		msleep(sleep_time);
	}
	return 0;
}
#endif

static void stmvl53l0_notify_data_event(struct stmvl53l0_data *data)
{
    struct i2c_data *client_object = NULL;
    struct v4l2_event ev; // NULL event
    if (data == NULL \
        || data->client_object == NULL)
    {
        vl53l0_errmsg("notify data event failed\n");
        return;
    }

    client_object = (struct i2c_data *) data->client_object;
    if (client_object == NULL)
    {
        vl53l0_errmsg("notify data event failed, i2c_data NULL\n");
        return;
    }

    ev.type = V4L2_EVENT_CTRL;
    ev.id = 0;
    laser_notify_data_event(client_object->ctrl, &ev);
}

/* work handler */
static void stmvl53l0_work_handler(struct work_struct *work)
{
    struct stmvl53l0_data *data = NULL;
    VL53L0_DEV vl53l0_dev;

    VL53L0_Error Status = VL53L0_ERROR_NONE;
    VL53L0_RangingMeasurementData_t tmp_rangeData;
    memset_s(&tmp_rangeData, sizeof(VL53L0_RangingMeasurementData_t), 0, sizeof(VL53L0_RangingMeasurementData_t));

    if(NULL == work) {
        vl53l0_errmsg("work queue is null\n");
        return;
    }
    data = container_of(work, struct stmvl53l0_data,
                    dwork.work);
    vl53l0_dev = data;
    mutex_lock(&data->work_mutex);

    if (vl53l0_dev->enable_ps_sensor == 1) {
        /* Check if ISR has scheduled this function */
        if (vl53l0_dev->interrupt_received == 1) {
            Status = papi_func_tbl->GetInterruptMaskStatus(
                            vl53l0_dev,
                            &vl53l0_dev->interruptStatus);
        if (Status != VL53L0_ERROR_NONE)
            pr_err("%s(%d) : Status = %d\n",
                        __func__, __LINE__, Status);
            vl53l0_dev->interrupt_received = 0;
        }
        if (vl53l0_dev->print_count%LASER_PRINTCYLE == 0)
        {
            vl53l0_dbgmsg("interruptStatus:0x%x, gpio_function:%d, interrupt_received:%d\n",
                vl53l0_dev->interruptStatus,vl53l0_dev->gpio_function, vl53l0_dev->interrupt_received);
        }
        if (vl53l0_dev->interruptStatus == vl53l0_dev->gpio_function) {
            Status = papi_func_tbl->GetRangingMeasurementData(
            vl53l0_dev, &tmp_rangeData);
            if (Status != VL53L0_ERROR_NONE)
                pr_err("%s(%d) : Status = %d\n", __func__, __LINE__, Status);
            /* to push the measurement */
            Status = papi_func_tbl->ClearInterruptMask(vl53l0_dev, 0);
            if (Status != VL53L0_ERROR_NONE) {
                pr_err("%s(%d) : Status = %d\n",
                            __func__, __LINE__, Status);
            }
            if(vl53l0_dev->print_count%LASER_PRINTCYLE == 0)
            {
                vl53l0_dbgmsg("laser work handler data=%d\n", tmp_rangeData.RangeMilliMeter);
            }
            mutex_lock(&data->data_mutex);
            memcpy_s(&(data->rangeData), sizeof(VL53L0_RangingMeasurementData_t), &tmp_rangeData, sizeof(VL53L0_RangingMeasurementData_t));
            mutex_unlock(&data->data_mutex);
            // notify laser event;
            stmvl53l0_notify_data_event(data);
        }
    }
    vl53l0_dev->interruptStatus = 0;
    mutex_unlock(&data->work_mutex);
}

static int stmvl53l0_status(struct stmvl53l0_data* data, hwlaser_status_t *laser_status)
{
    int rc = -EINVAL;
    if(NULL == data || NULL == laser_status)
        return rc;

    if(data->init_flag == 1){
        strncpy_s(laser_status->name, strlen(stmvl53l0_name), stmvl53l0_name, strlen(stmvl53l0_name));
        laser_status->status = 0;
    }else {
        laser_status->status = -1;
    }
    rc = laser_status->status;
    return rc;
}

static int stmvl53l0_perform_cal(struct stmvl53l0_data* data, hwlaser_ioctl_perform_calibration_t* cal)
{
    int rc = -EINVAL;
    FixPoint1616_t XTalkCompensationRateMegaCps;
    int32_t OffsetMicroMeter;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    uint32_t refSpadCount;
    uint8_t isApertureSpads;

    if(NULL == data || NULL == cal)
        return rc;

    switch(cal->calibration_type){
        case HWLASER_CALIBRATION_CROSSTALK:
            data->xtalkCalDistance = cal->param1;
            rc = papi_func_tbl->PerformXTalkCalibration(data, (data->xtalkCalDistance<<16),&XTalkCompensationRateMegaCps);
            if (rc != VL53L0_ERROR_NONE) {
                vl53l0_errmsg("%d- error perform xtalk %d\n", __LINE__, rc);
            }
            vl53l0_dbgmsg("Xtalk calibration:%d\n",XTalkCompensationRateMegaCps);
            data->XTalkCompensationRateMegaCps = XTalkCompensationRateMegaCps;
            data->setCalibratedValue |= SET_XTALK_COMP_RATE_MCPS_MASK;
            rc = 0;
            break;
        case HWLASER_CALIBRATION_OFFSET:
            data->offsetCalDistance = cal->param1;
            rc = papi_func_tbl->PerformOffsetCalibration(data,(data->offsetCalDistance<<16), &OffsetMicroMeter);
            if (rc != VL53L0_ERROR_NONE) {
                vl53l0_errmsg("%d- error perform offset cal %d\n", __LINE__, rc);
            }
            vl53l0_dbgmsg("Offset calibration:%d\n", OffsetMicroMeter);
            data->OffsetMicroMeter = OffsetMicroMeter;
            data->setCalibratedValue |= SET_OFFSET_CALIB_DATA_MICROMETER_MASK;
            rc = 0;
            break;
        case HWLASER_CALIBRATION_REFERENCE:
            rc = papi_func_tbl->PerformRefCalibration(data,
                &VhvSettings, &PhaseCal); /* Ref calibration */
            if (rc != VL53L0_ERROR_NONE) {
                vl53l0_errmsg("%d- error status %d\n", __LINE__, rc);
            }
            vl53l0_dbgmsg("VHV = %u, PhaseCal = %u\n", VhvSettings, PhaseCal);
            data->VhvSettings = VhvSettings;
            data->PhaseCal = PhaseCal;
            break;
        case HWLASER_CALIBRATION_SPAD:
            rc = papi_func_tbl->PerformRefSpadManagement(data, &refSpadCount,&isApertureSpads); /* Ref Spad Management */
            if (rc != VL53L0_ERROR_NONE) {
                vl53l0_errmsg("%d- error status %d\n", __LINE__, rc);
            }
            vl53l0_dbgmsg("SpadCount = %u, isAperature = %u\n",refSpadCount, isApertureSpads);
            data->refSpadCount = refSpadCount;
            data->isApertureSpads = isApertureSpads;
            break;
         default:
            rc = 0;
            break;
    }
    return rc;
}

static int stmvl53l0_laser_get_set_cal_data(struct stmvl53l0_data* data, hwlaser_calibration_data_t *p)
{
    VL53L0_Error rc = VL53L0_ERROR_NONE;
    FixPoint1616_t *xtalk;
    int32_t *offset;
    uint32_t *pspadcount;
    uint8_t *pspadmod;
    uint8_t *pvhvset;
    uint8_t *pphascal;
    hwlaser_calibration_data_t *cal_data = (hwlaser_calibration_data_t*)p;
    if(!data || !p)
        return -EINVAL;
    offset = &(cal_data->u.dataL0.offset);
    xtalk = &(cal_data->u.dataL0.xtalk);
    pspadcount = &(cal_data->u.dataL0.spadcount);
    pspadmod = &(cal_data->u.dataL0.spadmod);
    pvhvset = &(cal_data->u.dataL0.vhvset);
    pphascal = &(cal_data->u.dataL0.phascal);

    if(!cal_data->is_read)
    {
        vl53l0_dbgmsg("laser set offset=%d, xtalk=%d\n", *offset, *xtalk);
        rc = papi_func_tbl->SetReferenceSpads(data, *pspadcount, *pspadmod);
        if(rc != VL53L0_ERROR_NONE)
        {
            vl53l0_errmsg("set ref spad cal fail!!");
        }
        rc = papi_func_tbl->SetRefCalibration(data, *pvhvset, *pphascal);
        if(rc != VL53L0_ERROR_NONE)
        {
            vl53l0_errmsg("set ref cal  fail!!");
        }

        rc = papi_func_tbl->SetOffsetCalibrationDataMicroMeter(data, *offset);
        if(rc != VL53L0_ERROR_NONE)
        {
            vl53l0_errmsg("set offset cal fail!!");
        }

        rc = papi_func_tbl->SetXTalkCompensationEnable(data, 1);
        if (rc != VL53L0_ERROR_NONE) {
            vl53l0_errmsg("%d- error setxtalk %d\n", __LINE__, rc);
        }
        data->XTalkCompensationRateMegaCps = *xtalk;
        rc = papi_func_tbl->SetXTalkCompensationRateMegaCps(data,*xtalk);
        if(rc != VL53L0_ERROR_NONE)
        {
            vl53l0_errmsg("set xtalk  fail!!");
        }

    }
    else
    {
        vl53l0_dbgmsg("laser get offset, xtalk value\n");
        *xtalk = data->XTalkCompensationRateMegaCps;
        *offset = data->OffsetMicroMeter;
        *pspadcount = data->refSpadCount;
        *pspadmod = data->isApertureSpads;
        *pvhvset = data->VhvSettings;
        *pphascal = data->PhaseCal;
    }
    return (int)rc;
}

int stmvl53l0_power_on_off(struct stmvl53l0_data *data, int is_on)
{
    int rc = 0;
    struct i2c_data *i2cdata = NULL;
    if(NULL != data && NULL != data->client_object) {
        i2cdata = (struct i2c_data *) data->client_object;
        vl53l0_dbgmsg("Enter, on_off=%d\n", is_on);
        mutex_lock(&data->work_mutex);
        rc = gpio_direction_output(i2cdata->xsdn_gpio, is_on);
        if (rc) {
            vl53l0_errmsg("fail to power control laser %d\n", rc);
        }
        if(is_on == 0)
        {
            data->enable_ps_sensor = 0;
            data->interruptStatus = 0;
        }
        mutex_unlock(&data->work_mutex);
        msleep(2);
        vl53l0_dbgmsg("End, on_off=%d\n", is_on);
    }else{
        vl53l0_errmsg("param is NULL");
        rc = -EINVAL;
    }
    return rc;
}

long stmvl53l0_laser_ioctl(void *hw_data, unsigned int cmd, void  *p)
{
    struct stmvl53l0_data *data = (struct stmvl53l0_data*)hw_data;
    hwlaser_calibration_data_t *cal_data = NULL;
    hwlaser_parameter_t *param = NULL;
    hwlaser_status_t* status = NULL;
    hwlaser_ioctl_perform_calibration_t* cal_perform = NULL;
    int rc = 0;
    hwlaser_info_t *pinfo = NULL;

    if (!data)
        return -EINVAL;

    switch (cmd) {
        case HWLASER_IOCTL_GET_INFO:
            pinfo = (hwlaser_info_t *)p;
            strncpy_s(pinfo->name, HWLASER_NAME_SIZE-1 ,"vl53l0", HWLASER_NAME_SIZE-1);
            strncpy_s(pinfo->product_name, HWLASER_NAME_SIZE-1 ,"ALPS", HWLASER_NAME_SIZE-1);
            pinfo->version = HWLASER_L0_VERSION;
            pinfo->ap_pos = HWLASER_POS_AP;
            break;
        case HWLASER_IOCTL_CONFIG:
            break;
        case HWLASER_IOCTL_CALIBRATION_DATA:
            cal_data = (hwlaser_calibration_data_t *)p;
            rc = stmvl53l0_laser_get_set_cal_data(data,cal_data);
            break;
        case HWLASER_IOCTL_PERFORM_CALIBRATION:
            cal_perform = (hwlaser_ioctl_perform_calibration_t*) p;
            rc = stmvl53l0_perform_cal(data, cal_perform);
            vl53l0_dbgmsg("perform calibration stub\n");
            break;
        case HWLASER_IOCTL_POWERON:
            rc = stmvl53l0_power_on_off(data, true);
            break;
        case HWLASER_IOCTL_POWEROFF:
            rc = stmvl53l0_power_on_off(data, false);
            break;
        case HWLASER_IOCTL_START:
            rc = stmvl53l0_start(data);
            break;
        case HWLASER_IOCTL_STOP:
            rc = stmvl53l0_stop(data);
            break;
        case HWLASER_IOCTL_INIT:
            rc = stmvl53l0_laser_init(data);
            break;
        case HWLASER_IOCTL_ROI:
            vl53l0_dbgmsg("set roi not support");
            break;
        case HWLASER_IOCTL_PARAMETER:
            param = (hwlaser_parameter_t *)p;
            rc = stmvl53l0_laser_parameter(data,param);
            break;
        case HWLASER_IOCTL_MZ_DATA:
            rc = stmvl53l0_laser_get_data(data,p);

            break;
        case HWLASER_IOCTL_MATCHID:
            status = (hwlaser_status_t*) p;
            rc = stmvl53l0_status(data, status);
            break;
        default:
            vl53l0_dbgmsg("cmd not support!!");
            rc = -EINVAL;
            break;
    }
    return rc;
}

/*
 * Initialization function
 */
static int stmvl53l0_init_client(struct stmvl53l0_data *data)
{
    VL53L0_Error Status = VL53L0_ERROR_NONE;
    VL53L0_DEV vl53l0_dev = data;

    if(NULL == data)
    {
        vl53l0_errmsg("input data is NULL!!");
        return -EINVAL;
    }
    data->I2cDevAddr      = 0x52;
    data->comms_type      = 1;
    data->comms_speed_khz = 400;

    /* Setup API functions based on revision */
    stmvl53l0_setupAPIFunctions(data);

    /* Perform Ref and RefSpad calibrations and save the values */
    if (data->reset) {
        pr_info("Call of VL53L0_DataInit\n");
        /* Data initialization */
        Status = papi_func_tbl->DataInit(vl53l0_dev);
        /* data->reset = 0; */
        if (Status != VL53L0_ERROR_NONE) {
            vl53l0_errmsg( "%d- error status %d\n", __LINE__, Status);
            return Status;
    	}

    }
    /* Device Initialization */
    vl53l0_dbgmsg("Call of VL53L0_StaticInit\n");
    Status = papi_func_tbl->StaticInit(vl53l0_dev);
    if (Status != VL53L0_ERROR_NONE) {
        vl53l0_errmsg("%d- error status %d\n", __LINE__, Status);
        return Status;
    }
    
	return 0;
}


static int stmvl53l0_config_use_case(struct stmvl53l0_data *data)
{
	VL53L0_DEV		vl53l0_dev = data;
	VL53L0_Error	Status = VL53L0_ERROR_NONE;
	FixPoint1616_t	 signalRateLimit = 0;
	FixPoint1616_t	sigmaLimit = 0;
	uint32_t		preRangePulsePeriod = 0;
	uint32_t		finalRangePulsePeriod = 0;

    vl53l0_dbgmsg("Enter,  usecase = %d\n", data->useCase);
    mutex_lock(&data->work_mutex);
	switch (vl53l0_dev->useCase) {

	case USE_CASE_LONG_DISTANCE:
		sigmaLimit			= LONG_DISTANCE_SIGMA_LIMIT;
		signalRateLimit		= LONG_DISTANCE_SIGNAL_RATE_LIMIT;
		preRangePulsePeriod	= LONG_DISTANCE_PRE_RANGE_PULSE_PERIOD;
		finalRangePulsePeriod = LONG_DISTANCE_FINAL_RANGE_PULSE_PERIOD;
		break;

	case USE_CASE_HIGH_ACCURACY:
		sigmaLimit		= HIGH_ACCURACY_SIGMA_LIMIT;
		signalRateLimit	= HIGH_ACCURACY_SIGNAL_RATE_LIMIT;
		preRangePulsePeriod	= HIGH_ACCURACY_PRE_RANGE_PULSE_PERIOD;
		finalRangePulsePeriod = HIGH_ACCURACY_FINAL_RANGE_PULSE_PERIOD;
		break;

	case USE_CASE_HIGH_SPEED:
		sigmaLimit		= HIGH_SPEED_SIGMA_LIMIT;
		signalRateLimit	= HIGH_SPEED_SIGNAL_RATE_LIMIT;
		preRangePulsePeriod	= HIGH_SPEED_PRE_RANGE_PULSE_PERIOD;
		finalRangePulsePeriod = HIGH_SPEED_FINAL_RANGE_PULSE_PERIOD;
		break;

	case USE_CASE_CUSTOM:
		/* Set by application through IOCTL interface */
		sigmaLimit			= vl53l0_dev->sigmaLimit;
		signalRateLimit		= vl53l0_dev->signalRateLimit;
		preRangePulsePeriod = vl53l0_dev->preRangePulsePeriod;
		finalRangePulsePeriod = vl53l0_dev->finalRangePulsePeriod;
		break;

	default:
		vl53l0_errmsg(
			"Invalid use case = %d\n", vl53l0_dev->useCase);
            mutex_unlock(&data->work_mutex);
		/* Invalid parameter, should not reach here */
		return -EINVAL;
	}

    vl53l0_dev->sigmaLimit = sigmaLimit;
    vl53l0_dev->signalRateLimit = signalRateLimit;
    vl53l0_dev->preRangePulsePeriod = preRangePulsePeriod;
    vl53l0_dev->finalRangePulsePeriod = finalRangePulsePeriod;

	vl53l0_dbgmsg(
		"Configure UseCase(%d) : Sigma=%u,Signal=%u,Pre=%u,Final=%u,timingBudget=%u\n",
					vl53l0_dev->useCase,
					sigmaLimit,
					signalRateLimit,
					preRangePulsePeriod,
					finalRangePulsePeriod,
					vl53l0_dev->timingBudget);

	if (papi_func_tbl->SetLimitCheckEnable != NULL) {
		Status = papi_func_tbl->SetLimitCheckEnable(
			vl53l0_dev,
			VL53L0_CHECKENABLE_SIGMA_FINAL_RANGE,
			 1);
	}

	if (Status == VL53L0_ERROR_NONE) {
	    if (papi_func_tbl->SetLimitCheckEnable != NULL) {
		    Status = papi_func_tbl->SetLimitCheckEnable(
			    vl53l0_dev,
			    VL53L0_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
			    1);
        }
	} else {
		vl53l0_errmsg(
		"SetLimitCheckEnable(SIGMA_FINAL_RANGE) failed with errcode = %d\n",
		 Status);
	}

	if (Status == VL53L0_ERROR_NONE) {
		Status = papi_func_tbl->SetLimitCheckValue(
				vl53l0_dev,
				VL53L0_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
				signalRateLimit);
	} else {
		vl53l0_errmsg(
			"SetLimitCheckEnable(SIGNAL_RATE_FINAL_RANGE) failed with errcode = %d\n",
		 Status);
	}

	if (Status == VL53L0_ERROR_NONE) {
		Status = papi_func_tbl->SetLimitCheckValue(vl53l0_dev,
					VL53L0_CHECKENABLE_SIGMA_FINAL_RANGE,
					sigmaLimit);
	} else {
		vl53l0_dbgmsg(
			"SIGNAL_RATE_FINAL_RANGE failed with errcode = %d\n",
			 Status);
	}

	if (Status == VL53L0_ERROR_NONE) {
		Status =
		papi_func_tbl->SetMeasurementTimingBudgetMicroSeconds(
				vl53l0_dev,
				vl53l0_dev->timingBudget);
	} else {
		vl53l0_dbgmsg(
			"SIGMA_FINAL_RANGE failed with errcode = %d\n",
			 Status);
	}

	if (Status == VL53L0_ERROR_NONE) {
		Status = papi_func_tbl->SetVcselPulsePeriod(vl53l0_dev,
					VL53L0_VCSEL_PERIOD_PRE_RANGE,
					preRangePulsePeriod);
	} else {
		vl53l0_dbgmsg(
		 "SetMeasurementTimingBudget failed with errcode = %d\n",
			 Status);
	}

	if (Status == VL53L0_ERROR_NONE) {
		Status = papi_func_tbl->SetVcselPulsePeriod(vl53l0_dev,
			VL53L0_VCSEL_PERIOD_FINAL_RANGE,
			finalRangePulsePeriod);
	} else
		vl53l0_dbgmsg(
		"SetVcselPulsePeriod(PRE) failed with errcode = %d\n", Status);

	if (Status != VL53L0_ERROR_NONE)
		vl53l0_dbgmsg(
		"SetVcselPulsePeriod(FINAL)failed with errcode = %d\n", Status);
    mutex_unlock(&data->work_mutex);
	vl53l0_dbgmsg("End\n");
	return Status;
}


static int stmvl53l0_laser_parameter(struct stmvl53l0_data *data, hwlaser_parameter_t* param)
{
    int rc = 0;
    VL53L0_DEV vl53l0_dev = data;
    if(NULL == data || NULL == param)
        return -EINVAL;

    switch(param->name) {
        case (DEVICEMODE_PAR):
            if (param->is_read) {
                param->status = papi_func_tbl->GetDeviceMode(data, (VL53L0_DeviceModes *)&(param->value));
            } else {
                vl53l0_dbgmsg("laser set devicemode value =%d", param->value);
                rc = papi_func_tbl->SetDeviceMode(data, (VL53L0_DeviceModes)(param->value));
                if (rc != VL53L0_ERROR_NONE) {
                    vl53l0_errmsg("%d- error status %d\n", __LINE__, rc);
                    param->status = rc;
                    return rc;
                }
                data->deviceMode =  (VL53L0_DeviceModes)(param->value);
                if(data->deviceMode == VL53L0_DEVICEMODE_SINGLE_RANGING){
                    data->useCase = USE_CASE_HIGH_ACCURACY; //default mode
                }else if(data->deviceMode == VL53L0_DEVICEMODE_CONTINUOUS_RANGING){
                    data->useCase = USE_CASE_LONG_DISTANCE;// user mode
                }
                rc = papi_func_tbl->ClearInterruptMask(vl53l0_dev, 0);
                if (rc != VL53L0_ERROR_NONE) {
                    vl53l0_errmsg("Failed to ClearInterruptMask. Error = %d\n", rc);
                    param->status = rc;
                    return rc;
                }
                rc = stmvl53l0_config_use_case(vl53l0_dev);
                if (rc != VL53L0_ERROR_NONE) {
                    vl53l0_errmsg("Failed to configure Use case = %u\n", vl53l0_dev->useCase);
                    param->status = rc;
                    return rc;
                 }
            }
            rc = param->status;
            break;
        case OFFSET_PAR:
            rc = 0;
            break;
        case XTALKRATE_PAR:
            if (param->is_read) {
                param->value =  data->XTalkCompensationRateMegaCps;
                param->status = 0;
            }else{
                param->status = papi_func_tbl->SetXTalkCompensationEnable(data, 1);
                if (param->status != VL53L0_ERROR_NONE) {
                    vl53l0_errmsg("%d- error set xtalk %d\n", __LINE__, param->status);
                    return param->status;
                }
                data->XTalkCompensationRateMegaCps = param->value;
                param->status = papi_func_tbl->SetXTalkCompensationRateMegaCps(data,param->value);
                vl53l0_dbgmsg("laser set xtalk=%d\n", data->XTalkCompensationRateMegaCps);
            }
            rc = param->status;
            break;
        case XTALKENABLE_PAR:
            rc = 0;
            break;
        case GPIOFUNC_PAR:
            rc = 0;
            break;
        case LOWTHRESH_PAR:
            rc = 0;
            break;
        case HIGHTHRESH_PAR:
            rc = 0;
            break;
        case INTERMEASUREMENT_PAR:
            rc = 0;
            break;
        case REFERENCESPADS_PAR:
            rc = 0;
            break;
        case REFCALIBRATION_PAR:
            rc = 0;
            break;
        case POLLDELAY_PAR:
            rc = 0;
            break;
        case TIMINGBUDGET_PAR:
            if (param->is_read) {
                param->value = data->timingBudget/1000;
                param->status = 0;
            } else {
                data->timingBudget = param->value*1000;
                param->status = VL53L0_SetMeasurementTimingBudgetMicroSeconds(data, data->timingBudget);
                vl53l0_dbgmsg("laser timingBudget=%d\n", data->timingBudget);
            }
            rc = param->status;
            break;
        case DISTANCEMODE_PAR:
            rc = 0;
            break;
        case SIGMALIMIT_PAR:
            if (param->is_read) {
                param->value = data->sigmaLimit;
                param->status = 0;
            } else {
                data->sigmaLimit = param->value;
                param->status = papi_func_tbl->SetLimitCheckValue(data,
                        VL53L0_CHECKENABLE_SIGMA_FINAL_RANGE, data->sigmaLimit);
                vl53l0_dbgmsg("laser sigmaLimit=%d\n", data->sigmaLimit);
            }
            rc = param->status;
            break;
        case SIGNALLIMIT_PAR:
            if (param->is_read) {
                param->value = data->signalRateLimit;
                param->status = 0;
            } else {
                data->signalRateLimit = param->value;
                param->status = papi_func_tbl->SetLimitCheckValue(data,
				VL53L0_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, data->signalRateLimit);
                vl53l0_dbgmsg("laser signalRateLimit=%d\n", data->signalRateLimit);
            }
            rc = param->status;
            break;
        case RIT_PAR:
            if (param->is_read) {
                param->value = data->ritEnable;
                param->value2 = data->ritValue;
                param->status = 0;
            } else {
                data->ritEnable = param->value;
                data->ritValue = param->value2;
                if(data->ritEnable == 0){
                    param->status = papi_func_tbl->SetLimitCheckEnable(data,
                        VL53L0_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 0);
                }else{
                    param->status = papi_func_tbl->SetLimitCheckEnable(data,
                        VL53L0_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
                    if (param->status != VL53L0_ERROR_NONE) {
                        vl53l0_errmsg("%d- error status %d\n", __LINE__, param->status);
                        return param->status;
                    }
                    param->status = papi_func_tbl->SetLimitCheckValue(data,
                        VL53L0_CHECKENABLE_RANGE_IGNORE_THRESHOLD, data->ritValue);
                }
                vl53l0_dbgmsg("laser ritValue=%d, ritEnable=%d\n", data->ritValue, data->ritEnable);
            }
            rc = param->status;
            break;
        case NVM_PAR: 
            rc = 0;
            break;
        default:
            rc = 0;
            break;
        }
    return rc;
}

static int stmvl53l0_laser_get_data(struct stmvl53l0_data *data, void* p)
{
    hwlaser_RangingData_t* udata;
    VL53L0_RangingMeasurementData_t* rdata;
    if(NULL == data || NULL == p)
        return -EINVAL;
    mutex_lock(&data->data_mutex);
    rdata = &(data->rangeData);
    udata = (hwlaser_RangingData_t*) p;
    memcpy_s(&(udata->u.dataL0),sizeof(VL53L0_RangingMeasurementData_t) ,rdata, sizeof(VL53L0_RangingMeasurementData_t));
    if(data->print_count%LASER_PRINTCYLE == 0)
    {
        vl53l0_dbgmsg("laser distance = %d, status =%d\n", data->rangeData.RangeMilliMeter, data->rangeData.RangeStatus);
    }
    data->print_count++;
    mutex_unlock(&data->data_mutex);
    return 0;
}

static int stmvl53l0_laser_init(struct stmvl53l0_data *data)
{
    VL53L0_DEV vl53l0_dev = data;
    VL53L0_Error Status;
    if(!data)
    {
        vl53l0_errmsg("input data is NULL!!");
        return -EINVAL;
    }
    Status = stmvl53l0_init_client(data);
    if (Status) {
        vl53l0_errmsg("%d, error rc %d\n", __LINE__, Status);
        return -EINVAL;
    }
    /* set up device parameters */
    data->gpio_polarity = VL53L0_INTERRUPTPOLARITY_LOW;

	/* Following two calls are made from IOCTL as well */
    Status = papi_func_tbl->SetGpioConfig(vl53l0_dev, 0, 0,
        data->gpio_function, VL53L0_INTERRUPTPOLARITY_LOW);
    if (Status != VL53L0_ERROR_NONE) {
        vl53l0_errmsg("Failed to SetGpioConfig. Error = %d\n", Status);
        return -EPERM;
    }

    data->enable_ps_sensor = 0;
    vl53l0_dbgmsg("End\n");
    return Status;
}

VL53L0_Error WaitStopCompleted(VL53L0_DEV Dev)
{
	VL53L0_Error Status = VL53L0_ERROR_NONE;
	uint32_t StopCompleted = 0;
	uint32_t LoopNb;

	/* Wait until it finished
	 * use timeout to avoid deadlock
	 */
    if(!Dev)
        return -EINVAL;
	if (Status == VL53L0_ERROR_NONE) {
		LoopNb = 0;
		do {
			Status = papi_func_tbl->GetStopCompletedStatus(Dev,
							&StopCompleted);
			if ((StopCompleted == 0x00) ||
			    (Status != VL53L0_ERROR_NONE))
				break;

			LoopNb = LoopNb + 1;
			VL53L0_PollingDelay(Dev);
		} while (LoopNb < VL53L0_DEFAULT_MAX_LOOP);

		if (LoopNb >= VL53L0_DEFAULT_MAX_LOOP)
			Status = VL53L0_ERROR_TIME_OUT;

	}
	vl53l0_dbgmsg("End WaitStopCompleted - %d\n", Status);
	return Status;
}

static int stmvl53l0_stop(struct stmvl53l0_data *data)
{
    int rc = 0;
    VL53L0_DEV vl53l0_dev = data;

    vl53l0_dbgmsg("Enter\n");
    if(!data)
        return -EINVAL;

    mutex_lock(&data->work_mutex);
    /* stop - if continuous mode */
    if (data->deviceMode == VL53L0_DEVICEMODE_CONTINUOUS_RANGING ||
        data->deviceMode == VL53L0_DEVICEMODE_CONTINUOUS_TIMED_RANGING)
        papi_func_tbl->StopMeasurement(vl53l0_dev);

    rc = WaitStopCompleted(vl53l0_dev);
    if (rc != VL53L0_ERROR_NONE) {
        vl53l0_errmsg("wait stop error status %d\n", rc);
    }
    /* clean interrupt */
    papi_func_tbl->ClearInterruptMask(vl53l0_dev, 0);
    /* cancel work handler */
    stmvl53l0_cancel_handler(data);

    /* Clear updateUseCase pending operation */
    data->updateUseCase = 0;
    data->enable_ps_sensor = 0;
    data->interruptStatus = 0;
    mutex_unlock(&data->work_mutex);
    /* power down */
    vl53l0_dbgmsg("End\n");

    return rc;
}

static int stmvl53l0_start(struct stmvl53l0_data *data)
{
    int rc = 0;
    VL53L0_DEV vl53l0_dev = data;

    vl53l0_dbgmsg("Enter\n");
    if(!data)
        return -EINVAL;
    mutex_lock(&data->work_mutex);
    papi_func_tbl->StartMeasurement(vl53l0_dev);
    if(data->deviceMode == VL53L0_DEVICEMODE_SINGLE_RANGING){
        data->enable_ps_sensor = 0;
    }else{
        data->enable_ps_sensor = 1; //open interrupt handle
#ifndef USE_INT
	/* Unblock the thread execution */
	wake_up(&vl53l0_dev->poll_thread_wq);
#endif
    }
    data->print_count = 0;
    mutex_unlock(&data->work_mutex);
    return rc;
}

int stmvl53l0_setup(struct stmvl53l0_data *data)
{
    int rc = 0;
#ifdef USE_INT
    int irq = 0;
#endif
    if(NULL == data || NULL == data->client_object)
    {
        vl53l0_errmsg("data NULL");
        return -EINVAL;
    }
    papi_func_tbl = &stmvl53l0_api_func_tbl;
    /* init mutex */
    mutex_init(&data->update_lock);
    mutex_init(&data->work_mutex);
    mutex_init(&data->data_mutex);

#ifdef USE_INT
    rc = request_threaded_irq((unsigned int)(data->irq), NULL,
            stmvl53l0_interrupt_handler,
            IRQF_TRIGGER_FALLING|IRQF_ONESHOT,
            "vl53l0_interrupt",
            (void *)data);
    if (rc) {
        vl53l0_errmsg("allocate stmvl53l0 int fail ! result:%d\n",  rc);
        free_irq((unsigned int)(data->irq), data);
        goto exit_free_irq;
    }
#else

	init_waitqueue_head(&data->poll_thread_wq);

	data->poll_thread = kthread_run(&stmvl53l0_poll_thread,
							(void *)data,
							"STM-VL53L0");
	if (data->poll_thread == NULL) {
		pr_err(
	"%s(%d) - Failed to create Polling thread\n", __func__, __LINE__);
		goto exit_free_irq;
	}
#endif
    /* init work handler */
    INIT_DELAYED_WORK(&data->dwork, stmvl53l0_work_handler);

    /* init default device parameter value */
    data->enable_ps_sensor = 0;
    data->reset = 1;
    data->delay_ms = POLL_DELAY_TIME; /* delay time to 35ms */
    data->enableDebug = 0;
    data->gpio_polarity = VL53L0_INTERRUPTPOLARITY_LOW;
    data->gpio_function = VL53L0_GPIOFUNCTIONALITY_NEW_MEASURE_READY;
    data->low_threshold = 60;
    data->high_threshold = 200;
    data->deviceMode = VL53L0_DEVICEMODE_CONTINUOUS_RANGING;
    data->interMeasurems = 30;
    data->useCase = USE_CASE_LONG_DISTANCE;
    data->timingBudget = LONG_DISTANCE_TIMING_BUDGET;

    /* Set default values used in Custom Mode Use Case */
    data->signalRateLimit = LONG_DISTANCE_SIGNAL_RATE_LIMIT;
    data->sigmaLimit = LONG_DISTANCE_SIGMA_LIMIT;
    data->preRangePulsePeriod = LONG_DISTANCE_PRE_RANGE_PULSE_PERIOD;
    data->finalRangePulsePeriod = LONG_DISTANCE_FINAL_RANGE_PULSE_PERIOD;
    data->ritEnable = 0;
    data->interruptStatus = 0;
    vl53l0_dbgmsg("support ver. %s enabled\n", DRIVER_VERSION);
    return rc;

exit_free_irq:
#ifdef USE_INT
    free_irq(irq, data);
#endif
    return rc;
}

extern VL53L0_Error VL53L0_get_info_from_device(VL53L0_DEV Dev, uint8_t option);
int stmvl53l0_data_init(void** pdata, struct i2c_data *i2c_object)
{
    int rc = 0;
    struct stmvl53l0_data *vl53l0_data = NULL;
    if(pdata == NULL || i2c_object == NULL)
        return -EINVAL;
    vl53l0_data = kzalloc(sizeof(struct stmvl53l0_data), GFP_KERNEL);
    if (!vl53l0_data) {
    	rc = -ENOMEM;
    	return rc;
    }
    vl53l0_data->client_object = (void*)i2c_object;
    /* setup bus type */
    vl53l0_data->bus_type = I2C_BUS;
    /* setup device name */
    vl53l0_data->dev_name = dev_name(&i2c_object->client->dev);
    vl53l0_data->irq = i2c_object->irq;
    vl53l0_data->init_flag = 1;
    rc = stmvl53l0_setup(vl53l0_data);
    if(rc) {
        vl53l0_errmsg("vl53l0 setup fail");
        kfree(vl53l0_data);
        return rc;
    }
    rc = VL53L0_get_info_from_device(vl53l0_data, 1);
    *pdata = (void *)vl53l0_data;
    i2c_set_clientdata(i2c_object->client, vl53l0_data);
    vl53l0_dbgmsg("End\n");
    return rc;
}

int stmvl53l0_laser_remove(struct i2c_client *client)
{
    struct stmvl53l0_data *vl53l0_data = NULL;
    if(client == NULL)
        return -EINVAL;
    vl53l0_data = i2c_get_clientdata(client);
#ifndef USE_INT
    poll_thread_exit = 1;
    kthread_stop(vl53l0_data->poll_thread);
#endif
    kfree(vl53l0_data);
    client->adapter = NULL;
    return 0;
}

//lint -restore
