/*
 * hisi_charger_fcp.h
 *
 * fcp define for hisi charger
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HISI_CHARGER_FCP_H_
#define _HISI_CHARGER_FCP_H_

/* fcp adapter status */
#define FCP_ADAPTER_STATUS         0x28
#define FCP_ADAPTER_OVLT           0x04
#define FCP_ADAPTER_OCURRENT       0x02
#define FCP_ADAPTER_OTEMP          0x01

struct fcp_adapter_device_ops {
	int (*get_adapter_output_current)(void);
	int (*set_adapter_output_vol)(int *);
	int (*detect_adapter)(void);
	int (*is_support_fcp)(void);
	int (*switch_chip_reset)(void);
	int (*fcp_adapter_reset)(void);
	int (*stop_charge_config)(void);
	int (*is_fcp_charger_type)(void);
	int (*fcp_read_adapter_status)(void);
	int (*fcp_read_switch_status)(void);
	void (*reg_dump)(char *);
};

#endif /* _HISI_CHARGER_FCP_H_ */
