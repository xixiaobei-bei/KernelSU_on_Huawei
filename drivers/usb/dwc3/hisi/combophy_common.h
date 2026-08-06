/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Head file of ComboPHY Common Module on platform
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2  of
 * the License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __CHIP_COMBOPHY_COMMON_H__
#define __CHIP_COMBOPHY_COMMON_H__

const char *irq_type_string(enum tca_irq_type_e irq_type);
const char *mode_type_string(enum tcpc_mux_ctrl_type mode_type);
const char *dev_type_string(enum tca_device_type_e dev_type);

#endif /* __CHIP_COMBOPHY_COMMON_H__ */
