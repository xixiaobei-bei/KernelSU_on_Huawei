/*
 * Universal Flash Storage Host controller driver
 *
 * This code is based on drivers/scsi/ufs/ufs.h
 * Copyright (C) 2011-2019 Samsung India Software Operations
 *
 * Authors:
 *	Santosh Yaraganavi <santosh.sy@samsung.com>
 *	Vinayak Holikatti <h.vinayak@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * See the COPYING file in the top-level directory or visit
 * <http://www.gnu.org/licenses/gpl-2.0.html>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This program is provided "AS IS" and "WITH ALL FAULTS" and
 * without warranty of any kind. You are solely responsible for
 * determining the appropriateness of using and distributing
 * the program and assume all risks associated with your exercise
 * of rights with respect to the program, including but not limited
 * to infringement of third party rights, the risks and costs of
 * program errors, damage to or loss of data, programs or equipment,
 * and unavailability or interruption of operations. Under no
 * circumstances will the contributor of this Program be liable for
 * any damages of any kind arising from your use or distribution of
 * this program.
 */

#ifndef _UFS_H
#define _UFS_H

#include <linux/mutex.h>
#include <linux/types.h>
#include <scsi/ufs/ufs.h>
#include "ufs_func.h"

#define MAX_CDB_SIZE	16
#define GENERAL_UPIU_REQUEST_SIZE 32
#define QUERY_DESC_MAX_SIZE       255
#define QUERY_DESC_MIN_SIZE       2
#define QUERY_DESC_HDR_SIZE       2
#define QUERY_OSF_SIZE            (GENERAL_UPIU_REQUEST_SIZE - \
					(sizeof(struct utp_upiu_header)))
#define RESPONSE_UPIU_SENSE_DATA_LENGTH	18

#define UPIU_HEADER_DWORD(byte3, byte2, byte1, byte0)\
			cpu_to_be32(((byte3) << 24) | ((byte2) << 16) |\
			 ((byte1) << 8) | (byte0))
/*
 * UFS device may have standard LUs and LUN id could be from 0x00 to
 * 0x7F. Standard LUs use "Peripheral Device Addressing Format".
 * UFS device may also have the Well Known LUs (also referred as W-LU)
 * which again could be from 0x00 to 0x7F. For W-LUs, device only use
 * the "Extended Addressing Format" which means the W-LUNs would be
 * from 0xc100 (SCSI_W_LUN_BASE) onwards.
 * This means max. LUN number reported from UFS device could be 0xC17F.
 */
#define UFS_UPIU_MAX_UNIT_NUM_ID	0x7F
#define UFS_MAX_LUNS		(SCSI_W_LUN_BASE + UFS_UPIU_MAX_UNIT_NUM_ID)
#define UFS_UPIU_WLUN_ID	(1 << 7)
#define UFS_UPIU_MAX_GENERAL_LUN	8

/* IOCTL opcode for command - ufs set device read only */
#define UFS_IOCTL_BLKROSET      BLKROSET

/* JEDEC UFS Spec version */
enum {
	UFS_DEVICE_SPEC_1_1 = 0x0110,
	UFS_DEVICE_SPEC_2_0 = 0x0200,
	UFS_DEVICE_SPEC_2_1 = 0x0210,
	UFS_DEVICE_SPEC_2_2 = 0x0220,
	UFS_DEVICE_SPEC_3_0 = 0x0300,
	UFS_DEVICE_SPEC_3_1 = 0x0310,
};

/* Well known logical unit id in LUN field of UPIU */
enum {
	UFS_UPIU_REPORT_LUNS_WLUN	= 0x81,
	UFS_UPIU_UFS_DEVICE_WLUN	= 0xD0,
	UFS_UPIU_BOOT_WLUN		= 0xB0,
	UFS_UPIU_RPMB_WLUN		= 0xC4,
};

/**
 * ufs_is_valid_unit_desc_lun - checks if the given LUN has a unit descriptor
 * @lun: LU number to check
 * @return: true if the lun has a matching unit descriptor, false otherwise
 */
static inline bool ufs_is_valid_unit_desc_lun(u8 lun)
{
	return lun == UFS_UPIU_RPMB_WLUN || (lun < UFS_UPIU_MAX_GENERAL_LUN);
}

/* Geometry descriptor parameters offsets in bytes */
enum geometry_desc_param {
	GEOMETRY_DESC_LEN			= 0x0,
	GEOMETRY_DESC_TYPE			= 0x1,
	GEOMETRY_DESC_TOTAL_DEVICE_CAPACITY	= 0x4,
	GEOMETRY_DESC_RPMB_READ_WRITE_SIZE	= 0x17,
};

/*
 * Logical Unit Write Protect
 * 00h: LU not write protected
 * 01h: LU write protected when fPowerOnWPEn =1
 * 02h: LU permanently write protected when fPermanentWPEn =1
 */
enum ufs_lu_wp_type {
	UFS_LU_NO_WP		= 0x00,
	UFS_LU_POWER_ON_WP	= 0x01,
	UFS_LU_PERM_WP		= 0x02,
};

/* bActiveICCLevel parameter current units */
enum {
	UFSHCD_NANO_AMP		= 0,
	UFSHCD_MICRO_AMP	= 1,
	UFSHCD_MILI_AMP		= 2,
	UFSHCD_AMP		= 3,
};

#define POWER_DESC_MAX_SIZE			0x62
#define POWER_DESC_MAX_ACTV_ICC_LVLS		16

/* Attribute  bActiveICCLevel parameter bit masks definitions */
#define ATTR_ICC_LVL_UNIT_OFFSET	14
#define ATTR_ICC_LVL_UNIT_MASK		(0x3 << ATTR_ICC_LVL_UNIT_OFFSET)
#define ATTR_ICC_LVL_VALUE_MASK		0x3FF

/* Power descriptor parameters offsets in bytes */
enum power_desc_param_offset {
	PWR_DESC_LEN			= 0x0,
	PWR_DESC_TYPE			= 0x1,
	PWR_DESC_ACTIVE_LVLS_VCC_0	= 0x2,
	PWR_DESC_ACTIVE_LVLS_VCCQ_0	= 0x22,
	PWR_DESC_ACTIVE_LVLS_VCCQ2_0	= 0x42,
};

/* Exception event mask values */
enum {
	MASK_EE_STATUS		   = 0xFFFF,
	/* shift 2 for EE_URGENT_BKOPS  mask bit */
	MASK_EE_URGENT_BKOPS	   = (1 << 2),
	MASK_EE_WRITEBOOSTER_EVENT = (1 << 5),
	MASK_EE_BAD_BLOCK_OCCUR	= (1 << 14),
};

/* Background operation status */
enum bkops_status {
	BKOPS_STATUS_NO_OP               = 0x0,
	BKOPS_STATUS_NON_CRITICAL        = 0x1,
	BKOPS_STATUS_PERF_IMPACT         = 0x2,
	BKOPS_STATUS_CRITICAL            = 0x3,
	BKOPS_STATUS_MAX		 = BKOPS_STATUS_CRITICAL,
};

/* Query response result code */
enum {
	QUERY_RESULT_SUCCESS                    = 0x00,
	QUERY_RESULT_NOT_READABLE               = 0xF6,
	QUERY_RESULT_NOT_WRITEABLE              = 0xF7,
	QUERY_RESULT_ALREADY_WRITTEN            = 0xF8,
	QUERY_RESULT_INVALID_LENGTH             = 0xF9,
	QUERY_RESULT_INVALID_VALUE              = 0xFA,
	QUERY_RESULT_INVALID_SELECTOR           = 0xFB,
	QUERY_RESULT_INVALID_INDEX              = 0xFC,
	QUERY_RESULT_INVALID_IDN                = 0xFD,
	QUERY_RESULT_INVALID_OPCODE             = 0xFE,
	QUERY_RESULT_GENERAL_FAILURE            = 0xFF,
};

/* UTP Transfer Request Command Type (CT) */
enum {
	UPIU_COMMAND_SET_TYPE_SCSI	= 0x0,
	UPIU_COMMAND_SET_TYPE_UFS	= 0x1,
	UPIU_COMMAND_SET_TYPE_QUERY	= 0x2,
};

/* UTP Transfer Request Command Offset */
#define UPIU_COMMAND_TYPE_OFFSET	28

/* Offset of the response code in the UPIU header */
#define UPIU_RSP_CODE_OFFSET		8

enum {
	MASK_SCSI_STATUS		= 0xFF,
	MASK_TASK_RESPONSE              = 0xFF00,
	MASK_RSP_UPIU_RESULT            = 0xFFFF,
	MASK_QUERY_DATA_SEG_LEN         = 0xFFFF,
	MASK_RSP_UPIU_DATA_SEG_LEN	= 0xFFFF,
	MASK_RSP_EXCEPTION_EVENT        = 0x10000,
	MASK_TM_SERVICE_RESP		= 0xFF,
};

/* Task management service response */
enum {
	UPIU_TASK_MANAGEMENT_FUNC_COMPL		= 0x00,
	UPIU_TASK_MANAGEMENT_FUNC_NOT_SUPPORTED = 0x04,
	UPIU_TASK_MANAGEMENT_FUNC_SUCCEEDED	= 0x08,
	UPIU_TASK_MANAGEMENT_FUNC_FAILED	= 0x05,
	UPIU_INCORRECT_LOGICAL_UNIT_NO		= 0x09,
};

/* Possible values for dExtendedUFSFeaturesSupport */
enum {
	UFS_DEV_WRITE_BOOSTER_SUP = BIT(8),
};

/* UFS device power modes */
enum ufs_dev_pwr_mode {
	UFS_ACTIVE_PWR_MODE	= 1,
	UFS_SLEEP_PWR_MODE	= 2,
	UFS_POWERDOWN_PWR_MODE	= 3,
};

/**
 * struct utp_upiu_header - UPIU header structure
 * @dword_0: UPIU header DW-0
 * @dword_1: UPIU header DW-1
 * @dword_2: UPIU header DW-2
 */
struct utp_upiu_header {
	__be32 dword_0;
	__be32 dword_1;
	__be32 dword_2;
};

/**
 * struct utp_upiu_cmd - Command UPIU structure
 * @data_transfer_len: Data Transfer Length DW-3
 * @cdb: Command Descriptor Block CDB DW-4 to DW-7
 */
struct utp_upiu_cmd {
	__be32 exp_data_transfer_len;
	u8 cdb[MAX_CDB_SIZE];
};

/**
 * struct utp_upiu_query - upiu request buffer structure for
 * query request.
 * @opcode: command to perform B-0
 * @idn: a value that indicates the particular type of data B-1
 * @index: Index to further identify data B-2
 * @selector: Index to further identify data B-3
 * @reserved_osf: spec reserved field B-4,5
 * @length: number of descriptor bytes to read/write B-6,7
 * @value: Attribute value to be written DW-5
 * @reserved: spec reserved DW-6,7
 */
struct utp_upiu_query {
	u8 opcode;
	u8 idn;
	u8 index;
	u8 selector;
	__be16 reserved_osf;
	__be16 length;
	__be32 value;
	/* reserve 2 32bits field */
	__be32 reserved[2];
};

/**
 * struct utp_upiu_req - general upiu request structure
 * @header:UPIU header structure DW-0 to DW-2
 * @sc: fields structure for scsi command DW-3 to DW-7
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_req {
	struct utp_upiu_header header;
	union {
		struct utp_upiu_cmd sc;
		struct utp_upiu_query qr;
	};
};

/**
 * struct utp_cmd_rsp - Response UPIU structure
 * @residual_transfer_count: Residual transfer count DW-3
 * @reserved: Reserved double words DW-4 to DW-7
 * @sense_data_len: Sense data length DW-8 U16
 * @sense_data: Sense data field DW-8 to DW-12
 */
struct utp_cmd_rsp {
	__be32 residual_transfer_count;
	/* reserve 4 32bits field */
	__be32 reserved[4];
	__be16 sense_data_len;
	u8 sense_data[RESPONSE_UPIU_SENSE_DATA_LENGTH];
};

/**
 * struct utp_upiu_rsp - general upiu response structure
 * @header: UPIU header structure DW-0 to DW-2
 * @sr: fields structure for scsi command DW-3 to DW-12
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_rsp {
	struct utp_upiu_header header;
	union {
		struct utp_cmd_rsp sr;
		struct utp_upiu_query qr;
	};
};

/**
 * struct utp_upiu_task_req - Task request UPIU structure
 * @header - UPIU header structure DW0 to DW-2
 * @input_param1: Input parameter 1 DW-3
 * @input_param2: Input parameter 2 DW-4
 * @input_param3: Input parameter 3 DW-5
 * @reserved: Reserved double words DW-6 to DW-7
 */
struct utp_upiu_task_req {
	struct utp_upiu_header header;
	__be32 input_param1;
	__be32 input_param2;
	__be32 input_param3;
	/* reserve 2 32bits field */
	__be32 reserved[2];
};

/**
 * struct utp_upiu_task_rsp - Task Management Response UPIU structure
 * @header: UPIU header structure DW0-DW-2
 * @output_param1: Output parameter 1 DW3
 * @output_param2: Output parameter 2 DW4
 * @reserved: Reserved double words DW-5 to DW-7
 */
struct utp_upiu_task_rsp {
	struct utp_upiu_header header;
	__be32 output_param1;
	__be32 output_param2;
	/* reserve 3 32bits field */
	__be32 reserved[3];
};

/**
 * struct ufs_query_req - parameters for building a query request
 * @query_func: UPIU header query function
 * @upiu_req: the query request data
 */
struct ufs_query_req {
	u8 query_func;
	u8 lun;
	bool has_data;
	struct utp_upiu_query upiu_req;
};

/**
 * struct ufs_query_resp - UPIU QUERY
 * @response: device response code
 * @upiu_res: query response data
 */
struct ufs_query_res {
	u8 response;
	struct utp_upiu_query upiu_res;
};

#define UFS_VREG_VCC_MIN_UV	   2700000 /* uV */
#define UFS_VREG_VCC_MAX_UV	   3600000 /* uV */
#define UFS_VREG_VCC_1P8_MIN_UV    1700000 /* uV */
#define UFS_VREG_VCC_1P8_MAX_UV    1950000 /* uV */
#define UFS_VREG_VCCQ_MIN_UV	   1100000 /* uV */
#define UFS_VREG_VCCQ_MAX_UV	   1300000 /* uV */
#define UFS_VREG_VCCQ2_MIN_UV	   1650000 /* uV */
#define UFS_VREG_VCCQ2_MAX_UV	   1950000 /* uV */

/*
 * VCCQ & VCCQ2 current requirement when UFS device is in sleep state
 * and link is in Hibern8 state.
 */
#define UFS_VREG_LPM_LOAD_UA	1000 /* uA */

struct ufs_vreg {
	struct regulator *reg;
	const char *name;
	bool enabled;
	bool unused;
	int min_uv;
	int max_uv;
	int min_ua;
	int max_ua;
};

struct ufs_vreg_info {
	struct ufs_vreg *vcc;
	struct ufs_vreg *vccq;
	struct ufs_vreg *vccq2;
	struct ufs_vreg *vdd_hba;
};

struct ufs_dev_info {
	bool f_power_on_wp_en;
	/* Keeps information if any of the LU is power on write protected */
	bool is_lu_power_on_wp;
};

#define MAX_MODEL_LEN 16
/**
 * ufs_dev_desc - ufs device details from the device descriptor
 *
 * @wmanufacturerid: card details
 * @model: card model
 */
struct ufs_dev_desc {
	u16 wmanufacturerid;
	u16 wmanufacturer_date;
	u16 spec_version;
	u8  serial_num_index;
	uint32_t vendor_feature;
	char model[MAX_MODEL_LEN + 1];
};

#define HPB_READ16_CONTROL_VALUE 0x1
#define HPB_READ16_CMDLEN  0x10
#define HPB_READ16_TRANSLEN7  0x07
#define HPB_READ16_TRANSLEN8  0x08

#define HPB_SAMSUNG_BUFID 0x11
enum hpb_read16_compose {
	HPB_READ16_OP,
	HPB_RESERVED0,
	HPB_READ16_LBA1,
	HPB_READ16_LBA2,
	HPB_READ16_LBA3,
	HPB_READ16_LBA4,
	HPB_READ16_PPN1,
	HPB_READ16_PPN2,
	HPB_READ16_PPN3,
	HPB_READ16_PPN4,
	HPB_READ16_PPN5,
	HPB_READ16_PPN6,
	HPB_READ16_PPN7,
	HPB_READ16_PPN8,
	HPB_READ16_TRANS_LEN,
	HPB_READ16_CONTROL
};

#endif /* End of Header */
