/* Himax Android Driver Sample Code for HMX852xF chipset
*
* Copyright (C) 2017 Himax Corporation.
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

unsigned int hx_nc_update_flag = 0;
unsigned int hx_nc_reset_count = 0;
unsigned int hx_nc_esd_reset_count = 0;
unsigned long nc_fw_ver_maj_flash_addr = 0;
unsigned long nc_fw_ver_min_flash_addr = 0;
unsigned long nc_cfg_ver_maj_flash_addr = 0;
unsigned long nc_cfg_ver_min_flash_addr = 0;
unsigned long nc_cid_ver_maj_flash_addr = 0;
unsigned long nc_cid_ver_min_flash_addr = 0;


unsigned long fw_ver_maj_flash_leng = 0;
unsigned long fw_ver_min_flash_leng = 0;
unsigned long cfg_ver_maj_flash_leng = 0;
unsigned long cfg_ver_min_flash_leng = 0;
unsigned long cid_ver_maj_flash_leng = 0;
unsigned long cid_ver_min_flash_leng = 0;


extern char himax_nc_project_id[];
extern struct himax_ts_data *g_himax_nc_ts_data;

#define BOOT_UPDATE_FIRMWARE_FLAG_FILENAME "/system/etc/tp_test_parameters/boot_update_firmware.flag"
#define BOOT_UPDATE_FIRMWARE_FLAG "boot_update_firmware_flag:"

int hx_nc_rx_num = 0;
int hx_nc_tx_num = 0;
int hx_nc_bt_num = 0;
int hx_nc_x_res = 0;
int hx_nc_y_res = 0;
int hx_nc_max_pt = 0;
unsigned char ic_nc_checksum = 0;
unsigned char ic_nc_type = 0;
bool hx_nc_xy_reverse = false;
bool hx_int_is_edge = false;
int fw_update_boot_sd_flag = 0;

static int himax_check_update_firmware_flag(void);
extern void himax_nc_reload_disable(int on);

static int check_firmware_version(const struct firmware *fw)
{
	if (fw == NULL) {
		return FW_NO_EXIST;
	}
	if (!fw->data) {
		TS_LOG_INFO("======== fw requst fail =====");
		return FW_NO_EXIST;
	}
	TS_LOG_INFO("check_firmware_version: IMAGE FW_VER=%x.\n",
		((fw->data[nc_fw_ver_maj_flash_addr] << 8) |
		fw->data[nc_fw_ver_min_flash_addr]));
	TS_LOG_INFO("check_firmware_version: IMAGE CFG_VER=%x.\n",
		((fw->data[nc_cfg_ver_maj_flash_addr] << 8) |
		fw->data[nc_cfg_ver_min_flash_addr]));

	TS_LOG_INFO("check_firmware_version: curr FW_VER=%x.\n",
		g_himax_nc_ts_data->vendor_fw_ver);
	TS_LOG_INFO("check_firmware_version: curr CFG_VER=%x.\n",
		g_himax_nc_ts_data->vendor_config_ver);
	if (fw_update_boot_sd_flag == FW_UPDATE_BOOT) {
		if ((g_himax_nc_ts_data->vendor_fw_ver < ((fw->data[nc_fw_ver_maj_flash_addr] << 8) |
			fw->data[nc_fw_ver_min_flash_addr])) || (g_himax_nc_ts_data->vendor_config_ver <
			((fw->data[nc_cfg_ver_maj_flash_addr] << 8) | fw->data[nc_cfg_ver_min_flash_addr]))) {
			TS_LOG_INFO("firmware is lower, must upgrade.\n");
			return FW_NEED_TO_UPDATE;
		} else {
			TS_LOG_INFO("firmware not lower.\n");
			return FW_NO_NEED_TO_UPDATE;
		}
	} else if (fw_update_boot_sd_flag == FW_UPDATE_SD) {
		if ((g_himax_nc_ts_data->vendor_fw_ver != ((fw->data[nc_fw_ver_maj_flash_addr] << 8) |
			fw->data[nc_fw_ver_min_flash_addr])) || (g_himax_nc_ts_data->vendor_config_ver !=
			((fw->data[nc_cfg_ver_maj_flash_addr] << 8) | fw->data[nc_cfg_ver_min_flash_addr]))) {
			TS_LOG_INFO("firmware is different, must upgrade.\n");
			return FW_NEED_TO_UPDATE;
		} else {
			TS_LOG_INFO("firmware not different.\n");
			return FW_NO_NEED_TO_UPDATE;
		}
	}
	return FW_NO_NEED_TO_UPDATE;
}

static int check_need_firmware_upgrade(const struct firmware *fw)
{
	/* first, check firmware version */
	int retval;
	if (himax_nc_calculate_checksum(false) == 0) {
		TS_LOG_INFO("checksum failed, must upgrade.\n");
		return CAL_CHECKSUM_RUN_FAIL;
	} else {
		TS_LOG_INFO("checksum success.\n");
	}

	if (check_firmware_version(fw)) {
		retval = himax_check_update_firmware_flag();
		if (retval == 0) {
			TS_LOG_INFO("now is factory test mode, not update firmware\n");
			return 0;
		}
		return FW_NEED_TO_UPDATE;
	}

	return FW_NO_NEED_TO_UPDATE;
}

static void firmware_update(const struct firmware *fw)
{
	int retval = HX_ERR;
	int fullfilelength;

	if (!fw->data || !fw->size) {
		TS_LOG_INFO("firmware_update: fw requst fail\n");
		return;
	}
	g_himax_nc_ts_data->firmware_updating = true;
	himax_nc_hw_reset(HX_LOADCONFIG_DISABLE, HX_INT_EN);
	TS_LOG_INFO("firmware_update: fw size =%u \n", (unsigned int)fw->size);
	fullfilelength = (unsigned int)fw->size;

	switch (fullfilelength) {
	case FW_SIZE_64K:
	case FW_SIZE_128K:
		TS_LOG_INFO("%s: Now FW size is = %d\n", __func__,
			fullfilelength);
		break;
	default:
		TS_LOG_ERR("%s: FW size is fail = %d\n", __func__,
			fullfilelength);
		goto END_FUNC;
	}
	if ((check_need_firmware_upgrade(fw)) ||
		(fw_update_boot_sd_flag == FW_UPDATE_SD)) {
		__pm_stay_awake(&g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);
		TS_LOG_INFO("firmware_update: flash write start");
		retval = hx_nc_fts_ctpm_fw_upgrade_with_fs(
			(unsigned char *)fw->data, fullfilelength, true);
		if (retval == 0) {
			TS_LOG_ERR("%s: TP upgrade error\n", __func__);
		} else {
			TS_LOG_INFO("%s: TP upgrade OK\n", __func__);
			himax_nc_reload_disable(0);
			g_himax_nc_ts_data->vendor_fw_ver =
				(fw->data[nc_fw_ver_maj_flash_addr] <<
				LEFT_MOV_8BIT) |
				fw->data[nc_fw_ver_min_flash_addr];
			g_himax_nc_ts_data->vendor_config_ver =
				(fw->data[nc_cfg_ver_maj_flash_addr] <<
				LEFT_MOV_8BIT) |
				fw->data[nc_cfg_ver_min_flash_addr];
			TS_LOG_INFO("firmware_update: upgraded IMAGE FW_VER = %x.\n",
				(fw->data[nc_fw_ver_maj_flash_addr] <<
				LEFT_MOV_8BIT) |
				fw->data[nc_fw_ver_min_flash_addr]);
			TS_LOG_INFO("firmware_update: upgraded IMAGE CFG_VER = %x.\n",
				(fw->data[nc_cfg_ver_maj_flash_addr] <<
				LEFT_MOV_8BIT) |
				fw->data[nc_cfg_ver_min_flash_addr]);
		}
		__pm_relax(&g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->ts_wake_lock);
		TS_LOG_INFO("firmware_update: i_update function end ");
	} else {
		TS_LOG_INFO("firmware_update: don't need upgrade firmware");
	}
	himax_nc_hw_reset(HX_LOADCONFIG_EN, HX_INT_EN);
	g_himax_nc_ts_data->firmware_updating = false;
END_FUNC:
	return;
}


int himax_nc_loadsensorconfig(void)
{
	himax_nc_get_information();
	TS_LOG_INFO("nc_loadsensorconfig: initialization complete\n");

	return LOAD_SENSORCONFIG_OK;
}

static int himax_file_open_firmware(uint8_t *file_path,
	uint8_t *databuf, int *file_size)
{
	int retval = 0;
	unsigned int file_len;
	struct file *filp = NULL;
	struct inode *inode = NULL;
	mm_segment_t oldfs;

	if (file_path == NULL || databuf == NULL) {
		TS_LOG_ERR("file_open_firmware: path || buf is NULL\n");
		return -EINVAL;
	}

	TS_LOG_INFO("file_open_firmware: path = %s\n", file_path);
	oldfs = get_fs();
	set_fs(KERNEL_DS);
	filp = filp_open(file_path, O_RDONLY, S_IRUSR); // open file
	if (IS_ERR(filp)) {
		TS_LOG_ERR("file_open_firmware: open %s error\n", file_path);
		retval = -EIO;
		goto err;
	}

	if (filp->f_op == NULL) {
		TS_LOG_ERR("file_open_firmware: File Operation Method Error\n");
		retval = -EINVAL;
		goto exit;
	}

	inode = filp->f_path.dentry->d_inode;
	if (inode == NULL) {
		TS_LOG_ERR("file_open_firmware: Get inode from filp failed\n");
		retval = -EINVAL;
		goto exit;
	}

	// Get file size
	file_len = i_size_read(inode->i_mapping->host);
	if (file_len == 0) {
		TS_LOG_ERR("file_open_firmware: file size error,file_len = %d\n",
			file_len);
		retval = -EINVAL;
		goto exit;
	}

	// read image data to kernel
	if (filp->f_op->read(filp, databuf, file_len,
		&filp->f_pos) != file_len) {
		TS_LOG_ERR("file_open_firmware: file read error\n");
		retval = -EINVAL;
		goto exit;
	}
	*file_size = file_len;
exit:
	filp_close(filp, NULL);
err:
	set_fs(oldfs);
return retval;
}


static int himax_check_update_firmware_flag(void)
{
	int retval;
	int file_size = 0;
	int update_flag = 0;
	uint8_t *tmp = NULL;
	uint8_t *file_data = NULL;
	uint8_t *file_path = BOOT_UPDATE_FIRMWARE_FLAG_FILENAME;

	file_data = kzalloc(128, GFP_KERNEL);
	if (file_data == NULL) {
		TS_LOG_ERR("check_update_firmware_flag: kzalloc error\n");
		return -EINVAL;
	}
	retval = himax_file_open_firmware(file_path, file_data, &file_size);
	if (retval != 0) {
		TS_LOG_ERR("check_update_firmware_flag, file_open error, code = %d\n",
			retval);
		goto exit;
	}
	tmp = strstr(file_data, BOOT_UPDATE_FIRMWARE_FLAG);
	if (tmp == NULL) {
		TS_LOG_ERR("%s not found\n", BOOT_UPDATE_FIRMWARE_FLAG);
		retval = -1;
		goto exit;
	} else {
		tmp = tmp + strlen(BOOT_UPDATE_FIRMWARE_FLAG);
		sscanf(tmp, "%d", &update_flag);
	}
	if (update_flag == 1) {
		retval = 0;
		TS_LOG_INFO("check_update_firmware_flag: check success, flag=%d\n",
			update_flag);
		goto exit;
	} else {
		retval = -1;
		TS_LOG_INFO("check_update_firmware_flag: check failed, flag=%d\n",
			update_flag);
		goto exit;
	}
exit:
	kfree(file_data);
	file_data = NULL;
	return retval;
}
static void himax_get_fw_name(char *file_name)
{
	char *firmware_form = ".bin";
	char joint_chr = '_';

	strncat(file_name, himax_nc_project_id, strlen(himax_nc_project_id) + 1);
	strncat(file_name, &joint_chr, 1);
	strncat(file_name, g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->chip_data->module_name,
		strlen(g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->chip_data->module_name) + 1);
	strncat(file_name, firmware_form, strlen(firmware_form) + 1);
	TS_LOG_INFO("get_fw_name: firmware name: %s\n", file_name);
}
int himax_nc_fw_update_boot(char *file_name)
{
	int err;
	const struct firmware *fw_entry = NULL;
	char firmware_name[64] = "";
	char *fir_name = firmware_name;

	TS_LOG_INFO("fw_update_boot: enter\n");
	TS_LOG_INFO("fw_update_boot: start to request firmware %s", file_name);
	himax_get_fw_name(file_name);
	snprintf(firmware_name, sizeof(firmware_name), "ts/%s", file_name);

	TS_LOG_INFO("fw_update_boot: request_firmware name: %s\n",
		firmware_name);
	err = request_firmware(&fw_entry, fir_name,
		&g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->ts_dev->dev);

	if (err < 0) {
		TS_LOG_ERR("fw_update_boot: %d: Fail request firmware %s, retval = %d\n",
			__LINE__, firmware_name, err);
		err = 0; // huawei request :request_firmware fail, return NO ERROR
		goto err_request_firmware;
	}
	fw_update_boot_sd_flag = FW_UPDATE_BOOT;
	firmware_update(fw_entry);
	release_firmware(fw_entry);
	TS_LOG_INFO("fw_update_boot: end\n");

err_request_firmware:

	return err;
}

int himax_nc_fw_update_sd(void)
{
	int retval;
	char firmware_name[100] = "";
	const struct firmware *fw_entry = NULL;
	char *fir_name = firmware_name;

	TS_LOG_INFO("fw_update_sd: enter\n");
	sprintf(firmware_name, HX_FW_NAME);

	TS_LOG_INFO("fw_update_sd: start to request firmware %s",
		firmware_name);
	retval = request_firmware(&fw_entry, fir_name,
		&g_himax_nc_ts_data->tskit_himax_data->ts_platform_data->client->dev);
	if (retval < 0) {
		TS_LOG_ERR("fw_update_sd: %d: Fail request firmware %s, retval = %d\n",
			__LINE__, firmware_name, retval);
		retval = 0; // huawei request :request_firmware fail, return NO ERROR
		return retval;
	}
	fw_update_boot_sd_flag = FW_UPDATE_SD;
	firmware_update(fw_entry);
	release_firmware(fw_entry);

	TS_LOG_INFO("fw_update_sd: end\n");

	return retval;
}
