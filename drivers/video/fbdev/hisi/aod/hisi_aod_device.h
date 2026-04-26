/*
 *  linux/drivers/video/fbmem.c
 *
 *  Copyright (C) 1994 Martin Schaller
 *
 *	2001 - Documented with DocBook
 *	- Brad Douglas <brad@neruo.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#ifndef __HISI_AOD_DEVICE_H_
#define __HISI_AOD_DEVICE_H_

#include "protocol.h"
#include "contexthub_route.h"

#include <linux/module.h>
#include <linux/compat.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/vt.h>
#include <linux/init.h>
#include <linux/linux_logo.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/console.h>
#include <linux/kmod.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/efi.h>
#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/iommu.h>
#include <linux/of_reserved_mem.h>
#include <linux/delay.h>
#include "hisi_dss_ion.h"
#include <asm/fb.h>

#define ALIGN_UP(val, al)    (((val) + ((al)-1)) & ~((al)-1))

#define HISI_AOD_ERR(msg, ...)    \
	do { if (hisi_aod_msg_level > 0)  \
		printk(KERN_ERR "[hisi_aod]%s: "msg, __func__, ## __VA_ARGS__); } while (0)

#define HISI_AOD_INFO(msg, ...)    \
	do { if (hisi_aod_msg_level > 1)  \
		printk(KERN_INFO "[hisi_aod]%s: "msg, __func__, ## __VA_ARGS__); } while (0)

#define HISI_AOD_DEBUG(msg, ...)                                 \
	do {                                                     \
		if (hisi_aod_msg_level > 2)                      \
			printk(KERN_INFO "[hisi_aod]%s: "msg, __func__, ## __VA_ARGS__);  \
	} while (0)

#define AOD_IOCTL_AOD_START		_IOW(0xB2, 0x01, unsigned long)
#define AOD_IOCTL_AOD_STOP		_IO(0xB2, 0x02)
#define AOD_IOCTL_AOD_PAUSE 	_IOR(0xB2, 0x03, unsigned long)
#define AOD_IOCTL_AOD_RESUME 	_IOW (0xB2, 0x04, unsigned long)
#define AOD_IOCTL_AOD_START_UPDATING 	_IOR(0xB2, 0x05, unsigned long)
#define AOD_IOCTL_AOD_END_UPDATING	_IOW(0xB2, 0x06, unsigned long)
#define AOD_IOCTL_AOD_SET_TIME	_IOW(0xB2, 0x07, unsigned long)
#define AOD_IOCTL_AOD_SET_BITMAP_SIZE	_IOW(0xB2, 0x08, unsigned long)
#define AOD_IOCTL_AOD_SET_DISPLAY_SPACE		_IOW(0xB2, 0x09, unsigned long)
#define AOD_IOCTL_AOD_GET_PANEL_INFO		_IOW(0xB2, 0x0A, unsigned long)
#define AOD_IOCTL_AOD_FREE_BUF	_IOW(0xB2, 0x0B, unsigned long)
#define AOD_IOCTL_AOD_SET_FINGER_STATUS _IOW(0xB2, 0x0C, unsigned long)
#define AOD_IOCTL_AOD_GET_DYNAMIC_FB	_IOW(0xB2, 0x0D, unsigned long)
#define AOD_IOCTL_AOD_FREE_DYNAMIC_FB   _IOW(0xB2, 0x0E, unsigned long)
#define AOD_IOCTL_AOD_SET_MAX_AND_MIN_BACKLIGHT   _IOW(0xB2, 0x0F, unsigned long)
#define AOD_IOCTL_AOD_SET_COMMON_SENSORHUB _IOW(0xB2, 0x10, unsigned long)
#define AOD_IOCTL_AOD_SET_GMP _IOW(0xB2, 0x14, unsigned long)
#define AOD_IOCTL_AOD_SET_GENERAL_SENSORHUB _IOW(0xB2, 0x15, unsigned long)
#define AOD_IOCTL_AOD_GET_DYNAMIC_FB_NEW _IOW(0xB2, 0x16, unsigned long)
#define AOD_IOCTL_AOD_FREE_DYNAMIC_FB_NEW _IOW(0xB2, 0x17, unsigned long)
#define AOD_IOCTL_AOD_SET_FOLD_INFO _IOW(0xB2, 0x18, unsigned long)
#define AOD_IOCTL_AOD_PAUSE_NEW _IOR(0xB2, 0x19, unsigned long)
#define AOD_IOCTL_AOD_GET_POS _IOW(0xB2, 0x1A, unsigned long)
#define AOD_IOCTL_AOD_SET_MULTI_GMP _IOW(0xB2, 0x1B, unsigned long)

#define HISI_AOD_OK 0
#define HISI_AOD_FAIL (-1)
#define STATUS_FINGER_CHECK_OK 2
#define DISPALY_SCREEN_ON 0
#define DISPALY_SCREEN_OFF 1
#define SUB_CMD_TYPE 0
#define SCREEN_STATE 1
#define LCD_TYPE_UNKNOWN 0
#define LCD_TYPE_SAMSUNG_S6E3HF4 1

#define MAX_DISPLAY_SPACE_COUNT 13
#define MAX_DYNAMIC_ALLOC_FB_COUNT 64
#define MAX_ADDR_FOR_SENSORHUB     0xFFFFFFFF

#define MAX_BIT_MAP_SIZE 2

#define DIFF_NUMBER 1
#define SHMEM_START_MSG_CMD_TYPE	1
#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V600)
#define SHMEM_START_CONFIG	1
#else
#define SHMEM_START_CONFIG	0
#endif
enum aod_fb_pixel_format {
	AOD_FB_PIXEL_FORMAT_RGB_565 = 0,
	AOD_FB_PIXEL_FORMAT_RGBX_4444,
	AOD_FB_PIXEL_FORMAT_RGBA_4444,
	AOD_FB_PIXEL_FORMAT_RGBX_5551,
	AOD_FB_PIXEL_FORMAT_RGBA_5551,
	AOD_FB_PIXEL_FORMAT_RGBX_8888,
	AOD_FB_PIXEL_FORMAT_RGBA_8888,

	AOD_FB_PIXEL_FORMAT_BGR_565,
	AOD_FB_PIXEL_FORMAT_BGRX_4444,
	AOD_FB_PIXEL_FORMAT_BGRA_4444,
	AOD_FB_PIXEL_FORMAT_BGRX_5551,
	AOD_FB_PIXEL_FORMAT_BGRA_5551,
	AOD_FB_PIXEL_FORMAT_BGRX_8888,
	AOD_FB_PIXEL_FORMAT_BGRA_8888,

	AOD_FB_PIXEL_FORMAT_YUV_422_I,

	/* YUV Semi-planar */
	AOD_FB_PIXEL_FORMAT_YCbCr_422_SP, /* NV16 */
	AOD_FB_PIXEL_FORMAT_YCrCb_422_SP,
	AOD_FB_PIXEL_FORMAT_YCbCr_420_SP,
	AOD_FB_PIXEL_FORMAT_YCrCb_420_SP, /* NV21 */

	/* YUV Planar */
	AOD_FB_PIXEL_FORMAT_YCbCr_422_P,
	AOD_FB_PIXEL_FORMAT_YCrCb_422_P,
	AOD_FB_PIXEL_FORMAT_YCbCr_420_P,
	AOD_FB_PIXEL_FORMAT_YCrCb_420_P, /* AOD_FB_PIXEL_FORMAT_YV12 */

	/* YUV Package */
	AOD_FB_PIXEL_FORMAT_YUYV_422_Pkg,
	AOD_FB_PIXEL_FORMAT_UYVY_422_Pkg,
	AOD_FB_PIXEL_FORMAT_YVYU_422_Pkg,
	AOD_FB_PIXEL_FORMAT_VYUY_422_Pkg,
};

struct aod_common_data {
	uint32_t size;
	uint32_t cmd_type;
	uint32_t data[0];
};

struct aod_general_data {
	uint32_t size;
	uint32_t data[0];
};

struct aod_multi_gmp_data {
	uint32_t size;
	uint32_t index;
	uint32_t data[0];
};

/* aod start */
typedef struct aod_notif {
	size_t size;
	unsigned int aod_events;
} aod_notif_t;

/* AP struct */
typedef struct aod_start_config_ap {
	size_t size;
	aod_display_pos_t aod_pos;
	uint32_t intelli_switching;
	uint32_t fb_offset;
	uint32_t bitmaps_offset;
	int pixel_format;
	uint32_t fp_offset;
	uint32_t fp_count;
	uint32_t aod_type;
	uint32_t fp_mode;
} aod_start_config_ap_t;

typedef struct aod_pause_pos_data {
	size_t size;
	aod_display_pos_t aod_pos;
} aod_pause_pos_data_t;

/* finger down status */
typedef struct aod_notify_finger_down {
	size_t size;
	uint32_t finger_down_status;
} aod_notify_finger_down_t;

typedef struct aod_start_updating_pos_data {
	size_t size;
	aod_display_pos_t aod_pos;
} aod_start_updating_pos_data_t;

typedef struct aod_end_updating_pos_data {
	size_t size;
	aod_display_pos_t aod_pos;
	uint32_t aod_type;
} aod_end_updating_pos_data_t;

typedef struct aod_set_max_and_min_backlight_data {
	size_t size;
	int max_backlight;
	int min_backlight;
} aod_set_max_and_min_backlight_data_t;

typedef struct aod_end_updating_pos_data_to_sensorhub {
	aod_display_pos_t aod_pos;
	uint32_t aod_type;
} aod_end_updating_pos_data_to_sensorhub_t;

typedef struct aod_stop_pos_data {
	size_t size;
	aod_display_pos_t aod_pos;
} aod_stop_pos_data_t;

typedef struct aod_resume_config {
	size_t size;
	aod_display_pos_t aod_pos;
	int intelli_switching;
	uint32_t aod_type;
	uint32_t fp_mode;
} aod_resume_config_t;

typedef struct aod_time_config_ap {
	size_t size;
	uint64_t curr_time;
	int32_t time_zone;
	int32_t sec_time_zone;
	int32_t time_format;
} aod_time_config_ap_t;

typedef struct aod_bitmaps_size_ap {
	size_t size;
	int bitmap_type_count;
	aod_bitmap_size_t bitmap_size[MAX_BIT_MAP_SIZE];
} aod_bitmaps_size_ap_t;

typedef struct aod_display_spaces_ap {
	size_t size;
	unsigned char dual_clocks;
	unsigned char display_space_count;
	uint16_t pd_logo_final_pos_y;
	aod_display_space_t display_spaces[MAX_DISPLAY_SPACE_COUNT];
} aod_display_spaces_ap_t;

typedef struct aod_display_spaces_ap_temp {
	size_t size;
	uint32_t dual_clocks;
	uint32_t display_type;
	uint32_t display_space_count;
	aod_display_space_t display_spaces[MAX_DISPLAY_SPACE_COUNT];
} aod_display_spaces_ap_temp_t;

typedef struct aod_fold_info_config_mcu {
	uint32_t panel_id;
} aod_fold_info_config_mcu_t;

typedef struct aod_fold_info_data {
	uint32_t size;
	uint32_t panel_id;
} aod_fold_info_data_t;

// add
typedef struct aod_dss_ctrl_ap_status {
	struct pkt_header hd;
	uint32_t sub_cmd;
} aod_dss_ctrl_status_ap_t;

/* sensorhub struct */
typedef struct aod_dss_ctrl_sh_status {
	struct pkt_header hd;
	uint32_t sub_cmd;
	uint32_t dss_on; // 1 - dss on, 0 - dss off
	uint32_t success; // 0 - on/off success, non-zero - on/off fail
} aod_dss_ctrl_status_sh_t;

typedef struct aod_start_config_mcu {
#if SHMEM_START_CONFIG
	uint32_t cmd_sub_type; // 1:start msg
#endif
	aod_display_pos_t aod_pos;
	int32_t intelli_switching;
	int32_t aod_type;
	int32_t fp_mode;
	uint32_t dynamic_fb_count;
	uint32_t dynamic_ext_fb_count;
	uint32_t face_id_fb_count;
	uint32_t pd_logo_fb_count;
#if SHMEM_START_CONFIG
	uint32_t digital_clock_count;
	uint32_t analog_clock_count;
	uint32_t pattern_clock_count;
	uint32_t dynamic_reserve_count;
#endif
	uint32_t dynamic_fb_addr[MAX_DYNAMIC_ALLOC_FB_COUNT];
} aod_start_config_mcu_t;

typedef struct aod_info_mcu {
	struct pkt_header_resp head;
	aod_display_pos_t aod_pos;
} aod_info_mcu_t;

typedef struct aod_time_config_mcu {
	uint64_t curr_time;
	int32_t time_zone;
	int32_t sec_time_zone;
	int32_t time_format;
} aod_time_config_mcu_t;

typedef struct aod_display_spaces_mcu {
	unsigned char dual_clocks;
	unsigned char display_space_count;
	uint16_t pd_logo_final_pos_y;
	aod_display_space_t display_spaces[MAX_DISPLAY_SPACE_COUNT];
} aod_display_spaces_mcu_t;

typedef struct aod_bitmaps_size_mcu {
	int bitmap_type_count;
	aod_bitmap_size_t bitmap_size[MAX_BIT_MAP_SIZE];
} aod_bitmaps_size_mcu_t;

typedef struct aod_config_info_mcu {
	uint32_t aod_fb;
	uint32_t aod_digits_addr;
	aod_screen_info_t screen_info;
	aod_bitmaps_size_mcu_t bitmaps_size;
	int32_t fp_offset;
	int32_t fp_count;
} aod_set_config_mcu_t;
typedef struct {
	struct pkt_header hd;
	unsigned int subtype;
	union {
		aod_time_config_mcu_t time_param;
		aod_display_spaces_mcu_t display_param;
		aod_set_config_mcu_t set_config_param;
		aod_display_pos_t display_pos;
		aod_end_updating_pos_data_to_sensorhub_t end_updating_data_to_sensorhub;
	};
} aod_pkt_t;

typedef struct {
	struct pkt_header_resp hd;
	uint8_t data[100];
} aod_pkt_resp_t;

typedef struct aod_ion_buf_fb {
	uint32_t buf_size;
	int32_t ion_buf_fb;
} aod_ion_buf_fb_t;
typedef struct aod_dynamic_fb {
	uint32_t dynamic_fb_count;
	uint32_t dynamic_ext_fb_count;
	uint32_t face_id_fb_count;
	uint32_t pd_logo_fb_count;
#if SHMEM_START_CONFIG
	uint32_t digital_clock_count;
	uint32_t analog_clock_count;
	uint32_t pattern_clock_count;
	uint32_t dynamic_reserve_count;
#endif
	aod_ion_buf_fb_t  str_ion_fb[MAX_DYNAMIC_ALLOC_FB_COUNT] ;
} aod_dynamic_fb_space_t;

struct fb_buf {
	uint32_t size;
	uint32_t addr;
};
struct fb_list {
	uint32_t size;
	uint32_t cmd_type;
	uint32_t dynamic_fb_count;
	struct fb_buf fb[0];
};

struct aod_data_t {
	dev_t devno;
	unsigned long smem_size;
	unsigned long fb_start_addr;
	unsigned long aod_digits_addr;
	uint32_t x_res;
	uint32_t y_res;
	uint32_t bpp;
	uint32_t max_buf_size;
	struct class *aod_dev_class;
	struct device *dev;
	struct device *aod_cdevice;
	struct cdev cdev;
	struct mutex ioctl_lock;
	struct mutex mm_lock;
	bool fb_mem_alloc_flag;
	bool ion_dynamic_alloc_flag;
	struct ion_client *ion_client;
	struct ion_handle *ion_dyn_handle[MAX_DYNAMIC_ALLOC_FB_COUNT];
#ifdef ION_ALLOC_BUFFER
	struct ion_client *ion_client;
	struct ion_handle *ion_handle;
#else
	struct device *cma_device;
	char *buff_virt;
	phys_addr_t buff_phy;
#endif
	aod_start_config_mcu_t start_config_mcu;
	aod_set_config_mcu_t set_config_mcu;
	aod_time_config_mcu_t time_config_mcu;
	aod_bitmaps_size_mcu_t bitmaps_size_mcu;
	aod_display_spaces_mcu_t display_spaces_mcu;
	aod_fold_info_config_mcu_t fold_info_config_mcu;
	aod_display_pos_t pos_data;
	aod_notif_t aod_notify_data;

	int blank_mode;
	int aod_status;
	uint32_t finger_down_status;
	bool start_req_faster;
	bool no_need_enter_aod;
	struct semaphore aod_status_sem;
	struct mutex aod_lock;
	struct wakeup_source  wlock;
	// true:ap hold the aod lock; false:ap release the aod lock
	bool aod_lock_status;
};

#pragma pack(4)
typedef struct {
	struct pkt_header hd;
	uint32_t subtype;
	union {
		aod_fold_info_config_mcu_t fold_info_config_param;
	};
}aod_pkt_pack_t;
#pragma pack()
#endif
