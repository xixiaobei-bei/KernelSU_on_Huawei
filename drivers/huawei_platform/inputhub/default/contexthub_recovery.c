/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: contexthub recovery source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */
#include "contexthub_recovery.h"

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/of_address.h>
#include <linux/pm_wakeup.h>
#include <linux/reboot.h>
#include <linux/regulator/consumer.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>
#include <soc_mid.h>

#include "contexthub_boot.h"
#include "contexthub_route.h"
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
#include "hisi_noc_modid_para.h"
#endif
#include "contexthub_pm.h"
#include "kbhub_channel.h"
#include "motion_channel.h"
#include "motion_detect.h"
#include "sensor_config.h"

#define DUMP_FAIL (-1)
#define DUMP_REGS_MAGIC 0x7a686f75
#define DUMP_REGS_SIZE 128
#define REG_NAME_SIZE 25
#define LOG_EXTRA_NUM 2
#define CUR_PATH_LEN 64
#ifndef CONFIG_INPUTHUB_30
BLOCKING_NOTIFIER_HEAD(iom3_recovery_notifier_list);

wait_queue_head_t iom3_rec_waitq;
#endif
atomic_t iom3_rec_state = ATOMIC_INIT(IOM3_RECOVERY_UNINIT);
#ifndef CONFIG_INPUTHUB_30
struct rdr_register_module_result current_sh_info = {0};
struct completion sensorhub_rdr_completion;
void *g_sensorhub_extend_dump_buff;
uint8_t *g_sensorhub_dump_buff;
uint32_t g_enable_dump = 1;
uint32_t g_dump_extend_size;
uint32_t g_dump_region_list[DL_BOTTOM][DE_BOTTOM];
static uint32_t *dp_regs;

static struct workqueue_struct *iom3_rec_wq;
static struct delayed_work iom3_rec_work;
#endif
int g_iom3_state = IOM3_ST_NORMAL;
struct completion iom3_rec_done;
#ifndef CONFIG_INPUTHUB_30
static struct wakeup_source iom3_rec_wl;
static void __iomem *iomcu_cfg_base;
static struct mutex mutex_recovery_cmd;
static u64 current_sh_core_id = RDR_IOM3; /* const */
static struct semaphore rdr_sh_sem;
static struct semaphore rdr_exce_sem;
static void __iomem *sysctrl_base;
static void __iomem *watchdog_base;
static unsigned int *dump_vir_addr;
static struct wakeup_source rdr_wl;
static int nmi_reg;
static char g_dump_dir[PATH_MAXLEN] = SH_DMP_DIR;
static char g_dump_fs[PATH_MAXLEN] = SH_DMP_FS;
static uint32_t g_dump_index = -1;
static u64 g_rdr_core_mask = RDR_IOM3 | RDR_AP | RDR_LPM3;
static bool g_sensorhub_dump_reset = true;
static struct ipc_dbg_msg_track g_iom3_msg_buffer;

static const char * const sh_reset_reasons[] = {
	"SH_FAULT_HARDFAULT", /* 0 */
	"SH_FAULT_BUSFAULT",
	"SH_FAULT_USAGEFAULT",
	"SH_FAULT_MEMFAULT",
	"SH_FAULT_NMIFAULT",
	"SH_FAULT_ASSERT",    /* 5 */
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",  /* 10 */
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",
	"UNKNOW DUMP FAULT",  /* 15 */
	"SH_FAULT_INTERNELFAULT",
	"SH_FAULT_IPC_RX_TIMEOUT",
	"SH_FAULT_IPC_TX_TIMEOUT",
	"SH_FAULT_RESET",
	"SH_FAULT_USER_DUMP",
	"SH_FAULT_RESUME",
	"SH_FAULT_REDETECT",
	"SH_FAULT_PANIC",
	"SH_FAULT_NOC",
	"SH_FAULT_EXP_BOTTOM",
};

static const char * const sh_regs_name[] = {
	"SOURCE", "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9",
	"R10", "R11", "R12", "EXP_SP", "EXP_LR", "EXP_PC", "SAVED_PSR",
	"CFSR", "HFSR", "DFSR", "MMFAR", "BFAR", "AFSR", "PSP",
	"MSP", "ARADDR_CHK", "AWADDR_CHK", "PERI_USED", "AO_CNT",
};
static int iom3_need_recovery_for_dump(int modid, enum exp_source f);

static int get_watchdog_base(void)
{
	struct device_node *np = NULL;

	if (!watchdog_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,iomcu-watchdog");
		if (!np) {
			hwlog_err("can not find watchdog node\n");
			return -1;
		}

		watchdog_base = of_iomap(np, 0);
		if (!watchdog_base) {
			hwlog_err("get watchdog_base  error\n");
			return -1;
		}
	}
	return 0;
}

static int get_sysctrl_base(void)
{
	struct device_node *np = NULL;

	if (!sysctrl_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
		if (!np) {
			hwlog_err("can not find sysctrl node\n");
			return -1;
		}

		sysctrl_base = of_iomap(np, 0);
		if (!sysctrl_base) {
			hwlog_err("get sysctrl_base error\n");
			return -1;
		}
	}
	return 0;
}

static void __get_dump_region_list(struct device_node *dump_node,
	const char *prop_name, enum dump_loc loc)
{
	int i;
	int cnt;
	struct property *prop = NULL;

	prop = of_find_property(dump_node, prop_name, NULL);
	hwlog_info("======== get dump region %s config from dts\n", prop_name);

	if (!prop || !prop->value) {
		hwlog_err("%s prop %s invalid\n", __func__, prop_name);
	} else {
		cnt = prop->length / sizeof(uint32_t);
		if (of_property_read_u32_array(dump_node, prop_name,
			g_dump_region_list[loc - 1], cnt)) {
			hwlog_err("%s read %s from dts fail\n", __func__, prop_name);
		} else {
			for (i = 0; i < cnt; i++) {
				g_config_on_ddr->dump_config.elements[g_dump_region_list[loc - 1][i]].loc = loc;
				hwlog_info("region %d\n", g_dump_region_list[loc - 1][i]);
			}
		}
	}
}

static void get_dump_config_from_dts(void)
{
	char *p_str = NULL;
	struct device_node *dump_node = NULL;

	dump_node = of_find_node_by_name(NULL, "sensorhub_dump");
	if (!dump_node) {
		hwlog_err("%s fail find dts node sensorhub_dump\n", __func__);
		goto OUT;
	}

	if (of_property_read_u32(dump_node, "enable", &g_enable_dump))
		hwlog_err("%s failed to find property enable\n", __func__);

	if (!g_enable_dump)
		goto OUT;

	__get_dump_region_list(dump_node, "tcm_regions", DL_TCM);

	if (of_property_read_u32(dump_node, "extend_region_size",
		&g_dump_extend_size))
		hwlog_err("%s find proper extend_region_size err\n", __func__);

	hwlog_info("%s sensorhub extend_region_size is 0x%x\n", __func__,
		g_dump_extend_size);

	if (g_dump_extend_size)
		__get_dump_region_list(dump_node, "ext_regions", DL_EXT);

	if (of_property_read_string(dump_node, "dump_dir", (const char **)&p_str)) {
		hwlog_err("%s failed to find property dump_dir use default\n",
			__func__);
		memset(g_dump_dir, 0, sizeof(g_dump_dir));
		strncpy(g_dump_dir, SH_DMP_DIR, PATH_MAXLEN - 1);
	}

	if (of_property_read_string(dump_node, "dump_fs", (const char **)&p_str)) {
		hwlog_err("%s failed to find property dump_dir use default\n",
			__func__);
		memset(g_dump_fs, 0, sizeof(g_dump_fs));
		strncpy(g_dump_fs, SH_DMP_FS, PATH_MAXLEN - 1);
	}

OUT:
	hwlog_info("%s sensorhub dump enabled is %d\n", __func__, g_enable_dump);
	hwlog_info("log dir is %s parent dir is %s\n", g_dump_dir, g_dump_dir);
	g_config_on_ddr->dump_config.enable = g_enable_dump;
}

static int write_ramdump_info_to_sharemem(void)
{
	if (!g_config_on_ddr) {
		hwlog_err("g_config_on_ddr is NULL, maybe ioremap %x failed\n",
			IOMCU_CONFIG_START);
		return -1;
	}

	g_config_on_ddr->dump_config.finish = SH_DUMP_INIT;
	g_config_on_ddr->dump_config.dump_addr = SENSORHUB_DUMP_BUFF_ADDR;
	g_config_on_ddr->dump_config.dump_size = SENSORHUB_DUMP_BUFF_SIZE;
	g_config_on_ddr->dump_config.ext_dump_addr = 0;
	g_config_on_ddr->dump_config.ext_dump_size = 0;
	g_config_on_ddr->dump_config.reason = 0;
	g_config_on_ddr->bbox_conifg.log_addr = current_sh_info.log_addr;
	g_config_on_ddr->bbox_conifg.log_size = current_sh_info.log_len;

	hwlog_warn("%s dumpaddr low:%x,dumpaddr high:%x,dumplen:%x,end\n",
		__func__, (u32)(g_config_on_ddr->dump_config.dump_addr),
		(u32)((g_config_on_ddr->dump_config.dump_addr) >> 32), /* 32:addr r_shift */
		g_config_on_ddr->dump_config.dump_size);
	hwlog_info("%s: bbox log addr: low 0x%x, log len: 0x%x, high 0x%x\n",
		__func__, (u32)(current_sh_info.log_addr), current_sh_info.log_len,
		(u32)(current_sh_info.log_addr >> 32)); /* 32:addr r_shift */
	return 0;
}

static void __send_nmi(void)
{
	hwlog_warn("%s: sysctrl_base is %pK, nmi_reg is 0x%x\n", __func__,
		sysctrl_base, nmi_reg);
	if (sysctrl_base && nmi_reg != 0)
		writel(0x2, sysctrl_base + nmi_reg);
}

static void notify_rdr_thread(void)
{
	up(&rdr_sh_sem);
}

#ifdef CONFIG_HISI_BB
static void sh_dump(u32 modid, u32 etype, u64 coreid, char *pathname,
	pfn_cb_dump_done pfn_cb)
{
	enum exp_source f = SH_FAULT_REACT;

#ifdef CONFIG_CONTEXTHUB_IGS
	if (modid == SENSORHUB_FDUL_MODID)
		f = SH_FAULT_NOC;
#endif
	hwlog_warn("%s: modid=0x%x, etype=0x%x, coreid=0x%llx, f=%d\n",
		__func__, modid, etype, coreid, f);

	if (modid_react_dump(modid))
		iom3_need_recovery_for_dump(modid, f);

	if (pfn_cb)
		pfn_cb(modid, current_sh_core_id);
}

static void sh_reset(u32 modid, u32 etype, u64 coreid)
{
	hwlog_info("%s\n", __func__);

	if (dump_vir_addr)
		memset(dump_vir_addr, 0, current_sh_info.log_len);
}

static int clean_rdr_memory(struct rdr_register_module_result rdr_info)
{
	dump_vir_addr = (unsigned int *)hisi_bbox_map(rdr_info.log_addr,
		rdr_info.log_len);
	if (!dump_vir_addr) {
		hwlog_err("hisi_bbox_map %llx failed in %s\n",
			rdr_info.log_addr, __func__);
		return -1;
	}
	memset(dump_vir_addr, 0, rdr_info.log_len);
	return 0;
}

static int write_bbox_log_info_to_sharemem(struct rdr_register_module_result *rdr_info)
{
	if (!g_config_on_ddr) {
		hwlog_err("write_bbox_log_info_to_sharemem: g_config_on_ddr is NULL\n");
		return -1;
	}

	if (!rdr_info) {
		hwlog_err("write_bbox_log_info_to_sharemem: rdr_info is NULL\n");
		return -1;
	}

	g_config_on_ddr->bbox_conifg.log_addr = rdr_info->log_addr;
	g_config_on_ddr->bbox_conifg.log_size = rdr_info->log_len;

	hwlog_info("%s bbox log addr:low 0x%x,log len:0x%x,high 0x%x\n",
		__func__, (u32)(rdr_info->log_addr), rdr_info->log_len,
		(u32)((rdr_info->log_addr) >> 32)); /* 32:addr r_shift */
	return 0;
}
static int rdr_sensorhub_register_core(void)
{
	struct rdr_module_ops_pub s_module_ops;
	int ret;

	hwlog_info("%s start\n", __func__);

	s_module_ops.ops_dump = sh_dump;
	s_module_ops.ops_reset = sh_reset;

	ret = rdr_register_module_ops(current_sh_core_id, &s_module_ops,
		&current_sh_info);
	if (ret != 0)
		return ret;

	ret = clean_rdr_memory(current_sh_info);
	if (ret != 0)
		return ret;

	ret = write_bbox_log_info_to_sharemem(&current_sh_info);
	if (ret != 0)
		return ret;

	hwlog_info("%s end\n", __func__);
	return ret;
}

static const char sh_module_name[] = "RDR_IOM7";
static const char sh_module_desc[] = "RDR_IOM7 for watchdog timeout issue.";
static const char sh_module_user_desc[] = "RDR_IOM7 for user trigger dump.";
static const char sh_module_fdul_desc[] = "RDR_IOM7 for FD_DL trigger dump.";

#ifdef CONFIG_HISI_NOC_MODID_REGISTER
static void rdr_sh_fdul_e_callback(u32 modid, void *arg1)
{
	/* this callback is called by other core */
	return;
}
#endif

static int sh_rdr_register_exception(void)
{
	struct rdr_exception_info_s einfo;
	int ret;

	hwlog_info("%s start\n", __func__);
	memset(&einfo, 0, sizeof(struct rdr_exception_info_s));
	einfo.e_modid = SENSORHUB_MODID;
	einfo.e_modid_end = SENSORHUB_MODID;
	einfo.e_process_priority = RDR_WARN;
	einfo.e_reboot_priority = RDR_REBOOT_NO;
	einfo.e_notify_core_mask = g_rdr_core_mask;
	einfo.e_reentrant = RDR_REENTRANT_ALLOW;
	einfo.e_exce_type = IOM3_S_EXCEPTION;
	einfo.e_from_core = RDR_IOM3;
	einfo.e_upload_flag = RDR_UPLOAD_YES;
	einfo.e_save_log_flags = RDR_SAVE_LOGBUF;
	memcpy(einfo.e_from_module, sh_module_name,
		(sizeof(sh_module_name) > MODULE_NAME_LEN) ?
		MODULE_NAME_LEN : sizeof(sh_module_name));
	memcpy(einfo.e_desc, sh_module_desc,
		(sizeof(sh_module_desc) > STR_EXCEPTIONDESC_MAXLEN) ?
		STR_EXCEPTIONDESC_MAXLEN : sizeof(sh_module_desc));
	ret = rdr_register_exception(&einfo);
	if (ret == 0) {
		hwlog_err("%s:regist SENSORHUB_MODID err %d\n", __func__, ret);
		return ret;
	}

	einfo.e_modid = SENSORHUB_USER_MODID;
	einfo.e_modid_end = SENSORHUB_USER_MODID;
	einfo.e_exce_type = IOM3_S_USER_EXCEPTION;
	memcpy(einfo.e_desc, sh_module_user_desc,
		(sizeof(sh_module_user_desc) > STR_EXCEPTIONDESC_MAXLEN) ?
		STR_EXCEPTIONDESC_MAXLEN : sizeof(sh_module_user_desc));
	ret = rdr_register_exception(&einfo);
	if (ret == 0) {
		hwlog_err("%s:regist SENSORHUB_USER_MODID fail %d\n", __func__,
			ret);
		return ret;
	}

#ifdef CONFIG_CONTEXTHUB_IGS
	einfo.e_modid = SENSORHUB_FDUL_MODID;
	einfo.e_modid_end = SENSORHUB_FDUL_MODID;
	einfo.e_exce_type = IOM3_S_FDUL_EXCEPTION;
	einfo.e_notify_core_mask = g_rdr_core_mask;
	einfo.e_reset_core_mask = RDR_IOM3;
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
	einfo.e_callback = rdr_sh_fdul_e_callback;
#endif
	memcpy(einfo.e_desc, sh_module_fdul_desc,
		(sizeof(sh_module_fdul_desc) > STR_EXCEPTIONDESC_MAXLEN) ?
		STR_EXCEPTIONDESC_MAXLEN : sizeof(sh_module_fdul_desc));
	ret = rdr_register_exception(&einfo);
	if (ret == 0) {
		hwlog_err("%s:regist SENSORHUB_FDUL_MODID err %d\n", __func__,
			ret);
		return ret;
	}
#endif
	return ret;
}

static int sh_register_exception(void)
{
#ifdef CONFIG_CONTEXTHUB_IGS
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
	struct noc_err_para_s noc_err_info;
#endif
#endif
	int ret = sh_rdr_register_exception();
	if (!ret)
		return ret;

#ifdef CONFIG_CONTEXTHUB_IGS
#ifdef CONFIG_HISI_NOC_MODID_REGISTER
	noc_err_info.masterid = SOC_FD_UL_MID;
	noc_err_info.targetflow = 0xFF; /* any target flow */
	noc_err_info.bus = NOC_ERRBUS_SYS_CONFIG;
	noc_modid_register(noc_err_info, SENSORHUB_FDUL_MODID);
#endif
#endif
	return ret;
}
#endif

static void wdt_stop(void)
{
	writel(REG_UNLOCK_KEY, watchdog_base + WDOGLOCK);
	writel(0x0, watchdog_base + WDOGCTRL);
	writel(0x1, watchdog_base + WDOGINTCLR);
	writel(0x01, watchdog_base + WDOGLOCK);
}

static irqreturn_t watchdog_handler(int irq, void *data)
{
	wdt_stop();
	hwlog_warn("%s start\n", __func__);
	peri_used_request();
	__pm_stay_awake(&rdr_wl);
	/* release exception sem */
	up(&rdr_exce_sem);
	return IRQ_HANDLED;
}

static int __sh_create_dir(char *path, unsigned int len)
{
	int fd;
	mm_segment_t old_fs;

	if (len > CUR_PATH_LEN) {
		hwlog_err("invalid  parameter. len is %d\n", len);
		return -1;
	}
	if (!path) {
		hwlog_err("invalid  parameter. path:%pK\n", path);
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fd = sys_access(path, 0);
	if (fd != 0) {
		hwlog_info("sh: need create dir %s\n", path);
		fd = sys_mkdir(path, DIR_LIMIT);
		if (fd < 0) {
			hwlog_err("sh:create dir %s err ret = %d\n", path, fd);
			set_fs(old_fs);
			return fd;
		}

		hwlog_info("sh: create dir %s successed %d\n", path, fd);
	}
	set_fs(old_fs);
	return 0;
}

static void sh_wait_fs(const char *path)
{
	int fd;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	do {
		fd = sys_access(path, 0);
		if (fd) {
			msleep(10); /* 10:delay 10ms for get fs */
			hwlog_info("%s wait ...\n", __func__);
		}
	} while (fd);
	set_fs(old_fs);
}

static int sh_savebuf2fs(char *logpath, char *filename, void *buf, u32 len,
	u32 is_append)
{
	int ret;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char path[PATH_MAXLEN];

	if (!logpath || !filename || !buf || len <= 0) {
		hwlog_err("invalid para path:%pK, name:%pK buf:%pK len:0x%x\n",
			logpath, filename, buf, len);
		ret = -1;
		goto param_err;
	}

	snprintf(path, PATH_MAXLEN, "%s/%s", logpath, filename);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	flags = O_CREAT | O_RDWR | (is_append ? O_APPEND : O_TRUNC);
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		set_fs(old_fs);
		hwlog_err("%s():create file %s err\n", __func__, path);
		ret = -1;
		goto param_err;
	}

	vfs_llseek(fp, 0L, SEEK_END);
	ret = vfs_write(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		hwlog_err("%s:write file %s exception with ret %d\n",
			__func__, path, ret);
		goto write_err;
	}

	vfs_fsync(fp, 0);
write_err:
	filp_close(fp, NULL);

	ret = (int)sys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID);
	if (ret)
		hwlog_err("%s chown %s uid %d gid %d failed %d\n",
			__func__, path, ROOT_UID, SYSTEM_GID, ret);

	set_fs(old_fs);
param_err:
	return ret;
}

static int sh_readfs2buf(char *logpath, char *filename, void *buf, u32 len)
{
	int ret = -1;
	int flags;
	struct file *fp = NULL;
	mm_segment_t old_fs;
	char path[PATH_MAXLEN];

	if (!logpath || !filename || !buf || len <= 0) {
		hwlog_err("invalid para path:%pK, name:%pK buf:%pK len:0x%x\n",
			logpath, filename, buf, len);
		goto param_err;
	}

	snprintf(path, PATH_MAXLEN, "%s/%s", logpath, filename);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	if (sys_access(path, 0) != 0)
		goto file_err;

	flags = O_RDWR;
	fp = filp_open(path, flags, FILE_LIMIT);
	if (IS_ERR(fp)) {
		hwlog_err("%s():open file %s err\n", __func__, path);
		goto file_err;
	}

	vfs_llseek(fp, 0L, SEEK_SET);
	ret = vfs_read(fp, buf, len, &(fp->f_pos));
	if (ret != len) {
		hwlog_err("%s:read file %s exception with ret %d\n",
			__func__, path, ret);
		ret = -1;
	}
	filp_close(fp, NULL);
file_err:
	set_fs(old_fs);
param_err:
	return ret;
}

static int sh_create_dir(const char *path)
{
	char cur_path[CUR_PATH_LEN];
	int index = 0;

	if (!path) {
		hwlog_err("invalid  parameter. path:%pK\n", path);
		return -1;
	}
	memset(cur_path, 0, 64); /* 64: path len */
	if (*path != '/')
		return -1;
	cur_path[index++] = *path++;
	while (*path != '\0') {
		if (*path == '/')
			__sh_create_dir(cur_path, CUR_PATH_LEN);

		cur_path[index] = *path;
		path++;
		index++;
	}
	return 0;
}

static int get_dump_reason_idx(void)
{
	if (g_config_on_ddr->dump_config.reason >= ARRAY_SIZE(sh_reset_reasons))
		return ARRAY_SIZE(sh_reset_reasons) - 1;
	else
		return g_config_on_ddr->dump_config.reason;
}

static int write_sh_dump_history(void)
{
	int ret = 0;
	char buf[HISTORY_LOG_SIZE];
	struct kstat historylog_stat;
	mm_segment_t old_fs;
	char local_path[PATH_MAXLEN];
	char date[DATATIME_MAXLEN];

	hwlog_info("%s: write sensorhub dump history file\n", __func__);
	memset(date, 0, DATATIME_MAXLEN);
#ifdef CONFIG_HISI_BB
	snprintf(date, DATATIME_MAXLEN, "%s-%08lld", rdr_get_timestamp(),
		rdr_get_tick());
#endif

	memset(local_path, 0, PATH_MAXLEN);
	snprintf(local_path, PATH_MAXLEN, "%s/%s", g_dump_dir, "history.log");

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	/* check file size */
	if (vfs_stat(local_path, &historylog_stat) == 0 &&
		historylog_stat.size > HISTORY_LOG_MAX) {
		hwlog_info("truncate dump history log\n");
		sys_unlink(local_path); /* delete history.log */
	}

	set_fs(old_fs);
	/* write history file */
	memset(buf, 0, HISTORY_LOG_SIZE);
	if (dp_regs) {
		if (snprintf_s(buf, HISTORY_LOG_SIZE, HISTORY_LOG_SIZE - 1,
			"reason [%s], [%02d], time [%s], regs [pc-%x,lr-%x,fsr[c-%x,h-%x],far[m-%x,b-%x],peri-%x,ao-%x]\n",
			sh_reset_reasons[get_dump_reason_idx()], g_dump_index, date, dp_regs[EXP_PC],
			dp_regs[EXP_LR], dp_regs[CFSR], dp_regs[HFSR], dp_regs[MMFAR],
			dp_regs[BFAR], dp_regs[PERI_USED], dp_regs[AO_CNT]) == DUMP_FAIL)
			hwlog_err("%s write dp regs history failed\n", __func__);
	} else {
		if (snprintf_s(buf, HISTORY_LOG_SIZE, HISTORY_LOG_SIZE - 1, "reason [%s], [%02d], time [%s]\n",
			sh_reset_reasons[get_dump_reason_idx()], g_dump_index, date) == DUMP_FAIL)
			hwlog_err("%s write history failed\n", __func__);
	}
	sh_savebuf2fs(g_dump_dir, "history.log", buf, strlen(buf), 1);
	return ret;
}

static void get_max_dump_cnt(void)
{
	int ret;
	uint32_t index;

	/* find max index */
	ret = sh_readfs2buf(g_dump_dir, "dump_max", &index, sizeof(index));
	if (ret < 0)
		g_dump_index = -1;
	else
		g_dump_index = index;
	g_dump_index++;
	if (g_dump_index == MAX_DUMP_CNT)
		g_dump_index = 0;
	sh_savebuf2fs(g_dump_dir, "dump_max", &g_dump_index,
		sizeof(g_dump_index), 0);
}

static int write_sh_dump_file(void)
{
	char date[DATATIME_MAXLEN];
	char path[PATH_MAXLEN];
	struct dump_zone_head *dzh = NULL;

	memset(date, 0, DATATIME_MAXLEN);
#ifdef CONFIG_HISI_BB
	snprintf(date, DATATIME_MAXLEN, "%s-%08lld", rdr_get_timestamp(),
		rdr_get_tick());
#endif

	memset(path, 0, PATH_MAXLEN);
	snprintf(path, PATH_MAXLEN, "sensorhub-%02d.dmp", g_dump_index);
	hwlog_info("%s: write sensorhub dump  file %s\n", __func__, path);
	hwlog_err("sensorhub recovery source is %s\n",
		sh_reset_reasons[get_dump_reason_idx()]);
#if (KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE)
	flush_cache_all();
#endif

	/* write share part */
	if (g_sensorhub_dump_buff) {
		dzh = (struct dump_zone_head *)g_sensorhub_dump_buff;
		sh_savebuf2fs(g_dump_dir, path, g_sensorhub_dump_buff,
			min(g_config_on_ddr->dump_config.dump_size, dzh->len), 0);
	}
	/* write extend part */
	if (g_sensorhub_extend_dump_buff) {
		dzh = g_sensorhub_extend_dump_buff;
		sh_savebuf2fs(g_dump_dir, path, g_sensorhub_extend_dump_buff,
			min(g_config_on_ddr->dump_config.ext_dump_size, dzh->len), 1);
	}
	return 0;
}

static void get_sh_dump_regs(void)
{
	struct dump_zone_head *dzh = NULL;
	uint32_t i, magic;
	int32_t words = 0;
	int32_t ret;
	char buf[DUMP_REGS_SIZE] = {0};

	dzh = (struct dump_zone_head *)g_sensorhub_dump_buff;
	dp_regs = (uint32_t *)(g_sensorhub_dump_buff +
		sizeof(struct dump_zone_head) + sizeof(struct dump_zone_element_t) * dzh->cnt);
	if ((uintptr_t)(dp_regs + MAGIC) > (uintptr_t)(g_sensorhub_dump_buff + SENSORHUB_DUMP_BUFF_SIZE)) {
		hwlog_err("%s ramdump maybe failed\n", __func__);
		return;
	}
	magic = dp_regs[MAGIC];
	hwlog_info("%s dump regs magic 0x%x dump addr 0x%x dp regs addr 0x%x\n",
		__func__, magic, (uintptr_t)g_sensorhub_dump_buff, (uintptr_t)dp_regs);
	if (magic != DUMP_REGS_MAGIC) {
		dp_regs = NULL;
		return;
	}
	for (i = SOURCE; i < MAGIC; i++) {
		ret = snprintf_s(buf + words, DUMP_REGS_SIZE - words, REG_NAME_SIZE,
			"%s--%08x, ", sh_regs_name[i], dp_regs[i]);
		if (ret == DUMP_FAIL)
			return;
		words += ret;
		if (DUMP_REGS_SIZE - words < REG_NAME_SIZE + LOG_EXTRA_NUM) {
			buf[words] = '\n';
			buf[words + 1] = '\0';
			hwlog_info("%s", buf);
			words = 0;
		}
	}
}

static int save_sh_dump_file(void)
{
	if (!g_enable_dump) {
		hwlog_info("%s skipped\n", __func__);
		return 0;
	}
	sh_wait_fs(g_dump_fs);
	hwlog_info("%s fs ready\n", __func__);
	/* check and create dump dir */
	if (sh_create_dir(g_dump_dir)) {
		hwlog_err("%s fail to create dir %s\n", __func__, g_dump_dir);
		return -1;
	}
	get_max_dump_cnt();
	get_sh_dump_regs();
	/* write history file */
	write_sh_dump_history();
	/* write dump file */
	write_sh_dump_file();
	return 0;
}

static int rdr_sh_thread(void *arg)
{
	int timeout;

	hwlog_warn("%s start\n", __func__);

	while (1) {
		timeout = 2000; /* 2000:time max thred */
		down(&rdr_sh_sem);

		if (g_enable_dump) {
			hwlog_warn("======dump sensorhub log start========\n");
			while (g_config_on_ddr->dump_config.finish != SH_DUMP_FINISH &&
				timeout--)
				mdelay(1);
			hwlog_warn("=======sensorhub dump finished========\n");
			hwlog_warn("dump reason idx %d\n",
				g_config_on_ddr->dump_config.reason);
			/* write to fs */
			save_sh_dump_file();
			/* free buff */
			if (g_sensorhub_extend_dump_buff) {
				hwlog_info("%s free alloc\n", __func__);
				kfree(g_sensorhub_extend_dump_buff);
				g_sensorhub_extend_dump_buff = NULL;
			}
			hwlog_warn("=======dump sensorhub log end========\n");
		}
		__pm_relax(&rdr_wl);
		peri_used_release();
		complete_all(&sensorhub_rdr_completion);
	}

	return 0;
}

static int rdr_exce_thread(void *arg)
{
	hwlog_warn("%s start\n", __func__);
	while (1) {
		down(&rdr_exce_sem);
		hwlog_warn(" ==============trigger exception==============\n");
		iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_INTERNELFAULT);
	}

	return 0;
}

int register_iom3_recovery_notifier(struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&iom3_recovery_notifier_list, nb);
}
#endif

int get_iom3_state(void)
{
	return g_iom3_state;
}

int iom3_rec_sys_callback(const struct pkt_header *head)
{
	int ret;

#ifdef CONFIG_INPUTHUB_30
	iomcu_minready_done();
#endif
	if (atomic_read(&iom3_rec_state) == IOM3_RECOVERY_MINISYS) {
		if (((pkt_sys_statuschange_req_t *)head)->status == ST_MINSYSREADY) {
			hwlog_info("REC sys ready mini\n");
			ret = send_fileid_to_mcu();
			if (ret)
				hwlog_err("REC get sensors cfg err:%d\n", ret);
			else
				hwlog_info("REC get sensors cfg succ\n");
		} else if (((pkt_sys_statuschange_req_t *)head)->status == ST_MCUREADY) {
			hwlog_info("REC mcu all ready\n");
			ret = motion_set_cfg_data();
			if (ret < 0)
				hwlog_err("motion set cfg data err ret=%d\n", ret);
			ret = sensor_set_cfg_data();
			if (ret < 0)
				hwlog_err("REC sensor_chip_detect ret=%d\n", ret);
			else
				complete(&iom3_rec_done);
		}
	}

	return 0;
}

#ifndef CONFIG_INPUTHUB_30
static void notify_modem_when_iom3_recovery_finish(void)
{
	uint16_t status = ST_RECOVERY_FINISH;
	struct write_info pkg_ap;

	hwlog_info("notify_modem_when_iom3_recovery_finish\n");

	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_STATUSCHANGE_REQ;
	pkg_ap.wr_buf = &status;
	pkg_ap.wr_len = sizeof(status);
	write_customize_cmd(&pkg_ap, NULL, false);
}
#endif

static void disable_key_when_sysreboot(void)
{
	int ret;
	struct write_info winfo;

	if (strlen(get_sensor_chip_info_address(KEY)) == 0) {
		hwlog_err("no key\n");
		return;
	}

	winfo.tag = TAG_KEY;
	winfo.cmd = CMD_CMN_CLOSE_REQ;
	winfo.wr_len = 0;
	winfo.wr_buf = NULL;
	ret = write_customize_cmd(&winfo, NULL, false);
	if (ret < 0)
		hwlog_err("write close cmd err\n");

	hwlog_info("close key when reboot\n");
}

void enable_key_when_recovery_iom3(void)
{
	int ret;
	interval_param_t interval_param;

	memset(&interval_param, 0, sizeof(interval_param));
	if (strlen(get_sensor_chip_info_address(KEY)) == 0) {
		hwlog_err("no key\n");
		return;
	}

	hwlog_info("enable_key_when_recovery_iom3 ++\n");
	if (get_key_state()) { /* open */
		ret = inputhub_sensor_enable_nolock(TAG_KEY, true);
		if (ret) {
			hwlog_err("write open cmd err\n");
			return;
		}
		memset(&interval_param, 0, sizeof(interval_param));
		interval_param.period = 20; /* 20:interval period */
		interval_param.mode = AUTO_MODE;
		interval_param.batch_count = 1;
		ret = inputhub_sensor_setdelay_nolock(TAG_KEY, &interval_param);
		if (ret)
			hwlog_err("write interval cmd err\n");
	} else { /* close */
		ret = inputhub_sensor_enable_nolock(TAG_KEY, false);
		if (ret < 0)
			hwlog_err("write close cmd err\n");
	}
	hwlog_info("enable_key_when_recovery_iom3 --\n");
}

static void disable_sensors_when_sysreboot(void)
{
	int tag;

	for (tag = TAG_SENSOR_BEGIN; tag < TAG_SENSOR_END; ++tag) {
		if (sensor_status.status[tag]) {
			if (tag == TAG_STEP_COUNTER)
				inputhub_sensor_enable_stepcounter(false, TYPE_STANDARD);
			else
				inputhub_sensor_enable(tag, false);
			msleep(50); /* 50:wait 50ms after disable sensor */
			hwlog_info("disable sensor - %d before reboot\n", tag);
		}
	}
}
#ifndef CONFIG_INPUTHUB_30
static void operations_when_recovery_iom3(void)
{
	hwlog_info("operations_when_recovery_iom3\n");
	/* recovery key */
	enable_key_when_recovery_iom3();
	notify_modem_when_iom3_recovery_finish();
}

#ifdef CONFIG_HISI_BB
/*
 * return value
 * 0:success
 * <0:error
 */
static int rdr_sensorhub_init_early(void)
{
	int ret;
	/* register module. */
	ret = rdr_sensorhub_register_core();
	if (ret != 0)
		return ret;
	/* write ramdump info to share mem */
	ret = write_ramdump_info_to_sharemem();
	if (ret != 0)
		return ret;
	get_dump_config_from_dts();
	g_sensorhub_dump_buff = (uint8_t *)ioremap_wc(SENSORHUB_DUMP_BUFF_ADDR,
		SENSORHUB_DUMP_BUFF_SIZE);
	if (!g_sensorhub_dump_buff) {
		hwlog_err("%s failed remap dump buff\n", __func__);
		return -EINVAL;
	}
	/* regitster exception. 0:error */
	ret = sh_register_exception();
	if (ret == 0)
		ret = -EINVAL;
	else
		ret = 0;
	return ret;
}
#endif

static int sensorhub_panic_notify(struct notifier_block *nb,
	unsigned long action, void *data)
{
	int timeout;

	hwlog_warn("%s start\n", __func__);
	timeout = 100; /* 100:max time thred */
	__send_nmi();
	hwlog_warn("sensorhub_panic_notify\n");

	while ((g_config_on_ddr->dump_config.finish != SH_DUMP_FINISH) && (timeout--))
		mdelay(1);
	hwlog_warn("%s done\n", __func__);
	return NOTIFY_OK;
}

int sensorhub_noc_notify(int value)
{
	if (is_sensorhub_disabled()) {
		hwlog_err("[%s]do not handle noc as sensorhub is disabled\n", __func__);
		return 0;
	}

	hwlog_warn("%s start\n", __func__);
	iom3_need_recovery(SENSORHUB_MODID, SH_FAULT_NOC);
	wait_for_completion(&sensorhub_rdr_completion);
	hwlog_warn("%s done\n", __func__);

	return 0;
}

static struct notifier_block sensorhub_panic_block = {
	.notifier_call = sensorhub_panic_notify,
};

static int get_nmi_offset(void)
{
	struct device_node *sh_node = NULL;

	sh_node = of_find_compatible_node(NULL, NULL, "huawei,sensorhub_nmi");
	if (!sh_node) {
		hwlog_err("%s, can not find node sensorhub_nmi\n", __func__);
		return -1;
	}

	if (of_property_read_u32(sh_node, "nmi_reg", &nmi_reg)) {
		hwlog_err("%s:read nmi reg err:value is %d\n", __func__, nmi_reg);
		return -1;
	}

	hwlog_info("%s arch nmi reg is 0x%x\n", __func__, nmi_reg);
	return 0;
}

static int get_iomcu_cfg_base(void)
{
	struct device_node *np = NULL;

	if (!iomcu_cfg_base) {
		np = of_find_compatible_node(NULL, NULL, "hisilicon,iomcuctrl");
		if (!np) {
			hwlog_err("can not find  iomcuctrl node\n");
			return -1;
		}

		iomcu_cfg_base = of_iomap(np, 0);
		if (!iomcu_cfg_base) {
			hwlog_err("get iomcu_cfg_base  error\n");
			return -1;
		}
	}

	return 0;
}

static void show_iom3_stat(void)
{
	hwlog_err("CLK_SEL:0x%x,DIV0:0x%x,DIV1:0x%x,CLKSTAT0:0x%x, RSTSTAT0:0x%x\n",
		readl(iomcu_cfg_base + IOMCU_CLK_SEL),
		readl(iomcu_cfg_base + IOMCU_CFG_DIV0),
		readl(iomcu_cfg_base + IOMCU_CFG_DIV1),
		readl(iomcu_cfg_base + CLKSTAT0_OFFSET),
		readl(iomcu_cfg_base + RSTSTAT0_OFFSET));
}

static void reset_i2c_0_controller(void)
{
	unsigned long flags;

	local_irq_save(flags);
	writel(I2C_0_RST_VAL, iomcu_cfg_base + RSTEN0_OFFSET);
	udelay(5); /* 5:delay 5us between i2c write */
	writel(I2C_0_RST_VAL, iomcu_cfg_base + RSTDIS0_OFFSET);
	local_irq_restore(flags);
}

static void reset_sensor_power(void)
{
	int ret;

	if (get_no_need_sensor_ldo24()) {
		hwlog_info("%s: no_need set ldo24\n", __func__);
		return;
	}
	if (!need_reset_io_power) {
		hwlog_warn("%s: no need to reset sensor power\n", __func__);
		return;
	}

	if (IS_ERR(sensorhub_vddio)) {
		hwlog_err("%s: regulator_get fail\n", __func__);
		return;
	}
	ret = regulator_disable(sensorhub_vddio);
	if (ret < 0) {
		hwlog_err("failed to disable regulator sensorhub_vddio\n");
		return;
	}
	msleep(10); /* 10:delay 10ms after disable regulator */
	if (need_set_3v_io_power) {
		ret = regulator_set_voltage(sensorhub_vddio, SENSOR_VOLTAGE_3V,
			SENSOR_VOLTAGE_3V);
		if (ret < 0) {
			hwlog_err("failed set sensorhub_vddio volt to 3V\n");
			return;
		}
	}
	if (need_set_3_1v_io_power) {
		ret = regulator_set_voltage(sensorhub_vddio, SENSOR_VOLTAGE_3_1V,
			SENSOR_VOLTAGE_3_1V);
		if (ret < 0) {
			hwlog_err("failed set sensorhub_vddio volt to 3_1V\n");
			return;
		}
	}
	if (need_set_3_2v_io_power) {
		ret = regulator_set_voltage(sensorhub_vddio, SENSOR_VOLTAGE_3_2V,
			SENSOR_VOLTAGE_3_2V);
		if (ret < 0) {
			hwlog_err("failed set sensorhub_vddio volt to 3_2V\n");
			return;
		}
	}
	ret = regulator_enable(sensorhub_vddio);
	msleep(5); /* 5:delay 5ms after enable regulator */
	if (ret < 0) {
		hwlog_err("failed to enable regulator sensorhub_vddio\n");
		return;
	}
	hwlog_info("%s done\n", __func__);
}

static void iom3_recovery_work(struct work_struct *work)
{
	int rec_nest_count = 0;
	int rc;
	u32 ack_buffer;
	u32 tx_buffer;

	hwlog_err("%s enter\n", __func__);
	__pm_stay_awake(&iom3_rec_wl);
	peri_used_request();
	wait_for_completion(&sensorhub_rdr_completion);

recovery_iom3:
	if (rec_nest_count++ > IOM3_REC_NEST_MAX) {
		hwlog_err("unlucky recovery iom3 times exceed limit\n");
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_FAILED);
		blocking_notifier_call_chain(&iom3_recovery_notifier_list,
			IOM3_RECOVERY_FAILED, NULL);
		atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
		peri_used_release();
		__pm_relax(&iom3_rec_wl);
		hwlog_err("%s exit\n", __func__);
		return;
	}

	/* fix bug nmi can't be clear by iomcu, or iomcu will not start correctly */
	if ((unsigned int)readl(sysctrl_base + nmi_reg) & 0x2)
		hwlog_err("%s nmi remain\n", __func__);
	writel(0, sysctrl_base + nmi_reg);

	show_iom3_stat(); /* only for IOM3 debug */
	reset_sensor_power();
	/* reload iom3 system */
	tx_buffer = RELOAD_IOM3_CMD;
	rc = RPROC_ASYNC_SEND(get_ipc_ap_to_lpm_mbx(), &tx_buffer, 1);
	if (rc) {
		hwlog_err("RPROC reload iom3 failed %d, nest_count %d\n",
			rc, rec_nest_count);
		goto recovery_iom3;
	}

	show_iom3_stat(); /* only for IOM3 debug */
	reset_i2c_0_controller();
	reset_logbuff();
	write_ramdump_info_to_sharemem();
	write_timestamp_base_to_sharemem();

	msleep(5); /* 5:wait 5ms after write */
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_MINISYS);

	/* startup iom3 system */
	reinit_completion(&iom3_rec_done);
	tx_buffer = STARTUP_IOM3_CMD;
	rc = RPROC_SYNC_SEND(get_ipc_ap_to_iom_mbx(), &tx_buffer, 1,
		&ack_buffer, 1);
	if (rc) {
		hwlog_err("RPROC start iom3 failed %d, nest_count %d\n",
			rc, rec_nest_count);
		goto recovery_iom3;
	}

	hwlog_err("RPROC restart iom3 success\n");
	show_iom3_stat(); /* only for IOM3 debug */

	/* dynamic loading */
	if (!wait_for_completion_timeout(&iom3_rec_done, 5 * HZ)) { /* 5:time */
		hwlog_err("wait for iom3 system ready timeout\n");
		msleep(1000); /* 1000:wait 1000ms for iom3 system timeout */
		goto recovery_iom3;
	}

	/* repeat send cmd */
	msleep(100); /* 100:wait iom3 finish handle config-data */
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_DOING);
	hwlog_err("%s doing\n", __func__);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list,
		IOM3_RECOVERY_DOING, NULL);
	operations_when_recovery_iom3();
	/* recovery pdr */
	blocking_notifier_call_chain(&iom3_recovery_notifier_list,
		IOM3_RECOVERY_3RD_DOING, NULL);
	hwlog_err("%s pdr recovery\n", __func__);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
	__pm_relax(&iom3_rec_wl);
	hwlog_err("%s finish recovery\n", __func__);
	blocking_notifier_call_chain(&iom3_recovery_notifier_list,
		IOM3_RECOVERY_IDLE, NULL);
	hwlog_err("%s exit\n", __func__);
	peri_used_release();
}

void ipc_dbg_record(uint8_t dir, struct pkt_header *head, const char *info)
{
	int len;
	struct ipc_dbg_msg_track *pbuffer = &g_iom3_msg_buffer;
	struct ipc_dbg_msg *pmsg = NULL;

	len = strlen(info);
	if (len > IPC_DBG_MSG_SIZE)
		return;
	if (pbuffer->cur_site >= SENSORHUB_TRACK_SIZE)
		pbuffer->cur_site = 0;

	pmsg = &pbuffer->msg[pbuffer->cur_site];
	pmsg->dir = dir;
#ifdef CONFIG_HISI_BB
	pmsg->clock = hisi_getcurtime();
#endif
	memcpy(&pmsg->hd, head, sizeof(struct pkt_header));
	memcpy(pmsg->info, info, sizeof(char) * len);

	pbuffer->cur_site++;
}

static void ipc_dbg_record_dump(void)
{
	int i;
	uint64_t ts;
	unsigned long rem_nsec;

	hwlog_err("%s cur site = %d\n", __func__, g_iom3_msg_buffer.cur_site - 1);
	for (i = 0; i < SENSORHUB_TRACK_SIZE; i++) {
#ifdef CONFIG_HISI_BB
		ts = g_iom3_msg_buffer.msg[i].clock;
		rem_nsec = do_div(ts, NS_INTERVAL);
#endif

#ifndef CONFIG_INPUTHUB_30
		hwlog_err("%s [clk: %5lu.%06lus], dir=%d, tag=%d, cmd=%d, resp=%d, partial_order=%d, tranid=%d, length=%d, %s\n",
			__func__, (unsigned long)ts, rem_nsec / 1000, g_iom3_msg_buffer.msg[i].dir, /* 1000:accuracy */
			g_iom3_msg_buffer.msg[i].hd.tag, g_iom3_msg_buffer.msg[i].hd.cmd,
			g_iom3_msg_buffer.msg[i].hd.resp, g_iom3_msg_buffer.msg[i].hd.partial_order,
			g_iom3_msg_buffer.msg[i].hd.tranid, g_iom3_msg_buffer.msg[i].hd.length,
			g_iom3_msg_buffer.msg[i].info);
#else
		hwlog_err("%s [clk: %5lu.%06lus], dir=%d, tag=%d, cmd=%d, resp=%d, hw_trans_mode=%d, rsv=%d, partial_order=%d, tranid=%d, app_tag=%d, length=%d, %s\n",
			__func__, (unsigned long)ts, rem_nsec / 1000, g_iom3_msg_buffer.msg[i].dir, /* 1000:accuracy */
			g_iom3_msg_buffer.msg[i].hd.tag, g_iom3_msg_buffer.msg[i].hd.cmd,
			g_iom3_msg_buffer.msg[i].hd.resp, g_iom3_msg_buffer.msg[i].hd.hw_trans_mode,
			g_iom3_msg_buffer.msg[i].hd.rsv, g_iom3_msg_buffer.msg[i].hd.partial_order,
			g_iom3_msg_buffer.msg[i].hd.tranid, g_iom3_msg_buffer.msg[i].hd.app_tag,
			g_iom3_msg_buffer.msg[i].hd.length, g_iom3_msg_buffer.msg[i].info);
#endif
	}
}

static int __iom3_need_recovery(int modid, enum exp_source f, bool rdr_reset)
{
	int old_state;

	ipc_dbg_record_dump();
	if (!g_sensorhub_dump_reset) {
		hwlog_info("%s: skip dump reset\n", __func__);
		return 0;
	}

	old_state = atomic_cmpxchg(&iom3_rec_state, IOM3_RECOVERY_IDLE, IOM3_RECOVERY_START);
	hwlog_err("recovery prev state %d, modid 0x%x, f %u, rdr_reset %u\n",
		  old_state, modid, (uint8_t)f, (uint8_t)rdr_reset);

	/* prev state is IDLE start recovery progress */
	if (old_state == IOM3_RECOVERY_IDLE) {
		__pm_wakeup_event(&iom3_rec_wl, jiffies_to_msecs(10 * HZ)); /* 10:time accu */
		blocking_notifier_call_chain(&iom3_recovery_notifier_list,
			IOM3_RECOVERY_START, NULL);
		/* Complete the completion for response because for sensorhub reset and ipc will discard */
		complete(&type_record.resp_complete);

		if (f > SH_FAULT_INTERNELFAULT)
			g_config_on_ddr->dump_config.reason = (uint8_t)f;

		/* flush old logs */
		emg_flush_logbuff();

		/* write extend dump config */
		if (g_enable_dump && g_dump_extend_size &&
			!g_sensorhub_extend_dump_buff) {
			g_sensorhub_extend_dump_buff =
				kmalloc(g_dump_extend_size, GFP_KERNEL);
			hwlog_warn("%s alloc pages logic %pK phy addr 0x%lx\n", __func__,
				g_sensorhub_extend_dump_buff,
				virt_to_phys(g_sensorhub_extend_dump_buff));

			if (g_sensorhub_extend_dump_buff) {
				g_config_on_ddr->dump_config.ext_dump_addr =
					virt_to_phys(g_sensorhub_extend_dump_buff);
				g_config_on_ddr->dump_config.ext_dump_size = g_dump_extend_size;
				barrier();
			}
		}

#ifdef CONFIG_HISI_BB
		if (rdr_reset)
			rdr_system_error(modid, 0, 0);
#endif

		reinit_completion(&sensorhub_rdr_completion);
		__send_nmi();
		notify_rdr_thread();
		queue_delayed_work(iom3_rec_wq, &iom3_rec_work, 0);
	} else if ((f == SH_FAULT_INTERNELFAULT) &&
		completion_done(&sensorhub_rdr_completion)) {
		__pm_relax(&rdr_wl);
		peri_used_release();
	}
	return 0;
}

int iom3_need_recovery(int modid, enum exp_source f)
{
	return __iom3_need_recovery(modid, f, true);
}

static int iom3_need_recovery_for_dump(int modid, enum exp_source f)
{
	return __iom3_need_recovery(modid, f, false);
}

static int shb_recovery_notifier(struct notifier_block *nb,
		unsigned long foo, void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("%s %lu +\n", __func__, foo);
	mutex_lock(&mutex_recovery_cmd);
	switch (foo) {
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
		g_iom3_state = IOM3_ST_RECOVERY;
		break;
	case IOM3_RECOVERY_DOING:
	case IOM3_RECOVERY_3RD_DOING:
		g_iom3_state = IOM3_ST_REPEAT;
		break;
	case IOM3_RECOVERY_FAILED:
		hwlog_err("%s -recovery failed\n", __func__);
		/* fall-through */
	case IOM3_RECOVERY_IDLE:
		g_iom3_state = IOM3_ST_NORMAL;
		wake_up_all(&iom3_rec_waitq);
		break;
	default:
		hwlog_err("%s -unknow state %ld\n", __func__, foo);
		break;
	}
	mutex_unlock(&mutex_recovery_cmd);
	hwlog_info("%s -\n", __func__);
	return 0;
}
#endif
static int shb_reboot_notifier(struct notifier_block *nb, unsigned long foo,
	void *bar)
{
	/* prevent access the emmc now: */
	hwlog_info("shb:%s: %lu +\n", __func__, foo);
	if (foo == SYS_RESTART) {
		disable_sensors_when_sysreboot();
		disable_motions_when_sysreboot();
		disable_fingerprint_when_sysreboot();
		disable_fingerprint_ud_when_sysreboot();
		disable_key_when_sysreboot();
		disable_kb_when_sysreboot();
	}
	hwlog_info("shb:%s: -\n", __func__);
	return 0;
}

static struct notifier_block reboot_notify = {
	.notifier_call = shb_reboot_notifier,
	.priority = -1,
};

#ifndef CONFIG_INPUTHUB_30
static struct notifier_block recovery_notify = {
	.notifier_call = shb_recovery_notifier,
	.priority = -1,
};

static int rdr_sensorhub_init(void)
{
	int ret = 0;

#ifdef CONFIG_HISI_BB
	if (rdr_sensorhub_init_early() != 0) {
		hwlog_err("rdr_sensorhub_init_early faild\n");
		ret = -EINVAL;
	}
#endif
	sema_init(&rdr_sh_sem, 0);
	if (!kthread_run(rdr_sh_thread, NULL, "rdr_sh_thread")) {
		hwlog_err("create thread rdr_sh_main_thread faild\n");
		ret = -EINVAL;
		return ret;
	}
	sema_init(&rdr_exce_sem, 0);
	if (!kthread_run(rdr_exce_thread, NULL, "rdr_exce_thread")) {
		hwlog_err("create thread rdr_sh_exce_thread faild\n");
		ret = -EINVAL;
		return ret;
	}
	if (get_sysctrl_base() != 0) {
		hwlog_err("get sysctrl addr faild\n");
		ret = -EINVAL;
		return ret;
	}
	if (get_watchdog_base() != 0) {
		hwlog_err("get watchdog addr faild\n");
		ret = -EINVAL;
		return ret;
	}
	if (g_sensorhub_wdt_irq < 0) {
		hwlog_err("%s g_sensorhub_wdt_irq get error\n", __func__);
		return -EINVAL;
	}
	if (request_irq(g_sensorhub_wdt_irq, watchdog_handler, 0,
		"watchdog", NULL)) {
		hwlog_err("%s failure requesting watchdog irq\n", __func__);
		return -EINVAL;
	}
	if (get_nmi_offset() != 0)
		hwlog_err("get_nmi_offset faild\n");
	if (atomic_notifier_chain_register(&panic_notifier_list,
		&sensorhub_panic_block))
		hwlog_err("%s sensorhub panic register failed\n", __func__);
	wakeup_source_init(&rdr_wl, "rdr_sensorhub");
	init_completion(&sensorhub_rdr_completion);
	return ret;
}

#ifdef CONFIG_HISI_DEBUG_FS
#ifdef CONFIG_HISI_BB
static void sh_rdr_unregister_exception(void)
{
	(void)rdr_unregister_exception(SENSORHUB_MODID);
	(void)rdr_unregister_exception(SENSORHUB_USER_MODID);

#ifdef CONFIG_CONTEXTHUB_IGS
	(void)rdr_unregister_exception(SENSORHUB_FDUL_MODID);
#endif
}

static int sh_debugfs_u64_set(void *data, u64 val)
{
	int ret = 0;
	u64 mask_val;

	if (!data || !val) {
		hwlog_warn("%s invalid para\n", __func__);
		return -1;
	}

	mask_val = val & (RDR_AP | RDR_LPM3 | RDR_IOM3);

	hwlog_info("%s data=0x%llx, rdr mask=0x%llx, val=0x%llx, mask_val=0x%llx\n",
		__func__, *(u64 *)data, g_rdr_core_mask, val, mask_val);

	if (mask_val != g_rdr_core_mask) {
		*(u64 *)data = mask_val;
		sh_rdr_unregister_exception();
		ret = sh_rdr_register_exception();
		if (ret != 0) {
			hwlog_err("%s sh_rdr_register_exception failed %d\n", __func__, ret);
			return ret;
		}
	}

	return ret;
}

static int sh_debugfs_u64_get(void *data, u64 *val)
{
	if (!data || !val) {
		hwlog_warn("%s invalid para\n", __func__);
		return -1;
	}

	hwlog_info("%s data=0x%llx, rdr mask=0x%llx\n", __func__, *(u64 *)data,
		g_rdr_core_mask);
	*val = *(u64 *)data;
	return 0;
}

DEFINE_DEBUGFS_ATTRIBUTE(sh_fops_x64, sh_debugfs_u64_get, sh_debugfs_u64_set, "0x%016llx\n");

static struct dentry *sh_debugfs_create_x64(const char *name, umode_t mode,
	struct dentry *parent, u64 *value)
{
	hwlog_info("%s enter\n", __func__);
	return debugfs_create_file_unsafe(name, mode, parent, value, &sh_fops_x64);
}
#endif

static int rdr_create_file()
{
	struct dentry *ch_root = NULL;
	struct dentry *file_dentry = NULL;

	ch_root = debugfs_lookup("contexthub", NULL);
	if (IS_ERR_OR_NULL(ch_root)) {
		ch_root = debugfs_create_dir("contexthub", NULL);
		if (IS_ERR_OR_NULL(ch_root)) {
			hwlog_err("%s debugfs_create_dir contexthub err %ld\n",
				__func__, PTR_ERR(ch_root));
			ch_root = NULL;
			goto ERR_CREATE_DBGDIR;
		}
	} else {
		hwlog_info("%s dir contexthub contexthub exist\n", __func__);
	}

	/* 0660 : mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP */
	file_dentry = debugfs_create_bool("dump_reset", 0660, ch_root, &g_sensorhub_dump_reset);
	if (IS_ERR_OR_NULL(file_dentry)) {
		hwlog_err("%s debugfs_create_file dump_reset err\n", __func__);
		goto ERR_CREATE_DBGFS;
	}

#ifdef CONFIG_HISI_BB
	/* 0660 : mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP */
	file_dentry = sh_debugfs_create_x64("rdr_core_mask", 0660, ch_root, &g_rdr_core_mask);
	if (IS_ERR_OR_NULL(file_dentry)) {
		hwlog_err("%s debugfs_create_file rdr_core_mask failed\n", __func__);
		goto ERR_CREATE_DBGFS;
	}
#endif
	return 0;

ERR_CREATE_DBGFS:
	if (ch_root) {
		debugfs_remove_recursive(ch_root);
		ch_root = NULL;
	}

ERR_CREATE_DBGDIR:
	return -1;
}
#endif

int recovery_init(void)
{
	int ret;

	if (get_iomcu_cfg_base())
		return -1;
	ret = rdr_sensorhub_init();
	if (ret < 0) {
		hwlog_err("%s rdr_sensorhub_init ret=%d\n", __func__, ret);
		return -EFAULT;
	}
	mutex_init(&mutex_recovery_cmd);
	atomic_set(&iom3_rec_state, IOM3_RECOVERY_IDLE);
	iom3_rec_wq = create_singlethread_workqueue("iom3_rec_wq");
	if (!iom3_rec_wq) {
		hwlog_err("faild create iom3 recovery workqueue in %s\n", __func__);
		return -1;
	}

	INIT_DELAYED_WORK(&iom3_rec_work, iom3_recovery_work);
	init_completion(&iom3_rec_done);
	wakeup_source_init(&iom3_rec_wl, "iom3_rec_wl");

	init_waitqueue_head(&iom3_rec_waitq);
	register_iom3_recovery_notifier(&recovery_notify);
	register_reboot_notifier(&reboot_notify);
#ifdef CONFIG_HISI_DEBUG_FS
	rdr_create_file();
#endif
	return 0;
}
#else
int recovery_init(void)
{
	register_reboot_notifier(&reboot_notify);
	return 0;
}
#endif
