 /* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_display_effect.h"
#include "hisi_fb.h"
#include <linux/fb.h>
#include "global_ddr_map.h"
#include <linux/hisi/hw_cmdline_parse.h>

//lint -e747, -e838, -e774

#define COUNT_LIMIT_TO_PRINT_DELAY			(200)

typedef struct time_interval {
	long start; 				// microsecond
	long stop;
} time_interval_t;

typedef struct delay_record {
	const char *name;
	long max;
	long min;
	long sum;
	int count;
} delay_record_t;

static int hisifb_ce_do_contrast(struct hisi_fb_data_type *hisifd);
static void hisifb_ce_service_init(void);
static void hisifb_ce_service_deinit(void);

#define DEBUG_EFFECT_LOG					HISI_FB_ERR

#define _EFFECT_DEBUG_LOG(message, ...) \
	do { printk("%s: "message, __func__, ## __VA_ARGS__); } while (0)
#define EFFECT_DEBUG_LOG(feature, message, ...) \
	do { if (g_debug_effect & feature) \
		printk("%s: "message, __func__, ## __VA_ARGS__); } while (0)

#define EFFECT_GRADUAL_REFRESH_FRAMES		(30)

static bool g_is_effect_init = false;
static bool g_is_ce_service_init = false;
static struct mutex g_ce_service_lock;
static ce_service_t g_hiace_service;

static bool g_is_effect_lock_init = false;
static spinlock_t g_gmp_effect_lock;
static spinlock_t g_igm_effect_lock;
static spinlock_t g_xcc_effect_lock;
static spinlock_t g_gamma_effect_lock;
extern struct mutex g_rgbw_lock;

static time_interval_t interval_wait_hist = {0};
static time_interval_t interval_algorithm = {0};
static delay_record_t delay_wait_hist = {"event hist waiting", 0, 0xFFFFFFFF, 0, 0};
static delay_record_t delay_algorithm = {"algorithm processing", 0, 0xFFFFFFFF, 0, 0};

int g_enable_effect = ENABLE_EFFECT_HIACE | ENABLE_EFFECT_BL;
int g_debug_effect = 0;
static bool hiace_enable_status = false;

static inline long get_timestamp_in_us(void)
{
	struct timespec ts;
	long timestamp;
	ktime_get_ts(&ts);
	timestamp = ts.tv_sec * USEC_PER_SEC + ts.tv_nsec / NSEC_PER_USEC;
	return timestamp;
}

static inline void count_delay(delay_record_t *record, long delay)
{
	if (NULL == record) {
		return;
	}
	record->count++;
	record->sum += delay;
	if (delay > record->max) record->max = delay;
	if (delay < record->min) record->min = delay;
	if (!(record->count % COUNT_LIMIT_TO_PRINT_DELAY)) {
		DEBUG_EFFECT_LOG("[effect] Delay(us/%4d) | average:%5ld | min:%5ld | max:%8ld | %s\n", record->count, record->sum / record->count, record->min, record->max, record->name);
		record->count = 0;
		record->sum = 0;
		record->max = 0;
		record->min = 0xFFFFFFFF;
	}
}

static inline uint32_t get_fixed_point_offset(uint32_t half_block_size)
{
	uint32_t num = 2;
	uint32_t len = 2;
	while (len < half_block_size) {
		num++;
		len <<= 1;
	}
	return num;
}

#define ACM_HUE_LUT_LENGTH ((uint32_t)256)
#define ACM_SATA_LUT_LENGTH ((uint32_t)256)
#define ACM_SATR_LUT_LENGTH ((uint32_t)64)

#define LCP_GMP_LUT_LENGTH	((uint32_t)9*9*9)
#define LCP_XCC_LUT_LENGTH	((uint32_t)12)

#define IGM_LUT_LEN ((uint32_t)257)
#define GAMMA_LUT_LEN ((uint32_t)257)

#define BYTES_PER_TABLE_ELEMENT 4

static int hisi_effect_copy_to_user(uint32_t *table_dst, uint32_t *table_src, uint32_t table_length)
{
	unsigned long table_size = 0;

	if ((NULL == table_dst) || (NULL == table_src) || (table_length == 0)) {
		HISI_FB_ERR("invalid input parameters.\n");
		return -EINVAL;
	}

	table_size = (unsigned long)table_length * BYTES_PER_TABLE_ELEMENT;

	if (copy_to_user(table_dst, table_src, table_size)) {
		HISI_FB_ERR("failed to copy table to user.\n");
		return -EINVAL;
	}

	return 0;
}

static int hisi_effect_alloc_and_copy(uint32_t **table_dst, uint32_t *table_src,
	uint32_t lut_table_length, bool copy_user)
{
	uint32_t *table_new = NULL;
	unsigned long table_size = 0;

	if ((NULL == table_dst) ||(NULL == table_src) ||  (lut_table_length == 0)) {
		HISI_FB_ERR("invalid input parameter");
		return -EINVAL;
	}

	table_size = (unsigned long)lut_table_length * BYTES_PER_TABLE_ELEMENT;

	if (*table_dst == NULL) {
		table_new = (uint32_t *)kmalloc(table_size, GFP_ATOMIC);
		if (table_new) {
			memset(table_new, 0, table_size);
			*table_dst = table_new;
		} else {
			HISI_FB_ERR("failed to kmalloc lut_table!\n");
			return -EINVAL;
		}
	}

	if (copy_user) {
		if (copy_from_user(*table_dst, table_src, table_size)) {
			HISI_FB_ERR("failed to copy table from user\n");
			if (table_new)
				kfree(table_new);
			*table_dst = NULL;
			return -EINVAL;
		}
	} else {
		memcpy(*table_dst, table_src, table_size);
	}

	return 0;
}

static void hisi_effect_kfree(uint32_t **free_table)
{
	if (*free_table) {
		kfree((uint32_t *) *free_table);
		*free_table = NULL;
	}
}

static int hisifb_ce_do_contrast(struct hisi_fb_data_type *hisifd)
{
	ce_service_t *service = &g_hiace_service;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (g_is_ce_service_init) {
		service->new_hist = true;
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_wait_hist.start = get_timestamp_in_us();
		}
		wake_up_interruptible(&service->wq_hist);
	}

	return 0;
}

static inline int handle_err_hist(struct fb_info *info, int wait_ret) {
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return -EINVAL;
	}

	if (hisifd->panel_power_on) {
		if (wait_ret > 0) {
			ret = 3;//panel on hist not return hist stop is true
		} else if (wait_ret == -ERESTARTSYS){
			hisifb_ce_service_deinit();
			ret = 4;//system err and return -ERESTARTSYS
		} else {
			ret = 2;//hist not return time out
		}
	} else {
		ret = 1;//panel off hist not return hist stop is true
	}

	if ((g_debug_effect & DEBUG_EFFECT_ENTRY) && wait_ret != 0) {
		DEBUG_EFFECT_LOG("[effect] wait_event_interruptible_timeout() return %d, -ERESTARTSYS:%d\n", wait_ret, -ERESTARTSYS);
	}

	return ret;
}

void hisi_effect_init(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_ce_info_t *ce_info = NULL;
	hiace_alg_parameter_t *param = NULL;

	if (!g_is_effect_lock_init) {
		spin_lock_init(&g_gmp_effect_lock);
		spin_lock_init(&g_igm_effect_lock);
		spin_lock_init(&g_xcc_effect_lock);
		spin_lock_init(&g_gamma_effect_lock);
		mutex_init(&g_rgbw_lock);
		g_is_effect_lock_init = true;
	}

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not supported!\n");
		}
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ce_info = &hisifd->hiace_info;
		param = &pinfo->hiace_param;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	if (!g_is_effect_init) {
		mutex_init(&g_ce_service_lock);
		mutex_init(&(hisifd->al_ctrl.ctrl_lock));
		mutex_init(&(hisifd->ce_ctrl.ctrl_lock));
		mutex_init(&(hisifd->bl_ctrl.ctrl_lock));
		mutex_init(&(hisifd->bl_enable_ctrl.ctrl_lock));
		mutex_init(&(hisifd->metadata_ctrl.ctrl_lock));
		hisifd->bl_enable_ctrl.ctrl_bl_enable = 1;

		memset(ce_info, 0, sizeof(dss_ce_info_t));
		ce_info->algorithm_result = 1;
		mutex_init(&(ce_info->hist_lock));
		mutex_init(&(ce_info->lut_lock));

		param->iWidth = (int)pinfo->xres;
		param->iHeight = (int)pinfo->yres;
		param->iMode = 0;
		param->bitWidth = 10;
		param->iMinBackLight = (int)hisifd->panel_info.bl_min;
		param->iMaxBackLight = (int)hisifd->panel_info.bl_max;
		param->iAmbientLight = -1;

		memset(&g_hiace_service, 0, sizeof(g_hiace_service));
		init_waitqueue_head(&g_hiace_service.wq_hist);

		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] width:%d, height:%d, minbl:%d, maxbl:%d\n", param->iWidth, param->iHeight, param->iMinBackLight, param->iMaxBackLight);
		}

		g_is_effect_init = true;
	} else {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] bypass\n");
		}
	}
}

void hisi_effect_deinit(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_ce_info_t *ce_info = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not supported!\n");
		}
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ce_info = &hisifd->hiace_info;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	if (g_is_effect_lock_init) {
		g_is_effect_lock_init = false;
		mutex_destroy(&g_rgbw_lock);
	}

	down(&hisifd->hiace_hist_lock_sem);/*avoid  using  mutex_lock() but hist_lock was destoried by mutex_destory in  hisi_effect_deinit*/
	if (g_is_effect_init) {
		g_is_effect_init = false;

		mutex_destroy(&(ce_info->hist_lock));
		mutex_destroy(&(ce_info->lut_lock));

		mutex_destroy(&(hisifd->al_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->ce_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->bl_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->bl_enable_ctrl.ctrl_lock));
		mutex_destroy(&(hisifd->metadata_ctrl.ctrl_lock));

		mutex_destroy(&g_ce_service_lock);
	} else {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] bypass\n");
		}
	}
	up(&hisifd->hiace_hist_lock_sem);
}

static void hisifb_ce_service_init(void)
{
	mutex_lock(&g_ce_service_lock);
	if (!g_is_ce_service_init) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] step in\n");
		}

		g_hiace_service.is_ready = true;
		g_hiace_service.stop = false;
		g_hiace_service.hist_stop = false;

		g_is_ce_service_init = true;
	}
	mutex_unlock(&g_ce_service_lock);
}

static void hisifb_ce_service_deinit(void)
{
	mutex_lock(&g_ce_service_lock);
	if (g_is_ce_service_init) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] step in\n");
		}

		g_is_ce_service_init = false;
		g_hiace_service.is_ready = false;
		g_hiace_service.stop = true;
		g_hiace_service.hist_stop = true;

		wake_up_interruptible(&g_hiace_service.wq_hist);
	}
	mutex_unlock(&g_ce_service_lock);
}

static inline void enable_hiace(struct hisi_fb_data_type *hisifd, bool enable)
{
	if (enable) {
		hisifb_ce_service_init();
	} else {
		hisifb_ce_service_deinit();
	}

	down(&hisifd->blank_sem);
	if (hisifd->panel_power_on) {
		if (hisifd->hiace_info.hiace_enable != enable) { //lint !e731
			hisifd->hiace_info.hiace_enable = enable;
			EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] hiace:%d\n", (int)enable);
		}
	} else {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] fb%d, panel power off!\n", hisifd->index);
	}
	up(&hisifd->blank_sem);
}
int hisifb_ce_service_blank(int blank_mode, struct fb_info *info)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (pinfo->hiace_support) {
			if (blank_mode == FB_BLANK_UNBLANK) {
				hisifb_ce_service_init();
				if (hisifb_display_effect_is_need_ace(hisifd)) {
					enable_hiace(hisifd, true);
				}
			} else {
				if (hisifb_display_effect_is_need_ace(hisifd)) {
					g_hiace_service.use_last_value = true;
				}
				hisifd->hiace_info.gradual_frames = 0;
				hisifd->hiace_info.hiace_enable = false;
				hisifd->hiace_info.to_stop_hdr = false;
				hisifd->hiace_info.to_stop_sre = false;
				g_hiace_service.blc_used = false;
				hiace_enable_status = false;

				// Since java has no destruct function and Gallery will refresh metadata when power on, always close HDR for Gallery when power off.
				if (hisifd->ce_ctrl.ctrl_ce_mode == CE_MODE_IMAGE) {
					hisifd->ce_ctrl.ctrl_ce_mode = CE_MODE_DISABLE;
				}
				hisifb_ce_service_deinit();
			}
		}
	}
	return 0;
}

int hisifb_ce_service_get_support(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	unsigned int support = 0;
	int ret = 0;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support && (g_dss_version_tag == FB_ACCEL_KIRIN970)) {
		support = 1;
	}
	if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
		DEBUG_EFFECT_LOG("[effect] support:%d\n", support);
	}

	ret = (int)copy_to_user(argp, &support, sizeof(support));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user failed! ret=%d.\n", ret);
		return ret;
	}

	return ret;
}

int hisifb_ce_service_get_limit(struct fb_info *info, void __user *argp)
{
	int limit = 0;
	int ret = 0;

	if (NULL == argp) {
		HISI_FB_ERR("argp is NULL\n");
		return -EINVAL;
	}

	ret = (int)copy_to_user(argp, &limit, sizeof(limit));
	if (ret) {
		HISI_FB_ERR("copy_to_user failed! ret=%d.\n", ret);
		return ret;
	}

	return ret;
}

int hisifb_ce_service_get_hiace_param(struct fb_info *info, void __user *argp){
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_effect_info effect_info;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] Don't support HIACE\n");
		}
		return -EINVAL;
	}

	ret = copy_from_user(&effect_info, argp, sizeof(struct dss_effect_info));
	if (ret) {
		HISI_FB_ERR("failed to copy dss_effect_info from user space.\n");
		return	-EINVAL;
	}

	hisi_effect_hiace_info_get(hisifd, &(effect_info.hiace));

	ret = copy_to_user(argp, &effect_info, sizeof(struct dss_effect_info));
	if (ret) {
		HISI_FB_ERR("failed to copy result of ioctl to user space.\n");
		return	-EINVAL;
	}

	return 0;
}


int hisifb_ce_service_get_param(struct fb_info *info, void __user *argp)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	dss_display_effect_al_t *al_ctrl = NULL;
	dss_display_effect_metadata_t *metadata_ctrl = NULL;
	int mode = 0;
	struct timespec ts;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] Don't support HIACE\n");
		}
		return -EINVAL;
	}

	ce_ctrl = &(hisifd->ce_ctrl);
	al_ctrl = &(hisifd->al_ctrl);
	metadata_ctrl = &(hisifd->metadata_ctrl);
	mode = ce_ctrl->ctrl_ce_mode;
	if (mode != 0) {
		pinfo->hiace_param.iDoLCE = 1;
		pinfo->hiace_param.iDoAPLC = 1;
	}
	pinfo->hiace_param.iLevel = MAX(mode - 1, 0);
	pinfo->hiace_param.iAmbientLight = al_ctrl->ctrl_al_value;
	pinfo->hiace_param.iBackLight = (int)hisifd->bl_level;
	ktime_get_ts(&ts);
	pinfo->hiace_param.lTimestamp = ts.tv_sec * MSEC_PER_SEC + ts.tv_nsec / NSEC_PER_MSEC;
	if (metadata_ctrl->count <= META_DATA_SIZE) {
		memcpy(pinfo->hiace_param.Classifieresult, metadata_ctrl->metadata, (size_t)metadata_ctrl->count);
		pinfo->hiace_param.iResultLen = metadata_ctrl->count;
	}

	ret = (int)copy_to_user(argp, &pinfo->hiace_param, sizeof(pinfo->hiace_param));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user(hiace_param) failed! ret=%d.\n", ret);
		return ret;
	}

	if (g_debug_effect & DEBUG_EFFECT_FRAME) {
		DEBUG_EFFECT_LOG("[effect] iLevel:%d, iAmbientLight:%d, iBackLight:%d, lTimestamp:%ld(ms)\n",
						 pinfo->hiace_param.iLevel, pinfo->hiace_param.iAmbientLight, pinfo->hiace_param.iBackLight, pinfo->hiace_param.lTimestamp);
	}

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_algorithm.start = get_timestamp_in_us();
	}

	return ret;
}

int hisifb_ce_service_get_hist(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ce_service_t *service = &g_hiace_service;
	int ret = 0;
	long wait_ret = 0;
	long timeout = msecs_to_jiffies(100000);

	if (runmode_is_factory()) {
		return ret;
	}

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	if (g_debug_effect & DEBUG_EFFECT_FRAME) {
		DEBUG_EFFECT_LOG("[effect] wait hist\n");
	}

	if (g_is_effect_init) {
		unlock_fb_info(info);
		wait_ret = wait_event_interruptible_timeout(service->wq_hist, service->new_hist || service->hist_stop, timeout);
		lock_fb_info(info);
		service->hist_stop = false;
	}
	if (!g_is_effect_init) {
		HISI_FB_ERR("[effect] wq_hist uninit.\n");
		return -EINVAL;
	}

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_wait_hist.stop = get_timestamp_in_us();
		count_delay(&delay_wait_hist, interval_wait_hist.stop - interval_wait_hist.start);
	}

	down(&hisifd->hiace_hist_lock_sem);/*avoid  using  mutex_lock() but hist_lock was destoried by mutex_destory in  hisi_effect_deinit*/
	if (service->new_hist) {
		time_interval_t interval_copy_hist = {0};
		static delay_record_t delay_copy_hist = {"hist copy", 0, 0xFFFFFFFF, 0, 0};

		service->new_hist = false;

		if(!g_is_effect_init){
			HISI_FB_ERR("[effect] wq_hist uninit here\n");
			up(&hisifd->hiace_hist_lock_sem);
			return -EINVAL;
		}
		mutex_lock(&hisifd->hiace_info.hist_lock);
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_copy_hist.start = get_timestamp_in_us();
		}
		ret = (int)copy_to_user(argp, hisifd->hiace_info.histogram, sizeof(hisifd->hiace_info.histogram));
		if (ret) {
			HISI_FB_ERR("[effect] copy_to_user failed(param)! ret=%d.\n", ret);
			ret = -1;
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_copy_hist.stop = get_timestamp_in_us();
			count_delay(&delay_copy_hist, interval_copy_hist.stop - interval_copy_hist.start);
		}
		mutex_unlock(&hisifd->hiace_info.hist_lock);
	} else {
		ret = handle_err_hist(info, wait_ret);
	}
	up(&hisifd->hiace_hist_lock_sem);

	return ret;
}

int hisifb_ce_service_set_lut(struct fb_info *info, void __user *argp)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_display_effect_bl_t *bl_ctrl = NULL;
	dss_display_effect_bl_enable_t *bl_enable_ctrl = NULL;
	hiace_interface_set_t hiace_set_interface;
	int ret = 0;
	time_interval_t interval_copy_lut = {0};
	static delay_record_t delay_copy_lut = {"lut copy", 0, 0xFFFFFFFF, 0, 0};

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_algorithm.stop = get_timestamp_in_us();
		count_delay(&delay_algorithm, interval_algorithm.stop - interval_algorithm.start);
	}

	if (runmode_is_factory()) {
		return ret;
	}

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}

	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] Don't support HIACE\n");
		}
		return -EINVAL;
	}

	if (g_debug_effect & DEBUG_EFFECT_FRAME) {
		DEBUG_EFFECT_LOG("[effect] step in\n");
	}

	bl_ctrl = &(hisifd->bl_ctrl);
	bl_enable_ctrl = &(hisifd->bl_enable_ctrl);

	ret = (int)copy_from_user(&hiace_set_interface, argp, sizeof(hiace_interface_set_t));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return -2;
	}

	mutex_lock(&hisifd->hiace_info.lut_lock);
	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_copy_lut.start = get_timestamp_in_us();
	}
	ret = (int)hisi_effect_alloc_and_copy(&hisifd->hiace_info.lut_table, hiace_set_interface.lut, CE_SIZE_LUT, true);

	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(lut_table) failed! ret=%d.\n", ret);
		ret = -2;
	}

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_copy_lut.stop = get_timestamp_in_us();
		count_delay(&delay_copy_lut, interval_copy_lut.stop - interval_copy_lut.start);
	}
	mutex_unlock(&hisifd->hiace_info.lut_lock);
	hisifd->hiace_info.algorithm_result = 0;

	return ret;
}

ssize_t hisifb_display_effect_al_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_al_t *al_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	al_ctrl = &(hisifd->al_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", al_ctrl->ctrl_al_value);
}

ssize_t hisifb_display_effect_al_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;

	return (ssize_t)count;
}

ssize_t hisifb_display_effect_ce_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	ce_ctrl = &(hisifd->ce_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", ce_ctrl->ctrl_ce_mode);
}

ssize_t hisifb_display_effect_ce_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;
	return (ssize_t)count;
}

ssize_t hisifb_display_effect_bl_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_bl_t *bl_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	bl_ctrl = &(hisifd->bl_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", bl_ctrl->ctrl_bl_delta);
}

ssize_t hisifb_display_effect_bl_enable_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_bl_enable_t *bl_enable_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("[effect] info is NULL\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	bl_enable_ctrl = &(hisifd->bl_enable_ctrl);

	return snprintf(buf, PAGE_SIZE, "%d\n", bl_enable_ctrl->ctrl_bl_enable);
}

ssize_t hisifb_display_effect_bl_enable_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;

	return (ssize_t)count;
}

ssize_t hisifb_display_effect_sre_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_sre_t *sre_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	sre_ctrl = &(hisifd->sre_ctrl);

	return snprintf(buf, PAGE_SIZE, "sre_enable:%d, sre_al:%d\n", sre_ctrl->ctrl_sre_enable, sre_ctrl->ctrl_sre_al);
}
/*lint -e438, -e550, -e715*/
ssize_t hisifb_display_effect_sre_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_sre_t *sre_ctrl = NULL;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	sre_ctrl = &(hisifd->sre_ctrl);

	return (ssize_t)count;
}

ssize_t hisifb_display_effect_metadata_ctrl_show(struct fb_info *info, char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == info) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer\n");
		return -1;
	}

	return 0;
}

ssize_t hisifb_display_effect_metadata_ctrl_store(struct fb_info *info, const char *buf, size_t count)
{
	(void)info, (void)buf;

	return (ssize_t)count;
}

void hisifb_display_effect_func_switch(struct hisi_fb_data_type *hisifd, const char *command)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}
	if (NULL == command) {
		HISI_FB_ERR("[effect] command is NULL\n");
		return;
	}

	if (!strncmp("hiace:", command, strlen("hiace:"))) {
		if('0' == command[strlen("hiace:")]) {
			hisifd->panel_info.hiace_support = 0;
			HISI_FB_INFO("[effect] hiace disable\n");
		} else {
			hisifd->panel_info.hiace_support = 1;
			HISI_FB_INFO("[effect] hiace enable\n");
		}
	}
	if (!strncmp("effect_enable:", command, strlen("effect_enable:"))) {
		g_enable_effect = (int)simple_strtoul(&command[strlen("effect_enable:")], NULL, 0);
		HISI_FB_INFO("[effect] effect_enable changed to %d\n", g_enable_effect);
	}
	if (!strncmp("effect_debug:", command, strlen("effect_debug:"))) {
		g_debug_effect = (int)simple_strtoul(&command[strlen("effect_debug:")], NULL, 0);
		HISI_FB_INFO("[effect] effect_debug changed to %d\n", g_debug_effect);
	}
}

bool hisifb_display_effect_is_need_ace(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return false;
	}

	return (g_enable_effect & ENABLE_EFFECT_HIACE) && (hisifd->ce_ctrl.ctrl_ce_mode > 0 || hisifd->sre_ctrl.ctrl_sre_enable == 1 || hisifd->hiace_info.to_stop_hdr || hisifd->hiace_info.to_stop_sre);
}

bool hisifb_display_effect_is_need_blc(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
	return false;
	}
	return hisifd->de_info.blc_enable;
}

static int deltabl_process(struct hisi_fb_data_type *hisifd, int backlight_in)
{
	int ret = 0;
	int bl_min = (int)hisifd->panel_info.bl_min;
	int bl_max = (int)hisifd->panel_info.bl_max;
	bool HBMEnable = hisifd->de_info.amoled_param.HBMEnable ? true:false;
	bool AmoledDimingEnable = hisifd->de_info.amoled_param.AmoledDimingEnable ? true:false;
	int HBM_Threshold_BackLight = hisifd->de_info.amoled_param.HBM_Threshold_BackLight;
	int HBM_Min_BackLight = hisifd->de_info.amoled_param.HBM_Min_BackLight;
	int HBM_Max_BackLight = hisifd->de_info.amoled_param.HBM_Max_BackLight;
	//int HBM_MinLum_Regvalue = hisifd->de_info.amoled_param.HBM_MinLum_Regvalue;
	//int HBM_MaxLum_Regvalue = hisifd->de_info.amoled_param.HBM_MaxLum_Regvalue;
	int Hiac_DBVThres = hisifd->de_info.amoled_param.Hiac_DBVThres;
	int Hiac_DBV_XCCThres = hisifd->de_info.amoled_param.Hiac_DBV_XCCThres;
	int Hiac_DBV_XCC_MinThres = hisifd->de_info.amoled_param.Hiac_DBV_XCC_MinThres;
	int current_hiac_backlight = 0;
	int current_hiac_deltaBL = 0;
	int temp_hiac_backlight = 0;
	int origin_hiac_backlight = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL \n");
		return -1;
	}

	origin_hiac_backlight = (backlight_in - bl_min) * (HBM_Max_BackLight - HBM_Min_BackLight) /(bl_max - bl_min) + HBM_Min_BackLight;
	current_hiac_backlight = origin_hiac_backlight;

	if (HBMEnable && (HBM_Max_BackLight > HBM_Threshold_BackLight) && (HBM_Threshold_BackLight > (HBM_Min_BackLight + 1))) {
		if ((backlight_in >= bl_min) && (backlight_in < HBM_Threshold_BackLight)) {
			current_hiac_deltaBL = HBM_Min_BackLight + ((current_hiac_backlight - HBM_Min_BackLight) * (HBM_Max_BackLight - HBM_Min_BackLight) /(HBM_Threshold_BackLight - 1 - HBM_Min_BackLight)) - current_hiac_backlight;
		} else if ((current_hiac_backlight >= HBM_Threshold_BackLight) && (backlight_in <= bl_max)) {
			current_hiac_deltaBL = HBM_Max_BackLight - current_hiac_backlight;
		}
		current_hiac_backlight = current_hiac_backlight + current_hiac_deltaBL;
		HISI_FB_DEBUG("[effect] hiac_delta =  %d hiac_backlight =  %d, backlight = %d", current_hiac_deltaBL,current_hiac_backlight, backlight_in);
	}

	if (AmoledDimingEnable) {
		if ((current_hiac_backlight <= Hiac_DBVThres) && (current_hiac_backlight > Hiac_DBV_XCCThres)) {
			current_hiac_deltaBL = Hiac_DBVThres - origin_hiac_backlight;
		} else if ((current_hiac_backlight <= Hiac_DBV_XCCThres)) {
			temp_hiac_backlight = (current_hiac_backlight - HBM_Min_BackLight ) * (Hiac_DBVThres - Hiac_DBV_XCC_MinThres) /(Hiac_DBV_XCCThres - HBM_Min_BackLight) + Hiac_DBV_XCC_MinThres;
			current_hiac_deltaBL = temp_hiac_backlight - origin_hiac_backlight;
		}
		HISI_FB_DEBUG("[effect] hiac_delta =  %d hiac_backlight =  %d, backlight = %d", current_hiac_deltaBL,current_hiac_backlight, backlight_in);
	}

	hisifd->de_info.blc_delta = (bl_max - bl_min) * current_hiac_deltaBL /(HBM_Max_BackLight -HBM_Min_BackLight);
	HISI_FB_DEBUG("[effect] first screen on ! delta: -10000 -> %d bl: %d (%d)\n",hisifd->de_info.blc_delta,backlight_in,backlight_in+hisifd->de_info.blc_delta);

	return ret;
}

static void handle_first_deltabl(struct hisi_fb_data_type *hisifd, int backlight_in)
{
	bool HBMEnable = hisifd->de_info.amoled_param.HBMEnable ? true:false;
	bool AmoledDimingEnable = hisifd->de_info.amoled_param.AmoledDimingEnable ? true:false;
	int bl_min = (int)hisifd->panel_info.bl_min;
	int bl_max = (int)hisifd->panel_info.bl_max;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	if (!(HBMEnable || AmoledDimingEnable)) {
		HISI_FB_DEBUG("[effect] HBM and Amoled disable\n");
		return;
	}

	if (hisifd->bl_level == 0) {
		hisifd->de_info.blc_delta = -10000;
		return;
	}

	if (backlight_in >0 && (backlight_in < bl_min || backlight_in > bl_max)) {
		return;
	}

	if (hisifd->de_info.blc_delta == -10000) {
		deltabl_process(hisifd,backlight_in);
	}
}

bool hisifb_display_effect_check_bl_value(int curr, int last) {
	if (abs(curr - last) > 200) {
		return true;
	}
	return false;
}

bool hisifb_display_effect_check_bl_delta(int curr, int last) {
	if (abs(curr - last) > 20) {
		return true;
	}
	return false;
}

static inline void display_engine_bl_debug_print(int bl_in, int bl_out, int delta) {
	static int last_delta = 0;
	static int last_bl = 0;
	static int last_bl_out = 0;
	static int count = 0;
	if (hisifb_display_effect_check_bl_value(bl_in, last_bl) || hisifb_display_effect_check_bl_value(bl_out, last_bl_out) || hisifb_display_effect_check_bl_delta(delta, last_delta)) {
		if (count == 0) {
			HISI_FB_INFO("[effect] last delta:%d bl:%d->%d\n", last_delta, last_bl, last_bl_out);
		}
		count = DISPLAYENGINE_BL_DEBUG_FRAMES;
	}
	if (count > 0) {
		HISI_FB_INFO("[effect] delta:%d bl:%d->%d\n", delta, bl_in, bl_out);
		count--;
	}
	last_delta = delta;
	last_bl = bl_in;
	last_bl_out = bl_out;
}

bool hisifb_display_effect_fine_tune_backlight(struct hisi_fb_data_type *hisifd, int backlight_in, int *backlight_out)
{
	bool changed = false;
	struct hisi_panel_info* pinfo = NULL;
	int delta = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return false;
	}

	pinfo = &(hisifd->panel_info);
	if(NULL == pinfo) {
		HISI_FB_ERR("pinfo is NULL!\n");
		return false;
	}

	if (NULL == backlight_out) {
		HISI_FB_ERR("[effect] backlight_out is NULL\n");
		return false;
	}

	if (hisifd->panel_info.need_skip_delta) {
		hisifd->panel_info.need_skip_delta = 0;
		return changed;
	}

	handle_first_deltabl(hisifd,backlight_in);

	/*if ((pinfo->hbm_support == 1) && (backlight_in <= (int)hisifd->panel_info.bl_min)) {
		HISI_FB_DEBUG("[effect] don't need add delta_bl for bl_min\n");
		return false;
	}*/

	if (hisifd->bl_level > 0) {
		if (hisifb_display_effect_is_need_blc(hisifd)) {
			int bl = MIN((int)hisifd->panel_info.bl_max, MAX((int)hisifd->panel_info.bl_min, backlight_in + hisifd->de_info.blc_delta));

			if (hisifd->de_info.amoled_param.AmoledDimingEnable) {
				if (backlight_in >= hisifd->de_info.amoled_param.Lowac_DBV_XCCThres && backlight_in <= hisifd->de_info.amoled_param.Lowac_DBVThres) {
					bl = hisifd->de_info.amoled_param.Lowac_Fixed_DBVThres;
				}
			}

			if (*backlight_out != bl) {
				HISI_FB_DEBUG("[effect] delta:%d bl:%d(%d)->%d\n", hisifd->de_info.blc_delta, backlight_in, hisifd->bl_level, bl);
				*backlight_out = bl;
				changed = true;
				hisifd->de_info.blc_used = true;
			}
			delta = hisifd->de_info.blc_delta;
		} else {
			if (hisifd->de_info.blc_used) {
				if (*backlight_out != backlight_in) {
					HISI_FB_DEBUG("[effect] bl:%d->%d\n", *backlight_out, backlight_in);
					*backlight_out = backlight_in;
					changed = true;
				}
				hisifd->de_info.blc_used = false;
			}
			delta = 0;
		}
	}
	display_engine_bl_debug_print(backlight_in, *backlight_out, delta);

	return changed;
}


int hisifb_display_effect_blc_cabc_update(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -1;
	}

	if ((hisifd->panel_info.blpwm_input_ena || hisifb_display_effect_is_need_blc(hisifd)) && hisifd->cabc_update) {
		hisifd->cabc_update(hisifd);
	}

	return 0;
}

//lint -e845, -e732, -e774
/* HIACE */
//static u32 hiace_lut[] = {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992};

void init_hiace(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *hiace_base = NULL;
	struct hiace_info *hiace_param = NULL;
	unsigned long dw_jiffies = 0;
	uint32_t tmp = 0;
	bool is_ready = false;

	uint32_t global_hist_ab_work;
	uint32_t global_hist_ab_shadow;
	uint32_t gamma_ab_work;
	uint32_t gamma_ab_shadow;
	uint32_t width;
	uint32_t height;
	uint32_t half_block_w;
	uint32_t half_block_h;
	uint32_t pipe_mode;
	uint32_t partition_mode;
	uint32_t xPartition;
	uint32_t is_left_pipe;
	uint32_t lhist_quant;
	uint32_t lhist_sft;
	uint32_t slop;
	uint32_t th_max;
	uint32_t th_min;
	uint32_t up_thres;
	uint32_t low_thres;
	uint32_t fixbit_x;
	uint32_t fixbit_y;
	uint32_t reciprocal_x;
	uint32_t reciprocal_y;

	uint32_t block_pixel_num;
	uint32_t max_lhist_block_pixel_num;
	uint32_t max_lhist_bin_reg_num;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not supported!\n");
		}
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	hiace_param = &(hisifd->effect_info.hiace);


	set_reg(hiace_base + DPE_BYPASS_ACE, 0x1, 1, 0);
	set_reg(hiace_base + DPE_INIT_GAMMA, 0x1, 1, 0);
	set_reg(hiace_base + DPE_UPDATE_LOCAL, 0x1, 1, 0);
	set_reg(hiace_base + DPE_UPDATE_FNA, 0x1, 1, 0);

	/* parameters */
	width = hisifd->panel_info.xres & 0x1fff;
	height = hisifd->panel_info.yres & 0x1fff;
	set_reg(hiace_base + DPE_IMAGE_INFO, (height << 16) | width, 32, 0);

	hiace_param->image_info = (height << 16) | width;

	pipe_mode = 0;
	partition_mode = 0;
	is_left_pipe = 0;
	set_reg(hiace_base + DPE_DB_PIPE_CFG, (is_left_pipe << 31) | (partition_mode << 30) | pipe_mode, 32, 0);
	if (partition_mode == 0) {
		xPartition = 6;
	} else {
		xPartition = 4;
	}

	hiace_param->db_pipe_cfg = (is_left_pipe << 31) | (partition_mode << 30) | pipe_mode;

	lhist_quant = 0;
	set_reg(hiace_base + DPE_LHIST_EN, lhist_quant, 1, 10);

	hiace_param->lhist_en = lhist_quant<<10 | hiace_param->lhist_en;

	half_block_w = (width / (2 * xPartition)) & 0x1ff;
	half_block_h = ((height + 11) / 12) & 0x1ff;
	set_reg(hiace_base + DPE_HALF_BLOCK_INFO,
		(half_block_h << 16) | half_block_w, 32, 0);

	hiace_param->half_block_info = (half_block_h << 16) | half_block_w;

	block_pixel_num = (half_block_w * half_block_h) << 2;
	max_lhist_block_pixel_num = block_pixel_num << 2;
	max_lhist_bin_reg_num = (1 << 16) - 1; /* each local hist bin 20bit -> 16bit */
	if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 1)) {
		lhist_sft = 0;
	} else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 2)) {
		lhist_sft = 1;
	} else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 3)) {
		lhist_sft = 2;
	} else if (max_lhist_block_pixel_num < (max_lhist_bin_reg_num << 4)) {
		lhist_sft = 3;
	} else {
		lhist_sft = 4;
	}
	set_reg(hiace_base + DPE_LHIST_SFT, lhist_sft, 3, 0);
	pinfo->hiace_param.ilhist_sft = (int)lhist_sft;
	hiace_param->lhist_sft = lhist_sft;

	slop = 68 & 0xff;
	th_min = 0 & 0x1ff;
	th_max = 30 & 0x1ff;
	set_reg(hiace_base + DPE_HUE, (slop << 24) | (th_max << 12) | th_min, 32, 0);

	hiace_param->hue = (slop << 24) | (th_max << 12) | th_min;

	slop = 136 & 0xff;
	th_min = 50 & 0xff;
	th_max = 174 & 0xff;
	set_reg(hiace_base + DPE_SATURATION, (slop << 24) | (th_max << 12) | th_min, 32, 0);
	hiace_param->saturation = (slop << 24) | (th_max << 12) | th_min;

	slop = 136 & 0xff;
	th_min = 40 & 0xff;
	th_max = 255 & 0xff;
	set_reg(hiace_base + DPE_VALUE, (slop << 24) | (th_max << 12) | th_min, 32, 0);
	hiace_param->value = (slop << 24) | (th_max << 12) | th_min;

	set_reg(hiace_base + DPE_SKIN_GAIN, 128, 8, 0);
	hiace_param->skin_gain = 128;

	up_thres = 248 & 0xff;
	low_thres = 8 & 0xff;
	set_reg(hiace_base + DPE_UP_LOW_TH, (up_thres << 8) | low_thres, 32, 0);
	hiace_param->up_low_th = (up_thres << 8) | low_thres;

	fixbit_x = get_fixed_point_offset(half_block_w) & 0x1f;
	fixbit_y = get_fixed_point_offset(half_block_h) & 0x1f;
	reciprocal_x = (1U << (fixbit_x + 8)) / (2 * MAX(half_block_w, 1)) & 0x3ff;
	reciprocal_y = (1U << (fixbit_y + 8)) / (2 * MAX(half_block_h, 1)) & 0x3ff;
	set_reg(hiace_base + DPE_XYWEIGHT, (fixbit_y << 26) | (reciprocal_y << 16)
		| (fixbit_x << 10) | reciprocal_x, 32, 0);

	hiace_param->xyweight = (fixbit_y << 26) | (reciprocal_y << 16) | (fixbit_x << 10) | reciprocal_x;

	if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
		DEBUG_EFFECT_LOG("[effect] half_block_w:%d, half_block_h:%d, fixbit_x:%d, fixbit_y:%d, reciprocal_x:%d, reciprocal_y:%d, lhist_sft:%d\n",
						 half_block_w, half_block_h, fixbit_x, fixbit_y, reciprocal_x, reciprocal_y, lhist_sft);
	}

	/* wait for gamma init finishing */
	dw_jiffies = jiffies + HZ / 2;
	do {
		tmp = inp32(hiace_base + DPE_INIT_GAMMA);
		if ((tmp & 0x1) != 0x1) {
			is_ready = true;
			break;
		}
	} while (time_after(dw_jiffies, jiffies)); //lint !e550

	if (!is_ready) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] fb%d, HIACE_INIT_GAMMA is not ready! HIACE_INIT_GAMMA=0x%08X.\n",
							 hisifd->index, tmp);
		} else {
			HISI_FB_INFO("[effect] fb%d, HIACE_INIT_GAMMA is not ready! HIACE_INIT_GAMMA=0x%08X.\n",
						 hisifd->index, tmp);
		}
	}

	global_hist_ab_work = inp32(hiace_base + DPE_GLOBAL_HIST_AB_WORK);
	global_hist_ab_shadow = !global_hist_ab_work;

	gamma_ab_work = inp32(hiace_base + DPE_GAMMA_AB_WORK);
	gamma_ab_shadow = !gamma_ab_work;

	set_reg(hiace_base + DPE_GLOBAL_HIST_AB_SHADOW, global_hist_ab_shadow, 1, 0);
	set_reg(hiace_base + DPE_GAMMA_AB_SHADOW, gamma_ab_shadow, 1, 0);

	/* clear hiace interrupt */
	outp32(hiace_base + DPE_INT_STAT, 0x1);

	/* unmask hiace interrupt */
	set_reg(hiace_base + DPE_INT_UNMASK, 0x1, 1, 0);

	/*enable hiace */
	set_reg(hiace_base + DPE_BYPASS_ACE, 0x1, 1, 0);

	hiace_param->roi_start_point = 0x0;
	hiace_param->roi_width_high = (0x0780<<16) | 0x0438;
	hiace_param->roi_mode_ctrl = 0x0;
	hiace_param->roi_hist_stat_mode = 0x0;
	hiace_param->rgb_blend_weight = (0x05<<16) | (0x05<<8) | 0x05;
	hiace_param->fna_statistic = (0x0f<<8) | 0x03;
	hiace_param->gamma_w = 0;
	hiace_param->gamma_r = 0x0;
	hiace_param->fna_addr = 0x0;
	hiace_param->update_fna = 0x0;
	hiace_param->db_pipe_ext_width = 0x0;
	hiace_param->db_pipe_full_img_width = 0x0438;

	init_noisereduction(hisifd);
	hiace_param->enable = 0;

}

void init_noisereduction(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *nr_base = NULL;
	struct hiace_info *hiace_param = NULL;
	uint32_t somebrightness0;
	uint32_t somebrightness1;
	uint32_t somebrightness2;
	uint32_t somebrightness3;
	uint32_t somebrightness4;
	uint32_t minSigma;
	uint32_t maxSigma;
	uint32_t colorSigma0;
	uint32_t colorSigma1;
	uint32_t colorSigma2;
	uint32_t colorSigma3;
	uint32_t colorSigma4;
	uint32_t colorSigma5;

	uint32_t slop;
	uint32_t th_max;
	uint32_t th_min;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		nr_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] NR is not supported!\n");
		}
		return;
	}
	hiace_param = &(hisifd->effect_info.hiace);

	/* disable noisereduction */
	set_reg(nr_base + DPE_BYPASS_NR, 0x1, 1, 0);

	somebrightness0 = 0x398 & 0x3ff;
	somebrightness1 = 0x280 & 0x3ff;
	somebrightness2 = 0x180 & 0x3ff;
	somebrightness3 = 0x0cd & 0x3ff;
	somebrightness4 = 0x060 & 0x3ff;
	set_reg(nr_base + DPE_S3_SOME_BRIGHTNESS01, somebrightness0 | (somebrightness1 << 16), 32, 0);
	set_reg(nr_base + DPE_S3_SOME_BRIGHTNESS23, somebrightness2 | (somebrightness3 << 16), 32, 0);
	set_reg(nr_base + DPE_S3_SOME_BRIGHTNESS4, somebrightness4, 32, 0);

	hiace_param->s3_some_brightness01 = somebrightness0 | (somebrightness1 << 16);
	hiace_param->s3_some_brightness23 = somebrightness2 | (somebrightness3 << 16);
	hiace_param->s3_some_brightness4 = somebrightness4;

	minSigma = 0x1a & 0x1f;
	maxSigma = 0x15 & 0x1f;
	set_reg(nr_base + DPE_S3_MIN_MAX_SIGMA, minSigma | (maxSigma << 16), 32, 0);
	hiace_param->s3_min_max_sigma = minSigma | (maxSigma << 16);

	colorSigma0 = 0x17 & 0x1f;
	colorSigma1 = 0x17 & 0x1f;
	colorSigma2 = 0x18 & 0x1f;
	colorSigma3 = 0x19 & 0x1f;
	colorSigma4 = 0x1a & 0x1f;
	colorSigma5 = 0x1a & 0x1f;
	set_reg(nr_base + DPE_S3_GREEN_SIGMA03, colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24), 32, 0);
	set_reg(nr_base + DPE_S3_GREEN_SIGMA45, colorSigma4 | (colorSigma5 << 8), 32, 0);

	hiace_param->s3_green_sigma03 = colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24);
	hiace_param->s3_blue_sigma45 = colorSigma4 | (colorSigma5 << 8);

	colorSigma0 = 0x16 & 0x1f;
	colorSigma1 = 0x16 & 0x1f;
	colorSigma2 = 0x17 & 0x1f;
	colorSigma3 = 0x17 & 0x1f;
	colorSigma4 = 0x18 & 0x1f;
	colorSigma5 = 0x18 & 0x1f;
	set_reg(nr_base + DPE_S3_RED_SIGMA03, colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24), 32, 0);
	set_reg(nr_base + DPE_S3_RED_SIGMA45, colorSigma4 | (colorSigma5 << 8), 32, 0);

	hiace_param->s3_red_sigma03 = colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24);
	hiace_param->s3_red_sigma45 = colorSigma4 | (colorSigma5 << 8);

	colorSigma0 = 0x15 & 0x1f;
	colorSigma1 = 0x15 & 0x1f;
	colorSigma2 = 0x16 & 0x1f;
	colorSigma3 = 0x16 & 0x1f;
	colorSigma4 = 0x17 & 0x1f;
	colorSigma5 = 0x17 & 0x1f;
	set_reg(nr_base + DPE_S3_BLUE_SIGMA03, colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24), 32, 0);
	set_reg(nr_base + DPE_S3_BLUE_SIGMA45, colorSigma4 | (colorSigma5 << 8), 32, 0);

	hiace_param->s3_blue_sigma03 = colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24);
	hiace_param->s3_blue_sigma45 = colorSigma4 | (colorSigma5 << 8);

	colorSigma0 = 0x15 & 0x1f;
	colorSigma1 = 0x15 & 0x1f;
	colorSigma2 = 0x16 & 0x1f;
	colorSigma3 = 0x17 & 0x1f;
	colorSigma4 = 0x18 & 0x1f;
	colorSigma5 = 0x19 & 0x1f;
	set_reg(nr_base + DPE_S3_WHITE_SIGMA03, colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24), 32, 0);
	set_reg(nr_base + DPE_S3_WHITE_SIGMA45, colorSigma4 | (colorSigma5 << 8), 32, 0);

	set_reg(nr_base + DPE_S3_FILTER_LEVEL, 0x7, 5, 0);

	set_reg(nr_base + DPE_S3_SIMILARITY_COEFF, 0x34d, 10, 0);

	set_reg(nr_base + DPE_S3_V_FILTER_WEIGHT_ADJ, 0x1, 2, 0);

	hiace_param->s3_white_sigma03 = colorSigma0 | (colorSigma1 << 8) | (colorSigma2 << 16) | (colorSigma3 << 24);
	hiace_param->s3_white_sigma45 = colorSigma4 | (colorSigma5 << 8);
	hiace_param->s3_filter_level = 0x7;
	hiace_param->s3_similarity_coeff = 0x34d;
	hiace_param->s3_v_filter_weight_adj = 0x1;

	slop = 0x44 & 0xff;
	th_min = 0x0 & 0x1ff;
	th_max = 0x1e & 0x1ff;
	set_reg(nr_base + DPE_S3_HUE, (slop << 24) | (th_max << 12) | th_min, 32, 0);
	hiace_param->s3_hue = (slop << 24) | (th_max << 12) | th_min;

	slop = 0x88 & 0xff;
	th_min = 0x32 & 0xff;
	th_max = 0xae & 0xff;
	set_reg(nr_base + DPE_S3_SATURATION, (slop << 24) | (th_max << 12) | th_min, 32, 0);
	hiace_param->s3_saturation = (slop << 24) | (th_max << 12) | th_min;

	slop = 0x88 & 0xff;
	th_min = 0x28 & 0xff;
	th_max = 0xff & 0xff;
	set_reg(nr_base + DPE_S3_VALUE, (slop << 24) | (th_max << 12) | th_min, 32, 0);
	hiace_param->s3_value = (slop << 24) | (th_max << 12) | th_min;

	set_reg(nr_base + DPE_S3_SKIN_GAIN, 0x80, 8, 0);
	hiace_param->s3_skin_gain = 0x80;

	/* disable noisereduction */
	set_reg(nr_base + DPE_BYPASS_NR, 0x1, 1, 0);
	hiace_param->bypass_nr = 0x1;
}

void hisi_dss_dpp_hiace_set_reg(struct hisi_fb_data_type *hisifd)
{
	char __iomem *hiace_base = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	dss_ce_info_t *ce_info = NULL;
	int xPartition = 6;
	int j = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	if (NULL == hisifd->dss_base) {
		HISI_FB_ERR("[effect] dss_base is NULL\n");
		return;
	}

	if (hisifd->panel_info.hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_FRAME) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not support!\n");
		}
		return;
	}

	if (PRIMARY_PANEL_IDX == hisifd->index) {
		ce_ctrl = &(hisifd->ce_ctrl);
		ce_info = &(hisifd->hiace_info);
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!\n", hisifd->index);
		return;
	}

	if (!(g_enable_effect & ENABLE_EFFECT_HIACE) || ce_ctrl->ctrl_ce_mode == CE_MODE_DISABLE) {
		if (g_debug_effect & DEBUG_EFFECT_FRAME) {
			DEBUG_EFFECT_LOG("[effect] g_enable_effect is %d, ctrl_ce_mode is %d.\n",
						g_enable_effect, ce_ctrl->ctrl_ce_mode);
		}
		//ce_info->algorithm_result = 1;
		return;
	}

	/*if (g_debug_effect & DEBUG_EFFECT_FRAME) {
		DEBUG_EFFECT_LOG("[effect] step in\n");
	}*/

	//lint -e{438}
	if (ce_info->algorithm_result == 0 && hisifd->hiace_info.lut_table != NULL) {
		int gamma_ab_shadow = inp32(hiace_base + DPE_GAMMA_AB_SHADOW);
		int gamma_ab_work = inp32(hiace_base + DPE_GAMMA_AB_WORK);
		time_interval_t interval_lut = {0};
		static delay_record_t delay_lut = {"lut writing", 0, 0xFFFFFFFF, 0, 0};

		if (gamma_ab_shadow == gamma_ab_work) {
			int i = 0;

			/* write gamma lut */
			//HISI_FB_DEBUG("[effect] write gamma lut!\n");
			set_reg(hiace_base + DPE_GAMMA_EN, 1, 1, 31);

			if (g_debug_effect & DEBUG_EFFECT_DELAY) {
				interval_lut.start = get_timestamp_in_us();
			}

			mutex_lock(&ce_info->lut_lock);
			for (i = 0; i < (6 * xPartition * 8); i++) {
				j = i % 8;
				//lut_value = hiace_lut[4*j] | ((hiace_lut[4*j+1] - hiace_lut[4*j]) << 10)	|
				//	((hiace_lut[4*j+2] - hiace_lut[4*j+1]) << 17)  | ((hiace_lut[4*j+3] - hiace_lut[4*j+2]) << 24) ;
				//outp32(hiace_base + DPE_GAMMA_VxHy_3z2_3z1_3z_W, lut_value);
				outp32(hiace_base + DPE_GAMMA_VxHy_3z2_3z1_3z_W, hisifd->hiace_info.lut_table[i]);
			}
			mutex_unlock(&ce_info->lut_lock);
			if (g_debug_effect & DEBUG_EFFECT_DELAY) {
				interval_lut.stop = get_timestamp_in_us();
				count_delay(&delay_lut, interval_lut.stop - interval_lut.start);
			}

			set_reg(hiace_base + DPE_GAMMA_EN, 0, 1, 31);

			gamma_ab_shadow ^= 1;
			outp32(hiace_base + DPE_GAMMA_AB_SHADOW, gamma_ab_shadow);
		}

		ce_info->algorithm_result = 1;
	}
}

void hisi_dpp_hiace_end_handle_func(struct work_struct *work)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *hiace_base = NULL;
	uint32_t * global_hist_ptr = NULL;
	uint32_t * sat_global_hist_ptr = NULL;
	uint32_t * local_hist_ptr = NULL;
	uint32_t * fna_data_ptr = NULL;
	dss_ce_info_t *ce_info = NULL;

	int i = 0;
	int xPartition = 6;
	int lhist_band = 8;
	int lhist_en = 0;
	int lhist_quant = 0;
	int sum_sat = 0;
	int global_hist_ab_shadow = 0;
	int global_hist_ab_work = 0;
	int local_valid =0 ;
	int fna_valid =0 ;

	time_interval_t interval_total = {0};
	time_interval_t interval_hist_global = {0};
	time_interval_t interval_sat_hist_global = {0};
	time_interval_t interval_hist_local = {0};
	time_interval_t interval_fna_local = {0};

	static delay_record_t delay_total = {"interrupt handling", 0, 0xFFFFFFFF, 0, 0};
	static delay_record_t delay_hist_global = {"global hist reading", 0, 0xFFFFFFFF, 0, 0};
	static delay_record_t delay_sat_hist_global = {"sat_global hist reading", 0, 0xFFFFFFFF, 0, 0};
	static delay_record_t delay_hist_local = {"local hist reading", 0, 0xFFFFFFFF, 0, 0};
	static delay_record_t delay_fna_local = {"fna_local hist reading", 0, 0xFFFFFFFF, 0, 0};

	if (NULL == work) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	hisifd = container_of(work, struct hisi_fb_data_type, hiace_end_work);
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		if (g_debug_effect & DEBUG_EFFECT_FRAME) {
			DEBUG_EFFECT_LOG("[effect] HIACE is not support!\n");
		}
		return;
	}

	if (PRIMARY_PANEL_IDX == hisifd->index) {
		ce_info = &(hisifd->hiace_info);
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!\n", hisifd->index);
		return;
	}

	down(&hisifd->blank_sem);
	if (!hisifd->panel_power_on) {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] panel power off!\n");
		}
		up(&hisifd->blank_sem);
		return;
	}

	if (!(g_enable_effect & ENABLE_EFFECT_HIACE) || hisifd->ce_ctrl.ctrl_ce_mode == CE_MODE_DISABLE) {
		if (g_debug_effect & DEBUG_EFFECT_FRAME) {
			DEBUG_EFFECT_LOG("[effect] g_enable_effect is %d, ctrl_ce_mode is %d.\n",
						g_enable_effect, hisifd->ce_ctrl.ctrl_ce_mode);
		}
		goto ERR_OUT;
	}

	if (g_hiace_service.is_ready) {
		g_hiace_service.is_ready = false;
	} else {
		if (g_debug_effect & DEBUG_EFFECT_ENTRY) {
			DEBUG_EFFECT_LOG("[effect] service is not ready!\n");
		}
		goto ERR_OUT;
	}

	if (g_debug_effect & DEBUG_EFFECT_FRAME) {
		DEBUG_EFFECT_LOG("[effect] step in\n");
	}
	hisifb_ce_service_init();

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_total.start = get_timestamp_in_us();
	}

	hisifb_activate_vsync(hisifd);

	mutex_lock(&ce_info->hist_lock);

	sum_sat = (int)inp32(hiace_base + DPE_SUM_SATURATION);

	local_valid = inp32(hiace_base + DPE_LOCAL_VALID);

	lhist_en = inp32(hiace_base + DPE_LHIST_EN);

	lhist_quant = (lhist_en>>10) & 0x1;

	if (lhist_quant == 0) {
		lhist_band = 16;
	} else {
		lhist_band = 8;
	}

	if (local_valid == 1) {
		/* read local hist */
		local_hist_ptr = &hisifd->hiace_info.histogram[HIACE_GHIST_RANK * 2];
		set_reg(hiace_base + DPE_LHIST_EN, 1, 1, 31);

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_local.start = get_timestamp_in_us();
		}
		for (i = 0; i < (6 * xPartition * lhist_band); i++) {/* H  L */
			local_hist_ptr[i] = inp32(hiace_base + DPE_LOCAL_HIST_VxHy_2z_2z1);
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_local.stop = get_timestamp_in_us();
			count_delay(&delay_hist_local, interval_hist_local.stop - interval_hist_local.start);
		}

		set_reg(hiace_base + DPE_LHIST_EN, 0, 1, 31);
		outp32(hiace_base + DPE_UPDATE_LOCAL, 1);
	}

	fna_valid = inp32(hiace_base + DPE_FNA_VALID);
	if (fna_valid == 1) {
		/* read fna data */
		// cppcheck-suppress *
		fna_data_ptr = &hisifd->hiace_info.histogram[HIACE_GHIST_RANK * 2 + YBLOCKNUM * XBLOCKNUM * HIACE_LHIST_RANK];
		set_reg(hiace_base + DPE_FNA_EN, 1, 1, 31);

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_fna_local.start = get_timestamp_in_us();
		}
		for (i = 0; i < (6 * xPartition); i++) {/* FNA */
			fna_data_ptr[i] = inp32(hiace_base + DPE_FNA_VxHy);
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_fna_local.stop = get_timestamp_in_us();
			count_delay(&delay_fna_local, interval_fna_local.stop - interval_fna_local.start);
		}

		set_reg(hiace_base + DPE_FNA_EN, 0, 1, 31);
		outp32(hiace_base + DPE_UPDATE_FNA, 1);
	}

	global_hist_ab_shadow = inp32(hiace_base + DPE_GLOBAL_HIST_AB_SHADOW);
	global_hist_ab_work = inp32(hiace_base + DPE_GLOBAL_HIST_AB_WORK);
	if (global_hist_ab_shadow == global_hist_ab_work) {
		/* read global hist */
		global_hist_ptr = &hisifd->hiace_info.histogram[0];/* HIACE_GHIST_RANK */

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_global.start = get_timestamp_in_us();
		}
		for (i = 0; i < 32; i++) {
			global_hist_ptr[i] = inp32(hiace_base + DPE_GLOBAL_HIST_LUT_ADDR + i * 4);
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_hist_global.stop = get_timestamp_in_us();
			count_delay(&delay_hist_global, interval_hist_global.stop - interval_hist_global.start);
		}

		/* read sat_global hist */
		sat_global_hist_ptr = &hisifd->hiace_info.histogram[HIACE_GHIST_RANK];/* HIACE_GHIST_RANK */

		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_sat_hist_global.start = get_timestamp_in_us();
		}
		for (i = 0; i < 32; i++) {
			sat_global_hist_ptr[i] = inp32(hiace_base + DPE_SAT_GLOBAL_HIST_LUT_ADDR + i * 4);
		}
		if (g_debug_effect & DEBUG_EFFECT_DELAY) {
			interval_sat_hist_global.stop = get_timestamp_in_us();
			count_delay(&delay_sat_hist_global, interval_sat_hist_global.stop - interval_sat_hist_global.start);
		}

		outp32(hiace_base +  DPE_GLOBAL_HIST_AB_SHADOW, global_hist_ab_shadow ^ 1);
	}

	hisifd->hiace_info.histogram[CE_SIZE_HIST - 1] = sum_sat;
	mutex_unlock(&ce_info->hist_lock);

	hisifb_deactivate_vsync(hisifd);
	if ((local_valid == 1) ||(fna_valid == 1) || (global_hist_ab_shadow == global_hist_ab_work)) { /* global or local hist or fna is updated */
		hisifb_ce_do_contrast(hisifd);
	}

	g_hiace_service.is_ready = true;

	if (g_debug_effect & DEBUG_EFFECT_DELAY) {
		interval_total.stop = get_timestamp_in_us();
		count_delay(&delay_total, interval_total.stop - interval_total.start);
	}

ERR_OUT:
	/* clear INT */
	if (hisifd->panel_power_on) {
		hisifb_activate_vsync(hisifd);
		outp32(hiace_base + DPE_INT_STAT, 0x1);
		hisifb_deactivate_vsync(hisifd);
	}

	up(&hisifd->blank_sem);
} //lint !e550
//lint +e845, +e732, +e774

////////////////////////////////////////////////////////////////////////////////

void init_acm_ce(struct hisi_fb_data_type *hisifd)
{
	(void)hisifd;
}

void hisi_dss_dpp_ace_set_reg(struct hisi_fb_data_type *hisifd)
{
	(void)hisifd;
}

void hisi_dpp_ace_end_handle_func(struct work_struct *work)
{
	(void)work;
}

/*******************************************************************************
** GM IGM
*/
#define GM_LUT_LEN 257
#define GM_LUT_MHLEN 254
static uint16_t degm_gm_lut[GM_LUT_LEN *6];

int hisifb_use_dynamic_gamma(struct hisi_fb_data_type *hisifd, char __iomem *dpp_base)
{
	uint32_t i = 0;
	uint32_t index = 0;
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *gamma_pre_lut_base = NULL;//lint !e838

	if (hisifd == NULL) {
		return -1;
	}

	if (dpp_base == NULL) {
		return -1;
	}

	pinfo = &(hisifd->panel_info);

	if(hisifd->dynamic_gamma_info.use_dynamic_gm_init == 1) {
		uint16_t* gm_lut_r =  degm_gm_lut;
		uint16_t* gm_lut_g =  gm_lut_r + GM_LUT_LEN;
		uint16_t* gm_lut_b =  gm_lut_g + GM_LUT_LEN;

		for (i = 0; i < pinfo->gamma_lut_table_len / 2; i++) {
			index = i << 1;
			if (index >= GM_LUT_MHLEN)
			{
				index = GM_LUT_MHLEN;
			}
			outp32(dpp_base + (U_GAMA_R_COEF + i * 4), gm_lut_r[index] | gm_lut_r[index+1] << 16);
			outp32(dpp_base + (U_GAMA_G_COEF + i * 4), gm_lut_g[index] | gm_lut_g[index+1] << 16);
			outp32(dpp_base + (U_GAMA_B_COEF + i * 4), gm_lut_b[index] | gm_lut_b[index+1] << 16);

			if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
				gamma_pre_lut_base = hisifd->dss_base + DSS_DPP_GAMA_PRE_LUT_OFFSET;
				//GAMA PRE LUT
				outp32(gamma_pre_lut_base + (U_GAMA_PRE_R_COEF + i * 4), gm_lut_r[index] | gm_lut_r[index+1] << 16);
				outp32(gamma_pre_lut_base + (U_GAMA_PRE_G_COEF + i * 4), gm_lut_g[index] | gm_lut_g[index+1] << 16);
				outp32(gamma_pre_lut_base + (U_GAMA_PRE_B_COEF + i * 4), gm_lut_b[index] | gm_lut_b[index+1] << 16);
			}
		}
		if (pinfo->gamma_lut_table_len <= GM_LUT_LEN) {
			outp32(dpp_base + U_GAMA_R_LAST_COEF, gm_lut_r[pinfo->gamma_lut_table_len - 1]);
			outp32(dpp_base + U_GAMA_G_LAST_COEF, gm_lut_g[pinfo->gamma_lut_table_len - 1]);
			outp32(dpp_base + U_GAMA_B_LAST_COEF, gm_lut_b[pinfo->gamma_lut_table_len - 1]);
		}

		if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
			//GAMA PRE LUT
			outp32(gamma_pre_lut_base + U_GAMA_PRE_R_LAST_COEF, gm_lut_r[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_pre_lut_base + U_GAMA_PRE_G_LAST_COEF, gm_lut_g[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_pre_lut_base + U_GAMA_PRE_B_LAST_COEF, gm_lut_b[pinfo->gamma_lut_table_len - 1]);
		}
		return 1;//lint !e438
	}

	return 0;//lint !e438

}//lint !e550

int hisifb_use_dynamic_degamma(struct hisi_fb_data_type *hisifd, char __iomem *dpp_base)
{
	uint32_t i = 0;
	uint32_t index = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		return -1;
	}

	if (dpp_base == NULL) {
		return -1;
	}

	pinfo = &(hisifd->panel_info);

	if(hisifd->dynamic_gamma_info.use_dynamic_gm_init == 1) {

		uint16_t* degm_lut_r = degm_gm_lut + GM_LUT_LEN * 3;
		uint16_t* degm_lut_g = degm_lut_r + GM_LUT_LEN;
		uint16_t* degm_lut_b = degm_lut_g + GM_LUT_LEN;

		for (i = 0; i < pinfo->igm_lut_table_len / 2; i++) {
			index = i << 1;
			if(index >= GM_LUT_MHLEN)
			{
				index = GM_LUT_MHLEN;
			}
			outp32(dpp_base + (U_DEGAMA_R_COEF +  i * 4), degm_lut_r[index] | degm_lut_r[index+1] << 16);
			outp32(dpp_base + (U_DEGAMA_G_COEF +  i * 4), degm_lut_g[index] | degm_lut_g[index+1] << 16);
			outp32(dpp_base + (U_DEGAMA_B_COEF +  i * 4), degm_lut_b[index] | degm_lut_b[index+1] << 16);
		}
		if (pinfo->igm_lut_table_len <= GM_LUT_LEN) {
			outp32(dpp_base + U_DEGAMA_R_LAST_COEF, degm_lut_r[pinfo->igm_lut_table_len - 1]);
			outp32(dpp_base + U_DEGAMA_G_LAST_COEF, degm_lut_g[pinfo->igm_lut_table_len - 1]);
			outp32(dpp_base + U_DEGAMA_B_LAST_COEF, degm_lut_b[pinfo->igm_lut_table_len - 1]);
		}

		return 1;
	}

	return 0;

}

void hisifb_update_dynamic_gamma(struct hisi_fb_data_type *hisifd, const char* buffer, size_t len)
{
	struct hisi_panel_info *pinfo = NULL;
	if (hisifd == NULL || buffer == NULL) {
		return;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo == NULL) {
		return;
	}

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_GAMA)){
		return;
	}

	hisifd->dynamic_gamma_info.use_dynamic_gm = 0;
	hisifd->dynamic_gamma_info.use_dynamic_gm_init = 0;

	if (pinfo->gamma_support == 1) {
		if ((len > 0) && (len <= (int)sizeof(degm_gm_lut))) {
			memcpy((char*)degm_gm_lut, buffer, (size_t)len);
			hisifd->dynamic_gamma_info.use_dynamic_gm = 1;
			hisifd->dynamic_gamma_info.use_dynamic_gm_init = 1;
		}
	}

}

void hisifb_update_gm_from_reserved_mem(uint32_t *gm_r, uint32_t *gm_g, uint32_t *gm_b,
	uint32_t *igm_r, uint32_t *igm_g, uint32_t *igm_b)
{
	int i = 0;
	int len = 0;
	uint16_t *u16_gm_r = NULL;
	uint16_t *u16_gm_g = NULL;
	uint16_t *u16_gm_b = NULL;
	uint16_t *u16_igm_r = NULL;
	uint16_t *u16_igm_g = NULL;
	uint16_t *u16_igm_b = NULL;
	void *mem = NULL;
	unsigned long gm_addr = 0;
	unsigned long gm_size = 0;

	g_factory_gamma_enable = 0;

	if (gm_r == NULL || gm_g == NULL || gm_b == NULL
		|| igm_r == NULL || igm_g == NULL || igm_b == NULL) {
		return;
	}

	gm_addr = HISI_SUB_RESERVED_LCD_GAMMA_MEM_PHYMEM_BASE;
	gm_size = HISI_SUB_RESERVED_LCD_GAMMA_MEM_PHYMEM_SIZE;

	HISI_FB_INFO("gamma kernel gm_addr = 0x%x  gm_size = 0x%x \n", gm_addr, gm_size);

	mem = (void *)ioremap_wc(gm_addr, gm_size);
	if (mem == NULL) {
		HISI_FB_ERR("mem ioremap error ! \n");
		return;
	}
	memcpy(&len, mem, 4UL);
	HISI_FB_INFO("gamma read len = %d \n", len);
	if (len != GM_IGM_LEN) {
		HISI_FB_INFO("gamma read len error ! \n");
		iounmap(mem);
		return;
	}

	u16_gm_r = (uint16_t *)(mem + 4);
	u16_gm_g = u16_gm_r + GM_LUT_LEN;
	u16_gm_b = u16_gm_g + GM_LUT_LEN;

	u16_igm_r = u16_gm_b + GM_LUT_LEN;
	u16_igm_g = u16_igm_r + GM_LUT_LEN;
	u16_igm_b = u16_igm_g + GM_LUT_LEN;

	for (i = 0; i < GM_LUT_LEN; i++) {
		gm_r[i] = u16_gm_r[i];
		gm_g[i] = u16_gm_g[i];
		gm_b[i] = u16_gm_b[i];

		igm_r[i]  = u16_igm_r[i];
		igm_g[i] = u16_igm_g[i];
		igm_b[i] = u16_igm_b[i];
	}
	iounmap(mem);

	g_factory_gamma_enable = 1;
	return;
}

/*lint -e571, -e573, -e737, -e732, -e850, -e730, -e713, -e529, -e574, -e679, -e732, -e845, -e570, -e774*/

static void free_acm_table(struct acm_info *acm)
{
	hisi_effect_kfree(&acm->hue_table);
	hisi_effect_kfree(&acm->sata_table);
	hisi_effect_kfree(&acm->satr0_table);
	hisi_effect_kfree(&acm->satr1_table);
	hisi_effect_kfree(&acm->satr2_table);
	hisi_effect_kfree(&acm->satr3_table);
	hisi_effect_kfree(&acm->satr4_table);
	hisi_effect_kfree(&acm->satr5_table);
	hisi_effect_kfree(&acm->satr6_table);
	hisi_effect_kfree(&acm->satr7_table);
}

static void free_gamma_table(struct gamma_info *gamma)
{
	hisi_effect_kfree(&gamma->gamma_r_table);
	hisi_effect_kfree(&gamma->gamma_g_table);
	hisi_effect_kfree(&gamma->gamma_b_table);
}

int hisi_effect_arsr2p_info_get(struct hisi_fb_data_type *hisifd, struct arsr2p_info *arsr2p)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == arsr2p) {
		HISI_FB_ERR("fb%d, arsr2p is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.arsr2p_sharp_support) {
		HISI_FB_INFO("fb%d, arsr2p is not supported!\n", hisifd->index);
		return 0;
	}

	memcpy(&arsr2p[0], &(hisifd->dss_module_default.arsr2p[DSS_RCHN_V0].arsr2p_effect), sizeof(struct arsr2p_info));
	memcpy(&arsr2p[1], &(hisifd->dss_module_default.arsr2p[DSS_RCHN_V0].arsr2p_effect_scale_up), sizeof(struct arsr2p_info));
	memcpy(&arsr2p[2], &(hisifd->dss_module_default.arsr2p[DSS_RCHN_V0].arsr2p_effect_scale_down), sizeof(struct arsr2p_info));
	arsr2p[0].sharp_enable = hisifd->panel_info.prefix_sharpness2D_support;
	arsr2p[0].skin_enable  = arsr2p[0].skin_enable;
	arsr2p[0].shoot_enable = arsr2p[0].shoot_enable;
	arsr2p[1].sharp_enable = hisifd->panel_info.prefix_sharpness2D_support;
	arsr2p[1].skin_enable  = arsr2p[1].skin_enable;
	arsr2p[1].shoot_enable = arsr2p[1].shoot_enable;
	arsr2p[2].sharp_enable = hisifd->panel_info.prefix_sharpness2D_support;
	arsr2p[2].skin_enable  = arsr2p[2].skin_enable;
	arsr2p[2].shoot_enable = arsr2p[2].shoot_enable;

	return 0;
}

static void arsr1p_rog_init(struct arsr1p_info *arsr1p_rog)
{
	if (NULL == arsr1p_rog) {
		return;
	}

	arsr1p_rog->skin_thres_y = 0x534b;
	arsr1p_rog->skin_thres_u = 0x0a05;
	arsr1p_rog->skin_thres_v = 0x0c06;
	arsr1p_rog->skin_expected = 0x917196;
	arsr1p_rog->skin_cfg   = 0x30a06;
	arsr1p_rog->shoot_cfg1 = 0x2000014;
	arsr1p_rog->shoot_cfg2 = 0x7C0;
	arsr1p_rog->shoot_cfg3 = 0x200;
	arsr1p_rog->sharp_cfg1_h = 0x400030;
	arsr1p_rog->sharp_cfg1_l = 0x70003;
	arsr1p_rog->sharp_cfg2_h = 0x280000;
	arsr1p_rog->sharp_cfg2_l = 0x90005;
	arsr1p_rog->sharp_cfg3 = 0x640000;
	arsr1p_rog->sharp_cfg4 = 0x0;
	arsr1p_rog->sharp_cfg5 = 0x8c0000;
	arsr1p_rog->sharp_cfg6 = 0x1006;
	arsr1p_rog->sharp_cfg6_cut = 0x240014;
	arsr1p_rog->sharp_cfg7 = 0x20002;
	arsr1p_rog->sharp_cfg7_ratio = 0x500010;
	arsr1p_rog->sharp_cfg8 = 0xc00280;
	arsr1p_rog->sharp_cfg9 = 0x402300;
	arsr1p_rog->sharp_cfg10 = 0;
	arsr1p_rog->sharp_cfg11 = 0x5000000;
	arsr1p_rog->diff_ctrl = 0x140f;
	arsr1p_rog->skin_slop_y = 0x200;
	arsr1p_rog->skin_slop_u = 0x333;
	arsr1p_rog->skin_slop_v = 0x2aa;
}
int hisi_effect_arsr1p_info_get(struct hisi_fb_data_type *hisifd, struct arsr1p_info *arsr1p)
{
	struct arsr1p_info *arsr1p_param = NULL;
	struct arsr1p_info *arsr1p_rog = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == arsr1p) {
		HISI_FB_ERR("fb%d, arsr1p is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.arsr1p_sharp_support) {
		HISI_FB_INFO("fb%d, arsr1p sharp is not supported!\n", hisifd->index);
		return 0;
	}

	arsr1p_param = &(hisifd->effect_info.arsr1p[0]);

	arsr1p_param->sharp_enable = hisifd->panel_info.arsr1p_sharpness_support;
	arsr1p_param->skin_enable = arsr1p_param->sharp_enable;
	arsr1p_param->shoot_enable = arsr1p_param->skin_enable;

	arsr1p_param->skin_thres_y =  332<<10 | 300;
	arsr1p_param->skin_thres_u = 40<<10 | 20;
	arsr1p_param->skin_thres_v = 48<<10 | 24;
	arsr1p_param->skin_expected = 580<<20 | 452<<10 | 600;
	arsr1p_param->skin_cfg = 12<<16 | 10<<8 | 6;

	arsr1p_param->shoot_cfg1 = 8<<24 | 20;
	arsr1p_param->shoot_cfg2 = (-64 & 0x7ff);
	arsr1p_param->shoot_cfg3 = 512;

	arsr1p_param->sharp_cfg1_h = 256<<16 | 192;
	arsr1p_param->sharp_cfg1_l = 24<<16 | 8;
	arsr1p_param->sharp_cfg2_h = 256<<16 | 192;
	arsr1p_param->sharp_cfg2_l =  24<<16 | 8;
	arsr1p_param->sharp_cfg3 = 150<<16 | 150;
	arsr1p_param->sharp_cfg4 = 200<<16 | 0;
	arsr1p_param->sharp_cfg5 = 200<<16 | 0;
	arsr1p_param->sharp_cfg6 = 16<<16 | 6;
	arsr1p_param->sharp_cfg6_cut = 160<<16 | 96;
	arsr1p_param->sharp_cfg7 = 1<<17 | 4;
	arsr1p_param->sharp_cfg7_ratio = 160<<16 | 16;
	arsr1p_param->sharp_cfg8 = 3<<22 | 800;
	arsr1p_param->sharp_cfg9 = 8<<22 | 12800;
	arsr1p_param->sharp_cfg10 = 800;
	arsr1p_param->sharp_cfg11 = 20 << 22 | 12800;

	arsr1p_param->diff_ctrl = 20<<8 | 16;
	arsr1p_param->skin_slop_y = 512;
	arsr1p_param->skin_slop_u = 819;
	arsr1p_param->skin_slop_v = 682;

	arsr1p_rog = &(hisifd->effect_info.arsr1p[1]);
	arsr1p_rog->sharp_enable = hisifd->panel_info.arsr1p_sharpness_support;
	arsr1p_rog->skin_enable = arsr1p_rog->sharp_enable;
	arsr1p_rog->shoot_enable = arsr1p_rog->skin_enable;
	if (!hisifd->effect_info.arsr1p_rog_initialized) {
		arsr1p_rog_init(arsr1p_rog);
		hisifd->effect_info.arsr1p_rog_initialized = true;
	}
	memcpy(&(arsr1p[0]), arsr1p_param, sizeof(struct arsr1p_info));
	memcpy(&(arsr1p[1]), arsr1p_rog  , sizeof(struct arsr1p_info));

	return 0;
}

int hisi_effect_acm_info_get(struct hisi_fb_data_type *hisifd, struct acm_info *acm_dst)
{
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == acm_dst) {
		HISI_FB_ERR("fb%d, acm is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.acm_support) {
		HISI_FB_INFO("fb%d, acm is not supported!\n", hisifd->index);
		return 0;
	}

	pinfo = &hisifd->panel_info;

	acm_dst->acm_hue_rlh01 = (pinfo->r1_lh<<16) | pinfo->r0_lh;
	acm_dst->acm_hue_rlh23 = (pinfo->r3_lh<<16) | pinfo->r2_lh;
	acm_dst->acm_hue_rlh45 = (pinfo->r5_lh<<16) | pinfo->r4_lh;
	acm_dst->acm_hue_rlh67 = (pinfo->r6_hh<<16) | pinfo->r6_lh;
	acm_dst->acm_hue_param01 = pinfo->hue_param01;
	acm_dst->acm_hue_param23 = pinfo->hue_param23;
	acm_dst->acm_hue_param45 = pinfo->hue_param45;
	acm_dst->acm_hue_param67 = pinfo->hue_param67;
	acm_dst->acm_hue_smooth0 = pinfo->hue_smooth0;
	acm_dst->acm_hue_smooth1 = pinfo->hue_smooth1;
	acm_dst->acm_hue_smooth2 = pinfo->hue_smooth2;
	acm_dst->acm_hue_smooth3 = pinfo->hue_smooth3;
	acm_dst->acm_hue_smooth4 = pinfo->hue_smooth4;
	acm_dst->acm_hue_smooth5 = pinfo->hue_smooth5;
	acm_dst->acm_hue_smooth6 = pinfo->hue_smooth6;
	acm_dst->acm_hue_smooth7 = pinfo->hue_smooth7;
	acm_dst->acm_color_choose = pinfo->color_choose;
	acm_dst->acm_l_cont_en = pinfo->l_cont_en;
	acm_dst->acm_lc_param01  = pinfo->lc_param01;
	acm_dst->acm_lc_param23  = pinfo->lc_param23;
	acm_dst->acm_lc_param45  = pinfo->lc_param45;
	acm_dst->acm_lc_param67  = pinfo->lc_param67;
	acm_dst->acm_l_adj_ctrl = pinfo->l_adj_ctrl;
	acm_dst->acm_capture_ctrl = pinfo->capture_ctrl;
	acm_dst->acm_capture_in = pinfo->capture_in;
	acm_dst->acm_capture_out = pinfo->capture_out;
	acm_dst->acm_ink_ctrl = pinfo->ink_ctrl;
	acm_dst->acm_ink_out = pinfo->ink_out;
	acm_dst->acm_en = pinfo->acm_ce_support;

	if (hisi_effect_copy_to_user(acm_dst->hue_table, pinfo->acm_lut_hue_table, ACM_HUE_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm hue table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->sata_table, pinfo->acm_lut_sata_table, ACM_SATA_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm sata table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr0_table, pinfo->acm_lut_satr0_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr0 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr1_table, pinfo->acm_lut_satr1_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr1 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr2_table, pinfo->acm_lut_satr2_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr2 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr3_table, pinfo->acm_lut_satr3_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr3 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr4_table, pinfo->acm_lut_satr4_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr4 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr5_table, pinfo->acm_lut_satr5_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr5 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr6_table, pinfo->acm_lut_satr6_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr6 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisi_effect_copy_to_user(acm_dst->satr7_table, pinfo->acm_lut_satr7_table, ACM_SATR_LUT_LENGTH)) {
		HISI_FB_ERR("fb%d, failed to copy acm satr7 table to user!\n", hisifd->index);
		return -EINVAL;
	}

	return 0;
}

int hisi_effect_lcp_info_get(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == lcp) {
		HISI_FB_ERR("fb%d, lcp is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);

	if (hisifd->effect_ctl.lcp_gmp_support && (pinfo->gmp_lut_table_len == LCP_GMP_LUT_LENGTH)) {
		ret = hisi_effect_copy_to_user(lcp->gmp_table_low32, pinfo->gmp_lut_table_low32bit, LCP_GMP_LUT_LENGTH);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gmp_table_low32 to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(lcp->gmp_table_high4, pinfo->gmp_lut_table_high4bit, LCP_GMP_LUT_LENGTH);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gmp_table_high4 to user!\n", hisifd->index);
			goto err_ret;
		}
	}

	if (hisifd->effect_ctl.lcp_xcc_support && (pinfo->xcc_table_len == LCP_XCC_LUT_LENGTH)) {
		ret = hisi_effect_copy_to_user(lcp->xcc_table, pinfo->xcc_table, LCP_XCC_LUT_LENGTH);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy xcc_table to user!\n", hisifd->index);
			goto err_ret;
		}
	}

	if (hisifd->effect_ctl.lcp_igm_support&& (pinfo->igm_lut_table_len == IGM_LUT_LEN)) {
		ret = hisi_effect_copy_to_user(lcp->igm_r_table, pinfo->igm_lut_table_R, IGM_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy igm_r_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(lcp->igm_g_table, pinfo->igm_lut_table_G, IGM_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy igm_g_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(lcp->igm_b_table, pinfo->igm_lut_table_B, IGM_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy igm_b_table to user!\n", hisifd->index);
			goto err_ret;
		}
	}

err_ret:
	return ret;
}

int hisi_effect_hiace_info_get(struct hisi_fb_data_type *hisifd, struct hiace_info *hiace) {
	struct hisi_panel_info *pinfo = NULL;
	struct hiace_info *hiace_param = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == hiace) {
		HISI_FB_ERR("fb%d, hiace is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		HISI_FB_INFO("fb%d, hiace is not supported!\n", hisifd->index);
		return 0;
	}

	hiace_param = &(hisifd->effect_info.hiace);
	memcpy(hiace, hiace_param, sizeof(struct hiace_info));

	return ret;
}

int hisi_effect_gamma_info_get(struct hisi_fb_data_type *hisifd, struct gamma_info *gamma)
{
	struct hisi_panel_info *pinfo = NULL;
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == gamma) {
		HISI_FB_ERR("fb%d, gamma is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.gamma_support) {
		HISI_FB_INFO("fb%d, gamma is not supported!\n", hisifd->index);
		return 0;
	}

	pinfo = &(hisifd->panel_info);

	if (hisifd->effect_ctl.lcp_gmp_support && (pinfo->gamma_lut_table_len== GAMMA_LUT_LEN)) {
		gamma->para_mode = 0;

		ret = hisi_effect_copy_to_user(gamma->gamma_r_table, pinfo->gamma_lut_table_R, GAMMA_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gamma_r_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(gamma->gamma_g_table, pinfo->gamma_lut_table_G, GAMMA_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gamma_g_table to user!\n", hisifd->index);
			goto err_ret;
		}

		ret = hisi_effect_copy_to_user(gamma->gamma_b_table, pinfo->gamma_lut_table_B, GAMMA_LUT_LEN);
		if (ret) {
			HISI_FB_ERR("fb%d, failed to copy gamma_b_table to user!\n", hisifd->index);
			goto err_ret;
		}
	}

err_ret:
	return ret;
}
#define ARSR2P_MAX_NUM 3

int hisi_effect_arsr2p_info_set(struct hisi_fb_data_type *hisifd, struct arsr2p_info *arsr2p)
{
	uint32_t i;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == arsr2p) {
		HISI_FB_ERR("fb%d, arsr2p is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.arsr2p_sharp_support) {
		HISI_FB_INFO("fb%d, arsr2p sharp is not supported!\n", hisifd->index);
		return 0;
	}

	for (i = 0; i < ARSR2P_MAX_NUM; i++) {
		if (arsr2p[i].update == 1) {
			memcpy(&(hisifd->effect_info.arsr2p[i]), &(arsr2p[i]), sizeof(struct arsr2p_info));
		}
	}

	hisifd->effect_updated_flag.arsr2p_effect_updated = true;

	//debug info
	for (i = 0; i < ARSR2P_MAX_NUM; i++) {
		if (hisifd->effect_info.arsr2p[i].update) {
			HISI_FB_INFO("mode %d: enable : %u, sharp_enable:%u, shoot_enable:%u, skin_enable:%u, update: %u\n",
				i, hisifd->effect_info.arsr2p[i].enable, hisifd->effect_info.arsr2p[i].sharp_enable, hisifd->effect_info.arsr2p[i].shoot_enable, hisifd->effect_info.arsr2p[i].skin_enable,
                hisifd->effect_info.arsr2p[i].update);
		}
	}

	return 0;
}

int hisi_effect_arsr1p_info_set(struct hisi_fb_data_type *hisifd, struct arsr1p_info *arsr1p)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == arsr1p) {
		HISI_FB_ERR("fb%d, arsr1p is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	if (!hisifd->effect_ctl.arsr1p_sharp_support) {
		HISI_FB_INFO("fb%d, arsr1p sharp is not supported!\n", hisifd->index);
		return 0;
	}

	arsr1p->enable = (arsr1p->para_mode) & 0x1;
	memcpy(hisifd->effect_info.arsr1p, arsr1p, sizeof(struct arsr1p_info)*2);
	hisifd->effect_updated_flag.arsr1p_effect_updated = true;

	if (arsr1p[1].update == 1) {
		hisifd->effect_info.arsr1p_rog_initialized = true;
	}

	return 0;
}

static int set_acm_normal_param(struct acm_info *acm_dst, struct hisi_panel_info *pinfo)
{
	if (acm_dst == NULL) {
		HISI_FB_DEBUG("acm_dst is NULL!\n");
		return -1;
	}

	if (pinfo == NULL) {
		HISI_FB_DEBUG("pinfo is NULL!\n");
		return -1;
	}

	acm_dst->acm_hue_rlh01 = (pinfo->r1_lh<<16) | pinfo->r0_lh;
	acm_dst->acm_hue_rlh23 = (pinfo->r3_lh<<16) | pinfo->r2_lh;
	acm_dst->acm_hue_rlh45 = (pinfo->r5_lh<<16) | pinfo->r4_lh;
	acm_dst->acm_hue_rlh67 = (pinfo->r6_hh<<16) | pinfo->r6_lh;
	acm_dst->acm_hue_param01 = pinfo->hue_param01;
	acm_dst->acm_hue_param23 = pinfo->hue_param23;
	acm_dst->acm_hue_param45 = pinfo->hue_param45;
	acm_dst->acm_hue_param67 = pinfo->hue_param67;
	acm_dst->acm_hue_smooth0 = pinfo->hue_smooth0;
	acm_dst->acm_hue_smooth1 = pinfo->hue_smooth1;
	acm_dst->acm_hue_smooth2 = pinfo->hue_smooth2;
	acm_dst->acm_hue_smooth3 = pinfo->hue_smooth3;
	acm_dst->acm_hue_smooth4 = pinfo->hue_smooth4;
	acm_dst->acm_hue_smooth5 = pinfo->hue_smooth5;
	acm_dst->acm_hue_smooth6 = pinfo->hue_smooth6;
	acm_dst->acm_hue_smooth7 = pinfo->hue_smooth7;
	acm_dst->acm_color_choose = pinfo->color_choose;
	acm_dst->acm_l_cont_en = pinfo->l_cont_en;
	acm_dst->acm_lc_param01  = pinfo->lc_param01;
	acm_dst->acm_lc_param23  = pinfo->lc_param23;
	acm_dst->acm_lc_param45  = pinfo->lc_param45;
	acm_dst->acm_lc_param67  = pinfo->lc_param67;
	acm_dst->acm_l_adj_ctrl = pinfo->l_adj_ctrl;
	acm_dst->acm_capture_ctrl = pinfo->capture_ctrl;
	acm_dst->acm_ink_ctrl = pinfo->ink_ctrl;
	acm_dst->acm_ink_out = pinfo->ink_out;

	/* malloc acm_dst lut table memory*/
	if (hisi_effect_alloc_and_copy(&acm_dst->hue_table, pinfo->acm_lut_hue_table,
		ACM_HUE_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm hut table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->sata_table, pinfo->acm_lut_sata_table,
		ACM_SATA_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm sata table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr0_table, pinfo->acm_lut_satr0_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr0 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr1_table, pinfo->acm_lut_satr1_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr1 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr2_table, pinfo->acm_lut_satr2_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr2 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr3_table, pinfo->acm_lut_satr3_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr3 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr4_table, pinfo->acm_lut_satr4_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr4 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr5_table, pinfo->acm_lut_satr5_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr5 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr6_table, pinfo->acm_lut_satr6_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr6 table from panel\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr7_table, pinfo->acm_lut_satr7_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr7 table from panel\n");
		return -EINVAL;
	}

	return 0;
}

static int set_acm_cinema_param(struct acm_info *acm_dst, struct hisi_panel_info *pinfo)
{
	if (acm_dst == NULL) {
		HISI_FB_DEBUG("acm_dst is NULL!\n");
		return -1;
	}

	if (pinfo == NULL) {
		HISI_FB_DEBUG("pinfo is NULL!\n");
		return -1;
	}

	acm_dst->acm_hue_rlh01 = (pinfo->cinema_r1_lh<<16) | pinfo->cinema_r0_lh;
	acm_dst->acm_hue_rlh23 = (pinfo->cinema_r3_lh<<16) | pinfo->cinema_r2_lh;
	acm_dst->acm_hue_rlh45 = (pinfo->cinema_r5_lh<<16) | pinfo->cinema_r4_lh;
	acm_dst->acm_hue_rlh67 = (pinfo->cinema_r6_hh<<16) | pinfo->cinema_r6_lh;

	acm_dst->acm_hue_param01 = pinfo->hue_param01;
	acm_dst->acm_hue_param23 = pinfo->hue_param23;
	acm_dst->acm_hue_param45 = pinfo->hue_param45;
	acm_dst->acm_hue_param67 = pinfo->hue_param67;
	acm_dst->acm_hue_smooth0 = pinfo->hue_smooth0;
	acm_dst->acm_hue_smooth1 = pinfo->hue_smooth1;
	acm_dst->acm_hue_smooth2 = pinfo->hue_smooth2;
	acm_dst->acm_hue_smooth3 = pinfo->hue_smooth3;
	acm_dst->acm_hue_smooth4 = pinfo->hue_smooth4;
	acm_dst->acm_hue_smooth5 = pinfo->hue_smooth5;
	acm_dst->acm_hue_smooth6 = pinfo->hue_smooth6;
	acm_dst->acm_hue_smooth7 = pinfo->hue_smooth7;
	acm_dst->acm_color_choose = pinfo->color_choose;
	acm_dst->acm_l_cont_en = pinfo->l_cont_en;
	acm_dst->acm_lc_param01  = pinfo->lc_param01;
	acm_dst->acm_lc_param23  = pinfo->lc_param23;
	acm_dst->acm_lc_param45  = pinfo->lc_param45;
	acm_dst->acm_lc_param67  = pinfo->lc_param67;
	acm_dst->acm_l_adj_ctrl = pinfo->l_adj_ctrl;
	acm_dst->acm_capture_ctrl = pinfo->capture_ctrl;
	acm_dst->acm_ink_ctrl = pinfo->ink_ctrl;
	acm_dst->acm_ink_out = pinfo->ink_out;
	/* malloc acm_dst lut table memory*/
	if (hisi_effect_alloc_and_copy(&acm_dst->hue_table, pinfo->cinema_acm_lut_hue_table,
		ACM_HUE_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm hut table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->sata_table, pinfo->cinema_acm_lut_sata_table,
		ACM_SATA_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm sata table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr0_table, pinfo->cinema_acm_lut_satr0_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr0 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr1_table, pinfo->cinema_acm_lut_satr1_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr1 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr2_table, pinfo->cinema_acm_lut_satr2_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr2 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr3_table, pinfo->cinema_acm_lut_satr3_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr3 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr4_table, pinfo->cinema_acm_lut_satr4_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr4 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr5_table, pinfo->cinema_acm_lut_satr5_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr5 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr6_table, pinfo->cinema_acm_lut_satr6_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr6 table from panel cinema mode\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr7_table, pinfo->cinema_acm_lut_satr7_table,
		ACM_SATR_LUT_LENGTH, false)) {
		HISI_FB_ERR("failed to set acm satr7 table from panel cinema mode\n");
		return -EINVAL;
	}

	return 0;
}

static int set_acm_user_param(struct acm_info *acm_dst, struct acm_info *acm_src)
{
	if (acm_dst == NULL) {
		HISI_FB_DEBUG("acm_dst is NULL!\n");
		return -1;
	}

	if (acm_src == NULL) {
		HISI_FB_DEBUG("acm_src is NULL!\n");
		return -1;
	}
	acm_dst->acm_en = acm_src->acm_en;
	acm_dst->sata_offset = acm_src->sata_offset;
	acm_dst->acm_hue_rlh01 = acm_src->acm_hue_rlh01;
	acm_dst->acm_hue_rlh23 = acm_src->acm_hue_rlh23;
	acm_dst->acm_hue_rlh45 = acm_src->acm_hue_rlh45;
	acm_dst->acm_hue_rlh67 = acm_src->acm_hue_rlh67;
	acm_dst->acm_hue_param01 = acm_src->acm_hue_param01;
	acm_dst->acm_hue_param23 = acm_src->acm_hue_param23;
	acm_dst->acm_hue_param45 = acm_src->acm_hue_param45;
	acm_dst->acm_hue_param67 = acm_src->acm_hue_param67;
	acm_dst->acm_hue_smooth0 = acm_src->acm_hue_smooth0;
	acm_dst->acm_hue_smooth1 = acm_src->acm_hue_smooth1;
	acm_dst->acm_hue_smooth2 = acm_src->acm_hue_smooth2;
	acm_dst->acm_hue_smooth3 = acm_src->acm_hue_smooth3;
	acm_dst->acm_hue_smooth4 = acm_src->acm_hue_smooth4;
	acm_dst->acm_hue_smooth5 = acm_src->acm_hue_smooth5;
	acm_dst->acm_hue_smooth6 = acm_src->acm_hue_smooth6;
	acm_dst->acm_hue_smooth7 = acm_src->acm_hue_smooth7;
	acm_dst->acm_color_choose = acm_src->acm_color_choose;
	acm_dst->acm_l_cont_en = acm_src->acm_l_cont_en;
	acm_dst->acm_lc_param01 = acm_src->acm_lc_param01;
	acm_dst->acm_lc_param23 = acm_src->acm_lc_param23;
	acm_dst->acm_lc_param45 = acm_src->acm_lc_param45;
	acm_dst->acm_lc_param67 = acm_src->acm_lc_param67;
	acm_dst->acm_l_adj_ctrl = acm_src->acm_l_adj_ctrl;
	acm_dst->acm_capture_out = acm_src->acm_capture_out;
	acm_dst->acm_ink_ctrl = acm_src->acm_ink_ctrl;
	acm_dst->acm_ink_out = acm_src->acm_ink_out;

	/* malloc acm_dst lut table memory*/
	if (hisi_effect_alloc_and_copy(&acm_dst->hue_table, acm_src->hue_table,
		ACM_HUE_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm hut table from user\n");
		return -EINVAL;
	}

	if (hisi_effect_alloc_and_copy(&acm_dst->sata_table, acm_src->sata_table,
		ACM_SATA_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm sata table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr0_table, acm_src->satr0_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr0 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr1_table, acm_src->satr1_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr1 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr2_table, acm_src->satr2_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr2 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr3_table, acm_src->satr3_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr3 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr4_table, acm_src->satr4_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr4 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr5_table, acm_src->satr5_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr5 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr6_table, acm_src->satr6_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr6 table from user\n");
		return -EINVAL;
	}
	if (hisi_effect_alloc_and_copy(&acm_dst->satr7_table, acm_src->satr7_table,
		ACM_SATR_LUT_LENGTH, true)) {
		HISI_FB_ERR("failed to copy acm satr7 table from user\n");
		return -EINVAL;
	}

	return 0;
}

int hisi_effect_acm_info_set(struct hisi_fb_data_type *hisifd, struct acm_info *acm_src)
{
	struct acm_info *acm_dst = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (NULL == acm_src) {
		HISI_FB_ERR("fb%d, acm_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	acm_dst = &(hisifd->effect_info.acm);
	pinfo = &(hisifd->panel_info);

	if (!hisifd->effect_ctl.acm_support) {
		HISI_FB_INFO("fb%d, acm is not supported!\n", hisifd->index);
		return 0;
	}

	/*set acm info*/
	if (hisifd->effect_updated_flag.acm_effect_updated == false) {
		acm_dst->acm_en = acm_src->acm_en;
		if (acm_src->param_mode == 0) {
			if (set_acm_normal_param(acm_dst, pinfo)) {
				HISI_FB_ERR("fb%d, failed to set acm normal mode parameters\n", hisifd->index);
				goto err_ret;
			}
		} else if (acm_src->param_mode == 1) {
			if (set_acm_cinema_param(acm_dst, pinfo)) {
				HISI_FB_ERR("fb%d, failed to set acm cinema mode parameters\n", hisifd->index);
				goto err_ret;
			}
		} else if (acm_src->param_mode == 2) {
			if (set_acm_user_param(acm_dst, acm_src)) {
				HISI_FB_ERR("fb%d, failed to set acm cinema mode parameters\n", hisifd->index);
				goto err_ret;
			}
		} else {
			HISI_FB_ERR("fb%d, invalid acm para mode!\n", hisifd->index);
			return -EINVAL;
		}

		hisifd->effect_updated_flag.acm_effect_updated = true;
	}
	return 0;

err_ret:
	free_acm_table(acm_dst);
	return -EINVAL;
}

int hisi_effect_gmp_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src){
	struct lcp_info *lcp_dst = NULL;
	struct dss_effect *effect = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == lcp_src) {
		HISI_FB_ERR("fb%d, lcp_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	lcp_dst = &(hisifd->effect_info.lcp);
	effect = &(hisifd->effect_ctl);

	if (!effect->lcp_gmp_support) {
		HISI_FB_INFO("fb%d, lcp gmp is not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_gmp_effect_lock);

	lcp_dst->gmp_enable = lcp_src->gmp_enable;
	if (hisi_effect_alloc_and_copy(&lcp_dst->gmp_table_high4, lcp_src->gmp_table_high4,
		LCP_GMP_LUT_LENGTH, true)) {
		HISI_FB_ERR("fb%d, failed to set gmp_table_high4!\n", hisifd->index);
		goto err_ret;
	}

	if (hisi_effect_alloc_and_copy(&lcp_dst->gmp_table_low32, lcp_src->gmp_table_low32,
		LCP_GMP_LUT_LENGTH, true)) {
		HISI_FB_ERR("fb%d, failed to set gmp_lut_table_low32bit!\n", hisifd->index);
		goto err_ret;
	}

	hisifd->effect_updated_flag.gmp_effect_updated = true;

	spin_unlock(&g_gmp_effect_lock);
	return 0;

err_ret:
	hisi_effect_kfree(&lcp_dst->gmp_table_high4);
	hisi_effect_kfree(&lcp_dst->gmp_table_low32);

	spin_unlock(&g_gmp_effect_lock);
	return -EINVAL;
}

int hisi_effect_igm_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src){
	struct lcp_info *lcp_dst = NULL;
	struct dss_effect *effect = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == lcp_src) {
		HISI_FB_ERR("fb%d, lcp_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	lcp_dst = &(hisifd->effect_info.lcp);
	effect = &(hisifd->effect_ctl);

	if (!effect->lcp_igm_support) {
		HISI_FB_INFO("fb%d, lcp degamma is not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_igm_effect_lock);

	lcp_dst->igm_enable = lcp_src->igm_enable;

	if (hisi_effect_alloc_and_copy(&lcp_dst->igm_r_table, lcp_src->igm_r_table,
		IGM_LUT_LEN, true)) {
		HISI_FB_ERR("fb%d, failed to set igm_r_table!\n", hisifd->index);
		goto err_ret;
	}

	if (hisi_effect_alloc_and_copy(&lcp_dst->igm_g_table, lcp_src->igm_g_table,
		IGM_LUT_LEN, true)) {
		HISI_FB_ERR("fb%d, failed to set igm_g_table!\n", hisifd->index);
		goto err_ret;
	}

	if (hisi_effect_alloc_and_copy(&lcp_dst->igm_b_table, lcp_src->igm_b_table,
		IGM_LUT_LEN, true)) {
		HISI_FB_ERR("fb%d, failed to set igm_b_table!\n", hisifd->index);
		goto err_ret;
	}

	hisifd->effect_updated_flag.igm_effect_updated = true;

	spin_unlock(&g_igm_effect_lock);
	return 0;

err_ret:
	hisi_effect_kfree(&lcp_dst->igm_r_table);
	hisi_effect_kfree(&lcp_dst->igm_g_table);
	hisi_effect_kfree(&lcp_dst->igm_b_table);

	spin_unlock(&g_igm_effect_lock);
	return -EINVAL;
}

int hisi_effect_xcc_info_set(struct hisi_fb_data_type *hisifd, struct lcp_info *lcp_src){
	struct lcp_info *lcp_dst = NULL;
	struct dss_effect *effect = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == lcp_src) {
		HISI_FB_ERR("fb%d, lcp_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	lcp_dst = &(hisifd->effect_info.lcp);
	effect = &(hisifd->effect_ctl);

	if (!effect->lcp_xcc_support) {
		HISI_FB_INFO("fb%d, lcp xcc are not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_xcc_effect_lock);

	lcp_dst->xcc_enable = lcp_src->xcc_enable;

	if (hisi_effect_alloc_and_copy(&lcp_dst->xcc_table, lcp_src->xcc_table,
		LCP_XCC_LUT_LENGTH, true)) {
		HISI_FB_ERR("fb%d, failed to set xcc_table!\n", hisifd->index);
		goto err_ret;
	}

	hisifd->effect_updated_flag.xcc_effect_updated = true;

	spin_unlock(&g_xcc_effect_lock);
	return 0;

err_ret:
	hisi_effect_kfree(&lcp_dst->xcc_table);

	spin_unlock(&g_xcc_effect_lock);
	return -EINVAL;
}

static int hisi_efffect_gamma_param_set(struct gamma_info *gammaDst, struct gamma_info *gammaSrc,
                                  struct hisi_panel_info* pInfo) {

	if (gammaSrc->para_mode == 0) {
		//Normal mode
		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_r_table, pInfo->gamma_lut_table_R,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_r_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_g_table, pInfo->gamma_lut_table_G,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_g_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_b_table, pInfo->gamma_lut_table_B,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_b_table!\n");
			goto err_ret;
		}
	} else if (gammaSrc->para_mode == 1) {
		//Cinema mode
		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_r_table, pInfo->cinema_gamma_lut_table_R,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_r_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_g_table, pInfo->cinema_gamma_lut_table_G,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_g_table!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_b_table, pInfo->cinema_gamma_lut_table_B,
			GAMMA_LUT_LEN, false)) {
			HISI_FB_ERR("failed to set gamma_b_table!\n");
			goto err_ret;
		}
	} else if (gammaSrc->para_mode == 2) {
		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_r_table, gammaSrc->gamma_r_table,
			GAMMA_LUT_LEN, true)) {
			HISI_FB_ERR("failed to copy gamma_r_table from user!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_g_table, gammaSrc->gamma_g_table,
			GAMMA_LUT_LEN, true)) {
			HISI_FB_ERR("failed to copy gamma_g_table from user!\n");
			goto err_ret;
		}

		if (hisi_effect_alloc_and_copy(&gammaDst->gamma_b_table, gammaSrc->gamma_b_table,
			GAMMA_LUT_LEN, true)) {
			HISI_FB_ERR("failed to copy gamma_b_table from user!\n");
			goto err_ret;
		}
	} else {
		HISI_FB_ERR("not supported gamma para_mode!\n");
		return -EINVAL;
	}

	return 0;

err_ret:
	free_gamma_table(gammaDst);

	return -EINVAL;
}

int hisi_effect_gamma_info_set(struct hisi_fb_data_type *hisifd, struct gamma_info *gamma_src)
{
	struct gamma_info *gamma_dst = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -EINVAL;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return -EINVAL;
	}

	if (NULL == gamma_src) {
		HISI_FB_ERR("fb%d, gamma_src is NULL!\n", hisifd->index);
		return -EINVAL;
	}

	gamma_dst = &(hisifd->effect_info.gamma);
	pinfo = &(hisifd->panel_info);

	if (!hisifd->effect_ctl.gamma_support) {
		HISI_FB_INFO("fb%d, gamma is not supported!\n", hisifd->index);
		return 0;
	}

	spin_lock(&g_gamma_effect_lock);

	gamma_dst->enable = gamma_src->enable;
	gamma_dst->para_mode = gamma_src->para_mode;
	ret = hisi_efffect_gamma_param_set(gamma_dst, gamma_src, pinfo);
	if (ret < 0) {
		HISI_FB_ERR("fb%d, failed to set gamma table!\n", hisifd->index);
		spin_unlock(&g_gamma_effect_lock);
		return ret;
	}

	hisifd->effect_updated_flag.gamma_effect_updated = true;

	spin_unlock(&g_gamma_effect_lock);

	return 0;
}

void hisi_effect_acm_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct acm_info *acm_param = NULL;
	char __iomem *acm_base = NULL;
	char __iomem *acm_lut_base = NULL;
	static uint32_t acm_config_flag = 0;
	uint32_t acm_lut_sel;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (!hisifd->effect_ctl.acm_support) {
		return;
	}

	if (!hisifd->effect_updated_flag.acm_effect_updated) {
		return;
	}

	acm_base = hisifd->dss_base + DSS_DPP_ACM_OFFSET;
	acm_lut_base = hisifd->dss_base + DSS_DPP_ACM_LUT_OFFSET;

	if (acm_config_flag == 0) {
		//Disable ACM
		set_reg(acm_base + ACM_EN_ES, 0x0, 1, 0);
		acm_config_flag = 1;
		return;
	}

	acm_param = &(hisifd->effect_info.acm);

	if (NULL == acm_param->hue_table) {
		HISI_FB_INFO("fb%d, invalid acm hue table param!\n", hisifd->index);
		goto err_ret;
	}

	if (NULL == acm_param->sata_table) {
		HISI_FB_INFO("fb%d, invalid acm sata table param!\n", hisifd->index);
		goto err_ret;
	}

	if ((NULL == acm_param->satr0_table) || (NULL == acm_param->satr1_table)
		|| (NULL == acm_param->satr2_table) || (NULL == acm_param->satr3_table)
		|| (NULL == acm_param->satr4_table) || (NULL == acm_param->satr5_table)
		|| (NULL == acm_param->satr6_table) || (NULL == acm_param->satr7_table)) {
		HISI_FB_INFO("fb%d, invalid acm satr table param!\n", hisifd->index);
		goto err_ret;
	}

	set_reg(acm_base + ACM_SATA_OFFSET, acm_param->sata_offset, 6, 0);
	set_reg(acm_base + ACM_HUE_RLH01, acm_param->acm_hue_rlh01, 32, 0);
	set_reg(acm_base + ACM_HUE_RLH23, acm_param->acm_hue_rlh23, 32, 0);
	set_reg(acm_base + ACM_HUE_RLH45, acm_param->acm_hue_rlh45, 32, 0);
	set_reg(acm_base + ACM_HUE_RLH67, acm_param->acm_hue_rlh67, 32, 0);
	set_reg(acm_base + ACM_HUE_PARAM01, acm_param->acm_hue_param01, 32,0);
	set_reg(acm_base + ACM_HUE_PARAM23, acm_param->acm_hue_param23, 32,0);
	set_reg(acm_base + ACM_HUE_PARAM45, acm_param->acm_hue_param45, 32,0);
	set_reg(acm_base + ACM_HUE_PARAM67, acm_param->acm_hue_param67, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH0, acm_param->acm_hue_smooth0, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH1, acm_param->acm_hue_smooth1, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH2, acm_param->acm_hue_smooth2, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH3, acm_param->acm_hue_smooth3, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH4, acm_param->acm_hue_smooth4, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH5, acm_param->acm_hue_smooth5, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH6, acm_param->acm_hue_smooth6, 32,0);
	set_reg(acm_base + ACM_HUE_SMOOTH7, acm_param->acm_hue_smooth7, 32,0);
	set_reg(acm_base + ACM_COLOR_CHOOSE,acm_param->acm_color_choose,1,0);
	set_reg(acm_base + ACM_L_CONT_EN, acm_param->acm_l_cont_en, 1,0);
	set_reg(acm_base + ACM_LC_PARAM01, acm_param->acm_lc_param01, 32,0);
	set_reg(acm_base + ACM_LC_PARAM23, acm_param->acm_lc_param23, 32,0);
	set_reg(acm_base + ACM_LC_PARAM45, acm_param->acm_lc_param45, 32,0);
	set_reg(acm_base + ACM_LC_PARAM67, acm_param->acm_lc_param67, 32,0);
	set_reg(acm_base + ACM_L_ADJ_CTRL, acm_param->acm_l_adj_ctrl, 32,0);
	set_reg(acm_base + ACM_CAPTURE_CTRL, acm_param->acm_capture_ctrl,32,0);
	set_reg(acm_base + ACM_INK_CTRL, acm_param->acm_ink_ctrl, 32,0);
	set_reg(acm_base + ACM_INK_OUT, acm_param->acm_ink_out, 30,0);

	acm_set_lut_hue(acm_lut_base + ACM_U_H_COEF, acm_param->hue_table, ACM_HUE_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATA_COEF, acm_param->sata_table, ACM_SATA_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR0_COEF, acm_param->satr0_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR1_COEF, acm_param->satr1_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR2_COEF, acm_param->satr2_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR3_COEF, acm_param->satr3_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR4_COEF, acm_param->satr4_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR5_COEF, acm_param->satr5_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR6_COEF, acm_param->satr6_table, ACM_SATR_LUT_LENGTH);
	acm_set_lut(acm_lut_base + ACM_U_SATR7_COEF, acm_param->satr7_table, ACM_SATR_LUT_LENGTH);

	acm_lut_sel = (uint32_t)inp32(acm_base + ACM_LUT_SEL);
	set_reg(acm_base + ACM_LUT_SEL, (~(acm_lut_sel & 0x380)) & (acm_lut_sel | 0x380), 16, 0);

	set_reg(acm_base + ACM_EN, acm_param->acm_en, 1, 0);
err_ret:
	hisifd->effect_updated_flag.acm_effect_updated = false;

	//free_acm_table(acm_param);
	acm_config_flag = 0;
	return;
}

#define XCC_COEF_LEN	12
#define GMP_BLOCK_SIZE	137
#define GMP_CNT_NUM 18
#define GMP_COFE_CNT 729

static bool lcp_igm_set_reg(char __iomem *degamma_lut_base, struct lcp_info *lcp_param)
{
	int cnt;

	if (degamma_lut_base == NULL) {
		HISI_FB_ERR("lcp_lut_base is NULL!\n");
		return false;
	}

	if (lcp_param == NULL) {
		HISI_FB_ERR("lcp_param is NULL!\n");
		return false;
	}

	if (lcp_param->igm_r_table == NULL || lcp_param->igm_g_table == NULL || lcp_param->igm_b_table == NULL) {
		HISI_FB_INFO("igm_r_table or igm_g_table or igm_b_table	is NULL!\n");
		return false;
	}

	for (cnt = 0; cnt < IGM_LUT_LEN; cnt = cnt + 2) {
		set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + cnt * 2), lcp_param->igm_r_table[cnt], 12,0);
		if(cnt != IGM_LUT_LEN-1)
			set_reg(degamma_lut_base + (U_DEGAMA_R_COEF + cnt * 2), lcp_param->igm_r_table[cnt+1], 12,16);

		set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + cnt * 2), lcp_param->igm_g_table[cnt], 12,0);
		if(cnt != IGM_LUT_LEN-1)
			set_reg(degamma_lut_base + (U_DEGAMA_G_COEF + cnt * 2), lcp_param->igm_g_table[cnt+1], 12,16);

		set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + cnt * 2), lcp_param->igm_b_table[cnt], 12,0);
		if(cnt != IGM_LUT_LEN-1)
			set_reg(degamma_lut_base + (U_DEGAMA_B_COEF + cnt * 2), lcp_param->igm_b_table[cnt+1], 12,16);
	}
	return true;
}

static bool lcp_xcc_set_reg(char __iomem *xcc_base, struct lcp_info *lcp_param)
{
	int cnt;

	if (xcc_base == NULL) {
		HISI_FB_DEBUG("lcp_base is NULL!\n");
		return false;
	}

	if (lcp_param == NULL) {
		HISI_FB_DEBUG("lcp_param is NULL!\n");
		return false;
	}

	if (lcp_param->xcc_table == NULL) {
		HISI_FB_DEBUG("xcc_table is NULL!\n");
		return false;
	}
	for (cnt = 0; cnt < XCC_COEF_LEN; cnt++) {
		set_reg(xcc_base + XCC_COEF_00 +cnt * 4,  lcp_param->xcc_table[cnt], 17, 0);
	}
	return true;
}

static bool lcp_gmp_set_reg(char __iomem *gmp_lut_base, struct lcp_info *lcp_param)
{
	int i;

	if (gmp_lut_base == NULL) {
		HISI_FB_DEBUG("lcp_lut_base is NULL!\n");
		return false;
	}

	if (lcp_param == NULL) {
		HISI_FB_DEBUG("lcp_param is NULL!\n");
		return false;
	}
	if (lcp_param->gmp_table_low32 == NULL || lcp_param->gmp_table_high4 == NULL) {
		HISI_FB_DEBUG("gmp_table_low32 or gmp_table_high4 is NULL!\n");
		return false;
	}
	for (i = 0; i < GMP_COFE_CNT; i++) {
		set_reg(gmp_lut_base + i * 2 * 4, lcp_param->gmp_table_low32[i], 32, 0);
		set_reg(gmp_lut_base + i * 2 * 4 + 4, lcp_param->gmp_table_high4[i], 4, 0);
	}
	return true;
}

void hisi_effect_lcp_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct dss_effect *effect = NULL;
	struct lcp_info *lcp_param = NULL;
	char __iomem *xcc_base = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *degamma_base = NULL;
	char __iomem *gmp_lut_base = NULL;
	char __iomem *degamma_lut_base = NULL;
	char __iomem *lcp_base = NULL;
	uint32_t degama_lut_sel;
	uint32_t gmp_lut_sel;
	bool ret;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return;
	}

	effect = &hisifd->effect_ctl;

	lcp_base = hisifd->dss_base + DSS_DPP_LCP_OFFSET_ES;
	xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
	gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET;
	degamma_base = hisifd->dss_base + DSS_DPP_DEGAMMA_OFFSET;
	degamma_lut_base = hisifd->dss_base + DSS_DPP_DEGAMMA_LUT_OFFSET;
	gmp_lut_base = hisifd->dss_base + DSS_DPP_GMP_LUT_OFFSET;

	lcp_param = &(hisifd->effect_info.lcp);

	//Update De-Gamma LUT
	if (effect->lcp_igm_support && hisifd->effect_updated_flag.igm_effect_updated) {
		if (spin_can_lock(&g_igm_effect_lock)) {
			spin_lock(&g_igm_effect_lock);
			ret = lcp_igm_set_reg(degamma_lut_base, lcp_param);

			//Enable De-Gamma
			if (ret) {
				degama_lut_sel = inp32(degamma_base + DEGAMA_LUT_SEL);
				set_reg(degamma_base + DEGAMA_LUT_SEL, (~(degama_lut_sel & 0x1)) & 0x1, 1, 0);
				set_reg(degamma_base + DEGAMA_EN,  lcp_param->igm_enable, 1, 0);
			}
			hisifd->effect_updated_flag.igm_effect_updated = false;
			spin_unlock(&g_igm_effect_lock);
		} else {
			HISI_FB_INFO("igm effect param is being updated, delay set reg to next frame!\n");
		}
	}

	//Update XCC Coef
	if (effect->lcp_xcc_support && hisifd->effect_updated_flag.xcc_effect_updated) {
		if (spin_can_lock(&g_xcc_effect_lock)) {
			spin_lock(&g_xcc_effect_lock);
			ret = lcp_xcc_set_reg(xcc_base, lcp_param);
			//Enable XCC
			if (ret) {
				set_reg(xcc_base + XCC_EN,	lcp_param->xcc_enable, 1, 0);
				//Enable XCC pre
				//set_reg(xcc_base + XCC_EN,  lcp_param->xcc_enable, 1, 1);
			}
			hisifd->effect_updated_flag.xcc_effect_updated = false;
			spin_unlock(&g_xcc_effect_lock);
		} else {
			HISI_FB_INFO("xcc effect param is being updated, delay set reg to next frame!\n");
		}
	}

	//Update GMP LUT
	if (effect->lcp_gmp_support && hisifd->effect_updated_flag.gmp_effect_updated) {
		if (spin_can_lock(&g_gmp_effect_lock)) {
			spin_lock(&g_gmp_effect_lock);
			ret = lcp_gmp_set_reg(gmp_lut_base, lcp_param);
			//Enable GMP
			if (ret) {
				gmp_lut_sel = inp32(gmp_base + GMP_LUT_SEL);
				set_reg(gmp_base + GMP_LUT_SEL, (~(gmp_lut_sel & 0x1)) & 0x1, 1, 0);
				set_reg(gmp_base + GMP_EN, lcp_param->gmp_enable, 1, 0);
			}
			hisifd->effect_updated_flag.gmp_effect_updated = false;
			spin_unlock(&g_gmp_effect_lock);
		} else {
			HISI_FB_INFO("gmp effect param is being updated, delay set reg to next frame!\n");
		}
	}

	//free_lcp_table(lcp_param);
	return;
}

void hisi_effect_gamma_set_reg(struct hisi_fb_data_type *hisifd)
{
	struct gamma_info *gamma_param = NULL;
	char __iomem *gamma_base = NULL;
	char __iomem *gamma_lut_base = NULL;
	char __iomem *gamma_pre_lut_base = NULL;
	int cnt = 0;
	uint32_t gama_lut_sel;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (!g_is_effect_lock_init) {
		HISI_FB_INFO("display effect lock is not init!\n");
		return;
	}

	if (!hisifd->effect_ctl.gamma_support) {
		return;
	}

	if (!hisifd->effect_updated_flag.gamma_effect_updated) {
		return;
	}

	gamma_base = hisifd->dss_base + DSS_DPP_GAMA_OFFSET;
	gamma_lut_base = hisifd->dss_base + DSS_DPP_GAMA_LUT_OFFSET;
	gamma_pre_lut_base = hisifd->dss_base + DSS_DPP_GAMA_PRE_LUT_OFFSET;

	gamma_param = &(hisifd->effect_info.gamma);

	if (!spin_can_lock(&g_gamma_effect_lock)) {
	    HISI_FB_INFO("gamma effect param is being updated, delay set reg to next frame!\n");
	    return;
	}
	spin_lock(&g_gamma_effect_lock);

	if ((NULL == gamma_param->gamma_r_table) ||
		(NULL == gamma_param->gamma_g_table) ||
		(NULL == gamma_param->gamma_b_table)) {
		HISI_FB_INFO("fb%d, gamma table is null!\n", hisifd->index);
		goto err_ret;
	}

	//Update Gamma LUT
	for (cnt = 0; cnt < GAMMA_LUT_LEN; cnt = cnt + 2) {
		set_reg(gamma_lut_base + (U_GAMA_R_COEF + cnt * 2), gamma_param->gamma_r_table[cnt], 12,0);
		if (cnt != GAMMA_LUT_LEN - 1)
			set_reg(gamma_lut_base + (U_GAMA_R_COEF + cnt * 2), gamma_param->gamma_r_table[cnt+1], 12,16);

		set_reg(gamma_lut_base + (U_GAMA_G_COEF + cnt * 2), gamma_param->gamma_g_table[cnt], 12,0);
		if (cnt != GAMMA_LUT_LEN - 1)
			set_reg(gamma_lut_base + (U_GAMA_G_COEF + cnt * 2), gamma_param->gamma_g_table[cnt+1], 12,16);

		set_reg(gamma_lut_base + (U_GAMA_B_COEF + cnt * 2), gamma_param->gamma_b_table[cnt], 12,0);
		if (cnt != GAMMA_LUT_LEN - 1)
			set_reg(gamma_lut_base + (U_GAMA_B_COEF + cnt * 2), gamma_param->gamma_b_table[cnt+1], 12,16);
	}

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		for (cnt = 0; cnt < GAMMA_LUT_LEN; cnt = cnt + 2) {
			set_reg(gamma_pre_lut_base + (U_GAMA_PRE_R_COEF + cnt * 2), gamma_param->gamma_r_table[cnt], 12,0);
			if (cnt != GAMMA_LUT_LEN - 1)
				set_reg(gamma_pre_lut_base + (U_GAMA_PRE_R_COEF + cnt * 2), gamma_param->gamma_r_table[cnt+1], 12,16);


			set_reg(gamma_pre_lut_base + (U_GAMA_PRE_G_COEF + cnt * 2), gamma_param->gamma_g_table[cnt], 12,0);
			if (cnt != GAMMA_LUT_LEN - 1)
				set_reg(gamma_pre_lut_base + (U_GAMA_PRE_G_COEF + cnt * 2), gamma_param->gamma_g_table[cnt+1], 12,16);


			set_reg(gamma_pre_lut_base + (U_GAMA_PRE_B_COEF + cnt * 2), gamma_param->gamma_b_table[cnt], 12,0);
			if (cnt != GAMMA_LUT_LEN - 1)
				set_reg(gamma_pre_lut_base + (U_GAMA_PRE_B_COEF + cnt * 2), gamma_param->gamma_b_table[cnt+1], 12,16);
		}

		gama_lut_sel = inp32(gamma_base + GAMA_LUT_SEL);
		set_reg(gamma_base + GAMA_LUT_SEL, (~(gama_lut_sel & 0x1)) & 0x1, 1, 0);
	}

	//Enable Gamma
	set_reg(gamma_base + GAMA_EN,  gamma_param->enable, 1, 0);

err_ret:
	hisifd->effect_updated_flag.gamma_effect_updated = false;
	//free_gamma_table(gamma_param);

	spin_unlock(&g_gamma_effect_lock);

	return;
}

/*******************************************************************************
**ACM REG DIMMING
*/
/*lint -e866*/
static inline uint32_t cal_hue_value(uint32_t cur,uint32_t target,int count) {
	if (count <= 0) {
		return cur;
	}

	if (abs((int)(cur - target)) > HUE_REG_RANGE) {
		if (cur > target) {
			target += HUE_REG_OFFSET;
		} else {
			cur += HUE_REG_OFFSET;
		}
	}

	if (target > cur) {
		cur += (target - cur) / (uint32_t)count;
	} else if (cur > target) {
		cur -= (cur - target) / (uint32_t)count;
	}

	cur %= HUE_REG_OFFSET;

	return cur;
}

static inline uint32_t cal_sata_value(uint32_t cur,uint32_t target,int count) {
	if (count <= 0) {
		return cur;
	}

	if (target > cur) {
		cur += (target - cur) / (uint32_t)count;
	} else if (cur > target) {
		cur -= (cur - target) / (uint32_t)count;
	}

	return cur;
}

/*lint -e838*/
static inline int satr_unint32_to_int(uint32_t input) {
	int result = 0;

	if (input >= SATR_REG_RANGE) {
		result = (int)input - SATR_REG_OFFSET;
	} else {
		result = (int)input;
	}
	return result;
}

/*lint -e838*/
static inline uint32_t cal_satr_value(uint32_t cur,uint32_t target,int count) {
	int i_cur = 0;
	int i_target = 0;

	if (count <= 0) {
		return cur;
	}

	i_cur = satr_unint32_to_int(cur);
	i_target = satr_unint32_to_int(target);

	return (uint32_t)(i_cur + (i_target - i_cur) / count) % SATR_REG_OFFSET;
}

/*lint -e838*/
static inline void cal_cur_acm_reg(int count,acm_reg_t *cur_acm_reg,acm_reg_table_t *target_acm_reg) {
	int index = 0;

	if (NULL == cur_acm_reg || NULL == target_acm_reg) {
		return;
	}

	for (index = 0;index < ACM_HUE_SIZE;index++) {
		cur_acm_reg->acm_lut_hue_table[index] = cal_hue_value(cur_acm_reg->acm_lut_hue_table[index],target_acm_reg->acm_lut_hue_table[index],count);
	}
	for (index = 0;index < ACM_SATA_SIZE;index++) {
		cur_acm_reg->acm_lut_sata_table[index] = cal_sata_value(cur_acm_reg->acm_lut_sata_table[index],target_acm_reg->acm_lut_sata_table[index],count);
	}
	for (index = 0;index < ACM_SATR_SIZE;index++) {
		cur_acm_reg->acm_lut_satr0_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr0_table[index],target_acm_reg->acm_lut_satr0_table[index],count);
		cur_acm_reg->acm_lut_satr1_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr1_table[index],target_acm_reg->acm_lut_satr1_table[index],count);
		cur_acm_reg->acm_lut_satr2_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr2_table[index],target_acm_reg->acm_lut_satr2_table[index],count);
		cur_acm_reg->acm_lut_satr3_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr3_table[index],target_acm_reg->acm_lut_satr3_table[index],count);
		cur_acm_reg->acm_lut_satr4_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr4_table[index],target_acm_reg->acm_lut_satr4_table[index],count);
		cur_acm_reg->acm_lut_satr5_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr5_table[index],target_acm_reg->acm_lut_satr5_table[index],count);
		cur_acm_reg->acm_lut_satr6_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr6_table[index],target_acm_reg->acm_lut_satr6_table[index],count);
		cur_acm_reg->acm_lut_satr7_table[index] = cal_satr_value(cur_acm_reg->acm_lut_satr7_table[index],target_acm_reg->acm_lut_satr7_table[index],count);
	}
}

/*lint -e838*/
void hisi_effect_color_dimming_acm_reg_set(struct hisi_fb_data_type *hisifd, acm_reg_t *cur_acm_reg) {
	acm_reg_table_t target_acm_reg = {0};
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL!");
		return;
	}

	if (NULL == cur_acm_reg) {
		HISI_FB_ERR("cur_acm_reg is NULL!");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (SCENE_MODE_GALLERY == hisifd->user_scene_mode || SCENE_MODE_DEFAULT == hisifd->user_scene_mode) {
		target_acm_reg.acm_lut_hue_table = pinfo->acm_lut_hue_table;
		target_acm_reg.acm_lut_sata_table = pinfo->acm_lut_sata_table;
		target_acm_reg.acm_lut_satr0_table = pinfo->acm_lut_satr0_table;
		target_acm_reg.acm_lut_satr1_table = pinfo->acm_lut_satr1_table;
		target_acm_reg.acm_lut_satr2_table = pinfo->acm_lut_satr2_table;
		target_acm_reg.acm_lut_satr3_table = pinfo->acm_lut_satr3_table;
		target_acm_reg.acm_lut_satr4_table = pinfo->acm_lut_satr4_table;
		target_acm_reg.acm_lut_satr5_table = pinfo->acm_lut_satr5_table;
		target_acm_reg.acm_lut_satr6_table = pinfo->acm_lut_satr6_table;
		target_acm_reg.acm_lut_satr7_table = pinfo->acm_lut_satr7_table;
	} else {
		target_acm_reg.acm_lut_hue_table = pinfo->video_acm_lut_hue_table;
		target_acm_reg.acm_lut_sata_table = pinfo->video_acm_lut_sata_table;
		target_acm_reg.acm_lut_satr0_table = pinfo->video_acm_lut_satr0_table;
		target_acm_reg.acm_lut_satr1_table = pinfo->video_acm_lut_satr1_table;
		target_acm_reg.acm_lut_satr2_table = pinfo->video_acm_lut_satr2_table;
		target_acm_reg.acm_lut_satr3_table = pinfo->video_acm_lut_satr3_table;
		target_acm_reg.acm_lut_satr4_table = pinfo->video_acm_lut_satr4_table;
		target_acm_reg.acm_lut_satr5_table = pinfo->video_acm_lut_satr5_table;
		target_acm_reg.acm_lut_satr6_table = pinfo->video_acm_lut_satr6_table;
		target_acm_reg.acm_lut_satr7_table = pinfo->video_acm_lut_satr7_table;
	}

	cal_cur_acm_reg(hisifd->dimming_count,cur_acm_reg,&target_acm_reg);
}

/*lint -e838*/
void hisi_effect_color_dimming_acm_reg_init(struct hisi_fb_data_type *hisifd) {//add dimming reg init
	struct hisi_panel_info *pinfo = NULL;
	acm_reg_t *cur_acm_reg = NULL;
	int index = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd, null pointer warning.\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	cur_acm_reg = &(hisifd->acm_reg);

	if (SCENE_MODE_GALLERY == hisifd->user_scene_mode || SCENE_MODE_DEFAULT == hisifd->user_scene_mode) {
		for (index = 0;index < ACM_HUE_SIZE;index++) {
			cur_acm_reg->acm_lut_hue_table[index] = pinfo->acm_lut_hue_table[index];
		}
		for (index = 0;index < ACM_SATA_SIZE;index++) {
			cur_acm_reg->acm_lut_sata_table[index] = pinfo->acm_lut_sata_table[index];
		}
		for (index = 0;index < ACM_SATR_SIZE;index++) {
			cur_acm_reg->acm_lut_satr0_table[index] = pinfo->acm_lut_satr0_table[index];
			cur_acm_reg->acm_lut_satr1_table[index] = pinfo->acm_lut_satr1_table[index];
			cur_acm_reg->acm_lut_satr2_table[index] = pinfo->acm_lut_satr2_table[index];
			cur_acm_reg->acm_lut_satr3_table[index] = pinfo->acm_lut_satr3_table[index];
			cur_acm_reg->acm_lut_satr4_table[index] = pinfo->acm_lut_satr4_table[index];
			cur_acm_reg->acm_lut_satr5_table[index] = pinfo->acm_lut_satr5_table[index];
			cur_acm_reg->acm_lut_satr6_table[index] = pinfo->acm_lut_satr6_table[index];
			cur_acm_reg->acm_lut_satr7_table[index] = pinfo->acm_lut_satr7_table[index];
		}
	} else {
		for (index = 0;index < ACM_HUE_SIZE;index++) {
			cur_acm_reg->acm_lut_hue_table[index] = pinfo->video_acm_lut_hue_table[index];
		}
		for (index = 0;index < ACM_SATA_SIZE;index++) {
			cur_acm_reg->acm_lut_sata_table[index] = pinfo->video_acm_lut_sata_table[index];
		}
		for (index = 0;index < ACM_SATR_SIZE;index++) {
			cur_acm_reg->acm_lut_satr0_table[index] = pinfo->video_acm_lut_satr0_table[index];
			cur_acm_reg->acm_lut_satr1_table[index] = pinfo->video_acm_lut_satr1_table[index];
			cur_acm_reg->acm_lut_satr2_table[index] = pinfo->video_acm_lut_satr2_table[index];
			cur_acm_reg->acm_lut_satr3_table[index] = pinfo->video_acm_lut_satr3_table[index];
			cur_acm_reg->acm_lut_satr4_table[index] = pinfo->video_acm_lut_satr4_table[index];
			cur_acm_reg->acm_lut_satr5_table[index] = pinfo->video_acm_lut_satr5_table[index];
			cur_acm_reg->acm_lut_satr6_table[index] = pinfo->video_acm_lut_satr6_table[index];
			cur_acm_reg->acm_lut_satr7_table[index] = pinfo->video_acm_lut_satr7_table[index];
		}
	}
}

#define ARSR1P_MIN_SIZE 16
#define ARSR1P_MAX_SIZE 8192
#define ARSR1P_MAX_SRC_WIDTH_SIZE 3840
static int set_arsr1p_param(struct hisi_fb_data_type *hisifd, dss_arsr1p_t *post_scf, struct arsr1p_info *arsr1p_param, dss_overlay_t *pov_req)
{
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &(hisifd->panel_info);

	if (!hisifd->effect_ctl.arsr1p_sharp_support)
	{
		HISI_FB_DEBUG("hisifd, arsr1p not support.\n");
		return 0;
	}

	if ((pov_req->res_updt_rect.w != pinfo->xres)
		|| (pov_req->res_updt_rect.h != pinfo->yres)) {
		struct arsr1p_info *arsr1p_rog = &(hisifd->effect_info.arsr1p[1]);
		if (!hisifd->effect_info.arsr1p_rog_initialized) {
			arsr1p_rog_init(arsr1p_rog);
			hisifd->effect_info.arsr1p_rog_initialized = true;
		}
		post_scf->skin_thres_y = arsr1p_rog->skin_thres_y;
		post_scf->skin_thres_u = arsr1p_rog->skin_thres_u;
		post_scf->skin_thres_v = arsr1p_rog->skin_thres_v;
		post_scf->skin_expected = arsr1p_rog->skin_expected;
		post_scf->skin_cfg = arsr1p_rog->skin_cfg;
		post_scf->shoot_cfg1 = arsr1p_rog->shoot_cfg1;
		post_scf->shoot_cfg2 = arsr1p_rog->shoot_cfg2;
		post_scf->sharp_cfg3 = arsr1p_rog->sharp_cfg3;
		post_scf->sharp_cfg4 = arsr1p_rog->sharp_cfg4;
		post_scf->sharp_cfg5 = arsr1p_rog->sharp_cfg5;
		post_scf->sharp_cfg6 = arsr1p_rog->sharp_cfg6;
		post_scf->sharp_cfg7 = arsr1p_rog->sharp_cfg7;
		post_scf->sharp_cfg8 = arsr1p_rog->sharp_cfg8;
		post_scf->sharp_cfg9 = arsr1p_rog->sharp_cfg9;
		post_scf->sharp_cfg10 = arsr1p_rog->sharp_cfg10;
		post_scf->sharp_cfg11 = arsr1p_rog->sharp_cfg11;
		post_scf->diff_ctrl = arsr1p_rog->diff_ctrl;

		post_scf->shoot_cfg2 = arsr1p_rog->shoot_cfg2;
		post_scf->shoot_cfg3 = arsr1p_rog->shoot_cfg3;
		post_scf->sharp_cfg1_h = arsr1p_rog->sharp_cfg1_h;
		post_scf->sharp_cfg1_l = arsr1p_rog->sharp_cfg1_l;
		post_scf->sharp_cfg2_h = arsr1p_rog->sharp_cfg2_h;
		post_scf->sharp_cfg2_l = arsr1p_rog->sharp_cfg2_l;
		post_scf->sharp_cfg6_cut = arsr1p_rog->sharp_cfg6_cut;
		post_scf->sharp_cfg7_ratio = arsr1p_rog->sharp_cfg7_ratio;
		post_scf->skin_slop_y = arsr1p_rog->skin_slop_y;
		post_scf->skin_slop_u = arsr1p_rog->skin_slop_u;
		post_scf->skin_slop_v = arsr1p_rog->skin_slop_v;
		post_scf->force_clk_on_cfg = set_bits32(post_scf->force_clk_on_cfg, 0x0, 32, 0);
	} else {
		post_scf->mode = set_bits32(post_scf->mode, ~arsr1p_param->enable, 1, 0);
		post_scf->mode = set_bits32(post_scf->mode, arsr1p_param->sharp_enable, 1, 1);
		post_scf->mode = set_bits32(post_scf->mode, arsr1p_param->shoot_enable, 1, 2);
		post_scf->mode = set_bits32(post_scf->mode, arsr1p_param->skin_enable, 1, 3);

		post_scf->skin_thres_y = arsr1p_param->skin_thres_y;
		post_scf->skin_thres_u = arsr1p_param->skin_thres_u;
		post_scf->skin_thres_v = arsr1p_param->skin_thres_v;
		post_scf->skin_expected = arsr1p_param->skin_expected;
		post_scf->skin_cfg = arsr1p_param->skin_cfg;

		post_scf->shoot_cfg1 = arsr1p_param->shoot_cfg1;
		post_scf->shoot_cfg2 = arsr1p_param->shoot_cfg2;
		post_scf->shoot_cfg3 = arsr1p_param->shoot_cfg3;
		post_scf->sharp_cfg1_h = arsr1p_param->sharp_cfg1_h;
		post_scf->sharp_cfg1_l = arsr1p_param->sharp_cfg1_l;
		post_scf->sharp_cfg2_h = arsr1p_param->sharp_cfg2_h;
		post_scf->sharp_cfg2_l = arsr1p_param->sharp_cfg2_l;
		post_scf->sharp_cfg3 = arsr1p_param->sharp_cfg3;
		post_scf->sharp_cfg4 = arsr1p_param->sharp_cfg4;
		post_scf->sharp_cfg5 = arsr1p_param->sharp_cfg5;
		post_scf->sharp_cfg6 = arsr1p_param->sharp_cfg6;
		post_scf->sharp_cfg6_cut = arsr1p_param->sharp_cfg6_cut;
		post_scf->sharp_cfg7 = arsr1p_param->sharp_cfg7;
		post_scf->sharp_cfg7_ratio = arsr1p_param->sharp_cfg7_ratio;
		post_scf->sharp_cfg8 = arsr1p_param->sharp_cfg8;
		post_scf->sharp_cfg9 = arsr1p_param->sharp_cfg9;
		post_scf->sharp_cfg10 = arsr1p_param->sharp_cfg10;
		post_scf->sharp_cfg11 = arsr1p_param->sharp_cfg11;
		post_scf->diff_ctrl = arsr1p_param->diff_ctrl;
		post_scf->skin_slop_y = arsr1p_param->skin_slop_y;
		post_scf->skin_slop_u = arsr1p_param->skin_slop_u;
		post_scf->skin_slop_v = arsr1p_param->skin_slop_v;

		hisifd->effect_updated_flag.arsr1p_effect_updated = false;
	}

	return 0;
}

int hisi_arsr1p_set_rect(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req,dss_arsr1p_t *post_scf, struct hisi_panel_info *pinfo)
{
	int32_t ihinc = 0;
	int32_t ivinc = 0;
	int32_t ihleft = 0;
	int32_t ihright = 0;
	int32_t ihleft1 = 0;
	int32_t ihright1 = 0;
	int32_t ivtop = 0;
	int32_t ivbottom = 0;
	int32_t extraw = 0;
	int32_t extraw_left = 0;
	int32_t extraw_right = 0;
	dss_rect_t src_rect = {0};
	dss_rect_t dst_rect = {0};

	/*if((pov_req->res_updt_rect.w != hisifd->ov_req_prev.res_updt_rect.w)
	|| (pov_req->res_updt_rect.h != hisifd->ov_req_prev.res_updt_rect.h))
	{
		hisifd->ov_req_prev.res_updt_rect = pov_req->res_updt_rect;
	}*/

	if (pov_req->dirty_rect.w == 0 || pov_req->dirty_rect.h == 0) {
		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.w = pinfo->xres;
		dst_rect.h = pinfo->yres;
	} else {
		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.w = pov_req->dirty_rect.w;
		dst_rect.h = pov_req->dirty_rect.h;
	}

	if (((pov_req->dirty_rect.w > 0) && (pov_req->dirty_rect.h > 0)) ||
		((pov_req->res_updt_rect.w == 0) || (pov_req->res_updt_rect.h == 0))) {
		src_rect = dst_rect;
	} else {
		src_rect = pov_req->res_updt_rect;
	}

	hisifd->dss_module.post_scf_used = 1;
	if ((src_rect.w < ARSR1P_MIN_SIZE) || (src_rect.h < ARSR1P_MIN_SIZE)
		|| (src_rect.w > ARSR1P_MAX_SRC_WIDTH_SIZE) || (src_rect.h > ARSR1P_MAX_SIZE)
		|| (dst_rect.w > ARSR1P_MAX_SIZE) || (dst_rect.h > ARSR1P_MAX_SIZE)) {
		HISI_FB_ERR("fb%d, invalid input size: src_rect(%d,%d,%d,%d) should be larger than 16*16, less than 3840*8192!\n"
			"invalid output size: dst_rect(%d,%d,%d,%d) should be less than 8192*8192!\n",
			hisifd->index,
			src_rect.x, src_rect.y, src_rect.w, src_rect.h,
			dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
		// bypass
		post_scf->mode = 0x1;
		return -1;
	}

	ihinc = (int32_t)(ARSR1P_INC_FACTOR * (uint64_t)src_rect.w / dst_rect.w);
	ivinc = (int32_t)(ARSR1P_INC_FACTOR * (uint64_t)src_rect.h / dst_rect.h);

	// 0x2000<=ihinc<=ARSR1P_INC_FACTOR; 0x2000<=ivinc<=ARSR1P_INC_FACTOR;
	if ((ihinc < 0x2000) || (ihinc > ARSR1P_INC_FACTOR)
		|| (ivinc < 0x2000) || (ivinc > ARSR1P_INC_FACTOR)) {
		HISI_FB_ERR("invalid ihinc(0x%x), ivinc(0x%x)!\n", ihinc, ivinc);
		// bypass
		post_scf->mode = 0x1;
		return -1;
	}

	extraw = (8 * ARSR1P_INC_FACTOR) / ihinc;
	extraw_left = (extraw % 2) ? (extraw + 1) : (extraw);
	extraw = (2 * ARSR1P_INC_FACTOR) / ihinc;
	extraw_right = (extraw % 2) ? (extraw + 1) : (extraw);

	//ihleft1 = (startX_o * ihinc) - (ov_startX0 << 16)
	ihleft1 = dst_rect.x * ihinc - src_rect.x * ARSR1P_INC_FACTOR;
	if (ihleft1 < 0)
		ihleft1 = 0;
	//ihleft = ihleft1 - even(8 * 65536 / ihinc) * ihinc;
	ihleft = ihleft1 - extraw_left * ihinc;
	if (ihleft < 0)
		ihleft = 0;

	//ihright1 = ihleft1 + (oww-1) * ihinc
	ihright1 = ihleft1 + (dst_rect.w - 1) * ihinc;
	//ihright = ihright1 + even(2 * 65536/ihinc) * ihinc
	ihright = ihright1 + extraw_right * ihinc;
	//ihright >= img_width * ihinc
	if (ihright >= src_rect.w * ARSR1P_INC_FACTOR)
		ihright = src_rect.w * ARSR1P_INC_FACTOR - 1;

	//ivtop = (startY_o * ivinc) - (ov_startY0<<16)
	ivtop = dst_rect.y * ivinc - src_rect.y * ARSR1P_INC_FACTOR;
	if (ivtop < 0)
		ivtop = 0;
	//ivbottom = ivtop + (ohh - 1) * ivinc
	ivbottom = ivtop + (dst_rect.h - 1) * ivinc;
	//ivbottom >= img_height * ivinc
	if (ivbottom >= src_rect.h * ARSR1P_INC_FACTOR)
		ivbottom = src_rect.h * ARSR1P_INC_FACTOR - 1;

	//(ihleft1 - ihleft) % (ihinc) == 0;
	if ((ihleft1 - ihleft) % (ihinc)) {
		HISI_FB_ERR("(ihleft1(%d)-ihleft(%d))  ihinc(%d) != 0, invalid!\n",
			ihleft1, ihleft, ihinc);
		post_scf->mode = 0x1;
		return -1;
	}

	//(ihright1 - ihleft1) % ihinc == 0;
	if ((ihright1 - ihleft1) % ihinc) {
		HISI_FB_ERR("(ihright1(%d)-ihleft1(%d))  ihinc(%d) != 0, invalid!\n",
			ihright1, ihleft1, ihinc);
		post_scf->mode = 0x1;
		return -1;
	}

	post_scf->mode &= 0xFFFFFFFE; /*cancel arsr1p bypass*/
	post_scf->mode |= 0xc;/*skinctrl, shootdetect*/
	post_scf->mode |= 0x20;/*enable direction*/
	post_scf->mode |= 0x2;/*enable sharpness*/
	if ((ihinc < ARSR1P_INC_FACTOR) || (ivinc < ARSR1P_INC_FACTOR)) {
		post_scf->mode |= 0x10;/*enable diintplen*/
	} else {
		post_scf->mode |= 0x40;/*only sharp, enable nointplen*/
	}


	post_scf->ihleft = set_bits32(post_scf->ihleft, ihleft, 32, 0);
	post_scf->ihright = set_bits32(post_scf->ihright, ihright, 32, 0);
	post_scf->ihleft1 = set_bits32(post_scf->ihleft1, ihleft1, 32, 0);
	post_scf->ihright1 = set_bits32(post_scf->ihright1, ihright1, 32, 0);
	post_scf->ivtop = set_bits32(post_scf->ivtop, ivtop, 32, 0);
	post_scf->ivbottom = set_bits32(post_scf->ivbottom, ivbottom, 32, 0);
	post_scf->ihinc = set_bits32(post_scf->ihinc, ihinc, 32, 0);
	post_scf->ivinc = set_bits32(post_scf->ivinc, ivinc, 32, 0);

	post_scf->dpp_img_size_bef_sr = set_bits32(post_scf->dpp_img_size_bef_sr,
		(DSS_HEIGHT((uint32_t)src_rect.h) << 16) | DSS_WIDTH((uint32_t)src_rect.w), 32, 0);
	post_scf->dpp_img_size_aft_sr = set_bits32(post_scf->dpp_img_size_aft_sr,
		(DSS_HEIGHT((uint32_t)dst_rect.h) << 16) | DSS_WIDTH((uint32_t)dst_rect.w), 32, 0);
	post_scf->dpp_used = 1;

	return 0;
}

int hisi_effect_arsr1p_config(struct hisi_fb_data_type *hisifd, dss_overlay_t *pov_req)
{
	struct arsr1p_info *arsr1p_param = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_arsr1p_t *post_scf = NULL;
	int ret = 0;

	if ((NULL == hisifd) || (NULL == pov_req)) {
		HISI_FB_ERR("hisifd or pov_req is NULL!");
		return -EINVAL;
	}

	if ((!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF))
		|| (PRIMARY_PANEL_IDX != hisifd->index)){
		return 0;
	}

	pinfo = &(hisifd->panel_info);
	arsr1p_param = &(hisifd->effect_info.arsr1p[0]);
	post_scf = &(hisifd->dss_module.post_scf);


	if ((arsr1p_param->enable != 1) &&
		((pov_req->res_updt_rect.w == pinfo->xres)
		&& (pov_req->res_updt_rect.h == pinfo->yres))) {
		hisifd->dss_module.post_scf_used = 1;
		post_scf->mode = 0x1;
		return 0;
	}

	if ((pov_req->res_updt_rect.w < 0) || (pov_req->res_updt_rect.h < 0)) {
		HISI_FB_ERR("fb%d, res_updt_rect[%d,%d, %d,%d] is invalid!\n", hisifd->index,
			pov_req->res_updt_rect.x, pov_req->res_updt_rect.y,
			pov_req->res_updt_rect.w, pov_req->res_updt_rect.h);
		return -EINVAL;
	}

	if (!spin_can_lock(&hisifd->effect_lock)) {
		HISI_FB_DEBUG("fb%d, dss effect param is being updated, delay set reg to next frame!\n", hisifd->index);
		return 0;
	}

	spin_lock(&hisifd->effect_lock);

	/*update arsr1p rect*/
	if(hisi_arsr1p_set_rect(hisifd, pov_req, post_scf,pinfo)){
		ret = -EINVAL;
		goto err_return;
	}

	/*update arsr1p effect para*/
	if (set_arsr1p_param(hisifd, post_scf, arsr1p_param, pov_req)) {
		HISI_FB_ERR("fb%d, failed to set arsr1p param.\n", hisifd->index);
		ret = -EINVAL;
		goto err_return;
	}

err_return:
	spin_unlock(&hisifd->effect_lock);
	return ret;
}

static void hisi_effect_arsr2p_update(struct hisi_fb_data_type *hisifd, struct dss_arsr2p *arsr2p) {
	if (hisifd == NULL || arsr2p == NULL) {
		HISI_FB_ERR("hisifd or arsr2p is null. return\n");
		return;
	}

	if (hisifd->effect_info.arsr2p[0].update == 1) {
		memcpy(&(arsr2p->arsr2p_effect), &(hisifd->effect_info.arsr2p[0]), sizeof(struct arsr2p_info));
		hisifd->effect_info.arsr2p[0].update = 0;
	}

	if (hisifd->effect_info.arsr2p[1].update == 1) {
		memcpy(&(arsr2p->arsr2p_effect_scale_up), &(hisifd->effect_info.arsr2p[1]), sizeof(struct arsr2p_info));
		hisifd->effect_info.arsr2p[1].update = 0;
	}

	if (hisifd->effect_info.arsr2p[2].update == 1) {
		memcpy(&(arsr2p->arsr2p_effect_scale_down), &(hisifd->effect_info.arsr2p[2]), sizeof(struct arsr2p_info));
		hisifd->effect_info.arsr2p[2].update = 0;
	}
}

int hisi_effect_arsr2p_config(struct arsr2p_info *arsr2p_effect_dst, int ih_inc, int iv_inc)
{
	struct hisi_fb_data_type *hisifd_primary = NULL;
	struct dss_arsr2p *arsr2p = NULL;

	hisifd_primary = hisifd_list[PRIMARY_PANEL_IDX];
	if (NULL == hisifd_primary) {
		HISI_FB_ERR("hisifd_primary is NULL pointer, return!\n");
		return -EINVAL;
	}

	if (NULL == arsr2p_effect_dst) {
		HISI_FB_ERR("arsr2p_effect_dst is NULL pointer, return!\n");
		return -EINVAL;
	}

	arsr2p = &(hisifd_primary->dss_module_default.arsr2p[DSS_RCHN_V0]);

	hisi_effect_arsr2p_update(hisifd_primary, arsr2p);

	if ((ih_inc == ARSR2P_INC_FACTOR) && (iv_inc == ARSR2P_INC_FACTOR)) {
		memcpy(arsr2p_effect_dst, &(arsr2p->arsr2p_effect), sizeof(struct arsr2p_info));
	} else if ((ih_inc < ARSR2P_INC_FACTOR) || (iv_inc < ARSR2P_INC_FACTOR)) {
		memcpy(arsr2p_effect_dst, &(arsr2p->arsr2p_effect_scale_up), sizeof(struct arsr2p_info));
	} else {
		memcpy(arsr2p_effect_dst, &(arsr2p->arsr2p_effect_scale_down), sizeof(struct arsr2p_info));
	}

	return 0;
}
int hisifb_ce_service_enable_hiace(struct fb_info *info, void __user *argp)
{
	struct hisi_panel_info *pinfo = NULL;
	dss_display_effect_ce_t *ce_ctrl = NULL;
	dss_ce_info_t *ce_info = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	dss_display_effect_metadata_t *metadata_ctrl = NULL;
	int enable = 0;
	int mode = 0;
	int ret;
	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}
	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}
	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] HIACE is not supported!\n");
		return -1;
	}
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ce_ctrl = &(hisifd->ce_ctrl);
		ce_info = &(hisifd->hiace_info);
		metadata_ctrl = &(hisifd->metadata_ctrl);
	} else {
		HISI_FB_ERR("[effect] fb%d, not support!", hisifd->index);
		return -1;
	}
	ret = (int)copy_from_user(&enable, argp, sizeof(enable));
	if (ret) {
		HISI_FB_ERR("[effect] arg is invalid");
		return -EINVAL;
	}
	mode = enable;
	if (mode < 0) {
		mode = 0;
	} else if (mode >= CE_MODE_COUNT) {
		mode = CE_MODE_COUNT - 1;
	}
	if (mode != ce_ctrl->ctrl_ce_mode) {
		mutex_lock(&(ce_ctrl->ctrl_lock));
		ce_ctrl->ctrl_ce_mode = mode;
		mutex_unlock(&(ce_ctrl->ctrl_lock));
		if (mode == CE_MODE_DISABLE && hisifd->panel_power_on) {
			ce_info->gradual_frames = EFFECT_GRADUAL_REFRESH_FRAMES;
			ce_info->to_stop_hdr = true;
		}
		enable_hiace(hisifd, mode);
	}
	return 0;
}
int hisifb_get_reg_val(struct fb_info *info, void __user *argp) {
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_reg reg;
	uint32_t addr;
	int ret = 0;
	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}
	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}
	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}

	if (hisifd->panel_power_on == false) {
		HISI_FB_ERR("[effect] panel power off\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Don't support HIACE\n");
		return -EINVAL;
	}
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");
	ret = (int)copy_from_user(&reg, argp, sizeof(struct dss_reg));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return -2;
	}

	switch(reg.tag) {
		case TAG_ARSR_1P_ENABLE:
			addr = DSS_POST_SCF_OFFSET + ARSR_POST_MODE;
			break;
		case TAG_LCP_XCC_ENABLE:
			addr = DSS_DPP_XCC_OFFSET + XCC_EN;
			break;
		case TAG_LCP_GMP_ENABLE:
			addr = DSS_DPP_GMP_OFFSET + GMP_EN;
			break;
		case TAG_LCP_IGM_ENABLE:
			addr = DSS_DPP_DEGAMMA_OFFSET + DEGAMA_EN;
			break;
		case TAG_GAMMA_ENABLE:
			addr = DSS_DPP_GAMA_OFFSET + GAMA_EN;
			break;
		case TAG_HIACE_LHIST_SFT:
			addr = DSS_HI_ACE_OFFSET + DPE_LHIST_SFT;
			break;
		default:
			HISI_FB_ERR("[effect] invalid tag : %u", reg.tag);
			return -EINVAL;
	}

	reg.value = (uint32_t)inp32(hisifd->dss_base + addr);

	ret = (int)copy_to_user(argp, &reg, sizeof(struct dss_reg));
	if (ret) {
		HISI_FB_ERR("[effect] copy_to_user failed(param)! ret=%d.\n", ret);
		ret = -EINVAL;
	}
	return 0;
}
int hisifb_ce_service_set_param(struct fb_info *info, void __user *argp) {
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int ret = 0;

	if (runmode_is_factory()) {
		return ret;
	}

	if (NULL == info) {
		HISI_FB_ERR("info is NULL\n");
		return -EINVAL;
	}
	if (NULL == argp) {
		HISI_FB_ERR("[effect] argp is NULL\n");
		return -EINVAL;
	}
	hisifd = (struct hisi_fb_data_type *)info->par;
	if (NULL == hisifd) {
		HISI_FB_ERR("[effect] hisifd is NULL\n");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);
	if (!pinfo->hiace_support) {
		EFFECT_DEBUG_LOG(DEBUG_EFFECT_ENTRY, "[effect] Don't support HIACE\n");
		return -EINVAL;
	}
	EFFECT_DEBUG_LOG(DEBUG_EFFECT_FRAME, "[effect] step in\n");
	ret = (int)copy_from_user(&(hisifd->effect_info.hiace), argp, sizeof(struct hiace_info));
	if (ret) {
		HISI_FB_ERR("[effect] copy_from_user(param) failed! ret=%d.\n", ret);
		return -2;
	}
	hisifd->effect_updated_flag.hiace_effect_updated = true;
	return ret;
}
static int set_hiace_param(struct hisi_fb_data_type *hisifd) {
	char __iomem *hiace_base = NULL;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -1;
	}
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_DEBUG("[effect] fb%d, not support!", hisifd->index);
		return 0;
	}
	if (hisifd->fb_shutdown == true || hisifd->panel_power_on == false) {
		HISI_FB_ERR("[effect] fb_shutdown or panel power down");
		return -EINVAL;
	}
	if (hisifd->effect_updated_flag.hiace_effect_updated) {
		set_reg(hiace_base + DPE_HALF_BLOCK_INFO, hisifd->effect_info.hiace.half_block_info, 32, 0);
		set_reg(hiace_base + DPE_XYWEIGHT, hisifd->effect_info.hiace.xyweight, 32, 0);
		//set_reg(hiace_base + DPE_LHIST_SFT, hisifd->effect_info.hiace.lhist_sft, 32, 0);
		set_reg(hiace_base + DPE_ROI_START_POINT, hisifd->effect_info.hiace.roi_start_point, 32, 0);
		set_reg(hiace_base + DPE_ROI_WIDTH_HIGH, hisifd->effect_info.hiace.roi_width_high, 32, 0);
		set_reg(hiace_base + DPE_ROI_MODE_CTRL, hisifd->effect_info.hiace.roi_mode_ctrl, 32, 0);
		set_reg(hiace_base + DPE_ROI_HIST_STAT_MODE, hisifd->effect_info.hiace.roi_hist_stat_mode, 32, 0);
		set_reg(hiace_base + DPE_HUE, hisifd->effect_info.hiace.hue, 32, 0);
		set_reg(hiace_base + DPE_SATURATION, hisifd->effect_info.hiace.saturation, 32, 0);
		set_reg(hiace_base + DPE_VALUE, hisifd->effect_info.hiace.value, 32, 0);
		set_reg(hiace_base + DPE_SKIN_GAIN, hisifd->effect_info.hiace.skin_gain, 32, 0);
		set_reg(hiace_base + DPE_UP_LOW_TH, hisifd->effect_info.hiace.up_low_th, 32, 0);
		set_reg(hiace_base + DPE_RGB_BLEND_WEIGHT, hisifd->effect_info.hiace.rgb_blend_weight, 32, 0);
		set_reg(hiace_base + DPE_FNA_STATISTIC, hisifd->effect_info.hiace.fna_statistic, 32, 0);
		set_reg(hiace_base + DPE_UP_CNT, hisifd->effect_info.hiace.up_cnt, 32, 0);
		set_reg(hiace_base + DPE_LOW_CNT, hisifd->effect_info.hiace.low_cnt, 32, 0);
		set_reg(hiace_base + DPE_SUM_SATURATION, hisifd->effect_info.hiace.sum_saturation, 32, 0);
		//set_reg(hiace_base + DPE_LHIST_EN, hisifd->effect_info.hiace.lhist_en, 32, 0);
		set_reg(hiace_base + DPE_GAMMA_W, hisifd->effect_info.hiace.gamma_w, 32, 0);
		set_reg(hiace_base + DPE_GAMMA_R, hisifd->effect_info.hiace.gamma_r, 32, 0);
		set_reg(hiace_base + DPE_FNA_ADDR, hisifd->effect_info.hiace.fna_addr, 32, 0);
		set_reg(hiace_base + DPE_FNA_DATA, hisifd->effect_info.hiace.fna_data, 32, 0);
		set_reg(hiace_base + DPE_UPDATE_FNA, hisifd->effect_info.hiace.update_fna, 32, 0);
		set_reg(hiace_base + DPE_FNA_VALID, hisifd->effect_info.hiace.fna_valid, 32, 0);
		set_reg(hiace_base + DPE_DB_PIPE_CFG, hisifd->effect_info.hiace.db_pipe_cfg, 32, 0);
		set_reg(hiace_base + DPE_DB_PIPE_EXT_WIDTH, hisifd->effect_info.hiace.db_pipe_ext_width, 32, 0);
		set_reg(hiace_base + DPE_DB_PIPE_FULL_IMG_WIDTH, hisifd->effect_info.hiace.db_pipe_full_img_width, 32, 0);
		set_reg(hiace_base + DPE_BYPASS_NR, hisifd->effect_info.hiace.bypass_nr, 32, 0);
		set_reg(hiace_base + DPE_S3_SOME_BRIGHTNESS01, hisifd->effect_info.hiace.s3_some_brightness01, 32, 0);
		set_reg(hiace_base + DPE_S3_SOME_BRIGHTNESS23, hisifd->effect_info.hiace.s3_some_brightness23, 32, 0);
		set_reg(hiace_base + DPE_S3_SOME_BRIGHTNESS4, hisifd->effect_info.hiace.s3_some_brightness4, 32, 0);
		set_reg(hiace_base + DPE_S3_MIN_MAX_SIGMA, hisifd->effect_info.hiace.s3_min_max_sigma, 32, 0);
		set_reg(hiace_base + DPE_S3_GREEN_SIGMA03, hisifd->effect_info.hiace.s3_green_sigma03, 32, 0);
		set_reg(hiace_base + DPE_S3_GREEN_SIGMA45, hisifd->effect_info.hiace.s3_green_sigma45, 32, 0);
		set_reg(hiace_base + DPE_S3_RED_SIGMA03, hisifd->effect_info.hiace.s3_red_sigma03, 32, 0);
		set_reg(hiace_base + DPE_S3_RED_SIGMA45, hisifd->effect_info.hiace.s3_red_sigma45, 32, 0);
		set_reg(hiace_base + DPE_S3_BLUE_SIGMA03, hisifd->effect_info.hiace.s3_blue_sigma03, 32, 0);
		set_reg(hiace_base + DPE_S3_BLUE_SIGMA45, hisifd->effect_info.hiace.s3_blue_sigma45, 32, 0);
		set_reg(hiace_base + DPE_S3_WHITE_SIGMA03, hisifd->effect_info.hiace.s3_white_sigma03, 32, 0);
		set_reg(hiace_base + DPE_S3_WHITE_SIGMA45, hisifd->effect_info.hiace.s3_white_sigma45, 32, 0);
		set_reg(hiace_base + DPE_S3_FILTER_LEVEL, hisifd->effect_info.hiace.s3_filter_level, 32, 0);
		set_reg(hiace_base + DPE_S3_SIMILARITY_COEFF, hisifd->effect_info.hiace.s3_similarity_coeff, 32, 0);
		set_reg(hiace_base + DPE_S3_V_FILTER_WEIGHT_ADJ, hisifd->effect_info.hiace.s3_v_filter_weight_adj, 32, 0);
		set_reg(hiace_base + DPE_S3_HUE, hisifd->effect_info.hiace.s3_hue, 32, 0);
		set_reg(hiace_base + DPE_S3_SATURATION, hisifd->effect_info.hiace.s3_saturation, 32, 0);
		set_reg(hiace_base + DPE_S3_VALUE, hisifd->effect_info.hiace.s3_value, 32, 0);
		set_reg(hiace_base + DPE_S3_SKIN_GAIN, hisifd->effect_info.hiace.s3_skin_gain, 32, 0);
	}
	hisifd->effect_updated_flag.hiace_effect_updated = false;
	return 0;
}

int hisi_effect_hiace_config(struct hisi_fb_data_type *hisifd) {
	char __iomem *hiace_base = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL!\n");
		return -1;
	}

	pinfo = &(hisifd->panel_info);
	if (pinfo->hiace_support == 0) {
		HISI_FB_DEBUG("[effect] HIACE is not supported!\n");
		return 0;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		hiace_base = hisifd->dss_base + DSS_HI_ACE_OFFSET;
	} else {
		HISI_FB_DEBUG("[effect] fb%d, not support!", hisifd->index);
		return 0;
	}


	if (hiace_enable_status != hisifd->hiace_info.hiace_enable) {
		if (hisifd->hiace_info.hiace_enable) {
			if (hisifd->dirty_region_updt_enable == 0) {
				set_reg(hiace_base + HIACE_BYPASS_ACE, 0x0, 1, 0);
				set_reg(hiace_base + HIACE_INT_STAT, 0x1, 1, 0);
				hiace_enable_status = hisifd->hiace_info.hiace_enable;
			}
		} else {
			set_reg(hiace_base + HIACE_BYPASS_ACE, 0x1, 1, 0);
			hiace_enable_status = hisifd->hiace_info.hiace_enable;
		}
	}

	return set_hiace_param(hisifd);
}
/*lint +e571, +e573, +e737, +e732, +e850, +e730, +e713, +e529, +e574, +e679, +e732, +e845, +e570, +e774*/
//lint +e747, +e838
