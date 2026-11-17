/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Head file of ComboPHY Core Module on platform
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

#ifndef __CHIP_COMBOPHY_CORE_H__
#define __CHIP_COMBOPHY_CORE_H__

#include <linux/phy/phy.h>

#if IS_ENABLED(CONFIG_CHIP_COMBOPHY)
int combophy_set_mode(struct phy *phy,
		      enum tcpc_mux_ctrl_type mode_type,
		      enum typec_plug_orien_e typec_orien);
int combophy_init(struct phy *phy);
int combophy_exit(struct phy *phy);
int combophy_register_debugfs(struct phy *phy, struct dentry *root);
void combophy_regdump(struct phy *phy);
#else
static inline int combophy_set_mode(struct phy *phy,
		      enum tcpc_mux_ctrl_type mode_type,
		      enum typec_plug_orien_e typec_orien)
{
	return 0;
}

static inline int combophy_init(struct phy *phy)
{
	return 0;
}

static inline int combophy_exit(struct phy *phy)
{
	return 0;
}

static inline int combophy_register_debugfs(struct phy *phy,
					    struct dentry *root)
{
	return 0;
}

static inline void combophy_regdump(struct phy *phy) {}
#endif /* CONFIG_CHIP_COMBOPHY */

#endif /* __CHIP_COMBOPHY_CORE_H__ */
