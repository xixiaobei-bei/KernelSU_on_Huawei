/*
 * mt5735_fw.c
 *
 * mt5735 mtp, sram driver
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

#include "mt5735.h"
#include "mt5735_mtp.h"

#define HWLOG_TAG wireless_mt5735_fw
HWLOG_REGIST();

static int mt5735_get_major_fw_version(u16 *fw)
{
	return mt5735_read_word(MT5735_MTP_MAJOR_ADDR, fw);
}

static int mt5735_get_minor_fw_version(u16 *fw)
{
	return mt5735_read_word(MT5735_MTP_MINOR_ADDR, fw);
}

static int mt5735_mtp_version_check(void)
{
	int ret;
	u16 major_fw_ver = 0;
	u16 minor_fw_ver = 0;

	ret = mt5735_get_major_fw_version(&major_fw_ver);
	if (ret)
		return ret;
	hwlog_info("version_check: major_fw=0x%04x\n", major_fw_ver);

	ret = mt5735_get_minor_fw_version(&minor_fw_ver);
	if (ret)
		return ret;
	hwlog_info("version_check: minor_fw=0x%04x\n", minor_fw_ver);

	if ((major_fw_ver != MT5735_MTP_MAJOR_VER) ||
		(minor_fw_ver != MT5735_MTP_MINOR_VER))
		return -ENXIO;

	return 0;
}

static int mt5735_mtp_pwr_cycle_chip(void)
{
	int ret;

	ret = mt5735_write_byte(0x5808, 0x95); /* disable wtd */
	ret += mt5735_write_byte(0x5800, 0x01); /* clear wtd flag */
	ret += mt5735_write_byte(0x5244, 0x57); /* write key to map */
	ret += mt5735_write_byte(0x5200, 0x20); /* hold M0 */
	ret += mt5735_write_word(0x5218, 0x0FFF); /* select sram map addr */
	ret += mt5735_write_byte(0x5208, 0x08); /* select mtp map addr */
	if (ret) {
		hwlog_err("pwr_cycle_chip: failed\n");
		return -EIO;
	}

	msleep(DT_MSLEEP_50MS); /* for power on, typically 20ms */
	return 0;
}

static int mt5735_mtp_load_bootloader(struct mt5735_dev_info *di)
{
	int ret;
	int remaining = ARRAY_SIZE(g_mt5735_bootloader);
	int size_to_wr;
	int wr_already = 0;
	u16 chip_id = 0;
	u16 addr = MT5735_BTLOADR_ADDR;
	u8 wr_buff[MT5735_MTP_PGM_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > MT5735_MTP_PGM_SIZE ?
			MT5735_MTP_PGM_SIZE : remaining;
		memcpy(wr_buff, g_mt5735_bootloader + wr_already, size_to_wr);
		ret = mt5735_write_block(addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("load_bootloader: failed, addr=0x%04x\n", addr);
			return ret;
		}
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	ret = mt5735_write_byte(0x5200, 0x80); /* reset M0 */
	if (ret) {
		hwlog_err("load_bootloader: reset M0 failed\n");
		return ret;
	}
	msleep(DT_MSLEEP_50MS); /* for power on, typically 20ms */
	ret = mt5735_read_word(MT5735_MTP_CHIP_ID_ADDR, &chip_id);
	if (ret)
		return ret;

	hwlog_info("load_bootloader: chip_id=0x%x\n", chip_id);
	if (chip_id != MT5735_CHIP_ID)
		return -ENXIO;

	hwlog_info("[load_bootloader] succ\n");
	return 0;
}

static int mt5735_mtp_status_check(u16 expect_status)
{
	int i;
	int ret;
	u16 status = 0;

	/* wait for 10ms*50=500ms for status check, typically 300ms */
	for (i = 0; i < 50; i++) {
		power_usleep(DT_USLEEP_10MS);
		ret = mt5735_read_word(MT5735_MTP_STATUS_ADDR, &status);
		if (ret) {
			hwlog_err("status_check: read failed\n");
			return ret;
		}
		if (status == expect_status)
			return 0;
	}

	return -ENXIO;
}

static int mt5735_mtp_crc_check(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("crc_check: load bootloader failed\n");
		return ret;
	}

	/* write mtp start address */
	ret = mt5735_write_word(0x0002, 0x0000);
	/* write 16-bit MTP data size */
	ret += mt5735_write_word(0x0004, (u16)ARRAY_SIZE(g_mt5735_mtp));
	/* write the 16-bit CRC */
	ret += mt5735_write_word(0x0006, MT5735_MTP_CRC);
	/* start MTP data CRC-16 check */
	ret += mt5735_write_word(MT5735_MTP_CRC_ADDR, MT5735_MTP_CRC_CMD);
	ret += mt5735_mtp_status_check(MT5735_MTP_CRC_OK);
	if (ret) {
		hwlog_err("crc_check: failed\n");
		return ret;
	}

	hwlog_err("[crc_check] succ\n");
	return 0;
}

static int mt5735_check_mtp_match(struct mt5735_dev_info *di)
{
	int ret;

	ret = mt5735_mtp_version_check();
	if (ret)
		return ret;

	ret = mt5735_mtp_pwr_cycle_chip();
	if (ret)
		return ret;

	ret = mt5735_mtp_crc_check(di);
	if (ret)
		return ret;

	return 0;
}

static int mt5735_mtp_load_fw(struct mt5735_dev_info *di)
{
	int i;
	int ret;
	u16 addr = 0; /* start from adrr 0 */
	u16 remaining = (u16)ARRAY_SIZE(g_mt5735_mtp);
	u16 wr_size;
	u16 chksum;

	hwlog_info("[load_fw] mtp_size=%d\n", remaining);
	while (remaining > 0) {
		wr_size = remaining > MT5735_MTP_PGM_SIZE ?
			MT5735_MTP_PGM_SIZE : remaining;
		ret = mt5735_write_word(MT5735_MTP_PGM_ADDR_ADDR, addr);
		if (ret) {
			hwlog_err("load_fw: write addr failed\n");
			return ret;
		}
		ret = mt5735_write_word(MT5735_MTP_PGM_LEN_ADDR, wr_size);
		if (ret) {
			hwlog_err("load_fw: write len failed\n");
			return ret;
		}
		chksum = addr + wr_size;
		for (i = 0; i < wr_size; i++)
			chksum += g_mt5735_mtp[addr + i];
		ret = mt5735_write_word(MT5735_MTP_PGM_CHKSUM_ADDR, chksum);
		if (ret) {
			hwlog_err("load_fw: write checksum failed\n");
			return ret;
		}
		ret = mt5735_write_block(MT5735_MTP_PGM_DATA_ADDR,
			(u8 *)&g_mt5735_mtp[addr], wr_size);
		if (ret) {
			hwlog_err("load_fw: write mtp_data failed\n");
			return ret;
		}
		ret = mt5735_write_byte(MT5735_MTP_PGM_CMD_ADDR, MT5735_MTP_PGM_CMD);
		if (ret) {
			hwlog_err("load_fw: start programming failed\n");
			return ret;
		}
		ret = mt5735_mtp_status_check(MT5735_MTP_PGM_OK);
		if (ret) {
			hwlog_err("load_fw: check mtp status failed\n");
			return ret;
		}
		addr += wr_size;
		remaining -= wr_size;
	}

	return 0;
}

static int mt5735_fw_program_mtp(struct mt5735_dev_info *di)
{
	int ret;

	if (di->g_val.mtp_latest)
		return 0;

	mt5735_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	(void)mt5735_chip_enable(true, di);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	ret = mt5735_mtp_pwr_cycle_chip();
	if (ret) {
		hwlog_err("program_mtp: power cycle chip failed\n");
		goto exit;
	}
	ret = mt5735_mtp_load_bootloader(di);
	if (ret) {
		hwlog_err("program_mtp: load bootloader failed\n");
		goto exit;
	}
	ret = mt5735_mtp_load_fw(di);
	if (ret) {
		hwlog_err("program_mtp: load fw failed\n");
		goto exit;
	}
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	power_usleep(DT_USLEEP_10MS); /* for power off, typically 5ms */
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, true);
	power_usleep(DT_USLEEP_10MS); /* for power on, typically 5ms */
	ret = mt5735_check_mtp_match(di);
	if (ret) {
		hwlog_err("program_mtp: mtp mismatch\n");
		goto exit;
	}

	di->g_val.mtp_latest = true;
	hwlog_info("[program_mtp] succ\n");

exit:
	mt5735_enable_irq(di);
	wlps_control(di->ic_type, WLPS_PROC_OTP_PWR, false);
	return ret;
}

static int mt5735_fw_rx_program_mtp(unsigned int proc_type, void *dev_data)
{
	int ret;
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	hwlog_info("rx_program_mtp: type=%u\n", proc_type);
	if (!di->g_val.mtp_chk_complete)
		return -WLC_ERR_I2C_WR;
	di->g_val.mtp_chk_complete = false;
	ret = mt5735_fw_program_mtp(di);
	if (!ret)
		hwlog_info("[rx_program_mtp] succ\n");
	di->g_val.mtp_chk_complete = true;

	return ret;
}

static int mt5735_fw_check_mtp(void *dev_data)
{
	int ret;
	struct mt5735_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	if (di->g_val.mtp_latest)
		return 0;

	mt5735_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	msleep(DT_MSLEEP_100MS); /* for power on, typically 50ms */

	ret = mt5735_check_mtp_match(di);
	if (ret)
		goto exit;

	di->g_val.mtp_latest = true;
	hwlog_info("[check_mtp] mtp latest\n");

exit:
	mt5735_enable_irq(di);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	return ret;
}

int mt5735_fw_sram_update(void *dev_data)
{
	int ret;
	u16 minor_fw_ver = 0;

	ret = mt5735_get_minor_fw_version(&minor_fw_ver);
	if (ret) {
		hwlog_err("sram_update: get minor fw_ver failed\n");
		return ret;
	}
	hwlog_info("mtp_version=0x%x\n", minor_fw_ver);

	return 0;
}

static int mt5735_fw_get_mtp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct mt5735_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	di->g_val.mtp_chk_complete = false;
	ret = mt5735_fw_check_mtp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;
	di->g_val.mtp_chk_complete = true;

	return 0;
}

void mt5735_fw_mtp_check_work(struct work_struct *work)
{
	int i;
	int ret;
	struct mt5735_dev_info *di = container_of(work,
		struct mt5735_dev_info, mtp_check_work.work);

	if (!di)
		return;

	di->g_val.mtp_chk_complete = false;
	ret = mt5735_fw_check_mtp(di);
	if (!ret) {
		hwlog_info("[mtp_check_work] succ\n");
		goto exit;
	}

	/* program for 3 times until it's ok */
	for (i = 0; i < 3; i++) {
		ret = mt5735_fw_program_mtp(di);
		if (ret)
			continue;
		hwlog_info("mtp_check_work: update mtp succ, cnt=%d\n", i + 1);
		goto exit;
	}
	hwlog_err("mtp_check_work: update mtp failed\n");

exit:
	di->g_val.mtp_chk_complete = true;
}

static struct wireless_fw_ops g_mt5735_fw_ops = {
	.ic_name       = "mt5735",
	.program_fw    = mt5735_fw_rx_program_mtp,
	.get_fw_status = mt5735_fw_get_mtp_status,
	.check_fw      = mt5735_fw_check_mtp,
};

int mt5735_fw_ops_register(struct mt5735_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_mt5735_fw_ops.dev_data = (void *)di;
	return wireless_fw_ops_register(&g_mt5735_fw_ops);
}
