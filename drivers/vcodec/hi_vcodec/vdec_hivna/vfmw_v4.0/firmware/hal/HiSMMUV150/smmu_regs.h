#ifndef __HIVDEC_SMMU_REGS_H__
#define __HIVDEC_SMMU_REGS_H__

/*******************************************************************************
**SMMU COMMON registers
*/
#define SMMU_SCR        (0x0000)

//non-secure
#define SMMU_INTSTAT_NS (0x0018)
#define SMMU_SMRx_NS    (0x0020)    //(0x0020+n*0x4) SMMU control register per stream for non-secure
#define SMMU_CB_TTBR0   (0x0204)    //SMMU translation table base register for non-secure context bank0
#define SMMU_CB_TTBCR   (0x020C)    //SMMU Translation Table Base Control Register for Non-Secure Context Bank

#ifdef HISMMUV170
#define SMMU_CB_TTBR_MSB     (0x0224)
#define SMMU_ERR_ADDR_MSB_NS (0x0300)
#define SMMU_ERR_RDADDR_NS   (0x0304)
#define SMMU_ERR_WRADDR_NS   (0x0308)
#else
#define SMMU_FAMA_CTRL1_NS (0x0224) //SMMU Control Register for FAMA for TCU of Non-Secure Context Bank
#define SMMU_ADDR_MSB   (0x0300)    //Register for MSB of all 33-bits address configuration
#define SMMU_ERR_RDADDR (0x0304)    //SMMU Error Address of TLB miss for Read transaction
#define SMMU_ERR_WRADDR (0x0308)    //SMMU Error Address of TLB miss for Write transaction
#endif

/**********************************************
**MASTER(MFDE/SCD/BPD) registers
*/
#define REG_MFDE_MMU_CFG_SECURE (0x0008)
#define REG_MFDE_MMU_CFG_EN     (0x000c)
#define REG_SCD_MMU_CFG         (0x0820)
#define REG_BPD_MMU_CFG         (0x0004)
/***********************************************/

/*************************************************
**SMMU MASTER registers
*/
#define SMMU_MSTR_GLB_BYPASS    (0x0000)

#ifdef PLATFORM_HIVCODECV200
#define SMMU_MSTR_MEM_CTRL      (0x0008)
#endif
#define SMMU_MSTR_DBG_0         (0x0010)
#define SMMU_MSTR_DBG_1         (0x0014)
#define SMMU_MSTR_DBG_2         (0x0018)
#define SMMU_MSTR_DBG_3         (0x001c)
#define SMMU_MSTR_DBG_4         (0x0020)
#define SMMU_MSTR_DBG_5         (0x0024)
#define SMMU_MSTR_DBG_6         (0x0028)
/***********************************************/
#endif
