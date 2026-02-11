/*
* emcom_utils.c
*
*  utils of xengine module
*
* Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include "emcom_utils.h"
#include <linux/module.h>
#include <linux/types.h>

#undef HWLOG_TAG
#define HWLOG_TAG emcom_utils
HWLOG_REGIST();
MODULE_LICENSE("GPL");

EMCOM_SUPPORT_ENUM g_modem_emcom_support = MODEM_NOT_SUPPORT_EMCOM;

void Emcom_Ind_Modem_Support(uint8_t enSupport)
{
    EMCOM_LOGD("g_modem_emcom_support:%d\n", g_modem_emcom_support);
    g_modem_emcom_support = (EMCOM_SUPPORT_ENUM)enSupport;
}

bool emcom_is_modem_support( void )
{
    if(g_modem_emcom_support == MODEM_NOT_SUPPORT_EMCOM)
        return false;
    else
        return true;
}
