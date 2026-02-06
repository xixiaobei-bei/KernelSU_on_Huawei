/*
 * Universal Flash Storage Host controller driver
 *
 * This code is based on drivers/scsi/ufs/ufs.h
 * Copyright (C) 2011-2013 Samsung India Software Operations
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

#ifndef _UFS_FUNC_H_
#define _UFS_FUNC_H_

#include <linux/mutex.h>
#include <linux/types.h>
#include <scsi/ufs/ufs.h>

/*
 * UFS Protocol Information Unit related definitions
 * Task management functions
 */
enum {
	UFS_ABORT_TASK		= 0x01,
	UFS_ABORT_TASK_SET	= 0x02,
	UFS_CLEAR_TASK_SET	= 0x04,
	UFS_LOGICAL_RESET	= 0x08,
	UFS_QUERY_TASK		= 0x80,
	UFS_QUERY_TASK_SET	= 0x81,
};

/* UTP UPIU Transaction Codes Initiator to Target */
enum {
	UPIU_TRANSACTION_NOP_OUT	= 0x00,
	UPIU_TRANSACTION_COMMAND	= 0x01,
	UPIU_TRANSACTION_DATA_OUT	= 0x02,
	UPIU_TRANSACTION_TASK_REQ	= 0x04,
	UPIU_TRANSACTION_QUERY_REQ	= 0x16,
};

/* UTP UPIU Transaction Codes Target to Initiator */
enum {
	UPIU_TRANSACTION_NOP_IN		= 0x20,
	UPIU_TRANSACTION_RESPONSE	= 0x21,
	UPIU_TRANSACTION_DATA_IN	= 0x22,
	UPIU_TRANSACTION_TASK_RSP	= 0x24,
	UPIU_TRANSACTION_READY_XFER	= 0x31,
	UPIU_TRANSACTION_QUERY_RSP	= 0x36,
	UPIU_TRANSACTION_REJECT_UPIU	= 0x3F,
};

/* UPIU Read/Write flags */
enum {
	UPIU_CMD_FLAGS_NONE	= 0x00,
	UPIU_CMD_FLAGS_WRITE	= 0x20,
	UPIU_CMD_FLAGS_READ	= 0x40,
};

/* UPIU Task Attributes */
enum {
	UPIU_TASK_ATTR_SIMPLE	= 0x00,
	UPIU_TASK_ATTR_ORDERED	= 0x01,
	UPIU_TASK_ATTR_HEADQ	= 0x02,
	UPIU_TASK_ATTR_ACA	= 0x03,
};

/* UPIU Command Priority */
#define UPIU_CMD_PRIO		0x04

/* UPIU Query request function */
enum {
	UPIU_QUERY_FUNC_STANDARD_READ_REQUEST           = 0x01,
	UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST          = 0x81,
};

/* Flag idn for Query Requests */
enum flag_idn {
	QUERY_FLAG_IDN_FDEVICEINIT		= 0x01,
	QUERY_FLAG_IDN_PERMANENT_WPE		= 0x02,
	QUERY_FLAG_IDN_PWR_ON_WPE		= 0x03,
	QUERY_FLAG_IDN_BKOPS_EN			= 0x04,
	QUERY_FLAG_IDN_RESERVED1		= 0x05,
	QUERY_FLAG_IDN_PURGE_ENABLE		= 0x06,
	QUERY_FLAG_IDN_RESERVED2		= 0x07,
	QUERY_FLAG_IDN_FPHYRESOURCEREMOVAL      = 0x08,
	QUERY_FLAG_IDN_BUSY_RTC			= 0x09,
	QUERY_FLAG_IDN_WB_EN                            = 0x0E,
	QUERY_FLAG_IDN_WB_BUFF_FLUSH_EN                 = 0x0F,
	QUERY_FLAG_IDN_WB_BUFF_FLUSH_DURING_HIBERN8     = 0x10,
};

/* Attribute idn for Query requests */
enum attr_idn {
	QUERY_ATTR_IDN_BOOT_LU_EN		= 0x00,
	QUERY_ATTR_IDN_RESERVED			= 0x01,
	QUERY_ATTR_IDN_POWER_MODE		= 0x02,
	QUERY_ATTR_IDN_ACTIVE_ICC_LVL		= 0x03,
	QUERY_ATTR_IDN_OOO_DATA_EN		= 0x04,
	QUERY_ATTR_IDN_BKOPS_STATUS		= 0x05,
	QUERY_ATTR_IDN_PURGE_STATUS		= 0x06,
	QUERY_ATTR_IDN_MAX_DATA_IN		= 0x07,
	QUERY_ATTR_IDN_MAX_DATA_OUT		= 0x08,
	QUERY_ATTR_IDN_DYN_CAP_NEEDED		= 0x09,
	QUERY_ATTR_IDN_REF_CLK_FREQ		= 0x0A,
	QUERY_ATTR_IDN_CONF_DESC_LOCK		= 0x0B,
	QUERY_ATTR_IDN_MAX_NUM_OF_RTT		= 0x0C,
	QUERY_ATTR_IDN_EE_CONTROL		= 0x0D,
	QUERY_ATTR_IDN_EE_STATUS		= 0x0E,
	QUERY_ATTR_IDN_SECONDS_PASSED		= 0x0F,
	QUERY_ATTR_IDN_CNTX_CONF		= 0x10,
	QUERY_ATTR_IDN_CORR_PRG_BLK_NUM		= 0x11,
#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
	/* Hynix GC related idn */
	QUERY_ATTR_IDN_M_GC_START_STOP		= 0x12,
	QUERY_ATTR_IDN_M_GC_STATUS		= 0x13,
#endif
	QUERY_ATTR_IDN_REFCLK_GATE_TIME		= 0x17,
	QUERY_ATTR_IDN_WB_FLUSH_STATUS		= 0x1C,
	QUERY_ATTR_IDN_WB_AVAILABLE_BUFFER_SIZE = 0x1D,
	QUERY_ATTR_IDN_WB_BUFFER_LIFETIME_EST   = 0x1E,
	QUERY_ATTR_IDN_WB_CURRENT_BUFFER_SIZE   = 0x1F,
	QUERY_ATTR_IDN_VENDOR_FEATURE_OPTIONS	= 0xFF,
};

enum wb_flush_status {
	WB_FLUSH_IDLE        = 0x0,
	WB_FLUSH_IN_PROGRESS = 0x1,
	WB_FLUSH_STOPPED     = 0x2,
	WB_FLUSH_COMPLETE    = 0x3,
};

#define WB_T_BUFFER_THRESHOLD 0x3

enum wb_type {
	WB_DEDICATED     = 0x0,
	WB_SINGLE_SHARED = 0x1,
};

#define QUERY_ATTR_IDN_BOOT_LU_EN_MAX	0x02

/* Descriptor idn for Query requests */
enum desc_idn {
	QUERY_DESC_IDN_DEVICE		= 0x0,
	QUERY_DESC_IDN_CONFIGURATION	= 0x1,
	QUERY_DESC_IDN_UNIT		= 0x2,
	QUERY_DESC_IDN_RFU_0		= 0x3,
	QUERY_DESC_IDN_INTERCONNECT	= 0x4,
	QUERY_DESC_IDN_STRING		= 0x5,
	QUERY_DESC_IDN_RFU_1		= 0x6,
	QUERY_DESC_IDN_GEOMETRY		= 0x7,
	QUERY_DESC_IDN_POWER		= 0x8,
	QUERY_DESC_IDN_HEALTH           = 0x9,
	QUERY_DESC_IDN_RFU_2		= 0xA,
	QUERY_DESC_IDN_TT_UNIT		= 0xFD,
	QUERY_DESC_IDN_DEVICE_UNISTORE  = 0xFE,
	QUERY_DESC_IDN_UNIT_UNISTORE    = 0xFF,

#ifdef CONFIG_JOURNAL_DATA_TAG
	QUERY_DESC_IDN_VENDOR		= 0xFF,
	QUERY_DESC_IDN_MAX		= 0x100,
#else
	QUERY_DESC_IDN_MAX,
#endif
};

enum desc_header_offset {
	QUERY_DESC_LENGTH_OFFSET	= 0x00,
	QUERY_DESC_DESC_TYPE_OFFSET	= 0x01,
};

enum ufs_desc_def_size {
	QUERY_DESC_DEVICE_DEF_SIZE		= 0x5F,
	QUERY_DESC_CONFIGURATION_DEF_SIZE	= 0xE6,
	QUERY_DESC_UNIT_DEF_SIZE		= 0x2D,
	QUERY_DESC_INTERCONNECT_DEF_SIZE	= 0x06,
	QUERY_DESC_GEOMETRY_DEF_SIZE		= 0x59,
	QUERY_DESC_POWER_DEF_SIZE		= 0x62,
};

enum ufs_desc_max_size {
	QUERY_DESC_DEVICE_MAX_SIZE		= 0xFF,
	QUERY_DESC_CONFIGURAION_MAX_SIZE	= 0xE6,
	QUERY_DESC_UNIT_MAX_SIZE		= 0x2D,
	QUERY_DESC_INTERCONNECT_MAX_SIZE	= 0x06,
	/*
	 * Max. 126 UNICODE characters (2 bytes per character) plus 2 bytes
	 * of descriptor header.
	 */
	QUERY_DESC_STRING_MAX_SIZE		= 0xFE,
	QUERY_DESC_GEOMETRY_MAX_SIZE		= 0x59,
	QUERY_DESC_RPMB_UNIT_MAZ_SIZE		= 0x22,
	QUERY_DESC_POWER_MAX_SIZE		= 0x62,
	QUERY_DESC_HEALTH_MAX_SIZE		= 0x37,
	QUERY_DESC_RFU_MAX_SIZE			= 0x00,
	QUERY_DESC_TTUNIT_MAX_SIZE		= 0x04,
	QUERY_DESC_DEVICE_UNISTORE_MAX_SIZE 	= 0x1A,
	QUERY_DESC_UINT_UNISTORE_MAX_SIZE	= 0x20,
};

/* Unit descriptor parameters offsets in bytes */
enum unit_desc_param {
	UNIT_DESC_PARAM_LEN			= 0x0,
	UNIT_DESC_PARAM_TYPE			= 0x1,
	UNIT_DESC_PARAM_UNIT_INDEX		= 0x2,
	UNIT_DESC_PARAM_LU_ENABLE		= 0x3,
	UNIT_DESC_PARAM_BOOT_LUN_ID		= 0x4,
	UNIT_DESC_PARAM_LU_WR_PROTECT		= 0x5,
	UNIT_DESC_PARAM_LU_Q_DEPTH		= 0x6,
	UNIT_DESC_PARAM_MEM_TYPE		= 0x8,
	UNIT_DESC_PARAM_DATA_RELIABILITY	= 0x9,
	UNIT_DESC_PARAM_LOGICAL_BLK_SIZE	= 0xA,
	UNIT_DESC_PARAM_LOGICAL_BLK_COUNT	= 0xB,
	UNIT_DESC_PARAM_ERASE_BLK_SIZE		= 0x13,
	UNIT_DESC_PARAM_PROVISIONING_TYPE	= 0x17,
	UNIT_DESC_PARAM_PHY_MEM_RSRC_CNT	= 0x18,
	UNIT_DESC_PARAM_CTX_CAPABILITIES	= 0x20,
	UNIT_DESC_PARAM_LARGE_UNIT_SIZE_M1	= 0x22,
	UNIT_DESC_PARAM_WB_BUF_ALLOC_UNITS	= 0x29,
};

/* RPMB Unit descriptor parameters offsets in bytes */
enum rpmb_unit_desc_param {
	RPMB_UNIT_DESC_PARAM_RPMB_REGION_ENABLE	= 0x9, /* Rerserved:1bit(00h) */
	RPMB_UNIT_DESC_PARAM_LOGICAL_BLK_SIZE	= 0xA,
	RPMB_UNIT_DESC_PARAM_LOGICAL_BLK_COUNT	= 0xB,
	/* dEraseBlockSize:4bit(00h) */
	RPMB_UNIT_DESC_PARAM_RPMB_REGION0_SIZE	= 0x13,
	/* dEraseBlockSize:4bit(00h) */
	RPMB_UNIT_DESC_PARAM_RPMB_REGION1_SIZE	= 0x14,
	/* dEraseBlockSize:4bit(00h) */
	RPMB_UNIT_DESC_PARAM_RPMB_REGION2_SIZE	= 0x15,
	/* dEraseBlockSize:4bit(00h) */
	RPMB_UNIT_DESC_PARAM_RPMB_REGION3_SIZE	= 0x16,
};

/* Device descriptor parameters offsets in bytes */
enum device_desc_param {
	DEVICE_DESC_PARAM_LEN			= 0x0,
	DEVICE_DESC_PARAM_TYPE			= 0x1,
	DEVICE_DESC_PARAM_DEVICE_TYPE		= 0x2,
	DEVICE_DESC_PARAM_DEVICE_CLASS		= 0x3,
	DEVICE_DESC_PARAM_DEVICE_SUB_CLASS	= 0x4,
	DEVICE_DESC_PARAM_PRTCL			= 0x5,
	DEVICE_DESC_PARAM_NUM_LU		= 0x6,
	DEVICE_DESC_PARAM_NUM_WLU		= 0x7,
	DEVICE_DESC_PARAM_BOOT_ENBL		= 0x8,
	DEVICE_DESC_PARAM_DESC_ACCSS_ENBL	= 0x9,
	DEVICE_DESC_PARAM_INIT_PWR_MODE		= 0xA,
	DEVICE_DESC_PARAM_HIGH_PR_LUN		= 0xB,
	DEVICE_DESC_PARAM_SEC_RMV_TYPE		= 0xC,
	DEVICE_DESC_PARAM_SEC_LU		= 0xD,
	DEVICE_DESC_PARAM_BKOP_TERM_LT		= 0xE,
	DEVICE_DESC_PARAM_ACTVE_ICC_LVL		= 0xF,
	DEVICE_DESC_PARAM_SPEC_VER		= 0x10,
	DEVICE_DESC_PARAM_MANF_DATE		= 0x12,
	DEVICE_DESC_PARAM_MANF_NAME		= 0x14,
	DEVICE_DESC_PARAM_PRDCT_NAME		= 0x15,
	DEVICE_DESC_PARAM_SN			= 0x16,
	DEVICE_DESC_PARAM_OEM_ID		= 0x17,
	DEVICE_DESC_PARAM_MANF_ID		= 0x18,
	DEVICE_DESC_PARAM_UD_OFFSET		= 0x1A,
	DEVICE_DESC_PARAM_UD_LEN		= 0x1B,
	DEVICE_DESC_PARAM_RTT_CAP		= 0x1C,
	DEVICE_DESC_PARAM_FRQ_RTC		= 0x1D,
	DEVICE_DESC_PARAM_EXT_UFS_FEATURE_SUP	= 0x4F,
	DEVICE_DESC_PARAM_WB_TYPE		= 0x54,
	DEVICE_DESC_PARAM_WB_SHARED_ALLOC_UNITS = 0x55,
	DEVICE_DESC_PARAM_FEATURE		= 0xF0,
};

enum health_device_desc_param {
	HEALTH_DEVICE_DESC_PARAM_LEN		= 0x0,
	HEALTH_DEVICE_DESC_PARAM_TYPE		= 0x1,
	HEALTH_DEVICE_DESC_PARAM_PREEOL		= 0x2,
	HEALTH_DEVICE_DESC_PARAM_LIFETIMEA	= 0x3,
	HEALTH_DEVICE_DESC_PARAM_LIFETIMEB	= 0x4,
	HEALTH_DEVICE_DESC_PARAM_RESERVED	= 0x5,
};

/* QUERY VCMD READ 0xF8 */
enum {
	QUERY_TZ_IDN_CAP_INFO			= 0x8,
	QUERY_TZ_IDN_BLK_INFO			= 0x9,
	QUERY_PWRON_INFO_SYNC 			= 0x20,
	QUERY_OOB_INFO_FETCH 			= 0x21,
};

/* QUERY VCMD WRITE 0xF9 */
enum {
	QUERY_VENDOR_WRITE_IDN_PGR		= 0x7,
	QUERY_VENDOR_RESET_FTL			= 0x20,
	QUERY_VENDOR_DATA_MOVE			= 0x21,
	QUERY_VENDOR_SLC_MODE_CONFIGURATION	= 0x22,
	QUERY_VENDOR_FS_SYNC_DONE		= 0x23,
	QUERY_RESCUE_BLOCK_INJECT_DATA		= 0x24,
	QUERY_SYNC_READ_VERIFY			= 0x25,
	QUERY_BAD_BLOCK_ERROR_INJECT		= 0x27,
};

#endif
