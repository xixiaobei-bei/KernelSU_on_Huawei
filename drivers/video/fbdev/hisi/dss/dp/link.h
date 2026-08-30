/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#ifndef __DPTX_LINK_H__
#define __DPTX_LINK_H__
#include "../hisi_dp.h"

/**
 * Link training
 */
int dptx_link_training(struct dp_ctrl *dptx, uint8_t rate, uint8_t lanes);
int dptx_link_check_status(struct dp_ctrl *dptx);
int dptx_link_set_ssc(struct dp_ctrl *dptx, bool bswitchphy);
int dptx_link_adjust_drive_settings(struct dp_ctrl *dptx, int *out_changed);
int dptx_link_retraining(struct dp_ctrl *dptx, u8 rate, u8 lanes);
#endif
