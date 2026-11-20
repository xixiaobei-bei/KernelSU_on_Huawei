/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description:  headfile of wakeup.c
 * Author: zhangshengwei
 * Create: 2019-04-25
 * History: 2019-04-25 Creat new file
 */
#ifndef __WAKEUP_H__
#define __WAKEUP_H__

#define ENVP_LENTH            2000
#define ENVP_EXT_MEMBER       7
#define WAKEUP_REPORT_EVENT  _IOWR('N', 0x01, __u64)
#define WAKEUP_INFO_EVENT    _IOWR('N', 0x02, __u64)

#endif // __WAKEUP_H__