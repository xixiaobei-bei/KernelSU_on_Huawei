/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: implement hisee casd certification function
 * Create: 2018-05-05
 */
#ifndef HISEE_CASDCERT_H
#define HISEE_CASDCERT_H
#include <linux/types.h>

#define HEX_STR_LEN         2  /* One BYTE expressed in Two Char */
#define HISI_DEC_BASE       10 /* Decimal Base value */
#define HALF_BYTE_LEN       4  /* Bit Len of Half Byte */
#define CASD_U16_MAX_VALUE  0xFFFF

#define HISI_CASD_FILE_SIZE     (1024 * 16) /* 16K */
#define HISI_CASD_RECV_SIZE     ((HISI_CASD_FILE_SIZE) * 2)
#define HISI_256HASH_BYTE_LEN   32
#define HISI_256HASH_STR_LEN    ((HISI_256HASH_BYTE_LEN) * 2)

#define HISI_VERIFY_CASD_TOTAL_PACK  1

/*
 * @brief  used to store info when receive CASD data from AT
 */
struct hisee_casd_at_data {
	void *buffer;           /* location to store casd data */
	unsigned int size;      /* size of data received */
	unsigned int curr_pack; /* current pack number, used for at_result */
	unsigned int total_packs;    /* total packs number of casd data */
	unsigned int received_packs; /* number of packs that has received */
};

int hisi_receive_casd_func(const void *buf, int para);
int hisi_verify_casd_func(const void *buf, int para);
void set_support_casd(bool flag);
bool get_support_casd(void);
bool get_bypass_casd(void);

#endif
