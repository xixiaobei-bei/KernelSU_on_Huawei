/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
 
#ifndef __OSL_IRQ_H
#define __OSL_IRQ_H
#include "product_config.h"

#ifdef __KERNEL__
#include <linux/interrupt.h>

#define OSL_IRQ_FUNC(ret, func, irq, dev)  \
	ret func(int irq, void* dev)
#define osl_free_irq(irq,routine,dev_id) free_irq(irq,(void*)dev_id)

#elif defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__)
#include "sre_base.h"
#include "sre_hwi.h"
#include "sre_shell.h"
#include "sre_sys.h"
#include "osl_printf.h"
#define IRQF_NO_SUSPEND 0xA55ABCDE
typedef enum {
    IRQ_NONE,
    IRQ_HANDLED
} irqreturn_t;
typedef irqreturn_t (*irq_handler_t)(void *);

#define OSL_IRQ_FUNC(ret, func, irq, arg)  \
	ret func(void *arg)

#ifndef OK
#define OK                  (0)
#endif

#ifndef ERROR
#define ERROR               (1)
#endif

//lint -esym(683,*)

/*该宏传参不准传入__specific_flags名字*/
/*
*参数类型
*unsigned long __specific_flags
*/
#define local_irq_save(__specific_flags)	    \
	do \
    {                           \
		__specific_flags = (unsigned long)SRE_IntLock();			\
	} while (0)/*lint !e683*/
/*
*参数类型
*unsigned long __specific_flags
*/
#define local_irq_restore(__specific_flags)   \
	do \
    {                           \
        SRE_IntRestore((unsigned int)__specific_flags);          \
	} while (0)/*lint !e683*/

//lint +esym(683,*)

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
        const char *name, void *arg);

static inline void free_irq(unsigned int irq, void *arg)
{
    Ulong ret = SRE_HwiDisable((HwiHandle)irq);
    if(ret != OK)
    {
        osl_printf("SRE_HwiDisable irq %d errorNO 0x%x\n", irq, ret);
        return;
    }
}
static inline void osl_free_irq(unsigned int irq, irq_handler_t routine,int para)
{
    Ulong ret = SRE_HwiDisable((HwiHandle)irq);
    if(ret != OK)
    {
        osl_printf("SRE_HwiDisable irq %d error,ret=0x%x\n", irq,ret);
        return;
    }
}

static inline  int enable_irq(unsigned int irq)
{
    return (int)SRE_HwiEnable((HwiHandle)irq);
}
static inline int disable_irq( unsigned int num )
{
    if(OS_ERRNO_HWI_NUM_INVALID == SRE_HwiDisable((HwiHandle)num))
    {
        return ERROR;
    }
    return OK;
}
static inline int osl_int_connect( unsigned int num, irq_handler_t routine, int parameter )
{
    unsigned int ret = 0;
    ret = SRE_HwiSetAttr((HwiHandle)num, SRE_HWI_PRIORITY0, OS_HWI_MODE_ENGROSS);
    if(ret){
        osl_printf("irq 0x%x SRE_HwiSetAttr failed,ret = 0x%x\n", num,ret);
        return ERROR;
    }
    return (int)SRE_HwiCreate((HwiHandle)num,(HwiProcFunc)routine,(HwiArg)parameter);
}
static inline int osl_int_disconnect( unsigned int num, irq_handler_t routine, int parameter )
{
    return (int)SRE_HwiDelete((HwiHandle)num);
}
static inline unsigned int osl_int_context(void)
{
	enum OsThreadType ret;
	ret = SRE_CurThreadType();
      if(SYS_TASK == ret)
    {
		return 0;
	}
	else
	{
		return 1;
	}
}
static inline unsigned int osl_int_context_disirq(void)
{
	enum OsThreadType ret;
	#ifdef __OS_RTOSCK_SMP__
	ret = SRE_CurThreadTypeNoIntLock();
	#else
	ret = SRE_CurThreadType();
	#endif
    if(SYS_TASK == ret)
    {
		return 0;
	}
	else
	{
		return 1;
	}
}

static inline int osl_clear_pending_bit(unsigned int irq)
{
	return (int)SRE_HwiClearPendingBit((HwiHandle)irq);
}

#define PSR_I_BIT	0x00000080
static inline unsigned long arch_local_save_flags(void)
{
	unsigned long flags;
	__asm__ __volatile__(
		"	mrs	%0, cpsr	@ local_save_flags"
		: "=r" (flags) : : "memory", "cc");
	return flags;/*lint !e530*/
}
static inline int arch_irqs_disabled_flags(unsigned long flags)
{
	return flags & PSR_I_BIT;
}
static inline int irqs_disabled(void)
{
	return (int)arch_irqs_disabled_flags(arch_local_save_flags());
}
#if defined(__OS_RTOSCK_SMP__)
#define IPI_CALL_FUNC OS_HWI_IPI_NO_014
#define IPI_CPU_STOP OS_HWI_IPI_NO_015
#define IPI_CPU_DOWN OS_HWI_IPI_NO_013
#elif defined(__OS_RTOSCK__)
#define IPI_CALL_FUNC 1024
#define IPI_CPU_STOP 1024
#define IPI_CPU_DOWN 1024
#endif
static inline void smp_cross_call(unsigned int mask,unsigned int irq)
{
	#if defined(__OS_RTOSCK_SMP__)
	SRE_HwiMcTrigger(OS_TYPE_TRIGGER_BY_MASK, mask, irq);
	#endif
}
#if defined(__OS_RTOSCK_SMP__) 
static inline void smp_cross_call_trigger(UINT32 eIPIType, UINT32 uwCoreMsk, UINT32 uwHwiNum)
{
	SRE_HwiMcTrigger((enum OsHwiIpiType)eIPIType,uwCoreMsk,uwHwiNum);
}
static inline unsigned int irq_set_affinity(unsigned int irq, unsigned int coremask)
{
	return SRE_HwiSetAffinity(irq,coremask);
}
#elif defined(__OS_RTOSCK__)
static inline void smp_cross_call_trigger(UINT32 eIPIType, UINT32 uwCoreMsk, UINT32 uwHwiNum)
{
	return ;
}
static inline unsigned int irq_set_affinity(unsigned int irq, unsigned int coremask)
{
	return 0;
}
#endif
static inline void arch_local_irq_enable(void)
{
	__asm__ __volatile__(
		"	cpsie i			@ arch_local_irq_enable"
		:
		:
		: "memory", "cc");
}

static inline void arch_local_irq_disable(void)
{
	__asm__ __volatile__(
		"	cpsid i			@ arch_local_irq_disable"
		:
		:
		: "memory", "cc");
}

#define local_fiq_enable()  __asm__("cpsie f	@ __stf" : : : "memory", "cc")
#define local_fiq_disable() __asm__("cpsid f	@ __clf" : : : "memory", "cc")
#define local_irq_enable() arch_local_irq_enable()
#define local_irq_disable() arch_local_irq_disable()
#endif /* __KERNEL__ */

#endif

