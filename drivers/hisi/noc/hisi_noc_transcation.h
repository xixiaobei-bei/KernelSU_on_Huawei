/*
 * NoC. (NoC Mntn Module.)
 *
 * Copyright (c) 2016 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HISI_NOC_TRANSCATION
#define __HISI_NOC_TRANSCATION

#include "hisi_noc.h"

#define TRANS_M_CFGCTL_INIT_VALUE       0
#define TRANS_M_MAINCTL_INIT_VALUE      3
#define TRANS_M_MAINCTL_SET_VALUE       4
#define TRANS_M_MAINCTL_SET_EN_VALUE    3

#define TRANS_ENABLE                    0x1
#define TRANS_ENABLE_ALARM_INT          0x4
#define TRANS_ENABLE_PROBE              0x0
#define TRANS_P_OVERFLOWSTATUS_VALUE    0x1
#define TRANS_M_STATALARMCLR_VALUE      0x1

#define TRANS_CFG_STATAALARMMAX         0x00ffffff
#define TRANS_CFG_STATPERIOD            0x8
#define TRANS_CFG_TRANS_F_MODE          0x1
#define TRANS_CFG_TRANS_F_ADDRBASE_LOW  0x0
#define TRANS_CFG_TRANS_F_ADDRWINSIZE   0x20
#define TRANS_CFG_TRANS_F_OPCODE        0x3
#define TRANS_CFG_TRANS_F_USERMASK      0x0
#define TRANS_CFG_TRANS_F_SECMASK       0x1
#define TRANS_CFG_TRANS_P_MODE          0x0
#define TRANS_CFG_TRANS_P_THRESHOLD_0   0x0
#define TRANS_CFG_TRANS_P_THRESHOLD_1   0x1
#define TRANS_CFG_TRANS_P_THRESHOLD_2   0x2
#define TRANS_CFG_TRANS_P_THRESHOLD_3   0x3
#define TRANS_CFG_TRANS_M_CNT_0_SRC     0x20
#define TRANS_CFG_TRANS_M_CNT_1_SRC     0x21
#define TRANS_CFG_TRANS_M_CNT_2_SRC     0x22
#define TRANS_CFG_TRANS_M_CNT_3_SRC     0x23
#define TRANS_CFG_TRANS_M_CNT_0_ALARMM  0x2
#define TRANS_CFG_TRANS_M_CNT_1_ALARMM  0x2
#define TRANS_CFG_TRANS_M_CNT_2_ALARMM  0x2
#define TRANS_CFG_TRANS_M_CNT_3_ALARMM  0x2

/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_MODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_MODE                    0x0008
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_ADDRBASE_LOW_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_ADDRBASE_LOW            0x000c
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_ADDRWINDOWSIZE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_ADDRWINDOWSIZE          0x0014
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_OPCODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_OPCODE                  0x0020
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_USERBASE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_USERBASE                0x0024
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_USERMASK_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_USERMASK                0x0028
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_F_SECURITYMASK_ADDR(0) - TRANS_F_BASE) */
#define TRANS_F_SECURITYMASK            0x0030

/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_EN_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_EN                      0x0088
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_MODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_MODE                    0x008c
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_THRESHOLDS_0_0_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_THRESHOLDS_0_0          0x00ac
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_THRESHOLDS_0_1_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_THRESHOLDS_0_1          0x00b0
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_THRESHOLDS_0_2_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_THRESHOLDS_0_2          0x00b4
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_THRESHOLDS_0_3_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_THRESHOLDS_0_3          0x00b8
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_OVERFLOWSTATUS_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_OVERFLOWSTATUS          0x00ec
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_OVERFLOWRESET_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_OVERFLOWRESET           0x00f0
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_P_PRESCALER_ADDR(0) - TRANS_F_BASE) */
#define TRANS_P_PRESCALER               0x00f8

/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_MAINCTL_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_MAINCTL                 0x0408
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_CFGCTL_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_CFGCTL                  0x040c
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_STATPERIOD_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_STATPERIOD              0x0424
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_STATALARMMAX_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_STATALARMMAX            0x0430
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_STATALARMCLR_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_STATALARMCLR            0x0438

/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_0_SRC_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_0_SRC          0x0538
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_0_ALARMMODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_0_ALARMMODE    0x053c
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_0_VAL_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_0_VAL          0x0540
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_1_SRC_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_1_SRC          0x054c
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_1_ALARMMODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_1_ALARMMODE    0x0550
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_1_VAL_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_1_VAL          0x0554
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_2_SRC_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_2_SRC          0x0560
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_2_ALARMMODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_2_ALARMMODE    0x0564
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_2_VAL_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_2_VAL          0x0568
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_3_SRC_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_3_SRC          0x0574
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_3_ALARMMODE_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_3_ALARMMODE    0x0578
/* (SOC_CFG_SYS_NOC_BUS_ASP_TRANS_M_COUNTERS_3_VAL_ADDR(0) - TRANS_F_BASE) */
#define TRANS_M_COUNTERS_3_VAL          0x057c

void disable_transcation_probe(void __iomem *base);
void enable_transcation_probe(const struct noc_node *node, void __iomem *base);
void noc_transcation_probe_hanlder(const struct noc_node *node,
				   void __iomem *base, unsigned int idx);
void init_transcation_info(struct noc_node *node);
void enable_transcation_probe_by_name(const char *name);
void disable_transcation_probe_by_name(const char *name);
void config_transcation_probe(const char *name,
			      const struct transcation_configration *tran_cfg);
#endif
