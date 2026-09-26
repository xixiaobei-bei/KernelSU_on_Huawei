/*
 * This file define fd maps
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _IVP_MAP_H_
#define _IVP_MAP_H_

#include <linux/device.h>
#include "ivp.h"

int ivp_map_hidl_fd(struct device *dev, struct ivp_map_info *map_buf);
int ivp_unmap_hidl_fd(struct device *dev,
		struct ivp_map_info *map_buf);
#ifdef SEC_IVP_V300
int ivp_add_dma_ref(int fd);
int ivp_init_map_info(void);
void ivp_clear_map_info(void);
int ivp_init_sec_fd(struct ivp_fd_info *info);
int ivp_deinit_sec_fd(struct ivp_fd_info *info);
#endif

#endif /* _IVP_MAP_H_ */
