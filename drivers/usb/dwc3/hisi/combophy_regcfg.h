/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Head file of register config for USB
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

#ifndef __COMBOPHY_REGCFG_H__
#define __COMBOPHY_REGCFG_H__

void combophy_regcfg_reset_misc(void);
void combophy_regcfg_unreset_misc(void);
bool combophy_regcfg_is_misc_ctrl_unreset(void);
bool combophy_regcfg_is_misc_ctrl_clk_en(void);
void combophy_regcfg_phyreset(void);
void combophy_regcfg_phyunreset(void);
void combophy_regcfg_isodis(void);
void combophy_regcfg_exit_testpowerdown(void);
void combophy_regcfg_power_stable(void);
void combophy_regcfg_enter_testpowerdown(void);
bool combophy_regcfg_is_controller_ref_clk_en(void);
bool combophy_regcfg_is_controller_bus_clk_en(void);

#endif /* __COMBOPHY_REGCFG_H__ */
