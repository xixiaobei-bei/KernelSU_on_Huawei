/*
 * ivp rdr head file
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

#ifndef IVP_RDR_H_
#define IVP_RDR_H_
#include "ivp_platform.h"

int ivp_rdr_init(struct ivp_device *pdev);
int ivp_rdr_deinit(void);

#endif

