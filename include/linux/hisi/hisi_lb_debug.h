/*
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HISI_LB_DEBUG__
#define __HISI_LB_DEBUG__

#include <linux/hisi/hisi_lb.h>
#ifdef CONFIG_HISI_LB_DEBUG
#define lb_assert_page  __lb_assert_page
#define lb_assert_phys __lb_assert_phys
#define lb_assert_pte __lb_assert_pte
#else
#define lb_assert_page(page) WARN_ON(0)

#define lb_assert_phys(phys) WARN_ON(0)

#define lb_assert_pte(pte) WARN_ON(0)
#endif
#endif
