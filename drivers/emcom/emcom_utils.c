/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: utils of xengine module
 * Author: lianghenghui lianghenghui@huawei.com
 * Create: 2017-07-24
 */

#include "emcom_utils.h"
#include <linux/module.h>
#include <linux/types.h>

#undef HWLOG_TAG
#define HWLOG_TAG emcom_utils
HWLOG_REGIST();
MODULE_LICENSE("GPL");

emcom_support_enum g_modem_emcom_support = MODEM_NOT_SUPPORT_EMCOM;

void Emcom_Ind_Modem_Support(uint8_t enSupport)
{
	emcom_logd("g_modem_emcom_support:%d\n", g_modem_emcom_support);
	g_modem_emcom_support = (emcom_support_enum)enSupport;
}

bool emcom_is_modem_support(void)
{
	if (g_modem_emcom_support == MODEM_NOT_SUPPORT_EMCOM)
		return false;
	else
		return true;
}
