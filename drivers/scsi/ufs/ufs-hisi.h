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
#ifndef UFS_HISI_H_
#define UFS_HISI_H_
#include "ufshcd.h"

extern struct ufs_pa_layer_attr;
int hisi_uic_write_reg(
	struct ufs_hba *hba, uint32_t reg_offset, uint32_t value);
int hisi_uic_read_reg(struct ufs_hba *hba, uint32_t reg_offset, u32 *value);
int hisi_uic_peer_set(struct ufs_hba *hba, uint32_t reg_offset, uint32_t value);
int hisi_uic_peer_get(
	struct ufs_hba *hba, uint32_t reg_offset, uint32_t *value);
uint32_t snps_to_hisi_addr(uint32_t cmd, uint32_t arg1);
int hisi_dme_link_startup(struct ufs_hba *hba);
int ufshcd_hisi_wait_for_unipro_register_poll(
	struct ufs_hba *hba, u32 reg, u32 mask, u32 val, int timeout_ms);
int ufshcd_hisi_hibern8_op_irq_safe(struct ufs_hba *hba, bool h8_op);
int config_hisi_tr_qos(struct ufs_hba *hba);
void ufshcd_hisi_enable_auto_hibern8(struct ufs_hba *hba);
int ufshcd_hisi_disable_auto_hibern8(struct ufs_hba *hba);
void ufshcd_hisi_print_dme_hibern_ind(struct ufs_hba *hba);
void clear_unipro_intr(struct ufs_hba *hba, int dme_intr_clr);
void ufshcd_enable_clock_gating(struct ufs_hba *hba);
bool ufshcd_is_hisi_ufs_hc(struct ufs_hba *hba);
void ufshcd_hisi_enable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs);
void ufshcd_hisi_disable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs);
irqreturn_t ufshcd_hisi_unipro_intr(int unipro_irq, void *__hba);
void ufshcd_sl_fatal_intr(struct ufs_hba *hba, u32 intr_status);
irqreturn_t ufshcd_hisi_fatal_err_intr(int fatal_err_irq, void *__hba);
irqreturn_t ufshcd_hisi_core0_intr(int fatal_err_irq, void *__hba);
int wait_mphy_init_done(struct ufs_hba *hba);
void ufshcd_hisi_host_memory_configure(struct ufs_hba *hba);
int ufshcd_hisi_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode);
void ufshcd_enable_vs_intr(struct ufs_hba *hba, u32 intrs);
void ufshcd_disable_vs_intr(struct ufs_hba *hba, u32 intrs);
void dbg_hisi_dme_dump(struct ufs_hba *hba);
void ufshcd_hisi_enable_dev_tmt_intr(struct ufs_hba *hba);
void ufshcd_hisi_enable_pwm_intr(struct ufs_hba *hba);
void ufshcd_hisi_enable_idle_tmt_cnt(struct ufs_hba *hba);
void ufshcd_hisi_disable_idle_tmt_cnt(struct ufs_hba *hba);
void ufshcd_sl_vs_intr(struct ufs_hba *hba, u32 intr_status);
void ufs_hisi_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params);

extern struct ufs_pa_layer_attr;
void ufs_hisi_kirin_pwr_change_pre_change(
	struct ufs_hba *hba, struct ufs_pa_layer_attr *dev_req_params);
int auto_k_enable_pmc_check(
	struct ufs_hba *hba, struct ufs_pa_layer_attr final_params);

/* HISI HCI */
#define HISI_UNIPRO_BIT_SHIFT 2
#define UFS_AHIT_CTRL_OFF 0x110
#define UFS_HIBERNATE_EXIT_MODE UFS_BIT(1)
#define UFS_AUTO_HIBERN_EN UFS_BIT(0)

#define UFS_AUTO_H8_STATE_OFF 0x130
#define UFS_CFG_RAM_CTRL 0x039C
#define UFS_CFG_RAM_STATUS 0x03A0

#define UFS_HC_AH8_STATE 0xFF
#define AH8_XFER 0x1
#define UFS_BLOCK_CG_CFG 0x108

#define AH8_H8ING 0x20

/* HISI DME */
#define HIBERNATE_EXIT_MODE 0xD000
#define DME_LAYER_ENABLE 0xD002
#define DME_LAYERENABLE_STATE 0xD003
#define DME_LINKSTARTUPREQ 0xD008
#define DME_LINKSTARTUP_STATE 0xD009
#define DME_HIBERNATE_ENTER 0xD00B
#define DME_HIBERNATE_ENTER_STATE 0xD00C
#define DME_HIBERNATE_REQ_RECEIVED UFS_BIT(0)
#define DME_HIBERNATE_REQ_DENIED UFS_BIT(1)
#define DME_HIBERNATE_ENTER_IND 0xD00D
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

#define DME_HIBERNATE_EXIT 0xD00E
#define DME_HIBERNATE_EXIT_STATE 0xD00F

#define DME_HIBERNATE_EXIT_IND 0xD010
#define DME_HIBERNATE_EXIT_LOCAL_SUCC UFS_BIT(1)
#define DME_HIBERNATE_EXIT_BUSY UFS_BIT(3)

#define DME_POWERMODEIND 0xD011
#define PWR_BUSY UFS_BIT(3)

#define DME_INTR_ENABLE 0xD050
#define DME_INTR_CLR 0xD051
#define DME_INTR_STATUS 0xD052
#define DME_UNIPRO_STATE 0xD053

/* unipro intrs */
#define DME_HIBERN_ENTER_IND_INTR UFS_BIT(3)
#define DME_HIBERN_EXIT_IND_INTR UFS_BIT(4)
#define PMC_IND_INTR UFS_BIT(5)
#define DEBUG_COUNTER_OVER_FLOW_INTR UFS_BIT(8)
#define LINKUP_CNF_INTR UFS_BIT(19)
#define HSH8ENT_LR_INTR UFS_BIT(15)
#define ENDPOINT_RESET_CNF_INTR UFS_BIT(18)
#define DME_HIBERN_ENTER_CNF_INTR UFS_BIT(20)
#define DME_HIBERN_EXIT_CNF_INTR UFS_BIT(21)
#define LOCAL_ATTR_FAIL_INTR UFS_BIT(22)
#define PEER_ATTR_COMPL_INTR UFS_BIT(23)
#define DME_ENABLE_INTRS                                                       \
	(PMC_IND_INTR | DEBUG_COUNTER_OVER_FLOW_INTR |                         \
		ENDPOINT_RESET_CNF_INTR | LINKUP_CNF_INTR |                    \
		PEER_ATTR_COMPL_INTR)

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
#define PEER_ATTRCFGRDY (0x1 << 4)
#define DME_LOCAL_OPC_DBG 0xD064
#define DME_LOCAL_OPC_STATE 0xD066
#define DME_UECDL 0xD072
#define DME_CTRL0 0xD100
#define DME_CTRL1 0xD101
#define DEBUGCOUNTER0 0xD056
#define DEBUGCOUNTER1 0xD057
#define DBG_CNT0_MASK0 0xD058
#define DBG_CNT0_MASK1 0xD05A
#define DEBUGCOUNTER_EN 0xD05C
#define DME_POWER_MODE_CHANGE_SUCC                                             \
	(DME_POWER_MODE_LOCAL_SUCC | DME_POWER_MODE_REMOTE_SUCC)
#define DME_POWER_MODE_LOCAL_SUCC UFS_BIT(1)
#define DME_POWER_MODE_REMOTE_SUCC UFS_BIT(2)
#define PEER_ATTR_RES 0xF
#define LOCAL_ATTR_BUSY 0x9

#define PA_FSM_STATUS 0x9540
#define PA_STATUS 0x9541

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
#define AH8_ENTER_REQ_CNF_FAIL_INTR UFS_BIT(13)
#define AH8_EXIT_REQ_CNF_FAIL_INTR UFS_BIT(14)
#define UFS_RX_CPORT_TIMEOUT_INTR UFS_BIT(15)
#define RESTORE_REG_CMPL_INTR UFS_BIT(16)
#define SAVE_REG_CMPL_INTR UFS_BIT(17)
#define UFS_AH8_EXIT_INTR UFS_BIT(18)
#define IDLE_PREJUDGE_INTR UFS_BIT(19)

#define UFS_VS_ENABLE_INTRS                                                    \
	(AH8_ENTER_REQ_CNF_FAIL_INTR | AH8_EXIT_REQ_CNF_FAIL_INTR |            \
		RESTORE_REG_CMPL_INTR | SAVE_REG_CMPL_INTR)

#define UFS_VS_IS 0x0254
#define UFS_CFG_IDLE_TIME_THRESHOLD 0x390
#define UFS_CFG_IDLE_ENABLE 0x3A4
#define IDLE_PREJUDGE_TIMTER_EN UFS_BIT(0)

#define UNIPRO_CLK_AUTO_GATE_WHEN_HIBERN 0x3009C0F

#define UFS_PROC_MODE_CFG 0xC0
#define MASK_CFG_UTR_QOS_EN UFS_BIT(1)
#define CFG_RX_CPORT_IDLE_CHK_EN UFS_BIT(20)
#define CFG_RX_CPORT_IDLE_TIMEOUT_TRSH (0xFF << 12)
#define BIT_SHIFT_DEV_TMT_TRSH 12

/* HISI Qos related registers */
#define UFS_DOORBELL_0_7_QOS 0xC4
#define UFS_DOORBELL_8_15_QOS 0xC8
#define UFS_DOORBELL_16_23_QOS 0xCC
#define UFS_DOORBELL_24_31_QOS 0xD0
#define UFS_CORE_DOORBELL_QOS(i) (0x2010 + (i) * 0x80)

#define UFS_IS_INT_SET 0x010C
#define IP_RST_UFS_SUBSYS 14
#define IP_RST_UFS_SUBSYS_CRG 19
/* HSDT CRG related definitions */
#define GT_CLK_UFS_NOC_ASYNCBRG 13
#define IP_RST_UFS_NOC_ASYNCBRG 0

#define NOC_UFS_POWER_IDLEACK (1 << 1)
#define NOC_UFS_POWER_IDLE (1 << 1)
#define GT_CLK_UFS_SUBSYS (1 << 14)
#define SOC_SCTRL_SCPERRSTEN1 0x20C
#define SOC_SCTRL_SCPERDIS4 0x1B4
#define NOC_UFS_POWER_IDLEREQ_MASK (1 << 17)
#define MPHY_PLL_LOCK (1 << 1)
#define MPHY_PLL_LOCK_WAIT_TIME 500

/* PMC related definitions */
#define NOC_UFS_POWER_IDLEREQ 1
#define NOC_UFS_POWER_IDLEACK_0 (1 << 1)
#define NOC_UFS_POWER_IDLE_0 (1 << 1)

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
#define QOS_OUTSTANDING_NUM 8
#define OUTSTANDING_NUM 0x3F1F
#define UFS_TR_QOS_0_3_OUTSTANDING 0xD8
#define MASK_QOS_1 8
#define MASK_QOS_2 16
#define MASK_QOS_3 24
#define UFS_TR_QOS_4_7_OUTSTANDING 0xDC
#define MASK_QOS_5 8
#define MASK_QOS_6 16
#define MASK_QOS_7 24

#define QOS_PROMOTE_NUM 7
#define UFS_TR_QOS_0_3_PROMOTE 0xE0
#define UFS_TR_QOS_4_6_PROMOTE 0xE4

#define QOS_INCREASE_NUM 7
#define UFS_TR_QOS_0_3_INCREASE 0xE8
#define UFS_TR_QOS_4_6_INCREASE 0xEC

#define UFS_HISI_AUTO_H8_ENABLE true
#define UFS_HISI_AUTO_H8_DISABLE false

#define UFS_IE_KEY_KDF_EN 0x03AC

enum dme_local_opc_state {
	DME_LOCAL_OPC_SUCCESS = 0x0,
	DME_LOCAL_OPC_BUSY = 0x9,
};
enum hisi_uic_reg_dump_type {
	HISI_UIC_HIBERNATE_ENTER,
	HISI_UIC_HIBERNATE_EXIT,
	HISI_UIC_LINKUP_FAIL,
	HISI_UIC_PMC_FAIL,
};

void hisi_ufs_dme_reg_dump(
	struct ufs_hba *hba, enum hisi_uic_reg_dump_type dump_type);

#define UFS_HISI_H8_OP_ENTER true
#define UFS_HISI_H8_OP_EXIT false

#define DL_TC0RXINITCREDITVAL 0x2002
#define DL_TC0TXFCTHRESHOLD 0x2040

#define CFG_RX_CTRL 0x9521
#define DBG_CFG_RX 0x9580

#define DL_IMPL_STATE 0xA011

/* <<2 will be done in read/write_register functions */
#define ATTR_MTX0(x) ((0x00 << 16) | (x))
#define ATTR_MTX1(x) ((0x01 << 16) | (x))
#define ATTR_MRX0(x) ((0x02 << 16) | (x))
#define ATTR_MRX1(x) ((0x03 << 16) | (x))

#define PEER_MTX0(x) ((0x00 << 16) | (x))
#define PEER_MTX1(x) ((0x01 << 16) | (x))
#define PEER_MRX0(x) ((0x02 << 16) | (x))
#define PEER_MRX1(x) ((0x03 << 16) | (x))

/* For backward compatible between HISI and SNPS UFS */
#define SNPS_TX0_SEL 0x0000 /* also for CB */
#define SNPS_TX1_SEL 0x0001
#define SNPS_RX0_SEL 0x0004
#define SNPS_RX1_SEL 0x0005
#define HISI_TX0_SEL 0x0000 /* also for CB */
#define HISI_TX1_SEL 0x0001
#define HISI_RX0_SEL 0x0002
#define HISI_RX1_SEL 0x0003
#define SPEC_RX0_SEL 0x0004
#define SPEC_RX1_SEL 0x0005
#define UIC_SLE_MASK 0x0000FFFF
#define UIC_ADDR_MASK 0xFFFF0000
#define UIC_SHIFT 16
#define INVALID_CMD 0xFFFFFFFF

#define HISI_UIC_ACCESS_REG_RETRIES 3
#define HISI_UIC_ACCESS_REG_TIMEOUT 500
#define HISI_UFS_DME_LINKUP_TIMEOUT 250
#define HISI_AUTO_H8_XFER_TIMEOUT 50

/* MPHY registers */
#define RX_AFE_CTRL_III 0x13

struct ufs_attr_cfg {
	uint32_t addr;
	uint32_t val;
};
enum hisi_uic_reg_op {
	HISI_UIC_REG_READ,
	HISI_UIC_REG_WRITE,
};

struct hisi_uic_reg {
	uint32_t reg_offset;
	uint32_t value;
};

int hisi_set_each_cfg_attr(struct ufs_hba *hba, struct ufs_attr_cfg *cfg);
#endif /* UFS_HISI_H_ */
