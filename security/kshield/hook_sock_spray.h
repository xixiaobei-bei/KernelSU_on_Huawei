/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the hook_sock_spray.h provides API for kshield module
 * Author: dingpengfei
 * Create: 2019-11-09
 */

#ifndef _KSHIELD_HOOK_SOCK_SPRAY_H_
#define _KSHIELD_HOOK_SOCK_SPRAY_H_

#include "common.h"

int hook_sock_spray_init(void);
void hook_sock_spray_deinit(void);
int ks_enable_sock_spray(bool enable);
int ks_sock_spray_update(struct spray_meta meta);

#endif
