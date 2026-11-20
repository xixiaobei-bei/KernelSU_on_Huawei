/**********************************************************************
 * copyright (C) 2014 Cadence Design Systems
 * All rights reserved.
 ***********************************************************************
 * cps_bm.c
 *
 * Sample implementation of Cadence Platform Services for a bare-metal
 * system
 ***********************************************************************/

#include <linux/string.h>
#include <linux/io.h>
#include "cps.h"
/*lint -e578 -e144*/

/* see cps.h */
uint8_t CPS_UncachedRead8(volatile uint8_t* address) {
	return readb(address);
}

/* see cps.h */
uint16_t CPS_UncachedRead16(volatile uint16_t* address) {
	return readw(address);
}

/* see cps.h */
uint32_t CPS_UncachedRead32(volatile uint32_t* address) {
	return readl(address);
}

/* see cps.h */
void CPS_UncachedWrite8(volatile uint8_t* address, uint8_t value) {
	writeb(value, address);
}

/* see cps.h */
void CPS_UncachedWrite16(volatile uint16_t* address, uint16_t value) {
	writew(value, address);
}

/* see cps.h */
void CPS_UncachedWrite32(volatile uint32_t* address, uint32_t value) {
	writel(value, address);
}

/* see cps.h */
void CPS_WritePhysAddress32(volatile uint32_t* location, uint32_t addrValue) {
	writel(addrValue, location);
}

/* see cps.h */
void CPS_BufferCopy(volatile uint8_t *dst, volatile uint8_t *src, uint32_t size) {
	memcpy((void*)dst, (void*)src, size);
}

