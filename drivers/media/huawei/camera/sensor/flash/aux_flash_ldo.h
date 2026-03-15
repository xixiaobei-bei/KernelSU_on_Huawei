/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _AUX_FLASH_LDO_H_
#define _AUX_FLASH_LDO_H_

/***************extern function declare******************/

int aux_ldo_supply_enable(struct hw_flash_ctrl_t *flash_ctrl, unsigned int config_val);
int aux_ldo_supply_disable(void);

#endif /*_AUX_FLASH_LDO_H_*/
