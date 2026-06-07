/*
 * maxim_onewire.h
 *
 * maxim onewire head file
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

#ifndef _MAXIM_ONEWIRE_H_
#define _MAXIM_ONEWIRE_H_

#include "../onewire/onewire_common.h"

/* onewire common interface */
struct maxim_time_req {
	struct ow_treq ow_trq;
	unsigned int program_ms;
	unsigned int secret_program_ms;
	unsigned int compute_mac_ms;
};

#define MAXIM_PAGE_NUM                              4
#define MAXIM_SEGMENT_NUM                           8

struct maxim_ow_mem {
	unsigned char validity_rom_id;
	unsigned char validity_personality;
	unsigned char validity_sram;
	unsigned char validity_status;
	unsigned char validity_page[MAXIM_PAGE_NUM];
	unsigned char validity_segment[MAXIM_PAGE_NUM][MAXIM_SEGMENT_NUM];
	unsigned char validity_mac;
	unsigned char *block_status;
	unsigned char *rom_id;
	unsigned char *personality;
	unsigned char *sram;
	unsigned char *user_memory;
	unsigned char *mac;
	unsigned int ic_type;
	unsigned int rom_id_len;
	unsigned int block_number;
	unsigned int block_size;
	unsigned int personality_length;
	unsigned int sram_length;
	unsigned int page_number;
	unsigned int page_size;
	unsigned int mac_length;
	unsigned int sn_length_bits;
	unsigned int sn_page;
	unsigned int sn_offset_bits;
	struct mutex lock;
};

struct maxim_ow_ic_des;

struct maxim_mem_ops {
	int (*get_rom_id)(struct maxim_ow_ic_des *des);
	int (*get_personality)(struct maxim_ow_ic_des *des);
	int (*get_user_memory)(struct maxim_ow_ic_des *des, unsigned char page,
			       unsigned char seg_start, unsigned char seg_end);
	int (*set_user_memory)(struct maxim_ow_ic_des *des, unsigned char *data,
			       unsigned char page, unsigned char seg);
	int (*get_sram)(struct maxim_ow_ic_des *des);
	int (*set_sram)(struct maxim_ow_ic_des *des, unsigned char *newdata);
	int (*get_mac)(struct maxim_ow_ic_des *des, unsigned char anonymous,
		       unsigned char page);
	int (*get_status)(struct maxim_ow_ic_des *des);
	int (*set_status)(struct maxim_ow_ic_des *des, unsigned char block,
			  unsigned char protect_mode);
	int (*valid_mem_ops)(struct maxim_ow_ic_des *des,
			     struct platform_device *pdev);
};

/* struct maxim_ow_ic_des--maxim onewire ic memory and commution operations */
struct maxim_ow_ic_des {
	struct maxim_time_req trq;
	struct maxim_ow_mem memory;
	struct maxim_mem_ops mem_ops;
	struct ow_phy_ops phy_ops;
};

#define DOUBLE(x)                            ((x) << 1)

/* Maxim ROM */
#define MAXIM_ROM_SIZE                       8
#define MAXIM_SEGMENT_SIZE                   4
#define MAXIM_SECRET_SIZE                    32
#define MAXIM_MAC256_SIZE                    32
#define MAXIM_PAGE_SIZE                      32

/* Slave presence signal is low */
#define NO_SLAVE_RESPONSE(x)                 ((x) != 0)

/* MAXIM 1-wire memory and SHA function command */
#define WRITE_MEMORY                         0x55
#define READ_MEMORY                          0xF0
#define WRITE_BLOCK_PROTECTION               0xC3
#define READ_STAUS                           0xAA
#define READ_WRITE_SCRATCHPAD                0x0F
#define LOAD_AND_LOCK_SECRET                 0x33
#define COMPUTE_AND_LOCK_SECRET              0x3C
#define COMPUTE_AND_READ_PAGE_MAC            0xA5
#define AUTHENTICATED_WRITE_MEMORY           0x5A
#define AUT_WRT_BLOCK_PROT                   0xCC

/* MAXIM 1-wire rom function command */
#define READ_ROM                             0x33
#define MATCH_ROM                            0x55
#define SEARCH_ROM                           0xF0
#define SKIP_ROM                             0xCC
#define RESUME_COMMAND                       0xA5

/* Command Parameters */
#define SEGMENT_OFFSET                       0x05
#define MAXIM_PAGE_MASK                      0x01
#define SEGMENT_MASK                         0x07
#define ANONYMOUS_MODE                       0xE0
#define READ_MODE                            0x0F
#define WRITE_MODE                           0x00
#define BLOCK_STATUS_CONFIG                  0x00
#define PERSONALITY_CONFIG                   0xE0
#define ONEWIRE_RELEASE                      0xAA
#define BLOCK_MASK                           0x03
#define PROTECTION_MASK                      0xF0
#define MAXIM_PAGE0                          0
#define MAXIM_PAGE1                          1
#define MAXIM_BLOCK0                         0
#define MAXIM_BLOCK1                         1
#define MAXIM_BLOCK2                         2
#define MAXIM_BLOCK3                         3
#define MAXIM_SEGMENT0                       0
#define MAXIM_SEGMENT1                       1
#define MAXIM_SEGMENT2                       2
#define MAXIM_SEGMENT3                       3
#define MAXIM_SEGMENT4                       4
#define MAXIM_SEGMENT5                       5
#define MAXIM_SEGMENT6                       6
#define MAXIM_SEGMENT7                       7
#define MAXIM_MAN_ID_SIZE                    2
#define MAXIM_MAN_ID_OFFSET                  2
#define MAXIM_SEGMENTS2BYTES(x)              ((x) << 2)

/* Command success response */
#define MAXIM_ONEWIRE_COMMAND_SUCCESS        0xAA

#define BATTERY_CELL_FACTORY                 4
#define BATTERY_PACK_FACTORY                 5

/* 1-wire function operation return signals */
#define MAXIM_ONEWIRE_SUCCESS         ONEWIRE_SUCCESS
#define MAXIM_ERROR_BIT               0x80
#define MAXIM_ONEWIRE_DTS_ERROR       (MAXIM_ERROR_BIT | ONEWIRE_DTS_FAIL)
#define MAXIM_ONEWIRE_COM_ERROR       (MAXIM_ERROR_BIT | ONEWIRE_COM_FAIL)
#define MAXIN_ONEWIRE_CRC8_ERROR      (MAXIM_ERROR_BIT | ONEWIRE_CRC8_FAIL)
#define MAXIN_ONEWIRE_CRC16_ERROR     (MAXIM_ERROR_BIT | ONEWIRE_CRC16_FAIL)
#define MAXIM_ONEWIRE_NO_SLAVE        (MAXIM_ERROR_BIT | ONEWIRE_NO_SLAVE)
#define MAXIM_ONEWIRE_ILLEGAL_PAR     (MAXIM_ERROR_BIT | ONEWIRE_ILLEGAL_PARAM)

/* SHA ERROR */
#define SHA_SUCCESS                     0x00
#define SHA_PARA_ERR                    0x01
#define SHA_MALLOC_ERR                  0x02
#define SHA_LENGTH_ERR                  0x03

/* MAC computation mode */
#define MAXIM_SEGMENT_4BYTES            4
#define MAXIM_64BYTES                   64
#define MAXIM_128BYTES                  128

/* CRC result */
#define MAXIM_CRC16_RESULT              0xB001
#define MAXIM_CRC8_RESULT               0

/*
 * For SHA-1, SHA-224  and SHA-256, each message block has 512 bits, which are
 * represented as a sequence of sixteen 32-bit words.
 */
#define SHA256_BLOCKSIZE_512BITS         512
#define SHA256_WORDSIZE_32BITS           32
#define SHA256_WORDSIZE_4BYTES           4

/* MAC input data structure */
#define AUTH_MAC_ROM_ID_OFFSET           96
#define AUTH_MAC_PAGE_OFFSET             0
#define AUTH_MAC_SRAM_OFFSET             32
#define AUTH_MAC_KEY_OFFSET              64
#define AUTH_MAC_PAGE_NUM_OFFSET         106
#define AUTH_MAC_MAN_ID_OFFSET           104
#define MAX_MAC_SOURCE_SIZE              128

int maxim_onewire_register(struct maxim_ow_ic_des *des,
			   struct platform_device *pdev);

#endif /* _MAXIM_ONEWIRE_H_ */
