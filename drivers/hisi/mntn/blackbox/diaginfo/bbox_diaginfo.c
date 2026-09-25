/*
 * bbox_diaginfo.c
 *
 * bbox diaginfo main module for bbox diaginfo.
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <bbox_diaginfo.h>
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
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include <securec.h>
#include "../rdr_field.h"
#include "../rdr_inner.h"
#include "../rdr_print.h"
#include "../platform_ap/rdr_hisi_ap_adapter.h"
#include <linux/hisi/hisi_log.h>
#define HISI_LOG_TAG MNTN_BBOX_DIAGINFO

#define BBOX_DIAGINFO_FILE      "/data/hisi_logs/hisi_diaginfo.log"
#define BBOX_DIAGINFO_FILE_BACK "/data/hisi_logs/hisi_diaginfo_1.log"

#define bbox_diag_spin_lock(lock) \
do { if (g_bbox_diaginfo_need_lock) spin_lock_irqsave(&(lock), flags); } while (0)
#define bbox_diag_spin_unlock(lock) \
do { if (g_bbox_diaginfo_need_lock) spin_unlock_irqrestore(&(lock), flags); } while (0)

#define MODULE_MAXLEN 24
#define LOGLEVLE_MAXLEN 24
#define TYPE_MAXLEN 24
#define DIAGINFO_ID_MASK 0xffffffff

struct list_head g_bbox_diaginfo_list = LIST_HEAD_INIT(g_bbox_diaginfo_list);
static DEFINE_SPINLOCK(g_bbox_diaginfo_list_lock);
static DEFINE_SPINLOCK(g_bbox_diaginfo_ringbuffer_lock);
static struct semaphore g_bbox_diaginfo_sem;
static char *g_bbox_diaginfo_buffer_addr = NULL;
static int g_bbox_diaginfo_write_async;
static int g_bbox_diaginfo_init_flag;
static int g_bbox_diaginfo_need_lock = 1;
static int g_bbox_diaginfo_add_ts_init;
unsigned int g_bbox_diaginfo_need_check_count;
unsigned int g_last_diaginfo_id = DIAGINFO_ID_MASK;
u64 g_last_diaginfo_ts;

#undef __DIAGINFO_MODULE_MAP
#undef __DIAGINFO_MAP
#define __DIAGINFO_MAP(x, y, z, k)  { x, #x, y, z, k, 0 },
static struct diaginfo_map g_diaginfo_map[] = {
	#include <bbox_diaginfo_map.h>
};

/* module definition */
#undef __DIAGINFO_MODULE_MAP
#undef __DIAGINFO_MAP
#define __DIAGINFO_MODULE_MAP(x) { x, #x },
struct diaginfo_module {
	enum bbox_diaginfo_module module;
	char name[MODULE_MAXLEN];
};
static struct diaginfo_module g_diaginfo_module[] = {
	#include <bbox_diaginfo_map.h>
};

struct diaginfo_p {
	char *type;
	char *loglevel;
	char *module;
};

/* log level definition */
struct diaginfo_loglevel {
	enum bbox_diaginfo_level level;
	char name[LOGLEVLE_MAXLEN];
};

static struct diaginfo_loglevel g_diaginfo_loglvl[] = {
	{ Critical, "Critical" },
	{ Warning, "Warning" },
	{ INFO, "INFO" },
};

/* diaginfo type definition */
struct diaginfo_type {
	enum bbox_diaginfo_type type;
	char name[TYPE_MAXLEN];
};

static struct diaginfo_type g_diaginfo_type[] = {
	{ HW, "HW" },
	{ SW, "SW" },
};

static int bbox_diaginfo_check_counts(unsigned int err_id);
struct diaginfo_map *get_diaginfo_map(enum bbox_diaginfo_errid errid);

void set_bbox_diaginfo_need_check_count(unsigned int val)
{
	g_bbox_diaginfo_need_check_count = val;
}

void create_hisi_diaginfo_log_file(void)
{
	struct file *fp = NULL;

	fp = filp_open(BBOX_DIAGINFO_FILE, O_CREAT | O_RDWR | O_APPEND, FILE_LIMIT);
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("%s():%d:open %s fail!\n", __func__, __LINE__, BBOX_DIAGINFO_FILE);
		return;
	}
	filp_close(fp, NULL);
}
/*
 * Description: add diaginfo into msg list
 * Input:       err_id: diaginfo error id
 *              pdata: input string
 *              data_len: the size of the string, include the trailing '\0'
 * ts:          time stamp
 * Return:      0:success ; other: fail
 */

int bbox_diaginfo_register(unsigned int err_id, const char *date, const char *pdata, unsigned int data_len, u64 ts)
{
	struct bbox_diaginfo_param_s *p = NULL;
	unsigned long flags;
	int ret;

	if (!pdata) {
		BB_PRINT_ERR("[%s]:%d invalid pointer NULL!\n", __func__, __LINE__);
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	ret = bbox_diaginfo_check_counts(err_id);
	if (ret)
		return ret;

	p = kzalloc(sizeof(*p), GFP_ATOMIC);
	if (!p) {
		BB_PRINT_ERR("kzalloc bbox_diaginfo_param_s failed\n");
		return BBOX_DIAGINFO_NO_MEM;
	}

	if (memcpy_s(&(p->diaginfo.msg), MAX_DIAGINFO_LEN, pdata, data_len) != EOK) {
		BB_PRINT_ERR("[%s]:%d Memcpy fail pdata!\n", __func__, __LINE__);
		kfree(p);
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	p->diaginfo.len = data_len;
	p->diaginfo.errid = err_id;
	p->diaginfo.ts = ts;
	if ((date != NULL) && (strlen(date) >= BBOX_DIAG_DATE_MAXLEN)) {
		ret = memcpy_s(p->diaginfo.date, BBOX_DIAG_DATE_MAXLEN,
			date, BBOX_DIAG_DATE_MAXLEN);
		if (ret != EOK) {
			BB_PRINT_ERR("[%s]:%d Memcpy fail!\n", __func__, __LINE__);
			kfree(p);
			return BBOX_DIAGINFO_INVALIDPAR;
		}
		p->diaginfo.date[BBOX_DIAG_DATE_MAXLEN] = '\0';
	} else {
		ret = snprintf_s(p->diaginfo.date, (BBOX_DIAG_DATE_MAXLEN + 1),
			BBOX_DIAG_DATE_MAXLEN, "%s", rdr_get_timestamp());
		if (ret < 0) {
			BB_PRINT_ERR("[%s]:%d snprintf_s fail!\n", __func__, __LINE__);
			kfree(p);
			return BBOX_DIAGINFO_INVALIDPAR;
		}
	}

	spin_lock_irqsave(&g_bbox_diaginfo_list_lock, flags);
	list_add_tail(&p->diaginfo_list, &g_bbox_diaginfo_list);
	spin_unlock_irqrestore(&g_bbox_diaginfo_list_lock, flags);

	/* send sem to main thread */
	if (g_bbox_diaginfo_init_flag)
		up(&g_bbox_diaginfo_sem);

	return ret;
}

/*
 * Description: the main thread, read diaginfo and save the diaginfo
 *              into bbox buffer.
 * Input:       err_id: diaginfo id, defined in mntn_public_interface.h
 *              date : record time, if input is null, tag the current time
 *              fmt: The format string to use and the arguments
 * Return:      0:success ; other: fail
 */
int bbox_diaginfo_record(unsigned int err_id, const char *date, const char *fmt, ...)
{
	u64 ts;
	int ret;
	va_list args;
	size_t text_len;
	char tmpbuf[MAX_DIAGINFO_LEN] = "";

	if (!fmt)
		return BBOX_DIAGINFO_INVALIDPAR;

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
	if ((err_id == g_last_diaginfo_id) && (ts - g_last_diaginfo_ts) < SEC_TIME_INTERVAL)
		return BBOX_DIAGINFO_REPEATMSG;

	ret = bbox_diaginfo_register(err_id, date, tmpbuf, text_len + 1, ts);
	if (ret)
		return ret;

	g_last_diaginfo_ts = ts;
	g_last_diaginfo_id = err_id;

	return ret;
}
EXPORT_SYMBOL(bbox_diaginfo_record);

static int bbox_diaginfo_check_counts(unsigned int err_id)
{
	struct diaginfo_map *p_diaginfo_map = NULL;

	if (!g_bbox_diaginfo_need_check_count)
		return 0;

	p_diaginfo_map = get_diaginfo_map(err_id);
	if (!p_diaginfo_map) {
		BB_PRINT_ERR("undefine error id: %u\n", err_id);
		return BBOX_DIAGINFO_INV_ID;
	}

	if (p_diaginfo_map->count > DIAGINFO_COUNT_MAX)
		return BBOX_DIAGINFO_OVER_COUNT;

	return 0;
}

static void bbox_diaginfo_check_size(void)
{
	struct kstat diaglog_stat;
	int ret;

	if (vfs_stat(BBOX_DIAGINFO_FILE, &diaglog_stat) == 0) {
		if ((*(unsigned int *)&diaglog_stat.uid != ROOT_UID) ||
			(*(unsigned int *)&diaglog_stat.gid != SYSTEM_GID)) {
			ret = (int)sys_chown((const char __user *)BBOX_DIAGINFO_FILE, ROOT_UID, SYSTEM_GID);
			if (ret)
				BB_PRINT_ERR("chown %s uid [%u] gid [%u] failed err [%d]!\n",
				BBOX_DIAGINFO_FILE, ROOT_UID, SYSTEM_GID, ret);
		}

		if (diaglog_stat.size > rdr_get_diaginfo_size()) {
			if (vfs_stat(BBOX_DIAGINFO_FILE_BACK, &diaglog_stat) == 0)
				sys_unlink(BBOX_DIAGINFO_FILE_BACK);
			sys_rename(BBOX_DIAGINFO_FILE, BBOX_DIAGINFO_FILE_BACK);
			ret = (int)sys_chown((const char __user *)BBOX_DIAGINFO_FILE_BACK, ROOT_UID, SYSTEM_GID);
			if (ret)
				BB_PRINT_ERR("chown %s uid [%u] gid [%u] failed err [%d]!\n",
					BBOX_DIAGINFO_FILE_BACK, ROOT_UID, SYSTEM_GID, ret);
			pr_info("%s(): diaginfo exceed max size 0x%x, delete it\n",
				__func__, rdr_get_diaginfo_size());
		}
	}
}

static char *get_diaginfo_type_name(enum bbox_diaginfo_type type)
{
	unsigned int loop, i;
	char *p = NULL;

	loop = (unsigned int)ARRAY_SIZE(g_diaginfo_type);
	for (i = 0; i < loop; i++) {
		if (type == g_diaginfo_type[i].type) {
			p = g_diaginfo_type[i].name;
			break;
		}
	}

	return p;
}

static char *get_diaginfo_module_name(enum bbox_diaginfo_module module)
{
	unsigned int loop, i;
	char *p = NULL;

	loop = (unsigned int)ARRAY_SIZE(g_diaginfo_module);
	for (i = 0; i < loop; i++) {
		if (module == g_diaginfo_module[i].module) {
			p = g_diaginfo_module[i].name;
			break;
		}
	}

	return p;
}

static char *get_diaginfo_loglevel_name(enum bbox_diaginfo_level level)
{
	unsigned int loop, i;
	char *p = NULL;

	loop = (unsigned int)ARRAY_SIZE(g_diaginfo_loglvl);
	for (i = 0; i < loop; i++) {
		if (level == g_diaginfo_loglvl[i].level) {
			p = g_diaginfo_loglvl[i].name;
			break;
		}
	}

	return p;
}

struct diaginfo_map *get_diaginfo_map(enum bbox_diaginfo_errid errid)
{
	unsigned int loop, i;
	struct diaginfo_map *p = NULL;

	loop = (unsigned int)ARRAY_SIZE(g_diaginfo_map);
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
	char *ptime = NULL;
	int ret;

	if (likely(g_bbox_diaginfo_add_ts_init))
		return;

	g_bbox_diaginfo_add_ts_init = 1;

	if (!g_bbox_diaginfo_buffer_addr) {
		BB_PRINT_ERR("%s(), bbox_diaginfo_buffer does not init ok\n", __func__);
		return;
	}

	ptime = rdr_get_timestamp();
	ret = snprintf_s(pmsg, DIAGINFO_STRING_MAX_LEN, DIAGINFO_STRING_MAX_LEN - 1,
	"%14s: powerup, begin record\n", ptime);
	if (ret < 0) {
		BB_PRINT_ERR("%s():%d, snprintf_s ERROR\n", __func__, __LINE__);
		return;
	}

	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
		g_bbox_diaginfo_buffer_addr, (u8 *)pmsg);
}


#define UNDEFINE_NAME "undef"
#define DSM_AI_NAME "dsm_ai"
/*
 * Description:  save the diaginfo into bbox buffer
 * Input:        e_cur: diaginfo struct
 * Return:       0:success ; other: fail
 */
static int bbox_diaginfo_save_to_buffer(const struct bbox_diaginfo_param_s *pinfo)
{
	int ret;
	char pmsg[DIAGINFO_STRING_MAX_LEN] = {'\0'};
	struct diaginfo_p p_diaginfo;
	struct diaginfo_map *p_diaginfo_map = NULL;
	unsigned long flags = 0;

	p_diaginfo.type = NULL;
	p_diaginfo.loglevel = NULL;
	p_diaginfo.module = NULL;

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
		BB_PRINT_ERR("undefine error id: %u\n", pinfo->diaginfo.errid);
		return -1;
	}
	p_diaginfo_map->count++;

	p_diaginfo.loglevel = get_diaginfo_loglevel_name(p_diaginfo_map->log_level);
	if (!p_diaginfo.loglevel)
		p_diaginfo.loglevel = UNDEFINE_NAME;

	p_diaginfo.type = get_diaginfo_type_name(p_diaginfo_map->fault_type);
	if (!p_diaginfo.type)
		p_diaginfo.type = UNDEFINE_NAME;

	p_diaginfo.module = get_diaginfo_module_name(p_diaginfo_map->module);
	if (!p_diaginfo.module)
		p_diaginfo.module = UNDEFINE_NAME;
	else
		p_diaginfo.module = DSM_AI_NAME;

	/*
	 * Time [], Module [], Error_ID [Name:NO], Log_level [], Fault_Type [], Data []
	 * e.g. Time [20180118143000], Module [DDR], Error_ID [PLL_LOCK_FAIL:925200000],
	 * Log_Level [Critical], Fault_Type [HW], Data [PLL lock fail]
	 */
	ret = snprintf_s(pmsg, DIAGINFO_STRING_MAX_LEN, DIAGINFO_STRING_MAX_LEN - 1,
	"Time [%s], Module [%s], Error_ID [%s:%u], Log_Level [%s], Fault_Type [%s], Data [%s]\n",
	pinfo->diaginfo.date,
	p_diaginfo.module,
	p_diaginfo_map->error_id_name,
	p_diaginfo_map->errid,
	p_diaginfo.loglevel,
	p_diaginfo.type,
	pinfo->diaginfo.msg);

	if (ret < 0) {
		BB_PRINT_ERR("%s():%d snprintf_s ERROR\n", __func__, __LINE__);
		return -1;
	}

	if (!g_bbox_diaginfo_need_lock)
		BB_PRINT_PN("diaginfo:%s\n", pmsg);

	bbox_diag_spin_lock(g_bbox_diaginfo_ringbuffer_lock);
	bbox_diaginfo_add_ts_to_buffer();
	hisiap_ringbuffer_write((struct hisiap_ringbuffer_s *)
		g_bbox_diaginfo_buffer_addr, (u8 *)pmsg);
	bbox_diag_spin_unlock(g_bbox_diaginfo_ringbuffer_lock);

	return 0;
}

static int hisiap_ringbuffer_read_lock(struct hisiap_ringbuffer_s *addr, u8 *ptr, u32 len)
{
	int ret;
	unsigned long flags = 0;

	if (!addr || !ptr) {
		BB_PRINT_ERR("%s():%d invalid add or ptr\n", __func__, __LINE__);
		return -1;
	}

	bbox_diag_spin_lock(g_bbox_diaginfo_ringbuffer_lock);
	ret = hisiap_ringbuffer_read(addr, ptr, len);
	bbox_diag_spin_unlock(g_bbox_diaginfo_ringbuffer_lock);
	return ret;
}

static int bbox_diaginfo_save_one_to_fs(u8 *ptr, size_t count)
{
	int ret;
	size_t len;
	struct file *fp = NULL;

	if (!ptr) {
		BB_PRINT_ERR("%s():%d invalid ptr is NULL\n", __func__, __LINE__);
		return -1;
	}

	bbox_diaginfo_check_size();

	fp = filp_open(BBOX_DIAGINFO_FILE, O_CREAT | O_RDWR | O_APPEND, FILE_LIMIT);
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("open %s fail\n", BBOX_DIAGINFO_FILE);
		return -1;
	}

	vfs_llseek(fp, 0L, SEEK_END);
	len = strlen(ptr);
	if (count > len)
		count = len;

	ret = vfs_write(fp, (char *)ptr, count, &(fp->f_pos));
	if (ret != count)
		BB_PRINT_ERR("write file %s exception with ret %d\n", BBOX_DIAGINFO_FILE, ret);
	else
		ret = 0;

	vfs_fsync(fp, 0);
	filp_close(fp, NULL);

	return ret;
}

/*
 * Description:  read diaginfo from DDR buffer buffer, then write to fs
 * Input:        e_cur: diaginfo struct
 * Return:       0:success ; other: fail
 */
static int bbox_diaginfo_save_to_fs(void)
{
	int ret = 0;
	u8 ptr[DIAGINFO_STRING_MAX_LEN + 1] = {'\0'};

	if (!g_bbox_diaginfo_write_async) {
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
			ptr, DIAGINFO_STRING_MAX_LEN))
			break;

		ret = bbox_diaginfo_save_one_to_fs(ptr, DIAGINFO_STRING_MAX_LEN);
		if (ret < 0)
			break;

	} while (1);

	return ret;
}

static void bbox_process_diaginfo_from_list(void)
{
	struct bbox_diaginfo_param_s *info = NULL;
	struct bbox_diaginfo_param_s *n = NULL;
	struct bbox_diaginfo_param_s *e_cur = NULL;
	unsigned long flags;
	u64 ts;

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
			if (bbox_diaginfo_save_to_buffer(e_cur))
				BB_PRINT_ERR("saving diaginfo into ddr buffer is failed\n");

			/* free e_cur */
			kfree(e_cur);
			e_cur = NULL;

			bbox_diaginfo_save_to_fs();
		}
	} while (!list_empty(&g_bbox_diaginfo_list));
}

/*
 * Function:     bbox_lpmcu_diaginfo_main_thread
 * Description:  the main thread, read diaginfo and save the diaginfo
 *               into bbox buffer.
 * Return:       0:success ; other: fail
 */
static int bbox_diaginfo_main_thread(void *arg)
{
	while (!kthread_should_stop()) {
		if (down_interruptible(&g_bbox_diaginfo_sem)) {
			BB_PRINT_ERR("%s(), down sem fail\n", __func__);
			continue;
		}

		if (list_empty(&g_bbox_diaginfo_list))
			continue;
		/* process diaginfo from list */
		bbox_process_diaginfo_from_list();
	}

	return 0;
}

/*
 * Function:     bbox_diaginfo_dump_lastmsg
 * Description:  In ap reset process, dump diaginfo list msg into bbox buffer.
 *               Meantime, print out the diaginfo
 * Return:       0:success ; other: fail
 */
void bbox_diaginfo_dump_lastmsg(void)
{
	struct bbox_diaginfo_param_s *info = NULL;
	struct bbox_diaginfo_param_s *n = NULL;
	struct bbox_diaginfo_param_s *e_cur = NULL;
	u64 ts;

	g_bbox_diaginfo_need_lock = 0;
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

/*
 * Description:  notify hisi_logs dir is ready
 * Return:       0:success ; other: fail
 */
int bbox_diaginfo_exception_save2fs(void)
{
	g_bbox_diaginfo_write_async = 1;
	return 0;
}

/*
 * Description:  init globle variant for diaginfo, called by bbox main process
 * Return:       0:success ; other: fail
 */
int bbox_diaginfo_init(void)
{
	struct task_struct *diaginfo_thread = NULL;
	struct ap_eh_root *rdr_ap_root = NULL;
	const u64 hisiap_addr = get_hisiap_addr();

	if (g_bbox_diaginfo_init_flag)
		return 0;

	if (!hisiap_addr) {
		BB_PRINT_ERR("%s(), hisiap_addr does not init ok\n", __func__);
		return -1;
	}

	rdr_ap_root = (struct ap_eh_root *)(uintptr_t)hisiap_addr;
	g_bbox_diaginfo_buffer_addr = rdr_ap_root->hook_buffer_addr[HK_DIAGINFO];
	bbox_ap_ipc_init();
	sema_init(&g_bbox_diaginfo_sem, 0);
	diaginfo_thread = kthread_run(bbox_diaginfo_main_thread, NULL, "bbox_diaginfo");
	if (!diaginfo_thread) {
		BB_PRINT_ERR("create thread bbox_diaginfo faild\n");
		return -1;
	}

	bbox_lpmcu_diaginfo_init();
	g_bbox_diaginfo_init_flag = 1;
	g_bbox_diaginfo_need_check_count = 1;
	return 0;
}
