/*
 * ufs-hisi-dump.c
 *
 * ufs dump for hisilicon
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "ufs-hisi-dump.h"

#include <linux/vmalloc.h>
#include "ufs-hisi.h"
#include "ufshcd.h"


#define UFS_MAGIC_NUM 0x5A5A5A5A
static const uint32_t hci_reg_list[] = {
	REG_INTERRUPT_STATUS,
	REG_CONTROLLER_STATUS,
	REG_UTP_TASK_REQ_LIST_RUN_STOP,
	UFS_AHIT_CTRL_OFF,
	UFS_AUTO_H8_STATE_OFF,
	REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER,
	REG_UIC_ERROR_CODE_DATA_LINK_LAYER,
	REG_UIC_ERROR_CODE_NETWORK_LAYER,
	REG_UIC_ERROR_CODE_TRANSPORT_LAYER,
	REG_UTP_TRANSFER_REQ_DOOR_BELL,
	UFS_VS_IS,
	UFS_IO_TIMEOUT_CHECK,
	UFS_HW_PRESS_CFG,
	UFS_TRP_DFX0,
	UFS_TRP_DFX1,
	UFS_TRP_DFX2,
	UFS_TRP_DFX3,
	UFS_UTP_RX_DFX0,
	UFS_UTP_RX_DFX1,
	UFS_UTP_RX_DFX2,
	UFS_UTP_RX_DFX3,
	UFS_UTP_RX_DFX4,
	UFS_UTP_RX_NORM_NUM,
	UFS_UTP_RX_DISC_NUM,
	UFS_UTP_RX_DFX5,
	UFS_DMA1_RX_DFX0,
	UFS_DMA1_RX_DFX1,
	UFS_DMA1_RX_DFX2,
	UFS_DMA1_RX_DFX3,
	UFS_DMA1_RX_AXI_ERR_ADDRL,
	UFS_DMA1_RX_AXI_ERR_ADDRU,
	UFS_CORE_IS(0),
	UFS_CORE_IS(1),
	UFS_CORE_IS(2),
	UFS_CORE_IS(3),
	UFS_CORE_IS(4),
	UFS_CORE_IS(5),
	UFS_CORE_IS(6),
	UFS_CORE_IS(7),
	UFS_CORE_UTRLRSR(0),
	UFS_CORE_UTRLRSR(1),
	UFS_CORE_UTRLRSR(2),
	UFS_CORE_UTRLRSR(3),
	UFS_CORE_UTRLRSR(4),
	UFS_CORE_UTRLRSR(5),
	UFS_CORE_UTRLRSR(6),
	UFS_CORE_UTRLRSR(7),
};

static const uint32_t uic_reg_list[] = {
	DME_POWERMODEIND,
	DME_INTR_RAW_STATUS,
	DME_UNIPRO_STATE,
	VS_DEBUG_COUNTER0,
	VS_DEBUG_COUNTER1,
	VS_DEBUG_COUNTER_OVERFLOW,
	DME_UECPHY,
	DME_UECPA,
	DME_UECDL,
	DME_UECDME,
	DME_LOCAL_OPC_DBG,
	DME_DFX_MMI_TX_CFG_STATE,
	DME_DFX_MMI_RX_CFG_STATE,
	DME_DFX_MMI_TX_SYS_STATE0,
	DME_DFX_MMI_RX_SYS_STATE0,
	DME_DFX_TX_LATL_ITF_STATE,
	DME_DFX_RX_LATL_ITF_STATE,
	DL_IMPL_ERROR_PA_INIT_COUNT,
	PA_STATUS,
	PA_TX_DBG1,
	PA_DBG_TX_TIMER,
	DL_IMPL_STATE,
	DL_DBG_DL_TOP1,
	DL_DBG_DL_TOP2,
	DL_DBG_TX_TOP0,
	DL_DBG_TX_TOP1,
	DL_DBG_TX_TOP2,
	DL_DBG_TX_TOP3,
	DL_DBG_TX_TOP4,
	PA_PACP_FRAME_COUNT,
	PA_PACP_ERROR_COUNT,
	PA_ACTIVETXDATALANES,
	PA_TXPWRSTATUS,
	PA_TXGEAR,
	PA_PWRMODE,
	PA_ACTIVERXDATALANES,
	PA_RXPWRSTATUS,
	PA_RXGEAR,
	ATTR_MTX0(TX_HIBERN8_CONTROL),
	ATTR_MTX1(TX_HIBERN8_CONTROL),
	ATTR_MRX0(RX_ENTER_HIBERNATE),
	ATTR_MRX1(RX_ENTER_HIBERNATE),
	ATTR_MTX0(TX_FSM_STATE),
	ATTR_MTX1(TX_FSM_STATE),
	ATTR_MRX0(RX_FSM_STATE),
	ATTR_MRX1(RX_FSM_STATE),
	ATTR_MRX0(RX_MODE),
	ATTR_MRX1(RX_MODE),
	ATTR_MRX0(RX_ERR_STATUS),
	ATTR_MRX1(RX_ERR_STATUS),
	ATTR_MRX0(RX_SYM_ERR_COUNTER_HS),
	ATTR_MRX1(RX_SYM_ERR_COUNTER_HS),
};

/* dump format "0xXXXXXXXX:0xXXXXXXXX 0x... ", so the size is reg_num*2*11 */
#define MAX_DUMP_BUFFER_SIZE                                                   \
	((ARRAY_SIZE(hci_reg_list) + ARRAY_SIZE(uic_reg_list)) * 2 * 11 + 1)
static uint8_t dump_buffer[MAX_DUMP_BUFFER_SIZE];
#define LINE_MAX_SIZE 800 /* it should be less than LOG_LINE_MAX */

void hufs_key_reg_dump(struct ufs_hba *hba)
{
	int ret;
	uint32_t i;
	uint32_t reg;
	uint32_t val;
	uint32_t len = 0;
	uint8_t *buffer = dump_buffer;
	int max_size = MAX_DUMP_BUFFER_SIZE;

	memset(buffer, 0, MAX_DUMP_BUFFER_SIZE);

	for (i = 0; i < ARRAY_SIZE(hci_reg_list); i++) {
		reg = hci_reg_list[i];
		val = ufshcd_readl(hba, reg);
		ret = snprintf(buffer + len, max_size - len, "0x%x:0x%x ", reg, val);
		if (ret <= 0)
			goto out;
		len += ret;
		if (len >= LINE_MAX_SIZE) {
			dev_err(hba->dev, "REG DUMP: %s\n", buffer);
			memset(buffer, 0, MAX_DUMP_BUFFER_SIZE);
			len = 0;
		}
	}

	for (i = 0; i < ARRAY_SIZE(uic_reg_list); i++) {
		reg = uic_reg_list[i];
		ret = hufs_uic_read_reg(hba, reg, &val);
		if (ret)
			val = UFS_MAGIC_NUM;
		ret = snprintf(buffer + len, max_size - len, "0x%x:0x%x ", reg, val);
		if (ret <= 0)
			goto out;
		len += ret;
		if (len >= LINE_MAX_SIZE) {
			dev_err(hba->dev, "REG DUMP: %s\n", buffer);
			memset(buffer, 0, MAX_DUMP_BUFFER_SIZE);
			len = 0;
		}
	}

out:
	if (len)
		dev_err(hba->dev, "REG DUMP: %s\n", buffer);
}

