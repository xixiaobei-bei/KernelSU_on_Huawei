/*
 * hisi mntn test header file
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __HISI_MNTN_TEST_H__
#define __HISI_MNTN_TEST_H__

#ifdef CONFIG_HISI_DEBUG_FS
int mntn_test_startup_panic(void);
int mntn_test_shutdown_deadloop(void);
#endif

#endif
