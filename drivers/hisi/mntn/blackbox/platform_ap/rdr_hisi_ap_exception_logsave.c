/*
 * rdr_hisi_ap_exception_logsave.c
 *
 * Based on the RDR framework, adapt to the AP side to implement resource
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
#include "rdr_hisi_ap_exception_logsave.h"
#include "rdr_hisi_ap_adapter.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/thread_info.h>
#include <linux/hardirq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_fdt.h>
#include <linux/jiffies.h>
#include <linux/uaccess.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/preempt.h>
#include <asm/cacheflush.h>
#include <linux/kmsg_dump.h>
#include <linux/slab.h>
#include <linux/kdebug.h>
#include <linux/hisi/mntn_record_sp.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/stacktrace.h>
#include <linux/kallsyms.h>
#include <linux/blkdev.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_hisi_ap_ringbuffer.h>
#include "../rdr_inner.h"
#include "../../mntn_filesys.h"
#include <linux/hisi/mntn_dump.h>
#include <linux/hisi/eeye_ftrace_pub.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/hisi_powerkey_event.h>
#include <linux/hisi/hisi_sp805_wdt.h>
#include <linux/hisi/hisi_mntn_l3cache_ecc.h>
#include <securec.h>
#include <linux/version.h>
#include <linux/hisi/hisi_pstore.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/watchdog.h>
#include <mntn_subtype_exception.h>
#include "../rdr_print.h"
#include <linux/hisi/hisi_log.h>
#include <linux/hisi/hisi_hw_diag.h>
#include <linux/dma-direction.h>
#define HISI_LOG_TAG HISI_BLACKBOX_TAG


static char exception_buf[KSYM_SYMBOL_LEN] __attribute__((__section__(".data")));
static unsigned long exception_buf_len __attribute__((__section__(".data")));
static u64 g_mi_notify_lpm3_addr;
/*
 * Description:    Sets the abnormal pc that invokes the __show_regs.
 * Input:          buf:Address of the abnormal pc that invokes the __show_regs.
 */
void set_exception_info(unsigned long address)
{
	(void)memset_s(exception_buf, sizeof(exception_buf), 0, sizeof(exception_buf));

	exception_buf_len = sprint_symbol(exception_buf, address);
}

/*
 * Description:    Obtains the abnormal pc that invokes the __show_regs
 * Input:          NA
 * Output:         buf:Address of the abnormal pc that invokes the __show_regs.
 *                 buf_len:Obtains buf long
 * Return:         NA
 */
void get_exception_info(unsigned long *buf, unsigned long *buf_len)
{
	if (unlikely(!buf || !buf_len))
		return;

	*buf = (uintptr_t)exception_buf;
	*buf_len = exception_buf_len;
}

/*
 * Replacement for kernel memcpy function to solve KASAN problem in rdr stack dump flow.
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 * You should not use this function to access IO space, use memcpy_toio()
 * or memcpy_fromio() instead.
 */

/*
 * Description:  Read the ap_last_task memory allocation switch from the dts.
 * Return:       0:The read fails or the switch is turned off, non 0:The switch is turned on.
 */
unsigned int get_ap_last_task_switch_from_dts(void)
{
	int ret;
	struct device_node *np = NULL;
	unsigned int ap_last_task_switch;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,rdr_ap_adapter");
	if (!np) {
		BB_PRINT_ERR("[%s], cannot find rdr_ap_adapter node in dts!\n", __func__);
		return 0;
	}

	ret = of_property_read_u32(np, "ap_last_task_switch", &ap_last_task_switch);
	if (ret) {
		BB_PRINT_ERR("[%s], cannot find ap_last_task_switch in dts!\n", __func__);
		return 0;
	}

	return ap_last_task_switch;
}

static int read_version_property(char *version, int i, char *buf, struct file *fp, size_t count)
{
	char *p = NULL;

	vfs_read(fp, buf, i, &fp->f_pos);
	p = strstr(buf, BUILD_DISPLAY_ID);
	if (p != NULL) {
		p = p + strlen(BUILD_DISPLAY_ID);
		if (*p == '=') {
			p++;
			if (strncpy_s(version, count, p, strlen(p) >= count ? count - 1 : strlen(p)) != EOK) {
				BB_PRINT_ERR("%s():%d:strncpy_s fail!\n", __func__, __LINE__);
				return 0;
			}
			return -1;
		}
	}
	(void)memset_s(buf, BUFFER_SIZE, '\0', BUFFER_SIZE);

	return 0;
}

/* version < 32byte */
void get_product_version(char *version, size_t count)
{
	struct file *fp = NULL;
	char buf[BUFFER_SIZE];
	int i, ret;
	ssize_t length;

	if (IS_ERR_OR_NULL(version)) {
		BB_PRINT_ERR("[%s], invalid para version [0x%pK]!\n", __func__, version);
		return;
	}
	if (memset_s(version, count, 0, count) != EOK) {
		BB_PRINT_ERR("%s():%d:memset_s fail!\n", __func__, __LINE__);
		return;
	}

	/*
	 * Wait until the file system is ok,read /system/build.prop
	 * ro.confg.hw_systemversion=BalongV100R001C50B310 property
	 */
	/* Cannot be invoked in module_init */
	while (rdr_wait_partition("/data/lost+found", RDR_WAIT_PARTITION_TIME) != 0)
		;

	fp = filp_open(SYSTEM_BUILD_POP, O_RDONLY, FILE_LIMIT);
	if (IS_ERR_OR_NULL(fp)) {
		BB_PRINT_ERR("[%s], open [%s] failed! err [%pK]\n",
			__func__, SYSTEM_BUILD_POP, fp);
		return;
	}
	length = vfs_read(fp, buf, BUFFER_SIZE, &fp->f_pos);
	while (length > 0) {
		for (i = 0; i < BUFFER_SIZE; i++)
			if (buf[i] == '\n')
				break; /* Find the complete one line */
		(void)memset_s(buf, BUFFER_SIZE,'\0', BUFFER_SIZE);
		vfs_llseek(fp, -length, SEEK_CUR);
		if (i == BUFFER_SIZE) /* 1 line exceeds 128byte */
			i--;
		if (i != 0) {
			ret = read_version_property(version, i, buf, fp, count);
			if (ret < 0)
				break;
		} else {
			vfs_llseek(fp, 1, SEEK_CUR); /* Delete blank lines */
		}
		length = vfs_read(fp, buf, BUFFER_SIZE, &fp->f_pos);
	}

	filp_close(fp, NULL);
	BB_PRINT_PN("[%s], version [%s]!\n", __func__, version);
}

void rdr_regs_dump(void *dest, const void *src, size_t len)
{
	size_t remain, mult, i;
	u64 *u64_dst = NULL;
	const u64 *u64_src = NULL;

	remain  = len % sizeof(*u64_dst);
	mult    = len / sizeof(*u64_src);
	u64_dst = dest;
	u64_src = src;

	for (i = 0; i < mult; i++)
		*(u64_dst++) = *(u64_src++);

	for (i = 0; i < remain; i++)
		*((u8 *)u64_dst + i) = *((u8 *)u64_src + i);
}

int hisi_trace_hook_install(void)
{
	int ret = 0;
	enum hook_type hk;

	for (hk = HK_IRQ; hk < HK_MAX; hk++) {
		ret = hisi_ap_hook_install(hk);
		if (ret) {
			BB_PRINT_ERR("[%s], hook_type [%u] install failed!\n", __func__, hk);
			return ret;
		}
	}
	return ret;
}

void hisi_trace_hook_uninstall(void)
{
	enum hook_type hk;

	for (hk = HK_IRQ; hk < HK_MAX; hk++)
		hisi_ap_hook_uninstall(hk);
}

/*
 * Description:   get device id. ex: hi3650, hi6250...
 */
void get_device_platform(unsigned char *device_platform, size_t count)
{
	const char *tmp_platform = NULL;

	(void)memset_s(device_platform, count, '\0', count);

	tmp_platform = of_flat_dt_get_machine_name();
	if (tmp_platform) {
		if (strncpy_s((char *)device_platform, count, tmp_platform, strlen(tmp_platform)) != EOK) {
			BB_PRINT_ERR("%s():%d:strncpy_s fail!\n", __func__, __LINE__);
			return;
		}
	} else {
		if (strncpy_s((char *)device_platform, count, "unknown", strlen("unknown")) != EOK) {
			BB_PRINT_ERR("%s():%d:strncpy_s fail!\n", __func__, __LINE__);
			return;
		}
		BB_PRINT_ERR("unrecognizable device_id? new or old product\n");
	}
	device_platform[count - 1] = '\0';
}

/*
 * Description:    After cpoying kerneldump to filesystem, need to free memory.
 *                 So we just  need to read SRC_DUMPEND.
 */
static void read_dump_end(void)
{
	int fd = -1;
	long long cnt;
	char buf[SZ_4K / SZ_4];

	fd = sys_open(SRC_DUMPEND, O_RDONLY, FILE_LIMIT);
	if (fd < 0) {
		BB_PRINT_ERR("[%s]: open %s failed, return [%d]\n", __func__, SRC_DUMPEND, fd);
		return;
	}

	cnt = sys_read(fd, buf, SZ_4K / SZ_4);
	if (cnt < 0) {
		BB_PRINT_ERR("[%s]: read %s failed, return [%lld]\n", __func__, SRC_DUMPEND, cnt);
		goto out;
	}
out:
	sys_close(fd);
}

/*
 * Description:    save the log from file_node
 * Input:          arg:if arg is null, the func is called hisiap_dump
 * Return:         0:success;
 */
int save_mntndump_log(void *arg)
{
	int ret;
	struct kstat mem_stat;

	if (!check_himntn(HIMNTN_GOBAL_RESETLOG))
		return 0;

	while (rdr_wait_partition("/data/lost+found", RDR_WAIT_PARTITION_TIME) != 0)
		;

	if (vfs_stat(SRC_LPMCU_DDR_MEMORY, &mem_stat) == 0)
		save_lpmcu_exc_memory();



	if (vfs_stat(SRC_BL31_MNTN_MEMORY, &mem_stat) == 0)
		save_bl31_exc_memory();

	/* if not success flag ,means old kdump process */
	if (skp_skp_flag() != KDUMP_SKP_SUCCESS_FLAG) {
		if (vfs_stat(SRC_KERNELDUMP, &mem_stat) == 0)
			save_kernel_dump(arg);
	} else {
		save_kernel_dump(arg);
	}


#ifdef CONFIG_HISI_HHEE
	if (vfs_stat(SRC_HHEE_MNTN_MEMORY, &mem_stat) == 0)
		save_hhee_exc_memory();
#endif

	if (vfs_stat(SRC_LOGBUFFER_MEMORY, &mem_stat) == 0)
		save_logbuff_memory();

	if (vfs_stat(SRC_DUMPEND, &mem_stat) == 0)
		read_dump_end();

	ret = save_exception_info(arg);
	if (ret)
		BB_PRINT_ERR("save_exception_info fail, ret=%d\n", ret);

	return ret;
}

/*
 * Description:   After phone reboots, save hisiap_log
 * Input:         log_path:path;modid:excep_id
 */
void save_hisiap_log(char *log_path, u32 modid)
{
	struct rdr_exception_info_s temp;
	int ret, path_root_len;
	bool is_save_done = false;

	if (!log_path) {
		BB_PRINT_ERR("%s():%d:log_path is NULL!\n", __func__, __LINE__);
		return;
	}

	temp.e_notify_core_mask = RDR_AP;
	temp.e_reset_core_mask = RDR_AP;
	temp.e_from_core = RDR_AP;
	temp.e_exce_type = rdr_get_reboot_type();
	temp.e_exce_subtype = rdr_get_exec_subtype_value();

	if (temp.e_exce_type == LPM3_S_EXCEPTION)
		temp.e_from_core = RDR_LPM3;

	if (temp.e_exce_type == MMC_S_EXCEPTION)
		temp.e_from_core = RDR_EMMC;

	if ((temp.e_exce_type >= CP_S_EXCEPTION_START) &&
		(temp.e_exce_type <= CP_S_EXCEPTION_END))
		temp.e_from_core = RDR_CP;

	path_root_len = strlen(PATH_ROOT);

	/* reboot reason ap wdt which is error reported, means other failure result into ap wdt */
	if (unlikely(ap_awdt_analysis(&temp) == 0)) {
		BB_PRINT_ERR("[%s], ap_awdt_analysis correct reboot reason [%s]!\n",
			__func__, rdr_get_exception_type(temp.e_exce_type));
		incorrect_reboot_reason_analysis(log_path, &temp);
	}

	/* if last save not done, need to add "last_save_not_done" in history.log */
	if (modid == BBOX_MODID_LAST_SAVE_NOT_DONE)
		is_save_done = false;
	else
		is_save_done = true;

	rdr_save_history_log(&temp, &log_path[path_root_len], DATATIME_MAXLEN,
		is_save_done, get_last_boot_keypoint());

	ret = save_mntndump_log(NULL);
	if (ret)
		BB_PRINT_ERR("save_mntndump_log fail, ret=%d", ret);
}

int hisiap_nmi_notify_init(void)
{
	if (g_mi_notify_lpm3_addr)
		return 0;

	g_mi_notify_lpm3_addr = (uintptr_t)ioremap(NMI_NOTIFY_LPM3_ADDR, REG_MAP_SIZE);
	if (!g_mi_notify_lpm3_addr) {
		BB_PRINT_ERR("[%s]", __func__);
		return -1;
	}
	return 0;
}

/*
 * Description : Initialize the reset reason
 */
void hisiap_nmi_notify_lpm3(void)
{
	unsigned int value;
	uintptr_t addr;

	addr = g_mi_notify_lpm3_addr;
	value = readl((char *)addr);
	value |= (0x1 << LPM3_ADDR_SHIFT);
	writel(value, (char *)addr);
	value &= ~(0x1 << LPM3_ADDR_SHIFT);
	writel(value, (char *)addr);
}

/*
 * Description : print hisicurtime and hisicurslice and assign values before reset.
 */
void get_bbox_curtime_slice(void)
{
	u64 curtime, curslice;

	curtime = hisi_getcurtime();
	curslice = get_32k_abs_timer_value();

	BB_PRINT_PN("printk_time is %llu, 32k_abs_timer_value is %llu\n", curtime, curslice);
}

/*
 * Description : Reset function of the AP when an exception occurs
 */
void rdr_hisiap_reset(u32 modid, u32 etype, u64 coreid)
{
	u64 err1_status, err1_misc0;

	BB_PRINT_PN("%s start\n", __func__);
	get_bbox_curtime_slice();
	if (!in_atomic() && !irqs_disabled() && !in_irq())
		sys_sync();

	blk_power_off_flush(0); /* Flush the storage device cache */

	if (etype != AP_S_PANIC) {
		BB_PRINT_PN("etype is not panic\n");
		dump_stack();
		preempt_disable();
		smp_send_stop();
	}

	bbox_diaginfo_dump_lastmsg();
	/* HIMNTN_PANIC_INTO_LOOP will disbale ap reset */
	if (check_himntn(HIMNTN_PANIC_INTO_LOOP) == 1) {
		do {
		} while (1);
	}
	BB_PRINT_PN("%s blk flush ok\n", __func__);
	flush_ftrace_buffer_cache();
	mntn_show_stack_cpustall();
	kmsg_dump(KMSG_DUMP_PANIC);
	flush_cache_all();

	(void)l3cache_ecc_get_status(&err1_status, &err1_misc0);
	hisiap_nmi_notify_lpm3();
	BB_PRINT_PN("%s end\n", __func__);

	while (1)
		;
}

/*
 * Description : Used by the rdr to record exceptions to the pmu
 */
void record_exce_type(const struct rdr_exception_info_s *e_info)
{
	if (!e_info) {
		BB_PRINT_ERR("einfo is null\n");
		return;
	}
	set_reboot_reason(e_info->e_exce_type);
	set_subtype_exception(e_info->e_exce_subtype, false);
}

/*
 * Description : Parameters for registration exceptions.
 * This parameter is invoked between dump and reset
 */
void hisiap_callback(u32 argc, void *argv)
{
	int ret;

	if (check_himntn(HIMNTN_GOBAL_RESETLOG)) {
		ret = hisi_trace_hook_install();

		if (ret)
			BB_PRINT_ERR("[%s]\n", __func__);
	}
}

/*
 * Description : For large files, only the file name is created.
 * The fpga file is time-consuming, for example, kerneldump
 */
int rdr_copy_big_file_apend(const char *dst, const char *src)
{
	long fddst = -1;
	long fdsrc = -1;
	int ret = 0;

	if (!dst || !src) {
		BB_PRINT_ERR("rdr:%s():dst or src is NULL\n", __func__);
		return -1;
	}

	fdsrc = sys_open(src, O_RDONLY, FILE_LIMIT);
	if (fdsrc < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed, return [%ld]\n", __func__, src, fdsrc);
		ret = -1;
		goto out;
	}
	fddst = sys_open(dst, O_CREAT | O_WRONLY | O_APPEND, FILE_LIMIT);
	if (fddst < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed, return [%ld]\n", __func__, dst, fddst);
		sys_close(fdsrc);
		ret = -1;
		goto out;
	}

	sys_close(fdsrc);
	sys_close(fddst);
out:
	return ret;
}

/*
 * Description : Copy the content of a file to another file
 */
int rdr_copy_file_apend(const char *dst, const char *src)
{
	long fddst = -1;
	long fdsrc = -1;
	char buf[SZ_4K / SZ_4];
	long cnt;
	int ret = 0;

	if (!dst || !src) {
		BB_PRINT_ERR("rdr:%s():dst or src is NULL\n", __func__);
		return -1;
	}

	fdsrc = sys_open(src, O_RDONLY, FILE_LIMIT);
	if (fdsrc < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed, return [%ld]\n", __func__, src, fdsrc);
		ret = -1;
		goto out;
	}
	fddst = sys_open(dst, O_CREAT | O_WRONLY | O_APPEND, FILE_LIMIT);
	if (fddst < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed, return [%ld]\n", __func__, dst, fddst);
		sys_close(fdsrc);
		ret = -1;
		goto out;
	}

	while (1) {
		cnt = sys_read(fdsrc, buf, SZ_4K / SZ_4);
		if (cnt == 0)
			break;
		if (cnt < 0) {
			BB_PRINT_ERR("rdr:%s():read %s failed, return [%ld]\n", __func__, src, cnt);
			ret = -1;
			goto close;
		}

		cnt = sys_write(fddst, buf, SZ_4K / SZ_4);
		if (cnt <= 0) {
			BB_PRINT_ERR("rdr:%s():write %s failed, return [%ld]\n", __func__, dst, cnt);
			ret = -1;
			goto close;
		}
	}

close:
	sys_close(fdsrc);
	sys_close(fddst);
out:
	return ret;
}

/*
 * Description : Copy the content of a file to another file
 */
int skp_rdr_copy_file_apend(const char *dst, const char *src)
{
	struct sky_file_info info;
	char buf[SZ_4K / SZ_4];
	long cnt = -1;
	int ret = 0;
	struct memdump *tmp = NULL;
	long kdump_size;

	if (!dst || !src) {
		BB_PRINT_ERR("rdr:%s():dst or src is NULL\n", __func__);
		return -1;
	}
	BB_PRINT_PN("rdr:%s():dst=%s or src=%s\n", __func__, dst, src);

	info.fdsrc = -1;
	info.fddst = -1;
	info.seek_return = -1;

	info.fdsrc = sys_open(src, O_RDONLY, FILE_LIMIT);
	if (info.fdsrc < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed, return [%ld]\n", __func__, src, info.fdsrc);
		ret = -1;
		goto out;
	}

	info.fddst = sys_open(dst, O_CREAT | O_WRONLY | O_APPEND, FILE_LIMIT);
	if (info.fddst < 0) {
		BB_PRINT_ERR("rdr:%s():open %s failed, return [%ld]\n", __func__, dst, info.fddst);
		sys_close((unsigned int)info.fdsrc);
		ret = -1;
		goto out;
	}

	info.seek_value = (long)skp_skp_resizeaddr();
	BB_PRINT_PN("%s():%d:seek value[%lx]\n", __func__, __LINE__, info.seek_value);

	/* Offset address read, seek to resize addr */
	info.seek_return = sys_lseek((unsigned int)info.fdsrc, info.seek_value, SEEK_SET);
	if (info.seek_return < 0) {
		BB_PRINT_ERR("%s():%d:lseek fail[%ld]\n", __func__, __LINE__, info.seek_return);
		goto close;
	}

	/* read out the head of kdump,find the size to save */
	cnt = sys_read((unsigned int)info.fdsrc, buf, SZ_4K / SZ_4);
	if (cnt <= 0) {
		BB_PRINT_ERR("rdr:%s():read %s failed, return [%ld]\n", __func__, src, cnt);
		ret = -1;
		goto close;
	}

	tmp = (struct memdump *)buf;

	/* add SZ_4K, for size less than 4K condition */
	kdump_size = (long)(tmp->size + SZ_4K / SZ_4 + KDUMP_SKP_DATASAVE_OFFSET);
	if (kdump_size > KDUMP_MAX_SIZE) {
		BB_PRINT_ERR("%s():%d:kdump_size=%ld error\n", __func__, __LINE__, kdump_size);
		goto close;
	}

	BB_PRINT_PN("%s():%d:seek value[%lx]\n", __func__, __LINE__, tmp->size);

	/* Offset address read, seek to resize addr again */
	info.seek_return = (long)sys_lseek((unsigned int)info.fdsrc, info.seek_value, SEEK_SET);
	if (info.seek_return < 0) {
		BB_PRINT_ERR("%s():%d:lseek fail[%ld]\n", __func__, __LINE__, info.seek_return);
		goto close;
	}

	/* start to dump flash to data partiton */
	while (kdump_size > 0 && kdump_size <= KDUMP_MAX_SIZE) {
		cnt = sys_read((unsigned int)info.fdsrc, buf, SZ_4K / SZ_4);
		if (cnt == 0)
			break;
		if (cnt < 0) {
			BB_PRINT_ERR("rdr:%s():read %s failed, return [%ld]\n", __func__, src, cnt);
			ret = -1;
			goto close;
		}

		cnt = sys_write((unsigned int)info.fddst, buf, SZ_4K / SZ_4);
		if (cnt <= 0) {
			BB_PRINT_ERR("rdr:%s():write %s failed, return [%ld]\n", __func__, dst, cnt);
			ret = -1;
			goto close;
		}

		kdump_size = kdump_size - (SZ_4K / SZ_4);
	}

close:
	sys_close((unsigned int)info.fdsrc);
	sys_close((unsigned int)info.fddst);
out:
	return ret;
}

/*
 * Description:    copy file from /sys/fs/pstore to dst_dir_str
 */
void save_pstore_info(const char *dst_dir_str)
{
	int i, ret, tmp_cnt;
	char *pbuff = NULL;
	char dst_str[NEXT_LOG_PATH_LEN];
	char fullpath_arr[LOG_PATH_LEN];

	hisi_create_pstore_entry();
	if (rdr_wait_partition(PSTORE_PATH, WAIT_PSTORE_PATH)) {
		BB_PRINT_ERR("pstore is not ready\n");
		return;
	}

	tmp_cnt = MNTN_FILESYS_MAX_CYCLE * MNTN_FILESYS_PURE_DIR_NAME_LEN;

	/* Apply for a memory for storing all files name in the /sys/fs/pstore/ directory */
	pbuff = kmalloc(tmp_cnt, GFP_KERNEL);
	if (!pbuff) {
		BB_PRINT_ERR("kmalloc tmp_cnt fail, tmp_cnt = [%d]\n", tmp_cnt);
		return;
	}

	/* Clear the buff that you have applied for */
	if (memset_s((void *)pbuff, tmp_cnt, 0, tmp_cnt) != EOK) {
		BB_PRINT_ERR("%s():%d:memset_s fail!\n", __func__, __LINE__);
		kfree(pbuff);
		return;
	}

	/*
	 * Invoke the interface to save the names of all files in
	 * the /sys/fs/pstore/ directory to the pbuff
	 */
	tmp_cnt = mntn_filesys_dir_list(PSTORE_PATH, pbuff, tmp_cnt, DT_REG);

	/* Generate the last_kmsg and dmesg-ramoops-x files */
	for (i = 0; i < tmp_cnt; i++) {
		/* generated absolute paths of source file */
		(void)memset_s((void *)fullpath_arr, sizeof(fullpath_arr), 0, sizeof(fullpath_arr));
		ret = strncat_s(fullpath_arr, LOG_PATH_LEN, PSTORE_PATH, ((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
			kfree(pbuff);
			return;
		}
		BB_PRINT_PN("file is [%s]\n", (pbuff + ((unsigned long)i * MNTN_FILESYS_PURE_DIR_NAME_LEN)));
		ret = strncat_s(fullpath_arr, LOG_PATH_LEN,
				(const char *)(pbuff + ((unsigned long)i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
				((LOG_PATH_LEN - 1) - strlen(fullpath_arr)));
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
			kfree(pbuff);
			return;
		}
		/* If not the console, the destination file is not last_kmsg, which is dmesg-ramoops-x */
		if (strncmp((const char *)(pbuff + ((unsigned long)i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
				"console-ramoops",
				strlen("console-ramoops")) != 0) {
			(void)memset_s((void *)dst_str, NEXT_LOG_PATH_LEN, 0, NEXT_LOG_PATH_LEN);
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, dst_dir_str, ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				kfree(pbuff);
				return;
			}
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, "/", ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				kfree(pbuff);
				return;
			}
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN,
					(const char *)(pbuff + ((unsigned long)i * MNTN_FILESYS_PURE_DIR_NAME_LEN)),
					((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				kfree(pbuff);
				return;
			}
		} else {
			(void)memset_s((void *)dst_str, NEXT_LOG_PATH_LEN, 0, NEXT_LOG_PATH_LEN);
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, dst_dir_str, ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				kfree(pbuff);
				return;
			}
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, "/", ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				kfree(pbuff);
				return;
			}
			ret = strncat_s(dst_str, NEXT_LOG_PATH_LEN, "last_kmsg", ((NEXT_LOG_PATH_LEN - 1) - strlen(dst_str)));
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:strncat_s fail!\n", __func__, __LINE__);
				kfree(pbuff);
				return;
			}
		}

		/* Copy the content of the source file to the destination file */
		ret = rdr_copy_file_apend(dst_str, fullpath_arr);
		if (ret)
			BB_PRINT_ERR("rdr_copy_file_apend [%s] fail, ret = [%d]\n", fullpath_arr, ret);
	}

	hisi_remove_pstore_entry();
	kfree(pbuff);
}

/*
 * Description:    copy fastboot_log to dst_dir_str
 */
void save_fastboot_log(const char *dst_dir_str)
{
	int ret;
	u32 len;
	char fastbootlog_path[NEXT_LOG_PATH_LEN];
	char last_fastbootlog_path[NEXT_LOG_PATH_LEN];

	/* Absolute path of the fastbootlog file */
	(void)memset_s(last_fastbootlog_path, NEXT_LOG_PATH_LEN, 0, NEXT_LOG_PATH_LEN);

	(void)memset_s(fastbootlog_path, NEXT_LOG_PATH_LEN, 0, NEXT_LOG_PATH_LEN);
	len = strlen(dst_dir_str);
	ret = memcpy_s(last_fastbootlog_path, NEXT_LOG_PATH_LEN - 1, dst_dir_str, len);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	ret = memcpy_s(&last_fastbootlog_path[len], NEXT_LOG_PATH_LEN - strlen(last_fastbootlog_path),
			"/last_fastboot_log", strlen("/last_fastboot_log") + 1);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	ret = memcpy_s(fastbootlog_path, NEXT_LOG_PATH_LEN - 1, dst_dir_str, len);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}
	ret = memcpy_s(&fastbootlog_path[len], NEXT_LOG_PATH_LEN - strlen(fastbootlog_path), "/fastboot_log",
			strlen("/fastboot_log") + 1);
	if (ret != EOK) {
		BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
		return;
	}

	/* Generate the last_fastbootlog file */
	ret = rdr_copy_file_apend(last_fastbootlog_path, LAST_FASTBOOT_LOG_FILE);
	if (ret)
		BB_PRINT_ERR("rdr_copy_file_apend [%s] fail, ret = [%d]\n", LAST_FASTBOOT_LOG_FILE, ret);

	/* Generate the curr_fastbootlog file */
	ret = rdr_copy_file_apend(fastbootlog_path, FASTBOOT_LOG_FILE);
	if (ret)
		BB_PRINT_ERR("rdr_copy_file_apend [%s] fail, ret = [%d]\n", FASTBOOT_LOG_FILE, ret);
}

int create_exception_dir(const char *exce_dir, char *dst_dir_str)
{
	int fd = -1;
	int ret;
	u32 len;
	char date[DATATIME_MAXLEN];
	char default_dir[LOG_PATH_LEN];

	/*
	 * Open the abnormal directory. If the directory does not exist,
	 * create it as the current time
	 */
	fd = sys_open(exce_dir, O_DIRECTORY, 0);
	if (fd < 0) {
		BB_PRINT_ERR("sys_open exce_dir[%s] fail,fd = [%d]\n", exce_dir, fd);
		(void)memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);
		(void)memset_s(default_dir, LOG_PATH_LEN, 0, LOG_PATH_LEN);
		ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld", rdr_get_timestamp(), rdr_get_tick());
		if (ret < 0) {
			BB_PRINT_ERR("%s():%d:snprintf_s fail!\n", __func__, __LINE__);
			return -1;
		}
		ret = snprintf_s(default_dir, LOG_PATH_LEN, LOG_PATH_LEN - 1, "%s%s", PATH_ROOT, date);
		if (ret < 0) {
			BB_PRINT_ERR("%s():%d:snprintf_s fail!\n", __func__, __LINE__);
			return -1;
		}
		BB_PRINT_PN("default_dir is [%s]\n", default_dir);
		fd = sys_mkdir(default_dir, DIR_LIMIT);
		if (fd < 0) {
			BB_PRINT_ERR("sys_mkdir default_dir[%s] fail, fd = [%d]\n", default_dir, fd);
			return -1;
		}
		(void)memset_s(dst_dir_str, DEST_LOG_PATH_LEN, 0, DEST_LOG_PATH_LEN);
		len = strlen(default_dir);
		ret = memcpy_s(dst_dir_str, DEST_LOG_PATH_LEN - 1, default_dir, len);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return -1;
		}
		ret = memcpy_s(&dst_dir_str[len], DEST_LOG_PATH_LEN - strlen(dst_dir_str), "/ap_log",
				strlen("/ap_log") + 1);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return -1;
		}
	} else {
		sys_close(fd);
		(void)memset_s(dst_dir_str, DEST_LOG_PATH_LEN, 0, DEST_LOG_PATH_LEN);
		len = strlen(exce_dir);
		ret = memcpy_s(dst_dir_str, DEST_LOG_PATH_LEN - 1, exce_dir, len);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return -1;
		}
		ret = memcpy_s(&dst_dir_str[len], DEST_LOG_PATH_LEN - strlen(dst_dir_str), "/ap_log",
				strlen("/ap_log") + 1);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			return -1;
		}
	}

	/*
	 * Open the ap_log directory in the abnormal directory.
	 * If the directory does not exist, create it
	 */
	fd = sys_open(dst_dir_str, O_DIRECTORY, 0);
	if (fd < 0) {
		fd = sys_mkdir(dst_dir_str, DIR_LIMIT);
		if (fd < 0) {
			BB_PRINT_ERR("sys_mkdir dst_dir_str[%s] fail, fd = [%d]\n", dst_dir_str, fd);
			return -1;
		}
	} else {
		sys_close(fd);
	}
	return 0;
}

/*
 * Description : Write exceptions that are not registered to rdr to history.log
 */
int record_reason_task(void *arg)
{
	int ret;
	char date[DATATIME_MAXLEN];
	struct rdr_exception_info_s temp = {
		{ 0, 0 }, MODID_AP_S_PANIC, MODID_AP_S_PANIC, RDR_ERR, RDR_REBOOT_NOW,
		(u64)RDR_AP, (u64)RDR_AP, (u64)RDR_AP, (u32)RDR_REENTRANT_DISALLOW,
		(u32)COLDBOOT, 0, (u32)RDR_UPLOAD_YES, "ap", "ap", 0, 0, (void *)0, 0
	};
	temp.e_from_core = RDR_AP;
	temp.e_exce_type = rdr_get_reboot_type();

	while (rdr_wait_partition("/data/lost+found", RDR_WAIT_PARTITION_TIME) != 0)
		;
	(void)memset_s(date, DATATIME_MAXLEN, 0, DATATIME_MAXLEN);

	ret = snprintf_s(date, DATATIME_MAXLEN, DATATIME_MAXLEN - 1, "%s-%08lld", rdr_get_timestamp(), rdr_get_tick());
	if (ret < 0) {
		BB_PRINT_ERR("%s():%d:snprintf_s fail!\n", __func__, __LINE__);
		return -1;
	}
	rdr_save_history_log(&temp, &date[0], DATATIME_MAXLEN, true, get_last_boot_keypoint());
	return 0;
}

/*
 * Description : Hook function, which is used for the dead loop
 *               before the system panic processing, so that
 *               the field is reserved.
 */
int acpu_panic_loop_notify(struct notifier_block *nb,
				unsigned long event, void *buf)
{
	if (check_himntn(HIMNTN_PANIC_INTO_LOOP)) {
		do {} while (1);
	}

	return 0;
}

static bool is_modem_exception(void)
{
	int i;
	struct stack_trace trace;
	unsigned long entries[MAX_STACK_TRACE_DEPTH];
	struct module *modem_module = NULL;
	unsigned long module_start, module_end;

	BB_PRINT_ERR("%s start\n", __func__);
	modem_module = find_hisi_module("balong_modem", strlen("balong_modem"));
	if (!modem_module) {
		BB_PRINT_ERR("modem ko module no found\n");
		return false;
	}

	trace.nr_entries = 0;
	trace.max_entries = MAX_STACK_TRACE_DEPTH;
	trace.entries = entries;
	trace.skip = 0;

	module_start = (uintptr_t)modem_module->core_layout.base;
	BB_PRINT_ERR("[%s]:module_start %lx\n", __func__, module_start);
	module_end = (uintptr_t)module_start + modem_module->core_layout.size;
	BB_PRINT_ERR("[%s]:module end is %lx\n", __func__, module_end);
	save_stack_trace_tsk(current, &trace);
	BB_PRINT_ERR("[%s]:save_stack_trace_tsk done\n", __func__);

	for (i = 0; i < trace.nr_entries; i++) {
		BB_PRINT_ERR("[%s]:trace.entries[%d] = %lx\n", __func__, i, trace.entries[i]);
		if (trace.entries[i] >= module_start && trace.entries[i] <= module_end) {
			BB_PRINT_ERR("[%s]:modem drv panic happen\n", __func__);
			return true;
		}
	}

	return false;
}

unsigned int is_reboot_reason_from_modem(void)
{
	unsigned int reboot_reason;
	unsigned int subtype;

	reboot_reason = rdr_get_reboot_type();
	subtype = rdr_get_exec_subtype_value();
	if ((reboot_reason == AP_S_PANIC) && (subtype == HI_APPANIC_MODEM)) {
		BB_PRINT_ERR("[%s]:modem drv panic happen\n", __func__);
		return true;
	}

	return false;
}
EXPORT_SYMBOL(is_reboot_reason_from_modem);

/*
 * Description : panic reset hook function
 */
int rdr_hisiap_panic_notify(struct notifier_block *nb,
				unsigned long event, void *buf)
{
#ifdef CONFIG_HISI_HW_DIAG
	union hisi_hw_diag_info hwdiag_apanic_info;
#endif

	BB_PRINT_PN("[%s], ===> enter panic notify!\n", __func__);

#ifdef CONFIG_HISI_HW_DIAG
	hwdiag_apanic_info.cpu_info.cpu_num = smp_processor_id();
	hisi_hw_diaginfo_trace(CPU_PANIC_INFO, &hwdiag_apanic_info);
#endif

	if (watchdog_softlockup_happen()) {
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_SOFTLOCKUP, 0, 0);
	} else if (watchdog_othercpu_hardlockup_happen()) {
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_OTHERCPU_HARDLOCKUP, 0, 0);
	} else if (watchdog_sp805_hardlockup_happen()) {
		rdr_syserr_process_for_ap(MODID_AP_S_PANIC_SP805_HARDLOCKUP, 0, 0);
	} else {
		if (is_modem_exception())
			rdr_syserr_process_for_ap(MODID_AP_S_PANIC_MODEM, 0, 0);
		else
			rdr_syserr_process_for_ap(MODID_AP_S_PANIC, 0, 0);
	}

	return 0;
}

/*
 * Description : die reset hook function
 */
int rdr_hisiap_die_notify(struct notifier_block *nb,
				unsigned long event, void *p_reg)
{
	return 0;
}

