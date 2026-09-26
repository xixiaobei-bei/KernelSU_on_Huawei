/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: eipf interface for drv
 * Author: shennaiyu
 * Create: 2020-10-12
 */
#ifndef __BSP_EIPF_H__
#define __BSP_EIPF_H__

#ifdef CONFIG_EIPF
void bsp_ipf_reinit(void);
#else
static inline void bsp_ipf_reinit(void){};
#endif
#endif
