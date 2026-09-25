/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the hook_charact.h provides API for kshield module
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#ifndef _KSHIELD_HOOK_CHARACT_H_
#define _KSHIELD_HOOK_CHARACT_H_

#include <linux/types.h>
#include "common.h"

int hook_cha_init(void);
void hook_cha_deinit(void);
int hook_cha_enable(bool enable);
int hook_cha_update(struct cha_conf_meta meta);
bool kshield_chk_suspect(unsigned int hook_id);

#endif
