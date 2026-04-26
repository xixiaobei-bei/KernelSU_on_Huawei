/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARM timer implementation, found in Integrator, Versatile and Realview
 * platforms.  Not all platforms support all registers and bits in these
 * registers, so we mark them with A for Integrator AP, C for Integrator
 * CP, V for Versatile and R for Realview.
 *
 * Integrator AP has 16-bit timers, Integrator CP, Versatile and Realview
 * can have 16-bit or 32-bit selectable via a bit in the control register.
 *
 * Every SP804 contains two identical timers.
 */
#ifndef CONFIG_LPCPU_HITIMER
#define TIMER_1_BASE	0x00
#define TIMER_2_BASE	0x20

#define TIMER_LOAD	0x00			/* ACVR rw */
#define TIMER_VALUE	0x04			/* ACVR ro */
#define TIMER_CTRL	0x08			/* ACVR rw */
#define TIMER_CTRL_ONESHOT	(1 << 0)	/*  CVR */
#define TIMER_CTRL_32BIT	(1 << 1)	/*  CVR */
#define TIMER_CTRL_DIV1		(0 << 2)	/* ACVR */
#define TIMER_CTRL_DIV16	(1 << 2)	/* ACVR */
#define TIMER_CTRL_DIV256	(2 << 2)	/* ACVR */
#define TIMER_CTRL_IE		(1 << 5)	/*   VR */
#define TIMER_CTRL_PERIODIC	(1 << 6)	/* ACVR */
#define TIMER_CTRL_ENABLE	(1 << 7)	/* ACVR */

#define TIMER_INTCLR	0x0c			/* ACVR wo */
#define TIMER_RIS	0x10			/*  CVR ro */
#define TIMER_MIS	0x14			/*  CVR ro */
#define TIMER_BGLOAD	0x18			/*  CVR rw */
#else
#define TIMER_1_BASE	0x00
#define TIMER_2_BASE	0x400

#define TIMER_LOAD	0x04			/* ACVR rw */
#define TIMER_VALUE	0x14			/* ACVR ro */
#define TIMER_CTRL	0x08			/* ACVR rw */
#define TIMER_CTRL_ONESHOT	(1 << 2)	/*  CVR */
#define TIMER_CTRL_32BIT	(1 << 1)	/*  CVR */
#define TIMER_CTRL_DIV1		(0 << 2)	/* ACVR */
#define TIMER_CTRL_DIV16	(1 << 2)	/* ACVR */
#define TIMER_CTRL_DIV256	(2 << 2)	/* ACVR */
#define TIMER_CTRL_IE		(0 << 4)	/*   VR */
#define TIMER_CTRL_INT_MASK	(1 << 4)	/* INTR MASK */
#define TIMER_CTRL_PERIODIC	(1 << 3)	/* ACVR */
#define TIMER_CTRL_ENABLE	(1 << 0)	/* ACVR */

#define TIMER_INTCLR	0x10			/* ACVR wo */
#define TIMER_RIS	0x10			/*  CVR ro */
#define TIMER_MIS	0x14			/*  CVR ro */
#define TIMER_BGLOAD	0x18			/*  CVR rw */

#define LPCPU_CLK_CTRL		0x0
#define LPCPU_CLK_CTRL_VAL	(1 << 2)
#endif
