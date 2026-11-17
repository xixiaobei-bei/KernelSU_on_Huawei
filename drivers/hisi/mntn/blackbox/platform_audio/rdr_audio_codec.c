/*
 * rdr_audio_codec.c
 *
 * audio codec rdr
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "rdr_audio_codec.h"

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/thread_info.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/vmalloc.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <linux/hisi/da_combine_dsp/da_combine_dsp_misc.h>
#include <linux/hisi/audio_log.h>

#include "rdr_print.h"
#include "rdr_audio_adapter.h"
#include "om.h"

#define LOG_TAG "rdr_audio_codec"
#define MODID_AP_S_PANIC_AUDIO_CODEC 0x80000029 /* must be same with rdr_hisi_platform.h */
#define CODEC_ERR_RECORD_FILE "/data/hisi_logs/codecerr_times.log"
#define CODEC_ERR_REBOOT_THRESHOLD 3

struct rdr_codec_des_s {
	unsigned int modid;
	char *pathname;
	pfn_cb_dump_done dumpdone_cb;

	struct semaphore handler_sem;
	struct semaphore dump_sem;
	struct task_struct *kdump_task;
	struct task_struct *khandler_task;
	struct wakeup_source rdr_wl;
};

static struct rdr_codec_des_s g_priv_des;

static bool (*rdr_audio_get_ap_reset_cfg)(void);

static unsigned char rdr_audio_get_codec_err_times(void)
{
	struct file *fp = NULL;
	ssize_t red_len;
	unsigned char times = 0;
	mm_segment_t fs = 0;

	fp = filp_open(CODEC_ERR_RECORD_FILE, O_RDONLY, 0660);
	if (IS_ERR(fp)) {
		AUDIO_LOGI("open fail");
		return 0;
	}

	fs = get_fs(); /*lint !e501*/
	set_fs(KERNEL_DS); /*lint !e501*/

	vfs_llseek(fp, 0L, SEEK_SET);
	red_len = vfs_read(fp, &times, sizeof(times), &fp->f_pos);
	if (red_len == 0)
		times = 0;

	set_fs(fs);
	filp_close(fp, NULL);

	return times;
}

static void rdr_audio_set_codec_err_times(unsigned char times)
{
	struct file *fp = NULL;
	ssize_t write_len;
	char buf = 0;
	mm_segment_t fs = 0;

	fp = filp_open(CODEC_ERR_RECORD_FILE, O_CREAT | O_RDWR, 0660);
	if (IS_ERR(fp)) {
		AUDIO_LOGE("open fail");
		return;
	}

	fs = get_fs();
	set_fs(KERNEL_DS); /*lint !e501*/

	vfs_llseek(fp, 0L, SEEK_SET);
	write_len = vfs_write(fp, &times, sizeof(times), &(fp->f_pos));
	if (write_len == sizeof(buf))
		vfs_fsync(fp, 0);

	set_fs(fs);
	filp_close(fp, NULL);
}

void rdr_audio_clear_reboot_times(void)
{
	if (rdr_audio_get_codec_err_times() > 0) {
		AUDIO_LOGI("have rebooted before, clear count");
		rdr_audio_set_codec_err_times(0);
	}
}

void rdr_audio_register_get_ap_reset_cfg_cb(bool (*cb)(void))
{
	if (cb != NULL)
		rdr_audio_get_ap_reset_cfg = cb;
}

/*lint -e548*/
void rdr_audio_codec_err_process(void)
{
	unsigned char err_times;

	if (rdr_audio_get_ap_reset_cfg != NULL && rdr_audio_get_ap_reset_cfg()) {
		AUDIO_LOGI("ignore ap reset process");
		return;
	}

	err_times = rdr_audio_get_codec_err_times();
	err_times++;

	if (err_times <= CODEC_ERR_REBOOT_THRESHOLD) {
		AUDIO_LOGE("reboot for error %hhu times, reboot now", err_times);
		rdr_audio_set_codec_err_times(err_times);
		rdr_syserr_process_for_ap((u32)MODID_AP_S_PANIC_AUDIO_CODEC, 0UL, 0UL);
	} else {
		AUDIO_LOGE("reboot for error %hhu times, more than %u times, do not reboot again",
			err_times, CODEC_ERR_REBOOT_THRESHOLD);
	}
}

static void dump_codec(const char *filepath)
{
#ifdef CONFIG_CODEC_DSP_MISC
	WARN_ON(filepath == NULL);
	da_combine_dsp_dump_with_path(filepath);
#endif
}

void rdr_codec_dsp_watchdog_process(void)
{
#ifdef RDR_CODECDSP
	__pm_stay_awake(&g_priv_des.rdr_wl);
	up(&g_priv_des.handler_sem);
#endif
}

static int irq_handler_thread(void *arg)
{
	AUDIO_LOGI("enter blackbox");

	while (!kthread_should_stop()) {
		if (down_interruptible(&g_priv_des.handler_sem)) {
			AUDIO_LOGE("down priv des handler sem fail");
			continue;
		}

		AUDIO_LOGI("enter rdr process for dsp watchdog");
		rdr_system_error((unsigned int)RDR_AUDIO_CODEC_WD_TIMEOUT_MODID, 0, 0);
		AUDIO_LOGI("exit rdr process for dsp watchdog");
	}

	AUDIO_LOGI("exit  blackbox");

	return 0;
}

static int dump_thread(void *arg)
{
	AUDIO_LOGI("enter blackbox");

	while (!kthread_should_stop()) {
		if (down_interruptible(&g_priv_des.dump_sem)) {
			AUDIO_LOGE("down priv des dump sem fail");
			continue;
		}

		AUDIO_LOGD("begin to dump log");
		dump_codec(g_priv_des.pathname);
		AUDIO_LOGD("end dump log");

		if (g_priv_des.dumpdone_cb) {
			AUDIO_LOGD("begin dump dsp done callback, modid: 0x%x",
				g_priv_des.modid);
			g_priv_des.dumpdone_cb(g_priv_des.modid, (unsigned long long)RDR_HIFI);
			AUDIO_LOGD("end dump dsp done callback");
		}
	}

	AUDIO_LOGI("exit blackbox");

	return 0;
}

void rdr_audio_codec_dump(unsigned int modid, char *pathname, pfn_cb_dump_done pfb)
{
	WARN_ON(pathname == NULL);

	AUDIO_LOGI("enter blackbox");

	g_priv_des.modid = modid;
	g_priv_des.dumpdone_cb = pfb;
	g_priv_des.pathname = pathname;

	up(&g_priv_des.dump_sem);

	AUDIO_LOGI("exit blackbox");
}

void rdr_audio_codec_reset(unsigned int modid, unsigned int etype, u64 coreid)
{
	AUDIO_LOGI("enter blackbox");
#ifdef CONFIG_CODEC_DSP_MISC
	da_combine_wtdog_send_event();
#endif

	__pm_relax(&g_priv_des.rdr_wl);

	AUDIO_LOGI("exit blackbox");
}

int rdr_audio_codec_init(void)
{
	AUDIO_LOGI("enter blackbox");

	g_priv_des.modid = ~0;
	g_priv_des.pathname = NULL;
	g_priv_des.dumpdone_cb = NULL;

	wakeup_source_init(&g_priv_des.rdr_wl, "rdr_codechifi");
	sema_init(&g_priv_des.dump_sem, 0);
	sema_init(&g_priv_des.handler_sem, 0);
	g_priv_des.kdump_task = NULL;
	g_priv_des.khandler_task = NULL;

	g_priv_des.kdump_task =
	    kthread_run(dump_thread, NULL, "rdr_codec_hifi_dump_thread");
	if (g_priv_des.kdump_task == NULL) {
		AUDIO_LOGE("create dump thead fail");
		goto create_dump_thead_error;
	}

	g_priv_des.khandler_task =
	    kthread_run(irq_handler_thread, NULL, "rdr_codec_hifi_irq_handler_thread");
	if (g_priv_des.khandler_task == NULL) {
		AUDIO_LOGE("create rdr irq handler thead fail");
		goto create_irq_handler_thead_error;
	}

	AUDIO_LOGI("exit blackbox");

	return 0;

create_irq_handler_thead_error:
	if (g_priv_des.kdump_task) {
		kthread_stop(g_priv_des.kdump_task);
		up(&g_priv_des.dump_sem);
		g_priv_des.kdump_task = NULL;
	}

create_dump_thead_error:
	wakeup_source_trash(&g_priv_des.rdr_wl);

	AUDIO_LOGI("exit blackbox");

	return -1;
}
/*lint +e548*/

void rdr_audio_codec_exit(void)
{
	AUDIO_LOGI("enter blackbox");

	if (g_priv_des.khandler_task) {
		kthread_stop(g_priv_des.khandler_task);
		up(&g_priv_des.handler_sem);
		g_priv_des.khandler_task = NULL;
	}

	if (g_priv_des.kdump_task) {
		kthread_stop(g_priv_des.kdump_task);
		up(&g_priv_des.dump_sem);
		g_priv_des.kdump_task = NULL;
	}

	wakeup_source_trash(&g_priv_des.rdr_wl);

	AUDIO_LOGI("exit blackbox");
}

