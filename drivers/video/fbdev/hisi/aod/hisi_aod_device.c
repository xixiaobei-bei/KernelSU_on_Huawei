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
#include <linux/wakelock.h>
#include <asm/fb.h>
#include "hisi_aod_device.h"
#include "protocol.h"
#include "hisi_fb.h"
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include "contexthub_pm.h"
#include "contexthub_route.h"
#include <asm/cacheflush.h>
#include <asm/cachetype.h>
#include "shmem.h"


#define HISI_AOD_ION_CLIENT_NAME	"hisi_aod_ion"

#define HISI_FB0_NUM	(3)
#define DIGITS_COUNT (10)
#define AOD_SINGLE_CLOCK_DIGITS_RES_X (15)
#define AOD_SINGLE_CLOCK_DIGITS_RES_Y (32)

static int g_iom3_state_aod = 0;
static struct mutex g_lock;
static uint32_t ref_cnt = 0;
struct aod_data_t *g_aod_data = NULL;
extern int g_max_backlight_from_app;
extern int g_min_backlight_from_app;
struct timer_list my_timer;

uint32_t hisi_aod_msg_level = 2;
int aod_support = 0;
int dss_off_status = 0;
static int dss_on_status = 0;
static struct mutex dss_on_off_lock;
static int handle_sh_ipc = 0;

aod_dss_ctrl_status_sh_t strl_cmd;

extern struct ion_client *hisi_ion_client_create(const char *name);
extern int write_customize_cmd(const struct write_info *wr, struct read_info *rd, bool is_lock);
extern int hisi_aod_inc_atomic(struct hisi_fb_data_type *hisifd);
extern void hisi_aod_dec_atomic(struct hisi_fb_data_type *hisifd);

static struct completion iom3_status_completion;
extern int register_iom3_recovery_notifier(struct notifier_block *nb);
extern int send_cmd_from_kernel_nolock(unsigned char cmd_tag, unsigned char cmd_type, unsigned int subtype, char  *buf, size_t count);
static int hisi_aod_sensorhub_cmd_req(obj_cmd_t cmd);
static int hisi_aod_set_display_space_req(aod_display_spaces_mcu_t *display_spaces);
static void hisi_aod_set_config_init(struct aod_data_t *aod_data);
static int hisi_aod_setup_req(aod_set_config_mcu_t *set_config);
static int hisi_aod_set_time_req(aod_time_config_mcu_t *time_config);
static int hisi_aod_start_req(aod_start_config_mcu_t *start_config);
extern int hisi_sensorhub_aod_unblank(void);
extern int hisi_sensorhub_aod_blank(void);
void sh_recovery_handler(void);

int get_aod_support(void)
{
	return aod_support;
}

static int g_dmd_is_set_power_mode_record = 0;
static int g_dmd_is_sensorhub_recovery_record = 0;
int hisi_aod_report_dmd_err(AOD_DMD_TYPE_T type)
{

	if(type >= AOD_DMD_BUTT)
	{
		HISI_AOD_ERR(": aod dmd type undefined!\n");
		return -1;
	}
	if ( NULL == lcd_dclient )
	{
		HISI_AOD_ERR(": there is no lcd_dclient!\n");
		return -1;
	}

	/* try to get permission to use the buffer */
	if (dsm_client_ocuppy(lcd_dclient))
	{
		/* buffer is busy */
		HISI_AOD_ERR(": buffer is busy!\n");
		return -1;
	}

	HISI_AOD_INFO(": entry!\n");

	switch (type)
	{
		case AOD_DMD_SET_POWER_MODE_RECOVERY:
			if(0 == g_dmd_is_set_power_mode_record)
			{
				g_dmd_is_set_power_mode_record ++;
				dsm_client_record(lcd_dclient, "set power mode recovery, finger_down_status is %u.\n",
					g_aod_data->finger_down_status);
				dsm_client_notify(lcd_dclient, DSM_LCD_SET_POWER_MODE_RECOVERY_ERROR_NO);
			}
			break;
		case AOD_DMD_SENSORHUB_RECOVERY:
			if(0 == g_dmd_is_sensorhub_recovery_record)
			{
				g_dmd_is_sensorhub_recovery_record ++;
				dsm_client_record(lcd_dclient, "sensorhub recovery, aod_status is %d .\n",
					g_aod_data->aod_status);
				dsm_client_notify(lcd_dclient, DSM_LCD_SENSORHUB_RECOVERY_ERROR_NO);
			}
			break;
		default:
			HISI_AOD_ERR(": aod dmd type undefined!\n");
			break;
	}

	return 0;
}

extern void hisi_aod_schedule_wq(void);

static int hisi_aod_sendcmd2SensorHub(struct write_info *wr, struct read_info *rd, bool is_lock)
{
	int ret = 0;
	write_info_t *pkg_ap = wr;

	if (g_iom3_state_aod){
		ret = write_customize_cmd(wr, rd, 0);
		if (ret) {
			HISI_AOD_ERR("nolock:tag is %d, cmd is %d\n", pkg_ap->tag, pkg_ap->cmd);
		}
		return ret;
	}

	HISI_AOD_ERR("hisi_aod_sendcmd2SensorHub:%d,%d\n", wr->tag, wr->cmd);
	ret = write_customize_cmd(wr, rd, is_lock);
	if (ret) {
		HISI_AOD_ERR("cmd:tag is %d, cmd is %d\n", pkg_ap->tag, pkg_ap->cmd);
	}

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

	HISI_AOD_INFO("- .\n");

	return 0;
}
static int hisi_aod_start_recovery_req(struct aod_data_t *aod_data)
{
	int ret;
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
			if(g_aod_data->aod_status == true)
			{
				hisi_aod_report_dmd_err(AOD_DMD_SENSORHUB_RECOVERY);
			}
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
	struct aod_data_t *aod_data;
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

	aod_data->buff_virt = (void *)dma_alloc_coherent(aod_data->cma_device, aod_data->smem_size, &aod_data->buff_phy, GFP_KERNEL);
	if (aod_data->buff_virt == NULL) {
		HISI_AOD_ERR("dma_alloc_coherent failed\n");
		goto err_cma_alloc;
	}

	HISI_AOD_INFO("buff_virt is 0x%pK, buff_phy is 0x%llx\n",
		aod_data->buff_virt, aod_data->buff_phy);

	buf_addr = aod_data->buff_phy;
	aod_data->fb_start_addr = buf_addr;

	return buf_addr;
err_cma_alloc:
	return 0;
}
/*lint -e454, -e455, -e456, -e533*/
int dss_sr_of_sh_callback(const pkt_header_t *cmd)
{
	aod_dss_ctrl_status_ap_t *ctrl_cmd;
	int ret = 0;

	HISI_AOD_INFO("dss_sr_of_sh_callback enter!\n");

	if( NULL == cmd )
	{
		HISI_AOD_INFO("cmd is NULL!\n");
		return -1;
	}

	ctrl_cmd = (aod_dss_ctrl_status_ap_t*)cmd;

	if( NULL == ctrl_cmd)
	{
		HISI_AOD_INFO("ctrl_cmd is NULL!\n");
		return -1;
	}

	if( SUB_CMD_AOD_DSS_ON_REQ == ctrl_cmd->sub_cmd )
	{
		//call back dss on. wake_up lock, and start a timer of 1 s.
		wake_lock(&g_aod_data->wlock);
		mutex_lock(&dss_on_off_lock);
		if(dss_on_status)
		{
			HISI_AOD_INFO("dss on already run!\n");
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		if (!handle_sh_ipc) {
			HISI_AOD_INFO("SUB_CMD_AOD_DSS_ON_REQ:handle_sh_ipc is false!\n");
			wake_unlock(&g_aod_data->wlock);
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		my_timer.expires = jiffies + 5*HZ;
		add_timer(&my_timer);

		ret = hisi_sensorhub_aod_unblank();
		if(ret)
		{
			HISI_AOD_INFO("dss on fail!\n");
		}
		else
		{
			HISI_AOD_INFO("dss on success!\n");
			dss_on_status = 1;
			dss_off_status = 0;
		}
		mutex_unlock(&dss_on_off_lock);

	}
	else if ( SUB_CMD_AOD_DSS_OFF_REQ == ctrl_cmd->sub_cmd )
	{
		//call back dss off.   wake_unlock.
		mutex_lock(&dss_on_off_lock);
		if(!dss_on_status)
		{
			HISI_AOD_INFO("dss on not run onetime!\n");
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		del_timer(&my_timer);

		if (!handle_sh_ipc) {
			HISI_AOD_INFO("SUB_CMD_AOD_DSS_OFF_REQ:handle_sh_ipc is false!\n");
			wake_unlock(&g_aod_data->wlock);
			mutex_unlock(&dss_on_off_lock);
			return -1;
		}

		ret = hisi_sensorhub_aod_blank();
		if(ret)
		{
			HISI_AOD_INFO("dss off fail!\n");
		}
		else
		{
			HISI_AOD_INFO("dss off success!\n");
		}
		dss_on_status = 0;
		dss_off_status = 1;
		mutex_unlock(&dss_on_off_lock);
		wake_unlock(&g_aod_data->wlock);

	}

	HISI_AOD_INFO("dss_sr_of_sh_callback exit!\n");
	return 0;

}
/*lint +e454, +e455, +e456, +e533*/
/*lint -e455, -e456*/
void aod_timer_process(void)
{
	//int ret = 0;

	HISI_AOD_INFO("aod_timer_process enter!\n");
	mutex_lock(&dss_on_off_lock);
	if(dss_off_status)
	{
		HISI_AOD_INFO("dss is already off!\n");
		mutex_unlock(&dss_on_off_lock);
		return ;
	}

	if (!handle_sh_ipc) {
		HISI_AOD_INFO("aod_timer_process:handle_sh_ipc is false!\n");
		wake_unlock(&g_aod_data->wlock);
		mutex_unlock(&dss_on_off_lock);
		return ;
	}


	mutex_unlock(&dss_on_off_lock);
	wake_unlock(&g_aod_data->wlock);

	HISI_AOD_INFO("aod_timer_process enter!\n");
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
			wake_unlock(&g_aod_data->wlock);
			mutex_unlock(&dss_on_off_lock);
			return;
		}

		ret = hisi_sensorhub_aod_blank();
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
		wake_unlock(&g_aod_data->wlock);
	}

	mutex_unlock(&dss_on_off_lock);
	HISI_AOD_INFO("sh_recovery_handler exit!\n");

}
/*lint +e455, +e456*/

static void hisi_dump_time_config(aod_time_config_mcu_t *time_config)
{
	HISI_AOD_INFO("curr_time is %llu, time_zone is %d, sec_time_zone is %d, time_format is %d\n",
		time_config->curr_time, time_config->time_zone, time_config->sec_time_zone,	time_config->time_format);
}

static void hisi_dump_display_space(aod_display_spaces_mcu_t *display_space)
{
	int i = 0;
	HISI_AOD_INFO("dual_clocks is %d, display_space_count is %d, pd_logo_final_pos_y = %d\n",
		display_space->dual_clocks, display_space->display_space_count, display_space->pd_logo_final_pos_y);

	for(i = 0; i < display_space->display_space_count; i++) {
		HISI_AOD_INFO("display_spaces[%d].x_start is %d\n", i, display_space->display_spaces[i].x_start);
		HISI_AOD_INFO("display_spaces[%d].y_start is %d\n", i, display_space->display_spaces[i].y_start);
		HISI_AOD_INFO("display_spaces[%d].x_size is %d\n", i, display_space->display_spaces[i].x_size);
		HISI_AOD_INFO("display_spaces[%d].y_size is %d\n", i, display_space->display_spaces[i].y_size);
	}
}

static void hisi_dump_start_config(aod_start_config_mcu_t *start_config)
{
    uint32_t i = 0;
    uint32_t count = 0;
    HISI_AOD_INFO("intelli_switching is %d\n", start_config->intelli_switching);
    HISI_AOD_INFO("aod_type is %d\n", start_config->aod_type);
    HISI_AOD_INFO("fp_mode is %d\n", start_config->fp_mode);
    HISI_AOD_INFO("dynamic_fb_count is %u, dynamic_ext_fb_count is %u, face_id_fb_count is %u, pd_logo_fb_count is %u.\n",
            start_config->dynamic_fb_count,
            start_config->dynamic_ext_fb_count,
            start_config->face_id_fb_count,
            start_config->pd_logo_fb_count);
    count = start_config->dynamic_fb_count + start_config->dynamic_ext_fb_count + start_config->face_id_fb_count+ start_config->pd_logo_fb_count;
    for(i = 0; (i < count)&&(i < MAX_DYNAMIC_ALLOC_FB_COUNT); i++)
    {
        HISI_AOD_INFO("dynamic_fb_addr[%u] is 0x%x\n", i, start_config->dynamic_fb_addr[i]);
    }
    HISI_AOD_INFO("xstart is %d, ystart is %d\n", start_config->aod_pos.x_start, start_config->aod_pos.y_start);
}

static void hisi_dump_set_config_info(aod_set_config_mcu_t *set_config_info)
{
	int i = 0;

	HISI_AOD_INFO("aod_fb is 0x%x, aod_digits_addr is 0x%x\n", set_config_info->aod_fb, set_config_info->aod_digits_addr);
	HISI_AOD_INFO("fp_offset is 0x%x, fp_count is 0x%x\n", set_config_info->fp_offset, set_config_info->fp_count);
	HISI_AOD_INFO("screen_info.xres is %d, screen_info.yres is %d, screen_info.pixel_format is %d\n",
		set_config_info->screen_info.xres, set_config_info->screen_info.yres, set_config_info->screen_info.pixel_format);

	for(i = 0; i < set_config_info->bitmaps_size.bitmap_type_count; i++) {
		HISI_AOD_INFO("bitmap_size[%d].xres is %d\n", i, set_config_info->bitmaps_size.bitmap_size[i].xres);
		HISI_AOD_INFO("bitmap_size[%d].yres is %d\n", i, set_config_info->bitmaps_size.bitmap_size[i].yres);
	}
}

static int hisi_aod_sensorhub_cmd_req(obj_cmd_t cmd)
{
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;

	HISI_AOD_INFO("+.\n");

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	HISI_AOD_INFO("+1.\n");

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = cmd;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;
	HISI_AOD_INFO("+2.\n");

	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, &pkg_mcu, 1);
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
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	aod_pkt_t pkt;
	pkt_header_t *hd = (pkt_header_t *)&pkt;
	int i = 0;

	HISI_AOD_INFO("+.\n");

	if(NULL == display_spaces) {
		HISI_AOD_ERR("display_spaces is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

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
	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_INFO("-.\n");

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
	aod_data->set_config_mcu.screen_info.xres = aod_data->x_res;//144;
	aod_data->set_config_mcu.screen_info.yres = aod_data->y_res;//256;

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
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	aod_pkt_t pkt;
	pkt_header_t *hd = (pkt_header_t *)&pkt;
	int i = 0;

	HISI_AOD_INFO("+.\n");

	if(NULL == set_config) {
		HISI_AOD_ERR("config_info is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

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
	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, &pkg_mcu, 1);
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
	write_info_t pkg_ap;
	aod_pkt_t pkt;
	pkt_header_t *hd = (pkt_header_t *)&pkt;

	HISI_AOD_INFO("+.\n");

	if(NULL == time_config) {
		HISI_AOD_ERR("time_config is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));

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
	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, NULL, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_INFO("-.\n");

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
		HISI_AOD_INFO("hold aod_lock\n");
		mutex_lock(&dss_on_off_lock);
		handle_sh_ipc = 1;
		mutex_unlock(&dss_on_off_lock);
	}

	hisi_dump_start_config(start_config);

	down(&(aod_data->aod_status_sem));
	aod_data->aod_status = true;
	up(&(aod_data->aod_status_sem));
	ret = shmem_send(TAG_AOD, (void *)start_config, sizeof(aod_start_config_mcu_t));
	HISI_AOD_INFO("shmem_send tag is %d, size %ld\n", TAG_AOD,
		 sizeof(aod_start_config_mcu_t));
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
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	aod_pkt_t pkt;
	pkt_header_t *hd = (pkt_header_t *)&pkt;
	aod_display_pos_t *aod_pos;
	struct hisi_fb_data_type * hisifd = hisifd_list[PRIMARY_PANEL_IDX];

	HISI_AOD_INFO("+.\n");

	if(NULL == display_pos) {
		HISI_AOD_ERR("display_pos is NULL Pointer\n");
		return -1;
	}

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_STOP_REQ;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt.subtype);
	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
	}

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
		(void)hisi_sensorhub_aod_blank();
		dss_on_status = 0;
		dss_off_status = 1;
		wake_unlock(&g_aod_data->wlock);
	}
	mutex_unlock(&dss_on_off_lock);

	mutex_unlock(&(g_aod_data->aod_lock));
	HISI_AOD_INFO("release aod_lock\n");

	HISI_AOD_INFO("finger_down_status is %d \n", g_aod_data->finger_down_status);
	if (g_aod_data->finger_down_status == 2 && hisi_aod_inc_atomic(hisifd)){
		HISI_AOD_INFO("in fast unlock mode\n");
		mutex_lock(&(g_aod_data->aod_lock));
		HISI_AOD_INFO("hold aod_lock\n");
		g_aod_data->blank_mode = FB_BLANK_UNBLANK;
		hisi_aod_schedule_wq();
	}
	HISI_AOD_INFO("-.\n");

	return ret;
}
//lint +efunc(456,455,454,*)
/*lint +e454, +e455*/


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

	ret = hisi_aod_stop_req(&aod_data->pos_data);
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
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	aod_pkt_t pkt;
	pkt_header_t *hd = (pkt_header_t *)&pkt;

	HISI_AOD_INFO("+.\n");

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	if(NULL == pos_data) {
		HISI_AOD_ERR("pos_data is NULL Pointer\n");
		return -1;
	}

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_END_UPDATING_REQ;

	pkt.end_updating_data_to_sensorhub.aod_pos.x_start = pos_data->x_start;
	pkt.end_updating_data_to_sensorhub.aod_pos.y_start = pos_data->y_start;
        pkt.end_updating_data_to_sensorhub.aod_type        = aod_type;

	HISI_AOD_INFO("display_pos:x_start is %d, y_start is %d; aod_type is %u\n",
		pkt.end_updating_data_to_sensorhub.aod_pos.x_start,
		pkt.end_updating_data_to_sensorhub.aod_pos.y_start,
		pkt.end_updating_data_to_sensorhub.aod_type);

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, &pkg_mcu, 1);
	if (ret) {
		HISI_AOD_ERR("tag is %d, cmd is %d\n", pkg_ap.tag, pkg_ap.cmd);
		return ret;
	}

	HISI_AOD_INFO("-.\n");

	return ret;
}

static int hisi_aod_start_updating_req(aod_display_pos_t *display_pos)
{
	int ret = 0;
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	aod_pkt_t pkt;
	pkt_header_t *hd = (pkt_header_t *)&pkt;
	aod_display_pos_t *aod_pos;

	HISI_AOD_INFO("+.\n");

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = TAG_AOD;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkt.subtype = SUB_CMD_AOD_START_UPDATING_REQ;

	pkt.display_pos.x_start = display_pos->x_start;
	pkt.display_pos.y_start = display_pos->y_start;

	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt.subtype);
	ret = hisi_aod_sendcmd2SensorHub(&pkg_ap, &pkg_mcu, 1);
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

	HISI_AOD_INFO("-.\n");

	return ret;
}

static int hisi_set_bitmap_size(struct aod_data_t *aod_data, void __user* arg)
{
	aod_bitmaps_size_ap_t bitmaps_size;
	size_t bitmap_count = 0;
	uint32_t i = 0;

	HISI_AOD_INFO("+.\n");

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

	//HISI_AOD_INFO("bitmap_count %u.\n", bitmap_count);

	aod_data->bitmaps_size_mcu.bitmap_type_count = bitmap_count;
	for (i = 0; i < bitmap_count; i ++) {
		aod_data->bitmaps_size_mcu.bitmap_size[i].xres = bitmaps_size.bitmap_size[i].xres;
		aod_data->bitmaps_size_mcu.bitmap_size[i].yres = bitmaps_size.bitmap_size[i].yres;
		HISI_AOD_INFO("xres [%u] [%d].\n", i, aod_data->bitmaps_size_mcu.bitmap_size[i].xres);
		HISI_AOD_INFO("yres [%u] [%d].\n", i, aod_data->bitmaps_size_mcu.bitmap_size[i].yres);
	}

	HISI_AOD_INFO("- .\n");
	return 0;
}

static int hisi_set_time(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret;
	aod_time_config_ap_t time_config;

	HISI_AOD_INFO("+.\n");

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

	HISI_AOD_INFO("curr_time is %lld, time_zone is %d, sec_time_zone is %d, time_format is %d\n",
			time_config.curr_time, time_config.time_zone, time_config.sec_time_zone, time_config.time_format);

	ret = hisi_aod_set_time_req(&aod_data->time_config_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_set_time_req fail\n");
		return ret;
	}

	HISI_AOD_INFO("- .\n");
	return 0;
}

static int hisi_set_display_space(struct aod_data_t *aod_data, const void __user* arg)
{
	aod_display_spaces_ap_t display_spaces;
	aod_display_spaces_ap_temp_t display_spaces_temp;
	size_t display_space_count = 0;
	uint32_t i = 0;
	int ret;

	HISI_AOD_INFO("+.\n");

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}

	if(copy_from_user(&display_spaces_temp, arg, sizeof(display_spaces_temp)))
		return -EFAULT;
	display_spaces.dual_clocks = (unsigned char)display_spaces_temp.dual_clocks;
	display_spaces.display_space_count = (unsigned char)display_spaces_temp.display_space_count - DIFF_NUMBER;
	display_spaces.size = display_spaces_temp.size;
	display_spaces.pd_logo_final_pos_y = display_spaces_temp.display_spaces[display_spaces.display_space_count].y_start;
	memcpy(display_spaces.display_spaces, display_spaces_temp.display_spaces, sizeof(display_spaces.display_spaces[0])*MAX_DISPLAY_SPACE_COUNT);

	if((display_spaces.dual_clocks < 0)
		|| (display_spaces.display_space_count <= 0)
		|| (display_spaces.display_space_count > MAX_DISPLAY_SPACE_COUNT)) {
			HISI_AOD_ERR("invalid display_spaces:dual_clocks=%d ,display_space_count=%d, pd_logo_final_pos_y=%d\n",
			display_spaces.dual_clocks, display_spaces.display_space_count, display_spaces.pd_logo_final_pos_y);
		return -EINVAL;
	}

	display_space_count = display_spaces.display_space_count;

	for (i = 0; i < display_space_count; i++) {
		if (display_spaces.display_spaces[i].y_size <= 0){
			HISI_AOD_ERR("invalid parms:x_size=%d, x_start=%d,y_size=%d,y_start=%d\n",
				display_spaces.display_spaces[i].x_size,
				display_spaces.display_spaces[i].x_start,
				display_spaces.display_spaces[i].y_size,
				display_spaces.display_spaces[i].y_start);
			return -EINVAL;
		}
	}

	for (i = 0; i < display_space_count; i++) {
		aod_data->display_spaces_mcu.display_spaces[i].x_size = display_spaces.display_spaces[i].x_size;
		aod_data->display_spaces_mcu.display_spaces[i].x_start = display_spaces.display_spaces[i].x_start;
		aod_data->display_spaces_mcu.display_spaces[i].y_size = display_spaces.display_spaces[i].y_size;
		aod_data->display_spaces_mcu.display_spaces[i].y_start = display_spaces.display_spaces[i].y_start;
	}

	aod_data->display_spaces_mcu.dual_clocks = display_spaces.dual_clocks;
	aod_data->display_spaces_mcu.display_space_count = display_spaces.display_space_count;
	aod_data->display_spaces_mcu.pd_logo_final_pos_y = display_spaces.pd_logo_final_pos_y;
	ret = hisi_aod_set_display_space_req(&aod_data->display_spaces_mcu);
	if(ret) {
		HISI_AOD_ERR("hisi_aod_set_display_space_req fail\n");
		return ret;
	}

	HISI_AOD_INFO("- .\n");

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

//	HISI_AOD_INFO("pause_display_pos:x_start is %u, y_start is %u, size is %u\n",
//		pause_display_pos.aod_pos.x_start, pause_display_pos.aod_pos.y_start, pause_display_pos.size);

	if(copy_to_user(arg, &pause_display_pos, pause_display_pos.size)) {
		return -EFAULT;
	}

	HISI_AOD_INFO("- .\n");

	return 0;
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
    struct ion_handle * handle = NULL;
    int ret = 0;
    uint32_t i   = 0;
    unsigned long buf_addr;
    uint32_t tmp   = 0;
    uint32_t count = 0;
    aod_dynamic_fb_space_t ion_fb_config;
    if ((NULL == arg)||(NULL == aod_data))
    {
        HISI_AOD_ERR("arg or aod_data is NULL Pointer!\n");
        return -EINVAL;
    }
    if(copy_from_user(&ion_fb_config, arg, sizeof(ion_fb_config)))
    {
        return -EFAULT;
    }
    count = ion_fb_config.dynamic_ext_fb_count + ion_fb_config.dynamic_fb_count + ion_fb_config.face_id_fb_count + ion_fb_config.pd_logo_fb_count;
    if(count > MAX_DYNAMIC_ALLOC_FB_COUNT)
    {
        HISI_AOD_ERR("ion_fb_config.fb_count > %d !\n", MAX_DYNAMIC_ALLOC_FB_COUNT);
        return -EINVAL;
    }
    if(aod_data->ion_dynamic_alloc_flag != false)
    {
        HISI_AOD_ERR("Memory has been allocated!\n");
        return -EINVAL;
    }
    HISI_AOD_ERR("dynamic_ext_fb_count = (%u), dynamic_fb_count = (%u), face_id_fb_count = (%u), pd_logo_fb_count = (%u). \n",
        ion_fb_config.dynamic_ext_fb_count,
        ion_fb_config.dynamic_fb_count,
        ion_fb_config.face_id_fb_count,
        ion_fb_config.pd_logo_fb_count);

    for(i = 0; i < count; i++)
    {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
        handle = ion_import_dma_buf_fd(aod_data->ion_client, ion_fb_config.str_ion_fb[i].ion_buf_fb);
#else
        handle = ion_import_dma_buf(aod_data->ion_client, ion_fb_config.str_ion_fb[i].ion_buf_fb);
#endif
        if(IS_ERR(handle))
        {
            HISI_AOD_ERR("ion_import_dma_buf fail. ion_buf_fb %d, size %u, index %u!\n",
                ion_fb_config.str_ion_fb[i].ion_buf_fb,
                ion_fb_config.str_ion_fb[i].buf_size,
                i);
            continue;
        }

        tmp = ion_fb_config.str_ion_fb[i].buf_size;
        ret = hisifb_ion_phys(aod_data->ion_client, handle, aod_data->dev, &buf_addr,(size_t *)&tmp);
        if(ret < 0)
        {
            HISI_AOD_ERR("ion_phys fail. ion_buf_fb %d, size %u, index %u!\n",
                ion_fb_config.str_ion_fb[i].ion_buf_fb,
                ion_fb_config.str_ion_fb[i].buf_size,
                i);
            continue;
        }
        if(buf_addr > MAX_ADDR_FOR_SENSORHUB)
        {
            HISI_AOD_ERR("phys addr(0x%lx) invalid. ion_buf_fb %d, size %u, index %u!\n",
                buf_addr,
                ion_fb_config.str_ion_fb[i].ion_buf_fb,
                ion_fb_config.str_ion_fb[i].buf_size,
                i);
            continue;
        }

        aod_data->ion_dyn_handle[i] = handle;
        aod_data->start_config_mcu.dynamic_fb_addr[i] = (uint32_t)buf_addr;
        aod_data->ion_dynamic_alloc_flag = true;
        HISI_AOD_ERR("dynamic_fb_addr[%u] = (0x%x) .ion_buf_fb[%u] = 0x%x , buf_size %d.\n",
            i, aod_data->start_config_mcu.dynamic_fb_addr[i],
            i, ion_fb_config.str_ion_fb[i].ion_buf_fb,
            ion_fb_config.str_ion_fb[i].buf_size);
    }
    aod_data->start_config_mcu.dynamic_fb_count = ion_fb_config.dynamic_fb_count;
    aod_data->start_config_mcu.dynamic_ext_fb_count = ion_fb_config.dynamic_ext_fb_count;
    aod_data->start_config_mcu.face_id_fb_count = ion_fb_config.face_id_fb_count;
    aod_data->start_config_mcu.pd_logo_fb_count = ion_fb_config.pd_logo_fb_count;
    return 0;

}

static int hisi_aod_free_dynamic_fb(struct aod_data_t *aod_data, void __user* arg)
{
    uint32_t i     = 0;
    uint32_t count = 0;

    if (NULL == aod_data)
    {
        HISI_AOD_ERR("aod_data NULL Pointer!\n");
        return -EINVAL;
    }
    if(aod_data->ion_dynamic_alloc_flag != true)
    {
        HISI_AOD_ERR("Memory has been released!\n");
        return -EINVAL;
    }
    count = aod_data->start_config_mcu.dynamic_fb_count
        + aod_data->start_config_mcu.dynamic_ext_fb_count
        + aod_data->start_config_mcu.face_id_fb_count
        + aod_data->start_config_mcu.pd_logo_fb_count;

    for(i = 0; (i < count)&&(i < MAX_DYNAMIC_ALLOC_FB_COUNT); i++)
    {
        if(NULL == aod_data->ion_dyn_handle[i])
        {
            continue;
        }
        ion_free(aod_data->ion_client, aod_data->ion_dyn_handle[i]);
        aod_data->ion_dyn_handle[i]         = NULL;
        aod_data->start_config_mcu.dynamic_fb_addr[i] = 0;
    }
    aod_data->start_config_mcu.dynamic_fb_count     = 0;
    aod_data->start_config_mcu.dynamic_ext_fb_count = 0;
    aod_data->start_config_mcu.face_id_fb_count     = 0;
    aod_data->start_config_mcu.pd_logo_fb_count     = 0;
    aod_data->ion_dynamic_alloc_flag                = false;

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

	HISI_AOD_INFO("+.\n");

	//aod_data->aod_status = 1;

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

	HISI_AOD_INFO("- .\n");

	return 0;
}

static int hisi_set_max_and_min_backlight(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret = 0;
	aod_set_max_and_min_backlight_data_t backlight_config;
	struct hisi_fb_data_type * hisifd = hisifd_list[PRIMARY_PANEL_IDX];
	unsigned int temp_max = 0;
	unsigned int temp_min = 0;

	if (NULL == arg) {
		HISI_AOD_ERR("arg NULL Pointer!\n");
		return -EINVAL;
	}

	if (NULL == aod_data) {
		HISI_AOD_ERR("aod_data NULL Pointer!\n");
		return -EINVAL;
	}
	if(copy_from_user(&backlight_config, arg, sizeof(backlight_config)))
	{
		HISI_AOD_ERR("copy_from_user fail!\n");
		return -EFAULT;
	}
	if(NULL == hisifd)
	{
		HISI_AOD_ERR("hisifd NULL Pointer!\n");
		return -EINVAL;
	}
	if((backlight_config.max_backlight < 0)||(backlight_config.min_backlight < 0))
	{
		HISI_AOD_ERR("max_backlight or min_backlight is invlid!\n");
		return -EINVAL;
	}
	temp_max = (unsigned int)backlight_config.max_backlight;
	temp_min = (unsigned int)backlight_config.min_backlight;
	if((temp_max < hisifd->panel_info.bl_min)
		||(temp_max > hisifd->panel_info.bl_max)
		||(temp_min < hisifd->panel_info.bl_min)
		||(temp_min > hisifd->panel_info.bl_max))
	{
		HISI_AOD_ERR("backlight set invalid, maxbacklight(%d) minbacklight(%d)\n",
			backlight_config.max_backlight,
			backlight_config.min_backlight);
		return -EINVAL;
	}

	HISI_AOD_INFO("backlight value changed. old max (%d), new max (%d), old min (%d), new min (%d)\n",
		g_max_backlight_from_app, backlight_config.max_backlight,
		g_min_backlight_from_app, backlight_config.min_backlight);

	g_max_backlight_from_app = backlight_config.max_backlight;
	g_min_backlight_from_app = backlight_config.min_backlight;

	return ret;
}

static int hisi_end_updating(struct aod_data_t *aod_data, const void __user* arg)
{
	int ret = 0;
	aod_end_updating_pos_data_t end_updating_pos;

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

	HISI_AOD_INFO("- .\n");

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

static long hisi_aod_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user* argp = (void __user*)arg;
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

	if(!aod_data->fb_mem_alloc_flag) {
		if (!hisi_aod_fb_alloc(aod_data)) {
			HISI_AOD_ERR("hisi_aod_fb_alloc failed!\n");
			mutex_unlock(&aod_data->mm_lock);
			return -ENOMEM;
		} else {
			aod_data->fb_mem_alloc_flag = true;
		}
	}

	HISI_AOD_INFO("fb_start_addr is 0x%lx(%lu), smem_size is %lu\n", aod_data->fb_start_addr,
		aod_data->fb_start_addr, aod_data->smem_size);

	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	ret = vm_iomap_memory(vma, aod_data->fb_start_addr, aod_data->smem_size);

	mutex_unlock(&aod_data->mm_lock);

	return ret;
}

#define	HISI_AOD_SET_POWER_MODE_TRYLOCK_TIMES		600
#define	HISI_AOD_TRYLOCK_WAITTIME	5//ms
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
			hisi_aod_report_dmd_err(AOD_DMD_SET_POWER_MODE_RECOVERY);
			mutex_lock(&aod_data->ioctl_lock);
			HISI_AOD_INFO("aod force stop\n");
			hisi_send_aod_stop();
			mutex_unlock(&aod_data->ioctl_lock);
			break;
		}
	}
	return trylock_result;
}

/*lint -e454, -e455*/
//lint -efunc(456,455,454,*)
int hisi_aod_set_blank_mode(int blank_mode)
{
	struct aod_data_t *aod_data = g_aod_data;
	struct hisi_fb_data_type * hisifd = hisifd_list[PRIMARY_PANEL_IDX];

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

		if(hisi_aod_inc_atomic(hisifd)) {
			HISI_AOD_INFO(" !!!!wait aod exit. try get aod_lock!!!!\n");
			if (MUTEX_LOCK_FAILURE == hisi_aod_trylock(aod_data)) {
				HISI_AOD_INFO("ap force get aod_lock\n");
				mutex_lock(&(aod_data->aod_lock));
				HISI_AOD_INFO("force hold aod_lock success\n");
			} else {
				HISI_AOD_INFO("try get aod_lock success\n");
			}
			aod_data->blank_mode = blank_mode;
		}
	} else if ((blank_mode == FB_BLANK_POWERDOWN)
		&& (aod_data->blank_mode == FB_BLANK_UNBLANK)) {
		hisi_aod_dec_atomic(hisifd);

		mutex_unlock(&(aod_data->aod_lock));
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
    int i   = 0;
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
	wake_lock_init(&aod_data->wlock, WAKE_LOCK_SUSPEND, "wake_lock");

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
		HISI_AOD_ERR("%s alloc_chrdev_region failed ret = %d.\n", __func__, ret);
		goto alloc_chrdev_region_err;
	}

	aod_data->aod_cdevice = device_create(aod_data->aod_dev_class, NULL, aod_data->devno, NULL, "%s", "aod");
	if (NULL == aod_data->aod_cdevice) {
		HISI_AOD_ERR("%s Failed to create hall aod_cdevice.", __func__);
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

	aod_data->ion_client = hisi_ion_client_create(HISI_AOD_ION_CLIENT_NAME);
	if (IS_ERR_OR_NULL(aod_data->ion_client)) {
		HISI_AOD_ERR("failed to create ion client!\n");
		ret = -ENOMEM;
		goto alloc_fb_fail;
	}
    for(i = 0; i < MAX_DYNAMIC_ALLOC_FB_COUNT; i++)
    {
       aod_data->ion_dyn_handle[i]         = NULL;
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
	//my_timer.expires = jiffies + 1000;
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

	HISI_AOD_INFO("x_res = %d, y_res = %d, bpp = %d!\n", aod_data->x_res, aod_data->y_res, aod_data->bpp);

	/*fb memory size*/
	line_length = ALIGN_UP(aod_data->x_res * aod_data->bpp, 16);
	fb_max_len = roundup(line_length * aod_data->y_res, PAGE_SIZE);		//lint !e647

	/*bitmap memory size*/
	bitmaps_max_len = DIGITS_COUNT*fb_max_len;

	aod_data->max_buf_size = fb_max_len + bitmaps_max_len + sizeof(uint32_t);

	aod_data->blank_mode = FB_BLANK_POWERDOWN;
	aod_data->aod_status = false;
	aod_data->finger_down_status = 0;
	aod_data->start_req_faster = false;
	aod_data->no_need_enter_aod = false;
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
