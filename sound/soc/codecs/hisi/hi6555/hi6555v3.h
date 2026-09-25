/*
 * hi6555v3_driver codec driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HI6555V3_H__
#define __HI6555V3_H__

#include <sound/soc.h>

#define INVALID_MASK 0xFFF

#define PAGE_PMU_CODEC      0x0
#define PAGE_PMU_HKADC      0x2000
#define PAGE_PMU_CTRL_REGB  0x0
// because codec can't equal to pmu ctrl, here we just avoid compile error
#define PAGE_PMU_CTRL       INVALID_MASK

#define SoCCODEC_START      0x0
#define SoCCODEC_END        0x198
#define PMUCODEC_START      0x0
#define PMUCODEC_END        0x32F
#define PMUHKADC_START      0x0
#define PMUHKADC_END        0x10
#define PMUCLASSD_START     0x17A
#define PMUCLASSD_END       0x17D
#define PMUCTRL_START       0x0
#define PMUCTRL_END         0x300

#define ASPCFG_START        0x0
#define ASPCFG_END          0x248
#define AOIOC_START         0x0
#define AOIOC_END           0xF9C

#endif /* __HI6555V3_H__ */

