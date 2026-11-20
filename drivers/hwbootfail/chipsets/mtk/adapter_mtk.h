/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: define adapter for MTK
 * Author: wangsiwen
 * Create: 2020-09-20
 */

#ifndef ADAPTER_MTK_H
#define ADAPTER_MTK_H

#include <hwbootfail/core/adapter.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BFI_DEV_PATH "/dev/block/by-name/rrecord"
#define LONG_PRESS_DEV_PATH "/dev/block/by-name/bootfail_info"

int mtk_adapter_init(struct adapter *padp);
void mtk_save_long_press_logs(void);

int is_boot_success(void);

#ifdef __cplusplus
}
#endif
#endif
