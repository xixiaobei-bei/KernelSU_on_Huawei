/*
 * bitfield_translators.c
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

#include "vdm_types.h"
#include "bitfield_translators.h"
#include <linux/string.h>

/*
 * determines, based on the 32-bit header
 * whether a VDM is structured or unstructured
 */
enum pd_vdm_type get_vdm_type(u32 in)
{
	struct pd_unstructured_vdm_header vdm_header =
		get_unstructured_vdm_header(in);

	return vdm_header.vdm_type;
}

/*
 * process a 32 bit field and parses it for VDM Header data
 * returns Unstructured VDM Header struct
 * 16:bit 16~31 indicate svid
 * 15:bit 15 indicate vdm type
 * 0x00000001,0x0000FFFF, 0x00007FFF are masks
 */
struct pd_unstructured_vdm_header get_unstructured_vdm_header(u32 in)
{
	struct pd_unstructured_vdm_header ret;

	ret.svid = (in >> 16) & 0x0000FFFF;
	ret.vdm_type = (in >> 15) & 0x00000001;
	ret.info = in & 0x00007FFF;

	return ret;
}

/*
 * Turn the internal Unstructured VDM Header struct
 * representation into a 32 bit field
 * 16:bit 16 indicate svid
 * 15:bit 15 indicate vdm type
 */
u32 get_bits_unstructured_vdm_header(struct pd_unstructured_vdm_header in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.svid;
	ret |= tmp << 16;

	tmp = in.vdm_type;
	ret |= tmp << 15;

	tmp = in.info;
	ret |= tmp;

	return ret;
}

/*
 * process a 32 bit field and return a parsed StructuredVdmHeader
 * assumes that the 32 bits are actually structured and not unstructured.
 * returns parsed Structured VDM Header Struct
 * 16:bit 16~31 indicate svid
 * 15:bit 15 indicate vdm type
 * 13:bit 13~14 indicate svdm version
 * 8:bit 8~10 indicate obj position
 * 6:bit 6~7 indicate cmd type
 * 0x00000001,0x00000003,0x00000007,0x0000FFFF, 0x0000001F are masks
 */
struct pd_structured_vdm_header get_structured_vdm_header(u32 in)
{
	struct pd_structured_vdm_header ret;

	ret.svid = (in >> 16) & 0x0000FFFF;
	ret.vdm_type = (in >> 15) & 0x00000001;
	ret.svdm_version = (in >> 13) & 0x00000003;
	ret.obj_pos = (in >> 8) & 0x00000007;
	ret.cmd_type = (in >> 6) & 0x00000003;
	ret.command = in & 0x0000001F;

	return ret;
}

/*
 * Turn the internal Structured VDM Header struct
 * representation into a 32 bit field
 * 16:bit 16 indicate svid
 * 15:bit 15 indicate vdm type
 * 13:bit 13 indicate svdm version
 * 8:bit 8 indicate obj position
 * 6:bit 6 indicate cmd type
 */
u32 get_bits_structured_vdm_header(struct pd_structured_vdm_header in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.svid;
	ret |= tmp << 16;

	tmp = in.vdm_type;
	ret |= tmp << 15;

	tmp = in.svdm_version;
	ret |= tmp << 13;

	tmp = in.obj_pos;
	ret |= tmp << 8;

	tmp = in.cmd_type;
	ret |= tmp << 6;

	tmp = in.command;
	ret |= tmp;

	return ret;
}

/*
 * process a 32 bit field and parses it for ID Header data
 * returns parsed ID Header struct
 * 31:bit 31 indicate usb host data capable
 * 30:bit 30 indicate usb device data capable
 * 27:bit 27~29 indicate product type
 * 26:bit 26 indicate modal operation supported
 * 0x00000001,0x00000007,0x0000FFFF are masks
 */
struct pd_id_header get_id_header(u32 in)
{
	struct pd_id_header ret;

	ret.usb_host_data_capable = (in >> 31) & 0x00000001;
	ret.usb_device_data_capable = (in >> 30) & 0x00000001;
	ret.product_type = (in >> 27) & 0x00000007;
	ret.modal_op_supported = (in >> 26) & 0x00000001;
	ret.usb_vid = in & 0x0000FFFF;

	return ret;
}

/*
 * Turn the internal Structured VDM Header struct
 * representation into a 32 bit field
 * 31:bit 31 indicate usb host data capable
 * 30:bit 30 indicate usb device data capable
 * 27:bits 27~29 indicate product type
 * 26:bit 26 indicate modal op supported
 */
u32 get_bits_id_header(struct pd_id_header in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.usb_host_data_capable;
	ret |= tmp << 31;

	tmp = in.usb_device_data_capable;
	ret |= tmp << 30;

	tmp = in.product_type;
	ret |= tmp << 27;

	tmp = in.modal_op_supported;
	ret |= tmp << 26;

	tmp = in.usb_vid;
	ret |= tmp;

	return ret;
}

/*
 * process a 32 bit field and parses it for Product VDO data
 * returns parsed Product VDO struct
 * 16:bits 16~31 indicate svid
 * 0x0000FFFF is mask
 */
struct pd_product_vdo get_product_vdo(u32 in)
{
	struct pd_product_vdo ret;

	ret.usb_product_id = (in >> 16) & 0x0000FFFF;
	ret.bcd_device = in & 0x0000FFFF;

	return ret;
}

/*
 * Turn the internal Product VDO struct representation into a 32 bit field
 * 16:bits 16~31 storage usb product id
 */
u32 get_bits_product_vdo(struct pd_product_vdo in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.usb_product_id;
	ret |= tmp << 16;

	tmp = in.bcd_device;
	ret |= tmp;

	return ret;
}

/*
 * process a 32 bit field and parses it for Cert Stat VDO data
 * returns parsed Cert Stat VDO struct
 * 0x0000FFFF is mask
 */
struct pd_cert_stat_vdo get_cert_stat_vdo(u32 in)
{
	struct pd_cert_stat_vdo ret;

	ret.test_id = in & 0x000FFFFF;

	return ret;
}

/* Turn the internal Cert Stat VDO struct representation into a 32 bit field */
u32 get_bits_cert_stat_vdo(struct pd_cert_stat_vdo in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.test_id;
	ret |= tmp;

	return ret;
}

/*
 * process a 32 bit field and parses it for Cable VDO data
 * returns parsed Cable VDO struct
 * 28:bits 28~31 indicate cable hw version
 * 24:bist 24~31 indicate cable fw version
 * 18:bits 18~19 indicate cable to type
 * 17:bit 17 indicate cable_to pr
 * 13:bits 13~31 indicate cable latency
 * 11:bits 11~12 indicate cable term
 * 10/9:bit 10/9 indicate sstx1_dir_supp/sstx2_dir_supp
 * 8/7:bit 8/7 indicate ssrx1_dir_supp/ssrx2_dir_supp
 * 5:bits 5~6 indicate vbus current handling cap
 * 4:bit 4 indicate vbus thru cable
 * 3:bit 3 indicate sop2 presence
 * 0x0000000F,0x00000001,0x00000003,0x00000007 are masks
 */
struct pd_cable_vdo get_cable_vdo(u32 in)
{
	struct pd_cable_vdo ret;

	ret.cable_hw_version = (in >> 28) & 0x0000000F;
	ret.cable_fw_version = (in >> 24) & 0x0000000F;
	ret.cable_to_type = (in >> 18) & 0x00000003;
	ret.cable_to_pr = (in >> 17) & 0x00000001;
	ret.cable_latency = (in >> 13) & 0x0000000F;
	ret.cable_term = (in >> 11) & 0x00000003;
	ret.sstx1_dir_supp = (in >> 10) & 0x00000001;
	ret.sstx2_dir_supp = (in >> 9) & 0x00000001;
	ret.ssrx1_dir_supp = (in >> 8) & 0x00000001;
	ret.ssrx2_dir_supp = (in >> 7) & 0x00000001;
	ret.vbus_current_handling_cap = (in >> 5) & 0x00000003;
	ret.vbus_thru_cable = (in >> 4) & 0x00000001;
	ret.sop2_presence = (in >> 3) & 0x00000001;
	ret.usb_ss_supp = in & 0x00000007;

	return ret;
}

/*
 * turn the internal Cable VDO representation into a 32 bit field
 * 28:bits 28~31 indicate cable_hw_version
 * 24:bits 24~27 indicate cable_fw_version
 * 18:bits 18~23 indicate cable_to_type
 * 17:bit 17 indicate cable_to_pr
 * 13:bits 13~16 indicate cable_latency
 * 11:bits 11~12 indicate cable_term
 * 10/9:bit 10/9 indicate sstx1_dir_supp/sstx2_dir_supp
 * 8/7:bit 8/7 indicate ssrx1_dir_supp/ssrx2_dir_supp
 * 5:bits 5~6 indicate vbus_current_handling_cap
 * 4:bit 4 indicate vbus_thru_cable
 * 3:bit 3 indicate sop2_presence
 */
u32 get_bits_cable_vdo(struct pd_cable_vdo in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.cable_hw_version;
	ret |= tmp << 28;

	tmp = in.cable_fw_version;
	ret |= tmp << 24;

	tmp = in.cable_to_type;
	ret |= tmp << 18;

	tmp = in.cable_to_pr;
	ret |= tmp << 17;

	tmp = in.cable_latency;
	ret |= tmp << 13;

	tmp = in.cable_term;
	ret |= tmp << 11;

	tmp = in.sstx1_dir_supp;
	ret |= tmp << 10;

	tmp = in.sstx2_dir_supp;
	ret |= tmp << 9;

	tmp = in.ssrx1_dir_supp;
	ret |= tmp << 8;

	tmp = in.ssrx2_dir_supp;
	ret |= tmp << 7;

	tmp = in.vbus_current_handling_cap;
	ret |= tmp << 5;

	tmp = in.vbus_thru_cable;
	ret |= tmp << 4;

	tmp = in.sop2_presence;
	ret |= tmp << 3;

	tmp = in.usb_ss_supp;
	ret |= tmp;

	return ret;
}

/*
 * process a 32 bit field and parses it for AMA VDO data
 * returns parsed AMA VDO struct
 */
struct pd_ama_vdo get_ama_vdo(u32 in)
{
	struct pd_ama_vdo ret;

	memset(&ret, 0, sizeof(ret));

	ret.cable_hw_version = (in >> CABLE_HW_VERSION_OFFSET) &
		CABLE_HW_VERSION_MASK;
	ret.cable_fw_version = (in >> CABLE_FW_VERSION_OFFSET) &
		CABLE_FW_VERSION_MASK;
	ret.vdo_version = (in >> VDO_VERSION_OFFSET) & VDO_VERSION_MASK;
	ret.vconn_full_power = (in >> VCONN_FULL_POWER_OFFSET) &
		VCONN_FULL_POWER_MASK;
	ret.vconn_requirement = (in >> CVONN_REQUIREMENT_OFFSET) &
		CVONN_REQUIREMENT_MASK;
	ret.vbus_requirement = (in >> VBUS_REQUIREMENT_OFFSET) &
		VBUS_REQUIREMENT_MASK;
	ret.usb_ss_supp = (in >> USB_SS_SUPP_OFFSET) &
		USB_SS_SUPP_MASK;

	return ret;
}

/* turn the internal AMA VDO representation into a 32 bit field */
u32 get_bits_ama_vdo(struct pd_ama_vdo in)
{
	u32 ret = 0;
	u32 tmp;

	tmp = in.cable_hw_version;
	ret |= (tmp << CABLE_HW_VERSION_OFFSET);

	tmp = in.cable_fw_version;
	ret |= (tmp << CABLE_FW_VERSION_OFFSET);

	tmp = in.vdo_version;
	ret |= (tmp << VDO_VERSION_OFFSET);

	tmp = in.vconn_full_power;
	ret |= (tmp << VCONN_FULL_POWER_OFFSET);

	tmp = in.vconn_requirement;
	ret |= (tmp << CVONN_REQUIREMENT_OFFSET);

	tmp = in.vbus_requirement;
	ret |= (tmp << VBUS_REQUIREMENT_OFFSET);

	tmp = in.usb_ss_supp;
	ret |= (tmp << USB_SS_SUPP_OFFSET);

	return ret;
}
#endif /* FSC_HAVE_VDM */
