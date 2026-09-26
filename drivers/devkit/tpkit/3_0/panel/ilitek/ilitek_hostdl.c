/*
 * ILITEK Touch IC driver
 *
 * Copyright (C) 2011 ILI Technology Corporation.
 *
 * Author: Dicky Chiang <dicky_chiang@ilitek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "ilitek.h"

#define UPDATE_PASS 0
#define UPDATE_FAIL -1
#define TIMEOUT_SECTOR 500
#define TIMEOUT_PAGE 3500
#define TIMEOUT_PROGRAM 10

static struct touch_fw_data {
	u8 block_number;
	u32 start_addr;
	u32 end_addr;
	u32 new_fw_cb;
	int delay_after_upgrade;
	bool is_crc;;
	bool isboot;
	bool is80k;
	int hex_tag;
} g_touch_fw_data;

static struct flash_block_info {
	char *name;
	u32 start;
	u32 end;
	u32 len;
	u32 mem_start;
	u32 fix_mem_start;
	u8 mode;
} g_flash_block_info[FW_BLOCK_INFO_NUM];

static u8 *p_firmware;

static u32 hex_to_dec(const char *phex, s32 len)
{
	u32 ret = 0, temp = 0, i;
	u32 shift = (len - 1) * 4;

	for (i = 0; i < len; shift -= 4, i++) {
		if ((phex[i] >= '0') && (phex[i] <= '9'))
			temp = phex[i] - '0';
		else if ((phex[i] >= 'a') && (phex[i] <= 'f'))
			temp = (phex[i] - 'a') + 10;
		else if ((phex[i] >= 'A') && (phex[i] <= 'F'))
			temp = (phex[i] - 'A') + 10;
		else
			return -1;

		ret |= (temp << shift);
	}
	return ret;
}

static u32 calculate_check(u32 start_addr, u32 len, u8 *p_firmware)
{
	int i = 0, j = 0;
	int crc_poly = 0x04C11DB7;
	u32 tmp_crc = 0xFFFFFFFF;

	for (i = start_addr; i < start_addr + len; i++) {
		tmp_crc ^= (p_firmware[i] << 24);

		for (j = 0; j < 8; j++) {
			if ((tmp_crc & 0x80000000) != 0)
				tmp_crc = tmp_crc << 1 ^ crc_poly;
			else
				tmp_crc = tmp_crc << 1;
		}
	}
	return tmp_crc;
}

static int calc_hw_dma_crc(u32 start_addr, u32 block_size)
{
	int count = 50;
	u32 busy = 0;

	/* dma1 src1 address */
	if (ili_ice_mode_write(0x072104, start_addr, 4) < 0)
		TS_LOG_ERR("Write dma1 src1 address failed\n");
	/* dma1 src1 format */
	if (ili_ice_mode_write(0x072108, 0x80000001, 4) < 0)
		TS_LOG_ERR("Write dma1 src1 format failed\n");
	/* dma1 dest address */
	if (ili_ice_mode_write(0x072114, 0x0002725C, 4) < 0)
		TS_LOG_ERR("Write dma1 src1 format failed\n");
	/* dma1 dest format */
	if (ili_ice_mode_write(0x072118, 0x80000000, 4) < 0)
		TS_LOG_ERR("Write dma1 dest format failed\n");
	/* Block size*/
	if (ili_ice_mode_write(0x07211C, block_size, 4) < 0)
		TS_LOG_ERR("Write block size %d failed\n", block_size);
	/* crc off */
	if (ili_ice_mode_write(0x041016, 0x00, 1) < 0)
		TS_LOG_INFO("Write crc of failed\n");
	/* dma crc */
	if (ili_ice_mode_write(0x041017, 0x03, 1) < 0)
		TS_LOG_ERR("Write dma 1 crc failed\n");
	/* Dma1 stop */
	if (ili_ice_mode_write(0x072100, 0x02000000, 4) < 0)
		TS_LOG_ERR("Write dma1 stop failed\n");
	/* crc on */
	if (ili_ice_mode_write(0x041016, 0x01, 1) < 0)
		TS_LOG_ERR("Write crc on failed\n");

	/* clr int */
	if (ili_ice_mode_write(0x048006, 0x2, 1) < 0)
		TS_LOG_ERR("Write clr int failed\n");
	/* Dma1 start */
	if (ili_ice_mode_write(0x072100, 0x01000000, 4) < 0)
		TS_LOG_ERR("Write dma1 start failed\n");

	/* Polling BIT0 */
	while (count > 0) {
		mdelay(1);
		if (ili_ice_mode_read(0x048006, &busy, sizeof(u8)) < 0)
			TS_LOG_ERR("Read busy error\n");
		TS_LOG_DEBUG("busy = %x\n", busy);
		if ((busy & 0x02) == 2)
			break;
		count--;
	}

	if (count <= 0) {
		TS_LOG_ERR("BIT0 is busy\n");
		return -1;
	}

	if (ili_ice_mode_read(0x04101C, &busy, sizeof(u32)) < 0) {
		TS_LOG_ERR("Read dma crc error\n");
		return -1;
	}
	return busy;
}

static int ilitek_tddi_fw_iram_read(u8 *buf, u32 start, int len)
{
	int limit = SPI_RX_BUF_SIZE;
	int addr = 0, loop = 0, tmp_len = len, cnt = 0;
	u8 cmd[4] = {0};

	if (!buf) {
		TS_LOG_ERR("buf is null\n");
		return -ENOMEM;
	}

	if (len % limit)
		loop = (len / limit) + 1;
	else
		loop = len / limit;

	for (cnt = 0, addr = start; cnt < loop; cnt++, addr += limit) {
		if (tmp_len > limit)
			tmp_len = limit;

		cmd[0] = 0x25;
		cmd[3] = (char)((addr & 0x00FF0000) >> 16);
		cmd[2] = (char)((addr & 0x0000FF00) >> 8);
		cmd[1] = (char)((addr & 0x000000FF));

		if (ilits->wrapper(cmd, 4, NULL, 0, OFF, OFF) < 0) {
			TS_LOG_ERR("Failed to write iram data\n");
			return -ENODEV;
		}

		if (ilits->wrapper(NULL, 0, buf + cnt * limit, tmp_len, OFF, OFF) < 0) {
			TS_LOG_ERR("Failed to Read iram data\n");
			return -ENODEV;
		}

		tmp_len = len - cnt * limit;
		ilits->fw_update_stat = ((len - tmp_len) * 100) / len;
		TS_LOG_INFO("Reading iram data .... %d%c", ilits->fw_update_stat, '%');
	}
	return 0;
}

int ili_fw_dump_iram_data(u32 start, u32 end, bool save)
{
	struct file *f = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	int i, ret = 0;
	int len, tmp = debug_en;
	bool ice = atomic_read(&ilits->ice_stat);

	if (!ice) {
		ret = ili_ice_mode_ctrl(ENABLE, OFF);
		if (ret < 0) {
			TS_LOG_ERR("Enable ice mode failed\n");
			return ret;
		}
	}

	len = end - start + 1;

	if (len > MAX_HEX_FILE_SIZE) {
		TS_LOG_ERR("len is larger than buffer, abort\n");
		ret = -EINVAL;
		goto out;
	}

	for (i = 0; i < MAX_HEX_FILE_SIZE; i++)
		ilits->update_buf[i] = 0xFF;

	ret = ilitek_tddi_fw_iram_read(ilits->update_buf, start, len);
	if (ret < 0) {
		TS_LOG_ERR("Read IRAM data failed\n");
		goto out;
	}

	if (save) {
		f = filp_open(DUMP_IRAM_PATH, O_WRONLY | O_CREAT | O_TRUNC, 644);
		if (err_alloc_mem(f)) {
			TS_LOG_ERR("Failed to open the file at %ld.\n", PTR_ERR(f));
			ret = -ENOMEM;
			goto out;
		}

		old_fs = get_fs();
		set_fs(get_ds());
		set_fs(KERNEL_DS);
		pos = 0;
		vfs_write(f, ilits->update_buf, len, &pos);
		set_fs(old_fs);
		filp_close(f, NULL);
		TS_LOG_INFO("Save iram data to %s\n", DUMP_IRAM_PATH);
	} else {
		debug_en = DEBUG_ALL;
		ili_dump_data(ilits->update_buf, 8, len, 0, "IRAM");
		debug_en = tmp;
	}

out:
	if (!ice) {
		if (ili_ice_mode_ctrl(DISABLE, OFF) < 0)
			TS_LOG_ERR("Enable ice mode failed after code reset\n");
	}

	TS_LOG_INFO("Dump IRAM %s\n", (ret < 0) ? "FAIL" : "SUCCESS");
	return ret;
}

static int ilitek_tddi_fw_iram_program(u32 start, u8 *w_buf, u32 w_len, u32 split_len)
{
	int i, j;
	u32 addr;
	u32 end = start + w_len;
	bool fix_4_alignment = false;

	if (split_len % 4 > 0)
		TS_LOG_ERR("Since split_len must be four-aligned, it must be a multiple of four");

	if (split_len != 0) {
		for (addr = start, i = 0; addr < end; addr += split_len, i += split_len) {
			if ((addr + split_len) > end) {
				split_len = end - addr;
				if (split_len % 4 != 0)
					fix_4_alignment = true;
			}

			ilits->update_buf[0] = SPI_WRITE;
			ilits->update_buf[1] = 0x25;
			ilits->update_buf[2] = (char)((addr & 0x000000FF));
			ilits->update_buf[3] = (char)((addr & 0x0000FF00) >> 8);
			ilits->update_buf[4] = (char)((addr & 0x00FF0000) >> 16);

			for (j = 0; j < split_len; j++)
				ilits->update_buf[5 + j] = w_buf[i + j];

			if (fix_4_alignment) {
				TS_LOG_INFO("org split_len = 0x%X\n", split_len);
				TS_LOG_INFO("idev->update_buf[5 + 0x%X] = 0x%X\n", split_len - 4, ilits->update_buf[5 + split_len - 4]);
				TS_LOG_INFO("idev->update_buf[5 + 0x%X] = 0x%X\n", split_len - 3, ilits->update_buf[5 + split_len - 3]);
				TS_LOG_INFO("idev->update_buf[5 + 0x%X] = 0x%X\n", split_len - 2, ilits->update_buf[5 + split_len - 2]);
				TS_LOG_INFO("idev->update_buf[5 + 0x%X] = 0x%X\n", split_len - 1, ilits->update_buf[5 + split_len - 1]);
				for (j = 0; j < (4 - (split_len % 4)); j++) {
					ilits->update_buf[5 + j + split_len] = 0xFF;
					TS_LOG_INFO("idev->update_buf[5 + 0x%X] = 0x%X\n",j + split_len, ilits->update_buf[5 + j + split_len]);
				}

				TS_LOG_INFO("split_len %% 4 = %d\n", split_len % 4);
				split_len = split_len + (4 - (split_len % 4));
				TS_LOG_INFO("fix split_len = 0x%X\n", split_len);
			}
			if (ilits->spi_write_then_read(ilits->spi, ilits->update_buf, split_len + 5, NULL, 0)) {
				TS_LOG_ERR("Failed to write data via SPI in host download (%x)\n", split_len + 5);
				return -EIO;
			}
			if (w_len)
				ilits->fw_update_stat = (i * 100) / w_len;
		}
	} else {
		for (i = 0; i < MAX_HEX_FILE_SIZE; i++)
			ilits->update_buf[i] = 0xFF;

		ilits->update_buf[0] = SPI_WRITE;
		ilits->update_buf[1] = 0x25;
		ilits->update_buf[2] = (char)((start & 0x000000FF));
		ilits->update_buf[3] = (char)((start & 0x0000FF00) >> 8);
		ilits->update_buf[4] = (char)((start & 0x00FF0000) >> 16);

		memcpy(&ilits->update_buf[5], w_buf, w_len);
		if (w_len % 4 != 0) {
			TS_LOG_INFO("org w_len = %d\n", w_len);
			w_len = w_len + (4 - (w_len % 4));
			TS_LOG_INFO("w_len = %d w_len %% 4 = %d\n", w_len, w_len % 4);
		}
		/* It must be supported by platforms that have the ability to transfer all data at once. */
		if (ilits->spi_write_then_read(ilits->spi, ilits->update_buf, w_len + 5, NULL, 0) < 0) {
			TS_LOG_ERR("Failed to write data via SPI in host download (%x)\n", w_len + 5);
			return -EIO;
		}
	}
	return 0;
}

static int ilitek_tddi_fw_iram_upgrade(u8 *p_firmware)
{
	int i, ret = UPDATE_PASS;
	u32 mode, crc, dma, iram_crc = 0;
	u8 *fw_ptr = NULL, crc_temp[4], crc_len = 4;
	bool iram_crc_err = false;

	memset(crc_temp, 0, sizeof(crc_temp));
	if (!ilits->ddi_rest_done) {
		if (ilits->actual_tp_mode != P5_X_FW_GESTURE_MODE)
			ili_reset_ctrl(ilits->reset);

		ret = ili_ice_mode_ctrl(ENABLE, OFF);
		if (ret < 0)
			return -EFW_ICE_MODE;
	} else {
		/* Restore it if the wq of load_fw_ddi has been called. */
		ilits->ddi_rest_done = false;
	}

	if (ilits->chip->dma_reset) {
		TS_LOG_INFO("operate dma reset in reg after tp reset\n");
		if (ili_ice_mode_write(0x40040, 0x00800000, 4) < 0)
			TS_LOG_INFO("Failed to open DMA reset\n");
		if (ili_ice_mode_write(0x40040, 0x00000000, 4) < 0)
			TS_LOG_INFO("Failed to close DMA reset\n");
	}

	/* Point to p_firmware with different addresses for getting its block data. */
	fw_ptr = p_firmware;
	if (ilits->actual_tp_mode == P5_X_FW_TEST_MODE) {

		mode = MP;
	} else if (ilits->actual_tp_mode == P5_X_FW_GESTURE_MODE) {
		mode = GESTURE;
		crc_len = 0;
	} else {
		mode = AP;
	}

	/* Program data to iram acorrding to each block */
	for (i = 0; i < FW_BLOCK_INFO_NUM; i++) {
		if (g_flash_block_info[i].mode == mode && g_flash_block_info[i].len != 0) {
			TS_LOG_DEBUG("Download %s code from hex 0x%x to IRAM 0x%x, len = 0x%x\n",
					g_flash_block_info[i].name, g_flash_block_info[i].start, g_flash_block_info[i].mem_start, g_flash_block_info[i].len);

#if SPI_DMA_TRANSFER_SPLIT
			if (ilitek_tddi_fw_iram_program(g_flash_block_info[i].mem_start, (fw_ptr + g_flash_block_info[i].start), g_flash_block_info[i].len, SPI_UPGRADE_LEN) < 0)
				TS_LOG_ERR("IRAM program failed\n");
#else
			if (ilitek_tddi_fw_iram_program(g_flash_block_info[i].mem_start, (fw_ptr + g_flash_block_info[i].start), g_flash_block_info[i].len, 0) < 0)
				TS_LOG_ERR("IRAM program failed\n");
#endif
			crc = calculate_check(g_flash_block_info[i].start,
				g_flash_block_info[i].len - crc_len, fw_ptr);
			dma = calc_hw_dma_crc(g_flash_block_info[i].mem_start, g_flash_block_info[i].len - crc_len);

			if (mode != GESTURE) {
				ilitek_tddi_fw_iram_read(crc_temp, (g_flash_block_info[i].mem_start + g_flash_block_info[i].len - crc_len), sizeof(crc_temp));
				iram_crc = crc_temp[0] << 24 | crc_temp[1] << 16 |crc_temp[2] << 8 | crc_temp[3];
				if (iram_crc != dma)
					iram_crc_err = true;
			}

			TS_LOG_INFO("%s CRC is %s hex(%x) : dma(%x) : iram(%x), calculation len is 0x%x\n",
				g_flash_block_info[i].name,((crc != dma)||(iram_crc_err)) ? "Invalid !" : "Correct !", crc, dma, iram_crc, g_flash_block_info[i].len - crc_len);

			if ((crc != dma)|| iram_crc_err) {
				TS_LOG_ERR("CRC Error! print iram data with first 16 bytes\n");
				ili_fw_dump_iram_data(0x0, 0xF, false);
				return -EFW_CRC;
			}
		}
	}

	if (ilits->actual_tp_mode != P5_X_FW_GESTURE_MODE) {
		if (ili_reset_ctrl(TP_IC_CODE_RST) < 0) {
			TS_LOG_ERR("TP Code reset failed during iram programming\n");
			ret = -EFW_REST;
			return ret;
		}
	}

	if (ili_ice_mode_ctrl(DISABLE, OFF) < 0) {
		TS_LOG_ERR("Disable ice mode failed after code reset\n");
		ret = -EFW_ICE_MODE;
	}

	/* Waiting for fw ready sending first cmd */
	if (!ilits->info_from_hex || (ilits->chip->core_ver < CORE_VER_1410))
		mdelay(100);
	else
		mdelay(50);

	return ret;
}

static int ilitek_fw_calc_file_crc(u8 *p_firmware)
{
	int i;
	u32 ex_addr, data_crc, file_crc;

	for (i = 0; i < ARRAY_SIZE(g_flash_block_info); i++) {
		if (g_flash_block_info[i].end == 0)
			continue;
		ex_addr = g_flash_block_info[i].end;
		data_crc = calculate_check(g_flash_block_info[i].start,
			g_flash_block_info[i].len - 4, p_firmware);
		file_crc = p_firmware[ex_addr - 3] << 24 | p_firmware[ex_addr - 2] << 16 | p_firmware[ex_addr - 1] << 8 | p_firmware[ex_addr];
		TS_LOG_DEBUG("data crc = %x, file crc = %x\n", data_crc, file_crc);
		if (data_crc != file_crc) {
			TS_LOG_ERR("Content of fw file is broken. %d, %x, %x\n",
				i, data_crc, file_crc);
			return -1;
		}
	}

	TS_LOG_INFO("Content of fw file is correct\n");
	return 0;
}

static int ilitek_tddi_fw_update_block_info(u8 *p_firmware)
{
	u32 ges_area_section = 0, ges_info_addr = 0, ges_fw_start = 0, ges_fw_end = 0;
	u32 ap_end = 0, ap_len = 0;
	u32 fw_info_addr = 0, fw_mp_ver_addr = 0;

	if (g_touch_fw_data.hex_tag != BLOCK_TAG_AF) {
		TS_LOG_ERR("HEX TAG is invalid (0x%X)\n", g_touch_fw_data.hex_tag);
		return -EINVAL;
	}

	g_flash_block_info[AP].mem_start = (g_flash_block_info[AP].fix_mem_start != INT_MAX) ? g_flash_block_info[AP].fix_mem_start : 0;
	g_flash_block_info[DATA].mem_start = (g_flash_block_info[DATA].fix_mem_start != INT_MAX) ? g_flash_block_info[DATA].fix_mem_start : DLM_START_ADDRESS;
	g_flash_block_info[TUNING].mem_start = (g_flash_block_info[TUNING].fix_mem_start != INT_MAX) ? g_flash_block_info[TUNING].fix_mem_start :  g_flash_block_info[DATA].mem_start + g_flash_block_info[DATA].len;
	g_flash_block_info[MP].mem_start = (g_flash_block_info[MP].fix_mem_start != INT_MAX) ? g_flash_block_info[MP].fix_mem_start : 0;
	g_flash_block_info[GESTURE].mem_start = (g_flash_block_info[GESTURE].fix_mem_start != INT_MAX) ? g_flash_block_info[GESTURE].fix_mem_start :	0;
	g_flash_block_info[TAG].mem_start = (g_flash_block_info[TAG].fix_mem_start != INT_MAX) ? g_flash_block_info[TAG].fix_mem_start : 0;
	g_flash_block_info[PARA_BACKUP].mem_start = (g_flash_block_info[PARA_BACKUP].fix_mem_start != INT_MAX) ? g_flash_block_info[PARA_BACKUP].fix_mem_start : 0;
	g_flash_block_info[DDI].mem_start = (g_flash_block_info[DDI].fix_mem_start != INT_MAX) ? g_flash_block_info[DDI].fix_mem_start : 0;

	/* Parsing gesture info form AP code */
	ges_info_addr = (g_flash_block_info[AP].end + 1 - 60);
	ges_area_section = (p_firmware[ges_info_addr + 3] << 24) + (p_firmware[ges_info_addr + 2] << 16) + (p_firmware[ges_info_addr + 1] << 8) + p_firmware[ges_info_addr];
	g_flash_block_info[GESTURE].mem_start = (p_firmware[ges_info_addr + 7] << 24) + (p_firmware[ges_info_addr + 6] << 16) + (p_firmware[ges_info_addr + 5] << 8) + p_firmware[ges_info_addr + 4];
	ap_end = (p_firmware[ges_info_addr + 11] << 24) + (p_firmware[ges_info_addr + 10] << 16) + (p_firmware[ges_info_addr + 9] << 8) + p_firmware[ges_info_addr + 8];

	if (ap_end != g_flash_block_info[GESTURE].mem_start)
		ap_len = ap_end - g_flash_block_info[GESTURE].mem_start + 1;

	ges_fw_start = (p_firmware[ges_info_addr + 15] << 24) + (p_firmware[ges_info_addr + 14] << 16) + (p_firmware[ges_info_addr + 13] << 8) + p_firmware[ges_info_addr + 12];
	ges_fw_end = (p_firmware[ges_info_addr + 19] << 24) + (p_firmware[ges_info_addr + 18] << 16) + (p_firmware[ges_info_addr + 17] << 8) + p_firmware[ges_info_addr + 16];

	if (ges_fw_end != ges_fw_start)
		g_flash_block_info[GESTURE].len = ges_fw_end - ges_fw_start;

	/* update gesture address */
	g_flash_block_info[GESTURE].start = ges_fw_start;

	TS_LOG_INFO("==== Gesture loader info ====\n");
	TS_LOG_INFO("gesture move to ap addr => start = 0x%x, ap_end = 0x%x, ap_len = 0x%x\n", g_flash_block_info[GESTURE].mem_start, ap_end, ap_len);
	TS_LOG_INFO("gesture hex addr => start = 0x%x, gesture_end = 0x%x, gesture_len = 0x%x\n", ges_fw_start, ges_fw_end, g_flash_block_info[GESTURE].len);
	TS_LOG_INFO("=============================\n");

		g_flash_block_info[AP].name = "AP";
		g_flash_block_info[DATA].name = "DATA";
		g_flash_block_info[TUNING].name = "TUNING";
		g_flash_block_info[MP].name = "MP";
		g_flash_block_info[GESTURE].name = "GESTURE";
		g_flash_block_info[TAG].name = "TAG";
		g_flash_block_info[PARA_BACKUP].name = "PARA_BACKUP";
		g_flash_block_info[DDI].name = "DDI";

		/* upgrade mode define */
		g_flash_block_info[DATA].mode = g_flash_block_info[AP].mode = g_flash_block_info[TUNING].mode = AP;
		g_flash_block_info[MP].mode = MP;
		g_flash_block_info[GESTURE].mode = GESTURE;

	if (g_flash_block_info[AP].end > (64*K))
		g_touch_fw_data.is80k = true;

	/* Copy fw info  */
	fw_info_addr = g_flash_block_info[AP].end - INFO_HEX_ST_ADDR;
	TS_LOG_INFO("Parsing hex info start addr = 0x%x\n", fw_info_addr);
	memcpy(ilits->fw_info, (p_firmware + fw_info_addr), sizeof(ilits->fw_info));

	/* copy fw mp ver */
	fw_mp_ver_addr = g_flash_block_info[MP].end - INFO_MP_HEX_ADDR;
	TS_LOG_INFO("Parsing hex mp ver addr = 0x%x\n", fw_mp_ver_addr);
	memcpy(ilits->fw_mp_ver, p_firmware + fw_mp_ver_addr, sizeof(ilits->fw_mp_ver));

	/* copy fw core ver */
	ilits->chip->core_ver = (ilits->fw_info[68] << 24) | (ilits->fw_info[69] << 16) |
			(ilits->fw_info[70] << 8) | ilits->fw_info[71];
	TS_LOG_INFO("New FW Core version = %x\n", ilits->chip->core_ver);

	/* Get hex fw vers */
	g_touch_fw_data.new_fw_cb = (ilits->fw_info[48] << 24) | (ilits->fw_info[49] << 16) |
			(ilits->fw_info[50] << 8) | ilits->fw_info[51];

	/* Calculate update address */
	TS_LOG_INFO("New FW ver = 0x%x\n", g_touch_fw_data.new_fw_cb);
	TS_LOG_INFO("star_addr = 0x%06X, end_addr = 0x%06X, Block Num = %d\n", g_touch_fw_data.start_addr, g_touch_fw_data.end_addr, g_touch_fw_data.block_number);

	return 0;
}



static int ilitek_tddi_fw_hex_convert(u8 *phex, int size, u8 *p_firmware)
{
	int block = 0;
	u32 i = 0, j = 0, k = 0, num = 0;
	u32 len = 0, addr = 0, type = 0;
	u32 start_addr = 0x0, end_addr = 0x0, ex_addr = 0;
	u32 offset;

	memset(g_flash_block_info, 0x0, sizeof(g_flash_block_info));

	/* Parsing HEX file */
	for (; i < size;) {
		len = hex_to_dec((const char *)&phex[i + 1], 2);
		addr = hex_to_dec((const char *)&phex[i + 3], 4);
		type = hex_to_dec((const char *)&phex[i + 7], 2);

		if (type == 0x04) {
			ex_addr = hex_to_dec((const char *)&phex[i + 9], 4);
		} else if (type == 0x02) {
			ex_addr = hex_to_dec((const char *)&phex[i + 9], 4);
			ex_addr = ex_addr >> 12;
		} else if (type == BLOCK_TAG_AF) {
			/* insert block info extracted from hex */
			g_touch_fw_data.hex_tag = type;
			if (g_touch_fw_data.hex_tag == BLOCK_TAG_AF)
				num = hex_to_dec((const char *)&phex[i + 9 + 6 + 6], 2);
			else
				num = 0xFF;

			if (num > (FW_BLOCK_INFO_NUM - 1)) {
				TS_LOG_ERR("ERROR! block num is larger than its define %d, %d  \n",
						num, FW_BLOCK_INFO_NUM - 1);
				return -EINVAL;
			}

			g_flash_block_info[num].start =
				hex_to_dec((const char *)&phex[i + 9], 6);
			g_flash_block_info[num].end =
				hex_to_dec((const char *)&phex[i + 9 + 6], 6);
			g_flash_block_info[num].fix_mem_start = INT_MAX;
			g_flash_block_info[num].len = g_flash_block_info[num].end - g_flash_block_info[num].start + 1;
			TS_LOG_DEBUG("Block[%d]: start_addr = %x, end = %x", num, g_flash_block_info[num].start, g_flash_block_info[num].end);

			if (num == GESTURE)
				ilits->gesture_load_code = true;

			block++;
		} else if (type == BLOCK_TAG_B0 && g_touch_fw_data.hex_tag == BLOCK_TAG_AF) {
			num = hex_to_dec((const char *)&phex[i + 9 + 6], 2);

			if (num > (FW_BLOCK_INFO_NUM - 1)) {
				TS_LOG_ERR("ERROR! block num is larger than its define %d, %d  \n",
						num, FW_BLOCK_INFO_NUM - 1);
				return -EINVAL;
			}

			g_flash_block_info[num].fix_mem_start =
				hex_to_dec((const char *)&phex[i + 9], 6);
			TS_LOG_DEBUG("Tag 0xB0: change Block[%d] to addr = 0x%x\n", num, g_flash_block_info[num].fix_mem_start);
		}

		addr = addr + (ex_addr << 16);

		if (phex[i + 1 + 2 + 4 + 2 + (len * 2) + 2] == 0x0D)
			offset = 2;
		else
			offset = 1;

		if (addr > MAX_HEX_FILE_SIZE) {
			TS_LOG_ERR("Invalid hex format %d\n", addr);
			return -1;
		}

		if (type == 0x00) {
			end_addr = addr + len;
			if (addr < start_addr)
				start_addr = addr;
			/* fill data */
			for (j = 0, k = 0; j < (len * 2); j += 2, k++)
				p_firmware[addr + k] =
					hex_to_dec((const char *)&phex[i + 9 + j], 2);
		}
		i += 1 + 2 + 4 + 2 + (len * 2) + 2 + offset;
	}

	if (ilitek_fw_calc_file_crc(p_firmware) < 0)
		return -1;

	g_touch_fw_data.start_addr = start_addr;
	g_touch_fw_data.end_addr = end_addr;
	g_touch_fw_data.block_number = block;
	return 0;
}

static int ilitek_tdd_fw_hex_open(u8 op, u8 *p_firmware)
{
	int ret =0, fsize = 0;
	const struct firmware *fw = NULL;
	struct file *f = NULL;
	mm_segment_t old_fs;
	loff_t pos = 0;
	TS_LOG_INFO("Open file method = %s, path = %s\n",
		op ? "FILP_OPEN" : "REQUEST_FIRMWARE",
		op ? ilits->md_fw_filp_path : ilits->fw_name);

	switch (op) {
	case REQUEST_FIRMWARE:
		if (request_firmware(&fw, ilits->fw_name, ilits->dev) < 0) {
			TS_LOG_ERR("Request firmware failed, try again\n");
			if (request_firmware(&fw, ilits->md_fw_rq_path, ilits->dev) < 0) {
				TS_LOG_ERR("Request firmware failed after retry\n");
				ret = -1;
				goto out;
			}
		}

		fsize = fw->size;
		TS_LOG_INFO("fsize = %d\n", fsize);
		if (fsize <= 0) {
			TS_LOG_ERR("The size of file is invaild\n");
			release_firmware(fw);
			ret = -1;
			goto out;
		}

		ilits->tp_fw.size = 0;
		ilits->tp_fw.data = vmalloc(fsize);
		if (err_alloc_mem(ilits->tp_fw.data)) {
			TS_LOG_ERR("Failed to allocate tp_fw by vmalloc, try again\n");
			ilits->tp_fw.data = vmalloc(fsize);
			if (err_alloc_mem(ilits->tp_fw.data)) {
				TS_LOG_ERR("Failed to allocate tp_fw after retry\n");
				release_firmware(fw);
				ret = -ENOMEM;
				goto out;
			}
		}

		/* Copy fw data got from request_firmware to global */
		if (fsize * sizeof(*fw->data) > fsize)
			memcpy((u8 *)ilits->tp_fw.data, fw->data, fsize);
		else
			memcpy((u8 *)ilits->tp_fw.data, fw->data, fsize * sizeof(*fw->data));
		ilits->tp_fw.size = fsize;
		release_firmware(fw);
		break;
	case FILP_OPEN:
		f = filp_open(ilits->md_fw_filp_path, O_RDONLY, 0644);
		if (err_alloc_mem(f)) {
			TS_LOG_ERR("Failed to open the file, %ld\n", PTR_ERR(f));
			ret = -1;
			goto out;
		}

		fsize = f->f_inode->i_size;
		TS_LOG_INFO("fsize = %d\n", fsize);
		if (fsize <= 0) {
			TS_LOG_ERR("The size of file is invaild\n");
			filp_close(f, NULL);
			ret = -1;
			goto out;
		}

		ilits->tp_fw.size = 0;
		ilits->tp_fw.data = vmalloc(fsize);
		if (err_alloc_mem(ilits->tp_fw.data)) {
			TS_LOG_ERR("Failed to allocate tp_fw by vmalloc, try again\n");
			ilits->tp_fw.data = vmalloc(fsize);
			if (err_alloc_mem(ilits->tp_fw.data)) {
				TS_LOG_ERR("Failed to allocate tp_fw after retry\n");
				filp_close(f, NULL);
				ret = -ENOMEM;
				goto out;
			}
		}

		/* ready to map user's memory to obtain data by reading files */
		old_fs = get_fs();
		set_fs(get_ds());
		set_fs(KERNEL_DS);
		pos = 0;
		vfs_read(f, (u8 *)ilits->tp_fw.data, fsize, &pos);
		set_fs(old_fs);
		filp_close(f, NULL);
		ilits->tp_fw.size = fsize;
		break;
	default:
		TS_LOG_ERR("Unknown open file method, %d\n", op);
		break;
	}

	if (err_alloc_mem(ilits->tp_fw.data) || ilits->tp_fw.size <= 0) {
		TS_LOG_ERR("fw data/size is invaild\n");
		ret = -1;
		goto out;
	}

	/* Convert hex and copy data from tp_fw.data to p_firmware */
	if (ilitek_tddi_fw_hex_convert((u8 *)ilits->tp_fw.data, ilits->tp_fw.size, p_firmware) < 0) {
		TS_LOG_ERR("Convert hex file failed\n");
		ret = -1;
	}

out:
	ipio_vfree((void **)&(ilits->tp_fw.data));
	return ret;
}

int ili_fw_upgrade(int op)
{
	int i, ret = 0, retry = 3;

	if (!ilits->boot || ilits->force_fw_update || err_alloc_mem(p_firmware)) {
		ilits->gesture_load_code = false;

		if (err_alloc_mem(p_firmware)) {
			ipio_vfree((void **)&p_firmware);
			p_firmware = vmalloc(MAX_HEX_FILE_SIZE * sizeof(u8));
			if (err_alloc_mem(p_firmware)) {
				TS_LOG_ERR("Failed to allocate p_firmware memory, %ld\n", PTR_ERR(p_firmware));
				ipio_vfree((void **)&p_firmware);
				ret = -ENOMEM;
				goto out;
			}
		}

		for (i = 0; i < MAX_HEX_FILE_SIZE; i++)
			p_firmware[i] = 0xFF;

		if (ilitek_tdd_fw_hex_open(op, p_firmware) < 0) {
			TS_LOG_ERR("Open hex file fail\n");
			goto out;
		}

		if (ilitek_tddi_fw_update_block_info(p_firmware) < 0) {
			ret = -EFW_CONVERT_FILE;
			goto out;
		}
	}

	do {
		ret = ilitek_tddi_fw_iram_upgrade(p_firmware);
		if (ret == UPDATE_PASS)
			break;

		TS_LOG_ERR("Upgrade failed, do retry!\n");
	} while (--retry > 0);

	if (ret != UPDATE_PASS) {
		TS_LOG_ERR("Failed to upgrade fw %d times, erasing iram\n", retry);
		if (ili_reset_ctrl(ilits->reset) < 0)
				TS_LOG_ERR("TP reset failed while erasing data\n");
		ilits->xch_num = 0;
		ilits->ych_num = 0;
		return ret;
	}

out:
	ili_ic_get_core_ver();
	ili_ic_get_protocl_ver();
	ili_ic_get_fw_ver();
	ili_ic_get_tp_info();
	ili_ic_get_panel_info();
	ret = ili_ic_func_ctrl("roi_set",ilits->roi_switch_backup);
	if (ret)
		TS_LOG_INFO("roi control failed, enable = %d, ret = %d\n",
			ilits->roi_switch_backup, ret);
	return ret;
}

void ili_fw_read_flash_info(void)
{
		return;
}

void ili_flash_clear_dma(void)
{
		return;
}

void ili_flash_dma_write(u32 start, u32 end, u32 len)
{
		return;
}

int ili_fw_dump_flash_data(u32 start, u32 end, bool user)
{
		return 0;
}
