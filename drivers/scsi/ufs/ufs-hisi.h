/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef UFS_HISI_H_
#define UFS_HISI_H_

int hisi_auto_h8_op(struct ufs_hba *hba, bool auto_h8_op);
int hisi_uic_write_reg(
	struct ufs_hba *hba, uint32_t reg_offset, uint32_t value);
int hisi_uic_read_reg(
	struct ufs_hba *hba, uint32_t reg_offset, u32 *value);
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
void ufshcd_hisi_disable_auto_hibern8(struct ufs_hba *hba);
void ufshcd_hisi_print_dme_hibern_ind(struct ufs_hba *hba);
void clear_dme_intr(struct ufs_hba *hba, int dme_intr_clr);
void hisi_ufs_dme_reg_dump(struct ufs_hba *hba);
void ufshcd_enable_clock_gating(struct ufs_hba *hba);
bool ufshcd_is_hisi_ufs_hc_used(struct ufs_hba *hba);
void ufshcd_hisi_enable_unipro_intr(struct ufs_hba *hba, u32 unipro_intrs);
irqreturn_t ufshcd_hisi_unipro_intr(int unipro_irq, void *__hba);
void ufshcd_sl_fatal_intr(struct ufs_hba *hba, u32 intr_status);
irqreturn_t ufshcd_hisi_fatal_err_intr(int fatal_err_irq, void *__hba);
irqreturn_t ufshcd_hisi_core0_intr(int fatal_err_irq, void *__hba);
int wait_mphy_init_done(struct ufs_hba *hba);
int ufshcd_hisi_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode);

/*HISI HCI*/
#define HISI_UNIPRO_BIT_SHIFT 2
#define UFS_AHIT_CTRL_OFF 0x110
#define UFS_HIBERNATE_EXIT_MODE UFS_BIT(1)
#define UFS_AUTO_EN UFS_BIT(0)
#define UFS_AUTO_HIBERN_EN UFS_BIT(0)

#define UFS_AUTO_H8_STATE_OFF 0x130
#define UFS_CFG_RAM_CTRL 0x039C
#define UFS_CFG_RAM_STATUS 0x03A0

#define UFS_HC_AH8_STATE 0xFF
#define AH8_XFER 0x1
#define UFS_BLOCK_CG_CFG 0x108

#define RAM_STORE_OK 0x1
#define RAM_LOAD_OK 0x2
#define AH8_H8ING 0x20

/*HISI DME*/
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
#define DME_HIBERNATE_ENTER_LOCAL_SUCC UFS_BIT(1)
#define DME_HIBERNATE_ENTER_REMOTE_SUCC UFS_BIT(2)
#define CFG_LAYER_ENABLE UFS_BIT(0)
#define DME_ENABLE_CNF UFS_BIT(0)
#define LINK_STARTUP_CNF (UFS_BIT(0) | UFS_BIT(1))
#define LINK_STARTUP_SUCC       UFS_BIT(0)
#define LINK_STARTUP_FAIL       UFS_BIT(1)

#define DME_HIBERNATE_ENTER_SUCC                                               \
	(DME_HIBERNATE_ENTER_LOCAL_SUCC | DME_HIBERNATE_ENTER_REMOTE_SUCC)

#define DME_HIBERNATE_EXIT 0xD00E
#define DME_HIBERNATE_EXIT_STATE 0xD00F

#define DME_HIBERNATE_EXIT_IND 0xD010
#define DME_HIBERNATE_EXIT_LOCAL_SUCC UFS_BIT(1)

#define DME_POWERMODEIND 0xD011

#define DME_INTR_ENABLE 0xD050
#define DME_INTR_STATUS 0xD052

#define DME_INTR_CLR 0xD051
#define MASK_PMC_IND_INTR UFS_BIT(5)
#define DME_HIBERN_ENTER_CNF_INTR UFS_BIT(20)
#define DME_HIBERN_EXIT_CNF_INTR UFS_BIT(21)
#define DME_HIBERN_ENTER_IND_INTR UFS_BIT(3)
#define DME_HIBERN_EXIT_IND_INTR UFS_BIT(4)
#define DME_PMC_IND_INTR UFS_BIT(5)
#define MASK_DME_PMC_IND UFS_BIT(5)
#define MASK_LOCAL_ATTR_FAIL UFS_BIT(22)
#define MASK_PEER_ATTR_COMPL UFS_BIT(23)
//#define DME_ENABLE_INTRS DME_PMC_IND_INTR
#define DME_ENABLE_INTRS 0xCFFFE7
#define DME_HIBERN_ENTER_INTR                                                  \
	(DME_HIBERN_ENTER_CNF_INTR | DME_HIBERN_ENTER_IND_INTR)
#define DME_HIBERN_EXIT_INTR                                                   \
	(DME_HIBERN_EXIT_CNF_INTR | DME_HIBERN_EXIT_IND_INTR)
#define DME_HIBERN_INTR (DME_HIBERN_ENTER_INTR | DME_HIBERN_EXIT_INTR)

#define DME_UE_TL_INTR UFS_BIT(13)
#define DME_UE_NL_INTR  UFS_BIT(12)
#define DME_UE_DL_INTR  UFS_BIT(11)
#define DME_UE_PA_INTR  UFS_BIT(10)
#define DME_UE_PHY_INTR UFS_BIT(9)
#define DME_UE_INTR (DME_UE_TL_INTR | DME_UE_NL_INTR | DME_UE_DL_INTR | DME_UE_PA_INTR | DME_UE_PHY_INTR)

#define DME_UNIPRO_STATE 0xD053
#define MASK_DEBUG_UNIPRO_STATE 0x3F80
#define LINK_DISABLED (0x0 << 7)
#define LINK_DOWN (0x1 << 7)
#define LINK_UP (0x2 << 7)
#define LINK_HIBERN (0x03 << 7)

#define DME_PEER_OPC_CTRL 0xD060
#define DME_PEER_OPC_STATE 0xD061
#define DME_PEER_OPC_WDATA 0xD062
#define DME_PEER_OPC_RDATA 0xD063
#define Peer_AttrCfgRdy (0x1 << 4)
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
#define DME_POWER_MODE_CHANGE_SUCC      (DME_POWER_MODE_LOCAL_SUCC | DME_POWER_MODE_REMOTE_SUCC)
#define DME_POWER_MODE_LOCAL_SUCC UFS_BIT(1)
#define DME_POWER_MODE_REMOTE_SUCC UFS_BIT(2)
#define PEER_ATTR_RES 0xF

#define PA_FSM_STATUS 0x9540
#define PA_STATUS 0x9541

#define MPHY_DEEMPH_20T4_EN (1 << 4)
#define MPHY_INIT 0x000000D6
#define MASK_MPHY_INIT (UFS_BIT(0) | UFS_BIT(1))
#define LANE0_DONE UFS_BIT(0)
#define LANE1_DONE UFS_BIT(1)
#define MPHY_INIT_DONE (LANE0_DONE | LANE1_DONE)
#define MPHY_INIT_TIMEOUT 40

/* bit 4 ~ bit 30*/
#define ATTR_LOCAL_ERR_ADDR 0x7FFFFFF0
#define ATTR_LOCAL_ERR_RES 0xF


#define DME_POWER_MODE_IND_SUCC                                                \
	(DME_POWER_MODE_LOCAL_SUCC | DME_POWER_MODE_REMOTE_SUCC)

#define UNIPRO_CLK_AUTO_GATE_WHEN_HIBERN 0x3009C0F

/* HISI Qos related registers */
#define UFS_PROC_MODE_CFG 0xC0
#define MASK_CFG_UTR_QOS_EN UFS_BIT(1)

#define UFS_DOORBELL_0_7_QOS 0xC4
#define UFS_DOORBELL_8_15_QOS 0xC8
#define UFS_DOORBELL_16_23_QOS 0xCC
#define UFS_DOORBELL_24_31_QOS 0xD0
#define UFS_CORE_DOORBELL_QOS(i) (0x2010 + (i)*0x80)

#define UFS_IS_INT_SET 0x010C
#define IP_RST_UFS_SUBSYS              14
#define IP_RST_UFS_SUBSYS_CRG 19
/* HSDT CRG related definitions */
//#define GT_CLK_UFS_NOC_ASYNCBRG (1 << 13)
#define GT_CLK_UFS_NOC_ASYNCBRG 13
#define IP_RST_UFS_NOC_ASYNCBRG 0

#define NOC_UFS_POWER_IDLEACK (1 << 1)
#define NOC_UFS_POWER_IDLE (1 << 1)
#define GT_CLK_UFS_SUBSYS (1 << 14)
#define SOC_SCTRL_SCPERRSTEN1 0x20C
#define SOC_SCTRL_SCPERDIS4 0x1B4
#define NOC_UFS_POWER_IDLEREQ_MASK (1 << 17)
#define RAM_STORE_OK 0x1
#define MPHY_PLL_LOCK (1 << 1)

/* PMC related definitions */
#define NOC_UFS_POWER_IDLEREQ 1
#define NOC_UFS_POWER_IDLEACK_0 (1 << 1)
#define NOC_UFS_POWER_IDLE_0 (1 << 1)

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

/* QOS_I_OUTSTANDING_NUM is N, then in fact num is N+1 */
#define QOS_0_OUTSTANDING_NUM 0x0
#define QOS_1_OUTSTANDING_NUM 0x1
#define QOS_2_OUTSTANDING_NUM 0x2
#define QOS_3_OUTSTANDING_NUM 0x3
#define QOS_4_OUTSTANDING_NUM 0x4
#define QOS_5_OUTSTANDING_NUM 0x5
#define QOS_6_OUTSTANDING_NUM 0x6
#define QOS_7_OUTSTANDING_NUM 0x7

#define QOS_0_PROMOTE_NUM 0x0 // modify
#define QOS_1_PROMOTE_NUM 0x1
#define QOS_2_PROMOTE_NUM 0x2
#define QOS_3_PROMOTE_NUM 0x3
#define QOS_4_PROMOTE_NUM 0x4
#define QOS_5_PROMOTE_NUM 0x5
#define QOS_6_PROMOTE_NUM 0x6

#define QOS_0_INCREASE_NUM 0x0 // change this
#define QOS_1_INCREASE_NUM 0x1
#define QOS_2_INCREASE_NUM 0x2
#define QOS_3_INCREASE_NUM 0x3
#define QOS_4_INCREASE_NUM 0x4
#define QOS_5_INCREASE_NUM 0x5
#define QOS_6_INCREASE_NUM 0x6

#define UFS_TR_QOS_0_3_PROMOTE 0xE0
#define UFS_TR_QOS_4_6_PROMOTE 0xE4

#define UFS_TR_QOS_0_3_INCREASE 0xE8
#define UFS_TR_QOS_4_6_INCREASE 0xEC

#define UFS_HISI_AUTO_H8_ENABLE true
#define UFS_HISI_AUTO_H8_DISABLE false

#define UFS_IE_KEY_KDF_EN 0x03AC


enum dme_local_opc_state {
	DME_LOCAL_OPC_SUCCESS = 0x0,
	DME_LOCAL_OPC_BUSY = 0x9,
};
// enum dme_unipro_state {
// DME_UNIPRO_LINKUP,
//};

#define UFS_HISI_H8_OP_ENTER true
#define UFS_HISI_H8_OP_EXIT false

#define DL_TC0RxInitCreditVal 0x2002
#define DL_TC0TXFCThreshold 0x2040

#define CFG_RX_CTRL 0x9521
#define DBG_CFG_RX 0x9580

#define DL_IMPL_STATE 0xA011

#define ATTR_MTX0(x)                                                           \
	((0x00 << 16) |                                                        \
		x) /* <<2 will be done in read/write_register functions*/
#define ATTR_MTX1(x) ((0x01 << 16) | x)
#define ATTR_MRX0(x) ((0x02 << 16) | x)
#define ATTR_MRX1(x) ((0x03 << 16) | x)

#define PEER_MTX0(x) ((0x00 << 16) | x)
#define PEER_MTX1(x) ((0x01 << 16) | x)
#define PEER_MRX0(x) ((0x02 << 16) | x)
#define PEER_MRX1(x) ((0x03 << 16) | x)

/*For backward compatible between HISI and SNPS UFS*/
#define SNPS_TX0_SEL 0x0000 /*also for CB*/
#define SNPS_TX1_SEL 0x0001
#define SNPS_RX0_SEL 0x0004
#define SNPS_RX1_SEL 0x0005
#define HISI_TX0_SEL 0x0000 /*also for CB*/
#define HISI_TX1_SEL 0x0001
#define HISI_RX0_SEL 0x0002
#define HISI_RX1_SEL 0x0003
#define SPEC_RX0_SEL    0x0004
#define SPEC_RX1_SEL    0x0005
#define UIC_SLE_MASK 0x0000FFFF
#define UIC_ADDR_MASK 0xFFFF0000
#define UIC_SHIFT 16
#define INVALID_CMD 0xFFFFFFFF

#define HISI_UIC_ACCESS_REG_RETRIES 3
#define HISI_UIC_ACCESS_REG_TIMEOUT 500
#define HISI_UFS_DME_LINKUP_TIMEOUT 250

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

static struct ufs_attr_cfg hisi_mphy_v200_pre_link_attr[] = {
	{0x0000156A, 0x00000002}, /* PA_HSSeries */
	{0x00020009, 0x00000001}, /*Rx SKP_DET_SEL, lane0 */
	{0x00030009, 0x00000001}, /*Rx SKP_DET_SEL, lane1 */
	{0x000000DF, 0x00000003}, /*VCO_AUTO_CHG */
	/*FPGA can only support HS Gear 1, Gear 4 for only test chip*/
	{0x00000023, 0x00000004}, /*TX_HSGEAR */
	{0x00010023, 0x00000004}, /*TX_HSGEAR */
	{0x000200a3, 0x00000004}, /*RX_HSGEAR */
	{0x000300a3, 0x00000004}, /*RX_HSGEAR */
	{0x000200F1, 0x00000004}, /*RX_SQ_VREF, lane0 ,RX_SQ_VREF_140mv*/
	{0x000300F1, 0x00000004}, /*RX_SQ_VREF, lane1 ,RX_SQ_VREF_140mv*/
	{0x00020003, 0x0000000A}, /*AD_DIF_P_LS_TIMEOUT_VAL, lane0 */
	{0x00030003, 0x0000000A}, /*AD_DIF_P_LS_TIMEOUT_VAL, lane1 */
	{0x00020004, 0x00000064}, /*AD_DIF_N_LS_TIMEOUT_VAL, lane0 */
	{0x00030004, 0x00000064}, /*AD_DIF_N_LS_TIMEOUT_VAL, lane1 */
	{0x000200cf, 0x00000002}, /*RX_STALL*/
	{0x000300cf, 0x00000002}, /*RX_STALL*/
	{0x000200d0, 0x00000002}, /*RX_SLEEP*/
	{0x000300d0, 0x00000002}, /*RX_SLEEP*/
	{0x000200F0, 0x00000001}, /*RX enable, lane0 */
	{0x000300F0, 0x00000001}, /*RX enable, lane1 */
	{0x00000059, 0x0000000f}, /*reg 0x59,TX0*/
	{0x00010059, 0x0000000f}, /*reg 0x59,TX1*/
	{0x0000005A, 0x00000000}, /*reg 0x5A,TX0*/
	{0x0001005A, 0x00000000}, /*reg 0x5A,TX1*/
	{0x0000005B, 0x0000000f}, /*reg 0x5B,TX0*/
	{0x0001005B, 0x0000000f}, /*reg 0x5B,TX1*/
	{0x0000005C, 0x00000000}, /*reg 0x5C,TX0*/
	{0x0001005C, 0x00000000}, /*reg 0x5C,TX1*/
	{0x0000005D, 0x00000000}, /*reg 0x5D,TX0*/
	{0x0001005D, 0x00000000}, /*reg 0x5D,TX1*/
	{0x0000005E, 0x0000000a}, /*reg 0x5E,TX0*/
	{0x0001005E, 0x0000000a}, /*reg 0x5E,TX1*/
	{0x0000005F, 0x0000000a}, /*reg 0x5F,TX0*/
	{0x0001005F, 0x0000000a}, /*reg 0x5F,TX1*/
	{0x0000007A, 0x00000000}, /*reg 0x7A,TX0*/
	{0x0001007A, 0x00000000}, /*reg 0x7A,TX1*/
	{0x0000007B, 0x00000000}, /*reg 0x7B,TX0*/
	{0x0001007B, 0x00000000}, /*reg 0x7B,TX1*/
	/*Tell UFS that RX can exit H8 only when host TX exits H8 first*/
	{0x000200C3, 0x00000004}, /*RX_EXTERNAL_H8_EXIT_EN, lane0 */
	{0x000300C3, 0x00000004}, /*RX_EXTERNAL_H8_EXIT_EN, lane1 */
	{0x000200f6, 0x00000001}, /* RX_DLF Lane 0 */
	{0x000300f6, 0x00000001}, /* RX_DLF Lane 1 */
	{0x000000C7, 0x00000003}, /* measure the power, can close it */
	{0x000000C8, 0x00000003}, /* measure the power, can close it */
	{0x000000c5, 0x00000003}, /*RG_PLL_RXHS_EN*/
	{0x000000c6, 0x00000003}, /*RG_PLL_RXLS_EN*/

	{0x000200E9, 0x00000000}, /*RX_HS_DATA_VALID_TIMER_VAL0*/
	{0x000300E9, 0x00000000}, /*RX_HS_DATA_VALID_TIMER_VAL0*/
	{0x000200EA, 0x00000010}, /*RX_HS_DATA_VALID_TIMER_VAL1*/
	{0x000300EA, 0x00000010}, /*RX_HS_DATA_VALID_TIMER_VAL1*/

	/*rx_setting_better_perform*/
	{0x000200f4, 0x00000000}, {0x000300f4, 0x00000000},
	{0x000200f3, 0x00000000}, {0x000300f3, 0x00000000},
	{0x000200f2, 0x00000003}, {0x000300f2, 0x00000003},
	{0x000200f6, 0x00000002}, {0x000300f6, 0x00000002},
	{0x000200f5, 0x00000000}, {0x000300f5, 0x00000000},
	{0x000200fc, 0x0000001f}, {0x000300fc, 0x0000001f},
	{0x000200fd, 0x00000000}, {0x000300fd, 0x00000000},
	{0x000200fb, 0x00000005}, {0x000300fb, 0x00000005},
	{0x00020011, 0x00000011}, {0x00030011, 0x00000011},
	/*need to check the value of 0x9529 in the next version of test chip*/
	{0x00009529, 0x00000006},	      /* VS_DebugSaveConfigTime */
	{0x0000D014, 0x00000001}, /* update */ /*Hisi UniPro*/
	{0, 0},
};

static struct ufs_attr_cfg hisi_mphy_v200_post_link_attr[] = {
	{0x00003000, 0x00000000}, /*N_ DeviceID */
	{0x00003001, 0x00000001}, /*N_DeviceID_valid*/
	{0x00004020, 0x00000000}, /*T_ConnectionState*/
	{0x00004022, 0x00000000}, /*T_PeerCPortID*/
	{0x00004021, 0x00000001}, /*T_PeerDeviceID*/
	{0x00004023, 0x00000000}, /*T_TrafficClass*/
	{0x00004020, 0x00000001}, /*T_ConnectionState*/
	{0x00002044, 0x00000000}, /* Unipro DL_AFC0CreditThreshold */
	{0x00002045, 0x00000000}, /* Unipro DL_TC0OutAckThreshold */
	{0x00002040, 0x00000009}, /* Unipro DL_TC0TXFCThreshold */
	{0x0002000a, 0x0000001E}, /* RX H8_TIMEOUT_VAL, Lane 0 */
	{0x0003000a, 0x0000001E}, /* RX H8_TIMEOUT_VAL, Lane 1 */
	{0x000200EB, 0x00000064}, /*AD_DIF_N_TIMEOUT_VAL*/
	{0x000300EB, 0x00000064}, /*AD_DIF_N_TIMEOUT_VAL*/
	{0x0002000E, 0x000000F0}, /*RX_H8_EXIT_TIME*/
	{0x0003000E, 0x000000F0}, /*RX_H8_EXIT_TIME*/
	{0x000015a8, 0x00000005}, /*PA_TActivate*/
	{0x000000da, 0x0000004B}, /*PLL_LOCK_TIME_VAL*/
	{0x000000dd, 0x000000CB}, /*PLL_RSTB_TIME_VAL*/
	/*Toshiba will send LCC during PMC*/
	//{0x000200c2, 0x00000000},/*MC_PRESENT, Lane 0*/
	//{0x000300c2, 0x00000000},/*MC_PRESENT, Lane 1*/
	{0, 0},
};

static struct ufs_attr_cfg hisi_mphy_v120_pre_link_attr[] = {
	{0x0000156A, 0x2}, /* PA_HSSeries */

	{0x00000023, 0x00000004}, {0x00010023, 0x00000004},
	{0x000200a3, 0x00000004}, {0x000300a3, 0x00000004},
	{0x000200f1, 0x00000004}, {0x000300f1, 0x00000004},
	{0x00020004, 0x00000064}, {0x00030004, 0x00000064},
	{0x00020003, 0x0000000A}, {0x00030003, 0x0000000A},
	{0x000200f0, 0x00000001}, {0x000300f0, 0x00000001},

	{0x00020009, 0x00000001}, /* Rx SKP_DET_SEL, lane0 */
	{0x00030009, 0x00000001}, /* Rx SKP_DET_SEL, lane1 */
	{0x000000DF, 0x00000003}, /*VCO_AUTO_CHG */
	//{0x000200F1, 0x00000002}, /*RX_SQ_VREF, lane0 ,RX_SQ_VREF_175mv */
	//{0x000300F1, 0x00000002}, /*RX_SQ_VREF, lane1 ,RX_SQ_VREF_175mv */
	//{0x00020003, 0x00000080}, /*AD_DIF_P_LS_TIMEOUT_VAL, lane0 */
	//{0x00030003, 0x00000080}, /*AD_DIF_P_LS_TIMEOUT_VAL, lane1 */
	{0x000000C7, 0x3},	/*measure the power, can close it */
	{0x000000C8, 0x3},	/*measure the power, can close it */
	{0x000200cf, 0x00000002}, /*RX_STALL*/
	{0x000300cf, 0x00000002}, /*RX_STALL*/
	{0x000200d0, 0x00000002}, /*RX_SLEEP*/
	{0x000300d0, 0x00000002}, /*RX_SLEEP*/
	{0x000200cc, 0x00000003}, /*RX_HS_CLK_EN*/
	{0x000300cc, 0x00000003}, /*RX_HS_CLK_EN*/
	{0x000200cd, 0x00000003}, /*RX_LS_CLK_EN*/
	{0x000300cd, 0x00000003}, /*RX_LS_CLK_EN*/
	{0x000000c5, 0x00000003}, /*RG_PLL_RXHS_EN*/
	{0x000000c6, 0x00000003}, /*RG_PLL_RXLS_EN*/
	{0x000200E9, 0x00000000}, /*RX_HS_DATA_VALID_TIMER_VAL0*/
	{0x000300E9, 0x00000000}, /*RX_HS_DATA_VALID_TIMER_VAL0*/
	{0x000200EA, 0x00000010}, /*RX_HS_DATA_VALID_TIMER_VAL1*/
	{0x000300EA, 0x00000010}, /*RX_HS_DATA_VALID_TIMER_VAL1*/
	{0x00001552, 0x0000004F}, /*PA_TxHsG1SyncLength*/
	{0x00001554, 0x0000004F}, /*PA_TxHsG2SyncLength*/
	{0x00001556, 0x0000004F}, /*PA_TxHsG3SyncLength*/
	{0x00009509, 0x0000004F}, /**/
	{0x0000950A, 0x0000000F}, /**/
	{0x000200F4, 0x1},	/*RX_EQ_SEL_R*/
	{0x000300F4, 0x1},	/*RX_EQ_SEL_R*/
	{0x000200F2, 0x3},	/*RX_EQ_SEL_C*/
	{0x000300F2, 0x3},	/*RX_EQ_SEL_C*/
	{0x000200FB, 0x3},	/*RX_VSEL*/
	{0x000300FB, 0x3},	/*RX_VSEL*/
	{0x000200F6, 0x3},	/*RX_DLF Lane 0*/
	{0x000300F6, 0x3},	/*RX_DLF Lane 1*/
	{0x0002000a, 0x00000002}, /*RX H8_TIMEOUT_VAL, Lane 0*/
	{0x0003000a, 0x00000002}, /*RX H8_TIMEOUT_VAL, Lane 1*/
	{0x000000d4, 0x00000031}, /*RG_PLL_DMY0*/
	{0x00000073, 0x00000004}, /*TX_PHY_CONFIG II*/
	{0x00010073, 0x00000004}, /*TX_PHY_CONFIG II*/
	{0x000200F0, 0x00000001}, /*RX enable, lane0 */
	{0x000300F0, 0x00000001}, /*RX enable, lane1 */
	{0x0000D014, 0x1},	/*Unipro VS_MphyCfgUpdt*/
	{0, 0},
};

static struct ufs_attr_cfg hisi_mphy_v120_post_link_attr[] = {
	{0x00003000, 0x0},	/*N_ DeviceID */
	{0x00003001, 0x1},	/*N_DeviceID_valid*/
	{0x00004020, 0x0},	/*T_ConnectionState*/
	{0x00004022, 0x0},	/*T_PeerCPortID*/
	{0x00004021, 0x1},	/*T_PeerDeviceID*/
	{0x00004023, 0x0},	/*T_TrafficClass*/
	{0x00004020, 0x1},	/*T_ConnectionState*/
	{0x00002044, 0x0},	/* Unipro DL_AFC0CreditThreshold */
	{0x00002045, 0x0},	/* Unipro DL_TC0OutAckThreshold */
	{0x00002040, 0x9},	/* Unipro DL_TC0TXFCThreshold */
	{0x0002000a, 0x0000001E}, /* RX H8_TIMEOUT_VAL, Lane 0 */
	{0x0003000a, 0x0000001E}, /* RX H8_TIMEOUT_VAL, Lane 1 */
	{0x000200EB, 0x64},       /*AD_DIF_N_TIMEOUT_VAL*/
	{0x000300EB, 0x64},       /*AD_DIF_N_TIMEOUT_VAL*/
	{0x0002000E, 0xF0},       /*RX_H8_EXIT_TIME*/
	{0x0003000E, 0xF0},       /*RX_H8_EXIT_TIME*/
	{0x000000ca, 0x3},	/**/
	{0x000015a8, 10},	 /*PA_TActivate*/
	{0, 0},
};

/* PMC to fast/fast auto Gear 1 */
static struct ufs_attr_cfg hisi_mphy_V200_tc_pre_pmc_fsg1_attr[] = {
	{0x00000050, 0x00000000}, {0x00010050, 0x00000000},
	{0x00000051, 0x00000000}, {0x00010051, 0x00000000},
	{0x00000052, 0x00000000}, {0x00010052, 0x00000000},
	{0x00000056, 0x00000000}, {0x00010056, 0x00000000},
	{0x00000057, 0x00000000}, {0x00010057, 0x00000000},
	{0x00000058, 0x0000000F}, {0x00010058, 0x0000000F},
	{0x000000A1, 0x00000000}, {0x000100A1, 0x00000000},
	{0x000000A4, 0x00000000}, {0x000100A4, 0x00000000},
	{0x000000A2, 0x0000000B}, {0x000100A2, 0x0000000B},
	{0x000000A5, 0x00000000}, {0x000100A5, 0x00000000},
	{0x000000A3, 0x0000001F}, {0x000100A3, 0x0000001F},
	{0x000000A6, 0x00000009}, {0x000100A6, 0x00000009}, {0, 0},
};

/* PMC to fast/fast auto Gear 2 */
static struct ufs_attr_cfg hisi_mphy_V200_tc_pre_pmc_fsg2_attr[] = {
	{0x00000050, 0x00000010}, {0x00010050, 0x00000010},
	{0x00000051, 0x00000000}, {0x00010051, 0x00000000},
	{0x00000052, 0x00000000}, {0x00010052, 0x00000000},
	{0x00000056, 0x00000000}, {0x00010056, 0x00000000},
	{0x00000057, 0x00000001}, {0x00010057, 0x00000001},
	{0x00000058, 0x0000000F}, {0x00010058, 0x0000000F},
	{0x000000A1, 0x00000000}, {0x000100A1, 0x00000000},
	{0x000000A4, 0x00000000}, {0x000100A4, 0x00000000},
	{0x000000A2, 0x0000000B}, {0x000100A2, 0x0000000B},
	{0x000000A5, 0x00000000}, {0x000100A5, 0x00000000},
	{0x000000A3, 0x0000001F}, {0x000100A3, 0x0000001F},
	{0x000000A6, 0x0000000D}, {0x000100A6, 0x0000000D}, {0, 0},
};

/* PMC to fast/fast auto Gear 3 */
static struct ufs_attr_cfg hisi_mphy_V200_tc_pre_pmc_fsg3_attr[] = {
	{0x00000050, 0x00000010}, {0x00010050, 0x00000010},
	{0x00000051, 0x00000000}, {0x00010051, 0x00000000},
	{0x00000052, 0x00000000}, {0x00010052, 0x00000000},
	{0x00000056, 0x00000000}, {0x00010056, 0x00000000},
	{0x00000057, 0x00000007}, {0x00010057, 0x00000007},
	{0x00000058, 0x0000000F}, {0x00010058, 0x0000000F},
	{0x000000A1, 0x00000000}, {0x000100A1, 0x00000000},
	{0x000000A4, 0x00000002}, {0x000100A4, 0x00000002},
	{0x000000A2, 0x0000000B}, {0x000100A2, 0x0000000B},
	{0x000000A5, 0x00000000}, {0x000100A5, 0x00000000},
	{0x000000A3, 0x0000001F}, {0x000100A3, 0x0000001F},
	{0x000000A6, 0x0000001F}, {0x000100A6, 0x0000001F}, {0, 0},
};

/* PMC to fast/fast auto Gear 4 */
static struct ufs_attr_cfg hisi_mphy_V200_tc_pre_pmc_fsg4_attr[] = {
	{0x00000050, 0x00000010}, {0x00010050, 0x00000010},
	{0x00000051, 0x00000000}, {0x00010051, 0x00000000},
	{0x00000052, 0x00000000}, {0x00010052, 0x00000000},
	{0x00000056, 0x00000000}, {0x00010056, 0x00000000},
	{0x00000057, 0x0000001F}, {0x00010057, 0x0000001F},
	{0x00000058, 0x0000000F}, {0x00010058, 0x0000000F},
	{0x000000A1, 0x00000000}, {0x000100A1, 0x00000000},
	{0x000000A4, 0x00000023}, {0x000100A4, 0x00000023},
	{0x000000A2, 0x0000000B}, {0x000100A2, 0x0000000B},
	{0x000000A5, 0x00000000}, {0x000100A5, 0x00000000},
	{0x000000A3, 0x0000001F}, {0x000100A3, 0x0000001F},
	{0x000000A6, 0x0000001F}, {0x000100A6, 0x0000001F}, {0, 0},
};

/* PMC to slow/slow */
static struct ufs_attr_cfg hisi_mphy_V200_tc_pre_pmc_slow_attr[] = {
	{0x00000050, 0x00000000}, {0x00010050, 0x00000000},
	{0x00000051, 0x00000000}, {0x00010051, 0x00000000},
	{0x00000052, 0x00000000}, {0x00010052, 0x00000000},
	{0x00000056, 0x00000000}, {0x00010056, 0x00000000},
	{0x00000057, 0x00000000}, {0x00010057, 0x00000000},
	{0x00000058, 0x0000000F}, {0x00010058, 0x0000000F},
	{0x000000A1, 0x00000000}, {0x000100A1, 0x00000000},
	{0x000000A4, 0x00000000}, {0x000100A4, 0x00000000},
	{0x000000A2, 0x0000000B}, {0x000100A2, 0x0000000B},
	{0x000000A5, 0x00000000}, {0x000100A5, 0x00000000},
	{0x000000A3, 0x0000001F}, {0x000100A3, 0x0000001F},
	{0x000000A6, 0x0000001f}, {0x000100A6, 0x0000001f}, {0, 0},
};

/* clang-format on */

/* ASIC */
static struct ufs_attr_cfg hisi_mphy_V300_pre_link_attr[] = {
        {0x0000156A, 0x00000002}, /* PA_HSSeries */
        {0x00020009, 0x00000001},/*Rx SKP_DET_SEL, lane0 */
        {0x00030009, 0x00000001},/*Rx SKP_DET_SEL, lane1 */
        {0x000000c5, 0x00000003},/*RG_PLL_RXHS_EN*/
        {0x000000c6, 0x00000003},/*RG_PLL_RXLS_EN*/
        {0x000000c7, 0x00000003},/*RG_PLL_TXHS_EN*/
        {0x000000c8, 0x00000003},/*RG_PLL_TXLS_EN*/
        {0x000200cf, 0x00000002}, /*RX_STALL*/
        {0x000300cf, 0x00000002}, /*RX_STALL*/
        {0x000200d0, 0x00000002}, /*RX_SLEEP*/
        {0x000300d0, 0x00000002}, /*RX_SLEEP*/
        {0x000000DF, 0x00000002},/*VCO_AUTO_CHG */
        {0x00020003, 0x0000000A},/*AD_DIF_P_LS_TIMEOUT_VAL, lane0 */
        {0x00030003, 0x0000000A},/*AD_DIF_P_LS_TIMEOUT_VAL, lane1 */
        {0x00020004, 0x00000064},/*AD_DIF_N_LS_TIMEOUT_VAL, lane0 */
        {0x00030004, 0x00000064},/*AD_DIF_N_LS_TIMEOUT_VAL, lane1 */
        {0x00020017, 0x00000084}, /* RX_AFE_CTRL lane 0, HW calibration bit2 and 7 */
        {0x00030017, 0x00000084}, /* RX_AFE_CTRL lane 1, HW calibration bit2 and 7 */
        {0x000200F0, 0x00000001},/*RX enable, lane0 */
        {0x000300F0, 0x00000001},/*RX enable, lane1 */
        {0x0000D014, 0x00000001}, /* update *//*Hisi UniPro*/
        {0, 0},
};

static struct ufs_attr_cfg hisi_mphy_V300_post_link_attr[] = {
        {0x00003000, 0x00000000}, /* N_DeviceID */
        {0x00003001, 0x00000001},/* N_DeviceID_valid */
        {0x00004020, 0x00000000},/* T_ConnectionState */
        {0x00004022, 0x00000000},/* T_PeerCPortID */
        {0x00004021, 0x00000001},/* T_PeerDeviceID */
        {0x00004023, 0x00000000},/* T_TrafficClass */
        {0x00004020, 0x00000001},/* T_ConnectionState */
        {0x00002044, 0x00000000},/* Unipro DL_AFC0CreditThreshold */
        {0x00002045, 0x00000000}, /* Unipro DL_TC0OutAckThreshold */
        {0x00002040, 0x00000009}, /* Unipro DL_TC0TXFCThreshold */
        {0x0002000A, 0x0000001E}, /* RX H8_TIMEOUT_VAL, Lane 0 */
        {0x0003000A, 0x0000001E}, /* RX H8_TIMEOUT_VAL, Lane 1*/
        {0x000200EB, 0x00000064},/* AD_DIF_N_TIMEOUT_VAL */
        {0x000300EB, 0x00000064},/* AD_DIF_N_TIMEOUT_VAL */
        {0x0002000E, 0x000000F0},/*RX_H8_EXIT_TIME */
        {0x0003000E, 0x000000F0},/* RX_H8_EXIT_TIME */
        {0x000000DF, 0x00000003}, /* VCO_AUTO_CHG */
        {0x000015A8, 0x00000005}, /* PA_TActivate */
        {0x000000DA, 0x0000004B}, /* PLL_LOCK_TIME_VAL */
        {0, 0},
};

/* PMC to Fast/Fast auto G1 */
static struct ufs_attr_cfg hisi_mphy_V300_pre_pmc_fsg1_attr[] = {
        {0x00000057, 0x00000000},/* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
        {0x00010057, 0x00000000},/* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
        {0x00020013, 0x00000033},/* RX0 CDR KI/KP, RX 0x13 */
        {0x00030013, 0x00000033},/* RX1 CDR KI/KP, RX 0x13 */
        {0x000200F3, 0x00000000},/* RX0 RX_EQ_SEL_I, RX 0xF3 */
        {0x000300F3, 0x00000000},/* RX1 RX_EQ_SEL_I, RX 0xF3 */
        {0x0002000F, 0x0000000F},/* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0x0003000F, 0x0000000F},/* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0, 0},
};

/* PMC to Fast/Fast auto G2 */
static struct ufs_attr_cfg hisi_mphy_V300_pre_pmc_fsg2_attr[] = {
        {0x00000057, 0x00000000},/* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
        {0x00010057, 0x00000000},/* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
        {0x00020013, 0x00000022},/* RX0 CDR KI/KP, RX 0x13 */
        {0x00030013, 0x00000022},/* RX1 CDR KI/KP, RX 0x13 */
        {0x000200F3, 0x00000000},/* RX0 RX_EQ_SEL_I, RX 0xF3 */
        {0x000300F3, 0x00000000},/* RX1 RX_EQ_SEL_I, RX 0xF3 */
        {0x0002000F, 0x0000000E},/* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0x0003000F, 0x0000000E},/* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0, 0},
};

/* PMC to Fast/Fast auto G3 */
static struct ufs_attr_cfg hisi_mphy_V300_pre_pmc_fsg3_attr[] = {
        {0x00000057, 0x00000007},/* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
        {0x00010057, 0x00000007},/* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
        {0x00020013, 0x00000012},/* RX0 CDR KI/KP, RX 0x13 */
        {0x00030013, 0x00000012},/* RX1 CDR KI/KP, RX 0x13 */
        {0x000200F3, 0x00000000},/* RX0 RX_EQ_SEL_I, RX 0xF3 */
        {0x000300F3, 0x00000000},/* RX1 RX_EQ_SEL_I, RX 0xF3 */
        {0x0002000F, 0x00000001},/* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0x0003000F, 0x00000001},/* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0, 0},
};

/* PMC to Fast/Fast auto G4 */
static struct ufs_attr_cfg hisi_mphy_V300_pre_pmc_fsg4_attr[] = {
        {0x00000057, 0x0000001F},/* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
        {0x00010057, 0x0000001F},/* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
        {0x00020013, 0x00000012},/* RX0 CDR KI/KP, RX 0x13 */
        {0x00030013, 0x00000012},/* RX1 CDR KI/KP, RX 0x13 */
        {0x000200F3, 0x00000001},/* RX0 RX_EQ_SEL_I, RX 0xF3 */
        {0x000300F3, 0x00000001},/* RX1 RX_EQ_SEL_I, RX 0xF3 */
        {0x0002000F, 0x00000000},/* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0x0003000F, 0x00000000},/* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0, 0},
};

/* PMC to slow/slow auto */
static struct ufs_attr_cfg hisi_mphy_V300_pre_pmc_slow_attr[] = {
        {0x00000057, 0x00000000},/* RG_TX0_SL_EN_POST[7:0] TX 0x57 */
        {0x00010057, 0x00000000},/* RG_TX1_SL_EN_POST[7:0] TX 0x57 */
        {0x00020013, 0x00000000},/* RX0 CDR KI/KP, RX 0x13 */
        {0x00030013, 0x00000000},/* RX1 CDR KI/KP, RX 0x13 */
        {0x000200F3, 0x00000000},/* RX0 RX_EQ_SEL_I, RX 0xF3 */
        {0x000300F3, 0x00000000},/* RX1 RX_EQ_SEL_I, RX 0xF3 */
        {0x0002000F, 0x00000000},/* RX0 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0x0003000F, 0x00000000},/* RX1 RG_PI_LPFI/RG_PI_LPFO, RX 0x0F */
        {0, 0},
};

#endif /* UFS_HISI_H_ */
