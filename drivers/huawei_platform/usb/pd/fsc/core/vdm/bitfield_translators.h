/*
 * bitfield_translators.h
 *
 * bitfield_translators driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifdef FSC_HAVE_VDM

#ifndef _BITFIELD_TRANSLATORS_H_
#define _BITFIELD_TRANSLATORS_H_

#include "../platform.h"

#define CABLE_HW_VERSION_OFFSET 28
#define CABLE_FW_VERSION_OFFSET 24
#define VDO_VERSION_OFFSET 21
#define VCONN_FULL_POWER_OFFSET 5
#define CVONN_REQUIREMENT_OFFSET 4
#define VBUS_REQUIREMENT_OFFSET 3
#define USB_SS_SUPP_OFFSET 0

#define CABLE_HW_VERSION_MASK 0x0000000F
#define CABLE_FW_VERSION_MASK 0x0000000F
#define VDO_VERSION_MASK 0x00000007
#define VCONN_FULL_POWER_MASK 0x00000007
#define CVONN_REQUIREMENT_MASK 0x00000001
#define VBUS_REQUIREMENT_MASK 0x00000001
#define USB_SS_SUPP_MASK 0x00000007

/* Functions that convert bits into internal header representations */
/* converts 32 bits into an unstructured vdm header struct */
struct pd_unstructured_vdm_header get_unstructured_vdm_header(u32 in);
/* converts 32 bits into a structured vdm header struct */
struct pd_structured_vdm_header get_structured_vdm_header(u32 in);
/* converts 32 bits into an ID Header struct */
struct pd_id_header get_id_header(u32 in);
/* returns structured/unstructured vdm type */
enum pd_vdm_type get_vdm_type(u32 in);

/* Functions that convert internal header representations into bits */
/* converts unstructured vdm header struct into 32 bits */
u32 get_bits_unstructured_vdm_header(struct pd_unstructured_vdm_header in);
/* converts structured vdm header struct into 32 bits */
u32 get_bits_structured_vdm_header(struct pd_structured_vdm_header in);
/* converts ID Header struct into 32 bits */
u32 get_bits_id_header(struct pd_id_header in);

/* Functions that convert bits into internal VDO representations */
struct pd_cert_stat_vdo get_cert_stat_vdo(u32 in);
struct pd_product_vdo get_product_vdo(u32 in);
struct pd_cable_vdo get_cable_vdo(u32 in);
struct pd_ama_vdo get_ama_vdo(u32 in);

/* Functions that convert internal VDO representations into bits */
/* converts Product VDO struct into 32 bits */
u32 get_bits_product_vdo(struct pd_product_vdo in);
/* converts Cert Stat VDO struct into 32 bits */
u32 get_bits_cert_stat_vdo(struct pd_cert_stat_vdo in);
/* converts Cable VDO struct into 32 bits */
u32 get_bits_cable_vdo(struct pd_cable_vdo in);
/* converts AMA VDO struct into 32 bits */
u32 get_bits_ama_vdo(struct pd_ama_vdo in);
#endif /* _BITFIELD_TRANSLATORS_H_ */
#endif /* FSC_HAVE_VDM */
