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

#ifndef _VI5300_API_H_
#define _VI5300_API_H_
#include "vi5300_def.h"

VI5300_Error VI5300_Chip_PowerON(VI5300_DEV dev);
VI5300_Error VI5300_Chip_PowerOFF(VI5300_DEV dev);
VI5300_Error VI5300_Single_Measure(VI5300_DEV dev);
VI5300_Error VI5300_Start_Continuous_Measure(VI5300_DEV dev);
VI5300_Error VI5300_Stop_Continuous_Measure(VI5300_DEV dev);
VI5300_Error VI5300_Get_Measure_Data(VI5300_DEV dev);
VI5300_Error VI5300_Get_Interrupt_State(VI5300_DEV dev);
VI5300_Error VI5300_Chip_Init(VI5300_DEV dev);
VI5300_Error VI5300_Start_XTalk_Calibration(VI5300_DEV dev);
VI5300_Error VI5300_Start_Offset_Calibration(VI5300_DEV dev);
VI5300_Error VI5300_Get_XTalk_Parameter(VI5300_DEV dev);
VI5300_Error VI5300_Config_XTalk_Parameter(VI5300_DEV dev);

#endif
