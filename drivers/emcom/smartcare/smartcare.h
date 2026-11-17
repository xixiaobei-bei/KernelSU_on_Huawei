/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: define macros and structs for smartcare.c
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
 */
#ifndef __SMARTCARE_H_INCLUDED__
#define __SMARTCARE_H_INCLUDED__

#include <linux/types.h>

void smartcare_event_process(int32_t event, uint8_t *pdata, uint16_t len);
void smartcare_init(void);
void smartcare_deinit(void);

#endif // __SMARTCARE_H_INCLUDED__
