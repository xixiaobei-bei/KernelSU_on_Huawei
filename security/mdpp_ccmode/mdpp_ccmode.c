/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: get ccmode type
 * Author: yangzhenyuan yangzhenyuan@huawei.com
 * Create: 2019-10-25
 */
#include <chipset_common/security/mdpp_ccmode.h>
#include <linux/init.h>
#include <linux/string.h>
#include <huawei_platform/log/hw_log.h>

#define CCMODE_TYPE_ID "androidboot.sys.ccmode"

#define HWLOG_TAG mdpp_ccmode

HWLOG_REGIST();

static unsigned int ccmode_type __ro_after_init = CCMODE_TYPE_DISABLE; /* default ccmode disable */

static int __init early_parse_ccmode_cmdline(char *p)
{
	if ((p != NULL) && (strncmp(p, "enable", strlen("enable")) == 0)) {
		ccmode_type = CCMODE_TYPE_ENABLE;
		hwlog_info("ccmode is enable\n");
	} else {
		ccmode_type = CCMODE_TYPE_DISABLE;
		hwlog_info("ccmode is disable\n");
	}
	return 0;
}

early_param(CCMODE_TYPE_ID, early_parse_ccmode_cmdline);

/*
 * the function interface to check ccmode in kernel
 * return 0 is disable type , return 1 is enable type
 */
unsigned int get_ccmode(void)
{
	return ccmode_type;
}
