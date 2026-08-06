/*
 * a1007.h
 *
 * a1007 driver head file for battery checker
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

#ifndef _A1007_H_
#define _A1007_H_

#include <linux/types.h>
#include <linux/of.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <huawei_platform/power/batt_info_pub.h>
#include <huawei_platform/power/power_mesg_srv.h>
#include <linux/wakelock.h>
#include <linux/mutex.h>

#include "lib/a1007_crypto.h"
#include "lib/present80.h"
#include "../onewire/onewire_common.h"
#include "../batt_aut_checker.h"

#define A1007_SN_BLOCK_IDX      2
#define A1007_SN_LOCK_MASK      0x80
#define A1007_ORG_NAME_LEN      10
#define A1007_COM_NAME_LEN      12
#define A1007_PUBKEY_OFFSET     22
#define A1007_SIGNATURE_OFFSET  64
#define A1007_DEVICE_ID_ADDR    0xF8
#define A1007_DATA_HEAD_SIZE    2
#define A1007_MAC_LEN_SIZE      8
#define A1007_DEV_ID_BUFF_SIZE  3
#define A1007_LOCK_STATUS_SIZE  1
#define A1007_VIRTUAL_PAGE      7
#define A1007_CERTZ_SIZE        128
#define A1007_USER_MEM_SIZE     32
#define A1007_SIGN_SIZE         64
#define A1007_BLOCK_SIZE        128
#define A1007_BIG_BUFF_SIZE     192
#define A1007_MID_BUFF_SIZE     64
#define A1007_SML_BUFF_SIZE     32
/* 8-bit slave address of A1007 */
#define A1007_SLAVE_ADDR        0xA0
/* Certification SLOT */
#define USER_CERT_SLOT          0
/* Battery type SLOT */
#define A1007_CERT_COMP_TYPE    0x04
/* UID MEM address */
#define USER_CERT_UID_ADDR      0x0300
#define NXP_CERT_UID_ADDR       0x0310
#define USER_CERT_ADDR          0x0100
#define NXP_CERT_ADDR           0x0200
#define NXP_SN_ADDR             0x0C00
/* Size of SHA224 hash (in byte) */
#define SHA224_HASH_SIZE        28
/*  Size of SHA224 block (in byte) */
#define SHA224_BLOCK_SIZE       64

#define A1007_CMD_SIZE          2
#define A1007_EXE_ECDH          0x0800
#define A1007_READ_ECDH         0x0801
#define A1007_READ_ECDH_LEN     25
#define A1007_EXE_MAC           0x0802
#define A1007_READ_MAC          0x0803
#define A1007_READ_MAC_LEN      19
#define A1007_EEUNLOCK          0x0804
#define A1007_EE_WLOCK          0x080C
#define A1007_EE_WLOCK_LEN      1
#define A1007_GET_LOCK_STATUS   0x080D
#define A1007_GET_LSTATUS_LEN   4
#define A1007_GET_CRC           0x080F
#define A1007_GET_CRC_LEN       3
#define A1007_POWER_DOWN        0x0901
#define A1007_PD_PARA_LEN       1
#define A1007_OWI_WAKE          0x02
#define A1007_CMD_FIRST_BYTE(x) ((x) >> 8)

#define A1007_ADDR_WR(addr)     ((addr) & 0xFE)
#define A1007_ADDR_RD(addr)     ((addr) | 0x01)

#define A1007_MEM_TO_DATA(ptr)  ((ptr) + 1)
#define A1007_MEM_VALID(ptr)    (*(ptr))
#define A1007_DEVID_U16(ptr)    ((*((ptr) + 2) << 8) + *((ptr) + 3))

#define A1007_FAIL_CRC          256
#define A1007_INVALID_CMD       2
#define A1007_FAIL              (-1)
#define A1007_SUCCESS           0

struct a1007_mem {
	uint8_t lock_status[A1007_LOCK_STATUS_SIZE + 1];
	uint8_t btype[BATTERY_TYPE_BUFF_SIZE + 1];
	uint8_t certz[A1007_CERTZ_SIZE + 1];
	uint8_t sn[A1007_USER_MEM_SIZE + 1];
	uint8_t dev_id[A1007_DEV_ID_BUFF_SIZE + 1];
	uint8_t uid[A1007_UID_SIZE + 1];
	uint8_t mac_context[PRESENT80_CACHE_LEN + 1];
};

struct a1007_cmd_wait {
	uint32_t ecdsa_ms;
	uint32_t ecdh_ms;
	uint32_t eeprom_write_ms;
	uint32_t eeunlock_ms;
	uint32_t power_down_us;
};

struct a1007_des {
	uint8_t power_down;
	uint8_t addr;
	uint8_t sn_len;
	uint8_t rand[SHA224_HASH_SIZE];
	phandle phy_ctrl;
	const char *name;
	struct ow_gpio_des *gpio;
	struct a1007_mac_src mac_src;
	struct a1007_cmd_wait wait;
	struct batt_ct_ops_list reg_node;
	struct ow_phy_ops_v2 ops;
	struct ow_treq_v2 trq;
	struct a1007_mem mem;
	struct wake_lock wlock;
	struct mutex ops_mutex;
#ifdef ONEWIRE_STABILITY_DEBUG
	uint32_t err_cnt;
	uint16_t mem_addr;
	uint8_t buf_len;
	uint8_t mem_buff[A1007_BLOCK_SIZE];
#endif				/* ONEWIRE_STABILITY_DEBUG */
};

#endif /* _A1007_H_ */
