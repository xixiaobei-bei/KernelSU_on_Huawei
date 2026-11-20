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

#ifndef VI5300_FIRMWARE_H
#define VI5300_FIRMWARE_H

#include "vi5300_def.h"
#define FIRMWARE_NUM 8192
extern uint8_t Firmware[FIRMWARE_NUM];
uint32_t LoadFirmware(VI5300_DEV dev);

#endif
