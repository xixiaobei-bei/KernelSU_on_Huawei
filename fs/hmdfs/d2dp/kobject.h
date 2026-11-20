/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2DP kobject interface
 */

#ifndef D2D_KOBJECT_H
#define D2D_KOBJECT_H

struct d2d_protocol;

int d2dp_register_kobj(struct d2d_protocol *proto);

int d2dp_kobject_init(void);
void d2dp_kobject_deinit(void);

#endif /* D2D_KOBJECT_H */
