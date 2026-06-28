#ifndef __ILITEK_TEST_H__
#define __ILITEK_TEST_H__

#include "ilitek_mp_test.h"

#define ILITEK_TEST_DATA_LEN                   1280
#define ILITEK_TEST_RESULT_LEN                 256
#define ILITEK_TEST_ITEMS                      7
#define ILITEK_TEST_ITEM_RES_LEN               3

struct ilitek_test {
    struct mp_test_items *items[ILITEK_TEST_ITEMS];
    int orignal_data[ILITEK_TEST_ITEMS][ILITEK_TEST_DATA_LEN];
    char result[ILITEK_TEST_RESULT_LEN];
};

int ilitek_test_hash(mp_tests key);
int ilitek_get_raw_data(struct ts_rawdata_info *info,
    struct ts_cmd_node *out_cmd);
int ilitek_rawdata_print(struct seq_file *m, struct ts_rawdata_info *info,
    int range_size, int row_size);
int ilitek_test_init (void);
void ilitek_test_exit(void);
#endif
