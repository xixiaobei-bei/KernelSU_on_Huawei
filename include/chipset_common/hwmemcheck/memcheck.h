#ifndef _MEMCHECK_H
#define _MEMCHECK_H

#ifdef __KERNEL__
#ifdef CONFIG_HW_MEMCHECK
#include <linux/types.h>
#include <asm/ioctls.h>
#endif
#else /* __KERNEL__ */
#include <sys/types.h>
#include <sys/ioctl.h>
#endif /* __KERNEL__ */

#define MEMCHECK_MAGIC			0x5377FEFA
#define MEMCHECK_CMD_INVALID		0xFFFFFFFF
#define MEMCHECK_PID_INVALID		0xFFDEADFF

#define MEMCHECK_DETAILINFO_MAXSIZE	(5 * 1024 * 1024)
#define MEMCHECK_STACKINFO_MAXSIZE	(5 * 1024 * 1024)
#define MEMCHECK_OOM_LMK_MAXNUM		10
#define MEMCHECK_PATH_MAX		16
#define MEMCHECK_TASK_MAXNUM		2000

#define __MEMCHECKIO			0xAF

#define LOGGER_MEMCHECK_GET_MEMSTAT	_IO(__MEMCHECKIO, 1)
#define LOGGER_MEMCHECK_COMMAND		_IO(__MEMCHECKIO, 2)
#define LOGGER_MEMCHECK_DETAIL_READ	_IO(__MEMCHECKIO, 3)
#define LOGGER_MEMCHECK_STACK_READ	_IO(__MEMCHECKIO, 4)
#define LOGGER_MEMCHECK_LMK_OOM_READ	_IO(__MEMCHECKIO, 5)
#define LOGGER_MEMCHECK_STACK_SAVE	_IO(__MEMCHECKIO, 6)
#define LOGGER_MEMCHECK_LMK_OOM_SAVE	_IO(__MEMCHECKIO, 7)
#define LOGGER_MEMCHECK_FREE_MEM	_IO(__MEMCHECKIO, 8)
#define LOGGER_MEMCHECK_GET_TASK_TYPE	_IO(__MEMCHECKIO, 9)

#define LOGGER_MEMCHECK_MIN		LOGGER_MEMCHECK_GET_MEMSTAT
#define LOGGER_MEMCHECK_MAX		LOGGER_MEMCHECK_GET_TASK_TYPE

#define IDX_USER_START			0
#define IDX_USER_PSS_JAVA		(IDX_USER_START + 0)
#define IDX_USER_PSS_NATIVE		(IDX_USER_START + 1)
#define IDX_USER_VSS			(IDX_USER_START + 2)
#define IDX_USER_ION			(IDX_USER_START + 3)
#define IDX_USER_END			IDX_USER_ION
#define IDX_KERN_START			(IDX_USER_END + 1)
#define IDX_KERN_ION			(IDX_KERN_START + 0)
#define IDX_KERN_SLUB			(IDX_KERN_START + 1)
#define IDX_KERN_LSLUB			(IDX_KERN_START + 2)
#define IDX_KERN_VMALLOC		(IDX_KERN_START + 3)
#define IDX_KERN_CMA			(IDX_KERN_START + 4)
#define IDX_KERN_ZSMALLOC		(IDX_KERN_START + 5)
#define IDX_KERN_BUDDY			(IDX_KERN_START + 6)
#define IDX_KERN_SKB			(IDX_KERN_START + 7)
#define IDX_KERN_END			IDX_KERN_SKB
#define IDX_START			IDX_USER_START

#define NUM_ALL_MAX			(IDX_KERN_END + 1)
#define NUM_KERN_MAX			(IDX_KERN_END - IDX_KERN_START + 1)

#define MTYPE_NONE			0
/* VSS > RSS > PSS > USS */
#define MTYPE_USER_PSS_JAVA		(1 << IDX_USER_PSS_JAVA)
#define MTYPE_USER_PSS_NATIVE		(1 << IDX_USER_PSS_NATIVE)
#define MTYPE_USER_VSS			(1 << IDX_USER_VSS)
#define MTYPE_USER_ION			(1 << IDX_USER_ION)
#define MTYPE_KERN_ION			(1 << IDX_KERN_ION)
#define MTYPE_KERN_SLUB			(1 << IDX_KERN_SLUB)
#define MTYPE_KERN_LSLUB		(1 << IDX_KERN_LSLUB)
#define MTYPE_KERN_VMALLOC		(1 << IDX_KERN_VMALLOC)
#define MTYPE_KERN_CMA			(1 << IDX_KERN_CMA)
#define MTYPE_KERN_ZSMALLOC		(1 << IDX_KERN_ZSMALLOC)
#define MTYPE_KERN_BUDDY		(1 << IDX_KERN_BUDDY)
#define MTYPE_KERN_SKB			(1 << IDX_KERN_SKB)

#define MTYPE_USER_PSS	(MTYPE_USER_PSS_JAVA | MTYPE_USER_PSS_NATIVE)
#define MTYPE_KERNEL	(MTYPE_KERN_ION | MTYPE_KERN_SLUB |	\
			MTYPE_KERN_LSLUB | MTYPE_KERN_VMALLOC |	\
			MTYPE_KERN_CMA | MTYPE_KERN_ZSMALLOC |	\
			MTYPE_KERN_BUDDY | MTYPE_KERN_SKB)
#define MTYPE_USER	(MTYPE_USER_PSS | MTYPE_USER_VSS | MTYPE_USER_ION)
#define MTYPE_ALL	(MTYPE_USER | MTYPE_KERNEL)

#define SIGNO_MEMCHECK			44
#define ADDR_JAVA_ENABLE		(1 << 0)
#define ADDR_JAVA_DISABLE		(1 << 1)
#define ADDR_JAVA_SAVE			(1 << 2)
#define ADDR_JAVA_CLEAR			(1 << 3)
#define ADDR_NATIVE_ENABLE		(1 << 4)
#define ADDR_NATIVE_DISABLE		(1 << 5)
#define ADDR_NATIVE_SAVE		(1 << 6)
#define ADDR_NATIVE_CLEAR		(1 << 7)

/* do not change the order or insert any value before MEMCMD_CLEAR_LOG */
enum memcmd {
	MEMCMD_NONE,
	MEMCMD_ENABLE,
	MEMCMD_DISABLE,
	MEMCMD_SAVE_LOG,
	MEMCMD_CLEAR_LOG,
	MEMCMD_MAX
};

enum kill_type {
	KILLTYPE_NONE,
	KILLTYPE_USER_LMK,
	KILLTYPE_KERNEL_LMK,
	KILLTYPE_JAVA_OOM,
	KILLTYPE_KERNEL_OOM,
	KILLTYPE_MAX
};

#ifdef __KERNEL__
struct memstat_all {
	u32 magic;
	u32 id;
	u16 type;

	/* for user space size */
	struct pss_user {
		u64 native_pss;
		u64 java_pss;
	} pss;
	u64 total_pss;
	u64 vss;
	u64 ion_pid;

	/* for kernel space size */
	u64 memory;
};

struct track_cmd {
	u32 magic;
	u32 id;
	u16 type;
	u64 timestamp;
	enum memcmd cmd;
};

struct detail_info {
	u32 magic;
	u16 type;
	u64 size;
	char data[0];
};

struct stack_info {
	u32 magic;
	u16 type;
	u64 size;
	char data[0];
};

struct lmk_oom_rec {
	pid_t pid;
	pid_t tgid;
	enum kill_type ktype;
	char name[MEMCHECK_PATH_MAX + 1];
	u16 adj;
	u64 pss;
	u32 timestamp;
};

struct lmk_oom_read {
	u32 magic;
	u64 num;
	struct lmk_oom_rec data[0];
};

struct lmk_oom_write {
	u32 magic;
	struct lmk_oom_rec data;
};

struct task_type_rec {
	pid_t pid;
	u8 is_32bit;
};

struct task_type_read {
	u32 magic;
	u64 num;
	struct task_type_rec data[0];
};

#ifdef CONFIG_HW_MEMCHECK
struct file;
long memcheck_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int memcheck_report_lmk_oom(pid_t pid, pid_t tgid, const char *name,
			    enum kill_type ktype, short adj, size_t pss);
#else /* CONFIG_HW_MEMCHECK */
static inline long memcheck_ioctl(struct file *file, unsigned int cmd,
				  unsigned long arg)
{
	return MEMCHECK_CMD_INVALID;
}

static inline int memcheck_report_lmk_oom(pid_t pid, pid_t tgid, const char *name,
					  enum kill_type ktype, short adj,
					  size_t pss)
{
	return 0;
}
#endif /* CONFIG_HW_MEMCHECK */

#else /* __KERNEL__ */

struct memstat_all {
	__u32 magic;
	__u32 id;
	__u16 type;

	/* for user space size */
	struct pss_user {
		__u64 native_pss;
		__u64 java_pss;
	} pss;
	__u64 total_pss;
	__u64 vss;
	__u64 ion_pid;

	/* for kernel space size */
	__u64 memory;
};

struct track_cmd {
	__u32 magic;
	__u32 id;
	__u16 type;
	__u64 timestamp;
	enum memcmd cmd;
};

struct detail_info {
	__u32 magic;
	__u16 type;
	__u64 size;
	char data[0];
};

struct stack_info {
	__u32 magic;
	__u16 type;
	__u64 size;
	char data[0];
};

struct lmk_oom_rec {
	pid_t pid;
	pid_t tgid;
	enum kill_type ktype;
	char name[MEMCHECK_PATH_MAX + 1];
	__u16 adj;
	__u64 pss;
	__u32 timestamp;
};

struct lmk_oom_read {
	__u32 magic;
	__u64 num;
	struct lmk_oom_rec data[0];
};

struct lmk_oom_write {
	__u32 magic;
	struct lmk_oom_rec data;
};

struct task_type_rec {
	pid_t pid;
	__u8 is_32bit;
};

struct task_type_read {
	__u32 magic;
	__u64 num;
	struct task_type_rec data[0];
};

#endif /* __KERNEL__ */

#endif /* _MEMCHECK_H */
