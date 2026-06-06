#ifndef __NPU_FIRMWARE_H
#define __NPU_FIRMWARE_H

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <linux/pm.h>

#define DEVDRV_TS_BINARY_PATH      "/vendor/firmware/tsch_fw.bin"
#define DEVDRV_AICPU_BINARY_PATH  "/vendor/firmware/aicpu_fw.bin"

#define TS_FW_REMAP_SIZE (512*1024)
#define TS_FW_MAX_SIZE (1024*1024)

#define DEVDRV_TS_BIN_CHEKC_LEN        32
#define DEVDRV_TS_BIN_MAX_SEGMENT_NUM  16

#define BUFF_LEN (512)

enum devdrv_cpu_type {
	DEVDRV_FW_TYPE_AICPU,
	DEVDRV_FW_TYPE_TS,
	DEVDRV_FW_TYPE_MAX,
};

struct devdrv_check_sum {
	u8 check[DEVDRV_TS_BIN_CHEKC_LEN];
};

struct devdrv_ts_bin_segment {
	u32 offset;
	u32 len;
	struct devdrv_check_sum segment_check;
};

struct devdrv_ts_bin_info {
	u32 ts_check_file;
	u32 fw_data_len;
	struct devdrv_check_sum fw_data_check;
	u32 segment_num;
	struct devdrv_ts_bin_segment segment[DEVDRV_TS_BIN_MAX_SEGMENT_NUM];
};

int devdrv_load_cpu_fw(void);
u64 devdrv_get_firmware_phy_addr(int type);

#endif
