/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2015-2019. All rights reserved.
 *  Description: eFuse driver
 *  Author : security-ap
 *  Create : 2015/12/9
 */

#ifndef __HISI_EFUSE_H__
#define __HISI_EFUSE_H__

#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/string.h>

enum tag_efuse_log_level {
	LOG_LEVEL_DISABLE = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_TOTAL = LOG_LEVEL_INFO
};

enum tag_efuse_mem_attr {
	EFUSE_MEM_ATTR_NONE = -1,
	EFUSE_MEM_ATTR_HUK = 0,
	EFUSE_MEM_ATTR_SCP,
	EFUSE_MEM_ATTR_AUTHKEY,
	EFUSE_MEM_ATTR_CHIPID,
	EFUSE_MEM_ATTR_TSENSOR_CALIBRATION,
	EFUSE_MEM_ATTR_HUK_RD_DISABLE,
	EFUSE_MEM_ATTR_AUTHKEY_RD_DISABLE,
	EFUSE_MEM_ATTR_DBG_CLASS_CTRL,
	EFUSE_MEM_ATTR_DIEID,
#ifdef CONFIG_HISI_DEBUG_FS
	EFUSE_MEM_ATTR_SLTFINISHFLAG,
#endif
	EFUSE_MEM_ATTR_MAX
};

struct tag_efuse_attr_info {
	u32 bits_width;
};

struct tag_efusec_data {
	u32 efuse_group_max;
	phys_addr_t  paddr;
	u8 *vaddr;
	s32 (*atf_fn)(u64, u64, u64, u64);
	struct tag_efuse_attr_info efuse_attrs_from_dts[EFUSE_MEM_ATTR_MAX];
	struct mutex efuse_mutex; /* mutex to limit one caller in a time */
	u32 is_init_success;
};

#define OK                                 0
#define ERROR                              (-1)
#define ERROR_EXIT_PD                      (-2)
#define ERROR_ENTER_PD                     (-3)
#define ERROR_APB_PGM_DIS                  (-4)
#define ERROR_EFUSEC_READ                  (-5)
#define ERROR_PRE_WRITE                    (-6)
#define ERROR_PG_OPERATION                 (-7)

#define HISI_EFUSE_READ_CHIPID             0x1000
#define HISI_EFUSE_READ_DIEID              0x2000
#define HISI_EFUSE_WRITE_CHIPID            0x3000
#define HISI_EFUSE_READ_AUTHKEY            0x4000
#define HISI_EFUSE_WRITE_AUTHKEY           0x5000
#define HISI_EFUSE_READ_CHIPIDLEN          0x6000
#define HISI_EFUSE_WRITE_DEBUGMODE         0x7000
#define HISI_EFUSE_READ_DEBUGMODE          0x8000
#define HISI_EFUSE_READ_THERMAL            0x9000

#ifdef CONFIG_HISI_DEBUG_FS
#define HISI_EFUSE_TEST_WR                 0xa001
#define HISI_EFUSE_TEST_READ_CHIPID        0xa002
#define HISI_EFUSE_TEST_READ_DIEID         0xa003
#define HISI_EFUSE_TEST_READ_KCE           0xa004
#define HISI_EFUSE_TEST_WRITE_KCE          0xa005
#endif

#ifdef CONFIG_HISI_DEBUG_FS
#define HISI_EFUSE_WRITE_SLTFINISHFLAG     0xb000
#define HISI_EFUSE_READ_SLTFINISHFLAG      0xb001
#define EFUSE_SLTFINISHFLAG_LENGTH_BYTES   4
#endif

#define HISI_EFUSE_WRITE_DJTAGDEBUG        0xc000
#define HISI_EFUSE_READ_DJTAGDEBUG         0xd000
#define HISI_EFUSE_READ_DESKEW             0xe000
#define EFUSE_MODULE_INIT_SUCCESS          0x12345678

#define EFUSE_TIMEOUT_SECOND               1000
#define EFUSE_KCE_LENGTH_BYTES             16
#define EFUSE_HISEE_LENGTH_BYTES           8
#define EFUSE_DIEID_LENGTH_BYTES           20
#define EFUSE_CHIPID_LENGTH_BYTES          8
#define EFUSE_AUTHKEY_LENGTH_BYTES         8
#define EFUSE_SECDBG_LENGTH_BYTES          4
#define EFUSE_THERMAL_LENGTH_BYTES         8
#define EFUSE_FREQ_LENGTH_BYTES            4
#define EFUSE_DESKEW_LENGTH_BYTES          1
#define EFUSE_AVS_LENGTH_BYTES             1

#ifdef CONFIG_HI3XXX_EFUSE
s32 get_efuse_dieid_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_chipid_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_thermal_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_hisee_value(u8 *buf, u32 size, u32 timeout);
s32 set_efuse_hisee_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_freq_value(u8 *buf, u32 size);
s32 get_efuse_kce_value(u8 *buf, u32 size, u32 timeout);
s32 set_efuse_kce_value(u8 *buf, u32 size, u32 timeout);
s32 get_efuse_deskew_value(u8 *buf, u32 size, u32 timeout);
s32 hisi_efuse_read_value(u32 *buf, u32 buf_size, u32 func_id);
s32 hisi_efuse_write_value(u32 *buf, u32 buf_size, u32 func_id);
s32 get_efuse_avs_value(u8 *buf, u32 buf_size, u32 timeout);
#else
static inline s32 get_efuse_dieid_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_chipid_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_thermal_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_hisee_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 set_efuse_hisee_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_freq_value(u8 *buf, u32 size)
{
	return OK;
}

static inline s32 set_efuse_kce_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_kce_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 get_efuse_deskew_value(u8 *buf, u32 size, u32 timeout)
{
	return OK;
}

static inline s32 hisi_efuse_read_value(u32 *buf, u32 buf_size, u32 func_id)
{
	return OK;
}

static inline s32 hisi_efuse_write_value(u32 *buf, u32 buf_size, u32 func_id)
{
	return OK;
}

static inline s32 get_efuse_avs_value(u8 *buf, u32 buf_size, u32 timeout)
{
	return OK;
}
#endif

#ifdef CONFIG_HI3XXX_EFUSE
#define  EFUSE_DIEID_GROUP_START         32
#define  EFUSE_DIEID_GROUP_WIDTH         5
#define  EFUSE_CHIPID_GROUP_START        57
#define  EFUSE_CHIPID_GROUP_WIDTH        2
#define  EFUSE_KCE_GROUP_START           28
#define  EFUSE_KCE_GROUP_WIDTH           4
#endif

#endif /* __HISI_EFUSE_H__ */
