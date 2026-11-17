/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: Netfilter common head file.
 * Author: chenzhongxian@huawei.com
 * Create: 2016-05-28
 */

#ifndef _NF_HW_COMMON
#define _NF_HW_COMMON
#define MAX_HASH 256

/* max URL length is 8000 from rfc7230 */
#define MAX_URL_LENGTH 8000
#define DOUBLE_TIME 2
#define HEX_LEN 16
#define HEX_VALUE_OFFSET 10
#define ULR_LEN_OFFSET_2 2
#define ULR_LEN_OFFSET_32 32
#define ULR_LEN_OFFSET_64 64
#define US_MS 1000
#define POS_INVALID_VALUE 160
#define HTTP_HEAD_LEN 10
#define HASH_INITVAL 1005
#define DECIMAL_BASE 10

const char *strfind(const char *s1, const char *s2);
const char *strfindpos(const char *s1, const char *s2, int len);
const char *strfinds2(const char *s1, const char *s2, int s2l);
const char *substring(const char *src, const char *f, const char *s, int *l);
void byte_to_hex(const char *pin, int ilen, char *pout, int olen);
void hex_to_byte(const char *pin, int ilen, char *pout, int olen);
u64 get_cur_time(void);
void right_trim(char *p);
const char *left_trim(const char *p);
char *get_url_form_data(const char *data, int datalen);
char *get_url_path(const char *data, int datalen);
unsigned int get_hash_id(int uid);

#endif /* _NF_HW_COMMON */
