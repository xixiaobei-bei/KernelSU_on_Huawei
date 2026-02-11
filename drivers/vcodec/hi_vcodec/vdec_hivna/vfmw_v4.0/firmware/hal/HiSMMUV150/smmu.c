

#include "smmu.h"

#ifdef ENV_ARMLINUX_KERNEL
#include <asm/memory.h>
#include <linux/types.h>
#include <linux/gfp.h>
#endif

#include "smmu_regs.h"
#include "public.h"

#define SMRX_ID_SIZE            32
#define SMMU_RWERRADDR_SIZE     128

#define HIVDEC_SMMU_COMMON_OFFSET       0x20000
#define HIVDEC_SMMU_MASTER_OFFSET       0xF000

#define HIVDEC_SMMU_COMMON_BASE_ADDR \
	(g_vdh_reg_base_addr + HIVDEC_SMMU_COMMON_OFFSET)
#define HIVDEC_SMMU_MASTER_BASE_ADDR \
	(g_vdh_reg_base_addr + HIVDEC_SMMU_MASTER_OFFSET)

extern UINT32 g_vdec_qos_mode;

// SMMU common and Master(MFDE/SCD/BPD) virtual base address
typedef struct {
	SINT32 *p_smmu_common_base_vir_addr;
	SINT32 *p_smmu_master_base_vir_addr;
	SINT32 *p_smmu_mfde_reg_vir_addr;
	SINT32 *p_smmu_bpd_reg_vir_addr;
	SINT32 *p_smmu_scd_reg_vir_addr;
} smmu_reg_vir_s;

smmu_reg_vir_s g_smmu_reg_vir;
mem_desc_s g_alloc_mem_rd;
mem_desc_s g_alloc_mem_wr;

SINT32 g_smmu_init_flag;
SINT32 g_mfde_secure_flag;
SINT32 g_mfde_smmu_flag   = 1;
SINT32 g_scd_secure_flag;
SINT32 g_scd_smmu_flag    = 1;
SINT32 g_bpd_secure_flag;
SINT32 g_bpd_smmu_flag;

// smmu common regs r/w
#define rd_smmu_common_vreg(reg, dat) \
do \
	(dat) = readl(((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_common_base_vir_addr + (reg)))); \
while (0)

#define wr_smmu_common_vreg(reg, dat) \
do \
	writel((dat), ((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_common_base_vir_addr + (reg)))); \
while (0)
// smmu master regs r/w
#define rd_smmu_master_vreg(reg, dat) \
do \
	(dat) = readl(((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_master_base_vir_addr + (reg)))); \
while (0)

#define wr_smmu_master_vreg(reg, dat) \
do \
	writel((dat), ((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_master_base_vir_addr + (reg)))); \
while (0)

// mfde regs r/w
#define rd_smmu_mfde_vreg(reg, dat) \
do \
	(dat) = readl(((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_mfde_reg_vir_addr + (reg)))); \
while (0)

#define wr_smmu_mfde_vreg(reg, dat) \
do \
	writel((dat), ((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_mfde_reg_vir_addr + (reg)))); \
while (0)
// bpd regs r/w
#define rd_smmu_bpd_vreg(reg, dat) \
do \
	(dat) = readl(((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_bpd_reg_vir_addr + (reg)))); \
while (0)

#define wr_smmu_bpd_vreg(reg, dat) \
do \
	writel((dat), ((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_bpd_reg_vir_addr + (reg)))); \
while (0)
// scd regs r/w
#define rd_smmu_scd_vred(reg, dat) \
do \
	(dat) = readl(((volatile SINT32 *) \
	((SINT8 *)g_smmu_reg_vir.p_smmu_scd_reg_vir_addr + (reg)))); \
while (0)

#define wr_smmu_scd_vreg(reg, dat) \
do \
	writel((dat), ((volatile SINT32 *)( \
	(SINT8 *)g_smmu_reg_vir.p_smmu_scd_reg_vir_addr + (reg)))); \
while (0)

/*
  * function: set SMMU common register
  * addr: register's vir addr
  * val: value to be set
  * bw: bit width
  * bs: bit start
  */
static VOID set_common_reg(UADDR addr, SINT32 val, SINT32 bw, SINT32 bs)
{
	SINT32 mask = (1UL << bw) - 1UL;
	SINT32 tmp = 0;

	rd_smmu_common_vreg(addr, tmp);
	tmp &= ~(mask << bs); /* lint !e502 */
	wr_smmu_common_vreg(addr, tmp | ((val & mask) << bs));
}

/*
  * function: set SMMU master register
  * addr: register's vir addr
  * val: value to be set
  * bw: bit width
  * bs: bit start
  */
static VOID set_master_reg(UADDR addr, SINT32 val, SINT32 bw, SINT32 bs)
{
	SINT32 mask = (1UL << bw) - 1UL;
	SINT32 tmp = 0;

	rd_smmu_master_vreg(addr, tmp);
	tmp &= ~(mask << bs); /* lint !e502 */
	wr_smmu_master_vreg(addr, (tmp | ((val & mask) << bs)));

}

/*
  * function: set mfde/scd/bpd register
  * master_type: MFDE/SCD/BPD
  * addr: register's vir addr
  * val: value to be set
  * bw: bit width
  * bs: bit start
  */
static VOID set_vdh_master_reg(
	smmu_master_type master_type, UADDR addr,
	SINT32 val, SINT32 bw, SINT32 bs)
{
	SINT32 mask = (1UL << bw) - 1UL;
	SINT32 tmp = 0;

	switch (master_type) {
	case MFDE:
		rd_smmu_mfde_vreg(addr, tmp);
		tmp &= ~(mask << bs); /* lint !e502 */
		wr_smmu_mfde_vreg(addr, tmp | ((val & mask) << bs));
		break;

	case BPD:
		rd_smmu_bpd_vreg(addr, tmp);
		tmp &= ~(mask << bs); /* lint !e502 */
		wr_smmu_bpd_vreg(addr, tmp | ((val & mask) << bs));
		break;

	case SCD:
		rd_smmu_scd_vred(addr, tmp);
		tmp &= ~(mask << bs); /* lint !e502 */
		wr_smmu_scd_vreg(addr, tmp | ((val & mask) << bs));
		break;
	default:
		break;
	}
}

/*
  * function: set mfde mmu cfg register
  */
static VOID set_mmu_cfg_reg_mfde(
	smmu_master_type master_type, UINT32 secure_en,
	UINT32 mmu_en)
{
	if (mmu_en) {    // MMU enable
		// [12]mmu_en=1
		set_vdh_master_reg(master_type,
			REG_MFDE_MMU_CFG_EN, 0x1, 1, 12);
		if (secure_en) {  // secure
			dprint(PRN_ALWS, "IN %s not support this mode: mmu_en:secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
		    // [31]secure_en=1
			set_vdh_master_reg(master_type,
				REG_MFDE_MMU_CFG_SECURE, 0x1, 1, 31);
		} else { // non-secure
		    // [31]secure_en=0
			set_vdh_master_reg(master_type,
				REG_MFDE_MMU_CFG_SECURE, 0x0, 1, 31);
		}
	} else {    // MMU disable
	    // [12]mmu_en=0
		set_vdh_master_reg(master_type,
			REG_MFDE_MMU_CFG_EN, 0x0, 1, 12);
		if (secure_en) {    // secure
		    // [31]secure_en=1
			set_vdh_master_reg(master_type,
				REG_MFDE_MMU_CFG_SECURE, 0x1, 1, 31);
		} else {    // non-secure
			dprint(PRN_ALWS, "IN %s not support this mode: non_mmu:non_secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			// [31]secure_en=0
			set_vdh_master_reg(master_type,
				REG_MFDE_MMU_CFG_SECURE, 0x0, 1, 31);
		}
	}
}

/*
  * function: set bpd mmu cfg register
  */
static VOID set_mmu_cfg_reg_bpd(
	smmu_master_type master_type, UINT32 secure_en,
	UINT32 mmu_en)
{
	if (mmu_en) {    // MMU enable
	    // [21]mmu_en=1
		set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x1, 1, 21);
		if (secure_en) {    // secure
			dprint(PRN_ALWS, "IN %s not support this mode: mmu_en:secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			// [20]secure_en=1
			set_vdh_master_reg(master_type,
				REG_BPD_MMU_CFG, 0x1, 1, 20);
		} else {    // non-secure
		    // [20]secure_en=0
			set_vdh_master_reg(master_type,
				REG_BPD_MMU_CFG, 0x0, 1, 20);
		}
	} else {    // MMU disable
	    // [21]mmu_en=0
		set_vdh_master_reg(master_type, REG_BPD_MMU_CFG, 0x0, 1, 21);
		if (secure_en) {    // secure
		    // [20]secure_en=1
			set_vdh_master_reg(master_type,
				REG_BPD_MMU_CFG, 0x1, VDH_MASTER_BW, VDH_MASTER_BS);
		} else {    // non-secure
			dprint(PRN_ALWS, "IN %s not support this mode: non_mmu:non_secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			// [20]secure_en=0
			set_vdh_master_reg(master_type,
				REG_BPD_MMU_CFG, 0x0, VDH_MASTER_BW, VDH_MASTER_BS);
		}
	}
}

/*
  * function: set scd mmu cfg register
  */
static VOID set_mmu_cfg_reg_scd(
	smmu_master_type master_type, UINT32 secure_en, UINT32 mmu_en)
{
	if (mmu_en) {   // MMU enable
	    // [9]mmu_en=1
		set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x1, 1, 9);
		// [13]rdbuf_mmu_en=1
		set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x1, 1, 13);
		if (secure_en) {    // secure
			dprint(PRN_ALWS, "IN %s not support this mode: mmu_en:secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			// [7]secure_en=1
			set_vdh_master_reg(master_type,
				REG_SCD_MMU_CFG, 0x1, 1, 7);
		} else {    // non-secure //[7]secure_en=0
			set_vdh_master_reg(master_type,
				REG_SCD_MMU_CFG, 0x0, 1, 7);
		}
	} else {    // MMU disable  [9]mmu_en=0
		set_vdh_master_reg(master_type, REG_SCD_MMU_CFG, 0x0, 1, 9);
		if (secure_en) {    // secure
			// [7]secure_en=1
			set_vdh_master_reg(master_type,
				REG_SCD_MMU_CFG, 0x1, 1, 7);
		} else {    // non-secure
			dprint(PRN_ALWS, "IN %s not support this mode: non_mmu:non_secure\n", __func__);
			dprint(PRN_ALWS, "%s, secure_en:%d, mmu_en:%d\n", __func__, secure_en, mmu_en);
			// [7]secure_en=0
			set_vdh_master_reg(master_type,
				REG_SCD_MMU_CFG, 0x0, 1, 7);
		}
	}
}

static SINT32 smmu_mem_alloc(UINT32 size, mem_desc_s *p_mem_desc)
{
	hi_void *virt_addr = HI_NULL;

	if ((p_mem_desc == NULL) || (size == 0)) {
		dprint(PRN_ERROR, "%s: p_mem_desc is NULL or size is 0\n", __func__);
		return SMMU_ERR;
	}

	if (p_mem_desc->vir_addr != HI_NULL) {
		dprint(PRN_ERROR, "%s param StartVirAddr %pK is not NULL\n", __func__, (VOID *)p_mem_desc->vir_addr);
		return SMMU_ERR;
	}

	if (p_mem_desc->mem_type == MEM_CMA_ZERO)
		// restrict [0 ~ 4G]
		virt_addr = kzalloc(size, GFP_KERNEL | GFP_DMA);
	else
		// restrict [0 ~ 4G]
		virt_addr = kmalloc(size, GFP_KERNEL | GFP_DMA);

	if (!virt_addr) {
		dprint(PRN_FATAL, "%s Alloc virt_addr failed\n", __func__);
		return SMMU_ERR;
	}

	p_mem_desc->vir_addr = (UINT64)(uintptr_t)virt_addr;
	p_mem_desc->phy_addr = __pa((uintptr_t)virt_addr); /* lint !e648 */

	return SMMU_OK;
}

static VOID smmu_mem_dealloc(mem_desc_s *p_mem_desc)
{
	if (p_mem_desc == HI_NULL) {
		dprint(PRN_ERROR, "%s : Invalid p_mem_desc is NULL\n", __func__);
		return;
	}

	if (p_mem_desc->vir_addr == HI_NULL) {
		dprint(PRN_ERROR, "%s : Invalid p_mem_desc->vir_addr is NULL\n", __func__);
		return;
	}

	kfree((hi_void *)(uintptr_t)p_mem_desc->vir_addr);
	p_mem_desc->vir_addr = HI_NULL;
}

/*
  * function: Alloc MEM for TLB miss .
  */
#ifdef ENV_ARMLINUX_KERNEL
static SINT32 alloc_smmu_tlb_miss_addr(void)
{
	SINT32 ret;

	g_alloc_mem_rd.mem_type = MEM_CMA_ZERO;
	ret = smmu_mem_alloc(SMMU_RWERRADDR_SIZE, &g_alloc_mem_rd);
	if (ret != MEM_MAN_OK) {
		dprint(PRN_FATAL, "%s kzalloc mem for smmu rderr failed\n", __func__);
		return SMMU_ERR;
	}

	g_alloc_mem_wr.mem_type = MEM_CMA_ZERO;
	ret = smmu_mem_alloc(SMMU_RWERRADDR_SIZE, &g_alloc_mem_wr);
	if (ret != MEM_MAN_OK) {
		dprint(PRN_FATAL, "%s kzalloc mem for smmu wrerr failed\n", __func__);
		smmu_mem_dealloc(&g_alloc_mem_rd);
		return SMMU_ERR;
	}

	return SMMU_OK;
}
#endif

/*
  * function: init SMMU global registers.
  */
VOID smmu_init_global_reg(void)
{
	UINT32 i = 0;

	if (g_smmu_init_flag != 1) {
		dprint(PRN_DBG, "%s Smmu initialization failed\n", __func__);
		return;
	}
	// 0000 0000 0000 1111 0000 0000 0011 1000 --> 0x000f0038
	// SMMU_SCR[0].glb_bypass
	set_common_reg(SMMU_SCR, 0x0, 1, 0);
	// SMMU_SCR[1].rqos_en SMMU_SCR[2].wqos_en
	set_common_reg(SMMU_SCR, 0x3, 2, 1);

	if (g_vdec_qos_mode == 1) {
		set_common_reg(SMMU_SCR, 0x2, 4, 6);
		set_common_reg(SMMU_SCR, 0x2, 4, 10);
	}

	// SMRX_S had set default value. Only need to
	// set SMMU_SMRX_NS secure SID  bypass
	for (i = 0; i < SMRX_ID_SIZE; i += 2)
		set_common_reg(SMMU_SMRX_NS + i * 0x4, 0x1C, 32, 0);

	for (i = 1; i < SMRX_ID_SIZE; i += 2)
		set_common_reg(SMMU_SMRX_NS + i * 0x4, 0x1D, 32, 0);
	set_common_reg(SMMU_CB_TTBR0, g_smmu_page_base, 32, 0);

#ifdef HISMMUV170
	set_common_reg(SMMU_CB_TTBR_MSB, (g_smmu_page_base >> 32) & 0xFFFF, 16, 0);
#else
	set_common_reg(SMMU_FAMA_CTRL1_NS, (g_smmu_page_base >> 32) & 0x7F, 7, 0);
#endif

	set_common_reg(SMMU_CB_TTBCR, 0x1, 1, 0);

	if (g_alloc_mem_rd.phy_addr != 0 && g_alloc_mem_wr.phy_addr != 0) {
#ifdef HISMMUV170
		set_common_reg(SMMU_ERR_RDADDR_NS,
			(g_alloc_mem_rd.phy_addr & 0xFFFFFFFF), 32, 0);
		set_common_reg(SMMU_ERR_WRADDR_NS,
			(g_alloc_mem_wr.phy_addr & 0xFFFFFFFF), 32, 0);

		set_common_reg(SMMU_ERR_ADDR_MSB_NS,
			(g_alloc_mem_rd.phy_addr >> 32) & 0xFFFF, 16, 0);
		set_common_reg(SMMU_ERR_ADDR_MSB_NS,
			(g_alloc_mem_wr.phy_addr >> 32) & 0xFFFF, 16, 16);
#else
		set_common_reg(SMMU_ERR_RDADDR,
			(g_alloc_mem_rd.phy_addr & 0xFFFFFFFF), 32, 0);
		set_common_reg(SMMU_ERR_WRADDR,
			(g_alloc_mem_wr.phy_addr & 0xFFFFFFFF), 32, 0);

		set_common_reg(SMMU_ADDR_MSB,
			(g_alloc_mem_rd.phy_addr >> 32) & 0x7F, 7, 0);
		set_common_reg(SMMU_ADDR_MSB,
			(g_alloc_mem_wr.phy_addr >> 32) & 0x7F, 7, 7);
#endif
	}
	// glb_bypass, 0x0: normal mode, 0x1: bypass mode
	set_master_reg(SMMU_MSTR_GLB_BYPASS, 0x0, 32, 0);    // master mmu enable
}

/*
  * function: set MFDE/SCD/BPD mmu cfg register, MMU or secure.
  */
VOID smmu_set_master_reg(
	smmu_master_type master_type, UINT8 secure_en,
	UINT8 mmu_en)
{
	switch (master_type) {
	case MFDE:
		set_mmu_cfg_reg_mfde(master_type, secure_en, mmu_en);
		g_mfde_secure_flag = secure_en;
		g_mfde_smmu_flag = mmu_en;
		break;

	case SCD:
		set_mmu_cfg_reg_scd(master_type, secure_en, mmu_en);
		g_scd_secure_flag = secure_en;
		g_scd_smmu_flag = mmu_en;
		break;

	case BPD:
		set_mmu_cfg_reg_bpd(master_type, secure_en, mmu_en);
		g_bpd_secure_flag = secure_en;
		g_bpd_smmu_flag = mmu_en;
		break;

	default:
		dprint(PRN_FATAL, "%s unkown master type %d\n", __func__, master_type);
		break;
	}
}

#ifdef PLATFORM_HIVCODECV200
VOID smmu_set_mem_ctl_reg(void)
{
	const UINT32 d_32  = 0x02605550;

	wr_smmu_master_vreg(SMMU_MSTR_MEM_CTRL, d_32);
}
#endif

VOID smmu_int_serv_proc(void)
{
	SINT32 tmp = -1;

	dprint(PRN_ALWS, "%s enter", __func__);
	rd_smmu_common_vreg(SMMU_INTSTAT_NS, tmp);
	dprint(PRN_ALWS, "SMMU_INTSTAT_NS : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_0, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_0 : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_1, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_1 : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_2, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_2 : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_3, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_3 : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_4, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_4 : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_5, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_5 : 0x%x\n", tmp);
	rd_smmu_master_vreg(SMMU_MSTR_DBG_6, tmp);
	dprint(PRN_ALWS, "SMMU_MSTR_DBG_6 : 0x%x\n", tmp);
	dprint(PRN_ALWS, "%s end", __func__);
}

/*
  * function: get registers virtual address, and alloc mem for TLB miss.
  */
SINT32 smmu_init(void)
{
	SINT32 ret;

	ret = memset_s(&g_smmu_reg_vir, sizeof(g_smmu_reg_vir), 0,
		sizeof(g_smmu_reg_vir));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return SMMU_ERR;
	}

	g_smmu_reg_vir.p_smmu_mfde_reg_vir_addr =
		(SINT32 *) mem_phy_2_vir(g_vdh_reg_base_addr);
	if (g_smmu_reg_vir.p_smmu_mfde_reg_vir_addr == NULL) {
		dprint(PRN_FATAL, "%s p_smmu_mfde_reg_vir_addr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	g_smmu_reg_vir.p_smmu_scd_reg_vir_addr =
		(SINT32 *) mem_phy_2_vir(g_scd_reg_base_addr);
	if (g_smmu_reg_vir.p_smmu_scd_reg_vir_addr == NULL) {
		dprint(PRN_FATAL, "%s p_smmu_scd_reg_vir_addr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	g_smmu_reg_vir.p_smmu_bpd_reg_vir_addr =
		(SINT32 *) mem_phy_2_vir(g_bpd_reg_base_addr);
	if (g_smmu_reg_vir.p_smmu_bpd_reg_vir_addr == NULL) {
		dprint(PRN_FATAL, "%s p_smmu_bpd_reg_vir_addr is NULL, SMMU Init failed\n", __func__);
		return SMMU_ERR;
	}

	g_smmu_reg_vir.p_smmu_common_base_vir_addr =
		(SINT32 *) mem_phy_2_vir(HIVDEC_SMMU_COMMON_BASE_ADDR);
	if (g_smmu_reg_vir.p_smmu_common_base_vir_addr == NULL) {
		dprint(PRN_FATAL, "%s p_smmu_common_base_vir_addr is NULL, SMMU Init failed\n",
			__func__);
		return SMMU_ERR;
	}

	g_smmu_reg_vir.p_smmu_master_base_vir_addr =
		(SINT32 *) mem_phy_2_vir(HIVDEC_SMMU_MASTER_BASE_ADDR);
	if (g_smmu_reg_vir.p_smmu_master_base_vir_addr == NULL) {
		dprint(PRN_FATAL, "%s p_smmu_master_base_vir_addr is NULL, SMMU Init failed\n",
			__func__);
		return SMMU_ERR;
	}

	ret = memset_s(&g_alloc_mem_rd, sizeof(g_alloc_mem_rd), 0,
		sizeof(g_alloc_mem_rd));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return SMMU_ERR;
	}
	ret = memset_s(&g_alloc_mem_wr, sizeof(g_alloc_mem_wr), 0,
		sizeof(g_alloc_mem_wr));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return SMMU_ERR;
	}

#ifdef ENV_ARMLINUX_KERNEL
	ret = alloc_smmu_tlb_miss_addr();
	if (ret != SMMU_OK) {
		dprint(PRN_FATAL, "%s alloc_smmu_tlb_miss_addr failed\n", __func__);
		return SMMU_ERR;
	}
#endif

	g_smmu_init_flag = 1;

	return SMMU_OK;
}

/*
  * function: free mem of SMMU_ERR_RDADDR and SMMU_ERR_WRADDR.
  */
VOID smmu_deinit(void)
{
	if (g_alloc_mem_rd.phy_addr != 0)
		smmu_mem_dealloc(&g_alloc_mem_rd);

	if (g_alloc_mem_wr.phy_addr != 0)
		smmu_mem_dealloc(&g_alloc_mem_wr);
}
