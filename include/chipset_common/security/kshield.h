#ifndef _LINUX_KSHIELD_H_
#define _LINUX_KSHIELD_H_

#include <linux/types.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/uaccess.h>
#include <linux/sched/user.h>
#include <asm/pgtable.h>
#include <asm/pgtable-types.h>
#include <linux/syscalls.h>

#ifdef CONFIG_SECURITY_KSHIELD

#define kshield_func_ret_int(no, name, ...) \
	extern int sub_chk_##name(__MAP(no, __SC_DECL, __VA_ARGS__)); \
	static __always_inline int kshield_chk_##name(__MAP(no, __SC_DECL, __VA_ARGS__)) { \
		if (likely(!current->inspected)) return 0; \
		return sub_chk_##name(__MAP(no, __SC_CAST, __VA_ARGS__)); \
	}

#define kshield_func_ret_void(no, name, ...) \
	extern void sub_chk_##name(__MAP(no, __SC_DECL, __VA_ARGS__)); \
	static __always_inline void kshield_chk_##name(__MAP(no, __SC_DECL, __VA_ARGS__)) { \
		if (likely(!current->inspected)) return; \
		sub_chk_##name(__MAP(no, __SC_CAST, __VA_ARGS__)); \
	}

#define kshield_func_ret_int0(name) extern int sub_chk_##name(void); \
	static __always_inline int kshield_chk_##name(void) { \
		if (likely(!current->inspected)) return 0; \
		return sub_chk_##name(); \
	}

#define kshield_func_ret_void0(name) extern void sub_chk_##name(void); \
	static __always_inline void kshield_chk_##name(void) { \
		if (likely(!current->inspected)) return; \
		sub_chk_##name(); \
	}

long ks_dev_ioctl(unsigned int cmd, char *cur);

#else /* !CONFIG_SECURITY_KSHIELD */

#define kshield_func_ret_int(no, name, ...) \
	static __always_inline int kshield_chk_##name(__MAP(no, __SC_DECL, __VA_ARGS__)) \
	{ return 0; }

#define kshield_func_ret_void(no, name, ...) \
	static __always_inline void kshield_chk_##name(__MAP(no, __SC_DECL, __VA_ARGS__)) {}

#define kshield_func_ret_int0(name) \
	static __always_inline int kshield_chk_##name(void) { return 0; }

#define kshield_func_ret_void0(name) \
	static __always_inline void kshield_chk_##name(void) {}

static inline long ks_dev_ioctl(unsigned int cmd, char *cur) { return 0; }

#endif /* CONFIG_SECURITY_KSHIELD */

#define kshield_func_ret_int4(name, ...) \
	kshield_func_ret_int(4, name, __VA_ARGS__)
#define kshield_func_ret_int3(name, ...) \
	kshield_func_ret_int(3, name, __VA_ARGS__)
#define kshield_func_ret_int2(name, ...) \
	kshield_func_ret_int(2, name, __VA_ARGS__)
#define kshield_func_ret_int1(name, ...) \
	kshield_func_ret_int(1, name, __VA_ARGS__)

#define kshield_func_ret_void4(name, ...) \
	kshield_func_ret_void(4, name, __VA_ARGS__)
#define kshield_func_ret_void3(name, ...) \
	kshield_func_ret_void(3, name, __VA_ARGS__)
#define kshield_func_ret_void2(name, ...) \
	kshield_func_ret_void(2, name, __VA_ARGS__)
#define kshield_func_ret_void1(name, ...) \
	kshield_func_ret_void(1, name, __VA_ARGS__)

/* EXP */
kshield_func_ret_int4(change_mem, unsigned long, start, unsigned long,
	size, pgprot_t, set_mask, pgprot_t, clear_mask);
kshield_func_ret_int1(heap_ret2dir, const void *, x);
kshield_func_ret_int2(fops, const void *, ops, size_t, ops_sz);
kshield_func_ret_int1(sysctl, const struct ctl_table *, table);
kshield_func_ret_int2(set_procattr, const char *, value, size_t, size);
#ifdef CONFIG_SECURITY_SELINUX
kshield_func_ret_int0(sel_write_load);
#endif
kshield_func_ret_int2(user_mem, const char __user *, buf, size_t, copy);

/* CHARACTERISTIC */
kshield_func_ret_void1(heap_spray, unsigned int, count);
kshield_func_ret_void1(sock_spray, unsigned int, count);
#ifdef CONFIG_USERFAULTFD
kshield_func_ret_void0(userfaultfd);
#endif
kshield_func_ret_void0(kinfo);
kshield_func_ret_void0(mlock);
kshield_func_ret_void0(set_affinity);
kshield_func_ret_void1(futex, int, nr_waiters);
kshield_func_ret_void0(process_num);
kshield_func_ret_void0(warn);

#endif /* _LINUX_SHIELD_H_ */
