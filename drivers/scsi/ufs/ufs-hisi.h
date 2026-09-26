/*
 * ufs-hisi.h
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef HUFS_H_
#define HUFS_H_
#include "ufshcd.h"
#include "ufs-hisi-hci.h"

struct ufs_pa_layer_attr;
int hufs_uic_write_reg(
	struct ufs_hba *hba, uint32_t reg_offset, uint32_t value);
int hufs_uic_read_reg(struct ufs_hba *hba, uint32_t reg_offset, u32 *value);
int hufs_uic_peer_set(struct ufs_hba *hba, uint32_t reg_offset, uint32_t value);
int hufs_uic_peer_get(
	struct ufs_hba *hba, uint32_t reg_offset, uint32_t *value);
uint32_t snps_to_hufs_addr(uint32_t cmd, uint32_t arg1);
int hufs_dme_link_startup(struct ufs_hba *hba);
int ufshcd_wait_for_unipro_register_poll(
	struct ufs_hba *hba, u32 reg, u32 mask, u32 val, int timeout_ms);
int ufshcd_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op);
void ufshcd_hufs_enable_auto_hibern8(struct ufs_hba *hba);
int ufshcd_hufs_disable_auto_hibern8(struct ufs_hba *hba);
void ufshcd_hufs_print_dme_hibern_ind(struct ufs_hba *hba);
void clear_unipro_intr(struct ufs_hba *hba, int dme_intr_clr);
void ufshcd_idle_auto_gating(struct ufs_hba *hba);
void ufshcd_enable_clock_gating_autodiv(struct ufs_hba *hba);
bool ufshcd_is_hufs_hc(struct ufs_hba *hba);
void ufshcd_hufs_enable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs);
void ufshcd_hufs_disable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs);
irqreturn_t ufshcd_hufs_unipro_intr(int unipro_irq, void *__hba);
void unipro_clk_auto_gate_enable(struct ufs_hba *hba);
void unipro_clk_auto_gate_disable(struct ufs_hba *hba);
void ufshcd_sl_fatal_intr(struct ufs_hba *hba, u32 intr_status);
irqreturn_t ufshcd_hufs_fatal_err_intr(int fatal_err_irq, void *__hba);
int wait_mphy_init_done(struct ufs_hba *hba);
void ufshcd_hufs_host_memory_configure(struct ufs_hba *hba);
int ufshcd_hufs_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode);
void ufshcd_enable_vs_intr(struct ufs_hba *hba, u32 intrs);
void ufshcd_disable_vs_intr(struct ufs_hba *hba, u32 intrs);
void ufshcd_hufs_enable_dev_tmt_intr(struct ufs_hba *hba);
void ufshcd_hufs_enable_pwm_intr(struct ufs_hba *hba);
void ufshcd_hufs_enable_idle_tmt_cnt(struct ufs_hba *hba);
void ufshcd_hufs_disable_idle_tmt_cnt(struct ufs_hba *hba);
void ufs_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params);
int ufs_access_register_and_check(struct ufs_hba *hba, u32 access_mode);

int auto_k_enable_pmc_check(
	struct ufs_hba *hba, struct ufs_pa_layer_attr final_params);
void hufs_txpost_enable(struct ufs_pa_layer_attr *dev_req_params,u32 *value);

/* HUFS HCI */
#define HUFS_UNIPRO_BIT_SHIFT 2
#define UFS_AHIT_CTRL_OFF 0x110
#define UFS_HIBERNATE_EXIT_MODE UFS_BIT(1)
#define UFS_AUTO_HIBERN_EN UFS_BIT(0)

#define UFS_AHIT_EXIT_REQ 0x114
#define UFS_AUTO_H8_STATE_OFF 0x130
#define UFS_CFG_RAM_CTRL 0x039C
#define UFS_CFG_RAM_STATUS 0x03A0
/*
 * reg for ufs suspend and resume:
 * UFS_CFG_RAM_CTRL:	BIT(0)=1 enable ram write,
 * 			BIT(1)=1 enable ram read.
 * UFS_CFG_RAM_STATUS:	BIT(0)=1 indicates write completed,
 * 			BIT(1)=1 indicates read completed
 */
#define UFS_CFG_SUSPEND BIT(0)
#define UFS_CFG_RESUME BIT(1)

#define UFS_HC_AH8_STATE 0xFF
#define AH8_XFER 0x1
#define AH8_IDLE 0x2
#define UFS_BLOCK_CG_CFG 0x108

#define AH8_H8ING 0x20

/* HUFS DME */
#define HIBERNATE_EXIT_MODE 0xD000
#define DME_LAYERENABLE_STATE 0xD003
#define DME_LINKSTARTUPREQ 0xD008
#define DME_LINKSTARTUP_STATE 0xD009
#define DME_LINKLOSTIND_STATE 0xD00A
#define DME_HIBERNATE_ENTER_STATE 0xD00C
#define DME_HIBERNATE_EXIT_STATE 0xD00F

#define DME_HIBERNATE_REQ_RECEIVED UFS_BIT(0)
#define DME_HIBERNATE_REQ_DENIED UFS_BIT(1)
#define DME_HIBERNATE_ENTER_BUSY BIT(3)
#define DME_HIBERNATE_ENTER_LOCAL_SUCC UFS_BIT(1)
#define DME_HIBERNATE_ENTER_REMOTE_SUCC UFS_BIT(2)
#define CFG_LAYER_ENABLE UFS_BIT(0)
#define DME_ENABLE_CNF UFS_BIT(0)
#define LINK_STARTUP_CNF (UFS_BIT(0) | UFS_BIT(1))
#define LINK_STARTUP_SUCC UFS_BIT(0)
#define LINK_STARTUP_FAIL UFS_BIT(1)

#define DME_HIBERNATE_ENTER_SUCC                                               \
	(DME_HIBERNATE_ENTER_LOCAL_SUCC | DME_HIBERNATE_ENTER_REMOTE_SUCC)

#define DME_HIBERNATE_EXIT_LOCAL_SUCC UFS_BIT(1)
#define DME_HIBERNATE_EXIT_BUSY UFS_BIT(3)

#define DME_POWERMODEIND 0xD011
#define DME_PWR_BUSY UFS_BIT(3)

#define DME_INTR_RAW_STATUS 0xD049
#define DME_INTR_ENABLE 0xD050
#define DME_INTR_CLR 0xD051
#define DME_INTR_STATUS 0xD052
#define DME_UNIPRO_STATE 0xD053
#define DME_PEER_OPC_DBG 0xD065

/* unipro intrs */
#define DME_HIBERN_ENTER_IND_INTR UFS_BIT(3)
#define DME_HIBERN_EXIT_IND_INTR UFS_BIT(4)
#define PMC_IND_INTR UFS_BIT(5)
#define DEBUG_COUNTER_OVER_FLOW_INTR UFS_BIT(8)
#define LINKUP_CNF_INTR UFS_BIT(19)
#define ENDPOINT_RESET_CNF_INTR UFS_BIT(18)
#define DME_HIBERN_ENTER_CNF_INTR UFS_BIT(20)
#define DME_HIBERN_EXIT_CNF_INTR UFS_BIT(21)
#define PEER_ATTR_COMPL_INTR UFS_BIT(23)
#define DME_ENABLE_INTRS                                                       \
	(PMC_IND_INTR | ENDPOINT_RESET_CNF_INTR | LINKUP_CNF_INTR |            \
	 PEER_ATTR_COMPL_INTR | HSH8ENT_LR_INTR | LOCAL_ATTR_FAIL_INTR)

#define DME_HIBERN_ENTER_INTR                                                  \
	(DME_HIBERN_ENTER_CNF_INTR | DME_HIBERN_ENTER_IND_INTR)
#define DME_HIBERN_EXIT_INTR                                                   \
	(DME_HIBERN_EXIT_CNF_INTR | DME_HIBERN_EXIT_IND_INTR)
#define DME_HIBERN_INTR (DME_HIBERN_ENTER_INTR | DME_HIBERN_EXIT_INTR)

#define DME_UE_TL_INTR UFS_BIT(13)
#define DME_UE_NL_INTR UFS_BIT(12)
#define DME_UE_DL_INTR UFS_BIT(11)
#define DME_UE_PA_INTR UFS_BIT(10)
#define DME_UE_PHY_INTR UFS_BIT(9)
#define DME_UE_INTR                                                            \
	(DME_UE_TL_INTR | DME_UE_NL_INTR | DME_UE_DL_INTR | DME_UE_PA_INTR |   \
		DME_UE_PHY_INTR)

#define MASK_DEBUG_UNIPRO_STATE 0x3F80
#define LINK_DISABLED (0x0 << 7)
#define LINK_DOWN (0x1 << 7)
#define LINK_UP (0x2 << 7)
#define LINK_HIBERN (0x03 << 7)

#define DME_PEER_OPC_CTRL 0xD060
#define DME_PEER_OPC_STATE 0xD061
#define DME_PEER_OPC_WDATA 0xD062
#define DME_PEER_OPC_RDATA 0xD063
#define DME_UECPHY 0xD070
#define DME_UECPA 0xD071
#define DME_UECDL 0xD072
#define DME_UECNL 0xD073
#define DME_UECTL 0xD074
#define DME_UECDME 0xD075

#define INVALID_MIB_ATTRIBUTE 1
#define PEER_ATTRCFGRDY (0x1 << 4)
#define DME_LOCAL_OPC_DBG 0xD064
#define DME_LOCAL_OPC_STATE 0xD066
#define DME_CTRL0 0xD100
#define DME_CTRL1 0xD101
#define DME_POWER_MODE_CHANGE_SUCC                                             \
	(DME_POWER_MODE_LOCAL_SUCC | DME_POWER_MODE_REMOTE_SUCC)
#define DME_POWER_MODE_LOCAL_SUCC UFS_BIT(1)
#define DME_POWER_MODE_REMOTE_SUCC UFS_BIT(2)
#define PEER_ATTR_RES 0xF
#define LOCAL_ATTR_BUSY 0x9

#define DME_DFX_MMI_TX_CFG_STATE 0xD202
#define DME_DFX_MMI_RX_CFG_STATE 0xD203
#define DME_DFX_MMI_TX_SYS_STATE0 0xD204
#define DME_DFX_MMI_RX_SYS_STATE0 0xD206
#define DME_DFX_TX_LATL_ITF_STATE 0xD208
#define DME_DFX_RX_LATL_ITF_STATE 0xD209


#define TX_AUTO_BURST_SEL 0x952B
#define HSH8ENT_LR 0x952D
#define PA_FSM_STATUS 0x9540
#define PA_STATUS 0x9541
#define TX_CFG_DBG0 0x9581
#define PA_TX_DBG1 0x9582
#define PA_DBG_CFG_RX02 0x9593
#define PA_DBG_CFG_RX03 0x9594
#define PA_DBG_TX_TIMER 0x9596

/* L2 Registers */
#define DL_IMPL_STATE 0xA011
#define DL_DBG_DL_TOP1 0xA013
#define DL_DBG_DL_TOP2 0xA014
#define DL_DBG_TX_TOP0 0xA015
#define DL_DBG_TX_TOP1 0xA016
#define DL_DBG_TX_TOP2 0xA017
#define DL_DBG_TX_TOP3 0xA018
#define DL_DBG_TX_TOP4 0xA019
#define DL_IMPL_ERROR_PA_INIT_COUNT 0xA01B

#define MPHY_DEEMPH_20T4_EN (1 << 4)
#define MPHY_INIT 0x000000D6
#define MASK_MPHY_INIT (UFS_BIT(0) | UFS_BIT(1))
#define LANE0_DONE UFS_BIT(0)
#define LANE1_DONE UFS_BIT(1)
#define MPHY_INIT_DONE (LANE0_DONE | LANE1_DONE)
#define MPHY_INIT_TIMEOUT 40

/* bit 4 ~ bit 30 */
#define ATTR_LOCAL_ERR_ADDR 0x7FFFFFF0
#define ATTR_LOCAL_ERR_RES 0xF
#define UFS_VS_IE 0x011C
#define VS_IS_IO_TIMEOUT UFS_BIT(0)
#define RESTORE_REG_CMPL_INTR UFS_BIT(16)
#define SAVE_REG_CMPL_INTR UFS_BIT(17)
#define UFS_AH8_EXIT_INTR UFS_BIT(18)
#define IDLE_PREJUDGE_INTR UFS_BIT(19)

#define UFS_VS_ENABLE_INTRS                                                    \
	(VS_IS_IO_TIMEOUT | AH8_ENTER_REQ_CNF_FAIL_INTR |                      \
	 AH8_EXIT_REQ_CNF_FAIL_INTR)

#define UFS_VS_IS 0x0254
#define UFS_VS_IS_SET 0x0258
#define UFS_CFG_IDLE_TIME_THRESHOLD 0x390
#define UFS_CFG_IDLE_ENABLE 0x3A4
#define IDLE_PREJUDGE_TIMTER_EN UFS_BIT(0)

#define UNIPRO_CLK_AUTO_GATE_WHEN_HIBERN 0x03001C0E

#define UFS_PROC_MODE_CFG 0xC0
#define MASK_CFG_UTR_QOS_EN UFS_BIT(1)
#define CFG_RX_CPORT_IDLE_CHK_EN UFS_BIT(20)
#define CFG_RX_CPORT_IDLE_TIMEOUT_TRSH (0xFF << 12)
#define BIT_SHIFT_DEV_TMT_TRSH 12
#define DEV_TMT_VAL 0xC8

#define UFS_IO_TIMEOUT_CHECK 0x118
#define MASK_CFG_IO_TIMEOUT_TRSH 0x7F
#define MASK_CFG_IO_TIMEOUT_CHECK_EN UFS_BIT(8)
/* IO timeout trsh in second */
#define IO_TIMEOUT_TRSH_VAL 20

#define UFS_HW_PRESS_CFG 0x200
#define UFS_TR_TIMEOUT_STATUS 0x250
#define UFS_TRP_DFX0 0x290
#define UFS_TRP_DFX1 0x294
#define UFS_TRP_DFX2 0x298
#define UFS_TRP_DFX3 0x29C
#define UFS_UTP_RX_DFX0 0x2C0
#define UFS_UTP_RX_DFX1 0x2C4
#define UFS_UTP_RX_DFX2 0x2C8
#define UFS_UTP_RX_DFX3 0x2CC
#define UFS_UTP_RX_DFX4 0x2D0
#define UFS_UTP_RX_NORM_NUM 0x2D4
#define UFS_UTP_RX_DISC_NUM 0x2D8
#define UFS_UTP_RX_DFX5 0x2DC
#define UFS_DMA1_RX_DFX0 0x2F0
#define UFS_DMA1_RX_DFX1 0x2F4
#define UFS_DMA1_RX_DFX2 0x2F8
#define UFS_DMA1_RX_DFX3 0x2FC
#define UFS_DMA1_RX_AXI_ERR_ADDRL 0x300
#define UFS_DMA1_RX_AXI_ERR_ADDRU 0x304
#define TRP_MARB_DBR_NEW_DFX0 0x3B8

#define PMIC_CLK_EN_WAIT_TIME 250 /* us */
#define UFS_SUBSYS_DIV_MASK (0x3F << 16)
#define UFS_SUBSYS_DIV7 6
#define UFS_SYSCTRL_UFS_RESET 0x01FF01FF
#define HIBERNATE_EXIT_VAL 0x1

#define UFS_IS_INT_SET 0x010C
#define IP_RST_UFS_SUBSYS 14
#define IP_RST_UFS_SUBSYS_CRG 19
/* HSDT CRG related definitions */
#define GT_CLK_UFS_NOC_ASYNCBRG 13
#define IP_RST_UFS_NOC_ASYNCBRG 0
#define GT_CLK_HSDTBUS		1
#define IP_RST_HSDTBUS		1
#define IP_RST_HSDT		5

#define NOC_UFS_POWER_IDLEACK (1 << 1)
#define NOC_UFS_POWER_IDLE (1 << 1)
#define GT_CLK_UFS_SUBSYS (1 << 14)
#define SOC_SCTRL_SCPERRSTEN1 0x20C
#define SOC_SCTRL_SCPERDIS4 0x1B4
#define SOC_SCTRL_SCISODIS 0x044
#define NOC_UFS_POWER_IDLEREQ_MASK (1 << 17)
#define MPHY_PLL_LOCK (1 << 1)
#define MPHY_PLL_LOCK_WAIT_TIME 500

/* PMC related definitions */
#define NOC_UFS_POWER_IDLEREQ 1
#define NOC_UFS_POWER_IDLEACK_0 (1 << 1)
#define NOC_UFS_POWER_IDLE_0 (1 << 1)


#define HUFS_AUTO_H8_ENABLE true
#define HUFS_AUTO_H8_DISABLE false

#define UFS_IE_KEY_KDF_EN 0x03AC

#define SAVECFG_250NS 0x4
#define PA_GRANULARITY_VAL 0x6
#define PA_TACTIVATE_VAL 0xD

enum dme_local_opc_state {
	DME_LOCAL_OPC_SUCCESS = 0x0,
	DME_LOCAL_OPC_BUSY = 0x9,
};
enum hufs_uic_reg_dump_type {
	HUFS_UIC_HIBERNATE_ENTER,
	HUFS_UIC_HIBERNATE_EXIT,
	HUFS_UIC_LINKUP_FAIL,
	HUFS_UIC_PMC_FAIL,
};

void hufs_ufs_dme_reg_dump(
	struct ufs_hba *hba, enum hufs_uic_reg_dump_type dump_type);

#define HUFS_H8_OP_ENTER true
#define HUFS_H8_OP_EXIT false

#define DL_TC0RXINITCREDITVAL 0x2002
#define DL_TC0TXFCTHRESHOLD 0x2040

#define CFG_RX_CTRL 0x9521
#define DBG_CFG_RX 0x9580

/* <<2 will be done in read/write_register functions */
#define ATTR_MTX0(x) ((0x00 << 16) | (x))
#define ATTR_MTX1(x) ((0x01 << 16) | (x))
#define ATTR_MRX0(x) ((0x02 << 16) | (x))
#define ATTR_MRX1(x) ((0x03 << 16) | (x))

#define PEER_MTX0(x) ((0x00 << 16) | (x))
#define PEER_MTX1(x) ((0x01 << 16) | (x))
#define PEER_MRX0(x) ((0x04 << 16) | (x))
#define PEER_MRX1(x) ((0x05 << 16) | (x))

/* For backward compatible between HISI and SNPS UFS */
#define TX0_SEL 0x0000 /* also for CB */
#define TX1_SEL 0x0001
#define RX0_SEL 0x0004
#define RX1_SEL 0x0005
#define HUFS_TX0_SEL 0x0000 /* also for CB */
#define HUFS_TX1_SEL 0x0001
#define HUFS_RX0_SEL 0x0002
#define HUFS_RX1_SEL 0x0003
#define SPEC_RX0_SEL 0x0004
#define SPEC_RX1_SEL 0x0005
#define UIC_SLE_MASK 0x0000FFFF
#define UIC_ADDR_MASK 0xFFFF0000
#define UIC_SHIFT 16
#define INVALID_CMD 0xFFFFFFFF

#define RX_R_CAL 0xe7
#define TX_LEVEL 12
#define R_MULTIPLIER 10
#define R_DIVISOR 9
#define HALF_R_DIVISOR (R_DIVISOR / 2)
#define RX_LEVEL 9
#define RT_LEVEL_NUM 16
#define MAX_TX_R 0xF
#define MAX_RX_R 0x1F
#define MIN_TX_R 0x1
#define MIN_RX_R 0x1

#define UFS_AUTODIV_VAL 0x07E0283F
#define UFS_AUTODIV_LPM3_BYPASS                                                \
	BIT(SOC_SCTRL_SCUFS_AUTODIV_ufsbus_div_auto_reduce_bypass_lpm3_START)

#define HUFS_UIC_ACCESS_REG_RETRIES 3
#define HUFS_UIC_ACCESS_REG_TIMEOUT 500
#define HUFS_DME_LINKUP_TIMEOUT 250
#define HUFS_AUTO_H8_XFER_TIMEOUT 50

#define L15_START_OFF_1 0x1500
#define PA_PACP_FRAME_COUNT 0x15C0
#define PA_PACP_ERROR_COUNT 0x15C1
#define L15_END_OFF_1 0x15d6
#define L15_START_OFF_2 0x9500
#define L15_END_OFF_2 0x9597
#define HCI_END_OFF 0x375
#define HCI_OFFSET 4

#define MPHY_END_OFF 0xFF
#define MISC_START_OFF_1 0xD000
#define MISC_END_OFF_1 0xD103
#define MISC_START_OFF_2 0xD200
#define MISC_END_OFF_2 0xD215
#define MISC_START_OFF_3 0x2000
#define MISC_END_OFF_3 0x2068
#define MISC_START_OFF_4 0xa001
#define MISC_END_OFF_4 0xa01c

/* MPHY registers */
#define MPHY_VCO_CTRL_I 0xAE
#define MPHY_SET_VCO_BAND_I 0xBA

#define TX_HS_CLK_EN 0x76

#define RX_AFE_CTRL_III 0x13
#define RX_AFE_CTRL_IV 0x14
#define CDR_MANUAL_MODE BIT(0)
#define CDR_RESET_EN BIT(2)

#define TX_TRAILING_CLOCKS_REG 0x1564
#define TX_TRAILING_CLK_NUMS 0xFA

#define AFE_CONFIG_VIII 0x56
#define TX_AFE_CONFIG_VI 0x57

#define RX_ERR_STATUS 0x00C4
#define RG_PLL_TXLS_EN 0xC8
#define RG_PLL_EN 0xCC

#define RG_PLL_PRE_DIV 0xC2
#define RG_PLL_FBK_P 0xC3
#define RG_PLL_FBK_S 0xC4
#define RX_SYM_ERR_COUNTER 0xC5
#define RX_DA_SQUELCH_EN 0xC8
#define RG_PLL_SWC_EN 0xC9
#define RX_RG_RX_MODE 0xCA
#define RG_PLL_RXHSGR 0xCD
#define RG_PLL_RXLSGR 0xCE
#define RG_PLL_TXHSGR 0xCF
#define RG_PLL_TXLSGR 0xD0
#define RX_HS_DATA_VALID_TIMER_VAL 0xE9
#define SW_RX_EQ_SEL_CTRL BIT(7)

#define DATA_VALID_TIME 0x40
#define DATA_VALID_TIME1 0x1F

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
#define QOS_MAX_LVL	7
/* num of QOS Level, range from 0 to 7 */
#define QOS_LVL0_DFT_OUTSTD	8
#define QOS_LVL1_DFT_OUTSTD	8
#define QOS_LVL2_DFT_OUTSTD	8
#define QOS_LVL3_DFT_OUTSTD	8
#define QOS_LVL4_DFT_OUTSTD	8
#define QOS_LVL5_DFT_OUTSTD	32
#define QOS_LVL6_DFT_OUTSTD	32
#define QOS_LVL7_DFT_OUTSTD	32

#define QOS_LVL0_PRMT_DFT_OUTSTD	8
#define QOS_LVL1_PRMT_DFT_OUTSTD	8
#define QOS_LVL2_PRMT_DFT_OUTSTD	8
#define QOS_LVL3_PRMT_DFT_OUTSTD	8
#define QOS_LVL4_PRMT_DFT_OUTSTD	8
#define QOS_LVL5_PRMT_DFT_OUTSTD	32
#define QOS_LVL6_PRMT_DFT_OUTSTD	32

#define QOS_LVL0_INCRS_DFT_OUTSTD	30
#define QOS_LVL1_INCRS_DFT_OUTSTD	30
#define QOS_LVL2_INCRS_DFT_OUTSTD	30
#define QOS_LVL3_INCRS_DFT_OUTSTD	30
#define QOS_LVL4_INCRS_DFT_OUTSTD	30
#define QOS_LVL5_INCRS_DFT_OUTSTD	16
#define QOS_LVL6_INCRS_DFT_OUTSTD	16

#define SPEC_SLOT_NUM 32
#define CORE_SLOT_NUM 64
#define SLOT_NUM_EACH_QOS_REG 8
#define QOS_SLOT_8 8
#define QOS_SLOT_16 16
#define QOS_SLOT_24 24
#define BITS_EACH_SLOT_QOS 4
#define SLOT_NUM_EACH_CORE 8
#define CORE_NUM 8

#define UFS_TR_OUTSTANDING_NUM 0xD4
#define OUTSTANDING_NUM 0x3F1F
#define UFS_TR_QOS_0_3_OUTSTANDING 0xD8
#define MASK_QOS_1 8
#define MASK_QOS_2 16
#define MASK_QOS_3 24
#define UFS_TR_QOS_4_7_OUTSTANDING 0xDC
#define MASK_QOS_5 8
#define MASK_QOS_6 16
#define MASK_QOS_7 24

/* HUFS Qos related registers */
#define UFS_DOORBELL_0_7_QOS 0xC4
#define UFS_DOORBELL_8_15_QOS 0xC8
#define UFS_DOORBELL_16_23_QOS 0xCC
#define UFS_DOORBELL_24_31_QOS 0xD0
#define UFS_CORE_DOORBELL_QOS(i) (0x2010 + (i)*0x80)
#define UFS_CORE_UTRLRSR(i) (0x201C + (i) * 0x80)
#define UFS_CORE_IS(i) (0x2000 + (i)*0x80)
#define UFS_CORE_IE(i) (0x2004 + (i)*0x80)
#define UFS_CORE_IS_INT_SET(i) (0x2008 + (i)*0x80)
#define MASK_CORE_IS_IO_TIME_SIM UFS_BIT(1)
#define MASK_CORE_IS_UTRCES_SIM UFS_BIT(0)

#define UFS_CORE_UTRLDBR(i) (0x2014 + (i)*0x80)
#define MASK_CORE_IE_IO_CMPL UFS_BIT(0)
#define MASK_CORE_IE_IO_TIMEOUT UFS_BIT(1)

#define UFS_CORE_UTRLCLR(i) (0x2018 + (i)*0x80)

#define DEFAULT_CORE_TR_TIMEOUT 500
#define CORE_TR_TIMEOUT_STATUS(i) (0x2024 + (i)*0x80)
#define UFS_MUTILQ_ARB_TO_THSH 0x3B0

#define UTR_CORE_NUM 8

#define QOS_PROMOTE_NUM 7
#define UFS_TR_QOS_0_3_PROMOTE 0xE0
#define UFS_TR_QOS_4_6_PROMOTE 0xE4

#define QOS_INCREASE_NUM 7
#define UFS_TR_QOS_0_3_INCREASE 0xE8
#define UFS_TR_QOS_4_6_INCREASE 0xEC

void set_core_utr_slot_qos(struct ufs_hba *hba, unsigned int task_tag, unsigned int qos);
int config_hufs_tr_qos(struct ufs_hba *hba);
void core_utr_qos_ctrl_init(struct ufs_hba *hba);
u64 read_core_utr_doorbells(struct ufs_hba *hba);
void set_core_utr_qos_starve_timeout(struct ufs_hba *hba, unsigned int timeout);
void ufshcd_disable_core_run_stop_reg(struct ufs_hba *hba);
void ufshcd_enable_core_run_stop_reg(struct ufs_hba *hba);
void ufshcd_core_transfer_req_compl(struct ufs_hba *hba, unsigned int cpu);
int request_irq_for_core_irq(struct ufs_hba *hba, struct device *dev);
void ufshcd_enable_core_utr_intr(struct ufs_hba *hba);
u64 read_core_utr_doorbells(struct ufs_hba *hba);
int get_tag_per_cpu(struct ufs_hba *hba, unsigned int cpu);
void ufshcd_irq_cpuhp_remove(struct ufs_hba *hba);

#endif /* CONFIG_HISI_UFS_HC_CORE_UTR */

struct ufs_attr_cfg {
	uint32_t addr;
	uint32_t val;
};
enum hufs_uic_reg_op {
	HUFS_UIC_REG_READ,
	HUFS_UIC_REG_WRITE,
};

struct hufs_uic_reg {
	uint32_t reg_offset;
	uint32_t value;
};

int get_peer_rx_activate_time(struct ufs_hba *hba, u32 *value);
int hufs_set_each_cfg_attr(struct ufs_hba *hba, struct ufs_attr_cfg *cfg);
void hufs_error_reg_dump(struct ufs_hba *hba);
void hufs_enable_all_irq(struct ufs_hba *hba);
void hufs_disable_all_irq(struct ufs_hba *hba);
void ufs_rx_eq_sel_ctrl(struct ufs_hba *hba, bool flag);
#ifdef CONFIG_SCSI_UFS_INTR_HUB
irqreturn_t ufshcd_ufs_intr_hub_intr(int ufs_intr_hub_irq, void *__hba);

#define INTR_HUB_LEVEL2_SEC_INFO_N(L2_GROUP) ((L2_GROUP) * 0x10 + 0x0)
#define INTR_HUB_LEVEL2_INTR_MASK_NS_N(L2_GROUP) ((L2_GROUP) * 0x10 + 0x4)
#define INTR_HUB_LEVEL2_INTR_STATUS_NS_N(L2_GROUP) ((L2_GROUP) * 0x10 + 0xC)

#define INTR_HUB_UFS_GROUP 2
#define INTR_UNIPRO_GIC_GROUP 2
#define INTR_UNIPRO_GIC_BIT 0
#define INTR_UFSHC_GIC_GROUP 2
#define INTR_UFSHC_GIC_BIT 2
#endif

#define UNUSED(x) ((void)(x))
#endif /* HUFS_H_ */
