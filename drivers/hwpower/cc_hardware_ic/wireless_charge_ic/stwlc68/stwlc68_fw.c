/*
 * stwlc68_fw.c
 *
 * stwlc68 otp, sram driver
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

#include <chipset_common/hwpower/hardware_ic/stwlc68/stwlc68.h>
#include "stwlc68_fw.h"

#define HWLOG_TAG wireless_stwlc68_fw
HWLOG_REGIST();

static void stwlc68_fw_ps_control(struct stwlc68_dev_info *di, int scene, bool flag)
{
	static int ref_cnt;

	hwlog_info("[ps_control] cnt=%d, flag=%d\n", ref_cnt, flag);
	if (flag)
		++ref_cnt;
	else if (--ref_cnt > 0)
		return;

	wlps_control(di->ic_type, scene, flag);
}

static int stwlc68_fw_check_dc_power(struct stwlc68_dev_info *di)
{
	int ret;
	u8 op_mode = 0;

	ret = stwlc68_read_byte(STWLC68_OP_MODE_ADDR, &op_mode);
	if (ret)
		return ret;

	hwlog_info("[check_dc_power] op_mode 0x%02x\n", op_mode);
	if (op_mode != STWLC68_FW_OP_MODE_SA)
		return -EINVAL;

	return 0;
}

static void stwlc68_fw_reset_system(struct stwlc68_dev_info *di)
{
	int ret;
	u8 wr_buff = STWLC68_RST_SYS | STWLC68_RST_M0;

	ret = stwlc68_4addr_write_block(di, STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		hwlog_err("reset_system: ignore i2c error\n");
}

static int stwlc68_fw_disable_i2c(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_FW_I2C_DISABLE;

	return stwlc68_4addr_write_block(di, STWLC68_FW_I2C_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_hold_m0(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_RST_M0;

	return stwlc68_4addr_write_block(di, STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_check_wr(struct stwlc68_dev_info *di, u32 wr_already, u8 *wr_buff,
	u8 *rd_buff, int size_to_wr)
{
	int i;
	int ret;
	u32 bad_addr;

	ret = memcmp(wr_buff, rd_buff, size_to_wr);
	if (!ret)
		return 0;

	for (i = 0; i < size_to_wr; i++) {
		if (wr_buff[i] != rd_buff[i]) {
			bad_addr = wr_already + i;
			hwlog_err("check_wr: wr&&rd data mismatch, bad_addr=0x%x\n", bad_addr);
			if (bad_addr < STWLC68_RAM_FW_CODE_OFFSET)
				continue;
			if (di->g_val.sram_bad_addr >= bad_addr)
				di->g_val.sram_bad_addr = bad_addr;
			return -EINVAL;
		}
	}

	return 0;
}

static int stwlc68_fw_write_data(struct stwlc68_dev_info *di, u32 current_clean_addr,
	const u8 *otp_data, int otp_size)
{
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = current_clean_addr;
	u8 wr_buff[STWLC68_OTP_PROGRAM_WR_SIZE] = { 0 };
	u8 rd_buff[STWLC68_OTP_PROGRAM_WR_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_wr = remaining > STWLC68_OTP_PROGRAM_WR_SIZE ?
			STWLC68_OTP_PROGRAM_WR_SIZE : remaining;
		memcpy(wr_buff, otp_data + wr_already, size_to_wr);
		ret = stwlc68_4addr_write_block(di, addr, wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("write_data: wr failed, addr=0x%08x\n", addr);
			return ret;
		}
		usleep_range(500, 9500); /* 1ms */
		ret = stwlc68_4addr_read_block(di, addr, rd_buff, size_to_wr);
		if (ret) {
			hwlog_err("write_data: rd failed, addr=0x%08x\n", addr);
			return ret;
		}
		ret = stwlc68_fw_check_wr(di, wr_already, wr_buff,
			rd_buff, size_to_wr);
		if (ret)
			break;
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	return 0;
}

static int stwlc68_fw_load_ram_fw(struct stwlc68_dev_info *di, const u8 *ram, int len)
{
	int ret;

	ret = stwlc68_fw_disable_i2c(di);
	if (ret) {
		hwlog_err("load_ram_fw: disable fw_i2c failed\n");
		return ret;
	}
	ret = stwlc68_fw_hold_m0(di);
	if (ret) {
		hwlog_err("load_ram_fw: hold M0 failed\n");
		return ret;
	}

	ret = stwlc68_fw_write_data(di, STWLC68_RAM_FW_START_ADDR, ram, len);
	if (ret) {
		hwlog_err("load_ram_fw: write fw data failed\n");
		return ret;
	}

	return 0;
}

static int stwlc68_fw_ram_check(struct stwlc68_dev_info *di)
{
	int ret;
	u8 *ram = NULL;
	int size = sizeof(u8) * STWLC68_RAM_MAX_SIZE;

	ram = kzalloc(size, GFP_KERNEL);
	if (!ram)
		return -ENOMEM;

	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY);

	memset(ram, STWLC68_RAM_CHECK_VAL1, size);
	ret = stwlc68_fw_load_ram_fw(di, ram, size);
	if (ret) {
		kfree(ram);
		return ret;
	}
	memset(ram, STWLC68_RAM_CHECK_VAL2, size);
	ret = stwlc68_fw_load_ram_fw(di, ram, size);
	if (ret) {
		kfree(ram);
		return ret;
	}

	kfree(ram);
	return 0;
}

static int stwlc68_fw_set_boot_mode(struct stwlc68_dev_info *di, u8 boot_mode)
{
	int ret;

	ret = stwlc68_4addr_write_block(di, STWLC68_BOOT_SET_ADDR,
		&boot_mode, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	stwlc68_fw_reset_system(di);

	return 0;
}

static int stwlc68_fw_show_ram_fw_version(struct stwlc68_dev_info *di)
{
	int ret;
	u16 fw_ver = 0;

	ret = stwlc68_read_word(STWLC68_RAM_FW_VER_ADDR, &fw_ver);
	if (ret)
		return ret;

	hwlog_info("[show_ram_fw_version] 0x%04x\n", fw_ver);
	return 0;
}

static bool stwlc68_fw_is_rom_ok(struct stwlc68_dev_info *di)
{
	int i;
	int ret;
	u8 rd_buff = 0;

	ret = stwlc68_write_byte(STWLC68_OTP_WR_CMD_ADDR,
		STWLC68_ROM_CHECK_VAL);
	if (ret)
		return false;

	/* timeout: 25ms*40 = 1s */
	for (i = 0; i < 40; i++) {
		ret = stwlc68_4addr_read_block(di, STWLC68_ROM_CHECK_ADDR,
			&rd_buff, STWLC68_FW_ADDR_LEN);
		if (ret)
			return false;
		if (!rd_buff)
			goto check_sys_err;
		msleep(25);
	}

	return false;

check_sys_err:
	ret = stwlc68_read_byte(STWLC68_SYSTEM_ERR_ADDR, &rd_buff);
	if (ret)
		return false;
	if (!rd_buff)
		return true;
	return false;
}

static int stwlc68_fw_rom_check(struct stwlc68_dev_info *di)
{
	int ret;

	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY);

	ret = stwlc68_fw_load_ram_fw(di, g_stwlc68_ram_data,
		ARRAY_SIZE(g_stwlc68_ram_data));
	if (ret) {
		hwlog_err("rom_check: load sram failed\n");
		return ret;
	}

	ret = stwlc68_fw_set_boot_mode(di, STWLC68_BOOT_FROM_RAM);
	if (ret) {
		hwlog_err("rom_check: boot from sram failed\n");
		return ret;
	}

	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY);

	ret = stwlc68_fw_show_ram_fw_version(di);
	if (ret) {
		hwlog_err("rom_check: show sram version failed\n");
		goto check_fail;
	}

	if (!stwlc68_fw_is_rom_ok(di)) {
		hwlog_err("rom_check: rom is bad\n");
		goto check_fail;
	}

	(void)stwlc68_fw_set_boot_mode(di, STWLC68_BOOT_FROM_ROM);
	return 0;

check_fail:
	(void)stwlc68_fw_set_boot_mode(di, STWLC68_BOOT_FROM_ROM);
	return ret;
}

static int stwlc68_fw_system_check(struct stwlc68_dev_info *di)
{
	int ret;

	if (di->g_val.ram_rom_status == STWLC68_RAM_ROM_STATUS_OK)
		return 0;

	ret = stwlc68_fw_ram_check(di);
	if (ret) {
		hwlog_err("system_check: ram check failed\n");
		di->g_val.ram_rom_status = STWLC68_RAM_ROM_STATUS_BAD;
		return ret;
	}

	ret = stwlc68_fw_rom_check(di);
	if (ret) {
		hwlog_err("system_check: rom check failed\n");
		di->g_val.ram_rom_status = STWLC68_RAM_ROM_STATUS_BAD;
		return ret;
	}

	di->g_val.ram_rom_status = STWLC68_RAM_ROM_STATUS_OK;
	return 0;
}

static int stwlc68_fw_program_otp_pre_check(struct stwlc68_dev_info *di)
{
	int ret;

	ret = stwlc68_fw_check_dc_power(di);
	if (ret) {
		hwlog_err("program_otp_pre_check: OTP must be programmed in DC power\n");
		return ret;
	}

	ret = stwlc68_fw_system_check(di);
	if (ret) {
		hwlog_err("program_otp_pre_check: system check failed\n");
		return ret;
	}

	return 0;
}

static int stwlc68_fw_get_clean_addr_pointer(struct stwlc68_dev_info *di, u32 *clean_addr_pointer)
{
	u8 cut_id = 0;
	int ret;

	if (!clean_addr_pointer)
		return -EINVAL;

	ret = stwlc68_get_cut_id(di, &cut_id);
	if (ret) {
		hwlog_err("get_clean_addr_pointer: get cut_id failed\n");
		return ret;
	}

	if (cut_id == STWLC68_CUT_ID_V10)
		*clean_addr_pointer = STWLC68_CLEAN_ADDR_REV0;
	else if (cut_id >= STWLC68_CUT_ID_V11)
		*clean_addr_pointer = STWLC68_CLEAN_ADDR_REV1;
	else
		return -EINVAL;

	return 0;
}

static void stwlc68_fw_get_clean_addr(struct stwlc68_dev_info *di, u32 *clean_addr)
{
	int ret;
	u32 clean_addr_pointer = 0;
	u8 rd_buff[STWLC68_4ADDR_LEN] = { 0 };

	ret = stwlc68_fw_get_clean_addr_pointer(di, &clean_addr_pointer);
	if (ret) {
		hwlog_err("get_clean_addr: get clean_addr_addr failed\n");
		return;
	}

	ret = stwlc68_4addr_read_block(di, clean_addr_pointer,
		rd_buff, STWLC68_4ADDR_LEN);
	if (ret)
		return;

	*clean_addr = (u32)((rd_buff[0] << 0) | (rd_buff[1] << 8) |
		(rd_buff[2] << 16) | (rd_buff[3] << 24));
	hwlog_info("[get_clean_addr] clean addr: 0x%08x\n", *clean_addr);
}

static int stwlc68_fw_program_otp_choose_otp(struct stwlc68_dev_info *di, int *otp_id)
{
	int i;
	int ret;
	u8 cut_id = 0;
	int otp_num;

	ret = stwlc68_get_cut_id(di, &cut_id);
	if (ret) {
		hwlog_err("program_otp_choose_otp: get cut_id failed\n");
		return ret;
	}
	hwlog_info("[program_otp_choose_otp] cut_id = 0x%x\n", cut_id);
	/* determine what has to be programmed depending on version ids */
	otp_num = ARRAY_SIZE(stwlc68_otp_info);
	for (i = 0; i < otp_num; i++) {
		if (stwlc68_otp_info[i].dev_type != di->dev_type)
			continue;
		if ((cut_id >= stwlc68_otp_info[i].cut_id_from) &&
			(cut_id <= stwlc68_otp_info[i].cut_id_to)) {
			*otp_id = i;
			return 0;
		}
	}

	hwlog_err("program_otp_choose_otp: cut_id mismatch\n");
	return -EINVAL;
}

static int stwlc68_fw_program_otp_calc_otp_addr(struct stwlc68_dev_info *di, int otp_id,
	const u8 **data_to_program, u32 *data_to_program_size)
{
	int ret;
	u16 cfg_id = 0;
	u16 patch_id = 0;
	int cfg_id_mismatch = 0;
	int patch_id_mismatch = 0;

	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("calc_otp_addr: get cfg_id failed\n");
		return ret;
	}
	if (cfg_id != stwlc68_otp_info[otp_id].cfg_id) {
		hwlog_err("calc_otp_addr: cfg_id mismatch, running|latest:0x%x|0x%x\n",
			cfg_id, stwlc68_otp_info[otp_id].cfg_id);
		cfg_id_mismatch = 1;
	}
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("calc_otp_addr: get patch_id failed\n");
		return ret;
	}
	if (patch_id != stwlc68_otp_info[otp_id].patch_id) {
		hwlog_err("calc_otp_addr: patch_id mismatch, running|latest:0x%x|0x%x\n",
			patch_id, stwlc68_otp_info[otp_id].patch_id);
		patch_id_mismatch = 1;
	}
	if (cfg_id_mismatch && patch_id_mismatch) {
		*data_to_program_size = stwlc68_otp_info[otp_id].cfg_size +
			stwlc68_otp_info[otp_id].patch_size + STWLC68_OTP_DUMMY_LEN;
		*data_to_program = stwlc68_otp_info[otp_id].otp_arr;
	} else if (cfg_id_mismatch && !patch_id_mismatch) {
		*data_to_program_size = stwlc68_otp_info[otp_id].cfg_size;
		*data_to_program = stwlc68_otp_info[otp_id].otp_arr;
	} else if (!cfg_id_mismatch && patch_id_mismatch) {
		*data_to_program_size = stwlc68_otp_info[otp_id].patch_size;
		*data_to_program = stwlc68_otp_info[otp_id].otp_arr +
			stwlc68_otp_info[otp_id].cfg_size;
	} else {
		hwlog_info("[calc_otp_addr] cfg && patch are latest\n");
		return -EINVAL;
	}

	return 0;
}

static int stwlc68_fw_program_otp_check_free_addr(u32 current_clean_addr, u32 data_to_program_size)
{
	hwlog_info("[program_otp_check_free_addr] opt_max_size: 0x%04x, otp_size: 0x%04x\n",
		STWLC68_OTP_MAX_SIZE, data_to_program_size);
	if ((STWLC68_OTP_MAX_SIZE - (current_clean_addr & WORD_MASK)) <
		data_to_program_size) {
		hwlog_err("program_otp_check_free_addr: not enough space available\n");
		return -ENOMEM;
	}

	return 0;
}

static int stwlc68_fw_update_clean_addr(struct stwlc68_dev_info *di, u32 clean_addr)
{
	int ret;

	ret = stwlc68_write_word(STWLC68_FW_CLEAN_ADDR_ADDR,
		(u16)(clean_addr & WORD_MASK));
	if (ret)
		return ret;

	ret = stwlc68_write_byte(STWLC68_OTP_WR_CMD_ADDR,
		STWLC68_CLEAN_ADDR_UPDATE_VAL);
	if (ret)
		return ret;

	return 0;
}

static int stwlc68_fw_disable_iload(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_ILOAD_DISABLE_VALUE;

	return stwlc68_4addr_write_block(di, STWLC68_ILOAD_DRIVE_ADDR,
		&wr_buff, STWLC68_ILOAD_DATA_LEN);
}

static int stwlc68_fw_write_otp_len(struct stwlc68_dev_info *di, u16 len)
{
	hwlog_info("[write_otp_len] len=%d bytes\n", len);

	return stwlc68_write_word(STWLC68_OTP_WRITE_LENGTH_ADDR, len);
}

static int stwlc68_fw_exe_wr_otp_cmd(struct stwlc68_dev_info *di)
{
	return stwlc68_write_byte(STWLC68_OTP_WR_CMD_ADDR, STWLC68_OTP_ENABLE);
}

static bool stwlc68_fw_is_otp_successfully_written(struct stwlc68_dev_info *di)
{
	int i;
	int ret;

	if (!di)
		return false;

	/* timeout: 500ms*20=10s */
	for (i = 0; i < 20; i++) {
		msleep(500);
		if (!gpio_get_value(di->gpio_int)) {
			ret = stwlc68_write_byte(STWLC68_GPIO_STATUS_ADDR,
				STWLC68_GPIO_RESET_VAL);
			if (ret) {
				hwlog_err("is_otp_successfully_written: reset gpio failed\n");
				return false;
			}
			hwlog_info("[is_otp_successfully_written] succ, cnt=%d\n", i);
			return true;
		}
	}

	return false;
}

static int stwlc68_fw_write_otp_data(struct stwlc68_dev_info *di, const u8 *otp_data, int otp_size)
{
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u8 *wr_buff = NULL;

	wr_buff = kzalloc(sizeof(u8) * STWLC68_OTP_WR_BLOCK_SIZE, GFP_KERNEL);
	if (!wr_buff)
		return -ENOMEM;

	while (remaining > 0) {
		hwlog_info("[write_otp_data] gpio_int val = %d\n",
			gpio_get_value(di->gpio_int));
		size_to_wr = remaining > STWLC68_OTP_WR_BLOCK_SIZE ?
			STWLC68_OTP_WR_BLOCK_SIZE : remaining;
		ret = stwlc68_fw_write_otp_len(di, size_to_wr);
		if (ret) {
			hwlog_err("write_otp_data: wr otp len failed\n");
			goto write_fail;
		}
		memcpy(wr_buff, otp_data + wr_already, size_to_wr);
		ret = stwlc68_fw_write_data(di, STWLC68_OTP_FW_START_ADDR,
			wr_buff, size_to_wr);
		if (ret) {
			hwlog_err("write_otp_data: wr otp data failed\n");
			goto write_fail;
		}
		ret = stwlc68_fw_exe_wr_otp_cmd(di);
		if (ret) {
			hwlog_err("write_otp_data: wr otp data failed\n");
			goto write_fail;
		}
		wr_already += size_to_wr;
		remaining -= size_to_wr;
		if (!stwlc68_fw_is_otp_successfully_written(di))
			goto write_fail;
	}

	kfree(wr_buff);
	return 0;

write_fail:
	kfree(wr_buff);
	return -EIO;
}

static int stwlc68_fw_get_iload(struct stwlc68_dev_info *di)
{
	int ret;
	u8 iload_value = 0;

	ret = stwlc68_4addr_read_block(di, STWLC68_ILOAD_DRIVE_ADDR,
		&iload_value, STWLC68_ILOAD_DATA_LEN);
	if (ret)
		return ret;
	hwlog_info("[get_iload] iload_value=0x%x\n", iload_value);

	return 0;
}

static int stwlc68_fw_check_system_err(struct stwlc68_dev_info *di)
{
	int ret;
	u8 err = 0;

	ret = stwlc68_read_byte(STWLC68_SYSTEM_ERR_ADDR, &err);
	if (ret)
		return ret;

	if (err) {
		hwlog_err("check_system_err: err=0x%x\n", err);
		return -EINVAL;
	}

	return 0;
}

static int stwlc68_fw_program_otp_post_check(struct stwlc68_dev_info *di, int otp_id)
{
	int ret;
	u16 cfg_id = 0;
	u16 patch_id = 0;

	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("program_otp_post_check: get cfg_id failed\n");
		return ret;
	}
	if (cfg_id != stwlc68_otp_info[otp_id].cfg_id)
		hwlog_err("program_otp_post_check: cfg_id(0x%x) mismatch after flashing\n", cfg_id);

	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("program_otp_post_check: get patch_id failed\n");
		return ret;
	}
	if (patch_id != stwlc68_otp_info[otp_id].patch_id)
		hwlog_err("program_otp_post_check: patch_id(0x%x) mismatch after flashing\n",
			patch_id);

	return 0;
}

static int stwlc68_fw_program_otp(struct stwlc68_dev_info *di)
{
	int ret;
	int otp_id = 0;
	u32 current_clean_addr = 0;
	u32 data_to_program_size = 0;
	const u8 *data_to_program = NULL;

	stwlc68_disable_irq_nosync(di);
	stwlc68_fw_ps_control(di, WLPS_PROC_OTP_PWR, true);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */

	ret = stwlc68_fw_program_otp_pre_check(di);
	if (ret)
		goto exit;
	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	stwlc68_fw_get_clean_addr(di, &current_clean_addr);
	ret = stwlc68_fw_program_otp_choose_otp(di, &otp_id);
	if (ret)
		goto exit;
	hwlog_info("[program_otp] otp_index = %d\n", otp_id);
	ret = stwlc68_fw_program_otp_calc_otp_addr(di, otp_id,
		&data_to_program, &data_to_program_size);
	if (ret)
		goto exit;
	ret = stwlc68_fw_load_ram_fw(di, g_stwlc68_ram_data,
		ARRAY_SIZE(g_stwlc68_ram_data));
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_set_boot_mode(di, STWLC68_BOOT_FROM_RAM);
	if (ret)
		goto set_mode_exit;
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	ret = stwlc68_fw_show_ram_fw_version(di);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_program_otp_check_free_addr(STWLC68_OTP_CLEAN_ADDR,
		data_to_program_size);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_update_clean_addr(di, STWLC68_OTP_CLEAN_ADDR);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_disable_iload(di);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_write_otp_data(di, data_to_program, data_to_program_size);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_get_iload(di);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_check_system_err(di);
	if (ret)
		goto set_mode_exit;
	ret = stwlc68_fw_set_boot_mode(di, STWLC68_BOOT_FROM_ROM);
	if (ret)
		goto exit;
	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	ret = stwlc68_fw_program_otp_post_check(di, otp_id);
	if (ret)
		goto exit;
	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */

	stwlc68_fw_ps_control(di, WLPS_PROC_OTP_PWR, false);
	stwlc68_enable_irq(di);
	hwlog_info("[program_otp] succ\n");
	return 0;

set_mode_exit:
	(void)stwlc68_fw_set_boot_mode(di, STWLC68_BOOT_FROM_ROM);
exit:
	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	stwlc68_fw_ps_control(di, WLPS_PROC_OTP_PWR, false);
	stwlc68_enable_irq(di);
	hwlog_err("program_otp: failed\n");
	return ret;
}

static int stwlc68_fw_recover_otp_pre_check(struct stwlc68_dev_info *di)
{
	int ret;

	ret = stwlc68_fw_check_dc_power(di);
	if (ret) {
		hwlog_err("recover_otp_pre_check: OTP must be programmed in DC power\n");
		return ret;
	}

	ret = stwlc68_fw_system_check(di);
	if (ret) {
		hwlog_err("recover_otp_pre_check: system check failed\n");
		return ret;
	}

	return 0;
}

static int stwlc68_fw_osc_trim(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_OSC_TRIM_VAL;

	return stwlc68_4addr_write_block(di, STWLC68_SYSTEM_OSC_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_clk_div(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_CLK_DIV_VAL;

	return stwlc68_4addr_write_block(di, STWLC68_SYSTEM_CLK_DIV_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_reset_otp_block(struct stwlc68_dev_info *di)
{
	u8 rd_buff = 0;
	u8 wr_buff;
	u8 reset_reg;
	int ret;

	ret = stwlc68_4addr_read_block(di, STWLC68_RST_ADDR,
		&rd_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	reset_reg = rd_buff;
	reset_reg = reset_reg | (1 << 4);
	wr_buff = reset_reg;

	ret = stwlc68_4addr_write_block(di, STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	reset_reg = reset_reg & ~(1 << 4);
	wr_buff = reset_reg;

	return stwlc68_4addr_write_block(di, STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_enable_otp(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_OTP_ENABLE;

	return stwlc68_4addr_write_block(di, STWLC68_OTP_ENABLE_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_unlock_otp(struct stwlc68_dev_info *di)
{
	u8 wr_buff = STWLC68_OTP_UNLOCK_CODE;

	return stwlc68_4addr_write_block(di, STWLC68_OTP_PROGRAM_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
}

static int stwlc68_fw_config_otp_registers(struct stwlc68_dev_info *di)
{
	int ret;
	u32 addr;
	u8 write_buff;

	addr = STWLC68_PGM_IPS_MRR_HI_ADDR;
	write_buff = STWLC68_PGM_IPS_MRR_HI_VAL;
	ret = stwlc68_4addr_write_block(di, addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	addr = STWLC68_RD_VERIFY1_MREF_HI_ADDR;
	write_buff = STWLC68_RD_VERIFY1_MREF_VAL;
	ret = stwlc68_4addr_write_block(di, addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	addr = STWLC68_RD_VERIFY_OTP_MREF_ADDR;
	write_buff = STWLC68_RD_VERIFY2_OTP_HI_VAL;
	ret = stwlc68_4addr_write_block(di, addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	addr = STWLC68_OTP_FREQ_CTRL_ADDR;
	write_buff = STWLC68_OTP_FREQ_CTRL_VAL;
	ret = stwlc68_4addr_write_block(di, addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	return 0;
}

static int stwlc68_fw_recover_otp_prepare(struct stwlc68_dev_info *di)
{
	int ret;

	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY);
	ret = stwlc68_fw_disable_i2c(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: disable fw_i2c failed\n");
		return ret;
	}
	ret = stwlc68_fw_hold_m0(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: hold M0 failed\n");
		return ret;
	}
	msleep(STWLC68_OTP_OPS_DELAY);
	ret = stwlc68_fw_osc_trim(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: hold M0 failed\n");
		return ret;
	}
	ret = stwlc68_fw_clk_div(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: hold M0 failed\n");
		return ret;
	}
	ret = stwlc68_fw_reset_otp_block(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: OTP block reset failed\n");
		return ret;
	}
	ret = stwlc68_fw_enable_otp(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: OTP enable failed\n");
		return ret;
	}
	ret = stwlc68_fw_unlock_otp(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: unlock OTP failed\n");
		return ret;
	}
	ret = stwlc68_fw_config_otp_registers(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: config otp registers failed\n");
		return ret;
	}
	ret = stwlc68_fw_disable_iload(di);
	if (ret) {
		hwlog_err("recover_otp_prepare: disable otp iload failed\n");
		return ret;
	}

	return 0;
}

static bool stwlc68_fw_is_otp_err(struct stwlc68_dev_info *di)
{
	int ret;
	u8 err_status = 0;

	ret = stwlc68_4addr_read_block(di, STWLC68_OTP_ERR_STATUS_ADDR,
		&err_status, STWLC68_4ADDR_LEN);
	if (ret)
		return true;

	if (err_status) {
		hwlog_err("is_otp_err: status=0x%02x\n", err_status);
		if (di->g_val.otp_skip_soak_recovery_flag &&
			(err_status == 0x08)) /* 0x08:user compare error */
			return false;
		return true;
	}

	return false;
}

static int stwlc68_fw_recover_otp_data(struct stwlc68_dev_info *di, u32 current_clean_addr,
	const u8 *otp_data, int otp_size, u8 *dirty_otp_id, int dirty_otp_size)
{
	int id = 0;
	int ret;
	int remaining = otp_size;
	int size_to_wr;
	u32 wr_already = 0;
	u32 addr = current_clean_addr;
	u8 buff[STWLC68_OTP_RECOVER_WR_SIZE] = { 0 };

	while ((remaining > 0) && (id < dirty_otp_size)) {
		size_to_wr = remaining > STWLC68_OTP_RECOVER_WR_SIZE ?
			STWLC68_OTP_RECOVER_WR_SIZE : remaining;
		if (dirty_otp_id[id]) {
			memcpy(buff, otp_data + wr_already, size_to_wr);
			ret = stwlc68_4addr_write_block(di, addr, buff, size_to_wr);
			if (ret) {
				hwlog_err("recover_otp_data: failed, addr=0x%08x\n", addr);
				return ret;
			}
			usleep_range(9500, 10500); /* 10ms */
			if (!stwlc68_fw_is_otp_err(di))
				dirty_otp_id[id] = 0;
		}
		id++;
		addr += size_to_wr;
		wr_already += size_to_wr;
		remaining -= size_to_wr;
	}

	for (id = 0; id < otp_size / STWLC68_OTP_RECOVER_WR_SIZE; id++) {
		if ((id >= dirty_otp_size) || dirty_otp_id[id])
			return -EINVAL;
	}

	return 0;
}

static int stwlc68_fw_skip_soak_recover_otp(struct stwlc68_dev_info *di, u32 err_addr,
	const u8 *otp_data, int otp_size, u8 *dirty_otp_id, int dirty_otp_size)
{
	int ret;
	u32 addr;
	u8 write_buff;

	addr = STWLC68_OTP_SKIP_SOAK_ADDR;
	write_buff = STWLC68_OTP_SKIP_SOAK_VAL;
	ret = stwlc68_4addr_write_block(di, addr, &write_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		return ret;

	return stwlc68_fw_recover_otp_data(di, err_addr, otp_data, otp_size,
		dirty_otp_id, dirty_otp_size);
}

static void stwlc68_fw_soft_reset_system(struct stwlc68_dev_info *di)
{
	int ret;
	u8 wr_buff = STWLC68_RST_SYS;

	ret = stwlc68_4addr_write_block(di, STWLC68_RST_ADDR,
		&wr_buff, STWLC68_FW_ADDR_LEN);
	if (ret)
		hwlog_err("soft_reset_system: ignore i2c error\n");
}

static bool stwlc68_fw_is_otp_corrupt(struct stwlc68_dev_info *di)
{
	int ret;
	u8 err_status = 0;

	ret = stwlc68_4addr_read_block(di, STWLC68_OTP_ERR_STATUS_ADDR,
		&err_status, STWLC68_4ADDR_LEN);
	if (ret)
		return true;

	if (err_status & STWLC68_OTP_ERR_CORRUPT) {
		hwlog_err("is_otp_corrupt: status=0x%02x\n", err_status);
		return true;
	}

	return false;
}

static int stwlc68_fw_recover_otp_post_check(struct stwlc68_dev_info *di, int otp_id)
{
	int ret;

	ret = stwlc68_fw_program_otp_post_check(di, otp_id);
	if (ret)
		return ret;

	if (stwlc68_fw_is_otp_corrupt(di))
		return -EINVAL;

	return 0;
}

static int stwlc68_fw_recover_otp(struct stwlc68_dev_info *di)
{
	int ret;
	int otp_id = 0;
	u32 data_to_program_size;
	const u8 *data_to_program = NULL;
	u8 *dirty_otp_id = NULL;
	int dirty_otp_size = STWLC68_OTP_MAX_SIZE / STWLC68_OTP_RECOVER_WR_SIZE;

	stwlc68_disable_irq_nosync(di);
	wlps_control(di->ic_type, WLPS_RECOVER_OTP_PWR, true);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */

	ret = stwlc68_fw_recover_otp_pre_check(di);
	if (ret)
		goto exit;
	ret = stwlc68_fw_program_otp_choose_otp(di, &otp_id);
	if (ret)
		goto exit;
	hwlog_info("[recover_otp] otp_index = %d\n", otp_id);
	data_to_program = stwlc68_otp_info[otp_id].otp_arr;
	data_to_program_size = stwlc68_otp_info[otp_id].cfg_size +
		stwlc68_otp_info[otp_id].patch_size + STWLC68_OTP_DUMMY_LEN;
	ret = stwlc68_fw_recover_otp_prepare(di);
	if (ret)
		goto exit;
	di->g_val.otp_skip_soak_recovery_flag = false;
	dirty_otp_id = kzalloc(sizeof(u8) * dirty_otp_size, GFP_KERNEL);
	if (!dirty_otp_id)
		goto exit;
	memset(dirty_otp_id, 1, dirty_otp_size);
	ret = stwlc68_fw_recover_otp_data(di, STWLC68_OTP_CLEAN_ADDR,
		data_to_program, data_to_program_size, dirty_otp_id, dirty_otp_size);
	hwlog_info("[recover_otp] result = %d\n", ret);
	if (ret) {
		di->g_val.otp_skip_soak_recovery_flag = true;
		ret = stwlc68_fw_recover_otp_prepare(di);
		if (ret)
			goto exit;
		ret = stwlc68_fw_skip_soak_recover_otp(di, STWLC68_OTP_CLEAN_ADDR,
			data_to_program, data_to_program_size, dirty_otp_id, dirty_otp_size);
		if (ret)
			goto exit;
	}
	ret = stwlc68_fw_get_iload(di);
	if (ret)
		goto exit;

	stwlc68_fw_soft_reset_system(di);

	msleep(200); /* delay 200ms for system reset */
	ret = stwlc68_fw_recover_otp_post_check(di, otp_id);
	if (ret)
		goto exit;
	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	wlps_control(di->ic_type, WLPS_RECOVER_OTP_PWR, false);
	stwlc68_enable_irq(di);
	kfree(dirty_otp_id);
	hwlog_info("[recover_otp] succ\n");
	return 0;

exit:
	stwlc68_fw_reset_system(di);
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for system reset */
	wlps_control(di->ic_type, WLPS_RECOVER_OTP_PWR, false);
	stwlc68_enable_irq(di);
	kfree(dirty_otp_id);
	hwlog_err("recover_otp: failed\n");
	return ret;
}

static int stwlc68_fw_rx_program_otp(unsigned int proc_type, void *dev_data)
{
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	switch (proc_type) {
	case WIRELESS_PROGRAM_FW:
		return stwlc68_fw_program_otp(di);
	case WIRELESS_RECOVER_FW:
		return stwlc68_fw_recover_otp(di);
	default:
		break;
	}

	return -EINVAL;
}

static int stwlc68_fw_check_otp(void *dev_data)
{
	int ret, i, otp_num;
	u8 cut_id = 0;
	u16 cfg_id = 0;
	u16 patch_id = 0;
	struct stwlc68_dev_info *di = dev_data;

	if (!di)
		return -ENODEV;

	stwlc68_fw_ps_control(di, WLPS_PROC_OTP_PWR, true);
	(void)stwlc68_chip_enable(RX_EN_ENABLE, di); /* enable RX for i2c WR */
	msleep(STWLC68_OTP_OPS_DELAY); /* delay for power on */
	ret = stwlc68_fw_system_check(di);
	if (ret) {
		hwlog_err("check_otp: system check failed\n");
		goto exit;
	}
	stwlc68_fw_reset_system(di);
	msleep(100);
	ret = stwlc68_get_cut_id(di, &cut_id);
	if (ret) {
		hwlog_err("check_otp: get cut_id failed\n");
		goto exit;
	}
	hwlog_info("[check_otp] cut_id = 0x%x\n", cut_id);
	otp_num = ARRAY_SIZE(stwlc68_otp_info);
	for (i = 0; i < otp_num; i++) {
		if (stwlc68_otp_info[i].dev_type != di->dev_type)
			continue;
		if ((cut_id >= stwlc68_otp_info[i].cut_id_from) &&
			(cut_id <= stwlc68_otp_info[i].cut_id_to))
			break;
	}
	if (i == otp_num) {
		hwlog_err("check_otp: cut_id mismatch\n");
		ret = -EINVAL;
		goto exit;
	}
	ret = stwlc68_get_patch_id(&patch_id);
	if (ret) {
		hwlog_err("check_otp: get patch_id failed\n");
		goto exit;
	}
	ret = stwlc68_get_cfg_id(&cfg_id);
	if (ret) {
		hwlog_err("check_otp: get cfg_id failed\n");
		goto exit;
	}

	if ((patch_id != stwlc68_otp_info[i].patch_id) ||
		(cfg_id != stwlc68_otp_info[i].cfg_id))
		ret = -EINVAL;
exit:
	stwlc68_fw_ps_control(di, WLPS_PROC_OTP_PWR, false);
	return ret;
}

static int stwlc68_fw_get_otp_status(unsigned int *status, void *dev_data)
{
	int ret;
	struct stwlc68_dev_info *di = dev_data;

	if (!di || !status)
		return -EINVAL;

	ret = stwlc68_fw_check_otp(di);
	if (!ret)
		*status = WIRELESS_FW_PROGRAMED;
	else
		*status = WIRELESS_FW_ERR_PROGRAMED;

	return 0;
}

void stwlc68_fw_sram_scan_work(struct work_struct *work)
{
	struct stwlc68_dev_info *di =
		container_of(work, struct stwlc68_dev_info, sram_scan_work.work);

	if (!di) {
		hwlog_err("sram_scan_work: di null\n");
		return;
	}
	if (power_cmdline_is_factory_mode()) {
		di->g_val.sram_chk_complete = true;
		return;
	}

	hwlog_info("[sram_scan_work] sram scan begin\n");
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, true);
	usleep_range(9500, 10500); /* wait 10ms for power supply */
	(void)stwlc68_fw_ram_check(di);
	wlps_control(di->ic_type, WLPS_RX_EXT_PWR, false);
	hwlog_info("[sram_scan_work] sram scan end\n");

	di->g_val.sram_chk_complete = true;
}

static int stwlc68_fw_find_sram_id(struct stwlc68_dev_info *di,
	u32 sram_mode, unsigned int *sram_id)
{
	int i;
	int ret;
	unsigned int fw_sram_num;
	struct stwlc68_chip_info chip_info;

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("find_sram_id: get chip info failed\n");
		return ret;
	}

	fw_sram_num = ARRAY_SIZE(stwlc68_sram);
	for (i = 0; i < fw_sram_num; i++) {
		if (sram_mode != stwlc68_sram[i].fw_sram_mode)
			continue;
		if ((chip_info.cut_id < stwlc68_sram[i].cut_id_from) ||
			(chip_info.cut_id > stwlc68_sram[i].cut_id_to))
			continue;
		if ((chip_info.cfg_id < stwlc68_sram[i].cfg_id_from) ||
			(chip_info.cfg_id > stwlc68_sram[i].cfg_id_to))
			continue;
		if ((chip_info.patch_id < stwlc68_sram[i].patch_id_from) ||
			(chip_info.patch_id > stwlc68_sram[i].patch_id_to))
			continue;
		if (!power_cmdline_is_factory_mode() &&
			((di->g_val.sram_bad_addr < stwlc68_sram[i].bad_addr_from) ||
			(di->g_val.sram_bad_addr > stwlc68_sram[i].bad_addr_to)))
			continue;
		if ((stwlc68_sram[i].sram_size <= ST_RAMPATCH_HEADER_SIZE) ||
			(stwlc68_sram[i].sram_size > ST_RAMPATCH_MAX_SIZE))
			continue;
		if (stwlc68_sram[i].dev_type != di->dev_type)
			continue;

		hwlog_info("[find_sram_id] bad_addr=0x%x sram_id=%d\n",
			di->g_val.sram_bad_addr, i);
		*sram_id = i;
		return 0;
	}

	return -EINVAL;
}

static bool stwlc68_fw_is_updated_sram_match(struct stwlc68_dev_info *di, u16 sram_id)
{
	int ret;
	struct stwlc68_chip_info chip_info;

	ret = stwlc68_get_chip_info(&chip_info);
	if (ret) {
		hwlog_err("is_updated_sram_match: get chip info failed\n");
		return false;
	}

	hwlog_info("[is_updated_sram_match] sram_id = 0x%x\n", chip_info.sram_id);
	return (sram_id == chip_info.sram_id);
}

static int stwlc68_fw_pre_sramupdate(u32 sram_mode)
{
	int ret;

	if (sram_mode == WIRELESS_TX) {
		ret = stwlc68_sw2tx();
		if (ret)
			return ret;

		msleep(50); /* delay 50ms for switching to tx mode */
	}

	return 0;
}

static int stwlc68_fw_write_data_to_sram(struct stwlc68_dev_info *di, u32 start_addr,
	const u8 *sram_data, const int sram_size)
{
	int ret;
	int remaining = sram_size;
	int size_to_write;
	u32 written_already = 0;
	int address = start_addr;
	u8 buff[ST_RAMPATCH_PAGE_SIZE] = { 0 };

	while (remaining > 0) {
		size_to_write = remaining > ST_RAMPATCH_PAGE_SIZE ?
			ST_RAMPATCH_PAGE_SIZE : remaining;
		memcpy(buff, sram_data + written_already, size_to_write);
		ret = stwlc68_4addr_write_block(di, address, buff, size_to_write);
		if (ret) {
			hwlog_err("write_data_to_sram: failed, addr=0x%8x\n", address);
			return ret;
		}
		address += size_to_write;
		written_already += size_to_write;
		remaining -= size_to_write;
	}

	return 0;
}

static int stwlc68_fw_write_sram_start_addr(struct stwlc68_dev_info *di, u32 start_addr)
{
	return stwlc68_write_block(di, STWLC68_SRAM_START_ADDR_ADDR,
		(u8 *)&start_addr, STWLC68_SRAM_START_ADDR_LEN);
}

static int stwlc68_fw_write_sram_exe_cmd(struct stwlc68_dev_info *di)
{
	return stwlc68_write_byte(STWLC68_EXE_SRAM_ADDR, STWLC68_EXE_SRAM_CMD);
}

static int stwlc68_fw_program_sramupdate(struct stwlc68_dev_info *di,
	const struct stwlc68_sram_info *sram_info)
{
	int ret;
	u32 start_addr;

	/* start_addr obtained from sram_data[4:7] in little endian */
	start_addr = (u32)((sram_info->sram_data[4] << 0) |
		(sram_info->sram_data[5] << 8) |
		(sram_info->sram_data[6] << 16) |
		(sram_info->sram_data[7] << 24));

	ret = stwlc68_fw_write_data_to_sram(di, start_addr,
		sram_info->sram_data + ST_RAMPATCH_HEADER_SIZE,
		sram_info->sram_size - ST_RAMPATCH_HEADER_SIZE);
	if (ret)
		return ret;

	ret = stwlc68_fw_write_sram_start_addr(di, start_addr + 1);
	if (ret)
		return ret;

	ret = stwlc68_fw_write_sram_exe_cmd(di);
	if (ret)
		return ret;

	mdelay(5); /* delay 5ms for exe cmd */
	if (stwlc68_fw_is_updated_sram_match(di, sram_info->sram_id))
		return 0;

	mdelay(5); /* delay 5ms for exe cmd */
	if (!stwlc68_fw_is_updated_sram_match(di, sram_info->sram_id))
		return -EINVAL;

	return 0;
}

int stwlc68_fw_sram_update(struct stwlc68_dev_info *di, u32 sram_mode)
{
	int ret;
	unsigned int fw_sram_id = 0;
	unsigned int fw_sram_num;
	u8 chip_info[WLTRX_IC_CHIP_INFO_LEN] = { 0 };

	if (!di) {
		hwlog_err("sram_update: di null\n");
		return -ENODEV;
	}

	ret = stwlc68_get_chip_info_str(chip_info, WLTRX_IC_CHIP_INFO_LEN, NULL);
	if (ret)
		hwlog_err("sram_update: get chip_info failed\n");

	ret = stwlc68_fw_find_sram_id(di, sram_mode, &fw_sram_id);
	if (ret) {
		hwlog_err("sram_update: sram no need update\n");
		return ret;
	}

	fw_sram_num = ARRAY_SIZE(stwlc68_sram);
	if (fw_sram_id >= fw_sram_num) {
		hwlog_err("sram_update: fw_sram_id=%d err\n", fw_sram_id);
		return -EINVAL;
	}

	if (stwlc68_fw_is_updated_sram_match(di, stwlc68_sram[fw_sram_id].sram_id))
		return 0;

	ret = stwlc68_fw_pre_sramupdate(sram_mode);
	if (ret) {
		hwlog_err("sram_update: pre sramupdate failed\n");
		return ret;
	}

	ret = stwlc68_fw_program_sramupdate(di, &stwlc68_sram[fw_sram_id]);
	if (ret) {
		hwlog_err("sram_update: sram update failed\n");
		return ret;
	}

	ret = stwlc68_get_chip_info_str(chip_info, WLTRX_IC_CHIP_INFO_LEN, NULL);
	if (ret)
		hwlog_err("get chip_info failed\n");

	hwlog_info("[sram_update] sram update succ\n");

	return 0;
}

static struct wireless_fw_ops g_stwlc68_fw_ops = {
	.ic_name                = "stwlc68",
	.program_fw             = stwlc68_fw_rx_program_otp,
	.get_fw_status          = stwlc68_fw_get_otp_status,
	.check_fw               = stwlc68_fw_check_otp,
};

int stwlc68_fw_ops_register(struct stwlc68_dev_info *di)
{
	if (!di)
		return -ENODEV;

	g_stwlc68_fw_ops.dev_data = (void *)di;

	di->g_val.ram_rom_status = STWLC68_RAM_ROM_STATUS_UNKNOWN;
	di->g_val.sram_bad_addr = STWLC68_RAM_MAX_SIZE;

	return wireless_fw_ops_register(&g_stwlc68_fw_ops);
}
