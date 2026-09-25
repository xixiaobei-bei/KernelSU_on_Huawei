/*
 * wlc_mmi.h
 *
 * common interface, varibles, definiton etc for wireless charge test
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

#ifndef _WLC_MMI_H_
#define _WLC_MMI_H_

#define WLC_MMI_IOUT_HOLD_TIME       5
#define WLC_MMI_TO_BUFF              5 /* timeout buffer: 5s */

#define WLC_MMI_DFLT_EX_ICON         1
#define WLC_MMI_DFLT_EX_PROT         7
#define WLC_MMI_DFLT_EX_TIMEOUT      10

#define WLC_MMI_PROT_HANDSHAKE       BIT(0)
#define WLC_MMI_PROT_TX_CAP          BIT(1)
#define WLC_MMI_PROT_CERT            BIT(2)

enum wlc_mmi_sysfs_type {
	WLC_MMI_SYSFS_BEGIN = 0,
	WLC_MMI_SYSFS_START = WLC_MMI_SYSFS_BEGIN,
	WLC_MMI_SYSFS_TIMEOUT,
	WLC_MMI_SYSFS_RESULT,
	WLC_MMI_SYSFS_END,
};

enum wlc_mmi_test_para {
	WLC_MMI_PARA_BEGIN = 0,
	WLC_MMI_PARA_TIMEOUT = WLC_MMI_PARA_BEGIN,
	WLC_MMI_PARA_EXPT_PORT,
	WLC_MMI_PARA_EXPT_ICON,
	WLC_MMI_PARA_IOUT_LTH,
	WLC_MMI_PARA_VMAX_LTH,
	WLC_MMI_PARA_MAX,
};

struct wlc_mmi_para {
	int timeout;
	u32 expt_prot;
	int expt_icon;
	int iout_lth;
	int vmax_lth;
	int reserved;
};

struct wlc_mmi_data {
	int vmax;
	int icon;
	u32 prot_state;
	bool iout_first_match;
	struct notifier_block wlc_mmi_nb;
	struct delayed_work timeout_work;
	struct wlc_mmi_para dts_para;
};

#endif /* _WLC_MMI_H_ */
