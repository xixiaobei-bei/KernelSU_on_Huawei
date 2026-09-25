/*
 * wakeup.h -- wakeup driver
 *
 * Copyright (c) 2015 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __WAKEUP_H__
#define __WAKEUP_H__

#define ENVP_LENTH            2000
#define ENVP_EXT_MEMBER       7
#define WAKEUP_REPORT_EVENT  _IOWR('N', 0x01, __u64)
#define WAKEUP_INFO_EVENT    _IOWR('N', 0x02, __u64)

#endif //__WAKEUP_H__