/*
 * render_rt.h
 *
 * Copyright (c) 2019, Huawei Technologies Co., Ltd.
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

#ifndef __RENDER_RT_H
#define __RENDER_RT_H

#ifdef CONFIG_HISI_RENDER_RT
extern int init_render_rthread(const void __user *uarg);
extern int destroy_render_rthread(const void __user *uarg);
extern int stop_render_rthread(const void __user *uarg);
extern int get_render_rthread(void __user *uarg);
extern int get_render_hrthread(void __user *uarg);
#else
static inline int init_render_rthread(const void __user *uarg) { return -ENODEV; }
static inline int destroy_render_rthread(const void __user *uarg) { return -ENODEV; }
static inline int stop_render_rthread(const void __user *uarg) { return -ENODEV; }
static inline int get_render_rthread(void __user *uarg)  { return -ENODEV; }
static inline int get_render_hrthread(void __user *uarg)  { return -ENODEV; }
#endif /* CONFIG_HISI_RENDER_RT */

#endif /* __RENDER_RT_H  */
