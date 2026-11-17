/*
 * wireless_test_mmi.h
 *
 * common interface, variables, definition etc for wireless charge test
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _WIRELESS_TEST_MMI_H_
#define _WIRELESS_TEST_MMI_H_

#define WLRX_MMI_IOUT_HOLD_TIME       5
#define WLRX_MMI_TO_BUFF              5 /* timeout buffer: 5s */

#define WLRX_MMI_DFLT_EX_ICON         1
#define WLRX_MMI_DFLT_EX_PROT         7
#define WLRX_MMI_DFLT_EX_TIMEOUT      10

#define WLRX_MMI_PROT_HANDSHAKE       BIT(0)
#define WLRX_MMI_PROT_TX_CAP          BIT(1)
#define WLRX_MMI_PROT_CERT            BIT(2)

enum wlrx_mmi_sysfs_type {
	WLRX_MMI_SYSFS_BEGIN = 0,
	WLRX_MMI_SYSFS_START = WLRX_MMI_SYSFS_BEGIN,
	WLRX_MMI_SYSFS_TIMEOUT,
	WLRX_MMI_SYSFS_RESULT,
	WLRX_MMI_SYSFS_END,
};

enum wlrx_mmi_test_para {
	WLRX_MMI_PARA_BEGIN = 0,
	WLRX_MMI_PARA_TIMEOUT = WLRX_MMI_PARA_BEGIN,
	WLRX_MMI_PARA_EXPT_PORT,
	WLRX_MMI_PARA_EXPT_ICON,
	WLRX_MMI_PARA_IOUT_LTH,
	WLRX_MMI_PARA_VMAX_LTH,
	WLRX_MMI_PARA_END,
};

struct wlrx_mmi_para {
	int timeout;
	u32 expt_prot;
	int expt_icon;
	int iout_lth;
	int vmax_lth;
	int reserved;
};

struct wlrx_mmi_data {
	int vmax;
	int icon;
	u32 prot_state;
	bool iout_first_match;
	struct notifier_block wlrx_mmi_nb;
	struct delayed_work timeout_work;
	struct wlrx_mmi_para dts_para;
};

#endif /* _WIRELESS_TEST_MMI_H_ */
