/*
 * ufs-kirin.c
 *
 * basic interface for hufs
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019. All rights reserved.
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

#define pr_fmt(fmt) "ufshcd :" fmt

#include "ufs-kirin.h"
#include <linux/bootdevice.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/hisi/lpcpu_idle_sleep.h>
#include <linux/i2c.h>
#include <linux/kthread.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <soc_crgperiph_interface.h>
#include <soc_sctrl_interface.h>
#include <soc_ufs_sysctrl_interface.h>
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#include <teek_client_api.h>
#include <teek_client_constants.h>
#include <teek_client_id.h>
#endif
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
#include <linux/hisi/fbe_ctrl.h>
#include <log/imonitor.h>
#endif
#include <linux/hisi/rpmb.h>
#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>

#ifdef CONFIG_HUAWEI_UFS_DSM
#include "dsm_ufs.h"
#endif
#include "ufshcd.h"
#include "ufshci.h"
#include "unipro.h"
#include "ufs-hisi.h"
#include "ufshcd-kirin-extend.h"
#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
#include "hisi-ufs-bkops.h"
#endif

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
/* uuid to TA: 54ff868f-0d8d-4495-9d95-8e24b2a08274 */
#define UUID_TEEOS_UFS_INLINE_CRYPTO                                           \
	{                                                                      \
		0x54ff868f, 0x0d8d, 0x4495,                                    \
		{                                                              \
			0x9d, 0x95, 0x8e, 0x24, 0xb2, 0xa0, 0x82, 0x74         \
		}                                                              \
	}

#define CMD_ID_UFS_KEY_RESTORE 3
#endif

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
#define UFS_BIG_DATA_UPLOAD_CODE 940013001
#define USER_ID 65535
#define UFS_RESTORE_KEY 102

struct fbe3_ufs_restore_key_stat {
	uint8_t fbe3_enable;
	uint8_t msp_enable;
	uint8_t scene_type;
	uint8_t result;
	int hufs_delay;
	uint32_t user_id;
	int hufs_err_code;
	int vold_err_code;
};
#endif

#define PRODUCT_NAME_LEN 32
#define TEE_PARAM_TEMPREF2 2
#define TEE_PARAM_TEMPREF3 3
#define META_INPUT_LEN 11

static const struct st_register_dump unipro_reg_uic[] = {
#ifdef CONFIG_HISI_UFS_REG_DUMP
	{ DME_UECPHY, "DME_UECPHY" },
	{ DME_UECPA, "DME_UECPA" },
	{ DME_UECDL, "DME_UECDL" },
	{ DME_UECTL, "DME_UECTL" },
	{ DME_UECNL, "DME_UECNL" },
	{ DME_UECDME, "DME_UECDME" },
	{ DME_LINKLOSTIND_STATE, "DME_LINKLOSTIND_STATE" },
	{ DME_UNIPRO_STATE, "DME_UNIPRO_STATE" },
	{ DME_HIBERNATE_ENTER_STATE, "DME_HIBERNATE_ENTER_STATE" },
	{ DME_HIBERNATE_ENTER_IND, "DME_HIBERNATEENTERIND" },
	{ DME_HIBERNATE_EXIT_STATE, "DME_HIBERNATEEXIT_STATE" },
	{ DME_HIBERNATE_EXIT_IND, "DME_HIBERNATEEXITIND" },
	{ DME_POWERMODEIND, "DME_POWERMODEIND" },
	{ DME_PEER_OPC_DBG, "DME_PEER_OPC_DBG" },
	{ PA_ACTIVETXDATALANES, "Active Tx Data Lanes" },
	{ PA_ACTIVERXDATALANES, "Active Rx Data Lanes" },
	{ PA_TACTIVATE, "PA_TACTIVATE" },
	{ PA_GRANULARITY, "PA_GRANULARITY" },
	{ DME_TXRX_DL_LM_ERROR, "DME_TXRX_DL_LM_ERROR" },
	{ PA_TXPWRSTATUS, "PA_TXPWRSTATUS" },
	{ PA_RXPWRSTATUS, "PA_RXPWRSTATUS" },
	{ PA_FSM_STATUS, "PA_FSM_STATUS" },
	{ TX_AUTO_BURST_SEL, "TX_AUTO_BURST_SEL" },
	{ HSH8ENT_LR, "HSH8ENT_LR" },
	{ PA_STATUS, "PA_STATUS" },
	{ TX_CFG_DBG0, "TX_CFG_DBG0" },
	{ PA_TX_DBG1, "PA_TX_DBG1" },
	{ DL_IMPL_STATE, "DL_IMPL_STATE" },
	{ DME_INTR_STATUS, "DME_INTR_STATUS" },
#else
	{ 0xD030, "DME_HibernateEnter" },
	{ 0xD031, "DME_HibernateEnterInd" },
	{ 0xD032, "DME_HibernateExit" },
	{ 0xD033, "DME_HibernateExitInd" },
	{ 0xD060, "DME_ErrorPHYInd" },
	{ 0xD061, "DME_ErrorPAInd" },
	{ 0xD062, "DME_ErrorDInd" },
	{ 0xD063, "DME_ErrorNInd" },
	{ 0xD064, "DME_ErrorTInd" },
	{ 0xD082, "VS_L2Status" },
	{ 0xD083, "VS_PowerState" },
	{ 0xd092, "VS_DebugTxByteCount" },
	{ 0xd093, "VS_DebugRxByteCount" },
	{ 0xd094, "VS_DebugInvalidByteEnable" },
	{ 0xd095, "VS_DebugLinkStartup" },
	{ 0xd096, "VS_DebugPwrChg" },
	{ 0xd097, "VS_DebugStates" },
	{ 0xd098, "VS_DebugCounter0" },
	{ 0xd099, "VS_DebugCounter1" },
	{ 0xd09a, "VS_DebugCounter0Mask" },
	{ 0xd09b, "VS_DebugCounter1Mask" },
	{ 0xd09d, "VS_DebugCounterOverflow" },
	{ 0xd09f, "VS_DebugCounterBMask" },
	{ 0xd0a0, "VS_DebugSaveConfigTime" },
	{ 0xd0a1, "VS_DebugLoopback" },
#endif
	{ PA_TXPWRSTATUS, "PA_TXPWRSTATUS" },
	{ PA_RXPWRSTATUS, "PA_RXPWRSTATUS" },
	{ PA_HIBERN8TIME, "PA_Hibern8Time" },
	{ PA_GRANULARITY, "PA_Granularity" },
	{ PA_PACPFRAMECOUNT, "PA_PACPFrameCount" },
	{ PA_PACPERRORCOUNT, "PA_PACPErrorCount" },
	{ PA_TXHSG1SYNCLENGTH, "PA_TXHSG1SYNCLENGTH" },
	{ PA_TXHSG2SYNCLENGTH, "PA_TXHSG2SYNCLENGTH" },
	{ PA_TXHSG3SYNCLENGTH, "PA_TXHSG3SYNCLENGTH" },
};

static const struct st_register_dump tx_phy_uic[] = {
	{ 0x0021, "TX_MODE" },
	{ 0x0022, "TX_HSRATE_SERIES" },
	{ 0x0023, "TX_HSGEAR" },
	{ 0x0024, "TX_PWMGEAR" },
	{ 0x0041, "TX_FSM_STATE" },
#ifdef CONFIG_HISI_UFS_REG_DUMP
	{ TX_HIBERN8_CONTROL, "TX_HIBERN8_CONTROL" },
	{ TX_HS_SYNC_LENGTH, "TX_HS_SYNC_LENGTH" },
	{ TX_HS_PREPARE_LENGTH, "TX_HS_PREPARE_LENGTH" },
	{ TX_LS_PREPARE_LENGTH, "TX_LS_PREPARE_LENGTH" },
	{ TX_HS_CLK_EN, "TX_HS_CLK_EN" },
#endif
};

static const struct st_register_dump rx_phy_uic[] = {
	{ 0x00A1, "RX_MODE" },
	{ 0x00A2, "RX_HSRATE_SERIES" },
	{ 0x00A3, "RX_HSGEAR" },
	{ 0x00A4, "RX_PWMGEAR" },
	{ 0x00C1, "RX_FSM_STATE" },
#ifdef CONFIG_HISI_UFS_REG_DUMP
	{ RX_ENTER_HIBERNATE, "RX_ENTER_HIBERNATE" },
	{ RX_ERR_STATUS, "RX_ERR_STATUS" },
	{ RX_HS_DATA_VALID_TIMER_VAL0, "RX_HS_DATA_VALID_TIMER_VAL0" },
	{ RX_SQ_VREF, "RX_SQ_VREF" },
	{ RX_FSM_STATE, "RX_FSM_STATE" },
	{ RX_DA_SQUELCH_EN, "RX_DA_SQUELCH_EN" },
	{ RX_EXTERNAL_H8_EXIT_EN, "RX_EXTERNAL_H8_EXIT_EN" },
	{ RX_HS_CLK_EN, "RX_HS_CLK_EN" },
	{ RX_LS_CLK_EN, "RX_LS_CLK_EN" },
	{ RX_RG_RX_MODE, "RX_RG_RX_MODE" },
	{ RX_STALL, "RX_STALL" },
	{ RX_SLEEP, "RX_SLEEP" },
	{ RX_SYM_ERR_COUNTER, "RX_SYM_ERR_COUNTER" },
	{ RX_HSGEAR, "RX_HSGEAR" },
#endif
};

#ifdef CONFIG_HISI_UFS_REG_DUMP
static const struct st_register_dump common_phy_uic[] = {
	{ RG_PLL_TXLS_EN, "RG_PLL_TXLS_EN" },
	{ RG_PLL_EN, "RG_PLL_EN" },
	{ RG_PLL_BIAS_EN, "RG_PLL_BIAS_EN" },
	{ RG_PLL_PRE_DIV, "RG_PLL_PRE_DIV" },
	{ RG_PLL_SWC_EN, "RG_PLL_SWC_EN" },
	{ RG_PLL_FBK_S, "RG_PLL_FBK_S" },
	{ RG_PLL_FBK_P, "RG_PLL_FBK_P" },
	{ RG_PLL_TXHSGR, "RG_PLL_TXHSGR" },
	{ RG_PLL_RXHSGR, "RG_PLL_RXHSGR" },
	{ RG_PLL_TXLSGR, "RG_PLL_TXLSGR" },
	{ RG_PLL_RXLSGR, "RG_PLL_RXLSGR" },
	{ MPHY_VCO_CTRL_I, "MPHY_VCO_CTRL_I" },
	{ MPHY_SET_VCO_BAND_I, "MPHY_SET_VCO_BAND_I"},
};
#endif

struct st_caps_map {
	char *caps_name;
	uint64_t cap_bit;
};
static char ufs_product_name[PRODUCT_NAME_LEN];
static int __init early_parse_ufs_product_name_cmdline(char *arg)
{
	if (arg) {
		strncpy(ufs_product_name, arg,
			strnlen(arg, sizeof(ufs_product_name) - 1) + 1);
		ufs_product_name[PRODUCT_NAME_LEN - 1] = '\0';
#ifdef CONFIG_HISI_DEBUG_FS
		pr_info("cmdline ufs_product_name=%s\n", ufs_product_name);
#endif
	} else {
		pr_info("no ufs_product_name cmdline\n");
	}
	return 0;
}
early_param("ufs_product_name", early_parse_ufs_product_name_cmdline);

#ifndef CONFIG_HISI_DEBUG_FS
/*
 * remove product_name info in cmdline,
 * there is no need to pass the length of cmdline as parameter,
 * because the access to cmdline array will not cause overflow.
 */
void delete_ufs_product_name(char *cmdline)
{
	char *ufs_param = NULL;
	int i = 0;
	/*
	 * get the max length of ufs_product_name in cmdline,
	 * included the end mark '\0'
	 */
	int max_delete_length = sizeof("ufs_product_name=") + PRODUCT_NAME_LEN;

	if (!cmdline)
		return;

	ufs_param = strstr(cmdline, "ufs_product_name");
	if (!ufs_param)
		return;

	while ((ufs_param[i] != '\0') && (ufs_param[i] != ' ')) {
		if (i > max_delete_length)
			break;
		i++;
	}

	memmove(ufs_param, ufs_param + i, strlen(ufs_param + i) + 1);
}
#endif

/* Here external BL31 function declaration for UFS inline encrypt */
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#ifndef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
TEEC_Context *context;
TEEC_Session *session;

static int init_tee_context(
	TEEC_Operation *op, u32 *root_id, const char *package_name,
	int package_name_len)
{
	int ret;
	TEEC_Result result;

	/* initialize TEE environment */
	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: InitializeContext failed, Ret=0x%x\n", __func__,
			result);
		ret = result;
		return ret;
	}

	/* operation params create */
	op->started = 1;
	/* open session */
	op->paramTypes = TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE,
		TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT);

	op->params[TEE_PARAM_TEMPREF2].tmpref.buffer = (void *)root_id;
	op->params[TEE_PARAM_TEMPREF2].tmpref.size = sizeof(*root_id);
	op->params[TEE_PARAM_TEMPREF3].tmpref.buffer = (void *)package_name;
	op->params[TEE_PARAM_TEMPREF3].tmpref.size =
		(size_t)(package_name_len + 1);

	return 0;
}

static int uie_open_session(void)
{
	u32 root_id = 2012; /* specific root id for tee client */
	const char *package_name = "ufs_key_restore";
	TEEC_UUID svc_id = UUID_TEEOS_UFS_INLINE_CRYPTO;
	TEEC_Operation op = {0};
	TEEC_Result result;
	u32 origin;
	int ret;

	pr_err("%s: start ++\n", __func__);

	context = kzalloc(sizeof(TEEC_Context), GFP_KERNEL);
	if (!context) {
		ret = -ENOMEM;
		goto no_memory;
	}
	session = kzalloc(sizeof(TEEC_Session), GFP_KERNEL);
	if (!session) {
		ret = -ENOMEM;
		goto free_context;
	}

	ret = init_tee_context(
		&op, &root_id, package_name, strlen(package_name));
	if (ret)
		goto free_session;

	result = TEEK_OpenSession(context, session, &svc_id,
		TEEC_LOGIN_IDENTIFY, NULL, &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: OpenSession fail, RC=0x%x, RO=0x%x\n", __func__,
			result, origin);
		ret = result;
		goto finish_context;
	}

	pr_err("%s: end ++\n", __func__);
	return ret;

finish_context:
	TEEK_FinalizeContext(context);
free_session:
	if (session) {
		kfree(session);
		session = NULL;
	}
free_context:
	if (context) {
		kfree(context);
		context = NULL;
	}
no_memory:
	pr_err("%s: failed end ++\n", __func__);

	return ret;
}

static int set_key_in_tee(void)
{
	u32 root_id = 2012; /* specific root id for tee client */
	const char *package_name = "ufs_key_restore";
	TEEC_Operation op = {0};
	TEEC_Result result;
	u32 origin = 0;
	int ret = 0;

	if (!session) {
		pr_err("%s: session is null\n", __func__);
		return ret;
	}

	pr_err("%s: start ++\n", __func__);

	/* operation params create */
	op.started = 1;
	/* open session */
	op.paramTypes =
		TEEC_PARAM_TYPES(TEEC_NONE, TEEC_NONE, TEEC_NONE, TEEC_NONE);

	/* set root_id,package_name to params[2],params[3] */
	op.params[2].tmpref.buffer = (void *)&root_id;
	op.params[2].tmpref.size = sizeof(root_id);
	op.params[3].tmpref.buffer = (void *)package_name;
	op.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);

	result = TEEK_InvokeCommand(
		session, CMD_ID_UFS_KEY_RESTORE, &op, &origin);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: Invoke CMD fail, RC=0x%x, RO=0x%x\n", __func__,
			result, origin);
		ret = result;
	}

	pr_err("%s: end ++\n", __func__);

	return ret;
}

static int hufs_set_key(void)
{
	int err = 0;
	int i;

	/* 2 means try two times to set key */
	for (i = 0; i < 2; i++) {
		err = set_key_in_tee();
		if (!err)
			return err;

		pr_err("%s: set ufs crypto key error, times: %d\n", __func__,
			i + 1);
	}

	return err;
}
#endif
#else
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
noinline int atfd_hufs_uie_smc(
	u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 function_id asm("x0") = _function_id;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile(__asmeq("%0", "x0") __asmeq("%1", "x1") __asmeq("%2", "x2")
			     __asmeq("%3", "x3") "smc    #0\n"
		     : "+r"(function_id)
		     : "r"(arg0), "r"(arg1), "r"(arg2));

	return (int)function_id;
}
#endif
#endif

static u64 hufs_dma_mask = ~0ULL;

uint16_t hufs_mphy_read(struct ufs_hba *hba, uint16_t addr)
{
	uint16_t result;
	uint32_t value = 0;

	/* set addr high 8bit */
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGADDRMSB), (addr & 0xFF00) >> 8);
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGADDRLSB), (addr & 0xFF));
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGRDWRSEL), 0);
	ufshcd_dme_get(
		hba, UIC_ARG_MIB(0x811B), &value); /* Unipro VS_mphy_disable */
	result = (uint16_t)(value & 0xFF);
	result <<= 8; /* get high 8bit */
	ufshcd_dme_get(
		hba, UIC_ARG_MIB(0x811A), &value); /* Unipro VS_mphy_disable */
	result |= (uint16_t)(value & 0xFF);

	return result;
}

void hufs_mphy_write(struct ufs_hba *hba, uint16_t addr, uint16_t value)
{
	/* set addr high 8bit */
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGADDRMSB), (addr & 0xFF00) >> 8);
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGADDRLSB), (addr & 0xFF));
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGWRMSB), (value & 0xFF00) >> 8);
	/* set value high 8bit */
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGWRLSB), (value & 0xFF));
	ufshcd_dme_set(hba, UIC_ARG_MIB(CBCREGRDWRSEL), 1);
}

/* Not use interrupt, instead, use polling UIC_COMMAND_COMPL to save time */
static int ufshcd_polling_dme_set(
	struct ufs_hba *hba, u32 attr_sel, u32 mib_val)
{
	int retry;

	/* 100ms polling */
	retry = 0x5A9BE;
	while (--retry) {
		if (ufshcd_readl(hba, REG_CONTROLLER_STATUS) &
			UIC_COMMAND_READY)
			break;
	}
	if (unlikely(retry == 0)) {
		dev_err(hba->dev, "polling UCRDY timeout\n");
		return -EIO;
	}
	ufshcd_writel(hba, UIC_COMMAND_COMPL, REG_INTERRUPT_STATUS);
	/* Write Args */
	ufshcd_writel(hba, attr_sel, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, mib_val, REG_UIC_COMMAND_ARG_3);

	/* Write UIC Cmd */
	ufshcd_writel(
		hba, UIC_CMD_DME_SET & COMMAND_OPCODE_MASK, REG_UIC_COMMAND);
	/* 500ms polling */
	retry = 0x1C50B6;
	while (--retry) {
		if (ufshcd_readl(hba, REG_INTERRUPT_STATUS) & UIC_COMMAND_COMPL)
			break;
	}
	if (unlikely(retry == 0)) {
		dev_err(hba->dev, "polling UCCS timeout\n");
		return -ETIMEDOUT;
	}

	return 0;
}

/*
 * polling uic method, can use in phy firmware update, which is too many uic
 * command
 */
int hufs_polling_mphy_write(
	struct ufs_hba *hba, uint16_t addr, uint16_t value)
{
	unsigned int err = 0;
	 /* get addr high 8bit val */
	err |= (unsigned int)ufshcd_polling_dme_set(
		hba, UIC_ARG_MIB(0x8117), (addr & 0xFF00) >> 8);
	err |= (unsigned int)ufshcd_polling_dme_set(
		hba, UIC_ARG_MIB(0x8116), (addr & 0xFF));
	/* get value high 8bit val */
	err |= (unsigned int)ufshcd_polling_dme_set(
		hba, UIC_ARG_MIB(0x8119), (value & 0xFF00) >> 8);
	err |= (unsigned int)ufshcd_polling_dme_set(
		hba, UIC_ARG_MIB(0x8118), (value & 0xFF));
	err |= (unsigned int)ufshcd_polling_dme_set(
		hba, UIC_ARG_MIB(0x811C), 1);

	return (int)err;
}

#ifdef CONFIG_HISI_UFS_REG_DUMP
static void common_phy_reg_dump(struct ufs_hba *hba)
{
	u32 reg = 0;
	u32 index;
	u32 array_size;

	array_size = sizeof(common_phy_uic) / sizeof(struct st_register_dump);
	for (index = 0; index < array_size; index++) {
		/* dont print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, UIC_ARG_MIB(common_phy_uic[index].addr),
				   &reg))
			return;

		dev_err(hba->dev, ": %-27s 0x%x: 0x%08x\n",
			common_phy_uic[index].name, common_phy_uic[index].addr,
			reg);
	}
}
#endif

static void hufs_uic_log(struct ufs_hba *hba)
{
	unsigned int reg, reg_lane1, index, array_size;

	reg = 0;
	reg_lane1 = 0;

	array_size = sizeof(unipro_reg_uic) / sizeof(struct st_register_dump);
	for (index = 0; index < array_size; index++) {
		/* dont print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, UIC_ARG_MIB(unipro_reg_uic[index].addr),
				   &reg))
			return;

		dev_err(hba->dev, ": %-27s 0x%x : 0x%08x\n",
			unipro_reg_uic[index].name, unipro_reg_uic[index].addr,
			reg);
	}

	array_size = sizeof(tx_phy_uic) / sizeof(struct st_register_dump);
	for (index = 0; index < array_size; index++) {
		/* dont print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, UIC_ARG_MIB(tx_phy_uic[index].addr) |
						TX0_SEL, &reg))
			return;

		if (ufshcd_dme_get(hba, UIC_ARG_MIB(tx_phy_uic[index].addr) |
						TX1_SEL, &reg_lane1))
			return;

		dev_err(hba->dev,
			": %-27s 0x%x: LANE0: 0x%08x, LANE1: 0x%08x\n",
			tx_phy_uic[index].name, tx_phy_uic[index].addr, reg,
			reg_lane1);
	}

	array_size = sizeof(rx_phy_uic) / sizeof(struct st_register_dump);
	for (index = 0; index < array_size; index++) {
		/* do not print more info if one uic cmd failed */
		if (ufshcd_dme_get(hba, UIC_ARG_MIB(rx_phy_uic[index].addr) |
						RX0_SEL, &reg))
			return;

		if (ufshcd_dme_get(hba, UIC_ARG_MIB(rx_phy_uic[index].addr) |
						RX1_SEL, &reg_lane1))
			return;

		dev_err(hba->dev,
			": %-27s 0x%x: LANE0: 0x%08x, LANE1: 0x%08x\n",
			rx_phy_uic[index].name, rx_phy_uic[index].addr, reg,
			reg_lane1);
	}

#ifdef CONFIG_HISI_UFS_REG_DUMP
	common_phy_reg_dump(hba);
#endif
}

static void hufs_hci_reg_log(struct ufs_hba *hba)
{
	dev_err(hba->dev,
		": --------------------------------------------------- \n");
	dev_err(hba->dev, ": \t\tHCI STANDARD REGISTER DUMP\n");
	dev_err(hba->dev,
		": --------------------------------------------------- \n");
	dev_err(hba->dev, ": CAPABILITIES:                 0x%08x\n",
		ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES));
	dev_err(hba->dev, ": UFS VERSION:                  0x%08x\n",
		ufshcd_readl(hba, REG_UFS_VERSION));
	dev_err(hba->dev, ": PRODUCT ID:                   0x%08x\n",
		ufshcd_readl(hba, REG_CONTROLLER_DEV_ID));
	dev_err(hba->dev, ": MANUFACTURE ID:               0x%08x\n",
		ufshcd_readl(hba, REG_CONTROLLER_PROD_ID));
	dev_err(hba->dev, ": REG_CONTROLLER_AHIT:          0x%08x\n",
		ufshcd_readl(hba, REG_CONTROLLER_AHIT));
	dev_err(hba->dev, ": INTERRUPT STATUS:             0x%08x\n",
		ufshcd_readl(hba, REG_INTERRUPT_STATUS));
	dev_err(hba->dev, ": INTERRUPT ENABLE:             0x%08x\n",
		ufshcd_readl(hba, REG_INTERRUPT_ENABLE));
	dev_err(hba->dev, ": CONTROLLER STATUS:            0x%08x\n",
		ufshcd_readl(hba, REG_CONTROLLER_STATUS));
	dev_err(hba->dev, ": CONTROLLER ENABLE:            0x%08x\n",
		ufshcd_readl(hba, REG_CONTROLLER_ENABLE));
	dev_err(hba->dev, ": UIC ERR PHY ADAPTER LAYER:    0x%08x\n",
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER));
	dev_err(hba->dev, ": UIC ERR DATA LINK LAYER:      0x%08x\n",
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_DATA_LINK_LAYER));
	dev_err(hba->dev, ": UIC ERR NETWORK LATER:        0x%08x\n",
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_NETWORK_LAYER));
	dev_err(hba->dev, ": UIC ERR TRANSPORT LAYER:      0x%08x\n",
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_TRANSPORT_LAYER));
	dev_err(hba->dev, ": UIC ERR DME:                  0x%08x\n",
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_DME));
}

#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
static void hufs_hci_core_utr_reg_dump(struct ufs_hba *hba)
{
	unsigned int i;

	for (i = 0; i < UTR_CORE_NUM; i++)
		dev_err(hba->dev, ": UFS_CORE_UTRLDBR%u:	       0x%08x\n",
			i, ufshcd_readl(hba, UFS_CORE_UTRLDBR(i)));
	for (i = 0; i < UTR_CORE_NUM; i++)
		dev_err(hba->dev, ": UTP CORE UTRLCLR%u:	       0x%08x\n",
			i, ufshcd_readl(hba, UFS_CORE_UTRLCLR(i)));
	for (i = 0; i < UTR_CORE_NUM; i++)
		dev_err(hba->dev, ": UTP CORE UTRLRSR%u:	       0x%08x\n",
			i, ufshcd_readl(hba, UFS_CORE_UTRLRSR(i)));
	for (i = 0; i < UTR_CORE_NUM; i++)
		dev_err(hba->dev, ": UTP CORE IS%u:	               0x%08x\n",
			i, ufshcd_readl(hba, UFS_CORE_IS(i)));
}
#endif
static void hufs_hci_host_reg_log(struct ufs_hba *hba)
{
	dev_err(hba->dev, ": UTP TRANSF REQ INT AGG CNTRL: 0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST BASE L:   0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_BASE_L));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST BASE H:   0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_BASE_H));
#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	hufs_hci_core_utr_reg_dump(hba);
#else
	dev_err(hba->dev, ": UTP TRANSF REQ DOOR BELL:     0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST CLEAR:    0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_CLEAR));
	dev_err(hba->dev, ": UTP TRANSF REQ LIST RUN STOP: 0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_RUN_STOP));
#endif
	dev_err(hba->dev, ": UTP TASK REQ LIST BASE L:     0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_BASE_L));
	dev_err(hba->dev, ": UTP TASK REQ LIST BASE H:     0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_BASE_H));
	dev_err(hba->dev, ": UTP TASK REQ DOOR BELL:       0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL));
	dev_err(hba->dev, ": UTP TASK REQ LIST CLEAR:      0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_CLEAR));
	dev_err(hba->dev, ": UTP TASK REQ LIST RUN STOP:   0x%08x\n",
		ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_RUN_STOP));
#ifndef CONFIG_HISI_UFS_HC
	dev_err(hba->dev, ": UIC COMMAND:                  0x%08x\n",
		ufshcd_readl(hba, REG_UIC_COMMAND));
	dev_err(hba->dev, ": UIC COMMAND ARG1:             0x%08x\n",
		ufshcd_readl(hba, REG_UIC_COMMAND_ARG_1));
	dev_err(hba->dev, ": UIC COMMAND ARG2:             0x%08x\n",
		ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2));
	dev_err(hba->dev, ": UIC COMMAND ARG3:             0x%08x\n",
		ufshcd_readl(hba, REG_UIC_COMMAND_ARG_3));
	dev_err(hba->dev, ": DWC BUSTHRTL:                 0x%08x\n",
		ufshcd_readl(hba, UFS_REG_OCPTHRTL));
	dev_err(hba->dev, ": DWC HCLKDIV:                  0x%08x\n",
		ufshcd_readl(hba, UFS_REG_HCLKDIV));
#endif
}

#ifdef CONFIG_HISI_UFS_HC
static void hufs_log(struct ufs_hba *hba)
{
	/* add more core utr related reg */
	dev_err(hba->dev, ": UFS_AHIT_CTRL:                0x%08x\n",
		ufshcd_readl(hba, UFS_AHIT_CTRL_OFF));
	dev_err(hba->dev, ": UFS_VS_IE:                    0x%08x\n",
		ufshcd_readl(hba, UFS_VS_IE));
	dev_err(hba->dev, ": UFS_VS_IS:                    0x%08x\n",
		ufshcd_readl(hba, UFS_VS_IS));
	dev_err(hba->dev, ": UFS_AUTO_H8_STATE:            0x%08x\n",
		ufshcd_readl(hba, UFS_AUTO_H8_STATE_OFF));
	dev_err(hba->dev, ": UFS_BLOCK_CG_CFG:             0x%08x\n",
		ufshcd_readl(hba, UFS_BLOCK_CG_CFG));
	dev_err(hba->dev, ": UFS_PROC_MODE_CFG:            0x%08x\n",
		ufshcd_readl(hba, UFS_PROC_MODE_CFG));
	dev_err(hba->dev, ": UFS_CFG_RAM_CTRL:             0x%08x\n",
		ufshcd_readl(hba, UFS_CFG_RAM_CTRL));
	dev_err(hba->dev, ": UFS_CFG_RAM_STATUS:           0x%08x\n",
		ufshcd_readl(hba, UFS_CFG_RAM_STATUS));
	dev_err(hba->dev, ": UFS_CFG_IDLE_ENABLE:          0x%08x\n",
		ufshcd_readl(hba, UFS_CFG_IDLE_ENABLE));
	dev_err(hba->dev, ": UFS_CFG_IDLE_TIME_THRESHOLD:  0x%08x\n",
		ufshcd_readl(hba, UFS_CFG_IDLE_TIME_THRESHOLD));
}

static void hufs_hc_log(struct ufs_hba *hba)
{
#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	dev_err(hba->dev, ": UFS_DOORBELL_0_7_QOS:         0x%08x\n",
		ufshcd_readl(hba, UFS_DOORBELL_0_7_QOS));
	dev_err(hba->dev, ": UFS_DOORBELL_8_15_QOS:        0x%08x\n",
		ufshcd_readl(hba, UFS_DOORBELL_8_15_QOS));
	dev_err(hba->dev, ": UFS_DOORBELL_16_23_QOS:       0x%08x\n",
		ufshcd_readl(hba, UFS_DOORBELL_16_23_QOS));
	dev_err(hba->dev, ": UFS_DOORBELL_24_31_QOS:       0x%08x\n",
		ufshcd_readl(hba, UFS_DOORBELL_24_31_QOS));
	dev_err(hba->dev, ": UFS_TR_OUTSTANDING_NUM:       0x%08x\n",
		ufshcd_readl(hba, UFS_TR_OUTSTANDING_NUM));
	dev_err(hba->dev, ": UFS_TR_QOS_0_3_OUTSTANDING:   0x%08x\n",
		ufshcd_readl(hba, UFS_TR_QOS_0_3_OUTSTANDING));
	dev_err(hba->dev, ": UFS_TR_QOS_4_7_OUTSTANDING:   0x%08x\n",
		ufshcd_readl(hba, UFS_TR_QOS_4_7_OUTSTANDING));
	dev_err(hba->dev, ": UFS_TR_QOS_0_3_INCREASE:      0x%08x\n",
		ufshcd_readl(hba, UFS_TR_QOS_0_3_INCREASE));
	dev_err(hba->dev, ": UFS_TR_QOS_4_6_INCREASE:      0x%08x\n",
		ufshcd_readl(hba, UFS_TR_QOS_4_6_INCREASE));
	dev_err(hba->dev, ": UFS_TR_QOS_0_3_PROMOTE:       0x%08x\n",
		ufshcd_readl(hba, UFS_TR_QOS_0_3_PROMOTE));
	dev_err(hba->dev, ": UFS_TR_QOS_4_6_PROMOTE:       0x%08x\n",
		ufshcd_readl(hba, UFS_TR_QOS_4_6_PROMOTE));
#endif
	dev_err(hba->dev, ": UFS_IS_INT_SET:               0x%08x\n",
		ufshcd_readl(hba, UFS_IS_INT_SET));
	dev_err(hba->dev, ": UFS_IE_KEY_KDF_EN:            0x%08x\n",
		ufshcd_readl(hba, UFS_IE_KEY_KDF_EN));
}

static void ufs_hufs_sysctrl_log(struct ufs_hba *hba)
{
	struct hufs_host *host = hba->priv;

	dev_err(hba->dev, ": UFS_CRG_UFS_CFG:                   0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_CRG_UFS_CFG));
	dev_err(hba->dev, ": UFS_CRG_UFS_CFG1:                  0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_CRG_UFS_CFG1));
	dev_err(hba->dev, ": UFS_SYS_UFS_RESET_CTRL:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_UFS_RESET_CTRL));
	dev_err(hba->dev, ": UFS_SYS_UFSAXI_W_QOS_LMTR:         0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_UFSAXI_W_QOS_LMTR));
	dev_err(hba->dev, ": UFS_SYS_UFSAXI_R_QOS_LMTR:         0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_UFSAXI_R_QOS_LMTR));
	dev_err(hba->dev, ": UFS_SYS_CRG_UFS_STAT:              0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_CRG_UFS_STAT));
	dev_err(hba->dev, ": UFS_SYS_MEMORY_CTRL_D1W2R:         0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_MEMORY_CTRL_D1W2R));
	dev_err(hba->dev, ": UFS_SYS_UFS_CLKRST_BYPASS:         0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_UFS_CLKRST_BYPASS));
	dev_err(hba->dev, ": UFS_SYS_AO_MEMORY_CTRL:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_AO_MEMORY_CTRL));
	dev_err(hba->dev, ": UFS_SYS_MEMORY_CTRL:               0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_MEMORY_CTRL));
	dev_err(hba->dev, ": UFS_SYS_HIUFS_MPHY_CFG:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_HIUFS_MPHY_CFG));
	dev_err(hba->dev, ": UFS_SYS_HIUFS_DEBUG:               0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_HIUFS_DEBUG));
	dev_err(hba->dev, ": UFS_SYS_MPHYMCU_TEST_POINT:        0x%08x\n",
		ufs_sys_ctrl_readl(host, UFS_SYS_MPHYMCU_TEST_POINT));
}
#else

static void hufs_sysctrl_log(struct ufs_hba *hba)
{
	struct hufs_host *host = hba->priv;

	dev_err(hba->dev, ": UFSSYS_MEMORY_CTRL:             0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_MEMORY_CTRL));
	dev_err(hba->dev, ": UFSSYS_PSW_POWER_CTRL:          0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PSW_POWER_CTRL));
	dev_err(hba->dev, ": UFSSYS_PHY_ISO_EN:              0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PHY_ISO_EN));
	dev_err(hba->dev, ": UFSSYS_HC_LP_CTRL:              0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_HC_LP_CTRL));
	dev_err(hba->dev, ": UFSSYS_PHY_CLK_CTRL:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PHY_CLK_CTRL));
	dev_err(hba->dev, ": UFSSYS_PSW_CLK_CTRL:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PSW_CLK_CTRL));
	dev_err(hba->dev, ": UFSSYS_CLOCK_GATE_BYPASS:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_CLOCK_GATE_BYPASS));
	dev_err(hba->dev, ": UFSSYS_RESET_CTRL_EN:           0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_RESET_CTRL_EN));
	dev_err(hba->dev, ": UFSSYS_PHY_RESET_STATUS:        0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PHY_RESET_STATUS));
	dev_err(hba->dev, ": UFSSYS_HC_DEBUG:                0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_HC_DEBUG));
	dev_err(hba->dev, ": UFSSYS_PHY_MPX_TEST_CTRL:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PHY_MPX_TEST_CTRL));
	dev_err(hba->dev, ": UFSSYS_PHY_MPX_TEST_OBSV:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PHY_MPX_TEST_OBSV));
	dev_err(hba->dev, ": UFSSYS_PHY_DTB_OUT:             0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_PHY_DTB_OUT));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_HH:        0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_HH));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_H:         0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_H));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_L:         0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_L));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITORUP_H:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITORUP_H));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITORUP_L:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITORUP_L));
	dev_err(hba->dev, ": UFSSYS_MK2_CTRL:                0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_MK2_CTRL));
	dev_err(hba->dev, ": UFSSYS_UFS_SYSCTRL:             0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_UFS_SYSCTRL));
	dev_err(hba->dev, ": UFSSYS_UFS_RESET_CTRL:          0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_UFS_RESET_CTRL));
	dev_err(hba->dev, ": UFSSYS_UFS_UMECTRL:             0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_UFS_UMECTRL));
}

static void hufs_hc_sysctrl_log(struct ufs_hba *hba)
{
	struct hufs_host *host = hba->priv;

	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_UME_HH:    0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_UME_HH));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_UME_H:     0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_UME_H));
	dev_err(hba->dev, ": UFSSYS_DEBUG_MONITOR_UME_L:     0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_DEBUG_MONITOR_UME_L));
	dev_err(hba->dev, ": UFSSYS_UFS_MEM_CLK_GATE_BYPASS: 0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_UFS_MEM_CLK_GATE_BYPASS));
	dev_err(hba->dev, ": UFSSYS_CRG_UFS_CFG:             0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_CRG_UFS_CFG));
	dev_err(hba->dev, ": UFSSYS_CRG_UFS_CFG1:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_CRG_UFS_CFG1));
	dev_err(hba->dev, ": UFSSYS_UFSAXI_W_QOS_LMTR:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_UFSAXI_W_QOS_LMTR));
	dev_err(hba->dev, ": UFSSYS_UFSAXI_R_QOS_LMTR:       0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_UFSAXI_R_QOS_LMTR));
	/* add more core utr related reg */
	dev_err(hba->dev, ": UFSSYS_CRG_UFS_STAT:            0x%08x\n",
		ufs_sys_ctrl_readl(host, UFSSYS_CRG_UFS_STAT));
}
#endif

static void hufs_hci_log(struct ufs_hba *hba)
{
	hufs_hci_reg_log(hba);
	hufs_hci_host_reg_log(hba);
#ifdef CONFIG_HISI_UFS_HC
	hufs_log(hba);
	hufs_hc_log(hba);
#endif

	dev_err(hba->dev,
		": --------------------------------------------------- \n");
	dev_err(hba->dev, ": \t\tUFS SYSCTRL REGISTER DUMP\n");
	dev_err(hba->dev,
		": --------------------------------------------------- \n");
#ifdef CONFIG_HISI_UFS_HC
	ufs_hufs_sysctrl_log(hba);
#else
	hufs_sysctrl_log(hba);
	hufs_hc_sysctrl_log(hba);
#endif
}

#ifdef CONFIG_HISI_UFS_HC
void dbg_hufs_dme_dump(struct ufs_hba *hba)
{
	hufs_ufs_dme_reg_dump(hba, HUFS_UIC_HIBERNATE_ENTER);
	hufs_ufs_dme_reg_dump(hba, HUFS_UIC_HIBERNATE_EXIT);
	hufs_ufs_dme_reg_dump(hba, HUFS_UIC_LINKUP_FAIL);
	hufs_ufs_dme_reg_dump(hba, HUFS_UIC_PMC_FAIL);
}
#endif

int hufs_check_hibern8(struct ufs_hba *hba)
{
	unsigned int err;
	u32 tx_fsm_val_0 = 0;
	u32 tx_fsm_val_1 = 0;
	unsigned long timeout = jiffies + msecs_to_jiffies(HBRN8_POLL_TOUT_MS);

	do {
		err = ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 0),
			&tx_fsm_val_0);
		err |= (unsigned int)ufshcd_dme_get(hba,
			UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 1), &tx_fsm_val_1);
		if (err || (tx_fsm_val_0 == TX_FSM_HIBERN8 &&
				   tx_fsm_val_1 == TX_FSM_HIBERN8))
			break;

		/* sleep for 100us ~ 200us */
		usleep_range(100, 200);
	} while (time_before(jiffies, timeout));

	/*
	 * we might have scheduled out for long during polling so
	 * check the state again.
	 */
	if (time_after(jiffies, timeout)) {
		err = ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 0),
			&tx_fsm_val_0);
		err |= (unsigned int)ufshcd_dme_get(hba,
			UIC_ARG_MIB_SEL(MPHY_TX_FSM_STATE, 1), &tx_fsm_val_1);
	}

	if (err) {
		dev_err(hba->dev, "%s: unable to get TX_FSM_STATE, err %u\n",
			__func__, err);
	} else if (tx_fsm_val_0 != TX_FSM_HIBERN8 ||
		   tx_fsm_val_1 != TX_FSM_HIBERN8) {
		err = (unsigned int)(-1);
		dev_err(hba->dev,
			"%s: invalid TX_FSM_STATE, lane0 = %u, lane1 = %u\n",
			__func__, tx_fsm_val_0, tx_fsm_val_1);
	}

	return (int)err;
}

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
static uint32_t
get_restore_key_data(const struct fbe3_ufs_restore_key_stat *big_data,
		     struct imonitor_eventobj *obj)
{
	uint32_t stats = 0;

	if (!big_data || !obj) {
		pr_err("%s: param is invalid!", __func__);
		return EINVAL;
	}

	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "FBE3_Enable",
							 big_data->fbe3_enable);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "MSP_Enable",
							 big_data->msp_enable);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Scene_Type",
							 big_data->scene_type);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Result",
							 big_data->result);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "hufs_delay",
							 big_data->hufs_delay);
	stats |= (uint32_t)imonitor_set_param_integer_v2(obj, "Profile_ID",
							 big_data->user_id);
	stats |= (uint32_t)imonitor_set_param_integer_v2(
		obj, "Hufs_Error_Code", big_data->hufs_err_code);
	stats |= (uint32_t)imonitor_set_param_integer_v2(
		obj, "Vold_Error_Code", big_data->vold_err_code);

	return stats;
}

static void ufs_restore_key_upload_bigdata(int ret)
{
	struct fbe3_ufs_restore_key_stat stat_data;
	struct imonitor_eventobj *obj = NULL;
	uint32_t stats;
	int err;

	/* fill context of big data */
	stat_data.fbe3_enable = 1;
	stat_data.msp_enable = 0;
	stat_data.scene_type = UFS_RESTORE_KEY;
	stat_data.result = 0;
	stat_data.hufs_delay = 0;
	stat_data.user_id = USER_ID;
	stat_data.hufs_err_code = ret;
	stat_data.vold_err_code = 0;

	obj = imonitor_create_eventobj(UFS_BIG_DATA_UPLOAD_CODE);
	if (!obj) {
		pr_err("%s: get event obj failed!\n", __func__);
		return;
	}

	stats = get_restore_key_data(&stat_data, obj);
	if (stats != 0) {
		pr_err("%s: set param failed!\n", __func__);
		imonitor_destroy_eventobj(obj);
		return;
	}
	if ((err = imonitor_send_event(obj)) < 0)
		pr_err("%s: data send failed! err=%d\n", __func__, err);

	imonitor_destroy_eventobj(obj);

	return;
}
#endif

int hufs_uie_config_init(struct ufs_hba *hba)
{
	unsigned int reg_value;
	int err = 0;

	/* enable UFS cryptographic operations on transactions */
	reg_value = ufshcd_readl(hba, REG_CONTROLLER_ENABLE);
	reg_value |= CRYPTO_GENERAL_ENABLE;
	ufshcd_writel(hba, reg_value, REG_CONTROLLER_ENABLE);

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	err = fbex_enable_kdf();
	if (err)
		BUG();
	dev_err(hba->dev, "%s: UFS inline crypto V3.0.\n", __func__);
	if (ufshcd_eh_in_progress(hba)) {
		err = fbex_restore_key();
		ufs_restore_key_upload_bigdata(err);
		if (err)
			BUG();
	}
#else
	dev_err(hba->dev, "%s: UFS inline crypto V2.0.\n", __func__);
	if (ufshcd_eh_in_progress(hba)) {
		err = hufs_set_key();
		if (err)
			BUG();
	}
#endif
#else
	/* Here UFS driver, which set SECURITY reg 0x1 in BL31,
	 * has the permission to write scurity key registers.
	 */
	err = atfd_hufs_uie_smc(RPMB_SVC_UFS_TEST, 0x0, 0x0, 0x0);
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	if (err) {
		dev_err(hba->dev,
			"%s: first set ufs inline key failed,try again.\n",
			__func__);
		err = atfd_hufs_uie_smc(RPMB_SVC_UFS_TEST, 0x0, 0x0, 0x0);
		if (err)
			BUG_ON(1);
	}
#endif
#endif
	return err;
}

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
/*
 * generate the key index use bkdrhash alg,we limit
 * the result in the range of 0~29
 */
static u32 bkdrhash_alg(u8 *str, int len)
{
	static u32 seed = 131; /* hash algorithm typical seed */
	u32 hash = 0;
	int i;

	for (i = 0; i < len; i++)
		hash = hash * seed + str[i];

	return (hash & 0xFFFFFFFF);
}

#ifdef CONFIG_HISI_DEBUG_FS
static void test_generate_cci_dun_use_bkdrhash(u8 *key, int key_len)
{
	u32 crypto_cci;
	u64 dun;
	u32 hash_res;

	hash_res = bkdrhash_alg(key, key_len);
	crypto_cci = hash_res % MAX_CRYPTO_KEY_INDEX;
	dun = (u64)hash_res;
	pr_err("%s: ufs crypto key index is %u, dun is 0x%llu\n", __func__,
		crypto_cci, dun);
}
#endif
#endif

#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) &&                         \
	defined(CONFIG_HISI_DEBUG_FS)
static void hufs_inline_crypto_debug(
	struct ufs_hba *hba, u32 *hash_res, u32 *crypto_enable, u64 dun)
{
	if (hba->inline_debug_flag == DEBUG_LOG_ON)
		dev_err(hba->dev, "%s: dun is 0x%llx\n", __func__,
			(((u64)(*hash_res)) << 32) | dun); /* get low 32bit */

	if (hba->inline_debug_flag == DEBUG_CRYPTO_ON)
		*crypto_enable = UTP_REQ_DESC_CRYPTO_ENABLE;
	else if (hba->inline_debug_flag == DEBUG_CRYPTO_OFF)
		*crypto_enable = 0x0;
}
#endif

static void hufs_crypto_set_keyindex(
	struct ufs_hba *hba, struct ufshcd_lrb *lrbp, u32 *hash_res,
	u32 *crypto_cci, unsigned long *flags)
{
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
	int key_index;
#endif
#endif

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	*hash_res = bkdrhash_alg((u8 *)lrbp->cmd->request->ci_key,
		lrbp->cmd->request->ci_key_len);
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
	key_index = (int)((uint32_t)lrbp->cmd->request->ci_key_index &
			0xff);
	/* valid ci_key_index range 0-31 */
	if ((key_index < 0) || (key_index > 31)) {
		dev_err(hba->dev, "%s: ci_key index err is 0x%x\n", __func__,
			lrbp->cmd->request->ci_key_index);
		BUG();
	}

	*crypto_cci = (uint32_t)(key_index);
#else
	*crypto_cci = (*hash_res) % MAX_CRYPTO_KEY_INDEX;
#endif

#ifdef CONFIG_HISI_DEBUG_FS
	if (hba->inline_debug_flag == DEBUG_LOG_ON)
		dev_err(hba->dev, "%s: key index is %u\n", __func__,
			*crypto_cci);
#endif
#else
	*crypto_cci = lrbp->task_tag;
	spin_lock_irqsave(hba->host->host_lock, *flags);
	hufs_uie_key_prepare(
		hba, *crypto_cci, lrbp->cmd->request->ci_key);
	spin_unlock_irqrestore(hba->host->host_lock, *flags);
#endif
}

static void hufs_req_meta(
	struct ufs_hba *hba, struct hufs_utp_transfer_req_desc *hufs_req_desc,
	u32 *dword, u32 len)
{
#ifdef CONFIG_HISI_UFS_HC
	if (len < META_INPUT_LEN)
		dev_err(hba->dev, "%s: invalid dword len\n", __func__);
	if (ufshcd_is_hufs_hc(hba)) {
		hufs_req_desc->meta_data_random_factor_0 =
			cpu_to_le32(*(dword + 8)); /* dword offset 8 */
		hufs_req_desc->meta_data_random_factor_1 =
			cpu_to_le32(*(dword + 9)); /* dword offset 9 */
		hufs_req_desc->meta_data_random_factor_2 =
			cpu_to_le32(*(dword + 10)); /* dword offset 10 */
		hufs_req_desc->meta_data_random_factor_3 =
			cpu_to_le32(*(dword + 11)); /* dword offset 11 */
	}
#endif
}

static void hufs_get_meta_data_factor(struct ufshcd_lrb *lrbp,
					   struct ufs_hba *hba, u32 *dword_8,
					   u32 *dword_9, u32 *dword_10,
					   u32 *dword_11)
{
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	u32 *metadata_random_factor =
		(u32 *)lrbp->cmd->request->ci_metadata;

	if (metadata_random_factor) {
		/* copy sizeof a dword (4 bytes) for 4 times */
		memcpy(dword_8, metadata_random_factor, 4);
		memcpy(dword_9, metadata_random_factor + 1, 4);
		memcpy(dword_10, metadata_random_factor + 2, 4);
		memcpy(dword_11, metadata_random_factor + 3, 4);
	} else {
		*dword_8 = 0;
		*dword_9 = 0;
		*dword_10 = 0;
		*dword_11 = 0;
	}
#else
	*dword_8 = 0;
	*dword_9 = 0;
	*dword_10 = 0;
	*dword_11 = 0;
#endif
}

/* configure UTRD to enable cryptographic operations for this transaction */
void hufs_uie_utrd_prepare(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
	struct utp_transfer_req_desc *req_desc = NULL;
	struct hufs_utp_transfer_req_desc *hufs_req_desc = NULL;
	u32 dword[TEMP_BUFF_LENGTH] = {0};
	u64 dun;
	u32 crypto_cci;
	unsigned long flags;
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	u32 hash_res;
#endif
	if (!lrbp) {
		pr_err("%s: lrbp addr is NULL\n", __func__);
		return;
	}
	req_desc = lrbp->utr_descriptor_ptr;
	hufs_req_desc = lrbp->hufs_utr_descriptor_ptr;
	/*
	 * According to UFS 2.1 SPEC
	 * decrypte incoming payload if the command is SCSI READ operation
	 * encrypte outgoing payload if the command is SCSI WRITE operation
	 * And HUFS controller only support SCSI cmd as below:
	 * READ_6/READ_10/WRITE_6/WRITE_10
	 */
	if ((lrbp->cmd->cmnd[0] == READ_10) ||
	    (lrbp->cmd->cmnd[0] == WRITE_10) || (lrbp->cmd->cmnd[0] == READ_16))
		dword[12] =
			UTP_REQ_DESC_CRYPTO_ENABLE; /* dword[12] used for DESC */
	else
		return;

	if (lrbp->cmd->request && lrbp->cmd->request->ci_key)
		hufs_crypto_set_keyindex(
			hba, lrbp, &hash_res, &crypto_cci, &flags);
	else
		return;
	dun = (u64)lrbp->cmd->request->bio->index;
#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) &&                         \
	defined(CONFIG_HISI_DEBUG_FS)
	/* dword[12] used for DESC */
	hufs_inline_crypto_debug(hba, &hash_res, &dword[12], dun);
#endif
	/* set val for dword[8],dword[9],dword[10],dword[11] */
	hufs_get_meta_data_factor(lrbp, hba, &dword[8], &dword[9],
				       &dword[10], &dword[11]);

	/* set val for dword */
	dword[0] = dword[12] | crypto_cci;
	dword[1] = (u32)(dun & 0xffffffff);
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO
	dword[3] = (u32)((dun >> 32) | hash_res);
#else
	dword[3] = (u32)((dun >> 32) & 0xffffffff);
#endif
	if (ufshcd_is_hufs_hc(hba)) {
		hufs_req_desc->header.dword_0 |= cpu_to_le32(dword[0]);
		hufs_req_desc->header.dword_1 = cpu_to_le32(dword[1]);
		hufs_req_desc->header.dword_3 = cpu_to_le32(dword[3]);
	} else {
		req_desc->header.dword_0 |= cpu_to_le32(dword[0]);
		req_desc->header.dword_1 = cpu_to_le32(dword[1]);
		req_desc->header.dword_3 = cpu_to_le32(dword[3]);
	}
	hufs_req_meta(hba, hufs_req_desc, dword, sizeof(dword));
}
#endif

static int hufs_hce_enable_notify(
	struct ufs_hba *hba, enum ufs_notify_change_status status)
{
	if (status == PRE_CHANGE)
		hufs_pre_hce_notify(hba);

	return 0;
}

#ifndef CONFIG_HISI_UFS_HC
void hufs_mphy_busdly_config(struct ufs_hba *hba, struct hufs_host *host)
{
	uint32_t reg = 0;

	if (!host) {
		pr_err("%s: host is NULL\n", __func__);
		return;
	}
	if (host->caps & USE_HUFS_MPHY_TC) {
		/* UFS_BUSTHRTL_OFF BIT[18:23] */
		reg = ufshcd_readl(hba, UFS_REG_OCPTHRTL);
		reg &= (~((0x3f << 18) | 0xff));
		reg |= ((0x10 << 18) | 0xff);
		ufshcd_writel(hba, reg, UFS_REG_OCPTHRTL);
		(void)ufshcd_readl(hba, UFS_REG_OCPTHRTL);
	}
}
#endif

static int hufs_link_startup_notify(
	struct ufs_hba *hba, enum ufs_notify_change_status status)
{
	int err = 0;

	switch (status) {
	case PRE_CHANGE:
		err = hufs_link_startup_pre_change(hba);
		break;
	case POST_CHANGE:
		err = hufs_link_startup_post_change(hba);
		break;
	default:
		pr_err("%s: enter default branch)\n", __func__);
		break;
	}

	return err;
}

static int hufs_get_pwr_dev_param(
	struct hufs_dev_params *hufs_param,
	struct ufs_pa_layer_attr *dev_max, struct ufs_pa_layer_attr *agreed_pwr)
{
	int min_hufs_gear;
	int min_dev_gear;
	bool is_dev_sup_hs = false;
	bool is_hufs_max_hs = false;

	if (dev_max->pwr_rx == FASTAUTO_MODE || dev_max->pwr_rx == FAST_MODE)
		is_dev_sup_hs = true;

	if (hufs_param->desired_working_mode == FAST) {
		is_hufs_max_hs = true;
		min_hufs_gear = min_t(
			u32, hufs_param->hs_rx_gear, hufs_param->hs_tx_gear);
	} else {
		min_hufs_gear = min_t(u32, hufs_param->pwm_rx_gear,
			hufs_param->pwm_tx_gear);
	}

	/*
	 * device doesn't support HS but hufs_param->desired_working_mode is
	 * HS, thus device and hufs_param don't agree
	 */
	if (!is_dev_sup_hs && is_hufs_max_hs) {
		pr_err("%s: failed to agree on power mode (device doesn't "
		       "support HS but requested power is HS)\n", __func__);
		return -ENOTSUPP;
	} else if (is_dev_sup_hs && is_hufs_max_hs) {
		/*
		 * since device supports HS, it supports FAST_MODE.
		 * since hufs_param->desired_working_mode is also HS
		 * then final decision (FAST/FASTAUTO) is done according
		 * to hufs_params as it is the restricting factor
		 */
		agreed_pwr->pwr_rx = agreed_pwr->pwr_tx =
			hufs_param->rx_pwr_hs;
	} else {
		/*
		 * here hufs_param->desired_working_mode is PWM.
		 * it doesn't matter whether device supports HS or PWM,
		 * in both cases hufs_param->desired_working_mode will
		 * determine the mode
		 */
		agreed_pwr->pwr_rx = agreed_pwr->pwr_tx =
			hufs_param->rx_pwr_pwm;
	}

	/*
	 * we would like tx to work in the minimum number of lanes
	 * between device capability and vendor preferences.
	 * the same decision will be made for rx
	 */
	agreed_pwr->lane_tx =
		min_t(u32, dev_max->lane_tx, hufs_param->tx_lanes);
	agreed_pwr->lane_rx =
		min_t(u32, dev_max->lane_rx, hufs_param->rx_lanes);

	/* device maximum gear is the minimum between device rx and tx gears */
	min_dev_gear = min_t(u32, dev_max->gear_rx, dev_max->gear_tx);

	/*
	 * if both device capabilities and vendor pre-defined preferences are
	 * both HS or both PWM then set the minimum gear to be the chosen
	 * working gear.
	 * if one is PWM and one is HS then the one that is PWM get to decide
	 * what is the gear, as it is the one that also decided previously what
	 * pwr the device will be configured to.
	 */
	if ((is_dev_sup_hs && is_hufs_max_hs) ||
		(!is_dev_sup_hs && !is_hufs_max_hs))
		agreed_pwr->gear_rx = agreed_pwr->gear_tx =
			(u32)min_t(u32, min_dev_gear, min_hufs_gear);
	else
		agreed_pwr->gear_rx = agreed_pwr->gear_tx = (u32)min_hufs_gear;

	agreed_pwr->hs_rate = hufs_param->hs_rate;

	pr_err("ufs final power mode: gear = %u, lane = %u, pwr = %u, rate = %u\n",
		agreed_pwr->gear_rx, agreed_pwr->lane_rx, agreed_pwr->pwr_rx,
		agreed_pwr->hs_rate);

	return 0;
}

static void hufs_cap_fill(
	struct hufs_host *host, struct hufs_dev_params *hufs_cap)
{
	if (host->caps & USE_HS_GEAR4) {
		hufs_cap->hs_rx_gear = UFS_HS_G4;
		hufs_cap->hs_tx_gear = UFS_HS_G4;
		hufs_cap->desired_working_mode = FAST;
	} else if (host->caps & USE_HS_GEAR3) {
		hufs_cap->hs_rx_gear = UFS_HS_G3;
		hufs_cap->hs_tx_gear = UFS_HS_G3;
		hufs_cap->desired_working_mode = FAST;
	} else if (host->caps & USE_HS_GEAR2) {
		hufs_cap->hs_rx_gear = UFS_HS_G2;
		hufs_cap->hs_tx_gear = UFS_HS_G2;
		hufs_cap->desired_working_mode = FAST;
	} else if (host->caps & USE_HS_GEAR1) {
		hufs_cap->hs_rx_gear = UFS_HS_G1;
		hufs_cap->hs_tx_gear = UFS_HS_G1;
		hufs_cap->desired_working_mode = FAST;
	} else {
		hufs_cap->hs_rx_gear = 0;
		hufs_cap->hs_tx_gear = 0;
		hufs_cap->desired_working_mode = SLOW;
	}
}

static void hufs_cap_complet(
	struct ufs_hba *hba, struct hufs_host *host,
	struct hufs_dev_params *hufs_cap)
{
	if (host->caps & USE_ONE_LANE) {
		hufs_cap->tx_lanes = 1; /* ufs capability use one lane */
		hufs_cap->rx_lanes = 1;
	} else {
		hufs_cap->tx_lanes = 2; /* ufs capability use 2 lane */
		hufs_cap->rx_lanes = 2; /* ufs capability use 2 lane */
	}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	if (hba->hs_single_lane) {
		hufs_cap->tx_lanes = 1; /* ufs capability use one lane */
		hufs_cap->rx_lanes = 1;
	}
#endif
	hufs_cap_fill(host, hufs_cap);

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	if (hba->use_pwm_mode)
		hufs_cap->desired_working_mode = SLOW;
#endif
	hufs_cap->pwm_rx_gear = HUFS_LIMIT_PWMGEAR_RX;
	hufs_cap->pwm_tx_gear = HUFS_LIMIT_PWMGEAR_TX;
	hufs_cap->rx_pwr_pwm = HUFS_LIMIT_RX_PWR_PWM;
	hufs_cap->tx_pwr_pwm = HUFS_LIMIT_TX_PWR_PWM;
	/* hynix not support fastauto now */
	if (host->caps & BROKEN_FASTAUTO) {
		hufs_cap->rx_pwr_hs = FAST_MODE;
		hufs_cap->tx_pwr_hs = FAST_MODE;
	} else {
		hufs_cap->rx_pwr_hs = FASTAUTO_MODE;
		hufs_cap->tx_pwr_hs = FASTAUTO_MODE;
	}

	if (host->caps & USE_RATE_B)
		hufs_cap->hs_rate = PA_HS_MODE_B;
	else
		hufs_cap->hs_rate = PA_HS_MODE_A;
}

/* dev_max_params maybe null do not judge */
static int hufs_pwr_change_notify(
	struct ufs_hba *hba, enum ufs_notify_change_status status,
	struct ufs_pa_layer_attr *dev_max_params,
	struct ufs_pa_layer_attr *dev_req_params)
{
	struct hufs_dev_params hufs_cap;
	struct hufs_host *host = NULL;
	int ret = 0;

	if (!dev_req_params) {
		dev_err(hba->dev, "%s: incoming dev_req_params is NULL\n", __func__);
		return -EINVAL;
	}
	host = hba->priv;

	if (status == PRE_CHANGE) {
		hufs_cap_complet(hba, host, &hufs_cap);
		ret = hufs_get_pwr_dev_param(
			&hufs_cap, dev_max_params, dev_req_params);
		if (ret) {
			dev_err(hba->dev, "%s: failed to determine capabilities\n",
				__func__);
			return ret;
		}
		/* for hufs MPHY */
		deemphasis_config(host, hba, dev_req_params);
		if (host->caps & USE_HUFS_MPHY_TC) {
			if (!is_v200_mphy(hba))
				adapt_pll_to_power_mode(hba);
		}

		hufs_pwr_change_pre_change(hba, dev_req_params);
	}

	return ret;
}

static int hufs_get_common(struct hufs_host *host)
{
	struct device_node *np = NULL;

	if (!host->ufs_sys_ctrl) {
		dev_err(host->hba->dev,
			"cannot ioremap for ufs sys ctrl register\n");
		return -ENOMEM;
	}
	np = of_find_compatible_node(NULL, NULL, "hisilicon,crgctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,crgctrl\"\n");
		return -ENXIO;
	}
	host->pericrg = of_iomap(np, 0);
	if (!host->pericrg) {
		dev_err(host->hba->dev, "crgctrl iomap error!\n");
		return -ENOMEM;
	}
	np = of_find_compatible_node(NULL, NULL, "hisilicon,pctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,pctrl\"\n");
		return -ENXIO;
	}
	host->pctrl = of_iomap(np, 0);
	if (!host->pctrl) {
		dev_err(host->hba->dev, "pctrl iomap error!\n");
		return -ENOMEM;
	}
	np = of_find_compatible_node(NULL, NULL, "hisilicon,pmctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,pmctrl\"\n");
		return -ENXIO;
	}
	host->pmctrl = of_iomap(np, 0);
	if (!host->pmctrl) {
		dev_err(host->hba->dev, "pmctrl iomap error!\n");
		return -ENOMEM;
	}
	np = of_find_compatible_node(NULL, NULL, "hisilicon,sysctrl");
	if (!np) {
		dev_err(host->hba->dev,
			"can't find device node \"hisilicon,sysctrl\"\n");
		return -ENXIO;
	}
	host->sysctrl = of_iomap(np, 0);
	if (!host->sysctrl) {
		dev_err(host->hba->dev, "sysctrl iomap error!\n");
		return -ENOMEM;
	}

	return 0;
}

int hufs_get_resource(struct hufs_host *host)
{
	struct resource *mem_res = NULL;
	struct device *dev = NULL;
	struct platform_device *pdev = NULL;

	if (!host) {
		pr_err("host is NULL\n");
		return -EINVAL;
	}
	dev = host->hba->dev;
	pdev = to_platform_device(dev);
	/* get resource of ufs sys ctrl */
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	host->ufs_sys_ctrl = devm_ioremap_resource(dev, mem_res);

	if (hufs_get_common(host))
		return -ENOMEM;

#ifdef CONFIG_HISI_UFS_HC
	if (ufs_get_hufs_hc(host))
		return -ENOMEM;

#endif
	/* we only use 64 bit dma */
	dev->dma_mask = &hufs_dma_mask;

	return 0;
}

static ssize_t hufs_inline_stat_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	struct ufs_hba *hba = dev_get_drvdata(dev);
#endif
	int ret_show = 0;

#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	if (ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES) &
		MASK_INLINE_ENCRYPTO_SUPPORT)
#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
		ret_show = 3; /* inline crypto v3 */
#elif defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2)
		ret_show = 2; /* inline crypto v2 */
#else
		ret_show = 1; /* inline crypto v1 */
#endif
#endif

	return snprintf(buf, PAGE_SIZE, "%d\n",
		ret_show); /* unsafe_function_ignore: snprintf */
}

void hufs_inline_crypto_attr(struct ufs_hba *hba)
{
	hba->inline_state.inline_attr.show = hufs_inline_stat_show;
	sysfs_attr_init(&hba->inline_state.inline_attr.attr);
	hba->inline_state.inline_attr.attr.name = "ufs_inline_stat";
	hba->inline_state.inline_attr.attr.mode = S_IRUSR | S_IRGRP;
	if (device_create_file(hba->dev, &hba->inline_state.inline_attr))
		dev_err(hba->dev,
			"Failed to create sysfs for ufs_inline_state\n");
}

#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) &&                         \
	defined(CONFIG_HISI_DEBUG_FS)
static ssize_t hufs_inline_debug_show(
	struct device *dev, struct device_attribute *attr, char *buf)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (hba->inline_debug_flag == DEBUG_LOG_ON ||
	    hba->inline_debug_flag == DEBUG_CRYPTO_ON) {
		return snprintf(buf, PAGE_SIZE, "%s\n",
				"on"); /* unsafe_function_ignore: snprintf */
	} else {
		return snprintf(buf, PAGE_SIZE, "%s\n",
				"off"); /* unsafe_function_ignore: snprintf */
	}
}

static ssize_t hufs_inline_debug_store(
	struct device *dev, struct device_attribute *attr, const char *buf,
	size_t count)
{
	struct ufs_hba *hba = dev_get_drvdata(dev);

	if (sysfs_streq(buf, "off")) {
		hba->inline_debug_flag = DEBUG_LOG_OFF;
	} else if (sysfs_streq(buf, "on")) {
		hba->inline_debug_flag = DEBUG_LOG_ON;
	} else if (sysfs_streq(buf, "crypto_on")) {
		hba->inline_debug_flag = DEBUG_CRYPTO_ON;
	} else if (sysfs_streq(buf, "crypto_off")) {
		hba->inline_debug_flag = DEBUG_CRYPTO_OFF;
	} else {
		dev_err(hba->dev, "%s: invalid input debug parameter.\n", __func__);
		return -EINVAL;
	}

	return count;
}

static ssize_t hufs_inline_dun_cci_test(
	struct device *dev, struct device_attribute *attr, const char *buf,
	size_t count)
{
	int i;
#define UFS_CCI_KEY_LEN 65
	char buf_temp[UFS_CCI_KEY_LEN] = {0};

	if (count != UFS_CCI_KEY_LEN) {
		dev_err(dev, "%s: the input key len is not 64.\n", __func__);
		return count;
	}

	for (i = 0; i < (UFS_CCI_KEY_LEN - 1); i++)
		buf_temp[i] = buf[i];

	buf_temp[UFS_CCI_KEY_LEN - 1] = '\0';
	dev_err(dev, "%s: input key is %s\n", __func__, buf_temp);
	/* bkdr hash length 64 */
	test_generate_cci_dun_use_bkdrhash((u8 *)buf_temp, 64);

	return count;
}

static void hufs_inline_crypto_debug_init(struct ufs_hba *hba)
{
	hba->inline_debug_flag = DEBUG_LOG_OFF;

	hba->inline_debug_state.inline_attr.show = hufs_inline_debug_show;
	hba->inline_debug_state.inline_attr.store =
		hufs_inline_debug_store;
	sysfs_attr_init(&hba->inline_debug_state.inline_attr.attr);
	hba->inline_debug_state.inline_attr.attr.name = "ufs_inline_debug";
	hba->inline_debug_state.inline_attr.attr.mode = 0640; /* 0640 node attribute mode */
	if (device_create_file(hba->dev, &hba->inline_debug_state.inline_attr))
		dev_err(hba->dev,
			"Failed to create sysfs for inline_debug_state\n");

	hba->inline_dun_cci_test.inline_attr.store =
		hufs_inline_dun_cci_test;
	sysfs_attr_init(&hba->inline_dun_cci_test.inline_attr.attr);
	hba->inline_dun_cci_test.inline_attr.attr.name =
		"ufs_inline_dun_cci_test";
	hba->inline_dun_cci_test.inline_attr.attr.mode = 0200; /* 0200 node attribute mode */
	if (device_create_file(hba->dev, &hba->inline_dun_cci_test.inline_attr))
		dev_err(hba->dev,
			"Failed to create sysfs for inline_dun_cci_test\n");
}
#endif

void hufs_set_pm_lvl(struct ufs_hba *hba)
{
	hba->rpm_lvl = UFS_PM_LVL_1;
	hba->spm_lvl = UFS_PM_LVL_3;
}

/**
 * hufs_advertise_quirks - advertise the known HUFS controller quirks
 * @hba: host controller instance
 *
 * HUFS host controller might have some non standard behaviours (quirks)
 * than what is specified by UFSHCI specification. Advertise all such
 * quirks to standard UFS host controller driver so standard takes them into
 * account.
 */
void hufs_advertise_quirks(struct ufs_hba *hba)
{
}

static void hufs_populate_caps_dt(
	struct device_node *np, struct hufs_host *host)
{
	unsigned int idx;
	struct st_caps_map caps_arry[] = {
		{ "ufs-kirin-use-rate-B", USE_RATE_B },
		{ "ufs-kirin-broken-fastauto", BROKEN_FASTAUTO },
		{ "ufs-kirin-use-one-line", USE_ONE_LANE },
		{ "ufs-kirin-use-HS-GEAR4", USE_HS_GEAR4 },
		{ "ufs-kirin-use-HS-GEAR3", USE_HS_GEAR3 },
		{ "ufs-kirin-use-HS-GEAR2", USE_HS_GEAR2 },
		{ "ufs-kirin-use-HS-GEAR1", USE_HS_GEAR1 },
		{ "ufs-kirin-use-hisi-mphy-tc", USE_HUFS_MPHY_TC },
		{ "ufs-kirin-use-main-resume", USE_MAIN_RESUME },
		{ "ufs-kirin-broken-clk-gate-bypass", BROKEN_CLK_GATE_BYPASS },
		{ "ufs-kirin-rx-cannot-disable", RX_CANNOT_DISABLE },
		{ "ufs-kirin-rx-vco-vref", RX_VCO_VREF },
		{ "ufs-mphy-cdr-reset-workround",
		  USE_MPHY_CDR_RESET_WORKROUND },
		{ "ufs-mphy-auto-k-workround", USE_MPHY_AUTO_K_WORKROUND },
		{ "ufs-mphy-sense-amp-ldo-vol-workround",
		  USE_MPHY_LDO_VOL_WORKROUND },
		{ "ufs-mphy-cdr-bw-workround", USE_MPHY_CDR_BW_WORKROUND },
		{ "ufs-rpmb-pm-runtime-optimal-delay", RPMB_PM_RUNTIME_OPTIMAL_DELAY },
		{ "ufs-kirin-use-two-line", USE_TWO_LANE },
	};

	for (idx = 0; idx < sizeof(caps_arry) / sizeof(struct st_caps_map);
		idx++) {
		if (of_find_property(np, caps_arry[idx].caps_name, NULL))
			host->caps |= caps_arry[idx].cap_bit;
	}
	/* we use two lane np,clear one lane np */
	if (host->caps & USE_TWO_LANE)
		host->caps &= BIT_1LANE_MASK; /* bit3 is 1 lane mask */
}

static void hufs_populate_quirks_dt(
	struct device_node *np, struct hufs_host *host)
{
	if (of_find_property(np, "ufs-kirin-unipro-termination", NULL))
		host->hba->quirks |= UFSHCD_QUIRK_UNIPRO_TERMINATION;

	if (of_find_property(np, "ufs-kirin-unipro-scrambing", NULL))
		host->hba->quirks |= UFSHCD_QUIRK_UNIPRO_SCRAMBLING;
}

static void ufs_hw_populate_dt(
	struct device_node *np, struct hufs_host *host)
{
	int ret;

	ret = of_property_match_string(
		np, "ufs-gear3-product-names", ufs_product_name);
	if (ret >= 0) {
		host->caps &= ~USE_HS_GEAR4;
		host->caps |= USE_HS_GEAR3;
	}

	ret = of_property_match_string(
		np, "ufs-device-config-product-names", ufs_product_name);
	if (ret >= 0)
		host->hba->quirks |= UFSHCD_QUIRK_DEVICE_CONFIG;
}

#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
static void hufs_populate_mgc_dt(
	struct device_node *parent_np, struct hufs_host *host)
{
	struct device_node *child_np = NULL;
	struct device_model_para model_para;
	unsigned int man_id;
	int ret;
	int is_white;
	struct hufs_bkops_id *bkops_id = NULL;

	INIT_LIST_HEAD(&host->hba->bkops_whitelist);
	INIT_LIST_HEAD(&host->hba->bkops_blacklist);
	for_each_child_of_node (parent_np, child_np) {
		ret = of_property_read_string(
			child_np, "compatible",
			(const char **)(&(model_para.compatible)));
		if (ret) {
			dev_err(host->hba->dev, "check the compatible %s\n",
				child_np->name);
			continue;
		} else {
			if (!strcmp("white",
				    model_para.compatible)) {
				is_white = 1;
			} else if (!strcmp("black",
					   model_para.compatible)) {
				is_white = 0;
			} else {
				dev_err(host->hba->dev,
					"check the compatible %s\n",
					child_np->name);
				continue;
			}
		}
		ret = of_property_read_u32(child_np, "manufacturer_id",
					   &man_id);
		if (ret) {
#ifdef CONFIG_HISI_DEBUG_FS
			dev_err(host->hba->dev,
				"check the manufacturer_id %s\n",
				child_np->name);
#endif
			continue;
		}

		ret = of_property_read_string(
			child_np, "model",
			(const char **)(&(model_para.model)));
		if (ret) {
			dev_err(host->hba->dev, "check the model %s\n",
				child_np->name);
			continue;
		}

		ret = of_property_read_string(
			child_np, "rev", (const char **)(&(model_para.rev)));
		if (ret) {
			dev_err(host->hba->dev, "check the rev %s\n",
				child_np->name);
			continue;
		}

		bkops_id = devm_kzalloc(host->hba->dev, sizeof(*bkops_id),
					GFP_KERNEL);
		if (!bkops_id) {
			dev_err(host->hba->dev,
				"%s %d Failed to alloc bkops_id\n", __func__,
				__LINE__);
			return;
		}

		bkops_id->manufacturer_id = man_id;
		bkops_id->ufs_model = model_para.model;
		bkops_id->ufs_rev = model_para.rev;
		INIT_LIST_HEAD(&bkops_id->p);
		if (is_white)
			list_add(&bkops_id->p, &(host->hba)->bkops_whitelist);
		else
			list_add(&bkops_id->p, &(host->hba)->bkops_blacklist);
	}
}
#endif /* CONFIG_HISI_UFS_MANUAL_BKOPS */

static void hufs_populate_property(
	struct hufs_host *host, struct device_node *np)
{
	hufs_populate_caps_dt(np, host);
	ufs_hw_populate_dt(np, host);
#ifdef CONFIG_HISI_UFS_MANUAL_BKOPS
	hufs_populate_mgc_dt(np, host);
#endif
#ifdef CONFIG_SCSI_UFS_KIRIN_LINERESET_CHECK
	if (of_find_property(np, "ufs-kirin-linereset-check-disable", NULL))
		host->hba->bg_task_enable = false;
	else
		host->hba->bg_task_enable = true;
#endif

	if (of_find_property(np, "ufs-kirin-use-auto-H8", NULL))
		host->hba->caps |= UFSHCD_CAP_AUTO_HIBERN8;

	if (of_find_property(np, "ufs-kirin-pwm-daemon-intr", NULL))
		host->hba->caps |= UFSHCD_CAP_PWM_DAEMON_INTR;

	if (of_find_property(np, "ufs-kirin-dev-tmt-intr", NULL))
		host->hba->caps |= UFSHCD_CAP_DEV_TMT_INTR;

	if (of_find_property(np, "ufs-kirin-broken-idle-intr", NULL))
		host->hba->caps |= UFSHCD_CAP_BROKEN_IDLE_INTR;

	if (of_find_property(np, "ufs-kirin-wb", NULL))
		host->hba->caps |= UFSHCD_CAP_WRITE_BOOSTER;

	hufs_populate_quirks_dt(np, host);

	if (of_find_property(np, "ufs-kirin-ssu-by-self", NULL))
		host->hba->caps |= UFSHCD_CAP_SSU_BY_SELF;

	if (of_find_property(np, "ufs-on-emulator", NULL))
		host->hba->host->is_emulator = 1;
	else
		host->hba->host->is_emulator = 0;
}

void hufs_populate_dt(struct device *dev, struct hufs_host *host)
{
	int ret;
	struct device_node *np = NULL;

	if (!dev) {
		pr_err("dev is NULL");
		return;
	}

	if (!host) {
		dev_err(dev, "host is NULL\n");
		return;
	}

	np = dev->of_node;

	if (!np) {
		dev_err(dev, "can not find device node\n");
		return;
	}

	hufs_populate_property(host, np);
	ret = of_property_match_string(
		np, "ufs-0db-equalizer-product-names", ufs_product_name);
	if (ret >= 0) {
#ifdef CONFIG_HISI_DEBUG_FS
		dev_info(dev, "find %s in dts\n", ufs_product_name);
#endif
		host->tx_equalizer = 0;
	} else {
#ifdef UFS_TX_EQUALIZER_0DB
		host->tx_equalizer = 0;
#endif
#ifdef UFS_TX_EQUALIZER_35DB
		host->tx_equalizer = TX_EQUALIZER_35DB;
#endif
#ifdef UFS_TX_EQUALIZER_60DB
		host->tx_equalizer = 60;
#endif
	}

	/* ufs reset retry num */
	if (of_property_read_s32(
		    np, "reset_retry_max", &(host->hba->reset_retry_max))) {
		host->hba->reset_retry_max = MAX_HOST_RESET_RETRIES;
		dev_info(dev, "can not find retry max, use default val\n");
	}

	host->hba->ufs_reset_retries = host->hba->reset_retry_max;
	host->hba->ufs_init_retries = MAX_HOST_INIT_RETRIES;
	host->hba->manufacturer_id = 0;

	if (of_find_property(np, "broken-hce", NULL))
		host->hba->quirks |= UFSHCD_QUIRK_BROKEN_HCE;
#ifdef CONFIG_MAS_BLK
	if (of_find_property(np, "IO_latency", NULL))
		host->hba->host->queue_quirk_flag |=
			SHOST_QUIRK(SHOST_QUIRK_IO_LATENCY_PROTECTION);
#endif
}

bool ufshcd_is_hufs_hc(struct ufs_hba *hba)
{
	if (hba->use_hufs_hc)
		return true;
	return false;
}

/*
 * Samsumg UFS 3.1 device adapt bugfix. After switch to G4 (adapt open), close
 * adapt, switch to G4 again.
 */
int ufshcd_adapt_workaround(struct ufs_hba *hba,
				   struct ufs_pa_layer_attr *pwr_mode)
{
	if (!ufshcd_is_hufs_hc(hba))
		return 0;

	if (hba->manufacturer_id != UFS_VENDOR_SAMSUNG ||
	    hba->ufs_device_spec_version != UFS_DEVICE_SPEC_3_1)
		return 0;

	if ((pwr_mode->pwr_rx == FASTAUTO_MODE ||
	     pwr_mode->pwr_rx == FAST_MODE) &&
	    (pwr_mode->gear_rx >= UFS_HS_G4)) {
		dev_info(hba->dev, "ufs adapt closed\n");
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXHSADAPTTYPE),
			       PA_HS_APT_NO);
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_ADAPTAFTERLRSTINPA_INIT),
			       PA_HS_APT_NO);
		return ufshcd_uic_change_pwr_mode(
			hba, pwr_mode->pwr_rx << 4 | pwr_mode->pwr_tx);
	}

	return 0;
}

void sel_equalizer_by_device(struct ufs_hba *hba, u32 *equalizer)
{
	int ret;
	struct device *dev = NULL;
	struct device_node *np = NULL;

	if (!equalizer) {
		dev_err(hba->dev, "equalizer addr is NULL\n");
		return;
	}

	dev = hba->dev;
	np = dev->of_node;

	ret = of_property_match_string(
		np, "ufs-35db-equalizer-product-names", ufs_product_name);
	if (ret >= 0) {
#ifdef CONFIG_HISI_DEBUG_FS
		dev_err(dev, "%s found in 3.5db product names\n",
			ufs_product_name);
#endif
		*equalizer = TX_EQUALIZER_35DB; /* 35DB */
	}
}

static int hufs_i2c_config(
	struct ufs_hba *hba, struct hufs_host *host)
{
	int err = 0;
	struct device *dev = hba->dev;

	if ((host->caps & USE_HUFS_MPHY_TC) && !host->i2c_client) {
		i2c_chipsel_gpio_config(host, dev);
		err = create_i2c_client(hba);
		if (err) {
			dev_err(dev, "create i2c client error\n");
			return err;
		}
	}
	return err;
}

/**
 * hufs_init
 * @hba: host controller instance
 */
static int hufs_init(struct ufs_hba *hba)
{
	int err;
	struct device *dev = NULL;
	struct hufs_host *host = NULL;

	dev = hba->dev;
#ifdef CONFIG_HISI_BOOTDEVICE
	if (get_bootdevice_type() == BOOT_DEVICE_UFS)
		set_bootdevice_name(dev);
#endif

	host = devm_kzalloc(dev, sizeof(*host), GFP_KERNEL);
	if (!host) {
		err = -ENOMEM;
		dev_err(dev, "%s: no memory for hufs host\n", __func__);
		goto out;
	}

	host->hba = hba;
	hba->priv = (void *)host;

	host->clk_ufsio_ref = devm_clk_get(dev, "clk_ufsio_ref");
	if (IS_ERR(host->clk_ufsio_ref)) {
		err = PTR_ERR(host->clk_ufsio_ref);
		dev_err(dev, "clk_ufsio_ref not found.\n");
		goto out;
	}

	hufs_advertise_quirks(hba);
	hufs_set_pm_lvl(hba);
	hufs_populate_dt(dev, host);
	err = hufs_i2c_config(hba, host);
	if (err)
		goto host_free;

	err = hufs_get_resource(host);
	if (err)
		goto host_free;
	hufs_regulator_init(hba);
	ufs_clk_init(hba);
	ufs_soc_init(hba);
	ufshcd_host_ops_register(hba);
#ifdef CONFIG_HISI_BOOTDEVICE
	if (get_bootdevice_type() == BOOT_DEVICE_UFS) {
#endif
#ifdef CONFIG_RPMB_UFS
		rpmb_ufs_init(hba->host->is_emulator);
#endif
		hufs_inline_crypto_attr(hba);
#if defined(CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO) &&                         \
	defined(CONFIG_HISI_DEBUG_FS)
		hufs_inline_crypto_debug_init(hba);
#endif
#ifdef CONFIG_HISI_BOOTDEVICE
	}
#endif

	goto out;

host_free:
	hba->priv = NULL;
out:
	return err;
}

void hufs_exit(struct ufs_hba *hba)
{
#ifdef CONFIG_HISI_UFS_HC_CORE_UTR
	ufshcd_irq_cpuhp_remove(hba);
#endif
}

#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
int hufs_get_pwr_by_sysctrl(struct ufs_hba *hba)
{
	struct hufs_host *host = NULL;
	u32 link_state;
	u32 lane0_rx_state;

	host = (struct hufs_host *)hba->priv;
	/* tx rx rtcontrl enable */
	ufs_sys_ctrl_writel(host, 0x08081010, PHY_MPX_TEST_CTRL);
	/* need excecue in order */
	wmb();
	link_state = ufs_sys_ctrl_readl(host, PHY_MPX_TEST_OBSV);
	/* bit8-bit10 lane0 tx state */
	lane0_rx_state = (link_state & (0x7 << 8)) >> 8;
	/* rx state 2 slow mode, 3 means fastmode */
	if (lane0_rx_state == 2)
		return SLOW;
	else if (lane0_rx_state == 3)
		return FAST;
	else
		return -1;
}
#endif

bool is_v200_mphy(struct ufs_hba *hba)
{
	u32 reg = 0;

	/* V200 memorymap is not equal to V120 */
	ufs_i2c_readl(hba, &reg, REG_SC_APB_IF_V200);
	dev_info(hba->dev, "UFS MPHY  %s\n",
		(reg == MPHY_BOARDID_V200) ? "V200" : "V120");
	if (reg == MPHY_BOARDID_V200)
		return true;
	else
		return false;
}

/*
 * some ufs devices need to be powered off then on when they are reset or
 * initialized.
 */
void hufs_vcc_power_on_off(struct ufs_hba *hba)
{
	unsigned int value;
	int reg_offset;

	if ((hba->manufacturer_id != UFS_VENDOR_HI1861) &&
		(hba->manufacturer_id != 0))
		return;

#ifdef CONFIG_SCSI_UFS_LIBRA
	reg_offset = PMIC_LDO15_ONOFF_ADDR(0);
#else
	reg_offset = PMIC_LDO15_ONOFF_ECO_ADDR(0);
#endif

	if ((hba->ufs_init_retries < MAX_HOST_INIT_RETRIES) ||
		(hba->ufs_reset_retries < hba->reset_retry_max) ||
		(hba->curr_dev_pwr_mode == UFS_POWERDOWN_PWR_MODE)) {
		udelay(10); /* wait 10us */
		value = pmic_read_reg(reg_offset);
		pmic_write_reg(value & ~BIT(0), reg_offset);
		mdelay(16); /* wait 16ms */
		pmic_write_reg(value | BIT(0), reg_offset);
		udelay(200); /* wait 200us */
		dev_err(hba->dev, "ufs vcc power off then on.\n");
	}
}

/*
 * struct ufs_hba_hufs_vops - HUFS specific variant operations
 * The variant operations configure the necessary controller and PHY
 * handshake during initialization.
 */
const struct ufs_hba_variant_ops ufs_hba_hufs_vops = {
	.name = "kirin",
	.init = hufs_init,
	.exit = hufs_exit,
	.setup_clocks = NULL,
	.hce_enable_notify = hufs_hce_enable_notify,
	.link_startup_notify = hufs_link_startup_notify,
#ifdef CONFIG_HISI_UFS_HC
	.hufs_dme_link_startup = hufs_dme_link_startup,
	.hufs_uic_write_reg = hufs_uic_write_reg,
	.hufs_uic_read_reg = hufs_uic_read_reg,
	.hufs_uic_peer_set = hufs_uic_peer_set,
	.hufs_uic_peer_get = hufs_uic_peer_get,
	.ufshcd_hufs_disable_auto_hibern8 = ufshcd_hufs_disable_auto_hibern8,
	.ufshcd_hufs_enable_auto_hibern8 = ufshcd_hufs_enable_auto_hibern8,
	.ufshcd_hibern8_op_irq_safe = ufshcd_hibern8_op_irq_safe,
	.ufshcd_hufs_host_memory_configure = ufshcd_hufs_host_memory_configure,
	.ufshcd_hufs_uic_change_pwr_mode = ufshcd_hufs_uic_change_pwr_mode,
	.snps_to_hufs_addr = snps_to_hufs_addr,
	.dbg_hufs_dme_dump = dbg_hufs_dme_dump,
#endif
	.pwr_change_notify = hufs_pwr_change_notify,
	.full_reset = hufs_full_reset,
	.device_reset = hufs_device_hw_reset,
	.set_ref_clk = set_device_clk,
	.suspend_before_set_link_state =
		hufs_suspend_before_set_link_state,
	.suspend = hufs_suspend,
	.resume = hufs_resume,
	.resume_after_set_link_state = hufs_resume_after_set_link_state,
#ifdef CONFIG_SCSI_UFS_INLINE_CRYPTO
	.uie_config_init = hufs_uie_config_init,
	.uie_utrd_pre = hufs_uie_utrd_prepare,
#endif
	.dbg_hci_dump = hufs_hci_log,
	.dbg_uic_dump = hufs_uic_log,
#ifdef CONFIG_SCSI_UFS_KIRIN_LINERESET_CHECK
	.background_thread = hufs_daemon_thread,
#endif
#ifdef CONFIG_SCSI_UFS_HS_ERROR_RECOVER
	.get_pwr_by_debug_register = hufs_get_pwr_by_sysctrl,
#endif
	.vcc_power_on_off = hufs_vcc_power_on_off,
	.get_device_info = hufs_get_device_info,
};
EXPORT_SYMBOL(ufs_hba_hufs_vops);

static int __init uie_open_session_late(void)
{
	int err = 0;

#ifdef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V2
#ifndef CONFIG_SCSI_UFS_ENHANCED_INLINE_CRYPTO_V3
	err = uie_open_session();
	if (err)
		BUG();
#endif
#endif

	return err;
}

late_initcall(uie_open_session_late);
