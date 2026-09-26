/*
 * cps4057_fw.c
 *
 * cps4057 mtp, sram driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "cps4057.h"
#include "cps4057_mtp.h"

#define HWLOG_TAG wireless_cps4057_fw
HWLOG_REGIST();

static int cps4057_fw_unlock_i2c(void)
{
	return cps4057_aux_write_word(CPS4057_CMD_UNLOCK_I2C, CPS4057_I2C_CODE);
}

static int cps4057_fw_set_hi_addr(u32 hi_addr)
{
	return cps4057_aux_write_word(CPS4057_CMD_SET_HI_ADDR, hi_addr);
}

static int cps4057_fw_set_increase_mode(u16 mode)
{
	return cps4057_aux_write_word(CPS4057_CMD_INC_MODE, mode);
}

static int cps4057_fw_write_block(u32 addr, u8 *cmd, int len)
{
	int ret;
	u16 low_addr = addr & 0xFFFF; /* low 16bit addr */
	u16 hi_addr = addr >> 16; /* high 16bit addr */

	ret = cps4057_fw_set_hi_addr(hi_addr);
	if (ret)
		return ret;

	ret = cps4057_write_block(low_addr, cmd, len);
	if (ret)
		return ret;

	return 0;
}

static int cps4057_fw_read_block(u32 addr, u8 *data, int len)
{
	int ret;
	u16 low_addr = addr & 0xFFFF; /* low 16bit addr */
	u16 hi_addr = addr >> 16; /* high 16bit addr */

	ret = cps4057_fw_set_hi_addr(hi_addr);
	if (ret)
		return ret;

	ret = cps4057_read_block(low_addr, data, len);
	if (ret)
		return ret;

	return 0;
}

static void cps4057_fw_ps_control(unsigned int ic_type, int scene, bool flag)
{
	static int ref_cnt;

	hwlog_info("[ps_control] ref_cnt=%d, flag=%d\n", ref_cnt, flag);
	if (flag)
		++ref_cnt;
	else if (--ref_cnt > 0)
		return;

	wlps_control(ic_type, scene, flag);
}

static int cps4057_fw_hold_mcu(void)
{
	return cps4057_aux_write_word(CPS4057_CMD_HOLD_MCU, CPS4057_HOLD_MCU);
}

static void cps4057_fw_soft_reset_system(void)
{
	u32 cmd = CPS4057_SYS_SOFT_REST;

	/* soft_reset: ignore i2c failure */
	(void)cps4057_fw_write_block(CPS4057_SYS_SOFT_REST_ADDR,
		(u8 *)&cmd, CPS4057_SYS_CMD_LEN);
	hwlog_info("reset_system: ignore i2c failure\n");
	msleep(DT_MSLEEP_100MS); /* for system reset, typically 50ms */
}

static int cps4057_fw_sram_i2c_init(u8 inc_mode)
{
	int ret;
	u32 wr_data = CPS4057_I2C_TEST_VAL;
	u32 rd_data = 0;
	struct cps4057_dev_info *di = NULL;

	cps4057_get_dev_info(&di);
	if (!di)
		return -ENODEV;

	ret = cps4057_fw_unlock_i2c();
	if (ret)
		return ret;
	ret = cps4057_fw_set_increase_mode(inc_mode);
	if (ret)
		return ret;
	ret = cps4057_fw_set_hi_addr(CPS4057_SRAM_HI_ADDR);
	if (ret)
		return ret;

	ret = cps4057_write_block(CPS4057_I2C_TEST_ADDR,
		(u8 *)&wr_data, CPS4057_I2C_TEST_LEN);
	if (ret)
		return ret;
	ret = cps4057_read_block(CPS4057_I2C_TEST_ADDR,
		(u8 *)&rd_data, CPS4057_I2C_TEST_LEN);
	if (ret)
		return ret;

	if (rd_data != wr_data) {
		hwlog_err("sram_i2c_init: failed, rd_data=0x%x\n", rd_data);
		return -EIO;
	}

	return 0;
}

static int cps4057_fw_enable_remap(void)
{
	u32 cmd = CPS4057_SYS_REMAP_EN;

	return cps4057_fw_write_block(CPS4057_SYS_REMAP_EN_ADDR,
		(u8 *)&cmd, CPS4057_SYS_CMD_LEN);
}

static int cps4057_fw_disable_trim(void)
{
	u32 cmd = CPS4057_SYS_TRIM_DIS;

	return cps4057_fw_write_block(CPS4057_SYS_TRIM_DIS_ADDR,
		(u8 *)&cmd, CPS4057_SYS_CMD_LEN);
}

static int cps4057_fw_check_mtp_crc(void)
{
	int i;
	int ret;
	u16 crc = 0;

	ret = cps4057_fw_sram_i2c_init(CPS4057_BYTE_INC);
	if (ret) {
		hwlog_err("check_mtp_crc: i2c init failed\n");
		return ret;
	}
	ret = cps4057_write_byte_mask(CPS4057_TX_CMD_ADDR,
		CPS4057_TX_CMD_CRC_CHK, CPS4057_TX_CMD_CRC_CHK_SHIFT,
		CPS4057_TX_CMD_VAL);
	if (ret) {
		hwlog_err("check_mtp_crc: write cmd failed\n");
		return ret;
	}

	/* 100ms*5=500ms timeout for status check */
	for (i = 0; i < 5; i++) {
		msleep(DT_MSLEEP_100MS);
		ret = cps4057_read_word(CPS4057_CRC_ADDR, &crc);
		if (ret) {
			hwlog_err("check_mtp_crc: get crc failed\n");
			return ret;
		}
		if (crc == CPS4057_MTP_CRC)
			return 0;
	}

	hwlog_err("check_mtp_crc: timeout, crc=0x%x\n", crc);
	return -EIO;
}

static int cps4057_fw_check_mtp_version(void)
{
	int ret;
	struct cps4057_chip_info info;

	ret = cps4057_fw_sram_i2c_init(CPS4057_BYTE_INC);
	if (ret) {
		hwlog_err("check_mtp_match: i2c init failed\n");
		return ret;
	}
	ret = cps4057_get_chip_info(&info);
	if (ret) {
		hwlog_err("check_mtp_match: get chip_info failed\n");
		return ret;
	}

	hwlog_info("[check_mtp_match] mtp_ver=0x%04x\n", info.mtp_ver);
	if (info.mtp_ver != CPS4057_MTP_VER)
		return -WLC_ERR_MISMATCH;

	return 0;
}

static int cps4057_fw_status_check(u32 cmd)
{
	int i;
	int ret;
	u8 status = 0;

	ret = cps4057_fw_write_block(CPS4057_SRAM_STRAT_CMD_ADDR,
		(u8 *)&cmd, CPS4057_SRAM_CMD_LEN);
	if (ret) {
		hwlog_err("status_check: set check cmd failed\n");
		return ret;
	}

	/* wait for 50ms*10=500ms for status check */
	for (i = 0; i < 10; i++) {
		msleep(DT_MSLEEP_50MS);
		ret = cps4057_fw_read_block(CPS4057_SRAM_CHK_CMD_ADDR,
			&status, BYTE_LEN);
		if (ret) {
			hwlog_err("status_check: get status failed\n");
			return ret;
		}
		if (status == CPS4057_CHK_SUCC)
			return 0;
		if (status == CPS4057_CHK_FAIL) {
			hwlog_err("status_check: failed, stat=0x%x\n", status);
			return -WLC_ERR_CHECK_FAIL;
		}
	}

	hwlog_err("status_check: status=0x%x, program timeout\n", status);
	return -WLC_ERR_CHECK_FAIL;
}

static int cps4057_fw_write_sram_data(u32 cur_addr,
	const u8 *data, int len)
{
	int ret;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = cur_addr;
	int remaining = len;
	u8 wr_buff[CPS4057_FW_PAGE_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > CPS4057_FW_PAGE_SIZE ?
			CPS4057_FW_PAGE_SIZE : remaining;
		memcpy(wr_buff, data + wr_already, size_to_wr);
		ret = cps4057_write_block(addr, wr_buff, size_to_wr);
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

static int cps4057_fw_write_mtp_data(void)
{
	int ret;
	int offset = 0;
	int remaining = CPS4057_FW_MTP_SIZE;
	int wr_size;

	while (remaining > 0) {
		wr_size = remaining > CPS4057_SRAM_MTP_BUFF_SIZE ?
			CPS4057_SRAM_MTP_BUFF_SIZE : remaining;
		ret = cps4057_fw_write_sram_data(CPS4057_SRAM_MTP_BUFF0,
			g_cps4057_mtp + offset, wr_size);
		if (ret) {
			hwlog_err("write_mtp_data: write mtp failed\n");
			return ret;
		}
		ret = cps4057_fw_status_check(CPS4057_STRAT_CARRY_BUF0);
		if (ret) {
			hwlog_err("write_mtp_data: check crc failed\n");
			return ret;
		}
		offset += wr_size;
		remaining -= wr_size;
	}

	return 0;
}

static int cps4057_fw_load_mtp(void)
{
	int ret;

	ret = cps4057_fw_set_hi_addr(CPS4057_SRAM_HI_ADDR);
	if (ret) {
		hwlog_err("load_mtp: switch to sram addr failed\n");
		return ret;
	}
	ret = cps4057_fw_write_mtp_data();
	if (ret) {
		hwlog_err("load_mtp: write mtp data failed\n");
		return ret;
	}

	ret = cps4057_fw_status_check(CPS4057_START_CHK_MTP);
	if (ret) {
		hwlog_err("load_mtp: check mtp crc failed\n");
		return ret;
	}
	hwlog_info("[load_mtp] succ\n");
	return 0;
}

static int cps4057_fw_check_bootloader_version(void)
{
	int ret;
	u16 btl_ver = 0;

	ret = cps4057_read_word(CPS4057_SRAM_BTL_VER_ADDR, &btl_ver);
	if (ret) {
		hwlog_err("check_bootloader: get bootloader version failed\n");
		return ret;
	}
	hwlog_info("[check_bootloader] ver=0x%04x\n", btl_ver);
	if (btl_ver != CPS4057_BTL_VER) {
		hwlog_err("check_bootloader: exp_ver=0x%x\n", CPS4057_BTL_VER);
		return -WLC_ERR_CHECK_FAIL;
	}

	return 0;
}

static int cps4057_fw_check_bootloader(void)
{
	int ret;

	ret = cps4057_fw_sram_i2c_init(CPS4057_WORD_INC);
	if (ret) {
		hwlog_err("check_bootloader: i2c init failed\n");
		return ret;
	}
	ret = cps4057_fw_check_bootloader_version();
	if (ret) {
		hwlog_err("check_bootloader: check version failed\n");
		return ret;
	}
	ret = cps4057_fw_status_check(CPS4057_START_CHK_BTL);
	if (ret) {
		hwlog_err("check_bootloader: check crc failed\n");
		return ret;
	}

	return 0;
}

static int cps4057_fw_load_bootloader(void)
{
	int ret;

	ret = cps4057_fw_sram_i2c_init(CPS4057_WORD_INC);
	if (ret) {
		hwlog_err("load_bootloader: i2c init failed\n");
		return ret;
	}
	ret = cps4057_fw_write_sram_data(CPS4057_SRAM_BTL_BUFF,
		g_cps4057_bootloader, CPS4057_FW_BTL_SIZE);
	if (ret) {
		hwlog_err("load_bootloader: load bootloader data failed\n");
		return ret;
	}
	ret = cps4057_fw_enable_remap();
	if (ret) {
		hwlog_err("load_bootloader: enble remap failed\n");
		return ret;
	}
	ret = cps4057_fw_disable_trim();
	if (ret) {
		hwlog_err("load_bootloader: disable trim failed\n");
		return ret;
	}
	cps4057_fw_soft_reset_system();
	ret = cps4057_fw_check_bootloader();
	if (ret) {
		hwlog_err("load_bootloader: check bootloader failed\n");
		return ret;
	}

	hwlog_info("[load_bootloader] succ\n");
	return 0;
}

static int cps4057_fw_progam_post_process(struct cps4057_dev_info *di)
{
	int ret;

	ret = cps4057_fw_status_check(CPS4057_START_CHK_PGM);
	if (ret) {
		hwlog_err("progam_post_process: get program result failed\n");
		return ret;
	}

	cps4057_fw_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	power_usleep(DT_USLEEP_10MS); /* for power off, typically 5ms */
	cps4057_fw_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	ret = cps4057_fw_check_mtp_version();
	if (ret) {
		hwlog_err("progam_post_process: mtp_version mismatch\n");
		return ret;
	}

	return 0;
}

static int cps4057_fw_program_prev_process(void)
{
	int ret;

	ret = cps4057_fw_unlock_i2c();
	if (ret) {
		hwlog_err("program_prev_process: unlock i2c failed\n");
		return ret;
	}
	ret = cps4057_fw_hold_mcu();
	if (ret) {
		hwlog_err("program_prev_process: hold mcu failed\n");
		return ret;
	}

	return 0;
}

static int cps4057_fw_program_mtp(struct cps4057_dev_info *di)
{
	int ret;

	if (di->g_val.mtp_latest)
		return 0;

	cps4057_disable_irq_nosync(di);
	cps4057_fw_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	(void)cps4057_chip_enable(true, di);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	ret = cps4057_fw_program_prev_process();
	if (ret)
		goto exit;
	ret = cps4057_fw_load_bootloader();
	if (ret)
		goto exit;
	ret = cps4057_fw_load_mtp();
	if (ret)
		goto exit;
	ret = cps4057_fw_progam_post_process(di);
	if (ret)
		goto exit;

	di->g_val.mtp_latest = true;
	hwlog_info("[program_mtp] succ\n");

exit:
	cps4057_enable_irq(di);
	cps4057_fw_ps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	return ret;
}

static int cps4057_fw_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct cps4057_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("[rx_program_mtp] type=%u\n", proc_type);

	if (!di->g_val.mtp_chk_complete)
		return -WLC_ERR_I2C_WR;

	di->g_val.mtp_chk_complete = false;
	ret = cps4057_fw_program_mtp(di);
	if (!ret)
		hwlog_info("[rx_program_mtp] succ\n");
	di->g_val.mtp_chk_complete = true;

	return ret;
}

static int cps4057_fw_check_mtp(void *dev_data)
{
	int ret;
	struct cps4057_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->g_val.mtp_latest)
		return 0;

	cps4057_disable_irq_nosync(di);
	cps4057_fw_ps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	wlps_control(WLTRX_IC_MAIN, WLPS_RX_SW, false);
	charge_pump_chip_enable(CP_TYPE_MAIN, false);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	ret = cps4057_fw_check_mtp_version();
	if (ret) {
		hwlog_err("check_mtp: mtp_ver mismatch\n");
		goto exit;
	}

	ret = cps4057_fw_check_mtp_crc();
	if (ret) {
		hwlog_err("check_mtp: mtp_crc mismatch\n");
		goto exit;
	}

	di->g_val.mtp_latest = true;
	hwlog_info("[check_mtp] mtp latest\n");

exit:
	cps4057_enable_irq(di);
	cps4057_fw_ps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	return ret;
}

int cps4057_fw_sram_update(void *dev_data)
{
	int ret;
	struct cps4057_chip_info info = { 0 };

	ret = cps4057_get_chip_info(&info);
	if (!ret)
		hwlog_info("[chip_info] chip_id=0x%x, mtp_version=0x%x\n",
			info.chip_id, info.mtp_ver);

	return 0;
}

static int cps4057_fw_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct cps4057_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = cps4057_fw_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	di->g_val.mtp_chk_complete = true;

	return 0;
}

void cps4057_fw_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct cps4057_dev_info *di = container_of(work,
		struct cps4057_dev_info, mtp_check_work.work);

	if (!di)
		return;

	di->g_val.mtp_chk_complete = false;
	ret = cps4057_fw_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = cps4057_fw_program_mtp(di);
		if (ret)
			continue;
		hwlog_info("[mtp_check_work] update mtp succ, cnt=%d\n", i + 1);
		goto exit;
	}
	hwlog_err("mtp_check_work: update mtp failed\n");

exit:
	di->g_val.mtp_chk_complete = true;
}

static struct wireless_fw_ops g_cps4057_fw_ops = {
	.ic_name       = "cps4057",
	.program_fw    = cps4057_fw_rx_program_mtp,
	.get_fw_status = cps4057_fw_get_mtp_status,
	.check_fw      = cps4057_fw_check_mtp,
};

int cps4057_fw_ops_register(struct cps4057_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_cps4057_fw_ops.dev_data = (void *)di;
	return wireless_fw_ops_register(&g_cps4057_fw_ops);
}
