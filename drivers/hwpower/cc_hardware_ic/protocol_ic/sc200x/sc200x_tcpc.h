/*
 * sc200x_tcpc.h
 *
 * sc200x tcpc header file
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _SC200X_TCPC_H_
#define _SC200X_TCPC_H_

#include "sc200x.h"

#define SC200X_TYPEC_VBUS_INHIBIT       0
#define SC200X_TYPEC_VBUS_DEFAULT       5000 /* 5000 mV */
#define SC200X_TYPEC_IBUS_INHIBIT       0 /* 0 mA */
#define SC200X_TYPEC_IBUS_DEFAULT       900 /* 900 mA */
#define SC200X_TYPEC_IBUS_MIDDLE        1500 /* 1500 mA */
#define SC200X_TYPEC_IBUS_HIGH          3000 /* 3000 mA */

#define SC200X_TCPC_PR_UNKNOWN          0
#define SC200X_TCPC_PR_SOURCE           1
#define SC200X_TCPC_PR_SINK             2

/* reg=0x04, rw, role control */
#define SC200X_REG_ROLE_CTRL            0x04

#define SC200X_SRC_EN_MASK              BIT(7)
#define SC200X_SRC_EN_SHIFT             7
#define SC200X_RP_VALUE_MASK            (BIT(1) | BIT(0))
#define SC200X_RP_VALUE_SHIFT           0

#define SC200X_RP_VAL_UNKNOWN           0x00 /* no device is connected */
#define SC200X_RP_VAL_DEFAULT           0x01 /* default: 0.9A */
#define SC200X_RP_VAL_MIDDLE            0x02 /* 1.5A */
#define SC200X_RP_VAL_HIGH              0x03 /* 3.0A */

/* reg=0x14, rw, cc status */
#define SC200X_REG_CC_STATUS            0x14

#define SC200X_ATTACH_FLAG_MASK         BIT(5)
#define SC200X_ATTACH_FLAG_SHIFT        5
#define SC200X_CC_ROLE_MASK             BIT(4)
#define SC200X_CC_ROLE_SHIFT            4
#define SC200X_CC2_STATUS_MASK          (BIT(3) | BIT(2))
#define SC200X_CC2_STATUS_SHIFT         2
#define SC200X_CC1_STATUS_MASK          (BIT(1) | BIT(0))
#define SC200X_CC1_STATUS_SHIFT         0

#define SC200X_CC_ROLE_RD               0x00
#define SC200X_CC_ROLE_RP               0x01

int sc200x_tcpc_init(struct sc200x_device_info *di);
int sc200x_cc_state_alert_handler(struct sc200x_device_info *di, unsigned int alert);

#endif /* _SC200X_TCPC_H_ */
