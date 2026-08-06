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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include "vi5300.h"
#include "vi5300_platform.h"
#include "vi5300_firmware.h"
#include "vi5300_api.h"

#define PILEUP_A (9231000)
#define PILEUP_B (4896)
#define PILEUP_C (1922)
#define PILEUP_D (10)


VI5300_Error VI5300_Chip_PowerON(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = gpio_direction_output(dev->xshut_gpio, 0);
	mdelay(5);
	Status += gpio_direction_output(dev->xshut_gpio, 1);
	mdelay(5);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Chip Power ON Failed Status = %d\n", Status);
	return Status;
}

VI5300_Error VI5300_Chip_PowerOFF(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	Status = gpio_direction_output(dev->xshut_gpio, 0);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Chip Power OFF Failed Status = %d\n", Status);
	return Status;
}

VI5300_Error VI5300_RCO_Config(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status |= vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0F);
	Status |= vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0E);
	mdelay(1);
	Status |= vi5300_write_byte(dev, VI5300_REG_RCO_VREF, 0x24);
	Status |= vi5300_write_byte(dev, VI5300_REG_RCO_TEMP, 0x00);
	Status |= vi5300_write_byte(dev, VI5300_REG_VCSELDRV_PULSE, 0x00);
	udelay(4);

	return Status;
}

VI5300_Error VI5300_Wait_For_CPU_Ready(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t stat;
	int retry = 0;

	do {
		mdelay(1);
		Status = vi5300_read_byte(dev, VI5300_REG_DEV_STAT, &stat);
	}while((retry++ < VI5300_MAX_WAIT_RETRY)
		&&(Status == VI5300_ERROR_NONE)
		&&(stat & 0x01));
	if(retry >= VI5300_MAX_WAIT_RETRY)
		vi5300_errmsg("CPU Busy stat = %d\n", stat);

	return Status;
}

VI5300_Error VI5300_Init_FirmWare(VI5300_DEV dev)
{
	uint8_t  sys_cfg_data = 0;
	uint16_t fw_size = 0;
	uint16_t fw_send = 0;
	uint8_t val;
	VI5300_Error Status = VI5300_ERROR_NONE;

	fw_size = LoadFirmware(dev);
	if(!fw_size)
		return VI5300_ERROR_FW_FAILURE;

	VI5300_RCO_Config(dev);
	Status += vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x08);
	Status += vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0a);
	Status += vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x06);
	Status += vi5300_read_byte(dev, VI5300_REG_SYS_CFG, &sys_cfg_data);
	Status += vi5300_write_byte(dev, VI5300_REG_SYS_CFG, sys_cfg_data | (0x01 << 0));
	Status += vi5300_write_byte(dev, VI5300_REG_DIGLDO_VREF, 0x30);
	Status += vi5300_write_byte(dev, VI5300_REG_ANALDO_VREF, 0x30);
	Status += vi5300_write_byte(dev, VI5300_REG_CMD, 0x01);
	Status += vi5300_write_byte(dev, VI5300_REG_SIZE, 0x02);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x0);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, 0x0);
	while(fw_size >= 32)
	{
		Status += vi5300_write_reg_offset(dev, VI5300_REG_CMD, 0, VI5300_WRITEFW_CMD);
		Status += vi5300_write_reg_offset(dev, VI5300_REG_SIZE, 0, 0x20);
		vi5300_write_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, Firmware+fw_send*32, 32);
		udelay(10);
		fw_send += 1;
		fw_size -= 32;
	}
	if(fw_size > 0)
	{
		Status += vi5300_write_reg_offset(dev, VI5300_REG_CMD, 0, VI5300_WRITEFW_CMD);
		Status += vi5300_write_reg_offset(dev, VI5300_REG_SIZE, 0, (uint8_t)fw_size);
		vi5300_write_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, Firmware+fw_send*32, fw_size);
	}
	Status += vi5300_write_byte(dev, VI5300_REG_SYS_CFG, sys_cfg_data & ~(0x01 << 0));
	Status += vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x06);
	Status += vi5300_write_byte(dev, VI5300_REG_PD_RESET, 0xA0);
	Status += vi5300_write_byte(dev, VI5300_REG_PD_RESET, 0x80);
	Status += vi5300_write_byte(dev, VI5300_REG_RCO_VREF, 0x24);
	Status += vi5300_write_byte(dev, VI5300_REG_RCO_TEMP, 0x00);
	Status += vi5300_write_byte(dev, VI5300_REG_VCSELDRV_PULSE, 0x00);
	Status += vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x07);
	Status += vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x02);
	Status += vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x00);
	mdelay(5);
	Status = vi5300_read_byte(dev, VI5300_REG_SPCIAL_PURP, &val);
	if(Status != VI5300_ERROR_NONE || val != 0x66)
	{
		vi5300_errmsg("Download Firmware Failed Status = %d\n", Status);
		return VI5300_ERROR_FW_FAILURE;
	}
	return Status;
}
VI5300_Error VI5300_Integral_Counts_Write(VI5300_DEV dev, uint32_t inte_counts)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_RCO_Config(dev);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x03);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x01);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, *((uint8_t *)(&inte_counts)));
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, *((uint8_t *)(&inte_counts) + 1));
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 5, *((uint8_t *)(&inte_counts) + 2));
	Status += vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Write Integral Counts Failed Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Delay_Count_Write(VI5300_DEV dev, uint16_t delay_count)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_RCO_Config(dev);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x04);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, *((uint8_t *)(&delay_count) + 1));
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, *((uint8_t *)(&delay_count)));
	Status += vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Write Integral Times Failed Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Set_Integralcounts_Frame(VI5300_DEV dev, uint8_t fps, uint32_t intecoutns)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint32_t inte_time;
	uint32_t fps_time;
	uint32_t delay_time;
	uint16_t delay_counts;

	inte_time = intecoutns *1463/10;
	if (fps == 0) {
		vi5300_errmsg("fps is zero\n");
		return VI5300_ERROR_FPS_ZERO;
	}
	fps_time = 1000000000/fps;
	delay_time = fps_time - inte_time -1600000;
	delay_counts = (uint16_t)(delay_time/40900);

	Status = VI5300_Integral_Counts_Write(dev, intecoutns);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Integral Counts Write Failed Status = %d\n", Status);
		return VI5300_ERROR_INTEGRAL_COUNT;
	}
	Status = VI5300_Delay_Count_Write(dev, delay_counts);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Delay Counts Write Failed Status = %d\n", Status);
		return VI5300_ERROR_DELAY_COUNT;
	}
	return Status;
}


VI5300_Error VI5300_Single_Measure(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_Wait_For_CPU_Ready(dev);
	VI5300_RCO_Config(dev);
	Status += vi5300_write_byte(dev, VI5300_REG_CMD, 0x0E);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Single measure Failed Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Start_Continuous_Measure(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_Wait_For_CPU_Ready(dev);
	VI5300_RCO_Config(dev);
	Status += vi5300_write_byte(dev, VI5300_REG_CMD, 0x0F);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Start Measure Failed Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Stop_Continuous_Measure(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t val;
	vi5300_infomsg("enter VI5300_Stop_Continuous_Measure");
	Status = vi5300_read_byte(dev, VI5300_REG_DEV_STAT, &val);
	if(Status == VI5300_ERROR_NONE && (val & 0x01))
	{
		Status = vi5300_write_byte(dev, VI5300_REG_CMD, 0x00);
		if(Status != VI5300_ERROR_NONE)
			vi5300_errmsg("Stop Measure Failed Status = %d\n", Status);
	}

	return Status;
}

VI5300_Error VI5300_Get_Measure_Data(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t buf[32];
	int16_t milimeter;
	uint16_t noise_level;
	uint32_t peak1;
	uint32_t peak2;
	uint32_t integral_times;
	uint32_t confidence;
	uint32_t noise_aver;
	uint32_t noise_origin;
	uint32_t sigma;
	int32_t bias;
	uint32_t peak_tmp;

	Status = vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, buf, 32);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Get Range Data Failed Status = %d\n", Status);
		return Status;
	}
	milimeter = *((int16_t *)(buf + 12));
	noise_level = *((uint16_t *)(buf + 26));
	integral_times = *((uint32_t *)(buf + 22));
	peak1 = *((uint32_t *)(buf + 28));
	peak2 = *((uint32_t *)(buf + 8));
	integral_times = integral_times &0x00ffffff;
	if(peak2 > 65536)
		peak_tmp = peak2 * 256 / integral_times * 256;
	else
		peak_tmp = peak2 *65536 / integral_times;
	peak_tmp = peak_tmp >> 12;
	bias = (int32_t)(PILEUP_A / (PILEUP_B - peak_tmp * PILEUP_D) - PILEUP_C) / PILEUP_D;
	if(bias < 0)
		bias = 0;
	milimeter = milimeter + (int16_t)bias;
	milimeter = milimeter - dev->offset_config;

	noise_origin = noise_level + 16;
	noise_aver = noise_origin * 171 / 16;
	if(noise_origin < 320)
	{
		sigma = 5388 * noise_origin / 16000 - 203900 / (noise_origin * 1000 / 16 + 1975) + 104;
	} else if(noise_origin < 3200) {
		sigma = 1218 * noise_origin / 16000 - 497760 / (noise_origin + 1200) + 505;
	} else if(noise_origin <32000) {
		sigma = 3844 * noise_origin / 160000 - 15862400 / (noise_origin + 12080) + 1602;
	} else {
		sigma = 2082 * noise_origin / 160000 - 111312000 / (noise_origin + 45952) + 3020;
	}
	if(peak1 < (noise_aver + 3 * sigma)) {
		confidence = 0;
	} else if(peak1 > (noise_aver + 12 * sigma)) {
		confidence = 100;
	} else {
		confidence = 100 * (peak1 - (noise_aver + 3 * sigma)) / (9 * sigma);
	}
	dev->RangData.milimeter = milimeter;
	dev->RangData.confidence = confidence;
	if(dev->status == VI5300_ERROR_NONE)
	{
		if(confidence > 90)
			dev->RangData.status = 0;
		else if(confidence < 20)
			dev->RangData.status = 255;
		else
			dev->RangData.status = 1;
	} else
		dev->RangData.status = 1;

	return Status;
}

VI5300_Error VI5300_Get_Interrupt_State(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t stat;

	Status = vi5300_read_byte(dev, VI5300_REG_INTR_STAT, &stat);

	if(!(stat & 0x01))
		vi5300_errmsg("Get Interrupt State Failed Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Interrupt_Enable(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	int loop = 0;
	uint8_t enable = 0;

	do
	{
		Status += vi5300_read_byte(dev, VI5300_REG_INTR_MASK, &enable);
		enable |=  0x01;
		Status += vi5300_write_byte(dev, VI5300_REG_INTR_MASK, enable);
		Status += vi5300_read_byte(dev, VI5300_REG_INTR_MASK, &enable);
		loop++;
	} while((loop < VI5300_MAX_WAIT_RETRY)
		&& (Status == VI5300_ERROR_NONE)
		&& (!(enable & 0x01)));
	if(loop >= VI5300_MAX_WAIT_RETRY)
		vi5300_errmsg("Enable interrupt Failed Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Chip_Init(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint32_t integral;
	uint8_t frame;

	Status = VI5300_Wait_For_CPU_Ready(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Internal CPU busy!\n");
		return Status;
	}
	Status = VI5300_Interrupt_Enable(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Clear Interrupt Mask failed!\n");
		return Status;
	}
	Status = VI5300_Init_FirmWare(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Download Firmware Failed!\n");
		return Status;
	}
	integral = dev->integral_counts;
	frame = dev->frame_counts;
	Status = VI5300_Set_Integralcounts_Frame(dev, frame, integral);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Set Integral Counts and Frame Failed!\n");
		return Status;
	}

	return Status;
}


VI5300_Error VI5300_Start_XTalk_Calibration(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_Wait_For_CPU_Ready(dev);
	VI5300_RCO_Config(dev);
	Status = vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_XTALK_TRIM_CMD);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("XTALK Calibration Status = %d\n", Status);

	return Status;
}

VI5300_Error VI5300_Start_Offset_Calibration(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t buf[32];
	int16_t mm;
	uint32_t peak;
	uint32_t inte_t;
	int32_t bias;
	uint32_t peak_t;
	int16_t total = 0;
	int16_t offset = 0;
	int cnt = 0;
	uint8_t stat;

	Status = VI5300_Start_Continuous_Measure(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Offset Calibtration Start Failed!\n");
		return Status;
	}
	while(1)
	{
		mdelay(35);
		Status = vi5300_read_byte(dev, VI5300_REG_INTR_STAT, &stat);
		if(Status == VI5300_ERROR_NONE)
		{
			if((stat & 0x01) == 0x01)
			{
				Status = vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, buf, 32);
				if(Status != VI5300_ERROR_NONE)
				{
					vi5300_errmsg("Get Range Data Failed Status = %d\n", Status);
					break;
				}
				mm = *((int16_t *)(buf + 12));
				inte_t = *((uint32_t *)(buf + 22));
				peak = *((uint32_t *)(buf + 8));
				inte_t = inte_t &0x00ffffff;
				if(peak > 65536)
					peak_t = peak * 256 / inte_t * 256;
				else
					peak_t = peak *65536 / inte_t;
				peak_t = peak_t >> 12;
				bias = (int32_t)(PILEUP_A / (PILEUP_B - peak_t * PILEUP_D) - PILEUP_C) / PILEUP_D;
				if(bias < 0)
					bias = 0;
				mm = mm + (int16_t)bias;
				total += mm;
				++cnt;
			} else
				continue;
		} else { 
			vi5300_errmsg("can't get irq state!Status = %d\n", Status);
			break;
		}
		if (cnt >= 30)
			break;
	}
	Status = VI5300_Stop_Continuous_Measure(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Offset Calibtration Stop Failed!\n");
		return Status;
	}
	offset = total / 30;
	dev->OffsetData.offset_cal = offset - 50;

	return Status;
}

VI5300_Error VI5300_Get_XTalk_Parameter(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t val;
	uint8_t cg_buf[5];

	Status = vi5300_read_byte(dev, VI5300_REG_SPCIAL_PURP, &val);
	if(Status == VI5300_ERROR_NONE && val == 0xaa)
	{
		Status = vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, cg_buf, 5);
		if(Status != VI5300_ERROR_NONE)
		{
			vi5300_errmsg("Get XTALK parameter Failed Status = %d\n", Status);
			return Status;
		}
		dev->XtalkData.xtalk_cal = *((int8_t *)(cg_buf + 0));
		dev->XtalkData.xtalk_peak = *((uint16_t *)(cg_buf + 1));
		dev->XtalkData.xtalk_tof = *((int16_t *)(cg_buf + 3));
	} else {
		vi5300_errmsg("XTALK Calibration Failed Status = %d, val = 0x%02x\n", Status, val);
		return VI5300_ERROR_XTALK_CALIB;
	}

	return Status;
}

VI5300_Error VI5300_Config_XTalk_Parameter(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_Wait_For_CPU_Ready(dev);
	VI5300_RCO_Config(dev);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, VI5300_XTALKW_SUBCMD);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, VI5300_XTALK_SIZE);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x02, VI5300_XTALK_ADDR);
	Status += vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x03, *((uint8_t*)(&dev->xtalk_config)));
	Status += vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	if(Status != VI5300_ERROR_NONE)
		vi5300_errmsg("Config XTALK Failed Status = %d\n", Status);

	return Status;
}