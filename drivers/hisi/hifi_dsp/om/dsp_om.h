/*
 * dsp_om.h
 *
 * socdsp om.
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#ifndef __DSP_OM_H__
#define __DSP_OM_H__

#include <linux/platform_device.h>

#ifndef LOG_TAG
#define LOG_TAG "socdsp_misc "
#endif

#define SOCDSP_SEC_MAX_NUM 100

enum socdsp_om_work_id {
	SOCDSP_OM_WORK_VOICE_BSD = 0,
	SOCDSP_OM_WORK_AUDIO_OM_DETECTION,
	SOCDSP_OM_WORK_VOICE_3A,
	SOCDSP_OM_WORK_VOICE_BIGDATA,
	SOCDSP_OM_WORK_SMARTPA_DFT,
	SOCDSP_OM_WORK_AUDIO_DB,
	SOCDSP_OM_WORK_AUXHEAR_BIGDATA,
	SOCDSP_OM_WORK_MAX
};

enum print_level_num {
	PRINT_LEVEL_ERROR = 0,
	PRINT_LEVEL_WARN,
	PRINT_LEVEL_INFO,
	PRINT_LEVEL_DEBUG,
	PRINT_LEVEL_MAX
};

struct socdsp_om_work_ctl {
	struct workqueue_struct *wq;
	struct work_struct work;
	spinlock_t lock;
	struct list_head list;
};

struct socdsp_om_work_info {
	enum socdsp_om_work_id work_id;
	char *work_name;
	work_func_t func;
	struct socdsp_om_work_ctl ctl;
};

struct socdsp_om_ap_data {
	unsigned short msg_id;
	unsigned int data_len;
	unsigned char data[0];
};

struct socdsp_om_work {
	struct list_head om_node;
	unsigned int data_len;
	unsigned char data[0];
};

struct drv_socdsp_image_sec {
	unsigned short sn;
	unsigned char type;
	unsigned char load_attib;
	unsigned int src_offset;
	unsigned int des_addr;
	unsigned int size;
};

struct drv_socdsp_image_head {
	char time_stamp[24];
	unsigned int image_size;
	unsigned int sections_num;
	struct drv_socdsp_image_sec sections[SOCDSP_SEC_MAX_NUM];
};

unsigned int om_get_dsp_timestamp(void);
unsigned int om_get_print_level(void);

#define UNUSED_PARAMETER(x) (void)(x)
#define DSP_STAMP om_get_dsp_timestamp()

/*lint -e683 */
#define logd(fmt, ...) \
do { \
	if (om_get_print_level() >= 3) { \
		printk(LOG_TAG"[D][%u]:%s:%d: "fmt, DSP_STAMP, __func__, __LINE__, ##__VA_ARGS__); \
	} \
} while (0)

#define logi(fmt, ...) \
do { \
	if (om_get_print_level() >= 2) { \
		printk(LOG_TAG"[I][%u]:%s:%d: "fmt, DSP_STAMP, __func__, __LINE__, ##__VA_ARGS__); \
	} \
} while (0)


#define logw(fmt, ...) \
do { \
	if (om_get_print_level() >= 1) { \
		printk(LOG_TAG"[W][%u]:%s:%d: "fmt, DSP_STAMP, __func__, __LINE__, ##__VA_ARGS__); \
	} \
} while (0)

#define loge(fmt, ...) \
		printk(LOG_TAG"[E][%u]:%s:%d: "fmt, DSP_STAMP, __func__, __LINE__, ##__VA_ARGS__)
/*lint +e683 */

void om_set_print_level(unsigned int level);
bool is_dsp_img_loaded(void);
void dsp_om_init(struct platform_device *dev, unsigned char *unsec_virt_addr);
void dsp_om_deinit(struct platform_device *dev);

#endif /* __DSP_OM_H__ */

