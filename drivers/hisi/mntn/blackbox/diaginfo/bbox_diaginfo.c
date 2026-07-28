/*
 * bbox diaginfo main module for bbox diaginfo.
 *
 * Copyright (c) 2018 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/syscalls.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_ap_hook.h>
#include <bbox_diaginfo_id_def.h>
#include <bbox_diaginfo.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <libhwsecurec/securec.h>
#include "../rdr_field.h"
#include "../rdr_inner.h"
#include "../rdr_print.h"
#include "../platform_ap/rdr_hisi_ap_adapter.h"
#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG MNTN_BBOX_DIAGINFO

#define BBOX_DIAGINFO_FILE      "/data/hisi_logs/hisi_diaginfo.log"
#define BBOX_DIAGINFO_FILE_BACK "/data/hisi_logs/hisi_diaginfo_1.log"

#define	BBOX_DIAG_SPIN_LOCK(lock)   if (bbox_diaginfo_need_lock) {spin_lock_irqsave(&lock, flags);}
#define	BBOX_DIAG_SPIN_UNLOCK(lock) if (bbox_diaginfo_need_lock) {spin_unlock_irqrestore(&lock, flags);}

struct list_head g_bbox_diaginfo_list = LIST_HEAD_INIT(g_bbox_diaginfo_list);
static DEFINE_SPINLOCK(g_bbox_diaginfo_list_lock);
static DEFINE_SPINLOCK(g_bbox_diaginfo_ringbuffer_lock);
static struct semaphore bbox_diaginfo_sem;
static char* g_bbox_diaginfo_buffer_addr = NULL;
static int bbox_diaginfo_write_async = 0;
static int bbox_diaginfo_init_flag = 0;
static int bbox_diaginfo_need_lock = 1;
static int bbox_diaginfo_add_ts_init = 0;
unsigned int bbox_diaginfo_need_check_count = 0;
unsigned int last_diaginfo_id = 0xffffffff;
u64 last_diaginfo_ts = 0;

#undef __DIAGINFO_MODULE_MAP
#undef __DIAGINFO_MAP
#define __DIAGINFO_MAP(x,y,z,k)  {x, #x, y, z, k, 0},
static struct diaginfo_map g_diaginfo_map[] = {
	#include <bbox_diaginfo_map.h>
};

/* module definition */
#undef __DIAGINFO_MODULE_MAP
#undef __DIAGINFO_MAP
#define __DIAGINFO_MODULE_MAP(x) {x, #x},
struct diaginfo_module {
	enum bbox_diaginfo_module  module;
	char		name[24];
};
static struct diaginfo_module g_diaginfo_module[] ={
	#include <bbox_diaginfo_map.h>
};

/* log level definition */
struct diaginfo_loglevel {
	enum bbox_diaginfo_level  level;
	char		name[24];
};

static struct diaginfo_loglevel g_diaginfo_loglvl[] ={
{Critical,"Critical"},
{Warning,"Warning"},
{INFO,"INFO"},
};

/* diaginfo type definition */
struct diaginfo_type {
	enum bbox_diaginfo_type  type;
	char		name[24];
};

static struct diaginfo_type g_diaginfo_type[] ={
{HW,"HW"},
{SW,"SW"},
};

/* #define debug_diag(fmt, ...)     pr_err(fmt,##__VA_ARGS__) */
#define debug_diag(fmt, ...)

static int bbox_diaginfo_check_counts(unsigned int err_id);
struct diaginfo_map* get_diaginfo_map (enum bbox_diaginfo_errid  errid);

/************************************************************
Function:       bbox_diaginfo_register
Description:   add diaginfo into msg list
Input:           	err_id: diaginfo error id
			pdata: input string
			data_len: the size of the string, include the trailing '\0'
			ts: time stamp
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
 /*lint -e429*/
int bbox_diaginfo_register(unsigned int err_id, char* pdata, unsigned int data_len, u64 ts)
{
	struct bbox_diaginfo_param_s *p = NULL;//lint !e429
	unsigned long flags;
	int ret = 0;

	if (!pdata)
		return BBOX_DIAGINFO_INVALIDPAR;
	ret = bbox_diaginfo_check_counts(err_id);
	if (ret) {
		return ret;
	}

	p = kzalloc(sizeof(struct bbox_diaginfo_param_s), GFP_ATOMIC);
	if (p == NULL) {
		BB_PRINT_ERR("kzalloc bbox_diaginfo_param_s failed\n");
		return BBOX_DIAGINFO_NO_MEM;
	}

	memcpy_s(&(p->diaginfo.msg), MAX_DIAGINFO_LEN - 1, pdata, data_len);
	p->diaginfo.len = data_len + 1;
	p->diaginfo.errid = err_id;
	p->diaginfo.ts = ts;

	spin_lock_irqsave(&g_bbox_diaginfo_list_lock, flags);
	list_add_tail(&p->diaginfo_list, &g_bbox_diaginfo_list);
	spin_unlock_irqrestore(&g_bbox_diaginfo_list_lock, flags);

	/* send sem to main thread */
	if (bbox_diaginfo_init_flag)
		up(&bbox_diaginfo_sem);
	debug_diag("bbox_diaginfo_init_flag = %d \n",bbox_diaginfo_init_flag);

	return ret;
}
/*lint +e429*/

/************************************************************
Function: 	bbox_diaginfo_record
Description:  	the main thread, read diaginfo and save the diaginfo
		     	into bbox buffer.
Input:         	err_id: diaginfo id, defined in mntn_public_interface.h
			fmt: The format string to use and the arguments
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
int bbox_diaginfo_record(unsigned int err_id,const char *fmt, ...)
{
	u64 ts;
	int ret = 0;
	va_list args;
	size_t text_len = 0;
	char tmpbuf[MAX_DIAGINFO_LEN] = "";

	if (!fmt) {
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	va_start(args, fmt);
	/*
	  * we need the diaginfo prefix which might be passed-in as a parameter.
	  * text_len is the return size, not including the trailing '\0'.
	  */
	text_len = vscnprintf(tmpbuf, MAX_DIAGINFO_LEN, fmt, args);
	va_end(args);

	if (!text_len) {
		BB_PRINT_ERR(" input fmt size is ZERO\n");
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	/* if repeat the same msg in 1s, then ignore it. */
	ts = hisi_getcurtime();
	if ((err_id == last_diaginfo_id) && (ts - last_diaginfo_ts) < SEC_TIME_INTERVAL)
		return BBOX_DIAGINFO_REPEATMSG;

	ret = bbox_diaginfo_register(err_id, tmpbuf, text_len + 1, ts);
	if (ret)
		return ret;

	last_diaginfo_ts = ts;
	last_diaginfo_id = err_id;

	return ret;
}
EXPORT_SYMBOL(bbox_diaginfo_record);

static int bbox_diaginfo_check_counts(unsigned int err_id)
{
	struct diaginfo_map * p_diaginfo_map;

	if (!bbox_diaginfo_need_check_count)
		return 0;

	p_diaginfo_map = get_diaginfo_map(err_id);
	if (!p_diaginfo_map) {
		BB_PRINT_ERR("undefine error id: %d\n", err_id);
		return BBOX_DIAGINFO_INV_ID;
	}

	if (DIAGINFO_COUNT_MAX < p_diaginfo_map->count) {
		return BBOX_DIAGINFO_OVER_COUNT;
	}

	return 0;
}

#define BBOX_DIAG_CHOWN(path) \
	sys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID)

static void bbox_diaginfo_check_size(void)
{
	struct kstat diaglog_stat;
	int ret;

	if (0 == vfs_stat(BBOX_DIAGINFO_FILE, &diaglog_stat)) {
		if (ROOT_UID != *(unsigned int*)&diaglog_stat.uid ||
				SYSTEM_GID != *(unsigned int*)&diaglog_stat.gid) {
			ret = (int)BBOX_DIAG_CHOWN(BBOX_DIAGINFO_FILE);
			if (ret) {
				BB_PRINT_ERR("chown %s uid [%d] gid [%d] failed err [%d]!\n",
					BBOX_DIAGINFO_FILE, ROOT_UID, SYSTEM_GID, ret);
			}
		}

		if (diaglog_stat.size > rdr_get_diaginfo_size()) {
			if (0 == vfs_stat(BBOX_DIAGINFO_FILE_BACK, &diaglog_stat)) {
				ret = (int)sys_unlink(BBOX_DIAGINFO_FILE_BACK);
				if (ret) {
					BB_PRINT_ERR("sys_unlink %s failed err [%d]!\n", BBOX_DIAGINFO_FILE_BACK, ret);
				}
			}
			sys_rename(BBOX_DIAGINFO_FILE, BBOX_DIAGINFO_FILE_BACK);
			ret = (int)BBOX_DIAG_CHOWN(BBOX_DIAGINFO_FILE_BACK);
			if (ret) {
				BB_PRINT_ERR("chown %s uid [%d] gid [%d] failed err [%d]!\n",
					BBOX_DIAGINFO_FILE_BACK, ROOT_UID, SYSTEM_GID, ret);
			}
			pr_info("%s(): diaginfo exceed max size 0x%x, delete it.\n",
				   __func__, rdr_get_diaginfo_size());
		}
	}
}

char* get_diaginfo_type_name (enum bbox_diaginfo_type  type)
{
	unsigned int loop, i;
	char* p;
	loop =  (unsigned int)(sizeof(g_diaginfo_type) /sizeof(g_diaginfo_type[0]));
	p = NULL;
	for (	i = 0; i < loop; i++) {
		if(type == g_diaginfo_type[i].type){
			p = g_diaginfo_type[i].name;
			break;
		}
	}

	return p;
}

char* get_diaginfo_module_name (enum bbox_diaginfo_module  module)
{
	unsigned int loop, i;
	char* p;

	loop =  (unsigned int)(sizeof(g_diaginfo_module) /sizeof(g_diaginfo_module[0]));
	p = NULL;
	for (	i = 0; i < loop; i++) {
		if (module == g_diaginfo_module[i].module) {
			p = g_diaginfo_module[i].name;
			break;
		}
	}

	return p;
}

char* get_diaginfo_loglevel_name (enum bbox_diaginfo_level  level)
{
	unsigned int loop, i;
	char* p;

	loop =  (unsigned int)(sizeof(g_diaginfo_loglvl) /sizeof(g_diaginfo_loglvl[0]));
	p = NULL;
	for (	i = 0; i < loop; i++) {
		if (level == g_diaginfo_loglvl[i].level) {
			p = g_diaginfo_loglvl[i].name;
			break;
		}
	}

	return p;
}

struct diaginfo_map* get_diaginfo_map (enum bbox_diaginfo_errid  errid)
{
	unsigned int loop, i;
	struct diaginfo_map* p;

	loop =  (unsigned int)(sizeof(g_diaginfo_map) /sizeof(g_diaginfo_map[0]));
	p = NULL;
	for (i = 0; i < loop; i++) {
		if (errid == g_diaginfo_map[i].errid) {
			p = &(g_diaginfo_map[i]);
			break;
		}
	}

	return p;
}

static void bbox_diaginfo_add_ts_to_buffer(void)
{
	char pmsg[DIAGINFO_STRING_MAX_LEN] = {'\0'};
	char *ptime;
	if (likely(bbox_diaginfo_add_ts_init)) {
		return;
	}

	bbox_diaginfo_add_ts_init = 1;

	if (!g_bbox_diaginfo_buffer_addr) {
		BB_PRINT_ERR("%s(), bbox_diaginfo_buffer does not init ok\n", __func__);
		return;
	}

	ptime = rdr_get_timestamp();
	snprintf_s(pmsg, DIAGINFO_STRING_MAX_LEN,DIAGINFO_STRING_MAX_LEN -1 ,
	"%14s: powerup, begin record\n", ptime);

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
		g_bbox_diaginfo_buffer_addr, (u8 *)pmsg);
}

/************************************************************
Function:      bbox_diaginfo_save_to_buffer
Description:  save the diaginfo into bbox buffer
Input:          e_cur: diaginfo struct
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
#define UNDEFINE_NAME 	"undef"
static int bbox_diaginfo_save_to_buffer(struct bbox_diaginfo_param_s *pinfo)
{
	int ret = 0;
	char pmsg[DIAGINFO_STRING_MAX_LEN] = {'\0'};
	char* p_diaginfo_type, *p_diaginfo_loglevel, *p_diaginfo_module;
	struct diaginfo_map * p_diaginfo_map;
	unsigned long rem_nsec;
	u64 ts;
	unsigned long flags = 0;

	if (!g_bbox_diaginfo_buffer_addr) {
		BB_PRINT_ERR("%s(), bbox_diaginfo_buffer does not init ok\n", __func__);
		return -1;
	}

	if (!pinfo) {
		BB_PRINT_ERR("%s(), error input infor\n", __func__);
		return -1;
	}

	p_diaginfo_map = get_diaginfo_map(pinfo->diaginfo.errid);
	if (!p_diaginfo_map) {
		BB_PRINT_ERR("undefine error id: %d\n", pinfo->diaginfo.errid);
		return -1;
	}
	p_diaginfo_map->count ++;

	p_diaginfo_loglevel = get_diaginfo_loglevel_name(p_diaginfo_map->log_level);
	if (!p_diaginfo_loglevel)
		p_diaginfo_loglevel = UNDEFINE_NAME;

	p_diaginfo_type = get_diaginfo_type_name(p_diaginfo_map->fault_type);
	if (!p_diaginfo_type)
		p_diaginfo_type = UNDEFINE_NAME;

	p_diaginfo_module = get_diaginfo_module_name(p_diaginfo_map->module);
	if (!p_diaginfo_module)
		p_diaginfo_module = UNDEFINE_NAME;

	/*
	* Time [], Module [], Error_ID [Name:NO], Log_level [], Fault_Type [], Data []
	* e.g. Time [20180118143000], Module [DDR], Error_ID [PLL_LOCK_FAIL:925200000], 
	* 	   Log_Level [Critical], Fault_Type [HW], Data [PLL lock fail]
	*/
	ts = pinfo->diaginfo.ts;
	rem_nsec = do_div(ts, 1000000000);

	snprintf_s(pmsg, DIAGINFO_STRING_MAX_LEN,DIAGINFO_STRING_MAX_LEN -1 ,
	"Time [%6lu.%06lus], Module [%s],Error_ID [%s:%d], Log_Level [%s], Fault_Type [%s], Data [%s]\n",
	(unsigned long) ts,
	rem_nsec /1000,
	p_diaginfo_module,
	p_diaginfo_map->error_id_name,
	p_diaginfo_map->errid,
	p_diaginfo_loglevel,
	p_diaginfo_type,
	pinfo->diaginfo.msg);

	if (!bbox_diaginfo_need_lock) {
		BB_PRINT_PN("diaginfo:%s \n",pmsg);
	}
	BBOX_DIAG_SPIN_LOCK(g_bbox_diaginfo_ringbuffer_lock);
	bbox_diaginfo_add_ts_to_buffer();
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
		g_bbox_diaginfo_buffer_addr, (u8 *)pmsg);
	BBOX_DIAG_SPIN_UNLOCK(g_bbox_diaginfo_ringbuffer_lock);

	return  ret;
}

static int hisiap_ringbuffer_read_lock
(struct hisiap_ringbuffer_s *addr, u8 *ptr, u32 len) {
	int ret;
	unsigned long flags = 0;

	BBOX_DIAG_SPIN_LOCK(g_bbox_diaginfo_ringbuffer_lock);
	ret = hisiap_ringbuffer_read(addr, ptr, len);
	BBOX_DIAG_SPIN_UNLOCK(g_bbox_diaginfo_ringbuffer_lock);
	return ret;
}

static int bbox_diaginfo_save_one_to_fs(u8 *ptr, size_t count)
{
	int ret = 0;
	size_t len;
	struct file *fp;

	bbox_diaginfo_check_size();

	fp = filp_open(BBOX_DIAGINFO_FILE, O_CREAT | O_RDWR | O_APPEND, 0660);
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("open %s fail\n", BBOX_DIAGINFO_FILE);
		return -1;
	}

	vfs_llseek(fp, 0L, SEEK_END);
	len = strlen(ptr);
	if (count > len)
		count = len;

	ret = vfs_write(fp, (char*)ptr, count, &(fp->f_pos));
	if (ret != count) {
		BB_PRINT_ERR("write file %s exception with ret %d.\n",
		      BBOX_DIAGINFO_FILE, ret);
	} else {
		ret = 0;
	}

	debug_diag("write msg:%s\n", ptr);

	vfs_fsync(fp, 0);
	filp_close(fp, NULL);

	return ret;
}

/************************************************************
Function:      bbox_diaginfo_save_to_fs
Description:  read diaginfo from DDR buffer buffer, then write to fs
Input:          e_cur: diaginfo struct
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
int bbox_diaginfo_save_to_fs(void)
{
	int ret = 0;
	u8 ptr[DIAGINFO_STRING_MAX_LEN + 1] = {'\0'};

	if (!bbox_diaginfo_write_async) {
		pr_info("%s(), last diaginfo has not been saved\n", __func__);
		return -1;
	}

	if (!g_bbox_diaginfo_buffer_addr) {
		BB_PRINT_ERR("%s(), bbox_diaginfo_buffer does not init ok\n", __func__);
		return -1;
	}

	do {
		ptr[0] = '\0';
		if (hisiap_ringbuffer_read_lock((struct hisiap_ringbuffer_s *)g_bbox_diaginfo_buffer_addr,
			ptr, DIAGINFO_STRING_MAX_LEN)) {
			break;
		}

		ret = bbox_diaginfo_save_one_to_fs(ptr, DIAGINFO_STRING_MAX_LEN);
		if (ret < 0)
			break;

	} while (1);

	return  ret;
}

/************************************************************
Function:      bbox_lpmcu_diaginfo_main_thread
Description:  the main thread, read diaginfo and save the diaginfo
		     into bbox buffer.
Input:           N/A
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
static int bbox_diaginfo_main_thread(void *arg)
{
	struct bbox_diaginfo_param_s *info,*n,*e_cur = NULL;
	u64 ts;
	unsigned long flags;

	while (!kthread_should_stop()) {
		if (down_interruptible(&bbox_diaginfo_sem)) {
			BB_PRINT_ERR("%s(), down sem fail\n", __func__);
			continue;
		}

		if (list_empty(&g_bbox_diaginfo_list))
			continue;

		/* process diaginfo from list */
		do {
			ts = ~0UL;
			e_cur = NULL;

			spin_lock_irqsave(&g_bbox_diaginfo_list_lock, flags);
			list_for_each_entry_safe(info, n, &g_bbox_diaginfo_list, diaginfo_list) {
				if (ts >= info->diaginfo.ts) {
					ts = info->diaginfo.ts;
					e_cur = info;
				}
			}
			if (e_cur)
				list_del(&e_cur->diaginfo_list);
			spin_unlock_irqrestore(&g_bbox_diaginfo_list_lock, flags);

			if (e_cur) {
				/* store the msg into rdr buffer */
				if(bbox_diaginfo_save_to_buffer(e_cur)){
					BB_PRINT_ERR("saving diaginfo into ddr buffer is failed\n");
				}

				/*  free e_cur */
				kfree(e_cur);
				e_cur = NULL;

				bbox_diaginfo_save_to_fs();
			}
		} while (!list_empty(&g_bbox_diaginfo_list));
	}

	return 0;
}

/************************************************************
Function:      bbox_diaginfo_dump_lastmsg
Description:  In ap reset process, dump diaginfo list msg into bbox buffer.
		     Meantime, print out the diaginfo
Input:           N/A
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
void bbox_diaginfo_dump_lastmsg(void)
{
	struct bbox_diaginfo_param_s *info,*n,*e_cur = NULL;
	u64 ts;

	bbox_diaginfo_need_lock = 0;
	debug_diag("begin bbox_diaginfo_dump_lastmsg \n");
	/* check loop time and ring buffer spinlock, to make sure it can't be stucked */
	while (!list_empty(&g_bbox_diaginfo_list)) {
		ts = ~0UL;

		list_for_each_entry_safe(info, n, &g_bbox_diaginfo_list, diaginfo_list) {
			if (ts >= info->diaginfo.ts) {
				ts = info->diaginfo.ts;
				e_cur = info;
			}
		}

		if (e_cur) {
			bbox_diaginfo_save_to_buffer(e_cur);
			list_del(&e_cur->diaginfo_list);
			e_cur = NULL;
		}
	}
}

/************************************************************
Function:      bbox_diaginfo_exception_save2fs
Description:  notify hisi_logs dir is ready
Input:           N/A
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
int bbox_diaginfo_exception_save2fs(void)
{
	bbox_diaginfo_write_async = 1;
	return  0;
}

/************************************************************
Function:      bbox_diaginfo_init
Description:  init globle variant for diaginfo, called by bbox main process
Input:           N/A
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
extern u64 g_hisiap_addr;
int bbox_diaginfo_init(void)
{
	struct task_struct *diaginfo_thread = NULL;
	AP_EH_ROOT *rdr_ap_root;
	if(bbox_diaginfo_init_flag)
		return 0;

	if (!g_hisiap_addr) {
		BB_PRINT_ERR("%s(), g_hisiap_addr does not init ok\n", __func__);
		return -1;
	}

	rdr_ap_root = (AP_EH_ROOT *)g_hisiap_addr;
	g_bbox_diaginfo_buffer_addr = rdr_ap_root->hook_buffer_addr[HK_DIAGINFO];
	bbox_ap_ipc_init();

	sema_init(&bbox_diaginfo_sem, 0);

	diaginfo_thread = kthread_run(bbox_diaginfo_main_thread, NULL, "bbox_diaginfo");
	if (!diaginfo_thread) {
		BB_PRINT_ERR("create thread bbox_diaginfo faild.\n");
		return -1;
	}

	bbox_lpmcu_diaginfo_init();
	debug_diag("bbox_diaginfo_init_flag OK \n");
	bbox_diaginfo_init_flag = 1;
	bbox_diaginfo_need_check_count = 1;
	return 0;
}
