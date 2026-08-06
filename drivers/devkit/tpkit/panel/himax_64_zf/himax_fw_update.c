/* Himax Android Driver Sample Code for Himax chipset
 *
 * Copyright (C) 2021 Himax Corporation.
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

#include "himax_ic.h"
#ifdef CONFIG_APP_INFO
#include <misc/app_info.h>
#endif

unsigned int HX_ZF_UPDATE_FLAG;
unsigned int HX_ZF_RESET_COUNT;
unsigned long ZF_FW_VER_MAJ_FLASH_ADDR;
unsigned long ZF_FW_VER_MIN_FLASH_ADDR;
unsigned long ZF_CFG_VER_MAJ_FLASH_ADDR;
unsigned long ZF_CFG_VER_MIN_FLASH_ADDR;
unsigned long ZF_CID_VER_MAJ_FLASH_ADDR;
unsigned long ZF_CID_VER_MIN_FLASH_ADDR;
uint32_t NC_CFG_TABLE_FLASH_ADDR;

unsigned long ZF_FW_VER_MAJ_FLASH_LENG;
unsigned long ZF_FW_VER_MIN_FLASH_LENG;
unsigned long ZF_CFG_VER_MAJ_FLASH_LENG;
unsigned long ZF_CFG_VER_MIN_FLASH_LENG;
unsigned long ZF_CID_VER_MAJ_FLASH_LENG;
unsigned long ZF_CID_VER_MIN_FLASH_LENG;

extern char himax_zf_project_id[];
extern struct himax_ts_data *g_himax_zf_ts_data;


#define BOOT_UPDATE_FIRMWARE_FLAG_FILENAME	"/system/etc/tp_test_parameters/boot_update_firmware.flag"
#define BOOT_UPDATE_FIRMWARE_FLAG "boot_update_firmware_flag:"

int HX_ZF_RX_NUM;
int HX_ZF_TX_NUM;
int HX_ZF_BT_NUM;
int HX_ZF_X_RES;
int HX_ZF_Y_RES;
int HX_ZF_MAX_PT;
unsigned char IC_ZF_CHECKSUM;
unsigned char IC_ZF_TYPE;
bool HX_ZF_XY_REVERSE = false;

char himax_firmware_name[64] = {0};

static int fw_update_boot_sd_flag;
uint32_t dbg_reg_ary[4] = {
	FW_ADDR_FW_DBG_MSG_ADDR, FW_ADDR_CHK_FW_STATUS,
	FW_ADDR_CHK_DD_STATUS, FW_ADDR_FLAG_RESET_EVENT
};

void himax_zf_read_fw_status(void)
{
	uint8_t len = 0;
	uint8_t i = 0;
	uint8_t addr[4] = {0};
	uint8_t data[4] = {0};

	len = (uint8_t)(sizeof(dbg_reg_ary) / sizeof(uint32_t));

	for (i = 0; i < len; i++) {
		hx_reg_assign(dbg_reg_ary[i], addr);
		hx_zf_register_read(addr, FOUR_BYTE_CMD, data);

		TS_LOG_INFO("reg[0-3] : 0x%08X = 0x%02X, 0x%02X, 0x%02X, 0x%02X\n",
		dbg_reg_ary[i], data[0], data[1], data[2], data[3]);
	}
}

void hx_zf_power_on_init(void)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t retry = 20;
	int reload_status = 0;

	// After read finish, set initial register from driver
	hx_addr_reg_assign(ADDR_DIAG_REG_SET, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	hx_addr_reg_assign(ADDR_SORTING_MODE_SWITCH, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	hx_addr_reg_assign(ADDR_NFRAME_SEL, TMP_DATA, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);
	hx_addr_reg_assign(ADDR_SWITCH_FLASH_RLD, DATA_INIT, &tmp_addr[0], &tmp_data[0]);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);

	hx_zf_sense_on(SENSE_ON_0);

	TS_LOG_INFO("%s: waiting for FW reload data\n", __func__);

	while (reload_status == 0) {
		hx_reg_assign(ADDR_SWITCH_FLASH_RLD, tmp_addr);
		hx_zf_register_read(tmp_addr, FOUR_BYTE_CMD, tmp_data);

		if ((tmp_data[3] == 0x00) && (tmp_data[2] == 0x00) &&
			(tmp_data[1] == 0x72) && (tmp_data[0] == 0xC0)) {
			TS_LOG_INFO("reload OK!\n");
			reload_status = 1;
			break;
		} else if (retry == 0) {
			TS_LOG_INFO("reload 20 times! fail\n");
			break;
		} else {
			retry--;
			himax_zf_read_fw_status();
			usleep_range(10000, 11000);
			TS_LOG_INFO("reload fail ,delay 10ms retry=%d\n", retry);
		}
	}
	TS_LOG_INFO("%s : data[0]=0x%2.2X,data[1]=0x%2.2X,data[2]=0x%2.2X,data[3]=0x%2.2X\n",
		__func__, tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
	TS_LOG_INFO("reload_status=%d\n", reload_status);
}

/* Zero_flash_update start*/
static void hx_write_sram_0f(uint8_t *addr, const uint8_t *data, uint32_t len)
{
	int total_read_times = 0;
	int max_bus_size = HX_RECEIVE_BUF_MAX_SIZE;
	int total_size_temp = 0;
	int address = 0;
	int i = 0;

	uint8_t tmp_addr[4];
	uint8_t *tmp_data;

	total_size_temp = len;
	TS_LOG_INFO("%s, Entering - total write size=%d\n", __func__, total_size_temp);

	max_bus_size = (len > HX_MAX_WRITE_SZ)
			? HX_MAX_WRITE_SZ : len;

	hx_zf_burst_enable(1);

	tmp_addr[3] = addr[3];
	tmp_addr[2] = addr[2];
	tmp_addr[1] = addr[1];
	tmp_addr[0] = addr[0];
	TS_LOG_INFO("%s, write addr = 0x%02X%02X%02X%02X\n", __func__,
		tmp_addr[3], tmp_addr[2], tmp_addr[1], tmp_addr[0]);

	if (total_size_temp % max_bus_size == 0)
		total_read_times = total_size_temp / max_bus_size;
	else
		total_read_times = total_size_temp / max_bus_size + 1;

	if (len > FOUR_BYTE_CMD)
		hx_zf_burst_enable(1);
	else
		hx_zf_burst_enable(0);

	for (i = 0; i < (total_read_times); i++) {

		if (total_size_temp >= max_bus_size) {

			tmp_data = (uint8_t *)(data + i * max_bus_size);
			himax_zf_register_write(tmp_addr, max_bus_size, tmp_data);
			total_size_temp = total_size_temp - max_bus_size;
		} else {
			tmp_data = (uint8_t *)(data + i * max_bus_size);
			TS_LOG_INFO("last total_size_temp=%d\n",
				total_size_temp % max_bus_size);
			himax_zf_register_write(tmp_addr, total_size_temp % max_bus_size, tmp_data);
		}

		address = ((i+1) * max_bus_size);
		tmp_addr[0] = addr[0] + (uint8_t) ((address) & 0x00FF);

		if (tmp_addr[0] <  addr[0])
			tmp_addr[1] = addr[1]
				+ (uint8_t) ((address>>8) & 0x00FF) + 1;
		else
			tmp_addr[1] = addr[1]
				+ (uint8_t) ((address>>8) & 0x00FF);
		udelay(100);
	}
	TS_LOG_INFO("%s, End\n", __func__);
}

static int himax_mcu_Calculate_CRC_with_AP(unsigned char *FW_content,
		int CRC_from_FW, int len)
{
	int i, j, length = 0;
	int fw_data;
	int fw_data_2;
	int CRC = 0xFFFFFFFF;
	int PolyNomial = 0x82F63B78;

	length = len / 4;

	for (i = 0; i < length; i++) {
		fw_data = FW_content[i * 4];

		for (j = 1; j < 4; j++) {
			fw_data_2 = FW_content[i * 4 + j];
			fw_data += (fw_data_2) << (8 * j);
		}
		CRC = fw_data ^ CRC;
		for (j = 0; j < 32; j++) {
			if ((CRC % 2) != 0)
				CRC = ((CRC >> 1) & 0x7FFFFFFF) ^ PolyNomial;
			else
				CRC = (((CRC >> 1) & 0x7FFFFFFF));
		}
	}
	return CRC;
}

static int hx_sram_write_crc_check(uint8_t *addr, const uint8_t *data, uint32_t len)
{
	int retry = 0;
	int crc = -1;

	do {
		hx_write_sram_0f(addr, data, len);
		crc = hx_zf_hw_check_crc(addr, len);
		retry++;
		TS_LOG_INFO("%s, HW CRC %s in %d time\n", __func__,
			(crc == 0) ? "OK" : "Fail", retry);
	} while (crc != 0 && retry < 3);

	return crc;
}

static bool hx_bin_desc_data_get(uint32_t addr, uint8_t *flash_buf)
{
	uint8_t data_sz = 0x10;
	uint32_t i = 0, j = 0;
	uint16_t chk_end = 0;
	uint16_t chk_sum = 0;
	uint32_t map_code = 0;
	unsigned long flash_addr = 0;

	for (i = 0; i < FW_PAGE_SZ; i = i + data_sz) {
		for (j = i; j < (i + data_sz); j++) {
			chk_end |= flash_buf[j];
			chk_sum += flash_buf[j];
		}
		if (!chk_end) { /*1. Check all zero*/
			TS_LOG_INFO("%s: End in %X\n",	__func__, i + addr);
			return false;
		} else if (chk_sum % 0x100) { /*2. Check sum*/
			TS_LOG_INFO("%s: chk sum failed in %X\n",	__func__, i + addr);
		} else { /*3. get data*/
			map_code = flash_buf[i] + (flash_buf[i + 1] << 8)
			+ (flash_buf[i + 2] << 16) + (flash_buf[i + 3] << 24);
			flash_addr = flash_buf[i + 4] + (flash_buf[i + 5] << 8)
			+ (flash_buf[i + 6] << 16) + (flash_buf[i + 7] << 24);
			switch (map_code) {
			case FW_CID:
				ZF_CID_VER_MAJ_FLASH_ADDR = flash_addr;
				ZF_CID_VER_MIN_FLASH_ADDR = flash_addr + 1;
				TS_LOG_INFO("%s: CID_VER in %lX\n", __func__,
				ZF_CID_VER_MAJ_FLASH_ADDR);
				break;
			case FW_VER:
				ZF_FW_VER_MAJ_FLASH_ADDR = flash_addr;
				ZF_FW_VER_MIN_FLASH_ADDR = flash_addr + 1;
				TS_LOG_INFO("%s: FW_VER in %lX\n", __func__,
				ZF_FW_VER_MAJ_FLASH_ADDR);
				break;
			case CFG_VER:
				ZF_CFG_VER_MAJ_FLASH_ADDR = flash_addr;
				ZF_CFG_VER_MIN_FLASH_ADDR = flash_addr + 1;
				TS_LOG_INFO("%s: CFG_VER in = %08lX\n", __func__,
				ZF_CFG_VER_MAJ_FLASH_ADDR);
				break;
			case TP_CONFIG_TABLE:
				NC_CFG_TABLE_FLASH_ADDR = flash_addr;
				TS_LOG_INFO("%s: CONFIG_TABLE in %X\n",
					__func__, NC_CFG_TABLE_FLASH_ADDR);
				break;
			}
		}
		chk_end = 0;
		chk_sum = 0;
	}
	return true;
}

static bool hx_mcu_bin_desc_get(unsigned char *fw, uint32_t max_sz)
{
	uint32_t addr_t = 0;
	unsigned char *fw_buf = NULL;
	bool keep_on_flag = false;
	bool g_bin_desc_flag = false;

	do {
		fw_buf = &fw[addr_t];
		/*Check bin is with description table or not*/
		if (!g_bin_desc_flag) {
			if (fw_buf[0x00] == 0x00 && fw_buf[0x01] == 0x00
			&& fw_buf[0x02] == 0x00 && fw_buf[0x03] == 0x00
			&& fw_buf[0x04] == 0x00 && fw_buf[0x05] == 0x00
			&& fw_buf[0x06] == 0x00 && fw_buf[0x07] == 0x00
			&& fw_buf[0x0E] == 0x87)
				g_bin_desc_flag = true;
		}
		if (!g_bin_desc_flag) {
			TS_LOG_INFO("%s: fw_buf[0x00] = %2X, fw_buf[0x0E] = %2X\n",
			__func__, fw_buf[0x00], fw_buf[0x0E]);
			TS_LOG_INFO("%s: No description table\n", __func__);
			break;
		}
		/*Get related data*/
		keep_on_flag = hx_bin_desc_data_get(addr_t, fw_buf);

		addr_t = addr_t + FW_PAGE_SZ;
	} while (max_sz > addr_t && keep_on_flag);
	return g_bin_desc_flag;
}

static int hx_zf_part_info(const struct firmware *fw, int type)
{
	uint32_t table_addr = NC_CFG_TABLE_FLASH_ADDR;
	int pnum = 0;
	int ret = UPDATE_FAIL;
	uint8_t buf[16];
	struct zf_info *info = NULL;
	uint8_t *cfg_buf = NULL;
	uint8_t sram_min[4];
	int cfg_sz = 0;
	int cfg_crc_sw = 0;
	int cfg_crc_hw = 0;
	uint8_t i = 0;
	int i_max = 0;
	int i_min = 0;
	uint32_t dsram_base = 0xFFFFFFFF;
	uint32_t dsram_max = 0;
	int retry = 0;

	crc_data_prepare(fw);

	/* 1. initial check */
	pnum = fw->data[table_addr + 12];
	if (pnum < 2) {
		TS_LOG_ERR("%s: partition number is not correct\n", __func__);
		return UPDATE_FAIL;
	}
	info = kcalloc(pnum, sizeof(struct zf_info), GFP_KERNEL);
	if (info == NULL) {
		TS_LOG_ERR("%s: memory allocation fail[info]!!\n", __func__);
		return UPDATE_FAIL;
	}
	memset(info, 0, pnum * sizeof(struct zf_info));

	/* 2. record partition information */
	memcpy(buf, &fw->data[table_addr], 16);
	memcpy(info[0].sram_addr, buf, 4);
	info[0].write_size = buf[7] << 24 | buf[6] << 16 | buf[5] << 8 | buf[4];
	info[0].fw_addr = buf[11] << 24 | buf[10] << 16 | buf[9] << 8 | buf[8];

	for (i = 1; i < pnum; i++) {
		memcpy(buf, &fw->data[i*0x10 + table_addr], 16);

		memcpy(info[i].sram_addr, buf, 4);
		info[i].write_size = buf[7] << 24 | buf[6] << 16
				| buf[5] << 8 | buf[4];
		info[i].fw_addr = buf[11] << 24 | buf[10] << 16
				| buf[9] << 8 | buf[8];
		info[i].cfg_addr = info[i].sram_addr[0];
		info[i].cfg_addr += info[i].sram_addr[1] << 8;
		info[i].cfg_addr += info[i].sram_addr[2] << 16;
		info[i].cfg_addr += info[i].sram_addr[3] << 24;

		if (info[i].cfg_addr % 4 != 0)
			info[i].cfg_addr -= (info[i].cfg_addr % 4);

		TS_LOG_INFO("%s,[%d]SRAM addr=%08X, fw_addr=%08X, write_size=%d\n",
			__func__, i, info[i].cfg_addr, info[i].fw_addr,
			info[i].write_size);

		if (dsram_base > info[i].cfg_addr) {
			dsram_base = info[i].cfg_addr;
			i_min = i;
		}
		if (dsram_max < info[i].cfg_addr) {
			dsram_max = info[i].cfg_addr;
			i_max = i;
		}
	}

	/* 3. prepare data to update */

	for (i = 0; i < FOUR_BYTE_CMD; i++)
		sram_min[i] = (info[i_min].cfg_addr>>(8*i)) & 0xFF;

	cfg_sz = (dsram_max - dsram_base) + info[i_max].write_size;
	if (cfg_sz % 16 != 0)
		cfg_sz = cfg_sz + 16 - (cfg_sz % 16);

	TS_LOG_INFO("%s, cfg_sz = %d!, dsram_base = %X, dsram_max = %X\n", __func__,
			cfg_sz, dsram_base, dsram_max);
	/* config size should be smaller than DSRAM size */
	if (cfg_sz > DSRAM_SIZE) {
		TS_LOG_ERR("%s: config size error[%d, %d]!!\n", __func__,
			cfg_sz, DSRAM_SIZE);
		ret = UPDATE_FAIL;
		goto ALOC_CFG_BUF_FAIL;
	}

	cfg_buf = kcalloc(cfg_sz, sizeof(uint8_t), GFP_KERNEL);
	if (cfg_buf == NULL) {
		TS_LOG_ERR("%s: memory allocation fail[cfg_buf]!!\n", __func__);
		ret = UPDATE_FAIL;
		goto ALOC_CFG_BUF_FAIL;
	}

	for (i = 1; i < pnum; i++) {
		if (info[i].fw_addr + info[i].write_size > fw->size) {
			TS_LOG_ERR("%s: Out of bounds:fw_addr=%08X, write_size=%d, fw->size=%d\n", __func__,
				info[i].fw_addr, info[i].write_size, fw->size);
			goto BURN_SRAM_FAIL;
		}
		memcpy(&cfg_buf[info[i].cfg_addr - dsram_base],
			&fw->data[info[i].fw_addr], info[i].write_size);
	}

	/* 4. write to sram */
	/* FW entity */
	if (hx_sram_write_crc_check(info[0].sram_addr,
	&fw->data[info[0].fw_addr], info[0].write_size) != 0) {
		TS_LOG_ERR("%s: HW CRC FAIL\n", __func__);
		ret = UPDATE_FAIL;
		goto BURN_SRAM_FAIL;
	}

	cfg_crc_sw = himax_mcu_Calculate_CRC_with_AP(cfg_buf, 0, cfg_sz);
	do {
		hx_write_sram_0f(sram_min, cfg_buf, cfg_sz);
		cfg_crc_hw = hx_zf_hw_check_crc(sram_min, cfg_sz);
		if (cfg_crc_hw != cfg_crc_sw) {
			TS_LOG_ERR("Cfg CRC FAIL,HWCRC=%X,SWCRC=%X,retry=%d\n",
				cfg_crc_hw, cfg_crc_sw, retry);
		}
	} while (cfg_crc_hw != cfg_crc_sw && retry++ < 3);

	if (retry > 3) {
		ret = UPDATE_FAIL;
		goto BURN_SRAM_FAIL;
	}
	ret = UPDATE_PASS;
BURN_SRAM_FAIL:
	kfree(cfg_buf);
ALOC_CFG_BUF_FAIL:
	kfree(info);
	return ret;
/* ret = 1, memory allocation fail
 *     = 2, crc fail
 *     = 3, flow control error
 */
}

int hx_zf_fw_upgrade(const struct firmware *fw)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	int fw_update_ststus = DATA_INIT;

	HX_ZF_UPDATE_FLAG = UPDATE_ONGOING;
	TS_LOG_INFO("%s: enter\n", __func__);

	hx_addr_reg_assign(ADDR_AHBSPI_SYSRST, DATA_AHBSPI_SYSRST, tmp_addr, tmp_data);
	himax_zf_register_write(tmp_addr, FOUR_BYTE_CMD, tmp_data);

	himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 0);
	hx_zf_sense_off();
	hx_mcu_bin_desc_get((unsigned char *)fw->data, HX1K);

	fw_update_ststus = hx_zf_part_info(fw, HX_BOOT_UPDATE);
	hx_zf_reload_disable();
	hx_zf_power_on_init();

	himax_zf_int_enable(g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->irq_id, 1);

	TS_LOG_INFO("%s: end\n", __func__);
	HX_ZF_UPDATE_FLAG = UPDATE_DONE;

	return fw_update_ststus;
}
/* Zero_flash_update end*/

void  firmware_update(const struct firmware *fw)
{
	int retval = HX_ERR;

	if (!fw->data || !fw->size) {
		TS_LOG_INFO("HXTP fw requst fail\n");
		return;
	}
	g_himax_zf_ts_data->firmware_updating = true;

	TS_LOG_INFO("HXTP fw size = %u\n", (unsigned int)fw->size);
	__pm_stay_awake(&g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);
	TS_LOG_INFO("HXTP flash write start");
	retval = hx_zf_fw_upgrade(fw);
	if (retval == 0) {
		TS_LOG_ERR("%s:HXTP  TP upgrade error\n", __func__);
	} else {
		TS_LOG_INFO("%s: HXTP TP upgrade OK\n", __func__);
	}
	__pm_relax(&g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);
	TS_LOG_INFO("fw_update function end\n");
	g_himax_zf_ts_data->firmware_updating = false;

	return;
}

static void hx_get_fw_name(char *file_name)
{
	char *firmware_form = ".bin";

	strncat(file_name, himax_zf_project_id, strlen(himax_zf_project_id) + 1);
	strncat(file_name, ".bin", strlen(".bin"));
	TS_LOG_INFO("%s, firmware name: %s\n", __func__, file_name);
}

int hx_zf_fw_resume_update(char *file_name)
{
	int err = NO_ERR;
	const  struct firmware *fw_entry = NULL;

	TS_LOG_INFO("%s, request_firmware name: %s\n", __func__, file_name);
	err = request_firmware(&fw_entry, file_name,
		&g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_dev->dev);

	if (err < 0) {
		TS_LOG_ERR("HXTP %s %d: Fail request firmware %s, retval = %d\n",
			__func__, __LINE__, file_name, err);
		err = 0;
		goto err_request_firmware;
	}
	fw_update_boot_sd_flag = FW_UPDATE_BOOT;
	firmware_update(fw_entry);
	release_firmware(fw_entry);
	himax_zf_read_FW_info();
	TS_LOG_INFO("%s: end!\n", __func__);
	mdelay(10);

err_request_firmware:
	return err;
}

int hx_zf_fw_update_boot(char *file_name)
{
	int err = NO_ERR;
	const  struct firmware *fw_entry = NULL;
	char firmware_name[64] = "";

	TS_LOG_INFO("start to request firmware  %s", file_name);
	hx_get_fw_name(file_name);
	snprintf(firmware_name, sizeof(firmware_name), "ts/%s", file_name);

	memcpy(himax_firmware_name, firmware_name, strlen(firmware_name));

	TS_LOG_INFO("%s:request_firmware name: %s\n", __func__, firmware_name);
	err = request_firmware(&fw_entry, firmware_name, &g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->ts_dev->dev);
	if (err < 0) {
		TS_LOG_ERR("HXTP %s %d: Fail request firmware %s, retval = %d\n",
			__func__, __LINE__, firmware_name, err);
		err = 0; // huawei request :request_firmware fail, return NO ERROR
		goto err_request_firmware;
	}
	fw_update_boot_sd_flag = FW_UPDATE_BOOT;
	firmware_update(fw_entry);
	release_firmware(fw_entry);
	TS_LOG_INFO("%s: end!\n", __func__);

err_request_firmware:
	return err;
}

int hx_zf_fw_update_sd(void)
{
	int retval = 0;
	char firmware_name[100] = "";
	const struct firmware *fw_entry = NULL;

	TS_LOG_INFO("%s: enter!\n", __func__);
	sprintf(firmware_name, HX_FW_NAME);

	TS_LOG_INFO("HXTP start to request firmware %s", firmware_name);
	retval = request_firmware(&fw_entry, firmware_name, &g_himax_zf_ts_data->tskit_himax_data->ts_platform_data->client->dev);
	if (retval < 0) {
		TS_LOG_ERR("HXTP %s %d: Fail request firmware %s, retval = %d\n",
		__func__, __LINE__, firmware_name, retval);
		retval = 0;//huawei request :request_firmware fail, return NO ERROR
		return retval;
	}
	fw_update_boot_sd_flag = FW_UPDATE_SD;
	firmware_update(fw_entry);
	release_firmware(fw_entry);
	TS_LOG_INFO("%s: end!\n", __func__);
	return retval;
}
