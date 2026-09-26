/*
 * Copyright (C) 2016 Richtek Technology Corp.
 *
 * Author: TH <tsunghan_tsai@richtek.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef PD_DPM_PDO_SELECT_H
#define PD_DPM_PDO_SELECT_H

#include "include/tcpci.h"
#include <linux/hisi/usb/hisi_tcpm.h>
#include <linux/hisi/usb/hisi_typec.h>

#define DPM_APDO_TYPE_PPS	(TCPM_POWER_CAP_APDO_TYPE_PPS)
#define DPM_APDO_TYPE_PPS_CF	(TCPM_POWER_CAP_APDO_TYPE_PPS_CF)

void hisi_dpm_extract_pdo_info(
		uint32_t pdo, struct dpm_pdo_info_t *info);

bool dpm_find_match_req_info(struct dpm_rdo_info_t *req_info,
	uint32_t snk_pdo, int cnt, uint32_t *src_pdos,
	int min_uw, uint8_t policy);

#endif
