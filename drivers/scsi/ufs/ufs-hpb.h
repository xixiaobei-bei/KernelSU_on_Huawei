/*
 * ufs-hpb.h
 *
 * ufs hpb configuration
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _UFSHPB_H_
#define _UFSHPB_H_

#include <linux/circ_buf.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include "ufshcd.h"
#include "ufs.h"

#define ZERO_BYTE_SHIFT 0
#define ONE_BYTE_SHIFT 8
#define TWO_BYTE_SHIFT 16
#define THREE_BYTE_SHIFT 24
#define FOUR_BYTE_SHIFT 32
#define FIVE_BYTE_SHIFT 40
#define SIX_BYTE_SHIFT 48
#define SEVEN_BYTE_SHIFT 56
#define NODE_ID_DEBUG 2

#define SHIFT_BYTE_0(num) ((num) << ZERO_BYTE_SHIFT)
#define SHIFT_BYTE_1(num) ((num) << ONE_BYTE_SHIFT)
#define SHIFT_BYTE_2(num) ((num) << TWO_BYTE_SHIFT)
#define SHIFT_BYTE_3(num) ((num) << THREE_BYTE_SHIFT)
#define SHIFT_BYTE_4(num) ((num) << FOUR_BYTE_SHIFT)
#define SHIFT_BYTE_5(num) ((num) << FIVE_BYTE_SHIFT)
#define SHIFT_BYTE_6(num) ((num) << SIX_BYTE_SHIFT)
#define SHIFT_BYTE_7(num) ((num) << SEVEN_BYTE_SHIFT)

#define GET_BYTE_0(num) (((num) >> ZERO_BYTE_SHIFT) & 0xff)
#define GET_BYTE_1(num) (((num) >> ONE_BYTE_SHIFT) & 0xff)
#define GET_BYTE_2(num) (((num) >> TWO_BYTE_SHIFT) & 0xff)
#define GET_BYTE_3(num) (((num) >> THREE_BYTE_SHIFT) & 0xff)
#define GET_BYTE_4(num) (((num) >> FOUR_BYTE_SHIFT) & 0xff)
#define GET_BYTE_5(num) (((num) >> FIVE_BYTE_SHIFT) & 0xff)
#define GET_BYTE_6(num) (((num) >> SIX_BYTE_SHIFT) & 0xff)
#define GET_BYTE_7(num) (((num) >> SEVEN_BYTE_SHIFT) & 0xff)
#define ONE_BYTE_MASK 0xff

#define UFSHPB_READ_BUFFER 0xF9

#define SECTOR 512
#define INVALID_LBA 0xFEFEFEFE
#define TRAN_LEN 4096
#define UFS_BLOCK_SIZE 4096
#define HPB_OP_LUN 3
#define QUERY_FLAG_IDN_HPB_RESET 0x11
#define HPB_RESET_MAX_COUNT 1000

void ufshpb_reset(struct ufs_hba *hba);
void ufshpb_debug_init(struct ufs_hba *hba);
void ufshpb_init_handler(struct work_struct *work);

extern void scsi_init_command(struct scsi_device *dev, struct scsi_cmnd *cmd);
extern void ufshpb_check_rsp_upiu(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
extern void ufshpb_check_rsp_upiu_device_mode(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
extern int ufshpb_support(struct ufs_hba *hba);
extern int ufs_hpb_init_lru(struct ufs_hba *hba);
extern void ufshpb_prep_fn_lru(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
extern void ufshpb_prep_fn_condense(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
extern void ufshpb_prep_for_dev_mode(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
extern void init_radix_tree(void);

extern void parse_hpb_dts(struct ufs_hba *hba, struct device_node *np);
extern int feature_open(struct ufs_hba *hba);
extern int hpb_init(struct ufs_hba *hba);
extern int is_hpb_opened_card(struct ufs_hba *hba);
extern void suspend_wait_hpb(struct ufs_hba *hba);
#define UFS_FEATURES_SUPPORT 0x1F
#define UFS_DEVICE_DESCRIPTOR_HPB_IDN 0x00
#define UFS_DEVICE_DESCRIPTOR_HPB_SELECTOR 0x00

#define BUDO_BASE_OFFSET 0x1A
#define BUD_CONFIG_PLENGTH 0x1B

#define HPB_BUFFER_ID_INACTIVE 0x01
#define HPB_WRITE_BUFFER 0xFA
#define HPB_WRITE_BUFFER_CMD_LEN 0x0A
#define HPB_WRITE_BUFFER_RETRY 0x3
#define ONE_BLOCK_SECTORS 0x08
#define HPB_ONE_ENTRY_PPN_SIZE_JDEC 0x8
#define HPB_ONE_ENTRY_PPN_SIZE_1862 0x4

#define HPB_TURBO_TABLE_UNIT_DESC_IDN_1862 0xFD
#define HPB_TURBO_TABLE_ENBALE_OFFSET_1862 0x02
#define HPB_TURBO_TABLE_L2PSIZE_OFFSET_1862 0x03
#define HPB_VERSION 0x40
#define HPB_VERSION_1862 0xF4
#define HPB_SUPPORT_BIT 0x0080
#define HPB_SUPPORT_BIT_1862 0x0001
#define HPB_DEVICE_DESC_DEXTENDED_UFSFEATURE_SUPPORT 0x4F
#define HPB_DEVICE_DESC_VENDOR_FEATURE_SUPPORT_1862 0xF0
#define HPB_DEVICE_DESC_SELECTOR_SAMSUNG 0x01
#define HPB_DEVICE_DESC_SELECTOR_PROTOCOL 0x00
#define HPB_SUPPORT_DESC 0x1F
#define HPB_GEOMETRY_DESC_SELECTOR_SAMSUNG 0x01
#define HPB_GEOMETRY_DESC_SELECTOR_PROTOCOL 0x00
#define HPB_LU_NUMBER_OFFSET 0x49
#define HPB_REGION_SIZE_OFFSET 0x48
#define HPB_SUBREGION_SIZE_OFFSET 0x4A
#define HPB_MAX_ACTIVE_REGIONS 0x4B
#define HPB_MAX_UNIT_DESC_SIZE 0x2D
#define HPB_UNIT_DESC_SELECTOR_SAMSUNG 0x01
#define HPB_UNIT_DESC_SELECTOR_PROTOCOL 0x00
#define HPB_UNIT_DESC_LU_ENBALE 0x03
#define HPB_UNIT_DESC_LOGICAL_BLOCK_SIZE 0x0A
#define HPB_UNIT_DESC_LOGICAL_BLOCK_COUNT 0x0B
#define HPB_UNIT_DESC_LUMAX_ACTIVE_HPB_REGIONS 0x23
#define HPB_UNIT_DESC_PINNED_REGION_STARTIDX 0x25
#define HPB_UNIT_DESC_NUM_PINNED_REGIONS 0x27
#define HPB_UNIT 0x03
#define HPB_UNIT_DESCS_COUNT 0x08
#define HPB_MEMORY_SIZE (32 * 1024 * 1024)
#define HPB_SECTOR_SIZE 0x200
#define HPB_DESC_DEVICE_MAX_SIZE 0x59
#define HPB_DESC_DEVICE_MAX_SIZE_SAMSUNG 0x5F
#define HPB_DESC_DEVICE_MAX_SIZE_1862_SUPPORT 0xFE
#define HPB_VENDOR_UNIT_DESC_MAX_SIZE 0x10
#define HPB_UNIT_DESC_MAX_SIZE 0x2D
#define HPB_GEOMETRY_MAX_SIZE 0x57
#define HPB_GEOMETRY_MAX_SIZE_SAMSUNG 0x59

#define UFSHPB_PRINT_ALL_INFO 1
#define UFSHPB_TURN_OFF 2
#define UFSHPB_TURN_ON 3
#define UFSHPB_CONTROL_MODE_OFFSET 0x42
#define ONE_K 1024

#define HPB_REGION_NONE 0x0
#define HPB_REGION_UPDATE_REQ 0x01
#define HPB_REGION_RESET_REQ 0x02
#define HPB_REGION_OPREQ_OFFSET 24
#define HPB_REGION_VALID_OFFSET 26
#define HPB_REGION_INVALID_OFFSET 27
#define HPB_ACTIVE_REGION_ONE_OFFSET 28
#define HPB_ACTIVE_REGION_TWO_OFFSET 32
#define HPB_ONE_VALID 1
#define HPB_TWO_VALID 2
#define HPB_WRITE_BUFFER 0xFA
#define HPB_BUFFER_ID_INACTIVE 0x01
#define HPB_WRITE_BUFFER_CMD_LENGTH 0xA

#define HPB_DEVICE_DESC_UFS_VENDOR_FEATURE_SUPPORT_OFFSET 0xF0
#define HPB_DEVICE_DESC_UFS_TURBO_TABLE_VERSION_OFFSET 0xF4
#define HPB_DEVICE_DESC_HPB_LEN_OFFSET 0xFE

#define HPB_CONFIGURE_UNIT_DESC_LU_ENABLE 0x00
#define HPB_CONFIGURE_UNIT_DESC_LU_MAX_ACTIVE_HPB_REGIONS 0x10
#define HPB_CONFIGURE_UNIT_DESC_HPB_PINNED_REGION_STARTIDX 0x12
#define HPB_CONFIGURE_UNIT_DESC_HPB_PINNED_REGION_NUM 0x14
#define HPB_WRITE_CONFIG_DESC_IDN 0x01
#define HPB_WRITE_CONFIG_DESC_SELECTOR_SAMSUNG 0x01
#define HPB_WRITE_CONFIG_DESC_SELECTOR_PROTOCOL 0x01
#define HPB_WRITE_CONFIG_MAX_SIZE 0x200
#define HPB_LUN 0x03
#define HPB_FUNC_ENABLE 0x02

#define DEV_DATA_SEG_LEN 0x14
#define HPB_SENSE_DATA_LEN 0x12
#define HPB_DESC_TYPE 0x80
#define HPB_ADDITIONAL_LEN 0x10
#define MAX_ACTIVE_NUM 0x2
#define MAX_INACTIVE_NUM 0x2

#define MIN_SECTOR_SAMSUNG 8
#define MAX_SECTOR_SAMSUNG 64


enum write_buffer_cmd_compose {
	OPCODE = 0,
	BUFFER_ID,
	ID_HIGH,
	ID_LOW,
	RESERVD4,
	RESERVD5,
	RESERVD6,
	RESERVD7,
	RESERVD8,
	RESERVD9,
};

enum read_buffer_cmd_compose {
	RB_OPCODE = 0,
	RB_BUFFER_ID,
	RB_REGION_ID_HIGH,
	RB_REGION_ID_LOW,
	RB_SUBREGION_ID_HIGH,
	RB_SUBREGION_ID_LOW,
	RB_AL_LEN3,
	RB_AL_LEN2,
	RB_AL_LEN1,
	RB_CONTROL,
};

#define HPB_HISI_DEBUG_LRU
#define HPB_NEED_INIT 0x00
#define MAX_HPB_LUN_NUM 0x08
#define HPB_LUN_SELECT 0x03
#define HPB_SUBREGION_PPN_SIZE 0x8000
#define HPB_SUBREGION_PHYSICAL_SIZE 0x1000000
#define HPB_ALLOC_MEM_COUNT 0x08
#define HPB_MEMSIZE_PER_LOOP 0x400000
#define HPB_MEMSIZE_WHOLE 0x0800000
#define HPB_SUBREGION_NUM_PER_MEMSIZE 0x80
#define UP_LIMIT 0x800
#define RB_CMD_SIZE 36
#define RB_CDB_SIZE 10
#define RB_SCSI_TIMEOUT 1000

enum UFSHPB_STATE {
	UFSHPB_PRESENT = 1,
	UFSHPB_NOT_SUPPORTED = -1,
	UFSHPB_FAILED = -2,
	UFSHPB_NEED_INIT = 0,
	UFSHPB_RESET = -3,
};
enum ufshpb_node_status {
	UFSHPB_NODE_NEW = 0,
	UFSHPB_NEED_LOAD = 1,
	UFSHPB_NODE_AFTER = 3,
	UFSHPB_NODE_LRU = 4,
	UFSHPB_NODE_LATEST = 5

};

/* lru list struct */
enum subregion_node_status {
	NODE_EMPTY = 0,
	NODE_NEW,
	NODE_INACTIVATING,
	NODE_INACTIVAED,
	NODE_UPDATING,
};

struct subregion_node {
	u32 subregion_id;
	u32 subregion_id_old;
	u32 status;
	struct list_head active_list;
	struct list_head inactive_list;
	struct list_head lru_list;
	u64 *node_addr;
	spinlock_t lock;
	struct ufs_hba *hba;
};

struct ndlist_data {
	struct ufs_hba *hba;
	struct list_head active_list;
	struct list_head inactive_list;
	struct list_head lru_list;
	struct work_struct run_active_work;
	struct work_struct run_inactive_work;
	struct work_struct run_free_work;
	spinlock_t lock;
};

struct ufshpb_rsp_sense_data {
	__be16 sense_data_len;
	u8 desc_type;
	u8 additional_len;
	u8 hpb_opt;
	u8 lun;
	u8 act_cnt;
	u8 inact_cnt;
	__be16 act_region_0;
	__be16 act_subregion_0;
	__be16 act_region_1;
	__be16 act_subregion_1;
	__be16 inact_region_0;
	__be16 inact_region_1;
};
#define RSP_REVSIZE 3

struct ufshpb_rsp_upiu {
	__be32 residual_transfer_count;
	__be32 tt_dirty_cnt;
	__be32 reserved[RSP_REVSIZE];
	__be16 sense_data_len;
	u8 sense_data[RESPONSE_UPIU_SENSE_DATA_LENGTH];
};

struct ufshpb_node {
	enum ufshpb_node_status status;
	struct list_head list;
	unsigned int region_id;
	unsigned int node_id;
	unsigned int one_node_region_count;
	u8 *pool;
	unsigned int hit_miss;
	unsigned int hit_ppn;
	unsigned int hit_before;
	int loading;
	int load_complete;
	int load_cnt;
	int mark_dirty;
	u32 dirty_count;
	u32 write_dirty_count;
	struct page **pool_page;
	int ppn_loading;
	int node_info_ppn_loading;
	struct ufs_hba *hba;
	struct ufs_hpb_info *ufs_hpb;
	unsigned long long hit_total;
	ktime_t last_hit;
};

struct ufshpb_debug {
	struct device_attribute ufshpb_attr;
};

struct ufshpb_lu_control {
	spinlock_t hpb_control_lock;
	struct ufs_hba *hba;
	int lun;
	struct list_head node_list;
	struct work_struct hpb_sync_all_worker;
	struct delayed_work ufshpb_init_work;
	u64 hit_total;
	ktime_t last_hit;
	ktime_t hpb_last_load;
	int occupy_cnt;
	ktime_t occupy_last;
	int hit_wait;
	int lu_enable;
	int lu_control_init_status;
	struct subregion_node
		*subregion_array[HPB_ALLOC_MEM_COUNT * HPB_MEMSIZE_PER_LOOP /
				 HPB_SUBREGION_PPN_SIZE];

	struct ndlist_data *ndlist_data;
};

struct ufs_hpb_info {
	u32 ufs_hpb_support;
	struct ufs_hba *hba;
	int ufshpb_state;
	int ppn_size;
	int one_region_physical_size;
	int one_subregion_physical_size;
	int one_region_ppn_size;
	int one_region_ppn_number;
	u32 one_subregion_ppn_size;
	int one_subregion_ppn_number;
	int one_node_ppn_size;
	int one_node_ppn_number;
	int one_region_subregion_number;
	unsigned int one_node_region_count;
	int subregion_num_4k;
	int region_num_4k;
	u8 factor_region_256k;
	int region_entry_number;
	u8 hpb_lu_number;
	u32 hpb_logical_block_size;
	u8 hpb_logical_block_count;
	u8 bud0_base_offset;
	u8 bud_config_plength;
	int config_desc_length;
	u16 hpb_version;
	u8 hpb_control_mode;
	u32 hpb_extended_ufsfeature_support;
	u16 hpb_device_max_active_regions;
	u8 hpb_lu_enable[MAX_HPB_LUN_NUM];
	u16 hpb_lu_max_active_regions[MAX_HPB_LUN_NUM];
	u16 hpb_lu_pinned_region_startidx[MAX_HPB_LUN_NUM];
	u16 hpb_lu_pinned_region_num[MAX_HPB_LUN_NUM];

	/* used for 1862 */
	u8 turbo_table_en;
	u32 l2p_size;

	int ufshpb_debug_flag;
	struct ufshpb_debug ufshpb_debug_state;

	struct delayed_work ufshpb_init_work;

	struct ufshpb_lu_control *ufshpb_lu_control[MAX_HPB_LUN_NUM];
	int (*ufs_hpb_init_lru)(struct ufs_hba *hba);
};

#endif
