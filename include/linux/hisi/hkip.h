/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2021. All rights reserved.
 * Description: HKIP protect hooks
 * Date: 2017/08/15
 */

#ifndef __HKIP_H__
#define __HKIP_H__

#include <asm/alternative.h>
#include <asm/cpufeature.h>
#include <linux/sched.h>
#include <linux/threads.h>
#include <linux/types.h>
#include <linux/hisi/hkip_hhee.h>
#include <linux/hisi/hhee_prmem.h>

static inline bool hhee_is_present(void)
{
	bool ret = false;
#ifdef CONFIG_HHEE
	if (hhee_check_enable() == HHEE_ENABLE)
		ret = true;
#endif
	return ret;
}

u32 hkip_hvc2(u32, unsigned long);
u32 hkip_hvc3(u32, unsigned long, unsigned long);
u32 hkip_hvc4(u32, unsigned long, unsigned long, unsigned long);

static inline bool hkip_get_bit(const u8 *bits, size_t pos, size_t max)
{
	if (unlikely(pos >= max))
		return false;
	return ((unsigned int)READ_ONCE(bits[pos / 8]) >> (pos % 8)) & 1;
}

static inline void hkip_set_bit(u8 *bits, size_t pos, size_t max, bool value)
{
	if (hkip_get_bit(bits, pos, max) == value)
		return;

	if (unlikely(hkip_hvc4(HKIP_HVC_ROWM_SET_BIT, (unsigned long)(uintptr_t)bits,
				pos, value)))
		BUG();
}

static inline bool __hkip_get_task_bit(const u8 *bits,
					struct task_struct *task)
{
	return hkip_get_bit(bits, task_pid_nr(task), PID_MAX_DEFAULT);
}

static inline bool hkip_get_task_bit(const u8 *bits, struct task_struct *task,
					bool def_value)
{
	pid_t pid = task_pid_nr(task);
	if (pid != 0)
		return hkip_get_bit(bits, pid, PID_MAX_DEFAULT);
	return def_value;
}

static inline void hkip_set_task_bit(u8 *bits, struct task_struct *task,
					bool value)
{
	pid_t pid = task_pid_nr(task);
	if (pid != 0)
		hkip_set_bit(bits, pid, PID_MAX_DEFAULT, value);
}

static inline bool __hkip_get_current_bit(const u8 *bits)
{
	return __hkip_get_task_bit(bits, current);
}

static inline bool hkip_get_current_bit(const u8 *bits, bool def_value)
{
	return hkip_get_task_bit(bits, current, def_value);
}

static inline void hkip_set_current_bit(u8 *bits, bool value)
{
	hkip_set_task_bit(bits, current, value);
}

struct cred;

#ifdef CONFIG_UEFI_HHEE
int hkip_register_rox_text(const void *base, size_t size);
#else
static inline int hkip_register_rox_text(const void *base, size_t size)
{
	return 0;
}
#endif

#if defined(CONFIG_HHEE) && !defined(CONFIG_UEFI_HHEE)
int hkip_register_xo(const uintptr_t base, size_t size);

extern u8 hkip_addr_limit_bits[];

#define hkip_is_kernel_fs() \
	((current_thread_info()->addr_limit == KERNEL_DS) \
	&& hkip_get_current_bit(hkip_addr_limit_bits, true))
#define hkip_get_fs() \
	(hkip_is_kernel_fs() ? KERNEL_DS : USER_DS)
#define hkip_set_fs(fs) \
	hkip_set_current_bit(hkip_addr_limit_bits, (fs) == KERNEL_DS)

#ifndef CONFIG_HKIP_PROTECT_CRED
int hkip_check_uid_root(void);
int hkip_check_gid_root(void);
int hkip_check_xid_root(void);
void hkip_update_xid_root(const struct cred *creds);
#endif

void hkip_init_task(struct task_struct *task);

#else

#define hkip_set_fs(fs) ((void)(fs))

#ifndef CONFIG_HKIP_PROTECT_CRED
static inline int hkip_check_uid_root(void)
{
	return 0;
}
static inline int hkip_check_gid_root(void)
{
	return 0;
}
static inline int hkip_check_xid_root(void)
{
	return 0;
}
static inline void hkip_update_xid_root(const struct cred *creds) { }
#endif

static inline void hkip_init_task(struct task_struct *task) { }

static inline int hkip_register_xo(const uintptr_t base, size_t size)
{
	return 0;
}

#endif

#endif /* __HKIP_H__ */
