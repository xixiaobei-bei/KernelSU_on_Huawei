/* Copyright (c) , Huawei Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

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
#include <linux/platform_device.h>
#include <linux/ion.h>
#include <linux/pm_wakeup.h>
#include <asm/fb.h>
#include "hisi_aod_device.h"
#include "protocol.h"
#include "hisi_fb.h"
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include "contexthub_pm.h"
#include "contexthub_route.h"
#include <asm/cacheflush.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <asm/cachetype.h>
#endif
#ifdef CONFIG_CONTEXTHUB_SHMEM
#include "shmem.h"
#endif


#define HISI_AOD_ION_CLIENT_NAME	"hisi_aod_ion"

#define DIGITS_COUNT (10)
#define AOD_SINGLE_CLOCK_DIGITS_RES_X (15)
#define AOD_SINGLE_CLOCK_DIGITS_RES_Y (32)
#define IPC_MAX_SIZE 128
#define MAX_AOD_GEN_EVENT 100
#define MAX_ALLOC_SIZE 100000
#define MIN_ALLOC_SIZE 0
#define DEFAULT_PAUSE_DATA_NUM 3
#define MAX_MULTI_GMP_SIZE 3

static int g_iom3_state_aod = 0;
static struct mutex g_lock;
static uint32_t ref_cnt = 0;
struct aod_data_t *g_aod_data = NULL;
struct timer_list my_timer;
static struct aod_common_data *g_aod_common_data = NULL;
static struct aod_common_data *g_aod_gmp_data = NULL;
struct fb_list *g_ion_fb_config = NULL;
static struct aod_general_data *g_aod_gen_data[MAX_AOD_GEN_EVENT] = {NULL};
static struct aod_general_data *g_aod_pause_data = NULL;
static struct aod_general_data *g_aod_pos_data = NULL;
static struct aod_multi_gmp_data *g_aod_multi_gmp_data[MAX_MULTI_GMP_SIZE] = {NULL};

uint32_t hisi_aod_msg_level = 2;
int aod_support = 0;
int dss_off_status = 0;
static int dss_on_status = 0;
static struct mutex dss_on_off_lock;
static int handle_sh_ipc = 0;
static uint32_t hisi_aod_fold_panel_id;

aod_dss_ctrl_status_sh_t strl_cmd;
static unsigned long g_aod_jiffies = 0;
static struct workqueue_struct *g_aod_wq = NULL;
static struct work_struct g_aod_worker;

extern struct ion_client *hisi_ion_client_create(const char *name);
extern int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock);
extern int hisi_aod_inc_atomic(struct hisi_fb_data_type *dpufd);
extern void hisi_aod_dec_atomic(struct hisi_fb_data_type *dpufd);

static struct completion iom3_status_completion;
extern int register_iom3_recovery_notifier(struct notifier_block *nb);
extern int send_cmd_from_kernel_nolock(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, char  *buf, size_t count);
static int hisi_aod_sensorhub_cmd_req(enum obj_cmd cmd);
static int hisi_aod_set_display_space_req(aod_display_spaces_mcu_t *display_spaces);
static void hisi_aod_set_config_init(struct aod_data_t *aod_data);
static int hisi_aod_setup_req(aod_set_config_mcu_t *set_config);
static int hisi_aod_set_time_req(aod_time_config_mcu_t *time_config);
static int hisi_aod_start_req(aod_start_config_mcu_t *start_config);
extern int hisi_sensorhub_aod_unblank(uint32_t msg_no);
extern int hisi_sensorhub_aod_blank(uint32_t msg_no);
static int hisi_aod_fold_info_req(aod_fold_info_config_mcu_t *aod_fold_info);
void sh_recovery_handler(void);
static int hisi_free_buffer(struct aod_data_t *aod_data, void __user *arg);
static int hisi_aod_pause_new_req(void);
static void aod_multi_gmp_data_to_sensorhub(void);

int get_aod_support(void)
{
	return aod_support;
}

uint32_t hisi_aod_get_panel_id(void)
{
	HISI_AOD_INFO("panel_id is %u!\n", hisi_aod_fold_panel_id);
	return hisi_aod_fold_panel_id;
}

bool hisi_aod_need_fast_unblank(void)
{
	if ((hisi_get_panel_product_type() & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) == 0)
		return false;
	if ((g_aod_data->aod_status == true) && (g_aod_data->finger_down_status == 1))
		return true;
	return false;
}

static int hisif_aod_ion_phys(int shared_fd, struct device *dev, unsigned long *addr)
{
	struct sg_table *table = NULL;
	struct dma_buf *buf = NULL;
	struct dma_buf_attachment *attach = NULL;

	if (dev == NULL) {
		HISI_AOD_ERR("Invalid param\n");
		return -EFAULT;
	}
	buf = dma_buf_get(shared_fd);
	if (IS_ERR(buf)) {
		HISI_AOD_ERR("Invalid file handle %d\n", shared_fd);
		return -EFAULT;
	}
	attach = dma_buf_attach(buf, dev);
	if (IS_ERR(attach)) {
		dma_buf_put(buf);
		return -EFAULT;
	}
	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(table)) {
		dma_buf_detach(buf, attach);
		dma_buf_put(buf);
		return -EFAULT;
	}
	*addr = sg_phys(table->sgl); /* [false alarm] */
	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
	dma_buf_detach(buf, attach);
	dma_buf_put(buf);
	return 0;
}

#if defined(CONFIG_HISI_FB_AOD)
extern void hisi_aod_schedule_wq(void);
#endif

static int hisi_aod_send_cmd_to_sensorhub(struct write_info *wr,
	struct read_info *rd, bool is_lock)
{
	int ret = 0;
	struct write_info *pkg_ap = wr;

	if (g_iom3_state_aod) {
		ret = write_customize_cmd(wr, rd, 0);
		if (ret)
			HISI_AOD_ERR("nolock:tag is %d, cmd is %d\n",
				pkg_ap->tag, pkg_ap->cmd);
		return ret;
	}

	HISI_AOD_DEBUG("hisi_aod_send_cmd_to_sensorhub:%d,%d\n",
		wr->tag, wr->cmd);
	ret = write_customize_cmd(wr, rd, is_lock);
	if (ret)
		HISI_AOD_ERR("cmd:tag is %d, cmd is %d\n",
			pkg_ap->tag, pkg_ap->cmd);

	return ret;
}

static int hisi_aod_start_recovery_init(struct aod_data_t *aod_data)
{
	int ret = 0;

	HISI_AOD_INFO("+ .\n");

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	ret = hisi_aod_sensorhub_cmd_req(CMD_CMN_OPEN_REQ);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_sensorhub_cmd_req open fail\n");
		return ret;
	}

	ret = hisi_aod_set_display_space_req(&aod_data->display_spaces_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_set_display_space_req fail\n");
		return ret;
	}

	ret = hisi_aod_setup_req(&aod_data->set_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_setup_req fail\n");
		return ret;
	}

	ret = hisi_aod_set_time_req(&aod_data->time_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_set_time_req fail\n");
		return ret;
	}

	ret = hisi_aod_fold_info_req(&aod_data->fold_info_config_mcu);
	if (ret) {
		HISI_AOD_ERR("hisi_aod_fold_info_req fail\n");
		return ret;
	}

	HISI_AOD_INFO("- .\n");

	return 0;
}

static void aod_ion_fb_config(void)
{
	uint32_t offset;
	uint32_t len;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	int ret;

	if (g_ion_fb_config == NULL)
		return;
	offset = offsetof(struct fb_list, cmd_type);
	if (offset > g_ion_fb_config->size) {
		HISI_AOD_ERR("offset is bigger than len\n");
		return;
	}
	len = g_ion_fb_config->size - offset;
	if (len > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
		ret = shmem_send(TAG_AOD, &(g_ion_fb_config->cmd_type), len);
		HISI_AOD_INFO("shmem_send tag is %d, size %ud\n", TAG_AOD, len);
		return;
	}
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(&(g_ion_fb_config->cmd_type));
	pkg_ap.wr_len = len;
	/* 1 means with lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret)
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
	HISI_AOD_INFO("errno = %d\n", pkg_mcu.errno);
}

static void aod_gmp_data(void)
{
	uint32_t offset;
	uint32_t len;
	struct write_info pkg_ap;
	int ret;

	if (g_aod_gmp_data == NULL)
		return;

	offset = offsetof(struct aod_common_data, data);
	if (offset > g_aod_gmp_data->size) {
		HISI_AOD_ERR("offset is bigger than len\n");
		return;
	}
	len = g_aod_gmp_data->size - offset;
	if (len > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
		ret = shmem_send(TAG_AOD, g_aod_gmp_data->data, len);
		HISI_AOD_INFO("shmem_send tag is %d, size %ud\n", TAG_AOD, len);
		return;
	}
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(g_aod_gmp_data->data);
	pkg_ap.wr_len = len;
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
	if (ret)
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
}

static void aod_gen_data(void)
{
	uint32_t offset;
	uint32_t len;
	struct write_info pkg_ap;
	int ret;
	uint32_t i;

	for (i = 0; i < MAX_AOD_GEN_EVENT; i++) {
		if (g_aod_gen_data[i] == NULL)
			continue;
		HISI_AOD_INFO("g_aod_gen_data[%d]->size = %d",
			i, g_aod_gen_data[i]->size);
		offset = offsetof(struct aod_general_data, data);
		if (offset > g_aod_gen_data[i]->size) {
			HISI_AOD_ERR("offset is bigger than len\n");
			return;
		}
		len = g_aod_gen_data[i]->size - offset;
		if (len > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
			ret = shmem_send(TAG_AOD, g_aod_gen_data[i]->data, len);
			HISI_AOD_INFO("shmem_send tag is %d, size %ud\n", TAG_AOD, len);
			continue;
		}
		pkg_ap.tag = TAG_AOD;
		pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
		pkg_ap.wr_buf = (const void *)(g_aod_gen_data[i]->data);
		pkg_ap.wr_len = len;
		/* 1 means with lock */
		ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
		if (ret)
			HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
	}
}

static int hisi_aod_start_recovery_req(struct aod_data_t *aod_data)
{
	int ret;
#ifdef SHMEM_START_CONFIG
	uint32_t offset;
	uint32_t length;
#endif

	HISI_AOD_INFO("+ .\n");

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	down(&(aod_data->aod_status_sem));
	if(!aod_data->aod_status)
	{
		HISI_AOD_INFO("aod no need to start!\n");
		up(&(aod_data->aod_status_sem));
		return 0;
	}
	up(&(aod_data->aod_status_sem));
	aod_ion_fb_config();
#ifdef SHMEM_START_CONFIG
	if (g_aod_common_data != NULL) {
		offset = offsetof(struct aod_common_data, data);
		length = g_aod_common_data->size - offset;
#ifdef CONFIG_CONTEXTHUB_SHMEM
		ret = shmem_send(TAG_AOD, g_aod_common_data->data,
			length);
#endif
		if (ret)
			HISI_AOD_ERR("shmem_send common info fail\n");
		HISI_AOD_INFO("shmem_send common size %ud\n", length);
	}
#endif
	aod_gmp_data();
	aod_multi_gmp_data_to_sensorhub();
	aod_gen_data();
	ret = hisi_aod_start_req(&aod_data->start_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_start_req fail\n");
		return ret;
	}

	aod_data->finger_down_status = 0;

	HISI_AOD_INFO("- .\n");

	return 0;
}

static int sensorhub_recovery_notifier(struct notifier_block *nb, unsigned long action,
								 void *bar)
{
	struct aod_data_t *aod_start_data = g_aod_data;

	if(!aod_start_data){
		HISI_AOD_ERR("g_aod_data is NULL!\n");
		return -1;
	}
	/* prevent access the emmc now: */
	HISI_AOD_INFO("iom3 status:%lu +\n", action);
	switch (action) {
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_3RD_DOING:
		if(get_aod_support())
		{
			g_iom3_state_aod = 1;
			sh_recovery_handler();
			(void)hisi_aod_start_recovery_init(aod_start_data);
		}
		break;
	case IOM3_RECOVERY_IDLE:
		if(get_aod_support())
		{
			(void)hisi_aod_start_recovery_req(aod_start_data);
			g_iom3_state_aod = 0;
		}
		break;
	default:
		HISI_AOD_ERR("unknow state %lu\n", action);
		break;
	}
	HISI_AOD_INFO("-\n");
	return 0;
}

static struct notifier_block recovery_notify = {
	.notifier_call = sensorhub_recovery_notifier,
	.priority = -1,
};

static ssize_t hisi_aod_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long p = 0;
	unsigned long totol_size = 0;
	ssize_t cnt = 0;
	struct aod_data_t *aod_data = NULL;
	aod_notif_t notify_data;

	if((NULL == file)||(NULL == buf)||(NULL == ppos))
	{
		HISI_AOD_ERR("enter param is NULL Pointer!\n");
		return -EINVAL;
	}

	p = *ppos;
	aod_data = (struct aod_data_t*)file->private_data;
	if(NULL == aod_data)
	{
		HISI_AOD_ERR("enter param is NULL Pointer!\n");
		return -EINVAL;
	}
	notify_data = aod_data->aod_notify_data;
	reinit_completion(&iom3_status_completion);
	totol_size = sizeof(aod_notif_t);

	if(count == 0)
		return 0;

	if(p >= totol_size)
		return 0;

	if(count >= totol_size)
		count = totol_size;

	if((p + count)> totol_size)
		count = totol_size - p;

	wait_for_completion(&iom3_status_completion);
	notify_data.size = sizeof(notify_data);
	notify_data.aod_events = g_iom3_state_aod;

	if(copy_to_user(buf, &notify_data, count)) {
		HISI_AOD_ERR("Failed to copy to user\n");
		return -1;
	} else {
		cnt += p;
	}
	return cnt;
}

/*
 *	Frame buffer device initialization and setup routines
 */
static unsigned long hisi_aod_fb_alloc(struct aod_data_t *aod_data)
{
	unsigned long buf_addr = 0;

	if(!aod_data) {
		HISI_AOD_ERR("aod_data pointer is NULL\n");
		return 0;
	}

	HISI_AOD_INFO("smem_size is %lu, max_buf_size is %u\n", aod_data->smem_size, aod_data->max_buf_size);

	if((aod_data->smem_size <= 0) || (aod_data->smem_size > aod_data->max_buf_size)) {
		HISI_AOD_ERR("buf len is invalid: %lu, max_buf_size is %u\n", aod_data->smem_size, aod_data->max_buf_size);
		return 0;
	}
	(void)hisi_free_buffer(aod_data, NULL);

	aod_data->buff_virt = (void *)dma_alloc_coherent(aod_data->cma_device, aod_data->smem_size, &aod_data->buff_phy, GFP_KERNEL);
	if (aod_data->buff_virt == NULL) {
		HISI_AOD_ERR("dma_alloc_coherent failed\n");
		goto err_cma_alloc;
	}

	HISI_AOD_INFO("buff_virt is 0x%pK, buff_phy is 0x%llx\n",
		aod_data->buff_virt, aod_data->buff_phy);

	buf_addr = aod_data->buff_phy;
	aod_data->fb_start_addr = buf_addr;
	aod_data->fb_mem_alloc_flag = true;

	return buf_addr;
err_cma_alloc:
	return 0;
}
/*lint -e454, -e455, -e456, -e533*/
int dss_sr_of_sh_callback(const struct pkt_header *cmd)
{
	aod_dss_ctrl_status_ap_t *ctrl_cmd = NULL;
	int ret = 0;

	uint32_t msg_cmd;
	uint32_t msg_no;

	HISI_AOD_DEBUG("dss_sr_of_sh_callback enter!\n");

	if (cmd == NULL)
	{
		HISI_AOD_INFO("cmd is NULL!\n");
		return -1;
	}

	ctrl_cmd = (aod_dss_ctrl_status_ap_t *)cmd;

	/* high 16bit is msg No, low 16bit is msg cmd */
	msg_no = (ctrl_cmd->sub_cmd & 0xFFFF0000) >> 16;
	msg_cmd = ctrl_cmd->sub_cmd & 0xFFFF;

	if (msg_cmd == SUB_CMD_AOD_DSS_ON_REQ) {
		// call back dss on. wake_up lock, and start a timer of 1 s.
		__pm_stay_awake(&g_aod_data->wlock);
		mutex_lock(&dss_on_off_lock);
		if (dss_on_status) {
			HISI_AOD_INFO("dss on already run!\n");
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		if (!handle_sh_ipc) {
			HISI_AOD_INFO("SUB_CMD_AOD_DSS_ON_REQ:handle_sh_ipc is false!\n");
			__pm_relax(&g_aod_data->wlock);
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		my_timer.expires = jiffies + 15 * HZ;
		g_aod_jiffies = my_timer.expires;
		HISI_AOD_INFO("dss on jiffies=%lu expires=%lu\n", jiffies, my_timer.expires);
		if (!timer_pending(&my_timer))
			add_timer(&my_timer);
		else
			HISI_AOD_ERR("timer is pending\n");

		ret = hisi_sensorhub_aod_unblank(msg_no);
		if (ret) {
			HISI_AOD_INFO("dss on fail!\n");
		} else {
			HISI_AOD_DEBUG("dss on success!\n");
			dss_on_status = 1;
			dss_off_status = 0;
		}

		mutex_unlock(&dss_on_off_lock);
	}
	else if (msg_cmd == SUB_CMD_AOD_DSS_OFF_REQ)
	{
		// call back dss off. wake_unlock.
		mutex_lock(&dss_on_off_lock);
		if (!dss_on_status) {
			HISI_AOD_INFO("dss on not run onetime!\n");
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		ret = del_timer(&my_timer);
		HISI_AOD_INFO("dss off jiffies=%lu ret=%d\n", jiffies, ret);

		if (!handle_sh_ipc) {
			HISI_AOD_INFO("SUB_CMD_AOD_DSS_OFF_REQ:handle_sh_ipc is false!\n");
			__pm_relax(&g_aod_data->wlock);
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		ret = hisi_sensorhub_aod_blank(msg_no);
		if (ret) {
			HISI_AOD_INFO("dss off fail!\n");
		} else {
			HISI_AOD_DEBUG("dss off success!\n");
		}

		dss_on_status = 0;
		dss_off_status = 1;
		mutex_unlock(&dss_on_off_lock);
		__pm_relax(&g_aod_data->wlock);
	}

	HISI_AOD_DEBUG("dss_sr_of_sh_callback exit!\n");
	return 0;
}
/*lint +e454, +e455, +e456, +e533*/
/*lint -e455, -e456*/
void aod_off_handle(struct work_struct *work)
{
	int ret;

	HISI_AOD_INFO("aod_timer_process enter!\n");

	(void)work;
	mutex_lock(&dss_on_off_lock);
	if(dss_off_status)
	{
		HISI_AOD_INFO("dss is already off!\n");
		mutex_unlock(&dss_on_off_lock);
		return;
	}

	if (!handle_sh_ipc) {
		HISI_AOD_INFO("aod_timer_process:handle_sh_ipc is false!\n");
		__pm_relax(&g_aod_data->wlock);
		mutex_unlock(&dss_on_off_lock);
		return;
	}

	HISI_AOD_INFO("jiffies=%lu, g_aod_jiffies=%lu\n", jiffies, g_aod_jiffies);
	if (time_after_eq(jiffies, g_aod_jiffies)) {
		ret = hisi_sensorhub_aod_blank(0);
		if (ret)
			HISI_AOD_INFO("aod_timer_process dss off fail!\n");
		else
			HISI_AOD_INFO("aod_timer_process dss off success!\n");

		dss_on_status = 0;
		dss_off_status = 1;
	}

	mutex_unlock(&dss_on_off_lock);
	__pm_relax(&g_aod_data->wlock);

	HISI_AOD_INFO("aod_timer_process leave!\n");
}

void aod_timer_process(unsigned long data)
{
	(void)data;
	queue_work(g_aod_wq, &g_aod_worker);
}

void sh_recovery_handler(void)
{
	int ret = 0;

	HISI_AOD_INFO("sh_recovery_handler enter!\n");
	mutex_lock(&dss_on_off_lock);
	if(dss_on_status && !dss_off_status)
	{
		del_timer(&my_timer);

		if (!handle_sh_ipc) {
			HISI_AOD_INFO("sh_recovery_handler:handle_sh_ipc is false!\n");
			__pm_relax(&g_aod_data->wlock);
			mutex_unlock(&dss_on_off_lock);
			return;
		}

		ret = hisi_sensorhub_aod_blank(0);
		if(ret)
		{
			HISI_AOD_INFO("dss off fail!\n");
		}
		else
		{
			HISI_AOD_INFO("dss off success!\n");
			dss_off_status = 1;
			dss_on_status = 0;
		}
		__pm_relax(&g_aod_data->wlock);
	}

	mutex_unlock(&dss_on_off_lock);
	HISI_AOD_INFO("sh_recovery_handler exit!\n");

}
/*lint +e455, +e456*/

static void hisi_dump_time_config(aod_time_config_mcu_t *time_config)
{
	HISI_AOD_DEBUG("curr_tm %llu, tm_zone %d, sec_tm_zone %d, tm_fmt %d\n",
		time_config->curr_time, time_config->time_zone,
		time_config->sec_time_zone, time_config->time_format);
}

static void hisi_dump_display_space(aod_display_spaces_mcu_t *display_space)
{
	int i;

	HISI_AOD_DEBUG("dual_clk %d, disp_space %d, pd_logo_pos_y = %d\n",
		display_space->dual_clocks,
		display_space->display_space_count,
		display_space->pd_logo_final_pos_y);

	for (i = 0; i < display_space->display_space_count; i++) {
		HISI_AOD_DEBUG("display_spaces[%d].x_start is %d\n",
			i, display_space->display_spaces[i].x_start);
		HISI_AOD_DEBUG("display_spaces[%d].y_start is %d\n",
			i, display_space->display_spaces[i].y_start);
		HISI_AOD_DEBUG("display_spaces[%d].x_size is %d\n",
			i, display_space->display_spaces[i].x_size);
		HISI_AOD_DEBUG("display_spaces[%d].y_size is %d\n",
			i, display_space->display_spaces[i].y_size);
	}
}

static void hisi_dump_start_config(const aod_start_config_mcu_t *start_config)
{
	uint32_t i;
	uint64_t count;

	HISI_AOD_DEBUG("intelli_switching is %d\n",
		start_config->intelli_switching);
	HISI_AOD_DEBUG("aod_type is %d\n", start_config->aod_type);
	HISI_AOD_DEBUG("fp_mode is %d\n", start_config->fp_mode);
	HISI_AOD_INFO("dynamic_fb %u, ext_fb %u, face_id %u, pd_logo %u\n",
		start_config->dynamic_fb_count,
		start_config->dynamic_ext_fb_count,
		start_config->face_id_fb_count,
		start_config->pd_logo_fb_count);
	count = start_config->dynamic_fb_count +
		start_config->dynamic_ext_fb_count +
		start_config->face_id_fb_count +
		start_config->pd_logo_fb_count;
#if SHMEM_START_CONFIG
	HISI_AOD_DEBUG("digital_clock_count = %u",
		start_config->digital_clock_count);
	HISI_AOD_DEBUG("analog_clock_count = %u",
		start_config->analog_clock_count);
	HISI_AOD_DEBUG("pattern_clock_count = %u",
		start_config->pattern_clock_count);
	HISI_AOD_DEBUG("dynamic_reserve_count = %u",
		start_config->dynamic_reserve_count);
	count += start_config->digital_clock_count +
		start_config->analog_clock_count +
		start_config->pattern_clock_count +
		start_config->dynamic_reserve_count;
#endif
	if (count > MAX_DYNAMIC_ALLOC_FB_COUNT) {
		HISI_AOD_DEBUG("count > MAX_DYNAMIC_ALLOC_FB_COUNT");
		return;
	}
	for (i = 0; i < count; i++)
		HISI_AOD_DEBUG("dynamic_fb_addr[%u] is 0x%x\n",
			i, start_config->dynamic_fb_addr[i]);
	HISI_AOD_DEBUG("xstart is %d, ystart is %d\n",
		start_config->aod_pos.x_start, start_config->aod_pos.y_start);
}

static void hisi_dump_set_config_info(aod_set_config_mcu_t *set_config_info)
{
	int i;

	HISI_AOD_DEBUG("aod_fb is 0x%x, aod_digits_addr is 0x%x\n",
		set_config_info->aod_fb, set_config_info->aod_digits_addr);
	HISI_AOD_DEBUG("fp_offset is 0x%x, fp_count is 0x%x\n",
		set_config_info->fp_offset, set_config_info->fp_count);
	HISI_AOD_DEBUG("xres %d, yres %d, pixel_format %d\n",
		set_config_info->screen_info.xres,
		set_config_info->screen_info.yres,
		set_config_info->screen_info.pixel_format);

	for(i = 0; i < set_config_info->bitmaps_size.bitmap_type_count; i++) {
		HISI_AOD_DEBUG("bitmap_size[%d].xres is %d\n",
			i, set_config_info->bitmaps_size.bitmap_size[i].xres);
		HISI_AOD_DEBUG("bitmap_size[%d].yres is %d\n",
			i, set_config_info->bitmaps_size.bitmap_size[i].yres);
	}
}

static int hisi_aod_sensorhub_cmd_req(enum obj_cmd cmd)
{
	int ret = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;

	HISI_AOD_INFO("+.\n");

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset

	HISI_AOD_INFO("+1.\n");

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = cmd;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	HISI_AOD_INFO("+2.\n");
	/* 1 for lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_INFO("-.\n");

	return ret;
}

static int hisi_aod_set_display_space_req(aod_display_spaces_mcu_t *display_spaces)
{
	int ret = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	aod_pkt_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;
	int i = 0;

	HISI_AOD_DEBUG("+\n");

	if(NULL == display_spaces) {
		HISI_AOD_ERR("display_spaces is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_SET_DISPLAY_SPACE_REQ;

	pkt.display_param.dual_clocks = display_spaces->dual_clocks;
	pkt.display_param.display_space_count = display_spaces->display_space_count;
	pkt.display_param.pd_logo_final_pos_y = display_spaces->pd_logo_final_pos_y;

	for(i = 0; i < pkt.display_param.display_space_count; i++) {
		pkt.display_param.display_spaces[i].x_size = display_spaces->display_spaces[i].x_size;
		pkt.display_param.display_spaces[i].y_size = display_spaces->display_spaces[i].y_size;
		pkt.display_param.display_spaces[i].x_start= display_spaces->display_spaces[i].x_start;
		pkt.display_param.display_spaces[i].y_start = display_spaces->display_spaces[i].y_start;
	}
	hisi_dump_display_space(&pkt.display_param);

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	/* 1 for lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_DEBUG("-\n");

	return ret;
}

static void hisi_aod_set_config_init(struct aod_data_t *aod_data)
{
	int i = 0;
	size_t bitmap_count = 0;

	aod_data->set_config_mcu.aod_fb = aod_data->fb_start_addr;
	aod_data->aod_digits_addr += aod_data->fb_start_addr;
	aod_data->set_config_mcu.aod_digits_addr = aod_data->aod_digits_addr;
	aod_data->set_config_mcu.fp_offset += aod_data->fb_start_addr;

	aod_data->set_config_mcu.screen_info.pixel_format = AOD_FB_PIXEL_FORMAT_RGB_565;
	aod_data->set_config_mcu.screen_info.xres = aod_data->x_res; // 144;
	aod_data->set_config_mcu.screen_info.yres = aod_data->y_res; // 256;

	bitmap_count = aod_data->bitmaps_size_mcu.bitmap_type_count;

	aod_data->set_config_mcu.bitmaps_size.bitmap_type_count = aod_data->bitmaps_size_mcu.bitmap_type_count;
	for (i = 0; i < MAX_BIT_MAP_SIZE; i ++) {
		aod_data->set_config_mcu.bitmaps_size.bitmap_size[i].xres = aod_data->bitmaps_size_mcu.bitmap_size[i].xres;
		aod_data->set_config_mcu.bitmaps_size.bitmap_size[i].yres = aod_data->bitmaps_size_mcu.bitmap_size[i].yres;
	}
}

static int hisi_aod_setup_req(aod_set_config_mcu_t *set_config)
{
	int ret = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	aod_pkt_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;
	int i = 0;

	HISI_AOD_INFO("+.\n");

	if(NULL == set_config) {
		HISI_AOD_ERR("config_info is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_SETUP_REQ;

	pkt.set_config_param.aod_fb = set_config->aod_fb;
	pkt.set_config_param.aod_digits_addr = set_config->aod_digits_addr;
	pkt.set_config_param.fp_offset = set_config->fp_offset;
	pkt.set_config_param.fp_count = set_config->fp_count;
	pkt.set_config_param.screen_info.xres = set_config->screen_info.xres;
	pkt.set_config_param.screen_info.yres = set_config->screen_info.yres;
	pkt.set_config_param.screen_info.pixel_format = set_config->screen_info.pixel_format;
	pkt.set_config_param.bitmaps_size.bitmap_type_count = set_config->bitmaps_size.bitmap_type_count;

	for(i = 0; i < (pkt.set_config_param.bitmaps_size.bitmap_type_count); i ++) {
		pkt.set_config_param.bitmaps_size.bitmap_size[i].xres = set_config->bitmaps_size.bitmap_size[i].xres;
		pkt.set_config_param.bitmaps_size.bitmap_size[i].yres = set_config->bitmaps_size.bitmap_size[i].yres;
	}

	hisi_dump_set_config_info(&pkt.set_config_param);

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	/* 1 for lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	if (pkg_mcu.errno != 0) {
		HISI_AOD_ERR("errno = %d \n", pkg_mcu.errno);
		return -1;
	}

	HISI_AOD_INFO("-.\n");

	return ret;
}

static int hisi_aod_set_time_req(aod_time_config_mcu_t *time_config)
{
	int ret = 0;
	struct write_info pkg_ap;
	aod_pkt_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;

	HISI_AOD_DEBUG("+\n");

	if(NULL == time_config) {
		HISI_AOD_ERR("time_config is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_SET_TIME_REQ;

	pkt.time_param.curr_time = time_config->curr_time;
	pkt.time_param.sec_time_zone = time_config->sec_time_zone;
	pkt.time_param.time_format = time_config->time_format;
	pkt.time_param.time_zone = time_config->time_zone;

	hisi_dump_time_config(&pkt.time_param);

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	/* 1 for lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_DEBUG("-\n");

	return ret;
}
/*lint -e454, -e456, -e533*/
static int hisi_aod_start_req(aod_start_config_mcu_t *start_config)
{
	int ret = 0;
	struct aod_data_t *aod_data = g_aod_data;

	HISI_AOD_INFO("+.\n");

	if (!aod_data) {
		HISI_AOD_INFO("aod_data is NULL.\n");
		return -1;
	}

	if (aod_data->blank_mode != FB_BLANK_POWERDOWN) {
		HISI_AOD_INFO("blank_mode is %d.\n", aod_data->blank_mode);
		aod_data->start_req_faster = true;
		if (aod_data->no_need_enter_aod) {
			HISI_AOD_INFO("no need enter aod.\n");
			aod_data->no_need_enter_aod = false;
			aod_data->start_req_faster = false;
		}
		return -EAGAIN;
	}
	aod_data->start_req_faster = false;
	if (NULL == start_config) {
		HISI_AOD_ERR("display_spaces is NULL Pointer\n");
		return -1;
	}

	if (!aod_data->aod_status) {
		HISI_AOD_INFO("first enter aod!\n");
		mutex_lock(&(aod_data->aod_lock));
		aod_data->aod_lock_status = false;
		HISI_AOD_INFO("hold aod_lock\n");
		mutex_lock(&dss_on_off_lock);
		handle_sh_ipc = 1;
		mutex_unlock(&dss_on_off_lock);
	}

	hisi_dump_start_config(start_config);

	down(&(aod_data->aod_status_sem));
	aod_data->aod_status = true;
	up(&(aod_data->aod_status_sem));
#ifdef CONFIG_CONTEXTHUB_SHMEM
#if SHMEM_START_CONFIG
	start_config->cmd_sub_type = SHMEM_START_MSG_CMD_TYPE;
#endif
	ret = shmem_send(TAG_AOD, (void *)start_config, sizeof(aod_start_config_mcu_t));
	HISI_AOD_INFO("shmem_send tag is %d, size %ld\n", TAG_AOD,
		 sizeof(aod_start_config_mcu_t));
#endif
	if (ret) {
		HISI_AOD_ERR("shmem_send fail. TAG_AOD is %d, size %ld\n",
			 TAG_AOD, sizeof(aod_start_config_mcu_t));
		down(&(aod_data->aod_status_sem));
		aod_data->aod_status = false;
		up(&(aod_data->aod_status_sem));
		mutex_lock(&dss_on_off_lock);
		handle_sh_ipc = 0;
		mutex_unlock(&dss_on_off_lock);
		mutex_unlock(&(g_aod_data->aod_lock));
		aod_data->aod_lock_status = true;
		HISI_AOD_INFO("release aod_lock\n");
		return ret;
	}

	HISI_AOD_INFO("-.\n");
	return ret;
}
/*lint +e454, +e533, +e456*/
/*lint -e454, -e455*/
//lint -efunc(456,455,454,*)
static int hisi_aod_stop_req(aod_display_pos_t *display_pos)
{
	int ret = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	aod_pkt_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;
	aod_display_pos_t *aod_pos;
	struct hisi_fb_data_type * dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	HISI_AOD_INFO("+.\n");

	if(NULL == display_pos) {
		HISI_AOD_ERR("display_pos is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_STOP_REQ;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt.subtype);
	/*
	 * set is_lock be false to avoid blocking face lock
	 * 0 for nolock
	 */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 0);
	if (ret)
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);

	if (pkg_mcu.errno != 0) {
		HISI_AOD_ERR("errno = %d \n", pkg_mcu.errno);
	} else {
		aod_pos = (aod_display_pos_t *)&pkg_mcu.data;
		HISI_AOD_INFO("aod pos x is %d, y is %d.\n", aod_pos->x_start, aod_pos->y_start);
		display_pos->x_start = aod_pos->x_start;
		display_pos->y_start = aod_pos->y_start;
		ret = 0;
	}

	mutex_lock(&dss_on_off_lock);
	handle_sh_ipc = 0;
	if (dss_on_status) {
		del_timer(&my_timer);
		(void)hisi_sensorhub_aod_blank(0xFFFF);
		dss_on_status = 0;
		dss_off_status = 1;
		__pm_relax(&g_aod_data->wlock);
	}
	mutex_unlock(&dss_on_off_lock);

	mutex_unlock(&(g_aod_data->aod_lock));
	g_aod_data->aod_lock_status = true;
	HISI_AOD_INFO("release aod_lock\n");

	HISI_AOD_INFO("finger_down_status is %d \n", g_aod_data->finger_down_status);
	if (g_aod_data->finger_down_status == 2 && hisi_aod_inc_atomic(dpufd)){
		HISI_AOD_INFO("in fast unlock mode\n");
		mutex_lock(&(g_aod_data->aod_lock));
		g_aod_data->aod_lock_status = true;
		HISI_AOD_INFO("hold aod_lock\n");
		g_aod_data->blank_mode = FB_BLANK_UNBLANK;
		hisi_aod_schedule_wq();
	}
	HISI_AOD_INFO("-.\n");

	return ret;
}
//lint +efunc(456,455,454,*)
/*lint +e454, +e455*/

void default_aod_pause_data(void)
{
	g_aod_pause_data->size = sizeof(uint32_t) * DEFAULT_PAUSE_DATA_NUM;
	g_aod_pause_data->data[SUB_CMD_TYPE] = SUB_CMD_AOD_STOP_REQ;
	g_aod_pause_data->data[SCREEN_STATE] = DISPALY_SCREEN_OFF;
}

int hisi_send_aod_stop(void)
{
	int ret = 0;
	struct aod_data_t *aod_data = g_aod_data;

	if (!aod_data) {
		HISI_AOD_INFO("aod_data is NULL.\n");
		return -1;
	}
	HISI_AOD_INFO("+ .\n");

	if (!aod_data->aod_status) {
		HISI_AOD_INFO("already exit aod mode.\n");
		return 0;
	}
	if (!g_aod_pause_data) {
		ret = hisi_aod_stop_req(&aod_data->pos_data);
	} else {
		default_aod_pause_data();
		ret = hisi_aod_pause_new_req();
	}
	if(ret) {
		HISI_AOD_ERR("hisi_aod_stop_req fail\n");
		return -1;
	}
	down(&(aod_data->aod_status_sem));
	aod_data->aod_status = false;
	up(&(aod_data->aod_status_sem));

	HISI_AOD_INFO("- .\n");

	return 0;
}

static int hisi_aod_end_updating_req(aod_display_pos_t *pos_data, uint32_t aod_type)
{
	int ret = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	aod_pkt_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;

	HISI_AOD_DEBUG("+\n");

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset

	if(NULL == pos_data) {
		HISI_AOD_ERR("pos_data is NULL Pointer\n");
		return -1;
	}

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_END_UPDATING_REQ;

	pkt.end_updating_data_to_sensorhub.aod_pos.x_start = pos_data->x_start;
	pkt.end_updating_data_to_sensorhub.aod_pos.y_start = pos_data->y_start;
	pkt.end_updating_data_to_sensorhub.aod_type = aod_type;

	HISI_AOD_DEBUG("display_pos:x_start %d, y_start %d; aod_type %u\n",
		pkt.end_updating_data_to_sensorhub.aod_pos.x_start,
		pkt.end_updating_data_to_sensorhub.aod_pos.y_start,
		pkt.end_updating_data_to_sensorhub.aod_type);

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	/* 1 for lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_DEBUG("-\n");

	return ret;
}

static bool is_1bit_ramless_aod_panel(void)
{
	struct hisi_fb_data_type *dpufd = dpufd_list[PRIMARY_PANEL_IDX];
	struct hisi_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);
	if (dpufd->panel_info.ramless_aod == true)
		return true;
	return false;
}

static int hisi_aod_start_updating_req(aod_display_pos_t *display_pos)
{
	int ret = 0;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	aod_pkt_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;
	aod_display_pos_t *aod_pos = NULL;

	HISI_AOD_DEBUG("+\n");

	memset(&pkg_ap, 0, sizeof(pkg_ap)); // unsafe_function_ignore: memset
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_START_UPDATING_REQ;

	pkt.display_pos.x_start = display_pos->x_start;
	pkt.display_pos.y_start = display_pos->y_start;

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt.subtype);
	if (is_1bit_ramless_aod_panel())
		/* 0 means no lock */
		ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 0);
	else
		/* 1 means lock */
		ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	if (pkg_mcu.errno != 0) {
		HISI_AOD_ERR("errno = %d \n", pkg_mcu.errno);
		return -1;
	}else{
		aod_pos = (aod_display_pos_t *)&pkg_mcu.data;
		HISI_AOD_INFO("aod pos x is %d, y is %d.\n", aod_pos->x_start, aod_pos->y_start);
		display_pos->x_start = aod_pos->x_start;
		display_pos->y_start = aod_pos->y_start;
		ret = 0;
	}

	HISI_AOD_DEBUG("-\n");

	return ret;
}

static int hisi_set_bitmap_size(struct aod_data_t *aod_data,
	const void __user *arg)
{
	aod_bitmaps_size_ap_t bitmaps_size;
	size_t bitmap_count = 0;
	uint32_t i = 0;

	HISI_AOD_DEBUG("+\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if(copy_from_user(&bitmaps_size, arg, sizeof(bitmaps_size)))
		return -EFAULT;

	if((bitmaps_size.bitmap_type_count > MAX_BIT_MAP_SIZE) || (bitmaps_size.bitmap_type_count < 0)) {
		HISI_AOD_ERR("invalid bitmaps_size:bitmap_type_count=%d\n", bitmaps_size.bitmap_type_count);
		return -EINVAL;
	}

	bitmap_count = bitmaps_size.bitmap_type_count;

	for (i = 0; i < bitmap_count; i ++) {
		if((bitmaps_size.bitmap_size[i].xres > aod_data->x_res) || (bitmaps_size.bitmap_size[i].yres > aod_data->y_res)) {
			HISI_AOD_ERR("invalid bitmaps_size: i= %u, xres=%d,yres=%d\n", i,bitmaps_size.bitmap_size[i].xres,bitmaps_size.bitmap_size[i].yres);
			return -EINVAL;
		}
	}

	aod_data->bitmaps_size_mcu.bitmap_type_count = bitmap_count;
	for (i = 0; i < bitmap_count; i ++) {
		aod_data->bitmaps_size_mcu.bitmap_size[i].xres = bitmaps_size.bitmap_size[i].xres;
		aod_data->bitmaps_size_mcu.bitmap_size[i].yres = bitmaps_size.bitmap_size[i].yres;
		HISI_AOD_DEBUG("xres [%u] [%d]\n",
			i, aod_data->bitmaps_size_mcu.bitmap_size[i].xres);
		HISI_AOD_DEBUG("yres [%u] [%d]\n",
			i, aod_data->bitmaps_size_mcu.bitmap_size[i].yres);
	}

	HISI_AOD_DEBUG("-\n");
	return 0;
}

static int hisi_set_time(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret;
	aod_time_config_ap_t time_config;

	HISI_AOD_DEBUG("+\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if(copy_from_user(&time_config, arg, sizeof(time_config)))
		return -EFAULT;

	if((time_config.sec_time_zone < -720)
	|| (time_config.sec_time_zone > 840)
		|| (time_config.time_zone < -720)
		|| (time_config.time_zone > 840)) {
		HISI_AOD_ERR("invalid time_config:curr_time=%llu,sec_time_zone=%d,time_format=%d,time_zone=%d!\n",
			time_config.curr_time, time_config.sec_time_zone, time_config.time_format, time_config.time_zone);
		return -EINVAL;
	}

	aod_data->time_config_mcu.curr_time = time_config.curr_time;
	aod_data->time_config_mcu.sec_time_zone = time_config.sec_time_zone;
	aod_data->time_config_mcu.time_format = time_config.time_format;
	aod_data->time_config_mcu.time_zone = time_config.time_zone;

	HISI_AOD_DEBUG("curr_tm %llu, tm_zone %d, sec_tm_zone %d, tm_fmt %d\n",
			time_config.curr_time, time_config.time_zone,
			time_config.sec_time_zone, time_config.time_format);

	ret = hisi_aod_set_time_req(&aod_data->time_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_set_time_req fail\n");
		return ret;
	}

	HISI_AOD_DEBUG("-\n");
	return 0;
}

static int hisi_display_spaces_valid(aod_display_spaces_ap_temp_t spaces_temp)
{
	uint32_t i;
	uint32_t display_space_count;

	display_space_count = spaces_temp.display_space_count - DIFF_NUMBER;
	if ((display_space_count == 0) ||
		(display_space_count > MAX_DISPLAY_SPACE_COUNT)) {
		HISI_AOD_ERR("invalid display_spaces:dual_clocks=%u, display_space_count=%u!\n",
			spaces_temp.dual_clocks, display_space_count);
		return -EINVAL;
	}

	for (i = 0; i < display_space_count; i++)
		HISI_AOD_INFO("display_space%d:x_size=%d, x_start=%d, y_size=%d, y_start=%d\n",
			i, spaces_temp.display_spaces[i].x_size,
			spaces_temp.display_spaces[i].x_start,
			spaces_temp.display_spaces[i].y_size,
			spaces_temp.display_spaces[i].y_start);
	return 0;
}

static int hisi_set_display_space(struct aod_data_t *aod_data,
	const void __user *arg)
{
	aod_display_spaces_ap_temp_t display_spaces_temp;
	unsigned char display_space_count;
	int ret;

	HISI_AOD_DEBUG("+\n");

	if (!arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (!aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if (copy_from_user(&display_spaces_temp, arg,
		sizeof(display_spaces_temp))) {
		HISI_AOD_ERR("copy from user fail!\n");
		return -EFAULT;
	}

	ret = hisi_display_spaces_valid(display_spaces_temp);
	if (ret < 0)
		return ret;

	display_space_count = (unsigned char)display_spaces_temp.display_space_count - DIFF_NUMBER;
	if (display_space_count > MAX_DISPLAY_SPACE_COUNT) {
		HISI_AOD_ERR("display_space_count is too big\n");
		return ret;
	}
	/* unsafe_function_ignore: memcpy */
	memcpy(aod_data->display_spaces_mcu.display_spaces,
		display_spaces_temp.display_spaces,
		sizeof(display_spaces_temp.display_spaces[0]) * display_space_count);

	aod_data->display_spaces_mcu.dual_clocks = (unsigned char)display_spaces_temp.dual_clocks;
	aod_data->display_spaces_mcu.display_space_count = display_space_count;
	aod_data->display_spaces_mcu.pd_logo_final_pos_y = display_spaces_temp.display_spaces[display_space_count].y_start;
	ret = hisi_aod_set_display_space_req(&aod_data->display_spaces_mcu);
	if (ret) {
		HISI_AOD_ERR("hisi_aod_set_display_space_req fail\n");
		return ret;
	}

	HISI_AOD_DEBUG("-\n");

	return 0;
}

static int start_config_par(aod_start_config_ap_t start_config, struct aod_data_t *aod_data)
{
	if(NULL == aod_data)
	{
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -1;
	}

	if( (start_config.fb_offset >= aod_data->max_buf_size)
		|| (start_config.bitmaps_offset >= aod_data->max_buf_size)
		|| (start_config.pixel_format < AOD_FB_PIXEL_FORMAT_RGB_565)
		|| (start_config.pixel_format > AOD_FB_PIXEL_FORMAT_VYUY_422_Pkg)
		|| (start_config.aod_pos.x_start >= aod_data->x_res)
		|| (start_config.aod_pos.y_start >= aod_data->y_res)) {
		HISI_AOD_ERR("invalid start_config:intelli_switching=%d,fb_offset=%d,bitmaps_offset=%d,pixel_format=%d,x_start=%d,y_start=%d\n",
			start_config.intelli_switching,start_config.fb_offset,start_config.bitmaps_offset,
			start_config.pixel_format,start_config.aod_pos.x_start,start_config.aod_pos.y_start);
			return -1;
	}

	return 0;
}

static int hisi_aod_start(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret = 0;
	aod_start_config_ap_t start_config;

	HISI_AOD_INFO("+ .\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	down(&(aod_data->aod_status_sem));
	if (aod_data->aod_status) {
		HISI_AOD_INFO("already in aod mode.\n");
		up(&(aod_data->aod_status_sem));
		return 0;
	}
	up(&(aod_data->aod_status_sem));

	if(copy_from_user(&start_config, arg, sizeof(start_config))) {
		return -EFAULT;
	}

	ret = start_config_par(start_config, aod_data);
	if(ret<0)
	{
		return -EINVAL;
	}


	aod_data->aod_digits_addr = start_config.bitmaps_offset;
	aod_data->set_config_mcu.fp_offset = start_config.fp_offset;
	aod_data->set_config_mcu.fp_count = start_config.fp_count;

	hisi_aod_set_config_init(aod_data);
	ret = hisi_aod_setup_req(&aod_data->set_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_setup_req fail\n");
		return ret;
	}

	aod_data->start_config_mcu.aod_pos.x_start = start_config.aod_pos.x_start;
	aod_data->start_config_mcu.aod_pos.y_start = start_config.aod_pos.y_start;
	aod_data->start_config_mcu.intelli_switching = start_config.intelli_switching;
	aod_data->start_config_mcu.aod_type = start_config.aod_type;
	aod_data->start_config_mcu.fp_mode = start_config.fp_mode;
	ret = hisi_aod_start_req(&aod_data->start_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_start_req fail\n");
		return ret;
	}

	HISI_AOD_INFO("- .\n");

	return 0;
}

static void aod_mem_free()
{
	int i;

	if (g_ion_fb_config != NULL) {
		kfree(g_ion_fb_config);
		g_ion_fb_config = NULL;
	}
	for (i = 0; i < MAX_AOD_GEN_EVENT; i++) {
		if (g_aod_gen_data[i] != NULL) {
			kfree(g_aod_gen_data[i]);
			g_aod_gen_data[i] = NULL;
		}
	}
}

static int hisi_aod_pause(struct aod_data_t *aod_data, void __user* arg)
{
	int ret = 0;
	aod_pause_pos_data_t pause_display_pos;

	HISI_AOD_INFO("+.\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if (!aod_data->aod_status) {
		HISI_AOD_INFO("already exit aod mode.\n");
		return 0;
	}

	ret = hisi_aod_stop_req(&aod_data->pos_data);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_stop_req fail\n");
		return ret;
	}

	down(&(aod_data->aod_status_sem));
	aod_data->aod_status = false;
	up(&(aod_data->aod_status_sem));

	pause_display_pos.aod_pos.x_start = aod_data->pos_data.x_start;
	pause_display_pos.aod_pos.y_start = aod_data->pos_data.y_start;
	pause_display_pos.size = sizeof(pause_display_pos);

	if(copy_to_user(arg, &pause_display_pos, pause_display_pos.size)) {
		return -EFAULT;
	}
	aod_mem_free();
	HISI_AOD_INFO("- .\n");

	return 0;
}

static int aod_pause_data_receive(uint32_t *len, void __user *arg)
{
	unsigned int i;

	if (copy_from_user(len, arg, sizeof(uint32_t))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	if (g_aod_pause_data != NULL) {
		kfree(g_aod_pause_data);
		g_aod_pause_data = NULL;
	}
	HISI_AOD_INFO("receive ap length = %u\n", *len);
	if ((*len < sizeof(struct aod_general_data)) || (*len > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("len < 4 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_aod_pause_data = kzalloc(*len, GFP_KERNEL);
	if (!g_aod_pause_data) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(g_aod_pause_data, arg, *len)) {
		HISI_AOD_ERR("copy_from_user failed\n");
		kfree(g_aod_pause_data);
		g_aod_pause_data = NULL;
		return -EFAULT;
	}
	if (g_aod_pause_data->size != *len) {
		HISI_AOD_ERR("err size=%u, *len=%u\n", g_aod_pause_data->size, *len);
		kfree(g_aod_pause_data);
		g_aod_pause_data = NULL;
		return -EFAULT;
	}
	/* print AOD pause info */
	for (i = 0; i < (*len / 4); i++) /* 4 means uint32_t length */
		HISI_AOD_INFO("receive ap data[%d] = %u\n",
			i, *((uint32_t *)g_aod_pause_data + i));
	return HISI_AOD_OK;
}

static int hisi_aod_pause_new(struct aod_data_t *aod_data, void __user *arg)
{
	int ret;
	unsigned int i;
	uint32_t length = 0;
	HISI_AOD_INFO("+\n");

	if ((!arg) || (!aod_data)) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	if (!aod_data->aod_status) {
		HISI_AOD_INFO("already exit aod mode\n");
		return HISI_AOD_OK;
	}
	ret = aod_pause_data_receive(&length, arg);
	if (ret) {
		HISI_AOD_ERR("aod_pause_data_receive fail!\n");
		return ret;
	}
	ret = hisi_aod_pause_new_req();
	if (ret) {
		HISI_AOD_ERR("hisi_aod_pause_new_req fail\n");
		kfree(g_aod_pause_data);
		g_aod_pause_data = NULL;
		return ret;
	}
	down(&(aod_data->aod_status_sem));
	aod_data->aod_status = false;
	up(&(aod_data->aod_status_sem));
	/* print AOD layer pos info */
	for (i = 0; i < (length / 4); i++) /* 4 means uint32_t length */
		HISI_AOD_INFO("receive sensorhub data[%d] = %u\n",
			i, *((uint32_t *)g_aod_pause_data + i));
	if (copy_to_user(arg, g_aod_pause_data, length)) {
		HISI_AOD_ERR("copy_to_user failed\n");
		kfree(g_aod_pause_data);
		g_aod_pause_data = NULL;
		return -EFAULT;
	}
	aod_mem_free();
	HISI_AOD_INFO("-\n");
	return HISI_AOD_OK;
}

static int hisi_aod_pos_data_req(void)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;

	HISI_AOD_INFO("+\n");
	if (!g_aod_pos_data) {
		HISI_AOD_ERR("g_aod_pause_data is NULL Pointer\n");
		return HISI_AOD_FAIL;
	}
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(g_aod_pos_data->data);
	pkg_ap.wr_len = g_aod_pos_data->size - sizeof(uint32_t);
	/* set is_lock be false to avoid blocking face lock, 0 for nolock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 0);
	if (ret)
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
	if (pkg_mcu.errno != 0) {
		HISI_AOD_ERR("errno = %d\n", pkg_mcu.errno);
	} else {
		if (pkg_ap.wr_len < pkg_mcu.data_length) {
			HISI_AOD_ERR("pkg_mcu.data_length is too big\n");
			ret = HISI_AOD_FAIL;
		} else {
			memcpy(g_aod_pos_data->data,
				(const void *)(pkg_mcu.data),
				pkg_mcu.data_length);
			ret = HISI_AOD_OK;
		}
	}
	HISI_AOD_INFO("-\n");
	return ret;
}

static int aod_pos_data_receive(uint32_t *len, void __user *arg)
{
	uint32_t i;

	if (copy_from_user(len, arg, sizeof(uint32_t))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	if (g_aod_pos_data != NULL) {
		kfree(g_aod_pos_data);
		g_aod_pos_data = NULL;
	}
	HISI_AOD_INFO("receive ap length = %u\n", *len);
	if ((*len < sizeof(struct aod_general_data)) || (*len > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("len < 4 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_aod_pos_data = kzalloc(*len, GFP_KERNEL);
	if (!g_aod_pos_data) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(g_aod_pos_data, arg, *len)) {
		HISI_AOD_ERR("copy_from_user failed\n");
		kfree(g_aod_pos_data);
		g_aod_pos_data = NULL;
		return -EFAULT;
	}
	if (g_aod_pos_data->size != *len) {
		HISI_AOD_ERR("err size=%u, *len=%u\n", g_aod_pos_data->size, *len);
		kfree(g_aod_pos_data);
		g_aod_pos_data = NULL;
		return -EFAULT;
	}
	/* print AOD pause info */
	for (i = 0; i < (*len / 4); i++) /* 4 means uint32_t length */
		HISI_AOD_INFO("receive ap data[%d] = %u\n",
			i, *((uint32_t *)g_aod_pos_data + i));
	return HISI_AOD_OK;
}

static int hisi_aod_get_pos(struct aod_data_t *aod_data, void __user *arg)
{
	int ret;
	uint32_t i;
	uint32_t length = 0;
	HISI_AOD_INFO("+\n");

	if ((!arg) || (!aod_data)) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	ret = aod_pos_data_receive(&length, arg);
	if (ret) {
		HISI_AOD_ERR("aod_pause_data_receive fail!\n");
		return ret;
	}
	ret = hisi_aod_pos_data_req();
	if (ret) {
		HISI_AOD_ERR("hisi_aod_pause_new_req fail\n");
		kfree(g_aod_pos_data);
		g_aod_pos_data = NULL;
		return ret;
	}
	/* print AOD layer pos info */
	for (i = 0; i < (length / 4); i++) /* 4 means uint32_t length */
		HISI_AOD_INFO("receive sensorhub data[%d] = %u\n",
			i, *((uint32_t *)g_aod_pos_data + i));
	if (copy_to_user(arg, g_aod_pos_data, length)) {
		HISI_AOD_ERR("copy_to_user failed\n");
		kfree(g_aod_pos_data);
		g_aod_pos_data = NULL;
		return -EFAULT;
	}
	HISI_AOD_INFO("-\n");
	return HISI_AOD_OK;
}

static void hisi_disable_sh_ipc_and_blank(void)
{
	mutex_lock(&dss_on_off_lock);
	handle_sh_ipc = false;
	if (dss_on_status) {
		del_timer(&my_timer);
		(void)hisi_sensorhub_aod_blank(0xFFFF);
		dss_on_status = 0;
		dss_off_status = 1;
		__pm_relax(&g_aod_data->wlock);
	}
	mutex_unlock(&dss_on_off_lock);
}

static void fast_unlock_mode_handle(struct hisi_fb_data_type *dpufd)
{
	HISI_AOD_INFO("finger_down_status is %d\n",
		g_aod_data->finger_down_status);
	if ((g_aod_data->finger_down_status == STATUS_FINGER_CHECK_OK) &&
		hisi_aod_inc_atomic(dpufd)) {
		HISI_AOD_INFO("in fast unlock mode\n");
		mutex_lock(&(g_aod_data->aod_lock));
		g_aod_data->aod_lock_status = true;
		HISI_AOD_INFO("hold aod_lock\n");
		g_aod_data->blank_mode = FB_BLANK_UNBLANK;
		hisi_aod_schedule_wq();
	}
}

static int hisi_aod_pause_new_req(void)
{
	int ret;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	struct hisi_fb_data_type *dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	HISI_AOD_INFO("+\n");
	if (!g_aod_pause_data) {
		HISI_AOD_ERR("g_aod_pause_data is NULL Pointer\n");
		return HISI_AOD_FAIL;
	}
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(g_aod_pause_data->data);
	pkg_ap.wr_len = g_aod_pause_data->size - sizeof(uint32_t);
	/* set is_lock be false to avoid blocking face lock, 0 for nolock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 0);
	if (ret) {
		HISI_AOD_ERR("aod stop failed! retry (tag is %d, cmd is %d)\n", pkg_ap.tag, pkg_ap.cmd);
		ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 0);
	}
	if (pkg_mcu.errno != 0) {
		HISI_AOD_ERR("errno = %d \n", pkg_mcu.errno);
	} else {
		memcpy(g_aod_pause_data->data,
			(const void *)(pkg_mcu.data), pkg_mcu.data_length);
		ret = HISI_AOD_OK;
	}
	hisi_disable_sh_ipc_and_blank();
	mutex_unlock(&(g_aod_data->aod_lock));
	g_aod_data->aod_lock_status = true;
	HISI_AOD_INFO("release aod_lock\n");
	fast_unlock_mode_handle(dpufd);
	HISI_AOD_INFO("-\n");
	return ret;
}

static int hisi_aod_set_finger_status(struct aod_data_t *aod_data, const void __user* arg)
{
	aod_notify_finger_down_t finger_config;
	if ((NULL == arg)||(NULL == aod_data))
	{
		HISI_AOD_ERR("arg or aod_data is NULL Pointer!\n");
		return -EINVAL;
	}
	if(copy_from_user(&finger_config, arg, sizeof(finger_config)))
	{
		return -EFAULT;
	}
	HISI_AOD_INFO("finger_down_status changed, old is %u, new is %u.\n",
	aod_data->finger_down_status,
	finger_config.finger_down_status);
	aod_data->finger_down_status = finger_config.finger_down_status;
	return 0;
}

static int hisi_aod_get_dynamic_fb(struct aod_data_t *aod_data, const void __user* arg)
{
	struct ion_handle *handle = NULL;
	int ret;
	uint32_t i;
	unsigned long buf_addr;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	uint32_t tmp;
#endif
	uint64_t count;
	aod_dynamic_fb_space_t ion_fb_config;
	if ((arg == NULL) || (aod_data == NULL)) {
		HISI_AOD_ERR("arg or aod_data is NULL Pointer!\n");
		return -EINVAL;
	}
	if (copy_from_user(&ion_fb_config, arg, sizeof(ion_fb_config)))
		return -EFAULT;
	count = ion_fb_config.dynamic_ext_fb_count +
		ion_fb_config.dynamic_fb_count +
		ion_fb_config.face_id_fb_count +
		ion_fb_config.pd_logo_fb_count;
#if SHMEM_START_CONFIG
	count += ion_fb_config.digital_clock_count +
		ion_fb_config.analog_clock_count +
		ion_fb_config.pattern_clock_count +
		ion_fb_config.dynamic_reserve_count;
#endif
	if (count > MAX_DYNAMIC_ALLOC_FB_COUNT) {
		HISI_AOD_ERR("ion_fb_config.fb_count > %d !\n",
			MAX_DYNAMIC_ALLOC_FB_COUNT);
		return -EINVAL;
	}
	if (aod_data->ion_dynamic_alloc_flag != false) {
		HISI_AOD_ERR("Memory has been allocated!\n");
		return -EINVAL;
	}
	for (i = 0; i < count; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
		ret = hisif_aod_ion_phys(ion_fb_config.str_ion_fb[i].ion_buf_fb,
			aod_data->dev, &buf_addr);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
		handle = ion_import_dma_buf_fd(aod_data->ion_client,
			ion_fb_config.str_ion_fb[i].ion_buf_fb);
#else
		handle = ion_import_dma_buf(aod_data->ion_client,
			ion_fb_config.str_ion_fb[i].ion_buf_fb);
#endif
		if (IS_ERR(handle)) {
			HISI_AOD_ERR("ion_import_dma_buf fail, ion_buf_fb %d, size %u, index %u!\n",
				ion_fb_config.str_ion_fb[i].ion_buf_fb,
				ion_fb_config.str_ion_fb[i].buf_size,
				i);
			continue;
		}
		tmp = ion_fb_config.str_ion_fb[i].buf_size;
		ret = dpufb_ion_phys(aod_data->ion_client, handle,
			aod_data->dev, &buf_addr, (size_t *)&tmp);
#endif
		if (ret < 0) {
			HISI_AOD_ERR("ion_phys fail, ion_buf_fb %d, size %u, index %u!\n",
				ion_fb_config.str_ion_fb[i].ion_buf_fb,
				ion_fb_config.str_ion_fb[i].buf_size,
				i);
			continue;
		}
		if (buf_addr > MAX_ADDR_FOR_SENSORHUB) {
			HISI_AOD_ERR("phys addr(0x%lx) invalid, ion_buf_fb %d, size %u, index %u!\n",
				buf_addr,
				ion_fb_config.str_ion_fb[i].ion_buf_fb,
				ion_fb_config.str_ion_fb[i].buf_size,
				i);
			continue;
		}
		aod_data->ion_dyn_handle[i] = handle;
		aod_data->start_config_mcu.dynamic_fb_addr[i] = (uint32_t)buf_addr;
		aod_data->ion_dynamic_alloc_flag = true;
	}
	aod_data->start_config_mcu.dynamic_fb_count =
		ion_fb_config.dynamic_fb_count;
	aod_data->start_config_mcu.dynamic_ext_fb_count =
		ion_fb_config.dynamic_ext_fb_count;
	aod_data->start_config_mcu.face_id_fb_count =
		ion_fb_config.face_id_fb_count;
	aod_data->start_config_mcu.pd_logo_fb_count =
		ion_fb_config.pd_logo_fb_count;
#if SHMEM_START_CONFIG
	aod_data->start_config_mcu.digital_clock_count =
		ion_fb_config.digital_clock_count;
	aod_data->start_config_mcu.analog_clock_count =
		ion_fb_config.analog_clock_count;
	aod_data->start_config_mcu.pattern_clock_count =
		ion_fb_config.pattern_clock_count;
	aod_data->start_config_mcu.dynamic_reserve_count =
		ion_fb_config.dynamic_reserve_count;
#endif
	hisi_dump_start_config(&aod_data->start_config_mcu);
	return 0;

}

static int dynamic_fb_data_check(struct aod_data_t *aod_data,
	const void __user *arg, uint32_t *len)
{
	uint32_t max_count;

	if ((arg == NULL) || (aod_data == NULL)) {
		HISI_AOD_ERR("arg or aod_data is NULL Pointer!\n");
		return -EINVAL;
	}
	if (copy_from_user(len, arg, sizeof(uint32_t))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	HISI_AOD_INFO("len is %d\n", *len);
	if (g_ion_fb_config != NULL) {
		kfree(g_ion_fb_config);
		g_ion_fb_config = NULL;
	}
	if ((*len < sizeof(struct fb_list)) || (*len > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("len < 12 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_ion_fb_config = kzalloc(*len, GFP_KERNEL);
	if (!g_ion_fb_config) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(g_ion_fb_config, arg, *len)) {
		HISI_AOD_ERR("copy_from user failed\n");
		kfree(g_ion_fb_config);
		g_ion_fb_config = NULL;
		return -EFAULT;
	}
	if (g_ion_fb_config->size != *len) {
		HISI_AOD_ERR("err size=%u, *len=%u\n", g_ion_fb_config->size, *len);
		kfree(g_ion_fb_config);
		g_ion_fb_config = NULL;
		return -EFAULT;
	}
	HISI_AOD_INFO("fb_count is %u\n", g_ion_fb_config->dynamic_fb_count);
	max_count = (*len - sizeof(struct fb_list)) / sizeof(struct fb_buf);
	HISI_AOD_INFO("max_count is %u\n", max_count);
	if (g_ion_fb_config->dynamic_fb_count > max_count) {
		HISI_AOD_ERR("dynamic_fb_count > max_count\n");
		kfree(g_ion_fb_config);
		g_ion_fb_config = NULL;
		return HISI_AOD_FAIL;
	}
	return 0;
}

static int ion_fb_config_send(uint32_t len)
{
	uint32_t i;
	uint32_t length;
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	int ret = 0;

	for (i = 0; i < (len / sizeof(uint32_t)); i++)
		HISI_AOD_INFO("@@@@ptr[%d] = %u\n",
			i, *((uint32_t *)g_ion_fb_config + i));
	if (sizeof(uint32_t) > len) {
		HISI_AOD_ERR("offset is bigger than len\n");
		return ret;
	}
	length = len - sizeof(uint32_t);
	if (length > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
		ret = shmem_send(TAG_AOD, &(g_ion_fb_config->cmd_type), length);
		HISI_AOD_INFO("shmem_send tag is %d, size %ud\n", TAG_AOD, length);
		return ret;
	}
	memset(&pkg_mcu, 0, sizeof(pkg_mcu)); // unsafe_function_ignore: memset
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(&(g_ion_fb_config->cmd_type));
	pkg_ap.wr_len = length;
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}
	HISI_AOD_INFO("errno = %d\n", pkg_mcu.errno);
	return ret;
}

static int hisi_aod_get_dynamic_fb_new(struct aod_data_t *aod_data,
	const void __user *arg)
{
	int ret = 0;
	uint32_t i;
	unsigned long buf_addr;
	uint32_t count;
	uint32_t len;

	ret = dynamic_fb_data_check(aod_data, arg, &len);
	if (ret)
		return ret;
	count = g_ion_fb_config->dynamic_fb_count;
	HISI_AOD_INFO("count is %d\n", count);
	if (aod_data->ion_dynamic_alloc_flag != false) {
		HISI_AOD_ERR("Memory has been allocated!\n");
		return -EINVAL;
	}
	for (i = 0; i < count; i++) {
		ret = hisif_aod_ion_phys(g_ion_fb_config->fb[i].addr,
			aod_data->dev, &buf_addr);
		if (ret < 0) {
			HISI_AOD_ERR("ion_phys fail, ion_buf_fb %u, size %u, index %u!\n",
				g_ion_fb_config->fb[i].addr,
				g_ion_fb_config->fb[i].size, i);
			continue;
		}
		if (buf_addr > MAX_ADDR_FOR_SENSORHUB) {
			HISI_AOD_ERR("phys addr(0x%lx) invalid, ion_buf_fb %u, size %u, index %u!\n",
				buf_addr, g_ion_fb_config->fb[i].addr,
				g_ion_fb_config->fb[i].size, i);
			continue;
		}
		g_ion_fb_config->fb[i].addr = buf_addr;
		HISI_AOD_INFO("phys addr(0x%lx), ion_buf_fb %u, size %u, index %u!\n",
				buf_addr, g_ion_fb_config->fb[i].addr,
				g_ion_fb_config->fb[i].size, i);
		aod_data->ion_dynamic_alloc_flag = true;
	}
	ret = ion_fb_config_send(len);
	return ret;
}

static int hisi_aod_free_dynamic_fb_new(struct aod_data_t *aod_data, void __user *arg)
{
	uint32_t i;
	uint32_t count;

	if (aod_data == NULL) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}
	if (aod_data->ion_dynamic_alloc_flag != true) {
		HISI_AOD_ERR("Memory has been released!\n");
		return -EINVAL;
	}
	count = g_ion_fb_config->dynamic_fb_count;
	for (i = 0; i < count; i++) {
		if (aod_data->ion_dyn_handle[i] == NULL)
			continue;
		aod_data->ion_dyn_handle[i] = NULL;
		g_ion_fb_config->fb[i].addr = 0;
	}
	g_ion_fb_config->dynamic_fb_count = 0;
	aod_data->ion_dynamic_alloc_flag = false;
	return 0;
}

static int hisi_aod_free_dynamic_fb(struct aod_data_t *aod_data, void __user* arg)
{
	uint32_t i;
	uint64_t count;

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}
	if (aod_data->ion_dynamic_alloc_flag != true) {
		HISI_AOD_ERR("Memory has been released!\n");
		return -EINVAL;
	}
	count = aod_data->start_config_mcu.dynamic_fb_count +
		aod_data->start_config_mcu.dynamic_ext_fb_count +
		aod_data->start_config_mcu.face_id_fb_count +
		aod_data->start_config_mcu.pd_logo_fb_count;
#if SHMEM_START_CONFIG
	count += aod_data->start_config_mcu.digital_clock_count +
		aod_data->start_config_mcu.analog_clock_count +
		aod_data->start_config_mcu.pattern_clock_count +
		aod_data->start_config_mcu.dynamic_reserve_count;
#endif
	if (count > MAX_DYNAMIC_ALLOC_FB_COUNT) {
		HISI_AOD_ERR("count is too long\n");
		return -EINVAL;
	}
	for (i = 0; i < count; i++) {
		if (aod_data->ion_dyn_handle[i] == NULL)
			continue;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
		ion_free(aod_data->ion_client, aod_data->ion_dyn_handle[i]);
#endif
		aod_data->ion_dyn_handle[i] = NULL;
		aod_data->start_config_mcu.dynamic_fb_addr[i] = 0;
	}
	aod_data->start_config_mcu.dynamic_fb_count = 0;
	aod_data->start_config_mcu.dynamic_ext_fb_count = 0;
	aod_data->start_config_mcu.face_id_fb_count = 0;
	aod_data->start_config_mcu.pd_logo_fb_count = 0;
#if SHMEM_START_CONFIG
	aod_data->start_config_mcu.digital_clock_count = 0;
	aod_data->start_config_mcu.analog_clock_count = 0;
	aod_data->start_config_mcu.pattern_clock_count = 0;
	aod_data->start_config_mcu.dynamic_reserve_count = 0;
#endif
	aod_data->ion_dynamic_alloc_flag = false;
	return 0;
}


static int hisi_aod_resume(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret = 0;
	aod_resume_config_t resume_config;

	HISI_AOD_INFO("+.\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if(copy_from_user(&resume_config, arg, sizeof(resume_config)))
		return -EFAULT;

	if((resume_config.aod_pos.x_start >= aod_data->x_res) || (resume_config.aod_pos.y_start >= aod_data->y_res)
		|| (resume_config.intelli_switching < 0)) {
		HISI_AOD_ERR("invalid resume_config:x_start=%d, y_start=%d,intelli_switching=%d\n",
					resume_config.aod_pos.x_start,resume_config.aod_pos.y_start,resume_config.intelli_switching);
		return -EINVAL;
	}

	down(&(aod_data->aod_status_sem));
	if (aod_data->aod_status) {
		HISI_AOD_INFO("already in aod mode.\n");
		up(&(aod_data->aod_status_sem));
		return 0;
	}
	up(&(aod_data->aod_status_sem));

	aod_data->start_config_mcu.aod_pos.x_start = resume_config.aod_pos.x_start;
	aod_data->start_config_mcu.aod_pos.y_start = resume_config.aod_pos.y_start;
	aod_data->start_config_mcu.intelli_switching = resume_config.intelli_switching;
	aod_data->start_config_mcu.aod_type = resume_config.aod_type;
	aod_data->start_config_mcu.fp_mode = resume_config.fp_mode;

	ret = hisi_aod_start_req(&aod_data->start_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_start_req fail\n");
		return ret;
	}

	HISI_AOD_INFO("- .\n");

	return 0;
}

static int hisi_start_updating(struct aod_data_t *aod_data, void __user* arg)
{
	int ret = 0;
	aod_start_updating_pos_data_t start_updating_pos;

	HISI_AOD_DEBUG("+\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if (!aod_data->aod_status) {
		HISI_AOD_ERR("not in aod status(%d)!\n", aod_data->aod_status);
		return -EINVAL;
	}

	ret = hisi_aod_start_updating_req(&aod_data->pos_data);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_start_updating_req fail\n");
		return ret;
	}

	start_updating_pos.aod_pos.x_start = aod_data->pos_data.x_start;
	start_updating_pos.aod_pos.y_start = aod_data->pos_data.y_start;
	start_updating_pos.size = sizeof(start_updating_pos);

	if(copy_to_user(arg, &start_updating_pos, start_updating_pos.size))
		return -EFAULT;

	HISI_AOD_DEBUG("-\n");

	return 0;
}

static int hisi_set_max_and_min_backlight(struct aod_data_t *aod_data, const void __user* arg)
{
	return 0;
}

static int hisi_end_updating(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret = 0;
	aod_end_updating_pos_data_t end_updating_pos;

	HISI_AOD_DEBUG("+\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if (!aod_data->aod_status) {
		HISI_AOD_ERR("not in aod status(%d)!\n", aod_data->aod_status);
		return -EINVAL;
	}

	if(copy_from_user(&end_updating_pos, arg, sizeof(end_updating_pos)))
		return -EFAULT;

	if((end_updating_pos.aod_pos.x_start >= aod_data->x_res) || (end_updating_pos.aod_pos.y_start >= aod_data->y_res)) {
		HISI_AOD_ERR("invalid end_updating_pos:x_start=%d,y_start=%d\n",
			end_updating_pos.aod_pos.x_start, end_updating_pos.aod_pos.y_start);
		return -EINVAL;
	}

	aod_data->pos_data.x_start = end_updating_pos.aod_pos.x_start;
	aod_data->pos_data.y_start = end_updating_pos.aod_pos.y_start;

	ret = hisi_aod_end_updating_req(&aod_data->pos_data, end_updating_pos.aod_type);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_end_updating_req fail\n");
		return ret;
	}

	HISI_AOD_DEBUG("-\n");

	return 0;
}

static int hisi_aod_stop(struct aod_data_t *aod_data, void __user* arg)
{
	int ret = 0;
	aod_stop_pos_data_t stop_disp_pos;

	HISI_AOD_INFO("+.\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if (!aod_data->aod_status) {
		HISI_AOD_INFO("already exit aod mode.\n");
		return 0;
	}

	ret = hisi_aod_stop_req(&aod_data->pos_data);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_stop_req fail\n");
		return ret;
	}

	down(&(aod_data->aod_status_sem));
	aod_data->aod_status = false;
	up(&(aod_data->aod_status_sem));

	stop_disp_pos.aod_pos.x_start = aod_data->pos_data.x_start;
	stop_disp_pos.aod_pos.y_start = aod_data->pos_data.y_start;
	stop_disp_pos.size = sizeof(stop_disp_pos);

	if(copy_to_user(arg, &stop_disp_pos, stop_disp_pos.size)) {
		HISI_AOD_ERR("copy_to_user fail!\n");
		return -EFAULT;
	}
	HISI_AOD_INFO("- .\n");

	return 0;
}

static int hisi_get_panel_info(struct aod_data_t *aod_data, void __user* arg)
{
	aod_screen_info_t st_screen_info;

	HISI_AOD_INFO("+.\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	st_screen_info.pixel_format = AOD_FB_PIXEL_FORMAT_RGB_565;
	st_screen_info.xres = aod_data->x_res;
	st_screen_info.yres = aod_data->y_res;

	if(copy_to_user(arg, &st_screen_info, sizeof(st_screen_info)))
		return -EFAULT;

	HISI_AOD_INFO("-.\n");
	return 0;
}

static int hisi_free_buffer(struct aod_data_t *aod_data, void __user* arg)
{
	HISI_AOD_INFO("+.\n");

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if(aod_data->fb_mem_alloc_flag) {
		dma_free_coherent(aod_data->cma_device, aod_data->smem_size, aod_data->buff_virt, aod_data->buff_phy);

		aod_data->fb_mem_alloc_flag = false;
	}

	HISI_AOD_INFO("-.\n");

	return 0;
}

static int hisi_aod_set_common_info(struct aod_data_t *aod_data,
	const void __user *arg)
{
	int ret = 0;
	uint32_t i;
	uint32_t length = 0;
	uint32_t len;

	if ((!arg) || (!aod_data)) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	if (copy_from_user(&length, arg, sizeof(uint32_t))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}

	if (g_aod_common_data != NULL) {
		kfree(g_aod_common_data);
		g_aod_common_data = NULL;
	}
	if ((length < sizeof(struct aod_common_data)) || (length > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("length < 8 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_aod_common_data = kzalloc(length, GFP_KERNEL);
	if (!g_aod_common_data) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(g_aod_common_data, arg, length)) {
		HISI_AOD_ERR("copy_from_user failed\n");
		kfree(g_aod_common_data);
		g_aod_common_data = NULL;
		return -EFAULT;
	}
	if (g_aod_common_data->size != length) {
		HISI_AOD_ERR("err size=%u, length=%u\n", g_aod_common_data->size, length);
		kfree(g_aod_common_data);
		g_aod_common_data = NULL;
		return -EFAULT;
	}
	/* print common info */
	for (i = 0; i < (length / 4); i++)
		HISI_AOD_DEBUG("@@@@ptr[%d] = %u\n",
			i, *((uint32_t *)g_aod_common_data + i));
	len = length - (sizeof(uint32_t) + sizeof(uint32_t));
#ifdef CONFIG_CONTEXTHUB_SHMEM
	ret = shmem_send(TAG_AOD, g_aod_common_data->data, len);
	HISI_AOD_INFO("shmem_send tag is %d, size %ud\n", TAG_AOD, len);
#endif
	if (ret)
		HISI_AOD_ERR("common info send to fail, ret = %d\n", ret);
	return ret;
}

static int gmp_info_send_to_sensorhub(uint32_t length)
{
	struct write_info pkg_ap;
	uint32_t len;
	int ret = 0;

	if (offsetof(struct aod_common_data, data) > length) {
		HISI_AOD_ERR("offset is bigger than len\n");
		return ret;
	}
	len = length - offsetof(struct aod_common_data, data);
	if (len > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
		ret = shmem_send(TAG_AOD, g_aod_gmp_data->data, len);
		HISI_AOD_INFO("shmem_send tag is %d, size %ld\n", TAG_AOD, len);
		return ret;
	}
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(g_aod_gmp_data->data);
	pkg_ap.wr_len = len;
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}
	return ret;
}

static int hisi_aod_set_gmp_info(struct aod_data_t *aod_data,
	const void __user *arg)
{
	int ret = 0;
	uint32_t length = 0;
	uint32_t i;

	for (i = 0; i < MAX_MULTI_GMP_SIZE; i++) {
		if (g_aod_multi_gmp_data[i] != NULL) {
			HISI_AOD_INFO("free g_aod_multi_gmp_data[%u]\n", i);
			kfree(g_aod_multi_gmp_data[i]);
			g_aod_multi_gmp_data[i] = NULL;
		}
	}

	if ((!arg) || (!aod_data)) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	if (copy_from_user(&length, arg, sizeof(uint32_t))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	HISI_AOD_INFO("length = %d\n", length);
	if (g_aod_gmp_data != NULL) {
		kfree(g_aod_gmp_data);
		g_aod_gmp_data = NULL;
	}
	if ((length < sizeof(struct aod_common_data)) || (length > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("length < 8 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_aod_gmp_data = kzalloc(length, GFP_KERNEL);
	if (!g_aod_gmp_data) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(g_aod_gmp_data, arg, length)) {
		HISI_AOD_ERR("copy_from_user failed\n");
		kfree(g_aod_gmp_data);
		g_aod_gmp_data = NULL;
		return -EFAULT;
	}
	if (g_aod_gmp_data->size != length) {
		HISI_AOD_ERR("err size=%u len=%u\n", g_aod_gmp_data->size, length);
		kfree(g_aod_gmp_data);
		g_aod_gmp_data = NULL;
		return -EFAULT;
	}
	ret = gmp_info_send_to_sensorhub(length);
	return ret;
}

static int gen_info_send_to_sensorhub(uint32_t length,
	struct aod_general_data *aod_gen_data)
{
	int ret = 0;
	uint32_t i;
	uint32_t len;
	struct write_info pkg_ap;

	if (aod_gen_data == NULL) {
		HISI_AOD_ERR("aod_gen_data is NULL\n");
		return ret;
	}
	/* print general info */
	for (i = 0; i < (length / 4); i++)
		HISI_AOD_INFO("@@@@ptr[%d] = %u\n", i,
			*((uint32_t *)aod_gen_data + i));
	if (offsetof(struct aod_general_data, data) > length) {
		HISI_AOD_ERR("offset is bigger than len\n");
		return ret;
	}
	len = length - offsetof(struct aod_general_data, data);
	if (len > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
		ret = shmem_send(TAG_AOD, aod_gen_data->data, len);
		HISI_AOD_INFO("shmem_send tag is %d, size %ud\n", TAG_AOD, len);
		return ret;
	}
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(aod_gen_data->data);
	pkg_ap.wr_len = len;
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}
	return ret;
}

static int aod_gen_data_process(struct aod_general_data *aod_gen_data,
	uint32_t length)
{
	uint32_t cmd_type = aod_gen_data->data[0];

	if (cmd_type >= MAX_AOD_GEN_EVENT) {
		HISI_AOD_ERR("cmd_type is too big\n");
		return -EINVAL;
	}
	if (g_aod_gen_data[cmd_type] != NULL) {
		kfree(g_aod_gen_data[cmd_type]);
		g_aod_gen_data[cmd_type] = NULL;
	}
	if ((length <= MIN_ALLOC_SIZE) || (length > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("length <= 0 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_aod_gen_data[cmd_type] = kzalloc(length, GFP_KERNEL);
	if (!(g_aod_gen_data[cmd_type])) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	memcpy((char *)g_aod_gen_data[cmd_type], (char *)aod_gen_data, length);
	return 0;
}

static int hisi_aod_set_gen_info(struct aod_data_t *aod_data,
	const void __user *arg)
{
	int ret;
	uint32_t length = 0;
	struct aod_general_data *aod_gen_data = NULL;

	if ((!arg) || (!aod_data)) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	if (copy_from_user(&length, arg, sizeof(uint32_t))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	if ((length < sizeof(struct aod_general_data)) || (length > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("length < 4 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	aod_gen_data = kzalloc(length, GFP_KERNEL);
	if (!aod_gen_data) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(aod_gen_data, arg, length)) {
		HISI_AOD_ERR("copy_from_user failed\n");
		kfree(aod_gen_data);
		aod_gen_data = NULL;
		return -EFAULT;
	}
	if (aod_gen_data->size != length) {
		HISI_AOD_ERR("err size=%u len=%u\n", aod_gen_data->size, length);
		kfree(aod_gen_data);
		aod_gen_data = NULL;
		return -EFAULT;
	}
	ret = gen_info_send_to_sensorhub(length, aod_gen_data);
	if (ret) {
		kfree(aod_gen_data);
		aod_gen_data = NULL;
		return ret;
	}
	ret = aod_gen_data_process(aod_gen_data, length);
	kfree(aod_gen_data);
	aod_gen_data = NULL;
	return ret;
}

static int hisi_aod_fold_info_req(aod_fold_info_config_mcu_t *fold_info_config)
{
	int ret;
	struct write_info pkg_ap;
	aod_pkt_pack_t pkt;
	struct pkt_header *hd = (struct pkt_header *)&pkt;

	HISI_AOD_INFO("+\n");

	if (fold_info_config == NULL) {
		HISI_AOD_ERR("fold_config is NULL Pointer\n");
		return -1;
	}
	memset(&pkg_ap, 0, sizeof(pkg_ap));
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_SET_FOLD_INFO_REQ;
	pkt.fold_info_config_param.panel_id = fold_info_config->panel_id;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	/* 1 for lock */
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_INFO("-\n");
	return ret;
}
static int hisi_aod_set_fold_info(struct aod_data_t *aod_data,
	const void __user *arg)
{
	int ret;
	aod_fold_info_data_t aod_fold_info;

	HISI_AOD_INFO("+.\n");

	if (arg == NULL) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	if (aod_data == NULL) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}
	memset(&aod_fold_info, 0, sizeof(aod_fold_info));
	if (copy_from_user(&aod_fold_info, arg, sizeof(aod_fold_info))) {
		HISI_AOD_INFO("copy from user fail!\n.\n");
		return -EFAULT;
	}
	aod_data->fold_info_config_mcu.panel_id =
		aod_fold_info.panel_id;
	ret = hisi_aod_fold_info_req(&aod_data->fold_info_config_mcu);
	if (ret) {
		HISI_AOD_ERR("hisi_aod_fold_info_req fail\n");
		return ret;
	}
	hisi_aod_fold_panel_id = aod_data->fold_info_config_mcu.panel_id;

	HISI_AOD_INFO("- fold_panel_id 0x%x.\n", hisi_aod_fold_panel_id);
	return 0;
}

static int multi_gmp_info_send_to_sensorhub(uint32_t length, struct aod_multi_gmp_data *multi_gmp_data)
{
	struct write_info pkg_ap;
	uint32_t len;
	int ret = 0;

	HISI_AOD_INFO("multi gmp send to sensorhub start\n");
	if (offsetof(struct aod_multi_gmp_data, data) > length) {
		HISI_AOD_ERR("offset is bigger than len\n");
		return ret;
	}
	len = length - offsetof(struct aod_multi_gmp_data, data);
	if (len > IPC_MAX_SIZE - sizeof(struct pkt_header)) {
		ret = shmem_send(TAG_AOD, multi_gmp_data->data, len);
		HISI_AOD_INFO("shmem_send tag is %d, size %ld\n", TAG_AOD, len);
		return ret;
	}
	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = (const void *)(multi_gmp_data->data);
	pkg_ap.wr_len = len;
	ret = hisi_aod_send_cmd_to_sensorhub(&pkg_ap, NULL, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}
	HISI_AOD_INFO("multi gmp send to sensorhub end\n");
	return ret;
}

static void aod_multi_gmp_data_to_sensorhub(void)
{
	uint32_t i;

	HISI_AOD_INFO("multi gmp send start");
	for (i = 0; i < MAX_MULTI_GMP_SIZE; i++) {
		if (g_aod_multi_gmp_data[i] == NULL) {
			HISI_AOD_INFO("g_aod_multi_gmp_data[%u] is NULL\n", i);
			continue;
		}
		multi_gmp_info_send_to_sensorhub(g_aod_multi_gmp_data[i]->size, g_aod_multi_gmp_data[i]);
	}
	HISI_AOD_INFO("multi gmp send end");
}

static int dpu_aod_set_multi_gmp_info(struct aod_data_t *aod_data, const void __user *arg)
{
	int ret = 0;
	struct aod_multi_gmp_data data;

	if (g_aod_gmp_data != NULL) {
		HISI_AOD_INFO("free g_aod_gmp_data\n");
		kfree(g_aod_gmp_data);
		g_aod_gmp_data= NULL;
	}

	if ((!arg) || (!aod_data)) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}
	if (copy_from_user(&data, arg, sizeof(struct aod_multi_gmp_data))) {
		HISI_AOD_ERR("copy_from_user failed\n");
		return -EFAULT;
	}
	HISI_AOD_INFO("length = %d, index = %d\n", data.size, data.index);
	if (data.index >= MAX_MULTI_GMP_SIZE) {
		HISI_AOD_ERR("index err\n");
		return EINVAL;
	}
	if (g_aod_multi_gmp_data[data.index] != NULL) {
		kfree(g_aod_multi_gmp_data[data.index]);
		g_aod_multi_gmp_data[data.index] = NULL;
	}
	if ((data.size < sizeof(struct aod_multi_gmp_data)) || (data.size > MAX_ALLOC_SIZE)) {
		HISI_AOD_ERR("length < 8 or > MAX_ALLOC_SIZE\n");
		return HISI_AOD_FAIL;
	}
	g_aod_multi_gmp_data[data.index] = kzalloc(data.size, GFP_KERNEL);
	if (!g_aod_multi_gmp_data[data.index]) {
		HISI_AOD_ERR("alloc memory failed\n");
		return -ENOMEM;
	}
	if (copy_from_user(g_aod_multi_gmp_data[data.index], arg, data.size)) {
		HISI_AOD_ERR("copy_from_user failed\n");
		kfree(g_aod_multi_gmp_data[data.index]);
		g_aod_multi_gmp_data[data.index] = NULL;
		return -EFAULT;
	}
	if (g_aod_multi_gmp_data[data.index]->size != data.size) {
		HISI_AOD_ERR("err size=%u len=%u\n", g_aod_multi_gmp_data[data.index]->size, data.size);
		kfree(g_aod_multi_gmp_data[data.index]);
		g_aod_multi_gmp_data[data.index] = NULL;
		return -EFAULT;
	}
	ret = multi_gmp_info_send_to_sensorhub(data.size, g_aod_multi_gmp_data[data.index]);
	return ret;
}

static long hisi_aod_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *argp = (void __user *)(uintptr_t)arg;
	struct aod_data_t *aod_data = (struct aod_data_t*)file->private_data;

	if(NULL == aod_data) {
		HISI_AOD_ERR("aod_data is NULL pinter\n");
		return -EINVAL;
	}

	mutex_lock(&aod_data->ioctl_lock);

	switch (cmd) {
	case AOD_IOCTL_AOD_SET_BITMAP_SIZE:
		ret = hisi_set_bitmap_size(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_TIME:
		ret = hisi_set_time(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_DISPLAY_SPACE:
		ret = hisi_set_display_space(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_START:
		ret = hisi_aod_start(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_PAUSE:
		ret = hisi_aod_pause(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_RESUME:
		ret = hisi_aod_resume(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_START_UPDATING:
		ret = hisi_start_updating(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_END_UPDATING:
		ret = hisi_end_updating(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_STOP:
		ret = hisi_aod_stop(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_GET_PANEL_INFO:
		ret = hisi_get_panel_info(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_FREE_BUF:
		ret = hisi_free_buffer(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_FINGER_STATUS:
		ret = hisi_aod_set_finger_status(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_GET_DYNAMIC_FB:
		ret = hisi_aod_get_dynamic_fb(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_FREE_DYNAMIC_FB:
		ret = hisi_aod_free_dynamic_fb(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_MAX_AND_MIN_BACKLIGHT:
		ret = hisi_set_max_and_min_backlight(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_COMMON_SENSORHUB:
		ret = hisi_aod_set_common_info(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_PAUSE_NEW:
		ret = hisi_aod_pause_new(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_GET_POS:
		ret = hisi_aod_get_pos(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_GMP:
		ret = hisi_aod_set_gmp_info(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_GENERAL_SENSORHUB:
		ret = hisi_aod_set_gen_info(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_GET_DYNAMIC_FB_NEW:
		ret = hisi_aod_get_dynamic_fb_new(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_FREE_DYNAMIC_FB_NEW:
		ret = hisi_aod_free_dynamic_fb_new(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_FOLD_INFO:
		ret = hisi_aod_set_fold_info(aod_data, argp);
		break;
	case AOD_IOCTL_AOD_SET_MULTI_GMP:
		ret = dpu_aod_set_multi_gmp_info(aod_data, argp);
		break;
	default:
		HISI_AOD_ERR("unsupported ioctl (%x)\n", cmd);
		ret = -ENOSYS;
		break;
	}
	mutex_unlock(&aod_data->ioctl_lock);

	return ret;
}

static int hisi_aod_mmap(struct file *file, struct vm_area_struct * vma)
{
	struct aod_data_t *aod_data = NULL;
	unsigned long size = 0;
	unsigned long start = 0;
	unsigned long offset = 0;
	int ret = 0;

	HISI_AOD_INFO("+.\n");

	aod_data = (struct aod_data_t*)file->private_data;
	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data is NULL\n");
		return -EINVAL;
	}

	mutex_lock(&aod_data->mm_lock);

	HISI_AOD_INFO("vm_end is 0x%lx, vm_start is 0x%lx\n", vma->vm_end, vma->vm_start);

	size = vma->vm_end - vma->vm_start;
	start = vma->vm_start;
	offset = vma->vm_pgoff << PAGE_SHIFT;

	if(size > aod_data->max_buf_size || size <= 0) {
		HISI_AOD_ERR("size=%lu is out of range(%u)!\n", size, aod_data->max_buf_size);
		mutex_unlock(&aod_data->mm_lock);
		return -EINVAL;
	}

	if (vma->vm_pgoff > (~0UL >> PAGE_SHIFT)) {
		HISI_AOD_ERR("");
		mutex_unlock(&aod_data->mm_lock);
		return -EINVAL;
	}

	if (offset > aod_data->max_buf_size - size) {
		HISI_AOD_ERR();
		mutex_unlock(&aod_data->mm_lock);
		return -EINVAL;
	}

	aod_data->smem_size = vma->vm_end - vma->vm_start;

	if (!hisi_aod_fb_alloc(aod_data)) {
		HISI_AOD_ERR("hisi_aod_fb_alloc failed!\n");
		mutex_unlock(&aod_data->mm_lock);
		return -ENOMEM;
	}
	HISI_AOD_INFO("fb_start_addr is 0x%lx(%lu), smem_size is %lu\n", aod_data->fb_start_addr,
		aod_data->fb_start_addr, aod_data->smem_size);

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	ret = vm_iomap_memory(vma, aod_data->fb_start_addr, aod_data->smem_size);

	mutex_unlock(&aod_data->mm_lock);

	return ret;
}

#define	HISI_AOD_SET_POWER_MODE_TRYLOCK_TIMES		600
#define	HISI_AOD_TRYLOCK_WAITTIME	5 // ms
#define	MUTEX_LOCK_SUCCESS	1
#define	MUTEX_LOCK_FAILURE	0
// AP trylock aod_lock, and if AP get aod_lock, that means Sensorhub has release the authority of the DSS operation,
// Otherwise means that Sensorhub is operating DSS and AP should wait until Sensorhub release the aod_lock. If AP wait for
// Sensorhub for 3 seconds and still can't get the aod_lock, there may be something wrong happens with whatever User
// done. Just in case we got the issue for AP waiting aod_lock over 3 seconds, we need to force stop Sensorhub's DSS operating
// and put the DSS operating authority back to AP.
static int hisi_aod_trylock(struct aod_data_t *aod_data){
	int retry_count = 0;
	int trylock_result = MUTEX_LOCK_FAILURE;

	if (!aod_data) {
		HISI_AOD_INFO("aod_data is NULL.\n");
		return -1;
	}

	while (MUTEX_LOCK_FAILURE == (trylock_result = mutex_trylock(&(aod_data->aod_lock)))) {
		mdelay(HISI_AOD_TRYLOCK_WAITTIME);
		retry_count++;
		if (HISI_AOD_SET_POWER_MODE_TRYLOCK_TIMES == retry_count) {
			mutex_lock(&aod_data->ioctl_lock);
			HISI_AOD_INFO("aod force stop\n");
			hisi_send_aod_stop();
			mutex_unlock(&aod_data->ioctl_lock);
			break;
		}
	}
	return trylock_result;
}

void hisi_aod_wait_stop_nolock(void)
{
	int retry_count = 0;

	while (hisi_aod_get_aod_status()) {
		mdelay(HISI_AOD_TRYLOCK_WAITTIME);
		retry_count++;
		if (retry_count == HISI_AOD_SET_POWER_MODE_TRYLOCK_TIMES) {
			mutex_lock(&g_aod_data->ioctl_lock);
			HISI_AOD_INFO("aod force stop\n");
			hisi_send_aod_stop();
			mutex_unlock(&g_aod_data->ioctl_lock);
			break;
		}
	}
}

/*lint -e454, -e455*/
//lint -efunc(456,455,454,*)
int hisi_aod_set_blank_mode(int blank_mode)
{
	struct aod_data_t *aod_data = g_aod_data;
	struct hisi_fb_data_type * dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	HISI_AOD_INFO("+.\n");
	if (!aod_data) {
		HISI_AOD_INFO("aod_data is NULL.\n");
		return -1;
	}

	if (blank_mode == aod_data->blank_mode) {
		return -1;
	}

	HISI_AOD_INFO("finger_down_status is %d", g_aod_data->finger_down_status);

	if ((blank_mode == FB_BLANK_UNBLANK)
		&& (aod_data->blank_mode == FB_BLANK_POWERDOWN)
		&& (aod_data->aod_status == true)
		&& (g_aod_data->finger_down_status == 1
			|| g_aod_data->finger_down_status == 2
			|| g_aod_data->finger_down_status == 3)) {
		HISI_AOD_INFO("in fast unlock mode\n");
		return -3;
	}

	if ((blank_mode == FB_BLANK_UNBLANK)
		&& (aod_data->blank_mode == FB_BLANK_POWERDOWN)) {
		if (aod_data->start_req_faster) {
			HISI_AOD_INFO("no need enter aod\n");
			aod_data->no_need_enter_aod = true;
		}

		if(hisi_aod_inc_atomic(dpufd)) {
			HISI_AOD_INFO(" !!!!wait aod exit. try get aod_lock!!!!\n");
			if (MUTEX_LOCK_FAILURE == hisi_aod_trylock(aod_data)) {
				HISI_AOD_INFO("ap force get aod_lock\n");
				mutex_lock(&(aod_data->aod_lock));
				HISI_AOD_INFO("force hold aod_lock success\n");
			} else {
				HISI_AOD_INFO("try get aod_lock success\n");
			}
			aod_data->blank_mode = blank_mode;
			aod_data->aod_lock_status = true;
		}
	} else if ((blank_mode == FB_BLANK_POWERDOWN)
		&& (aod_data->blank_mode == FB_BLANK_UNBLANK)) {
		hisi_aod_dec_atomic(dpufd);

		mutex_unlock(&(aod_data->aod_lock));
		aod_data->aod_lock_status = false;
		HISI_AOD_INFO("release aod_lock\n");
		aod_data->blank_mode = blank_mode;
	}

	HISI_AOD_INFO("-.\n");

	return 0;
}
//lint +efunc(456,455,454,*)
/*lint +e454, +e455*/

bool hisi_aod_get_aod_status(void)
{
	if (g_aod_data == NULL)
		return false;
	return g_aod_data->aod_status;
}

bool hisi_aod_get_aod_lock_status(void)
{
	if (!g_aod_data)
		return true;
	return g_aod_data->aod_lock_status;
}

static int hisi_aod_open(struct inode* inode, struct file* file)
{
	struct aod_data_t *aod_data = NULL;
	int ret = 0;

	mutex_lock(&g_lock);
	HISI_AOD_INFO("+.\n");

	if(ref_cnt) {
		HISI_AOD_INFO("can not open opened aod device.\n");
		mutex_unlock(&g_lock);
		return -1;
	}

	if(!ref_cnt) {
		HISI_AOD_INFO("open aod device.\n");
		aod_data = container_of(inode->i_cdev, struct aod_data_t, cdev);
		if(NULL == aod_data) {
			HISI_AOD_ERR("aod_data is NULL pointer\n");
			mutex_unlock(&g_lock);
			return -1;
		}

		file->private_data = aod_data;
		ref_cnt ++;
	}

	ret = hisi_aod_sensorhub_cmd_req(CMD_CMN_OPEN_REQ);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_sensorhub_cmd_req open fail\n");
		mutex_unlock(&g_lock);
		return ret;
	}

	HISI_AOD_INFO("-.\n");
	mutex_unlock(&g_lock);
	return 0;
}

static int hisi_aod_release(struct inode* inode, struct file* file)
{
	struct aod_data_t *aod_data = NULL;

	mutex_lock(&g_lock);
	HISI_AOD_INFO("+.\n");
	if (!ref_cnt) {
		HISI_AOD_INFO("try to close unopened aod device!\n");
		mutex_unlock(&g_lock);
		return -EINVAL;
	}

	ref_cnt --;

	if(!ref_cnt) {
		HISI_AOD_INFO("close aod device.\n");
		aod_data = (struct aod_data_t*)file->private_data;
		if (NULL == aod_data) {
			HISI_AOD_ERR("aod_data is NULL\n");
			mutex_unlock(&g_lock);
			return -1;
		}
		HISI_AOD_INFO("wait ioctl_lock.\n");
		mutex_lock(&aod_data->ioctl_lock);
		HISI_AOD_INFO("got ioctl_lock.\n");
		(void)hisi_send_aod_stop();
		(void)hisi_aod_sensorhub_cmd_req(CMD_CMN_CLOSE_REQ);
		(void)hisi_aod_free_dynamic_fb(aod_data, NULL);
		(void)hisi_free_buffer(aod_data, NULL);
		mutex_unlock(&aod_data->ioctl_lock);
		HISI_AOD_INFO("release ioctl_lock.\n");
	}

	HISI_AOD_INFO("-.\n");
	mutex_unlock(&g_lock);
	return 0;
}

static struct file_operations hisi_aod_fops = {
	.owner = THIS_MODULE,
	.open = hisi_aod_open,
	.release = hisi_aod_release,
	.read = hisi_aod_read,
	.mmap = hisi_aod_mmap,
	.unlocked_ioctl = hisi_aod_ioctl,
};

static int hisi_aod_probe(struct platform_device *pdev)
{
	struct aod_data_t *aod_data = NULL;
	struct device *dev = &pdev->dev;
	struct device_node *np = NULL;
	uint32_t line_length = 0;
	uint32_t fb_max_len = 0;
	uint32_t bitmaps_max_len = 0;
	int i;
	int ret = 0;

	HISI_AOD_INFO("%s\n", __func__);

	aod_data = kzalloc(sizeof(struct aod_data_t), GFP_KERNEL);
	if(NULL == aod_data) {
		HISI_AOD_ERR("alloc memory for aod_data failed\n");
		ret = -ENOMEM;
		goto alloc_aod_data_fail;
	}

	aod_data->dev = dev;
	dev_set_drvdata(dev, aod_data);
	mutex_init(&g_lock);
	mutex_init(&dss_on_off_lock);
	mutex_init(&aod_data->ioctl_lock);
	mutex_init(&aod_data->mm_lock);
	wakeup_source_init(&aod_data->wlock, "wake_lock");

	aod_data->fb_mem_alloc_flag = false;
	aod_data->ion_dynamic_alloc_flag = false;
	aod_data->aod_dev_class = class_create(THIS_MODULE, "aod_device");
	if (IS_ERR(aod_data->aod_dev_class)) {
		HISI_AOD_ERR("Unable to create aod class; errno = %ld\n", PTR_ERR(aod_data->aod_dev_class));
		aod_data->aod_dev_class = NULL;
		ret = -ENOMEM;
		goto create_aod_dev_class_err;
	}

	ret = alloc_chrdev_region(&aod_data->devno, 0, 1, "aod");
	if (ret) {
		HISI_AOD_ERR("%s alloc_chrdev_region failed ret = %d\n",
			__func__, ret);
		goto alloc_chrdev_region_err;
	}

	aod_data->aod_cdevice = device_create(aod_data->aod_dev_class, NULL, aod_data->devno, NULL, "%s", "aod");
	if (NULL == aod_data->aod_cdevice) {
		HISI_AOD_ERR("%s Failed to create hall aod_cdevice", __func__);
		ret = -ENOMEM;
		goto create_aod_cdevice_err;
	}

	cdev_init(&aod_data->cdev, &hisi_aod_fops);
	aod_data->cdev.owner = THIS_MODULE;

	ret = cdev_add(&aod_data->cdev, aod_data->devno, 1);
	if (ret) {
		HISI_AOD_ERR("cdev_add failed ret = %d.\n", ret);
		ret = -ENOMEM;
		goto add_cdev_err;
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	aod_data->ion_client = hisi_ion_client_create(HISI_AOD_ION_CLIENT_NAME);
	if (IS_ERR_OR_NULL(aod_data->ion_client)) {
		HISI_AOD_ERR("failed to create ion client!\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
#endif
	for(i = 0; i < MAX_DYNAMIC_ALLOC_FB_COUNT; i++) {
		aod_data->ion_dyn_handle[i] = NULL;
		aod_data->start_config_mcu.dynamic_fb_addr[i] = 0;
	}
	aod_data->start_config_mcu.dynamic_fb_count = 0;

	ret = of_reserved_mem_device_init(&pdev->dev);
	if(ret < 0) {
		HISI_AOD_ERR("shared cma memeory failed\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
	aod_data->cma_device = &pdev->dev;


	init_completion(&iom3_status_completion);

	init_timer(&my_timer);
	my_timer.function = (void *)aod_timer_process;
	my_timer.data = 0;

	np = of_find_compatible_node(NULL, NULL, "huawei,aod");
	if (!np) {
		HISI_AOD_ERR("not found device node huawei,aod!\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
	else
	{
		aod_support = 1;
	}

	ret = of_property_read_u32(np, "x_res", &aod_data->x_res);
	if (ret) {
		HISI_AOD_ERR("x_res get lcd_display_type failed!\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
	ret = of_property_read_u32(np, "y_res", &aod_data->y_res);
	if (ret) {
		HISI_AOD_ERR("y_res get lcd_display_type failed!\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
	ret = of_property_read_u32(np, "bpp", &aod_data->bpp);
	if (ret) {
		HISI_AOD_ERR("bpp get lcd_display_type failed!\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}

	g_aod_wq = create_singlethread_workqueue("aod_workqueue");
	if (g_aod_wq == NULL) {
		HISI_AOD_ERR("create_singlethread_workqueue fail\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
	INIT_WORK(&g_aod_worker, aod_off_handle);

	HISI_AOD_INFO("x_res = %d, y_res = %d, bpp = %d!\n", aod_data->x_res, aod_data->y_res, aod_data->bpp);

	/* fb memory size */
	line_length = ALIGN_UP(aod_data->x_res * aod_data->bpp, 16);
	fb_max_len = roundup(line_length * aod_data->y_res, PAGE_SIZE); // lint !e647

	/* bitmap memory size */
	bitmaps_max_len = DIGITS_COUNT*fb_max_len;
	aod_data->max_buf_size = fb_max_len + bitmaps_max_len + sizeof(uint32_t);

	aod_data->blank_mode = FB_BLANK_POWERDOWN;
	aod_data->aod_status = false;
	aod_data->finger_down_status = 0;
	aod_data->start_req_faster = false;
	aod_data->no_need_enter_aod = false;
	aod_data->aod_lock_status = true;
	aod_data->fold_info_config_mcu.panel_id = 0;
	mutex_init(&(aod_data->aod_lock));
	sema_init(&(aod_data->aod_status_sem), 1);

	g_aod_data = aod_data;
	register_iom3_recovery_notifier(&recovery_notify);

	return 0;

alloc_fb_fail:
	cdev_del(&aod_data->cdev);
add_cdev_err:
	device_destroy(aod_data->aod_dev_class,aod_data->devno);
create_aod_cdevice_err:
	unregister_chrdev_region(aod_data->devno, 1);
alloc_chrdev_region_err:
	class_destroy(aod_data->aod_dev_class);
create_aod_dev_class_err:
	kfree(aod_data);
	aod_data = NULL;
alloc_aod_data_fail:
	return ret;
}

static int hisi_aod_remove(struct platform_device *pdev)
{
	g_aod_data = NULL;
	if (g_aod_wq != NULL)
		destroy_workqueue(g_aod_wq);
	return 0;
}

static struct of_device_id aod_device_supply_ids[] = {
	{.compatible = "huawei,aod"},
	{}
};

static struct platform_driver hisi_aod_driver = {
	.probe		= hisi_aod_probe,
	.remove		= hisi_aod_remove,
	.shutdown = NULL,
	.driver = {
		.name = "huawei_aod",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(aod_device_supply_ids),
	},
};

static int __init aod_driver_init(void)
{
	int ret = 0;

	HISI_AOD_ERR("%s\n", __func__);

	ret = platform_driver_register(&hisi_aod_driver);
	if(ret) {
		HISI_AOD_ERR("platform aod driver register failed\n");
		return ret;
	}

	return ret;
}

late_initcall_sync(aod_driver_init);
MODULE_LICENSE("GPL");
