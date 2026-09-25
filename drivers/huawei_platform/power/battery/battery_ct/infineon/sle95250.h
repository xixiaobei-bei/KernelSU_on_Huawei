/*
 * sle95250.h
 *
 * sle95250 driver head file for battery checker
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

#ifndef _SLE95250_H_
#define _SLE95250_H_

#include <linux/types.h>

#include "../onewire/onewire_common.h"
#include "../batt_aut_checker.h"

/* sysfs debug */
#define CYC_MAX                        0xffff
#define ACT_MAX_LEN                    64
#define TEST_ACT_GAP                   1
#define ORG_TYPE                       0

#define LOW4BITS_MASK                  0x0f
#define HIGH4BITS_MASK                 0xf0
#define ODD_MASK                       0x0001
#define DECIMAL_BASE                   10

#define BIT_P_BYT                      8

#define ASCII_0                        0x30

#define OPTIGA_SINGLE_CHIP_ADDR        0x0000

/* CRC */
#define CRC16_INIT_VAL                 0xffff

/* retry */
#define OPTIGA_REG_UIDR_RTY            5  /* uid read retry */
#define OPTIGA_REG_CYCLK_RTY           3  /* read/set cycle lock status */
#define OPTIGA_NVM_RTY                 3  /* nvm read/write retry */
#define OPTIGA_CYC_INIT_RTY            3  /* cycle val init retry */
#define OPTIGA_CYC_READ_RETRY          3  /* cycle val read retry */
#define SLE95250_ACT_RTY               5  /* activation signature read retry */
#define SLE95250_PGLK_RTY              3  /* set/read page lock retry */
#define SLE95250_SN_RTY                5  /* sn read retry */

/*length */
#define SLE95250_CELVED_IND            1
#define SLE95250_PKVED_IND             0
#define SLE95250_SN_BIN_LEN            12
#define SLE95250_SN_ASC_LEN            16
#define SLE95250_ACT_LEN               60
#define SLE95250_ACT_CRC_BYT0          2
#define SLE95250_ACT_CRC_LEN           2
#define SLE95250_TE_PBK_LEN            48
#define SLE95250_UID_LEN               12
#define SLE95250_ODC_LEN               48
#define SLE95250_BATTTYP_LEN           2

#define SLE95250_DEFAULT_TAU           7  /* default: 7us */
#define SLE95250_CORE_NUM              8
#define SLE95250_WAIT_CPU_SWITCH_DONE  10 /* 10 ms */

/* SN */
#define SLE95250_SN_BYT_OFFSET         4
#define SLE95250_SN_PG_OFFSET          4

enum sle95250_sysfs_type {
	SLE95250_SYSFS_BEGIN = 0,
	/* read only */
	SLE95250_SYSFS_ECCE,
	SLE95250_SYSFS_IC_TYPE,
	SLE95250_SYSFS_UID,
	SLE95250_SYSFS_BATT_TYPE,
	SLE95250_SYSFS_BATT_SN,
	SLE95250_SYSFS_PGLK,
	/* read and write */
	SLE95250_SYSFS_ACT_SIG,
	SLE95250_SYSFS_BATT_CYC,
	SLE95250_SYSFS_SPAR_CK,
	SLE95250_SYSFS_CYCLK,
	SLE95250_SYSFS_END,
};

struct sle95250_memory {
	uint8_t uid[SLE95250_UID_LEN];
	bool uid_ready;
	uint8_t batt_type[SLE95250_BATTTYP_LEN];
	bool batttp_ready;
	uint8_t sn[SLE95250_SN_ASC_LEN];
	bool sn_ready;
	uint8_t res_ct[SLE95250_ODC_LEN];
	bool res_ct_ready;
	uint8_t act_sign[SLE95250_ACT_LEN];
	bool act_sig_ready;
	enum batt_source source;
	bool ecce_pass;
	enum batt_ic_type ic_type;
	enum batt_cert_state cet_rdy;
};

struct sle95250_dev {
	struct device *dev;
	struct ow_gpio_des pl061_gpio;
	struct sle95250_memory mem;
	bool sysfs_mode;
	uint8_t tau;
	uint16_t product_id;
	uint16_t cyc_full;
	uint32_t cyc_num;
};

#endif /* _SLE95250_H_ */
