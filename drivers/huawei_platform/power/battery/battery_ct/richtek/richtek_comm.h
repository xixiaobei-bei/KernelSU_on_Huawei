/*
 * richtek_comm.h
 *
 * Richtek IC communication protocol.
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef _RICHTEK_COMM_H_
#define _RICHTEK_COMM_H_

#include "../onewire/onewire_common.h"

#define RT_READ_STATUS 0x0055
#define RT_READ_CMD_CRC 0x0051
#define RT_AUTH_WO_ROM_ID 0x8150
#define RT_AUTH_WI_ROM_ID 0x8151
#define RT_GEN_WORK_SECRET 0x8105
#define RT_WRITE_CHALLENGE 0x8115
#define RT_READ_MAC 0x0125
#define RT_READ_ROM_ID 0x0135

#define RT_LOCK_MFA_PAGE1 0x8245
#define RT_LOCK_MFA_PAGE2 0x8255

#define RT_UNSEAL_SYS 0x8410
#define RT_SEAL_NVM 0x8480

#define RT_WRITE_MFA_PAGE1 0x8800
#define RT_WRITE_MFA_PAGE2 0x8801
#define RT_WRITE_SYS_PAGE 0x8802
#define RT_WRITE_SYS_PWD 0x8804
#define RT_READ_MFA_PAGE1 0x800
#define RT_READ_MFA_PAGE2 0x801
#define RT_READ_SYS_PAGE 0x802
#define RT_READ_DEVICE_ID 0x0805

#define RT_INC_CYC_CNT 0x9055
#define RT_READ_CYC_CN 0x1055

#define RT_MFA_PAGE1_SIZE 18
#define RT_MFA_PAGE2_SIZE 4
#define RT_SYS_PAGE_SIZE 12
#define RT_MODE_PSW_SIZE 2
#define RT_CYC_CNT_SIZE 2

#define RT_MAX_DATA_LEN 64
#define RT_MAX_CMD_LEN 2
#define RT_MAX_BUF_LEN (RT_MAX_DATA_LEN + RT_MAX_CMD_LEN)
#define RT_BIT_PER_BYTE 8

#define RT_COM_RETRY_NONE_MASK 0x1

enum rt_mfa_page_no {
	RT_MFA_PAGE1 = 0,
	RT_MFA_PAGE2,
	RT_MFA_MAX_PAGE_NO,
};

enum rt_lock_mode {
	RT_LOCK_NONE = 0,
	RT_SYS_LOCK,
};

enum rt_status_bit {
	RT_SYS_MODE = 0,
	RT_IC_BUSY = 2,
	RT_SWI_CRC_ERR = 6,
	RT_SWI_TIME_OUT = 7,
	RT_WK_SECRET_RDY = 8,
	RT_MAC_RDY = 9,
	RT_CHALLENGE_RDY = 10,
	RT_SECRET_RDY = 11,
	RT_SECRET_LCK = 12,
	RT_MFA_PAGE1_LCK = 13,
	RT_MFA_PAGE2_LCK = 14,
};

enum rt_com_err_no {
	RT_COM_OK = 0,
	RT_COM_CRC_ERR = -1,
	RT_COM_TIME_OUT = -2,
	RT_COM_PARA_ERR = -3,
	RT_COM_PHY_ERR = -4,
	RT_COM_MEM_ERR = -5,
	RT_COM_UNKNOWN_ERR = -6,
};

struct rt_cmd {
	unsigned int no;
	unsigned int len;
	unsigned int retry;
	unsigned int delay;
	unsigned int lock;
	const char *str;
};

struct rt_com_stat {
	unsigned int *totals;
	unsigned int *errs;
	unsigned int max_cerrs;
	unsigned int cerrs_cnt;
};

struct rt_com_obj {
	const struct rt_cmd *cmd_info;
	struct rt_com_stat com_stat;
	struct ow_phy_ops phy_ops;
	struct ow_treq ow_trq;
	unsigned int cmd_nums;
	unsigned int sign;
	struct mutex com_lock;
};

struct rt_comm_ops {
	int (*read)(struct rt_com_obj *, unsigned int,
		unsigned char *, unsigned int);
	int (*write)(struct rt_com_obj *, unsigned int,
		unsigned char *, unsigned int);
	int (*read_one_shot)(struct rt_com_obj *, unsigned int,
		unsigned char *, unsigned int);
	int (*write_one_shot)(struct rt_com_obj *, unsigned int,
		unsigned char *, unsigned int);
	int (*get_stats)(struct rt_com_obj *, char *, unsigned int);
	int (*register_com_phy)(struct rt_com_obj *, unsigned int);
	void (*reset_stats)(struct rt_com_obj *);
};

struct rt_ic_des {
	struct rt_com_obj obj;
	struct rt_comm_ops com_ops;
};

int rt_comm_register(struct rt_ic_des *des, struct platform_device *pdev,
	unsigned int phy_id);
void rt_destroy_com_obj(struct rt_com_obj *obj, struct platform_device *pdev);

#endif /* _RICHTEK_COM_H_ */
