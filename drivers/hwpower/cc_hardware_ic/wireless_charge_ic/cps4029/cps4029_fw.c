/*
 * cps4029_fw.c
 *
 * cps4029 mtp, sram driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include "cps4029.h"
#include "cps4029_mtp.h"

#define HWLOG_TAG wireless_cps4029_fw
HWLOG_REGIST();

static int cps4029_fw_unlock_i2c(struct cps4029_dev_info *di)
{
	return cps4029_aux_write_word(di, CPS4029_CMD_UNLOCK_I2C, CPS4029_I2C_CODE);
}

static int cps4029_fw_set_hi_addr(struct cps4029_dev_info *di, u32 hi_addr)
{
	return cps4029_aux_write_word(di, CPS4029_CMD_SET_HI_ADDR, hi_addr);
}

static int cps4029_fw_set_increase_mode(struct cps4029_dev_info *di, u16 mode)
{
	return cps4029_aux_write_word(di, CPS4029_CMD_INC_MODE, mode);
}

static int cps4029_fw_write_block(struct cps4029_dev_info *di, u32 addr, u8 *cmd, int len)
{
	int ret;
	u16 low_addr = addr & 0xFFFF; /* low 16bit addr */
	u16 hi_addr = addr >> 16; /* high 16bit addr */

	ret = cps4029_fw_set_hi_addr(di, hi_addr);
	if (ret)
		return ret;

	ret = cps4029_write_block(di, low_addr, cmd, len);
	if (ret)
		return ret;

	return 0;
}

static int cps4029_fw_read_block(struct cps4029_dev_info *di, u32 addr, u8 *data, int len)
{
	int ret;
	u16 low_addr = addr & 0xFFFF; /* low 16bit addr */
	u16 hi_addr = addr >> 16; /* high 16bit addr */

	ret = cps4029_fw_set_hi_addr(di, hi_addr);
	if (ret)
		return ret;

	ret = cps4029_read_block(di, low_addr, data, len);
	if (ret)
		return ret;

	return 0;
}

static int cps4029_fw_hold_mcu(struct cps4029_dev_info *di)
{
	return cps4029_aux_write_word(di, CPS4029_CMD_HOLD_MCU, CPS4029_HOLD_MCU);
}

static void cps4029_fw_soft_reset_system(struct cps4029_dev_info *di)
{
	u32 cmd = CPS4029_SYS_SOFT_REST;

	/* soft_reset: ignore i2c failure */
	(void)cps4029_fw_write_block(di, CPS4029_SYS_SOFT_REST_ADDR,
		(u8 *)&cmd, CPS4029_SYS_CMD_LEN);
	hwlog_info("reset_system: ignore i2c failure\n");
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
}

int cps4029_fw_sram_i2c_init(struct cps4029_dev_info *di, u8 inc_mode)
{
	int ret;
	u32 wr_data = CPS4029_I2C_TEST_VAL;
	u32 rd_data = 0;

	if (!di)
		return -ENODEV;

	ret = cps4029_fw_unlock_i2c(di);
	if (ret)
		return ret;
	ret = cps4029_fw_set_increase_mode(di, inc_mode);
	if (ret)
		return ret;
	ret = cps4029_fw_set_hi_addr(di, CPS4029_SRAM_HI_ADDR);
	if (ret)
		return ret;

	ret = cps4029_write_block(di, CPS4029_I2C_TEST_ADDR,
		(u8 *)&wr_data, CPS4029_I2C_TEST_LEN);
	if (ret)
		return ret;
	ret = cps4029_read_block(di, CPS4029_I2C_TEST_ADDR,
		(u8 *)&rd_data, CPS4029_I2C_TEST_LEN);
	if (ret)
		return ret;

	if (rd_data != wr_data) {
		hwlog_err("sram_i2c_init: failed, rd_data=0x%x\n", rd_data);
		di->g_val.sram_i2c_ready = false;
		return -EINVAL;
	}

	di->g_val.sram_i2c_ready = true;
	return 0;
}

static int cps4029_fw_enable_remap(struct cps4029_dev_info *di)
{
	u32 cmd = CPS4029_SYS_REMAP_EN;

	return cps4029_fw_write_block(di, CPS4029_SYS_REMAP_EN_ADDR,
		(u8 *)&cmd, CPS4029_SYS_CMD_LEN);
}

static int cps4029_fw_disable_trim(struct cps4029_dev_info *di)
{
	u32 cmd = CPS4029_SYS_TRIM_DIS;

	return cps4029_fw_write_block(di, CPS4029_SYS_TRIM_DIS_ADDR,
		(u8 *)&cmd, CPS4029_SYS_CMD_LEN);
}

static int cps4029_fw_check_mtp_crc(struct cps4029_dev_info *di)
{
	int i;
	int ret;
	u16 crc = 0;

	ret = cps4029_fw_sram_i2c_init(di, CPS4029_BYTE_INC);
	if (ret) {
		hwlog_err("check_mtp_crc: i2c init failed\n");
		return ret;
	}
	ret = cps4029_write_byte_mask(di, CPS4029_TX_CMD_ADDR,
		CPS4029_TX_CMD_CRC_CHK, CPS4029_TX_CMD_CRC_CHK_SHIFT,
		CPS4029_TX_CMD_VAL);
	if (ret) {
		hwlog_err("check_mtp_crc: write cmd failed\n");
		return ret;
	}

	/* 100ms*5=500ms timeout for status check */
	for (i = 0; i < 5; i++) {
		(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
		ret = cps4029_read_word(di, CPS4029_CRC_ADDR, &crc);
		if (ret) {
			hwlog_err("check_mtp_crc: get crc failed\n");
			return ret;
		}
		if (crc == CPS4029_MTP_CRC)
			return 0;
	}

	hwlog_err("check_mtp_crc: timeout, crc=0x%x\n", crc);
	return -EINVAL;
}

static int cps4029_fw_check_mtp_version(struct cps4029_dev_info *di)
{
	int ret;
	struct cps4029_chip_info info;

	ret = cps4029_fw_sram_i2c_init(di, CPS4029_BYTE_INC);
	if (ret) {
		hwlog_err("check_mtp_match: i2c init failed\n");
		return ret;
	}
	ret = cps4029_get_chip_info(di, &info);
	if (ret) {
		hwlog_err("check_mtp_match: get chip_info failed\n");
		return ret;
	}

	hwlog_info("[check_mtp_match] mtp_ver=0x%04x\n", info.mtp_ver);
	if (info.mtp_ver != CPS4029_MTP_VER) {
		hwlog_info("[check_mtp_match] ver=0x%04x\n", CPS4029_MTP_VER);
		return -EINVAL;
	}

	return 0;
}

static int cps4029_fw_status_check(struct cps4029_dev_info *di, u32 cmd)
{
	int i;
	int ret;
	u8 status = 0;

	ret = cps4029_fw_write_block(di, CPS4029_SRAM_STRAT_CMD_ADDR,
		(u8 *)&cmd, CPS4029_SRAM_CMD_LEN);
	if (ret) {
		hwlog_err("status_check: set check cmd failed\n");
		return ret;
	}

	/* wait for 50ms*10=500ms for status check */
	for (i = 0; i < 10; i++) {
		(void)power_msleep(DT_MSLEEP_50MS, 0, NULL);
		ret = cps4029_fw_read_block(di, CPS4029_SRAM_CHK_CMD_ADDR,
			&status, BYTE_LEN);
		if (ret) {
			hwlog_err("status_check: get status failed\n");
			return ret;
		}
		if (status == CPS4029_CHK_SUCC)
			return 0;
		if (status == CPS4029_CHK_FAIL) {
			hwlog_err("status_check: failed, stat=0x%x\n", status);
			return -EINVAL;
		}
	}

	hwlog_err("status_check: status=0x%x, program timeout\n", status);
	return -EINVAL;
}

static int cps4029_fw_write_sram_data(struct cps4029_dev_info *di, u32 cur_addr,
	const u8 *data, int len)
{
	int ret;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = cur_addr;
	int remaining = len;
	u8 wr_buff[CPS4029_FW_PAGE_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > CPS4029_FW_PAGE_SIZE ?
			CPS4029_FW_PAGE_SIZE : remaining;
		memcpy(wr_buff, data + wr_already, size_to_wr);
		ret = cps4029_write_block(di, addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("write_sram_data: fail, addr=0x%x\n", addr);
			return ret;
		}
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	return 0;
}

static int cps4029_fw_enter_load_mtp_mode(struct cps4029_dev_info *di)
{
	int ret;
	u32 test_en = CPS4029_PGM_EN_TEST;
	u32 carry_en = CPS4029_PGM_EN_CARRY;

	ret = cps4029_fw_write_block(di, CPS4029_PGM_EN_TEST_ADDR,
		(u8 *)&test_en, CPS4029_PGM_CMD_LEN);
	if (ret) {
		hwlog_err("enter_load_mtp_mode: enable test mode failed\n");
		return ret;
	}
	ret = cps4029_fw_write_block(di, CPS4029_PGM_EN_CARRY_ADDR,
		(u8 *)&carry_en, CPS4029_PGM_CMD_LEN);
	if (ret) {
		hwlog_err("enter_load_mtp_mode: enable SRAM->MTP failed\n");
		return ret;
	}

	return 0;
}

static int cps4029_fw_exit_load_mtp_mode(struct cps4029_dev_info *di)
{
	int ret;
	u32 test_dis = CPS4029_PGM_DIS_TEST;
	u32 carry_dis = CPS4029_PGM_DIS_CARRY;

	ret = cps4029_fw_write_block(di, CPS4029_PGM_EN_TEST_ADDR,
		(u8 *)&test_dis, CPS4029_PGM_CMD_LEN);
	if (ret) {
		hwlog_err("exit_load_mtp_mode: disable test mode failed\n");
		return ret;
	}
	ret = cps4029_fw_write_block(di, CPS4029_PGM_EN_CARRY_ADDR,
		(u8 *)&carry_dis, CPS4029_PGM_CMD_LEN);
	if (ret) {
		hwlog_err("exit_load_mtp_mode: disable SRAM->MTP failed\n");
		return ret;
	}

	return 0;
}

static int cps4029_fw_write_mtp_data(struct cps4029_dev_info *di)
{
	int ret;
	int offset = 0;
	int remaining = CPS4029_FW_MTP_SIZE;
	int wr_size;

	while (remaining > 0) {
		wr_size = remaining > CPS4029_SRAM_MTP_BUFF_SIZE ?
			CPS4029_SRAM_MTP_BUFF_SIZE : remaining;
		ret = cps4029_fw_write_sram_data(di, CPS4029_SRAM_MTP_BUFF0,
			g_cps4029_mtp + offset, wr_size);
		if (ret) {
			hwlog_err("write_mtp_data: write mtp failed\n");
			return ret;
		}
		ret = cps4029_fw_status_check(di, CPS4029_STRAT_CARRY_BUF0);
		if (ret) {
			hwlog_err("write_mtp_data: check crc failed\n");
			return ret;
		}
		offset += wr_size;
		remaining -= wr_size;
	}

	return 0;
}

static int cps4029_fw_load_mtp(struct cps4029_dev_info *di)
{
	int ret;

	ret = cps4029_fw_enter_load_mtp_mode(di);
	if (ret) {
		hwlog_err("load_mtp: enable load_mtp failed\n");
		return ret;
	}
	ret = cps4029_fw_set_hi_addr(di, CPS4029_SRAM_HI_ADDR);
	if (ret) {
		hwlog_err("load_mtp: switch to sram addr failed\n");
		return ret;
	}
	ret = cps4029_fw_write_mtp_data(di);
	if (ret) {
		hwlog_err("load_mtp: write mtp data failed\n");
		return ret;
	}
	ret = cps4029_fw_exit_load_mtp_mode(di);
	if (ret) {
		hwlog_err("load_mtp: disable load_mtp failed\n");
		return ret;
	}
	ret = cps4029_fw_status_check(di, CPS4029_START_CHK_MTP);
	if (ret) {
		hwlog_err("load_mtp: check mtp crc failed\n");
		return ret;
	}
	hwlog_info("[load_mtp] succ\n");

	return 0;
}

static int cps4029_fw_check_bootloader_version(struct cps4029_dev_info *di)
{
	int ret;
	u16 btl_ver = 0;

	ret = cps4029_read_word(di, CPS4029_SRAM_BTL_VER_ADDR, &btl_ver);
	if (ret) {
		hwlog_err("check_bootloader: get bootloader version failed\n");
		return ret;
	}
	hwlog_info("[check_bootloader] ver=0x%04x\n", btl_ver);
	if (btl_ver != CPS4029_BTL_VER) {
		hwlog_err("check_bootloader: exp_ver=0x%x\n", CPS4029_BTL_VER);
		return -EINVAL;
	}

	return 0;
}

static int cps4029_fw_check_bootloader(struct cps4029_dev_info *di)
{
	int ret;

	ret = cps4029_fw_sram_i2c_init(di, CPS4029_WORD_INC);
	if (ret) {
		hwlog_err("check_bootloader: i2c init failed\n");
		return ret;
	}
	ret = cps4029_fw_check_bootloader_version(di);
	if (ret) {
		hwlog_err("check_bootloader: check version failed\n");
		return ret;
	}
	ret = cps4029_fw_status_check(di, CPS4029_START_CHK_BTL);
	if (ret) {
		hwlog_err("check_bootloader: check crc failed\n");
		return ret;
	}

	return 0;
}

static int cps4029_fw_load_bootloader(struct cps4029_dev_info *di)
{
	int ret;

	ret = cps4029_fw_sram_i2c_init(di, CPS4029_WORD_INC);
	if (ret) {
		hwlog_err("load_bootloader: i2c init failed\n");
		return ret;
	}
	ret = cps4029_fw_write_sram_data(di, CPS4029_SRAM_BTL_BUFF,
		g_cps4029_bootloader, CPS4029_FW_BTL_SIZE);
	if (ret) {
		hwlog_err("load_bootloader: load bootloader data failed\n");
		return ret;
	}
	ret = cps4029_fw_enable_remap(di);
	if (ret) {
		hwlog_err("load_bootloader: enble remap failed\n");
		return ret;
	}
	ret = cps4029_fw_disable_trim(di);
	if (ret) {
		hwlog_err("load_bootloader: disable trim failed\n");
		return ret;
	}
	cps4029_fw_soft_reset_system(di);
	ret = cps4029_fw_check_bootloader(di);
	if (ret) {
		hwlog_err("load_bootloader: check bootloader failed\n");
		return ret;
	}

	hwlog_info("[load_bootloader] succ\n");
	return 0;
}

static int cps4029_fw_progam_post_process(struct cps4029_dev_info *di)
{
	int ret;

	ret = cps4029_fw_enter_load_mtp_mode(di);
	if (ret) {
		hwlog_err("load_mtp: enable load_mtp failed\n");
		return ret;
	}
	ret = cps4029_fw_status_check(di, CPS4029_START_CHK_PGM);
	if (ret) {
		hwlog_err("progam_post_process: get program result failed\n");
		return ret;
	}
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	power_usleep(DT_USLEEP_10MS);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	ret = cps4029_fw_check_mtp_version(di);
	if (ret) {
		hwlog_err("progam_post_process: mtp_version mismatch\n");
		return ret;
	}
	ret = cps4029_fw_exit_load_mtp_mode(di);
	if (ret) {
		hwlog_err("load_mtp: disable load_mtp failed\n");
		return ret;
	}

	return 0;
}

static int cps4029_fw_program_prev_process(struct cps4029_dev_info *di)
{
	int ret;

	ret = cps4029_fw_unlock_i2c(di);
	if (ret) {
		hwlog_err("program_prev_process: unlock i2c failed\n");
		return ret;
	}
	ret = cps4029_fw_hold_mcu(di);
	if (ret) {
		hwlog_err("program_prev_process: hold mcu failed\n");
		return ret;
	}

	return 0;
}

static int cps4029_fw_program_mtp(struct cps4029_dev_info *di)
{
	int ret;

	if (di->g_val.mtp_latest)
		return 0;

	cps4029_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	cps4029_chip_enable(RX_EN_ENABLE, di);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	ret = cps4029_fw_program_prev_process(di);
	if (ret)
		goto exit;
	ret = cps4029_fw_load_bootloader(di);
	if (ret)
		goto exit;
	ret = cps4029_fw_load_mtp(di);
	if (ret)
		goto exit;
	ret = cps4029_fw_progam_post_process(di);
	if (ret)
		goto exit;

	di->g_val.mtp_latest = true;
	hwlog_info("[program_mtp] succ\n");

exit:
	cps4029_enable_irq(di);
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	return ret;
}

static int cps4029_fw_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct cps4029_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("[rx_program_mtp] type=%u\n", proc_type);

	if (!di->g_val.mtp_chk_complete)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = cps4029_fw_program_mtp(di);
	if (!ret)
		hwlog_info("[rx_program_mtp] succ\n");
	di->g_val.mtp_chk_complete = true;

	return ret;
}

static int cps4029_fw_check_mtp(void *dev_data)
{
	struct cps4029_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->g_val.mtp_latest)
		return 0;

	wlps_control(di->ic_type, WLPS_TX_PWR_SW, true);
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);

	if (cps4029_fw_check_mtp_version(di)) {
		hwlog_err("check_mtp: mtp_ver mismatch\n");
		goto check_fail;
	}

	if (cps4029_fw_check_mtp_crc(di)) {
		hwlog_err("check_mtp: mtp_crc mismatch\n");
		goto check_fail;
	}

	di->g_val.mtp_latest = true;
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	hwlog_info("[check_mtp] mtp latest\n");
	return 0;

check_fail:
	wlps_control(di->ic_type, WLPS_TX_PWR_SW, false);
	return -EINVAL;
}

int cps4029_fw_sram_update(void *dev_data)
{
	return 0;
}

static int cps4029_fw_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct cps4029_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = cps4029_fw_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	di->g_val.mtp_chk_complete = true;

	return 0;
}

void cps4029_fw_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct cps4029_dev_info *di =
		container_of(work, struct cps4029_dev_info, mtp_check_work.work);

	if (!di) {
		hwlog_err("mtp_check_work: di null\n");
		return;
	}

	di->g_val.mtp_chk_complete = false;
	ret = cps4029_fw_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = cps4029_fw_program_mtp(di);
		if (ret)
			continue;
		hwlog_info("[mtp_check_work] update mtp succ, cnt=%d\n", i + 1);
		goto exit;
	}
	hwlog_err("mtp_check_work: update mtp failed\n");

exit:
	di->g_val.mtp_chk_complete = true;
}

static struct wireless_fw_ops g_cps4029_fw_ops = {
	.ic_name                = "cps4029",
	.program_fw             = cps4029_fw_rx_program_mtp,
	.get_fw_status          = cps4029_fw_get_mtp_status,
	.check_fw               = cps4029_fw_check_mtp,
};

int cps4029_fw_ops_register(struct wltrx_ic_ops *ops, struct cps4029_dev_info *di)
{
	if (!ops || !di)
		return -ENODEV;

	ops->fw_ops = kzalloc(sizeof(*(ops->fw_ops)), GFP_KERNEL);
	if (!ops->fw_ops)
		return -ENOMEM;

	memcpy(ops->fw_ops, &g_cps4029_fw_ops, sizeof(g_cps4029_fw_ops));
	ops->fw_ops->dev_data = (void *)di;

	return wireless_fw_ops_register(ops->fw_ops);
}
