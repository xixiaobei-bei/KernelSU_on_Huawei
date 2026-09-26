/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the hook_spray.h provides API for kshield module
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#ifndef _KSHIELD_HOOK_SPRAY_H_
#define _KSHIELD_HOOK_SPRAY_H_

#include "common.h"

int hook_spray_init(void);
void hook_spray_deinit(void);
int ks_enable_heap_spray(bool enable);
int ks_heap_spray_update(struct spray_meta meta);

#endif
