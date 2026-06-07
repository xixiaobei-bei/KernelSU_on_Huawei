/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2019. All rights reserved.
 * Description: recovery misc
 * Author: jimingquan jimingquan@huawei.com
 * Create: 2015-05-19
 */

#ifndef __RECOVERY_MISC_H_
#define __RECOVERY_MISC_H_

#define MISC_LOG_INFO(fmt, args...) hwlog_info(fmt, ## args)
#define MISC_LOG_ERR(fmt, args...) hwlog_err(fmt, ## args)
#define MISC_LOG_DEBUG(fmt, args...) hwlog_debug(fmt, ## args)

typedef struct {
	char command[32];
	char status[32];
	char recovery[1024];
} misc_message_type;

#define MISC_BOOT_RECOVERY_STR  "boot-recovery"
#define MISC_NODE_PATH  "/dev/block/bootdevice/by-name/misc"

#endif
