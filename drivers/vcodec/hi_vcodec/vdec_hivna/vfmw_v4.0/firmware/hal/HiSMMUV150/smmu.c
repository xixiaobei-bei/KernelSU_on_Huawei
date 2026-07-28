/*
 * vdec driver for smmu master
 *
 * Copyright (c) 2017 Hisilicon Limited
 *
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation.
 *
 */

#ifdef ENV_ARMLINUX_KERNEL
#include <asm/memory.h>
#include <linux/types.h>
#include <linux/gfp.h>
#endif

#include "smmu.h"
#include "smmu_regs.h"
#include "public.h"

#define SMRx_ID_SIZE            32
#define SMMU_RWERRADDR_SIZE     128

#define HIVDEC_SMMU_COMMON_OFFSET       (0x20000)
#define HIVDEC_SMMU_MASTER_OFFSET       (0xF000)

#define HIVDEC_SMMU_COMMON_BASE_ADDR (gVdhRegBaseAddr + HIVDEC_SMMU_COMMON_OFFSET)
#define HIVDEC_SMMU_MASTER_BASE_ADDR (gVdhRegBaseAddr + HIVDEC_SMMU_MASTER_OFFSET)

extern UINT32 gVdecQosMode;

//SMMU common and Master(MFDE/SCD/BPD) virtual base address
typedef struct {
	SINT32 *pSMMUCommonBaseVirAddr;
	SINT32 *pSMMUMasterBaseVirAddr;
	SINT32 *pSMMUMFDERegVirAddr;
	SINT32 *pSMMUBPDRegVirAddr;
	SINT32 *pSMMUSCDRegVirAddr;
} SMMU_REG_VIR_S;

SMMU_REG_VIR_S gSmmuRegVir;
MEM_DESC_S gAllocMem_RD;
MEM_DESC_S gAllocMem_WR;

SINT32 gSmmuInitFlag   = 0;
SINT32 gMfdeSecureFlag = 0;
SINT32 gMfdeSmmuFlag   = 1;
SINT32 gScdSecureFlag  = 0;
SINT32 gScdSmmuFlag    = 1;
SINT32 gBpdSecureFlag  = 0;
SINT32 gBpdSmmuFlag    = 0;

//smmu common regs r/w
#define RD_SMMU_COMMON_VREG( reg, dat )               \
do {                    \
	(dat) = readl(((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUCommonBaseVirAddr + (reg)))); \
} while(0)

#define WR_SMMU_COMMON_VREG( reg, dat )               \
do {                    \
    writel((dat), ((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUCommonBaseVirAddr + (reg)))); \
} while(0)
//smmu master regs r/w
#define RD_SMMU_MASTER_VREG( reg, dat )               \
do {                    \
	(dat) = readl(((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUMasterBaseVirAddr + (reg)))); \
} while(0)

#define WR_SMMU_MASTER_VREG( reg, dat )               \
do {                    \
	writel((dat), ((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUMasterBaseVirAddr + (reg)))); \
} while(0)

//mfde regs r/w
#define RD_SMMU_MFDE_VREG( reg, dat )               \
do {                    \
	(dat) = readl(((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUMFDERegVirAddr + (reg)))); \
} while(0)

#define WR_SMMU_MFDE_VREG( reg, dat )               \
do {                    \
    writel((dat), ((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUMFDERegVirAddr + (reg)))); \
} while(0)
//bpd regs r/w
#define RD_SMMU_BPD_VREG( reg, dat )               \
do {                    \
	(dat) = readl(((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUBPDRegVirAddr + (reg)))); \
} while(0)

#define WR_SMMU_BPD_VREG( reg, dat )               \
do {                    \
    writel((dat), ((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUBPDRegVirAddr + (reg)))); \
} while(0)
//scd regs r/w
#define RD_SMMU_SCD_VREG( reg, dat )               \
do {                    \
	(dat) = readl(((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUSCDRegVirAddr + (reg)))); \
} while(0)

#define WR_SMMU_SCD_VREG( reg, dat )               \
do {                    \
    writel((dat), ((volatile SINT32*)((SINT8*)gSmmuRegVir.pSMMUSCDRegVirAddr + (reg)))); \
} while(0)

/**
 *function: set SMMU common register
 *addr: register's vir addr
 *val: value to be set
 *bw: bit width
 *bs: bit start
 */
static VOID set_common_reg(UADDR addr, SINT32 val, SINT32 bw, SINT32 bs)
{
	SINT32 mask = (1UL << bw) - 1UL;
	SINT32 tmp = 0;

	RD_SMMU_COMMON_VREG(addr, tmp);
	tmp &= ~(mask << bs);/*lint !e502*/
	WR_SMMU_COMMON_VREG(addr, tmp | ((val & mask) << bs));
}

/**
 *function: set SMMU master register
 *addr: register's vir addr
 *val: value to be set
 *bw: bit width
 *bs: bit start
 */
static VOID set_master_reg(UADDR addr, SINT32 val, SINT32 bw, SINT32 bs)
{
	SINT32 mask = (1UL << bw) - 1UL;
	SINT32 tmp = 0;

	RD_SMMU_MASTER_VREG(addr, tmp);
	tmp &= ~(mask << bs);/*lint !e502*/
	WR_SMMU_MASTER_VREG(addr, (tmp | ((val & mask) << bs)));

}

/**
 *function: set mfde/scd/bpd register
 *master_type: MFDE/SCD/BPD
 *addr: register's vir addr
 *val: value to be set
 *bw: bit width
 *bs: bit start
 */
static VOID set_vdh_master_reg(SMMU_MASTER_TYPE master_type, UADDR addr, SINT32 val, SINT32 bw, SINT32 bs)
{
	SINT32 mask = (1UL << bw) - 1UL;
	SINT32 tmp = 0;

	switch (master_type) {
	case MFDE:
		RD_SMMU_MFDE_VREG(addr, tmp);
		tmp &= ~(mask << bs);/*lint !e502*/
		WR_SMMU_MFDE_VREG(addr, tmp | ((val & mask) << bs));
		break;

	case BPD:
		RD_SMMU_BPD_VREG(addr, tmp);
		tmp &= ~(mask << bs);/*lint !e502*/
		WR_SMMU_BPD_VREG(addr, tmp | ((val & mask) << bs));
		break;

	case SCD:
		RD_SMMU_SCD_VREG(addr, tmp);
		tmp &= ~(mask << bs);/*lint !e502*/
		WR_SMMU_SCD_VREG(addr, tmp | ((val & mask) << bs));
		break;

	default:
		break;
	}
}

/**
 *function: set mfde mmu cfg register
 */
static VOID set_mmu_cfg_reg_mfde(SMMU_MASTER_TYPE master_type, UINT32 secure_en, UINT32 mmu_en)
{
	if (mmu_en) {    //MMU enable
		set_vdh_master_reg(master_type, REG_MFDE_MMU_CFG_EN, 0x1, 1, 12);    //[12]mmu_en=1
		if (secure_en) {  //secure
			dprint(PRN_ALWS, "IN %s not support this mode: mmu_en:secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			set_vdh_master_reg(master_type, REG_MFDE_MMU_CFG_SECURE, 0x1, 1, 31);    //[31]secure_en=1
		} else { //non-secure
			set_vdh_master_reg(master_type, REG_MFDE_MMU_CFG_SECURE, 0x0, 1, 31);    //[31]secure_en=0
		}
	} else {    //MMU disable
		set_vdh_master_reg(master_type, REG_MFDE_MMU_CFG_EN, 0x0, 1, 12);    //[12]mmu_en=0
		if (secure_en) {    //secure
			set_vdh_master_reg(master_type, REG_MFDE_MMU_CFG_SECURE, 0x1, 1, 31);    //[31]secure_en=1
		} else {    //non-secure
			dprint(PRN_ALWS, "IN %s not support this mode: non_mmu:non_secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			set_vdh_master_reg(master_type, REG_MFDE_MMU_CFG_SECURE, 0x0, 1, 31);	//[31]secure_en=0
		}
	}
}

/**
 *function: set bpd mmu cfg register
 */
static VOID set_mmu_cfg_reg_bpd(SMMU_MASTER_TYPE master_type, UINT32 secure_en, UINT32 mmu_en)
{
	if (mmu_en) {    //MMU enable
		set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x1, 1, 21);    //[21]mmu_en=1
		if (secure_en) {    //secure
			dprint(PRN_ALWS, "IN %s not support this mode: mmu_en:secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x1, 1, 20);    //[20]secure_en=1
		} else {    //non-secure
			set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x0, 1, 20);    //[20]secure_en=0
		}
	} else {    //MMU disable
		set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x0, 1, 21);    //[21]mmu_en=0
		if (secure_en) {    //secure
			set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x1, 1, 20);    //[20]secure_en=1
		} else {    //non-secure
			dprint(PRN_ALWS, "IN %s not support this mode: non_mmu:non_secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x0, 1, 20);	//[20]secure_en=0
		}
	}
}

/**
 *function: set scd mmu cfg register
 */
static VOID set_mmu_cfg_reg_scd(SMMU_MASTER_TYPE master_type, UINT32 secure_en, UINT32 mmu_en)
{
	if (mmu_en) {   //MMU enable
		set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x1, 1, 9);//[9]mmu_en=1
		set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x1, 1, 13);//[13]rdbuf_mmu_en=1
		if (secure_en) {    //secure
			dprint(PRN_ALWS, "IN %s not support this mode: mmu_en:secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x1, 1, 7);    //[7]secure_en=1
		} else {    //non-secure
			set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x0, 1, 7);    //[7]secure_en=0
		}
	} else {    //MMU disable
		set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x0, 1, 9);    //[9]mmu_en=0
		if (secure_en) {    //secure
			set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x1, 1, 7);    //[7]secure_en=1
		} else {    //non-secure
			dprint(PRN_ALWS, "IN %s not support this mode: non_mmu:non_secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x0, 1, 7);	//[7]secure_en=0
		}
	}
}

static SINT32 smmu_mem_alloc(UINT32 size, MEM_DESC_S *pMemDesc)
{
	HI_VOID *virt_addr = HI_NULL;

	if (pMemDesc == NULL) {
		dprint(PRN_ERROR, "%s: invalid param pMemDesc is NULL\n", __func__);
		return SMMU_ERR;
	}

	if (pMemDesc->VirAddr != HI_NULL) {
		dprint(PRN_ERROR, "%s param StartVirAddr %pK is not NULL\n", __func__, (VOID *)pMemDesc->VirAddr);
		return SMMU_ERR;
	}

	if (MEM_CMA_ZERO == pMemDesc->MemType)
		virt_addr = kzalloc(size, GFP_KERNEL | GFP_DMA);    //restrict [0 ~ 4G]
	else
		virt_addr = kmalloc(size, GFP_KERNEL | GFP_DMA);    //restrict [0 ~ 4G]

	if (!virt_addr) {
		dprint(PRN_FATAL, "%s Alloc virt_addr failed\n", __func__);
		return SMMU_ERR;
	}

	pMemDesc->VirAddr = (UINT64)virt_addr;
	pMemDesc->PhyAddr = __pa(virt_addr);/*lint !e648*/

	return SMMU_OK;
}

static VOID smmu_mem_dealloc(MEM_DESC_S *pMemDesc)
{
	if (pMemDesc == HI_NULL) {
		dprint(PRN_ERROR, "%s : Invalid pMemDesc is NULL\n", __func__);
		return;
	}

	if (pMemDesc->VirAddr == HI_NULL) {
		dprint(PRN_ERROR, "%s : Invalid pMemDesc->VirAddr is NULL\n", __func__);
		return;
	}

	kfree((HI_VOID *)pMemDesc->VirAddr);
	pMemDesc->VirAddr = HI_NULL;
	return;
}

/**
 *function: Alloc MEM for TLB miss .
 */
#ifdef ENV_ARMLINUX_KERNEL
static SINT32 alloc_smmu_tlb_miss_addr(VOID)
{
	SINT32 ret = SMMU_ERR;

	gAllocMem_RD.MemType = MEM_CMA_ZERO;
	ret = smmu_mem_alloc(SMMU_RWERRADDR_SIZE, &gAllocMem_RD);
	if (ret != MEM_MAN_OK) {
		dprint(PRN_FATAL, "%s kzalloc mem for smmu rderr failed\n", __func__);
		return SMMU_ERR;
	}

	gAllocMem_WR.MemType = MEM_CMA_ZERO;
	ret = smmu_mem_alloc(SMMU_RWERRADDR_SIZE, &gAllocMem_WR);
	if (ret != MEM_MAN_OK) {
		dprint(PRN_FATAL, "%s kzalloc mem for smmu wrerr failed\n", __func__);
		smmu_mem_dealloc(&gAllocMem_RD);
		return SMMU_ERR;
	}

	return SMMU_OK;
}
#endif

VOID SMMU_ConfigSMR(VOID)
{
	UINT32 i = 0;
	for(i = 0; i < SMRx_ID_SIZE; i++)
	{
		set_common_reg(SMMU_SMRx_P + i * 0x4, 0x0, 32, 0); //smmu_smrx_p.smr_prtectec_en=1
	}
}

/**
 *function: init SMMU global registers.
 */
VOID SMMU_InitGlobalReg(VOID)
{
	UINT32 i = 0;

	if (gSmmuInitFlag != 1) {
		dprint(PRN_DBG, "%s Smmu initialization failed\n", __func__);
		return;
	}
	//0000 0000 0000 1111 0000 0000 0011 1000 --> 0x000f0038
	set_common_reg(SMMU_SCR, 0x0, 1, 0);//SMMU_SCR[0].glb_bypass
	set_common_reg(SMMU_SCR, 0x3, 2, 1);//SMMU_SCR[1].rqos_en SMMU_SCR[2].wqos_en

	if (gVdecQosMode == 1) {
		set_common_reg(SMMU_SCR, 0x2, 4, 6);
		set_common_reg(SMMU_SCR, 0x2, 4, 10);
	}

	//SMRX_S had set default value. Only need to set SMMU_SMRx_NS secure SID  bypass
	//SMMU_SMRx[0]smr_bypass=0(non-bypass); SMMU_SMRx[2:3]smr_ptw_qos=0x3;
	for (i = 0; i < SMRx_ID_SIZE; i += 2) {
		set_common_reg(SMMU_SMRx_NS + i * 0x4, 0x1C, 32, 0);//0x00000003 none secure
	}

	for (i = 1; i < SMRx_ID_SIZE; i += 2) {
		set_common_reg(SMMU_SMRx_NS + i * 0x4, 0x1D, 32, 0);//0x00000002 secure
	}
	set_common_reg(SMMU_CB_TTBR0, gSmmuPageBase, 32, 0);
	set_common_reg(SMMU_FAMA_CTRL1_NS, (gSmmuPageBase>>32)&0x7F, 32, 0);
	set_common_reg(SMMU_CB_TTBCR, 0x1, 1, 0);

	if (gAllocMem_RD.PhyAddr != 0 && gAllocMem_WR.PhyAddr != 0) {
		set_common_reg(SMMU_ERR_RDADDR, (gAllocMem_RD.PhyAddr & 0xFFFFFFFF), 32, 0);
		set_common_reg(SMMU_ADDR_MSB, (gAllocMem_RD.PhyAddr>>32)&0x7F, 7, 0);

		set_common_reg(SMMU_ERR_WRADDR, (gAllocMem_WR.PhyAddr & 0xFFFFFFFF), 32, 0);
		set_common_reg(SMMU_ADDR_MSB, (gAllocMem_WR.PhyAddr>>32)&0x7F, 7, 7);
	}
	//glb_bypass, 0x0: normal mode, 0x1: bypass mode
	set_master_reg(SMMU_MSTR_GLB_BYPASS, 0x0, 32, 0);    //master mmu enable
}

/**
 *function: set MFDE/SCD/BPD mmu cfg register, MMU or secure.
 */
VOID SMMU_SetMasterReg(SMMU_MASTER_TYPE master_type, UINT8 secure_en, UINT8 mmu_en)
{
	switch (master_type) {
	case MFDE:
		set_mmu_cfg_reg_mfde(master_type, secure_en, mmu_en);
		gMfdeSecureFlag = secure_en;
		gMfdeSmmuFlag = mmu_en;
		break;

	case SCD:
		set_mmu_cfg_reg_scd(master_type, secure_en, mmu_en);
		gScdSecureFlag = secure_en;
		gScdSmmuFlag = mmu_en;
		break;

	case BPD:
		set_mmu_cfg_reg_bpd(master_type, secure_en, mmu_en);
		gBpdSecureFlag = secure_en;
		gBpdSmmuFlag = mmu_en;
		break;

	default:
		dprint(PRN_FATAL, "%s unkown master type %d\n", __func__, master_type);
		break;
	}
}

#ifdef PLATFORM_HIVCODECV200
VOID SMMU_SetMemCtlReg(VOID)
{
	UINT32 D32  = 0x02605550;
	WR_SMMU_MASTER_VREG(SMMU_MSTR_MEM_CTRL, D32);
}
#endif

VOID SMMU_IntServProc(VOID)
{
	SINT32 tmp = -1;
	dprint(PRN_ALWS, "%s enter", __func__);
	RD_SMMU_COMMON_VREG(SMMU_INTSTAT_NS, tmp);
	dprint(PRN_ALWS, "SMMU_INTSTAT_NS : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_0, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_0 : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_1, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_1 : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_2, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_2 : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_3, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_3 : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_4, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_4 : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_5, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_5 : 0x%x\n", tmp);
	RD_SMMU_MASTER_VREG(SMMU_MSTR_DBG_6, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_6 : 0x%x\n", tmp);
	dprint(PRN_ALWS, "%s end", __func__);
}

/**
 *function: get registers virtual address, and alloc mem for TLB miss.
 */
SINT32 SMMU_Init(VOID)
{
	SINT32 ret = SMMU_ERR;
	memset(&gSmmuRegVir, 0, sizeof(gSmmuRegVir));

	gSmmuRegVir.pSMMUMFDERegVirAddr = (SINT32 *) MEM_Phy2Vir(gVdhRegBaseAddr);
	if (gSmmuRegVir.pSMMUMFDERegVirAddr == NULL) {
		dprint(PRN_FATAL, "%s pSMMUMFDERegVirAddr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	gSmmuRegVir.pSMMUSCDRegVirAddr = (SINT32 *) MEM_Phy2Vir(gScdRegBaseAddr);
	if (gSmmuRegVir.pSMMUSCDRegVirAddr == NULL) {
		dprint(PRN_FATAL, "%s pSMMUSCDRegVirAddr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	gSmmuRegVir.pSMMUBPDRegVirAddr = (SINT32 *) MEM_Phy2Vir(gBpdRegBaseAddr);
	if (gSmmuRegVir.pSMMUBPDRegVirAddr == NULL) {
		dprint(PRN_FATAL, "%s pSMMUBPDRegVirAddr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	gSmmuRegVir.pSMMUCommonBaseVirAddr = (SINT32 *) MEM_Phy2Vir(HIVDEC_SMMU_COMMON_BASE_ADDR);
	if (gSmmuRegVir.pSMMUCommonBaseVirAddr == NULL) {
		dprint(PRN_FATAL, "%s pSMMUCommonBaseVirAddr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	gSmmuRegVir.pSMMUMasterBaseVirAddr = (SINT32 *) MEM_Phy2Vir(HIVDEC_SMMU_MASTER_BASE_ADDR);
	if (gSmmuRegVir.pSMMUMasterBaseVirAddr == NULL) {
		dprint(PRN_FATAL, "%s pSMMUMasterBaseVirAddr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	memset(&gAllocMem_RD, 0, sizeof(gAllocMem_RD));
	memset(&gAllocMem_WR, 0, sizeof(gAllocMem_WR));

#ifdef ENV_ARMLINUX_KERNEL
	ret = alloc_smmu_tlb_miss_addr();
	if (ret != SMMU_OK) {
		dprint(PRN_FATAL, "%s alloc_smmu_tlb_miss_addr failed\n", __func__);
		return SMMU_ERR;
	}
#endif

	gSmmuInitFlag = 1;

	return SMMU_OK;
}

/**
 *function: free mem of SMMU_ERR_RDADDR and SMMU_ERR_WRADDR.
 */
VOID SMMU_DeInit(VOID)
{
	if (gAllocMem_RD.PhyAddr != 0)
		smmu_mem_dealloc(&gAllocMem_RD);

	if (gAllocMem_WR.PhyAddr != 0)
		smmu_mem_dealloc(&gAllocMem_WR);
}
