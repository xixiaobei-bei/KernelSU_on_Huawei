/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: HKIP prmem support
 * Date: 2020/04/15
 */

#ifndef LINUX_HKIP_PRMEM_H
#define LINUX_HKIP_PRMEM_H

#include <linux/bug.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <asm/page.h>
#include <asm/sysreg.h>

#define HKIP_HVC_RO_REGISTER            0xC6001030
#define HKIP_HVC_RO_MOD_REGISTER        0xC6001032
#define HKIP_HVC_RO_MOD_UNREGISTER      0xC6001033
#define HKIP_HVC_ROWM_REGISTER          0xC6001040
#define HKIP_HVC_ROWM_MOD_REGISTER      0xC6001042
#define HKIP_HVC_ROWM_MOD_UNREGISTER    0xC6001043
#define HKIP_HVC_ROWM_SET_BIT           0xC6001050
#define HKIP_HVC_ROWM_WRITE             0xC6001051
#define HKIP_HVC_ROWM_WRITE_8           0xC6001054
#define HKIP_HVC_ROWM_WRITE_16          0xC6001055
#define HKIP_HVC_ROWM_WRITE_32          0xC6001056
#define HKIP_HVC_ROWM_WRITE_64          0xC6001057
#define HKIP_HVC_ROWM_SET               0xC6001058
#define HKIP_HVC_ROWM_CLEAR             0xC6001059
#define HKIP_HVC_ROWM_CLEAR_8           0xC600105A
#define HKIP_HVC_ROWM_CLEAR_16          0xC600105B
#define HKIP_HVC_ROWM_CLEAR_32          0xC600105C
#define HKIP_HVC_ROWM_CLEAR_64          0xC600105D
#define HKIP_HVC_ROWM_XCHG_8            0xC60010C8
#define HKIP_HVC_ROWM_XCHG_16           0xC60010C9
#define HKIP_HVC_ROWM_XCHG_32           0xC60010CA
#define HKIP_HVC_ROWM_XCHG_64           0xC60010CB
#define HKIP_HVC_ROWM_CMPXCHG_8         0xC60010CC
#define HKIP_HVC_ROWM_CMPXCHG_16        0xC60010CD
#define HKIP_HVC_ROWM_CMPXCHG_32        0xC60010CE
#define HKIP_HVC_ROWM_CMPXCHG_64        0xC60010CF
#define HKIP_HVC_ROWM_ADD_8             0xC60010D0
#define HKIP_HVC_ROWM_ADD_16            0xC60010D1
#define HKIP_HVC_ROWM_ADD_32            0xC60010D2
#define HKIP_HVC_ROWM_ADD_64            0xC60010D3
#define HKIP_HVC_ROWM_OR_8              0xC60010D4
#define HKIP_HVC_ROWM_OR_16             0xC60010D5
#define HKIP_HVC_ROWM_OR_32             0xC60010D6
#define HKIP_HVC_ROWM_OR_64             0xC60010D7
#define HKIP_HVC_ROWM_AND_8             0xC60010D8
#define HKIP_HVC_ROWM_AND_16            0xC60010D9
#define HKIP_HVC_ROWM_AND_32            0xC60010DA
#define HKIP_HVC_ROWM_AND_64            0xC60010DB
#define HKIP_HVC_ROWM_XOR_8             0xC60010DC
#define HKIP_HVC_ROWM_XOR_16            0xC60010DD
#define HKIP_HVC_ROWM_XOR_32            0xC60010DE
#define HKIP_HVC_ROWM_XOR_64            0xC60010DF

#ifdef CONFIG_HHEE

u32 hkip_hvc2(u32, unsigned long);
u32 hkip_hvc3(u32, unsigned long, unsigned long);
u32 hkip_hvc4(u32, unsigned long, unsigned long, unsigned long);

static inline int hkip_reg_unreg(unsigned long op, const void *base,
				 size_t size)
{
	uintptr_t addr = (uintptr_t)base;

	BUG_ON((addr | size) & ~PAGE_MASK);

	if (hkip_hvc3(op, addr, size))
		return -ENOTSUPP;
	return 0;
}

static inline int hkip_register_ro(const void *base, size_t size)
{
	return hkip_reg_unreg(HKIP_HVC_RO_REGISTER, base, size);
}

static inline int hkip_register_ro_mod(const void *base, size_t size)
{
	return hkip_reg_unreg(HKIP_HVC_RO_MOD_REGISTER, base, size);
}

static inline int hkip_unregister_ro_mod(const void *base, size_t size)
{
	return hkip_reg_unreg(HKIP_HVC_RO_MOD_UNREGISTER, base, size);
}

static inline int hkip_register_rowm(const void *base, size_t size)
{
	return hkip_reg_unreg(HKIP_HVC_ROWM_REGISTER, base, size);
}

static inline int hkip_register_rowm_mod(const void *base, size_t size)
{
	return hkip_reg_unreg(HKIP_HVC_ROWM_MOD_REGISTER, base, size);
}

static inline int hkip_unregister_rowm_mod(const void *base, size_t size)
{
	return hkip_reg_unreg(HKIP_HVC_ROWM_MOD_UNREGISTER, base, size);
}

/* It works like memcpy() */
static inline const void * hkip_write_rowm(const void *dst, const void *src,
					   size_t size)
{
	if (hkip_hvc4(HKIP_HVC_ROWM_WRITE, (unsigned long)(uintptr_t)dst,
		      (unsigned long)(uintptr_t)src, (unsigned long)size))
		return NULL;
	return dst;
}

/* It works like the assignment operator */
static inline uint8_t hkip_write_rowm_8(const void *dst, const uint8_t val)
{
	hkip_hvc3(HKIP_HVC_ROWM_WRITE_8, (unsigned long)(uintptr_t)dst,
		  (unsigned long)val);
	return *(uint8_t *)dst;
}

static inline uint16_t hkip_write_rowm_16(const void *dst, const uint16_t val)
{
	hkip_hvc3(HKIP_HVC_ROWM_WRITE_16, (unsigned long)(uintptr_t)dst,
		  (unsigned long)val);
	return *(uint16_t *)dst;
}

static inline uint32_t hkip_write_rowm_32(const void *dst, const uint32_t val)
{
	hkip_hvc3(HKIP_HVC_ROWM_WRITE_32, (unsigned long)(uintptr_t)dst,
		  (unsigned long)val);
	return *(uint32_t *)dst;
}

static inline uint64_t hkip_write_rowm_64(const void *dst, const uint64_t val)
{
	hkip_hvc3(HKIP_HVC_ROWM_WRITE_64, (unsigned long)(uintptr_t)dst,
		  (unsigned long)val);
	return *(uint64_t *)dst;
}

/* It works like memset() */
static inline const void * hkip_set_rowm(const void *dst, uint8_t val,
					 size_t size)
{
	hkip_hvc4(HKIP_HVC_ROWM_SET, (unsigned long)(uintptr_t)dst,
		  (unsigned long)val, (unsigned long)size);
	return dst;
}

static inline const void * hkip_clear_rowm(const void *dst, size_t size)
{
	hkip_hvc3(HKIP_HVC_ROWM_CLEAR, (unsigned long)(uintptr_t)dst,
		  (unsigned long)size);
	return dst;
}

static inline uint8_t hkip_clear_rowm_8(const void *dst)
{
	hkip_hvc2(HKIP_HVC_ROWM_CLEAR_8, (unsigned long)(uintptr_t)dst);
	return *(uint8_t *)dst;
}

static inline uint16_t hkip_clear_rowm_16(const void *dst)
{
	hkip_hvc2(HKIP_HVC_ROWM_CLEAR_16, (unsigned long)(uintptr_t)dst);
	return *(uint16_t *)dst;
}

static inline uint32_t hkip_clear_rowm_32(const void *dst)
{
	hkip_hvc2(HKIP_HVC_ROWM_CLEAR_32, (unsigned long)(uintptr_t)dst);
	return *(uint32_t *)dst;
}

static inline uint64_t hkip_clear_rowm_64(const void *dst)
{
	hkip_hvc2(HKIP_HVC_ROWM_CLEAR_64, (unsigned long)(uintptr_t)dst);
	return *(uint64_t *)dst;
}

#else

static inline int hkip_register_ro(const void *base, size_t size)
{
	return 0;
}

static inline int hkip_register_ro_mod(const void *base, size_t size)
{
	return 0;
}

static inline int hkip_unregister_ro_mod(const void *base, size_t size)
{
	return 0;
}

static inline int hkip_register_rowm(const void *base, size_t size)
{
	return 0;
}

static inline int hkip_register_rowm_mod(const void *base, size_t size)
{
	return 0;
}

static inline int hkip_unregister_rowm_mod(const void *base, size_t size)
{
	return 0;
}

static inline const void * hkip_write_rowm(const void *dst, const void *src,
					   size_t size)
{
	return memcpy((void *)dst, src, size);
}

static inline uint8_t hkip_write_rowm_8(const void *dst, const uint8_t val)
{
	return *(uint8_t *)dst = val;
}

static inline uint16_t hkip_write_rowm_16(const void *dst, const uint16_t val)
{
	return *(uint16_t *)dst = val;
}

static inline uint32_t hkip_write_rowm_32(const void *dst, const uint32_t val)
{
	return *(uint32_t *)dst = val;
}

static inline uint64_t hkip_write_rowm_64(const void *dst, const uint64_t val)
{
	return *(uint64_t *)dst = val;
}

static inline const void * hkip_set_rowm(const void *dst, uint8_t val,
					 size_t size)
{
	return memset((void *)dst, val, size);
}

static inline const void * hkip_clear_rowm(const void *dst, size_t size)
{
	return memset((void *)dst, 0, size);
}

static inline uint8_t hkip_clear_rowm_8(const void *dst)
{
	return *(uint8_t *)dst = 0;
}

static inline uint16_t hkip_clear_rowm_16(const void *dst)
{
	return *(uint16_t *)dst = 0;
}

static inline uint32_t hkip_clear_rowm_32(const void *dst)
{
	return *(uint32_t *)dst = 0;
}

static inline uint64_t hkip_clear_rowm_64(const void *dst)
{
	return *(uint64_t *)dst = 0;
}

#endif

#endif /* LINUX_HKIP_PRMEM_H */
