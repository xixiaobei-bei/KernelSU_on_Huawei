/*
 * fusb30X.c
 *
 * fusb30X driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/printk.h>
#include "fusb30X.h"
#include "platform.h"

FSC_BOOL DeviceWrite(FSC_U8 regAddr, FSC_U8 length, FSC_U8* data)
{
    FSC_BOOL error;
    error = platform_i2c_write(FUSB300SlaveAddr, FUSB300AddrLength, length, length, FUSB300IncSize, regAddr, data);
    if (error == FALSE)
        return TRUE;
    pr_info("FUSB %s - I2C Write NAK!\n", __func__);
    return FALSE;
}

FSC_BOOL DeviceRead(FSC_U8 regAddr, FSC_U8 length, FSC_U8* data)
{
    FSC_BOOL error;
    error = platform_i2c_read(FUSB300SlaveAddr, FUSB300AddrLength, length, length, FUSB300IncSize, regAddr, data);
    if (error == FALSE)
        return TRUE;
    pr_info("FUSB %s - I2C Read NAK!\n", __func__);
    return FALSE;
}
